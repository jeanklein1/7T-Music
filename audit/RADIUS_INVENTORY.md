# RAD-1 — VISIBILITY RADIUS INVENTORY (read-only)

the_board, branch `FINAL_TOUCH`, anchored at HEAD `94238e9`. **No code
changed.** This pass measures every radius/threshold in the visibility
pipeline (§1), renders the sync verdict for concern (i) with numeric
boundary analysis (§2), and scopes the smooth-reveal arc for concern (iii)
without designing it (§3). RAD-2 (sync fix) and RAD-3 (reveal) are ruled
by Jean against these values.

## §1 — RADIUS / THRESHOLD TABLE

Frames: **P** = pawn-relative (from `player_.readback_x/z`, the GPU
readback of the pawn world position); **C** = recenter-cursor grid-coord
(`world_state_.last_center_x/z`); **cam** = camera/view frustum; **W** =
world/sky-anchored. All wu = world units; PATCH_EXTENT is the unit bridge.

| Symbol | Site | Value | Unit | Frame | Role |
|---|---|---|---|---|---|
| `PATCH_EXTENT` | state.hpp:103 | **50** | wu/patch | — | the unit bridge (patches ↔ wu) |
| `PATCH_GRID_RADIUS` | state.hpp:106 | **3** (7×7) | patches | C | inner priority ring |
| `PATCH_RENDER_RADIUS` | state.hpp:108 | **5** = 250 wu | patches | C | *retained/legacy* — NOT the visibility gate (see note) |
| `PATCH_PREGEN_RADIUS` | state.hpp:110 | **7** = 350 wu | patches | C | deep pre-gen **allocation** ring (15×15) |
| `MAX_ACTIVE_PATCHES` | state.hpp:112 | **225** = 15² | patches | — | all patch buffers/textures sized to this (= PREGEN) |
| `TILE_GRID_SIDE` | state.hpp:486 | **17** = 2·(7+1)+1 | tiles | C | modifier tile grid (pregen + 1 pad) |
| `active_radius` | cartridge.hpp:563 | **7** (=PREGEN at boot) | patches | C | streaming/allocation window; capped to `finite_radius` in finite mode (3512) |
| `VISIBLE_RADIUS` | cartridge.hpp:2514 | **5.5** | patches | P | **the terrain visibility ring** |
| `VISIBILITY_CYLINDER_RADIUS(_SQ)` | cartridge.hpp:2532-2533 | **275 wu** (75 625 wu²) | wu / wu² | P | terrain visible ⇔ patch-edge distance ≤ this |
| `LOD_FULL_RADIUS` | cartridge.hpp:2520 | **3.5** | patches | P | LOD-0/LOD-1 split |
| `LOD0_CYLINDER_RADIUS(_SQ)` | cartridge.hpp:2534-2535 | **175 wu** | wu / wu² | P | the ~175 wu ring (seam investigation) |
| `FORGET_RADIUS` | cartridge.hpp:1876 | **9** = PREGEN+2 | patches | C | patch eviction (Chebyshev/box in grid coords) |
| `finite_radius` | cartridge.hpp:871 (`derive_finite_radius`) | mood-driven, `min + hash%range` | patches | C | finite-world ring; world bounded to `[-fr·50, (fr+1)·50]` wu |
| `ENTITY_CULL_BASE` | spawn_engine.inl:316 | **350 wu** = PREGEN·EXTENT | wu | P | entity distance-cull floor ("never cull inside pregen") |
| `ENTITY_CULL_ARCH_SCALE` | spawn_engine.inl:317 | **2.5** | wu / wu of size | P | widen far by `max(span, height)` |
| `ENTITY_CULL_COL_SCALE` | spawn_engine.inl:318 | **3.0** | wu / wu of height | P | widen far by column height |
| `ENTITY_CULL_HYSTERESIS` | spawn_engine.inl:319 | **50 wu** | wu | P | show/hide band width |
| per-entity `cull_far` | spawn_engine.inl:404/434/465 | **≥ 350 wu**, grows w/ size | wu | P | `BASE + size·scale`; `cull_near = far − 50` |
| `FLOATER_EVICTION_RADIUS(_SQ)` | world.wgsl:6152-6153 | **400 wu** | wu | P (GPU) | sphere/cube despawn (separate GPU stage) |
| `allow_frustum_cull` | cartridge.hpp:349; set mood.inl:664 | per-mood bool | — | cam | GPU LOD-0 terrain frustum cull — **separate stage**, mood-gated |
| `ORB_DOME_RADIUS` | orbs.inl:58 | **450 wu** | wu | W | sky orb dome — separate ORB pass, note only |

**Allocation vs. visibility (explicit).** Every patch buffer/texture is
sized to `MAX_ACTIVE_PATCHES = 225 = PREGEN_SIDE²` (the **350 wu** ring) —
patches are *generated/retained* out to PREGEN. But terrain is only
*drawn* within `VISIBILITY_CYLINDER_RADIUS = 275 wu`. Allocation ring
(350) ≠ visibility ring (275). `ENTITY_CULL_BASE` was tied to the
**allocation** ring, not the visibility ring — this is the root of the
desync (§2).

**Note on `RENDER_RADIUS` (5 / 250 wu).** Despite the name, it is *not*
the visibility gate. cartridge.hpp:561: "Visibility uses circular
`VISIBLE_RADIUS`; `RENDER_RADIUS` retained for allocation bounds and GoL
zone eviction." The live terrain visibility radius is `VISIBLE_RADIUS =
5.5` (275 wu), not 5.

**Distance metrics differ.** Terrain uses `patch_distance_sq`
(cartridge.hpp:2539) = point-to-patch-**AABB** with `half = 25 wu` (zero
inside the patch). Entities use point-to-**point** Euclidean
(`sqrt(dx²+dz²)`, spawn_engine.inl:401). Both are pawn-relative from the
same `readback_x/z`, so they are directly comparable up to this ~25 wu
edge-vs-center offset.

## §2 — THE SYNC VERDICT (concern i)

### a. Same frame? Convertible?

**Yes.** Both are **pawn-relative** from the identical source
`player_.readback_x/z`:
- terrain: `pawn_wx = player_.readback_x` (cartridge.hpp:3790),
  `d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, 25)` vs
  `VISIBILITY_CYLINDER_RADIUS_SQ` (3810).
- entity: `dx = a.world_x − player_.readback_x` … `dist` vs `cull_far`
  (spawn_engine.inl:399-408).

Both in wu, same origin. Conversion is identity; the only wrinkle is the
edge-vs-center metric (~25 wu), folded into the analysis below.

### b. Ordering at the boundary — lead hypothesis CONFIRMED

For a patch column at pawn-distance `d` (center-to-center):
- **Terrain** patch is drawn while its nearest edge ≤ 275, i.e. center
  distance `d ≲ 275 + 25 = 300 wu` (open mode).
- **Entity** on it is drawn while its center ≤ `cull_far` = **≥ 350 wu**.

So there is a band — roughly **`d ∈ (300, 350+] wu`** (raw radii 275 vs
350; ~50 wu after the edge/center offset, ≥75 wu ignoring it) — where the
**entity draws but the terrain beneath it does not**. The entity floats
over culled ground. The lead is **≥ ~50–75 wu at the base and grows with
entity size**: a tall column (height 40 × COL_SCALE 3 = +120) has
`cull_far = 470`, leading terrain by ~170 wu. **Hypothesis confirmed
numerically**, and it is worse for larger entities.

Cause, exactly as suspected: `ENTITY_CULL_BASE = PATCH_PREGEN_RADIUS ·
PATCH_EXTENT = 350` (spawn_engine.inl:316) is tied to the **pre-gen /
allocation** ring, while terrain visibility uses the **VISIBLE** ring
(275). The cull floor should have referenced the visible ring.

### c. Is `ENTITY VISIBLE ⇒ TERRAIN BENEATH VISIBLE` enforced? — NO

The entity visibility pass `update_entity_draw_visibility`
(spawn_engine.inl:390-489) toggles `draw_visible` **purely by distance**
(`dist ≤ cull_far/near`) for arches, columns, antennas. There is **no
query of patch residency or terrain visibility** anywhere in the path
(grep: none). The two culls are fully independent; nothing couples an
entity's visibility to the ground under it.

### d. Do entities ALWAYS lead, or size-dependent?

**Always lead, and the lead scales with entity size.** `cull_far = BASE +
size·scale ≥ 350 > 300`, monotonically increasing in size. No entity
culls before its terrain; larger entities lead more. (One consistent
direction — no family pops *after* its terrain.)

**Finite mode is exempt.** The terrain test is `finite_mode || d2 ≤
CYLINDER_SQ` (cartridge.hpp:3810) — in finite mode the short-circuit makes
**all in-world patches visible**, and the finite world (`fr` ≈ 2–4 ⇒
≤ ~200 wu) is smaller than both radii, so no lead band exists. **The
desync is an open-world phenomenon only.**

### Fix classification for RAD-2 (NOT applied)

**ONE-RELATIONSHIP — warranted.** Frames and units already match (both
pawn-relative wu), so a scalar re-base suffices; no residency query is
needed. Rebase the entity cull onto the **terrain-visible** ring with a
margin so entities cull *inside* the terrain edge:

- Set `ENTITY_CULL_BASE = (VISIBLE_RADIUS − 0.5) · PATCH_EXTENT = 5.0 · 50
  = 250 wu` (a full-patch, 50 wu margin inside the 300 wu terrain center-
  reach). Then a base entity culls at center-distance 250, terrain reaches
  ~300 → entity is always over solid ground, ~50 wu inside the edge.
- **Decouple the additive size-widening from `cull_far`.** The widening
  (`+ size·scale`) is what re-introduces the lead for tall entities; it
  must not push `cull_far` past the terrain edge. Its original intent —
  keeping tall landmarks from popping at the screen's far edge — is a
  *reveal* concern, not a *ground-beneath* concern, and belongs to RAD-3's
  fade, not to drawing an entity beyond its terrain. Options: drop the
  term from `cull_far`, or fold it only into `cull_near`/hysteresis so it
  affects the show/hide band, never the outer bound.
- Keep the 50 wu hysteresis (`cull_near = cull_far − 50`).

Net invariant achieved: `max entity cull_far (250) < terrain visible
(~300)` ⇒ **entities never lead terrain**; they reveal ~50 wu inside the
terrain edge, fully over ground. **STRUCTURAL (residency-query) is not
warranted** — it would only be needed if entity and terrain lived in
different frames or if per-entity terrain visibility were non-monotonic in
distance; neither holds here.

Trade-off to flag for the ruling: rebasing to 250 means an entity now
culls *before* its terrain, so a patch can briefly show as bare ground
before its entity reveals on approach — the safe direction (never a
floating entity), and exactly what RAD-3's fade would smooth.

## §3 — SMOOTH-REVEAL SCOPE (concern iii — design notes only)

### Where the POP happens today (both binary)

1. **Terrain:** a patch flips visible/invisible at
   `VISIBILITY_CYLINDER_RADIUS = 275 wu` (binary include/exclude in the
   LOD band pack, cartridge.hpp:3810). No fade.
2. **Entities:** each flips `draw_visible` at its `cull_far` (≥350 wu),
   with 50 wu hysteresis — a step with a deadband, still a pop
   (spawn_engine.inl:404-409 etc.).

### Fade path — DITHER, confirmed by the blend state

Pipeline blend audit (renderer.hpp):
- **Terrain** — "Patch Terrain (instanced)" (1986) + "Patch Terrain
  Indirect" (2017): colorTarget has **no `.blend`** → **opaque**.
- **Solid entities** — Pawn/Sphere/Monolith/Catenary Arch/Column/Palm/
  Cactus/Blade/Pyramid/Indoor Shell/Sky Ribbon (2096-2310): all
  "(Rasterized)", **no `.blend`** → **opaque**.
- Alpha-blend (`SrcAlpha/OneMinusSrcAlpha`) is used **only** by Gallery
  Frame + Wall-Painting passes (2408) and the full-screen "Fade Overlay"
  (2875-2884); additive only by "Orb Sky Layer" (2342). None of these are
  the terrain or the solid entities.

Because terrain and entities are opaque depth-tested geometry, a true
alpha-blend reveal would fight depth ordering. The correct path is
**distance-driven DITHER** (screen-door / `discard` on a threshold pattern
keyed to a distance factor) in each fragment shader — order-independent,
depth-compatible, no blend-state change, and it "fits the aesthetic" as
noted. One shared `world.wgsl` helper `reveal_dither(dist, edge, band)`
called from each FS.

### Interactions to flag

- **vs. LOD boundary:** the reveal band wants to sit at the **VISIBLE**
  ring (275 wu, the outer edge), *independent* of the LOD-0/LOD-1 split at
  175 wu. Do not couple the fade to LOD; they are different rings.
- **vs. the precision seam:** a reveal fade at the far edge would
  **cosmetically mask** a precision crack that happens to fall in the fade
  band. It must **not** be mistaken for curing it — RAD-3 and the
  floating-origin fix (from `INVESTIGATION_mood_seam.md`) are independent;
  the seam can appear anywhere inside the visible field, not only at the
  fade edge.
- **vs. the transition fade:** the "Fade Overlay" (2884) is a global
  *time* fade during mood transitions — orthogonal to a per-*distance*
  reveal band. Don't conflate them.

### Arc-shape estimate (NOT a design)

A reveal band touches the fragment shaders of **~2 terrain pipelines +
~11 solid-entity pipelines** (≈13 FS) plus **one shared dither helper** in
world.wgsl, and a CPU-side band constant feeding each. Wide but
**mechanical** (one technique applied uniformly); shadow passes and the
alpha gallery/orb passes are out of band. Sequencing: RAD-2 (rebase the
cull so the reveal band is the *same* ring for terrain and entities)
should land **before** RAD-3, so the fade has a single coherent edge to
work against.

## SCOPE BOUNDARY

Concern (ii) is answered in full (§1 + frames). Concern (i) has its
verdict and a classified, arithmetic-backed fix (§2) — **RAD-2 not
applied.** Concern (iii) is scoped only (§3) — **RAD-3 not designed.**
Nothing committed to code.

---

**ADDENDUM (2026-07-11, TER-2 1e) — RAD-2 has landed.** The §1
`ENTITY_CULL_BASE = 350` rows and §2's "fix classification" above are the
pre-RAD-2 record: the live cull base is now
`cull_base = VISIBILITY_CYLINDER_RADIUS − ENTITY_CULL_EDGE_MARGIN
= 275 − 25 = 250 wu`, with the size term re-signed to a capped inward
inset (spawn_engine.inl, Entity Distance Culling block). Entities never
lead terrain.
