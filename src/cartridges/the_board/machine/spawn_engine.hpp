#pragma once
#include <cstdint>
#include <vector>         // the two queues
#include <iostream>       // DIAG block in the preamble template
#include <cmath>      // std::floor, std::sqrt, std::min/max companions   // (impl, merged)
#include <algorithm>  // std::min, std::max   // (impl, merged)
#include <iomanip>    // census column formatting   // (impl, merged)
#include "cartridges/the_board/contracts/keyhole.hpp"       // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── spawn_engine.hpp (S3 · MERGED: vocabulary + state + impl) ─────
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
// entities.hpp (ActiveColumn/EntitiesState — COMPLETE, the merged
// bodies deref them), patch_system.hpp (PATCH_EXTENT — the preamble
// template reads it at definition), renderer.hpp. MERGED at the
// cohort tail (DISSOLVE-1 Batch C, the B ruling): the decl tier
// lives in contracts/spawn_services.hpp; every pre-tail caller binds
// by same-TU late definition (templates at end-of-TU).

namespace t7 {
namespace the_board {

// ─── Entity Distance Culling — THE RING (re-ruled) ─────────────────
//
// THE RING is the draw authority (chain, state.hpp Dim; live value =
// config veil_ring): an entity is IN the draw set iff any part of it
// reaches inside the ring — center-distance MINUS its horizontal extent
// ≤ ring (the "center±extent" metric; replaces the retired per-size
// inset). Hysteresis sits OUTSIDE the ring, in the fully-iced zone:
//   show when (dist − extent) ≤ ring        (entering fragments are at
//                                            icing = 1 → invisible join)
//   hide when (dist − extent) > ring + HYST (fragments fully iced for
//                                            the whole band → invisible exit)
// Both toggle edges are behind the icing — materialize inside the fade.
inline constexpr float ENTITY_CULL_HYSTERESIS     = 40.0f;   // toggle band, wholly beyond the ring
inline constexpr float ENTITY_THIN_EXTENT         = 5.0f;    // columns/antennas: conservative horizontal half-reach

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

// ─── Property Index Registry ────────────────────────────────────

// ── Proximity affinity ─────────────────────────────────────────────
//
// WHAT: the clustering system — five tables, one mechanism. When a
//   family with a non-zero AFFINITY row is being placed, nearby
//   attractor footprints (a) MULTIPLY its spawn chance (boost) and
//   (b) SHRINK its required separation gap (gap reduction).
// AXES: the four vectors are indexed by the PLACING family; the matrix
//   is PROXIMITY_AFFINITY[placing][existing-neighbor] — e.g.
//   [Palm][Palm]=0.65 means a palm being placed is strongly attracted
//   to standing palms; [Palm][Cactus]=0.3 a milder pull.
// UNITS: RADIUS = wu (neighbor-scan distance around the candidate);
//   MAX_BOOST = multiplier ceiling on the spawn-chance boost;
//   THRESHOLD = count (minimum qualifying neighbors before any boost);
//   GAP_REDUCTION = fraction 0-1 of MIN_SEPARATION removed, scaled by
//   the pair's affinity; AFFINITY = dimensionless weight 0-1, summed
//   over neighbors into boost = min(1 + Σaff, MAX_BOOST).
// ORDER: every axis follows PopFamily order (PYRAMID=0 … GALLERY=11),
//   PINNED by the F-1 static_assert at roster.hpp.
// CONSUMERS: proximity_affinity_boost() below (RADIUS/MAX_BOOST/
//   THRESHOLD/AFFINITY → the adj_mod spawn multiplier);
//   check_position() (AFFINITY × GAP_REDUCTION → the effective gap);
//   proximity_row_active() (constexpr row precheck).
// SENTINELS: RADIUS 0 = family never scans (boost hard-disabled);
//   MAX_BOOST 1.0 = no boost possible; THRESHOLD 0 = no minimum;
//   GAP_REDUCTION 0 = gap never shrinks; an all-zero AFFINITY row
//   short-circuits the whole mechanism for that family.
// Spawn + placement determinant — frozen biography (§12): these numbers
// shape both the rate and the geometry of every cluster ever born.
// Only COLUMN and the flora trio (PALM/CACTUS/BLADE) cluster today.

//                              Pyr    Arch   Col    Ant    Palm   Cact   Blad   Sph    Ribn   Cube   GoL    Gall
inline constexpr float    PROXIMITY_RADIUS[PopFamily::COUNT] = { 0.0f,  0.0f, 60.0f,  0.0f,150.0f,120.0f,120.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f };   // wu; 0 = never scans
inline constexpr float    PROXIMITY_MAX_BOOST[PopFamily::COUNT] = { 1.0f,  1.0f,  2.0f,  1.0f,  3.0f,  3.0f,  3.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f };   // ×ceiling; 1 = no boost
inline constexpr uint32_t PROXIMITY_THRESHOLD[PopFamily::COUNT] = { 0,     0,     2,     0,     1,     1,     1,     0,     0,     0,     0,     0 };   // min neighbors; 0 = none
inline constexpr float    PROXIMITY_GAP_REDUCTION[PopFamily::COUNT] = { 0.0f, 0.0f, 0.3f, 0.0f, 0.6f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };   // fraction of MIN_SEPARATION; 0 = keep full gap

// AFFINITY[placing][existing]: rows follow PopFamily; only Col + the
// flora trio have non-zero rows (all others never cluster).
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

// ═══ MODULE FUNCTIONS ══════════════════════════════════════════════
//
// DECLARATIONS graduated to contracts/spawn_services.hpp (the
// machine's decl tier — DISSOLVE-1 Batch C) with the boundary DTOs
// (SpawnGatePreambleResult / PositionResult / SpawnPreamble), the
// ActiveColumn fwd, MIN_SEPARATION, and GLOBAL_ENTITY_DENSITY (gol +
// gallery read it pre-tail). Definitions are all below.

// ── Helper 1: SpawnGatePreamble ──────────────────────────────

// SEAM[spawn_engine:P11] the canonical templated active-array helper.
// THE TEMPLATE KEYHOLE, retired to a doorway (DISSOLVE-1 Batch C):
// every instantiation now deduces C = MachineCtx (the machine face);
// the DECLARATION lives in contracts/spawn_services.hpp so the ten
// pre-tail callers bind here at end-of-TU instantiation. The typename
// C stays — one implementation, ten callers, the active-array type
// still varies per family (ActiveT).
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
    r.theme_idx = c->themes_state_.temporal_flavor;
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


// ═══ MODULE IMPLEMENTATION (merged; was spawn_engine.inl) ═════════
//
// The engine's verbs: position negotiation, the footprint registry,
// mesh-param rebuilds + distance culling, the census, gate
// evaluation, proximity affinity, and the select → place → commit
// dispatch loops. Reaches the machine face for the root organs
// (c->world_state_ / c->time_state_ / c->mood_state_ /
// c->themes_state_ / c->tile_world_state_ / c->entities_state_ /
// c->player_) and the GPU wire (c->gpuState_); the loops route
// through FAMILY_DISPATCH with the machine face as the row argument.


// ── Helper 2: NegotiatePosition ─────────────────────────────

inline PositionResult negotiate_position(MachineCtx* c,
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    float footprint_r, uint32_t family, uint32_t tier)
{
    PositionResult r{};
    r.ok = false;

    // 1. Jittered position
    jittered_position(seed, trigger_gx, trigger_gz,
        pos_x_prop, pos_z_prop, jitter, r.cx, r.cz);
    r.rotation = cpu_hash_f(seed, rotation_seed_prop) * 6.283185f;

    //
    // In finite indoor worlds, push the candidate inward so the
    // entity's footprint stays at least INDOOR_ENTITY_WALL_MARGIN
    // from every wall. We clamp instead of rejecting because
    // rejection would silently drop entities anchored to corner
    // patches (their seed-determined position keeps landing in
    // the wall margin and never recovers). Clamping shifts the
    // candidate to the boundary of the legal box, then the
    // existing footprint-overlap check handles any pile-ups.
    //
    // If the room is too small for the entity plus margins on
    // both sides (lo > hi), we clamp to the room center —
    // shouldn't happen for typical indoor entities (max
    // footprint at radius=1 is 65; rescaled entities are well
    // under that).
    if (c->world_state_.finite_mode && MOOD_TABLE[c->mood_state_.active].indoor) {
        float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
        float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
        float clearance = INDOOR_ENTITY_WALL_MARGIN + footprint_r;
        float lo = bmin + clearance;
        float hi = bmax - clearance;
        if (lo > hi) {
            float center = (bmin + bmax) * 0.5f;
            r.cx = center;
            r.cz = center;
        }
        else {
            if (r.cx < lo) r.cx = lo;
            else if (r.cx > hi) r.cx = hi;
            if (r.cz < lo) r.cz = lo;
            else if (r.cz > hi) r.cz = hi;
        }
    }

    // 2. Separation + footprint check (single pass)
    if (!check_position(c, r.cx, r.cz, footprint_r, family))
        return r;

    // 3. Host patch + footprint registration (Q6a: one key derivation)
    auto hk = tile_key(r.cx, r.cz);
    r.host_gx = hk.x; r.host_gz = hk.z;
    if (register_footprint(c, r.cx, r.cz, footprint_r,
        r.host_gx, r.host_gz, family, tier) == UINT32_MAX) return r;

    r.ok = true;
    return r;
}

// ── Helper 3: record_placement_bookkeeping ──────────────────

inline void record_placement_bookkeeping(uint32_t /*family*/, uint32_t /*tier_idx*/)
{
}

// ═══ MESH GEN PREPARERS + CULLING ════════════════════════════════

// ─── Column / Arch / Pyramid mesh-gen preparers ───────────────

// Rebuild GPUArchMeshParams from cached ActiveArch data.
inline GPUArchMeshParams build_arch_mesh_params(MachineCtx* c, uint32_t slot) {
    const auto& a = c->entities_state_.arches[slot];
    GPUArchMeshParams p{};
    p.center_x = a.world_x;
    p.center_z = a.world_z;
    p.rotation = a.rotation;
    p.half_span = a.half_span;
    p.rise = a.rise;
    p.depth = a.depth;
    p.thickness = a.thickness;
    p.pier_height = a.pier_height;
    p.burial = a.burial;
    p.catenary_a = solve_catenary_a(a.half_span, a.rise);
    p.segs_u = a.segs_u;
    p.segs_v = a.segs_v;
    // Portal color override (mirrors spawn logic)
    if (a.is_portal) {
        const float* pc = a.is_back_portal
            ? PORTAL_COLOR_BACK
            : PORTAL_COLORS[a.destination.mood % MOOD_COUNT];
        p.color_r = pc[0]; p.color_g = pc[1]; p.color_b = pc[2];
    }
    else {
        p.color_r = a.col_r; p.color_g = a.col_g; p.color_b = a.col_b;
    }
    p.is_active = 1;
    return p;
}

// Rebuild GPUColumnMeshParams from cached ActiveColumn data.
inline GPUColumnMeshParams build_column_mesh_params_from(const ActiveColumn& c) {
    GPUColumnMeshParams p{};
    p.center_x = c.world_x;
    p.center_z = c.world_z;
    p.height = c.height;
    p.shaft_radius = c.shaft_radius;
    p.taper = c.taper;
    p.entasis = c.entasis;
    p.base_height = c.base_height;
    p.base_overhang = c.base_overhang;
    p.capital_height = c.cap_height;
    p.capital_overhang = c.cap_overhang;
    p.burial = c.burial;
    p.color_r = c.col_r;
    p.color_g = c.col_g;
    p.color_b = c.col_b;
    p.base_layers = c.base_layers;
    p.capital_layers = c.cap_layers;
    p.segs_around = c.segs_around;
    p.shaft_rings = c.shaft_rings;
    p.is_active = 1;
    p.tier = c.tier_idx;
    p.drum_color_r1 = c.drum_colors[0];
    p.drum_color_g1 = c.drum_colors[1];
    p.drum_color_b1 = c.drum_colors[2];
    p.drum_color_r2 = c.drum_colors[3];
    p.drum_color_g2 = c.drum_colors[4];
    p.drum_color_b2 = c.drum_colors[5];
    p.drum_color_r3 = c.drum_colors[6];
    p.drum_color_g3 = c.drum_colors[7];
    p.drum_color_b3 = c.drum_colors[8];
    return p;
}

inline GPUColumnMeshParams build_column_mesh_params(MachineCtx* c, uint32_t slot) {
    return build_column_mesh_params_from(c->entities_state_.columns[slot]);
}

// Scan all active entities, toggle draw_visible with hysteresis,
// and upload mesh param changes. Returns count of currently hidden entities.
// THE RING is the correctness gate (re-ruled): draw membership = any part
// of the entity inside the live ring (center − extent ≤ ring). Anchor: the
// point (readback — the same yardstick as the terrain band). Both toggle
// edges sit at/beyond the ring where the icing is already 1 — invisible.
inline uint32_t update_entity_draw_visibility(MachineCtx* c, wgpu::Queue& queue) {
    uint32_t culled = 0;

    const float ring = c->gpuState_.veil_ring();   // live chain value — the draw authority

    // Arches
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (!c->entities_state_.arches[i].active) continue;
        const auto& a = c->entities_state_.arches[i];
        float dx = a.world_x - c->player_.readback_x;
        float dz = a.world_z - c->player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);

        float nearest = dist - a.half_span;            // the arch's closest reach (center − extent)
        bool should_show = a.draw_visible
            ? (nearest <= ring + ENTITY_CULL_HYSTERESIS)  // visible: hide once fully iced past the band
            : (nearest <= ring);                          // hidden:  show as fragments enter the icing

        if (should_show != a.draw_visible) {
            c->entities_state_.arches[i].draw_visible = should_show;
            if (should_show) {
                c->gpuState_.upload_arch_mesh_params_slot(queue, i, build_arch_mesh_params(c, i));
            }
            else {
                GPUArchMeshParams empty{};
                c->gpuState_.upload_arch_mesh_params_slot(queue, i, empty);
            }
            c->entities_state_.arch_mesh_gen_pending = true;
        }

        if (!c->entities_state_.arches[i].draw_visible) culled++;
    }

    // Columns
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        if (!c->entities_state_.columns[i].active) continue;
        const auto& col = c->entities_state_.columns[i];
        float dx = col.world_x - c->player_.readback_x;
        float dz = col.world_z - c->player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);

        float nearest = dist - std::max(col.shaft_radius, ENTITY_THIN_EXTENT);
        bool should_show = col.draw_visible
            ? (nearest <= ring + ENTITY_CULL_HYSTERESIS)
            : (nearest <= ring);

        if (should_show != col.draw_visible) {
            c->entities_state_.columns[i].draw_visible = should_show;
            if (should_show) {
                c->gpuState_.upload_column_mesh_params_slot(queue, i, build_column_mesh_params(c, i));
            }
            else {
                GPUColumnMeshParams empty{};
                c->gpuState_.upload_column_mesh_params_slot(queue, i, empty);
            }
            c->entities_state_.column_mesh_gen_pending = true;
        }

        if (!c->entities_state_.columns[i].draw_visible) culled++;
    }

    // Antennas
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        if (!c->entities_state_.antennas[i].active) continue;
        const auto& ant = c->entities_state_.antennas[i];
        float dx = ant.world_x - c->player_.readback_x;
        float dz = ant.world_z - c->player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);
        uint32_t gpu_slot = i + Dim::ANTENNA_SLOT_OFFSET;

        float nearest = dist - ENTITY_THIN_EXTENT;   // antennas are thin masts
        bool should_show = ant.draw_visible
            ? (nearest <= ring + ENTITY_CULL_HYSTERESIS)
            : (nearest <= ring);

        if (should_show != ant.draw_visible) {
            c->entities_state_.antennas[i].draw_visible = should_show;
            if (should_show) {
                c->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, build_column_mesh_params_from(ant));
            }
            else {
                GPUColumnMeshParams empty{};
                c->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, empty);
            }
            c->entities_state_.column_mesh_gen_pending = true;
        }

        if (!c->entities_state_.antennas[i].draw_visible) culled++;
    }

    return culled;
}

// ═══ FOOTPRINT REGISTRY ══════════════════════════════════════════

inline bool check_position(MachineCtx* c, float px, float pz, float placing_radius,
    uint32_t placing_family) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        float dx = px - c->spawn_engine_state_.footprints_[i].x;
        float dz = pz - c->spawn_engine_state_.footprints_[i].z;
        float effective_min = placing_radius + c->spawn_engine_state_.footprints_[i].radius;
        if (c->spawn_engine_state_.footprints_[i].family < PopFamily::COUNT) {
            float min_gap = MIN_SEPARATION[placing_family][c->spawn_engine_state_.footprints_[i].family];
            if (min_gap > 0.0f) {
                float aff = PROXIMITY_AFFINITY[placing_family][c->spawn_engine_state_.footprints_[i].family];
                if (aff > 0.0f) min_gap *= (1.0f - aff * PROXIMITY_GAP_REDUCTION[placing_family]);
                effective_min += min_gap;
            }
        }
        if (dx * dx + dz * dz < effective_min * effective_min) return false;
    }
    return true;
}

inline uint32_t register_footprint(MachineCtx* c, float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family,
    uint32_t tier) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) {
            c->spawn_engine_state_.footprints_[i] = { x, z, radius, gx, gz, family, tier, c->time_state_.seconds, true };
            return i;
        }
    }
    return UINT32_MAX;  // full — entity should not spawn
}

inline void unregister_footprints_for_patch(MachineCtx* c, int32_t gx, int32_t gz) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (c->spawn_engine_state_.footprints_[i].active &&
            c->spawn_engine_state_.footprints_[i].patch_gx == gx && c->spawn_engine_state_.footprints_[i].patch_gz == gz) {
            c->spawn_engine_state_.footprints_[i].active = false;
        }
    }
}

// ═══ ENTITY CENSUS ═══════════════════════════════════════════════

inline const char* family_short_name(uint32_t family) {
    static const char* NAMES[] = { "pyr", "arch", "col", "ant", "palm", "cact", "blad", "sph", "ribn", "cube", "gol", "gall" };
    return (family < PopFamily::COUNT) ? NAMES[family] : "???";
}

inline void dump_entity_census(MachineCtx* c, const char* trigger) {
    uint32_t count = 0;
    uint32_t by_family[PopFamily::COUNT] = {};
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        if (c->spawn_engine_state_.footprints_[i].family >= PopFamily::COUNT) continue;
        count++;
        by_family[c->spawn_engine_state_.footprints_[i].family]++;
    }

    std::cout << "[CENSUS t=" << std::fixed << std::setprecision(1) << c->time_state_.seconds
        << " trigger=" << trigger << "] " << count << " entities (";
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        if (f > 0) std::cout << " ";
        std::cout << family_short_name(f) << ":" << by_family[f];
    }
    std::cout << ")\n";

    // Per-entity detail, sorted by family then spawn_time
    struct CensusEntry { uint32_t fp_idx; uint32_t family; uint32_t tier; float spawn_time; };
    CensusEntry entries[MAX_FOOTPRINTS];
    uint32_t n = 0;
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active || c->spawn_engine_state_.footprints_[i].family >= PopFamily::COUNT) continue;
        entries[n++] = { i, c->spawn_engine_state_.footprints_[i].family, c->spawn_engine_state_.footprints_[i].tier, c->spawn_engine_state_.footprints_[i].spawn_time };
    }
    // Insertion sort by (family, spawn_time)
    for (uint32_t i = 1; i < n; i++) {
        CensusEntry key = entries[i]; uint32_t j = i;
        while (j > 0 && (entries[j - 1].family > key.family ||
            (entries[j - 1].family == key.family && entries[j - 1].spawn_time > key.spawn_time))) {
            entries[j] = entries[j - 1]; j--;
        }
        entries[j] = key;
    }
    for (uint32_t i = 0; i < n; i++) {
        const auto& fp = c->spawn_engine_state_.footprints_[entries[i].fp_idx];
        std::cout << "  " << family_short_name(fp.family)
            << " t" << fp.tier
            << " (" << std::setw(8) << std::setprecision(1) << fp.x
            << "," << std::setw(8) << fp.z << ")"
            << " p(" << std::setw(3) << fp.patch_gx << "," << std::setw(3) << fp.patch_gz << ")"
            << " age=" << std::setprecision(1) << (c->time_state_.seconds - fp.spawn_time)
            << "\n";
    }
    std::cout << std::flush;
}

// ═══ SPAWN UTILITIES ═════════════════════════════════════════════
//
// The spawn lifecycle's smallest building blocks: gate evaluation,
// jittered position, the family enum, the global density dial,
// and two load-bearing tag tables (Spawn Configuration Summary,
// Property Index Registry) that document the contracts every family
// participates in.

// ─── Spawn gate ──────────────────────────────────────────────────

// Evaluate the spawn gate: seed + flat probability check.
// adjacency_mod is a multiplier from the full spawn cascade.
inline SpawnPreamble evaluate_spawn_gate(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float spawn_chance,
    float adjacency_mod) {
    SpawnPreamble result{};
    // (per-gate archetype lookup CUT — composition recon R5: computed for
    //  every generic gate, read by nobody; the sole archetype consumer
    //  (gallery) calls tile_archetype itself in its bespoke funnel.)
    result.seed = tile_seed(c->world_state_.active_seed, gx, gz);
    float chance = std::min(spawn_chance * adjacency_mod, 1.0f);
    result.passed = cpu_hash_f(result.seed, spawn_roll_prop) < chance;
    return result;
}

// Jittered world position within a patch.
inline void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z) {
    out_x = (gx + 0.5f) * PATCH_EXTENT + (cpu_hash_f(seed, prop_x) - 0.5f) * PATCH_EXTENT * jitter;
    out_z = (gz + 0.5f) * PATCH_EXTENT + (cpu_hash_f(seed, prop_z) - 0.5f) * PATCH_EXTENT * jitter;
}

inline float proximity_affinity_boost(MachineCtx* c, float cx, float cz, uint32_t family) {
    if (!proximity_row_active(family)) return 1.0f;
    float radius = PROXIMITY_RADIUS[family];
    if (radius <= 0.0f) return 1.0f;
    float r2 = radius * radius;
    float weighted = 0.0f;
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        if (c->spawn_engine_state_.footprints_[i].family >= PopFamily::COUNT) continue;
        float aff = PROXIMITY_AFFINITY[family][c->spawn_engine_state_.footprints_[i].family];
        if (aff <= 0.0f) continue;
        float dx = cx - c->spawn_engine_state_.footprints_[i].x;
        float dz = cz - c->spawn_engine_state_.footprints_[i].z;
        if (dx * dx + dz * dz < r2) {
            weighted += aff;
            count++;
        }
    }
    if (count < PROXIMITY_THRESHOLD[family]) return 1.0f;
    return std::min(1.0f + weighted, PROXIMITY_MAX_BOOST[family]);
}

// ─── Select / Place / Commit dispatch loops ─────────────────────

inline void select_entities_for_patch(MachineCtx* c, int32_t gx, int32_t gz) {
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        if (!ROSTER.family_enabled(f)) continue;  // ROSTER-GATE family (b) — disabled family never selected -> never placed/committed/meshed/drawn. Budgeted stream path, not the per-frame hot path.
        EntityQueueEntry e{};
        e.family = f;
        e.gx = gx; e.gz = gz;
        if (FAMILY_DISPATCH[f].try_select(c, gx, gz, e))
            c->spawn_engine_state_.entityQueue_.push_back(e);
    }
}

// ─── Place: spatial negotiation (no GPU writes) ──────────────

inline void place_entity_queue(MachineCtx* c) {
    for (auto& e : c->spawn_engine_state_.entityQueue_) {
        PlacementEntry pe{};
        if (FAMILY_DISPATCH[e.family].try_place(c, e, pe))
            c->spawn_engine_state_.placementResults_.push_back(pe);
    }
    c->spawn_engine_state_.entityQueue_.clear();
}

// ─── Commit: GPU writes from placement results ──────────────

inline void commit_entity_queue(MachineCtx* c, wgpu::Queue& queue) {
    for (auto& pe : c->spawn_engine_state_.placementResults_)
        FAMILY_DISPATCH[pe.family].try_commit(c, pe, queue);
    c->spawn_engine_state_.placementResults_.clear();
}

} // namespace the_board
} // namespace t7
