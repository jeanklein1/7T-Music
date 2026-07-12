#pragma once

// ─── demo.hpp (THE SELECTOR: which sentence builds) ────────────────
// Born at DEMO-1 (the config spine): history in audit/LADDER.md.
//
// DEMO SELECTION IS COMPILE-TIME in v0 (the design refinement on
// record): the incubator's idiom, parallel to INCUBATE_RENDER —
// pass INCUBATE_DEMO=<sentence> (a demos/ basename) as a define;
// default when undefined: full. One define and a rebuild is the
// production line's cadence, not a compromise. The boot-time dial
// stays parked with its puller named (live switching / the panel
// era — see contracts/demo_config.hpp's maturity dial).
//
// ROSTER (the selected sentence's manifest) is DEFINED here: the
// type and the gate law live in contracts/roster.hpp; the constant
// initializes from the selected DemoConfig, namespace-scope inline
// constexpr as before — the compiler still folds every gate, and
// the FIRST EDGE still refuses illegal sentences at compile time.

#ifndef INCUBATE_DEMO
#define INCUBATE_DEMO full
#endif

#define T7B_DEMO_STR2(x) #x
#define T7B_DEMO_STR(x) T7B_DEMO_STR2(x)
#define T7B_DEMO_HEADER(name) T7B_DEMO_STR(cartridges/the_board/demos/name.hpp)

#include T7B_DEMO_HEADER(INCUBATE_DEMO)

namespace t7 {
namespace the_board {

inline constexpr Roster ROSTER = DEMO.roster;

// THE FIRST EDGE — transitions REQUIRE portal: portals are both the
// trigger IN and the guaranteed return OUT; transitions on + portal
// off soft-locks. CONDITIONAL so transitions+portal both-off (the
// lean build) stays legal.
static_assert(!ROSTER.transitions || ROSTER.portal,
    "ROSTER: portal disabled while transitions enabled — "
    "transitions REQUIRE portal (the trigger in, the return path out)");

} // namespace the_board
} // namespace t7
