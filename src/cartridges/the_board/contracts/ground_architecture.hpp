#pragma once
// ─── ground_architecture.hpp ─────────────────────────────────────
//
// Canonical registry for the ground query architecture: contributors,
// explicit dependency DAG, and policies.
//
// L3 (docs/LAWS.md) governs this file: CONTRIBUTOR_COUNT,
// POLICY_COUNT, the enum values, and the POLICIES bitmasks are
// mirrored by const values in world.wgsl. Drift means
// query_ground_<policy> evaluates a different contributor set on the
// GPU than CPU placement believed. L9 defines the STATUS: tags the
// rows below carry.
// ─────────────────────────────────────────────────────────────────

#include <cstdint>

namespace t7 {
namespace the_board {

enum ContributorId : uint32_t {
    CONTRIB_TERRAIN_LATTICE   = 0,   // fused into contrib_static_base_at
    CONTRIB_TILE_MODIFIERS    = 1,   // fused into contrib_static_base_at
    CONTRIB_SOLIDS            = 2,   // retired tag (the pier bake left in BATCH G); pyramids carry the solid ground
    CONTRIB_PYRAMIDS          = 3,
    CONTRIB_GOL_ZONES         = 4,   // slow_dynamic
    CONTRIB_TERRAIN_WAVES     = 5,   // deformation_field, global
    CONTRIB_RADIAL_PULSES     = 6,   // deformation_field, global
    CONTRIB_PAWN_AURA         = 7,   // deformation_field, global pawn-centered (two consumer forms: _at_self scalar peak / _at_external grid — see world.wgsl's contributor notes)
    CONTRIB_GOL_SUPPRESSION   = 8,  // deformation_field, consumer-local (subtractive)
    CONTRIB_COUNT             = 9,
};

// These ids are mirrored byte-for-byte as the WGSL POLICY_* consts
// (world.wgsl, above the POLICY_*_MASK block) — manifold_resolve
// switches on them. Keep the two in lock-step (same order/values), as
// with CONTRIB_*. tools/gates/glaw2/run.py checks both mirrors.
enum PolicyId : uint32_t {
    POLICY_BAKED_HEIGHTFIELD    = 0,
    POLICY_FLYER                = 1,
    POLICY_WALKER               = 2,
    POLICY_WALKER_TILT          = 3,   // walker minus the self aura; carries walker's pawn-centered GoL suppression
    POLICY_WALKER_AGENT         = 4,
    POLICY_TERRAIN_RENDER       = 5,   // the fused render set: baked + aura + waves + pulses + GoL-via-the-card (UNIFIED_GROUND_1)
    POLICY_COUNT                = 6,
};

// ═══ DEPENDENCY DAG ══════════════════════════════════════════════

struct ContributorEdge {
    ContributorId from;
    ContributorId to;
};

inline constexpr ContributorEdge CONTRIBUTOR_DAG[] = {
    // STATUS: REALIZED — the composition order of every pyramid-bearing
    // query (contrib_static_base_at + contrib_pyramids_at) and the bake.
    { CONTRIB_TERRAIN_LATTICE,  CONTRIB_PYRAMIDS         },
    { CONTRIB_TILE_MODIFIERS,   CONTRIB_PYRAMIDS         },
    { CONTRIB_SOLIDS,           CONTRIB_PYRAMIDS         },
};
inline constexpr uint32_t CONTRIBUTOR_DAG_EDGE_COUNT =
    sizeof(CONTRIBUTOR_DAG) / sizeof(CONTRIBUTOR_DAG[0]);

// ═══ POLICY DEFINITIONS ══════════════════════════════════════════

struct PolicyDef {
    PolicyId    id;
    const char* name;
    uint32_t    contributors;       // bitmask: bit k set iff contributor k is in the policy
};

// The three fused static-base contributors travel together in every
// policy that wants a landform base.
inline constexpr uint32_t GROUND_STATIC_BASE_MASK =
    (1u << CONTRIB_TERRAIN_LATTICE) |
    (1u << CONTRIB_TILE_MODIFIERS)  |
    (1u << CONTRIB_SOLIDS);

inline constexpr PolicyDef POLICIES[] = {

    // Baked heightfield — cached static ground texture consumed by
    // patch VS interpolation and CPU readbacks.
    // STATUS: REALIZED (no arm) — the baked path is consumed DIRECTLY, not
    // through the dispatcher: fused twin ground_formed_with_complexity
    // feeds the bake, the analytic form is the zone-mesh fallback, and
    // sample_terrain_y_at is the texture variant (which is also what the
    // switch's default arm returns). R8: the switch holds exactly the
    // policies that travel through it, and this id never does — its WGSL
    // constant and its arm went in PRUNING_1 P1 5b. The row stays because
    // the contributor set is real and the DAG still validates it.
    { POLICY_BAKED_HEIGHTFIELD, "baked_heightfield",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS) },

    // Fly-over policy — spheres, cubes, cameras. Includes all global
    // deformation fields so flyers ride pulses and auras.
    // STATUS: REALIZED — camera clamp, sphere orbit clearance, cube
    // hover + clearance (query_ground_flyer). Gradients, where a consumer
    // wants them, come from manifold_resolve's finite difference over this
    // same policy; there is no per-policy gradient function and there is
    // no need for one. (query_ground_flyer_gradient existed with zero
    // callers and was deleted — PRUNING_1 P1 5b.)
    { POLICY_FLYER, "flyer",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA) },

    // Walker — the pawn. Everything flyer includes, plus the
    // consumer-local GoL suppression that flattens the zone under
    // the querying consumer's feet.
    // STATUS: REALIZED — pawn_ground_resolve via query_ground_walker_pair;
    // the live tilt path is terrain_normal_at's 3-tap over walker_tilt.
    { POLICY_WALKER, "walker",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA)
        | (1u << CONTRIB_GOL_SUPPRESSION) },

    // Walker-tilt — walker minus the self aura, used for tilt/normal
    // computation and step-climb decisions. Excludes CONTRIB_PAWN_AURA
    // (the self form is a zero-gradient scalar; excluded so self-centered
    // fields never drive tilt). It CARRIES the same pawn-centered GoL
    // suppression the walker applies (truth-fix: the mask now
    // states what the body computes) — the suppression is flat
    // (supp_factor = 1, zero gradient) within the eps = 0.5 tilt-sample
    // ring, so no slope is manufactured. The pawn still STANDS on full
    // POLICY_WALKER ground; only tilt and step decisions read this policy.
    // STATUS: REALIZED — terrain_normal_at (3-tap) and
    // query_ground_walker_pair's tilt half.
    { POLICY_WALKER_TILT, "walker_tilt",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_GOL_SUPPRESSION) },  // pawn-centered; same suppression walker applies

    // Walker-agent — agents feel the full GoL lift (no suppression).
    // STATUS: REALIZED — agent_post_step ground snap (scalar only).
    { POLICY_WALKER_AGENT, "walker_agent",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA) },

    // Terrain-render — the fused render-side set: the baked heightfield
    // (static base + pyramids) + pawn aura + terrain waves + radial
    // pulses — and GoL, which the mask below SETS; its inline note there
    // carries the how (the card's .a, cell-nearest, pawn-suppressed).
    // This policy has NO query_ground_* function by design — its
    // realizations are hand-fused for per-vertex cost: patch_terrain_vs
    // (the full set; gradients realized there via texture .yz — the
    // heightfield's baked pair + the card's live pair) and shadow_patch_terrain_vs (baked + waves
    // subset, documented at its site). The ~15 entity/painting VS sites
    // that add contrib_terrain_waves_at alone atop the entity ground
    // atlas are sanctioned single-contributor consumptions of this same
    // render set.
    // STATUS: REALIZED (fused-only, no arm) — hand-fused into
    // patch_terrain_vs and shadow_patch_terrain_vs, never dispatched. R8:
    // the row and its contributor set stay and the DAG still validates
    // them; the WGSL constant went in PRUNING_1 P1 5b, having never had an
    // arm to lose.
    { POLICY_TERRAIN_RENDER, "terrain_render",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_GOL_ZONES)   // realized as the card's .a, cell-nearest, pawn-suppressed — UNIFIED_GROUND_1 (DAG: GoL has no ancestors)
        | (1u << CONTRIB_PAWN_AURA) },                  // realized in the fused VS (texture .yz + analytic wave gradient)
};
inline constexpr uint32_t POLICY_COUNT_IN_TABLE =
    sizeof(POLICIES) / sizeof(POLICIES[0]);

static_assert(POLICY_COUNT_IN_TABLE == POLICY_COUNT,
              "POLICIES table must declare one row per PolicyId");

// ═══ COMPILE-TIME DAG-CLOSURE VALIDATION ═════════════════════════
//
// For every contributor in a policy's set, all of its ancestors via
// CONTRIBUTOR_DAG must also be in the set. The predicate ITERATES the
// CONTRIBUTOR_DAG[] table (over CONTRIBUTOR_DAG_EDGE_COUNT) — the
// declared table is load-bearing: adding an edge re-validates every
// policy with no edits here.

#define ASSERT_POLICY_DAG_CLOSED(POLICY_IDX, POLICY_NAME)                       \
    static_assert(                                                              \
        [] {                                                                    \
            for (uint32_t e = 0u; e < CONTRIBUTOR_DAG_EDGE_COUNT; e++) {        \
                const uint32_t mask = POLICIES[POLICY_IDX].contributors;        \
                const bool has_to   = ((mask >> CONTRIBUTOR_DAG[e].to)   & 1u) != 0u; \
                const bool has_from = ((mask >> CONTRIBUTOR_DAG[e].from) & 1u) != 0u; \
                if (has_to && !has_from) { return false; }                      \
            }                                                                   \
            return true;                                                        \
        }(),                                                                    \
        POLICY_NAME ": contributor mask not closed under CONTRIBUTOR_DAG"       \
                    " (a masked contributor is missing a DAG ancestor)")

ASSERT_POLICY_DAG_CLOSED(POLICY_BAKED_HEIGHTFIELD,    "POLICY_BAKED_HEIGHTFIELD");
ASSERT_POLICY_DAG_CLOSED(POLICY_FLYER,                "POLICY_FLYER");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER,               "POLICY_WALKER");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER_TILT,          "POLICY_WALKER_TILT");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER_AGENT,         "POLICY_WALKER_AGENT");
ASSERT_POLICY_DAG_CLOSED(POLICY_TERRAIN_RENDER,       "POLICY_TERRAIN_RENDER");

#undef ASSERT_POLICY_DAG_CLOSED

} // namespace the_board
} // namespace t7
