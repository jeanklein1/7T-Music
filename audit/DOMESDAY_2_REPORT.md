# DOMESDAY_2 — the batch report

The closing batch: instruments completed, deeds written, the walk's
last affordance shipped. All three instrument gates
(`binding_gen.py --check`, `binding_ledger.py --check`,
`command_census.py --check`) are fully green on merged master. No
compile ran in this environment; glaw1 remains the batch's compile
gate (B10's msaa arm and A13's three feature enumerators are the
spellings it arbitrates).

**Global gate 2, the codified merge-state verification, fired:**
`claude/domesday-1` was fully green but unmerged; merged as the
enacting step at `4bc958f`, no conflicts, gates green on the merged
tree before any edit. At the batch's end `claude/domesday-2` was
merged likewise at `9524dbc` per Jean's standing instruction.

## §1 — unit table

| unit | verdict | where |
|---|---|---|
| A10 — immediates column made true | **LANDED** `8517824` | `master` — PIPELINES gains `immediate_size`, captured from the pipeline-layout creation sites; MANIFEST and ledger Table B print the real bytes; **witness M-2's first run caught a live B6 defect** (below) |
| A11-fix — resource label law | **CLOSED, zero edits** | the census found every creation site already labeled — makeBuffer/makeTexture take the label as a parameter, the samplers and query set carry descriptor labels; R-Label pins the verdict |
| A11-rel — RESOURCES relation | **LANDED** `deba10c` | `master` — 92 rows at landing (94 after B10), diffed both directions; R-1, R-2, R-Label all green; **zero orphans** |
| A12 — the floors' one home | **LANDED** `6565eff` | `master` — schema NEEDS table → `limits_floor.gen.inc` → the boot print and the three below-floor nets; witness R-3 |
| A13 — deeds and truthing | **LANDED** `325aa19` | `master` — both pools deeded verbatim; +3 feature cases; FRAME_1 comments trued; item 3 had been completed by A12 |
| B10 — `?msaa=` | **LANDED** `3adc7f0` | `claude/domesday-2` — the walk's last instrument; default face byte-identical |
| B-close — regenerate | **LANDED** `f58bd84` | `claude/domesday-2` |
| merges | `4bc958f` (domesday-1), `9524dbc` (domesday-2) | `master` |

**M-2's first catch, fixed inside A10:** B6 set `immediateSize` on
`shadowRenderLayout` and missed its sibling `galleryShadowLayout` —
the two shadow artwork pipelines read the `shadow_slot` immediate
through a pipeline layout that declared 0 bytes, which Dawn would
have rejected at creation on the next boot. The tree-vs-schema diff
surfaced it the first time the fact existed to compare;
`galleryShadowLayout` now passes `sizeof(uint32_t)`. The instrument
caught the defect before the build gate could.

## §2 — the ruling record addendum (verbatim)

> **R5 — staging retirement: WITHDRAWN IN FULL, both halves,
> superseding _1's adoption of the authored half.** The snapshot pool
> is the photographer's portfolio (B8's census, _1 report §1). The
> authored half falls to the same lens plus unified memory: on the
> audience device (ARM, unified LPDDR) a retained CPU-side inventory
> costs the same physical bytes as the staging texture it would
> replace, so the −32 MiB is an accounting illusion; decode-at-hang
> would add stb work inside the world transition the no-teleportation
> principle protects; the only genuinely cheaper inventory is
> browser-owned ImageBitmaps, blocked behind the un-vendored port
> (A2's wall). Both pools are deeded (unit A13). Re-open only with a
> vendored port and an ImageBitmap design, as one named campaign.

## §3 — wallet final state (post-A10 MANIFEST, merged master)

| lane | worst used / limit | at |
|---|---|---|
| uniform | 11 / 12 | `updatePlayerAgentPipeline_` C (+3 more — the panel campaign's opening row) |
| storage | 5 / 8 | `updatePlayerAgentPipeline_` C (+7 more; no render row above 4) |
| sampled | 6 / 16 | `patchTerrainPipeline_` F |
| samplers | 3 / 16 | — |
| storagetex | 2 / 4 | — |
| **immediates** | **4 / 64 — the column's first true nonzero row** | `shadowPatchTerrainPipeline_` V (+12 more, the whole shadow family) |

`dyn_u`/`dyn_s` stand at 0/8 and 0/4 program-wide (witness `0d-1`);
NEEDS carries seven floors, the seventh being the immediate lane's
own 4 bytes — the program's first stated need above a Core default's
mere reliance.

## §4 — orphans found by R-2

**None.** 94 rows, every one reached by a bind-group entry, a pass
attachment, a draw/dispatch argument, or a copy/write site. The reach
net learned two tree truths on the way (the writeArray/writeStruct
wrappers; the drawable-table convention of buffers riding helper
arguments) — recorded in A11's commit, no orphan behind either.

## §5 — the soak-walk matrix, ready to hand

Arms × cap × msaa, one 30 s meter window per cell (the meter's
existing cadence, `CENSUS_DUMP_INTERVAL`). Two purse questions —
native density and edge quality — one walk:

| arm | URL | cells |
|---|---|---|
| S0 OPEN_SUNSET | `?seed=42&mood=0` | × `cap={1.5, 2.25}` × `msaa={1, 4}` |
| S1 INDOOR_FLAT | `?seed=42&mood=1` | × `cap={1.5, 2.25}` × `msaa={1, 4}` |
| S2 INDOOR_VAULT | `?seed=42&mood=2` | × `cap={1.5, 2.25}` × `msaa={1, 4}` |
| S3 FINITE_OUTDOOR | `?seed=42&mood=3` | × `cap={1.5, 2.25}` × `msaa={1, 4}` |

Example cell: `?seed=42&mood=2&cap=2.25&msaa=4`. Sixteen cells,
each self-reporting: the `[Params]` line names the cell, the
`[GPU Budget]` census names the msaa allocations when active, and
the meter's window closes each measurement. Native spelling:
`--seed=42 --mood=2 --cap=2.25 --msaa=4`.

## §6 — anything unexpected, one line each

- **M-2's first run found a real defect** (B6's missed sibling
  layout) — the happiest possible failure of a new witness, fixed in
  the same commit that born it.
- **A11-fix had nothing to fix**: the label law for resources was
  already satisfied by construction — the makeBuffer/makeTexture
  pattern was the law before the law was named.
- **NEEDS grew a seventh row beyond the handoff's six**: since B6 the
  program genuinely stands on `maxImmediateSize ≥ 4`, so the floor
  prints 4, not "0 — unused"; A13's item 3 was thereby completed by
  A12.
- **RESOURCES extends the handoff's kinds**: sampler and querySet rows
  join buffer|texture so "one row per GPU object" holds literally.
- **The command census renders conditional attachments as
  "base or arm"** — B10 made pass rows 7 and 16 two-faced, and the
  ledger now says so instead of showing whichever assignment came
  last.

One line for the spirit: **the survey ends with every holding deeded
or earning, every object owned, and one instrument in Jean's hand —
what the frame can still afford is now a question with an address.**
