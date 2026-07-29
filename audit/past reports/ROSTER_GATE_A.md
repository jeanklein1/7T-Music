# ROSTER-1b — PHASE R' — GATE-(a) CLASSIFICATION (read-only)

the_board only. Branch **MOD_1_ROSTER**, on top of Phase I (`3774c56`) +
Commit Zero (`d787616`). Extends ROSTER_RECON R2 (the bind-group census)
with the decisive **creation-site** read. Method: verify-first, the tree
wins over the planning hypothesis; disclosed-judgment-and-continue for
this phase (per the go-order). This table is the requirements face's
first **cost column** (M-j material) — it outlives this arc.

Three gate kinds (from ROSTER-1a): **(a)** buffer creation (THIS arc),
**(b)** dispatch/registration (Phase I), **(c)** boot/teardown (Phase I).

---

## THE TEST (what "SEPARABLE" actually requires)

R2 gave bind-group *co-residency*. Gate (a) needs one more fact per
piece: **can its creation be skipped without (1) nulling a shared bind
group at boot, or (2) crashing an un-gated draw?** Two decisive reads
settle it:

1. **Exclusive-buffer-in-a-megabind.** A buffer a piece owns *exclusively*
   but that is *bound into* a shared group (Compute Entity / Entity
   Placement / Render Entity / Photographer / Render Texture / Compute
   Texture / Shadow Texture) cannot be un-created — the group's creation
   would see a null binding and boot validation fails. A buffer that is
   only **co-owned** (shared instance store: `floatingEntityBuffer_`,
   `plantComputeGroundBuffer_`) stays created for its co-owners and the
   disabled piece's slots simply stay pristine (Rider A).
2. **Draw self-gating.** Skipping a piece's pipeline/VB is only safe if
   its draw path early-returns when the piece is absent. `draw_shell` /
   `draw_shadow_shell` do (`if (indexCount==0) return;`). The family
   forward draws (`draw_sphere`, `draw_palm`, …) do **not** — they issue
   `SetPipeline` unconditionally, then an instanced or dynamic-count
   `DrawIndexed` (shader-side per-instance cull). So skipping their
   creation needs a coupled draw-gate first.

Applying both tests **overturns two of R2's three separable hypotheses**:
pawn_aura and orbs are SHARED (a texture / a state buffer is megabind-
resident), and the ground-placed families are SHARED (exclusive ground
buffers). The clean set is one piece — `indoor_shell` — plus a distinct
"draw-coupled" tier the co-owned families fall into.

---

## THE PER-PIECE COST TABLE (gate-(a) status)

Legend — **SEP** = separable now (skip all creation, boot+draws safe,
zero render change); **SH·mb** = shared, megabind-resident buffer/texture
(created-pristine; retirement = **re-section** the group); **SH·dc** =
dedicated resources but draw-coupled (created-pristine; retirement = add a
behavior-identical `if(indexCount==0) return` self-gate to the forward
draw, then skip — a render-side coupling, **no re-section**); **NO-RES**
= no gateable creation of its own.

| # | piece | status | exclusive creations (skippable / blocked) | megabind blocker | retirement |
|---|---|---|---|---|---|
| — | **indoor_shell** | **SEP** | `shellVertexBuffer_`,`shellIndexBuffer_`,`shellPipeline_`,`shadowShellPipeline_` — ALL skippable | none | **skip now** (draws self-gate on `shell_index_count==0`) |
| 0 | pyramid | SH·mb | mesh VB/IB/params, mesh-gen group, `pyramid`/`shadowPyramid`/`pyramidMeshGen` pipelines skippable; **blocked** by instance+ground | `pyramidInstancesBuffer_` (Compute Entity), `pyramidGroundBuffer_` (Entity Placement) | re-section Compute Entity + Entity Placement |
| 1 | arch | SH·mb | mesh VB/IB/params + 3 pipelines skippable; **blocked** by ground | `archGroundBuffer_` (Entity Placement) [pier co-owned] | re-section Entity Placement |
| 2 | column | SH·mb | mesh VB/IB/params + 3 pipelines skippable; **blocked** by ground | `columnGroundBuffer_` (Entity Placement) | re-section Entity Placement |
| 3 | antenna | **NO-RES** | rides column mesh (`prepare_mesh_column`/`mesh_gen_column`) — owns nothing | — | none |
| 4 | palm | **SH·dc** | `palmVertexBuffer_`,`palmIndexBuffer_`,`palmMeshParamsBuffer_`, mesh-gen group, `palm`/`shadowPalm`/`palmMeshGen` pipelines | none exclusive — `plantComputeGroundBuffer_` **co-owned** (palm+cactus+blade) | draw-gate `draw_palm`, then skip |
| 5 | cactus | **SH·dc** | `cactus*` VB/IB/params + group + 3 pipelines | co-owned plant ground | draw-gate `draw_cactus`, then skip |
| 6 | blade | **SH·dc** | `blade*` VB/IB/params + group + 3 pipelines | co-owned plant ground | draw-gate `draw_blade`, then skip |
| 7 | sphere | **SH·dc** | `sphereVertexBuffer_`,`sphereIndexBuffer_`,`sphere`/`shadowSphere`/`updateSphere` pipelines | none exclusive — `floatingEntityBuffer_` **co-owned** (sphere+cube) | draw-gate `draw_sphere`, then skip |
| 8 | ribbon | SH·mb | `ribbonRing` pipeline + readback staging skippable; **blocked** by state | `ribbonBuffer_`,`ringTransformsBuffer_` (Render Entity, Photographer) | re-section Render Entity + Photographer |
| 9 | cube | **SH·dc** | `monolithVertexBuffer_`,`monolithIndexBuffer_`,`monolith`/`shadowMonolith`/`updateCube` pipelines | none exclusive — `floatingEntityBuffer_` co-owned | draw-gate `draw_monolith`, then skip |
| 10 | gol | SH·mb | `zoneMesh{Vertex,Index,Indirect}Buffer_`,`zoneDeriveRequestBuffer_`,`zoneLifeTexture_`, GoL Zone group, 5 gol pipelines + `zoneExtrusion` skippable; **blocked** by config+life | `zoneConfigBuffer_`,`zoneLifeBuffer_` (Compute Entity, Entity Placement) | re-section Compute Entity + Entity Placement |
| 11 | gallery | SH·mb | gallery/wall-painting/photographer pipelines + offscreen textures + gallery groups skippable; **blocked** by painting slots | `paintingSlotsBuffer_` (Compute Entity, Entity Placement) | re-section Compute Entity + Entity Placement |
| F | pawn_aura | SH·mb | `pawnAuraConfigBuffer_`,`pawnAuraCellsBuffer_`, Pawn Aura group, `pawnAura` pipeline skippable; **blocked** by texture | `pawnAuraTexture_`/`pawnAuraReadView_` (Render Texture, Compute Texture) | re-section Render Texture + Compute Texture |
| F | orbs | SH·mb | `orbStatePrevBuffer_`,`orbQuadVB_`,`orbQuadIB_`, Orb Compute+Copy groups, 5 orb pipelines skippable; **blocked** by state+config | `orbStateBuffer_`,`orbConfigBuffer_` (Render Entity, Photographer) | re-section Render Entity + Photographer |
| F | spot_lights | SH·mb | `spotVPStagingBuffer_`,`spotShadowMapTexture_` (atlas) partly dedicated; **blocked** by lights + atlas-in-group | `spotLightArrayBuffer_` (Render Entity, Photographer); atlas in Shadow Texture | re-section Render Entity + Photographer + Shadow Texture |
| F | portal | **NO-RES** | force-spawns into `entities_state_.arches[]` + `pierBuffer_` — rides ARCH storage (R3) | — | none |
| F | transitions | **NO-RES** | owns only `fadeOverlayPipeline_`; fade drawn every frame (alpha 0 idle) via a **shared** group → skipping needs a coupled (b) draw-gate | — | draw-gate the fade, then skip the pipeline (latent) |
| F | wanderers | **NO-RES** | rides foundational `agentStateBuffer_` slots 1+ (the pawn is slot 0) | — | none |

**Tally (19 pieces):** 1 SEP · 9 SH·mb · 5 SH·dc · 4 NO-RES.

### Creation-site map (for the gates + LATENT tags)

- `createBuffers()` state.hpp:2864 — foundational + the shared instance
  stores (agent/floater/pier/patch/portal/light/painting-slots…).
- `createMeshBuffers()`→per-family `create{Arch,Column,Palm,Cactus,Blade,
  Pyramid,Sphere,Monolith,Shell}Mesh()` state.hpp:2984–3384 — family mesh
  VB/IB/params. **Shell:3371–3384 (SEP).**
- `createGoLZoneBuffers()` state.hpp:3386–3509 — **misnamed**: creates gol
  zone buffers **and** pawn-aura buffers (3452) **and** orb buffers (3473)
  in one function. Each is a distinct `makeBuffer` block (atomically
  separable within the function) — but all three pieces are SH·mb by the
  megabind test, so this arc only tags them.
- `createTextures()` state.hpp:3512 — pawn-aura tex (3515), gol zone tex
  (in createGoLZoneBuffers), shadow map (sun, foundational, 3604), spot
  shadow atlas (3616).
- `createBindGroups()` state.hpp:3685 — 24 layouts + 25 groups; the
  megabinds (Compute Entity 4802, Render Entity 4899, Entity Placement
  5311, Photographer 5215/5253, Render/Compute Texture 4988/5010, Shadow
  Texture 4941) plus per-piece groups (Pawn Aura 5440, Orb 5465/5497, GoL
  Zone 5408, per-family Mesh Gen 5518–5639).
- renderer.hpp pipelines — 31 compute + 34 render; per-piece render +
  shadow pipelines are dedicated (droppable); layouts are shared.

---

## FOUNDATIONAL (non-gateable, out of the roster — for a complete census)

The sun (`directionalLightBuffer_` + `shadowMapTexture_` + shadow
pipelines; the sun rider comment now sits at `upload_lights`), the camera,
terrain substrate (`tileGridBuffer_`, patch buffers, patch heightfield /
cell-color / cell-fields textures, terrain-index gen), frustum cull, the
agent machinery (`agentStateBuffer_`, agent kernel, slot 0 = the pawn),
and the frame signal / config / VP. None carry a bit (ROSTER-1a
FOUNDATIONAL ruling). `pierBuffer_` and `portalArrayBuffer_` are
foundational shared stores that arch/column/portal write into.

---

## DISCLOSURE (the two hypotheses the census overturns, and the H-gate note)

- **pawn_aura and orbs are SHARED, not separable** (R2's hypothesis). The
  aura *texture* is sampled by the terrain FS (Render Texture + Compute
  Texture groups); the orb *state/config* buffers are read by the entity
  render + photographer passes. Their compute stacks are fully dedicated
  and could be skipped, but the one shared resource each pins them
  created-pristine until the group is re-sectioned.
- **The ground-placed families (pyramid/arch/column) are SHARED**;
  the **floating (sphere/cube) and plant (palm/cactus/blade) families are
  draw-coupled-separable** — their instance stores are co-owned, so only a
  draw self-gate stands between them and skippable.
- **H-gate reconciliation (for the rig):** H1 (orbs off) and H2 (pawn-aura
  off) land pieces **pristine** (empty sky / no aura — via Phase I's (b)
  gates) but with **no creations skipped** — they are SH·mb. H3 (gol off)
  likewise pristine, not un-created — so the residue recipe **stays the
  Phase-I pristine form** (no "never created" upgrade this arc; §3f's
  precondition "if gol classifies SEPARABLE" is not met). Only **H4's lean
  build** actually skips creations — because `indoor_shell` (SEP) is in the
  lean-off set. The boot summary (3g) therefore fires for the lean build
  and any SEP piece disabled, and is silent otherwise. If Jean wants
  orbs/pawn_aura/gol to *skip* creation too, that is the re-section /
  draw-gate work priced in the table above — a follow-on order, not this
  arc.

---

## PHASE I' PLAN (what lands next, per the go-order)

Roster graduates to `roster.hpp` (3a: second consumer = `GPUState::init`).
Then: **gate `indoor_shell` creation** (the one SEP piece) as an atomic
block (3b); **LATENT[gate-a-shared]** tags at every SH·mb / SH·dc creation
site with the retirement above (3c); NO-RES pieces recorded in the
manifest doc (3d); the manifest **gains the gate-(a) status column** (3e);
gol residue recipe **unchanged** (3f n/a); boot summary line when ≥1 piece
disabled, silent when all-enabled (3g). All-enabled stays byte-identical
(build + stdout) to `3774c56`.

---

## PHASE I' — LANDED

- **3a** `roster.hpp` created; `inline constexpr ROSTER` + struct + edge +
  full doc block (incl. the gate-(a) status column and NO-RES recording).
  `family_enabled` uses literal indices; the PopFamily binding is asserted
  in cartridge.hpp. Included by state.hpp (second consumer) + cartridge.hpp.
- **3b** `indoor_shell` (SEP) creation gated (`ROSTER-GATE indoor_shell (a)`
  ×3): shell VB/IB (`createShellMesh` early-return) + shell/shadow
  pipelines (both `tPipe` calls). Draws self-gate on `shell_index_count==0`,
  so zero render change.
- **3c** 14 `LATENT[gate-a-shared]` tags (9 SH·mb + 5 SH·dc) at their
  creation sites, each with its retirement. Also logged as a Constitution
  §5 class.
- **3d/3e** in the manifest doc block (roster.hpp), landed with 3a.
- **3f** n/a — gol is SH·mb; residue recipe stays the Phase-I pristine form.
- **3g** boot summary: `if constexpr (!ROSTER.all_enabled())`-discarded when
  all-enabled (byte-identical binary + stdout); prints the disabled pieces
  + creations-skipped line otherwise.

Invariants held: all-enabled byte-identical by construction (every gate is
`if constexpr`, discarded or transparent); per-piece atomicity
(indoor_shell off → zero shell buffers + pipelines, boot valid); zero
stray consults; Rider A for SH pieces; scope guard (no algorithm, no
world.wgsl). Rig gates H0–H4 confirm at the rig.
