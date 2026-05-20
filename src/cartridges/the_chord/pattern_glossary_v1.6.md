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

Currently in use: **P1, P3, P4, P5, P8, P9, P10, P11, P12**.
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

### P12 — Integration glue: per-family wrappers binding modules to a dispatch table

A class of small wrapper functions whose only job is to bind per-family
module functions to function-pointer slots in a dispatch table. Each
wrapper is uniform in shape (typically 3–12 lines), carries no domain
logic, and lives near the dispatch table that consumes it. Adding a
new family means: write the family's logic in its owning module, add
wrappers here, add one row to the dispatch table.

**The defining property: the wrapper's only job is signature-shaping
between the table's call convention and the module's function shape.**
If the wrapper does any non-trivial work — parameter transformation,
state lookup, error handling — it isn't P12, it's a function that
happens to be called from the dispatch.

**Canonical instance.** `cartridge.hpp`'s FAMILY_DISPATCH wrapper
section (SEAM[spine:family-dispatch], ~400 lines): `dispatch_evict_*`,
`dispatch_prepare_mesh_*`, `dispatch_mesh_gen_*` for all 12 entity
families, each a 3–12 line wrapper around the family's module
function. The wrappers exist because FAMILY_DISPATCH is a constexpr
table of function pointers with a uniform signature, and the per-
family functions have slight signature variations that the wrappers
normalize.

**Other instances.** `entity_pipeline.inl`'s per-family dispatch
adapters — each generic-pipeline family contributes 3 wrappers
(dispatch_select_<family>_generic, dispatch_place_<family>_generic,
dispatch_commit_<family>_generic). Same shape, same role, different
file because the eight generic-pipeline families share machinery and
their wrappers belong with that shared machinery.

**When to use.** When a uniform table-driven dispatch needs binding
to per-family or per-X module functions whose signatures can't be
made identical at the source. The wrappers are honest about
themselves — they're glue, not logic. Naming them and grouping them
makes the integration boundary visible.

**When *not* to use.** When the dispatch can route directly to the
module function (signature already matches the table). P12
specifically describes the case where wrapping introduces small but
non-zero work — parameter forwarding, slight signature reshape, host
pointer resolution.

**Identifying it in the wild.** A long contiguous block of small
wrapper functions, each with a near-identical shape, called only
from a single dispatch table or function-pointer registry. Each
wrapper's body is "forward args, call module function, return
result" with at most light reshape.

**Adjacent to.** P12 is adjacent to but distinct from P11 (Templated
active-array helper). P11 collapses N near-identical functions into
one template; P12 keeps them separate because each binds to a
different module function. The decision: if the *target* of the work
is identical across families, use P11; if the target differs per
family but the *shape* of the wrapping is identical, use P12.

---

### P13 — Leaf module vs orchestration module

A taxonomic distinction governing **where module state lives**. Modules
fall into two shapes that the contributor experience must respect.

**Leaf modules** are called *by* the spine but do not call back into
other module surfaces. Their `.inl` file can be `#include`'d early in
the Cartridge class body, making their state struct visible to every
later include. Examples: `ribbon`, `gol_zones`, `agents`, `cube_behaviors`,
`pawn`, `gallery`, `orbs`, `musical`, `entities`. State for these
modules lives **inside the module's `.inl`** as `XState x_state_;`.

**Orchestration modules** call into many other module surfaces and
must therefore be `#include`'d *after* every module they depend on.
Their includes naturally land at the end of the class body. Their state
cannot live in the module's `.inl` because static functions in earlier-
included leaf modules cannot see types declared later. Their state
must be **spine-resident** (declared near the top of `cartridge.hpp`).

**The defining property.** The dependency direction. A module is
orchestration iff it calls into other module surfaces; a module is leaf
iff it is only called from spine code (or from `cartridge.hpp` body
helpers). This is observable in source: the orchestration module's
`.inl` contains calls like `configure_orbs(...)`, `reset_musical_couplings(...)`,
`force_spawn_back_portal(...)`, etc.

**Canonical instance — leaf.** `musical.inl` (P1 + P3 + P10 + P11
combined). Defines `MusicalState ms_state_;` at the top of the file.
Static functions take `MusicalState&` explicitly. Included near the
top of the Cartridge class.

**Canonical instance — orchestration.** `mood.inl`. Calls into orbs,
musical, agents, gallery, ribbon, indoor-shell, portal, and ~20 other
helpers from cartridge.hpp body. Its include sits at the end of the
class body. `MoodState mood_state_;` is therefore declared in
`cartridge.hpp`'s spine block (alongside `WorldState`, `TimeState`,
`PlayerState`), not inside `mood.inl`.

**When to use.** When deciding where a new module's state struct
should live: trace the module's outbound function calls. If it calls
into other module surfaces, it's orchestration — its state goes spine-
resident. If it only emits writes to GPU buffers and is itself called,
it's a leaf — its state goes in the module's `.inl`.

**When *not* to use.** As a description of code that doesn't have a
state struct yet (config tables, header-only utilities). P13 is
specifically about **state location** for stateful modules.

**Identifying it in the wild.** A search for `c->X(...)` calls inside
the module's `.inl` where `X` is another module's exported function.
Many such calls → orchestration. None → leaf.

**Adjacent to.** P13 is the structural prerequisite for P1 (per-frame
coupling decomposed into module): P1 describes the function-shape
pattern, P13 describes which subset of those functions can take
`State&` directly versus which must reach state through a `Cartridge*`
parameter. Leaf modules satisfy both. Orchestration modules satisfy
P1 but not the leaf-state-locality half of the convention.

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

### Naming conventions for tunable constants

Constants that scale a per-frame coupling typically take one of two
forms, and the names should reflect which:

- **Gain coefficients** — named `*_GAIN`, with the implicit form
  `output = base * (1 + intensity * GAIN)`. The gain expresses how
  much the coupling can amplify the base value when intensity is at
  full. Output range: `[base, base * (1 + GAIN)]`.
- **Ceiling caps** — named `*_CEILING`, with the form
  `output = min(intensity * scale, CEILING)` or similar. The ceiling
  expresses an absolute upper bound that the coupling cannot exceed.

These two forms are not interchangeable: a `*_CEILING` gives an
absolute bound, while a `*_GAIN` gives a multiplier on the base.
Misnaming a gain as a ceiling causes silent off-by-one drift in
commentary — `GOL_TICK_GAIN = 3.0f` produces `1 + 1.0 × 3.0 = 4×`
slower zones, not "up to 3× slower" as the name suggests.

**Where these live.** Gain constants live with the *coupling that
drives them*, not with the *parameter they scale*. The coupling layer
holds the intensity-to-gain mapping; the consumer module reads only
the result. This may put a gain constant in one module and the
parameter it scales in another — that's correct. The producer module
declares; consumer modules read across the include boundary.

### Stripping authoring artifacts

Comments describe present behavior, not the path that got us here.
Several phrases recur as authoring-process residue and should be
stripped during structural cleanup passes:

- **Phase numbers.** `Phase 1`, `Phase 2.3`, `Pass 4`, `Step 5` —
  refer to the phase numbering of the cleanup work. Strip the phase
  reference; preserve the substance.
- **Seam-map authoring chunks.** `Ch. N chunk M`, `seam map chunk N`,
  `(Ch. 15 chunk 1)`, `Per Ch. 15 of the seam map` — refer to the
  development chapters of the seam map document. Strip; the seam
  map is now historical record (per §9 above).
- **(was X) breadcrumbs.** `PORTAL_DENSITY = 1.00f (was 0.25)`,
  `15% (was 12%)` — record old values that the comment describes.
  Strip.
- **(legacy X removed) comments.** `(legacy chart constants removed)`,
  `(verify_motor_norm removed — diagnostic, never called)`,
  `(zone_terrain_height removed in Step 5 ...)`. Strip; the file
  describes what's there now, not what used to be.
- **First X migrated style claims.** `First family migrated`, `Pass 1
  scaffold`, `(Renamed from pawn_aura.inl ...)`. Strip; either the
  work is done or it isn't.
- **NEW FINDING wording.** `NEW FINDING (Ch. 15)`, `(new finding —
  …)`, `Verified by Ch. 13 chunk-3 read`. Strip; if the substance is
  load-bearing, it belongs in a SEAM tag at the load-bearing site.
- **Stale completion checklists.** `What to delete after blade
  migration is validated`, `Future cleanup notes that no longer
  apply`. Strip if the work is done; convert to TODO if it isn't.

**Sweep recipe.** Use grep with patterns:
`Phase [0-9]`, `Pass [0-9]+ `, `Step [0-9]`, `Ch\. [0-9]+`,
`chunk [0-9]+`, `\(was [0-9]`, `\(legacy.*removed`, `NEW FINDING`,
`First.*migrated`, `Renamed from`. Apply during structural cleanup
passes; preserve the substance and strip the temporal-process wording.

### Cross-file tag declarations

When other files reference your tags by name (e.g. `SEAM[mood:K3]`),
your file must declare those tags with a corresponding annotation at
the appropriate site. References without declarations are dangling —
they read like a promise the destination file hasn't kept.

The cleanup discipline:

1. **For every cross-file reference**, verify the destination file
   has a corresponding declaration. A grep sweep across the codebase
   identifies dangling references.
2. **Declarations belong at the load-bearing site**, not in the
   header. `SEAM[mood:K3] anchor — mode intensities, pulse, palette
   drift` lives at the call site that closed K3, not just at the
   top of mood.inl. The header may also carry the tag, but the
   anchor site is mandatory.
3. **Compound tags should be split.** `DONE[musical:K2 / mood:K3]`
   (one tag bracket carrying two locators) should become two clean
   tags: `DONE[musical:K2]` plus `DONE[mood:K3]`, each at the
   appropriate site. The compound form can't be matched by tools
   that scan the codebase for tag declarations.

### Indentation policy for source comments

When promoting comments from the spine to a module include site (or
similar relocations):

- **If the comment is currently indented** (typically 12 spaces, the
  class-body level), preserve the indentation level it had at its
  destination. Class-body context expects 12-space indent.
- **If the comment is currently at column 0**, leave it at column 0.

The principle: don't reformat comments unless the relocation forces
a column change. A 12-space-indented class-body comment that gets
moved into another class-body context stays at 12-space indent.
A column-0 file-header comment stays at column 0 even when nestled
near indented code. Files that arrive at an audit pre-aligned at
column 0 don't need an un-indent pass.

### Section numbering in WGSL files

WGSL files (currently `world.wgsl`) use chapter-numbered sections
(§1, §2, §3.4, §8.0.5) as the primary navigation. The numbering
reflects **file position**, not topical grouping.

If a topical sibling needs to live elsewhere for binding-locality or
forward-reference reasons, give it the destination's positional number
(e.g. `§8.0.5` for a frustum-cull pass that's topically a §7 compute
pass but lives between §8.0 and §8.1) and add a NOTE explaining the
placement. Topical grouping is captured in the SECTION MAP at the top
of the file; the granular numbering must follow the file's actual
order so that navigation by number works.

### Lineage notes within a P10 host

When a module hosts multiple instances of a pattern — most often P10
(per-family vocabulary block) — and some instances are genealogically
related (sibling pairs, design cell divisions, shared design
constraints), the relationships are captured under a "Lineage."
paragraph in each family banner.

Examples currently visible in `entities.inl`:

- **Column ↔ Antenna**: explicit sibling pair. Antenna is a design
  cell division from Column (shared `ColumnTierRow` shape, shared
  `ActiveColumn` struct, shared mesh-gen pipeline; distinct tier
  vocabulary, distinct GPU tier indices).
- **Palm / Cactus / Blade**: vegetation cluster — no piers, no
  collision solids, no heightfield contribution. Same paradigm
  (body-part bases with per-instance variance), distinct mesh
  shapes.

The Lineage paragraph names what a family inherits structurally and
where it diverges. Useful when adding a new family that's a sibling
of an existing one, and when a future reader is trying to understand
why two families look so similar.

### Input-vocabulary fluidity

Input bindings in this codebase are fluid scaffolding. Numpad keys,
function keys (F1/F2/F3), Caps Lock, and other diagnostic toggles
are temporary placeholders during development — the *functions*
they control will persist (and may be driven by music, MIDI, or
other inputs in the future); the *keys* are placeholder bindings.

This is structural rather than incidental. The cartridge sits below
an analysis layer that consumes letter keys (A–Z) as MIDI piano
notes; pressing a letter is *literally* playing a note. Diagnostic
controls live on function keys, the numpad, and Caps Lock because
those keys aren't claimed by the synth above. The visualizer is
being driven by music in the strict sense.

The convention this enables: it's normal for a key binding to move
between development sessions. Tag the *function* as the durable
artifact; tag the *binding* as scaffolding. The framing belongs in
input.inl's header.

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
| `P12`         | Pattern  | Integration glue (per-family wrappers)     |
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

## 11. Reference exemplars

A small list of files and code blocks that are the cleanest live
instances of the conventions described above. When working on a new
module or wondering "how should this be shaped," these are the files
to compare against.

### `cube_behaviors.inl` — all conventions in one place

The most thorough application of glossary conventions in the codebase.
Demonstrates: header with public-surface block, SEAM tags throughout,
named property registries, structured `[fn=name]` diagnostic prefix
on `std::cout` calls, K2-style sub-function decomposition, P10
vocabulary block, and the `:dual-algorithm` named locator at module-
internal granularity (registered-behavior dispatch). Reference for:
full-file shape, applying the conventions together rather than
piecemeal.

### `apply_mood` (in `mood.inl`) — K2-style decomposition exemplar

A 217-line function decomposed into five named sub-functions where
the orchestrator owns only ordering. The structure:

```cpp
void apply_mood(uint32_t mood, wgpu::Queue& queue) {
    // ... activate-mood bookkeeping ...
    apply_mood_lighting(m, queue);
    apply_mood_spot_lights(m, queue);
    apply_mood_indoor_shell(m, queue);
    apply_mood_band_motion();
    reset_musical_couplings(queue);
    apply_mood_anchor_ribbon(mood, queue);
    configure_orbs(ORB_MOOD_TABLE[mood], queue);
}
```

The orchestrator reads as a recipe — each line is one named concern.
Reference for: any function that's grown beyond the point where its
top-level shape is hard to scan. Don't split mechanically into "lines
1-50, lines 50-100"; split by named sub-step.

### `gallery.inl` — `:dual-role` and `:complete-subsystem` exemplar

A bespoke subsystem where the file owns its full lifecycle (vocabulary
+ state + lifecycle + dispatch) and where the module deliberately
carries two related concerns — painting-on-terrain (outdoor) and
painting-on-wall (indoor) — sharing infrastructure but spawning
divergently. The TUNING CONSOLE structure with multiple named
sub-configs (`PhotographerCaptureConfig`, `GalleryConfig`,
`WallArtConfig`, plus four property index registries) is also
exemplary. Reference for: bespoke subsystems with shared-but-
divergent concerns, and TUNING CONSOLE shape.

### `ground_architecture.inl` + `world.wgsl` §3.4 — paired CPU+GPU documentation

A foundation library module documenting the ground-architecture system
on the CPU side: ContributorId / PolicyId / CONTRIBUTOR_DAG / POLICIES[]
bitmasks plus compile-time DAG closure validation
(`ASSERT_POLICY_DAG_CLOSED` macro pattern). Its WGSL counterpart is
`world.wgsl` §3.4, which documents the same system on the GPU side:
`contrib_*_at` functions, `POLICY_*_MASK` constants matching the CPU
bitmasks, and `query_ground_<policy>` dispatch. Together they form a
**paired documentation pattern**: same vocabulary, same extension
steps, complementary content. Reference for: foundational library
modules, compile-time invariant assertions over a registry table,
and CPU/GPU paired documentation of a system that genuinely spans
both sides.

The `ASSERT_*_CLOSED` macro pattern deserves a specific note: a
member constexpr can't be invoked from a class-body `static_assert`
in C++. The macro is the principled workaround — it generates a
free function that performs the check, asserted at the file's top
level. The constraint is documented inline at the macro definition.

### `cpu_gpu_pair_manifest.md` — CPU/GPU struct pair traceability

A manifest documenting every `GPU*` struct in `state.hpp` paired
with its WGSL counterpart in `world.wgsl`. 49 entries across frame-
level state, terrain, lighting, agents, pawn, ribbon, GoL, floaters,
per-family entities, portals, and photographer. Each row carries
file/line locations and current sizes. Two CPU-only structs are
flagged as vestigial.

The discipline:

1. **Every GPU struct has a `static_assert(sizeof(...) == N)`**
   on the CPU side. Catches accidental size changes during edits.
   48 of 49 are direct asserts; `GPUPortalEntry` is verified
   indirectly via `GPUPortalArray`'s assertion on
   `16 + MAX_GPU_PORTALS * 32`.

2. **High-traffic pairs carry reciprocal "MUST match"
   comments** on both sides. The 6 per-family `*MeshParams`
   structs (touched whenever an entity family evolves) are the
   demonstrated exemplars; the rest of the surface uses the
   manifest as cross-reference.

3. **Static_assert messages mention the WGSL counterpart
   explicitly**: `"GPUBladeClusterMeshParams must be 80 bytes
   — keep in sync with world.wgsl::BladeClusterMeshParams"`.
   The error message is the moment of friction; making it point
   at WGSL means the developer thinks about pairing right when
   they're about to break it.

4. **A drift sweep recipe** in the manifest (Python script) verifies
   structural alignment programmatically: every CPU struct has an
   assertion, every CPU struct has a WGSL pair (or is on the
   vestigial list), naming asymmetries are documented.

What this discipline catches:
- Size drift (CPU side at compile time, runtime via Dawn).
- Missing CPU/WGSL pair (drift sweep).
- Missing static_assert (drift sweep).

What this discipline does *not* catch:
- Layout drift (same total size, different field offsets).
- Semantic drift (same layout, field meaning changed on one side).

For these, the reciprocal comment blocks plus the textual
parallelism of field declarations are the mitigation. Code review
remains the last line.

This is the productive use of "compile-time CPU/GPU assertions" —
not because the assertions enforce everything, but because they
*surface the link* at the edit site. Combined with the manifest as
the canonical cross-reference, the pairing becomes traceable
without being onerous.

### Diagnostic prefix discipline (cross-module)

The `[Module]` or `[fn=name]` prefix on `std::cout` calls is
de-facto convention rather than belonging to one exemplar. Files
that demonstrate it consistently:

- `cube_behaviors.inl` — `[fn=name]` form
- `gallery.inl` — `[Photographer]`, `[Gallery]`, `[Authored]`,
  `[WallPainting]`
- `mood.inl` — `[Lighting]`, `[Mood]`, `[Shell]`, `[Portal]`,
  `[World]`
- Many others.

The principle: every diagnostic line carries a structured prefix
identifying its source. Free-form chains (no prefix, raw text)
are the outlier; the convention is to prefix.

### Smaller-scope exemplars

- `seed_utils.inl` for P9 (library without state).
- `trajectory.inl` for `:contract` and `:foundations`.
- `entities.inl` for P10's canonical home.
- `orbs.inl` for P1 (canonical instance) and P3 (canonical
  player-vs-mood split).
- `cartridge.hpp`'s FAMILY_DISPATCH wrappers for P12 (canonical
  integration-glue instance).

---

## 12. Migration methodology lessons

This section captures procedural lessons from the Scope B per-module
state-struct rollout (cumulative migrations #1-#11, May 2026). These
are not in-source patterns — they're pitfalls and disciplines for
*how to perform* a multi-file rename/restructure safely. They live
here because future migrations of similar shape are likely, and the
failure modes are not obvious until they bite.

### M1 — sed scope creep

When mass-renaming a field with a regex like `\bX\b → struct.X`, the
substitution can match identifiers beyond the intended target if the
chosen pattern overlaps with **method names** that share a prefix.

**Concrete failure.** A migration of `gallery.inl` ran
`sed s/\bphotographer_/gs.photographer/g` to rename the
`photographer_` field. The pattern also matched method names
`photographer_compute_group()` and `photographer_render_entity_group()`
(both methods of `GPUState`), corrupting them to
`gs.photographercompute_group()` and `gs.photographerrender_entity_group()`
— eight call sites that compiled fine after a first verification grep
(because the *method* names matched the substitution pattern at a word
boundary on both ends).

**Mitigation.** When a renamed field shares a prefix with method names
that contain the field's identifier as a substring, either:
- Use a tighter regex (e.g. `\bphotographer_\s*=` for the declaration
  + `\bphotographer_\s*[\.;\)]` for usages), or
- Split the rename into two passes: (a) declaration, (b) usages by
  exact suffix shape, leaving method names untouched.

**Detection grep.** After a sed pass, scan for malformed substitutions
of shape `struct\.field[a-z]+`. This catches field-prefix-eaten-method-
name patterns.

### M2 — stateless helper misclassification

A function whose **core behavior** transforms its parameters but which
has **peripheral side-effects** on module state will compile cleanly
when classified as stateless and converted to a pure helper — and then
fail at runtime, or fail when a later refactor exposes the side effect.

**Concrete failure.** During the orbs migration, `pack_palette_` was
classified as stateless (its core job is to populate a `GPUOrbConfig`
from a palette ID). Missed: it also writes `os.current_palette_id =
pal_id` as a peripheral side-effect. The `pack_flocking_` function
similarly *reads* `os.gesture_idx[]` and `os.speed_mult_current`
without these being part of its core signature. Both functions
compiled when re-declared without `OrbsState&`, and the side-effect
references became "undeclared identifier" errors.

**Mitigation.** Default classification: **stateful**. Let the compiler
flag unused parameters via warning if the function doesn't actually
use the state. Do not classify stateless until a clean compile with
state passed in shows the parameter is genuinely unreferenced.

### M3 — Forward-declaration ordering for state structs

When a state struct contains arrays of types that are themselves
defined throughout the file (per-family `Active*` types interleaved
with vocabulary blocks), the state struct must be defined at the
**file's end** (after all its dependencies), and any module functions
taking it as a parameter must follow it.

**Concrete failure.** Migration #9 introduced `EntitiesState` containing
arrays of `ActiveArch`, `ActiveColumn`, `ActivePalm`, `ActiveCactus`,
`ActiveBlade`, `ActivePyramid`. Three `prepare_*_mesh_gen` functions
were left interleaved with their family vocab blocks (lines 410, 488,
562); each of these functions signatures named `EntitiesState&` as a
parameter type. `EntitiesState` was defined at line ~620, **after**
the functions referencing it — yielding `error C2061: syntax error:
identifier 'EntitiesState'` and a 30-error cascade.

**Mitigation.** Establish a file-end discipline:
- All `Active*` family types where they are
- `XState` struct at end of file
- `XState x_state_;` immediately after
- All static functions consuming `XState&` at the very end, after the
  state struct

**Detection.** Before declaring a migration done, view the file's
linear structure: every reference to the state struct should appear
*after* the struct's definition.

### M4 — Audit-closure sync gaps

When a previous architectural audit modifies **both** a struct
definition and its consumers, a partial sync (consumer file syncs
but struct file does not) stays silent until init lists or member
accesses force resolution. The build can compile fine for many
unrelated changes before the gap surfaces.

**Concrete failure.** During migration #9 verification, the build
reported `apply_indoor_rescale: is not a member of EntityFamilyAdapter`
plus init-list shape mismatches across 9 adapter declarations. The
local `entity_types.inl` was missing the `apply_indoor_rescale` slot
that an earlier audit had added. The corresponding consumer code
(`entity_pipeline.inl` calling `adapter.apply_indoor_rescale(...)`)
had been synced, but the struct definition file had not. The shape
of the error perfectly matched "struct missing one slot, init lists
shifting by one" — which would have been the diagnostic shortcut had
the gap been considered as a hypothesis from the start.

**Mitigation.** When an audit closure modifies multiple files, treat
the file-set atomically: produce a closure manifest naming every
file in the change, and verify each appears in the next sync. The
glossary's section on rollout output rosters is appropriate here.

**Detection shortcut.** Init-list shape errors that "shift by one
slot" almost always indicate a struct definition out of sync with its
init-list users. The error pattern: slot N gets a value with the
signature of slot N+1.

---

## Version

**v1.6.** Captures lessons from the Scope B per-module state-struct
rollout (cumulative migrations #1-#11). One pattern addition; one new
section.

- **P13 added — Leaf module vs orchestration module.** A taxonomic
  distinction governing where module state lives. Leaf modules (called
  by spine, never call back into other module surfaces) own their state
  inside their `.inl`. Orchestration modules (call into many other
  module surfaces) must remain spine-resident because their `#include`
  comes last in the class body. Discovery: `mood.inl` cannot move
  `MoodState` into itself because static functions in earlier-included
  modules (`gol_zones`, `gallery`, `spawn_engine`, `entity_pipeline`)
  reference `c->mood_state_` and would lose visibility. This is a
  **structural** distinction, not a stylistic one — it follows from
  the dependency direction.
- **Section 12 added — Migration methodology lessons.** Four procedural
  pitfalls captured (M1-M4): sed scope creep, stateless helper
  misclassification, forward-declaration ordering for state structs
  whose fields depend on family types, and audit-closure sync gaps.
  These are not in-source patterns but pre-conditions on safely
  performing migrations of the shape this rollout used.
- **No retirements.** P1-P12 unchanged.

The cumulative state of the codebase after the rollout: **12 named
state containers** organize what used to be ~120 raw fields scattered
across the Cartridge class. Nine module-state structs (one per leaf
module) plus four spine-state structs (`TimeState`, `PlayerState`,
`MoodState`, `WorldState`). Every module function takes its state
explicitly via parameter. SEAM[spine:P8] retired (`aura_presence` +
`mmode_intensities` migrated from `pawn_state_` / `musical_state_`
into `PlayerState` — they now travel with the player, not the body
or the world).

---

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

---

**v1.4.** Folds in the post-rollout audit findings: one new pattern,
one new top-level section, six new convention sub-sections in §9.

The rollout (cartridge spine + WGSL spine + 19 modules) generated 17
organization-guide-correction items. v1.4 lands them all in one
revision so future audits start from a glossary that includes the
current state of the conventions.

Changes:

- **New pattern P12** — Integration glue: per-family wrappers binding
  modules to a dispatch table. Earned by recurrence: `cartridge.hpp`'s
  FAMILY_DISPATCH wrappers (~400 lines) are the canonical instance,
  with `entity_pipeline.inl`'s per-family dispatch adapters as the
  module-level sibling. P12 was previously tagged with the bespoke
  `:K2-related` locator; promoted because the shape is real and
  repeatable. `:K2-related` is retained as a historical bespoke
  locator.
- **New §11 Reference exemplars** — A curated list of files that
  demonstrate the conventions cleanly. Folds in observations from
  five rollout passes (cube_behaviors as full-conventions exemplar,
  apply_mood as K2-split exemplar, gallery as `:dual-role` exemplar,
  ground_architecture + world.wgsl §3.4 as paired CPU+GPU exemplar,
  diagnostic prefix discipline as cross-module convention).
- **§9 Naming conventions for tunable constants** — Distinguishes
  `*_GAIN` (multiplier on base) from `*_CEILING` (absolute bound),
  with the residency rule that gain constants live with the
  *coupling that drives them*, not the parameter they scale. Five
  known instances of the gain-vs-ceiling drift identified during
  the rollout.
- **§9 Stripping authoring artifacts** — Sweep recipe for the
  recurring paper-trail phrases (Phase N, Step N, Ch.N chunk M,
  (was X), (legacy X removed), NEW FINDING, First X migrated,
  Renamed from). Several subspecies were named during the rollout;
  v1.4 collapses them into one cleanup discipline with a grep recipe.
- **§9 Cross-file tag declarations** — Convention rule: when other
  files reference your tags, your file must declare them at the
  load-bearing site. Compound forms like `DONE[musical:K2 / mood:K3]`
  must be split. The rule was implicit in the rollout; making it
  explicit means future audits can grep for dangling references and
  fix them mechanically.
- **§9 Indentation policy for source comments** — Un-indent if the
  file is currently class-body-indented; leave at column 0 if
  already there. Small but explicit so future audits don't run an
  un-indent script that corrupts pre-aligned files.
- **§9 Section numbering in WGSL files** — Section numbers reflect
  file position, not topical grouping. If a topical sibling needs
  to live elsewhere for binding-locality reasons, give it the
  destination's positional number with a NOTE explaining the
  placement. Surfaced during the world.wgsl audit (§7.5 GPU Frustum
  Culling was numbered §7.5 but located inside §8; fixed by
  renumbering to §8.0.5).
- **§9 Lineage notes within a P10 host** — When a module hosts
  multiple instances of P10 and some are genealogically related
  (sibling pairs, design cell divisions), the relationships are
  captured in a "Lineage" paragraph in each family banner.
  Convention applied in entities.inl (Column ↔ Antenna explicit
  sibling pair, Palm/Cactus/Blade vegetation cluster).
- **§9 Input-vocabulary fluidity** — Framing: input bindings are
  fluid scaffolding, the cartridge sits below an analysis layer
  that consumes A–Z as MIDI piano notes, diagnostic toggles live
  on function keys/numpad/Caps Lock because A–Z are claimed.
  The framing belongs in input.inl's header.

P12 added to the Quick reference (§10) table.

The pattern definitions, locator meanings, and existing conventions
from v1.3 are unchanged. v1.4 *adds* — it doesn't revise.

---

**v1.5.** Adds the CPU/GPU pair manifest exemplar to §11.

The post-modularity-audit work surfaced an important discovery:
48 of 49 GPU structs in state.hpp already had `static_assert(sizeof(...))`
guards on the CPU side, and the 49th (`GPUPortalEntry`) was protected
indirectly through its enclosing array's assertion. The byte-level
enforcement was already in place.

What was missing was the **link between the CPU and WGSL definitions**.
v1.5 adds an exemplar entry for `cpu_gpu_pair_manifest.md`, which
catalogs all 49 pairs and documents the cross-reference discipline:

- Every GPU struct has its `static_assert` mention the WGSL pair
  explicitly in the failure message.
- High-traffic pairs (the 6 per-family `*MeshParams`) carry
  reciprocal "MUST match" comments on both sides.
- A drift sweep recipe in the manifest verifies alignment
  programmatically.

The pattern is *not* a new pattern (P12 is still the latest), and
the conventions are unchanged. The exemplar names a specific
discipline that complements `ground_architecture.inl + world.wgsl §3.4`
— that exemplar is the in-source paired-documentation case;
the CPU/GPU pair manifest is the catalog-level case for the broader
struct surface.

No glossary structure changes; no pattern additions. v1.5 is a §11
exemplar update.
