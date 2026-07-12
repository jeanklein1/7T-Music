// ─── pawn.inl (IMPL: post-class definitions) ─────────────────────
// Impl of pawn.hpp (LADDER-2 c2): history in audit/LADDER.md.
//
// Definitions for pawn.hpp's declared laws — tick_pawn_couplings
// dereferences c->player_, c->time_state_, c->gpuState_.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at
// FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <cmath>  // std::exp (the real-time presence ramp)

namespace t7 {
namespace the_board {

// ─── Per-frame pawn coupling tick ────────────────────────────────
inline void tick_pawn_couplings(PawnState& ps, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    // Aura presence ramp: smooth 0→1 on enable / 1→0 on disable.
    // (aura_presence lives on player_ — SEAM[spine:P8]; see Cartridge::PlayerState)
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

} // namespace the_board
} // namespace t7
