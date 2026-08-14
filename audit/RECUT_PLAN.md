# THE RECUT PLAN (LOOM_2 U1)

Derived by `binding_gen.py --plan` from the authored stratum map and
the tree's reach closure. THE TOOL DERIVES; IT DOES NOT DECIDE —
every rule it applied is stated below, and everything the rules
could not place is in the STOP list. Nothing here touches the tree:
this document is the ratification instrument, and U2 may not begin
until Jean ratifies it.

## The rules as applied

- R1/R2 verbatim from the handoff (WORLD = config + tile_grid;
  FRAME = signal, both vp faces, both camera faces, render_lighting,
  shadow_slot with its window, the two shared samplers).
- R3a: home = the sole GPU-writing family of the slot's RESOURCE
  (faces share a resource through the backing member, views resolved
  to their textures).
- R3b' (stated for ratification): a CPU-written slot homes with its
  sole consuming family, COMPUTE families first — a render read of
  an authored table is R4's cross-family read, not a home claim.
- R4: every reading family gets a seat at the slot's number in its
  own stratum layout; visibility is the union of reaching stages
  (P-vis — declared equals actual by construction).
- P-num: WORLD/FRAME numbered in authored order; family bands of
  width 20 in roster order (AGENTS, TERRAIN, ZONES, ORBS, RIBBON, GALLERY, MESHGEN, SCENE, SHADOW) within groups 2 and 3;
  home slots ascend by old number; aliases keep shared numbers;
  the five MESHGEN scratch trios CONVERGE onto one params, one
  vertices, one indices slot (MESHGEN3; column adds its ground
  read as MESHGEN4).
- Family-less pipelines (compute_vp, update_camera) ADOPT the
  family layout that holds what they read; fade_overlay binds
  WORLD only. Render families derive from today's layout lists:
  gallery-entity users are GALLERY, depth-only are SHADOW, the
  fade overlay is FADE, the rest are SCENE.

## ★ STOP LIST — the rules could not place these ★

- slot (0, 112) (agent_figure_profiles): CPU-written, consumed by ['SCENE', 'SHADOW'] — R3b' cannot pick a sole family
- slot (0, 360) (render_ribbon): CPU-written, consumed by ['SCENE', 'SHADOW'] — R3b' cannot pick a sole family
- Generate Patch Heights (2D, pass 1) / C: storage 11 over the Core 8
- Generate Patch Gradients (2D, pass 2) / C: storage 11 over the Core 8
- Generate Patch Cells (2D, on demand) / C: storage 11 over the Core 8
- Compute Entity Placement (0D) / C: storage 11 over the Core 8
- Frustum Cull Patches / C: storage 11 over the Core 8

## Findings (facts the plan carries, not stops)

- decl render_vp is backed by 2 resources: photographerVPBuffer_, vpBuffer_
- decl render_camera is backed by 2 resources: cameraBuffer_, photographerCameraBuffer_

## The family roster, derived

| family | pipelines |
|---|---|
| AGENTS | `pawnAuraPipeline_`, `updateCubePipeline_`, `updateOtherAgentsPipeline_`, `updatePlayerAgentPipeline_`, `updateSpherePipeline_` |
| TERRAIN | `entityPlacementPipeline_`, `frustumCullPipeline_`, `generatePatchCellsPipeline_`, `generatePatchGradientsPipeline_`, `generatePatchHeightsPipeline_` |
| ZONES | `liveCardHeightsPipeline_`, `liveCardResolvePipeline_`, `zoneDeriveParamsPipeline_`, `zoneGolEvolvePipeline_`, `zoneGolSyncPipeline_`, `zoneSeedMaskPipeline_` |
| ORBS | `orbCopyPrevPipeline_`, `orbDynamicsPipeline_`, `orbInitPipeline_`, `orbRecolorPipeline_` |
| RIBBON | `ribbonRingPipeline_` |
| GALLERY | `galleryFramePipeline_`, `photographerVPPipeline_`, `wallPaintingCanvasPipeline_`, `wallPaintingFramePipeline_` |
| MESHGEN | `archMeshGenPipeline_`, `bladeMeshGenPipeline_`, `cactusMeshGenPipeline_`, `columnMeshGenPipeline_`, `palmMeshGenPipeline_` |
| SCENE | `archPipeline_`, `bladePipeline_`, `cactusPipeline_`, `columnPipeline_`, `monolithPipeline_`, `orbRenderPipeline_`, `palmPipeline_`, `patchTerrainIndirectPipeline_`, `patchTerrainPipeline_`, `pawnPipeline_`, `ribbonPipeline_`, `shellPipeline_`, `spherePipeline_` |
| SHADOW | `shadowArchPipeline_`, `shadowBladePipeline_`, `shadowCactusPipeline_`, `shadowColumnPipeline_`, `shadowGalleryFramePipeline_`, `shadowMonolithPipeline_`, `shadowPalmPipeline_`, `shadowPatchTerrainPipeline_`, `shadowPawnPipeline_`, `shadowRibbonPipeline_`, `shadowShellPipeline_`, `shadowSpherePipeline_`, `shadowWallPaintingPipeline_` |
| FADE | `fadeOverlayPipeline_` |
| (WORLD+FRAME) | `computeVPPipeline_`, `updateCameraPipeline_` |

## The four-strata seat roster

### WORLD (group 0, 2 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `config` | uniform | VFC | read by AGENTS, read by GALLERY, read by ORBS, read by SCENE, read by SHADOW, read by TERRAIN, read by ZONES |
| 1 | `tile_grid` | uniform | FC | read by AGENTS, read by SCENE, read by TERRAIN, read by ZONES |

### FRAME (group 1, 9 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `signal` | uniform | C | read by AGENTS, read by ZONES |
| 1 | `vp_data` | storage | C | read by TERRAIN |
| 2 | `render_vp` | storage | VF | read by GALLERY, read by SCENE, read by SHADOW |
| 3 | `camera_state` | storage | C | read by AGENTS, read by GALLERY |
| 4 | `render_camera` | storage | VF | read by GALLERY, read by SCENE |
| 5 | `render_lighting` | uniform | VF | read by SCENE, read by SHADOW |
| 6 | `shadow_slot` | uniform | V | read by SHADOW |
| 7 | `bilinear_sampler` | samplers | VFC | read by AGENTS, read by GALLERY, read by SCENE, read by SHADOW |
| 8 | `nearest_sampler` | samplers | VFC | read by AGENTS, read by SCENE, read by SHADOW, read by TERRAIN |

### AGENTS_STATE (group 2, 14 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `agent_state` | storage | C | home |
| 1 | `portal_array` | uniform | C | home |
| 2 | `floating_entities` | storage | C | home |
| 3 | `agent_behaviors` | uniform | C | home |
| 4 | `agent_tier_gains` | uniform | C | home |
| 5 | `pawn_aura_cfg` | uniform | C | home |
| 6 | `pawn_aura_cells` | storage | C | home |
| 9 | `occupier_cmg` | uniform | C | home |
| 10 | `occupier_amg` | uniform | C | home |
| 11 | `field_head_poses` | uniform | C | home |
| 12 | `field_forces` | storage | C | home |
| 13 | `field_ribbon` | uniform | C | home |
| 14 | `field_authored` | uniform | C | home |
| 28 | `patch_grid` | storage | C | R4 read (AGENTS) |

### AGENTS_TEXTURES (group 3, 5 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `pawn_aura_tex_write` | storagetex | C | home |
| 1 | `pawn_aura_read` | sampled | C | home |
| 22 | `photo_heightfield` | sampled | C | R4 read (AGENTS) |
| 23 | `photo_sampler` | samplers | C | R4 read (AGENTS) |
| 43 | `live_card_read` | sampled | C | R4 read (AGENTS) |

### CAMERA_READS_STATE (group 2, 3 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `agent_state` | storage | C | family-less read (update_camera) |
| 2 | `floating_entities` | storage | C | family-less read (update_camera) |
| 28 | `patch_grid` | storage | C | family-less read (update_camera) |

### CAMERA_READS_TEXTURES (group 3, 4 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 1 | `pawn_aura_read` | sampled | C | family-less read (update_camera) |
| 22 | `photo_heightfield` | sampled | C | family-less read (update_camera) |
| 23 | `photo_sampler` | samplers | C | family-less read (update_camera) |
| 43 | `live_card_read` | sampled | C | family-less read (update_camera) |

### GALLERY_STATE (group 2, 6 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `agent_state` | storage | C | R4 read (GALLERY) |
| 28 | `patch_grid` | storage | C | R4 read (GALLERY) |
| 34 | `painting_slots` | storage | VF | R4 read (GALLERY) |
| 100 | `photographer_config` | uniform | C | home |
| 101 | `photographer_vp` | storage | C | home |
| 102 | `photographer_camera_out` | storage | C | home |

### GALLERY_TEXTURES (group 3, 5 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 22 | `photo_heightfield` | sampled | C | R4 read (GALLERY) |
| 23 | `photo_sampler` | samplers | C | R4 read (GALLERY) |
| 43 | `live_card_read` | sampled | V | R4 read (GALLERY) |
| 100 | `painting_array` | sampled | F | home |
| 101 | `painting_sampler_filt` | samplers | F | home |

### MESHGEN_STATE (group 2, 5 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 29 | `cmg_column_ground` | storage | C | R4 read (MESHGEN) |
| 120 | `palmg_params` | storage | C | home |
| 121 | `palmg_vertices` | storage | C | home |
| 122 | `palmg_indices` | storage | C | home |
| 123 | `cmg_config` | uniform | C | home |

### ORBS_STATE (group 2, 5 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 60 | `orb_state` | storage | C | home |
| 61 | `orb_config` | uniform | C | home |
| 62 | `orb_state_prev` | storage | C | home |
| 63 | `orb_state_ro` | storage | C | home |
| 64 | `orb_state_prev_rw` | storage | C | home |

### RIBBON_STATE (group 2, 3 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 80 | `ribbon_state` | uniform | C | home |
| 81 | `ring_xforms` | storage | C | home |
| 82 | `head_poses` | storage | C | home |

### SCENE_STATE (group 2, 7 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 4 | `agent_tier_gains` | uniform | V | R4 read (SCENE) |
| 7 | `render_agents` | storage | VF | R4 read (SCENE) |
| 8 | `render_floating` | uniform | VF | R4 read (SCENE) |
| 30 | `patch_instances` | storage | V | R4 read (SCENE) |
| 31 | `visible_patch_indices` | storage | V | R4 read (SCENE) |
| 44 | `zone_params` | storage | F | R4 read (SCENE) |
| 83 | `render_ring_xforms` | storage | V | R4 read (SCENE) |

### SCENE_TEXTURES (group 3, 9 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 1 | `pawn_aura_read` | sampled | VF | R4 read (SCENE) |
| 25 | `entity_ground_atlas` | sampled | V | R4 read (SCENE) |
| 26 | `patch_heightfield_array_read` | sampled | V | R4 read (SCENE) |
| 27 | `patch_cell_color_array_read` | sampled | F | R4 read (SCENE) |
| 42 | `zone_life_read` | sampled | F | R4 read (SCENE) |
| 43 | `live_card_read` | sampled | VF | R4 read (SCENE) |
| 140 | `shadow_map` | sampled | F | home |
| 141 | `shadow_sampler` | samplers | F | home |
| 142 | `spot_shadow_map` | sampled | F | home |

### SHADOW_STATE (group 2, 5 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 7 | `render_agents` | storage | V | R4 read (SHADOW) |
| 8 | `render_floating` | uniform | V | R4 read (SHADOW) |
| 30 | `patch_instances` | storage | V | R4 read (SHADOW) |
| 34 | `painting_slots` | storage | V | R4 read (SHADOW) |
| 83 | `render_ring_xforms` | storage | V | R4 read (SHADOW) |

### SHADOW_TEXTURES (group 3, 3 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 25 | `entity_ground_atlas` | sampled | V | R4 read (SHADOW) |
| 26 | `patch_heightfield_array_read` | sampled | V | R4 read (SHADOW) |
| 43 | `live_card_read` | sampled | V | R4 read (SHADOW) |

### TERRAIN_STATE (group 2, 12 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 20 | `fc_draw_plan` | uniform | C | home |
| 21 | `patch_params` | uniform | C | home |
| 22 | `patch_height_scratch` | storage | C | home |
| 23 | `pyramid_instances` | uniform | C | home |
| 24 | `photo_painting_slots` | storage | C | home |
| 25 | `arch_ground` | storage | C | home |
| 26 | `column_ground` | storage | C | home |
| 27 | `plant_ground` | storage | C | home |
| 28 | `patch_grid` | storage | C | home |
| 30 | `patch_instances` | storage | C | home |
| 32 | `fc_visible` | storage | C | home |
| 33 | `fc_indirect` | storage | C | home |

### TERRAIN_TEXTURES (group 3, 6 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 20 | `patch_heightfield_array_write` | storagetex | C | home |
| 21 | `patch_cell_color_array_write` | storagetex | C | home |
| 22 | `photo_heightfield` | sampled | C | home |
| 23 | `photo_sampler` | samplers | C | home |
| 24 | `entity_ground_atlas_write` | storagetex | C | home |
| 43 | `live_card_read` | sampled | C | R4 read (TERRAIN) |

### VP_READS_STATE (group 2, 1 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 0 | `agent_state` | storage | C | family-less read (compute_vp) |

### ZONES_STATE (group 2, 4 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 40 | `live_card_scratch` | storage | C | home |
| 41 | `zone_config` | storage | C | home |
| 42 | `zone_life` | storage | C | home |
| 43 | `zone_derive_requests` | uniform | C | home |

### ZONES_TEXTURES (group 3, 2 seats)

| binding | decl | kind | visibility | why |
|---|---|---|---|---|
| 40 | `live_card_write` | storagetex | C | home |
| 41 | `zone_life_tex_write` | storagetex | C | home |

Seats: 141 today -> 110 planned — 45 duplicates collapse, 14 new R4
read seats appear, net -31. Layouts: 25 today -> 20 planned
(+ EMPTY at indices 1, 2, 3 — three empties or an amended 0c-4; the
plan provisions per-index empties and flags the choice).

## The old -> new number map (all 98 declarations)

| decl | old (g,b) | new (g,b) | home | reason |
|---|---|---|---|---|
| `agent_behaviors` | (0,110) | (2,3) | AGENTS | R3b' sole consuming family |
| `agent_tier_gains` | (0,111) | (2,4) | AGENTS | R3b' sole consuming family (compute-priority) |
| `agent_figure_profiles` | (0,112) | **UNPLACED** | — | — |
| `occupier_cmg` | (2,0) | (2,9) | AGENTS | authored (roster parenthetical / R5) |
| `occupier_amg` | (2,1) | (2,10) | AGENTS | authored (roster parenthetical / R5) |
| `field_head_poses` | (2,2) | (2,11) | AGENTS | authored (roster parenthetical / R5) |
| `field_forces` | (2,3) | (2,12) | AGENTS | authored (roster parenthetical / R5) |
| `field_ribbon` | (2,4) | (2,13) | AGENTS | authored (roster parenthetical / R5) |
| `field_authored` | (2,5) | (2,14) | AGENTS | authored (roster parenthetical / R5) |
| `pyramid_instances` | (0,30) | (2,23) | TERRAIN | R3b' sole consuming family |
| `signal` | (0,0) | (1,0) | FRAME | R2 authored |
| `config` | (0,1) | (0,0) | WORLD | R1 authored |
| `vp_data` | (0,2) | (1,1) | FRAME | R2 authored |
| `agent_state` | (0,60) | (2,0) | AGENTS | R3a writer family |
| `portal_array` | (0,62) | (2,1) | AGENTS | R3b' sole consuming family |
| `camera_state` | (0,80) | (1,3) | FRAME | R2 authored |
| `floating_entities` | (0,100) | (2,2) | AGENTS | R3a writer family |
| `ribbon_state` | (0,120) | (2,80) | RIBBON | R3b' sole consuming family |
| `render_vp` | (0,201) | (1,2) | FRAME | R2 authored |
| `render_agents` | (0,260) | (2,7) | AGENTS | R3a writer family |
| `render_camera` | (0,280) | (1,4) | FRAME | R2 authored |
| `render_floating` | (0,300) | (2,8) | AGENTS | R3a writer family |
| `render_ribbon` | (0,360) | **UNPLACED** | — | — |
| `render_ring_xforms` | (0,361) | (2,83) | RIBBON | R3a writer family |
| `entity_ground_atlas` | (0,390) | (3,25) | TERRAIN | R3a writer family |
| `ring_xforms` | (0,121) | (2,81) | RIBBON | R3a writer family |
| `head_poses` | (0,122) | (2,82) | RIBBON | R3b' sole consuming family |
| `render_lighting` | (0,320) | (1,5) | FRAME | R2 authored |
| `shadow_slot` | (0,362) | (1,6) | FRAME | R2 authored |
| `bilinear_sampler` | (1,22) | (1,7) | FRAME | R2 authored |
| `nearest_sampler` | (1,23) | (1,8) | FRAME | R2 authored |
| `shadow_map` | (1,25) | (3,140) | SCENE | R3b' sole consuming family |
| `shadow_sampler` | (1,26) | (3,141) | SCENE | R3b' sole consuming family |
| `spot_shadow_map` | (1,27) | (3,142) | SCENE | R3b' sole consuming family |
| `patch_params` | (0,23) | (2,21) | TERRAIN | R3b' sole consuming family |
| `patch_heightfield_array_write` | (0,24) | (3,20) | TERRAIN | R3a writer family |
| `tile_grid` | (0,25) | (0,1) | WORLD | R1 authored |
| `patch_cell_color_array_write` | (0,27) | (3,21) | TERRAIN | R3a writer family |
| `patch_height_scratch` | (0,28) | (2,22) | TERRAIN | R3a writer family |
| `patch_instances` | (0,340) | (2,30) | TERRAIN | R3b' sole consuming family (compute-priority) |
| `visible_patch_indices` | (0,391) | (2,31) | TERRAIN | R3a writer family |
| `patch_heightfield_array_read` | (1,28) | (3,26) | TERRAIN | R3a writer family |
| `patch_cell_color_array_read` | (1,29) | (3,27) | TERRAIN | R3a writer family |
| `zone_config` | (0,160) | (2,41) | ZONES | R3a writer family |
| `zone_life` | (0,161) | (2,42) | ZONES | R3a writer family |
| `zone_life_tex_write` | (0,162) | (3,41) | ZONES | R3a writer family |
| `zone_life_read` | (1,31) | (3,42) | ZONES | R3a writer family |
| `zone_params` | (1,32) | (2,44) | ZONES | R3a writer family |
| `pawn_aura_read` | (1,33) | (3,1) | AGENTS | R3a writer family |
| `live_card_read` | (1,34) | (3,43) | ZONES | R3a writer family |
| `pawn_aura_cfg` | (0,170) | (2,5) | AGENTS | R3b' sole consuming family |
| `pawn_aura_cells` | (0,171) | (2,6) | AGENTS | R3a writer family |
| `pawn_aura_tex_write` | (0,172) | (3,0) | AGENTS | R3a writer family |
| `live_card_write` | (0,31) | (3,40) | ZONES | R3a writer family |
| `live_card_scratch` | (0,32) | (2,40) | ZONES | R3a writer family |
| `zone_derive_requests` | (0,166) | (2,43) | ZONES | R3b' sole consuming family |
| `photographer_config` | (0,140) | (2,100) | GALLERY | R3b' sole consuming family |
| `photographer_vp` | (0,141) | (2,101) | GALLERY | R3a writer family |
| `photographer_camera_out` | (0,142) | (2,102) | GALLERY | R3a writer family |
| `photo_painting_slots` | (0,143) | (2,24) | TERRAIN | R3a writer family |
| `photo_heightfield` | (0,145) | (3,22) | TERRAIN | R3a writer family |
| `photo_sampler` | (0,146) | (3,23) | TERRAIN | authored (roster parenthetical / R5) |
| `arch_ground` | (0,147) | (2,25) | TERRAIN | R3a writer family |
| `column_ground` | (0,148) | (2,26) | TERRAIN | R3a writer family |
| `plant_ground` | (0,150) | (2,27) | TERRAIN | R3a writer family |
| `entity_ground_atlas_write` | (0,151) | (3,24) | TERRAIN | R3a writer family |
| `patch_grid` | (0,152) | (2,28) | TERRAIN | authored (roster parenthetical / R5) |
| `fc_config` | (0,1) | (0,0) | WORLD | R1 authored |
| `fc_vp` | (0,2) | (1,1) | FRAME | R2 authored |
| `fc_patches` | (0,340) | (2,30) | TERRAIN | R3b' sole consuming family (compute-priority) |
| `fc_visible` | (0,500) | (2,32) | TERRAIN | R3a writer family |
| `fc_indirect` | (0,501) | (2,33) | TERRAIN | R3a writer family |
| `fc_draw_plan` | (0,22) | (2,20) | TERRAIN | R3b' sole consuming family |
| `painting_slots` | (1,50) | (2,34) | TERRAIN | R3a writer family |
| `painting_array` | (1,51) | (3,100) | GALLERY | R3b' sole consuming family |
| `painting_sampler_filt` | (1,52) | (3,101) | GALLERY | R3b' sole consuming family |
| `amg_params` | (0,193) | (2,120) | MESHGEN | R3b' sole consuming family |
| `amg_vertices` | (0,194) | (2,121) | MESHGEN | R3a writer family |
| `amg_indices` | (0,195) | (2,122) | MESHGEN | R3a writer family |
| `cmg_params` | (0,196) | (2,120) | MESHGEN | R3b' sole consuming family |
| `cmg_vertices` | (0,197) | (2,121) | MESHGEN | R3a writer family |
| `cmg_indices` | (0,198) | (2,122) | MESHGEN | R3a writer family |
| `cmg_config` | (0,190) | (2,123) | MESHGEN | R3b' sole consuming family |
| `cmg_column_ground` | (0,191) | (2,29) | TERRAIN | R3a writer family |
| `palmg_params` | (0,180) | (2,120) | MESHGEN | R3b' sole consuming family |
| `palmg_vertices` | (0,181) | (2,121) | MESHGEN | R3a writer family |
| `palmg_indices` | (0,182) | (2,122) | MESHGEN | R3a writer family |
| `cactusg_params` | (0,183) | (2,120) | MESHGEN | R3b' sole consuming family |
| `cactusg_vertices` | (0,184) | (2,121) | MESHGEN | R3a writer family |
| `cactusg_indices` | (0,185) | (2,122) | MESHGEN | R3a writer family |
| `bladeg_params` | (0,186) | (2,120) | MESHGEN | R3b' sole consuming family |
| `bladeg_vertices` | (0,187) | (2,121) | MESHGEN | R3a writer family |
| `bladeg_indices` | (0,188) | (2,122) | MESHGEN | R3a writer family |
| `orb_state` | (0,410) | (2,60) | ORBS | R3a writer family |
| `orb_config` | (0,411) | (2,61) | ORBS | R3b' sole consuming family |
| `orb_state_prev` | (0,412) | (2,62) | ORBS | R3a writer family |
| `orb_state_ro` | (0,413) | (2,63) | ORBS | R3a writer family |
| `orb_state_prev_rw` | (0,414) | (2,64) | ORBS | R3a writer family |

## Predicted Table B against today's (declared, per stage)

Every (pipeline, stage) row under the recut, with today's declared
counts beside it. The gate law: uniform 12 / storage 8 / sampled 16
/ samplers 16 / storage-tex 4 per stage. Any exceedance is in the
STOP list above.

| pipeline | st | new u/s/t/sm/st | today u/s/t/sm/st |
|---|---|---|---|
| Update Player Agent (0D, 1 thread) | C | 12/7/3/3/1 | 10/6/3/3/0 |
| Update Other Agents (1D, 32 threads) | C | 12/7/3/3/1 | 10/6/3/3/0 |
| Update Camera (0D) | C | 3/5/3/3/0 | 5/5/3/3/0 |
| Update Sphere (0D) | C | 12/7/3/3/1 | 10/6/3/3/0 |
| Update Cube (0D) | C | 12/7/3/3/1 | 10/6/3/3/0 |
| Compute VP Matrix (0D) | C | 3/3/0/2/0 | 5/5/1/1/0 |
| Generate Patch Heights (2D, pass 1) | C | 6/11/2/3/3 | 4/1/0/0/2 |
| Generate Patch Gradients (2D, pass 2) | C | 6/11/2/3/3 | 4/1/0/0/2 |
| Generate Patch Cells (2D, on demand) | C | 6/11/2/3/3 | 4/1/0/0/2 |
| Compute Ribbon Rings (1D, per frame) | C | 4/4/0/2/0 | 1/2/0/0/0 |
| Compute Photographer VP (0D) | C | 4/6/1/3/0 | 2/5/1/1/0 |
| Compute Entity Placement (0D) | C | 6/11/2/3/3 | 1/5/3/3/1 |
| Frustum Cull Patches | C | 6/11/2/3/3 | 2/4/0/0/0 |
| Compute Pawn Aura (2D) | C | 12/7/3/3/1 | 3/2/0/0/1 |
| Live Card Heights (2D) | C | 4/5/0/2/2 | 2/3/0/0/1 |
| Live Card Resolve (2D) | C | 4/5/0/2/2 | 2/3/0/0/1 |
| Orb Init | C | 4/6/0/2/0 | 2/2/0/0/0 |
| Orb Dynamics | C | 4/6/0/2/0 | 2/2/0/0/0 |
| Orb Recolor | C | 4/6/0/2/0 | 2/2/0/0/0 |
| Orb State Prev Copy | C | 4/6/0/2/0 | 1/2/0/0/0 |
| GoL Zone Sync | C | 4/5/0/2/2 | 2/2/0/0/1 |
| GoL Zone Evolve | C | 4/5/0/2/2 | 2/2/0/0/1 |
| Zone Derive Params | C | 4/5/0/2/2 | 2/2/0/0/1 |
| Zone Seed Mask (2D) | C | 4/5/0/2/2 | 3/2/0/0/0 |
| Arch Mesh Gen | C | 4/6/0/2/0 | 0/3/0/0/0 |
| Column Mesh Gen | C | 4/6/0/2/0 | 1/4/0/0/0 |
| Palm Mesh Gen | C | 4/6/0/2/0 | 0/3/0/0/0 |
| Cactus Mesh Gen | C | 4/6/0/2/0 | 0/3/0/0/0 |
| Blade Mesh Gen | C | 4/6/0/2/0 | 0/3/0/0/0 |
| Patch Terrain (instanced) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Patch Terrain (instanced) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Patch Terrain Indirect (VS indirection) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Patch Terrain Indirect (VS indirection) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Pawn Entity (Chess Pawn) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Pawn Entity (Chess Pawn) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Sphere Entity (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Sphere Entity (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Monolith Entity (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Monolith Entity (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Catenary Arch (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Catenary Arch (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Generative Column (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Generative Column (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Palm Tree (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Palm Tree (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Cactus (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Cactus (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Blade Cluster (Rasterized) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Blade Cluster (Rasterized) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Indoor Shell (Ceiling + Walls) | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Indoor Shell (Ceiling + Walls) | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Sky Ribbon Entity | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Sky Ribbon Entity | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Orb Sky Layer | V | 5/6/4/2/0 | 7/6/4/2/0 |
| Orb Sky Layer | F | 4/4/6/3/0 | 4/4/6/3/0 |
| Gallery Frame | V | 3/3/1/2/0 | 1/2/1/1/0 |
| Gallery Frame | F | 3/3/1/3/0 | 1/2/1/1/0 |
| Wall Painting Canvas | V | 3/3/1/2/0 | 1/2/1/1/0 |
| Wall Painting Canvas | F | 3/3/1/3/0 | 1/2/1/1/0 |
| Wall Painting Frame | V | 3/3/1/2/0 | 1/2/1/1/0 |
| Wall Painting Frame | F | 3/3/1/3/0 | 1/2/1/1/0 |
| Shadow Patch Terrain | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Pawn | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Sphere | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Monolith | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Catenary Arch | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Generative Column | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Palm Tree | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Cactus | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Blade Cluster | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Indoor Shell | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Sky Ribbon | V | 4/6/3/2/0 | 7/6/3/2/0 |
| Shadow Gallery Frame | V | 4/6/3/2/0 | 7/7/2/1/0 |
| Shadow Wall Painting | V | 4/6/3/2/0 | 7/7/2/1/0 |
| Fade Overlay | V | 1/0/0/0/0 | 0/0/0/0/0 |
| Fade Overlay | F | 2/0/0/0/0 | 1/0/0/0/0 |

## The pipeline-layout rewrite list (renderer.hpp)

| pipeline | today | planned strata |
|---|---|---|
| `updatePlayerAgentPipeline_` | `computeEntityBindGroupLayout_` → `computeTextureBindGroupLayout_` → `roomLayout_` | `WORLD` → `FRAME` → `AGENTS_STATE` → `AGENTS_TEXTURES` |
| `updateOtherAgentsPipeline_` | `computeEntityBindGroupLayout_` → `computeTextureBindGroupLayout_` → `roomLayout_` | `WORLD` → `FRAME` → `AGENTS_STATE` → `AGENTS_TEXTURES` |
| `updateCameraPipeline_` | `computeEntityBindGroupLayout_` → `computeTextureBindGroupLayout_` | `WORLD` → `FRAME` → `CAMERA_READS_STATE` → `CAMERA_READS_TEXTURES` |
| `updateSpherePipeline_` | `computeEntityBindGroupLayout_` → `computeTextureBindGroupLayout_` → `roomLayout_` | `WORLD` → `FRAME` → `AGENTS_STATE` → `AGENTS_TEXTURES` |
| `updateCubePipeline_` | `computeEntityBindGroupLayout_` → `computeTextureBindGroupLayout_` → `roomLayout_` | `WORLD` → `FRAME` → `AGENTS_STATE` → `AGENTS_TEXTURES` |
| `computeVPPipeline_` | `computeEntityBindGroupLayout_` | `WORLD` → `FRAME` → `VP_READS_STATE` → `EMPTY` |
| `generatePatchHeightsPipeline_` | `patchGenLayout_` | `WORLD` → `FRAME` → `TERRAIN_STATE` → `TERRAIN_TEXTURES` |
| `generatePatchGradientsPipeline_` | `patchGenLayout_` | `WORLD` → `FRAME` → `TERRAIN_STATE` → `TERRAIN_TEXTURES` |
| `generatePatchCellsPipeline_` | `patchGenLayout_` | `WORLD` → `FRAME` → `TERRAIN_STATE` → `TERRAIN_TEXTURES` |
| `ribbonRingPipeline_` | `ribbonComputeLayout_` | `WORLD` → `FRAME` → `RIBBON_STATE` → `EMPTY` |
| `photographerVPPipeline_` | `photographerComputeLayout_` | `WORLD` → `FRAME` → `GALLERY_STATE` → `GALLERY_TEXTURES` |
| `entityPlacementPipeline_` | `entityPlacementComputeLayout_` → `computeTextureBindGroupLayout_` | `WORLD` → `FRAME` → `TERRAIN_STATE` → `TERRAIN_TEXTURES` |
| `frustumCullPipeline_` | `frustumCullLayout_` | `WORLD` → `FRAME` → `TERRAIN_STATE` → `TERRAIN_TEXTURES` |
| `pawnAuraPipeline_` | `pawnAuraComputeLayout_` | `WORLD` → `FRAME` → `AGENTS_STATE` → `AGENTS_TEXTURES` |
| `liveCardHeightsPipeline_` | `liveCardWriterLayout_` | `WORLD` → `FRAME` → `ZONES_STATE` → `ZONES_TEXTURES` |
| `liveCardResolvePipeline_` | `liveCardWriterLayout_` | `WORLD` → `FRAME` → `ZONES_STATE` → `ZONES_TEXTURES` |
| `orbInitPipeline_` | `orbComputeLayout_` | `WORLD` → `FRAME` → `ORBS_STATE` → `EMPTY` |
| `orbDynamicsPipeline_` | `orbComputeLayout_` | `WORLD` → `FRAME` → `ORBS_STATE` → `EMPTY` |
| `orbRecolorPipeline_` | `orbComputeLayout_` | `WORLD` → `FRAME` → `ORBS_STATE` → `EMPTY` |
| `orbCopyPrevPipeline_` | `orbCopyLayout_` | `WORLD` → `FRAME` → `ORBS_STATE` → `EMPTY` |
| `zoneGolSyncPipeline_` | `zoneGolComputeLayout_` | `WORLD` → `FRAME` → `ZONES_STATE` → `ZONES_TEXTURES` |
| `zoneGolEvolvePipeline_` | `zoneGolComputeLayout_` | `WORLD` → `FRAME` → `ZONES_STATE` → `ZONES_TEXTURES` |
| `zoneDeriveParamsPipeline_` | `zoneGolComputeLayout_` | `WORLD` → `FRAME` → `ZONES_STATE` → `ZONES_TEXTURES` |
| `zoneSeedMaskPipeline_` | `zoneMaskLayout_` | `WORLD` → `FRAME` → `ZONES_STATE` → `ZONES_TEXTURES` |
| `archMeshGenPipeline_` | `archMeshGenLayout_` | `WORLD` → `FRAME` → `MESHGEN_STATE` → `EMPTY` |
| `columnMeshGenPipeline_` | `columnMeshGenLayout_` | `WORLD` → `FRAME` → `MESHGEN_STATE` → `EMPTY` |
| `palmMeshGenPipeline_` | `palmMeshGenLayout_` | `WORLD` → `FRAME` → `MESHGEN_STATE` → `EMPTY` |
| `cactusMeshGenPipeline_` | `cactusMeshGenLayout_` | `WORLD` → `FRAME` → `MESHGEN_STATE` → `EMPTY` |
| `bladeMeshGenPipeline_` | `bladeMeshGenLayout_` | `WORLD` → `FRAME` → `MESHGEN_STATE` → `EMPTY` |
| `patchTerrainPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `patchTerrainIndirectPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `pawnPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `spherePipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `monolithPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `archPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `columnPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `palmPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `cactusPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `bladePipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `shellPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `ribbonPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `orbRenderPipeline_` | `renderEntityBindGroupLayout_` → `renderTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SCENE_STATE` → `SCENE_TEXTURES` |
| `galleryFramePipeline_` | `galleryEntityBindGroupLayout_` → `galleryTextureBindGroupLayout_` | `WORLD` → `FRAME` → `GALLERY_STATE` → `GALLERY_TEXTURES` |
| `wallPaintingCanvasPipeline_` | `galleryEntityBindGroupLayout_` → `galleryTextureBindGroupLayout_` | `WORLD` → `FRAME` → `GALLERY_STATE` → `GALLERY_TEXTURES` |
| `wallPaintingFramePipeline_` | `galleryEntityBindGroupLayout_` → `galleryTextureBindGroupLayout_` | `WORLD` → `FRAME` → `GALLERY_STATE` → `GALLERY_TEXTURES` |
| `shadowPatchTerrainPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowPawnPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowSpherePipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowMonolithPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowArchPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowColumnPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowPalmPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowCactusPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowBladePipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowShellPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowRibbonPipeline_` | `renderEntityBindGroupLayout_` → `shadowTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowGalleryFramePipeline_` | `renderEntityBindGroupLayout_` → `galleryTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `shadowWallPaintingPipeline_` | `renderEntityBindGroupLayout_` → `galleryTextureBindGroupLayout_` | `WORLD` → `FRAME` → `SHADOW_STATE` → `SHADOW_TEXTURES` |
| `fadeOverlayPipeline_` | `meshGenEntityBindGroupLayout_` | `WORLD` → `EMPTY` → `EMPTY` → `EMPTY` |

## The SetBindGroup rewrite list (from M7)

Per site: the group member bound today, and the strata the
pipelines behind that site require. Sites binding a group whose
layout serves a whole pass head are marked HOIST (bind once per
pass; bind-group state is sticky).

| site | fn | binds today | families served | planned binds |
|---|---|---|---|---|
| `gallery.hpp:1502` | `render_snapshot_pass` | 0 ← `photographerRenderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &kSlotZero |
| `gallery.hpp:1503` | `render_snapshot_pass` | 1 ← `renderTextureBindGroup_` | SCENE | 0 WORLD · 1 FRAME · 2/3 per family |
| `gallery.hpp:1529` | `render_snapshot_pass` | 0 ← `galleryPhotographerEntityBindGroup_` | GALLERY | 0 WORLD · 1 FRAME · 2/3 per family |
| `gallery.hpp:1530` | `render_snapshot_pass` | 1 ← `galleryTextureBindGroup_` | GALLERY, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:150` | `dispatch_placement_correction` | 1 ← `computeTextureBindGroup_` | (WORLD+FRAME), AGENTS, TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:197` | `dispatch_compute` | 0 ← `computeEntityBindGroup_` | (WORLD+FRAME), AGENTS | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:198` | `dispatch_compute` | 1 ← `computeTextureBindGroup_` | (WORLD+FRAME), AGENTS, TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:199` | `dispatch_compute` | 2 ← `roomBindGroup_` | AGENTS | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:330` | `render_shadow_pass` | 0 ← `renderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &slotOffset |
| `render_passes.hpp:331` | `render_shadow_pass` | 1 ← `shadowTextureBindGroup_` | SHADOW | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:365` | `render_shadow_pass` | 0 ← `renderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &slotOffset |
| `render_passes.hpp:366` | `render_shadow_pass` | 1 ← `shadowTextureBindGroup_` | SHADOW | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:465` | `draw_shadow_all` | 1 ← `galleryTextureBindGroup_` | GALLERY, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:529` | `render_main_pass` | 1 ← `renderTextureBindGroup_` | SCENE | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:553` | `render_main_pass` | 0 ← `renderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &kSlotZero |
| `render_passes.hpp:570` | `render_main_pass` | 0 ← `galleryEntityBindGroup_` | GALLERY | 0 WORLD · 1 FRAME · 2/3 per family |
| `render_passes.hpp:571` | `render_main_pass` | 1 ← `galleryTextureBindGroup_` | GALLERY, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:418` | `dispatch_generate_patch_heights` | 0 ← `patchGenBindGroup_` | TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:429` | `dispatch_generate_patch_gradients` | 0 ← `patchGenBindGroup_` | TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:439` | `dispatch_generate_patch_cells` | 0 ← `patchGenBindGroup_` | TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:450` | `dispatch_compute_ribbon_rings` | 0 ← `ribbonComputeBindGroup_` | RIBBON | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:460` | `dispatch_compute_photographer_vp` | 0 ← `photographerComputeBindGroup_` | GALLERY | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:469` | `dispatch_entity_placement` | 0 ← `entityPlacementComputeBindGroup_` | TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:478` | `dispatch_frustum_cull` | 0 ← `frustumCullBindGroup_` | TERRAIN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:492` | `dispatch_compute_pawn_aura` | 0 ← `pawnAuraComputeGroup_` | AGENTS | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:503` | `dispatch_live_card_write` | 0 ← `liveCardWriterGroup_` | ZONES | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:518` | `dispatch_orb_init` | 0 ← `orbComputeGroup_` | ORBS | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:529` | `dispatch_orb_dynamics` | 0 ← `orbComputeGroup_` | ORBS | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:540` | `dispatch_orb_recolor` | 0 ← `orbComputeGroup_` | ORBS | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:551` | `dispatch_orb_copy_prev` | 0 ← `orbCopyGroup_` | ORBS | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:569` | `draw_orbs` | 0 ← `renderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &kShadowSlotZero |
| `renderer.hpp:570` | `draw_orbs` | 1 ← `renderTextureBindGroup_` | SCENE | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:587` | `dispatch_zone_gol_sync` | 0 ← `zoneGolComputeBindGroup_` | ZONES | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:604` | `dispatch_zone_gol_evolve` | 0 ← `zoneGolComputeBindGroup_` | ZONES | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:617` | `dispatch_zone_derive_params` | 0 ← `zoneGolComputeBindGroup_` | ZONES | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:629` | `dispatch_zone_seed_mask` | 0 ← `zoneMaskGroup_` | ZONES | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:644` | `dispatch_arch_mesh_gen` | 0 ← `archMeshGenBindGroup_` | MESHGEN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:655` | `dispatch_column_mesh_gen` | 0 ← `columnMeshGenBindGroup_` | MESHGEN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:665` | `dispatch_palm_mesh_gen` | 0 ← `palmMeshGenBindGroup_` | MESHGEN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:675` | `dispatch_cactus_mesh_gen` | 0 ← `cactusMeshGenBindGroup_` | MESHGEN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:685` | `dispatch_blade_mesh_gen` | 0 ← `bladeMeshGenBindGroup_` | MESHGEN | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:708` | `draw_patch_terrain_plan_slot` | 0 ← `renderEntityBindGroupPlanB_`, `renderEntityBindGroupPlanC_`, `renderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &kShadowSlotZero |
| `renderer.hpp:725` | `draw_patch_terrain_direct` | 0 ← `photographerRenderEntityBindGroup_` | SCENE, SHADOW | 0 WORLD · 1 FRAME · 2/3 per family · offsets: 1, &kShadowSlotZero |
| `renderer.hpp:726` | `draw_patch_terrain_direct` | 1 ← `renderTextureBindGroup_` | SCENE | 0 WORLD · 1 FRAME · 2/3 per family |
| `renderer.hpp:950` | `draw_fade_overlay` | 0 ← `meshGenEntityBindGroup_` | FADE | 0 WORLD · 1 FRAME · 2/3 per family |

## Retired seats (collapsed duplicates)

Old seats whose slot now carries fewer seats than it did; each old
home named. Collapse is legal only where (slot, kind, access) were
identical — P-cons — and every declaration survives.

| slot (old) | decl | old seats | planned seats |
|---|---|---|---|
| (0,0) | `signal` | Compute Entity Layout[0]; Live Card Writer Layout[0] | FRAME |
| (0,1) | `config` | Compute Entity Layout[1]; Render Entity Layout[0]; Mesh Gen Entity Layout[0]; Patch Gen Layout[0]; Gallery Entity Layout[0]; Photographer Compute Layout[0]; Entity Placement Compute Layout[0]; Frustum Cull Compute Layout[0]; GoL Zone Compute Layout[0]; Pawn Aura Compute Layout[0]; Live Card Writer Layout[1]; Zone Mask Layout[0]; Orb Compute Layout[3] | WORLD |
| (0,2) | `vp_data` | Compute Entity Layout[2]; Frustum Cull Compute Layout[1] | FRAME |
| (0,25) | `tile_grid` | Render Entity Layout[9]; Patch Gen Layout[3]; Pawn Aura Compute Layout[5]; Zone Mask Layout[1] | WORLD |
| (0,80) | `camera_state` | Compute Entity Layout[4]; Photographer Compute Layout[8] | FRAME |
| (0,112) | `agent_figure_profiles` | Render Entity Layout[13] | — |
| (0,160) | `zone_config` | GoL Zone Compute Layout[1]; Live Card Writer Layout[2]; Zone Mask Layout[2] | ZONES_STATE |
| (0,161) | `zone_life` | GoL Zone Compute Layout[2]; Live Card Writer Layout[3]; Zone Mask Layout[3] | ZONES_STATE |
| (0,166) | `zone_derive_requests` | GoL Zone Compute Layout[4]; Zone Mask Layout[4] | ZONES_STATE |
| (0,183) | `cactusg_params` | Cactus Mesh Gen Layout[0] | — |
| (0,184) | `cactusg_vertices` | Cactus Mesh Gen Layout[1] | — |
| (0,185) | `cactusg_indices` | Cactus Mesh Gen Layout[2] | — |
| (0,186) | `bladeg_params` | Blade Mesh Gen Layout[0] | — |
| (0,187) | `bladeg_vertices` | Blade Mesh Gen Layout[1] | — |
| (0,188) | `bladeg_indices` | Blade Mesh Gen Layout[2] | — |
| (0,193) | `amg_params` | Arch Mesh Gen Layout[0] | — |
| (0,194) | `amg_vertices` | Arch Mesh Gen Layout[1] | — |
| (0,195) | `amg_indices` | Arch Mesh Gen Layout[2] | — |
| (0,196) | `cmg_params` | Column Mesh Gen Layout[0] | — |
| (0,197) | `cmg_vertices` | Column Mesh Gen Layout[1] | — |
| (0,198) | `cmg_indices` | Column Mesh Gen Layout[2] | — |
| (0,201) | `render_vp` | Render Entity Layout[1]; Gallery Entity Layout[1] | FRAME |
| (0,280) | `render_camera` | Render Entity Layout[3]; Gallery Entity Layout[2] | FRAME |
| (0,360) | `render_ribbon` | Render Entity Layout[7] | — |
| (0,411) | `orb_config` | Orb Compute Layout[1]; Orb Copy Layout[1] | ORBS_STATE |
| (1,22) | `bilinear_sampler` | Shadow Texture Layout[0]; Render Texture Layout[0]; Compute Texture Layout[0]; Gallery Texture Layout[3] | FRAME |
| (1,23) | `nearest_sampler` | Shadow Texture Layout[1]; Render Texture Layout[1]; Compute Texture Layout[1] | FRAME |

Collapsed seats total: 45.

## P-inv — the render = compute + 200 band, retired

The four static_asserts of the registry's witness band are declared
RETIRED by this plan: the recut ends the +200 mirror numbering the
band checked, so the invariants die with it. The registry prose
keeps the epitaph: the band was the witness over authored literals
from C6 until LOOM_2, and the schema's --check is its successor.

## Ratification

★ THE CAMPAIGN HALTS HERE. ★ Nothing in U2 — no schema rewrite, no
--write, no --write-wgsl, no renderer or call-site edit — may begin
until Jean ratifies this plan, and the STOP list above is ruled on first.
