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
#include <iostream>   // diagnostics feedback   // (impl, merged)
#include <cstdio>     // std::fprintf — the [ZOETROPE] witness line
#include <numeric>    // std::gcd — the helix coprimality witness
#include <algorithm>  // std::min / std::max — the walk's clamps and settle norm
#include "core/instruments.hpp"   // THE INSTRUMENTS DIAL: INSTRUMENTS.zoetrope_witness gates the [ZOETROPE] line

namespace t7 {
namespace the_board {

// ═══ MODULE DEPS ════════════════════════════════════════════════════
// The cube commands' requirements face: corral/kite center on THE
// POINT through the point's own record (point_.x/z — the agent
// slot reach retired with it); all reads except the GPU wire.
struct CubeDeps {
    GPUState&        gpuState_;
    const TimeState& time_state_;
    const PlayerState& player_;
    const PointState&  point_;   // the point's house (position mirror — the corral ring's center)
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

// ─ Diagnostics (coordination cycle) ─────────────────────────────

inline constexpr float FLOATER_COORDINATION_STEPS[3] = { 0.0f, 0.5f, 1.0f };

// ─ WGSL kernel constants (NOT here, but documented) ─────────────
//
//   cube_force_phasewave (world.wgsl):
//     k_x         = 0.020    wavefront freq in X
//     k_z         = 0.012    wavefront freq in Z (asymmetric)
//     omega       = 1.5      1/s, temporal frequency
//     amplitude   = 30.0     force magnitude (vertical)

// ═══ ZOETROPE — the lattice panel ═══ all Jean-tunable; beats units
inline constexpr uint32_t LATTICE_ROWS  = 7;   // the mode's degrees
inline constexpr uint32_t LATTICE_COLS  = Dim::MAX_CUBE_INSTANCES / LATTICE_ROWS;          // 36
inline constexpr uint32_t LATTICE_CELLS = LATTICE_ROWS * LATTICE_COLS;  // 252 — the LIVING ceiling; capacity stays 256
inline constexpr float ZOETROPE_TICK_BEATS        = 0.25f;  // automaton heartbeat
inline constexpr float ZOETROPE_REV_BEATS         = 16.0f;  // write-head revolution
inline constexpr float ZOETROPE_EXCITE_DIFFUSE    = 0.20f;  // per-tick neighbor share
inline constexpr float ZOETROPE_ASYMMETRY         = 0.15f;  // seed-hashed weight skew
inline constexpr float ZOETROPE_EXCITE_HALF_BEATS = 1.0f;   // flash memory
inline constexpr float ZOETROPE_PIGMENT_GAIN      = 0.35f;  // deposit rate
inline constexpr float ZOETROPE_PIGMENT_HALF_BEATS = 48.0f; // long memory
static_assert(4.0f * ZOETROPE_EXCITE_DIFFUSE * (1.0f + ZOETROPE_ASYMMETRY) < 1.0f,
              "lattice diffusion unstable — lower DIFFUSE or ASYMMETRY");
inline constexpr float ZOETROPE_PIGMENT_R = 0.55f;  // ethereal ice —
inline constexpr float ZOETROPE_PIGMENT_G = 0.75f;  // ruled at the
inline constexpr float ZOETROPE_PIGMENT_B = 1.00f;  // visual gate
inline constexpr uint32_t ZOETROPE_CELL_STRIDE   = LATTICE_COLS + 1;  // 37 — the helix
inline constexpr uint32_t ZOETROPE_CELL_UNSTRIDE = 109;               // 37⁻¹ mod 252
static_assert((ZOETROPE_CELL_STRIDE * ZOETROPE_CELL_UNSTRIDE) % LATTICE_CELLS == 1u,
              "helix inverse broken — recompute UNSTRIDE for these dims");
static_assert(std::gcd(ZOETROPE_CELL_STRIDE, LATTICE_CELLS) == 1u,
              "helix stride must be coprime to the lattice");
inline constexpr float ZOETROPE_RING_RADIUS = 60.0f;  // arc = 2π·R/36 ≈ 10.5 wu; FOV° ≈ 2·atan(3.5·H_STEP/RADIUS) ≈ 55°
inline constexpr float ZOETROPE_H_BASE      = 8.0f;   // row-0 height above ground (wu)
inline constexpr float ZOETROPE_H_STEP      = 9.0f;   // wu per mode degree — screen ≈ rows 8..62 wu
inline constexpr float ZOETROPE_PIXEL_RADIUS = 3.2f;  // pixel half-size; full ≈ 6.4 wu
inline constexpr float ZOETROPE_FACE_SPLAY = 1.5f; // added face_variance at full excitation
// ─ the scatter seat (the gathering, not the instrument) ─────────
inline constexpr float ZOETROPE_SCATTER_RADIUS  = 90.0f;  // wu — the gathering's mean reach
inline constexpr float ZOETROPE_SCATTER_JITTER_R = 0.45f; // × radius — deep, this is a flock
inline constexpr float ZOETROPE_SCATTER_JITTER_H = 28.0f; // wu — free vertical spread
inline constexpr uint32_t ZOETROPE_SCATTER_SEED = 0x5CA77E12u;
inline constexpr float ZOETROPE_LIFT_TAU    = 1.1f;   // s — the climb's own walk law; birth-equal to CUBE_GLIDE_TAU, independently tunable
inline constexpr float ZOETROPE_SETTLE_EPS  = 0.05f;  // wu — snap-and-stop threshold
inline constexpr float ZOETROPE_RESEAT_JUMP = 40.0f;  // wu/frame — no motion moves the point this far; only possess() does

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════

struct CubeTierGain {
    uint32_t tier_idx;
    const char* name;
    float spring_stiffness_mult;
    float drag_mult;
    float behavior_amp_mult;   // reserved; not yet consumed by kernel
    float plasticity;          // CONTACT_3 K2c: per-tier RELATIVE λ character;
                               // the live master is config.cube_plasticity.
                               // Baked per-instance at spawn — changing a
                               // ROW needs a respawn; the MASTER is live.
                               // Jean-tunable in-row.
};

inline constexpr CubeTierGain CUBE_TIER_GAINS[CUBE_TIER_COUNT] = {
    //                            spring  drag   amp   λ (relative character — K2c; master = config.cube_plasticity)
    /* 0 SmallCube */ { 0, "SmallCube", 1.0f, 1.0f, 1.0f, 1.0f },
    /* 1 MedCube   */ { 1, "MedCube",   1.0f, 1.0f, 1.0f, 0.8f },
    /* 2 LargeCube */ { 2, "LargeCube", 1.0f, 1.0f, 1.0f, 1.2f },
    /* 3 Monolith  */ { 3, "Monolith",  1.0f, 1.0f, 1.0f, 0.5f },
};

static_assert(sizeof(CUBE_TIER_GAINS) / sizeof(CUBE_TIER_GAINS[0]) == CUBE_TIER_COUNT,
              "CUBE_TIER_GAINS must declare one row per cube tier");

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════
//
// Mood ordering matches MOOD_TABLE (contracts/spine_state.hpp).
//
// Cubes carry a mood term, mood_mult_for(PopFamily::CUBE) from
// MOOD_SPAWN_MULT (surface/population_themes.hpp). That column is
// {1, 1, 1, 1} — all identity. This banner used to claim
// {1, 1, 0, 0, 1, 0} and conclude that cubes "don't spawn in indoor
// moods"; the indoor zeros never existed in the live table, and the
// per-row "exists for hygiene" notes inherited the error. The mood
// term suppresses no row today, so every row here IS consulted.
// Indoor cube spawning is shaped by indoor_bounds_clamp
// (machine/spawn_engine.hpp), not by this multiplier.

struct CubePopulationDef {
    uint32_t mood_id;
    std::array<float, CUBE_BEHAVIOR_COUNT> behavior_weights;
};

inline constexpr CubePopulationDef CUBE_POPULATIONS[MOOD_COUNT] = {
    /* MOOD_OPEN_SUNSET */
    { MOOD_OPEN_SUNSET,
      //                    stat curl  wave
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_INDOOR_FLAT */
    { MOOD_INDOOR_FLAT,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_INDOOR_VAULT */
    { MOOD_INDOOR_VAULT,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
    /* MOOD_FINITE_OUTDOOR */
    { MOOD_FINITE_OUTDOOR,
      /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f } },
};

static_assert(sizeof(CUBE_POPULATIONS) / sizeof(CUBE_POPULATIONS[0]) == MOOD_COUNT,
              "CUBE_POPULATIONS must declare one row per mood");
static_assert(CUBE_POPULATIONS[0].mood_id == MOOD_OPEN_SUNSET,    "row 0 must be MOOD_OPEN_SUNSET");
static_assert(CUBE_POPULATIONS[1].mood_id == MOOD_INDOOR_FLAT,    "row 1 must be MOOD_INDOOR_FLAT");
static_assert(CUBE_POPULATIONS[2].mood_id == MOOD_INDOOR_VAULT,   "row 2 must be MOOD_INDOOR_VAULT");
static_assert(CUBE_POPULATIONS[3].mood_id == MOOD_FINITE_OUTDOOR, "row 3 must be MOOD_FINITE_OUTDOOR");

// ═══ DIAGNOSTIC STATE (owned by the tools) ═══════════════════════

// One lattice cell: fast excitation (the flash) and slow pigment (the
// long memory). THE HELIX (G2): cell = slot·37 mod 252 — consecutive
// spawns land one column over, one row up; the screen densifies with
// the population.
struct ZoetropeCell {
    float excite  = 0.0f;
    float pigment = 0.0f;
};

struct CubeBehaviorsState {
    uint32_t   coordination_step  = 0;
    uint32_t   behavior_override  = CUBE_BEHAVIOR_STATIONARY;
    bool       kite_mode          = false;
    ActiveCube activeCubes_[Dim::MAX_CUBE_INSTANCES]{};

    // ── The reveal machine (C6R; H1: F6 IS A CYCLE) ── staged capture,
    // CPU flush-walk climb. Three resting states — roam, scattered,
    // screen — and a walk between each: roam → scatter → screen → roam.
    // Every transition is a walk; nothing teleports.
    enum class Formation : uint8_t {
        ROAM, TO_SCATTER, SCATTERED, TO_SCREEN, SCREEN, TO_ROAM };
    Formation formation = Formation::ROAM;
    // The reveal BORROWS the kite; it never owns it. kite_mode stays the
    // FLOCK's own duty (F7's flag), handed back untouched at TO_ROAM.
    bool  kite_by_reveal = false;
    bool  stations_sent = false;
    bool  stage_wait    = false;          // the press frame: the 3u sentinel is in
                                          // flight (V1) — no target/height writes
    // The walker's own shadows (G5) — height, radius, aspects; it never
    // reads the GPU values; the walker owns the scalars while the walk
    // lives. The PRIOR needs no stage: THE MIRROR IS THE PRIOR
    // (activeCubes_ keeps every tier draw; release targets read it).
    struct ZoeWalk { float h, r, ay, az; } walk_[LATTICE_CELLS]{};
    bool  settled[LATTICE_CELLS]{};
    // ── The reseat watch (G4) ── the point's last seen position; a
    // per-frame step no motion can make marks a possession seam.
    float last_px = 0.0f, last_pz = 0.0f;
    bool  point_seen = false;

    // ── The zoetrope lattice (C4) ── zero-init is the law: boot is a
    // transition from nothing — the lattice wakes silent, never replayed.
    ZoetropeCell cells[LATTICE_CELLS]{};
    float        cell_scratch[LATTICE_CELLS]{};   // diffusion inflow accumulator
    float        wdir[LATTICE_CELLS][4]{};        // fixed-seed asymmetric weights, built at prime
    float        last_tick_beat = 0.0f;           // the automaton's tick anchor
    bool         primed         = false;          // weights built + clock anchored
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
void reveal_zoetrope(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
uint32_t set_cube_kite(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue, bool on);  // the ONE kite home (G3)
bool cube_kite_effective(const CubeBehaviorsState& cbs);   // duty OR the reveal's borrow (H1)
void toggle_cube_kite_mode(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
// Per-frame
void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data);
// The zoetrope (the lattice substrate — spine-wired at U4; the C5
// projector pokes ride the same calls, so they carry the GPU wire)
void zoetrope_strike(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    uint32_t active_seed, const float rows[7], float t_beats);
void zoetrope_service(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    uint32_t active_seed, float t_beats, float dt, float point_x, float point_z);
// The projector (C5): one home — cells reach pixels here and nowhere else
uint32_t zoetrope_slot_seed(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot);
float zoetrope_cell_intensity(const CubeBehaviorsState& cbs, uint32_t slot);  // I — ONE computation (G6)
void project_cell_color(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot,
    float& out_r, float& out_g, float& out_b);

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
// ─── Corral / kite (the anchor law) ────────────────────────────────
//
// The CPU authors TARGETS only; update_cube walks the live param
// toward them (CUBE_GLIDE_TAU). Mode switches ride the follow_pawn
// sentinels (3u kite-capture / 2u kite-release) — captured in-kernel
// from the true present, position-preserving even under drift.

// DEPS-FORM PRECEDENT: explicit GPUState& parameter —
// the deps form's first citizen; not a MachineCtx bypass.
inline void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        cbs.activeCubes_[i] = ActiveCube{};
        GPUFloatingEntityState empty{};
        gpu.upload_cube_entity_slot(queue, i, empty);
    }
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

// ── The zoetrope's stations (C6R) — the ONE home for geometry ───
// THE HELIX (G2): cell = slot·37 mod 252 — consecutive spawns land one
// column over, one row up; the screen densifies with the population.
// The stride is coprime to the lattice, so slot↔cell is a bijection
// (UNSTRIDE is its inverse); every row is reachable at any count.
inline uint32_t cell_of_slot(uint32_t s) { return (s * ZOETROPE_CELL_STRIDE)   % LATTICE_CELLS; }
inline uint32_t slot_of_cell(uint32_t c) { return (c * ZOETROPE_CELL_UNSTRIDE) % LATTICE_CELLS; }

// ── THE TWO STATIONS, side by side (H1b) ────────────────────────
// SCATTER is the flock drawn in: own bodies, own altitudes, deep
// jitter. SCREEN is the instrument: uniform pixels, ranked rows,
// tight jitter. One function each; the cycle chooses.
//
// Both seat from the same construction — cell = cell_of_slot(slot),
// theta from that cell's column — so a cube keeps its angular
// identity across the whole cycle, and only its reach, its rank and
// its body change between the two.
struct ZoetropeStation { float off_x; float off_z; float h; };

inline ZoetropeStation zoetrope_station(uint32_t slot) {
    const uint32_t cell = cell_of_slot(slot);
    const uint32_t row = cell / LATTICE_COLS;
    const uint32_t col = cell % LATTICE_COLS;
    const float two_pi = 6.28318530718f;
    const float theta = two_pi * float(col) / float(LATTICE_COLS);
    return { std::cos(theta) * ZOETROPE_RING_RADIUS,
             std::sin(theta) * ZOETROPE_RING_RADIUS,
             ZOETROPE_H_BASE + float(row) * ZOETROPE_H_STEP };
}

// The gathering's seat. Fixed-seed jitters (ZOETROPE_SCATTER_SEED), so
// re-pressing lands every cube on the same seat it left — the flock has
// a shape, not a shuffle.
inline ZoetropeStation station_scatter(const CubeBehaviorsState& cbs, uint32_t slot) {
    const uint32_t cell = cell_of_slot(slot);
    const uint32_t col  = cell % LATTICE_COLS;
    const float two_pi = 6.28318530718f;
    const float theta = two_pi * float(col) / float(LATTICE_COLS);
    // Deep radial jitter — this is a flock, not a ring.
    const float jr = (cpu_hash_f(ZOETROPE_SCATTER_SEED, cell * 2u) - 0.5f) * 2.0f;
    const float radius = ZOETROPE_SCATTER_RADIUS * (1.0f + jr * ZOETROPE_SCATTER_JITTER_R);
    // Free vertical spread around the cube's OWN spawn altitude: the
    // flock keeps its altitude character — it is gathered, not ranked.
    // Floored at the screen's own base so no gathered cube sinks in.
    const float jh = (cpu_hash_f(ZOETROPE_SCATTER_SEED, cell * 2u + 1u) - 0.5f) * 2.0f;
    const float h = std::max(ZOETROPE_H_BASE,
        cbs.activeCubes_[slot].orbit_height + jh * ZOETROPE_SCATTER_JITTER_H);
    return { std::cos(theta) * radius, std::sin(theta) * radius, h };
}

inline void reveal_zoetrope(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    using Formation = CubeBehaviorsState::Formation;

    // F6 IS THE CYCLE (H1): roam → scatter → screen → roam. One door,
    // three destinations; F7 never touches it. A press MID-WALK counts as
    // its destination's press — the cycle advances and the walk simply
    // re-aims, because the targets are read from the state every frame.
    Formation next;
    const char* line;
    switch (cbs.formation) {
        case Formation::ROAM:
        case Formation::TO_ROAM:
            next = Formation::TO_SCATTER;
            line = "[Zoetrope] gather: the flock draws in";    break;
        case Formation::SCATTERED:
        case Formation::TO_SCATTER:
            next = Formation::TO_SCREEN;
            line = "[Zoetrope] reveal: the screen assembles";  break;
        default:   // SCREEN | TO_SCREEN
            next = Formation::TO_ROAM;
            line = "[Zoetrope] release: the swarm walks home"; break;
    }

    // THE KITE, BORROWED. The formation is kite-native — it tracks the
    // point or it is not a formation — and an anchor-mode cube handed
    // ring OFFSETS would glide to the WORLD ORIGIN (the documented
    // spawn-mode desync trap), so the capture is not optional while one
    // stands. But the kite DUTY is the flock's, and F7's alone:
    // set_cube_kite is the one sentinel home (G3) and writes the duty
    // flag as F7's caller needs, so the borrow saves it and hands it
    // straight back. TO_ROAM then restores exactly what the flock asked
    // for, whether or not F7 was touched while the formation stood.
    if (next == Formation::TO_ROAM) {
        cbs.kite_by_reveal = false;
        set_cube_kite(cbs, c->gpuState_, queue, cbs.kite_mode);   // the duty, handed back
    } else if (!cbs.kite_by_reveal) {
        const bool duty = cbs.kite_mode;
        set_cube_kite(cbs, c->gpuState_, queue, true);
        cbs.kite_mode = duty;
        cbs.kite_by_reveal = true;
    }

    // STAGED (V1): the sentinel eats target_x/z in the frame it is
    // consumed, so the press writes NO targets and NO bodies — it stages,
    // and the service walks from the next frame. Nothing new is recorded
    // either: THE MIRROR IS THE PRIOR (G5). The walk re-arms from its own
    // live shadows; only on the first press out of ROAM are those shadows
    // seeded from the mirror, because in ROAM the GPU scalars ARE the
    // mirror's draws — nothing has walked them yet.
    const bool from_roam = (cbs.formation == Formation::ROAM);
    uint32_t staged = 0;
    for (uint32_t i = 0; i < LATTICE_CELLS; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        const ActiveCube& ac = cbs.activeCubes_[i];
        if (from_roam)
            cbs.walk_[i] = { ac.orbit_height, ac.body_radius, ac.aspect_y, ac.aspect_z };
        cbs.settled[i] = false;
        staged++;
    }

    cbs.formation = next;
    cbs.stations_sent = false;
    cbs.stage_wait = true;      // this frame stages; the service acts next frame

    std::cout << line << " (" << staged << " cube(s))\n";
}

// ─── Kite mode toggle (F7) ──────────────────────────────────────

// THE ONE KITE HOME (G3): flag write + sentinel loop, extracted whole
// from the toggle; no print — callers narrate. DEPS-FORM PRECEDENT
// (clear_cubes): explicit GPUState& param, so the spine's service can
// call it without a deps face. Returns the affected count.
//   ON  → 3u kite-capture: offset := the true present
//         (pos − point − drift); target := offset.
//   OFF → 2u kite-release: anchor := current pos; target :=
//         anchor; drift zeroed.
// Both xz-position-preserving even under drift — the capture
// happens where drift lives. (Release clears drift.xz only; the
// vertical WALKS home on the existing spring/drag rather than
// snapping. Capture is xz-exact to f32.)
inline uint32_t set_cube_kite(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue, bool on) {
    cbs.kite_mode = on;
    const uint32_t sentinel = on ? 3u : 2u;
    uint32_t affected = 0;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        gpu.upload_cube_follow_pawn(queue, i, sentinel);
        affected++;
    }
    return affected;
}

// THE EFFECTIVE KITE (H1): the flock's own duty OR the reveal's borrow.
// The GPU is kited whenever either says so, and kite_mode alone stopped
// answering that question the moment the borrow started restoring it.
// Every reader that asks "is the flock kited RIGHT NOW" must ask here —
// a cube born anchored while its neighbours are kited would read the
// next seat pass's ring OFFSETS as world coordinates and glide to the
// world origin (the documented spawn-mode desync trap).
inline bool cube_kite_effective(const CubeBehaviorsState& cbs) {
    return cbs.kite_mode || cbs.kite_by_reveal;
}

inline void toggle_cube_kite_mode(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    // F7 IS ONLY THE KITE (H1). While the reveal HOLDS the borrow, F7
    // flips the flock's own duty flag and says so — no sentinel, nothing
    // disturbed — and the duty is handed back when the cycle closes.
    // The gate is the borrow itself, NOT formation != ROAM: the handback
    // happens at the press that ENTERS TO_ROAM, so during that walk the
    // flock owns its kite again and F7 must send a real sentinel.
    if (cbs.kite_by_reveal) {
        cbs.kite_mode = !cbs.kite_mode;
        std::cout << "[Floaters] kite mode: " << (cbs.kite_mode ? "ON" : "OFF")
                  << " (formation stands — flock already kited)\n";
        return;
    }

    const uint32_t affected = set_cube_kite(cbs, c->gpuState_, queue, !cbs.kite_mode);

    std::cout << "[Floaters] kite mode: " << (cbs.kite_mode ? "ON" : "OFF")
              << " (" << affected << " cube(s))\n";
}

// ═══ THE EVICTOR ══════════════════════════════════════════════════

inline void evict_cube(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->cube_behaviors_state_.activeCubes_[slot].active = false;  // cube state owned by CubeBehaviorsState
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_cube_entity_slot(queue, slot, empty);
}

// ═══ THE CUBE RECIPE ══════════════════════════════════════════════
//
// Tier tables, traits, adapter, and dispatch funnels — beside the
// evictor. Funnels declared above; the FAMILY_DISPATCH rows
// (cartridge.hpp, post-class) point here.

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
    PopFamily::CUBE, LATTICE_CELLS,   // the LIVING ceiling (7×36 = 252); capacity stays 256 — slots 252-255 never allocate
    false,                // NOT grounded — hovers and drifts; claims no ground (ruling 21)
    CubeProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
    mood_mult_for(PopFamily::CUBE), CubeConfig::POSITION_JITTER,
    CUBE_TIER_COUNT, CubeProp::TIER,
    CUBE_PARAM_DEFS, CUBE_PARAM_COUNT,
    CubeProp::ANCHOR_X, CubeProp::ANCHOR_Z, CubeProp::ROTATION,
    0, nullptr,
};

inline SpawnGateOutput cube_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    return gate_from_traits(c, gx, gz, CUBE_TRAITS, c->cube_behaviors_state_.activeCubes_);
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
    ac.last_alloc_time = c->time_state_.seconds;
    ac.orbit_height  = inst.params[CubeIdx::ORBIT_HEIGHT];    // the tier draws — THE MIRROR IS THE
    ac.body_radius   = inst.params[CubeIdx::BODY_RADIUS];     // PRIOR (C6R V2 + G5): release walks
    ac.aspect_y      = inst.params[CubeIdx::ASPECT_Y];        // every possessed scalar back here
    ac.aspect_z      = inst.params[CubeIdx::ASPECT_Z];
    ac.face_variance = inst.params[CubeIdx::FACE_VARIANCE];
    ac.active = true;
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
    // ZOETROPE (C5): the ghost dresses the newborn — color projects the
    // cell's accumulated state over the seed base (write_active has already
    // seated the mirror, so the slot's seed recomputes; at I = 0 this is
    // inst.colors bit-exactly — the silent path is today's).
    project_cell_color(c->cube_behaviors_state_, c->world_state_.active_seed, inst.slot,
                       fe.color[0], fe.color[1], fe.color[2]);
    fe.aspect_y = inst.params[CubeIdx::ASPECT_Y];
    fe.aspect_z = inst.params[CubeIdx::ASPECT_Z];
    // ZOETROPE (G6): the ghost's splay dresses the newborn whole-slot —
    // variance = the tier draw + SPLAY·I of the inherited cell (the
    // mirror keeps the bare draw; the projector relaxes it as I dims).
    fe.face_variance = inst.params[CubeIdx::FACE_VARIANCE]
                     + ZOETROPE_FACE_SPLAY
                       * zoetrope_cell_intensity(c->cube_behaviors_state_, inst.slot);
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
    // BIRTH INTO THE LIVE MODE (RIDER[cube:spawn-mode-desync]).
    // Per-cube mode truth lives on the GPU; kite_mode is one CPU flag
    // for the flock. A newborn left in anchor mode while the flock is
    // kited would be handed RING OFFSETS by the next corral — and a
    // mode-0 cube walks its ANCHOR toward its target, reading those
    // offsets as absolute world coordinates: a smooth glide to a ring
    // around the WORLD ORIGIN. So a newborn joins the mode that is
    // live at its spawn.
    //
    // Drift is exactly zero at birth (set below), so the offset is
    // exact — no sentinel round-trip, no capture frame. This is the
    // ONE init home for target in both arms: at rest target == param,
    // so update_cube's glide term is exactly zero either way.
    if (cube_kite_effective(c->cube_behaviors_state_)) {
        // Kite arm: the param is the OFFSET from the point, and the
        // point is point_.x/z — the same host-authored
        // snapshot the reveal rings around. The EFFECTIVE kite decides
        // (H1): under a standing formation the flock is kited by the
        // reveal's borrow even when the flock's own duty says otherwise,
        // and a newborn is born into the mode its neighbours are in.
        fe.follow_pawn = 1u;
        using Formation = CubeBehaviorsState::Formation;
        const auto formation = c->cube_behaviors_state_.formation;
        const bool born_to_screen  = (formation == Formation::TO_SCREEN
                                   || formation == Formation::SCREEN);
        const bool born_to_scatter = (formation == Formation::TO_SCATTER
                                   || formation == Formation::SCATTERED);
        if (born_to_screen || born_to_scatter) {
            // ZOETROPE (C6R E7 + G5 + H1): a newborn under a standing
            // formation flies to its cell's seat, written whole at commit
            // — the screen's seat takes PIXEL SCALE too, the gathering's
            // leaves the body its own spawn draw. Birth may leap: boot is
            // a transition from nothing, and so is birth. The mirror
            // keeps its true tier draws (write_active), so the walk home
            // scatters it as if it had always roamed.
            const ZoetropeStation st = born_to_screen
                ? zoetrope_station(inst.slot)
                : station_scatter(c->cube_behaviors_state_, inst.slot);
            fe.orbit_height = st.h;
            if (born_to_screen) {
                fe.body_radius = ZOETROPE_PIXEL_RADIUS;
                fe.aspect_y    = 1.0f;
                fe.aspect_z    = 1.0f;
            }
            fe.pawn_offset[0] = st.off_x;
            fe.pawn_offset[1] = 0.0f;
            fe.pawn_offset[2] = st.off_z;
        } else {
            fe.pawn_offset[0] = inst.cx - c->point_.x;
            fe.pawn_offset[1] = 0.0f;
            fe.pawn_offset[2] = inst.cz - c->point_.z;
        }
        fe.target_x = fe.pawn_offset[0];
        fe.target_z = fe.pawn_offset[2];
    } else {
        // Anchor arm: the param is anchor.xz, written above from the
        // same spawn position.
        fe.follow_pawn = 0u;
        fe.pawn_offset[0] = 0.0f; fe.pawn_offset[1] = 0.0f; fe.pawn_offset[2] = 0.0f;
        fe.target_x = inst.cx;
        fe.target_z = inst.cz;
    }
    c->gpuState_.upload_cube_entity_slot(queue, inst.slot, fe);

    // ZOETROPE (C6R + G5): birth bookkeeping — the walker's shadows
    // mirror the scalars just written; a newborn is born settled — no
    // walk owed, in any formation state. The PRIOR needs no record
    // here: the mirror is the prior (write_active seated the draws).
    auto& zcbs = c->cube_behaviors_state_;
    zcbs.walk_[inst.slot] = { fe.orbit_height, fe.body_radius, fe.aspect_y, fe.aspect_z };
    zcbs.settled[inst.slot] = true;
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


// ═══ THE ZOETROPE — the lattice substrate (C4) ════════════════════
//
// A 7×36 cylinder automaton over the cube slots. THE HELIX (G2):
// cell = slot·37 mod 252 — consecutive spawns land one column over, one
// row up; the screen densifies with the population (cell_of_slot /
// slot_of_cell are the only border crossings; cells stay cells).
// Columns wrap (the cylinder); rows clamp — the pitch edges are OPEN,
// top/bottom outflow is lost. Two fields per cell: fast excitation
// (diffuses one cell per tick, fixed-seed asymmetric weights) and slow
// pigment (integrates excitation, long half-life). Ticks ride t_beats —
// the musical clock; the write head is stateless, a pure function of
// t_beats. The automaton never stops, including during transit.

// Per-tick decay factors, derived from the panel (inline const, not
// constexpr — std::exp2 is runtime at this standard; computed once).
inline const float ZOETROPE_EXCITE_DECAY  = std::exp2(-ZOETROPE_TICK_BEATS / ZOETROPE_EXCITE_HALF_BEATS);
inline const float ZOETROPE_PIGMENT_DECAY = std::exp2(-ZOETROPE_TICK_BEATS / ZOETROPE_PIGMENT_HALF_BEATS);

// The weight table's seed grammar (primitives/seed_utils.hpp cpu_hash_f):
// one FIXED seed, property = cell·4 + dir — per-(cell,dir) skew,
// world-seed-independent (same MIDI, same screen).
inline constexpr uint32_t ZOETROPE_WEIGHT_SEED = 0x20E7A0DEu;

// ── The projector (C5) ──────────────────────────────────────────
//
// One home, one function: cells reach pixels HERE and nowhere else.
// base is RECOMPUTED through the seed fn (cube_compute_colors — base
// color's one home) from the slot's TRUE spawn seed, never cached:
// the gate drew tile_seed(active world seed, trigger patch)
// (machine/spawn_engine.hpp evaluate_spawn_gate), and the mirror
// keeps the trigger patch, so the seed reconstructs bit-exactly.
// I = min(1, excite + pigment); out = mix(base, PIGMENT, I). At
// I = 0 the mix returns base bit-exactly — silence projects today's
// tree unchanged, spawn included.

inline uint32_t zoetrope_slot_seed(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot) {
    const ActiveCube& ac = cbs.activeCubes_[slot];
    return tile_seed(active_seed, ac.patch_gx, ac.patch_gz);
}

// The intensity — I's ONE computation (G6): both the color mix and the
// face splay read the cell through this door and no other. Carries the
// helix crossing (G2) with it.
inline float zoetrope_cell_intensity(const CubeBehaviorsState& cbs, uint32_t slot) {
    const ZoetropeCell& cell = cbs.cells[cell_of_slot(slot)];
    return std::min(1.0f, cell.excite + cell.pigment);
}

inline void project_cell_color(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot,
    float& out_r, float& out_g, float& out_b) {
    const float I = zoetrope_cell_intensity(cbs, slot);
    EntityInstance tmp{};
    tmp.seed = zoetrope_slot_seed(cbs, active_seed, slot);
    // The seed fn's exact signature takes traits + tier; it reads neither
    // (both unnamed) — the call adapts, the law does not. G5 V2 verdict:
    // profile-INVARIANT — no profile field is consulted (and CUBE_TIERS'
    // color_var column is 0.0 in every row), so profile(0) is bit-exact
    // for every tier.
    cube_compute_colors(tmp, CUBE_TRAITS, cube_get_tier_profile(0));
    out_r = tmp.colors[0] + (ZOETROPE_PIGMENT_R - tmp.colors[0]) * I;
    out_g = tmp.colors[1] + (ZOETROPE_PIGMENT_G - tmp.colors[1]) * I;
    out_b = tmp.colors[2] + (ZOETROPE_PIGMENT_B - tmp.colors[2]) * I;
}

inline void zoetrope_strike(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    uint32_t active_seed, const float rows[7], float t_beats) {
    // The write head: col = the sweep's phase of revolution — stateless.
    const float phase = t_beats / ZOETROPE_REV_BEATS;
    const float fract = phase - std::floor(phase);
    const uint32_t col = uint32_t(fract * float(LATTICE_COLS)) % LATTICE_COLS;
    for (uint32_t r = 0; r < LATTICE_ROWS; ++r)
        if (rows[r] > 0.0f) {
            const uint32_t cell = r * LATTICE_COLS + col;
            cbs.cells[cell].excite += rows[r];
            // The attack never waits for a tick: the struck cell pokes
            // its slot — through the helix — immediately (mirror-active
            // only; ghosts stay unseen).
            const uint32_t slot = slot_of_cell(cell);
            if (cbs.activeCubes_[slot].active) {
                float cr, cg, cb;
                project_cell_color(cbs, active_seed, slot, cr, cg, cb);
                gpu.upload_cube_color(queue, slot, cr, cg, cb);
                // The splay rides the projector (G6): faces answer the music.
                const float I = zoetrope_cell_intensity(cbs, slot);
                gpu.upload_cube_face_variance(queue, slot,
                    cbs.activeCubes_[slot].face_variance + ZOETROPE_FACE_SPLAY * I);
            }
        }
    // Strike witness (the [CHECKER] form): one line per strike-frame,
    // on the dial. If the ears bound but this never prints, the fault
    // is upstream — routing, transport, or the extractor.
    if constexpr (INSTRUMENTS.zoetrope_witness) {
        bool struck = false;
        for (uint32_t r = 0; r < LATTICE_ROWS; ++r)
            if (rows[r] > 0.0f) { struck = true; break; }
        if (struck)
            std::fprintf(stderr,
                "[ZOETROPE] strike col=%02u rows= %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n",
                col, rows[0], rows[1], rows[2], rows[3], rows[4], rows[5], rows[6]);
    }
}

inline void zoetrope_service(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    uint32_t active_seed, float t_beats, float dt, float point_x, float point_z) {
    // ── The reseat watch (G4) ── possession moves the point in one
    // frame farther than any motion can; a standing formation answers by
    // re-using the capture two-step at the seam — recapture from the
    // true present, restage, resend seats around the new host. A resting
    // state re-enters its own walk so the seat pass runs; the bodies are
    // already settled, so it re-forms in one breath. TO_ROAM/ROAM: watch
    // only — a dissolving formation has no host to follow.
    {
        using Formation = CubeBehaviorsState::Formation;
        if (!cbs.point_seen) {
            cbs.last_px = point_x; cbs.last_pz = point_z; cbs.point_seen = true;
        }
        const float ddx = point_x - cbs.last_px;
        const float ddz = point_z - cbs.last_pz;
        const float delta = std::sqrt(ddx * ddx + ddz * ddz);
        if ((cbs.formation == Formation::TO_SCATTER || cbs.formation == Formation::SCATTERED
             || cbs.formation == Formation::TO_SCREEN || cbs.formation == Formation::SCREEN)
            && delta > ZOETROPE_RESEAT_JUMP) {
            // The borrow again: recapture without spending the flock's duty.
            const bool duty = cbs.kite_mode;
            set_cube_kite(cbs, gpu, queue, true);   // re-capture at the new host
            cbs.kite_mode = duty;
            if (cbs.formation == Formation::SCATTERED)   cbs.formation = Formation::TO_SCATTER;
            else if (cbs.formation == Formation::SCREEN) cbs.formation = Formation::TO_SCREEN;
            cbs.stations_sent = false;
            cbs.stage_wait = true;
            std::cout << "[Zoetrope] reseat: the screen follows its new host\n";
        }
        cbs.last_px = point_x; cbs.last_pz = point_z;
    }

    if (!cbs.primed) {
        // First service: build the fixed-seed asymmetric weight table —
        // per (cell, dir) a skew in [-ASYMMETRY, +ASYMMETRY] around
        // DIFFUSE — and anchor the tick clock. Cells stay zero: the
        // lattice wakes silent.
        cbs.primed = true;
        for (uint32_t cell = 0; cell < LATTICE_CELLS; ++cell)
            for (uint32_t dir = 0; dir < 4; ++dir) {
                const float skew = (cpu_hash_f(ZOETROPE_WEIGHT_SEED, cell * 4u + dir) - 0.5f)
                                 * 2.0f * ZOETROPE_ASYMMETRY;
                cbs.wdir[cell][dir] = ZOETROPE_EXCITE_DIFFUSE * (1.0f + skew);
            }
        cbs.last_tick_beat = std::floor(t_beats / ZOETROPE_TICK_BEATS) * ZOETROPE_TICK_BEATS;
        return;
    }

    // The checker/ears backward-jump rule (visual_canvas.hpp CADENCE
    // precedent — threshold one span below the next scheduled event,
    // which for a stored last-anchor is the anchor itself): a looping
    // clip re-anchors the clock; the cells PERSIST — the screen keeps
    // its memory across the loop seam.
    if (t_beats < cbs.last_tick_beat)
        cbs.last_tick_beat = std::floor(t_beats / ZOETROPE_TICK_BEATS) * ZOETROPE_TICK_BEATS;

    // Transport-leap guard: more than 64 pending ticks means the clock
    // leapt (a seek, a stall) — re-prime the anchor at the current beat.
    // The cells persist: the automaton never replays.
    if (t_beats - cbs.last_tick_beat > 64.0f * ZOETROPE_TICK_BEATS)
        cbs.last_tick_beat = std::floor(t_beats / ZOETROPE_TICK_BEATS) * ZOETROPE_TICK_BEATS;

    bool ticked = false;
    while (cbs.last_tick_beat + ZOETROPE_TICK_BEATS <= t_beats) {
        ticked = true;
        // One tick. Explicit 4-neighbor diffusion on the cylinder —
        // dir order: 0 = +col (east), 1 = −col (west), 2 = +row (up),
        // 3 = −row (down). Outflow toward a missing row is still shed
        // (open edges); scratch gathers inflows from the OLD field.
        for (uint32_t i = 0; i < LATTICE_CELLS; ++i) cbs.cell_scratch[i] = 0.0f;
        for (uint32_t cell = 0; cell < LATTICE_CELLS; ++cell) {
            const float e = cbs.cells[cell].excite;
            if (e <= 0.0f) continue;
            const uint32_t row = cell / LATTICE_COLS;
            const uint32_t col = cell % LATTICE_COLS;
            cbs.cell_scratch[row * LATTICE_COLS + (col + 1) % LATTICE_COLS]                += e * cbs.wdir[cell][0];
            cbs.cell_scratch[row * LATTICE_COLS + (col + LATTICE_COLS - 1) % LATTICE_COLS] += e * cbs.wdir[cell][1];
            if (row + 1 < LATTICE_ROWS) cbs.cell_scratch[cell + LATTICE_COLS] += e * cbs.wdir[cell][2];
            if (row > 0)                cbs.cell_scratch[cell - LATTICE_COLS] += e * cbs.wdir[cell][3];
        }
        for (uint32_t cell = 0; cell < LATTICE_CELLS; ++cell) {
            ZoetropeCell& c = cbs.cells[cell];
            const float shed = cbs.wdir[cell][0] + cbs.wdir[cell][1]
                             + cbs.wdir[cell][2] + cbs.wdir[cell][3];
            c.excite  = (c.excite - c.excite * shed + cbs.cell_scratch[cell]) * ZOETROPE_EXCITE_DECAY;
            c.pigment = std::min(1.0f, c.pigment * ZOETROPE_PIGMENT_DECAY
                                     + c.excite * ZOETROPE_PIGMENT_GAIN);
        }
        cbs.last_tick_beat += ZOETROPE_TICK_BEATS;
    }

    // The projector's flush (C5): when a tick ran, every mirror-active
    // slot wears its cell — ≤252 12-byte pokes per tick, beneath the
    // spawn-commit idiom. Ghost cells (inactive slots) evolve unseen.
    if (ticked) {
        for (uint32_t slot = 0; slot < LATTICE_CELLS; ++slot) {
            if (!cbs.activeCubes_[slot].active) continue;
            float cr, cg, cb;
            project_cell_color(cbs, active_seed, slot, cr, cg, cb);
            gpu.upload_cube_color(queue, slot, cr, cg, cb);
            // The splay rides the projector (G6): variance relaxes to the
            // mirror's rest as the cell dims.
            const float I = zoetrope_cell_intensity(cbs, slot);
            gpu.upload_cube_face_variance(queue, slot,
                cbs.activeCubes_[slot].face_variance + ZOETROPE_FACE_SPLAY * I);
        }
    }

    // ── THE CLIMB (C6R): the formation walk — a CPU flush-walk on the
    // height scalar, the glide law's grammar; steady state pokes NOTHING.
    using Formation = CubeBehaviorsState::Formation;

    // The release watch: F6 is the ONE door; F7 is the flock's kite duty
    // and never touches the formation. The reveal's own borrow flag
    // falling — by any path — sends every cube walking home. reveal_
    // zoetrope authors that transition at the press itself, so today this
    // is the STANDING GUARD behind it, not the live author: it catches
    // any future path that drops the borrow without moving the state.
    if ((cbs.formation == Formation::TO_SCATTER || cbs.formation == Formation::SCATTERED
         || cbs.formation == Formation::TO_SCREEN || cbs.formation == Formation::SCREEN)
        && !cbs.kite_by_reveal) {
        cbs.formation = Formation::TO_ROAM;
        for (uint32_t i = 0; i < LATTICE_CELLS; i++)
            if (cbs.activeCubes_[i].active) cbs.settled[i] = false;
        std::cout << "[Zoetrope] release: the swarm walks home\n";
    }

    if (cbs.formation == Formation::TO_SCATTER || cbs.formation == Formation::TO_SCREEN
        || cbs.formation == Formation::TO_ROAM) {
        const bool to_screen  = (cbs.formation == Formation::TO_SCREEN);
        const bool to_scatter = (cbs.formation == Formation::TO_SCATTER);
        if (cbs.stage_wait) {
            // The press frame (V1): the sentinel is in flight — no
            // target or body writes until it has eaten.
            cbs.stage_wait = false;
        } else {
            if ((to_screen || to_scatter) && !cbs.stations_sent) {
                // First service after the press: the XZ seats, uniform,
                // one pass — the sentinel ate its 3u last frame. TO_ROAM
                // sends none: the walk home is a walk of BODIES, and the
                // xz stays where the cycle left it (the duty's own 2u
                // freezes it in place if the flock asked to be let go).
                for (uint32_t slot = 0; slot < LATTICE_CELLS; slot++) {
                    if (!cbs.activeCubes_[slot].active) continue;
                    const ZoetropeStation st = to_screen ? zoetrope_station(slot)
                                                         : station_scatter(cbs, slot);
                    gpu.upload_cube_glide_target(queue, slot, st.off_x, st.off_z);
                }
                cbs.stations_sent = true;
            }
            // THE WALK'S TARGETS — one table, one home (H1 E3):
            //   TO_SCATTER : the scatter seat's h; body = the MIRROR's own
            //                spawn draw — the flock keeps its variety
            //   TO_SCREEN  : the screen seat's h; body = the uniform pixel
            //   TO_ROAM    : the mirror's four — THE MIRROR IS THE PRIOR
            // The walker owns its four shadows (G5) and never reads the
            // GPU values while the walk lives. One k for all four;
            // aspects weigh 10× in the settle test (they are ratios).
            // Eviction mid-walk: inactive slots drop from the set; the
            // ghost keeps its cell.
            const float k = 1.0f - std::exp(-dt / ZOETROPE_LIFT_TAU);
            bool all_settled = true;
            for (uint32_t slot = 0; slot < LATTICE_CELLS; slot++) {
                if (!cbs.activeCubes_[slot].active) continue;
                if (cbs.settled[slot]) continue;
                const ActiveCube& ac = cbs.activeCubes_[slot];
                float th, tr, tay, taz;
                if (to_screen) {
                    th = zoetrope_station(slot).h; tr = ZOETROPE_PIXEL_RADIUS;
                    tay = 1.0f; taz = 1.0f;
                } else if (to_scatter) {
                    th = station_scatter(cbs, slot).h;
                    tr = ac.body_radius; tay = ac.aspect_y; taz = ac.aspect_z;
                } else {
                    th = ac.orbit_height; tr = ac.body_radius;
                    tay = ac.aspect_y; taz = ac.aspect_z;
                }
                CubeBehaviorsState::ZoeWalk& w = cbs.walk_[slot];
                w.h  += (th  - w.h)  * k;
                w.r  += (tr  - w.r)  * k;
                w.ay += (tay - w.ay) * k;
                w.az += (taz - w.az) * k;
                const float miss = std::max(
                    std::max(std::fabs(th - w.h), std::fabs(tr - w.r)),
                    10.0f * std::max(std::fabs(tay - w.ay), std::fabs(taz - w.az)));
                if (miss <= ZOETROPE_SETTLE_EPS) {
                    w = { th, tr, tay, taz };
                    cbs.settled[slot] = true;
                } else {
                    all_settled = false;
                }
                gpu.upload_cube_orbit_height(queue, slot, w.h);
                gpu.upload_cube_body_radius(queue, slot, w.r);
                gpu.upload_cube_aspects(queue, slot, w.ay, w.az);
            }
            if (all_settled) {
                cbs.formation = to_screen  ? Formation::SCREEN
                              : to_scatter ? Formation::SCATTERED
                                           : Formation::ROAM;
                std::cout << (to_screen  ? "[Zoetrope] screen: the instrument stands\n"
                            : to_scatter ? "[Zoetrope] gathered: the flock stands drawn in\n"
                                         : "[Zoetrope] roam: the swarm is whole\n");
            }
        }
    }
    // SCATTERED, SCREEN and ROAM poke nothing: steady-state traffic is zero.
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
        }
    }
}

} // namespace the_board
} // namespace t7
