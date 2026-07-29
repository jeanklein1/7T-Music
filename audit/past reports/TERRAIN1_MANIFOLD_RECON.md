# TERRAIN-1 — THE MANIFOLD RECON (read-only; ONE map; two targets)

The reframe this serves (Jean, ratified — the terrain's point-model): **terrain
is a PLACEMENT MANIFOLD** — a surface defined by its boundaries / coordinate
system, answering *"where is the surface at this coordinate, and what is its
normal?"* (position + normal, for snapping). **The heightfield `y = f(x,z)` is
our current DEFAULT CAST** of that manifold — incidental, not essential.
**Finite vs infinite is a BOUNDARY CONDITION** on the manifold, not a separate
code path. The **composition order** must hold uniformly across near and far
boundaries. This recon does NOT design the spine — it measures how far the code
is from it, so the design conversation reads a map, not a guess. Nothing moves.

METHOD. Four parallel deep-reads (query-face + finite/infinite; heightfield-
assumption blast; raymarch archaeology) plus a synthesis over the ratified
policy masks. Every claim file:line'd.

---

# TARGET 1 — HOW MANIFOLD-SHAPED IS THE CODE ALREADY?

## M1 — THE QUERY FACE (does a manifold interface implicitly exist?)

**Verdict: the manifold interface implicitly exists, but every cast of it is
heightfield-baked — `xz → scalar y`, Y-up, one height per column. There is no
`coord → surface point` signature anywhere. Yet the *scaffold* for one already
exists, unused.**

Every ground query shares one convention: `xz: vec2` (plus an auxiliary
`QueryInputs{consumer_pos: vec3, t_seconds}`) → `f32`. The `consumer_pos` 3D
vector exists *only* so walker policies can measure `distance(xz,
consumer_pos.xz)` for pawn-centered GoL suppression (`world.wgsl:2742, 2809`);
its `.y` is never read. So even the "3D-input" queries are 2D-in / scalar-out.

| query | coord in | returns | shape |
|---|---|---|---|
| `contrib_static_base_at` (`world.wgsl:2333`) | 2D xz | scalar y | heightfield-baked (the root base) |
| `query_ground_{baked,flyer,walker,walker_agent}` (`:2681-2846`) | 2D xz | scalar y | heightfield-baked |
| `query_ground_walker_pair` (`:2797`) | 2D xz | `vec2` (walker-y, tilt-y) | *two heights, same column* — still one-per-column |
| `sample_terrain_y_at` (`:7917`) | 2D xz | scalar y (texture .x) | heightfield-baked |
| `estimate_terrain_height` (`tile_world.hpp:465`) | 2D xz | scalar y (`bias + amp·5`) | heightfield-baked (coarse CPU proxy) |
| `pawn_ground_resolve` (`:5459`) **LIVE** | xz candidate + prev xz/y | **`vec4(x,y,z,ok)`** — position + blocked flag | heightfield walk (xz slides + column y) |
| `terrain_normal_at` (`:5427`) **LIVE** | 2D xz | **`vec3` normal** `normalize(vec3(-dx, 1.0, -dz))` | heightfield gradient-normal (the literal `1.0` in Y *is* the assumption) |
| `query_ground_{flyer,walker}_gradient` / `_walkable` (`:2859-2896`) | 2D xz + eps | **`vec3(h, dh/dx, dh/dz)`** — height + slope | **manifold-adjacent — but LATENT (zero callers)** |
| `query_ground_placement_{pyramid,painting,vegetation}` (`:2638-2669`) | 2D xz | scalar y | LATENT (zero callers) |

**The finding under the finding: the manifold's future interface is already
scaffolded and idle.** The `*_gradient` family already returns `(height,
slope_x, slope_z)` — one algebraic step from `coord → position + normal` — and
the `POLICY_PLACEMENT_*` rows already declare a clean placement-query API. Both
are **LATENT with zero live callers** (`ground_architecture.hpp:121-143`); the
live paths (`sample_terrain_y_at`, `terrain_normal_at`, the hand-fused
`compute_entity_placement`) are the heightfield casts *bypassing* the scaffold.
So the manifold spine does not need to be invented from nothing — it needs the
already-declared query API to gain a position+normal return and its live
consumers to move onto it. `QueryInputs.consumer_pos` being a full `vec3` is the
same tell: the *signature* already carries a 3D point; the scalar-Y assumption
lives in the bodies, not the interface.

## M2 — THE HEIGHTFIELD ASSUMPTION (how baked is `y = f(x,z)`?)

**Verdict: the value provider is a thin LEAF; "Y-up / XZ-ground" is a pervasive
SPREAD. A non-heightfield cast is a rewrite of the storage, the mesh, the normal
basis, the movement frame, and the spatial index — only the per-column value
functions survive.**

**LEAF — genuinely swappable behind the query face:** the *value body* of
`sample_terrain_y_at`, `contrib_static_base_at`, `terrain_height_at`,
`estimate_terrain_height`, the scalar `query_ground_*` bodies. `tile_world.hpp:171-188`
even advertises this seam explicitly ("a generated-once surface cast could
implement these four and the occupiers would never know"). **Caveat:** all are
*typed* `→ scalar`. They are leaves only while the answer stays one Y; a manifold
answer (`position + normal`) breaks the return type and cascades.

**SPREAD — the four structural welds a cast must break:**
1. **Texel storage contract** — `rgba16float` = `vec4(height, grad_x, grad_z,
   complexity)`: store `world.wgsl:6974`; decode `:3604-3608, 3767, 5376,
   7934`; scratch buffer sized `256×256×2 floats` (height+complexity only,
   `state.hpp:2736, 4824`). No room for a stored position or full normal.
2. **Mesh VS** — `patch_terrain_vs` / `shadow_patch_terrain_vs` (`:3580, 3750`)
   decode a regular planar `PATCH_MESH_N=64` XZ grid and displace **only Y**
   (`world_pos = (origin.x+u, height, origin.z+v)`). The mesh cannot fold, wrap,
   or overhang — the covering law is structural.
3. **Normal basis** — never stored; reconstructed as `normalize(vec3(-gx, 1.0,
   -gz))` at the FS (`:3646`) and `terrain_normal_at` (`:5434`); the middle
   component is a literal `1.0`. The `generate_patch_gradients` kernel (`:6887`)
   finite-differences the scalar height into planar partials — meaningless for
   a manifold.
4. **Y-up movement + XZ spatial index** — ~30 `world_pos.y +=` / `pos.y =`
   sites; `agent_post_step` integrates XZ then *snaps* Y (`:5560-5565`);
   `PAWN_STEP_HEIGHT` compares world-Y differences (`:5472-5485`); heading is
   yaw about `+Y`; and the entire streaming/LOD/eviction/tile system keys on
   `(gx,gz)` with `GPUPatchInstance.origin[2]` XZ-only (`state.hpp:1219`),
   `patch_distance_sq` XZ (`patch_system.hpp:431`), finite bounds `vec2` XZ.

**The distance, stated plainly:** swap the value function and pure "give me y at
xz" callers don't notice (leaf). But "up = world Y, ground = world XZ" is welded
into the texel format, both vertex shaders, every normal, the 2-DOF+snap
movement integrator, the step/camera clamps, and the XZ streaming grid. A
spherical/arbitrary cast is *far* from a swap.

## M3 — THE BOUNDARY / FINITE-INFINITE STRUCTURE

**Verdict: on the SURFACE side, finite and infinite are ALREADY one mechanism.
The divergence is not in generation — it is ~6 scattered imperative branches in
the streaming controller and the position clamps. "1.5 code paths," and the
gap to "finiteness is a boundary condition on one manifold" is small and
structural, not algorithmic.**

- **Generation is identical.** No terrain producer (`terrain_height_at`,
  `tile_modifiers_at`, `structure_height_at`, pyramids, GoL, waves, pulses,
  aura) reads `finite_mode`, `finite_radius`, or `world_bound_*`. The same seed
  yields the same infinite field; finiteness only decides *how much is streamed*
  and *where the pawn/camera may stand*.
- **`finite_radius` is already the boundary parameter** (in patch units, default
  2 → 5×5). Open mode is literally `world_bound = (0,0,0,0)` ("0,0 = infinite",
  `world.wgsl:1394-1395`), computed purely from `finite_radius` at
  `cartridge.hpp:655-662`.
- **What finite actually flips** — six branches, each independently testing
  `finite_mode`: (1) streaming center pinned to origin (`patch_system.hpp:558`);
  (2) radius capped (`:569`); (3) LOD "all visible," no fog band (`:810`); (4)
  pawn position clamp (`world.wgsl:5663`); (5) camera position clamp + ceiling
  (`:6432`); (6) containment decoration (indoor shell, back-portal, ribbon
  release, finite-only portal distribution).
- **Always streamed — nothing pre-bakes a finite set.** Finite worlds run the
  identical `stream_patches`; the `fullRegen` bootstrap is *not* finite-specific
  (fires on the first frame of *any* world). Finite = "the streaming window
  stops growing and stops following the pawn."
- **`world_bound` is a CONTAINMENT SHELL, not terrain extent** — a clamp on the
  *input coordinate* (pawn/camera), never a property returned by a query. The
  surface extends infinitely past it. `MOOD_FINITE_OUTDOOR` is finite with **no
  walls at all** — just the invisible clamp. And finiteness is only ever
  *entered by walking through a portal* whose destination mood is finite; the
  world always boots open (`DemoConfig` carries no finite field —
  `demo_config.hpp:21` "radii/finiteness later").

**The unification distance:** the manifold model wants (a) the six `finite_mode`
branches collapsed behind one "window = intersect(follow-window, boundary)" rule
where open = boundary-at-infinity, and (b) the query face to carry the boundary
so a query near/over the edge resolves it, instead of clamping `xz` at each call
site. **Generation needs no change — it never knew the world was finite.** The
code is structurally close to Jean's model; the gap is refactor, not rework.

## M4 — THE COMPOSITION ORDER (the tangle, mapped not designed)

**The author list, in fold order.** The ground base is
`contrib_static_base_at` = `terrain_height_at(seed)·tile_amp + tile_bias +
structure_height_at(piers)` (`world.wgsl:2333-2336`) — three authors fused into
one scalar (seed lattice + tile character + spawn-placed piers). The bake adds
pyramids (`ground_formed_with_complexity`, `:2373`). Live ground queries then
add GoL zones, terrain waves, radial pulses, and pawn aura on top — *per policy*.

**The six-consumer / six-ground divergence (exact, from the policy masks,
`ground_architecture.hpp:114-245`):**

| consumer | policy | contributor set | omits |
|---|---|---|---|
| **pawn stands** | WALKER | base + pyramids + GoL + waves + pulses + aura + **GoL-suppression** (flat under feet) | — |
| **pawn tilts** | WALKER_TILT | walker **minus self-aura** | pawn aura |
| **NPCs snap** | WALKER_AGENT | base + pyramids + GoL + waves + pulses + aura (**full GoL, no suppression**) | suppression |
| **camera / spheres / cubes clamp** | FLYER | base + pyramids + GoL + waves + pulses + aura | suppression |
| **entities placed** | BAKED_HEIGHTFIELD (`sample_terrain_y_at`) | **base + pyramids only** | **GoL, waves, pulses, aura** |
| **terrain renders** | TERRAIN_RENDER (`patch_terrain_vs`) | base + pyramids + waves + pulses + aura | **GoL** (zones are a separate extrusion pass — A6) |
| **terrain shadows** | (hand-fused) `shadow_patch_terrain_vs` | baked(base+pyr) + waves | **aura, pulses** |
| **ribbon anchors** | CPU `estimate_terrain_height` | **tile mods only** (coarse) | piers, pyramids, GoL, waves, aura |
| **paintings placed** | PLACEMENT_PAINTING | base + pyramids + **GoL** (re-added analytically, `:8085`) | waves, pulses, aura |

**The precise disagreement:** the pawn walks on a surface that includes GoL and
aura; the entity beside it is placed on a surface that has *neither*; the shadow
it casts is computed from a *third* surface (no aura/pulses); and a ribbon near
it hangs off a *fourth* (coarse tile-only). Today this is **partly masked**
because the wave layer is dead (driverless — see M6), so waves contribute 0
uniformly. But **GoL and aura are LIVE**: where a GoL zone lifts the ground, the
pawn and camera rise with it, placed entities do not (they sample the baked
texture that excludes GoL), and paintings compensate by re-adding GoL by hand.
The masking is a coincidence of the dead voice; revive the wave voice and the
divergence widens. **This is the raw material for "one consistent manifold
query" — mapped, not fixed.**

---

# TARGET 2 — THE RAYMARCHING RESIDUE

## M5 — THE SDF/RAYMARCH EXCAVATION MAP

**Verdict: exactly ONE raymarch-era subsystem survives, and it is essentially
fully ISOLATED — the `TerrainState` / `render_terrain` GPU buffer and everything
that computes into it. It is write-only across the entire shader (no VS/FS
reader, no CPU readback), so it is a whole dead limb. One entangled wire.**

**The smoking gun — `lipschitz_factor` (a cone/sphere-trace step bound):**
`terrain_state.lipschitz_factor = sqrt(1 + max_grad²)` (`world.wgsl:5415`) is
*exactly* the Lipschitz constant of a heightfield graph — the max-slope bound
used to derive conservative safe step lengths when sphere/cone-tracing a terrain
distance field. It is a scalar bound, not a normal, so it cannot be shading data,
and **nothing reads it** (GPU or CPU). Its producer `dynamics_terrain_gradient_max`
(`:3073-3081`) sums the legacy `WAVES` table; the code's own comment names the
provenance: *"Legacy fixed-wave dynamics (Lipschitz bound still alive) … Only
gradient_max survives for Lipschitz factor computation"* (`:3061, 3070`), and
lists the removed SDF siblings (`dynamics_terrain_wave_eval`, `wave_frozen`,
`dynamics_terrain_normal`, …, `:3067-3071`).

**The subsystem (all excises together):**
- The buffer + struct: `TerrainState` (`world.wgsl:643-650`) / `GPUTerrainState`
  (`state.hpp:426-433`, 32-byte assert `:1261`); `Idle::*` init (`state.hpp:187-190,
  5470-5479`).
- **Two bindings, both dead:** compute `binding 20` (`:4748`) and — the tell —
  **`binding 220` `render_terrain`, Fragment-visible read-only** (`:4811`,
  `state.hpp:3605-3607, 4586, 4908`), **read by nobody.** That FS socket is
  precisely where an SDF shade-fragment plugged in (reading `amplitude_scale` /
  `max_amplitude` / `size` for the distance-field envelope, `lipschitz_factor`
  for step sizing, `tint` for the marched surface). Rasterization draws the
  patch mesh + baked texture and reads none of it.
- The writer kernel `update_terrain_config` (all outputs unread) + its dispatch
  (`renderer.hpp:24, 341, 1361-1367`; `render_passes.hpp:185`).
- The legacy `WAVES` table + freeze machinery: `WaveComponent`/`WAVES`/`WAVE_COUNT`/
  `HEIGHT_MAX_AMPLITUDE` (`:1449-1462`), `wave_enabled`/`wave_enable_mask`
  (`:3063, 1380`), and the fully-inert `wave_freeze_mask`/`wave_frozen_t`
  (`:1381-1382`, `state.hpp:317-319, 5416-5420`). The amplitude-trajectory
  feeder (`IDLE_AMPLITUDE_SCALE`/`AMPLITUDE_ATTACK_TIME`/`_RELEASE_TIME` `:1463-1465`
  → `coupling_signal_polyphony_to_terrain_amplitude` → `amplitude_scale`)
  dead-ends into the same limb.

**THE ONE ENTANGLED WIRE:** `terrain_state.tint` is written inside the **live**
`update_sphere` kernel (`:6520-6525`, fed by `coupling_sphere_to_terrain_tint`
`:3034`). Removing the buffer means surgically deleting those three dead-store
lines + that coupling from *within a live kernel* — not a clean file-level lift.
Name it: **`update_sphere → terrain_state.tint`**.

**Explicitly NOT residue (live rasterization — do not tar):** the
finite-difference *normal* functions (`query_ground_*_gradient`,
`terrain_normal_at`, the patch-VS gradient normals) return a `vec3` normal used
for live shading/tilt — they look march-y (eps, finite-diff) but are
load-bearing. And `build_view_projection_matrix`'s `"matches
raymarch_get_direction convention"` comment (`:3218`) is a stale breadcrumb — the
VP math is live rasterization; only the comment is vestigial (`raymarch_get_direction`
is undefined).

## M6 — RESIDUE vs DORMANT-VOICE vs DEAD vs FRAGILE (the disposition table)

The critical separation: **the dead VOICE (waves/palette — a built instrument
switched off, to be REVIVED as DEMO-2) is NOT the SDF RESIDUE (to be
EXCAVATED).** And they are *easy to confuse* — there are **two wave systems**:
the live-render **overlay** voice and the dead **legacy** SDF table.

| item | where | disposition |
|---|---|---|
| **Overlay-wave geometry voice** — `OVERLAY_WAVES`, `contrib_terrain_waves_at`, `terrain_wave_overlay_with_gradient`, `get_band_blend/phase`, the 12 `band_*` config fields | `world.wgsl:2404-2530, 504-526`; `state.hpp:345-356` | **DORMANT-VOICE** (revive) — driverless (`band_blend=-1`), wired into ~24 live VS sites; the geometry half of the voice, awaiting its driver |
| **Substrate temporal machinery** — `terrain_activity_at`, `band_activity_level`, `WAVE_THRESHOLD`, the frozen↔moving mix | `world.wgsl:356-401, 461-497` | **DORMANT-VOICE** (revive) — inert only because both callers pass `t_beats=0`; the substrate's own animation |
| **Color voice** — `animated_cell_color`, `mode_*` dials, `MODE_COUPLING_MAGNITUDE=0.0` archetype→mode coupling | `world.wgsl:7122-7157, 1532, 7030-7037`; `state.hpp:361-365` | **DORMANT-VOICE** (revive) — the palette/color half; the `×0` kill-switch is a held-off coupling |
| **`tile_world` `activation_scale`** — authored/uploaded, no height caller | `tile_world.hpp:108-110`; `world.wgsl:934` | **DORMANT-VOICE** (revive) — the tile's intended input to the band-activity voice; wakes with the temporal voice |
| **`TerrainState`/`render_terrain` buffer + `update_terrain_config` + binding 220** | `world.wgsl:643-650, 4748, 4811, 5412-5415`; `state.hpp:426-433, 3605, 4488` | **SDF-RESIDUE** (excavate) — the raymarch terrain param block; FS socket with no consumer |
| **`lipschitz_factor` + `dynamics_terrain_gradient_max`** | `world.wgsl:647, 3073-3081, 5415` | **SDF-RESIDUE** (excavate) — the cone-march step bound |
| **Legacy `WAVES` table + `wave_enable/freeze/frozen_t`** | `world.wgsl:1449-1462, 3063, 1380-1382`; `state.hpp:317-319` | **SDF-RESIDUE** (excavate) — the animated field the SDF marched + its freeze controls; NOT the overlay voice |
| **amplitude-trajectory feeder** (`IDLE_AMPLITUDE_SCALE`/`ATTACK`/`RELEASE` → polyphony→amplitude → `amplitude_scale`) | `world.wgsl:1463-1465, 1805-1818` | **SDF-RESIDUE** (excavate) — dead-ends into the Lipschitz limb; *confirm before deleting the constants* (name adjacent to the live overlay amplitude) |
| **`terrain_state.tint` + `coupling_sphere_to_terrain_tint` + `SAND_DUNE_*`** | `world.wgsl:3034, 6520-6525, 1466-1467` | **SDF-RESIDUE / LIVE-BUT-FRAGILE** — the residue's one entangled wire (dead store inside live `update_sphere`); the *concept* (spheres tint nearby ground) could re-home as color-voice if wanted |
| **`WAVES→gradient_max→lipschitz_factor` "still alive" comment** | `world.wgsl:3061` | **SDF-RESIDUE** — stale; the limb is dead |
| **`complexity` texel channel (.w)** — baked, unread | `world.wgsl:6974, 6971-6973` | **GENUINELY-DEAD** (delete) — a metric with no reader (note: could be a future LOD/aesthetic input) |
| **entity-density lattice** — `DENSITY_MIN==MAX==1.0`, all `density_mult==1.0` | `tile_world.hpp:65-66, 328-347` | **GENUINELY-DEAD** (delete) — a spawn-density no-op, *not* terrain voice |
| **`ActivePatch::animated`, `RENDER_RADIUS/SIDE/GRID_SIDE`, `VISIBLE_RADIUS_SQ`/`LOD_FULL_RADIUS_SQ`, binding 144 `photo_patch_instances`** | `surface_services.hpp:78-80, 99, 145-148`; `world.wgsl:7841` | **GENUINELY-DEAD** (delete) — superseded aliases/fields, no readers, no revive intent |
| **`POLICY_PLACEMENT_*` rows + `query_ground_placement_*` + all `*_gradient` variants** | `ground_architecture.hpp:121-143`; `world.wgsl:2638-2911` | **LATENT-SCAFFOLD (KEEP)** — *not dead*: this is the manifold's own future interface (M1). The `*_gradient` family is the `coord → position+normal` upgrade point; the placement rows are where placement moves onto the policy API |
| **Hand-mirror hazards** — GoL suppression triple (`:7585-7594`); the hot-path family `ground_formed_with_complexity` / `patch_terrain_vs` / `shadow_patch_terrain_vs` (the shadow copy already diverges, dropping aura+pulses) | `world.wgsl:2370, 3620-3629, 3774` | **LIVE-BUT-FRAGILE** (keep + note) — one role hand-copied across sites that must stay in sync; the composition-consistency risk M4 maps |

---

# STOP — THE MAP

Returned: the query face (M1 — uniformly heightfield-baked, but the `*_gradient`
+ placement scaffold and the `vec3 consumer_pos` are the idle manifold seam);
the heightfield-assumption blast (M2 — LEAF value provider, SPREAD Y-up/XZ across
storage/mesh/normals/movement/streaming); the extent model (M3 — the surface is
already one mechanism; finite is ~6 scattered input-clamp branches, "1.5 code
paths," generation untouched); the composition order + the six-consumer /
six-ground divergence (M4); the raymarch excavation map (M5 — one isolated
`TerrainState` subsystem, one entangled tint wire); and the disposition table
separating DORMANT-VOICE / SDF-RESIDUE / GENUINELY-DEAD / LIVE-BUT-FRAGILE, plus
the LATENT-SCAFFOLD that is the manifold's own future interface (M6).

NO design, NO cuts, NO merges. The distances this map measures, for the
manifold-spine conversation:

1. **The interface is one return-type away.** The query face already carries a
   3D input and a LATENT gradient API; the manifold cast is "give the declared
   query API a position+normal return and move the live consumers onto it,"
   not a new subsystem.
2. **The cast is a rewrite, the value is a leaf.** Swapping *what height* is
   cheap; swapping *away from Y-up/XZ* rewrites storage, mesh, normals,
   movement, and the spatial index.
3. **Finiteness is already a boundary parameter** — the gap to "one manifold
   with a boundary condition" is collapsing six imperative clamps into one
   window rule + a boundary-carrying query; generation needs nothing.
4. **The composition tangle is the real spine question** — six consumers read
   six different grounds; the wave-deadness masks it today, and reviving the
   voice widens it. One consistent manifold query is the design target M4 lays
   out the raw material for.
5. **Wire first, clean second (Jean's order):** the SDF residue is one
   isolated excavation (after the manifold wiring, when "dead" is decidable),
   the dormant voice is a separate revival (DEMO-2), and the `*_gradient`
   scaffold is neither — it is kept, because it is where the manifold lands.
