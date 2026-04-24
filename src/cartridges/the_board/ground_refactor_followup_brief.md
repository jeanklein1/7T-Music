# Claude Code Follow-Up Brief — Ground Refactor Finish-Work & Post-Pass Issues

This brief covers what remains from the ground architecture refactor
after Steps 1–5 landed successfully:

1. **Step 6 — documentation** that was deferred during execution.
2. **Comment/annotation hygiene** to prevent drift in the two places the
   refactor left as "keep in sync by hand."
3. **Pawn tilt regression** — a real bug introduced by the refactor.
4. **Camera clamp expansion** — finishing the camera migration we
   backed off from mid-pass.

Items 1 and 2 are finish-work. Items 3 and 4 are post-pass fixes.

The original design document is `ground_hierarchy_design.md`. The
original execution brief is `ground_refactor_claude_code_brief.md`.
Read both before starting if context is needed.

---

## Part A — Step 6 Documentation

The refactor deferred in-shader documentation during execution. The
old three-level comment block at `world.wgsl:1966–1987` is still
present, labeled "(legacy description — rewritten in Step 6)" —
but the rewrite never happened. Also, individual `contrib_*_at` and
`query_ground_*` functions need header comments.

### A.1 Rewrite the architecture comment block

**Location:** `world.wgsl:1966–1987` (the "Composable Ground
Hierarchy (legacy description — rewritten in Step 6)" block).

**Replace the entire block** with a new header that describes the
contributor graph / policy architecture. Keep it aligned with the
structure of `ground_hierarchy_design.md` §2–§5 but compact. Target
shape:

```wgsl
// --- Ground Architecture: Contributor Graph + Policies --------------
//
// The ground at any world XZ is the sum of selected contributors.
// Each consumer declares a POLICY; a query function evaluates the
// policy-selected contributor sum.
//
// CONTRIBUTORS (by class):
//
//   static_landform — placed once, baked, part of permanent geometry.
//                     Have dependency edges (see CONTRIBUTOR_DAG).
//       CONTRIB_TERRAIN_LATTICE | TILE_MODIFIERS | SOLIDS
//         → fused into contrib_static_base_at.
//       CONTRIB_PYRAMIDS
//       CONTRIB_PAINTINGS_BASES  (stub, returns 0.0)
//       CONTRIB_VEGETATION_BASES (stub, returns 0.0)
//
//   slow_dynamic — per-frame state, not music-modulated.
//       CONTRIB_GOL_ZONES
//
//   deformation_field — temporally or locally active. Orthogonal
//                       to the DAG. Applied additively.
//       CONTRIB_TERRAIN_WAVES    (global, polyphony-driven)
//       CONTRIB_RADIAL_PULSES    (global, note-onset wavefronts)
//       CONTRIB_PAWN_AURA        (global, pawn-centered)
//       CONTRIB_GOL_SUPPRESSION  (consumer-local, subtractive)
//
// POLICIES (by consumer role):
//
//   POLICY_PLACEMENT_PYRAMID     — pyramid spawn (static base only)
//   POLICY_PLACEMENT_PAINTING    — paintings (static + GoL)
//   POLICY_PLACEMENT_VEGETATION  — trees/columns (static base only)
//   POLICY_BAKED_HEIGHTFIELD     — cached texture cache contents
//   POLICY_FLYER                 — spheres, cubes: all + aura, no suppression
//   POLICY_WALKER                — pawn: all + aura + self-suppression
//   POLICY_WALKER_TILT           — pawn tilt/step-climb: walker minus self-centered
//                                  (aura + suppression) — see A.3 below, added in
//                                  this follow-up pass
//   POLICY_WALKER_AGENT          — agents (no self-suppression)
//   POLICY_CELESTIAL             — empty (sun/stars/sky)
//
// Query entry points: one per policy.
//   query_ground_<policy>(xz [, QueryInputs])           → f32
//   query_ground_<policy>_gradient(xz, qi, eps)         → vec3  (h, ∂x, ∂z)
//   query_ground_walker_walkable(xz, qi, eps, step_h)   → vec3  (cliff-clamped)
//
// EXTENSION RULES:
//   - New static landform: declare in ContributorId, add CONTRIBUTOR_DAG
//     edges, add to policies that should see it.
//   - New deformation field: add contrib_<name>_at, add to policies it
//     affects (usually POLICY_FLYER and POLICY_WALKER at minimum).
//   - New consumer: pick a policy, pass QueryInputs, call query_*.
//     Do NOT compose contributors inline at the call site.
//
// AUTHORITATIVE REGISTRY: modules/ground_architecture.inl (C++ side),
// mirrored by CONTRIB_* / POLICY_*_MASK constants below.
//
// DESIGN DOC: ground_hierarchy_design.md — read for rationale and
// open questions (per-agent auras, deformation non-commutativity,
// fused-VS consistency).
// --------------------------------------------------------------------
```

Exact shape is a guideline, not literal — adjust to fit the existing
comment style and line width conventions in `world.wgsl`.

### A.2 Per-function header comments

Add a one-block comment header above each `contrib_*_at` and
`query_ground_*` function. Many already have partial headers;
standardize them to this shape.

**For `contrib_*_at` functions** (lines 1845, 1861, 1894, 1994, 2003,
2010, 2085, 2195, 2233), each header names:

```wgsl
// CONTRIB_<NAME> — <class>, <scope>.
// Contributes: <one-line what it adds/subtracts>.
// Dependencies (via DAG): <list, or "none">.
// Notes: <optional — e.g. "reads pawn_state internally", "stub returning 0.0">.
```

Most of these already exist; verify each one matches the template and
completes any missing info.

**For `query_ground_*` functions** (lines 2261, 2267, 2275, 2284,
2293, 2307, 2319, 2333, 2341, 2351, 2365), each header names:

```wgsl
// POLICY_<NAME> — <role>.
// Contributors: <list, matching POLICIES[] in ground_architecture.inl>.
// Typical consumers: <list — e.g. "pyramid spawn", "sphere clearance",
//                     "pawn ground resolve">.
// Notes: <optional>.
```

### A.3 Add POLICY_WALKER_TILT to the documentation

This is the new policy introduced by Part C. Document it alongside
the others. See Part C for the semantic definition.

---

## Part B — Drift-Risk Annotations

Two places in the codebase now duplicate contributor logic by
necessity (performance, or cross-stage architectural constraints).
Both need comment annotations naming them as the hand-maintained
equivalents of their `contrib_*_at` / `query_ground_*` counterparts
so a future developer doesn't quietly drift them.

### B.1 The patch terrain VS (`patch_terrain_vs`)

**Location:** `world.wgsl` around lines 3040–3110.

The VS inlines composition of static heightfield + pawn aura + waves
+ pulses per vertex for per-vertex performance. It uses a mix of
`contrib_*_at`, direct function calls, and the analytic gradient
helper `terrain_wave_overlay_with_gradient`.

**Add a header comment** immediately above `fn patch_terrain_vs(...)`:

```wgsl
// patch_terrain_vs — hand-fused POLICY_FLYER-ish evaluation.
//
// Inlines the contributor sum for per-vertex performance: patch
// heightfield texture (cached CONTRIB_STATIC_BASE + CONTRIB_PYRAMIDS)
// + CONTRIB_PAWN_AURA + CONTRIB_TERRAIN_WAVES + CONTRIB_RADIAL_PULSES.
// Does NOT include CONTRIB_GOL_ZONES (the patch heightfield does not
// cache GoL — zones are rendered as a separate extrusion pass).
//
// Uses terrain_wave_overlay_with_gradient (not contrib_terrain_waves_at)
// because the gradient is needed for the fragment normal and is computed
// analytically in the same pass.
//
// Keep consistent with POLICY_FLYER: if a new deformation field is
// added to POLICY_FLYER, add it here too, or explicitly document why
// the render side diverges.
//
// See ground_hierarchy_design.md §8 (fused inline evaluations).
```

### B.2 The zone extrusion VS/FS GoL suppression logic

**Locations:**
- `world.wgsl:5710–5770` (approx): `zone_extrusion_vs` inlines its
  own GoL suppression factor using `ZONE_SUPPRESS_INNER/OUTER` and
  `render_pawn.pos.xz`. The comment at ~line 5729 even says "uses same
  radii as zone_gol_height_at so VS geometry matches pawn Y resolve."
- `zone_extrusion_fs` around line 5762 also uses `sample_pawn_aura`
  inline for tinting.

**Add an annotation comment** above the suppression block in
`zone_extrusion_vs`:

```wgsl
// GoL suppression — render-side mirror of contrib_gol_suppression_at.
// Must stay in sync with the contributor's smoothstep (same inner/outer
// radii, same shape). The two cannot easily share a function because
// the VS is a render-stage consumer and contrib_* is compute-stage,
// but if either changes, update the other.
```

**Grep check** after annotating:

```
grep -rn "ZONE_SUPPRESS_INNER\|ZONE_SUPPRESS_OUTER" *.wgsl *.hpp *.inl
```

List every occurrence in the annotation to make it audit-friendly. If
any location uses these constants *without* being either
`contrib_gol_suppression_at` or the zone extrusion VS, that's a drift
site we didn't know about — flag it for discussion before committing.

---

## Part C — Fix the Pawn Tilt Regression

### C.1 Diagnosis

The refactor introduced a real bug: with aura active, the pawn tilts
erratically as it walks. The aura is a field centered on the pawn
itself; `terrain_normal_at` samples `query_ground_walker` at center
+ (ε, 0) + (0, ε) to compute a gradient; the aura contributes
significantly to those samples' heights and its radial profile gets
read *as if it were terrain slope*. Result: a manufactured gradient
that tilts the pawn based on the pawn's own aura shape, not the real
ground beneath it.

The pre-refactor code read the gradient from a function that didn't
include the aura, so the aura lifted the pawn translationally (same
amount at every sample) and didn't contribute to tilt.

Same issue, symmetrically, applies to step-climb: `query_ground_walker`
includes aura, so step-climb compares heights that include the pawn's
own aura contribution. The pawn can't "climb" its own aura; its own
aura isn't terrain.

And same again for GoL suppression — it's self-centered and would
contribute a bogus gradient slope to tilt if the pawn is standing
on or near a GoL zone.

### C.2 Fix — Introduce POLICY_WALKER_TILT

Add a new policy whose contributor set is **POLICY_WALKER minus the
self-centered deformation fields**:

```
  POLICY_WALKER_TILT
    contributors = GROUND_STATIC_BASE_MASK
                 | (1u << CONTRIB_PYRAMIDS)
                 | (1u << CONTRIB_GOL_ZONES)
                 | (1u << CONTRIB_TERRAIN_WAVES)
                 | (1u << CONTRIB_RADIAL_PULSES)
    // No CONTRIB_PAWN_AURA        — self-centered, breaks tilt
    // No CONTRIB_GOL_SUPPRESSION  — self-centered, breaks tilt
    gradient_supported = true
```

The *height* of the walker is still `query_ground_walker` (pawn
stands on aura-lifted ground). Only the *gradient* reads the
tilt-safe policy.

### C.3 Implementation

**Step C.3a — Extend the registry**

In `modules/ground_architecture.inl`:

- Add `POLICY_WALKER_TILT` to the `PolicyId` enum (before
  `POLICY_COUNT`).
- Add a row to `POLICIES[]` with the contributor set described in
  C.2.
- Add a `ASSERT_POLICY_DAG_CLOSED` line for it.

Mirror in `world.wgsl` near the other `const POLICY_*_MASK` (around
line 1935 region): add `const POLICY_WALKER_TILT_MASK: u32 = ...` if
that pattern is established, or skip if the shader side uses only
named query functions (likely).

**Step C.3b — Add the query function**

In `world.wgsl`, next to `query_ground_walker` (around line 2307):

```wgsl
// POLICY_WALKER_TILT — walker minus self-centered deformations.
// Used for gradient/tilt sampling and walkable step-climb, where
// the pawn's own aura and self-suppression would contribute
// manufactured slopes. Same composition as POLICY_WALKER
// minus CONTRIB_PAWN_AURA and CONTRIB_GOL_SUPPRESSION.
// Typical consumers: terrain_normal_at, pawn_ground_resolve step-climb.
fn query_ground_walker_tilt(xz: vec2<f32>, qi: QueryInputs) -> f32 {
    var h = contrib_static_base_at(xz);
    h += contrib_pyramids_at(xz);
    h += contrib_gol_zones_at(xz);
    h += contrib_terrain_waves_at(xz);
    h += contrib_radial_pulses_at(xz, qi.t_seconds);
    return h;
}
```

Note: no aura. No suppression.

**Step C.3c — Migrate `terrain_normal_at`**

Currently (around line 4641):

```wgsl
fn terrain_normal_at(xz: vec2<f32>, qi: QueryInputs) -> vec3<f32> {
    let eps = 0.5;
    let h0  = query_ground_walker(xz, qi);
    let h_x = query_ground_walker(xz + vec2(eps, 0.0), qi);
    let h_z = query_ground_walker(xz + vec2(0.0, eps), qi);
    ...
}
```

Change three calls to `query_ground_walker_tilt`.

**Step C.3d — Migrate pawn step-climb**

In `pawn_ground_resolve` (around line 4657): the step-climb
comparisons currently use `query_ground_walker`. Change to
`query_ground_walker_tilt`.

**Important subtlety:** the *final resolved y* returned from
`pawn_ground_resolve` must still be the `query_ground_walker` value —
the pawn stands on the aura-lifted ground, it just doesn't trip over
its own aura. So the function queries both: `_tilt` for step-climb
decisions, `_walker` for the final y value.

Concretely:

```wgsl
fn pawn_ground_resolve(...) -> vec4<f32> {
    let y      = query_ground_walker(new_xz, qi);       // where we stand
    let y_tilt = query_ground_walker_tilt(new_xz, qi);  // for step-climb

    let moved = any(new_xz != prev_xz);
    if (!moved || y_tilt - prev_y_tilt <= PAWN_STEP_HEIGHT) {
        return vec4(new_xz.x, y, new_xz.y, 1.0);
    }
    ...
}
```

This needs `prev_y_tilt` threaded through — currently `prev_y` is the
"where we stood last frame" value. Either:
- Pass `prev_y_tilt` alongside `prev_y` from `update_pawn` (store the
  tilt-height from last frame in `pawn_state` as a new field), or
- Compute `prev_y_tilt` by sampling `query_ground_walker_tilt` at
  `prev_xz` inside `pawn_ground_resolve` (1 extra query per frame —
  cheap, no state changes).

**Recommendation: second option.** No `pawn_state` struct change, no
alignment worries, no FXC surprises. One extra query per frame is
negligible.

Verify the step-climb slide-axis logic (x_y, z_y comparisons around
line 4670) uses `query_ground_walker_tilt` consistently; they compare
against `prev_y`, which is still the aura-lifted height, but the
comparisons themselves decide whether to slide vs revert based on
`PAWN_STEP_HEIGHT` — switching those to tilt-heights keeps the
semantics ("is this step too tall to climb?") pointing at the real
terrain.

### C.4 Verification

- **Walk the pawn around with aura fully active.** Pawn should no
  longer tilt erratically. Tilt should track the actual ground
  slope beneath the pawn.
- **Walk up a hill.** Step-climb should behave the same as
  pre-refactor (no new stuck points).
- **Stand near a GoL zone while pulses are active.** Pawn tilt
  should read the pulse wavefront's slope; no erratic jitter from
  pawn aura or self-suppression.
- **Confirm the pawn still stands correctly on aura-lifted ground.**
  The aura still lifts the pawn's Y position; it just no longer
  contributes to tilt decisions.

---

## Part D — Camera Clamp: Migrate to POLICY_FLYER

### D.1 Why

Currently `update_camera` clamps against `sample_terrain_y_at`
(cached baked heightfield). Comment at line 4815 is honest: the camera
compute pipeline's bind group doesn't include aura/zone resources, so
extending it was out of scope during the main refactor.

The intended design (per `ground_hierarchy_design.md` §5.1) is for
the camera to use `POLICY_FLYER` — clearing aura-lifted *and*
pulse-lifted terrain. This finishes that migration.

### D.2 Extend the camera compute pipeline's bind group

**In `renderer.hpp`:** find the compute pipeline used by
`update_camera`. It's likely the same "Compute Entity Layout" that
was extended for `patch_grid` in the mid-pass fix, or it's a
dedicated camera layout.

Add bindings for the resources `query_ground_flyer` transitively
reads:
- `@binding(31)` — `zone_life_read: texture_2d_array<f32>` (GoL height data)
- `@binding(32)` — `zone_params: storage<GoLZoneArray, read>` (GoL zone defs)
- `@binding(33)` — `pawn_aura_read: texture_2d<f32>` (aura field)

Group index: whichever group the camera pipeline's layout uses for
these (check `update_pawn` or `update_sphere` — they just got this
treatment and work).

Then in the bind-group creation code for this pipeline, add the
corresponding resources.

### D.3 Change the shader

**In `world.wgsl:4815–4830`** (the camera clamp block):

Replace:

```wgsl
// POLICY_BAKED_HEIGHTFIELD consumer (texture variant).
// sample_terrain_y_at reads the cached patch heightfield, which
// captures static_base + pyramids only (no gol zones, no terrain
// waves, no radial pulses, no pawn aura). That's a pragmatic
// trade-off: ...
{
    let min_clearance = 1.5;
    let ground_at_cam = sample_terrain_y_at(camera.pos.xz);
    camera.pos.y = max(camera.pos.y, ground_at_cam + min_clearance);
}
```

With:

```wgsl
// POLICY_FLYER — camera clears pawn aura, terrain waves, and radial
// pulses. Reads live contributors rather than the cached heightfield,
// so ridges lifted by animated deformations do not clip the camera.
{
    let min_clearance = 1.5;
    let qi = QueryInputs(camera.pos, signal.t_seconds);
    let ground_at_cam = query_ground_flyer(camera.pos.xz, qi);
    camera.pos.y = max(camera.pos.y, ground_at_cam + min_clearance);
}
```

### D.4 Verification

- **Start a session and walk into an aura-active region.** Camera
  should never dip below the aura-lifted ground; third-person view
  should not suddenly clip into the pawn or the terrain beneath the
  pawn's aura halo.
- **Stand still while a pulse wavefront passes.** Camera should
  ride the lifted ground, not stay below it.
- **No regressions** — normal walking, indoor ceilings, finite
  boundary clamp all behave as before.

---

## Execution Order & Checkpointing

Each part is independent. Execute in order listed (A → B → C → D) so
that the documentation is up-to-date before the new policy and
migration land. After each part: build, test, commit, report.

- **Part A** — docs only, visual parity. Expected success: build
  succeeds, identical runtime behavior.
- **Part B** — comments only. Same success criterion.
- **Part C** — adds new policy, fixes tilt regression. Expected
  success: pawn no longer tilts erratically in aura; step-climb
  still works at hills; standing on aura-lifted ground still puts
  pawn at the lifted height.
- **Part D** — extends bind group + migrates camera. Expected
  success: camera clears aura-lifted and pulse-lifted terrain.

If anything looks wrong after a part — visual regression, unexpected
tilt behavior, bind-group layout error, FXC compilation failure —
**stop and report** before continuing to the next part.

---

## Out of Scope

- The larger agent system implementation.
- The floater backbone refactor.
- Any new contributor or deformation field.
- Replacing the zone extrusion VS's inline suppression with a
  shared function (architecturally tempting, practically awkward
  because of the compute/render stage split; stays a drift-risk
  annotation for now).
- Refactoring the patch terrain VS to call `contrib_*_at` fully —
  the wave gradient requires the analytic function; keeping it
  partial and annotated is correct for now.

---

## Questions to Raise

- **Part C prev_y_tilt approach.** Recommendation is to sample at
  `prev_xz` inside `pawn_ground_resolve`. If that turns out to
  structurally conflict with how `pawn_state` flows today, the
  alternative is adding `prev_y_tilt` as a new field on
  `pawn_state` — raise before committing either.
- **Part D bind-group group index.** The three aura/GoL bindings
  live in `@group(1)` per the shader declarations. The camera
  pipeline may currently only have `@group(0)` declared. If that
  requires adding a whole second bind group to the camera pipeline
  (not just extending an existing group), mention it — it's the
  same scope of work but worth flagging.
- **Part B grep for ZONE_SUPPRESS.** If the grep finds a site we
  haven't accounted for, list it before annotating. It might be a
  drift site that needs migrating, not just annotating.
