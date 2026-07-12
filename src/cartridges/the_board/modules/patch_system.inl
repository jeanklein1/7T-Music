// ─── patch_system.inl (S2 · IMPL: post-class definitions) ─────────
// Born at LADDER-6 (S2 extraction): history in audit/LADDER.md.
//
// The active-patch machine's verbs: the registry lifecycle (allocate
// → spawn → generate → evict), the frame budgets, world teardown, the
// layer allocator, and the streaming conductor. Reaches the keyhole
// for the root organs (c->world_state_ / c->player_ / the module
// states), the S3 dispatch seam (select_entities_for_patch / place /
// commit — spawn_engine.hpp), and the GPU wire (c->gpuState_ /
// c->renderer_).
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.

#include <cmath>          // std::floor, std::sqrt, std::abs
#include <algorithm>      // std::min, std::max
#include <cstring>        // std::memcpy (instance banding)
#include <unordered_set>  // the O(1) existence scan (continuous allocation)
#include <iostream>       // DIAG blocks (lifecycle audit + evict trace)

namespace t7 {
namespace the_board {

// ── The patch registry ─────────────────────────────────────────────

inline ActivePatch* find_patch(Cartridge* c, int32_t gx, int32_t gz) {
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (c->patch_system_state_.patches_[i].valid && c->patch_system_state_.patches_[i].grid_x == gx && c->patch_system_state_.patches_[i].grid_z == gz)
            return &c->patch_system_state_.patches_[i];
    }
    return nullptr;
}

// Hook: full eviction of a single patch.
inline void evict_patch(Cartridge* c, uint32_t pi, wgpu::Queue& queue) {
    free_layer(c, c->patch_system_state_.patches_[pi].layer);
    // Painting eviction now handled by evict_gallery (gallery.inl) via entity_refs
    evict_patch_entities(c, c->patch_system_state_.patches_[pi], queue);
    unregister_footprints_for_patch(c, c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z);
    c->patch_system_state_.patches_[pi].valid = false;
}

inline void evict_patch_entities(Cartridge* c, ActivePatch& patch, wgpu::Queue& queue) {
#ifdef DIAG_ENTITY_LIFECYCLE
    if (patch.entity_ref_count > 0) {
        float wx = (patch.grid_x + 0.5f) * PATCH_EXTENT;
        float wz = (patch.grid_z + 0.5f) * PATCH_EXTENT;
        float dx = wx - c->player_.readback_x, dz = wz - c->player_.readback_z;
        std::cout << "[DIAG:EVICT] patch(" << patch.grid_x << "," << patch.grid_z
            << ") dist=" << std::sqrt(dx * dx + dz * dz)
            << " refs=" << patch.entity_ref_count << "\n";
    }
#endif
    for (uint32_t i = 0; i < patch.entity_ref_count; i++) {
        auto& ref = patch.entity_refs[i];
        FAMILY_DISPATCH[ref.family].evict_slot(c, ref.slot, queue);
    }

    patch.entity_ref_count = 0;
}

inline void audit_entity_integrity(Cartridge* c) {
    (void)c;
#ifdef DIAG_ENTITY_LIFECYCLE
    //
    uint32_t act_a = 0, act_c = 0, act_n = 0, act_p = 0;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) if (c->entities_state_.arches[i].active) act_a++;
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) if (c->entities_state_.columns[i].active) act_c++;
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) if (c->entities_state_.antennas[i].active) act_n++;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) if (c->entities_state_.pyramids[i].active) act_p++;

    // Count consistency
    if (act_a != c->entities_state_.arch_count)
        std::cout << "[DIAG:AUDIT] ARCH COUNT active=" << act_a << " tracked=" << c->entities_state_.arch_count << "\n";
    if (act_c != c->entities_state_.column_count)
        std::cout << "[DIAG:AUDIT] COL COUNT active=" << act_c << " tracked=" << c->entities_state_.column_count << "\n";
    if (act_n != c->entities_state_.antenna_count)
        std::cout << "[DIAG:AUDIT] ANT COUNT active=" << act_n << " tracked=" << c->entities_state_.antenna_count << "\n";
    if (act_p != c->entities_state_.pyramid_count)
        std::cout << "[DIAG:AUDIT] PYR COUNT active=" << act_p << " tracked=" << c->entities_state_.pyramid_count << "\n";

    // Collect refs from all patches
    bool ra[Dim::MAX_ARCH_INSTANCES]{};
    bool rc[Dim::MAX_COLUMN_ONLY]{};
    bool rn[Dim::MAX_ANTENNA_ONLY]{};
    bool rp[Dim::MAX_PYRAMID_INSTANCES]{};
    for (uint32_t p = 0; p < c->world_state_.active_patch_count; p++) {
        if (!c->patch_system_state_.patches_[p].valid) continue;
        for (uint32_t r = 0; r < c->patch_system_state_.patches_[p].entity_ref_count; r++) {
            auto& ref = c->patch_system_state_.patches_[p].entity_refs[r];
            switch (ref.family) {
            case PopFamily::PYRAMID:
                if (ref.slot < Dim::MAX_PYRAMID_INSTANCES) {
                    if (rp[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF pyr slot=" << ref.slot << " patch=(" << c->patch_system_state_.patches_[p].grid_x << "," << c->patch_system_state_.patches_[p].grid_z << ")\n";
                    rp[ref.slot] = true;
                } break;
            case PopFamily::ARCH:
                if (ref.slot < Dim::MAX_ARCH_INSTANCES) {
                    if (ra[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF arch slot=" << ref.slot << " patch=(" << c->patch_system_state_.patches_[p].grid_x << "," << c->patch_system_state_.patches_[p].grid_z << ")\n";
                    ra[ref.slot] = true;
                } break;
            case PopFamily::COLUMN:
                if (ref.slot < Dim::MAX_COLUMN_ONLY) {
                    if (rc[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF col slot=" << ref.slot << " patch=(" << c->patch_system_state_.patches_[p].grid_x << "," << c->patch_system_state_.patches_[p].grid_z << ")\n";
                    rc[ref.slot] = true;
                } break;
            case PopFamily::ANTENNA:
                if (ref.slot < Dim::MAX_ANTENNA_ONLY) {
                    if (rn[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF ant slot=" << ref.slot << " patch=(" << c->patch_system_state_.patches_[p].grid_x << "," << c->patch_system_state_.patches_[p].grid_z << ")\n";
                    rn[ref.slot] = true;
                } break;
            }
        }
    }

    // Ghost: active but no ref (will never be evicted)
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++)
        if (c->entities_state_.arches[i].active && !ra[i])
            std::cout << "[DIAG:AUDIT] GHOST arch slot=" << i << " host=(" << c->entities_state_.arches[i].host_gx << "," << c->entities_state_.arches[i].host_gz << ")\n";
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++)
        if (c->entities_state_.columns[i].active && !rc[i])
            std::cout << "[DIAG:AUDIT] GHOST col slot=" << i << " host=(" << c->entities_state_.columns[i].host_gx << "," << c->entities_state_.columns[i].host_gz << ")\n";
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++)
        if (c->entities_state_.antennas[i].active && !rn[i])
            std::cout << "[DIAG:AUDIT] GHOST ant slot=" << i << " host=(" << c->entities_state_.antennas[i].host_gx << "," << c->entities_state_.antennas[i].host_gz << ")\n";
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++)
        if (c->entities_state_.pyramids[i].active && !rp[i])
            std::cout << "[DIAG:AUDIT] GHOST pyr slot=" << i << " host=(" << c->entities_state_.pyramids[i].host_gx << "," << c->entities_state_.pyramids[i].host_gz << ")\n";

    // Orphan: ref but not active (ref points to freed slot)
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++)
        if (!c->entities_state_.arches[i].active && ra[i])
            std::cout << "[DIAG:AUDIT] ORPHAN arch slot=" << i << "\n";
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++)
        if (!c->entities_state_.columns[i].active && rc[i])
            std::cout << "[DIAG:AUDIT] ORPHAN col slot=" << i << "\n";
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++)
        if (!c->entities_state_.antennas[i].active && rn[i])
            std::cout << "[DIAG:AUDIT] ORPHAN ant slot=" << i << "\n";
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++)
        if (!c->entities_state_.pyramids[i].active && rp[i])
            std::cout << "[DIAG:AUDIT] ORPHAN pyr slot=" << i << "\n";

    // Ref overflow: any patch at capacity
    for (uint32_t p = 0; p < c->world_state_.active_patch_count; p++) {
        if (c->patch_system_state_.patches_[p].valid && c->patch_system_state_.patches_[p].entity_ref_count >= ActivePatch::MAX_ENTITY_REFS)
            std::cout << "[DIAG:AUDIT] REF FULL patch=(" << c->patch_system_state_.patches_[p].grid_x << "," << c->patch_system_state_.patches_[p].grid_z << ") count=" << c->patch_system_state_.patches_[p].entity_ref_count << "\n";
    }
#endif
}

// ── Dynamic budgets ────────────────────────────────────────────────

inline uint32_t count_pending_patches(Cartridge* c) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        if (c->patch_system_state_.patches_[i].phase == PatchPhase::SPAWNED ||
            c->patch_system_state_.patches_[i].phase == PatchPhase::NEEDS_REGEN) n++;
    }
    return n;
}

inline uint32_t patches_budget_this_frame(Cartridge* c) {
    uint32_t pending = count_pending_patches(c);
    uint32_t budget = PATCH_BUDGET_MIN;
    if (pending >= PATCH_PENDING_TIER_4) budget = 6;
    else if (pending >= PATCH_PENDING_TIER_3) budget = 4;
    else if (pending >= PATCH_PENDING_TIER_2) budget = 3;
    else if (pending >= PATCH_PENDING_TIER_1) budget = 2;

    bool moving = (std::abs(c->inputState_.move_x) > 0.01f ||
        std::abs(c->inputState_.move_z) > 0.01f);
    if (moving && pending > PATCH_BUDGET_MOVE_THRESHOLD)
        budget += 1;

    return std::min(budget, PATCH_BUDGET_MAX);
}

// ── World lifecycle ────────────────────────────────────────────────
//
// Keyhole form (Phase R stamp, R-b). CALLER: the transition machine
// (root); OWNER: patch_system.
inline void teardown_world(Cartridge* c, wgpu::Queue& queue) {
    // Patches + tile cache
    init_patch_system(c);
    c->world_state_.last_center_x = INT32_MAX;  // force full regen on next frame
    c->world_state_.last_center_z = INT32_MAX;

    // Terrain tokens
    for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
        c->tile_world_state_.terrainTokens_[t] = TerrainToken{};
    }

    c->entityQueue_.clear();
    c->placementResults_.clear();

    // Theme envelope
    c->themes_state_ = ThemesState{};

    // Clear all entity piers (keep test rig at slots 0-2)
    for (uint32_t i = Dim::PIER_ARCH_BASE; i < Dim::PIER_TOTAL; i++) {
        clear_pier(c, queue, i);
    }

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

    // GoL zones
    for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
        c->gol_state_.zones[i] = GoLZoneState{};
    }
    c->gol_state_.zone_count = 0;
    c->gol_state_.active_slot_count = 0;
    c->gol_state_.pending_derive_requests.count = 0;
    GPUGoLZoneArray emptyZones{};
    c->gpuState_.upload_zone_config(queue, emptyZones);

    // Ribbon — clear all slots
    {
        for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
            c->ribbon_state_.active[i] = ActiveRibbon{};
            c->ribbon_state_.gpu[i] = GPURibbonState{};
        }
        c->ribbon_state_.active_count = 0;
        c->ribbon_state_.rendered_slot = UINT32_MAX;
        GPURibbonState empty{};
        c->gpuState_.upload_ribbon(queue, empty);
    }

    // Sphere + cube clears are per-owner functions (clear_spheres /
    // clear_cubes) — CPU + per-slot-GPU paired. See §5 TEARDOWN BULK SWEEPS.
    clear_spheres(c->sphere_state_, c->gpuState_, queue);
    clear_cubes(c->cube_behaviors_state_, c->gpuState_, queue);

    // Gallery / paintings — clear all exhibition + slots, keep staging intact
    for (uint32_t i = 0; i < MAX_GALLERIES; i++) {
        c->gallery_state_.gallery_centers[i] = GalleryCenter{};
    }
    c->gallery_state_.pending_snapshot.active = false;
    c->gallery_state_.pending_promotion_count = 0;
    c->gallery_state_.wall_frame_count = 0;
    c->gallery_state_.active_painting_count = 0;
    // Clear all painting slots (CPU + GPU)
    for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
        c->gallery_state_.painting_slots[i] = GPUPaintingSlot{};
    }
    {
        GPUPaintingSlot empty[Dim::PAINTING_MAX_SLOTS]{};
        c->gpuState_.upload_painting_slots(queue, empty, Dim::PAINTING_MAX_SLOTS);
    }
    // Free all exhibition layers (staging persists across worlds)
    for (uint32_t i = 0; i < Dim::EXHIBITION_LAYERS; i++) c->gallery_state_.exhibition_occupied[i] = false;
    c->gallery_state_.exhibition_count = 0;
    rotate_authored_staging(c->gallery_state_, c, queue);
    for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) c->gallery_state_.authored_staging[i].consumed = false;

    // Footprints
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        c->footprints_[i] = GroundFootprint{};
    }

    // Aura
    if constexpr (ROSTER.pawn_aura) {  // ROSTER-GATE pawn_aura (c) — teardown clear skipped when disabled (no aura to clear)
        c->pawn_state_.aura_needs_clear = true;
        c->pawn_state_.aura_cfg_dirty = true;
    }

    // Sky orbs: apply_mood re-enables + re-seeds as needed
    if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (c) — teardown one-shot skipped when disabled
        teardown_orbs(c->orbs_state_, c);

    // Indoor shell
    c->gpuState_.set_shell_index_count(0);

    // Lights need re-upload with potentially new config
    c->entities_state_.lights_dirty = true;

    // New world decides its own upload frequency policy
    c->gpuState_.set_config_dynamic(false);
}

// ── The pier writers ───────────────────────────────────────────────
//
// Rode in from spawn_engine at its conversion (Phase R stamp: PIERS
// ride patch_system); cpuPiers_ is module state (patch_system_state_).
inline void write_pier(Cartridge* c, wgpu::Queue& queue, uint32_t slot, const GPUPierInstance& pier) {
    c->patch_system_state_.cpuPiers_[slot] = pier;
    c->gpuState_.upload_pier_slot(queue, slot, pier);
    c->world_state_.pier_count_dirty = true;
    c->world_state_.ground_entries_dirty = true;
}

inline void clear_pier(Cartridge* c, wgpu::Queue& queue, uint32_t slot) {
    GPUPierInstance empty{};
    c->patch_system_state_.cpuPiers_[slot] = empty;
    c->gpuState_.upload_pier_slot(queue, slot, empty);
    c->world_state_.pier_count_dirty = true;
    c->world_state_.ground_entries_dirty = true;
}

inline void recompute_and_upload_pier_count(Cartridge* c, wgpu::Queue& queue) {
    uint32_t highest = 0;
    for (uint32_t i = 0; i < Dim::PIER_TOTAL; i++) {
        if (c->patch_system_state_.cpuPiers_[i].is_active) highest = i + 1;
    }
    c->gpuState_.config().pier_count = highest;
    c->gpuState_.upload_pier_count(queue);
}

inline void flush_pier_count(Cartridge* c, wgpu::Queue& queue) {
    if (!c->world_state_.pier_count_dirty) return;
    c->world_state_.pier_count_dirty = false;
    recompute_and_upload_pier_count(c, queue);
}

// Rode in from spawn_engine at its conversion (Phase R stamp): the
// pier writers' regen fan-out over the registry.
inline void mark_patches_for_regen(Cartridge* c, float min_wx, float min_wz,
    float max_wx, float max_wz,
    int32_t home_gx, int32_t home_gz) {
    int32_t pg_x0 = (int32_t)std::floor(min_wx / PATCH_EXTENT);
    int32_t pg_x1 = (int32_t)std::floor(max_wx / PATCH_EXTENT);
    int32_t pg_z0 = (int32_t)std::floor(min_wz / PATCH_EXTENT);
    int32_t pg_z1 = (int32_t)std::floor(max_wz / PATCH_EXTENT);

    for (uint32_t p = 0; p < c->world_state_.active_patch_count; p++) {
        if (c->patch_system_state_.patches_[p].phase != PatchPhase::GENERATED) continue;
        if (c->patch_system_state_.patches_[p].grid_x == home_gx && c->patch_system_state_.patches_[p].grid_z == home_gz) continue;
        if (c->patch_system_state_.patches_[p].grid_x >= pg_x0 && c->patch_system_state_.patches_[p].grid_x <= pg_x1 &&
            c->patch_system_state_.patches_[p].grid_z >= pg_z0 && c->patch_system_state_.patches_[p].grid_z <= pg_z1) {
            c->patch_system_state_.patches_[p].phase = PatchPhase::NEEDS_REGEN;
        }
    }
}

// ── Patch subsystem setup ──────────────────────────────────────────

inline void init_patch_system(Cartridge* c) {
    for (uint32_t i = 0; i < MAX_PATCHES; i++) {
        c->patch_system_state_.freeLayerStack_[i] = MAX_PATCHES - 1 - i;
    }
    c->world_state_.free_layer_count = MAX_PATCHES;
    c->world_state_.active_patch_count = 0;
    c->world_state_.render_patch_count = 0;
    c->world_state_.lod0_patch_count = 0;
    c->world_state_.all_patch_count = 0;
    c->gpuState_.config().placement_patch_count = 0;
    c->tile_world_state_.tileCache_.clear();
    c->world_state_.pier_count_dirty = true;
    c->world_state_.ground_entries_dirty = true;
    c->world_state_.patch_instances_dirty = true;
    c->world_state_.placement_dirty = true;
}

// Test rig piers: ramp + plateau + block at pier slots 0-2.
// Same geometry as the old test rig solids, now as GPUPierInstance.
// TESTING[test-rig-piers] (ROSTER-1a §1 ruling): a debug ground
//   fixture, NOT a roster piece (roster rows are design pieces,
//   not scaffolds). Mortal retirement: dies at ship (checklist).
//   Joins the future exhibition-guard discussion alongside
//   SEAM[spawn_engine:L1]'s DIAG_ENTITY_LIFECYCLE. Constitution §5
//   TESTING class.
inline void setup_test_rig_piers(Cartridge* c, wgpu::Queue queue) {
    // Ramp: height 0→3 along +X.
    GPUPierInstance ramp{};
    ramp.origin[0] = 12.0f;  ramp.origin[1] = 0.0f;
    ramp.half_size[0] = 6.5f; ramp.half_size[1] = 3.0f;
    ramp.height_near = 0.0f;  ramp.height_far = 3.0f;
    ramp.rotation = 0.0f;
    ramp.edge_blend = 0.5f;
    ramp.tier = PierTier::TEST_RIG;
    ramp.is_active = 1;
    write_pier(c, queue, 0, ramp);

    // Plateau: flat at height 3, overlaps ramp at x=18.
    GPUPierInstance plat{};
    plat.origin[0] = 21.0f;  plat.origin[1] = 0.0f;
    plat.half_size[0] = 3.5f; plat.half_size[1] = 3.0f;
    plat.height_near = 3.0f;  plat.height_far = 3.0f;
    plat.rotation = 0.0f;
    plat.edge_blend = 0.5f;
    plat.tier = PierTier::TEST_RIG;
    plat.is_active = 1;
    write_pier(c, queue, 1, plat);

    // Block: sharp edges → step-height walls (impassable).
    GPUPierInstance block{};
    block.origin[0] = 21.0f;  block.origin[1] = 0.0f;
    block.half_size[0] = 1.2f; block.half_size[1] = 1.2f;
    block.height_near = 5.0f;  block.height_far = 5.0f;
    block.rotation = 0.0f;
    block.edge_blend = 0.0f;
    block.tier = PierTier::TEST_RIG;
    block.is_active = 1;
    write_pier(c, queue, 2, block);
}

// ── Patch generation ───────────────────────────────────────────────

inline void generate_patch_batch(Cartridge* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count,
    uint32_t stagingOffset) {
    if (count == 0) return;

    // One WriteBuffer: all params into staging at the given offset
    c->gpuState_.upload_patch_staging(queue, params, count, stagingOffset);

    for (uint32_t i = 0; i < count; i++) {
        // Copy this patch's params from staging slot → active params buffer
        encoder.CopyBufferToBuffer(
            c->gpuState_.patch_staging_buffer(), (stagingOffset + i) * sizeof(GPUPatchParams),
            c->gpuState_.patch_params_buffer(), 0,
            sizeof(GPUPatchParams));

        // Pass 1: heights only (one ground_formed_with_complexity per texel)
        {
            wgpu::ComputePassDescriptor cpd{};
            cpd.label = "Patch Heights (pass 1)";
            wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
            c->renderer_.dispatch_generate_patch_heights(cp, c->gpuState_.patch_gen_group(), GPUState::patch_heightfield_workgroups());
            cp.End();
        }

        // Pass 2: gradients from neighbor reads + complexity + cell colors
        {
            wgpu::ComputePassDescriptor cpd{};
            cpd.label = "Patch Gradients + Cells (pass 2)";
            wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
            c->renderer_.dispatch_generate_patch_gradients(cp, c->gpuState_.patch_gen_group(), GPUState::patch_heightfield_workgroups());
            c->renderer_.dispatch_generate_patch_cells(cp, c->gpuState_.patch_gen_group(), GPUState::patch_cell_workgroups());
            cp.End();
        }
    }
}

inline GPUPatchParams make_patch_params(Cartridge* c, int32_t gx, int32_t gz, uint32_t layer) {
    GPUPatchParams p{};
    p.origin[0] = (gx + 0.5f) * PATCH_EXTENT;
    p.origin[1] = (gz + 0.5f) * PATCH_EXTENT;
    p.extent = PATCH_EXTENT;
    p.resolution = 256;
    p.master_seed = c->world_state_.active_seed;
    p.time = 0.0f;
    p.layer = layer;
    p._pad1 = 0.0f;
    return p;
}

// ── Layer allocator ────────────────────────────────────────────────

inline uint32_t alloc_layer(Cartridge* c) {
    if (c->world_state_.free_layer_count == 0) {
        // Safety: no free layers — recycle layer 0 rather than crash.
        // This shouldn't happen if eviction works correctly.
        return 0;
    }
    return c->patch_system_state_.freeLayerStack_[--c->world_state_.free_layer_count];
}

inline void free_layer(Cartridge* c, uint32_t layer) {
    c->patch_system_state_.freeLayerStack_[c->world_state_.free_layer_count++] = layer;
}

// Check if grid coordinate is within the allocation window (world_state_.active_radius = PREGEN_RADIUS)
inline bool in_render_window(Cartridge* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
    int32_t r = (int32_t)c->world_state_.active_radius;
    return gx >= cx - r && gx <= cx + r &&
        gz >= cz - r && gz <= cz + r;
}

// ── Visibility cylinder ────────────────────────────────────────────

// Distance² from point (px,pz) to nearest edge of a patch AABB.
// Zero when the point is inside the patch.
inline float patch_distance_sq(float px, float pz,
    float origin_x, float origin_z, float half) {
    float dx = std::max(0.0f, std::abs(px - origin_x) - half);
    float dz = std::max(0.0f, std::abs(pz - origin_z) - half);
    return dx * dx + dz * dz;
}

// ── Patch streaming helpers ────────────────────────────────────────

template<typename Pred>
inline uint32_t collect_sorted_patches(Cartridge* c, PatchCandidate* out,
    float pawn_wx, float pawn_wz, Pred&& pred, bool nearest_first)
{
    float half = PATCH_EXTENT * 0.5f;
    uint32_t count = 0;
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        if (!pred(c->patch_system_state_.patches_[i])) continue;
        float ox = (c->patch_system_state_.patches_[i].grid_x + 0.5f) * PATCH_EXTENT;
        float oz = (c->patch_system_state_.patches_[i].grid_z + 0.5f) * PATCH_EXTENT;
        float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);
        out[count++] = { i, d2 };
    }
    for (uint32_t i = 1; i < count; i++) {
        PatchCandidate key = out[i];
        uint32_t j = i;
        if (nearest_first) {
            while (j > 0 && out[j - 1].dist2 > key.dist2) {
                out[j] = out[j - 1]; j--;
            }
        }
        else {
            while (j > 0 && out[j - 1].dist2 < key.dist2) {
                out[j] = out[j - 1]; j--;
            }
        }
        out[j] = key;
    }
    return count;
}

// Check if grid coordinate is within the priority window (GRID_RADIUS)
inline bool in_priority_window(Cartridge* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
    int32_t r = (int32_t)GRID_RADIUS;
    return gx >= cx - r && gx <= cx + r &&
        gz >= cz - r && gz <= cz + r;
}

// Process entity spawn for pre-collected patch candidates.
inline void spawn_selected_patches(Cartridge* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::Queue& queue)
{
    for (uint32_t s = 0; s < count; s++) {
        uint32_t pi = candidates[s].idx;
        c->themes_state_.active_theme_idx_ = evaluate_theme_envelope(c->themes_state_, c, 
            tile_seed(c->world_state_.active_seed, c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z));
        select_entities_for_patch(c, c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z);
        c->patch_system_state_.patches_[pi].phase = PatchPhase::SPAWNED;
    }
    place_entity_queue(c);
    commit_entity_queue(c, queue);

    for (uint32_t s = 0; s < count; s++) {
        uint32_t pi = candidates[s].idx;
        int32_t gx = c->patch_system_state_.patches_[pi].grid_x;
        int32_t gz = c->patch_system_state_.patches_[pi].grid_z;
        for (uint32_t r = 0; r < MAX_RIBBON_INSTANCES; r++) {
            auto& ar = c->ribbon_state_.active[r];
            if (!ar.active) continue;
            // Check near tip
            if (!ar.near_tip_registered &&
                ar.near_tip_gx == gx && ar.near_tip_gz == gz) {
                c->patch_system_state_.patches_[pi].record_entity(PopFamily::RIBBON, r);
                ar.near_tip_registered = true;
                ar.ref_count++;
            }
            // Check far tip
            if (!ar.far_tip_registered &&
                ar.far_tip_gx == gx && ar.far_tip_gz == gz) {
                c->patch_system_state_.patches_[pi].record_entity(PopFamily::RIBBON, r);
                ar.far_tip_registered = true;
                ar.ref_count++;
            }
        }
    }
}

// Hook: fires once when a patch transitions SPAWNED → GENERATED.
inline void on_patch_first_generated(Cartridge* c, uint32_t pi, wgpu::Queue& queue) {
    // Galleries → entity pipeline (select_gallery_for_patch)
    // GoL zones → entity pipeline (select_gol_for_patch)
    (void)c; (void)pi; (void)queue;
}

// Process heightfield generation for pre-collected patch candidates.
inline void generate_selected_patches(Cartridge* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    uint32_t& patchStagingOffset, bool& tileGridDirty)
{
    if (count == 0) return;
    if (tileGridDirty) {
        upload_tile_grid_now(c->tile_world_state_, c, queue, c->world_state_.last_center_x, c->world_state_.last_center_z);
        tileGridDirty = false;
    }
    GPUPatchParams batchParams[MAX_PATCHES];
    uint32_t batchIdx[MAX_PATCHES];
    for (uint32_t i = 0; i < count; i++) {
        uint32_t pi = candidates[i].idx;
        batchParams[i] = make_patch_params(c, 
            c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z, c->patch_system_state_.patches_[pi].layer);
        batchIdx[i] = pi;
    }
    generate_patch_batch(c, encoder, queue, batchParams, count, patchStagingOffset);
    patchStagingOffset += count;
    for (uint32_t b = 0; b < count; b++) {
        uint32_t pi = batchIdx[b];
        bool first_gen = (c->patch_system_state_.patches_[pi].phase == PatchPhase::SPAWNED);
        c->patch_system_state_.patches_[pi].phase = PatchPhase::GENERATED;
        if (first_gen) {
            on_patch_first_generated(c, pi, queue);
        }
    }
    c->world_state_.patch_instances_dirty = true;
}

// --- Patch streaming: determine active 7×7 grid, generate new patches ---
// THE CONDUCTOR (Phase R stamp, R-c): the per-frame step — recenter,
// eviction, allocation, spawn + generation budgets, visibility
// banding, deferred uploads.
// SEAM[patch:spawn-trigger] the S3-trigger calls are the declared
//   seam face: select_entities_for_patch / place_entity_queue /
//   commit_entity_queue (via spawn_selected_patches), plus
//   update_entity_draw_visibility + flush_pier_count at the
//   frame tail — the surface machine waking the occupier machine.
inline void stream_patches(Cartridge* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
    // ─── Patch Generation Pipeline ─────────────────────────────────

    int32_t centerX, centerZ;
    uint32_t patchStagingOffset = 0;  // running offset into staging buffer (multiple batches per frame)
    bool tileGridDirty = false;        // coalesce tile grid uploads to one per frame
    if (c->world_state_.finite_mode) {
        centerX = 0;
        centerZ = 0;
    }
    else {
        centerX = (int32_t)std::floor(c->player_.readback_x / PATCH_EXTENT);
        centerZ = (int32_t)std::floor(c->player_.readback_z / PATCH_EXTENT);
    }

    // In finite mode, cap the effective radius
    uint32_t savedRadius = c->world_state_.active_radius;
    if (c->world_state_.finite_mode && c->world_state_.active_radius > c->world_state_.finite_radius) {
        c->world_state_.active_radius = c->world_state_.finite_radius;
    }

    bool gridChanged = (centerX != c->world_state_.last_center_x || centerZ != c->world_state_.last_center_z);

    if (gridChanged) {
        int32_t oldCX = c->world_state_.last_center_x;
        int32_t oldCZ = c->world_state_.last_center_z;
        c->world_state_.last_center_x = centerX;
        c->world_state_.last_center_z = centerZ;

        bool fullRegen = (oldCX == INT32_MAX);  // first frame

        // Lightweight cache maintenance (no GPU buffer writes)
        evict_distant_tiles(c->tile_world_state_, centerX, centerZ);

        if (!fullRegen) {
            tileGridDirty = true;
        }

        // ─── FULLREGEN: synchronous bootstrap ────────────────────
        //
        if (fullRegen) {
            int32_t rr = (int32_t)c->world_state_.active_radius;
            static constexpr int32_t TILE_PAD = 1;
            int32_t rp = rr + TILE_PAD;
            for (int32_t gz = centerZ - rp; gz <= centerZ + rp; gz++) {
                for (int32_t gx = centerX - rp; gx <= centerX + rp; gx++) {
                    GridKey key{ gx, gz };
                    if (c->tile_world_state_.tileCache_.find(key) == c->tile_world_state_.tileCache_.end()) {
                        TileState ts = generate_tile_state(c->tile_world_state_, c, gx, gz);
                        tick_terrain_tokens(c->tile_world_state_, ts, tile_seed(c->world_state_.active_seed, gx, gz));
                        c->tile_world_state_.tileCache_[key] = ts;
                    }
                }
            }

            // NOW spawn portals — tile cache is populated, terrain heights are correct
            if (c->mood_state_.back_portal_pending) {
                force_spawn_back_portal(c, queue);
            }
            for (int32_t gz = centerZ - rr; gz <= centerZ + rr; gz++) {
                for (int32_t gx = centerX - rr; gx <= centerX + rr; gx++) {
                    bool found = false;
                    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
                        if (c->patch_system_state_.patches_[i].grid_x == gx && c->patch_system_state_.patches_[i].grid_z == gz) {
                            found = true; break;
                        }
                    }
                    if (!found && c->world_state_.free_layer_count > 0) {
                        uint32_t layer = alloc_layer(c);
                        c->patch_system_state_.patches_[c->world_state_.active_patch_count] = ActivePatch{};
                        c->patch_system_state_.patches_[c->world_state_.active_patch_count].grid_x = gx;
                        c->patch_system_state_.patches_[c->world_state_.active_patch_count].grid_z = gz;
                        c->patch_system_state_.patches_[c->world_state_.active_patch_count].layer = layer;
                        c->patch_system_state_.patches_[c->world_state_.active_patch_count].valid = true;
                        c->world_state_.active_patch_count++;
                    }
                }
            }
            tileGridDirty = true;

            // Spawn inner patches
            PatchCandidate spawnCands[MAX_PATCHES];
            uint32_t spawnCount = collect_sorted_patches(c, spawnCands,
                c->player_.readback_x, c->player_.readback_z,
                [&](const ActivePatch& p) {
                    return p.phase == PatchPhase::ALLOCATED &&
                        in_priority_window(c, p.grid_x, p.grid_z, centerX, centerZ);
                }, true);
            spawn_selected_patches(c, spawnCands, spawnCount, queue);

            // Generate inner patches
            PatchCandidate genCands[MAX_PATCHES];
            uint32_t genCount = collect_sorted_patches(c, genCands,
                c->player_.readback_x, c->player_.readback_z,
                [&](const ActivePatch& p) {
                    return p.phase == PatchPhase::SPAWNED &&
                        in_priority_window(c, p.grid_x, p.grid_z, centerX, centerZ);
                }, true);
            generate_selected_patches(c, genCands, genCount,
                encoder, queue, patchStagingOffset, tileGridDirty);
        }
    }

    // ─── CONTINUOUS PATCH EVICTION ────────────────────────────────
    //
    {
        PatchCandidate candidates[MAX_PATCHES];
        uint32_t count = collect_sorted_patches(c, candidates,
            c->player_.readback_x, c->player_.readback_z,
            [&](const ActivePatch& p) {
                return !in_render_window(c, p.grid_x, p.grid_z,
                    c->world_state_.last_center_x, c->world_state_.last_center_z);
            }, false);  // farthest first

        uint32_t evictThisFrame = std::min(count, EVICT_BUDGET_PER_FRAME);
        for (uint32_t e = 0; e < evictThisFrame; e++) {
            evict_patch(c, candidates[e].idx, queue);
        }

        if (evictThisFrame > 0) {
            uint32_t write = 0;
            for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
                if (c->patch_system_state_.patches_[i].valid) c->patch_system_state_.patches_[write++] = c->patch_system_state_.patches_[i];
            }
            c->world_state_.active_patch_count = write;
            c->world_state_.patch_instances_dirty = true;
        }
    }

    // ─── CONTINUOUS PATCH ALLOCATION ──────────────────────────────
    //
    {
        int32_t pawnGX = (int32_t)std::floor(c->player_.readback_x / PATCH_EXTENT);
        int32_t pawnGZ = (int32_t)std::floor(c->player_.readback_z / PATCH_EXTENT);
        int32_t rr = (int32_t)c->world_state_.active_radius;
        float pawn_wx = c->player_.readback_x;
        float pawn_wz = c->player_.readback_z;
        float half = PATCH_EXTENT * 0.5f;

        // O(1) patch existence lookup (replaces O(N) inner scan)
        std::unordered_set<GridKey, GridKeyHash> activePatchSet;
        activePatchSet.reserve(c->world_state_.active_patch_count);
        for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
            activePatchSet.insert({ c->patch_system_state_.patches_[i].grid_x, c->patch_system_state_.patches_[i].grid_z });
        }

        struct AllocCandidate { int32_t gx, gz; float dist2; };
        AllocCandidate candidates[MAX_PATCHES];
        uint32_t candidateCount = 0;

        for (int32_t gz = pawnGZ - rr; gz <= pawnGZ + rr; gz++) {
            for (int32_t gx = pawnGX - rr; gx <= pawnGX + rr; gx++) {
                // Must be within allocation window of grid center
                if (!in_render_window(c, gx, gz, c->world_state_.last_center_x, c->world_state_.last_center_z)) continue;
                bool found = activePatchSet.count({ gx, gz }) > 0;
                if (!found && c->world_state_.free_layer_count > 0 && candidateCount < MAX_PATCHES) {
                    float ox = (gx + 0.5f) * PATCH_EXTENT;
                    float oz = (gz + 0.5f) * PATCH_EXTENT;
                    float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);
                    candidates[candidateCount++] = { gx, gz, d2 };
                }
            }
        }

        // Sort by distance (nearest first)
        for (uint32_t i = 1; i < candidateCount; i++) {
            AllocCandidate key = candidates[i];
            uint32_t j = i;
            while (j > 0 && candidates[j - 1].dist2 > key.dist2) {
                candidates[j] = candidates[j - 1];
                j--;
            }
            candidates[j] = key;
        }

        bool allocated_any = false;
        uint32_t allocThisFrame = std::min(candidateCount, ALLOC_BUDGET_PER_FRAME);
        for (uint32_t a = 0; a < allocThisFrame; a++) {
            int32_t gx = candidates[a].gx;
            int32_t gz = candidates[a].gz;
            // Ensure tile cache entry (primary — ticks terrain tokens)
            GridKey key{ gx, gz };
            if (c->tile_world_state_.tileCache_.find(key) == c->tile_world_state_.tileCache_.end()) {
                TileState ts = generate_tile_state(c->tile_world_state_, c, gx, gz);
                tick_terrain_tokens(c->tile_world_state_, ts, tile_seed(c->world_state_.active_seed, gx, gz));
                c->tile_world_state_.tileCache_[key] = ts;
            }
            // Also cache neighbors for tile grid padding
            for (int dz = -1; dz <= 1; dz++) for (int dx = -1; dx <= 1; dx++) {
                GridKey nk{ gx + dx, gz + dz };
                if (c->tile_world_state_.tileCache_.find(nk) == c->tile_world_state_.tileCache_.end()) {
                    c->tile_world_state_.tileCache_[nk] = generate_tile_state(c->tile_world_state_, c, gx + dx, gz + dz);
                }
            }
            uint32_t layer = alloc_layer(c);
            c->patch_system_state_.patches_[c->world_state_.active_patch_count] = ActivePatch{};
            c->patch_system_state_.patches_[c->world_state_.active_patch_count].grid_x = gx;
            c->patch_system_state_.patches_[c->world_state_.active_patch_count].grid_z = gz;
            c->patch_system_state_.patches_[c->world_state_.active_patch_count].layer = layer;
            c->patch_system_state_.patches_[c->world_state_.active_patch_count].valid = true;
            c->world_state_.active_patch_count++;
            allocated_any = true;
        }

        // Mark tile grid and patch instances dirty whenever new patches were allocated
        if (allocated_any) {
            tileGridDirty = true;
            c->world_state_.patch_instances_dirty = true;
        }
    }

    // ─── DISTANCE-DRIVEN ENTITY SPAWNING ─────────────────────────
    //
    {
        PatchCandidate candidates[MAX_PATCHES];
        uint32_t count = collect_sorted_patches(c, candidates,
            c->player_.readback_x, c->player_.readback_z,
            [](const ActivePatch& p) {
                return p.phase == PatchPhase::ALLOCATED;
            }, true);
        spawn_selected_patches(c, candidates,
            std::min(count, SPAWN_BUDGET_PER_FRAME), queue);
    }

    // ─── DISTANCE-DRIVEN HEIGHTFIELD GENERATION ──────────────────
    //
    {
        PatchCandidate candidates[MAX_PATCHES];
        uint32_t count = collect_sorted_patches(c, candidates,
            c->player_.readback_x, c->player_.readback_z,
            [](const ActivePatch& p) {
                return p.phase == PatchPhase::SPAWNED ||
                    p.phase == PatchPhase::NEEDS_REGEN;
            }, true);
        generate_selected_patches(c, candidates,
            std::min(count, patches_budget_this_frame(c)),
            encoder, queue, patchStagingOffset, tileGridDirty);
    }

    {
        GPUPatchInstance instances[MAX_PATCHES]{};
        uint32_t lod0Count = 0;
        uint32_t lod1Count = 0;
        uint32_t pregenCount = 0;

        // Temporary arrays for each band
        GPUPatchInstance lod0[MAX_PATCHES]{};
        GPUPatchInstance lod1[MAX_PATCHES]{};
        GPUPatchInstance pregen[MAX_PATCHES]{};

        float pawn_wx = c->player_.readback_x;
        float pawn_wz = c->player_.readback_z;
        float half = PATCH_EXTENT * 0.5f;

        for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
            if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED &&
                c->patch_system_state_.patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;

            float ox = (c->patch_system_state_.patches_[i].grid_x + 0.5f) * PATCH_EXTENT;
            float oz = (c->patch_system_state_.patches_[i].grid_z + 0.5f) * PATCH_EXTENT;

            GPUPatchInstance inst{};
            inst.origin[0] = ox;
            inst.origin[1] = oz;
            inst.extent = PATCH_EXTENT;
            inst.layer = c->patch_system_state_.patches_[i].layer;

            float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);

            // Finite mode: all patches visible (walls define boundary, not fog)
            if (c->world_state_.finite_mode || d2 <= VISIBILITY_CYLINDER_RADIUS_SQ) {
                if (d2 <= LOD0_CYLINDER_RADIUS_SQ) {
                    lod0[lod0Count++] = inst;
                }
                else {
                    lod1[lod1Count++] = inst;
                }
            }
            else {
                pregen[pregenCount++] = inst;
            }
        }

        // Pack: LOD-0, then LOD-1, then pregen
        uint32_t w = 0;
        std::memcpy(instances + w, lod0, lod0Count * sizeof(GPUPatchInstance)); w += lod0Count;
        std::memcpy(instances + w, lod1, lod1Count * sizeof(GPUPatchInstance)); w += lod1Count;
        std::memcpy(instances + w, pregen, pregenCount * sizeof(GPUPatchInstance)); w += pregenCount;

        c->gpuState_.upload_patch_instances(queue, instances, w);
        c->world_state_.lod0_patch_count = lod0Count;
        c->world_state_.render_patch_count = lod0Count + lod1Count;
        c->world_state_.all_patch_count = w;

        // Sync placement_patch_count so compute_entity_placement
        // can sample heightfields from the current frame's patch set.
        c->gpuState_.config().placement_patch_count = w;
        c->gpuState_.upload_placement_patch_count(queue);

        c->gpuState_.config().lod_pawn_x = pawn_wx;
        c->gpuState_.config().lod_pawn_z = pawn_wz;
        c->gpuState_.upload_lod_pawn(queue);

        // ─── Patch grid: O(1) spatial index for sample_terrain_y_at ────────
        {
            GPUPatchGrid grid{};
            grid.side = Dim::PATCH_PREGEN_SIDE;
            grid.cell_extent = PATCH_EXTENT;

            int32_t min_gx = INT32_MAX;
            int32_t min_gz = INT32_MAX;
            for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
                if (!c->patch_system_state_.patches_[i].valid) continue;
                if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED &&
                    c->patch_system_state_.patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;
                min_gx = std::min(min_gx, c->patch_system_state_.patches_[i].grid_x);
                min_gz = std::min(min_gz, c->patch_system_state_.patches_[i].grid_z);
            }
            if (min_gx == INT32_MAX) { min_gx = 0; min_gz = 0; }
            grid.origin_x = min_gx;
            grid.origin_z = min_gz;

            for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
                if (!c->patch_system_state_.patches_[i].valid) continue;
                if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED &&
                    c->patch_system_state_.patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;
                int32_t lx = c->patch_system_state_.patches_[i].grid_x - grid.origin_x;
                int32_t lz = c->patch_system_state_.patches_[i].grid_z - grid.origin_z;
                if (lx < 0 || lz < 0 ||
                    lx >= int32_t(grid.side) || lz >= int32_t(grid.side)) continue;
                grid.entries[lz * grid.side + lx] = c->patch_system_state_.patches_[i].layer + 1u;
            }

            c->gpuState_.upload_patch_grid(queue, grid);
        }
    }
    // GPU Y-correction is additive (ground_y += terrain), so ground
    // entries must be re-uploaded with offset-only values whenever
    // the heightfield changes. Tie groundEntriesDirty to patch changes.
    c->world_state_.ground_entries_dirty = c->world_state_.ground_entries_dirty || c->world_state_.patch_instances_dirty;
    c->world_state_.placement_dirty = c->world_state_.placement_dirty || c->world_state_.patch_instances_dirty;
    c->world_state_.patch_instances_dirty = false;

    // ─── Entity distance culling ─────────────────────────────
    c->world_state_.entities_culled = update_entity_draw_visibility(c, queue);

    // ─── Deferred uploads (one per frame max) ────────────────
    if (tileGridDirty) upload_tile_grid_now(c->tile_world_state_, c, queue, c->world_state_.last_center_x, c->world_state_.last_center_z);
    flush_pier_count(c, queue);

    audit_entity_integrity(c);

    // Restore radius if we capped it for finite mode
    if (c->world_state_.finite_mode) { c->world_state_.active_radius = savedRadius; }
}

} // namespace the_board
} // namespace t7
