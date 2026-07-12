// ─── entities.inl (IMPL: post-class definitions) ─────────────────
//
// Definitions for entities.hpp's declared preparers + the arch
// force-spawn author (LADDER-4 — K4's channel). Included AFTER the
// Cartridge class (LADDER-2 c1 header/impl split) so the keyhole is a
// complete type — each preparer dereferences c->gpuState_'s index-count
// setters; the force-spawn author additionally reaches c->write_pier
// (K1's authoring channel), c->gpuState_'s mesh-params upload, and the
// in-class statics (Cartridge::ARCH_TIERS / Cartridge::ArchIdx /
// Cartridge::solve_catenary_a / Cartridge::PATCH_EXTENT — they reside
// in entity_pipeline.inl's class-body chapter). Vocabulary +
// EntitiesState + declarations live in entities.hpp (file scope, above
// the class).
//
// WRAPPING FORM (the proven fix-2 rule): this file is SELF-WRAPPING — it
// opens t7::the_board itself — so the MODULE IMPLEMENTATIONS zone includes
// it at FILE SCOPE, after the namespace closes. Definitions are `inline`
// free functions.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>   // std::max (portal-arch burial floor)
#include <cmath>       // std::floor, std::cos, std::sin (portal-arch placement)
#include <cstdint>

namespace t7 {
namespace the_board {

// ═══ MESH-GEN PREPARERS ═══════════════════════════════════════════
//
// Per-family CPU-side mesh-gen prep. Each preparer:
//   • Reads the family's *_mesh_gen_pending flag and clears it
//   • Scans the family's active array to find the highest active slot
//   • Uploads index count = (max_slot + 1) * indices_per_slot to GPU
//
// Counterparts for arch / column / antenna / pyramid live here too:
// migration #10 hoisted them out of spawn_engine.inl alongside the
// remaining helpers.

inline bool prepare_palm_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
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

inline bool prepare_cactus_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
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

inline bool prepare_blade_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
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

inline bool prepare_column_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
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

inline bool prepare_arch_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
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

inline bool prepare_pyramid_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
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

// ═══ THE ARCH FORCE-SPAWN AUTHOR (LADDER-4 — K4's channel) ═══════
//
// The arch's owner authors the forced portal-arch. Body EXTRACTED
// VERBATIM from mood's force_spawn_portal_at (LADDER-4): the mutation
// half of that function — everything from the slot scan to
// mesh-pending — moved here; mood keeps the value-computing half
// (color, destination, position) and calls through. Same operations,
// same order, same call timing: behavior-identical by construction.
// portals_dirty is the CALLER's flag (mood's own state) — mood sets it
// on success; the request-flags stay channel-shaped as ruled.

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

    const auto& tp = Cartridge::ARCH_TIERS[static_cast<uint32_t>(ArchTier::DOORWAY)];
    float half_span = tp.profile.params[Cartridge::ArchIdx::SPAN].mean * 0.5f;
    float rise = tp.profile.params[Cartridge::ArchIdx::RISE].mean;
    float depth = tp.profile.params[Cartridge::ArchIdx::DEPTH].mean;
    float thickness = tp.profile.params[Cartridge::ArchIdx::THICKNESS].mean;
    float pier_height = tp.profile.params[Cartridge::ArchIdx::PIER_HEIGHT].mean;
    float pier_padding = tp.profile.params[Cartridge::ArchIdx::PIER_PADDING].mean;
    float edge_blend = tp.profile.params[Cartridge::ArchIdx::EDGE_BLEND].mean;

    float pier_half_x = thickness * 0.5f + pier_padding + edge_blend;
    float pier_half_z = depth * 0.5f + pier_padding + edge_blend;

    int32_t gx = static_cast<int32_t>(std::floor(cx / Cartridge::PATCH_EXTENT));
    int32_t gz = static_cast<int32_t>(std::floor(cz / Cartridge::PATCH_EXTENT));

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
    c->write_pier(queue, pier_l_slot, pl);

    GPUPierInstance pr{};
    pr.origin[0] = pr_x;  pr.origin[1] = pr_z;
    pr.half_size[0] = pier_half_x;  pr.half_size[1] = pier_half_z;
    pr.height_near = pier_height;  pr.height_far = pier_height;
    pr.rotation = rotation;  pr.edge_blend = edge_blend;
    pr.tier = PierTier::ARCH_DOORWAY;
    pr.is_active = 1;
    c->write_pier(queue, pier_r_slot, pr);

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
    meshParams.catenary_a = Cartridge::solve_catenary_a(half_span, rise);
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

} // namespace the_board
} // namespace t7
