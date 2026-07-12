#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/state.hpp"                       // Dim::MAX_CUBE_INSTANCES, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/modules/floater_vocabulary.hpp"  // ActiveCube, CUBE_TIER_COUNT
#include "cartridges/the_board/modules/mood_constants.hpp"      // MOOD_COUNT + the Mood IDs
#include "cartridges/the_board/modules/keyhole.hpp"             // Cartridge + wgpu::Queue fwds (the keyhole)
#include "cartridges/the_board/modules/entity_types.hpp"   // queue types (the funnel signatures)

// ─── cube_behaviors.hpp (HEADER: registries + console + state + decls) ─
// Converted (LADDER-3 c3): history in audit/LADDER.md.
//
// Cube behavior system.
//
// The impl additionally hashes with seed_utils and reaches the keyhole;
// the cube recipe (cube_behaviors.inl) calls into the two spawn
// helpers; world.wgsl holds the force functions and dispatch switch.
// ──────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ BEHAVIOR IDS ════════════════════════════════════════════════

inline constexpr uint32_t CUBE_BEHAVIOR_STATIONARY = 0;
inline constexpr uint32_t CUBE_BEHAVIOR_CURLFIELD  = 1;
inline constexpr uint32_t CUBE_BEHAVIOR_PHASEWAVE  = 2;
// CUBE_BEHAVIOR_COUNT_WGSL exists on the WGSL side (world.wgsl §7
// cube behavior registry). MUST match this value — when adding a
// behavior, bump both. Mirrors the agents pattern
// (AGENT_BEHAVIOR_COUNT_WGSL).
inline constexpr uint32_t CUBE_BEHAVIOR_COUNT      = 3;

inline constexpr const char* CUBE_BEHAVIOR_NAMES[CUBE_BEHAVIOR_COUNT] = {
    "stationary", "curlfield", "phasewave"
};

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

// ─ Substrate (drift integrator) ─────────────────────────────────

inline constexpr float CUBE_DEFAULT_SPRING_STIFFNESS = 4.0f;   // 1/s², ~0.5s settle
inline constexpr float CUBE_DEFAULT_DRAG             = 1.5f;   // 1/s,  gentle damping

// ─ Diagnostics (corral) ─────────────────────────────────────────

inline constexpr float CUBE_CORRAL_RADIUS   = 120.0f;  // ring radius around pawn (world units)
inline constexpr float CUBE_CORRAL_DURATION = 4.0f;    // glide duration (seconds)

// ─ Diagnostics (coordination cycle) ─────────────────────────────

inline constexpr float FLOATER_COORDINATION_STEPS[3] = { 0.0f, 0.5f, 1.0f };

// ─ WGSL kernel constants (NOT here, but documented) ─────────────
//
//   cube_force_phasewave (world.wgsl):
//     k_x         = 0.020    wavefront freq in X
//     k_z         = 0.012    wavefront freq in Z (asymmetric)
//     omega       = 1.5      1/s, temporal frequency
//     amplitude   = 30.0     force magnitude (vertical)

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════

struct CubeTierGain {
    uint32_t tier_idx;
    const char* name;
    float spring_stiffness_mult;
    float drag_mult;
    float behavior_amp_mult;   // reserved; not yet consumed by kernel
};

inline constexpr CubeTierGain CUBE_TIER_GAINS[CUBE_TIER_COUNT] = {
    //                            spring  drag   amp
    /* 0 SmallCube */ { 0, "SmallCube", 1.0f, 1.0f, 1.0f },
    /* 1 MedCube   */ { 1, "MedCube",   1.0f, 1.0f, 1.0f },
    /* 2 LargeCube */ { 2, "LargeCube", 1.0f, 1.0f, 1.0f },
    /* 3 Monolith  */ { 3, "Monolith",  1.0f, 1.0f, 1.0f },
};

static_assert(sizeof(CUBE_TIER_GAINS) / sizeof(CUBE_TIER_GAINS[0]) == CUBE_TIER_COUNT,
              "CUBE_TIER_GAINS must declare one row per cube tier");

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════
//
// Mood ordering matches MOOD_TABLE in cartridge.hpp. Cubes are gated
// by CubeConfig::MOOD_MULTIPLIER (in floater_vocabulary.hpp) which is
// {1, 1, 0, 0, 1, 0} — cubes don't spawn in indoor moods or in
// MOOD_FINITE_OUTDOOR_REF, so those rows here are never consulted in
// practice. We declare them anyway for hygiene; if the spawn gate
// ever changes, the populations will already exist.

struct CubePopulationDef {
    uint32_t mood_id;
    std::array<float, CUBE_BEHAVIOR_COUNT> behavior_weights;
};

inline constexpr CubePopulationDef CUBE_POPULATIONS[MOOD_COUNT] = {
    /* MOOD_OPEN_DEFAULT */
    { MOOD_OPEN_DEFAULT,
      //                    stat curl  wave
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_OPEN_SUNSET */
    { MOOD_OPEN_SUNSET,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_INDOOR_FLAT — cubes don't spawn here; row exists for hygiene */
    { MOOD_INDOOR_FLAT,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_INDOOR_VAULT — cubes don't spawn here; row exists for hygiene */
    { MOOD_INDOOR_VAULT,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_FINITE_OUTDOOR */
    { MOOD_FINITE_OUTDOOR,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_FINITE_OUTDOOR_REF — cubes don't spawn here; row exists for hygiene */
    { MOOD_FINITE_OUTDOOR_REF,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
};

static_assert(sizeof(CUBE_POPULATIONS) / sizeof(CUBE_POPULATIONS[0]) == MOOD_COUNT,
              "CUBE_POPULATIONS must declare one row per mood");
static_assert(CUBE_POPULATIONS[0].mood_id == MOOD_OPEN_DEFAULT,        "row 0 must be MOOD_OPEN_DEFAULT");
static_assert(CUBE_POPULATIONS[1].mood_id == MOOD_OPEN_SUNSET,         "row 1 must be MOOD_OPEN_SUNSET");
static_assert(CUBE_POPULATIONS[2].mood_id == MOOD_INDOOR_FLAT,         "row 2 must be MOOD_INDOOR_FLAT");
static_assert(CUBE_POPULATIONS[3].mood_id == MOOD_INDOOR_VAULT,        "row 3 must be MOOD_INDOOR_VAULT");
static_assert(CUBE_POPULATIONS[4].mood_id == MOOD_FINITE_OUTDOOR,      "row 4 must be MOOD_FINITE_OUTDOOR");
static_assert(CUBE_POPULATIONS[5].mood_id == MOOD_FINITE_OUTDOOR_REF,  "row 5 must be MOOD_FINITE_OUTDOOR_REF");

// ═══ DIAGNOSTIC STATE TYPES ══════════════════════════════════════

struct CubeCorralAnim {
    bool   active;
    float  t0;
    float  duration;
    float  ax_from, az_from;
    float  ax_to,   az_to;
};

// ═══ DIAGNOSTIC STATE (owned by the tools) ═══════════════════════

struct CubeBehaviorsState {
    uint32_t       coordination_step  = 0;
    uint32_t       behavior_override  = CUBE_BEHAVIOR_STATIONARY;
    CubeCorralAnim corral_anim[Dim::MAX_CUBE_INSTANCES] = {};
    bool           kite_mode          = false;
    float          pawn_offset[Dim::MAX_CUBE_INSTANCES][2] = {};  // xz only
    ActiveCube     activeCubes_[Dim::MAX_CUBE_INSTANCES]{};
    uint32_t       activeCubeCount_ = 0;
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Spawn-side (stateless — consumed by entity_pipeline.inl's cube_write_gpu)
void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx);
uint32_t pick_cube_behavior_for_spawn(uint32_t mood_id, uint32_t seed);
// Teardown owner-clear
void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue);
// The evictor — keyhole-shaped
// to match the FAMILY_DISPATCH evict slot (table in family_dispatch.inl)
void evict_cube(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
// Dispatch funnels (table-shaped; defined in cube_behaviors.inl beside the recipe)
bool dispatch_select_cube_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_cube_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_cube_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);
// Player commands
void cycle_floater_coordination(CubeBehaviorsState& cbs, Cartridge* c);
void cycle_cube_behavior_override(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);
void corral_cubes(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);
void toggle_cube_kite_mode(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);
// Per-frame
void tick_cube_corral_animations(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
