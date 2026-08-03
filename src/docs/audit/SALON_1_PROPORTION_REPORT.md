# SALON_1 — PROPORTION REPORT: the weld restored, and what supply does next

Read at `3849ef0`. Three findings, none of them tuned.

---

## §0 — TWO PROCEDURAL NOTES

**The handoff's HEAD was behind master.** PROPORTION (final) states HEAD at
`ee5e076`. Master was at `0436905`, two commits ahead — the previous
PROPORTION's delete and count, pushed the turn before. Those are this
document's commits 2 and 3, and the document says so: *"Supersedes PROPORTION
as issued — commit 1 is new."*

Master was **not** rewound. Force-pushing over commits Jean may already have
fast-forwarded is the failure GIT LAW exists to prevent, and the end state is
identical either way; only the ordering differs. Commit 1 landed alone on top,
which still gives it the isolation *"land this alone"* asks for — arguably
more, since it is now the last delta before the gate rather than the first.

`origin/master == master` was confirmed before any anchor was derived.

**Commits 2 and 3 as landed, against the document.** Two deltas, both reported
when they landed and both inside branches the document offers:

| document | landed | why |
|---|---|---|
| `plan[24]`, budget `4 × 24 = 144` | `plan[INDOOR_MAX_ROW_COUNT]` = 19, budget `4 × 19 + 48 = 124` | *"report if a constexpr form of `max_span / target_spacing` is available"* — it is. `PATCH_EXTENT` through gallery's own include of `state.hpp`, `MOOD_TABLE` by the cohort order the R4 assert already uses. The array and the budget now move with `target_spacing` instead of trailing a padded literal. |
| `count = max(1, …)` | `count = min(bound, max(1, …))` | The array bound made unconditional rather than true-by-argument. One term; an index cannot outrun its storage if a future mood widens the room. |

`frames_per_wall_max` was also deleted in commit 2 though the document did not
name it — its only three readers were the budget assert's fill term,
`DENSITY_TIERS`' `full` row, and the fill row-array bound, all fill-system.

---

## §1 — D HAS TWO HALVES, AND THE HANDOFF NAMED ONE

The reader survey found every reader inside `place_wall_paintings` — **no
STOP** — but more symbols than the commit brief lists:

| half | symbols | what it did |
|---|---|---|
| **placement** (named) | `snapLayer`, `first_use` | one exhibition layer shared across many frames |
| **selection** (not named) | `planPromoted`, `planLastUse`, `plan_ordinal`, the spacing-rule branch | handed one **record** out to two frames |

Deleting only the placement half would have **kept the visible bug and paid
more for it**: selection still resolves two frames to the same record,
placement now allocates each a fresh layer, and the identical image draws twice
on two layers instead of one. *"One frame, one layer, one image"* requires the
selection half to stop producing duplicate records in the first place.

Both halves are gone. What replaces them is nothing: selection takes fresh
records until there are none, falls through to the other pool, and ends the
wall on the `break` that was already there.

**The layer-supply assert survives and its reasoning inverts.** It was written
last commit as *one layer per distinct record*, because `snapLayer` shared.
Demand is now one layer per **frame** — but with no repeats, frames are bounded
by distinct records, which is the same `2 × STAGING_LAYERS`. Measured over
20 000 seeds at every site type and both radii: **layers used peaks at exactly
32 and never more.** The exhibition array can therefore never be the thing that
ends a row. Content is — which is what makes the console a true reading of
supply, and what makes a supply stage possible next.

---

## §2 — THE GATE'S EXAMPLE LINE WILL NOT APPEAR

The document's gate reads:

```
[WallPainting] Placed 19 painting(s) + 0 snapshot(s) across 4 walls (AUTHORED)
```

19 authored pieces cannot happen. `to_load = min(57 on disk, STAGING_LAYERS)`
caps the authored pool at **16**, and with repeats gone 16 is the hard ceiling
on authored frames in a placement event. The real line at radius 4 is:

```
[WallPainting] Placed 16 painting(s) + 0 snapshot(s) across 4 walls (AUTHORED)
```

**and three of those four walls will be bare.**

Realised room totals, 20 000 seeds, all four walls, no repeats:

| snapshots banked | site | r | asked | paintings | snapshots | total | **walls hung** | peak layers |
|---:|---|---:|---:|---:|---:|---:|---:|---:|
| **0** | AUTHORED | 1 | 20 | 16.0 | 0.0 | 16.0 | **4.00** | 16 |
| **0** | AUTHORED | 4 | 76 | 16.0 | 0.0 | 16.0 | **1.00** | 16 |
| **8** | MIXED | 4 | 76 | 16.0 | 8.0 | 24.0 | **2.00** | 24 |
| **16** | SNAPSHOT | 1 | 20 | 4.0 | 16.0 | 20.0 | **4.00** | 20 |
| **16** | SNAPSHOT | 4 | 76 | 16.0 | 16.0 | 32.0 | **2.00** | 32 |
| **16** | AUTHORED | 4 | 76 | 16.0 | 16.0 | 32.0 | **2.00** | 32 |

Read the `walls hung` column. At radius 1 the room asks for 20 and supply meets
it, so all four walls hang and the result is the brief working. **At radius 4
the room asks for 76 against a hard 32, so the first wall or two consume the
entire pool and the rest are empty.**

This is the supply limit the handoff's AFTER section names — but it arrives
harder than *"a 19-piece wall in a SNAPSHOT room can only ever fill from 16"*
suggests. The loss at radius 4 is not density on every wall; it is **whole
walls**. Two mitigating facts:

- The walls are Fisher-Yates shuffled off `site_seed`, so it is not always the
  same physical wall left bare — it re-rolls with the room.
- The proportional count is being read correctly. The gate's stated failure
  signature (*"a long wall still shows 1–5"*) does **not** occur: the first
  wall asks for and takes 16–19.

**Reported, not tuned.** Three obvious levers exist and all three are supply or
distribution decisions, not this stage's:

1. **Raise `to_load`** past `STAGING_LAYERS` — the supply model, and the AFTER
   section already claims it as the next stage.
2. **Divide the pool across active walls** before selecting, so four walls get
   8 each instead of one getting 32. Distribution, not supply; it changes what
   "proportional" means when supply binds.
3. **Nothing** — accept one dense wall and three bare, on the grounds that
   supply is about to rise anyway and the bare walls are the argument for it.

---

## §3 — WHAT THE THREE COMMITS ADD UP TO

Against Jean's brief, in his words:

> populate the REST of the wall with the SAME configurations as we already had
> … more paintings/snapshots per wall, PROPORTIONAL to the size of the wall …
> more numbers when the wall had SNAPSHOTS or PAINTINGS or A MIX OF BOTH.

| clause | state |
|---|---|
| *same configurations* | **met.** Buckets, `site_type` mix, gap, centring, Y clamps and the trim on real widths are E-a's, untouched through all three commits. One system, one scale, no seam — the second system is deleted. |
| *proportional to the size of the wall* | **met at the ask.** 5 / 10 / 14 / 19 pieces at radius 1–4, derived from `usable_span / target_spacing`, with the plan array and slot budget riding the same constexpr. |
| *more numbers* | **met at radius 1, supply-bound above it.** The room asks proportionally; the staging arrays deliver 32. §2 is the whole of the gap. |
| *snapshots or paintings or a mix* | **met, and now honest.** With the weld restored the console's split is a true content count rather than a count of frames pointing at fewer images. |

The campaign's own arc, for the ledger: E-b built a second system and it was
starved; E-b2 corrected its geometry and made it work; PROPORTION deleted it
and put the same effect through the one system that was already there. The
deletion is 350 lines against E-b2's 240 added — **the net of the three E
stages is smaller than what preceded them**, and the behaviour Jean asked for
is now a single dial on a single row.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| PROPORTION — 1/3, revert D | **landed** | code `3849ef0`, report this commit | D had TWO halves and the handoff named one; deleting only `snapLayer`/`first_use` would have kept the repeat and paid a second layer for it, so the selection-side spacing rule (`planPromoted`, `planLastUse`, `plan_ordinal`) went too. No reader outside `place_wall_paintings`; **no STOP**. Layer-supply assert survives with inverted reasoning — demand is now per-FRAME but frames are bounded by distinct records, same `2 × STAGING_LAYERS`; **peak layers measured at exactly 32**, so the exhibition array can never end a row and the console is a true reading of supply. **THE GATE'S EXAMPLE LINE CANNOT APPEAR**: `to_load = min(57, 16)` caps authored at 16, so radius 4 reads `16 painting(s) + 0 snapshot(s)` **with three walls bare** — a room asking 76 against a hard 32 front-loads onto one or two walls. Radius 1 asks 20, supply meets it, all four hang. Walls are shuffled per room, so the bare ones move. The proportional dial IS being read — the stated failure signature (1–5 on a long wall) does not occur. **Reported, not tuned**; three levers named, all supply or distribution. Procedural: handoff HEAD `ee5e076` was two commits behind master; **master not rewound**, commit 1 landed alone on top, end state identical. glaw2 GREEN. |
