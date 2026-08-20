# REGAIN_1 — REPORT
Base sha: ce4717b979919e1c17bd10c5ca4164ae68240245   Branch: master (see FLAG-1)   Read: 2026-08-20

The immediates lane is retired. Both declarations are dynamic-offset uniforms,
the directive is gone, three of the floor's four arms are gone, and
`tools/wgsl_gate.py` is `naga world.wgsl` with nothing between them for the
first time since DOMESDAY_2 F3-a. No boot has been taken; §6 is what remains.

---

## 1. Parameters (every Phase 1.2 answer, each with its recipe)

### 1.1 The authority arms — BOTH HELD

```
git grep -n "requires immediate_address_space" -- src
  world.wgsl:171:requires immediate_address_space;                       [ARM A — present]

git grep -n "var<immediate>" -- src
  renderer.hpp:415   " var<immediate> and cannot compile here (R3 floor).\n";   [C++ string]
  state.hpp:2060     // retired. `patch_params` is `var<immediate>` (world.wgsl  [comment]
  world.wgsl:153     // THE SHADER NAMES ITS OWN DEPENDENCY. `var<immediate>`    [comment]
  world.wgsl:6362    var<immediate> shadow_slot: u32;                     [DECLARATION 1]
  world.wgsl:6397    var<immediate> patch_params: PatchParams;            [DECLARATION 2]
  patch_system.hpp:189 // `var<immediate>` now (world.wgsl §7.0a), ...     [comment]
```

ARM B disambiguated by `grep -nE '^\s*var<immediate>' world.wgsl` — **exactly two
declarations**, exactly `patch_params` and `shadow_slot`. The other four hits are
prose or a string literal, every one of them inside a block this campaign
rewrote. No extras. Phase 4 was clear to run.

### 1.2 (1) The two declarations, verbatim, with their § numbers

**`shadow_slot`** — world.wgsl §7.0, banner `─── THE SHADOW TILE'S LIGHT INDEX
(DOMESDAY_1 B6, R3) ───`. The banner named IMMEDIATE DATA as the carrier, named
A3's printed grant (`maxImmediateSize=64` on the Pixel) as the licence, and
carried the D2' *index-not-matrix* reasoning. Declaration:

```
var<immediate> shadow_slot: u32;
```

**`patch_params`** — world.wgsl §7.0a, banner `PROBATE_I — PatchParams rides the
immediates lane (the lane's second spend; shadow_slot, DOMESDAY_1 B6, was the
first)`. It named the g2:40 seat, the params buffer and the 225-slot staging
ladder as having left the program with it. Declaration:

```
var<immediate> patch_params: PatchParams;
```

The WGSL store type is `PatchParams` (mirror of `GPUPatchParams`, 32 B,
`static_assert(sizeof(GPUPatchParams) == 32)` in state.hpp).

### 1.2 (2) The entry points that statically access each

`shadow_slot` — read at exactly one site, inside `shadow_light_vp()`
(`frame_r.lighting.spots.lights[shadow_slot].view_proj`). Recipe:

```
grep -c "shadow_light_vp()" world.wgsl   ->  14   (13 call sites + 1 definition)
```

The **13** callers are the shadow VS family: `shadow_patch_terrain_vs`,
`shadow_pawn_vs`, `shadow_sphere_vs`, `shadow_monolith_vs`, `shadow_arch_vs`,
`shadow_column_vs`, `shadow_palm_vs`, `shadow_cactus_vs`, `shadow_blade_cluster_vs`,
`shadow_shell_vs`, `shadow_ribbon_vs`, `shadow_gallery_frame_vs`,
`shadow_wall_painting_vs`. No fragment or compute reader.

`patch_params` — 11 read sites over **3** entry points: `generate_patch_heights`,
`generate_patch_gradients`, `generate_patch_cells`. Recipe:
`grep -n "patch_params\." world.wgsl`.

Both sets are exactly what the tree's own witness computed independently:
`binding_gen --check` reported at base sha `M-2 … (2 symbols, 16 pipelines
nonzero)` — 13 shadow + 3 patchgen. The expected set and the true set agree; no
FLAG.

### 1.2 (3) Which bind group layout each consuming pipeline takes at group 2

`strataLayoutFor(label, frame, state, tex)` wraps the strata as
`{ worldLayout_, frame, state, tex }`, so **group 2 is the `state` argument**.

| pipelines | group-2 layout |
|---|---|
| the three patchgen kernels | `patchgenStateLayout_` |
| the eleven scene-family shadow VSes (`shadowRenderLayout`, renderer.hpp) | `shadowStateLayout_` |
| the two shadow ARTWORK VSes (`galleryShadowLayout`, renderer.hpp) | `shadowStateLayout_` |

**The risk the handoff named does not exist.** ATLAS_1revB G2 moved the two
artwork shadow pipelines at **group 0** (to the render-entity layout, for
`frame_r.lighting`) and their gallery pair is **group 1/3**. Group 2 was, and
is, `shadowStateLayout_` on both sibling layouts. So the shadow seat is ONE
schema row, not two, and it is seated on the same layout all thirteen take.
This is strictly better than what B6 had: B6's immediate size was a
pipeline-layout fact that had to be set on two sibling layouts, and A10 exists
because B6 set it on one and missed the other. A seat cannot be missed that way.

Recipe: `git grep -n "strataLayoutFor" -- src` (27 call sites at base sha; 5
carried a fifth argument — 3 patchgen at `sizeof(GPUPatchParams)`,
`shadowRenderLayout` and `galleryShadowLayout` at `sizeof(uint32_t)`).

### 1.2 (4) The band and the next free binding index in it

Recipe — the g2 bands, printed from `REGISTRY_NAMESPACES` with each constant's
binding:

| band | occupied at base sha | chosen |
|---|---|---|
| PATCHGEN (40–59) | 41 `patch_height_scratch`, 42 `pyramid_instances`, 43 `patch_grid` | **`patch_params` → g2:40** |
| SCENE (200–219) | 200 `scene_constants` | **`shadow_slot` → g2:201** |

g2:40 is the lowest free number in the PATCHGEN band **and** the exact seat
PROBATE_I freed — world.wgsl's own banner named it ("The g2:40 seat … left the
program with it"). Taking it back is the restoration, not a new number.

g2:201 is the lowest free number in the group-2 SCENE band. The handoff pointed
at the band holding `shadow_map` / `shadow_sampler` / `spot_shadow_map`; those
are the **group-3** SCENE band (200–202). The group-2 equivalent is the SCENE
band 200–219, confirmed, and 201 is its next free.

Both confirmed unoccupied against the module itself before use:
`grep -n "@group(2) @binding(40)\|@group(2) @binding(201)" world.wgsl` → no hits.

### 1.2 (5) The `uniform used/12` figures — NO ROW IS AT 12

`audit/MANIFEST.md` prints `used / free`, not `used / limit`.

| row | before | after |
|---|---|---|
| `generatePatchHeightsPipeline_` C | 4 / 8 | **5 / 7** |
| `generatePatchGradientsPipeline_` C | 4 / 8 | **5 / 7** |
| `generatePatchCellsPipeline_` C | 4 / 8 | **5 / 7** |
| all 13 shadow rows, V | 3 / 9 | **4 / 8** |

Worst uniform row program-wide is unchanged at 5 of 12 (`updatePlayerAgentPipeline_`
C); the patchgen three joined it there, which is why the wallet summary's "at"
column went from "+3 more" to "+6 more". The STOP condition did not fire.

### 1.2 (6) The pre-B6 shape, and the diff

```
git log --oneline -S "SHADOW_SLOT_STRIDE" -- src
  f8defbd  DOMESDAY_1 B6: shadow_slot becomes immediate data (R3) — first coin in the lane
  2177e57  ATLAS_1revB U1": shadow_slot + shadow_light_vp() — the closure commit
```

`f8defbd` read whole. Before B6, `shadow_slot` was **g1:2, a 16-byte `ShadowSlot`
struct with one meaningful field `li`**, seated on the FRAME group
(`frameRLayout_`), backed by `lightSlotBuffer_` — `MAX_SPOT_LIGHTS × 256 B`
windows, window *i* holding *i*, written once at boot — and the dynamic offset
rode the **FRAME** bind, so every pass that bound group 1 had to carry an offset
argument.

The restored shape differs in three ways, all improvements, all forced by the
LOOM_2 strata rather than chosen: it is a **bare `u32`, not a struct**, so
`SHADOW_SLOT_SIZE` is honestly 4 where it had to be 16 (a uniform struct rounds
to a 16-byte alignment, which is what B6's retired comment was explaining); it
seats on **`shadowStateLayout_`, the shadow family's own stratum**, not the
frame group that eleven unrelated pipelines share, so no other pass grows an
offset argument; and the buffer, its stride, its one boot fill and its
`buffers_ok` term come back verbatim from `f8defbd^`, label and all.

---

## 2. What was built, per phase

### P2 — `patch_params` (49d7112)

- **Schema.** DECLS `patch_params` g2:40 uniform `PatchParams`; REGISTRY row at
  the head of the g2 PATCHGEN band; SEAT `('patchgenStateLayout_', 2)` with
  `has_dynamic_offset: True`, `min_binding_size: sizeof(GPUPatchParams)`; GROUPS
  entry 2 on `patchgenStateGroup_` backed by `patchParamsBuffer_` with
  `size = sizeof(GPUPatchParams)` (the WINDOW, not the buffer); RESOURCES row for
  the buffer; the three patchgen PIPELINES rows `immediate_size: 32 -> 0`; the
  registry banner caption `86 declarations over 71 slots -> 87 over 72`.
- **Shader.** `@group(2) @binding(40) var<uniform> patch_params: PatchParams;`.
  One match. No read site changed — every one already spells `patch_params.field`.
- **Buffer.** `PATCH_PARAMS_STRIDE = 256` beside the FC geometry block, with the
  ATLAS_1revB D3" 256-alignment paragraph named as its legality argument.
  `patchParamsBuffer_` = `Dim::MAX_ACTIVE_PATCHES * PATCH_PARAMS_STRIDE` = **57,600 B**,
  `UU`. `patchParamsWindows_`, the CPU side of the single write.
- **Door.** `upload_patch_params_batch(queue, params, count)` — packs `count`
  windows at 256-byte stride and issues **one** `WriteBuffer`. See FLAG-3 for its
  return type.
- **Call site.** `(void)queue;` deleted, the door called once before the loop,
  `const uint32_t off = i * PATCH_PARAMS_STRIDE;` at the top of the `for (i)` body.
- **THE SHAPE FOUND, per §2.4's instruction to read the helpers first.** The
  three `dispatch_generate_patch_*` helpers **do** bind group 2
  (`pass.SetBindGroup(2, stateGroup)`), and each has exactly one caller. So the
  offset belongs THERE, not at the call site: each helper gained a
  `uint32_t paramsOffset` parameter and binds
  `pass.SetBindGroup(2, stateGroup, 1, &paramsOffset)`. The handoff's FIND/REPLACE
  templates were **not** used; this is the shape it anticipated might not fit.
- **Layouts.** `sizeof(GPUPatchParams)` removed from all 3 patchgen
  `strataLayoutFor` calls. Expected 3, found 3, all carrying the
  `// PROBATE_I: the patch_params immediate` trailing comment.
- **Banners.** The three false PROBATE_I banners in state.hpp replaced by one at
  the creation site saying what stands, and saying explicitly that this is NOT
  the pre-PROBATE_I ladder and why (the ladder existed for a
  `CopyBufferToBuffer` that a dynamic offset makes unnecessary). The
  `generate_patch_batch` banner's "the params no longer travel through a buffer"
  is rewritten; the RAW contract paragraph and the reason the pass pair stays are
  untouched, because both are still true.
- Also corrected: `contracts/surface_services.hpp`'s PROBATE_I note, which said
  the params "ride the pass encoder as immediate data now."

### P3 — `shadow_slot` (0034977)

- **Schema.** DECLS `shadow_slot` g2:201 uniform `u32`; REGISTRY row in the g2
  SCENE band; SEAT `('shadowStateLayout_', 6)`, `has_dynamic_offset: True`,
  `min_binding_size: SHADOW_SLOT_SIZE`, visibility `V`; GROUPS entry 6 on
  `shadowStateGroup_` backed by `lightSlotBuffer_`, `size = SHADOW_SLOT_SIZE`;
  RESOURCES row; the 13 shadow PIPELINES rows `immediate_size: 4 -> 0`; caption
  `87/72 -> 88/73`.
- **Store type confirmed against the tree**, not assumed: the floor's message
  called it one u32, `f8defbd` called it one u32 in a 16-byte wrapper, and the
  declaration was `var<immediate> shadow_slot: u32`. A bare `u32` uniform was
  verified legal by running naga on a two-line module before it was adopted.
- **Buffer.** `SHADOW_SLOT_STRIDE = 256` / `SHADOW_SLOT_SIZE = 4` restored
  directly under the D3" paragraph that argues 256's legality, with a note on why
  SIZE is 4 now and was 16 then. `lightSlotBuffer_` created at GPU-state init and
  filled once, `MAX_SPOT_LIGHTS` windows, window *i* = *i*. **No per-frame door
  exists and none was added** (YAGNI, stated in the banner).
- **Both pass sites.** The atlas arm's per-light `SetImmediates` became
  `{ const uint32_t off = li * SHADOW_SLOT_STRIDE;
  pass.SetBindGroup(2, shadow_state_group(), 1, &off); }`. The outdoor arm's
  `kLightZero` became `kLightZeroOffset` and rides the pass-head bind.
- **The pass-head bind in the atlas arm was DELETED, verified rather than
  assumed.** Every draw in that pass is inside the light loop: `draw_shadow_all`
  is called there and nowhere else in that arm, and the loop always executes at
  least once (`if (first >= live) break;` guarantees `li = first < live`). The
  tree's own witness confirms it independently — `P-seq` simulates all 20 pass
  spans in encode order and checks all 85 draw/dispatch events against the
  last-bound 2/3 pair; it passes.
- The outdoor arm keeps its pass-head bind, now with the offset argument.
- **Layouts.** The immediate size removed from `shadowRenderLayout` and
  `galleryShadowLayout`. The ATLAS_1revB G2 rationale in renderer.hpp is
  rewritten: the "since B6, the shadow_slot IMMEDIATE — a pipeline-layout fact,
  not a group member" clause is false now and is replaced with why a seat cannot
  repeat A10's near-miss.
- **Banners.** state.hpp's B6 sentence retiring `SHADOW_SLOT_STRIDE` /
  `SHADOW_SLOT_SIZE` is deleted — it is exactly what this phase falsifies. The
  D3" paragraph above it is kept and now describes something real again.
  render_passes.hpp's B6 paragraph is rewritten into the present tense with the
  load-bearing half kept and set apart: what this needs is a value that can
  change INSIDE a render pass, because a buffer write cannot be recorded there,
  and that is what keeps the one-pass-per-light split retired.

### P4 — the lane retires (09a9f09)

Precondition tested as state (P12), not as the string: `grep -nE '^\s*var<immediate>'`
over world.wgsl returned nothing, and `binding_gen --check` reported
`M-2 … (0 symbols, 0 pipelines nonzero)` — the generator computing the immediate
symbol set fresh from the module and finding it empty.

- **`tools/wgsl_gate.py`.** The `DIRECTIVE not in src` branch that split a `%s`
  from its operand is gone with the transform that needed it (see FLAG-2). The
  file is now `naga` over `world.wgsl` directly: `transform()`, `SHIM_TAG`,
  `IMMEDIATE_BASE_BINDING`, the ARM 4 naga tripwire, the tempdir and the
  blind-spot paragraph all retired. The shallow-clone note (L29) and the
  naga-absent failure are both intact. The tint arm is kept and its justification
  restated — see FLAG-4. The banner names this campaign as the reason.
- **`world.wgsl`.** `requires immediate_address_space;` removed. The language-
  extension banner is rewritten to say the module names no dependency and why
  that is the point. The COMPILER FLOOR block is recut: the Tint trio is no
  longer the floor of record, the Firefox PENDING clause and its HELD price are
  history, and the block says what the gate is now — and says plainly that no
  engine is refused by the file any more but none outside Chromium has been
  witnessed either.
- **`renderer.hpp`.** Arms 1, 2 and 3 deleted with their banner paragraphs.
  `floorHolds()` is deleted outright rather than kept as an empty predicate, and
  its call in `init()` with it — arm 4 lives in `loadShader()`, so nothing
  remained for it to check. The surviving banner names the three retired arms and
  where arm 4 is, so no reader is left hunting. Arm 4 prints the same
  `[Floor] STOP` line, so the shell's card still fires.
- **`console.hpp`.** `report_wgsl_language_features`, `wgslImmediate_` and its
  orphaned comment, the `askedImmediate` gate and its WITHHELD branch, the
  `[Device] wgsl language features` boot line, the "post-B6 shader cannot compile
  here" line, and the `maxImmediateSize` column of the granted-vs-floor census —
  all retired. **The PORT_6c/L14 core-defaults paragraph stays**, untouched; it
  was never about immediates and it is why the request has its shape. The request
  now prints `exceptions carried: none`.
- **Schema / generator.** NEEDS r7 (`maxImmediateSize`) deleted;
  `IMMEDIATE_LANE_BYTES` and the MANIFEST's immediates column deleted with it.
  R-3 now reads 6 NEEDS rows; `FLOOR_MAX_IMMEDIATE_SIZE` is gone from
  `limits_floor.gen.inc`.
- **Also retired** (FLAG-5): `strataLayoutFor`'s optional `immediateSize`
  parameter and the `d.immediateSize` it set.
- **Three stale pass-head notes corrected** (FLAG-6).

### P5 — the shell (7a6a1a5)

Jean's Appendix was not struck, so it was applied verbatim. `fallback()`'s browser
list stops being a list. `floor()` becomes "The world arrived damaged." and
`reload` flips to `true`. The PROBATE_SEAL3 banner's reload paragraph is rewritten
— its old reasoning was about a browser not growing a feature between two
presses, which is not the case this card covers any more; a truncated fetch CAN
succeed on a second try, so the button is honest here for the first time.

Both replacement subs were asserted character-for-character against the Appendix
text before the write.

### P6 — OPEN, report (this commit)

Two lines die (FIREFOX REGAIN HELD; the wgsl_gate.py TypeError), one line is
added, per §6 verbatim. One count corrected in place — FLAG-7.

---

## 3. Gate output (verbatim)

`python3 tools/wgsl_gate.py` — **the campaign's whole verdict.** naga reads
`world.wgsl` directly, with no transform between them, for the first time since
DOMESDAY_2 F3-a:

```
wgsl-gate: PASS — world.wgsl parses, scopes and validates under naga, whole and unmodified
  no shim: the module naga read is the module the browser is served (REGAIN_1 retired the transform with the immediates lane)
  NOT gated here: pipeline-layout conformance, minBindingSize and dynamic-offset alignment — Dawn's checks at pipeline creation, witnessed by the web boot (world.wgsl COMPILER FLOOR block).
  [gate] tint arm DORMANT — set T7_TINT to a tint executable to light it
```

naga 30.0.0, installed into this container per the standing OPEN item that
prices exactly this (`cargo install naga-cli`). It is the same naga 30 the old
banner named as refusing the module — and at base sha it did:
`[gate] naga-raw REJECTS the module, as expected`. That line is gone because the
thing it watched for has happened.

`python3 tools/binding_gen.py --check` — all relations agree, every witness
passes. Recorded here as of the pre-push run; **S-6 is a landing witness by
design** ("green only at the landing … a mid-flight run reports its dirt
honestly and fails; that is the point"), so it reports the uncommitted report
file and the unpushed tip. §3a below carries the post-push run.

```
RELATIONS (tree vs schema, both directions)
  DECLS       88 rows  agree
  REGISTRY    73 rows  agree
  SEATS      108 rows  agree
  LAYOUTS     29 rows  agree
  GROUPS      34 rows  agree
  PIPELINES   59 rows  agree
  REGISTRY_FILE_BANNER   agree
  REGISTRY_NAMESPACES    agree
  REGISTRY_INVARIANTS    agree

EMIT TARGETS (schema -> bytes vs disk)
  binding_registry.hpp: generated — OK
  binding_surface.gen.inc: generated — OK
  MANIFEST.md: generated — OK
  limits_floor.gen.inc: generated — OK
  [PASS] R-3  emitted floors equal the schema's 6 NEEDS rows, and every cited Dim:: symbol exists at its stated value
  features_wallet.gen.inc: generated — OK
  [PASS] F-1  the device request is exactly the schema's granted set: {TimestampQuery} (5 vaulted, unrequested)

WITNESSES
  [PASS] S-1  schema cardinalities equal the ledger's: 88/73/108/29/34/59 vs 88/73/108/29/34/59
  RESOURCES  93 rows  agree
  [PASS] R-1  every seat backing and pass-attachment view resolves to exactly one RESOURCES row (131 backings, 20 passes; the swapchain backbuffer exempt by nature)
  [PASS] R-2  every RESOURCES row is reached (bind-group entry, pass attachment, draw/dispatch argument, or copy/write site): 93 rows, 0 orphan(s)
  [PASS] R-Label  every RESOURCES row carries a label (93 rows)
  [PASS] M-2  immediate_size nonzero iff the module set statically accesses a var<immediate> (0 symbols, 0 pipelines nonzero)
  [PASS] M-1  MANIFEST lane sums equal per-seat counts on all 76 (pipeline, stage) rows; worst: uniform 5/12 (updatePlayerAgentPipeline_ C), storage 5/8 (updatePlayerAgentPipeline_ C), sampled 6/16 (patchTerrainPipeline_ F), samplers 3/16 (updatePlayerAgentPipeline_ C), storagetex 2/4 (generatePatchHeightsPipeline_ C)
  [PASS] S-2  11 Table H sites in the transcribed mirrors; every trigger matches the schema's prose
  [PASS] S-3  --write-wgsl round-trip with unchanged numbers is the identity on world.wgsl
  [PASS] S-4  every declared slot is seated in at least one layout (73 slots)
  [PASS] S-5  every pipeline reaches every slot its entry points touch through its own strata (59 pipelines)
  [PASS] P-scope(R)  4 render pass spans composed from M7; per-pass usage merge (render arm of L23'):
           render_snapshot_pass:1502   6 groups, 19 backings, 0 conflict(s)
           render_shadow_pass:338      4 groups, 13 backings, 0 conflict(s)
           render_shadow_pass:387      4 groups, 13 backings, 0 conflict(s)
           render_main_pass:566        7 groups, 19 backings, 0 conflict(s)
  [PASS] S-5b 70 buffer seats face-checked against the union of their member pipelines' reached declarations (0 reached by no member pipeline — asserted nothing)
  [PASS] S-7  expression closure over 94 emitted expressions: every free identifier resolves at the include point (346 class/namespace-scope constants + include closure)
  [PASS] P-seq 20 pass spans simulated in encode order, 85 draw/dispatch events checked against the last-bound 2/3 pair
  [PASS] P-scope(C) 29 dispatch sites snapshotted with full bound groups; single-writability per buffer, pessimistic by law; group-local: 34 groups checked
  [FAIL] S-6  commit integrity: porcelain DIRTY; HEAD 7a6a1a5 vs upstream ce4717b
```

`python3 tools/binding_ledger.py` regenerated `audit/BINDING_LEDGER.md` at every
phase, LF-CLEAN, all witnesses green. The one line worth quoting:

```
  [PASS] 0d-1       dynamic-offset bindings: 1 of 8 uniform, 0 of 4 storage, program-wide
```

At base sha that read `0 of 8`. It is a per-pipeline-layout maximum against the
core default of 8, so the two seats do not sum: `patchgenComputeLayout` carries
one, the two shadow layouts carry one, and nothing carries two.

## 3a. The landing check

Run at `7c2501f`, immediately after the push, with the tree clean and HEAD equal
to the pushed tip — the state S-6 exists to witness. **Exit status 0.**

```
  [PASS] S-6  commit integrity: working tree clean; HEAD 7c2501f == pushed tip

--check: all relations agree, all witnesses pass.
```

Every other line is identical to §3's; only S-6 moved, and only because the
landing happened. (This section is itself a later commit, so a `--check` run now
sees one more commit on top — the green above is the record of the state it
describes.)

---

## 4. Lines retired — code / comment, per site

| site | what left |
|---|---|
| `world.wgsl` §1 | `requires immediate_address_space;` — the directive |
| `world.wgsl` §1 | the language-extension banner's dependency argument (18 lines) |
| `world.wgsl` head | the COMPILER FLOOR block's Tint-trio floor, Firefox PENDING clause, HELD price, and the shim sentence (20 lines) |
| `world.wgsl` §7.0 | `var<immediate> shadow_slot: u32;` |
| `world.wgsl` §7.0a | `var<immediate> patch_params: PatchParams;` |
| `render_passes.hpp` | 2 × `SetImmediates` (per-light and outdoor); the atlas arm's pass-head group-2 bind |
| `patch_system.hpp` | 2 × `SetImmediates`; `(void)queue;` |
| `renderer.hpp` | `floorHolds()` whole — 3 arms, 1 `EM_ASM_INT` probe, 4 `[Floor] STOP` messages, 1 `__EMSCRIPTEN__` guard (~70 lines) and its call in `init()` |
| `renderer.hpp` | `strataLayoutFor`'s `immediateSize` parameter and `d.immediateSize` |
| `renderer.hpp` | 5 × the fifth argument at `strataLayoutFor` call sites |
| `console.hpp` | `report_wgsl_language_features()` and its call (~26 lines) |
| `console.hpp` | `wgslImmediate_` and its comment |
| `console.hpp` | the `askedImmediate` gate, its WITHHELD branch, the F3-b/F3-f paragraphs (~38 lines) |
| `console.hpp` | the `wgsl:immediate_address_space (instance)` half of the request line, and the loud R3-floor line |
| `console.hpp` | the `maxImmediateSize` column of `[Device] granted vs floor` + A3's probe-row paragraph |
| `state.hpp` | the B6 sentence retiring `SHADOW_SLOT_STRIDE`/`SIZE`; 3 false PROBATE_I banners |
| `binding_schema.py` | NEEDS r7 |
| `binding_gen.py` | `IMMEDIATE_LANE_BYTES`, the MANIFEST immediates column, the per-row `imm` fact, the wallet's immediates row |
| `wgsl_gate.py` | `transform()`, `SHIM_TAG`, `IMMEDIATE_BASE_BINDING`, `DIRECTIVE`, the TypeError branch, ARM 4, the tempdir, the blind-spot paragraph (~110 lines) |
| `limits_floor.gen.inc` | `FLOOR_MAX_IMMEDIATE_SIZE` |
| `MANIFEST.md` | the `immediates(bytes) /64` column, all 76 rows, and its wallet line |
| `index.html` | the FLOOR card's immediates sentence and the FALLBACK card's browser list |
| `OPEN.md` | FIREFOX REGAIN HELD; the wgsl_gate.py TypeError |

### §7's third check, with a verdict on every hit

```
git grep -c "var<immediate>" -- src        ->  (no matches)   as EXPECTED
git grep -c "SetImmediates"  -- src        ->  (no matches)   as EXPECTED
git grep -c "requires immediate_address_space" -- src  ->  world.wgsl: 2
```

`git grep -in "immediate" -- src tools`, every lane-relevant hit adjudicated:

| hit | verdict |
|---|---|
| `world.wgsl:23`, `:166` — `` `requires immediate_address_space;` `` in backticks | **HISTORY.** Both inside REGAIN_1 paragraphs naming what was retired and why. |
| `world.wgsl:27`, `:161`, `:6356`, `:6399`, `:6412` | **HISTORY.** REGAIN_1 banners naming the lane in the past tense. |
| `world.wgsl:3279`, `:3329` — "the immediate radius" | **UNRELATED.** GoL suppression geometry. |
| `render_passes.hpp:303`, `state.hpp:3993`, `binding_surface.gen.inc:859` | **HISTORY.** "less per-frame traffic than the immediates path it replaces." |
| `renderer.hpp:197`, `:340`, `:342`, `:344` | **HISTORY.** The retirement banners naming the three retired arms and the retired parameter. |
| `console.hpp:592`, `:689`, `:856` | **HISTORY.** The three retirement banners. |
| `binding_gen.py:522`, `:1645-1676` — `immediate_size`, M-2 | **LIVE, AND DELIBERATELY SO.** M-2 is the tripwire that fails the run if a `var<immediate>` reappears anywhere in the module. It reads `0 symbols, 0 pipelines nonzero` and is the only thing holding the lane at zero. §4.5 named NEEDS r7 and `IMMEDIATE_LANE_BYTES` for retirement, not M-2 — it survives on the same logic arm 4 does. |
| `binding_gen.py:859`, `:937-940` | **HISTORY.** The column's own epitaph. |
| `binding_ledger.py:1801-2084`, `:3696` — `imm_bytes_of`, the `immediate_size` slot | **LIVE, AND M-2's PLUMBING.** The `strataLayoutFor` regex already treated the fifth argument as optional (B6 taught it), so removing the parameter is transparent to it; `imm_bytes_of` now always returns 0. Retiring it would retire M-2 with it. |
| `binding_gen.py:2936` — `--bootstrap-recut`'s `sym == "shadow_slot"` special case | **FOSSIL, NOW HALF-TRUE.** A one-shot recut path, not reached by `--write`/`--check`. It hardcodes shadow_slot as the dynamic-offset seat with `SHADOW_SLOT_SIZE` — correct again as of P3, but it does not know about `patch_params`. Left alone (instrument, not subject). Named here so a future recut does not trust it. |
| `console_gate/run.py:17`, `PROVENANCE.md:74` — "the SetImmediates hole" | **HISTORY.** A past-tense account of why the gate exists. |
| everything else | **UNRELATED.** "immediately", "immediate hold", "immediate mid-render submit", "immediate strike poke", and one panel tooltip. |

No present-tense claim that the program uses immediates survives.

---

## 5. FLAGS

**FLAG-1 — BRANCH. This campaign landed on `master`, not `claude/regain-1`.**
The handoff's §2 held the branch, and its reasoning is sound: the only witness
that this works is a boot, and CC does not boot. Jean's covering message when
sending the handoff overrode it — *"Remember that we're working from the master
branch, so commit the changes to it by the end of the task."* CLAUDE.md's boot
preflight says "Work on master unless the handoff says otherwise. Handoff
outranks harness defaults"; a live instruction from the gate-holder outranks the
handoff. Five commits sit on master and are pushed. **The consequence Jean
should weigh: there is no branch to discard.** If the boot goes badly the revert
is `git revert 7a6a1a5..09a9f09` or a reset to `ce4717b`, not a deleted branch.
Everything else in this report is unaffected.

**FLAG-2 — the OPEN docket's TypeError died with its branch, not by the one-line
cure.** The docket priced the fix at one line, and §4.1 said to fix it *before*
removing the directive so the gate would not break in the commit that should
prove it passing. The transform's retirement removes the `DIRECTIVE not in src`
branch entirely, so the defective line has no home to be fixed in and the hazard
never existed: 4.1 and 4.2 landed in one commit and the gate was run after both,
passing. The docket item is closed by deletion of the code, which is a closure.

**FLAG-3 — `upload_patch_params_batch` returns `uint32_t`, not `void`.** §2.3
specified `void` plus a loud `stderr` line and a clamp. A `void` clamp would have
been decorative: `generate_patch_batch` would still have looped to the caller's
original `count` and handed `i * 256` to a bind for a window the buffer does not
contain — a Dawn validation error, not a silent one, but a real one. The door
returns what it wrote and the loop walks that, so the clamp is one fact in one
place. The `stderr` line and the clamp are exactly as specified. (Unreachable in
practice: `generate_selected_patches` sizes its own `batchParams` array at
`Dim::MAX_ACTIVE_PATCHES`, so `count` is already bounded by construction.)

**FLAG-4 — the wgsl_gate tint arm is KEPT.** §4.1 named `transform()`, `SHIM_TAG`,
`IMMEDIATE_BASE_BINDING`, the ARM 4 watcher and the blind-spot paragraph for
retirement, and did not name ARM 3 (tint). Its original justification is now
moot — it existed to close the shim's hole — but Tint is the compiler family
every supported browser actually runs, so a Tint pass sits one step closer to the
boot than a naga pass. It is kept, still DORMANT, with that justification
restated in its banner. Say the word and it goes.

**FLAG-5 — `strataLayoutFor`'s `immediateSize` parameter was retired, and §4 did
not name it.** §2.5 and §3 named the *arguments*; nothing named the parameter.
Left standing it would have been a parameter no caller can use, setting a
descriptor field nothing reads — dead machinery in the tree, against CLAUDE.md's
"the tree holds living matter only", and a live-machinery hit under §7's
`immediate` grep. Removed as the last piece of the lane's pipeline-layout half.
Verified transparent to both instruments: `binding_ledger.py`'s call-site regex
has treated the fifth argument as optional since B6, and M-2 still passes. One
hunk to revert if Jean disagrees.

**FLAG-6 — three pass-head notes said something that stopped being true.** Not in
the handoff, found by §7's grep. `render_passes.hpp` (main pass) and
`gallery.hpp` (snapshot pass) both said "FRAME binds with no offset argument —
the dynamic-offset machinery left the program", which P3 falsified, and both
added "carries no immediate", which is now vacuous. `console.hpp` carried an
orphaned comment for the deleted `wgslImmediate_`. All three corrected to name
which layouts do carry the two dynamic-offset seats.

**FLAG-7 — one count in another OPEN item moved, and was corrected in place.**
GUARD DEBT says "four `__EMSCRIPTEN__` guards survive SUNSET_1". `floorHolds()`
was one of them; three survive. The item is NOT closed and its line does not die
— only its number changed, with a parenthetical naming this campaign. §6's "one
line is ADDED, and only one" is about OPEN's line inventory, which is unchanged
by this; leaving a knowably-false count in the law book seemed worse than the
edit. Revert it if the reading is wrong.

**FLAG-8 — `python tools\binding_gen.py` with no flag is not a command.** §2.6
and §4.5 both spell the gate that way; the tool requires one of
`--bootstrap | --check | --write | --write-wgsl | --plan | --bootstrap-recut` and
exits 2 with a usage message otherwise. Read as `--check` throughout, which is
what "must report no drift against the ledgers" describes.

**FLAG-9 — `audit/BINDING_LEDGER.md` is regenerated by its own tool.**
`binding_gen.py --write` writes five files and not that one; witness S-1 reads
its cardinality captions, so `python3 tools/binding_ledger.py` was run at each
phase. Not a defect, just not stated in the handoff's recipe.

**FLAG-10 — two schema captions are hand-maintained and had to move.** The
`binding_registry.hpp` banner's "N declarations over M slots" is the authority
witness 0b-1 reads, and it does not derive: 86/71 → 87/72 at P2 → 88/73 at P3.
The generator STOPped until each was corrected, which is the authority working.

**FLAG-11 — NO COMPILE WITNESS WAS TAKEN.** §2 says CC does not build, and glaw1
is Jean's gate, so nothing in this campaign has been through a C++ compiler. The
C++ edits are substantial: a deleted member function and its call, a deleted
member variable, a changed function signature at four sites, a new member array
and door. The WGSL half has a real witness (naga, over the whole module) and the
binding half has thirteen (`binding_gen --check`, `binding_ledger`), but the C++
half has none until step 1 of §6.

**FLAG-12 — naga was installed into this container.** The standing OPEN item
("CC's container lacks naga … cargo install naga-cli, priced at the sitting")
prices exactly this, so it was paid rather than skipped: `cargo install naga-cli`
→ naga 30.0.0. That OPEN item stays open — it is about the container baseline,
which a fresh session recreates.

**FLAG-13 — `docs/HANDOFFS/` did not exist and this campaign creates it.**
CLAUDE.md reads its absence as health. Its presence now says what it should: this
work order is open until Jean boots and merges. It should die with the merge.

---

## 6. What Jean must do

```
1. cmake --preset the-board-web && cmake --build --preset the-board-web
2. Boot in Chrome. The world must be identical — same seed, same frame, no
   validation errors. This is the regression gate; everything else is new reach.
3. Boot in Firefox. Boot on the iPhone. Open DETAILS on each and paste the log.
4. Merge or discard. The branch dies either way (ATTIC LAW).
```

Step 4 reads differently under FLAG-1: there is no branch. The work is on
`master` and pushed. "Discard" means a revert of the five commits, not a deleted
branch.

Two boot lines changed shape and are worth knowing before reading a log:
`[Device] requesting CORE DEFAULTS; exceptions carried: none (REGAIN_1 …)`, and
`[Device] granted vs floor:` no longer ends with a `maxImmediateSize` column. The
`[Device] wgsl language features:` line is gone entirely.
