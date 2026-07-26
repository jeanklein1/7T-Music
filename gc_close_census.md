# GROUND_CARD_CLOSE — HANDOFF A: THE CLOSING CENSUS

**Mode:** READ-ONLY. No source edits made. No commits to master.
**Tree:** `jeanklein1/7t-pawns`, single cartridge `src/cartridges/the_board`.
**HEAD at census time:** `8d90d44` — *DESIGNER_CAMERA [3A]: compare — Family B, on the pawn's shape*
**Report branch:** `claude/ground-card-census-7tl6tu` (transient review branch).
**Campaign read:** `src/docs/old docs/ground_card_campaign_v2.md` (identical copy at `audit/ground_card_campaign_v2.md`).

Recipe convention: every count below carries the exact command that produced
it, inline, run from the repo root. All `file:line` are against `8d90d44`.

---

## PROVENANCE — re-verified against master, 2026-07-26

This report was written against `8d90d44` and then merged to `master`. The
audited tree and current master carry an **identical source tree**:

```
git diff --stat 8d90d44 origin/master                       # → gc_close_census.md only
git diff --stat 8d90d44 origin/master -- src/ CMakeLists.txt tools/ assets/ audit/
                                                            # → empty (zero source-path changes)
git merge-base --is-ancestor 8d90d44 origin/master           # → true
```

The report file itself is the only delta, so every `file:line` below still
resolves. Machine re-verification run on the master checkout:

| check | result |
|---|---|
| all quoted source lines re-resolved byte-exact | **609 / 609**, 0 mismatched |
| CC-6 budget recount | identical; `flags` = `[]` |
| binding mirror census | 95 decls / 92 slots vs 92 constants / 92 slots, 0 orphans |
| entry-point closure | 64 / 64, symmetric difference 0 |
| §Q3 ABSENT verdicts re-greped | unchanged |
| headline finding re-greped | `config.indoor_height_cap` readers = **0** |

**A caveat on the shallow clone.** The container clones at `--depth=50`. Before
`git fetch --unshallow`, `master` and `origin/master` appear to have *no common
ancestor* (50 commits each side, `git merge-base` exits 1) and a fetch reports a
`forced update`. That is entirely an artifact of the truncated history — after
deepening, `60818b0` is a plain ancestor of `8d90d44` (845 → 940 commits
visible). **Deepen before drawing any ancestry conclusion in this environment.**

### Correction log (counts amended after machine re-verification)

Eleven stated counts did not reproduce their own recipes and have been corrected
in place. **No verdict changed** — every verdict rests on a hit list that was
opened and classified line by line; the defects were in the tallies and in one
recipe that was written but never executed.

| § | was | is | cause |
|---|---|---|---|
| anchor-1 | 16 `GOL_ZONE_*` | **17** | miscount |
| anchor-1 | 21 `grid_size` | **23** | miscount |
| anchor-2 | "16 + 21 + 16 hit lines" | **31 + 22 + 16** | the three greps were never separately tallied |
| anchor-7d | 28 card sites | **31** (2 defs, 2 comments, 27 calls) | miscount |
| anchor-10c | 4 renderer headers | **5** (4 empty + 1 mislabel) | 5 lines reported as 4 items |
| anchor-13 | "9 hits: 2 in source (+1 call)" / "all three" | **4 in `src/cartridges/`** / "all four" | prose disagreed with its own 4-row table |
| anchor-14 | 5 `on_patch_first_generated` | **6** | miscount |
| anchor-14 | 4 `PatchPhase::SPAWNED` | **5** — added `:359` (the write), fixed `:793`→`:791` | missed the assignment site; it *strengthens* the argument |
| anchor-15 | 16 / "all six" | **21** / "all eight" | prose disagreed with its own 8-row table |
| anchor-16 | 16 fog hits (6/2/8) | **17** (4/2/11) | miscount in two of three buckets |
| anchor-16 | member-access recipe "→ 0 hits" | **recipe corrected** — as written it returns **5** | ⚠ the recipe was stated but not run; the `config_.` exclusion it needed was missing. The five hits are all `config_.fog_color[…]` (`GPUDesignConfig`), so the ZERO-READER VERDICT IS UNCHANGED |
| headline | `grep … src/` "→ 6 hits" | **`src/cartridges/` → 6** (8 in `src/`; 2 are archived docs) | wrong path in the recipe |

The last row is the one that matters as a process failure: a number was
published with a recipe that had not been executed. It is the exact thing this
census's own law forbids, and it is recorded here rather than quietly fixed.

---

## SUMMARY (five lines)

1. **§Q1 — NONE FOUND.** No site derives a cell index from one authority and a
   stride/bound/extent from the other. The CAPACITY/SIZE split is coherent
   everywhere: field offsets and the texture normalizer are capacity-derived
   (1024 / 32), every index and every bound is `grid_size`-derived. The one real
   asymmetry is the **dispatch**: `dispatch_zone_gol_sync/evolve/seed_mask`
   hard-code `DispatchWorkgroups(4, 4, n)` = 32×32 threads for every zone,
   guarded by an in-kernel `grid_size` bound — correct, but an 8-cell zone
   launches 1024 threads to do 64 cells' work.
2. **§Q2 — THE CHAIN HOLDS, EXACTLY.** wu/texel = 800/512 = 1.5625; PATCH_CELL_SIZE
   3.125 = **exactly 2×** that, both exact binary fractions. But the relation is
   asserted only in *prose* (the live `static_assert` uses hard-coded 25/16, never
   names `PATCH_CELL_SIZE`), and **the REST law is a conjunction that the writer's
   own banner denies** — the banner says "terrain_time ≤ 0 ⇒ zeros"; the pulse add
   sits outside the gate.
3. **§Q3 — 10 of 10 retired.** Every Stage-5 retirement is ABSENT from source.
   All four suspected residue sites CONFIRMED, plus **eight more** the handoff did
   not list. Budgets recount clean: zero layouts at or over 10 storage / 12 uniform.
4. **§Q4 — all four are empty and deletable, but TWO PREMISES ARE WRONG.**
   `audit_entity_integrity` has **one** live caller, not zero.
   `record_placement_bookkeeping` has **four** callers, not one. The fog ruling
   stands (zero readers, verified), but the fields live in
   `contracts/spine_state.hpp`, not `direction/mood.hpp`.
5. **MOST ALARMING — `config.indoor_height_cap` is a live writer with zero
   readers, and its retirement was a silent behavior loss.** `apply_mood_lighting`
   still stages `INDOOR_HEIGHT_CAP_FRACTION × ceiling_height` every mood change;
   `grep -c "config.indoor_height_cap" world.wgsl` → **0**. Its only reader was
   the retired zone-extrusion kernel. **Indoor GoL zones are no longer height-capped
   at all** — a Monolith-tier zone (`alive_height_mean` 42 wu) can now lift through
   a 20 wu `MOOD_INDOOR_FLAT` ceiling. This is not residue; it is a live
   regression that Stage 5 introduced and campaign v2's retirement list never named.

---

# §Q1 — ZONE SIZE: TWO AUTHORITIES

## [anchor-1] Every read site, C++ and WGSL

**Recipe (C++ + WGSL, one pass):**
```
grep -rn "GOL_ZONE_GRID\|GOL_ZONE_CELLS\|GOL_ZONE_LIFE_STRIDE" \
  --include=*.hpp --include=*.wgsl --include=*.cpp --include=*.h src/ | sort
grep -rn "grid_cells" --include=*.hpp --include=*.wgsl --include=*.cpp src/ | sort
grep -rn "grid_size" --include=*.hpp --include=*.wgsl --include=*.cpp src/ | sort
grep -n "GOL_ZONE_STRIDE\|GOL_CELL_\|GOL_ZONE_TEX_N\|ZONE_DERIVE_CELL_SIZE" \
  src/cartridges/the_board/realization/world.wgsl
```
Counts: **17** `GOL_ZONE_*` hits (3 declarations + 14 uses), **9** `grid_cells`
hits, **23** `grid_size` hits in the cartridge (25 raw, less the 2
`src/external/implot` hits — an unrelated local variable), **0** hits for
`ZONE_EXTENT` / `zone_extent` / `ZONE_WORLD` / `zone_world`.

### The two authorities, named

| authority | spelling | value(s) | room |
|---|---|---|---|
| CAPACITY | `Dim::GOL_ZONE_GRID` / `GOL_ZONE_CELLS` / `GOL_ZONE_LIFE_STRIDE` | 32 / 1024 / 7168 | C++ `state.hpp:251-253` |
| CAPACITY | `GOL_ZONE_TEX_N` / `GOL_ZONE_STRIDE` / `GOL_CELL_*` | 32.0 / 7168u / k×1024u | WGSL `world.wgsl:8095-8103` |
| SIZE | `GoLTierProfile.grid_cells` / `GolPulseTierProfile.grid_cells` | {8,16,24,32} | C++ `gol_zones.hpp:161, 230` |
| SIZE | `GoLTierParams.grid_cells` / `GolPulseTierParams.grid_cells` | {8,16,24,32} | WGSL `world.wgsl:1999, 2059` |
| SIZE (GPU mirror field) | `GoLZoneConfig.grid_size` / `.extent` | tier-derived | WGSL `world.wgsl:5440`; C++ `state.hpp:1053` |
| SIZE (world extent unit) | `PATCH_CELL_SIZE` / `ZONE_DERIVE_CELL_SIZE` | 3.125 | C++ `surface_services.hpp:70`; WGSL `world.wgsl:5729` |

There is **no** constant named `ZONE_EXTENT`. A zone's world extent is carried
per-zone as `GoLZoneConfig.extent`, computed at derive time from `grid_cells ×
3.125`, and on the CPU by `gol_tier_extent()`.

### C++ sites

| file:line | expression | verdict | reads |
|---|---|---|---|
| `state.hpp:251` | `constexpr uint32_t GOL_ZONE_GRID = 32;` | CAPACITY | — (declaration) |
| `state.hpp:252` | `GOL_ZONE_CELLS = GOL_ZONE_GRID * GOL_ZONE_GRID` | CAPACITY | GOL_ZONE_GRID |
| `state.hpp:253` | `GOL_ZONE_LIFE_STRIDE = GOL_ZONE_CELLS * 7` | CAPACITY | GOL_ZONE_CELLS |
| `state.hpp:2818` | `size_t base = slot * Dim::GOL_ZONE_LIFE_STRIDE * sizeof(float);` | CAPACITY | CAPACITY |
| `state.hpp:2820` | `size_t slot_stride = Dim::GOL_ZONE_CELLS * sizeof(float);` | CAPACITY | CAPACITY |
| `state.hpp:3486` | `Dim::MAX_GOL_ZONES * Dim::GOL_ZONE_LIFE_STRIDE * sizeof(float)` (buffer alloc) | CAPACITY | CAPACITY |
| `state.hpp:3495` | `desc.size = { Dim::GOL_ZONE_GRID, Dim::GOL_ZONE_GRID, Dim::MAX_GOL_ZONES };` | CAPACITY | CAPACITY |
| `state.hpp:3519-3520` | boot log `" zones × " << GOL_ZONE_GRID << "×" << GOL_ZONE_GRID` | CAPACITY | CAPACITY |
| `state.hpp:5313` | `entries[2].size = MAX_GOL_ZONES * GOL_ZONE_LIFE_STRIDE * sizeof(float);` | CAPACITY | CAPACITY |
| `state.hpp:5385` | `entries[3].size = …` (Live Card Writer group) | CAPACITY | CAPACITY |
| `state.hpp:5421` | `entries[3].size = …` (Zone Mask group) | CAPACITY | CAPACITY |
| `gol_zones.hpp:581` | `std::vector<float> life(Dim::GOL_ZONE_CELLS, 0.0f);` | CAPACITY | CAPACITY |
| `gol_zones.hpp:584` | `for (uint32_t i = 0; i < Dim::GOL_ZONE_CELLS; i++)` | CAPACITY | CAPACITY |
| `gol_zones.hpp:595` | `std::vector<float> height_factors(Dim::GOL_ZONE_CELLS);` | CAPACITY | CAPACITY |
| `gol_zones.hpp:596` | `for (uint32_t i = 0; i < Dim::GOL_ZONE_CELLS; i++)` | CAPACITY | CAPACITY |
| `gol_zones.hpp:604` | `upload_zone_life(queue, slot, life.data(), height_factors.data(), Dim::GOL_ZONE_CELLS)` | CAPACITY | CAPACITY |
| `gol_zones.hpp:161` | `uint32_t grid_cells;  // zone side in cells ∈ {8..32}` | SIZE | — (field) |
| `gol_zones.hpp:230` | same, Pulse profile | SIZE | — (field) |
| `gol_zones.hpp:258-259` | `GOL_TIERS[tier_idx].grid_cells : GOL_PULSE_TIERS[…].grid_cells` | SIZE | SIZE |
| `gol_zones.hpp:267-268` | `extent_x = (float)cx * PATCH_CELL_SIZE;` | SIZE | SIZE |
| `gol_zones.hpp:455, 457` | corner snap `floor((raw − extent*0.5)/PATCH_CELL_SIZE)*PATCH_CELL_SIZE` | SIZE | SIZE |
| `gol_zones.hpp:482` | `sel.footprint_r = 0.5f * std::hypot(extent_x, extent_z);` | SIZE | SIZE |
| `state.hpp:1053` | `uint32_t grid_size;` (GPUGoLZoneConfig field) | SIZE | — (field) |

### WGSL sites

| file:line | expression | verdict | reads |
|---|---|---|---|
| `world.wgsl:8095` | `const GOL_ZONE_TEX_N: f32 = 32.0;` | CAPACITY | — (declaration) |
| `world.wgsl:8096` | `const GOL_ZONE_STRIDE: u32 = 7168u;` | CAPACITY | — |
| `world.wgsl:8097-8103` | `GOL_CELL_VISUAL 0 … GOL_CELL_COLOR_VELOCITY 6144` (k×1024) | CAPACITY | — |
| `world.wgsl:2715` | `let base = z * GOL_ZONE_STRIDE;` | CAPACITY | CAPACITY |
| `world.wgsl:2717-2718` | `zone_life[base + GOL_CELL_VISUAL + idx]`, `… + GOL_CELL_HEIGHT_FACTOR + idx` | field CAPACITY, index SIZE | both, correctly |
| `world.wgsl:8114, 8129` | `let base = zone_id * GOL_ZONE_STRIDE;` | CAPACITY | CAPACITY |
| `world.wgsl:5913` | `let idx = req.slot * GOL_ZONE_STRIDE + GOL_CELL_HEIGHT_FACTOR` | CAPACITY | CAPACITY |
| `world.wgsl:5914` | `+ gid.y * zp.grid_size + gid.x;` | SIZE | SIZE |
| `world.wgsl:4063` | `let uv = (vec2<f32>(local_cell) + 0.5) / GOL_ZONE_TEX_N;` | index SIZE, normalizer CAPACITY | both, **by design** |
| `world.wgsl:1999, 2059` | `grid_cells: u32,` (tier struct fields) | SIZE | — |
| `world.wgsl:2007-2015, 2066-2070` | GOL_TIERS / GOL_PULSE_TIERS cells column | SIZE | — |
| `world.wgsl:5440` | `grid_size: u32,` (GoLZoneConfig field) | SIZE | — |
| `world.wgsl:5729` | `const ZONE_DERIVE_CELL_SIZE: f32 = 3.125;` | SIZE unit | — |
| `world.wgsl:5803, 5850` | `zc.grid_size = tp.grid_cells; / = pp.grid_cells;` | SIZE | SIZE |
| `world.wgsl:5804, 5851` | `zc.extent = f32(zc.grid_size) * ZONE_DERIVE_CELL_SIZE;` | SIZE | SIZE |
| `world.wgsl:5882-5884` | corner snap + `zc.origin` | SIZE | SIZE |
| `world.wgsl:2708` | `let cell_size = zp.extent / f32(zp.grid_size);` | SIZE | SIZE |
| `world.wgsl:2713` | `if (cx < 0 \|\| cx >= i32(zp.grid_size) \|\| …)` | SIZE | SIZE |
| `world.wgsl:2716` | `let idx = u32(cy) * zp.grid_size + u32(cx);` | SIZE | SIZE |
| `world.wgsl:4060-4061` | FS coverage bounds `< i32(zp.grid_size)` | SIZE | SIZE |
| `world.wgsl:5906` | `if (gid.x >= zp.grid_size \|\| gid.y >= zp.grid_size) { return; }` | SIZE | SIZE |
| `world.wgsl:5907` | `let cs = zp.extent / f32(zp.grid_size);` | SIZE | SIZE |
| `world.wgsl:8112, 8127` | `if (cell.x >= z.grid_size \|\| cell.y >= z.grid_size) { return; }` | SIZE | SIZE |
| `world.wgsl:8115, 8130` | `let idx = cell.y * z.grid_size + cell.x;` | SIZE | SIZE |
| `world.wgsl:8145` | `let gs = i32(z.grid_size);` (neighbour wrap modulus) | SIZE | SIZE |
| `world.wgsl:8151` | `let ni = ny * z.grid_size + nx;` | SIZE | SIZE |
| `world.wgsl:8226` | `textureStore(zone_life_tex_write, cell, i32(zone_id), …)` | SIZE coords into CAPACITY layer | both, by design |
| `renderer.hpp:615, 627` | `pass.DispatchWorkgroups(4, 4, zone_count);  // 32/8=4 per axis` | CAPACITY | CAPACITY (hard-coded) |
| `renderer.hpp:654` | `pass.DispatchWorkgroups(4, 4, request_count);` (seed mask) | CAPACITY | CAPACITY (hard-coded) |

## [anchor-2] The mixing check

> *Is there ANY site where a cell index is derived from one authority and a
> stride, bound, or extent from the other?*

**NONE FOUND.**

The `zone_life` addressing convention is a single, consistent law, honoured at
every one of its five sites (`world.wgsl:2716, 5914, 8115, 8130, 8151`):

```
address = slot * 7168            // CAPACITY: per-zone block
        + field * 1024           // CAPACITY: field plane inside the block
        + y * grid_size + x      // SIZE:     dense row-major over the zone's own grid
```

`0 ≤ y*grid_size + x < grid_size² ≤ 1024`, so the SIZE-derived index is always a
proper sub-range of the CAPACITY-sized field plane. That is the *correct* pairing,
not a mix — and `world.wgsl:5911-5912` states the law verbatim at the mask kernel:

```wgsl
5910:    let vis = step(0.5, discrete_visibility_rest(center, cell_address(center)));
5911:    // the sim kernels' own dense row-major (idx = y * grid_size + x) —
5912:    // NOT a fixed-32 stride; the U5b gate (a) verified convention.
5913:    let idx = req.slot * GOL_ZONE_STRIDE + GOL_CELL_HEIGHT_FACTOR
5914:            + gid.y * zp.grid_size + gid.x;
5915:    zone_life[idx] = zone_life[idx] * vis;   // Gaussian seed × mask
```

The texture path is the same shape and equally coherent: the sim writes texels
`[0, grid_size)²` of a 32² layer (`world.wgsl:8226`), and the FS normalizes by
32 — never by `grid_size` — at `world.wgsl:4063`. The convention is stated at
`world.wgsl:8091-8094`:

```wgsl
8090:// Two compute passes per frame (when zones are active):
8091:// The zone life texture's side — twin of Dim::GOL_ZONE_GRID
8092:// (state.hpp). FIXED at 32 while zp.grid_size is tier-derived over
8093:// {8..32}: the sim writes texels [0, grid_size)² of a 32² layer, so
8094:// every fetch normalizes by THIS, never by the zone's own grid.
8095:const GOL_ZONE_TEX_N: f32 = 32.0;
8096:const GOL_ZONE_STRIDE: u32 = 7168u;     // floats per zone (7 slots × 1024 cells)
```

**Recipe used to look:** every occurrence of a `*` or `+` adjoining a
`GOL_CELL_`, `GOL_ZONE_STRIDE`, `GOL_ZONE_TEX_N`, `GOL_ZONE_CELLS` or
`grid_size` token was read in context —
```
grep -n "GOL_ZONE_STRIDE\|GOL_CELL_\|GOL_ZONE_TEX_N" src/cartridges/the_board/realization/world.wgsl
grep -n "grid_size" src/cartridges/the_board/realization/world.wgsl
grep -rn "GOL_ZONE_CELLS\|GOL_ZONE_LIFE_STRIDE\|GOL_ZONE_GRID" --include=*.hpp src/
```
→ **31 + 22 + 16 hit lines** (WGSL stride/offset constants · WGSL `grid_size` ·
C++ `GOL_ZONE_*`), each opened and classified in the [anchor-1] table.
No pairing violates the law.

### Two things that are NOT bugs but are worth the record

**(i) Slot re-use is safe.** When a slot is recycled by a zone of a different
`grid_cells`, `seed_gol_zone` rewrites **all 1024 cells of all 7 fields**
(`gol_zones.hpp:604` → `state.hpp:2815-2836`, seven `WriteBuffer` calls at
`cell_count = 1024`). No stale sub-grid can survive. The **texture** keeps the
previous zone's values in texels `[grid_size, 32)²`, but those texels are never
read: the only reader (`world.wgsl:4063-4066`) indexes with `local_cell`, which
`world.wgsl:4060-4061` has already bounded by `grid_size`. Clean.

**(ii) A latent trap in the CPU seeder.** `seed_gol_zone` fills a *linear* 1024
array with per-index i.i.d. hashes (`cpu_hash_f(seed + i, …)`), and the GPU
reinterprets `[0, grid_size²)` as a `grid_size`-wide 2D grid. Because the CPU
makes no 2D assumption at all, the distribution is identical for every
`grid_size` and nothing is wrong today. But the *moment* anyone gives the seeder
a spatially structured pattern (a glider, a symmetric motif, a mask), it will
land scrambled on every non-32 tier — which is 8 of the 10 tier rows. That is a
comment worth adding at `gol_zones.hpp:580`, not a fix.

## [anchor-3] The four consumers

### (a) `zone_gol_sync` / `zone_gol_evolve` — dispatch extent vs in-kernel bound

**Dispatch extent: CAPACITY. In-kernel bound: SIZE.**

There is no 12-cell tier (authored values are 8/16/24/32 — see [anchor-4]), so I
answer the question as posed for any sub-32 zone: **it dispatches 32, not its own
size.** `renderer.hpp:606-628`:

```cpp
606:            void dispatch_zone_gol_sync(
607:                wgpu::ComputePassEncoder& pass,
608:                wgpu::BindGroup zoneComputeBindGroup,
609:                uint32_t zone_count
610:            ) {
611:                if constexpr (!(ROSTER.gol)) return;  // ROSTER-GATE gol (a') — pipeline never created; the holder tolerates
612:                if (zone_count == 0) return;
613:                pass.SetPipeline(zoneGolSyncPipeline_);
614:                pass.SetBindGroup(0, zoneComputeBindGroup);
615:                pass.DispatchWorkgroups(4, 4, zone_count);  // 32/8=4 per axis, z=zones
616:            }
617:
618:            void dispatch_zone_gol_evolve(
619:                wgpu::ComputePassEncoder& pass,
620:                wgpu::BindGroup zoneComputeBindGroup,
621:                uint32_t zone_count
622:            ) {
623:                if constexpr (!(ROSTER.gol)) return;  // ROSTER-GATE gol (a') — pipeline never created; the holder tolerates
624:                if (zone_count == 0) return;
625:                pass.SetPipeline(zoneGolEvolvePipeline_);
626:                pass.SetBindGroup(0, zoneComputeBindGroup);
627:                pass.DispatchWorkgroups(4, 4, zone_count);
628:            }
```

The `4, 4` and its `// 32/8=4 per axis` comment are the last hard-coded 32 in the
dispatch path — a **Stage-5 leftover**: the workgroup count was never re-aimed at
the per-tier size. It is **correct**, because both kernels early-out on the SIZE
bound (`world.wgsl:8112` and `8127`, identical):

```wgsl
8111:    let cell = gid.xy;
8112:    if (cell.x >= z.grid_size || cell.y >= z.grid_size) { return; }
```

Cost: a Drift-tier zone (`grid_cells = 8`, 64 cells) launches 1024 threads and
retires 960 of them immediately — 16× over-dispatch. Same for
`dispatch_zone_seed_mask` (`renderer.hpp:654`, `DispatchWorkgroups(4, 4, request_count)`)
against `world.wgsl:5906`. Since `grid_cells ∈ {8,16,24,32}` are all multiples of
the workgroup edge 8, the derived form `(grid_cells + 7) / 8` would be exact —
the same fix shape as the `dispatch_frustum_cull` 4→5 correction already landed
at `renderer.hpp:512-515`. Flagged, **not fixed**.

### (b) `zone_life` indexing — what sizes `idx`

**SIZE.** `idx = y * grid_size + x` at all five sites
(`world.wgsl:2716, 5914, 8115, 8130, 8151`). `base` and the field offsets are
CAPACITY. See [anchor-2] for the full law.

### (c) The zone's WORLD extent — spawn footprint radius and the mask predicate

Both read **SIZE**. A 32-cell zone is 100 wu; an 8-cell zone is 25 wu; each site
reads its own zone's number.

`select_gol_for_patch`, `bodies/gol_zones.hpp:445-483`:
```cpp
445:            // Zone extent + corner (cell-grid-snapped), from the tier's
446:            // own size. Snapping subtracts an exact multiple of
447:            // PATCH_CELL_SIZE, so corner + extent/2 returns the snapped
448:            // raw centre for every tier — the same identity the GPU's
449:            // zone_derive_params relies on, which is why the centre was
450:            // right even while the extent was a fixed 100.
451:            float extent_x = 0.0f, extent_z = 0.0f;
452:            gol_tier_extent(tier_idx, extent_x, extent_z);
453:
454:            float corner_x = std::floor(
455:                (raw_cx - extent_x * 0.5f) / PATCH_CELL_SIZE) * PATCH_CELL_SIZE;
456:            float corner_z = std::floor(
457:                (raw_cz - extent_z * 0.5f) / PATCH_CELL_SIZE) * PATCH_CELL_SIZE;
…
482:            sel.footprint_r = 0.5f * std::hypot(extent_x, extent_z);
```
`gol_tier_extent` (`gol_zones.hpp:264-269`) is `grid_cells × PATCH_CELL_SIZE` —
pure SIZE. The comment at 450 explicitly retires the "fixed 100" era.

The mask predicate, `world.wgsl:5900-5915`:
```wgsl
5900:@compute @workgroup_size(8, 8, 1)
5901:fn zone_seed_mask(@builtin(global_invocation_id) gid: vec3<u32>) {
5902:    let req_idx = gid.z;
5903:    if (req_idx >= zone_derive_requests.count) { return; }
5904:    let req = zone_derive_requests.requests[req_idx];
5905:    let zp = zone_config.zones[req.slot];
5906:    if (gid.x >= zp.grid_size || gid.y >= zp.grid_size) { return; }
5907:    let cs = zp.extent / f32(zp.grid_size);
5908:    let corner = zp.origin - vec2(zp.extent * 0.5);
5909:    let center = corner + (vec2(f32(gid.x), f32(gid.y)) + vec2(0.5)) * cs;
```
`zp.extent` and `zp.grid_size` both come from `zone_derive_params`
(`world.wgsl:5803-5804`, `5850-5851`) — SIZE throughout. `cs` recovers exactly
3.125 for every tier.

The runtime coverage test in the terrain FS is also SIZE (`world.wgsl:4050-4061`),
and its comment records the very failure this question is hunting:
```wgsl
4052:                    // COVERAGE, NOT LATTICE (UG_FIELDS_1 S1). The bounds
4053:                    // test IS the membership test. The retired pre-filter
4054:                    // compared lattice nodes, which assumes a zone never
4055:                    // leaves its 120 wu node; a 32-cell zone is already
4056:                    // 100 wu and fragments across the boundary lost it.
4057:                    // This is also the FS's last MODE_LATTICE_SPACING
4058:                    // reader — the zone's SIZE is now free of the lattice
4059:                    // that places it.
4060:                    if (local_cell.x >= 0 && local_cell.x < i32(zp.grid_size) &&
4061:                        local_cell.y >= 0 && local_cell.y < i32(zp.grid_size)) {
```

### (d) The card's `.a` cell-snap

**CONFIRMED: the snap is to the CELL grid (3.125), with no 32-derived term
anywhere in the card path.** `world.wgsl:227-232`:

```wgsl
223:// ── THE LIVE CARD (GROUND_CARD_1; C++ room: Dim::LIVE_CARD_*) ──
224:const LIVE_CARD_SIZE: u32 = 512u;
225:const LIVE_CARD_EXTENT: f32 = 800.0;
226:const SHELL_RING_WIDTH: f32 = 0.35;   // wu, half-width of a ring band (DEBUG_VIEW 6)
227:fn live_card_origin() -> vec2<f32> {
228:    let cs = PATCH_EXTENT / f32(PATCH_CELL_N);           // 3.125
229:    let raw = vec2(config.lod_point_x, config.lod_point_z)
230:            - vec2(LIVE_CARD_EXTENT * 0.5);
231:    return floor(raw / cs) * cs;                          // cell snap
232:}
```

and `world.wgsl:8238-8249`:
```wgsl
8238:fn live_card_uv(world_xz: vec2<f32>) -> vec2<f32> {
8239:    return (world_xz - live_card_origin()) / LIVE_CARD_EXTENT;
8240:}
8241:fn sample_live_card(world_xz: vec2<f32>) -> vec4<f32> {
8242:    return textureSampleLevel(live_card_read, bilinear_sampler,
8243:                              live_card_uv(world_xz), 0.0);
8244:}
8245:fn sample_live_card_gol(world_xz: vec2<f32>) -> f32 {
8246:    // nearest + cell-snapped origin ⇒ cell-exact raw lift
8247:    return textureSampleLevel(live_card_read, nearest_sampler,
8248:                              live_card_uv(world_xz), 0.0).w;
8249:}
```

`cs` = `PATCH_EXTENT / PATCH_CELL_N` = 50/16 = 3.125 — the **patch cell grid**,
identical to `PATCH_CELL_SIZE` (C++) and `ZONE_DERIVE_CELL_SIZE` (WGSL). Nothing
in the chain reads `GOL_ZONE_GRID`, `GOL_ZONE_TEX_N`, or any zone's `grid_size`.

Cell-exactness of the nearest fetch is a consequence of three facts that all
hold: card origin on the 3.125 grid; zone corners on the 3.125 grid
(`world.wgsl:5882-5883`); and one cell = exactly 2 texels ([anchor-6]). Both
texels inside a cell were written from `contrib_gol_zones_at` evaluated at their
own centres, and both centres lie inside the same GoL cell — so either texel
returns the same `.a`.

## [anchor-4] Authored `grid_cells` per tier

**Recipe:** `sed -n '168,176p;237,241p' src/cartridges/the_board/bodies/gol_zones.hpp`
and `sed -n '2007,2015p;2066,2070p' src/cartridges/the_board/realization/world.wgsl`.

The handoff says "the 7 tiers"; there are in fact **10 authored rows** — 7 Conway
plus 3 Pulse, addressed by one compound index (`gol_zones.hpp:248-262`). Both
tables reported.

**Conway — `GOL_TIERS` (`gol_zones.hpp:169-175` / `world.wgsl:2008-2014`):**

| # | tier | weight | `grid_cells` | extent (wu) |
|---|---|---|---|---|
| 0 | Pillars | 0.10 | **16** | 50 |
| 1 | Sparse | 0.20 | **32** | 100 |
| 2 | Moderate | 0.18 | **32** | 100 |
| 3 | Dense | 0.10 | **16** | 50 |
| 4 | Flash | 0.17 | **24** | 75 |
| 5 | Monolith | 0.12 | **16** | 50 |
| 6 | Glacier | 0.13 | **24** | 75 |

**Pulse — `GOL_PULSE_TIERS` (`gol_zones.hpp:238-240` / `world.wgsl:2067-2069`):**

| # | tier | weight | `grid_cells` | extent (wu) |
|---|---|---|---|---|
| 0 | Breathe | 0.45 | **32** | 100 |
| 1 | Sparkle | 0.30 | **16** | 50 |
| 2 | Drift | 0.25 | **8** | 25 |

Three checks, all pass:
- **Mirror (L3) exact.** Both rooms carry identical cells columns, row for row.
- **The authoring rule verifies.** Campaign v2 §6 says "authored defaults by
  weight order thirds, 32/24/16". Conway rows sorted by descending weight are
  Sparse .20, Moderate .18, Flash .17, Glacier .13, Monolith .12, Pillars .10,
  Dense .10 → cells 32, 32, 24, 24, 16, 16, 16. That is exactly thirds of 7 as
  2/2/3. Pulse: "32/16/8 by weight order" → .45→32, .30→16, .25→8. Exact.
- **Weights sum to 1.0** in both tables (0.10+0.20+0.18+0.10+0.17+0.12+0.13 = 1.00;
  0.45+0.30+0.25 = 1.00).
- **The stride-edge ruling holds:** max extent 100 wu, at the "> 100 wu = stride
  edge, out of scope" boundary, never over it.

---

# §Q2 — THE CARD'S NUMERIC CHAIN

## [anchor-5] Values and homes

**Recipe:**
```
grep -rn "LIVE_CARD_SIZE\|LIVE_CARD_EXTENT\|PATCH_CELL_SIZE\|VEIL_RING\|VEIL_ICING\|LOD0_RADIUS\|EXIST_RADIUS\|AGENT_EVICTION_RADIUS\|MAX_GOL_ZONES" \
  --include=*.hpp --include=*.wgsl src/ | sort
```

| constant | value | C++ home | WGSL home | L3 status |
|---|---|---|---|---|
| `LIVE_CARD_SIZE` | **512** | `state.hpp:93` | `world.wgsl:224` | **MIRRORED** (literal in both) |
| card world extent | **800.0** | `state.hpp:94` (`LIVE_CARD_EXTENT_WU`) | `world.wgsl:225` (`LIVE_CARD_EXTENT`) | **MIRRORED** (literal in both; *names differ by the `_WU` suffix*) |
| `PATCH_CELL_SIZE` | **3.125** | `surface_services.hpp:70` (`= PATCH_EXTENT / 16.0f`) | **no named constant** — recomputed inline as `PATCH_EXTENT / f32(PATCH_CELL_N)` at `world.wgsl:228` and `:242`, and authored as a **literal 3.125** at `world.wgsl:5729` (`ZONE_DERIVE_CELL_SIZE`) | **MIRRORED, THREE SPELLINGS** — one C++ derivation, two WGSL derivations, one WGSL literal |
| `VEIL_RING_DEFAULT` | **325.0** (`6.5f × PATCH_EXTENT`) | `state.hpp:124` | none — rides `config.veil_ring` | **C++ only** |
| `VEIL_ICING_DEFAULT` | **40.0** | `state.hpp:125` | none — rides `config.veil_icing` | **C++ only** |
| `LOD0_RADIUS_DEFAULT` | **175.0** (`3.5f × PATCH_EXTENT`) | `state.hpp:123` | none — rides `config.lod0_radius` / `fc_config.lod0_radius` | **C++ only** |
| `EXIST_RADIUS` | **350.0** (`7.0f × PATCH_EXTENT`) | `state.hpp:126` | none by that name | **C++ only** |
| `AGENT_EVICTION_RADIUS` | **350.0** | `bodies/agents.hpp:117` | `world.wgsl:6815` | **MIRRORED**, and the mirror is *asserted CPU-side*: `agents.hpp:119` `static_assert(AGENT_EVICTION_RADIUS == Dim::EXIST_RADIUS, …)`; the WGSL half is held only by the comment at `world.wgsl:6813-6814` |
| `MAX_GOL_ZONES` | **8** | `state.hpp:242` | **unnamed literal `8`** — `array<GoLZoneConfig, 8>` (`world.wgsl:5592`), `array<ZoneDeriveRequest, 8>` (`world.wgsl:5722`) | **MIRRORED as a bare literal** — no WGSL constant to grep |

Three L3 observations for commit 3, none of them a bug today:
- `PATCH_CELL_SIZE` has **four** authoring sites for one number. `ZONE_DERIVE_CELL_SIZE`
  is the only hard literal; if `PATCH_CELL_N` ever moves, it drifts silently.
- `MAX_GOL_ZONES`'s WGSL half is a bare `8` in two array extents. A rename or a
  raise is a two-site edit with nothing to grep for.
- The card extent's two names differ (`LIVE_CARD_EXTENT_WU` vs `LIVE_CARD_EXTENT`),
  which breaks the registry's usual "names deliberately equal across rooms" habit.

## [anchor-6] CELL-EXACTNESS

**THE CLAIM HOLDS EXACTLY. The integer is 2.**

```
wu-per-texel = LIVE_CARD_EXTENT / LIVE_CARD_SIZE = 800.0 / 512 = 1.5625
PATCH_CELL_SIZE                                  = 50.0 / 16   = 3.125
ratio = 3.125 / 1.5625                           = 2           (exactly)
```

Both operands are exact binary fractions — 1.5625 = 25/16 = 0b1.1001,
3.125 = 25/8 = 0b11.001 — so the ratio is exact in `f32` with no rounding
anywhere. One patch cell spans exactly **two card texels** per axis, and because
`live_card_origin()` snaps to the same 3.125 grid the cells sit on, cell borders
land on texel borders. Both texels of a cell were written from
`contrib_gol_zones_at` at their own centres, both centres inside the same GoL
cell, so `sample_live_card_gol` returns the same value from either — **cell-exact,
as claimed.**

**What is NOT pinned (this is the commit-3 finding).** The live assert
(`state.hpp:95-97`) pins the *texel size*, not the *relation to `PATCH_CELL_SIZE`*:

```cpp
 87:            // ── THE LIVE CARD (GROUND_CARD_1) ──
 88:            // One 2D RGBA16F field over the ground window, point-centered,
 89:            // fully rewritten per frame. R = waves+pulses Δh; G/B = wave ∂x/∂z
 90:            // (waves-only this campaign — pulse shading is Stage 6); A = raw
 91:            // GoL lift. Window ORIGIN SNAPS to the 3.125 cell grid so a
 92:            // nearest fetch of .a is cell-exact.
 93:            constexpr uint32_t LIVE_CARD_SIZE      = 512;
 94:            constexpr float    LIVE_CARD_EXTENT_WU = 800.0f;
 95:            static_assert(LIVE_CARD_SIZE * 25u == (uint32_t)LIVE_CARD_EXTENT_WU * 16u,
 96:                "live card: texel must be PATCH_CELL_SIZE/2 (1.5625 wu) — "
 97:                "512*25 == 800*16");
```

The expression uses hard-coded `25u` and `16u`; `PATCH_CELL_SIZE` appears only in
the *message string*. The relation is prose, not machinery. It cannot be fixed in
place — `PATCH_CELL_SIZE` is declared in `contracts/surface_services.hpp:70`,
which *includes* `state.hpp`, so a real assert has to live on the
`surface_services.hpp` side (or `PATCH_CELL_SIZE` has to move into `Dim`).

A **second** unasserted invariant sits beside it: the writer's two dispatches
require `LIVE_CARD_SIZE % 8 == 0` and `% 16 == 0`
(`renderer.hpp:537-541`, `DispatchWorkgroups(LIVE_CARD_SIZE/8u, …)` and `/16u`).
512 satisfies both; nothing checks it. Any card resize that is not a multiple of
16 would silently under-dispatch the resolve pass and leave a strip of the card
never written.

## [anchor-7] WINDOW COVERAGE

### (a) Bounds test — **CONFIRMED ABSENT**

Neither sampler function performs any bounds test; both hand a possibly
out-of-`[0,1]` UV straight to `textureSampleLevel`. Quoted verbatim with context
at [anchor-3](d) above (`world.wgsl:8238-8249`). Outside the window, the result
is the ClampToEdge texel — the smear, exactly as the handoff supposes.

### (b) Sampler address modes — **BOTH ClampToEdge**

`state.hpp:3700-3721`:
```cpp
3700:            bool createSamplers() {
3701:                {
3702:                    wgpu::SamplerDescriptor desc{};
3703:                    desc.label = "Bilinear Sampler (height field interpolation)";
3704:                    desc.magFilter = wgpu::FilterMode::Linear;
3705:                    desc.minFilter = wgpu::FilterMode::Linear;
3706:                    desc.addressModeU = wgpu::AddressMode::ClampToEdge;
3707:                    desc.addressModeV = wgpu::AddressMode::ClampToEdge;
3708:                    bilinearSampler_ = device_.CreateSampler(&desc);
3709:                    if (!bilinearSampler_) return false;
3710:                }
3711:
3712:                {
3713:                    wgpu::SamplerDescriptor desc{};
3714:                    desc.label = "Nearest Sampler (cell boundaries)";
3715:                    desc.magFilter = wgpu::FilterMode::Nearest;
3716:                    desc.minFilter = wgpu::FilterMode::Nearest;
3717:                    desc.addressModeU = wgpu::AddressMode::ClampToEdge;
3718:                    desc.addressModeV = wgpu::AddressMode::ClampToEdge;
3719:                    nearestSampler_ = device_.CreateSampler(&desc);
3720:                    if (!nearestSampler_) return false;
3721:                }
```
`addressModeW` is left at its default (`ClampToEdge`) and is irrelevant for a 2D
texture. **The smear claim's premise is verified.**

### (c) Half-extent vs EXIST / AGENT_EVICTION — margins in wu

The window is **not** exactly centred: `live_card_origin()` floors to the 3.125
grid, so the window shifts by up to one cell in −x and −z. Writing
`r = lod_point.x − 400` and `origin = floor(r/3.125)·3.125 ∈ (r − 3.125, r]`:

| direction | half-extent | guaranteed |
|---|---|---|
| +x, +z | `origin + 800 − lod_point` ∈ **(396.875, 400.0]** | **≥ 396.875 wu** |
| −x, −z | `lod_point − origin` ∈ **[400.0, 403.125)** | **≥ 400.0 wu** |

Worst-case guaranteed half-extent: **396.875 wu** (not the nominal 400 the
campaign's §7 covenant states — that is a small premise correction worth taking).

| against | value | margin (worst case) |
|---|---|---|
| `EXIST_RADIUS` | 350.0 | **+46.875 wu** |
| `AGENT_EVICTION_RADIUS` | 350.0 | **+46.875 wu** |
| `VEIL_RING_DEFAULT` | 325.0 | +71.875 wu |
| farthest **drawn** patch vertex (see (d)) | 375.0 per axis | **+21.875 wu** |
| farthest **patch-owned entity** (pregen ring) | 450.0 per axis | **−53.125 wu** ⚠ |

The window is a **square**, so per-axis offset is what matters, not radial
distance. A point 395 wu away on the diagonal is only 279 wu per axis — well
inside.

### (d) Consumers sampling beyond the half-extent — the §4 table, row by row

I enumerated every call site first:
```
grep -n "sample_live_card\|sample_live_card_gol" src/cartridges/the_board/realization/world.wgsl
```
→ **31 hit lines**: 2 definitions (`8241`, `8245`), 2 in comments (`2899`,
`8791`), and **27 call sites** — of which `355` is the shared `ug_cell_lift`
helper that the patch VS and shadow patch VS both reach through.

| §4 row | sites | max per-axis offset from `lod_point` | verdict |
|---|---|---|---|
| **patch VS** | `world.wgsl:3882` (`sample_live_card`), `:355` via `ug_cell_lift` (`.a`) | **375 wu** | **INSIDE** (+21.875 margin) |
| **shadow patch VS** | `world.wgsl:4170`, `:4174` via `ug_cell_lift` | **375 wu** infinite mode; **up to 450 wu** in finite mode | **INSIDE infinite; OUTSIDE finite** ⚠ |
| **entity/wall VS ×12** | arch `4716`/`4731`, column `4744`/`4759`, wall painting `9367`, palm `10598`/`10619`, cactus `10935`/`10953`, blade `11171`/`11189` | **up to 450 wu** | **OUTSIDE for the far band** ⚠ (main-pass result discarded; shadow twins keep it) |
| **WALKER** (`query_ground_walker_pair`) | `world.wgsl:3025` (`.a`), `:3026` (`.x`) | queried at the pawn, ≈0 wu | **INSIDE** |
| **WALKER_TILT** | same function, `.y` output | pawn ± eps 0.5 wu | **INSIDE** |
| **WALKER_AGENT** | `world.wgsl:3054` → `manifold_overlay_stack` `:2920` | agents evicted at 350 | **INSIDE** (+46.875) |
| **FLYER** | `world.wgsl:2938` → `:2920` | spheres/cubes/camera, ≤ 350–400 | **INSIDE at 350; floaters at 400 are at −3.125 to +0 margin** ⚠ marginal |
| **placement** (`compute_entity_placement`) | `world.wgsl:8803, 8824, 8834, 8845, 8856, 8868, 8869` | **up to 450 wu** | **OUTSIDE for the far band, and the result IS consumed** ⚠ |
| **CELESTIAL** | — | no card sample | n/a (0, unchanged) |
| debug view 5 | `world.wgsl:3934` | after the rim discard, ≤ 325 | **INSIDE** |

**How the 375 and 450 numbers are derived.**

*Drawn patch set (375).* `band_patches` (`patch_system.hpp:461-475`) admits a
patch when `patch_distance_sq(point, centre, half=25) ≤ veil_ring²`, i.e.
`max(0,|Δcx|−25)² + max(0,|Δcz|−25)² ≤ 325²`. Worst per-axis: `|Δcx| = 350`;
a patch spans centre ±25, so the farthest vertex is **375 wu** per axis. Both
passes draw only `[0, render_patch_count)` — the pregen band is packed after and
never drawn (`render_passes.hpp:313-324` shadow, `:364-392` main).

*Patch-owned entities (450).* Patches are allocated over
`gx ∈ [pawnGX − 8, pawnGX + 8]` (`patch_system.hpp:717-718`, `active_radius =
PATCH_PREGEN_RADIUS = 8`), and the pawn sits anywhere inside cell `pawnGX`.
Extremes: `(pawnGX+9)·50 − pawnGX·50 = 450` and
`(pawnGX+1)·50 − (pawnGX−8)·50 = 450`. Flora, columns, arches and outdoor
paintings live as long as their host patch does (`evict_patch` →
`evict_patch_entities`, `patch_system.hpp:43-58`) — they are **not**
`EXIST_RADIUS`-gated. So `compute_entity_placement` samples the card up to
**450 wu** per axis and writes the clamped result into `entity_ground_atlas`.

**Three concrete out-of-window reads, in severity order:**

1. **`compute_entity_placement` (`world.wgsl:8775-8874`) — result consumed.**
   For any active flora/column/arch/painting in the 397–450 wu band, the
   `sample_live_card_gol` term is the clamped edge texel, not the entity's own
   cell. Written to `entity_ground_atlas` and read by the VS next frame. Mitigated
   in practice: `placement_dirty` is re-raised on every patch change
   (`patch_system.hpp:806`), so an entity's `ground_y` is recomputed before the
   pawn walks near enough to see it. Not a visible bug today; it *is* a real
   covenant breach and the one row where the smear is stored rather than
   discarded.

2. **Finite mode breaks the covenant outright.** `band_patches` bypasses the ring
   entirely when finite (`patch_system.hpp:464`,
   `if (c->world_state_.finite_mode || d2 <= ring_sq)`), so **all** patches join
   the drawn set. With `finite_radius = 4` (the `MOOD_TABLE` max,
   `spine_state.hpp:222-225`) the world spans `[−200, +250]` = 450 wu
   (`cartridge.hpp:843-844`), and the pawn is clamped to that same box. A pawn at
   one wall is **450 wu** from the far edge — **53.125 wu beyond the guaranteed
   half-extent**. Both the patch VS and the shadow patch VS then read clamped
   card texels for that far strip. Partly masked: the terrain FS rim discard
   (`world.wgsl:3927`) is *unconditional* and still bounds shaded fragments to
   `veil_ring` = 325, so only triangles straddling the rim leak a smeared vertex
   height inward — but the **shadow** pass has no rim discard at all.

3. **Flora VS sample-before-kill.** `palm_vs` / `cactus_vs` / `blade_cluster_vs`
   sample the card and *then* test the ring (`world.wgsl:10598` vs `:10608`;
   `10935` vs `10942`; `11171` vs `11178`). The main-pass result is thrown away
   with the vertex, but `shadow_palm_vs` / `shadow_cactus_vs` /
   `shadow_blade_cluster_vs` (`world.wgsl:10619, 10953, 11189`) have **no ring
   kill** and keep the clamped value.

**Where I disagree with campaign v2 §7.5.** The covenant reads: *"card covers
±400; all live consumers inside EXIST; edge-clamp named."* Two corrections:
the guaranteed cover is **±396.875**, not ±400 (the origin snap eats up to one
cell on the + side); and "all live consumers inside EXIST" is **false for the
patch-owned families** — flora, columns, arches and outdoor paintings are
patch-lifetime, not EXIST-lifetime, and reach 450 wu.

## [anchor-8] THE REST LAW

### (a) Placement of the pulse add — **CONFIRMED: outside the gate**

`world.wgsl:8250-8282`, verbatim:
```wgsl
8250:// THE TWO-PASS WRITER (TRUEBAND_CONTACT_1 T1b — the bake's model at
8251:// card size). Pass 1 evaluates the TRUE-BAND delta (the terrain's own
8252:// waves: Σ bands of blend × Σnodes band_act (moving − frozen)) + pulses
8253:// into the stride-2 scratch; pass 2 resolves gradients (the bake's
8254:// cooperative-tile stencils) and stores vec4(h, gx, gz, gol).
8255:// terrain_time ≤ 0 ⇒ zeros (rest bit-frozen). Waking anti-teleport is
8256:// inherited: t_eff = 0 at the origin ⇒ moving ≡ frozen ⇒ a woken band
8257:// grows out of the frozen shape.
8258:@compute @workgroup_size(8, 8, 1)
8259:fn write_live_card_heights(@builtin(global_invocation_id) gid: vec3<u32>) {
8260:    if (gid.x >= LIVE_CARD_SIZE || gid.y >= LIVE_CARD_SIZE) { return; }
8261:    let texel = LIVE_CARD_EXTENT / f32(LIVE_CARD_SIZE);
8262:    let p = live_card_origin()
8263:          + (vec2<f32>(gid.xy) + vec2(0.5)) * texel;
8264:    var dh = 0.0;
8265:    if (config.terrain_time > 0.0) {
8266:        let af = terrain_activity_at(p, config.world_seed);
8267:        for (var b = 0u; b < TERRAIN_BAND_COUNT; b++) {
8268:            if (b == 4u) { continue; }   // the fine ripple stays bake-only —
8269:                                         // the Nyquist ruling (campaign v2 §6)
8270:            let blend = get_band_blend(b);
8271:            if (blend <= 0.0) { continue; }   // −1 sentinel + 0
8272:            let t_eff = config.terrain_time - get_band_phase_origin(b);
8273:            dh += clamp(blend, 0.0, 1.0)
8274:                * true_band_delta_contribution(p, config.world_seed,
8275:                      t_eff, b, af.x, af.y);
8276:        }
8277:    }
8278:    dh += contrib_radial_pulses_at(p, signal.t_seconds);
8279:    let base = (gid.y * LIVE_CARD_SIZE + gid.x) * 2u;
8280:    live_card_scratch[base]      = dh;
8281:    live_card_scratch[base + 1u] = contrib_gol_zones_at(p);
8282:}
```

The gate closes at **8277**. The pulse add is at **8278 — outside it**. The
handoff's inference is correct, and it lands harder than stated: **the banner at
line 8255 asserts the wrong law.** "terrain_time ≤ 0 ⇒ zeros (rest bit-frozen)"
is false whenever a pulse is live. The banner is not merely silent about the
conjunction — it denies it.

The pulse contributor's own early-out (`world.wgsl:2793-2794`) supplies the
second conjunct:
```wgsl
2793:fn contrib_radial_pulses_at(world_xz: vec2<f32>, t_seconds: f32) -> f32 {
2794:    if (config.pulse_count == 0u) { return 0.0; }
```

**The true rest law, stated precisely:**
```
card.r ≡ 0  ⟺  config.terrain_time ≤ 0
            AND ( config.pulse_count == 0
                  OR every live slot has age<0, age>PULSE_MAX_AGE, or amplitude<0.001 )
```
The parenthesised disjunct comes from `world.wgsl:2802`
(`if (age < 0.0 || age > PULSE_MAX_AGE || p.w < 0.001) { continue; }`) — a
`pulse_count > 0` ring whose entries have all aged out still yields 0. For a
`static_assert`-able boot pin, `pulse_count == 0` is the conjunct that matters;
the disjunct is a runtime grace note, and I would state it in the comment rather
than try to encode it.

**Note also `card.a`.** `live_card_scratch[base + 1u] = contrib_gol_zones_at(p)`
(line 8281) is **also** outside the gate and has no gate of its own. So the
*card as a whole* is only zero when a **third** conjunct holds: no active zone
with `alive_height ≥ 0.01` and `transition_fraction > 0` covers the texel
(`world.wgsl:2702-2706`). At true boot rest `zone_config.count == 0`, so it
holds — but "rest ⇒ zeros" is a three-way conjunction, not the two-way one the
handoff proposes. Worth pinning as such.

### (b) REST pins in `surface/terrain_looks.hpp` ROW 2

| pin | value | file:line |
|---|---|---|
| `REST_TERRAIN_TIME` | **`0.0f`** | `surface/terrain_looks.hpp:95` |
| `REST_PULSE_COUNT` | **`0`** (`std::uint32_t`) | `surface/terrain_looks.hpp:124` |

Verbatim, with context:
```cpp
 95:inline constexpr float REST_TERRAIN_TIME = 0.0f;                    // frozen clock
 96:inline constexpr float REST_BAND_BLEND[6] =                        // all bands inactive
 97:    { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
 98:inline constexpr float REST_BAND_PHASE_ORIGIN[6] = {};             // zero origins
```
```cpp
119:inline constexpr float REST_CHECKER_RESULTANT[3] = { 0.0f, 0.0f, 0.0f };
120:inline constexpr float REST_CHECKER_AMOUNT = 0.0f;
121:inline constexpr float REST_CHECKER_VARIANCE = 0.0f;
122:// Pulse ring rest: count 0 with a zeroed ring IS the rest (the boot
123:// pin sources it from here).
124:inline constexpr std::uint32_t REST_PULSE_COUNT = 0;
```

Both conjuncts are satisfied: `0.0f > 0.0` is false, so the band sum is gated
off; `pulse_count == 0` makes `contrib_radial_pulses_at` return 0.0 on line
2794. **Rest is bit-exact at boot** — but by the conjunction of two pins that no
single site names together.

Note the two pins live in different sub-blocks of ROW 2 (95 and 124), separated
by the 15-line CHECKER-REBUILD block. That physical separation is arguably part
of why the conjunction was never written down.

### (c) Is the conjunction stated in a comment at the writer?

**NO — and the comment that is there states the opposite.**

- The writer banner (`world.wgsl:8255`) says "terrain_time ≤ 0 ⇒ zeros
  (rest bit-frozen)" — the single-conjunct claim, which is wrong.
- The pulse block header (`world.wgsl:2770-2777`) says "Radial pulses: expanding
  ring wavefronts from note onsets" with no mention of the card or of rest.
- The C++ ROW-2 header (`terrain_looks.hpp:87-88`) says "terrain_time = beats
  (≤ 0 freezes the true-band writer evaluators — rest IS today's stillness)" —
  same single-conjunct claim, second room.
- `world.wgsl:1834` (ROW 7 banner) says "terrain_time ≤ 0 freezes **both**
  overlay evaluators" — the same false claim a third time, and the word "both"
  is itself a fossil (the wave loop it counted is retired).

**That is your commit-3 comment**, and it needs to land in four places, not one.
Suggested text for the writer, stated as the conjunction the code actually
implements:

> REST IS A CONJUNCTION, not a single gate. `card.r` is zero only when
> `terrain_time ≤ 0` **and** the pulse ring is empty — the pulse add on the line
> below sits OUTSIDE the band gate on purpose (pulses have their own clock,
> `signal.t_seconds`, and their own early-out at `pulse_count == 0`). `card.a`
> adds a third conjunct: no covering zone. Boot pins all three:
> `REST_TERRAIN_TIME` and `REST_PULSE_COUNT` (`terrain_looks.hpp` ROW 2) and
> `zone_config.count == 0`.

---

# §Q3 — THE STAGE 5 RETIREMENT CENSUS

## [anchor-9] The ten retirements — **10 of 10 ABSENT**

**Recipe (one pass, all ten):**
```
grep -rn "zone_mesh\|zoneMesh\|ZONE_MESH\|ZONE_EXTRUSION\|zone_extrusion\|SHADOW_ZONE\|\
zone_patch_instances\|zonePatchInstances\|zone_heightfield\|zoneHeightfield\|\
apply_gol_extrusion_color" --include=*.hpp --include=*.wgsl --include=*.cpp src/
```
→ **4 hit lines total**, all four `Dim::ZONE_MESH_MAX_*` (item 1's residue,
see [anchor-10a/b]). Every other pattern: **zero hits.**

| # | retirement | verdict | live reader? |
|---|---|---|---|
| 1 | zone-mesh kernels ×2 | **ABSENT** (0 WGSL entry points named `zone_gol_mesh*`) | n/a — but the *budget constants* survive, see [anchor-10a] |
| 2 | entry points ×3 (`ZONE_EXTRUSION_VS`, `ZONE_EXTRUSION_FS`, `SHADOW_ZONE_EXTRUSION_VS`) | **ABSENT at all three layers** — see per-layer table below | n/a |
| 3 | mesh buffer trio | **ABSENT** (`grep -n "zoneMesh" state.hpp` → 0 members) | n/a |
| 4 | pier entry (pre-certified) | **ABSENT from the GoL Zone layout** — `b26` now appears only in Patch Gen Layout | n/a |
| 5 | `zone_patch_instances` | **ABSENT** (0 hits tree-wide) | n/a |
| 6 | `zone_heightfield` + its sampler | **ABSENT** (0 hits for `zone_heightfield`, `zone_hf_sampler`) | n/a |
| 7 | `apply_gol_extrusion_color` | **ABSENT** (0 hits) — but its doc comment survives, see [anchor-10] extra (e) | n/a |
| 8 | drawable-table zone row | **ABSENT** — `DRAWABLES[]` (`drawable_table.hpp:104-115`) has 10 rows, no zone; no `dt_zone` thunk | n/a — but the header comment still lists "zone", see §Q5 |
| 9 | the indirect slot | **RETURNED** — `g0` registry jumps 166 → 170; slots 167/168/169 (`zone_mesh_vertices`/`_indices`/`_indirect` in the pre-S5 snapshot) are free | n/a |
| 10 | suppression triple → pair | **COLLAPSED FURTHER — it is ONE form, not a pair** | 3 live call sites |

**Item 2, per layer as requested:**

| layer | site | verdict |
|---|---|---|
| name constants (`Renderer::Entry`) | `renderer.hpp:86-91` | **ABSENT** — 3 GoL constants remain (`ZONE_GOL_SYNC`, `ZONE_GOL_EVOLVE`, `ZONE_DERIVE_PARAMS`), plus `ZONE_SEED_MASK` at `:42`; no `ZONE_EXTRUSION_*`. Line 91 is an **empty section header** — see [anchor-10c] |
| pipeline members | `renderer.hpp:274-284` | **ABSENT** — no `zoneExtrusionPipeline_` / `shadowZoneExtrusionPipeline_`. Line 281 is an **empty section header** |
| pipeline creation calls | `renderer.hpp:1445-1466` | **ABSENT** — the GoL block creates exactly 4 pipelines: `zone_gol_sync`, `zone_gol_evolve`, `zone_derive_params`, `zone_seed_mask` |

**Entry-point closure check (zero orphans, both directions).** Recipe:
```
grep -n "^@compute\|^@vertex\|^@fragment" -A1 src/cartridges/the_board/realization/world.wgsl \
  | grep "^[0-9]*-fn " | sed 's/.*fn \([a-z_0-9]*\).*/\1/' | sort > /tmp/wgsl_entries.txt
grep -oE 'constexpr const char\* +[A-Z_0-9]+ += +"[a-z_0-9]+"' \
  src/cartridges/the_board/realization/renderer.hpp | sed 's/.*"\(.*\)"/\1/' | sort > /tmp/entry_consts.txt
comm -3 /tmp/wgsl_entries.txt /tmp/entry_consts.txt
```
→ **WGSL entry points: 64. `Entry::` constants: 64. `comm -3` output: empty.**
No `Entry::` constant points at a deleted WGSL entry point, and no WGSL entry
point lacks a constant. Item 2 is closed at every layer.

**Item 10, the correction.** The campaign predicted "triple → pair". The tree
went one further: `contrib_gol_suppression_at` **does not exist as a function**
and there is exactly **ONE** suppression form.

Recipe: `grep -n "contrib_gol_suppression_at\|fn pawn_gol_suppression\|pawn_gol_suppression(" world.wgsl`
→ 8 lines: **1 definition** (`:2540`), **3 call sites** (`:3032` compute,
`:3890` patch VS, `:4175` shadow patch VS), and **4 comment-only references to
`contrib_gol_suppression_at`** (`:2700, :2960, :2963, :3016`) — a function name
that resolves to nothing. Those four are L8 tombstones; see §Q5.

The one form, `world.wgsl:2535-2543`:
```wgsl
2535:// THE ONE SUPPRESSION FORM (UNIFIED_GROUND_1) — the walker's exact
2536:// inline shape, extracted. pawn_xz is the caller's stage-appropriate
2537:// point source: compute passes qi.consumer_pos.xz, render passes
2538:// render_pawn_pos().xz. Returns the suppression FACTOR (1 at the
2539:// pawn, 0 beyond OUTER).
2540:fn pawn_gol_suppression(world_xz: vec2<f32>, pawn_xz: vec2<f32>) -> f32 {
2541:    return 1.0 - smoothstep(ZONE_SUPPRESS_INNER, ZONE_SUPPRESS_OUTER,
2542:                            distance(world_xz, pawn_xz));
2543:}
```
Corroborated by the U3 handoff's own line
(`src/docs/HANDOFFS/GPU CAMPAIGN/U 0-6/u3_decode_lift.txt:18`):
*"The suppression triple is now ONE form."* The contributor id
`CONTRIB_GOL_SUPPRESSION = 8` correctly survives in
`contracts/ground_architecture.hpp:30` — the *policy* still declares the
subtraction; only the duplicated code is gone.

## [anchor-10] The four suspected residue sites — **ALL FOUR CONFIRMED**

### (a) `Dim::ZONE_MESH_MAX_VERTICES` / `ZONE_MESH_MAX_INDICES` — **PRESENT, one reader, and that reader is itself residue**

`realization/state.hpp:249-262`:
```cpp
249:            constexpr uint32_t CUBE_SLOT_OFFSET = MAX_SPHERE_INSTANCES;
250:            constexpr uint32_t TOTAL_FLOATING_SLOTS = MAX_SPHERE_INSTANCES + MAX_CUBE_INSTANCES;  // 264
251:            constexpr uint32_t GOL_ZONE_GRID = 32;      // cells per zone side
252:            constexpr uint32_t GOL_ZONE_CELLS = GOL_ZONE_GRID * GOL_ZONE_GRID;  // 1024
253:            constexpr uint32_t GOL_ZONE_LIFE_STRIDE = GOL_ZONE_CELLS * 7;  // 7 slots: visual, velocity, target, next, height_factor, color_visual, color_velocity
254:
255:            // Zone cell mesh extrusion budget
256:            constexpr uint32_t ZONE_MESH_MAX_VERTICES = 50000;
257:            constexpr uint32_t ZONE_MESH_MAX_INDICES = 75000;
258:
259:            // Orb sky layer — luminous points on a dome above the world
260:            constexpr uint32_t MAX_ORBS = 256;
261:
262:            // Agent system — unified entity layer. Slot 0 is the player's
```
**Readers:** exactly two, both in the boot log at `state.hpp:3525-3526` — i.e.
site (b). Delete (b) and these two constants plus the `// Zone cell mesh
extrusion budget` header at 255 become unreferenced and go with it.

### (b) The boot line — **PRESENT, and it is the exact L8 shape**

`realization/state.hpp:3514-3532`:
```cpp
3514:
3515:                // Zero-init the config buffer
3516:                GPUGoLZoneArray empty{};
3517:                device_.GetQueue().WriteBuffer(zoneConfigBuffer_, 0, &empty, sizeof(GPUGoLZoneArray));
3518:
3519:                std::cout << "[GPUState] GoL zone buffers: " << Dim::MAX_GOL_ZONES
3520:                    << " zones × " << Dim::GOL_ZONE_GRID << "×" << Dim::GOL_ZONE_GRID << " grid\n";
3521:
3522:                // Zone cell mesh extrusion buffers
3523:
3524:                std::cout << "[GPUState] Zone mesh buffers: "
3525:                    << Dim::ZONE_MESH_MAX_VERTICES << " vert, "
3526:                    << Dim::ZONE_MESH_MAX_INDICES << " index capacity\n";
3527:
3528:                // Pawn aura buffers
3529:                // LATENT[gate-a-shared] pawn_aura (SH·mb): config/cells buffers + Pawn Aura group + pawnAura pipeline droppable, but pawnAuraTexture_ (created in createTextures) is sampled by the terrain FS → bound in Render Texture + Compute Texture groups. Retire = re-section those two groups.
3530:                pawnAuraConfigBuffer_ = makeBuffer("Pawn Aura Config",
3531:                    sizeof(GPUPawnAuraConfig),
3532:                    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst);
```

**Still printed: YES.** Unconditional; every boot prints
`[GPUState] Zone mesh buffers: 50000 vert, 75000 index capacity`.
**Buffers still created: NO.** Line 3522 is a bare section header with **nothing
under it** — the `makeBuffer` calls it once introduced are gone. Contrast the
`// Pawn aura buffers` header at 3528, which is immediately followed by its
`makeBuffer` calls.

This is the precise L8 shape the handoff describes: a log reporting capacity for
buffers that do not exist. It is also actively misleading at the console — a
reader debugging VRAM would count 50000 verts of zone mesh that were never
allocated.

### (c) `renderer.hpp` empty section headers — **CONFIRMED, and there are THREE, not one**

Recipe: `grep -n "Zone extrusion\|Zone mesh gen" src/cartridges/the_board/realization/renderer.hpp`
→ **5 hits** (`:91`, `:278`, `:281`, `:630`, `:713`), of which **4** head an
empty region and 1 mislabels a survivor.

**(c-1) `renderer.hpp:83-96` — the `Entry::` namespace:**
```cpp
83:            // GPU frustum culling (every frame, after compute_vp)
84:            constexpr const char* FRUSTUM_CULL_PATCHES = "frustum_cull_patches";
85:
86:            // GoL zone compute (zone-local automaton)
87:            constexpr const char* ZONE_GOL_SYNC = "zone_gol_sync";
88:            constexpr const char* ZONE_GOL_EVOLVE = "zone_gol_evolve";
89:            constexpr const char* ZONE_DERIVE_PARAMS = "zone_derive_params";
90:
91:            // Zone extrusion rendering
92:
93:            // GPU Entity Mesh Gen (Phase 2: Arches, Phase 3: Columns — pyramid mesh-gen CUT)
94:            constexpr const char* ARCH_MESH_GEN = "arch_mesh_gen";
95:            constexpr const char* COLUMN_MESH_GEN = "column_mesh_gen";
96:            constexpr const char* PALM_MESH_GEN = "palm_mesh_gen";
```

**(c-2) `renderer.hpp:272-286` — the pipeline members:**
```cpp
272:            wgpu::RenderPipeline  orbRenderPipeline_;
273:
274:            // GoL zone compute pipelines (dedicated layout, z-dispatched per zone)
275:            wgpu::ComputePipeline zoneGolSyncPipeline_;
276:            wgpu::ComputePipeline zoneGolEvolvePipeline_;
277:
278:            // Zone mesh gen (two-group: compute entity + mesh gen)
279:            wgpu::ComputePipeline zoneDeriveParamsPipeline_;
280:
281:            // Zone extrusion render
282:
283:            // Fade overlay (fullscreen alpha-blended triangle)
284:            wgpu::RenderPipeline fadeOverlayPipeline_;
285:
286:            // GPU entity mesh gen (Phase 2: arches, Phase 3: columns — pyramid mesh-gen CUT)
```
Two defects here: line 281 heads nothing, **and** line 278 (`// Zone mesh gen
(two-group: compute entity + mesh gen)`) now labels
`zoneDeriveParamsPipeline_` — a survivor that is neither mesh gen nor two-group
(it uses the shared single GoL layout, `renderer.hpp:1453-1458`). A mislabel is
worse than an empty header.

**(c-3) `renderer.hpp:708-720` — the draw verbs:**
```cpp
708:                pass.SetPipeline(bladeMeshGenPipeline_);
709:                pass.SetBindGroup(0, group);
710:                pass.DispatchWorkgroups(Dim::MAX_BLADE_INSTANCES, 1, 1);
711:            }
712:
713:            // Zone extrusion rendering
714:
715:
716:            void draw_patch_terrain_lod0_indirect(
717:                wgpu::RenderPassEncoder& pass,
718:                wgpu::BindGroup entityBindGroup,
719:                wgpu::BindGroup textureBindGroup,
720:                wgpu::Buffer indexBufferLOD0,
```

**(c-4) `renderer.hpp:628-634` — a fourth, same family:**
```cpp
628:            }
629:
630:            // Zone mesh gen (single group — same layout as sync/evolve)
631:
632:            // Zone parameter derivation (GPU-authoritative tier selection + Gaussian sampling)
633:            void dispatch_zone_derive_params(
634:                wgpu::ComputePassEncoder& pass,
```

### (d) `zone_patch_instances` — **ABSENT; the 169-vs-225 mismatch is DEAD**

Recipe: `grep -rn "zone_patch_instances\|zonePatchInstances" src/` → **0 hits.**
Also `grep -rn "\b169\b" --include=*.wgsl src/` finds no surviving hard-coded
count. The earlier audit's live-and-wrong finding retired with its subject; there
is nothing for commit 4 here. **Dead → the ruling is "already deleted", not
"delete".**

### Eight more residue sites the handoff did not list

**(e) `world.wgsl:5585` — an orphaned doc comment for a deleted function.**
```wgsl
5581:    let alive_color = clamp(base_color * dark_factor + vec3(r_shift, g_shift, -r_shift), vec3(0.0), vec3(1.0));
5582:    return mix(base_color, alive_color, blend * GOL_TINT_STRENGTH);
5583:}
5584:
5585:// Extrusion block color: starts from per-cell terrain color, applies mode
5586:
5587:struct GoLZoneArray {
5588:    count: u32,
5589:    t_beats: f32,
5590:    dt: f32,
5591:    tick_mask: u32,              // bit N = zone N should tick Conway this frame
```
This is `apply_gol_extrusion_color`'s header comment, left behind when the
function was cut (item 7). Textbook L8.

**(f) `world.wgsl:8228-8234` — an empty `§7.3` section.**
```wgsl
8226:    textureStore(zone_life_tex_write, cell, i32(zone_id), vec4(visual, color_visual, 0.0, 0.0));
8227:}
8228:
8229:
8230:// §7.3 GOL ZONE MESH GENERATION — Cell extrusion geometry
8231:
8232:
8233:
8234:// ═══ §7.3b THE LIVE CARD (GROUND_CARD_1) ═══════════════════════════════
8235:// The per-frame deformation field. The writer CALLS the existing
8236:// evaluators at texel centers — one derivation, one new sampling
```
The section numbering `§7.3b` now depends on an empty `§7.3` to make sense.

**(g) `state.hpp:1655` — the C++ twin of (f).**
```cpp
1647:            // GoL zone system buffers
1648:            wgpu::Buffer zoneConfigBuffer_;        // GPUGoLZoneArray storage (read_write)
1649:            wgpu::Buffer zoneDeriveRequestBuffer_; // GPUZoneDeriveRequestArray uniform
1650:            wgpu::Buffer zoneLifeBuffer_;          // life state: MAX_ZONES × 4096 floats
1651:            wgpu::Texture zoneLifeTexture_;        // 32×32 × MAX_ZONES r32float texture array
1652:            wgpu::TextureView zoneLifeWriteView_;  // storage texture write (compute)
1653:            wgpu::TextureView zoneLifeReadView_;   // sampled texture read (fragment)
1654:
1655:            // Zone cell mesh extrusion buffers
1656:
1657:            // Pawn aura system
1658:            wgpu::Buffer pawnAuraConfigBuffer_;    // GPUPawnAuraConfig uniform
1659:            wgpu::Buffer pawnAuraCellsBuffer_;     // GPUPawnAuraCell[] storage (N×N toroidal grid)
1660:            wgpu::Texture pawnAuraTexture_;         // N×N RGBA16Float (compute writes, FS reads)
1661:            wgpu::TextureView pawnAuraWriteView_;   // storage texture write (compute)
```
**Two bonus stale facts in the same block, both wrong numbers:**
- Line 1650: "MAX_ZONES × **4096** floats" — the real stride is
  `GOL_ZONE_LIFE_STRIDE` = **7168** (7 fields × 1024, `state.hpp:253`). 4096 is
  the pre-7-field number.
- Line 1651: "32×32 × MAX_ZONES **r32float**" — the actual format is
  **RG32Float** (`state.hpp:3496`, `desc.format = wgpu::TextureFormat::RG32Float`).
  Same wrong word repeated at `world.wgsl:5676`'s neighbourhood context.

**(h) `cartridge.hpp:1779` — the FAMILY_DISPATCH gol row's trailing comment.**
```cpp
1774:            { dispatch_select_cube_generic, dispatch_place_cube_generic, dispatch_commit_cube_generic,
1775:              evict_cube, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
1776:              "cube" },  // no CPU mesh gen — GPU compute handles update_cube
1777:            { dispatch_select_gol, dispatch_place_gol, dispatch_commit_gol,
1778:              evict_gol, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
1779:              "gol" },   // zone mesh gen is a separate compute pass
1780:            { dispatch_select_gallery, dispatch_place_gallery, dispatch_commit_gallery,
1781:              evict_gallery, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
1782:              "gall" },
```
"zone mesh gen is a separate compute pass" — there is no zone mesh gen. The row
routes to `dispatch_mesh_gen_none` because GoL has **no mesh at all** now, the
same reason as pyramid (`cartridge.hpp:1748`, which states its reason correctly).

**(i) `drawable_table.hpp:12` — the PIXEL-SAFETY roster still lists "zone".**
```
 10:// PIXEL-SAFETY. Every drawable IN THIS TABLE is OPAQUE (depth-tested,
 11:// depth-write, no blend — or an alpha=1.0 output that makes SrcAlpha a
 12:// no-op): terrain(fork), zone, pawn, sphere, monolith, ribbon, arch,
 13:// column, palm, cactus, blade, shell, and the gallery/wall FORKS. Draw
 14:// order among OPAQUE geometry is immaterial — the depth test resolves
 15:// visibility identically regardless of order — so the ONE canonical order
 16:// (the shadow order) reproduces every pass pixel-for-pixel, and the ribbon
```
The table below it (`:104-115`) has ten rows and no zone. "Every drawable IN THIS
TABLE" is a claim about the table; the list backing it names a drawable that left.

**(j) `contracts/ground_architecture.hpp:155-178` — a comment that contradicts the
code four lines below it.**
```cpp
155:    // Terrain-render — the fused render-side set: the baked heightfield
156:    // (static base + pyramids) + pawn aura + terrain waves + radial
157:    // pulses. Deliberately NO CONTRIB_GOL_ZONES: the patch heightfield
158:    // does not cache GoL; zones render as their own extrusion pass.
159:    // This policy has NO query_ground_* function by design — its
…
172:    { POLICY_TERRAIN_RENDER, "terrain_render",
173:      GROUND_STATIC_BASE_MASK
174:        | (1u << CONTRIB_PYRAMIDS)
175:        | (1u << CONTRIB_TERRAIN_WAVES)
176:        | (1u << CONTRIB_RADIAL_PULSES)
177:        | (1u << CONTRIB_GOL_ZONES)   // realized as the card's .a, cell-nearest, pawn-suppressed — UNIFIED_GROUND_1 (DAG: GoL has no ancestors)
178:        | (1u << CONTRIB_PAWN_AURA) },                  // realized in the fused VS (texture .yz + analytic wave gradient)
```
Line 157-158 says "**Deliberately NO** CONTRIB_GOL_ZONES … zones render as their
own extrusion pass"; line 177 **sets** `CONTRIB_GOL_ZONES` and correctly explains
why. The stale sentence was not deleted when the mask was updated. This is the
worst of the tombstones because the file is the *canonical registry* for the
policy sets — a reader trusting the prose gets the opposite of the law.

**(k) `world.wgsl:2531` — the suppression radii header.**
```wgsl
2531:// --- Pawn GoL suppression radii (shared between height_at + extrusion VS)
2532:const ZONE_SUPPRESS_INNER: f32 = 4.0;   // full suppression inside this radius
2533:const ZONE_SUPPRESS_OUTER: f32 = 15.0;  // zero suppression beyond this radius
```
"shared between height_at + extrusion VS" — the extrusion VS is gone; the
sharers are now the compute policy and the two patch VS.

**(l) `world.wgsl:2890` — the baked-path banner still cites the zone mesh as
its typical consumer.**
```wgsl
2886:// --- Baked heightfield: all static, no dynamic, no deformation ---
2887:
2888:// POLICY_BAKED_HEIGHTFIELD — what the cached patch heightfield texture caches.
2889:// Contributors: contrib_static_base_at + CONTRIB_PYRAMIDS.
2890:// Typical consumers: zone-mesh analytical fallback, any compute that wants
2891://   the ground-without-dynamics. The texture variant is sample_terrain_y_at.
2892:// Notes: must stay consistent with ground_formed_with_complexity (the
2893://   two-pass patch heightfield generator) — same contributor set.
```
The "zone-mesh analytical fallback" is the pier chain campaign v2 §3 pre-certified
for removal at Stage 5. It was removed; the citation was not.

## [anchor-11] Two Stage-5 items to confirm LANDED

### (a) The mask — does derive write the vocabulary predicate into `height_factor`?

**LANDED.** `world.wgsl:5889-5916`:
```wgsl
5889:// THE VOCABULARY MASK (UNIFIED_GROUND_1 U5) — the birth-moment kernel.
5890:// Multiplies the color system's REST discrete-visibility predicate into
5891:// the CPU-Gaussian-seeded height_factor plane: smooth ground does not
5892:// extrude, lift, or carry walker height. STATIC at birth (the dynamic,
5893:// tide-following form is Layer E — campaign v2 §9).
5894:// GRANULARITY TRUTH: the predicate is evaluated at ZONE-cell centers,
5895:// which ARE color-mosaic cells (extent = grid × 3.125, corner cell-
5896:// snapped) — one address, no resampling.
5897:// ORDERING LAW: derive writes zone_config[slot]; this kernel reads it —
5898:// sequential dispatches in ONE pass suffice (storage-buffer visibility
5899:// between dispatches is guaranteed).
5900:@compute @workgroup_size(8, 8, 1)
5901:fn zone_seed_mask(@builtin(global_invocation_id) gid: vec3<u32>) {
…
5910:    let vis = step(0.5, discrete_visibility_rest(center, cell_address(center)));
5915:    zone_life[idx] = zone_life[idx] * vis;   // Gaussian seed × mask
```

Two precisions on the premise: (i) it is **not** `zone_derive_params` that writes
the mask — it is a **separate kernel**, `zone_seed_mask`, on its own layout
(`renderer.hpp:1460-1466`) and its own dispatch; derive only writes
`zone_config[slot]`. (ii) The two are correctly ordered as sequential dispatches
in one pass, and the ordering law is asserted in prose at both ends
(`world.wgsl:5897-5899` and `gol_zones.hpp:645-647`). Landed, and landed cleanly.

### (b) LOD1 — old topology + card.a lift, no curtains?

**LANDED, exactly as specified.**

*Old topology, no curtains:* the LOD1 index buffer (`state.hpp:3116-3149`) emits
only legacy grid indices (`i00 = (z*step)*stride + (x*step)`, stride 65) plus a
coarse skirt ring. It never references `Dim::UG_CAP_BASE` or `Dim::UG_BASE_BASE`,
so no cap or curtain triangle can appear in an LOD1 draw:
```cpp
3117:                    constexpr uint32_t step = Dim::PATCH_MESH_N / Dim::PATCH_MESH_N_LOD1;  // = 2
3118:                    constexpr uint32_t stride = Dim::PATCH_MESH_N + 1;  // 65 verts per row
3119:                    std::vector<uint32_t> idx;
3120:                    idx.reserve(Dim::PATCH_INDEX_COUNT_LOD1);
3121:                    for (uint32_t z = 0; z < Dim::PATCH_MESH_N_LOD1; z++) {
3122:                        for (uint32_t x = 0; x < Dim::PATCH_MESH_N_LOD1; x++) {
3123:                            uint32_t i00 = (z * step) * stride + (x * step);
```

*card.a lift present:* `ug_decode` gives the legacy band `lift_scale = 1.0` and a
derived owning cell (`world.wgsl:307-312`), and the shared VS applies
`ug_cell_lift × (1 − suppression) × lift_scale` unconditionally
(`world.wgsl:3889-3891`). So an LOD1 vertex takes the same cell-slab lift as an
LOD0 cap vertex — only the curtains are absent, which is exactly "distance hides
the slope".

## [anchor-12] BUDGET RECOUNT

**Recipe:** `python3 audit/cc6_layout_budgets.py` — the CC-6 machine counter,
run unmodified against current `state.hpp` + `binding_registry.hpp`.
It parses all 25 `BindGroupLayout` blocks, resolves `bind::gN::` constants, and
crosses each against both laws. **`declared_entry_count == parsed_entry_count`
for all 25 layouts** (no parser gap), and **`flags` is `[]`.**

| layout (§5 name) | campaign prediction | **ACTUAL** (storage / uniform) | verdict |
|---|---|---|---|
| Compute Entity | after S4: **5/5** | Compute **5 s / 5 u** (+1 tex, 1 smp) | ✅ **MATCH** |
| Compute Texture (g1) | after S4: **2/1** (tex/smp) | Compute 0 s / 0 u, **2 tex / 2 smp** | ⚠ **tex matches, samplers are 2 not 1** |
| Entity Placement | after S4: **5/1** | Compute **5 s / 1 u** (+1 tex, 1 storage-tex, 1 smp) | ✅ **MATCH** |
| Frustum Cull | after S1: **4/1** | Compute **4 s / 1 u** | ✅ **MATCH** |
| Ribbon | after S1: **2/1** | Compute **2 s / 1 u** | ✅ **MATCH** |
| Photographer | after S1: **5/2** | Compute **5 s / 2 u** (+1 tex, 1 smp) | ✅ **MATCH** |
| GoL Zone | after S5: **2/2** | Compute **2 s / 2 u** (+1 storage-tex) | ✅ **MATCH** |
| Render Entity | "8 VS / 7+1 FS — unchanged" | VS **7 s / 7 u** (+1 tex); FS **6 s / 3 u** | ⚠ **one storage LOWER on each stage than the table predicted** |

**Banner law (10 storage / 12 uniform per stage): SATISFIED EVERYWHERE.**
Worst storage stage is Render Entity VS at 7/10 (3 free); worst uniform stage is
Render Entity VS at 7/12 (5 free). Under the stricter WebGPU-default 8/12 law
the tool also flags nothing.

**Render Entity, in the detail the handoff asked for.** The earlier audit's
"10/10 VS and FS" does **not** reproduce. Machine count, 18 entries:

```
b1   uniform  VS,FS      b320 storage  FS         b390 sampled-tex VS
b201 storage  VS,FS      b321 storage  FS         b391 storage     VS
b260 storage  VS,FS      b322 storage  FS         b400 storage     VS
b280 storage  VS,FS      b340 storage  VS         b411 uniform     VS
b300 uniform  VS,FS      b360 uniform  VS         b111 uniform     VS
b25  uniform  VS,FS      b361 storage  VS         b112 uniform     VS
```
→ **VS: 7 storage, 7 uniform, 1 sampled texture. FS: 6 storage, 3 uniform.**
Three storage slots and five uniform slots of headroom on the worst stage. The
campaign's "the Grade-B `zone_params` VS widen is AVOIDED" held — `zone_params`
(g1:32) stays FS-only, in Render Texture Layout.

**Two things the §5 table does not model, worth recording.** (i) The per-stage
device limit applies to the **sum across bind groups**, not per layout. Combined
pipeline totals: terrain/entity render = VS 7 s / 7 u / 4 tex / 2 smp,
FS 7 s / 3 u / 6 tex / 3 smp (g0 Render Entity + g1 Render Texture); compute
entity = 5 s / 5 u / 3 tex / 3 smp (g0 + g1 Compute Texture). Still inside 10/12
on every stage. (ii) Two layouts the §5 table predates now exist and should be
added to it: **Live Card Writer** (3 s / 2 u / 1 storage-tex) and **Zone Mask**
(2 s / 3 u).

**The Compute Texture delta explained.** The layout is
`b22 bilinear_sampler (Filtering)`, `b23 nearest_sampler (NonFiltering)`,
`b33 pawn_aura_read`, `b34 live_card_read` — 2 textures, 2 samplers. The §5 row
predicted the CT nearest_sampler would be harvested at Stage 1 ("CT
nearest_sampler" is named in the Stage-1 harvest list) and the card read added at
S4, giving 2/1. The card read landed; **the nearest_sampler harvest did not** —
and it *could not have*, because `sample_live_card_gol` (`world.wgsl:8247`) needs
`nearest_sampler` in exactly the compute stages this layout serves. The
prediction was self-defeating; the actual 2/2 is correct and the §5 row is what
is wrong.

**Two closure checks run alongside, both clean:**

*Binding mirror (C6/L6).* Recipe: a fresh cross of `@group/@binding` declarations
in `world.wgsl` against `binding_registry.hpp` constants (the shipped
`audit/cc7_mirror_cross.py` reads a **stale** `cc7_output.json` snapshot and
still reports `zone_heightfield`, `zone_patch_instances`, `zone_mesh_*` — do not
trust its output; it is measuring a pre-S5 file). Fresh count:
**95 WGSL declarations over 92 slots; 92 registry constants over 92 slots;
zero orphans in either direction; three documented aliases**
(`g0:b1 config/fc_config`, `g0:b2 vp_data/fc_vp`, `g0:b340
patch_instances/fc_patches`). That exactly matches the banner at
`binding_registry.hpp:16-17`. The campaign's §3 "102 declarations / 97 slots /
five fc_ aliases" is the **pre-S5** figure; the retirements took 7 declarations
and 5 slots with them, and two of the five aliases.

*Stage-4/5 evictions.* `Compute Entity Layout` has no `b160/b161` (zone pair),
no `b25` (tile_grid), no `b30` (pyramids). `Entity Placement Compute Layout` has
no `b160/b161`. `GoL Zone Compute Layout` has no `b26` (pier). All four §5
eviction claims verified by the machine dump.

---

# §Q4 — THE RULED DELETIONS

## [anchor-13] `audit_entity_integrity` — **PREMISE REFUTED: one live caller, not zero**

**Recipe (tree-wide, excluding `.git`):**
```
grep -rn "audit_entity_integrity" . 2>/dev/null | grep -v "^\./\.git/"
```
→ **9 hits: 4 in `src/cartridges/`, 1 in an archived doc (`src/docs/`), 4 in
`audit/`.**

Source hits, all four:
| file:line | kind |
|---|---|
| `contracts/surface_services.hpp:183` | **declaration** |
| `contracts/surface_services.hpp:22` | SEAM comment naming it |
| `surface/patch_system.hpp:60` | **definition** (empty body) |
| `surface/patch_system.hpp:816` | **CALL SITE — live, unguarded** |

**The handoff's premise "ZERO callers tree-wide" is wrong.** There is exactly
one, at the tail of `stream_patches`, and it is not behind any `#if`
(`grep -rn "DIAG_ENTITY_LIFECYCLE" src/` → 0 hits in source; the guard that
`audit/PATCH_GEN_SPAWN_RECON.md:159` remembers no longer exists). The ruling
still stands — the body is `(void)c;` — but the edit is **four sites, not three**,
and one of them is a statement inside a live function.

**Other trees:** none. `ls src/cartridges/` → `the_board` only. `the_chord`
appears in this repo exclusively inside `audit/*.md` and `src/docs/`, never as
source. No second cartridge calls it. **Reported separately as requested: N/A.**

**Verbatim, definition (`surface/patch_system.hpp:51-66`):**
```cpp
51:inline void evict_patch_entities(MachineCtx* c, ActivePatch& patch, wgpu::Queue& queue) {
52:    for (uint32_t i = 0; i < patch.entity_ref_count; i++) {
53:        auto& ref = patch.entity_refs[i];
54:        FAMILY_DISPATCH[ref.family].evict_slot(c, ref.slot, queue);
55:    }
56:
57:    patch.entity_ref_count = 0;
58:}
59:
60:inline void audit_entity_integrity(MachineCtx* c) {
61:    (void)c;
62:}
63:
64:// ── Dynamic budgets ────────────────────────────────────────────────
65:
66:inline uint32_t count_pending_patches(MachineCtx* c) {
```

**Verbatim, call site (`surface/patch_system.hpp:810-822`):**
```cpp
810:    c->world_state_.entities_culled = update_entity_draw_visibility(c, queue);
811:
812:    // ─── Deferred uploads (one per frame max) ────────────────
813:    if (tileGridDirty) upload_tile_grid_now(tile_world_state, &tile_world_deps, queue, c->world_state_.last_center_x, c->world_state_.last_center_z);
814:    flush_pier_count(c, queue);
815:
816:    audit_entity_integrity(c);
817:
818:    // Restore radius if we capped it for finite mode
819:    if (c->world_state_.finite_mode) { c->world_state_.active_radius = savedRadius; }
820:}
821:
822:} // namespace the_board
```

**Verbatim, declaration (`contracts/surface_services.hpp:176-190`):**
```cpp
176:// THE S2/S3 BOUNDARY FACE: the patch registry is read across the
177:// boundary by the occupier commits (host->record_entity via
178:// find_patch) — the interface trio's registry member.
179:ActivePatch* find_patch(MachineCtx* c, int32_t gx, int32_t gz);
180:
181:void evict_patch(MachineCtx* c, uint32_t pi, wgpu::Queue& queue);
182:void evict_patch_entities(MachineCtx* c, ActivePatch& patch, wgpu::Queue& queue);
183:void audit_entity_integrity(MachineCtx* c);
184:uint32_t count_pending_patches(MachineCtx* c);
185:uint32_t patches_budget_this_frame(MachineCtx* c, const InputState& inputState);
186:
187:// Root-called owner verb. CALLERS: boot (init_renderer) AND the transition
188:// machine (root); OWNER: patch_system. One door, both paths — boot is a
189:// transition from nothing (LAWS L10).
190:void reset_surface(MachineCtx* c, wgpu::Queue& queue,
```

**Fourth site — the SEAM comment (`contracts/surface_services.hpp:20-26`):**
```cpp
15:// The active-patch machine: the streamed patch registry and its
16:// lifecycle (allocate → spawn → generate → evict), the frame budgets,
17:// the layer allocator, the visibility cylinder, and the per-frame
18:// streaming conductor (stream_patches).
19://
20:// SEAM[spine:active-patch-system] the ActivePatch struct, the
21://   patches_ registry, find_patch / evict_patch / evict_patch_entities /
22://   audit_entity_integrity, plus the entity_refs registry on each
23://   ActivePatch. Cross-module readers: machine/spawn_engine.hpp (commit
24://   functions call host->record_entity), bodies/ribbon.hpp (two-tip late
25://   registration), bodies/gallery.hpp (evict_paintings_for_patch via the
26://   owner-side evict_gallery), and the family dispatch eviction rows.
```

## [anchor-14] `on_patch_first_generated` — **ONLY caller confirmed; the `first_gen` local dies with it**

**Recipe:** `grep -rn "on_patch_first_generated" . | grep -v "^\./\.git/"`
→ **6 hits: 3 in `src/cartridges/`, 3 in `audit/`.**

| file:line | kind |
|---|---|
| `contracts/surface_services.hpp:222` | declaration |
| `surface/patch_system.hpp:375` | definition (empty body) |
| `surface/patch_system.hpp:406` | **the only call site** |

**CONFIRMED: exactly one caller**, inside `generate_selected_patches`.

**The real question — does anything else consume the `first_gen` edge?**
**NO. Nothing.** `first_gen` is a function-local `bool` declared at line 403 and
read once at line 405. It escapes nowhere: it is not stored, not returned, not
passed to any other call. And the `SPAWNED → GENERATED` transition has **no other
observer** — `PatchPhase::SPAWNED` is read in exactly three places, all of them
*predicates for selecting work*, none of them an edge detector:
```
grep -rn "PatchPhase::SPAWNED" src/
```
→ **5 hits**: `patch_system.hpp:70` (`count_pending_patches`), `:359` (the
**write** — `phase = PatchPhase::SPAWNED` in `spawn_selected_patches`), `:403`
(this local), `:666` (fullregen gen-candidate predicate), `:791` (steady-state
gen-candidate predicate). One write and four reads; every read asks "is it
*currently* SPAWNED?", never "did it just *stop* being SPAWNED?". The write at
`:359` is the edge's only producer and it keeps no record of having fired.

**Therefore: the local dies with the call.** Deleting the hook lets lines 401-408
collapse to:
```cpp
for (uint32_t b = 0; b < count; b++) {
    c->patch_system_state_.patches_[batchIdx[b]].phase = PatchPhase::GENERATED;
}
```
— which the compiler will further reduce. No behavior change; the hook body is
empty.

**Verbatim, definition + call site (`surface/patch_system.hpp:373-410`):**
```cpp
373:
374:// Hook: fires once when a patch transitions SPAWNED → GENERATED.
375:inline void on_patch_first_generated(MachineCtx* c, uint32_t pi, wgpu::Queue& queue) {
376:    // Galleries → entity pipeline (select_gallery_for_patch)
377:    // GoL zones → entity pipeline (select_gol_for_patch)
378:    (void)c; (void)pi; (void)queue;
379:}
380:
381:// Process heightfield generation for pre-collected patch candidates.
382:inline void generate_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
…
399:    generate_patch_batch(c, encoder, queue, batchParams, count, patchStagingOffset);
400:    patchStagingOffset += count;
401:    for (uint32_t b = 0; b < count; b++) {
402:        uint32_t pi = batchIdx[b];
403:        bool first_gen = (c->patch_system_state_.patches_[pi].phase == PatchPhase::SPAWNED);
404:        c->patch_system_state_.patches_[pi].phase = PatchPhase::GENERATED;
405:        if (first_gen) {
406:            on_patch_first_generated(c, pi, queue);
407:        }
408:    }
409:    c->world_state_.patch_instances_dirty = true;
410:}
```

**Verbatim, declaration (`contracts/surface_services.hpp:216-227`):**
```cpp
216:uint32_t collect_sorted_patches(MachineCtx* c, PatchCandidate* out,
217:    float pawn_wx, float pawn_wz, Pred&& pred, bool nearest_first);
218:bool in_priority_window(MachineCtx* c, int32_t gx, int32_t gz, int32_t cx, int32_t cz);
219:void spawn_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
220:    wgpu::Queue& queue,
221:    ThemesState& themes_state);
222:void on_patch_first_generated(MachineCtx* c, uint32_t pi, wgpu::Queue& queue);
223:void generate_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
224:    wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
225:    uint32_t& patchStagingOffset, bool& tileGridDirty,
226:    TileWorldState& tile_world_state, TileWorldDeps& tile_world_deps);
227:
```

Note also that the hook's own comment (lines 376-377) is itself a tombstone: it
points at `select_gallery_for_patch` / `select_gol_for_patch` as if they were
this hook's business, when both actually run in `spawn_selected_patches` one
phase earlier — the same misordering §Q5 [anchor-18] pins on `PatchPhase::GENERATED`.

## [anchor-15] `record_placement_bookkeeping` — **PREMISE REFUTED: FOUR callers, not one**

**Recipe:** `grep -rn "record_placement_bookkeeping" . | grep -v "^\./\.git/"`
→ **21 hits: 8 in `src/cartridges/`, 3 in `src/docs/` (archived), 10 in
`audit/`.**

Source hits, all eight:
| file:line | kind |
|---|---|
| `contracts/spawn_services.hpp:222` | **declaration** — confirmed as the handoff states |
| `machine/spawn_engine.hpp:332` | **definition** (empty body) |
| `machine/spawn_engine.hpp:330` | its section header |
| `machine/entity_pipeline.hpp:198` | **caller 1** — `generic_place` (covers all 9 generic families) |
| `bodies/gol_zones.hpp:527` | **caller 2** — `place_gol_from_selection` |
| `bodies/gallery.hpp:862` | **caller 3** — `place_gallery_from_selection` |
| `bodies/ribbon.hpp:1209` | **caller 4** — `place_ribbon_from_selection` |
| `bodies/ribbon.hpp:22` | a fifth *comment* reference in ribbon's module banner |

The handoff says "empty body, called from `generic_place`. Confirm the only
caller." **Refuted.** `generic_place` is one of four. This is not a new
discovery — `audit/SWEEP_CLOSEOUT.md:112` recorded the same count ("**4 direct
call sites**") in a prior sweep and explicitly corrected an earlier "ten callers"
claim. The ruling still stands (the body is empty), but the FIND blocks are
**seven**, not three: declaration + definition + section header + four call sites,
plus the ribbon banner mention.

**Verbatim, declaration (`contracts/spawn_services.hpp:215-228`):**
```cpp
215:bool indoor_bounds_clamp(MachineCtx* c, uint32_t family,
216:    float footprint_r, float containment_r, float& cx, float& cz);
217:PositionResult negotiate_position(MachineCtx* c,
218:    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
219:    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
220:    uint32_t rotation_seed_prop,
221:    float footprint_r, float containment_r, uint32_t family, uint32_t tier = 0);
222:void record_placement_bookkeeping(uint32_t family, uint32_t tier_idx);
223:GPUArchMeshParams build_arch_mesh_params(MachineCtx* c, uint32_t slot);
224:GPUColumnMeshParams build_column_mesh_params_from(const ActiveColumn& c);
225:GPUColumnMeshParams build_column_mesh_params(MachineCtx* c, uint32_t slot);
226:uint32_t update_entity_draw_visibility(MachineCtx* c, wgpu::Queue& queue);
227:const char* family_short_name(uint32_t family);
228:void dump_entity_census(MachineCtx* c, const char* trigger);
```

**Verbatim, definition (`machine/spawn_engine.hpp:324-340`):**
```cpp
324:        r.host_gx, r.host_gz, family, tier) == UINT32_MAX) return r;
325:
326:    r.ok = true;
327:    return r;
328:}
329:
330:// ── Helper 3: record_placement_bookkeeping ──────────────────
331:
332:inline void record_placement_bookkeeping(uint32_t /*family*/, uint32_t /*tier_idx*/)
333:{
334:}
335:
336:// ═══ MESH GEN PREPARERS + CULLING ════════════════════════════════
337:
338:// ─── Column / Arch / Pyramid mesh-gen preparers ───────────────
339:
340:// Rebuild GPUArchMeshParams from cached ActiveArch data.
```

**Caller 1 (`machine/entity_pipeline.hpp:186-200`):**
```cpp
186:        traits.rotation_prop,
187:        inst.solid_half, /*containment_r*/ inst.solid_half, traits.family_id, inst.tier_idx);
188:    if (!pos.ok) return false;
189:
190:    inst.host_gx  = pos.host_gx;
191:    inst.host_gz  = pos.host_gz;
192:    inst.cx       = pos.cx;
193:    inst.cz       = pos.cz;
194:    inst.rotation = pos.rotation;
195:
196:    inst.cached_ground_y = 0.0f;
197:
198:    record_placement_bookkeeping(traits.family_id, inst.tier_idx);
199:    return true;
200:}
```

**Caller 2 (`bodies/gol_zones.hpp:521-533`):**
```cpp
521:    plan.corner_z = sel.corner_z;
522:    plan.algorithm = sel.algorithm;
523:    plan.tick_period = sel.tick_period;
524:    plan.initial_density = sel.initial_density;
525:    plan.height_enabled = sel.height_enabled;
526:
527:    record_placement_bookkeeping(PopFamily::GOL, plan.tier_idx);
528:    return true;
529:}
530:
531:// ─── commit_gol ──────────────────────────────────────────────
532:
533:inline void commit_gol(GoLState& gs, MachineCtx* c,
```

**Caller 3 (`bodies/gallery.hpp:856-868`):**
```cpp
856:    plan.archetype = sel.archetype;
857:    plan.painting_count = sel.painting_count;
858:    plan.facing_angle = sel.facing_angle;
859:    plan.gallery_size_mean = sel.gallery_size_mean;
860:    plan.site_type = sel.site_type;
861:
862:    record_placement_bookkeeping(PopFamily::GALLERY, plan.tier_idx);
863:    return true;
864:}
865:
866:// ── commit_gallery ──
867:
868:inline void commit_gallery(GalleryState& gs, MachineCtx* c,
```

**Caller 4 (`bodies/ribbon.hpp:1202-1215`):**
```cpp
1202:    plan.vertical_amp = sel.vertical_amp;
1203:    plan.color_mode = sel.color_mode;
1204:    std::memcpy(plan.color, sel.color, sizeof(plan.color));
1205:    std::memcpy(plan.color_b, sel.color_b, sizeof(plan.color_b));
1206:    plan.checker_scatter = sel.checker_scatter;
1207:    plan.checker_hue_spread = sel.checker_hue_spread;
1208:
1209:    record_placement_bookkeeping(PopFamily::RIBBON, plan.tier_idx);
1210:    return true;
1211:}
1212:
1213:// ─── commit_ribbon ───────────────────────────────────────────
1214://
1215:// Dual entry: also called from mood.hpp::apply_mood for mood-5
```

## [anchor-16] `MoodProfile.fog_density` / `fog_color` — **ZERO READERS CONFIRMED; ruling stands**

**Recipe (the discriminating one — excludes the same-named but unrelated
`GPUDesignConfig::fog_*` and the `visual_canvas` `TargetBinding`s):**
```
grep -rn "fog_density\|fog_color" --include=*.hpp --include=*.cpp --include=*.h src/ \
  | grep -v "config_\.fog\|GPUDesignConfig\|fog_density_dst_\|fog_color_dst_"
```
→ **17 hits.** Classification:
- **4 hits in `contracts/spine_state.hpp`** — the two field declarations
  (`:186`, `:187`), the first line of the STATUS comment (`:182` — the only one
  of its four lines carrying the token), and the column header (`:218`). The six
  `MOOD_TABLE` rows do **not** match: they carry the values positionally, with
  no field name, which is exactly the hazard flagged at the end of this anchor.
- **2 hits in `realization/state.hpp:381,383`** — `GPUDesignConfig::fog_density`
  / `fog_color`. **Different struct.** Written by `set_fog`
  (`state.hpp:2290-2295`), whose only caller is `cartridge.hpp:796` reading the
  **visual-canvas** pipes, never `MoodProfile`.
- **11 hits in `src/coupling/visual_canvas.hpp`** — `fog_density_` / `fog_color_`
  / `fog_color_seg_`, members of `VisualCanvas`. **Different type.** This is the
  field-driven path that replaced the mood baseline.

**No site reads `MoodProfile::fog_density` or `MoodProfile::fog_color`.**
Corroborating recipe (member-access form, tree-wide). The `config_.` exclusion
is load-bearing: without it the trailing `\.fog_color\[` alternative also matches
the five `GPUDesignConfig` writes in `state.hpp` (`:2292`, `:2294`, `:5659-5661`),
and the raw pattern returns **5**:
```
grep -rn "m\.fog_\|profile\.fog_\|->fog_density\|->fog_color\|\.fog_color\[" src/ \
  | grep -v "config_\."
```
→ **0 hits.** Also verified negatively at the one place a reader would live:
`apply_mood_lighting` (`direction/mood.hpp:~575-588`) touches
`m.clear_color`, `m.terrain_amp_ceiling`, `m.ceiling_height`, `m.indoor` — and
never `m.fog_*`.

**No reader exists. The ruling's premise holds.** Proceed.

### Every site that needs editing — the complete FIND list

All in `src/cartridges/the_board/contracts/spine_state.hpp`. **Premise
correction: the fields are NOT in `direction/mood.hpp`.** That file holds the
mood *verbs*; the `MoodProfile` struct and `MOOD_TABLE` live in the contracts
tier. A FIND block aimed at `mood.hpp` will not apply.

| # | site | file:line |
|---|---|---|
| 1 | the two struct fields | `spine_state.hpp:186-187` |
| 2 | the INTENT / STATUS comment block (4 lines) | `spine_state.hpp:181-185` |
| 3 | the column-header comment above the table | `spine_state.hpp:218` |
| 4 | `MOOD_OPEN_DEFAULT` row | `spine_state.hpp:220` |
| 5 | `MOOD_OPEN_SUNSET` row | `spine_state.hpp:221` |
| 6 | `MOOD_INDOOR_FLAT` row | `spine_state.hpp:222` |
| 7 | `MOOD_INDOOR_VAULT` row | `spine_state.hpp:223` |
| 8 | `MOOD_FINITE_OUTDOOR` row | `spine_state.hpp:224` |
| 9 | `MOOD_FINITE_OUTDOOR_REF` row | `spine_state.hpp:225` |

**Second premise correction:** the tag string **`INTENT[mood-fog-baseline]` does
not exist.** `grep -rn "mood-fog-baseline\|INTENT\[" --include=*.hpp src/` →
**0 hits.** The tag is written in the L9 `STATUS:` form, not the bracketed form:

**Verbatim, sites 1 + 2 (`contracts/spine_state.hpp:175-193`):**
```cpp
175:    // ─── Lighting ───────────────────────────────────────────
176:    float  sun_direction[3];       // directional light vector (normalized)
177:    float  sun_color[3];           // sun RGB
178:    float  sun_intensity;          // diffuse strength
179:    float  sun_ambient;            // ambient fill strength
180:
181:    // ─── Atmosphere ─────────────────────────────────────────
182:    // STATUS: INTENT — fog_density/fog_color have ZERO readers. Fog left
183:    //   apply_mood when it went field-driven (the visual-canvas fog flush
184:    //   owns it per-frame); the authored per-mood baselines are kept as
185:    //   intent. Revive-or-delete at the panel era.
186:    float  fog_density;            // exponential fog coefficient
187:    float  fog_color[3];           // fog/horizon RGB
188:
189:    // ─── Indoor shell ───────────────────────────────────────
190:    bool   indoor;                 // true = enclosed space with ceiling
191:    CeilingType ceiling_type;      // NONE / FLAT / VAULT
192:    float  ceiling_height;         // ceiling Y (world units)
193:    float  terrain_amp_ceiling;    // indoor terrain-amp cap (0 = uncapped, outdoor)
```

**Verbatim, sites 3-9 (`contracts/spine_state.hpp:214-226`):**
```cpp
214:// has_anchor_ribbon flag (last column): mood 5 (FINITE_OUTDOOR_REF)
215://   is the only row that sets it true. The mood ID is an identifier,
216://   not a discriminator — atmospheric data is profile-driven. See
217://   SEAM[mood:L1] above for the gating call site.
218://                                  fin  r_min r_max  sun_dir                sun_color              int   amb   fog_d   fog_color               indoor  ceil       ceil_h  amp_c  clear_color            zones  aura   cull   ribbon
219:inline constexpr MoodProfile MOOD_TABLE[MOOD_COUNT] = {
220:    /* MOOD_OPEN_DEFAULT       */  { false, 2, 2, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  0.0f,  {0.85f, 0.78f, 0.72f}, true,  true,  true,  false },
221:    /* MOOD_OPEN_SUNSET        */  { false, 2, 2, { 0.96f,-0.26f,-0.13f}, {1.0f, 0.75f, 0.45f}, 0.90f, 0.20f, 0.0050f, {0.95f, 0.70f, 0.45f},  false, CeilingType::NONE,  0.0f,  0.0f,  {0.95f, 0.70f, 0.45f}, true,  true,  true,  false },
222:    /* MOOD_INDOOR_FLAT        */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f, 0.0003f, {0.15f, 0.12f, 0.10f},  true,  CeilingType::FLAT,  20.0f, 0.5f,  {0.15f, 0.12f, 0.10f}, true,  true,  false, false },
223:    /* MOOD_INDOOR_VAULT       */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f, 0.0003f, {0.15f, 0.12f, 0.10f},  true,  CeilingType::VAULT, 25.0f, 0.5f,  {0.15f, 0.12f, 0.10f}, true,  true,  false, false },
224:    /* MOOD_FINITE_OUTDOOR     */  { true,  1, 4, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  0.0f,  {0.85f, 0.78f, 0.72f}, true,  true,  true,  false },
225:    /* MOOD_FINITE_OUTDOOR_REF */  { true,  1, 4, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  0.0f,  {0.85f, 0.78f, 0.72f}, true,  true,  true,  true  },
226:};
```

**One warning for the edit.** The rows are brace-initialised **positionally**
against a struct with no designated initialisers. Removing `fog_density` and
`fog_color[3]` shifts every subsequent field by four initialisers. Get one row
wrong and `indoor`, `ceiling_type`, `ceiling_height`, `terrain_amp_ceiling`,
`clear_color[3]` and the four feature bools all take the wrong values — **and
nothing will catch it**: the file's only guard is `static_assert` on the mood
*ids* (`spine_state.hpp:232-237`), which pins row order, not column offsets.
`0.0030f` in the wrong slot compiles cleanly into `terrain_amp_ceiling`. Cut all
six rows and the header in **one** hunk, never row by row.

## [anchor-17] Verbatim FIND blocks

Every deletion site for all four rulings is quoted verbatim with surrounding
context in [anchor-13] through [anchor-16] above (14 blocks in total, each with
≥6 lines of context). Restating the site inventory as a checklist:

| ruling | sites | of which are live statements |
|---|---|---|
| `audit_entity_integrity` | **4** (decl, def, SEAM comment, call) | 1 |
| `on_patch_first_generated` | **4** (decl, def, call, and the `first_gen` local + `if`) | 2 |
| `record_placement_bookkeeping` | **7** (decl, def, section header, 4 calls) + 1 comment | 4 |
| `MoodProfile.fog_*` | **9** (2 fields, comment block, column header, 6 rows) | 0 |

**Verdict on §Q4: all four are clear to delete. Two premises about their reach
were wrong** — `audit_entity_integrity` has a caller, and
`record_placement_bookkeeping` has four. Neither changes the ruling; both change
the FIND blocks commit 4 needs.

---

# §Q5 — THE STALE COMMENT SWEEP (report only)

## [anchor-18] `PatchPhase::GENERATED` — **CONFIRMED TOMBSTONE**

**The comment (`contracts/surface_services.hpp:72-80`):**
```cpp
72:// ── The patch registry ─────────────────────────────────────────────
73:
74:enum class PatchPhase : uint8_t {
75:    ALLOCATED,      // layer assigned, tile cached, no entities yet
76:    SPAWNED,        // entities selected + placed + committed
77:    GENERATED,      // heightfield computed, gallery + GoL spawned
78:    NEEDS_REGEN,    // heightfield stale (new pier in range)
79:};
80:
```

**The ordering, verified.** GoL is `FAMILY_DISPATCH` row **10** and gallery row
**11** (zero-based; `cartridge.hpp:1777-1782`, the last two rows of a
`PopFamily::COUNT == 12` table — the handoff's "rows 11 and 10" reading, one-based
and reversed, names the same two rows).

Both are selected inside `spawn_selected_patches`, which runs at
**ALLOCATED → SPAWNED** (`surface/patch_system.hpp:350-372`):
```cpp
350:// Process entity spawn for pre-collected patch candidates.
351:inline void spawn_selected_patches(MachineCtx* c, const PatchCandidate* candidates, uint32_t count,
352:    wgpu::Queue& queue,
353:    ThemesState& themes_state) {
354:    for (uint32_t s = 0; s < count; s++) {
355:        uint32_t pi = candidates[s].idx;
356:        evaluate_theme_envelope(themes_state, c,
357:            tile_seed(c->world_state_.active_seed, c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z));
358:        select_entities_for_patch(c, c->patch_system_state_.patches_[pi].grid_x, c->patch_system_state_.patches_[pi].grid_z);
359:        c->patch_system_state_.patches_[pi].phase = PatchPhase::SPAWNED;
360:    }
361:    place_entity_queue(c);
362:    commit_entity_queue(c, queue);
```
`select_entities_for_patch` walks **all twelve** `FAMILY_DISPATCH` rows
(`machine/spawn_engine.hpp:695`, `if (FAMILY_DISPATCH[f].try_select(c, gx, gz, e))`),
gallery and GoL included. The heightfield is computed later, in a **separate**
conductor unit, `generate_selected_patches`, which is what sets `GENERATED`
(`patch_system.hpp:404`). In `stream_patches` the two are ordered
spawn-then-generate at both the fullregen path (`:659` then `:669`) and the
steady-state path (`:780` then `:794`).

**So: gallery and GoL are selected, placed and committed at ALLOCATED → SPAWNED,
strictly before any heightfield exists. The comment on line 77 is a tombstone.**
Half of it is true (`heightfield computed`); the other half (`gallery + GoL
spawned`) describes work that finished one phase earlier and belongs on line 76.

**The design is fine and I confirm the handoff's reading of it.** Every family
places pre-heightfield, and Y-correction is additive and later
(`compute_entity_placement`, `world.wgsl:8775`, whose banner at `:8759-8774`
states the world-anchored overlay ride explicitly). Only the comment lies.

Two adjacent comments carry the same lie and should ride the same commit:
- `patch_system.hpp:376-377` — `on_patch_first_generated`'s body comment
  ("Galleries → entity pipeline … GoL zones → entity pipeline") implies this hook
  is where they happen. It is not. (This one dies anyway if §Q4 anchor-14 is
  executed.)
- `contracts/surface_services.hpp:16-18` — "the streamed patch registry and its
  lifecycle (allocate → spawn → generate → evict)" is *correct*, and reads as a
  direct contradiction of line 77 eight lines below it.

## [anchor-19] Other comments whose subject the campaign deleted

Listed, not fixed. Ordered by how much a reader would be misled.

### The live-card-writer layout banner — **CONFIRMED, and the eviction HAS landed**

`realization/state.hpp:4462-4470`:
```cpp
4460:                }
4461:
4462:                // -- Live card writer layout (Group 0) -- bindings 0, 1, 160, 161, 31 --
4463:                // (GROUND_CARD_1) The writer kernel calls the existing evaluators at
4464:                // texel centers: signal (band blends + pulse clock), config (waves +
4465:                // pulses + lod_point origin), the zone pair (raw GoL lift), and the
4466:                // card's storage-texture write. NOTE: this layout is the zone pair's
4467:                // future sole home outside the GoL sim (post-H5 Compute Entity
4468:                // eviction — GROUND_CARD_1).
4469:                {
4470:                    std::array<wgpu::BindGroupLayoutEntry, 6> entries{};
```

**The eviction has landed.** Machine-verified from the CC-6 dump: `Compute Entity
Layout` (`state.hpp:3753`) binds `0, 1, 2, 60, 62, 80, 100, 110, 111, 145, 146,
152` — **no 160, no 161**. Recipe:
```
python3 audit/cc6_layout_budgets.py | python3 -c "import json,sys; d=json.load(sys.stdin); \
  print([e['binding'] for l in d['layouts'] if l['label']=='Compute Entity Layout' for e in l['entries']])"
```
The zone pair now appears in exactly three layouts: GoL Zone Compute, Zone Mask,
and Live Card Writer. Both of the first two *are* the GoL sim — so the sentence's
**claim is now true**, stated in the **future tense**. Exactly the shape the
handoff predicted: a tombstone that reads as a promise.

**Bonus defect in the same banner:** line 4462 lists "bindings 0, 1, 160, 161,
31" — **five**, but the layout declares **six** (`entries[5]` is
`bind::g0::live_card_scratch` = 32, added by TRUEBAND_CONTACT_1 at
`state.hpp:4494`). The header list was never updated. Compare the Zone Mask
banner at `state.hpp:4506` ("bindings 1, 25, 160, 161, 166"), which is correct at
five of five.

### The pre-card wave-loop description — **CONFIRMED PRESENT**

`realization/world.wgsl:2755-2770`:
```wgsl
2755:    let hc = terrain_height_and_complexity(world_xz, config.world_seed, 0.0);
2756:    let mods = tile_modifiers_at(world_xz);
2757:    let height = hc.x * mods.x + mods.y + structure_height_at(world_xz) + contrib_pyramids_at(world_xz);
2758:    return vec2(height, hc.y);
2759:}
2760:
2761:// ─── Polyphony-driven wave overlay ──────────────────────────────────────
2762://
2763:// 6 cheap directional sine waves layered on the frozen lattice terrain.
2764:// Polyphony count activates waves progressively (fine ripples first,
2765:// continental swells last). Blend ramp and phase origin prevent teleportation.
2766:// Seed-derived jitter makes each finite outdoor world feel different.
2767://
2768:// Cost: 6 sin() calls per evaluation point. Called in VS + pawn + camera.
2769:
2770:// ─── Radial pulses: expanding ring wavefronts from note onsets ──────────
```

This is the header for the 6-wave loop the card replaced at Stage 3. **The
function it documents is gone** — line 2769 is blank and the next construct is
the pulse section. Every sentence is now false: there is no 6-sin loop, no
per-evaluation-point cost, and the "Called in VS + pawn + camera" claim points at
three call sites that now read `sample_live_card`. This is the single most
misleading surviving comment in `world.wgsl`, because a reader estimating shader
cost will believe it.

### `terrain_time ≤ 0 freezes both overlay evaluators` — the rest lie, third and fourth rooms

`world.wgsl:1831-1835`:
```wgsl
1831:// ── ROW 7 — THE MOVEMENT THIRD ──────────────────────────────────────
1832:// The surface voice's motion vocabulary (moved here from §1.6 —
1833:// TERRAIN_LOOKS gather; values unchanged). REST pins live in the C++
1834:// room (ROW 2): terrain_time ≤ 0 freezes both overlay evaluators —
1835:// rest IS today's stillness; band blend -1 = inactive.
```
Two faults in one line: "**both** overlay evaluators" counts the deleted wave
loop; and `terrain_time` freezes the band sum only — never the pulses
(see [anchor-8]). Its C++ twin, `surface/terrain_looks.hpp:87-88`, carries the
same single-conjunct claim.

### The `§7.3` / zone-mesh residue family

Already itemised as [anchor-10] (e)-(l): the orphaned
`apply_gol_extrusion_color` header (`world.wgsl:5585`); the empty
`§7.3 GOL ZONE MESH GENERATION` section (`world.wgsl:8230`); the empty
`// Zone cell mesh extrusion buffers` headers (`state.hpp:1655`, `:3522`); the
four dead `contrib_gol_suppression_at` references (`world.wgsl:2700, 2960, 2963,
3016`); the extrusion-VS suppression header (`world.wgsl:2531`); the zone-mesh
fallback citation (`world.wgsl:2890`); the FAMILY_DISPATCH gol row comment
(`cartridge.hpp:1779`); the drawable-table "zone" roster
(`drawable_table.hpp:12`); and the self-contradicting POLICY_TERRAIN_RENDER
banner (`ground_architecture.hpp:157-158`).

### Two wrong numbers in comments (not "deleted subject", but same commit)

- `state.hpp:1650` — `// life state: MAX_ZONES × 4096 floats`. Actual stride is
  **7168** (`GOL_ZONE_LIFE_STRIDE`, `state.hpp:253`).
- `state.hpp:1651` — `// 32×32 × MAX_ZONES r32float texture array`. Actual format
  is **RG32Float** (`state.hpp:3496`). The correct description is three lines
  above it at `state.hpp:3491`.

### `indoor_height_cap` — three comments describing a reader that no longer exists

This one is more than a comment; see the PARKED-adjacent finding below. The three
prose sites are `state.hpp:436-441`, `world.wgsl:1586`, `mood.hpp:582-584`, plus
`contracts/indoor_module.hpp:62` ("its height cap is GPU-side (the zone mesh
kernel)") and `:74` ("height capped GPU-side").

---

# ⚠ THE ONE FINDING THAT IS NOT RESIDUE

**`config.indoor_height_cap`: a live writer, zero readers, and a lost behavior.**

**Recipe:**
```
grep -rn "indoor_height_cap" src/cartridges/           # → 6 hits
                                                       # (8 in src/; the other 2
                                                       #  are archived docs)
grep -c "config.indoor_height_cap" src/cartridges/the_board/realization/world.wgsl   # → 0
```

| site | role |
|---|---|
| `contracts/indoor_module.hpp:37` | `INDOOR_HEIGHT_CAP_FRACTION = 0.75f` — the dial |
| `direction/mood.hpp:582-586` | **live writer**, every `apply_mood_lighting` |
| `realization/state.hpp:2330-2334` | `set_indoor_height_cap` setter, dirty-flagged |
| `realization/state.hpp:442` | `GPUDesignConfig::indoor_height_cap` field |
| `realization/world.wgsl:1586` | the WGSL struct field |
| — | **no WGSL read. Anywhere.** |

The writer, `direction/mood.hpp:580-586`:
```cpp
580:    c->gpuState_.set_terrain_amp_ceiling(m.terrain_amp_ceiling);
581:    c->mood_state_.terrain_amp_ceiling = m.terrain_amp_ceiling;
582:    // The GoL cap rides beside the amp column: indoors the zone mesh
583:    // caps at the module's fraction of the ceiling; 0 disables (the
584:    // kernel select's false arm — outdoor byte-identical).
585:    c->gpuState_.set_indoor_height_cap(
586:        m.indoor ? INDOOR_HEIGHT_CAP_FRACTION * m.ceiling_height : 0.0f);
```
"the zone mesh… the kernel select's false arm" names its own dead reader. The
zone-extrusion kernel *was* the only consumer, and Stage 5 retired it — item 1 of
the retirement list — without re-homing the cap.

**Nothing else caps it.** The lift path is
`ug_cell_lift` → `sample_live_card_gol` → card `.a` ← `contrib_gol_zones_at`
(`world.wgsl:2719`):
```wgsl
2719:        return visual * zp.alive_height * height_factor * config.mode_gol_height_scale;
```
- `mode_gol_height_scale` is pinned at **1.0** at boot (`state.hpp:5668`) and
  written exactly once, by `cartridge.hpp:434`
  (`set_mode_gol_scales(1.0f, 1.0f); // GoL's jurisdiction — stays inline`).
  Never mood-driven.
- `terrain_amp_ceiling` is **not** a GoL cap — its only readers are
  `world.wgsl:561-562`, clamping a terrain *wave* amplitude.

**Consequence.** `MOOD_INDOOR_FLAT` has `ceiling_height = 20.0`
(`spine_state.hpp:222`); `MOOD_INDOOR_VAULT` has 25.0. A Monolith-tier zone
(`alive_height_mean = 42.0, σ 12.0`, `gol_zones.hpp:174`) or a Pillars-tier zone
(30.0 ± 9.0) will now lift cells **through the ceiling** in an indoor mood. The
0.75 × 20 = 15 wu cap that used to clamp it is computed, uploaded, and read by
nobody.

This belongs in commit 4, not commit 3 — it is not a comment. The minimal fix is
one `min()` in `contrib_gol_zones_at` behind the existing `> 0.0` disable
sentinel; the honest alternative is to delete the field, the setter, the writer
and the five comments, and record the indoor cap as deliberately withdrawn. That
is a ruling, not a refactor, so I state it and stop.

---

# PARKED — noticed while reading, no action taken

Per the OUT OF SCOPE list. Each of these was observed incidentally; none was
investigated.

1. **`veil_ring` is never re-staged for finite mode, but the FS rim discard is
   unconditional.** `world.wgsl:3927` discards any terrain fragment beyond
   `config.veil_ring` (325) from `lod_point`, and `config.veil_ring` is written
   only once, at boot (`state.hpp:5653`). Meanwhile `band_patches` deliberately
   admits *all* patches in finite mode (`patch_system.hpp:464`) and
   `phase_stage_world` sets `veil_strength = 0` "because walls define the
   boundary there, not fog" (`cartridge.hpp:850-853`). With `finite_radius = 4`
   the world is 450 wu across and its far corners sit well beyond 325 wu from a
   wall-hugging pawn — so terrain that finite mode explicitly banded in gets
   discarded by the rim. Either `veil_strength` was meant to gate the rim discard
   too, or `veil_ring` was meant to be raised in finite mode. **This is the
   containment-column family; parked as instructed.**

2. **Finite-mode card coverage** ([anchor-7d] item 2) shares the same root and the
   same parked area.

3. **`GoLZoneSpawnConfig::MODE_THRESHOLD`** (`gol_zones.hpp:107`) is declared but
   I found no reader in `select_gol_for_patch` — the mode gate now runs through
   `compose_spawn_chance`. Not chased; it is spawn-path work.

4. **`GoLState::zone_count` vs `active_slot_count`.** `evict_gol`
   (`gol_zones.hpp:697`) does `zone_count--` unconditionally, while
   `dispatch_place_gol`'s failure arm (`:674`) and `dispatch_commit_gol`'s
   no-host arm (`:687`) clear `active` **without** having incremented it (the
   increment is in `commit_gol`, `:548`). Whether a slot can be evicted after a
   failed place — and underflow an unsigned counter — is a spawn-lifecycle
   question. **Parked for the spawning/initialization audit.**

---

# PREMISES OF THIS HANDOFF THAT I REFUTE

Recorded together, as requested.

| # | premise (handoff §) | verdict |
|---|---|---|
| 1 | anchor-13: "`audit_entity_integrity` … Confirm ZERO callers tree-wide" | **REFUTED** — one live caller, `patch_system.hpp:816` |
| 2 | anchor-15: "`record_placement_bookkeeping` … Confirm the only caller" (`generic_place`) | **REFUTED** — **four** callers |
| 3 | anchor-16: "`MoodProfile.fog_density / fog_color`" implicitly in `direction/mood.hpp` | **REFUTED** — they live in `contracts/spine_state.hpp:186-187` |
| 4 | anchor-16: "carried as `INTENT[mood-fog-baseline]`" | **REFUTED** — no such tag exists; it is written `STATUS: INTENT` (`spine_state.hpp:182`) |
| 5 | anchor-9 item 10: "suppression triple → pair (is it a pair now …?)" | **REFUTED, favourably** — it is **ONE** form with 3 call sites, plus 4 dead references |
| 6 | anchor-4: "the authored `grid_cells` value of each of the **7** tiers" | **INCOMPLETE** — there are **10** authored rows (7 Conway + 3 Pulse) |
| 7 | anchor-3(a): "Does a **12-cell** zone dispatch 12 or 32?" | **NO 12-CELL TIER EXISTS** — authored values are {8, 16, 24, 32}. Answer for any sub-32 zone: it dispatches 32 |
| 8 | anchor-11(a): "does **derive** write the vocabulary predicate into `height_factor`?" | **PARTLY** — a **separate** kernel, `zone_seed_mask`, does; `zone_derive_params` writes only `zone_config[slot]` |
| 9 | campaign v2 §7.5 (quoted as background): "card covers ±400; all live consumers inside EXIST" | **REFUTED on both halves** — guaranteed cover is **±396.875**, and patch-owned families reach **450 wu**, outside EXIST |
| 10 | campaign v2 §5: Compute Texture "after S4: 2/1" | **REFUTED** — actual is 2 tex / **2** smp, and 2/1 was never achievable (`sample_live_card_gol` needs `nearest_sampler` in compute) |
| 11 | anchor-12 background: "the earlier audit had [Render Entity] at 10/10 VS and FS" | **DOES NOT REPRODUCE** — machine count is VS 7 s / 7 u, FS 6 s / 3 u |
| 12 | anchor-10(d): "`zone_patch_instances` … if it survives, is the mismatch still live?" | **DEAD** — 0 hits; the 169-vs-225 finding retired with its subject |

---

*END OF CENSUS. No source files were modified. `git status` is clean apart from
this report.*
