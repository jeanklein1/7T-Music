# SPAWN_SWEEP — campaign v2

Supersedes v1. Revised against `SPAWN_0_PART_B_AUDIT.md` (CC, this session).
Cartridge: `the_board` (incubator_dual). Base: master @ `3ca9498`.

Changes from v1 are marked **[v2]**. Three of them are corrections to my own
errors; one is a promotion of a finding that outranks the campaign's original
premise.

---

## THE LAW

**A footprint is a claim on the ground, not a claim on space.**

1. **A family registers iff its own extent touches the ground plane.**
2. **A footprint has an owner: (family, slot). The owner releases it.**
3. **Any phase may reject; a rejecting phase releases what earlier phases
   reserved.**

Unchanged. The audit confirmed all three and sharpened corollary 3: place-time
failure is already clean tree-wide, because `register_footprint` is the last
step of every registering path. **The entire leak is commit-side.** That halves
SPAWN_3.

### What the registry is not

`check_position` is read from three sites — `negotiate_position`,
`gallery.hpp:836`, `gol_zones.hpp:502` — all at spawn. It is a
**placement-exclusion registry**. Runtime collision is the occupier query face
on the GPU and never reads it.

---

## **[v2]** THE FINDING THAT RESHAPES THE CAMPAIGN

**The registry leaks permanently, and the leak is on a schedule.**

Every generic committer does `find_patch(self, host_gx, host_gz)` and, on
`nullptr`, frees only the family slot. The footprint is not released — and
because it is keyed to a host patch that does not exist,
`unregister_footprints_for_patch` can only ever match it if that exact grid
cell streams in **and evicts again**.

On a wandering camera: eventually. **On a linear traverse: never.**

A two-hour recording is a linear traverse. At any nonzero abort rate the
registry fills monotonically until `register_footprint` returns the sentinel,
after which spawning silently degrades and *who* gets dropped is decided by
PopFamily order. Ruling 8's raise to 512 buys 4× the runway and fixes nothing.

**This makes SPAWN_3 load-bearing for roadmap priority 2** (visual variety
across a 2hr recording), not hygiene. It also makes SPAWN_1's census the
instrument that measures the schedule: the leak presents as monotone upward
drift in `claimed − active`, every family, without bound.

Three leak paths, all commit-side, none needing a hook on the place path:

| # | path | severity |
|---|---|---|
| 1 | host-patch-missing commit abort (all twelve families) | **permanent** on a traverse |
| 2 | gallery zero-painting abort | bounded — host patch exists |
| 3 | ribbon zero-tips reject | bounded — host patch exists |

---

## RULINGS RECORD

| # | Question | Ruling |
|---|---|---|
| 1 | Floater footprints | Floaters claim no ground → leave the registry; self-separation goes local. **Confirmed clean across all eight positional tables**, not just `MIN_SEPARATION`. |
| 2 | Portal arches, anchor ribbon | Register. **[v2] Rescoped** — see ruling 14. |
| 3 | Commit-stage rejection | Dissolved for gallery by the reservation; corollary 3 covers ribbon. |
| 4 | The three identity factors | `MOOD_SPAWN_MULT` live — keep. `GLOBAL_ENTITY_DENSITY` — idle dial, keep. Density lattice — give it range and look. **[v2] retargeted, see ruling 16.** |
| 5 | `PLACEMENT_ORDER` as data | Yes. |
| 6 | The census | Measure what it is named. **[v2] scope grew — see ruling 15.** |
| 7 | Moving cubes | Superseded by ruling 1. |
| 8 | `MAX_FOOTPRINTS` | Raise to 512, loud line on saturation. **[v2] necessary but not sufficient — the leak is unbounded.** |
| 9 | `unregister_footprints_for_patch` | Delete — last, after the census proves it. **[v2] and only after the census is actually wired.** |
| 10–12 | capacity · gallery reservation · wall tenancy | Stamped. |
| 13 | Formations | **PARKED — last.** |
| **14** | **[v2] Portal registration scope** | **Channel B only.** `PORTAL_DENSITY = 1.00f`, so every DOORWAY-tier arch through dispatch becomes a portal — and those already register via `negotiate_position`. Only `force_spawn_portal_arch` registers nothing. **Release is new for both channels**, because no evictor touches the registry at all. |
| **15** | **[v2] Census wiring** | **Wire before rebuild.** Zero callers today. First gate is "printed at all." |
| **16** | **[v2] Density knob identity** | The dead lattice pins **`entity_density`** — a per-tile *global scalar*. **`spatial_density` is live**, per-family, written from blended theme weights. The v1 spec had these inverted. |
| **17** | **[v2] `PierTier`** | **Cut the enum and the six writes; the field becomes `_pad0`.** 48 − 4 = 44, not 16-aligned — the padding is *required* by the struct-alignment law, so this is compliance, not reserved capability. Stride, WGSL layout and the 48-byte assert all unchanged; zero FXC risk. |
| **18** | **[v2] `7t_theme_tool.jsx`** | 9×9 against a live 12×12, numbers drifted (pyramid self-sep 15 vs 65). **If used: make the C++ tables the source and import them**, so drift is structurally impossible. **If unused: retire.** It cannot remain a second implementation of the placement law with its own numbers while SPAWN_3 makes retuning necessary. |
| **19** | **[v2] Force-spawned portals in the census** | **Count them — they exist.** `active` is what is in the array. The `claimed` side is short by exactly the live force-spawned portal count; that gap is a *prediction*, and SPAWN_3 closes it. |
| **20** | **[v2] `host_gx`/`host_gz` for force-spawned portals** | **SPAWN_3 prerequisite.** `force_spawn_portal_arch` never writes them; the slot retains stale values. Registering Channel B first would key its footprint to an arbitrary patch. **Write them from the portal's world position, then register.** |

---

## **[v2]** THE ORDERING RULE, WITH ITS REASON

v1's handoff said "Part B first, whole" and gave no reason. It was violated,
and the violation cost an adversarial pass chasing ~20 phantom errors that were
pure line drift, plus one drifted table in CC's own report.

**A read-only audit and an edit pass do not overlap** — not because sequence is
tidy, but because every line number in the audit is invalidated the moment an
edit lands, and the resulting divergence is indistinguishable from a real
finding. Both snapshots are internally correct; only the reader is wrong.

Instructions ship with their reasons from here.

---

## THE STAGES

### SPAWN_0 — DONE
Landed as `ac08f05` (anchors 1,2,3,5,6) and `db326d2` (anchor 4). glaw1 GREEN
at both. 28 lines across 7 files: comments plus one dead parameter.

Two divergences handled correctly rather than guessed: anchor-1's *prescription*
was wrong (it kept `GLOBAL_ENTITY_DENSITY`, which lives in `spawn_services.hpp`
— the same ground on which it cut the family enum); anchor-3's comment sat at
the COMPOSITION LAW banner, not above `evaluate_spawn_gate`, and the survivor
was relocated to the function it names.

B9 resolved to **nothing to cut** — the pyramid tombstone was a tombstone for a
tombstone; the C6 cut it forward-referenced had already been executed.

### **[v2]** SPAWN_1 — census RESURRECTION, then rebuild
**Behavior:** none.

**1a. Wire it.** Four-line mirror of `cartridge.hpp:1214-1222` into the empty
stub at `:1225`. The agent census directly above is the working template and has
three live triggers (`"boot"`, `"mood-transition"`, `"periodic"`); the entity
census has the exact mirror-image scaffolding already present and unused.
**Gate 1a: it printed at all.**

**1b. Rebuild it.** `FamilyDispatch` gains `uint32_t (*active_count)(const MachineCtx*)`.
Twelve one-liners. Print `active` and `claimed` side by side, per family. They
must agree; disagreement names the family.

**Counts come from `.active` scans, never the stored fields.** All seven
`EntitiesState` count fields are **write-only** — every occurrence is a
declaration, `++`, `--`, or `= 0`. The scan-vs-stored disagreement is then a
free standing check: if they ever diverge, an evictor is leaking.

Traps, from B4:
- `MAX_RIBBON_INSTANCES` and `MAX_GALLERIES` are `t7::the_board` namespace
  constants, **not** `Dim::` members. `Dim::MAX_GALLERIES` will not compile.
- `Dim::ANTENNA_SLOT_OFFSET` (16) and `Dim::CUBE_SLOT_OFFSET` (8) are **GPU-side
  only**. Both CPU arrays are 0-based. Do not apply them.
- ANTENNA does not share an array with COLUMN. The flora trio does not share one
  either.
- `gol_state_.active_slot_count` and `cpu_pyramids.count` are **high-water
  marks**, not populations. Both are live-read, which makes them tempting.
- GALLERY's `painting_slots[32]` is shared with `indoor_shell` via
  `form_type == WALL_FRAME`. The census wants `gallery_centers`.
- ROSTER-disabled families must read zero on **both** sides — that agreement is
  itself a check.
- The cube corral already derives an active count by scanning
  (`cube_behaviors.hpp:290-292`). The twelve accessors must agree with it.

**1c. Ship the predicted mismatches.** State, before the first print, what
`claimed − active` should read per family and why:

| family | predicted at first print | why |
|---|---|---|
| sph, cube | **0** | they register today; SPAWN_2 drives claimed to 0 |
| arch | **claimed < active**, by the live force-spawned portal count | Channel B registers nothing (ruling 14) |
| ribbon | **claimed > active** if any 0-tip reject occurred | leak path 3 |
| gallery | **claimed > active** if any 0-painting abort occurred | leak path 2 |
| all | **monotone upward drift over time** | leak path 1, unbounded on a traverse |
| ROSTER-disabled | **0 on both sides** | never selected |

The first print is then a *test*, not an observation. Falsification names the
family.

### SPAWN_2 — floaters leave the registry
**Behavior:** yes.

Sphere and cube stop registering. Self-separation (`[Sph][Sph]=20`,
`[Cube][Cube]=15`) moves to a local scan against `activeSpheres_[]` /
`activeCubes_[]`. Values stay where authored; only the reader changes.

**[v2] The trade, made explicit at the moment it is made.** `check_position`'s
radii-sum overlap term is **unconditional** — a zero table entry skips only the
*additive gap*, never the overlap test. So leaving the registry discards
bodily overlap-exclusion against all twelve families, both directions, not just
the self-gap. That is exactly ruling 1's intent ("the ground beneath them is
free") and SPAWN_2's own visual gate already asserts it. Confirmation, not
objection.

**[v2] Clean across all eight positional tables, not one.** `PROXIMITY_RADIUS`,
`_MAX_BOOST`, `_THRESHOLD`, `_GAP_REDUCTION` and the `PROXIMITY_AFFINITY` rows
are all at their documented sentinels for both floaters, and
`proximity_row_active` short-circuits them before the scan.

**Gate:** census shows `sph`/`cube` claimed = 0, active unchanged.

### **[v2]** SPAWN_3 — the footprint gets an owner
**Behavior:** yes. **Rescoped, and promoted in importance** — this is the stage
that stops the permanent leak.

- `GroundFootprint` gains `slot`. `unregister_footprint_for(family, slot)` —
  ABSENT from live code today.
- **Every `register_footprint` call currently discards its index.** All three of
  them. `PositionResult` carries no footprint-index field. That is *why* no
  per-owner release exists: no owner has ever learned its slot. Plumbing the
  index back is the substance of this stage.
- **Release hooks on three commit-side aborts** (leak paths 1–3), and on the
  evictors. v1 said "route release through every evictor" — necessary, **not
  sufficient**.
- **No hook on any place path.** Place-time failure is already clean tree-wide;
  every family already rolls its slot back.
- **Channel B registration only** (ruling 14), **after** `host_gx`/`host_gz` are
  written (ruling 20).
- Anchor ribbon registers at slot 0.
- `MAX_FOOTPRINTS` → 512, loud line on saturation.

**Gate:** the monotone drift stops. Census quiets for every family except
gallery (SPAWN_4's case).

### SPAWN_4 — the gallery becomes honest
**Behavior:** yes.

**[v2] The radius originates at `select`**, not place — `sel.footprint_r` at
`gallery.hpp:813`; place only copies it to `plan.footprint_r`. The reservation
must move the radius computation too, not merely add a field.

**[v2] Containment is the same variable, not a similar formula:**
`indoor_bounds_clamp(c, GALLERY, sel.footprint_r, sel.footprint_r, ...)`.
Gallery is one of only two `FULL` families, and `FULL` **skips outright**
rather than recentring.

**[v2] Quantified.** `PAINTINGS_MAX_BY_ARCHETYPE = {8,10,12,12}`,
`ROW_SPACING = 18` → radii **87 / 105 / 123 / 123** wu against a median true
half-span near 50. Over-reservation ~1.7–2.5×.

| archetype | chance | r | R=1 | R=2 | R=3 | R=4 |
|---|---|---|---|---|---|---|
| 0 mountain | 0.03 | 87 | SKIP | ok | ok | ok |
| 1 varied | 0.06 | 105 | SKIP | degenerate | ok | ok |
| 2 basin | **0.30** | 123 | SKIP | **SKIP** | ok | ok |
| 3 pool | **0.40** | 123 | SKIP | **SKIP** | ok | ok |

Every indoor gallery is skipped at R=1; the two archetypes carrying 70% of the
spawn chance are also skipped at R=2. With R uniform over 1–4, they fail
indoors half the time on room size alone. Shrinking to the realized count
(~50 wu) clears R=2 for every archetype and R=1 for archetypes 0–1.

**[v2] Two traps for the reservation:**
- `GalleryPlacement` is a **union member** of `PlacementEntry`, whose
  constructor memsets only the `generic` arm. A new field is not zeroed and
  needs an explicit write at `gallery.hpp:846-860`. It must be a plain built-in
  (`uint32_t`), not a gallery-owned type.
- `gallery.hpp:955` — `row_start = -(float)(painting_count - 1) * ...` with
  `painting_count` a `uint32_t`. At 0 it wraps to `0xFFFFFFFF`
  (`row_start ≈ -3.87e10`). Inert today because the loop never runs at 0.
  **SPAWN_4 is exactly the refactor that hoists this math.** Guard it.

Ribbon's 0-tips reject releases its footprint (corollary 3).

### SPAWN_5 — delete the sweep
**Behavior:** none, if SPAWN_3 was right.
**[v2] Hard dependency on SPAWN_1a.** Deleting a function on the strength of a
census that never printed is unsound. `unregister_footprints_for_patch` has
exactly one caller — `patch_system.hpp:47`, in `evict_patch`.

### SPAWN_6 — the panel work
**Behavior:** none / bit-identical.
**[v2] 10 of 10 censused traits fields are dead — zero readers each.**
`has_footprint` is authored `true` at all nine sites, so cutting it discards no
intent. `creates_ground`, `piers_per_entity`, `gpu_ground_y` occur **once** in
the tree: their own declarations.

**[v2] Nine `*_TRAITS` objects, not twelve** — RIBBON, GOL and GALLERY have
none. **`FamilyDispatch` has no `traits` pointer**; its seven members are
`try_select`, `try_place`, `try_commit`, `evict_slot`, `prepare_mesh`,
`dispatch_mesh`, `name`. The `run_gate` collapse cannot route through a
dispatch traits pointer — it needs a different seam.

`PLACEMENT_ORDER[COUNT]`, identity default, `static_assert` it is a permutation.

### SPAWN_7 — wall tenancy
Unchanged. A 1D span allocator per wall, living with `place_wall_paintings`.
`find_free_painting_slot` has exactly two call sites, as expected.

### **[v2]** SPAWN_8 — the density experiment, retargeted
The knob is **`entity_density`**, a per-tile global scalar across all twelve
families — not `spatial_density`.

`pop.entity_density = DENSITY_MIN + density * (DENSITY_MAX - DENSITY_MIN)`
with both at 1.0 annihilates the lattice. It is pinned **twice**: all five
themes also author `density_mult = 1.0f`.

**The reframe that matters.** `spatial_density` is a live per-family array
written from blended theme spawn weights, varying per tile and per family. So
*"areas with more palm trees"* — the thing that couldn't be got right — is
**already half-built and running**, through themes rather than through the
lattice. What is dead is the *global* thick/thin dial.

That is arguably better for visibility than v1 assumed: whole regions thin and
thicken together, which clears the Poisson-variance problem that made the
per-family version undetectable. The "why it felt random" analysis survives
intact and describes the `entity_density` lattice.

Composition law in practice today: **mood × spatial_density**. Two factors.

Experiment: `DENSITY_MIN = 0.6`, `DENSITY_MAX = 1.5`, look, rule.

**Formations remain a different thing** (ruling 13, parked): density decides
*how many*, formation decides *in what arrangement*. No tuning of the first
produces the second.

---

## WHAT THE SWEEP MUST NOT TIDY

- The composition law's multiplication **order** — the bit-identity contract.
- The place phase writes no GPU state.
- F-1 / F-2.
- Evictors live with their owners.
- The single-line ROSTER gate at select.
- `indoor_bounds_clamp` as one law on a policy table.
- Seed domains. Every roll keeps its property index.
- **[v2]** `GPUColumnMeshParams.tier` — live, load-bearing, selects the antenna
  profile and gates the indoor ceiling fit. A casual `.tier` grep hits it.
  Two structs, one field name, opposite verdicts.

---

## OPEN

- Ruling 18 — theme tool: import or retire. Needs Jean's use-or-not.
- Ruling 13 — formations. Parked, last.
