#pragma once
#include <cstdint>

// ─── mood_constants.hpp ──────────────────────────────────────────
//
// The mood-count constant, graduated to file scope (LADDER-2 prereq,
// per Jean's ruling 2026-07-11). It was a Cartridge static member
// (`static constexpr uint32_t MOOD_COUNT`); the config-bearing modules
// converting in this arc — entities, orbs, floater_vocabulary — size
// their per-mood tables by MOOD_COUNT, so a file-scope header cannot see
// an in-class constant. The standing law (readings publish at the second
// consumer) applies: MOOD_COUNT met its file-scope consumers and moves to
// a shared header, exactly as ROSTER did.
//
// Namespace t7::the_board (the cartridge's own); every existing MOOD_COUNT
// reference is unqualified, so it resolves here unchanged. The Mood IDs
// (MOOD_OPEN_DEFAULT … MOOD_FINITE_OUTDOOR_REF) stay in-class for now —
// their consumers (mood, cube_behaviors, input, agents) are all still
// class-body includes; they graduate if and when a file-scope consumer
// needs them.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// How many moods. Canonical count for every per-mood table (MOOD_TABLE,
// ORB_MOOD_TABLE, AGENT_POPULATIONS, CUBE_POPULATIONS, the per-family
// MOOD_MULTIPLIER arrays). Indices are the Mood IDs in cartridge.hpp.
inline constexpr uint32_t MOOD_COUNT = 6;

} // namespace the_board
} // namespace t7
