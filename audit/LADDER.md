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

TWO GATE LAWS (restored at the LADDER-6 FIX; permanent):
G-LAW 1 — THE STANDALONE COMPILE RETURNS: every generated or edited
header AND impl compiles whole, standalone, at final HEAD (LADDER-1
R7, un-narrowed). Targeted red/green law-probes are ADDITIONS to this
gate, never replacements. Any conforming compiler suffices — braces
and scope fail everywhere. In-container executable form:
audit/tools/glaw1/run.sh (the real cartridge TU under g++
-fsyntax-only with only the absent SDK surface stubbed).
G-LAW 2 — NO EDIT AFTER THE LAST GATE: gates certify the COMMIT, not
a moment mid-flight. Any byte-touching pass (prose scripts included)
reruns the full suite behind it. If a pass cannot afford the rerun,
the pass moves before the gates.
COROLLARIES (recorded beside them; ratified in full at the post-L5
rider):
— Per-file brace balance is insufficient alone: an unbalanced header
  poisons the TU downstream; the standalone compile is the gate that
  sees it.
— THE MIRROR LAW: A CHECKER THAT SHARES THE GENERATOR'S EXTRACTION
  IS A MIRROR, NOT A GATE — every checker derives its ground truth
  independently of the machinery it checks. (The s2 token checker
  sliced constructs with the same `};`-suffix heuristic the generator
  truncated with, and certified the truncation.)
— THE WRONGLY-QUALIFIED BLIND SPOT: sweeps hunt unqualified reaches;
  only the compiler sees stale-qualified ones — G-LAW 1 is that
  auditor.

PATTERN LAW — THE TEMPLATE KEYHOLE (ratified at the post-L5 rider):
a header-resident generic may take the cartridge as a DEDUCED
parameter (template<typename C, ...>, C* c); reaches are checked at
instantiation, in complete-class context, and are documented in the
K2 reaches line like any impl. The form names no Cartridge and is
therefore arrow-compliant by construction; at the services era the
deduced parameter becomes the services struct without the signature
changing shape. run_spawn_preamble (spawn_engine.hpp) is the
canonical instance.

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
RATIFIED GRADES (post-L5 rider): the deletion test carries two
grades — ROOT-ONLY (the end-state: delete the module and only the
root changes) and FACES-ONLY (the transitional pass: every non-root
edit falls on a declared face). population_themes PASSES FACES-ONLY.
A-PROGRAM LEDGER ITEM — the first severability recipe: the theme
axis is a cross-cutting selection modifier by design; the cut point
is the adapter slot get_theme_tier_weights + a null-object THEMES
row, which would raise the module to ROOT-ONLY.

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

## LADDER-6 FIX — THE UNCLOSED BRACE (one root, eleven sites; compile-cleared)

**The root (Jean's rig, MSVC; his diagnosis verified exactly):**
tile_world.hpp shipped at s2 with TWO unclosed structs. The s2
generator's construct-end detection matched any line ENDING in `};` —
and both TerrainToken's archetype_bias and TileState's theme_spawn
carry brace-initializers ending in `};`. TerrainToken lost budget +
active + its closing brace; TileState lost theme_idx + its closing
brace. The error log's nested name TerrainToken::TileState::
TileWorldState was the confession: siblings parsed as matryoshka, the
namespace leaked past EOF into patch_system.hpp, and a standard
header expanded inside t7::the_board (the fix-2 landmine at TU
scale). The s2 token checker certified the truncation because it
sliced with the same heuristic — a mirror, not a gate. REPAIR: both
struct tails restored; TerrainToken and TileState re-verified
token-identical against the pre-move originals at the parent commit
(39 and 109 tokens, PASS).

**The arc-wide clearance (G-LAW 1, by compile, not re-reading):**
audit/tools/glaw1/ — the real cartridge TU under g++ -std=c++20
-fsyntax-only, project code real, only the absent SDK surface stubbed
(webgpu + GLFW as a universal-member lattice; Dawn's transitive std
headers mirrored). The harness converged in rounds and found TEN MORE
defect sites beyond the root, ALL inside the arc's generated/edited
set — no third cause:
- spawn_engine.inl update_entity_draw_visibility: the keyhole
  transform collided with the pre-existing locals `const auto& c =
  ...columns[i]/antennas[i]` — self-referential deduction, and every
  c-> reach in those two blocks bound to the shadow. Locals renamed
  col/ant (disclosed shadow-rename transform). This would have failed
  MSVC identically; the 3b-i token gate passed it because the checker
  applied the same mechanical transform — the mirror lesson again.
- gallery.inl (x4) + gol_zones.inl (x2): c->tileCache_ survived the
  s2 re-path (its census caught unqualified reads only, not
  keyhole-form reads in owner impls) -> c->tile_world_state_.tileCache_.
- patch_system.inl teardown_world (x3): c->entityQueue_ /
  c->placementResults_ / c->footprints_ survived the 3b-i rider
  (wrongly-qualified reaches are invisible to the implicit-this
  sweep, which hunts unqualified ones) -> c->spawn_engine_state_.*.

**Gates behind the fix (G-LAW 2):** the full TU compiles GREEN
standalone; structural balance ALL files; zone census 18 == 18;
sentinels 23/5; encodings match HEAD. The two G-LAWS + corollary are
in THE RECIPE above, permanently; the harness is committed at
audit/tools/glaw1/ (stubs generated at run time, gitignored).

**Standing:** ratification of the Phase I close-out — including the
template-keyhole pattern (its law needs writing if it stays) and the
deletion-test ledger entry — is SUSPENDED until L5 take two at the
rig, which also certifies COMPACT-2's sweep. Jean rebuilds.

## LADDER-6 — SUSPENSION LIFTED (L5 take two GREEN; all ratifications land)

L5 green at the rig certifies the arc, the fix (5116c77), and
COMPACT-2's sweep. Phase I close-out RATIFIED as committed, fix
included. The mirror law stands in its full form and the
wrongly-qualified blind spot beside it (THE RECIPE's corollaries,
above); THE TEMPLATE KEYHOLE is pattern law (arrow-compliant by
construction — the form names no Cartridge; at the services era the
deduced parameter becomes the services struct without the signature
changing shape); the glaw1 harness STAYS at audit/tools/glaw1 with
its jurisdiction line in the banner (our names, scope, and syntax —
SDK-call correctness remains the rig's); the deletion test carries
its two grades (ROOT-ONLY end-state, FACES-ONLY transitional) with
population_themes passing FACES-ONLY and the A-ledger holding its
first severability recipe. No code moved. The queue awaits Jean's
fork ruling: the dissolution era (A2 -> A3) or the coupling dogfood.

## DEMO-1 — STAGE ZERO: THE REHOMING (the directory becomes the map)

One atomic commit, pure moves (git mv, history carries): 45 files
into 7 rooms — cartridge.hpp ALONE at the top; primitives/
(seed_utils) · contracts/ (roster, keyhole, entity_types,
mood_constants, floater_vocabulary, ground_architecture) · surface/
(patch_system, tile_world, population_themes — pairs) · machine/
(spawn_engine, entity_pipeline — pairs; family_dispatch.inl) ·
bodies/ (pawn, entities, orbs, spheres, cube_behaviors, ribbon,
gallery, gol_zones, agents — pairs co-located) · direction/ (mood,
input — pairs) · realization/ (render_passes pair, renderer.hpp,
state.hpp, world.wgsl). modules/ is gone. No pair splits a room.
Content untouched except #include path strings (94 lines, 22 files),
renderer.hpp's two shader search strings (the hot-reload census),
and the two ordered LATENT[naming] tags (entities.hpp — the
grounded-seven registry misnamed; floater_vocabulary.hpp — contracts/
resident despite the name; debts flagged, not paid). world.wgsl moved
as BYTES. Purity proven per file: whole-file identity vs HEAD modulo
those classes; BOM quintet intact at new paths; encodings match HEAD
per file. Gates at new paths: glaw1 GREEN, zone census 18 == 18,
sentinels 23/5. CMake verified glob-recursive and path-agnostic
(nothing names modules/). Stale comment-level modules/ pointers
censused, not edited (12: cartridge.hpp x2, state.hpp x10 — prose,
not includes). demos/ NOT created (YAGNI).
THE_CHORD LADDER-REPLAY LEDGER (first line): the rehoming replays
onto the_chord when its ladder runs — same map, same purity gate.

## DEMO-1 — THE CONFIG SPINE (demos/ is born; minimal.hpp is sentence one)

**The room and the sentences (eight rooms now):** demos/ arrives by
need, holding demo.hpp (the selector), full.hpp (THE GOLDEN TWIN —
today's program as a sentence; roster literal + seed 42 + boot mood
MOOD_OPEN_DEFAULT, token-identical to the pre-spine constant), and
minimal.hpp (the degenerate proof: all 12 family bits + all 7
feature bits OFF, seed 42, the open default — legal by the FIRST
EDGE's conditional form). contracts/demo_config.hpp is the contract:
DemoConfig v0 = { Roster roster; uint32_t seed; uint32_t boot_mood }
— nothing else; the maturity dial (compile-time include -> boot-time
table -> panel) and the D1-D5 growth axes sit beside the type; the
boot-time dial stays PARKED with its puller named.

**The selection mechanism (disclosed form):** the incubator's idiom
exactly — INCUBATE_DEMO=<demos basename>, default full when
undefined; demos/demo.hpp stringify-includes the sentence
(T7B_DEMO_HEADER, the RENDER_HEADER shape) and then DEFINES
`inline constexpr Roster ROSTER = DEMO.roster;` plus the FIRST EDGE
static_assert (both token-identical to their roster.hpp originals).
THE DAG REASON on record: DemoConfig embeds Roster by value, so
demo_config.hpp includes roster.hpp; had roster.hpp kept the
constant it would need the selector back — a pragma-once re-entrant
cycle that breaks when demo_config.hpp is the entry include. The
constant therefore lives at the selection point; roster.hpp keeps
the type, the vocabulary, and the gate law text. CMake grew
INCUBATOR_DUAL_DEMO (cache var -> INCUBATE_DEMO define), the render-
cartridge idiom mirrored.

**Plumbing censuses (one source of truth each):** the seed enters at
exactly ONE site — WorldState.active_seed (surface/patch_system.hpp),
formerly `= 42`, now `= DEMO.seed` (world transitions still mutate it
at runtime; boot comes from the sentence). Boot mood enters at
exactly ONE site — MoodState.active (cartridge.hpp), formerly `= 0`,
now `= DEMO.boot_mood`; apply_mood runs only on transitions, and 0
IS MOOD_OPEN_DEFAULT, so full is value-identical. The constexpr
chain is intact end to end: G5 verified by probe — a deliberately
illegal scratch sentence (transitions on, portal off) fails the
compile at demos/demo.hpp's FIRST EDGE with the exact refusal text;
scratch deleted after the red. glaw1 GREEN at default (=full) and at
INCUBATE_DEMO=minimal. The 12 stale modules/ prose pointers rode
this commit as the ruled one-line pass (bodies/agents.inl x10,
bodies/ribbon.inl x2 across cartridge.hpp + state.hpp).

**M1-SKY CHECKPOINT (censused, reported, nothing repaired):** the
sky's disposition under ribbon-off is PRESENT BY CONSTRUCTION — the
visible sky is the clear color (spine, clearColor_ 0.85/0.78/0.72)
plus mood fog and sun defaults (direction-owned), none of it
roster-gated; the orbs sky-dome is absent by its OWN bit, as
intended. SEAM[ribbon:sky-mode] names sky-FLIGHT (the pawn's ribbon
mount), not sky rendering. THE COMPOSITION FINDING is adjacent: F8
(toggle_sky_mode, direction/input.inl) is NOT ribbon-gated — under
minimal it sets player_.sky_mode and the wgsl mount branch
(signal.sky_mode != 0) mounts the pawn to a zeroed head pose with no
ribbon alive. One keypress reaches a dead subsystem's door. LEDGER
ENTRY: the sky-flight door rides ribbon's bit conceptually; the
gate (a ROSTER.ribbon check at the key site or the door) is a ruled
follow-up, not silently repaired here.

**Dormant cargo (out of scope, as ordered):** the SH
created-but-pristine allocations, world.wgsl's maximal compile, and
state.hpp's full contract text travel with minimal untouched —
parked, priced (audit/ROSTER_GATE_A.md), pulled on their own days.

**Rig gates (Jean performs):** G0' FULL GOLDEN — demo=full
byte-identical to head, same seed, one run. M1 MINIMAL — boots
sunlit, terrain streams and culls, the pawn walks and snaps, camera
couples, NOTHING else ever appears; two runs, pixel-stable. DEMO-2
(the terrain's voice — the wave rewired) queues behind Jean's word
and M1's disposition.

## DEMO-1 RIDER — THE CMAKE DOOR (one edit, then never again)

THE EXACT PAIR (disclosed): THE_BOARD_DEMO (the CMake cache variable,
default "full") -> INCUBATE_DEMO (the compile define) ->
demos/<name>.hpp (the sentence, stringify-included by the selector).
The DEMO-1 arc had shipped the door per-target (INCUBATOR_DUAL_DEMO);
the rider renames it to the cartridge-owned name and wires it into
BOTH incubator targets' define blocks (inert unless the target's
render cartridge is the_board — only its selector consults the
define). CMakeLists edited once, here; never again per demo.

THE GOLDEN HOLDS, verified by inspection: the real configure needs
Dawn (rig-only), so a harness carrying the door lines verbatim
around a stub target was configured both ways and the GENERATED
COMPILE COMMAND read directly — default configure (no -D) puts
-DINCUBATE_DEMO=full on the line (the cache default IS the golden
path); -DTHE_BOARD_DEMO=minimal puts -DINCUBATE_DEMO=minimal there;
the cache holds across reconfigure (the door remembers). The rig's
real configure re-proves on Jean's next build. Sighting on record:
the define blocks sit under if(MSVC) beside their INCUBATE_RENDER
siblings — consistent, the build file is MSVC/Dawn-pathed throughout.

THE SWITCH RECIPE (the production line's cadence, written where a
stranger finds it):
  command line:  cmake -B build -DTHE_BOARD_DEMO=minimal
                 cmake --build build --target incubator_dual
  Visual Studio: Project > CMake Settings (CMakeSettings.json) >
                 CMake variables -> set THE_BOARD_DEMO to the
                 sentence name (e.g. minimal); rebuild. Back to
                 golden: set it to full — or delete the cache
                 entry; the default IS full.

Rig gate unchanged: Jean configures default (G0' golden), then
-DTHE_BOARD_DEMO=minimal for M1 and the sky checkpoint, reporting
dispositions as the DEMO-1 handoff ordered.

## DEMO-1c — THE HANDLE AND THE PIPELINE GATE (two frictions, one rider)

**THE HANDLE (CMakePresets.json; the deprecated pane retires):** three
configure presets over a hidden base (Ninja, Debug, the Dawn paths) —
"the_board — full" (render=the_board, THE_BOARD_DEMO=full),
"the_board — minimal" (same render, demo=minimal), "the_chord"
(render=the_chord). Each preset pins BOTH knobs so intent is one
click and the inert-define trap is impossible; matching buildPresets
name the target (incubator_dual / incubator). Verified: cmake
--list-presets surfaces all three (VS reads the same file into the
configuration dropdown — Jean eyeballs at the rig). The cache-var
mechanism stands exactly as landed; presets are a handle over it.
THE SWITCH RECIPE (updated): one click on the preset in VS — or
  cmake --preset the-board-minimal && cmake --build --preset the-board-minimal

**THE PIPELINE GATE (a') — PHASE R' CENSUS (65 pipelines, one
creation site + one reader helper each; the helper owns the only
SetPipeline):**
FOUNDATIONAL (15, never gated): updateTerrainConfig,
updatePlayerAgent (THE PLAYER), updateCamera, computeVP (camera; the
spot path computes VPs CPU-side — spot_lights owns no pipeline),
generateTerrainIndices, generatePatchHeights/Gradients/Cells
(surface), patchTerrain + patchTerrainIndirect + shadowPatchTerrain
(surface draw), pawn + shadowPawn (the playable character),
entityPlacement (shared by ALL families — the ambiguity rule),
frustumCull (render infra).
GATED (50, by owner): sphere/cube/ribbon/arch/palm/cactus/blade/
pyramid x3 each (mesh-or-update compute + draw + shadow);
column+antenna x3 SHARED-PAIR (gate on column||antenna — the antenna
family rides the column pipelines by design); gol x7; gallery x6
(photographerVP rides gallery's bit per the roster's LATENT split
note); orbs x5; pawn_aura, wanderers, transitions (fadeOverlay — its
only consumer) x1 each; indoor_shell x2.
RE-SECTION CASES: NONE — every gated pipeline references shared
layouts only; nothing joins the parked ledger. Desc-mutation chains
audited: every inter-piece descriptor dependency flows through
unconditional outside-lambda setup; gated lambdas that mutate the
shared desc each set all varying fields themselves.

**PHASE I' (the cut, form disclosed):** each creation tPipe statement
wrapped in `if constexpr (ROSTER.bit)` (50 consults); each owned
helper opens with `if constexpr (!bit) return;` (49 consults — the
holder tolerates a never-created handle at its single SetPipeline
choke point; callers untouched, gate (b) semantics unchanged).
reload() re-runs the same gated creation path — hot-reload honors
the demo. Renderer::pipelines_skipped() computes the count constexpr;
the boot summary's existing all_enabled()-gated line extends with it
(buffer-creations wording tightened same-line) — all-enabled stdout
unchanged by construction. Every new consult carries its sentinel:
ROSTER-GATE 23 -> 122 (+50 creation, +49 holder); RESIDUE 5
unchanged — and the gol residue recipe extends naturally
(never-created pipelines are the stronger pristine).

**GATES:** glaw1 GREEN at full and at minimal; zone census 18 == 18;
encodings — renderer.hpp's BOM preserved through 100 insertions,
cartridge.hpp clean, CMakePresets.json new no-BOM/LF. At minimal the
constexpr count says 50 pipelines skipped (every gated one). THE RIG
CLOSES: demo=full — byte-identical build, unchanged boot stdout,
zero skips, golden; demo=minimal — boots FAST, pawn walks, the skip
line reports "pipelines skipped: 50"; two runs pixel-stable.
Boot-time before/after: Jean supplies the two timings — the
instrument's reading, quantified.

**SCOPE GUARD held:** megabind re-section, SH-dc draw unlock, and
buffer non-creation stay parked with their prices; this rider paid
only the FXC half the demo made hurt.


## REBUILD-0 — PHASE R: THE RECON (read-only; report landed, awaiting stamp)

Eight parallel censuses over the full cartridge; ONE report at
audit/REBUILD0_RECON.md — score map + proposed movement prose, deps
table + graduation list, channel census, witness boundary, sky priced
both ways (P3 material), packer forecast, the 34-second diagnosis
(P4), and the census-adjusted stage order m1-m7. Zero bytes of code
touched. The movements cut only after Jean stamps the set.

## REBUILD-0 — PHASE I BEGINS (stamp received; nine decisions, D1-D9)

The lens landed first (Jean's push): theory v3 + the demo contract in
src/docs/ (v2 superseded and removed by the same push). The campaign
runs m1 -> m6; m7 packers NO-GO (assert-first rider recorded as the
standing precondition for a future pull). R7's levers ledger: chain
multiplier, lattice bake, pipeline blob caching — parked, outside
this campaign.

**m1 — SERVICES GRADUATION (+ the D9 micro-item).** contracts/
spine_state.hpp is born: TimeState, PlayerState (SEAM[spine:P8]
rides with the struct), and TransitionPhase graduate to file scope —
TOKEN-IDENTICAL modulo indentation (gated). MoodState graduates to
direction/mood.hpp per D3 (the WorldState pattern, R-a: struct with
its semantic owner, instance spine-resident; the mood.hpp "OWNS NO
STATE" banner becomes "OWNS NO INSTANCE", K4 as amended by the
stamp). The Cartridge:: qualification tax retires at mood.inl
1001/1008 and agents.inl 227. D9: the F8 entry door gains
if constexpr (ROSTER.ribbon) — the pawn-to-origin trap in ribbon-less
demos closes five stages early; m6 still re-homes the machinery.
Readback enums + dispatch wrappers STAY per the evidence (recon
§2.1). GATES: glaw1 GREEN at full and at minimal; four moved blocks
token-identical vs HEAD; zone census 18 == 18; sentinels ROSTER-GATE
122 -> 123 (+1, the D9 gate), RESIDUE 5 unchanged; encodings — the
BOM quintet intact (agents.inl edited below its BOM), new contract
no-BOM/LF.

**m2 — THE SCORE REWRITE + THE SCORE CENSUS.** update()/render()/boot/
teardown now read as EXPLICIT PROSE with movement banners (THE SCORE
RULING): CLOCK/SIGNAL -> S4 DRIVERS -> S4 BODIES -> REALIZATION
STAGING (part one, pre-machine by cited constraint) -> THE MACHINE ->
STAGING (part two) -> WITNESS -> BOOKKEEPING in update(); WITNESS
HARVEST (O-2 vetoes the charter's after-motion seat — banner names
it) -> S2 -> S3 -> S4 BODIES -> REALIZATION -> WITNESS CAPTURE ->
REALIZATION CONTINUED in render(). The typelist fold DISSOLVED:
twelve explicit constexpr-gated prepare lines. NINE STRAYS came home
as owner verbs (token-identity gated, 15/15 blocks): mirror
reconciliation (spheres/cubes; SPAWN_PROTECTION_S graduated to
floater_vocabulary), dispatch_pawn_aura, seed/reseed_player_body (the
twins stay twins — byte-exactness outranks unification),
release_finite_ribbons, drain_gallery_promotions, the three GoL pass
verbs. TEARDOWN became per-owner verbs per D4 (teardown_world ->
teardown_surface + teardown_entities/gol/ribbon/gallery/pawn_aura +
the standing clear/teardown trio); the new teardown gates eliminate
only zeros-over-pristine GPU writes (disclosed); the gallery organ
gates on (gallery || indoor_shell) — wall frames share the painting
slots. **P1 AND P2 ARE DEAD STRUCTURALLY** (update_photographer and
load_authored_textures under ROSTER.gallery at their named lines).
The ungated three gained gates: corral (cube), ribbon_frame_tick
(ribbon), the GoL block (D7 — structural above runtime). REORDERS
RC-1 (respawn after stream) + RC-2 (corral after stream) cut under
the stamped policy with written safety arguments in place; the rig's
pixel gate arbitrates — failure reverts per the policy. Disclosed
residue: within-frame stdout interleave shifts (census lines now
follow corral instead of stream); intra-teardown per-organ clear
order changed (independent writes, argument in the score text).
THE SCORE CENSUS lands at audit/tools/score/run.py — the standing
tool beside glaw1: Direction A (every piece's conductor calls present
AND gated, incl. the delegated doors in mood.inl/entities.inl/
input.inl), Direction B (every free call in the score attributed:
30 manifest + 24 foundational-with-justification; an ungated module
tick is UNWRITABLE — how P1/P2 stay dead). Teeth-tested red/green
(ungated-photographer specimen goes RED; restore goes GREEN).
GATES: glaw1 GREEN full + minimal; score census GREEN; 15/15
relocations token-identical vs HEAD modulo disclosed transforms
(c-> keyhole prefixes, own-state renames); zone census 18 == 18;
sentinels ROSTER-GATE 123 -> 148 (+25, itemized), RESIDUE 5;
encodings — BOM pair (agents.inl, ribbon.inl) intact, all files
unchanged.

**m3a — THE BYPASS CLEANUPS (the invariant slice of m3; recipe note
holds the rest).** The four GPUState&-signature functions take the
keyhole: clear_spheres moved header->inl (the pre-class header could
not deref the incomplete Cartridge — that WAS the bypass's cause),
clear_cubes re-shaped in place, ribbon_advance_head +
ribbon_rebuild_body_upload re-shaped with both call sites. The raw
config() pokes are NAMED: stage_pier_count / stage_placement_patch_
count / stage_lod_pawn / stage_floater_coordination — STAGED setters,
deliberately without configDirty_ (the pokes ride targeted sub-range
uploads or the next dirty/dynamic full upload; the house setter's
dirty flag would have CHANGED upload behavior — scope guard held).
The gol device_ self-submit is DEFERRED BY NAME: folding it into the
frame encoder reorders GPU submission — not behavior-safe without
the rig. THE REST OF m3 AWAITS A RECIPE STAMP: the reach-order sweep
self-blocks on services-last (censused in audit/REBUILD0_M3_RECIPE.md),
the table seam keeps keyhole faces by standing stamp, pair merges are
unspecified, and theory v3 §7 re-frames A3 as pulled-per-joint — the
lens governs where framing differs; Q1-Q3 put the fork to Jean.
GATES: glaw1 GREEN full + minimal; score census GREEN; clear_spheres
move token-identical; sentinels 148/5 unchanged; zone 18 == 18;
encodings clean.

**m3 — THE RULING (the lens overrules the stamp; PRECEDENT).** Jean's
ruling on audit/REBUILD0_M3_RECIPE.md Q1-Q3: v3 §7 GOVERNS — the
seventeen-module sweep is PARKED; dissolution is a toolbox pulled per
joint with a demo-axis justification named at pull time. THE
PRECEDENT, recorded explicitly: the stamped stage order was AMENDED
BY THE LENS (strata law) — stamps bow to the lens, including the
stamp that ordered the sweep. Q2 is moot and recorded as the PROOF:
both poisons (services-first inverts pull-value; c-through-deps fakes
the dissolution) being real is why the era-shape was wrong. Q3: PAIR
MERGES DEFINED AND PARKED — a merge collapses hpp+inl into one
pre-class header, lawful ONLY at zero keyhole residue in any
signature INCLUDING dispatch rows (today's eligible set: likely
empty); the zone architecture remains standing law; no merge rides
m3.

**m3a — AMENDED per the ruling.** The four GPUState&-parameter
functions are restored byte-identical to their pre-m3a forms and
RECLASSIFIED: bypass -> DEPS-FORM PRECEDENT (the deps form's first
citizens, born-converted, callable without the complete Cartridge);
their banners now say so, and say they are not to be re-shaped
keyhole-ward. KEPT from the first cut: the staged setters
(stage_pier_count / stage_placement_patch_count / stage_lod_pawn /
stage_floater_coordination — named verbs for the poke-then-flush
idiom, deliberately without configDirty_) and the gol device_
self-submit PARKED with its hazard named (submission-order
arbitration is the rig's).

**m3b — THE S2 BOUNDARY FACE (v3 §7's first toolbox pull; D1 is the
puller).** tile_world's boundary is now FOUR DECLARED FACES, each
carrying the invariant in its banner (callable WITHOUT the complete
Cartridge — a generated-once surface cast could implement them):
F1 estimate_terrain_height + F2 terrain_tile_warm (pre-existing,
invariant stated); F3 tile_apply_spawn_mult (APPLIES onto the
accumulator — the two multiplies stay separate and ordered, bit-
identity under FP non-associativity); F4 tile_archetype (BOOL-OUT —
the miss default stays with the caller: the pace site keeps 1.0, the
placement sites keep archetype 1; the naive default-1 form would
have silently changed cold-tile pace to PHOTO_PACE_BY_ARCHETYPE[1]
= 0.8). Five consumer sites converted (gol select, gallery pace,
gallery place, the P11 preamble template, evaluate_spawn_gate);
zero raw tileCache_ reads remain outside surface/.
GATES: glaw1 GREEN full + minimal; score census GREEN; the four
restored functions byte-identical vs 3701b5b; sentinels 148/5;
zone 18 == 18; encodings clean.

**m4 — THE CHANNELS (D5 executed; disclosure rule honored).** Seven
conversions, all token-identity gated: (1) the input fan's pawn pair
— keys 2/3 now speak through pawn command doors (toggle_aura_height /
toggle_aura), matching the orbs/agents/cube pattern; (2) mood's aura
force-off speaks through apply_aura_mood_policy — the pawn's own door,
BYTE-IDENTICAL semantics (the recon's mood_allowed-flag idea would
have changed behavior — preference persistence across moods — and was
set aside under the disclosure rule); (3) lights_dirty RE-HOMED
entities_state_ -> mood_state_ (mood was both producer and consumer;
the organ was wrong, not the channel) — the peer-write is erased;
(4) the anchor ribbon's immediate promotion folds into
promote_ribbon_to_rendered (the trailing rendered_slot write lives
with its owner); (5) the tile cache gains its authoring doors —
ensure_tile (ticks terrain tokens) / ensure_tile_padding (does NOT —
padding must not advance the terrain's memory) / reset_tile_cache /
reset_terrain_memory; (6) evaluate_theme_envelope stores its own
result + reset_theme_envelope (the teardown reset); (7) the INT32_MAX
poke is NAMED: request_recenter, the streaming conductor's door,
called by the radius command.

THE CHANNEL REGISTRY (standing, post-m4): FLAG CHANNELS —
mood_state_.portals_dirty [entities/pipeline/mood/patch -> mood];
world.ground_entries_dirty [owners -> render()];
mood_state_.lights_dirty [mood/patch -> mood; re-homed];
mood_state_.back_portal_pending [spine -> mood via patch timing];
gol_state_.mood_allowed [mood -> gol]; pawn.aura_cfg_dirty
[pawn/input-doors/patch -> render()]; *_mesh_gen_pending [owners ->
preparers]; world.pier_count_dirty [pier writers -> flush].
COMMAND DOORS — input -> {orbs x4, agents x4, cubes x4, pawn x2,
request_mood_transition, request_recenter, toggle_sky_mode(D9-gated),
set_render_radius}; mood -> {force_spawn_portal_arch (THE CHANNEL),
commit_ribbon + promote_ribbon_to_rendered, place/clear_wall_paintings,
configure_orbs, apply_aura_mood_policy}. DECLARED ROOT DOORS —
possession (agents -> player_.possessed_slot), P8 (pawn ->
player_.aura_presence). THE F6 SOCKET stays RESERVED: when a driver
must address a body it does not own by synchronous command, the
addressed-intent socket (v3 §9 Act II, §13) is where it routes;
nothing today requires it.
GATES: glaw1 GREEN full + minimal; score census GREEN; relocations
token-identical (tips, tile doors, toggles, promotion, policy door);
sentinels 148/5; zone 18 == 18; encodings clean.

**m5 — THE WITNESS (declared and toothed).** The witness census
(recon R4) already proved the boundary law-abiding — m5 makes it LAW:
THE WITNESS CONTRACT is written at the record (PlayerState,
contracts/spine_state.hpp, citing v3 §11 and §9 Act III — possession
is re-anchoring; the camera is what we control and HAS NO CPU MIRROR;
there is no readback_y; neither is to be invented), and the score
census gains DIRECTION W, the sole-author teeth: the readback trio
writes only at the spine (P5 harvest + teardown reset + portal
consume), possessed_slot only behind the agents door, aura_presence
only in pawn.inl (P8). A module write to any guarded field is now a
census RED. The consumers-through-deps clause rides the parked sweep;
the S2 faces (m3b) + this contract carry the boundary meanwhile.
GATES: glaw1 GREEN full + minimal; score census GREEN incl. W.

**m6 — THE SKY COMES HOME (Option A, per D6; the campaign's last
movement).** The rider state leaves the witness record: RibbonState
gains the SkyFlight fixture (mode / mode_prev / yaw_eased) — the
mount was always ribbon-owned (RibbonHead.mount); this completes the
ownership with a single CPU owner. The SNAP-1 resync moves from the
score to the ribbon tick's TAIL — O-1 holds BY CONSTRUCTION now (the
head advances above it, dispatch_compute follows the tick in the
score, queue writes apply in submission order); update()'s signal
block ships all-neutral zeros including the mode word (the tail
resync is the sole author). F8 retargets to the fixture and keeps its
D9 gate; with ribbon off, the gated tick eliminates the resync too
and the sky words hold zeros forever — the ribbon-less contract,
closed both sides. PlayerState sheds its last SEAM[ribbon:sky-mode]
tenants; the witness contract text updates to past tense; the two
head readers leave the score census whitelist with the resync.
world.wgsl and state.hpp: untouched, as priced (dumb wires).
GATES: glaw1 GREEN full + minimal; score census GREEN (A, B, W);
player_.sky greps to zero; sentinels 148/5; zone 18 == 18; encodings
clean (ribbon.inl BOM intact).

## REBUILD-0 — THE CAMPAIGN CLOSES (m1-m6 landed; m7 NO-GO per D8)

The derivation campaign is cut whole: the lens landed (v3 + the demo
contract), the spine's types graduated (m1), the score became prose
with the census standing guard (m2, P1/P2 dead), the dissolution
became a pulled toolbox with the S2 boundary as its first pull (m3
per the ruling — the lens amended the stamp, precedent on record),
the channels converted with the registry standing (m4), the witness
contract is law with census teeth (m5), and the sky came home to its
single owner (m6). Standing throughout: PRIME INVARIANT, the score
census (A/B/W), glaw1 at both demo sentences, the minimal witness.
AWAITING THE RIG: RC-1/RC-2 pixel arbitration (m2's two reorders —
failure reverts per the stamped policy) and the two-demo run over
the campaign's head. Parked with names: the seventeen-module sweep
(pulled per joint), pair merges (defined, zero-keyhole-residue law),
the gol device_ self-submit (submission-order hazard), m7 packers
(assert-first rider is the standing precondition), and R7's three
levers (chain multiplier, lattice bake, pipeline blob caching).

## DISSOLVE-1 — THE MACHINE FACE AND THE MERGES (Phase R landed; ONE STOP)

Puller on record: THE COVENANT (comprehension) — Jean's sentence,
now verbatim in v3 §7 (d0 rides this commit): "The .inl were not
desired from the beginning." The m3 poisons are cured by ORDER — the
services face (MachineCtx) cuts first. Phase R's face census is ONE
report at audit/DISSOLVE1_FACES.md: R1 the MachineCtx composition
(organ-named reference members — the template keyhole's escape
clause executed: deduced C becomes MachineCtx with byte-identical
bodies; const trio tiles/time/player load-bearing; the DUAL-ENTRY
RULE names the four doors that template-ize), R2 the finalized deps
table (post-campaign, const-encoded, witness clause folds in with a
Direction W extension), R3 the merge forecast (THE COHORT LAW — each
merge lands with a topological include-order proof; the BOM plan —
the four .inl BOMs die with the .inls, renderer.hpp remains the lone
deliberate BOM; patch_system merges last), R4 the holdouts (the gol
self-submit DISSOLVES BY DECLARATION on GolDeps — submission-order
analysis first-hand: the derive pass must keep executing before the
frame's agent kernels, so the refactor stays forbidden and the
handover is declared instead; rows with behavior-changing forms:
NONE). FULL STOP — d1 cuts only after the stamp (S1-S5).

## DISSOLVE-1 — PHASE I (the stamp: S1-S5 resolved; the face cuts)

**d1 — THE MACHINE FACE.** MachineCtx lands in the dispatch contract
(contracts/entity_types.hpp): sixteen organ-named reference members,
the const trio (tiles/time/player) as the arrow law's compiler teeth,
bound once at the root (the constructor's one init list — the default
ctor retires). The conversion: 228 signature lines Cartridge* ->
MachineCtx* (rows, adapters, machine verbs, the 12 spine wrappers, 9
patch_system verbs, 13+9 entities rows, the family row sets), 36
call-site context-args where Cartridge-world callers now hand the
face (&machine_ctx_ / &c->machine_ctx_), bodies BYTE-IDENTICAL by the
organ-naming key — the diff classifier proved the shape (signature /
context-arg / face-block / comments; zero unexplained lines).
run_spawn_preamble UNCHANGED (deduced C now lands on MachineCtx —
the escape clause executed textually). THE MACHINE ROOM READS ZERO:
grep over machine/ finds no Cartridge* in code — the table seam's
keyhole is RETIRED (the K2 stamp updated by its own escape clause).
DISCLOSED per S2: the dual-entry doors landed CONCRETE on the face,
not as deduced templates — the census showed all their reaches fit
MachineCtx whole, so callers pass the context and zero new template
machinery was minted (same access outcome; the simpler mechanism);
also derive_finite_radius turned out never to take the keyhole — the
dual set was three (commit_ribbon, load_authored_textures,
pick_portal_mood), not four. The S5 ruling (GolDeps gains the
declared device member) executes at d2-gol.
GATES: glaw1 GREEN full + minimal; score census GREEN (A/B/W);
sentinels 148/5; zone 18 == 18; encodings all match HEAD; the diff
classified line-by-line.

**d2-themes + d3 MERGE #1 — THE .INL ERA'S FIRST RETIREMENT.**
population_themes: the envelope takes the face (evaluate_theme_
envelope's keyhole param becomes MachineCtx*; the DIAG census dump
rides it; the stream caller hands the face) — residue hits ZERO in
every signature, no dispatch rows, and the parked definition meets
its condition. THE MERGE: hpp + inl collapse to ONE pre-class header;
the .inl DELETED; the zone include retired — ZONE CENSUS 18 -> 17,
the .inl census reads 17. THE COHORT PROOF (first exercise of the
law): every callee is declared earlier in the cohort except the
DIAG-gated census dump, fwd-declared under its own flag with the
disclosure written at the site. THE KEYHOLE INCLUDE IS RETIRED with
the merge — nothing in the module names Cartridge (the two grep hits
are the disclosure comments). Encoding: clean UTF-8/LF, byte-verified
(both halves were clean; the BOM plan untouched by merge #1).
GATES: glaw1 GREEN full + minimal; score census GREEN (A/B/W);
zone 17 == 17; sentinels 148/5. The form is proven; the remaining
d2 conversions and merges follow it module by module in the stamped
order (tile_world next).

## DISSOLVE-1 — BATCH A (the low-risk floor; → Rig Boundary A)

Four .inls retired (zone 16 -> 13), the batch cadence's first block.
**A1 family_dispatch**: no struct, no pair — SEAM[spine:owns] work that
takes the Cartridge mesh-wrapper static addresses; inlined into the
composition root's post-class zone at its include point, .inl deleted.
**A2 deps conversions**: TileWorldDeps {const WorldState&, const
MoodState&, GPUState&}, SphereDeps {const TimeState&}, PawnDeps
{PlayerState&, const TimeState&, GPUState&, Renderer&} — requirements
faces made literal; Cartridge* -> XDeps* with byte-identical bodies
(the organ-naming key); the root binds each once in the ctor init
list. PawnDeps.player_ is non-const (the P8 door); the score census
gains DIRECTION W (deps): a writable PlayerState& outside pawn/agents/
input is a RED, teeth-tested. **A3 merges**: the three pairs meet the
parked definition's condition and collapse to one pre-class header
each, .inls deleted. COHORT PROOFS: pawn after renderer.hpp (Renderer/
GPUState + PATCH_CELL_SIZE); tile_world after patch_system.hpp
(WorldState + PATCH_EXTENT/PREGEN_RADIUS) and themes/mood; spheres
after entity_pipeline.hpp (generic funnels) + spawn_engine.hpp (the
preamble template). The merge moved pawn's aura_presence write
hpp-side — the census W file-map followed (pawn.inl -> pawn.hpp),
caught by the gate as designed. Merged files clean UTF-8/LF; the BOM
census now reads renderer.hpp + the four still-unmerged .inls
(entity_pipeline, render_passes, agents, ribbon — they die in B/C).
GATES per commit: glaw1 GREEN full + minimal; score census GREEN
(A/B/W + W-deps); zone 13==13; sentinels 148/5; encodings verified.
AWAITING RIG BOUNDARY A: full golden (RC set standing) + minimal
witness (boot line, no leaks, sky zeros).

## DISSOLVE-1 — BATCH B (the mid cluster; → Rig Boundary B)

Five more .inls retired (zone 13 -> 8). CONVERSIONS: OrbsDeps,
AgentsDeps (player_ non-const — possession door), CubeDeps, GolDeps
(+ S5: the DECLARED wgpu::Device& handover, SEAM[gol:derive-submit] —
immediate mid-render submit, the refactor stays forbidden); entities'
two residual functions (teardown_entities, force_spawn_portal_arch)
took MachineCtx whole (no EntitiesDeps). teardown_gol gained a
GoLState& param so GolDeps stays a pure external face. All five read
zero Cartridge in code. MERGES: each collapsed with its cohort proof.
Two findings worth the record: (1) entities' merge exposed a genuine
bidirectional coupling the .inl zone had hidden — spawn_engine.hpp
names ActiveColumn (entities vocab) in a declaration while entities'
bodies need the machine's preamble template; it CLOSED with ONE
forward-decl (struct ActiveColumn;), the minimal cohort proof, not a
cycle. (2) PATCH_CELL_SIZE was mis-homed in gol_zones.hpp (it is
PATCH_EXTENT/16, patch vocabulary, consumed by pawn's aura too); the
gol merge would have moved it after pawn, so it graduated to
patch_system.hpp beside PATCH_EXTENT — a clean vocabulary relocation.
The census W file-map followed both merges (possessed_slot, force-spawn
door). BOM census: renderer.hpp + entity_pipeline.inl +
render_passes.inl + ribbon.inl (the three still-unmerged BOM .inls die
in C/D). GATES per commit: glaw1 GREEN full + minimal; score census
GREEN (A/B/W + W-deps); zone 8==8; sentinels 148/5; encodings verified.
AWAITING RIG BOUNDARY B: golden + minimal witness; zone count 8.

## DISSOLVE-1 — BATCH C, part 1 (ribbon + gallery; → Rig Boundary C)

Two more .inls retired (zone 8 -> 6). CONVERSIONS: RibbonDeps (12
members — the widest deps face yet: GPUState&, and const views of
time/tile_world/player/input/world/mood/visual_canvas + four const
TargetBinding&, the ribbon's four aim channels); GalleryDeps (GPUState&,
Renderer&, const views of world/tile_world/ribbon/player/mood + two
const float(&)[3] — sunDirection_, clearColor_). teardown_ribbon and
release_finite_ribbons gained a RibbonState& param; teardown_gallery a
GalleryState& — the teardown-verb pattern holds, the deps stay pure
external faces. Both read zero Cartridge in code. MERGES: ribbon
collapsed after visual_canvas.hpp (it derefs VisualCanvas + TargetBinding
whole); gallery after ribbon (GalleryDeps holds a const RibbonState&).
Two doors worth the record: (1) gallery's dual-entry pair
(load_authored_textures, load_authored_image_to_staging) is called from
BOTH the MachineCtx commit row AND GalleryDeps' place_wall_paintings —
GalleryDeps carries no machine_ctx_, so both took the S2 deps-form
(explicit GPUState& gpu), the context-agnostic door, five call sites
retargeted. (2) gallery-merged names stbi_load; stb_image.h had been
cartridge.hpp's terminal include (after gallery in the cohort), so it
moved INTO gallery.hpp — the module names its own external dep, and
cartridge.hpp sheds its last body-include. The census W file-map needed
no change (readback trio still pawn/agents; gallery authors no witness
slot). BOM census now reads renderer.hpp + entity_pipeline.inl +
render_passes.inl (ribbon.inl's BOM died with its file; the last two die
in the machine-natives ruling + Batch D). GATES per commit: glaw1 GREEN
full + minimal; score census GREEN (A/B/W + W-deps); zone 6==6; sentinels
148/5; encodings verified. AWAITING RIG BOUNDARY C: golden + minimal
witness; zone count 6.

Part 2 (input + machine natives) is STOPPED at the disclosure rule —
two findings reported to Jean for a ruling before any cut:

  FINDING C-1 (machine natives — the before/after-entities split). The
  merge of spawn_engine.{hpp,inl} and entity_pipeline.{hpp,inl} does NOT
  close. The two files each hold a genuine two-tier shape the .inl split
  had been quietly expressing: a DECL tier that must precede entities
  (spawn_engine.hpp's run_spawn_preamble<C,ActiveT> template + entity_
  pipeline.hpp's generic_* family, consumed by entities.hpp at the family
  recipes — run_spawn_preamble called 3×, generic_select/place/commit and
  rescale_to_rolled_target throughout), and a BODY tier that must FOLLOW
  entities (spawn_engine.inl's build_arch_mesh_params / build_column_mesh_
  params / update_entity_draw_visibility and entity_pipeline.inl's frame
  bodies — 29 c->entities_state_ derefs across arches/columns/antennas/
  pyramids, needing EntitiesState complete). A single pre-class header
  cannot be both before and after entities in a linear include model, and
  the escape hatch fails: the decl tier is a TEMPLATE, which cannot be
  forward-declared and instantiated later. This is the one shape the
  Batch-B ActiveColumn forward-decl trick does NOT reach. The RULING owed:
  relocate the body tier (build_*/update_entity_draw_visibility + entity_
  pipeline's frame bodies) into entities.hpp post the EntitiesState def —
  or accept spawn_engine.inl + entity_pipeline.inl as the NAMED, REASONED
  .inl remainder (the welded-four is already stamped machine-side by the
  dispatch seam).

  FINDING C-2 (input defers to Batch D). input.inl calls request_mood_
  transition(c, MOOD_*) 5× — a mood-owned door whose signature is still
  void request_mood_transition(Cartridge* c, uint32_t) (mood converts in
  Batch D). An InputDeps built now would have to hand a Cartridge* into
  that door — a faked conversion carrying the keyhole through. input
  therefore rides Batch D, after mood's door sheds Cartridge*. A disclosed
  census re-rank, not a workaround.

## DISSOLVE-1 — BATCH C, part 2 (the B ruling executed; → Rig Boundary C)

JEAN'S RULING (B): "InputDeps is input's own organs plus its true
reaches; the command fan routes through the existing m4 doors; the
target organs live at the call sites, not in the struct; the F6 socket
stays reserved for a real addressing need. Then input, spawn_engine,
and entity_pipeline all merge with their cohort proofs." Three commits
executed it; three more .inls retired (zone 6 -> 3).

COMMIT 1 — InputDeps, the driver's face. Eight members: the driver's
own organs (inputState_/keys_/mouse_) + its true reaches (player_ the
anchor toggle, world_state_ the radius command, ribbon_state_ the sky
fixture, gpuState_ the freeze toggle + fpv wire, wgpu::Device& the
queue fetch — the S5-style declared handle). The command fan's TARGET
organs are NOT members: on_key_down takes them as organ-named
PARAMETERS (pawn/orbs/agents/cubes state+deps pairs + the transition
channel trio), and the root's on_input addresses the fan's bodies per
event — the driver law made literal (v3 §9 Act I: a driver writes
intents through bodies it does not own). THE F6 SOCKET stays RESERVED.
The two m4 doors input calls shed their keyholes deps-form (the m3
precedent class): request_mood_transition(TransitionPhase&,
PortalDestination&, MoodState&, const WorldState&, uint32_t) and
request_recenter(WorldState&) — the C-2 finding resolved not by
waiting for Batch D but by converting the DOORS' faces now; mood's and
patch_system's bodies otherwise untouched.

COMMIT 2 — InputState graduates + input merges. struct InputState
graduates to contracts/spine_state.hpp (the m1 pattern: type at the
contract tier, instance at the root) — it is the driver's intent
CHANNEL, read by the spine's signal fill and the ribbon's sky flight,
so it must precede ribbon in the cohort; KeyState/MouseState stay with
input (the driver's private organs). input.hpp+inl collapse to ONE
header at the cohort tail (after ribbon — toggle_sky_mode derefs
RibbonState.sky); the GLFW include + fallback #defines ride the module
(preprocessor is namespace-blind). Census F8-door file-map followed.

COMMIT 3 — the machine natives merge; the C-1 cycle closes by the
CONTRACT SPLIT. A new contract is born — contracts/spawn_services.hpp
(entity_types' sibling, the machine's decl tier): the spawn-service
decls, the preamble + rescale template DECLARATIONS, the generic_*
decls, the boundary DTOs (SpawnGatePreambleResult / PositionResult /
SpawnPreamble), the arch vocabulary (ArchIdx / ArchTierRow /
ARCH_TIERS), MIN_SEPARATION, GLOBAL_ENTITY_DENSITY (the sweep caught
gol + gallery reading it pre-tail — the census's named-set grep had
missed it), and the ActiveColumn fwd (the Batch B proof, re-homed).
With the early tier gone, spawn_engine and entity_pipeline each merged
to ONE header at the cohort tail; entity_pipeline's BOM died with its
file. THE BINDING LAW, named at the contract: an inline function or
template declared before its callers may be DEFINED later in the same
TU — templates instantiate at end-of-TU — the mechanism the .inl zone
always relied on, now written down. THE CORRECTION owed to the record:
the C-1 report claimed "templates cannot be forward-declared" — WRONG
for a single-TU design; the fix IS the ruling's mechanism. The P11
seam note followed (C deduces MachineCtx; the template keyhole is a
doorway now).

The .inl census reads THREE, all Batch D: mood.inl, render_passes.inl,
patch_system.inl. BOM census: renderer.hpp + render_passes.inl. GATES
per commit: glaw1 GREEN full + minimal; score census GREEN (A/B/W +
W-deps); zone 6==6 / 5==5 / 3==3; sentinels 148/5; encodings verified.
RIG BOUNDARY C: CLEAN (Jean, golden run — "All is good with the build").

## DISSOLVE-1 — BATCH D (mood, render_passes, patch_system; d3 COMPLETE)

Five commits; the last three .inls retired (zone 3 -> 0). The
pattern-book from A/B/C carried the whole batch: the B law (fan
targets at the call sites, never in the struct), MachineCtx-whole
where the face covers it, deps-form doors, the contract split for
two-tier orchestrators.

MOOD (2 commits). MoodDeps — the atmosphere author's face, 12
members: the mood organ, a const world view, the realization pokes
(GPUState + the frustum-cull flag on Renderer), the gol mood gate
(the m4 flag channel), a const entities view (the portal-array upload
reads arch positions), the sun/clear channel (three float(&)[3]), the
CPU light + portal staging arrays, the back-portal anchor. The fan —
ribbon/orbs/gallery/pawn + the machine face — rides apply_mood's tail
parameters; the force-spawn internals carry the machine face down
(the arch's owner writes through its own door, so the deps view of
entities stays const). THE GRADUATION had one course correction worth
the record: the first-cut home (mood_constants) closed a CYCLE with
the demo sentence — the demos include mood_constants for the Mood
IDs, and MoodState's boot default reads DEMO — so the DEMO-reading
tier (MoodState / CeilingType / MoodProfile / MOOD_TABLE + the
request door decl) rides contracts/spine_state.hpp instead: the
InputState precedent, the mood organ was ALWAYS spine-resident with
its transition machine (K4), and demo.hpp precedes spine_state in the
cohort (the patch_system DEMO.seed precedent). mood.hpp+inl collapsed
to ONE header at the tail; the census file-map followed (three
delegated-gate sites mood.inl -> mood.hpp — the gate caught the stale
map again, as designed).

RENDER_PASSES (1 commit). The realization conductor stands on THE
MACHINE FACE whole — its nine organ reaches are all machine members,
byte-identical through the face. The three reaches outside it ride
the call site: the CPU spot-light array (const, shadow pass), the
clear color (const, main pass), the orbs pair (render_orbs — the one
sibling door). Merged BEFORE mood in the tail (mood's spot-light
applier calls compute_spot_light_vp) — no graduation needed at all.
render_passes.inl's BOM died with the file: the BOM census read
renderer.hpp ALONE from this commit on.

PATCH_SYSTEM (2 commits — LAST, alone, per the cadence). Fifteen
keyholed verbs convert to MachineCtx* (bodies byte-identical; the
fifteen &c->machine_ctx_ handoffs collapse to c). The reaches outside
the face ride the call sites — and the COMPILER found the one the
census grep missed: the machine face's tile_world/themes views are
CONST (the arrow law's stamped teeth), but patch_system is the
surface's LIFECYCLE owner and mutates both through the m4 doors
(ensure_tile / reset_tile_cache / reset_terrain_memory /
evaluate_theme_envelope / reset_theme_envelope). The stamped const
face stayed untouched; the writable organs thread down from the
spine's call sites with the tile deps, the mood deps (the back-portal
door), and the driver's intent organ (the movement budget read).
THEN THE LAST MERGE: contracts/surface_services.hpp is born
(spawn_services' sibling — WorldState, the patch registry vocabulary,
PatchSystemState, budgets + visibility, the surface service decls),
and surface/patch_system.hpp became the machine's bodies whole at the
cohort tail's end.

## DISSOLVE-1 — d4 CLOSE-OUT (the .inl era ends)

THE COVENANT IS KEPT (v3 §7, the second puller): the .inl were not
desired from the beginning, and the census now reads what the theory
asked for —
  · .inl files: ZERO (was eighteen).
  · The post-class zone: EMPTY OF MODULES — eighteen impl includes
    once stood at that file scope; what remains is FAMILY_DISPATCH,
    the spine's own table, which was never a module.
  · Cartridge* keyhole residue in module code: ZERO files. The
    keyhole survives only as keyhole.hpp's forward declaration and
    the root's own class.
  · BOM census: renderer.hpp ALONE (the lone deliberate carrier).
  · File census: 34 hpp / 0 inl — the module impl tier (18 files)
    is GONE; the contracts tier grew by two named boundaries
    (spawn_services, surface_services — the machine's and the
    surface's decl tiers).
  · The cohort, readable in one pass of cartridge.hpp's includes:
    contracts (roster/demo/seed_utils/ground_architecture/
    entity_types/spawn_services/mood_constants/spine_state/
    floater_vocabulary) -> state -> themes -> surface_services ->
    tile_world -> entities -> agents -> cube -> spheres -> renderer
    -> pawn -> orbs -> gol -> visual_canvas -> ribbon -> gallery ->
    input -> render_passes -> mood -> spawn_engine -> entity_pipeline
    -> patch_system -> THE CLASS -> FAMILY_DISPATCH.
GATES at close: glaw1 GREEN full + minimal; score census GREEN
(A/B/W + W-deps); sentinels 148/5; encodings clean UTF-8/LF.
AWAITING RIG BOUNDARY D (the campaign's last): golden + minimal
witness; zone count 0. Lane 3 (the pipeline blob cache) is next in
the queue after the boundary.

## PANEL-0 — THE AUTHORING-SURFACE RECON (report-first; ONE STOP)

Read-only. The campaign that makes demos easy to author at dev time —
a pieces × demos MATRIX (existence) + per-module PARAMETER PANELS
(tuning), both compile-time, edited before any request reaches Claude.
Report: audit/PANEL0_RECON.md. Five movements sized; two parallel
scouts (the world.wgsl call-tree for R1, the per-module constant
readiness for R4) + the spine's own reads (pawn fusion, the existence
surface, terrain in full).

FINDINGS. R1 (the honest perf question, settled first): the ~34s boot
is world.wgsl recompiled every launch (no blob cache exists); the hot
locus is behavior_player_controlled's 7 walker-chains × 24 lattice
unrolls (pawn_ground_resolve + terrain_normal_at, world.wgsl:5438/5406)
— and the cost is BODY-side (terrain-snap/step-climb/tilt), NOT the
player/driver, NOT the gait. THE SEPARATION HELD: the pawn split is
COMPREHENSION; boot time is untouched by a CPU rename alone — the perf
lever is a BODY-chain pipeline extraction or a lattice-bake (R7's
levers), and the blob cache is the orthogonal warm-boot lever. R2: the
pawn is three near-separate things wearing one name — the PLAYER
(PlayerState.possessed_slot, the anchor + witness), the BODY (agent
slot 0, FOUNDATIONAL/ungated), the gait (behavior 0, WGSL); the split
is low-cost, no weld resists, the rows are always-green. R3: 19 roster
bits, DemoConfig{roster,seed,boot_mood}, two demos, ONE dependency edge
(THE FIRST EDGE transitions⟹portal); the matrix = pieces × demos grid
preserving the constexpr fold, grow-by-pull. R4: terrain's dials live
in FOUR homes (surface_services/tile_world/population_themes C++ +
world.wgsl's bands/palette/voice/mesh), each gathered within, scattered
across — the panel challenge is the four-home scatter + the C++/WGSL
split (recommend v1 = consolidate C++ + index WGSL; the uniform bridge
is a named sub-movement). Non-terrain: 9 GATHERED, 3 MIXED (mood the
biggest scatter), 2 SCATTERED (input trivial, render_passes the "look"
dials); entities + ribbon are the exemplars. R5: p1 pawn decomposition
(comprehension; perf per R1, separate) → p2 the matrix (a terrain-only
sentence becomes a new standing witness) → p3 the panels (terrain
first). FULL STOP: four stamp questions (matrix shape, terrain panel
scope, whether a fourth perf movement, p1's roster form). p1 cuts only
after the stamp.

## PANEL-0 p1a — THE POINT AND ITS TWO HOSTS (the kite untouched;
## free-fly born; the first parameter panel)

Jean's correction, ratified mid-handoff (the point model): THE POINT
IS THE PARENT — the anchor IS a point; the point owns the bubble; the
camera is its permanent witness but does not own it; the point is
HOSTED, like a spirit, wherever context demands. The recon's finding
held through the cut: the code already lived this model —
possessed_slot was always a host pointer restricted to agent slots;
sky mode is the chain alive (input -> ribbon -> pawn -> camera, the
intent channel already host-routed at ribbon.hpp's head steering);
the damped aim point (tau 0.30) IS the kite string; and the terrain
rule's three values all name existing realizations (the walker
resolve, the flyer min-clearance clamp, skip). Three commits:

COMMIT 1 — contracts/point.hpp: PointHost {PAWN, CAMERA},
PointTerrainRule {NONE, SOFT_FLOOR, SNAP}, the host table, the bubble
declared whole (sensors dormant; live per-host sensing is p1b, by
pull), the instance (point_) at the root beside the witness record.
Structure complete first; no behavior.

COMMIT 2 — the two hosts. Config gains point_host + point_fly_speed
by PIGGYBACK on the lod-pawn pad pair (struct stays 400 bytes — the
possessed_slot precedent; C++/WGSL mirrors in lockstep, the size
assert the standing proof). The PAWN host is byte-untouched: the kite
path (damped aim, orbit, every clamp) unchanged; the one bridging
edit is the player kernel's input-coupling condition gaining
"&& !point_camera_hosted()" — value-identical when the pawn hosts.
The CAMERA host (free-fly) is an early branch in update_camera: mouse
rotates (FPV elevation range), W/S ride the look direction, A/D
strafe the ground plane, pan translates the view plane; TERRAIN RULE
NONE (every clamp skipped — the revision camera); PAWN_SPEED select
fallback. The pawn idles by construction (its coupling unrouted →
the existing zero-velocity arm): bubble sensors dormant, portal path
untouched, slot-0 machinery untouched — exactly the amendment's
resolution. ONE INTENT CHANNEL, host-routed (the sky-mode precedent):
arrows author move for the pawn host (byte-identical arm), the new
W/A/S/D held-keys for the camera host; key 4 is the driver's host
toggle (ungated — the camera always exists); InputDeps gains the
point.

COMMIT 3 — CameraControls, the campaign's FIRST parameter panel
(deliberately minimal; the p3 FORM TEST): LOOK_SENSITIVITY (was
on_mouse_move's inline 0.005f — value identical) + MOVE_SPEED (wired
to config.point_fly_speed at boot; the pawn's walk speed untouched);
deferred growth named in the block, not carried. input.hpp's
SCATTERED panel grade from the recon is retired.

DISCLOSED EDGES (dev-tool caveats, no engineering): the sun/shadow VP
stays kite-coupled to the pawn (fly far and you leave the shadow
window — the shadow follows the body's neighborhood; the point
witnesses it); sky mode + free-fly both consume the intent channel
(the combo steers both — a dev-tool corner); pawn-aura and the sphere
forcefield remain body-centered (correct: the body's effects live
with the body).

GATES per commit: glaw1 GREEN full + minimal; score census GREEN
(A/B/W + W-deps); sentinels 148/5; encodings clean. NO WGSL COMPILER
exists in this rig — the four WGSL edit sites were review-verified
against the file's own idiom; the runtime proof is Jean's.
AWAITING THE RIG: the PIXEL-IDENTICAL KITE (demo=full, same seed, two
runs — pivot distance, mouse-rotate-around-pawn, indistinguishable
from pre-p1a head) + the FREE-FLY PROBE (key 4: mouse looks, WASD
flies camera-forward/strafe, terrain renders, clips freely, no body
influence, no portal fire; key 4 again returns the kite) + the panel
retune check (both dials visibly change feel). p1b (tickable body +
portal/bubble re-seat) stays queued by pull; p2 (the matrix) next on
Jean's word.

## PANEL-0 p1a-fix — THE UNIVERSAL MOVE CHANNEL (WASD binds; arrows
## retire; the host owns its mapping)

Rig Boundary p1a: the kite PIXEL-IDENTICAL (Jean); key 4 switches
hosts correctly; WASD moved nothing because WASD WAS NEVER BOUND —
the camera host read a channel nothing wrote. The fix, per Jean's
ratified principle: ONE move-intent channel, authored by W/S/A/D,
consumed by WHOEVER HOSTS THE POINT under that host's own constraint
AND mapping — THE CONSTRAINT-AND-MAPPING IS THE HOST'S BEHAVIORAL
IDENTITY (succession note for v3 §11, joining the point model; sky
mode's arrow-routing special case retires into the general rule).

THE CENSUS CAME BACK EMPTY: arrows existed in code ONLY as the eight
movement key cases (input.hpp) — no non-movement arrow use anywhere;
nothing kept. The ribbon steers via the CHANNEL (RibbonDeps'
inputState_), not via arrows — its "arrow" mentions were comments,
now corrected to the channel truth.

THE CUT: W/S/A/D author keys_.forward/backward/left/right (the same
fields — arrows' cases replaced, the p1a fly_* fields retired); with
ONE key set the p1a CPU routing branch COLLAPSES — update_movement_
intent's fold returns to its pre-p1a body byte-identical, which
satisfies the narrow pixel gate BY CONSTRUCTION (same fields, same
fold, same signal; pixels cannot tell which physical key authored the
intent). Consumption stays host-routed downstream exactly as cut in
p1a: the pawn kernel's point_camera_hosted guard, the camera's fly
branch, the ribbon's sky steering — zero WGSL changes this fix. Host
mappings recorded: pawn camera-relative full-directional SNAP
(unchanged math); camera camera-relative full-directional NONE;
ribbon forward-biased grammar UNCHANGED (a ribbon that could reverse
and strafe wouldn't be a ribbon — W/S throttle, A/D yaw, S is no
thrust).

SEPARATELY QUEUED (held for p1b, Jean's second rig point): the
SHADOW-VP RE-ANCHOR to the point — the distinguishing rule for the
record: VIEWPOINT-SERVING resources follow the POINT (shadow-VP);
ENTITY-EMANATING fields follow the BODY (aura, forcefield). Touches
the pixel-gated shadow path; pairs with p1b's point-following work.

GATES: glaw1 GREEN full + minimal; score census GREEN (A/B/W +
W-deps); sentinels 148/5; encodings clean; arrows ZERO in code.
AWAITING THE RIG: pawn host — WASD moves the pawn exactly as arrows
did (the narrow pixel gate), arrows do nothing; key 4 — WASD flies
the camera (the probe now passes); sky mode — the ribbon steers by
its own grammar via WASD, feel unchanged.

## PANEL-0 p1a-fix2 — THE MUSIC-KEY QUARANTINE (the keyboard is the
## world's by default; the split kept behind a build flag)

The rig's WASD-does-nothing was NOT a the_board bug: the harness's
input router (incubator.cpp / incubator_dual.cpp, is_music_key) sent
every QWERTY letter A-Z to the ANALYSIS cartridge, so W/A/S/D never
reached the_board — arrows (non-letters) fell through to render, which
is why they alone worked. The analysis canvas (canvas_1) even IGNORES
on_input (its note source is the DAW), so the letters were routed to a
handler that drops them: nothing was competing for the keys.

Jean's ruling: the QWERTY-piano was the note source BEFORE Ableton;
it isn't used now — quarantine it, keyboard non-musical by default,
restorable at build. THE CUT (the idleness principle at the harness):
is_music_key's body is wrapped in `#ifdef INCUBATE_MUSIC_KEYS` — the
default returns false for every key (all keys fall to render; the
world owns W/A/S/D and the rest), the letters->analysis routing kept
WHOLE behind the flag for the day a keyboard-piano returns. The
dispatch site is untouched (the split still functions when the flag is
built). Applied to both mains that carry the router (incubator.cpp,
render default the_chord; incubator_dual.cpp, render default
the_board — the campaign's main); the_lab.cpp is analysis-only (no
render split) and untouched.

SCOPE NOTE: first change outside src/cartridges/the_board/ this
campaign — Jean-directed, the harness router that was swallowing the
ratified control channel. NO RIG GATE for the incubator mains here
(glaw1 is the_board syntax-only); the edit is a well-formed #ifdef
guard around an existing function (balanced directives/braces,
review-verified), the runtime proof Jean's build/run. the_board gates
unchanged (glaw1 GREEN full + minimal; score census GREEN; sentinels
148/5) — the_board untouched this fix.
AWAITING THE RIG: default build — W/A/S/D now reach the_board (pawn
walks in pawn-host, key 4 flies the camera); every previously-swallowed
key the world binds now arrives.

## PANEL-0 p1b — THE PAWN/POINT DISENTANGLEMENT AUDIT (read-only; the
## viewpoint/body split enumerated; ONE STOP for the stamp)

Jean's trigger: patch generation centers on the pawn, so in camera-host
the world stops streaming — one of a whole class of pawn-position
couplings that need classifying. The question: what must the PAWN own
by definition (the body), and what should the POINT own instead (the
viewpoint/awareness primitive that lives in whichever host)? NOTHING
CUT — this is the recon that precedes the cut. Product:
audit/POINT_P1B_AUDIT.md.

METHOD: two parallel censuses read EVERY pawn-position consumer — the
GPU (every compute_pawn_pos / render_pawn_pos / config.lod_pawn read
across world.wgsl) and the CPU (every readback_x/z, lod_pawn,
slots[possessed_slot].pos read across cartridge.hpp, the surface, the
bodies). Every claim file:line'd. v3 §9 (driver law/anchor) and §11
(witness/bubble) the lens.

THE DEFINITIONAL ANSWER: the pawn's position does DOUBLE DUTY today —
it is at once the BODY's location and the VIEWPOINT's location, because
the pawn was the only host the point ever had. The whole entanglement
is that one conflation. THE PAWN owns (the body): its walk +
terrain-snap, its draw/tilt, its ENTITY-EMANATING fields (aura dome,
speed-grown forcefield), its AI REFERENCE FRAME (NPCs pursue/flee/
cluster the body; floaters/cubes leashed), its POSSESSION TARGET /
portal-stepping / photographic-subject roles. THE POINT owns (the
viewpoint/awareness): the position the camera renders from, the
STREAMING/generation center, LOD/cull center, visibility window,
recenter cursor, the SHADOW-VP center, and the BUBBLE (v3 §11). The
existing design's luck: the CPU ALREADY splits these into two places —
player_.readback_x/z (de-facto viewpoint) vs slots[possessed_slot].pos
(body reference frame); today they hold the same value; the
disentanglement makes readback_x/z follow the POINT and leaves
slots[].pos the BODY. Half-built already.

THE CENTRAL SIMPLIFICATION: ONE lever re-tracks the whole CPU side.
cartridge.hpp:835-837 (the witness HARVEST) is the SOLE AUTHOR of
readback_x/z and reads the possessed pawn slot without consulting
config.point_host — so in camera-host readback_x/z freezes and every
CPU viewpoint consumer downstream freezes with it (Jean's terrain
freeze). Host-routing THAT ONE SITE re-tracks the entire CPU viewpoint
set (streaming center, recenter, LOD banding, lod_pawn stage, entity
draw-cull, orb anchor) — none needs its own edit. The catch the
witness law predicts: in camera-host the point = the CAMERA, whose
position is GPU-resident and NEVER read back to CPU today (confirmed —
only agent_state + floating_entity are staged, cartridge.hpp:1064-74).
So the ONE genuinely new piece of machinery is a CAMERA-POSITION
READBACK.

THE GPU SIDE is even more contained: only TWO reads, and one rides the
CPU change. VIEWPOINT-migrate: shadow-VP (world.wgsl:6768,
coupling_pawn_to_sun_vp(compute_pawn_pos())) — the ONLY GPU hand-edit;
and the frustum-cull LOD0 center (world.wgsl:8216, config.lod_pawn) —
but that is CPU-staged, so it follows the harvest automatically, NO GPU
edit. The camera aim (world.wgsl:6359) is ALREADY point-correct — the
camera-hosted branch returns before it (:6335). BODY reads confirmed
staying put every site: aura (sample_pawn_aura), forcefield
(zone_pawn_ff, radius grows with body speed), GoL suppression,
floater/cube leashing, and every possessed_slot IDENTITY/AI-frame read.

THE MECHANISM (sized): the point gains a WORLD POSITION, host-sourced,
both sides. GPU: point_pos() = point_camera_hosted() ? camera_state.pos
: compute_pawn_pos(); repoint the shadow-VP (one read); cull follows
CPU lod_pawn. CPU: a point-position readback — pawn-host reads the slot
exactly as today (PIXEL-IDENTICAL), camera-host reads back
camera_state.pos.xz. Two options: (A) camera-state readback only in
camera-host, pawn-host path byte-untouched (RECOMMENDED — pixel gate
binding, identity by construction); (B) a unified point-position
buffer both hosts read (more faithful to "the point has a position,"
but routes pawn-host through a new buffer so identity rests on value
not path). CENSUS-W impact: readback_x/z becomes the POINT's
(host-authored) while readback_portal_trigger stays the BODY's until
the bubble moves — the readback trio SPLITS along the pawn/point line;
Direction W's sole-author record updates to name the point.

THE ONE REAL HAZARD: the anti-flicker coupling. lod_pawn is CPU-banded
on purpose so the CPU banding and GPU cull "partition with the same
yardstick" (state.hpp:379-389 / world.wgsl:8209-15). Because the cull
reads the CPU-staged lod_pawn, routing the harvest to the point moves
BOTH sides together — the yardstick stays shared. Touch only one side
and boundary flicker returns. Keep coupled.

THE AMBIGUOUS SET (rulings owed, leanings given): the proposed rule —
the VIEWPOINT owns terrain existence and framing (streaming, LOD,
shadow, orb dome); the BODY owns the living world around it (NPCs,
floaters, possession, the photograph). Under it only the ORB DOME (and
the deferred bubble/portal) move to the point; NPC-respawn, possession
radius, cube corral, ribbon-ride selection, photographer VP all stay
BODY. Portal/bubble trigger leans POINT but stays pawn-realized until
the bubble machinery moves (p1a's dormant-bubble ruling), a bubble
sub-movement.

STOP — FOUR STAMP QUESTIONS: (1) is the viewpoint/body rule right, so
only orb dome + deferred bubble move? (2) readback option A (pixel-safe)
or B (unified buffer)? (3) move the portal trigger to the bubble in p1b
or defer it? (4) photographer stays body-anchored or follows the point?
NOTHING CUT — p1b cuts only after the stamp; the pawn-host pixel gate
is Jean's rig, held binding.

## PANEL-0 p1b — THE STAMP ANSWERS & THE REFINED MODEL (audit §7
## addendum; Jean's rulings folded in; still read-only, still STOP)

Jean answered and opened a fifth question. His answers push a CLEANER
line than the audit's §4 "body owns the living world": readback = option
A (ruled); photographer FOLLOWS THE POINT (ruled); floaters FOLLOW THE
POINT (ruled); portal = the POINT owns it, with the sharp question —
flying HIGH but over a portal's xz, does it fire?; and a new one —
when the camera flies past the stranded pawn, do we possess the nearest
pawn? Grounded each in the code (audit §7).

THE GROUNDING: the photographer is HALF-FREE — its trigger already reads
readback_x/z (gallery.hpp:583-600, walk-distance accumulation), so it
follows the point the moment the harvest is host-routed (in free-fly it
snapshots the FLIGHT); only its VP (world.wgsl:7921) needs point_pos().
The portal is 2D — a flat xz ellipse on the possessed slot
(world.wgsl:5697-5711), NO altitude term, so owning it as-is means
flying over at any height teleports (Jean's worry, confirmed). The
living world is BODY-CENTERED in EVERY lifecycle path: agent eviction
(world.wgsl:6288, beyond 360u of the possessed slot dies), respawn
cluster (agents.hpp:470, around the possessed slot), possession search
(agents.hpp:510, within 20u of the OLD body). So in free-fly the whole
population lives around the stranded pawn; fly past 360u and nothing
EXISTS near you — possess-nearest-the-point only works if the
population's EXISTENCE also follows the point. The non-obvious
consequence; it re-rules §4's NPC/possession leanings.

THE REFINED MODEL (supersedes §4): the doubled position splits into
PRESENCE vs EMANATION. THE POINT owns PRESENCE — where the world
happens around you: streaming/LOD/cull/shadow, the entities that
populate your surroundings (agents AND floaters — spawn/evict/cluster/
possess-reach), the gallery's record of your journey, the bubble that
senses portals. THE PAWN owns EMANATION & BODY-IDENTITY — only what a
body IS or emits: walk/draw/tilt, the aura dome, the speed-grown
forcefield, the AI-pursuit-target role — all IDLE in free-fly by
construction. Under presence-vs-emanation the ambiguous set flips the
other way from §4: floaters/agents/possession/photographer → POINT;
aura/forcefield/pursuit-target → PAWN.

THE BUBBLE GETS ITS FIRST FIELD: Jean's altitude question forces the
bubble out of p1a dormancy. Point-owned portal + "high shouldn't fire"
= the point's awareness needs a 3D extent, so PointBubble{} (empty
since p1a) gains its first real field — a RADIUS — and the portal
becomes its FIRST SENSOR (fires when the arch enters the bubble, tested
in 3D / xz-ellipse gated by a vertical band). The dormant-bubble ruling
is lifted for exactly this.

RESEQUENCED p1b — FOUR single-intent sub-movements, each guarded
point_camera_hosted() ? <point> : <pawn> so pawn-host stays
byte-identical: (a) THE POINT'S POSITION — point_pos() + camera-pos
readback (opt A) + host-route the harvest + repoint shadow-VP; fixes
the terrain freeze, carries the photographer trigger free. (b) THE
LIVING WORLD FOLLOWS THE POINT — agent+floater evict/respawn/possess/
kite centers → the point; answers possession; pursuit/flee idle-to-
wander when no body. (c) THE POINT'S RECORD — photographer VP →
point_pos(). (d) THE BUBBLE'S FIRST SENSE — portal → point_pos() + 3D
gate; PointBubble gains its radius.

FOUR PRECISION QUESTIONS RETURNED: (1) confirm population EXISTENCE
follows the point (naming the tradeoff — NPCs then wander around a
bodiless camera vs. today's dead expanse); (2) "floaters" = spheres AND
cubes (lean both, kite target → point) or spheres only; (3) pursuit/
flee wander-when-no-body (lean) or reference the point; (4) accept the
photographer's empty-center landscape travelogue in free-fly or hold it
body-only. NOTHING CUT — the model is now medium (four cuts) not the
small the audit first sized; every sub-movement pawn-host pixel-
identical; the rig binding.

## PANEL-0 p1b — THE STAMP CLOSED (all rulings ratified; the orb
## skybox added; audit §7.7 + §8; STILL nothing cut, awaiting the go)

Jean ratified the presence/emanation model and answered all four:
(1) population existence follows the point — CONFIRMED; (2) floaters =
spheres AND cubes, kite target → point — CONFIRMED; (3) pursuit/flee
behavior unchanged (wander when no active body) but the agents SPAWN
in the xz plane around the point — existence → point, behavior stays;
(4) photographer = the travelogue, empty-center landscape accepted
("modern art") — CONFIRMED.

FIFTH RULING — THE ORBS: center on the camera to look static, enlarge
the radius so movement doesn't affect it. The code splits it: HALF IS
FREE — the dome already follows readback_x/z (orbs.hpp:782-793,
pawn_anchored/KP_9), so p1b-a's harvest re-route makes it center on the
point=camera in free-fly, pawn-host pixel-safe. THE STATIC LOOK is a
deliberate polish = the classic skybox: center the dome on the camera
EYE (camera_state.pos, GPU-resident, all axes) + bigger radius
(ORB_DOME_RADIUS 450, orbs.hpp:44). Today dome_center.y is pinned to
ground (upload_orb_dome_center(q,x,0,z)) so flying up sinks the orbs;
an eye-centered dome rises with you. THIS IS THE ONE AUTHORIZED
DEPARTURE FROM THE PAWN-HOST PIXEL GATE — it intentionally shifts the
pawn-host sky (kited eye off the pawn), for the better, by Jean's call.
Mechanism is a SIMPLIFICATION: eye-center on the GPU (no readback),
retire the CPU ground-anchor, bump the radius (a rig dial). Becomes
p1b-e.

THE FINAL MOVEMENT LIST (each single-intent; a-d pawn-host pixel-
identical, e the authorized sky look-change): p1b-a THE POINT'S
POSITION (point_pos() + camera-pos readback opt A + host-route harvest
+ repoint shadow-VP; fixes the terrain freeze, carries the photographer
trigger AND the orb free-fly follow free); p1b-b THE LIVING WORLD
FOLLOWS THE POINT (agent+floater evict/respawn/possess/kite → point;
pursuit/flee wander-when-no-body); p1b-c THE POINT'S RECORD
(photographer VP → point_pos()); p1b-d THE BUBBLE'S FIRST SENSE (portal
→ point_pos() + 3D gate; PointBubble gains its radius); p1b-e THE ORB
SKYBOX (eye-center the dome + enlarge radius; retire the CPU
ground-anchor).

LAST OPEN DIALS (not blockers — rig tuning): orb radius value (450→?),
skybox-y (lean eye-centered all axes), and one untouched §4 item —
ribbon-ride selection STAYS BODY (riding is a host-migration onto the
body's neighborhood; held at the §4 lean unless Jean re-rules). NOTHING
CUT — p1b-a…e cut on Jean's go; the audit is now the ratified spec.
