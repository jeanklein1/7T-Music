#pragma once
#include "cartridges/the_board/contracts/demo_config.hpp"
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_OPEN_DEFAULT

// ─── full.hpp (DEMO SENTENCE: today's program) ─────────────────────
// Born at DEMO-1 (the config spine): history in audit/LADDER.md.
//
// THE GOLDEN TWIN: all bits as they stand at head, the current seed,
// the current boot mood. Building with demo=full must be
// byte-identical to head — the sentence machinery costs the current
// program nothing (G0' at the rig).

namespace t7 {
namespace the_board {

inline constexpr DemoConfig DEMO = {
    {
    // families, PopFamily order: pyr arch col ant palm cact blade sph rib cube gol gall
    true, true, true, true, true, true, true, true, true, true, true, true,
    // features: pawn_aura orbs spot_lights indoor_shell portal transitions wanderers
    true,      true, true,       true,        true,   true,       true,
    },
    /* seed */      42,
    /* boot_mood */ MOOD_OPEN_DEFAULT,
};

} // namespace the_board
} // namespace t7
