#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/roster.hpp"   // struct Roster (the piece-enable manifest)

// ─── demo_config.hpp (CONTRACT: what a demo sentence declares) ─────
//
// A DEMO is a sentence in the roster's grammar: one piece-enable
// manifest + one world seed + one boot mood. v0 carries NOTHING else
// — design-table overrides are D3's future, pulled when the musician
// demos demand them, not pre-built.
//
// MATURITY DIAL (v0 -> the panel era): compile-time include (a demo
// is a header; selection is a define + rebuild) -> boot-time table
// (DemoConfig instances in a runtime registry — the roster's dial
// unparked) -> panel (live demo switching). The boot-time dial is
// PARKED with its puller named: live demo switching / the panel era.
//
// AXES THIS TYPE GROWS (D1-D5, the demo contract): D1 piece manifest
// (here), D2 world params (seed + boot mood here; radii/finiteness
// later), D3 design-table overrides, D4 coupling/canvas selection,
// D5 capture scripting. Each axis arrives with the demo that
// demands it.

namespace t7 {
namespace the_board {

struct DemoConfig {
    Roster   roster;      // D1 — which pieces exist
    uint32_t seed;        // D2 — the world master seed (WorldState boot)
    uint32_t boot_mood;   // D2 — the mood the world wakes in
};

} // namespace the_board
} // namespace t7
