// ─── ribbon.inl (IMPL: post-class definitions) ───────────────────
// Impl of ribbon.hpp (LADDER-3 c5): history in audit/LADDER.md.
//
// Definitions for ribbon.hpp's declared lifecycle + conductor + head-law
// functions. The bodies reach c->gpuState_ / c->time_state_ / c->player_ /
// c->inputState_ / c->visual_canvas_ + the four ribbon canvas bindings
// and the spawn-engine services (run_spawn_preamble /
// negotiate_position / record_placement_bookkeeping) + the tile_world
// surface samplers (estimate_terrain_height / terrain_tile_warm),
// plus THEMES (population_themes.hpp) and PATCH_EXTENT
// (patch_system.hpp); PopFamily is roster.hpp vocabulary.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
//
// PAIRING (the mirror law, AMENDED at LADDER-3 c5): the byte-identical
// mirror with the_chord's ribbon.inl is SUSPENDED by the header ladder's
// declared structural divergence — see the amendment in ribbon.hpp's
// banner. The BOM stays with this file on both sides.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>   // std::max, std::min
#include <array>       // body pose staging
#include <cfloat>      // FLT_MAX (nearest-slot adoption)
#include <cmath>       // std::sin, std::cos, std::atan2, std::exp, std::sqrt, std::fabs, std::floor, std::remainder, std::atan
#include <cstdint>
#include <cstring>     // std::memcpy (placement color copy)
#include <iostream>    // the spawn log

namespace t7 {
namespace the_board {

// ═══ AUTHOR SEATS ════════════════════════════════════════════════
//
// The steering integrator has three seats, all writing the same two
// inputs (yaw_in, throttle_in): the PLAYER (cartridge frame block,
// sky mode), the WANDERER below (the idle script), and one EMPTY
// SEAT reserved for the musical canvas. One control law, many
// authors.

inline float wander_rand01(uint32_t& s) {
    // xorshift32 → [0,1)
    s ^= s << 13; s ^= s >> 17; s ^= s << 5;
    return (float)(s & 0x00FFFFFFu) / 16777216.0f;
}

inline void ribbon_wander_inputs(ActiveRibbon& ar,
                                 float head_x, float head_z, float heading,
                                 float dt, float& yaw_in, float& thr_in)
{
    // Free roam: waypoints are picked AHEAD of the current motion — a bearing
    // spread around where the ribbon is already going, a leg of 200-500 units.
    // No leash: a rendered wanderer is pinned against eviction instead, and
    // the yaw cap below keeps every turn at body scale (radius >= RIBBON_R_MIN /
    // WANDER_YAW_MAX). Gorgeous, contemplative arcs by construction.
    ar.wander_retarget -= dt;
    const float wdx = ar.wander_tx - head_x;
    const float wdz = ar.wander_tz - head_z;
    if (ar.wander_retarget <= 0.0f
        || wdx * wdx + wdz * wdz < WANDER_ARRIVE_RADIUS * WANDER_ARRIVE_RADIUS) {
        const float move_dir = heading + 3.14159265f;           // movement = -heading
        const float spread = (wander_rand01(ar.wander_rng) * 2.0f - 1.0f) * WANDER_SPREAD;
        const float leg = WANDER_LEG_MIN
                        + (WANDER_LEG_MAX - WANDER_LEG_MIN) * wander_rand01(ar.wander_rng);
        const float b = move_dir + spread;
        ar.wander_tx = head_x + leg * std::cos(b);
        ar.wander_tz = head_z + leg * std::sin(b);
        ar.wander_retarget = WANDER_RETARGET_MIN
                           + WANDER_RETARGET_VAR * wander_rand01(ar.wander_rng);
    }

    const float bearing = std::atan2(ar.wander_tz - head_z, ar.wander_tx - head_x);
    const float desired = bearing + 3.14159265f;                // movement = -heading
    const float err = std::remainder(desired - heading, 6.2831853f);
    float cmd = err / WANDER_STEER_SOFT;
    cmd = (cmd >  WANDER_YAW_MAX) ?  WANDER_YAW_MAX :
          (cmd < -WANDER_YAW_MAX) ? -WANDER_YAW_MAX : cmd;
    // Ease the steering: the body replays the heading history, and bang-bang
    // commands print elbows. First-order toward the command keeps curvature
    // continuous — turns enter and exit as curves, never as joints.
    const float ease = 1.0f - std::exp(-dt / WANDER_YAW_TAU);
    ar.wander_yaw_state += (cmd - ar.wander_yaw_state) * ease;
    yaw_in = ar.wander_yaw_state;
    thr_in = ar.wander_cruise;
}

// ═══ HEAD LAWS ═══════════════════════════════════════════════════
//
// The head mover: the steering integrator, the altitude pen, the
// propagation history, and THE LAW that rebuilds the body from it.
// CPU authors intent; the GPU realizes geometry — these laws write
// the GPU exactly once per frame, through
// GPUState::upload_ribbon_head_poses (a dumb wire).

inline void ribbon_history_sample(const RibbonState& rs, float age, float& h, float& y) {
    const RibbonHead& hd = rs.head;
    float fidx = age / RibbonHead::HIST_DT;
    if (fidx < 0.0f) fidx = 0.0f;
    const float fmax = static_cast<float>(RibbonHead::HIST_CAP - 2u);
    if (fidx > fmax) fidx = fmax;
    const uint32_t j0 = static_cast<uint32_t>(fidx);
    const float frac = fidx - static_cast<float>(j0);
    const uint32_t i0 = (hd.hist_head + RibbonHead::HIST_CAP - j0) % RibbonHead::HIST_CAP;
    const uint32_t i1 = (hd.hist_head + RibbonHead::HIST_CAP - (j0 + 1u)) % RibbonHead::HIST_CAP;
    h = hd.hist_heading[i0] + (hd.hist_heading[i1] - hd.hist_heading[i0]) * frac;
    y = hd.hist_y[i0]       + (hd.hist_y[i1]       - hd.hist_y[i0])       * frac;
}

// THE LAW: the body is the head's past, replayed at propagation
// speed. Ring k wears the head's state from age = k·spacing/P
// seconds ago — its delayed heading orients the segment and fills
// the yaw channel (continuous by construction); its delayed y is
// the altitude. XZ integrates the delayed heading TAILWARD
// (+heading) from the live head. A turn made now travels down the
// body at P through space; so does an altitude swell; so will
// every musical gesture at the head. Replaces the spatial trail:
// a bend is motion, not a mark on the floor.
inline void ribbon_rebuild_body_upload(RibbonState& rs, GPUState& gpuState,
                                       wgpu::Queue& queue, const GPURibbonState& ribbon,
                                       float head_x, float head_y, float head_z) {
    const uint32_t n = std::min(ribbon.cube_count, Dim::RIBBON_MAX_RINGS);
    if (n < 2u) return;
    const float spacing = ribbon.cube_size;
    const float inv_p = 1.0f / std::max(ribbon.propagation_speed, 0.001f);

    std::array<float, 4 * Dim::RIBBON_MAX_RINGS> poses{};
    poses[0] = head_x;
    poses[1] = head_y;
    poses[2] = head_z;
    poses[3] = rs.head.heading;   // ring 0: the live head, live heading

    float px = head_x, pz = head_z;
    for (uint32_t k = 1u; k < n; ++k) {
        const float age = (static_cast<float>(k) * spacing) * inv_p;
        float h, y;
        ribbon_history_sample(rs, age, h, y);
        px += spacing * std::cos(h);   // tailward = +heading
        pz += spacing * std::sin(h);
        poses[4u * k + 0u] = px;
        poses[4u * k + 1u] = y;
        poses[4u * k + 2u] = pz;
        poses[4u * k + 3u] = h;        // yaw channel: the delayed heading
    }
    gpuState.upload_ribbon_head_poses(queue, poses.data(),
                                      poses.size() * sizeof(float));
}

inline void ribbon_advance_head(RibbonState& rs, GPUState& gpuState,
                                wgpu::Queue& queue, const GPURibbonState& ribbon,
                                uint32_t slot, float t,
                                bool flown, float yaw_in, float throttle_in, float dt,
                                float ground_y, bool ground_valid) {
    RibbonHead& hd = rs.head;

    if (!hd.seeded || hd.slot != slot) {
        hd.origin[0] = ribbon.anchor[0];
        hd.origin[2] = ribbon.anchor[2];
        hd.heading = ribbon.orientation;
        hd.pos[0] = hd.origin[0];
        hd.pos[2] = hd.origin[2];
        hd.hist_heading.fill(hd.heading);
        hd.hist_head = 0;
        hd.hist_time = t;
        hd.alt_baked = false;   // altitude bakes below, latching on the first warm ground sample
        hd.seeded = true;
        hd.slot = slot;
    }

    // THE BAKE — altitude is a birthright: the ground of the birthplace
    // plus the seed-drawn clearance (ribbon.height), baked ONCE at first
    // truth, never chased. After a mood flip the estimator returns 0 for
    // a still-cold tile — indistinguishable from flat ground by value —
    // so the bake re-runs each frame until the call site reports a WARM
    // sample, then latches. Parked ribbons hold this number forever.
    // SEAM[ribbon:sky-mode].
    if (!hd.alt_baked) {
        hd.origin[1]  = ground_y + ribbon.height;   // first truth, once
        hd.pos[1]     = hd.origin[1];
        hd.alt_target = hd.origin[1];
        hd.y_vel      = 0.0f;
        hd.hist_y.fill(hd.pos[1]);   // the body's constant past re-bases with the bake
        hd.alt_baked  = ground_valid;
    }

    float head_x, head_y, head_z;
    if (flown) {
        // Planar flight: yaw the heading, throttle the head along it in
        // the horizontal plane. The head moves along -heading so the
        // straight seed (laid +heading from the anchor) trails behind it.
        // Pitch is deferred (the frame is horizontal-only by
        // construction); altitude is managed by the pen below. Constants
        // live in the tuning console (head control law). SEAM[ribbon:sky-mode].
        // Steering model: yaw is STEERING, not free aim. The available
        // yaw rate is min(RIBBON_YAW_RATE, speed / RIBBON_R_MIN): the
        // heading can only change while moving, and the flown path can
        // never be tighter than the minimum turn radius. So the velocity
        // is always the face's outward normal (trajectory orthogonal to
        // the face, by construction), the face can never turn inward
        // against the body, and heading-vs-path divergence stays at a
        // few degrees. Reverse is forbidden (a snake does not burrow
        // into its own body): down-arrow is no thrust.
        // SEAM[ribbon:sky-mode].
        const float speed = std::max(throttle_in, 0.0f) * RIBBON_MAX_SPEED;
        const float yaw_avail = std::min(RIBBON_YAW_RATE, speed / RIBBON_R_MIN);
        hd.heading += yaw_in * yaw_avail * dt;
        const float ch = std::cos(hd.heading);
        const float sh = std::sin(hd.heading);
        const float step = speed * dt;
        hd.pos[0] -= ch * step;
        hd.pos[2] -= sh * step;
        // Sky altitude with a terrain FLOOR (not a tether): the target is
        // the ribbon's baked birthright altitude, floored by the smoothed
        // ground plus a constant safety margin. Valleys and mood changes
        // leave it untouched at its sky; only ground tall enough to
        // threaten it lifts it — and it settles back beyond. The low-pass
        // keeps the floor reading the LANDSCAPE (long swells); zero travel
        // (hover) freezes the target; the critically damped PEN below
        // turns every correction into a smooth S-curve — never a
        // constant-rate ramp, never a corner.
        {
            const float floor_y = ground_y + RIBBON_FLOOR_MARGIN;
            const float raw_target = (hd.origin[1] > floor_y)
                                   ? hd.origin[1] : floor_y;
            const float travel = std::fabs(step);   // this frame's distance
            const float alpha = 1.0f - std::exp(-travel / RIBBON_ALT_SMOOTH_DIST);
            hd.alt_target += (raw_target - hd.alt_target) * alpha;

            const float damp = 2.0f * std::sqrt(RIBBON_ALT_STIFF);
            hd.y_vel += ((hd.alt_target - hd.pos[1]) * RIBBON_ALT_STIFF
                         - damp * hd.y_vel) * dt;
            hd.y_vel = (hd.y_vel >  RIBBON_CLIMB_RATE) ?  RIBBON_CLIMB_RATE :
                       (hd.y_vel < -RIBBON_CLIMB_RATE) ? -RIBBON_CLIMB_RATE : hd.y_vel;
            hd.pos[1] += hd.y_vel * dt;
        }

        head_x = hd.pos[0];
        head_y = hd.pos[1];
        head_z = hd.pos[2];
    } else {
        hd.heading = ribbon.orientation;
        hd.pos[0] = ribbon.anchor[0];
        hd.pos[1] = hd.origin[1];
        hd.pos[2] = ribbon.anchor[2];
        head_x = hd.pos[0];
        head_y = hd.pos[1];
        head_z = hd.pos[2];
    }

    // Pawn mount point (sky mode): the SEAT — centerline + the wave in
    // the ring frame, sampled at the SADDLE's arc age (s_age), lifted
    // half a tube along the seat frame's up (seat polish) so the pawn's
    // feet ride the top face through roll and pitch. Mirrors
    // ribbon_spine_at at the seat's arc: ring 0's yaw-channel value IS
    // the live heading (head_poses[0].w), so right = (-sin h, 0, cos h),
    // wave vertical on world-up. The pawn, its frame, and the tube
    // beneath the seat share one wave at one age — the saddle's —
    // exactly. SEAM[ribbon:sky-mode].
    {
        const float p_spd = std::max(ribbon.propagation_speed, 1e-3f);
        const float s_age = ribbon.time - RIBBON_MOUNT_SETBACK / p_spd;
        const float lat = std::sin(ribbon.lateral_freq  * s_age) * ribbon.lateral_amp;
        const float ver = std::sin(ribbon.vertical_freq * s_age) * ribbon.vertical_amp;
        const float ch  = std::cos(hd.heading);
        const float sh  = std::sin(hd.heading);

        const float sl_lat = std::cos(ribbon.lateral_freq  * s_age)
                           * ribbon.lateral_amp  * ribbon.lateral_freq;
        const float sl_ver = std::cos(ribbon.vertical_freq * s_age)
                           * ribbon.vertical_amp * ribbon.vertical_freq;
        // Negated tangent-align, in LOCKSTEP with the GPU ring motor
        // (the drift-trap resolution: the sweep test flipped the shader
        // side; this mirror flips with it, same commit).
        hd.mount_yaw_off = -MOUNT_TANGENT_ALIGN * std::atan(sl_lat / p_spd);
        hd.mount_pitch   = -MOUNT_TANGENT_ALIGN * std::atan(sl_ver / p_spd);
        const float bank = MOUNT_BANK_GAIN * (sl_lat / p_spd);
        hd.mount_roll = (bank >  MOUNT_BANK_MAX) ?  MOUNT_BANK_MAX :
                        (bank < -MOUNT_BANK_MAX) ? -MOUNT_BANK_MAX : bank;

        // Seat lift along the FRAME's up (seat polish, ruled): the top
        // face tilts with roll/pitch (BNK-1); a world-vertical lift
        // sinks the feet by half·(1/cos tilt − 1) mid-swing. Rotate ŷ
        // through the same roll → pitch → yaw the GPU frames use —
        // closed form verified against the quaternion path to 1e-12.
        // Identity at a level frame: u = world up exactly.
        const float cr = std::cos(hd.mount_roll),  sr = std::sin(hd.mount_roll);
        const float cp = std::cos(hd.mount_pitch), sp = std::sin(hd.mount_pitch);
        const float uxl = -sp * cr;   // local up after roll, then pitch
        const float uyl =  cp * cr;
        const float uzl =  sr;
        const float thy = -hd.heading - hd.mount_yaw_off;
        const float cy = std::cos(thy), sy = std::sin(thy);
        const float ux =  uxl * cy + uzl * sy;   // base yaw into world
        const float uz = -uxl * sy + uzl * cy;
        const float half_t = ribbon.cube_size * 0.5f;

        hd.mount[0] = head_x + lat * (-sh) + RIBBON_MOUNT_SETBACK * ch + half_t * ux;
        hd.mount[1] = head_y + ver                                    + half_t * uyl;
        hd.mount[2] = head_z + lat * ( ch) + RIBBON_MOUNT_SETBACK * sh + half_t * uz;
    }

    // Record the head's state into the propagation history (catch-up
    // at fixed cadence; a frame hitch holds the current state across
    // the gap — the past never has holes).
    while (t - hd.hist_time >= RibbonHead::HIST_DT) {
        hd.hist_head = (hd.hist_head + 1u) % RibbonHead::HIST_CAP;
        hd.hist_heading[hd.hist_head] = hd.heading;
        hd.hist_y[hd.hist_head]       = hd.pos[1];
        hd.hist_time += RibbonHead::HIST_DT;
    }

    ribbon_rebuild_body_upload(rs, gpuState, queue, ribbon, head_x, head_y, head_z);
}

inline void ribbon_invalidate_head(RibbonState& rs) { rs.head.seeded = false; }

inline bool ribbon_head_is(const RibbonState& rs, uint32_t slot) {
    return rs.head.seeded && rs.head.slot == slot;
}

// Last computed ribbon head pose — the SADDLE (mount): pen + wave +
// setback. Read by the pawn mount and camera (sky mode).
inline void ribbon_head_pose(const RibbonState& rs, float& x, float& y, float& z, float& heading) {
    x = rs.head.mount[0];
    y = rs.head.mount[1];
    z = rs.head.mount[2];
    heading = rs.head.heading;
}

inline void ribbon_head_frame(const RibbonState& rs, float& yaw_off, float& pitch, float& roll) {
    yaw_off = rs.head.mount_yaw_off;
    pitch   = rs.head.mount_pitch;
    roll    = rs.head.mount_roll;
}

// The PEN — the true integrated head. Steering reads this; the
// mount above is the SADDLE (pen + wave + setback), for the pawn
// and camera only. Two consumers, two truths, never mixed.
inline void ribbon_head_pen(const RibbonState& rs, float& x, float& z, float& heading) {
    x = rs.head.pos[0];
    z = rs.head.pos[2];
    heading = rs.head.heading;
}

// ═══ FRAME ORCHESTRATION ═════════════════════════════════════════
//
inline void ribbon_frame_tick(RibbonState& rs, Cartridge* c, wgpu::Queue& queue) {

    // Phase clock on all CPU mirrors — MUSICAL TIME: the
    // sway integrates at the tempo follower's rate, scaled
    // so 100 BPM reproduces wall seconds exactly (the
    // calibration anchor). Held-last: transport stops, the
    // world keeps the pulse.
    const float phase_rate = c->time_state_.beat_rate
                           * (60.0f / RIBBON_REFERENCE_BPM);
    for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
        auto& par = rs.active[i];
        if (!par.active) continue;
        par.phase += phase_rate * c->time_state_.dt;
        rs.gpu[i].time = par.phase;
        {
            const VisualParams& vp = c->visual_canvas_.params();
            const float ml = c->ribbon_amp_lat_dst_.valid
                ? vp.get(c->ribbon_amp_lat_dst_.base)  : 1.0f;
            const float mv = c->ribbon_amp_vert_dst_.valid
                ? vp.get(c->ribbon_amp_vert_dst_.base) : 1.0f;
            rs.gpu[i].lateral_amp  = rs.active[i].spawn_lateral_amp  * ml;
            rs.gpu[i].vertical_amp = rs.active[i].spawn_vertical_amp * mv;

            // Line tint (color gen-2): gpu.color = lerp(spawn, stim, mix).
            // Rest = mix 0 = the seed-drawn color exactly; the held-slot
            // upload_ribbon_color line ships it unchanged.
            const float mix = c->ribbon_tint_mix_dst_.valid
                ? vp.get(c->ribbon_tint_mix_dst_.base) : 0.0f;
            const float* st = c->ribbon_tint_stim_dst_.valid
                ? vp.run(c->ribbon_tint_stim_dst_.base) : nullptr;
            for (int c2 = 0; c2 < 3; ++c2) {
                const float s = st ? st[c2] : 0.0f;
                rs.gpu[i].color[c2] =
                    par.spawn_color[c2]
                    + (s - par.spawn_color[c2]) * mix;
            }
        }
    }

    // Sky mode just ended — release the pinned (now anchor-less)
    // ribbon so a fresh one can spawn. SEAM[ribbon:sky-mode].
    if (c->player_.sky_mode_prev && !c->player_.sky_mode) {
        uint32_t s = rs.rendered_slot;
        if (s != UINT32_MAX && rs.active[s].active) {
            rs.active[s] = ActiveRibbon{};
            rs.gpu[s] = GPURibbonState{};
            if (rs.active_count > 0) rs.active_count--;
            GPURibbonState empty{};
            c->gpuState_.upload_ribbon(queue, empty);
            rs.rendered_slot = UINT32_MAX;
            // Successor ribbons reuse this slot — force re-init.
            ribbon_invalidate_head(rs);
        }
    }
    c->player_.sky_mode_prev = c->player_.sky_mode;

    // Render one ribbon: hold the current slot until it's evicted,
    // then pick the nearest active ribbon as the new rendered slot.
    bool current_alive = rs.rendered_slot != UINT32_MAX
        && rs.active[rs.rendered_slot].active;

    // Flight input for the head mover: the player when sky mode is
    // on; the wander policy when the rendered ribbon is a wanderer;
    // parked otherwise. One control law, many authors.
    // SEAM[ribbon:sky-mode].
    bool  ribbon_flown  = c->player_.sky_mode;
    float ribbon_yaw_in = ribbon_flown ?  c->inputState_.move_x : 0.0f;
    float ribbon_thr_in = ribbon_flown ? -c->inputState_.move_z : 0.0f;
    // The player's pen, eased like the wanderer's (RIBBON_SKY_YAW_TAU
    // in the tuning console).
    {
        if (c->player_.sky_mode) {
            const float a = 1.0f - std::exp(-c->time_state_.dt / RIBBON_SKY_YAW_TAU);
            c->player_.sky_yaw_eased += (ribbon_yaw_in - c->player_.sky_yaw_eased) * a;
            ribbon_yaw_in = c->player_.sky_yaw_eased;
        } else {
            c->player_.sky_yaw_eased = 0.0f;
        }
    }
    if (!ribbon_flown && current_alive
        && ribbon_head_is(rs, rs.rendered_slot)
        && rs.active[rs.rendered_slot].wander) {
        float whx, whz, whh;
        ribbon_head_pen(rs, whx, whz, whh);
        ribbon_wander_inputs(
            rs.active[rs.rendered_slot],
            whx, whz, whh, c->time_state_.dt,
            ribbon_yaw_in, ribbon_thr_in);
        ribbon_flown = true;
    }

    if (current_alive) {
        // Hold — update time + color. The gen-1 §5 coupling retired; the
        // gen-2 line tint (color_stim × color_mix over spawn; see
        // coupling_layer_migration_map.md) now computes the color per
        // frame in the flush loop above, and this upload ships that lerp
        // (rest = mix 0 = the seed-drawn color exactly).
        c->gpuState_.upload_ribbon_time(queue,
            rs.gpu[rs.rendered_slot].time);
        c->gpuState_.upload_ribbon_color(queue,
            rs.gpu[rs.rendered_slot].color);
        c->gpuState_.upload_ribbon_wave_amps(queue,
            rs.gpu[rs.rendered_slot].lateral_amp,
            rs.gpu[rs.rendered_slot].vertical_amp);
        // The head mover must run EVERY frame for the held ribbon,
        // not only on slot eviction. Without this the trail is seeded
        // straight once and never advances, so the ribbon looks
        // stationary despite is_roaming = 1. The mover's init guard
        // (slot unchanged) makes this a pure per-frame advance — no
        // re-seed, no double work with the eviction-branch call below.
        float rib_gnd;
        bool  rib_gnd_valid;
        {
            const auto& rb = rs.gpu[rs.rendered_slot];
            float gx = rb.anchor[0], gz = rb.anchor[2];
            if (ribbon_head_is(rs, rs.rendered_slot)) {
                float hy, hh; ribbon_head_pose(rs, gx, hy, gz, hh);
            }
            rib_gnd = estimate_terrain_height(c->tile_world_state_, gx, gz);
            rib_gnd_valid = terrain_tile_warm(c->tile_world_state_, gx, gz);
        }
        ribbon_advance_head(rs, c->gpuState_, queue,
            rs.gpu[rs.rendered_slot],
            rs.rendered_slot, c->time_state_.seconds,
            ribbon_flown, ribbon_yaw_in, ribbon_thr_in, c->time_state_.dt,
            rib_gnd, rib_gnd_valid);
    }
    else {
        // Current slot is gone — find nearest active ribbon
        uint32_t nearest = UINT32_MAX;
        float nearest_d2 = FLT_MAX;
        for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
            if (!rs.active[i].active) continue;
            float dx = rs.active[i].anchor_x - c->player_.readback_x;
            float dz = rs.active[i].anchor_z - c->player_.readback_z;
            float d2 = dx * dx + dz * dz;
            if (d2 < nearest_d2) { nearest = i; nearest_d2 = d2; }
        }

        if (nearest != UINT32_MAX) {
            c->gpuState_.upload_ribbon(queue, rs.gpu[nearest]);
            float rib_gnd;
            bool  rib_gnd_valid;
            {
                const auto& rb = rs.gpu[nearest];
                float gx = rb.anchor[0], gz = rb.anchor[2];
                if (ribbon_head_is(rs, nearest)) {
                    float hy, hh; ribbon_head_pose(rs, gx, hy, gz, hh);
                }
                rib_gnd = estimate_terrain_height(c->tile_world_state_, gx, gz);
                rib_gnd_valid = terrain_tile_warm(c->tile_world_state_, gx, gz);
            }
            ribbon_advance_head(rs, c->gpuState_, queue, rs.gpu[nearest], nearest, c->time_state_.seconds,
                ribbon_flown, ribbon_yaw_in, ribbon_thr_in, c->time_state_.dt,
                rib_gnd, rib_gnd_valid);  // 2b: head mover
            rs.rendered_slot = nearest;
        }
        else if (rs.rendered_slot != UINT32_MAX) {
            GPURibbonState empty{};
            c->gpuState_.upload_ribbon(queue, empty);
            rs.rendered_slot = UINT32_MAX;
        }
    }
}

// ═══ LIFECYCLE — three-phase + shared helper ═════════════════════

// ─── fill_ribbon_selection_geometry ───────────────────────────
// Shared geometry + color sampler used by both the dispatch
// pipeline and the mood forced-spawn path (see SEAM[ribbon:dual-entry]).
// Pure: no ribbon state access; all sampling is from `seed`.
inline void fill_ribbon_selection_geometry(
    uint32_t seed, uint32_t tier_idx,
    RibbonSelection& sel)
{
    const auto& tp = RIBBON_TIERS[tier_idx];

    float count_f = std::max(MIN_CUBE_COUNT,
        cpu_sample_gaussian(seed, RibbonProp::CUBE_COUNT, tp.cube_count_mean, tp.cube_count_sigma));
    sel.cube_count = std::min((uint32_t)count_f, Dim::RIBBON_MAX_RINGS);
    sel.cube_size = std::max(MIN_CUBE_SIZE,
        cpu_sample_gaussian(seed, RibbonProp::CUBE_SIZE, tp.cube_size_mean, tp.cube_size_sigma));

    // Length cap — keeps anchor coverage viable (700 u = 14 patches)
    if ((float)sel.cube_count * sel.cube_size > RIBBON_MAX_LENGTH)
        sel.cube_count = (uint32_t)(RIBBON_MAX_LENGTH / sel.cube_size);

    // The seed draw is this ribbon's CLEARANCE above its birthplace — pure
    // from seed; the ground joins once, at head init (ribbon_advance_head).
    sel.height = std::max(MIN_ADDED_HEIGHT,
        cpu_sample_gaussian(seed, RibbonProp::HEIGHT, tp.height_mean, tp.height_sigma));

    sel.orientation = cpu_hash_f(seed, RibbonProp::ORIENTATION) * 6.2831853f;

    sel.lateral_amp = std::max(0.1f, cpu_sample_gaussian(seed, RibbonProp::LATERAL_AMP, tp.lateral_amp_mean, tp.lateral_amp_sigma));
    sel.lateral_cycles = std::max(0.1f, cpu_sample_gaussian(seed, RibbonProp::LATERAL_CYCLES, tp.lateral_cycles_mean, tp.lateral_cycles_sigma));

    sel.vertical_amp = std::max(0.1f, cpu_sample_gaussian(seed, RibbonProp::VERTICAL_AMP, tp.vertical_amp_mean, tp.vertical_amp_sigma));

    // Color
    sel.color_mode = select_tier(seed, RibbonProp::COLOR_ROLL,
        RibbonColorMode::WEIGHTS, RibbonColorMode::COUNT);

    if (sel.color_mode == RibbonColorMode::SMOOTH) {
        uint32_t pal_idx = (uint32_t)(cpu_hash_f(seed, RibbonProp::PALETTE_IDX) * RIBBON_SMOOTH_PALETTE_COUNT);
        if (pal_idx >= RIBBON_SMOOTH_PALETTE_COUNT) pal_idx = RIBBON_SMOOTH_PALETTE_COUNT - 1;
        float var = cpu_hash_f(seed, RibbonProp::COLOR_R) * SMOOTH_VAR_RANGE + SMOOTH_VAR_BIAS;
        sel.color[0] = RIBBON_SMOOTH_PALETTE[pal_idx][0] + var;
        sel.color[1] = RIBBON_SMOOTH_PALETTE[pal_idx][1] + var * SMOOTH_VAR_G_SCALE;
        sel.color[2] = RIBBON_SMOOTH_PALETTE[pal_idx][2] + var * SMOOTH_VAR_B_SCALE;
    }
    else if (sel.color_mode == RibbonColorMode::TINTED) {
        sel.color[0] = cpu_hash_f(seed, RibbonProp::COLOR_R) * TINTED_RANGE[0] + TINTED_BASE[0];
        sel.color[1] = cpu_hash_f(seed, RibbonProp::COLOR_G) * TINTED_RANGE[1] + TINTED_BASE[1];
        sel.color[2] = cpu_hash_f(seed, RibbonProp::COLOR_B) * TINTED_RANGE[2] + TINTED_BASE[2];
    }
    else {
        if (cpu_hash_f(seed, RibbonProp::MEDIAN_SPECIES_ROLL) < CELLS_MEDIAN_CHANCE) {
            // MEDIAN-FIELD: one raffled median; color_b == color kills the
            // parity; texture (value + hue machinery) carries the cells.
            const auto lerpf = [](const float b[2], float t) {
                return b[0] + (b[1] - b[0]) * t; };
            const float ang = cpu_hash_f(seed, RibbonProp::MEDIAN_H) * 6.2831853f;
            const float ca = std::cos(ang), sa = std::sin(ang);
            const float luma   = lerpf(MEDIAN_LUMA,   cpu_hash_f(seed, RibbonProp::MEDIAN_L));
            const float chroma = lerpf(MEDIAN_CHROMA, cpu_hash_f(seed, RibbonProp::MEDIAN_C));
            for (int i = 0; i < 3; ++i) {
                sel.color[i] = luma + (CHROMA_D1[i]*ca + CHROMA_D2[i]*sa) * chroma;
                sel.color_b[i] = sel.color[i];
            }
            sel.checker_scatter = lerpf(MEDIAN_VALUE_VAR,
                cpu_hash_f(seed, RibbonProp::MEDIAN_VALUE_ROLL));
            sel.checker_hue_spread = lerpf(MEDIAN_HUE_VAR,
                cpu_hash_f(seed, RibbonProp::MEDIAN_HUE_ROLL)) * 3.14159265f;
        } else if (cpu_hash_f(seed, RibbonProp::FREE_MODE_ROLL) < FREE_PAIR_CHANCE) {
            // FREE RAFFLE — both medians as raffled (luma, chroma, hue)
            // points; both variances raffled. lerp helper inline.
            const auto lerpf = [](const float b[2], float t) {
                return b[0] + (b[1] - b[0]) * t; };
            const auto median = [&](float luma, float chroma, float ang,
                                    float out[3]) {
                const float ca = std::cos(ang), sa = std::sin(ang);
                for (int i = 0; i < 3; ++i)
                    out[i] = luma + (CHROMA_D1[i]*ca + CHROMA_D2[i]*sa) * chroma;
            };
            median(lerpf(FREE_DARK_LUMA,  cpu_hash_f(seed, RibbonProp::FREE_DARK_L)),
                   lerpf(FREE_DARK_CHROMA,cpu_hash_f(seed, RibbonProp::FREE_DARK_C)),
                   cpu_hash_f(seed, RibbonProp::FREE_DARK_H) * 6.2831853f,
                   sel.color);
            median(lerpf(FREE_LIGHT_LUMA,  cpu_hash_f(seed, RibbonProp::FREE_LIGHT_L)),
                   lerpf(FREE_LIGHT_CHROMA,cpu_hash_f(seed, RibbonProp::FREE_LIGHT_C)),
                   cpu_hash_f(seed, RibbonProp::FREE_LIGHT_H) * 6.2831853f,
                   sel.color_b);
            sel.checker_scatter = lerpf(FREE_VALUE_VAR,
                cpu_hash_f(seed, RibbonProp::FREE_VALUE_ROLL));
            sel.checker_hue_spread = lerpf(FREE_HUE_VAR,
                cpu_hash_f(seed, RibbonProp::FREE_HUE_ROLL)) * 3.14159265f;
        } else {
            // The pair raffle — cumulative-weight pick, the terrain's roll and
            // SMOOTH's pick, one mechanism.
            float w[CHECKER_PAIR_COUNT];
            for (uint32_t i = 0; i < CHECKER_PAIR_COUNT; ++i) w[i] = CHECKER_PAIRS[i].weight;
            uint32_t pick = select_tier(seed, RibbonProp::CHECKER_PAIR_ROLL, w, CHECKER_PAIR_COUNT);
            const CheckerPair& pr = CHECKER_PAIRS[pick];
            // One shared jitter moves both medians together: siblings differ,
            // the pair's designed contrast survives.
            const float jr = (cpu_hash_f(seed, RibbonProp::CHECKER_JIT_R) - 0.5f) * 2.0f * CHECKER_PAIR_JITTER;
            const float jg = (cpu_hash_f(seed, RibbonProp::CHECKER_JIT_G) - 0.5f) * 2.0f * CHECKER_PAIR_JITTER;
            const float jb = (cpu_hash_f(seed, RibbonProp::CHECKER_JIT_B) - 0.5f) * 2.0f * CHECKER_PAIR_JITTER;
            sel.color[0]   = pr.dark[0]  + jr;  sel.color_b[0] = pr.light[0] + jr;
            sel.color[1]   = pr.dark[1]  + jg;  sel.color_b[1] = pr.light[1] + jg;
            sel.color[2]   = pr.dark[2]  + jb;  sel.color_b[2] = pr.light[2] + jb;
            sel.checker_scatter = pr.value_var;
            {
                // hue_spread (radians, [0, pi]) = the pair's authored hue_var,
                // sibling-jittered per ribbon, scaled onto the shader's axis.
                const float sib = (cpu_hash_f(seed, RibbonProp::CHECKER_HUE_JITTER_ROLL) - 0.5f)
                                * 2.0f * CHECKER_HUE_SIBLING_JITTER;
                float hv = pr.hue_var + sib;
                hv = (hv < 0.0f) ? 0.0f : (hv > 1.0f) ? 1.0f : hv;
                sel.checker_hue_spread = hv * 3.14159265f;
            }
        }
    }

    sel.footprint_r = FOOTPRINT_RADIUS;
}

// ─── select_ribbon_for_patch ──────────────────────────────────
//
inline bool select_ribbon_for_patch(RibbonState& rs, Cartridge* c,
    int32_t gx, int32_t gz, RibbonSelection& sel) {
    // Tip-overlap idempotency: reject if ANY active ribbon's
    // near or far tip falls within this trigger patch.
    for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
        if (!rs.active[i].active) continue;
        if ((rs.active[i].near_tip_gx == gx && rs.active[i].near_tip_gz == gz) ||
            (rs.active[i].far_tip_gx == gx && rs.active[i].far_tip_gz == gz))
            return false;
    }
    auto gate = run_spawn_preamble(c, gx, gz,
        rs.active, MAX_RIBBON_INSTANCES,
        RibbonProp::SPAWN_ROLL, RibbonConfig::SPAWN_CHANCE,
        RibbonConfig::MOOD_MULTIPLIER,
        PopFamily::RIBBON, "ribn");
    if (!gate.ok) return false;

    // Tier selection with theme bias
    float tier_weights[RIBBON_TIER_COUNT];
    for (uint32_t t = 0; t < RIBBON_TIER_COUNT; t++)
        tier_weights[t] = RIBBON_BASE_TIER_WEIGHTS[t];
    for (uint32_t t = 0; t < RIBBON_TIER_COUNT; t++)
        tier_weights[t] *= THEMES[gate.theme_idx].tier_wt_ribbon[t];
    uint32_t tier_idx = select_tier(gate.seed, RibbonProp::TIER,
        tier_weights, RIBBON_TIER_COUNT);

    sel.seed = gate.seed;
    sel.trigger_gx = gx;
    sel.trigger_gz = gz;
    sel.slot = gate.slot;
    sel.tier_idx = tier_idx;

    fill_ribbon_selection_geometry(gate.seed, tier_idx, sel);

    {
        float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
        float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
        float away_angle = std::atan2(patch_cz - c->player_.readback_z,
            patch_cx - c->player_.readback_x);
        float hash_spread = cpu_hash_f(gate.seed, RibbonProp::ORIENTATION);
        sel.orientation = away_angle + (hash_spread * 2.0f - 1.0f) * ORIENTATION_SPREAD;
    }

    return true;
}

// ─── place_ribbon_from_selection ──────────────────────────────
//
inline bool place_ribbon_from_selection(Cartridge* c,
    const RibbonSelection& sel, RibbonPlacement& plan) {
    auto pos = negotiate_position(c, sel.seed,
        sel.trigger_gx, sel.trigger_gz,
        RibbonProp::ANCHOR_X, RibbonProp::ANCHOR_Z,
        RibbonConfig::POSITION_JITTER,
        RibbonProp::ORIENTATION,
        sel.footprint_r, PopFamily::RIBBON, sel.tier_idx);
    if (!pos.ok) return false;

    plan = RibbonPlacement{};
    plan.slot = sel.slot;
    plan.seed = sel.seed;
    plan.trigger_gx = sel.trigger_gx;
    plan.trigger_gz = sel.trigger_gz;
    plan.host_gx = pos.host_gx;
    plan.host_gz = pos.host_gz;
    plan.tier_idx = sel.tier_idx;
    plan.cx = pos.cx;
    plan.cz = pos.cz;

    plan.cube_count = sel.cube_count;
    plan.cube_size = sel.cube_size;
    plan.height = sel.height;
    plan.orientation = sel.orientation;
    plan.lateral_amp = sel.lateral_amp;
    plan.lateral_cycles = sel.lateral_cycles;
    plan.vertical_amp = sel.vertical_amp;
    plan.color_mode = sel.color_mode;
    std::memcpy(plan.color, sel.color, sizeof(plan.color));
    std::memcpy(plan.color_b, sel.color_b, sizeof(plan.color_b));
    plan.checker_scatter = sel.checker_scatter;
    plan.checker_hue_spread = sel.checker_hue_spread;

    record_placement_bookkeeping(PopFamily::RIBBON, plan.tier_idx);
    return true;
}

// ─── commit_ribbon ───────────────────────────────────────────
//
// Dual entry: also called from mood.inl::apply_mood for mood-5
// forced spawn (SEAM[ribbon:dual-entry]).
inline void commit_ribbon(RibbonState& rs, Cartridge* c,
    const RibbonPlacement& plan,
    int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue)
{
    GPURibbonState r{};
    r.anchor[0] = plan.cx;
    r.anchor[1] = 0.0f;
    r.anchor[2] = plan.cz;
    r.time = c->time_state_.seconds;
    r.cube_count = plan.cube_count;
    r.cube_size = plan.cube_size;
    r.height = plan.height;
    r.orientation = plan.orientation;
    {
        const float P = RIBBON_TIERS[plan.tier_idx].propagation_speed;
        const float total_length = (float)plan.cube_count * plan.cube_size;
        const float k = 6.2831853f * P / std::max(total_length, 1e-6f);
        r.propagation_speed = P;
        r.lateral_freq  = plan.lateral_cycles * k;
        r.vertical_freq = r.lateral_freq;
    }
    r.lateral_amp = plan.lateral_amp;
    r.vertical_amp = plan.vertical_amp;
    r.color_mode = plan.color_mode;
    r.color[0] = plan.color[0];
    r.color[1] = plan.color[1];
    r.color[2] = plan.color[2];
    r.color_b[0] = plan.color_b[0];
    r.color_b[1] = plan.color_b[1];
    r.color_b[2] = plan.color_b[2];
    r.checker_scatter = plan.checker_scatter;
    r.hue_spread = plan.checker_hue_spread;
    r.seed = plan.seed;
    r.is_visible = 1u;
    r.is_roaming = 1u;   // head-roaming on (player-flown or wandering; parked when neither)

    // Store in CPU mirror (per-frame nearest-selection uploads to GPU)
    uint32_t s = plan.slot;
    rs.gpu[s] = r;

    auto& ar = rs.active[s];
    // Snapshot the spawn color as the idle base — the home the line-tint
    // coupling (gen-2, T2) mixes over: gpu.color = lerp(spawn, stim, mix)
    // in the conductor's flush; mix rests 0 ⇒ this color exactly.
    ar.spawn_color[0] = r.color[0];
    ar.spawn_color[1] = r.color[1];
    ar.spawn_color[2] = r.color[2];
    ar.spawn_lateral_amp  = r.lateral_amp;
    ar.spawn_vertical_amp = r.vertical_amp;
    ar.phase = r.time;    // seed = wall clock, so the 100 BPM anchor reproduces the old clock exactly
    ar.patch_gx = trigger_gx;
    ar.patch_gz = trigger_gz;
    ar.host_gx = plan.host_gx;
    ar.host_gz = plan.host_gz;
    ar.anchor_x = plan.cx;
    ar.anchor_z = plan.cz;

    ar.wander = cpu_hash_f(plan.seed, RibbonProp::WANDER_ROLL) < WANDER_CHANCE;
    {
        float cr = cpu_sample_gaussian(plan.seed, RibbonProp::WANDER_CRUISE,
                                       WANDER_CRUISE_BASE, WANDER_CRUISE_SIGMA);
        ar.wander_cruise = (cr < WANDER_CRUISE_MIN) ? WANDER_CRUISE_MIN
                         : (cr > WANDER_CRUISE_MAX) ? WANDER_CRUISE_MAX : cr;
    }
    ar.wander_rng = 1u + (uint32_t)(cpu_hash_f(plan.seed, RibbonProp::WANDER_RNG)
                                    * 16777215.0f);
    ar.wander_tx = plan.cx - WANDER_HATCH_LEG * std::cos(r.orientation);
    ar.wander_tz = plan.cz - WANDER_HATCH_LEG * std::sin(r.orientation);
    ar.wander_retarget = WANDER_RETARGET_MIN;
    ar.wander_yaw_state = 0.0f;
    if (ribbon_head_is(rs, s))
        ribbon_invalidate_head(rs);

    // Two-tip anchoring: anchor IS the near tip (t=0).
    // Body extends entirely in the orientation direction (away from pawn).
    float total_length = (float)plan.cube_count * plan.cube_size;
    float dir_x = std::cos(plan.orientation);
    float dir_z = std::sin(plan.orientation);

    const float far_x = plan.cx + dir_x * total_length;
    const float far_z = plan.cz + dir_z * total_length;
    ar.near_tip_gx = (int32_t)std::floor(plan.cx / PATCH_EXTENT);
    ar.near_tip_gz = (int32_t)std::floor(plan.cz / PATCH_EXTENT);
    ar.far_tip_gx = (int32_t)std::floor(far_x / PATCH_EXTENT);
    ar.far_tip_gz = (int32_t)std::floor(far_z / PATCH_EXTENT);

    ar.near_tip_registered = false;
    ar.far_tip_registered = false;
    ar.ref_count = 0;

    ar.active = true;
    rs.active_count++;
    // SEAM[ribbon:L1] unconditional stdout — exhibition guard
    //   candidate. Same family as the [DIAG:*] stdout pattern
    //   noted across the codebase. Phase 1+ batch: wrap in
    //   #ifdef DIAG_RIBBON or similar before exhibition.
    std::cout << "[Ribbon] SPAWN slot=" << s << " at (" << plan.cx << ", " << plan.cz
        << ") tier=" << plan.tier_idx
        << " len=" << total_length
        << " near=(" << ar.near_tip_gx << "," << ar.near_tip_gz
        << ") far=(" << ar.far_tip_gx << "," << ar.far_tip_gz << ")\n";
}

// ═══ DISPATCH FUNNELS (table-shaped; declared in entity_types.hpp) ═

inline bool dispatch_select_ribbon(Cartridge* self,
    int32_t gx, int32_t gz, EntityQueueEntry& e) {
    return select_ribbon_for_patch(self->ribbon_state_, self, gx, gz, e.ribbon);
}

inline bool dispatch_place_ribbon(Cartridge* self,
    EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (place_ribbon_from_selection(self, e.ribbon, pe.ribbon)) {
        return true;
    }
    else {
        self->ribbon_state_.active[e.ribbon.slot].active = false;
        return false;
    }
}

inline void dispatch_commit_ribbon(Cartridge* self,
    PlacementEntry& pe, wgpu::Queue& queue) {
    // Commit the ribbon state (GPU mirror, active record, tip positions)
    commit_ribbon(self->ribbon_state_, self, pe.ribbon, pe.gx, pe.gz, queue);

    uint32_t slot = pe.ribbon.slot;
    auto& ar = self->ribbon_state_.active[slot];

    // Register with tip patches that currently exist.
    // Late registration handles the other tip when its patch is allocated.
    uint32_t refs = 0;
    auto* near_host = find_patch(self, ar.near_tip_gx, ar.near_tip_gz);
    if (near_host) {
        near_host->record_entity(PopFamily::RIBBON, slot);
        ar.near_tip_registered = true;
        refs++;
    }
    auto* far_host = find_patch(self, ar.far_tip_gx, ar.far_tip_gz);
    if (far_host && (ar.far_tip_gx != ar.near_tip_gx || ar.far_tip_gz != ar.near_tip_gz)) {
        far_host->record_entity(PopFamily::RIBBON, slot);
        ar.far_tip_registered = true;
        refs++;
    }

    if (refs == 0) {
        std::cout << "[Ribbon] REJECT slot=" << slot
            << " — no tip patches alive\n";
        ar = ActiveRibbon{};
        self->ribbon_state_.gpu[slot] = GPURibbonState{};
        self->ribbon_state_.active_count--;
        return;
    }
    ar.ref_count = refs;
}

// ═══ THE EVICTOR ══════════════════════════════════════════════════

inline void evict_ribbon(Cartridge* self,
    uint32_t slot, wgpu::Queue& queue) {
    auto& ar = self->ribbon_state_.active[slot];
    if (!ar.active) return;

    // Sky mode: the flown ribbon is pinned for the flight's duration.
    // Its anchor patches stream out as the player flies away, but the
    // ribbon must persist — skip eviction entirely while it is the
    // mounted, rendered ribbon. update() releases it on exit (the
    // sky_mode_prev edge). A rendered WANDERER is pinned the same way:
    // it drifts freely off its spawn patch, and with one slot the
    // world's ribbon persists — a contemplative object should.
    // SEAM[ribbon:sky-mode].
    if (slot == self->ribbon_state_.rendered_slot
        && (self->player_.sky_mode || ar.wander)) {
        return;
    }

    // Decrement ref count — one anchor patch has been evicted.
    // Only fully evict when all referencing patches are gone.
    if (ar.ref_count > 1) {
        ar.ref_count--;
        return;
    }

    // Final reference gone — full eviction
    ar = ActiveRibbon{};
    self->ribbon_state_.gpu[slot] = GPURibbonState{};
    self->ribbon_state_.active_count--;
    if (self->ribbon_state_.rendered_slot == slot) {
        GPURibbonState empty{};
        self->gpuState_.upload_ribbon(queue, empty);
        self->ribbon_state_.rendered_slot = UINT32_MAX;
        // Successor ribbons reuse this slot — force re-init.
        ribbon_invalidate_head(self->ribbon_state_);
    }
    std::cout << "[Ribbon] EVICT slot=" << slot << "\n";
}

} // namespace the_board
} // namespace t7
