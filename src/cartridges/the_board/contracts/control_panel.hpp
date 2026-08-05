#pragma once
#include <cstdint>

// ─── control_panel.hpp (CONTRACT: the master control panel) ──────
// History: audit/FIELD_BRIDGE.md
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
// RUNTIME TUNABILITY IS NOT HERE YET. These are authored rests,
// compiled in; the dials become live when FIELD_5 / the panel
// proper give them a transport. Changing one is still a rebuild.
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
// and antennas share occupier_cmg. Ribbon-only: these do not touch
// what floaters and agents feel.
//
// CLASSICAL COLUMNS: MUTED (Jean's ruling). A column is a short thick
// post — PILLAR is r 1.8 at h 6.5 — and against the head's shell
// (slack 3.0) it deflected a body that flies well above the capital.
// Antennas and arch legs still speak: the tall thin masts are what a
// flying ribbon must actually thread, and the arch's legs are what
// keep its SPAN honest. Restore by raising this to 1.0.
inline constexpr float RIBBON_FIELD_COLUMN_GAIN  = 0.0f;
inline constexpr float RIBBON_FIELD_ANTENNA_GAIN = 1.0f;
inline constexpr float RIBBON_FIELD_ARCHLEG_GAIN = 1.0f;

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

} // namespace the_board
} // namespace t7
