#pragma once
#include "cartridges/the_board/contracts/demo_config.hpp"
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_OPEN_DEFAULT

// ─── minimal.hpp (DEMO SENTENCE: the degenerate proof) ─────────────
// Born at DEMO-1 (the config spine): history in audit/LADDER.md.
//
// The sentence that proves the grammar: every family bit OFF, every
// feature bit OFF. Surface + the playable character + the sun,
// nothing else. Legal by the FIRST EDGE's conditional form
// (transitions + portal both off — the lean build). M1 at the rig:
// boots sunlit, terrain streams and culls, the pawn walks and snaps,
// camera couples; NOTHING else ever appears; no portal can spawn, no
// transition can occur.

namespace t7 {
namespace the_board {

inline constexpr DemoConfig DEMO = {
    {
    // families, PopFamily order: pyr arch col ant palm cact blade sph rib cube gol gall
    false, false, false, false, false, false, false, false, false, false, false, false,
    // features: pawn_aura orbs spot_lights indoor_shell portal transitions wanderers
    false,     false, false,      false,       false,  false,      false,
    },
    /* seed */      42,
    /* boot_mood */ MOOD_OPEN_DEFAULT,
};

} // namespace the_board
} // namespace t7
