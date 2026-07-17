#pragma once
#include <cstdint>
#include <cstdio>    // the loud drop (record_entity overflow)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)

// ─── surface_services.hpp (CONTRACT: the surface's decl tier) ─────
// History: audit/LADDER.md
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
//   patches_ registry, find_patch / evict_patch / evict_patch_entities /
//   audit_entity_integrity, plus the entity_refs registry on each
//   ActivePatch. Cross-module readers: spawn_engine.inl (commit
//   functions call host->record_entity), ribbon.inl (two-tip late
//   registration), gallery.inl (evict_paintings_for_patch via the
//   owner-side evict_gallery), and the family dispatch eviction rows.
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

    // ── Patch counts (this frame) ──
    uint32_t active_patch_count = 0;
    uint32_t render_patch_count = 0;    // drawn patches (within the live RING — the draw authority)
    uint32_t lod0_patch_count   = 0;    // subset of drawn: within lod0_radius (full mesh)
    uint32_t all_patch_count    = 0;    // all generated patches (including pre-gen ring)
    uint32_t entities_culled    = 0;    // entities hidden by the EXIST-ring overdraw cull this frame

    // ── Dirty flags (deferred GPU uploads) ──
    bool pier_count_dirty       = false;  // defer recompute_and_upload_pier_count
    bool ground_entries_dirty   = true;   // defer upload_ground_entries (true at boot)
    bool patch_instances_dirty  = true;   // defer LOD sort + upload_patch_instances
    bool placement_dirty        = true;   // defer dispatch_placement_correction

    // ── Free-layer pool ──
    uint32_t free_layer_count = Dim::MAX_ACTIVE_PATCHES;
};

// One spelling: patch dimensions are Dim:: everywhere (the veil-chain law).
inline constexpr float PATCH_CELL_SIZE = (float)Dim::PATCH_EXTENT / 16.0f;  // 3.125 — patch-grid cell; pawn aura + gol zones consume it

// ── The patch registry ─────────────────────────────────────────────

enum class PatchPhase : uint8_t {
    ALLOCATED,      // layer assigned, tile cached, no entities yet
    SPAWNED,        // entities selected + placed + committed
    GENERATED,      // heightfield computed, gallery + GoL spawned
    NEEDS_REGEN,    // heightfield stale (new pier in range)
};

struct ActivePatch {
    int32_t grid_x = 0;
    int32_t grid_z = 0;
    uint32_t layer = 0;
    bool valid = false;
    PatchPhase phase = PatchPhase::ALLOCATED;
    bool animated = false;   // true if patch overlaps an active pool

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

    void unregister_entity(uint32_t family, uint32_t slot) {
        for (uint32_t i = 0; i < entity_ref_count; i++) {
            if (entity_refs[i].family == family && entity_refs[i].slot == slot) {
                entity_refs[i] = entity_refs[--entity_ref_count];
                return;
            }
        }
    }

};

// ── Dynamic budgets ────────────────────────────────────────────────

inline constexpr uint32_t SPAWN_BUDGET_PER_FRAME = 4;    // max patches to spawn entities for
inline constexpr uint32_t ALLOC_BUDGET_PER_FRAME = 4;    // max patches to allocate per frame
inline constexpr uint32_t EVICT_BUDGET_PER_FRAME = 4;    // max patches to evict per frame
inline constexpr uint32_t PATCH_BUDGET_MIN = 1;
inline constexpr uint32_t PATCH_BUDGET_MAX = 6;
inline constexpr uint32_t PATCH_PENDING_TIER_1 = 3;
inline constexpr uint32_t PATCH_PENDING_TIER_2 = 8;
inline constexpr uint32_t PATCH_PENDING_TIER_3 = 20;
inline constexpr uint32_t PATCH_PENDING_TIER_4 = 40;
inline constexpr uint32_t PATCH_BUDGET_MOVE_THRESHOLD = 4;

// ── Visibility — THE VEIL CHAIN (re-ruled: RING = draw authority) ──
//
// The old two-group vocabulary (VISIBLE_RADIUS/VISIBILITY_CYLINDER_* and
// LOD_FULL_RADIUS/LOD0_CYLINDER_* — seven spellings of two numbers) is
// RETIRED. The chain is declared ONCE in Dim (state.hpp: VEIL_RING_DEFAULT
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
    // The unified pier mirror (PIERS ride patch_system)
    GPUPierInstance cpuPiers_[Dim::PIER_TOTAL]{};
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═══════════════════════════════
//
// DEFINED in surface/patch_system.hpp (merged, cohort tail): the machine stands on
// THE MACHINE FACE (MachineCtx), the S3 dispatch seam
// (select/place/commit — contracts/spawn_services.hpp), and the GPU
// wire (gpuState_ / renderer_). The reaches outside the face ride
// the call sites: the tile doors' deps, the mood deps, the driver's
// intent organ.
struct TileWorldState;  // tile_world.hpp — the tile cache organ (fwd: the lifecycle owner mutates it through the m4 doors; the machine face's view is const)
struct ThemesState;     // population_themes.hpp — the theme envelope organ (fwd: same law)
struct TileWorldDeps;   // tile_world.hpp — the tile doors' face (fwd: reference param)
struct MoodDeps;        // mood.hpp — the back-portal door's face (fwd: reference param)

// THE S2/S3 BOUNDARY FACE: the patch registry is read across the
// boundary by the occupier commits (host->record_entity via
// find_patch) — the interface trio's registry member.
ActivePatch* find_patch(MachineCtx* c, int32_t gx, int32_t gz);

void evict_patch(MachineCtx* c, uint32_t pi, wgpu::Queue& queue);
void evict_patch_entities(MachineCtx* c, ActivePatch& patch, wgpu::Queue& queue);
void audit_entity_integrity(MachineCtx* c);
uint32_t count_pending_patches(MachineCtx* c);
uint32_t patches_budget_this_frame(MachineCtx* c, const InputState& inputState);

// Keyhole form. CALLER: the transition machine
// (root); OWNER: patch_system.
void teardown_surface(MachineCtx* c, wgpu::Queue& queue,
    TileWorldState& tile_world_state, ThemesState& themes_state);  // was teardown_world; reduced to the surface core

void init_patch_system(MachineCtx* c, TileWorldState& tile_world_state);
// The recenter door (m4): names the hidden regen request — the
// streaming conductor re-evaluates the full window next frame.
// Caller: the radius command (direction/input). DEPS-FORM: the
// driver world holds no MachineCtx — the door takes its
// one organ explicitly (the m3 precedent class, clear_spheres).
void request_recenter(WorldState& ws);
void write_pier(MachineCtx* c, wgpu::Queue& queue, uint32_t slot, const GPUPierInstance& pier);
void clear_pier(MachineCtx* c, wgpu::Queue& queue, uint32_t slot);
void recompute_and_upload_pier_count(MachineCtx* c, wgpu::Queue& queue);
void flush_pier_count(MachineCtx* c, wgpu::Queue& queue);
void mark_patches_for_regen(MachineCtx* c, float min_wx, float min_wz,
    float max_wx, float max_wz,
    int32_t home_gx, int32_t home_gz);
void setup_test_rig_piers(MachineCtx* c, wgpu::Queue queue);
void generate_patch_batch(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count,
    uint32_t stagingOffset = 0);
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
    wgpu::Queue& queue,
    ThemesState& themes_state);
void on_patch_first_generated(MachineCtx* c, uint32_t pi, wgpu::Queue& queue);
void generate_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    uint32_t& patchStagingOffset, bool& tileGridDirty,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps);

// THE CONDUCTOR: the per-frame streaming step.
void stream_patches(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    TileWorldState& tile_world_state, ThemesState& themes_state,
    TileWorldDeps& tile_world_deps, MoodDeps& mood_deps, const InputState& inputState);

} // namespace the_board
} // namespace t7
