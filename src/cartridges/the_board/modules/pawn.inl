// ─── pawn.inl (IMPL: post-class definitions) ─────────────────────
//
// Definitions for pawn.hpp's declared laws. Included AFTER the Cartridge
// class (LADDER-2 c2 header/impl split, per Jean) so the keyhole is a
// complete type — tick_pawn_couplings dereferences c->player_,
// c->time_state_, c->gpuState_. The state STRUCT + declarative laws live in
// pawn.hpp (file scope, above the class). Namespace t7::the_board.
// ─────────────────────────────────────────────────────────────────

#include <cmath>  // std::exp (the real-time presence ramp)

namespace t7 {
namespace the_board {

// ─── Per-frame pawn coupling tick ────────────────────────────────
void tick_pawn_couplings(PawnState& ps, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    // Aura presence ramp: smooth 0→1 on enable / 1→0 on disable.
    // (aura_presence migrated to player_ in SEAM[spine:P8] — see Cartridge::PlayerState)
    {
        const float target = ps.aura_enabled ? 1.0f : 0.0f;
        const float prev   = c->player_.aura_presence;
        const float rate   = (target > prev) ? AURA_PRESENCE_ATTACK : AURA_PRESENCE_RELEASE;
        // Real-time exponential BY RULING: a possession affordance, not a
        // musical gesture — the beat clock stays out of UI ramps. Shape
        // mirrors the GPU release primitive (world.wgsl §1.2); arithmetic
        // lifted verbatim from the retired COMPAT overload.
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

} // namespace the_board
} // namespace t7
