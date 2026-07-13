// ─── entities.inl (IMPL: post-class definitions) ─────────────────
// Impl of entities.hpp (LADDER-2 c1; LADDER-4 channel): history in audit/LADDER.md.
//
// Definitions for entities.hpp's declared preparers + the arch
// force-spawn author + the seven grounded-family evictors + the
// clean-three recipes (blade / palm / cactus: tier tables, traits,
// adapters, dispatch funnels). The preparers dereference
// c->gpuState_'s index-count setters; the force-spawn author
// additionally reaches write_pier (patch_system.hpp), c->gpuState_'s
// mesh-params upload, ARCH_TIERS / ArchIdx (entity_pipeline.hpp),
// solve_catenary_a (seed_utils.hpp), and PATCH_EXTENT
// (patch_system.hpp); the recipes reach the machine's generic
// three-phase verbs and services via the keyhole and
// THEMES (INTENT[services:themes]).
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>   // std::max (portal-arch burial floor)
#include <cmath>       // std::floor, std::cos, std::sin (portal-arch placement)
#include <cstdint>

namespace t7 {
namespace the_board {

// ═══ MESH-GEN PREPARERS ═══════════════════════════════════════════

inline bool prepare_palm_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.palm_mesh_gen_pending) return false;
    es.palm_mesh_gen_pending = false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
        if (es.palms[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_palm_index_count(anyActive
        ? (maxSlot + 1) * Dim::PALMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_cactus_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.cactus_mesh_gen_pending) return false;
    es.cactus_mesh_gen_pending = false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
        if (es.cacti[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_cactus_index_count(anyActive
        ? (maxSlot + 1) * Dim::CACTUSG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_blade_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.blade_mesh_gen_pending) return false;
    es.blade_mesh_gen_pending = false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
        if (es.blades[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_blade_index_count(anyActive
        ? (maxSlot + 1) * Dim::BLADEG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_column_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.column_mesh_gen_pending) return false;
    es.column_mesh_gen_pending = false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        if (es.columns[i].active) { maxSlot = i; anyActive = true; }
    }
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        if (es.antennas[i].active) {
            maxSlot = i + Dim::ANTENNA_SLOT_OFFSET;
            anyActive = true;
        }
    }
    c->gpuState_.set_column_index_count(anyActive
        ? (maxSlot + 1) * Dim::CMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_arch_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.arch_mesh_gen_pending) return false;
    es.arch_mesh_gen_pending = false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (es.arches[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_arch_index_count(anyActive
        ? (maxSlot + 1) * Dim::AMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_pyramid_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.pyramid_mesh_gen_pending) return false;
    es.pyramid_mesh_gen_pending = false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        if (es.pyramids[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_pyramid_index_count(anyActive
        ? (maxSlot + 1) * Dim::PMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

// ═══ THE ARCH FORCE-SPAWN AUTHOR (the portal channel) ═══════════

inline uint32_t force_spawn_portal_arch(EntitiesState& es, Cartridge* c, wgpu::Queue& queue,
    float cx, float cz, float rotation,
    const PortalDestination& dest, bool is_back_portal,
    const float portal_color[3]) {
    // ROSTER-GATE portal (b) — THE SECOND DOOR. Portals force-spawn arches
    // directly (bypassing FAMILY_DISPATCH — ROSTER_RECON R3), so this is the
    // single choke point every portal spawner routes through (back, finite,
    // future). Disabled: spawn nothing (no arch, no piers, no mesh-pending),
    // return the no-free-slot sentinel so callers treat it as "none placed".
    // HOME (LADDER-4 / K4): MIGRATED here from mood's force_spawn_portal_at —
    // the door's written retirement condition ("when mood converts and
    // force-spawn becomes a request channel, this door MIGRATES INTO that
    // channel"), fulfilled. The arch's owner holds the door to the arch's
    // storage; mood computes values upstream.
    if constexpr (!ROSTER.portal) { (void)queue; (void)cx; (void)cz; (void)rotation; (void)dest; (void)is_back_portal; (void)portal_color; return UINT32_MAX; }

    uint32_t slot = UINT32_MAX;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (!es.arches[i].active) { slot = i; break; }
    }
    if (slot == UINT32_MAX) return UINT32_MAX;

    const auto& tp = ARCH_TIERS[static_cast<uint32_t>(ArchTier::DOORWAY)];
    float half_span = tp.profile.params[ArchIdx::SPAN].mean * 0.5f;
    float rise = tp.profile.params[ArchIdx::RISE].mean;
    float depth = tp.profile.params[ArchIdx::DEPTH].mean;
    float thickness = tp.profile.params[ArchIdx::THICKNESS].mean;
    float pier_height = tp.profile.params[ArchIdx::PIER_HEIGHT].mean;
    float pier_padding = tp.profile.params[ArchIdx::PIER_PADDING].mean;
    float edge_blend = tp.profile.params[ArchIdx::EDGE_BLEND].mean;

    float pier_half_x = thickness * 0.5f + pier_padding + edge_blend;
    float pier_half_z = depth * 0.5f + pier_padding + edge_blend;

    int32_t gx = static_cast<int32_t>(std::floor(cx / PATCH_EXTENT));
    int32_t gz = static_cast<int32_t>(std::floor(cz / PATCH_EXTENT));

    float cos_r = std::cos(rotation);
    float sin_r = std::sin(rotation);
    uint32_t pier_l_slot = Dim::PIER_ARCH_BASE + slot * 2;
    uint32_t pier_r_slot = pier_l_slot + 1;

    float pl_x = cx + (-half_span) * cos_r;
    float pl_z = cz + (-half_span) * sin_r;
    float pr_x = cx + half_span * cos_r;
    float pr_z = cz + half_span * sin_r;

    GPUPierInstance pl{};
    pl.origin[0] = pl_x;  pl.origin[1] = pl_z;
    pl.half_size[0] = pier_half_x;  pl.half_size[1] = pier_half_z;
    pl.height_near = pier_height;  pl.height_far = pier_height;
    pl.rotation = rotation;  pl.edge_blend = edge_blend;
    pl.tier = PierTier::ARCH_DOORWAY;
    pl.is_active = 1;
    write_pier(&c->machine_ctx_, queue, pier_l_slot, pl);

    GPUPierInstance pr{};
    pr.origin[0] = pr_x;  pr.origin[1] = pr_z;
    pr.half_size[0] = pier_half_x;  pr.half_size[1] = pier_half_z;
    pr.height_near = pier_height;  pr.height_far = pier_height;
    pr.rotation = rotation;  pr.edge_blend = edge_blend;
    pr.tier = PierTier::ARCH_DOORWAY;
    pr.is_active = 1;
    write_pier(&c->machine_ctx_, queue, pier_r_slot, pr);

    auto& aa = es.arches[slot];
    aa.patch_gx = gx;
    aa.patch_gz = gz;
    aa.active = true;
    aa.draw_visible = true;
    aa.world_x = cx;
    aa.world_z = cz;
    aa.rotation = rotation;
    aa.half_span = half_span;
    aa.total_height = pier_height + rise;
    aa.tier = ArchTier::DOORWAY;
    aa.depth = depth;
    aa.thickness = thickness;
    aa.rise = rise;
    aa.pier_height = pier_height;
    aa.burial = std::max(0.2f, pier_height * tp.burial);
    aa.segs_u = tp.segs_u;
    aa.segs_v = tp.segs_v;
    aa.col_r = 0.75f;  aa.col_g = 0.68f;  aa.col_b = 0.60f;

    {
        // GPU compute_entity_placement handles ground_y from heightfield
        aa.cached_ground_y = 0.0f;
    }

    aa.is_portal = true;
    aa.is_back_portal = is_back_portal;
    aa.position_hash = cpu_hash(static_cast<uint32_t>(cx * 73856093.0f), static_cast<uint32_t>(cz * 19349663.0f));
    aa.destination = dest;

    es.arch_count++;

    GPUArchMeshParams meshParams{};
    meshParams.center_x = cx;
    meshParams.center_z = cz;
    meshParams.rotation = rotation;
    meshParams.half_span = half_span;
    meshParams.rise = rise;
    meshParams.depth = depth;
    meshParams.thickness = thickness;
    meshParams.pier_height = pier_height;
    meshParams.burial = aa.burial;
    meshParams.catenary_a = solve_catenary_a(half_span, rise);
    meshParams.segs_u = tp.segs_u;
    meshParams.segs_v = tp.segs_v;
    meshParams.color_r = portal_color[0];
    meshParams.color_g = portal_color[1];
    meshParams.color_b = portal_color[2];
    meshParams.is_active = 1;
    c->gpuState_.upload_arch_mesh_params_slot(queue, slot, meshParams);
    es.arch_mesh_gen_pending = true;

    return slot;
}

// ═══ THE EVICTORS ═════════════════════════════════════════════════

inline void evict_pyramid(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    self->entities_state_.cpu_pyramids.instances[slot] = GPUPyramidInstance{};
    self->entities_state_.pyramids[slot].active = false;
    self->entities_state_.pyramid_count--;
    self->world_state_.ground_entries_dirty = true;
    { GPUPyramidMeshParams ep{}; self->gpuState_.upload_pyramid_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.pyramid_mesh_gen_pending = true;

    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        if (self->entities_state_.pyramids[i].active) max_idx = i + 1;
    }
    self->entities_state_.cpu_pyramids.count = max_idx;
    self->gpuState_.upload_pyramids(queue, self->entities_state_.cpu_pyramids);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   pyr slot=" << slot << "\n";
#endif
}

inline void evict_arch(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    clear_pier(self, queue, Dim::PIER_ARCH_BASE + slot * 2);
    clear_pier(self, queue, Dim::PIER_ARCH_BASE + slot * 2 + 1);
    self->entities_state_.arches[slot].active = false;
    self->entities_state_.arch_count--;
    self->mood_state_.portals_dirty = true;
    { GPUArchMeshParams ep{}; self->gpuState_.upload_arch_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.arch_mesh_gen_pending = true;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   arch slot=" << slot << "\n";
#endif
}

inline void evict_column(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    clear_pier(self, queue, Dim::PIER_COLUMN_BASE + slot);
    self->entities_state_.columns[slot].active = false;
    self->entities_state_.column_count--;
    { GPUColumnMeshParams ep{}; self->gpuState_.upload_column_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.column_mesh_gen_pending = true;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   col slot=" << slot << "\n";
#endif
}

inline void evict_antenna(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    uint32_t gpu_slot = slot + Dim::ANTENNA_SLOT_OFFSET;
    clear_pier(self, queue, Dim::PIER_COLUMN_BASE + gpu_slot);
    self->entities_state_.antennas[slot].active = false;
    self->entities_state_.antenna_count--;
    { GPUColumnMeshParams ep{}; self->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, ep); }
    self->entities_state_.column_mesh_gen_pending = true;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   ant slot=" << slot << "\n";
#endif
}

inline void evict_palm(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    self->entities_state_.palms[slot].active = false;
    self->entities_state_.palm_count--;
    { GPUPalmMeshParams ep{}; self->gpuState_.upload_palm_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.palm_mesh_gen_pending = true;
    self->world_state_.ground_entries_dirty = true;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   palm slot=" << slot << "\n";
#endif
}

inline void evict_cactus(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    self->entities_state_.cacti[slot].active = false;
    self->entities_state_.cactus_count--;
    { GPUCactusMeshParams ep{}; self->gpuState_.upload_cactus_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.cactus_mesh_gen_pending = true;
    self->world_state_.ground_entries_dirty = true;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   cact slot=" << slot << "\n";
#endif
}

inline void evict_blade(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    self->entities_state_.blades[slot].active = false;
    self->entities_state_.blade_count--;
    { GPUBladeClusterMeshParams ep{}; self->gpuState_.upload_blade_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.blade_mesh_gen_pending = true;
    self->world_state_.ground_entries_dirty = true;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   blad slot=" << slot << "\n";
#endif
}

// ═══ THE CLEAN THREE — BLADE / PALM / CACTUS RECIPES ══════════════
//
// Per-family tier tables, traits, adapters, and dispatch funnels.
// Each funnels into the machine's generic three-phase verbs via the
// keyhole; THEMES is reached as THEMES
// (INTENT[services:themes] at its definition); the table rows point
// here (family_dispatch.inl).

// ═══ FAMILY: BLADE ════════════════════════════════════════════════

// ─── Blade Parameter Index Registry ──────────────────────────────

struct BladeIdx {
    static constexpr uint32_t BLADE_COUNT = 0;
    static constexpr uint32_t BLADE_H     = 1;
    static constexpr uint32_t BLADE_H_VAR = 2;
    static constexpr uint32_t BLADE_W     = 3;
    static constexpr uint32_t SPLAY       = 4;
    static constexpr uint32_t CURVE       = 5;
    static constexpr uint32_t TWIST       = 6;
    static constexpr uint32_t TAPER       = 7;
    static constexpr uint32_t COUNT       = 8;
};

// ─── Parameter Definitions ───────────────────────────────────────
// prop must exactly match BladeProp::{name}, floor must match the
// std::max() in select_blade_for_patch.
//
//                                             prop                    floor   round  dist
inline constexpr TierParamDef BLADE_PARAM_DEFS[] = {
    { BladeProp::BLADE_COUNT,                  2.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { BladeProp::HEIGHT,                       0.5f, 1e30f,  false, ParamDist::GAUSSIAN },
    { BladeProp::HEIGHT_VAR,                   0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { BladeProp::WIDTH,                        0.05f, 1e30f, false, ParamDist::GAUSSIAN },
    { BladeProp::SPLAY,                        0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { BladeProp::CURVE,                        0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { BladeProp::TWIST,                        0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { BladeProp::TAPER,                        0.3f, 1e30f,  false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t BLADE_PARAM_COUNT = sizeof(BLADE_PARAM_DEFS) / sizeof(TierParamDef);

// params[] order MUST match BLADE_PARAM_DEFS:
//   [0]BLADE_COUNT [1]BLADE_H [2]BLADE_H_VAR [3]BLADE_W
//   [4]SPLAY [5]CURVE [6]TWIST [7]TAPER
struct BladeTierRow {
    TierProfile profile;
    float       color_over;
    uint32_t    blade_segs;
};

inline constexpr BladeTierRow BLADE_TIERS[] = {
    /* SPROUT  */ {
        { 0.50f, 0.06f, { {3.0f, 0.5f}, {1.8f, 0.4f}, {0.35f, 0.08f}, {0.30f, 0.06f},
                   {0.18f, 0.06f}, {0.12f, 0.04f}, {0.05f, 0.02f}, {0.85f, 0.05f} }},
        0.15f, 5
    },
    /* CLUMP   */ {
        { 0.35f, 0.06f, { {5.0f, 1.0f}, {3.2f, 0.6f}, {0.40f, 0.10f}, {0.45f, 0.08f},
                   {0.25f, 0.08f}, {0.18f, 0.06f}, {0.08f, 0.03f}, {0.82f, 0.05f} }},
        0.20f, 6
    },
    /* THICKET */ {
        { 0.15f, 0.08f, { {6.0f, 1.0f}, {5.5f, 1.2f}, {0.45f, 0.10f}, {0.55f, 0.10f},
                   {0.30f, 0.10f}, {0.22f, 0.08f}, {0.10f, 0.04f}, {0.80f, 0.06f} }},
        0.25f, 7
    },
};

inline const TierProfile& blade_get_tier_profile(uint32_t tier_idx) {
    return BLADE_TIERS[tier_idx].profile;
}

// ─── Color Parts ─────────────────────────────────────────────────

inline constexpr ColorPartDef BLADE_COLOR_PARTS[] = {
    { { 0.28f, 0.52f, 0.22f },   // BLADE_BODY_BASE
      0.0f,                        // variance set per-tier in adapter
      BladeProp::COLOR_VAR_R, 0 },
    { { 0.48f, 0.45f, 0.28f },   // BLADE_AGED_BASE
      0.0f,
      BladeProp::COLOR_VAR_R, 10 },
};

// ─── Traits Declaration ──────────────────────────────────────────

inline constexpr EntityFamilyTraits BLADE_TRAITS = {
    PopFamily::BLADE, "blad",
    Dim::MAX_BLADE_INSTANCES,
    true, false, 0,                                    // grounded, no piers
    true, 100.0f, 1.0f,                               // footprint, cull
    BladeProp::SPAWN_ROLL, BladeClusterConfig::SPAWN_CHANCE,
    BladeClusterConfig::MOOD_MULTIPLIER,
    BladeClusterConfig::POSITION_JITTER,
    BLADE_TIER_COUNT, BladeProp::TIER,
    BLADE_PARAM_DEFS, BLADE_PARAM_COUNT,
    BladeProp::POSITION_X, BladeProp::POSITION_Z, BladeProp::ROTATION, true,
    2, BLADE_COLOR_PARTS,
};

// ─── Blade Adapter Functions ─────────────────────────────────────

inline SpawnGateOutput blade_run_gate(MachineCtx* c,
    int32_t gx, int32_t gz) {
    auto gate = run_spawn_preamble(c, gx, gz,
        c->entities_state_.blades, Dim::MAX_BLADE_INSTANCES,
        BladeProp::SPAWN_ROLL, BladeClusterConfig::SPAWN_CHANCE,
        BladeClusterConfig::MOOD_MULTIPLIER,
        PopFamily::BLADE, "blad");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}

inline const float* blade_get_theme_tier_weights(uint32_t theme_idx) {
    return THEMES[theme_idx].tier_wt_blade;
}

inline void blade_compute_solid_half(EntityInstance& inst,
    const TierProfile& /*tier*/) {
    inst.solid_half = inst.params[BladeIdx::BLADE_W] * 0.5f;
    inst.burial = 0.0f;
}

inline void blade_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ab = c->entities_state_.blades[inst.slot];
    ab.patch_gx = inst.trigger_gx;
    ab.patch_gz = inst.trigger_gz;
    ab.host_gx  = inst.host_gx;
    ab.host_gz  = inst.host_gz;
    ab.active   = true;
    ab.draw_visible = true;
    ab.world_x  = inst.cx;
    ab.world_z  = inst.cz;
    ab.height   = inst.params[BladeIdx::BLADE_H];
    ab.radius   = inst.params[BladeIdx::BLADE_W] + inst.params[BladeIdx::SPLAY];
    ab.tier_idx = inst.tier_idx;
    ab.cached_ground_y = inst.cached_ground_y;
    c->entities_state_.blade_count++;
}

inline void blade_write_gpu(MachineCtx* c,
    const EntityInstance& inst, wgpu::Queue& queue) {
    GPUBladeClusterMeshParams mp{};
    mp.center_x    = inst.cx;
    mp.center_z    = inst.cz;
    mp.blade_count = inst.params[BladeIdx::BLADE_COUNT];
    mp.blade_h     = inst.params[BladeIdx::BLADE_H];
    mp.blade_h_var = inst.params[BladeIdx::BLADE_H_VAR];
    mp.blade_w     = inst.params[BladeIdx::BLADE_W];
    mp.splay       = inst.params[BladeIdx::SPLAY];
    mp.curve       = inst.params[BladeIdx::CURVE];
    mp.twist       = inst.params[BladeIdx::TWIST];
    mp.taper       = inst.params[BladeIdx::TAPER];
    mp.blade_r     = inst.colors[0];
    mp.blade_g     = inst.colors[1];
    mp.blade_b     = inst.colors[2];
    mp.aged_r      = inst.colors[3];
    mp.aged_g      = inst.colors[4];
    mp.aged_b      = inst.colors[5];
    mp.blade_segs  = BLADE_TIERS[inst.tier_idx].blade_segs;
    mp.is_active   = 1;
    mp.seed        = inst.seed;
    c->gpuState_.upload_blade_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.blade_mesh_gen_pending = true;
}

inline constexpr EntityFamilyAdapter BLADE_ADAPTER = {
    blade_run_gate,
    blade_get_theme_tier_weights,
    nullptr,                  // apply_indoor_rescale → not eligible (outdoor < ceiling)
    blade_compute_solid_half,
    nullptr,                  // compute_colors → use generic (Q24)
    blade_write_active,
    blade_write_gpu,
    nullptr,                  // no post_commit (no piers, no portals)
    blade_get_tier_profile,
};

// ── Blade dispatch wrappers ──

inline bool dispatch_select_blade_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, BLADE_TRAITS, BLADE_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::BLADE; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_blade_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, BLADE_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.blades[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_blade_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { generic_commit(self, BLADE_TRAITS, BLADE_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::BLADE, pe.generic.slot); }
    else { self->entities_state_.blades[pe.generic.slot].active = false; }
}

// ═══ FAMILY: PALM ═════════════════════════════════════════════════

struct PalmIdx {
    static constexpr uint32_t HEIGHT       = 0;
    static constexpr uint32_t BASE_R       = 1;
    static constexpr uint32_t TOP_R        = 2;
    static constexpr uint32_t LEAN         = 3;
    static constexpr uint32_t LEAN_DIR     = 4;
    static constexpr uint32_t BARK_RINGS   = 5;
    static constexpr uint32_t BARK_DEPTH   = 6;
    static constexpr uint32_t FROND_COUNT  = 7;
    static constexpr uint32_t FROND_LEN    = 8;
    static constexpr uint32_t FROND_WIDTH  = 9;
    static constexpr uint32_t FROND_DROOP  = 10;
    static constexpr uint32_t FROND_ARCH   = 11;
    static constexpr uint32_t CROWN_SPREAD = 12;
    static constexpr uint32_t CROWN_SKIRT  = 13;
    static constexpr uint32_t SOLID_PAD    = 14;
    static constexpr uint32_t EDGE_BLEND   = 15;
    static constexpr uint32_t COUNT        = 16;
};

inline constexpr TierParamDef PALM_PARAM_DEFS[] = {
    { PalmProp::HEIGHT,       2.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::BASE_R,       0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::TOP_R,        0.05f, 1e30f, false, ParamDist::GAUSSIAN },
    { PalmProp::LEAN,         0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::LEAN_DIR,     0.0f, 1e30f,  false, ParamDist::UNIFORM_TAU },
    { PalmProp::BARK_RINGS,   3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { PalmProp::BARK_DEPTH,   0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_COUNT,  3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { PalmProp::FROND_LEN,    1.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_WIDTH,  0.3f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_DROOP,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_ARCH,   0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::CROWN_SPREAD, 0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::CROWN_SKIRT,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::SOLID_PADDING,0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::EDGE_BLEND,   0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t PALM_PARAM_COUNT = sizeof(PALM_PARAM_DEFS) / sizeof(TierParamDef);

// params[] order MUST match PALM_PARAM_DEFS:
//   [0]HEIGHT [1]BASE_R [2]TOP_R [3]LEAN [4]LEAN_DIR(uniform — {0,0})
//   [5]BARK_RINGS [6]BARK_DEPTH [7]FROND_COUNT [8]FROND_LEN
//   [9]FROND_WIDTH [10]FROND_DROOP [11]FROND_ARCH [12]CROWN_SPREAD
//   [13]CROWN_SKIRT [14]SOLID_PAD [15]EDGE_BLEND
//
// LEAN_DIR is UNIFORM_TAU — uniform [0, 2π] has no meaningful mean/sigma,
// the slot is a literal {0, 0} placeholder.
struct PalmTierRow {
    TierProfile profile;
    float       color_over;
    float       burial;
    float       trunk_var;
    float       frond_var;
    uint32_t    trunk_segs;
    uint32_t    frond_segs;
};

inline constexpr PalmTierRow PALM_TIERS[] = {
    /* SAPLING */ {
        { 0.50f, 0.0f, { {8.0f, 2.0f},  {0.25f, 0.05f}, {0.12f, 0.02f}, {0.15f, 0.05f}, {0.0f, 0.0f},
                   {8.0f, 2.0f},  {0.03f, 0.01f}, {7.0f, 1.0f},   {3.0f, 0.5f},   {0.8f, 0.15f},
                   {0.3f, 0.1f},  {0.4f, 0.1f},   {0.6f, 0.1f},   {0.3f, 0.1f},   {0.5f, 0.1f},
                   {0.5f, 0.1f} }},
        0.1f, 0.1f, 0.06f, 0.04f, 12, 4
    },
    /* COASTAL */ {
        { 0.35f, 0.0f, { {16.0f, 3.0f}, {0.40f, 0.08f}, {0.15f, 0.03f}, {0.20f, 0.08f}, {0.0f, 0.0f},
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {11.0f, 2.0f},  {5.0f, 0.8f},   {1.2f, 0.2f},
                   {0.5f, 0.15f}, {0.5f, 0.12f},  {0.7f, 0.1f},   {0.4f, 0.1f},   {0.8f, 0.15f},
                   {0.8f, 0.15f} }},
        0.15f, 0.15f, 0.05f, 0.03f, 16, 5
    },
    /* ROYAL   */ {
        { 0.15f, 0.0f, { {28.0f, 5.0f}, {0.55f, 0.10f}, {0.18f, 0.04f}, {0.10f, 0.05f}, {0.0f, 0.0f},
                   {18.0f, 4.0f}, {0.05f, 0.01f}, {15.0f, 2.0f},  {7.0f, 1.0f},   {1.5f, 0.3f},
                   {0.6f, 0.15f}, {0.6f, 0.15f},  {0.8f, 0.1f},   {0.5f, 0.1f},   {1.0f, 0.2f},
                   {1.0f, 0.2f} }},
        0.20f, 0.2f, 0.04f, 0.03f, 20, 6
    },
};

inline const TierProfile& palm_get_tier_profile(uint32_t tier_idx) {
    return PALM_TIERS[tier_idx].profile;
}

inline constexpr ColorPartDef PALM_COLOR_PARTS[] = {
    { {0.45f,0.35f,0.25f}, 0.0f, PalmProp::COLOR_VAR_R, 0 },   // trunk
    { {0.25f,0.45f,0.20f}, 0.0f, PalmProp::COLOR_VAR_R, 10 },  // frond
    { {0.35f,0.38f,0.18f}, 0.0f, PalmProp::COLOR_VAR_R, 20 },  // aged
};

inline constexpr EntityFamilyTraits PALM_TRAITS = {
    PopFamily::PALM, "palm", Dim::MAX_PALM_INSTANCES,
    true, false, 0,
    true, 100.0f, 1.0f,
    PalmProp::SPAWN_ROLL, PalmConfig::SPAWN_CHANCE,
    PalmConfig::MOOD_MULTIPLIER, PalmConfig::POSITION_JITTER,
    PALM_TIER_COUNT, PalmProp::TIER,
    PALM_PARAM_DEFS, PALM_PARAM_COUNT,
    PalmProp::POSITION_X, PalmProp::POSITION_Z, PalmProp::ROTATION, true,
    3, PALM_COLOR_PARTS,
};

// ── Palm adapter functions ──

inline SpawnGateOutput palm_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    auto gate = run_spawn_preamble(c, gx, gz,
        c->entities_state_.palms, Dim::MAX_PALM_INSTANCES,
        PalmProp::SPAWN_ROLL, PalmConfig::SPAWN_CHANCE,
        PalmConfig::MOOD_MULTIPLIER, PopFamily::PALM, "palm");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}

inline const float* palm_get_theme_tier_weights(uint32_t theme_idx) {
    return THEMES[theme_idx].tier_wt_palm;
}

inline constexpr uint32_t PALM_INDOOR_RESCALE_PARAMS[] = {
    PalmIdx::HEIGHT, PalmIdx::BASE_R, PalmIdx::TOP_R, PalmIdx::BARK_DEPTH,
    PalmIdx::FROND_LEN, PalmIdx::FROND_WIDTH, PalmIdx::CROWN_SPREAD,
    PalmIdx::CROWN_SKIRT, PalmIdx::SOLID_PAD, PalmIdx::EDGE_BLEND,
    // LEAN/LEAN_DIR (angles), BARK_RINGS/FROND_COUNT (counts),
    // FROND_DROOP/FROND_ARCH (angles) intentionally not scaled.
};

// Palms read as canopy-defining architectural anchors and look wrong
// when too small — tighter target range than other indoor families.
inline void palm_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    rescale_to_rolled_target(inst, ceiling_h,
        /*target_lo*/ 0.80f, /*target_hi*/ 0.95f,
        /*current_h*/ inst.params[PalmIdx::HEIGHT],
        PALM_INDOOR_RESCALE_PARAMS);
}

inline void palm_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    float base_r = inst.params[PalmIdx::BASE_R];
    float pad    = inst.params[PalmIdx::SOLID_PAD];
    float blend  = inst.params[PalmIdx::EDGE_BLEND];
    inst.solid_half = base_r + pad + blend;
    inst.burial = PALM_TIERS[inst.tier_idx].burial;
}

inline void palm_compute_colors(EntityInstance& inst, const EntityFamilyTraits& traits, const TierProfile& /*tier*/) {
    const auto& tp = PALM_TIERS[inst.tier_idx];
    // trunk: trunk_var
    inst.colors[0] = traits.color_parts[0].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R) - 0.5f) * tp.trunk_var;
    inst.colors[1] = traits.color_parts[0].base[1] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_G) - 0.5f) * tp.trunk_var;
    inst.colors[2] = traits.color_parts[0].base[2] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_B) - 0.5f) * tp.trunk_var;
    // frond: frond_var
    inst.colors[3] = traits.color_parts[1].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R + 10u) - 0.5f) * tp.frond_var;
    inst.colors[4] = traits.color_parts[1].base[1] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_G + 10u) - 0.5f) * tp.frond_var;
    inst.colors[5] = traits.color_parts[1].base[2] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_B + 10u) - 0.5f) * tp.frond_var;
    // aged: frond_var (same variance as frond)
    inst.colors[6] = traits.color_parts[2].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R + 20u) - 0.5f) * tp.frond_var;
    inst.colors[7] = traits.color_parts[2].base[1] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_G + 20u) - 0.5f) * tp.frond_var;
    inst.colors[8] = traits.color_parts[2].base[2] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_B + 20u) - 0.5f) * tp.frond_var;
}

inline void palm_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ap = c->entities_state_.palms[inst.slot];
    ap.patch_gx = inst.trigger_gx; ap.patch_gz = inst.trigger_gz;
    ap.host_gx = inst.host_gx; ap.host_gz = inst.host_gz;
    ap.active = true; ap.draw_visible = true;
    ap.world_x = inst.cx; ap.world_z = inst.cz;
    ap.height = inst.params[PalmIdx::HEIGHT];
    ap.base_r = inst.params[PalmIdx::BASE_R];
    ap.tier_idx = inst.tier_idx;
    ap.cached_ground_y = inst.cached_ground_y;
    c->entities_state_.palm_count++;
}

inline void palm_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    const auto& tp = PALM_TIERS[inst.tier_idx];
    GPUPalmMeshParams mp{};
    mp.center_x     = inst.cx;
    mp.center_z     = inst.cz;
    mp.height       = inst.params[PalmIdx::HEIGHT];
    mp.base_r       = inst.params[PalmIdx::BASE_R];
    mp.top_r        = inst.params[PalmIdx::TOP_R];
    mp.lean         = inst.params[PalmIdx::LEAN];
    mp.lean_dir     = inst.params[PalmIdx::LEAN_DIR];
    mp.bark_rings   = inst.params[PalmIdx::BARK_RINGS];
    mp.bark_depth   = inst.params[PalmIdx::BARK_DEPTH];
    mp.frond_count  = inst.params[PalmIdx::FROND_COUNT];
    mp.frond_len    = inst.params[PalmIdx::FROND_LEN];
    mp.frond_width  = inst.params[PalmIdx::FROND_WIDTH];
    mp.frond_droop  = inst.params[PalmIdx::FROND_DROOP];
    mp.frond_arch   = inst.params[PalmIdx::FROND_ARCH];
    mp.crown_spread = inst.params[PalmIdx::CROWN_SPREAD];
    mp.crown_skirt  = inst.params[PalmIdx::CROWN_SKIRT];
    mp.burial       = inst.burial;
    mp.trunk_r = inst.colors[0]; mp.trunk_g = inst.colors[1]; mp.trunk_b = inst.colors[2];
    mp.frond_r = inst.colors[3]; mp.frond_g = inst.colors[4]; mp.frond_b = inst.colors[5];
    mp.aged_r  = inst.colors[6]; mp.aged_g  = inst.colors[7]; mp.aged_b  = inst.colors[8];
    mp.trunk_segs = tp.trunk_segs;
    mp.frond_segs = tp.frond_segs;
    mp.is_active = 1;
    c->gpuState_.upload_palm_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.palm_mesh_gen_pending = true;
}

inline constexpr EntityFamilyAdapter PALM_ADAPTER = {
    palm_run_gate, palm_get_theme_tier_weights,
    palm_apply_indoor_rescale,
    palm_compute_solid_half, palm_compute_colors,
    palm_write_active, palm_write_gpu, nullptr,
    palm_get_tier_profile,
};

// ── Palm dispatch wrappers ──

inline bool dispatch_select_palm_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, PALM_TRAITS, PALM_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::PALM; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_palm_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, PALM_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.palms[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_palm_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { generic_commit(self, PALM_TRAITS, PALM_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::PALM, pe.generic.slot); }
    else { self->entities_state_.palms[pe.generic.slot].active = false; }
}

// ═══ FAMILY: CACTUS ═══════════════════════════════════════════════

struct CactusIdx {
    static constexpr uint32_t HEIGHT     = 0;
    static constexpr uint32_t RADIUS     = 1;
    static constexpr uint32_t TAPER      = 2;
    static constexpr uint32_t RIBS       = 3;
    static constexpr uint32_t RIB_DEPTH  = 4;
    static constexpr uint32_t LEAN       = 5;
    static constexpr uint32_t LEAN_DIR   = 6;
    static constexpr uint32_t CAP_ROUND  = 7;
    static constexpr uint32_t ARM_COUNT  = 8;
    static constexpr uint32_t ARM_HEIGHT = 9;
    static constexpr uint32_t ARM_LENGTH = 10;
    static constexpr uint32_t ARM_RADIUS = 11;
    static constexpr uint32_t ARM_CURVE  = 12;
    static constexpr uint32_t COUNT      = 13;
};

inline constexpr TierParamDef CACTUS_PARAM_DEFS[] = {
    { CactusProp::HEIGHT,     1.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::RADIUS,     0.05f, 1e30f, false, ParamDist::GAUSSIAN },
    { CactusProp::TAPER,      0.5f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::RIBS,       4.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { CactusProp::RIB_DEPTH,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::LEAN,       0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::LEAN_DIR,   0.0f, 1e30f,  false, ParamDist::UNIFORM_TAU },
    { CactusProp::CAP_ROUND,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::ARM_COUNT,  0.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { CactusProp::ARM_HEIGHT, 0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::ARM_LENGTH, 0.5f, 1e30f,  false, ParamDist::GAUSSIAN },
    { CactusProp::ARM_RADIUS, 0.03f, 1e30f, false, ParamDist::GAUSSIAN },
    { CactusProp::ARM_CURVE,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t CACTUS_PARAM_COUNT = sizeof(CACTUS_PARAM_DEFS) / sizeof(TierParamDef);

// params[] order MUST match CACTUS_PARAM_DEFS:
//   [0]HEIGHT [1]RADIUS [2]TAPER [3]RIBS [4]RIB_DEPTH [5]LEAN
//   [6]LEAN_DIR(uniform — {0,0}) [7]CAP_ROUND [8]ARM_COUNT
//   [9]ARM_HEIGHT [10]ARM_LENGTH [11]ARM_RADIUS [12]ARM_CURVE
struct CactusTierRow {
    TierProfile profile;
    float       color_over;
    uint32_t    trunk_segs;
    uint32_t    arm_segs;
};

inline constexpr CactusTierRow CACTUS_TIERS[] = {
    /* FINGER     */ {
        { 0.50f, 0.04f, { {3.0f, 1.0f},   {0.15f, 0.03f}, {0.85f, 0.05f}, {8.0f, 1.0f},   {0.04f, 0.01f},
                   {0.1f, 0.05f},  {0.0f, 0.0f},   {0.6f, 0.1f},   {0.0f, 0.0f},
                   {0.5f, 0.1f},   {1.0f, 0.3f},   {0.08f, 0.02f}, {0.7f, 0.1f} }},
        0.1f, 12, 6
    },
    /* SAGUARO    */ {
        { 0.35f, 0.03f, { {8.0f, 2.0f},   {0.35f, 0.06f}, {0.9f, 0.04f},  {12.0f, 2.0f},  {0.05f, 0.01f},
                   {0.08f, 0.04f}, {0.0f, 0.0f},   {0.5f, 0.1f},   {1.5f, 0.7f},
                   {0.45f, 0.1f},  {3.0f, 0.8f},   {0.2f, 0.04f},  {0.6f, 0.15f} }},
        0.15f, 16, 8
    },
    /* CANDELABRA */ {
        { 0.15f, 0.03f, { {14.0f, 3.0f},  {0.45f, 0.08f}, {0.92f, 0.03f}, {16.0f, 2.0f},  {0.06f, 0.01f},
                   {0.05f, 0.03f}, {0.0f, 0.0f},   {0.4f, 0.1f},   {3.0f, 0.8f},
                   {0.4f, 0.1f},   {5.0f, 1.0f},   {0.25f, 0.05f}, {0.5f, 0.15f} }},
        0.2f, 20, 10
    },
};

inline const TierProfile& cactus_get_tier_profile(uint32_t tier_idx) {
    return CACTUS_TIERS[tier_idx].profile;
}

inline constexpr ColorPartDef CACTUS_COLOR_PARTS[] = {
    { {0.30f,0.45f,0.25f}, 0.0f, CactusProp::COLOR_VAR_R, 0 },   // body
    { {0.35f,0.55f,0.30f}, 0.0f, CactusProp::COLOR_VAR_R, 10 },  // rib
};

inline constexpr EntityFamilyTraits CACTUS_TRAITS = {
    PopFamily::CACTUS, "cact", Dim::MAX_CACTUS_INSTANCES,
    true, false, 0,
    true, 100.0f, 1.0f,
    CactusProp::SPAWN_ROLL, CactusConfig::SPAWN_CHANCE,
    CactusConfig::MOOD_MULTIPLIER, CactusConfig::POSITION_JITTER,
    CACTUS_TIER_COUNT, CactusProp::TIER,
    CACTUS_PARAM_DEFS, CACTUS_PARAM_COUNT,
    CactusProp::POSITION_X, CactusProp::POSITION_Z, CactusProp::ROTATION, true,
    2, CACTUS_COLOR_PARTS,
};

// ── Cactus adapter functions ──

inline SpawnGateOutput cactus_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    auto gate = run_spawn_preamble(c, gx, gz,
        c->entities_state_.cacti, Dim::MAX_CACTUS_INSTANCES,
        CactusProp::SPAWN_ROLL, CactusConfig::SPAWN_CHANCE,
        CactusConfig::MOOD_MULTIPLIER, PopFamily::CACTUS, "cact");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}

inline const float* cactus_get_theme_tier_weights(uint32_t theme_idx) {
    return THEMES[theme_idx].tier_wt_cactus;
}

inline void cactus_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    inst.solid_half = inst.params[CactusIdx::RADIUS] + 0.5f;
    inst.burial = 0.0f;
}

inline void cactus_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ac = c->entities_state_.cacti[inst.slot];
    ac.patch_gx = inst.trigger_gx; ac.patch_gz = inst.trigger_gz;
    ac.host_gx = inst.host_gx; ac.host_gz = inst.host_gz;
    ac.active = true; ac.draw_visible = true;
    ac.world_x = inst.cx; ac.world_z = inst.cz;
    ac.height = inst.params[CactusIdx::HEIGHT];
    ac.radius = inst.params[CactusIdx::RADIUS];
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
    c->entities_state_.cactus_count++;
}

inline void cactus_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    const auto& tp = CACTUS_TIERS[inst.tier_idx];
    GPUCactusMeshParams mp{};
    mp.center_x   = inst.cx;
    mp.center_z   = inst.cz;
    mp.height     = inst.params[CactusIdx::HEIGHT];
    mp.radius     = inst.params[CactusIdx::RADIUS];
    mp.taper      = inst.params[CactusIdx::TAPER];
    mp.ribs       = inst.params[CactusIdx::RIBS];
    mp.rib_depth  = inst.params[CactusIdx::RIB_DEPTH];
    mp.lean       = inst.params[CactusIdx::LEAN];
    mp.lean_dir   = inst.params[CactusIdx::LEAN_DIR];
    mp.cap_round  = inst.params[CactusIdx::CAP_ROUND];
    mp.arm_count  = inst.params[CactusIdx::ARM_COUNT];
    mp.arm_height = inst.params[CactusIdx::ARM_HEIGHT];
    mp.arm_length = inst.params[CactusIdx::ARM_LENGTH];
    mp.arm_radius = inst.params[CactusIdx::ARM_RADIUS];
    mp.arm_curve  = inst.params[CactusIdx::ARM_CURVE];
    mp.body_r = inst.colors[0]; mp.body_g = inst.colors[1]; mp.body_b = inst.colors[2];
    mp.rib_r  = inst.colors[3]; mp.rib_g  = inst.colors[4]; mp.rib_b  = inst.colors[5];
    mp.trunk_segs = tp.trunk_segs;
    mp.arm_segs   = tp.arm_segs;
    mp.is_active  = 1;
    mp.seed       = inst.seed;
    c->gpuState_.upload_cactus_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.cactus_mesh_gen_pending = true;
}

inline constexpr EntityFamilyAdapter CACTUS_ADAPTER = {
    cactus_run_gate, cactus_get_theme_tier_weights,
    nullptr,                              // apply_indoor_rescale → not eligible
    cactus_compute_solid_half, nullptr,   // compute_colors → use generic (Q24)
    cactus_write_active, cactus_write_gpu, nullptr,
    cactus_get_tier_profile,
};

// ── Cactus dispatch wrappers ──

inline bool dispatch_select_cactus_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, CACTUS_TRAITS, CACTUS_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::CACTUS; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_cactus_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, CACTUS_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.cacti[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_cactus_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { generic_commit(self, CACTUS_TRAITS, CACTUS_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::CACTUS, pe.generic.slot); }
    else { self->entities_state_.cacti[pe.generic.slot].active = false; }
}


// ─── Teardown (owner verb; REBUILD-0 m2, stamp D4) ────────────────
// The grounded families' half of the world-teardown sweep — CPU slot
// clears + GPU param-slot clears + mesh-gen re-arm, seven families in
// their one organ. UNGATED by design: the seven families share this
// organ, and per-family gating buys nothing (empty arrays clear to
// empty). The arch clear announces the portal-set change on the
// standing flag channel (mood_state_.portals_dirty).
inline void teardown_entities(Cartridge* c, wgpu::Queue& queue) {
    // Arches
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        c->entities_state_.arches[i] = ActiveArch{};
    }
    c->entities_state_.arch_count = 0;
    c->mood_state_.portals_dirty = true;
    c->gpuState_.set_arch_index_count(0);
    // Clear all arch mesh gen param slots
    {
        GPUArchMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
            c->gpuState_.upload_arch_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.arch_mesh_gen_pending = true;
    }

    // Columns + Antennas
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        c->entities_state_.columns[i] = ActiveColumn{};
    }
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        c->entities_state_.antennas[i] = ActiveColumn{};
    }
    c->entities_state_.column_count = 0;
    c->entities_state_.antenna_count = 0;
    c->gpuState_.set_column_index_count(0);
    // Clear all column mesh gen param slots
    {
        GPUColumnMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_COLUMN_INSTANCES; i++) {
            c->gpuState_.upload_column_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.column_mesh_gen_pending = true;
    }

    // Palms
    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
        c->entities_state_.palms[i] = ActivePalm{};
    }
    c->entities_state_.palm_count = 0;
    c->gpuState_.set_palm_index_count(0);
    {
        GPUPalmMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
            c->gpuState_.upload_palm_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.palm_mesh_gen_pending = true;
    }

    // Cacti
    for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
        c->entities_state_.cacti[i] = ActiveCactus{};
    }
    c->entities_state_.cactus_count = 0;
    c->gpuState_.set_cactus_index_count(0);
    {
        GPUCactusMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
            c->gpuState_.upload_cactus_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.cactus_mesh_gen_pending = true;
    }

    // Blade clusters
    for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
        c->entities_state_.blades[i] = ActiveBlade{};
    }
    c->entities_state_.blade_count = 0;
    c->gpuState_.set_blade_index_count(0);
    {
        GPUBladeClusterMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
            c->gpuState_.upload_blade_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.blade_mesh_gen_pending = true;
    }

    // Pyramids
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        c->entities_state_.pyramids[i] = ActivePyramid{};
    }
    c->entities_state_.pyramid_count = 0;
    c->entities_state_.cpu_pyramids = GPUPyramidArray{};
    c->gpuState_.upload_pyramids(queue, c->entities_state_.cpu_pyramids);
    c->gpuState_.set_pyramid_index_count(0);
    // Clear all mesh gen param slots (inactive → degenerates on next dispatch)
    {
        GPUPyramidMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
            c->gpuState_.upload_pyramid_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.pyramid_mesh_gen_pending = true;
    }
}

} // namespace the_board
} // namespace t7
