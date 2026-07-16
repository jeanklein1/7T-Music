# THE RESIDUE SWEEP — RECON (read-only; the deferred clean-second)

The "clean-second" after the campaigns landed (manifold · patch-gen · cable ·
spine · registry · husks · veil/rim): what does the CURRENT system no longer
need? Scope: contracts/ · demos/ · cartridge.hpp · world.wgsl. Readers grep'd
tree-wide at HEAD (`e16baee`) — a symbol is dead only if reader-free EVERYWHERE.
`backup_board/` and `the_chord/` are SIBLING cartridges with their own copies —
NOT readers of the_board. METHOD: four parallel scope-readers + the standing-flag
inventory hand-verified. **Nothing moved; `git status` clean but for this file.**

---

## §0 HEADLINE

The refactors left a **clean** tree — most residue is **stale comments**, not dead
code. The real cuts are small and named: the `cull_*` trait fields (T1, blessed),
one empty access block (T1), a handful of discharged breadcrumbs (T0). But the
sweep surfaced **TWO things worth a ruling, not a reflex delete**:
- **A missed 4th husk** — the pyramid GROUND-ATLAS write path (binding 149). Husk-2
  kept `pyramidGroundBuffer_` as "LIVE"; the recon finds its only consumer
  (`pyramid_vs`) was cut in C2 *before* husk-2, so it writes an atlas slot nobody
  samples. A genuine binding-slot husk (the §4 re-index pattern) — CORRECTS the
  husk-2 record.
- **The wide-`EntityFamilyTraits` debt** — beyond the blessed `cull_*` pair, **10
  more fields** of that struct are zero-reader (the pipeline reads the per-family
  `*Config`/`*Prop` constants directly, not traits). A contract-face shrink; its
  own ruling (each field cut shifts all 9 positional rows).

Two ship/TESTING flags confirmed un-reverted (ribbon `SPAWN_CHANCE 0.900`;
test-rig piers). `demos/` is **100% live** — nothing to cut inside it.

---

## §1 THE STANDING-FLAG INVENTORY (confirmed at HEAD)

| flag | status at HEAD | class | tier |
|---|---|---|---|
| **cull_base / cull_height_scale** | 2 fields `entity_types.hpp:140-141` + 9 positional rows; ZERO `.cull_*` readers tree-wide (the veil's RING authority removed the last reader) | GENUINELY-DEAD | T1 |
| **patch-gen dead register → C6** | C6 + husk sweep LANDED; the "→ C6" forward-pointers are now discharged: `world.wgsl:4422, 5049, 8372`, `entities.hpp:638`, the terrain-husk "follow-on" note `world.wgsl:5603` | STALE-BREADCRUMB | T0 |
| **query_ground_placement_*** (SMP-a) | LIVE caller chain: `query_ground_placement_{pyramid,painting,vegetation}` (`world.wgsl:2665/2678/2696`) ← `manifold_height_hf` switch (3017-19) ← `manifold_position`/`manifold_resolve` ← live compute pipelines. Runtime-latent (no consumer passes policy 0/1/2 today) — the comment @8223 is accurate. | LATENT-INTERFACE | PROTECT |
| **ribbon SPAWN_CHANCE 0.900f** | NEVER reverted — still `0.900f` (`ribbon.hpp:93`); ship value `0.400f`; the "revert before ship" note (`spawn_engine.hpp:77-78`) stands | TESTING-OVERRIDE (behavior) | ship-revert (rig) |
| **GLOBAL_ENTITY_DENSITY = 1.0** | 3 live readers (`spawn_engine.hpp:181`, `gallery.hpp:718`, `gol_zones.hpp:315`, all `adj_mod *= it`); identity no-op today but a wired density knob | LATENT-INTERFACE | PROTECT |
| **GPUPierInstance.tier** | `evaluate_pier` reads is_active/origin/rotation/half_size/edge_blend/height_* — NOT `.tier`; no WGSL reads `pier.tier` (the `.tier` reads @9418/9547 are the column/antenna param struct). Accurate retained C++/GPU layout-parity metadata. | LATENT-INTERFACE | PROTECT |

---

## §2 DISPOSITION BY SCOPE

### contracts/ — clean but for the trait fields
- **cull_base + cull_height_scale** — DEAD (2 fields + the 9 rows below). See §1. **T1.**
  Rows: `COLUMN_TRAITS entity_pipeline.hpp:371` · `ANTENNA :383` · `PYRAMID :726` ·
  `ARCH :881` · `BLADE entities.hpp:953` · `PALM :1151` · `CACTUS :1361` ·
  `SPHERE spheres.hpp:154` · `CUBE cube_behaviors.hpp:512`.
- **the WIDE-TRAITS debt (ESCALATE, §4a)** — 10 MORE zero-reader `EntityFamilyTraits`
  fields (`short_name, max_instances, grounded, creates_ground, piers_per_entity,
  has_footprint, spawn_roll_prop, spawn_chance, mood_multiplier, gpu_ground_y`,
  `entity_types.hpp:134-153`); the pipeline gets these from the per-family
  `*Config`/`*Prop` constants passed to `run_spawn_preamble`, not traits. **T3** (its
  own ruling — each cut re-columns 9 rows; no `DEAD` marker yet, unlike the cull pair).
- **veil-retired constants** — GONE from the_board; only 2 tombstone comments remain
  (`surface_services.hpp:79`, `state.hpp:66`) pointing at discharged veil work. **T0.**
  (The veil-chain explainer `surface_services.hpp:140-151` describes the CURRENT
  live arch — keep.)
- **TilePopulation / ribbon DTOs** — CLEAN. TilePopulation lives wholly in
  `population_themes.hpp`; `RibbonSelection/Placement` (`entity_types.hpp:275/297`)
  are boundary DTOs correctly consumed through the queue/placement unions. Not orphaned.
- **GLOBAL_ENTITY_DENSITY** — PROTECT (§1). **ground_architecture.hpp** — PROTECTED whole
  (POLICIES[] fold, CONTRIBUTOR_DAG, `gradient_supported`/`*_gradient` LATENT[policy-surface]).
- **record_placement_bookkeeping** (`spawn_engine.hpp:295`) — a live no-op stub (4
  callers, params commented out). Idle, not dead → leave.

### demos/ — 100% LIVE, PROTECTED whole
Two files, both live: `matrix.hpp` (the grammar/grid: `Piece`/`GRID`/`DEMO_SEED`/
`column_to_roster`/`demo_config`/golden asserts) → `demo.hpp` (the selector:
`INCUBATE_DEMO`→`DEMO`→`ROSTER`). Both `full` + `minimal` columns are build-time
selectable (`-DTHE_BOARD_DEMO=`, CMakeLists.txt:262/540/608) → neither is dead.
Nothing to cut inside the folder. Residue OUTSIDE it (T0 comment/doc): the CMake
help text `CMakeLists.txt:262` and the v0 doc `docs/demo_contract_v0.md:68-69` still
name the retired `full.hpp`/`minimal.hpp`; the "148 gate sites" magic count
`matrix.hpp:37`.

### cartridge.hpp — post-spine, 7 stale comments + 1 empty block
| # | item | line | class | action |
|---|---|---|---|---|
| 1 | terrain CPU-mirror tombstone divider | 289 | STALE-BREADCRUMB | T0 drop |
| 2 | stale layout prose "~400 lines below FamilyDispatch" | 18-22 | STALE-BREADCRUMB | T0 fix |
| 3 | "the six … adapter pairs" (5 now; pyramid's cut) | 297 | STALE-BREADCRUMB | T0 six→five |
| 4 | "prepare_pyramid_mesh_gen CUT/removed" narration | 316-320 | STALE-BREADCRUMB | T0 trim (keep rationale) |
| 5 | ROSTER path "cartridges/the_board/roster.hpp" (→ contracts/) | 368-369 | STALE-BREADCRUMB | T0 fix path |
| 6 | truncated dangling comment "…defined at file" | 361-364 | STALE-BREADCRUMB | T0 complete/drop |
| 7 | "silently dropped for the frame (recon E-1)" — the face law now build-fails | 902-905 | STALE-BREADCRUMB (superseded failure mode) | T0 (low-conf) |
| 8 | empty `private:`/`public:` block (class is all-public) | 1565-1567 | GENUINELY-DEAD (structural) | T1 trivial |
Protected: all O-#/RC + E-3/E-4/E-9 law-lines (carry rationale the asserts don't),
the veil setter, boot neutrals, transition machine, the 5 live mesh wrapper pairs.

### world.wgsl — breadcrumbs + section fixes + the pyramid-ground husk
| item | line | class | action |
|---|---|---|---|
| 4× "→ C6"/"follow-on" forward-pointers | 4422, 5049, 8372, 5603 | STALE-BREADCRUMB | T0 drop the pointer |
| §9 TOC/header still lists "pyramids" (§9.0 removed) | 126, 8915 | STALE-BREADCRUMB | T0 fix to 2 families |
| RAYMARCH/SDF + husk-sweep backward tombstones (several explain binding/§ gaps) | 1482-89, 3185, 5603, 6695, 4945, 5008, 8659, 8901, 8920-26 … | STALE-BREADCRUMB (navigational) | T0 OPTIONAL trim — many are useful gap markers; keep the gap-explainers |
| **the pyramid GROUND-ATLAS husk (ESCALATE, §4b)** | struct `PyramidGroundEntry` 8052; binding `pyramid_ground` 149 @8062; placement arm 8352-8373; `GROUND_ATLAS_PYRAMID` 5047 | GENUINELY-DEAD (as a unit) | **T2/T3 — verify + re-index** |
| PMG island / TerrainState / complexity | tombstones only | (already cut) | none |

---

## §3 THE CUT PLAN — RISK TIERS (sequenced; one commit per tier or finer)

**T0 — comment-only (breadcrumbs, stale docs; no gate but glaw1/census stay green).**
One commit. All the STALE-BREADCRUMB rows above: cartridge.hpp #1-7; world.wgsl the
4 "→ C6" pointers + the 2 §9 "pyramids" listings (+ optional tombstone trims,
keeping the gap-explainers); contracts veil tombstones (surface_services.hpp:79,
state.hpp:66); demos-adjacent (CMake:262, doc, matrix:37). Zero behavior risk.

**T1 — compiler-driven C++ (glaw1-gated, behavior-identical).**
(a) The `cull_*` sweep: delete `entity_types.hpp:140-141` → the 9 rows flag as
too-many-initializers → strip the 2 literals per row (the blessed trivial sweep).
(b) The empty `private:/public:` block (cartridge.hpp:1565-1567). Own commit(s);
glaw1 GREEN is the gate.

**T2 — blind-WGSL, rig-gated (the pyramid-ground husk's shader half).**
The placement-kernel pyramid arm + `PyramidGroundEntry` + `pyramid_ground` var +
`GROUND_ATLAS_PYRAMID` — excised atomically with its C++ binding-149 re-index (§4b).
Rig gate (WGSL is glaw1-blind): terrain + placement unchanged; pyramids still bake
(the LIVE `contrib_pyramids_at` instance path is untouched). **Do the verification
in §4b first.**

**T3 — contract-face (needs a design look, not a delete).**
The wide-`EntityFamilyTraits` shrink (§4a) — 10 more fields + their 9 rows. A
contract narrowing; ruling + own cut. Higher blast radius, no correctness urgency.

**Not a tier — ship/TESTING flags (behavior, your call):** revert ribbon
`SPAWN_CHANCE 0.900f → 0.400f` (`ribbon.hpp:93`); the test-rig-piers debug ground
(`patch_system.hpp:313 TESTING[test-rig-piers]`). Both rig/ship decisions.

---

## §4 THE TWO ESCALATIONS

### §4a — the wide-`EntityFamilyTraits` debt (contract shrink; T3)
`EntityFamilyTraits` (`entity_types.hpp:132-155`) has ~22 fields; the generic
pipeline reads ~11. The other ~12 (the `cull_*` pair + the 10 in §2) are
zero-reader because every adapter passes the per-family `*Config`/`*Prop` constant
DIRECTLY to `run_spawn_preamble`, bypassing traits. The struct is a wide contract
face most of whose columns nothing consumes — a SUPERSEDED-CONTRACT-shaped debt.
RECOMMENDATION: land the blessed `cull_*` pair (T1) now; take the remaining 10 as
its own ruling (confirm each has no future intent — e.g. `gpu_ground_y`,
`creates_ground` sound like they *should* be load-bearing but aren't; verify
they're truly vestigial vs a half-wired interface before cutting the contract).

### §4b — the pyramid GROUND-ATLAS husk (corrects husk-2; T2 + C++ re-index)
CHAIN (verified this recon): `upload_pyramid_origins` (state.hpp:2418, called LIVE
render_passes.hpp:120) → `pyramidGroundBuffer_` (binding 149) → the placement kernel
arm (world.wgsl:8352-8373) computes each pyramid's ground_y → `textureStore` to the
atlas at `GROUND_ATLAS_PYRAMID` (8373) → **nobody samples that atlas slot** (its
reader, `pyramid_vs`, was cut in the C2 orphan sweep). Every frame it computes and
writes dead output.
CORRECTION: husk-2 KEPT `pyramidGroundBuffer_` as "LIVE: placement → ground atlas →
heightfield" (state.hpp:2412 comment). That is WRONG — the heightfield bakes
pyramids via `contrib_pyramids_at` (the INSTANCE array, binding 30, LIVE and
separate); the ground ATLAS is entity-Y-correction, which only `pyramid_vs`
consumed. So husk-2 should have swept this too.
THE UNIT (a binding-slot husk, the §4 re-index pattern): C++ `pyramidGroundBuffer_`
member/alloc/isReady + `pyramid_ground_buffer()` accessor + `upload_pyramid_origins`
+ its render_passes.hpp:120 caller (build `pyramidOrigins` + upload) +
`GPUPyramidGroundEntry` struct + binding 149 in the Entity-Placement layout/group
(state.hpp:4148, 5068-5069) + `bind::g0::pyramid_ground` (registry:93); WGSL
`PyramidGroundEntry`/`pyramid_ground`/`GROUND_ATLAS_PYRAMID`/the kernel arm.
VERIFY BEFORE CUT (one grep + a design nod): confirm `GROUND_ATLAS_PYRAMID` is
sampled nowhere (this recon found only the write @8373); confirm removing the
placement arm leaves the placement kernel's live arms (arch/column/plant) intact;
then re-index binding 149 out of the Entity-Placement group (14→13) per the §4
recipe. Rig-gated (WGSL blind) + glaw1 (the C++ re-index).

---

## §5 CONFIRMED-PROTECTED (touched NOTHING)
OVERLAY_WAVES + contrib_terrain_waves_at + band_* (DORMANT DEMO-2); the `*_gradient`
scaffold + `gradient_supported` (LATENT[policy-surface]); ground_architecture
POLICIES[]/DAG; the manifold_resolve/position interface + query_ground_placement_*
(LATENT); the frozen bit-identity + floater_vocabulary hash-stable naming; the demo
grammar/matrix/DEMO contract/DEMO.seed + ROSTER; floaters-400 (documented fork); the
veil/icing/ring config + render_point_pos + all ring draw gates; GLOBAL_ENTITY_DENSITY
+ GPUPierInstance.tier (retained metadata); MoodProfile fog INTENT[mood-fog-baseline]
(DORMANT); PointBubble/active_couplings (LATENT); the spine tables/laws/validate_spine
+ the E-3/E-4/E-9 by-design lag law-lines; `contrib_pyramids_at`/PyramidInstance/
binding 30 (LIVE terrain path — NOT the dead ground-atlas of §4b).

---

## §6 DISCIPLINE
Read-only. Nothing cut, no field/comment/shader touched. `git status` clean but for
this file. RECOMMENDED SEQUENCE: T0 (one safe comment commit) → T1 (cull sweep +
empty block, glaw1) → §4b verify → T2 (pyramid-ground husk, rig) → §4a ruling → T3.
The two escalations (§4a wide-traits, §4b the missed husk) want your nod before the
cut. Full stop for the stamp.

---

## §7 APPENDIX — THE ORPHAN-FILE CHECK (closes the structural blind spot)

Symbol-level reader-greps can't see a whole file no include reaches. This appendix
computes the real closure and diffs it against disk. Method: `g++ -MM` on the glaw1
TU (the REAL root — `incubator.cpp` reaches the cartridge via
`#include RENDER_HEADER(INCUBATE_RENDER)` → `cartridges/the_board/cartridge.hpp`,
the exact header glaw1's `tu.cpp` compiles), same `-I` flags, stubs generated.

### §7a THE DIFF — ZERO ORPHANS

| on disk (find) | reachable | via |
|---|---|---|
| 36 `.hpp` | 36 | transitive include closure from `cartridge.hpp` (g++ -MM, exact preprocessor) |
| 1 `.wgsl` (`world.wgsl`) | 1 | RUNTIME-LOADED — `renderer.hpp:1244-1245` disk-search list (the only runtime file-path literal in the tree) |
| 0 `.inl` | — | none exist under the_board |

**Disk minus reachable = ∅. T-minus-1 is EMPTY** — the cheapest cut class has zero
members. Every one of the 37 files is load-bearing.

### §7b THE SPECIFIC ASKS
- **keyhole.hpp** — EXISTS at `contracts/keyhole.hpp` (the COMPACT-1 boilerplate
  consolidation, since relocated modules/ → contracts/). **18 includers** (every
  body/ + machine/ + surface/ file, mood, render_passes, entity_types,
  spawn/surface_services) — it IS the `Cartridge` + `wgpu::Queue` forward-decl
  contract. LIVE, in-closure, protected.
- **demos/** — exactly `matrix.hpp` + `demo.hpp` at HEAD. ✔ Both in-closure.

### §7c WHY THE CLOSURE IS DEFINE-INVARIANT (no hidden file behind a flag)
- ZERO `#if`-gated `#include` lines anywhere under the_board — the file set cannot
  change with preprocessor state.
- The demo door (`INCUBATE_DEMO`, CMake `THE_BOARD_DEMO`) is a **token-paste
  enumerator** (`DemoCol::<name>`, demo.hpp), NOT a file include — no demo name can
  pull a hidden header. (Re-confirms §2's T0 row: the CMake:262 help text
  "demos/<name>.hpp" describes the retired per-file mechanism.)
- CMake references the_board only via `file(GLOB ...)` of `*.hpp/*.inl/*.wgsl`
  (target-source/IDE listing — lists whatever exists, confers no liveness; exactly
  why this check was needed) + the `INCUBATE_RENDER/INCUBATE_DEMO` defines.
- Tools: glaw1 compiles `cartridge.hpp` (the closure root itself); the score census
  GLOBS `**/*.hpp` + reads `cartridge.hpp`/`spine_state.hpp` by name — all
  in-closure. No tool reaches a file the TU doesn't.

### §7d VERDICT
No orphan files. The sweep's cut plan gains no T-minus-1 tier; §3 stands as the
complete plan. Read-only; nothing moved.
