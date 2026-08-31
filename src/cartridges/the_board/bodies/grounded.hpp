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
//   converters, no derived tables). The tier enum class (PyramidTier)
//   stays here — it is indexing semantics, not data.
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
// hundred plain pyramids read as one stone. A raffled spread makes some
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

    // ── Pyramid ──────────────────────────────────────────────────
    ActivePyramid   pyramids[Dim::MAX_PYRAMID_INSTANCES]{};
    GPUPyramidArray cpu_pyramids{};                 // CPU mirror for heightfield baking
};

// ═══ MESH-GEN PREPARERS — DECLARATIONS ════════════════════════════


// ═══ THE EVICTORS — DECLARATIONS ═══════════════════════════════════

void evict_pyramid(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
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


// ─── Teardown (owner verb) ────────────────────────────────────────
// The grounded families' half of the world-teardown sweep — CPU slot
// clears + GPU param-slot clears + mesh-gen re-arm, two families in
// their one organ. UNGATED by design: the two families share this
// organ, and per-family gating buys nothing (empty arrays clear to
// empty).
inline void teardown_entities(MachineCtx* c, wgpu::Queue& queue) {

    // Pyramids
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        c->entities_state_.pyramids[i] = ActivePyramid{};
    }
    c->entities_state_.cpu_pyramids = GPUPyramidArray{};
    c->gpuState_.upload_pyramids(queue, c->entities_state_.cpu_pyramids);
}

} // namespace the_board
} // namespace t7
