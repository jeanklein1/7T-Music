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

## LADDER-3 — L-MID: SEVEN RUNGS, TWO GRADUATIONS (converted; mood held)

Order as ratified: c1 gol_zones → c2 agents (+G1) → c3 cube_behaviors →
c4 gallery → c5 ribbon → c6 input (+G2) → c7 render_passes. Mood
EXCLUDED (K4 held for Jean). One mid-arc defect found and fixed by the
c4 pre-flight (below).

| rung | module | status | note (commit) |
|---|---|---|---|
| c1 | **gol_zones** | ✅ LANDED | near-verbatim; GoLSelection/GoLPlacement came home from spawn_engine (038c30c) |
| c2 | **agents** | ✅ LANDED | + G1: the six Mood IDs → mood_constants.hpp; BOM preserved, LF (the CRLF half of the spec's claim didn't hold) (459253d) |
| c3 | **cube_behaviors** | ✅ LANDED | G1's second consumer; carries c0's cube active array (4f8b5dd) |
| — | **wiring fix** | ✅ LANDED | c2/c3 impls were self-wrapped but NEVER ADDED to the zone — the fix-2 LNK2019 shape reproduced by omission; caught by the c4 pre-flight re-read of the zone tail, before any rig build ran over it; + repaired c1's truncated zone comment/newline (5862f3c) |
| c4 | **gallery** | ✅ LANDED | largest module; GallerySelection/GalleryPlacement came home; frame-preset section hoisted + 3 impl-internal fwd decls (namespace scope has no class-body two-pass lookup); 19 bodies verified byte-identical mod transforms (d0b63b6) |
| c5 | **ribbon** | ✅ LANDED | THE PAIRING RULING: byte-identical mirror with the_chord SUSPENDED, named in ribbon.hpp's banner (pawn precedent); RibbonSelection/RibbonPlacement came home — spawn_engine's payload section is now three relocation notes, zero structs; MOUNT_* lockstep mirrors travel intact; BOM preserved, LF (ce92d14) |
| c6 | **input** | ✅ LANDED | + G2: InputState/KeyState/MouseState → input.hpp, instances stay at the root; THE REAL RETROFIT begins — ~41 ambient reads → keyhole, all ten fns gain Cartridge* c, six call sites gain `this`; GLFW dependency UNPAPERED (input.inl includes <GLFW/glfw3.h> itself) (77fc3a3) |
| c7 | **render_passes** | ✅ LANDED | the heavy retrofit — 243 ambient reads → keyhole (mechanical transform, re-verified independently); if-census 19=19, ZERO draw self-gates added; light-VP helpers stay pure (compute_sun_matrices: zero callers, LATENT, travels); BOM preserved (cdc93ca) |

**L-MID IS CONVERTED (minus mood).** The L2 gate — ONE end-build at the
rig, screen-golden — is now due. The disclosed checkpoint option at c6
(77fc3a3) remains a valid bisection point if the end-build goes red.

### LADDER-3 close-out (census with recipes)

**D3 verdict, refined:** FALSIFIED again for the five stateful modules
(c1–c5: every function already took (State&/Cartridge*, ...); zero
signature changes; all callers unqualified) — but c6/c7 were true
member-function modules, and the retrofit recipe ran exactly as priced:
functions gain a leading `Cartridge* c`, ambient reads gain `c->`,
call sites gain `this` (input: 6 sites; render_passes: 6 sites), bodies
otherwise verbatim (verified per-function by normalized byte-compare on
every rung: c4 19/19 fns, c5 15/15 fns + 60 console constants + all
structs, c6 10/10 fns, c7 9/9 fns + gate census).

**Keyhole static form (running total this arc):** PATCH_EXTENT ×13
(gol/gallery/ribbon), PopFamily ×11 (GOL/GALLERY/RIBBON),
GLOBAL_ENTITY_DENSITY ×2, TransitionPhase ×1, THEMES ×1,
GRID_RADIUS/PREGEN_RADIUS ×2 (input).

**Payloads home (the entity_types precedent, completed):** GoL (c1),
Gallery (c4), Ribbon (c5) Selection/Placement pairs all relocated to
their modules' headers; spawn_engine.inl's payload section now carries
three relocation notes and no structs.

**Graduations:** G1 (c2) six Mood IDs; G2 (c6) three input structs.
Both by the second-consumer law; instances at the COMPOSITION ROOT.

**Specimen probes (all real .hpp+.inl, real fix-2 wiring, environment
stubbed):** c1 lattice→commit→derive lifecycle; c2 registry→spawn→
possess→census; c3 corral/kite/coordination/clear; c4 capture cadence →
snapshot render → 3-phase outdoor → evict (center persists) → authored
staging → wall paintings → rotation (exit 0); c5 3-phase spawn →
conductor adoption → altitude bake (ground+clearance exact) → wanderer
motion through the one integrator → succession kill → sky-release drain
(exit 0); c6 full dispatch table (12 sibling commands), fallback key
constants exercised by a sparse shadow GLFW, intent normalization,
radius clamp through Cartridge:: statics (exit 0); c7 all seven entity
families' ground entries, dispatch variants (8/7), frustum cull
outdoor/indoor, shadow outdoor + 3-light indoor atlas, main pass
indirect/direct, finite light-VPs incl. header defaults (exit 0).

**Bytes report:** new headers gol_zones.hpp, gallery.hpp, ribbon.hpp,
input.hpp, render_passes.hpp all no-BOM/LF; agents.inl, ribbon.inl,
render_passes.inl BOM (`ef bb bf`) PRESERVED, LF; gallery.inl,
input.inl, cube_behaviors.inl, spawn_engine.inl no-BOM/LF;
cartridge.hpp no-BOM/LF, code-only braces 640/640 (the G2 struct-def
graduation accounts for exactly −3 pairs from 643).

**The composition root now holds:** sphere_state_,
cube_behaviors_state_, pawn_state_, entities_state_, orbs_state_,
gol_state_, agent_state_, gallery_state_, ribbon_state_, inputState_,
keys_, mouse_. **The MODULE IMPLEMENTATIONS zone holds (in ladder
order):** pawn, entities, orbs, gol_zones, agents, cube_behaviors,
gallery, ribbon, input, render_passes.

**Pattern addendum (the wiring-fix lesson):** the zone edit is PART OF
THE RUNG — a specimen proves the module, not the cartridge's wiring;
the rung recipe now ends with "re-read the zone tail." Second lesson:
namespace scope has no class-body two-pass lookup — before-definition
intra-module calls need either original-order luck (ribbon), a header
declaration (public fns), or an impl-internal forward-declaration block
(gallery).

**STANDING INVARIANT — THE ZONE CENSUS (LADDER-3 rider; closes the
omission class):** the specimen wires its own zone, so it can never see
the real zone's omissions. Every rung's verification therefore includes,
BESIDE the specimen probe, this mechanical cross-check (no compile
needed):

    set(post-class impl files in modules/)
      == set(includes in the MODULE IMPLEMENTATIONS zone),
    each exactly once.

Discriminator: a post-class impl is SELF-WRAPPING (opens `namespace
t7 {` itself); a class-body .inl is not — so the census also checks the
converse for free (each class-body .inl included exactly once, inside
the class, never in the zone; no duplicates on either side).

Retroactive run over the LADDER-3 head (08e565c): **CLEAN** — 10
self-wrapping impls == 10 zone includes (pawn, entities, orbs,
gol_zones, agents, cube_behaviors, gallery, ribbon, input,
render_passes), each exactly once; 3 class-body files (spawn_engine,
entity_pipeline, mood) each exactly once, in-class.

**RATIFICATION (LADDER-3 rider):** the arc is RATIFIED as committed,
wiring fix included. Explicitly ratified as disclosed judgment calls:
the spawn_engine payload relocations to their owner headers (the weld
thinning from the correct side, ahead of the hubs era); the ribbon
pairing suspension named in its banner; the c4 hoists + forward
declarations (the two-pass-lookup corollary, now the addendum above).

## L-HUBS (queued — the remaining class-body includes)

mood.inl (**K4 — Jean's marination item, unchanged**: transition
machine, portal force-spawn, apply_mood fan-out, dual-entry ownership),
spawn_engine.inl (services hub: run_spawn_preamble/negotiate_position/
footprints — every converted module reaches it through the keyhole),
entity_pipeline.inl (the generic family dispatch), and the spine
chapters that remain the cartridge's own body. No rung is cut until
K4 is ruled.

## LADDER-4 — MOOD, PER K4 AS RULED (converted; L-MID COMPLETE)

Gated on L2; Jean reported GREEN, the rung proceeded. One module, one
commit, the proven pattern — plus the one structural move the ruling
ordered (the channel).

**The ruling this encodes (K4, Jean, 2026-07-11):** mood is vocabulary +
appliers + six doors; the transition machine is spine orchestration; the
force-spawn mutation belongs to the arch's owner. No MoodState exists or
is invented.

| piece | status | note |
|---|---|---|
| mood.hpp | ✅ NEW | CeilingType + MoodProfile + MOOD_TABLE + mood_name decl + portal vocabulary (PORTAL_DENSITY/COLORS/COLOR_BACK) + IndoorPalette/INDOOR_PALETTES + INDOOR_ENTITY_WALL_MARGIN (second consumer: negotiate_position) + the six doors + appliers + derivers declared. Mood IDs consumed from G1, not moved. static constexpr → inline constexpr. |
| mood.inl | ✅ REPURPOSED | no-BOM/LF verified BEFORE write, preserved. Self-wrapping, own includes, 17 inline definitions in original order (one impl-internal fwd decl: force_spawn_finite_portals). The lighting-scheme tables stay IMPL-side per the census (single consumer derive_indoor_lights — the agents precedent); the class-body private/public toggle they needed is RETIRED (its written retirement, fulfilled). The sun rider at upload_lights travels untouched. |
| composition root | ✅ NOTHING | mood owns no state. MoodState/mood_state_, transitionPhase_, pendingDestination_, backPortalPosition_, cpuPortalArray_, cpuSpotLights_ stay SPINE-RESIDENT — SEAM[spine:transitions] added at the machine's banner naming the ruling (§2 residency, the P5-readbacks legitimacy class); one §2 line rides in the constitution. |
| THE CHANNEL | ✅ LANDED | entities gains force_spawn_portal_arch (decl entities.hpp, def entities.inl): the arch owner authors the slot scan, Doorway tier-mean geometry, pier authorship (c->write_pier — K1's channel), the ~24 slot writes, arch_count, mesh-params upload + mesh-pending. Mood's force_spawn_portal_at SHRANK to computing values (color/destination/position/flags) + calling through; portals_dirty stays mood's flag, set caller-side on success (same frame, read only by upload_portal_array — behavior-identical). All three spawner paths route through the one choke point, as before. |
| THE ROSTER DOOR | ✅ MIGRATED | the ROSTER-GATE portal (b) early-return moved from mood's force_spawn_portal_at into force_spawn_portal_arch — its written retirement condition, fulfilled. Comment chain updated (roster.hpp's second-door note + FIRST-EDGE dual-maturity text; the door's own HOME text names the migration). G5/G6 semantics unchanged; sentinel census re-run: 2 UINT32_MAX returns (gate + no-slot), callers treat both as "none placed", exactly as before. |
| derivers | ✅ GRADUATED | derive_finite_radius (pure) + pick_portal_mood (keyhole — world_state_.finite_mode read) moved from cartridge members to the mood module. entity_pipeline's c->pick_portal_mood(...) became pick_portal_mood(c, ...) — the arc's ONE non-door call-site signature change; derive_finite_radius callers carry unqualified. |
| orphans | ✅ TAGGED | MoodProfile.fog_density/fog_color → INTENT[mood-fog-baseline], retirement "revive-or-delete at the panel era"; §5 ledger entry rides the commit (TER-2 doctrine; Jean holds the delete-override). |

**All-callers census (every door, every spawner path):** apply_mood ×1
(the machine's activation site, +this), request_mood_transition ×5
(input.inl F-row keys → (c, MOOD_*)) + the in-flight bail intact,
force_spawn_back_portal ×1 (teardown site, +this), upload_lights ×1 +
upload_portal_array ×1 (frame sites, +this), mood_name in-module ×5 +
zero external, force_spawn_portal_at ×3 (back ×2 branches + finite ×1 —
all in-module, all through the channel), pick_portal_mood ×2 (mood +
entity_pipeline), derive_finite_radius ×3 (mood ×2 + entity_pipeline).
Zero qualified stragglers (Cartridge::MOOD_TABLE etc. grep: none).

**Bodies verified** (normalized byte-compare): 17/17 mood functions
identical mod sanctioned transforms — two disclosed cosmetic deltas:
comment-column alignment in apply_mood's ROSTER-gate rows, and `(void)c;`
joining the gated early-return's cast list. The CHANNEL extraction
verified separately: the mutation span of old force_spawn_portal_at is
byte-identical inside force_spawn_portal_arch mod es./keyhole/Cartridge::
forms, the color parameter, and the two moved lines (portals_dirty →
caller; pc computation → caller).

**Specimen** (real mood pair + real entities pair + real
mood_constants/seed_utils/roster; shadow state + sibling stubs): indoor
vault apply (Sanctum + vault uplight, palette substitution, groin-vault
shell 1105 verts, wall paintings, ceiling clamp, orbs), mood-5 anchor
ribbon dual-entry, transition request + in-flight bail, THE CHANNEL
(back + 2 forward portals authored by entities: 6 pier writes, 3
mesh-param uploads, mesh-pending, Doorway tier-mean half-span exact,
back-portal destination intact, portals_dirty caller-set), portal-array
+ lights dirty protocols, derivers bounded/biased, outdoor apply clears
shell + walls. Exit 0.

**Zone census (both directions): CLEAN** — 11 impls == 11 zone includes,
each once; **the class-body census reads exactly TWO (spawn_engine.inl,
entity_pipeline.inl)** — the L-mid target, hit.

**Bytes:** mood.hpp 81/81 braces no-BOM/LF; mood.inl 146/146 no-BOM/LF
(byte truth verified before write); entities.hpp 71/71; entities.inl
30/30; cartridge.hpp code-only 582/582 (the graduated vocabulary chapter
accounts for the drop from 640). Constitution §2 line + §5 INTENT entry
ride the commit.

**L-MID IS COMPLETE — the last non-hub module is a citizen.** The L3
golden gate at the rig is due: one build at this head, same seed, two
runs, indistinguishable (L2 ran green, so this build certifies LADDER-4
alone; bisection anchors — the pre-LADDER-4 head bfd143d and c6 77fc3a3
— stand if it disagrees).

## COMPACT-2 — THE PROTOTYPE COMMENT LAW (one sweep, stamped)

Jean's ruling (2026-07-12): per-file structural prose retired until
ship; the four keep-classes (identity sentence / requirements-face
prose / ledger / code-adjacent one-liners) are the documentation;
§6 datasheets return at certification. Comment tokens 112,669 ->
65,608 (−47.1k; forecast −55.3k — the delta is D3's pin retention on
the GPU contract surface, itemized in the close-out). 134 constraint
rescues hand-ruled. Constitution §1 prototype-regime clause +
contract §6 note + §5 ledger entry rode the sweep. Gates green:
code-token-identical 37/37, encodings (BOM = quartet + renderer.hpp),
zone census 13 == 13. Close-out: audit/COMPACT2_SWEEP.md.

## COMPACT-1 — THE SWEEP (comment-only; F3 executed)

The narration stratum this record describes was compressed out of the
source: every NARRATION block is now at most one provenance pointer
line per file ("Converted <arc>: history in audit/LADDER.md" — this
file is that history). F3 executed on both strata (DONE[] 26→0,
Scope-B 11→0; SEAM[] stays); the graduated-symbol memorials in
cartridge.hpp are gone. Present behavior revealed inside cut narration
was rescued in present phrasing, never deleted. Boilerplate
consolidation rode as its own commit: modules/keyhole.hpp (the
forward-decl home) now provides the Cartridge + wgpu::Queue fwds once;
the twelve module headers include it. Gates: code-token-identical per
file (commit 1), declaration-identical with disclosed recipe
(commit 2), zone census + encodings throughout. Full close-out with
totals, per-file bytes, rescue disclosures, and the code-repetition
appendix: audit/COMPACT1_SWEEP.md.

## LADDER-5 — THE CONTRACTS STRATUM (extraction, not conversion; LANDED)

Map: audit/LADDER5_RECON.md (its counts governed). Four stages, four
commits, bisection-ready. Scope guard held: code moved, boundaries
drawn, zero generation/placement redesign.

### e1 — THE GRADUATIONS
EntityQueueEntry + PlacementEntry + the FamilyDispatch row type
graduated to entity_types.hpp — THE CONTRACT HOME (it gained the three
bespoke subsystem includes; the union IS the coupling, and the
consequence is structural: owner headers can never include the
contract home, so queue-shaped signatures declare there).
PopFamily graduated to roster.hpp: identity + enablement one
self-binding document; family_enabled's literal-index switch died into
named labels; cartridge.hpp's ~20-line static_assert binding block
died entirely; 11 Cartridge::PopFamily sites re-pathed; 100
unqualified uses carried (G1 precedent). Riders: the misplaced
seed_utils note deleted; spawn_engine's stale export-box rows fixed.
Gates: all four constructs TOKEN-IDENTICAL at their new homes.

### e2 — THE TABLE AT FILE SCOPE + THE WRAPPER SPLIT
FAMILY_DISPATCH: extern-declared in the contract home, DEFINED at file
scope in modules/family_dispatch.inl (born; zone include, last) — the
spine's loops kept reading it by namespace lookup, zero consumer edits
at all 7 sites. Specifier disclosed: static constexpr member ->
inline const namespace definition (address-constant initializers,
static initialization; zero constant-expression consumers). The 9
bespoke funnels moved verbatim to their owners' impls; the 10
identical no-op prepare/mesh stubs collapsed into ONE shared pair
(the mechanical subset — the remainder, 12 real one-line adapters
with unmatchable signatures, stays spine-side, disclosed).

### e3 — THE EVICTORS WENT HOME (§5 retired per condition)
The twelve evict bodies relocated into their owners' impls as
keyhole-shaped evict_<family>, declared in owner headers, named
directly by the table: entities.inl ×7, cube_behaviors.inl,
gol_zones.inl, gallery.inl, ribbon.inl (sky-mode pin + ref-count law
traveled intact), and modules/spheres.inl BORN for the sphere evictor
(SphereState's owner had no impl file — disclosed; census 13 == 13).
Constitution §5 EVICTION THUNKS: RETIRED PER CONDITION, same commit.
Gates: 12 bodies TOKEN-IDENTICAL (rename + inline the only deltas);
dispatch_evict_* = 0 remaining.

### e4 — THE CLEAN THREE RELOCATED (close-out)
blade (196 L), palm (232 L), cactus (180 L) recipes moved whole to
entities.inl beside their preparers and evictors — 611 lines exactly
as forecast. Two disclosed transforms only: line-start static ->
inline, THEMES -> Cartridge::THEMES (tagged INTENT[services:themes]
at its definition — future services-graduation item, not graduated,
by order). Funnel declarations joined the contract home; 9 table rows
re-pointed. Welds honored: column+antenna untouched (one GPU store);
arch/pyramid stay with the machine.

### CLOSE-OUT

**Wrapper-inventory delta (70 -> 62):** 27 generic funnels (18 stay
in-class with the six machine-side families; 9 relocated with the
clean three) + 9 bespoke funnels (owners) + 12 evictors (owners) + 12
real prepare/mesh adapters (spine) + 2 shared no-ops = 62 functions.
Spine-side family wrappers: 70 -> 30 (18 funnels + 12 adapters);
owner-side: 0 -> 30; shared: 2. The DISPATCH WRAPPERS chapter fell
394 -> 37 code lines.

**§5 update:** EVICTION THUNKS retired per its own written condition
(e3, same commit). the_chord's copy untouched.

**Sphere/cube relocation forecast (DEFERRED BY DEFAULT, Jean may
pull):** the same shape as the clean three, now purely mechanical —
sphere block ~165 L -> spheres.inl, cube block ~200 L ->
cube_behaviors.inl; funnel declarations to the contract home; 6 table
slots re-pointed; the THEMES read qualifies identically. No new
blockers: e1/e2 removed them all. ~365 lines, one commit.

**Spine chapter map restated (for the LADDER-6 / arrow-law A6
ruling):** cartridge.hpp 2,423 -> 2,068 code lines this arc.
COMPOSITION ROOT ~699 · WORLD-ENGINE ~1,330 · DISPATCH 394 -> 37.
The hubs: spawn_engine.inl 550 -> 513 code (the machine — queues,
footprints, services, loops), entity_pipeline.inl 1,672 -> 1,177
(generic verbs + six family blocks), entities.inl 154 -> 774 (the
owner grew into itself). The world-engine class (~1,330 spine lines +
spawn_engine's 513) remains the confession for Jean's ruling on
whether the spine walks the ladder.

**Separation/proximity census (rider — report, Jean rules):**
MIN_SEPARATION[PopFamily::COUNT][PopFamily::COUNT] (cartridge.hpp,
TERRAIN TOKENS chapter) and the five PROXIMITY_* tables
(spawn_engine.inl) are pair-separation vocabulary split across the two
hubs. ALL consumers are machine-side: check_position reads
MIN_SEPARATION + PROXIMITY_AFFINITY + PROXIMITY_GAP_REDUCTION;
proximity_affinity_boost reads the rest. Nothing crosses the
machine/owner boundary, so they do NOT belong in the contract file
(which carries only what crosses); the flaw is the SPLIT, not the
residence. Recommendation if ruled: MIN_SEPARATION joins its only
readers beside the PROXIMITY_* tables in spawn_engine.inl — one
machine-side vocabulary block, no contract involvement. The integrity
audit's four-family coverage stands as choice-not-bug, as ruled.

**Gates standing at close:** zone census 13 == 13 (family_dispatch.inl
+ spheres.inl born this arc); encodings byte-checked per stage (BOM
quartet intact); every moved construct token-verified at its new home.
L4 GOLDEN due at the rig: one build post-e4, same seed, two runs,
indistinguishable. Optional family-off spot check offered (flip one
ROSTER family bit; semantics must survive the table's relocation).

### FIX — THE IMPLICIT-THIS CALL (rig-found, entities.inl:714)

Rig C3861: `rescale_to_rolled_target` — a STATIC member of the
class-body pipeline (entity_pipeline.inl), called unqualified inside
e4's moved palm recipe. Inside the class, membership supplied the
lookup; at namespace scope it supplies nothing (and ADL cannot find
class statics). Token-identity gates are BLIND to this by design —
the call moved verbatim; VERBATIM WAS THE BUG. Membership's fourth
privilege, now named beside the LADDER-2 corollaries:
**ACCESS, LINKAGE, LOOKUP ORDER, IMPLICIT-THIS CALLS.**

**The fix:** `Cartridge::rescale_to_rolled_target(...)` — a static,
so the class qualifies it (the adapter signature carries no keyhole;
c-> for members, Cartridge:: for statics).

**The sweep (recipe disclosed):** direct-member census = every name
declared at CLASS SCOPE (relative brace depth 0) across cartridge.hpp's
class body + spawn_engine.inl + entity_pipeline.inl (the class-body
includes) — 325 names after subtracting file-scope collisions; then
every region this arc moved (the e2 funnels ×3 impls, the e3 evictors
×6 impls, the e4 recipes, family_dispatch.inl) scanned for unqualified
identifier reaches — reads AND calls — against that census. RESULT:
**1 found (the rig's site), 0 siblings.** The recipe was validated
against the pre-fix tree first: it flags exactly the rig's site and
nothing else.

**Specimen post-mortem (hypothesis CORRECTED, not confirmed):** no
stub supplied the missing name at namespace scope — e4's gate suite
contained NO compile specimen at all. This environment builds no Dawn;
the gates were textual (token identity, censuses, zone), with compile
certification disclosed as rig-side. The blindness was ABSENCE, not a
lying stub — but the ordered law lands regardless, and the corrected
stub now EXISTS and ran here (g++ is available for reduced
specimens): a mock Cartridge supplying rescale_to_rolled_target AS A
MEMBER, with the moved recipe shape at namespace scope. PRE-FIX it
reproduces the rig's exact error class ("'rescale_to_rolled_target'
was not declared in this scope" — the C3861 analog); POST-FIX it
compiles clean. Recipe: scratchpad specimen_lookup_{prefix,postfix}.cpp,
g++ -std=c++20 -fsyntax-only.

**LAWS (pattern section):**
- STUBS SUPPLY MEMBERS ONLY — a mock Cartridge provides member
  functions and organs mirroring the real access topology; it never
  provides namespace-scope names for things that are members in the
  tree. A SPECIMEN COMPILE FAILURE IS DATA, never an inconvenience to
  stub away.
- PREFLIGHT AMENDMENT — the ambient-reach scan censuses CALLS, not
  just data/statics: every identifier a moved region invokes is
  checked against the member census. A reach is a reach whether it
  reads or invokes.
- Every extraction arc carries a LOOKUP SPECIMEN henceforth: the
  moved region's lookup shape compiled against the corrected stub,
  pre-fix red / post-fix green.

Rebuild is take two; the e5 rider and COMPACT-2 stay queued behind
that green, in that order. (Take two ran GREEN; the arc is ratified.)

### e5 — THE RIDER (post-L4-green; three moves)

**e5a — DTO repatriation (the arrow correction).** The three
Selection/Placement pairs moved from their owner headers into
entity_types.hpp beside the unions that are their reason to exist.
Principle on record: A DTO THAT EXISTS TO CROSS A BOUNDARY BELONGS TO
THE BOUNDARY'S CONTRACT, not to either side. The DTOs are plain
aggregates of built-ins, so the contract home now carries NO owner
vocabulary: its three module includes died (a local
wgpu::ComputePassEncoder fwd replaces their transitive completeness),
owner headers include the contract home, and the 18 queue-shaped
funnel declarations went back to their owners' headers (gol_zones /
gallery / ribbon ×3, entities ×9). The e1 circularity consequence is
struck — the arrow points the right way. Gates: 6 DTOs
token-identical; include graph acyclic; decls 0 contract-side.

**e5b — the sphere/cube pull (Jean pulled it).** The floater recipes
moved whole to their M-c owners: sphere 164 lines → spheres.inl, cube
199 lines → cube_behaviors.inl, beside their evictors — same two
transforms as e4. entity_pipeline.inl keeps FOUR family blocks: the
welded set (column+antenna, pyramid, arch), exactly the families that
weld to the machine's pier/regen services. Six table rows re-pointed.
The amended-preflight LOOKUP SWEEP (member census, reads AND calls)
ran over every owner-side region: 0 unqualified member reaches.

**e5c — MIN_SEPARATION home.** The pair-separation matrix (46 lines,
token-identical) left the spine's TERRAIN TOKENS chapter and joined
its only reader (check_position) beside the PROXIMITY_* tables in
spawn_engine.inl — the separation/proximity vocabulary written once,
where the machine that consumes it lives. Legality is the same
complete-class-context lookup PROXIMITY_* already relied on.

**Standing at rider close:** zone census 13 == 13; encodings green
(BOM quartet intact); wrapper geography final — spine-side 12 real
prepare/mesh adapters + 12 in-class generic funnels (the welded
four), owner-side everything else. entity_pipeline.inl is down to the
generic verbs + four welded family blocks; every clean family lives
whole with its owner. COMPACT-2 is next in the queue.

## LADDER-6 — THE STRATA ARC (three S2 extractions, two S3 conversions, §1 COMPLETE)

**The arc (Phase R census 9257e66 stamped at three modules; Phase I
c92120b, b308393, 0993c3a, 2e73561, bcbf1e9):** the world-engine
class walked the ladder and the ladder ended. Three S2 extractions in
reach order — population_themes (s1), tile_world (s2, tokens merged),
patch_system (s3, the conductor rides whole per R-c) — then the last
two class-body citizens converted (spawn_engine 3b-i, entity_pipeline
3b-ii), and §1's completion sentence executed same-commit with the
last conversion (3c). CLASS-BODY INCLUDE COUNT: ZERO. The cartridge
is the composition root alone: organs, conductors, and the spine's
12 prepare/mesh dispatch wrappers (the e2 integration-glue ruling
stands).

**Stamp rulings executed:** R-a — WorldState's struct home is
patch_system.hpp, the world_state_ instance is a ROOT ORGAN. R-b —
teardown_world in keyhole form, identity line CALLER: the transition
machine (root); OWNER: patch_system. R-c — stream_patches moved WHOLE
as the per-frame conductor, its banner declaring
SEAM[patch:spawn-trigger] (select/place/commit +
update_entity_draw_visibility + flush_pier_count as the seam face).
PIERS (write/clear/recompute/flush + cpuPiers_) rode patch_system at
spawn_engine's conversion; estimate_terrain_height/terrain_tile_warm
rode tile_world (state-first, pure over the state);
mark_patches_for_regen rode patch_system. Census-ruled homes:
solve_catenary_a -> seed_utils.hpp (pure math, four cross-module
consumers); theme_short_name -> population_themes.inl (sole consumer,
its own vocabulary).

**The four boundary faces carry their identity lines:** THEMES
(population_themes.hpp, s1) · the tile cache (tile_world.hpp, s2) ·
the patch registry via find_patch/record_entity (patch_system.hpp,
s3) · the surface samplers estimate_terrain_height/terrain_tile_warm
(tile_world.inl, 3b-i). All remain keyhole/state-first form this arc
as stamped.

**THE TEMPLATE KEYHOLE (new law, 3b-i):** a header-defined template
above the incomplete class takes its cartridge parameter DEDUCED
(template<typename C, ...>, C* c) — the c-> reaches become dependent
and are checked at instantiation in complete-class context.
run_spawn_preamble is the canonical instance. The specimen runs
-Werror: GCC delays the concrete-keyhole diagnostic to a warning by
default; the deduced form is warning-clean. THE ZONE-ORDER LAW
(3b-ii): FAMILY_DISPATCH's address-of initializers require the
wrapper definitions' zone include to precede family_dispatch.inl —
specimen-proven (red: 'was not declared' at the table).

**Gates, per stage:** token identity modulo disclosed transforms
(29 + 37 + 11 constructs across the five stages, stream_patches at
3,017 tokens the largest single construct); g++ -std=c++20 lookup
specimens red/green per stage; scope-aware implicit-this sweeps
(teeth-tested: planted violations detected; underscore-convention
census names shielded from header-word leakage); zone census closed
at every stage (15->16->17->18 == includes, family_dispatch.inl
last); sentinels stable throughout (ROSTER-GATE 23, ROSTER-RESIDUE
5); encodings match HEAD per file every stage (BOM riders:
ribbon.inl, entity_pipeline.inl, render_passes.inl edited in place).
L5 GOLDEN certifies at the rig — this arc's build and COMPACT-2's
compile both ride Jean's first build.

**§4 DELETION-TEST DRY RUN (population_themes, report-only):**
deleting the module touches the composition root (3 lines: cohort
include, organ comment+decl, zone include) plus 20 non-root sites in
9 files — VERDICT: boundary honest, deletion NOT root-only.
Classification: every non-root reach passes through a declared
surface — (a) the THEMES face: ten per-family tier-weight accessors
(entity_pipeline x4, entities x3, spheres, cube_behaviors, ribbon)
+ tile_world's lattice blend; (b) the TileState.theme_spawn columns
(tile_world.hpp store; spawn preamble, gol_zones, gallery bespoke
preambles read them); (c) the conductor seam (patch_system's
evaluate_theme_envelope call + teardown reset); (d) the S3 preamble's
active_theme_idx_ read. ZERO reaches outside the declared faces.
A-PROGRAM LEDGER ITEM: the theme axis is a cross-cutting selection
modifier by design; if severability is wanted, the cut point is the
adapter slot get_theme_tier_weights + a null-object THEMES row —
that would confine deletion to the root and the table's home.

**Deltas:** cartridge.hpp 2,901 -> 1,209 lines this arc (-58%; from
3,283 at the MOD campaign's start). spawn_engine 744 -> 461 impl +
285 header; entity_pipeline 1,144 -> 1,076 impl + 131 header.
modules/ stands at 41 files (23 headers, 18 self-wrapping impls).
Strata tags as-built: S2 population_themes / tile_world /
patch_system; S3 spawn_engine / entity_pipeline; prior-era modules
untagged (their strata land when their arcs come). Root retains:
COMPOSITION ROOT (organs), TIME/MOOD/PLAYER/PORTAL/READBACK state,
FAMILY DISPATCH chapter prose + the 12 prepare/mesh wrappers,
initialize/init_renderer/update/render/on_input, the frame-signal
fill, and the transition machine.

**§1 COMPLETE (3c, same commit as the last conversion):** the
transitional clause STRUCK per its own completion sentence; the
mistake clause holds one mistake (the single TU), its second died
with its subject; §5 HEADER LADDER entry dated 2026-07-12; the
cartridge banner reads ONE REGIME. The one-TU line survives inside
the completion record — boundary honesty, not compilation strategy.

**Queue restated (Jean's word):** the dissolution era (A2 -> A3 per
the theory doc's arrow-law program) or the coupling dogfood — his
fork. M-m/M-n stay parked; the spine's remaining confession is the
root's size (1,209 lines of assembly), which is now an honest number.
