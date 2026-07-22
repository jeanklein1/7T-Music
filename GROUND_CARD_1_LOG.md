# GROUND_CARD_1 — CAMPAIGN LOG

Campaign: GROUND_CARD_1 (ground_card_campaign_v2.md Stages 1–4, per
src/docs/HANDOFFS/ h0–h6, with H0's two approved amendments folded).
Branch: `GROUND_CARD_1`.

---

## H0 — INDEX + PREFLIGHT

### Base

- CLOSURE_GPU HEAD at cut time: `bd405d927a3de8ae47da9b719ad441a31e5c326e`
  (matches H0's expected base exactly).
- GROUND_CARD_1 cut from `c7f4ef4bedad906ff37f40560571836f8c80fe9b`
  (current master tip), a **verified descendant** of bd405d92 per H0's
  "or a verified descendant" clause. Chosen because the audit instruments
  (glaw1, cc4/cc6/cc7, probes) and the campaign doc live only in the
  descendant commits; H6 requires committing recount outputs "beside the
  originals", which is only possible on this base.
- Drift report, bd405d92 → c7f4ef4 (per-file, exhaustive classes):
  - `AUDIT_REPORT.md` + `audit/**` ADDED (instruments + campaign doc).
  - `src/cartridges/backup_board/**` DELETED (moved to Jean's local
    backup; the H6 note about backup_board BOMs is therefore moot).
  - `src/docs/**` doc adds/moves (HANDOFFS, campaign doc, `old docs/`).
  - `src/cartridges/the_board/**`: **ZERO changes** — the code tree the
    anchors bind to is byte-identical to CLOSURE_GPU HEAD.

### Anchor table (a–i)

| # | File | Anchor | Expect | Found | Verdict |
|---|------|--------|--------|-------|---------|
| a | realization/binding_registry.hpp | `pyramid_instances          = 30` | 1 | 1 | PASS |
| b | realization/binding_registry.hpp | `pawn_aura_read             = 33` | 1 | 1 | PASS |
| c | realization/state.hpp | `Patch Heightfield Array (225x256x256` | 1 | 1 | PASS |
| d | realization/renderer.hpp | `DispatchWorkgroups(4, 1, 1)` | 1 @ frustum-cull site | 1, at renderer.hpp:518, directly under `SetBindGroup(0, frustumCullBindGroup)` and the `// ceil(MAX_ACTIVE_PATCHES / 64) = ceil(225/64) = 4` comment; no other occurrences in the file | PASS |
| e | realization/world.wgsl | `Only flying ribbons now; no terrain-following needed.` | 1 | 1 | PASS |
| f | realization/world.wgsl | `// §7.4 PAWN AURA` | 1 | 1 | PASS |
| g | realization/world.wgsl | `fn query_ground_walker(` | 1 | 1 | PASS |
| h | realization/world.wgsl | `fn compute_entity_placement(` | 1 | 1 | PASS |
| i | realization/state.hpp | nine `desc.label` strings (Compute Entity / Compute Texture / Entity Placement Compute / Photographer Compute / Frustum Cull Compute / Ribbon Compute / Render Texture / Shadow Texture / Pawn Aura Compute Layout) | 1 each | 1 each (desc.label form; no stray duplicates anywhere in state.hpp) | PASS |

All anchors PASS → proceeding to H1.

### Baseline gate

- glaw1 (audit/tools/glaw1/run.sh) at base c7f4ef4, before any edit:
  `G-LAW 1: GREEN` (stub webgpu: 73 types, 64 statics, 117 members).

### NOTE for Jean (copied per H0 item 4)

The 5-minute Windows witness run (audit/probe_dawn_witness.mjs in Chrome
on the design machine, plus the wgslLanguageFeatures query) is the
campaign's remaining precondition; it can run any time before the
batch-end build.

### Amendments in force (H0, approved)

1. FUTURE-LIVE entries RETAINED, not removed-then-readded: Compute
   Entity 145 photo_heightfield / 146 photo_sampler / 152 patch_grid,
   Compute Texture 23 nearest_sampler. Comments corrected to truth in
   H1; they go live at H5.
2. Card G/B = WAVES-ONLY gradient this batch (exact parity with current
   VS normals). Pulse-shaded normals are Stage 6 work.

---

## H1 — STAGE 1a: THE DEAD-ENTRY SWEEP

All removals paired (layout + group), array sizes decremented, indices
renumbered contiguous 0..N-1 (machine-verified per block). All edits in
realization/state.hpp unless noted. Audit cross-check before editing:
cc4_output.json confirms compute_entity_placement uses neither 60 nor
144; frustum_cull_patches uses neither 60(fc_agents) nor 80(fc_camera);
compute_photographer_vp uses 60/80 but NOT 144.

### Commit [1a] — 6490b47

1. CLASS B, Compute Entity Layout + BindGroup (desc.label anchors):
   REMOVED entry pair bind::g0::trajectories (101). Arrays 18→17.
   BEFORE (layout): `entries[6].binding = bind::g0::trajectories;` +
   visibility Compute + buffer.type Storage. AFTER: block deleted,
   [7..17]→[6..16]. Group side identical shape (buffer =
   trajectoriesBuffer_). Group header comment claimed "19 entries"
   while the array held 18 — pre-existing drift; corrected to 17.
2. CLASS B, retained future-live comment truth (H0 amendment 1): the
   claim comment over 145/146/152 in BOTH the layout and the group
   ("Required by compute pipelines that do per-frame baked-path Y
   lookups: update_camera, update_agents…") REPLACED with:
   "dead today (agents are analytic; sole baked readers are
   photographer + placement — audit cc4). Goes LIVE at Stage 4: the
   agents' baked ground path (GROUND_CARD_1 H5)."
3. CLASS B, Compute Texture Layout g1:23 nearest_sampler comment
   BEFORE: "nearest_sampler (matches render texture layout; retained
   for future compute consumers)" AFTER: "dead today; goes LIVE at
   Stage 4 — the live card's cell-exact GoL fetch (GROUND_CARD_1 H5)."

### Commit [1b] — a70375e

4. CLASS B, Entity Placement Compute Layout + BindGroup: REMOVED pairs
   agent_state (60) and photo_patch_instances (144). Arrays 13→11;
   [2]→[1], [4..12]→[2..10]. Header count comment folded to 11 with
   removal note.
5. CLASS B, Photographer Compute Layout + BindGroup: REMOVED pair
   photo_patch_instances (144). Arrays 10→9; [6..9]→[5..8] (the
   out-of-order entries[9] camera_state renumbered to [8] by index,
   position untouched). WGSL side: world.wgsl:8406 declaration
   `@group(0) @binding(144) var<storage, read> photo_patch_instances`
   REMOVED together with its TODO[seam-map:cleanup] block — this is
   exactly the coordinated edit that TODO called for; grep confirms
   zero remaining code references (only a historical prose mention at
   the patch_grid helper comment, retained).
6. CLASS B, Frustum Cull Compute Layout + BindGroup: REMOVED pairs
   fc_camera/camera_state (80) and fc_agents/agent_state (60). Arrays
   7→5; [3..5]→[2..4]. WGSL side: dedicated declarations
   `fc_camera` (world.wgsl:8760) and `fc_agents` (world.wgsl:8764)
   REMOVED — cc3/cc4 certify frustum_cull_patches never references
   them; grep confirms zero remaining references.

### Commit [1c] — 1c86e13

7. CLASS B (probe_a-modeled), Ribbon Compute Layout + BindGroup:
   REMOVED pairs tile_grid (25) and pier_instances (26). Arrays 5→3;
   [2..4]→[0..2]. Matches audit/probe_a.patch ribbon hunks; probe_a's
   REFUTED GoL-pier and aura-tile hunks NOT applied.
8. CLASS B, ribbon header comment: enumeration "tile_grid @25,
   pier_instances @26, ribbon_state @120, ring_xforms @121, head_poses
   @122" trimmed to "ribbon_state @120, ring_xforms @121, head_poses
   @122". Group header count 5→3.
9. WGSL: NO edit (per handoff — compute_ribbon_rings uses only shared
   g0 declarations still carried by other layouts).

### Gate

glaw1 after [1c]: `G-LAW 1: GREEN`.

Post-H1 shape (informational; formal recount at H6): Compute Entity 17
entries (8 storage incl. 3 future-live dead / 7 uniform + tex +
sampler); Placement 11; Photographer 9; Cull 5; Ribbon 3.

---

## H2 — STAGE 1b: THE FRUSTUM FIX + THE TEXT FOLD

### Commit [2a] — a323dc2 (CLASS A; the live bug, audit CC-8a)

FIND (renderer.hpp:517–518, adjacent exactly as specified):
    // ceil(MAX_ACTIVE_PATCHES / 64) = ceil(225/64) = 4
    pass.DispatchWorkgroups(4, 1, 1);
REPLACED with the derived dispatch:
    pass.DispatchWorkgroups((Dim::MAX_ACTIVE_PATCHES + 63u) / 64u, 1, 1);
`Dim::` is visible at this site (renderer.hpp already uses Dim:: at 14
sites; state.hpp defines Dim::MAX_ACTIVE_PATCHES = 17*17 = 289).
DISCLOSURE (campaign v2 §7.7): at a full 289-slot window this may
visibly restore patches that were previously never culled/emitted —
correctness delta. New workgroup count: 5 (320 threads ≥ 289 slots;
kernel's own gid bound check covers the tail).

### Commit [2b] — 5f180fb (text fold)

2. CLASS A, state.hpp:3538: "Patch Heightfield Array (225x256x256,
   RGBA16Float)" → "(289x256x256, RGBA16Float; 289 =
   Dim::MAX_ACTIVE_PATCHES)".
3. CLASS A, state.hpp:3557: "Patch Cell Color Array (225x16x16,
   RGBA8Unorm)" → "(289x16x16, RGBA8Unorm; 289 =
   Dim::MAX_ACTIVE_PATCHES)".
4. CLASS B, binding_registry.hpp banner: "The 108 WGSL @binding
   literals in world.wgsl stay a MIRROR" → "The WGSL @binding literals
   in world.wgsl (102 declarations over 97 slots — audit cc7; five
   documented fc_ aliases share slots) stay a MIRROR"; rest of the
   sentence intact. NOTE (timing): the parenthetical cites the cc7
   audit taken at CLOSURE_GPU HEAD; H1 removed 3 declarations and
   H3/H5 add 2 — the H6 cc7 recount (_post_gc1) records the batch-end
   truth beside this citation.
5. CLASS B, state.hpp GPURibbonRingTransform.terrain_y.
   BEFORE (pasted): `float terrain_y;           // tile-modified terrain height ( 4) = 48`
   AFTER: "( 4) = 48 — always 0.0: flying ribbons (no terrain follow).
   Field retained for the 48-byte stride; removal is a cleanup-campaign
   item (VS input layout stride)."
6. CLASS B, state.hpp struct banner.
   BEFORE (pasted): `// Pre-computed per-ring transform (compute pass output, VS + update_world input)`
   AFTER: "(compute_ribbon_rings output; ribbon_vs + shadow_ribbon_vs
   input via render_ring_xforms)". JUDGMENT NOTE: update_world's split
   successors (the five agent kernels) do NOT consume this struct —
   cc4 shows its only compute toucher is compute_ribbon_rings (writer)
   and its readers are the two ribbon VS entry points via
   render_ring_xforms@361; the banner now names the real ones.
7. CLASS B, world.wgsl:889 RibbonRingTransform mirror.
   BEFORE (pasted): `terrain_y: f32,         // tile-modified terrain height at center XZ`
   AFTER: same truth as edit 5, WGSL phrasing.
8. NOT touched (per handoff): Pawn Aura tile_grid entry + its
   "archetype lookup" comment (CC-8d LIVE, refuted seed); g0:29 /
   g1:30 tombstones.

### Gate

glaw1 after [2b]: `G-LAW 1: GREEN`.

---

## H3 — STAGE 2: THE LIVE CARD

### Commit [3a] — 4f38cf7

1. CLASS B, Dim:: cluster (anchor: MAX_ACTIVE_PATCHES line): appended
   the LIVE CARD block. ADAPTATION (house style): plain `constexpr`
   like every sibling in Dim (spec wrote `inline constexpr`). Only the
   REAL witness committed (512*25 == 800*16 ⇔ texel = 1.5625 wu); the
   handoff's placeholder assert dropped as instructed.
2. CLASS A, binding_registry.hpp g0: live_card_write = 31 appended
   directly under pyramid_instances = 30.
3. CLASS A, binding_registry.hpp g1: live_card_read = 34 appended
   directly under pawn_aura_read = 33.
4. CLASS B, texture: pawnAuraTexture_ pattern cloned in full — members
   (liveCardTexture_/liveCardWriteView_/liveCardView_), createTextures
   block (512x512 RGBA16Float, TextureBinding|StorageBinding, label
   "Live Card (512x512, RGBA16Float — GROUND_CARD_1)"), accessor
   live_card_view().

### Commit [3b] — 26f5b55

5. CLASS B, world.wgsl beside PATCH_CELL_N/PATCH_EXTENT: mirror pair
   LIVE_CARD_SIZE/LIVE_CARD_EXTENT + live_card_origin() (3.125 cell
   snap of the point-centered window).
6. CLASS B, declarations: g1:34 live_card_read beside pawn_aura_read
   (g1:33); g0:31 live_card_write beside pawn_aura_tex_write (g0:172).
   Slot check: g0:31 and g1:34 both free pre-edit (the only @binding(31)
   was GROUP 1's zone_life_read — different group, no collision).
7. CLASS B, §7.3b block inserted immediately before "// §7.4 PAWN
   AURA": live_card_uv, sample_live_card (bilinear), sample_live_card_gol
   (nearest, cell-exact), write_live_card @8x8.
   SIGNATURES VERIFIED (logged per handoff):
   - terrain_wave_overlay_with_gradient(world_xz: vec2<f32>) -> vec3<f32>
     {h,gx,gz} — matches spec.
   - contrib_gol_zones_at(world_xz: vec2<f32>) -> f32 — matches spec.
   - contrib_radial_pulses_at(world_xz: vec2<f32>, t_seconds: f32) -> f32
     — DIFFERS from spec (extra t_seconds param). Call site adapted
     mechanically: contrib_radial_pulses_at(p, signal.t_seconds), per
     the contributor's own doc ("compute stages (signal.t_seconds)").
   Transitive binding needs of the writer verified: config (waves,
   pulses, origin, band blends — get_band_blend reads config.band_blend_*),
   signal (t_seconds), zone_config/zone_life (GoL). All in the H3 layout.

### Commit [3c] — 26d38dd

8. CLASS B, Live Card Writer Layout + BindGroup appended after the
   Pawn Aura blocks; 5 entries (0 signal U, 1 config U, 160 zone_config,
   161 zone_life, 31 storage-tex write); zone-pair home note in the
   block comment. DEVIATION (logged): spec said ReadOnlyStorage for the
   zone pair; the WGSL declarations are var<storage, read_write>
   (world.wgsl:5711–5712) and ReadOnlyStorage against read_write fails
   createComputePipeline validation — used Storage, exactly as the
   Compute Entity layout documents for the same pair.
9. CLASS B, renderer.hpp: aura pipeline pattern cloned —
   Entry::WRITE_LIVE_CARD, liveCardWriterLayout_/Pipeline_ members,
   layout pulled via live_card_writer_layout(), creation block (not
   roster-gated — always on), dispatch_live_card_write(pass, group)
   dispatching (LIVE_CARD_SIZE/8)^2 x1 = 64x64x1.
10. CLASS B, render_passes.hpp: own-pass free function
    dispatch_live_card_write(MachineCtx*, encoder), label "Live Card
    Write". JUDGMENT NOTE: spec said clone "the dispatch_pawn_aura
    free-function pattern" — no such free function exists (aura is
    dispatched from bodies/pawn.hpp); cloned dispatch_placement_correction,
    the house own-pass shape.
11. CLASS B, cartridge.hpp spine: LiveCardWrite inserted in RPhase
    after UploadPortalLights; table row mirrors the neighbor shape
    (pasted neighbors: UploadPortalLights row Driver::Algo/F_CONFIG,
    DispatchCompute row Driver::Mixed/F_COMPUTE; the card row uses
    Driver::Mixed + F_COMPUTE — it is a music-driven compute dispatch);
    phase_live_card_write calls the free function; the two order
    static_asserts appended beside the existing spine asserts (house
    style IS assert-on-enum-order — verbatim messages from the spec).
    Spine density assert (RENDER_SPINE size == RPhase::COUNT) holds —
    glaw1 compiles it.
12. Debug eye: LIVE_CARD_DEBUG_VIEW = 0u const landed beside the new
    consts (in [3c] per the handoff's item numbering); the terrain-FS
    branch lands in H4 [4a] per H3's recorded decision.

### Gate

glaw1 after [3c]: `G-LAW 1: GREEN`.
Rest-safety note: at rest terrain_time <= 0 zeroes the wave overlay,
pulse_count = 0 zeroes pulses, zone count 0 zeroes GoL — the card
writes zeros; nothing reads it yet (except the eye, off by default).
