# WINNOW_CENSUS.md — WINNOW-1, the census (read-only)

**Round:** WINNOW-1 · **Mode:** reconnaissance, read-only · **Branch:** master
**Deliverable:** this file, and nothing else. Scaffold — it dies at campaign CLOSE.
**Authority:** standard triangle. Nothing is cut this round; the knife is WINNOW-2,
after Jean gates the kill list.

This census is a **map, not a verdict**. Every DEAD carries evidence in its own row;
a DEAD that could not carry evidence was demoted to UNSURE, as the register law requires.

---

## C0 — Procedure, and what resisted it

Recorded first because two of these change how the rest of the file should be read.

### C0.1 — The clone arrived shallow. Dates would have been fiction.

`git rev-parse --is-shallow-repository` returned **true** on entry: 51 commits, graft
boundary at `0ced0c9` (2026-08-16) and `b9629c2` (2026-08-10). C2 requires
`git log -1 --format=%cs -- <path>` per file. On a shallow clone that command does not
fail — **it silently returns the graft-boundary commit's date** for every file whose last
real edit predates the boundary. The census would have reported ~2026-08-16 for hundreds
of files and been internally consistent, plausible, and wrong.

Fixed before any row was written:

```
git fetch --deepen=200 origin     # 51 → 533 commits
git fetch --unshallow origin      # 533 → 1687 commits, root f584dc8 (2026-04-03)
```

Every date below is measured against complete history. This is the same failure
`606924f` ("every gate announces a shallow clone before it speaks") and `8d186a2`
already recorded; it recurred here on a fresh container, which suggests the announcement
is not yet load-bearing for a *cloned* session.

### C0.2 — `master` looked 107 files behind. It was not.

On entry `master` and `origin/master` both pointed at `b491115` "LOOM_0: the mirror
census" (2026-08-13), while `HEAD` sat at `606924f` (2026-08-17) with **no merge base**
and 107 differing files. That is exactly the illusion `8d186a2` adjudicated
("there was never an orphan line; the clone was shallow and F15 read the graft boundary
as history"). The deepen resolved it: `origin/master` force-updated `b491115...606924f`.
`master` **is** `606924f`; the stale ref was an artifact of the shallow fetch. No
divergence existed, and `git diff master HEAD` showed zero deletions — consistent with a
stale ref, not a fork. The census is taken on `606924f`.

### C0.3 — The LIVING symbol test is permissive, and that is the round's real finding

The spec's LIVING clause admits a doc if *"symbols named in its title/first section exist
in the tree."* Applied literally, almost every technical document passes: a 2026-07 recon
of a since-rewritten subsystem still names `CopyBufferToBuffer` or `SetBindGroup`, and
those still exist. The test as written would have returned ~LIVING for nearly all 283
rows and handed WINNOW-2 a blunt knife.

The discriminator that actually separates them is **whether the document's own subject
token survives in first-party code**, not whether the mechanisms it mentions in passing do:

| doc | subject token in code | incidental symbol in code |
|---|---|---|
| `BEQUEST_REPORT.md` | `BEQ_CLOSE` = **0** | `GPUFrameR` = 5 |
| `GATEHOUSE_REPORT.md` | `GATEHOUSE_G6` = **0** | `T7_WEB_DIAG` = 2 |
| `ORGAN_1_ROUND_REPORT.md` | `f48ad7a` = **0** | `apply_mood` = 11 |

**Stated assumption, for Jean to override:** LIVING is awarded on *reference from
first-party code/law/build* (the strong clause) or an *open finding*; the symbol clause is
recorded as evidence but is not sufficient on its own to overturn an archival DEAD. Both
signals are printed in every row, so the ruling can be reversed per-file without re-running
the census.

### C0.4 — Register

- Commands that errored: **none**. All `git log`/`git grep`/`git ls-files` invocations
  returned 0 across 283 files.
- Files unreadable: **none**. `note.md` is UTF-16LE + CRLF and was decoded with
  null-stripping rather than skipped.
- Halts: **none**.

---

## C1 — Directory census

`git ls-files` reports **508** tracked files: 493 under eight top-level directories,
15 loose at root.

### Top-level directories

| directory | tracked | class | evidence (one line) |
|---|---|---|---|
| `assets/` | 58 | **FIRST-PARTY** | 56 `.jpeg` + 1 `.jpg` + 1 `.mp3`; source of truth for the exhibition — `.gitignore` records `dist/paintings/` as *"re-encoded copies of assets/, which IS tracked"*. |
| `audit/` | 165 | **MIXED** | Hand-written reports **plus 5 machine-generated ledgers** (`MANIFEST.md`, `BINDING_LEDGER.md`, `MIRROR_LEDGER.md`, `COMMAND_LEDGER.md`, `past reports/PRUNING_1_CENSUS.md`) and first-party `audit/tools/`. |
| `CLAUDE CODE/` | 25 | **FIRST-PARTY** | Campaign logs, audit reports and 2 `.patch` files; authored here, referenced by `tools/` as `CLAUDE.md`. Directory name contains a space. |
| `docs/` | 141 | **MIXED** | First-party prose and law **plus 2 vendored spec PDFs** (`WebGPU.pdf`, `WebGPU Shading Language.pdf`) that are upstream W3C/Dawn artifacts, not ours. |
| `src/` | 65 | **MIXED** | First-party C++/WGSL **plus vendored `src/external/stb_image.{h,cpp}`** (single-file public-domain library). |
| `third_party/` | 19 | **VENDORED** | `emdawnwebgpu_pkg/` is a byte-pinned upstream payload: `PINNED.md` records its zip sha256, `.gitattributes` sets `-text` so *"the working tree stays byte-for-byte upstream"*, and it ships two `LICENSE` files. |
| `tools/` | 18 | **MIXED** | First-party Python (census/gate/deploy chain) **plus vendored `tools/gates/console_gate/stubs/`** — GLFW/emscripten headers whose sha256s are recorded in `PROVENANCE.md` and pinned `-text` by `.gitattributes` (GATE_1). |
| `web/` | 2 | **FIRST-PARTY** | `index.html` + `organ_panel.js`. `.gitignore` states *"web/index.html is SOURCE and stays tracked"*; the three build outputs beside it are ignored. |

### Vendored / generated sub-trees inside the MIXED directories

Named explicitly because C2 and C3 must exclude them, and a directory-level class alone
would not say where the boundary falls.

| sub-tree | files | class | evidence |
|---|---|---|---|
| `third_party/emdawnwebgpu/` | 19 | **VENDORED** | sha256-pinned upstream payload; `.gitattributes` `-text`. |
| `src/external/` | 2 | **VENDORED** | `stb_image` upstream single-file library. |
| `tools/gates/console_gate/stubs/` | 7 | **VENDORED** | per-file sha256 in `PROVENANCE.md`; `.gitattributes` `-text`. |
| `docs/*.pdf` | 2 | **VENDORED** | upstream WebGPU / WGSL specifications. |
| `audit/MANIFEST.md` | 1 | **GENERATED-BUT-TRACKED** | `tools/binding_gen.py:856`; header says *"GENERATED … do not hand-edit"*. |
| `audit/BINDING_LEDGER.md` | 1 | **GENERATED-BUT-TRACKED** | `tools/binding_ledger.py:47` `DEFAULT_OUT`. |
| `audit/MIRROR_LEDGER.md` | 1 | **GENERATED-BUT-TRACKED** | `tools/mirror_census.py:57` `DEFAULT_OUT`. |
| `audit/COMMAND_LEDGER.md` | 1 | **GENERATED-BUT-TRACKED** | `tools/command_census.py:64` `DEFAULT_OUT`. |
| `audit/past reports/PRUNING_1_CENSUS.md` | 1 | **GENERATED-BUT-TRACKED** | `tools/pruning_census.py:47` `OUT` — **at a stale path, see F-1**. |

**Grep exclusion set used by C2 and C3** (no *whole* top-level tree is
GENERATED-BUT-TRACKED, so only the vendored sub-trees are excluded):

```
git grep -n -F "<token>" -- ':!third_party' ':!src/external' ':!tools/gates/console_gate/stubs'
```

### Root-level loose files

| file | class | evidence |
|---|---|---|
| `.gitattributes` | **FIRST-PARTY** | Carries the WGSL LF pin and both vendored `-text` pins; part of the deploy chain. |
| `.gitignore` | **FIRST-PARTY** | Declares `dist/` and the three web build outputs non-authoritative. |
| `BEQUEST_REPORT.md` | **FIRST-PARTY** | Current-campaign report, 2026-08-17. |
| `CMakeLists.txt` | **FIRST-PARTY** | The build authority. Also lands in C2 scope via the `*.txt` glob — see W-2. |
| `CMakePresets.json` | **FIRST-PARTY** | "The CMake presets" — a named law authority. |
| `GATEHOUSE_REPORT.md` | **FIRST-PARTY** | Current-campaign report, 2026-08-16. |
| `PROBATE_E_REPORT.md` | **FIRST-PARTY** | Current-campaign report, 2026-08-16. |
| `PROBATE_REPORT.md` | **FIRST-PARTY** | Current-campaign report, 2026-08-16. |
| `PROBATE_SEAL_REPORT.md` | **FIRST-PARTY** | Current-campaign report, 2026-08-16. |
| `README.md` | **FIRST-PARTY** | Repo front door; last touched 2026-04-04, named by `tools/pruning_census.py:2754`. |
| `gc_close_census.md` | **FIRST-PARTY** | Campaign census, 2026-07-26. |
| `note.md` | **FIRST-PARTY** | **UTF-16LE + CRLF, 6594 B** — the only such file in the tree; see F-3. |
| `device` | **UNSURE** | **0 bytes, no extension, no content.** Reads as a shell-redirect accident, but nothing proves it. Not decided. |
| `rmdir` | **UNSURE** | **0 bytes, no extension, no content.** Same shape as `device`. Not decided. |
| `search_log.txt` | **UNSURE** | **0 bytes**, 0 references repo-wide. Nothing to test; see F-2. |

---

## C2 — Document census

**Scope, mechanical:** `git ls-files '*.md' '*.txt' '*.rst' '*.adoc'` → **273** files
(198 `.md`, 75 `.txt`; zero `.rst`, zero `.adoc`).
**Added by suspicion:** **10** further tracked files that are documentary but fall outside
the four globs (6 `.patch`, 2 spec `.pdf`, 2 vendored `LICENSE`). Per the round's rule they
are listed, classed **UNSURE**, and **not decided**.
**Census total: 283 rows.**

Classes are tested in order, first hit rules: **LAW → LIVING → DEAD → UNSURE.**

| class | count |
|---|---|
| **LAW** | 19 |
| **LIVING** | 46 |
| **DEAD** | 155 |
| **UNSURE** | 63 |
| **total** | **283** |

Reference recipe used for every row, token = basename without extension:

```
git grep -l -F "<token>" -- ':!third_party' ':!src/external' ':!tools/gates/console_gate/stubs'
```

Hits were then split into **first-party code/law/build** (`src/`, `tools/`, `audit/tools/`,
`web/`, `CMakeLists.txt`, `CMakePresets.json`, `docs/LAWS.md`, `docs/HANDOFFS/PROCESS_LAWS.md`,
`audit/MANIFEST.md`, `audit/BINDING_LEDGER.md`) versus **prose-only** mentions. Only the
first bucket earns LIVING — a document cited solely by other archived reports is not alive,
and counting those would have marked most of the archive LIVING.

### Root-level

*10 rows — LAW 1, LIVING 7, UNSURE 2*

| path | last commit | class | evidence |
|---|---|---|---|
| `BEQUEST_REPORT.md` | 2026-08-17 | **LIVING** | current campaign window (2026-08-17); symbol `master` lives in 14 first-party file(s) |
| `CMakeLists.txt` | 2026-08-16 | **LAW** | the build authority; names docs/LAWS.md + PINNED.md (in scope via *.txt glob) |
| `GATEHOUSE_REPORT.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `master` lives in 14 first-party file(s) |
| `PROBATE_E_REPORT.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `master` lives in 14 first-party file(s) |
| `PROBATE_REPORT.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `master` lives in 14 first-party file(s) |
| `PROBATE_SEAL_REPORT.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `master` lives in 14 first-party file(s) |
| `README.md` | 2026-04-04 | **LIVING** | referenced from first-party code/law/build: `CMakeLists.txt` (+1 more) |
| `gc_close_census.md` | 2026-07-26 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `master` still lives in 14 file(s); landed: 4f12479 src/docs -> docs: the reorg's tail, including one  |
| `note.md` | 2026-08-05 | **LIVING** | referenced from first-party code/law/build: `audit/BINDING_LEDGER.md` (+37 more) |
| `search_log.txt` | 2026-08-13 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 051c37e DOMESDAY_1 B9: the parameter surface — ?seed= ?m |

### `docs/` (top)

*9 rows — LAW 3, LIVING 3, UNSURE 3*

| path | last commit | class | evidence |
|---|---|---|---|
| `docs/7t_program_theory_v3.md` | 2026-08-16 | **LAW** | named by docs/LAWS.md |
| `docs/CHORD.md` | 2026-08-16 | **LAW** | named by tools/binding_schema.py:253,268 as the CHORD_5 reversal authority |
| `docs/DAWN_REFERENCE.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `Debug` lives in 4 first-party file(s) |
| `docs/ESTATE_LOOM_WALLET.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `shadow_slot` lives in 9 first-party file(s) |
| `docs/LAWS.md` | 2026-08-17 | **LAW** | the authority itself — the numbered rule book |
| `docs/NEXT_SESSION_OPENER.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `Discard` still lives in 3 file(s); landed: 9489b8d CLOSE_0 A7: the next session opener |
| `docs/ORGAN.md` | 2026-08-16 | **LIVING** | referenced from first-party code/law/build: `CMakeLists.txt` (+11 more) |
| `docs/WebGPU Shading Language.pdf` | 2026-08-07 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `docs/WebGPU.pdf` | 2026-08-07 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |

### `docs/audit/`

*13 rows — LAW 2, LIVING 3, UNSURE 8*

| path | last commit | class | evidence |
|---|---|---|---|
| `docs/audit/PROBATE_CLOSE.md` | 2026-08-17 | **LIVING** | referenced from first-party code/law/build: `tools/wgsl_gate.py` |
| `docs/audit/SALON_1.md` | 2026-08-16 | **LAW** | named by docs/LAWS.md |
| `docs/audit/SALON_1_19B_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `load_authored_textures` still lives in 2 file(s); no landed commit found |
| `docs/audit/SALON_1_B4_REPORT.md` | 2026-08-07 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/bodies/gallery.hpp` |
| `docs/audit/SALON_1_E_B2_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section |
| `docs/audit/SALON_1_E_B_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `WALL_ART` still lives in 3 file(s); landed: 5490cb1 SALON_1 E-b: the fill tier — mechanism lands, an |
| `docs/audit/SALON_1_E_C_REPORT.md` | 2026-08-16 | **LAW** | named by docs/LAWS.md |
| `docs/audit/SALON_1_P4.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `INT32_MAX` still lives in 12 file(s); no landed commit found |
| `docs/audit/SALON_1_P5_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `ceiling_height` still lives in 4 file(s); no landed commit found |
| `docs/audit/SALON_1_P7_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `wall_height` still lives in 5 file(s); no landed commit found |
| `docs/audit/SALON_1_PROPORTION_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `PATCH_EXTENT` still lives in 12 file(s); no landed commit found |
| `docs/audit/SALON_1_SUPPLY_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `compute_photographer_vp` still lives in 5 file(s); no landed commit found |
| `docs/audit/TEX_C0_PRICE.md` | 2026-08-17 | **LIVING** | current campaign window (2026-08-17); symbol `authoredStagingTexture_` lives in 2 first-party file(s) |

### `docs/HANDOFFS/` — active

*18 rows — LIVING 10, UNSURE 8*

| path | last commit | class | evidence |
|---|---|---|---|
| `docs/HANDOFFS/CHORD/CHORD_HANDOFF.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `master` lives in 14 first-party file(s) |
| `docs/HANDOFFS/CHORD/CHORD_ROUND_REPORT.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `updatePlayerAgentPipeline_` lives in 3 first-party file(s) |
| `docs/HANDOFFS/DOMESDAY/DOMESDAY_1_HANDOFF.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `fc_visible` lives in 5 first-party file(s) |
| `docs/HANDOFFS/DOMESDAY/DOMESDAY_2_HANDOFF.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `immediate_size` lives in 3 first-party file(s) |
| `docs/HANDOFFS/ORGAN/ORGAN_1_ROUND_REPORT.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `master` lives in 14 first-party file(s) |
| `docs/HANDOFFS/PIVOT/PIVOT_0d_ROUND_REPORT.md` | 2026-08-12 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `master` still lives in 14 file(s); landed: dddb30d TOGGLE_0 U2: retire the control; verdict — posit |
| `docs/HANDOFFS/PROBATE/PROBATE_E_HANDOFF.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `update_player_agent` lives in 7 first-party file(s) |
| `docs/HANDOFFS/PROBATE/PROBATE_HANDOFF.md` | 2026-08-16 | **LIVING** | current campaign window (2026-08-16); symbol `PatchParams` lives in 8 first-party file(s) |
| `docs/HANDOFFS/PROCESS_LAWS.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `patchIndexCount_` lives in 2 first-party file(s) |
| `docs/HANDOFFS/TETRIS/ATLAS_1_HANDOFF.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `Store` still lives in 6 file(s); landed: 08c0d10 LOOM_CLOSE C1: hygiene — the twins fold, the fin |
| `docs/HANDOFFS/TETRIS/ATLAS_1revA_HANDOFF.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `compute_vp` still lives in 7 file(s); landed: 6a08357 CLOSE_0 tail: the RECORD sweep — harvest, header |
| `docs/HANDOFFS/TETRIS/ATLAS_1revB_HANDOFF.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `compute_vp` still lives in 7 file(s); landed: 08c0d10 LOOM_CLOSE C1: hygiene — the twins fold, the fin |
| `docs/HANDOFFS/TETRIS/DISCARD_0_HANDOFF.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `RenderAttachment` still lives in 5 file(s); landed: 23ec88f DOMESDAY_0: report |
| `docs/HANDOFFS/TETRIS/TETRIS_HANDOFF.md` | 2026-08-10 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `painting_slots` still lives in 7 file(s); landed: 6bceca6 TETRIS HANDOFFS |
| `docs/HANDOFFS/TETRIS/TOGGLE_0_HANDOFF.md` | 2026-08-13 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: a281a96 DOMESDAY_2 F5-d: the dialect knob that actually ar |
| `docs/HANDOFFS/TETRIS/TOGGLE_1_HANDOFF.md` | 2026-08-13 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: a281a96 DOMESDAY_2 F5-d: the dialect knob that actually ar |
| `docs/HANDOFFS/WEB/PIPE_0_DECISION.md` | 2026-08-07 | **LIVING** | open: "PARKED per Jean's 2026-08-07 directive" — open finding outranks archival |
| `docs/HANDOFFS/WEB/STREAM_0_DECISION.md` | 2026-08-07 | **LIVING** | open: "PARKED per Jean's 2026-08-07 directive" — open finding outranks archival |

### `docs/HANDOFFS/past campaigns/` — archived

*81 rows — DEAD 81*

| path | last commit | class | evidence |
|---|---|---|---|
| `docs/HANDOFFS/past campaigns/7 - INIT/cc_handoff_boot_one_voice.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_boot_one_voice` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/A FEW TWEAKS/cc_handoff_mood_cut_B.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_mood_cut_B` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/A FEW TWEAKS/cc_handoff_tweaks_A.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_tweaks_A` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/COLLISION TIDINESS/AUDIT_5_COLLISION_CENSUS.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `AUDIT_5_COLLISION` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/DAWN RELEASE CAMPAIGN/HANDOFF_MERGE_1.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `HANDOFF_MERGE_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/DAWN RELEASE CAMPAIGN/HANDOFF_RELEASE_1.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `HANDOFF_RELEASE_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/FLORA/FLORA_1.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `FLORA_1` has 0 hits in first-party code/law/build; landed: fe38e5d FLORA_1 F7b: the blade count is capped by the slot |
| `docs/HANDOFFS/past campaigns/GOL ZONES FIX AND MORE/REPAIR 1 LIFT and TINT/cc_handoff_ug_repair_A_lift_sign.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_repair_A_lift_sign` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GOL ZONES FIX AND MORE/REPAIR 1 LIFT and TINT/cc_handoff_ug_repair_B_tint_uv.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_repair_B_tint_uv` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GOL ZONES FIX AND MORE/REPAIR 2 CELL ALIGNMENT/cc_handoff_ug_repair_2_A_decode_cell_owner.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_repair_2_A_decode_cell_owner` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GOL ZONES FIX AND MORE/REPAIR 2 CELL ALIGNMENT/cc_handoff_ug_repair_2_B_carried_address.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_repair_2_B_carried_address` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GOL ZONES FIX AND MORE/REPAIR 2 CELL ALIGNMENT/cc_handoff_ug_repair_2_C_tint_local_cell.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_repair_2_C_tint_local_cell` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/C 0-4/c0_c1_geometry_plasticity.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `c0_c1_geometry_plasticity` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/C 0-4/c2_c3_steering_social.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `c2_c3_steering_social` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/C 0-4/c4_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `c4_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/F 0-4/f0_f1_f2_sign_and_mask.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `f0_f1_f2_sign_and_mask` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/F 0-4/f3_f4_topology_split_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `f3_f4_topology_split_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h0_index_preflight.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h0_index_preflight` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h1_stage1_sweep.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h1_stage1_sweep` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h2_stage1_fix_fold.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h2_stage1_fix_fold` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h3_stage2_card.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h3_stage2_card` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h4_stage3_render.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h4_stage3_render` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h5_stage4_compute.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h5_stage4_compute` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/H 0-6/h6_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `h6_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/HANDOFF AUDIT 3/cc_handoff_audit3.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_audit3` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/K 0-3/k0_k1_imposed_voluntary.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `k0_k1_imposed_voluntary` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/K 0-3/k2_tuning_pass.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `k2_tuning_pass` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/K 0-3/k3_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `k3_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/P 0-3/p0_p1_influence_law.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `p0_p1_influence_law` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/P 0-3/p2_point_law.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `p2_point_law` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/P 0-3/p3_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `p3_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/S 0-3/s0_s1_scale_ledger.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `s0_s1_scale` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/S 0-3/s2_corrections.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `s2_corrections` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/S 0-3/s3_shells_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `s3_shells_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/T 0-3/t0_index_preflight.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `t0_index_preflight` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/T 0-3/t1_stage6_trueband.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `t1_stage6_trueband` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/T 0-3/t2_t3_stage7_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `t2_t3_stage7_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/U 0-6/u0_index_preflight.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `u0_index_preflight` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/U 0-6/u1_opening.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `u1_opening` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/U 0-6/u2_topology.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `u2_topology` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/U 0-6/u3_decode_lift.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `u3_decode_lift` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/GPU CAMPAIGN/U 0-6/u4_u5_u6_retirement_mask_closeout.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `u4_u5_u6_retirement_mask_closeout` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/MOSAIC/MOSAIC_HANDOFFS.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `MOSAIC` has 0 hits in first-party code/law/build; landed: 0b31f7d ARENA_0: mesh-arena recon report (read-only) |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/BOOT_RUNBOOK.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `BOOT_RUNBOOK` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/CENSUS_1_REPORT.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `CENSUS_1` has 0 hits in first-party code/law/build; landed: 5519c32 PROCESS: the platform event — Dawn Release build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/ECONOMY_1_CHARTER.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `ECONOMY_1_CHARTER` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_census_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_census_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_census_1_addendum.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_census_1_addendum` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_census_2.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_census_2` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_census_3.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_census_3` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_draw_plan_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_draw_plan_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_economy_1a.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_economy_1a` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_economy_close.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_economy` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_hotfix_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_hotfix_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_probe_1_rev2.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_probe_1_rev2` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_probe_arms.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_probe_arms` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/OPTIMIZATION 1/cc_handoff_sweep_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_sweep_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/PAWN DESIGN CAMPAIGN/H1_pawn_figures_registry.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `H1_pawn_figures_registry` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/PAWN DESIGN CAMPAIGN/H2_gpu_state_and_binding.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `H2_gpu_state_and_binding` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/PAWN DESIGN CAMPAIGN/H3_world_wgsl_figures.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `H3_world_wgsl_figures` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/PAWN DESIGN CAMPAIGN/H4_sampler_and_boot.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `H4_sampler_and_boot` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/PRUNING/cc_p2a_dead_doors.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_p2a_dead_doors` has 0 hits in first-party code/law/build; landed: 251f44a cc_p2a_dead_doors.txt and some chaneges in pawn de |
| `docs/HANDOFFS/past campaigns/PRUNING/pruning_1_p0_census.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `pruning_1_p0` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/ROUND AND SHARP/cc_handoff_batch_c.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_batch_c` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/ROUND AND SHARP/cc_handoff_batch_d.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_batch_d` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/ROUND AND SHARP/cc_handoff_point_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_point_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/ROUND AND SHARP/cc_handoff_request_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_request_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/SHADOWS/PENUMBRA_1_HANDOFFS.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `PENUMBRA_1` has 0 hits in first-party code/law/build; landed: ac0b588 FORMAT_1 U2: depth16unorm shadow maps |
| `docs/HANDOFFS/past campaigns/SHADOWS/UMBRA_HANDOFFS.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `UMBRA` has 0 hits in first-party code/law/build; landed: fff6a15 SOAK_0: the purse table, and the shadow lever move |
| `docs/HANDOFFS/past campaigns/SPAWN CAMPAIGN/cc_handoff_spawn_0.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_spawn_0` has 0 hits in first-party code/law/build; landed: a396c94 SPAWN_0: the Part B audit report |
| `docs/HANDOFFS/past campaigns/SPAWN CAMPAIGN/cc_handoff_spawn_1.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_spawn_1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/SPAWN CAMPAIGN/spawn_campaign_v1.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `spawn_campaign_v1` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/SPAWN CAMPAIGN/spawn_campaign_v2.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `spawn_campaign_v2` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/STRAWBERRY FIELDS/STRAWBERRY FIELDS.txt` | 2026-08-12 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `STRAWBERRY FIELDS` has 0 hits in first-party code/law/build; landed: d1fbbcb LAW: banner witness protocol merged; L14 default-l |
| `docs/HANDOFFS/past campaigns/UG FIELDS FOREVER/FIRST ROUND/cc_handoff_ug_fields_1_A0_audit.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_fields_1_A0_audit` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/UG FIELDS FOREVER/FIRST ROUND/ug_fields_1_campaign.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `ug_fields_1_campaign` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/UG FIELDS FOREVER/SECOND ROUND/cc_handoff_ug_fields_1_S1_coverage.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_fields_1_S1_coverage` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/UG FIELDS FOREVER/SECOND ROUND/cc_handoff_ug_fields_1_S2_footprint.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_handoff_ug_fields_1_S2_footprint` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/WEB 1/C6_8FIT_HANDOFF.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `C6_8FIT` has 0 hits in first-party code/law/build |
| `docs/HANDOFFS/past campaigns/WEB 1/OPT_1_HANDOFF.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `OPT_1` has 0 hits in first-party code/law/build; landed: 3aa371c docs filing: the live governing files come back ou |
| `docs/HANDOFFS/past campaigns/WEB 1/SHIP_0_HANDOFF.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `SHIP_0` has 0 hits in first-party code/law/build; landed: 9a00f11 LANTERN U1: the web grants census — the treasury |

### `docs/past docs/` — archived

*19 rows — DEAD 13, LAW 2, LIVING 3, UNSURE 1*

| path | last commit | class | evidence |
|---|---|---|---|
| `docs/past docs/AFTER_READ_REPORT.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `AFTER_READ` has 0 hits in first-party code/law/build; landed: 9e951c4 AFTER_READ_REPORT, mop.patch and Coupling saga fin |
| `docs/past docs/COUPLING_SAGA_FINISHER.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `COUPLING_SAGA_FINISHER` has 0 hits in first-party code/law/build |
| `docs/past docs/CURTAIN_REPORT.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `CURTAIN` has 0 hits in first-party code/law/build; landed: 345628b curtain: the slab walls enter the shadow map |
| `docs/past docs/DAWN_REFERENCE.md` | 2026-08-16 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `DAWN_REFERENCE` has 0 hits in first-party code/law/build; landed: e7f3242 PROBATE_E5: the shelved DAWN_REFERENCE carries its |
| `docs/past docs/FLORA_1_LEDGER.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `FLORA_1` has 0 hits in first-party code/law/build; landed: fe38e5d FLORA_1 F7b: the blade count is capped by the slot |
| `docs/past docs/MIDI_PORT_INTEGRATION.txt` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `MIDI_PORT_INTEGRATION` has 0 hits in first-party code/law/build |
| `docs/past docs/TUNE_1.md` | 2026-08-07 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/bodies/cube_behaviors.hpp` (+8 more) |
| `docs/past docs/TUNE_2.md` | 2026-08-07 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/bodies/cube_behaviors.hpp` (+1 more) |
| `docs/past docs/UMBRA_REPORT.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `UMBRA` has 0 hits in first-party code/law/build; landed: fff6a15 SOAK_0: the purse table, and the shadow lever move |
| `docs/past docs/cartridge_constitution.md` | 2026-08-07 | **LAW** | named by docs/LAWS.md and tools/pruning_census.py:42 |
| `docs/past docs/cc_session_entities_K1.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `cc_session_entities_K1` has 0 hits in first-party code/law/build |
| `docs/past docs/demo_contract_v0.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `demo_contract_v0` has 0 hits in first-party code/law/build; landed: 16fc997 Residue sweep T0: the comment sweep — breadcrumb |
| `docs/past docs/entity_contract_v0.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `entity_contract_v0` has 0 hits in first-party code/law/build |
| `docs/past docs/ground_card_campaign_v2.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `ground_card_campaign_v2` has 0 hits in first-party code/law/build; landed: 7e76bec TIDY_0b: EOL_1 — trailing newline on the untermi |
| `docs/past docs/mop.patch` | 2026-08-07 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `docs/past docs/ribbon_certification_transcript.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `ribbon_certification_transcript` has 0 hits in first-party code/law/build; landed: dd2b3ed Ratification residue: the last audit findings clos |
| `docs/past docs/ribbon_color_coupling_datasheet.md` | 2026-08-07 | **DEAD** | archived by 9f0109f "HANDOFFS FOR WEB" (2026-08-07); subject token `ribbon_color_coupling_datasheet` has 0 hits in first-party code/law/build; landed: 5f2b27a both cartridges: CB-1f — the free raffle (terrai |
| `docs/past docs/terrain_program_charter.md` | 2026-08-07 | **LAW** | named by docs/LAWS.md |
| `docs/past docs/the_board_seam_map.md` | 2026-08-07 | **LIVING** | referenced from first-party code/law/build: `docs/HANDOFFS/PROCESS_LAWS.md` |

### `audit/` (top)

*27 rows — LAW 7, LIVING 6, UNSURE 14*

| path | last commit | class | evidence |
|---|---|---|---|
| `audit/ARENA_0_RECON.md` | 2026-08-10 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `columnMeshParamsBuffer_` still lives in 3 file(s); landed: 0b31f7d ARENA_0: mesh-arena recon report (read-only) |
| `audit/ATLAS_1_RECON.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `sample_spot_shadow_pcf` still lives in 2 file(s); landed: 08c0d10 LOOM_CLOSE C1: hygiene — the twins fold, the fin |
| `audit/ATLAS_1revA_GATE.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `instance_index` still lives in 2 file(s); landed: 6a08357 CLOSE_0 tail: the RECORD sweep — harvest, header |
| `audit/ATLAS_1revB_U0.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `minUniformBufferOffsetAlignment` still lives in 2 file(s); landed: 6a08357 CLOSE_0 tail: the RECORD sweep — harvest, header |
| `audit/BINDING_LEDGER.md` | 2026-08-17 | **LAW** | authority itself; GENERATED by tools/binding_ledger.py |
| `audit/COMMAND_LEDGER.md` | 2026-08-15 | **LAW** | GENERATED by tools/command_census.py (DEFAULT_OUT:64) |
| `audit/DOMESDAY_0_REPORT.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `master` lives in 14 first-party file(s) |
| `audit/DOMESDAY_1_REPORT.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `master` lives in 14 first-party file(s) |
| `audit/DOMESDAY_2_REPORT.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `master` lives in 14 first-party file(s) |
| `audit/DOMESDAY_CLOSE.md` | 2026-08-15 | **LIVING** | current campaign window (2026-08-15); symbol `immediate_size` lives in 3 first-party file(s) |
| `audit/FXC_LAWS_RECORD.md` | 2026-08-16 | **LAW** | named by docs/LAWS.md and audit/BINDING_LEDGER.md |
| `audit/HEADROOM_0_U0.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `URLSearchParams` still lives in 2 file(s); landed: 8531439 CLOSE_0 A5 / HEADROOM_0 U3: the entity census gets |
| `audit/LANTERN_CENSUS.md` | 2026-08-15 | **LIVING** | referenced from first-party code/law/build: `src/core/boot_params.hpp` |
| `audit/MANIFEST.md` | 2026-08-17 | **LAW** | authority itself; GENERATED by tools/binding_gen.py, holds witness M-1 |
| `audit/MIRROR_LEDGER.md` | 2026-08-14 | **LAW** | GENERATED by tools/mirror_census.py (DEFAULT_OUT:57) |
| `audit/OIL_LEDGER.md` | 2026-08-09 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `RENDER_SPINE` still lives in 2 file(s); landed: cef3771 audit: OIL_1c closes the R1 finding — 15 FIXED,  |
| `audit/OPT_1_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 3aa371c docs filing: the live governing files come back ou |
| `audit/PASS_LEDGER.md` | 2026-08-13 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/realization/render_passes.hpp` |
| `audit/RECUT_PLAN.md` | 2026-08-14 | **LAW** | named by the LOOM_2 tool chain as the recut destination |
| `audit/SHADE_0_CENSUS.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `shadow` still lives in 25 file(s); landed: dddb30d TOGGLE_0 U2: retire the control; verdict — posit |
| `audit/SHADOW_0_RECON.md` | 2026-08-12 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `master` still lives in 14 file(s); landed: 89ab1b8 WEB_METER_0 baseline: the indoor windows (capture  |
| `audit/SHIP_0_REPORT.md` | 2026-08-07 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 9a00f11 LANTERN U1: the web grants census — the treasury |
| `audit/SKY_0_RECON.md` | 2026-08-12 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `master` still lives in 14 file(s); landed: c17df89 SKY_0: sky/portal design census (read-only) |
| `audit/SOAK_0.md` | 2026-08-13 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: fff6a15 SOAK_0: the purse table, and the shadow lever move |
| `audit/THE BOARD FULL RELEASE CONSOLE.md` | 2026-08-07 | **LAW** | named by docs/LAWS.md |
| `audit/TOGGLE_1_U0.md` | 2026-08-13 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `nullptr` still lives in 22 file(s); landed: 6a08357 CLOSE_0 tail: the RECORD sweep — harvest, header |
| `audit/WEB_METER_0_BASELINE.md` | 2026-08-12 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `indoor_vault` still lives in 3 file(s); landed: a690f7a REFOUND_0: ledger regen at a73267c |

### `audit/past reports/` — archived

*73 rows — DEAD 61, LAW 2, LIVING 7, UNSURE 3*

| path | last commit | class | evidence |
|---|---|---|---|
| `audit/past reports/ANCHOR_1_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `ANCHOR_1` has 0 hits in first-party code/law/build; landed: 327501c ANCHOR_D1: the walk — targets in the kernel, the |
| `audit/past reports/BATCH_A_WITNESS.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_A_WITNESS` has 0 hits in first-party code/law/build |
| `audit/past reports/BATCH_B_PREDICTION.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_B_PREDICTION` has 0 hits in first-party code/law/build |
| `audit/past reports/BATCH_C_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_C` has 0 hits in first-party code/law/build; landed: 05e0df1 BATCH C + D: the verification addendum — six ske |
| `audit/past reports/BATCH_D_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_D` has 0 hits in first-party code/law/build; landed: 05e0df1 BATCH C + D: the verification addendum — six ske |
| `audit/past reports/BATCH_E_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_E` has 0 hits in first-party code/law/build |
| `audit/past reports/BATCH_FB_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_FB` has 0 hits in first-party code/law/build |
| `audit/past reports/BATCH_F_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_F` has 0 hits in first-party code/law/build; landed: 059d67b SALON_1 A: the recon ledger (read-only) |
| `audit/past reports/BATCH_G_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BATCH_G` has 0 hits in first-party code/law/build |
| `audit/past reports/BINDING_REGISTRY_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `BINDING_REGISTRY` has 0 hits in first-party code/law/build |
| `audit/past reports/CARTRIDGE_MODULE_CENSUS.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `CARTRIDGE_MODULE` has 0 hits in first-party code/law/build; landed: b21eca6 CENSUS_3 U3: delete the PointTerrainRule enum —  |
| `audit/past reports/CC_AUDIT_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `CC_AUDIT` has 0 hits in first-party code/law/build; landed: faf26c3 drift sweep: comments state present behavior (audi |
| `audit/past reports/CLOSEOUT_CAMPAIGN_AB.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `CLOSEOUT_CAMPAIGN_AB` has 0 hits in first-party code/law/build |
| `audit/past reports/COLOR_STACK_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `COLOR_STACK` has 0 hits in first-party code/law/build |
| `audit/past reports/COMPACT0_CENSUS.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `COMPACT0` has 0 hits in first-party code/law/build |
| `audit/past reports/COMPACT1_SWEEP.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `COMPACT1_SWEEP` has 0 hits in first-party code/law/build; landed: ca0c483 COMPACT-1 (3/3): close-out — tokens recovered, p |
| `audit/past reports/COMPACT2_SWEEP.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `COMPACT2_SWEEP` has 0 hits in first-party code/law/build; landed: 9ed5274 COMPACT-2: the prototype comment law — one sweep |
| `audit/past reports/COMPAT_CONSUMER_CENSUS.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `COMPAT_CONSUMER` has 0 hits in first-party code/law/build |
| `audit/past reports/COMPOSITION_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `COMPOSITION` has 0 hits in first-party code/law/build; landed: e81dc49 ECONOMY_1 E1: cap-only index buffer + lift-conserv |
| `audit/past reports/CUT_1_LIMITS_FIT_NOTE.md` | 2026-08-07 | **LIVING** | open: "held branch; awaiting Jean's design" — open finding outranks archival |
| `audit/past reports/DEFERRED_REGISTER.md` | 2026-08-07 | **LIVING** | referenced from first-party code/law/build: `tools/pruning_census.py` |
| `audit/past reports/DISSOLVE1_FACES.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `DISSOLVE1_FACES` has 0 hits in first-party code/law/build |
| `audit/past reports/FIELD_1_REPORT.md` | 2026-08-07 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `FIELD_1` has 0 hits in first-party code/law/build; landed: 0f84442 FIELD_1: avoidance field recon ledger (read-only) |
| `audit/past reports/FIELD_BRIDGE.md` | 2026-08-07 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/contracts/control_panel.hpp` (+1 more) |
| `audit/past reports/FRAME_CONDUCTOR_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `FRAME_CONDUCTOR` has 0 hits in first-party code/law/build |
| `audit/past reports/INVESTIGATION_mood_seam.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `INVESTIGATION_mood_seam` has 0 hits in first-party code/law/build |
| `audit/past reports/LADDER.md` | 2026-07-29 | **LIVING** | referenced from first-party code/law/build: `audit/tools/glaw1/run.sh` (+34 more) |
| `audit/past reports/LADDER3_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `LADDER3` has 0 hits in first-party code/law/build |
| `audit/past reports/LADDER5_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `LADDER5` has 0 hits in first-party code/law/build; landed: c4fe407 audit: LADDER-5 RECON — the hubs (read-only; con |
| `audit/past reports/LADDER6_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `LADDER6` has 0 hits in first-party code/law/build |
| `audit/past reports/LEDGER_1_REPORT.md` | 2026-08-07 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `LEDGER_1` has 0 hits in first-party code/law/build; landed: 6e82aba OPT_1 O0-f: the per-mood frustum-cull knob is iner |
| `audit/past reports/PANEL0_P2_MATRIX_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `PANEL0_P2_MATRIX` has 0 hits in first-party code/law/build; landed: 46897af PANEL-0 p2 (recon): the matrix — pieces×demos g |
| `audit/past reports/PANEL0_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `PANEL0` has 0 hits in first-party code/law/build; landed: 46897af PANEL-0 p2 (recon): the matrix — pieces×demos g |
| `audit/past reports/PATCH_GEN_SPAWN_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `PATCH_GEN_SPAWN` has 0 hits in first-party code/law/build; landed: 00e172e audit: retire the test-rig entries; cite symbols,  |
| `audit/past reports/POINT_1_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `POINT_1` has 0 hits in first-party code/law/build; landed: 186fd39 LEDGER_1: terrain compute ledger — producers, co |
| `audit/past reports/POINT_P1B_AUDIT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `POINT_P1B_AUDIT` has 0 hits in first-party code/law/build; landed: 36434d5 PANEL-0 p1b-a: the point's position (host-sourced; |
| `audit/past reports/PORTAL_0_REPORT.md` | 2026-08-07 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `PORTAL_0` has 0 hits in first-party code/law/build; landed: 91dcf74 PORTAL_0: door census (read-only) |
| `audit/past reports/PORTAL_COLOR_1.md` | 2026-08-07 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `PORTAL_COLOR_1` has 0 hits in first-party code/law/build; landed: dbcabbe PORTAL_1 C3+C4: every arch mesh-param producer rea |
| `audit/past reports/PORT_0_SEAM_LEDGER.md` | 2026-08-07 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `PORT_0_SEAM` has 0 hits in first-party code/law/build; landed: e0d248b CUT_1a: vestigial includes, dead assets, dead post |
| `audit/past reports/PRUNING_1_CENSUS.md` | 2026-07-29 | **LAW** | GENERATED by tools/pruning_census.py (OUT:47) — see STALE-PATH flag |
| `audit/past reports/RADIUS_INVENTORY.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `RADIUS_INVENTORY` has 0 hits in first-party code/law/build; landed: 3aac3db campaign: L0 alignment — the ground's ratificati |
| `audit/past reports/REACH_GRAPH.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `REACH_GRAPH` has 0 hits in first-party code/law/build |
| `audit/past reports/REBUILD0_M3_RECIPE.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `REBUILD0_M3_RECIPE` has 0 hits in first-party code/law/build; landed: d1b715a REBUILD-0 m3a: the bypass cleanups — keyholes fo |
| `audit/past reports/REBUILD0_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `REBUILD0` has 0 hits in first-party code/law/build; landed: 00e172e audit: retire the test-rig entries; cite symbols,  |
| `audit/past reports/RENDER_UPDATE_API_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `RENDER_UPDATE_API` has 0 hits in first-party code/law/build; landed: 3e76135 Binding registry C6 recon (read-only): the single- |
| `audit/past reports/REQUEST_1_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `REQUEST_1` has 0 hits in first-party code/law/build; landed: 186fd39 LEDGER_1: terrain compute ledger — producers, co |
| `audit/past reports/RESIDUE_SWEEP_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `RESIDUE_SWEEP` has 0 hits in first-party code/law/build |
| `audit/past reports/ROSTER_GATE_A.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `ROSTER_GATE_A` has 0 hits in first-party code/law/build; landed: 93e6c10 SWEEP 03 — ARCHAEOLOGY COLLAPSE: provenance to o |
| `audit/past reports/ROSTER_RECON.md` | 2026-07-29 | **LIVING** | referenced from first-party code/law/build: `src/New chat first handoff.txt` |
| `audit/past reports/SHELL.md` | 2026-08-12 | **LIVING** | referenced from first-party code/law/build: `CMakeLists.txt` (+11 more) |
| `audit/past reports/SIBLING_PRUNE_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SIBLING_PRUNE` has 0 hits in first-party code/law/build |
| `audit/past reports/SIGNAL_SOURCE_LEDGER.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SIGNAL_SOURCE` has 0 hits in first-party code/law/build |
| `audit/past reports/SPAWN_0_PART_B_AUDIT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SPAWN_0_PART_B_AUDIT` has 0 hits in first-party code/law/build |
| `audit/past reports/SPAWN_1_PREDICTION.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SPAWN_1_PREDICTION` has 0 hits in first-party code/law/build; landed: b280d2c SPAWN_1a: wire the census — three trigger sites, |
| `audit/past reports/SPAWN_1_REPORT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SPAWN_1` has 0 hits in first-party code/law/build; landed: cf11a45 SPAWN_2: floaters leave the registry — the subtr |
| `audit/past reports/SPAWN_2_PREDICTION.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SPAWN_2_PREDICTION` has 0 hits in first-party code/law/build |
| `audit/past reports/SPAWN_ENGINE_DOSSIER.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SPAWN_ENGINE_DOSSIER` has 0 hits in first-party code/law/build |
| `audit/past reports/SWEEP_CLOSEOUT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `SWEEP_CLOSEOUT` has 0 hits in first-party code/law/build; landed: faf26c3 drift sweep: comments state present behavior (audi |
| `audit/past reports/TERRAIN0_AUDIT.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `TERRAIN0_AUDIT` has 0 hits in first-party code/law/build; landed: de53a19 TERRAIN-0 (audit): the "what is terrain?" definiti |
| `audit/past reports/TERRAIN1_MANIFOLD_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `TERRAIN1_MANIFOLD` has 0 hits in first-party code/law/build; landed: d25cb7f TERRAIN-1 (recon): the manifold map — two target |
| `audit/past reports/TERRAIN2_STAGE1_INTERFACE.md` | 2026-07-29 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/realization/world.wgsl` |
| `audit/past reports/TERRAIN_COUPLING_LEDGER.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `TERRAIN_COUPLING` has 0 hits in first-party code/law/build |
| `audit/past reports/TERRAIN_DOSSIER.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `TERRAIN_DOSSIER` has 0 hits in first-party code/law/build; landed: 00e172e audit: retire the test-rig entries; cite symbols,  |
| `audit/past reports/VEIL_VISIBILITY_RECON.md` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `VEIL_VISIBILITY` has 0 hits in first-party code/law/build; landed: 376115f The veil: one point-anchored visibility authority  |
| `audit/past reports/WEB_PORT_LEDGER.md` | 2026-07-29 | **LAW** | named by docs/LAWS.md and .gitattributes:7 (deploy chain) |
| `audit/past reports/ZOETROPE_1_REPORT.md` | 2026-08-07 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `ZOETROPE_1` has 0 hits in first-party code/law/build; landed: 5dbff97 ZOETROPE_1: cube lattice recon ledger (read-only) |
| `audit/past reports/cc1_diff_binding_registry.patch` | 2026-07-28 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `audit/past reports/cc1_diff_spine_state.patch` | 2026-07-28 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `audit/past reports/cc1_hunks_state.txt` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `cc1_hunks_state` has 0 hits in first-party code/law/build |
| `audit/past reports/cc1_hunks_wgsl.txt` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `cc1_hunks_wgsl` has 0 hits in first-party code/law/build |
| `audit/past reports/cc2_grep_log.txt` | 2026-07-29 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `cc2_grep` has 0 hits in first-party code/law/build |
| `audit/past reports/ground_card_campaign_v2.md` | 2026-08-12 | **DEAD** | archived by 4c1a804 "reorganizing audit folder" (2026-08-07); subject token `ground_card_campaign_v2` has 0 hits in first-party code/law/build; landed: 7e76bec TIDY_0b: EOL_1 — trailing newline on the untermi |
| `audit/past reports/probe_a.patch` | 2026-07-28 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |

### `audit/tools/`

*1 rows — LIVING 1*

| path | last commit | class | evidence |
|---|---|---|---|
| `audit/tools/glaw1/members.txt` | 2026-07-30 | **LIVING** | referenced from first-party code/law/build: `audit/BINDING_LEDGER.md` (+27 more) |

### `CLAUDE CODE/`

*25 rows — LIVING 4, UNSURE 21*

| path | last commit | class | evidence |
|---|---|---|---|
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/A2_P1_cleanup.patch` | 2026-07-24 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/A2_P2_stage5_retirement.patch` | 2026-07-24 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/AUDIT2_REPORT.md` | 2026-07-24 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 6a50738 [U6] Closeout: _post_ug1 recount, U-roster Dawn wi |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/AUDIT3_REPORT.md` | 2026-07-24 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 8465b4e [T3c] BATCH_REPORT_TC1.md + campaign log T3 — th |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/AUDIT4_REPORT.md` | 2026-07-24 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 6375070 [AUDIT-4] THE LEVER AUDIT: AUDIT4_REPORT.md (read- |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/AUDIT_5_REPORT.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `influence_response` still lives in 1 file(s); landed: faae818 [AUDIT-5] THE COLLISION CENSUS: AUDIT_5_REPORT.md  |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/AUDIT_REPORT.md` | 2026-07-24 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 79a122c CLOSE_0 A2: canonize the session's five laws as L1 |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT.md` | 2026-07-24 | **LIVING** | referenced from first-party code/law/build: `tools/pruning_census.py` |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_C2.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `CONTACT_2` still lives in 5 file(s); landed: 663f805 [C4] CONTACT_2 closeout: _post_c2 instruments + BA |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_C3.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `agent_post_step` still lives in 2 file(s); landed: b977e4b [K3] CONTACT_3 closeout: _post_c3 instruments + BA |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_C4.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `personal_radius` still lives in 3 file(s); landed: cd6c0f4 [S3b] CONTACT_4 closeout: _post_c4 instruments + B |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_C5.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `influence_response` still lives in 1 file(s); landed: 60818b0 [P3] CONTACT_5 closeout: _post_c5 instruments + BA |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_TC1.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `evaluate_lattice_wave_pair` still lives in 1 file(s); landed: 8465b4e [T3c] BATCH_REPORT_TC1.md + campaign log T3 — th |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_TIDY1.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `CUBE_PUSH_CAP` still lives in 1 file(s); landed: 0af8869 [T3] TIDY_1: the collision charter + batch report  |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/BATCH_REPORT_UG1.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `ug_cell_lift` still lives in 1 file(s); landed: 6a50738 [U6] Closeout: _post_ug1 recount, U-roster Dawn wi |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/CLAUDE.md` | 2026-07-25 | **LIVING** | referenced from first-party code/law/build: `tools/pruning_census.py` |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/COLLISION_CHARTER.md` | 2026-07-24 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/realization/world.wgsl` |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/CONTACT_2_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `master` still lives in 14 file(s); landed: 327501c ANCHOR_D1: the walk — targets in the kernel, the |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/CONTACT_3_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `agent_post_step` still lives in 2 file(s); landed: cd6c0f4 [S3b] CONTACT_4 closeout: _post_c4 instruments + B |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/CONTACT_4_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `PATCH_EXTENT` still lives in 12 file(s); landed: 0f4d248 [T2b] TIDY_1: cube reach -- ceiling + planar senti |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/CONTACT_5_LOG.md` | 2026-07-24 | **LIVING** | referenced from first-party code/law/build: `src/cartridges/the_board/realization/world.wgsl` |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/GROUND_CARD_1_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `GROUND_CARD_1` still lives in 9 file(s); landed: bacde5b CENSUS_3a: prose dispositions — R4-R6, R8-R13 (d |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/TIDY_1_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 refs anywhere, no greppable symbol in title/first section; landed: 35d30be Merge TIDY_1 (collision refactor + charter) + F3 c |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/TRUEBAND_CONTACT_1_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `active` still lives in 37 file(s); landed: 57be2b1 [T2] Contact: profile columns + walker gathers + c |
| `CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/UNIFIED_GROUND_1_LOG.md` | 2026-07-24 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `UNIFIED_GROUND_1` still lives in 7 file(s); landed: eda1472 METER_1 [2]: GPU pass timestamps — timestamp-que |

### `src/`

*1 rows — UNSURE 1*

| path | last commit | class | evidence |
|---|---|---|---|
| `src/New chat first handoff.txt` | 2026-08-12 | **UNSURE** | not archived, 0 code/law/build refs; subject token absent from code, but symbol `init_renderer` still lives in 6 file(s); landed: 7e76bec TIDY_0b: EOL_1 — trailing newline on the untermi |

### `tools/`

*1 rows — LAW 1*

| path | last commit | class | evidence |
|---|---|---|---|
| `tools/gates/console_gate/PROVENANCE.md` | 2026-08-16 | **LAW** | named by .gitattributes (GATE_1 sha256 pin); deploy/gate chain |

### `third_party/` — VENDORED

*5 rows — LAW 1, LIVING 2, UNSURE 2*

| path | last commit | class | evidence |
|---|---|---|---|
| `third_party/emdawnwebgpu/PINNED.md` | 2026-08-16 | **LAW** | named by docs/LAWS.md, CMakeLists.txt, .gitattributes (sha256 pin) |
| `third_party/emdawnwebgpu/emdawnwebgpu_pkg/README.md` | 2026-08-16 | **LIVING** | referenced from first-party code/law/build: `CMakeLists.txt` (+1 more) |
| `third_party/emdawnwebgpu/emdawnwebgpu_pkg/VERSION.txt` | 2026-08-16 | **LIVING** | referenced from first-party code/law/build: `CMakeLists.txt` (+3 more) |
| `third_party/emdawnwebgpu/emdawnwebgpu_pkg/webgpu/src/LICENSE` | 2026-08-16 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
| `third_party/emdawnwebgpu/emdawnwebgpu_pkg/webgpu_cpp/LICENSE` | 2026-08-16 | **UNSURE** | ADDED BY SUSPICION — documentary but outside the four globs; not decided this round |
### Untracked documents

`git ls-files --others --exclude-standard` matching the four globs returns **nothing**.
The working tree has **zero** untracked files of any kind, so this table is empty by
measurement, not by omission.

| path | class |
|---|---|
| *(none — 0 untracked files in the tree)* | — |

---

## C3 — Search-noise census

For each VENDORED / GENERATED-BUT-TRACKED tree: one identifier from its main public
header that a first-party search falsely hits, with the verification command and its
measured hit count. This seeds the `.ignore` in WINNOW-2.

Counts are **matching lines** (`git grep -n`), measured on `606924f`.

| tree | identifier | from | verification command | in-tree | outside |
|---|---|---|---|---|---|
| `third_party/emdawnwebgpu/` | `WGPUDevice` | `emdawnwebgpu_pkg/webgpu/include/webgpu/webgpu.h` | `git grep -n -F 'WGPUDevice' -- third_party/emdawnwebgpu` | **174** | 0 |
| `third_party/emdawnwebgpu/` | `wgpuDeviceCreateBuffer` | same header | `git grep -l -F 'wgpuDeviceCreateBuffer'` | 5 files | 0 files |
| `src/external/` | `stbi__` | `stb_image.h` | `git grep -n -F 'stbi__' -- src/external` | **1478** | 1 |
| `src/external/` | `stbi_load` | `stb_image.h` | `git grep -n -F 'stbi_load' -- src/external` | 37 | 14 |
| `tools/gates/console_gate/stubs/` | `GLFWwindow` | `stubs/GLFW/glfw3.h` | `git grep -n -F 'GLFWwindow' -- tools/gates/console_gate/stubs` | **130** | 13 |
| `tools/gates/console_gate/stubs/` | `glfwCreateWindow` | `stubs/GLFW/glfw3.h` | `git grep -n -F 'glfwCreateWindow' -- tools/gates/console_gate/stubs` | 18 | 7 |
| `tools/gates/console_gate/stubs/` | `EMSCRIPTEN_KEEPALIVE` | `stubs/emscripten.h` | `git grep -l -F 'EMSCRIPTEN_KEEPALIVE'` | 1 file | 2 files |

**`src/external/stbi__` is the worst offender in the tree**: 1478 lines of noise against
a single legitimate hit elsewhere. `WGPUDevice` is the cleanest win — 174 in-tree lines,
zero first-party cost to suppressing them.

### The generated ledgers are a *different* kind of noise

The vendored trees hold foreign identifiers. The generated ledgers hold **our own
identifiers, echoed back** — so an `.ignore` that suppresses them changes what a search for
a real pipeline symbol returns. Measured:

| identifier | lines in generated ledgers | lines in first-party code | repo total |
|---|---|---|---|
| `updatePlayerAgentPipeline_` | **7** | 5 | 23 |
| `patchTerrainPipeline_` | **6** | 5 | 21 |
| `shadowPawnPipeline_` | **4** | 4 | 12 |
| `scene_constants` | 11 | 26 | 52 |

For pipeline members the ledgers **out-report the source** — searching
`updatePlayerAgentPipeline_` surfaces more generated rows than real declarations.
This is a judgement call for WINNOW-2 and is deliberately left open: suppressing
`audit/*LEDGER.md` + `audit/MANIFEST.md` sharpens symbol search, but those files are
**LAW** and a reader looking for the wallet lane of a pipeline genuinely wants them.
Recommend a separate `.ignore` profile rather than one blanket file.

---

## Completeness witness

```
$ git ls-files '*.md' '*.txt' '*.rst' '*.adoc' | wc -l
273
```

| quantity | value |
|---|---|
| glob count (command above) | **273** |
| C2 rows from the mechanical glob | **273** |
| **gap** | **0** |
| rows added by suspicion (outside the globs) | 10 |
| **total C2 rows** | **283** |

The mechanical scope and the row count are **equal**; there is no gap to explain.
The 10 added rows are listed separately above and in F-6 so the glob total stays auditable.
Every one of the 283 rows was emitted from the `git ls-files` output itself — the grouping
pass asserted 0 ungrouped rows, so no file can have been dropped silently between
enumeration and table.

---

## Register — flags raised, nothing acted on

Read-only round. Each item is recorded for WINNOW-2; none was fixed.

**F-1 — `tools/pruning_census.py` writes to a path that no longer exists.**
`OUT = "audit/PRUNING_1_CENSUS.md"` (line 47), but the file now lives at
`audit/past reports/PRUNING_1_CENSUS.md`. A census run would **create a second, divergent
file at the old path** rather than update the real one. The tool's `--check` staleness gate
is therefore checking a file that is not there.

**F-2 — Three zero-byte files are tracked at root.**
`device`, `rmdir`, `search_log.txt` — all 0 bytes. `rmdir` and `device` are the names of
shell commands, which is consistent with a mistyped redirect, but nothing in history proves
intent, so all three are **UNSURE**, not DEAD. `search_log.txt` additionally has 0
references repo-wide. Prime WINNOW-2 candidates; Jean's call.

**F-3 — `note.md` is the tree's only UTF-16LE file.**
UTF-16LE + CRLF, 6594 B, while `.gitattributes` sets `* text=auto`. It survives, but it is
the one file that would be mangled by any normalization pass, and it is invisible to a
plain `git grep` for its own contents. Its title is *"CUT_1 / C6 — THE 8-FIT NOTE
(held branch; awaiting Jean's design)"* — i.e. it carries an **open** item.

**F-4 — 41 references point at `audit/LADDER.md`, which moved.**
34 of them are `// History: audit/LADDER.md` header comments across `src/cartridges/`.
The file is at `audit/past reports/LADDER.md`. The archival commit `4c1a804` moved the
target without updating the citations. Full stale-path tally:

| named path | refs | actual location |
|---|---|---|
| `audit/LADDER.md` | 41 | `audit/past reports/LADDER.md` |
| `audit/WEB_PORT_LEDGER.md` | 7 | `audit/past reports/WEB_PORT_LEDGER.md` — **one is `.gitattributes:7`** |
| `web/PORT_MAP.md` | 4 | **absent** — deleted with the web mirror |
| `audit/FIELD_BRIDGE.md` | 4 | `audit/past reports/FIELD_BRIDGE.md` |
| `audit/PRUNING_1_CENSUS.md` | 3 | `audit/past reports/PRUNING_1_CENSUS.md` |
| `audit/DEFERRED_REGISTER.md` | 1 | `audit/past reports/DEFERRED_REGISTER.md` |

60 stale citations in total. WINNOW-2 will move more files; this is the cost of moving
without a citation sweep, already paid once.

**F-5 — Retired vocabulary: two of the four probes are already clean, one is a trap.**

| vocabulary | status |
|---|---|
| `Abbott` / `Costello` / `Louise` | **0 hits repo-wide.** Already fully gone; no document speaks it. |
| `GPUPierInstance` | 0 hits in first-party code; survives in **18 files, all archived reports or `.patch` files**. Genuinely retired machinery. |
| bare `pier` | **NOT retired — 139 hits in live `src/`** (`write_pier`, `pier_height`, catenary arch geometry). |
| `web/` as deploy target | retired; `dist/` is the deploy folder per `.gitignore` and `tools/web_dist.py`. |

**The `pier` trap is the important one.** A WINNOW-2 grep for `pier` as retired vocabulary
would falsely condemn live architectural geometry. Only `GPUPierInstance` is retired; the
arch pier is load-bearing, in both senses. An early pass of this census flagged 53 documents
on a bare-`pier` probe; all 53 were false positives and the probe was narrowed.

`docs/LAWS.md` carries `BOM`/`CRLF` vocabulary at line 22 (*"`world.wgsl` is BOM-free,
LF-terminated, on every platform"*). That is **law recording a retirement**, so it is LAW,
not DEAD — exactly the case the round's spec anticipated. 64 documents mention BOM/CRLF;
vocabulary alone condemned none of them.

**F-6 — Ten documentary files sit outside the four globs.**
6 `.patch` (`CLAUDE CODE/…/A2_P1_cleanup.patch`, `A2_P2_stage5_retirement.patch`,
`audit/past reports/cc1_diff_binding_registry.patch`, `cc1_diff_spine_state.patch`,
`probe_a.patch`, `docs/past docs/mop.patch`), 2 spec `.pdf`, 2 vendored `LICENSE`.
Added to the census as **UNSURE**, undecided, per the round's rule.

**W-2 — `CMakeLists.txt` entered C2 through the `*.txt` glob.**
It is the build authority, not a document. It is kept as a row because the scope is
mechanical and dropping it would make the witness count lie; it is classed **LAW**.
`audit/tools/glaw1/members.txt` (a symbol data file) enters the same way and is classed
LIVING on 28 first-party references.

**F-7 — Token collisions were verified, not trusted.**
Four basenames are common English or ubiquitous strings and their raw grep counts are
meaningless. Re-measured against the literal filename:

| file | raw token hits | literal-path refs |
|---|---|---|
| `note.md` | 190 | **1** |
| `README.md` | 5 | 4 |
| `members.txt` | 84 | 2 |
| `search_log.txt` | 0 | **0** |

`note.md`'s 190 "references" were the English word *note* in code comments. Rows for these
four use the literal-path count.

---

## What WINNOW-2 inherits

- **155 DEAD** rows, each citing the archival commit (`4c1a804` / `9f0109f`) plus a subject
  token with 0 hits in first-party code. This is the kill list's raw material — 169 of the
  283 rows live under a `past *` tree, and 13 of those were promoted back out on real code
  references, so the archive is **not** uniformly safe to delete.
- **63 UNSURE** rows, listed verbatim in the round report. These are the ones that need
  Jean, not a rule.
- **A seeded `.ignore`**: three vendored trees with verified counts, and an explicit warning
  that the generated ledgers are a different problem needing a different profile.
- **60 stale citations** that any further move will multiply.

Nothing in this file has been cut, moved, or edited. One commit, one new file.
