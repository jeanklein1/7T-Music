# SALON_1 — 19b REPORT: `authored_staged_count`, and what the baseline actually is

Report-first stage. **No edit is in this commit.** Read at `2e00352`.

The E-split addendum §2 asks one question with a threshold attached:

> Report `authored_staged_count`: how it is populated, and its typical value.
> If it sits below 20, the majority path has been dropping frames all along,
> D's effect is far larger than 0.84 %, and the room about to be tagged as
> baseline is already unlike the one Jean remembers.

**It sits at 16 — below the threshold. The conclusion does not follow.**
The 80 % path does not drop frames on authored exhaustion; it falls back to
snapshots, at a line the hypothesis did not account for. The baseline is the
room Jean remembers.

But the same read turns up something that matters more, and it is about **E-b**,
not about the tag. §6 of Amendment II is applied to one path and not the other.

---

## §1 — HOW IT IS POPULATED, AND ITS VALUE

`load_authored_textures` (`gallery.hpp:1511-1535`):

```cpp
    uint32_t manifest_size = (uint32_t)gs.authored_disk_manifest.size();
    uint32_t to_load = std::min(manifest_size, Dim::STAGING_LAYERS);
    for (uint32_t i = 0; i < to_load; i++) {
        load_authored_image_to_staging(gs, gpu, queue, i, i, gs.authored_disk_manifest[i].c_str());
        if (gs.authored_staging[i].valid) gs.authored_staged_count++;
    }
```

The manifest comes from `scan_paintings_folder` (`gallery.hpp:1459`), which
globs `PAINTING_*.jpg|jpeg` under `assets/paintings` and sorts numerically.

**On disk today: 57 files**, all git-tracked (`assets/paintings/`, counted at
this HEAD). So:

```
  manifest_size          = 57
  to_load = min(57, 16)  = 16      <- Dim::STAGING_LAYERS
  authored_staged_count  = 16
```

**The cap is the code's, not the content's.** 41 of the 57 painting files are
never staged in a given world. `rotate_authored_staging` (`gallery.hpp:1537`)
cycles *which* 16 across teardowns via `authored_disk_cursor`, so the library
is reachable over a session — but at any one placement event the authored pool
is 16, and 16 is `STAGING_LAYERS`, not a content limit.

That answers the threshold: **16 < 20, yes.**

---

## §2 — WHY THE CONCLUSION DOES NOT FOLLOW

The hypothesis assumes an AUTHORED_ONLY room exhausting its 16 images drops the
remaining frames. It does not. `gallery.hpp:1814`:

```cpp
            if (!use_snapshot && count_unused_authored(gs, usedAuthored) == 0) {
                use_snapshot = true;
            }
```

**Authored exhaustion falls through to the snapshot path.** A frame is dropped
only when the *combined* distinct pool is exhausted — the two `continue`s at
`gallery.hpp:1851` and `:1904` both require `count_unused_authored == 0` *and*
no usable snapshot.

So the drop condition is `frames > 16 + N`, where `N` is the count of
unconsumed snapshot records:

| unconsumed snapshots `N` | pool | P(a room wants more) |
|---:|---:|---:|
| 0 | 16 | 4.05 % |
| 2 | 18 | 0.58 % |
| **4** | **20** | **0.00 %** |
| 16 | 32 | 0.00 % |

Four walls is the only wall-count that can exceed 16 at all — three walls tops
out at 15 — so the 4.05 % is `P(4 walls) × P(sum of four U{1..5} > 16)`
= `0.7225 × 35/625`.

`N ≥ 4` needs roughly two photographer triggers, about **100 world units
walked** (`TRIGGER_DISTANCE_MEAN` 50, E[shots] 2 per trigger). After that the
drop probability is **exactly zero at every content type**, because 20 is the
hard maximum frames a room can want and `16 + 4 = 20`.

**Conclusion:** the majority path has not been dropping frames all along. It
drops only in an early-session room entered before the photographer has fired
twice — and then it drops regardless of content type, because the pool is
shared. D's visibility estimate stands at ~0.6 % of indoor entries
(`0.15 × 0.0405`), close to the 0.84 % figure. **The room about to be tagged is
the room Jean remembers.**

One nuance worth having at the gate: in an AUTHORED_ONLY room wanting 17–20
frames, the last few are **snapshots**, not paintings. That is today's
behaviour, not something the campaign introduced, and it is invisible at these
counts.

---

## §3 — THE FINDING THAT MATTERS, AND IT IS ABOUT E-b

Amendment II §6 struck the uniqueness rule for the snapshot path:

> At the full tier that rule is unsatisfiable, and its failure mode is silent.
> … A uniqueness rule would find no free layer and either truncate the hang or
> spin. **A full-tier room would silently become sparse, and the dial would
> appear not to work.**

D applied that ruling to the snapshot path. **The authored path still runs the
uniqueness rule**, and it is the 80 % path:

- `usedAuthored[Dim::STAGING_LAYERS]` (`gallery.hpp:1676`) is declared **per
  call**, across all walls.
- `gallery.hpp:1904` — `if (auth_stg == UINT32_MAX) continue;`
- `gallery.hpp:1914` — `if (best == UINT32_MAX) continue;` after scanning for
  an unused record.

So authored images **cannot repeat within one placement event, ever.** At
today's ≤ 20 frames that is invisible: 16 authored covers almost every room,
and the fallback covers the tail.

**At E-b's full tier it stops being invisible.** 4 walls × 48 = 192 frames
against 16 authored images that cannot repeat:

| | authored | snapshot (D reuses) | dropped |
|---|---:|---:|---:|
| AUTHORED_ONLY room, full tier | **16** | 176 | 0 |

Nothing is dropped — D's reuse absorbs it — but **an AUTHORED_ONLY room at the
full tier is 92 % snapshots.** The room's declared content type survives only in
its first sixteen frames. That is not the failure §6 predicted (silent
sparseness); it is a quieter one: the dial works, the wall fills, and the
content type is overwhelmed by its own fallback.

**Reported, not fixed.** Three responses exist and the choice is not mine:

1. **Extend §6's ruling to the authored path** — give `usedAuthored` the same
   spacing-rule treatment D gave the snapshot side. Symmetric, and it makes the
   fill tier honour the site type.
2. **Raise the authored pool** — `to_load` is capped at `STAGING_LAYERS` while
   57 images sit on disk. This is the supply model, explicitly out of scope
   since Amendment I.
3. **Rule that the fill tier is snapshots by definition** — D1 already says the
   fill tier rides smaller footprints and the centre band holds the paintings.
   If the fill tier is *meant* to be snapshots, then 16 authored in the centre
   band plus snapshots everywhere else is the design working, and only the
   AUTHORED_ONLY label is misleading.

Option 3 is consistent with D1 as written and costs nothing. It is recorded
first among equals because it may already be the answer, not because it is
mine to choose.

---

## §4 — WHAT THIS CHANGES ABOUT THE TAG

Nothing. The baseline is sound and E-a's gate question is unaffected — the
drift bias the addendum §3 names is a geometry defect and has no interaction
with content supply.

The finding lands on **E-b's** gate, and belongs beside the symptom row the
addendum §4 just added:

| symptom at the gate | cause | response |
|---|---|---|
| An AUTHORED_ONLY room's fill reads as snapshots, not paintings | `usedAuthored` is a uniqueness rule; authored images cannot repeat, so 16 of 192 frames are authored | §3 above — extend the spacing rule, raise the pool, or rule the fill tier snapshot-by-definition |

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| 19b — `authored_staged_count` | **reported** | this commit | **16** (`min(57 on disk, STAGING_LAYERS)`) — below the threshold, but the conclusion does not follow: authored exhaustion falls back to snapshots (`gallery.hpp:1814`), so drops need `frames > 16 + N` and vanish once ~100 wu have been walked. **Baseline is sound.** Separate finding: the authored path still runs the uniqueness rule §6 struck — invisible at 20 frames, makes an AUTHORED_ONLY full-tier room 92 % snapshots at 192. |
