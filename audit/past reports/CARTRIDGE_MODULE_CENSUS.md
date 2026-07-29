> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# the_board — MODULE CENSUS (census-only; no bodies, no exposition)

Scope: `cartridge.hpp` + all 34 `*.hpp` + `world.wgsl` (no `*.inl` exist). Vocabulary anchored to `src/docs/7t_program_theory_v3.md` (§2 strata L0–L5; §9 terrain strata S1–S4) + `audit/LADDER.md` (M6 disposition).

**stratum** — general modules get L0 substrate · L1 primitives · L2 entities · L3 couplings · L4 orchestration · L5 realization; modules whose *primary* job is the ground substrate get the §9 terrain tag S1 surface · S2 lifecycle · S3 placement · S4 motion. (Contracts tagged by the layer they serve.)
**stability** — SPINE (landed; markers are named/resolved SEAMs or STATUS:REALIZED) · IN-FLIGHT (open surface: STATUS:INTENT stub, TODO/"revert before ship", unbuilt) · RESIDUE (dead/driverless/excavate per M6). **No module is wholly RESIDUE — residue is intra-module** (see the RESIDUE-LOCI line under §1).
**requires** — internal (`the_board/…`) includes only, as basenames; std headers dropped. Leaf = no internal deps.

## SECTION 1 — MODULE TABLE

| module | strat | role (≤10w) | offer-face | requires | stability | terrain-touch |
|---|---|---|---|---|---|---|
| `keyhole.hpp` | L0 | shared fwd decls provided once (the keyhole) | `Cartridge`, `wgpu::Queue` (fwd) | leaf | SPINE | N |
| `seed_utils.hpp` | L1 | pure hash/gaussian/weight/catenary math (WGSL mirrors) | `cpu_hash`, `tile_seed`, `cpu_sample_gaussian`, `select_weighted`, `solve_catenary_a` | leaf | SPINE | N |
| `mood_constants.hpp` | L1 | mood-count + Mood IDs + PortalDestination | `MOOD_COUNT`, `MOOD_*` ids, `PortalDestination` | leaf | SPINE | N |
| `roster.hpp` | L4 | spine-owned piece-enable manifest gating doors | `PopFamily`, `Roster`, `family_enabled` | leaf | SPINE | N |
| `entity_types.hpp` | L2 | generic entity-pipeline DTOs + machine face + dispatch | `MachineCtx`, `EntityFamilyTraits`, `FamilyDispatch`, `FAMILY_DISPATCH` | roster, keyhole | SPINE | Y (ground_y_offset added to terrain Y; gpu_ground_y; PatchSystemState mbr) |
| `floater_vocabulary.hpp` | L2 | sphere+cube floater family vocabulary | `SphereConfig`, `CubeConfig`, `ActiveFloater`, `ActiveCube` | mood_constants | SPINE (naming-latent) | N |
| `point.hpp` | L4 | the point: host + terrain rule + bubble | `PointState`, `PointHost`, `PointTerrainRule`, `PointBubble` | leaf | SPINE | Y (host terrain rule: SNAP/SOFT_FLOOR/NONE) |
| `spine_state.hpp` | L4 | spine organ types: time/player/input/mood + mood table | `PlayerState`, `MoodProfile`, `MOOD_TABLE`, `TransitionPhase`, `CeilingType` | mood_constants | SPINE | Y (terrain_amp_ceiling, allow_pawn_aura/cull, finite_radius bounds) |
| `demo_config.hpp` | L5 | one demo sentence: manifest + seed + boot mood | `DemoConfig` | roster | SPINE | N |
| `ground_architecture.hpp` | **S1** | ground-query contributor registry + DAG + policies | `ContributorId`, `PolicyId`, `POLICIES`, `CONTRIBUTOR_DAG`, `GROUND_STATIC_BASE_MASK` | leaf | **IN-FLIGHT** (INTENT stubs: CONTRIB_PAINTINGS/VEGETATION_BASES, fused endpoints "0.0 today"; LATENT[policy-surface] = manifold's future iface, KEEP) | Y (the terrain contract; POLICY_* mirror to world.wgsl) |
| `surface_services.hpp` | **S1/S2** | surface decl tier: WorldState, patch registry, streaming decls | `WorldState`, `ActivePatch`, `PatchSystemState`, `stream_patches`, `find_patch`/`evict_patch` | keyhole | SPINE | Y (patch/heightfield lifecycle, PATCH_EXTENT, visibility cylinder) |
| `spawn_services.hpp` | **S3** | machine decl tier: spawn/place decls + separation matrix + arch vocab | `MIN_SEPARATION`, `ARCH_TIERS`, `evaluate_spawn_gate`, `generic_select/place/commit`, `run_spawn_preamble` | roster, keyhole, entity_types | SPINE | Y (grounded-entity placement, ground mesh params, footprints) |
| `patch_system.hpp` | **S2** | active-patch registry + per-frame streaming conductor | `stream_patches`, `init_patch_system`, `make_patch_params`, `mark_patches_for_regen` | keyhole, surface_services | SPINE (TESTING test-rig-piers = ruled debug fixture) | Y (patch lifecycle; GPU heightfield gen; finite center-pin+radius-cap L558/569/810/892) |
| `tile_world.hpp` | **S1** | terrain memory: per-tile shape/population + landform tokens | `TileShape`, `TilePopulation`, `estimate_terrain_height`, `tile_archetype`, `generate_tile_state` | roster, keyhole | SPINE (1 dormant field: activation_scale = M6 DORMANT-VOICE) | Y (base-shape amp/bias/archetype; F1 CPU height estimate; landform tokens) [spans S1+S3] |
| `population_themes.hpp` | **S3** | per-region compositional vocabulary + envelope selector | `THEMES`, `PopulationTheme`, `ThemesState`, `evaluate_theme_envelope` | roster, seed_utils | SPINE | N (population/composition only; feeds TilePopulation, no height math) |
| `entity_pipeline.hpp` | **S3** | generic 3-phase entity lifecycle + welded family adapters | `generic_select/place/commit`, `*_ADAPTER` (COLUMN/ARCH/PYRAMID/ANTENNA), `rescale_to_rolled_target` | keyhole | SPINE (L808 dead color_override branch, both identical) | Y (pyramids bake into heightfield → mark_patches_for_regen; arch/column write piers) [spans S3+S1-edit] |
| `spawn_engine.hpp` | **S3** | spawn helpers, footprints, culling + select/place/commit loops | `run_spawn_preamble`, `negotiate_position`, `SpawnEngineState`, `GroundFootprint` | keyhole | SPINE | Y (spawn-time ground sample via F3/F4; finite indoor wall-margin clamp L255-272) |
| `agents.hpp` | L2 | agent population: behaviors, tiers, spawn, possession | `AgentState`, `AgentBehaviorId`, `spawn_population_for_mood`, `try_possess_nearest` | state, mood_constants, keyhole | SPINE (SEAM[agents:L2] hw-mirror const drift risk) | N (spawns on the point; pos_y=0, no height read) |
| `cube_behaviors.hpp` | L2 | cube floater behaviors: spawn/corral/kite/dispatch/evict | `CubeBehaviorsState`, `evict_cube`, `corral_cubes`, `toggle_cube_kite_mode` | state, floater_vocabulary, mood_constants, keyhole, entity_types | SPINE | N (floaters, anchor y=0) |
| `entities.hpp` | L2 | grounded-seven vocabulary + state + mesh-gen/evict/dispatch | `EntitiesState`, `ActiveArch/Column/Pyramid`, `force_spawn_portal_arch`, `evict_*` | state, mood_constants, keyhole, entity_types | SPINE (LATENT[unused] PAWN_HEIGHT_UNITS zero-caller; LATENT[naming]) | Y (grounded families; cached_ground_y pier-top; ground_entries_dirty) |
| `gallery.hpp` | L2 | art system: photographer + outdoor/indoor gallery placement | `GalleryState`, `PhotographerState`, `update_photographer`, `place_wall_paintings`, `WALL_ART` | state, mood_constants, seed_utils, keyhole, entity_types | SPINE (NOTE gallery:shadows-missing known gap) | Y (FormType::TERRAIN_QUAD paintings; tile_archetype/spawn_mult) |
| `gol_zones.hpp` | L2 | zone-local Game-of-Life + Pulse automata subsystem | `GoLState`, `evict_gol`, `flush_zone_derive_requests`, `dispatch_zone_evolve/mesh` | state, mood_constants, keyhole, entity_types | SPINE (M6 LIVE-BUT-FRAGILE; roster-gateable → ROSTER-RESIDUE recipe when off) | Y (cell-grid-snapped PATCH_CELL_SIZE; per-cell height extrude) |
| `orbs.hpp` | L2 | sky-dome orb layer: palettes, tiers, motion gestures | `OrbsState`, `ORB_MOOD_TABLE`, `configure_orbs`, `dispatch_orb_dynamics`/`render_orbs` | mood_constants, keyhole | SPINE (DEAD WIRE dome_center ABI-kept; driverless noise_amp floor) | N (skybox dome, eye-centered) |
| `pawn.hpp` | L3 | player-relative aura field + presence-ramp coupling | `PawnState`, `PawnAuraProfile`, `tick_pawn_couplings`, `dispatch_pawn_aura`, `toggle_aura` | keyhole | SPINE (SEAM[spine:P8] aura on player_) | Y (aura cell_size=PATCH_CELL_SIZE; aura_height → terrain VS extrusion) |
| `ribbon.hpp` | L2 | sky ribbon subsystem: head-steering law + propagation body | `RibbonState`, `RibbonHead`, `ribbon_frame_tick`, `ribbon_advance_head`, `commit_ribbon` | state, mood_constants, keyhole, entity_types | **IN-FLIGHT** (⚠ TESTING SPAWN_CHANCE=0.900 "revert before ship" L93; else settled) | Y (estimate_terrain_height floor; RIBBON_FLOOR_MARGIN over tall ground) |
| `spheres.hpp` | L2 | orbital-sphere floater state + recipe/dispatch/evict | `SphereState`, `evict_sphere`, `reconcile_sphere_mirror`, `SPHERE_ADAPTER` | state, floater_vocabulary, keyhole, entity_types | SPINE (STATUS:LATENT[naming] ActiveFloater→ActiveSphere) | N (floaters, anchor y=0) |
| `input.hpp` | L4 | input dispatch: keys/mouse → move-intent + view toggles | `CameraControls`, `on_key_down`, `update_movement_intent`, `toggle_sky_mode`, `set_render_radius` | spine_state, mood_constants, point | **IN-FLIGHT** (sky-mode fade transition "remains unbuilt" L405) | Y (set_render_radius clamps active_radius GRID..PREGEN; keys 5-9 finite moods) |
| `mood.hpp` | L4 | atmosphere/lighting/indoor-shell/portals + 6 doors | `apply_mood`(+lighting/spot/indoor_shell/anchor_ribbon), `request_mood_transition`, `derive_finite_radius`, `INDOOR_ENTITY_WALL_MARGIN` | state, mood_constants, spine_state, keyhole | SPINE | Y (finite box: bmin=-r·EXTENT, bmax=(r+1)·EXTENT → shell/walls; set_ceiling_height; derive_finite_radius) |
| `render_passes.hpp` | L5 | GPU dispatch + draw-call bodies on MachineCtx face | `upload_ground_entries`, `dispatch_frustum_cull`, `render_shadow_pass`, `render_main_pass`, `compute_sun_matrices` | state, keyhole | SPINE | Y (draw_patch_terrain LOD0/LOD1; draw_shadow_patch_terrain; cull patches; reads cpuPiers_) |
| `renderer.hpp` | L5 | WebGPU pipeline factory + per-family dispatch/draw wrappers | `class Renderer`, `namespace Entry`, `dispatch_generate_patch_heights/_gradients/_cells`, `draw_patch_terrain_*`, `use_indirect_terrain` | state | SPINE | Y (patchTerrain/shadowPatchTerrain pipelines; heightfield-gen dispatch; finite→indirect gate) |
| `state.hpp` | L5 | CPU/GPU data contract: Dim consts, GPU DTOs, GPUState buffers | `namespace Dim` (PATCH_*, MAX_ACTIVE_PATCHES), `GPUState`, `set_world_bounds`, `GPUTileGrid(Entry)`, `GPUPatchGrid/Instance/Params`, `GPUDesignConfig` | demo | SPINE (DEAD WIRE dome_center_x ABI; 14 LATENT[gate-a-shared] = landed retire-recipes) | Y (PATCH_EXTENT/*RADIUS; GPUTileGrid; GPUPatchGrid; world_bound_min/max + set_world_bounds) |
| `world.wgsl` | L5 | the GPU cast: all VS/FS/compute kernels + the manifold interface | `manifold_resolve`, `manifold_position`, `manifold_height_hf`, `SurfaceHit`/`Boundary`, `query_ground_*`, `terrain_normal_at`, `ground_formed_with_complexity`, `sample_terrain_y_at`; entry pts `patch_terrain_vs`, `generate_patch_heights/gradients/cells`, `compute_entity_placement` | N/A (GPU leaf cast, loaded as string) | SPINE — **hosts the RESIDUE** (M6 SDF block: TerrainState + lipschitz chain + legacy WAVES + freeze/amp feeder + sphere-tint wire; + genuinely-dead: complexity texel channel, density-lattice no-op, ActivePatch::animated, binding 144) | Y (IS the terrain — see §2) |
| `demo.hpp` | L5 | compile-time demo selector; folds ROSTER from a matrix column | `DEMO`, `ROSTER`, `INCUBATE_DEMO`, `T7B_DEMO_COL` token-paste | matrix | SPINE | N |
| `matrix.hpp` | L5 | pieces×demos boolean grid → constexpr Roster/DemoConfig | `GRID[Piece::COUNT][DemoCol::COUNT]`, `Piece`, `DemoCol`, `column_to_roster`, `demo_config` | demo_config, mood_constants | SPINE | Y (THE SURFACE = always-on foundational row, not a tickable Piece — deferred body-tickable cut noted) |
| `cartridge.hpp` | L4 | composition root / driver: owns all state, drives frame | `class Cartridge` (`initialize`, `update`, `render`), `FAMILY_DISPATCH` hub, `dispatch_prepare_mesh_*` | ALL (composition root, 30 internal) | SPINE (27 SEAM[spine:*], all named/resolved; ROSTER-RESIDUE gol instrumentation) | Y (finite_mode→set_world_bounds; init_patch_system; owns Patch/World/TileWorld state) |

**RESIDUE-LOCI (intra-module, per M6 — nothing to delete at file granularity):**
`world.wgsl` — SDF-RESIDUE block (excavate whole: TerrainState buffer+struct, lipschitz chain @5415, legacy WAVES table, wave_enable/freeze/frozen_t, amplitude feeder, binding 20/220) + ONE entangled wire (`terrain_state.tint` dead store inside live `update_sphere` + `coupling_sphere_to_terrain_tint`, surgical); GENUINELY-DEAD (delete): complexity texel channel, density-lattice no-op, `ActivePatch::animated`, RENDER_RADIUS aliases, binding 144. `orbs.hpp`/`state.hpp` — `dome_center` DEAD WIRE (ABI ballast). `tile_world.hpp` — `activation_scale` DORMANT-VOICE (revive-later, not dead). `ground_architecture.hpp` — POLICY_PLACEMENT_* + *_gradient = LATENT-SCAFFOLD (KEEP: the manifold's landing site).

## SECTION 2 — TERRAIN MAP (terrain-touching modules → manifold anatomy)

Legend: **IFACE** = manifold_resolve/SurfaceHit/position · **BASE** = base-shape cast (lattice+tile+piers+pyramids) · **W-tx/W-vs/W-nb/W-mi** = the four welds (texel-storage / mesh-VS / normal-basis / movement+index) · **OVL** = overlay fold (GoL/waves/pulses/aura) · **SMP-b/SMP-a/SMP-c** = the three samplers (baked / analytic-live / CPU-estimate). ⚑ = spans >1.

| module | IFACE | BASE | W-tx | W-vs | W-nb | W-mi | OVL | SMP | span? |
|---|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| `world.wgsl` | ● decl+consumers | ● bodies | ● gen kernels | ● patch_terrain_vs | ● terrain_normal_at | ● xz→idx | ● fold in height eval | ● b+a | **⚑ ALL** |
| `ground_architecture.hpp` | ● POLICIES[] fold + POLICY_* mirror | ● contributor registry | | | | | | | ⚑ IFACE+BASE |
| `tile_world.hpp` | | ● TileShape amp/bias/archetype | | | | | | ● SMP-c (F1 estimate) | ⚑ BASE+SMP-c |
| `entity_pipeline.hpp` | | ● pyramid bake→regen | | | | | | | (also S3 place) |
| `patch_system.hpp` | | | ● generate_patch_batch, mark_regen | | | ● center-pin/floor idx | | | ⚑ W-tx+W-mi |
| `state.hpp` | | | ● GPUPatchGrid rgba16f | ● GPUPatchInstance/Params | | ● Dim::PATCH_* idx, set_world_bounds | | | ⚑ W-tx+W-vs+W-mi |
| `renderer.hpp` | | | ● gen-height/grad/cells pipelines | ● patchTerrain/shadow pipelines | | | | | ⚑ W-tx+W-vs |
| `render_passes.hpp` | | | | ● draw_patch_terrain LOD0/1 + shadow | | | | ● SMP-b (upload_ground) | ⚑ W-vs+SMP-b |
| `surface_services.hpp` | | | | | | ● ActivePatch/WorldState streaming | | | (S1/S2 lifecycle) |
| `pawn.hpp` | | | | ● aura_height→terrain VS extrude | | | ● aura influence grid | | ⚑ W-vs+OVL |
| `gol_zones.hpp` | | | | | | | ● per-cell height extrude | | (OVL) |
| `ribbon.hpp` | | | | | | | ○ rides surface (floor) | ● SMP-c (estimate) | ⚑ OVL+SMP-c |
| `entity_types.hpp` | ○ ground_y_offset/gpu_ground_y | | | | ○ ground_y correction | | | | (DTO glue) |
| `spawn_engine.hpp` | | | | | | | | ● SMP-c consumer (F3/F4) | (S3 place) |
| `mood.hpp` | | | | | | ● finite box bmin/bmax → shell | | | (W-mi bound) |
| `cartridge.hpp` | | | | | | ● finite_mode→set_world_bounds | | | (W-mi driver) |
| `input.hpp` | | | | | | ● active_radius clamp | | | (W-mi driver) |
| `spine_state.hpp` | | | | | | ○ finite_radius/amp_ceiling | | | (contract) |
| `point.hpp` | ○ host terrain rule (SNAP/SOFT_FLOOR) | | | | | | | | (consumer) |
| `gallery.hpp` | | | | | | | | ● SMP-c (tile_archetype) | (TERRAIN_QUAD) |

**Spanner findings:** (1) `world.wgsl` is the whole manifold — interface, cast, all four welds, overlay, and two of three samplers live in one file (the Stage-3 blast radius). (2) The **four welds are smeared across 6 C++ modules** (patch_system, state, renderer, render_passes drive W-tx/W-vs; mood/cartridge/input drive W-mi) — the "welds are the real cost" claim is confirmed structurally: no weld is single-module. (3) **W-mi (movement+index) is the most diffuse weld** — 5 modules touch patch indexing / finite bounds. (4) The **three-sampler divergence** is real and cross-module: SMP-b (render_passes/world), SMP-a (world only), SMP-c (tile_world authors, ribbon/spawn/gallery consume) — b2b's agreement flip would have to reconcile authors in `world.wgsl` + `tile_world.hpp`. (5) IFACE currently has exactly **2 authors** (world.wgsl body + ground_architecture contract) and its consumers are all inside world.wgsl — the frozen signature's surface is genuinely small, which is what makes the sphere plug-in cheap above the welds.
