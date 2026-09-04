#pragma once
#include <cstdint>

// ─── contracts/driver_surface.hpp — THE DRIVERS' ROOM ──────────────
// A driven parameter wears no dial on its value; it wears dials on its
// DRIVER. This room holds those dials' homes and nothing else: no module
// facts live here, no GPU block reads it, and the seams read it once per
// tick. (docs/ORGAN.md had a "The drivers' room" section once; it went
// at ORGAN_7 P5 with the persistence ladder's, and the ladder's home
// is docs/LAWS.md now. This file IS the drivers' room.)
//   rest — what a seam holds when the driver's authority is dialled away.
//          The fog's rest is the WORLD's (SkyState.fog_rest_*), so this
//          room keeps only the fog's gain.
//   gain — the blend: out = rest + gain·(driven − rest). 1 is the coupling
//          verbatim, 0 is full manual.
//   aura intent — the presence ramp's rest TARGET; key 3's door, the aura
//          policy door (force-off) and the panel write it.

namespace t7 {
namespace the_board {

struct DriverSurface {
    struct Fog {
        float    gain;            // 0 manual … 1 coupling verbatim
    } fog;
    struct Aura {
        uint32_t intent;          // the ramp's rest target: 0 off, 1 on
        float    attack;          // 1/s — presence rise rate
        float    release;         // 1/s — presence fall rate
        float    height_gain;     // × profile.height_scale at the tick
    } aura;
    // THE CHECKER FIELD'S SEAM. phase_motion_drivers flushes the
    // pitch-class colour field through set_checker_color_field every
    // frame; same shape as fog, same recipe. The rests are law: "amount 0
    // (the GPU maps that to each cell's seed color) and variance 0 — a
    // return to seed, not gray" (surface/terrain_looks.hpp).
    struct Checker {
        float rest_resultant[3];  // the music colour at gain 0
        float rest_amount;        // enveloped presence at gain 0
        float rest_variance;      // enveloped distinct-pc count at gain 0
        float gain;               // 0 manual … 1 coupling verbatim
    } checker;
    // THE RIBBON'S FOUR PIPES. ribbon_frame_tick reads four seams every
    // frame, and the rests are that seam's own fallbacks. color_stim's
    // fallback is a NULL POINTER: downstream is
    // `const float s = st ? st[c2] : 0.0f`, so its rest SHAPE is {0,0,0}.
    // ONE GAIN, THE SEAM'S VOLUME — the four pipes are one gesture, the
    // canvas moving a ribbon, so they share a gain as fog's terms do.
    struct Ribbon {
        float rest_amp_lat;       // 1.0 — identity: the seed's dance
        float rest_amp_vert;      // 1.0
        float rest_tint_stim[3];  // {0,0,0} — the null branch's shape
        float rest_tint_mix;      // 0.0
        float gain;               // one gain, the seam's volume
    } ribbon;
    // THE CUBE CHOIR'S SEAM. phase_motion_drivers reads the canvas's
    // "cube.light" run — one enveloped intensity per key — and mirrors
    // gain·I into the cube body's own state, where the projector
    // composes it. The REST here is DARK (I = 0), and dark is not a
    // colour: at I = 0 the projector returns each cube's SEED DRAW
    // bit-exactly, the checker's return-to-seed law wearing a different
    // subject. So the room keeps the incandescence the mix AIMS at, and
    // the gain — there is no rest triple to keep, because rest is the
    // absence of the mix, not a value of it.
    struct Cube {
        float light_color[3];     // the incandescence the mix aims at
        float gain;               // 0 manual … 1 coupling verbatim
    } cube;
    // THE GROUND'S VOICE. phase_motion_drivers reads two SOURCE pipes —
    // the room's held energy and its polyphonic density — and composes
    // them into the two multipliers config carries for the automaton:
    //   height_mul = clamp(1 + height_gain · energy,      0.25, 4.0)
    //   tick_mul   = clamp(1 / (1 + tick_gain · density), 0.25, 4.0)
    // TWO GAINS, NOT ONE, because these are two gestures and not one:
    // the room's energy LIFTS the ground, and its density QUICKENS it.
    // The fog and the ribbon share a gain because each is one gesture in
    // several lanes; this is the other case.
    //
    // NO REST TRIPLE HERE, and the reason is unusual enough to state.
    // The rest of a driven parameter normally lives somewhere else — the
    // world's for the fog, the terrain panel's for the checker. These
    // two rests live in the DRIVEN FIELDS THEMSELVES: config's
    // mode_gol_*_scale are boot-pinned to 1.0 and are still WRITABLE
    // organ dials. So gain 0 does not mean "compose against a rest", it
    // means HANDS OFF — the seam passes the dial's own value back, and
    // the dial is the author again. That is what "0 manual" has always
    // said; here it is literally true.
    struct Ground {
        float height_gain;        // × the room's energy, into the lift
        float tick_gain;          // × the room's density, into the period
    } ground;
};

// The authored design — the code panel. The fog row carries the gain
// alone: its rests live on the REGIME (Regime.fog_density / .fog_color,
// contracts/spine_state.hpp).
inline constexpr DriverSurface DRIVER_TABLE = {
    { 0.63f },                  // fog: the desk dialled the coupling back —
                                // 0.63 driven, the rest held at the regime's rest
    { 0u, 1.0f, 1.5f, 1.0f },   // aura: intent off, the authored rates.
                                // (It rested ON for one commit, d3b1f6d;
                                // the next desk export put it back off.)
    { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 1.0f },   // checker: a return to seed
    { 1.0f, 1.0f, { 0.0f, 0.0f, 0.0f }, 0.0f, 1.0f },   // ribbon: the seam's own fallbacks
    { { 1.0f, 0.92f, 0.72f }, 1.0f },   // cube: warm incandescent against
                                        // seed-cool bodies (Jean's desk)
    { 0.8f, 0.15f },                    // ground: the lift and the quickening
                                        // (Jean's desk — see the two notes below)
};

// THE TWO DESK NUMBERS, WITH WHAT THEY ACTUALLY DO — counted, because a
// gain whose effect saturates reads as a broken dial rather than a
// strong one.
//
// height_gain 0.8 against a field of 0..6 gives 1.0 / 1.8 / 2.6 / 3.4 /
// 4.2 / 5.0 / 5.8, and GROUND_SCALE_MAX is 4.0 — so fields 4, 5 and 6
// all clamp to the same ground and the top HALF of the field range is
// one height. That may be exactly right (a loud room is a loud room),
// but it is a choice and not an accident, so: **0.5 is the gain that
// maps the full field range onto the full clamp**, field 6 landing on
// 4.0 exactly. One token either way; Jean's desk.
//
// tick_gain 0.15 against voices 0..12 gives period × 1.0 / 0.87 / 0.77 /
// 0.69 / 0.63 / 0.53 / 0.45 / 0.36 — the whole useful range well clear
// of GROUND_SCALE_MIN, and monotone. This one wants no note.

// The live surface — the panel's fourth block and the seams' read.
inline DriverSurface DRIVER_LIVE = DRIVER_TABLE;
static_assert(sizeof(DriverSurface) == 24 * sizeof(float),
    "DRIVER_LIVE is a whole-struct copy of the design row: a field added "
    "to one is added to the other by construction. 24 words — fog 1, "
    "aura 4, checker 6, ribbon 7, cube 4, ground 2");

} // namespace the_board
} // namespace t7
