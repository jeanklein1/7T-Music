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
  asked of the instance via `HasWGSLLanguageFeature`. **F3-e supersedes
  the exact string**: each twin appends the instance it measured —
  `… =YES (instance: enableExperimental)` native,
  `… =YES (browser default — this twin enables nothing)` web — so a
  native log can never be read as a promise about the phone. The
  handoff's string is normative as the *prefix*.
- **F3-b** (`e90de9d`) — the wrong enum leaves entirely (zero
  `wgpu::FeatureName::ImmediateAddressSpace` references remain in the
  tree); `requiredFeatures` is timestamp-query-only on both twins
  again. The **limit** stays — `maxImmediateSize = FLOOR_MAX_IMMEDIATE_SIZE`
  (NEEDS r7), now unconditional on the web modest path, because that
  half of F2-a was always true: the shadow family's layouts declare 4
  immediate bytes and a core-defaults request grants 0 unless asked.
  The exceptions line names the gate that exists. **F3-e and F3-f
  supersede the exact string**, because a device request cannot carry a
  dialect and must not claim bytes it withheld; what HEAD prints is
  `exceptions carried: maxImmediateSize=4 (NEEDS r7); wgsl:immediate_address_space (instance) present|ABSENT`
  — or `carried: none (maxImmediateSize withheld, see above)` when the
  adapter cannot back the ask. Both halves of the handoff's string
  survive as facts; only their grammar changed, from *claimed* to
  *reported*.
- **F3-c** (this section).
- **F3-e** (`caf54b4`) — an adversarial verification pass over F3-a/F3-b
  (three independent lenses, then an adjudicator required to prove each
  finding from the file) caught two real defects and four truths. The
  defects are worth naming because both are *this campaign's own
  recurring species*: (i) the web modest path claimed
  `wgsl:immediate_address_space` unconditionally and could deny it two
  lines later — a boot log contradicting itself, so both twins now
  print what the request CARRIES separately from what the instance
  HAS; and (ii) **the retraction had stopped at the token** — F2-c's
  comment at the `var<immediate>` declaration still taught the
  device-feature doctrine and pointed at a request site F3-b had
  deleted. A9's and A7's lesson, third time: the token and the prose
  that explains it are one edit, never two.
- **F3-f** (`3f25060`) — the same verification pass's major finding, and
  a regression F3-b introduced: F2-a's `maxImmediateSize` ask was gated
  (and, on a nonexistent device feature, never fired); F3-b made it
  **unconditional**. WebGPU rejects a required limit better than the
  adapter reports, and this file's rejection path reissues as full
  passthrough — the one shape PORT_6c/L14 forbids as a design, stated
  in that function's own banner — while the granted-vs-floor census
  lives under `if (!passthrough)` and is therefore skipped on the
  reissue. So on any adapter reporting `maxImmediateSize` under 4, the
  boot would have lost the immediate lane's only printed number on
  exactly the device whose lane was in question. The ask is now gated
  on the adapter's own reported value, the withholding prints its
  reason, and the exceptions line reports what was actually requested.
  The Pixel reports 64 (A3's row at the DOMESDAY_1 boot), so the gate
  stands open there; it exists for the adapter that reports 0.

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

## §10 — the one-generation law, first enforcement: three toolchains, three ages

*Numbering.* The F5 handoff names this section "§10 (replacing F4's
never-run draft)". F4 never ran, so the slot it would have taken was
never occupied — what stood at §10 was F3-d's pair of ruling slots.
Those keep their text and move down one, to §11, where Slot 2 is
amended in exactly one place: it stops awaiting a first enforcement,
because this section is it. `third_party/emdawnwebgpu/PINNED.md`'s
cross-reference to "§10" resolves here, as it was written to.

### F5's units

| unit | verdict | where |
|---|---|---|
| F5-a — vendor the port | **LANDED WITH A FLAG** `a059949` | `master` — the pin recorded (`third_party/emdawnwebgpu/PINNED.md`), CMake wired to prefer it in three verified states, `requires immediate_address_space;` placed in `world.wgsl`. **The bytes are not in the tree** — see below |
| F5-b — native dev tier | **LANDED** `cd91a29` | `master` — right about the *tier*, wrong about the *mechanism*; corrected inside the round by F5-d, reverted in nothing |
| F5-d — the knob that arrives | **LANDED** `a281a96` | `master` — **CC-authored, not in the handoff**; `allow_unsafe_apis` as an instance toggle, the wire control kept and relabelled |
| F5-c — this section | **LANDED** (this commit) | `master` |

### The corrected generation map

Three arbitration rounds treated one question — *does this program's
immediate lane exist?* — as though the toolchain had one answer. It has
three, and they disagree by age. **The ordering is the finding: the
audience is ahead of the bench, and the bench is ahead of the port.**
Every round so far read the lane off the bench, which is the middle
age, and the one nobody is shipping to.

| # | toolchain | its generation | what it says about the lane |
|---|---|---|---|
| 1 | the web twin's port, pre-F5-a | emsdk-bundled emdawnwebgpu — **revision unknowable by construction**: it follows whatever emsdk is installed | the API half does not exist: no `SetImmediates`, no `ImmediateAddressSpace` enumerator — but `Limits::maxImmediateSize` is already there |
| 2 | the native bench | Dawn `f0bf8ab547a9…`, **authored 2025-12-22** | the API half is core-shaped; the dialect is a **Dawn extension** at the **unsafe-experimental** tier |
| 3 | the audience | Chrome stable, the Pixel | granted `maxImmediateSize=64` at the DOMESDAY_0 boot; ships the dialect as standard (the dialect half is owed by the court probe) |
| — | the pin — what #1 becomes | `56f332d7d8d0…`, tag `v20260814.182433`, **2026-08-14** | the dialect is a **standard** language feature at `FeatureStatus::kShipped` — exposed by default, no control of any kind |

Age 2's dates are worth reading twice. The commit is from **December
2025**; the boots that trusted it are dated 2026-07-29 and 2026-08-13
(`docs/LAWS.md:560`, `console.hpp:1342`). The bench the campaign has
been arbitrating against is eight months older than the sessions
consulting it, and nothing in the tree said so until this round —
which is the whole argument for the law below.

#### Age 1 — the port: the evidence, and the one thing owed

The three refusals are quoted from the F5 handoff, because the
compiler's own words are not in this repository:

> the oldest Dawn in the toolchain (its RenderPassEncoder lacks
> `SetImmediates`; its WGSLLanguageFeatureName lacks the enum; its
> Limits struct already carries maxImmediateSize, which is why A3
> compiled)

**OWED, one slot:** the web build's three error lines, verbatim. They
reached CC as prose, never as text, and CC's environment has no emsdk
to reproduce them. The slot stays open rather than being filled with a
paraphrase of a compiler — the same discipline that made F1-a's error
text the specification.

One clause is corroborated in-tree without them: **A3 compiled on the
web twin** (`1356750`, DOMESDAY_0 §1 — the granted-vs-floor row reads
`Limits::maxImmediateSize`), which is only possible if that member
already existed in the port's generated header. So the port is old
enough to lack the entry points and new enough to carry the limit. That
middle state is exactly why the wall stayed invisible for two batches:
the probe row that was meant to survey the lane compiled *because* it
touched the one part of the lane the port already had.

#### Age 2 — the bench: four citations, read from Dawn this round

Read from `google/dawn` at `f0bf8ab547a9a23b8b78ff67d8085d4a26600a7d`
— the same forty hex digits the boot log prints (`audit/SOAK_0.md`,
`THE BOARD FULL RELEASE CONSOLE.md:288`), fetched from upstream this
round through the git lane, not recalled:

1. `src/dawn/dawn.json` entry 11 — `{"value": 11, "name": "immediate
   address space", "jsrepr": "'immediate_address_space'", "tags":
   ["dawn"]}`. **Dawn-tagged**: a vendor extension at this generation.
   At the pin the same entry carries no tags at all — it has become
   the statute's own feature. That single field difference *is* the
   generational gap the pin closes.
2. `src/tint/lang/wgsl/feature_status.cc` — `kImmediateAddressSpace`
   sits in the *Experimental features* group and returns
   `FeatureStatus::kUnsafeExperimental`. At the pin it sits in the
   *Shipped* group.
3. `src/dawn/native/Instance.cpp`, `InstanceBase::GatherWGSLFeatures` —
   for `kUnsafeExperimental`, `enable = mToggles.IsEnabled(Toggle::
   AllowUnsafeAPIs)`. `ExposeWGSLExperimentalFeatures` appears one case
   below, on `kExperimental` only, and therefore cannot reach it.
4. `src/dawn/native/Toggles.cpp` — `Toggle::AllowUnsafeAPIs` spells
   `"allow_unsafe_apis"` and is `ToggleStage::Instance`, defaulted
   false at instance creation (`Instance.cpp:212`).

F5-b read the tier correctly from this. What it could not know without
(3) and (4) is that the *struct* it set is not what Dawn reads — which
is F5-d, below.

#### Age 3 — the audience: the strongest signal, and the least consulted

From the DOMESDAY_1 handoff's opening line, which is Jean's own report
of the DOMESDAY_0 Pixel boot:

> DOMESDAY_0 closed fully green: the branch is merged, all three twins
> run, the Pixel granted `maxImmediateSize=64`.

Sixty-four bytes granted, against a program that stands on four
(`FLOOR_MAX_IMMEDIATE_SIZE = 4`, NEEDS r7 — the shadow light index).
Sixteen times the need, from the only device the artwork is actually
for, recorded eight months before the bench was asked whether the lane
was experimental. That is the number the map should have been built
from.

It is the API half only. The dialect half — whether the phone's Tint
compiles `var<immediate>` — is what the ten-second court probe asks,
and it is the last cell of this table still empty.

#### The pin — the generation the web twin adopts

Verified in F5-a against the tagged tree and restated here for the map:
untagged entry 11, `FeatureStatus::kShipped`, which
`feature_status.h` defines as "exposed by default and cannot be turned
off." At this generation the entire F3 enablement apparatus is a
courtesy to older toolchains, not a requirement. Full pin table,
provenance and census: `third_party/emdawnwebgpu/PINNED.md`.

### The statute, cited

- **WGSL §14.3** — the address-space table's `immediate` row.
- **WGSL §7.3** — the immediate-data variable paragraph (`var<immediate>`).
- **WGSL §4.1.2** — `immediate_address_space` as a standard language
  extension. The `requires` directive is that section's own mechanism,
  and F5-a placed it at the head of `world.wgsl` §1 so the shader names
  its one dependency and fails by name if an implementation lacks it.
- **WebGPU API core** — `setImmediates`,
  `GPUPipelineLayoutDescriptor.immediateSize`,
  `supportedLimits.maxImmediateSize`. Core, not an extension: which is
  why the native header carries them unconditionally (§9's probe lines)
  and why only the *dialect* was ever gated.

*Carried from the handoff. The spec's own working-draft date is not
recorded in-tree — under the law this section enacts, a small second
debt: the next round that touches the lane records the date beside the
section numbers, and the citation stops being a document without a
revision.*

### F4 — retracted, unexecuted, superseded

F4 was retracted by the F5 handoff's own opening: *"F4 IS RETRACTED —
never run it; this supersedes."* It never ran. No F4 unit was begun, no
F4 edit exists, no F4 commit is in the history — `git log --all` over
every subject line matches `F4` zero times (the earlier `grep -ci`
hits were sha substrings, disambiguated before acting). Its report
draft is superseded by this section and its tuition line by the one
below. Nothing of F4 is carried forward, and nothing was reverted to
undo it, because there was nothing to undo.

### F5-d — the round's own correction, disclosed

**CC-authored, not in the handoff.** The F3-f precedent: verify the
round's premise against primary sources, and when the verification
finds a defect in the round's own unit, fix it inside the round and
report it here.

F5-b set `DawnWireWGSLControl::enableUnsafe`. That struct is read in
exactly one place at `f0bf8ab` — `src/dawn/wire/client/Instance.cpp`,
where `Instance::Initialize` unpacks it and the client's own
`GatherWGSLFeatures` reads its three bools. **Nothing under
`src/dawn/native/` reads it**; the native `GatherWGSLFeatures` takes
only a blocklist, and `InstanceBase::Initialize` reads only
`DawnInstanceDescriptor` off the chain. This twin links `dawn::native`
directly (`dawnProcSetProcs(&dawn::native::GetProcs())`) — there is no
wire client anywhere in the program. `dawn.json` lists the instance
descriptor as the struct's chain root, so `ValidateAndUnpack` accepts
it and then no one reads it: **accepted and ignored, PIVOT_0a's exact
species, and by P6 indistinguishable in the log from a switch that
never fired.** F3-a's `enableExperimental` has been inert on this twin
since it landed, and F5-b's `enableUnsafe` would have been inert too.

The fix is one line of intent and a small restructure: the instance
descriptor now always chains a `DawnTogglesDescriptor` enabling
`allow_unsafe_apis`, with `use_dxc` joining the same array under the
DXC plan instead of owning the chain link. It sites the toggle at
**Instance** stage, which is where Dawn reads it — L21 again, not an
exception to it — and it does not depend on the instance → adapter →
device inheritance TOGGLE_0 proved unreliable here, because the reader
*is* the instance. The wire control stays, relabelled `INERT on
dawn::native`: correct for a wire client, one ignored chain link here,
and deleting it would erase the finding along with its subject (the
same reason `use_dxc` stays under its own banner).

Prose moved with the token, F3-e's lesson for the fourth time: the boot
line now reads `(dev tier: allow_unsafe_apis — the audience court is
Chrome stable, which ships the lane standard)`, crediting the knob and
not the struct beside it; the dialect-list comment states that no other
flag remains to try, so a `no` there now means *not implemented at this
revision*; and `world.wgsl`'s generation note names the older
generation's actual gate.

**What F5-d does not do:** it reverts nothing. F5-b's diagnosis of the
tier was right and is now carried by a mechanism that arrives.

*A sibling for the defended-site index, proposed beside F3's line: a
struct whose members name your three tiers is not thereby the thing
that reads them — find the reader, not the noun.*

### The bytes — the FLAG stands

**Expected:** the release zip vendored under `third_party/emdawnwebgpu/`.
**Found:** CC's environment cannot fetch it. Anonymous *git* reads of
public GitHub repositories work (every upstream fact in this section
was verified that way); `github.com` HTTP does not — the releases API,
the releases page and the asset URL all return 403 through the proxy.
Attaching the repository with credentials was declined by the
classifier, and was not worked around. Dawn's sanctioned local build of
the package needs emsdk, which this environment does not have.
Hand-assembling a look-alike from the source tree was **refused on
principle**: an unofficial package with no upstream hash is precisely
"a reference document without a stated revision," and vendoring one as
this law's first enforcement would defeat the law it enforces.

Four lines close it, from any machine with the network:

```
curl -L -o emdawnwebgpu_pkg-v20260814.182433.zip \
  https://github.com/google/dawn/releases/download/v20260814.182433/emdawnwebgpu_pkg-v20260814.182433.zip
sha256sum emdawnwebgpu_pkg-v20260814.182433.zip     # record it in PINNED.md's table
unzip emdawnwebgpu_pkg-v20260814.182433.zip -d third_party/emdawnwebgpu/
test -f third_party/emdawnwebgpu/emdawnwebgpu_pkg/emdawnwebgpu.port.py && echo PINNED
```

No build-system edit follows: CMake already prefers that path the
moment the file exists, says which of the three states it is in at
configure time, and falls back loudly to the emsdk built-in port until
then. Nothing is silent either way.

### The side effect F5-a flagged for the report, not for action

The vendored header heals **A2's wall** — the feature-name map that was
FLAGGED in DOMESDAY_0 for want of an in-tree authority (`no
authoritative WGPUFeatureName enum in the tree`) gets one the moment
the package lands, and the web twin's three unprobed identifiers stop
being unprobed. Completing that map is a later unit, named here and not
smuggled into this one.

### The law earns its number

**L24 — THE ONE-GENERATION LAW** *(numeral proposed; Jean confirms)*.
`docs/LAWS.md` runs to L23′, so L24 is the next free numeral. Proposed
text, for `docs/LAWS.md` once ratified:

> The web twin's WebGPU generation is **this pin**; the native checkout
> **tracks** it; a reference document without a stated revision is
> **RECALLED, not CITED**. The pin, the native revision, and the fact
> that they match are recorded in-tree and updated as one act.
>
> **Witnesses:** `third_party/emdawnwebgpu/PINNED.md`; the boot's
> `[Console] Dawn revision:` line; the dialect testimony, which names
> the instance it measured.
>
> **Paid for by** three arbitration rounds and one retraction. F1 cost
> a compile on a spelling read from a document whose revision was not
> the compiler's. F2 cost a boot on an enum that does not exist at any
> revision. F3 corrected it from a probe of the right header — and was
> still reading the *bench's* generation, eight months behind the
> audience's. F4 was withdrawn before it ran. The one fact that would
> have shortened all four was a pinned generation, written down.

It is **not** written into `docs/LAWS.md` by this unit. Recording a law
under a numeral the estate has not ratified would be the same species
of error the law exists to forbid — so the text waits here, one word
from Jean, and the LAWS.md entry is a two-line unit whenever he gives
it.

### Tuition

**three toolchains, three ages — locate the laggard before ruling on
the lane; weigh the audience's direct signal above any proxy
generation.**

### AFTER-CC, as F5-d amends it

0. **The ten-second court probe** — unchanged, and now the *decisive*
   evidence rather than a nicety: it is the only empty cell in the
   generation map, and it settles age 3's dialect half directly instead
   of by proxy. Desktop Chrome stable, DevTools:
   `const a = await navigator.gpu.requestAdapter(); console.log(a.limits.maxImmediateSize, [...navigator.gpu.wgslLanguageFeatures])`
   — expect `64` and a set containing `immediate_address_space`.
1. **Web build with the vendored port** — gated on the bytes FLAG
   above; the four lines first, then `cmake --preset the-board-web`.
   The three refusals should be gone. (If any survives, *that* error
   text is the next specification, F1-a's protocol.)
2. **Native: glaw1 + run** — F5-d changes what the two outcomes mean.
   `YES` proves the toggle route, and the map's age-2 row becomes
   fully witnessed. `no` beside a populated dialect list no longer
   leaves a flag untried: `allow_unsafe_apis` is the only gate Dawn
   consults for a `kUnsafeExperimental` feature at this revision, so a
   `no` means the feature is not implemented at `f0bf8ab` at all, and
   the remedy is the law's other half — the checkout tracks the pin.
   Native runtime stays parked until then, and nothing else waits on
   it.
3. **Pixel boot, deploy, the sixteen-cell walk** — unchanged, and
   finally unblocked.

## §11 — the strategy rulings (slots, awaiting Jean's word)

Recorded here as F3-d specifies: no code, two slots, filled when given.

**Slot 1 — the twins ruling.** *Awaiting Jean.*
- **KEEP-THROUGH-ORGAN** *(recommended)* — the web is the product, the
  native twin is the bench; the bench retires when the web gains WGSL
  hot reload **and** the port is vendored. The criterion, not the mood,
  ends it.
- **RETIRE-NOW** — then a retirement campaign is authored and glaw1
  becomes the Emscripten link gate.

**Slot 2 — the one-generation law.** *Enforced first at F5-a/F5-d; see
§10, which proposes the numeral **L24** and the text. What remains
awaiting Jean is the numeral's confirmation and the LAWS.md entry, not
the enforcement.* The native checkout's revision is
pinned equal to the port's pinned revision; both recorded in-tree;
updated as one act; a reference document without a stated revision is
RECALLED, not CITED. (This campaign is its own argument: three
arbitration rounds, each one a spelling read from a document whose
revision was not pinned to the compiler's.)
