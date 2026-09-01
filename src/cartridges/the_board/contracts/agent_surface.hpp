#pragma once

// ─── agent_surface.hpp ───────────────────────────────────────────
// THE AGENTS' PANEL — how many walk this world, and what they are.
//
// ONE_WORLD-II U1c, founded per ruling. AGENT_POPULATIONS carried seven
// rows and the spawners indexed the live mood; there is one world now, so
// there is one row. This is the contracts seat the agents did not have:
// the bank stands here, its authored source and its witness stay beside
// AGENT_POPULATIONS in bodies/agents.hpp until that table leaves, and the
// spawners read the bank.
//
// TWO FACTS WORE ONE NAME in revB, and the ruling split them: this is the
// AGENT population bank. The five entity families' spawn dials and
// GLOBAL_ENTITY_DENSITY are the POPULATION PANEL's — an institution at
// population_themes.hpp, refounded at U3, enrolled at U6. They were never
// the same struct and never the same file.
//
// THE SEED TRAP, WRITTEN DOWN. The bank seeds from the SUNSET row because
// sunset is the look this campaign keeps — explicitly NOT from the walled
// row, even though the world pins FINITE. That row was
// `/* unpopulated */`: count 0, every weight 0, and both spawn guards test
// `count > 0 && beh_sum > 0 && tier_sum > 0`, so matching the seed row to
// the pinned shape would empty the world in silence. The pinned SHAPE and
// the worn WEATHER are separate facts by design.

#include <array>
#include <cstdint>
#include "cartridges/the_board/contracts/agent_tiers.hpp"   // AGENT_BEHAVIOR_COUNT / AGENT_TIER_COUNT — the two widths this bank is cut to

namespace t7 {
namespace the_board {

// The row, minus the id that selected it. Field order is
// AgentPopulationDef's, so the transcription below reads against the
// authored table line for line.
struct AgentPopulationBank {
    uint32_t count;                                          // 0..Dim::MAX_AGENTS-1
    std::array<float, AGENT_BEHAVIOR_COUNT> behavior_weights;
    std::array<float, AGENT_TIER_COUNT>     tier_weights;
    float    spawn_inner_radius;                             // world units (annulus inner)
    float    spawn_radius;                                   // world units (annulus outer)
    float    spawn_center_forward;                           // world units the annulus' CENTRE
                                                             // rides along the arrival gaze
    float    home_seeding_radius;                            // world units from spawn point
};

// THE DESIGN, transcribed from AGENT_POPULATIONS' sunset row (the
// canonized pattern: transcribe and pin, never derive from the dying).
// The witness that proves it lives beside that table, where both symbols
// are in scope, and leaves with it.
inline constexpr AgentPopulationBank AGENTS_TABLE = {
    /*count=*/ 10,
    //                       player rwalk  bwalk wandr hseek slowp pursu  flee flock  levy
    /*behavior_weights=*/ {    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    //                     worker scout sentl leadr
    /*tier_weights=*/     {  1.0f, 3.0f, 0.0f, 0.0f },
    /*spawn_inner_radius=*/ 200.0f,
    /*spawn_radius=*/       340.0f,
    /*spawn_center_forward=*/ 0.0f,
    /*home_seeding_radius=*/ 8.0f,
};

inline AgentPopulationBank AGENTS_LIVE = AGENTS_TABLE;

// THE SUMS ARE THE BANK'S, TAKEN LIVE. OIL_1 U6 retired a per-frame
// re-sum in favour of constexpr AGENT_BEH_SUMS / AGENT_TIER_SUMS over the
// authored table — correct while the table was the only truth. A bank is
// writable, so a constexpr sum over the DESIGN would go stale the moment a
// weight dial moved and the spawner would normalise against a total that
// no longer matched its own weights: a mirror split of exactly the kind
// Amendment D exists to prevent. Fourteen adds, at the two call sites that
// already walk both arrays.
inline float agents_behavior_sum() {
    float s = 0.0f;
    for (uint32_t b = 0; b < AGENT_BEHAVIOR_COUNT; ++b) s += AGENTS_LIVE.behavior_weights[b];
    return s;
}
inline float agents_tier_sum() {
    float s = 0.0f;
    for (uint32_t t = 0; t < AGENT_TIER_COUNT; ++t) s += AGENTS_LIVE.tier_weights[t];
    return s;
}

} // namespace the_board
} // namespace t7
