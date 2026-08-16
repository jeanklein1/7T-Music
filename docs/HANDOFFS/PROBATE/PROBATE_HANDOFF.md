# PROBATE — the closing handoff

One round, six units, one halt condition. PROBATE executes the estate of
the dead compiler and puts a statute on everything the living device
grants: the FXC scars get rulings, the feature vault gets a wallet, the
immediates lane gets its second spend, and every storage escape gets a
price tag instead of a purchase. When this round lands, the campaign
closes.

Authored by Claude against the 2026-08-16 upload set (BINDING_LEDGER at
commit `ba327a7`, MANIFEST, binding_schema.py, world.wgsl, Pixel console).
The tree outranks this document: where they disagree, FLAG and follow the
protocol below.

---

## §0 STANDING ORDERS FOR THIS ROUND

**O-1. Work on master HEAD. No branch.** Every unit is recoverable by
commit; that is the whole safety story and it is enough.

**O-2. FLAG, don't halt.** When the tree disagrees with an expectation in
this document, write a flag into `PROBATE_REPORT.md` §Flags with five
fields — `unit / site (symbol) / expected / found / action` — where
action is one of `skipped-step`, `skipped-unit`, `graduated` (did the
subset that still holds), and continue to the next independent step. Do
not stop to ask. Do not improvise on anything authority-bearing (O-6).
A unit whose load-bearing assumption fails is skipped whole, its commit
unmade, its flag written. **The only full halt in this round is §1's
baseline failing.**

**O-3. One commit per unit; instrument and subject never share one.**
Commit messages are given verbatim per unit. Generated mirrors emitted by
`binding_gen.py --write` ride the subject commit that changed the schema
(both rooms, same commit). `BINDING_LEDGER.md` regeneration and any
defended-site-control re-key are the round's single instrument commit
(§6) — authorized in §6, and only there.

**O-4. Gates per commit:** naga on `world.wgsl` after any WGSL edit;
`python tools/binding_gen.py --check` after any schema-touching commit;
LF-only, no BOM, everywhere (L1). Do not build or boot — Jean holds
glaw1 and the boot witness; the report tells him what to look for.

**O-5. Recon before every edit.** Each unit names its recon steps. Read
the site, confirm the expected shape, then edit. Cite symbols in every
comment and flag; the line numbers in this document are anchors for
*finding* sites in the 2026-08-16 snapshot, never for editing.

**O-6. CC may adapt / may never adapt.** May adapt: dict field ordering,
include placement, whitespace, the exact wording of comment prose *within
a given template*. May never adapt: binding numbers, struct layouts,
schema relation semantics, ruling text, law text, witness definitions,
commit boundaries. A blocked never-adapt item is a flag, not a judgment
call.

**O-7. Maximum load.** Finish every unit that can be finished. Partial
completion with flags beats a clean stop every time.

---

## §1 PROBATE_B — baseline census (no edits)

1. `git rev-parse HEAD` — record in the report header.
2. `python tools/binding_gen.py --check` — must be green.
3. `python tools/binding_ledger.py` (regenerate to a scratch path or
   however the tool runs read-only; do NOT commit output here) — record:
   declaration count (expect 87), slot count (expect 72), Table H total
   (expect 74), `FXC` trigger site count (expect 20), tightest storage
   row (expect Update Player Agent C at 5/8).
4. Copy Table D's queued-item key strings verbatim into a scratch note —
   §5 consumes them exactly as printed.

**If step 2 is red at HEAD: full halt.** Write the failure into
`PROBATE_REPORT.md`, commit nothing else, end the round. Tree ≠ schema
means every downstream unit builds on sand. Any other deviation in §1 is
a flag with recorded actuals, and the round proceeds using the actuals.

---

## §2 PROBATE_I — patch_params rides the immediates lane

**What.** `PatchParams` (32 B, one writer, per-dispatch cadence) moves
from a g2:40 uniform seat to `var<immediate>`. The seat, the params
buffer, and the 225-slot staging ladder leave the program; the storm path
sets 32 bytes on the pass encoder instead of copying them between
buffers. This is the lane's second spend; `shadow_slot` (DOMESDAY_1 B6)
was the first and is the precedent to mirror at every C++ site.

**Load-bearing assumptions** (any one failing → flag; see graduation
rule at the end of this unit):
- The pinned emdawnwebgpu generation exposes the immediates call on
  **compute** pass encoders, not only render. The shadow site
  (`render_passes.hpp`, the `SetImmediates` the world.wgsl §7.0 comment
  names) proves render; recon the generated headers for the compute-pass
  equivalent before editing. If absent: skip the whole unit, flag
  `NEEDS-generation-check`.
- `patch_params` is read by exactly the three patchgen entry points and
  no other (ledger says vis C, patchgen layouts only).
- No patchgen entry point statically reaches `shadow_slot` (one
  immediate variable per entry point, WGSL §14.3). Expect trivially true.

### 2.1 WGSL (world.wgsl)

FIND — expect **exactly 1** match:

```
@group(2) @binding(40) var<uniform> patch_params: PatchParams;
```

REPLACE with:

```
// PROBATE_I — PatchParams rides the immediates lane (the lane's second
// spend; shadow_slot, DOMESDAY_1 B6, was the first). One struct, one
// dispatch cadence, 32 of the 64-byte grant. The g2:40 seat, the params
// buffer and the 225-slot staging ladder left the program with it; the
// storm path sets these bytes on the pass encoder instead of copying
// them between buffers. Every read below is untouched — the address
// space moved, the name did not.
var<immediate> patch_params: PatchParams;
```

That is the entire WGSL edit. **Zero body edits**: all reads keep the
name. Witnesses, run immediately:
- `grep -c "var<immediate> patch_params" world.wgsl` == 1
- `grep -c "@binding(40) var<uniform> patch_params" world.wgsl` == 0
- total `grep -c "patch_params" world.wgsl` == 14 (unchanged — the name
  survived the move)
- naga green.

The `.a`-reserve comment (LOOM ruling) anchors to
`patch_heightfield_array_write`, not to this declaration — confirm it is
untouched.

### 2.2 C++ (recon-first; expected shapes below)

- **state.hpp**: delete `patchParamsBuffer_` and `patchStagingBuffer_`
  (members, creation sites). `GPUPatchParams` the struct **stays** — it
  is now the value type handed to the immediates call.
  `patchgenStateLayout_` drops its entry 0 (the g2:40 uniform seat); the
  layout's `std::array` size and `entryCount` shrink with it (witness
  0a-1/0a-1b discipline). The matching bind group drops the entry.
  Pipeline-layout creation for the three patchgen pipelines declares the
  immediate size — **mirror, verbatim in style, how the DOMESDAY_1 B6
  site declares the shadow pipelines' 4 bytes**, substituting
  `sizeof(GPUPatchParams)`.
- **Dispatch site** (recon: `surface/patch_system.hpp` and/or
  `renderer.hpp` — follow `dispatch_generate_patch_heights`): expected
  current shape is *either* per-patch `CopyBufferToBuffer(staging[i] →
  params)` interleaved with passes, *or* a write-then-pass sequence.
  After: inside the compute pass, per patch, set immediates
  (offset 0, `&params`, `sizeof(GPUPatchParams)`) then issue the
  height/gradient (and, on its own cadence, cells) dispatches. The
  per-patch copy and any per-patch pass churn that existed only to
  refresh the uniform are deleted. Preserve dispatch order and cadence
  exactly — heights before gradients per patch (Table E:
  `patch_height_scratch` bars the other order), cells on demand as
  today.
- **Graduation rule**: if recon finds `patchStagingBuffer_` has a
  consumer beyond this path, keep that buffer, still retire
  `patchParamsBuffer_` + the seat + the copy, and flag the survivor with
  its consumer named.

### 2.3 Schema (binding_schema.py) + mirrors

- `DECLS`: transform the `patch_params` row to the **shape of the
  `shadow_slot` row** (the immediate precedent already in the table).
- `SEATS` / `REGISTRY`: retire the g2:40 seat and `bind::g2::patch_params`.
- `LAYOUTS`: patchgen state layout loses the entry.
- `PIPELINES`: the three patchgen rows carry
  `immediate_size = sizeof(GPUPatchParams)` spelled exactly as the
  shadow rows spell theirs (A10 shape).
- `RESOURCES`: delete `patchParamsBuffer_` and `patchStagingBuffer_`
  rows (or only the former, under the graduation rule).
- `NEEDS` r7: `maxImmediateSize` floor `4 → 32`, source
  `'sizeof(GPUPatchParams)'`, note: `'the patchgen params (PROBATE_I) —
  the lane statute grants 64; the program stands on 32; shadow_slot
  rides within it'`.
- `python tools/binding_gen.py --write` then `--check` — green. The
  regenerated `binding_registry.hpp`, `binding_surface.gen.inc`,
  `MANIFEST.md`, `limits_floor.gen.inc` ride this commit.

**Expected MANIFEST after regen** (verify, flag deviations): three
patchgen rows read uniform 4 (was 5), immediates 32/32 free 32 → i.e.
`32 / 32`; wallet summary immediates worst becomes 32 at the patchgen
pipelines; uniform worst row unchanged (room family 5/12).

**Commit (subject):**
`PROBATE_I: patch_params rides the immediates lane — the g2:40 seat, the params buffer and the staging ladder leave the storm path`

---

## §3 PROBATE_F — the feature wallet

**What.** The Pixel offers nineteen optional features; the program
requests one, and today that discipline lives in prose. It becomes a
schema table with a statute: nothing is requested at the device except
through this table, no row is granted without a fallback ruling, and the
literals live here and nowhere else — exactly the `NEEDS` pattern, one
shelf over.

### 3.1 Schema addition (verbatim data; place adjacent to `NEEDS`)

```python
# ═══ THE FEATURE WALLET (PROBATE_F) ═════════════════════════════════
# No optional feature is requested at the device except through
# status='granted' here, and no row is granted without a fallback
# ruling. binding_gen.py --write emits the request list and the wallet
# print from these rows — the literals live here and nowhere else.
# 'vaulted' = offered by the floor device, deliberately unrequested,
# priced for a future campaign. A grant is a schema edit plus Jean's
# gate, never an ad-hoc request-site edit.
FEATURES = {
    'timestamp-query': {'status': 'granted', 'consumer': 'the METER (per-pass GPU timing)', 'fallback': 'meters go silent; the frame loop is untouched', 'coverage': 'near-universal'},
    'shader-f16': {'status': 'vaulted', 'consumer': 'none yet — candidate: terrain fragment ALU', 'fallback': 'the f32 shader exactly as it stands', 'coverage': 'Valhall yes (Pixel console 2026-08-16); per-browser check precedes any grant'},
    'texture-compression-astc': {'status': 'vaulted', 'consumer': 'none yet — candidate: authored painting uploads (TEX_C0, parked)', 'fallback': 'rgba8unorm decode-and-upload as today', 'coverage': 'mobile; adapter guarantees BC or (ETC2+ASTC), so the trio below covers every device'},
    'texture-compression-etc2': {'status': 'vaulted', 'consumer': 'partner row of astc', 'fallback': 'as astc', 'coverage': 'mobile guarantee partner of astc'},
    'texture-compression-bc': {'status': 'vaulted', 'consumer': 'desktop half of the astc row', 'fallback': 'as astc', 'coverage': 'desktop'},
    'subgroups': {'status': 'vaulted', 'consumer': 'none — prospective (reduction-shaped passes)', 'fallback': 'n/a until a consumer is named', 'coverage': 'offered on the floor; uneven elsewhere'},
}
```

### 3.2 Emission + request site (recon-first)

- Recon how `NEEDS` → `src/console/limits_floor.gen.inc` is emitted in
  `binding_gen.py` and how `console.hpp` consumes it. Mirror that pair:
  emit `src/console/features_wallet.gen.inc` carrying (a) the granted
  list in whatever form the device-request site can consume, (b) the
  wallet print data.
- Recon the device-request site (follow the boot line `granted
  timestamp-query=YES`): replace its literal feature list with the
  generated one. The request must remain **exactly** `{timestamp-query}`
  after this unit — that is witness F-1, checked by grep on the
  generated include.
- Boot print contract (Jean verifies at his gate): one `[Device]` line of
  the form `feature wallet: granted timestamp-query; vaulted 5 (schema
  FEATURES)`. Exact wording free within the template; content fixed.

**Commit (subject):**
`PROBATE_F: the feature vault gets a wallet — six rows, one grant, literals in the schema and nowhere else`

---

## §4 PROBATE_X — the FXC probate

**What.** PIVOT_0 retired the compiler; twenty sites still defend on its
name, four on it alone. Every site gets a ruling. **No lesson is lost:
every rewritten or retired comment block is copied VERBATIM into
`audit/FXC_LAWS_RECORD.md` first**, under a new appendix, keyed by
symbol. The tree's comments then state living reasons or point at the
record; history keeps one home.

### 4.1 The record appendix (create once, then fill as you go)

Append to `audit/FXC_LAWS_RECORD.md`:

```
## PROBATE appendix — the probate of 2026-08-16
Rulings of record for every site the FXC trigger matched at PROBATE_B.
RETIRE = the mention is history, moved here, tree comment dropped or
shrunk. REWRITE = the shape stays for a living reason now stated in the
tree; the FXC pricing lives here. KEEP = the tree text already speaks in
history tense or is the pivot statement itself.

| symbol / site | ruling | old text (verbatim) |
```

One row per site, filled as each ruling executes.

### 4.2 Scan set and classes

Scan: the ledger's four primary inputs + nine caller files (§Provenance
lists them) + `docs/LAWS.md` + `docs/` + `audit/` (record file exempt).
`grep -rn "\bFXC\b"`. Expect ~20 sites; record the actual count.

Classes for sites not pre-ruled below:
- **KEEP** — pivot statements, record pointers, history-tense prose.
- **RETIRE** — FXC named as sole or decorative justification for a shape
  that needs no defense (or whose living defense is already co-stated):
  move old block to the record, drop the FXC clause, keep any living
  clause.
- **REWRITE** — the shape has a living reason (RAW, budget, occupancy,
  structure) that the comment must now carry *instead of* the compiler's
  name; template: `<living reason>. (Shaped under FXC — retired,
  PIVOT_0; audit/FXC_LAWS_RECORD.md §PROBATE.)`
- A site defending a shape with NO living reason and no cheap unwind
  gets REWRITE with the closing clause `no living law bars reshaping
  here; reshape only with a measured reason` — that sentence is the
  probate's product: the fear dies, the shape stays until a measurement
  asks.

### 4.3 world.wgsl — pre-ruled (13 sites, symbol-anchored)

| anchor symbol | ruling |
|---|---|
| file banner (two mentions, COMPILER FLOOR block) | KEEP — the pivot statement itself. |
| §3 block near `audit/FXC_LAWS_RECORD.md` pointer (snapshot ~2528, "retired FXC laws forbade… The banner forbids") | KEEP the record pointer; recon what "the banner forbids" resolves to — if it still enforces a prohibition with no living reason, REWRITE that clause to the living gate (naga per commit; four-browser boot per landing). |
| `contrib_pawn_aura_at_self` | RETIRE — dead-code elimination of uniform branches is every backend's behavior; restate compiler-neutral, FXC clause to the record. |
| `query_ground_flyer` | REWRITE — the loop-unrolling price was FXC's; lesson to the record; if no living shape-reason remains, close with the `no living law…` clause. |
| `sample_shadow_pcf` | RETIRE the "FXC-clean by construction" blessing; keep the construction description (no array indexing, comparison sampler) as plain description. |
| `pawn_ground_resolve` (the liberation site) | REWRITE: "Factored out of each behavior body — good structure on its own. (Shaped under the FXC compile-cost law — retired, PIVOT_0; audit/FXC_LAWS_RECORD.md §PROBATE.) No living law bars branching in this chain; reshape only with a measured reason." |
| `agent_post_step` ("the old FXC sanctum") | KEEP the record pointer; shift any remaining active prohibition to history tense. |
| the kernel-split banner near `behavior_levy_flight` (the 48 s pricing; PIVOT_0 already marked FXC retired there) | REWRITE-FINISH: the split's living defense is Table E — RAW on `agent_state` / `floating_entities` / `field_forces` bars re-unification regardless of compiler. The 48 s pricing moves wholly to the record. |
| `orb_hsv_to_rgb` | RETIRE — describe the shape (explicit ifs, single path) without the dead compiler's blessing. |
| `orb_sample_palette` | RETIRE — same treatment. |
| `orb_dynamics` ("no FXC divergence penalty") | RETIRE — same treatment. |

### 4.4 Other files

Apply the classes. Two expectations: any remaining `renderer.hpp`
boilerplate PIVOT_0c missed → RETIRE; `state.hpp`'s SceneConstants
"do not upgrade to var<storage>" block defends on the living storage
budget (L14), not on FXC — expect no token there; if the grep disagrees,
KEEP the budget clause, probate only the FXC clause.

### 4.5 Rider — DAWN_REFERENCE.md

Recon its in-tree path. Insert one line under the title: `Documents the
ARCHIVED native build (native-sunset, PIVOT_0). Kept as record; nothing
here binds the web twin.` No other edit.

**Commits (two, subject):**
`PROBATE_X: world.wgsl probated — thirteen FXC sites ruled; lessons keep one home in the record`
`PROBATE_X: the wider tree probated — remaining FXC sites ruled; DAWN_REFERENCE marked as archive`

---

## §5 PROBATE_D — demand rulings and three statutes

### 5.1 Schema data (verbatim values; keys copied from §1's scratch note)

Add adjacent to `DEFENDED_SITES`:

```python
# ═══ DEMAND RULINGS (PROBATE_D) ═════════════════════════════════════
# The judgment cells of the ledger's Table D, as data — one home, the
# ledger joins by the queued-item key string it prints. A ruling here is
# Jean-gated; editing one is a schema edit like any other.
DEMAND_RULINGS = {
```

For **each of the 13 A2 rows** (key = the exact queued-item string):

```python
    '<key verbatim from Table D>': {
        'demand': 'none standing',
        'buys': 'one storage slot in the row(s) binding this layout',
        'costs': 'one uniform slot in the same rows + BufferUsage::Uniform on the backing buffer + a var<uniform> alias where another layout binds the slot with a different access',
        'ruling': 'PRICED, NOT SPENT — spend only when a named feature lands on this (pipeline, stage) and needs the slot; authorization is per-row and Jean\'s',
    },
```

For **the room-family row** (key verbatim):

```python
    '<room-family key verbatim>': {
        'demand': 'standing — the wall itself',
        'buys': 'n/a',
        'costs': 'n/a',
        'ruling': 'THE ROOM GROWS BY TEXTURE OR UNIFORM (see LAWS.md, PROBATE statutes). If a storage seat ever becomes non-negotiable, the one named payment is the agent_state + field_forces merge — two seats become one; both are already RAW-coupled everywhere (Table E) — authorized only then, by name, at Jean\'s gate.',
    },
}
```

### 5.2 Ledger join (instrument-adjacent but rides this subject commit,
because the emitter must learn the join for --check-era coherence)

Recon `binding_ledger.py`'s Table D emitter (search the section title).
Populate the four judgment columns from `DEMAND_RULINGS` by exact key;
unmatched key → emit the cell empty and print a warning line (a future
rename must not silently orphan a ruling). Update Table D's preamble
sentence to name the schema as the rulings' home.

### 5.3 LAWS.md — three statutes (recon existing numbering/format; append
in-form)

1. *The agents' room grows by texture or uniform, never by a new storage
   seat; the one named payment, if ever unavoidable, is the
   `agent_state` + `field_forces` merge.*
2. *A new compute-written surface prefers a layer in an existing
   storage-texture array over a new seat; the lane stands at 2 of 4.*
3. *A field the tree marks dead — "retained until the next relayout"
   and kin — dies in the same sitting that next opens its struct; twin
   rooms, one commit.* (Standing subjects today: `RibbonState.is_roaming`,
   `RibbonRingTransform._pad0`, `OrbConfig`'s driverless gen-1 block.)

**Commit (subject):**
`PROBATE_D: fourteen demand cells ruled from the schema; three statutes — the room grows by texture, arrays before seats, marked-dead dies on next opening`

---

## §6 PROBATE_R — the instrument close (single instrument commit)

**Authorization, per finding 12's protocol, granted here in advance:**
if any `DEFENDED_SITES` control site's expected trigger set changed
because §4 removed its `FXC` token by ruling, update the control's
expectation in this commit — identity keys stay, expectations follow the
probated prose. The `FXC` trigger itself **stays in W4-1, prospective** —
a zero-site trigger is a predicate over future text.

1. Apply any control-expectation updates (schema).
2. `python tools/binding_gen.py --check` — green.
3. Regenerate `BINDING_LEDGER.md`; commit it.
4. Record the movements in the report: Table H total (expect a drop from
   74 as probated boilerplate stops matching — the index getting more
   honest, again), `FXC` live-site count (expect ≈ the KEEPs), tightest
   storage row (expect unchanged: room family 5/8), patchgen lanes
   (uniform 4, immediates 32).

**Commit (instrument):**
`PROBATE_R: instrument close — controls re-keyed to probated prose, ledger regenerated; Table H rebased, ruling of record`

---

## §7 THE REPORT — PROBATE_REPORT.md (repo root; rides the §6 commit)

Sections, in order: **Header** (HEAD before/after, commit list);
**Flags** (the O-2 table — empty is a result too); **Numbers**
(§1 baseline vs §6 close, side by side); **Per-unit status** (done /
graduated / skipped, one line each); **Jean's gate checklist**, verbatim:

1. Pull master; the log shows five to seven `PROBATE_*` commits.
2. glaw1 build; boot the web twin on a pinned seed.
3. Console reads: `maxImmediateSize=64/32` in granted-vs-floor; the
   feature-wallet line with `granted timestamp-query; vaulted 5`;
   `[GPU Budget] buffers` down by roughly 7 KiB; all pipelines compile;
   zero validation errors.
4. The terrain on the pinned seed is identical by eye — heights, cell
   colors, GoL placement. This is PROBATE_I's visual gate.
5. Read §Flags. Anything there is queued judgment, not failure.

---

*Held out of this round, on the record: TEX_C0 (painting compression —
parked behind the wallet and Jean's eye against ASTC banding), every A2
spend (priced above), the agent merge (named payment, unspent), and the
perf watchlist for the SOAK round to come: gol_zone_compute, orb_sky,
frustum_cull, the serial 0D chain, the patch storm's pacing.*
