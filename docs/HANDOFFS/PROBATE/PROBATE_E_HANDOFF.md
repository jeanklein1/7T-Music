# PROBATE_E — the errata, and the close

One small round. PROBATE's report surfaced three live defects in the
instruments and two loose ends in the tree; this round settles all five
and the campaign closes. Same constitution as the main round — the
standing orders below restate it with the two amendments the flags
earned.

Authored by Claude against the PROBATE_R artifacts (report, regenerated
ledger, record, both consoles). Rulings referenced here are issued; CC
executes, flags, finishes.

---

## §0 STANDING ORDERS (as PROBATE §0, with two amendments)

O-1..O-3, O-5..O-7 stand exactly as written in PROBATE_HANDOFF.md §0.

**O-4 (amended, per F14):** naga runs through the shim gate (§E3) after
any WGSL edit; `binding_gen.py --check` after any schema-touching
commit, **with S-1 and S-6 understood to clear only at the instrument
close** — a mid-round red on those two is the commit discipline working,
not a failure. LF-only, no BOM. No builds, no boots — Jean witnesses.

**O-8 (new — the join-coherence law, from F3 and §E1):** when a schema
datum and the emitter that joins on it must move together, they move in
ONE commit; splitting a key from its join manufactures a red between
commits. This is the only sanctioned form of instrument-and-subject
sharing a commit, and the commit message must name it.

**Halt condition:** `--check` red at HEAD before any edit. Everything
else is a flag.

---

## §E0 — baseline (no edits)

`git rev-parse HEAD`; `binding_gen.py --check` green; record Table H
total (expect 68), the room row's printed text, and the Table D key
strings as they stand. Report file for this round:
`PROBATE_E_REPORT.md`, same flag form.

---

## §E1 — the lying number (F10): identity, computed prose, stable ids

One commit, under O-8, touching `binding_ledger.py` +
`binding_schema.py` together.

1. **`room_rows` by identity.** Derive the room family from the four
   entry points — `update_player_agent`, `update_other_agents`,
   `update_sphere`, `update_cube` — resolving their pipelines and
   reading the storage lane from the same data Table B prints. Never a
   layout-name literal. The ATLAS_1revB guard comment beside it —
   itself keyed to the pre-LOOM_2 name, the defect's second body —
   is rewritten in identity terms in the same commit.
2. **Computed sentence.** The room row's queued-item prose becomes,
   verbatim template: `The room family's storage lane stands at {used}
   of 8 with {demotable} demotable seats ({demotable} = its seats
   holding an A2 CANDIDATE verdict). Any new storage binding reachable
   from update_player_agent / update_other_agents / update_sphere /
   update_cube needs a demotion to pay for it, and none is for sale.`
   Expected print at close: `5 of 8` and `0`. Finding 4's prose in the
   ledger renders from the same corrected source — recon whether it is
   the same template string; if separate, correct it identically.
3. **Stable ids for the Table D join.** Every queued item the emitter
   builds carries an `id`: A2 rows derive
   `a2:{binding_const}@{layout_member}` (e.g.
   `a2:bind::g2::render_agents@sceneStateLayout_`); the room row is
   `room-wall`. `DEMAND_RULINGS` re-keys to these ids — **the fourteen
   ruling VALUES move verbatim, never-adapt; only the keys change.**
   The join runs on id; printed prose is free to change forever after.
   The loud-orphan warning path stays, now keyed on id.

Witness E1-a: regenerating the ledger prints `5 of 8`, `0 demotable`,
join 14/14, zero warnings. (Regeneration itself is committed at §E6.)

**Commit:** `PROBATE_E1: room_rows keyed by identity, the room sentence computed, Table D joined on stable ids — key and join move together (O-8)`

---

## §E2 — the stale preamble (F1): no hand number in a generated file

1. In `binding_ledger.py`, the Table H preamble's narrative paragraph
   ("REBASED 110 → 74 … ruling of record") is replaced by a computed
   line — verbatim template: `{n} sites at this run. The index's
   history — every rebase, with causes — lives in
   audit/FXC_LAWS_RECORD.md §index-history; a regenerated artifact
   carries no hand-carried number (P5, one home).` The predicate table,
   the prospective-trigger doctrine and the attachment rules all stay.
2. Append to `audit/FXC_LAWS_RECORD.md`:

```
## §index-history — Table H, every rebase
| when | movement | cause |
|---|---|---|
| 2026-08-12 | 110 → 74 | PIVOT_0c despelled the ROSTER-GATE boilerplate: one lesson counted forty times |
| between PIVOT_0c and 3e18c39 | 74 → 68 | six sites left the index across two campaigns without the preamble following — the hand number this section exists to retire |
| PROBATE (2026-08-16) | 68 → 68 | orb_sample_palette out (FXC was its sole trigger); update_sphere in (named by the rewritten kernel-split banner citing the Table E bar) |
```

**Commit:** `PROBATE_E2: Table H preamble computed; the index history moves to the record — no hand number survives in a generated file`

---

## §E3 — the gate ruling (F5): the shim is promoted, the banner tells the truth

1. **Materialize the gate.** Recon what §PROBATE's shim was (script or
   ad-hoc). Pin it as `tools/wgsl_gate.py`: the exact transform —
   comment out `requires immediate_address_space;`; rewrite each
   `var<immediate> N: T;` to `@group(0) @binding(90x) var<uniform>
   N: T;`, x enumerated from 0 — then naga on the transformed module,
   green or fail. Deterministic, no options. This is the per-commit
   WGSL gate of record.
2. **Banner amendment**, `world.wgsl` COMPILER FLOOR block —
   never-adapt text. The clause naming naga as the per-commit gate
   gains: `naga gates the module THROUGH THE IMMEDIATE SHIM
   (tools/wgsl_gate.py; the transform is the gate's pinned half). The
   immediate address space itself is witnessed by boots alone — one
   generation wider than the blind spot this block already names.` The
   supported-compilers line amends `naga (Firefox)` to: `Firefox:
   PENDING — naga 30 lacks immediate_address_space; the floor of record
   is the Tint trio until a Firefox boot witnesses otherwise (Jean's
   witness, queued).` Old clauses verbatim to the record appendix, per
   the probate template.
3. **Two probes, time-boxed, flag-not-block** (results are report
   facts, not gates): (a) does a newer naga-cli accept the directive —
   one registry/install attempt, record version and verdict; (b) does
   the pinned emdawnwebgpu tree carry a buildable standalone `tint`
   target — one target listing, build only if trivial, record the
   verdict. These decide nothing this round; they price the next gate.

**Commit:** `PROBATE_E3: the shim gate is the gate — pinned as tools/wgsl_gate.py, blind spot in the banner, Firefox marked PENDING until its boot testifies`

---

## §E4 — struck-law citations in active voice (F12, generalized)

Subject set: active-voice citations of struck law numbers, outside
`docs/LAWS.md` and the record. Recon the record for the struck set —
expect exactly `L2`. Then `grep -rn "\bL2\b"` across the §4.2 scan set
plus `docs/`.

1. **`slope_passable`** (the known site): old header block verbatim to
   the record; in-tree REWRITE, never-adapt text: `No living law
   governs branching in this chain — L2 was struck (PIVOT_0;
   audit/FXC_LAWS_RECORD.md). The shape stands until a measurement
   asks; a reshape's witness is the per-browser boot.`
2. Every other hit: KEEP if history-tense or struck-text; REWRITE by
   the same class if active-voice; flag anything ambiguous rather than
   adapting. Record rows for every ruling, as in the main round.
3. Expect Table H to grow slightly at §E6 — the new record pointers
   carry the `law-ref` trigger. Report actuals; growth here is the
   index doing its job.

**Commit:** `PROBATE_E4: L2 speaks only in the past tense — active citations of the struck law ruled, lessons to the record`

---

## §E5 — the shelf twin (F13)

Stamp `docs/past docs/DAWN_REFERENCE.md` under its title: `SHELVED
COPY — documents the ARCHIVED native build (native-sunset, PIVOT_0).
The live copy is docs/DAWN_REFERENCE.md; nothing here binds the web
twin.` No other edit. (Jean may prefer deletion — one home — in which
case this unit is a one-file `git rm`; default is the stamp, which
loses nothing.)

**Commit:** `PROBATE_E5: the shelved DAWN_REFERENCE carries its archive stamp`

---

## §E6 — instrument close

1. `LAWS.md`: append **L27**, in-form: *A schema datum and the emitter
   that joins on it move in one commit; splitting a key from its join
   manufactures a red between commits (the join-coherence law; F3 and
   E1 are its precedents).*
2. Regenerate `BINDING_LEDGER.md`; `--check` all green; S-6 clears on
   push.
3. `PROBATE_E_REPORT.md`: flags, the numbers (room row now `5 of 8 / 0
   demotable`; Table H computed count and any E4 growth, named; probe
   verdicts from §E3.3), per-unit status, and Jean's checklist:
   pull; boot once on the pinned seed; console still clean; **then the
   two witnesses only he can give** — the step-4 terrain word for
   PROBATE_I, and one Firefox load of everexpandingboard.com: boots, or
   errors naming the directive. Either answer settles the banner.

**Commit:** `PROBATE_E6: instrument close — ledger regenerated over corrected instruments; L27 on the books; the campaign closes`

---

*Standing after this round, on the record and untouched: TEX_C0, the
A2 price list (now id-keyed), the agent merge (L24's named payment),
the SOAK watchlist, and the gate-of-the-future question that §E3.3's
probe verdicts will price.*
