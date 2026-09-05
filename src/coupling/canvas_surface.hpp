#pragma once
#include <cstdint>

// ─── coupling/canvas_surface.hpp ───────────────────────────────────
//
// THE CANVAS'S ENVELOPE AUTHORITIES, GRADUATED (ORGAN_3b P2, C2). The
// ledger priced this graduation and named its obstacle exactly: the
// enrollment macros spelled `offsetof(the_board::STRUCT, FIELD)` and the
// canvas lives one tier BELOW the cartridge, so the organ could not name
// it. P2 paid the price — one namespace parameter, invisible to every
// existing line — rather than inverting the layering to land dials.
//
// WHAT LIVES HERE AND WHAT DOES NOT. These are ENVELOPE AUTHORITIES:
// spans, cadences, ceilings and rates the couplings read while shaping a
// signal into a parameter. They are NOT rests — a pipe's rest lives in
// PARAM_LAYOUT, where the register map is, and a pipe's rest at the SEAM
// lives in the drivers' room. One fact, one home, three homes for three
// different facts.
//
// CANVAS_TABLE is the design; CANVAS_LIVE is what tick() reads. Seeded
// row for row, so behavior is byte-stable at boot and every value below
// is the one the module carried before this commit.
//
// A NESTED NAMESPACE, ON PURPOSE. `t7::canvas` gives the enrollment macro
// a clean namespace token (`ORGAN_PARAM_NS(canvas, CanvasSurface, …)`)
// without the organ having to name `t7::` from inside `t7::organ`, where
// unqualified lookup would find it but the macro's `NS::STRUCT` form
// needs something to write.
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace canvas {

struct CanvasSurface {
    // ── Fog ──────────────────────────────────────────────────────
    float fog_span;             // beats — the glide into a new field

    // ── The ribbon's sustain swell ───────────────────────────────
    float swell_ceiling;        // × idle (ruled)
    float swell_ramp;           // beats (ruled)
    float swell_attack;         // beats
    float swell_release;        // beats

    // ── The ribbon's pitch tint ──────────────────────────────────
    float pitch_vec_origin;     // radians — rotates the hue seating
    float tint_luma;            // the tint's value
    float tint_chroma;          // the tint's saturation
    float tint_mix_max;         // the ceiling on the tint's presence
    float tint_mix_attack;      // beats
    float tint_mix_release;     // beats
    float tint_hue_span;        // beats — the hue's own glide

    // (The checker field's cadence trio stood here — CHECKER-REBUILD →
    // INK_0, excised with its field.)

    // ── The cube choir's light ────────────────────────────────────
    // ACTIVATION AND DEACTIVATION ONLY — no held-length book is kept;
    // the envelope itself is the memory. The attack is the house's
    // standing exponential-approach idiom aimed at 1 (CUBE_GLIDE_TAU is
    // the same k-form; ZOETROPE_LIFT_TAU was too, until the CPU walk it
    // timed retired at WHEEL_0 U3): steepest at switch-on and
    // decelerating into the plateau, at τ = plateau/6 since CHOIR_1
    // (it was plateau/4 — the divisor is the SNAP, this row is the
    // LENGTH, and only the divisor moved). The commission calls that shape
    // LOGARITHMIC and it reads as one — but the curve is 1 − e^(−t/τ),
    // a saturating exponential whose INVERSE is the log, and the
    // difference is not pedantry: this one has an asymptote at 1 and a
    // true logarithm does not. The release is a FIXED SLOPE, not a
    // span: a dimmer key falls proportionally sooner than a bright one.
    float light_plateau;        // beats — attack plateau; τ = plateau/6 ⇒ I(plateau) ≈ 0.9975 (CHOIR_1: was /4, 0.982)
    float light_release;        // beats — FULL-SCALE linear fall to dark (the slope is 1/this)
};

// The authored design — values carried verbatim from the module's own
// tuning rows, which keep their banners and lose their numbers. One row
// has moved since: fog_span was tuned on the desk and transcribed back
// here at ship time, and says so.
inline constexpr CanvasSurface CANVAS_TABLE = {
    5.72f,     // fog_span — tuned on the desk; the module's own value was
               // 2.0, and 3.04 was one desk ago
    2.00f,     // swell_ceiling — × idle (ruled)
    8.0f,      // swell_ramp — beats (ruled)
    0.35f,     // swell_attack
    2.0f,      // swell_release
    0.0f,      // pitch_vec_origin
    0.55f,     // tint_luma
    0.35f,     // tint_chroma
    0.85f,     // tint_mix_max
    0.5f,      // tint_mix_attack
    3.0f,      // tint_mix_release
    2.0f,      // tint_hue_span
    8.0f,      // light_plateau — the choir's attack plateau (beats)
    8.0f,      // light_release — full brightness to dark in 8 beats
};

inline CanvasSurface CANVAS_LIVE = CANVAS_TABLE;
static_assert(sizeof(CanvasSurface) == 14 * sizeof(float),
    "CANVAS_LIVE is a whole-struct copy of the design row: a field added "
    "to one is added to the other by construction");

} // namespace canvas
} // namespace t7
