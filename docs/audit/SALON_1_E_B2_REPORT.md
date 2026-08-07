# SALON_1 — E-b2 REPORT: the ring, measured

Read at `f66bff5`; `origin/master == master` confirmed before any anchor was
derived, per the push-before-handoff clause.

**No painting moved. No bucket shrank. No dial was turned.** The four fill
dials and `DENSITY_TIERS` carry E-b's values unchanged.

§2 asked for the predicted table to be confirmed by simulation. It is confirmed
on lateral space and rows exactly, and the frame counts run **higher** than
predicted. §3's two aesthetic changes were **already the landed behaviour** —
verified with anchors below, which changes what the filmstrip is evidence of.
§4's assert form is reported here and deliberately not written; the strict
version of it is **false**, and the true one is weaker than §4 assumed.

---

## §1 — THE PREDICTED TABLE, CONFIRMED AND CORRECTED

§2's table against 20 000 seeds per cell, the ring extracted verbatim from
`gallery.hpp:2096-2330`. "y-lines" counts distinct row heights; each y-line
places up to two column segments, one per side.

| | usable span | lateral (both) | predicted rows | **measured y-lines** | predicted frames | **measured capacity** |
|---|---:|---:|---:|---:|---:|---:|
| FLAT r=1 | 126 | **77** ✓ | 2 | **1.86** ✓ | ~16 | **23.4** |
| FLAT r=4 | 426 | **377** ✓ | 2 | **1.86** ✓ | ~92 | **116.9** |

Lateral space is exact. Rows are exact. **The frame counts were conservative
by 45 % and 27 %** — the estimate priced the columns only, and the strips plus
a slightly narrower mean snapshot make up the difference.

Full capacity, ceiling lifted off:

| ceiling | r=1 | r=2 | r=3 | r=4 |
|---|---:|---:|---:|---:|
| **FLAT** | 23.4 | 54.4 | 85.6 | **116.9** |
| **VAULT** | 32.4 | 66.1 | 100.0 | **133.9** |

**`frames_per_wall_max` is now the binding constraint**, which is what §2 said
this stage is for. At the `full` tier the ceiling of 48 is reached on **74.7 %**
of walls at r=4 and **30.5 %** at r=1. Before E-b2 it was reached on none.

### The strips are genuinely the remainder

| ceiling | radius | columns | strips |
|---|---:|---:|---:|
| FLAT | 1 | 23.34 | **0.01** |
| FLAT | 4 | 48.00 | **0.00** |
| VAULT | 1 | 25.45 | **6.79** |
| VAULT | 4 | 48.00 | **0.00** |

The columns carry the hang. The strips fire only on VAULT at the smallest
radius, where the extra 5 wu of wall height opens a row the columns' 48-frame
budget has not already spent. They cost nothing to keep and they are the reason
a wall whose columns are squeezed to nothing still gets a fill — see §4.

### R4 holds

**Zero breaches in 320 000 wall simulations**, checked per frame on all four
edges: top against `wall_height − top_margin`, floor against
`min_bottom_height`, and both corner margins against `usable_span`. Highest
frame top measured is 18.00 on FLAT and 23.00 on VAULT — the limits exactly,
touched but never crossed.

---

## §2 — R1, AND WHAT IS NOW IN THE WAY

Coverage = (band + fill) / (`wall_span` × `wall_height`):

| ceiling | radius | at the 48 ceiling | ceiling lifted off |
|---|---:|---:|---:|
| FLAT | 1 | 25.2 % | 25.2 % |
| FLAT | 4 | 13.3 % | **27.2 %** |
| VAULT | 1 | 24.5 % | 24.6 % |
| VAULT | 4 | 10.7 % | **24.5 %** |

Against E-b's 4.0 % / 13.8 %, the ring is between **2× and 6.8×**. It is still
not a *majority*, and the reason has changed completely:

- **E-b was blocked by the rect model.** No dial reached, because the vertical
  complement had no room at any setting. That was the STOP.
- **E-b2 is blocked by packing fractions.** Rows are pitched at
  `snapshot_height_hi + snapshot_gap` = 6.5 while frames average 4.0 tall, so
  vertical packing is 0.62; within a row the gap costs another 0.75. The
  product against the usable rect lands around 27 %.

**Both of those are dials, and turning either one now does something.** That is
the difference this stage makes. `snapshot_gap` — inert in E-b's sweep, because
the binding constraint was whether a single row fit at all — is now a live
coverage knob, and the comment beside it (`gallery.hpp:339`, "0.589 coverage
ceiling; 1.0 gives 0.692") is finally describing something reachable.

**No dial was turned.** Which of them moves, and how far, is the gate's
question.

---

## §3 — THE AESTHETIC ITEMS WERE ALREADY LANDED

§3 asked for two changes. Both are already the behaviour as of E-b, verified:

**1. Heights vary within the row — already per-frame.** `gallery.hpp:2201`:

```cpp
ff.height = WALL_ART.snapshot_height_lo
    + cpu_hash_f(f_seed, 0u) * (WALL_ART.snapshot_height_hi - WALL_ART.snapshot_height_lo);
```

`f_seed` is `cpu_hash(w_seed, FILL_BASE + i * FILL_STRIDE)` with `i` the frame
ordinal, so it is redrawn per frame, not per row. Heights span `[3, 5]` — a
1.67× range.

**2. Frames centre on the row's line — already true, in the shader.**
`world.wgsl:10082-10098`, `compute_wall_painting_geometry`:

```wgsl
let hw = s.scale_x * 0.5;
let hh = s.scale_y * 0.5;
...
let corners = array<vec2<f32>, 4>(
    vec2(-hw, -hh), vec2(hw, -hh), vec2(-hw, hh), vec2(hw, hh)
);
out.world_pos = canvas_pos + right * c.x + s.up * c.y;
```

The quad is symmetric about `slot.position`. E-b passes `row_y` as that
position, so every frame in a row already shares a **centre line**, not a
baseline. There is no bottom-alignment anywhere in the wall-frame path.

Aspect is not the flattener either: `SHOT_PARAMS` spans `[0.56, 2.39]`
(`gallery.hpp:103-111`) and `capture_snapshot` draws uniformly within the
picked shot type's range (`:828`).

**So the filmstrip was structural, not stylistic.** E-b placed at most **one
row**, on VAULT only — and one row of images is a filmstrip by construction,
whatever its heights do. What breaks it is the second row and the two columns,
which is E-b2. The two named changes needed no edit; the code comment at
`:2197` now records why they are the way they are, so the next reader does not
re-derive this.

Nothing else in §3 was changed. Row pitch, row centring on real widths and the
`frames_per_wall` ceiling are E-b's, untouched.

---

## §4 — THE ASSERT'S FORM, REPORTED

§4 asks for the form before it is written, on the ground that with the columns
nothing gives way. **The strict invariant is false**, and the measurement is
what says so.

### The invariant §4 proposes, tested

> the lateral columns admit at least one row

A column's half-width is `(usable_span − band_width)/2 − painting_clearance`.
The widest snapshot a fill row can contain is constexpr:

```
snapshot_height_hi × max(SHOT_PARAMS[*].aspect_hi) = 5.0 × 2.39 = 11.95
```

Over 4 000 000 walls, with band aspects drawn the way the band actually draws
them (15 % SNAPSHOT_ONLY plus MIXED take `SHOT_PARAMS`; authored take the
library's measured 1.09):

| ceiling | radius | mean column | **worst column** | P(column too narrow) | P(zero fill on the wall) |
|---|---:|---:|---:|---:|---:|
| FLAT | 1 | 37.4 | **−2.00** | **1.80 %** | **0.244 %** |
| FLAT | 2 | 87.4 | 36.19 | 0 | 0 |
| FLAT | 3–4 | 137–187 | 86–136 | 0 | 0 |
| VAULT | 1 | 37.4 | **−2.00** | **1.80 %** | **0.035 %** |
| VAULT | 2–4 | 87–187 | 36–136 | 0 | 0 |

The worst column is **negative**: at the smallest room a five-piece band of
wide shots plus its two moats overruns `usable_span`, because E-a's trim bounds
the band by `usable_span` and this panel has no say in it. So a column that
admits nothing is **reachable by design on 1.8 % of the smallest rooms**, and
an assert claiming otherwise would be a false statement that happens to compile.

Two things follow, and both are already right in the code:

- **The `span <= 0.0f` guard at `gallery.hpp:2156` is load-bearing, not
  defensive.** It is what a negative column hits. Without it the row planner
  would compare real widths against a negative budget, place nothing, and the
  wall would look identical — the failure would be silent.
- **The strips are the fallback.** P(zero fill) is 0.244 % on FLAT r=1 against
  1.8 % of squeezed columns, so roughly seven in eight squeezed walls are
  rescued by the strips above and below the band. On VAULT it is nineteen in
  twenty. That is the whole reason to keep two rects that place ~0 frames in
  the ordinary case.

### The form that IS true, and can be written

The unconditional half, which needs nothing that is not constexpr:

```cpp
// The fill tier has somewhere to go: one row fits the shortest wall's usable
// height, whatever the band does horizontally.
static_assert(WALL_ART.snapshot_height_hi
              <= MOOD_TABLE[MOOD_INDOOR_FLAT].wall_height
                 - WALL_ART.top_margin - WALL_ART.min_bottom_height,
    "fill: one row must fit the shortest wall's usable height");
```

`5.0 ≤ 20.0 − 2.0 − 1.0 = 17.0`, holding with **12.0 wu** of spare. This is the
statement E-b's report wanted and could not make, and it is true without
choosing between R1 and R2 — because nothing gives way.

The horizontal half cannot be written without a band-width ceiling, and there
is none: `band_width = Σ hᵢ × aspectᵢ + (n−1) × painting_gap`, and `aspectᵢ`
for authored content is `width/height` off disk at load (`gallery.hpp:1520`).
Three ways to get one, none of them free, none of them mine to pick:

1. **A constexpr aspect ceiling clamped at load.** Makes the assert exact. It
   is a change to how authored content enters the world — the supply model,
   out of scope since Amendment I.
2. **Assert against `SHOT_PARAMS` only** and accept that it says nothing about
   AUTHORED rooms. Provable, and 80 % of rooms fall outside the claim.
3. **Assert the geometric budget instead of the outcome** — that at
   `finite_radius_min` the usable span exceeds `2 × painting_clearance` plus
   two widest snapshots plus a *stated* band budget, and let the band budget be
   a documented `WALL_ART` number rather than a derived one. Turns an
   unprovable claim into a dial, at the cost of a dial nobody asked for.

**Recorded, not chosen.** The unconditional assert belongs beside the budget
assert at `gallery.hpp:363` the moment it is stamped; the horizontal one needs
a ruling first.

### One assert this stage DID write

`gallery.hpp:2288-2291`, and it is a different animal — a local array bound of
the same family as `frames_per_wall_max <= 64`:

```cpp
static_assert(MOOD_TABLE[MOOD_INDOOR_VAULT].wall_height
            - WALL_ART.min_bottom_height - WALL_ART.top_margin
              <= 16.0f * (WALL_ART.snapshot_height_hi + WALL_ART.snapshot_gap),
    "fill ring: 16 rows must span the tallest wall at the current pitch");
```

`ring_rows[16]` is the ring's only fixed bound. Today the tallest wall needs
3.4 rows, so 16 is generous — but `snapshot_gap` and `snapshot_height_hi` are
exactly the dials §2 above says are now live, and turning both down far enough
would silently truncate the ring. This fires instead. It is not §4's assert and
does not stand in for it.

---

## §5 — TWO CORRECTIONS TO THE E-b REPORT, ACCEPTED

**§4's lever was misnamed.** `SALON_1_E_B_REPORT.md` §4 said the lateral
columns fire the HELD sub-rect-packing trigger. They do not — HELD is
`uv_offset` texture packing, a capacity-and-distinctness question that would
cost a GROWTH-LAW struct edit; the columns are layout. Two different things
that both got called "sub-rect". **HELD stays held**, and its trigger is Jean's
eye on a *filled* wall, which is what E-b2 produces for the first time. The
correction is recorded inline in that report.

**§5's un-written assert.** Reported above. §4's premise — that with the
columns nothing gives way — is right about R1/R2 and is what makes the
unconditional form writable. It does not extend to the horizontal claim, for a
reason that has nothing to do with R1 or R2: the band's width is content, not
geometry.

---

## §6 — WHAT DOES NOT CHANGE

Held, and verified rather than asserted:

- **No painting moves.** The band's selection, trim, centring, Y offsets and
  all three clamps are E-a's, byte-identical. E-b2 only *reads* the band —
  `band_left`/`band_right` are tracked in the same place `band_top`/`band_bottom`
  already were (`gallery.hpp:2078-2088`), over what actually placed.
- **No bucket shrinks.** `intimate` / `standard` / `statement` unchanged;
  `WALL_ART`'s positional pins re-run at this HEAD, still `ALL POSITIONAL PINS OK`.
- **No dial is turned.** All four fill dials and `DENSITY_TIERS` hold E-b's
  values.
- **`max_bottom_height` stays at 4.0.** It is why the vertical complement was
  starved; laterally it is irrelevant — and it is now load-bearing in a second
  way, since `band_mid` is the band's real centre after that clamp
  (`gallery.hpp:2141`) and seeding the columns from the nominal `paint_y_base`
  would start the fill up to 5 wu above the art it flanks.
- **glaw2 GREEN** at this HEAD.

---

## §7 — THE GATE

The console instrument is E-b's, unchanged, and now says something:

```
[WallPainting] Placed 12 painting(s) + 48 snapshot(s) across 4 walls (AUTHORED, tier=full, fill 48/192)
```

`fill 48/192` at the `full` tier is expected on 74.7 % of the largest walls and
30.5 % of the smallest. A short read — `fill 31/192` — is the geometry binding
before the ceiling does, not a failure. `fill 0/192` at `tier=full` should now
be **rare**: 0.244 % of FLAT walls at the smallest radius, and only there.

**The question stands: do the snapshots run out to the sides of the paintings,
on the same line, on every wall?** The first row in each column sits on the
band's own vertical centre, so "the same line" is answerable directly.

Performance: at the 48 ceiling a four-wall room draws up to 192 wall frames,
which is what the slot budget assert was proved against and what B5's live
`slotHighWater` draw count scales to. Geometric capacity is 117–134 per wall,
so `frames_per_wall_max` is the knob if frame time moves — up or down, and the
budget assert bounds it in the up direction.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| E-b2 — the sides | **landed; THE VISUAL GATE** | this commit | The rect model corrected: the band is a rect, not a slab, and its complement is a **ring** — two lateral columns at full usable height plus E-b's two strips as the remainder. Columns first, seeded on the band's real vertical centre and alternating outward and left/right. §2's table **confirmed exactly** on lateral space (77 / 377) and rows (1.86 y-lines); frame capacity runs **higher** than predicted — 23.4 at FLAT r=1, **116.9** at r=4. `frames_per_wall_max` is now **binding**, reached on 74.7 % of r=4 walls. R1 goes 4.0 % → **27.2 %** FLAT, 13.8 % → 24.5 % VAULT; still not a majority, but the blocker is now **packing fractions, which are dials**, not the model. **Zero R4 breaches in 320 k walls.** §3's two aesthetic items were **already landed** — heights are per-frame (`:2201`) and the quad centres on `slot.position` (`world.wgsl:10082`); the filmstrip was *one row*, which is structural. §4's strict assert is **FALSE**: a column can be negative-width on **1.8 %** of smallest rooms because E-a's trim bounds the band by `usable_span`, so the `span <= 0.0f` guard at `:2156` is load-bearing and the strips rescue seven of eight such walls (P(zero fill) 0.244 %). The **unconditional** form is reported and holds with 12.0 wu spare; the horizontal half needs a band-width ceiling that is not constexpr — three routes recorded, none chosen. One local assert written: `ring_rows[16]` must span the tallest wall at the current pitch (`:2288`). **No painting moved, no bucket shrank, no dial turned.** HELD (`uv_offset` packing) stays held. glaw2 GREEN. |
