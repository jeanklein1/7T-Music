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
//          dialed away: the curator's baseline.
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
        float    rest_density;    // exponential coefficient at gain 0
        float    rest_color[3];   // fog/sky RGB at gain 0
        float    gain;            // 0 manual … 1 coupling verbatim
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
};

// The authored design — the code panel. Values with a /* D1 */ marker
// are copied from the fog pipe's rest entries in PARAM_LAYOUT
// (src/coupling/visual_canvas.hpp); if the layout carries no explicit
// color rest, fall back to GPUDesignConfig's fog_color default
// initializer (state.hpp) and say which source fed it, here:
//   rest source: density — PARAM_LAYOUT's "fog.density" rest, which IS
//   FOG_DENSITY_NONE (visual_canvas.hpp), 0.0030f. Colour — the
//   FALLBACK: PARAM_LAYOUT's "fog.color" rest is one scalar (0.80f)
//   that the table's own comment calls "only the pre-first-tick
//   placeholder", so it is not an explicit color rest. The colour comes
//   from GPUDesignConfig's fog_color boot seed in state.hpp
//   (0.85/0.78/0.72), which is FOG_COLOR_NONE — the same anchor the
//   density wears, twinned by the boot config.
inline constexpr DriverSurface DRIVER_TABLE = {
    { /* D1 */ 0.0030f, { /* D1 */ 0.85f, 0.78f, 0.72f }, 1.0f },
    { 0u, 1.0f, 1.5f, 1.0f },   // attack/release: the authored values
                                // AURA_PRESENCE_ATTACK/RELEASE carried
                                // (bodies/pawn.hpp, retired by U5)
    { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 1.0f },   // checker: the rests are
                                // terrain_looks REST_CHECKER_* verbatim —
                                // a return to seed, not gray (ORGAN_3 w4)
};

// The live surface — the panel's fourth block and the seams' read.
inline DriverSurface DRIVER_LIVE = DRIVER_TABLE;

} // namespace the_board
} // namespace t7
