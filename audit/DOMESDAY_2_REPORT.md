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
## §7 — the glaw1 arbitration, round one

The campaign's first build bounce, in exactly the two units the _2
handoff pre-declared as glaw1-arbitrated. The error text, verbatim —
the specification:

```
C:\dev\7t\src\core\boot_params.hpp(51): error C3861: 'boot_params': identifier not found   (×2)
C:\dev\7t\src\console\console.hpp(178): error C2838/C2065/C2131/C2051: 'SubgroupSizeControl' … case expression not constant
```

The two fixes, master direct:

- **F1-a** (`7e9f8c8`) — the `SubgroupSizeControl` case deleted; only
  it (`PrimitiveIndex` and `TextureComponentSwizzle` drew no error and
  stay). A8's standing protocol executing, not a reversal; the id
  keeps printing as a number on both twins until the Dawn checkout's
  `wgpu::FeatureName` learns the identifier — the re-add condition,
  recorded at the switch.
- **F1-b** (`228ac37`) — the predicted defect class exactly: B10's
  `effective_msaa()` was defined above the `boot_params()` accessor it
  calls, in the same header. The block moved below the accessor —
  no restructure, no rename, byte-identical bodies. All 13 consumer
  sites verified resolving by grep.

The three instruments rerun green — nothing schema-facing changed.

**Standing note:** this build died in the first translation unit, and
it was the first native compile since B6's frontier spellings
(`var<immediate>`, `PipelineLayoutDescriptor::immediateSize`,
`SetImmediates`) — a second stratum may follow once the compile
proceeds past these two lines. Same channel if it does: send the
error text exactly; it worked.

## §8 — the runtime arbitration: the lane's real gate

Round two, from Dawn's validator rather than MSVC. The two primary
error lines, verbatim:

```
Error while parsing WGSL: shader requires the language feature 'immediate_address_space', but it is not enabled
[Invalid PipelineLayout (unlabeled)] is invalid
```

One root cause: `var<immediate>` (world.wgsl) and the
`immediateSize = 4` pipeline layouts require the device feature
`ImmediateAddressSpace` — dawn.json: WGSL language feature
`immediate_address_space`, value 11, tagged `dawn`; the
`pipeline layout descriptor`'s `immediate size` member; `limits`'
`max immediate size` — and the boot requested core defaults +
timestamp-query only. Every invalid module, pipeline, and submit in
the log was cascade from that one gate.

The fixes, master direct:

- **F2-a** (`837f481`) — both twins request the feature where the
  adapter offers it; the web twin's modest path carries
  `maxImmediateSize = FLOOR_MAX_IMMEDIATE_SIZE` (the NEEDS table's own
  seventh row, sourced from `limits_floor.gen.inc`, never a literal),
  and the testimony line names the exception. Not offered → one loud
  line and the failure surface stands, named. `feature_name()` gains
  `immediate-address-space(dawn)`. The pre-authorized second step
  (the instance-level WGSL control, native-only) was NOT taken — it
  engages only if the runtime bounces again with the feature granted.
- **F2-b1** (`74b73a2`) — the label law's second enforcement,
  triggered by the error message's own `(unlabeled)`, mirroring the
  first: `strataLayoutFor` takes a leading label, all 27 call sites
  named.
- **F2-b2** (`3007afc`) — the ledger parser learns the label argument
  (its own commit, per the instrument law); all three instruments
  green.
- **F2-c** (`c7de848`) — the `var<immediate>` banner points at the
  request site; the request site remains the fact's home.

**The learning, stated for the defended-site index:** a granted limit
is not an enabled feature — A3 probed the ceiling of a lane whose
door was still locked; probe the gate you intend to walk through.
B6's first parse in any court was this boot; the web twin is equally
untested post-B6 and faces the same gate at the next Pixel boot,
which will state its quadrant: offered-and-requested (testimony
line), core-shipped (no line, the lane just works), or unavailable
(the loud line — and then R3 gets re-examined with evidence, not
assumption).

## §9 — the runtime arbitration, round three: a language feature, not a device feature

Round two's fix specified the wrong enum. Jean's header probes
(f0bf8ab, `webgpu_cpp.h`) are the specification, and one line settles
it above all others:

```
939:    ImmediateAddressSpace = WGPUWGSLLanguageFeatureName_ImmediateAddressSpace,
```

— inside `928: enum class WGSLLanguageFeatureName : uint32_t {`. There
is **no** device `FeatureName` for it at this revision. The probes also
give the query and the control:

```
1797:    inline void GetWGSLLanguageFeatures(SupportedWGSLLanguageFeatures * features) const;
1798:    inline Bool HasWGSLLanguageFeature(WGSLLanguageFeatureName feature) const;
2588: struct DawnWireWGSLControl : ChainedStruct {
5007: static_assert(offsetof(DawnWireWGSLControl, enableExperimental) == ...
5009: static_assert(offsetof(DawnWireWGSLControl, enableUnsafe) == ...
5011: static_assert(offsetof(DawnWireWGSLControl, enableTesting) == ...
```

**The corrected mechanism.** The API half is core-shaped and
unconditional in the header — `SetImmediates` (2134/2160/2194),
`PipelineLayoutDescriptor::immediateSize` (4159),
`Limits::maxImmediateSize` (3875). Only the *dialect* is gated, and it
is gated at the **instance**, experimental-stage, through
`DawnWireWGSLControl`. Nothing about it is ever requested at the
device. That is why F2-a's device request could not have worked at any
value of any argument.

**The error, named.** F2-a's specification was Claude's authoring
error: a device feature invented from Dawn's error prose rather than
read out of Dawn's type system. It was faithfully implemented, and it
was correctly rejected by the runtime. The handoff's "pre-authorized
second step" — the instance-level control — was in truth the first and
only step; there was never a first step for it to be a fallback to.

**The tuition, for the defended-site index:** *Dawn's error prose is
not its type system — the enum's home, not the message's noun, names
the gate.*

The fixes:

- **F3-a** (`fd56a58`) — `DawnWireWGSLControl{ enableExperimental = true }`
  chained to the native `InstanceDescriptor` (the DXC toggle, when the
  plan selects it, rides the same chain behind it). It chains at the
  stage that *consumes* it, which is TOGGLE_1revA's standing law rather
  than an exception to it. The testimony, both twins:
  `[Device] wgsl language features: immediate-address-space=YES/no`,
  asked of the instance via `HasWGSLLanguageFeature`.
- **F3-b** (`e90de9d`) — the wrong enum leaves entirely (zero
  `wgpu::FeatureName::ImmediateAddressSpace` references remain in the
  tree); `requiredFeatures` is timestamp-query-only on both twins
  again. The **limit** stays — `maxImmediateSize = FLOOR_MAX_IMMEDIATE_SIZE`
  (NEEDS r7), now unconditional on the web modest path, because that
  half of F2-a was always true: the shadow family's layouts declare 4
  immediate bytes and a core-defaults request grants 0 unless asked.
  The exceptions line names the gate that exists:
  `wgsl:immediate_address_space (instance) + maxImmediateSize=4 (NEEDS r7)`.
- **F3-c** (this section).

Three disclosures, so the next round starts from evidence:

1. **`enableExperimental` alone, `enableUnsafe` recorded and not
   taken.** Enabling unsafe would widen the bench's dialect past
   anything the browser can offer the product — a bench more permissive
   than the product stops being a bench, because it would let native
   compile a shader the web twin refuses. If the testimony says `no`,
   the dialect list beside it (below) says whether the feature is
   merely un-enabled or absent at this revision, and *that* decides the
   next knob.
2. **One addition beyond spec, native-only:**
   `[Device] wgsl dialect allowed (N): …` — the instance's whole
   allowed set, named through a 17-case switch transcribed from the
   probe (929–945; transcription verified exact, in order, nothing
   invented or omitted). It converts a possible `no` into a *diagnosed*
   no inside the same boot, which is the whole lesson of three rounds.
   Fenced `#ifndef __EMSCRIPTEN__`: the web port's header is not in
   evidence, so that twin prints ids alone rather than guessing
   spellings.
3. **The one spelling taken without header evidence:** the web twin now
   calls `Instance::HasWGSLLanguageFeature` with
   `WGSLLanguageFeatureName::ImmediateAddressSpace`. The handoff
   requires the testimony on both twins and it is the only way to learn
   the web quadrant; the emdawnwebgpu header is outside the repository
   (A2's wall). If the web build rejects either identifier, that is the
   F1-a protocol again — send the error text, and the line degrades to
   a native-only testimony with the web quadrant still owed.

## §10 — the strategy rulings (slots, awaiting Jean's word)

Recorded here as F3-d specifies: no code, two slots, filled when given.

**Slot 1 — the twins ruling.** *Awaiting Jean.*
- **KEEP-THROUGH-ORGAN** *(recommended)* — the web is the product, the
  native twin is the bench; the bench retires when the web gains WGSL
  hot reload **and** the port is vendored. The criterion, not the mood,
  ends it.
- **RETIRE-NOW** — then a retirement campaign is authored and glaw1
  becomes the Emscripten link gate.

**Slot 2 — the one-generation law.** *Awaiting Jean's ratification;
numbered at first enforcement.* The native checkout's revision is
pinned equal to the port's pinned revision; both recorded in-tree;
updated as one act; a reference document without a stated revision is
RECALLED, not CITED. (This campaign is its own argument: three
arbitration rounds, each one a spelling read from a document whose
revision was not pinned to the compiler's.)
