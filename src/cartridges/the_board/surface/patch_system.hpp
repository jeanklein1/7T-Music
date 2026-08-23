#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/surface_services.hpp"  // the surface's decl tier (this module's own contract)
#include <cmath>          // std::floor, std::sqrt, std::abs   // (impl, merged)
#include <algorithm>      // std::min, std::max   // (impl, merged)
#include <cstring>        // std::memcpy (instance banding)   // (impl, merged)
#include <unordered_set>  // the O(1) existence scan (continuous allocation)   // (impl, merged)
#include <iostream>       // DIAG blocks (lifecycle audit + evict trace)   // (impl, merged)
#include <cstdio>         // std::fprintf — RIBBON_5's conservation witness (always-on, every build)
#include "core/instruments.hpp"   // RIBBON_5 — INSTRUMENTS.frame_meter gates the [STREAM] line

// ─── patch_system.hpp (S2 · MERGED: the active-patch machine) ──────
// The decl tier lives in contracts/surface_services.hpp; this file is the
// machine's bodies whole — the registry lifecycle (allocate → spawn →
// generate → evict), the frame budgets, world teardown, the layer
// allocator, and the streaming conductor. Stands on THE MACHINE FACE
// (the root organs are machine members), the S3 dispatch seam
// (select/place/commit — contracts/spawn_services.hpp), and the GPU
// wire (c->gpuState_ / c->renderer_). The reaches OUTSIDE the face
// ride the call site (the B law): the WRITABLE tile-cache + theme
// organs (the face's views are const — the lifecycle owner mutates
// them through the owner doors), the tile doors' deps, the mood deps
// (the back-portal door), and the driver's intent organ (the movement
// budget read). COHORT: the tail's last — after the machine natives
// (spawn service defs) and mood (the back-portal door's def).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {


// ── The patch registry ─────────────────────────────────────────────

inline ActivePatch* find_patch(MachineCtx* c, int32_t gx, int32_t gz) {
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (c->patch_system_state_.patches_[i].valid && c->patch_system_state_.patches_[i].grid_x == gx && c->patch_system_state_.patches_[i].grid_z == gz)
            return &c->patch_system_state_.patches_[i];
    }
    return nullptr;
}

// Hook: full eviction of a single patch.
// OIL_1 U8/U9 — THE RAISES THIS PRIMITIVE OWES BUT DOES NOT MAKE:
// removing a patch changes the grid set (patch_instances_dirty) and
// frees a layer (alloc_scan_pending). Both raises live in this
// function's ONE caller, the eviction block in stream_patches, which
// batches them after its compaction. A second call site would have to
// carry them itself.
inline void evict_patch(MachineCtx* c, uint32_t pi, wgpu::Queue& queue) {
    free_layer(c, c->patch_system_state_.patches_[pi].layer);
    // Painting eviction now handled by evict_gallery (bodies/gallery.hpp) via entity_refs
    evict_patch_entities(c, c->patch_system_state_.patches_[pi], queue);
    c->patch_system_state_.patches_[pi].valid = false;
}

inline void evict_patch_entities(MachineCtx* c, ActivePatch& patch, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < patch.entity_ref_count; i++) {
        auto& ref = patch.entity_refs[i];
        FAMILY_DISPATCH[ref.family].evict_slot(c, ref.slot, queue);
    }

    patch.entity_ref_count = 0;
}

// ── Dynamic budgets ────────────────────────────────────────────────

// RIBBON_4: NEEDS_REGEN alone. A SPAWNED patch is no longer this budget's
// business — the sliced conductor owns it, one slice a frame — and counting
// it here would inflate the regen burst with work that is already paced.
inline uint32_t count_pending_patches(MachineCtx* c, bool include_spawned = false) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        const PatchPhase ph = c->patch_system_state_.patches_[i].phase;
        if (ph == PatchPhase::NEEDS_REGEN) n++;
        else if (include_spawned && ph == PatchPhase::SPAWNED) n++;
    }
    return n;
}

// A YOUNG WORLD IS A TRANSITION (RIBBON_5). The steady cadence — one spawn,
// one sliced bake — is for a world that is WHOLE. While the window is mostly
// unbuilt (a rebirth, or the healing migration after one, when the point
// mirror still echoed the dead world), the conductor runs at TRANSITION pace:
// the old budgets, whole bakes, the backlog ladder. RIBBON_4 narrowed the
// steady channels and, by narrowing them, narrowed the rebuild too — a world
// that used to heal in a second crawled for a minute, and Jean walked through
// the holes. The burst serves an EMPTY window; the slice serves a MOVING one.
//
// The predicate is the registry's own arithmetic — no new state, nothing to
// keep in sync, and it clears itself the moment the window is half built.
inline bool world_is_young(MachineCtx* c) {
    uint32_t built = 0;
    uint32_t cells = 2u * c->world_state_.active_radius + 1u;
    cells *= cells;
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        if (c->patch_system_state_.patches_[i].phase == PatchPhase::GENERATED ||
            c->patch_system_state_.patches_[i].phase == PatchPhase::NEEDS_REGEN) built++;
    }
    return built * 2u < cells;   // under half the window built = young
}

inline uint32_t patches_budget_this_frame(MachineCtx* c, const InputState& inputState, bool young = false) {
    // RIBBON_5: while young the ladder counts BOTH pending phases, because the
    // burst arm serves both — SPAWNED is the rebuild, and there is no sliced
    // conductor running beside it to own that phase.
    uint32_t pending = count_pending_patches(c, young);
    uint32_t budget = PATCH_BUDGET_MIN;
    if (pending >= PATCH_PENDING_TIER_4) budget = 6;
    else if (pending >= PATCH_PENDING_TIER_3) budget = 4;
    else if (pending >= PATCH_PENDING_TIER_2) budget = 3;
    else if (pending >= PATCH_PENDING_TIER_1) budget = 2;

    bool moving = (std::abs(inputState.move_x) > 0.01f ||
        std::abs(inputState.move_z) > 0.01f);
    if (moving && pending > PATCH_BUDGET_MOVE_THRESHOLD)
        budget += 1;

    return std::min(budget, PATCH_BUDGET_MAX);
}

// ── World lifecycle ────────────────────────────────────────────────
//
// Root-called owner verb. CALLER: the transition machine
// (root); OWNER: patch_system. The bulk
// sweep over sibling organs dissolved into per-owner teardown verbs
// called by the score's TEARDOWN movement; this core keeps the
// surface's own concerns — patches, tiles, themes, dispatch queues,
// footprints — plus the world-rebirth GPU staging lines.
// ── The recenter door (deps-form) ─────────────────────────────────
inline void request_recenter(WorldState& ws) {
    ws.last_center_x = INT32_MAX;
    ws.last_center_z = INT32_MAX;
}

// THE ONE SURFACE RESET. Called from BOTH paths — boot and the transition
// machine — because boot is a transition from nothing (LAWS L10). Every line
// below was already true at boot, but by in-struct default rather than by
// call: the equality was asserted by luck and would have parted silently the
// day any default moved. Now it is enforced by call.
inline void reset_surface(MachineCtx* c, wgpu::Queue& queue,
    TileWorldState& tile_world_state, ThemesState& themes_state) {
    // Patches + tile cache
    init_patch_system(c, tile_world_state);
    c->world_state_.last_center_x = INT32_MAX;  // force full regen on next frame
    c->world_state_.last_center_z = INT32_MAX;

    // Terrain tokens — through the owner's door
    reset_terrain_memory(tile_world_state);

    c->spawn_engine_state_.entityQueueCount_ = 0;
    c->spawn_engine_state_.placementCount_ = 0;

    // Theme envelope — through the owner's door
    reset_theme_envelope(themes_state);

    // Footprints
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        c->spawn_engine_state_.footprints_[i] = GroundFootprint{};
    }

    // Indoor shell
    c->gpuState_.set_shell_index_count(0);

    // Lights need re-upload with potentially new config
    c->mood_state_.lights_dirty = true;

    // New world decides its own upload frequency policy
    c->gpuState_.set_config_dynamic(false);
}

// The regen fan-out over the registry (pyramids bake into the
// heightfield; their post-commit is the caller).
inline void mark_patches_for_regen(MachineCtx* c, float min_wx, float min_wz,
    float max_wx, float max_wz,
    int32_t home_gx, int32_t home_gz) {
    int32_t pg_x0 = (int32_t)std::floor(min_wx / Dim::PATCH_EXTENT);
    int32_t pg_x1 = (int32_t)std::floor(max_wx / Dim::PATCH_EXTENT);
    int32_t pg_z0 = (int32_t)std::floor(min_wz / Dim::PATCH_EXTENT);
    int32_t pg_z1 = (int32_t)std::floor(max_wz / Dim::PATCH_EXTENT);

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

inline void init_patch_system(MachineCtx* c, TileWorldState& tile_world_state) {
    for (uint32_t i = 0; i < Dim::MAX_ACTIVE_PATCHES; i++) {
        c->patch_system_state_.freeLayerStack_[i] = Dim::MAX_ACTIVE_PATCHES - 1 - i;
    }
    c->world_state_.free_layer_count = Dim::MAX_ACTIVE_PATCHES;
    c->world_state_.active_patch_count = 0;
    c->world_state_.render_patch_count = 0;
    c->world_state_.lod0_patch_count = 0;
    c->world_state_.all_patch_count = 0;
    c->gpuState_.stage_placement_patch_count(0);
    reset_tile_cache(tile_world_state);  // owner door
    c->world_state_.ground_entries_dirty = true;
    c->world_state_.patch_instances_dirty = true;
    c->world_state_.placement_dirty = true;
    c->world_state_.alloc_scan_pending = true;   // OIL_1 U9 — a fresh pool is all demand
}

// ── Patch generation ───────────────────────────────────────────────

// The params ride a DYNAMIC-OFFSET uniform seat: one write per patch into
// the ring before any pass is recorded, then one offset per dispatch.
// `ringCursor` is stream_patches' per-frame cursor — a full regen makes two
// batches into one encoder, and the queue orders both writes before the
// command buffer, so the second must not land on the first's records.
// THE PER-PATCH PASS PAIR STAYS: `patch_height_scratch` is ONE patch's
// worth, so heights must reach gradients before the next patch overwrites
// them — H_i → G_i → C_i is Table E's RAW contract.
inline void generate_patch_batch(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count, uint32_t& ringCursor) {
    if (count == 0) return;
    const uint32_t base = ringCursor;
    count = c->gpuState_.upload_patch_params(queue, params, count, base);
    if (count == 0) return;
    ringCursor = base + count;

    for (uint32_t i = 0; i < count; i++) {
        const uint32_t paramsOffset = (base + i) * Dim::UNIFORM_DYNAMIC_STRIDE;
        // Pass 1: heights only (one ground_formed_with_complexity per texel)
        {
            wgpu::ComputePassDescriptor cpd{};
            cpd.label = "Patch Heights (pass 1)";
            cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::StreamPatches);
            wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
            // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
            { cp.SetBindGroup(0, c->gpuState_.world_group());
              cp.SetBindGroup(1, c->gpuState_.frame_c_group()); }
            c->renderer_.dispatch_generate_patch_heights(cp, paramsOffset, c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(), GPUState::patch_heightfield_workgroups());
            cp.End();
        }

        // Pass 2: gradients from neighbor reads + complexity + cell colors
        {
            wgpu::ComputePassDescriptor cpd{};
            cpd.label = "Patch Gradients + Cells (pass 2)";
            cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::StreamPatches);
            wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
            // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
            { cp.SetBindGroup(0, c->gpuState_.world_group());
              cp.SetBindGroup(1, c->gpuState_.frame_c_group()); }
            c->renderer_.dispatch_generate_patch_gradients(cp, paramsOffset, c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(), GPUState::patch_heightfield_workgroups());
            c->renderer_.dispatch_generate_patch_cells(cp, paramsOffset, c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(), GPUState::patch_cell_workgroups());
            cp.End();
        }
    }
}

// ── THE SLICED BAKE (RIBBON_4) ─────────────────────────────────────
//
// One patch's heights, one row band, one frame. The whole-bake pair
// (generate_patch_batch) stays exactly as it was and still serves the
// urgent case, the boot burst and the regen burst; this is the steady
// state's own encoder.
//
// WHY A BAND AND NOT A PATCH. A bake is 65 536 texels of
// ground_formed_with_complexity plus gradients plus cells. At 50 wu a
// patch and a 15x15 window, every 50 wu of flight — 1.25 s at cruise,
// 0.3 s at the top of the dial — demands a whole ROW of patches. The old
// conductor answered a deep backlog by working harder per frame, which is
// precisely when the rider is moving fastest. Cutting the heights pass
// into bands spreads one patch's cost over PATCH_BAKE_SLICES frames and
// makes the worst frame the same as the best one.
//
// WHY GRADIENTS AND CELLS ARE NOT SLICED. They are neighbour reads over
// patch_height_scratch, which holds ONE patch — so they must run after
// that patch's last height band and before any other patch's first. They
// are cheap beside the heights pass, and keeping them whole is what makes
// "one bake in flight" the only interlock this needs.
inline void generate_patch_heights_band(MachineCtx* c, wgpu::CommandEncoder& encoder,
    wgpu::Queue& queue, const GPUPatchParams& params, uint32_t& ringCursor,
    uint32_t rowGroups, bool finish) {
    const uint32_t base = ringCursor;
    // ONE RECORD PER SLICE. The slices of a bake differ in row0, so each
    // needs its own record: the ring's writes all land before the command
    // buffer, and a rewritten record would hand every band the last band's
    // row0. The ring is MAX_ACTIVE_PATCHES deep and this frame spends at
    // most one record, so the cursor has room the batch path already needs.
    if (c->gpuState_.upload_patch_params(queue, &params, 1, base) == 0) return;
    ringCursor = base + 1;
    const uint32_t paramsOffset = base * Dim::UNIFORM_DYNAMIC_STRIDE;

    {
        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Patch Heights (sliced)";
        cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::StreamPatches);
        wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
        { cp.SetBindGroup(0, c->gpuState_.world_group());
          cp.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_generate_patch_heights(cp, paramsOffset,
            c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(),
            GPUState::patch_heightfield_workgroups(), rowGroups);
        cp.End();
    }
    if (!finish) return;

    // The last band: gradients + cells over the scratch this bake just
    // filled, same pass shape as generate_patch_batch's pass 2.
    {
        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Patch Gradients + Cells (sliced)";
        cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::StreamPatches);
        wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
        { cp.SetBindGroup(0, c->gpuState_.world_group());
          cp.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_generate_patch_gradients(cp, paramsOffset, c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(), GPUState::patch_heightfield_workgroups());
        c->renderer_.dispatch_generate_patch_cells(cp, paramsOffset, c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(), GPUState::patch_cell_workgroups());
        cp.End();
    }
}

inline GPUPatchParams make_patch_params(MachineCtx* c, int32_t gx, int32_t gz, uint32_t layer) {
    GPUPatchParams p{};
    p.origin[0] = (gx + 0.5f) * Dim::PATCH_EXTENT;
    p.origin[1] = (gz + 0.5f) * Dim::PATCH_EXTENT;
    p.extent = Dim::PATCH_EXTENT;
    p.resolution = Dim::PATCH_HEIGHTFIELD_N;  // Q9: one source of truth (was a 256 literal decoupled from the Dim const that sizes the write texture + the dispatch divisor)
    p.master_seed = c->world_state_.active_seed;
    p.time = 0.0f;
    p.layer = layer;
    p.row0 = 0u;      // RIBBON_4 — a whole bake; the sliced path overwrites it per band
    return p;
}

// ── Layer allocator ────────────────────────────────────────────────

inline uint32_t alloc_layer(MachineCtx* c) {
    if (c->world_state_.free_layer_count == 0) {
        // CONSERVATION BREAK (RIBBON_5): with MAX_ACTIVE_PATCHES equal to the
        // full 15x15 window there is zero headroom by design, and a caller
        // that reaches an empty pool is a caller the guards above failed.
        // Recycling layer 0 silently is how one mutating patch could chase a
        // player across an empty world — every comer writing the same
        // heightfield layer, endlessly re-baked. Loud, always-on, rate-limited.
        static uint32_t s_starved = 0;
        if ((s_starved++ & 63u) == 0u)
            std::fprintf(stderr, "[Patch] LAYER POOL EMPTY — alloc refused "
                "(active=%u free=%u). A layer leaked or headroom is zero.\n",
                c->world_state_.active_patch_count, c->world_state_.free_layer_count);
        return UINT32_MAX;   // refuse: no patch on a stolen layer
    }
    return c->patch_system_state_.freeLayerStack_[--c->world_state_.free_layer_count];
}

inline void free_layer(MachineCtx* c, uint32_t layer) {
    // CONSERVATION (RIBBON_5), the other end: a layer returned twice, or one
    // returned that was never taken, overflows the stack and corrupts whatever
    // follows it. The pool cannot hold more than it owns.
    if (c->world_state_.free_layer_count >= Dim::MAX_ACTIVE_PATCHES ||
        layer >= Dim::MAX_ACTIVE_PATCHES) {
        std::fprintf(stderr, "[Patch] LAYER POOL OVERFLOW — free(%u) refused "
            "(free=%u of %u). A layer was returned twice.\n",
            layer, c->world_state_.free_layer_count, Dim::MAX_ACTIVE_PATCHES);
        return;
    }
    c->patch_system_state_.freeLayerStack_[c->world_state_.free_layer_count++] = layer;
}

// Check if grid coordinate is within the allocation window (world_state_.active_radius = Dim::PATCH_PREGEN_RADIUS)
inline bool in_render_window(MachineCtx* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
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
inline uint32_t collect_sorted_patches(MachineCtx* c, PatchCandidate* out,
    float point_wx, float point_wz, Pred&& pred, bool nearest_first)
{
    float half = Dim::PATCH_EXTENT * 0.5f;
    uint32_t count = 0;
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        if (!pred(c->patch_system_state_.patches_[i])) continue;
        float ox = (c->patch_system_state_.patches_[i].grid_x + 0.5f) * Dim::PATCH_EXTENT;
        float oz = (c->patch_system_state_.patches_[i].grid_z + 0.5f) * Dim::PATCH_EXTENT;
        float d2 = patch_distance_sq(point_wx, point_wz, ox, oz, half);
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

// Check if grid coordinate is within the priority window (Dim::PATCH_GRID_RADIUS)
inline bool in_priority_window(MachineCtx* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
    int32_t r = (int32_t)Dim::PATCH_GRID_RADIUS;
    return gx >= cx - r && gx <= cx + r &&
        gz >= cz - r && gz <= cz + r;
}

// Process entity spawn for pre-collected patch candidates.
inline void spawn_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::Queue& queue,
    ThemesState& themes_state) {
    for (uint32_t s = 0; s < count; s++) {
        uint32_t pi = candidates[s].idx;
        evaluate_theme_envelope(themes_state, c,
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
        // Two-tip registration through the owner's door: the
        // ref-count protocol lives whole in bodies/ribbon.hpp now.
        ribbon_register_tips_at(c->ribbon_state_, c->patch_system_state_.patches_[pi], gx, gz);
    }
}

// Process heightfield generation for pre-collected patch candidates.
inline void generate_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    bool& tileGridDirty,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps,
    uint32_t& ringCursor) {
    if (count == 0) return;
    if (tileGridDirty) {
        upload_tile_grid_now(tile_world_state, &tile_world_deps, queue, c->world_state_.last_center_x, c->world_state_.last_center_z);
        tileGridDirty = false;
    }
    GPUPatchParams batchParams[Dim::MAX_ACTIVE_PATCHES];
    uint32_t batchIdx[Dim::MAX_ACTIVE_PATCHES];
    for (uint32_t i = 0; i < count; i++) {
        uint32_t pi = candidates[i].idx;
        batchParams[i] = make_patch_params(c, 
            c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z, c->patch_system_state_.patches_[pi].layer);
        batchIdx[i] = pi;
    }
    generate_patch_batch(c, encoder, queue, batchParams, count, ringCursor);
    for (uint32_t b = 0; b < count; b++) {
        uint32_t pi = batchIdx[b];
        c->patch_system_state_.patches_[pi].phase = PatchPhase::GENERATED;
    }
    c->world_state_.patch_instances_dirty = true;
}

// THE BAND ARITHMETIC, in WORKGROUP ROWS and not texel rows — because the
// dispatch is the thing that must tile the patch exactly, and a band that
// starts mid-workgroup cannot exist. rowGroups is the ceiling, so the last
// band is the short one and every texel row is covered exactly once for ANY
// slice count. The obvious formula (row0 = slice × N / slices, dispatch
// workgroups / slices) is only exact when the slice count divides 16, and
// PATCH_BAKE_SLICES is a one-word ruling: at 3 it would leave five unwritten
// texel rows per band, which is a band of garbage terrain, silently.
struct PatchBandPlan {
    uint32_t row_groups;   // workgroup rows per band
    uint32_t bands;        // how many bands actually result
};
inline PatchBandPlan patch_band_plan(uint32_t slices) {
    const uint32_t wg = GPUState::patch_heightfield_workgroups();
    const uint32_t s = (slices == 0u) ? 1u : (slices < wg ? slices : wg);
    const uint32_t rg = (wg + s - 1u) / s;          // ceiling
    return { rg, (wg + rg - 1u) / rg };             // ceiling again: the real count
}
// Workgroup rows this band dispatches (the last one is short), and where its
// first texel row sits. WORKGROUP_SIZE is 16 in generate_patch_heights.
inline uint32_t patch_band_rows(const PatchBandPlan& plan, uint32_t band) {
    const uint32_t wg = GPUState::patch_heightfield_workgroups();
    const uint32_t start = band * plan.row_groups;
    return (start >= wg) ? 0u : std::min(plan.row_groups, wg - start);
}
inline uint32_t patch_band_row0(const PatchBandPlan& plan, uint32_t band) {
    return band * plan.row_groups * 16u;
}

// ── THE STEADY WORLD's conductor (RIBBON_4) ────────────────────────
//
// One bake in flight, one slice a frame. A patch within
// PATCH_URGENT_MARGIN × PATCH_EXTENT of lod0_radius bakes WHOLE this
// frame — a hole is worse than a hitch; a leisurely one takes
// PATCH_BAKE_SLICES frames for its heights, then gradients + cells the
// frame after. The scratch holds one patch, which is why there is one.
//
// THE LAW IS STATE, not a queue: the BAKING phase on the patch itself is
// the whole interlock. Eviction needs no special case (see the eviction
// block: BAKING is treated as SPAWNED — the layer is freed and there is
// nothing to finish), and a mid-bake mood transition resets the registry
// like any other.
inline void generate_patch_sliced(MachineCtx* c, wgpu::CommandEncoder& encoder,
    wgpu::Queue& queue, bool& tileGridDirty,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps,
    uint32_t& ringCursor, float gen_cx, float gen_cz) {

    // (1) A bake is in flight: carry it one band and no further.
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        auto& p = c->patch_system_state_.patches_[i];
        if (!p.valid || p.phase != PatchPhase::BAKING) continue;

        const PatchBandPlan plan = patch_band_plan(p.bake_slices);
        GPUPatchParams params = make_patch_params(c, p.grid_x, p.grid_z, p.layer);
        params.row0 = patch_band_row0(plan, p.bake_slice);
        const bool last = (uint32_t(p.bake_slice) + 1u >= plan.bands);
        generate_patch_heights_band(c, encoder, queue, params, ringCursor,
                                    patch_band_rows(plan, p.bake_slice), last);
        p.bake_slice++;
        if (last) {
            p.phase = PatchPhase::GENERATED;
            p.bake_slice = 0;
            c->world_state_.patch_instances_dirty = true;
        }
        return;   // one slice a frame, and it was this one
    }

    // (2) Nothing in flight: take the nearest SPAWNED patch, from the point
    //     the look-ahead names.
    PatchCandidate cands[Dim::MAX_ACTIVE_PATCHES];
    uint32_t count = collect_sorted_patches(c, cands, gen_cx, gen_cz,
        [](const ActivePatch& p) { return p.phase == PatchPhase::SPAWNED; }, true);
    if (count == 0) return;

    const uint32_t pi = cands[0].idx;
    auto& p = c->patch_system_state_.patches_[pi];

    // The tile grid must be current before ANY bake reads it — the whole
    // path's precondition, lifted from generate_selected_patches.
    if (tileGridDirty) {
        upload_tile_grid_now(tile_world_state, &tile_world_deps, queue,
                             c->world_state_.last_center_x, c->world_state_.last_center_z);
        tileGridDirty = false;
    }

    // URGENCY. Inside lod0 plus a patch of margin the eye is about to be
    // there, so it bakes whole this frame through the untouched batch path.
    const float urgent = c->gpuState_.lod0_radius() + PATCH_URGENT_MARGIN * Dim::PATCH_EXTENT;
    if (cands[0].dist2 <= urgent * urgent) {
        GPUPatchParams whole = make_patch_params(c, p.grid_x, p.grid_z, p.layer);
        generate_patch_batch(c, encoder, queue, &whole, 1, ringCursor);
        p.phase = PatchPhase::GENERATED;
        c->world_state_.patch_instances_dirty = true;
        return;
    }

    // Leisurely: open the bake and encode its first band now.
    const PatchBandPlan plan = patch_band_plan(PATCH_BAKE_SLICES);
    p.bake_slices = uint8_t(plan.bands);
    p.bake_slice = 0;
    p.phase = PatchPhase::BAKING;
    const bool single = (plan.bands == 1u);
    GPUPatchParams params = make_patch_params(c, p.grid_x, p.grid_z, p.layer);
    params.row0 = patch_band_row0(plan, 0u);
    generate_patch_heights_band(c, encoder, queue, params, ringCursor,
                                patch_band_rows(plan, 0u), /*finish*/ single);
    p.bake_slice = 1;
    if (single) {
        p.phase = PatchPhase::GENERATED;
        p.bake_slice = 0;
        c->world_state_.patch_instances_dirty = true;
    }
}

// ── band_patches (visibility/LOD → the draw-instance set) ──────────
// One unit of the conductor: walk the GENERATED patches, gate each on the
// world-space visibility cylinder (finite mode = all visible, "walls
// define boundary, not fog"), split LOD0/LOD1/pregen, pack them
// [lod0|lod1|pregen] (the draw-split contract the render reads by count),
// upload the instance buffer + the lod0/render/all counts + the placement
// patch count, and push lod_point so the frustum-cull shader re-bands on
// the SAME point the CPU banded on (the anti-flicker contract).
//   offer-face: GPUPatchInstance[] + lod0/render/all counts + lod_point.
//   requires:   patches_ registry, the point readback, patch_distance_sq,
//               the LIVE veil chain (config veil_ring/lod0_radius),
//               finite_mode; the upload doors.
// Bit-safe: pack order = wire layout, not a draw. Separate from
// build_patch_grid (different consumer/offer-face); they only coincided
// in the same tail block.
inline void band_patches(MachineCtx* c, wgpu::Queue& queue) {
    // OIL_1 U10 (ledger: R3 band_patches, C1): the four arrays carry no
    // value-init — ~14.4 KB of per-frame stack zeroing retired. Safe by
    // the prefix proof: each band array is written [0, count) before the
    // memcpy reads [0, count), instances is written [0, w) by the three
    // memcpys before the upload reads [0, w), and every GPUPatchInstance
    // field is assigned at the pack site (inst carries its own init).
    GPUPatchInstance instances[Dim::MAX_ACTIVE_PATCHES];
    uint32_t lod0Count = 0;
    uint32_t lod1Count = 0;
    uint32_t pregenCount = 0;

    // Temporary arrays for each band
    GPUPatchInstance lod0[Dim::MAX_ACTIVE_PATCHES];
    GPUPatchInstance lod1[Dim::MAX_ACTIVE_PATCHES];
    GPUPatchInstance pregen[Dim::MAX_ACTIVE_PATCHES];

    float point_wx = c->point_.x;   // THE POINT (1-frame stale by law E-4)
    float point_wz = c->point_.z;
    float half = Dim::PATCH_EXTENT * 0.5f;

    // THE VEIL CHAIN, live: THE RING is the draw authority (nothing draws
    // beyond it); lod0 is the full/half-mesh split. The same config values
    // the GPU gate + the fragment icing read — one yardstick everywhere.
    const float ring_sq = c->gpuState_.veil_ring()   * c->gpuState_.veil_ring();
    const float lod0_sq = c->gpuState_.lod0_radius() * c->gpuState_.lod0_radius();

    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED &&
            c->patch_system_state_.patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;

        float ox = (c->patch_system_state_.patches_[i].grid_x + 0.5f) * Dim::PATCH_EXTENT;
        float oz = (c->patch_system_state_.patches_[i].grid_z + 0.5f) * Dim::PATCH_EXTENT;

        GPUPatchInstance inst{};
        inst.origin[0] = ox;
        inst.origin[1] = oz;
        inst.extent = Dim::PATCH_EXTENT;
        inst.layer = c->patch_system_state_.patches_[i].layer;

        float d2 = patch_distance_sq(point_wx, point_wz, ox, oz, half);

        // Finite mode: all patches visible (walls define boundary, not fog)
        if (c->world_state_.finite_mode || d2 <= ring_sq) {
            if (d2 <= lod0_sq) {
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
    c->gpuState_.stage_placement_patch_count(w);
    c->gpuState_.upload_placement_patch_count(queue);

    c->gpuState_.stage_lod_point(point_wx, point_wz);
    c->gpuState_.upload_lod_point(queue);
}

// ── build_patch_grid (the (gx,gz)→layer index the baked sampler reads) ──
// The OTHER conductor-tail unit: build the GPUPatchGrid — the O(1) spatial
// index (origin + side + per-cell layer+1, 0=empty) that
// sample_terrain_y_at hashes into to find a patch's heightfield layer. Its
// OWN walk over the GENERATED patches; a separate consumer and offer-face
// from band_patches — they only coincided in the same tail block, never
// shared the walk.
//   offer-face: GPUPatchGrid (origin_x/z, side, entries[]) uploaded.
//   requires:   patches_ registry (grid_x/grid_z/layer/valid/phase),
//               PATCH_PREGEN_SIDE, Dim::PATCH_EXTENT; upload_patch_grid.
// Bit-safe: index layout = wire layout, not a draw. entries = layer+1
// (0 = empty) is the frozen convention the WGSL sampler decodes.
inline void build_patch_grid(MachineCtx* c, wgpu::Queue& queue) {
    GPUPatchGrid grid{};
    grid.side = Dim::PATCH_PREGEN_SIDE;
    grid.cell_extent = Dim::PATCH_EXTENT;

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

// ── build_active_patch_set (the (gx,gz) existence set — O(1) lookup) ────
// Rebuilt from the registry each call — a per-scan LOCAL, NOT a maintained
// member (evict/compaction keep patches_ correct; this reads it fresh). It
// enumerates ALL active-count entries (no .valid filter — matching the raw
// scans it replaces, so existence is bit-identical). Shared by the alloc
// window scan AND the fullRegen re-roll (killing that inner scan's O(N^2)).
// find_patch stays the O(N) single-lookup handle — a persistent index for
// one lookup isn't worth an alloc/evict/compaction invariant. Bit-safe:
// existence, not value.
inline std::unordered_set<GridKey, GridKeyHash> build_active_patch_set(MachineCtx* c) {
    std::unordered_set<GridKey, GridKeyHash> set;
    set.reserve(c->world_state_.active_patch_count);
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        set.insert({ c->patch_system_state_.patches_[i].grid_x, c->patch_system_state_.patches_[i].grid_z });
    }
    return set;
}

// --- Patch streaming: determine active 7×7 grid, generate new patches ---
// THE CONDUCTOR: the per-frame step — recenter,
// eviction, allocation, spawn + generation budgets, visibility
// banding, deferred uploads.
// SEAM[patch:spawn-trigger] the S3-trigger calls are the declared
//   seam face: select_entities_for_patch / place_entity_queue /
//   commit_entity_queue (via spawn_selected_patches), plus
//   update_entity_draw_visibility at the frame tail — the surface
//   machine waking the occupier machine.
inline void stream_patches(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    TileWorldState& tile_world_state, ThemesState& themes_state,
    TileWorldDeps& tile_world_deps, MoodDeps& mood_deps, const InputState& inputState) {
    // ─── Patch Generation Pipeline ─────────────────────────────────

    int32_t centerX, centerZ;
    // THE PARAMS RING'S CURSOR, one frame's worth. A full regen generates
    // two batches into this one encoder, and the queue orders every
    // WriteBuffer before the whole command buffer — so the second batch
    // takes fresh records rather than overwriting the first's.
    uint32_t patchParamsCursor = 0;
    bool tileGridDirty = false;        // coalesce tile grid uploads to one per frame
    if (c->world_state_.finite_mode) {
        centerX = 0;
        centerZ = 0;
    }
    else {
        centerX = (int32_t)std::floor(c->point_.x / Dim::PATCH_EXTENT);
        centerZ = (int32_t)std::floor(c->point_.z / Dim::PATCH_EXTENT);
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
        c->world_state_.alloc_scan_pending = true;   // OIL_1 U9 — the window moved: new cells may be empty

        bool fullRegen = (oldCX == INT32_MAX);  // first frame

        // Lightweight cache maintenance (no GPU buffer writes). CONTAINMENT
        // (FIX_TILE_PATCH_CONTAINMENT): spare every tile a live patch stands on.
        // build_active_patch_set enumerates the registry with no .valid filter
        // and runs BEFORE the budgeted patch-eviction pass below, so it includes
        // patches already out of the render window but not yet drained -- exactly
        // the ones a centre jump would orphan.
        auto liveTiles = build_active_patch_set(c);
        evict_distant_tiles(tile_world_state, centerX, centerZ,
            [&](const GridKey& k) { return liveTiles.count(k) > 0; });

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
                    ensure_tile(tile_world_state, &tile_world_deps, gx, gz);  // owner door
                }
            }

            // NOW spawn portals — tile cache is populated, terrain heights are correct
            if (c->mood_state_.back_portal_pending) {
                force_spawn_back_portal(&mood_deps, queue, *c);
            }
            // Built ONCE before the window loop: bit-identical to the old
            // per-cell O(N) scan (each cell is unique, so a patch added at an
            // earlier cell never matches a later one). Kills the O(N^2).
            auto existingPatches = build_active_patch_set(c);
            for (int32_t gz = centerZ - rr; gz <= centerZ + rr; gz++) {
                for (int32_t gx = centerX - rr; gx <= centerX + rr; gx++) {
                    bool found = existingPatches.count({ gx, gz }) > 0;
                    if (!found && c->world_state_.free_layer_count > 0) {
                        // RIBBON_5: the array bound, stated not derived (see the
                        // continuous loop for why). This loop's own guard is
                        // per-iteration already, so the pool cannot empty under it.
                        if (c->world_state_.active_patch_count >= Dim::MAX_ACTIVE_PATCHES) continue;
                        uint32_t layer = alloc_layer(c);
                        if (layer == UINT32_MAX) continue;   // the pool refused
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
            PatchCandidate spawnCands[Dim::MAX_ACTIVE_PATCHES];
            uint32_t spawnCount = collect_sorted_patches(c, spawnCands,
                c->point_.x, c->point_.z,
                [&](const ActivePatch& p) {
                    return p.phase == PatchPhase::ALLOCATED &&
                        in_priority_window(c, p.grid_x, p.grid_z, centerX, centerZ);
                }, true);
            spawn_selected_patches(c, spawnCands, spawnCount, queue, themes_state);

            // Generate inner patches
            PatchCandidate genCands[Dim::MAX_ACTIVE_PATCHES];
            uint32_t genCount = collect_sorted_patches(c, genCands,
                c->point_.x, c->point_.z,
                [&](const ActivePatch& p) {
                    return p.phase == PatchPhase::SPAWNED &&
                        in_priority_window(c, p.grid_x, p.grid_z, centerX, centerZ);
                }, true);
            generate_selected_patches(c, genCands, genCount,
                encoder, queue, tileGridDirty, tile_world_state, tile_world_deps,
                patchParamsCursor);

            // THE DOOR GUARANTEE (U2) — after the synchronous population
            // above, so Channel A's DOORWAY portals are countable: a world
            // that populated doorless gets one forced door; every other
            // world sees a no-op (the count-gate is inside).
            force_spawn_door_fallback(&mood_deps, queue, *c);
        }
    }

    // ─── THE FRAME'S PACE (RIBBON_5) ──────────────────────────────
    //
    // `young` decides the whole frame: the spawn budget, the eviction budget,
    // and which generation shape runs. It is read before the eviction block
    // because that block spends it, and a patch evicted this frame does not
    // change whether the window was mostly unbuilt when the frame began.
    const bool young = world_is_young(c);

    // ─── CONTINUOUS PATCH EVICTION ────────────────────────────────
    //
    {
        PatchCandidate candidates[Dim::MAX_ACTIVE_PATCHES];
        uint32_t count = collect_sorted_patches(c, candidates,
            c->point_.x, c->point_.z,
            [&](const ActivePatch& p) {
                return !in_render_window(c, p.grid_x, p.grid_z,
                    c->world_state_.last_center_x, c->world_state_.last_center_z);
            }, false);  // farthest first

        // RIBBON_4: a BAKING patch evicts exactly as a SPAWNED one does —
        // the layer is freed and there is nothing to finish, because a bake
        // in flight owns no state outside the patch record itself. The
        // sliced conductor re-finds its work by SCANNING for BAKING, not by
        // index, so the compaction below may move any patch freely.
        // RIBBON_5: a young world evicts at transition pace. The healing
        // migration after a rebirth is EVICTION-BOUND — the wrong window must
        // drain before the right one can allocate, and at two a frame a full
        // 225-patch migration takes 113 frames before the first right patch
        // even exists.
        uint32_t evictThisFrame = std::min(count, young ? 4u : EVICT_BUDGET_PER_FRAME);
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
            // OIL_1 U9 — layers were freed: a capacity-blocked vacancy
            // (counted as zero candidates while free_layer_count was 0)
            // can now allocate, so the scan must run again.
            c->world_state_.alloc_scan_pending = true;
        }
    }

    // ─── CONTINUOUS PATCH ALLOCATION ──────────────────────────────
    //
    // OIL_1 U9 (ledger: R3 continuous allocation, C2): the scan — a
    // fresh unordered_set heap build plus the 15x15 window walk — runs
    // only while demand can exist (alloc_scan_pending; raiser census at
    // the flag's home in surface_services.hpp). Steady frames skip it;
    // a budget-capped pass re-arms itself below.
    {
        // The box raiser: the candidate set is box ∩ window, and the box
        // follows the pawn cell — an input no other raiser tracks once
        // the window is pinned (finite mode). Two compares, and the gate
        // stops depending on why the box and the window agree.
        const int32_t scanGX = (int32_t)std::floor(c->point_.x / Dim::PATCH_EXTENT);
        const int32_t scanGZ = (int32_t)std::floor(c->point_.z / Dim::PATCH_EXTENT);
        if (scanGX != c->world_state_.last_alloc_scan_gx ||
            scanGZ != c->world_state_.last_alloc_scan_gz) {
            c->world_state_.last_alloc_scan_gx = scanGX;
            c->world_state_.last_alloc_scan_gz = scanGZ;
            c->world_state_.alloc_scan_pending = true;
        }
    }
    if (c->world_state_.alloc_scan_pending) {
        int32_t pawnGX = (int32_t)std::floor(c->point_.x / Dim::PATCH_EXTENT);
        int32_t pawnGZ = (int32_t)std::floor(c->point_.z / Dim::PATCH_EXTENT);
        int32_t rr = (int32_t)c->world_state_.active_radius;
        float point_wx = c->point_.x;
        float point_wz = c->point_.z;
        float half = Dim::PATCH_EXTENT * 0.5f;

        // O(1) patch existence lookup (replaces O(N) inner scan)
        auto activePatchSet = build_active_patch_set(c);

        struct AllocCandidate { int32_t gx, gz; float dist2; };
        AllocCandidate candidates[Dim::MAX_ACTIVE_PATCHES];
        uint32_t candidateCount = 0;
        uint32_t vacancies_blocked = 0;   // RIBBON_5 — cells that wanted a layer and found none

        for (int32_t gz = pawnGZ - rr; gz <= pawnGZ + rr; gz++) {
            for (int32_t gx = pawnGX - rr; gx <= pawnGX + rr; gx++) {
                // Must be within allocation window of grid center
                if (!in_render_window(c, gx, gz, c->world_state_.last_center_x, c->world_state_.last_center_z)) continue;
                bool found = activePatchSet.count({ gx, gz }) > 0;
                // RIBBON_5: a vacancy the POOL refused is demand that still
                // exists; the re-arm below needs to know it was seen.
                if (!found && c->world_state_.free_layer_count == 0) vacancies_blocked++;
                if (!found && c->world_state_.free_layer_count > 0 && candidateCount < Dim::MAX_ACTIVE_PATCHES) {
                    float ox = (gx + 0.5f) * Dim::PATCH_EXTENT;
                    float oz = (gz + 0.5f) * Dim::PATCH_EXTENT;
                    float d2 = patch_distance_sq(point_wx, point_wz, ox, oz, half);
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
            // Ensure tile cache entry (primary — ticks terrain tokens),
            // then cache neighbors for tile grid padding — both through
            // the owner's doors.
            ensure_tile(tile_world_state, &tile_world_deps, gx, gz);
            for (int dz = -1; dz <= 1; dz++) for (int dx = -1; dx <= 1; dx++) {
                ensure_tile_padding(tile_world_state, &tile_world_deps, gx + dx, gz + dz);
            }
            // RIBBON_5 — TWO GUARDS, AND THEY ARE INDEPENDENT ON PURPOSE.
            // The capacity check that mattered used to sit in the CANDIDATE
            // SCAN above, which decrements nothing: with free=2 and 15 vacant
            // cells every cell became a candidate, allocThisFrame was
            // min(15, ALLOC_BUDGET_PER_FRAME)=4, and iterations 2 and 3
            // allocated against an empty pool. The second guard is the array
            // bound, stated rather than derived: patches_ is exactly
            // MAX_ACTIVE_PATCHES long and freeLayerStack_ is the member
            // immediately after it, so an overrun here corrupts the free list
            // itself. It must not rest on the pool arithmetic being right.
            if (c->world_state_.active_patch_count >= Dim::MAX_ACTIVE_PATCHES) break;
            uint32_t layer = alloc_layer(c);
            if (layer == UINT32_MAX) break;   // the pool refused; no patch on a stolen layer
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

        // OIL_1 U9 — the backlog carry: a pass capped by
        // ALLOC_BUDGET_PER_FRAME leaves live candidates, so the scan
        // stays armed; a drained pass (every candidate allocated, or
        // none found) disarms it. A vacancy blocked by zero capacity
        // counts no candidate and disarms too — the eviction that
        // eventually frees a layer is the raiser that re-arms (see the
        // eviction block above).
        // RIBBON_5 — THE RE-ARMER'S HOLE. A vacancy blocked by ZERO CAPACITY
        // counted no candidate, so the line below disarmed the scan and left
        // the only re-armer an eviction. If the registry ever sits with real
        // vacancies and an empty pool, and nothing is evictable that frame,
        // the scan sleeps and the world stops growing. Capacity is a
        // TEMPORARY blocker, so it must not disarm: stay armed while a
        // vacancy exists that only the pool refused.
        c->world_state_.alloc_scan_pending =
            (candidateCount > allocThisFrame) ||
            (vacancies_blocked > 0 && c->world_state_.free_layer_count == 0);
    }

    // THE LOOK-AHEAD (RIBBON_4): under a rider the point moves at flight speed,
    // and the patches that matter are the ones ahead. The spawn and bake scans
    // order their candidates from a point PATCH_LOOK_AHEAD along the flight —
    // the saddle's heading, from the same readback the point mirror is filled
    // by. Flight is −dir(heading) (the ribbon's law). The allocation window and
    // the eviction stay on the point itself: the window is the authority on
    // what EXISTS, and only the order in which existing work is served moves.
    const float look = (c->point_.host == PointHost::RIBBON) ? PATCH_LOOK_AHEAD : 0.0f;
    const float gen_cx = c->point_.x - std::cos(c->point_.heading) * look;
    const float gen_cz = c->point_.z - std::sin(c->point_.heading) * look;

    // ─── DISTANCE-DRIVEN ENTITY SPAWNING ─────────────────────────
    //
    {
        PatchCandidate candidates[Dim::MAX_ACTIVE_PATCHES];
        uint32_t count = collect_sorted_patches(c, candidates,
            gen_cx, gen_cz,
            [](const ActivePatch& p) {
                return p.phase == PatchPhase::ALLOCATED;
            }, true);
        // RIBBON_5: the burst is the transition's, the one-a-frame is the
        // steady world's.
        spawn_selected_patches(c, candidates,
            std::min(count, young ? 4u : SPAWN_BUDGET_PER_FRAME), queue, themes_state);
    }

    // THE SCRATCH'S OWNER (RIBBON_5), read AFTER the eviction compaction and
    // not before it: patch_height_scratch holds ONE patch, a BAKING patch
    // fills it across frames, and a whole bake run between its bands would
    // overwrite the rows already laid. Computed here because the compaction
    // above can evict the baker itself — asking earlier would answer for a
    // patch that no longer exists and cost the regen arm a frame for nothing.
    // The answer is a BOOL, never an index: the compaction rewrites patches_
    // by position, so an index taken before it is stale by construction (the
    // hazard RIBBON_4's eviction comment names).
    bool scratch_busy = false;
    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        if (c->patch_system_state_.patches_[i].phase == PatchPhase::BAKING) { scratch_busy = true; break; }
    }

    // ─── HEIGHTFIELD GENERATION ──────────────────────────────────
    //
    // THE BURST SERVES AN EMPTY WINDOW; THE SLICE SERVES A MOVING ONE
    // (RIBBON_5). A world world_is_young() calls YOUNG takes the pre-RIBBON_4
    // shape whole: one scan over both pending phases, whole bakes, the backlog
    // ladder. A whole world takes RIBBON_4's steady shape: the regen arm at
    // its own budget, then one band of one bake. In finite mode the predicate
    // reads the CAPPED active_radius, so a small world is judged against the
    // window it actually has.
    //
    // THE ONE GUARD youth needs is the scratch. If a bake was in flight when
    // youth began — a transition that did not clear the registry, or a window
    // that emptied around a patch mid-bake — the burst would run whole bakes
    // through patch_height_scratch while that patch still owns it. So the
    // carry finishes FIRST, through the sliced conductor, and the burst opens
    // on the frame after. At two slices that costs one frame, once.
    if (young && !scratch_busy) {
        PatchCandidate candidates[Dim::MAX_ACTIVE_PATCHES];
        uint32_t count = collect_sorted_patches(c, candidates,
            gen_cx, gen_cz,
            [](const ActivePatch& p) {
                return p.phase == PatchPhase::SPAWNED ||
                    p.phase == PatchPhase::NEEDS_REGEN;
            }, true);
        generate_selected_patches(c, candidates,
            std::min(count, patches_budget_this_frame(c, inputState, /*young*/ true)),
            encoder, queue, tileGridDirty, tile_world_state, tile_world_deps,
            patchParamsCursor);
    }
    else {
        // REGEN is a transition's debris — a pyramid landed and stale heights
        // must catch up — so it keeps its burst and its whole-bake path, which
        // patches_budget_this_frame paces by backlog. Its patches are already
        // drawn (the grid and the band gate on GENERATED|NEEDS_REGEN), so a
        // slow answer here shows as stale ground, not as a hole.
        //
        // ONE SCRATCH, ONE WRITER (RIBBON_5): a BAKING patch owns
        // patch_height_scratch across frames; a whole bake between its bands
        // would overwrite the rows already laid. The regen arm waits its turn.
        if (!scratch_busy) {
            PatchCandidate candidates[Dim::MAX_ACTIVE_PATCHES];
            uint32_t count = collect_sorted_patches(c, candidates,
                gen_cx, gen_cz,
                [](const ActivePatch& p) {
                    return p.phase == PatchPhase::NEEDS_REGEN;
                }, true);
            generate_selected_patches(c, candidates,
                std::min(count, patches_budget_this_frame(c, inputState, /*young*/ false)),
                encoder, queue, tileGridDirty, tile_world_state, tile_world_deps,
                patchParamsCursor);
        }

        // THE STEADY WORLD (RIBBON_4): one bake in flight, one slice a frame.
        generate_patch_sliced(c, encoder, queue, tileGridDirty,
            tile_world_state, tile_world_deps, patchParamsCursor, gen_cx, gen_cz);
    }

    // The conductor tail as a sequence of named units (was one inline block).
    band_patches(c, queue);
    // OIL_1 U8 (ledger: R3 build_patch_grid, C1): the grid reads only the
    // patch registry — no point input — so it rebuilds and re-uploads only
    // when the registry changed. patch_instances_dirty is read HERE, before
    // its consumption-and-clear below, and its raiser set covers every
    // grid-content change (re-verified at edit time): boot/reset
    // (init_patch_system, and the flag boots true), a patch entering the
    // grid set (generation -> GENERATED), and a patch leaving it (eviction
    // compaction). GENERATED -> NEEDS_REGEN keeps both phases inside the
    // set with identical (gx,gz,layer), so the grid bytes cannot move on
    // that transition.
    if (c->world_state_.patch_instances_dirty) build_patch_grid(c, queue);
    // GPU Y-correction is additive (ground_y += terrain), so ground
    // entries must be re-uploaded with offset-only values whenever
    // the heightfield changes. Tie groundEntriesDirty to patch changes.
    c->world_state_.ground_entries_dirty = c->world_state_.ground_entries_dirty || c->world_state_.patch_instances_dirty;
    c->world_state_.placement_dirty = c->world_state_.placement_dirty || c->world_state_.patch_instances_dirty;
    c->world_state_.patch_instances_dirty = false;

    // ─── Entity distance culling ─────────────────────────────
    c->world_state_.entities_culled = update_entity_draw_visibility(c, queue);

    // ─── Deferred uploads (one per frame max) ────────────────
    if (tileGridDirty) upload_tile_grid_now(tile_world_state, &tile_world_deps, queue, c->world_state_.last_center_x, c->world_state_.last_center_z);

    // ─── THE CONSERVATION WITNESS (RIBBON_5) ─────────────────
    //
    // Always-on, O(N), once a second: valid patches + free layers == the pool.
    // A drift here IS the one-patch world — every comer on a recycled layer,
    // one heightfield mutating under a moving player — and it used to say
    // nothing. This is deliberately NOT behind the instruments dial: a broken
    // world must say so in every build, including the shipped one. The
    // [STREAM] line beside it is diagnosis and rides the meter; this is an
    // alarm and rides nothing.
    {
        static uint32_t s_wit = 0;
        if ((s_wit++ % 60u) == 0u) {
            uint32_t valid_count = 0;
            uint32_t ph[5] = {};
            for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
                if (!c->patch_system_state_.patches_[i].valid) continue;
                valid_count++;
                ph[(uint32_t)c->patch_system_state_.patches_[i].phase]++;
            }
            if (valid_count + c->world_state_.free_layer_count != Dim::MAX_ACTIVE_PATCHES) {
                std::fprintf(stderr,
                    "[Patch] CONSERVATION BROKEN — valid=%u + free=%u != %u "
                    "(active=%u). Layers leaked; the world will stop growing.\n",
                    valid_count, c->world_state_.free_layer_count,
                    Dim::MAX_ACTIVE_PATCHES, c->world_state_.active_patch_count);
            }
            if constexpr (t7::INSTRUMENTS.frame_meter) {
                std::fprintf(stderr,
                    "[STREAM] active=%u free=%u | ALLOC=%u SPAWN=%u BAKING=%u GEN=%u REGEN=%u | "
                    "young=%d center=(%d,%d) point=(%.0f,%.0f)\n",
                    c->world_state_.active_patch_count, c->world_state_.free_layer_count,
                    ph[(uint32_t)PatchPhase::ALLOCATED], ph[(uint32_t)PatchPhase::SPAWNED],
                    ph[(uint32_t)PatchPhase::BAKING], ph[(uint32_t)PatchPhase::GENERATED],
                    ph[(uint32_t)PatchPhase::NEEDS_REGEN],
                    young ? 1 : 0,
                    c->world_state_.last_center_x, c->world_state_.last_center_z,
                    (double)c->point_.x, (double)c->point_.z);
            }
        }
    }

    // Restore radius if we capped it for finite mode
    if (c->world_state_.finite_mode) { c->world_state_.active_radius = savedRadius; }
}

} // namespace the_board
} // namespace t7
