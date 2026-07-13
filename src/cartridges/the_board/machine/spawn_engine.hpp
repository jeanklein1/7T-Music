#pragma once
#include <cstdint>
#include <vector>         // the two queues
#include <iostream>       // DIAG block in the preamble template
#include "cartridges/the_board/contracts/keyhole.hpp"       // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── spawn_engine.hpp (S3 · HEADER: vocabulary + state + decls) ────
// Converted at LADDER-6 (the first of the last two class-body
// citizens); payload relocations (LADDER-3) and conversion history
// in audit/LADDER.md.
//
// How and when things appear: shared spawn helpers, footprint
// registry, proximity affinity, mesh-param rebuilds, distance
// culling, census, plus the dispatch loops that drive both generic
// and bespoke families through select → place → commit.
//
// SEAM[spawn_engine:P11] home of pattern P11 (templated active-array
//   helper) — run_spawn_preamble<C, ActiveT> is the canonical
//   instance. One implementation, ten callers.
// SEAM[spawn_engine:structural] RETIRED. The EntityQueueEntry /
//   PlacementEntry unions and every type they embed live together in
//   entity_types.hpp (the contract home); this module holds only the
//   queues and loops. spawn_engine stays ONE pair, never split.
// SEAM[spawn_engine:L1] latent diagnostic — DIAG_ENTITY_LIFECYCLE is
//   compile-time guarded (flag defined at build time, off by
//   default). Same family as the [DIAG:*] stdout pattern noted
//   across the codebase. Document alongside any other diagnostic
//   switches when the exhibition-guard discussion happens.
//
// Depends on cohort include order: roster.hpp (PopFamily),
// entity_types.hpp (queue unions), state.hpp (GPU mesh params),
// entities.hpp (ActiveColumn), patch_system.hpp (PATCH_EXTENT — the
// preamble template reads it at definition).

namespace t7 {
namespace the_board {

// ── Shared spawn helper vocabulary ─────────────────────────────────

struct SpawnGatePreambleResult {
    uint32_t seed;          // from evaluate_spawn_gate
    uint32_t slot;          // reserved slot index
    uint32_t theme_idx;     // themes_state_.active_theme_idx_ at evaluation time
    bool ok;                // false = early exit (idempotency, gate, no slot)
};

struct PositionResult {
    float cx, cz, rotation;
    int32_t host_gx, host_gz;
    bool ok;
};

// ─── Entity Distance Culling ─────────────────────────────────────
//
// Size-awareness is re-signed: a taller entity culls slightly EARLIER (its
// base stays safely inside the edge), never later — the old outward lead is
// gone. The inset is small and capped. Hysteresis prevents oscillation.
inline constexpr float ENTITY_CULL_EDGE_MARGIN    = 0.5f * Dim::PATCH_EXTENT;  // 25 wu inside the visible edge
inline constexpr float ENTITY_CULL_SIZE_INSET     = 0.5f;    // wu of inward inset per unit of entity size
inline constexpr float ENTITY_CULL_SIZE_INSET_MAX = 60.0f;   // cap: never cull nearer than base − this
inline constexpr float ENTITY_CULL_HYSTERESIS     = 40.0f;   // band: hide at the (inset) edge, show 40 wu inside

// ── Footprint registry vocabulary ──────────────────────────────────

struct GroundFootprint {
    float x = 0.0f, z = 0.0f;
    float radius = 0.0f;
    int32_t patch_gx = 0, patch_gz = 0;
    uint32_t family = UINT32_MAX;  // PopFamily index
    uint32_t tier = 0;             // tier index within family
    float spawn_time = 0.0f;       // time_state_.seconds at registration
    bool active = false;
};

inline constexpr uint32_t MAX_FOOTPRINTS = 128;
inline constexpr float CENSUS_DUMP_INTERVAL = 30.0f;

// ── Spawn gate vocabulary ──────────────────────────────────────────

struct SpawnPreamble {
    uint32_t seed;          // tile_seed(world_state_.active_seed, gx, gz)
    uint32_t archetype;     // 0=mountainous, 1=varied, 2=basin, 3=pool
    bool passed;            // false if spawn gate failed
};

// ─── Spawn Configuration Summary ────────────────────────────────
//
// * Ribbon CHANCE 0.900 is a TESTING bump for ribbon-dev visibility;
//   ship value 0.400 (ribbon.inl SPAWN_CHANCE, reverted at ship).

// ─── Global Entity Density ──────────────────────────────────────
inline constexpr float GLOBAL_ENTITY_DENSITY = 1.0f;

// ─── Property Index Registry ────────────────────────────────────

// ── Minimum Separation Matrix ─────────────────────────────────────
//
// Read as: row = entity being placed, column = existing entity.
// The check is asymmetric: placing an arch near a pyramid may have a
// different minimum than placing a pyramid near an arch.
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

// ── Proximity affinity ─────────────────────────────────────────────

//                              Pyr    Arch   Col    Ant    Palm   Cact   Blad   Sph    Ribn   Cube   GoL    Gall
inline constexpr float    PROXIMITY_RADIUS[PopFamily::COUNT] = { 0.0f,  0.0f, 60.0f,  0.0f,150.0f,120.0f,120.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f };
inline constexpr float    PROXIMITY_MAX_BOOST[PopFamily::COUNT] = { 1.0f,  1.0f,  2.0f,  1.0f,  3.0f,  3.0f,  3.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f };
inline constexpr uint32_t PROXIMITY_THRESHOLD[PopFamily::COUNT] = { 0,     0,     2,     0,     1,     1,     1,     0,     0,     0,     0,     0 };
inline constexpr float    PROXIMITY_GAP_REDUCTION[PopFamily::COUNT] = { 0.0f, 0.0f, 0.3f, 0.0f, 0.6f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

inline constexpr float PROXIMITY_AFFINITY[PopFamily::COUNT][PopFamily::COUNT] = {
    //           near: Pyr   Arch  Col   Ant   Palm  Cact  Blad  Sph   Ribn  Cube  GoL   Gall
    /* Pyr   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Arch  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Col   */ { 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Ant   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Palm  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.65f, 0.3f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Cact  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.5f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Blad  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Sph   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Ribn  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Cube  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* GoL   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Gall  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
};

// Precomputed: does this family have any non-zero affinity?
inline constexpr bool proximity_row_active(uint32_t family) {
    for (uint32_t f = 0; f < PopFamily::COUNT; f++)
        if (PROXIMITY_AFFINITY[family][f] > 0.0f) return true;
    return false;
}

// ═══ MODULE STATE ══════════════════════════════════════════════════

// ═══ BESPOKE-FAMILY SELECTION/PLACEMENT PAYLOADS ═════════════════
//
// Three bespoke families (GoL, Gallery, Ribbon) don't fit the
// generic pipeline's EntityInstance shape — their selection
// records carry family-specific fields (lattice node, painting
// count, wave parameters). The payload DTOs AND the tagged unions
// that carry them (EntityQueueEntry / PlacementEntry) live together
// in entity_types.hpp — the contract home; a DTO that exists to
// cross a boundary belongs to the boundary's contract. See
// SEAM[spawn_engine:structural] in the file header.

// ─── The queues (machine state) ──────────────────────────────────
//
// EntityQueueEntry / PlacementEntry are contract vocabulary
// (entity_types.hpp); the QUEUES they fill are spine state and live
// here. entityQueue_ decouples WHAT exists from WHERE it goes;
// placementResults_ holds entities past spatial negotiation, ready
// for GPU commit.

// Instance (spawn_engine_state_) lives at the composition root.
struct SpawnEngineState {
    std::vector<EntityQueueEntry> entityQueue_;
    std::vector<PlacementEntry> placementResults_;
    GroundFootprint footprints_[MAX_FOOTPRINTS]{};
    float lastCensusDump_ = -999.0f;
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═══════════════════════════════
//
// DEFINED in spawn_engine.inl (post-class): the engine reaches the
// keyhole for the root organs (world/time/mood/themes/tile state,
// entities_state_, the GPU wire) and routes the twelve families
// through FAMILY_DISPATCH.

SpawnPreamble evaluate_spawn_gate(Cartridge* c, int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float spawn_chance,
    float adjacency_mod = 1.0f);
void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z);
float proximity_affinity_boost(Cartridge* c, float cx, float cz, uint32_t family);
bool check_position(Cartridge* c, float px, float pz, float placing_radius,
    uint32_t placing_family);
uint32_t register_footprint(Cartridge* c, float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family = UINT32_MAX,
    uint32_t tier = 0);
void unregister_footprints_for_patch(Cartridge* c, int32_t gx, int32_t gz);
PositionResult negotiate_position(Cartridge* c,
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    float footprint_r, uint32_t family, uint32_t tier = 0);
void record_placement_bookkeeping(uint32_t family, uint32_t tier_idx);
GPUArchMeshParams build_arch_mesh_params(Cartridge* c, uint32_t slot);
GPUColumnMeshParams build_column_mesh_params_from(const ActiveColumn& c);
GPUColumnMeshParams build_column_mesh_params(Cartridge* c, uint32_t slot);
uint32_t update_entity_draw_visibility(Cartridge* c, wgpu::Queue& queue);
const char* family_short_name(uint32_t family);
void dump_entity_census(Cartridge* c, const char* trigger);
void select_entities_for_patch(Cartridge* c, int32_t gx, int32_t gz);
void place_entity_queue(Cartridge* c);
void commit_entity_queue(Cartridge* c, wgpu::Queue& queue);

// ── Helper 1: SpawnGatePreamble ──────────────────────────────

// SEAM[spawn_engine:P11] the canonical templated active-array helper.
// THE TEMPLATE KEYHOLE: the cartridge parameter is deduced (typename
// C) so this definition stays legal above the incomplete class —
// every instantiation deduces C = Cartridge, and the c-> reaches are
// checked at instantiation, in complete-class context.
template<typename C, typename ActiveT>
SpawnGatePreambleResult run_spawn_preamble(C* c,
    int32_t gx, int32_t gz,
    ActiveT* active_arr, uint32_t max_instances,
    uint32_t spawn_roll_prop, float spawn_chance,
    const float* mood_mult,
    uint32_t family, const char* diag_name)
{
    SpawnGatePreambleResult r{};
    r.ok = false;

    // 1. Idempotency
    for (uint32_t i = 0; i < max_instances; i++) {
        if (active_arr[i].active &&
            active_arr[i].patch_gx == gx &&
            active_arr[i].patch_gz == gz) {
            return r;
        }
    }

    // 2-6. Spawn modifier chain
    float adj_mod = mood_mult[c->mood_state_.active];
    adj_mod *= GLOBAL_ENTITY_DENSITY;
    r.theme_idx = c->themes_state_.active_theme_idx_;
    tile_apply_spawn_mult(c->tile_world_state_, gx, gz, family, adj_mod);  // F3 (m3b): the S2 boundary face

    // 6b. Proximity affinity boost (nearby entities attract)
    {
        float pcx = (gx + 0.5f) * PATCH_EXTENT;
        float pcz = (gz + 0.5f) * PATCH_EXTENT;
        adj_mod *= proximity_affinity_boost(c, pcx, pcz, family);
    }

    // 7. Spawn gate
    auto ctx = evaluate_spawn_gate(c, gx, gz, spawn_roll_prop,
        spawn_chance, adj_mod);
    if (!ctx.passed) return r;

    // 8-9. Find and reserve slot
    uint32_t slot = UINT32_MAX;
    for (uint32_t i = 0; i < max_instances; i++) {
        if (!active_arr[i].active) { slot = i; break; }
    }
    if (slot == UINT32_MAX) return r;
    active_arr[slot].active = true;

#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:SEL] " << diag_name << " slot=" << slot
        << " patch=(" << gx << "," << gz << ")\n";
#endif

    r.seed = ctx.seed;
    r.slot = slot;
    r.ok = true;
    return r;
}

} // namespace the_board
} // namespace t7
