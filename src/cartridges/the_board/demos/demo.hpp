#pragma once
#include "cartridges/the_board/demos/matrix.hpp"   // THE MATRIX: Piece rows × DemoCol columns → demo_config

// ─── demo.hpp (THE SELECTOR: which column of the matrix builds) ────
// History: audit/LADDER.md
// The config spine; the per-demo headers retired into the MATRIX.
//
// DEMO SELECTION IS COMPILE-TIME in v0: pass INCUBATE_DEMO=<name> (a
// DemoCol enumerator — a column of the matrix) as a define; default
// when undefined: full. One define and a rebuild is the production
// line's cadence. The boot-time dial stays parked with its puller
// named (live switching / the panel era — see demo_config.hpp's
// maturity dial).
//
// THE SELECTION reduces to a token-paste: INCUBATE_DEMO=<name> resolves
// to DemoCol::<name>, and demo_config() reads that column into the
// DemoConfig. A bad name (INCUBATE_DEMO=xyz) becomes DemoCol::xyz — an
// unknown enumerator, a clean compile error. There are no per-demo
// header FILES any more; adding a demo is adding a column to the
// matrix (Jean's p2 stamp: the grid is the single authoring surface).
//
// ROSTER (the selected sentence's manifest) is DEFINED here: the type
// and the gate law live in contracts/roster.hpp; the constant
// initializes from the selected column's DemoConfig, namespace-scope
// inline constexpr as before — the compiler still folds every gate,
// and the FIRST EDGE still refuses illegal sentences at compile time.

#ifndef INCUBATE_DEMO
#define INCUBATE_DEMO full
#endif

#define T7B_DEMO_COL2(x) t7::the_board::DemoCol::x
#define T7B_DEMO_COL(x)  T7B_DEMO_COL2(x)

namespace t7 {
namespace the_board {

inline constexpr DemoConfig DEMO = demo_config( T7B_DEMO_COL(INCUBATE_DEMO) );

inline constexpr Roster ROSTER = DEMO.roster;

// THE FIRST EDGE — transitions REQUIRE portal: portals are both the
// trigger IN and the guaranteed return OUT; transitions on + portal
// off soft-locks. CONDITIONAL so transitions+portal both-off (the
// lean build) stays legal. Rides the folded ROSTER, so it fires
// identically whether the sentence came from the matrix or (once) a
// hand-written header — the guardrail the grid FEEDS but never
// reimplements.
static_assert(!ROSTER.transitions || ROSTER.portal,
    "ROSTER: portal disabled while transitions enabled — "
    "transitions REQUIRE portal (the trigger in, the return path out)");

} // namespace the_board
} // namespace t7
