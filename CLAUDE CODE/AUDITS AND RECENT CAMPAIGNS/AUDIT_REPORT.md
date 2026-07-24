# GROUND CARD PRE-CAMPAIGN AUDIT — REPORT

```
Audit base (trunk)     : bd405d927a3de8ae47da9b719ad441a31e5c326e
                         = COUPLING_SAGA_SWEEP_CHECKERS HEAD, pushed 2026-07-21
                         00:21 UTC ("DOOR AXES: Movement 1's graduated signed
                         ranges"), republished as branch CLOSURE_GPU per the
                         owner's instruction (the local branch of this lineage
                         is named CLOSURE_GPU; no branch of that name existed
                         on the GitHub remote before this audit).
Snapshot reference     : f8940079f44a56f5d03f93d655232487c913e48f
                         = COUPLING_SAGA_SWEEP HEAD (the CC-1 recipe's named
                         reference point for the design-session snapshot).
Audit branch           : CLOSURE_GPU_AUDIT  (cut from CLOSURE_GPU HEAD; also
                         mirrored to claude/ground-card-audit-exb0m5, the
                         session's designated remote branch)
Probe branches         : CLOSURE_GPU_CARD_AUDIT_A / _B / _C — throwaway, cut
                         from the audit branch, never merged, kept local; their
                         diffs and per-commit results are preserved under
                         audit/ so each is reproducible anywhere.
Audit date             : 2026-07-21
glaw1 (compile gate)   : audit/tools/glaw1/run.sh — runs IN this container
                         (g++ -fsyntax-only over the real TU with stub SDK).
                         Baseline: G-LAW 1: GREEN in 24.7 s.
Shader-side witness    : headless Chromium WebGPU = real Dawn/Tint
                         (Vulkan/SwiftShader; adapter google/swiftshader,
                         limits 10 storage / 12 uniform / 4 storage-tex per
                         stage). Harness: audit/probe_dawn_witness.mjs.
No fixes applied anywhere. Findings only. The audit branch adds only
AUDIT_REPORT.md and audit/ artifacts; no trunk source file is modified.
```

**Witness DELTA (applies to CC-3/4/5).** The native build is Windows-only
(CMakeLists.txt pins `C:/dev/dawn`, MSVC `.lib`s), so "Build (glaw1). Boot."
decomposes here into: glaw1 = the tree's own in-container compile gate (run
as-is, verbatim results below), and Dawn validation = pipeline creation
against layouts machine-parsed from realization/state.hpp (registry-resolved
constants, per-entry view dimensions), executed in Chromium — which *is*
Dawn, on the Vulkan/SwiftShader backend, not the target D3D12/FXC device. A
full boot and FXC timing remain on-device work. world.wgsl (11,942 lines)
compiles **clean** in this Tint: 0 diagnostics, 116–336 ms; all 17 compute
pipeline families validate against the reconstructed baseline layouts (also
proving the reconstruction is faithful).

**Encoding law: FAIL (scoped).** LF-only holds tree-wide (0 CRLF in 224 text
files). BOM law is violated by **7 files, all inside the stale copy
`src/cartridges/backup_board/`**: renderer.hpp and modules/{agents, entity_pipeline,
render_passes, pawn, musical, orbs}.inl each begin with a UTF-8 BOM. The live
cartridge (`the_board/`), audit/, and web/ are BOM-free. Recipe: python walk
testing `\xEF\xBB\xBF` prefix and `\r\n` over src/, audit/, web/ + root files.

---

## [CC-1] DRIFT GATE — snapshot vs trunk

**VERDICT: DELTA (enumerated, absorbable).** All 19 snapshot files exist at
HEAD (paths below). Diffing the CC-1 reference point (COUPLING_SAGA_SWEEP
HEAD f8940079) against the audit base (bd405d92): **16 of 19 files changed**
(~1,405 changed lines); only `contracts/ground_architecture.hpp`,
`realization/drawable_table.hpp`, and `contracts/wgpu_fwd.hpp` are IDENTICAL.
Recipe: `git diff f8940079..bd405d92 -- <file>` per file; hunks touching
bindings / layouts / Dim:: / contributor-query functions / spine tables
extracted verbatim to `audit/cc1_hunks_state.txt`, `audit/cc1_hunks_wgsl.txt`,
`audit/cc1_diff_binding_registry.patch`, `audit/cc1_diff_spine_state.patch`.

All 19 files, located (base directory `src/cartridges/the_board/`):

| File | Path | SWEEP→HEAD |
|---|---|---|
| cartridge.hpp | `cartridge.hpp` | +203/−143 (summary: roster/mood/movement-1 close-out; no binding content) |
| renderer.hpp | `realization/renderer.hpp` | +7/−7 |
| render_passes.hpp | `realization/render_passes.hpp` | +1/−1 |
| state.hpp | `realization/state.hpp` | +198/−116 — **18 hunks touch layouts/Dim/bindings** (see cc1_hunks_state.txt) |
| world.wgsl | `realization/world.wgsl` | +742/−488 — **9 hunks touch bindings/contrib/query fns** (see cc1_hunks_wgsl.txt) |
| patch_system.hpp | `surface/patch_system.hpp` | +11/−11 |
| population_themes.hpp | `surface/population_themes.hpp` | +39/−4 |
| tile_world.hpp | `surface/tile_world.hpp` | +6/−6 |
| entity_pipeline.hpp | `machine/entity_pipeline.hpp` | +51/−62 |
| spawn_engine.hpp | `machine/spawn_engine.hpp` | +66/−38 |
| ground_architecture.hpp | `contracts/ground_architecture.hpp` | IDENTICAL |
| binding_registry.hpp | `realization/binding_registry.hpp` | +16/−11 — **quoted in full below** |
| drawable_table.hpp | `realization/drawable_table.hpp` | IDENTICAL |
| terrain_looks.hpp | `surface/terrain_looks.hpp` | +27/−3 |
| surface_services.hpp | `contracts/surface_services.hpp` | +7/−7 |
| wgpu_fwd.hpp | `contracts/wgpu_fwd.hpp` | IDENTICAL |
| point.hpp | `contracts/point.hpp` | +5/−5 |
| spawn_services.hpp | `contracts/spawn_services.hpp` | +11/−13 |
| spine_state.hpp | `contracts/spine_state.hpp` | +20/−19 (spine tables — full diff in cc1_diff_spine_state.patch) |

Binding-relevant drift the plan report must absorb (verbatim in the patch
files; the load-bearing lines):

```
-                inline constexpr uint32_t cell_fields_write          = 29;
+                // (cell_fields_write = 29 RETIRED — Commit C, the LUT retirement.
+                //  Number reserved; do not reuse.)
...
+                inline constexpr uint32_t cmg_config                 = 190;  // DesignConfig view for the cmg kernel (the ceiling gate)
+                inline constexpr uint32_t cmg_column_ground          = 191;  // read-only column_ground view (the terrain delta)
...
-                inline constexpr uint32_t cell_fields_read           = 30;
+                // (cell_fields_read = 30 RETIRED — Commit C. Number reserved.)
```

and in world.wgsl:

```
+const TILE_GRID_CAPACITY: u32 = 1024u;
 struct TileGrid {
-    side: u32,             // grid dimension (up to 17)
+    side: u32,             // grid dimension (up to 19)
-    entries: array<TileGridEntry, 289>,
+    entries: array<TileGridEntry, TILE_GRID_CAPACITY>,
```

So: if the design session's snapshot was taken at (or near) SWEEP, its plan
still assumes `cell_fields_write/read` exist (g0:29 / g1:30 — both RETIRED at
HEAD), does not know the `cmg_config`/`cmg_column_ground` rebirths at 190/191,
and has a 289-entry TileGrid where HEAD authored `TILE_GRID_CAPACITY = 1024`.
One structural note: `src/cartridges/backup_board/` is a full stale copy of
the old-format cartridge (BOM-infected, see header) that shadows every grep —
census commands below exclude/report it separately.

---

## [CC-2] TREE-CLOSURE — the evictee consumer table

**VERDICT: PASS.** C++ touches the six buffers only through creation /
upload / bind-entry (plus creation null-checks); the WGSL reader sets close
exactly onto the claimed funnel. Zero OTHER consumers. Commands + verbatim
hits: `audit/cc2_grep_log.txt`.

### (a) Actual C++ handles (realization/state.hpp — spec names not trusted)

| Resource | Handle | Declared | WGSL var / binding |
|---|---|---|---|
| pier instances | `pierBuffer_` | state.hpp:1477 | `pier_instances` g0:26 |
| tile grid | `tileGridBuffer_` | state.hpp:1493 | `tile_grid` g0:25 |
| pyramid instances | `pyramidInstancesBuffer_` | state.hpp:1544 | `pyramid_instances` g0:30 |
| zone config | `zoneConfigBuffer_` | state.hpp:1562 | `zone_config` g0:160 (+ g1:32 `zone_params` FS read) |
| zone life | `zoneLifeBuffer_` | state.hpp:1564 | `zone_life` g0:161 |
| patch grid | `patchGridBuffer_` | state.hpp:1489 | `patch_grid` g0:152 (world.wgsl:8462) |

### (b) Full-tree grep census

`grep -rn "<handle>" src CMakeLists.txt --include=*.hpp --include=*.cpp
--include=*.inl --include=*.wgsl` per handle: **every hit lands in
realization/state.hpp** (plus the stale backup_board copy, reported
STALE-COPY, 58 hits). Classification (all hits, live file):

| Handle | creation | upload (`WriteBuffer` / `writeStruct` / `writeSlot`†) | bind-entry | OTHER |
|---|---|---|---|---|
| `pierBuffer_` | 1477, 2903, 2947 | 1919, 5734 | 4652, 4944, 4976, 5244 | none |
| `tileGridBuffer_` | 1493, 2902, 2947 | 1837 | 4647, 4763, 4940, 4972, 5065, 5241, 5318 | none |
| `pyramidInstancesBuffer_` | 1544, 3340, 3344 | 2537, 3348 | 4657, 4951, 5247 | none |
| `zoneConfigBuffer_` | 1562, 3379, 3391 | 2747, 2759, 2768, 3419 | 4662, 4870, 5168, 5252 | none |
| `zoneLifeBuffer_` | 1564, 3387, 3391 | 2787–2799 (7×) | 4666, 5171, 5255 | none |
| `patchGridBuffer_` | 1489, 2909, 2948 | 1845 | 4685, 5130, 5185 | none |

† `writeStruct`/`writeSlot` (state.hpp:1705/1709) are thin
`queue.WriteBuffer` wrappers — verified, classified as upload.

Registry constants: `bind::g0::{pier_instances, tile_grid, pyramid_instances,
zone_config, zone_life, patch_grid}` — 47 hits, ALL of them layout-entry or
group-entry lines in realization/state.hpp (the registry doing exactly its
job); `bind::g1::zone*` read-side constants likewise confined to the two
texture-layout blocks.

### (c) WGSL reader closure (recipe: `audit/cc4_wgsl_static_usage.py` —
call-graph transitive closure, comment-stripped, local-shadow-aware)

- `pier_instances` → read ONLY in `structure_height_at`; reached via
  `contrib_static_base_at` (live/query side) and
  `ground_formed_with_complexity` (bake side, sole caller
  `generate_patch_heights`).
- `pyramid_instances` → read ONLY in `contrib_pyramids_at`.
- `zone_config` / `zone_life` → `contrib_gol_zones_at` + the GoL kernels
  (`zone_gol_sync/evolve/mesh_gen`, `zone_derive_params`,
  `zone_mesh_gen_cell`).
- `tile_grid` → `evaluate_cell_fields`, `tile_grid_lookup`,
  `tile_modifiers_at` (color path + terrain-modifier path).
- `patch_grid` → `sample_terrain_y_at` only (readers:
  `compute_photographer_vp`, `compute_entity_placement`).

This is the claimed three-contrib funnel, PLUS the bake-side entrance
(`generate_patch_heights → ground_formed_with_complexity`) which the plan's
"three functions" phrasing must absorb — it is a consumer by design (the
heightfield baker), not a leak. Full per-entry-point binding tables:
`audit/cc4_output.json` (66 entry points, 279 functions).

---

## [CC-3] PROBE A — the three dead bindings (compiler as witness)

**VERDICT: DELTA — claim 1 CONFIRMED, claims 2 and 3 REFUTED, with the two
hidden consumers named verbatim by Dawn.** Probe branch
`CLOSURE_GPU_CARD_AUDIT_A` removes each claimed-dead entry from BOTH the
layout array and the bind-group entries array (six blocks in
realization/state.hpp, arrays renumbered; diff: `audit/probe_a.patch`).
Recipe as run: edits → `sh audit/tools/glaw1/run.sh` → Dawn witness against
the probe branch's own re-parsed layouts (`audit/probe_a_branch_results.json`).

- **glaw1: GREEN** on the probe branch (C++ names/scopes stay clean under all
  three removals).
- **Dawn validation: exactly two failures.**

1. **Ribbon tile_grid (g0:25) + pier_instances (g0:26): DEAD — CONFIRMED.**
   `compute_ribbon_rings` statically uses only `ribbon_state` (120),
   `ring_xforms` (121), `head_poses` (122). The hardwire, verbatim
   (realization/world.wgsl:4979):

   ```wgsl
   let terrain_y: f32 = 0.0;  // Only flying ribbons now; no terrain-following needed.
   ```

   With both entries removed, `compute_ribbon_rings` validates clean.

2. **GoL Zone pier_instances (g0:26): NOT DEAD — REFUTED.** The GoL layout
   serves all five zone kernels (`zone_mesh_gen_layout()` aliases
   `zoneGolComputeLayout_`, state.hpp:2630), and the mesh-gen kernel reaches
   the pier reader through the baked-terrain sampler's fallback chain:
   `zone_gol_mesh_gen → zone_mesh_gen_cell → zone_sample_baked_terrain_y →
   query_ground_baked_heightfield → contrib_static_base_at →
   structure_height_at`. Dawn, verbatim — this output IS the hidden consumer:

   ```
   Binding doesn't exist in [BindGroupLayoutInternal (unlabeled)].
    - While validating that the entry-point's declaration for @group(0) @binding(26) matches [BindGroupLayoutInternal (unlabeled)]
    - While validating the entry-point's compatibility for group 0 with [BindGroupLayoutInternal (unlabeled)]
    - While validating compute stage ([ShaderModule "world.wgsl (audit witness)"], entryPoint: "zone_gol_mesh_gen").
   ```

   (`zone_gol_sync`, `zone_gol_evolve`, `zone_gol_mesh_reset`,
   `zone_derive_params` all validate clean without it — the claim is true of
   the life kernels, false of the shared layout.)

3. **Pawn Aura tile_grid (g0:25): NOT DEAD — REFUTED. The "archetype lookup"
   comment is NOT a fossil.** Path: `compute_pawn_aura →
   gol_composite_cell_color → evaluate_cell_fields` (which reads tile_grid).
   Dawn, verbatim:

   ```
   Binding doesn't exist in [BindGroupLayoutInternal (unlabeled)].
    - While validating that the entry-point's declaration for @group(0) @binding(25) matches [BindGroupLayoutInternal (unlabeled)]
    - While validating the entry-point's compatibility for group 0 with [BindGroupLayoutInternal (unlabeled)]
    - While validating compute stage ([ShaderModule "world.wgsl (audit witness)"], entryPoint: "compute_pawn_aura").
   ```

**Machine-found dead entries the plan does NOT list** (cross:
`audit/cc3_dead_entry_cross.py` → `audit/cc3_output.json`; each verified
removable-clean by Dawn in `audit/probe_results.json` probeA section):

| Layout | Dead entries (registry names) |
|---|---|
| Compute Entity Layout | 101 `trajectories`, 145 `photo_heightfield`, 146 `photo_sampler`, 152 `patch_grid` |
| Compute Texture Layout (g1) | 23 `nearest_sampler` |
| Photographer Compute Layout | 144 `photo_patch_instances` (its own comment at world.wgsl:8401 already says "unused — sample_terrain_y_at now reads patch_grid at binding 152") |
| Entity Placement Compute Layout | 60 `agent_state`, 144 `photo_patch_instances` |
| Frustum Cull Compute Layout | 60 `fc_agents`, 80 `fc_camera` |

---

## [CC-4] PROBE B — Dawn-enumerated consumers of the Compute Entity evictees

**VERDICT: PASS — the enumerated consumer set matches the plan's claimed set
exactly; no entry point outside it.** Probe branch
`CLOSURE_GPU_CARD_AUDIT_B`: five commits, each restoring baseline then
removing ONE evictee from the Compute Entity layout AND bind group (spec
order). Recipe per sub-probe: mechanical entry removal + renumber → glaw1 →
Dawn witness at that commit. glaw1: **GREEN ×5**. patch_grid (g0:152) not
probed per spec — though note CC-3's finding that at THIS base it is a dead
entry of the Compute Entity layout (the baked card's index is read by the
photographer/placement pipelines, not the agents).

Result matrix (Dawn pipeline creation, per sub-probe commit; verbatim error
for every FAIL in `audit/probe_b_branch_results.json` — 25 errors, all of the
four-line "Binding doesn't exist … entryPoint: <name>" shape):

| Removed | update_player_agent | update_other_agents | update_camera | update_sphere | update_cube | compute_vp |
|---|---|---|---|---|---|---|
| pyramid_instances (30) | FAIL | FAIL | FAIL | FAIL | FAIL | ok |
| tile_grid (25) | FAIL | FAIL | FAIL | FAIL | FAIL | ok |
| pier_instances (26) | FAIL | FAIL | FAIL | FAIL | FAIL | ok |
| zone_config (160) | FAIL | FAIL | FAIL | FAIL | FAIL | ok |
| zone_life (161) | FAIL | FAIL | FAIL | FAIL | FAIL | ok |

Representative verbatim (pyramid_instances sub-probe, first failure):

```
Binding doesn't exist in [BindGroupLayoutInternal (unlabeled)].
 - While validating that the entry-point's declaration for @group(0) @binding(30) matches [BindGroupLayoutInternal (unlabeled)]
 - While validating the entry-point's compatibility for group 0 with [BindGroupLayoutInternal (unlabeled)]
 - While validating compute stage ([ShaderModule "world.wgsl (audit witness)"], entryPoint: "update_player_agent").
```

Reading: **all five evictees are consumed by all five live-contributor
agents** (every agent runs the `query_ground_*` policy dispatch, which pulls
the full contributor set), and `compute_vp` consumes none. That is exactly
"update_player_agent / update_other_agents / update_camera / update_sphere /
update_cube / compute_vp subsets per the funnel" — with the subsets being
{all five agents} for every evictee, and compute_vp always empty. No foreign
entry point appeared in any error.

---

## [CC-5] PROBE C — FXC storage-texture tolerance

**VERDICT: DELTA — executed on Dawn/Vulkan/SwiftShader; the FXC half stays
on-device.** Kernels live on probe branch `CLOSURE_GPU_CARD_AUDIT_C`
(`audit/probe_c_kernels.wgsl` there) and are embedded in the harness, which
ran them here. The identical harness runs on the design machine in Chrome —
that run (or a native compile of the kernel file) is the remaining on-device
step that decides paint-card v2 residency.

(a) `readonly_and_readwrite_storage_textures`: **EXPOSED** on this Dawn
(`wgslLanguageFeatures` = packed_4x8_integer_dot_product,
unrestricted_pointer_parameters, pointer_composite_access,
readonly_and_readwrite_storage_textures). The
`texture_storage_2d<r32float, read_write>` kernel: **module 1.2 ms, pipeline
5.0 ms**. Caveat: the D3D12/FXC device's feature set must be queried there —
do not bank residency options on (a) until then.

(b) Card-writer patterns — all far under the ~10 s FLAG:

| Kernel | module | pipeline |
|---|---|---|
| ping-pong, core form (`texture_2d` read + r32float write) | 0.8 ms | 4.7 ms |
| ping-pong, feature form (r32float read + r32float write) | 0.8 ms | 4.8 ms |
| rgba16float write-only card writer | 0.5 ms | 4.8 ms |

Supporting datum for the FXC-hang family: the full 11,942-line world.wgsl
compiles + validates in 116–336 ms on this Tint front-end. The banner FXC
block (world.wgsl:44–56) was read before probing, per the boundary; its laws
(lean instance structs, uniform-bounded loops, no texture-array stamps in the
collision chain, one indirect draw per pass, 10/12 per stage) are untouched
by these standalone kernels.

---

## [CC-6] MACHINE BUDGET COUNTS

**VERDICT: PASS — machine counts match the plan's claimed pressure on all
five named layouts, except Ribbon which gained one storage buffer since the
plan was written.** Recipe committed: `audit/cc6_layout_budgets.py` (parses
all 23 `std::array<wgpu::BindGroupLayoutEntry, N>` blocks; binding constants
resolved from binding_registry.hpp; declared == parsed everywhere). Full
table: `audit/cc6_output.json`.

Per layout, per stage — storage bufs / uniform bufs / sampled tex / storage
tex / samplers:

| Layout (state.hpp line) | Compute | Vertex | Fragment |
|---|---|---|---|
| Compute Entity (3656) | **9/7**/1/0/1 | | |
| Render Entity (3755) | | **8**/6/1/0/0 | **7**/3/0/0/0 |
| Mesh Gen Entity (3854) | 0/1/0/0/0 | 0/1/0/0/0 | 0/1/0/0/0 |
| Shadow Texture (3873) | | 0/0/1/0/2 | 0/0/0/0/2 |
| Render Texture (3900) | | 0/0/2/0/2 | **1**/0/5/0/3 |
| Compute Texture (3962) | 0/0/1/0/2 | | |
| Terrain Index Gen (3989) | 1/0/0/0/0 | | |
| Patch Gen (4007) | 2/4/0/2/0 | | |
| Ribbon Compute (4057) | **3**/2/0/0/0 | | |
| Gallery Entity (4089) | | 1/1/0/0/0 | 3/1/0/0/0 |
| Gallery Texture (4117) | | 1/0/0/0/0 | 1/0/1/0/1 |
| Photographer Compute (4146) | 6/2/1/0/1 | | |
| Entity Placement (4205) | **9**/1/1/1/1 | | |
| Frustum Cull (4273) | 6/1/0/0/0 | | |
| GoL Zone (4317) | **7**/4/1/1/1 | | |
| Pawn Aura (4394) | 2/3/0/1/0 | | |
| Orb / Orb Copy (4435/4465) | 2/1 each | | |
| Arch/Palm/Cactus/Blade Mesh Gen | 3/0 each | | |
| Column Mesh Gen (4516) | 4/1/0/0/0 | | |

Against the plan's claims:

| Plan claim | Machine count | Verdict |
|---|---|---|
| Compute Entity 9 storage + 7 uniform | 9 + 7 | **PASS** |
| Entity Placement 9 storage | 9 | **PASS** |
| Render Entity 8 storage VS / 8 FS | VS 8; FS 7 + 1 (g1 `zone_params`) = 8 per stage | **PASS** |
| GoL 7 | 7 | **PASS** |
| Ribbon 2 | **3** (`head_poses` @122 added — ribbon head-path) | **DELTA (+1)** |

Banner law (storage 10 / uniform 12 per stage, world.wgsl:56): no layout
combination exceeds it. Headroom notes: Compute Entity compute stage sits at
9/10 storage; Entity Placement at 9/10; the render pass at 8/10 per stage.
Two spots exceed the WebGPU *default* 8-storage law (Compute Entity 9,
Entity Placement 9) — which is why console.hpp:179–185 requests full adapter
limits. On the witness adapter the ceiling is exactly 10: **one free storage
slot in the agents' compute stage** is what the live-card proposal has to
live inside — and note CC-3's finding that 4 of the Compute Entity layout's
current entries are dead (101/145/146/152), i.e. the *real* live pressure is
lower than the layout's footprint.

---

## [CC-7] C6 MIRROR CHECK + PROPOSED SLOT FREEDOM

**VERDICT: PASS (mirror clean; 31/34 free) with one stale banner number.**
Recipes: `audit/cc7_wgsl_binding_census.py` + `audit/cc7_mirror_cross.py`;
outputs beside them.

- Literal census of realization/world.wgsl: **102 @group/@binding
  declarations** (89 in g0, 13 in g1). Raw `@binding(` occurrences: 112 — the
  10 extras are all comments (2 removal notes at lines 5270/5333, 8
  orb-section doc lines at 11108–11117). Raw `@group(` count: 102. **The
  banner's "108" (binding_registry.hpp:28) matches neither count — stale
  number**, presumably pre-dating the terrain_state/cell_fields removals and
  orb consolidation.
- Registry mirror (by (group, number) AND name): registry has **97
  constants / 97 slots**; WGSL has 102 declarations over **97 slots**; every
  registry constant has a literal and every literal a constant — **zero
  orphans in either direction**. The 5 extra WGSL declarations are the
  documented frustum-cull aliases sharing slots (`fc_config`@g0:1,
  `fc_vp`@g0:2, `fc_agents`@g0:60, `fc_camera`@g0:80, `fc_patches`@g0:340);
  the registry carries them as "aka" comments on the canonical names, so they
  surface as name-mismatches-by-design, not orphans.
- **Proposed slot freedom: g0 binding 31 — FREE in both rooms** (no WGSL
  literal, no registry constant; neighbors: 30 `pyramid_instances`, and g0:29
  is RETIRED-reserved "do not reuse"). **g1 binding 34 — FREE in both rooms**
  (neighbors: g1:31 `zone_life_read`, 32 `zone_params`, 33 `pawn_aura_read`).
  `live_card_write` @ g0:31 / `live_card_read` @ g1:34 collide with nothing.

---

## [CC-8] STALE-TEXT CONFIRMATIONS (cleanup-fold seeds)

**VERDICT: (a) CONFIRMED + one LIVE BUG sibling; (b) CONFIRMED;
(c) CONFIRMED (narrowed); (d) NOT STALE — the comment is live, refuting the
cleanup seed.** No edits.

**(a) "225x…" labels vs Dim::MAX_ACTIVE_PATCHES = 289 — CONFIRMED.**
`realization/state.hpp:86`:

```cpp
constexpr uint32_t MAX_ACTIVE_PATCHES = PATCH_PREGEN_SIDE * PATCH_PREGEN_SIDE; // 289
```

Stale sites (tree-wide grep "225", stale backup_board excluded):

- state.hpp:3538 `desc.label = "Patch Heightfield Array (225x256x256, RGBA16Float)";`
- state.hpp:3557 `desc.label = "Patch Cell Color Array (225x16x16, RGBA8Unorm)";`
- renderer.hpp:517 `// ceil(MAX_ACTIVE_PATCHES / 64) = ceil(225/64) = 4`

**LIVE BUG (finding, not just stale text):** the comment at renderer.hpp:517
sits on `dispatch_frustum_cull`, which hardcodes
`pass.DispatchWorkgroups(4, 1, 1)` (renderer.hpp:518). With
MAX_ACTIVE_PATCHES now 289 and `@workgroup_size(64)`
(world.wgsl:8796), 4 workgroups = 256 threads: patch slots 256–288 are never
processed by `frustum_cull_patches` when the active set fills (the kernel
guard at world.wgsl:8799 bounds by `fc_config.placement_patch_count`, which
can reach 289). Correct count is ceil(289/64) = 5. Goes to the cleanup
campaign's disposition table as a code fix, not a comment fix.

**(b) §2.2 ROW 7 "cycles per world unit" vs radians-per-unit — CONFIRMED.**
The ROW 7 panel exists (`── ROW 7 — THE MOVEMENT THIRD ──`,
world.wgsl:1721). Comment, verbatim (world.wgsl:1752):

```
//   freq       Spatial frequency (cycles per world unit). Higher = tighter ripples.
```

Application, verbatim (world.wgsl:2723–2725, same pattern at 2758 in the
second evaluator):

```wgsl
        let spatial  = bp.freq * dot(bp.dir, world_xz);

        h += blend * bp.amp * sin(spatial + temporal);
```

No 2π between stored freq and sin() — freq is applied as **radians per world
unit** (freq = 1.0 → wavelength 2π wu). The temporal term does get its 2π.

**(c) Ribbon terrain-following comments — CONFIRMED, narrowed.** The kernel
banner is now honest ("flying ribbons; no terrain follow", world.wgsl:4957)
and hardwires `terrain_y = 0.0` (4979, quoted in CC-3.1). Remaining
terrain-following residue:

- state.hpp:727 `float terrain_y;           // tile-modified terrain height ( 4) = 48` — the `GPURibbonRingTransform` field the kernel always zeroes;
- state.hpp:722 struct banner `(compute pass output, VS + update_world input)` — `update_world` no longer exists (split entry points);
- the layout/group header comment (state.hpp:4053–4055) still enumerates
  `tile_grid @25, pier_instances @26` as ribbon bindings — the dead pair
  probe A removes;
- WGSL mirror `RibbonRingTransform.terrain_y` (world.wgsl:889 comment
  "tile-modified terrain height at center XZ").

**(d) Pawn Aura "archetype lookup" comment — PREMISE REFUTED.** The comment
exists exactly where claimed (state.hpp:4418):

```cpp
entries[5].binding = bind::g0::tile_grid;   // tile_grid (uniform — evaluator's archetype lookup)
```

but it is **not** a fossil: `compute_pawn_aura → gol_composite_cell_color →
evaluate_cell_fields` reads tile_grid, and Dawn refuses the pipeline without
the entry (CC-3.3 verbatim error). This seed must NOT land in the cleanup
fold as a removal — neither the comment nor the entry is dead.

---

## Artifact index (all committed on the audit branch)

| Path | Role |
|---|---|
| `audit/cc1_diff_binding_registry.patch`, `cc1_diff_spine_state.patch` | CC-1 verbatim diffs (bindings, spine tables), SWEEP→HEAD |
| `audit/cc1_hunks_state.txt`, `cc1_hunks_wgsl.txt` | CC-1 filtered hunks (layouts/Dim/contrib/query) |
| `audit/cc2_grep_log.txt` | CC-2 exact commands + verbatim hits |
| `audit/cc4_wgsl_static_usage.py` / `cc4_output.json` | static-usage analyzer + 66-entry-point binding tables |
| `audit/cc3_dead_entry_cross.py` / `cc3_output.json` | layout × usage dead-entry cross |
| `audit/cc6_layout_budgets.py` / `cc6_output.json` | CC-6 counter (registry-resolving) + full table |
| `audit/cc7_wgsl_binding_census.py` / `cc7_output.json` | CC-7 literal census |
| `audit/cc7_mirror_cross.py` / `cc7_mirror_output.json` | CC-7 registry mirror |
| `audit/probe_dawn_witness.mjs` / `probe_results.json` | Dawn witness harness + baseline/probe raw results |
| `audit/probe_a.patch` / `probe_a_branch_results.json` | Probe A branch diff + its glaw1/Dawn run |
| `audit/probe_b_branch_results.json` | Probe B per-sub-probe verbatim Dawn errors (25) |

Probe branches exist locally in the audit container only (throwaway per
spec); everything needed to re-create them is in the artifacts above.
