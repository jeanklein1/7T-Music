#pragma once
#include <cstdint>
#include <cstdio>    // the loud drop (record_entity overflow)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)

// ─── surface_services.hpp (CONTRACT: the surface's decl tier) ─────
//
// WorldState (the root organ every stratum reads), the patch registry
// vocabulary (ActivePatch + the S2/S3 boundary face), PatchSystemState,
// the budgets + visibility cylinder, and the surface service DECLS.
// The BODIES ride surface/patch_system.hpp, merged at the cohort tail
// — the same-TU late-definition law, named at spawn_services.
//
// The active-patch machine: the streamed patch registry and its
// lifecycle (allocate → spawn → generate → evict), the frame budgets,
// the layer allocator, the visibility cylinder, and the per-frame
// streaming conductor (stream_patches).
//
// SEAM[spine:active-patch-system] the ActivePatch struct, the
//   patches_ registry, find_patch / evict_patch / evict_patch_entities,
//   plus the entity_refs registry on each
//   ActivePatch. Cross-module readers: machine/spawn_engine.hpp (commit
//   functions call host->record_entity), bodies/ribbon.hpp (two-tip late
//   registration), and the family dispatch eviction rows.
//
// Depends on cohort include order: state.hpp (Dim:: + the GPU patch
// DTOs) precedes this header — the one SANCTIONED cohort cable (array
// extents cannot be root-assigned). The DEMO value-cable is cut: boot
// values are authored at the composition root.

namespace t7 {
namespace the_board {

// ── World state ────────────────────────────────────────────────────

// ROOT ORGAN: the struct's home is here; the
// instance (world_state_) stays at the composition root.
struct WorldState {
    // ── Seed + dimensions ──
    uint32_t active_seed   = 0;  // world master seed — authored at the composition root from DEMO.seed; mutable for world transitions
    uint32_t active_radius = Dim::PATCH_PREGEN_RADIUS;
    bool     finite_mode   = false;
    uint32_t finite_radius = 2;      // 2 → 5×5 = 25 patches
    uint32_t world_gen     = 0;

    // ── Recenter cursor ──
    int32_t last_center_x = INT32_MAX;  // force full regeneration on first frame
    int32_t last_center_z = INT32_MAX;

    // A WORLD IS YOUNG ONCE (RIBBON_6). Set when a world BEGINS — a rebirth,
    // a boot, a radius change — and cleared once, by the conductor, when the
    // window it was given is three quarters built. It is an AGE, not an
    // observation: nothing the player does sets it again, which is what stops
    // a world that has merely fallen behind from being handed a rebirth's
    // burst. Boot is a transition from nothing (L10), so it boots true.
    bool world_young = true;

    // ── Patch counts (this frame) ──
    uint32_t active_patch_count = 0;
    uint32_t render_patch_count = 0;    // drawn patches (within the live RING — the draw authority)
    uint32_t lod0_patch_count   = 0;    // subset of drawn: within lod0_radius (full mesh)
    uint32_t all_patch_count    = 0;    // all generated patches (including pre-gen ring)
    uint32_t entities_culled    = 0;    // entities hidden by the EXIST-ring overdraw cull this frame

    // ── Dirty flags (deferred GPU uploads) ──
    bool ground_entries_dirty   = true;   // defer upload_ground_entries (true at boot)
    bool patch_instances_dirty  = true;   // defer LOD sort + upload_patch_instances
    bool placement_dirty        = true;   // defer dispatch_placement_correction
    // OIL_1 U9 (ledger: R3 continuous allocation, C2): the allocation
    // scan runs only when demand can exist. Raisers, covering every
    // INPUT to the candidate set:
    //   · boot (this default) and init_patch_system (reset);
    //   · gridChanged — the render window moved (this also covers the
    //     cross-module active_radius writer in direction/input.hpp,
    //     which recenters through the same door);
    //   · the scan BOX moved — last_alloc_scan_gx/gz below;
    //   · a freed layer (the eviction block — free_layer's one caller is
    //     evict_patch, whose one caller is that block);
    //   · the budget backlog (candidates exceeded ALLOC_BUDGET_PER_FRAME).
    bool alloc_scan_pending     = true;   // gate on the continuous-allocation scan
    // THE BOX IS NOT THE WINDOW, and that is why it needs its own raiser.
    // The scan intersects box(pawn cell ± radius) with the render window
    // around last_center. In open worlds the two share an origin, so a
    // box move IS a gridChanged. In FINITE mode the window is pinned at
    // (0,0) while the box still follows the pawn — so the box is an
    // independent input, and these two ints are what make the raiser set
    // provably complete instead of complete-by-conjunction (today the
    // finite window is fully allocated by the fullRegen bootstrap and
    // nothing there evicts, so the divergence is unreachable — a fact in
    // three other places, which is one too many to lean on).
    int32_t last_alloc_scan_gx  = INT32_MAX;   // INT32_MAX = never scanned
    int32_t last_alloc_scan_gz  = INT32_MAX;

    // ── Free-layer pool ──
    uint32_t free_layer_count = Dim::MAX_ACTIVE_PATCHES;

    // THE WORLD'S FRAME CLOCK (PANORAMA_1). A monotonic count of served
    // frames, advanced once in phase_advance_clock beside the seconds and the
    // beats — the frame's other two clocks. It exists because the mesh-gen
    // settle needs to say "not again for N frames" and the only counter in
    // the tree was the gallery's own, which was ROSTER-gated and has since
    // left with it. Never reset: a world change is a `world_young` fact, not
    // a clock fact, and a settle that read a restarting clock would fire on
    // the wrap instead of on the wait.
    uint32_t frame_index = 0;
};

// One spelling: patch dimensions are Dim:: everywhere (the veil-chain law).
// PATCH_CELL_SIZE moved to Dim (state.hpp, beside PATCH_CELL_N) — it IS a
// dimension, and the card's cell-exactness assert has to reference it.

// ── The patch registry ─────────────────────────────────────────────

enum class PatchPhase : uint8_t {
    ALLOCATED,      // layer assigned, tile cached, no entities yet
    SPAWNED,        // entities selected + placed + committed
    GENERATED,      // heightfield computed. EVERY family — GoL included —
                    //   was already placed at ALLOCATED->SPAWNED, before
                    //   this heightfield existed; Y-correction is additive
                    //   and lands later (compute_entity_placement).
    NEEDS_REGEN,    // heightfield stale (new pyramid in range)
};

struct ActivePatch {
    int32_t grid_x = 0;
    int32_t grid_z = 0;
    uint32_t layer = 0;
    bool valid = false;
    PatchPhase phase = PatchPhase::ALLOCATED;

    // Entity ownership (recorded at commit, read at eviction)
    struct EntityRef {
        uint32_t family;   // PopFamily index
        uint32_t slot;     // index into Active* array
    };
    static constexpr uint32_t MAX_ENTITY_REFS = 16;   // 10 recording families; host-side clustering can stack triggers — headroom + the loud drop below
    EntityRef entity_refs[MAX_ENTITY_REFS]{};
    uint32_t entity_ref_count = 0;

    void record_entity(uint32_t family, uint32_t slot) {
        if (entity_ref_count < MAX_ENTITY_REFS) {
            entity_refs[entity_ref_count++] = { family, slot };
        } else {
            // LOUD DROP (always-on): an unrecorded entity outlives this
            // patch's eviction — a leak, never silent.
            std::fprintf(stderr,
                "[ActivePatch] entity_ref OVERFLOW patch(%d,%d) family=%u slot=%u dropped (cap %u)\n",
                grid_x, grid_z, family, slot, MAX_ENTITY_REFS);
        }
    }

    // Release-by-owner for RECORDS (REQUEST_1 rider): an owner that dies
    // while this patch still lives takes its record back, so no stale
    // {family, slot} can ever misdirect a later eviction at a successor
    // in the reused slot. Swap-with-last; one record per (family, slot)
    // per patch. NOT to be called from inside evict_patch_entities' own
    // iteration (it would skip the swapped-in record's evictor) — the
    // callers are owner-side death verbs outside any patch loop.
    void unrecord_entity(uint32_t family, uint32_t slot) {
        for (uint32_t i = 0; i < entity_ref_count; i++) {
            if (entity_refs[i].family == family && entity_refs[i].slot == slot) {
                entity_refs[i] = entity_refs[--entity_ref_count];
                return;
            }
        }
    }

};

// ── Dynamic budgets ────────────────────────────────────────────────

// ONE BAKE A FRAME (RIBBON_6). RIBBON_4 was right that the conductor must
// pace by CADENCE and wrong about why: the meter shows streaming's worst
// frame at 2 ms of GPU and frame_total at 2.2 ms of CPU, so it never cost a
// frame — but a pace must still be EVEN, and it must be FAST ENOUGH. A grid
// crossing demands 15 cells; at the top of the speed dial one arrives every
// 19 frames. So the unit is one whole patch per frame — one patch, one
// dispatch, the same shape every streaming frame, and 15 frames to a
// crossing. Evenness by construction, adequacy by arithmetic, and no ladder
// to make a deep backlog work harder exactly when the point is moving fastest.
inline constexpr uint32_t SPAWN_BUDGET_PER_FRAME = 2;   // spawning is CPU and precedes the bake; two keeps the pipeline fed
inline constexpr uint32_t ALLOC_BUDGET_PER_FRAME = 4;   // bookkeeping
inline constexpr uint32_t EVICT_BUDGET_PER_FRAME = 4;   // matched to ALLOC: the pool balances by symmetry as well as by refusal (RIBBON_5's FLAG-38)
inline constexpr uint32_t BAKE_BUDGET_PER_FRAME  = 1;   // the law
inline constexpr uint32_t BAKE_BUDGET_YOUNG      = 6;   // a world being born is a transition and keeps a transition's burst

// THE LOOK-AHEAD (RIBBON_4): how far along the flight the spawn and bake
// scans order their candidates from, under a rider. Zero in every other
// host — a walker's point IS where the work is.
inline constexpr float    PATCH_LOOK_AHEAD       = 100.0f;  // wu

// ── Visibility — THE VEIL CHAIN (RING = draw authority) ────────────
//
// The chain is declared ONCE in Dim (state.hpp: VEIL_RING_DEFAULT
// 325 / VEIL_ICING_DEFAULT 40 / LOD0_RADIUS_DEFAULT 175 / EXIST_RADIUS
// 350, chain static_asserts). The LIVE values ride config (veil_ring/
// veil_icing/lod0_radius, tunable): the CPU band reads gpuState_.
// veil_ring()/lod0_radius(), the entity cull reads veil_ring(), the GPU
// LOD0 gate reads fc_config.lod0_radius, every VS draw gate + the
// fragment icing read config — one yardstick, by construction,
// everywhere. Grid-based allocation/eviction unchanged.

// ── Distance-sorted patch scan helper ──

struct PatchCandidate {
    uint32_t idx;
    float dist2;
};

// ═══ MODULE STATE ══════════════════════════════════════════════════

// Instance (patch_system_state_) lives at the composition root.
struct PatchSystemState {
    ActivePatch patches_[Dim::MAX_ACTIVE_PATCHES]{};
    // Free-list of available texture layers
    uint32_t freeLayerStack_[Dim::MAX_ACTIVE_PATCHES]{};
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═══════════════════════════════
//
// DEFINED in surface/patch_system.hpp (merged, cohort tail): the machine stands on
// THE MACHINE FACE (MachineCtx), the S3 dispatch seam
// (select/place/commit — contracts/spawn_services.hpp), and the GPU
// wire (gpuState_ / renderer_). The reaches outside the face ride
// the call sites: the tile doors' deps, the mood deps, the driver's
// intent organ.
struct TileWorldState;  // tile_world.hpp — the tile cache organ (fwd: the lifecycle owner mutates it through the owner doors; the machine face's view is const)
struct TileWorldDeps;   // tile_world.hpp — the tile doors' face (fwd: reference param)
struct MoodDeps;        // mood.hpp — the back-portal door's face (fwd: reference param)

// THE S2/S3 BOUNDARY FACE: the patch registry is read across the
// boundary by the occupier commits (host->record_entity via
// find_patch) — the interface trio's registry member.
ActivePatch* find_patch(MachineCtx* c, int32_t gx, int32_t gz);

void evict_patch(MachineCtx* c, uint32_t pi, wgpu::Queue& queue);
void evict_patch_entities(MachineCtx* c, ActivePatch& patch, wgpu::Queue& queue);

// Root-called owner verb. CALLERS: boot (init_renderer) AND the transition
// machine (root); OWNER: patch_system. One door, both paths — boot is a
// transition from nothing (LAWS L10).
void reset_surface(MachineCtx* c, wgpu::Queue& queue,
    TileWorldState& tile_world_state);  // was teardown_world -> teardown_surface; reduced to the surface core, then called from boot too

void init_patch_system(MachineCtx* c, TileWorldState& tile_world_state);
// The recenter door: names the hidden regen request — the
// streaming conductor re-evaluates the full window next frame.
// Caller: the radius command (direction/input). DEPS-FORM: the
// driver world holds no MachineCtx — the door takes its
// one organ explicitly (the deps-form precedent, clear_spheres).
void request_recenter(WorldState& ws);
void mark_patches_for_regen(MachineCtx* c, float min_wx, float min_wz,
    float max_wx, float max_wz,
    int32_t home_gx, int32_t home_gz);
// LATTICE_1 — one pass, two dispatches, the whole batch.
void generate_patch_batch(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count);
GPUPatchParams make_patch_params(MachineCtx* c, int32_t gx, int32_t gz, uint32_t layer);
uint32_t alloc_layer(MachineCtx* c);
void free_layer(MachineCtx* c, uint32_t layer);
bool in_render_window(MachineCtx* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz);
float patch_distance_sq(float px, float pz, float origin_x, float origin_z, float half);
template<typename Pred>
uint32_t collect_sorted_patches(MachineCtx* c, PatchCandidate* out,
    float pawn_wx, float pawn_wz, Pred&& pred, bool nearest_first);
bool in_priority_window(MachineCtx* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz);
void spawn_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::Queue& queue);
void generate_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    bool& tileGridDirty,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps);

// THE CONDUCTOR: the per-frame streaming step.
void stream_patches(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    TileWorldState& tile_world_state,
    TileWorldDeps& tile_world_deps, MoodDeps& mood_deps);

} // namespace the_board
} // namespace t7
