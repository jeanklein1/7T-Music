#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // Dim::*, GPUZoneDeriveRequestArray, wgpu
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT (sizes the mood gate)
#include "cartridges/the_board/contracts/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the keyhole)
#include "cartridges/the_board/contracts/entity_types.hpp"     // GoLSelection/GoLPlacement (the boundary DTOs) + queue types

// ─── gol_zones.hpp (HEADER: vocabulary + state + decls) ──────────
// Converted (LADDER-3 c1): history in audit/LADDER.md.
//
// Zone-local Game of Life + Pulse automata.
//
// The impl additionally reaches the spawn-engine services and
// GLOBAL_ENTITY_DENSITY (spawn_engine.hpp, keyhole form), and
// PATCH_EXTENT (patch_system.hpp); PopFamily is roster.hpp
// vocabulary.
//
// SEAM[gol_zones:complete-subsystem] complete bespoke pipeline in one
//   module — vocabulary + state + lifecycle + dispatch all together.
//   Distinguishable from the cockpit pattern (multiple decoupled
//   commands); this is single-lifecycle bespoke. Same family as
//   gallery and ribbon.
// SEAM[gol_zones:dual-algorithm] this module houses two algorithms —
//   Conway (GoLTierProfile, GOL_TIERS[]) and Pulse (PulseTierProfile,
//   PULSE_TIERS[]) — gated by PULSE_ALGORITHM_CHANCE. The shared
//   infrastructure (zone state, seeding, dispatch) is single-track;
//   only the parameter sampling and life initialization branch.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

// ── Spatial constants ────────────────────────────────────────────
// MUST match world.wgsl's MODE_LATTICE_SPACING (TUNING SURFACE
// DIRECTORY: "MODE_LATTICE_SPACING 120 wu — smooth/discrete
// clusters"). Hardware mirror — when tuning, change both sides.
inline constexpr float MODE_LATTICE_SPACING = 120.0f;
inline constexpr float PATCH_CELL_SIZE = (float)Dim::PATCH_EXTENT / 16.0f;  // 3.125

// ── Algorithm gate ───────────────────────────────────────────────
inline constexpr float PULSE_ALGORITHM_CHANCE = 0.35f;

// ═══ ALGORITHM TYPES (shared) ════════════════════════════════════

struct AlgorithmType {
    static constexpr uint32_t CONWAY = 0;
    static constexpr uint32_t PULSE = 1;
};

struct BoundaryMode {
    static constexpr uint32_t REFLECT = 0;
    static constexpr uint32_t WRAP = 1;
};

// ═══ ZONE-LEVEL VOCABULARY (shared across both algorithms) ═══════

// ── Property Index Registry (seed band 250, indices 920–939) ─────
struct GoLZoneProp {
    static constexpr uint32_t SEED_BAND = 250u;
    // Zone-level decisions
    static constexpr uint32_t SPAWN_ROLL = 920u;
    static constexpr uint32_t TIER = 921u;
    static constexpr uint32_t HEIGHT_ROLL = 922u;
    static constexpr uint32_t COLOR_ROLL = 923u;
    // Per-zone continuous parameters (Gaussian draws)
    static constexpr uint32_t DENSITY = 930u;
    static constexpr uint32_t TICK_PERIOD = 931u;
    static constexpr uint32_t SPRING = 932u;
    static constexpr uint32_t HEIGHT = 933u;
    static constexpr uint32_t TRANSITION = 934u;
    // Per-zone color target
    static constexpr uint32_t TARGET_R = 935u;
    static constexpr uint32_t TARGET_G = 936u;
    static constexpr uint32_t TARGET_B = 937u;
    // Per-cell seeding
    static constexpr uint32_t HEIGHT_FACTOR = 938u;
};

// ── Spawn Configuration ──────────────────────────────────────────
struct GoLZoneSpawnConfig {
    static constexpr float SPAWN_CHANCE = 0.15f;  // fraction of checkerboard zones
    static constexpr float HEIGHT_CHANCE = 0.30f;  // fraction of zones that get extrusion
    static constexpr float ZONE_EXTENT = 100.0f; // 32 × 3.125 = cell-aligned
    static constexpr float MODE_THRESHOLD = 0.50f;  // min interpolated mode for eligibility
    // Per-cell height factor seeding (Gaussian draw per cell)
    static constexpr float HEIGHT_FACTOR_MEAN = 1.0f;
    static constexpr float HEIGHT_FACTOR_SIGMA = 0.15f;
    static constexpr float HEIGHT_FACTOR_CLAMP_LO = 0.6f;
    static constexpr float HEIGHT_FACTOR_CLAMP_HI = 1.4f;
    // Lens target color range: color = hash * RANGE + LO
    static constexpr float LENS_TARGET_LO = 0.2f;
    static constexpr float LENS_TARGET_RANGE = 0.6f;
    // Footprint: inscribed circle of 100×100 zone
    static constexpr float FOOTPRINT_RADIUS = 50.0f;
    // SEAM[gol_zones:P4] hygiene rows pattern (P4): MOOD_MULTIPLIER
    //   is { open, sunset, [indoor_flat=0], [indoor_vault=0],
    //   [finite_outdoor=1], [finite_outdoor_ref=0] }. The 0
    //   entries are reachable via mood IDs but the gate
    //   intentionally suppresses them. Same family as
    //   floaters:P4 (cube populations). Defensive declaration.
    // Mood gate (suppressed in flat/vault/finR — same as spheres/cubes)
    static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
};

// ── Color Modes ──────────────────────────────────────────────────
struct GoLColorMode {
    static constexpr uint32_t NEUTRAL = 0;  // no color change (height-only extrusion)
    static constexpr uint32_t LENS = 1;  // shift toward per-zone target color
    static constexpr uint32_t BLACKISH = 2;  // darken toward near-black
    static constexpr uint32_t COUNT = 3;

    static constexpr float WEIGHTS_HEIGHT[COUNT] = { 0.30f, 0.40f, 0.30f };
    static constexpr float WEIGHTS_NO_HEIGHT[COUNT] = { 0.00f, 0.55f, 0.45f };
};

// ═══ CONWAY ALGORITHM ════════════════════════════════════════════

// ── Tier Profile (mean+sigma, matches ColumnTierParams pattern) ──
inline constexpr uint32_t GOL_TIER_COUNT = 7;

struct GoLTierProfile {
    // ─── Initial conditions ──────────────────────────────────
    float density_mean, density_sigma;

    // ─── Temporal ────────────────────────────────────────────
    float tick_period_mean, tick_period_sigma;

    // ─── Visual transition ───────────────────────────────────
    float spring_stiffness_mean, spring_stiffness_sigma;
    float transition_fraction_mean, transition_fraction_sigma;

    // ─── Height ──────────────────────────────────────────────
    float alive_height_mean, alive_height_sigma;

    // ─── Per-cell variation ──────────────────────────────────
    float spring_variance;     // [0,1] per-cell spring speed scatter

    // ─── Selection ───────────────────────────────────────────
    float weight;
    bool  force_no_height;
};

//                                          dens_μ   σ    tick_μ  σ    spring_μ σ    trans_μ  σ     ht_μ    σ    sv    wt    no_h
inline constexpr GoLTierProfile GOL_TIERS[GOL_TIER_COUNT] = {
    /* 0: Pillars  */ { 0.30f, 0.05f,   8.0f, 2.0f,   0.5f, 0.1f,   0.05f, 0.01f,  30.0f, 9.0f,  0.30f,  0.10f, false },
    /* 1: Sparse   */ { 0.15f, 0.05f,   2.0f, 0.5f,   4.0f, 1.0f,   0.12f, 0.03f,  18.0f, 6.0f,  0.20f,  0.20f, false },
    /* 2: Moderate */ { 0.30f, 0.08f,   1.0f, 0.3f,   8.0f, 2.0f,   0.15f, 0.03f,   9.0f, 3.0f,  0.15f,  0.18f, false },
    /* 3: Dense    */ { 0.45f, 0.10f,   0.5f, 0.15f, 12.0f, 3.0f,   0.25f, 0.05f,   6.0f, 1.5f,  0.10f,  0.10f, false },
    /* 4: Flash    */ { 0.35f, 0.10f,  0.25f, 0.05f, 20.0f, 5.0f,   0.30f, 0.05f,   0.0f, 0.0f,  0.40f,  0.17f, true  },
    /* 5: Monolith */ { 0.20f, 0.03f,  12.0f, 3.0f,   0.3f, 0.05f,  0.03f, 0.01f,  42.0f, 12.f,  0.05f,  0.12f, false },
    /* 6: Glacier  */ { 0.12f, 0.03f,   4.0f, 1.0f,   2.0f, 0.5f,   0.08f, 0.02f,  24.0f, 7.5f,  0.25f,  0.13f, false },
};

inline constexpr const char* GOL_TIER_NAMES[] = {
    "Pillars", "Sparse", "Moderate", "Dense",
    "Flash", "Monolith", "Glacier"
};

inline constexpr const char* GOL_COLOR_NAMES[] = {
    "neutral", "lens", "blackish"
};

// ═══ PULSE ALGORITHM ═════════════════════════════════════════════

// ── Property Indices for Pulse-specific parameters ───────────────
struct PulseZoneProp {
    static constexpr uint32_t ALGORITHM_ROLL = 950u;
    static constexpr uint32_t PULSE_TIER = 951u;
    static constexpr uint32_t PHASE_RANDOM = 952u;
    static constexpr uint32_t WANDER = 953u;
    static constexpr uint32_t TEMPO_RANDOM = 954u;
};

// ── Pulse Tier Profile ───────────────────────────────────────────
inline constexpr uint32_t PULSE_TIER_COUNT = 3;

struct PulseTierProfile {
    // ─── Temporal ────────────────────────────────────────────
    float tick_period_mean, tick_period_sigma;

    // ─── Visual transition ───────────────────────────────────
    float spring_stiffness_mean, spring_stiffness_sigma;
    float transition_fraction_mean, transition_fraction_sigma;

    // ─── Phase scatter ───────────────────────────────────────
    float phase_randomness_mean, phase_randomness_sigma;

    // ─── Tempo scatter ───────────────────────────────────────
    float tempo_randomness_mean, tempo_randomness_sigma;

    // ─── Height ──────────────────────────────────────────────
    float alive_height_mean, alive_height_sigma;

    // ─── Wander ──────────────────────────────────────────────
    float wander_radius_mean, wander_radius_sigma;

    // ─── Per-cell variation ──────────────────────────────────
    float spring_variance;

    // ─── Selection ───────────────────────────────────────────
    float weight;
    bool  force_no_height;
    uint32_t boundary_mode;
};

//                                              tick_μ   σ    spring_μ σ    trans_μ  σ    phase_μ  σ    tempo_μ σ    ht_μ   σ    wand_μ  σ    sv    wt    no_h  bnd
inline constexpr PulseTierProfile PULSE_TIERS[PULSE_TIER_COUNT] = {
    /* 0: Breathe  */ { 2.0f, 0.5f,   4.0f, 1.0f,   0.20f, 0.05f,   0.15f, 0.05f,   0.10f, 0.03f,   2.0f, 0.8f,  10.0f, 3.0f,   0.20f,  0.45f, false, BoundaryMode::REFLECT },
    /* 1: Sparkle  */ { 0.5f, 0.15f, 12.0f, 3.0f,   0.25f, 0.05f,   0.90f, 0.10f,   0.60f, 0.15f,   0.0f, 0.0f,   5.0f, 2.0f,   0.50f,  0.30f, true,  BoundaryMode::REFLECT },
    /* 2: Drift    */ { 4.0f, 1.0f,   1.5f, 0.4f,   0.10f, 0.03f,   0.50f, 0.15f,   0.40f, 0.10f,   4.0f, 1.5f,  25.0f, 8.0f,   0.35f,  0.25f, false, BoundaryMode::WRAP    },
};

inline constexpr const char* PULSE_TIER_NAMES[] = {
    "Breathe", "Sparkle", "Drift"
};

// ═══ SPAWN PAYLOADS — AT THE CONTRACT HOME ═══════════════════════
//
// The GoL Selection/Placement DTOs live in entity_types.hpp,
// beside the EntityQueueEntry / PlacementEntry unions that are their
// reason to exist: a DTO that exists to cross a boundary belongs to
// the boundary's contract, not to either side.

// ═══ RUNTIME CPU STATE ═══════════════════════════════════════════

// ── Per-instance zone state ──────────────────────────────────────
struct GoLZoneState {
    int32_t zone_nx = 0, zone_nz = 0;
    int32_t host_gx = 0, host_gz = 0;   // host patch (for entity_refs eviction)
    bool active = false;
    uint32_t algorithm = AlgorithmType::CONWAY;
    float tick_period = 1.0f;        // CPU derives this for tick mask (matches GPU)
    float initial_density = 0.3f;    // CPU needs this for life buffer seeding
    int32_t last_tick_index = -1;
};

// ── GoL module state ──────────────────────────────────────────
struct GoLState {
    GoLZoneState zones[Dim::MAX_GOL_ZONES]{};
    uint32_t     zone_count = 0;
    uint32_t     active_slot_count = 0;     // highest active slot + 1 (for dispatch sizing)

    bool         mood_allowed = true;

    // Derive request queue: accumulated during patch gen, flushed once
    // per frame as a single GPU compute dispatch (zone_derive_params).
    GPUZoneDeriveRequestArray pending_derive_requests{};
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Lifecycle (three-phase + helper)
bool select_gol_for_patch(GoLState& gs, MachineCtx* c,
    int32_t gx, int32_t gz, GoLSelection& sel);
bool place_gol_from_selection(MachineCtx* c,
    const GoLSelection& sel, GoLPlacement& plan);
void commit_gol(GoLState& gs, MachineCtx* c,
    const GoLPlacement& plan,
    int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue);
// The evictor — keyhole-shaped
// to match the FAMILY_DISPATCH evict slot (table in family_dispatch.inl)
void evict_gol(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
// Dispatch funnels (table-shaped; the FAMILY_DISPATCH rows point here)
bool dispatch_select_gol(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_gol(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_gol(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue);
void seed_gol_zone(GoLState& gs, MachineCtx* c,
    uint32_t slot, wgpu::Queue& queue);
// Per-frame
void upload_gol_zone_config(GoLState& gs, Cartridge* c, wgpu::Queue& queue);
void flush_zone_derive_requests(GoLState& gs, Cartridge* c, wgpu::Queue& queue);
void teardown_gol(Cartridge* c, wgpu::Queue& queue);
void dispatch_zone_sync(GoLState& gs, Cartridge* c, wgpu::CommandEncoder& encoder);
void dispatch_zone_evolve(GoLState& gs, Cartridge* c, wgpu::CommandEncoder& encoder);
void dispatch_zone_mesh(GoLState& gs, Cartridge* c, wgpu::CommandEncoder& encoder);

} // namespace the_board
} // namespace t7
