#pragma once
#include <cstdint>

// ─── contracts/pawn_surface.hpp ────────────────────────────────────
//
// THE PAWN'S AURA VOCABULARY, GRADUATED (ORGAN_3 w2, C2). The same
// move MoodProfile and AgentTierDef made, for the same reason: the
// organ needs to name the definition, and the organ may not include
// a body.
//
// PAWN_AURA_DEFAULT is the DESIGN — the authored profile, two jobs
// only: seeding PAWN_AURA_LIVE and standing under its assert.
// PAWN_AURA_LIVE is the live surface the aura tick and the aura
// dispatch read.
//
// WHY THE MODULE'S OWN COPY RETIRED. PawnState carried
// `active_aura_profile`, seeded from PAWN_AURA_DEFAULT and described
// as "swappable by landmarks/commands" — but the census found NO
// writer anywhere in the tree. It was a copy of a constant nothing
// changed, so a panel edit to the bank could never have reached it.
// The two readers now read the bank, and the field is gone: the same
// disposition PawnState.aura_enabled took at ORGAN_2a, for the same
// reason. If a landmark ever does swap profiles, it assigns the bank
// — one fact, one home.
//
// ORGAN_2a took the ramp's rates (intent/attack/release) and the
// height gain into the drivers' room, because those are the SEAM's
// dials. What stays here is the profile itself: the module's own
// facts, which is exactly the line ORGAN_2a's banner drew.
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

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
    uint32_t effect_mask;      // STATUS: INTENT — uploaded, never read.
                               // Bit 0=color / bit 1=height was the intent;
                               // no shader tests it (TUNE_1 A4 census). The
                               // live gates are tint_strength (color) and
                               // height_scale (height).
    float height_scale;        // height extrusion in world units
};

inline constexpr PawnAuraProfile PAWN_AURA_DEFAULT = {
    20.0f,             // influence_radius
    12.0f,             // attack_stiffness
    0.7f,              // attack_damping
    1.5f,              // release_rate
    0.0f,              // tint_strength — MUTED (TUNE_1 A4). The tint's
                       // only consumer is color_blend in the aura compute
                       // kernel (world.wgsl), so 0 silences the terrain
                       // tint outright. effect_mask bit 0 does NOT gate
                       // that write — the field is uploaded and never
                       // read — so the mask was not the dial. The height
                       // effect rides height_scale and is untouched.
    0.4f, 0.2f, 0.5f, // tint RGB (purple) — authored, kept
    PawnAuraDeltaMode::CONVERGENT,
    0.3f,              // delta_magnitude (used in random mode)
    0x3u,              // effect_mask: authored; unread (see the field at
                       // the struct, above — no shader tests this)
    3.0f,              // height_scale
};

// The live surface — the panel's block and the aura's read.
inline PawnAuraProfile PAWN_AURA_LIVE = PAWN_AURA_DEFAULT;
static_assert(sizeof(PawnAuraProfile) == sizeof(PAWN_AURA_DEFAULT),
    "PAWN_AURA_LIVE is a whole-struct copy of the design profile: a field "
    "added to one is added to the other by construction");

} // namespace the_board
} // namespace t7
