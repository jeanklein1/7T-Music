#pragma once
#include <cstdint>

// ─── contracts/driver_surface.hpp ──────────────────────────────────
//
// THE DRIVERS' ROOM (ORGAN_2a). Rests and gains at the seams — the
// surface the ORGAN_2 ruling named and did not build. A driven
// parameter never wears a dial on its value; it wears dials on its
// DRIVER. This room holds those dials' homes and nothing else: no
// module facts live here, and no GPU block reads it. The seams read
// it once per tick.
//
//   rest — the value a seam holds when the driver's authority is
//          dialed away: the curator's baseline. (The fog's rest is the
//          MOOD's since ATMOS_1 — drawn per world from the atmosphere
//          into MoodState.fog_rest_*; the seam reads it there, and
//          this room keeps only the fog's gain.)
//   gain — the blend at the seam: out = rest + gain·(driven − rest).
//          Gain 1 is the coupling verbatim; gain 0 is full manual.
//   the aura intent — the presence ramp's rest TARGET (the ramp
//          drives presence toward it), moved home from
//          PawnState.aura_enabled: same fact, one home, four ABI
//          bytes. Its authors are unchanged in role: key 3's door,
//          the mood policy door (force-off), and the panel.
//
// Lineage: MOOD_TABLE → MOOD_LIVE (contracts/spine_state.hpp) — the
// authored table is the design, the inline instance is the live
// surface, and seeding is a copy at static init. Same pattern here.
// docs/ORGAN.md, "The drivers' room".
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct DriverSurface {
    struct Fog {
        float    gain;            // 0 manual … 1 coupling verbatim. The rest
                                  // is the mood's (ATMOS_1): out = mood rest
                                  // + gain · the canvas's deviation.
    } fog;
    struct Aura {
        uint32_t intent;          // the ramp's rest target: 0 off, 1 on
        float    attack;          // 1/s — presence rise rate
        float    release;         // 1/s — presence fall rate
        float    height_gain;     // × profile.height_scale at the tick
    } aura;
    // ORGAN_3 w4 — THE CHECKER FIELD'S SEAM. phase_motion_drivers flushes
    // the pitch-class colour field through set_checker_color_field every
    // frame; same shape as fog, same recipe. The rests are what the
    // TERRAIN_LOOKS panel calls law: "amount 0 (the GPU maps that to each
    // cell's seed color) and variance 0 — a return to seed, not gray."
    struct Checker {
        float rest_resultant[3];  // the music colour at gain 0
        float rest_amount;        // enveloped presence at gain 0
        float rest_variance;      // enveloped distinct-pc count at gain 0
        float gain;               // 0 manual … 1 coupling verbatim
    } checker;
    // ORGAN_4 P2 — THE RIBBON'S FOUR PIPES. ribbon_frame_tick reads four
    // already-wired C4 seams every frame, and their hardcoded fallbacks
    // ARE the rests: `: 1.0f`, `: 1.0f`, `: nullptr`, `: 0.0f`. Wave 4
    // held them back on the fourth, whose fallback is a NULL POINTER and
    // not a value — moving it into the room needed the null branch's rest
    // SHAPE read first. It is read now: downstream is
    // `const float s = st ? st[c2] : 0.0f`, so the shape is {0,0,0} —
    // exactly PARAM_LAYOUT's rest column for `ribbon.color_stim`, and the
    // same "return to seed, not gray" the checker's rests carry.
    //
    // ONE GAIN, THE SEAM'S VOLUME. The four pipes are one gesture — the
    // canvas moving a ribbon — so they share a gain, as fog's density and
    // colour do.
    struct Ribbon {
        float rest_amp_lat;       // 1.0 — identity: the seed's dance
        float rest_amp_vert;      // 1.0
        float rest_tint_stim[3];  // {0,0,0} — the null branch's shape, read
        float rest_tint_mix;      // 0.0
        float gain;               // one gain, the seam's volume
    } ribbon;
};

// The authored design — the code panel.
// The fog row carries the gain alone. Its rests came home to the mood at
// ATMOS_1, and to the REGIME at ATMOS_2 (Regime.fog_density /
// .fog_color, contracts/spine_state.hpp); the D1 lineage
// that sourced them is recorded there, beside ATMOS_SUNSET.
inline constexpr DriverSurface DRIVER_TABLE = {
    { 1.0f },                   // fog: gain 1, the coupling verbatim
    { 0u, 1.0f, 1.5f, 1.0f },   // attack/release: the authored values
                                // AURA_PRESENCE_ATTACK/RELEASE carried
                                // (bodies/pawn.hpp, retired by U5)
    { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 1.0f },   // checker: the rests are
                                // terrain_looks REST_CHECKER_* verbatim —
                                // a return to seed, not gray (ORGAN_3 w4)
    { 1.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f, 1.0f },   // ribbon: the four
                                // hardcoded fallbacks at ribbon.hpp's seam,
                                // carried verbatim — 1.0/1.0 are the
                                // amp identities (the seed's own dance),
                                // {0,0,0} is the null branch's shape as the
                                // downstream `st ? st[c2] : 0.0f` reads it,
                                // and mix 0 is the seed-drawn colour
                                // exactly (ORGAN_4 P2)
};

// The live surface — the panel's fourth block and the seams' read.
inline DriverSurface DRIVER_LIVE = DRIVER_TABLE;
static_assert(sizeof(DriverSurface) == 18 * sizeof(float),
    "DRIVER_LIVE is a whole-struct copy of the design row: a field added "
    "to one is added to the other by construction. 18 words — fog 1, "
    "aura 4, checker 6, ribbon 7 (88 -> 72 bytes at ATMOS_1)");

} // namespace the_board
} // namespace t7
