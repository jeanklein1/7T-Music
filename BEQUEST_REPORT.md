# BEQUEST — the estate's two bequests

Two units, three commits, no halt. The one parked move the campaign's own
law mandated is executed; the one whose price only the tree could reveal
is priced and shelved. **And the executed move paid a dividend the
handoff had ruled out — see BF1, which is the finding of this round.**

---

## Header

| field | value |
|---|---|
| HEAD before | `a5e84bf` — GATEHOUSE_G6 |
| HEAD after | this commit (`BEQ_CLOSE`) |
| branch | `master`, per O-1 |
| baseline (§B0) | `--check`, wgsl, TU, sha256 — **all GREEN**; tree clean |
| digest at authoring | `sha256(world.wgsl)` = **`3cf2e02f`** — **matched exactly.** No drift; the tree is the set this handoff was authored against |

| # | commit | subject |
|---|---|---|
| 1 | `c35e77c` | BEQ_A: sphere slot 0 rides frame_r — the terrain fragment stage stops reading storage per pixel; third passenger, same sovereignty |
| 2 | `de1d5db` | BEQ_B: TEX_C0 priced at its true bill — the exhibition array's shared format blocks compression; the price sheet stands where the next campaign will look |
| 3 | this one | BEQ_CLOSE: ledger regenerated; the A1 dividend BEQ_A uncovered is recorded, unclaimed |

---

## FIND counts, as applied

Every FIND verified **before** any edit, per O-5. All six were exactly 1.

| unit | FIND | expected | found |
|---|---|---|---|
| B1.1 | FrameR struct + `Offsets: … camera 976.` | 1 | **1** |
| B1.1 | `zone_sphere_ff(…, render_floating.entities[0].pos)` | 1 | **1** |
| B1.2 | `GPUFrameR` mirror + its three static_asserts | 1 | **1** |
| B1.2 | banner tail `…the readback law is why the copy exists at all.` | 1 | **1** |
| B1.2 | main camera copy | 1 | **1** |
| B1.2 | photographer camera copy | 1 | **1** |

---

## Flags

| # | unit | site | expected | found | action |
|---|---|---|---|---|---|
| **BF1** | §B1 | `render_floating` F-stage visibility | *"`entity_fs` reads `render_floating` per-fragment at its own sites, so the scene layout's F-visibility **stays** — no schema visibility edit, **no slot dividend**"* | **the premise is false, and the dividend is real.** `entity_fs` does not read `render_floating` **at all** — verified by reading its whole body. All four surviving code reads are VERTEX: `sphere_vs`, `monolith_vs`, `shadow_sphere_vs`, `shadow_monolith_vs`. The terrain fragment read BEQ_A promoted was **the program's only fragment-stage read of that binding** | `skipped-step` — flagged, **not claimed.** The ledger's A1 sweep raised the row by itself the moment the ledger regenerated (quoted below): `render_floating` now declares `VF`, is reached by `V`, delta `F`, **1 F-stage storage slot freed** if taken. Claiming it is a schema visibility edit — exactly what §B1 excluded, and O-6 forbids adapting schema relation semantics on an executor's authority. **Nothing is broken:** over-declared visibility is legal and merely charges a slot; `--check` is green and every witness passes. The dividend sits in Table C, banked, for the campaign that wants it. |
| BF2 | §B1.1 | the `render_floating` witness | *"`grep -c "render_floating" world.wgsl` drops by exactly 1"* | **raw count is 6, unchanged** | `graduated` — arithmetic, not substance. **Code sites did drop by exactly 1** (6 → 5); the raw count holds at 6 because the handoff's own never-adapt replacement prose names the binding in a comment (`// fragment stage's one frame-uniform read of render_floating,`). The witness's intent is satisfied and its literal form is not; recorded so the next reader is not misled by a green count that should have moved. |
| BF3 | §B1.3 | the two `RESOURCES` rows | *"1024 → 1040 in each"* | **no literal to edit.** Both rows carry `size_expr: 'sizeof(GPUFrameR)'` — symbolic, so the growth propagates itself | `graduated` — no edit owed there. The only literal `1024` in the schema was the seat's trailing prose (`// frame_r (uniform, 1024 B — CHORD_3)`), corrected to `1040` as a count doing registry work (P5). The seat's prose block, which enumerates the block's passengers, gained sphere_pos — comment prose within a template, O-6 may-adapt. |
| BF4 | §B1.3 | the MANIFEST diff | *"the FrameR size cells only"* | **the MANIFEST diff is EMPTY** — not one cell moved | `graduated` — MANIFEST carries lane counts, not byte sizes; the `1024 B` the handoff attributes to "MANIFEST Table A" is in **the ledger's** Table A, computed at regeneration. The stronger half of the expectation held perfectly: **no lane count moved anywhere**, and M-1's five worst rows are identical before and after. |
| BF5 | §B2 | the estate's dimensions | *"16 layers"* staging each, *"32 layers"* exhibition | **32 / 32 / 40.** `Dim::STAGING_LAYERS = 32` per staging array, `Dim::EXHIBITION_LAYERS = 40` — raised by SUPPLY, whose comment says so in place | `graduated` — the price sheet is written from the tree's numbers, not the sketch's. The blocking fact is **unaffected** (it is about formats, not counts) but every figure downstream of it changes, and the resident total is 104 MiB rather than the ~64 the old counts imply. |
| BF6 | §B2 | the shared format | *"rgba8 as today"* | **stronger than stated.** All three arrays are created at `colorFormat_`, which is `caps.formats[0]` — **the surface's own swapchain format** (typically `BGRA8Unorm`), not a constant the program picks | `graduated` — recorded in the sheet, because it makes the blocking fact worse in a useful way: the exhibition array's format is not merely shared between tiers, it is **inherited from the surface** and was never the program's to choose. |
| BF7 | §B2 | the prize | *"~5–8× smaller"* | unsourced, and it silently assumes the mobile path — **BC7 has no block smaller than 4×4, so desktop is capped at 4×** | `graduated` — replaced with the arithmetic and its basis: 4× (BC7 / ASTC 4×4) to 9× (ASTC 6×6), tabulated per format, with the desktop cap stated. The sheet says plainly that the mobile and desktop dividends differ and only the mobile one is large. |

---

## The dividend BEQ_A uncovered (BF1), quoted from the regenerated ledger

```
### A1 — OVER-VISIBLE (`vis_delta` ≠ ∅)

| layout | # | symbol | kind | slot | vis_declared | vis_actual | vis_delta | slots freed, per stage | reached by |
| Scene State Layout | 1 | `render_floating` | buffer | storage | `VF` | `V` | `F` | F: 1 storage | Monolith Entity (Rasterized)/monolith_vs, Sphere Entity (Rasterized)/sphere_vs |
```

The F-stage occupancy fell on every entity pipeline's fragment row —
`patch_terrain_fs` storage `3 / 3 / 3` → `3 / 2 / 2`, and `entity_fs`,
`ribbon_fs`, `orb_fs` each `3 / 3 / …` → `3 / 2 / …` (declared /
actual / reached). Declared stays 3 because the declaration still claims
F; actual and reached fell because nothing in F reaches it any more.

**That is the whole shape of the finding:** BEQ_A was sold as a
bandwidth move on the terrain path, and it was one — but it also
retired the last fragment-stage reader of a storage binding, which is a
*slot* move the handoff explicitly said would not happen. A1's own
preamble is the right frame for it: *"Correcting one is not an
optimization: it is the removal of a false statement that happened to
cost a slot."* The false statement is now in the tree, named, in the
table built to find exactly this.

---

## Numbers

| quantity | §B0 baseline | §B3 close |
|---|---|---|
| `FrameR` / `GPUFrameR` size | 1024 B | **1040 B** |
| `sphere_pos` offset | — | **1024** (both rooms) |
| ledger Table A, `frame_r` | 1024 | **1040** |
| witness `0b-4` | FrameR 1024 B | **FrameR 1040 B** |
| C++ byte-mirror asserts | 3 | **4** |
| `render_floating` code sites | 6 | **5** (4 vertex reads + the declaration) |
| fragment-stage readers of `render_floating` | 1 | **0** |
| Tier A1 rows | 0 | **1** (unclaimed) |
| MANIFEST cells moved | — | **0** |
| M-1 worst rows | uniform 5/12, storage 5/8, sampled 6/16, samplers 3/16, storagetex 2/4 | **identical** |
| `sha256(world.wgsl)` | `3cf2e02f` (637 679 B) | **`36fd46ab`** (638 167 B) |
| GPU budget delta | — | **+32 B** (16 B × two frame_r instances) |

### Both rooms witnessed independently

The L3 mirror is held by two instruments that share no code:

- **C++** — four `static_assert`s: `sizeof(GPUFrameR) == 1040`,
  `offsetof(vp) == 848`, `offsetof(camera) == 976`,
  `offsetof(sphere_pos) == 1024`. A wrong offset refuses to compile, and
  the TU gate compiles it.
- **WGSL** — the ledger's own layout calculator recomputed `FrameR` from
  the struct and got **1040 B**, matching the marker (`0b-4`). It was
  not told the number; it derived it.

### Copy legality, for the record

`floatingEntityBuffer_` offset 0 → `frameRMainBuffer_` / `frameRPhotoBuffer_`
offset 1024, 12 bytes. Src and dst offsets are multiples of 4; size is a
multiple of 4; different buffers, no overlap; `CopySrc` on the source and
`CopyDst` on both destinations were **already granted** — nothing was
widened. And `pos` is at offset 0 of `GPUFloatingEntityState` **and** of
WGSL's `FloatingEntityState`, with `entities` at offset 0 of
`FloatingEntityArray`, so offset 0 is exactly sphere slot 0's position —
verified in both rooms before the edit, because a wrong offset here is a
ring in the wrong place and nothing would say so.

### Gates

| gate | result |
|---|---|
| `binding_gen.py --check` | all relations agree, all witnesses pass |
| `binding_ledger.py` | 42 PASS, zero warnings; `E1-identity` PASS |
| TU gate (cartridge + console) | **PASS**, zero diagnostics |
| `wgsl_gate.py` | **PASS** |
| `sha256_gate` | **PASS** — digest moved, which is the witness working |
| L1 | clean on every changed file |

---

## Per-unit status

| unit | status | one line |
|---|---|---|
| §B0 | **done** | All gates green; digest matched `3cf2e02f` exactly — no drift, and all six FIND counts confirmed at 1 before any edit. |
| §B1 | **done, one flag** | Executed verbatim. Both rooms witnessed at 1040 independently. Uncovered a slot dividend the unit's own recon ruled out (BF1) — flagged, unclaimed. |
| §B2 | **done** | Price sheet written from the tree's numbers, not the sketch's: the blocking fact confirmed and strengthened (BF6), dimensions corrected (BF5), the prize tabulated with its basis (BF7). |
| §B3 | **done** | Ledger regenerated; the A1 row recorded; this report. |

---

## Jean's checklist

1. **Pull, build.** The four `static_assert`s compiling **is** the
   byte-mirror witness — a wrong offset refuses to compile. Zero
   warnings stands (GATEHOUSE's posture is unchanged).
2. **Deploy.** Read `web_dist.py`'s sha line; the digest moved with
   B1.1, by design. The console must match whatever that line prints —
   not a number written here.
3. **Pinned seed, one flight.** The landscape identical as before, and
   then the one behaviour only your eye can witness:
   **the sphere's force-field ring on the terrain still follows the
   sphere as it moves.** A ring frozen at origin, or gone, is this
   unit's failure wearing its face — it would mean the 12-byte copy is
   landing wrong or not landing at all. `[GPU Budget]` unchanged at
   print precision (+32 B total); the ledger shows `FrameR` at 1040 and
   no lane count moved.
   *This flight also carries the terrain word for PROBATE_I, four rounds
   outstanding.*
4. **`docs/audit/TEX_C0_PRICE.md`** reads correctly to your eye — it is
   the document the gallery campaign opens first, and the only one that
   says out loud that the original sketch is impossible.
5. **BF1 wants a ruling, not a look.** One F-stage storage slot is
   sitting unclaimed in Table C because BEQ_A retired the last fragment
   reader of `render_floating`. Taking it is a one-line schema
   visibility edit plus regeneration — cheap, and this round had no
   authority to make it.

---

*With this round the campaign's technical estate is fully disposed:
every move the law permitted is executed, every move it deferred is
priced, and the one move the tree refused is refused in writing. The
effects campaign inherits no mysteries — and one small, named,
already-measured dividend.*
