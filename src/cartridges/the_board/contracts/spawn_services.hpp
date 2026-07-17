#pragma once
#include <cstdint>
#include <cstddef>                                            // size_t (the rescale template's array extent)
#include "cartridges/the_board/contracts/roster.hpp"          // PopFamily (sizes MIN_SEPARATION)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"    // MachineCtx + EntityInstance + traits/adapter + TierProfile

// ─── spawn_services.hpp (CONTRACT: the machine's decl tier) ───────
// History: audit/LADDER.md
// The machine natives (spawn_engine, entity_pipeline) each held a
// two-tier shape — a DECL tier consumed BEFORE entities (the preamble
// + generic + rescale templates, the service decls, the arch
// vocabulary, MIN_SEPARATION) and a BODY tier that needs entities
// COMPLETE. The decl tier graduates HERE so the merged machine
// headers ride the cohort tail whole. The bodies bind by the TU's own
// law: an inline function or template declared before its callers may
// be DEFINED later in the same TU (templates instantiate at
// end-of-TU) — named at the contract.
//
// OWNERS: every DEFINITION lives in machine/spawn_engine.hpp or
// machine/entity_pipeline.hpp (merged, cohort tail). This file owns
// only the boundary — entity_types.hpp's sibling.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// fwd — entities vocabulary named in a declaration only (graduated
// here with the decl that names it); the
// definitions, at the cohort tail, see the complete type.
struct ActiveColumn;
// fwd — state.hpp's GPU mesh-param records (return values below; a
// non-defining declaration tolerates the incomplete type).
struct GPUArchMeshParams;
struct GPUColumnMeshParams;

// ── Shared spawn helper vocabulary ─────────────────────────────────

struct SpawnGatePreambleResult {
    uint32_t seed;          // from evaluate_spawn_gate
    uint32_t slot;          // reserved slot index
    uint32_t theme_idx;     // themes_state_.temporal_flavor at evaluation time
    bool ok;                // false = early exit (idempotency, gate, no slot)
};

struct PositionResult {
    float cx, cz, rotation;
    int32_t host_gx, host_gz;
    bool ok;
};

// ── Spawn gate vocabulary ──────────────────────────────────────────

struct SpawnPreamble {
    uint32_t seed;          // tile_seed(world_state_.active_seed, gx, gz)
    bool passed;            // false if spawn gate failed
};

// ─── Global Entity Density ──────────────────────────────────────
// (Graduated with the decl tier: gol's and gallery's bespoke spawn
// funnels read it before the machine tail.)
inline constexpr float GLOBAL_ENTITY_DENSITY = 1.0f;

// ── Minimum Separation Matrix ─────────────────────────────────────
//
// WHAT: the extra edge-to-edge gap a candidate placement must keep from
//   every registered footprint, per ordered family pair.
// AXES: MIN_SEPARATION[placing][existing] — row = family being PLACED,
//   column = the EXISTING footprint's family. ASYMMETRIC: [Antenna][Arch]
//   = 60 (an antenna being placed keeps 60 wu from a standing arch) but
//   [Arch][Antenna] = 8 (an arch being placed tolerates 8 wu to a
//   standing antenna).
// UNITS: world-units, ADDITIVE — the consumer sums the two footprint
//   radii first, then adds this gap on top (and may shrink it by
//   PROXIMITY_GAP_REDUCTION × affinity for clustering families).
// ORDER: rows and columns both follow PopFamily order (PYRAMID=0 …
//   GALLERY=11), PINNED by the F-1 static_assert at roster.hpp —
//   renumbering a family is a compile error, not a silent re-column.
// CONSUMER: check_position(), machine/spawn_engine.hpp (sole reader).
// SENTINEL: 0.0 = no gap constraint for that pair (only the radii sum
//   applies; the consumer skips the gap term entirely).
// Placement determinant — frozen biography (§12): changing a number
// changes which candidate positions survive, i.e. changes worlds.
inline constexpr float MIN_SEPARATION[PopFamily::COUNT][PopFamily::COUNT] = {
    //                near:  Pyr    Arch   Col    Ant    Palm   Cact   Blad   Sph    Ribn   Cube   GoL    Gall
    /* placing Pyramid  */ { 65.0f, 60.0f,  5.0f, 55.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Arch     */ { 60.0f, 20.0f, 10.0f, 60.0f,  8.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Column   */ {  5.0f, 10.0f,  8.0f,  6.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Antenna  */ { 55.0f, 60.0f,  6.0f, 12.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Palm     */ {  5.0f,  8.0f,  5.0f,  5.0f,  8.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Cactus   */ {  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  8.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Blade    */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Sphere   */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 20.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Ribbon   */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 40.0f,  0.0f,  0.0f,  0.0f },
    /* placing Cube     */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 15.0f,  0.0f,  0.0f },
    /* placing GoL      */ { 10.0f, 10.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 60.0f,  0.0f },
    /* placing Gallery  */ { 10.0f, 10.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 10.0f, 30.0f },
};

// ── Arch vocabulary (graduated with the decl tier: read by entities'
//    recipes and direction/mood.hpp's portal/doorway geometry) ──

struct ArchIdx {
    static constexpr uint32_t SPAN         = 0;  // full span (halved in compute_solid_half)
    static constexpr uint32_t RISE         = 1;
    static constexpr uint32_t DEPTH        = 2;
    static constexpr uint32_t THICKNESS    = 3;
    static constexpr uint32_t PIER_HEIGHT  = 4;
    static constexpr uint32_t PIER_PADDING = 5;
    static constexpr uint32_t EDGE_BLEND   = 6;
    static constexpr uint32_t COUNT        = 7;
};

// params[] order MUST match ARCH_PARAM_DEFS:
//   [0]SPAN [1]RISE [2]DEPTH [3]THICKNESS [4]PIER_HEIGHT [5]PIER_PADDING [6]EDGE_BLEND
//
// Note: name is ArchTierRow, not ArchTier — the enum class ArchTier
// (DOORWAY/STANDARD/MONUMENTAL) already occupies that name in grounded.hpp.
struct ArchTierRow {
    TierProfile profile;
    float       color_override;
    float       burial;
    uint32_t    segs_u;
    uint32_t    segs_v;
};

// ── Arch tier table ────────────────────────────────────────────────
// WHAT: per-tier arch recipe — tier-selection weight + the Gaussian
//   {mean, sigma} of each geometry parameter + mesh tessellation.
// AXES: row = arch tier, in enum class ArchTier order
//   (0 DOORWAY / 1 STANDARD / 2 MONUMENTAL); inner params[] column =
//   ArchIdx order (the MUST-match note above).
// UNITS: weight = relative tier-selection weight (normalized over rows
//   by select_tier); {μ,σ} = world-units for SPAN/RISE/DEPTH/THICKNESS/
//   PIER_HEIGHT, dimensionless ratio for PIER_PADDING/EDGE_BLEND;
//   burial = fraction of pier_height sunk below ground; segs_u/v =
//   mesh tessellation counts.
// CONSUMERS: entity_pipeline.hpp (arch_tier_profile, burial, segs into
//   mesh params); grounded.hpp force-spawn portal (DOORWAY row);
//   mood.hpp portal/doorway geometry (DOORWAY row).
// SENTINELS: color_var 0 = fall back to ColorPartDef.variance;
//   color_override 0 = no override.
// Biography determinant — frozen biography (§12): weight feeds
// select_tier(gate.seed), {μ,σ} feed cpu_sample_gaussian(gate.seed);
// changing a number changes every arch ever born.
inline constexpr ArchTierRow ARCH_TIERS[] = {
    //   {  weight, color_var, { {μ,σ}: SPAN      RISE        DEPTH       THICKNESS     PIER_HEIGHT  PIER_PAD     EDGE_BLEND } },  col_ovr  burial  segs_u  segs_v
    /* DOORWAY    */ {
        { 0.50f, 0.0f, { {12.0f, 2.4f}, {12.0f, 2.4f}, {4.5f, 0.9f}, {1.2f, 0.18f}, {1.5f, 0.9f}, {0.9f, 0.3f}, {0.9f, 0.15f} }},
        0.15f, 0.20f, 16, 4
    },
    /* STANDARD   */ {
        { 0.15f, 0.0f, { {50.0f, 15.0f}, {42.0f, 7.0f}, {5.6f, 1.1f}, {1.4f, 0.21f}, {5.6f, 2.1f}, {0.7f, 0.3f}, {0.7f, 0.14f} }},
        0.25f, 0.20f, 32, 8
    },
    /* MONUMENTAL */ {
        { 0.15f, 0.0f, { {60.0f, 10.0f}, {80.0f, 12.0f}, {10.0f, 2.0f}, {2.5f, 0.30f}, {8.0f, 2.5f}, {1.0f, 0.3f}, {0.8f, 0.15f} }},
        0.35f, 0.20f, 48, 12
    },
};

// ═══ SPAWN SERVICES — DECLARATIONS (spawn_engine) ═════════════════
//
// DEFINED in machine/spawn_engine.hpp (merged, cohort tail): the
// engine reaches the machine face for the root organs (world/time/
// mood/themes/tile state, entities_state_, the GPU wire) and routes
// the twelve families through FAMILY_DISPATCH.

// ═══ THE COMPOSITION LAW — the collapse ═════════════════════
// ONE stack, authored once, called by all three spawn authors (the
// generic preamble, GoL, Gallery). Per-consumer FACTS travel as DATA:
// base-chance authority (scalar, or archetype-indexed — resolved by
// the caller before the call), clamp policy, proximity on/off, and
// the mood-zero veto style. The float multiplication ORDER inside the
// definition is the bit-identity contract:
// mood → GLOBAL_ENTITY_DENSITY → tile (entity_density →
// spatial_density) → [proximity] → base × adj → clamp. Seed domains
// and the rolls themselves stay with the consumers.

enum class SpawnClamp : uint32_t {
    MIN1,     // min(chance, 1.0)        — the generic preamble
    RANGE01,  // max(0, min(1, chance))  — GoL
    NONE,     // raw base × adj          — Gallery. Sub-ruling: the
              //   absent clamp is CARRIED AS DATA (behavior-identical);
              //   ruling a clamp IN is a separate taste gate.
};

struct SpawnChanceResult {
    float chance;   // the composed probability (post clamp policy)
    bool  vetoed;   // true only under veto_on_zero_mood with mood ≤ 0
};

SpawnChanceResult compose_spawn_chance(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t family, float base_chance, const float* mood_mult,
    bool use_proximity, bool veto_on_zero_mood, SpawnClamp clamp);

SpawnPreamble evaluate_spawn_gate(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float chance);
void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z);
float proximity_affinity_boost(MachineCtx* c, float cx, float cz, uint32_t family);
bool check_position(MachineCtx* c, float px, float pz, float placing_radius,
    uint32_t placing_family);
uint32_t register_footprint(MachineCtx* c, float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family = UINT32_MAX,
    uint32_t tier = 0);
void unregister_footprints_for_patch(MachineCtx* c, int32_t gx, int32_t gz);
PositionResult negotiate_position(MachineCtx* c,
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    float footprint_r, uint32_t family, uint32_t tier = 0);
void record_placement_bookkeeping(uint32_t family, uint32_t tier_idx);
GPUArchMeshParams build_arch_mesh_params(MachineCtx* c, uint32_t slot);
GPUColumnMeshParams build_column_mesh_params_from(const ActiveColumn& c);
GPUColumnMeshParams build_column_mesh_params(MachineCtx* c, uint32_t slot);
uint32_t update_entity_draw_visibility(MachineCtx* c, wgpu::Queue& queue);
const char* family_short_name(uint32_t family);
void dump_entity_census(MachineCtx* c, const char* trigger);
void select_entities_for_patch(MachineCtx* c, int32_t gx, int32_t gz);
void place_entity_queue(MachineCtx* c);
void commit_entity_queue(MachineCtx* c, wgpu::Queue& queue);

// The preamble template (SEAM[spawn_engine:P11]) — DECLARATION only;
// the definition rides machine/spawn_engine.hpp (end-of-TU
// instantiation binds every pre-tail caller).
template<typename C, typename ActiveT>
SpawnGatePreambleResult run_spawn_preamble(C* c,
    int32_t gx, int32_t gz,
    ActiveT* active_arr, uint32_t max_instances,
    uint32_t spawn_roll_prop, float spawn_chance,
    const float* mood_mult,
    uint32_t family, const char* diag_name);

// ═══ PIPELINE VERBS — DECLARATIONS (entity_pipeline) ══════════════
//
// DEFINED in machine/entity_pipeline.hpp (merged, cohort tail): the
// verbs reach the machine face for c->mood_state_ / c->world_state_
// and route through the spawn services; the family adapters write
// c->entities_state_ and the GPU wire.

bool generic_select(MachineCtx* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    int32_t gx, int32_t gz,
    EntityInstance& inst);
bool generic_place(MachineCtx* c,
    const EntityFamilyTraits& traits,
    EntityInstance& inst);
void generic_commit(MachineCtx* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    const EntityInstance& inst,
    wgpu::Queue& queue);

// The rescale template — DECLARATION only; the definition (and its
// hand-curated per-family param-index law) rides
// machine/entity_pipeline.hpp.
template<size_t N>
inline void rescale_to_rolled_target(EntityInstance& inst, float ceiling_h,
    float target_lo, float target_hi, float current_h,
    const uint32_t (&params_to_scale)[N]);

} // namespace the_board
} // namespace t7
