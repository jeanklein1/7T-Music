#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/realization/state.hpp"                       // Dim::MAX_CUBE_INSTANCES, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/contracts/floaters.hpp"  // ActiveCube, CUBE_TIER_COUNT
#include "cartridges/the_board/contracts/mood_constants.hpp"      // MOOD_COUNT + the Mood IDs
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"   // queue types (the funnel signatures)

// ─── cube_behaviors.hpp (HEADER: registries + console + state + decls) ─
// History: audit/LADDER.md
//
// Cube behavior system.
//
// The impl additionally hashes with seed_utils; the cube recipe
// (below) calls into the two spawn helpers; world.wgsl holds the
// force functions and dispatch switch.
// ──────────────────────────────────────────────────────────────────

#include <cmath>      // std::cos, std::sin   // (impl, merged)
#include <algorithm>  // std::clamp   // (impl, merged)
#include <iostream>   // diagnostics feedback   // (impl, merged)

namespace t7 {
namespace the_board {

// ═══ MODULE DEPS ════════════════════════════════════════════════════
// The cube commands' requirements face: corral/kite center on THE
// POINT through the witness record (readback_x/z — the agent
// slot reach retired with it); all reads except the GPU wire.
struct CubeDeps {
    GPUState&        gpuState_;
    const TimeState& time_state_;
    const PlayerState& player_;
    const MoodState& mood_state_;
};

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

inline constexpr float CUBE_CORRAL_RADIUS   = 120.0f;  // ring radius around the point (world units)
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
    float plasticity;          // CONTACT_2 λ: 0 = elastic (today, bit-exact);
                               // 1 = fully sculptable. Jean-tunable.
};

inline constexpr CubeTierGain CUBE_TIER_GAINS[CUBE_TIER_COUNT] = {
    //                            spring  drag   amp   λ (plasticity — Jean-tunable)
    /* 0 SmallCube */ { 0, "SmallCube", 1.0f, 1.0f, 1.0f, 0.0f },
    /* 1 MedCube   */ { 1, "MedCube",   1.0f, 1.0f, 1.0f, 0.0f },
    /* 2 LargeCube */ { 2, "LargeCube", 1.0f, 1.0f, 1.0f, 0.0f },
    /* 3 Monolith  */ { 3, "Monolith",  1.0f, 1.0f, 1.0f, 0.0f },
};

static_assert(sizeof(CUBE_TIER_GAINS) / sizeof(CUBE_TIER_GAINS[0]) == CUBE_TIER_COUNT,
              "CUBE_TIER_GAINS must declare one row per cube tier");

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════
//
// Mood ordering matches MOOD_TABLE in cartridge.hpp. Cubes are gated
// by mood_mult_for(PopFamily::CUBE) (in floaters.hpp) which is
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

// Spawn-side (stateless — consumed by entity_pipeline.hpp's cube_write_gpu)
void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx);
uint32_t pick_cube_behavior_for_spawn(uint32_t mood_id, uint32_t seed);
// Teardown owner-clear
void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue);  // DEPS-FORM PRECEDENT: explicit GPUState& param, born-converted
// The evictor — MachineCtx-shaped
// to match the FAMILY_DISPATCH evict slot (table in cartridge.hpp, post-class)
void evict_cube(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
// Dispatch funnels (table-shaped; defined below beside the recipe)
bool dispatch_select_cube_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_cube_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_cube_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue);
// Player commands
void cycle_floater_coordination(CubeBehaviorsState& cbs, CubeDeps* c);
void cycle_cube_behavior_override(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
void corral_cubes(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
void toggle_cube_kite_mode(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
// Per-frame
void tick_cube_corral_animations(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data);

// ═══ IMPL:
// rows deref cube_state(own) + mood/time/world via MachineCtx; corral/kite
// read AgentState + player_ via CubeDeps. COHORT: after agents (AgentState)
// + entity_pipeline (generic_*) + spawn_engine (preamble) + mood/state.

// Apply tier gains to base substrate values. Called by cube_write_gpu
// during spawn — the result is what gets stored on the cube.
inline void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx) {
    if (tier_idx >= CUBE_TIER_COUNT) return;  // defensive; tier_idx is bounded at select
    const auto& g = CUBE_TIER_GAINS[tier_idx];
    spring_stiffness *= g.spring_stiffness_mult;
    drag             *= g.drag_mult;
}

//
inline uint32_t pick_cube_behavior_for_spawn(uint32_t mood_id, uint32_t seed) {
    if (mood_id >= MOOD_COUNT) return CUBE_BEHAVIOR_STATIONARY;
    const auto& pop = CUBE_POPULATIONS[mood_id];

    float total = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) total += pop.behavior_weights[i];
    if (total <= 0.0f) return CUBE_BEHAVIOR_STATIONARY;

    uint32_t h = cpu_hash(seed, 0xBEEF11A0u);
    float r = (float(h) / 4294967296.0f) * total;

    float acc = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) {
        acc += pop.behavior_weights[i];
        if (r < acc) return i;
    }
    // Numerical edge case (r == total exactly): return last non-zero.
    for (uint32_t i = CUBE_BEHAVIOR_COUNT; i > 0; i--) {
        if (pop.behavior_weights[i - 1] > 0.0f) return i - 1;
    }
    return CUBE_BEHAVIOR_STATIONARY;  // unreachable; total > 0 was checked
}

// ═══ DIAGNOSTICS ═════════════════════════════════════════════════
//
// ─── Coordination cycle ─────────────────────────────────────────────
//
// ─── Behavior override ──────────────────────────────────────────────
//
// ─── Corral animation ──────────────────────────────────────────────
//
// ─── Kite mode ─────────────────────────────────────────────────────
//
// Toggle on  — captures each cube's current xz as a point-relative
// offset (the kite leashes to the point). Cube xz is preserved exactly; y resets to pawn.y +
// orbit_height (small visible jump only if pawn's altitude differs
// from where the cube was hovering).

// DEPS-FORM PRECEDENT: explicit GPUState& parameter —
// the deps form's first citizen; not a MachineCtx bypass.
inline void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        cbs.activeCubes_[i] = ActiveCube{};
        GPUFloatingEntityState empty{};
        gpu.upload_cube_entity_slot(queue, i, empty);
    }
    cbs.activeCubeCount_ = 0;
}

inline float corral_ease_(float t) {
    return t * t * (3.0f - 2.0f * t);  // smoothstep
}

inline void cycle_floater_coordination(CubeBehaviorsState& cbs, CubeDeps* c) {
    cbs.coordination_step = (cbs.coordination_step + 1) % 3;
    float v = FLOATER_COORDINATION_STEPS[cbs.coordination_step];
    c->gpuState_.stage_floater_coordination(v);
    std::cout << "[Floaters] coordination: " << v << "\n";
}

inline void apply_cube_behavior_override(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        c->gpuState_.upload_cube_behavior_id(queue, i, cbs.behavior_override);
    }
}

inline void cycle_cube_behavior_override(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    cbs.behavior_override = (cbs.behavior_override + 1) % CUBE_BEHAVIOR_COUNT;
    apply_cube_behavior_override(cbs, c, queue);
    std::cout << "[Floaters] cube behavior: "
              << CUBE_BEHAVIOR_NAMES[cbs.behavior_override] << "\n";
}

inline void corral_cubes(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    (void)queue;
    // THE POINT: the corral ring forms around the point —
    // readback_x/z, host-authored (pawn-host value-identical: same
    // P5 harvest snapshot as the slot mirror).
    const float px = c->player_.readback_x;
    const float pz = c->player_.readback_z;

    uint32_t active_count = 0;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (cbs.activeCubes_[i].active) active_count++;
    }
    if (active_count == 0) {
        std::cout << "[Floaters] corral: no active cubes\n";
        return;
    }

    uint32_t armed = 0;
    uint32_t k = 0;
    const float two_pi = 6.28318530718f;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        float theta = (float(k) / float(active_count)) * two_pi;
        float target_x, target_z;
        if (cbs.kite_mode) {
            target_x = std::cos(theta) * CUBE_CORRAL_RADIUS;
            target_z = std::sin(theta) * CUBE_CORRAL_RADIUS;
        } else {
            target_x = px + std::cos(theta) * CUBE_CORRAL_RADIUS;
            target_z = pz + std::sin(theta) * CUBE_CORRAL_RADIUS;
        }

        // Start position: current interpolated value if a previous
        // animation is in flight, else the CPU mirror.
        float from_x, from_z;
        if (cbs.corral_anim[i].active) {
            float t_norm = (c->time_state_.seconds - cbs.corral_anim[i].t0)
                           / cbs.corral_anim[i].duration;
            t_norm = std::clamp(t_norm, 0.0f, 1.0f);
            float eased = corral_ease_(t_norm);
            from_x = cbs.corral_anim[i].ax_from
                    + (cbs.corral_anim[i].ax_to - cbs.corral_anim[i].ax_from) * eased;
            from_z = cbs.corral_anim[i].az_from
                    + (cbs.corral_anim[i].az_to - cbs.corral_anim[i].az_from) * eased;
        } else if (cbs.kite_mode) {
            from_x = cbs.pawn_offset[i][0];
            from_z = cbs.pawn_offset[i][1];
        } else {
            from_x = cbs.activeCubes_[i].cx;
            from_z = cbs.activeCubes_[i].cz;
        }

        cbs.corral_anim[i] = {
            /*active=*/   true,
            /*t0=*/       c->time_state_.seconds,
            /*duration=*/ CUBE_CORRAL_DURATION,
            from_x, from_z,
            target_x, target_z,
        };
        if (cbs.kite_mode) {
            cbs.pawn_offset[i][0] = target_x;
            cbs.pawn_offset[i][1] = target_z;
        } else {
            cbs.activeCubes_[i].cx = target_x;
            cbs.activeCubes_[i].cz = target_z;
        }
        armed++;
        k++;
    }

    std::cout << "[Floaters] corral: " << armed << " cube(s) gliding "
              << (cbs.kite_mode ? "to ring offset around the point" : "to ring")
              << " radius " << CUBE_CORRAL_RADIUS
              << " over " << CUBE_CORRAL_DURATION << "s\n";
}

inline void tick_cube_corral_animations(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        auto& anim = cbs.corral_anim[i];
        if (!anim.active) continue;
        if (!cbs.activeCubes_[i].active) { anim.active = false; continue; }

        float t_norm = (c->time_state_.seconds - anim.t0) / anim.duration;
        bool finishing = false;
        if (t_norm >= 1.0f) { t_norm = 1.0f; finishing = true; }
        if (t_norm < 0.0f) t_norm = 0.0f;
        float eased = corral_ease_(t_norm);
        float ax = anim.ax_from + (anim.ax_to - anim.ax_from) * eased;
        float az = anim.az_from + (anim.az_to - anim.az_from) * eased;
        if (cbs.kite_mode) {
            c->gpuState_.upload_cube_pawn_offset(queue, i, ax, 0.0f, az);
        } else {
            c->gpuState_.upload_cube_anchor(queue, i, ax, 0.0f, az);
        }
        if (finishing) anim.active = false;
    }
}

// ─── Kite mode toggle (F7) ──────────────────────────────────────

inline void toggle_cube_kite_mode(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    cbs.kite_mode = !cbs.kite_mode;
    // THE POINT: the kite leashes to the point — the offset
    // capture here and the GPU kite home (update_cube) moved in
    // LOCK-STEP, so the F7 toggle still preserves world position
    // exactly. Pawn-host value-identical (same harvest snapshot).
    const float px = c->player_.readback_x;
    const float pz = c->player_.readback_z;

    uint32_t affected = 0;
    if (cbs.kite_mode) {
        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
            if (!cbs.activeCubes_[i].active) continue;
            float ox = cbs.activeCubes_[i].cx - px;
            float oz = cbs.activeCubes_[i].cz - pz;
            cbs.pawn_offset[i][0] = ox;
            cbs.pawn_offset[i][1] = oz;
            c->gpuState_.upload_cube_pawn_offset(queue, i, ox, 0.0f, oz);
            c->gpuState_.upload_cube_follow_pawn(queue, i, 1u);
            affected++;
        }
    } else {
        // Toggle OFF: rather than write anchor on CPU (which can't see
        // the cube's drift, so anchor would land at home rather than
        // at the cube's visible position), we hand the work to the
        // kernel. follow_pawn = 2u is a "release pending" sentinel:
        // the next update_cube run sees it, sets anchor = current pos,
        // zeroes drift, switches follow_pawn back to 0. Visible
        // position is preserved exactly — no CPU drift estimation
        // needed, no readback latency.
        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
            if (!cbs.activeCubes_[i].active) continue;
            float ax = px + cbs.pawn_offset[i][0];
            float az = pz + cbs.pawn_offset[i][1];
            cbs.activeCubes_[i].cx = ax;
            cbs.activeCubes_[i].cz = az;
            c->gpuState_.upload_cube_follow_pawn(queue, i, 2u);
            affected++;
        }
    }

    std::cout << "[Floaters] kite mode: " << (cbs.kite_mode ? "ON" : "OFF")
              << " (" << affected << " cube(s))\n";
}

// ═══ THE EVICTOR ══════════════════════════════════════════════════

inline void evict_cube(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->cube_behaviors_state_.activeCubes_[slot].active = false;  // cube state owned by CubeBehaviorsState
    self->cube_behaviors_state_.activeCubeCount_--;
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_cube_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   cube slot=" << slot << "\n";
#endif
}

// ═══ THE CUBE RECIPE ══════════════════════════════════════════════
//
// Tier tables, traits, adapter, and dispatch funnels — beside the
// evictor. Funnels declared above; the FAMILY_DISPATCH rows
// (cartridge.hpp, post-class) point here. THEMES is reached as THEMES
// (INTENT[services:themes] at its definition).

// ═══ FAMILY: CUBE ═════════════════════════════════════════════════

struct CubeIdx {
    static constexpr uint32_t BODY_RADIUS      = 0;
    static constexpr uint32_t ORBIT_HEIGHT     = 1;
    static constexpr uint32_t INFLUENCE_RADIUS = 2;
    static constexpr uint32_t SPIN_SPEED       = 3;
    static constexpr uint32_t BOB_AMPLITUDE    = 4;
    static constexpr uint32_t BOB_PERIOD       = 5;
    static constexpr uint32_t ASPECT_Y         = 6;
    static constexpr uint32_t ASPECT_Z         = 7;
    static constexpr uint32_t FACE_VARIANCE    = 8;
    static constexpr uint32_t COUNT            = 9;
};

inline constexpr TierParamDef CUBE_PARAM_DEFS[] = {
    { CubeProp::BODY_RADIUS,      0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::ORBIT_HEIGHT,     3.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::INFLUENCE_RADIUS, 3.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::SPIN_SPEED,       0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::BOB_AMPLITUDE,    0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::BOB_PERIOD,       0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::ASPECT_Y,         0.2f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::ASPECT_Z,         0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::FACE_VARIANCE,    0.0f, 1e30f, false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t CUBE_PARAM_COUNT = sizeof(CUBE_PARAM_DEFS) / sizeof(TierParamDef);
static_assert(CUBE_PARAM_COUNT == CubeIdx::COUNT,
    "F-4: CUBE_PARAM_DEFS must cover CubeIdx exactly (row order IS the index)");

// params[] order MUST match CUBE_PARAM_DEFS:
//   [0]BODY_RADIUS [1]ORBIT_HEIGHT [2]INFLUENCE_RADIUS [3]SPIN_SPEED
//   [4]BOB_AMPLITUDE [5]BOB_PERIOD [6]ASPECT_Y [7]ASPECT_Z [8]FACE_VARIANCE
struct CubeTierRow {
    TierProfile profile;
    float       spin_tilt_sigma;
};

// ── Cube tier table ────────────────────────────────────────────────
// Row = cube tier index (0 SmallCube / 1 MedCube / 2 LargeCube /
// 3 Monolith — plain index, no enum class; CUBE_TIER_COUNT pinned in
// floaters.hpp). Each row = { weight, color_var, { 9 {μ,σ}
// pairs in CubeIdx order:
//   BODY_RADIUS ORBIT_HEIGHT INFLUENCE_RADIUS SPIN_SPEED BOB_AMPLITUDE
//   BOB_PERIOD ASPECT_Y ASPECT_Z FACE_VARIANCE } }, spin_tilt_sigma.
// UNITS: radii/height/amplitude = wu; SPIN_SPEED = rad/s; BOB_PERIOD =
//   s; ASPECT_Y/Z / FACE_VARIANCE = multipliers; spin_tilt_sigma =
//   radians. CONSUMERS: cube_get_tier_profile (generic sampling);
//   spin_tilt_sigma at cube write_gpu.
// Biography determinant — frozen biography (§12).
inline constexpr CubeTierRow CUBE_TIERS[CUBE_TIER_COUNT] = {
    /* 0: SmallCube */ {
        { 0.40f, 0.0f, { {1.8f, 0.5f}, {25.0f, 20.0f}, {6.0f, 1.5f},  {0.04f, 0.015f},
                   {1.0f, 0.3f}, {5.0f, 1.5f},
                   {1.0f, 0.15f}, {1.0f, 0.15f}, {0.40f, 0.12f} }},
        0.12f
    },
    /* 1: MedCube   */ {
        { 0.32f, 0.0f, { {4.0f, 1.2f}, {45.0f, 30.0f}, {10.0f, 2.0f}, {0.03f, 0.01f},
                   {1.5f, 0.4f}, {6.0f, 2.0f},
                   {1.0f, 0.20f}, {1.0f, 0.20f}, {0.45f, 0.15f} }},
        0.10f
    },
    /* 2: LargeCube */ {
        { 0.20f, 0.0f, { {8.0f, 2.5f}, {75.0f, 45.0f}, {14.0f, 3.0f}, {0.02f, 0.008f},
                   {2.0f, 0.5f}, {8.0f, 2.5f},
                   {1.0f, 0.25f}, {1.0f, 0.25f}, {0.35f, 0.10f} }},
        0.08f
    },
    /* 3: Monolith  */ {
        { 0.08f, 0.0f, { {3.0f, 0.8f}, {12.0f, 8.0f}, {12.0f, 3.0f}, {0.015f, 0.005f},
                   {1.2f, 0.3f}, {6.0f, 2.0f},
                   {5.0f, 1.2f}, {0.15f, 0.03f}, {0.45f, 0.12f} }},
        0.10f
    },
};

inline const TierProfile& cube_get_tier_profile(uint32_t tier_idx) {
    return CUBE_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits CUBE_TRAITS = {
    PopFamily::CUBE, "cube", Dim::MAX_CUBE_INSTANCES,
    false, false, 0,
    true,
    CubeProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
    mood_mult_for(PopFamily::CUBE), CubeConfig::POSITION_JITTER,
    CUBE_TIER_COUNT, CubeProp::TIER,
    CUBE_PARAM_DEFS, CUBE_PARAM_COUNT,
    CubeProp::ANCHOR_X, CubeProp::ANCHOR_Z, CubeProp::ROTATION, false,
    0, nullptr,
};

inline SpawnGateOutput cube_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    auto gate = run_spawn_preamble(c, gx, gz, c->cube_behaviors_state_.activeCubes_, Dim::MAX_CUBE_INSTANCES,
        CubeProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
        mood_mult_for(PopFamily::CUBE), PopFamily::CUBE, "cube");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}

inline void cube_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    inst.solid_half = inst.params[CubeIdx::BODY_RADIUS];
    inst.ground_y_offset = 0.0f;
    inst.burial = 0.0f;
}

inline void cube_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    inst.colors[0] = cpu_hash_f(inst.seed, CubeProp::COLOR_R) * 0.55f + 0.35f;
    inst.colors[1] = cpu_hash_f(inst.seed, CubeProp::COLOR_G) * 0.50f + 0.30f;
    inst.colors[2] = cpu_hash_f(inst.seed, CubeProp::COLOR_B) * 0.60f + 0.20f;
}

inline void cube_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ac = c->cube_behaviors_state_.activeCubes_[inst.slot];
    ac.patch_gx = inst.trigger_gx; ac.patch_gz = inst.trigger_gz;
    ac.host_gx = inst.host_gx; ac.host_gz = inst.host_gz;
    ac.cx = inst.cx; ac.cz = inst.cz;
    ac.last_alloc_time = c->time_state_.seconds;
    ac.active = true;
    c->cube_behaviors_state_.activeCubeCount_++;
}

inline void cube_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    // Spin tilt: custom derivation from tier constant (not a sampled param)
    float tilt_sigma = CUBE_TIERS[inst.tier_idx].spin_tilt_sigma;
    float tilt_x = (cpu_hash_f(inst.seed, CubeProp::SPIN_TILT_X) - 0.5f) * 2.0f * tilt_sigma;
    float tilt_z = (cpu_hash_f(inst.seed, CubeProp::SPIN_TILT_Z) - 0.5f) * 2.0f * tilt_sigma;

    GPUFloatingEntityState fe{};
    fe.anchor[0] = inst.cx; fe.anchor[1] = 0.0f; fe.anchor[2] = inst.cz;
    fe.body_radius = inst.params[CubeIdx::BODY_RADIUS];
    fe.orbit_radius = 0.0f;
    fe.orbit_height = inst.params[CubeIdx::ORBIT_HEIGHT];
    fe.orbit_speed = 0.0f;
    fe.influence_radius = inst.params[CubeIdx::INFLUENCE_RADIUS];
    fe.spin_speed = inst.params[CubeIdx::SPIN_SPEED];
    fe.bob_amplitude = inst.params[CubeIdx::BOB_AMPLITUDE];
    fe.bob_period = inst.params[CubeIdx::BOB_PERIOD];
    fe.spin_tilt_x = tilt_x; fe.spin_tilt_z = tilt_z;
    fe.base_color[0] = inst.colors[0]; fe.base_color[1] = inst.colors[1]; fe.base_color[2] = inst.colors[2];
    fe.color[0] = inst.colors[0]; fe.color[1] = inst.colors[1]; fe.color[2] = inst.colors[2];
    fe.aspect_y = inst.params[CubeIdx::ASPECT_Y];
    fe.aspect_z = inst.params[CubeIdx::ASPECT_Z];
    fe.face_variance = inst.params[CubeIdx::FACE_VARIANCE];
    fe.geometry_type = 1; fe.motion_type = 1;
    fe.entity_seed = Dim::CUBE_SLOT_OFFSET + inst.slot;
    fe.t = 0.0f; fe.orientation[3] = 1.0f;
    fe.pos[0] = inst.cx; fe.pos[1] = fe.orbit_height; fe.pos[2] = inst.cz;
    fe.is_active = 1;
    //
    fe.spring_stiffness = CUBE_DEFAULT_SPRING_STIFFNESS;
    fe.drag             = CUBE_DEFAULT_DRAG;
    fe.tier_idx = inst.tier_idx;
    apply_cube_tier_gains(fe.spring_stiffness, fe.drag, inst.tier_idx);
    // CONTACT_2 C1b: bake the tier's plasticity λ per-instance (no GPU
    // cube-tier array + no new bindings this batch — rides the fe pad).
    fe.plasticity = (inst.tier_idx < CUBE_TIER_COUNT)
                        ? CUBE_TIER_GAINS[inst.tier_idx].plasticity : 0.0f;
    fe.drift[0] = 0.0f; fe.drift[1] = 0.0f; fe.drift[2] = 0.0f;
    fe.drift_vel[0] = 0.0f; fe.drift_vel[1] = 0.0f; fe.drift_vel[2] = 0.0f;
    fe.behavior_id    = pick_cube_behavior_for_spawn(c->mood_state_.active, inst.seed);
    fe.behavior_phase = cpu_hash(inst.seed, 0xF10A7E70u);
    // Kite mode starts disabled — cube is anchored to its spawn patch
    // until the user explicitly toggles kite mode via toggle_cube_kite_mode (below).
    fe.follow_pawn = 0;
    fe.pawn_offset[0] = 0.0f; fe.pawn_offset[1] = 0.0f; fe.pawn_offset[2] = 0.0f;
    c->gpuState_.upload_cube_entity_slot(queue, inst.slot, fe);
}

inline constexpr uint32_t CUBE_INDOOR_RESCALE_PARAMS[] = {
    CubeIdx::BODY_RADIUS, CubeIdx::ORBIT_HEIGHT, CubeIdx::INFLUENCE_RADIUS,
    CubeIdx::BOB_AMPLITUDE,
    // SPIN_SPEED (a rate), BOB_PERIOD (a time), ASPECT_Y / ASPECT_Z /
    // FACE_VARIANCE (ratios) intentionally not scaled.
};

// Cube policy: CAP (INDOOR_TREATMENT). Vertical extent =
// orbit_height + the body's half-height + bob_amplitude; the
// half-height is body_radius × aspect_y (the render kernel scales Y
// by r · aspect_y; the GPU floor clamp uses the same term). ASPECT_Y
// is a ratio, so scaling BODY_RADIUS carries the half-height.
inline void cube_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    cap_to_ceiling(inst, ceiling_h, INDOOR_HEIGHT_CAP_FRACTION,
        /*current_h*/ inst.params[CubeIdx::ORBIT_HEIGHT]
            + inst.params[CubeIdx::BODY_RADIUS] * inst.params[CubeIdx::ASPECT_Y]
            + inst.params[CubeIdx::BOB_AMPLITUDE],
        CUBE_INDOOR_RESCALE_PARAMS);
}

inline constexpr EntityFamilyAdapter CUBE_ADAPTER = {
    cube_run_gate,
    cube_apply_indoor_rescale,            // CAP (INDOOR_TREATMENT): the floaters joined the module
    cube_compute_solid_half, cube_compute_colors,
    cube_write_active, cube_write_gpu, nullptr,
    cube_get_tier_profile,
};

inline bool dispatch_select_cube_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, CUBE_TRAITS, CUBE_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::CUBE; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_cube_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, CUBE_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->cube_behaviors_state_.activeCubes_[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_cube_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) {
        generic_commit(self, CUBE_TRAITS, CUBE_ADAPTER, pe.generic, queue);
        // Lifecycle Phase 2: cube lifetime decoupled from host patch.
        // See dispatch_commit_sphere_generic for the rationale.
    }
    else { self->cube_behaviors_state_.activeCubes_[pe.generic.slot].active = false; }
}


// ─── Readback mirror reconciliation (owner verb) ─
// the cube half of the floater-readback funnel.
inline void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data) {
    float now = c->time_state_.seconds;
    // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
        // cube active-slot mirror owned by CubeBehaviorsState (cube_behaviors.hpp)
        if (cs.activeCubes_[i].active && !gpu_active &&
            (now - cs.activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
            cs.activeCubes_[i].active = false;
            if (cs.activeCubeCount_ > 0) cs.activeCubeCount_--;
        }
    }
}

} // namespace the_board
} // namespace t7
