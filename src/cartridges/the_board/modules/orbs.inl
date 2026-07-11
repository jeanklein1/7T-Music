// ─── orbs.inl (IMPL: post-class definitions) ─────────────────────
//
// Definitions for orbs.hpp's declared laws, plus the module-internal
// helpers (GPU layout mapping, config packing, logging). Included AFTER
// the Cartridge class (LADDER-2 c3 header/impl split) so the keyhole is a
// complete type — the functions dereference c->gpuState_, c->renderer_,
// c->world_state_, c->player_, c->time_state_. The console, registries,
// OrbMoodConfig + ORB_MOOD_TABLE, OrbsState, and declarations live in
// orbs.hpp (file scope, above the class).
//
// WRAPPING FORM (the proven fix-2 rule): this file is SELF-WRAPPING — it
// opens t7::the_board itself and carries its own standard includes — so
// the MODULE IMPLEMENTATIONS zone includes it at FILE SCOPE, after the
// namespace closes. Definitions are `inline` free functions. Requires
// state.hpp (GPUOrbConfig, Dim::MAX_ORBS) and renderer.hpp earlier in the
// TU (both precede the class).
//
// ORB-1 (open ruling — anchor semantics): update_orb_anchor and
// toggle_orb_anchor moved VERBATIM. Move, don't improve.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::sqrt (rotation-axis normalization)
#include <iostream>   // operator feedback prints
#include <algorithm>  // std::min

namespace t7 {
namespace the_board {

// ═══ GPU LAYOUT HELPERS ══════════════════════════════════════════

// Map a tier index (0..3) to the first float of its 40-byte block
// inside a GPUOrbConfig instance. Matches the per-offset layout in
// state.hpp: tier0 starts at 192, stride 40.
inline float* orb_tier_block_ptr(GPUOrbConfig& cfg, uint32_t i) {
    auto* base = reinterpret_cast<char*>(&cfg);
    return reinterpret_cast<float*>(base + 192u + i * 40u);
}

// Map a tier index (0..3) to the first float of its 16-byte flocking-
// gains block. Flocking gains are stored in a parallel block after
// the main tier block: base offset 416, stride 16. Fields per tier:
// sep_gain, align_gain, coh_gain, pad.
inline float* orb_tier_flock_ptr(GPUOrbConfig& cfg, uint32_t i) {
    auto* base = reinterpret_cast<char*>(&cfg);
    return reinterpret_cast<float*>(base + 416u + i * 16u);
}


// ═══ CONFIGURE HELPERS ═══════════════════════════════════════════

// Apply mood's first-run defaults to player-owned state. Anchor and
// flock gesture are both "mood seeds once, player wins after."
inline void apply_mood_first_run_defaults_(OrbsState& os, const OrbMoodConfig& cfg) {
    if (!os.anchor_initialized) {
        os.pawn_anchored = cfg.anchor_to_pawn_default;
        os.anchor_initialized = true;
    }
    // Seed each rule's gesture index on first configure.
    // The mood carries one default (flock_gesture_default); we reuse
    // it for all three rules with per-rule count clamping. A future
    // pass can split this into per-rule mood defaults if wanted.
    if (!os.gesture_initialized[ORB_RULE_BROWNIAN]) {
        os.gesture_idx[ORB_RULE_BROWNIAN] = std::min(
            cfg.flock_gesture_default, ORB_BROWNIAN_GESTURE_COUNT - 1u);
        os.gesture_initialized[ORB_RULE_BROWNIAN] = true;
    }
    if (!os.gesture_initialized[ORB_RULE_ORBITAL]) {
        os.gesture_idx[ORB_RULE_ORBITAL] = std::min(
            cfg.flock_gesture_default, ORB_ORBITAL_GESTURE_COUNT - 1u);
        os.gesture_initialized[ORB_RULE_ORBITAL] = true;
    }
    if (!os.gesture_initialized[ORB_RULE_FLOCKING]) {
        os.gesture_idx[ORB_RULE_FLOCKING] = std::min(
            cfg.flock_gesture_default, ORB_FLOCK_GESTURE_COUNT - 1u);
        os.gesture_initialized[ORB_RULE_FLOCKING] = true;
    }
    // ORB_RULE_FROZEN has no gestures — index stays at 0, unread.
}

// Pack the active palette's per-entry HSV pockets into GPU config.
inline void pack_palette_(OrbsState& os, GPUOrbConfig& gpuCfg, uint32_t palette_id) {
    uint32_t pal_id = std::min(palette_id, ORB_PAL_COUNT - 1u);
    os.current_palette_id = pal_id;
    const auto& pal = ORB_PALETTES[pal_id];

    gpuCfg.palette_count = pal.count;
    gpuCfg.value_variance = pal.value_variance;
    gpuCfg.pal0_hue = pal.entries[0].hue;
    gpuCfg.pal0_hue_var = pal.entries[0].hue_var;
    gpuCfg.pal0_sat = pal.entries[0].saturation;
    gpuCfg.pal0_weight = pal.entries[0].weight;
    gpuCfg.pal1_hue = pal.entries[1].hue;
    gpuCfg.pal1_hue_var = pal.entries[1].hue_var;
    gpuCfg.pal1_sat = pal.entries[1].saturation;
    gpuCfg.pal1_weight = pal.entries[1].weight;
    gpuCfg.pal2_hue = pal.entries[2].hue;
    gpuCfg.pal2_hue_var = pal.entries[2].hue_var;
    gpuCfg.pal2_sat = pal.entries[2].saturation;
    gpuCfg.pal2_weight = pal.entries[2].weight;
    gpuCfg.pal3_hue = pal.entries[3].hue;
    gpuCfg.pal3_hue_var = pal.entries[3].hue_var;
    gpuCfg.pal3_sat = pal.entries[3].saturation;
    gpuCfg.pal3_weight = pal.entries[3].weight;
}

// Pack the selected tier set into the main tier block (with
// cumulative weights) AND the parallel flocking-gains block.
// Sentinel tierset_id → zero everything; orb_init falls back to
// legacy uniform population.
inline void pack_tiers_(GPUOrbConfig& gpuCfg, uint32_t tierset_id) {
    // Note: offsets 180-188 used to be _pad_tier0/1/2 here. They now
    // hold Brownian gesture fields (brownian_radial_sign/vert_bias/
    // coherence), written by pack_flocking_. Do NOT zero them here.

    if (tierset_id >= ORB_TIERSET_COUNT) {
        // Legacy path: zero main tier block + flocking gains.
        // Skip pf[3] for tier 0 — offset 428 is orbital_speed_var_mult
        // (repurposed), written later by pack_flocking_.
        gpuCfg.tier_count = 0;
        for (uint32_t i = 0; i < MAX_ORB_TIERS; i++) {
            float* p = orb_tier_block_ptr(gpuCfg, i);
            for (int k = 0; k < 10; k++) p[k] = 0.0f;
            float* pf = orb_tier_flock_ptr(gpuCfg, i);
            pf[0] = 1.0f; pf[1] = 1.0f; pf[2] = 1.0f;
            if (i != 0u) pf[3] = 0.0f;
        }
        return;
    }

    const auto& ts = ORB_TIERSETS[tierset_id];
    uint32_t n = std::min(ts.count, MAX_ORB_TIERS);
    gpuCfg.tier_count = n;

    // Deliberately distinct from seed_utils select_weighted: this BUILDS
    // the cumulative table the shader rolls against; it picks nothing.
    // Normalize weights → cumulative table so the shader can roll
    // a single uniform sample and bucket it into a tier.
    float wsum = 0.0f;
    for (uint32_t i = 0; i < n; i++) wsum += ts.tiers[i].weight;
    if (wsum < 1e-6f) wsum = 1.0f;  // pathological: avoid div-by-zero

    float cum = 0.0f;
    for (uint32_t i = 0; i < MAX_ORB_TIERS; i++) {
        float* p = orb_tier_block_ptr(gpuCfg, i);
        float* pf = orb_tier_flock_ptr(gpuCfg, i);

        if (i < n) {
            const auto& t = ts.tiers[i];
            cum += t.weight / wsum;
            if (i == n - 1) cum = 1.0f;   // clamp final bucket to avoid rounding drift
            p[0] = t.mass_mult;
            p[1] = t.drag_mult;
            p[2] = t.size_min;
            p[3] = t.size_max;
            p[4] = t.brightness_min;
            p[5] = t.brightness_max;
            p[6] = t.noise_gain;
            p[7] = t.force_gain;
            p[8] = t.color_gain;
            p[9] = cum;
            pf[0] = t.flock_sep_gain;
            pf[1] = t.flock_align_gain;
            pf[2] = t.flock_coh_gain;
            // pf[3] for tier 0 is orbital_speed_var_mult — don't touch.
            if (i != 0u) pf[3] = 0.0f;
        }
        else {
            // Unused tier slot: zero fields, cumulative = 1.0 so no roll lands here.
            for (int k = 0; k < 10; k++) p[k] = 0.0f;
            p[9] = 1.0f;
            pf[0] = 1.0f; pf[1] = 1.0f; pf[2] = 1.0f;
            if (i != 0u) pf[3] = 0.0f;
        }
    }
}

// Pack flocking params (mood-authored, sanitized), gesture signs
// (from current player-owned gesture index), and per-rule drag
// multipliers (sanitized with zero → pass-through).
inline void pack_flocking_(const OrbsState& os, GPUOrbConfig& gpuCfg,
    float sep_r, float align_r, float coh_r,
    float sep_w, float align_w, float coh_w,
    float max_speed,
    float rule_drag_bwn, float rule_drag_orb,
    float rule_drag_frz, float rule_drag_flk) {
    gpuCfg.flock_sep_radius = sep_r;
    gpuCfg.flock_align_radius = align_r;
    gpuCfg.flock_coh_radius = coh_r;
    gpuCfg.flock_sep_weight = sep_w;
    gpuCfg.flock_align_weight = align_w;
    gpuCfg.flock_coh_weight = coh_w;
    gpuCfg.flock_max_speed = max_speed;
    gpuCfg.flock_coupling_intensity = 0.0f;

    // Pack all three rule gesture bundles. Each rule reads
    // its own slice in the dynamics kernel; writing all three at
    // configure time keeps them in sync whatever rule becomes active.
    {
        const auto& gb = ORB_BROWNIAN_GESTURES[os.gesture_idx[ORB_RULE_BROWNIAN]];
        gpuCfg.brownian_radial_sign = gb.radial_sign;
        gpuCfg.brownian_vert_bias = gb.vert_bias;
        gpuCfg.brownian_coherence = gb.coherence;

        const auto& go = ORB_ORBITAL_GESTURES[os.gesture_idx[ORB_RULE_ORBITAL]];
        gpuCfg.orbital_alignment_mode = go.alignment_mode;
        gpuCfg.orbital_speed_var_mult = go.speed_var_mult;

        const auto& gf = ORB_FLOCK_GESTURES[os.gesture_idx[ORB_RULE_FLOCKING]];
        gpuCfg.flock_sep_sign = gf.sep_sign;
        gpuCfg.flock_align_sign = gf.align_sign;
        gpuCfg.flock_coh_sign = gf.coh_sign;
    }

    // Per-rule drag: zero → 1.0× pass-through (mood has no opinion).
    auto passthrough = [](float authored) {
        return (authored > 0.0f) ? authored : 1.0f;
        };
    gpuCfg.rule_drag_brownian = passthrough(rule_drag_bwn);
    gpuCfg.rule_drag_orbital = passthrough(rule_drag_orb);
    gpuCfg.rule_drag_frozen = passthrough(rule_drag_frz);
    gpuCfg.rule_drag_flocking = passthrough(rule_drag_flk);

    // Preserve the current smoothed speed multiplier across
    // mood transitions — use the in-flight value rather than a literal
    // 1.0 so a mood portal doesn't snap the sky back to baseline.
    gpuCfg.speed_mult = os.speed_mult_current;
}

// Log the effective config after sanitization. Shows what the GPU
// actually runs with (not what the mood authored), which is what
// the operator cares about when cycling rules or tuning.
inline void log_configure_(const OrbsState& os, const OrbMoodConfig& cfg,
    float eff_drag, float eff_orbital_speed,
    uint32_t palette_id) {
    static const char* RULE_NAMES[] = { "brownian", "orbital", "frozen", "flocking" };

    std::cout << "[Orbs] Configured: count=" << os.count
        << " palette=" << ORB_PAL_NAMES[palette_id]
        << " drag=" << eff_drag
        << " noise=" << ORB_NOISE_FLOOR
        << " rule=" << RULE_NAMES[std::min(os.current_motion_rule, 3u)]
        << " rot=" << cfg.rotation_speed
        << " orbital=" << eff_orbital_speed
        << " anchor=" << (os.pawn_anchored ? "pawn" : "origin")
        << " tiers="
        << (cfg.tierset_id < ORB_TIERSET_COUNT
            ? ORB_TIERSET_NAMES[cfg.tierset_id]
            : "legacy")
        << "\n";
}


// ═══ LIFECYCLE ═══════════════════════════════════════════════════

// Mood entry: sanitize rule-critical zeros against system defaults,
// apply first-run player defaults, pack the full GPU config, arm
// the init kernel. Called from apply_mood (mood.inl) and from the
// initial-mood setup in the init path (cartridge.hpp).
inline void configure_orbs(OrbsState& os, Cartridge* c, const OrbMoodConfig& cfg, wgpu::Queue& queue) {
    os.active = cfg.enabled;
    os.count = std::min(cfg.count, (uint32_t)Dim::MAX_ORBS);
    if (!os.active || os.count == 0) return;

    // Effective values: "zero = no opinion, use system default."
    auto eff = [](float authored, float fallback) {
        return (authored > 0.0f) ? authored : fallback;
        };
    const float eff_drag = eff(cfg.drag, ORB_DEFAULT_DRAG);
    const float eff_orbital_speed = eff(cfg.orbital_base_speed, ORB_DEFAULT_ORBITAL_SPEED);
    const float eff_flock_sep_r = eff(cfg.flock_sep_radius, ORB_DEFAULT_FLOCK_SEP_R);
    const float eff_flock_align_r = eff(cfg.flock_align_radius, ORB_DEFAULT_FLOCK_ALIGN_R);
    const float eff_flock_coh_r = eff(cfg.flock_coh_radius, ORB_DEFAULT_FLOCK_COH_R);
    const float eff_flock_sep_w = eff(cfg.flock_sep_weight, ORB_DEFAULT_FLOCK_SEP_W);
    const float eff_flock_align_w = eff(cfg.flock_align_weight, ORB_DEFAULT_FLOCK_ALIGN_W);
    const float eff_flock_coh_w = eff(cfg.flock_coh_weight, ORB_DEFAULT_FLOCK_COH_W);
    const float eff_flock_max_speed = eff(cfg.flock_max_speed, ORB_DEFAULT_FLOCK_MAX_SPEED);

    // First-run mood defaults for player-owned state.
    apply_mood_first_run_defaults_(os, cfg);
    // Motion rule is player-owned (like the flock gesture): seed once to
    // Brownian, then leave it — mood transitions no longer overwrite it.
    if (!os.motion_rule_initialized) {
        os.current_motion_rule = ORB_RULE_BROWNIAN;
        os.motion_rule_initialized = true;
    }

    // Normalize rotation axis on CPU (GPU renormalizes too but this
    // keeps uploaded values unit-length to avoid surprises).
    float rx = cfg.rotation_axis[0];
    float ry = cfg.rotation_axis[1];
    float rz = cfg.rotation_axis[2];
    float rlen = std::sqrt(rx * rx + ry * ry + rz * rz);
    if (rlen > 0.001f) { rx /= rlen; ry /= rlen; rz /= rlen; }
    else { rx = 0.0f; ry = 1.0f; rz = 0.0f; }

    // Build the GPU config in one place.
    GPUOrbConfig gpuCfg{};
    gpuCfg.count = os.count;
    gpuCfg.seed = c->world_state_.active_seed;
    gpuCfg.base_hue = cfg.base_hue;
    gpuCfg.hue_variance = cfg.hue_variance;
    gpuCfg.brightness = cfg.brightness;
    gpuCfg.drag = eff_drag;
    gpuCfg.noise_amp = ORB_NOISE_FLOOR;   // rests at the floor (driverless since the gen-1 retirement)
    gpuCfg.dome_radius = ORB_DOME_RADIUS;
    gpuCfg.base_size = ORB_BASE_SIZE;
    gpuCfg.dt = 0.0f;
    gpuCfg.t_seconds = 0.0f;
    gpuCfg.force_radial = 0.0f;
    gpuCfg.motion_rule = os.current_motion_rule;
    gpuCfg.rotation_speed = cfg.rotation_speed;
    gpuCfg.rotation_axis_x = rx;
    gpuCfg.rotation_axis_y = ry;
    gpuCfg.rotation_axis_z = rz;
    gpuCfg.orbital_base_speed = eff_orbital_speed;

    pack_palette_(os, gpuCfg, cfg.palette_id);

    // Color dynamics rest at zero — driverless capabilities since the
    // gen-1 retirement (gen-2 coupling targets; see
    // coupling_layer_migration_map.md). hue_converge_target is
    // mood-scoped (changes only at mood entry).
    gpuCfg.color_pulse = 0.0f;
    gpuCfg.color_converge = 0.0f;
    gpuCfg.color_surge = 0.0f;
    gpuCfg.hue_converge_target = cfg.hue_converge_target;

    // Dome center: start at origin; per-frame update_orb_anchor catches
    // up next frame with fresh pawnReadback values (not yet ready here).
    gpuCfg.dome_center_x = 0.0f;
    gpuCfg.dome_center_y = 0.0f;
    gpuCfg.dome_center_z = 0.0f;
    gpuCfg._pad_anchor = 0.0f;
    os.dome_center_initialized = false;   // force dirty-flag re-eval

    pack_tiers_(gpuCfg, cfg.tierset_id);
    pack_flocking_(os, gpuCfg,
        eff_flock_sep_r, eff_flock_align_r, eff_flock_coh_r,
        eff_flock_sep_w, eff_flock_align_w, eff_flock_coh_w,
        eff_flock_max_speed,
        cfg.rule_drag_brownian, cfg.rule_drag_orbital,
        cfg.rule_drag_frozen, cfg.rule_drag_flocking);

    c->gpuState_.upload_orb_config(queue, gpuCfg);
    os.init_pending = true;

    log_configure_(os, cfg, eff_drag, eff_orbital_speed, os.current_palette_id);
}

// Mood exit: stop dispatching. Resets mood-owned runtime state
// (speed multiplier, dome-center cache). Preserves player-owned
// state (anchor, flock gesture) across transitions.
inline void teardown_orbs(OrbsState& os, Cartridge* c) {
    (void)c;
    os.active = false;
    os.count = 0;
    os.init_pending = false;
    os.recolor_pending = false;

    // Speed multiplier resets with the mood (not player state).
    os.speed_mult_current = 1.0f;

    // Force fresh anchor upload on next configure (cache cleared, flag state preserved).
    os.last_dome_center_x = 0.0f;
    os.last_dome_center_z = 0.0f;
    os.dome_center_initialized = false;
}


// ═══ PLAYER COMMANDS ═════════════════════════════════════════════

// 0: cycle palette forward. Re-arms the recolor kernel so
// positions and colors both refresh on the next frame. Session-
// local within a mood — the next mood transition resets to that
// mood's configured palette.
inline void cycle_orb_palette(OrbsState& os, Cartridge* c, wgpu::Queue& queue) {
    if (!os.active || os.count == 0) {
        std::cout << "[Orbs] Palette cycle ignored (no active dome)\n";
        return;
    }

    os.current_palette_id = (os.current_palette_id + 1u) % ORB_PAL_COUNT;
    const auto& pal = ORB_PALETTES[os.current_palette_id];

    float pal_data[16];
    for (uint32_t i = 0; i < 4; i++) {
        pal_data[i * 4 + 0] = pal.entries[i].hue;
        pal_data[i * 4 + 1] = pal.entries[i].hue_var;
        pal_data[i * 4 + 2] = pal.entries[i].saturation;
        pal_data[i * 4 + 3] = pal.entries[i].weight;
    }
    c->gpuState_.upload_orb_palette(queue, pal.count, pal.value_variance, pal_data);

    // Color-only refresh: positions, velocities, twinkle phase all
    // persist so the sky "holds" and only the hues transition.
    os.recolor_pending = true;

    std::cout << "[Orbs] Palette: " << ORB_PAL_NAMES[os.current_palette_id] << "\n";
}

// KP_8: cycle motion rule (Brownian → Orbital → Frozen → Flocking → …).
// Does NOT reset orb state — positions and velocities carry over so
// the character shifts seamlessly into the new rule.
inline void cycle_orb_motion_rule(OrbsState& os, Cartridge* c, wgpu::Queue& queue) {
    if (!os.active || os.count == 0) {
        std::cout << "[Orbs] Motion rule cycle ignored (no active dome)\n";
        return;
    }

    os.current_motion_rule = (os.current_motion_rule + 1u) % 4u;
    c->gpuState_.upload_orb_motion_rule(queue, os.current_motion_rule);

    static const char* RULE_NAMES[] = { "brownian", "orbital", "frozen", "flocking" };
    std::cout << "[Orbs] Motion rule: " << RULE_NAMES[os.current_motion_rule];
    const uint32_t r = os.current_motion_rule;
    const uint32_t gidx = os.gesture_idx[r];
    if (r == ORB_RULE_BROWNIAN && gidx != 0u) {
        std::cout << " (gesture: " << ORB_BROWNIAN_GESTURES[gidx].name << ")";
    }
    else if (r == ORB_RULE_ORBITAL && gidx != 0u) {
        std::cout << " (gesture: " << ORB_ORBITAL_GESTURES[gidx].name << ")";
    }
    else if (r == ORB_RULE_FLOCKING && gidx != 0u) {
        std::cout << " (gesture: " << ORB_FLOCK_GESTURES[gidx].name << ")";
    }
    std::cout << "\n";
}

// KP_DECIMAL: cycle the gesture registry for the CURRENTLY ACTIVE
// motion rule. Brownian / Orbital / Flocking each have their own
// table; Frozen has none and short-circuits. Player state: each
// rule's index persists across mood transitions.
inline void cycle_orb_gesture(OrbsState& os, Cartridge* c, wgpu::Queue& queue) {
    const uint32_t r = os.current_motion_rule;

    if (r == ORB_RULE_BROWNIAN) {
        os.gesture_idx[r] = (os.gesture_idx[r] + 1u) % ORB_BROWNIAN_GESTURE_COUNT;
        const auto& g = ORB_BROWNIAN_GESTURES[os.gesture_idx[r]];
        c->gpuState_.upload_orb_brownian_gesture(queue,
            g.radial_sign, g.vert_bias, g.coherence);
        std::cout << "[Orbs] Brownian gesture: " << g.name << "\n";
        return;
    }
    if (r == ORB_RULE_ORBITAL) {
        os.gesture_idx[r] = (os.gesture_idx[r] + 1u) % ORB_ORBITAL_GESTURE_COUNT;
        const auto& g = ORB_ORBITAL_GESTURES[os.gesture_idx[r]];
        c->gpuState_.upload_orb_orbital_gesture(queue,
            g.alignment_mode, g.speed_var_mult);
        std::cout << "[Orbs] Orbital gesture: " << g.name << "\n";
        return;
    }
    if (r == ORB_RULE_FLOCKING) {
        os.gesture_idx[r] = (os.gesture_idx[r] + 1u) % ORB_FLOCK_GESTURE_COUNT;
        const auto& g = ORB_FLOCK_GESTURES[os.gesture_idx[r]];
        c->gpuState_.upload_orb_flock_signs(queue,
            g.sep_sign, g.align_sign, g.coh_sign);
        std::cout << "[Orbs] Flocking gesture: " << g.name
            << " (sep=" << (g.sep_sign > 0 ? "+" : "-")
            << " align=" << (g.align_sign > 0 ? "+" : "-")
            << " coh=" << (g.coh_sign > 0 ? "+" : "-")
            << ")\n";
        return;
    }

    // ORB_RULE_FROZEN: stillness is the rule's defining property.
    std::cout << "[Orbs] Frozen has no gestures (stillness is the rule).\n";
}

// KP_9: flip dome anchor between world origin and pawn-following.
// Player state: persists across mood transitions.
inline void toggle_orb_anchor(OrbsState& os, const Cartridge* c) {
    os.pawn_anchored = !os.pawn_anchored;
    std::cout << "[Orbs] Anchor: "
        << (os.pawn_anchored ? "ON — dome follows pawn"
            : "OFF — dome fixed at world origin")
        << "  (pawn readback: "
        << c->player_.readback_x << ", " << c->player_.readback_z << ")"
        << "\n";
}


// ═══ PER-FRAME UPDATES ═══════════════════════════════════════════

// Push the current dome center to the GPU. Dirty-flagged: a stationary
// anchored pawn or an unanchored session produces no per-frame traffic.
// Horizontal-only: Y is always 0 regardless of pawn altitude so the
// sky doesn't bob with terrain.
inline void update_orb_anchor(OrbsState& os, Cartridge* c, float pawn_x, float pawn_z, wgpu::Queue& queue) {
    if (!os.active || os.count == 0) return;

    float target_x = os.pawn_anchored ? pawn_x : 0.0f;
    float target_z = os.pawn_anchored ? pawn_z : 0.0f;

    bool changed = !os.dome_center_initialized
        || target_x != os.last_dome_center_x
        || target_z != os.last_dome_center_z;

    if (changed) {
        c->gpuState_.upload_orb_dome_center(queue, target_x, 0.0f, target_z);
        os.last_dome_center_x = target_x;
        os.last_dome_center_z = target_z;
        os.dome_center_initialized = true;
    }
}

// ═══ GPU DISPATCHES ══════════════════════════════════════════════

// One-shot seed-to-dome kernel. Fires once per configure_orbs when
// os.init_pending is armed.
inline void dispatch_orb_init(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder) {
    if (!os.init_pending) return;
    os.init_pending = false;

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Init";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (os.count + 63u) / 64u;
    c->renderer_.dispatch_orb_init(pass, c->gpuState_.orb_compute_group(), wgs);
    pass.End();

    std::cout << "[Orbs] Init dispatched: " << os.count
        << " orbs, " << wgs << " workgroups\n";
}

// Palette re-sample kernel. Fires on cycle_orb_palette — keeps
// positions/velocities/twinkle, only hues shift.
inline void dispatch_orb_recolor(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder) {
    if (!os.recolor_pending) return;
    os.recolor_pending = false;

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Recolor";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (os.count + 63u) / 64u;
    c->renderer_.dispatch_orb_recolor(pass, c->gpuState_.orb_compute_group(), wgs);
    pass.End();
}

// Snapshot orb_state → orb_state_prev. Dynamics reads the snapshot
// for neighbor queries so flocking invocations all see a stable
// previous-frame view. Cheap (N parallel copies) — runs every frame
// regardless of motion_rule so future rules can rely on prev.
inline void dispatch_orb_copy_prev(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder) {
    if (!os.active || os.count == 0) return;

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Copy Prev";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (os.count + 63u) / 64u;
    c->renderer_.dispatch_orb_copy_prev(pass, c->gpuState_.orb_copy_group(), wgs);
    pass.End();
}

// Per-frame rule + couplings. Uploads dt/t_seconds, then dispatches.
inline void dispatch_orb_dynamics(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder,
    wgpu::Queue& queue) {
    if (!os.active || os.count == 0) return;

    c->gpuState_.upload_orb_frame(queue, c->time_state_.dt, c->time_state_.seconds);

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Dynamics";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (os.count + 63u) / 64u;
    c->renderer_.dispatch_orb_dynamics(pass, c->gpuState_.orb_compute_group(), wgs);
    pass.End();
}

// ═══ RENDER ══════════════════════════════════════════════════════

// Additive billboard draw into the main render pass.
inline void render_orbs(OrbsState& os, Cartridge* c, wgpu::RenderPassEncoder& pass) {
    if (!os.active || os.count == 0) return;
    c->renderer_.draw_orbs(pass,
        c->gpuState_.render_entity_group(),
        c->gpuState_.render_texture_group(),
        c->gpuState_.orb_quad_vb(),
        c->gpuState_.orb_quad_ib(),
        os.count);
}

} // namespace the_board
} // namespace t7
