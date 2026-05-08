# Pattern Glossary

The source-side reference for tag interpretation. You see a SEAM tag in
source, you look up what its locator means here.

The project has four documentation surfaces. Two are **active references**
consulted in ongoing work; two are **historical records** that describe
how the codebase reached its current state.

**Active references** (consulted whenever working on the codebase):

| Document                          | Read when…                                                |
| --------------------------------- | --------------------------------------------------------- |
| **Pattern glossary** (this document) | I see a tag in source and want to know what its locator means. |
| **Module organization guide**     | I'm creating or restructuring a module.                   |

**Historical records** (consulted to understand how things came to be —
not as reference for current state):

| Document                  | Records…                                                       |
| ------------------------- | -------------------------------------------------------------- |
| **Seam map**              | The architectural mapping pass that preceded the cleanup work. |
| **Annotation manifest**   | Which surgical edits landed in which Claude Code sessions.     |

The current state of the codebase lives in source — the SEAM/TODO/DONE/
NOTE tags themselves are the live navigation layer. The historical
records describe the journey to that state and remain available for
context, but ongoing work should consult the active references.

When this document and the seam map disagree about a *definition*
(what a name means), this glossary is authoritative. When they
disagree about an *observation* (what a piece of code looks like),
the source itself is authoritative — both documents are descriptions
of source, and source has moved since the seam map was written.

This file lives at the repo root (or wherever the other docs live) and is
referenced by SEAM tags throughout the cartridge source.

---

## 1. Tag grammar

Tags in the source take the form:

```
PREFIX[locator] short summary
  optional continuation
  optional continuation
```

or, more often:

```
PREFIX[module:locator] short summary
  optional continuation
```

`module` is the seam-map chapter or sub-chapter the tag refers to (e.g.
`spine`, `mood`, `gallery`, `entities`, `gol_zones`). `locator` identifies
what kind of observation the tag is making — see §3.

A tag without a `module:` prefix is interpreted relative to the file it
sits in (`SEAM[spine] ...` in `cartridge.hpp` is unambiguous).

## 2. Tag prefixes

There are five prefixes. Each has a distinct purpose; mixing them is a
sign of drift.

### `SEAM[…]` — observation

A structural observation about the code, pointing back to the seam map.
SEAM tags do not imply action. They name something that is *already true*
and would otherwise have to be re-derived by every reader.

```
SEAM[entities:P10] this block is the canonical home of pattern P10
  (per-family vocabulary block). Eight family applications follow…
```

Use SEAM when you want a future reader (human or AI) to see the structure
through the same lens you did.

### `TODO[…]` — action

Action tag for Claude Code or the human author. Always carries a phase
qualifier so tasks can be batched:

```
TODO[phase-2:gol_zones] extract gol_zones.inl as freestanding header
```

A TODO without a phase tag is suspect; phases are how the seam map
sequences work, and a phase-less TODO floats outside the migration plan.

### `DONE[…]` — completed migration

Marks work that has landed. The migration's *reasoning* lives next to the
code that changed, not in a CHANGELOG. DONE tags are kept indefinitely
until the surrounding context makes them redundant — they are a paper
trail, not a regret.

```
DONE[input:L1] five copy-paste cases collapsed into one helper call.
  request_mood_transition() lives in mood.inl.
```

### `NOTE[…]` — explanatory

Annotates something that *looks* like a deviation but isn't. NOTE tags
exist to short-circuit a future reader's "this looks wrong, I should fix
it" reflex. The most common form is `NOTE[seam-map]`, used when the seam
map has explicitly considered the placement and approved it:

```
NOTE[seam-map] genuinely spine-owned, not a leak.
NOTE[seam-map] keep wrappers here; they're the integration layer.
```

NOTE differs from SEAM: SEAM points at structure; NOTE preempts a
mistaken attempt to "clean up" structure that's intentional.

### `FOLLOW-UP[…]` — queued seam-map work

Queues a future revision of the seam map itself, distinct from a code
TODO. Used when the code is fine but the documentation needs to evolve —
typically when an observation has been made in source that the seam map
hasn't yet absorbed into its narrative:

```
FOLLOW-UP[seam-map] Before extending coverage to Palm/Cactus/Blade,
  the audit's structure should be reviewed…
```

`FOLLOW-UP[seam-map]` is the canonical form. The prefix exists because
the seam map is itself a working document with a backlog: source-side
findings ("the gol_zones block houses two algorithms, not one") need a
queue that points back at the document that should be updated, separate
from any code work the finding implies. A `SEAM[…]` would mis-tag this
(it's not an observation about the *code*, it's an observation about
*coverage of the code in another document*); a `TODO[…]` would mis-tag
it too (it's not code work).

When the seam map absorbs the finding in its next revision, the
FOLLOW-UP tag is removed, replaced by a `SEAM[…]` if appropriate
(naming the structure the seam map now explicitly covers).

---

## 3. Locator types

Three flavors of locator follow the colon.

**Numbered.** `P1`–`Pn` (patterns), `K1`–`Kn` (keystone migrations),
`L1`–`Ln` (local refinements). Stable identifiers; once assigned, never
renumbered.

**Named.** A small vocabulary of well-known nouns: `:owns`, `:contract`,
`:taxonomy`, `:foundations`, `:complete-subsystem`, `:dual-role`,
`:dual-algorithm`, `:dual-entry`, `:structural`. These describe a *kind
of observation* the tag is making.

**Bespoke.** Ad-hoc descriptive locators for one-off findings, e.g.
`:per-mood-data`, `:cx-cz-mirror`, `:K2-related`. These start as one-offs;
if they recur, they get promoted to named locators or numbered patterns.

The next three sections cover each in turn.

---

## 4. Patterns (numbered)

Patterns are architectural shapes that recur across multiple modules.
Numbering is sparse for historical reasons: as patterns were named
during seam-map authoring, numbering was assigned roughly in discovery
order; some early provisional numbers got merged or dropped during
chapter writes, leaving gaps. Compacting the numbering would break
every existing tag reference, so the gaps are preserved.

Currently in use: **P1, P3, P4, P5, P8, P9, P10, P11**.
Currently unassigned: **P2, P6, P7**. Treat the gaps as reserved — do
not reassign without a deliberate review. A future maintainer *can*
assign to them if a real pattern emerges that wants a low number; the
review is to ensure the assignment doesn't collide with any prior
provisional use that left traces in the seam map's history.

### P1 — Per-frame coupling decomposed into module

A per-frame ramp or coupling lives inside its owning module as a named
`tick_*` or `update_*` function, *not* inline in `cartridge.hpp::update()`.
The spine then calls these functions in a documented sequence; the spine
itself becomes a phase orchestrator rather than a procedure.

**Canonical instance.** `orbs.inl::update_orb_coupling`. The orb system's
musical coupling (force, color, flock, speed) is decomposed into a single
module-owned function called from the spine.

**Other instances.** `musical.inl::tick_musical_couplings`,
`mood.inl::reset_musical_couplings`, `pawn.inl::tick_pawn_couplings`. All
three were created by closing musical:K2, mood:K3, pawn:K1 respectively.

**When to use.** Any per-frame logic that mutates module-local state in
response to the analysis signal. The test: if you can describe the work
as "this module's response to the frame," it belongs in a tick function
inside the module.

**When *not* to use.** Genuine integration glue — work that ties multiple
modules together and has no single owner. Those legitimately live in the
spine; see `:owns` and the spine's `update()` body.

**Identifying it in the wild.** Look for `tick_<module>_couplings(queue)`
or `update_<module>_<phase>(...)` called once from the spine, with the
implementation and the per-frame state both in the module.

**Historical note.** P1 was originally identified as the *counter-example*
to ramp-in-spine — the shape `orbs.inl` already had that demonstrated
what the K1/K2/K3 resolutions should produce. The pattern was named
after the fact; it has been the de-facto target shape for per-frame
coupling work since.

---

### P3 — Player state vs mood state, explicit split

Configuration that mixes **player-controlled state** with **mood-authored
state** is split into two named sub-structures, not flattened into one
config. The split makes provenance visible: a future reader can see which
values come from the player's commands and which come from the mood table.

**The defining property is *input cadence*.** Player state is mutated by
input events between mood transitions (a key press flips a toggle, cycles
a palette, swaps a rule); mood state is set once at mood entry and is
immutable until the next transition. The two halves participate in
different input loops, and conflating them obscures *which loop* a value
belongs to.

P3 is specifically the player-vs-mood split, not the broader pattern of
"two-source config." A config receiving values from two authored sources
that share an input cadence (e.g. mood-table-base + computed-from-mood-
at-entry) does not trigger P3 — it can be a single struct with no loss
of clarity.

**Canonical instance.** `orbs.inl` — orb config separates `OrbPlayerState`
(palette/rule/gesture cycling, anchor toggle — keys press, state mutates)
from per-mood `ORB_MOOD_TABLE` rows (set at mood entry, immutable until
next transition).

**Other instances.** `gallery.inl` painting spawn configuration
(SEAM[gallery:L2]).

**When to use.** Whenever a module receives both player-mutated state
and mood-authored state. The split surfaces the input-loop boundary.

**When *not* to use.** A config that is set once at mood entry and never
mutated mid-mood is single-source for P3 purposes, regardless of how
many places authored its values. Multiple authoring sources with the
same cadence don't need the split.

---

### P4 — Hygiene rows in lookup tables

Tables indexed by a stable enum (mood, family, tier) include rows for
indices that the consumer's gate intentionally suppresses. The
suppressed rows are filled with zeros or no-op values rather than being
omitted, so that the table is *complete* — every enum value has a row,
and the suppression happens at the gate, not at the lookup.

**The defining property: the gate decides at consumption time, the table
itself is complete and uniform.** A sparse table that only authors the
active indices is morally similar but not P4 — P4 specifically pays the
small memory cost of hygiene rows for the legibility benefit of "every
enum value has a row, scannable as a matrix." Sparse authoring forces
the reader to cross-reference against the enum to know what's missing
and why; uniform authoring with hygiene rows surfaces the gating policy
in the table itself.

**Canonical instance.** Per-family `MOOD_MULTIPLIER` arrays, e.g.
`GoLZoneSpawnConfig::MOOD_MULTIPLIER = { 1, 1, 0, 0, 1, 0 }` — the zero
entries cover moods where GoL zones are suppressed; the multiplier is
still indexable by mood ID, and the array reads as a complete picture
of the family's mood profile.

**Other instances.** Sphere, cube, and ribbon `MOOD_MULTIPLIER` arrays.
The pattern recurs across all generic-pipeline floater families and
several grounded ones.

**When to use.** Any table indexed by a stable, ordered enum where
some indices are gated off and the table is small enough that the
hygiene-row cost is trivial. The hygiene row makes the table self-
describing (a reader can see at a glance which moods are active for
this family).

**When *not* to use.** Sparse maps where the indices are unstable or
unbounded. Hygiene rows trade memory for legibility; they are not
appropriate for large or sparse tables. Also: any table where the
consumer iterates the *authored* entries rather than indexing by
enum — P4 specifically supports indexed-by-enum lookup, not
authored-set iteration.

**Identifying it in the wild.** Look for arrays sized by a stable
`*_COUNT`, where some entries are deliberately `0.0f` or no-ops, and
the consumer indexes by an enum value rather than iterating.

---

### P5 — Release-pending sentinel / race protection

A field protects against stale signals from a prior state, or encodes
intent across a boundary the CPU can't directly observe. Generalized:
when CPU mirror and GPU truth disagree about timing or state, encode
the disagreement so the later-arriving side knows to re-check or act.
Three common shapes:

- **GPU-side sentinel value.** The CPU writes a sentinel value into a
  GPU buffer field that the kernel recognizes next frame and acts on.
  Used when the CPU can't accurately predict GPU state but can encode
  "do this on the next read." Avoids CPU drift estimation and readback
  latency.
- **Generation counter.** A monotonic `worldGen_` (or equivalent) is
  bumped at teardown; in-flight callbacks check the counter and
  drop themselves if it has changed.
- **CPU-side timestamp.** `last_alloc_time` on a slot protects
  freshly-allocated slots from being marked inactive by a stale
  GPU readback that targeted the previous occupant.

**Canonical instance.** `floaters.inl::toggle_cube_kite_mode` writes
`follow_pawn = 2u` as the GPU sentinel for "kite mode OFF, take the
next opportunity to corral." This is the originating P5 instance —
the one that earned the pattern its name. The pattern was generalized
in seam-map Ch. 14 to cover the other mechanisms.

**Other instances.** `worldGen_++` at `cartridge.hpp::update()::
TEARDOWN` (generation counter, SEAM[spine:P5]); `ActiveCube::
last_alloc_time` and `ActiveFloater::last_alloc_time` (CPU-side
timestamps protecting slot reuse against stale GPU readbacks). Same
intent, different mechanisms.

**When to use.** Any time work is queued asynchronously across a state
boundary the producer doesn't directly observe — readbacks, deferred
callbacks, queued GPU dispatches — or when the CPU needs to encode
intent that the GPU will act on next frame. The cost (one field, one
increment, or one sentinel value) is small; the bug it prevents (stale
data corrupting a fresh slot, or the CPU and GPU disagreeing about
when a transition happened) is hard to debug.

**When *not* to use.** Synchronous work in a single state. Adding the
sentinel "just in case" is overengineering; add it when an actual
async or CPU-vs-GPU boundary exists.

**Identifying it in the wild.** Look for `*Gen_`, `last_alloc_*`,
`pending_*`, sentinel values like `follow_pawn = 2u` written into GPU
buffers, or any field whose comment says "guards", "protects",
"stale", or "next frame."

---

### P8 — Latent infrastructure

Code written ahead of the feature that will use it. Currently
unreferenced, but the artist's note-to-self about what's coming.
Distinct from dead code (a feature that came and went) — latent
infrastructure has a known migration path; dead code does not.
Default to "latent" when in doubt: the cost of carrying is small,
the cost of deleting a future foundation is large.

**The pattern takes many shapes.** Among them:

- **Placeholder functions or code paths.** Entire functions or
  branches written ahead of consumers — `compute_sun_matrices`
  (the sun's musical expression awaits), `generic_compute_colors`
  (the default for future non-exotic families), WGSL placeholder
  stubs (`contrib_paintings_base_at`, `contrib_vegetation_base_at`,
  `query_ground_celestial`, `query_ground_walker_agent`).
- **Commented-out or deferred struct fields.** Fields scheduled to
  migrate in once their owning module exists, e.g.
  `PlayerState::aura_presence`, `mmode_intensities` (scheduled for
  migration once `pawn.inl` exists).
- **Reserved registry entries.** Binding slot reservations,
  COUPLING_* bits held for legacy systems, near-zero-weighted tier
  or population entries (e.g. `ENVIRONMENTAL` shot tier with weight
  0.01 in `gallery.inl`).

**Canonical instance — C++ side.** `PlayerState`'s `// Future
(deferred):` block (SEAM[spine:P8]).

**Canonical instance — WGSL side.** The pattern is older than its
seam-map name. The WGSL audit identified ~15 explicit `reserved` and
`placeholder` annotations across the file, in active production use —
binding slot reservations, COUPLING_* bits held for legacy systems,
and stub functions. P8 was named on the C++ side after the codebase
had been practicing it on the GPU side all along.

**Other instances.** `ENVIRONMENTAL` in `SHOT_PARAMS`
(DONE[gallery:L1]); `compute_sun_matrices`; `generic_compute_colors`.

**When to use.** When deletion would force you to renumber, reindex,
or remove a label that would be hard to restore — and when the
feature consuming the latent code is genuinely on the roadmap. The
comment names what the latent infrastructure is *for*, so a future
reader understands the weight, the empty field, or the unreachable
function is a placeholder, not an oversight.

**When *not* to use.** As a substitute for actual TODO discipline.
Latent code with no path to activation should be deleted; this
pattern is for things that have a known migration path but aren't
ready yet. If you can't say what feature will consume it, it isn't
latent — it's dead.

**Identifying it in the wild.** Look for `// Future:`, `// Deferred:`,
`// reserved`, `// placeholder`, weights below ~0.05, struct members
behind `#if 0` or commented out, and **functions or branches that
aren't called from anywhere yet but are documented as future
foundations**.

---

### P9 — Library without state

A module that is a textbook collection of pure free functions — no class
members touched, no domain assumptions, no compile-order constraints.
The easiest module to extract from the cartridge class body, since its
only dependency is on the C++ standard library.

**Canonical instance.** `seed_utils.inl` — `cpu_hash`, `cpu_hash_f`,
`tile_seed`, `cpu_lattice_node_seed`, `cpu_smoothstep`,
`cpu_sample_gaussian`, `select_tier`. Eight functions, zero state, zero
domain knowledge.

**Other instances.** `trajectory.inl` is close (one struct + one
function, but the struct is shape-mirrored against WGSL — see
`:contract` for that lens).

**When to use.** Whenever a module emerges that is genuinely pure. P9
modules are the first candidates for promotion from `.inl` to `.hpp`,
since they have no class-body coupling to lose.

**When *not* to use.** Modules that read or write `Cartridge::*` state
are not P9 even if their public surface is small. The defining
property is *zero* class-body coupling.

**Identifying it in the wild.** A module where `static` is the right
qualifier on every function and no member variables are referenced.

---

### P10 — Per-family vocabulary block

Each entity family has a fixed structural template: tier enum, color
palette, config struct, property index registry, active-tracking
struct. Ten families instantiate the same shape across two files. The
repetition is intentional — each family's specifics differ in
important ways, but the *shape* is uniform so a reader who has seen
one family knows where to look in any other.

**Canonical instance.** `entities.inl` is the named home of P10
(SEAM[entities:P10]) and holds eight instances: seven grounded
families (Arch, Column, Antenna, Palm, Cactus, Blade, Pyramid) plus
Ribbon as a bespoke-family vocabulary instance. Two further P10
instances live in `floater_vocabulary.inl`: Sphere and Cube. Total:
**ten instances** of the template across the codebase.

The eight-instance count occasionally seen is a near-miss caused by
counting only the SEAM tag's home file. The pattern's correct scope
is cross-file.

**When to use.** When adding a new entity family. Don't fight the
cookie-cutter; copy the template and fill in the specifics. The
uniform shape is what lets `entity_pipeline.inl` be data-driven.

**When *not* to use.** Subsystems that aren't entities-with-tiers —
GoL zones, gallery paintings, agents. These have their own shapes
(see `:complete-subsystem` and the agent-specific surface in
`agents.inl`).

**Identifying it in the wild.** Look for the sequence: `enum class
*Tier { ... }`, `*_PALETTE[]`, `struct *Config { ... }`,
`struct *Prop { ... }`, `struct Active* { ... }`.

**See also.** Sibling entity families in P10 are sometimes related by
"design cell division" — column and antenna are an example. The
divergence between siblings deserves its own per-adapter "Differs
from sibling" comment in `entity_pipeline.inl` (planned).

---

### P11 — Templated active-array helper

A single templated function that operates uniformly across all `Active*`
structs that share a known set of fields (`active`, `patch_gx`,
`patch_gz`). One implementation, many callers. The C++ template
mechanism replaces what would otherwise be 8+ copies of a near-identical
function.

**Canonical instance.** `spawn_engine.inl::run_spawn_preamble<ActiveT>`
— the spawn gate's idempotency-through-slot-reservation logic. Used by
every entity family's `*_run_gate` adapter function.

**When to use.** When several `Active*` structs already share the
relevant fields and the operation is the same across them. P11 is the
template-level analog of P10: P10 says "all families have the same
shape," P11 says "all families can be processed by the same algorithm."

**When *not* to use.** When the operation varies meaningfully across
families (per-family color logic, per-family tier sampling). Those
correctly stay in the per-family adapter.

**Identifying it in the wild.** A `template<typename ActiveT>` function
that touches `arr[i].active` and `arr[i].patch_gx/gz`.

---

### P2, P6, P7 — unassigned

Numbering gaps. The sparse numbering reflects seam-map authoring history
(early provisional patterns merged or dropped during chapter writes);
the gaps are not "rejected" patterns, just numbers that ended up unused.

Treat as reserved going forward: do not reassign without a deliberate
review. The review exists to ensure that a future assignment doesn't
collide with traces of provisional use left in the seam map's history.
If a clear new pattern emerges and a maintainer wants to claim a low
number, the path is open.

---

## 5. Named locators

Named locators describe a *kind* of observation rather than a specific
instance. They form a small vocabulary; new ones should be added
sparingly.

### `:owns` — declarative ownership claim

Asserts that a piece of code legitimately belongs in the layer it sits
in, even when it might look like a leak. Used most often in
`cartridge.hpp` to defend code that *looks* like it should be in a
module but is actually integration work.

**Examples.** `SEAM[spine:owns]` on `FAMILY_DISPATCH`, `render()`,
`stream_patches`. Each one defends spine residence against the
default assumption "this should be in a module."

### `:contract` — CPU/GPU agreement constraint

Names a place where a CPU value, struct, or function must match a
WGSL counterpart. Often paired with a `static_assert` for byte-level
checks; for semantic checks (e.g. release-rate formulas), the
`:contract` tag is the load-bearing thing.

**Examples.** `SEAM[trajectory:contract]` on the trajectory release
formula. `SEAM[seed_utils:contract]` on `cpu_lattice_node_seed`,
`cpu_sample_gaussian`. Across the codebase, "MUST match WGSL" or
"must agree" comments are members of this family.

### `:taxonomy` — classification claim

Names where in the structural taxonomy a thing belongs, often pre-
empting a "should this be elsewhere?" question.

**Examples.** `SEAM[entities:taxonomy]` (vocabulary for grounded
families lives here, not in `cartridge.hpp`). `SEAM[ribbon:taxonomy]`
(machinery here, vocabulary in `entities.inl` — the unique split).
`SEAM[sphere:taxonomy]` (sphere vocabulary lives in
`floater_vocabulary.inl`, not `entities.inl`).

### `:foundations` — pure-math/utility module

A module that lives in the foundational layer — no domain logic, no
class-body coupling, available to all consumers above it. Often
overlaps with P9 but is more about *position* in the dependency
graph than about *shape*.

**Examples.** `SEAM[trajectory:foundations]`.

### `:complete-subsystem` — bespoke pipeline

A subsystem that owns its full lifecycle in a single module: vocabulary
+ state + lifecycle + dispatch all together. Distinct from generic-
pipeline families, which are decomposed across `entities.inl` (vocab),
`entity_pipeline.inl` (machinery), and per-family adapters.

**Examples.** `SEAM[gallery:complete-subsystem]`,
`SEAM[gol_zones:complete-subsystem]`. Ribbon is a partial member —
its machinery is in `ribbon.inl` but its vocabulary is in
`entities.inl`.

### `:dual-role`, `:dual-algorithm`, `:dual-entry` — intentional sharing

Names a block that intentionally carries two related concerns. The
"dual" prefix flags that the duplication is deliberate; without the
tag, a reader might try to split it.

**Examples.**
- `SEAM[gallery:dual-role]` — painting-on-terrain and painting-on-wall
  share infrastructure but spawn divergently.
- `SEAM[gol_zones:dual-algorithm]` — Conway and Pulse algorithms share
  the zone subsystem, gated by `PULSE_ALGORITHM_CHANCE`.
- `SEAM[ribbon:dual-entry]` — `commit_ribbon` has two callers
  (`FAMILY_DISPATCH` and mood forced spawn).

### `:structural` — file-organization observation

A tag about how the *file itself* is laid out, rather than about the
code semantics. Used to defend unusual structural choices.

**Examples.** `SEAM[spawn_engine:structural]` defends the load-bearing
mid-block `#include "modules/entity_types.inl"` — explains why the
include lands where it does (C++ union member ordering constraint)
and why the file wasn't split into pre/post halves.

---

## 6. Bespoke locators

When an observation is a one-off — useful enough to tag, not yet
recurring enough to deserve a named slot — it gets a bespoke locator.
These are descriptive nouns or short phrases.

Currently in use:

- `:per-mood-data` — finding about per-mood data ownership in the spine
- `:K2-related` — work adjacent to keystone migration K2
- `:cx-cz-mirror` — `ActiveCube` field that mirrors a GPU value

A bespoke locator should be considered for promotion to a named locator
or pattern when it appears in three or more places. Until then,
descriptive ad-hoc locators are preferable to forced naming.

---

## 7. Numbered locators that are not patterns

### `K1, K2, K3, …` — keystone migrations

Project-level structural moves, each given a number scoped to a module.
A keystone migration is bigger than a local refinement and usually
involves coordinating changes across multiple files. Each Kn has a
definite scope and a closure condition: it is either pending or done.

**Examples.**
- `mood:K1` — indoor/outdoor binary lives as `bool finite` plus shell
  fields.
- `musical:K2` — per-frame ramps moved out of `cartridge.hpp::update()`
  into `tick_musical_couplings()`.
- `mood:K3` — per-mood-transition reset moved into
  `reset_musical_couplings()`.
- `pawn:K1` — aura presence ramp moved into `tick_pawn_couplings()`.
- `mood:K4` — mood-5 row is the bit-identical reference clone of
  mood-0.
- `entities:K1` — `TierProfile` per-family migration (Option B).
- `spine:K2` — large-scale dispatch consolidation work (the
  `:K2-related` bespoke tag refers to follow-on work in this area).

**Lifecycle.** Open: `TODO[phase-N:module:Kn]`. Closed:
`DONE[module:Kn]`. The DONE tag is preserved in the source as a paper
trail.

### `L1, L2, L3, …` — local refinements

Smaller, file-local cleanups. Less weight than a keystone migration;
typically one file, one commit.

**Examples.**
- `input:L1` — five copy-paste cases collapsed into a single helper
  call.
- `gallery:L1` — `ENVIRONMENTAL` weight kept at 0.01 deliberately
  (latent infrastructure; see P8).
- `agents:L2` — GPU constants must match (hardware-mirror family).
- `musical:L2` — mode-name registry promoted to module-level constant.
- `gol_zones:L1` — `MODE_LATTICE_SPACING` mirrors WGSL.

**Lifecycle.** Same as Kn but lighter weight.

---

## 8. Cross-cutting families

Some patterns are not patterns proper — they are families of related
SEAM tags scattered across modules, each instance with its own locator
but sharing a kind.

### Hardware mirror family

CPU code that must agree exactly with a WGSL counterpart. Members:
`trajectory:contract`, `seed_utils:contract`, `agents:L2`,
`gol_zones:L1`, `cube:cx-cz-mirror`, `musical` (PULSE_MAX_AGE), and
the `static_assert`s on byte-aligned GPU structs in `state.hpp` (S4).

The family is held together by the `:contract` named locator and by
"MUST match WGSL" comments.

**Canonical phrasing.** `:contract` is the canonical named locator for
this family. The phrases "FXC mirror" and "hardware mirror" appear in
some existing tags and comments as legacy framings; they refer to the
same family, but new tags should prefer `:contract`. Existing tags
using the legacy phrasings will harmonize toward `:contract` over time
during cross-module rollout work.

**When to use which.** Prefer `:contract` if the agreement constraint
is the module's defining surface (a foundation module whose entire
purpose is to mirror a WGSL primitive). Prefer an `Ln` locator if the
constraint is local to a single field or constant within a module
that has other concerns.

### Cookie-cutter family

The intentional repetition across P10 instances and P10-derived
adapter blocks (in `entity_pipeline.inl`). Named in
`SEAM[entities:P10]`: "Don't fight the cookie-cutter — it's
intentional specificity per family." When refactoring entity code,
this tag is the load-bearing reminder *not* to over-generalize.

### Phase numbers

`Phase 1, Phase 2, …` are the migration phases per Ch. 15 of the
seam map. Phases are not locators but they are referenced in TODO
and DONE tags as the work-batching unit. Current state of phase
numbering should be checked in `seam_map.md`.

---

## 9. Working with the glossary

### Adding a new pattern

There are two paths to a new Pn entry.

**Path A — earned by recurrence.** The third instance is the one that
earns the name. First instance: maybe a one-off. Second: coincidence.
Third: pattern. A new Pn entry is justified when:

1. The shape recurs in **three or more** independent places in the
   source.
2. The shape has a name people already use informally when discussing
   the code.
3. A future reader who saw one instance would be helped by knowing the
   others exist.

**Path B — named in the seam map with a predicted instance set.** A
pattern can be named *before* it has three instances if the seam-map
authoring or design conversation explicitly identifies the shape and
predicts future instances. P10 was assigned when only Arch and Column
had been tagged, with the prediction that the other six grounded
families would instantiate it — and they did. This path is for shapes
that are obviously load-bearing in the architecture before all
instances exist; it should be used sparingly, since a prediction that
fails to materialize leaves the codebase with an entry that doesn't
earn its keep.

For path B, the entry should explicitly note the prediction at time of
assignment: "Predicted instances: …; instances at v1.0: …."

The next available pattern number is the smallest unused integer
greater than 0. Do not skip numbers (except where existing gaps
already exist — P2, P6, P7 are unassigned by history).

A new pattern entry must include: description, canonical instance,
other instances (at least two for path A; at least one for path B with
a predicted set), when-to-use, when-*not*-to-use, and how to recognize
it.

### Promoting a bespoke locator

A bespoke locator (`:foo`) is promoted to a named locator when it
appears in three or more places with a consistent meaning. The
promotion threshold for named locators is lower than for numbered
patterns because a named-locator promotion is a smaller commitment —
it adds a §5 entry but doesn't claim "this is a recurring architectural
shape," only "this name is used consistently enough to deserve a
definition." Existing tags do not need to be rewritten unless the
meaning has drifted.

### Retiring a pattern or locator

Patterns are retired by **annotation**, not deletion. If P3 ceased to
recur, its entry would gain a "**Retired** as of ..." line. The
number is not reclaimed.

The same holds for Kn and Ln — even closed migrations keep their
entries, since the DONE tags in the source reference them.

### Relationship to the other documents

The glossary and the module organization guide are the **active
references** for ongoing work. The glossary defines *what* tags mean;
the organization guide prescribes *how* a module should be shaped.
The two compose: the organization guide references patterns by number
when a pattern is the cleanest way to name a module-level move.

The seam map and the annotation manifest are **historical records**.
The seam map walks the codebase top-to-bottom as it stood at the time
of the mapping pass; the manifest records which surgical edits landed
in which Claude Code sessions. Both are useful for context — "why was
this decision made" or "when was this work done" — but they describe
a state of affairs that has since moved.

New patterns get their entries here in the glossary. Existing patterns
in the seam map's pattern table (Ch. 14) remain as historical record
of how each pattern was originally framed; the glossary entry is the
current authoritative definition. If the two disagree, the glossary
wins for *definitions*. If the seam map and source code disagree about
an *observation*, the source wins — both documents describe source,
and source has moved since the seam map was written.

The annotation manifest reports the source's tag inventory at a moment
in time; current tag inventory is whatever's actually in source today.
A `git grep 'SEAM\['` is more authoritative than the manifest for
"what tags exist now."

---

## 10. Quick reference

| Locator       | Kind     | Meaning                                    |
| ------------- | -------- | ------------------------------------------ |
| `P1`          | Pattern  | Per-frame coupling decomposed into module  |
| `P3`          | Pattern  | Player state vs mood state, explicit split |
| `P4`          | Pattern  | Hygiene rows in lookup tables              |
| `P5`          | Pattern  | Release-pending sentinel                   |
| `P8`          | Pattern  | Latent infrastructure                      |
| `P9`          | Pattern  | Library without state                      |
| `P10`         | Pattern  | Per-family vocabulary block                |
| `P11`         | Pattern  | Templated active-array helper              |
| `Kn`          | Numbered | Keystone migration                         |
| `Ln`          | Numbered | Local refinement                           |
| `:owns`       | Named    | Belongs in this layer (defends placement)  |
| `:contract`   | Named    | CPU/GPU agreement constraint               |
| `:taxonomy`   | Named    | Classification claim                       |
| `:foundations`| Named    | Pure-math/utility module                   |
| `:complete-subsystem` | Named | Bespoke pipeline (full lifecycle in one block) |
| `:dual-role`  | Named    | Block carries two related concerns         |
| `:dual-algorithm` | Named | Block carries two algorithm variants     |
| `:dual-entry` | Named    | Single function with two callers           |
| `:structural` | Named    | File-organization observation              |

| Prefix          | Use                                                   |
| --------------- | ----------------------------------------------------- |
| `SEAM[…]`       | Observation about existing structure                  |
| `TODO[…]`       | Action item, always with a phase qualifier            |
| `DONE[…]`       | Completed migration, kept as paper trail              |
| `NOTE[…]`       | Preempts a mistaken "fix" of intentional structure    |
| `FOLLOW-UP[…]`  | Queues work on the seam map itself                    |

---

## Version

**v1.2.** Folds in the cross-check pass against the seam map's
authoritative chapter discussions. Three substantive edits and one
optional refinement landed.

- **P5 broadened.** Added GPU-side sentinel value as a third
  mechanism, alongside generation counter and CPU-side timestamp.
  Canonical instance changed from `worldGen_++` to
  `floaters.inl::toggle_cube_kite_mode`, the originating P5 instance
  per the seam map's history. `worldGen_++` and `last_alloc_time`
  fields moved to "other instances."
- **P8 broadened.** Reframed from "two shapes" to "many shapes."
  Added placeholder-functions-and-code-paths as a third major shape
  (e.g. `compute_sun_matrices`, `generic_compute_colors`, the WGSL
  stubs `contrib_paintings_base_at`, `query_ground_celestial`).
  Renamed the entry from "Latent infrastructure / commented-future
  fields" to just "Latent infrastructure" since commented-future-
  fields is only one shape. Added a WGSL-side canonical instance
  noting that the pattern is older than its seam-map name and is
  more widespread on the WGSL side (~15 instances) than the C++
  side.
- **P10 corrected.** Instance count fixed from eight to ten. Sphere
  and Cube live in `floater_vocabulary.inl` and were missed in v1.0
  because the count was extracted from the SEAM[entities:P10] tag's
  home file. Added a brief note explaining the near-miss so a
  future maintainer doesn't re-introduce the off-by-two.
- **P1 historical note (optional refinement).** Added a brief note
  that P1 was originally identified as the *counter-example* to
  ramp-in-spine, named after the fact once the K1/K2/K3 resolutions
  used it as a target shape.

P9 and P11 confirmed without changes. The optional P9 refinement
(contrast with an unnumbered Speaker-at-end-of-chain pattern) was
deferred until that pattern receives a number, since cross-references
to unnumbered patterns dilute the entry.

**Methodological note.** v1.0 generalized P3 too broadly from a single
source-side tag; v1.1 corrected P3 and P4 but predicted that the
remaining six entries would need only narrowing. The v1.2 cross-check
revealed that two of those six (P5, P8) needed *broadening*, not
narrowing — both because the originating instances and broader scope
are documented in the seam map's chapter discussions but not in the
SEAM tags I worked from. The shared root: source-side tags are an
incomplete evidence base. The seam map's chapter discussions are
authoritative for pattern scope, and the cross-check pass is the
mechanism by which glossary entries are reconciled with that authority.

**Pending for v1.3 (no urgency).** The v1.2 cross-check examined the
seam map's chapter index and Ch. 14's pattern table but did not
re-read every chapter line-by-line. If the agents/orbs rollout surfaces
a SEAM tag whose locator definition feels off against this glossary,
that is a candidate for v1.3 review — it may be a deeper-reading
discovery the v1.2 pass didn't reach.

---

**v1.3.** Reframed the documentation toolkit from "four equally-active
references" to "two active references plus two historical records."

The seam map and the annotation manifest were tools for the structural
mapping pass that preceded the cleanup work. That pass is finished. The
documents remain valuable as historical context — "why was this decision
made," "when did this work land" — but they aren't consulted as
authoritative reference for current work. The current state of the
codebase lives in source tags; the active references are this glossary
(for tag vocabulary) and the module organization guide (for module
hygiene).

Changes:

- **Opening table reframed.** The four-document table is split into
  "Active references" (glossary, organization guide) and "Historical
  records" (seam map, annotation manifest). The framing tells the
  reader which docs to consult for which kinds of work.
- **§9 Relationship section rewritten.** Where v1.2 said "new patterns
  should be discussed in two places: the glossary and the seam map's
  relevant chapter," v1.3 says new patterns get their entries here in
  the glossary alone — the seam map is closed historical record. The
  authority resolution between glossary and seam map is recast: the
  glossary wins for definitions; for observations, source itself is
  authoritative since both documents describe source and source has
  moved.
- **Manifest framing adjusted.** The annotation manifest is named as
  a snapshot in time; for current tag inventory, `git grep` is more
  authoritative than the manifest.

This revision changes nothing about pattern definitions, locator
meanings, or working discipline. It changes only how the document
positions itself relative to the rest of the documentation toolkit.
