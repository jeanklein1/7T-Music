#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // Dim::*, wgpu
#include "cartridges/the_board/contracts/automaton_surface.hpp"           // AUTO_TABLE — the transcription witness at the bottom pins it here
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"     // GoLSelection/GoLPlacement (the boundary DTOs) + queue types

// ─── gol_zones.hpp (HEADER: vocabulary + state + decls) ──────────
//
// Zone-local Game of Life + Pulse automata.
//
// The impl additionally reaches the spawn-engine services and
// GLOBAL_ENTITY_DENSITY (contracts/spawn_services.hpp), and
// Dim::PATCH_EXTENT (patch_system.hpp); PopFamily is roster.hpp
// vocabulary.
//
// SEAM[gol_zones:complete-subsystem] complete bespoke pipeline in one
//   module — vocabulary + state + lifecycle + dispatch all together.
//   Distinguishable from the cockpit pattern (multiple decoupled
//   commands); this is single-lifecycle bespoke. Same family as
//   ribbon.
// SEAM[gol_zones:dual-algorithm] this module houses two algorithms —
//   Conway (GoLTierProfile, GOL_TIERS[]) and Pulse (GolPulseTierProfile,
//   GOL_PULSE_TIERS[]) — gated by GOL_PULSE_ALGORITHM_CHANCE. The shared
//   infrastructure (zone state, seeding, dispatch) is single-track;
//   only the parameter sampling and life initialization branch.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::floor, std::hypot (the footprint radius)   // (impl, merged)
#include <algorithm>  // std::max, std::min   // (impl, merged)
#include <iostream>   // the spawn log   // (impl, merged)
#include "core/instruments.hpp"   // PURSE_0 R3 — INSTRUMENTS.stream_witness gates the spawn log
#include <vector>     // life / height-factor staging   // (impl, merged)

namespace t7 {
namespace the_board {

// ═══ SEAM[gol:derive-submit] — TOMBSTONE (ONE_SURFACE-II U1) ═════════
//
// The banner that stood here declared a module dep and a prohibition:
// GolDeps carried `wgpu::Device&` as a DECLARED handover (stamp S5)
// because flush_zone_derive_requests submitted its derive pass on its
// OWN encoder, MID-RENDER, and that submit MUST execute before the same
// frame's agent kernels. Folding it into the frame encoder was FORBIDDEN
// — the program's one standing refactor prohibition.
//
// THE PROHIBITION IS LIFTED BY ITS CAUSE LEAVING, not by anyone deciding
// it was safe after all. The seam existed for RUNTIME ZONE SPAWNS: a
// zone born mid-frame had to have its parameters derived before the
// kernels that read them ran, in that frame. The automaton is seeded at
// BIRTH, on the birth encoder, outside the frame loop, and nothing
// spawns after — so there is no mid-frame submit to order, no device
// handover to declare, and nothing left to forbid. The frame has one
// submit again.
//
// (ONE_SURFACE-I had already taken half its premise: it removed the only
// thing that created patches after birth, so nothing could host a new
// zone. U1 removed the other half.)
class Renderer;
struct GolDeps {
    GPUState&        gpuState_;
    Renderer&        renderer_;
    wgpu::Device&    device_;   // SEAM[gol:derive-submit] — immediate mid-render submit; never folds into the frame encoder
    const TimeState& time_state_;  // upload_gol_zone_config reads beats/dt for the header
};

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

// ── Spatial constants ────────────────────────────────────────────
// MUST match world.wgsl's MODE_LATTICE_SPACING (TUNING SURFACE
// DIRECTORY: "MODE_LATTICE_SPACING 120 wu — smooth/discrete
// clusters"). Hardware mirror — when tuning, change both sides.
inline constexpr float MODE_LATTICE_SPACING = 120.0f;

// ── Algorithm gate ───────────────────────────────────────────────
inline constexpr float GOL_PULSE_ALGORITHM_CHANCE = 0.35f;

// ═══ ALGORITHM TYPES (shared) ════════════════════════════════════

struct AlgorithmType {
    static constexpr uint32_t CONWAY = 0;
    static constexpr uint32_t PULSE = 1;
};

struct BoundaryMode {
    static constexpr uint32_t REFLECT = 0;
    static constexpr uint32_t WRAP = 1;
};

// The Pulse rows' field function: which spatial law writes the per-cell
// target. BREATH is the per-cell sinusoid the algorithm shipped with.
// L3 MIRROR: world.wgsl PULSE_FIELD_*.
struct PulseField {
    static constexpr uint32_t BREATH = 0;   // what ships today
    static constexpr uint32_t SPIRAL = 1;
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
    static constexpr float SPAWN_CHANCE = 0.60f;  // fraction of checkerboard zones
    // Fraction of zones that get extrusion. The roll refuses no zone; the
    // only flat zones left are the three tiers whose identity is flatness
    // (Conway Flash, Pulse Sparkle and Pulse Spiral set force_no_height),
    // so the delivered rate is 0.8335. Recipe: each family's weights sum
    // to 1, and GOL_PULSE_ALGORITHM_CHANCE = 0.35 splits them, so the flat
    // weight is 0.65 x Flash 0.03 + 0.35 x (Sparkle 0.24 + Spiral 0.18)
    // = 0.0195 + 0.147 = 0.1665, and 1.0 - 0.1665 = 0.8335. See the tier
    // tables below.
    // (TUNE_1 A10 moved Flash 0.17 -> 0.03, which is an INPUT to this
    //  recipe. As of A10 flat zones went 0.2155 -> 0.1245 — about one in
    //  eight, not rare — and Pulse Sparkle, untouched by A10, owned 84%
    //  of what flatness remained.
    //  GOL_RULES_1 is this recipe's SECOND input: it reweighted both
    //  families and added Spiral, a second force_no_height Pulse row.
    //  Flat zones are now 0.1665 — about one in six — and the flat weight
    //  splits Flash 12% / Sparkle 50% / Spiral 38%.)
    static constexpr float HEIGHT_CHANCE = 1.00f;
    static constexpr float MODE_THRESHOLD = 0.50f;  // min interpolated mode for eligibility
    // Per-cell height factor seeding (Gaussian draw per cell)
    static constexpr float HEIGHT_FACTOR_MEAN = 1.0f;
    static constexpr float HEIGHT_FACTOR_SIGMA = 0.15f;
    static constexpr float HEIGHT_FACTOR_CLAMP_LO = 0.6f;
    // L3 MIRROR: world.wgsl GOL_HEIGHT_FACTOR_MAX. This is the upper bound on
    // the per-cell multiplier, and the rooms' height cap divided by it at
    // zone_derive_params so the capped lift is exact. Change both rooms.
    static constexpr float HEIGHT_FACTOR_CLAMP_HI = 1.4f;
    // Lens target color range: color = hash * RANGE + LO
    static constexpr float LENS_TARGET_LO = 0.2f;
    static constexpr float LENS_TARGET_RANGE = 0.6f;
    // The GoL mood row and the veto it armed left at ONE_WORLD-II U3.
    //   The column was all 1.0, so the term rested at identity and
    //   suppressed nothing; the veto was the live mechanism awaiting a
    //   value that never came, and it was GoL's alone.
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

// ── Tier Profile (mean+sigma, the per-family TierRow pattern) ──
inline constexpr uint32_t GOL_TIER_COUNT = 10;

struct GoLTierProfile {
    // ─── Rule ────────────────────────────────────────────────
    // Conway B/S as a bitset: bit n = birth on n neighbours, bit 9+n =
    // survival on n. B3/S23 is 0x1808u. L3 MIRROR: world.wgsl GOL_TIERS.
    uint32_t rule_mask;

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

    // ─── Size (UNIFIED_GROUND_1 U5; cells, not world units) ──
    uint32_t grid_cells;       // zone side in cells ∈ {8..32}
};

// MUST match world.wgsl's GOL_TIERS cells column. Hardware mirror — when
// tuning, change both sides.
// PROVENANCE, no longer an invariant: the cells column was authored by
// UNIFIED_GROUND_1 U5 as "defaults by weight order thirds, 32/24/16", and
// at that time weight-descending order gave 32,32,24,24,16,16,16. TUNE_1
// A10 re-ranked the weights without re-authoring the cells (cells were not
// in its bind), so the order now reads 24,32,16,16,32,16,24. The column is
// Jean-tunable per row and its VALUES are unchanged; only the descending-
// rank pattern is gone. Re-author the cells if the pattern is wanted back.
// (GOL_RULES_1 reweighted the seven again and appended three rows, so the
//  seven-value list above describes the original rows only.)
//                                     rule       dens_μ   σ    tick_μ  σ    spring_μ σ    trans_μ  σ     ht_μ    σ    sv    wt    no_h   cells
inline constexpr GoLTierProfile GOL_TIERS[GOL_TIER_COUNT] = {
    /* 0: Pillars  */ { 0x1808u,  0.30f, 0.05f,  16.0f, 4.0f,   0.5f, 0.1f,   0.05f, 0.01f,  30.0f, 9.0f,  0.30f,  0.11f, false, 16u },
    /* 1: Sparse   */ { 0x1808u,  0.15f, 0.05f,   4.0f, 1.0f,   4.0f, 1.0f,   0.12f, 0.03f,  18.0f, 6.0f,  0.20f,  0.17f, false, 32u },
    /* 2: Moderate */ { 0x1808u,  0.30f, 0.08f,   2.0f, 0.6f,   8.0f, 2.0f,   0.15f, 0.03f,   9.0f, 3.0f,  0.15f,  0.09f, false, 32u },
    /* 3: Dense    */ { 0x1808u,  0.45f, 0.10f,   1.0f,  0.3f, 12.0f, 3.0f,   0.25f, 0.05f,   6.0f, 1.5f,  0.10f,  0.03f, false, 16u },
    /* 4: Flash    */ { 0x1808u,  0.35f, 0.10f,   0.5f,  0.1f, 20.0f, 5.0f,   0.30f, 0.05f,   0.0f, 0.0f,  0.40f,  0.03f, true,  24u },
    /* 5: Monolith */ { 0x1808u,  0.20f, 0.03f,  24.0f, 6.0f,   0.3f, 0.05f,  0.03f, 0.01f,  42.0f, 12.f,  0.05f,  0.12f, false, 16u },
    /* 6: Glacier  */ { 0x1808u,  0.12f, 0.03f,   8.0f, 2.0f,   2.0f, 0.5f,   0.08f, 0.02f,  24.0f, 7.5f,  0.25f,  0.21f, false, 24u },
    // GOL_TEMPO_1 doubled tick_period_mean and _sigma on EVERY row of both
    // tables — the whole board at half rate. It is one column because
    // transition_fraction is a FRACTION: the extrusion lasts
    // transition_fraction x tick_period, so doubling the period doubles
    // the update interval and the rise-and-sink time together and leaves
    // every row's duty cycle where its author put it. Do not reach for
    // config.mode_gol_tick_scale to do this: that dial is read only by
    // pulse_cell_target, so it would slow the Pulse fields and leave both
    // Conway's tick gate and every spring at speed.
    //
    // GOL_RULES_1 authored these three rules. GOL_ROWS_1 re-authored two
    // of the row VALUES from the headless witness, which overturned the
    // belief they had been tuned around: Cauldron never terminates and
    // Day & night did — the opposite of the assumption. GOL_ROWS_2 then
    // retimed Cauldron for a boil, and GOL_ROWS_3 replaced Day & night's
    // MASK outright: it reached one uniform extreme or the other too
    // often for the slot, and a majority rule fills that slot without the
    // failure mode. Every number below comes from tools/gol_census.py.
    // Read them as intent:
    //  · Plateau is the majority rule (Vote, B5678/S45678): a cell takes
    //    the state most of its neighbourhood is already in. What matters
    //    here is that it PINS at its interfaces rather than coarsening
    //    all the way to consensus — across a straight edge the live cell
    //    sees exactly 5 and survives, the dead cell sees 3 and is not
    //    born, so the boundary is itself a fixed point. Domains smooth
    //    their edges and then stop, and neither extreme is reachable from
    //    a 0.50 seed. Censused at these values over 32 zone seeds: 0/32
    //    dark, 0/32 saturated, 32/32 structured, 32/32 reaching a true
    //    fixed point, ~43% live; over 512 seeds the dark rate is 0.4%.
    //    Dark is the failure the census watches — such a zone renders
    //    nothing (no height, and the tint's color_val > 0.01 fails) while
    //    still holding its footprint against every other zone. This slot
    //    held Day & night twice before, at 0.50 and again biased to 0.58,
    //    and reached one uniform extreme or the other about a third and a
    //    fifth of the time. Those numbers, and the next candidate's, come
    //    from tools/gol_census.py. The treatment was authored for a
    //    terminal, structured, tall row, and this is the first mask that
    //    is one: a slow tick with a crisp rise inside it
    //    (omega = 3 / (0.10 x 8.0) = 3.75) and low variance. Plateaus that
    //    commit and then hold.
    //  · Cauldron, once "Walled cities", builds no walls at any reachable
    //    size: 0 of 32 seeds reached a fixed point in 4000 generations and
    //    not one cell was static, at every size 24..128 and every density
    //    0.25..0.65. It is a dense boil at ~53% live, and it is named for
    //    what it does. Mask, density and cells are the rule's own
    //    requirement and stay. LOW height is what makes a permanently
    //    churning field a textured surface instead of a strobing forest.
    //    The transition 0.40 is the load-bearing number: the cell spends
    //    about two fifths of its tick in transit, so the field shimmers
    //    continuously instead of stepping. THE TICK HAS BEEN BOTH WAYS.
    //    GOL_ROWS_2 took it 6.0 -> 2.5 because at 120bpm 6.0 was three
    //    seconds a generation, slower than a boil can be seen to be;
    //    GOL_TEMPO_1 then halved the whole board's rate and carried this
    //    row with it, 2.5 -> 5.0, which is two and a half seconds a
    //    generation and back within reach of the argument that rejected
    //    6.0 (omega = 3 / (0.40 x 5.0) = 1.5). If the boil stops reading
    //    as one, this row is the first place to look and 2.5 is where it
    //    was.
    //  · HighLife must be 32 cells or the replicator has no room and the
    //    row reads as thin Conway. Brisk tick so replication is visible.
    //    Untouched by GOL_ROWS_1 — the witness ran Hickerson's replicator
    //    on this row's own grid and watched one 12-cell seed become two.
    /* 7: Plateau  */ { 0x3E1E0u, 0.50f, 0.06f,   8.0f, 2.0f,   6.0f, 1.5f,   0.10f, 0.02f,  30.0f, 8.0f,  0.08f,  0.09f, false, 32u },
    /* 8: Cauldron */ { 0x79F0u,  0.50f, 0.05f,   5.0f, 1.2f,   1.2f, 0.3f,   0.40f, 0.08f,   5.0f, 1.5f,  0.15f,  0.08f, false, 24u },
    /* 9: HighLife */ { 0x1848u,  0.30f, 0.05f,   1.2f,  0.3f,  9.0f, 2.0f,   0.20f, 0.04f,  10.0f, 3.0f,  0.22f,  0.07f, false, 32u },
};

inline constexpr const char* GOL_TIER_NAMES[] = {
    "Pillars", "Sparse", "Moderate", "Dense",
    "Flash", "Monolith", "Glacier",
    "Plateau", "Cauldron", "HighLife"
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
inline constexpr uint32_t GOL_PULSE_TIER_COUNT = 4;

struct GolPulseTierProfile {
    // ─── Field ───────────────────────────────────────────────
    uint32_t field_fn;         // PulseField:: — which law writes the target

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

    // ─── Size (UNIFIED_GROUND_1 U5; cells, not world units) ──
    uint32_t grid_cells;       // zone side in cells ∈ {8..32}
};

// MUST match world.wgsl's GOL_PULSE_TIERS cells column
// (UNIFIED_GROUND_1 U5 — "32/16/8 by weight order"). Hardware
// mirror — when tuning, change both sides.
//                                     field                  tick_μ   σ    spring_μ σ    trans_μ  σ    phase_μ  σ    tempo_μ σ    ht_μ   σ    wand_μ  σ    sv    wt    no_h  bnd                    cells
inline constexpr GolPulseTierProfile GOL_PULSE_TIERS[GOL_PULSE_TIER_COUNT] = {
    /* 0: Breathe  */ { PulseField::BREATH,  4.0f, 1.0f,   4.0f, 1.0f,   0.20f, 0.05f,   0.15f, 0.05f,   0.10f, 0.03f,   2.0f, 0.8f,  10.0f, 3.0f,   0.20f,  0.38f, false, BoundaryMode::REFLECT, 32u },
    /* 1: Sparkle  */ { PulseField::BREATH,  1.0f,  0.3f, 12.0f, 3.0f,   0.25f, 0.05f,   0.90f, 0.10f,   0.60f, 0.15f,   0.0f, 0.0f,   5.0f, 2.0f,   0.50f,  0.24f, true,  BoundaryMode::REFLECT, 16u },
    /* 2: Drift    */ { PulseField::BREATH,  8.0f, 2.0f,   1.5f, 0.4f,   0.10f, 0.03f,   0.50f, 0.15f,   0.40f, 0.10f,   4.0f, 1.5f,  25.0f, 8.0f,   0.35f,  0.20f, false, BoundaryMode::WRAP, 8u },
    // GOL_RULES_1. The first Pulse row that is not BREATH, and the first
    // whose target is continuous rather than binary. Read the values as
    // intent:
    //  · the continuous target is TRACKED rather than smeared by
    //    omega = 3 / (0.30 x 6.0) = 1.67. transition_fraction x
    //    tick_period IS the spring, so GOL_TEMPO_1's doubling of the tick
    //    halved the rotation AND halved the rise in one edit — which is
    //    why the fraction was the column that must not move. The
    //    spring_stiffness column beside it is written and never read
    //    (docs/OPEN.md carries that);
    //  · phase scatter stays at 0.03; TEMPO SCATTER IS ZERO (GOL_ROWS_1).
    //    The two are not the same kind of term: tempo is a per-cell
    //    FREQUENCY multiplier, so its phase error integrates in t_beats
    //    and never saturates, while phase is a bounded static offset. At
    //    tempo 0.02 the arms measured 0.99 correlation against a
    //    scatter-free spiral at 20 beats, 0.66 at 75, and 0.02 by 150 —
    //    gone inside a minute at 120bpm. (Those beat counts were measured
    //    at the then-current 3.0 tick; the decoherence scales with the
    //    period, so at 6.0 the same collapse takes twice as long. The
    //    value is 0 and none of it happens.) Coherence is the point, and
    //    only one of the two scatters could ever keep it;
    //  · wander_radius 0 — the spiral centre IS the zone centre and does
    //    not move; a wandering centre smears the arms;
    //  · 32 cells, because a spiral does not read at 16;
    //  · force_no_height, which is also the safety of the fractional
    //    target: select_gol_zone forces height_enabled = false for such
    //    rows, so height never reads a fractional visual. Tint does, and
    //    that is the intent.
    /* 3: Spiral   */ { PulseField::SPIRAL,  6.0f, 1.6f,   8.0f, 2.0f,   0.30f, 0.06f,   0.03f, 0.01f,    0.0f, 0.0f,   0.0f, 0.0f,   0.0f, 0.0f,   0.10f,  0.18f, true,  BoundaryMode::WRAP, 32u },
};

inline constexpr const char* GOL_PULSE_TIER_NAMES[] = {
    "Breathe", "Sparkle", "Drift", "Spiral"
};

// ═══ THE FAMILY'S RESIDUE (U2 removes it) ════════════════════════

// The family's slot capacity. It was Dim::MAX_GOL_ZONES and it is local
// now, because it is no longer a fact about the WORLD — the automaton's
// capacity is Dim::AUTO_GRID_MAX — only about how many rows a roster
// family that spawns nothing still has to carry.
inline constexpr uint32_t MAX_GOL_SLOTS = 8;

// ── Per-slot state, down to what the census reads ────────────────
// GoLZoneState carried lattice-node coordinates, a host patch, a
// persisted world footprint (corner + extent), an algorithm, a tick
// period, an initial density and a tick cursor. Every one of those
// described an ISLAND. What the tree still touches is `active`, through
// census_scan_active / census_scan_slots.
struct GoLZoneState {
    bool active = false;
};

// ── Module state ─────────────────────────────────────────────────
// active_slot_count (the dispatch high-water mark), zones_allowed's
// twin gate and pending_derive_requests all left with the machinery
// they sized. `zones_allowed` stays because sky.hpp writes it — the
// [sky -> gol] flag channel — and that channel is U2's to close.
struct GoLState {
    GoLZoneState zones[MAX_GOL_SLOTS]{};
    uint32_t     zone_count = 0;
    bool         zones_allowed = true;
};

// ═══ THE THREE FUNNELS, AT THE NONE-FORK ═════════════════════════
//
// The lifecycle they fronted — select a lattice node, place a zone in a
// free slot, commit it with a derive request and a seeded life buffer —
// has no subject. They keep their signatures because FAMILY_DISPATCH is
// a POSITIONAL TABLE and its GOL column must still point at something
// with the right shape until U2 re-columns it. This is the shape the
// tree already uses for a family hook with nothing behind it (the
// pyramid's mesh hook took the same route).

inline bool dispatch_select_gol(MachineCtx* self,
    int32_t gx, int32_t gz, EntityQueueEntry& e) {
    (void)self; (void)gx; (void)gz; (void)e;
    return false;   // nothing to select: the Game of Life is the ground now
}

inline bool dispatch_place_gol(MachineCtx* self,
    EntityQueueEntry& e, PlacementEntry& pe) {
    (void)self; (void)e; (void)pe;
    return false;   // unreachable: select never yields
}

inline void dispatch_commit_gol(MachineCtx* self,
    PlacementEntry& pe, wgpu::Queue& queue) {
    (void)self; (void)pe; (void)queue;
    // unreachable: place never yields
}

// ═══ THE TRANSCRIPTION WITNESS — SPENT AT U2 ═════════════════════
//
// AUTO_TABLE's literals were typed by hand into
// contracts/automaton_surface.hpp while these tables still stood, for
// ATMOS_TABLE's reason stated verbatim there: a `= GOL_TIERS[6]`
// initializer would read well and then die with the table, leaving the
// numbers to be typed at the one moment nothing could check them.
//
// These asserts are that check. They do their whole job in this commit
// and leave with the tables at U2; the numbers they prove stay.
//
// PROVEN TO BITE, as Amendment A requires of any net: the injection was
// AUTO_TABLE.density 0.12f -> 0.13f, which fails the first assert below
// with its own message. Reverted; the assert stands.

// ── Conway: GOL_TIERS[6] "Glacier", the highest-weight row (0.21) ──
static_assert(AUTO_TABLE.density             == GOL_TIERS[6].density_mean
           && AUTO_TABLE.density_spread      == GOL_TIERS[6].density_sigma,
    "AUTO_TABLE density is Glacier's, transcribed");
static_assert(AUTO_TABLE.tick_period         == GOL_TIERS[6].tick_period_mean
           && AUTO_TABLE.tick_period_spread  == GOL_TIERS[6].tick_period_sigma,
    "AUTO_TABLE tick_period is Glacier's, transcribed");
static_assert(AUTO_TABLE.transition_fraction        == GOL_TIERS[6].transition_fraction_mean
           && AUTO_TABLE.transition_fraction_spread == GOL_TIERS[6].transition_fraction_sigma,
    "AUTO_TABLE transition_fraction is Glacier's, transcribed");
static_assert(AUTO_TABLE.alive_height        == GOL_TIERS[6].alive_height_mean
           && AUTO_TABLE.alive_height_spread == GOL_TIERS[6].alive_height_sigma,
    "AUTO_TABLE alive_height is Glacier's, transcribed");
static_assert(AUTO_TABLE.spring_variance     == GOL_TIERS[6].spring_variance,
    "AUTO_TABLE spring_variance is Glacier's, transcribed");
static_assert(AUTO_TABLE.rule_mask           == GOL_TIERS[6].rule_mask,
    "AUTO_TABLE rule_mask is Glacier's B3/S23, transcribed");

// GLACIER IS THE MODAL ROW, and the assert says so in arithmetic rather
// than in prose: no other Conway tier outweighs it. That is the whole
// justification for picking one of ten, so it is checked, not claimed.
static_assert(GOL_TIERS[6].weight >= GOL_TIERS[0].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[1].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[2].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[3].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[4].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[5].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[7].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[8].weight
           && GOL_TIERS[6].weight >= GOL_TIERS[9].weight,
    "the transcription source must be the row the retiring world drew most "
    "often; Glacier is that row and this is the check, not the claim");

// ── Pulse: GOL_PULSE_TIERS[0] "Breathe", the highest-weight row (0.38) ──
// Only the PULSE-ONLY columns come from here — the ones a Conway world
// never reads, so that flipping `algorithm` to PULSE lands on the modal
// Pulse world rather than on zeros.
static_assert(AUTO_TABLE.phase_randomness == GOL_PULSE_TIERS[0].phase_randomness_mean,
    "AUTO_TABLE phase_randomness is Breathe's, transcribed");
static_assert(AUTO_TABLE.tempo_randomness == GOL_PULSE_TIERS[0].tempo_randomness_mean,
    "AUTO_TABLE tempo_randomness is Breathe's, transcribed");
static_assert(AUTO_TABLE.field_fn == GOL_PULSE_TIERS[0].field_fn,
    "AUTO_TABLE field_fn is Breathe's BREATH, transcribed");
static_assert(GOL_PULSE_TIERS[0].weight >= GOL_PULSE_TIERS[1].weight
           && GOL_PULSE_TIERS[0].weight >= GOL_PULSE_TIERS[2].weight
           && GOL_PULSE_TIERS[0].weight >= GOL_PULSE_TIERS[3].weight,
    "and Breathe is the modal Pulse row");

// ── The shared spawn config: the per-cell height factor ──
static_assert(AUTO_TABLE.height_factor_mean  == GoLZoneSpawnConfig::HEIGHT_FACTOR_MEAN
           && AUTO_TABLE.height_factor_sigma == GoLZoneSpawnConfig::HEIGHT_FACTOR_SIGMA
           && AUTO_TABLE.height_factor_lo    == GoLZoneSpawnConfig::HEIGHT_FACTOR_CLAMP_LO
           && AUTO_TABLE.height_factor_hi    == GoLZoneSpawnConfig::HEIGHT_FACTOR_CLAMP_HI,
    "AUTO_TABLE's per-cell height factor is the zones' own draw, transcribed");
static_assert(AUTO_TABLE.mode_threshold == GoLZoneSpawnConfig::MODE_THRESHOLD,
    "and the eligibility threshold — the one term of the zone-lattice "
    "decision that was never about zones. It is a HARDWARE MIRROR rather "
    "than a transported field (world.wgsl AUTO_MODE_THRESHOLD); this "
    "assert still pins the bank, which is its one home");

// ── The colour target: a UNIFORM range written as centre + half-range ──
// zone_derive_params drew each channel as hash * LENS_TARGET_RANGE +
// LENS_TARGET_LO — uniform over [0.2, 0.8]. A bank speaks centre and
// spread, so the transcription is LO + RANGE/2 and RANGE/2. The
// distribution SHAPE changes (uniform to Gaussian) and that is disclosed
// rather than hidden: the range is identical and the centre is identical,
// and a colour target is not a shape anyone can see.
static_assert(AUTO_TABLE.target[0] == GoLZoneSpawnConfig::LENS_TARGET_LO + GoLZoneSpawnConfig::LENS_TARGET_RANGE * 0.5f
           && AUTO_TABLE.target[1] == GoLZoneSpawnConfig::LENS_TARGET_LO + GoLZoneSpawnConfig::LENS_TARGET_RANGE * 0.5f
           && AUTO_TABLE.target[2] == GoLZoneSpawnConfig::LENS_TARGET_LO + GoLZoneSpawnConfig::LENS_TARGET_RANGE * 0.5f
           && AUTO_TABLE.target_spread == GoLZoneSpawnConfig::LENS_TARGET_RANGE * 0.5f,
    "AUTO_TABLE's colour target is the zones' uniform [LO, LO+RANGE], "
    "written as centre and half-range");

} // namespace the_board
} // namespace t7
