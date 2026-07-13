// ─── cube_behaviors.inl (IMPL: post-class definitions) ───────────
// Impl of cube_behaviors.hpp (LADDER-3 c3): history in audit/LADDER.md.
//
// Definitions for cube_behaviors.hpp's declared laws, plus the module-
// internal helpers (corral_ease_, apply_cube_behavior_override). The
// bodies reach c->gpuState_ / c->agent_state_ / c->player_ /
// c->time_state_.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at
// FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::cos, std::sin
#include <algorithm>  // std::clamp
#include <iostream>   // diagnostics feedback

namespace t7 {
namespace the_board {

// Apply tier gains to base substrate values. Called by cube_write_gpu
// during spawn — the result is what gets stored on the cube.
inline void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx) {
    if (tier_idx >= CUBE_TIER_COUNT) return;  // defensive; tier_idx is bounded at select
    const auto& g = CUBE_TIER_GAINS[tier_idx];
    spring_stiffness *= g.spring_stiffness_mult;
    drag             *= g.drag_mult;
}

//
inline uint32_t pick_cube_behavior_for_spawn(uint32_t mood_id, uint32_t seed) {
    if (mood_id >= MOOD_COUNT) return CUBE_BEHAVIOR_STATIONARY;
    const auto& pop = CUBE_POPULATIONS[mood_id];

    float total = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) total += pop.behavior_weights[i];
    if (total <= 0.0f) return CUBE_BEHAVIOR_STATIONARY;

    uint32_t h = cpu_hash(seed, 0xBEEF11A0u);
    float r = (float(h) / 4294967296.0f) * total;

    float acc = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) {
        acc += pop.behavior_weights[i];
        if (r < acc) return i;
    }
    // Numerical edge case (r == total exactly): return last non-zero.
    for (uint32_t i = CUBE_BEHAVIOR_COUNT; i > 0; i--) {
        if (pop.behavior_weights[i - 1] > 0.0f) return i - 1;
    }
    return CUBE_BEHAVIOR_STATIONARY;  // unreachable; total > 0 was checked
}

// ═══ DIAGNOSTICS ═════════════════════════════════════════════════
//
// ─── Coordination cycle ─────────────────────────────────────────────
//
// ─── Behavior override ──────────────────────────────────────────────
//
// ─── Corral animation ──────────────────────────────────────────────
//
// ─── Kite mode ─────────────────────────────────────────────────────
//
// Toggle on  — captures each cube's current xz as a pawn-relative
// offset. Cube xz is preserved exactly; y resets to pawn.y +
// orbit_height (small visible jump only if pawn's altitude differs
// from where the cube was hovering).

inline void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        cbs.activeCubes_[i] = ActiveCube{};
        GPUFloatingEntityState empty{};
        gpu.upload_cube_entity_slot(queue, i, empty);
    }
    cbs.activeCubeCount_ = 0;
}

inline float corral_ease_(float t) {
    return t * t * (3.0f - 2.0f * t);  // smoothstep
}

inline void cycle_floater_coordination(CubeBehaviorsState& cbs, Cartridge* c) {
    cbs.coordination_step = (cbs.coordination_step + 1) % 3;
    float v = FLOATER_COORDINATION_STEPS[cbs.coordination_step];
    c->gpuState_.config().floater_coordination = v;
    std::cout << "[Floaters] coordination: " << v << "\n";
}

inline void apply_cube_behavior_override(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        c->gpuState_.upload_cube_behavior_id(queue, i, cbs.behavior_override);
    }
}

inline void cycle_cube_behavior_override(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    cbs.behavior_override = (cbs.behavior_override + 1) % CUBE_BEHAVIOR_COUNT;
    apply_cube_behavior_override(cbs, c, queue);
    std::cout << "[Floaters] cube behavior: "
              << CUBE_BEHAVIOR_NAMES[cbs.behavior_override] << "\n";
}

inline void corral_cubes(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    const float px = c->agent_state_.slots[c->player_.possessed_slot].pos_x;
    const float pz = c->agent_state_.slots[c->player_.possessed_slot].pos_z;

    uint32_t active_count = 0;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (cbs.activeCubes_[i].active) active_count++;
    }
    if (active_count == 0) {
        std::cout << "[Floaters] corral: no active cubes\n";
        return;
    }

    uint32_t armed = 0;
    uint32_t k = 0;
    const float two_pi = 6.28318530718f;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        float theta = (float(k) / float(active_count)) * two_pi;
        float target_x, target_z;
        if (cbs.kite_mode) {
            target_x = std::cos(theta) * CUBE_CORRAL_RADIUS;
            target_z = std::sin(theta) * CUBE_CORRAL_RADIUS;
        } else {
            target_x = px + std::cos(theta) * CUBE_CORRAL_RADIUS;
            target_z = pz + std::sin(theta) * CUBE_CORRAL_RADIUS;
        }

        // Start position: current interpolated value if a previous
        // animation is in flight, else the CPU mirror.
        float from_x, from_z;
        if (cbs.corral_anim[i].active) {
            float t_norm = (c->time_state_.seconds - cbs.corral_anim[i].t0)
                           / cbs.corral_anim[i].duration;
            t_norm = std::clamp(t_norm, 0.0f, 1.0f);
            float eased = corral_ease_(t_norm);
            from_x = cbs.corral_anim[i].ax_from
                    + (cbs.corral_anim[i].ax_to - cbs.corral_anim[i].ax_from) * eased;
            from_z = cbs.corral_anim[i].az_from
                    + (cbs.corral_anim[i].az_to - cbs.corral_anim[i].az_from) * eased;
        } else if (cbs.kite_mode) {
            from_x = cbs.pawn_offset[i][0];
            from_z = cbs.pawn_offset[i][1];
        } else {
            from_x = cbs.activeCubes_[i].cx;
            from_z = cbs.activeCubes_[i].cz;
        }

        cbs.corral_anim[i] = {
            /*active=*/   true,
            /*t0=*/       c->time_state_.seconds,
            /*duration=*/ CUBE_CORRAL_DURATION,
            from_x, from_z,
            target_x, target_z,
        };
        if (cbs.kite_mode) {
            cbs.pawn_offset[i][0] = target_x;
            cbs.pawn_offset[i][1] = target_z;
        } else {
            cbs.activeCubes_[i].cx = target_x;
            cbs.activeCubes_[i].cz = target_z;
        }
        armed++;
        k++;
    }

    std::cout << "[Floaters] corral: " << armed << " cube(s) gliding "
              << (cbs.kite_mode ? "to ring offset around pawn" : "to ring")
              << " radius " << CUBE_CORRAL_RADIUS
              << " over " << CUBE_CORRAL_DURATION << "s\n";
}

inline void tick_cube_corral_animations(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        auto& anim = cbs.corral_anim[i];
        if (!anim.active) continue;
        if (!cbs.activeCubes_[i].active) { anim.active = false; continue; }

        float t_norm = (c->time_state_.seconds - anim.t0) / anim.duration;
        bool finishing = false;
        if (t_norm >= 1.0f) { t_norm = 1.0f; finishing = true; }
        if (t_norm < 0.0f) t_norm = 0.0f;
        float eased = corral_ease_(t_norm);
        float ax = anim.ax_from + (anim.ax_to - anim.ax_from) * eased;
        float az = anim.az_from + (anim.az_to - anim.az_from) * eased;
        if (cbs.kite_mode) {
            c->gpuState_.upload_cube_pawn_offset(queue, i, ax, 0.0f, az);
        } else {
            c->gpuState_.upload_cube_anchor(queue, i, ax, 0.0f, az);
        }
        if (finishing) anim.active = false;
    }
}

// ─── Kite mode toggle (F7) ──────────────────────────────────────

inline void toggle_cube_kite_mode(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    cbs.kite_mode = !cbs.kite_mode;
    const float px = c->agent_state_.slots[c->player_.possessed_slot].pos_x;
    const float pz = c->agent_state_.slots[c->player_.possessed_slot].pos_z;

    uint32_t affected = 0;
    if (cbs.kite_mode) {
        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
            if (!cbs.activeCubes_[i].active) continue;
            float ox = cbs.activeCubes_[i].cx - px;
            float oz = cbs.activeCubes_[i].cz - pz;
            cbs.pawn_offset[i][0] = ox;
            cbs.pawn_offset[i][1] = oz;
            c->gpuState_.upload_cube_pawn_offset(queue, i, ox, 0.0f, oz);
            c->gpuState_.upload_cube_follow_pawn(queue, i, 1u);
            affected++;
        }
    } else {
        // Toggle OFF: rather than write anchor on CPU (which can't see
        // the cube's drift, so anchor would land at home rather than
        // at the cube's visible position), we hand the work to the
        // kernel. follow_pawn = 2u is a "release pending" sentinel:
        // the next update_cube run sees it, sets anchor = current pos,
        // zeroes drift, switches follow_pawn back to 0. Visible
        // position is preserved exactly — no CPU drift estimation
        // needed, no readback latency.
        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
            if (!cbs.activeCubes_[i].active) continue;
            float ax = px + cbs.pawn_offset[i][0];
            float az = pz + cbs.pawn_offset[i][1];
            cbs.activeCubes_[i].cx = ax;
            cbs.activeCubes_[i].cz = az;
            c->gpuState_.upload_cube_follow_pawn(queue, i, 2u);
            affected++;
        }
    }

    std::cout << "[Floaters] kite mode: " << (cbs.kite_mode ? "ON" : "OFF")
              << " (" << affected << " cube(s))\n";
}

// ═══ THE EVICTOR ══════════════════════════════════════════════════

inline void evict_cube(Cartridge* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->cube_behaviors_state_.activeCubes_[slot].active = false;  // cube state owned by CubeBehaviorsState
    self->cube_behaviors_state_.activeCubeCount_--;
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_cube_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   cube slot=" << slot << "\n";
#endif
}

// ═══ THE CUBE RECIPE ══════════════════════════════════════════════
//
// Tier tables, traits, adapter, and dispatch funnels — beside the
// evictor. Funnels declared in cube_behaviors.hpp; table rows point
// here (family_dispatch.inl). THEMES is reached as THEMES
// (INTENT[services:themes] at its definition).

// ═══ FAMILY: CUBE ═════════════════════════════════════════════════

struct CubeIdx {
    static constexpr uint32_t BODY_RADIUS      = 0;
    static constexpr uint32_t ORBIT_HEIGHT     = 1;
    static constexpr uint32_t INFLUENCE_RADIUS = 2;
    static constexpr uint32_t SPIN_SPEED       = 3;
    static constexpr uint32_t BOB_AMPLITUDE    = 4;
    static constexpr uint32_t BOB_PERIOD       = 5;
    static constexpr uint32_t ASPECT_Y         = 6;
    static constexpr uint32_t ASPECT_Z         = 7;
    static constexpr uint32_t FACE_VARIANCE    = 8;
    static constexpr uint32_t COUNT            = 9;
};

inline constexpr TierParamDef CUBE_PARAM_DEFS[] = {
    { CubeEntityProp::BODY_RADIUS,      0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::ORBIT_HEIGHT,     3.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::INFLUENCE_RADIUS, 3.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::SPIN_SPEED,       0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::BOB_AMPLITUDE,    0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::BOB_PERIOD,       0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::ASPECT_Y,         0.2f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::ASPECT_Z,         0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeEntityProp::FACE_VARIANCE,    0.0f, 1e30f, false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t CUBE_PARAM_COUNT = sizeof(CUBE_PARAM_DEFS) / sizeof(TierParamDef);

// params[] order MUST match CUBE_PARAM_DEFS:
//   [0]BODY_RADIUS [1]ORBIT_HEIGHT [2]INFLUENCE_RADIUS [3]SPIN_SPEED
//   [4]BOB_AMPLITUDE [5]BOB_PERIOD [6]ASPECT_Y [7]ASPECT_Z [8]FACE_VARIANCE
struct CubeTierRow {
    TierProfile profile;
    float       spin_tilt_sigma;
};

inline constexpr CubeTierRow CUBE_TIERS[CUBE_TIER_COUNT] = {
    /* 0: SmallCube */ {
        { 0.40f, 0.0f, { {1.8f, 0.5f}, {25.0f, 20.0f}, {6.0f, 1.5f},  {0.04f, 0.015f},
                   {1.0f, 0.3f}, {5.0f, 1.5f},
                   {1.0f, 0.15f}, {1.0f, 0.15f}, {0.40f, 0.12f} }},
        0.12f
    },
    /* 1: MedCube   */ {
        { 0.32f, 0.0f, { {4.0f, 1.2f}, {45.0f, 30.0f}, {10.0f, 2.0f}, {0.03f, 0.01f},
                   {1.5f, 0.4f}, {6.0f, 2.0f},
                   {1.0f, 0.20f}, {1.0f, 0.20f}, {0.45f, 0.15f} }},
        0.10f
    },
    /* 2: LargeCube */ {
        { 0.20f, 0.0f, { {8.0f, 2.5f}, {75.0f, 45.0f}, {14.0f, 3.0f}, {0.02f, 0.008f},
                   {2.0f, 0.5f}, {8.0f, 2.5f},
                   {1.0f, 0.25f}, {1.0f, 0.25f}, {0.35f, 0.10f} }},
        0.08f
    },
    /* 3: Monolith  */ {
        { 0.08f, 0.0f, { {3.0f, 0.8f}, {12.0f, 8.0f}, {12.0f, 3.0f}, {0.015f, 0.005f},
                   {1.2f, 0.3f}, {6.0f, 2.0f},
                   {5.0f, 1.2f}, {0.15f, 0.03f}, {0.45f, 0.12f} }},
        0.10f
    },
};

inline const TierProfile& cube_get_tier_profile(uint32_t tier_idx) {
    return CUBE_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits CUBE_TRAITS = {
    PopFamily::CUBE, "cube", Dim::MAX_CUBE_INSTANCES,
    false, false, 0,
    true, 200.0f, 0.0f,
    CubeEntityProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
    CubeConfig::MOOD_MULTIPLIER, CubeConfig::POSITION_JITTER,
    CUBE_TIER_COUNT, CubeEntityProp::TIER,
    CUBE_PARAM_DEFS, CUBE_PARAM_COUNT,
    CubeEntityProp::ANCHOR_X, CubeEntityProp::ANCHOR_Z, CubeEntityProp::ROTATION, false,
    0, nullptr,
};

inline SpawnGateOutput cube_run_gate(Cartridge* c, int32_t gx, int32_t gz) {
    auto gate = run_spawn_preamble(c, gx, gz, c->cube_behaviors_state_.activeCubes_, Dim::MAX_CUBE_INSTANCES,
        CubeEntityProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
        CubeConfig::MOOD_MULTIPLIER, PopFamily::CUBE, "cube");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}
inline const float* cube_get_theme_tier_weights(uint32_t ti) { return THEMES[ti].tier_wt_cube; }

inline void cube_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    inst.solid_half = inst.params[CubeIdx::BODY_RADIUS];
    inst.ground_y_offset = 0.0f;
    inst.burial = 0.0f;
}

inline void cube_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    inst.colors[0] = cpu_hash_f(inst.seed, CubeEntityProp::COLOR_R) * 0.55f + 0.35f;
    inst.colors[1] = cpu_hash_f(inst.seed, CubeEntityProp::COLOR_G) * 0.50f + 0.30f;
    inst.colors[2] = cpu_hash_f(inst.seed, CubeEntityProp::COLOR_B) * 0.60f + 0.20f;
}

inline void cube_write_active(Cartridge* c, const EntityInstance& inst) {
    auto& ac = c->cube_behaviors_state_.activeCubes_[inst.slot];
    ac.patch_gx = inst.trigger_gx; ac.patch_gz = inst.trigger_gz;
    ac.host_gx = inst.host_gx; ac.host_gz = inst.host_gz;
    ac.cx = inst.cx; ac.cz = inst.cz;
    ac.last_alloc_time = c->time_state_.seconds;
    ac.active = true;
    c->cube_behaviors_state_.activeCubeCount_++;
}

inline void cube_write_gpu(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
    // Spin tilt: custom derivation from tier constant (not a sampled param)
    float tilt_sigma = CUBE_TIERS[inst.tier_idx].spin_tilt_sigma;
    float tilt_x = (cpu_hash_f(inst.seed, CubeEntityProp::SPIN_TILT_X) - 0.5f) * 2.0f * tilt_sigma;
    float tilt_z = (cpu_hash_f(inst.seed, CubeEntityProp::SPIN_TILT_Z) - 0.5f) * 2.0f * tilt_sigma;

    GPUFloatingEntityState fe{};
    fe.anchor[0] = inst.cx; fe.anchor[1] = 0.0f; fe.anchor[2] = inst.cz;
    fe.body_radius = inst.params[CubeIdx::BODY_RADIUS];
    fe.orbit_radius = 0.0f;
    fe.orbit_height = inst.params[CubeIdx::ORBIT_HEIGHT];
    fe.orbit_speed = 0.0f;
    fe.influence_radius = inst.params[CubeIdx::INFLUENCE_RADIUS];
    fe.spin_speed = inst.params[CubeIdx::SPIN_SPEED];
    fe.bob_amplitude = inst.params[CubeIdx::BOB_AMPLITUDE];
    fe.bob_period = inst.params[CubeIdx::BOB_PERIOD];
    fe.spin_tilt_x = tilt_x; fe.spin_tilt_z = tilt_z;
    fe.base_color[0] = inst.colors[0]; fe.base_color[1] = inst.colors[1]; fe.base_color[2] = inst.colors[2];
    fe.color[0] = inst.colors[0]; fe.color[1] = inst.colors[1]; fe.color[2] = inst.colors[2];
    fe.aspect_y = inst.params[CubeIdx::ASPECT_Y];
    fe.aspect_z = inst.params[CubeIdx::ASPECT_Z];
    fe.face_variance = inst.params[CubeIdx::FACE_VARIANCE];
    fe.geometry_type = 1; fe.motion_type = 1;
    fe.entity_seed = Dim::CUBE_SLOT_OFFSET + inst.slot;
    fe.t = 0.0f; fe.orientation[3] = 1.0f;
    fe.pos[0] = inst.cx; fe.pos[1] = fe.orbit_height; fe.pos[2] = inst.cz;
    fe.is_active = 1;
    //
    fe.spring_stiffness = CUBE_DEFAULT_SPRING_STIFFNESS;
    fe.drag             = CUBE_DEFAULT_DRAG;
    fe.tier_idx = inst.tier_idx;
    apply_cube_tier_gains(fe.spring_stiffness, fe.drag, inst.tier_idx);
    fe.drift[0] = 0.0f; fe.drift[1] = 0.0f; fe.drift[2] = 0.0f;
    fe.drift_vel[0] = 0.0f; fe.drift_vel[1] = 0.0f; fe.drift_vel[2] = 0.0f;
    fe.behavior_id    = pick_cube_behavior_for_spawn(c->mood_state_.active, inst.seed);
    fe.behavior_phase = cpu_hash(inst.seed, 0xF10A7E70u);
    // Kite mode starts disabled — cube is anchored to its spawn patch
    // until the user explicitly toggles kite mode via cube_behaviors.inl.
    fe.follow_pawn = 0;
    fe.pawn_offset[0] = 0.0f; fe.pawn_offset[1] = 0.0f; fe.pawn_offset[2] = 0.0f;
    c->gpuState_.upload_cube_entity_slot(queue, inst.slot, fe);
}

inline constexpr EntityFamilyAdapter CUBE_ADAPTER = {
    cube_run_gate, cube_get_theme_tier_weights,
    nullptr,                              // apply_indoor_rescale → not eligible (floaters, not grounded)
    cube_compute_solid_half, cube_compute_colors,
    cube_write_active, cube_write_gpu, nullptr,
    cube_get_tier_profile,
};

inline bool dispatch_select_cube_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, CUBE_TRAITS, CUBE_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::CUBE; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_cube_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, CUBE_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->cube_behaviors_state_.activeCubes_[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_cube_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) {
        generic_commit(self, CUBE_TRAITS, CUBE_ADAPTER, pe.generic, queue);
        // Lifecycle Phase 2: cube lifetime decoupled from host patch.
        // See dispatch_commit_sphere_generic for the rationale.
    }
    else { self->cube_behaviors_state_.activeCubes_[pe.generic.slot].active = false; }
}


// ─── Readback mirror reconciliation (owner verb; REBUILD-0 m2 —
// stray (1) comes home) ─ the cube half of the floater-readback funnel.
inline void reconcile_cube_mirror(CubeBehaviorsState& cs, Cartridge* c, const GPUFloatingEntityState* data) {
    float now = c->time_state_.seconds;
    // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
        // cube active-slot mirror owned by CubeBehaviorsState (cube_behaviors.hpp)
        if (cs.activeCubes_[i].active && !gpu_active &&
            (now - cs.activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
            cs.activeCubes_[i].active = false;
            if (cs.activeCubeCount_ > 0) cs.activeCubeCount_--;
        }
    }
}

} // namespace the_board
} // namespace t7
