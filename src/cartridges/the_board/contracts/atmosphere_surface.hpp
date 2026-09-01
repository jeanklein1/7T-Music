#pragma once

// ─── atmosphere_surface.hpp ──────────────────────────────────────
// THE ATMOSPHERE PANEL — what the world wears, as one live bank.
//
// ONE_WORLD-II U1. The seven moods each carried an `Atmosphere`: a sun
// bearing plus FOUR regimes, of which the world wore one, chosen by a
// weighted roll under the seed. Every mood weighted regime 0 at 1.0 and
// the other three at 0 — read row by row off MOOD_TABLE, not inferred —
// so the array was four slots for one fact and the roll was a walk with
// one destination. Both leave; the DRAW stays.
//
// THE DRAW IS THE POINT. `draw_atmosphere` takes (seed, this bank) and
// returns one sky: every field here is a CENTRE, every `*_spread` a
// uniform ± around it, and a spread of 0 draws the centre EXACTLY —
// no hash, no trig round trip, the IEEE identity. That is what lets a
// point row stay bit-identical to the value it replaced, and it is why
// the bank keeps its spreads even while every one of them is 0 today.
// The panel writes the distribution, and the draw moves WITH the dial
// rather than re-rolling: the promise spine_state.hpp made before the
// banks existed, now true.
//
// ORGAN_3 SHAPE, the tree's standing form for a live bank:
//   ATMOS_TABLE  the DESIGN — authored, constexpr, two jobs only:
//                seeding the bank and standing under its asserts
//   ATMOS_LIVE   the BANK — what every runtime reader reads
// Enrollment landed at U6: the panel's twelve sky rows address ATMOS_LIVE
// itself, so a dial's write and the draw's read are the same memory.
//
// FLAT, AND NOT AN ARRAY. A family that wants regimes back subscribes
// the way the old comment described — its own columns, its own index —
// but nothing in the program does, and a four-slot array for a
// one-slot fact is the shape ONE_WORLD-II exists to remove.


namespace t7 {
namespace the_board {

// THE BANK. The sun's bearing, then the twelve fields the old `Regime`
// carried — centres and spreads, in the order they were authored in.
struct AtmosphereBank {
    // ─── The sun's bearing ────────────────────────────────────
    float sun_direction[3];      // the light vector's CENTRE — the direction light
                                 // travels; its readers normalize it
    float sun_az_spread_deg;     // ± azimuth turn about +Y, degrees (180 = any bearing)
    float sun_el_spread_deg;     // ± elevation, degrees; the draw clamps elevation to [5°, 88°]
    // ─── The sky ──────────────────────────────────────────────
    float sun_color[3];          // sun RGB — the centre
    float sun_color_spread;      // ± brightness
    float intensity;             // diffuse strength — the centre
    float intensity_spread;      // ± around it, uniform
    float ambient;               // ambient fill strength — the centre
    float ambient_spread;        // ± around it, uniform
    float fog_density;           // the REST the drivers' seam composes over — the centre
    float fog_density_spread;    // ± around it, uniform
    float fog_color[3];          // the rest colour — the centre
    float fog_color_spread;      // ± brightness
    float clear_color[3];        // sky RGB — the centre
    float clear_color_spread;    // ± brightness
};

// THE DESIGN. Transcribed from ATMOS_SUNSET's bearing and its regime 0 —
// the sunset is the look ONE_WORLD-II keeps, ruled, and it is a POINT
// row: every spread is 0, so the boot sky is the centre exactly.
//
// TRANSCRIBED, NOT DERIVED, ON PURPOSE. A `= MOOD_TABLE[...]` initializer
// would read well and then die with the table at U2, leaving the literals
// to be typed at the one moment nothing could check them. So they are
// typed HERE, while the source still stands, and pinned field by field by
// the witness below. The asserts do their whole job in this commit and
// leave with ATMOS_SUNSET; the numbers they proved stay.
inline constexpr AtmosphereBank ATMOS_TABLE = {
    { 0.94f, -0.29f, -0.13f }, 0.0f, 0.0f,
    { 1.0f, 0.75f, 0.45f }, 0.0f,          // sun colour, ±
    0.90f, 0.0f,                           // intensity, ±
    0.20f, 0.0f,                           // ambient, ±
    0.0030f, 0.0f,                         // fog density, ±
    { 0.85f, 0.78f, 0.72f }, 0.0f,         // fog colour, ±
    { 0.95f, 0.70f, 0.45f }, 0.0f,         // clear colour, ±
};

// THE SEEDING WITNESS IS SPENT (ONE_WORLD-II U2). Four asserts pinned
// every field of ATMOS_TABLE against ATMOS_SUNSET while that row still
// stood, and one injection (intensity 0.90 -> 0.91) proved they bit. The
// row left with the moods; the numbers it proved are the literals above,
// and the POINT-ROW witness below still guards the property that made
// them safe to transcribe.

// THE POINT-ROW WITNESS. The seeded bank draws its centres exactly —
// every spread 0 — which is what makes the boot sky bit-identical to the
// one the sunset mood drew. A spread authored here later is a design
// change, deliberate, and takes this assert with it on purpose.
static_assert(ATMOS_TABLE.sun_az_spread_deg == 0.0f
           && ATMOS_TABLE.sun_el_spread_deg == 0.0f
           && ATMOS_TABLE.sun_color_spread  == 0.0f
           && ATMOS_TABLE.intensity_spread  == 0.0f
           && ATMOS_TABLE.ambient_spread    == 0.0f
           && ATMOS_TABLE.fog_density_spread == 0.0f
           && ATMOS_TABLE.fog_color_spread   == 0.0f
           && ATMOS_TABLE.clear_color_spread == 0.0f,
    "the seeded atmosphere is a POINT row: draw_atmosphere short-circuits "
    "every axis and the boot sky is the centre exactly");

// THE BANK. Seeded whole — one struct, one copy, no per-row list to
// forget a member of.
inline AtmosphereBank ATMOS_LIVE = ATMOS_TABLE;

} // namespace the_board
} // namespace t7
