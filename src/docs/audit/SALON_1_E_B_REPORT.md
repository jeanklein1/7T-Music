# SALON_1 — E-b REPORT: the fill tier is built, and it has nowhere to go

Read at `5490cb1`, the commit that landed the mechanism this report measures.
**No dial was turned.** The four fill dials and `DENSITY_TIERS` carry the values
the E-b document authored, verbatim.

E-b's §5 asked for the positional verification before trusting any new field,
§6 for the console instrument, §7 for the visual gate. The first two are done
and pass. **The third cannot be answered**, and this report is why: on today's
dials the fill tier places nothing on **99 % of FLAT walls** and **exactly one
row** on VAULT. R1 tops out at **4.0 %** of a FLAT wall and **13.8 %** of a
VAULT wall, against a requirement that reads *majority*.

This is arithmetic, not a defect in the walk. The walk is correct; there is no
room for it.

---

## §1 — THE VERIFICATION E-b §5 REQUIRED

> `WALL_ART` is positionally brace-initialised — compile the struct and table
> standalone with pins on every field after the insertions before trusting any
> of them, as B3b and P1 both did.

Done, both directions, as B3b did.

**Holds.** Every field from `wall_count_t1` through `mix_snapshot_chance`
pinned to its authored value; the three buckets pinned member-by-member;
`DENSITY_TIERS` pinned against `DENSITY_TIER_COUNT` and against
`DENSITY_TIER_NAMES` (the console line indexes that array with `density_idx`,
so a short name table would be an out-of-bounds read at the instrument).

**Fires.** Two negative tests, because a pin that cannot fail is decoration:

| injected fault | caught by |
|---|---|
| one initializer dropped (`snapshot_gap`) | brace-conversion error at the table + every downstream pin goes non-constant |
| two same-type fields transposed (`snapshot_height_hi` ↔ `snapshot_gap`) | `snapshot_height_hi`, `snapshot_gap`, and the `fill range` invariant |

The second is the one that mattered. A dropped field is loud on its own; a
transposition between two `float`s is exactly the silent shift the pins exist
for, and it is caught three times.

Measured incidentally: `sizeof(WallArtConfig)` is **136 bytes**, 34 words, no
padding — the four insertions are all `float` and land in a run of `float`.

---

## §2 — THE FINDING

### 2.1 The vertical budget

A wall's usable vertical strip is bounded by R4's two clamps:

```
  usable = wall_height − top_margin − min_bottom_height
  FLAT   = 20.0 − 2.0 − 1.0 = 17.00
  VAULT  = 25.0 − 2.0 − 1.0 = 22.00
```

The band takes its own height plus a moat on each side:

```
  band + two moats = height + 2 × painting_clearance
    intimate  min  =  6.0 + 4.0 = 10.00   → 7.00 left
    statement max  = 14.0 + 4.0 = 18.00   → −1.00 left
```

A fill row costs `snapshot_height_hi` = **5.00**, and it **cannot straddle the
band** — the leftover is split into two sub-rects, so 7.00 of slack spread
across both sides buys nothing if neither side holds 5.00 alone.

### 2.2 `max_bottom_height` decides which side gets the slack

`gallery.hpp:2001` clamps a piece whose bottom sits above `max_bottom_height`
(4.0) back down to it. The band's own comment at `:2004` records that this
fires for nearly everything: intimate's natural bottom spans `[3.5, 8.0]`,
standard's `[1.5, 6.5]`.

So the band's bottom is pinned at 4.0 and the budget below it is fixed:

```
  room below = 4.0 − painting_clearance − min_bottom_height = 1.00 wu
```

**One quarter of what a row costs, on every wall, permanently.** The entire
budget is therefore spent upward:

```
  room above = (wall_height − top_margin) − (4.0 + h) − painting_clearance
  FLAT       = 18.0 − 4.0 − h − 2.0 = 12.0 − h
```

which reaches 5.00 only when `h ≤ 7.0` — inside intimate's `[6, 11]` and
nowhere else. `P(one piece ≤ 7.0)` = `0.25 × 0.2` = **0.05**, and it must hold
for the *tallest* of the 1–5 pieces on that wall.

### 2.3 Measured, over real seeds

Replaying the real chain — bucket → height → `y_offset` → all three clamps —
across 200 000 seeds, 742 097 walls:

| ceiling | `wall_height` | mean room above | mean room below | P(≥ 1 row) | mean rows |
|---|---:|---:|---:|---:|---:|
| **FLAT** | 20.0 | 1.94 | **−0.83** | **0.0105** | **0.0105** |
| **VAULT** | 25.0 | 6.23 | 0.23 | 0.948 | 0.948 |

The mean room *below* the band on a FLAT wall is **negative**. And VAULT's
0.948 is not "about one row on average" — it is **exactly one row or none,
never two**: 6.23 of mean headroom against a pitch of 6.5.

`pick_portal_mood` (`mood.hpp:1214`) splits indoor rooms 15 % FLAT / 15 %
VAULT, so **half of all indoor rooms are the FLAT row.**

### 2.4 R1, measured

Coverage = (band area + fill area) / (`wall_span` × `wall_height`), 60 000
seeds:

| ceiling | radius | usable span | band % | fill % | **TOTAL %** |
|---|---:|---:|---:|---:|---:|
| FLAT | 1 | 126 | 11.5 | 0.1 | **11.6** |
| FLAT | 2 | 226 | 6.9 | 0.1 | **7.0** |
| FLAT | 3 | 326 | 4.9 | 0.1 | **5.1** |
| FLAT | 4 | 426 | 3.8 | 0.1 | **4.0** |
| VAULT | 1 | 126 | 9.2 | 9.5 | **18.7** |
| VAULT | 2 | 226 | 5.5 | 10.2 | **15.7** |
| VAULT | 3 | 326 | 3.9 | 10.5 | **14.5** |
| VAULT | 4 | 426 | 3.1 | 10.7 | **13.8** |

R1 asks for the **majority**. The best cell in this table is 18.7 %.

---

## §3 — NO SINGLE DIAL REACHES

Each row is that one change against the authored baseline, everything else
held. Rows per wall; `full` wants 48 frames per wall, which is 2–3 rows at
radius 4 and about 8 at radius 1.

| change | FLAT rows | FLAT P(≥1) | VAULT rows |
|---|---:|---:|---:|
| **as authored** | **0.01** | **0.010** | **0.95** |
| `max_bottom_height` 4.0 → 2.0 | 0.08 | 0.076 | 1.02 |
| `max_bottom_height` 4.0 → 0.0 | 0.37 | 0.373 | 1.11 |
| `painting_clearance` 2.0 → 1.0 | 0.04 | 0.035 | 1.01 |
| `top_margin` 2.0 → 1.0 | 0.04 | 0.035 | 1.01 |
| `snapshot_height_hi` 5.0 → 3.0 | 0.15 | 0.151 | 1.12 |
| `snapshot_height_hi` 5.0 → 2.5 | 0.26 | 0.260 | 1.32 |
| `snapshot_gap` 1.5 → 0.75 | 0.01 | 0.010 | 0.95 |
| **all four fill dials, aggressive** | 0.42 | 0.417 | 1.62 |
| **+ `max_bottom_height` → 2.0** | 0.86 | 0.828 | 2.02 |

`snapshot_gap` moves nothing — it is the *within-row* spacing and the row
pitch, and the binding constraint is whether a single row fits at all.

Every fill dial turned to an aggressive setting, plus the band's own bottom
clamp, still lands at **0.86 rows per FLAT wall**. The only lever that clears
2 rows is shrinking the painting buckets — and **R2 closes that**: *paintings
keep the centre unchanged*. For the record, buckets at ×0.6 alone give 0.81
rows, and combined with everything above give 1.98.

**R1 and R2 are in direct tension on a 20-wu wall.** Paintings 6–14 wu tall,
pinned to a 4.0 bottom, are most of the wall's height by construction. There is
no setting of E-b's dials that makes the *vertical* complement a majority while
R2 holds.

---

## §4 — THE LEVER THE ARITHMETIC POINTS AT

The campaign has one item **HELD** whose trigger was recorded as *Jean's eye on
E-b*: **sub-rect packing**. The measurement fires that trigger analytically.

E-b's simplification — the band is a horizontal slab, so its complement is two
horizontal sub-rects — is exactly right about the *vertical* complement and
throws away the *horizontal* one. A band of three pieces is roughly 45 wu wide.
The usable span is 126 wu at radius 1 and **426 at radius 4**. So between
64 % and **89 % of the wall's width beside the band** is empty, band-height
tall, and discarded by the two-sub-rect split.

Adding the two lateral columns, same row walk, same dials, nothing else moved:

| ceiling | radius | as authored | **+ lateral columns** |
|---|---:|---:|---:|
| FLAT | 1 | 11.6 % | **23.4 %** |
| FLAT | 4 | 4.0 % | **24.0 %** |
| VAULT | 1 | 18.7 % | **27.6 %** |
| VAULT | 4 | 13.8 % | **28.9 %** |

It is the difference between a mechanism that is inert and one that works, and
it costs no dial and does not touch the centre band. **It is still not a
majority** — 24–29 % against R1's *majority*. Reaching a majority needs lateral
packing **and** dial movement, or a re-reading of what R1 means by it.

**Nothing was implemented.** The item is HELD; this is the arithmetic that says
the trigger has fired, not a proposal, and the two columns above are a
measurement of an unbuilt thing.

---

## §5 — THE ASSERT THAT WOULD HAVE CAUGHT THIS

`gallery.hpp:383` is the R4 clamp-conflict assert this stage added:

```cpp
static_assert(WALL_ART.min_bottom_height
            + WALL_ART.statement.height_hi
            + WALL_ART.top_margin
              <= MOOD_TABLE[MOOD_INDOOR_FLAT].wall_height,
    "R4: floor margin + tallest bucket + top margin must fit the FLAT wall");
```

It holds: `1.0 + 14.0 + 2.0 = 17.0 ≤ 20.0`. It is also the wrong claim for this
stage. **It proves a painting fits. E-b needs a fill row to fit**, which is a
different and much tighter statement — the same shape as the budget assert at
`:363`, which proves slots exist for frames that geometry never asks for.

The assert that would have failed at compile time is the one that says the fill
tier has somewhere to go. **It is not written here**, because writing it means
choosing which side of R1/R2 gives way, and that is the dial decision this
report exists to hand back. When the dials settle, that assert belongs beside
them, and it is the one that keeps this from recurring the next time a bucket
height or `max_bottom_height` moves.

---

## §6 — WHAT LANDED, AND WHY IT LANDED ANYWAY

The mechanism is committed. It is correct; it is starved. Three reasons it
belongs in master rather than in a branch:

1. **The console instrument is the diagnostic.** §6 asked for the tier in the
   line so a `bare` roll and a broken stage are distinguishable. It carries the
   split counts and the fill against its target (`gallery.hpp:2242`):

   ```
   [WallPainting] Placed 3 painting(s) + 0 snapshot(s) across 4 walls (AUTHORED, tier=full, fill 0/192)
   ```

   `tier=full, fill 0/192` is this report in one line, live, at every room
   entry. Without it the starvation reads as `bare` and is invisible.

2. **`bare` at 0.25 means today's room exactly.** A quarter of rooms are
   unchanged by design, and on FLAT the other three quarters are unchanged by
   arithmetic. Landing this changes nothing a player sees — it makes the dials
   and the instrument available for the decision.

3. **The walk is verified independent of the dials.** It terminates (each
   iteration advances `up_y` or `down_y` by the pitch, and a row that places
   nothing breaks the wall loop), it never breaches R4 in either direction, and
   it centres every row on real widths the way E-a centres the band.

One thing was changed inside the walk on evidence, and it is not a dial:
`gallery.hpp:2110` charges the moat only when the band exists.

```cpp
const float clear = band_any ? WALL_ART.painting_clearance : 0.0f;
```

E-a's trim can empty a wall — one statement piece wider than `usable_span` at
the smallest radius — and `band_top == band_bottom == paint_y_base` there.
Charging clearance against a slab that is not on the wall leaves a
`2 × painting_clearance` hole at the paint line with nothing to explain it. At
zero the two sub-rects meet exactly and the fill takes the whole wall, which is
what the seeding comment at `:1968` already promises. Verified both ways: as
written the fill spans `[9.04, 13.96]` continuously; with the moat charged it
spans `[11.04, 15.96]` with a 4.0-wu gap at the paint line.

---

## §7 — THE GATE, HELD OPEN

E-b §7 asks *does the room read as a hang?* and instructs: press 6 / 7 until
the console says `tier=full`.

**It will say `tier=full` and the wall will look identical to `bare`.** That is
not the stage failing to work; it is the stage reporting that its dials do not
reach. Pressing until `full` and finding nothing is the expected result on FLAT
and one thin row near the ceiling on VAULT.

The gate question is real and still worth Jean's eye — but it is a question
about the *dials*, and on today's numbers there is nothing at the gate to
judge. **The visual gate is held open**, and with it **E-c**, which §7 gates on
E-b being stamped.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| E-b — the fill tier | **landed; INERT — STOP on the dials** | mechanism `5490cb1`, report this commit | Mechanism, R5 roll, and the §6 instrument are in and verified; §5's positional pins hold and fire (two injected faults, incl. a same-type transposition). **The fill places nothing on 99 % of FLAT walls and exactly one row on VAULT**, because usable vertical is 17.0, band + two moats eats 10.0–18.0, and a 5.0 row cannot straddle; `max_bottom_height` pins the band's bottom so room below is a fixed 1.00 wu. R1 measures **4.0 % FLAT / 13.8 % VAULT** at r=4 against *majority*. **No single dial reaches** — all four fill dials aggressive + `max_bottom_height` → 0.86 rows/wall; only shrinking the buckets clears 2, and **R2 forbids it**. The arithmetic points at the **HELD sub-rect-packing** item: the band is ~45 wu of a 126–426 wu span, so 64–89 % of the wall's *width* is discarded by the two-sub-rect split; adding lateral columns gives **24.0 % FLAT / 28.9 % VAULT** — working, still not a majority. **No dial turned; nothing implemented.** One evidence-driven change inside the walk: no band, no moat (`:2110`). The assert that would have caught this at compile time is named in §5 and deliberately not written — it requires choosing which of R1/R2 gives way. **Visual gate held open; E-c gated behind it.** |
