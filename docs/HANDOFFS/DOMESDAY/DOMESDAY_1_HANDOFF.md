# DOMESDAY_1 — RULINGS ENACTED, THE ESTATE SPENT INTO
## Handoff to CC · authored by Claude · rulings delegated by Jean and decided below · Jean holds the gates

DOMESDAY_0 closed fully green: the branch is merged, all three twins
run, the Pixel granted `maxImmediateSize=64`. This batch enacts the
rulings that evidence made ready. Same protocol as _0 — read it there
if fresh: **FLAG-AND-CONTINUE** (per-unit census gates; a failed gate
writes a FLAG to the report, reverts that unit, continues; one unit =
one commit or zero; dependents cascade-skip; the report always lands).

**Global stops only:** dirty tree / not on current master at start;
`binding_gen.py --check`, `binding_ledger.py --check`, or
`command_census.py --check` failing **before any edit**. L1 encoding
law throughout (LF, no BOM, writers pin `newline="\n"`).

## THE RULING RECORD (write into the _1 report verbatim)

Jean delegated these six decisions; Claude ruled; recorded here so no
future context re-litigates:

- **R1 — orb ping-pong→bind-group-swap: DECLINED.** L23′'s letter
  (LAWS.md:577) bars mixed-writability faces of one buffer sharing a
  layout; its pessimism clause prices any relaxation at a witnessed
  Dawn behavior test; the recoverable purse (two seats in a lane with
  six free, one two-workgroup dispatch) does not pay for the test.
  The copy pass is the law's price.
- **R2 — plan-group collapse: ADOPTED** (unit B5).
- **R3 — shadow_slot → immediate: ADOPTED, single path, browser floor
  raised** (unit B6). Jean's stated principle governs: *impress all
  who can open the site rather than mildly impress everybody.* No
  dual path; `fallback()` catches the ancient.
- **R4 — reconfigure debounce: ADOPTED, K = 6 stable frames**
  (unit B7).
- **R5 — staging retirement: snapshot half ADOPTED now** (unit B8);
  authored half is DOMESDAY_2, alone.
- **R6 — feature names by enumerator switch: ADOPTED** (unit A8).

Held, named, untouched: caster instancing · stream_patches
amortization · render bundles · wider indirect · **MSAA** — newly
recorded as held with its coupling: main-pass multisampling forces a
pipeline `multisample.count` shared with the snapshot pass, so it must
be designed *with* the exhibition-direct render path, not before it.

---

## STREAM A — master direct. Prefix `DOMESDAY_1 A<n>:`

### A7 — stale-prose sweep (delete, don't annotate)

Three sites, all recorded in the _0 report §5:

1. `world.wgsl:12713-12722` — the orb banner describing bind topology
   by retired pre-recut numbers (410–414, 201, 280). Rewrite to
   present truth in ≤ the same line count, or delete lines that only
   narrate history.
2. The `cullStateLayout_` seat-2 trailing comment glossing
   `fc_visible` by the retired sibling name `visible_patch_indices`
   (schema `LAYOUTS` prose → regenerate). Describe content
   ("cull-visible patch ids"), name no retired symbol.
3. At the orb copy-kernel comment (`world.wgsl:13055-13059`), append
   one line: `// Swap-two-groups alternative DECLINED under L23'
   (DOMESDAY_1 R1).` — so the proposal cannot be innocently remade.

Census gate: each anchor text found where cited; FLAG any that moved.
Comment-only + schema-prose edits; `--check` after the schema touch.

### A8 — feature ids get names (R6)

At the print site `src/console/console.hpp:643` (`adapter offers`),
add a `feature_name(WGPUFeatureName)`-style helper: a `switch` over
**`wgpu::FeatureName` enumerator identifiers** — never numeric values.
Provenance rule: the compiling header supplies every value; glaw1 is
the witness that each identifier exists. Author the case list from the
WebGPU spec's feature names (kebab-case), Dawn PascalCase convention:

`CoreFeaturesAndLimits, DepthClipControl, Depth32FloatStencil8,
TimestampQuery, TextureCompressionBC, TextureCompressionBCSliced3D,
TextureCompressionETC2, TextureCompressionASTC,
TextureCompressionASTCSliced3D, IndirectFirstInstance, ShaderF16,
RG11B10UfloatRenderable, BGRA8UnormStorage, Float32Filterable,
Float32Blendable, ClipDistances, DualSourceBlending, Subgroups,
TextureFormatsTier1, TextureFormatsTier2`

`default:` prints the number (unknown ids stay honest). Print names
in the same `adapter offers` line (or one wrapped line beneath — keep
the id list too; the pair is the census). **Note in the report:** any
enumerator glaw1 rejects gets removed in a follow-up — the gate's
error text is the correction; do not guess spellings twice.

### A9 — label law, first enforcement (two commits: subject, then instrument)

- **A9-fix:** every `CreateCommandEncoder` and every
  `Begin{Render,Compute}Pass` descriptor carries a `label`. The pass
  descriptors already do (COMMAND_LEDGER §1 has 20 labels); the
  evidence of absence is the native error log's
  `[CommandEncoder (unlabeled)]`. Census all encoder-creation sites
  (the _0 scan set: render_passes.hpp, cartridge.hpp, patch_system.hpp,
  gol_zones.hpp, pawn.hpp, gallery.hpp, orbs.hpp, incubator_dual.cpp);
  label each from its enclosing function (e.g. `"frame"`,
  `"flush_zone_derive_requests"`). Mechanical strings, no behavior.
- **A9-wit:** `command_census.py` gains **witness C-7**: every
  encoder-creation and pass-begin site carries a label (count them;
  zero unlabeled). Regenerate COMMAND_LEDGER; commit.

Label law text for the report: *labels are emitted where objects are
created, from the creating function's name — never hand-swept again;
C-7 stands at every landing.*

---

## STREAM B — branch `claude/domesday-1` off merged master. Prefix `DOMESDAY_1 B<n>:`
*Held for Jean: glaw1 + visual + merge. Expected visual delta of the
whole branch: none, except B7's accepted ≤100 ms scale-softness during
resize animation. Order below is execution order — B6 last by design.*

### B5 — collapse the byte-identical scene groups (R2)

Census gate: enumerate the bind groups instantiating
`sceneStateLayout_` (0a-6 listed four: base, PlanB, PlanC,
Photographer). Byte-compare their entry lists **post-B3**. Expected:
base ≡ PlanB ≡ PlanC (the _0 report §5 says their segment windows
were the only difference and B3 retired them); Photographer compared,
collapsed **iff** identical, kept with one comment line iff not.
Edits: delete the redundant group objects (schema `GROUPS` rows +
creation sites + bind call sites re-pointed to the survivor).
`--check`, regenerate, one commit. FLAG if the byte-compare
contradicts the expectation.

### B7 — reconfigure debounce, K = 6 (R4)

At `Console::begin_frame` (`console.hpp:1291`, quoted in
COMMAND_LEDGER §3): replace the bare not-equal branch with a settle
window —

```
if (fb != current) {
  if (fb == pending) { if (++stable >= 6) reconfigure(fb); }
  else { pending = fb; stable = 1; }
} else { stable = 0; }
```

(members `pendingW_/pendingH_/stableFrames_`; `reconfigure` = the
existing body: sizes, `surface_.Configure`, `createDepthBuffer`,
`frame1_report`). Boot path untouched — `initSurface`
(`console.hpp:1177`) still configures immediately. **Keep the FRAME_1
print**: it is this unit's acceptance witness — the next Pixel boot
should show it fire ~once per settled size instead of a burst.

### B8 — snapshot renders straight into the Exhibition (R5, first half)

Census gates (all from _0 report §2, verify in-tree):
- `exhibitionTexture_` usage today is `CopyDst | TextureBinding`
  (`state.hpp`); `snapshotStagingTexture_` exists with
  `Dim::STAGING_LAYERS`; `render_snapshot_pass` (`gallery.hpp:1495`)
  renders into `offscreen_color_view()` then tail-copies encoder-local
  into the staging layer; `promote_to_exhibition` copies staging →
  exhibition; `drain_gallery_promotions` runs after
  `render_snapshot_pass` (the O-7 order note).
- **Reader census for `offscreenColorTexture_`: the tail copy must be
  its ONLY reader.** FLAG if any other consumer exists.
- Viewport/scissor/aspect handling inside the snapshot pass: record
  it; it must be preserved verbatim against the new target.

Edits:
1. `exhibitionTexture_` usage += `RenderAttachment`.
2. The snapshot pass's color attachment becomes a **single-layer 2D
   view of the exhibition layer** being captured, `loadOp Clear /
   storeOp Store` — the Clear IS the full-layer-overwrite invariant;
   move the invariant's banner sentence from `promote_to_exhibition`
   to the pass site (one fact, one home).
3. Delete the tail `CopyTextureToTexture`, the snapshot branch of
   `promote_to_exhibition`/`drain_gallery_promotions` (slot
   bookkeeping — layer/aspect/valid — now marks at capture), the
   `snapshotStagingTexture_` allocation, and `offscreenColorTexture_`
   if the reader census cleared it (offscreen **depth** stays — the
   pass needs it).
4. `[GPU Budget]` will report the drop by itself (it is a census, not
   a tally). Expected: −32 MiB, −33 if offscreen color dies.

Behavior: none — same pixels, one hop instead of three. The authored
pool and its promotion path are untouched (DOMESDAY_2's ground).

### B9 — the parameter surface (LANTERN's deferred U2): `?seed= ?mood= ?cap=`

**Census gate first:** read `audit/LANTERN_CENSUS.md` §L2 in full. Its
recorded constraints outrank this spec — FLAG with the quote on any
conflict. The ruled purpose stands: **measurement first; boot-read;
no mid-run mutation; invisible to ordinary visitors.**

Design (web: parse `location.search` once, before device request, via
one EM_JS/EM_ASM read; native: `--seed= --mood= --cap=` argv):
- `seed` — u32, overrides the drawn boot seed
  (`[World] Boot seed=… (drawn)` becomes `(param)`).
- `mood` — integer index into the existing mood table, forces the
  boot mood (census how boot mood is chosen today and override at
  that exact site).
- `cap` — float, clamped to [0.5, 3.0], overrides
  `MAX_DEVICE_PIXEL_RATIO` (console.hpp) for this run. **This is the
  soak walk's key and the frame's largest lever** — it lets Jean
  price cap 2.25 against the purse on the audience device.
- Absent/invalid → silently ignored; present → one boot line
  `[Params] seed=… mood=… cap=…`. No UI, no mid-run reread.
- Document in the report the canonical soak arms as (seed, mood)
  pairs — data, not code.

### B6 — `shadow_slot` becomes immediate data (R3) — LAST, the API-frontier unit

**Why last:** exact spellings live at the edge of the port. Statute
names: WGSL `var<immediate>`, pipeline-layout `immediateSize`, pass
`setImmediates`. Dawn C++ convention: `SetImmediates(offset, data,
size)`, `PipelineLayoutDescriptor::immediateSize`. A3 proved the
port's `Limits` carries the field and the Pixel grants 64; if glaw1
rejects a spelling, FLAG with the error text — it IS the correction —
and do not guess twice.

Census gates:
- `shadow_slot` today: g1:2, uniform, 16 B `ShadowSlot`, vis V, the
  program's **only** dynamic-offset binding (`0d-1`). Read the struct:
  if one meaningful field (the light index), the immediate is a bare
  `u32`; else carry the struct (≤64 B).
- WGSL readers: census every entry point reading `shadow_slot`
  (expected: the shadow VS family; FLAG if a fragment/compute reader
  appears).
- CPU side: the per-light bind with dynamic offset in
  `render_shadow_pass` (render_passes.hpp:325/367) and the
  `shadowSlotBuffer_` creation + strided writes.

Edits:
1. WGSL: delete the g1:2 declaration; add
   `var<immediate> shadow_slot: u32;` (or the struct); readers
   unchanged in expression form.
2. Schema: delete the `shadow_slot` DECLS/REGISTRY/SEATS rows
   (`--check`, `--write` — Frame R Layout loses entry 1; registry
   82→81).
3. Pipeline layouts of every pipeline whose module reads the
   immediate: set `immediateSize` accordingly (others stay 0).
4. `render_shadow_pass`: replace the dynamic-offset rebinds with
   `SetImmediates` per light; bind the Frame R group once, no offset
   array.
5. Delete `shadowSlotBuffer_`, its writes, and the 256-B stride
   arithmetic — the whole dynamic-offset machinery leaves the
   program (`dyn_u` reads 0/8 program-wide afterward; MANIFEST and
   the ledger regenerate and say so).

Budget effect: uniform −1 on every render (pipeline, V) row — main
family 9→8 of 12; the immediates lane reads its first nonzero.

### B-close — regenerate everything

Rerun `binding_gen.py --write`, `binding_ledger.py`,
`command_census.py`; commit regenerated artifacts as
`DOMESDAY_1 B-ledger: regenerate`.

---

## THE REPORT — `audit/DOMESDAY_1_REPORT.md` (final commit, master)

1. Unit table (A7, A8, A9-fix, A9-wit, B5–B9, B-close): LANDED /
   FLAGGED / SKIPPED, with shas.
2. **The ruling record R1–R6 verbatim from this handoff's header**,
   marked *delegated by Jean, ruled by Claude, enacted here*.
3. The held list including MSAA with its snapshot coupling sentence.
4. Wallet movement: expected end-state — render V uniform 8/12,
   storage ≤3/8; `dyn_u` 0/8; immediates lane first spend; estate
   −32/33 MiB (the budget print's own rows quoted).
5. Soak-arm table: the canonical (seed, mood) pairs for Jean's walk,
   plus the cap values worth pricing (1.5 baseline, 2.25 native).
6. Anything unexpected, one line each.

## AFTER CC — Jean's gates and the acceptance evidence

1. glaw1 + visual + merge on `claude/domesday-1` (B6 is the one unit
   that may bounce a spelling off glaw1 — its error text comes back
   to Claude, one-line fix follows).
2. One Pixel boot, console captured. It should say, all by itself:
   the feature names beside their ids · `[GPU Budget]` down ~32 MiB
   with `Snapshot Staging` gone · FRAME_1 firing once per settle, not
   in bursts · `dyn` machinery gone from nothing but our memory · and,
   on a test load with `?cap=2.25&seed=…&mood=…`, the `[Params]` line
   and the purse's answer to the only question left in the wallet's
   time column.

One line for the spirit: **the estate stops being surveyed and starts
being spent — first coin into the immediate lane, thirty-two megabytes
back to the phone, and the instrument panel for the walk that prices
what beauty the frame can still afford.**
