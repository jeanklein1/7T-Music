#pragma once
#include <cstdint>

// ─── mood_constants.hpp ──────────────────────────────────────────
// Born of the graduations (LADDER-2 prereq, LADDER-3 G1): history in audit/LADDER.md.
//
// The mood-count constant, the Mood IDs, and PortalDestination at file
// scope: the config-bearing module headers size their per-mood tables
// by MOOD_COUNT and name the IDs declaration-side; every reference is
// unqualified and resolves here.
//
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// How many moods. Canonical count for every per-mood table (MOOD_TABLE,
// ORB_MOOD_TABLE, AGENT_POPULATIONS, CUBE_POPULATIONS, the per-family
// MOOD_MULTIPLIER arrays). Indices are the Mood IDs below.
inline constexpr uint32_t MOOD_COUNT = 6;

// ─── Mood IDs ───────────────────────────────────────────────────
//
// Named indices into MOOD_TABLE / ORB_MOOD_TABLE / AGENT_POPULATIONS /
// CUBE_POPULATIONS and any per-mood multiplier array elsewhere in the
// codebase. The order is canonical — every per-mood table is written in
// this order, and the population tables carry per-row static_asserts
// that catch reordering.
//
inline constexpr uint32_t MOOD_OPEN_DEFAULT = 0;
inline constexpr uint32_t MOOD_OPEN_SUNSET = 1;
inline constexpr uint32_t MOOD_INDOOR_FLAT = 2;
inline constexpr uint32_t MOOD_INDOOR_VAULT = 3;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR = 4;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR_REF = 5;

// Portal destination — describes the world a door leads to. Also used
// as the pending transition target (keys + portal crossings). Shared
// mood/transition vocabulary; ActiveArch (entities.hpp) embeds one per
// arch.
struct PortalDestination {
    uint32_t seed = 0;
    bool finite = false;
    uint32_t finite_radius = 2;
    uint32_t mood = 0;               // 0=open, 1=finite (expandable)
};

} // namespace the_board
} // namespace t7
