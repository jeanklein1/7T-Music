#pragma once
#include <cstdint>

// ─── control_panel.hpp (CONTRACT: the master control panel) ──────
//
// THE PANEL IS THE ONE HOME. A dial authored here is read by every
// room that wears it — the CPU dialects directly, the GPU through
// the config uniform its rest value stages. Before the panel, a
// dial that lived in two rooms was kept in step by a LOCKSTEP
// MIRROR comment and by nothing else; Gate E is what that costs
// (the 2026-08 stamp edited the WGSL room live, the ribbon room
// froze at the pre-stamp values, and the ribbon went numb with
// every mirror comment still reading true). One home makes that
// failure structurally impossible, which is the panel's whole
// argument.
//
// PRECEDENT: the fog anchor (coupling/visual_canvas.hpp
// FOG_DENSITY_NONE / FOG_COLOR_NONE — "THE ANCHOR — one home for
// both rows that wear it"), twinned into the boot config. The
// panel generalizes that shape from one voice to the program.
//
// SCOPE, EXHIBIT ONE (FIELD_2a/2b): the field's dials. The ribbon
// dialect (bodies/ribbon.hpp) and the beacon writer (cartridge.hpp
// phase_motion_drivers) read these names directly; the WGSL room
// reads them as DesignConfig fields staged from these rests.
//
// EXHIBIT TWO IS CLOSED (RIBBON_1). It carried the ribbon's own
// consumption dials and the three frame-law mirrors it kept in lockstep
// with the shader, both "not yet moved". Neither is here to move: the
// consumption dials went with the CPU head's field reader, and the frame
// law is the body kernel's alone now, so the mirror has one half and
// nothing to drift from. The ribbon's live dials are
// contracts/ribbon_surface.hpp, boot-pinned into config.ribbon_*.
//
// RUNTIME TUNABILITY ARRIVED (ORGAN_3). The field's eight dials reach
// the panel through the config uniform they were already boot-pinned
// into — one ORGAN_PARAM line each, no bank needed. The two families
// that had NO transport get one below: PANEL_LIVE, the live surface
// for the beacon's rests and the camera's controls. The banner above
// predicted this sitting — "they join when the panel proper is
// designed" — and ORGAN_3 is the panel proper.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ THE FIELD ════════════════════════════════════════════════════
// The pair law's shape, shared by both dialects: the GPU field
// (world.wgsl field_pair / field_sum) and the ribbon head's CPU sum
// (bodies/ribbon.hpp). Jean's stamp, 2026-08.

inline constexpr float FIELD_SLACK = 3.0f;    // shell factor over summed radii
inline constexpr float FIELD_K     = 300.0f;  // accel per unit of quadratic shell depth
inline constexpr float FIELD_FMAX  = 600.0f;  // magnitude clamp on the summed force

// The mute switches — Jean's gate instrument, emitter side. Zeroing
// one silences its class of source exactly, in both dialects.
inline constexpr float FIELD_OCCUPIER_GAIN = 1.0f;  // standing geometry (shafts + arch legs)
inline constexpr float FIELD_AUTHORED_GAIN = 1.0f;  // the authored table (the beacon, the lure)

// The gate instrument's subscriber half — any class zeroes
// independently. Applied AFTER the FMAX clamp: the summed shape is
// bounded once, then scaled per class. These three were WGSL-only
// before the panel (no C++ twin existed to relocate in FIELD_2a), so
// the panel is their FIRST home, not their second — authored here
// from the values the deleted consts carried.
inline constexpr float FIELD_GAIN_CUBE   = 4.0f;
inline constexpr float FIELD_GAIN_SPHERE = 1.0f;
inline constexpr float FIELD_GAIN_AGENT  = 4.0f;

// ── The ribbon's per-family occupier dials ───────────────────────
// The head's CPU sum reads the three standing families from separate
// arrays (columns / antennas / arch legs), so it can weigh them
// independently — the GPU cannot without a slot split, since columns
// and antennas share agent_room.occupier_cmg. Ribbon-only: these do
// not touch what floaters and agents feel.
//
// ═══ THE BEACON (FIELD_4 — the first authored emitter) ════════════
// S rides config.floater_coordination: strength is s * coord, and
// coord == 0 turns the emitter off. Its one author is the
// Interaction · Cubes dial, 0…1 continuous. R0 sits outside the
// point's bubble (20). The runtime transport is already the authored
// table (the g2:5 uniform), so these are rests the writer reads, not
// a second wire.
inline constexpr float FIELD_BEACON_R0   = 25.0f;
inline constexpr float FIELD_BEACON_R    = 120.0f;
inline constexpr float FIELD_BEACON_S    = 200.0f;
inline constexpr float FIELD_BEACON_LIFT = 20.0f;

// THE RING SELF-SPACES, PROVED RATHER THAN CLAIMED. The beacon's
// pull at the ring must lose to the contact repulsion that spaces
// the gathered bodies; co-location is what lets the claim compile.
static_assert(FIELD_BEACON_S < FIELD_K,
    "the beacon's pull must lose to field repulsion at the ring — "
    "otherwise the gather clots instead of spacing (FIELD_4's ruling)");

// ORGAN_3b P5 — THE RULING, GIVEN A RUNTIME ANSWER. The assert above
// proves the AUTHORED pair and keeps proving it; a compile-time proof
// simply cannot speak for a dialed value. This ceiling is the panel's
// half: S enrolls with it as its max, so the dial cannot reach a value
// the ruling forbids at the authored k. The writer carries the other
// half — it clamps against config's LIVE field_k, because field_k is a
// dial too and lowering it is the second way to break the same ruling.
// One fact, stated where each half can act on it.
inline constexpr float FIELD_BEACON_S_MAX = FIELD_K - 1.0f;
static_assert(FIELD_BEACON_S <= FIELD_BEACON_S_MAX,
    "the authored S must be reachable by its own dial — if this fails "
    "the enrolled range would silently clamp the design's own value");

// ═══ THE LIVE SURFACE (ORGAN_3 w2, C2) ═══════════════════════════
// The rests above are the DESIGN; PANEL_LIVE is what the writers
// read. Two families, one bank, because they share one home: this
// file is "THE PANEL — one home, every room", and a second contracts
// header per family would be two homes for one idea.
//
// THE BEACON'S S IS HERE NOW (ORGAN_3b P5). ORGAN_3 left it out
// because a compile-time proof cannot guard a runtime dial, and named
// the two ways out: a clamp at the writer, or the ruling restated as
// a paired range. Jean stamped the clamp — it guards every future
// author, couplings included, and not merely the panel. So all four
// beacon rests are live, and the ruling is now proved twice: once for
// the authored pair (the assert), once for the dialed one (the
// ceiling above and the writer's clamp against the live k).
//
// THE CAMERA'S CONTROLS ARE CONTROLS, NOT POSE. Camera pose is GPU
// truth and has no section by ORGAN_3's rule; what lives here is the
// input grammar — how far a wheel notch zooms, how a keypress steps
// the look sensitivity and where that step is clamped.
//
// ORGAN_4 P3b — AND POSSESSION'S REACH JOINS THEM, for the reason this
// file's own banner gives: possession is input grammar, this file is
// "THE PANEL — one home, every room", and a second contracts header for
// one scalar would be two homes for one idea. It was an
// `inline constexpr` in bodies/agents.hpp, which the ORGAN may not
// include — so a dial on it was impossible until it had a contracts
// home. Its `_SQ` twin was a second constexpr derived at declaration,
// which is exactly the disagreement its DEFER row named: dial the radius
// and the square would keep the old word. The square is DERIVED AT THE
// READ now, from the live value, so the two cannot disagree.
struct PanelSurface {
    struct Beacon {
        float r0;      // inner radius — sits outside the point's bubble
        float r;       // outer radius
        float s;       // pull strength — ceiling'd below the live field k
        float lift;    // height above the point
    } beacon;
    struct Camera {
        float look_sens_init;    // the clamp's anchor
        float look_sens_step;    // multiplicative, per keypress
        float look_sens_range;   // clamp half-width: init/R … init*R
        float scroll_zoom_scale; // orbit distance per wheel notch
    } camera;
    struct Possession {
        float radius;            // how far the POINT reaches to take a body
    } possession;
};

// ═══ POSSESSION (ORGAN_4 P3b) ═════════════════════════════════════
// The authored reach, carried verbatim from bodies/agents.hpp's TUNING
// CONSOLE where it lived as `POSSESSION_RADIUS` beside a derived `_SQ`.
// The square does not come with it: a second constant is the whole
// defect, and the one read site squares the LIVE value instead.
inline constexpr float POSSESSION_RADIUS = 20.0f;

inline constexpr PanelSurface PANEL_TABLE = {
    { FIELD_BEACON_R0, FIELD_BEACON_R, FIELD_BEACON_S, FIELD_BEACON_LIFT },
    { 0.005f, 1.25f, 8.0f, 2.0f },   // carried verbatim from
                                     // CameraControls (direction/input.hpp);
                                     // retired there and its readers moved
                                     // here at ORGAN_3b — w2 built the bank
                                     // and left the readers behind, so these
                                     // four wrote a home nothing read
    { POSSESSION_RADIUS },           // ORGAN_4 P3b
};

inline PanelSurface PANEL_LIVE = PANEL_TABLE;
static_assert(PANEL_TABLE.beacon.r0 == FIELD_BEACON_R0
           && PANEL_TABLE.beacon.r  == FIELD_BEACON_R
           && PANEL_TABLE.beacon.s  == FIELD_BEACON_S
           && PANEL_TABLE.beacon.lift == FIELD_BEACON_LIFT
           && PANEL_TABLE.possession.radius == POSSESSION_RADIUS,
    "PANEL_TABLE's beacon and possession rows are seeded FROM the authored "
    "rests above — one fact, one home; if they can disagree the seeding is "
    "wrong");
static_assert(PANEL_TABLE.possession.radius > 0.0f,
    "the possession reach is squared at its read site; a zero or negative "
    "reach would make the search unwinnable rather than merely small — the "
    "enrollment line floors the dial above zero");

} // namespace the_board
} // namespace t7
