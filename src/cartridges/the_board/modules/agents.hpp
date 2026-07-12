#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/state.hpp"                    // Dim::MAX_AGENTS, GPUAgentState, GPU_AGENT_*_COUNT, wgpu
#include "cartridges/the_board/modules/mood_constants.hpp"   // MOOD_COUNT + the Mood IDs
#include "cartridges/the_board/modules/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── agents.hpp (HEADER: registries + console + state + decls) ───
// Converted (LADDER-3 c2, G1): history in audit/LADDER.md.
//
// Unified entity registry: the control panel for the agent system.
// Every pawn-like body on the board — the one the player inhabits and
// every mood-authored wanderer — is one slot in agentStateBuffer_,
// driven by one of these behaviors, tinted by one of these tiers,
// populated per mood from AGENT_POPULATIONS.
//
// The player's relationship to this array is `player_.possessed_slot`;
// the compute kernel treats that slot as the PlayerControlled branch
// and every other active slot as its authored behavior. See
// state.hpp AgentState and world.wgsl §7 for the wiring.
//
// ┌─── Three registries ────────────────────────────────────────────┐
// │                                                                  │
// │  AGENT_BEHAVIORS    per-behavior motion parameters               │
// │                     (step rate, step size, drag, speed cap, ...) │
// │                                                                  │
// │  AGENT_TIER_GAINS   per-tier multipliers + render color          │
// │                     (Worker / Scout / Sentinel / Leader)         │
// │                                                                  │
// │  AGENT_POPULATIONS  per-mood population authoring                │
// │                     (count, behavior/tier weights, spawn radius) │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// All ten behaviors (PlayerControlled + nine algorithmic) are
// implemented and tunable. Tier gains are authored for all four
// tiers. Populations are authored for the outdoor + gallery moods;
// the two finite_outdoor moods default to count=0 (unpopulated).
//
// ┌─── Public surface (called from outside this module) ────────────┐
// │                                                                  │
// │  Module functions take AgentState& explicitly                    │
// │  (or const AgentState& when read-only).                          │
// │                                                                  │
// │  Lifecycle:                                                      │
// │    upload_agent_registries_to_gpu(c, queue)   — once at init     │
// │      (note: takes no AgentState — uploads constexpr registries   │
// │       only, doesn't touch agent slots)                           │
// │    spawn_population_for_mood(as, c, ...)      — mood entry       │
// │    respawn_evicted_agents(as, c, ...)         — every-frame fill │
// │                                                                  │
// │  Player commands:                                                │
// │    try_possess_nearest(as, c, queue)          — Caps Lock        │
// │                                                                  │
// │  Diagnostic cycling (wired in input.inl):                       │
// │    cycle_agent_behavior_override(as, c, q)    F1                 │
// │    cycle_agent_tier_override(as, c, q)        F2                 │
// │    force_respawn_population(as, c, q)         F3                 │
// │                                                                  │
// │  Logging:                                                        │
// │    dump_agent_census(as, c, trigger)          — periodic+on event│
// │                                                                  │
// │  Cross-module reads (consumed by other modules):                 │
// │    agent_state_.slots[player_.possessed_slot] — read by spine,  │
// │      cube_behaviors.inl (player position queries)                │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// ┌─── Why not EntityFamilyAdapter? ─────────────────────────────────┐
// │                                                                  │
// │  Other living things in the world (blades, palms, arches, etc.)  │
// │  flow through entity_pipeline.inl's generic select / place /     │
// │  commit machinery. Agents are deliberately separate because:     │
// │                                                                  │
// │   • Fixed-size 32-slot array, not streaming. No per-frame        │
// │     entity_refs eviction; the agent kernel decides who lives.    │
// │   • Player-possessable. Slot PLAYER_SLOT is special-cased on     │
// │     both CPU (camera, input, readback) and GPU (separate kernel  │
// │     update_player_agent vs update_other_agents).                 │
// │   • Has its own state-evolution kernel; entities are placed once │
// │     and only re-rendered.                                        │
// │                                                                  │
// │  The `tier` vocabulary overlaps but the machinery is unrelated:  │
// │  AgentTierDef is not EntityFamilyTraits::TierProfile.            │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// Depends on: state.hpp (Dim::MAX_AGENTS, GPUAgentState, the GPU count
// twins), mood_constants.hpp (MOOD_COUNT + Mood IDs), <array>. The impl
// additionally reads COLUMN_PALETTE (entities.hpp) and reaches the
// keyhole.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ BEHAVIOR IDS ════════════════════════════════════════════════
//
// Stable indices into AGENT_BEHAVIORS. The compute kernel's behavior
// switch dispatches on these values. Names below are also exported as
// AGENT_BEHAVIOR_NAMES[] for diagnostics — keep the two in lockstep.

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
    AGENT_BEHAVIOR_COUNT             = 10,
};

// ═══ TIER IDS ════════════════════════════════════════════════════
//
// Visual + parametric archetype — a property of the body, not of the
// driver. A Scout stays a Scout when the player leaves it; a
// RandomWalk scout moves with Scout-tier speed/persistence.

enum AgentTierId : uint32_t {
    AGENT_TIER_WORKER   = 0,
    AGENT_TIER_SCOUT    = 1,
    AGENT_TIER_SENTINEL = 2,
    AGENT_TIER_LEADER   = 3,
    AGENT_TIER_COUNT    = 4,
};

// Cross-check: GPU-side count constants in state.hpp must match the
// authoritative enums above. If you add a behavior or a tier, bump
// the GPU constant in state.hpp at the same time. The compiler will
// catch any drift here.
static_assert(AGENT_BEHAVIOR_COUNT == GPU_AGENT_BEHAVIOR_COUNT,
    "AGENT_BEHAVIOR_COUNT must match GPU_AGENT_BEHAVIOR_COUNT in state.hpp");
static_assert(AGENT_TIER_COUNT == GPU_AGENT_TIER_COUNT,
    "AGENT_TIER_COUNT must match GPU_AGENT_TIER_COUNT in state.hpp");

// Display names for diagnostics (census output, error messages).
// Order MUST match the enums above — index by the enum value.
// Mirrors the ORB_PAL_NAMES / ORB_TIERSET_NAMES pattern from orbs.hpp.
inline constexpr const char* AGENT_BEHAVIOR_NAMES[AGENT_BEHAVIOR_COUNT] = {
    "player",        //  0  PLAYER_CONTROLLED
    "random_walk",   //  1  RANDOM_WALK
    "biased_walk",   //  2  BIASED_WALK
    "wanderer",      //  3  WANDERER
    "home_seeker",   //  4  HOME_SEEKER
    "slow_patrol",   //  5  SLOW_PATROL
    "pursuit",       //  6  PURSUIT
    "flee",          //  7  FLEE
    "flock2d",       //  8  FLOCK2D
    "levy_flight",   //  9  LEVY_FLIGHT
};

inline constexpr const char* AGENT_TIER_NAMES[AGENT_TIER_COUNT] = {
    "worker",        //  0
    "scout",         //  1
    "sentinel",      //  2
    "leader",        //  3
};

// ═══ TUNING CONSOLE ══════════════════════════════════════════════
//
// All authored radii live here so the relationships between them are
// visible at a glance. Mirrors the tuning-console block at the top of
// orbs.hpp. If you change one of these, scan the comments below to
// see what else may need to move.

// PLAYER_SLOT — slot 0 of agentStateBuffer_ is the player's INITIAL
// body and the canonical "skip this in respawn loops" sentinel. Every
// spawn/respawn skips slot 0. Caps Lock possession transfer can move
// the player into a non-zero slot (player_.possessed_slot tracks
// which one); reads of "where is the player right now" should go
// through agent_state_.slots[player_.possessed_slot], not [PLAYER_SLOT].
inline constexpr uint32_t PLAYER_SLOT = 0;

// POSSESSION_RADIUS — Caps Lock teleports the player into the nearest
// active agent within this radius. Set so the player can comfortably
// pick a target by walking near it; any larger and possession becomes
// "leap to wherever Caps Lock feels like it." See try_possess_nearest.
inline constexpr float POSSESSION_RADIUS    = 20.0f;
inline constexpr float POSSESSION_RADIUS_SQ = POSSESSION_RADIUS * POSSESSION_RADIUS;

// AGENT_EVICTION_RADIUS — outdoor agents that drift further than this
// from the player are evicted (set is_active=0 by the GPU kernel) and
// later refilled by respawn_evicted_agents.
//
// SEAM[agents:L2] hardware mirror — AGENT_EVICTION_RADIUS must agree
//   with world.wgsl's identically-named const. The compiler cannot
//   catch drift; the prose below is the contract.
//
// MIRRORED MANUALLY in world.wgsl as `const AGENT_EVICTION_RADIUS:
// f32 = 360.0` (the WGSL needs a compile-time const for FXC inlining;
// no runtime upload exists). If you change this value, change the
// WGSL constant too — the compiler will not catch the drift.
//
// Couples with each population's spawn_radius below: a population
// whose spawn_radius approaches the eviction radius will see immediate
// re-eviction and look like it's flickering. Keep at least ~20 units
// of headroom (spawn_radius=340 + this=360 gives a 20-unit "alive"
// band where agents persist before being recycled).
inline constexpr float AGENT_EVICTION_RADIUS    = 360.0f;
inline constexpr float AGENT_EVICTION_RADIUS_SQ = AGENT_EVICTION_RADIUS * AGENT_EVICTION_RADIUS;

// AGENT_CENSUS_INTERVAL — period (seconds) between automatic
// [AGENTS] census log lines + the [Player] position emission.
inline constexpr float AGENT_CENSUS_INTERVAL = 30.0f;

// ═══ REGISTRY: BEHAVIORS ═════════════════════════════════════════
//
// Per-behavior motion parameters. Units:
//   step_rate         steps per *beat* (musical time)
//   step_size         world units per step (BiasedWalk) /
//                     waypoint radius from home (SlowPatrol)
//   persistence       [0,1] — directional commitment (BiasedWalk:
//                     1=lock to travel azimuth, 0=full random)
//   drag              1/s — velocity decay coefficient (exponential)
//   home_pull         1/s² — spring coefficient (SlowPatrol uses for
//                     waypoint steering)
//   neighbor_radius   world units — flock/cohesion neighbor radius
//   speed_cap         world units/s — per-agent max speed
//
// PlayerControlled rows are all zero: the kernel switch case reads
// input directly rather than these parameters.
//
// Home tether (home_x/y/z in GPUAgentState) is consumed by:
//   WANDERER     — soft pull back to home, lets the agent meander
//   HOME_SEEKER  — strong spring to home, dominant force
//   SLOW_PATROL  — home is the patrol anchor; waypoints orbit it
// Other behaviors ignore the home position; spawn still seeds it
// (cheap, harmless) so the field is meaningful if a slot's behavior
// later changes via possession transfer or override.

struct AgentBehaviorDef {
    AgentBehaviorId id;
    const char*     name;
    float           step_rate;
    float           step_size;
    float           persistence;
    float           drag;
    float           home_pull;
    float           neighbor_radius;
    float           speed_cap;
};

inline constexpr AgentBehaviorDef AGENT_BEHAVIORS[AGENT_BEHAVIOR_COUNT] = {
    //  id                                  name               step_rate  step_size  persistence  drag  home_pull  neighbor_radius  speed_cap
    { AGENT_BEHAVIOR_PLAYER_CONTROLLED, "player_controlled",   0.0f,      0.0f,      0.0f,        0.0f, 0.0f,      0.0f,            0.0f    },
    { AGENT_BEHAVIOR_RANDOM_WALK,       "random_walk",         0.8f,      1.5f,      0.0f,        3.0f, 0.0f,      0.0f,            3.0f    },
    { AGENT_BEHAVIOR_BIASED_WALK,       "biased_walk",         0.5f,      2.5f,      0.85f,       0.6f, 0.0f,      25.0f,           5.0f    },
    { AGENT_BEHAVIOR_WANDERER,          "wanderer",            0.7f,      1.8f,      0.0f,        1.0f, 0.4f,      0.0f,            4.0f    },
    { AGENT_BEHAVIOR_HOME_SEEKER,       "home_seeker",         0.4f,      1.0f,      0.0f,        1.5f, 3.0f,      0.0f,            3.0f    },
    { AGENT_BEHAVIOR_SLOW_PATROL,       "slow_patrol",         0.25f,     8.0f,      0.0f,        2.0f, 4.0f,      0.0f,            2.0f    },
    { AGENT_BEHAVIOR_PURSUIT,           "pursuit",             0.5f,      1.5f,      0.0f,        1.0f, 5.0f,      40.0f,           5.0f    },
    { AGENT_BEHAVIOR_FLEE,              "flee",                0.4f,      1.0f,      0.0f,        1.5f, 8.0f,      30.0f,           8.0f    },
    { AGENT_BEHAVIOR_FLOCK2D,           "flock2d",             0.6f,      1.5f,      0.7f,        0.6f, 0.0f,      30.0f,           4.5f    },
    { AGENT_BEHAVIOR_LEVY_FLIGHT,       "levy_flight",         0.4f,      0.8f,      0.0f,        1.5f, 0.0f,      0.0f,            8.0f    },
};

static_assert(sizeof(AGENT_BEHAVIORS) / sizeof(AGENT_BEHAVIORS[0]) == AGENT_BEHAVIOR_COUNT,
              "AGENT_BEHAVIORS must declare one row per AgentBehaviorId");

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════
//
// Per-tier multipliers on behavior parameters, plus render color.
// Compound with behavior params: a Scout running RandomWalk takes
// longer, less-persistent steps than a Worker running RandomWalk.
//
// Color authored as RGB [0,1]. The tier color is the body's identity
// — a Scout is bronze regardless of who's driving it.

struct AgentTierDef {
    AgentTierId id;
    const char* name;
    float       step_gain;       // multiplies step_size
    float       persist_gain;    // multiplies persistence
    float       speed_gain;      // multiplies speed_cap
    float       color_r;
    float       color_g;
    float       color_b;
};

inline constexpr AgentTierDef AGENT_TIER_GAINS[AGENT_TIER_COUNT] = {
    //  id                     name        step  persist  speed  color
    { AGENT_TIER_WORKER,   "worker",   1.0f, 1.0f, 1.0f, 0.60f, 0.62f, 0.65f },  // slate gray
    { AGENT_TIER_SCOUT,    "scout",    1.8f, 0.4f, 1.4f, 0.85f, 0.65f, 0.40f },  // bronze
    { AGENT_TIER_SENTINEL, "sentinel", 0.6f, 1.2f, 0.5f, 0.30f, 0.40f, 0.70f },  // deep blue
    { AGENT_TIER_LEADER,   "leader",   1.2f, 0.9f, 1.1f, 0.95f, 0.85f, 0.55f },  // pale gold
};

static_assert(sizeof(AGENT_TIER_GAINS) / sizeof(AGENT_TIER_GAINS[0]) == AGENT_TIER_COUNT,
              "AGENT_TIER_GAINS must declare one row per AgentTierId");

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════
//
// Per-mood population authoring. Indexed by mood id; one row per
// mood. `count = 0` means the mood spawns no agents (the player is
// alone — a valid configuration).
//
// behavior_weights[] and tier_weights[] are probabilities (any
// non-negative values; they're normalized at spawn time).
//
// Spawn distribution: agents appear on an *annulus* around the player
// (uniform area distribution). spawn_inner_radius and spawn_radius
// define the annulus bounds. When inner == 0, the annulus collapses
// to a uniform disk — used for indoor/gallery moods where there's no
// "horizon" to spawn beyond.
//
// Outdoor moods set inner just outside the visible horizon (~200) and
// outer near the patch pre-gen edge (~340), so agents appear at
// distance and walk inward. This matches the floater lifecycle.
// Gallery moods set inner = 0 and outer to the room's reach, so
// agents spawn within the room.
//
// home_seeding_radius is how far each agent's home tether offset
// ranges from its spawn point.

struct AgentPopulationDef {
    uint32_t mood_id;
    uint32_t count;                                          // 0..Dim::MAX_AGENTS-1
    std::array<float, AGENT_BEHAVIOR_COUNT> behavior_weights;
    std::array<float, AGENT_TIER_COUNT>     tier_weights;
    float    spawn_inner_radius;                             // world units (annulus inner)
    float    spawn_radius;                                   // world units (annulus outer)
    float    home_seeding_radius;                            // world units from spawn point
};

// ─── Why no constexpr helper builders ───────────────────────────
//
// The weight arrays are written out unfolded rather than factored
// through constexpr helpers; the restyle is a NAMED LATER STAGE
// (clean bisection).
//
// The fix is plain literal initialization with column-header comments
// above each row indicating which behavior / tier slot is non-zero.
// std::array accepts braced lists via brace elision (single braces
// suffice), so the rows stay readable.

// Mood ordering matches MOOD_TABLE (mood.hpp). The named constants
// (mood_constants.hpp) and per-row static_asserts below catch any
// reordering at compile time.
//
// Outdoor moods spawn agents on a wide annulus near the patch
// pre-gen edge — agents appear at distance, walk inward, evict at
// the world's horizon. Gallery moods use a tight annulus inside the
// indoor world bounds; agents spawn within visibility because there
// is no "horizon" inside a room.
//
// Each row's behavior_weights and tier_weights are preceded by a
// column-header comment so the active slot is visible without
// counting zeros.
inline constexpr AgentPopulationDef AGENT_POPULATIONS[MOOD_COUNT] = {
    /* MOOD_OPEN_DEFAULT — desert travelers (BiasedWalk) */
    { /*mood_id=*/ MOOD_OPEN_DEFAULT, /*count=*/ 10,
      //                       player rwalk  bwalk wandr hseek slowp pursu  flee flock  levy
      /*behavior_weights=*/ {    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
      //                     worker scout sentl leadr
      /*tier_weights=*/     {  2.0f, 2.0f, 1.0f, 0.0f },
      /*spawn_inner_radius=*/ 200.0f,
      /*spawn_radius=*/       340.0f,
      /*home_seeding_radius=*/ 5.0f },
    /* MOOD_OPEN_SUNSET — Scout-heavy travelers (BiasedWalk) */
    { /*mood_id=*/ MOOD_OPEN_SUNSET, /*count=*/ 10,
      //                       player rwalk  bwalk wandr hseek slowp pursu  flee flock  levy
      /*behavior_weights=*/ {    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
      //                     worker scout sentl leadr
      /*tier_weights=*/     {  1.0f, 3.0f, 0.0f, 0.0f },
      /*spawn_inner_radius=*/ 200.0f,
      /*spawn_radius=*/       340.0f,
      /*home_seeding_radius=*/ 8.0f },
    /* MOOD_INDOOR_FLAT — gallery walkers (SlowPatrol) */
    { /*mood_id=*/ MOOD_INDOOR_FLAT, /*count=*/ 4,
      //                       player rwalk  bwalk wandr hseek slowp pursu  flee flock  levy
      /*behavior_weights=*/ {    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
      //                     worker scout sentl leadr
      /*tier_weights=*/     {  2.0f, 0.0f, 2.0f, 1.0f },
      /*spawn_inner_radius=*/ 0.0f,
      /*spawn_radius=*/       60.0f,
      /*home_seeding_radius=*/ 30.0f },
    /* MOOD_INDOOR_VAULT — gallery walkers (SlowPatrol) */
    { /*mood_id=*/ MOOD_INDOOR_VAULT, /*count=*/ 4,
      //                       player rwalk  bwalk wandr hseek slowp pursu  flee flock  levy
      /*behavior_weights=*/ {    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
      //                     worker scout sentl leadr
      /*tier_weights=*/     {  2.0f, 0.0f, 2.0f, 1.0f },
      /*spawn_inner_radius=*/ 0.0f,
      /*spawn_radius=*/       60.0f,
      /*home_seeding_radius=*/ 30.0f },
    /* MOOD_FINITE_OUTDOOR — unpopulated */
    { /*mood_id=*/ MOOD_FINITE_OUTDOOR, /*count=*/ 0,
      /*behavior_weights=*/ {},
      /*tier_weights=*/     {},
      /*spawn_inner_radius=*/ 0.0f,
      /*spawn_radius=*/       0.0f,
      /*home_seeding_radius=*/ 0.0f },
    /* MOOD_FINITE_OUTDOOR_REF — unpopulated */
    { /*mood_id=*/ MOOD_FINITE_OUTDOOR_REF, /*count=*/ 0,
      /*behavior_weights=*/ {},
      /*tier_weights=*/     {},
      /*spawn_inner_radius=*/ 0.0f,
      /*spawn_radius=*/       0.0f,
      /*home_seeding_radius=*/ 0.0f },
};

static_assert(sizeof(AGENT_POPULATIONS) / sizeof(AGENT_POPULATIONS[0]) == MOOD_COUNT,
              "AGENT_POPULATIONS must declare one row per mood");

// Row order must match the mood ids in MOOD_TABLE (mood.hpp).
// Unfolded rather than a constexpr loop — the restyle is a named
// later stage.
static_assert(AGENT_POPULATIONS[MOOD_OPEN_DEFAULT      ].mood_id == MOOD_OPEN_DEFAULT,       "AGENT_POPULATIONS row 0 must be MOOD_OPEN_DEFAULT");
static_assert(AGENT_POPULATIONS[MOOD_OPEN_SUNSET       ].mood_id == MOOD_OPEN_SUNSET,        "AGENT_POPULATIONS row 1 must be MOOD_OPEN_SUNSET");
static_assert(AGENT_POPULATIONS[MOOD_INDOOR_FLAT       ].mood_id == MOOD_INDOOR_FLAT,        "AGENT_POPULATIONS row 2 must be MOOD_INDOOR_FLAT");
static_assert(AGENT_POPULATIONS[MOOD_INDOOR_VAULT      ].mood_id == MOOD_INDOOR_VAULT,       "AGENT_POPULATIONS row 3 must be MOOD_INDOOR_VAULT");
static_assert(AGENT_POPULATIONS[MOOD_FINITE_OUTDOOR    ].mood_id == MOOD_FINITE_OUTDOOR,     "AGENT_POPULATIONS row 4 must be MOOD_FINITE_OUTDOOR");
static_assert(AGENT_POPULATIONS[MOOD_FINITE_OUTDOOR_REF].mood_id == MOOD_FINITE_OUTDOOR_REF, "AGENT_POPULATIONS row 5 must be MOOD_FINITE_OUTDOOR_REF");

// ═══ AGENT MODULE STATE ══════════════════════════════════════════
//
// All agent-owned state lives in this struct, accessed via
// agent_state_ on the Cartridge (declared at the composition root).
// Module functions take `AgentState& as` explicitly rather than
// reaching via Cartridge*, making ownership language-visible and
// dependencies explicit in signatures.
//
// Fields:
//
//   slots — Host-side mirror of agentStateBuffer_ refreshed from a
//     GPU readback. Used by Caps Lock targeting (nearest-within-
//     radius) and by patch streaming (which reads the possessed
//     slot's XZ).
//
//   respawn_counters — Per-slot respawn count. Mixed into the seed
//     each time a slot is refilled, so successive respawns of the
//     same slot roll different attributes / pose.
//
//   behavior_override / tier_override — Inspection toggles. When set,
//     every active non-player slot is forced to the override value,
//     bypassing the population's natural rolls. AGENT_OVERRIDE_NONE
//     means "no override — population wins." Overrides apply both to
//     existing active agents (cycle_*_override rewrites them in
//     place) and to newly-respawned agents (populate_agent_slot_
//     consults the override). Clearing the override does NOT revert
//     existing agents — only future respawns roll naturally again.
//     Intended as developer/diagnostic dials, not gameplay state.
//
//   last_census_dump — Periodic census log timestamp. Read +
//     written from the cartridge's update loop.

inline constexpr uint32_t AGENT_OVERRIDE_NONE = 0xFFFFFFFFu;

struct AgentState {
    GPUAgentState slots[Dim::MAX_AGENTS]            = {};
    uint32_t      respawn_counters[Dim::MAX_AGENTS] = {};
    uint32_t      behavior_override                 = AGENT_OVERRIDE_NONE;
    uint32_t      tier_override                     = AGENT_OVERRIDE_NONE;
    float         last_census_dump                  = -999.0f;
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════
//
// DEFINED in agents.inl (post-class, self-wrapping). populate_agent_slot_
// and apply_agent_overrides_ are module-internal helpers (impl-only, not
// declared here).

// Lifecycle
void upload_agent_registries_to_gpu(Cartridge* c, wgpu::Queue& queue);
void spawn_population_for_mood(AgentState& as, Cartridge* c,
                               uint32_t mood_id,
                               uint32_t seed,
                               float center_x, float center_z,
                               wgpu::Queue& queue);
void respawn_evicted_agents(AgentState& as, Cartridge* c,
                            uint32_t mood_id,
                            uint32_t world_seed,
                            wgpu::Queue& queue);
// Player commands
void try_possess_nearest(AgentState& as, Cartridge* c, wgpu::Queue& queue);
// Diagnostic cycling (wired in input.inl)
void cycle_agent_behavior_override(AgentState& as, Cartridge* c, wgpu::Queue& queue);
void cycle_agent_tier_override(AgentState& as, Cartridge* c, wgpu::Queue& queue);
void force_respawn_population(AgentState& as, Cartridge* c, wgpu::Queue& queue);
// Logging
void dump_agent_census(const AgentState& as, const Cartridge* c, const char* trigger);

} // namespace the_board
} // namespace t7
