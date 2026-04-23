# Ground Architecture Design Document

_A contributor graph with explicit dependencies, a set of named policies
that consumers query through, and a clear separation between
placement-time and evaluation-time. Supersedes the five-level hierarchy
sketch; third sibling to `floater_backbone_design.md` and
`agent_system_design.md`; prerequisite for both._

---

## 1. Motivation

The program has no centralized model for what "the ground" means at a
point. There are functions that compose parts of the answer
(`ground_terrain`, `ground_formed`, `effective_ground_y`,
`terrain_wave_overlay`, `evaluate_radial_pulses`, pawn aura), and
each consumer ad-hoc composes the subset it happens to need. A cube
reads `ground_formed + terrain_wave_overlay` — missing pulses, missing
aura, missing GoL. A sphere reads the same combination. The pawn reads
everything. The camera reads the baked heightfield. Pyramid placement
reads a base level, painting placement reads a later level, spawn code
inherits whatever was in scope.

A first framing of this pass proposed a five-level linear stack. That
framing correctly identified the drift but was too rigid for the real
structure: static landforms have genuine dependency edges among
themselves (pyramids on terrain, piers above or below pyramids,
paintings on everything static) while deformation fields like pulses,
waves, and the pawn aura are **orthogonal** to the landform stack —
they don't compete for a slot, they act across the stack.

The correct framing is a **partially-ordered graph of contributors**
and a **set of named query policies** that filter which contributors
a consumer sees. "Level 3" and "level 4" weren't levels; they were
policies. Making that framing explicit is the goal of this pass.

---

## 2. Core Concepts

Four objects define the system. Every system-level behavior reduces to
these four.

### 2.1 Contributor

A named source of height or displacement at a point. Each contributor
has:

- **An `id`** — stable across builds, used for policy declarations
  and the explicit dependency edges.
- **A `class`** — coarse behavioral category (§2.2).
- **An `eval(xz)`** function returning an `f32` delta — the height
  (or displacement) this contributor adds at this world XZ. May take
  additional parameters (e.g. `consumer_pos` for self-centered
  deformation fields).
- **An optional `eval_with_gradient(xz, eps)`** returning
  `vec3<f32>` — the height plus its `∂/∂x` and `∂/∂z` components for
  surface normal construction. Provided where a consumer uses it.

Contributors are **additive**: a ground query is the sum of the
deltas from all included contributors. No contributor's eval ever
reads another contributor's state directly; a contributor with a
dependency reads it *only through the ground query system*, with the
query respecting the dependency edge (§2.3).

### 2.2 Class

Three classes of contributor, distinguished by how they participate
in composition:

- **`static_landform`** — placed once, baked, part of the permanent
  geometry of the world. Has dependency edges on other static
  landforms. Terrain lattice, tile modifiers, piers, pyramids,
  arches, columns, paintings, vegetation. `eval(xz)` is a pure
  function of xz and seed.

- **`slow_dynamic`** — changes over time but not music-modulated,
  frame-to-frame state. Currently only GoL zones. `eval(xz)` reads
  per-frame buffers. No dependency edges.

- **`deformation_field`** — temporally or locally active
  displacement that acts *across* the static/dynamic stack. Not part
  of the DAG. Includes musical overlays (waves, pulses), entity-
  centered deformations (pawn aura), and any future analysis-driven
  terrain modulation. May be global (`eval(xz)`) or self-centered
  (`eval(xz, consumer_pos)`).

### 2.3 Dependency Edge

A directed edge `A → B` means **A is placed beneath B** — equivalently,
B's evaluation assumes A is already present and composes on top of it.
Edges exist only among `static_landform` contributors.

Edges are **explicit**, not inferred from call order. Each static
contributor declares its dependencies in a registry (§4). The system
topologically sorts contributors to establish placement order and to
validate that a query's contributor set is closed under dependencies
(you can't include pyramids without including terrain and piers).

### 2.4 Policy

A **named filter** over the contributor set. A policy specifies:

- Which contributors are in scope.
- For deformation fields that require `consumer_pos`, how
  `consumer_pos` is resolved (typically: the consumer passes its own
  position; for the pawn aura, the aura field itself uses
  `pawn_state.pos` internally regardless of who's querying, because
  the aura is pawn-centered in world space).
- Whether gradient evaluation is available.

A consumer doesn't assemble its ground value by calling contributors.
It declares its policy and calls a single query function. The query
system looks up the policy, fetches the contributors, respects the
DAG, sums the deltas, returns a value.

The policy is part of the consumer's *identity* — a property of the
entity type, the behavior, or the spawn role, not a choice made
per-call-site. This is what makes the architecture resistant to drift:
changing what a consumer sees requires changing its declared policy,
which is a visible architectural act.

---

## 3. Contributors

The full registry, by class. Each entry names the contributor, its
`id`, its dependencies (for static landforms), and the eval shape.

### 3.1 Static landforms

Ordered by the current implicit placement order, formalized here as
the authored DAG.

```
  id                 deps                            eval shape
  ────────────────────────────────────────────────────────────
  terrain_lattice    —                               xz
  tile_modifiers     —                               xz  (multiplier + additive)
  solids             terrain_lattice, tile_modifiers xz  (piers, ramps)
  pyramids           terrain_lattice, tile_modifiers,
                     solids                          xz
  paintings_bases    terrain_lattice, tile_modifiers,
                     solids, pyramids                xz  (flat bases for frames)
  vegetation_bases   terrain_lattice, tile_modifiers,
                     solids                          xz  (planters/tile slots)
```

`terrain_lattice` and `tile_modifiers` are special: they compose
multiplicatively into the base height (`lattice × mods.x + mods.y +
solids`). The DAG records them as having no dependencies because
their eval is self-contained; the composition rule for them is
authored into the query evaluator (§5).

`solids` subsumes piers, ramps, and the structure-height contribution.
Its placement depends on terrain+mods so that decisions about "where
does this pier sit" are made against the bare landscape, not against
later additions.

`pyramids` depend on everything below them, so a pyramid placed in a
region with a pier sees the pier and sits correctly. The piers-below-
pyramids choice is encoded as the edge; flipping it would be an edit
of one line in the registry.

`paintings_bases` and `vegetation_bases` are placement-time static
landforms — they determine spawn-time Y for paintings and trees but
don't themselves contribute geometry the same way piers do. They
exist in the contributor registry so that spawn queries can include
them when asking "where should the next painting go" without
circularity.

### 3.2 Slow-dynamic

```
  id            eval shape
  ────────────────────────────
  gol_zones     xz
```

`gol_zones` has no dependencies; it composes onto the static stack
additively. Today it internally subtracts near the pawn — a walker
concern hidden inside a contributor eval. This pass extracts that
subtraction into a separate deformation field (§3.3).

### 3.3 Deformation fields

Orthogonal to the DAG. Applied additively to the static+dynamic sum
when a policy includes them.

```
  id                    eval shape          scope
  ─────────────────────────────────────────────────────────
  terrain_waves         xz                  global
  radial_pulses         xz, t_seconds       global
  pawn_aura             xz                  global, pawn-centered
  gol_suppression       xz, consumer_pos    consumer-local
```

- `terrain_waves` — the polyphony-driven sinusoidal overlay. Global:
  same effect at XZ regardless of who's asking.
- `radial_pulses` — note-onset ring wavefronts. Global, with an
  implicit time parameter from the signal.
- `pawn_aura` — the terrain lift under the pawn. Global in the sense
  that it affects the world around the pawn regardless of who's
  querying; the pawn's position is an implicit input read inside
  the contributor (from `pawn_state.pos`). A column caught in the
  aura's path rises; a cube hovering over it rises.
- `gol_suppression` — the subtractive contribution that *reduces* the
  GoL zone height near the querying consumer. Consumer-local: it's
  subtracted from the consumer's ground value near the consumer's
  own position, because the intent is "GoL doesn't push me up into
  the air while I'm standing on a zone." This is the one field where
  `consumer_pos` is genuinely the consumer's own position, not a
  world-fixed reference.

**Design call:** `gol_suppression` as a deformation field is
conceptually clean but means every walker-class consumer applies the
suppression at its own position. The current behavior (suppress
around the pawn, period) falls out if agents' policies *don't*
include `gol_suppression` — they feel the full GoL lift, only the
pawn gets the suppression. First pass: only the "walker" policy
(pawn-specific) includes `gol_suppression`. Agent policy excludes it.
Revisit if agents stuck on GoL zones look wrong.

### 3.4 Future contributors

New static landforms are added with their DAG edges declared. New
deformation fields are added to the `deformation_field` class without
edges. A new musical analysis that modulates terrain = one new
deformation field + zero hierarchy changes. A new kind of pier-like
structure = one new static landform + authored edges.

---

## 4. Explicit Dependency DAG

The DAG is a first-class data structure, not an implicit convention.
Authored as:

```cpp
// In a central registry — likely cartridge.hpp or a new ground_graph.inl
struct ContributorEdge {
    ContributorId from;
    ContributorId to;
};

static constexpr ContributorEdge CONTRIBUTOR_DAG[] = {
    { CONTRIB_TERRAIN_LATTICE,   CONTRIB_SOLIDS },
    { CONTRIB_TILE_MODIFIERS,    CONTRIB_SOLIDS },
    { CONTRIB_SOLIDS,            CONTRIB_PYRAMIDS },
    { CONTRIB_PYRAMIDS,          CONTRIB_PAINTINGS_BASES },
    { CONTRIB_SOLIDS,            CONTRIB_PAINTINGS_BASES },
    { CONTRIB_SOLIDS,            CONTRIB_VEGETATION_BASES },
    // ...
};
```

The DAG serves three purposes:

1. **Placement order** — topological sort produces the order in which
   landforms are placed at spawn time. The current spawn code's
   ordering becomes the DAG's topo sort; making the DAG explicit
   changes nothing about what happens, only about what's checkable.

2. **Policy closure validation** — when a policy declares its
   contributor set, a compile-time (or init-time) check verifies the
   set is closed under dependencies. A policy that includes
   `pyramids` but not `terrain_lattice` is rejected before it can
   silently produce wrong geometry.

3. **Documentation as truth** — "does pyramids see piers?" is
   answered by looking at the DAG, not by reading placement code.

The DAG does not order deformation fields. Deformation fields compose
with the static+dynamic sum additively, with whatever order within
the deformation-field set falls out of iteration (currently
commutative — all additive).

**Non-commutativity in deformation fields** is an open issue (§8).
First pass assumes all deformation fields commute. If a future
analysis contributes non-commutatively (multiplicatively, or with
conditional application), the deformation-field set grows an order
annotation.

---

## 5. Policies

A policy is a named filter. Each policy specifies:

- **A set of contributor ids** to include.
- **A gradient flag** — whether the policy supports
  `eval_with_gradient`.
- **Contributor-specific arg resolution** — for deformation fields
  that need extra parameters, how those are resolved for this policy.

Policies are declared in a central registry:

```cpp
struct PolicyDef {
    PolicyId id;
    const char* name;
    ContributorMask contributors;  // bitmask or enum-indexed array
    bool gradient_supported;
    // ... (arg-resolution metadata)
};
```

### 5.1 The initial policy catalog

Eight policies cover the current and near-future consumer set.

```
  policy_id                   who queries with it              contributors
  ────────────────────────────────────────────────────────────────────────────
  POLICY_PLACEMENT_PYRAMID    pyramid spawn                    terrain_lattice,
                                                               tile_modifiers,
                                                               solids
  POLICY_PLACEMENT_PAINTING   painting spawn                   all static, no dynamic,
                                                               no deformation
  POLICY_PLACEMENT_VEGETATION tree/column/arch spawn           all static, no dynamic,
                                                               no deformation
  POLICY_BAKED_HEIGHTFIELD    terrain VS source texture        all static, no dynamic,
                                                               no deformation
                                                               (this is what the VS
                                                               texture caches)
  POLICY_FLYER                sphere clearance, cube home,     all static, slow_dynamic,
                              camera clamp, shadow VP          terrain_waves,
                                                               radial_pulses, pawn_aura.
                                                               No gol_suppression
                                                               (flyers don't get it).
  POLICY_WALKER               pawn ground resolve              all static, slow_dynamic,
                                                               all deformation fields
                                                               including gol_suppression
                                                               with consumer_pos = pawn_pos
  POLICY_WALKER_AGENT         agent ground resolve             same as POLICY_WALKER
                                                               but gol_suppression
                                                               excluded (§3.3 design call)
  POLICY_CELESTIAL            no-ground entities               empty contributor set —
                                                               the ground is 0.0
```

Policies are authored once. A consumer declares its policy; the
engine resolves the policy to a contributor set at init time; the
query function dispatches against the set.

### 5.2 Policies and placement vs. evaluation

A key distinction: a consumer may query with **different policies at
different lifecycle stages**. The canonical example is the palm tree:

- At **spawn time**, the palm queries with
  `POLICY_PLACEMENT_VEGETATION` — it wants a stable Y on the static
  stack, ignoring deformations that would teleport it around at
  placement.
- At **render time**, the palm queries with a policy that includes
  deformation fields — so when the pawn aura or a pulse passes
  through, the palm rises with them.

This is the "palm tree caught by aura" behavior made explicit. Same
for a column, arch, painting — any static landform whose rendered Y
should respond to deformations applied after placement.

This splits the current single "ground query" into two: **spawn-time
query** (usually excludes deformations and sometimes excludes newer
static layers) and **render-time query** (usually includes
deformations). Consumers with render-time queries carry a
`render_policy` field; consumers with spawn-time queries carry a
`spawn_policy` field; consumers that have both — paintings are the
clearest example — carry both.

### 5.3 Gradient support

Some policies need gradients (walkers for tilt, cameras for
aim-direction clamp). Others don't (placement policies just want
height). The gradient flag in the policy definition tells the engine
whether to provide `query_ground_with_gradient(xz, eps, policy)`.

Gradient evaluation is 5× the plain cost (center + 4 neighbors) plus
any additional tap overhead for deformation fields. The walker policy
pays this; placement policies don't, even though they'd technically
work.

### 5.4 Walkability

The pawn's `effective_ground_with_gradients_walkable` clamps
gradients at cliffs. That's a **variant of the gradient evaluation**,
not a separate contributor set. Adding a `walkable` flag to
`query_ground_with_gradient` — third argument after `eps`, or a
separate function — handles this. Walker policies enable it; fly-over
policies don't need it.

---

## 6. The Query API

The system exposes three call shapes. Every consumer uses one of
these; no consumer composes contributors by hand.

```wgsl
// Plain height — static + dynamic + deformation, per the policy.
fn query_ground(xz: vec2<f32>, policy: PolicyId,
                consumer_pos: vec3<f32>) -> f32;

// Height + gradient (∂/∂x, ∂/∂z).
// Requires policy to have gradient_supported.
fn query_ground_gradient(xz: vec2<f32>, policy: PolicyId,
                         consumer_pos: vec3<f32>, eps: f32) -> vec3<f32>;

// Height + gradient, with cliff-clamping for walkers.
fn query_ground_walkable(xz: vec2<f32>, policy: PolicyId,
                         consumer_pos: vec3<f32>,
                         eps: f32, step_height: f32) -> vec3<f32>;
```

`consumer_pos` is always present. Global deformation fields ignore
it; self-centered ones (today: `gol_suppression`) use it.

Consumer code becomes:

```wgsl
// Pawn ground resolve (walker with tilt)
let g = query_ground_walkable(pos.xz, POLICY_WALKER, pawn_state.pos,
                              0.5, PAWN_STEP_HEIGHT);
pos.y = g.x;
let normal = normalize(vec3(-g.y, 1.0, -g.z));

// Cube hover base (flyer)
let h = query_ground(anchor.xz, POLICY_FLYER, fe.pos);
pos.y = h + orbit_height + bob;

// Pyramid placement (spawn-time)
let h = query_ground(cand_xz, POLICY_PLACEMENT_PYRAMID, vec3(0.0));
```

No consumer calls `terrain_height_at`, `tile_modifiers_at`,
`pyramid_height_at`, `terrain_wave_overlay`, `evaluate_radial_pulses`,
or `sample_pawn_aura` directly. Those become internal contributor
implementations, not public functions.

### 6.1 Internals

Behind the query API, implementation is straightforward:

```wgsl
// Pseudocode — actual impl will be specialized per policy for FXC friendliness.
fn query_ground(xz: vec2<f32>, policy: PolicyId,
                consumer_pos: vec3<f32>) -> f32 {
    var h: f32 = 0.0;

    // Static landforms — evaluated in DAG topo order.
    if (policy_includes(policy, CONTRIB_TERRAIN_LATTICE)) {
        h = contrib_static_base_at(xz);  // lattice × mods + solids (fused)
    }
    if (policy_includes(policy, CONTRIB_PYRAMIDS)) { h += contrib_pyramids_at(xz); }
    if (policy_includes(policy, CONTRIB_PAINTINGS_BASES))  { h += contrib_paintings_base_at(xz); }
    if (policy_includes(policy, CONTRIB_VEGETATION_BASES)) { h += contrib_vegetation_base_at(xz); }

    // Slow-dynamic.
    if (policy_includes(policy, CONTRIB_GOL_ZONES)) { h += contrib_gol_zones_at(xz); }

    // Deformation fields.
    if (policy_includes(policy, CONTRIB_TERRAIN_WAVES))   { h += contrib_terrain_waves_at(xz); }
    if (policy_includes(policy, CONTRIB_RADIAL_PULSES))   { h += contrib_radial_pulses_at(xz); }
    if (policy_includes(policy, CONTRIB_PAWN_AURA))       { h += contrib_pawn_aura_at(xz); }
    if (policy_includes(policy, CONTRIB_GOL_SUPPRESSION)) { h -= contrib_gol_suppression_at(xz, consumer_pos); }

    return h;
}
```

In practice this will be specialized per policy at compile time
(WGSL `const` evaluation on the contributor mask, then dead-branch
elimination in FXC) so each consumer pays only for the contributors
in its set. FXC uniform-branching is preserved because `policy` is a
compile-time constant per consumer.

### 6.2 CPU-side story

The CPU does not grow a parallel query system. `estimate_terrain_height`
stays as the approximate fast path with an honest comment. CPU
consumers that need accurate height perform a GPU readback against
the baked heightfield (which is `POLICY_BAKED_HEIGHTFIELD` — all
static, no dynamic, no deformation). The policy name makes it clear
what the heightfield does and doesn't capture.

If a CPU consumer ever needs accurate height *including* deformations
(unlikely but flagged), the right design is a readback of a
separately-baked dynamic texture, or a per-frame CPU-side evaluation
of deformation fields over a small XZ neighborhood. Out of scope.

---

## 7. Migration Plan

Single pass, six steps. Each step has a clean verify-build checkpoint.
Visual-parity is the contract for steps 1–5; step 6 is where the
cube/sphere pulse fix becomes visible.

### Step 1 — Registry scaffolding

- Define `ContributorId` enum with all current contributors.
- Define `PolicyId` enum with all planned policies.
- Define `CONTRIBUTOR_DAG` edge list.
- Define `POLICIES` array mapping PolicyId → contributor mask +
  flags.
- Validate at init: every policy's contributor set is closed under
  DAG dependencies.

Nothing in the hot path is called yet; the registries sit defined.
Verify: build succeeds, init-time validation passes.

### Step 2 — Contributor extraction

- For each contributor in §3, define its eval function with a
  standardized name: `contrib_<id>_at(xz[, args])`. Existing
  functions either get renamed or become thin wrappers.
- `pyramid_height_at` → `contrib_pyramids_at`
- `zone_gol_height_at` (without suppression) → `contrib_gol_zones_at`
- `sample_pawn_aura * config.pawn_aura_height` → `contrib_pawn_aura_at`
- `terrain_wave_overlay` → `contrib_terrain_waves_at`
- `evaluate_radial_pulses` → `contrib_radial_pulses_at` (time input
  encapsulated internally)
- GoL suppression extracted into `contrib_gol_suppression_at` with
  `(xz, consumer_pos)` signature
- Solids / terrain_lattice / tile_modifiers composed into
  `contrib_static_base_at(xz)` (the lattice × mods + solids
  composition, which is inseparable in the current shader)

Old names remain as forwarding aliases temporarily. Verify: build
succeeds, no visual change.

### Step 3 — Query API implementation

- Define `query_ground`, `query_ground_gradient`,
  `query_ground_walkable`.
- Implement the dispatch against the contributor mask.
- Specialize per policy (one function per policy, dispatched via
  compile-time const `PolicyId`) to keep FXC happy.

Still nobody calls it; the API exists. Verify: build succeeds, no
visual change.

### Step 4 — Consumer migration, policy-by-policy

Subdivided into committable units, each one group of consumers:

- **Step 4a — Placement consumers.** Pyramid/arch/column/painting/
  tree spawn code migrated to `POLICY_PLACEMENT_*`. Largest
  callsite count but simplest behaviorally.
- **Step 4b — Fly-over consumers.** Sphere clearance, cube home,
  camera clamp, shadow VP. Migrated to `POLICY_FLYER`. **This is
  where the cube/sphere/pulse bug fix lands** — the visible
  improvement of the whole pass.
- **Step 4c — The walker.** Pawn ground resolve migrated to
  `POLICY_WALKER`. Delicate step; verify pawn behavior against a
  pre-refactor build.
- **Step 4d — The baked heightfield.** Terrain VS and its source
  texture use `POLICY_BAKED_HEIGHTFIELD`. Verify the heightfield
  texture still matches its source.

Each sub-step: migrate, verify, commit. Old function names get
deprecated-but-kept through the whole migration; a final pass removes
them.

### Step 5 — Remove deprecated aliases

`effective_ground_y`, `zone_gol_height_at` (the pawn-suppressing
variant), any other forwarders — deleted. If a call site was missed,
the build fails, making the oversight findable.

### Step 6 — Document the registry as the source of truth

The header comment block on the ground section of world.wgsl is
rewritten to reflect the new architecture: contributors, classes,
DAG, policies, extension rules.

**Success criterion for the whole pass**: visual parity on every
consumer except cube and sphere clearance, which now correctly ride
radial pulses (previously, a pulse wavefront passing through a
hovering cube would briefly put the cube inside the ground — that
bug is fixed).

---

## 8. Open Questions

**Deformation field non-commutativity.** First pass assumes all
deformation fields commute (all additive). If a future analysis
contributes a multiplicative or conditional modulation, the
deformation-field set grows an explicit order annotation. Flag,
don't solve.

**GoL suppression for agents.** §3.3 called this: agents currently
excluded from `gol_suppression`, meaning agents feel the full GoL
lift. If this looks wrong in practice (agents stuck on top of a GoL
zone), the fix is adding `CONTRIB_GOL_SUPPRESSION` to
`POLICY_WALKER_AGENT` with consumer_pos = agent pos.

**Policy granularity for agents.** Do all agents share
`POLICY_WALKER_AGENT`, or does each behavior pick (flying behaviors
→ `POLICY_FLYER`, walking behaviors → `POLICY_WALKER_AGENT`)?
Per-behavior, per the agent doc's ground_level field. Resolves on
execution.

**Fused evaluation in the terrain VS.** The patch terrain vertex
shader currently inlines lattice × mods + solids + pyramids + waves
+ pulses + aura at every vertex, avoiding a function call per
contributor per vertex. Keeping this inline is correct for
performance. The policy framework names this as "the terrain VS is
a hand-fused `POLICY_WALKER`-style evaluation, authoritative for its
consumer (rendering)." The fused version must stay consistent with
the policy's contributor set; a reviewer catches drift, and a unit
test comparing a sampled policy value against the VS value at a few
XZs would catch it mechanically. First pass: keep inline, document.
Add the consistency test only if drift is observed.

**Per-policy mute flags.** Today couplings have individual mute
bits. With policies, should there be a "mute all deformation fields
for policy X" toggle, for debugging? Probably not needed — per-
contributor muting through the coupling registry handles it. Flag
if debugging gets awkward.

**Contributor registration order invariance.** The DAG validator
must be order-independent — it should reject a bad policy regardless
of which order contributors are declared. Tested at init.

**Aura propagation to static landforms.** A palm tree's render-time
policy includes `pawn_aura`, so the palm rises with the aura. But
the palm's vertex shader is currently a separate pipeline that
doesn't go through the ground query. This pass **does not** fix
palm-tree-rides-aura. That's a per-consumer migration: every static
landform's VS that wants to ride deformations needs to start
querying at render time. The plumbing is in place after this pass;
the migration is a follow-up, one static-landform family at a time.

**What goes in the coupling registry vs. the contributor registry.**
The coupling registry names source→parameter wires (polyphony →
terrain_waves amplitude). The contributor registry names *what
contributors exist*. These are separate abstractions. The coupling
registry's "target_param" references contributors by id; this pass
establishes that cross-reference cleanly.

---

## 9. Design Principle, Reaffirmed

This architecture is a specific instance of a general principle
that emerged while sketching it:

> **Cross-cutting concerns need explicit centralization from the
> start, because the first consumer is the de facto owner and the
> second consumer has no incentive to refactor the first's
> decisions into a general abstraction.**

Ground queries are a textbook cross-cutting concern: sampled by
placement, by rendering, by physics-ish, by camera, by shadow, by
entity clearance, by locomotion. The fact that the system drifted
into ad-hoc composition is structural, not a failure of past
judgment — it's the default trajectory when centralization isn't
enforced.

Other cross-cutting concerns in this program to audit with the same
lens, at their own paces:

- **Time domains.** `t_seconds`, `t_beats`, `dt`, `terrain_time` —
  probably healthy, probably worth a short audit.
- **Seeded randomness.** `cpu_hash`, property indices, tile_seed —
  mostly healthy, centralized years ago.
- **Couplings.** The `COUPLING_*` bitfield — centralized, healthy.
- **Lifecycle.** FAMILY_DISPATCH — centralized, healthy.
- **Entity ownership (player/mood/seed).** Named explicitly in the
  orb system, propagates through the floater and agent designs.

Whenever a new feature needs to sample, dispatch, or coordinate
across multiple systems, the first instinct should be: **find or
create the canonical registry for that concern, don't read the
nearest available value.** If the registry doesn't exist, creating
it is part of the feature's cost.

This document is an instance of that instinct applied to ground
queries.

---

## 10. Summary in One Sentence

The ground is a graph of named contributors with explicit dependency
edges among static landforms and orthogonal deformation fields for
everything temporal or local, queried through a small set of named
policies that filter which contributors each consumer sees — making
both the cube-through-pulse bug and the palm-rides-aura behavior
into direct consequences of consumer-chosen policies rather than
ad-hoc composition at each call site, and giving the coming
analyses, agents, and floaters a single place to plug into rather
than ninety.
