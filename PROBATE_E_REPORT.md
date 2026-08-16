# PROBATE_E — the errata, and the close

Five defects settled, six commits, no halt. The campaign closes.

---

## Header

| field | value |
|---|---|
| HEAD before | `187b646` — PROBATE_R |
| HEAD after | this commit (`PROBATE_E6`) |
| branch | `master`, per O-1 |
| baseline gate (§E0) | `binding_gen.py --check` **GREEN** at `187b646`, working tree clean — no halt |
| §E0 census | Table H **68**; Table D **14** keys, all joined; room row printed **"stands at 0 of 8"** — the lying number, as expected |

| # | commit | subject |
|---|---|---|
| 1 | `70a16a7` | PROBATE_E1: room_rows keyed by identity, the room sentence computed, Table D joined on stable ids — key and join move together (O-8) |
| 2 | `25df924` | PROBATE_E2: Table H preamble computed; the index history moves to the record — no hand number survives in a generated file |
| 3 | `daca95a` | PROBATE_E3: the shim gate is the gate — pinned as tools/wgsl_gate.py, blind spot in the banner, Firefox marked PENDING until its boot testifies |
| 4 | `4f4613e` | PROBATE_E4: L2 speaks only in the past tense — active citations of the struck law ruled, lessons to the record |
| 5 | `e7f3242` | PROBATE_E5: the shelved DAWN_REFERENCE carries its archive stamp |
| 6 | this one | PROBATE_E6: instrument close — ledger regenerated over corrected instruments; L27 on the books; the campaign closes |

---

## Flags

| # | unit | site (symbol) | expected | found | action |
|---|---|---|---|---|---|
| EF1 | §E1 | my own `E1-identity` witness | a witness that fails the run when an entry-point name stops resolving | **it fired into nothing.** Placed inside `emit()`, it recorded into `w` *after* `w.report()` had already run and the gate had already passed — so a deliberately typo'd entry-point name produced **no message and exit 0**, indistinguishable from a pass | `graduated` — caught by negative control before the commit, not after. `room_family_census()` is now module-scope and called in `main()` **before** the gate. Re-controlled: typo → `[FAIL] E1-identity … names entry point(s) NO pipeline carries: update_cube_TYPO`, `STOP`, **exit 1**. This is P6's exact complaint and ECONOMY_1 E1's inert arm, reproduced inside the fix for a different silent-zero defect — worth the flag on its own. |
| EF2 | §E1 | the A2 id for the `fc_vp` row | `a2:bind::g2::fc_vp@cullStateLayout_` by the naive reading of the spec | **`a2:bind::g2::vp_data@cullStateLayout_`** — `fc_vp` is an ALIAS; the seat's binding constant is `bind::g2::vp_data` at g2:240 | none needed — the spec says `{binding_const}@{layout_member}`, and the binding constant is the identity. Recorded because the printed prose and the id name different symbols for that one row, which looks like an error and is not. The positional re-key was verified against **layout identity** rather than symbol for exactly this reason. |
| EF3 | §E3.3(a) | a newer naga | a newer naga-cli accepting `immediate_address_space` | **`naga-cli 30.0.0` is the newest published version** — `cargo search` returns 30.0.0 as latest, which is what is installed. There is no newer naga to upgrade to | `graduated` — the probe's verdict is *nothing to buy yet*. The gate cannot be fixed by upgrading today; `tools/wgsl_gate.py` is the gate until wgpu ships the extension. Recorded as a fact, gating nothing, per §E3.3. |
| EF4 | §E3.3(b) | a standalone `tint` | a buildable standalone `tint` target in the pinned emdawnwebgpu tree | **none exists.** The pinned package is 6 headers, 5 JS files, 1 cpp, 1 port script and 2 markdown files — no CMake, no GN, no build system, no Tint source. Its only Tint mentions are a reference to a Dawn source path that is not vendored, and PINNED.md's own note that what matters is *the browser's* Tint | `graduated` — no build attempted (there was no target to build). A Tint-backed gate would need a separate Dawn checkout, which SUNSET_0 archived. Prices the next gate at "resurrect a Dawn checkout, or wait for naga" — Jean's call, not this round's. |
| EF5 | §E4.2 | `update_other_agents` — the FIELD_2/FIELD_B presence-law banner | a clean KEEP or REWRITE | **genuinely ambiguous**: `(Durable home per the L2-banner precedent: the law lives where it binds.)` cites L2's *banner placement* as a precedent for where documentation lives — a method precedent, not a live constraint on the code | `skipped-step` — flagged rather than adapted, exactly as §E4.2 directs. A method precedent plausibly survives the striking of the law that demonstrated it; whether a struck law may go on being cited that way is Jean's ruling. Record row written. |
| EF6 | §E4.2 | `docs/audit/SALON_1.md`, `SALON_1_E_C_REPORT.md` | history-tense report prose | nine hits treating `L2.4` as a **live ceiling**, at a value wrong twice over: the clause survived L2's striking and lives in **L14**, and the core default is **8**, not the **10** these cite | `skipped-step` — KEEP as dated reports (history by nature, same class as PROBATE's en-masse archive ruling), but they sit unstamped in `docs/audit/`, greppable and believable. This is F13's defect one folder over and P4's own test. §E4 authorizes no stamp; flagged. |
| EF7 | §E4.3 | Table H | "expect Table H to grow slightly — the new record pointers carry the `law-ref` trigger" | **no growth: 68 → 68**, no site in or out | `graduated` — reported with the mechanism. The two rewritten sites were **already indexed** (`slope_passable` carried `compile-time, law-ref`; the occupier wire sits inside `update_player_agent`'s body), so another `law-ref` enriches rather than adds. Three sites did gain triggers from the new ruling text — `slope_passable`, `pawn_ground_resolve`, `query_ground_walker_pair` each picked up `measured` and `witness` from "until a measurement asks; a reshape's witness is the per-browser boot". Same mechanism as PROBATE's F11, one round on. |
| EF8 | §E4 (observation) | `SEAM[gallery:L2]`, `LANTERN_CENSUS.md §L2`, the theory stack's `L2 ENTITIES` | — | three `\bL2\b` hits that are **not law citations** at all — a seam label, a section reference, and a layer name | none needed; recorded in the appendix so the next `L2` sweep does not re-litigate them. |
| EF9 | O-4 (standing) | S-6 | green per commit | red at every intermediate commit — it requires a clean tree **and** `HEAD == pushed tip` | `graduated` — exactly what amended O-4 anticipates. S-1 stayed **green throughout this round**, unlike the main round: nothing here changed a schema cardinality. S-6 clears on this commit's push. |

**Not flagged, because O-8 worked.** §E1 opened a fourteen-way orphan
warning and closed it inside one commit. Under the old discipline that
would have been a red spanning two commits and an entry in this table;
under L27 it is a thing that never happened.

---

## Numbers — §E0 baseline vs §E6 close

### The lying number (F10), both bodies

| where | baseline | close |
|---|---|---|
| Table D, room row | `storage stage stands at **0 of 8**` | `storage lane stands at **5 of 8** with **0** demotable seats` |
| Finding 4, same claim, **separate template string** | `stands at **0 of 8** storage` | `stands at **5 of 8** storage with **0** demotable seats` |
| derivation | `any("roomLayout_" in g …)` — a name LOOM_2 retired; matched nothing; `max(…, default=0)` | four entry points by name, resolved to pipelines |
| guard against recurrence | a comment keyed to the retired name — the defect's second body | witness **`E1-identity`**, negative-controlled |

The room family resolves to **4 (pipeline, stage) rows over 4 layouts,
from all 4 entry points by name**. `demotable = 0` is measured, not
asserted: no seat of this family carries an A2 CANDIDATE verdict, which
is why the wall's ruling says none is for sale.

### The join

| quantity | baseline | close |
|---|---|---|
| Table D rows | 14 | 14 |
| rows with judgment cells filled | 14 | **14** |
| join key | the printed prose | **stable id** — `a2:{binding_const}@{layout_member}`, or `room-wall` |
| warnings | 0 | **0** |
| ruling **values** | — | **byte-identical and in order**, verified by capturing them before the re-key and comparing after |

The room row's prose was rewritten *in the same commit* as the re-key —
which is the point of the ids: under the old key that rewrite would have
orphaned its own ruling.

### Table H

| quantity | baseline | close |
|---|---|---|
| rows | 68 | **68** |
| preamble | `**REBASED 110 → 74 … and 74 is the ruling of record.**` — a hand number, stale by six sites across two campaigns | `68 sites at this run.` — computed, with the history in `audit/FXC_LAWS_RECORD.md §index-history` |
| sites in / out from §E4 | — | none / none |
| trigger sets enriched | — | 3 (`slope_passable`, `pawn_ground_resolve`, `query_ground_walker_pair`, each `+measured, +witness`) |

### The gate

| quantity | baseline | close |
|---|---|---|
| per-commit WGSL gate | `naga world.wgsl` — **failing open since DOMESDAY_2 F3-a**, and unrunnable in fact | `tools/wgsl_gate.py` — pinned transform, no options, in the tree |
| gate on the real module | naga: `unknown language extension immediate_address_space` | **PASS** |
| negative control | — | broken module → `FAIL`, **exit 1** |
| `world.wgsl` banner, Firefox | listed as a supported compiler | **PENDING**, with the reason and the witness that would settle it |
| banner, blind spot | named for pipeline-layout conformance and minBindingSize | **+ the immediate address space**, one generation wider |
| probe (a) — newer naga | — | **none exists**; 30.0.0 is latest published |
| probe (b) — standalone tint | — | **not in the pinned tree**; no build system vendored |

Two immediates are shimmed by the gate: `shadow_slot → @group(0)
@binding(900)`, `patch_params → @group(0) @binding(901)`.

### Struck-law citations

| quantity | baseline | close |
|---|---|---|
| struck laws (recon) | — | exactly **L2** |
| active-voice citations in the code scan set | **2** | **0** |
| `L2` mentions remaining in `world.wgsl` | 3 | 3 — two now past-tense with record pointers, one flagged ambiguous (EF5) |
| false positives recorded so the next sweep skips them | — | 3 (EF8) |
| record rows added this round | — | **13** across three appendices (E3 gate ruling, E4 citations, §index-history) |

### Gates run

| gate | result |
|---|---|
| `binding_gen.py --check` at `187b646` (the halt condition) | **GREEN** |
| `binding_gen.py --check` at close | all witnesses PASS, S-6 clearing on push |
| `tools/wgsl_gate.py` | **PASS** after §E3 and after §E4; negative-controlled (exit 1 on a broken module) |
| `binding_ledger.py` | zero warnings, zero failures; `E1-identity` **PASS**, negative-controlled (exit 1 on an unresolvable name) |
| L1 — LF only, no BOM | clean on every changed file, every commit |
| ruling-value preservation | **byte-identical**, verified across the re-key |

---

## Per-unit status

| unit | status | one line |
|---|---|---|
| §E0 | **done** | Baseline green, tree clean. Census confirmed all three expected values, including the lying `0 of 8`. |
| §E1 | **done** | Both bodies of F10 corrected from one identity-derived source; sentence computed; 14 rulings re-keyed to ids with values verbatim; O-8 honored in one commit. One self-inflicted defect caught by negative control (EF1). |
| §E2 | **done** | Preamble computed; `§index-history` in the record carries all three rebases with causes. |
| §E3 | **done** | Shim materialized as `tools/wgsl_gate.py`, negative-controlled; banner amended with old clauses to the record; both probes returned verdicts (EF3, EF4). |
| §E4 | **done** | Struck set confirmed as exactly L2; 2 REWRITEs, 5 KEEPs, 2 flagged (EF5, EF6), 3 false positives recorded. Table H actual reported against the expectation (EF7). |
| §E5 | **done** | Default taken — the stamp, not the deletion. |
| §E6 | **done** | L27 on the books, contiguous with the numbered laws; ledger regenerated over the corrected instruments. |

---

## Jean's checklist

1. **Pull.** Six `PROBATE_E*` commits on `master`.
2. **Boot once on the pinned seed.** Console still clean: no new lines
   this round, no validation errors, all pipelines compile. Nothing here
   touches the frame — §E1/E2 are emitters, §E3–E5 are comments and a new
   tool, §E6 is a regenerated artifact. The only in-tree code files this
   round edits are `world.wgsl` comments.
3. **Then the two witnesses only you can give.**
   - **The step-4 terrain word, for PROBATE_I.** Still outstanding from
     the main round, and still the one gate no instrument here can
     stand in for. What you are looking for, in the words you would use
     if it were wrong: *"the whole terrain is the same hill repeated"*,
     or *"everything outside the first patch went flat"* — and the
     quieter variant, *"the shading and the cell colors belong to the
     patch next door"*. Anything subtler than those is not that edit.
   - **One Firefox load of everexpandingboard.com.** It boots, or it
     errors naming `immediate_address_space`. Either answer settles the
     banner: boots → Firefox comes off PENDING and back onto the
     supported line; errors → PENDING becomes a stated exclusion and
     the audience floor is the Tint trio on the record, not by default.
4. **Read §Flags.** EF5 and EF6 are yours to rule; EF1–EF4 and EF7–EF9
   are settled and recorded.

---

*Standing after this round, on the record and untouched: TEX_C0, the A2
price list (now id-keyed), the agent merge (L24's named payment), the
SOAK watchlist, and the gate-of-the-future question — which EF3 and EF4
now price at "a resurrected Dawn checkout, or wait for wgpu".*
