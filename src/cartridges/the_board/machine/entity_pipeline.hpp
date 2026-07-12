#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/keyhole.hpp"       // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── entity_pipeline.hpp (S3 · HEADER: vocabulary + decls) ─────────
// Converted at LADDER-6 (the LAST class-body citizen — §1's
// completion sentence executes this commit): history in
// audit/LADDER.md.
//
// Generic entity lifecycle for the cookie-cutter families: the
// three-phase verbs (select → place → commit) driven per family by
// traits + adapter rows; the welded four blocks (column, antenna,
// pyramid, arch) live in the impl. The module owns no state — the
// queues it fills are spawn_engine's.
//
// SEAM[entity_pipeline:K1] tier sampling profile + extras live as a
//   single per-family TierRow struct in the impl. Single source of
//   truth — no converters, no derived tables.
//
// Depends on cohort include order: entity_types.hpp (traits/adapter/
// instance vocabulary), entities.hpp (props/configs/palettes + the
// tier enums), state.hpp (GPU mesh params), mood.hpp (MOOD_TABLE /
// PORTAL_DENSITY / portal doors).

namespace t7 {
namespace the_board {

// ─── Indoor Rescale Helper ───────────────────────────────────────
//
//   • Columns: HEIGHT is set to ceiling_height exactly, and every
//     other length param scales by the same ratio so proportions
//     hold. The capital meets the ceiling — the column reads as
//     part of the room's architecture, not a freestanding object.
//
// Param indices below are hand-curated per family — only LENGTH
// dimensions get scaled, never ratios (TAPER, ENTASIS, ASPECT...),
// counts (BASE_LAYERS, RIBS, ARM_COUNT...), or angles (LEAN_DIR,
// FROND_DROOP...). Adding a new eligible family means picking which
// pattern it follows, declaring its own
// <family>_apply_indoor_rescale, and registering it in the adapter.
// Property index 7777u is reserved for the rescale-target hash
// (no other family uses it).
//
// SEAM[entity_pipeline:rescale-per-family] DONE — was a free-function
//   switch on family_id; lifted to per-family adapter slot during
//   Pass 7 of the modularity rollout.

//
// Column does NOT use this helper — its policy is "snap to ceiling
// exactly" rather than "roll a target ratio."
//
// The templated array reference avoids needing <initializer_list>;
// each caller passes a constexpr uint32_t array of param indices.
template<size_t N>
inline void rescale_to_rolled_target(EntityInstance& inst, float ceiling_h,
    float target_lo, float target_hi, float current_h,
    const uint32_t (&params_to_scale)[N]) {
    if (current_h <= 1e-3f) return;
    constexpr uint32_t RESCALE_TARGET_PROP = 7777u;
    const float t = cpu_hash_f(inst.seed, RESCALE_TARGET_PROP);
    const float target_h = ceiling_h * (target_lo + (target_hi - target_lo) * t);
    const float scale = target_h / current_h;
    for (size_t i = 0; i < N; i++) inst.params[params_to_scale[i]] *= scale;
}

// ── Arch vocabulary (graduated: read by entities.inl's recipes and
//    mood.inl's portal/doorway geometry) ──

struct ArchIdx {
    static constexpr uint32_t SPAN         = 0;  // full span (halved in compute_solid_half)
    static constexpr uint32_t RISE         = 1;
    static constexpr uint32_t DEPTH        = 2;
    static constexpr uint32_t THICKNESS    = 3;
    static constexpr uint32_t PIER_HEIGHT  = 4;
    static constexpr uint32_t PIER_PADDING = 5;
    static constexpr uint32_t EDGE_BLEND   = 6;
    static constexpr uint32_t COUNT        = 7;
};

// params[] order MUST match ARCH_PARAM_DEFS:
//   [0]SPAN [1]RISE [2]DEPTH [3]THICKNESS [4]PIER_HEIGHT [5]PIER_PADDING [6]EDGE_BLEND
//
// Note: name is ArchTierRow, not ArchTier — the enum class ArchTier
// (DOORWAY/STANDARD/MONUMENTAL) already occupies that name in entities.hpp.
struct ArchTierRow {
    TierProfile profile;
    float       color_override;
    float       burial;
    uint32_t    segs_u;
    uint32_t    segs_v;
};

inline constexpr ArchTierRow ARCH_TIERS[] = {
    /* DOORWAY    */ {
        { 0.50f, 0.0f, { {12.0f, 2.4f}, {12.0f, 2.4f}, {4.5f, 0.9f}, {1.2f, 0.18f}, {1.5f, 0.9f}, {0.9f, 0.3f}, {0.9f, 0.15f} }},
        0.15f, 0.20f, 16, 4
    },
    /* STANDARD   */ {
        { 0.15f, 0.0f, { {50.0f, 15.0f}, {42.0f, 7.0f}, {5.6f, 1.1f}, {1.4f, 0.21f}, {5.6f, 2.1f}, {0.7f, 0.3f}, {0.7f, 0.14f} }},
        0.25f, 0.20f, 32, 8
    },
    /* MONUMENTAL */ {
        { 0.15f, 0.0f, { {60.0f, 10.0f}, {80.0f, 12.0f}, {10.0f, 2.0f}, {2.5f, 0.30f}, {8.0f, 2.5f}, {1.0f, 0.3f}, {0.8f, 0.15f} }},
        0.35f, 0.20f, 48, 12
    },
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═══════════════════════════════
//
// DEFINED in entity_pipeline.inl (post-class): the verbs reach the
// keyhole for c->mood_state_ / c->world_state_ and route through the
// spawn-engine services; the family adapters write c->entities_state_
// and the GPU wire.

bool generic_select(Cartridge* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    int32_t gx, int32_t gz,
    EntityInstance& inst);
bool generic_place(Cartridge* c,
    const EntityFamilyTraits& traits,
    EntityInstance& inst);
void generic_commit(Cartridge* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    const EntityInstance& inst,
    wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
