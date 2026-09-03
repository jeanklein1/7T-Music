# SKIRT_WELD_1 — BUILD REPORT

**Branch:** `master`. **Base:** `c8b856dc` (HEM_1 U4+U5+U6). Tree clean at start.
**Rounds:** one. **No unit held, no unit quarantined.** U2's acceptance passed,
so no STOP fired and U3 ran on its own independence rather than on a fallback.

---

## UNIT TABLE

| Unit | Subject | Status |
|---|---|---|
| U0 | recon, no edits | **DONE** — 5 items, 4 confirmed, 1 confirmed *with a correction to §1* |
| U1 | the inverse walk + its compile-time battery | **DONE** — `constexpr` `static_assert`, no fallback |
| U2 | re-aim both call sites, excise `skirt_cap_index` | **DONE** — four counts byte-identical |
| U3 | the tint's variation becomes a gain | **DONE** — clamp excised |
| U4 | gates + the record | **DONE** — ten green; **the record ritual deliberately not run**, see D3 |
| U5 | this report | **DONE** |

---

## 1 · U0 — EVERY ITEM, CONFIRMED OR CORRECTED

**Item 1 — `skirt_cap_index` has exactly two call sites. CONFIRMED, with the
line numbers one row off.** The definition sat at `state.hpp:3985`. The two
call-site pairs were at **4057–4058** (LOD0, inside `build_lod0_ib`) and
**4128–4129** (LOD1, inside the stride-2 walk) — the handoff's numbers exactly.
A whole-file grep found no third reader, no forward declaration, no mention in
prose outside the two band banners the handoff already named. After U2 the
symbol appears nowhere in the tree.

**Item 2 — `in.skirt` has exactly one consumer. CONFIRMED.** Written once at
`world.wgsl:4610` (`out.skirt = d.wall;`) and read once at `world.wgsl:4723`
(`if (in.skirt > 0.01)`, inside the `DEBUG_VIEW == 3u` arm — SKIRT PAINT). No
other read, in any entry point, in either patch VS or the shadow pair.

**Item 3 — the two bands' world position for the same `(cell, lx, lz)`.
CONFIRMED bit-identical in the ADDRESS, and deliberately NOT in the lift.**
The two decode lines, side by side, from `ug_decode`:

```wgsl
// cap band   (vi < UG_BASE_BASE)
let lx = k % UG_CAP_STRIDE;
d.vx = (cell % PATCH_CELL_N) * UG_QUADS + lx;

// base band  (vi >= UG_BASE_BASE)
let g = ug_cell_perimeter(r % 16u);
d.vx = (cell % PATCH_CELL_N) * UG_QUADS + g.x;
```

Same expression, same operands, same integer arithmetic — so `d.vx`/`d.vz` are
equal, and therefore `uv`, `world_pos.xz`, the `textureLoad` of
`patch_heightfield_array_read`, the aura and the live card all resolve to the
same values. **The one difference is the point of the campaign:** the base arm
additionally sets `d.wall = 1.0`, and `d.lift_scale = 1.0 - d.wall` then makes
`world_pos.y += lift * d.lift_scale` a no-op. Cap lifts, base does not.

**Item 4 — every skirt ring vertex lands on a cell perimeter. CONFIRMED on all
four edges and all four corners.** The ring walks `vz == 0`, `vx == N`,
`vz == N`, `vx == 0` with `N = PATCH_MESH_N = 64`. On the two `vz`-pinned edges
`lz` is `0` (at `vz = 0`) or `4` (at `vz = 64`, where `cz` clamps to 15 and
`lz = 64 − 4·15 = 4`); on the two `vx`-pinned edges the same holds for `lx`. So
`lx ∈ {0,4}` **or** `lz ∈ {0,4}` at every one of the 256 slots. At the far
corner `(64, 64)` both reach 4 and `cell_perimeter_slot(4,4)` returns 8, which
is exactly where `cell_perimeter` emits `(4,4)`. **U1 and U2 did not
quarantine.**

**Item 5 — the four index counts. RECORDED, and they are U2's acceptance.**
See §3.

**THE CORRECTION — §1's attribution of the two-tone is one step off, and §6's
own residual 2 has it right.** §1 says skirt verts decode with
`cell_local.z == 0`, so the FS falls back to `cell_texel` — the floored world
position — and "the colour flips mid-face". That is true of the ring *copies*
(`PATCH_GRID_VERT_COUNT ≤ vi < UG_CAP_BASE`), but the copies are never the
provoking vertex. `cell_local` is `@interpolate(flat)`, whose provoking vertex
under WebGPU is the primitive's **first**, and the skirt emission is
`a, b, sa` / `b, sb, sa` — so triangle 1 provokes from `a` and triangle 2 from
`b`, and both were cap verts (`vi ≥ UG_CAP_BASE` ⇒ `z = 1`) before the re-aim
and are base verts (`z = 1`) after. The FS therefore took `in.cell_local.xy`,
not `cell_texel`, on both sides of the change. **The two-tone is the
per-triangle cell STEP at a seam** — `a` in cell *c*, `b` in cell *c+1*, so the
quad's two halves shade from two different cells along its diagonal — which is
precisely what §6's residual 2 describes. It does not move the ruling: U2 rests
on the geometry argument (a quad straddling two independent lifts), which is
untouched, and the step survives the re-aim by design.

---

## 2 · WHAT LANDED

**U1 — the inverse walk, and it is `constexpr`.** `cell_perimeter_slot` sits
immediately beneath `cell_perimeter` in `state.hpp`, verbatim as the handoff
wrote it plus `constexpr` on the lambda. Beside it, two `static_assert`s: the
shape guard (`UG_QUADS_PER_CELL == 4 && UG_BASE_VERTS_PER_CELL == 16`) and the
round-trip battery.

**U2 — both call-site pairs re-aimed, the old lambda excised.**
`skirt_base_index` replaces `skirt_cap_index` wholesale; the return is
`UG_BASE_BASE + (cz·PATCH_CELL_N + cx)·UG_BASE_VERTS_PER_CELL +
cell_perimeter_slot(lx, lz)`. Both pairs — LOD0 (now `state.hpp:4102–4103`) and
the stride-2 LOD1 walk (now `4173–4174`) — carry the new name. The two band
banners that stated the old attachment were rewritten in the same unit; both now
read *base twins*. The maximum index the new lambda can emit is
`10881 + 255·16 + 15 = 14976`, inside both `UG_DECODE_VERTS = 14977` and the
`uint16_t` index buffer.

**U3 — the variation is a gain.** `apply_automaton_color`'s BLACKISH tail now
multiplies: `gain = vec3(1 + r_shift, 1 + g_shift, 1 − r_shift)`,
`alive_color = base_color * dark_factor * gain`. The authored red↔blue seesaw
survives as `1.0 − r_shift`; neither `_SHIFT_RANGE` value moved; the LENS branch
is untouched. **The clamp is excised, not kept.** Ceiling
`0.55 · 1.05 = 0.578` against a base ≤ 1, floor ≥ 0 because every factor is —
a clamp there would have been a lie about what the line can produce.

---

## 3 · THE FOUR INDEX COUNTS — BEFORE AND AFTER, AS LITERALS

| count | before | after | composition |
|---|---|---|---|
| `patchIndexCount_` | **50,688** | **50,688** | cap 24,576 + curtain 24,576 + skirt 1,536 |
| `patchIndexCountCapOnly_` | **26,112** | **26,112** | cap 24,576 + skirt 1,536 |
| `patchIndexCountRingClean_` | **6,912** | **6,912** | cap 6,144 + skirt 768 (stride 2) |
| `patchIndexCountRingZoned_` | **19,200** | **19,200** | the clean prefix + curtain tail 12,288 |

**Byte-identical, and structurally so.** The diff changed only the *values* of
`a` and `b`; every loop bound (`UG_CELLS_PER_PATCH`, `UG_QUADS_PER_CELL`,
`SKIRT_RING_N = 4·PATCH_MESH_N`, the `k += s` stride) is untouched, and each
iteration still pushes the same six indices `a, b, sa, b, sb, sa`. `50,688`
also still matches the builder's own `~50,688` banner and the `19200` in
LOD1's `idx.reserve` comment. **No count moved, so U2's STOP never armed.**

---

## 4 · THE `constexpr` ROUND TRIP — IT LANDED, IN A FORM THE HANDOFF DID NOT
   SPELL, AND THE REASON MATTERS

**It landed as a `constexpr` `static_assert`. It did not fall back to the
`#ifndef NDEBUG` / `assert` loop.** The TU gate is green on it, both tiers.

**The divergence:** the handoff asks for "a `constexpr` round-trip check over
`k ∈ [0,16)`" — i.e. drive `cell_perimeter(k, lx, lz)` and assert
`cell_perimeter_slot(lx, lz) == k`. That call is impossible from a constant
expression: `cell_perimeter` is a **non-`constexpr` runtime lambda**, and §3
FROZEN forbids editing it to add `constexpr`. The two ways out were (a)
restate `cell_perimeter`'s walk inside the assert — which would give the fact a
second home that cannot catch divergence, exactly the failure the PROTECT LIST
warns about for `cell_local.z` — or (b) state the sixteen `(lx,lz) → k` pairs as
literals. **I took (b).** It is stronger than it looks: the sixteen right-hand
sides are `0..15`, each exactly once, so the assert is simultaneously the
round-trip check and a **bijection proof** of the cap perimeter onto the base
band. The comment beside it says plainly that the right-hand column *is*
`cell_perimeter`'s emission and why it is transcribed rather than called.

`static_assert` over `assert` was not a preference: `state.hpp` uses
`static_assert` throughout and contains no `assert()` and no `<cassert>`
anywhere.

---

## 5 · THE TWO NAMED RESIDUALS — BOTH PRESENT, NEITHER A BLOCKER

**R1 — `in.skirt` on the perimeter quad goes from `0 → 1` to constant `1`.**
Before, the top edge was a cap vert (`wall = 0`) and the bottom a ring copy
(`wall = 1`), so the varying interpolated across the quad and DEBUG_VIEW 3's
`in.skirt > 0.01` test left a thin band at the very top unpainted. Both ends now
carry `wall = 1`, so the whole quad paints magenta. Sole consumer is that debug
arm; **the art does not move, and the instrument gets more truthful** — the
whole quad *is* a wall.

**R2 — the per-triangle `cell_local` step at a cell seam survives.** Flat
interpolation, provoking vertex = first, emission `a, b, sa` / `b, sb, sa`: at a
ring slot that is a multiple of `UG_QUADS_PER_CELL`, `a` and `b` sit in adjacent
cells, so the quad's two triangles resolve `owned_texel` — and therefore the
baked cell colour and the GoL tint — from two different cells, split along the
quad's diagonal. It was true before the re-aim and is true after. **What the
re-aim removed is the ramp the step was riding on:** the quad no longer climbs
`alive_height`, so the step now falls on a flat ring skirt hanging below ground
level. Recorded; not chased.

---

## 6 · GATES

| Gate | Result |
|---|---|
| TU gate | **PASS ×2** — CARTRIDGE and CONSOLE, zero diagnostics. The `constexpr` battery compiles. |
| WGSL gate (naga) | **PASS** — parses, scopes, validates raw |
| G-LAW 2 | **GREEN** — 258 fn, 257 const, 55 struct, 55 binding, 34 entry points; 18 retired cleanly |
| mirror census | **GREEN** — regenerated; ML-0/ML-1/M2-0/M3-0/M5-0/M7-0/M4-h/ML-2w all PASS |
| mirror offsets `--check` | **PASS** — 128 members, 7 structs |
| binding surface `--check` | **PASS** — every relation, S-8 over 11 fixed extents, P-seq, P-scope(R/C) |
| score census | **GREEN** — bijection both directions, 23 rows |
| shell gate | **PASS** — 5 scenes, the scripted session, the round trip |
| organ ledger `--check` | **NO SUSPECTS** — 240 proved, 0 suspect |
| organ gap `--gate` | **PASS** — 0 surviving readers across 14 graduated pairs |
| G-LAW 1 · the probe | **Jean's** |

Zero unexpected reds. `binding_gen --check`'s S-6 read DIRTY mid-round, as it
must on an uncommitted tree; it is green at the pushed tip.

---

## 7 · DIVERGENCE LOG

**D1 — the round trip is sixteen literals, not a driven loop.** §4 above, in
full. This is the one place the campaign's letter and its intent parted, and I
chose intent: a check that cannot silently agree with a copy of itself.

**D2 — U0 item 1's line numbers were one row off, and the two band banners were
staler than the handoff said.** The LOD0 banner read "top edge = cap outer
verts; copies keep the legacy ring slots **(their decode changes in U3)**" —
that parenthesis names a *previous* campaign's U3 and had outlived it. Both
banners are now correct and both name SKIRT_WELD_1.

**D3 — THE RECORD RITUAL WAS NOT RUN. Deliberate; flagged, not executed.** U4
asks for `python3 tools/gates/glaw2/run.py --record`. The handoff itself
concedes glaw2 cannot see the one name that retires here — `skirt_cap_index` is
**C++** — and this campaign adds no WGSL entry point and no WGSL const, so
glaw2 is **GREEN with no re-record needed** (verified: 34 entry points, 18
symbols retired cleanly, unchanged). `--record` rewrites the whole baseline
*including the `declared` retirement ledger*, which RETRACT_0's R10 flagged as
Jean-gated and destructive. Rewriting that ledger over a campaign that moved
nothing glaw2 tracks would put a false generation stamp on it. **The tombstone
for `skirt_cap_index` is the diff's, as the handoff says.** If Jean wants the
baseline re-stamped anyway, it is one command and I will run it on his word.

**D4 — no `world.wgsl` prose was corrected, though some has decayed.** §3
FROZEN forbids touching `world.wgsl` outside `apply_automaton_color`, so two
now-imprecise banners stand and are reported instead: the `PatchTerrainVarying`
comment at `4489` ("`0` on legacy **and skirt**, whose quads straddle cells")
and `patch_skirt_grid`'s mirror note at `303–304` ("the two MUST agree so each
skirt quad's top edge reads the right composited height"). The first is about
the *copies*, which is still true, but reads as though it covered the ring's top
edge, which is now a base vert with `z = 1`. The second is still true — the
mirror the C++ ring walk must match is `patch_skirt_grid`, and
`skirt_base_index` still walks it — but "cap lattice" thinking is one hop away
from it. Both are one-line truth-fixes for whoever next unfreezes the file.

---

## 8 · WHAT MADE ME HESITATE

1. **The provoking-vertex question, twice.** §1 and §6 of the handoff give two
   different mechanisms for the same two-tone. Resolving it meant deciding that
   WebGPU pins flat interpolation to the primitive's **first** vertex, which
   makes §6 right and §1 one step off. If that premise is wrong, R2 changes
   shape and §1's `cell_texel` reading returns — but the ruling and the fix do
   not move either way, because U2 rests on geometry, not colour.
2. **Excising the clamp.** Deleting a clamp is the kind of edit that looks
   reckless in isolation. The bound is arithmetic and I stated it in the code
   rather than in this report alone, so a future reader can re-derive it without
   finding this file.
3. **Not running `--record`.** Every other unit of this campaign is a mechanical
   instruction I executed as written; this is the one I declined, and declining
   an instruction from a handoff is exactly the thing CC does not get to do on
   its own authority. I flagged rather than acted because the destructive half
   is Jean-gated by a prior ruling and the constructive half is a no-op.

---

## 9 · FILES TOUCHED

| File | What |
|---|---|
| `realization/state.hpp` | U1 `cell_perimeter_slot` + two `static_assert`s; U2 `skirt_base_index`, both call-site pairs, both band banners; `skirt_cap_index` excised |
| `realization/world.wgsl` | U3 only — `apply_automaton_color`'s BLACKISH tail, gain in place of offset, clamp excised |
| `docs/OPEN.md` | the SKIRT_WELD_1 section, carrying the HELD shading-normal seam |
| `docs/SKIRT_WELD_1_REPORT.md` | this file |
| `audit/MIRROR_LEDGER.md`, `audit/BINDING_LEDGER.md` | regenerated (line-indexed; both source files moved) |

No `.gen.inc` hand-edited. No schema change, so no `binding_gen --write`.
`audit/ORGAN.md` and `audit/COMMAND_LEDGER.md` regenerated to no change.

---

## 10 · FOR JEAN — THE VISUAL GATE

Build → `--probe` → glaw1 → walk. **Behaviour identity will not catch this**;
it wants the two named shots.

1. **THE SHOT.** A live cell at a **patch boundary**, camera at pawn height,
   automaton running, `alive_height` at or above the value in shots 226/227.
   Before and after. The scattered slivers poking through the seam should be
   gone; the seam should read as one skirt with the per-cell curtain doing the
   sealing.
2. **THE SECOND SHOT — U3's.** The same frame over **dark** ground. The red
   wedge is the subject and **its absence is the verdict**. Black should stay
   black; the per-cell scatter should still be there, just proportional.
3. **`DEBUG_VIEW = 3u` paints more than it did.** The whole skirt quad is
   magenta now, top edge included. That is R1, and it is correct.
4. **If you still see a colour step across a seam quad's diagonal**, that is R2
   and it is expected — it is flat, not a ramp, and it is not this campaign's.
5. **The record stamp is yours if you want it.** D3 explains why I did not run
   `glaw2 --record`. One word and it runs.
