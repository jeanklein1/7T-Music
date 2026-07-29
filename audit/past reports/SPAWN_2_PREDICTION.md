> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# SPAWN_2 — the prediction, recorded before the build

Campaign: SPAWN_SWEEP v2. Handoff: SPAWN_2 (floaters leave the registry).
Base: master @ `84cf09b`. Committed **ahead of the W1–W4 code**, so the
ordering is provable from `git log` rather than asserted.

---

## 0. RULING 21 — PARTITION VERIFIED, EXACT

`grounded` is field 4 of `EntityFamilyTraits` (`contracts/entity_types.hpp`);
all nine `*_TRAITS` are positional aggregates.

| family | `grounded` | site |
|---|---|---|
| PYRAMID | `true` | `entity_pipeline.hpp:765` |
| ARCH | `true` | `entity_pipeline.hpp:922` |
| COLUMN | `true` | `entity_pipeline.hpp:390` |
| ANTENNA | `true` | `entity_pipeline.hpp:402` |
| PALM | `true` | `grounded.hpp:1145` |
| CACTUS | `true` | `grounded.hpp:1373` |
| BLADE | `true` | `grounded.hpp:927` |
| **SPHERE** | **`false`** | `spheres.hpp:149` |
| **CUBE** | **`false`** | `cube_behaviors.hpp:526` |

**`grounded == false` for exactly sphere and cube. No third family.** The
partition holds; the stage proceeds. `has_footprint` reads `true` at all nine,
confirming again that it has never distinguished anything.

---

## 1. PREDICTED CENSUS, FIRST OUTDOOR TRAVERSE AFTER THE STAGE

| observation | predicted |
|---|---|
| `sph` / `cube` — `claimed`, `delta`, `new` | **`—`** (non-participant, not zero) |
| `sph` / `cube` — `active` | **UNCHANGED** from pre-stage |
| the nine grounded families — `delta` | **0**, as before |
| TOTAL `claimed`, outdoor traverse | **roughly halves**, ~105 → ~53 |
| `footprints N/128` | comfortably off the cap; no near-saturation |
| ground beneath floaters | now spawnable |

The measured baseline this is predicted against (SPAWN_1 census, outdoor
traverse, t=471.7): `cube claimed 46 + sph claimed 6 = 52 of 105 occupied`,
registry at 80–98% of 128 all session, peaking at 125/128 at t=351.6.

### Second-order, and the more interesting prediction

If the registry stops saturating, the families that were losing the silent
`register_footprint` race should start appearing. **Saturation drops are
resolved by PopFamily order, so the losers are the tail: CUBE(9), GOL(10),
GALLERY(11).** Gallery spawned *once* in the entire measured session.

**Predicted: `gall` and `gol` active counts rise on a comparable traverse.**
Not by a specific number — the spawn chances are low and Poisson-noisy — but
the direction is falsifiable, and if gallery still spawns ~once then
saturation was not what was suppressing it and SPAWN_4's diagnosis carries
more weight than this stage's.

---

## 2. FALSIFICATIONS, NAMED IN ADVANCE

| observation | what it would mean |
|---|---|
| `sph`/`cube` **active changes** | the footprint was doing more than self-separation — something else read it. **Stop and report.** |
| TOTAL `claimed` **does not drop materially** | floaters were not the occupancy driver; the 52/105 reading was misread |
| **a grounded family's delta goes nonzero** | W1 skipped a step for the wrong family; the partition is wrong |
| **visible stacking** of spheres or cubes | ruling 22 was wrong; the values come back from git |
| `gall`/`gol` **unchanged** after saturation clears | saturation was not suppressing them; look to SPAWN_4 |

The first is the one to watch. `sph`/`cube` `active` must not move: this stage
removes a *placement-exclusion* constraint, and if the live population changes,
the registry was load-bearing for something the audit did not find.

---

## 3. WHY `—` AND NOT `0` (the W4 reasoning, recorded before it is built)

After W1, `sph` and `cube` hold a live `active` against `claimed 0`, forever,
by design. The census's stated law is *"two independent registries, one line,
and they MUST agree; a nonzero delta is a leak that names its family."*

Two rows that disagree permanently and correctly would teach the reader to
discount the delta column — and a column the reader discounts has stopped
catching the thing it exists for. `0` is a measurement; `—` is a statement that
the family does not participate in this registry. They are different claims and
the print must not conflate them.

TOTAL `delta` therefore sums only the grounded rows: TOTAL `active` reports
what exists (all twelve), TOTAL `claimed` naturally sums only registrants, and
their raw difference would report a leak that is not one.

## 4. THE SEAM CHOSEN FOR W4, AND WHY IT WAS FORCED

The census must answer "is this family a registry participant?" for **all
twelve** families — ribbon, gol and gallery have no `*_TRAITS` object at all.

`cartridge.hpp`'s include order settles it: `entity_pipeline.hpp` is included
at line **82**, *after* `machine/spawn_engine.hpp` at line **81**. So a
constexpr predicate living in `spawn_engine.hpp` — where the census is defined
— **structurally cannot see** `PYRAMID_TRAITS`, `ARCH_TRAITS`,
`COLUMN_TRAITS` or `ANTENNA_TRAITS`. Four of the nine are out of scope at that
point in the cohort.

`FAMILY_DISPATCH` sits post-class at the tail of `cartridge.hpp`, after every
include, and is already the seam the census uses for `active_count`. It is the
only place that can both read the nine TRAITS and state a value for the three
bespoke families.

So: **`FamilyDispatch` gains `bool grounded`**, initialised from
`<FAMILY>_TRAITS.grounded` for the nine — a *view* of the authored field, not a
second copy of the policy, so the two cannot drift — and stated explicitly for
the three that have no TRAITS (all `true`: ribbon is anchored, gol and gallery
register directly).

The handoff preferred no new row. The alternative would have been a second
authored copy of grounded-ness keyed on `PopFamily`, which is precisely the
duplicate-model failure this campaign keeps uncovering — the 9×9 theme tool
against a 12×12 world, the write-only count fields, the v1 spec's inverted
density fields. One authored source, one view of it.
