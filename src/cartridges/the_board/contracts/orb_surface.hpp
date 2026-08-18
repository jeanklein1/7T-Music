#pragma once
#include <cstdint>

// ─── contracts/orb_surface.hpp ─────────────────────────────────────
//
// THE ORB CONSOLE, GRADUATED (ORGAN_3 w2, C2). The three dials of
// bodies/orbs.hpp's TUNING CONSOLE that are read live rather than
// rolled at spawn: the dome the sky is painted on, the base size
// every orb scales from, and the noise floor the motion rests at.
//
// ORB_CONSOLE is the DESIGN — the authored row, two jobs only:
// seeding ORB_CONSOLE_LIVE and standing under its assert.
// ORB_CONSOLE_LIVE is what configure_orbs reads.
//
// The dome radius carries its own provenance: orbs.hpp's comment says
// "skybox radius — 700 fell into the fog; 500 is the visible dial
// (Jean's dial)". A constant that names itself Jean's dial and then
// cannot be turned is the exact condition this campaign exists to end.
//
// NOT HERE, deliberately: the nine ORB_DEFAULT_* fallbacks (every
// live path overrides them from the mood config — a dial on a default
// nothing reads would never be seen to move), the palettes, the tier
// sets and the three gesture registries (D5 tables, priced in the
// ledger), and ORB_MOOD_TABLE, whose disposition is a DEFINITION and
// not a bank (ORGAN_3 w3).
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct OrbConsole {
    float dome_radius;   // skybox radius the orb field is painted on
    float base_size;     // world units before per-tier scaling
    float noise_floor;   // motion noise amplitude at rest
};

// The authored design — values carried verbatim from bodies/orbs.hpp's
// TUNING CONSOLE, where they lived before ORGAN_3 w2.
inline constexpr OrbConsole ORB_CONSOLE = {
    500.0f,   // dome_radius — 700 fell into the fog; 500 is the visible dial (Jean's dial)
    3.0f,     // base_size
    0.3f,     // noise_floor — rests at the floor (driverless since the gen-1 retirement)
};

// The live surface — the panel's block and configure_orbs' read.
inline OrbConsole ORB_CONSOLE_LIVE = ORB_CONSOLE;
static_assert(sizeof(OrbConsole) == 3 * sizeof(float),
    "ORB_CONSOLE_LIVE is a whole-struct copy of the design row: a field "
    "added to one is added to the other by construction");

} // namespace the_board
} // namespace t7
