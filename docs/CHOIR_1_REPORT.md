# CHOIR_1 — BUILD REPORT

**Branch:** `master`. **Base:** `67e229ef` (CHOIR_0 U6h). Tree clean at start,
clone not shallow. **Rounds:** one. **No unit held, no unit quarantined, no
STOP fired, and NO DIVERGENCE from the handoff's ruled numbers** — every value
below is the one §1 ruled. **Working name; naming is Jean's gate.**

**No new mechanism anywhere.** Every edit is a number in a table that already
existed, plus the comments that state what those numbers mean. `world.wgsl` is
byte-identical, nothing was enrolled, no name retired, no gate baseline
re-recorded.

---

## UNIT TABLE

| Unit | Subject | Status |
|---|---|---|
| U1 | the token | **DONE** — `CUBE_CHOIR_N` 36 → 24, one token as banked |
| U2 | the tier retune | **DONE** — 7 pairs moved, every other pair stands |
| U3 | the snap | **DONE** — τ divisor 4 → 6, all four derivation homes, table re-counted |
| U4 | gates + report | **DONE** — every runnable gate green; probe + desk are Jean's |

**Commits (2):** `c533d1c6` U1+U2+U3 · this one U4. U1/U2/U3 are independent
by §3 and land together because they are one desk pass on one instrument;
splitting three number changes into three commits would say they were three
decisions.

---

## 1 · THE RULED NUMBERS, AS THEY LANDED

| tier | BODY_RADIUS μ,σ | ORBIT_HEIGHT μ,σ | ASPECT_Y μ,σ |
|---|---|---|---|
| 0 SmallCube | `{1.8, 0.33}` *stands* | `{25, 20}` → **`{12, 6}`** | *stands* |
| 1 MedCube | `{4.0, 0.8}` → **`{3.0, 0.60}`** | `{45, 30}` → **`{16, 8}`** | *stands* |
| 2 LargeCube | `{8.0, 1.67}` → **`{5.0, 1.00}`** | `{75, 45}` → **`{22, 10}`** | *stands* |
| 3 Monolith | `{3.0, 0.53}` → **`{2.2, 0.40}`** | `{12, 8}` → **`{10, 5}`** | `{5.0, 0.80}` → **`{3.5, 0.55}`** |

Seven pairs moved. `INFLUENCE_RADIUS`, `SPIN_SPEED`, `BOB_AMPLITUDE`,
`BOB_PERIOD`, `ASPECT_Z`, `FACE_VARIANCE`, every `spin_tilt_sigma` and the
weight column `{0.40, 0.32, 0.20, 0.08}` all stand — the protect list held.

---

## 2 · THE AUDIT, COUNTED RATHER THAN ASSERTED

Every claim the handoff made about these numbers was re-derived before the
edit, not after.

### Radius CV — SCALE_0's law at the new means

| tier | σ/μ before | σ/μ after |
|---|---|---|
| 0 Small | 0.1833 | **0.1833** *(unmoved)* |
| 1 Med | 0.2000 | **0.2000** |
| 2 Large | 0.2087 | **0.2000** *(tightened)* |
| 3 Monolith | 0.1767 | **0.1818** *(loosened)* |

**ONE CORRECTION TO THE HANDOFF'S PROSE, NOT ITS NUMBERS.** §U2 says "Radii
keep CV = 0.20 exactly". That is true of the two rows that carried 0.20 —
Med holds it and Large is pulled back onto it from 0.209 — and it is **not**
true of the other two: Small stands at 0.183 because it does not move at all,
and the Monolith goes 0.177 → 0.182. **The ruled numbers are implemented
verbatim**; only the sentence describing them is looser than they are, and
the tier banner now states the per-row truth rather than the round one. σ
moving *with* μ is the substance of the law here — a retune moving μ alone
would have widened every silhouette in relative terms, which is the
compounding trap SCALE_0's own paragraph names.

### Height CV — the new authored choice

| tier | σ/μ before | σ/μ after | floor (3.0 wu) in σ, and the tail it clips |
|---|---|---|---|
| 0 Small | 0.800 | **0.500** | −1.10σ (13.6%) → **−1.50σ (6.7%)** |
| 1 Med | 0.667 | **0.500** | −1.40σ (8.1%) → **−1.62σ (5.2%)** |
| 2 Large | 0.600 | **0.455** | −1.60σ (5.5%) → **−1.90σ (2.9%)** |
| 3 Monolith | 0.667 | **0.500** | −1.12σ (13.0%) → **−1.40σ (8.1%)** |

"CV ≈ 0.5, was 0.6–0.8" holds exactly. **The floor column is a finding the
handoff did not claim and is worth having**: lowering the means could easily
have pushed the low tail *into* the 3.0 wu clip, and the narrower σ does the
opposite — every tier clips less than before, so the drawn distribution sits
closer to the authored one than it used to.

### The Monolith's silhouette

`radius × aspect_y` = **7.7 wu** of slab, from **15.0**. It stays the tall
one; `ASPECT_Z {0.15, 0.02}` stands, so it stays the thin one too.

---

## 3 · U3 — THE SNAP, RE-COUNTED AT THE NEW τ

Not scaled from CHOIR_0's table. Counted, at 120 BPM / 60 fps (Δ = 1/30 beat),
plateau 8, ε = 1e-3:

| quantity | τ = plateau/4 (CHOIR_0) | τ = plateau/6 (CHOIR_1) |
|---|---|---|
| τ | 2.0000 beats | **1.3333 beats** |
| switch-on slope 1/τ | 0.500 /beat | **0.750 /beat** *(+50%)* |
| per-frame rise coefficient | 0.016529 | **0.024690** |
| I at the 8-beat plateau | 0.98168 | **0.99752** |
| ε-gate closes at | frame 169, I = 0.940 | **frame 129, I = 0.960** |
| attack pokes, of the plateau's 240 frames | 199 | **158** |
| further pokes over the next ten plateaus | ~17 | **~2** |
| release pokes, of 240 | 240 | **240** *(unmoved)* |
| worst-case bytes/frame, screen standing | 720 (36 keys) | **480** (24 keys) |

**THE SNAP IS CHEAPER, AND THAT IS NOT OBVIOUS.** A sharper attack sounds
like it should cost more pokes; it costs 41 fewer, because a steeper climb
reaches the flat part sooner and the flat part is where the ε-gate stops
paying. Two consequences worth naming: the row is *more* literally "plateaus
at 8" than it was (99.75% against 98.2%), and **the release is now the
expensive half by a wide margin** — it is the only phase that pokes every
frame it lasts, and it did not move.

**FOUR HOMES STATE THE DERIVATION AND ALL FOUR MOVED**: the `tick()` ATTACK
banner, the file's own header paragraph, the `light_plateau` row comment in
`canvas_surface.hpp`, and the `CHOIR_FLUSH_EPS` band's counted table. §0(d)
says τ has one home — it does, the computation — but the *derivation* is
stated in four places, and a campaign that moved the divisor and left three of
them saying `/4` would be the exact defect class CHOIR_0's review spent itself
on.

---

## 4 · WHAT U1 FALSIFIED, AND WHY IT WAS FIXED RATHER THAN FLAGGED

The `[CHOIR]` boot witness printed `-> 36 keys, 3 rank(s)` from `CHOIR_LANES`.
CHOIR_0 wrote that line *because* an earlier review caught it conflating the
ear's width with the keyboard's — and U1 falsified it again from the other
side, because there are **three** widths now and the line claimed one it does
not own:

| width | value | whose fact |
|---|---|---|
| the EAR | 12 pc lanes | the analysis side's — a pitch-class vector |
| the PIPE | `CHOIR_LANES` = 36 | the canvas's — it envelopes every one |
| the KEYBOARD | `CUBE_CHOIR_N` = 24 | **the cartridge's** — the canvas may not name it |

The canvas's line now reports the two it owns and hands keys to the seam's own
witness, which already prints them from `CUBE_CHOIR_N` and needed no change.
The per-key `[CHOIR] key=NN` line reports a **lane**, and now says so: with
the choir narrower than the pipe, a lane ≥ 24 prints an activation edge and
lights no cube.

**This is not a §3 scope violation.** §3 forbids acting on a *discovered
adjacency*; a witness that U1's own edit turned false is U1's consequence, and
leaving it would have shipped a boot line that lies about the campaign that
printed it.

---

## 5 · FLAGS — REPORTED, NOT ACTED ON

**F1 — THE GATHERING WILL READ FLATTER, and it is the heights' doing.**
`station_scatter` seats a cube at `max(ZOETROPE_H_BASE 8, orbit_height +
jh·ZOETROPE_SCATTER_JITTER_H)` with `JITTER_H = 28` wu — and the jitter did
not shrink with the means, because §1 did not rule it. So the fraction landing
**exactly on the 8 wu floor** in the SCATTERED state rises sharply:

| tier | floors at h = 8, before | after |
|---|---|---|
| 0 Small | 23.8% | **42.7%** |
| 1 Med | 12.4% | **35.6%** |
| 2 Large | 6.8% | **25.5%** |
| 3 Monolith | 42.1% | **46.3%** |

(Monte Carlo, 200k draws per row, over the clipped Gaussian.) The flock keeps
its bearing scatter and its size-biased depth, but a lot more of it now sits
on one plane. **The fix is a taste call on `ZOETROPE_SCATTER_JITTER_H`** — 28
wu was chosen against means of 25–75 — and it belongs on Jean's desk with the
rest of the visual gate, not inside a unit that was not asked to touch it.

**F2 — THE SCATTER'S DEPTH BIAS SHRANK WITH THE RADII.**
`ZOETROPE_SCATTER_SIZE_BIAS = 4.0` wu of extra reach per wu of body radius,
and a LargeCube's radius went 8 → 5, so its outward seat moves 32 → 20 wu.
The mechanism that makes the gathering read as depth rather than as a ring is
proportionally weaker. Second-order beside F1, same desk.

**F3 — INFLUENCE RADIUS DID NOT MOVE WITH THE BODIES.** `INFLUENCE_RADIUS` μ
stands at `{6, 10, 14, 12}` while radii fell, so the influence-to-body ratio
roughly doubles on the big tiers (Large 1.75× → 2.8×). Not ruled, not
touched; named because a "smaller cubes" pass could reasonably have been read
to include it and deliberately was not.

**F4 — TWELVE ENVELOPE LANES ARE COMPUTED AND NEVER READ.** The pipe is wider
than the choir by design (that is what made the flip one token), so the canvas
envelopes lanes 24–35 that the cartridge never mirrors. Twelve float updates
a frame — real, cheap, and the price of the promise CHOIR_0 banked. Narrowing
the pipe would spend the promise.

**F5 — THE PAWN'S COLUMN WILL REACH THEM MORE OFTEN**, as §U2 predicted: at
these heights most cubes sit under the presence push's altitude ceiling.
Accepted for the test; the pawn-dynamics layer is under a deprioritization
ruling in flight.

---

## 6 · GATES

| gate | verdict |
|---|---|
| TU gate (`console_gate`) | **PASS** — both tiers, zero diagnostics |
| G-LAW 2 | **GREEN** — 18 symbols retired cleanly, **unchanged**. No record: C++ values only, no WGSL, no retired name |
| shell gate | **PASS** |
| score census | **GREEN** — 7 update + 16 render rows |
| WGSL gate | **PASS**, and a genuine no-op: `world.wgsl` is **byte-identical** |
| binding surface (`--check`) | **PASS** — every relation and witness, S-6 included once pushed |
| organ gap (`--gate`) | **PASS** — nothing enrolled; the freeze holds |
| organ ledger (`--check`) | **PASS** — NO SUSPECTS |
| mirror census | **GREEN** |
| mirror offsets (`--check`) | **PASS** — 128 members, 7 structs |
| G-LAW 1 | **JEAN'S** |
| **`--probe=N`** | **JEAN'S — THE DEVICE GATE. No visual sign-off before it.** |

**THE LEDGERS DID NOT NEED RE-STAMPING, AND THAT IS THE RULE WORKING, NOT
BEING SKIPPED.** All six tools were re-run after the code landed and every
artifact came back byte-identical — because U1/U2/U3 touched
`cube_behaviors.hpp`, `visual_canvas.hpp` and `canvas_surface.hpp`, and **not
one of those is a scanned input** for BINDING (`binding_registry.hpp`,
`world.wgsl`, `state.hpp`, `binding_surface.gen.inc`, `renderer.hpp`),
COMMAND (`cartridge.hpp` and the pass/body files) or MIRROR. CHOIR_0 §10's
chain — regenerate last, ledgers commit alone, MIRROR follows iff BINDING
moved with it — has nothing to bite on here. Verified by running the
regeneration, not by reasoning about it; had a stamp moved, this campaign
would carry the settle commits CHOIR_0's did.

---

## 7 · FILES TOUCHED

| file | what |
|---|---|
| `src/cartridges/the_board/bodies/cube_behaviors.hpp` | `CUBE_CHOIR_N` 36 → 24 + the two-rank banner; `CUBE_TIERS`' seven pairs + the SIZE μ and HEIGHT μ,σ paragraphs; the `CHOIR_FLUSH_EPS` band re-counted |
| `src/coupling/visual_canvas.hpp` | τ's one home; the tick banner; the file banner; both `[CHOIR]` witnesses |
| `src/coupling/canvas_surface.hpp` | the `light_plateau` row comment and its banner |
| `docs/COUPLING_ATLAS.md` | §3's keyboard, rank-law, shape and witness rows — the live map |
| `docs/OPEN.md` | the CHOIR_1 entry; a forward pointer on CHOIR_0's section; the campaign-ledger and atlas-index rows |
| `docs/CHOIR_1_REPORT.md` | this |

**`docs/CHOIR_0_REPORT.md` was deliberately NOT edited.** It is the record of
what CHOIR_0 did, and its numbers were true when it was written; rewriting a
landed report to match a later tree would destroy the only account of the
state it describes. `OPEN.md`'s CHOIR_0 section carries the forward pointer
instead — the same treatment that section already gives the coupling atlas's
own landing numbers.

---

## 8 · FOR JEAN — THE VISUAL GATE

1. **`the-board --probe=120` first.** No visual sign-off before PROBE GREEN.
2. **Pin `--seed=`** for the test — that plus 24 keys is what makes count,
   tiers and placement reproducible run over run. No spawn-path edit was made.
3. **The band.** Heights are lower *and* narrower on purpose (CV ≈ 0.5). If it
   reads too tight rather than too low, raise σ and leave μ; if too low, raise
   μ and scale σ with it — that is the discipline the banner now records.
4. **The big tiers.** Med 3.0, Large 5.0, Monolith 2.2 × 3.5. If the Monolith
   has stopped reading as a monolith, its `ASPECT_Y` is the knob, not its
   radius — the radius is what keeps it in scale with the others.
5. **The snap at τ = 8/6.** Switch-on is 50% steeper and the plateau is
   tighter. If it now reads as a hard on/off, the divisor is the knob —
   `light_plateau` itself is the length, not the shape.
6. **The flatter gathering (F1)** — press F6 twice and look at the flock, not
   the screen. `ZOETROPE_SCATTER_JITTER_H` is the number if it reads as a
   plane.
7. **The name.** `CHOIR_1` is a working name.
