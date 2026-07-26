# SPAWN_0 — Part A closeout + Part B audit

Campaign: SPAWN_SWEEP (spec: `src/docs/HANDOFFS/SPAWN CAMPAIGN/spawn_campaign_v1.md`)
Handoff: `src/docs/HANDOFFS/SPAWN CAMPAIGN/cc_handoff_spawn_0.txt`
Cartridge: `the_board` (incubator_dual)

---

## 0. PREFLIGHT — the GIT LAW precondition FIRED

The handoff's git-law paragraph is the one it calls "learned rather than
assumed." It earned that billing again:

```
git rev-parse --is-shallow-repository   →  true      (expected: false)
```

The clone **was shallow**. Per the law, `git fetch --unshallow` was run before
any ancestry reasoning:

```
after unshallow:  is-shallow = false,  rev-list --count HEAD = 951
```

951 against the handoff's expected ~936 — consistent (the campaign's own
commits land on top). No ancestry claim in this report was drawn before that
fetch.

### The gate

`glaw1` is present and runnable in-container: `sh audit/tools/glaw1/run.sh`
(stubs the absent Dawn/GLFW SDK surface, then `g++ -std=c++20 -fsyntax-only`
over the whole real cartridge TU).

| Moment | Result |
|---|---|
| Baseline, before any edit | `G-LAW 1: GREEN` |
| After the prose sweep | `G-LAW 1: GREEN` |
| After the `diag_name` cut | `G-LAW 1: GREEN` |

Encoding: LF-only throughout; no CR byte introduced in any touched file.

---

## 1. PART A — per-anchor disposition

Reporting convention as the handoff requires: MATCHED / DIVERGED / ABSENT.

| Anchor | Target | Status | Action |
|---|---|---|---|
| 1 | phantom tag tables | **MATCHED** (text) / **DIVERGED** (suggested replacement) | Cut, replacement text corrected — see below |
| 2 | empty `Property Index Registry` banner | **MATCHED** | Cut |
| 3 | stale `adjacency_mod` comment | **MATCHED** (text) / **DIVERGED** (position) | Cut + survivor relocated |
| 4 | `diag_name` dead parameter | **MATCHED** | Cut: decl + def + 10 call sites |
| 5 | `<iostream>` attribution | **MATCHED** | Comment replaced, include kept |
| 6 | pyramid tombstones ×2 | **MATCHED** | Both cut |
| 7 | `PierTier` | **MATCHED** | **Report only — no commit** |

Commits: `ac08f05` (anchors 1,2,3,5,6) and `db326d2` (anchor 4), kept separate
for bisection exactly as the land order specifies.

### [anchor-1] — the precondition passed, but the *prescription* was wrong

Precondition clean: `rg "Spawn Configuration Summary" src/` hits only the
sentence being cut, plus docs. The table does not exist.

**The handoff's suggested replacement kept "the global density dial" and cut
the family enum, on the grounds that `PopFamily` lives in `contracts/roster.hpp`
rather than here. But `GLOBAL_ENTITY_DENSITY` lives in
`contracts/spawn_services.hpp:62` — also not here.** Keeping it would have
reproduced the exact defect the anchor exists to remove.

The sentence now names what the banner actually heads (`compose_spawn_chance`,
`evaluate_spawn_gate`, `jittered_position`, `proximity_affinity_boost`).

Worth recording against a future sweep: `Property Index Registry` is **not** a
phantom globally. It is a live banner convention carrying real content at ten
other sites (`grounded.hpp` ×7, `gol_zones.hpp`, `floaters.hpp` ×2). It was
empty only in `spawn_engine.hpp`.

### [anchor-3] — positional divergence, reported rather than guessed

The two-line comment matched verbatim, but **it was not above
`evaluate_spawn_gate`.** It sat at the top of the COMPOSITION LAW banner, two
functions earlier. Deleting only the stale line would have left "Evaluate the
spawn gate: seed + flat probability check." glued to `compose_spawn_chance`,
describing the wrong function.

The stale line was cut; the survivor was relocated to the function it names.
That relocation is the one judgment call in the sweep.

### [anchor-4] — completeness proven by the gate, not by grep

Exactly ten call sites, matching both the handoff's estimate and the file
header's own claim ("one implementation, ten callers"):

| file:line | family | active array |
|---|---|---|
| `machine/entity_pipeline.hpp:778` | `"pyr"` | `entities_state_.pyramids` |
| `machine/entity_pipeline.hpp:935` | `"arch"` | `entities_state_.arches` |
| `machine/entity_pipeline.hpp:415` | `"col"` | `entities_state_.columns` |
| `machine/entity_pipeline.hpp:557` | `"ant"` | `entities_state_.antennas` |
| `bodies/grounded.hpp:1166` | `"palm"` | `entities_state_.palms` |
| `bodies/grounded.hpp:1394` | `"cact"` | `entities_state_.cacti` |
| `bodies/grounded.hpp:950` | `"blad"` | `entities_state_.blades` |
| `bodies/spheres.hpp:160` | `"sph"` | `sphere_state_.activeSpheres_` |
| `bodies/cube_behaviors.hpp:537` | `"cube"` | `cube_behaviors_state_.activeCubes_` |
| `bodies/ribbon.hpp:1106` | `"ribn"` | `ribbon_state_.active` |

The nine generic families plus ribbon. GoL and gallery run their own spawn
funnels and never touch this template. The template is never taken by address
and never used as a function pointer, so its arity is fixed by these ten sites
alone.

`run_spawn_preamble` is a **template**: a missed call site would pass eleven
arguments to a ten-parameter template and fail instantiation at end-of-TU.
`glaw1` compiles the whole real TU, so GREEN is a proof of completeness that
grep alone could not give.

### [anchor-7] — PierTier: REPORT ONLY, ruling deferred to Jean

**(a) Zero readers of `GPUPierInstance.tier`, C++ and WGSL both — CONFIRMED.**

- Struct: `realization/state.hpp:861`; the field is `uint32_t tier;` at `:868`,
  commented "metadata for future use".
- `namespace PierTier` at `state.hpp:877` — nine constants, `ARCH_DOORWAY = 1`
  … `COL_ANTENNA_COLOSSAL = 9`.
- **Six writers, zero readers.** Writers: `bodies/grounded.hpp:678`, `:687`
  (force-spawn portal piers); `machine/entity_pipeline.hpp:523`, `:665`
  (column/antenna piers), `:1088`, `:1099` (arch piers).
- WGSL: the mirror struct `PierInstance` (`world.wgsl:2556`) declares
  `tier: u32, // pier tier (metadata — not read by evaluation)`. The binding
  `pier_instances` (`:2569`) has exactly one consumer chain —
  `structure_height_at` (`:2617`) → `evaluate_pier` (`:2573`) — and
  `evaluate_pier` reads `is_active`, `origin`, `rotation`, `half_size`,
  `edge_blend`, `height_near`, `height_far`. **It never touches `inst.tier`.**

**(b) The 48-byte assert — CONFIRMED.** `state.hpp:873`:
`static_assert(sizeof(GPUPierInstance) == 48, "GPUPierInstance must be 48 bytes");`

**A trap for whoever acts on this ruling.** A casual `.tier` grep over the
shader *appears* to refute the finding — `world.wgsl:9896`, `:9935`, `:10064`
all read `p.tier` against `TIER_ANTENNA_FIRST`. Those are a **different
struct**: `p = cmg_params[slot]`, i.e. `GPUColumnMeshParams`, on a **different
numbering** (column-local, `TIER_ANTENNA_FIRST = 3u`, vs `PierTier` where
`COL_ANTENNA = 7`). That field **is** live and load-bearing — it selects the
antenna profile and gates the indoor ceiling fit, and it is written by
`build_column_mesh_params_from` (`spawn_engine.hpp:384`, `p.tier = c.tier_idx`).

Two structs, one field name, opposite verdicts. Cutting the wrong one changes
every column and antenna in the world.

---

## 2. PART B — the audit

Nine read-only queries, each independently derived and the five load-bearing
ones adversarially re-derived by a second pass.

**Method note, and a mistake worth recording.** The handoff's land order says
"Part B first, whole. Report before touching anything." I ran the Part A cuts
while the audit was still in flight — every anchor was independently verified
first, so no cut was unsafe, but the *ordering* instruction existed for a
reason I only saw afterwards. `ac08f05` removed six lines from `grounded.hpp`
and `db326d2` changed `spawn_services.hpp` and `spawn_engine.hpp` **mid-audit**,
so queries that read those files early cite line numbers exactly six (and in
places two) higher than the current tree. The adversarial pass duly flagged
~20 "errors" that are nothing but this drift — both snapshots were internally
correct.

Every line number in *this* report has been re-derived against the tree as it
stands at `db326d2`. The lesson stands for the next stage: a read-only audit
and an edit pass should not overlap, even when the edits are provably safe.

### The three findings that change the campaign

Everything else is detail. These three are the ones that alter what the next
stages must do.

---

#### FINDING 1 — the census has **zero callers**. SPAWN_1 is a resurrection, not a rebuild.

`dump_entity_census` is declared (`contracts/spawn_services.hpp:227`), defined
(`machine/spawn_engine.hpp:550`), compiled into every build — and **never
invoked anywhere in the tree**.

```
rg "dump_entity_census" src/ --glob '!src/docs/**'
  → contracts/spawn_services.hpp:227   (declaration)
  → machine/spawn_engine.hpp:550       (definition)
  ... and nothing else.
```

Its whole cadence apparatus is equally dormant:

| symbol | site | status |
|---|---|---|
| `CENSUS_DUMP_INTERVAL = 30.0f` | `spawn_engine.hpp:65` | **zero reads** |
| `SpawnEngineState::lastCensusDump_` | `spawn_engine.hpp:154` | **zero reads, zero writes** (only its default member initializer) |

No `#ifdef` gates it — `spawn_engine.hpp` contains no preprocessor directives
at all. It is not reachable from a console (there is no command console in this
codebase — `src/console/console.hpp` is the platform shell: window, GPU device,
surface, input, frame timing) and not from any key binding
(`direction/input.hpp:223-277` is a complete camera/pawn/mood/orb/cube/possession
fan with no diagnostic verb).

**Why this matters more than any other finding.** SPAWN_1 is built *first*
precisely so every later stage has a gate. Its stated gate is "the census
prints." As written — add an `active_count` row to `FamilyDispatch`, twelve
one-liners, rewrite the print — **the census still would not print**, because
nothing calls it. SPAWN_2 through SPAWN_5 would then each "pass" a gate that
never ran, and SPAWN_5 in particular is designed to *delete a function* on the
strength of the census staying quiet. A silent census and a quiet census are
indistinguishable.

**The fix is small and the template is already in the file.** The call site was
scaffolded and left empty — `cartridge.hpp:1225`:

```cpp
                    agent_state_.last_census_dump = time_state_.seconds;
                }

                // Periodic entity census dump          ← the stub, and then nothing
            }
```

Directly above it, the **agent** census is fully wired and is the working
pattern to mirror:

```cpp
if (time_state_.seconds - agent_state_.last_census_dump >= AGENT_CENSUS_INTERVAL) {
    dump_agent_census(agent_state_, &agents_deps_, "periodic");
    ...
    agent_state_.last_census_dump = time_state_.seconds;
}
```

`dump_agent_census` has three live triggers — `"boot"` (`cartridge.hpp:552`),
`"mood-transition"` (`:948`), `"periodic"` (`:1215`). The entity census has the
exact mirror-image scaffolding already present and unused: its own interval
constant, its own `lastCensusDump_`, its own `trigger` parameter. SPAWN_1's
wiring is a four-line mirror of `cartridge.hpp:1214-1222` dropped into the stub
at `:1225`.

**Recommendation: wire the census in SPAWN_1 before touching its contents, and
make "the census printed at all" the first gate.**

---

#### FINDING 2 — `DENSITY_MIN`/`DENSITY_MAX` pin **`entity_density`**, not `spatial_density`. The spec has the two fields swapped.

The campaign states (SPAWN_8): "`DENSITY_MIN = DENSITY_MAX = 1.0f` pins the
spatial-density lattice to a constant," and asks in OPEN: "`entity_density` —
confirmed pinned? (`spatial_density` is …)".

**Both halves are inverted.** `surface/population_themes.hpp:297`:

```cpp
pop.entity_density = DENSITY_MIN + density * (DENSITY_MAX - DENSITY_MIN);
```

With `DENSITY_MIN == DENSITY_MAX == 1.0f` (`population_themes.hpp:32-33`) this
is exactly `1.0`. The lattice machinery the spec correctly identifies —
`DENSITY_LATTICE_SPACING = 250.0f`, `DENSITY_SEED_BAND = 160u`,
`DENSITY_EXPONENT = 0.6f`, the `std::pow`, the bilinear sample
(`population_themes.hpp:279-298`) — computes `density` and then multiplies it
by zero. The spec identified the right constants and the right dead machine;
it attached them to the wrong field name.

`spatial_density` is a different thing entirely: a **per-family array**
(`TilePopulation::spatial_density[PopFamily::COUNT]`,
`population_themes.hpp:266`), written from the blended theme spawn weights at
`:328` (`pop.spatial_density[f] = blended_spawn[f];`). It has no lattice of its
own and was never pinned by these constants. It genuinely varies, per tile and
per family.

**`entity_density` is pinned twice over, independently.** Beyond the lattice
annihilation, `population_themes.hpp:332` applies `pop.entity_density *=
blended_density` — and all five themes author `density_mult = 1.0f`
(TRANSITION / MONUMENTAL / COLONNADE / ANTENNA / BARREN). So `entity_density`
arrives at `tile_apply_spawn_mult` as 1.0 to within a ULP.

Consequently `tile_apply_spawn_mult` (`surface/tile_world.hpp:492-493`) —

```cpp
adj_mod *= it->second.pop.entity_density;        // ← effectively 1.0
adj_mod *= it->second.pop.spatial_density[family];  // ← the live factor
```

— is a **one-factor face today**, not two.

**This does not void SPAWN_8; it aims it.** The prescribed experiment ("set
`DENSITY_MIN = 0.6`, `DENSITY_MAX = 1.5` and look") is still the correct lever,
and it still works, because the second pin is a multiply by 1.0 rather than a
zero. Two adjustments to the stage's framing:

1. The knob moves **`entity_density`**, a *global scalar* affecting all twelve
   families in a tile at once — not a per-family value. That is arguably better
   for visibility than the spec assumed: whole regions thin and thicken
   together, rather than one family's rate wobbling inside Poisson noise.
2. The spec's "why it felt random" analysis (250 wu cells, 25 patches, Bernoulli
   variance swamping a 1.5× contrast) is sound and survives intact — it just
   describes the `entity_density` lattice.

Also answered, since the OPEN section asks: **the composition law is a
two-factor stack in practice** — mood × spatial_density — with
`GLOBAL_ENTITY_DENSITY` (1.0), `entity_density` (1.0) and the density lattice
all currently inert.

---

#### FINDING 3 — SPAWN_3's premise sentence is false in both halves, today.

The spec (SPAWN_3) says: "`force_spawn_portal_arch` registers; `evict_arch`
already releases."

**Neither is true against the code as it stands.** It reads as a plan already
written in the past tense.

- `force_spawn_portal_arch` (`bodies/grounded.hpp:626-742`) calls **no**
  `check_position` and **no** `register_footprint`. The complete set of
  `register_footprint` call sites is three:
  `machine/spawn_engine.hpp:321` (inside `negotiate_position`),
  `bodies/gol_zones.hpp:508`, `bodies/gallery.hpp:842`. The portal spawner is
  none of them.
- `evict_arch` (`bodies/grounded.hpp:764-774`) clears two piers, sets
  `arches[slot].active = false`, decrements `arch_count`, marks portals dirty
  and zeroes the GPU mesh params. It touches the footprint registry **not at
  all** — no family evictor does.

So portal arches are invisible to the registry in **both** directions: they
claim no ground, and nothing collide-tests against them. This is exactly the
gap SPAWN_3 exists to close — the work is real, but it is *new* work, not the
wiring-up of something already half-present. Ruling 2 ("portal arches register")
stands as a decision, not an observation.

A consequence for SPAWN_1's gate: because the current census counts
**footprints**, and portals hold none, the existing census undercounts arches
by exactly the number of live portals. An array-based census will not have that
gap — so the arch row is expected to *change* at SPAWN_1, and that change is
correct, not a leak.

---

### B1 — `EntityFamilyTraits` dead-field census

**10 of 10 censused fields are DEAD — zero readers each.**

| field | readers | note |
|---|---|---|
| `short_name` | 0 | duplicate of the live table in `family_short_name()` (`spawn_engine.hpp:545`) |
| `max_instances` | 0 | each `run_gate` passes the raw `Dim::` constant |
| `grounded` | 0 | |
| `creates_ground` | 0 | identifier occurs **once** in the tree: its own declaration |
| `piers_per_entity` | 0 | occurs once |
| `has_footprint` | 0 | occurs once — see below |
| `spawn_roll_prop` | 0 | shares a spelling with a `run_spawn_preamble` parameter |
| `spawn_chance` | 0 | ditto; families pass `XxxConfig::SPAWN_CHANCE` directly |
| `mood_multiplier` | 0 | live path is `mood_mult_for(PopFamily::X)` at each `run_gate` |
| `gpu_ground_y` | 0 | occurs once |

Two structural facts the stage will need:

- **There are nine `*_TRAITS` objects, not twelve.** RIBBON, GOL and GALLERY
  have no `EntityFamilyTraits` at all. A field cut touches only the nine
  generic families.
- **`FamilyDispatch` has no `traits` pointer.** Its seven members are
  `try_select`, `try_place`, `try_commit`, `evict_slot`, `prepare_mesh`,
  `dispatch_mesh`, `name` (struct at `contracts/entity_types.hpp:342`; the
  table closes at `cartridge.hpp:1784`). The access form
  `FAMILY_DISPATCH[f].traits-><field>` does not exist.

**`has_footprint` — the flagged claim is CONFIRMED, and stronger than stated.**
The identifier appears exactly once in live code: its declaration at
`contracts/entity_types.hpp:133`. `generic_place`
(`machine/entity_pipeline.hpp:177-200`) forwards exactly five traits values —
`pos_x_prop`, `pos_z_prop`, `position_jitter`, `rotation_prop`, `family_id` —
and `has_footprint` is not among them; `negotiate_position` takes no
footprint-policy boolean in its signature at all, and both its footprint steps
are unconditional. **All nine sites author it `true`** (e.g. `BLADE_TRAITS`,
`grounded.hpp:930`, the one site whose trailing comment names the field) — it
has never distinguished anything, so cutting it discards no authored intent.

### B3 — `run_spawn_preamble` call sites

Ten, enumerated in §1 [anchor-4] above. `diag_name` had zero body references;
the cut is landed and gate-green.

### B4 — per-family active array + count field (all twelve)

| # | family | active array | bound | count field | predicate |
|---|---|---|---|---|---|
| 0 | PYRAMID | `c->entities_state_.pyramids` | `Dim::MAX_PYRAMID_INSTANCES` = 8 | `pyramid_count` — **write-only** | `.pyramids[i].active` |
| 1 | ARCH | `c->entities_state_.arches` | `Dim::MAX_ARCH_INSTANCES` = 16 | `arch_count` — **write-only** | `.arches[i].active` |
| 2 | COLUMN | `c->entities_state_.columns` | `Dim::MAX_COLUMN_ONLY` = 16 | `column_count` — **write-only** | `.columns[i].active` |
| 3 | ANTENNA | `c->entities_state_.antennas` | `Dim::MAX_ANTENNA_ONLY` = 16 | `antenna_count` — **write-only** | `.antennas[i].active` |
| 4 | PALM | `c->entities_state_.palms` | `Dim::MAX_PALM_INSTANCES` = 24 | `palm_count` — **write-only** | `.palms[i].active` |
| 5 | CACTUS | `c->entities_state_.cacti` | `Dim::MAX_CACTUS_INSTANCES` = 20 | `cactus_count` — **write-only** | `.cacti[i].active` |
| 6 | BLADE | `c->entities_state_.blades` | `Dim::MAX_BLADE_INSTANCES` = 32 | `blade_count` — **write-only** | `.blades[i].active` |
| 7 | SPHERE | `c->sphere_state_.activeSpheres_` | `Dim::MAX_SPHERE_INSTANCES` = 8 | `activeSphereCount_` — guard-read only | `.activeSpheres_[i].active` |
| 8 | RIBBON | `c->ribbon_state_.active` | `MAX_RIBBON_INSTANCES` = **1** | `active_count` — **genuinely read** | `.active[i].active` |
| 9 | CUBE | `c->cube_behaviors_state_.activeCubes_` | `Dim::MAX_CUBE_INSTANCES` = 256 | `activeCubeCount_` — guard-read only | `.activeCubes_[i].active` |
| 10 | GOL | `c->gol_state_.zones` | `Dim::MAX_GOL_ZONES` = 8 | `zone_count` — **genuinely read** | `.zones[i].active` |
| 11 | GALLERY | `c->gallery_state_.gallery_centers` | `MAX_GALLERIES` = 48 | **DERIVED — none exists** | `.gallery_centers[i].active` |

**All seven `EntitiesState` count fields are write-only.** Every occurrence of
each is a declaration, a `++`, a `--`, or a `= 0` — verified individually. A
census that trusts them is trusting a number no reader has ever validated;
scanning `.active` is the only ground truth. (This is also a standing
consistency check the campaign gets for free: if the scan and the stored count
ever disagree, one of the evictors is leaking.)

Traps for the author of SPAWN_1's twelve one-liners:

- `MAX_RIBBON_INSTANCES` (`bodies/ribbon.hpp:346`) and `MAX_GALLERIES`
  (`bodies/gallery.hpp:420`) are namespace-scope `t7::the_board` constants,
  **not** `Dim::` members. `Dim::MAX_GALLERIES` will not compile.
- `Dim::ANTENNA_SLOT_OFFSET` (= 16) and `Dim::CUBE_SLOT_OFFSET` (= 8) are
  **GPU-side offsets only**. Both CPU arrays are 0-based. A census must not
  apply them.
- ANTENNA does **not** share the CPU array with COLUMN — two distinct arrays of
  the same type `ActiveColumn`. The flora trio does **not** share an array
  either: three arrays, three element types, no kind tag needed.
- `gol_state_.active_slot_count` and `entities_state_.cpu_pyramids.count` are
  **high-water marks** ("highest active slot + 1"), not populations. Both are
  read by live code, which makes them tempting; either would over-count
  whenever slots are fragmented.
- GALLERY has a second slot store, `painting_slots[32]`, which *does* carry a
  read count (`active_painting_count`, drawn at `render_passes.hpp:422`) — but
  it is **shared with the `indoor_shell` feature** via `form_type ==
  FormType::WALL_FRAME`, so counting it would mix outdoor paintings with indoor
  wall frames. The census wants `gallery_centers`.

### B5 — MIN_SEPARATION sphere/cube columns: **self-only, confirmed both ways**

Column 7 (SPHERE) read down all twelve rows: a single non-zero, the diagonal
`[SPHERE][SPHERE] = 20.0f`. Column 9 (CUBE): a single non-zero, the diagonal
`[CUBE][CUBE] = 15.0f`. Rows 7 and 9 read across: one non-zero each, both
diagonal. `[SPHERE][CUBE]` and `[CUBE][SPHERE]` are both `0.0f` — the two
floaters do not even exclude each other.

Indexing was cross-checked three ways: the header-comment token order
(`spawn_services.hpp:85`), the `PopFamily` enum (`roster.hpp:57-69`), and a
diagonal test — all twelve self-values land on `[i][i]`, which an off-by-one
would have thrown off. `check_position` is the sole reader, as the table's own
header claims.

**But the table is only half the exclusion, and the campaign should know which
half it is discarding.** In `check_position` (`spawn_engine.hpp:508`) the
radii-sum term is **unconditional**:

```cpp
float effective_min = placing_radius + footprints_[i].radius;   // always
if (footprints_[i].family < PopFamily::COUNT) {
    float min_gap = MIN_SEPARATION[placing_family][...];         // additive, may be 0
    ...
}
```

A zero table entry only skips the *additive gap*; it never skips the overlap
test. So removing sphere/cube from the registry (SPAWN_2's subtractive fix)
discards not just the self-gap but the unconditional radii-sum exclusion
against all twelve families, both directions.

**That is precisely what ruling 1 intends** — "floaters claim no ground …
the ground beneath them is free," and SPAWN_2's own visual gate says "ground
under floaters now spawnable." So this is a confirmation, not an objection: the
thing being deliberately given up is bodily overlap-exclusion with a hovering
object, and the self-separation that must be preserved (20 wu sphere↔sphere,
15 wu cube↔cube) is exactly what the local scan replaces. No further ruling
needed — flagged only so the trade is explicit at the moment it is made.

**A drifted second model of the placement law.** `check_position` is the sole
*reader* of `MIN_SEPARATION`, but it is not the only *implementation* of the
law. `src/tools/7t_theme_tool.jsx:265-276` carries a line-for-line structural
clone of the separation + overlap pass, affinity gap-reduction included:

```js
let minGap = P.sep[fam]?.[fp.fam] ?? 0;
if (minGap > 0) {
    ...
    if (aff > 0) minGap *= (1 - aff * (P.prox.gapReduction[fam] ?? 0));
    effMin += minGap;
}
```

It is **not** a copy of the C++ values — it carries its own matrix
(`7t_theme_tool.jsx:103-111`) — but that matrix has **drifted badly**: it is
**9×9** (`const NF = 9`, `:43`) against the live 12×12, and the numbers no
longer agree. Row 0 is `[15, 10, 5, 5, 5, 5, 0, 0, 0]` where
`MIN_SEPARATION`'s placing-Pyramid row is `{65, 60, 5, 55, 5, 5, …}` — pyramid
self-separation 15 vs 65, pyramid↔arch 10 vs 60.

This does not affect the sole-reader verdict (the tool reads nothing from the
C++ table). It matters because the campaign states the real cost of SPAWN_3 is
"paid in `MIN_SEPARATION` retuning" — and the tool one would naturally reach
for to do that retuning is modelling a nine-family world with different
numbers. Worth a ruling of its own: resync it, or retire it.

### B6 — footprint call-site census

**Every `register_footprint` call discards the returned index. All three of
them.** `PositionResult` carries no footprint-index field, and no persistent
index exists anywhere in the tree. The campaign's expectation holds, and the
reason there is no per-owner release is structural: no owner ever learns its
slot.

| verb | sites |
|---|---|
| `register_footprint` | `spawn_engine.hpp:321` (`negotiate_position`), `gallery.hpp:842`, `gol_zones.hpp:508` |
| `check_position` | `spawn_engine.hpp:315`, `gallery.hpp:836`, `gol_zones.hpp:502` |
| `unregister_footprints_for_patch` | **exactly one**: `surface/patch_system.hpp:47`, in `evict_patch` |

`unregister_footprints_for_patch`'s trigger is **patch eviction only** — the
chain is `evict_patch` ← `stream_patches` (`patch_system.hpp:672`, the
CONTINUOUS PATCH EVICTION block, budgeted by `EVICT_BUDGET_PER_FRAME`) ←
`phase_stream_patches`, driven per frame from the `RENDER_SPINE`. It is **not**
mood change and **not** teardown; those go through `reset_surface`
(`patch_system.hpp:136-137`, a bulk wipe) at boot (`cartridge.hpp:511`) and at
transition TEARDOWN (`cartridge.hpp:901`).

**All twelve families register.** Nine generic + ribbon reach
`register_footprint` through `negotiate_position`; GoL and gallery bypass it
and call `register_footprint` directly. `unregister_footprint_for(family, slot)`
exists only in the spec — ABSENT from live code.

**Two leak paths the campaign has not accounted for.** Both arise because the
footprint is registered at *place* but the owner can still be abandoned at
*commit*:

1. **Host-patch-missing commit abort.** Every generic committer does
   `find_patch(self, pe.generic.host_gx, pe.generic.host_gz)` and on `nullptr`
   frees only the family slot (e.g. `entity_pipeline.hpp:551`, and the
   equivalent in each body; bespoke: `gol_zones.hpp:691`, `gallery.hpp:1743`).
   The footprint is not released — and because it is keyed to a host patch that
   does not exist, `unregister_footprints_for_patch` can **never** match it. It
   survives until `reset_surface`. This is a permanent leak, and it is the one
   most likely to be what SPAWN_1's census surfaces first.
2. **Gallery zero-painting abort** (see B7). Bounded — the host patch exists, so
   eviction eventually reclaims it.

**This matters for SPAWN_3.** A per-owner release keyed on `(family, slot)`
needs hooks on these abort paths, not only on the evictors. The spec's
"Release routed through every evictor" is necessary but not sufficient.

### B7 — gallery staging, and why indoor galleries are absent

**The footprint formula is verbatim as the campaign quotes it**
(`bodies/gallery.hpp:776-778`):

```cpp
float footprint_r = (float)GalleryConfig::PAINTINGS_MAX_BY_ARCHETYPE[archetype]
    * 0.5f * GalleryConfig::ROW_SPACING + 15.0f;
```

One correction: it is evaluated in **select**, not place (`sel.footprint_r`,
`gallery.hpp:813`). `place_gallery_from_selection` only consumes it and copies
it to `plan.footprint_r`. SPAWN_4's "resolve the count at place" therefore has
to move the radius computation as well, not just add a reservation.

`PAINTINGS_MAX_BY_ARCHETYPE = { 8, 10, 12, 12 }`, `ROW_SPACING = 18.0f` →
radii **87 / 105 / 123 / 123** wu. Against `PAINTINGS_MEAN = 5.0f`, a median
gallery's true half-span is roughly 50 wu. **Over-reservation is ~1.7×–2.5× at
the median.**

**The containment hypothesis is confirmed, and stronger than the spec states.**
The campaign supposes containment is sized from the archetype maximum by a
similar formula. It is not similar — it is the *same variable*
(`gallery.hpp:833-834`):

```cpp
if (!indoor_bounds_clamp(c, PopFamily::GALLERY,
    sel.footprint_r, sel.footprint_r, cx, cz))   // footprint_r AND containment_r
    return false;
```

Gallery is one of only two `FULL` families (the other is ribbon), and `FULL` is
the arm that **skips outright** rather than recentring — a `MARGIN` family that
overflows survives at the room centre; a `FULL` family is dropped with the
`[DIAG:INDOOR-SKIP]` line.

The arithmetic, with `INDOOR_ENTITY_WALL_MARGIN = 20`, `PATCH_EXTENT = 50`, and
`finite_radius ∈ [1,4]` (room width `(2R+1)*50`):

| archetype | spawn chance | r | R=1 | R=2 | R=3 | R=4 |
|---|---|---|---|---|---|---|
| 0 mountain | 0.03 | 87 | **SKIP** | ok | ok | ok |
| 1 varied | 0.06 | 105 | **SKIP** | degenerate (lo = hi) | ok | ok |
| 2 basin | **0.30** | 123 | **SKIP** | **SKIP** | ok | ok |
| 3 pool | **0.40** | 123 | **SKIP** | **SKIP** | ok | ok |

**Every indoor gallery is skipped at `finite_radius == 1`, and the two
archetypes carrying 70% of the spawn chance between them are also skipped at
`finite_radius == 2`.** With R uniform over 1–4, basin and pool galleries fail
indoors half the time on room size alone. That is the mechanism behind "indoor
galleries feel absent," quantified — and shrinking the radius to the realized
`painting_count` (~50 wu) clears R=2 for every archetype and R=1 for
archetypes 0–1.

`find_free_painting_slot` has **exactly two call sites** — `commit_gallery`
(`gallery.hpp:961`) and `place_wall_paintings` (`gallery.hpp:1550`) — and no
others, as the handoff expected.

`GalleryPlacement` (`contracts/entity_types.hpp:252-264`) notes for the
reservation field: it is a **union member** of `PlacementEntry`, whose
constructor memsets only the `generic` arm — a new field is *not* zeroed for
the gallery arm and needs an explicit write at `gallery.hpp:846-860`. The
contract home carries only plain built-ins by design, so the reservation must
be e.g. `uint32_t`, not a gallery-owned type.

**Incidental defect, inert today, live trap for SPAWN_4.**
`gallery.hpp:955`:

```cpp
float row_start = -(float)(painting_count - 1) * 0.5f * GalleryConfig::ROW_SPACING;
```

`painting_count` is `uint32_t`. The cap at `:948` can drive it to 0 (a
`SNAPSHOT_ONLY` gallery whose mono-tier filter empties `candidate_count` after
the `have_snapshots` guard was evaluated on the pre-filter count), and
`0u - 1` wraps to `0xFFFFFFFF`, giving `row_start ≈ -3.87e10`. Harmless now
because the loop never executes at `painting_count == 0` and `row_start` is
read only inside it. **SPAWN_4 is exactly the refactor that would hoist this
layout math** — guard it when the count becomes a reservation.

### B8 — census wiring

Covered in FINDING 1. Summary: zero callers, zero interval reads, zero cadence
reads, no console, no key binding, no `#ifdef`. The stub is at
`cartridge.hpp:1225`; the working template is `cartridge.hpp:1214-1222`.

### B9 — pyramid dead-store status: **nothing to cut**

Both `pyramid_mesh_gen_pending` and `set_pyramid_index_count` are **ABSENT**
from live code. Verified at the base commit `c917869` as well as at HEAD: the
only occurrence of either identifier in the whole tree was **inside the
tombstone comment itself** (`grounded.hpp:627`). The C6 cut the tombstone
forward-referenced had already been executed.

So the tombstone was a marker for code that no longer existed — a tombstone for
a tombstone. Cutting it under [anchor-6] removed the last trace, and the
"SEPARATE FINDING, do not act" item resolves to: **no follow-up commit is
needed.**

Corroborating structure: every other mesh-gen family still carries its flag in
`EntitiesState` (`arch_mesh_gen_pending`, `column_mesh_gen_pending`,
`palm_`, `cactus_`, `blade_`); the pyramid block (`grounded.hpp:495-498`) is
three members with no flag, and the preparer declaration block lists exactly
five preparers with no pyramid.

The FAMILY_DISPATCH pyramid row still carries the fact, verified before the cut
— `cartridge.hpp:1748`: `// mesh hook → none-fork: pyramid mesh dead-by-design;
placement feeds the heightfield`, with fuller prose at `cartridge.hpp:319-322`.

---

## 3. WHAT THIS MEANS FOR THE REMAINING STAGES

| Stage | Status after this audit |
|---|---|
| SPAWN_1 | **Scope grew.** Wire the census first (4-line mirror at `cartridge.hpp:1225`), then rebuild it. Counts must come from `.active` scans, never the stored fields. Two bounds are not in `Dim::`. |
| SPAWN_2 | **Unblocked, clean.** Self-only confirmed both directions. The trade being made (losing the unconditional radii-sum overlap test) is exactly ruling 1's intent. |
| SPAWN_3 | **Bigger than written.** Portal registration and arch release are both *new*. Per-owner release also needs hooks on the two commit-abort paths, not just the evictors. |
| SPAWN_4 | **Confirmed and quantified.** Radius originates at select, not place. Containment is the same variable, not a similar formula. Guard the `row_start` underflow. |
| SPAWN_5 | **Depends entirely on SPAWN_1 being wired.** Deleting the sweep on the strength of a census that never printed would be unsound. |
| SPAWN_6 | **Ready.** 10/10 traits fields dead; `has_footprint` uniformly `true`, so cutting it discards no intent. Nine traits objects, not twelve; no `traits` pointer on `FamilyDispatch`. |
| SPAWN_8 | **Retargeted.** The knob is `entity_density`, a global scalar — not `spatial_density`. The lever works; the name in the spec is wrong. |

---

## 4. OPEN — for Jean

1. **`PierTier`** — ruling still deferred, as instructed. Zero readers
   confirmed in C++ and WGSL; 48-byte assert confirmed. Note the
   `GPUColumnMeshParams.tier` collision before anyone acts.
2. **Portals in the census** — an array-based census will see force-spawned
   portal arches (same array, `is_portal` discriminant). Do they count as ARCH
   population? The arch row will move at SPAWN_1 either way; the question is
   which number is "right."
3. **Ruling 13 (formations)** — untouched, still parked.
4. **`force_spawn_portal_arch` never writes `aa.host_gx` / `aa.host_gz`.** They
   retain whatever the slot last held. Any census or eviction logic keying on an
   arch's host patch reads a meaningless value for portals. Not acted on;
   flagged because SPAWN_3 will make portals registry-visible and the host key
   is what the registry is keyed by.

---

## 5. BRANCH NOTE

The handoff's LAND ORDER step 6 calls for merging to `master`, pushing, and
deleting `claude/spawn-0-audit` the same day. **That was not done**, for two
reasons, and the difference is deliberate:

- This session is pinned to the branch `claude/spawn-campaign-handoffs-wsblrd`,
  not `claude/spawn-0-audit`.
- Its operating instructions forbid pushing to another branch — `master`
  included — without explicit permission.

Both SPAWN_0 commits (`ac08f05`, `db326d2`) are on
`claude/spawn-campaign-handoffs-wsblrd` and pushed. The merge to master is
Jean's to make.
