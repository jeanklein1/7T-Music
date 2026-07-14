# TERRAIN-0 — THE "WHAT IS TERRAIN?" AUDIT (read-only; ONE report; the map)

Definitional recon, not an inventory. The question, answered from the code
the way the pawn audit answered "what is the player": WHAT IS TERRAIN,
essentially — and where are its SEAMS (the distinct roles fused under one
word, the terrain's version of "the pawn is anchor + body + gait")? Nothing
moves. This is the map the terrain campaign's design conversation reads to
decide the wiring.

METHOD. Nine parallel deep-readers (one per seam A1-A5, plus a module-reality
pass and a dependency/dependent sweep), each distinctness verdict
adversarially challenged, then a completeness critic hunting a missed seam.
Every claim below carries a file:line. Three lenses held throughout (v3):
terrain is L0/S1 the SURFACE (the mandatory reference frame, §3); S1+S2 the
frame AND its lifecycle (§9); and D2 the surface's VOICE (§2.2, currently
driverless).

---

## PART A — THE DEFINITIONAL ANSWER (the primary product)

**Terrain is not one thing. It is SIX roles wearing one word — and, unlike
the pawn's three cleanly-nested roles, terrain's are welded by an
ASYMMETRY that means there is no single "terrain boundary" to hold.**

The one-sentence answer:

> **Terrain is a near-leaf AUTHORING MEMORY on its input face (a seed-pure,
> stateless height field plus a per-tile character memory) that becomes a
> deeply-FUSED COMPOSITION SURFACE on its output face — because "the ground
> at (x,z)" is not stored anywhere; it is re-composed inside every query
> from five different authors (the seed lattice, the tile character, the
> spawn-placed solids, the GoL field, and the pawn's aura). The word
> "terrain" spans the leaf and the composition; that span is the whole
> finding.**

### The six seams

| # | seam | what it essentially IS | verdict (survived challenge) |
|---|---|---|---|
| **A1** | THE FRAME | a policy-parameterized **composition-and-dispatch architecture** answering "where is the ground?" per consumer by choosing *which contributors it integrates* (POLICIES[] masks → `query_ground_<policy>`) | **PARTIALLY_FUSED** |
| **A2** | THE GENERATOR | the stateless seed→height **6-band lattice-wave field** (`terrain_height_at`) that *decides* the raw landform | **DISTINCT** (but frozen; homeless) |
| **A3** | THE LIFECYCLE | the per-frame **residency machine** — which patches exist around the point, layer-pooled, phase-piped | **PARTIALLY_FUSED** |
| **A4** | THE SPAWNING SURFACE | it **fractures on the verb**: "can I stand here?" is terrain-*agnostic*; only "at what Y?" is terrain-owned | **PARTIALLY_FUSED** |
| **A5** | THE VOICE | the couplable **expression** — geometry-wave + color-palette — fully authored but **driverless** | **PARTIALLY_FUSED** |
| **A6** | THE LIVING-TILE (GoL) GEOMETRY | *(found by the completeness critic)* a **parallel terrain-geometry organ** A1 only "composes" and nobody owns as terrain | **UNOWNED** — new seam |

### What each seam really is (the prose)

**A1 — THE FRAME is composition, not height.** Its owned decision is *"which
named contributors does each consumer integrate"* — realized as the
`POLICIES[]` bitmasks (`ground_architecture.hpp:119-245`), mirrored to
`POLICY_*_MASK` (`world.wgsl:2140-2187`), dispatched by the `query_ground_*`
family (`world.wgsl:2638-2848`), and validated by a compile-time DAG-closure
assert (`ground_architecture.hpp:260-285`). A camera is a FLYER, the pawn a
WALKER_PAIR, an NPC a WALKER_AGENT, the bake a BAKED_HEIGHTFIELD, the render
mesh a TERRAIN_RENDER — *the policy IS the consumer's identity.* This is a
genuinely distinct role with a pure declarative home. **Why only
PARTIALLY_FUSED:** the composition truth is not single-homed — three logical
contributor ids collapse into one function (`contrib_static_base_at`,
`world.wgsl:2333`, the enum itself confesses "fused", `ground_architecture.hpp:52-54`),
and the two hottest realizations **hand-copy** the mask instead of dispatching
it: `ground_formed_with_complexity` (the bake, `world.wgsl:2370`, carries the
warning "if the baked policy's contributor set ever changes, update this
function") and `patch_terrain_vs` (the render mesh, `world.wgsl:3620-3629`,
has *no* `query_ground_*` by design). So the "who sees what" decision is a
manifest that field-realization code duplicates by hand. *(The challenge
correctly demoted the pawn-gait "fusion" — step/slide/revert is consumer-owned,
so A1 is consumed by gait, not fused with it.)*

**A2 — THE GENERATOR is distinct, frozen, and homeless.** `terrain_height_at`
(`world.wgsl:576`) is a pure function of (xz, seed): a 6-band hierarchical
wave-lattice (radial rings + directional ridges with exp-damping envelopes,
*not* fBm), each lattice node deriving a full wave from its hashed seed. It
enters composition as *one clean multiplicand* (`raw_h × mods.x + mods.y +
structure`), and with the identity-default modulator (`amp=1, bias=0`) raw_h
*is* the terrain — a textbook separable overlay, so **DISTINCT** survives.
**Two findings ride it.** (1) **The living generator is STATIC/frozen:** its
entire temporal machinery (activity field, beat frequencies, `WAVE_THRESHOLD`
gating, the frozen↔moving phase mix) is fully computed but **output-inert**
because both call sites hardcode `t_beats=0.0` (`world.wgsl:2334, 2371`) — the
mix collapses to identity. (2) **It has no single home:** the base field is
WGSL §1.6; the per-tile amp/bias modulation is C++ `tile_world.hpp`; the solids
are authored by the pier writers in `patch_system.hpp`; and *how they compose*
is declared by `ground_architecture.hpp`. "The generator" is a distributed
committee, not a module.

**A3 — THE LIFECYCLE is residency with a visibility sub-seam braided in.**
The distinct core owns *"which grid cells hold a live patch around the point"*
— an integer-windowed allocation ring + eviction + a 225-slot layer pool +
the ALLOCATED→SPAWNED→GENERATED phase pipe, all in `stream_patches`
(`patch_system.hpp:550-893`), keyed on the recenter cursor. **Braided into the
same function** is a separable VISIBILITY/LOD-banding sub-seam (stage 6,
`:779-874` + `frustum_cull_patches`, `world.wgsl:8236`) that uses a *different
metric* (continuous world-cylinder dist², not integer grid windows) and a
*different cursor* (the raw point, shipped as `lod_pawn`). The tell the
challenge could not dissolve: the LOD0 threshold is **duplicated across the
C++/WGSL boundary** (`LOD0_CYLINDER_RADIUS_SQ` ↔ `FRUSTUM_LOD0_RADIUS_SQ`) with
`lod_pawn` uploaded *specifically* so the two agree — you do not ship a
dedicated uniform to keep two computations equal unless it is a real decision
boundary. Residency depends on nothing of visibility; visibility depends on
residency — the one-way signature of a separable sub-role.

**A4 — THE SPAWNING SURFACE fractures on the verb, and terrain owns only
half.** "Can I stand here?" (XZ admission) is a **terrain-agnostic** 2D
entity-vs-entity packer — `check_position`/`register_footprint`/`MIN_SEPARATION`
(`spawn_engine.hpp:474-513`, `spawn_services.hpp:71`) never reads a height, and
**terrain never vetoes a spawn**. "At what Y?" is the only terrain-owned part:
`sample_terrain_y_at` + `compute_entity_placement` (`world.wgsl:7917, 8058`),
which *reports* (never denies) a height and is the *one* place footprint SHAPE
meets terrain (arch = 2-pt min at pier feet, pyramid = 5-pt min at corners,
else single-center). A third organ — the patch-to-placement **tether**
(`entity_refs` + footprint patch-tags, `surface_services.hpp:93-125`) — keeps
admission and eviction symmetric (its fragility is why `audit_entity_integrity`
exists). So the seam named "how terrain allows placement" is mostly *not
terrain*: terrain reports a Y after the occupier machine has already admitted
the entity on a flat plane.

**A5 — THE VOICE is a fully-built instrument with no player.** Its owned
decision — *"given a driver, how does the surface express it"* — is separable
from the driver (the transform math and neutral dials are wired; only the
writer is missing). But it is PARTIALLY_FUSED two ways: (1) **internally two
sub-seams** — a geometry channel (the overlay wave `contrib_terrain_waves_at`
+ the lattice's frozen↔moving blend) and a color channel (palette/mode drift
`animated_cell_color` + `mode_*` dials) that share *only* the "driverless"
property; (2) **it owns no module** — every transform writes into a substrate's
own channel (geometry summed into `world_pos.y` at ~24 sites; color re-composites
the same palette pipeline that bakes the baseline). **The defining fact:** every
voice dial has exactly one live writer — the boot block (`cartridge.hpp:411-425`)
— and it holds them all at neutral. The expressive apparatus is present; the
driver (retired gen-1 polyphony/mood) is absent.

**A6 — THE LIVING-TILE (GoL) GEOMETRY is a whole terrain organ nobody owns.**
A1 files GoL as "a slow_dynamic contributor it composes but does not author,"
and A5 covers only waves+palette — so **no seam owns GoL-as-terrain**, yet it
is a full parallel terrain-geometry subsystem: excluded from the baked
heightfield (`world.wgsl:3565-3566`), emitting its *own* extruded mesh that
samples baked terrain as its base (`zone_sample_baked_terrain_y`,
`world.wgsl:7455-7466`), feeding height *back* into every walker query
(`contrib_gol_zones_at`, `:2061`), and carrying a suppression law **hand-synced
across three sites** (the contributor + `zone_extrusion_vs` + `shadow_zone_extrusion_vs`,
explicit warning at `world.wgsl:7585-7594`). That triple hand-mirror is exactly
the fused-role hazard this audit hunts. It merits a seam letter of its own.

### The two structural laws (what makes terrain terrain)

**LAW 1 — THE ASYMMETRY (the master finding).** Terrain is a **near-leaf on
its INPUT face, a deep composition on its OUTPUT face.** The authoring memory
(seed + tile-character tokens + themes) depends only on the seed and one mood
bit; but "the ground at (x,z)" is re-composed inside every query from *five
authors*: `contrib_static_base_at` = `terrain_height_at(seed)·tile_amp +
tile_bias + structure_height_at(piers)`, then the bake adds pyramids, then
walker/flyer queries fold in GoL zones and pawn aura. **The composition
happens inside the query — there is no stored "terrain surface" to hold a
boundary against.** Rewiring the height *function* ripples to every consumer;
but rewiring any *input* (a pier move, a GoL step, an aura change) silently
re-authors the ground too.

**LAW 2 — THE `tile_world` CROSS-CUT (terrain's anchor+body+gait).** One
module fuses **half of A2 and half of A4**: `TileState` (`tile_world.hpp:102-116`)
welds *landform character* (archetype → `amp_scale`/`height_bias`/`activation_scale`)
with *entity-population fields* (`entity_density`, per-family `theme_spawn[]`).
Two distinct roles — *how tall the ground is* vs *how many things stand on it*
— welded only because both are rolled at tile-generation time and cached in one
struct. The F1-F4 interface exposes the fusion: `estimate_terrain_height` and
`tile_archetype` (A2) sit beside `tile_apply_spawn_mult` (A4) as co-equal
"surface services." This is the terrain's pawn-moment: two modules wearing one
name, sharing a cache.

**Corollary — THE BAKED/LIVE DIVERGENCE.** Two populations read two different
grounds. *Baked* consumers (entity placement, shadow VS, the photographer
camera `world.wgsl:8004`) see `static_base + pyramids` only; *live* consumers
(primary camera, pawn, agents, spheres) see aura + waves + pulses + GoL too.
When those dynamic fields are nonzero, entities and shadows sit on a *different*
surface than the pawn — today mostly masked because the wave layer is dead, but
**GoL and aura are live**, and paintings already re-add GoL analytically
(`world.wgsl:8085`) to compensate. There are in fact **three disagreeing height
samplers**: the CPU `estimate_terrain_height` (coarse, ribbon-only), the baked
`sample_terrain_y_at`, and the analytic `contrib_static_base_at`.

---

## PART B — WHAT TERRAIN DEPENDS ON (requirements, per seam)

- **The seed** (DOWN, the single scalar the whole landform derives from):
  `WorldState.active_seed = DEMO.seed` (`surface_services.hpp:47`) →
  `make_patch_params.master_seed` (`patch_system.hpp:398`) → GPU
  `config.world_seed`; also CPU `tile_seed`/`cpu_lattice_node_seed`
  (`tile_world.hpp:306,341`).
- **Primitives** (DOWN, pure): `seed_utils.hpp` (`cpu_hash_f`, `tile_seed`,
  `cpu_lattice_node_seed`) + GPU `lattice_node_seed`/`hash_property`/
  `sample_gaussian` (`world.wgsl:404,282-302`).
- **Mood — a THIN MODIFIER, not an author** (SIDEWAYS, narrow): the landform
  is seed-pure. Mood touches terrain at *exactly two amplitude-only points*:
  (1) the indoor flag tilts the POOL archetype prior in `generate_tile_state`
  (`tile_world.hpp:271`, indoor→1.5 / outdoor→0.05 weight on the flat POOL
  archetype); (2) `apply_mood` sets `terrain_amp_ceiling = indoor?0.5:0`
  (`mood.hpp:582-583`) clamping wave amplitude (`world.wgsl:470-472`). Mood is
  a strong author of *atmosphere* (light, ceilings, walls, fog, gating) and a
  faint thumb on *landform*.
- **Piers / pyramids / GoL / pawn-aura** (SIDEWAYS — terrain's biggest reach):
  the ground base *reads occupier/spawn outputs*. `structure_height_at`
  (`world.wgsl:1969`) reads pier instances written by the spawn pipeline; the
  bake folds in pyramids (`world.wgsl:2373`); walker/flyer queries fold in GoL
  (`:2061`) and aura (`:2580`). **Terrain height is not a clean leaf — it is a
  composition point where terrain, spawn, GoL and pawn meet.**
- **The point** (SIDEWAYS): `player_.readback_x/z` is A3's sole external input
  (streaming center, LOD banding, budget), wired to the p1b camera-host
  readback (`patch_system.hpp:563`, `cartridge.hpp:914`).
- **The would-be voice driver** (DOWN, unwired): `terrain_time`, `band_blend_*`,
  `band_phase_origin_*` — set *only* to 0.0/−1.0 at boot; the retired gen-1
  polyphony/music coupling that would animate the waves.
- **GPU services** (DOWN): tile grid, patch instances, the 2-pass heightfield
  compute, the `patch_grid` index, textures (`state.hpp`, `patch_system.hpp`).

---

## PART C — WHAT DEPENDS ON TERRAIN (dependents + blast radii, per seam)

**FRAME (A1/A2) consumers** — every one reads the composed ground:
- Pawn ground-resolve + tilt (`world.wgsl:5459, 5427`, WALKER_PAIR / WALKER_TILT):
  **blast** — revising the walker contributor set or the height fn moves where
  the pawn stands and re-tunes step-climb/slide/block; the whole locomotion feel.
- Agent ground-snap (`world.wgsl:5565`, WALKER_AGENT, full GoL lift): **blast** —
  every mood-authored NPC's footing; a GoL-height change lifts/drops all NPCs.
- Primary camera clamp (`world.wgsl:6428`, FLYER, live contributors): **blast** —
  when the camera is pushed off ridges; animated deformation clips gate here.
- Photographer clamp (`world.wgsl:8004`, **baked** texture): **blast** — clamps
  to a *different* ground than the live camera (the baked/live divergence seam).
- Shadow terrain VS (`world.wgsl:3750`, baked + waves only, **no aura/pulses**):
  **blast** — terrain self-shadowing shifts; already an asymmetric hand-fused copy.

**SPAWNING (A4) consumers:**
- `compute_entity_placement` — the live entity-Y pipeline (`world.wgsl:8058`):
  **the single largest dependent** — any change to the baked contributor set
  (`static_base + pyramids`, which *must* mirror `ground_formed_with_complexity`)
  shifts every placed entity's footing.
- `estimate_terrain_height` — ribbon-only CPU proxy (`tile_world.hpp:465`):
  **blast** — a divergent ground truth (no piers/pyramids/waves); ribbon Y
  silently drifts from the baked ground every other family sits on.
- `patch_grid` + `placement_patch_count` (`patch_system.hpp:842-872`): **blast** —
  a stale/misplaced index returns 0.0 → camera falls through terrain, entities
  snap to y=0.

**VOICE (A5) consumers:**
- `patch_terrain_vs` / `_fs` — the visible ground mesh + color (`world.wgsl:3580,
  3644`): **blast** — the player sees terrain shape/normals/palette directly;
  this is `POLICY_TERRAIN_RENDER` hand-fused.
- ~24 entity/painting VS sites adding `contrib_terrain_waves_at` into
  `world_pos.y` (`world.wgsl:4149-10705`): **blast today = nil** (output is
  identically 0 while driverless); the risk is a shared additive *contract*,
  not behavior.
- The palette vocabulary `PALETTE_*` (`world.wgsl:1470-1493`): **blast** — the
  highest, *always-live* color dependency; revising it re-colors the entire world.

---

## PART D — THE MODULE REALITY (the four-home question)

| module | file | seams owned | ownership | note |
|---|---|---|---|---|
| **ground_architecture** | `contracts/ground_architecture.hpp` | A1 composition *contract* | clean but **minimal** | self-labels P9 "pure declarations + validation, zero runtime logic" — a mirror-registry, not a behavior owner; its value is the compile-time DAG-closure assert |
| **surface_services** | `contracts/surface_services.hpp` | A3 decls + A1 constants | **cross-cut** | the DECL tier of A3 (bodies in `patch_system.hpp`) *and* the A1 frame-constant vocabulary *and* the WorldState root — one A3 module split across two files for compile-ordering |
| **patch_system** | `surface/patch_system.hpp` | A3 bodies (+ A1 banding, A2 piers, A4 spawn-trigger) | **cross-cut** | owns A3 cleanly but reaches: LOD banding (A1), pier/solids authoring (A2), the GPU height dispatch (A2), and the `SEAM[patch:spawn-trigger]` (A4). Holds spawn-*timing*; others hold spawn-*content* |
| **tile_world** | `surface/tile_world.hpp` | **half A2 + half A4** | **THE KEY CROSS-CUT** | landform character *welded to* spawn density/theme via one `TileState` + one generation moment (Law 2) |
| **population_themes** | `surface/population_themes.hpp` | A4 composition | **clean** | the cleanest single-seam owner; touches neither shape nor color; wide *reader* fan-out (~10 entity adapters) is healthy |
| **state.hpp DTOs** | `realization/state.hpp` | A1/A2/A3 GPU mirror | wire | owns no decision; the marshalling boundary (byte-mirror under `static_assert`) |
| **world.wgsl** | `realization/world.wgsl` | **A2 + A5 + A6 + GPU realization of A1/A3** | **multi-seam monolith** | holds the *actual* generator (§1.6), the *actual* voice (§2.2 palette), GoL geometry, and the render/compute realization — the GPU mirror of the C++ committee |

**MINIMALITY flags** (v3 §8 — smaller-than-a-module; flag only, no merge):
- `ground_architecture.hpp` — self-labeled pure-declaration mirror-registry.
- `surface_services.hpp` — separate file from `patch_system.hpp` only for the
  decl-before-body cohort law; one A3 module in two files.
- `get_band_blend`/`get_band_phase_origin` (`world.wgsl:504-526`) — thin
  switch accessors over 6 flat config scalars (WGSL can't index them).
- `composite_cell_color` vs `_biased`, `animated_cell_color` vs `_lut` — two
  functions per one decision (a bias=0 special case; a perf fork).
- `on_patch_first_generated`, `record_placement_bookkeeping` — live-but-empty
  extension hooks.

**DEAD / DRIVERLESS register** (report-only; "dead" is decidable only *after*
wiring — do not cut):
- **The entire wave-overlay chain is driverless-dead:** `band_blend=-1`,
  `terrain_time=0` set once at boot, never updated → `contrib_terrain_waves_at`
  returns 0 at all ~24 sites; and both `terrain_height_at` callers pass
  `t_beats=0` → the substrate lattice wave is frozen too. `OVERLAY_WAVES`,
  `get_band_blend/phase`, the 12 `band_*` config fields — live plumbing carrying
  a permanently-zero signal.
- **The `WAVES → dynamics_terrain_gradient_max → lipschitz_factor` chain is a
  dead limb:** `lipschitz_factor` (`GPUTerrainState`) is written
  (`world.wgsl:5415`) and read by *nobody* — a fog-field-class dead output.
- **`MODE_COUPLING_MAGNITUDE = 0.0`** (`world.wgsl:1532`) annihilates the
  archetype→mode color coupling (the strength/direction lattice is fully
  evaluated, then ×0).
- **The entity-density lattice is a structural no-op:** `DENSITY_MIN ==
  DENSITY_MAX == 1.0` and every theme `density_mult == 1.0`
  (`tile_world.hpp:65`) → `tile_apply_spawn_mult`'s density term is always 1.
- **`terrain_state.tint` / `render_terrain` (binding 220) has a dead consumer:**
  `coupling_sphere_to_terrain_tint` computes and stores a tint no shader reads
  (`render_terrain` is declared and never referenced).
- **The placement-policy API is LATENT:** `POLICY_PLACEMENT_*` rows +
  `query_ground_placement_*` + all `query_ground_*_gradient` variants have zero
  callers; the live Y path is the baked hybrid.
- **Smaller latents:** the `complexity` texel channel (baked, unread);
  `activation_scale` (authored/uploaded/interpolated, consumed by no height
  caller); `ActivePatch::animated`, `RENDER_RADIUS/SIDE/GRID_SIDE`,
  `VISIBLE_RADIUS_SQ/LOD_FULL_RADIUS_SQ` (no readers); binding 144
  `photo_patch_instances` (superseded by `patch_grid`).
- **Hand-mirror hazards** (not dead, but consistency-fragile): the GoL
  suppression triple (`world.wgsl:7585-7594`); the height-policy hot-path family
  (`ground_formed_with_complexity` / `patch_terrain_vs` / `shadow_patch_terrain_vs`
  — the shadow copy already *diverges*, dropping aura+pulses).

---

## PART E — THE PANEL SURFACE (what the control panel would gather, by seam)

The decisive panel finding: **the core aesthetic dials live in WGSL
compile-time consts with NO C++ mirror — they are not panel-authorable
without the graduate-or-index decision (the p3 fork).** The panel splits
cleanly into a C++ side ready today and a WGSL side that needs that fork.

**GENERATOR (A2) — mostly WGSL, the p3 fork lives here:**
- `TERRAIN_BANDS[6]` (spacing / freq μσ / amp μσ / damping μσ / activation /
  temporal) — **WGSL const, not panel-ready.** *The* landform character dial.
- `world_seed` — **C++, panel-ready** (which of the infinite worlds).
- `terrain_amp_ceiling` — **C++, panel-ready** (the one live runtime modulation).

**VOICE (A5) — dials mirrored & panel-ready, but DRIVERLESS:**
- `band_blend[6]`, `band_phase_origin[6]`, `terrain_time` (geometry voice);
  `mode_color_shift/checker_scatter/palette_target/palette_intensity/discrete_tier`
  (color voice) — **mirror, panel-ready** (setters exist), but each writes a
  currently-neutral value; a panel would *become* the missing driver.
- `PALETTE_CENTER/LIGHT/VARIANCE/WEIGHT`, `OVERLAY_WAVES[6]` — **WGSL const,
  not panel-ready** (the p3 fork again; the palette has no C++ home).

**LIFECYCLE (A3) — the panel-ready majority:**
- `active_radius`, ALLOC/EVICT/SPAWN budgets, the dynamic gen-budget curve,
  `finite_mode/radius`, the LOD cylinder radii (`VISIBLE_RADIUS`/`LOD_FULL_RADIUS`)
  — **C++, panel-ready.**
- `PATCH_EXTENT`, `MAX_ACTIVE_PATCHES`, `FRUSTUM_LOD0_RADIUS_SQ`, `lod_pawn` —
  **mirror, not a free dial** (structural / must-stay-consistent).

**SPAWNING (A4) — C++ but panel-ready=false (occupier-owned, not terrain):**
- `MIN_SEPARATION[12][12]`, `PROXIMITY_*`, `GLOBAL_ENTITY_DENSITY`,
  `MAX_FOOTPRINTS=128`, `MAX_ENTITY_REFS=10`, per-family jitter — the spacing/
  packing law. Belongs on a *population* panel, not the terrain panel; the only
  terrain-owned A4 dial is the per-family footprint-reduction policy (WGSL).

**So the terrain panel emerges in two tiers:** a **ready tier** (seed,
amp-ceiling, the whole lifecycle set, and the voice *dial values*) that is
C++/mirror today, and a **fork tier** (the band spectrum, the palette, the
overlay-wave spectrum) that holds the actual generation + color character in
WGSL consts and needs the graduate-or-index decision before it can be authored.
The two-dial camera panel already built (p1b: fly-speed, sensitivity) is the
form the ready tier inherits.

---

## STOP — THE MAP IS DRAWN

Returned: the definitional answer (Part A — terrain is six roles under one
word, welded by the input/output asymmetry and the `tile_world` cross-cut,
with A2 frozen and A5 driverless), the per-seam dependency (B — mood is a thin
modifier; terrain's height query is a five-author composition point) and
dependent maps with blast radii (C — the baked/live divergence, the three
disagreeing samplers), the four-home module reality with minimality + a full
dead/driverless register (D), and the panel's would-be contents split into a
ready tier and a WGSL-fork tier (E).

NO cuts, NO merges, NO deletions. The audit answers "what is terrain"; the
campaign that follows decides what to do about it. The design conversation's
first forks, surfaced by the map:
1. **The `tile_world` split** (Law 2) — landform-character vs spawn-fields, the
   terrain's anchor+body+gait.
2. **The baked/live divergence** — do placement/shadow/photographer and the
   pawn/camera agree on one ground, or is the split intentional?
3. **The graduate-or-index fork** (Part E) — the band spectrum + palette must
   leave WGSL consts for the panel to author them.
4. **The frozen generator + driverless voice** — is the terrain campaign the
   moment the temporal machinery and the voice get their driver, or do they
   stay latent?
5. **A6 (GoL geometry)** — does the living-tile organ get a seam owner, and does
   the shadow-caster's dropped-contributor divergence get reconciled?
