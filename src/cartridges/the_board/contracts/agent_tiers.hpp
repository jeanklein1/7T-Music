#pragma once
#include <cstdint>

// ─── contracts/agent_tiers.hpp ─────────────────────────────────────
//
// THE TIER VOCABULARY, GRADUATED (ORGAN_2b). Same move MoodProfile
// made to contracts/spine_state.hpp, for the same reason: the organ
// needs to name the definition, and the organ may not include a body.
//
// AGENT_TIER_GAINS is the DESIGN — the authored table, two jobs only:
// seeding TIER_LIVE and standing under the asserts. TIER_LIVE is the
// WORLD'S DEFINITION — what this world means by its tiers. The author
// (upload_agent_registries_to_gpu, bodies/agents.hpp) reads the bank;
// a panel edit to the bank outlives the author because the author
// reads it on every re-speak, world init included.
//
// THE ELIGIBILITY RULE, world analog: a tier field may carry a TIER
// definition only if upload_agent_registries_to_gpu is the bank's
// only runtime reader. Compile-time budget readers (the gallery
// pattern) stay on AGENT_TIER_GAINS on purpose: a wall's allowance
// is not a live dial.
//
// AGENT_TIER_COUNT rode along: it is declared inside AgentTierId,
// which the table's rows name, so the ids are the table's vocabulary
// and could not stay behind. The GPU cross-check that stands on the
// count (AGENT_TIER_COUNT == GPU_AGENT_TIER_COUNT) stays in
// bodies/agents.hpp — a contract may not include a realization.
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TIER IDS ════════════════════════════════════════════════════

enum AgentTierId : uint32_t {
    AGENT_TIER_WORKER   = 0,
    AGENT_TIER_SCOUT    = 1,
    AGENT_TIER_SENTINEL = 2,
    AGENT_TIER_LEADER   = 3,
    AGENT_TIER_COUNT    = 4,
};

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════

struct AgentTierDef {
    AgentTierId id;
    const char* name;
    float       step_gain;       // multiplies step_size
    float       persist_gain;    // multiplies persistence
    float       speed_gain;      // multiplies speed_cap
    float       color_r;
    float       color_g;
    float       color_b;
    float       contact_radius;  // TRUEBAND_CONTACT_1: body radius (wu) — Jean-tunable
    float       contact_mass;    // relative yield authority — Jean-tunable
    float       personal_radius; // CONTACT_2: social shell (flock sense + flee trigger) — Jean-tunable
    float       flee_gain_player;// CONTACT_2: flee response gain vs the point-source — Jean-tunable
};

inline constexpr AgentTierDef AGENT_TIER_GAINS[AGENT_TIER_COUNT] = {
    //  id                     name        step  persist  speed  color                 c_radius c_mass  p_radius flee_g (CONTACT_2 p_radius 30; CONTACT_4 S2a flee_g < 1 — the CATCHABILITY LAW)
    // CATCHABILITY LAW (CONTACT_4): the escape is a velocity FLOOR, so a gain
    // >= 1.0 means the agent matches or beats the player's radial speed and can
    // NEVER be approached (nor possessed). Gains < 1 close the gap at
    // (1 - gain*radial_share) of player speed. The tangential split
    // (esc = normalize(dir + tang*0.6), radial share ~= 0.86) makes the dodge
    // read as evasive at gains well under 1. Jean-tunable; keep every row < 1.0.
    { AGENT_TIER_WORKER,   "worker",   1.0f, 1.0f, 1.0f, 0.60f, 0.62f, 0.65f, 1.6f, 1.0f, 30.0f, 0.70f },  // slate gray
    { AGENT_TIER_SCOUT,    "scout",    1.8f, 0.4f, 1.4f, 0.85f, 0.65f, 0.40f, 1.4f, 0.8f, 30.0f, 0.85f },  // bronze
    { AGENT_TIER_SENTINEL, "sentinel", 0.6f, 1.2f, 0.5f, 0.30f, 0.40f, 0.70f, 2.0f, 1.5f, 30.0f, 0.50f },  // deep blue
    { AGENT_TIER_LEADER,   "leader",   1.2f, 0.9f, 1.1f, 0.95f, 0.85f, 0.55f, 1.8f, 1.2f, 30.0f, 0.60f },  // pale gold
};

static_assert(sizeof(AGENT_TIER_GAINS) / sizeof(AGENT_TIER_GAINS[0]) == AGENT_TIER_COUNT,
              "AGENT_TIER_GAINS must declare one row per AgentTierId");

// The bank — one struct so offsetof can name an element
// (offsetof(AgentTierBank, t[2].color_r) is the enrollment's spelling).
struct AgentTierBank {
    AgentTierDef t[AGENT_TIER_COUNT];
};

inline AgentTierBank TIER_LIVE = {
    { AGENT_TIER_GAINS[0], AGENT_TIER_GAINS[1],
      AGENT_TIER_GAINS[2], AGENT_TIER_GAINS[3] }
};
static_assert(AGENT_TIER_COUNT == 4,
    "TIER_LIVE is seeded row by row (constexpr copy, one per tier): "
    "a new tier needs its row here as well as in AGENT_TIER_GAINS");

} // namespace the_board
} // namespace t7
