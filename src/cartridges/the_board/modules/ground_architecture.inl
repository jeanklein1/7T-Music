// ─── ground_architecture.inl ─────────────────────────────────────
//
// Ground query registry: contributors, dependency DAG, and policies.
//
// See ground_hierarchy_design.md for design rationale and
// ground_refactor_claude_code_brief.md for migration steps.
//
// A *contributor* is a named source of height (or subtractive
// displacement) at a world XZ. A *policy* is a named filter over
// the contributor set: a consumer declares its policy, and a single
// query function in world.wgsl evaluates the policy-selected
// contributor sum for that consumer.
//
// This file declares:
//   - ContributorId        — the stable id for each contributor
//   - PolicyId             — the stable id for each policy
//   - CONTRIBUTOR_DAG      — explicit dependency edges among static
//                            landforms (used for policy closure checks
//                            and placement order validation)
//   - POLICIES             — per-policy contributor bitmask + flags
//   - static_assert checks that every policy is closed under the DAG
//
// Step 1 of the ground refactor: registry scaffolding only. No
// queries are dispatched yet; nothing in the hot path is called.
//
// Included inside the Cartridge class body.
// Depends on: nothing (pure enum + table definitions).
// ─────────────────────────────────────────────────────────────────

enum ContributorId : uint32_t {
    CONTRIB_TERRAIN_LATTICE   = 0,   // fused into contrib_static_base_at
    CONTRIB_TILE_MODIFIERS    = 1,   // fused into contrib_static_base_at
    CONTRIB_SOLIDS            = 2,   // piers, ramps; fused into contrib_static_base_at
    CONTRIB_PYRAMIDS          = 3,
    CONTRIB_PAINTINGS_BASES   = 4,   // placeholder (stub contributor — returns 0.0)
    CONTRIB_VEGETATION_BASES  = 5,   // placeholder (stub contributor — returns 0.0)
    CONTRIB_GOL_ZONES         = 6,   // slow_dynamic
    CONTRIB_TERRAIN_WAVES     = 7,   // deformation_field, global
    CONTRIB_RADIAL_PULSES     = 8,   // deformation_field, global
    CONTRIB_PAWN_AURA         = 9,   // deformation_field, global pawn-centered
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
    POLICY_WALKER_AGENT         = 6,
    POLICY_CELESTIAL            = 7,
    POLICY_COUNT                = 8,
};

// --- Dependency DAG ---
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

static constexpr ContributorEdge CONTRIBUTOR_DAG[] = {
    { CONTRIB_TERRAIN_LATTICE,  CONTRIB_PYRAMIDS         },
    { CONTRIB_TILE_MODIFIERS,   CONTRIB_PYRAMIDS         },
    { CONTRIB_SOLIDS,           CONTRIB_PYRAMIDS         },
    { CONTRIB_PYRAMIDS,         CONTRIB_PAINTINGS_BASES  },
    { CONTRIB_SOLIDS,           CONTRIB_PAINTINGS_BASES  },
    { CONTRIB_SOLIDS,           CONTRIB_VEGETATION_BASES },
};
static constexpr uint32_t CONTRIBUTOR_DAG_EDGE_COUNT =
    sizeof(CONTRIBUTOR_DAG) / sizeof(CONTRIBUTOR_DAG[0]);

// --- Policy definitions ---
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
static constexpr uint32_t GROUND_STATIC_BASE_MASK =
    (1u << CONTRIB_TERRAIN_LATTICE) |
    (1u << CONTRIB_TILE_MODIFIERS)  |
    (1u << CONTRIB_SOLIDS);

static constexpr PolicyDef POLICIES[] = {
    // Placement policies — spawn-time Y correction. No deformation
    // fields (placement should be stable against animated terrain).
    { POLICY_PLACEMENT_PYRAMID, "placement_pyramid",
      GROUND_STATIC_BASE_MASK,
      /*gradient=*/false },

    { POLICY_PLACEMENT_PAINTING, "placement_painting",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES),       // paintings sit on current GoL (preserves pre-refactor behavior)
      /*gradient=*/false },

    { POLICY_PLACEMENT_VEGETATION, "placement_vegetation",
      GROUND_STATIC_BASE_MASK,              // trees don't stand on pyramids
      /*gradient=*/false },

    // Baked heightfield — cached static ground texture consumed by
    // patch VS interpolation and CPU readbacks.
    { POLICY_BAKED_HEIGHTFIELD, "baked_heightfield",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS),
      /*gradient=*/false },

    // Fly-over policy — spheres, cubes, cameras. Includes all global
    // deformation fields so flyers ride pulses and auras.
    { POLICY_FLYER, "flyer",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA),
      /*gradient=*/true },

    // Walker — the pawn. Everything flyer includes, plus the
    // consumer-local GoL suppression that flattens the zone under
    // the querying consumer's feet.
    { POLICY_WALKER, "walker",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA)
        | (1u << CONTRIB_GOL_SUPPRESSION),
      /*gradient=*/true },

    // Walker-agent — agents feel the full GoL lift (no suppression).
    { POLICY_WALKER_AGENT, "walker_agent",
      GROUND_STATIC_BASE_MASK
        | (1u << CONTRIB_PYRAMIDS)
        | (1u << CONTRIB_GOL_ZONES)
        | (1u << CONTRIB_TERRAIN_WAVES)
        | (1u << CONTRIB_RADIAL_PULSES)
        | (1u << CONTRIB_PAWN_AURA),
      /*gradient=*/true },

    // Celestial — no ground (sun, stars). Symmetry slot.
    { POLICY_CELESTIAL, "celestial",
      0u,
      /*gradient=*/false },
};
static constexpr uint32_t POLICY_COUNT_IN_TABLE =
    sizeof(POLICIES) / sizeof(POLICIES[0]);

static_assert(POLICY_COUNT_IN_TABLE == POLICY_COUNT,
              "POLICIES table must declare one row per PolicyId");

// --- Compile-time DAG-closure validation ---
// For every contributor in a policy's set, all of its ancestors via
// CONTRIBUTOR_DAG must also be in the set. Expressed as one
// static_assert per (policy, edge) pair via the DAG_EDGE_CLOSED macro:
//
//   mask has `to` → mask has `from`
// = !(mask has `to`) || mask has `from`
//
// Using a macro avoids calling a member constexpr function from a
// class-body static_assert (which fails because the enclosing class
// isn't complete at that point).

#define DAG_EDGE_CLOSED(MASK, FROM, TO) \
    ( (((MASK) >> (TO)) & 1u) == 0u || (((MASK) >> (FROM)) & 1u) != 0u )

#define ASSERT_POLICY_DAG_CLOSED(POLICY_IDX, POLICY_NAME)                                                       \
    static_assert(DAG_EDGE_CLOSED(POLICIES[POLICY_IDX].contributors,                                            \
                                  CONTRIB_TERRAIN_LATTICE, CONTRIB_PYRAMIDS),                                   \
                  POLICY_NAME ": includes PYRAMIDS but not TERRAIN_LATTICE");                                   \
    static_assert(DAG_EDGE_CLOSED(POLICIES[POLICY_IDX].contributors,                                            \
                                  CONTRIB_TILE_MODIFIERS, CONTRIB_PYRAMIDS),                                    \
                  POLICY_NAME ": includes PYRAMIDS but not TILE_MODIFIERS");                                    \
    static_assert(DAG_EDGE_CLOSED(POLICIES[POLICY_IDX].contributors,                                            \
                                  CONTRIB_SOLIDS, CONTRIB_PYRAMIDS),                                            \
                  POLICY_NAME ": includes PYRAMIDS but not SOLIDS");                                            \
    static_assert(DAG_EDGE_CLOSED(POLICIES[POLICY_IDX].contributors,                                            \
                                  CONTRIB_PYRAMIDS, CONTRIB_PAINTINGS_BASES),                                   \
                  POLICY_NAME ": includes PAINTINGS_BASES but not PYRAMIDS");                                   \
    static_assert(DAG_EDGE_CLOSED(POLICIES[POLICY_IDX].contributors,                                            \
                                  CONTRIB_SOLIDS, CONTRIB_PAINTINGS_BASES),                                     \
                  POLICY_NAME ": includes PAINTINGS_BASES but not SOLIDS");                                     \
    static_assert(DAG_EDGE_CLOSED(POLICIES[POLICY_IDX].contributors,                                            \
                                  CONTRIB_SOLIDS, CONTRIB_VEGETATION_BASES),                                    \
                  POLICY_NAME ": includes VEGETATION_BASES but not SOLIDS")

ASSERT_POLICY_DAG_CLOSED(POLICY_PLACEMENT_PYRAMID,    "POLICY_PLACEMENT_PYRAMID");
ASSERT_POLICY_DAG_CLOSED(POLICY_PLACEMENT_PAINTING,   "POLICY_PLACEMENT_PAINTING");
ASSERT_POLICY_DAG_CLOSED(POLICY_PLACEMENT_VEGETATION, "POLICY_PLACEMENT_VEGETATION");
ASSERT_POLICY_DAG_CLOSED(POLICY_BAKED_HEIGHTFIELD,    "POLICY_BAKED_HEIGHTFIELD");
ASSERT_POLICY_DAG_CLOSED(POLICY_FLYER,                "POLICY_FLYER");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER,               "POLICY_WALKER");
ASSERT_POLICY_DAG_CLOSED(POLICY_WALKER_AGENT,         "POLICY_WALKER_AGENT");
ASSERT_POLICY_DAG_CLOSED(POLICY_CELESTIAL,            "POLICY_CELESTIAL");

#undef DAG_EDGE_CLOSED
#undef ASSERT_POLICY_DAG_CLOSED
