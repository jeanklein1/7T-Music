# GROUND_CARD_1 — BATCH REPORT

Campaign: GROUND_CARD_1 (ground_card_campaign_v2.md Stages 1–4, per
src/docs/HANDOFFS/ h0–h6). This is the batch's witness stand (H6).

## Base + final

- CLOSURE_GPU HEAD (H0 expected base): `bd405d927a3de8ae47da9b719ad441a31e5c326e`
- Branch base (verified descendant; carries the audit instruments):
  `c7f4ef4bedad906ff37f40560571836f8c80fe9b` —
  `src/cartridges/the_board/**` byte-identical to bd405d92 at cut time.
- Commit list (one-line intents):

| Commit | Hash | Intent |
|--------|------|--------|
| [H0]   | 38b2e7e | Preflight: anchor table a–i all PASS; baseline glaw1 GREEN |
| [1a]   | 6490b47 | CE drops trajectories(101); future-live comment truth (145/146/152, g1:23) |
| [1b]   | a70375e | Placement drops 60+144; Photographer drops 144; Cull drops fc_agents/fc_camera; dead WGSL declarations removed |
| [1c]   | 1c86e13 | Ribbon drops tile_grid(25)+pier_instances(26) (probe_a shapes; refuted hunks NOT applied) |
| [2a]   | a323dc2 | Frustum-cull dispatch derived from Dim::MAX_ACTIVE_PATCHES (live bug CC-8a) |
| [2b]   | 5f180fb | Text fold: 225→289 labels, registry banner census, ribbon terrain_y truth |
| [3a]   | 4f38cf7 | Dim::LIVE_CARD_* + witness assert + registry 31/34 + the card texture |
| [3b]   | 26f5b55 | WGSL: mirror consts, origin snap, declarations, reader helpers, write_live_card |
| [3c]   | 26d38dd | Writer layout/group/pipeline/dispatch + spine phase RLiveCardWrite + order asserts |
| [4a]   | f7e689d | live_card_read(34) into Render+Shadow texture groups; FS debug eye |
| [4b]   | 3269bb8 | Patch VS pair rides the card (height+gradients; shadow height) |
| [4c]   | ac73c2c | 11 entity/wall VS sites → sample_live_card().x; gallery group gains 22+34 |
| [5a]   | 84e429e | CT group gains the card; placement gains group 1; comments flip LIVE |
| [5b]   | e393bed | Query family speaks card; MACHINE GATE PASS |
| [5c]   | e2866c3 | Evictions: CE −{25,26,30,160,161}, Placement −{160,161} |

(Per-handoff log commits 3e5512c / bbec4a0 / 8249f51 / c4c0cc9 / d4bf6de
carry GROUND_CARD_1_LOG.md, the per-edit record.)

## CC-6-post vs campaign v2 §5 expectations

Full table: `audit/cc6_output_post_gc1.json`. Compute-stage
storage/uniform counts, pre → post, with the campaign's expected shape:

| Layout | Entries pre→post | Storage pre→post | Uniform pre→post | Expected (v2 §5) | Verdict |
|--------|-----------------|------------------|------------------|------------------|---------|
| Compute Entity | 18→12 | 9→**5** | 7→**5** | 5 storage {vp, agents, camera, floaters, patch_grid} / 5 uniform {signal, config, portals, behaviors, tier_gains} + hf tex + sampler | **MATCH** |
| Entity Placement | 13→9 | 9→**5** | 1→1 | 5 storage + card via g1 | **MATCH** |
| Compute Texture | 3→4 | — | — | 2 sampled (aura, card) / 2 samplers | **MATCH** |
| Frustum Cull | 7→5 | 6→**4** | 1→1 | 4 | **MATCH** |
| Ribbon | 5→3 | 3→**2** | 2→**1** | 2 / 1 | **MATCH** |
| Photographer | 10→9 | 6→**5** | 2→2 | 5 | **MATCH** |
| Render Entity | 18→18 | unchanged | unchanged | unchanged | **MATCH** |
| Live Card Writer | —→5 | 2 (zone pair) | 2 (signal, config) | new (H3) | **MATCH** |
| Render Texture | 10→11 (+card) / Shadow Texture 3→4 (+card) / Gallery Texture 3→5 (+22,+34 — the wall-painting adaptation, logged) | | | | as built |

cc6 `flags`: **EMPTY** — no layout exceeds either law (8s/12u WebGPU
default or 10s/12u banner FXC law) on any stage.

cc7-post (`audit/cc7_output_post_gc1.json` + mirror
`audit/cc7_mirror_output_post_gc1.json`): 101 declarations (102 − 3
removed in H1 + 2 added in H3); g0:31 = live_card_write and g1:34 =
live_card_read occupied by exactly the registry names (mirror clean);
zero WGSL-literal-without-registry orphans. fc_ aliases: fc_config(1),
fc_vp(2), fc_patches(340) remain; fc_camera(80)/fc_agents(60) removed in
H1 — the H6 "as before" expectation predates that removal, superseded by
H1's certified sweep. One registry-without-WGSL residue:
`photo_patch_instances = 144` (the constant outlives its declaration —
H1 removed the WGSL side per spec; registry constant retained, cleanup
candidate for AUDIT-2).

## The [5b] machine-gate binding sets (verbatim)

    update_player_agent:  [0, 1, 22, 23, 33, 34, 60, 62, 80, 145, 146, 152]
    update_other_agents:  [0, 1, 22, 23, 33, 34, 60, 80, 110, 111, 145, 146, 152]
    update_camera:        [0, 1, 22, 23, 33, 34, 60, 80, 145, 146, 152]
    update_sphere:        [0, 1, 22, 23, 33, 34, 60, 80, 100, 145, 146, 152]
    update_cube:          [0, 1, 22, 23, 33, 34, 60, 80, 100, 145, 146, 152]
    compute_entity_placement: [1, 23, 34, 143, 145, 146, 147, 148, 150, 151, 152]

None of the five references {25, 26, 30, 160, 161}; placement references
neither zone binding. REQUIRED RESULT met. Additionally, every compute
entry point's reference set was verified covered by its pipeline's
layout(s) — including compute_vp (CE only), photographer (no g1 leak),
and the zone family (analytic fallback intact within its own layout).

## Residue censuses (verbatim)

H4 item 7 — contrib_terrain_waves_at after [4c]:

    2719: fn contrib_terrain_waves_at(world_xz: vec2<f32>) -> f32 {
    2986:     h += contrib_terrain_waves_at(xz);              (manifold_overlay_stack — H5 removed)
    3095:     let waves    = contrib_terrain_waves_at(xz);    (walker pair — H5 removed)
    8128:     let wave_y = contrib_terrain_waves_at(pos.xz);  (zone_extrusion_vs — SKIP-DOOMED)
    8237:     let wave_y = contrib_terrain_waves_at(pos.xz);  (shadow_zone_extrusion_vs — SKIP-DOOMED)

H5 item 8 — after [5b] (code sites; definitions excluded):

    contrib_static_base_at:   2961 (query_ground_baked_heightfield — zone fallback chain)
    contrib_pyramids_at:      2666 (ground_formed bake chain), 2962 (zone fallback chain)
    contrib_gol_zones_at:     2380 (contrib_gol_suppression_at — LATENT, zero callers), 8283 (write_live_card)
    contrib_terrain_waves_at: 8132, 8241 (the two SKIP-DOOMED zone sites)
    contrib_radial_pulses_at: 8282 (write_live_card)
    terrain_wave_overlay_with_gradient: 8281 (write_live_card)

= exactly the sanctioned set {bake chain, zone-mesh/fallback sites,
write_live_card}, plus one LATENT zero-caller reference form (noted).

## Dawn witness (H6 item 1d)

`audit/probe_dawn_witness_post_gc1.mjs` (baseline-only variant of the
original — families updated to the batch renderer: placement =
[Placement, Compute Texture]; new live_card family; eviction probes moot
post-eviction). Real Dawn via headless Chromium (Vulkan/SwiftShader):

- world.wgsl module: compiles with **zero** compilation messages (149.8 ms).
- **ALL PIPELINE FAMILIES GREEN** — every compute pipeline in every
  family (live_contrib ×5, compute_vp, patch_gen ×3, ribbon,
  photographer, entity_placement, frustum_cull, pawn_aura,
  **write_live_card**, orb ×3, orb_copy, gol_zone ×5, five mesh-gen
  families, terrain_index_gen) validates against the batch tree's
  re-parsed layouts. Full output: `audit/probe_results_post_gc1.json`.

## Deviations log (every CLASS-B judgment call / adaptation)

1. **Branch base**: cut from c7f4ef4 (verified descendant clause) rather
   than bd405d92 — the audit instruments and campaign doc exist only in
   the descendant commits; the_board tree byte-identical. Drift report in
   GROUND_CARD_1_LOG.md H0.
2. **[1a]** CE group header claimed "19 entries" while holding 18 —
   pre-existing drift, corrected to 17 (now 12 post-[5c]).
3. **[2b]** Registry banner: the specified cc7 citation text (102/97)
   is the pre-batch audit's truth; H6 recount (101 decls) recorded here
   and in the _post_gc1 outputs.
4. **[2b]** `update_world` replacement: its split successors do not
   consume RibbonRingTransform — banner now names the real pair
   (compute_ribbon_rings out; ribbon_vs + shadow_ribbon_vs in via
   render_ring_xforms@361), per cc4.
5. **[3a]** Dim consts use the cluster's plain-`constexpr` house style
   (spec wrote `inline constexpr`); the placeholder assert was not
   committed per the handoff's own instruction.
6. **[3b]** `contrib_radial_pulses_at` takes (vec2, t_seconds) — call
   adapted to pass `signal.t_seconds` per the contributor's own
   compute-stage doc.
7. **[3c]** Zone pair in the writer layout is **Storage**, not the
   spec'd ReadOnlyStorage — the WGSL declarations are
   `var<storage, read_write>` (world.wgsl:5711–5712); ReadOnlyStorage
   fails createComputePipeline validation. Same adaptation the CE
   layout already documented for the same pair.
8. **[3c]** No `dispatch_pawn_aura` free function exists (aura
   dispatches from bodies/pawn.hpp) — cloned
   `dispatch_placement_correction`, the house own-pass shape.
9. **[3c]** Spine row Driver::Mixed + F_COMPUTE (mirrors DispatchCompute,
   the closest compute-dispatch neighbor).
10. **[4a]** Debug eye placed AFTER the rim discard (eye respects the
    veil ring); `world_pos.xz` spelled `in.world_pos.xz` per the block.
11. **[4c]** wall_painting_vs rides the gallery pipeline layout —
    Gallery Texture Layout + Group grew bilinear_sampler(22, Vertex) +
    live_card_read(34, Vertex), 3→5 (the handoff's shadow-sampler
    contingency shape applied where the need actually was).
12. **[4c]** The handoff's site-location guesses (arch/column mains at
    ~10470) differed from the tree (4700s cluster) — the partition RULE
    matched exactly 11 entity/wall VS sites; count in the expected band.
13. **[5b]** Pair query is ONE point with two outputs (not the spec'd
    "two points") — transformed to one baked + one card + one gol fetch
    shared by both outputs, mirroring its actual shape.
14. **[5b]** The LATENT placement query bodies + the switch's baked and
    default arms were ALSO rewired to the baked path — they are
    statically reachable from the agent kernels via the policy switch,
    and leaving them analytic fails the machine gate. The declared
    no-pyramids intents are preserved in comments (zero live callers).
    `query_ground_baked_heightfield`'s analytic body STAYS for the zone
    baked-sampler fallback chain; the switch's case 3u rides the texture
    form (byte-consistent by construction).
15. **[H6]** Witness harness: original hardcodes CLOSURE_GPU families
    and reads the pre-batch cc6 output; a baseline-only _post_gc1
    variant was added beside it (original untouched).
16. **ABORTs**: none — every anchor matched exactly once or was
    adapted under CLASS B with the before/after logged.

## Encoding sweep

LF-only, no BOM across all touched files (cartridge.hpp, state.hpp,
renderer.hpp, render_passes.hpp, binding_registry.hpp, world.wgsl, the
log, the report, all _post_gc1 outputs) — machine-checked. The
world.wgsl banner FXC block (lines ~44–56) is byte-untouched.
backup_board/ no longer exists in-tree (moved to Jean's local backup
pre-campaign) — its BOM note is moot.

## glaw1

GREEN at base, after every handoff, and at batch end.

─────────────────────────────────────────────────────────────────
JEAN'S GATE LIST (Windows, one session):
[ ] Witness harness (if not yet run): audit/probe_dawn_witness.mjs
    in Chrome + log wgslLanguageFeatures (decides paint-card v2
    residency later; nothing in this batch depends on it).
[ ] Build (the real glaw1 + MSVC/Dawn) on GROUND_CARD_1.
[ ] Boot. FXC watch: the only NEW kernel is write_live_card (the
    proven standalone-writer shape); the rewired query family has
    FEWER loops than before, not more.
[ ] Idle rig: world at rest must be pixel-identical to CLOSURE_GPU
    (card = zeros by construction). Capture stills A/B.
[ ] Debug eye: flip LIVE_CARD_DEBUG_VIEW = 1u → black at rest;
    paints |Δh| (R) and GoL lift (G) under music / near zones.
[ ] Motion motif: waves/pulses via the card (bilinear 1.5625 wu vs
    analytic — motif-preserving by ruling, not bit-locked).
[ ] Walkabout: pawn step/slide on slopes, arch-pier footing, stairs
    of a fresh pier during the NEEDS_REGEN window (transient,
    disclosed), camera clamp, agent snap, cube kite/hover ground,
    sphere clearance, pawn bow-wave on GoL (unchanged this batch —
    zones still render as extrusion until Stage 5).
[ ] Frustum fix check: full-window scene — patches beyond slot 256
    now culled/emitted correctly (may visibly restore patches).
─────────────────────────────────────────────────────────────────

## Next session seeds

AUDIT-2 runs the same instrument set against GROUND_CARD_1 post-merge
(cleanup candidates already spotted: the dangling photo_patch_instances
registry constant; trajectoriesBuffer_ creation+upload now binding-free).
Then THE UNIFIED GROUND (campaign v2 Stage 5) designs on the verified
tree — its retirement list (zone-mesh stack, the GoL pier entry
pre-certified by probe A's life-kernel result, zone_heightfield +
sampler + zone_patch_instances) and the topology work (cap tiles +
cell curtains, index-gen + VS decode lockstep) are already specified
at v2 §6/S5. Stages 6–7 follow per the stamped arc.
