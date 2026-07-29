# LADDER-6 PHASE R — THE GROUPING CENSUS (report-first; nothing moved)

Scope: the world-engine block (post-C2 chapter map, cartridge.hpp now
2,902 lines / 2,068 code) + the two class-body hubs. Recipes:
per-chapter identifier harvest + organ-reach scan + tree-wide inbound
call census (grep, counts include definitions).

## (a) THE REACH CENSUS

| chapter | code | state owned | reaches (outbound) | inbound (who calls) |
|---|---|---|---|---|
| ACTIVE PATCH SYSTEM | 170 | ActivePatch, patches_[64], WorldState (struct), freeLayerStack_, GRID/RENDER radii | FAMILY_DISPATCH (evict routing), entities_state_ (audit counts), player_, world_state_ | find_patch: **14 external sites across 7 owner impls**; evict/audit: internal only |
| DYNAMIC BUDGETS | 32 | budget constants | patches_, world_state_, inputState_ | internal only |
| TILE WORLD SYSTEM | 28 | ARCHETYPES tables | (pure vocabulary) | generate_tile_state |
| POPULATION THEMES | 174 | PopulationTheme, THEMES, ThemeEnvelope, themeEnvelope_, active_theme_idx_ | self-contained | tile gen + spawn preamble (via tile entries) + **9 family adapters (Cartridge::THEMES — the INTENT[services:themes] readers)** |
| TERRAIN TOKENS | 206 | TerrainToken, terrainTokens_[], TileState, tileCache_, TERRAIN_EMISSION | THEMES, themeEnvelope_, active_theme_idx_, gpuState_, mood_state_/MOOD_TABLE, world_state_ | spawn preamble reads tileCache_ (theme_spawn, entity_density); estimate_terrain_height reads it |
| TOKEN TICK + EMISSION | 48 | — | terrainTokens_, tileCache_ | stream_patches |
| WORLD LIFECYCLE | 144 | — | **17 organs** (the teardown conductor: every owner clear + patch/tile/token/theme resets + gen bump) | update() transition machine |
| PATCH SETUP / GEN / ALLOC / VISIBILITY / STREAM HELPERS | 224 | PatchCandidate, LOD radii | patches_, world_state_, gpuState_, renderer_, ribbon_state_ (late tip registration), active_theme_idx_ | internal only |
| stream_patches (in PUBLIC) | 277 | — | drives 16 of the above functions (map below) | render() |

**The decisive finding: the S2 machinery is CLOSED.** Every function
above is internal-only except four cross-module surfaces:
1. `find_patch` (+ `ActivePatch::record_entity`) — 14 sites in 7 owner
   impls: S3 commits registering with the patch registry.
2. `estimate_terrain_height` / `terrain_tile_warm` — ribbon (S4) +
   spawn_engine (S3) sampling the surface approximation.
3. `mark_patches_for_regen` — entity_pipeline + spawn_engine: S3
   stamping the surface (the occupier face's embryo, theory v2 §3b).
4. `Cartridge::THEMES` — the nine family adapters (the tagged reader).

## (b) THE MODULE GROUPING PROPOSAL

The census argues TWO deviations from the seed hypothesis, both on §8
minimality:

**S2-1 · patch_system** (~700 code with stream_patches; the surface's
lifecycle): ACTIVE PATCH SYSTEM + DYNAMIC BUDGETS + PATCH SETUP +
PATCH GENERATION + LAYER ALLOCATOR + VISIBILITY + STREAM HELPERS +
stream_patches. One owned decision: WHICH PATCHES EXIST — generation,
allocation, streaming, eviction, visibility, budgets. The seed's
four-way split would make layer_alloc an 18-line module and
visibility a 10-line one — no owned decision apiece; their constants
and functions are one machine around patches_/freeLayerStack_.
State: PatchSystemState (patches_, freeLayerStack_, PatchCandidate
scratch). find_patch/record_entity/evict remain its public surface.

**S2-2 · tile_world** (~282 code; terrain memory): TILE WORLD
(archetypes) + TERRAIN TOKENS + TOKEN TICK — **the seed's
terrain_tokens merges in.** The census shows the interlock is
bidirectional: generate_tile_state consumes tokens to bias archetype
selection AND rolls emission to write tokens; tileCache_ and
terrainTokens_ are declared in one chapter and read/written by the
same three functions. Split, each half owns half a decision; together
they own one: WHAT THE TERRAIN REMEMBERS. State: TileWorldState
(tileCache_, terrainTokens_). Public surface: generate_tile_state,
tick_terrain_tokens, evict_distant_tiles, upload_tile_grid_now,
estimate_terrain_height, terrain_tile_warm (the last two migrate here
from spawn_engine at its conversion — they are tile-cache reads).

**S2-3 · population_themes** (~174 code; as seeded): THEMES +
PopulationTheme + ThemeEnvelope + the three theme functions. State:
ThemesState (themeEnvelope_, active_theme_idx_). THEMES graduates
with it; INTENT[services:themes] retires by its own condition; the
nine adapter reads re-path from Cartridge::THEMES to the module's
vocabulary.

**Three rulings the stamp must resolve:**
- **R-a WorldState / world_state_**: recommend ROOT ORGAN (like
  time_state_/player_) — it is the frame's identity (gen counter,
  radius, center, finite mode) read by every stratum tree-wide
  (100+ sites); the machinery moves, the identity stays.
- **R-b teardown_world (144 code)**: recommend patch_system residency
  in keyhole form (the apply_mood precedent: a conductor that touches
  every organ can live module-side) — it is S2's reset verb, invoked
  by the root transition machine. The alternative reading ("boot/
  teardown stay root by definition") would keep it in PUBLIC; the
  census notes the §2 exclusion names CARTRIDGE boot/teardown, and
  teardown_world is the WORLD's.
- **R-c stream_patches**: recommend moving WHOLE into patch_system as
  its per-frame conductor, with its two S3-trigger calls
  (spawn_selected_patches → the select/place/commit loops; culling)
  named as a declared seam (SEAM[patch:spawn-trigger]) — keyhole
  calls, not include dependencies, so the arrow law holds. The
  alternative: leave the S3-trigger lines in render() and move only
  the S2 verbs (a smaller, uglier cut).

## (c) THE WELD MAP

**stream_patches' 277 lines drive:** collect_sorted_patches ×5,
generate_selected_patches ×2, spawn_selected_patches ×2 (→ S3 loops),
evict_patch, alloc_layer ×2, budgets, tick_terrain_tokens ×2,
generate_tile_state ×3, evict_distant_tiles, upload_tile_grid_now,
update_entity_draw_visibility (S3/S4 culling), audit_entity_integrity,
flush_pier_count (piers), in_render/priority_window, patch_distance_sq.
All S2-internal except the three named S3/S4 touches.

**Worldgen/readback boundary:** the P5 machines (root, standing) guard
against stale worlds via world_state_.world_gen capture — the boundary
is ONE counter, which stays with WorldState (root organ per R-a).
Clean under any grouping.

**spawn_engine's S3 seams into S2:** run_spawn_preamble reads
tileCache_ entries (theme_spawn, entity_density) and active_theme_idx_
— becomes two reads through tile_world/themes state via the keyhole;
negotiate_position/footprints stay S3-owned; mark_patches_for_regen is
S3 stamping S2 (moves to patch_system; spawn_engine calls it via
keyhole); write_pier/clear_pier/flush_pier_count + cpuPiers_ — piers
are terrain-raising volumes (surface authoring): recommend they ride
patch_system (S2) at spawn_engine's conversion; flag for the stamp.

**Standing exclusions (honored, no nomination):** the census found no
argument to move the transition machine (K4) or the P5 readbacks —
both are pure orchestration closures over root state; the tags stand.

## (d) THE HUBS POST-LADDER-5

- **spawn_engine.inl — 527 code lines** (was 550 pre-arc; MIN_SEPARATION
  joined, recipes left). S3 core: queues + loops, preamble, negotiate,
  footprints, separation/proximity vocabulary, spawn gate. S2-resident
  services to redistribute at conversion: estimate_terrain_height /
  terrain_tile_warm (→ tile_world), mark_patches_for_regen
  (→ patch_system), the pier trio + cpuPiers_ (→ patch_system, per
  weld map). Conversion candidate: **S3, the proven pair pattern.**
- **entity_pipeline.inl — 899 code lines** (was 1,672 pre-arc). The
  generic three-phase verbs + the four welded family blocks
  (column+antenna, pyramid, arch) + shared helpers. Conversion
  candidate: **S3.** The welded four stay in it (their welds are to
  S2 services reachable via keyhole).

## THE MAP TO STAMP (summary)

Three S2 modules — patch_system (+R-b, +R-c contents), tile_world
(tokens merged in), population_themes — then the two S3 conversions
(spawn_engine, entity_pipeline), then the §1 completion touch. Zero
class-body includes at close. Deletion-test dry run: under the merge,
Jean's named subject terrain_tokens becomes **tile_world** (or
population_themes as the cleanest alternative). Rulings owed: R-a,
R-b, R-c, piers. Nothing moves until the stamp returns.
