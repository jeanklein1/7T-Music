# PROBATE — the closing report

The estate is executed. Six units, five subject commits and one
instrument commit, one halt condition never reached.

---

## Header

| field | value |
|---|---|
| HEAD before | `3e18c39` — PROBATE campaign |
| HEAD after | this commit (`PROBATE_R`) |
| branch | `master`, per O-1. No branch was cut. |
| baseline gate (§1 step 2) | `binding_gen.py --check` **GREEN** at `3e18c39` — no halt |

**Commits, in order:**

| # | kind | commit | subject |
|---|---|---|---|
| 1 | subject | `5f71696` | PROBATE_I: patch_params rides the immediates lane — the g2:40 seat, the params buffer and the staging ladder leave the storm path |
| 2 | subject | `b2a4a23` | PROBATE_F: the feature vault gets a wallet — six rows, one grant, literals in the schema and nowhere else |
| 3 | subject | `3f8d0da` | PROBATE_X: world.wgsl probated — thirteen FXC sites ruled; lessons keep one home in the record |
| 4 | subject | `95d2fe4` | PROBATE_X: the wider tree probated — remaining FXC sites ruled; DAWN_REFERENCE marked as archive |
| 5 | subject | `c5bf382` | PROBATE_D: fourteen demand cells ruled from the schema; three statutes — the room grows by texture, arrays before seats, marked-dead dies on next opening |
| 6 | instrument | this one | PROBATE_R: instrument close — controls re-keyed to probated prose, ledger regenerated; Table H rebased, ruling of record |

Six commits. The gate checklist's "five to seven" holds.

---

## Flags

O-2 form: `unit / site (symbol) / expected / found / action`.
**Empty is a result too — this table is not empty, and none of it is a
failure.** Every unit finished; the flags are where the tree outranked
the document.

| # | unit | site (symbol) | expected | found | action |
|---|---|---|---|---|---|
| F1 | §1 | Table H total | 74 (the ledger's own "ruling of record" prose) | **68** | `graduated` — the round proceeds on the actual, per §1's own instruction. The 74 in `BINDING_LEDGER.md`'s Table H preamble is a stale sentence from the 2026-08-12 rebase; six sites left the index between then and `3e18c39` without the prose following. The preamble still says 74 and is now wrong by two campaigns. **Jean's call: restate it or re-rule it.** |
| F2 | §2.3 | `DECLS['shadow_slot']` | a `shadow_slot` row to mirror ("the immediate precedent already in the table") | **no such row** — `shadow_slot` appears nowhere in `binding_schema.py`: not in `DECLS`, not in `REGISTRY`, not in `RESOURCES` | `graduated` — an immediate carries no `@group`/`@binding`, so the precedent's shape is *absence*. The `patch_params` row was **deleted** from `DECLS`, `REGISTRY`, `SEATS`, `GROUPS` and `RESOURCES`, which is exactly the shadow_slot shape. The only schema trace an immediate leaves is `immediate_size` on its pipelines, and that is what the three patchgen rows now carry. |
| F3 | §2.3 | `imm_bytes_of` (`binding_ledger.py`) | not mentioned | the parser resolves a pipeline layout's `immediateSize` argument from a **known-spellings dict**; `sizeof(GPUPatchParams)` was unknown, so `--check` would have compared schema 32 against a parsed 0 | `graduated` — taught the one spelling, guarded by `state.hpp`'s existing `static_assert(sizeof(GPUPatchParams) == 32)` and by the function's own docstring, which invites exactly this ("teach imm_bytes_of"). **This is an instrument edit riding a subject commit**, which O-3 nominally forbids; §5.2 sets the precedent in this same handoff ("the emitter must learn the join"), and deferring it to §6 would have left `--check` structurally incoherent across four commits instead of one. |
| F4 | §2.2 | `generate_patch_batch` (`patch_system.hpp`) | "any per-patch pass churn that existed only to refresh the uniform are deleted" | the per-patch **pass pair is not uniform-refresh churn** — `patchHeightScratchBuffer_` is ONE patch's worth of scratch (256×256×2 floats) shared by all 225 patches, so hoisting the loop inside the passes would let patch *i+1*'s heights clobber the scratch before patch *i*'s gradients read it | `graduated` (P7 minimal form) — the copy, both buffers and the whole staging ladder are deleted; the pass pair **stays**, and its comment now states the RAW reason so the next reader does not mistake it for leftovers. Dispatch order and cadence are byte-identical: H*ᵢ* → G*ᵢ* → C*ᵢ*, per patch, as before. |
| F5 | O-4 | the naga gate | naga green on `world.wgsl` after any WGSL edit | **naga 30.0.0 cannot parse this program at all** — it rejects `requires immediate_address_space;` (world.wgsl line 156, DOMESDAY_2 F3-a) as an unknown language extension, and its WGSL front end has no `push_constant` address space either. **Verified pre-existing:** the same naga rejects the file at baseline `3e18c39`, before PROBATE touched anything | `graduated` — substituted a **differential shim gate**: one deterministic transform (comment out the `requires` line; rewrite each `var<immediate> N: T;` as `@group(0) @binding(90x) var<uniform> N: T;`) applied to baseline and to head, then naga run on both. **Both GREEN**, at §2 and again at §4. That gates the whole module — parse, scope, type — except the address space itself, which is the one thing this naga generation cannot judge. The `world.wgsl` banner's own COMPILER FLOOR block already names the web boot as the witness of record for what naga cannot see; this is that gap, one generation wider than the banner describes. **Jean: the per-commit naga gate is inoperative at naga 30 and needs a newer naga or an explicit ruling.** |
| F6 | §4.3 | `pawn_ground_resolve` ("the liberation site") | the ruling text applied at `pawn_ground_resolve` | `pawn_ground_resolve`'s own header carries **no** `FXC` token — its Table H trigger arrives by B:named from the `query_ground_walker` block. The prose the ruling text quotes ("Factored out of each behavior body") is the **AGENT POST-STEP HELPER** banner, verbatim: "Pulled out of each behavior body so FXC compiles the common epilogue once per kernel" | `graduated` — the ruling text is never-adapt (O-6) and was applied **verbatim**, at the site whose prose it quotes, with the symbol cited correctly in the record. O-5/P2: the document's anchors find sites; the tree decides which. |
| F7 | §4.3 | `orb_hsv_to_rgb` | an `FXC` token to RETIRE | `orb_hsv_to_rgb` carries none. The third orb token is on the **tier-accessors** banner ("so FXC handles them without divergence penalty") | `graduated` — same RETIRE treatment, correct anchor, recorded as such. All three orb sites ruled. |
| F8 | §4.4 | `state.hpp` | "expect no token there" | **one** token — not at `GPUSceneConstants` (which is clean, and whose defence is the living L14 storage budget exactly as predicted) but at the **MOSAIC_2 dial**: "the probe's reason for a runtime gate is discharged — FXC compiled the walk" | `graduated` — REWRITE. The living statement is that the walk compiles on the supported floor; which compiler first proved it is history and went to the record. The §4.4 expectation about `GPUSceneConstants` was **confirmed**, and `renderer.hpp` is at zero tokens, so PIVOT_0c left no remainder either. |
| F9 | §4.2 | `docs/7t_program_theory_v3.md` | not pre-ruled | two **present-tense** assertions that the FXC laws are live: "The FXC realization laws … are CAST-SCOPED and **stand untouched**", and "Leaving Dawn would be a recast of L5 **plus retirement of the cast-scoped FXC laws**" | `graduated` — in the §4.2 scan set (`docs/`), unruled by §4.3–4.4, REWRITE by class. This is the defect the probate exists to catch: a live document where a reader could grep and believe a law struck on 2026-08-12 was still standing (P4's own test). The cast-scoping claim is true and survives; only the status claim was wrong, and "leaving Dawn" is now priced at a recast of L5 alone. |
| F10 | §5.1 | the room-family Table D key | a key naming the room family's storage stand | the key says **"stands at 0 of 8"**; the true figure is **5 of 8**. Cause: the emitter's `room_rows` filter selects pipelines whose group layouts contain `roomLayout_`, a name **LOOM_2 retired** (it is `agentsStateLayout_` now), so the filter matches nothing and `max(..., default=0)` prints 0 | `skipped-step` — the key was used **verbatim as printed**, per §5.1's explicit instruction, so the join is exact and warning-free. The emitter was **not** fixed: it is an instrument defect outside §5.2's authorization (which covers teaching the join only), and fixing it would change the key and orphan the ruling in the same commit that wrote it. **Jean: this is a live wrong number in the ledger, and it is the exact failure ATLAS_1revB's comment three lines above it was written to prevent** — that fix was keyed to the pre-LOOM_2 layout name and LOOM_2 silently orphaned it. One-line fix, one re-key, next round. |
| F11 | §6 | Table H total | "expect a drop from 74 as probated boilerplate stops matching" | **68 → 68, no net movement.** One site out, one site in | `graduated` — reported with the mechanism (see Numbers). The drop did not come because RETIRE mostly removed *one trigger from a multi-trigger site*, not whole blocks. The 110→74 rebase worked differently: forty blocks matched `FXC` **and nothing else**. Here only `orb_sample_palette` was FXC-sole, and it did leave the index. `update_sphere` entered it, legitimately: the rewritten kernel-split banner names it as part of the Table E RAW bar it now cites. |
| F12 | §4 (observation) | `slope_passable` (`world.wgsl`) | — | its header defends a shape as "**the L2 posture** for the collision/ground chain (no new runtime branching)" — an active-voice prohibition sourced from a **struck** law, with no strike-through and no record pointer | `skipped-step` — carries no `FXC` token, so it is outside the probate's subject set, and adding sites is improvising on authority-bearing text (O-6). Flagged rather than edited. It is the same defect class as F9, one file over, and it is the natural first item of any successor round. |
| F13 | §4.5 (observation) | `docs/past docs/DAWN_REFERENCE.md` | — | a **second** copy of DAWN_REFERENCE, in `past docs/`, carrying no archival stamp | `skipped-step` — §4.5 names one file and one line ("No other edit"); the rider went on the live `docs/DAWN_REFERENCE.md`. P4 says shelving is not filing, so the shelved twin is unstamped and greppable. Jean's call. |
| F14 | O-4 (standing) | `--check` witnesses S-1 and S-6 | green after each schema-touching commit | **structurally impossible mid-round.** S-1 compares the schema's cardinalities to the *committed* `BINDING_LEDGER.md`, which O-3 defers to §6's single instrument commit; S-6 requires a clean tree **and** `HEAD == pushed tip`, which no mid-round moment satisfies | `graduated` — every other witness was run and held green at every commit. Both cleared at §6: **S-1 PASSES** (86/71/106/29/34/59 both sides) and S-6 clears on this commit's push. Not a defect, but the handoff's O-4 asks for a green that the round's own commit discipline forbids. |
| F15 | environment | local `master` | the campaign line | local `master` was `b491115` — an **unrelated history** with *no merge base* with `origin/master`, 89 files and ~45k lines apart. `origin/master` had been force-updated to `3e18c39`, which is what the session's clone actually held | `graduated` — `git fetch origin master` first (P9, and this is precisely the UMBRA failure that law was written for: the cached ref said one thing, the remote another). Local `master` reset to `origin/master` = `3e18c39`; nothing was lost, since the orphan line was already discarded on the remote. All work is on `master`, per O-1. |

---

## Numbers — §1 baseline vs §6 close

### Cardinalities and the binding surface

| quantity | §1 baseline (`3e18c39`) | §6 close | note |
|---|---|---|---|
| declarations | 87 | **86** | `patch_params` left the table (F2) |
| slots | 72 | **71** | the g2:40 seat retired |
| seats (layout, entry) | 107 | **106** | patchgen state layout 3 entries → 2 |
| layouts | 29 | 29 | — |
| bind groups | 34 | 34 | — |
| pipelines | 59 | 59 | — |
| `RESOURCES` rows | 93 | **91** | two buffers left the program |
| registry banner | "87 declarations over 72 slots" | "86 declarations over 71 slots" | P5: a count doing registry work, corrected and witnessed (0b-1) |

### The immediates lane

| quantity | baseline | close |
|---|---|---|
| `var<immediate>` symbols in the module | 1 (`shadow_slot`) | **2** (`+ patch_params`) |
| pipelines with nonzero `immediate_size` | 13 (shadow family) | **16** (+ three patchgen) |
| patchgen uniform lane | 5 / 12 | **4 / 12** |
| patchgen immediates lane | 0 | **32 / 32** (free 32 of the 64-byte grant) |
| immediates worst row (wallet summary) | 4, shadow family | **32, `generatePatchHeightsPipeline_` C (+2 more)** |
| `NEEDS` r7 `maxImmediateSize` floor | 4 — `sizeof(uint32_t)` | **32 — `sizeof(GPUPatchParams)`** |
| buffers retired | — | **2**: "Patch Params" 32 B + "Patch Params Staging" 225 × 32 B = **7 232 B (7.06 KiB)** |

The MANIFEST landed exactly as §2.3 predicted — three patchgen rows at
uniform 4 (was 5) and immediates `32 / 32`, worst-immediates moved to the
patchgen pipelines, uniform worst row unchanged.

### The wallet

| quantity | baseline | close |
|---|---|---|
| optional features requested at the device | 1, as a literal at the request site | **1, from `FEATURES`** — witness F-1 holds the emitted request to the schema's granted set |
| vaulted rows, priced and unrequested | 0 (prose only) | **5** |
| home of the feature literals | `console.hpp`, hand-carried | `tools/binding_schema.py`, emitted to `src/console/features_wallet.gen.inc` |

### The probate

| quantity | baseline | close |
|---|---|---|
| `FXC` tokens, §4.2 code + `LAWS.md` scan set | **20** (exactly as expected) | **14** |
| — of which `world.wgsl` | 13 | **7** |
| — of which `state.hpp` | 1 | 1 (rewritten; the survivor is the record pointer) |
| — of which `docs/LAWS.md` | 6 | 6 (all KEEP — law text, already struck and dated) |
| Table H sites carrying the `FXC` trigger | **20** | **12** |
| `FXC` as **sole** trigger | 4 | **3** |
| Table H total | **68** (not the 74 the handoff expected — F1) | **68** (F11) |
| — sites out | — | `orb_sample_palette` (FXC was its only trigger) |
| — sites in | — | `update_sphere` (named by the rewritten kernel-split banner, which cites the Table E bar it participates in) |
| record rows written | — | **23**, one per ruled site plus the two "no probate owed" confirmations and the archive's en-masse KEEP |

**Other trigger movements** (all consequences of the probated prose, none
of them edits to the predicate): `time-cost` 7 → **3** and `landed-at`
4 → **0** — the 48 s and 20,227 ms prices moved wholly into
`FXC_LAWS_RECORD.md` as ruled, so `landed-at` joins `hangs` and
`regressed` as **prospective**; `compile-time` 10 → **15** and `budget`
6 → **15**, because the kernel-split rewrite states the Table E bar in
budget-and-barrier vocabulary and B:named attaches it to `agent_state`,
`field_forces`, `floating_entities` and `update_cube`; `witness`
17 → **14**.

One movement worth naming, because it was not a site I edited:
`sample_spot_shadow_pcf` lost its `FXC` trigger. The 4×4 PCF block in
`sample_shadow_pcf` names it forty-seven lines further down ("the SAME
arrangement `sample_spot_shadow_pcf` has always…"), so the retired
"FXC-clean by construction" blessing had been attaching to both symbols
by B:named. One lesson, two anchors; retiring it retired both. Neither
site lost a word of its construction description.

### The wallet's tightest rows — unchanged, as required

| lane | worst row | baseline | close |
|---|---|---|---|
| storage | `updatePlayerAgentPipeline_` C | 5 / 8 | **5 / 8** |
| uniform | `updatePlayerAgentPipeline_` C | 5 / 12 | **5 / 12** |
| sampled | `patchTerrainPipeline_` F | 6 / 16 | 6 / 16 |
| samplers | `updatePlayerAgentPipeline_` C | 3 / 16 | 3 / 16 |
| storage textures | `generatePatchHeightsPipeline_` C | 2 / 4 | 2 / 4 |

### Gates run

| gate | result |
|---|---|
| `binding_gen.py --check` at `3e18c39` (§1, the halt condition) | **GREEN** |
| `binding_gen.py --check` at close | **all witnesses PASS**, S-6 clearing on this commit's push |
| naga, differential shim (F5) | **GREEN** at baseline, after §2, after §4 |
| console gate (GATE_1, `clang++` vs the pinned emdawnwebgpu) | **PASS** after §3 and after §4 |
| L1 — LF only, no BOM | **clean** on every changed file, every commit |
| P3 refuter, before the §2 deletions | **clean** — every remaining mention of the eight deleted symbols is a PROBATE_I epitaph comment; zero live references |
| Table D join | **14 / 14 matched, zero warnings** |
| `world.wgsl` diff at §4 | **comment-only** — no code line moved |

---

## Per-unit status

| unit | status | one line |
|---|---|---|
| §1 `PROBATE_B` | **done** | Baseline green — no halt. Four of five expected numbers confirmed exactly (87 / 72 / FXC 20 / storage 5-of-8); Table H came in at 68, not 74 (F1). |
| §2 `PROBATE_I` | **done, graduated** | All three load-bearing assumptions **confirmed** before editing: `ComputePassEncoder::SetImmediates` exists in the pinned generation (`webgpu_cpp.h:5695`, JS shim `library_webgpu.js:1412`); exactly the three patchgen entry points read `patch_params`; none reaches `shadow_slot`. The graduation rule did **not** fire — `patchStagingBuffer_` had no consumer beyond this path, so both buffers retired. Graduations: F2, F3, F4. |
| §3 `PROBATE_F` | **done** | Six rows, one grant, five vaulted. New emitter pair mirroring `NEEDS`→`limits_floor.gen.inc`, new witness **F-1**, request site reads the wallet, boot line added. Type-checked by the console gate. |
| §4 `PROBATE_X` | **done, graduated** | 20 sites ruled — 7 KEEP, 6 RETIRE, 5 REWRITE, 1 rider, plus two "no probate owed" confirmations and the archive's en-masse KEEP. No lesson lost: every rewritten block is verbatim in the record first. Graduations: F6, F7, F8, F9. |
| §5 `PROBATE_D` | **done** | 14 rulings in the schema, joined into Table D by exact key with a loud-orphan warning path; L24, L25, L26 appended in-form, contiguous with the numbered laws. F10 flagged, not fixed. |
| §6 `PROBATE_R` | **done** | **No control re-key was needed** — §6's advance authorization went unused. W4-2 still finds all five defended sites with non-empty trigger sets after the probate; the two agent kernels simply trade `FXC, landed-at, time-cost` for `budget`. `FXC` stays in W4-1 at 12 live sites. Ledger regenerated; S-1 green. |

**Held out of this round, unchanged and on the record:** TEX_C0, every A2
spend (priced, not spent — now as data), the `agent_state` +
`field_forces` merge (named payment, unspent — now L24), and the SOAK
watchlist.

---

## Jean's gate checklist

1. Pull master; the log shows five to seven `PROBATE_*` commits.
2. glaw1 build; boot the web twin on a pinned seed.
3. Console reads: `maxImmediateSize=64/32` in granted-vs-floor; the
   feature-wallet line with `granted timestamp-query; vaulted 5`;
   `[GPU Budget] buffers` down by roughly 7 KiB; all pipelines compile;
   zero validation errors.
4. The terrain on the pinned seed is identical by eye — heights, cell
   colors, GoL placement. This is PROBATE_I's visual gate.
5. Read §Flags. Anything there is queued judgment, not failure.

### What to look at, from this side of the gate

The report owes you the "could BREAK" column the checklist does not
carry (P10, and the CURTAIN_1 candidate beside it):

- **Step 3, the budget line.** The number to expect is **7 232 B**, not
  a round 7 KiB — 32 B for "Patch Params" and 7 200 B for the 225-slot
  staging ladder.
- **Step 4 is the one that matters, and here is the artifact to name.**
  If the immediates handoff is wrong, the failure is not subtle
  corruption: every patch would generate from **one patch's parameters**
  — most likely the last batch member's, or zeroes. What you would say
  out loud is *"the whole terrain is the same hill repeated"*, or
  *"everything outside the first patch went flat"*, with the cell colors
  and GoL placement wrong in the same tiling pattern. Anything subtler
  than that is not this edit.
- **A second, quieter failure mode for step 4:** immediate data is
  **pass state**, so it is set twice per patch — once in the heights
  pass, once in the gradients+cells pass. If the second call were
  missing, heights would be right and *gradients and cell colors would
  lag one patch behind*: correct silhouettes, shading and colors
  belonging to a neighbour. Both `SetImmediates` calls are in the tree;
  this row exists so the artifact has a name if it ever appears.
- **Step 3, the pipelines.** If naga's blind spot bites (F5), it bites
  at pipeline creation as a validation error naming `immediateSize` or
  the patchgen layout — not as a pixel. Zero validation errors is the
  whole of that check.
- **What this round could NOT break:** §3, §4, §5 and §6 touch no frame
  and no binding surface. §4 is comment-only in `world.wgsl` (verified
  by diff), §5 is schema data plus `LAWS.md` prose, §6 is a regenerated
  artifact. §3 changes one request site, and the shape it can break is
  the *device request*, visible at step 3 as a wallet line that does not
  say `granted timestamp-query; vaulted 5`.

### And three things that want a ruling rather than a look

- **F5 — the naga gate is inoperative.** naga 30 cannot parse this
  program, at baseline or now. O-4's per-commit gate has been running on
  a shim since `patch_params` moved; it needs a newer naga or an explicit
  ruling about what the per-commit WGSL gate now is.
- **F10 — the ledger prints a wrong number.** Table D's room-family row
  says the room stands at **0 of 8**; it stands at **5 of 8**. One-line
  filter fix, one re-key of the ruling that now quotes it.
- **F1 — Table H's preamble is stale.** It calls 74 the ruling of
  record; the index has held 68 since before this round opened.
