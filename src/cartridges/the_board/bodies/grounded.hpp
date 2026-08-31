#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // Dim::*, GPUPyramidArray, wgpu
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT, WORLD_DRAW_LIVE
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"     // queue types (the dispatch funnels' signatures)

// ─── grounded.hpp (HEADER: the grounded-seven registry) ──────────
//
// Vocabulary for the grounded entity families that share the generic dispatch pipeline.
//
// SEAM[entities:P10] this block is the canonical home of pattern P10
//   (per-family vocabulary block). Two family applications follow.
//   Each block has the same structural template: TierEnum / Color
//   palette / Config / Prop registry / Active tracking. Don't fight
//   the cookie-cutter — it's intentional specificity per family.
// SEAM[entities:taxonomy] this block holds vocabulary for the seven
//   grounded families that share machine/entity_pipeline.hpp. Sphere/Cube
//   vocabulary is in floaters.hpp; Ribbon and GoL are complete
//   subsystems in their own files.
// Tier sampling profiles + extras live as a single per-family TierRow
//   struct in machine/entity_pipeline.hpp (single source of truth, no
//   converters, no derived tables). The tier enum classes (ArchTier,
//   ArchTier, PyramidTier) stay here — they're indexing semantics, not data.
// ─────────────────────────────────────────────────────────────────

#include <cstdint>   // (impl, merged)

namespace t7 {
namespace the_board {

// ═══ SHARED CONSTANTS ════════════════════════════════════════════

// STATUS: LATENT[unused] — zero callers in the tree;
// declared as the CPU mirror of WGSL PAWN_HEIGHT. Kept per flag-don't-
// delete; revive-or-delete when the pawn-height coupling is next worked.
inline constexpr float PAWN_HEIGHT_UNITS = 1.5f;     // matches WGSL PAWN_HEIGHT

// ═══ THE SPREAD LAW (MOSAIC_2) ═══════════════════════════════════
//
// The terrain's sorted-variance shape at the body's rate: raffle a
// median, then raffle how far THIS body may sit from it. A fixed ±3%
// puts every body the same distance from its color, which is why a
// hundred plain arches read as one stone. A raffled spread makes some
// bodies nearly flat and others strongly mottled — that is what
// "varied" means, and it is the terrain's own law (var = base +
// raw·span) at a smaller number.
//
// The span sits well under the terrain's 0.23 deliberately: a region is
// a field the eye averages across, while a body wears its whole spread
// at once. 0.16 is a body reading as its palette color in a different
// light; 0.23 is a body reading as a different color.
inline constexpr float ENTITY_SPREAD_BASE = 0.02f;
inline constexpr float ENTITY_SPREAD_SPAN = 0.16f;
// ═══ VOCABULARY: ARCH ════════════════════════════════════════════

enum class ArchTier : uint32_t {
    DOORWAY = 0,   // human-scale passage
    STANDARD = 1,   // the arch we started with
    MONUMENTAL = 2,   // cathedral-scale gateway
    COUNT = 3
};

// ── Color Palette ────────────────────────────────────────────────
// A QUARRIED vocabulary (MOSAIC_2d), deliberately distinct from the two
// sets already in the tree: AGENT_PALETTE (bodies/agents.hpp) is
// painted (sky, coral, lavender), MOSAIC_MEDIANS is ceramic. Index 0 is
// the incumbent grey
// unchanged, so every existing arch that rolled palette keeps a color
// it could already have had. ARCH_SANDSTONE_BASE is deliberately NOT
// duplicated here — it has one home.
inline constexpr float ARCH_PALETTE[][3] = {
    { 0.82f, 0.80f, 0.78f },   // 0: grey stone   (the incumbent)
    { 0.88f, 0.85f, 0.76f },   // 1: limestone
    { 0.28f, 0.28f, 0.30f },   // 2: basalt
    { 0.72f, 0.42f, 0.30f },   // 3: terracotta
    { 0.80f, 0.62f, 0.32f },   // 4: ochre
    { 0.44f, 0.62f, 0.56f },   // 5: verdigris
    { 0.48f, 0.30f, 0.34f },   // 6: porphyry
    { 0.92f, 0.90f, 0.86f },   // 7: chalk
};
inline constexpr uint32_t ARCH_PALETTE_COUNT = 8;
inline constexpr float ARCH_SANDSTONE_BASE[3] = { 0.75f, 0.68f, 0.60f };

// ── Spawn Configuration ──────────────────────────────────────────
struct ArchConfig {
    static constexpr float SPAWN_CHANCE = 0.050f;
    // Per-mood spawn multiplier (Bayesian: prior × mood_factor × adjacency_factor)
    // Position jitter within patch (fraction of Dim::PATCH_EXTENT)
    static constexpr float POSITION_JITTER = 0.35f;
};

// ── Property Index Registry ──────────────────────────────────────
// Named constants for cpu_hash_f(seed, prop) → deterministic draw.
// 600-series: decorrelated from ribbons (400) and galleries (500).
struct ArchProp {
    static constexpr uint32_t SPAWN_ROLL = 600u;
    static constexpr uint32_t POSITION_X = 601u;
    static constexpr uint32_t POSITION_Z = 602u;
    static constexpr uint32_t ROTATION = 603u;
    static constexpr uint32_t TIER = 604u;
    static constexpr uint32_t SPAN = 610u;
    static constexpr uint32_t RISE = 611u;
    static constexpr uint32_t DEPTH = 612u;
    static constexpr uint32_t THICKNESS = 613u;
    static constexpr uint32_t PIER_HEIGHT = 614u;
    static constexpr uint32_t PIER_PADDING = 615u;
    static constexpr uint32_t EDGE_BLEND = 616u;
    static constexpr uint32_t COLOR_OVER = 620u;
    static constexpr uint32_t COLOR_VAR_R = 621u;
    static constexpr uint32_t COLOR_VAR_G = 622u;
    static constexpr uint32_t COLOR_VAR_B = 623u;
    static constexpr uint32_t MOSAIC_ROLL = 650u;   // MOSAIC_2: the tier's ceramic roll
    static constexpr uint32_t MOSAIC_SEED = 651u;   // MOSAIC_1: 16-bit paint identity
    static constexpr uint32_t COLOR_SPREAD = 652u;  // MOSAIC_2: how far THIS body sits from its median
};

// ── Active Arch Tracking ─────────────────────────────────────────
struct ActiveArch {
    int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
    int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
    bool active = false;
    bool draw_visible = true;    // false = mesh zeroed for distance culling

    // Geometry (for mesh rebuild)
    float world_x = 0.0f, world_z = 0.0f;
    float rotation = 0.0f;               // facing angle (radians)
    float half_span = 0.0f;
    float total_height = 0.0f;            // pier_height + rise
    ArchTier tier = ArchTier::DOORWAY;

    // Cached mesh parameters (set at spawn, read by rebuild)
    float depth = 0.0f;
    float thickness = 0.0f;
    float rise = 0.0f;                    // catenary height above piers
    float pier_height = 0.0f;
    float burial = 0.0f;
    uint32_t segs_u = 16, segs_v = 4;
    float col_r = 0.75f, col_g = 0.68f, col_b = 0.60f;

    // Placement (computed once at spawn, immutable)
    float cached_ground_y = 0.0f;         // absolute ground Y for VS offset

    uint32_t position_hash = 0;           // for crossing_seed

    uint32_t mosaic_seed = 0;   // MOSAIC_1 — frozen at spawn; 0 = plain
};

// ═══ VOCABULARY: PYRAMID ═════════════════════════════════════════

enum class PyramidTier : uint32_t {
    OBELISK = 0,     // tall narrow marker, pointed apex
    TEMPLE = 1,      // medium, truncated platform top
    COLOSSUS = 2,    // massive landmark, slight or no truncation
    COUNT = 3
};

// ── Color Palette ────────────────────────────────────────────────
// Sandstone base only. All pyramids derive color from a single base.
inline constexpr float PYRAMID_SANDSTONE_BASE[3] = { 0.80f, 0.72f, 0.58f };
inline constexpr float PYRAMID_SANDSTONE_VARIANCE = 0.05f;

// ── Spawn Configuration ──────────────────────────────────────────
struct PyramidConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    static constexpr float POSITION_JITTER = 0.25f;
};

// ── Property Index Registry ──────────────────────────────────────
struct PyramidProp {
    static constexpr uint32_t SPAWN_ROLL = 800u;
    static constexpr uint32_t POSITION_X = 801u;
    static constexpr uint32_t POSITION_Z = 802u;
    static constexpr uint32_t ROTATION = 803u;
    static constexpr uint32_t TIER = 804u;
    static constexpr uint32_t HEIGHT = 810u;
    static constexpr uint32_t BASE_HALF = 811u;
    static constexpr uint32_t ASPECT = 812u;
    static constexpr uint32_t TRUNCATION = 813u;
    static constexpr uint32_t EDGE_BLEND = 814u;
    static constexpr uint32_t COLOR_OVER = 820u;
    static constexpr uint32_t COLOR_VAR_R = 821u;
    static constexpr uint32_t COLOR_VAR_G = 822u;
    static constexpr uint32_t COLOR_VAR_B = 823u;
};

// ── Active Pyramid Tracking ──────────────────────────────────────
struct ActivePyramid {
    int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
    int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
    bool active = false;
    // Cached color (set at spawn, read by rebuild)
    float col_r = 0.80f, col_g = 0.72f, col_b = 0.58f;

    // Placement (computed once at spawn, immutable)
    float cached_ground_y = 0.0f;         // absolute base Y for VS offset
};

// ═══ ENTITIES MODULE STATE ════════════════════════════════════════

struct EntitiesState {
    // ── Arch ─────────────────────────────────────────────────────
    ActiveArch arches[Dim::MAX_ARCH_INSTANCES]{};
    bool       arch_mesh_gen_pending = false;
    float      arch_mesh_gen_since = -1.0f;   // the settle's stamp (PANORAMA_1)

    // ── Pyramid ──────────────────────────────────────────────────
    ActivePyramid   pyramids[Dim::MAX_PYRAMID_INSTANCES]{};
    GPUPyramidArray cpu_pyramids{};                 // CPU mirror for heightfield baking
};

// ═══ MESH-GEN PREPARERS — DECLARATIONS ════════════════════════════

bool prepare_arch_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue);

// ═══ THE EVICTORS — DECLARATIONS ═══════════════════════════════════

void evict_pyramid(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void evict_arch(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void teardown_entities(MachineCtx* c, wgpu::Queue& queue);

// ═══ IMPL:
// bodies deref EntitiesState(own) + World/Mood/GPU via MachineCtx; no
// Cartridge. COHORT: after contracts/spawn_services.hpp (generic_* +
// preamble DECLS — the machine bodies ride the cohort tail) +
// patch_system.hpp (WorldState, find_patch) + mood.hpp.

// ═══ MESH-GEN PREPARERS ═══════════════════════════════════════════

// THE SETTLE (PANORAMA_1). A family regenerates at most once per
// MESH_GEN_SETTLE_FRAMES outside a world's birth. A crossing raises
// `pending` on several consecutive frames; without this each raise was a
// whole-family rebake (4–8 ms on the floor device), so a burst of spawns
// bought a burst of firings. What arrives late arrives at the ring's edge,
// materializing through the icing, where a ~130 ms delay is invisible; what
// leaves late leaves beyond the ring, where it is already veiled — eviction
// is radius-driven and fires outside the render window by construction.
//
// THE BIRTH BURST BYPASSES IT, and `world_young` is the whole signal: it
// boots true, `reset_surface` re-raises it on every rebirth BEFORE
// teardown_entities runs in the same block, and `request_recenter` raises it
// on a radius change. So a boot, a portal and a re-centre all regenerate at
// once — behind the veil or behind the fade — and the settle governs only
// the steady world, which is the only place it was ever paying for anything.
//
// STAMPED AT FIRST SIGHT, NOT AT THE RAISE. The preparers run every frame, so
// the first frame that observes a raise is the raise's own frame or the next;
// against a settle of eight that is exact enough, and it puts the stamp in ONE
// place instead of at twenty raise sites, where a new raiser could forget it.
//
// PACED IN SECONDS, BECAUSE THE CLAIM WAS ALWAYS A TIME CLAIM (WRAP_0 U6).
// PANORAMA_1 wrote this as eight FRAMES and argued it in milliseconds — "a
// ~130 ms delay is invisible" — which are the same number only at 60 Hz. At
// `?pace=2` a frame is 33 ms and the window silently became ~266 ms. The
// constant now reads as what the argument always was, and no pace can double
// it.
inline constexpr float MESH_GEN_SETTLE_S = 0.133f;

inline bool mesh_gen_settled(bool& pending, float& since, const TimeState& ts,
                             const WorldState& ws) {
    if (!pending) return false;
    if (since < 0.0f) since = ts.seconds;
    if (!ws.world_young && ts.seconds - since < MESH_GEN_SETTLE_S) return false;
    pending = false;
    since = -1.0f;
    return true;
}


inline bool prepare_arch_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!mesh_gen_settled(es.arch_mesh_gen_pending, es.arch_mesh_gen_since,
                          c->time_state_, c->world_state_)) return false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (es.arches[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_arch_index_count(anyActive
        ? (maxSlot + 1) * Dim::AMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

// ═══ THE EVICTORS ═════════════════════════════════════════════════

inline void evict_pyramid(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::PYRAMID, slot);   // the hand that claims is the hand that frees
    self->entities_state_.cpu_pyramids.instances[slot] = GPUPyramidInstance{};
    self->entities_state_.pyramids[slot].active = false;
    self->world_state_.ground_entries_dirty = true;

    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        if (self->entities_state_.pyramids[i].active) max_idx = i + 1;
    }
    self->entities_state_.cpu_pyramids.count = max_idx;
    self->gpuState_.upload_pyramids(queue, self->entities_state_.cpu_pyramids);
}

inline void evict_arch(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::ARCH, slot);   // the hand that claims is the hand that frees
    self->entities_state_.arches[slot].active = false;
    { GPUArchMeshParams ep{}; self->gpuState_.upload_arch_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.arch_mesh_gen_pending = true;
}

// ─── Teardown (owner verb) ────────────────────────────────────────
// The grounded families' half of the world-teardown sweep — CPU slot
// clears + GPU param-slot clears + mesh-gen re-arm, two families in
// their one organ. UNGATED by design: the two families share this
// organ, and per-family gating buys nothing (empty arrays clear to
// empty).
inline void teardown_entities(MachineCtx* c, wgpu::Queue& queue) {
    // Arches
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        c->entities_state_.arches[i] = ActiveArch{};
    }
    c->gpuState_.set_arch_index_count(0);
    // Clear all arch mesh gen param slots
    {
        GPUArchMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
            c->gpuState_.upload_arch_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.arch_mesh_gen_pending = true;
    }

    // Pyramids
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        c->entities_state_.pyramids[i] = ActivePyramid{};
    }
    c->entities_state_.cpu_pyramids = GPUPyramidArray{};
    c->gpuState_.upload_pyramids(queue, c->entities_state_.cpu_pyramids);
}

} // namespace the_board
} // namespace t7
