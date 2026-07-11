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

## LADDER-1 — CLOSE-OUT

Three commits (c1 `876b32a`, c2 `6efdeee`, c3 `38efdc5`) on MOD_1_ROSTER,
stacked on ROSTER-1b, never force-pushed. One build after c3 (L0).

**Bytes report** (per file created/touched; the ladder's own headers match
roster.hpp's no-BOM/LF precedent; `entity_pipeline.inl` keeps its BOM):

| file | first-3 bytes | EOL | note |
|---|---|---|---|
| modules/seed_utils.hpp | `23 70 72` (no-BOM) | LF | created (c1) |
| modules/ground_architecture.hpp | `23 70 72` (no-BOM) | LF | created (c2) |
| modules/entity_types.hpp | `23 70 72` (no-BOM) | LF | created (c3) |
| audit/LADDER.md | `23 20 54` (no-BOM) | LF | created (c1) |
| cartridge.hpp | `23 70 72` (no-BOM) | LF | touched (all) |
| cartridge_constitution.md | `23 20 54` (no-BOM) | LF | touched (c1–c3) |
| modules/spawn_engine.inl | `2f 2f 20` (no-BOM) | LF | touched (c3) |
| modules/entity_pipeline.inl | `ef bb bf` (BOM) | LF | touched (c3), BOM preserved |
| modules/{seed_utils,ground_architecture,entity_types}.inl | — | — | DELETED (relocated) |

**Census (with recipes):**
- Standalone compile probes: 3/3 PASS (self-containedness — the
  boundary-honesty test). Recipe: throwaway TU `#include`-ing only each
  header; `g++ -std=c++20 -I.`.
- Call-site census: seed_utils 241 calls / 0 qualified; ground_architecture
  0 runtime C++ consumers (source-of-truth mirrored by world.wgsl);
  entity_types 0 qualified. Recipe: grep `Cartridge::<sym>` / `this-><sym>`
  across the_board. Zero qualified forms anywhere → all call sites carry.
- Old-path census: zero `.inl` INCLUDE references to the three in the_board.
  Recipe: grep `#include "modules/<name>.inl"`.
- Unused-symbol census: none — every converted symbol has a live consumer
  (or, for ground_architecture, is a compile-time-validated + GPU-mirrored
  contract). Zero STATUS tags needed; flag-don't-delete not triggered.
- Braces: cartridge.hpp 652/652, spawn_engine.inl 131/131,
  entity_pipeline.inl 702/702, and the three new headers balanced.

**SEAM ledger:** SEAM[seed_utils:P9] and SEAM[ground_architecture:P9]
updated to their extracted state; SEAM[spawn_engine:structural] RETIRED
(all four mentions carry the retirement reason); the FXC/mirror contract
seams (seed_utils:contract, ground_architecture:contract) preserved
verbatim; world.wgsl untouched.

**DISCLOSED (deferred, not silent):** "Depends on: seed_utils.inl" comment
notes still stand in four unconverted modules (entities, gol_zones,
gallery, ribbon) and world.wgsl's mirror note names the old `.inl`.
world.wgsl is untouchable (scope guard / R5); the module notes update as
each converts. entity_pipeline.inl's and spawn_engine.inl's notes were
updated in c3 (they named the retired mechanism / converted deps directly).

## LADDER-2 — L-NEXT (in progress)

Stacks on LADDER-1. Two of Jean's rulings shaped it:
1. **M-c** (per-species floater ownership) — the state splits to owners, the
   types stay shared vocabulary, "floater" gets no CPU module.
2. **Conversion model** for stateful modules whose laws dereference the
   complete Cartridge → **header/impl split**: the header (state struct +
   configs + declarations) sits above the class so the instance member
   works; the definitions sit in a `.inl` included AFTER the class, in a new
   `MODULE IMPLEMENTATIONS (post-class)` zone, where the keyhole is complete.
   Still one TU. A stateful module's `.inl` is REPURPOSED as that impl, not
   deleted (unlike LADDER-1's pure-header leaves).

   **THE ACCESS COROLLARY (rig-found, now part of the pattern's
   definition).** Post-class definitions see the complete type but stand
   OUTSIDE the class — membership-exemption is gone, so private organs deny
   the keyhole itself (the rig's C2248 at pawn.inl:22, MSVC). Jean's
   ruling: THE ORGANS GO PUBLIC — one `public:` at the head of the organ
   region, banner stating the law (sight free; writes through declared
   seams; the census enforces the seam law, access control never did; the
   outside enters only through the RenderCartridge interface + public
   lifecycle). Friend-registry was priced (per-function ceremony scaling
   with the whole ladder; signature drift as a new failure mode; a friend
   sees all privates anyway) and declined. Residuals that stay private:
   the 4132–4137 stretch (input.inl's region) and mood.inl's internal
   `private:` stretch (its own authoring palettes) — no post-class
   consumer needs either; census re-checks at each conversion.
   **Harness corollary upgrade:** the pattern probe's mock is now a
   `class` mirroring the REAL access topology. Recorded runs — pre-fix
   topology (organs private): FAILS at pawn.inl:22 "player_ is private
   within this context" (the rig's exact error, reproduced); post-fix
   topology (public organ region): compiles, links, runs. The access
   dimension is tested from now on.

   **THE WIRING COROLLARY (rig-found, fix 2 — the unresolved conductor).**
   Impl files are SELF-WRAPPING (each opens `t7::the_board` itself and
   carries its own standard includes), so the MODULE IMPLEMENTATIONS zone
   sits at FILE SCOPE, after both namespace closes. Including a
   self-wrapping impl INSIDE the namespaces double-wraps its symbols into
   `t7::the_board::t7::the_board::` — a legal nested namespace that
   compiles silently and satisfies nothing (the rig's LNK2019: unresolved
   `tick_pawn_couplings`, call side correct by the mangling). The audit
   ran the ranked three: (a) `static` survivals — none in pawn.inl;
   (b) signature parity — exact; (c) wiring — CONFIRMED (the cause).
   Rule recorded into the pattern's definition: **impl-file definitions
   are `inline` free functions; the class-body `static` never survives
   the move** (inline landed with this fix — one TU makes plain legal,
   inline makes tomorrow legal too).

   **PROBE THE ARTIFACTS, NOT A RE-CREATION.** Two rig failures, one
   root: the probes validated hand-written portraits (first
   implicitly-public organs; then a correct include topology the real
   tree didn't have — the include-site topology IS part of the
   artifact). The probe is now a SPECIMEN: it compiles and links the
   REAL pawn.hpp + REAL pawn.inl around a mock Cartridge supplying
   exactly the members pawn reaches, replicating the tree's include
   wiring. Recorded runs — pre-fix (real files, the double-wrap wiring):
   LINK FAILS, "undefined reference to t7::the_board::
   tick_pawn_couplings(...)" referenced from Cartridge::update — the
   rig's exact missing symbol; post-fix (real files, file-scope
   include): compiles, links, runs. All future conversion stages
   (c1/c3/c4 and every L-mid rung) use specimen probes.

A prereq was also required and ruled: **MOOD_COUNT graduated to file scope**
(mood_constants.hpp) — the config-bearing modules size per-mood tables by it
and a file-scope header can't see an in-class constant.

| stage | module | status | notes |
|---|---|---|---|
| prereq | MOOD_COUNT | ✅ LANDED | graduated to mood_constants.hpp (ab8e79c) |
| c0 | **M-c floater split** | ✅ LANDED | spheres.hpp (SphereState, born converted) + cube→CubeBehaviorsState + floater_vocabulary.hpp (types); owner clears; §5 slivers (d56347d) |
| c2 | **pawn** | ✅ LANDED | header/impl split — pattern established (86190b5); the two access/wiring corollaries were rig-found on this head and fixed (16f836e, b0a0095); rebuild GREEN |
| c1 | **entities** | ✅ LANDED | D3 suspect FALSIFIED — zero retrofit; PortalDestination graduated to mood_constants.hpp; PAWN_HEIGHT_UNITS tagged LATENT[unused] (74b3d7f) |
| c3 | **orbs** | ✅ LANDED | near-verbatim; ORB_MOOD_TABLE to header; ORB-1 anchor semantics untouched (b67983c) |
| c4 | **floater_vocabulary** | ✅ LANDED | configs/registries joined the .hpp; .inl deleted; the banner tells the truth (c704b4a) |

**L-NEXT IS CONVERTED.** One rig build (L1) certifies the arc: same seed,
two runs, pixel-stable, indistinguishable from the LADDER-1 head.

### LADDER-2 close-out (census with recipes)

**The two mid-arc rulings** (recon-surfaced, Jean-decided): MOOD_COUNT
graduates first (the config-bearing headers size per-mood tables by it);
conversion model = header/impl split. A third small graduation followed
the same second-consumer law at c1: PortalDestination (embedded in
ActiveArch) → mood_constants.hpp.

**D3 verdict (the arc's headline):** the retrofit suspect was FALSIFIED.
Every stateful module's functions already took (State&, Cartridge*, ...)
explicitly — entities' 6 preparers, orbs' 11 public functions, pawn's
tick. ZERO signatures changed across the whole arc; every call site
carried on unqualified lookup (R4 recipe: grep `Cartridge::<sym>` /
`this-><sym>` per moved symbol — zero hits, all stages).

**Specimen probes (all real artifacts, real include wiring):**
- c1 entities: state.hpp shadowed by an environment stub (Dawn absent in
  the probe env; state.hpp itself is rig-proven) — preparer scans slots,
  uploads (max+1)×stride via the keyhole. PASS.
- c3 orbs: environment mocked to the exact reached surface — configure
  packs/uploads/arms; init one-shots; anchor pushes pawn coords (ORB-1
  verified unchanged); rule cycles. PASS.
- c4 floater_vocabulary: pure header, real deps only — probed with NO
  mocks. PASS.

**Bytes report** (created/touched this arc; all new headers no-BOM/LF):
mood_constants.hpp, spheres.hpp, floater_vocabulary.hpp, pawn.hpp,
entities.hpp, orbs.hpp `23 70 72`/LF; pawn.inl, entities.inl, orbs.inl,
cube_behaviors.inl, input.inl, spawn_engine.inl `2f 2f 20`/LF;
entity_pipeline.inl + render_passes.inl BOM (`ef bb bf`) PRESERVED;
cartridge.hpp no-BOM/LF, code-only braces 643/643;
floater_vocabulary.inl DELETED (c4).

**Unused-symbol census:** PAWN_HEIGHT_UNITS (entities.hpp) — zero
callers; STATUS: LATENT[unused], kept per flag-don't-delete. Nothing else
surfaced.

**The composition root now holds:** sphere_state_, pawn_state_,
entities_state_, orbs_state_ (converted); cube_behaviors_state_ remains
in cube_behaviors.inl (unconverted regime, converts in L-mid). The
MODULE IMPLEMENTATIONS zone holds pawn.inl, entities.inl, orbs.inl.

**LADDER-1 findings consumed by this arc** (one line each): the
header/impl access corollary (organs public) and the wiring corollary
(self-wrapping impl → file-scope zone) became the pattern's definition;
the specimen rule replaced portrait probes; roster.hpp's namespace
precedent (t7::the_board) held for every new header; the encoding law
(match tree reality per file) held — two BOM files preserved.

## L-MID (queued — next rungs)

cube_behaviors (its state now carries the cube active array; converts
whole), gol_zones, gallery, ribbon, agents, input, render_passes,
entity_pipeline, spawn_engine, ground/terrain spine chapters. **K4 —
mood's conversion shape — is the next MARINATION ITEM for Jean's queue:**
mood.inl is the largest unconverted module with the deepest spine
entanglement (transition machine, portal force-spawn, apply_mood fan-out,
its own internal private palettes); its conversion shape (one module vs
transition-machine split; what K4's request channel changes) wants a
ruling before the rung is cut.

## L-NEXT-NEXT (preview only — no action; design ruling for Jean's queue)

The leaves done, L-next continues the ladder. **`floater_vocabulary`
carries the M-c precondition**: the live floater state needs an OWNER
module first (the vocabulary can't convert cleanly while the mutable
floater state it reads has no home). That is a design ruling for Jean, not
a CC task — flagged here so the ladder's next rung isn't taken blind.
