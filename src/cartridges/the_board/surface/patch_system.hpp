#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/surface_services.hpp"  // the surface's decl tier (this module's own contract)
#include <cmath>          // std::floor, std::sqrt, std::abs   // (impl, merged)
#include <algorithm>      // std::min, std::max   // (impl, merged)
#include <cstring>        // std::memcpy (instance banding)   // (impl, merged)
#include <iostream>       // DIAG blocks (lifecycle audit + evict trace)   // (impl, merged)
#include <cstdio>         // std::fprintf — RIBBON_5's conservation witness (always-on, every build)
#include "core/instruments.hpp"   // RIBBON_5 — INSTRUMENTS.frame_meter gates the [STREAM] line

// ─── patch_system.hpp (S2 · MERGED: the active-patch machine) ──────
// The decl tier lives in contracts/surface_services.hpp; this file is the
// machine's bodies whole — the registry lifecycle (allocate → spawn →
// generate), world teardown, the layer allocator, and the ONE-SHOT
// BUILDER. It carried a fourth lifecycle stage (evict), the per-frame
// budgets that paced all four, and the streaming conductor that spent
// them; a finite world is built once, so all three left at ONE_SURFACE-I
// U2 and U3. Stands on THE MACHINE FACE (the root organs are machine
// members), the S3 dispatch seam (select/place/commit —
// contracts/spawn_services.hpp), and the GPU wire (c->gpuState_ /
// c->renderer_). The reaches OUTSIDE the face ride the call site (the B
// law): the WRITABLE tile-cache organ (the face's view is const — the
// lifecycle owner mutates it through the owner doors) and the tile
// doors' deps. COHORT: the tail's last — after the machine natives
// (spawn service defs) and sky (reset_surface's def).
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

// ══ THE EVICTION LANE STOOD HERE (ONE_SURFACE-I U3) ═════════════════
//
// `evict_patch` freed a patch's layer and swept its entities;
// `evict_patch_entities` walked the host patch's `entity_refs` and called
// FAMILY_DISPATCH[f].evict_slot on each. Their one caller was the
// conductor's CONTINUOUS PATCH EVICTION block, which left at U2 — patches
// never die mid-world now, so patch-death eviction has no subject.
//
// THE GUARD THE HANDOFF DEMANDED, ANSWERED BEFORE THE CUT. `rebirth_world`
// was traced verb by verb: `reset_surface` (which resets the registry
// wholesale through `init_patch_system`, never walking a ref),
// `teardown_entities`, `teardown_gol`, `teardown_ribbon`, `clear_spheres`,
// `clear_cubes`, `teardown_pawn_aura`, `teardown_orbs`. NOT ONE reads
// `entity_refs`. Every teardown sweeps by OWNER. `unrecord_entity`, the
// release-by-owner half of the protocol, already had zero callers before
// this campaign found it.
//
// ── Dynamic budgets ────────────────────────────────────────────────

// ── World lifecycle ────────────────────────────────────────────────
//
// Root-called owner verb. CALLER: rebirth_world
// (root); OWNER: patch_system. The bulk
// sweep over sibling organs dissolved into per-owner teardown verbs
// called by the score's TEARDOWN movement; this core keeps the
// surface's own concerns — patches, tiles, themes, dispatch queues,
// footprints — plus the world-rebirth GPU staging lines.
// THE ONE SURFACE RESET. Called from BOTH paths — boot and rebirth_world
// — because boot is a birth from nothing (LAWS L10). Every line
// below was already true at boot, but by in-struct default rather than by
// call: the equality was asserted by luck and would have parted silently the
// day any default moved. Now it is enforced by call.
inline void reset_surface(MachineCtx* c, wgpu::Queue& queue,
    TileWorldState& tile_world_state) {
    // Patches + tile cache
    init_patch_system(c, tile_world_state);
    c->world_state_.last_center_x = INT32_MAX;  // force full regen on next frame
    c->world_state_.last_center_z = INT32_MAX;

    // Terrain tokens — through the owner's door
    reset_terrain_memory(tile_world_state);

    c->spawn_engine_state_.entityQueueCount_ = 0;
    c->spawn_engine_state_.placementCount_ = 0;

    // Footprints
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        c->spawn_engine_state_.footprints_[i] = GroundFootprint{};
    }

    // Lights need re-upload with potentially new config
    c->sky_state_.lights_dirty = true;

    // New world decides its own upload frequency policy
    c->gpuState_.set_config_dynamic(false);
}
// `mark_patches_for_regen` stood here — the pyramid re-bake lane. It
// marked GENERATED patches NEEDS_REGEN when a pyramid committed inside
// them, so the heightfield would bake again over the new contribution.
// It is structurally unreachable in a world built once: its sole caller
// is `pyramid_post_commit`, and at a birth every patch is ALLOCATED or
// SPAWNED when the pyramids commit — nothing is GENERATED yet, so the
// scan matched nothing even before this campaign. The lane existed for
// pyramids committed AFTER some patches had baked, which only a streamed
// steady state produces (ONE_SURFACE-I U6).

// ── Patch subsystem setup ──────────────────────────────────────────

inline void init_patch_system(MachineCtx* c, TileWorldState& tile_world_state) {
    for (uint32_t i = 0; i < Dim::MAX_ACTIVE_PATCHES; i++) {
        c->patch_system_state_.freeLayerStack_[i] = Dim::MAX_ACTIVE_PATCHES - 1 - i;
    }
    c->world_state_.free_layer_count = Dim::MAX_ACTIVE_PATCHES;
    c->world_state_.active_patch_count = 0;
    c->world_state_.render_patch_count = 0;
    c->gpuState_.stage_placement_patch_count(0);
    reset_tile_cache(tile_world_state);  // owner door
    c->world_state_.patch_instances_dirty = true;
}

// ── Patch generation ───────────────────────────────────────────────

// The params ride a read-only STORAGE array: one contiguous write for the
// whole batch before the pass is recorded, and workgroup_id.z picks the
// record. ONE BATCH PER SUBMIT (LATTICE_1) — see build_world's bake loop.
// ONE PASS, TWO DISPATCHES, THE WHOLE BATCH (LATTICE_1). This was a pass PAIR
// per patch: N patches cost 2N passes and 3N dispatches, each pass carrying a
// bind and a boundary that existed to order a scratch buffer which no longer
// exists. The bake reads its patch from patch_params_batch[workgroup_id.z], so
// the batch is the dispatch's z extent and the pass count stops depending on
// the batch size at all.
//
// CELLS RIDE THE SAME PASS. The binding ledger's W1-4 row lists
// {generate_patch_cells, generate_patch_heights} and {generate_patch_cells,
// generate_patch_gradients} as hazard-free in both directions — the pair
// writes disjoint textures — so the boundary between them was never buying
// ordering either.
inline void generate_patch_batch(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count) {
    if (count == 0) return;
    count = c->gpuState_.upload_patch_params(queue, params, count);
    if (count == 0) return;

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Patch Bake (fused)";
    // THE BAKE IS A BIRTH PASS AND THE FRAME METER CANNOT HOLD IT
    // (ONE_SURFACE-I U2). It armed meter_row::StreamPatches while the
    // conductor baked one patch a frame. build_world is its only caller now,
    // and meter_frame_begin() resets the allocator at every frame head — so
    // a pair allocated at birth is discarded before any resolve can read it.
    // An arm that can never be read is a lie in the meter's own table.
    wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
    // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
    { cp.SetBindGroup(0, c->gpuState_.world_group());
      cp.SetBindGroup(1, c->gpuState_.frame_c_group()); }
    c->renderer_.dispatch_bake_patches(cp,
        c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(),
        GPUState::patch_heightfield_workgroups(), count);
    c->renderer_.dispatch_generate_patch_cells(cp,
        c->gpuState_.patchgen_state_group(), c->gpuState_.patchgen_textures_group(),
        GPUState::patch_cell_workgroups(), count);
    cp.End();
}

inline GPUPatchParams make_patch_params(MachineCtx* c, int32_t gx, int32_t gz, uint32_t layer) {
    GPUPatchParams p{};
    (void)c;   // LATTICE_1: the seed reaches the bake through config.world_seed
    p.origin[0] = (gx + 0.5f) * Dim::PATCH_EXTENT;
    p.origin[1] = (gz + 0.5f) * Dim::PATCH_EXTENT;
    p.layer = layer;
    p._pad0 = 0u;
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

// `free_layer` stood here — the pool's return half, carrying RIBBON_5's
// double-free guard. Its only caller was `evict_patch`. A finite world
// draws from the pool once, at birth, and never returns to it: the stack
// is filled by `init_patch_system` and popped (2R+1)^2 times, and
// build_world's conservation check is what proves that (ONE_SURFACE-I U3).

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

// Process entity spawn for pre-collected patch candidates.
//
// ONE PATCH, DRAINED WHOLE (PANORAMA_0 RIDE_1). The three verbs used to run
// as three passes over the whole candidate list: every patch selected, then
// everything placed, then everything committed. That made the queue's
// capacity a function of the CANDIDATE COUNT, and the fullRegen arm hands
// this function all 49 patches of the priority window in one call — so the
// queue overflowed at boot and at every portal, and what it dropped was the
// tail of PLACEMENT_ORDER — whatever the order listed last, dropped first.
// The birth of every world was being truncated by a bound that called
// itself proven.
//
// Draining per patch makes the bound true instead of arguing it: at most one
// patch's selections are ever in flight, so PopFamily::COUNT is the real
// ceiling and no candidate list can reach it.
//
// PLACEMENT ORDER IS UNCHANGED. Select already queued patch-by-patch, and
// footprints register at PLACE — so patch N+1's place sees exactly the ground
// it saw before, whether patch N's place happened one iteration earlier or one
// pass earlier. The one difference is real and is an improvement: a commit
// that RELEASES ground (a family that placed nothing unregisters its
// footprint) now does so before the next patch places, so ground that is
// genuinely free can be used by it.
inline void spawn_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::Queue& queue) {
    // THE ENVELOPE'S TICK LEFT (ONE_WORLD-II U3). Each patch advanced the
    // theme envelope from its own tile seed before selecting entities —
    // the temporal half of the engine that died there, stateful and
    // sequenced. The selection is the whole of the work now.
    for (uint32_t s = 0; s < count; s++) {
        uint32_t pi = candidates[s].idx;
        select_entities_for_patch(c, c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z);
        c->patch_system_state_.patches_[pi].phase = PatchPhase::SPAWNED;
        place_entity_queue(c);
        commit_entity_queue(c, queue);
    }

    // THE LATE TIP REGISTRATION STOOD HERE. A ribbon committed against a
    // patch whose other tip had not been allocated yet registered that tip
    // when its patch arrived. The builder allocates the WHOLE grid before
    // the first commit, so `find_patch` resolves both tips at commit time
    // and there is no later to register in (ONE_SURFACE-I U3).
}

// Process heightfield generation for pre-collected patch candidates.
inline void generate_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    bool& tileGridDirty,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps) {
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
    generate_patch_batch(c, encoder, queue, batchParams, count);
    for (uint32_t b = 0; b < count; b++) {
        uint32_t pi = batchIdx[b];
        c->patch_system_state_.patches_[pi].phase = PatchPhase::GENERATED;
    }
    c->world_state_.patch_instances_dirty = true;
}

// ── band_patches (visibility/LOD → the draw-instance set) ──────────
// One unit of the conductor: walk the GENERATED patches, gate each on the
// world-space visibility cylinder (finite mode = all visible, "walls
// define boundary, not fog"), split LOD0/LOD1/pregen, pack them
// [lod0|lod1|pregen] (the draw-split contract the render reads by count),
// upload the instance buffer + the lod0/render/all counts + the placement
// patch count, and push cull_point so the frustum-cull shader culls on
// the SAME point the CPU banded on (the anti-flicker contract).
//   offer-face: GPUPatchInstance[] + render_patch_count + cull_point.
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
    // ONE BAND (ONE_SURFACE-I U5). The tri-pack was [lod0|lod1|pregen]:
    // three arrays, three counts, three memcpys into one instance buffer,
    // and the pack ORDER was the wire contract the draw read by count.
    // The pregen band left at U4 with the ring gate that filled it; the
    // LOD1 band leaves here with the half-mesh it drew. What is left is
    // the walk itself.
    GPUPatchInstance instances[Dim::MAX_ACTIVE_PATCHES];
    uint32_t w = 0;

    float point_wx = c->point_.x;   // THE POINT (1-frame stale by law E-4)
    float point_wz = c->point_.z;

    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED) continue;

        float ox = (c->patch_system_state_.patches_[i].grid_x + 0.5f) * Dim::PATCH_EXTENT;
        float oz = (c->patch_system_state_.patches_[i].grid_z + 0.5f) * Dim::PATCH_EXTENT;

        GPUPatchInstance inst{};
        inst.origin[0] = ox;
        inst.origin[1] = oz;
        inst.extent = Dim::PATCH_EXTENT;
        inst.layer = c->patch_system_state_.patches_[i].layer;

        // EVERY PATCH IS VISIBLE AND EVERY PATCH IS FULL MESH: a walled
        // world draws all of what it has, at one density.
        instances[w++] = inst;
    }

    c->gpuState_.upload_patch_instances(queue, instances, w);
    c->world_state_.render_patch_count = w;

    // Sync placement_patch_count so compute_entity_placement
    // can sample heightfields from the current frame's patch set.
    c->gpuState_.stage_placement_patch_count(w);
    c->gpuState_.upload_placement_patch_count(queue);

    c->gpuState_.stage_cull_point(point_wx, point_wz);
    c->gpuState_.upload_cull_point(queue);
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
        if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED) continue;
        min_gx = std::min(min_gx, c->patch_system_state_.patches_[i].grid_x);
        min_gz = std::min(min_gz, c->patch_system_state_.patches_[i].grid_z);
    }
    if (min_gx == INT32_MAX) { min_gx = 0; min_gz = 0; }
    grid.origin_x = min_gx;
    grid.origin_z = min_gz;

    for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++) {
        if (!c->patch_system_state_.patches_[i].valid) continue;
        if (c->patch_system_state_.patches_[i].phase != PatchPhase::GENERATED) continue;
        int32_t lx = c->patch_system_state_.patches_[i].grid_x - grid.origin_x;
        int32_t lz = c->patch_system_state_.patches_[i].grid_z - grid.origin_z;
        if (lx < 0 || lz < 0 ||
            lx >= int32_t(grid.side) || lz >= int32_t(grid.side)) continue;
        grid.entries[lz * grid.side + lx] = c->patch_system_state_.patches_[i].layer + 1u;
    }

    c->gpuState_.upload_patch_grid(queue, grid);
}

// ══ build_world — THE ONE-SHOT BUILDER (ONE_SURFACE-I U1) ═══════════
//
// A FINITE WORLD IS BUILT ONCE. The conductor below streams a window
// across an endless plane: it recenters, evicts what falls out, allocates
// what falls in, and paces the spend so no frame pays for more than a few
// patches. Every one of those verbs answers a question a finite world does
// not ask — the window never moves, nothing ever falls out, and the whole
// grid is known the moment the radius is drawn. So the grid is allocated,
// spawned, baked, banded and uploaded HERE, at the world's birth, and the
// frames that follow find it already standing.
//
// THE THREE GPU FACTS COME FIRST, AND THIS IS NOT OPTIONAL. The bake reads
// the world seed through `config.world_seed` (make_patch_params' own note),
// and `set_world_seed` / `set_world_bounds` only STAGE — the write is
// phase_stage_upload's, a per-frame UPDATE row that has not run when a boot
// builder bakes. So the builder stages the pair and drains the config
// itself, before its first dispatch. phase_stage_world keeps doing the same
// per frame, dirty-gated and idempotent; this is not a second author, it is
// the same author reached at a moment the spine has not yet arrived at.
//
// ONE BATCH PER SUBMIT (LATTICE_1), AND THE ARITHMETIC SAYS ONE SUBMIT.
// generate_patch_batch writes the whole batch to patch_params at record 0
// and the queue orders every WriteBuffer ahead of the entire command
// buffer — so two batches on one encoder land the second's params on the
// first's records before either dispatch runs. The loop below therefore
// closes an encoder per batch. patchParamsBuffer_ holds MAX_ACTIVE_PATCHES
// (225) records and the widest world the pin allows is radius 4 — 9x9 = 81
// patches — so the loop runs EXACTLY ONCE at every radius in
// [FINITE_RADIUS_MIN, FINITE_RADIUS_MAX]. It is written as a loop anyway,
// because a bound that is enforced by structure cannot rot the way a bound
// that is merely true today can.
//
// NEAREST FIRST, ONE PASS, AND THE ONE DISCLOSED DELTA. The conductor
// spawned the priority window (PATCH_GRID_RADIUS, 7x7) inside its
// fullRegen arm and left the rest to later frames' distance-driven block.
// The builder spawns all of it in one nearest-first pass, which is the
// tree's own stated priority law with no budget left to serve. The two
// orders are NOT the same sequence: (4,0) is 200 wu from the origin and
// (3,3) is 212, so a one-pass sort serves a patch OUTSIDE the old priority
// window before one inside it. Entity SELECTION is seed-driven and
// unchanged; what can differ is which of two entities wins ground both
// want, since footprints register at PLACE in candidate order. Disclosed
// for the walk, not claimed as identity.
// SEAM[patch:spawn-trigger] — RE-FOUNDED HERE (ONE_SURFACE-I U6). The
// S3-trigger calls are the declared seam face: select_entities_for_patch /
// place_entity_queue / commit_entity_queue, through spawn_selected_patches
// at step 4 below. The seam fired from the conductor's own cadence and now
// fires ONCE, at a world's birth — the surface machine waking the occupier
// machine, at the one moment there is a surface to wake it about. Its old
// banner also named `update_entity_draw_visibility` "at the frame tail";
// that function returned a constant 0 from ONE_WORLD-I U3 onward and left
// at U6.
inline void build_world(MachineCtx* c, wgpu::Device& device, wgpu::Queue& queue,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps) {
    // ── 1. The world's identity, on the GPU, before anything reads it ──
    c->gpuState_.set_world_seed(c->world_state_.active_seed);
    {
        const float bmin = -(float)c->world_state_.finite_radius * Dim::PATCH_EXTENT;
        const float bmax = ((float)c->world_state_.finite_radius + 1.0f) * Dim::PATCH_EXTENT;
        c->gpuState_.set_world_bounds(bmin, bmin, bmax, bmax);
    }
    c->gpuState_.upload_config(queue);

    const int32_t r  = (int32_t)c->world_state_.finite_radius;
    const int32_t cx = 0;   // the finite grid is centred on the origin, always
    const int32_t cz = 0;
    c->world_state_.last_center_x = cx;
    c->world_state_.last_center_z = cz;

    // ── 2. Tiles, over the grid plus one pad ring (the conductor's TILE_PAD) ──
    static constexpr int32_t TILE_PAD = 1;
    for (int32_t gz = cz - (r + TILE_PAD); gz <= cz + (r + TILE_PAD); gz++)
        for (int32_t gx = cx - (r + TILE_PAD); gx <= cx + (r + TILE_PAD); gx++)
            ensure_tile(tile_world_state, &tile_world_deps, gx, gz);

    // ── 3. Allocate every cell of the grid ────────────────────────────
    // The pool is MAX_ACTIVE_PATCHES deep and this asks for (2r+1)^2 <= 81,
    // so alloc_layer cannot refuse; the guards stay because a refusal that
    // cannot happen is still a refusal that must not be silent.
    for (int32_t gz = cz - r; gz <= cz + r; gz++) {
        for (int32_t gx = cx - r; gx <= cx + r; gx++) {
            if (c->world_state_.active_patch_count >= Dim::MAX_ACTIVE_PATCHES) break;
            const uint32_t layer = alloc_layer(c);
            if (layer == UINT32_MAX) break;   // the pool refused; it says so itself
            ActivePatch& p = c->patch_system_state_.patches_[c->world_state_.active_patch_count];
            p = ActivePatch{};
            p.grid_x = gx;
            p.grid_z = gz;
            p.layer  = layer;
            p.valid  = true;
            c->world_state_.active_patch_count++;
        }
    }

    // ── 3b. THE CONSERVATION WITNESS, at the one moment it can be broken ──
    // RIBBON_5 ran this every second inside the conductor, because alloc and
    // evict churned every frame and a leaked layer was a world that stopped
    // growing. Nothing churns now: the pool is drawn from once, here, and
    // never returned to. So the check moves to the only edge that remains —
    // the allocation loop above — and stays ALWAYS-ON and outside the
    // instruments dial, for RIBBON_5's own reason: a broken world must say
    // so in every build, including the shipped one.
    {
        uint32_t valid_count = 0;
        for (uint32_t i = 0; i < c->world_state_.active_patch_count; i++)
            if (c->patch_system_state_.patches_[i].valid) valid_count++;
        const uint32_t want = (uint32_t)((2 * r + 1) * (2 * r + 1));
        if (valid_count != want ||
            valid_count + c->world_state_.free_layer_count != Dim::MAX_ACTIVE_PATCHES) {
            std::fprintf(stderr,
                "[Ground] CONSERVATION BROKEN AT BIRTH — valid=%u (wanted %u) "
                "+ free=%u != %u. The world was built short.\n",
                valid_count, want, c->world_state_.free_layer_count,
                Dim::MAX_ACTIVE_PATCHES);
        }
    }

    // ── 4. Spawn — every patch, nearest first from the point ──────────
    {
        PatchCandidate cands[Dim::MAX_ACTIVE_PATCHES];
        const uint32_t n = collect_sorted_patches(c, cands, c->point_.x, c->point_.z,
            [](const ActivePatch& p) { return p.phase == PatchPhase::ALLOCATED; }, true);
        spawn_selected_patches(c, cands, n, queue);
    }

    // ── 5. Bake — one batch per submit ────────────────────────────────
    {
        PatchCandidate cands[Dim::MAX_ACTIVE_PATCHES];
        const uint32_t n = collect_sorted_patches(c, cands, c->point_.x, c->point_.z,
            [](const ActivePatch& p) { return p.phase == PatchPhase::SPAWNED; }, true);
        bool tileGridDirty = true;   // the grid is new; generate_selected_patches drains it first
        for (uint32_t base = 0; base < n; base += Dim::MAX_ACTIVE_PATCHES) {
            const uint32_t take = std::min(n - base, Dim::MAX_ACTIVE_PATCHES);
            wgpu::CommandEncoderDescriptor encDesc{};
            encDesc.label = "build_world";
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encDesc);
            generate_selected_patches(c, cands + base, take, encoder, queue,
                tileGridDirty, tile_world_state, tile_world_deps);
            wgpu::CommandBufferDescriptor cmdDesc{};
            cmdDesc.label = "build_world";
            wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
            queue.Submit(1, &commands);
        }
        // The tile grid rides generate_selected_patches' own drain when there
        // is a batch; a world with no patches to bake still owes the upload.
        if (tileGridDirty)
            upload_tile_grid_now(tile_world_state, &tile_world_deps, queue, cx, cz);
    }

    // ── 6. The conductor's tail, once ─────────────────────────────────
    band_patches(c, queue);
    build_patch_grid(c, queue);
    c->world_state_.patch_instances_dirty = false;

    // ── 7. RIBBON_6's AGE, and the field that carried it ──────────────
    // `world_young` was set at a world's beginning and cleared once, when
    // its window was three-quarters built, so a world being born could be
    // handed a burst the steady state did not get. A world built whole is
    // born already grown — there is no partial state to be young IN, and no
    // budget to select. The field left at ONE_SURFACE-I U6; the LAW it
    // stated is this function.

    // THE BIRTH CENSUS (OVERTURE_0), kept at its own moment: the first
    // count of a world that exists.
    dump_entity_census(c, "born");

    std::cout << "[Ground] Built " << c->world_state_.active_patch_count
              << " patches (" << (2 * r + 1) << "x" << (2 * r + 1) << ")\n";
}

// ══ THE CONDUCTOR STOOD HERE (ONE_SURFACE-I U2) ═════════════════════
//
// `stream_patches` was the per-frame step of an endless plane: it
// recentred the window on the point, evicted what fell out, allocated
// what fell in, spawned and baked under per-frame budgets, banded, and
// re-armed its own scan. Nine hundred lines of answer to a question a
// finite world does not ask — the window never moves, nothing ever falls
// out, and the whole grid is known the moment the radius is drawn.
//
// TAKEN BY ENUMERATION (Amendment B clause 3), each death-verified:
//   stream_patches · request_recenter · in_render_window ·
//   in_priority_window · build_active_patch_set · the six budgets
//   (SPAWN/ALLOC/EVICT/BAKE_BUDGET_PER_FRAME, BAKE_BUDGET_YOUNG,
//   PATCH_LOOK_AHEAD) · alloc_scan_pending and its two scan-box cursors ·
//   the [STREAM] diagnostic · the per-second conservation witness (its
//   assertion moved into build_world, where it can still be violated) ·
//   evict_distant_tiles' KeepFn overload (the moved-window sweep) ·
//   set_render_radius and the [ ] keys that drove it.
//
// WHAT WAS PER-FRAME AND STILL IS: band_patches, which re-bands the draw
// set as the point moves and stages the point the cull kernel re-bands
// on, and update_entity_draw_visibility, the entity distance cull. Both
// are functions of a MOVING POINT, not of a moving window, so both keep
// their cadence and moved to phase_surface_visibility.
//
// WHAT WAS THE CONDUCTOR'S AND IS NOW THE BUILDER'S: build_patch_grid and
// the tile-grid upload. Neither can change after a birth — nothing raises
// patch_instances_dirty once the grid is whole — so a per-frame gate on
// them was a gate on a constant.
//
// The eviction lane it drove (evict_patch, evict_patch_entities,
// free_layer, the entity_refs registry) still compiles and is now
// callerless; U3 takes it under its own guard.

} // namespace the_board
} // namespace t7
