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


// ═══ THE BEHAVIOUR VOCABULARY, GRADUATED (ORGAN_3 w3) ═════════════
// It rides HERE, beside the tier bank, because it rides the SAME
// AUTHOR: upload_agent_registries_to_gpu reads both tables in one
// function and the frame boundary already re-speaks it whenever the
// tier bank changes (ORGAN_2b U4). A second contracts file would have
// been a second home for one author's material; a second dirty flag
// would have been a second name for one occasion. Kind BEHAVIOR
// therefore raises the TIER flag — one flag, one boundary, two banks.
//
// AGENT_BEHAVIORS is the DESIGN; BEHAVIOR_LIVE is what the translator
// reads. Eleven rows of eight float columns (ATRIUM_4 added the passer
// and its aux) — over D5's ≤8 cut, and enrolled per-row anyway: D5's
// line is about COMPOSITE rows (names, modes, nested tables), and these
// are uniform float columns with a stable row identity. The exception is
// deliberate and this sentence is its reason.

enum AgentBehaviorId : uint32_t {
    AGENT_BEHAVIOR_PLAYER_CONTROLLED = 0,
    AGENT_BEHAVIOR_RANDOM_WALK       = 1,
    AGENT_BEHAVIOR_BIASED_WALK       = 2,   // persistent direction + soft cohesion
    AGENT_BEHAVIOR_WANDERER          = 3,   // random walk + soft home tether
    AGENT_BEHAVIOR_HOME_SEEKER       = 4,   // strong spring to home
    AGENT_BEHAVIOR_SLOW_PATROL       = 5,   // waypoints around home, slow
    AGENT_BEHAVIOR_PURSUIT           = 6,   // steers toward player when in range
    AGENT_BEHAVIOR_FLEE              = 7,   // flees player when in range, idles otherwise
    AGENT_BEHAVIOR_FLOCK2D           = 8,   // Vicsek alignment + cohesion
    AGENT_BEHAVIOR_LEVY_FLIGHT       = 9,   // power-law step magnitudes
    AGENT_BEHAVIOR_PASSER            = 10,  // ATRIUM_4 — walks a route of doors, in and out
    AGENT_BEHAVIOR_COUNT             = 11,
};

struct AgentBehaviorDef {
    AgentBehaviorId id;
    const char*     name;
    float           step_rate;
    float           step_size;      // world units per step — AND, on PASSER, the
                                    // WAYPOINT RADIUS: the field's documented second role
    float           persistence;
    float           drag;
    float           home_pull;
    float           neighbor_radius;
    float           speed_cap;
    // ATRIUM_4 — a behaviour-specific scalar: PASSER = the band, wu, either
    // side of a door's plane; 0 elsewhere. One column rather than a second
    // table, because exactly one behaviour has ever wanted a number of its
    // own and a column the other rows read as 0 costs them nothing.
    float           aux;
};

inline constexpr AgentBehaviorDef AGENT_BEHAVIORS[AGENT_BEHAVIOR_COUNT] = {
    //  id                                  name               step_rate  step_size  persistence  drag  home_pull  neighbor_radius  speed_cap  aux
    { AGENT_BEHAVIOR_PLAYER_CONTROLLED, "player_controlled",   0.0f,      0.0f,      0.0f,        0.0f, 0.0f,      0.0f,            0.0f,      0.0f },
    { AGENT_BEHAVIOR_RANDOM_WALK,       "random_walk",         0.8f,      1.5f,      0.0f,        3.0f, 0.0f,      0.0f,            3.0f,      0.0f },
    { AGENT_BEHAVIOR_BIASED_WALK,       "biased_walk",         0.5f,      2.5f,      0.85f,       0.6f, 0.0f,      25.0f,           5.0f,      0.0f },
    { AGENT_BEHAVIOR_WANDERER,          "wanderer",            0.7f,      1.8f,      0.0f,        1.0f, 0.4f,      0.0f,            4.0f,      0.0f },
    { AGENT_BEHAVIOR_HOME_SEEKER,       "home_seeker",         0.4f,      1.0f,      0.0f,        1.5f, 3.0f,      0.0f,            3.0f,      0.0f },
    { AGENT_BEHAVIOR_SLOW_PATROL,       "slow_patrol",         0.25f,     8.0f,      0.0f,        2.0f, 4.0f,      0.0f,            2.0f,      0.0f },
    { AGENT_BEHAVIOR_PURSUIT,           "pursuit",             0.5f,      1.5f,      0.0f,        1.0f, 5.0f,      40.0f,           5.0f,      0.0f },
    { AGENT_BEHAVIOR_FLEE,              "flee",                0.4f,      1.0f,      0.0f,        1.5f, 8.0f,      30.0f,           8.0f,      0.0f },
    { AGENT_BEHAVIOR_FLOCK2D,           "flock2d",             0.6f,      1.5f,      0.7f,        0.6f, 0.0f,      30.0f,           4.5f,      0.0f },
    { AGENT_BEHAVIOR_LEVY_FLIGHT,       "levy_flight",         0.4f,      0.8f,      0.0f,        1.5f, 0.0f,      0.0f,            8.0f,      0.0f },
    // THE PASSER WALKS, and this row stopped being slow_patrol's (ATRIUM_6).
    //   speed_cap 9.0 is 0.6 x PAWN_SPEED (15, world.wgsl). ATRIUM_6 made it
    //     1.5x and the passage read as a chase; the PROPERTY this number is
    //     for is the opposite one — THE VISITOR OVERTAKES A PASSER AT A WALK
    //     and can follow one to a door without falling behind. The passage
    //     is something you can join. At 0.6 the visitor closes at 6 wu/s,
    //     which is a gain of one body length a second (ATRIUM_8).
    //   drag 1.8 is THE BLEND RATE OF THE WALK, not a decay: the arm eases
    //     direction toward a constant-speed desired velocity and calls
    //     agent_post_step with drag 0, so the turn completes over about half
    //     a second and the speed never sags. At 3.0 they darted between
    //     waypoints — the frolic, and the turn was a third of what a body
    //     that size should need (ATRIUM_8).
    //   step_size 2.5 is the WAYPOINT RADIUS — a geometry, not a pace, so
    //     the slower walk did not move it.
    //   aux 8.0 is the band, the turnaround outside a door.
    //   home_pull and step_rate are UNREAD on this arm: the tether was what
    //     made ATRIUM_5's passers crawl the last metres and stall, and the
    //     step kick was never a gait — only noise on the velocity, which on
    //     a constant-speed walk is a shove, so ATRIUM_6 cut it from the arm
    //     and there is nothing left to mute. persistence and neighbor_radius
    //     were never read here.
    { AGENT_BEHAVIOR_PASSER,            "passer",              0.25f,     2.5f,      0.0f,        1.8f, 4.0f,      0.0f,            9.0f,      8.0f },
};

struct AgentBehaviorBank {
    AgentBehaviorDef b[AGENT_BEHAVIOR_COUNT];
};

inline AgentBehaviorBank BEHAVIOR_LIVE = {
    { AGENT_BEHAVIORS[0], AGENT_BEHAVIORS[1], AGENT_BEHAVIORS[2],
      AGENT_BEHAVIORS[3], AGENT_BEHAVIORS[4], AGENT_BEHAVIORS[5],
      AGENT_BEHAVIORS[6], AGENT_BEHAVIORS[7], AGENT_BEHAVIORS[8],
      AGENT_BEHAVIORS[9], AGENT_BEHAVIORS[10] }
};
static_assert(AGENT_BEHAVIOR_COUNT == 11,
    "BEHAVIOR_LIVE is seeded row by row (constexpr copy, one per behavior): "
    "a new behavior needs its row here as well as in AGENT_BEHAVIORS");

} // namespace the_board
} // namespace t7
