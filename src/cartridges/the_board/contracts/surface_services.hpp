#pragma once
#include <cstdint>
#include <cstdio>    // the loud drop (record_entity overflow)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/world_surface.hpp"   // FINITE_RADIUS_MIN/MAX — the pin's dials, and WORLD_LIVE which carries them live (THE_PANEL I U3)

// ─── surface_services.hpp (CONTRACT: the surface's decl tier) ─────
//
// WorldState (the root organ every stratum reads), the patch registry
// vocabulary (ActivePatch + the S2/S3 boundary face), PatchSystemState,
// the budgets + visibility cylinder, and the surface service DECLS.
// The BODIES ride surface/patch_system.hpp, merged at the cohort tail
// — the same-TU late-definition law, named at spawn_services.
//
// The active-patch machine: the patch registry and its lifecycle
// (allocate → spawn → generate), the layer allocator, and the one-shot
// builder (build_world). It was a STREAMED registry with a fourth
// lifecycle stage — evict — and a per-frame conductor pacing all four
// under budgets; a finite world is built once (ONE_SURFACE-I).
//
// SEAM[spine:active-patch-system] the ActivePatch struct, the patches_
//   registry, and find_patch. Cross-module readers: the occupier commits
//   (a body commits iff find_patch resolves its host — machine/
//   entity_pipeline.hpp, bodies/gol_zones.hpp, bodies/ribbon.hpp).
//
//   IT NAMED THINGS THAT WERE NOT THERE, and the sweep says so rather
//   than quietly fixing it: `evict_patch` / `evict_patch_entities` and
//   the entity_refs registry left at ONE_SURFACE-I U3, the family
//   dispatch eviction rows with them, and the ribbon's two-tip late
//   registration at the same unit. It also credited machine/
//   spawn_engine.hpp with `host->record_entity` calls that file has
//   never contained — the callers were always the three named above.
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
// THE PIN (ONE_WORLD-II U5). WorldShape carried `finite` per mood —
// SHAPE_OPEN false, the rooms and the finite field true — and boot wore
// the sunset's open row. U2 moved the fact here without changing it; this
// is the unit that changes it.
//
// THE WORLD IS FINITE, AND NOTHING UNSETS IT. Not a mood, not a key, not
// a param: one constant, read once at every world's birth through the L10
// door. A finite world is a bounded one — the containment clamp
// (finite_bounds_resolve, world.wgsl) is its wall, and the veil stands
// down because a wall defines the boundary where fog used to.
//
// THE OPEN PATHS ARE UNTOUCHED, deliberately. Every `finite_mode`-false
// branch still stands, compiled and correct; they are campaign 3's
// subject, not this one's. A pin is a value, not an excision.
inline constexpr bool WORLD_FINITE = true;

// THE PIN'S DIALS MOVED TO THE WORLD'S OWN HEADER (THE_PANEL I U3).
// FINITE_RADIUS_MIN / FINITE_RADIUS_MAX are declared in
// contracts/world_surface.hpp now, beside the live bank they seat and the
// two dials they bound — one home for what a world is chosen by. They are
// still constexpr, still the DESIGN, and still the CAPACITY the three
// asserts below bind; this file reads them through that include (added at
// the top) and states the capacity law, which is the half that needs
// Dim:: and FC_SEG_* and could never have moved.

// THE WIDEST WORLD THE PIN ALLOWS MUST FIT WHAT DRAWS IT (ONE_SURFACE-I
// U5a). Every patch is one band since U5, so the frustum cull classifies
// the whole draw set into segment A or B by ZONE OVERLAP — and in the
// worst case every patch lands in one of them. 128 entries each against
// (2*4+1)^2 = 81. Stated rather than trusted: the idiom is
// tile_world.hpp's, where upload_tile_grid_now binds the same constant to
// the GPUTileGrid DTO, and the reason is the same — raising the pin
// should fail the BUILD, not the kernel.
//
// (The A/B split's WORDING moved at ONE_SURFACE-II U1 and its ARITHMETIC
// did not. "A world entirely covered by GoL zones fills A, a world with
// none fills B" described the two extremes when overlap was a property
// of eight islands. The automaton is the ground, so every patch that
// carries discrete cells overlaps it and A is the common case rather
// than an extreme — which is a load question for the walk, not a
// capacity one. Both segments still hold 128 and the world still holds
// 81.)
static_assert((2 * FINITE_RADIUS_MAX + 1) * (2 * FINITE_RADIUS_MAX + 1)
              <= FC_SEG_A_BYTES / sizeof(uint32_t),
    "the widest world the pin allows must fit frustum-cull segment A");
static_assert((2 * FINITE_RADIUS_MAX + 1) * (2 * FINITE_RADIUS_MAX + 1)
              <= FC_SEG_B_BYTES / sizeof(uint32_t),
    "and segment B");
static_assert((2 * FINITE_RADIUS_MAX + 1) * (2 * FINITE_RADIUS_MAX + 1)
              <= Dim::MAX_ACTIVE_PATCHES,
    "and the layer pool build_world draws (2R+1)^2 layers from");

// AND THE AUTOMATON'S GRID IS THE GROUND'S, EXACTLY (ONE_SURFACE-II U1).
// Dim::AUTO_GRID_MAX is spelled as a literal because state.hpp stands
// UPSTREAM of this header in the cohort and cannot see FINITE_RADIUS_MAX
// — the same reason the three asserts above live here rather than there.
// This is the identity that makes the literal safe: raise the radius pin
// and the automaton's buffer must grow with it, and the BUILD is where
// that must be discovered.
//
// EQUALITY, NOT `<=`, and the difference matters. The three asserts above
// ask whether the world FITS a capacity; this one asks whether two
// spellings of the SAME NUMBER agree. A grid one cell too large wastes
// memory silently; a grid one cell too small indexes past the end of the
// life buffer on the world's last row, every frame, and Dawn would be
// the only witness. So: equal, or the build stops.
static_assert(Dim::AUTO_GRID_MAX == (2 * FINITE_RADIUS_MAX + 1) * Dim::PATCH_CELL_N,
    "the automaton's grid capacity IS the widest world's cell grid: "
    "(2 * FINITE_RADIUS_MAX + 1) patches per side, PATCH_CELL_N cells per "
    "patch. Raise the radius pin and this literal moves with it");

struct WorldState {
    // ── Seed + dimensions ──
    uint32_t active_seed   = 0;  // world master seed — authored at the composition root from DEMO.seed; mutable for world transitions
    // `active_radius` — the STREAMING window's half-width — stood here.
    // It was PATCH_PREGEN_RADIUS (7) by default, driven by the [ ] keys,
    // and capped to finite_radius for the duration of every conductor
    // call. A finite world's window IS the world, so the two facts were
    // one fact wearing two names; `finite_radius` below is the survivor
    // (ONE_SURFACE-I U2).
    // THE FINITE FACTS, REHOMED (ONE_WORLD-II U2, §1.7). They were
    // WorldShape's — finite, finite_radius_min, finite_radius_max — and
    // the shape died with the moods. TWO of the three survive it: the
    // MODE (a pin, flipped at U5) and the RANGE the world's seed draws a
    // radius from. The range is the pin's DIALS: a radius per world is
    // what the parametric spirit wants, and a pinned world still draws.
    bool     finite_mode   = false;
    uint32_t finite_radius = 2;      // 2 → 5×5 = 25 patches — drawn per world

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
    // `world_young` stood here (RIBBON_6). Youth was an AGE — set when a
    // world began, cleared once when its window was three-quarters built —
    // and it existed so a world being born could be handed a burst the
    // steady state did not get. There are no budgets to select between and
    // no partial world to be young: `build_world` returns a world already
    // whole. Its last reader was `mesh_gen_settled`, which had no callers
    // of its own (ONE_SURFACE-I U6).

    // ── Patch counts (this frame) ──
    uint32_t active_patch_count = 0;
    uint32_t render_patch_count = 0;    // drawn patches (within the live RING — the draw authority)
    // `lod0_patch_count` (the full-mesh subset) and `all_patch_count` (the
    // drawn set plus the pregen ring) stood here. The pregen band left at
    // ONE_SURFACE-I U4 and the LOD split at U5, so both counts named a
    // partition of one set: `render_patch_count` is the whole of it.
    // `all_patch_count` had no reader anywhere in the tree even before that.
    // `entities_culled` stood here. Its writer was
    // `update_entity_draw_visibility`, which has returned a constant 0
    // since the ARCH loop — the only family whose mesh could be zeroed at
    // range — left at ONE_WORLD-I U3. Zero readers (ONE_SURFACE-I U6).

    // ── Dirty flags (deferred GPU uploads) ──
    // `ground_entries_dirty` and `placement_dirty` stood here, deferring
    // `upload_ground_entries` and `dispatch_placement_correction`. Both
    // flags had ZERO readers tree-wide and both functions they name exist
    // nowhere but in comments; the conductor was the last thing still
    // maintaining them (ONE_SURFACE-I U6).
    bool patch_instances_dirty  = true;   // defer LOD sort + upload_patch_instances

    // THE CONTINUOUS-ALLOCATION SCAN STOOD HERE (ONE_SURFACE-I U2), with
    // its raiser flag `alloc_scan_pending` and the two cursors that made
    // its raiser set provably complete — `last_alloc_scan_gx/gz`, the scan
    // BOX's own move, which in a finite world diverged from the pinned
    // window. All three were the streaming conductor's: a finite grid is
    // allocated once, in build_world, and there is no second moment at
    // which demand can appear.

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
    // NEEDS_REGEN stood here — "heightfield stale (new pyramid in range)".
    // `mark_patches_for_regen` was its sole writer and only ever marked
    // patches already GENERATED; its sole caller is `pyramid_post_commit`.
    // At a birth every patch is ALLOCATED or SPAWNED when the pyramids
    // commit, so nothing is GENERATED yet — the lane existed for pyramids
    // committed AFTER some patches had baked, which is the streamed steady
    // state alone. Nothing is allocated after birth now (ONE_SURFACE-I U6).
};

struct ActivePatch {
    int32_t grid_x = 0;
    int32_t grid_z = 0;
    uint32_t layer = 0;
    bool valid = false;
    PatchPhase phase = PatchPhase::ALLOCATED;

    // THE ENTITY-REF REGISTRY STOOD HERE (ONE_SURFACE-I U3): `EntityRef`,
    // `entity_refs[MAX_ENTITY_REFS]`, `entity_ref_count`, `record_entity`
    // with its always-on LOUD DROP, and `unrecord_entity`. It existed for
    // ONE consumer — `evict_patch_entities`, which walked it to evict a
    // dying patch's bodies — and patches do not die in a world that is
    // built once. The guard §1.5 demanded was answered first: no teardown
    // path reads it; every one sweeps by OWNER. `unrecord_entity` already
    // had zero callers before the campaign found it.
};

// ── Dynamic budgets ────────────────────────────────────────────────

// ONE BAKE A FRAME (RIBBON_6). RIBBON_4 was right that the conductor must
// pace by CADENCE and wrong about why: the meter shows streaming's worst
// THE PER-FRAME BUDGETS STOOD HERE (ONE_SURFACE-I U2). SPAWN 2, ALLOC 4,
// EVICT 4, BAKE 1 with a young world's 6, and the rider's 100 wu
// PATCH_LOOK_AHEAD. Every one of them paced a spend across frames, and a
// world that is built once has one frame to pace. RIBBON_6's sentence —
// "a pace must be EVEN, and it must be FAST ENOUGH" — was true of a
// window crossing a plane; the arithmetic that made it true (15 cells a
// crossing, one arriving every 19 frames at the top of the speed dial) is
// arithmetic about a moving window, and the window does not move.
//
// ONE BAKE A FRAME survives as ONE BATCH PER SUBMIT, which is what
// LATTICE_1 actually proved: the queue orders every WriteBuffer ahead of
// the whole command buffer, so two batches on one encoder corrupt the
// first's params. That is a SUBMIT law and it lives in build_world.

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
// the call sites: the tile doors' deps, the sky's deps, the driver's
// intent organ.
struct TileWorldState;  // tile_world.hpp — the tile cache organ (fwd: the lifecycle owner mutates it through the owner doors; the machine face's view is const)
struct TileWorldDeps;   // tile_world.hpp — the tile doors' face (fwd: reference param)
// `struct SkyDeps;` stood here for reset_surface's face. reset_surface has
// not taken it since the conductor's signature left at ONE_SURFACE-I U2.

// THE S2/S3 BOUNDARY FACE: the patch registry is read across the boundary
// by the occupier commits — a body commits iff `find_patch` resolves its
// host. It was also read by the patch-death registry, which left at
// ONE_SURFACE-I U3 with the two evictors declared beside this line.
ActivePatch* find_patch(MachineCtx* c, int32_t gx, int32_t gz);

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
// `request_recenter(WorldState&)` stood here and left at ONE_SURFACE-I U2
// with the conductor it re-armed; `mark_patches_for_regen(MachineCtx*,
// float min_wx, float min_wz, float max_wx, float max_wz, int32_t home_gx,
// int32_t home_gz)` beside it, at U6 — see PatchPhase above for why the
// pyramid re-bake lane is unreachable in a world built once.
// LATTICE_1 — one pass, two dispatches, the whole batch.
void generate_patch_batch(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
    const GPUPatchParams* params, uint32_t count);
GPUPatchParams make_patch_params(MachineCtx* c, int32_t gx, int32_t gz, uint32_t layer);
uint32_t alloc_layer(MachineCtx* c);
// `free_layer` (the pool's return half) left at ONE_SURFACE-I U3 with the
// eviction that was its only caller; `in_render_window` at U2, with the
// window that moved.
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
// THE ONE-SHOT BUILDER (ONE_SURFACE-I U1). Takes the device because it
// owns its own encoders: one batch per submit is LATTICE_1's law, and a
// builder that borrowed the frame's encoder could not honour it.
void build_world(MachineCtx* c, wgpu::Device& device, wgpu::Queue& queue,
    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps);

// `stream_patches` stood here — the per-frame conductor, and the largest
// single verb in the surface. It left at ONE_SURFACE-I U2 with the
// question it answered.

} // namespace the_board
} // namespace t7
