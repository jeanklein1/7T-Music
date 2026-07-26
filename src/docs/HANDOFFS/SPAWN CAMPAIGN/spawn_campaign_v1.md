# SPAWN_SWEEP — campaign v1

Cartridge: `the_board` (incubator_dual). Ratified this session.
Scope: the spawn machinery, end to end — gate, negotiation, registry,
commit, eviction, census.

---

## THE LAW

**A footprint is a claim on the ground, not a claim on space.**

Three corollaries, each load-bearing:

1. **A family registers iff its own extent touches the ground plane.**
   Pyramid, arch, column, antenna, palm, cactus, blade, GoL, gallery,
   ribbon (tips are anchored), portal arch — yes.
   Sphere, cube — no. They hover; the ground beneath them is free.

2. **A footprint has an owner: (family, slot). The owner releases it.**
   No by-patch sweep, no shared janitor. The hand that claims is the
   hand that frees.

3. **Any phase may reject; a rejecting phase releases what earlier
   phases reserved.** Select decides what, place decides where, commit
   writes — but discovery is allowed anywhere, and discovery obliges
   release.

Everything below is these three sentences applied.

### The distinction the registry is NOT

`check_position` is read from exactly one site — `negotiate_position`,
at spawn. It is a **placement-exclusion registry**: "do not put a new
thing where a thing already is." It is not runtime collision. Runtime
collision is the occupier query face on the GPU and never reads this
registry. Conflating the two is what produced the floater orphan.

### Three concepts, previously carried by one word

| Concept | Question | Home |
|---|---|---|
| Ground claim | is this patch of ground taken? | the footprint registry |
| Containment | does the whole body fit inside the room? | `indoor_bounds_clamp` (MARGIN / FULL / FREE) |
| Surface tenancy | does this wall have span left for a frame? | **absent** — SPAWN_7 |

Correctly separate today: 1 and 2. Absent entirely: 3.

---

## RULINGS RECORD

| # | Question | Ruling |
|---|---|---|
| 1 | Floater footprints | Things that move carry the footprint or don't have one. **Resolved subtractively:** floaters claim no ground → they leave the registry; self-separation goes local against their own arrays. |
| 2 | Portal arches, anchor ribbon | **Register.** A portal you can spawn a pyramid inside of is a real artifact. |
| 3 | Commit-stage rejection | **Dissolved for gallery** by the content reservation (SPAWN_4). Corollary 3 covers ribbon's tip case. |
| 4 | The three identity factors | `MOOD_SPAWN_MULT` is **live** (row 5 is the whole fin_ref mechanism) — keep. `GLOBAL_ENTITY_DENSITY` — keep as a declared idle dial. Density lattice — **give it range and look** (SPAWN_8). |
| 5 | `PLACEMENT_ORDER` as data | **Yes.** |
| 6 | The census | **Measure what it is named** — active AND claimed, side by side, per family. |
| 7 | Moving cubes | Superseded by ruling 1. Nothing to carry. |
| 8 | `MAX_FOOTPRINTS` | **Raise to 512**, loud line on saturation. |
| 9 | `unregister_footprints_for_patch` | **Delete — last.** After the census proves the per-owner releases are complete. |
| 10 | Capacity 512 + loud line | Stamped. |
| 11 | Gallery content reservation at place | Stamped. |
| 12 | Wall tenancy as its own mechanism | Stamped. |
| 13 | Formations as a named feature | **PARKED — last.** Adds noise now. |

---

## THE STAGES

Dependency-ordered. Commits stay separate for bisection; one end build
per stage. The census (SPAWN_1) is built **first** so every later stage
has a gate for an otherwise invisible system.

### SPAWN_0 — prose, tombstones, dead parameter
**Behavior:** none.
Targets: the `SPAWN UTILITIES` banner's citation of two tag tables that
do not exist; the empty `Property Index Registry` banner;
`evaluate_spawn_gate`'s stale `adjacency_mod` comment; `diag_name` on
`run_spawn_preamble` (unreferenced, ~10 call sites pass a literal for
nothing); the `<iostream>` include comment attributing itself to a
deleted DIAG block; the `prepare_pyramid_mesh_gen` tombstones in
`grounded.hpp` (two sites plus a `→ C6` reference).
**Gate:** `glaw1`. No visual gate needed.

### SPAWN_1 — the census, rebuilt
**Behavior:** none (adds the instrument).
`FamilyDispatch` gains one row: `uint32_t (*active_count)(const MachineCtx*)`.
Twelve one-liners, each reading its own family's array. The census then
prints per family:

```
[CENSUS t=  312.4 trigger=periodic]
  pyr   active 3  claimed 3
  arch  active 5  claimed 5
  sph   active 7  claimed 0
  gall  active 2  claimed 3
```

Two independent registries, one line. **They must agree.** A mismatch
is a leak — a body with no ground, or ground with no body — and it names
the family. It ships with its recipe: *active* is the family's own
array; *claimed* is footprints keyed to that family.

The per-entity detail listing stays (XZ and age live in the registry)
but is labelled **claimed ground**, not entities.

Two things fall out free: portal arches and the anchor ribbon appear in
a census for the first time (they live in `entities_state_.arches` and
`ribbon_state_.active[0]`, which the active side sees); and findings S1
and S2 become visible rather than argued.

**Gate:** the census prints. Expect it to be **noisy** — that is the
stage succeeding.

### SPAWN_2 — floaters leave the registry
**Behavior:** yes. Fixes S1 subtractively.
Sphere and cube stop registering footprints. Their `MIN_SEPARATION`
diagonal (`[Sph][Sph]=20`, `[Cube][Cube]=15` — self-only; every other
column is zero) moves to a local self-separation scan against
`activeSpheres_[]` / `activeCubes_[]`. Exact, correctly lifetimed, never
stale — where the shared-registry version was stale the moment corral
moved a cube.

The `MIN_SEPARATION` values stay where they are authored. Only the
reader changes.

**Gate:** census shows `sph`/`cube` claimed = 0, active unchanged.
Visual: floater spacing unchanged; ground under floaters now spawnable.

### SPAWN_3 — the footprint gets an owner
**Behavior:** yes.
- `GroundFootprint` gains `slot`. It already carries `family`.
- `unregister_footprint_for(family, slot)`.
- Release routed through every evictor.
- `force_spawn_portal_arch` registers; `evict_arch` already releases.
- The anchor ribbon registers at slot 0.
- `MAX_FOOTPRINTS` → 512, loud line when `register_footprint` returns
  the sentinel.

The loud line matters more than the capacity: today saturation is a
**silent skip decided by enum order** — pyramid always wins, gallery
always loses.

**Gate:** census quiets for every family except gallery and ribbon
(SPAWN_4's cases). Visual: entities no longer spawn inside portals.

### SPAWN_4 — the gallery becomes honest
**Behavior:** yes. Fixes S2; unblocks indoor galleries.

The current footprint is a fiction:
`footprint_r = PAINTINGS_MAX_BY_ARCHETYPE[archetype] * 0.5 * ROW_SPACING + 15`
— sized from the archetype **maximum**, before commit caps the count to
available content, and registered as a disc while commit lays the
paintings out as a **line**. Too big and too weak at once: it
over-claims perpendicular to the fan, over-claims again whenever fewer
paintings land, and constrains no individual painting.

This is very likely why indoor galleries feel absent — not failing to
fit, but failing a FULL containment test sized for a gallery that was
never going to be built.

The fix: **resolve the count at place by reserving staging layers
there**, recorded in `GalleryPlacement`. Commit draws from a reservation
instead of discovering scarcity. Then
`footprint_r = (count-1) * 0.5 * ROW_SPACING + painting_half + margin`
— honest, far smaller, and FULL containment starts passing in rooms
where a 3-painting gallery genuinely fits.

Ribbon's 0-tips rejection releases its footprint (corollary 3).

**Gate:** census fully quiet. Visual: indoor galleries appear; outdoor
gallery spacing tightens.

### SPAWN_5 — delete the sweep
**Behavior:** none, if SPAWN_3 was right.
Delete `unregister_footprints_for_patch`. The sequencing is the proof:
the census went quiet with the sweep still in (SPAWN_4); if it stays
quiet without it, the sweep was doing nothing and its deletion is
demonstrated. If it goes noisy, the sweep was hiding something and the
census names exactly what.

### SPAWN_6 — the panel work
**Behavior:** none / bit-identical.
- `EntityFamilyTraits` dead-field cut (ten candidates; CC greps and
  reports before cutting). `has_footprint` is the worst — it declares a
  policy `negotiate_position` does not consult.
- `run_gate` collapse: nine near-identical bodies, each restating five
  constants already in its own TRAITS row. The only genuine per-family
  fact is the active array. Collapse to one law plus a per-family slot
  view.
- `PLACEMENT_ORDER[COUNT]` permutation read by `select_entities_for_patch`;
  identity default; `static_assert` it is a permutation. PopFamily stays
  pinned (F-1). "Who wins contested ground" becomes a panel row.

**Gate:** `run_gate` collapse must be **bit-identical** — the
composition law's multiplication order is the contract.

### SPAWN_7 — wall tenancy
**Behavior:** yes. New mechanism.
A wall is a 1D span; a frame consumes width; a frame hangs only if the
remaining span fits. A small packing allocator per wall, living with
`place_wall_paintings`. Genuinely absent — not a missing case of an
existing mechanism, which is why it is its own stage and not folded into
the registry.

### SPAWN_8 — the density experiment
**Behavior:** visual gate.
`DENSITY_MIN = DENSITY_MAX = 1.0f` pins the spatial-density lattice to a
constant. Behind that pin sits a real machine: `LATTICE_SPACING = 250`,
`SEED_BAND = 160`, `EXPONENT = 0.6`, a `pow`, and a bilinear sample, run
per tile, to produce 1.0.

**Why it felt random.** The lattice multiplies a *probability*, and each
patch is one Bernoulli roll. A 250wu density cell holds 25 patches of
50wu. At a palm base chance of ~0.03, a 1.5x boost moves the expected
count in that cell from ~0.75 palms to ~1.1. Poisson variance swamps the
signal completely. It was not random — it was **mathematically
undetectable** at that contrast and that sample size.

Three levers make it visible: much higher contrast (3–5x, not 1.5x),
larger density cells, or higher base chances. Set `DENSITY_MIN = 0.6`,
`DENSITY_MAX = 1.5` and look. If it does not earn the screen, cut the
lattice whole — four constants and a per-tile sample.

**Separately:** "columns aligned as polygons" is not this lever and never
was. That is a **formation** — one seed generating N positions in a
geometric relationship — where density decides *how many* and formation
decides *in what arrangement*. No tuning of the first produces the
second. Ruling 13 parks it; this note exists so the density lattice is
never again asked to deliver something it structurally cannot.

---

## WHAT THE SWEEP MUST NOT TIDY

These are the model. Point at them, not at them.

- **The composition law collapse** — one stack, per-consumer facts as
  data, and the float multiplication order as the bit-identity contract.
- **The place phase writes no GPU state.** Preserve absolutely.
- **F-1 / F-2** — the PopFamily order static_assert and the boot
  name-check in `validate_spine`.
- **Evictors live with their owners.**
- **The single-line ROSTER gate at select** — a disabled family is never
  selected, therefore never placed, committed, meshed, or drawn.
- **The queue's decoupling of *what exists* from *where it goes*.**
- **`indoor_bounds_clamp` as one law dispatching on a policy table.**
- **Seed domains.** Every roll keeps its property index. Changing one
  changes worlds.

---

## COSTS, MEASURED IN THE RIGHT CURRENCY

Per frame the registry is scanned by `check_position` (once per
placement attempt; ceiling `SPAWN_BUDGET_PER_FRAME(4) x 12` = 48,
realistically under 10), `proximity_affinity_boost` (constexpr-gated to
column + the flora trio, ~16 worst case), and the release path
(`EVICT_BUDGET_PER_FRAME` = 4). Call it ~68 scans of N.

`GroundFootprint` is ~40 bytes. N=512 is 20KB — L1-resident, a
straight-line walk with a couple of float compares per element. These
are order-of-magnitude estimates, not measurements, but the shape does
not get expensive at these sizes. If it ever does, the fix is a spatial
hash keyed by patch cell — premature now.

**The real cost is not computational. Every footprint is a veto.**
Adding portals and honest gallery discs makes the world sparser at fixed
spawn chances. That is the cost you will feel, and it is paid in
`MIN_SEPARATION` retuning, not in milliseconds.

---

## OPEN

- `PierTier` — nine constants, written at every pier site, **read by
  nothing**. Not dead: a write-only field in a 48-byte struct. Census
  item, not a cut. Ruling deferred.
- `entity_density` — confirmed pinned? (`spatial_density` is; the other
  half of `tile_apply_spawn_mult` needs a grep before the composition
  law is called two-factor.)
- Ruling 13 — formations. Parked, last.
