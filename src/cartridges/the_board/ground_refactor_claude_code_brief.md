# Claude Code Execution Brief — Ground Architecture Refactor

This brief directs the implementation of the ground architecture
refactor described in `ground_hierarchy_design.md`. The design
document is the source of truth for **why**; this brief is the source
of truth for **how** and **in what order**. When they conflict, the
design doc wins conceptually — come back to ask.

Read the design doc once, in full, before starting. Read this brief
in full before starting. Keep both in context throughout execution.

---

## 0. Context

**The project.** 7T — a WebGPU/Dawn (C++20 + WGSL) procedurally
generated interactive world. Active cartridge: `t7::the_board`
(`n_dimensional_4`). Canonical files per cartridge: `cartridge.hpp`,
`state.hpp`, `renderer.hpp`, `world.wgsl`, with `.inl` modules
textually included into `cartridge.hpp`.

**The refactor.** Today, ground-height queries are ad-hoc inline
compositions of contributor functions spread across ~90 call sites
in `world.wgsl`. Consumers pick different subsets inconsistently —
a cube reads `ground_formed + terrain_wave_overlay`, a sphere reads
the same, the pawn reads everything, the camera reads a baked
heightfield. Pulses and pawn aura are missing from some consumers
that should have them.

This refactor centralizes the composition as a **contributor graph
with explicit dependency edges** and a **set of named policies that
consumers query through**. After the refactor, no consumer composes
contributors by hand — each consumer declares a policy and calls a
single `query_ground_*` function. Adding a new contributor or new
musical analysis becomes a registry edit, not a callsite sweep.

**The scope of this pass.** The ground architecture itself, plus
migration of every existing consumer. No new features land; this is
a structural refactor with visual-parity success criteria, except
for one visible bug fix (cube/sphere now ride radial pulses instead
of sitting inside them).

---

## 1. Preflight Checks — Do Before Any Code Change

### 1.1 Read the design doc

Open `/mnt/user-data/outputs/ground_hierarchy_design.md`. Skim §2–§6
carefully. §7 is the migration plan this brief operationalizes. §8
is open questions — do not resolve them, defer to the user.

### 1.2 Read these files for context

- `world.wgsl` — lines 1768–2180 contain the current ground chain
  (static contributors, dynamic contributors, deformation fields).
  Line 1887 has the existing three-level comment block.
- `cartridge.hpp` — lines 75, 1472, 1475, 2418, 5907, 8593, 8597,
  8624 for the `.inl` module includes (pattern to follow for
  `ground_architecture.inl`).
- `pawn_aura.inl` — one example of a pre-existing module that owns
  a contributor. Will need minor touching to expose its eval
  through the contributor registry.

### 1.3 Project-specific tripwires

These are **known hardware and toolchain constraints** from
accumulated experience. Respect them throughout:

- **FXC uniform branching.** Claude Code writes WGSL for Dawn's FXC
  (HLSL) backend. FXC hangs on non-uniform branching in loops and
  on dynamic indexing into arrays of structs. Policy dispatch must
  be uniform — the `PolicyId` the caller passes is a compile-time
  constant in each call site, not a runtime variable.
- **Specialize per policy, don't runtime-switch.** Implement query
  functions as per-policy specializations (one function per policy,
  named by policy) rather than a single runtime `switch`. Each
  consumer calls the specialization matching its declared policy.
- **`bitcast<f32>(entity_idx)` is always wrong.** Use `f32(entity_idx)`
  for int-to-float conversions. (This one isn't in scope for the
  refactor, just a standing rule.)
- **`target` is a reserved WGSL keyword.** Do not introduce fields
  or variables named `target`; use `tgt` or similar.
- **WGSL struct alignment.** New structs in storage address space
  must have explicit padding to 16-byte alignment, verified with
  `static_assert` on the C++ side. See existing structs in
  `state.hpp` for the pattern.
- **Storage buffer budget (vertex stage).** Currently ~10 storage
  buffers used. This refactor adds **zero new storage buffers** —
  we're reorganizing functions, not adding bindings.
- **Uniform buffer budget.** 12/stage. No change from this refactor.
- **`array<f32, 64>` in uniform address space requires 16-byte
  element stride.** Not relevant to this pass unless you introduce
  a new uniform array; don't.
- **`DrawIndexedIndirect` quirks.** Not relevant.
- **`Storage | Indirect` on same buffer corrupts.** Not relevant.
- **Ground-resolve coherence.** The patch terrain VS, the pawn ground
  resolve, and any GoL-adjacent sampling must stay consistent. Any
  drift between what the terrain renders as and what the pawn walks
  on is a visible bug. This refactor should **reduce** the surface
  area where such drift can hide, not introduce new inconsistencies.

### 1.4 Policy: study before touching

Every change in this brief references a specific location (file +
line range). Open and read that location first. Do not edit based on
the brief's summary alone — the brief gives you the change, the file
gives you the context to make it correctly.

---

## 2. Target Namespace

Before writing code, understand the new names this refactor introduces.
Old names deprecate but persist as forwarders until Step 5.

### 2.1 Contributor IDs

```
  CONTRIB_TERRAIN_LATTICE        // internal to contrib_static_base_at
  CONTRIB_TILE_MODIFIERS          // internal to contrib_static_base_at
  CONTRIB_SOLIDS                  // piers, ramps; internal to contrib_static_base_at
  CONTRIB_PYRAMIDS
  CONTRIB_PAINTINGS_BASES
  CONTRIB_VEGETATION_BASES
  CONTRIB_GOL_ZONES               // slow_dynamic
  CONTRIB_TERRAIN_WAVES           // deformation_field, global
  CONTRIB_RADIAL_PULSES           // deformation_field, global
  CONTRIB_PAWN_AURA               // deformation_field, global pawn-centered
  CONTRIB_GOL_SUPPRESSION         // deformation_field, consumer-local
```

The first three combine into a single base eval (`contrib_static_base_at`)
because the current code composes `lattice × mods + solids`
multiplicatively and extracting them separately would require a
non-trivial refactor of contributors for no benefit.

### 2.2 Contributor eval functions

One per contributor id (except the three fused into the static base):

```wgsl
fn contrib_static_base_at(xz: vec2<f32>) -> f32;      // lattice×mods+solids
fn contrib_pyramids_at(xz: vec2<f32>) -> f32;
fn contrib_paintings_base_at(xz: vec2<f32>) -> f32;
fn contrib_vegetation_base_at(xz: vec2<f32>) -> f32;
fn contrib_gol_zones_at(xz: vec2<f32>) -> f32;        // no suppression
fn contrib_terrain_waves_at(xz: vec2<f32>) -> f32;
fn contrib_radial_pulses_at(xz: vec2<f32>, t_seconds: f32) -> f32;
fn contrib_pawn_aura_at(xz: vec2<f32>) -> f32;        // reads pawn_state internally
fn contrib_gol_suppression_at(xz: vec2<f32>,
                              consumer_pos: vec3<f32>) -> f32;
```

Each is a thin wrapper around the existing logic in the first pass.
Do not refactor the internals in this pass — preserve their current
behavior exactly.

### 2.3 Policy IDs

```
  POLICY_PLACEMENT_PYRAMID
  POLICY_PLACEMENT_PAINTING
  POLICY_PLACEMENT_VEGETATION
  POLICY_BAKED_HEIGHTFIELD
  POLICY_FLYER
  POLICY_WALKER
  POLICY_WALKER_AGENT
  POLICY_CELESTIAL
```

Contributor sets per policy: see design doc §5.1. Encode these as
**compile-time constants** — one constant per policy, each a bitmask
of contributor ids or a small enum-indexed array. The per-policy
query specialization reads its own constant at compile time.

### 2.4 Query inputs and query functions

`QueryInputs` bundles per-caller state, so future additions don't
break call signatures:

```wgsl
struct QueryInputs {
    consumer_pos: vec3<f32>,
    t_seconds:    f32,
}
```

Query functions — **specialized per policy**. Each policy gets its
own entry point:

```wgsl
fn query_ground_flyer(xz: vec2<f32>, qi: QueryInputs) -> f32;
fn query_ground_walker(xz: vec2<f32>, qi: QueryInputs) -> f32;
fn query_ground_walker_agent(xz: vec2<f32>, qi: QueryInputs) -> f32;
fn query_ground_placement_pyramid(xz: vec2<f32>) -> f32;
fn query_ground_placement_painting(xz: vec2<f32>) -> f32;
fn query_ground_placement_vegetation(xz: vec2<f32>) -> f32;
fn query_ground_baked_heightfield(xz: vec2<f32>) -> f32;
fn query_ground_celestial(xz: vec2<f32>) -> f32;  // returns 0.0 always

// gradient variants where needed
fn query_ground_walker_gradient(
    xz: vec2<f32>, qi: QueryInputs, eps: f32) -> vec3<f32>;
fn query_ground_walker_walkable(
    xz: vec2<f32>, qi: QueryInputs, eps: f32, step_h: f32) -> vec3<f32>;
fn query_ground_flyer_gradient(
    xz: vec2<f32>, qi: QueryInputs, eps: f32) -> vec3<f32>;
```

Placement policies don't take `QueryInputs` because they don't
include any deformation fields. Celestial just returns 0.0 — kept
for symmetry and future celestial entity use.

---

## 3. GoL Suppression — The Non-Trivial Extraction

This is the one place in the refactor where the naive
"subtract a suppression contribution" doesn't map cleanly onto the
current code. Read this section fully before touching
`zone_gol_height_at`.

### 3.1 Current behavior

`zone_gol_height_at` at `world.wgsl:1855–1884`:

```wgsl
var h = visual * zp.alive_height * height_factor * config.mode_gol_height_scale;
let pawn_dist = distance(world_xz, pawn_state.pos.xz);
let suppression = 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER, pawn_dist);
h *= (1.0 - suppression);
return h;
```

The suppression is **multiplicative on `h`**. Mathematically:
`h * (1 - supp) == h - h * supp`, so it can be expressed additively.

### 3.2 The extraction

After extraction:

- `contrib_gol_zones_at(xz)` returns the raw `h` (the GoL height
  contribution computed through `visual × alive_height × …`),
  **without suppression**.
- `contrib_gol_suppression_at(xz, consumer_pos)` returns the
  suppression *amount* — the quantity to subtract from `h`. It
  evaluates:

  ```
  suppression_amount = contrib_gol_zones_at(xz)
                        * smoothstep_factor(distance(xz, consumer_pos.xz))
  ```

  where `smoothstep_factor = 1 - smoothstep(ZONE_SUPPRESS_INNER,
  ZONE_SUPPRESS_OUTER, dist)`.

- The composition: `h_net = contrib_gol_zones_at(xz) - contrib_gol_suppression_at(xz, consumer_pos)`
  which equals `h * (1 - supp_factor)` — identical to the old result.

### 3.3 Why this double-evaluates GoL

Both `contrib_gol_zones_at` and `contrib_gol_suppression_at` evaluate
the raw GoL height internally. This is redundant work. Two options:

**Option A (simpler):** accept the double evaluation. The GoL
evaluation is cheap (a few texture reads / buffer reads + a multiply).
At this point in the execution we're not profiling, just refactoring.
Use option A unless profile data justifies the complexity.

**Option B (if needed later):** the query function fuses the two —
computes raw GoL once, then applies suppression in the same
function, never calling `contrib_gol_suppression_at` as a separate
function. The "contributor" remains a logical entity in the policy
registry but its implementation is inlined into the query
specialization. This is what the walker query ends up doing anyway
under specialization.

**Decision for this brief: use Option A.** Each contributor is a
separate function. Optimization is a separate concern tracked in the
design doc's open questions (§8).

### 3.4 Sanity check

After extraction, standing the pawn adjacent to a live GoL zone must
produce **identical** visual behavior to pre-refactor. The GoL lift
should flatten near the pawn with the same shape and radius. Test
this before committing Step 3 (§5.3 below).

---

## 4. Time Input — The Shader-Stage Wrinkle

`render_signal.t_seconds` is the binding name in rendering stages;
`signal.t_seconds` is the name in compute stages. A contributor that
reads time cannot use either name directly and work in both stages.

**The fix:** `contrib_radial_pulses_at` takes `t_seconds` as an
explicit parameter. The query functions take `QueryInputs` (§2.4),
and each caller fills `qi.t_seconds` with whatever is visible in
its shader stage:

```wgsl
// In a vertex/fragment shader:
let qi = QueryInputs(pawn_state.pos, render_signal.t_seconds);

// In a compute shader:
let qi = QueryInputs(pawn_state.pos, signal.t_seconds);
```

Placement queries don't need time (no deformation fields), so they
skip `QueryInputs` entirely per §2.4.

---

## 5. Execution Steps

Six steps. After each, **compile and run**. Visual parity is the
checkpoint for Steps 1–4; Step 4b produces the single visible bug
fix.

### Step 1 — Registry scaffolding

**Goal:** declare contributor ids, policy ids, the DAG, and the
policy table. Nothing called yet.

**Create new file:** `modules/ground_architecture.inl`. Include it
from `cartridge.hpp` in an appropriate place (see §1.2 for the
pattern; likely near the other contributor-related includes around
`pawn_aura.inl`).

**Define in the new file:**

```cpp
// C++ side — contributor ids and policy ids as enums.
enum ContributorId : uint32_t {
    CONTRIB_TERRAIN_LATTICE   = 0,
    CONTRIB_TILE_MODIFIERS    = 1,
    CONTRIB_SOLIDS            = 2,
    CONTRIB_PYRAMIDS          = 3,
    CONTRIB_PAINTINGS_BASES   = 4,
    CONTRIB_VEGETATION_BASES  = 5,
    CONTRIB_GOL_ZONES         = 6,
    CONTRIB_TERRAIN_WAVES     = 7,
    CONTRIB_RADIAL_PULSES     = 8,
    CONTRIB_PAWN_AURA         = 9,
    CONTRIB_GOL_SUPPRESSION   = 10,
    CONTRIB_COUNT             = 11,
};

enum PolicyId : uint32_t { /* ... per §2.3 ... */ };

// The DAG — explicit edges. Used for topo sort and validation.
struct ContributorEdge { ContributorId from, to; };
static constexpr ContributorEdge CONTRIBUTOR_DAG[] = {
    { CONTRIB_TERRAIN_LATTICE,  CONTRIB_PYRAMIDS },
    { CONTRIB_TILE_MODIFIERS,   CONTRIB_PYRAMIDS },
    { CONTRIB_SOLIDS,           CONTRIB_PYRAMIDS },
    { CONTRIB_PYRAMIDS,         CONTRIB_PAINTINGS_BASES },
    { CONTRIB_SOLIDS,           CONTRIB_PAINTINGS_BASES },
    { CONTRIB_SOLIDS,           CONTRIB_VEGETATION_BASES },
    // Note: CONTRIB_TERRAIN_LATTICE, TILE_MODIFIERS, SOLIDS are
    // fused into contrib_static_base_at; edges still declared so
    // policy validation works.
};

// Policy definition — contributor set as a bitmask.
struct PolicyDef {
    PolicyId id;
    const char* name;
    uint32_t contributors;   // bitmask indexed by ContributorId
    bool gradient_supported;
};
constexpr uint32_t contrib_bit(ContributorId c) { return 1u << (uint32_t)c; }

static constexpr PolicyDef POLICIES[] = {
    { POLICY_PLACEMENT_PYRAMID, "placement_pyramid",
      contrib_bit(CONTRIB_TERRAIN_LATTICE) | contrib_bit(CONTRIB_TILE_MODIFIERS)
      | contrib_bit(CONTRIB_SOLIDS),
      false },
    // ... see design doc §5.1 for contributor sets per policy ...
};

// Init-time validation: verify every policy's contributor set is
// closed under DAG dependencies. Write a constexpr validator if
// feasible; otherwise a static_assert-based check on each policy.
```

**In WGSL (add to `world.wgsl`):** mirror the contributor ids as
WGSL `const` values so shader code can reference them by symbol:

```wgsl
const CONTRIB_TERRAIN_LATTICE: u32  = 0u;
// ... etc.

const POLICY_FLYER_MASK: u32 = /* precomputed bitmask for flyer */;
// ... etc.
```

Place these near the top of the ground section of `world.wgsl`
(around line 1887, where the old three-level comment block is).

**Verify:**

- Build succeeds.
- Compile-time validation of policy closure under DAG passes.
- Running the program: **zero visual change** (nothing calls the new
  machinery yet).

**Commit message suggestion:** `ground: Step 1 - contributor and
policy registry scaffolding`.

### Step 2 — Contributor extraction

**Goal:** create the thin `contrib_*_at` wrappers around existing
contributor logic. Existing functions become aliases that forward
to the new names.

**In `world.wgsl`:**

1. **`contrib_static_base_at`** — new function, body is the current
   body of `ground_terrain` (line 1910). `ground_terrain` becomes a
   thin forwarder.
2. **`contrib_pyramids_at`** — rename of `pyramid_height_at` (line
   1843). Add a forwarding alias for `pyramid_height_at`.
3. **`contrib_paintings_base_at`** — new. First pass: placeholder
   returning 0.0 (no actual paintings contributor today; the
   placeholder reserves the slot and the policy). Add a comment
   marking it as a stub.
4. **`contrib_vegetation_base_at`** — new. Same: placeholder
   returning 0.0. Stub comment.
5. **`contrib_gol_zones_at`** — extract from `zone_gol_height_at`
   (lines 1855–1884). The new function has the body of the old one
   **minus the suppression block** (lines 1875–1880). Return the
   raw `h`.
6. **`contrib_gol_suppression_at`** — new. Body evaluates raw GoL
   at `xz` (same as `contrib_gol_zones_at`) then applies the
   `smoothstep_factor` using `consumer_pos.xz` instead of
   `pawn_state.pos.xz`. Returns the subtractive amount per §3.
7. **`contrib_terrain_waves_at`** — rename of `terrain_wave_overlay`
   (line 1977). Add forwarding alias.
8. **`contrib_radial_pulses_at`** — rename of `evaluate_radial_pulses`
   (line 2092). Signature `(xz, t_seconds)`. Add forwarding alias.
9. **`contrib_pawn_aura_at`** — new. Body is
   `sample_pawn_aura(xz, pawn_state.pos.xz).r * config.pawn_aura_height`.
   Reads `pawn_state.pos` internally.
10. **`zone_gol_height_at` becomes a forwarder.** Its body becomes
    `return contrib_gol_zones_at(xz) - contrib_gol_suppression_at(xz, vec3(pawn_state.pos.xy, 0.0));`
    **Wait — check the signature.** `consumer_pos` is `vec3<f32>` in
    the new API; `pawn_state.pos` in current code is `vec3<f32>`
    already. Just pass it through.

**Verify:**

- Build succeeds.
- **Pawn adjacent to a live GoL zone: identical visual behavior.**
  This is the Step 2 checkpoint that specifically exercises the
  suppression extraction. The pawn should see the same flattened
  GoL dip as before.
- No other visual change anywhere.

**Commit:** `ground: Step 2 - contributor extraction with GoL
suppression split`.

### Step 3 — Query API implementation

**Goal:** implement `query_ground_*` per-policy specializations.
Still nobody calls them.

**In `world.wgsl`, below the contributor definitions:**

Define `QueryInputs` struct. Then, for each policy, write a
specialization:

```wgsl
fn query_ground_flyer(xz: vec2<f32>, qi: QueryInputs) -> f32 {
    var h = contrib_static_base_at(xz);
    h += contrib_pyramids_at(xz);
    // paintings/vegetation are placement-only, not included in flyer
    h += contrib_gol_zones_at(xz);
    h += contrib_terrain_waves_at(xz);
    h += contrib_radial_pulses_at(xz, qi.t_seconds);
    h += contrib_pawn_aura_at(xz);
    return h;
}

fn query_ground_walker(xz: vec2<f32>, qi: QueryInputs) -> f32 {
    var h = contrib_static_base_at(xz);
    h += contrib_pyramids_at(xz);
    h += contrib_gol_zones_at(xz);
    h += contrib_terrain_waves_at(xz);
    h += contrib_radial_pulses_at(xz, qi.t_seconds);
    h += contrib_pawn_aura_at(xz);
    h -= contrib_gol_suppression_at(xz, qi.consumer_pos);
    return h;
}

fn query_ground_walker_agent(xz: vec2<f32>, qi: QueryInputs) -> f32 {
    // Same as walker MINUS gol_suppression — agents feel full GoL lift.
    var h = contrib_static_base_at(xz);
    h += contrib_pyramids_at(xz);
    h += contrib_gol_zones_at(xz);
    h += contrib_terrain_waves_at(xz);
    h += contrib_radial_pulses_at(xz, qi.t_seconds);
    h += contrib_pawn_aura_at(xz);
    return h;
}

fn query_ground_placement_pyramid(xz: vec2<f32>) -> f32 {
    // Just the base — pyramids don't see themselves.
    return contrib_static_base_at(xz);
}

fn query_ground_placement_painting(xz: vec2<f32>) -> f32 {
    var h = contrib_static_base_at(xz);
    h += contrib_pyramids_at(xz);
    h += contrib_gol_zones_at(xz);  // slow-dynamic; paintings sit on current
    return h;
}

fn query_ground_placement_vegetation(xz: vec2<f32>) -> f32 {
    return contrib_static_base_at(xz);  // trees don't stand on pyramids
}

fn query_ground_baked_heightfield(xz: vec2<f32>) -> f32 {
    // What the baked heightfield texture caches — all static, no dynamic,
    // no deformation.
    var h = contrib_static_base_at(xz);
    h += contrib_pyramids_at(xz);
    return h;
}

fn query_ground_celestial(_xz: vec2<f32>, _qi: QueryInputs) -> f32 {
    return 0.0;
}
```

Then the gradient variants for the policies that need them
(walker, flyer):

```wgsl
fn query_ground_flyer_gradient(xz: vec2<f32>, qi: QueryInputs, eps: f32) -> vec3<f32> {
    let h   = query_ground_flyer(xz, qi);
    let hpx = query_ground_flyer(xz + vec2(eps, 0.0), qi);
    let hmx = query_ground_flyer(xz - vec2(eps, 0.0), qi);
    let hpz = query_ground_flyer(xz + vec2(0.0, eps), qi);
    let hmz = query_ground_flyer(xz - vec2(0.0, eps), qi);
    let ddx = (hpx - hmx) * (0.5 / eps);
    let ddz = (hpz - hmz) * (0.5 / eps);
    return vec3(h, ddx, ddz);
}

fn query_ground_walker_gradient(xz: vec2<f32>, qi: QueryInputs, eps: f32) -> vec3<f32> {
    // Same pattern, walker specialization.
}

fn query_ground_walker_walkable(xz: vec2<f32>, qi: QueryInputs, eps: f32, step_h: f32) -> vec3<f32> {
    // Body equivalent to effective_ground_with_gradients_walkable
    // (world.wgsl:2132) but with query_ground_walker instead of
    // effective_ground_y.
}
```

The existing `effective_ground_with_gradients` (line 2122) and
`effective_ground_with_gradients_walkable` (line 2132) stay for now
as forwarders to the walker variants.

**Verify:**

- Build succeeds.
- Zero visual change (no migrated callers yet).

**Commit:** `ground: Step 3 - query API specializations per policy`.

### Step 4 — Consumer migration

Each sub-step migrates one group of consumers. Commit and verify
after each.

#### Step 4a — Placement consumers

**Scope:** every CPU-side or GPU-side call that does spawn-time
height sampling for placement of a pyramid, arch, column, painting,
tree, or similar static landform.

**Current call sites:** grep for `ground_terrain`, `ground_formed`,
`effective_ground_y`, `estimate_terrain_height` in both `world.wgsl`
and `cartridge.hpp` + `.inl` files. Identify which are placement-
time queries (usually in spawn engines, entity placement loops,
heightfield baking code).

**Migration:** replace with the appropriate `query_ground_placement_*`
call. For CPU-side `estimate_terrain_height`, **do not migrate** —
see §1.3 and design doc §6.2. Leave the CPU fast path as-is with a
comment noting it's not a policy query.

**Verify:**

- Build succeeds.
- Entities (pyramids, paintings, trees, columns, arches) spawn in
  the same positions as pre-refactor. Walk around and compare to a
  pre-refactor build; pay attention to landforms that sit near tile
  boundaries or on piers.

**Commit:** `ground: Step 4a - migrate placement consumers to policies`.

#### Step 4b — Fly-over consumers — the bug fix

**Scope:**

- **Sphere terrain clearance coupling** in
  `coupling_terrain_to_sphere_orbit_height` (around line 2165 or
  wherever the sphere's orbit-y clamp reads
  `ground_formed + terrain_wave_overlay`). Migrate to
  `query_ground_flyer`. This **adds** radial pulse and pawn aura
  awareness to sphere clearance.
- **Cube hover base** in `update_cube` at line 4673:
  `let ground = ground_formed(base_xz) + terrain_wave_overlay(base_xz);`
  Migrate to `query_ground_flyer`. This adds pulse/aura awareness.
- **Camera eye altitude clamp** around line 5863 in
  `build_shadow_vp` or the equivalent camera-clamp code. Today uses
  `sample_terrain_y_at` — the baked heightfield. The design doc
  wanted this at `POLICY_FLYER`. **Design call to make here:** does
  the camera clamp want to ride pulses? I think yes (camera shouldn't
  clip into a pulse-lifted ridge) but confirm against user intent.
  If in doubt, leave as `POLICY_BAKED_HEIGHTFIELD` and flag in the
  commit message.
- **Shadow VP ground reference** — if shadow rendering samples
  terrain height to position its view frustum, use
  `POLICY_FLYER`.

**Verify:**

- Build succeeds.
- **Standing a cube above a spot where a radial pulse is rolling
  through:** the cube now rises with the pulse instead of sitting
  inside the lifted ground. This is the visible fix.
- **Spheres orbiting over pulses:** same — orbit clearance rises
  with the pulse.
- No other visible change.
- The patch terrain still looks correct — we're not touching its
  inline composition.

**Commit:** `ground: Step 4b - migrate fly-over consumers, fix
cube/sphere pulse clipping`.

#### Step 4c — The walker

**Scope:** pawn ground resolve in `pawn_ground_resolve` and
`update_pawn` (around lines 2100–2165, 4400–4500). Today uses
`effective_ground_y`, `effective_ground_with_gradients_walkable`,
and adds pulse/wave/aura separately.

**Migration:** replace the entire composition with a single
`query_ground_walker_walkable` call for the walkable case, or
`query_ground_walker` / `query_ground_walker_gradient` for the
height / gradient cases.

Carefully read the step-climb logic in `pawn_ground_resolve` (lines
~2140, 4412–4423) before touching. It currently does:

1. Sample heights at the desired XZ and a few axis-aligned alternatives.
2. Reject steps that exceed `PAWN_STEP_HEIGHT`.
3. Apply fallback (axis slide) logic.

The sampling calls migrate; the step-climb *logic* is unchanged.
Just swap the sampler.

**Verify:**

- Build succeeds.
- **Walk around everywhere.** The pawn should behave identically —
  same tilt on slopes, same step rejection at cliffs, same GoL
  suppression around it, same aura extrusion under its feet, same
  rise when a pulse passes through.
- This is the delicate step. Give it extra verification time. Consider
  a side-by-side video if possible.

**Commit:** `ground: Step 4c - migrate pawn to POLICY_WALKER`.

#### Step 4d — The baked heightfield

**Scope:** heightfield baking code — wherever the baked heightfield
texture is populated, likely in patch heightfield generation. Uses
`ground_formed` today.

**Migration:** use `query_ground_baked_heightfield`. The contributor
set matches exactly what the old function did, so behavior is
identical.

Also the patch terrain VS inline composition (`world.wgsl:2840–2855`):
**do not refactor this into a query call.** It's hand-fused for
per-vertex performance. Add a comment block above it referencing
the policy system:

```wgsl
// The patch terrain VS is a hand-fused POLICY_WALKER-style evaluation.
// It inlines contrib_static_base + pyramids + gol_zones + waves
// + pulses + aura for per-vertex performance. Must stay consistent
// with the POLICY_WALKER contributor set. See ground_hierarchy_design.md §8.
```

**Verify:**

- Build succeeds.
- Baked heightfield texture content unchanged.
- Patch terrain rendering unchanged.

**Commit:** `ground: Step 4d - migrate baked heightfield consumer`.

### Step 5 — Remove deprecated aliases

**Goal:** delete the forwarder functions introduced in Step 2.

**Scope:**

- `ground_terrain`, `ground_formed`, `ground_formed_with_complexity`,
  `effective_ground_y`, `effective_ground_with_gradients`,
  `effective_ground_with_gradients_walkable` in `world.wgsl`.
- `zone_gol_height_at` in `world.wgsl`.
- `terrain_wave_overlay` — only if no external call site remains.
  (grep to confirm.)
- `evaluate_radial_pulses` — same.
- `pyramid_height_at` — same.

**Before deleting each:** `grep -n <name>` in `world.wgsl`,
`cartridge.hpp`, and all `.inl` files to confirm no callers remain.
If grep finds callers, migrate them (some will be things Step 4
missed).

The build failing on a missed call site is the safety net — that's
how we find stragglers.

**Verify:**

- Build succeeds.
- Zero visual change.

**Commit:** `ground: Step 5 - remove deprecated forwarder aliases`.

### Step 6 — Documentation

**Goal:** the ground section of `world.wgsl` explains the
architecture to anyone reading the file.

**Actions:**

1. Rewrite the comment block at `world.wgsl:1887–1908` (the old
   three-level hierarchy block) into a new block that describes the
   contributor graph, classes, DAG, and policies. Reference
   `ground_hierarchy_design.md` for the full design rationale.

2. Above each `contrib_*_at` function, add a header comment naming:
   - Which contributor id it implements.
   - Its class (static_landform, slow_dynamic, deformation_field).
   - Its dependencies (for static landforms).
   - Its scope (global vs. consumer-centered, for deformation fields).

3. Above each `query_ground_*` function, add a header comment naming:
   - The policy it specializes.
   - The contributor set.
   - Typical consumers.

4. In `ground_architecture.inl`, document the registries and the
   init-time validation.

**Verify:**

- Build succeeds.
- No visual change.

**Commit:** `ground: Step 6 - in-code documentation of new
architecture`.

---

## 6. Success Criteria

After Step 6:

1. **Visual parity** for every consumer except cube and sphere
   clearance. Pawn behaves identically. Placement of static
   landforms is identical. Baked heightfield is identical. Terrain
   renders identically.

2. **Cube and sphere now ride radial pulses.** Before: a pulse
   rolling through a cube's spot puts the cube inside the lifted
   ground briefly. After: the cube rises with the pulse and stays
   above.

3. **No new storage buffers** in the vertex stage. No uniform slots
   consumed. No FXC compilation hangs.

4. **Grep for old names finds nothing** (after Step 5). Specifically:
   `grep -rn "effective_ground_y\|ground_formed\|zone_gol_height_at\|pyramid_height_at\|terrain_wave_overlay\|evaluate_radial_pulses" *.hpp *.inl *.wgsl` returns
   only definitions (none of which match the old names anymore), or
   returns only in dead-code/comments that reference the history.

5. **New consumer adding** is documented by example — adding a new
   agent policy, or a new musical-analysis deformation field, is a
   registry edit plus one query specialization, not a codebase
   sweep.

---

## 7. Out of Scope for This Pass

Do not do any of these, even if tempted while touching nearby code:

- **Agent system implementation.** That's a separate pass (see
  `agent_system_design.md`). This refactor prepares the ground
  architecture for it but does not implement it.
- **Floater backbone refactor.** Also a separate pass
  (`floater_backbone_design.md`). The sphere and cube migrations in
  Step 4b touch floater code only at their existing ground-sampling
  sites; do not change the motion model or struct layout.
- **CPU-side ground query hierarchy.** The design doc explicitly
  keeps CPU on the approximate fast path. Do not build a parallel
  registry on the C++ side.
- **The fused inline patch terrain VS.** Keep it inline. Add the
  consistency comment (Step 4d) and move on.
- **Palm-tree-rides-aura / static-landform-rides-deformation.** The
  plumbing is in place after this pass, but migrating each static
  landform's VS to query at render time is a follow-up, one family
  at a time.
- **Per-level or per-policy mute flags.** The coupling registry
  already handles per-contributor muting; no need for a separate
  mechanism.
- **Paintings and vegetation bases as actual contributors.** For
  this pass these are stubs returning 0.0 (§Step 2). Real
  implementations come later when a consumer wants them.
- **Profile-driven optimization.** Don't fuse contributors for
  performance unless something measurably broke. Option A in §3.3.

---

## 8. Questions for the User

Raise these before or during execution. Do not guess, do not defer
silently.

- **Step 4b camera clamp policy.** Does the camera eye-clamp want to
  ride pulses (POLICY_FLYER) or stay on the baked static heightfield
  (POLICY_BAKED_HEIGHTFIELD)? The design doc §5.1 puts it at FLYER;
  this brief defaults to that. Confirm or override before committing
  Step 4b.

- **Step 4a CPU-side sites.** If you encounter a CPU-side caller of
  `estimate_terrain_height` that seems to need more accuracy than
  the fast path provides, **flag it, don't fix it**. The design doc
  deliberately keeps CPU on the approximate fast path. Raising the
  accuracy requirement is a separate design conversation.

- **Unexpected callers.** If Step 5 grep finds unexpected survivors,
  stop and report. The refactor should leave zero stragglers; the
  fact that grep found one means Step 4 missed a site.

- **Visual regressions.** Any visual difference after any step
  except 4b's pulse fix is a regression. Stop and report before
  committing.

---

## 9. Appendix — File Reference Summary

Fast lookup for the files most touched:

| File | Role in refactor |
|---|---|
| `world.wgsl:1768–2180` | Current ground chain — most extraction work |
| `world.wgsl:1855–1884` | `zone_gol_height_at` — suppression split (§3) |
| `world.wgsl:1887–1908` | Old three-level comment block — rewritten in Step 6 |
| `world.wgsl:2092` | `evaluate_radial_pulses` — renamed |
| `world.wgsl:2122, 2132` | Gradient variants — walker migration |
| `world.wgsl:2840–2855` | Patch terrain VS fused composition — **keep inline** |
| `world.wgsl:4116` | `sample_pawn_aura` — wrapped |
| `world.wgsl:4400–4500` | `update_pawn` / `pawn_ground_resolve` — Step 4c |
| `world.wgsl:4673` | `update_cube` hover base — Step 4b |
| `world.wgsl:5787, 5863` | `sample_terrain_y_at`, camera clamp — Step 4d |
| `cartridge.hpp:2639` | `estimate_terrain_height` — CPU fast path, do not touch |
| `cartridge.hpp` (multiple) | `.inl` includes — add `ground_architecture.inl` |
| `modules/ground_architecture.inl` | New file — Step 1 registry |
| `modules/pawn_aura.inl` | Existing pawn aura module — may need minor signature alignment |

---

## 10. How to Work This Brief

- Read the whole brief before Step 1. Re-read each step's section
  before starting that step.
- After each step: commit, verify, and **report progress** before
  proceeding. The user reviews each step's result before the next
  one starts. Do not batch multiple steps into a single commit.
- If anything looks wrong — a callsite doesn't match the brief's
  description, a signature doesn't compose, a test fails — **stop**,
  describe what you see, and wait for direction. The brief is the
  plan, not the territory.
- If a question arises that isn't answered by the brief or the
  design doc, ask. Ambiguity resolved on the conversation is cheaper
  than ambiguity that propagates through a bad commit.
