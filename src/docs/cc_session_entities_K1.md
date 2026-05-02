# Claude Code session — entities:K1 resolution (Option C with converters)

## Context

The seam map's `entities:K1` knot was deferred from Phase 5 with the
note "decide after Phase 5 settles." Phase 5 is now complete and the
decision has been made: **Option C with converter**.

The current state has tier data duplicated across two parallel tables
for every generic-pipeline family. For example, Pyramid:

- `entities.inl::PYRAMID_TIERS[]` — named struct
  `PyramidTierParams { height_mean, height_sigma, base_half_mean, ...
  color_override, color_variance, weight }`. Artist-friendly, named
  fields, family-specific shape.
- `entity_pipeline.inl::PYRAMID_TIER_TABLE[]` — generic
  `TierProfile { weight, TierMuSigma params[MAX_ENTITY_PARAMS] }`.
  Pipeline-friendly, uniform shape, indexed by `*_PARAM_DEFS` order.

The same Gaussian values appear verbatim in both. They are kept in
sync by hand discipline. A change to one without the other produces
silent drift.

**Decision:** the named struct in `entities.inl` (or
`floater_vocabulary.inl` for floaters) becomes the **single source of
truth**. The generic `*_TIER_TABLE` is rewritten as a derived array
populated by a per-family `*_to_profile()` constexpr converter. The
pipeline machinery still reads `*_TIER_TABLE` exactly as it does
today; the only change is that the table's values come from the
converter rather than being hand-authored.

## Scope — 9 family pairs

Seven grounded families with tier authoring in `entities.inl`:

| Family   | Named struct                  | Generic table             |
|----------|-------------------------------|---------------------------|
| Arch     | `ArchTierParams ARCH_TIERS`   | `ARCH_TIER_TABLE`         |
| Column   | `ColumnTierParams COLUMN_TIERS` | `COLUMN_TIER_TABLE`     |
| Antenna  | `ColumnTierParams ANTENNA_TIERS` | `ANTENNA_TIER_TABLE`   |
| Palm     | `PalmTierParams PALM_TIERS`   | `PALM_TIER_TABLE`         |
| Cactus   | `CactusTierParams CACTUS_TIERS` | `CACTUS_TIER_TABLE`     |
| Blade    | `BladeClusterTierParams BLADE_TIERS` | `BLADE_TIER_TABLE` |
| Pyramid  | `PyramidTierParams PYRAMID_TIERS` | `PYRAMID_TIER_TABLE`  |

Two floater families with tier authoring in `floater_vocabulary.inl`:

| Family   | Named struct                          | Generic table          |
|----------|---------------------------------------|------------------------|
| Sphere   | `SphereTierProfile SPHERE_TIERS`      | `SPHERE_TIER_TABLE`    |
| Cube     | `CubeTierProfile CUBE_TIERS`          | `CUBE_TIER_TABLE`      |

Note that Antenna shares `ColumnTierParams` with Column but has its
own `ANTENNA_TIERS` table. Antenna is a separate migration with its
own converter call.

## The pattern (using Pyramid as worked example)

### Before

In `entities.inl`:
```cpp
struct PyramidTierParams {
    float height_mean, height_sigma;
    float base_half_mean, base_half_sigma;
    float aspect_ratio_mean, aspect_ratio_sigma;
    float truncation_mean, truncation_sigma;
    float edge_blend_mean, edge_blend_sigma;
    float color_override;
    float color_variance;
    float weight;
};

static constexpr PyramidTierParams PYRAMID_TIERS[] = {
    /* OBELISK  */ { 28.0f, 6.0f,  16.0f, 3.0f,  1.0f, 0.15f,
                     0.00f, 0.00f,  1.5f, 0.3f,  0.10f, 0.04f, 0.50f },
    /* TEMPLE   */ { 45.0f, 8.0f,  40.0f, 6.0f,  1.0f, 0.20f,
                     0.25f, 0.08f,  3.0f, 0.75f, 0.15f, 0.04f, 0.25f },
    /* COLOSSUS */ { 78.0f, 14.4f, 60.0f, 9.6f,  1.0f, 0.10f,
                     0.05f, 0.04f,  3.6f, 1.0f,  0.20f, 0.04f, 0.25f },
};
```

In `entity_pipeline.inl` (hand-authored, duplicates the same values):
```cpp
static constexpr TierProfile PYRAMID_TIER_TABLE[] = {
    /* OBELISK  */ { 0.50f, { {28.0f,6.0f},{16.0f,3.0f},{1.0f,0.15f},
                              {0.00f,0.00f},{1.5f,0.3f} }},
    /* TEMPLE   */ { 0.25f, { {45.0f,8.0f},{40.0f,6.0f},{1.0f,0.20f},
                              {0.25f,0.08f},{3.0f,0.75f} }},
    /* COLOSSUS */ { 0.25f, { {78.0f,14.4f},{60.0f,9.6f},{1.0f,0.10f},
                              {0.05f,0.04f},{3.6f,1.0f} }},
};
```

### After

`entities.inl` is **unchanged** — the named struct remains the source.

`entity_pipeline.inl` replaces the hand-authored table with a
converter and a derived table:
```cpp
// Converter: takes the named-struct row, produces a generic TierProfile.
// The order of TierMuSigma pairs MUST match PYRAMID_PARAM_DEFS order
// (HEIGHT, BASE_HALF, ASPECT, TRUNCATION, EDGE_BLEND).
static constexpr TierProfile pyramid_to_profile(const PyramidTierParams& p) {
    TierProfile out{};
    out.weight = p.weight;
    out.params[PyrIdx::HEIGHT]     = { p.height_mean,        p.height_sigma };
    out.params[PyrIdx::BASE_HALF]  = { p.base_half_mean,     p.base_half_sigma };
    out.params[PyrIdx::ASPECT]     = { p.aspect_ratio_mean,  p.aspect_ratio_sigma };
    out.params[PyrIdx::TRUNCATION] = { p.truncation_mean,    p.truncation_sigma };
    out.params[PyrIdx::EDGE_BLEND] = { p.edge_blend_mean,    p.edge_blend_sigma };
    return out;
}

// Derived table — populated by the converter, not authored.
// DONE[entities:K1] this table now derives from PYRAMID_TIERS in
//   entities.inl. Edits to tier values happen there, not here.
static constexpr TierProfile PYRAMID_TIER_TABLE[] = {
    pyramid_to_profile(PYRAMID_TIERS[0]),
    pyramid_to_profile(PYRAMID_TIERS[1]),
    pyramid_to_profile(PYRAMID_TIERS[2]),
};
```

The pipeline machinery downstream of `PYRAMID_TIER_TABLE` is
unchanged — the table still has the same shape and the same values.

### What does NOT change

- The named struct's field names, order, or count. Stay identical.
- The named struct's "extras" fields (`color_override`, `color_variance`,
  any family-specific extras like `burial`, `segs_u`, `segs_v`,
  `pier_height`, etc.). These already live only in the named struct
  and continue to do so. They are read by per-family adapter
  functions in `entity_pipeline.inl` that already know to look them
  up by name.
- `TierProfile`, `TierMuSigma`, `EntityFamilyTraits` definitions in
  `entity_types.inl`. Untouched.
- The `*_PARAM_DEFS` arrays in `entity_pipeline.inl`. Untouched —
  these define the param ordering that the converter must match.
- `*Idx` structs (`PyrIdx`, `ArchIdx`, etc.) defining the param
  indices. Untouched.
- All adapter functions, gate logic, dispatch, mesh gen — untouched.
- All consumers of the named struct fields elsewhere in the codebase.
  They still read `PYRAMID_TIERS[0].color_override` etc. exactly as
  before.

## Step-by-step procedure

### Step 0 — Verify the assumption

Before any code changes, confirm by running a comparison: for each of
the 9 families, the values in the named-struct table and the values
in the generic table agree numerically. If any pair *disagrees*, stop
and report which family and which value differ — there may be a
hidden reason for the discrepancy that needs to be understood before
collapsing the duplication.

This is a `git grep` + visual diff exercise. The expected result is
"all 9 pairs agree exactly."

### Step 1 — One family as proof, then the rest

Start with **Pyramid**. Implement the converter, replace
`PYRAMID_TIER_TABLE` with the derived form, build, run.

If pyramid moods (mood-0 sunset, mood-1, mood-4 finite_outdoor — the
moods where pyramids spawn) look identical to before, the pattern is
confirmed and you can apply it to the remaining 8 families.

If anything looks different visually, stop and investigate. The
converter is a no-op transformation in principle; any visible
difference means the named-struct values diverge from the
hand-authored table values, or the field-to-PARAM_DEF mapping in the
converter is wrong.

### Step 2 — Apply to remaining 8 families

In order: Arch, Column, Antenna (note: shares struct with Column,
distinct table), Palm, Cactus, Blade, Sphere, Cube.

Each family follows the same pattern:

1. Find the family's `*Idx` struct in `entity_pipeline.inl` to know
   the param-def order.
2. Find the family's `*_PARAM_DEFS` array — same order as `*Idx`.
3. Find the family's named struct in `entities.inl` (or
   `floater_vocabulary.inl` for sphere/cube).
4. Write the converter `family_to_profile()` immediately above the
   `*_TIER_TABLE` declaration. Map each `*_PARAM_DEFS` entry to the
   corresponding `mean`/`sigma` field pair in the named struct.
5. Replace the body of `*_TIER_TABLE` with calls to the converter,
   one per tier row.
6. Add a `// DONE[entities:K1]` comment above the new table.

### Step 3 — Per-family field mapping reference

For each family, the converter's `params[*Idx::FOO] = { p.foo_mean,
p.foo_sigma }` mappings need to match the family's specific fields.
**Do not assume a uniform name pattern.** Some fields use prefixes
the param def doesn't (e.g., `aspect_ratio_mean` maps to `ASPECT`).
Read each family carefully.

The expected tier counts:
- Arch: 4 tiers
- Column: 3 tiers (NEEDLE, PILLAR, COLOSSUS — verify)
- Antenna: 3 tiers (uses `ColumnTierParams`, separate values)
- Palm: 3 tiers (verify)
- Cactus: 3 tiers (verify)
- Blade: 3 tiers (verify)
- Pyramid: 3 tiers (OBELISK, TEMPLE, COLOSSUS)
- Sphere: 2 tiers (Sentinel, Anomaly)
- Cube: 4 tiers (SmallCube, MedCube, LargeCube, Monolith)

Verify each by counting rows in the named-struct table.

### Step 4 — Build and verify

After all 9 conversions, run the project's normal build. The build
should pass. The `*_TIER_TABLE` arrays are still `static constexpr`
so all values are computed at compile time; runtime behavior is
identical.

Run the cartridge through every mood that spawns each family:
- Outdoor moods (sunset, default): pyramids, arches, columns,
  antennas, palms, cacti, blades, spheres, cubes spawn.
- Finite outdoor: same families.
- Indoor moods (flat, vault): no entity families spawn (mood gate
  zeroes them out).
- finite_outdoor_ref: pyramids do not spawn (mood multiplier is 0);
  others do.

Visual confirmation: each family's instances should look identical
to the pre-change baseline. **Do not make tuning changes during this
session** — even ones you think might improve things. The test is
"behavior unchanged."

## Constraints

- **Do not touch the named structs in `entities.inl` or
  `floater_vocabulary.inl`.** The whole point is that those become
  the source of truth. Field names, types, order, count must stay
  exactly as they are.
- **Do not touch `entity_types.inl`** — `TierProfile`, `TierMuSigma`,
  `MAX_ENTITY_PARAMS`. Untouched.
- **Do not touch `*_PARAM_DEFS` arrays.** They define the indexing
  the converter must match.
- **Do not touch any adapter function or family-specific compute
  function** (`pyramid_compute_solid_half`,
  `pyramid_compute_colors`, etc.). Their behavior depends on
  reading values from `*_TIER_TABLE` and from the named struct's
  extras; both shapes are preserved.
- **Do not change tier count, weight values, mean values, or sigma
  values for any family.** The migration is shape-preserving by
  design.
- **Save build verification for the end of the session**, after all
  9 families are migrated. A per-family build check is fine if
  something looks wrong; otherwise one final build at the end.

## Output

- The modified `entity_pipeline.inl` (only file that changes).
- A summary report:
  - Step 0 result: did all 9 pairs agree numerically? Any
    discrepancies?
  - For each family: converter written, table replaced, no other
    changes.
  - Build outcome.
  - Any visual differences observed during run-time check (expected:
    none).

## Conflict resolution

- If Step 0 reveals any value disagreement between named struct and
  generic table, **stop and report**. Do not "fix" the disagreement
  — the discrepancy needs to be understood first.
- If the converter for some family doesn't have an obvious mapping
  (e.g., the named struct has no `*_mean`/`*_sigma` field for one of
  the param-def entries), **stop and report**. This shouldn't
  happen, but if it does, the family's structure may differ from the
  pattern.
- If you find that the named struct has fields with different *units*
  or *meanings* than the generic table (e.g., one stores degrees and
  the other radians), **stop and report**. This would be a hidden
  semantic divergence.

## Why this matters (briefly, for context)

Today, every change to pyramid (or any family's) tier values
requires editing two files in lockstep. After this migration, edits
happen in `entities.inl` only; the generic table is mechanically
derived. This is a step toward the eventual `control_surface.inl`
direction — making the named structs the canonical home for
tunables means the next migration moves them into a
project-wide control surface rather than refactoring out of
machinery-shaped tables.

The architectural posture is: **machinery is generic, vocabulary is
specific, vocabulary is authored, machinery is derived**.
