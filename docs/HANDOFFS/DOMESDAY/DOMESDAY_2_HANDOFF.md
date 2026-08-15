# DOMESDAY_2 — DEEDS, CLOSURE, AND THE WALK'S LAST INSTRUMENT
## Handoff to CC · authored by Claude · Jean holds the gates

The closing batch. No earned migrations remain — the tree refused R5's
both halves (unified-memory argument, recorded below), so _2 completes
the instruments, writes the deeds, ships the one remaining measurement
affordance, and closes the campaign onto the panel's ground.

**Protocol:** FLAG-AND-CONTINUE, as _0/_1 (per-unit census gates; a
failed gate FLAGs, reverts that unit, continues; one unit = one commit
or zero; dependents cascade-skip; the report always lands). L1
encoding law throughout.

**Global gates (stop only here):**
1. Working tree clean.
2. **Merge-state verification (codified from _1's precedent):** if
   `claude/domesday-1` is fully green but unmerged into master, merge
   it as the enacting step (no-conflict expected; record the sha).
   Jean's send of the post-batch console is the standing
   authorization. Then proceed on merged master.
3. `binding_gen.py --check`, `binding_ledger.py --check`,
   `command_census.py --check` all green before any edit.

## RULING RECORD ADDENDUM (write into the _2 report verbatim)

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

---

## STREAM A — master direct. Prefix `DOMESDAY_2 A<n>:`

### A10 — MANIFEST's immediates column made true

The lane took its first coin (B6) but the schema carries no
immediate-size fact, so MANIFEST still prints 0/64 (named gap, _1
report §6).

- `binding_schema.py`: PIPELINES rows gain `immediate_size` (bytes,
  default 0). Bootstrap the values from the tree: the pipeline-layout
  creation sites' `immediateSize` argument (the optional fourth
  `strataLayoutFor` arg the ledger's parser was taught in B6 — reuse
  that parse).
- `binding_gen.py`: capture on `--bootstrap`-style read, verify on
  `--check`, and both emitters (MANIFEST lane table + wallet; ledger
  Table B) print the real per-pipeline bytes.
- **Witness M-2:** a pipeline's `immediate_size` is nonzero **iff**
  its module set statically accesses a `var<immediate>` declaration
  (cross-check against the WGSL census). Expected today: the shadow
  family nonzero (4 B), all others 0.

### A11 — the RESOURCES relation (two commits: subject, then relation)

The graph's last open edge: seats point at backings the schema does
not own. Close it — "no dead ends," literally.

- **A11-fix (subject):** the label law extends to buffers and
  textures. Census every `CreateBuffer` / `CreateTexture` /
  `makeTextureArray`-style creation in `state.hpp` (and any strays the
  grep finds elsewhere); every descriptor carries a `label` derived
  from the owning member name. Most already do (the budget print names
  them); fix the unlabeled remainder. Mechanical strings.
- **A11-rel (relation + witnesses):** `binding_schema.py` gains
  `RESOURCES`: one row per GPU object —
  `{member, kind (buffer|texture), size_expr or (format, dims,
  layers, samples), usage_flags, label}` — bootstrapped from the same
  creation sites, verbatim expressions (Dim:: symbols, not evaluated
  numbers). `binding_gen.py --check` gains:
  - **R-1:** every SEATS backing (the buffer/view member each
    bind-group entry names) resolves to exactly one RESOURCES row;
    every pass-attachment view in COMMAND_LEDGER's rows resolves
    likewise.
  - **R-2 (orphan detection):** every RESOURCES row is reached by at
    least one of {bind-group entry, pass attachment,
    copy/write site (grep `CopyTextureToTexture`, `CopyBufferTo`,
    `WriteTexture`, `WriteBuffer` receivers)}. Expected orphans:
    **zero**; any found are FLAGGED in the report, not deleted —
    deletion is a ruling.
  - **R-Label:** every RESOURCES row carries a label (pairs with
    A11-fix).

### A12 — the floors get one home (granted-vs-used, closed)

The boot's granted-vs-floor line hand-carries six floor values
(2048, 225, 8, 12, 65536, 4). Give them one home:

- Schema gains a small `NEEDS` table: each floor value with its source
  expression (e.g. `maxTextureArrayLayers: 225 =
  Dim::MAX_ACTIVE_PATCHES`; where a value is a Core default the
  program merely relies on, say so).
- `binding_gen.py --write` emits `limits_floor.gen.inc`; the boot
  print (console.hpp) consumes it — the six literals leave the C++.
- **Witness R-3:** emitted values == schema values; and where a NEEDS
  row cites a `Dim::` symbol, the symbol exists.
- This closes LANTERN's print-vs-enforce hazard: the log now reads
  the wallet's own statement of need on the actual device.

### A13 — deeds and truthing (one commit)

1. **Deed the snapshot pool** — prose at `snapshotStagingTexture_`'s
   creation (state.hpp):
   `// DEEDED: the photographer's portfolio. 32 speculative shots
   persist here between capture and hang-time curation; retiring this
   pool changes which shots hang. Do not retire without a
   photographer-model ruling (DOMESDAY R5 withdrawal, _1 report B8).`
2. **Deed the authored pool** — at `authoredStagingTexture_`:
   `// DEEDED: the painting inventory. Prefetch-decoded canvases wait
   here across world rebuilds; on unified memory a CPU-side inventory
   costs the same bytes, and decode-at-hang regresses the transition.
   Re-open only via a browser-owned (ImageBitmap) path on a vendored
   port (DOMESDAY R5 withdrawal).`
3. **Truth the immediate row's annotation:** the granted-vs-floor
   print still says `maxImmediateSize=…/0 (floor — unused)`; the lane
   now carries 4 B. Drop the annotation to `(floor)` — MANIFEST owns
   spend truth after A10. (Grep the literal `floor — unused`.)
4. **feature_name gains three spec-cited cases** (identifiers only,
   values from the compiling header, glaw1 prunes):
   `PrimitiveIndex`, `TextureComponentSwizzle`, `SubgroupSizeControl`
   — the spec's GPUFeatureName enum lists all three; the Pixel's
   unknown ids 21/22 are expected to resolve to the first two by
   enum order.
5. **Truth the FRAME_1 comment:** `// FRAME_1 (temporary)` →
   `// FRAME_1 — debounce witness; retire after the soak walk
   confirms single-fire per settle.`

---

## STREAM B — branch `claude/domesday-2`. Prefix `DOMESDAY_2 B<n>:`
*Held for Jean: glaw1 + visual + merge. Expected visual delta at
default settings: none — B10's default is current behavior,
byte-identical pipelines.*

### B10 — `?msaa=` — the walk's last instrument

Extend the boot-param surface (B9's `boot_params.hpp`) with
`msaa ∈ {1, 4}` (web `?msaa=`, native `--msaa=`; anything else → 1;
`[Params]` line reports it). Boot-read, pipelines created once with
the value — no mid-run mutation, per the surface's law.

**Census gates:**
- Locate the shared `GPUMultisampleState` (or per-pipeline
  `multisample.count`) at the render-pipeline creation sites;
  confirm the shadow family's pipelines are creation-separate (they
  are — `shadow*Pipeline_` members) and that non-shadow render
  pipelines draw **only** into the main and snapshot passes
  (COMMAND_LEDGER §1: passes 7 and 16). FLAG any non-shadow render
  pipeline reachable from another render pass.
- Backbuffer format resolve-eligibility is statute (bgra8unorm:
  multisampling ✓, resolve ✓).

**Edits (active only when msaa=4; msaa=1 leaves every descriptor
byte-identical):**
1. Main pass: create `msaaColor` (canvas-sized, count 4,
   RenderAttachment) and recreate the console depth at count 4 —
   inside `createDepthBuffer`'s existing recreate path so the
   debounce owns resizing. Pass 7's color attachment becomes
   msaaColor with `resolveTarget = backbuffer`, `storeOp: Discard`
   (resolve is independent of storeOp — tiler-ideal); depth stays
   Clear/Discard at count 4.
2. Snapshot pass: `offscreenMsaaColor` (512², count 4) with
   `resolveTarget = offscreen_color_view()`, `storeOp: Discard`;
   `offscreenDepthTexture_` created at count 4. The portfolio copy
   chain downstream is untouched (it reads the resolved
   single-sample offscreen color, exactly as today).
3. `multisample.count = msaa` on every **non-shadow** render
   pipeline; shadow family stays 1.
4. The `[GPU Budget]` census will report the msaa allocations by
   itself when active; nothing to hand-tally.

**Purpose (report language):** measurement affordance. Default stays
1 until the soak walk prices the matrix; the default flip afterward
is one constant, under Jean's principle — *impress all who can open
the site.*

### B-close — regenerate everything; commit as
`DOMESDAY_2 B-ledger: regenerate`.

---

## THE REPORT — `audit/DOMESDAY_2_REPORT.md` (master)

1. Unit table with shas (A10, A11-fix, A11-rel, A12, A13, B10,
   B-close).
2. The R5-withdrawal addendum verbatim.
3. Wallet final state (post-A10 MANIFEST: the immediates column's
   first true nonzero row).
4. Orphans found by R-2 (expected none).
5. **The soak-walk matrix**, ready to hand:
   arms S0–S3 (seed 42, moods 0–3) × cap {1.5, 2.25} × msaa {1, 4} —
   e.g. `?seed=42&mood=2&cap=2.25&msaa=4` — with the meter's 30 s
   window per cell. Two purse questions, one walk.
6. Anything unexpected, one line each.

## DOMESDAY_CLOSE.md — the campaign's headstone (master, final commit)

One page, citing artifacts, no new claims:
- **Surveyed:** bindings (LOOM's ground, extended), passes and
  submits (COMMAND_LEDGER), stagings (A5), laws (A6), objects and
  backings (RESOURCES), floors (NEEDS).
- **Moved:** two demotions, one vertex-pull, one immediate (the
  dynamic-offset machinery deleted whole), four groups → one, the
  debounce; wallet from render V 7u/6s to 8u/3s… quote MANIFEST.
- **Deeded:** heightfield `.a` (standing), snapshot portfolio,
  authored inventory, the paintings fence (standing).
- **Declined / withdrawn / refused by the tree:** orb swap (L23′);
  R5 both halves; B4 found pre-paid (DISCARD_0).
- **Standing witnesses at every landing:** LOOM's set + M-1, M-2,
  C-1…C-7, R-1, R-2, R-3, R-Label.
- **Held, named:** caster instancing · stream_patches amortization ·
  render bundles · wider indirect · MSAA default (pending the walk) ·
  ImageBitmap inventory (pending vendored port) · feature ids beyond
  the switch (glaw1-arbitrated as the port moves).
- **Handover:** the panel campaign opens on the agents family's
  11/12 uniform row — relief by coalescence into few wide generated
  blocks (tile_grid 16.4 KB the anchor tenant), every block born a
  RESOURCES citizen. Nothing in the GPU logistic is unowned,
  unlabeled, or unwatched.

## AFTER CC — Jean's part

1. glaw1 + visual + merge on `claude/domesday-2` (default face
   byte-identical; the msaa=4 face is gated behind the param).
2. **The soak walk**, phone in hand, matrix in §5 of the report —
   the purse's two open questions (native density; edge quality)
   answered in one pass. Send the numbers; the default-face ruling
   follows, then FRAME_1 retires and DOMESDAY is closed.

One line for the spirit: **the survey ends with every holding deeded
or earning, every object owned, and one instrument in Jean's hand —
what the frame can still afford is now a question with an address.**
