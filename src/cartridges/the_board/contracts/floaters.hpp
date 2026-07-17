#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT (sizes the configs' MOOD_MULTIPLIER)

// ─── floaters.hpp ────────────────────────────────────────────────
// History: audit/LADDER.md
//
// Vocabulary for the two generic-pipeline floater families: Sphere
// (orbital, PGA motor-driven) and Cube (hover-bob monoliths).
//
// Renamed (this sweep): props are numeric at every hash site, so
// identifier names never feed seeds — the values below are the
// frozen contract, pinned by the static_assert.
//
// SEAM[floaters:taxonomy] generic-pipeline floater families
//   parallel grounded families (grounded.hpp) but live here because
//   their tier shapes differ (sphere has orbit_radius/orbit_speed,
//   cube doesn't). Three concerns, three files: vocabulary here,
//   sampling profile in machine/entity_pipeline.hpp, cube behavior in
//   bodies/cube_behaviors.hpp. Spheres have no behavior layer.
// SEAM[sphere:taxonomy] sphere VOCABULARY lives here, not in
//   grounded.hpp. Generic-pipeline floater family — vocabulary class
//   distinct from grounded families.
// SEAM[cube:taxonomy] cube VOCABULARY lives here, not in grounded.hpp.
//   Three concerns, three files (vocabulary / sampling profile /
//   behavior gains), each correct.
// SEAM[sphere:P5] last_alloc_time is pattern P5 (release-pending sentinel /
//   race protection) — CPU-timestamp variant. When GPU readback arrives
//   stale ("kernel evicted this slot"), the timestamp protects freshly-
//   allocated slots from being incorrectly marked inactive. Same intent as
//   cube_behaviors.hpp::toggle_cube_kite_mode's GPU sentinel; different
//   mechanism.
// SEAM[cube:cx-cz-mirror] ActiveCube has cx, cz fields — CPU mirror of GPU
//   anchor for cube_behaviors.hpp::corral_cubes / toggle_cube_kite_mode to
//   read without GPU readback. Same family as agents:D2 (slot-0 reads);
//   when the pawn module provides accessors, corral/kite could analogously
//   have cube_anchor(slot) accessors.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ FAMILY: SPHERE ═══════════════════════════════════════════════

// ── Tier registry ────────────────────────────────────────────────
inline constexpr uint32_t SPHERE_TIER_COUNT = 2;

inline constexpr float SPHERE_BASE_TIER_WEIGHTS[SPHERE_TIER_COUNT] = { 0.65f, 0.35f };
inline constexpr const char* SPHERE_TIER_NAMES[] = { "Sentinel", "Anomaly" };

// ── Readback spawn-protection window (shared sphere/cube vocabulary) ──
inline constexpr float SPAWN_PROTECTION_S = 0.10f;

// ── Spawn Configuration ──────────────────────────────────────────
struct SphereConfig {
    static constexpr float SPAWN_CHANCE = 0.015f;
    static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };  // per-mood spawn ×: open sunset ind_flat ind_vault finite fin_ref (mood_constants order)
    static constexpr float POSITION_JITTER = 0.4f;
};

// ── Property Index Registry ──────────────────────────────────────
struct SphereProp {
    static constexpr uint32_t SPAWN_ROLL = 100u;
    static constexpr uint32_t ANCHOR_X = 101u;
    static constexpr uint32_t ANCHOR_Z = 102u;
    static constexpr uint32_t TIER = 103u;
    static constexpr uint32_t BODY_RADIUS = 110u;
    static constexpr uint32_t ORBIT_RADIUS = 111u;
    static constexpr uint32_t ORBIT_HEIGHT = 112u;
    static constexpr uint32_t ORBIT_SPEED = 113u;
    static constexpr uint32_t INFLUENCE_RADIUS = 114u;
    static constexpr uint32_t SPIN_SPEED = 115u;
    static constexpr uint32_t BOB_AMPLITUDE = 116u;
    static constexpr uint32_t BOB_PERIOD = 117u;
    static constexpr uint32_t SPIN_TILT_X = 118u;
    static constexpr uint32_t SPIN_TILT_Z = 119u;
    static constexpr uint32_t COLOR_R = 120u;
    static constexpr uint32_t COLOR_G = 121u;
    static constexpr uint32_t COLOR_B = 122u;
    static constexpr uint32_t ASPECT_Y = 123u;
    static constexpr uint32_t ASPECT_Z = 124u;
    static constexpr uint32_t FACE_VARIANCE = 125u;
    static constexpr uint32_t ROTATION = 126u;
};
static_assert(SphereProp::SPAWN_ROLL == 100u && SphereProp::ANCHOR_X == 101u
           && SphereProp::ANCHOR_Z == 102u && SphereProp::TIER == 103u,
    "SphereProp values are the sphere seed contract — frozen biography");

// ── Active Sphere Tracking (TYPE — state lives in spheres.hpp) ───
struct ActiveSphere {
    int32_t patch_gx = 0, patch_gz = 0;
    int32_t host_gx = 0, host_gz = 0;
    float   last_alloc_time = -1000.0f;
    bool active = false;
};

// ═══ FAMILY: CUBE ═════════════════════════════════════════════════

// ── Tier registry ────────────────────────────────────────────────
inline constexpr uint32_t CUBE_TIER_COUNT = 4;

inline constexpr float CUBE_BASE_TIER_WEIGHTS[CUBE_TIER_COUNT] = { 0.40f, 0.32f, 0.20f, 0.08f };
inline constexpr const char* CUBE_TIER_NAMES[] = { "SmallCube", "MedCube", "LargeCube", "Monolith" };

// ── Spawn Configuration ──────────────────────────────────────────
struct CubeConfig {
    static constexpr float SPAWN_CHANCE = 0.60f;
    static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };  // per-mood spawn ×: open sunset ind_flat ind_vault finite fin_ref (mood_constants order)
    static constexpr float POSITION_JITTER = 0.4f;
};

// ── Property Index Registry ──────────────────────────────────────
// Range: 130–156 (avoids sphere's 100–126).
struct CubeProp {
    static constexpr uint32_t SPAWN_ROLL = 130u;
    static constexpr uint32_t ANCHOR_X = 131u;
    static constexpr uint32_t ANCHOR_Z = 132u;
    static constexpr uint32_t TIER = 133u;
    static constexpr uint32_t BODY_RADIUS = 140u;
    static constexpr uint32_t ORBIT_HEIGHT = 142u;
    static constexpr uint32_t INFLUENCE_RADIUS = 144u;
    static constexpr uint32_t SPIN_SPEED = 145u;
    static constexpr uint32_t BOB_AMPLITUDE = 146u;
    static constexpr uint32_t BOB_PERIOD = 147u;
    static constexpr uint32_t SPIN_TILT_X = 148u;
    static constexpr uint32_t SPIN_TILT_Z = 149u;
    static constexpr uint32_t COLOR_R = 150u;
    static constexpr uint32_t COLOR_G = 151u;
    static constexpr uint32_t COLOR_B = 152u;
    static constexpr uint32_t ASPECT_Y = 153u;
    static constexpr uint32_t ASPECT_Z = 154u;
    static constexpr uint32_t FACE_VARIANCE = 155u;
    static constexpr uint32_t ROTATION = 156u;
};
static_assert(CubeProp::SPAWN_ROLL == 130u && CubeProp::ANCHOR_X == 131u
           && CubeProp::ANCHOR_Z == 132u && CubeProp::TIER == 133u,
    "CubeProp values are the cube seed contract — frozen biography");

// ── Active Cube Tracking (TYPE — state lives in CubeBehaviorsState) ─
struct ActiveCube {
    int32_t patch_gx = 0, patch_gz = 0;
    int32_t host_gx = 0, host_gz = 0;
    float   cx = 0.0f, cz = 0.0f;
    float   last_alloc_time = -1000.0f;
    bool active = false;
};

} // namespace the_board
} // namespace t7
