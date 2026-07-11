#pragma once
// ─── ground_architecture.hpp ─────────────────────────────────────
//
// Canonical registry for the ground query architecture:
// contributors, explicit dependency DAG, and policies. Single source
// of truth on the C++ side; world.wgsl mirrors the same ids and
// per-policy bitmasks as `const` values so shader code can refer to
// them by symbol. Keep the two in sync.
//
// The design rationale now lives in the code: CONTRIBUTOR_DAG /
// POLICIES[] below, and the architecture overview comment at the top
// of the ground section in world.wgsl, which describes contributor
// classes, extension patterns, and the fused-inline hot paths that
// bypass the query API.
//
// ┌─── Public surface (consumed by other files) ────────────────────┐
// │                                                                  │
// │  Identifiers:                                                    │
// │    ContributorId            — stable id per contributor          │
// │    PolicyId                 — stable id per policy               │
// │    CONTRIB_COUNT, POLICY_COUNT                                   │
// │                                                                  │
// │  Tables:                                                         │
// │    CONTRIBUTOR_DAG[]        — placement-order edges              │
// │    POLICIES[]               — per-policy contributor masks       │
// │                                                                  │
// │  Mask helpers:                                                   │
// │    GROUND_STATIC_BASE_MASK  — fused static-base trio             │
// │                                                                  │
// │  Validation (compile-time only — no runtime symbols exported):   │
// │    DAG closure asserts run at the bottom of this file.           │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// ── Vocabulary ──────────────────────────────────────────────────
//
// A *contributor* is a named source of height (or subtractive
// displacement) at a world XZ. A *policy* is a named filter over the
// contributor set: each consumer declares its policy, and a single
// `query_ground_<policy>` function in world.wgsl evaluates the
// policy-selected contributor sum.
//
// ── STATUS convention (TER-2 alignment) ─────────────────────────
//
// Every policy row and declared capability carries an explicit status
// marker, so declaration-vs-realization is visible instead of implied:
//
//   STATUS: REALIZED      — wired and live (the consumer is cited).
//   STATUS: LATENT[name]  — capability with a plausible future; kept
//                           and tagged; revive-or-rewire when this
//                           region is next worked (the DRIVERLESS
//                           pattern, generalized).
//   STATUS: INTENT        — declared, zero realization yet; kept in
//                           the declaration with the status stated
//                           (honest futures, not lies).
//
// The registry says what is realized rather than shrinking to it.
//
// ── What this file declares ─────────────────────────────────────
//
//   ContributorId         Stable id per contributor (0..CONTRIB_COUNT).
//                         Drives bit positions in policy masks and
//                         endpoints in CONTRIBUTOR_DAG edges.
//
//   PolicyId              Stable id per policy (0..POLICY_COUNT).
//
//   CONTRIBUTOR_DAG       Explicit dependency edges among
//                         static_landform contributors. "A → B" means
//                         A is placed beneath B (B's eval composes on
//                         top of A). Deformation fields are not in
//                         the DAG — they act orthogonally.
//
//   POLICIES              Per-policy row: id + name + contributor
//                         bitmask + gradient_supported flag. Indexed
//                         by PolicyId (verified by a count
//                         static_assert).
//
//   policy mask helpers   GROUND_STATIC_BASE_MASK groups the three
//                         fused static-base contributors (lattice,
//                         tile_modifiers, solids) for readability.
//
// ── Compile-time validation ─────────────────────────────────────
//
// A policy's contributor set must be *closed under CONTRIBUTOR_DAG*:
// for every edge {from, to}, if the mask contains `to` it must also
// contain `from`. This prevents policies like "pyramids without
// terrain_lattice" from compiling.
//
// The check ITERATES CONTRIBUTOR_DAG[] itself (over
// CONTRIBUTOR_DAG_EDGE_COUNT), so the edge table is load-bearing:
// adding an edge to the table re-validates every policy with no
// further edits here. It is expressed as an immediately-invoked
// constexpr lambda inside each static_assert rather than a
// constexpr function. NOTE (LADDER-1 c2): the class-body constraint
// that ORIGINALLY forced the lambda — a class-body static_assert
// cannot call a member function of the still-incomplete enclosing
// class — has DISSOLVED now that this is a file-scope header; the
// restyle to a namespace constexpr function is a NAMED LATER STAGE,
// kept AS-IS here for a clean bisection. The lambda's body is parsed
// in place and reads only the already-initialized inline constexpr
// tables above it.
//
// Every policy runs the check for every DAG edge at compile time;
// adding a new policy means adding one ASSERT_POLICY_DAG_CLOSED
// invocation at the bottom of this file. Adding an edge means adding
// one CONTRIBUTOR_DAG row — the assert iterates the table.
//
// A REAL HEADER at file scope (LADDER-1 c2) — no longer included inside
// the Cartridge class body. Namespace t7::the_board (the cartridge's own).
// Depends on: <cstdint> only (pure enum + table definitions + macro checks).
//
// SEAM[ground_architecture:P9] header-style file: pure declarations
//   (enums + tables) + compile-time validation macros, zero runtime
//   logic, no class-member coupling — its header-style nature now
//   REALIZED as a real file-scope header (LADDER-1 c2). Same family as
//   entity_types, coupling/trajectory.hpp, seed_utils as P9 instances.
//   The ASSERT_POLICY_DAG_CLOSED macro pattern is the unique
//   contribution of this file — compile-time relational integrity
//   over a registry table.
// SEAM[ground_architecture:contract] CONTRIBUTOR_COUNT, POLICY_COUNT,
//   the contributor enum values, and POLICIES bitmasks must mirror
//   identical const values in world.wgsl. Drift would mean
//   query_ground_<policy> evaluates a different contributor set on
//   GPU than what CPU placement believed. Same family as agents:L2
//   and seed_utils:contract.
// ─────────────────────────────────────────────────────────────────

#include <cstdint>

namespace t7 {
namespace the_board {

enum ContributorId : uint32_t {
    CONTRIB_TERRAIN_LATTICE   = 0,   // fused into contrib_static_base_at
    CONTRIB_TILE_MODIFIERS    = 1,   // fused into contrib_static_base_at
    CONTRIB_SOLIDS            = 2,   // piers, ramps; fused into contrib_static_base_at
    CONTRIB_PYRAMIDS          = 3,
    CONTRIB_PAINTINGS_BASES   = 4,   // STATUS: INTENT — 0.0 stub (contrib_paintings_base_at); in no policy mask
    CONTRIB_VEGETATION_BASES  = 5,   // STATUS: INTENT — 0.0 stub (contrib_vegetation_base_at); in no policy mask
    CONTRIB_GOL_ZONES         = 6,   // slow_dynamic
    CONTRIB_TERRAIN_WAVES     = 7,   // deformation_field, global
    CONTRIB_RADIAL_PULSES     = 8,   // deformation_field, global
    CONTRIB_PAWN_AURA         = 9,   // deformation_field, global pawn-centered (two consumer forms: _at_self scalar peak / _at_external grid — see world.wgsl's contributor notes)
    CONTRIB_GOL_SUPPRESSION   = 10,  // deformation_field, consumer-local (subtractive)
    CONTRIB_COUNT             = 11,
};

enum PolicyId : uint32_t {
    POLICY_PLACEMENT_PYRAMID    = 0,
    POLICY_PLACEMENT_PAINTING   = 1,
    POLICY_PLACEMENT_VEGETATION = 2,
    POLICY_BAKED_HEIGHTFIELD    = 3,
    POLICY_FLYER                = 4,
    POLICY_WALKER               = 5,
    POLICY_WALKER_TILT          = 6,   // walker minus the self aura; carries walker's pawn-centered GoL suppression
    POLICY_WALKER_AGENT         = 7,
    POLICY_CELESTIAL            = 8,
    POLICY_TERRAIN_RENDER       = 9,   // the fused render set: baked + aura + waves + pulses (no GoL)
    POLICY_COUNT                = 10,
};

// ═══ DEPENDENCY DAG ══════════════════════════════════════════════
//
// A → B means "A is placed beneath B" — B's evaluation composes on
// top of A. Edges exist only among static_landform contributors.
// Deformation fields are orthogonal to the DAG (applied additively
// across the static+dynamic sum).
//
// Note: LATTICE, TILE_MODIFIERS, SOLIDS fuse into contrib_static_base_at
// at the shader level; the edges are still declared here so policy
// closure validation works on logical contributor ids.

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
    // STATUS: INTENT — endpoints are 0.0 stubs today; real composition
    // pending paintings/vegetation bases. Kept: closure over them is
    // well-defined (currently vacuous — no mask includes the stubs).
    { CONTRIB_PYRAMIDS,         CONTRIB_PAINTINGS_BASES  },
    { CONTRIB_SOLIDS,           CONTRIB_PAINTINGS_BASES  },
    { CONTRIB_SOLIDS,           CONTRIB_VEGETATION_BASES },
};
inline constexpr uint32_t CONTRIBUTOR_DAG_EDGE_COUNT =
    sizeof(CONTRIBUTOR_DAG) / sizeof(CONTRIBUTOR_DAG[0]);

// ═══ POLICY DEFINITIONS ══════════════════════════════════════════
//
// Each policy declares its contributor set as a bitmask indexed by
// ContributorId, plus a flag for gradient evaluation support.

struct PolicyDef {
    PolicyId    id;
    const char* name;
    uint32_t    contributors;       // bitmask: bit k set iff contributor k is in the policy
    bool        gradient_supported;
};

// The three fused static-base contributors travel together in every
// policy that wants a landform base.
inline constexpr uint32_t GROUND_STATIC_BASE_MASK =
    (1u << CONTRIB_TERRAIN_LATTICE) |
    (1u << CONTRIB_TILE_MODIFIERS)  |
    (1u << CONTRIB_SOLIDS);

inline constexpr PolicyDef POLICIES[] = {
    // Placement policies — spawn-time Y correction. No deformation
    // fields (placement should be stable against animated terrain).
    //
    // STATUS: LATENT[policy-surface] (all three placement rows) — the
    // declared placement queries have no live caller; the live Y path is
    // compute_entity_placement's baked-heightfield hybrid (world.wgsl
    // §compute_entity_placement). These rows are the future interface's
    // landing sites — rewiring candidates when placement moves onto the
    // policy API. NOTE where a declared mask EXCLUDES a contributor the
    // live baked path includes (pyramid / vegetation exclude
    // CONTRIB_PYRAMIDS; the bake caches it): the declared-intent
    // exclusion is a possible future aesthetic ruling (Jean's); changing
    // behavior is a BEHAVIOR stage, not a truth-fix.
    { POLICY_PLACEMENT_PYRAMID, "placement_pyramid",
      GROUND_STATIC_BASE_MASK,               // declared intent: "pyramids don't see themselves"; live baked path includes pyramids
      /*gradient=*/false },

    { POLICY_PLACEMENT_PAINTING, "placement_painting",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES),       // paintings sit on current GoL (preserves pre-refactor behavior)
      /*gradient=*/false },                 // realized-of-record: the documented baked+analytic-GoL hybrid matches this set

    { POLICY_PLACEMENT_VEGETATION, "placement_vegetation",
      GROUND_STATIC_BASE_MASK,              // declared intent: "trees don't stand on pyramids"; live baked path includes pyramids
      /*gradient=*/false },

    // Baked heightfield — cached static ground texture consumed by
    // patch VS interpolation and CPU readbacks.
    // STATUS: REALIZED — fused twin ground_formed_with_complexity feeds
    // the bake; analytic form is the zone-mesh fallback; texture variant
    // is sample_terrain_y_at.
    { POLICY_BAKED_HEIGHTFIELD, "baked_heightfield",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS),
      /*gradient=*/false },

    // Fly-over policy — spheres, cubes, cameras. Includes all global
    // deformation fields so flyers ride pulses and auras.
    // STATUS: REALIZED — camera clamp, sphere orbit clearance, cube
    // hover + clearance (query_ground_flyer). gradient=true is intent:
    // query_ground_flyer_gradient exists with zero callers — see its
    // LATENT[policy-surface] tag.
    { POLICY_FLYER, "flyer",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA),
      /*gradient=*/true },                  // (intent; gradient path uncalled — see status)

    // Walker — the pawn. Everything flyer includes, plus the
    // consumer-local GoL suppression that flattens the zone under
    // the querying consumer's feet.
    // STATUS: REALIZED — pawn_ground_resolve via query_ground_walker_pair.
    // gradient=true is intent: query_ground_walker_gradient and
    // query_ground_walker_walkable exist with zero callers (the live
    // tilt path is terrain_normal_at's 3-tap over walker_tilt).
    { POLICY_WALKER, "walker",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA)
        | (1u << CONTRIB_GOL_SUPPRESSION),
      /*gradient=*/true },                  // (intent; gradient path uncalled — see status)

    // Walker-tilt — walker minus the self aura, used for tilt/normal
    // computation and step-climb decisions. Excludes CONTRIB_PAWN_AURA
    // (the self form is a zero-gradient scalar; excluded so self-centered
    // fields never drive tilt). It CARRIES the same pawn-centered GoL
    // suppression the walker applies (truth-fix TER-2: the mask now
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
        | (1u << CONTRIB_GOL_SUPPRESSION),  // pawn-centered; same suppression walker applies
      /*gradient=*/true },

    // Walker-agent — agents feel the full GoL lift (no suppression).
    // STATUS: REALIZED — agent_post_step ground snap (scalar only).
    { POLICY_WALKER_AGENT, "walker_agent",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA),
      /*gradient=*/true },                  // (intent; gradient path unrealized — no gradient fn, no multi-sample consumer)

    // Celestial — no ground (sun, stars). Symmetry slot.
    // STATUS: INTENT — declared, zero realization ("none today");
    // kept for symmetry.
    { POLICY_CELESTIAL, "celestial",
      0u,
      /*gradient=*/false },

    // Terrain-render — the fused render-side set: the baked heightfield
    // (static base + pyramids) + pawn aura + terrain waves + radial
    // pulses. Deliberately NO CONTRIB_GOL_ZONES: the patch heightfield
    // does not cache GoL; zones render as their own extrusion pass.
    // This policy has NO query_ground_* function by design — its
    // realizations are hand-fused for per-vertex cost: patch_terrain_vs
    // (the full set; gradients realized there via texture .yz + the
    // analytic wave gradient) and shadow_patch_terrain_vs (baked + waves
    // subset, documented at its site). The ~15 entity/painting VS sites
    // that add contrib_terrain_waves_at alone atop the entity ground
    // atlas are sanctioned single-contributor consumptions of this same
    // render set.
    // STATUS: REALIZED (fused-only).
    { POLICY_TERRAIN_RENDER, "terrain_render",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA),
      /*gradient=*/true },                  // realized in the fused VS (texture .yz + analytic wave gradient)
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
//
// Shape (TER-2 2d): an immediately-invoked constexpr lambda per
// static_assert, kept AS-IS across the LADDER-1 c2 extraction. The
// class-body constraint that ORIGINALLY forced the lambda (a class-body
// static_assert cannot call a member function of the still-incomplete
// enclosing class) has dissolved at file scope; the restyle to a
// namespace constexpr function is a NAMED LATER STAGE. The lambda's body
// is parsed in place and reads only the already-initialized inline
// constexpr tables above. INTENT edges (stub endpoints) participate like
// any other edge: their endpoints exist as ContributorId bits, so
// closure over them is well-defined (and currently vacuous — no mask
// includes the stubs).

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

ASSERT_POLICY_DAG_CLOSED(POLICY_PLACEMENT_PYRAMID,    "POLICY_PLACEMENT_PYRAMID");
ASSERT_POLICY_DAG_CLOSED(POLICY_PLACEMENT_PAINTING,   "POLICY_PLACEMENT_PAINTING");
ASSERT_POLICY_DAG_CLOSED(POLICY_PLACEMENT_VEGETATION, "POLICY_PLACEMENT_VEGETATION");
ASSERT_POLICY_DAG_CLOSED(POLICY_BAKED_HEIGHTFIELD,    "POLICY_BAKED_HEIGHTFIELD");
ASSERT_POLICY_DAG_CLOSED(POLICY_FLYER,                "POLICY_FLYER");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER,               "POLICY_WALKER");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER_TILT,          "POLICY_WALKER_TILT");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER_AGENT,         "POLICY_WALKER_AGENT");
ASSERT_POLICY_DAG_CLOSED(POLICY_CELESTIAL,            "POLICY_CELESTIAL");
ASSERT_POLICY_DAG_CLOSED(POLICY_TERRAIN_RENDER,       "POLICY_TERRAIN_RENDER");

#undef ASSERT_POLICY_DAG_CLOSED

} // namespace the_board
} // namespace t7
