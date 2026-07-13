#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/keyhole.hpp"  // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── pawn.hpp (HEADER: state + configs + declarations) ───────────
// Converted (LADDER-2 c2): history in audit/LADDER.md.
//
// Player-relative state: aura field (toroidal 64×64 spring grid that
// activates near the pawn and releases when it moves away), presence
// trajectory, per-frame coupling tick.
//
// The aura ramp is a self-contained real-time exponential (std::exp, in the .inl).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

// ── Aura presence ramp ───────────────────────────────────────────
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

// ═══ PAWN MODULE STATE ═══════════════════════════════════════════
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
//   (aura_presence — lives at player_.aura_presence, SEAM[spine:P8];
//    read as c->player_.aura_presence; written in tick_pawn_couplings.)

struct PawnState {
    PawnAuraProfile active_aura_profile = PAWN_AURA_DEFAULT;
    bool            aura_enabled        = false;
    bool            aura_height_enabled = true;
    bool            aura_needs_clear    = false;
    bool            aura_cfg_dirty      = true;
};

// ─── Per-frame pawn coupling tick — DECLARATION ──────────────────
void tick_pawn_couplings(PawnState& ps, Cartridge* c, wgpu::Queue& queue);
void teardown_pawn_aura(PawnState& ps);
void dispatch_pawn_aura(PawnState& ps, Cartridge* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
