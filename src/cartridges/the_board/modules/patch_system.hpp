#pragma once
#include <cstdint>
#include "cartridges/the_board/modules/keyhole.hpp"       // Cartridge + wgpu::Queue fwds (the keyhole)

// LOCKSTEP INSURANCE (mirrors orbs.hpp): the encoder handle named in
// the conductor/generation decls, forward-declared in webgpu_cpp.h's form.
namespace wgpu { class CommandEncoder; }

// ─── patch_system.hpp (S2 · HEADER: vocabulary + state + decls) ────
// Born at LADDER-6 (S2 extraction; conductor rides whole by the
// stamped map, R-c): history in audit/LADDER.md.
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
// DTOs) precedes this header in cartridge.hpp.

namespace t7 {
namespace the_board {

// ── World state ────────────────────────────────────────────────────

// ROOT ORGAN (Phase R stamp, R-a): the struct's home is here; the
// instance (world_state_) stays at the composition root.
struct WorldState {
    // ── Seed + dimensions ──
    uint32_t active_seed   = 42;     // world master seed (mutable for world transitions)
    uint32_t active_radius = Dim::PATCH_PREGEN_RADIUS;
    bool     finite_mode   = false;
    uint32_t finite_radius = 2;      // 2 → 5×5 = 25 patches
    uint32_t world_gen     = 0;

    // ── Recenter cursor ──
    int32_t last_center_x = INT32_MAX;  // force full regeneration on first frame
    int32_t last_center_z = INT32_MAX;

    // ── Patch counts (this frame) ──
    uint32_t active_patch_count = 0;
    uint32_t render_patch_count = 0;    // visible patches (within circular VISIBLE_RADIUS)
    uint32_t lod0_patch_count   = 0;    // subset of rendered: within LOD_FULL_RADIUS (full mesh)
    uint32_t all_patch_count    = 0;    // all generated patches (including pre-gen ring)
    uint32_t entities_culled    = 0;    // entities hidden by distance culling this frame

    // ── Dirty flags (deferred GPU uploads) ──
    bool pier_count_dirty       = false;  // defer recompute_and_upload_pier_count
    bool ground_entries_dirty   = true;   // defer upload_ground_entries (true at boot)
    bool patch_instances_dirty  = true;   // defer LOD sort + upload_patch_instances
    bool placement_dirty        = true;   // defer dispatch_placement_correction

    // ── Free-layer pool ──
    uint32_t free_layer_count = Dim::MAX_ACTIVE_PATCHES;
};

// Patch dimensions aliased from Dim:: for local readability
inline constexpr float    PATCH_EXTENT = Dim::PATCH_EXTENT;
inline constexpr uint32_t GRID_RADIUS = Dim::PATCH_GRID_RADIUS;   // inner priority (3 → 7×7)
inline constexpr uint32_t GRID_SIDE = Dim::PATCH_GRID_SIDE;
inline constexpr uint32_t RENDER_RADIUS = Dim::PATCH_RENDER_RADIUS;  // visible radius (5)
inline constexpr uint32_t RENDER_SIDE = Dim::PATCH_RENDER_SIDE;
inline constexpr uint32_t PREGEN_RADIUS = Dim::PATCH_PREGEN_RADIUS; // deep pre-gen buffer (7)
inline constexpr uint32_t MAX_PATCHES = Dim::MAX_ACTIVE_PATCHES;    // 225

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
    static constexpr uint32_t MAX_ENTITY_REFS = 10;
    EntityRef entity_refs[MAX_ENTITY_REFS]{};
    uint32_t entity_ref_count = 0;

    void record_entity(uint32_t family, uint32_t slot) {
        if (entity_ref_count < MAX_ENTITY_REFS) {
            entity_refs[entity_ref_count++] = { family, slot };
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

// ── Visibility cylinder ────────────────────────────────────────────
//
// Grid-based allocation/eviction is unchanged; only the
// draw-list gate uses world-space distance.
inline constexpr float VISIBLE_RADIUS = 5.5f;
inline constexpr float VISIBLE_RADIUS_SQ = VISIBLE_RADIUS * VISIBLE_RADIUS;

inline constexpr float LOD_FULL_RADIUS = 3.5f;
inline constexpr float LOD_FULL_RADIUS_SQ = LOD_FULL_RADIUS * LOD_FULL_RADIUS;
inline constexpr float VISIBILITY_CYLINDER_RADIUS = VISIBLE_RADIUS * PATCH_EXTENT;
inline constexpr float VISIBILITY_CYLINDER_RADIUS_SQ = VISIBILITY_CYLINDER_RADIUS * VISIBILITY_CYLINDER_RADIUS;
inline constexpr float LOD0_CYLINDER_RADIUS = LOD_FULL_RADIUS * PATCH_EXTENT;
inline constexpr float LOD0_CYLINDER_RADIUS_SQ = LOD0_CYLINDER_RADIUS * LOD0_CYLINDER_RADIUS;

// ── Distance-sorted patch scan helper ──

struct PatchCandidate {
    uint32_t idx;
    float dist2;
};

// ═══ MODULE STATE ══════════════════════════════════════════════════

// Instance (patch_system_state_) lives at the composition root.
struct PatchSystemState {
    ActivePatch patches_[MAX_PATCHES]{};
    // Free-list of available texture layers
    uint32_t freeLayerStack_[MAX_PATCHES]{};
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═══════════════════════════════
//
// DEFINED in patch_system.inl (post-class): the machine reaches the
// keyhole for the root organs, the S3 dispatch members (select/place/
// commit, piers), and the GPU wire (gpuState_ / renderer_).

// THE S2/S3 BOUNDARY FACE: the patch registry is read across the
// boundary by the occupier commits (host->record_entity via
// find_patch) — the interface trio's registry member.
ActivePatch* find_patch(Cartridge* c, int32_t gx, int32_t gz);

void evict_patch(Cartridge* c, uint32_t pi, wgpu::Queue& queue);
void evict_patch_entities(Cartridge* c, ActivePatch& patch, wgpu::Queue& queue);
void audit_entity_integrity(Cartridge* c);
uint32_t count_pending_patches(Cartridge* c);
uint32_t patches_budget_this_frame(Cartridge* c);

// Keyhole form (Phase R stamp, R-b). CALLER: the transition machine
// (root); OWNER: patch_system.
void teardown_world(Cartridge* c, wgpu::Queue& queue);

void init_patch_system(Cartridge* c);
void setup_test_rig_piers(Cartridge* c, wgpu::Queue queue);
void generate_patch_batch(Cartridge* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count,
    uint32_t stagingOffset = 0);
GPUPatchParams make_patch_params(Cartridge* c, int32_t gx, int32_t gz, uint32_t layer);
uint32_t alloc_layer(Cartridge* c);
void free_layer(Cartridge* c, uint32_t layer);
bool in_render_window(Cartridge* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz);
float patch_distance_sq(float px, float pz, float origin_x, float origin_z, float half);
template<typename Pred>
uint32_t collect_sorted_patches(Cartridge* c, PatchCandidate* out,
    float pawn_wx, float pawn_wz, Pred&& pred, bool nearest_first);
bool in_priority_window(Cartridge* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz);
void spawn_selected_patches(Cartridge* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::Queue& queue);
void on_patch_first_generated(Cartridge* c, uint32_t pi, wgpu::Queue& queue);
void generate_selected_patches(Cartridge* c, const PatchCandidate* candidates, uint32_t count,
    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    uint32_t& patchStagingOffset, bool& tileGridDirty);

// THE CONDUCTOR (Phase R stamp, R-c): the per-frame streaming step.
void stream_patches(Cartridge* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
