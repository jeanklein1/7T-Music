> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# THE VEIL — VISIBILITY RECON (read-only; the design map for a single-authority veil)

Campaign: the veil — collapse the many visibility authorities into ONE point-
anchored radius chain (R_pregen > R_band > R_veil), so the stage ends where
awareness ends. This recon VERIFIES the claim (multiple authors, mismatched
radii/metrics/anchors) and maps the ground for the design ruling. Line numbers
are HEAD (post-`c37bbbe`). **Nothing moved; `git status` clean but for this file.**
METHOD: three read-only readers (authority table · radius chain + dead constants ·
witness + feasibility), synthesized here and cross-checked by hand at the two
pivot sites (`band_patches`, `update_entity_draw_visibility`, `lod_pawn`).

---

## §0 THE VERDICT (up front)

**CONFIRMED with one correction.** Visibility has **9–10 distinct authors** that
disagree on **radius** (7 different values) and **metric** (4 different kinds).
But the claim's third axis — *anchor* — is **REFUTED for the streaming/LOD/cull
authors**: p1b already migrated every one of them to **the point**. The anchor
"disagreement" is not point-vs-pawn; it is (a) **CPU-stale vs GPU-live** copies of
the *same* point, (b) the **`lod_pawn` NAME fossil** (carries the point, says
"pawn"), and (c) the **fog/fade machinery is EYE-anchored** (`render_camera.pos`),
which a naïve veil would wrongly inherit. So the veil's job is not "re-anchor to
the point" (mostly done) but "**one witness, one chain, one metric**" — collapse
the copies, unify the radii, pick one distance kind, and add the render-stage
point the fade needs.

The two symptoms are real and pinned (§2): **flora/agents/floaters draw ~350–400
wu, past the 275 terrain band** (grid-residency / existence-eviction, no draw
cull); **arch/column/antenna reveal at ~150–210 wu, inside the 175 LOD0 ring**
(center-distance cull with size-inset + hysteresis).

---

## §1 THE AUTHORITY TABLE (with the WITNESS column)

Frame order visibility is authored (spine, `cartridge.hpp:1413-1436`):
`WitnessHarvest` (sets `player_.readback_x/z`, **1-frame stale by law E-4**,
`cartridge.hpp:595,604`) → `StreamPatches` (**band_patches** + **lod_pawn upload**
+ **update_entity_draw_visibility**) → `EntityMeshGen` → `DispatchCompute`
(**floater/agent eviction**) → `GolZoneCompute` → `OrbSky` → `GroundEntries` →
`FrustumCull` (**frustum_cull_patches**) → `ShadowPass` → `MainPass` → `SnapshotPass`.

WITNESS legend: **P-stale** = `player_.readback_x/z` (the point, 1-frame stale, CPU);
**P-lag** = `config.lod_pawn_x/z` (the point, staged copy of P-stale, GPU);
**P-live** = `point_pos()` (the point, live, GPU compute); **EYE** = `render_camera.pos`
(the live camera — NOT the point in pawn-host 3rd person); **VP** = live camera
frustum (`fc_vp`); **none** = no per-frame anchor.

| family | GATE | RADIUS | METRIC | WITNESS | HYST / INSET | file:line |
|---|---|---|---|---|---|---|
| terrain LOD0 | GPU `frustum_cull_patches`: frustum AABB **AND** LOD0 dist → indirect draw | **175** (3.5) | frustum planes **+ nearest-edge dist²** | **VP (live) + P-lag (stale)** — split | none | `world.wgsl:8347,8389-93`; `FRUSTUM_LOD0_RADIUS_SQ:8305`; dispatch `render_passes.hpp:226` |
| terrain LOD1 | CPU `render_patch_count − lod0_patch_count`, direct DrawIndexed | (175,**275**] ring | nearest-edge dist² (`patch_distance_sq`) | **P-stale** | none | `patch_system.hpp:588-598,431,608-609`; draw `render_passes.hpp:389-398` |
| arch | CPU `update_entity_draw_visibility` → `draw_visible` → empty/real mesh params | **250** − inset | **center-distance** | **P-stale** | **hyst 40**; inset ≤60 (taller→earlier) | `spawn_engine.hpp:369,372,378-389`; guard `world.wgsl:9099` |
| column | same author (columns loop) | 250 − inset(height) | center-distance | P-stale | hyst 40; inset ≤60 | `spawn_engine.hpp:407-420` |
| antenna | same author; drawn via **column pipeline** (+SLOT_OFFSET) | 250 − inset(height) | center-distance | P-stale | hyst 40; inset ≤60 | `spawn_engine.hpp:438-452` |
| palm | **grid residency only** — `is_active`; mesh-gen zeroes if inactive; VS no test | ~**350** (pregen window) | **grid residency** (Chebyshev) | patch-window center (P-stale/50) | none | VS `world.wgsl:9979`; guard `9744`; window `patch_system.hpp:705-706` |
| cactus | grid residency only (as palm) | ~350 | grid residency | stale window | none | VS `world.wgsl:10309`; guard `10395` |
| blade | grid residency only (as palm) | ~350 | grid residency | stale window | none | VS `world.wgsl:10541`; guard `10395` |
| sphere | VS scale-to-zero on `is_active`; killed by **GPU floater eviction** | **400** (evict) | center-distance | **P-live** | none | VS `world.wgsl:4114`; evict `6577`; const `6359` |
| cube (monolith) | VS scale-to-zero; GPU floater eviction | **400** (evict) | center-distance | **P-live** | none | VS `world.wgsl:4135`; evict `6741` |
| ribbon | CPU picks **single nearest active** `rendered_slot`; `is_visible` bit; draw gated `ribbon_active` | nearest-of (no radius) | **center-distance (nearest-of)** | **P-stale** (pick) | none | `ribbon.hpp:930-962`; VS `world.wgsl:4609`; DrawBind `render_passes.hpp:406` |
| GoL zone | draw gated `zone_count>0`; geometry grid-residency; FS camera fog-fade | ~350 residency; FS fade 150/300 | **grid residency** + FS camera-dist fade | window (stale) + FS **EYE** | none | VS `world.wgsl:7647`; guard `render_passes.hpp:407`; fade `3800` |
| indoor shell | always drawn (`dt_shell`); geometry exists iff mood built it | none (whole room) | none | **none** (absolute world verts) | none | VS `world.wgsl:4352`; row `drawable_table.hpp:121` |
| gallery frames | draw skipped if `active_painting_count==0`; VS scale-to-zero | ~350 residency | **grid residency** + count | `slot.position` | none | VS `world.wgsl:8483,8491`; count `renderer.hpp:955` |
| wall paintings | draw skipped if `wall_frame_count==0`; VS scale-to-zero | count-gated (indoor) | **runtime count** | `slot` | none | VS `world.wgsl:8728,8743`; count `renderer.hpp:969` |
| orbs | draw skipped if `count==0`; **camera-anchored skybox dome**, no world cull | none (sky, r≈500) | none (dome billboards) | **EYE** (`render_camera.pos`) | none | VS `world.wgsl:11380,11405`; dome r=500 `orbs.hpp:44` |
| fade overlay | draw skipped if `fade_alpha<0.001`; fullscreen triangle | fullscreen | none | **none** (clip-space) | none | VS `world.wgsl:4844`; guard `renderer.hpp:990` |
| pawn | VS scale-to-zero `is_active`; possessed slot **never evicted**; others 360 | none (possessed) / 360 (others) | (possessed none) / center-dist | possessed exempt; others **P-live** | none | VS `world.wgsl:4012`; exempt `6405`; evict `6437` |
| pyramids | **NOT DRAWN** (VS cut in C2) — terrain heightfield contributor only | n/a | n/a | n/a | n/a | contributor `world.wgsl:2042` |

Every entity draw is additionally wrapped in compile-time `if constexpr
(!ROSTER.<family>) return;` (`renderer.hpp:804,821,836,954,968,983,989`).

**THE WITNESS FINDING (Jean's added column):** **no author reads a raw pawn.**
p1b migrated all of them — CPU authors read **P-stale** (`player_.readback` = the
point, `spine_state.hpp:84-85`, `cartridge.hpp:979-988/1052-1054`), GPU authors
read **P-live** (`point_pos()`, `world.wgsl:4919`; eviction comments literally say
"p1b-b: was the possessed slot", `:6428/:6573`). The three "anchor" gaps are:
1. **Staleness split** — CPU authors use P-stale (1-frame lag); GPU authors use
   P-live; `frustum_cull_patches` is split *within itself* (live VP planes + stale
   P-lag distance) — the deliberate anti-flicker contract, `world.wgsl:8382-8388`.
2. **`lod_pawn` NAME fossil** — the field (`state.hpp:389-390`, offset-384
   asserted `:1676`), the setter `stage_lod_pawn` (`:2206`), the locals
   `pawn_wx/pawn_wz` (`patch_system.hpp:568-569`), and the shader comment
   (`world.wgsl:8382-83`) all say "pawn" but move **the point**.
3. **EYE-anchored fog/fade** — `shade_lit` fog, GoL zone fade, gallery/wall fog all
   use `render_camera.pos` (`world.wgsl:3646-49, 3800, 8572/8766/8776`). Legit as a
   *view* effect (eye==point in camera-host), but in pawn-host 3rd-person the eye
   orbits off the point. **Not a streaming author that failed to migrate — but
   exactly what a naïve veil would reuse, and reusing it verbatim makes the veil
   eye-anchored (§4).**

---

## §2 THE TWO SYMPTOMS PINNED (verified, not assumed)

**Symptom A — BEYOND-RADIUS entities (drawn past the terrain band).** The terrain
draws to **275** (5.5) and demotes patches at 275–350 to undrawn `pregen`
(`patch_system.hpp:588`). But three author-classes draw past 275:
- **Flora (palm/cactus/blade)** — gated by **grid residency only** (`is_active` on
  the pregen window ~350; VS has no distance test). They render on patches the
  terrain band has demoted to `pregen` → flora floating past the visible ground
  edge. (Confirmed: `update_entity_draw_visibility` covers arch/column/antenna
  ONLY — `spawn_engine.hpp:369-470` — flora is absent, so nothing clips it.)
- **Agents 360 / spheres+cubes 400** — gated by **existence eviction**, not a draw
  cull. They persist (and draw via `is_active`) out to 360/400, past both the
  terrain band (275) and the patch residency (350). This is the recon's
  **VIOLATION 1** (§3): existence radius > patch residency.
The suspect ("grid-residency 7=350 vs cylinder 5.5=275") is **correct and
broader**: it is grid-residency (350) for flora AND existence-eviction (360/400)
for movers, both exceeding the 275 draw band, none using the cylinder.

**Symptom B — LATE-REVEAL (arch/column/antenna appear inside the ground).**
`update_entity_draw_visibility` (`spawn_engine.hpp:369`): `cull_base = 275 − 25 =
250`; per entity `cull_far = 250 − inset` (inset = `min(size·0.5, 60)`, so a tall
arch loses up to 60 → cull_far 190); `cull_near = cull_far − 40` (hysteresis). A
hidden entity shows only inside `cull_near`: **small ≈ 210 wu, tall ≈ 150 wu**,
measured **center-distance** from P-stale. Meanwhile the *terrain* under it is
already full-detail out to 175 (LOD0) and drawn out to 275. So the ground (and its
flora) appear well before the arch/column standing on it — the suspect (inset +
hysteresis) is **confirmed**, and it compounds with the metric mismatch (the
entity uses center-distance to its origin; the terrain uses nearest-edge).

**The deep cause of both: three metrics, one of them measured to a different
feature.** Terrain = **nearest-edge** (a patch is "here" when its closest edge is
within R). Entities = **center-distance** (a monolith is "here" when its center is
within R). Flora/zones = **grid-residency** (in the window at all). So at the same
R, entities read as *closer* (center) or *farther* (residency window) than the
terrain they sit on. A single veil must pick ONE metric (recommend nearest-point-
of-entity vs the point, or plain center — but the SAME for terrain and entities).

---

## §3 THE CHAIN TODAY (values + violations)

Sorted by value (PATCH_EXTENT = 50 wu):

| radius (role) | value | patches | source |
|---|---|---|---|
| floater existence (sphere/cube evict) | **400** | 8.0 | `world.wgsl:6359` |
| agent existence (evict) | **360** | 7.2 | `world.wgsl:6330`, `agents.hpp:115` |
| **R_pregen** — patch residency / grid window | **350** | 7.0 | `state.hpp:68`; `surface_services.hpp:48` |
| **R_draw** — terrain visibility cylinder | **275** | 5.5 | `surface_services.hpp:149`; used `patch_system.hpp:588` |
| **R_reveal** — entity cull base (arch/col/ant) | **250** | 5.0 | `spawn_engine.hpp:372` (−inset ≤60, −hyst 40) |
| **R_band** — terrain LOD0 (full mesh) | **175** | 3.5 | CPU `surface_services.hpp:151`; GPU `world.wgsl:8305` |
| point awareness sensor (NOT visibility) | 20 | 0.4 | `world.wgsl:6368`, `point.hpp` |

LOD bands: **LOD0 0–175** (full 64×64), **LOD1 175–275** (half 32×32, direct
draw), **pregen 275–350** (resident, undrawn). The visible-terrain sub-chain **350
> 275 > 175 is monotonic and correct.** The design's `R_pregen > R_band > R_veil`
is **VIOLATED in three places:**

- **V1 — existence overshoots residency.** floater 400 > pregen 350 (**+50 / 1.0
  patch**); agent 360 > 350 (**+10 / 0.2 patch**). Movers exist where no patch is
  resident (`world.wgsl:6437,6577,6741`). Breaks R_pregen ≥ R_exist.
- **V2 — reveal exceeds the band.** entity reveal 250 > LOD0 band 175 (**+75 / 1.5
  patch**). Entities draw full-detail on half-res LOD1 terrain (175–250). If
  "R_veil" is the entity reveal, then today **R_veil (250) > R_band (175)** — the
  veil is *bigger* than the band, inverted from the design goal.
- **V3 — the outer visible radius is 275, not 175.** R_band (175) is NOT the outer
  edge of visible terrain (that's 275, LOD1). So "the band" is two numbers (175
  full, 275 outer); the veil must decide which one R_veil chases. (`+100` LOD1 fill
  by design, `patch_system.hpp:588`.)

Stale in-code claim to correct: `world.wgsl:6319-6320` asserts a single 350 anchor
shared by cull-base and eviction — **false**: live cull-base is 250, live eviction
is 360/400, none is 350.

---

## §4 THE VEIL FEASIBILITY

**The shared-FS fact — `shade_lit` is the universal choke.** `ENTITY_FS`
(`renderer.hpp:48`) is a one-liner calling `shade_lit(world_pos, normal, color)`
(`world.wgsl:4126-28`), shared by ~10 pipelines via `makeEntity` (pawn, sphere,
monolith, arch, column, palm, cactus, blade, shell — `renderer.hpp:1696-1799`) +
ribbon (`:1808`). **Terrain uses a separate FS** (`patch_terrain_fs`,
`world.wgsl:3767`) but it **also returns `shade_lit(...)`** (`:3867`). So
`shade_lit` (`world.wgsl:3631-3650`) is where a **fragment veil reaches terrain +
every entity_fs family (including flora) in one edit** — and it already has
`world_pos` and already computes a distance fade (the fog). Own-FS forks that
bypass it: orbs `ORB_FS`, gallery/wall `*_FS`, fade `FADE_OVERLAY_FS`, GoL
`ZONE_EXTRUSION_FS`.

**The anchor fact — `lod_pawn` (config @384) carries THE POINT; the name is a
FOSSIL.** Written from `player_.readback_x/z` at `patch_system.hpp:617`. So the
veil MAY reuse this slot as a point-anchor — but only after the rename (per Jean's
ruling); until then the slot's *name* lies. NOTE the deeper trap: the fog/fade the
veil wants to reuse is **EYE-anchored** (`render_camera.pos`), NOT the point. **A
render-stage point accessor does not exist** — `point_pos()` (`world.wgsl:4919`) is
compute-only (reads compute bindings). The render stage has `render_camera` (280)
and `render_pawn_pos()` (`:4934`); a render-point = `point_camera_hosted() ?
render_camera.pos : render_pawn_pos()` (`point_camera_hosted()` reads
`config.point_host`, available everywhere, `:1796`) — trivial to add, **the veil's
one prerequisite**.

**Flora VS needs — nothing, for a fade.** `palm_vs`/`cactus_vs`/`blade_cluster_vs`
(`world.wgsl:9979/10309/10541`) already emit `world_pos` and route through
`entity_fs`→`shade_lit`; a distance-to-point fade in `shade_lit` reaches flora for
free. A VS change is needed ONLY to *remove* geometry (per-instance
`distance(world_pos.xz, point.xz)` → scale-to-zero), an overdraw optimization, not
a correctness need — and it too needs the render-stage point.

**Fade vs dissolve vs fog-wall.** Order model: opaque depth-tested geometry is
draw-order-independent; only orbs (additive) + fade (alpha) are order-sensitive.
So an **alpha fade to transparent on opaque geometry is order-SENSITIVE** (needs
blend + sort) — avoid. Order-safe options, ranked least-machinery first:
1. **FOG-WALL** — reuse the existing `shade_lit` fog `mix(lit, wall_color, band)`,
   re-anchored on the point and banded with `smoothstep(NEAR, FAR)`. Zero new blend
   state, hits terrain + all entity_fs at once, order-independent. The GoL zone
   fade already proves the exact shape (`GOL_FADE_NEAR=150/FAR=300`,
   `world.wgsl:5058-59,3800`). **Recommended.**
2. **DITHER / DISSOLVE discard** keyed on distance-to-point — order-independent,
   truly removes pixels, but NEW machinery (no dither exists today).
3. alpha-fade — order-sensitive, rejected.

**Discipline 2 — the GENUINE forks (must NOT join the point-band veil):**
- **ORBS** — eye-centered skybox dome, `render_camera.pos`, radius **500**
  (`orbs.hpp:44`; the ladder's 700 was walked back to 500), additive + depth-no-
  write, own `ORB_FS` (`world.wgsl:11405`). It IS the far backdrop — the veil's
  *wall*, not its subject. FORK.
- **FADE OVERLAY** — fullscreen scene-transition, alpha, no `world_pos`
  (`world.wgsl:4854`). A temporal transition orthogonal to a spatial veil. FORK.
- **RIBBON** — a flown sky structure, NOT band-bounded (cruises far, wander legs
  200u, meant to be seen/ridden; `ribbon.hpp:119-149`). It SHARES `ENTITY_FS`, so a
  `shade_lit` veil WOULD hit it — **exempt it** (gate on `ribbon.is_visible`, don't
  point-band it). FORK.
- **GALLERY / INDOOR** — own FS + own fog; **wall-bounded** interiors
  (`world_bound_min/max`, camera clamp `world.wgsl:6541-52`). Visibility is walls,
  not a point radius. FORK.

---

## §5 THE DEAD GENERATIONS (fossil visibility constants)

**Truly DEAD (zero readers; self-documented "dead render radius vocabulary",
`patch_system.hpp:554-556`):**
| name | value | file:line |
|---|---|---|
| `Dim::PATCH_RENDER_RADIUS` | 5 (250 wu) | `state.hpp:66` |
| `Dim::PATCH_RENDER_SIDE` | 11 | `state.hpp:67` |
| `RENDER_RADIUS` (alias) | 5 | `surface_services.hpp:79` |
| `RENDER_SIDE` (alias) | 11 | `surface_services.hpp:80` |
| `VISIBLE_RADIUS_SQ` | 30.25 (5.5², patch-count form) | `surface_services.hpp:145` |
| `LOD_FULL_RADIUS_SQ` | 12.25 (3.5², patch-count form) | `surface_services.hpp:148` |

A single-authority veil retires **all six** (they are the pre-cylinder grid-render
generation; the ancestors still show a runtime-toggleable render radius, so these
are true fossils the_board superseded).

**LIVE-REDUNDANT (multiple live spellings a veil unifies to one authority):**
- **Patch draw** — `VISIBLE_RADIUS 5.5` / `VISIBILITY_CYLINDER_RADIUS 275` / `_SQ`
  (3 spellings of one 275 gate). `surface_services.hpp:144,149,150`.
- **LOD0 band** — `LOD_FULL_RADIUS 3.5` / `LOD0_CYLINDER_RADIUS 175` / `_SQ` **+
  GPU `FRUSTUM_LOD0_RADIUS_SQ 30625`** — **four** spellings across CPU/GPU, hand-
  kept, MUST match or the LOD0/LOD1 boundary flickers. `surface_services.hpp:147,
  151,152` + `world.wgsl:8305`. The classic CPU↔GPU duplication.
- **Entity reveal** — `ENTITY_CULL_EDGE_MARGIN 25` + derived `cull_base 250` +
  `_SIZE_INSET/_MAX/_HYSTERESIS`. `spawn_engine.hpp:50-53,372`.
- **Existence** — `AGENT_EVICTION_RADIUS 360` (CPU `agents.hpp:115` **has no live
  CPU reader** — borders DEAD, survives only as the GPU-mirror source) + GPU
  `world.wgsl:6330`; `FLOATER_EVICTION_RADIUS 400` GPU-only. + `POINT_BUBBLE_RADIUS
  20` (awareness sensor, same mirror pattern). A veil = one existence radius, one
  anchor, auto-mirrored.

**Stale naming (live-but-misleading):** `render_patch_count`/`lod0_patch_count`
comments name the DEAD `VISIBLE_RADIUS`/`LOD_FULL_RADIUS` rather than the live
cylinders (`surface_services.hpp:59-60`); `lod_pawn`/`pawn_wx`/frustum "pawn"
comments (§1); the false 350-unification comment (`world.wgsl:6319`).

---

## §6 FOR THE DESIGN CONVERSATION (the ruling surface)

Jean's declaration: **the veil's anchor is THE POINT — one witness, one chain
(R_pregen > R_band > R_veil), every family reads both.** The recon says this is
*achievable and half-built* (all authors already read the point), and surfaces the
decisions the cut needs:

1. **THE PREREQUISITE (not optional):** add a **render-stage point accessor**
   (`render_point_pos()` = `point_camera_hosted() ? render_camera.pos :
   render_pawn_pos()`). Without it the veil is eye-anchored (the fog trap, §1/§4).
2. **ONE METRIC:** terrain=nearest-edge, entities=center, flora=residency today.
   The veil must pick one for the *visual* band (recommend nearest-point-of-entity
   or center, applied identically to terrain + entities). Residency stays for
   *existence* (a separate, wider ring), not visibility.
3. **THE CHAIN NUMBERS:** is R_veil the LOD0 band (175), the draw cylinder (275),
   or a new value? Today reveal (250) sits between them and is *inverted* vs the
   band. Fix V1 (existence 360/400 ≤ residency 350) and V2 (reveal ≤ band) as part
   of the chain, or declare the LOD1 fill (175→275) as the veil's soft edge.
4. **THE MECHANISM:** fog-wall in `shade_lit` (recommended, least machinery, reuses
   the GoL smoothstep shape) vs dither/dissolve. Fade-to-transparent is rejected
   (order-sensitive on opaque geometry).
5. **THE FORKS (Discipline 2):** orbs (sky wall), fade (transition), ribbon (flown,
   exempt though it shares ENTITY_FS), gallery/indoor (wall-bounded) do NOT join
   the point-band veil. Confirm the exemption list.
6. **THE FOSSILS:** the veil cut retires the 6 dead constants (§5) and renames the
   `lod_pawn` fossil to a point name; the 4 live-redundant groups collapse to the
   one chain.

---

## §7 DISCIPLINE

Read-only. No radius, gate, anchor, or shader touched. `git status` clean but for
this file. Full stop for the design ruling before any veil cut.
