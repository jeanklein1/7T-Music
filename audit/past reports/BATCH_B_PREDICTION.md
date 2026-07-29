# BATCH B — stop condition, and the prediction

Campaign: SPAWN_SWEEP v2. Base: master @ `586ceff`.
Committed **before** the SPAWN_4 / SPAWN_3 code, so the ordering is provable
from `git log`.

---

## 1. SPAWN_5's STOP CONDITION — **NOT CLEARED. The sweep stays.**

`unregister_footprints_for_patch` is **not deleted**, and SPAWN_5 leaves this
batch. Here is the work behind that.

### What I could establish structurally

`MAX_ENTITY_REFS = 16` (`contracts/surface_services.hpp:98`).

**Ten recording families, not twelve.** Enumerated every `record_entity` call
site: PYRAMID, ARCH, COLUMN, ANTENNA, PALM, CACTUS, BLADE, GOL, GALLERY, RIBBON.
**SPHERE and CUBE never record** — both committers say so explicitly
(`spheres.hpp:230-236`, `cube_behaviors.hpp`): their lifetime was decoupled
from the host patch, and the GPU-side distance test is their sole eviction
path. The struct comment's "10 recording families" is exact.

**One select pass per patch, ever.** `spawn_selected_patches` filters
`phase == ALLOCATED` (`patch_system.hpp:763`) and sets `SPAWNED` immediately
(`:355`). `generate_selected_patches` handles `SPAWNED || NEEDS_REGEN` but does
not select. So a patch is never re-selected in its lifetime.

**≤1 entry per family per pass** — `select_entities_for_patch` walks
`PLACEMENT_ORDER` once and pushes at most one entry per family.

**Outdoors, host patch == trigger patch.** Every `POSITION_JITTER` is ≤ 0.45
(max is cube/sphere at 0.4, cactus 0.45), and `jittered_position` offsets by
`(hash − 0.5) × PATCH_EXTENT × jitter` → at most ±11.25 wu against a patch
half-width of 25. An entity cannot leave its trigger patch by jitter alone.

**Outdoor bound: ≤ 10 own families + 1 foreign ribbon tip = 11 of 16.**
(`MAX_RIBBON_INSTANCES = 1`, and a ribbon registers with its two tip patches,
so one foreign ref is the most any patch can receive.) Comfortable, and
provable.

### Why I did not clear it anyway

**`indoor_bounds_clamp` breaks the host == trigger invariant**, and that is the
whole difficulty. Two sub-cases:

- *Normal clamp* — the candidate is pushed to the nearest point of the legal
  box. Entities from different patches clamp to different boundary regions, so
  this does not concentrate. Corner patches map both axes to one point, but only
  their own ≤10 families reach it.
- *Collapsed box* (`lo > hi`) — **every MARGIN family lands on the exact room
  centre**, regardless of which patch triggered it. This is a genuine
  many-patches-to-one-patch map, and seven of the ten recording families are
  MARGIN (`INDOOR_TREATMENT`: pyramid, arch, column, antenna, palm, cactus,
  blade — gol is FREE, gallery and ribbon are FULL).

That collapse case *is* self-limiting: all seven are `grounded`, so
`check_position` runs, and colocated bodies at distance 0 reject each other
because `effective_min = radius + radius + MIN_SEPARATION` is positive for
every pair. Only the first survives; the rest fail place and never commit, so
they never record a ref. Bound holds at ~11.

**But that last step rests on data, not on structure.** It holds because
`MIN_SEPARATION` values and footprint radii happen to be non-degenerate. And
SPAWN_3 is precisely the stage that makes `MIN_SEPARATION` retuning necessary —
the campaign says so itself ("the real cost is paid in `MIN_SEPARATION`
retuning"). A deletion whose safety depends on numbers that the very next
tuning pass is expected to move is not a deletion I can call proven.

**And I cannot answer the third question the stop condition asks** — *has the
line ever fired in a log?* There is no binary and no log in this container. The
one empirical check that would settle it is unavailable to me.

### What would clear it, for whoever picks this up

```
grep "entity_ref OVERFLOW" <any captured session log>
```

Silent across a long outdoor traverse **and** an indoor session in the smallest
room (`finite_radius = 1`, where the collapse path is most reachable) → the
bound is empirically clean and the sweep can go.

One thing worth recording either way: **the overflow already leaks today, and
the sweep only masks half of it.** A dropped ref means `evict_patch_entities`
never calls that family's evictor, so the entity's *slot* is never freed —
permanently, sweep or no sweep. The sweep reclaims the *footprint* only. So the
overflow is a pre-existing slot leak that deleting the sweep would widen into a
ground leak as well.

**Consequence for this batch:** commits 1–4 land, commit 5 does not. Per the
handoff, that is a good outcome — the two release paths coexist harmlessly and
the sweep becomes near-redundant rather than wrong.

**And the ribbon orphan does not close.** `ribn active 1 claimed 0 delta −1`
at t=305.6 was SPAWN_5's sharpest gate; without the deletion, footprints still
die with patches, so a ribbon far from its anchor patch keeps losing its
ground. Predicted below as **unchanged**, not fixed.

---

## 2. PREDICTION

`delta = claimed − active`, per the census.

| observation | predicted | why |
|---|---|---|
| `arch` delta, finite world | **0** | ruling 20 then 14: Channel B writes its host patch, then registers |
| `[DIAG:INDOOR-SKIP] gall` in a 3×3 room | **gone** | radius falls from 123 to ~50 against a legal 55 |
| indoor galleries | **appear** | same |
| `gall` active, outdoor traverse | **rises** | honest radius over-claims less ground |
| the nine grounded families' delta | **0** | evictor release hooks |
| `footprints N/128` | **unchanged**, ~45–65% | ruling 8; SPAWN_4 frees ground but SPAWN_3 adds Channel B |
| `ribn` delta far from anchor | **−1, UNCHANGED** | SPAWN_5 did not land — see §1 |

### Falsifications, named in advance

| observation | what it means |
|---|---|
| a grounded family's delta goes nonzero | an evictor's release hook is missing — and the census names the family |
| `gall` still skipped at R=2 | the radius did not really move; check the computation was **relocated**, not duplicated |
| `arch` delta still negative in a finite world | ruling 20's host-patch write landed *after* registration |
| occupancy climbs across a long traverse | a release path is missing; with the sweep still in, this should be impossible for patch-hosted families |
| `entity_ref OVERFLOW` appears | §1's bound is wrong — and the sweep must stay regardless |

### The sharpest gate

`arch` delta reaching **0** in a finite world. It has been measured at exactly
`−2` (two Channel B portals) and `−4` (four), twice, in two different worlds.
It is an integer with a known cause, and SPAWN_3c is the only thing that can
move it.
