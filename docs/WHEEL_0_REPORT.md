# WHEEL_0 — THE INTERVAL WHEEL · campaign report

**Landed on `master`.** Working name; naming is Jean's gate. Three commits
plus a ledger settle. Every gate CC can run is green; the probe and G-LAW 1
are Jean's, and this campaign is almost entirely a VISUAL claim, so the desk
list at the end is the real verdict.

---

## §1 — WHAT LANDED

### U1 — the five axes (`532f52f8`)

`PanelSurface` gains a `Wheel` block beside `Possession`:

```cpp
struct Wheel {
    float step;       // the transformation axis: 1 chromatic … 7 fifths
    float radius;     // inner rank's radius (wu)
    float rank_sep;   // wu added per rank outward
    float twist;      // radians of rotation added per rank
    float phase;      // radians — the whole wheel's rotation
} wheel;
```

`PANEL_TABLE` rest `{ 1.0f, 60.0f, 14.0f, 0.0f, 0.0f }`, held by two
static_asserts: that the rest IS the chromatic circle (step 1, unrotated,
untwisted) and that `radius` and `rank_sep` are both positive, or the ranks
collide.

### U2 + U3 — the serve and the mode fold (`a1cca702`)

**Landed as one edit, and they were always going to.** The serve cannot
exist without a `WHEEL` state to be served in; the fold cannot stand without
a serve to be the transition it removes. Reported as two, committed as one,
and the commit says so.

**The law**, in `wheel_station(w, k)`:

```
pc = k % 12,  r = k / 12
theta  = phase + twist·r + (2π/12)·wrap12(step · pc)
radius = radius + rank_sep·r
```

`wrap12` lifts a negative `fmod` (a backwards wheel is a legal setting) and
wraps the **continuous** product — no rounding, no shortest-arc. A key
crossing the seam walks the long way, and the glide door absorbs it. The
braid is the percept.

**The serve** is one pass per frame over the twenty-four keys: compute the
station, compare against the station that key was **last served at**, and
poke `upload_cube_glide_target` only past `WHEEL_SERVE_EPS = 0.05` wu. This
is the poke gate's own reading, deliberately: comparing against the last
SERVE rather than the last frame is the difference between thinning and
stopping, so a wheel turning slower than epsilon per frame still accumulates
and pokes.

**XZ only.** Heights are the tiers' calm band (CHOIR_1's) and the wheel
never authors one.

**The fold.** `Formation` is `{ ROAM, WHEEL }`, rest `WHEEL`. Retired with
tombstones: the whole LATTICE band and the helix pair with both witnesses,
the screen and scatter station bands, the walk's three numbers, the climb,
the settle, the reseat watch, the hand-back, `stage_wait`, `stations_sent`,
`walk_[]`, `settled[]`, `ZOETROPE_REST_DIM`.

**`ZOETROPE_SWELL_GAIN` survives, and moves twice.** It is gated on `WHEEL`
where it was gated on `SCREEN` — and it multiplies **the mirror's own tier
draw** where it multiplied the screen's uniform pixel, because the screen
made every cube a pixel and the wheel does not touch bodies at all. A
Monolith swells like a Monolith. The projector now writes `body_radius` in
**both** modes with the gain zeroed off-wheel, which makes it self-restoring
the way the face variance already was; that is affordable only because the
CPU walk that used to contend for that scalar is gone, leaving the projector
the one writer outside the birth.

**`repaint_all` keeps one edge where it had three** — the ROAM↔WHEEL flip,
which changes what a lit key looks like without moving its light. The dim's
two edges went with the dim; the arrival went with the walk.

### U2c — the settle (`6440e302`)

Provenance only. `COMMAND_LEDGER` and `MIRROR_LEDGER` stamp the last commit
touching a file they scan. The standing rule from CHOIR_0 held for the third
campaign running: **a follow-up settle is owed exactly when the commit
before it moved `audit/BINDING_LEDGER.md`.** Verified to a fixed point after.

---

## §2 — THE FINDING A GATE MADE

**`birth_station` read `PANEL_TABLE`, and `organ_gap --gate` refused it on
the first run.**

I wrote it that way on a determinism argument: the birth is on the protect
list, so let it stand on the REST wheel and no scene applied before world
build can move the instrument. **The argument answers the wrong question.**
What the protect list guards is the BODY draw — `CHOIR_SEED` and the tier
table — and that was never in question. Where a key STANDS is the wheel's
business, and the wheel is a dial.

More to the point, `PanelSurface` is a **graduated pair**, and the law of
one is that the design table is a seed and an assert subject and never a
runtime read. That is precisely the class the reader witness exists to
catch, and it caught it by name and line:

```
PANEL_TABLE  definition=1 seed=1 static_assert=11 comment=7 violation=1
      VIOLATION  bodies/cube_behaviors.hpp:656  return wheel_station(PANEL_TABLE.wheel, k);
```

**The fix made ROAM better than the draft.** With `birth_station` reading
`PANEL_LIVE`, ROAM could no longer be "recompute the birth station" —
that would make letting go of the wheel still follow the wheel's dials, the
one thing ROAM exists not to do. ROAM aims at a **recorded** birth anchor
(`birth_ax/az`, seeded once from the target the birth actually wrote, never
rewritten afterwards). A record, not a recomputation: the wheel as it stood
on the day, frozen. Which also gives the mode flip a clean fixed point —
until a dial is turned, both arms name the same point and the flip costs
nothing but the projector's forced pass.

---

## §3 — THE FOUR FLAGS

### 1 · The star-polygon degeneracy is real, and is not guarded

At `step = 12m/d` two distinct pitch classes land on the same bearing, and
within a rank on the same POINT. Verified numerically against the shipped
law:

| sweep | result |
|---|---|
| rest wheel (step 1) | min pair separation **14.000 wu** (the rank gap) |
| step 7 (fifths) | min pair separation **14.000 wu** |
| step 0.01 → 6.00, 600 samples | worst min separation **0.000 wu**, at step **1.20** |

At step 1.20, keys 0 and 10 coincide exactly: `1.2 × 10 = 12 ≡ 0`.

**This is the law's own content, not a defect.** At `step = 6` the wheel
collapses onto the tritone axis, at 4 onto a triangle, at 3 onto a square —
those are the musical facts the axis exists to draw, and turning through
such a step is the braid passing through itself. Named here because nothing
in the tree says it out loud, and because it means **cubes will
interpenetrate at some settings** and that will read as a bug to anyone who
was not told.

### 2 · `rank_sep = 14` clears the bodies barely, and not at all on the tail

The tight pair is same-pitch-class across adjacent ranks. Worst by tier
means: key 10 (MedCube, radius mean 3.0) against key 22 (LargeCube, mean
5.0) — 8.0 wu of half-extent across a 14 wu gap, and **12.8 wu at full
swell** (×1.6). Clears by 1.2 wu at the means; does not clear on the upper
tail of the draws (σ 0.60 and 1.00), before aspect ratios are counted.
`rank_sep` is a live dial and this is a desk number, not a fix.

### 3 · The LATTICE band retired too — one step past the stated list

§U3 names "the helix pair **and now its constants**".
`LATTICE_ROWS/COLS/CELLS` are a separate band, and every reader of theirs
left with the two station functions. Retired rather than left standing dead,
under the tree's own "living matter only" law, and the one surviving assert
re-aimed to the constraint that was always the real one:

```cpp
static_assert(CUBE_CHOIR_N <= Dim::MAX_CUBE_INSTANCES,
    "a key is a SLOT (key k = slot k, by construction) — the choir may not outrun the slots");
```

### 4 · §U4's "the door's scripted cycle re-aimed" has no subject

The shell gate scripts `door 3` (rebirth) and `door 99` (the out-of-range
refusal). It never presses door 5, and **no tool under `tools/` mentions the
zoetrope at all**. There was nothing to re-aim.

What I did change: door 5's **label**, on the reading that the label is the
verb text §U3 asks to rename —
`"Zoetrope: gather / reveal / release"` → `"Wheel: take the choir / let it
roam"`. The identifier `ORGAN_DOOR_ZOETROPE` and its bit stand, so nothing
renumbers, and the fuller rename (the function, the constants' prefix, the
`INSTRUMENTS.zoetrope_witness` dial) stays Jean's gate.

**A fifth, minor:** §U1 says "the four paste-ready lines" for a block with
**five** axes. Five are below.

---

## §4 — THE PARKED ORGAN ROWS

Five rows, PARKED under the ORGAN_REST registry freeze. Paste-ready into
`src/console/organ_params.inc`, beside the `PANEL` block:

```cpp
// ── THE INTERVAL WHEEL (WHEEL_0) ── the choir's formation, five axes.
// step is the TRANSFORMATION AXIS: 1 is the chromatic circle, 7 the
// circle of fifths, and the walk between passes every star polygon.
ORGAN_PARAM(PANEL, PanelSurface, wheel.step,     F32,  0.0f,  12.0f,  0.01f, "Instruments · Wheel", "step")
ORGAN_PARAM(PANEL, PanelSurface, wheel.radius,   F32,  5.0f, 200.0f,  0.5f,  "Instruments · Wheel", "radius")
ORGAN_PARAM(PANEL, PanelSurface, wheel.rank_sep, F32,  1.0f,  60.0f,  0.5f,  "Instruments · Wheel", "rank sep")
ORGAN_PARAM(PANEL, PanelSurface, wheel.twist,    F32, -3.15f,  3.15f, 0.01f, "Instruments · Wheel", "twist")
ORGAN_PARAM(PANEL, PanelSurface, wheel.phase,    F32, -6.29f,  6.29f, 0.01f, "Instruments · Wheel", "phase")
```

**Every range here is evidence, not taste**, per the file's own rule:

- **`step` 0 … 12.** 0 collapses every pitch class onto one bearing (a
  legitimate reading: "all keys at one hour"), 12 is a full turn per class
  and is equivalent to 0 by `wrap12`. The commission's span is 1 → 7 and
  sits inside. Negative steps work by construction (`wrap12` lifts them) but
  mirror the wheel rather than transforming it, so the floor is 0.
- **`radius` 5 … 200 wu.** Floor: below ~5 wu the ring is inside the largest
  body. Ceiling: 200 wu is past the world's own reach at
  `WORLD_RADIUS_PIN = 2` (a 250 wu box, 176 wu to a corner), so the top of
  the range already puts the wheel off the stage.
- **`rank_sep` 1 … 60 wu.** Must be positive (the panel asserts it). The
  desk number 14 clears the tier means by 1.2 wu at full swell (flag 2), so
  the useful band starts around there; 60 puts rank 1 at 120 wu.
- **`twist` and `phase` ± one turn**, in radians. A live `phase` is the
  named gen-2 coupling target (the pc-DFT's phase rotating the wheel) and is
  the one of the five most likely to want a per-frame voice.

**Cadence: `live`, all five.** Nothing is baked at spawn — the serve
recomputes every station every frame — so a turn of any of them is heard on
the next frame with no rebirth.

---

## §5 — THE GATES

| gate | verdict |
|---|---|
| TU gate | **PASS** — both tiers, zero diagnostics |
| shell gate | **PASS** — 5 scenes, the scripted session, the round trip |
| G-LAW 2 | **GREEN** — 257 fn, 249 const, 55 struct, 55 binding, 34 entry points; 27 symbols retired cleanly |
| score census | **GREEN** — 7 update + 15 render rows, bijection both ways |
| WGSL gate | **PASS** — naga, raw |
| binding surface `--check` | **PASS** (S-6 wants the pushed tip) |
| organ gap `--gate` | **PASS** — 0 surviving runtime readers across 14 graduated pairs *(RED first: see §2)* |
| organ ledger `--check` | **PASS** — 240 proved, 0 suspect |
| mirror census | **GREEN** |
| mirror offsets `--check` | **PASS** — 128 members, 7 structs |
| **the probe** | **JEAN'S.** Nothing in this campaign touches WGSL, a buffer stride or a binding — it is CPU-side formation logic and one panel block — but the whole claim is visual |

**No WGSL was touched, so no record ritual applies** (§U4's conditional).
G-LAW 2's baseline is untouched, and STAGE_0's held `--record` diff still
stands unrecorded and still needs Jean's word.

---

## §6 — THE DESK LIST

1. **The braid at a hand-turned `step`.** Set `PANEL.wheel.step` moving
   through 1 → 7 and watch twenty-four cubes weave past each other. This is
   the campaign. If this does not read, nothing else matters.
2. **The rest wheel.** `radius` 60 and `rank_sep` 14: does the inner
   twelve read as a clock, and does rank 1 read as a second ring rather
   than as noise? (Flag 2 says the swell is tight here.)
3. **The swell on the wheel.** Play a chord and watch lit keys grow — now
   proportionally to their own tier, so a Monolith should visibly out-swell
   a SmallCube where the screen made them identical.
4. **The ROAM↔WHEEL flip** (door 5). Turn a dial first, or the flip is a
   no-op by construction. Out: the choir walks home to the boot wheel and
   drift takes it. In: it walks back onto the live wheel.
5. **Interpenetration at rational steps** (flag 1) — decide whether it
   reads as the braid passing through itself or as a defect.
