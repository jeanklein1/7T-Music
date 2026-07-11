#pragma once
#include <cstdint>

// ─── pawn.hpp (HEADER: state + configs + declarations) ───────────
//
// Player-relative state: aura field (toroidal 64×64 spring grid that
// activates near the pawn and releases when it moves away), presence
// trajectory, per-frame coupling tick.
//
// CONVERTED (LADDER-2 c2, header/impl split per Jean's ruling): this
// header owns the state STRUCT (PawnState) + the declarative laws
// (PawnAuraProfile / PAWN_AURA_DEFAULT / the ramp constants) + the
// function DECLARATION. It is included at file scope above the class, so
// the cartridge can declare the instance (pawn_state_) in its COMPOSITION
// ROOT chapter. The function DEFINITION dereferences the complete Cartridge
// (the keyhole) — c->player_, c->time_state_, c->gpuState_ — so it lives in
// pawn.inl, included AFTER the class where Cartridge is a complete type.
// Namespace t7::the_board.
//
// ┌─── Public surface (called from outside this file) ──────────────┐
// │  Per-frame updates:                                              │
// │    tick_pawn_couplings(ps, c, queue) — presence ramp + height   │
// │  Cross-module writers (set pawn_state_ flags from outside):     │
// │    input.inl  — toggles aura_enabled/aura_height_enabled; dirty │
// │    mood.inl   — clears aura_enabled when mood disallows aura     │
// │  Cross-module readers: cartridge.hpp aura compute dispatch      │
// └──────────────────────────────────────────────────────────────────┘
//
// Depends on: <cstdint>; forward declarations of Cartridge (keyhole, taken
// by pointer) and wgpu::Queue (taken by reference). The aura ramp is a
// self-contained real-time exponential (std::exp, in the .inl).
// ─────────────────────────────────────────────────────────────────

namespace wgpu { class Queue; }

namespace t7 {
namespace the_board {

class Cartridge;  // fwd — tick_pawn_couplings takes the keyhole Cartridge* (defined in pawn.inl, post-class)

// ═══ TUNING CONSOLE ══════════════════════════════════════════════
//
// System-level dials for the pawn aura. Profile-authored values
// (radius, stiffness, tints, mode) live in PawnAuraProfile below;
// these are dials that apply regardless of which profile is active.

// ── Aura presence ramp ───────────────────────────────────────────
// aura_presence ramps 0→1 on enable and 1→0 on disable. Smooths
// the transition so terrain extrusion and pawn height change
// gradually rather than snapping. Asymmetric: enable feels
// deliberate (~3s), disable feels release-y (~2s).
inline constexpr float AURA_PRESENCE_ATTACK  = 1.0f;   // 1/s — ~3s to full (spring converges in ~0.5s)
inline constexpr float AURA_PRESENCE_RELEASE = 1.5f;   // 1/s — ~2s to zero


struct PawnAuraDeltaMode {
    static constexpr uint32_t CONVERGENT = 0;  // all cells shift toward signature tint
    static constexpr uint32_t RANDOM = 1;  // each cell gets unique random delta
};

struct PawnAuraProfile {
    float influence_radius;
    float attack_stiffness;
    float attack_damping;
    float release_rate;
    float tint_strength;
    float tint_r, tint_g, tint_b;
    uint32_t delta_mode;
    float delta_magnitude;     // random mode: max offset per channel
    uint32_t effect_mask;      // bit 0=color, bit 1=height
    float height_scale;        // height extrusion in world units
};

inline constexpr PawnAuraProfile PAWN_AURA_DEFAULT = {
    20.0f,             // influence_radius
    12.0f,             // attack_stiffness
    0.7f,              // attack_damping
    1.5f,              // release_rate
    0.5f,              // tint_strength
    0.4f, 0.2f, 0.5f, // tint RGB (purple)
    PawnAuraDeltaMode::CONVERGENT,
    0.3f,              // delta_magnitude (used in random mode)
    0x3u,              // effect_mask: color tint + height
    3.0f,              // height_scale
};

// ═══ PAWN MODULE STATE (Scope B migration #5) ═══════════════════
//
// All pawn-owned state lives in this struct, accessed via pawn_state_
// on the Cartridge (declared at the composition root). Module functions
// take `PawnState& ps` explicitly rather than reaching via Cartridge*,
// making ownership language-visible and dependencies explicit in
// signatures.
//
// Field roles:
//   active_aura_profile — Currently active profile (PAWN_AURA_DEFAULT;
//     swappable by landmarks/commands).
//   aura_enabled — On/off intent (numpad 3, input.inl); the presence ramp
//     smooths the transition. Temporary binding; the function persists.
//   aura_height_enabled — Height-effect gate (key 2, input.inl); leaves the
//     color tint visible while flattening the extrusion.
//   aura_needs_clear — Internal: clear cells next frame after aura disable.
//   aura_cfg_dirty — Internal: full-config upload flag; true at boot; set by
//     external writers on a parameter shift.
//   (aura_presence — migrated to player_.aura_presence in SEAM[spine:P8];
//    read as c->player_.aura_presence; written in tick_pawn_couplings.)

struct PawnState {
    PawnAuraProfile active_aura_profile = PAWN_AURA_DEFAULT;
    bool            aura_enabled        = false;
    bool            aura_height_enabled = true;
    bool            aura_needs_clear    = false;
    bool            aura_cfg_dirty      = true;
};

// ─── Per-frame pawn coupling tick — DECLARATION ──────────────────
// Defined in pawn.inl (post-class), where the keyhole is complete.
// DONE[pawn:K1] aura presence ramp + height computation moved out of
//   cartridge.hpp::update() into this single named tick. The presence
//   value uses an inlined real-time exponential step (mirroring WGSL §1.2).
// Caller: cartridge.hpp::update() — once per frame after the signal is
// built and before world bounds are uploaded.
void tick_pawn_couplings(PawnState& ps, Cartridge* c, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
