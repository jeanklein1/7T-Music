#pragma once
#include <cstdint>

// ─── mood_constants.hpp ──────────────────────────────────────────
// History: audit/LADDER.md
//
// The mood-count constant, the Mood IDs, and PortalDestination at file
// scope: the config-bearing module headers size their per-mood tables
// by MOOD_COUNT and name the IDs declaration-side; every reference is
// unqualified and resolves here.
//
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

inline constexpr uint32_t MOOD_COUNT = 6;

// ─── Mood IDs ───────────────────────────────────────────────────
//
inline constexpr uint32_t MOOD_OPEN_DEFAULT = 0;
inline constexpr uint32_t MOOD_OPEN_SUNSET = 1;
inline constexpr uint32_t MOOD_INDOOR_FLAT = 2;
inline constexpr uint32_t MOOD_INDOOR_VAULT = 3;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR = 4;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR_REF = 5;

struct PortalDestination {
    uint32_t seed = 0;
    bool finite = false;
    uint32_t finite_radius = 2;
    uint32_t mood = 0;               // destination mood id (MOOD_* above)
};

} // namespace the_board
} // namespace t7
