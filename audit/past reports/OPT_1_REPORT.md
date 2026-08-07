# OPT_1 + C6 — CAMPAIGN REPORT

Closes the two WEB handoffs (`src/docs/HANDOFFS/WEB/C6_8FIT_HANDOFF.md`,
`OPT_1_HANDOFF.md`). Every claim carries file + symbol at HEAD. Absences are
stated plainly and were verified over whole files (P11). Line numbers are
hints; the symbols are the authority (P2).

**Both handoffs were largely executed before this session.** What this report
adds is the O0 recensus the campaign demanded and never filed, the ruling on
the one conditional unit, the C6 gate re-proof against a HEAD that moved
thirty-nine commits after C6 landed, and three follow-on units: two sweeping
residue the landed commits left behind, one tagging a defect the recensus
found.

---

## HASHES

| unit | commit | state |
|---|---|---|
| C6 the 8-fit | `af839dd` | LANDED (merged at `6e86da3`) |
| OPT_1a card-writer rest skip | `bd4f1f2` | LANDED |
| OPT_1b pregen radius 7 | `bb0b68d` | LANDED |
| OPT_1e LOD1 clean prefix at rest | `599aca5` | LANDED (unit invented post-handoff) |
| **OPT_1c indoor eye gate** | — | **SKIP — the finding dissolves (§O0-f)** |
| **OPT_1d R-5 cell-granular LOD1** | — | **NOT STARTED — no stamp (§STOPS)** |
| **OPT_1 residue sweep** | `09838e2` | THIS SESSION |
| **OPT_1 P6 witnesses** | `918ed2e` | THIS SESSION |
| **OPT_1 inert cull knob tagged** | `6e82aba` | THIS SESSION (L9; the cut is Jean's) |

Base: `4c1a804` (`origin/master`, confirmed by `git fetch origin master` in the
same command sequence — P9; the fetch reported a forced update from a stale
cached `cab1a0f`, so the label would have lied).

---

## O0 — THE RECENSUS, IN FULL

### O0-a — the segment plan is live

L-1 is live exactly as the amendment describes. `FC_ARGS_BYTES = 15 *
sizeof(uint32_t)` (`state.hpp`) — three 5-u32 `DrawIndexedIndirect` arg slots.
`reset_frustum_indirect` writes all fifteen words:

```
patchIndexCount_,               0,0,0,0,   // A  full IB (zone-overlapped)
patchIndexCountCapOnly_,        0,0,0,0,   // B  cap-only IB (clean LOD0)
patch_index_count_lod1_live(),  0,0,0,0,   // C  LOD1 IB
```

Draw sites: `render_main_pass` (`render_passes.hpp`), three
`draw_patch_terrain_plan_slot` calls at indirect offsets **0 / 20 / 40**, each
on its own plan window (`render_entity_group()` / `_plan_b()` / `_plan_c()`)
and its own index buffer. The instanceCounts are the cull kernel's three
atomics (arg indices 1 / 6 / 11); the indexCounts are CPU-authored.

Zone-rect classification: `dispatch_frustum_cull` packs up to 8 active zone
world rects into `GPUDrawPlanParams` (`plan.rects[n]` = corner_x, corner_z,
extent_x, extent_z) beside `lod0_count` / `render_count`, uploads them, resets
the args, dispatches the kernel, then copies compute → indirect (Dawn D3D12
cannot share `Storage|Indirect`).

OPT_1e's change to slot C: only the **count** moves. The clean count is a
prefix of the zoned IB, so buffer and count cannot split. No WGSL moved.

### O0-b — UMBRA is live, and the spot-terrain kill with it

`draw_shadow_all(c, pass, cast_terrain)`. Terrain casts under
`if (cast_terrain)` only, and:

- `render_shadow_pass` calls it `/*cast_terrain=*/false` for **every** spot
  atlas tile, `/*cast_terrain=*/true` for the sun pass. The spot-terrain kill
  is live.
- **Both** sun bands draw `patch_index_buffer_lod1()`. Band 0 via
  `draw_shadow_patch_terrain(..., patch_index_count_lod1_live(),
  lod0_patch_count)`; band 1 via a bare `DrawIndexed` on the same already-bound
  IB. So a caster's silhouette never re-tessellates as the camera nears it.

Both sun draws take OPT_1e's live count, and R17 FrustumCull precedes R18
ShadowPass in the spine, so the flag is fresh for both same-frame.

### O0-c — the card writer, post-OPT_1a

The two dispatches and their arithmetic, confirmed: `LIVE_CARD_SIZE = 640`;
heights kernel (workgroup 8) = (640/8)² × 64 = 409,600; resolve kernel
(workgroup 16) = (640/16)² × 256 = 409,600. **819,200 invocations per frame**,
as the handoff stated.

The gate is at dispatch; the kernels are untouched. The state machine in
`phase_live_card_write` is correct:

| card_live | liveCardRestClean_ | action | flag after |
|---|---|---|---|
| true | either | write | false |
| false | false | **write once** (the clearing write) | true |
| false | true | **skip** | true |

Boots `false`, so frame 1 always writes. Reset to `false` at TEARDOWN beside
`world_gen++`, so a fresh world writes its rest field once even if no zone ever
goes live there. **No path sets the flag clean while skipping the write** —
the only assignment to `true` is on the same branch that falls through to
`dispatch_live_card_write`. Consumers cannot read a stale non-zero texel.

### O0-d — the pulse ring is driverless; O0-e — terrain_time is pinned

Both absence claims verified over the whole tree, untruncated (P11).

**Pulses.** `config_.pulse_count` has exactly two writers: the boot zero-pin
(`state.hpp`, `config_.pulse_count = 0` beside `pulse_data[i] = 0.0f`) and the
setter `set_pulse_data(count, data)`. `set_pulse_data` has **exactly one call
site in the live tree** — `cartridge.hpp`, the boot block, with
`terrain_looks::REST_PULSE_COUNT` and a `zero_pulses` array. The archived
charter already recorded the ring as **DRIVERLESS**; it still is.

**terrain_time.** Writers: the boot zero-pin (`config_.terrain_time = 0.0f`)
and `set_terrain_time`, whose one caller passes
`terrain_looks::REST_TERRAIN_TIME`, and `REST_TERRAIN_TIME = 0.0f`. Still
exactly the boot-pin-to-zero the handoff expected (F-4b unchanged).

**Consequence.** The rest law's two musical conjuncts are structurally
satisfied, so OPT_1a's three-conjunct predicate reduces at runtime to *"any GoL
zone live"*. That is ECONOMY_1 E1's exact predicate — which is why it now
carries a witness (§THE THREE NEW UNITS).

### O0-f — the indoor eye side: THE FINDING DISSOLVES

This is the unit's whole hinge, and the answer is exact rather than
approximate. Three separate premises of OPT_1c are false at HEAD.

**(1) Indoors is NOT unculled.** `dispatch_frustum_cull` opens with its own
disclaimer: *"the kernel runs in EVERY mood now — the finite/indoor path draws
through the plan too, so the old indoor skip is retired with the direct path it
fed."* `render_main_pass` draws the three-slot plan unconditionally. Per-patch
frustum culling is already delivered indoors, and by an actual frustum test
rather than a wall heuristic. (Retired by `f460469`, 2026-08-03 — two days
*before* the handoff was committed at `14ead11`. The premise was already stale
when it was written, exactly as the handoff's own REGISTER warned about every
LEDGER_1 body reference.)

**(1b) — and the flag that says otherwise is INERT.** Chasing premise (1)
surfaced a live defect. `MOOD_TABLE` carries an `allow_frustum_cull` column,
**false for both indoor rows**, and `apply_mood` pokes it into the renderer via
`set_frustum_cull_active(m.allow_frustum_cull)`. It lands in
`useIndirectTerrainPipeline_` — **and nothing reads it.** Verified over the
whole tree, untruncated (P11): `use_indirect_terrain()` has *zero* call sites
in `src/`; its one former reader was `render_passes.hpp`'s `if
(!c->renderer_.use_indirect_terrain()) { return; }` early-out, which LEDGER_1
recorded and `f460469` deleted. (`draw_patch_terrain_direct` survives with one
live caller — the photographer's snapshot pass, which the main pass already
names as the one path that cannot read the plan.)

So a reader of `MOOD_TABLE` concludes indoor terrain is not culled. It is. The
knob has been decorative since `f460469`, and the two `false` rows are held in
place by column-drift static_asserts that pin a value nothing consumes. Tagged
`STATUS: LATENT[mood_cull_opt_out]` at both rooms in `6e82aba`; the cut is
Jean's (§FOR JEAN).

**(2) Indoors is NOT all-LOD0.** `band_patches` bypasses only the **ring**
test in finite mode; the `d2 <= lod0_sq` LOD split still runs. With
`finite_radius` 4 the room's corners sit ~318 wu out against
`LOD0_RADIUS_DEFAULT` 175, so they band to LOD1 like anywhere else.

**(3) Nothing is behind a wall.** The decisive arithmetic. Both the resident
set and the walls are built from the *same two expressions*:

```
resident grid box   in_render_window: [cx-R, cx+R]²,  cx = last_center_x = 0
                    (finite: stream_patches pins centerX/centerZ = 0 and caps
                     active_radius to finite_radius R)
patch i spans       [i·PATCH_EXTENT, (i+1)·PATCH_EXTENT]
=> footprint        [-R·50, (R+1)·50]²

walls               phase_stage_world / generate_indoor_shell:
                      bmin = -(float)finite_radius * Dim::PATCH_EXTENT
                      bmax = ((float)finite_radius + 1.0f) * Dim::PATCH_EXTENT
                    four quads at exactly x=bmin, x=bmax, z=bmin, z=bmax
=> shell            [-R·50, (R+1)·50]²
```

**The wall plane IS the outer edge of the outermost resident patch, on all four
sides, for every R.** Not "close to" — the same expression, character for
character, in `cartridge.hpp`, `mood.hpp` and `spawn_engine.hpp`. The shared
asymmetry proves it: both are centred on +25 wu, not on the origin, because
both index the cell that spans [0, 50].

`in_render_window` also gates **eviction** (`patch_system.hpp`), so a patch
outside the box is actively removed, not merely not allocated. The resident set
cannot exceed the wall footprint even transiently.

**RULING: SKIP. Zero of the (2R+1)² resident patches is provably hidden;
every one of them is visible floor.** The handoff's own escape clause applies
verbatim: *"If O0-f shows every resident patch IS visible floor, report and
skip this unit — the finding dissolves."*

**The ruling was stress-tested against every branch that could overturn it, and
none does:**

| branch | walls? | ruling |
|---|---|---|
| `MOOD_INDOOR_FLAT` (finite, indoor, ceiling FLAT) | yes, at bmin/bmax | footprint == walls → skip |
| `MOOD_INDOOR_VAULT` (finite, indoor, ceiling VAULT) | yes, at bmin/bmax | same; the vault only changes the ceiling, `vault_crown` takes the same bmin/bmax |
| `MOOD_FINITE_OUTDOOR` (finite, **not** indoor, ceiling NONE) | **none** — `apply_mood_indoor_shell` takes the `clear_indoor_shell` arm | nothing to hide behind at all; not an OPT_1c target either |
| `MOOD_OPEN_SUNSET` (not finite) | none | ring test applies normally; out of scope |
| `ROSTER.indoor_shell = false` (the `minimal` demo column) | **none** — `apply_mood_indoor_shell` is `if constexpr`-gated out | a fortiori: no walls, nothing hidden |

Default build is `INCUBATE_DEMO=full` → `ROSTER.indoor_shell = true`, so the
walls-present rows are the live case. In every row the answer is the same, and
in the two rows without walls it is the same *more strongly*.

And the handoff's fallback — *"or restore the ring test with
`finite_radius`-derived bounds"* — would be **actively harmful**.
`phase_stage_world` sets `veil_strength = 0` in finite mode with the ruling
written beside it: *"THE VEIL (ruled): OFF in finite/indoor — walls define the
boundary there, not fog (the same law that makes all patches visible in finite
mode)."* The ring bypass in `band_patches` and the veil-off in
`phase_stage_world` are **one ruling in two places**. A ring test indoors cuts
floor with no fog to hide the cut — it would produce exactly the artifact the
ruling exists to prevent.

### O0-g — has_mode_bias IS reachable. The handoff's hypothesis is FALSIFIED.

The handoff expected E3 to be unreachable post-CUT_1 (*"likely unreachable
post-CUT_1 (O0-g decides)"*). It decides the other way.

`has_mode_bias` (`world.wgsl`, the terrain fragment path) is a four-term OR:

| term | writers in the live tree | verdict |
|---|---|---|
| `config.mode_color_shift` | `set_mode_color_shift` — **one** caller, the boot rest-pin | driverless |
| `config.mode_checker_scatter` | `set_mode_checker_scatter` — **one** caller, the boot rest-pin | driverless |
| `config.mode_palette_intensity` | `set_mode_palette_drift` — **one** caller, the boot rest-pin | driverless |
| `config.checker_music_amount` | `set_checker_color_field` — boot rest-pin **AND `phase_motion_drivers`, every frame** | **LIVE** |

The live writer is the CHECKER-REBUILD flush in `phase_motion_drivers`: gated
on `checker_mean_dst_.valid && checker_var_dst_.valid`, it pushes
`cp.get(checker_var_dst_.base)` — `music_amount` — from the visual canvas each
frame. Post-CUT_1c the canvas is ticked by the BeatClock-fed signal spine, so
the amount can and does rise off zero.

**So the per-pixel `animated_cell_color` recompute is reachable through one
live term of four.** E3 is a real cost, not a dead branch. No edit here — the
handoff asked for a report and forbade one ("report, no edit") — but the entry
in CLOSED/DEFERRED that reads *"E3: likely unreachable post-CUT_1"* is wrong
and should not be carried forward as settled.

### O0-h — the radius facts, post-OPT_1b

```
PATCH_PREGEN_RADIUS  7        PATCH_EXTENT      50.0
PATCH_PREGEN_SIDE    15  = 2R+1
MAX_ACTIVE_PATCHES   225 = 15²          (was 289; default cap 256)
TILE_GRID_SIDE       17  = 2(R+1)+1     (was 19)
TILE_GRID_MAX        289 = 17²          (was 361; cap TILE_GRID_CAPACITY 1024)
EXIST_RADIUS         350 = 7.0 · 50     VEIL_RING_DEFAULT 325 = 6.5 · 50
LOD0_RADIUS_DEFAULT  175 = 3.5 · 50
```

Both texture creations are symbolic — `desc.size = { PATCH_HEIGHTFIELD_N,
PATCH_HEIGHTFIELD_N, Dim::MAX_ACTIVE_PATCHES }` and `viewDesc.arrayLayerCount =
Dim::MAX_ACTIVE_PATCHES` — for the heightfield array and the cell-colour array
alike. No hardcoded layer count survives in either room; `world.wgsl` carries
no 17/289 literal (`PatchGrid.entries` is runtime-sized, `TILE_GRID_CAPACITY`
is authored, not derived).

**The veil chain now holds with EXACTLY ZERO margin.** `static_assert
(PATCH_PREGEN_RADIUS * PATCH_EXTENT >= EXIST_RADIUS)` reads `350 >= 350`. The
draw set is genuinely unchanged — the ring (325) is strictly inside 350, so the
same patches are drawn — but one more ring off the radius breaks the assert,
loudly, at compile time. That is the right failure mode; it is worth knowing it
is one step away.

**Nine prose sites still carried radius-8 numbers.** Five were real and are
fixed in `09838e2` (§THE THREE NEW UNITS); four were checked and correctly left
alone, and the non-findings matter as much as the findings:

- `TILE_GRID_MAX // 289` is **right**, by coincidence — 17² equals the old
  `MAX_ACTIVE_PATCHES`. A grep flags it; the arithmetic clears it.
- `renderer.hpp` `"was hardcoded 4 = 256 threads vs 289 slots"` is marked
  history (audit CC-8a) and reads as history.
- the `world.wgsl` floater eviction margin was already re-pinned by OPT_1b.
- `state.hpp` `"floaters 400"` sits beside `FLOATER_EVICTION_RADIUS = 800.0`,
  but the clause **pre-dates** OPT_1b, so it is flagged, not cut (§FOR JEAN).

---

## C6 — THE FOUR GATES, RE-PROVED AT HEAD

C6 landed at `af839dd` and **thirty-nine** commits landed after it
(`git rev-list --count af839dd..4c1a804`). All four gates still pass.

**Gate 1 — alignment.** The room bind group's `entries[2]` sets `.buffer =
headPosesBuffer_` and `.size = sizeof(float) * 4 * Dim::RIBBON_MAX_RINGS`
(= 4 × 4 × 400 = **6,400 B**) and **sets no `.offset`**, so the offset is 0.
0 ≡ 0 mod 256 — `minUniformBufferOffsetAlignment` satisfied trivially, because
the "window" is the whole buffer. 6,400 B is 9.8% of the 64 KiB uniform cap.

**Gate 2 — reader/binding census.** Exhaustive over the tree.
`headPosesBuffer_` is bound at exactly two sites, and they are different
`(group, slot)` pairs — the documented occupier-window pattern (L6 item 4):

| site | binding | layout type | WGSL decl |
|---|---|---|---|
| Ribbon Compute BindGroup | `g0::head_poses` (122) | `ReadOnlyStorage` | `@group(0) @binding(122) var<storage, read> head_poses` |
| Room BindGroup (g2) | `g2::field_head_poses` (2) | `Uniform` | `@group(2) @binding(2) var<uniform> field_head_poses` |

Exactly one module-scope declaration per site; no third declaration aliases the
region. `field_head_poses` has one reader, in the field kernel's emitter loop.
The buffer's usage flags carry **both** address spaces —
`Storage | CopyDst | Uniform` — which is what makes the split legal.

**Gate 3 — the counts.** Room-family compute-stage layout entries, g2:
`occupier_cmg` (ReadOnlyStorage), `occupier_amg` (ReadOnlyStorage),
`field_head_poses` (**Uniform**), `field_forces` (Storage, read_write),
`field_ribbon` (Uniform), `field_authored` (Uniform). Storage entries across
the family: **8 of the 8 core default** (was 9). Uniforms: **8 of 12**, the
note's 7 plus the demoted one, exactly as the handoff predicted. L2 item 4 and
L14 satisfied with zero headroom on storage and four uniforms to spare.

**Gate 4 — writer path.** `headPosesBuffer_` is written only by
`queue.WriteBuffer(headPosesBuffer_, 0, data, bytes)` in
`upload_ribbon_head_poses`. Both WGSL declarations are read-only
(`var<storage, read>` and `var<uniform>`); no compute kernel writes the
binding; no `CopyBufferToBuffer` targets it. CPU-written, never GPU-written —
so uniform usage on the region is legal.

Registry and layout comments (`binding_registry.hpp` g2:2, `state.hpp` layout
entry, buffer creation) all state the demotion correctly. No P5 drift.

**Exceedance 1 is closed. Exceedance 2 closed with OPT_1b. L14's two numbers
— storage 8/8, texture array layers 225/256 — are both true at HEAD.**

---

## THE THREE NEW UNITS

### `09838e2` — OPT_1 residue: the radius-8 numbers left in prose

Five surviving restatements of the pre-edit radius. Comments only, no
behaviour.

The one that mattered is **THE WINDOW COVENANT** — the margin a future author
consults before touching `LIVE_CARD_EXTENT` or the radius, and every number in
it was one ring stale:

| | was (R=8) | is (R=7) |
|---|---|---|
| allocation reach `(R+1)·PATCH_EXTENT` | 450 wu | **400 wu** |
| with the snap | 453.125 | **403.125** |
| guaranteed half-extent +x/+z | 496.875 | 496.875 |
| **SLACK** | **43.75 wu** | **93.75 wu** |

OPT_1b more than doubled the covenant's headroom and the comment still recorded
the tight figure. The reach is now written as `(PATCH_PREGEN_RADIUS + 1) ·
PATCH_EXTENT` — P5's de-numbering applied to the *derivation* rather than the
result, so the next dial move cannot leave it stale again.

Also: the veil-chain narration claimed *"the pregen ring now reaches 400 — the
deepened buffer"* where the tree has 350 and **no slack at all** (§O0-h) —
restated as tight, since a comment claiming margin the tree does not have fails
in the dangerous direction. And the TILE_GRID `side` gloss at three sites, all
saying 19; two of them are an **L3 mirror pair** (the `GPUTileGrid` DTO and its
`world.wgsl` twin) and therefore moved in one commit, as L3 requires.

L1 held: `world.wgsl` still BOM-free and LF-only.

### `918ed2e` — OPT_1 P6: the two rest switches get their witnesses

Both landed units introduced a runtime switch that selects behaviour, and
neither was fully witnessed. P6 is a numbered law that names this failure, so
this closes it rather than reporting it.

| unit | switch | had | missing |
|---|---|---|---|
| OPT_1a | `liveCardRestClean_` | nothing | transition log **and** boot line |
| OPT_1e | `zonesActiveAnywherePrev_` | transition log | boot line (the P6 corollary) |

**OPT_1a's gap is the one with teeth.** Its entire claim is that 819,200
invocations per resting frame stop happening, and nothing in the log
distinguishes *"the skip fires"* from *"the skip is inert"*. Per O0-d the
predicate reduces to "any GoL zone live" — which is ECONOMY_1 E1 verbatim: the
flag was correct, the invariant was sound, the arm never released because zones
are alive globally almost always, and the boot looked like a pass.

**OPT_1e's gap** is the corollary half the Release boot paid for: the memory
boots to 0 and the gate prints on change only, so a boot with no zones prints
nothing, and *"no transition"* and *"no witness"* are again indistinguishable.
It sat two lines from the site that does it correctly.

The edit: `live_card_is_live()` as the rest law's one home (P7 — a handoff that
names a print has named the home of the thing printed; the minimal form is one
extraction, not a second copy), behaviour-identical to the inline predicate;
`live_card_state_label(bool)` so the boot line and the transition line cannot
describe one state in two vocabularies; and all three switches seeded and
printed in the existing P6-corollary block at the foot of init, each from the
same function its gate reads.

The boot log gains two lines:

```
[Ground] zone rects in core: N (boot)
[Ground] zones active anywhere: N (boot)                                    <- new
[Card] live-card field: AT REST — one clearing write, then skipped (boot)   <- new
```

### `6e82aba` — OPT_1 O0-f: the inert cull knob, tagged (L9)

The recensus finding of §O0-f 1b, recorded where a reader will hit it:
`STATUS: LATENT[mood_cull_opt_out]` on the `MoodProfile` column and on the
renderer's setter/getter pair, each naming the other and naming the full cut.

**Tagged rather than cut, deliberately.** The cut is five sites — the
`MoodProfile` column, its two column-drift static_asserts, the `apply_mood`
poke, and the renderer pair — in a table whose own comment says a column added
or cut mid-row *"shifts every field after it with no diagnostic"*, and
`allow_frustum_cull` is currently the **tail probe**, so removing it means
re-homing that witness onto whatever becomes last. That edit is correct to make
and wrong to make blind. CC never builds; this is exactly the class of edit
those witnesses exist to catch. It goes to Jean with the sites enumerated
(§FOR JEAN) rather than being taken here.

L9's rider is acknowledged in the tag: a tag buys one reading, not permanent
residence. The handle `mood_cull_opt_out` is chosen so every site of the
capability greps together when the ruling lands.

---

## SAVINGS — PREDICTED vs STRUCTURAL

Structural = what the tree now provably does. Nothing here is measured; the
METER pair below is where measurement goes.

| unit | predicted | structural, verified |
|---|---|---|
| C6 | storage 9 → 8 | **8/8.** Zero headroom on the core default; the room family fits with no adapter grant. Behaviour pixel-identical (address space only). |
| OPT_1a | 819,200 invocations/resting frame | **819,200 exactly** — (640/8)²·64 + (640/16)²·256, both dispatches gated. Realised only while no GoL zone is live anywhere (O0-d); the new witness is what tells you whether that is ever true. |
| OPT_1b | 289 → 225 layers | **−64 layers.** Heightfield 256²×8 B = 524,288 B/layer → **−32.0 MiB**; cell colour 16²×4 B = 1,024 B/layer → **−64 KiB**. Total ≈ **−32.06 MiB**, −22.1%. Draw set unchanged by construction; what thins is pregen hysteresis, one ring of stream-in lookahead. |
| OPT_1e | LOD1 6,912 vs 19,200 indices | **−64.0% of LOD1 indices at true rest**, on the eye pass's slot C and both sun bands. Exact at rest, not approximate. |
| OPT_1c | — | **0.** The finding dissolves (§O0-f). |
| OPT_1d | −27.8% LOD1 indices (6,912 → 4,992) | not started (§STOPS). |

## METER — the delta table (Jean fills)

Phase 0 was never captured; these rows are the campaign's ledger and they are
still open. Build with `T7_INSTRUMENTS`, run machine-clean.

| scene | METER_1 before | METER_1 after | adapter line |
|---|---|---|---|
| outdoor rest | | | |
| outdoor, live GoL zone | | | |
| indoor default mood | | | |

The adapter line answers LEDGER_1's F4-1 gap (which adapter the milliseconds
belong to). Chrome rows sit beside these if first light has run. **Every CPU
number recorded before 2026-07-29 is Debug-inflated and retired** (the DAWN
RELEASE BUILD scheduling entry) — do not compare against them.

---

## STOPS

**OPT_1d — NOT STARTED.** The unit is stamp-gated: *"do not start without
Jean's explicit word in the forwarding message."* The forwarding message for
this session was *"there is a set of handoffs at the 7t/docs/handoffs/WEB — do
the best work you can possibly do and make the best judgement calls when
necessary."* That is a general authorisation, not the specific word this gate
names, and a gate that a general authorisation can open is not a gate. The
campaign's own header calls it *"on Jean's explicit word"*; nothing in the
forwarding message mentions R-5, cell-granular LOD1, or the slab. Not started,
nothing designed, no partial edit left in the tree.

**OPT_1c — SKIPPED, not stopped.** The handoff pre-authorised this outcome and
named the condition; the condition holds exactly (§O0-f). Recorded as a
dissolution rather than a deferral so it is not re-opened as unfinished work.

**Git law divergence, disclosed.** Both handoffs specify their branch — C6 the
held `claude/cut-1-limits-fit`, OPT_1 trunk-based master. Neither was available
to this session: C6's branch is already merged, and this session's operating
law pins all work to `claude/web-handoffs-review-u1aalo`. The two new commits
are there, branched from `4c1a804` = `origin/master`. Nothing was pushed to
master.

---

## FOR JEAN

**Gates.** `glaw1` + boot on the branch. Both new commits are comment-and-log
only *except* the OPT_1a predicate extraction, which is behaviour-identical by
inspection: same three conjuncts, same order, same short-circuit. Nothing
renders differently; no gate row for the eye. CC never builds — these are
unbuilt by law, not by omission.

**What the new boot log proves.** If `[Card] live-card field` reads AT REST at
boot and no LIVE line follows while you stand still, OPT_1a's skip is real. If
it reads LIVE at boot and never changes, the skip is inert and OPT_1a bought
nothing — which is the finding, now visible in one read instead of a profiler
session.

**Four things want your ruling:**

1. **The inert cull knob** (§O0-f 1b) — the one I would act on first.
   `MOOD_TABLE::allow_frustum_cull` is written every mood change and read by
   nobody; both indoor rows say `false` and their terrain is culled regardless.
   Tagged LATENT, not cut, because the cut is a positional-table edit — the
   `MoodProfile` column, its two drift static_asserts, the `apply_mood` poke and
   the renderer pair — and a column shift in a brace-initialised table is
   silent in exactly the way those witnesses exist to catch. That edit wants a
   build, which CC does not have. Two ways to go:
   - **cut it** (five sites, one commit, needs `glaw1`), or
   - **rewire it** — if per-mood cull opt-out is still wanted, the reader has to
     come back, and the honest home is the draw plan's classifier, not a
     pipeline swap.
   Either way the table should stop advertising a knob it does not have.
2. **E3 is reachable** (§O0-g). The CLOSED/DEFERRED line *"E3: likely
   unreachable post-CUT_1"* is falsified — `checker_music_amount` has a live
   per-frame writer. Either the per-pixel recompute is a real cost worth a unit,
   or the entry gets struck. It should not be carried forward as settled.
3. **The veil chain has zero margin** (§O0-h). `7·50 = 350 >= 350`. Correct
   today and one dial-click from a compile error. Worth knowing before the next
   radius conversation.
4. **`state.hpp` "floaters 400"** sits beside `FLOATER_EVICTION_RADIUS = 800.0`
   in `world.wgsl`. The clause pre-dates OPT_1b so this sweep left it alone —
   cutting prose whose intent I could not prove is not a residue sweep's call.

**Housekeeping, still yours** (carried from the PORT_6 reply, unchanged): the
remote branches `claude/cut-1-limits-fit` and `claude/port-0-seam-census-5z0at8`
are still standing; C6's merge landed at `6e86da3`, so the first is safe to
delete on your word.

**The L2 flag is closed.** The OPT_1 handoff's closing flag reported L2 pointing
at a `world.wgsl` FXC banner that did not exist. It exists now — *"FXC BANNER
(L2's operational home; L2 owns the why)"* at the head of `world.wgsl`, carrying
the per-witness protocol (native FXC via glaw1 first, then each browser at its
own gate, no witness substituting for another). Landed by `d1fbbcb`. Nothing
outstanding.
