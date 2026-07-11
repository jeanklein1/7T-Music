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
// graduated too (LADDER-3 G1 — the file-scope consumers arrived; see the
// Mood IDs block below).
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
// GRADUATED here from the Cartridge class body (LADDER-3 G1, rides the
// agents rung): the stated condition of the earlier "stay in-class for
// now" note arrived — agents' AGENT_POPULATIONS and cube_behaviors'
// CUBE_POPULATIONS need the IDs DECLARATION-side in file-scope headers.
// All body-side consumers (input, mood, cartridge) resolve unqualified,
// unchanged.
inline constexpr uint32_t MOOD_OPEN_DEFAULT = 0;
inline constexpr uint32_t MOOD_OPEN_SUNSET = 1;
inline constexpr uint32_t MOOD_INDOOR_FLAT = 2;
inline constexpr uint32_t MOOD_INDOOR_VAULT = 3;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR = 4;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR_REF = 5;

// Portal destination — describes the world a door leads to. Also used as
// the pending transition target (keys + portal crossings). GRADUATED here
// from the Cartridge class body (LADDER-2 c1): ActiveArch (entities.hpp,
// file scope) embeds one per arch, and a file-scope header cannot see an
// in-class type — the second-consumer law, applied as for MOOD_COUNT.
// This header is the mood/transition shared vocabulary.
struct PortalDestination {
    uint32_t seed = 0;
    bool finite = false;
    uint32_t finite_radius = 2;
    uint32_t mood = 0;               // 0=open, 1=finite (expandable)
};

} // namespace the_board
} // namespace t7
