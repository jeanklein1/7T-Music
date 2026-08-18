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
// EXHIBIT TWO, NOT YET MOVED (carried in the inventory, untouched
// by FIELD_2): the ribbon's own consumption dials
// (RIBBON_FIELD_GAIN_XZ / _GAIN_Y / _LOOKAHEAD) and the frame-law
// mirrors (MOUNT_TANGENT_ALIGN / MOUNT_BANK_GAIN / MOUNT_BANK_MAX,
// ribbon.hpp:156-158). Single-homed today, so no L3 hazard — they
// join when the panel proper is designed.
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
// S rides config.floater_coordination (F5): 0 / 0.5 / 1.0 — the
// knob's first visible meaning in open_sunset. R0 sits outside the
// point's bubble (20). The runtime transport is already the
// authored table (the g2:5 uniform), so these are rests the writer
// reads, not a second wire.
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

// ═══ THE LIVE SURFACE (ORGAN_3 w2, C2) ═══════════════════════════
// The rests above are the DESIGN; PANEL_LIVE is what the writers
// read. Two families, one bank, because they share one home: this
// file is "THE PANEL — one home, every room", and a second contracts
// header per family would be two homes for one idea.
//
// THE BEACON'S S IS NOT HERE. The static_assert above proves
// FIELD_BEACON_S < FIELD_K at compile time, and a compile-time proof
// cannot guard a runtime dial: a panel that pushed S past field_k
// would clot the gather with the assert still reading true and no
// witness anywhere. S stays authored until the writer carries a
// clamp or the ruling is restated as a paired range. The other three
// beacon rests have no such partner and go live.
//
// THE CAMERA'S CONTROLS ARE CONTROLS, NOT POSE. Camera pose is GPU
// truth and has no section by ORGAN_3's rule; what lives here is the
// input grammar — how far a wheel notch zooms, how a keypress steps
// the look sensitivity and where that step is clamped.
struct PanelSurface {
    struct Beacon {
        float r0;      // inner radius — sits outside the point's bubble
        float r;       // outer radius
        float lift;    // height above the point
    } beacon;
    struct Camera {
        float look_sens_init;    // the clamp's anchor
        float look_sens_step;    // multiplicative, per keypress
        float look_sens_range;   // clamp half-width: init/R … init*R
        float scroll_zoom_scale; // orbit distance per wheel notch
    } camera;
};

inline constexpr PanelSurface PANEL_TABLE = {
    { FIELD_BEACON_R0, FIELD_BEACON_R, FIELD_BEACON_LIFT },
    { 0.005f, 1.25f, 8.0f, 2.0f },   // carried verbatim from
                                     // CameraControls (direction/input.hpp),
                                     // retired there by ORGAN_3 w2
};

inline PanelSurface PANEL_LIVE = PANEL_TABLE;
static_assert(PANEL_TABLE.beacon.r0 == FIELD_BEACON_R0
           && PANEL_TABLE.beacon.r  == FIELD_BEACON_R
           && PANEL_TABLE.beacon.lift == FIELD_BEACON_LIFT,
    "PANEL_TABLE's beacon row is seeded FROM the authored rests above — "
    "one fact, one home; if they can disagree the seeding is wrong");

} // namespace the_board
} // namespace t7
