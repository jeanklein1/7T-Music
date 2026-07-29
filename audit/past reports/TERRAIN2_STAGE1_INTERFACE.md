> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# TERRAIN-2 (STAGE 1) — THE MANIFOLD INTERFACE, SPHERE-DIRECTED
## Phase A — the interface on paper (design; ONE report; STOP for the stamp)

The destination is the SPHERE (and its family — polar, torus, non-planar
sheets — expressive environments for the music visualizer). Stage 1 builds the
INTERFACE the sphere plugs into, filled by the heightfield as the sole
placeholder cast. **The query face and composition fold are shaped by what a
SPHERICAL cast will require — not by tidying the heightfield.** No code until
Jean stamps A1-A5. Grounded in the current signatures (file:line throughout);
the heightfield stays the only cast, so the world is observably unchanged (the
one authorized exception is A4's disclosed sampler-agreement delta).

---

## A1 — THE QUERY SIGNATURE (sphere-directed)

**The one query every consumer uses to ask "where is the surface, and how is it
oriented, at this coordinate, within this boundary?":**

```wgsl
struct SurfaceHit {
    position: vec3<f32>,   // the surface point in WORLD space (the cast fills all 3 axes)
    normal:   vec3<f32>,   // the TRUE surface normal (cast-computed; NOT reconstructed-planar)
    valid:    u32,         // 1 = resolved inside boundary; 0 = boundary-projected (was pushed in)
}

fn manifold_resolve(query_pos: vec3<f32>, policy: u32, qi: QueryInputs) -> SurfaceHit
```

**INPUT — the coordinate, without templates and without a new struct.** The
input is a **world-space `vec3` position** — the coordinate the consumer already
has (`QueryInputs.consumer_pos` is *already* a `vec3`, `world.wgsl:2621`). The
signature does **not** hardcode xz as "the" coordinate: the CAST projects the
world position into its own parameter space. The heightfield cast reads `.xz`
(ignores `.y`); the sphere cast takes the direction `normalize(query_pos -
center)`; a torus cast takes `(u,v)` from the position. **This is the answer to
"how to type it without templates exploding": there is no coordinate generic and
no coordinate struct — a world position is the universal coordinate, and
projecting it is the cast's private business.** (The manifold's *intrinsic
parameter* — the uv/lat-long the GENERATOR walks — is a different face, the
GENERATE face, which is one of the four welds Stage 1 does NOT touch; Stage 1
freezes only the RESOLVE face, and world-pos-in serves every cast for resolve.)

**OUTPUT — a full surface point + a true normal, RETURNED not ASSUMED.** Today
the caller passes `xz`, gets a scalar `y`, and *reconstructs* the normal itself
as `vec3(-dx, 1.0, -dz)` (`terrain_normal_at`, `world.wgsl:5434` — the literal
`1.0` is the heightfield assumption living in the caller). Under the interface,
the heightfield cast fills `position = vec3(query_pos.x, contrib(query_pos.xz),
query_pos.z)` and `normal = normalize(vec3(-gx, 1, -gz))` — **its current values,
now returned by the cast instead of assumed by the caller.** Pixel-identical
today; sphere-ready tomorrow (the sphere returns `position = center + dir·radius`
and a real tangent-plane `normal = dir` perturbed by overlays — the caller never
learns which cast answered).

**BOUNDARY — carried in the query, resolved by the cast (open = at infinity).**
The resolve reads a per-frame `Boundary { center: vec3, extent: f32 }` (the
existing `world_bound` promoted; `extent = 0` = infinite, matching today's
`(0,0,0,0)` convention, `world.wgsl:1394`). The heightfield cast returns
`valid=0` with `position` projected back onto the bound when `query_pos` is
outside (this IS the pawn/camera finite clamp, `world.wgsl:5663, 6432` — now a
return value, not a caller-side `clamp`). **The sphere makes this natural: a
closed manifold has no edge to clamp — every direction is inside, so `valid` is
always 1** (a small sphere is finite by being closed; a large sphere is
"effectively infinite with seeded novelty over distance" — Jean's exact model,
realized without a boundary branch).

**Proof a spherical cast implements this signature UNCHANGED** (walk-through):
`manifold_resolve(query_pos, POLICY_WALKER, qi)` on the sphere cast →
`let dir = normalize(query_pos - sphere_center);` `let r = sphere_base_radius +
overlay_fold(dir, POLICY_WALKER, qi);` → `position = sphere_center + dir * r;`
`normal = tangent_perturbed(dir, overlay_gradient);` `valid = 1u;`. The pawn's
step-climb reads `position`, the camera clamps to `|pos-center| ≥ r + clearance`
(the "push out along the normal to clearance" rule generalizes the heightfield's
`pos.y ≥ h + clearance`), placement orients the entity to `normal`. **Same
signature, same three consumers, different cast — no interface edit.**

**The scaffold this adopts (TERRAIN-1 M1):** `manifold_resolve` is the graduation
of the idle `*_gradient` family — `query_ground_flyer_gradient` already returns
`vec3(h, dh/dx, dh/dz)` (`world.wgsl:2859`), one step from position+normal, with
zero callers. The `POLICY_*` argument already exists (the policy masks). The
interface is *named*, not invented.

---

## A2 — THE CAST BOUNDARY (interface vs cast)

The exact line — what Stage 1 FREEZES vs what Stage 3 REPLACES:

| | **INTERFACE** (frozen Stage 1) | **CAST** (heightfield now, sphere later) |
|---|---|---|
| **query** | `manifold_resolve` + `SurfaceHit` + `Boundary` (A1); the `POLICY_*` ids; the boundary rule | the projection world-pos → parameter (`.xz`); the value/normal bodies |
| **fold** | the declared author order + the policy→contributor mapping (A3) — **`ground_architecture.hpp` already IS this** (the `POLICIES[]` masks are the fold declaration) | the base-shape authors: `terrain_height_at` (§1.6 generator), `contrib_static_base_at`, `tile_modifiers`, `structure_height_at` (piers), pyramids |
| **storage** | — (the interface names *that* a base shape is cached, not *how*) | the four welds: the `rgba16float` texel format, the mesh VS, the normal basis, the Y-up/XZ movement + spatial index (TERRAIN-1 M2) |
| **consumers** | every `query_ground_*` / `sample_terrain_y_at` / `estimate_terrain_height` call site, now calling `manifold_resolve` | — (consumers are interface-side; they never touch the cast directly) |

**The files, partitioned:**
- **INTERFACE** — `contracts/ground_architecture.hpp` (the policy/fold declaration; TERRAIN-0 already called it a pure declaration-and-validation registry — it becomes the manifold interface's home) + a small manifold-query contract for `SurfaceHit`/`Boundary`. The LATENT `*_gradient`/`POLICY_PLACEMENT` scaffold graduates here as the live path.
- **CAST (heightfield)** — `world.wgsl` §1.6 (`terrain_height_at` + the generator), the `contrib_*_at` base-shape bodies, `sample_terrain_y_at` (the cached path), `terrain_normal_at` (the normal computation), `generate_patch_heights/gradients` + `patch_terrain_vs` (the storage/mesh welds), `tile_world.hpp`'s shape half (A5), `state.hpp`'s heightfield formats.

**What Stage 3 inherits vs replaces:** it inherits the interface (signature +
fold + consumers, all proven against the heightfield) and replaces exactly the
CAST column — the four welds + the base-shape bodies + the projection. It pays
the welds ONCE, behind a frozen interface, under a test baseline. That staging
is the entire point.

---

## A3 — THE COMPOSITION FOLD (Stage 2's seed, carried by the interface)

TERRAIN-1 M4: the surface is composed from ~5 authors folded in different orders
at different sites → three disagreeing samplers. The unifying insight is a
**BASE-SHAPE vs OVERLAY partition**, because it is exactly the cast/interface
line:

- **BASE-SHAPE authors (the CAST's — replaced by the sphere):** the seed lattice
  × tile-amp + tile-bias + piers (`contrib_static_base_at`) + pyramids. These
  DEFINE the surface's shape. A sphere composes its OWN base shape
  (`radius(dir) = base + seed_noise + …`) — same role, different cast.
- **OVERLAY authors (UNIVERSAL — fold on top of any cast, along its normal):**
  GoL zones, terrain waves (the voice), radial pulses, pawn aura. These are
  deformations/interactions that apply to any manifold — a sphere gets aura'd
  and GoL'd identically (displace along the cast's normal by the overlay amount).

**The one declared fold every query runs:**

```
SurfaceHit manifold_resolve(pos, policy, qi):
    base = CAST.base_shape(pos)                    # position + normal from the cast (texture or analytic)
    d    = Σ overlay_k(pos, qi)  for k in policy   # GoL + wave + pulse + aura, the GLOBAL overlays
         + consumer_local(policy, qi)              # policy-parameterized: GoL-suppression center, self-aura exclusion
    return displace(base, d along base.normal)     # push the base point out along its own normal
```

**Why this makes consumers agree without erasing intentional divergence.** The
overlays split into GLOBAL (identical for everyone: GoL/wave/pulse/external-aura)
and CONSUMER-LOCAL (policy-parameterized, legitimately different: the walker
flattens the GoL zone under its own feet; the tilt policy excludes the pawn's
self-aura so it doesn't tilt on its own bump). The three-sampler *bug* is that
the BAKED sampler omits the global overlays entirely while the LIVE samplers
include them — that is the disagreement A4 fixes. The consumer-local terms stay
policy-parameterized (correct divergence, kept). **`ground_architecture.hpp`'s
`POLICIES[]` masks already encode this fold** — Stage 1 makes them the ONE place
the fold is declared, and `manifold_resolve` the ONE place it runs, so no site
hand-copies it (killing the hot-path drift TERRAIN-0 flagged: the shadow VS that
silently drops aura+pulses becomes a policy choice, not a hand-fusion).

**The base/overlay line is load-bearing for the sphere:** the sphere replaces
BASE-SHAPE; it keeps OVERLAY untouched (the voice, GoL, aura fold onto the sphere
exactly as onto the plane, along the normal). That is why the partition, not
just the order, is the deliverable.

---

## A4 — THE SAMPLER UNIFICATION (the free bug-fix, delta disclosed loudly)

The three samplers collapse to `manifold_resolve` with the one fold. Per-sampler
migration + the exact behavioral delta:

| current sampler | callers | → migration | behavioral delta |
|---|---|---|---|
| **CPU `estimate_terrain_height`** (coarse: `bias + amp·5`, `tile_world.hpp:465`) | ribbon tips (`ribbon.hpp:919,950`) | the **CPU cast** of `manifold_resolve` (base-shape, CPU form) | ribbon Y becomes accurate (was coarse — no piers/pyramids/lattice). Small, positive. *Or* keep coarse as a documented cheap CPU variant — **Jean rules** (see below). |
| **baked `sample_terrain_y_at`** (texture: base+pyramids, `world.wgsl:7917`) | placement, shadow, photographer | the **cached base-shape path** (texture `.x`→position, `.yz`→normal) + the overlay fold | **THE DISCLOSED DELTA** — these gain the global overlays (GoL, aura, pulses) |
| **analytic `query_ground_*`** (live: base+pyramids+overlays) | pawn, camera, agents | the **analytic base-shape path** + the overlay fold | none — already runs the fold |

**THE EXACT BEHAVIORAL DELTA (loud):** today, where a GoL zone lifts the ground
or the pawn's aura deforms it, the PAWN stands on the deformed surface but the
ENTITY beside it is placed on the *un-deformed* baked ground (it sinks into a
lifted zone / floats over a lowered one); the SHADOW is cast from ground without
aura/pulses (a shadow that doesn't match the lit surface); the PHOTOGRAPHER
clamps to un-overlaid ground. **Unifying means they AGREE** — entities ride the
GoL lift, shadows include the aura, the photographer clamps to the real surface.
This is a **correctness fix** (the divergence is a latent bug — paintings already
hand-compensate by re-adding GoL analytically, `world.wgsl:8085`). Because the
wave voice is dead today (0), the *visible* delta right now is confined to **GoL
zones and the pawn-aura region**; it widens when the voice revives.

**The gating recommendation (Jean rules).** Split b2 so the interface can land
pixel-identically and the fix rides its own rig-verified gate:
- **b2a — the fold STRUCTURE, pixel-identical.** Every policy keeps its *current*
  contributor set, now declared once and run through `manifold_resolve`. No
  consumer's contributor set changes → the world is byte-identical. This lands
  the interface.
- **b2b — the AGREEMENT flip, the disclosed delta, its own gate.** Baked
  consumers gain the global overlays; the gate becomes "baked and live agree,"
  pixel-identical EXCEPT the disclosed GoL/aura-on-baked change. Rig-verified.
- Recommendation: **land b2a in Stage 1; hold b2b as the next gated cut** — it
  honors the prime gate (Stage 1 observably unchanged) while still freezing the
  interface, and gives the visible correctness change its own reviewable, rig-
  proven commit. (The ribbon CPU-accuracy delta rides the same choice: unify in
  b2b, or keep the documented-coarse CPU variant.)

---

## A5 — THE tile_world CROSS-CUT SPLIT (TERRAIN-0 Law 2, resolved)

`TileState` (`tile_world.hpp:102-116`) welds two concerns; split by concern:

| field | concern | → home |
|---|---|---|
| `archetype`, `height_bias`, `amp_scale`, `activation_scale`, `amp_momentum` | **landform SHAPE** (the cast's base-shape modulation + the terrain-token momentum) | **the heightfield CAST** (shape authorship) |
| `entity_density`, `theme_spawn[PopFamily::COUNT]`, `theme_idx` | **POPULATION** (spawn density/theme — NOT terrain) | **the population concern** (`population_themes` — the clean owner TERRAIN-0 named) |

**`TileState` → `TileShape` + `TilePopulation`.** The F1-F4 boundary face splits
with it: **F1 HEIGHT / F2 WARMTH / F4 ARCHETYPE = shape (cast); F3 SPAWN-MULT =
population** (`tile_world.hpp:176-187`). The `TerrainToken` Markov momentum
machinery (`amp_momentum`) goes with `TileShape` — it carries landform amplitude
excess tile-to-tile, pure shape.

**Two structural gifts that make the split clean:**
1. **The GPU mirror is already shape-only.** `GPUTileEntry` uploads only
   amp/bias/activation (`tile_world.hpp:232-235`); the population fields are
   CPU-only (spawn is CPU). So the split does not touch the C++/WGSL mirror — the
   population half never crosses to the GPU.
2. **One generation moment fills both.** `generate_tile_state` rolls shape and
   population together (shared `(gx,gz)` key, shared seed moment). The split keeps
   ONE generator that returns both halves; only the STORAGE (two caches, or one
   struct-of-two) and the READERS (shape readers vs `tile_apply_spawn_mult`)
   separate by concern. Behavior-identical.

**Why this belongs in Stage 1:** the base-shape/overlay partition (A3) needs the
tile-shape to be unambiguously the cast's — a sphere replaces `TileShape` (its
own per-region shape modulation) while `TilePopulation` is cast-agnostic
(entities populate any manifold). The split draws that line in the data, not just
the fold.

---

## PHASE B — THE CUT (shape only; sized at the stamp)

Four single-intent sub-movements, each with its pixel/behavior gate + the
no-WGSL-compiler hand-verification pass (the welds touch shaders):

- **b1 — THE INTERFACE lands.** `manifold_resolve` + `SurfaceHit` + `Boundary`;
  the heightfield cast formally behind it (position/normal/valid RETURNED by the
  cast); the LATENT `*_gradient`/`POLICY_PLACEMENT` scaffold becomes the live
  path; consumers call `manifold_resolve`. **Pixel-identical** (heightfield
  unchanged; the returned normal equals today's reconstructed one).
- **b2a — THE FOLD STRUCTURE lands.** One declared author order (base-shape +
  overlays); all queries run it; every policy keeps its current contributor set.
  **Pixel-identical.**
- **b2b — THE AGREEMENT flip (GATED per A4; land now or defer).** Baked consumers
  gain the global overlays. **Pixel-identical EXCEPT the disclosed
  overlay-on-baked delta.** Rig-verified. *Recommend defer to its own cut.*
- **b3 — THE FINITE COLLAPSE.** The 6 `finite_mode` branches behind one "window =
  intersect(follow-window, boundary)" rule + the boundary carried in
  `manifold_resolve`; open = boundary-at-infinity. **Behavior-identical** (finite
  worlds behave as today; the mechanism unifies).
- **b4 — THE CROSS-CUT SPLIT.** `TileState` → `TileShape` (cast) + `TilePopulation`
  (themes) per A5. **Behavior-identical.**

---

## STOP — THE STAMP REQUEST

Phase A returns the one load-bearing design. Open for the stamp:

1. **A1 — the signature.** `manifold_resolve(query_pos: vec3, policy, qi) ->
   SurfaceHit{position, normal, valid}`, world-position input (cast projects, no
   coordinate generic/struct), boundary carried + resolved by the cast. Confirm
   the shape — this is what the sphere plugs into untouched.
2. **A2 — the cast boundary.** Interface = signature + fold declaration
   (`ground_architecture.hpp`) + consumers; cast = the four welds + base-shape
   bodies + projection. Confirm the partition.
3. **A3 — the fold + the base-shape/overlay partition.** Base-shape = cast's
   (lattice/tile/piers/pyramids); overlays = universal (GoL/wave/pulse/aura),
   folded along the normal; consumer-local terms stay policy-parameterized.
   Confirm.
4. **A4 — the sampler-delta ruling.** Land the fold structure pixel-identical
   (b2a) in Stage 1 and DEFER the agreement flip (b2b) to its own gated cut
   (recommended) — or land the flip in Stage 1 with the disclosed delta? And the
   ribbon CPU sampler: unify or keep documented-coarse?
5. **A5 — the cross-cut split.** `TileState` → `TileShape` (cast) +
   `TilePopulation` (themes); one generator fills both; GPU mirror unaffected.
   Confirm.

NO code until the stamp — the interface is the sphere's foundation and is gotten
right on paper first. Deferred, pulled-not-pushed: Stage 2 proper (if A3 surfaces
more than the fold), Stage 3 the sphere (the four-weld rewrite as a new cast
behind this interface), and the SDF excavation (TERRAIN-1 M5/M6, wire-first-
clean-second).
