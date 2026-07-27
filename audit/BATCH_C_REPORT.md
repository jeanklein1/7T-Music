# BATCH C — SPAWN_SWEEP v2, the closure batch — report

Cartridge: `the_board` (`incubator_dual`). Executed by CC against the handoff
stamped by Jean (chat, July 2026). Season law in force: LEANNESS IS BANDWIDTH.

**Preflight.** `git rev-parse --is-shallow-repository` reported `true`;
`git fetch --unshallow` was run BEFORE any ancestry reasoning, and the check
now reports `false`. Base: `5a9fafd59206ed7e96940347aa4fe08b7c879b93`,
`git rev-list --count HEAD` = 985 (the pre-unshallow count of 50 was the
shallow clone's artifact, reported here as the law requires both values).
Encoding: LF-only, no BOM, no CR byte introduced. Gate: `glaw1` GREEN at
base before the first edit.

ORDER LAW honored: everything in Part 0 below was gathered and written whole
before the first edit. Citations are symbols, never FILE:LINE.

---

## PART 0 — READ-ONLY CENSUS

### [C0-a] Counter census

Method: `rg -n "<id>" src/ --glob '!src/docs/**'` per identifier; every hit
classified as exactly one of { declaration | ++ | -- | = 0 |
guard-of-own-decrement }, or reported outside the classes.

**The nine expected-dead identifiers — every code hit falls inside the five
classes. None is DIVERGED. All nine are confirmed dead.**

| identifier | declaration | ++ | -- | = 0 | guard-of-own-decrement |
|---|---|---|---|---|---|
| `pyramid_count` | `EntitiesState` (grounded) | `pyramid_write_active` (entity_pipeline) | `evict_pyramid` | `teardown_entities` | — |
| `arch_count` | `EntitiesState` | `arch_write_active` (entity_pipeline) **and** `force_spawn_portal_arch` (grounded, the handoff's force-spawn anchor) | `evict_arch` | `teardown_entities` | — |
| `column_count` | `EntitiesState` | `column_write_active` (entity_pipeline) | `evict_column` | `teardown_entities` | — |
| `antenna_count` | `EntitiesState` | `antenna_write_active` (entity_pipeline) | `evict_antenna` | `teardown_entities` | — |
| `palm_count` | `EntitiesState` | `palm_write_active` (grounded) | `evict_palm` | `teardown_entities` | — |
| `cactus_count` | `EntitiesState` | `cactus_write_active` (grounded) | `evict_cactus` | `teardown_entities` | — |
| `blade_count` | `EntitiesState` | `blade_write_active` (grounded) | `evict_blade` | `teardown_entities` | — |
| `activeSphereCount_` | `SphereState` (spheres) | `sphere_write_active` | `evict_sphere` | `clear_spheres` | `reconcile_sphere_mirror` — single line `if (… > 0) …--;` |
| `activeCubeCount_` | `CubeBehaviorsState` (cube_behaviors) | `cube_write_active` | `evict_cube` | `clear_cubes` | `reconcile_cube_mirror` — single line `if (… > 0) …--;` |

Hits outside the classes, all non-code or same-name-different-symbol, none a
divergence:

- The one comment that names the family: the census banner above
  `census_scan_active` in `cartridge.hpp` ("EntitiesState count fields
  (pyramid_count … blade_count) are write-only …"). It is the *documentation
  of this very cut* — truth-fixed in C1 (the fields stop existing, so the
  sentence about their write-only occurrences dies; the SCAN-NEVER-A-FIELD
  law it states survives, now with nothing to be confused with).
- `arch_count` in `New chat first handoff.txt` (prose handoff at src root,
  not code — the doc-glob simply doesn't cover it).
- `column_count` in `src/external/imgui/imgui_demo.cpp` — imgui's own local,
  foreign scope, untouched.
- `blade_count` as a **different symbol**: `GPUBladeClusterMeshParams.blade_count`
  (state, mirrored in `world.wgsl::BladeClusterMeshParams` and written by
  `blade_write_gpu` from `BladeIdx::BLADE_COUNT`) — the live blades-per-cluster
  GPU mesh param, plus its mirror in `7t_blade_cluster_designer.jsx`. Alive,
  out of scope, and the reason C1's blade cut is scoped strictly to
  `EntitiesState`.

**The expected keeps, verified genuinely read:**

- `RibbonState.active_count` — read by the finite-mode release check in
  `ribbon.hpp` (`finite_mode && rs.active_count > 0 && !MOOD_TABLE[…]
  .has_anchor_ribbon`) — a genuine branch condition deciding teardown, not a
  guard of its own decrement. KEEP.
- `GoLState.zone_count` — read by the zone-compute data-guards and the ROSTER
  residue proof in `cartridge.hpp` (`if (gol_state_.zone_count == 0) return;`
  twice, plus the violation report). KEEP.
- Gallery: **no stored count field exists** — confirmed; the census row is
  `active_count_gallery` = `census_scan_active(gallery_centers)`, derived.
  (`wall_count_t*`, `per_wall_count_lo/hi`, `snapshot_count` are config
  thresholds and an unrelated counter, not an active-population count.)

**Agreement with B4:** this census agrees with the
`audit/SPAWN_0_PART_B_AUDIT.md` B4 table on all twelve rows — seven
`EntitiesState` counts write-only, sphere/cube guard-read only, ribbon and
GoL genuinely read, gallery derived. No finding against B4.

### [C0-b] Density census

- **(i)** `DENSITY_MIN == DENSITY_MAX == 1.0f` — CONFIRMED. Evidence
  (population_themes, the constants block):

  ```
  inline constexpr float DENSITY_MIN = 1.0f;
  inline constexpr float DENSITY_MAX = 1.0f;
  ```

  So `pop.entity_density = DENSITY_MIN + density * (DENSITY_MAX - DENSITY_MIN)`
  is `1.0f + density * 0.0f` — the whole lattice sample (two `cpu_lattice_node_seed`
  draws per axis, `cpu_hash_f`, `std::pow(raw, DENSITY_EXPONENT)`, bilinear
  smoothstep fold) computes a `density` that is then multiplied by zero.
  Identity to a ULP, exactly as the handoff asserts.

- **(ii)** All five `THEMES` rows author `density_mult = 1.0f` — CONFIRMED.
  Each of TRANSITION / MONUMENTAL / COLONNADE / ANTENNA / BARREN carries the
  positional row `1.0f,` with the `// density` legend. The `blended_density`
  fold is therefore a weighted average of five 1.0s = 1.0.

- **(iii)** Complete reader set of `TilePopulation.entity_density`:
  1. write in `generate_tile_population` (the lattice block:
     `pop.entity_density = DENSITY_MIN + …`);
  2. write in `generate_tile_population` (the theme block:
     `pop.entity_density *= blended_density;`);
  3. the multiply in `tile_apply_spawn_mult` (tile_world):
     `adj_mod *= it->second.pop.entity_density;`.

  Nothing else in C++. (`7t_population_designer.jsx` carries its own JS
  mirror `pop.entityDensity` — a tool-side model, not a reader of the C++
  field; noted, untouched.) **Expected set exactly — zero surviving readers
  after C2's cut, so the field itself dies.**

- **(iv)** Complete reader set of `PopulationTheme.density_mult`: the
  `blended_density += theme.density_mult * w;` fold in
  `generate_tile_population`, and nothing else. **Expected set exactly —
  with (ii) holding, `density_mult` dies.**

- **(v)** `PopulationTheme` rows in `THEMES` are **positional aggregates** —
  brace-initialized, legend-by-comment, no designated initializers. The C2
  pin obligation is therefore LIVE: cutting the `density_mult` member and the
  five positional `1.0f` rows lands with F-5-pattern static_asserts pinning
  the neighbors (`tier_wt_cube` → `spike` adjacency and the struct tail) in
  the same commit.

### [C0-c] Theme tool references

`rg "7t_theme_tool"` across the tree:

- the file itself (`src/tools/7t_theme_tool.jsx`);
- docs: `audit/SPAWN_0_PART_B_AUDIT.md` (three mentions, including the
  retire-or-resync recommendation), the old SPAWN campaign handoffs, and
  this batch's own handoff;
- **DIVERGED from the expected census: one live code reference** — the
  launcher row in `src/tools/registry.js` (`load: () => import("./7t_theme_tool.jsx")`).

**Disposition (recorded, not improvised):** the registry row is the loader's
mechanical companion to the file — `git rm` alone would leave a dangling
dynamic import and a dead launcher tile. C3 cuts the registry row in the same
commit and records the divergence in the commit message. The precondition
"C0-c clean" is read as "no consumer that would *survive* the cut": nothing
else in the tree references the tool.

### [C0-d] Agent census triggers

`dump_agent_census` call sites — **exactly the expected three**:

1. `"boot"` — cartridge boot validation (STAYS);
2. `"mood-transition"` — the teardown-completeness assertion (STAYS);
3. `"periodic"` — the wall-clock block in `phase_census_dumps` (DIES in C4).

`AGENT_CENSUS_INTERVAL` readers — **two, and only one dies**:

1. the periodic agent-census gate in `phase_census_dumps`
   (`time_state_.seconds - agent_state_.last_census_dump >= AGENT_CENSUS_INTERVAL`)
   — dies with the block;
2. **the ROSTER gol-residue proof cadence** in the same phase
   (`time_state_.seconds - rosterGolResidueDump_ >= AGENT_CENSUS_INTERVAL`,
   inside `if constexpr (!ROSTER.gol)`) — a live, non-census consumer.

**C0-d therefore decides: `AGENT_CENSUS_INTERVAL` SURVIVES** (its comment is
truth-fixed to name its surviving consumer). **`AgentState.last_census_dump`
is orphaned once the periodic block dies** (its only other occurrence is its
declaration) **and dies with it.**

One site the handoff did not name, reported before the cut: the periodic
block also contains the `[Player]` position stdout emission, inside the same
interval gate. It is part of the periodic spam and dies with the block —
consistent with [G:runtime] "periodic agent spam gone". The entity census
(`spawn_engine_state_.lastCensusDump_` / `CENSUS_DUMP_INTERVAL`, the
instrument BATCH_A_WITNESS and Jean's gates read) is UNTOUCHED.

---

## ANCHOR VERIFICATION (candidate anchors from the handoff)

| anchor | status |
|---|---|
| `uint32_t       activeCubeCount_ = 0;` (CubeBehaviorsState block) | **MATCHED** byte-for-byte |
| `cbs.activeCubeCount_ = 0;` (clear_cubes tail) | **MATCHED** byte-for-byte |
| `es.arch_count++;` (force_spawn_portal_arch) | **MATCHED** byte-for-byte |
| `pop.entity_density = DENSITY_MIN + density * (DENSITY_MAX - DENSITY_MIN);` | **MATCHED** byte-for-byte |
| `adj_mod *= it->second.pop.entity_density;` (tile_apply_spawn_mult) | **MATCHED** byte-for-byte |

---

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| BATCH C: the four censuses, before the first cut | `e3fe593` | GREEN (base) | LF, no BOM, no CR |
| SPAWN_C1: the nine dead counters | `112dfbe` | **GREEN** | LF, no BOM, no CR |
| SPAWN_C2: the density machine | `41727b4` | **GREEN** | LF, no BOM, no CR |
| SPAWN_C3: retire the theme tool | `010e00c` | **GREEN** | LF, no BOM, no CR |
| SPAWN_C4: the agent census gate — periodic trigger only | `76c6395` | **GREEN** | LF, no BOM, no CR |
| SPAWN_C5 **(PREPARED, LAND-GATED — NOT on the trunk)** | `f9a9c76` on branch `claude/batch-c5-prepared` | **GREEN** | LF, no BOM, no CR |

The batch base was `5a9fafd`; C1–C4 are landed in order on the trunk line.
**C5 is authored and gated**: it lives only on `claude/batch-c5-prepared`
(branched from the C4 head), and does not land until J1's paste — the
BATCH_B stop condition is restated in its commit message. If either
"entity_ref OVERFLOW" fires, the branch is discarded and the overflow
finding outranks this batch.

## GATE STATUS

- **[G:glaw1]** — CC, per commit: table above, all GREEN.
- **[G:census-bit]** — Jean's, runbook unchanged: the
  `audit/BATCH_A_WITNESS.md` procedure verbatim — two boots on base
  (`5a9fafd`) prove the witness, then base vs the C4 head (`76c6395`),
  twelve `active` integers. **PREDICTION: identical.** Nothing in C1–C4 may
  move a spawn — the counters were unread, entity_density was 1.0 to a ULP,
  the theme tool never executed in the cartridge, and the census cut is
  print-only. The witness reads the ENTITY census (`trigger=periodic`),
  which C4 left byte-identical in format. A mismatch is reported by the
  witness's own shape table, and the batch stops.
- **[G:visual]** — nothing moves a pixel: no cut touches a render input.
- **[G:runtime]** — periodic agent spam (and its `[Player]` line) gone;
  boot/transition prints remain; entity census byte-identical in format.

## JEAN GATES (the runbook stands as handed off)

- **J1 — SPAWN_5 EMPIRICS**: grep `entity_ref OVERFLOW` across every
  captured session log, then one smallest-room session (`finite_radius = 1`),
  ≥5 min wandering, capture the censuses. Both silent ⇒ one word authorizes
  C5 (merge `claude/batch-c5-prepared`). Either firing ⇒ C5 stays out; paste
  the line.
- **J2 — FLOATER-BRIDGE WITNESS**: re-apply the preserved probe from the
  dossier appendix (verify its hunks against the live tree first — Batch D
  has since rewired the corral/kite paths in `cube_behaviors.hpp`, so the
  probe's surroundings HAVE moved; that diff is a record of its day). One
  run; read the `[FLOATER]` lines; then delete both `DIAG_FLOATER_BRIDGE`
  blocks. Paste the lines either way — a dead bridge is a finding, not a
  failure.
