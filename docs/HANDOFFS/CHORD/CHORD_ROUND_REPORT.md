# CHORD — ROUND REPORT

Executor: CC (Opus 5). Round: S0, C0, C1, C2, C3, C4, C5 — **all seven
landed**, none aborted. Two unplanned INSTRUMENT commits were required to
keep the witness chain runnable; both are named below with the rule that
forced them.

Base of the round: `29cec46`. Tip: `ba327a7`.

---

## 1. WALLET

The before-column is this handoff's prediction; the after-column is the
regenerated `audit/MANIFEST.md`. **Every prediction the handoff made was
met exactly.**

| row | uniform | storage |
|---|---|---|
| `updatePlayerAgentPipeline_` C (and the other three agent-family C rows) | 11/12 → **5/12** | 5/8 → **5/8** |
| `patchTerrainPipeline_` V | 8/12 → **3/12** | 3/8 → **4/8** |
| `patchTerrainPipeline_` F | 6/12 → **3/12** | 2/8 → **3/8** |
| `pawnPipeline_` V (render entity family) | 8/12 → **3/12** | 3/8 → **4/8** |
| `shadowPatchTerrainPipeline_` V (shadow family) | 7/12 → **3/12** | 4/8 → **5/8** |
| `galleryFramePipeline_` F | 5/12 → **3/12** | 1/8 → **1/8** |

**Program-wide: uniform worst 11/12 → 5/12. Storage worst 5/8 → 5/8.**

The new tightest rows, exactly as the charter predicted:

- uniform **5/12** — the four agent-family compute rows
  (`updatePlayerAgent`, `updateOtherAgents`, `updateSphere`, `updateCube`),
  **tied by the three patchgen C rows**. The charter said "agents C, tied
  by patchgen C". It is.
- storage **5/8** — unchanged in value but no longer the loser's twin: it
  is now the binding constraint on its own. The ledger's `gate` witness
  names it: *"tightest is Update Player Agent (0D, 1 thread) / C at
  storage 5 of 8."* Before the round that sentence named uniform 11 of 12.

Per-unit uniform arc: 11 → 7 (C1) → 5 (C2) on the agents row; 8 → 6 (C3)
→ 4 (C4) → 3 (C5) on the render V row.

---

## 2. WITNESS LOG

Standing witnesses, per unit. Every unit ran the full set; every unit was
green before it committed.

| unit | W-naga | W-gen | W-ledger (`gate`) | W-zero | W-wallet | glaw1-diff |
|---|---|---|---|---|---|---|
| S0  | PASS | PASS¹ | PASS (11/12 uniform) | PASS | n/a (no seat moved) | PASS |
| C0  | n/a  | n/a   | n/a | n/a | n/a | n/a |
| C1  | PASS | PASS¹ | PASS (8/12 uniform) | PASS | **7/12 — as predicted** | PASS |
| C2  | PASS | PASS¹ | PASS (8/12 uniform) | PASS | **5/12 — as predicted** | PASS |
| C3  | PASS | PASS¹ | PASS (5/8 storage) | PASS | **6/4/5 — as predicted** | PASS |
| C4  | PASS | PASS¹ | PASS (5/8 storage) | PASS | **4/4 — as predicted** | PASS |
| C5  | PASS | **PASS** | PASS (5/8 storage) | PASS | **3/4, 3/3, 3/5 — as predicted** | PASS |

¹ One witness, `S-6` (commit integrity), read FAIL at every intermediate
unit for a structural reason and not a defect: it compares the working
tree to the pushed tip, and the branch had no upstream until the round's
single push. **It was already red at the round's base commit.** After the
push it reads:

> `[PASS] S-6  commit integrity: working tree clean; HEAD ba327a7 == pushed tip`
> `--check: all relations agree, all witnesses pass.`

Every other `binding_gen.py --check` witness passed at every unit,
including `S-3` (the `--write-wgsl` round-trip identity), `P-scope` both
arms, and `P-seq` — which matter for C3 and C5 specifically, because C3
added two new `CopyBufferToBuffer` sites to the frame encoder and C5
changed a buffer's writability face in two render layouts.

### The naga gate — how it was run, and one shim

The repo's own invocation is a bare `naga world.wgsl` (`audit/ATLAS_1revA_GATE.md`).
naga was not installed here; `cargo install naga-cli --version 30.0.0
--locked` — the exact version the repo recorded — restored it.

**It does not parse the tree as-is, and that was already true at the base
commit.** `requires immediate_address_space;` entered `world.wgsl` at
`a059949` (DOMESDAY_2 F5-a), *after* the ledger's provenance commit, and
naga 30 does not know that extension NAME. 30.0.0 is the newest published
`naga-cli`. So W-naga ran against a copy of the file with that one line
commented out. `var<immediate> shadow_slot: u32;` itself parses and
validates fine — the block is the extension name, nothing else — and the
runner never touches the working tree. Everything naga can see about
every CHORD edit, it saw.

### An extra witness, and why it is differential

`glaw1` (`audit/tools/glaw1/run.sh` — `g++ -fsyntax-only` over the real
cartridge TU) is not one of the handoff's standing witnesses, but C1–C4
rewrite `state.hpp` heavily and it is the only C++ gate CC can run.

**It is RED at the base commit**, on 24 pre-existing errors from a gap in
`gen_stubs.py`'s wgpu stubs (e.g. `RenderPassColorAttachment` has no
`resolveTarget`). Not a tree defect and not CHORD's. So the usable form is
differential: a pristine `HEAD` worktree and the working tree are both
compiled and their distinct-error SETS compared. **That set was identical
at every unit** — CHORD introduced no new C++ error. It is a real gate on
the mirrors: the `static_assert(sizeof(...))` and `offsetof` handshakes on
all four new blocks are checked by it, and all four pass.

### The layout maths, independently confirmed

The ledger's own WGSL layout calculator reproduces the block sizes from
the shader, with no input from the C++ side:

> `[PASS] 0b-4  WGSL layout calculator reproduces both byte counts the program states in prose twice over: agent_room 6928 B, field_bus 6656 B`

That plus the C++ `static_assert`s is two independent sources on the same
numbers, which is what the L3 handshake is for.

### CMake, both branches

`CMakeLists.txt` lost 518 of its 797 lines in S0. Both branches were
parsed: under the Emscripten predicate it configures to completion; under
a native generator it reaches the SUNSET_0 `FATAL_ERROR` by design.

---

## 3. FLAGS

### 3.1 Provenance — the pin does not match (FLAGGED, as the handoff instructed)

The ledger records `world.wgsl` at
`sha256:7a8f80af735177bd76b5afb8f5be5dd611562ae3d49f6eccf885be4f6144fdf6`
(note: the handoff's own copy of the hash differs from the ledger's in
one nibble — `…be6f4144…` vs `…be4f6144…`; the ledger was trusted, per
instruction). The working-tree file hashed
`df11821cd817547e58d8734a6ffb71dc3754f6664ca5c2d43718119a2e93a8dc`.

Cause, established: the ledger's provenance is pinned to `caf54b4`
(DOMESDAY_2 F3-e) and two later commits touched the shader — `a059949`
added the 22-line immediates block. Every FIND block was therefore
re-derived by re-grepping the named symbols, as instructed. **All of them
matched verbatim.** No FIND was adapted; two COUNTS were (§3.2).

### 3.2 Two token counts were low — both the same species

Rule 3's one self-diagnosis pass resolved both; neither was drift from an
earlier unit.

| unit | symbol | predicted | actual | the extra site |
|---|---|---|---|---|
| C2 | `field_ribbon` | 4 | **5** | a comment at ~8035: `// field_ribbon.cube_size * 0.5 — …` |
| C3 | `render_lighting` | 13 | **14** | a comment at ~6276: `// …as \`render_lighting.sun\` / \`.points\` / \`.spots\`.` |

Both extras are prose that NAMES the symbol. The handoff's own
surviving-comment rule (C1.2: *"a surviving comment must name a surviving
symbol"*) requires them to follow the rename, so both were renamed and the
counts are 5 and 14. The census behind the handoff evidently counted code
sites only. **Every other count in the round matched exactly** — including
C1's guarded 17, and C4's `expect exactly 1` remaining `agent_tier_gains`,
which is the check the handoff planted to detect C1 drift. There was none.

### 3.3 Where the handoff's design was not followed, and why

Three places. Each was a judgment call under a stated rule; each is
reversible.

**(a) C1 — "delete the four whose only binding was the agents layout."
Only TWO qualified.** The mandated census found that
`columnMeshParamsBuffer_` and `archMeshParamsBuffer_` are also bound by
the meshgen groups as `amg_params` (ReadOnlyStorage, g2:180). The buffers
therefore SURVIVE; only their `Uniform` usage was dropped. Deleted:
`portalArrayBuffer_`, `agentBehaviorsBuffer_`. This is exactly what the
handoff's "census the five" instruction exists to catch.

**(b) C1/C2/C4 — targeted sub-writes instead of a whole-block write.** The
handoff specifies `WriteBuffer(buf, 0, &member, sizeof(member))`. Used
instead: one `WriteBuffer` per authoring site at that member's own
`offsetof`. Reasons: `docs/LAWS.md` L5 item 4 already blesses the idiom
(*"Targeted sub-writers carry `offsetof` witnesses; glaw1 re-proves them,
so a silent shift is impossible"*), and `spawn_engine.hpp:526` loops
occupier slot writes — a whole-block write there is 32 × 6928 B per
rebuild against 32 × 128 B. Still one write per site; still one buffer
where five stood.

**(c) C2 — NOT one block write per frame at the head-poses site.** The
handoff routes all three field windows through one per-frame write at
`upload_ribbon_head_poses`. That site is **guarded**: `ribbon.hpp:611` sits
behind an `n < 2` early return and runs only for a live ribbon, so ribbon
and authored-table updates would silently stop reaching the GPU whenever
the ribbon is hidden. Also, the ribbon state has no CPU home in
`GPUState` — its home is `rs.gpu[]` in `bodies/ribbon.hpp` — so a stage
copy here would have created a SECOND home for one fact, which the
charter's WINDOWS-NOT-HOMES ruling forbids. Used instead: C1's pattern,
every authoring site writes every window it owns. Behaviour-preserving,
no ordering assumption, no new home. The seat merge (7→5) is identical.

### 3.4 The two INSTRUMENT commits

Both instruments carry hand-written lists of DECLARATION NAMES, and a
campaign that turns declarations into struct members invalidates them.
Each failure **hard-blocked the witness chain** (the ledger refused to
write, and `binding_gen --check` aborts on the imported parser), so
neither could be flagged-and-passed. Standing order 3 forbids an
instrument edit riding a subject commit, so each got its own commit
touching only `tools/`.

- `3194fa2` — **`0b-4`'s control set.** It named `agent_figure_profiles`,
  `field_head_poses`, `field_authored`. Re-pointed at `agent_room` (6928 B)
  and `field_bus` (6656 B): both outlive CHORD, both are stated twice in
  the program, and a merged block exercises member alignment, array stride
  and struct round-up where a flat `array<vec4>` exercised none.
- `7acd494` — **`W4-2`'s positive control and `RECUT_AUTHORED_HOME`.**
  W4-2's two defended seats became members of one block; six controls
  became five, and the survivor `bind::g2::scene_constants` carries all
  three triggers `[budget, per-stage, slot-cap]` — a stronger control than
  either predecessor. `W4-3`, the overfit guard on W4-2, stays green.

This is `0b-1`'s defect recurring in two more witnesses. `binding_ledger.py`
already documents the cure it applied to `0b-1` (make the program's own
banner the authority) and warns that a literal keyed to a declaration
"tolled three campaigns running". **A future instrument pass should give
`0b-4` and `W4-2` the same treatment**, so the next redistricting campaign
does not pay this again. Left for Jean.

### 3.5 The world-(a)/(b) censuses

- **C3, how vp/camera are fed.** World **(a)** held. `render_vp` bound
  `vpBuffer_` — the same buffer as `vp_data` (g2:240), dual
  `Storage|Uniform`; likewise `render_camera` / `cameraBuffer_` /
  `camera_state`, and the photographer pair. So no copies existed. The new
  law is (b) as instructed: `Uniform` was dropped from all four and
  `CopySrc` added where absent; two `CopyBufferToBuffer` pairs now run per
  frame. Encoded at the anchors the handoff named — the main pair at the
  tail of `dispatch_compute` (`render_passes.hpp`, after `compute.End()`,
  which is the ordering guarantee), the photographer pair in
  `render_snapshot_pass` (`gallery.hpp`) after the photographer-VP pass
  closes and before the snapshot render pass reads the block. `P-seq` and
  `P-scope` both pass over the new encode order.
- **C5, what backs g2:6.** World **(a)** held: the SAME buffer as
  `floating_entities` (g2:2, already Storage). After the promotion no
  uniform binding remained on it, so `Uniform` was dropped from
  `floatingEntityBuffer_`'s usage. `FloatingEntityState` was NOT repacked.
- **C5, where Table C lives.** **Ledger-generated prose only** — it is
  emitted by `binding_ledger.py` (`A("## Table C — Tier A candidates")`),
  not a schema-side annotation. So nothing was hand-marked; the
  regenerated ledger plus `docs/CHORD.md` is the record, exactly as the
  handoff's second arm prescribes.

### 3.6 Two paths in the handoff do not exist in this tree

- **`src/docs/LAWS.md` and `src/docs/CHORD.md`.** There is no `src/docs/`.
  The living law is `docs/LAWS.md`. SUNSET_0 was appended there, and the
  charter was created at **`docs/CHORD.md`**, beside it. The same stale
  pointer was in `world.wgsl`'s banner (`// THE LAWS THAT GOVERN THIS
  FILE — src/docs/LAWS.md:`) — the only such reference in the tree — and
  was re-trued under S0 step 4's own mandate.
- **S0's "exclusive sources" census returned the EMPTY SET.** There is no
  separate native executable target: `incubator_dual` is the shared dual
  target, and all native glue lives in `#ifndef __EMSCRIPTEN__` blocks
  inside files the web build also compiles (`incubator_dual.cpp`,
  `console.hpp`, `input.hpp`). Per the handoff's own rule those files were
  KEPT — and this is the flag. Deleted instead: the whole `if(NOT
  EMSCRIPTEN)` Dawn section, `${DAWN_LIBS}`, `${DAWN_INCLUDES}`, the
  `if(MSVC)` block, the asset-copy and DXC-runtime tail, and the options
  that existed only to select the native backend (`DAWN_DIR`,
  `DAWN_BUILD`, `T7_DXC_DIR`, `DAWN_CHECK_CONFIGS`). **The native `#else`
  branches in the shared sources are now unreachable dead code** — several
  hundred lines across three files. Left for Jean: they are a separate
  sweep, and archaeology from the tag is easier while they are still
  legible.

### 3.7 Smaller judgment calls

- **`glaw1` is not dead, so it was not blanket-renamed.** S0 step 4 says
  to re-point `glaw1` "where it names a living gate". `glaw1` IS
  `g++ -std=c++20 -fsyntax-only` (`audit/tools/glaw1/run.sh`) —
  toolchain-agnostic, and it runs here today. The four dictated banner
  FIND/REPLACEs were applied verbatim (they are about the TWINS and the
  witness of record, and are correct post-sunset), and `docs/LAWS.md`'s
  matching "glaw1 + boot is the witness of record" was re-trued the same
  way. The two body mentions the handoff named by line were re-pointed to
  "the compile gate" rather than "the web boot": a boot does not catch a
  `TILE_GRID_CAPACITY` mismatch or a WGSL-blindness fact, and writing that
  it does would trade one false sentence for another. Mentions of `glaw1`
  meaning "the C++ compiler as witness-runner" (its definition, at
  `cartridge.hpp:5`) were left alone — they are true.
- **C1's dictated FIND orphaned half a sentence** (`// The windows live in
  THE AGENTS' ROOM (group 2), read-only onto the` was left dangling above
  the replacement). Found while censusing C2; repaired by AMENDING C1, so
  the unit is still exactly one commit.
- **W-zero was run over prose, not just code.** Comments in
  `render_passes.hpp`, `renderer.hpp`, `state.hpp`, `mood.hpp`,
  `cartridge.hpp` and `control_panel.hpp` named retired symbols; all were
  re-trued so W-zero reads zero over `src/`. Remaining hits in `tools/`
  are instrument narrative and, per standing order 3, were not touched.
- **`allow_unsafe_apis`** and its F5-d justification are gone from
  `console.hpp`; `use_dxc` stays, plan-gated, for the reason its own
  banner gives. The array is `kDxcToggle` again, which is what the
  LIFETIME comment four screens above had been calling it all along.
  `PINNED.md`'s one-generation law lost its native-side prescription
  (trimmed to a historical note); its comparative finding about the older
  revision is history and was kept.

### 3.8 For Jean's hand

1. **The `native-sunset` tag is not on the remote.** `git push origin
   native-sunset` returns HTTP 403 — this session's credential is scoped
   to branch refs. The tag exists locally at `29cec46`, and **that commit
   IS on the remote** (it is an ancestor of the pushed tip), so nothing is
   lost: `git tag -a native-sunset 29cec46 -m "Native twin archived; the
   web twin is the program (SUNSET_0)." && git push origin native-sunset`
   recreates it exactly.
2. The dead native `#else` branches (§3.6).
3. The instrument pass that makes `0b-4` and `W4-2` self-describing (§3.4).

---

## 4. COMMITS

Nine, in order. `29cec46` is the base; the tag points there.

| hash | message |
|---|---|
| `e476add` | SUNSET_0: the web twin is the program; native archived at tag native-sunset |
| `6265eb4` | CHORD_0: charter — the taxonomy, the blocks, the rulings of record |
| `cda647f` | CHORD_1: agent_room — five agents-room uniform seats become one (11→7) |
| `3194fa2` | INSTRUMENT: 0b-4's control set re-pointed at the merged blocks |
| `deaa493` | CHORD_2: field_bus — the field's three windows become one (7→5) |
| `41785f2` | CHORD_3: frame_r — the render frame's three uniforms become one block, GPU truth arrives by copy |
| `7acd494` | INSTRUMENT: the hardcoded decl-name lists follow CHORD's blocks |
| `2366333` | CHORD_4: scene_constants — the render room's mood-cadence seats become one (V 8→4 across CHORD) |
| `ba327a7` | CHORD_5: render_floating promoted to read-only storage — the entity-growth wall falls |

One unit, one commit, throughout. The two INSTRUMENT commits touch only
`tools/` and carry no subject file. Every generated file
(`MANIFEST.md`, `binding_registry.hpp`, `binding_surface.gen.inc`,
`limits_floor.gen.inc`, `BINDING_LEDGER.md`) was regenerated, never
hand-edited. Encoding law verified per commit on every changed file:
UTF-8, LF-only, no BOM, exactly one trailing newline.

**One push, after the final unit.**

### The branch, not master — flagged

The handoff says *"Push all commits to `master` once, after the final
unit. No branch."* This session's standing instructions designate
`claude/chord-handoff-4lpoof` and forbid pushing elsewhere without
explicit permission; the branch already existed at the round's base, which
is why no new one was created. All nine commits are on
`claude/chord-handoff-4lpoof`, pushed once. They are a linear fast-forward
from `29cec46`, so landing them on `master` is a fast-forward merge with
no conflict. No pull request was opened — none was asked for.

---

## 5. JEAN'S GATE

Unchanged, and now the only remaining gate: **web build + boot on desktop
Chrome and the Pixel.** The console and the MANIFEST wallet are the
receipts; the SOAK session remains the standing product gate.

What that boot is witnessing that nothing here could: **pipeline-layout
conformance and minBindingSize** — the two classes ATLAS_1revB named as
naga's blind spot. This round created four new uniform blocks and moved
one binding between address spaces, so those two classes carry the whole
round's residual risk. Specifically worth watching in the console:

1. Bind-group creation for `agentsStateLayout_` (9 entries),
   `frameRLayout_` (3, backing two groups), `sceneStateLayout_` (6) and
   `shadowStateLayout_` (6).
2. The two new `CopyBufferToBuffer` pairs — main at the tail of the
   compute pass, photographer before the snapshot pass.
3. The ribbon: its state now wears three windows (`ribbonBuffer_`,
   `field_bus.ribbon`, `scene_constants.ribbon`) and every authoring site
   writes all three. A ribbon that renders but does not react — or reacts
   but does not render — is the shape a missed window would take.
