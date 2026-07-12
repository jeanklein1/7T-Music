#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/state.hpp"                       // Dim::MAX_CUBE_INSTANCES, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/modules/floater_vocabulary.hpp"  // ActiveCube, CUBE_TIER_COUNT
#include "cartridges/the_board/modules/mood_constants.hpp"      // MOOD_COUNT + the Mood IDs
#include "cartridges/the_board/modules/keyhole.hpp"             // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── cube_behaviors.hpp (HEADER: registries + console + state + decls) ─
// Converted (LADDER-3 c3): history in audit/LADDER.md.
//
// Cube behavior system. Spheres do their own thing (analytical PGA
// orbit, no behavior layer); this module is cube-only by design.
// Floater vocabulary (Sphere + Cube tier counts, configs, prop
// registries, active-tracking types) lives in floater_vocabulary.hpp;
// behavior gains (forces, coordination, kite, corral) live here.
//
// The cube_behaviors → cubes rename stays FLAGGED, not performed.
//
// ┌─── Three registries ────────────────────────────────────────────┐
// │                                                                  │
// │  CUBE_BEHAVIORS     id + name table; force functions live in     │
// │                     world.wgsl as cube_force_<n> + a switch      │
// │                     case in cube_behavior_force                  │
// │  CUBE_TIER_GAINS    per-tier multipliers on spring/drag/forces;  │
// │                     applied at spawn so each tier gets its own   │
// │                     baked-in dynamics signature                  │
// │  CUBE_POPULATIONS   per-mood weights across behaviors; consulted │
// │                     at spawn to assign behavior_id stochastically│
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// ┌─── The four-axis vocabulary (from the design doc) ──────────────┐
// │                                                                  │
// │  LATTICE     where cubes can be (anchor xz, pawn-relative)       │
// │  COUPLING    how cubes relate to each other (shared field        │
// │              samples vs independent, shared phase vs random)     │
// │  TRAJECTORY  what each cube does over time (the behaviors)       │
// │  TIER        which "weight class" the cube belongs to            │
// │                                                                  │
// │ Coordination ∈ [0,1] is the system-wide knob that slides each    │
// │ behavior between high-individual and high-coupled regimes.       │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// ┌─── Public surface (called from outside this module) ────────────┐
// │                                                                  │
// │  Module functions (other than the two pure spawn helpers below)  │
// │  take CubeBehaviorsState& explicitly. Spawn helpers are           │
// │  stateless and stay free.                                        │
// │                                                                  │
// │  Spawn-side (stateless, no struct ref needed):                   │
// │    pick_cube_behavior_for_spawn(mood, seed)  → behavior_id u32   │
// │    apply_cube_tier_gains(spring, drag, tier) → adjusted (sp, dr) │
// │      Both consumed by entity_pipeline.inl's cube_write_gpu.      │
// │                                                                  │
// │  Teardown:                                                       │
// │    clear_cubes(cbs, gpu, queue)   — teardown owner clear         │
// │                                                                  │
// │  Per-frame:                                                      │
// │    tick_cube_corral_animations(cbs, c, queue)                    │
// │      Called from render(); advances any in-flight glides.        │
// │                                                                  │
// │  Player commands (function keys; chosen to avoid the A-Z piano   │
// │                   range the analysis layer consumes):            │
// │    cycle_cube_behavior_override(cbs, c, q)  F4  next behavior    │
// │    cycle_floater_coordination(cbs, c)       F5  step coordination│
// │    corral_cubes(cbs, c, q)                  F6  ring around pawn │
// │    toggle_cube_kite_mode(cbs, c, q)         F7  follow pawn      │
// │                                                                  │
// │  Owned state: activeCubes_[] / activeCubeCount_                   │
// │    live in CubeBehaviorsState below (was floater_vocabulary).    │
// │  Cross-module reads (this module reads, doesn't own):            │
// │    agent_state_.slots, player_.possessed_slot                    │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// Depends on: state.hpp (Dim::MAX_CUBE_INSTANCES, GPUState,
// GPUFloatingEntityState, GPUDesignConfig::floater_coordination),
// floater_vocabulary.hpp (ActiveCube, CUBE_TIER_COUNT),
// mood_constants.hpp (MOOD_COUNT + Mood IDs). The impl additionally
// hashes with seed_utils and reaches the keyhole;
// entity_pipeline.inl's cube_write_gpu calls into the two spawn helpers;
// world.wgsl holds the force functions and dispatch switch.
// ──────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ BEHAVIOR IDS ════════════════════════════════════════════════
//
// Mirror the WGSL constants in world.wgsl (search "Cube behavior
// registry"). Adding a behavior is four steps:
//   1) Add an id constant here and bump CUBE_BEHAVIOR_COUNT
//   2) Add the name in CUBE_BEHAVIOR_NAMES
//   3) Add a const + force function + switch case in world.wgsl
//   4) Populate CUBE_POPULATIONS rows with a non-zero weight where
//      the behavior should appear

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
//
// One place for every system-level number that shapes the cube
// system's feel. WGSL kernel constants (force amplitudes, noise
// frequencies, wave wavelengths) cannot be moved here — they're
// shader literals — but they're documented below as the source of
// truth for tuning. If you ever want CPU-side adjustability, promote
// them to GPUDesignConfig fields and read them from the kernel.

// ─ Substrate (drift integrator) ─────────────────────────────────
// Cube tuning constants live here so the file is the one place to
// look when adjusting cube dynamics. Each spawn multiplies these by
// the row's tier gain (see CUBE_TIER_GAINS) so different tiers get
// distinct dynamics signatures from the same defaults.

inline constexpr float CUBE_DEFAULT_SPRING_STIFFNESS = 4.0f;   // 1/s², ~0.5s settle
inline constexpr float CUBE_DEFAULT_DRAG             = 1.5f;   // 1/s,  gentle damping

// ─ Diagnostics (corral) ─────────────────────────────────────────
// Used by F6 to glide every active cube into a ring around the pawn.
// Kite mode (F7) doesn't change the radius/duration — it just changes
// whether the ring is in world or pawn-relative space.

inline constexpr float CUBE_CORRAL_RADIUS   = 120.0f;  // ring radius around pawn (world units)
inline constexpr float CUBE_CORRAL_DURATION = 4.0f;    // glide duration (seconds)

// ─ Diagnostics (coordination cycle) ─────────────────────────────
// F5 steps the system-wide coordination knob through these values.
// floater_coordination is uploaded to GPUDesignConfig each frame.

inline constexpr float FLOATER_COORDINATION_STEPS[3] = { 0.0f, 0.5f, 1.0f };

// ─ WGSL kernel constants (NOT here, but documented) ─────────────
//
//   cube_force_curlfield (world.wgsl):
//     freq_high   = 0.040    high spatial frequency at coordination=0
//     freq_low    = 0.005    low  spatial frequency at coordination=1
//     amplitude   = 12.0     force magnitude (≈ 10× spring)
//     time_scale  = 0.25     1/s, evolution rate
//
//   cube_force_phasewave (world.wgsl):
//     k_x         = 0.020    wavefront freq in X
//     k_z         = 0.012    wavefront freq in Z (asymmetric)
//     omega       = 1.5      1/s, temporal frequency
//     amplitude   = 30.0     force magnitude (vertical)
//
// To change these, edit world.wgsl. Promote to config if you need
// CPU-side adjustability (~16 bytes of uniform space + 4 lines).

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════
//
// Per-tier multipliers applied at spawn time. The cube's stored
// spring_stiffness and drag are CUBE_DEFAULT_* × tier gain, so each
// tier gets a baked-in dynamics signature without runtime branching.
//
// behavior_amp_mult is reserved for future use — the kernel applies
// forces uniformly today. To wire it up, kernels would need to read
// tier_idx and multiply force outputs. Defer until a behavior demands
// per-tier amplitude differentiation.
//
// Defaults are all 1.0 (no per-tier differentiation). Character pass
// will populate with real values.

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
// Per-mood weights across the three behaviors. Each spawn picks a
// behavior stochastically by sampling these weights. Weights need not
// sum to 1; the picker normalizes.
//
// Mood ordering matches MOOD_TABLE in cartridge.hpp. Cubes are gated
// by CubeConfig::MOOD_MULTIPLIER (in floater_vocabulary.hpp) which is
// {1, 1, 0, 0, 1, 0} — cubes don't spawn in indoor moods or in
// MOOD_FINITE_OUTDOOR_REF, so those rows here are never consulted in
// practice. We declare them anyway for hygiene; if the spawn gate
// ever changes, the populations will already exist.
//
// Defaults are all-Stationary. Character pass will populate with
// real per-mood character.

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
//
// (The diagnostics themselves are documented at their definitions in
// cube_behaviors.inl: coordination cycle F5, behavior override F4,
// corral animation F6, kite mode F7.)

struct CubeCorralAnim {
    bool   active;
    float  t0;
    float  duration;
    float  ax_from, az_from;
    float  ax_to,   az_to;
};

// ═══ DIAGNOSTIC STATE (owned by the tools) ═══════════════════════
//
// All diagnostic-tool state lives in this struct, accessed via
// cube_behaviors_state_ on the Cartridge (declared at the composition
// root). Module functions take `CubeBehaviorsState& cbs` explicitly
// rather than reaching via Cartridge*, making ownership language-
// visible and dependencies explicit in signatures.

struct CubeBehaviorsState {
    uint32_t       coordination_step  = 0;
    uint32_t       behavior_override  = CUBE_BEHAVIOR_STATIONARY;
    CubeCorralAnim corral_anim[Dim::MAX_CUBE_INSTANCES] = {};
    bool           kite_mode          = false;
    float          pawn_offset[Dim::MAX_CUBE_INSTANCES][2] = {};  // xz only
    // The behavior layer owns cube runtime state; the cube active-slot
    // mirror (activeCubes_[] / activeCubeCount_) lives here with it.
    // ActiveCube stays shared vocabulary (floater_vocabulary.hpp).
    ActiveCube     activeCubes_[Dim::MAX_CUBE_INSTANCES]{};
    uint32_t       activeCubeCount_ = 0;
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════
//
// DEFINED in cube_behaviors.inl (post-class, self-wrapping).
// apply_cube_behavior_override and corral_ease_ are module-internal
// helpers (impl-only, not declared here).

// Spawn-side (stateless — consumed by entity_pipeline.inl's cube_write_gpu)
void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx);
uint32_t pick_cube_behavior_for_spawn(uint32_t mood_id, uint32_t seed);
// Teardown owner-clear
void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue);
// The evictor — lifecycle, absorbed per §5 EVICTION THUNKS; keyhole-shaped
// to match the FAMILY_DISPATCH evict slot (table in family_dispatch.inl)
void evict_cube(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
// Player commands
void cycle_floater_coordination(CubeBehaviorsState& cbs, Cartridge* c);
void cycle_cube_behavior_override(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);
void corral_cubes(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);
void toggle_cube_kite_mode(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);
// Per-frame
void tick_cube_corral_animations(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
