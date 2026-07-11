# THE HEADER LADDER — record

the_board only. Branch **MOD_1_ROSTER**, stacked on ROSTER-1b. The ladder
converts each class-body-include module into a real file-scope header,
one per commit, **behavior-identical** (screen-gated, not binary — moving
symbols from class scope to namespace scope legitimately changes the
binary). Constitution §1 was amended (2026-07-11) from the single-organism
law to the composition law, under a two-regime transitional clause.

## GATE CALIBRATION (read this before diffing a ladder commit)

- **Roster stages** promised **byte-identical binaries** (all-enabled).
- **Conversion stages** promise **BEHAVIOR-identical** — the gate is the
  SCREEN: same seed, two runs, pixel-stable, indistinguishable from the
  pre-ladder head. Nobody chases a binary-hash diff on these commits.

## THE RECIPE (per module — LADDER-1 §2)

R1 create `modules/<name>.hpp` (`#pragma once`, banner preserved with the
identity line updated, namespace `t7::the_board`, body verbatim minus
class-body-isms: `static` member fns → `inline` namespace fns, static
constexpr member tables → `inline constexpr`). R2 include at file scope
with roster.hpp's cohort, before the class. R3 delete the class-body
include; remove the moved-out `.inl` same-commit (relocation, git carries
history, no tombstones). R4 call-site census (unqualified lookup carries;
fix any `Cartridge::` / `this->`). R5 preserve twin/mirror notes (path
updates only; world.wgsl untouched). R6 SEAM updates. R7 per-commit: brace
+ encoding gates, a standalone compile probe of the new header, a grep
census proving zero remaining `.inl` **include** references, an
unused-symbol census as STATUS tags + a ledger line.

## STAGES

| stage | module | status | notes |
|---|---|---|---|
| c1 | **seed_utils** | ✅ LANDED | the self-nominated easiest leaf; the §1 amendment rides this commit |
| c2 | **ground_architecture** | ✅ LANDED | tables + asserts move whole; IIFE static_assert idiom UNTOUCHED (restyle is a named later stage) |
| c3 | **entity_types** | ✅ LANDED | + spawn_engine mid-file include removal + `SEAM[spawn_engine:structural]` retirement |

Jean builds ONCE after c3 (L0 LADDER GOLDEN certifies all three at once;
bisection exists if it ever fails).

### c1 — seed_utils (LANDED)

- `modules/seed_utils.hpp` created — 8 pure functions (`cpu_hash`,
  `cpu_hash_f`, `tile_seed`, `cpu_lattice_node_seed`, `cpu_smoothstep`,
  `cpu_sample_gaussian`, `select_weighted`, `select_tier`), `static` →
  `inline`, namespace `t7::the_board`. Deps: `<cstdint> <algorithm>
  <cmath>` (self-contained). Standalone compile probe: PASS.
- Call-site census: **241 calls, all unqualified, 0 qualified forms** — all
  carry unchanged. No shadowing definitions.
- SEAM[seed_utils:P9] updated to the extracted state; SEAM[seed_utils:
  contract] (FXC twins) + SEAM[seed_utils:Q10-target] preserved verbatim.
- The §1 amendment + banner two-regime note + a §5 ledger line rode this
  commit.
- Unused-symbol census: none — all 8 symbols have live callers (counts
  above). Zero STATUS tags needed for c1.
- DISCLOSED: dependency-note comments elsewhere ("Depends on:
  seed_utils.inl" in unconverted modules, and world.wgsl's mirror note)
  still name the old `.inl`. These are documentation, not includes;
  world.wgsl is untouchable (scope guard / R5), so they are left to update
  as each module converts — not chased in c1 (would create a split state).

### c2 — ground_architecture (LANDED)

- `modules/ground_architecture.hpp` created — 2 enums (`ContributorId`,
  `PolicyId`), 2 structs (`ContributorEdge`, `PolicyDef`), the tables
  (`CONTRIBUTOR_DAG`, `POLICIES`, `GROUND_STATIC_BASE_MASK`, the counts)
  `static constexpr` → `inline constexpr`, and the 10 compile-time
  DAG-closure `static_assert`s via the `ASSERT_POLICY_DAG_CLOSED` macro.
  Namespace `t7::the_board`. Dep: `<cstdint>`. Standalone compile probe:
  PASS — the DAG-closure asserts hold at namespace scope.
- The IIFE (immediately-invoked-lambda) static_assert idiom is UNTOUCHED;
  the comment now notes the class-body constraint that forced it has
  dissolved at file scope, and the restyle to a namespace constexpr
  function is a NAMED LATER STAGE (clean bisection).
- Call-site census: the enums/tables have **zero runtime C++ consumers** —
  they are a compile-time-validated source-of-truth that only world.wgsl
  mirrors (the 4 files matching `POLICY_WALKER` etc. do so in comments).
  Nothing to carry; 0 qualified forms.
- SEAM[ground_architecture:P9] updated (header-style nature now realized);
  SEAM[ground_architecture:contract] (world.wgsl mirror) preserved
  verbatim; world.wgsl untouched.
- Unused-symbol census: the tables are consumed only at compile time +
  by the GPU mirror; that IS their contract (the banner states "no runtime
  symbols exported"). Not unused — validated-and-mirrored. Zero STATUS
  tags needed.

### c3 — entity_types (LANDED)

- `modules/entity_types.hpp` created — pipeline-contract constants
  (`MAX_ENTITY_PARAMS`/`MAX_COLOR_CHANNELS` static constexpr → inline
  constexpr), `enum class ParamDist`, and the structs (`TierParamDef`,
  `TierMuSigma`, `TierProfile`, `ColorPartDef`, `EntityFamilyTraits`,
  `SpawnGateOutput`, `EntityInstance`, `EntityFamilyAdapter`). Namespace
  `t7::the_board`. Self-contained: dep `<cstdint>` plus **forward
  declarations** of `Cartridge` (adapter fn-ptrs take `Cartridge*`) and
  `wgpu::Queue` (taken by reference in two fn-ptrs) — both used only as
  pointer/reference in typedefs, so forward decls suffice and make every
  dependency explicit at the boundary (§1). Standalone compile probe:
  PASS (no wgpu build artifact needed).
- **THE STRUCTURAL SEAM RETIREMENT** (R3): the mid-file
  `#include` of entity_types in `spawn_engine.inl` is deleted;
  `SEAM[spawn_engine:structural]` is retired in place (all 4 mentions now
  read the retirement) — entity_types precedes the `EntityQueueEntry`
  union by construction at file scope, so the union-member-ordering
  constraint is satisfied without the mid-file include. The "keep one
  file" ruling is honored (spawn_engine was never split). entity_types'
  banner drops the mid-file caveat.
- Call-site census: 0 qualified forms; state.hpp/renderer.hpp don't use
  the types (cohort order-free). R5: no GPU twin (types are CPU-only;
  world.wgsl mirrors are per-struct byte contracts noted at those
  structs, unchanged). Stale union-ordering + dependency comments in
  `entity_pipeline.inl` updated same-commit (they named the retired
  mechanism / the converted deps).
- R7: zero remaining `entity_types.inl` path references in the_board
  (grep-clean). Unused-symbol census: none — every type has a live
  consumer in entity_pipeline.inl / spawn_engine.inl.

**THE THREE LEAVES ARE CONVERTED.** L0 (Jean, one build) certifies all
three at once.

## L-NEXT (preview only — no action; design ruling for Jean's queue)

The leaves done, L-next continues the ladder. **`floater_vocabulary`
carries the M-c precondition**: the live floater state needs an OWNER
module first (the vocabulary can't convert cleanly while the mutable
floater state it reads has no home). That is a design ruling for Jean, not
a CC task — flagged here so the ladder's next rung isn't taken blind.
