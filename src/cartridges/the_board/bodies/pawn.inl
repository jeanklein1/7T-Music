// ─── pawn.inl (IMPL: post-class definitions) ─────────────────────
// Impl of pawn.hpp (LADDER-2 c2): history in audit/LADDER.md.
//
// Definitions for pawn.hpp's declared laws — tick_pawn_couplings
// dereferences c->player_, c->time_state_, c->gpuState_.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at
// FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>  // std::min (aura config assembly)
#include <cmath>      // std::exp (the real-time presence ramp)
#include <iostream>   // command-door logs (m4)

namespace t7 {
namespace the_board {

// ─── Per-frame pawn coupling tick ────────────────────────────────
inline void tick_pawn_couplings(PawnState& ps, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    // Aura presence ramp: smooth 0→1 on enable / 1→0 on disable.
    // (aura_presence lives on player_ — SEAM[spine:P8]; see PlayerState, contracts/spine_state.hpp)
    {
        const float target = ps.aura_enabled ? 1.0f : 0.0f;
        const float prev   = c->player_.aura_presence;
        const float rate   = (target > prev) ? AURA_PRESENCE_ATTACK : AURA_PRESENCE_RELEASE;
        c->player_.aura_presence = prev + (target - prev) * (1.0f - std::exp(-rate * c->time_state_.dt));

        // Snap to endpoints to avoid perpetual drift
        if (c->player_.aura_presence < 0.001f && target == 0.0f) c->player_.aura_presence = 0.0f;
        if (c->player_.aura_presence > 0.999f && target == 1.0f) c->player_.aura_presence = 1.0f;
        if (c->player_.aura_presence != prev) ps.aura_cfg_dirty = true;
    }

    // Pawn aura height: presence × base height. Same value is consumed
    // by terrain VS for extrusion, so pawn and terrain always agree.
    const float effective_aura_height = ps.aura_height_enabled
        ? ps.active_aura_profile.height_scale * c->player_.aura_presence
        : 0.0f;
    c->gpuState_.set_pawn_aura_height(effective_aura_height);
    // Keep compute running while ramping down (so the trail decays cleanly).
    c->gpuState_.set_aura_enabled(c->player_.aura_presence > 0.001f);
}


// ─── Teardown (owner verb; REBUILD-0 m2, stamp D4) ────────────────
// The pawn half of the world-teardown sweep: schedule a one-frame
// aura clear + full config re-upload for the new world.
inline void teardown_pawn_aura(PawnState& ps) {
    ps.aura_needs_clear = true;
    ps.aura_cfg_dirty = true;
}

// ─── Aura compute (owner verb; REBUILD-0 m2 — stray (2) comes home) ─
// Persistent terrain influence. Runs while presence > 0 (ramping down
// after toggle-off) or clearing; no-op otherwise. DEFERRED-UPLOAD FLAG
// aura_cfg_dirty (O-4): full config upload on change, dt/t_beats-only
// in steady state.
inline void dispatch_pawn_aura(PawnState& ps, Cartridge* c,
                               wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
    if (!(c->player_.aura_presence > 0.0f || ps.aura_needs_clear)) return;
        if (ps.aura_cfg_dirty) {
            // Full config upload — profile changed or first frame
            ps.aura_cfg_dirty = false;
            const auto& ap = ps.active_aura_profile;

            // Presence scales all aura params for smooth raise/lower
            float p = c->player_.aura_presence;

            GPUPawnAuraConfig auraCfg{};
            auraCfg.cell_size = PATCH_CELL_SIZE;
            auraCfg.influence_radius = ap.influence_radius * p;
            auraCfg.attack_stiffness = ap.attack_stiffness;
            auraCfg.attack_damping = ap.attack_damping;
            auraCfg.release_rate = (p > 0.01f) ? ap.release_rate : 999.0f;
            auraCfg.dt = c->time_state_.dt;
            auraCfg.effect_mask = ap.effect_mask;
            auraCfg.aura_n = 64;
            auraCfg.tint_strength = std::min(ap.tint_strength * p, 1.0f);
            auraCfg.tint_r = ap.tint_r;
            auraCfg.tint_g = ap.tint_g;
            auraCfg.tint_b = ap.tint_b;
            auraCfg.delta_mode = ap.delta_mode;
            auraCfg.delta_magnitude = ap.delta_magnitude;
            auraCfg.t_beats = c->time_state_.beats;
            // height_scale gates the compute shader's R channel write (> 0.01 = enabled).
            // Actual terrain extrusion magnitude comes from config.pawn_aura_height in the VS.
            auraCfg.height_scale = (ps.aura_height_enabled && p > 0.01f) ? ap.height_scale : 0.0f;
            c->gpuState_.upload_pawn_aura_config(queue, auraCfg);
        }
        else {
            // Steady state — only dt and t_beats change per frame
            c->gpuState_.upload_pawn_aura_frame(queue, c->time_state_.dt, c->time_state_.beats);
        }

        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Pawn Aura";
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
        c->renderer_.dispatch_compute_pawn_aura(pass,
            c->gpuState_.pawn_aura_compute_group(),
            GPUState::pawn_aura_workgroups());
        pass.End();

        // After one cleanup frame with release_rate=999, all cells are zero
        if (ps.aura_needs_clear) { ps.aura_needs_clear = false; }
}


// ─── Player commands (owner verbs; REBUILD-0 m4 — the input fan's
// pawn pair, matching the orbs/agents/cube command pattern) ────────
inline void toggle_aura_height(PawnState& ps, Cartridge* c) {
    (void)c;
    ps.aura_height_enabled = !ps.aura_height_enabled;
    ps.aura_cfg_dirty = true;
    std::cout << "[Aura] Height extrusion: " << (ps.aura_height_enabled ? "ON" : "OFF") << "\n";
}

inline void toggle_aura(PawnState& ps, Cartridge* c) {
    (void)c;
    ps.aura_enabled = !ps.aura_enabled;
    ps.aura_cfg_dirty = true;
    std::cout << "[Aura] Field: " << (ps.aura_enabled ? "ON" : "OFF") << "\n";
}

// ─── Mood policy door (REBUILD-0 m4): respect player preference when
// permitted, force off when forbidden — the mood driver speaks through
// the pawn's own door instead of writing the organ. Semantics
// byte-identical to the direct write it replaces (disclosure rule).
inline void apply_aura_mood_policy(PawnState& ps, bool allow) {
    if (!allow) ps.aura_enabled = false;
}

} // namespace the_board
} // namespace t7
