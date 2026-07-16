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

## PANEL-0 p1b-a — THE POINT'S POSITION (the go given; dials closed;
## the terrain freeze fixed; pawn-host byte-untouched)

Jean closed the dials (skybox-y eye-centered ALL AXES; orb radius 700;
ribbon-ride stays BODY) and gave the go. p1b-a is the foundational cut:
THE POINT GAINS A WORLD POSITION, host-sourced, on both sides.

THE GPU: point_pos() lands beside the possessed-agent helpers
(world.wgsl, compute stage) — camera_state.pos when the camera hosts,
compute_pawn_pos() when the pawn hosts. ONE consumer repointed: the
sun/shadow VP in compute_vp (the 300-unit shadow box must cover what
the eye sees) — coupling_pawn_to_sun_vp(point_pos()). Pawn-host
identical by construction (point_pos IS compute_pawn_pos there). The
frustum-cull center needed NO GPU edit — it reads the CPU-staged
lod_pawn, which now follows the harvest (the anti-flicker yardstick
moves as one, exactly as the audit required).

THE CPU: the point readback, option A as stamped. The camera buffer
gains CopySrc; a 48-byte staging buffer (Camera State Readback
Staging) + a third state machine (CameraReadbackState IDLE/COPIED/
MAPPING) mirror the agent pattern exactly. THE COPY IS ENCODED ONLY IN
CAMERA-HOST (point_.host == CAMERA) — the pawn-host frame is
byte-untouched, the binding pixel gate satisfied by construction. THE
HARVEST is host-routed at its one source (cartridge.hpp P5): the agent
harvest authors readback_x/z only when the pawn hosts; the new camera
harvest authors it from camera pos.xz only when the camera hosts (host
re-checked inside the async callback so a mid-flight toggle cannot let
a stale value overwrite the other author). readback_portal_trigger
stays the BODY's until p1b-d. Downstream, UNTOUCHED BY DESIGN: the
streaming center, recenter cursor, LOD banding, lod_pawn stage, entity
draw-cull, orb anchor, photographer trigger ALL follow the point
through readback_x/z with zero edits — the audit's central
simplification, realized.

THE WITNESS CONTRACT AMENDED IN PLACE (spine_state.hpp): readback_x/z
is now THE POINT's position, host-authored, sole author still the P5
harvest; the trio splits along the pawn/point line
(readback_portal_trigger the body's until p1b-d); the no-CPU-mirror
clause gains its ONE sanctioned window (camera-host pos.xz, two floats
into the trio, not a mirror) — no readback_y, still not to be
invented. Score census Direction W: HOLDS unchanged (the writes stay
spine-side P5).

GATES: glaw1 GREEN full + minimal; score census GREEN (A/B/W +
W-deps); sentinels 148/5; encodings clean UTF-8/LF, no CR. NO WGSL
COMPILER HERE — the shader edit is two sites (accessor + one repoint)
reading only bindings compute_vp already reads (camera_state, config,
agent_state); runtime proof is the rig's.
AWAITING THE RIG: pawn host — pixel-identical everywhere (streaming,
shadow, LOD; the copy never encodes). Key 4 free-fly — TERRAIN NOW
STREAMS under the camera (the p1b trigger fixed); the shadow box
follows the view; LOD/cull recenter; the orb dome follows (its anchor
reads the point now); the photographer accumulates flight distance.

## PANEL-0 p1b-b — THE LIVING WORLD FOLLOWS THE POINT (existence →
## point; behaviors untouched; the kite lock-step; a deps face shrinks)

The presence half of the ratified rule, cut. EXISTENCE follows the
point; BEHAVIOR stays the body's (Jean's precision: "agents wander
like they do, but spawn in the xz plane around the point").

THE GPU (world.wgsl, all in kernels whose main compute layout already
carries binding 80): agent eviction re-centers on point_pos() (was
agent_state[possessed_slot]); sphere eviction + cube eviction
re-center on point_pos().xz (was compute_pawn_pos().xz); the cube
KITE HOME re-targets the point (follow_pawn=1: home.xz = point.xz +
offset); the sphere→terrain tint picks the nearest sphere to the
POINT (the tint colors the terrain around the viewpoint — presence,
not emanation; a judgment call under the ratified rule, disclosed).
Pursuit/flee (behaviors 6/7) UNTOUCHED — they still reference the
body, per the ruling. The pawn-host values are identical everywhere
(point_pos IS the possessed slot's pos there).

THE CPU: respawn clustering (agents.hpp) and possession reach
(try_possess_nearest) re-center on readback_x/z — THE POINT's
position; cube corral + kite-offset capture (cube_behaviors.hpp)
likewise. VALUE-IDENTICAL in pawn-host: the slot mirror and
readback_x/z are written from the SAME P5 harvest snapshot, so the
old slot read and the new readback read cannot differ there. THE
KITE LOCK-STEP: the CPU offset capture (cube.cx - point.xz) and the
GPU kite home (point.xz + offset) moved in the same commit — the F7
toggle still preserves world position exactly, the audit's
lock-step law honored (same shape as the lod yardstick).

A DEPS FACE SHRINKS: corral/kite were CubeDeps' only agent_state_
readers — the member is retired (struct + wiring + banner); the cube
module now reaches the witness record only. The deps law (faces carry
true reaches) enforced by the cut itself.

TWO RIG-SAVING CATCHES (the no-WGSL-compiler gap, hand-checked): the
cube kernel's SECOND pawn_xz use — the behavior-force call — would
have dangled after the rename (fixed: passes point_xz; the param was
unused inside, renamed for honesty). Wire names follow_pawn /
pawn_offset (GPU struct fields) NOT renamed — they are the buffer
ABI; renaming is a separate hygiene cut if ever wanted.

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
148/5; encodings clean UTF-8/LF, no CR.
AWAITING THE RIG: pawn host — identical (all centers = the pawn's
pos, same values, same math). Free-fly — agents evict/respawn under
the camera (a living world you fly over); Caps Lock possesses a body
wherever you flew; spheres/cubes live around the view; F7 kite
follows the flight; the tint tracks the view.

## PANEL-0 p1b-c — THE POINT'S RECORD (the photographer follows;
## the layout grows a binding; the travelogue is live)

The smallest cut, as audited — the TRIGGER already followed the point
the moment p1b-a host-routed the harvest (update_photographer reads
readback_x/z; its step>5 teleport guard even absorbs host-toggle
jumps for free). This cut moves the VP.

THE GPU: compute_photographer_vp frames point_pos() — eye = spherical
offset from the point, aim = the point, light-VP = the point's sun
box. Pawn-host identical (the point IS the body there — the
self-portrait unchanged); free-fly frames the vantage with no body at
the center — THE TRAVELOGUE, Jean's "modern art", accepted by stamp.
Locals renamed honest (point_p, cam_to_point); the §8 banner names
the journey.

THE ONE STRUCTURAL EDIT: the photographer compute LAYOUT did NOT
carry binding 80 (camera_state) — the "retained for future use" slot
was the placement layout's, not the photographer's (the audit's
grounding caught this before the rig could). point_pos() statically
reads camera_state, so the photographer layout + bind group grew to
10 entries (binding 80 → cameraBuffer_), edited in LOCK-STEP (layout
and group must match or pipeline creation fails).

THE CPU: no functional edit (the trigger followed at p1b-a). Honesty
renames only: prev_pawn_x/z → prev_point_x/z (module-internal, 8
sites) + the trigger comment (the point travels; the body's walk in
pawn-host, the flight in free-fly).

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
148/5; encodings clean.
AWAITING THE RIG: pawn host — identical (same subject, same VP, same
trigger cadence). Free-fly — snapshots accumulate along the flight;
paintings appear on terrain streamed under the camera; each is a
landscape from the flight path (no body in frame).

## PANEL-0 p1b-d — THE BUBBLE'S FIRST SENSE (the portal is the
## point's; the altitude gate; the bubble gains its radius)

Jean's fly-above question, answered in code. The portal is the
bubble's FIRST SENSOR and the bubble gains its FIRST FIELD.

THE PROBE IS HOST-SOURCED — with a freshness subtlety the cut had to
honor: in pawn-host the probe is agent.pos THIS FRAME (the local var
mid-kernel — byte-identical to the pre-p1b test; point_pos() was
deliberately NOT used, it reads the storage copy which is one frame
stale and would have made the pawn's portal fire a frame late). In
camera-host the probe is camera_state.pos (the point; last frame's —
free-fly tolerates the standard lag).

THE VERTICAL GATE (camera-host only): on an xz ellipse hit, the arch
ground is sampled (query_ground_flyer at the portal's center — runs
only on a hit, at most one query per frame) and the trigger fires only
if |probe.y - arch_ground| < POINT_BUBBLE_RADIUS. Skim over a portal →
fire; fly high above its xz → the arch is outside the bubble → no fire
(Jean's ruling made mechanism). Pawn-host: the gate branch is never
taken — identical by construction.

THE BUBBLE IS REAL: PointBubble{} (empty since p1a) gains float radius
= POINT_BUBBLE_RADIUS (20.0f, contracts/point.hpp) mirrored as the
WGSL const (MUST-match pair, the eviction-radius pattern — no runtime
upload; a rig dial). The point.hpp banner sheds its dormant-bubble
prophecy for the realized state (presence follows the point; emanation
stays the body's; the bubble live). The witness contract updates:
readback_portal_trigger is the point's bubble sensor riding the
possessed slot's wire in both hosts — the wire is the realization,
the bubble is the semantics; same sole-author law.

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
148/5; encodings clean.
AWAITING THE RIG: pawn host — portal stepping identical (same probe,
same ellipse, no gate). Free-fly — flying THROUGH a portal at arch
height triggers the transition; flying OVER its xz high up does not;
the 20-unit band is a dial (bump POINT_BUBBLE_RADIUS in both mirrors).

## PANEL-0 p1b-e — THE ORB SKYBOX (eye-centered all axes; radius 700;
## the anchor machinery retired whole; sentinels 148→147 by design)

The fifth movement — the ONE authorized departure from the pawn-host
pixel gate (Jean's ruling: the orbs center on the camera to look
static; radius up so movement doesn't read on the sky).

THE CUT IS A SIMPLIFICATION. The orb VS already carried the camera
(render_camera, the billboard basis) — the dome center becomes
render_camera.pos, ALL THREE AXES, every frame, GPU-side, zero
uploads: dome_center = render_camera.pos. Flying up, the sky rises
with you (the old dome was ground-pinned at y=0 — climb and the orbs
sank below). The photographer pass binds its own camera at the same
slot, so SNAPSHOTS inherit a correct skybox automatically.
ORB_DOME_RADIUS 450 → 700 (Jean's dial).

THE ANCHOR MACHINERY RETIRED WHOLE (dead the moment the VS
eye-centers): update_orb_anchor + toggle_orb_anchor (fns + decls);
the five OrbsState anchor fields; the mood table's
anchor_to_pawn_default COLUMN (struct field + all six positional
rows, hue-value-disambiguated edits); the first-run anchor seed; the
status print's anchor field; the teardown reset; upload_orb_dome_
center (state.hpp); the spine's anchor movement (cartridge.hpp); the
KP_9 case (the key FREED — noted in place). The GPU struct's
dome_center_* fields stay as ABI bytes, zero-filled, marked DEAD WIRE
in both mirrors (C++ + WGSL).

THE GATE BASELINE MOVES, DOCUMENTED: sentinels 148/5 → 147/5 — the
retired anchor movement carried a ROSTER-GATE orbs (b); the score
census manifest retired the matching 'anchor' row + the
update_orb_anchor whitelist entry IN THE SAME COMMIT (Direction A/B
both re-prove GREEN at the new baseline). the_chord keeps its own
copy of the old anchor (separate cartridge, out of campaign scope).

GATES: glaw1 GREEN full + minimal; score census GREEN at the new
manifest; sentinels 147/5 (the new baseline, minus-one by design);
encodings clean; residual grep for every retired name: ZERO.
AWAITING THE RIG (the authorized look-change, BOTH hosts): the orb
sky reads as a true skybox — static under all movement, rising with
flight; slightly shifted vs the old ground-pinned dome in pawn-host
(the kited eye sits off the pawn); radius 700 the dial to taste.

## PANEL-0 p1b — THE CAMPAIGN CLOSES (a…e landed; the point is real)

Five single-intent cuts, one arc: the point OWNS presence (position →
streaming/LOD/cull/shadow/orb-follow, p1b-a; the living population,
p1b-b; the record, p1b-c; the bubble's first sense, p1b-d) and the
sky became the witness's skybox (p1b-e). The pawn keeps emanation and
body-identity, idle in free-fly by construction. a–d pawn-host
pixel-identical by construction (host-guarded branches; same-snapshot
value identity; the probe freshness rule); e the one stamped
look-change. The witness contract, the point contract, and the census
manifest all amended in place — the paper trail is the code's.

## PANEL-0 p1b-tune — TWO DIALS (Jean, from the rig): free-fly ×2,
## the orb dome out of the fog

Two rig-observed tuning turns, no structure:
- CameraControls::MOVE_SPEED 15 → 30 (the free-fly velocity, wired to
  config.point_fly_speed at boot — camera-host ONLY; the pawn walk
  Idle::PAWN_SPEED is a separate constant, untouched). W/S/A/D flies
  twice as fast.
- ORB_DOME_RADIUS 700 → 500 (the p1b-e skybox radius). At 700 the dome
  had fallen past the fog far-plane and vanished; 500 sits inside it,
  visible again. Still a true skybox (eye-centered, p1b-e); only the
  shell distance moved.

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
147/5; encodings clean. Two constexpr edits, no wire/score change.

## PANEL-0 p2 — THE MATRIX RECON (read-only; the grid shape proposed;
## ONE STOP for the stamp)

Goal (ratified): replace the hand-written per-demo headers with ONE
constexpr grid — pieces as rows, demos as columns, cells booleans —
COMPLETE (foundational pieces as locked rows) and CODE-MATRIX (the
grid IS C++), preserving the compile-time boolean fold the pipeline
gate depends on. First p-series cut with NO pixel gate. NOTHING
touched. Product: audit/PANEL0_P2_MATRIX_RECON.md.

THE CURRENT SURFACE (R1): DemoConfig = {Roster roster; uint32_t seed;
uint32_t boot_mood}. Exactly three consumers — DEMO.roster → ROSTER
(demo.hpp:33, the 148 gate sites); DEMO.seed → surface_services.hpp:47
(WorldState boot); DEMO.boot_mood → spine_state.hpp:134 (mood wake).
full.hpp/minimal.hpp are positional aggregate brace-lists (19 bools in
Roster field order + seed + mood). Selection: INCUBATE_DEMO (default
full) → demo.hpp token-pastes the header path → #include
demos/<name>.hpp defines DEMO → demo.hpp derives ROSTER + the FIRST
EDGE assert. DROP-IN LAW: the grid must still emit inline constexpr
DemoConfig DEMO at the same point — then demo.hpp/ROSTER/the 148 gates
are untouched.

THE ROW CENSUS (R2, COMPLETE): 19 TICKABLE rows (12 families + 7
features). 4 LOCKED rows, census-checked against the score
FOUNDATIONAL whitelist: THE SURFACE, THE SUN, THE POINT (witness, the
p1b parent), THE PAWN BODY (slot-0, never evicted). Per-column scalars
(not rows): seed, boot_mood. Clean seam: slot-0 body LOCKED vs slots
1+ = wanderers TICKABLE. BODY DISPOSITION FLAGGED: lean LOCKED-and-
noted (complete census; the body always exists; a graduation note says
it becomes tickable when a demo pulls a bodiless world) vs off-grid;
p2 does NOT make it tickable either way. Jean rules.

THE TABLE SHAPE (R3): constexpr bool GRID[Piece::COUNT][DemoCol::COUNT]
— rows = pieces (name down the left, the spreadsheet layout), columns
= demos (named header), locked four on a comment banner above (always-
on, not cells). column_to_roster(DemoCol) returns Roster{...19 in field
order...}, fully folded; a static_assert pins Piece::COUNT==19 (the one
ordering invariant). Seed+mood as DEMO_SEED[]/DEMO_BOOT_MOOD[] per
column; DEMO built from all three (DemoConfig unchanged, downstream
byte-identical). SELECTOR two options: token-paste DemoCol::
INCUBATE_DEMO + retire the per-demo headers (LEAN), or thin one-line
column-selector headers (selection path byte-unchanged). FREE TICKING
CONFIRMED: the census finds ONE legality edge (transitions⇒portal, the
FIRST EDGE assert) — the grid does NOT encode it; the assert rides
ROSTER and fires identically. The cross-bit reads (gallery||indoor_
shell, column||antenna ×7) are shared-resource GATES not legality
edges — no encoding. The matrix feeds the resolver, never reimplements
it.

MIGRATION (R4): full = 19 tickable 1; minimal = 19 tickable 0; both
seed 42 / OPEN_DEFAULT. Byte-equivalent by construction (same 19 bools
same order → same ROSTER fold). GOLDEN: ROSTER's 19 fields + DEMO.seed
+ DEMO.boot_mood value-for-value vs the pre-matrix headers. Headers:
recommend retire (primary selector); keep-as-thin-selector the
conservative alt.

TERRAIN-ONLY (R5): expressible — a terrain column (every tickable 0)
EQUALS minimal's column (locked surface/sun/point/body always on) and
boots. THE FINDING: because the body is LOCKED (not tickable in p2),
terrain-only-as-defined IS minimal (pawn present-but-idle, you free-
fly). A truly BODILESS terrain-only needs the body-tickable cut first.
So p3 either revises terrain on minimal (pawn idle) today, or pulls
the body-tickable cut so terrain unticks the body row (the one bit
distinguishing it from minimal). Flagged; not a p2 blocker (scopes p3).

SIZING (R6): small — pure config authoring, no WGSL, no behavior, no
pixel gate. Gates: glaw1 full+minimal unchanged; the byte-equivalence
golden (full+minimal ROSTER/seed/mood identical to pre-matrix); score
census GREEN; sentinels 147/5; encodings clean.

STOP — THREE STAMP QUESTIONS: (1) body row LOCKED-and-noted (lean) or
off-grid; (2) selector token-paste-retire-headers (lean) or thin
column-selectors; (3) p3 terrain-only rides minimal (pawn idle) or
needs the body-tickable prerequisite. Seed/mood as per-column scalars
(DemoConfig unchanged) the recommended default. NOTHING CUT — p2 cuts
after the stamp.

## PANEL-0 p2 — THE MATRIX (the grid cut; headers retired; the golden
## has teeth; composing a demo is ticking cells)

Jean's three stamps, all confirmed and cut: (1) the body row
LOCKED-AND-NOTED; (2) the per-demo headers RETIRED, the column
token-pasted; (3) terrain-only rides minimal-with-idle-pawn, the
body-tickable cut stays deferred. First p-series cut with NO pixel
gate — pure authoring surface.

THE GRID (demos/matrix.hpp, NEW): constexpr bool
GRID[Piece::COUNT][DemoCol::COUNT] — rows = pieces (named down the
left, Roster field order), columns = demos (full, minimal). The four
FOUNDATIONAL pieces (surface, sun, the point, the pawn body) ride a
LOCKED banner above the grid — always-on, so not tickable cells; the
census reads whole without pretending they toggle. The body carries
its locked-AND-NOTED note (tickable the day a demo pulls the deferred
body-tickable cut — then terrain gains a cleaner column, one cell
flipped, no rework). column_to_roster(DemoCol) aggregate-inits a
Roster in field order — folds whole, so every one of the 148 gate
sites sees the same constexpr bool it always did. Seed + boot_mood are
per-column arrays (DEMO_SEED / DEMO_BOOT_MOOD); demo_config() assembles
the full DemoConfig.

THE DROP-IN (demo.hpp): DEMO is now demo_config(T7B_DEMO_COL(
INCUBATE_DEMO)) — the token-paste resolves INCUBATE_DEMO=<name> to
DemoCol::<name>, no per-demo header FILE (full.hpp + minimal.hpp
DELETED). A bad name → DemoCol::xyz → clean unknown-enumerator error.
ROSTER = DEMO.roster and the FIRST EDGE assert are BYTE-UNCHANGED —
the assert still rides the folded ROSTER, the guardrail the grid feeds
but never reimplements (free ticking: the grid encodes zero edges).

THE BYTE-EQUIVALENCE GOLDEN (Jean's mandatory gate, made PERMANENT):
four static_asserts in matrix.hpp pin the two migrated columns to the
retired headers' exact values — full.roster.all_enabled() +
seed 42/OPEN_DEFAULT; minimal.roster.none_enabled() (the new mirror
method on Roster, its one consumer) + seed 42/OPEN_DEFAULT. The
headers are gone; these asserts are what prove the retirement lost
nothing and keep it lossless forever. A clean compile IS the proof.
PROVEN TO HAVE TEETH: a negative test (full's pyramid cell flipped
off) fired the exact assertion — the gate is not vacuous. Plus a
Piece::COUNT==19 assert pins the row/field-order mapping (add a Roster
field ⇒ add a row ⇒ or the build trips).

GATES: glaw1 GREEN full (default→DemoCol::full) + minimal
(INCUBATE_DEMO=minimal→DemoCol::minimal); the golden asserts compile
(byte-equivalence proven, negative-tested); score census GREEN (no
gate site touched); sentinels 147/5 (ROSTER identical, no gate
added/removed); encodings clean UTF-8/LF. NO PIXEL GATE, no WGSL, no
behavior — pure config authoring; the rig sees demo=full and
demo=minimal exactly as before.

THE WORKBENCH: composing a demo is ticking cells in one visible grid.
p3 — the terrain panel on the terrain-only column (= minimal today,
pawn present-but-idle) — is the last movement: sit in the isolated
surface and tune it, the two-dial camera panel already proving the
form terrain inherits.

## TERRAIN-0 — THE "WHAT IS TERRAIN?" AUDIT (read-only; the definitional
## map; ONE report; STOP for the design conversation)

The critical section's recon, cut deep-not-wide via a 13-agent workflow
(5 seam readers A1-A5, each distinctness verdict adversarially challenged,
+ a module-reality pass + a dependency/dependent sweep + a completeness
critic; ~1M tokens, 0 errors). Every claim file:line'd. NOTHING moved.
Product: audit/TERRAIN0_AUDIT.md.

THE DEFINITIONAL ANSWER: terrain is SIX roles under one word, and — unlike
the pawn's three cleanly-nested roles — welded by an ASYMMETRY: a near-leaf
AUTHORING MEMORY on its input face (seed-pure stateless height + per-tile
character memory) that becomes a deeply-FUSED COMPOSITION SURFACE on its
output face, because "the ground at (x,z)" is NOT stored — it is re-composed
inside every query from five authors (seed lattice + tile character +
spawn-placed solids/piers + GoL zones + pawn aura). There is no single
"terrain boundary" to hold; the composition happens inside the query.

THE SIX SEAMS (verdicts survived adversarial challenge): A1 THE FRAME =
PARTIALLY_FUSED (a distinct policy-composition-and-dispatch architecture —
ground_architecture.hpp POLICIES[] → query_ground_* — whose composition
truth is duplicated by hand in the leaf-collapsed static base + the hot
paths ground_formed_with_complexity/patch_terrain_vs/shadow_patch_terrain_vs;
gait is clean delegation NOT fusion). A2 THE GENERATOR = DISTINCT (a
stateless seed→height 6-band lattice-wave field entering composition as one
clean multiplicand) BUT (i) its temporal machinery is INERT — both callers
pass t_beats=0, the living generator is FROZEN; (ii) it has NO single home
(a committee across §1.6 WGSL + tile_world amp/bias + patch_system piers +
ground_architecture composition). A3 THE LIFECYCLE = PARTIALLY_FUSED
(distinct residency core — integer-windowed alloc/evict/layer-pool/phase —
with a separable VISIBILITY/LOD-banding sub-seam braided into stream_patches,
different metric + cursor, the LOD0 constant hand-duplicated across the
C++/WGSL boundary with lod_pawn shipped to keep them equal). A4 THE SPAWNING
SURFACE = PARTIALLY_FUSED — fractures on the VERB: "can I stand here?" is
terrain-AGNOSTIC (occupier machine, 2D packing, terrain never vetoes); only
"at what Y?" is terrain-owned (sample_terrain_y_at + compute_entity_placement);
+ the patch-tether. A5 THE VOICE = PARTIALLY_FUSED (two sub-seams geometry-
wave + color-palette; owns NO module, only additive/re-composite hooks; fully
authored but DRIVERLESS — every dial's one live writer is the boot block
holding it neutral). A6 THE LIVING-TILE (GoL) GEOMETRY = the critic's found
seam: a parallel terrain-geometry organ (own extrusion mesh + a suppression
law hand-synced across THREE sites) that A1 only composes and nobody owns as
terrain.

TWO STRUCTURAL LAWS: LAW 1 the asymmetry (near-leaf input, five-author
composition output). LAW 2 the tile_world CROSS-CUT (terrain's anchor+body+
gait) — one module fuses HALF of A2 (landform character: amp/bias/activation)
+ HALF of A4 (spawn density/theme) via one TileState + one generation moment.
COROLLARY the baked/live divergence: baked consumers (placement/shadow/
photographer) see static_base+pyramids; live consumers (camera/pawn/agents)
see aura+waves+pulses+GoL — three disagreeing height samplers; paintings
already re-add GoL analytically to compensate. Mood is a THIN MODIFIER not
the author (seed-pure landform + two amplitude-only touches).

MODULE REALITY (D): population_themes = cleanest single-seam owner;
ground_architecture + surface_services = minimality candidates (pure-decl
mirror-registry; decl-tier-split-for-cohort-law); tile_world = the key
cross-cut; world.wgsl = the multi-seam monolith (generator + voice + GoL +
GPU realization). Large DEAD/DRIVERLESS register (report-only): the whole
wave-overlay chain (band_blend=-1/terrain_time=0), the WAVES→lipschitz_factor
dead limb, MODE_COUPLING_MAGNITUDE=0, the density lattice no-op
(DENSITY_MIN==MAX==1.0), terrain_state.tint/render_terrain dead consumer, the
LATENT placement-policy API, + smaller latents; and the hand-mirror hazards
(GoL suppression triple, the hot-path family whose shadow copy already
diverges by dropping aura+pulses).

PANEL SURFACE (E): the decisive finding — the CORE aesthetic dials
(TERRAIN_BANDS spectrum, PALETTE_*, OVERLAY_WAVES) live in WGSL compile-time
consts with NO C++ mirror → not panel-authorable without the graduate-or-
index fork. The panel emerges in two tiers: a READY tier (seed, amp_ceiling,
the whole lifecycle set, the voice dial VALUES — C++/mirror today) and a FORK
tier (band spectrum + palette + overlay-wave spectrum). The two-dial camera
panel (p1b) is the form the ready tier inherits.

FIVE DESIGN FORKS surfaced for the conversation: (1) the tile_world split;
(2) the baked/live divergence — one ground or intentional split; (3) the
graduate-or-index fork; (4) the frozen generator + driverless voice — is
terrain the moment they get a driver; (5) A6 GoL geometry — a seam owner +
the shadow-caster divergence. NOTHING CUT — the audit answers "what is
terrain"; the campaign decides what to do.

## TERRAIN-1 — THE MANIFOLD RECON (read-only; two targets, one map;
## STOP for the manifold-spine design conversation)

The reframe (Jean, ratified): terrain is a PLACEMENT MANIFOLD (coord →
position + normal); the heightfield y=f(x,z) is our DEFAULT CAST, not
the essence; finite vs infinite is a BOUNDARY CONDITION on one manifold.
This recon measures the distance from the code to that model. Four
parallel deep-reads + a policy-mask synthesis; every claim file:line'd.
NOTHING moved. Product: audit/TERRAIN1_MANIFOLD_RECON.md.

TARGET 1 — HOW MANIFOLD-SHAPED IS THE CODE ALREADY?
M1 THE QUERY FACE: every ground query is xz→scalar y, Y-up, one height
per column — NO coord→surface-point signature exists. BUT the manifold's
future interface is already SCAFFOLDED and idle: the *_gradient family
already returns (height, slope_x, slope_z) — one step from coord→
position+normal — and the POLICY_PLACEMENT_* rows declare a clean
placement-query API; both are LATENT (zero callers), the live paths
bypass them. QueryInputs.consumer_pos is already a vec3 (the signature
carries a 3D point; the scalar-Y assumption is in the bodies). The
manifold spine is one return-type + a consumer-move away, not a new
subsystem.
M2 THE HEIGHTFIELD BLAST: the value provider is a thin LEAF (swap the
body, "give me y at xz" callers unaffected — tile_world F1-F4 advertises
this). "Y-up/XZ-ground" is a pervasive SPREAD across FOUR welds: the
rgba16float texel contract (height,grad_x,grad_z,complexity — no room
for position+normal), both mesh VS (planar XZ grid displaced only in Y —
cannot fold/overhang), the normal basis (reconstructed vec3(-gx,1.0,-gz),
the literal 1.0 IS the assumption), and the Y-up movement + XZ spatial
index (~30 pos.y+= sites, agent XZ-integrate-then-snap-Y, PAWN_STEP along
Y, patch/tile grids keyed (gx,gz)). A non-heightfield cast rewrites
storage+mesh+normals+movement+index; only the per-column value functions
survive.
M3 THE EXTENT MODEL: on the SURFACE side finite and infinite are ALREADY
ONE MECHANISM — generation is byte-identical regardless of finiteness
(no producer reads finite_mode/finite_radius/world_bound); finite_radius
IS the boundary parameter, open = world_bound(0,0,0,0). "Finite" is ~6
scattered imperative branches (center-pin, radius-cap, all-visible band,
pawn-clamp, camera-clamp, containment decoration), each clamping the
INPUT xz before the query — "1.5 code paths," generation fully shared.
Always streamed (nothing pre-bakes; finite = "the window stops growing +
stops following the pawn"); world_bound is a CONTAINMENT SHELL (input
clamp), not terrain extent (MOOD_FINITE_OUTDOOR is finite with no walls);
finiteness only ever entered via portal, the world always boots open.
Distance to Jean's model: small + structural — collapse the six branches
behind one window=intersect(follow,boundary) rule + a boundary-carrying
query; generation needs zero change.
M4 THE COMPOSITION ORDER: base = terrain_height(seed)*tile_amp + bias +
piers (three authors, one scalar); bake +pyramids; live queries +GoL
+waves +pulses +aura per policy. The SIX-CONSUMER / SIX-GROUND
divergence, exact from the masks: pawn stands on base+pyr+GoL+waves+
pulses+aura(+suppression); camera clamps to same minus suppression;
entities placed on baked base+pyr ONLY (no GoL/aura); terrain renders
base+pyr+waves+pulses+aura (NO GoL — separate extrusion); shadows from
baked+waves (no aura/pulses); ribbon on CPU tile-only estimate;
paintings re-add GoL analytically. Today partly masked by the dead wave
voice; GoL+aura are LIVE, so reviving the voice WIDENS the divergence.
The raw material for one consistent manifold query — mapped, not fixed.

TARGET 2 — THE RAYMARCHING RESIDUE
M5 THE EXCAVATION MAP: exactly ONE raymarch-era subsystem survives and
is essentially fully ISOLATED — the TerrainState/render_terrain buffer +
everything computing into it, WRITE-ONLY across the whole shader (no
VS/FS reader, no CPU readback). Smoking gun: lipschitz_factor =
sqrt(1+max_grad²) (world.wgsl:5415), the cone/sphere-trace step bound of
a heightfield graph, read by nobody; its own comment names the
provenance ("Legacy fixed-wave dynamics … Only gradient_max survives for
Lipschitz"). The FS-visible binding 220 render_terrain (read by nobody)
is exactly where an SDF shade-fragment plugged in. Excises whole: buffer
+ struct + Idle init + update_terrain_config kernel + dispatch + binding
20/220 + the legacy WAVES table + wave_enable/freeze/frozen_t + the
amplitude-trajectory feeder. THE ONE ENTANGLED WIRE: terrain_state.tint
is a dead store inside the LIVE update_sphere kernel (+ coupling_sphere_
to_terrain_tint) — a surgical delete, not a clean lift. NOT residue
(live): the finite-diff normal functions (return vec3 normals for
shading/tilt) + the VP builder's stale "raymarch_get_direction" comment.
M6 THE DISPOSITION TABLE (the critical separation — TWO wave systems,
easily confused): DORMANT-VOICE (revive as DEMO-2) = the OVERLAY-wave
geometry voice + substrate temporal machinery + color voice/mode
coupling + tile activation_scale. SDF-RESIDUE (excavate) = the
TerrainState block + lipschitz chain + the LEGACY WAVES table +
freeze/amplitude feeder + the sphere-tint wire (entangled). GENUINELY-
DEAD (delete) = complexity texel channel, the density lattice no-op,
ActivePatch::animated + RENDER_RADIUS aliases + binding 144. LATENT-
SCAFFOLD (KEEP) = the POLICY_PLACEMENT_* + *_gradient API — NOT dead, it
is the manifold's own future interface. LIVE-BUT-FRAGILE (keep+note) =
the GoL-suppression triple + the hot-path family (shadow copy already
diverges).

FIVE DISTANCES for the design conversation: (1) the interface is one
return-type away (scaffold idle); (2) the cast is a rewrite, the value a
leaf; (3) finiteness is already a boundary parameter (six clamps → one
window rule); (4) the composition tangle is the real spine question (six
grounds; wave-deadness masks it, revival widens it); (5) wire-first-
clean-second — the SDF residue is one isolated excavation, the dormant
voice a separate revival, the gradient scaffold kept as the manifold's
landing site. NOTHING CUT — the map is drawn; the campaign decides.

=== CARTRIDGE MODULE CENSUS (read-only; audit/CARTRIDGE_MODULE_CENSUS.md) ===
Whole-cartridge census: 34 hpp + world.wgsl (no .inl exist), anchored to
theory v3 (§2 L0-L5, §9 S1-S4) + M6. §1 module table (strat | role |
offer-face | requires | stability | terrain-touch); §2 terrain map
(each terrain-toucher → IFACE / BASE / 4 welds / OVL / 3 samplers).
STABILITY: SPINE-dominant. IN-FLIGHT = 3 — ground_architecture.hpp
(INTENT contributor stubs + LATENT policy-surface), ribbon.hpp (TESTING
SPAWN_CHANCE=0.900 "revert before ship" L93), input.hpp (sky-mode fade
unbuilt). NO whole-module RESIDUE — residue is intra-module: world.wgsl
hosts the SDF block (M6 excavate) + dead channels; dome_center dead wire
(orbs/state, ABI); activation_scale DORMANT-VOICE (tile_world);
POLICY_* scaffold KEEP (ground_architecture). §2 SPANNER FINDINGS:
world.wgsl = the whole manifold (Stage-3 blast radius); the four welds
smear across 6 C++ modules (welds ARE the cost, confirmed structurally);
W-mi most diffuse (5 modules); IFACE has only 2 authors (world.wgsl body
+ ground_architecture contract), consumers all in-shader — the frozen
signature's surface is small, which is what makes the sphere plug-in
cheap above the welds. Read-only, no code touched.

## TERRAIN-2 (STAGE 1 cont.) — b2 THE FOLD, PULLED (Jean's hierarchy ruling)
Jean handed the reconciled stack (via Claude Chat): CAST = geology →
pyramids (drape) → piers; OVERLAY = waves (bottom) → GoL+pulses → aura+
suppression. Rulings: b2a = the safe shared-stack helper (shape a, NOT
the data-driven fold); b2b = RULED scope (placed structures gain the
WORLD-ANCHORED overlays; mover-anchored stay divergent); b2b mechanism =
CC's call; Delta 2 (pyramid drape/union) = separate base-shape fork.

RECON (world.wgsl, verified): patch_terrain_vs folds waves(3751) +
pulses(3755) + aura(3747) into the rendered mesh; GoL height is a
SEPARATE extrusion pass (not in the VS). The b2b mechanism already
exists in-tree: paintings do sample_terrain_y_at + contrib_gol_zones_at
(8212) = baked static + analytic world-anchored delta; the other
grounded families read BARE sample_terrain_y_at (columns 8229, palm/
cactus/blade 8238-58, arch 8269 min, pyramid 8288 5-pt min). So b2b =
extend the painting pattern (GoL+pulses delta) to every grounded family.
compute_entity_placement is per-frame → structures will ride.

=== b2a — THE SHARED DYNAMIC-OVERLAY STACK (pixel-identical) ===
manifold_overlay_stack(xz, qi, gol_term) = static_base + pyramids +
gol_term + waves + pulses (world.wgsl:2723). The four dynamic policies
(flyer/walker/walker_tilt/walker_agent) now call it; the MOVER-ANCHORED
aura is added AFTER the stack by each caller (external/self/none) — the
world/mover split made explicit at the call site, pre-figuring b2b.
BIT-IDENTICAL by construction: all four already summed in the identical
order base→pyr→gol→waves→pulses→aura; gol_term carries the caller's
resolved GoL (raw for flyer/agent; inline pawn-suppressed gol*(1−supp)
kept a SINGLE term for the walkers); walker_tilt = the stack alone (no
aura, no +0.0). walker_pair LEFT hand-fused (its shared-eval halves the
per-XZ work — the ground_formed_with_complexity precedent). Gradient
variants + manifold_height_hf unchanged (same signatures/returns).
ORDER NOTE: the sum keeps the historical operand order (GoL before
waves) for bit-identity; the hierarchy's canonical order (waves at the
overlay bottom) is DOCUMENTATION until a layer goes non-additive.
GATES: glaw1 GREEN; score census GREEN (bijection holds); world.wgsl
clean UTF-8/LF, no CR. Blind-WGSL (no WGSL compiler in env) — hand-
verified bit-identity; the RIG is the pixel/frame proof. HELD for rig.
RIG: GREEN (Jean, "you may continue") — b2a pixel-identical confirmed.

=== b2b — WORLD-ANCHORED OVERLAY RIDE (observable) ===
The sink/float fix: surface-standing families now sit on the LIVE zone
surface the mesh renders, not the baked static height. In
compute_entity_placement, every standing family gains + contrib_gol_
zones_at (raw — structures aren't movers, no suppression), extending the
PAINTING precedent (which already did sample_terrain_y_at + gol) to
column/antenna (8264), palm (8274), cactus (8285), blade (8296), arch
per-foot-then-min (8308). PYRAMIDS EXCLUDED — they are CAST (buried
occupiers the terrain drapes over), not surface-standers.
MECHANISM = delta-on-baked (keeps the O(1) baked geology cached, adds
the cheap analytic overlay), per-frame (the pass re-runs; gol reads the
re-simmed zone_life buffer — no explicit time needed). Bindings verified:
zone_config/zone_life/config all in the entity_placement layout (paintings
already use contrib_gol_zones_at here) — state.hpp:4055-4114.
DISCLOSURES / RESOLUTIONS: (1) PULSES — RULED A NON-ISSUE (Jean), not
deferred-pending: structures were never meant to ride pulses, nothing
reads as floating, the baked+GoL height is correct. The FrameSignal
bind-group change is NOT pursued; the banner note stays as documentation
of the decision. (2) WAVES — dead today (config.terrain_time gates to 0,
a no-op); whether a revived wave carries structures is a future call, not
a pending gap. (3) STALE COMMENT FIXED — the header said "Blade: excluded
(CPU mirror)" but the blade GPU path is LIVE (compute writes
GROUND_ATLAS_BLADE, the blade VS reads it, world.wgsl:10841/10855).
GATES: glaw1 GREEN; score census GREEN; world.wgsl clean UTF-8/LF, no CR.
RIG: GREEN — b2b observable ride confirmed (standing structures rise with
GoL zones; no change where gol is off/absent).
DELTA 2 (pyramid drape/union): WITHDRAWN (Jean) — NO bug; additive
base+pyramid is correct and always has been; the drape/union idea was a
misread of a metaphor. Composition stays as-is: not fixed, not
mark-intent.
b2a STATUS: LANDED as its own commit 7d30038 (rig-green before b2b) —
manifold_overlay_stack dedups the four query_ground_* bodies into one
additive core. THE b2 THREAD IS FULLY CLOSED: b2a landed; b2b GoL-ride
landed; pulses / waves / Delta-2 resolved-closed.

## PATCH GENERATION + SPAWNING — RECON (read-only; audit/PATCH_GEN_SPAWN_RECON.md)
The candidate API for the two systems + their seam. Read-only, nothing
moved; delivered as ONE report, STOP for the design conversation. Method:
an 8-way parallel deep-read workflow (A residency/LOD-vis/gen; B sampler/
packing/grounding/population; the seam+wire) over an anchor (theory v3
§9/§12/§0 + the standing record), synthesized, completeness-critiqued (12
gaps), revised; load-bearing file:lines spot-verified by hand.
SHAPE: §1 ownership table · §2 ~29 unit cards (offer/requires face, home,
bit-identity, C++/WGSL) · §3 the seam · §4 constraints · §5 questions +
dead register. BOUNDARY: stops at data-ready-for-GPU (the DTO/upload
wire), not the patch VS/FS or the draw.
KEY RESULTS: (1) the seam is clean — A meets B at generate_tile_state
under one (gx,gz) key; b4 split the TYPE-line only, but shape and pop draw
DISJOINT seed streams (tile_seed vs cpu_lattice_node_seed), which is why
the split is bit-clean and the TilePopulation physical relocation is
safe-but-deferred. (2) Sharpest forks: the LOD-banding block has no
callable home + braids the baked-sampler grid index; three
(gx,gz)->existence impls; the mesh-param map transcribed 3x; the bucket
walk reached the entity selectors but not the two theme selectors. (3)
Dead register an API must NOT enshrine: the whole "render" radius
vocabulary (RENDER_RADIUS/VISIBLE_RADIUS_SQ/LOD_FULL_RADIUS_SQ) is dead
(live gate = the world-space cylinder); cached_ground_y==0, ground_y_
offset/gpu_ground_y/theme_idx dead, entity_density a provable x1.0 no-op.
(4) Latent hazard: GPUPatchParams.resolution=256 is a hand-typed literal
decoupled from Dim::PATCH_HEIGHTFIELD_N=256 (no static_assert link).
No cut order, no sequencing. HELD for the design conversation.

## PATCH-GEN + SPAWN — THE CUT (stamped plan; per-commit)
Rulings + sequenced plan STAMPED (Jean). Governing rule: unify mechanism,
never touch the biography draw — bit-identity NONE merges freely, LIVE
stays forked. TWO recon reads corrected under the flag clause (Jean
accepted both as corrections, not overrides): Q2 — the O(1) set is a
per-scan LOCAL (patch_system.hpp:687), not a maintained member, so
find_patch STAYS O(N)/untouched and NO persistent invariant is introduced
(kill the fullRegen O(N^2) via a shared build_active_patch_set); Q3 —
antenna is the SAME GPUColumnMeshParams layout (antenna_write_gpu
entity_pipeline.hpp:637 vs build_column_mesh_params_from :329, proven by
the live reupload path :457), merge gated on FIELD-PARITY (a divergence
surfaces as a pre-existing commit-vs-reupload bug, NOT forced). Q7 names:
spatial_density / temporal_flavor. SEQUENCE: [1] band_patches · [2]
build_patch_grid · [3] Q2 · [4] Q3 · [5] Q9 · [6] Q8 · [7] Q5 · [8] Q4 ·
[9] Q6a key-helper · [10] Q6b relocate (RIG) · [11] Q7 rename. Baked-
sampler-path commits to watch: 2 (grid) + 10 (Q6b), non-colliding
(grid=shape/layer, population=CPU-only). Batch-review breakpoints: after
2, after 8, after 11. Gate class: all gate-only except Q6b (rig, cross-
module); Q1 ruled gate-only, rig smoke-check optional.

=== commit 1 — band_patches (the conductor split, part 1) ===
Extracted the visibility/LOD banding tail block of stream_patches into
band_patches(c, queue) (patch_system.hpp, defined just above the
conductor): walk GENERATED patches -> gate on the visibility cylinder
(finite = all-visible) -> split LOD0/LOD1/pregen -> pack [lod0|lod1|
pregen] -> upload instances + lod0/render/all counts + placement_patch_
count + lod_pawn (the anti-flicker push). PURE extraction, byte-identical
(same ops, same order); stream_patches now CALLS it. The patch-grid block
stays inline (commit 2). offer-face: GPUPatchInstance[] + counts +
lod_pawn. Did NOT carry the dead "render" radius vocabulary
(RENDER_RADIUS/VISIBLE_RADIUS_SQ) into the offer-face — flagged, not
enshrined. stream_patches moves from organ toward sequencer.
GATE: gate-only (bit-safe, pack order = wire layout, not a draw). glaw1
GREEN (real C++ compile — extraction valid); score census GREEN;
patch_system.hpp clean UTF-8/LF, no CR. Behavior-identical.

=== commit 2 — build_patch_grid (the conductor split, part 2) — CHAPTER DONE ===
Extracted the patch-grid block into build_patch_grid(c, queue): its own
walk over the GENERATED patches building the GPUPatchGrid (origin + side +
per-cell layer+1, 0=empty) that sample_terrain_y_at hashes into — the
baked-sampler O(1) index. Kept SEPARATE from band_patches (different
consumer/offer-face; they never shared the walk — two independent walks in
the old block, confirming the ruling's "perf coincidence"). Flattened the
now-vestigial wrapper `{}`: the conductor tail is now a SEQUENCE of named
units — `band_patches(c,queue); build_patch_grid(c,queue);` — stream_
patches is a SEQUENCER, not an organ. Byte-identical (same ops, same
order; locals moved from block-scope to function-scope, same lifetime).
BAKED-SAMPLER-PATH commit (Jean's rig-attention flag #1 of 2) — grid =
shape/layer, does NOT collide with Q6b (population, CPU-only).
GATE: gate-only (bit-safe, index layout = wire layout). glaw1 GREEN;
score census GREEN; encodings clean. Behavior-identical; rig smoke-check
optional (a baked-sampler-path commit — first place to look if it twitches).
THE CONDUCTOR CHAPTER (Q1) IS COMPLETE. Breakpoint: after-2 (Jean's batch
review). Dedups + guards (commits 3-8) ride next.
CHAPTER STAMPED (Jean): build confirms commit 2 behavior-identical.

=== commit 3 — Q2: kill the fullRegen O(N^2) via build_active_patch_set ===
Extracted the (gx,gz) existence set into build_active_patch_set(c)
(patch_system.hpp) — the per-scan LOCAL alloc already used inline, now
shared. fullRegen's raw O(N) inner scan (per window cell) → one pre-loop
set + O(1) .count(); alloc's inline build → the helper call. find_patch
UNTOUCHED (stays the O(N) single-lookup handle). Bit-identical: both scans
enumerate all active-count entries (no .valid filter, matching the
originals); the pre-loop set == the fresh per-cell scan because each
window cell is unique (an earlier-added patch never matches a later cell).
The flagged premise correction holds — NO persistent member, NO alloc/
evict/compaction invariant introduced (the CARE note was moot).
GATE: gate-only. glaw1 GREEN (helper + both sites + GridKey visibility +
no name conflict); score census GREEN; encodings clean. Behavior-identical.

=== commit 4 — Q3: one build_mesh_params (commit paths → build_from) ===
The two commit-path inline builds (column_write_gpu entity_pipeline.hpp:478,
antenna_write_gpu :637) now route through build_column_mesh_params_from
(spawn_engine.hpp:329) — the same builder the reupload/cull path uses — so
there is ONE producer of GPUColumnMeshParams. FIELD-PARITY GATE PASSED (the
stamped gate): hand-verified all ~20 fields for BOTH families — write_active
(column :451 / antenna :610) runs before write_gpu and stores every field
build_from reads (segs/rings from COLUMN_TIERS vs ANTENNA_TIERS resp.,
already baked into the ActiveColumn), so build_from(the just-written
ActiveColumn) reproduces each inline mp byte-for-byte. NO divergence → NO
pre-existing bug surfaced (had one diverged, it would have been flagged, not
forced). Removed antenna's now-unused raw_tier. Visibility: spawn_engine.hpp
precedes entity_pipeline.hpp in cartridge.hpp, so the builder is in scope
(glaw1 confirms — real compile).
GATE: gate-only (DTO pack, parity-verified). glaw1 GREEN; score census
GREEN; encodings clean. Behavior-identical.

=== commit 5 — Q9: GPUPatchParams.resolution from the Dim source ===
make_patch_params (patch_system.hpp:397) now sets p.resolution =
Dim::PATCH_HEIGHTFIELD_N instead of the 256 literal — one source of truth,
closing the silent-divergence surface (the same Dim const sizes the write
texture at state.hpp:3396 and the dispatch divisor at :2632; a literal in
the DTO could have drifted from the actual texel side with no static_assert
link). Byte-identical today (256==256).
GATE: gate-only (free guard). glaw1 GREEN; score census GREEN; clean.

=== commit 6 — Q8: TILE_GRID capacity guard (static_assert) ===
upload_tile_grid_now (tile_world.hpp:243) builds tileGridSide =
2*(active_radius + TILE_PAD)+1 into GPUTileGrid.entries[TILE_GRID_MAX=289]
(17^2, sized from PATCH_PREGEN_RADIUS+1). Overflow iff tileGridSide^2 > 289
<=> active_radius > PATCH_PREGEN_RADIUS. Both the DTO side and the
active_radius clamp (set_render_radius; finite cap lower) track
PATCH_PREGEN_RADIUS, so the only free variable that could overflow is
TILE_PAD (the DTO's pad is +1). Added static_assert(TILE_PAD <= 1) with the
full-invariant comment — closes the compile-time half; the runtime half is
the existing active_radius clamp. At active_radius=7 the built side is
exactly 17 (fills the DTO, zero margin) — the guard makes that structural
fact enforced, not incidental. No behavior change (guard only).
GATE: gate-only (free guard). glaw1 GREEN; score census GREEN; clean.

=== commit 7 — Q5: one tier-weight accessor (the plugs collapse) ===
The 9 per-family *_get_theme_tier_weights plugs collapse into ONE accessor
theme_tier_weights(theme_idx, family_id) (population_themes.hpp) — a
family->member switch/map. The generic pipeline keys it on
traits.family_id, which EntityFamilyTraits already carries
(entity_types.hpp:133), so the consumer is clean; the get_theme_tier_
weights fn-ptr is REMOVED from EntityFamilyAdapter and from all 9 positional
initializers, and the 9 wrappers are deleted. Tables untouched; bit-safe
(same const float* into the PopulationTheme row). BREADTH NOTE: this is the
batch's widest commit — 6 files (population_themes/entity_types/entity_
pipeline/entities/spheres/cube_behaviors), a struct change + 9 initializers.
It is NOT riskier for it: the adapter fn-ptrs are type-distinct, so any
positional misalignment is a glaw1 compile error, not a silent swap — no
silent-bug surface. Ribbon's inline tier_wt_ribbon use is bespoke (not an
adapter plug), left as-is. Fixed a stale THEMES boundary comment.
GATE: gate-only (pointer return). glaw1 GREEN (generic + struct/init
alignment + zero dangling refs + call-site resolves); score census GREEN;
encodings clean; residual get_theme_tier_weights refs are comments only.
Behavior-identical.

=== commit 8 — Q4: document the fork (REJECT — leave forked) ===
Doc-only. select_weighted (seed_utils.hpp) is STATELESS (pre-normalized
weights, one draw); the two theme selectors are a different shape:
select_theme_at_node NORMALIZES inline against the live weight-sum, and
evaluate_theme_envelope is a STATEFUL SEQUENCED sampler (its per-patch call
sequence IS the biography). Added reciprocal comments at select_weighted
and select_theme_at_node explaining why they stay separate (not one
abstraction twice; merging would touch a draw path for a 2-site payoff with
only rig-level bit-catch). No migration — Q4 REJECT ruling.
GATE: gate-only (doc). glaw1 GREEN; score census GREEN; clean.

=== AFTER-8 BREAKPOINT (dedups + guards complete) ===
Commits 3-8 landed, all gate-only, behavior-identical: Q2 (kill fullRegen
O(N^2)), Q3 (one build_mesh_params, parity-verified — no bug surfaced), Q9
(resolution from Dim), Q8 (TILE_GRID static_assert), Q5 (9 tier-weight
plugs -> one accessor, 6 files), Q4 (fork documented). Two premise
corrections held (Q2 no persistent set; Q3 clean parity). REMAINING: the
seam — [9] Q6a tile_key helper, [10] Q6b relocate TilePopulation (RIG,
cross-module), [11] Q7 rename spatial_density/temporal_flavor. HELD for
Jean's after-8 batch review.
BATCH STAMPED (Jean): build green, pipelines clean. Into the seam; order
Q6a → Q7 → Q6b (Q6b = the one real rig pass, per-commit stamp; 9+11 ride
ahead).

=== commit 9 — Q6a: the tile_key helper (one key derivation) ===
Extracted the world-xz → tile grid key into tile_key(wx,wz)->GridKey
(tile_world.hpp) = the ONE (int)floor(w/PATCH_EXTENT). Repointed the three
recompute sites: F1 estimate_terrain_height, F2 terrain_tile_warm
(tile_world), and negotiate_position's host_gx/gz (spawn_engine — visible
via the include order, tile_world precedes spawn_engine). Bit-identical
(same floor form → same keys). The prereq for Q6b's relocate.
GATE: gate-only. glaw1 GREEN (helper visibility + GridKey.x/.z + repoints);
score census GREEN; the only residual floor(w/EXTENT) is inside tile_key.
Behavior-identical.

=== commit 11 — Q7: rename to spatial_density / temporal_flavor ===
Rename-only (no semantics), so the two independent axes stop reading as
duplication. theme_spawn[] -> spatial_density (TilePopulation, tile_world:
decl + write + F3 read) = the SPATIAL axis (per-family position-locked
density, applied by F3 tile_apply_spawn_mult). active_theme_idx_ ->
temporal_flavor (ThemesState, population_themes decl + write; spawn_engine
read; spawn_services comment) = the TEMPORAL axis (drifting theme index ->
tier weights). Distinct identifiers (no collision with theme_idx). Added a
paired-axis note at each declaration. NOTE: a comment edit dropped the
spatial_density array's terminating ';' — glaw1 caught it (error at the
next member theme_idx), fixed, re-gated GREEN. The blind-catch worked.
Landed BEFORE Q6b so TilePopulation relocates under its final name.
GATE: gate-only (rename). glaw1 GREEN; score census GREEN; zero residual
old-name references; encodings clean. Behavior-identical.
=== commit 10 — Q6b: relocate TilePopulation into population_themes.hpp (RIG) ===
The seam's ONE cross-module commit. Moved the population half off tile_world
onto its vocabulary: (a) TilePopulation struct, (b) the DENSITY_* lattice
constants, (c) a new generate_tile_population(active_seed,gx,gz)->TilePopulation
holding the two authoring blocks verbatim (entity-density field + theme field).
generate_tile_state now calls it: `ts.pop = generate_tile_population(c->world_
state_.active_seed, gx, gz);`. TileState keeps `TilePopulation pop;` — complete
because population_themes (:59) precedes tile_world (:61) in the cohort; F3
tile_apply_spawn_mult STAYS in tile_world (reads tileCache_).
LADDER CONSTRAINT solved: population_themes sits at :59, BEFORE surface_services
(:60, PATCH_EXTENT alias + WorldState) and tile_world (:61, DENSITY_*). So the
move required pulling DENSITY_* along (they're population-authoring inputs, not
shape) and two substitutions that preserve the bytes: PATCH_EXTENT ->
Dim::PATCH_EXTENT (state.hpp :58, same constexpr 50.0f), and the WorldState
deref lifted to the active_seed parameter (uint32_t, read once = same value).
Added #include <cmath> (std::floor/std::pow, was transitive via seed_utils).
BIT-IDENTITY: LIVE (hash arithmetic) but byte-identical BY ARGUMENT — every
op/constant/loop-order/seed-band unchanged; only symbol resolution differs and
both resolve to the identical value. Disjoint from the shape draws (tile_seed
props) that stay in tile_world — the b4 TYPE-line split still holds; the one
(gx,gz) generation moment and the shared call site are untouched.
GATE: rig (cross-module, live draw). glaw1 GREEN (Dim:: visibility at :59,
TilePopulation complete at TileState, no dangling DENSITY_*/select_theme_at_node
in tile_world); score census GREEN. RIG-HELD: this is the one commit that is
byte-identical by argument, not by construction — HELD for Jean's deliberate rig
pass (watch spawn density + themes in a GoL-active world; confirm placement /
density / flavor all unchanged) and per-commit stamp.
SEAM STATUS: 9 (Q6a) + 11 (Q7) + 10 (Q6b) all cut & pushed. The 11-commit
patch-gen + spawn cut plan is COMPLETE pending Jean's Q6b rig stamp.

## GoL TWO-BUG DIAGNOSIS (read-only) + the b3 camera fix (pulled forward)
Two GoL symptoms in the new build, diagnosed read-only (two parallel Explore
agents + git archaeology), reported with file:line, then ONE fix cut per Jean's
ruling.
S1 — GoL frozen (no beat-time evolution). MECHANISM: the Conway rule is GPU-
gated on zone_config.tick_mask (world.wgsl:7516); the CPU producer
(upload_gol_zone_config, gol_zones.hpp:549-568) sets a zone's bit only when
floor(time_state_.beats / tick_period) crosses an integer. The GoL clock is
time_state_.beats = signal.t_beats = the DAW transport position (port_.beats(),
canvas.hpp:141), NOT wall-clock. Springs/mesh/pawn-aura run on wall-clock dt —
so the pawn still deforms cells while Conway is frozen. git range c312440..HEAD
touched NONE of gol_zones.hpp / canvas.hpp / spine_state.hpp / the beat-block —
the tick path is unchanged by the campaign. GPUPatchParams.time (=0) is a red
herring (never read by the GoL shader). RULING: S1 CLOSED — 1a, working as
designed (music off -> no beat -> static GoL). No fix. Transport was the missing
piece.
S2 — camera rides GoL. MECHANISM: the pawn-host camera clamp read
manifold_position(camera.pos, POLICY_FLYER).y (world.wgsl:6628); POLICY_FLYER
folds RAW contrib_gol_zones_at with NO suppression (mask :2188-2193), while the
pawn body uses POLICY_WALKER (gol*(1-supp), :2799-2812). So the body sat low
(suppressed) but the camera floor rode the un-suppressed GoL and lifted. ORIGIN:
a82ab7d ("migrate camera clamp to POLICY_FLYER"), PRE-TERRAIN-2 — b1-cohort
(byte-identical) and b2a (pixel-identical) only re-expressed it; b2b touched
compute_entity_placement (structures), not the camera. The campaign made it
PROMINENT (b2b's world-anchored GoL + S1's freeze pinned GoL up), not present.
"One shared cause" REFUTED: no shared dirty-flag (GoL uses tick_mask/
last_tick_index; terrain/placement use ground_entries_dirty/placement_dirty,
cartridge.hpp:1174-1182); Q6b's active_seed move never touched world_state_ or
any time/GoL read.
THE b3 CAMERA FIX (pulled forward, own commit; advances b3 task #52):
world.wgsl:6628 POLICY_FLYER -> POLICY_WALKER_TILT, and the clamp's qi
consumer_pos camera.pos -> pawn_pos. WHY WALKER_TILT (not WALKER): WALKER adds
contrib_pawn_aura_at_self() = config.pawn_aura_height, a CONSTANT peak (:2643) —
wrong to bake into the camera floor; WALKER_TILT is the walker's world surface
(base+pyramids+gol*(1-supp)+waves+pulses) MINUS that self-aura = exactly b3's
"live minus suppression". WHY consumer_pos = pawn_pos: genuine 2a centers the
suppression on the PAWN (matching the pawn body's own center, gol_zones/pawn
qi at world.wgsl:5872), so pawn-local GoL — where the body stands flat — does
not lift the camera, while world-anchored waves/pulses and GoL AWAY from the
pawn still clear the camera (no clip of animated ridges). consumer_pos = camera
would degenerate to full self-suppression (d=0 -> gol*0), i.e. 2b (GoL excluded
everywhere) — rejected; Jean chose 2a. The clamp block runs pawn-host only
(camera-host returns at world.wgsl:6579), so pawn_pos is always the real pawn.
SIDE EFFECT flagged for the rig: WALKER_TILT drops the external pawn aura the
flyer carried — the camera no longer rides the aura bump either (consistent with
"don't lift for pawn-local deformation", but rig-observe for aura-ridge clip).
GATE: blind WGSL (glaw1 compiles C++ only — GREEN, unchanged; census GREEN).
RIG-HELD: pawn walks into a GoL zone -> camera tracks the body (sits with it),
does NOT lift above it. Per-commit rig stamp; on green, b3 task #52 advances one
notch (the camera-suppression piece landed ahead of the finite collapse).

## RAYMARCH / SDF EXCAVATION (cleanup chapter, first cut; M5/M6 verified at HEAD)
Jean RULED raymarch GENUINELY-DEAD -> delete (not dormant-voice). Anchor: the
TERRAIN-1 M5 isolated-subsystem map + M6 disposition table. Per-item safety gate
BEFORE deleting: grep-at-HEAD reader-freedom (lines shifted since recon). Three
read-only verification agents ran the three clusters; ALL returned reader-free
(no VS/FS/CPU consumer). One agent discrepancy CAUGHT by the gate: IDLE_AMPLITUDE_
SCALE / HEIGHT_MAX_AMPLITUDE claimed "live" by one agent — grep proved they are
read ONLY inside the dead chain (coupling_signal_polyphony_to_terrain_amplitude +
dynamics_terrain_gradient_max, both confined to update_terrain_config). Deletable.

THE TRAP (navigated): two wave systems. The LIVE OVERLAY voice (OVERLAY_WAVES,
contrib_terrain_waves_at, terrain_wave_overlay_with_gradient, band_*) is DORMANT-
VOICE (wired into ~24 live VS sites incl. the manifold_overlay_stack) — LEFT
UNTOUCHED. Only the LEGACY WAVES table (the animated field the SDF marched) was
cut. Confirmed contrib_terrain_waves_at reads OVERLAY_WAVES, never the legacy WAVES.

=== commit 1 — the dead computation + the writer kernel (SAFE, glaw1-gated) ===
Deleted (world.wgsl): the legacy WAVES table (WaveComponent/WAVES/WAVE_COUNT/
HEIGHT_MAX_AMPLITUDE), the amplitude-trajectory feeder (IDLE_AMPLITUDE_SCALE/
ATTACK/RELEASE + coupling_signal_polyphony_to_terrain_amplitude), the lipschitz
chain (wave_enabled + dynamics_terrain_gradient_max — WAVES -> gradient_max ->
lipschitz_factor, a cone-march step bound read by nobody), and the whole
update_terrain_config @compute kernel (the sole writer of amplitude_scale +
lipschitz_factor, both unread). Deleted (C++): renderer.hpp Entry::UPDATE_TERRAIN_
CONFIG + updateTerrainConfigPipeline_ member + dispatch_update_terrain_config +
its pipeline-creation block; render_passes.hpp the per-frame dispatch call.
PRESERVE honored: finite-diff normals untouched; the stale "raymarch_get_direction
convention" comment in the VP builder FIXED (comment only); the "0D split into 5
entry points" comment corrected to 4.
GATE: glaw1 GREEN (all C++ buffer/dispatch/pipeline refs resolve after deletion);
no dangling refs (only breadcrumb comments name the removed symbols). RIG: PIXEL-
IDENTICAL — the whole limb was write-only, so removing it cannot move a pixel.
FREED GPU RESOURCE: one per-frame compute dispatch (update_terrain_config,
1x1x1) eliminated. (The buffer itself is not yet freed — see the flag below.)

=== commit 2 — the one entangled wire (surgical, inside a LIVE kernel) ===
terrain_state.tint was a DEAD STORE inside the live update_sphere kernel. The
nearest-active-sphere search that computed it wrote ONLY terrain_state.tint
(read by nobody); its locals fed nothing live. Deleted (world.wgsl): the whole
tint block inside update_sphere, coupling_sphere_to_terrain_tint (its sole
producer), and SAND_DUNE_CENTER/SAND_DUNE_VARIANCE (used only by that coupling).
PRESERVED: update_sphere's live work (slot eviction + orbital motion via
compose_sphere_from_orbit_pga + polyphony->color, all writing floating_entities)
is above the tint block and untouched — the fn closes cleanly after the excision.
The COUPLING_SPHERE_TO_TERRAIN_TINT bitmask slot is left as-is (an enum bit,
harmless once unreferenced — not renumbered).
GATE: glaw1 GREEN (C++ untouched; WGSL-only, blind — rig-gated); score census
GREEN. RIG: PIXEL-IDENTICAL — a dead store into an unread buffer; removing it
cannot move a pixel. Executed SECOND (after commit 1) but order-independent:
neither commit removes the buffer, so neither dangles.

FLAG — the buffer/binding/struct HUSK is a LAYOUT/STORAGE-WELD follow-on, NOT cut
here. The verification is 100% clean (reader-free), but the PHYSICAL removal of
the terrain_state buffer + bindings 20 (compute) / 220 (fragment) + GPUTerrainState
struct + the GPUConfig wave fields (wave_enable_mask/wave_freeze_mask/wave_frozen_t)
is a glaw1-BLIND risk class: (1) bindings 20/220 live in five fixed std::array<
BindGroupLayoutEntry/BindGroupEntry, 19> blocks (compute layout+group, render
layout, main+photographer render groups) — removal needs entry re-index + count
shrink, and glaw1 (syntax-only) cannot validate the runtime bind-group layout;
(2) removing the GPUConfig wave fields shifts world_seed's offset (LIVE, used
everywhere) in a heavily-used uniform with NO size assert, WGSL<->C++ lockstep
required. Same risk class Jean carved out for the complexity texel. After commit 1
the buffer is a fully-dead husk (no writer, no reader) — behavior-neutral,
allocated-but-unused. RECOMMEND: its own commit with a CRASH-AWARE rig gate (does
the app launch — bind-group validation — in addition to pixel-identical), OR fold
into the complexity-texel storage-weld follow-on.
RULING (Jean): B — DEFER, do NOT cut now. Post-commit the husk is allocated-but-
fully-dead (no writer/reader/dispatch), costing only a static allocation; its
removal is a glaw1-blind offset-shift + 5 bind-array re-indexes whose failure mode
is a CRASH / corrupted world_seed, NOT a pixel-diff — folding it in would poison
this campaign's pixel-identical proof model. BATCHED with the complexity texel as
ONE storage-weld follow-on under a crash-aware gate (app launches + bind-group
validation passes). Marker dropped at the GPUTerrainState decl (state.hpp) —
"DEAD: no writer/reader post-851ce68; awaiting storage-weld removal (see LADDER)"
— so the husk doesn't read as live infra to the next sweep (the gradient-
scaffold's "not dead" marker, inverted).
(Ordering note: the husk removal must come AFTER commit 2's tint-store removal,
since that store writes the buffer.)

=== STORAGE-WELD FOLLOW-ON (parked; own crash-aware rig gate) ===
Two items, one risk class (format/layout weld, glaw1-blind at runtime):
- the TerrainState/render_terrain HUSK: buffer + bindings 20/220 (5 bind-array
  re-indexes) + GPUTerrainState struct + GPUConfig wave fields (world_seed offset
  shift). Reader-free (verified); marker in place.
- the complexity texel channel (.w of the heightfield rgba16float) — dead, removal
  changes the TEXTURE FORMAT.
GATE when pulled: app launches + bind-group/pipeline validation passes, THEN
pixel-identical. Not this campaign's pixel-only gate.

## RENDER/UPDATE — CABLE MANAGEMENT (opening: C1, then C3/C4)
Anchor: the render/update recon (bf9c237). GOVERNING FRAME (Jean): the frame's
graph is buried under redundancy, not absent — each redundancy collapsed is an
edge the graph was forced to fork. We excavate the graph BY removing the noise;
behavior-neutral cable management that reveals structure, not a redesign. NO
model decisions this campaign.
TWO STANDING DISCIPLINES (every cut): (1) NOTE THE REVEALED EDGE — each entry
carries a "GRAPH EDGE REVEALED:" line so the graph accretes from the margins.
(2) COLLAPSE MECHANISM, NEVER A DISTINCTION — unify only what is genuinely the
same thing written N times; if a unification needs per-case flags/branches to
stay behavior-identical, STOP and FLAG — that resistance is a REAL FORK (a graph
edge), not noise. A partial collapse with a flagged remainder beats a forced one.

=== C1 — the vertex-format size asserts (Tier 0; glaw1-terminal) ===
Added static_assert(sizeof(ArchVertex)==40) + static_assert(sizeof(ShellVertex)
==36) to the state.hpp assert block, pinning the two GPU-written/GPU-read vertex
mirrors that had NO guard (the recon's standout latent hazard) to the vertex-
buffer arrayStride the render + shadow pipelines declare (archVBL/shadowArchVBL
=40 renderer.hpp:2141/2681; shellVBL/shadowShellVBL=36 :2241/2776) — which is ALSO
the WGSL ArchVertexInput/ShellVertexInput layout. The assert value is the GPU/WGSL
CONTRACT, not a C++ self-reference, so it catches future C++<->WGSL drift.
GATE: glaw1 GREEN — both asserts PASS -> the byte layout is correct TODAY,
behavior-identical, glaw1-terminal (no rig; a failing assert here would have been
a bug-find, per the discipline, but none surfaced).
GRAPH EDGE REVEALED: ArchVertex is a SHARED vertex format across SIX mesh-gen
families — arch, column, palm, cactus, blade, pyramid all size their VB + bind
entry via sizeof(ArchVertex) and their VS reads ArchVertexInput (state.hpp
2990/3034/3073/3101/3127/3163; 5308/5333/5358/5383/5406/5427; world.wgsl arch_vs/
column_vs/palm_vs/cactus.../blade.../pyramid_vs). ShellVertex is the indoor-shell
format (ceiling/walls/floor). The "six families, one vertex format" edge was
implicit in the shared sizeof; it is now compiler-enforced.

=== C3 (part 1 — the COMPUTE side) — the compute-pipeline creation collapse ===
All 30 compute pipeline creations routed through TWO helpers on Renderer:
computeLayoutFor(bgl) (wraps a single bind-group layout into a pipeline layout —
the ~24 dedicated compute pipelines each repeated that 4-line boilerplate) and
makeComputePipeline(label, dbgLabel, layout, entry, out&) (the uniform creation
ALL 30 shared: desc.label/layout/module=shaderModule_/entryPoint + Create +
null-check). HARD CONSTRAINTS honored: every entry string passed VERBATIM as
Entry::X (the sole C++->shader link); the SAME 30 pipelines are built (the 4
render orphans are C2's problem, untouched); no pipeline gains/loses a binding.
VERIFIED: 30 call sites, exactly one CreateComputePipeline remains (inside the
helper), zero tPipe left in createComputePipelines, all 30 Entry->member pairs
cross-check by name (UPDATE_CAMERA->updateCameraPipeline_, ...) — the mitigation
for the one blind axis (entry/member transcription).
Two boot-only, non-pixel side effects, noted for the rig: (1) the layout-build
moved just outside the tPipe timing block, so the boot-leaderboard ms for
dedicated-layout pipelines now excludes the trivial layout creation; (2) repeated
per-family/per-orb/per-zone `if constexpr (ROSTER.x)` gates consolidated to one
per group (same compile-time condition; a disabled group now also skips its
now-pointless layout build — that layout was unused when the pipeline was skipped).
GATE: glaw1 GREEN. HELD for Jean's BOOT CHECK (all 30 compute pipelines still
create — the glaw1-blind failure mode is a wrong-but-valid entry string) +
pixel-identical rig.
GRAPH EDGE REVEALED: every compute pipeline is a pure (bind-group-layout,
shader-entry) pair over the ONE shaderModule_ — the descriptor carried no other
per-pipeline state; that is now literal in makeComputePipeline's signature. The
LAYOUT is the only fork, and it partitions the 30: 6 world-updates share the
pre-built liveContrib/compute layouts; ~24 carry dedicated single-BGL layouts
(patch-gen x3 share patchGenLayout; orb x3 share; zone x2 + zone-mesh x3 share;
the 6 mesh-gen families each own one). The frame's compute graph = 30 nodes over
~a dozen distinct bind-group layouts.
DISCIPLINE-2 FLAG (a REAL FORK, not forced): the 34 RENDER pipelines did NOT fold
into makeComputePipeline — they RESIST because their descriptors genuinely diverge
(vertex-buffer layout/stride, topology, cull, depth, blend, dual-FS, the
USE_PATCH_INDIRECTION override). That resistance is the edge: render pipelines are
a genuine category carrying real per-pipeline GPU state, NOT "the same thing
written N times" the way compute is. Deferred to C3 part 2 (own commit) for two
reasons: (a) risk isolation — a subtle render-descriptor diff is a PIXEL change,
not glaw1-catchable, unlike compute (which only selects which kernel runs); (b) the
render side has its OWN sub-collapse to earn its own edge-notes — the entity-render
pipelines (arch/column/palm/cactus/blade/pyramid + pawn/sphere/monolith) already
share a mutated desc + ENTITY_FS (an entity-builder candidate), the shadow
pipelines are a depth-only category, and terrain/zone/orb/fade/gallery/wall-painting
are the genuinely-unique specials. Per Jean's sequencing (separate commits let the
rig isolate + keep the edge-notes clean).

=== C3 (part 2a — the RENDER side, ENTITY category) — the entity-render builder ===
(Cut AFTER C1 + C3-compute rigged green — no second blind cut stacked on an
unrigged one.) The 10 ENTITY_FS render pipelines (pawn, sphere, monolith, arch,
column, palm, cactus, blade, pyramid, shell) routed through one builder — a
makeEntity(label, dbgLabel, vsEntry, vbl*, cull, out&) lambda inside
createRenderPipelines that captures the shared scaffold (renderLayout +
depthStencil + colorTarget + ENTITY_FS + TriangleList + frontFace CCW). Entry
strings passed VERBATIM; same 10 pipelines built (pyramid render stays built — it
is an orphan but its removal is C2, untouched here); no binding changes. The
per-format vertex-buffer-layout construction (meshVBL 24 / archVBL 40 / shellVBL
36 / bufferless) stays inline at the call site — the genuine fork.
DISCIPLINE 2 HELD (the important part): cullMode did NOT fold flat — it is a
parameter, because it is a REAL per-pipeline field. Single-sided quads
(column/palm/cactus/blade + pawn + shell) use CullMode::None; solids
(sphere/monolith/arch/pyramid) use Back. Forcing one cull would have flipped
backface culling on the single-sided families — a PIXEL change. The Back-vs-None
split is an edge, so it survives as an argument, not a flag buried in the helper.
BYTE-IDENTITY: same 10 descriptors field-for-field (VS/VBL/cull each verified);
the only structural change is the shared-desc-mutated-in-place pattern
(sphere->monolith; arch->column->...->pyramid) becomes a fresh desc per call —
identical result. Also collapsed the shell's redundant double-nested
`if constexpr (ROSTER.indoor_shell)` to one (same compile-time condition).
GATE: glaw1 GREEN (10 makeEntity calls, 24 untouched render creations + 1 in the
helper). HELD for Jean's BOOT CHECK (all entity pipelines create) + pixel-identical
rig — this is descriptor-sensitive, so a wrong cull/VBL is a pixel diff, not a
compile error.
GRAPH EDGE REVEALED: the entity-render category = one FS (ENTITY_FS) + one layout
over {3 vertex formats: bufferless / MeshVertex-24 / ArchVertex-40 / ShellVertex-36}
x {2 cull modes: Back solids / None single-sided}, selected per pipeline. That
product — 3(+1)formats x 2 culls — IS the structure of the render half's first
category; compute had no such product (it was pure (layout, entry)), which is
exactly why render carries the model and compute didn't.
STILL AHEAD (this cut's siblings, each its own commit + rig): part 2b the
SHADOW/DEPTH builder (depth-only, fragment=nullptr, shadowRenderLayout, differ by
shadow-VS + VBL); part 2c the genuine SPECIALS stay bespoke (patch-terrain +
USE_PATCH_INDIRECTION override, zone-extrusion, ribbon, orb, fade, gallery-frame,
dual-FS wall-painting) — flagged, not collapsed.

=== C3 (part 2b — the RENDER side, SHADOW/DEPTH category) — the shadow builder ===
(Cut after part 2a rigged green.) The 13 shadow pipelines (patch_terrain, pawn,
sphere, monolith, arch, column, palm, cactus, blade, pyramid, shell, ribbon,
zone_extrusion) routed through a SECOND builder makeShadow(label, dbgLabel,
vsEntry, vbl*, cull, out&) inside the shadow cluster block, capturing
shadowRenderLayout + shadowDepth (Depth32Float shadow-map state). Entry strings
verbatim; same 13 pipelines; no binding changes. Per-format VBL construction
(shadowMeshVBL 24 / shadowArchVBL 40 / shadowShellVBL 36 / zone 44 / bufferless)
stays inline — the fork. All 13 cull modes verified against the original (patch/
sphere/monolith/arch/pyramid/zone Back; pawn/column/palm/cactus/blade/shell/ribbon
None). Collapsed the shadow-shell's redundant double-nested indoor_shell gate.
The 2 ORPHAN shadows (shadow_gallery_frame, shadow_wall_painting) stay bespoke,
embedded in their gallery/wall-painting special blocks — untouched (2 of the 4
orphans; their removal is C2).
DISCIPLINE 2 (the category boundary): makeShadow is a SEPARATE builder from
makeEntity, NOT makeEntity(isShadow=true). Color-vs-depth is a real boundary —
different layout (shadowRenderLayout), different depth state (Depth32Float shadow
map vs the main depth), and NO fragment. Folding them into one builder with an
isShadow flag would bury that edge under a branch; two builders keep it explicit.
GATE: glaw1 GREEN (13 makeShadow calls; residual 13 CreateRenderPipeline = 9
bespoke specials + 2 orphan shadows + the 2 builder lambdas). HELD for Jean's
BOOT CHECK (all shadow pipelines create) + pixel-identical rig (shadows are
descriptor-sensitive: a wrong cull would show as shadow acne / peter-panning).
GRAPH EDGE REVEALED: the shadow/depth category is the ENTITY category projected
onto depth — same families, same vertex formats, same Back/None cull split, minus
the fragment shader + color target, plus the Depth32Float shadow-map depth state.
So the render half has exactly TWO node-types (makeEntity, makeShadow), and the
shadow half = the color half with the color stripped and the depth swapped. The
two builders ARE the render graph's node-types; the specials (part 2c) are the
non-conforming remainder that keeps its own shape.

=== C4a — the upload collapse, Shape A (helpers + dirty-driven whole-writes) ===
REFRAME (Jean's, on record): the shape partition IS a CADENCE partition, so C4's
edge-notes are the graph's first TEMPORAL edges. Added three shape helpers to
GPUState — writeStruct<T> / writeSlot<T> / writeArray<T> — each DERIVING the write
size from the value's type, so a write can never carry a mismatched sizeof (the
recon's silent-corruption class, removed by construction). Converted the 18
Shape-A upload methods (signal, config, directional/point/spot lights, patch_params,
tile_grid, patch_grid, ribbon, photographer_vp/camera/config, pyramids,
pawn_aura_config, orb_config, zone_config, zone_derive_requests, portal_array)
to writeStruct.
PER-SITE CHECK (Jean's gate) ran on every site: original size == sizeof(the
argument's actual type). All 18 passed. TWO correctly EXCLUDED as partial writes
wearing Shape A's clothes: upload_zone_config_header (writes only the 16-byte
header of the bigger zone buffer — "does NOT overwrite per-zone configs") and
deactivate_zone_slot (an offsetof field write). Those stay bespoke.
GATE: glaw1 GREEN (18 writeStruct calls; the only residual WriteBuffer(buf,0,&v,
sizeof) are the helper body + the boot writes — see flag). HELD for Jean's rig
(one build between a and b).
FLAG (out of scope, a FOURTH cadence): initializeState()'s 6 boot-init full-struct
writes (config_/terrain/camera/floating-entity/ribbon/zeroVP, state.hpp ~5545-5699)
are Shape-A-shaped but are the BOOT-ONCE tempo, not the per-frame upload API — left
untouched; a future consistency pass could route them through writeStruct too.
GRAPH EDGE REVEALED (temporal): Shape A is the frame's DIRTY-DRIVEN whole-write
cadence — the "the whole DTO changed, replace it" tempo, pushed when its CPU-side
state is marked dirty (canonical: upload_config's configDirty_ guard). This is
one of three write tempos the upload surface runs: A dirty-driven whole-writes
(here), B commit-driven slot-writes + C count-driven arrays (C4b), and the ~25
bespoke offsetof field-writers = the per-frame HOT fields (a third tempo, named
in C4b). The upload API is not one thing — it is three clocks.

=== C4b — the upload collapse, Shapes B+C (commit-slots + count-arrays) ===
(Cut after C4a rigged green — build + pixel-identical.) Converted 20 sites: 10
Shape-B slot writes -> writeSlot (floating_entity_slot, pier_slot, painting_slot,
agent_slot [guard + *src], arch/column/palm/cactus/blade/pyramid_mesh_params_slot)
and 10 Shape-C array writes -> writeArray (agent_registries x2, patch_instances,
painting_slots, agent_state_all, arch/column/pyramid_origins [std::min clamp
preserved as the count arg], shell_mesh x2). PER-SITE CHECK ran on each (offset ==
slot*sizeof(T); size == sizeof(T)*count). The offset arithmetic that was hand-typed
at 19 sites now lives in the TWO audited formulas inside writeSlot/writeArray.
SUB-FORK (Discipline 2): the armed writeSlot `base` param went UNUSED — no clean
full-slot writer has a byte base (the cube region's offset is index arithmetic in
a delegating wrapper: upload_cube_entity_slot -> upload_floating_entity_slot with
CUBE_SLOT_OFFSET+slot). Left armed for future header-before-array slots.
A DIFFERENT fork FLAGGED, left bespoke: upload_patch_staging is an ARRAY write at a
SLOT offset (WriteBuffer(buf, offset*sizeof(T), params, sizeof(T)*count)) — fits
neither formula (writeArray is offset-0, writeSlot is single-item); a genuine
array-at-offset one-off, not forced.
GATE: glaw1 GREEN (10 writeSlot + 10 writeArray). HELD for Jean's rig (the build
AFTER b).
GRAPH EDGE REVEALED — the CADENCE TAXONOMY completed. The upload surface runs on
FOUR clocks, now legible in the code:
  A writeStruct — DIRTY-driven whole-buffer writes (the whole DTO changed; config
    signal lights patch/tile/grid ribbon photographer pyramids aura orb zone portal)
  B writeSlot   — COMMIT-driven slot writes (one entity spawned/evicted at a time;
    floater/pier/painting/agent + the 5 family mesh-params slots)
  C writeArray  — COUNT-driven array writes (a whole set rebuilt; patch_instances,
    painting_slots, agent_state_all, the *_origins arrays, registries, shell_mesh)
  HOT (bespoke, NAMED) — PER-FRAME field writes, the fastest clock: the ~25 offsetof
    field-writers written in isolation to avoid re-uploading a whole struct —
    resync_sky_head, config sub-writers (pier_count/placement_patch_count/lod_pawn),
    stage_spot_vps, ribbon_time/color/wave_amps, the cube kite-state writers, the
    12-method orb frame cluster, pawn_aura_frame, zone_config_header, zone_life.
Out of band (a fifth, slowest cadence, out of the per-frame upload scope):
reset/boot verbs — reset_player_agent, reset_frustum_indirect, initializeState's
boot writes. The "upload API" was never one surface; it is a clock tower.
=== CABLE MANAGEMENT — OPENING COMPLETE (C1 + C3 + C4). The render/update surface
now carries its own structure in the cleaned code: the pipeline graph (compute =
(layout,entry); render = makeEntity/makeShadow over {format}x{cull} + irreducible
specials) and the upload cadence taxonomy (dirty/commit/count/hot). Parked next
cuts, unblocked as the graph is now clearer: C2 orphan sweep (+ pyramid ruling),
C5 family/draw table, C6 binding registry (the storage-weld climax that also
retires the parked TerrainState husk), C7/C8 ordering barriers. ===

## CABLE MANAGEMENT C2 — ORPHAN SWEEP (verbs now, nouns → C6; the CAST
## ruling made structural; own commit; glaw1 + pixel-identical rig)
Jean's ruling: the pyramid mesh is dead-BY-DESIGN — never meant to render;
the live pyramid path is buffer → ground atlas → patch-gen kernel → heightfield
(untouched). VERIFY FIRST (grep-not-memory, both passes, all draw lists):
the four orphans — draw_pyramid, draw_shadow_pyramid, draw_shadow_gallery_frames,
draw_shadow_wall_paintings — carry ONLY their definitions, NO callers anywhere.
The shadow trap held clear: render_passes.hpp shows pyramid ONLY as
upload_pyramid_origins (the LIVE placement noun, lines 106-120), NOT a draw; the
gallery-frame and wall-painting COLOR draws are live but their SHADOW variants
were never called (frames/paintings never cast a mesh-shadow — the terrain bake
already carries their footprint). Score census + glaw1 both GREEN post-cut.

CUT (verbs; ~309 net lines): WGSL — pyramid_vs, shadow_pyramid_vs,
shadow_gallery_frame_vs, shadow_wall_painting_vs, pyramid_mesh_gen kernel entry.
renderer.hpp — 5 Entry:: strings, 5 pipeline members, dispatch_pyramid_mesh_gen,
the 4 draw methods, 5 creation blocks (mesh-gen compute + makeEntity pyramid +
makeShadow shadow_pyramid + the two bespoke shadow_gallery_frame/shadow_wall_
painting blocks), and the pipelines_skipped() counts (pyramid line removed;
gallery 6→4). cartridge.hpp — the two pyramid mesh wrappers deleted, the
FAMILY_DISPATCH pyramid row's mesh hook routed to the none-fork
(dispatch_prepare_mesh_none / dispatch_mesh_gen_none). entities.hpp —
prepare_pyramid_mesh_gen (decl + def).

LEFT (nouns, flagged DEAD → C6 layout-weld basket, joining the TerrainState
husk): WGSL — the PMG_* constants / PyramidMeshParams / bindings 190-192 /
pmg_* writer+geometry helpers (a write-only husk with no entry point), and the
GROUND_ATLAS_PYRAMID=48 slot + its 5-pt placement-kernel sampling (now write-
only; live blind WGSL, don't cut here). renderer.hpp — pyramidMeshGenLayout_.
state.hpp — pyramid_index_buffer/count + set_pyramid_index_count,
upload_pyramid_mesh_params_slot, pyramid_mesh_gen_layout/group + the underlying
buffers/bind group. entities.hpp — pyramid_mesh_gen_pending + its three live
writers, now harmless dead-stores. Every husk carries an inline DEAD (C2) → C6
marker so the storage-weld cut finds them.

DISCIPLINE 2 (collapse mechanism, never a distinction): the cut honored Jean's
verb/noun line exactly — the entry point (the thing that makes the kernel a
kernel) came out; the storage + its C++ bind-group layout stayed together as ONE
future C6 unit rather than half-removed now. The pure pmg_* helpers were LEFT
with the bindings (not split off as "cut the verbs too") — after the kernel they
are unreachable, so leaving them is pixel-safe and keeps the C6 basket cohesive.

GRAPH EDGE REVEALED — the CAST ruling made structural. Mesh-gen families 6→5
(pyramid drops out; arch/column/palm/cactus/blade remain). ArchVertex (stride 40,
C1 assert stands) is now shared by exactly FIVE render families, no longer six.
And the deepest edge: the pyramid is the FIRST entity whose realization IS the
terrain. Its roster row keeps select/place/commit/evict — placement is fully
live, feeding the heightfield — but it has NO mesh realization of its own; the
FAMILY_DISPATCH mesh hook is a none-fork. Every other family realizes as a mesh
it draws; the pyramid realizes as a fold in the ground. Design + placement
WITHOUT self-drawn realization — that is the CAST edge, and it now lives in the
none-fork of the table rather than in a dead pipeline nobody named.

## THE FRAME SPINE — CUT 1: THE EXTRACTION (pixel-identical; glaw1 +
## score-census GREEN; own commit; HOLD for the rig)
Jean's ruling (the conductor becomes the program's temporal dispatch table,
FAMILY_DISPATCH-shaped): AUTHORED order, CHECKED by validation, never COMPUTED
— no topo-solver; deliberate stale-reads and write-order designs are LAWS, not
bugs; pixel-identity culture holds. Anchor: FRAME_CONDUCTOR_RECON (0e89d77)
§1 timeline. Four cuts, one commit + rig each. This is CUT 1.

WHAT: update()'s ten movements (§1a: U1..U10) and render()'s twenty-one
(§1b: R1..R21) are lifted VERBATIM into named phase methods; update()/render()
become pages of calls in the recon's order. PURE LIFT — no reordering, no
logic change (glaw1-strong: this is C++-only, no WGSL blindness; the rig is the
pixel formality). U7 (the transition machine) is ONE phase, internals
untouched. The hidden 2nd submit (R12's flush_zone_derive_requests) is its own
named phase (phase_gol_derive_flush), split from the zone sync/evolve/mesh
dispatch (phase_gol_zone_compute) — the ruling's "hidden submit = its own phase"
made structural. 31 phase methods, 32 counting the R12 split.

DESIGN CHOICES (flagged for the rig, adjustable before CUT 2):
- NATURAL signatures, not a uniform ctx yet — each phase takes what it needs
  (gpuSignal threaded as an update() local through U1/U2→U8; encoder/queue/
  backbuffer/depth passed to the render phases). The uniform-row question — and
  Discipline 2's fork-flagging — belongs to CUT 2 (the table), where a phase
  that resists the uniform ctx is the REAL FORK.
- Whole-movement `if constexpr(ROSTER.x)` gates stay at the CALL SITE (they
  become the CUT-2 spine-row's roster-gate column). Runtime data-guards
  (zone_count>0, dirty flags, rendered_slot) live INSIDE their phase. Intra-
  movement gates (R8's 12 per-family prepares; U7's per-owner teardown verbs)
  stay inside their phase, untouched.

THE CENSUS RODE ALONG (score/run.py). The bijection tool pattern-matches the
LITERAL conductor; the extraction moved the family calls one hop into the phase
methods, so Direction A (gate+call adjacency) and Direction B (free-call
attribution) both went RED — tool-staleness, not a bijection break (glaw1 says
behavior-identical). Fix: the census now SEES THROUGH the extraction — it
inlines each `phase_XXX(...)` conductor call with its method body before
checking, so the flattened conductor reads as the pre-extraction score and the
manifest + attribution sets are UNCHANGED. Two `blk` patterns gained comment-
tolerance + an optional block-brace (a single-phase movement inlines to bare
statements after the gate). Analog to C2's pipelines_skipped() update: the
audit tool must track the code it audits. (CUT 2 migrates the census's source
of truth onto the declarative spine table — its natural machine-readable home.)

GRAPH EDGE REVEALED — the frame's DAG now has NAMES in code, not comments. The
recon's `MOVEMENT:` banners and O-#/RC-# constraints were a phase list written
in the margins (recon §5); CUT 1 lands them as 31 first-class methods. What is
still NOT structural: the phase SEQUENCE (reorder two calls → still compiles),
each phase's read/write FACE, and the frame-truth axes (§3). Those are CUT 2
(the table + the O-1..O-7 validation block) and CUT 4 (C7 drain). The straining
toward a declared phase sequence is now half-resolved: the phases exist; the
ORDER-AND-FACES-AS-LAW is the next cut. GATES: glaw1 + score-census GREEN;
pixel-identical rig pending (Jean's machine).

## THE FRAME SPINE — CUT 2: THE SPINE (the temporal dispatch table;
## glaw1 + boot-validated + census-on-spine GREEN; HOLD for the rig)
Jean's ruling (three bindings): (1) the census migrates WITH the table — its
source of truth becomes the spine rows, the see-through shim retires; (2)
uniform-row design from CUT 1's natural signatures, a resister is a REAL FORK to
flag; (3) the validation block is the deliverable — every O-#/RC becomes a
static/boot assert over row indices, C8 dissolves here, by-design lags declared
as law lines. Behavior-identical by construction (same calls, same order, gates
as row columns). glaw1 + boot validation + pixel rig.

WHAT LANDED. update()/render() are now LOOPS over two constexpr spine tables
(UPDATE_SPINE: 10 rows / RENDER_SPINE: 22 rows). Each row = {phase id, name,
member fn (Cartridge::*), driver(§9), roster gate (constexpr-folded bool), face
tags}. The conductor is `for (row : SPINE) if (row.enabled) (this->*row.fn)(ctx)`.
The 32 phase methods were re-signatured to ONE uniform shape — `(UpdateCtx&)` /
`(RenderCtx&)` — via a ctx struct bundling the frame-transient inputs (signal/
aspect/queue/gpuSignal ; encoder/queue/backbuffer/depth); bodies unpack only what
they read. Phase ids are enums whose DECLARATION ORDER == authored order == row
index.

DISCIPLINE 2 — NO REAL FORKS FOUND. Every phase fit the uniform ctx row. The
one candidate resister, the GoL block (R12), fit after a verification: its
compound call-site guard `if(zone_count>0){ counter++; flush; ...5 }` split into
two rows (GolDeriveFlush = the hidden submit + the residue counter;
GolZoneCompute = config/sync/evolve/mesh), each self-guarding on zone_count.
BEHAVIOR-IDENTICAL because flush_zone_derive_requests provably does NOT touch
zone_count (gol_zones.hpp:572-590; the only mutators are select/evict/teardown,
none in the R12 block), so the two independent guards equal the original single
one. The counter moved into GolDeriveFlush; the residue proof (G3) holds (gol
off -> rows disabled -> never called -> pristine).

THE VALIDATION BLOCK (binding 3, the deliverable). Every CROSS-PHASE ordering
law is now a static_assert over row indices — a reorder fails the BUILD, not the
pixel rig:
  O-5a  FillSignal < AdvanceClock        (dt_beats read before clock advance)
  E-3   SkyNeutral < StageFadeUpload      (neutral sky before the signal upload)
  O-5e  ClearInputDeltas is dead-last
  O-1   RibbonTick < DispatchCompute      (sky resync before compute reads it)
  O-2   WitnessHarvest < DispatchCompute < WitnessCapture  (the witness ring)
  RC-1/2 StreamPatches < RespawnAgents, < MotionCorral
  O-4   GroundEntries < PlacementCorrection
  O-7   FrustumCull < ShadowPass, < MainPass
  draw  ShadowPass < MainPass < SnapshotPass ; GolDeriveFlush < GolZoneCompute
BOOT asserts (validate_spine, wired into init_renderer — a constexpr member fn
can't static_assert inside its own incomplete class): row-order integrity
(row[i].id == i, both tables) + the O-5b/c FACE law (no F_SIGNAL/F_CONFIG
staging phase may follow the StageFadeUpload drain — face-based, so a FUTURE
staging phase placed after the drain fails to boot). INTRA-phase laws, not
row-index laws: O-3 (teardown fixed sequence, inside phase_transition_machine)
and O-6a (zone sync->evolve->mesh barrier = the three separate compute passes
inside phase_gol_zone_compute). The by-design lags (E-3 sky write-order, E-4
witness 1-frame lag, E-9 portal-spans-a-frame) are declared as LAW LINES in the
spine header. **C8 is dissolved — nothing of it survives as a separate cut.**

THE CENSUS MIGRATED WITH THE TABLE (binding 1). The CUT-1 see-through shim is
GONE. score/run.py now PARSES the spine rows and audits them: Direction A =
per-family frame work is a spine ROW gated by its bit (FRAME_ROWS/SHARED_ROWS) +
the non-frame obligations (teardown/boot/mesh-prep/delegated doors, which live
intra-phase or at boot and grep unchanged); Direction B = every ROW is
attributed (gate is a ROSTER bit -> that family, or `true` -> a FOUNDATIONAL
phase with a one-line justification) AND the phases<->rows bijection holds (32
phases == 32 rows). An ungated spine phase is now UNWRITABLE. The manifest IS
the table; attribution IS row membership.

GRAPH EDGE REVEALED — the frame's DAG is now DECLARED, and its order is LAW.
CUT 1 gave the movements names; CUT 2 gives them a table, a uniform face
(the ctx), a driver tag, a gate column, and — the payload — an order that the
compiler enforces. The recon §5 "straining toward a declared phase sequence" is
resolved: the phases exist (CUT 1), the order-and-faces-are-law (CUT 2). What
remains of the recon's model question is the C7 drain (E-1's silent staging
drop — the ONE ordering hazard not yet structural, now the last cut) and the
draw-list half of C5 (§1d's triplication, still three hand-synced lists — the
spine governs the CONDUCTOR order, not yet the draw enumeration). ONE tradeoff
flagged for the rig: the conductor's ROSTER gate is now a RUNTIME row column
(row.enabled folds from the constexpr bit) rather than a compile-time
`if constexpr` — Jean granted this ("gates preserved as row columns"); behavior
is identical (same skip), the pipeline-creation constexpr gates in the renderer
(the FXC-skip DEMO-1c cares about) are untouched. GATES: glaw1 + boot-validation
+ score-census GREEN; pixel-identical rig pending (Jean's machine).

## THE FRAME SPINE — CUT 3: C5-DRAW, THE DRAWABLE TABLE (glaw1 GREEN;
## HOLD for the rig — shadows/main/snapshot pixel-identical)
Jean's ruling: one row per drawable {name, shadow/main/snapshot membership,
order, bind convention}; the three pass functions iterate it filtered. Kills the
triplication + the ribbon's ordinal drift; a new drawable is ONE row. Discipline
2 armed for any drawable that resists the row (the specials — gallery/orb/fade —
may be the genuine forks; flag, don't force).

WHAT LANDED. A new header realization/drawable_table.hpp carries the DRAWABLE
TABLE: 11 rows (zone, pawn, sphere, monolith, ribbon, arch, column, palm, cactus,
blade, shell), each {name, pass membership bits, a ctx-agnostic draw thunk}. The
thunks take (Renderer&, GPUState&, pass, DrawBind) — DrawBind carries the pass's
entity+texture bind groups + the two precomputed runtime guards (ribbon rendered,
zone active), so the SAME thunks serve all three passes even though shadow/main
run on MachineCtx and snapshot on GalleryDeps (both expose Renderer&/GPUState&).
The three pass functions now: draw terrain (the fork), then draw_table(filter),
then (main only) the trailing forks. draw_shadow_all -218-ish; render_main_pass's
scattered entity draws collapse to one call; render_snapshot_pass's six draws to
one. The header is included after renderer.hpp, BEFORE gallery.hpp + render_passes
.hpp, so both the snapshot pass and shadow/main see it (no function relocation).

THE PIXEL ARGUMENT (why the ribbon drift dies for FREE). Every drawable in the
table is OPAQUE — depth-tested, depth-write, no blend, or an alpha=1.0 output
making SrcAlpha a no-op. VERIFIED each: ribbon uses ENTITY_FS + the shared
depthStencil/colorTarget (bespoke VS only, no blend); gallery_frame_fs and
wall_painting_{canvas,frame}_fs all return alpha=1.0 with depth-write (discard
for the transparent texels). Draw order among opaque geometry is IMMATERIAL (the
depth test resolves visibility identically), so ONE canonical order (the shadow
order) reproduces all three passes pixel-for-pixel. The lists' only real
divergence was ribbon's position (main drew it late, at ~13; the table draws it
with the entities, at ~5) — provably pixel-identical because ribbon and every
draw between are opaque. Shadow is depth-only (order doubly immaterial); snapshot
is opaque-only. Per-pass table order confirmed byte-identical to the originals
except that one safe ribbon move.

DISCIPLINE 2 — THE FORKS, FLAGGED NOT FORCED. Kept explicit in their pass
functions, NOT crammed into a uniform row:
  - terrain: THREE different per-pass codes (shadow LOD0 + manual LOD1; main LOD0
    indirect/direct + LOD1 direct; snapshot one direct draw) — a genuine fork.
  - wall_paintings / gallery_frames: their OWN gallery bind groups (galleryEntity
    + galleryTexture), main-only.
  - orbs (additive) / fade (alpha, no depth write): the ORDER-SENSITIVE pair —
    they MUST stay last and in order; the table's opacity invariant does not hold
    for them, so they are forks by construction. This is exactly the resister
    Jean anticipated.

GRAPH EDGE REVEALED — the draw enumeration is now single-sourced, and the frame's
opacity structure is explicit. §1d's three hand-synced lists (recon) collapse to
one table + three filtered iterations; the ribbon ordinal drift (recon §1d, the
proof the lists had drifted) is gone. What the table makes legible: the frame's
drawables partition into an OPAQUE core (order-free, tabled) and an ORDER-
SENSITIVE tail (orbs/fade, forked) — the same opaque/blended boundary that
governs any depth-buffered renderer, now named in the code. The C5 draw-list half
is done; the family/mesh-gen half of C5 (a new family = one row feeding mesh-gen
+ draws + ROSTER) remains open but was never in this cut's scope. GATES: glaw1 +
score-census GREEN; pixel-identical rig pending (Jean's machine — shadows, main,
and snapshot).

## THE FRAME SPINE — CUT 4: C7, THE DRAIN AS SOLE UPLOADER + THE SKY
## RELAY TO ONE AUTHOR (E-3 mechanized, E-1 dies on the signal side;
## glaw1 GREEN; HOLD for the rig — pixel-identical)
Jean's ruling: staged signal/config writes with ONE drain phase as the sole
uploader (E-1 dies by construction); the sky relay (E-3) collapses from three
writers to ONE author; the by-design witness lags (E-4/E-9) STAY declared law,
not "fixed"; Discipline 2 armed — a setter that genuinely needs a post-drain
write is a fork to FLAG, not force. This cut closes the recon's edge list.

WHERE E-1 ALREADY STOOD (disclosed, not re-done). The CONFIG half was already
mechanized (Cable Management C4): config_ is the CPU staging struct, every
setter writes it + sets configDirty_, and upload_config is the sole whole-struct
config drain (dirty-gated). The SIGNAL half's post-drain hazard was already
structurally blocked at CUT 2: the O-5b/c face law (no_staging_after_drain)
fails the BUILD if any F_SIGNAL/F_CONFIG phase is authored after the drain
(StageFadeUpload). What REMAINED — the one legitimate post-drain write of the
signal buffer — was the sky relay. This cut removes exactly that.

WHAT LANDED (the sky relay, mechanized). The eight sky_* words are the TRAILING
32 bytes of GPUFrameSignal (offsetof(sky_mode)==304, sizeof==336 — static-
asserted at the split site). The relay was three writers: U2 phase_sky_neutral
wrote neutral zeros into the per-frame signal, U8's upload_signal drained the
WHOLE struct (sky words included), then R7's tail (resync_sky_head) OVERWROTE
the sky words in render — correctness riding queue submission order across
update()->render(). Now:
  - upload_signal writes only the first 304 bytes (queue.WriteBuffer(.., 0, ..,
    offsetof(sky_mode))) — the drain no longer carries the sky block.
  - phase_sky_neutral is DELETED; UPhase::SkyNeutral is removed from the enum;
    the SkyNeutral row is removed from UPDATE_SPINE (9 update rows now, was 10);
    the E-3 ordering static_assert (SkyNeutral < StageFadeUpload) is replaced by
    a "MECHANIZED, not an ordering assert" note.
  - resync_sky_head is the SOLE author of the sky block: per-frame at R7 (ribbon
    on), and a boot-neutral in initialize() (resync_sky_head(q, 0u, 0..0)) covers
    the ribbon-OFF case. One writer, one disjoint region.
  - update()'s gpuSignal is value-initialized (GPUFrameSignal{}) so the excluded
    sky words are provably zero even though the drain never reads them.
The spine-header LAW E-3 paragraph is rewritten from a preserved write-order law
to "MECHANIZED — no longer a law" (disjoint regions have no order to preserve).

THE PIXEL ARGUMENT (byte-identical at DispatchCompute). The sky block's contents
at R10 (the kernel read) are provably unchanged. RIBBON-OFF: old wrote neutral 0
every frame (U2) and R7 never ran (gated) → block is 0; new writes neutral 0
ONCE at boot and R7 never runs → block is 0. Identical. RIBBON-ON: old wrote 0
(U2), drained it, then R7 overwrote with this frame's real pose (submission
order) → block is the real pose; new leaves boot-0 there, the drain writes the
DISJOINT first 304 bytes (never touching the block), then R7 writes the same
real pose → block is the real pose. Identical. The drain and the sky author now
write NON-OVERLAPPING byte ranges, so their relative order is immaterial — the
submission-order paragraph that made the old code correct is deleted because the
structure can no longer be wrong. (WGSL is untouched: the GPU-side struct layout
is unchanged; only the CPU write is split into two disjoint WriteBuffers.)

E-1 DIES ON THE SIGNAL SIDE. The only post-drain toucher of the signal buffer
was the sky resync; with the sky block disjoint from the drain, R7 is a sole
author of its own region, not an overwrite of already-uploaded bytes. A staging
setter can no longer land in the sky window and be silently clobbered by a
neutral-then-overwrite relay — there is no relay. Combined with CUT 2's face law
(no staging phase may follow the drain, enforced at BUILD), the signal-side E-1
is now closed by construction.

DISCIPLINE 2 — THE FORKS, FLAGGED NOT FORCED. Two config offset-writers refuse
the staged-drain shape and are kept as genuine forks: upload_pier_count (offset
124, from write_pier/clear_pier) and upload_placement_patch_count (offset 144,
from stream_patches). Each is a targeted 4-byte direct write to configBuffer_ at
its OWN moment — a count that must be visible to a same-frame dispatch AFTER the
whole-struct drain has run (the "fastest clock"). These are real same-frame
dependencies, exactly the resister Jean anticipated; they stay direct, offset-
asserted, and are NOT folded into the dirty-gated drain. The sky resync itself
(R7) is the third such fork on the signal side — a render-time write that MUST
follow update's drain — now made safe by disjointness rather than ordering.

GRAPH EDGE REVEALED — the recon's edge list CLOSES. E-3 (the sky write-order
relay) is dissolved: three writers → one author, an ordering law → a structural
guarantee. E-1 (setter-after-drain drop) is closed on both halves: config by the
dirty-gated drain + the two flagged offset-forks, signal by the face law + the
now-disjoint sky author. The by-design lags E-4 (witness 1-frame stale) and E-9
(portal spans a frame) STAY declared as spine-header law lines — they are
correct designs, not defects, and were never in scope to "fix." What the frame's
write graph now says plainly: the signal buffer has TWO disjoint authoring
regions (the per-frame drain over the first 304 bytes; the sky block owned solely
by the ribbon tick, boot-neutral otherwise), and the config buffer has ONE
dirty-gated drain plus two named same-frame offset-forks. No neutral placeholders,
no cross-function submission-order dependencies. GATES: glaw1 GREEN (the split's
offset static_asserts + the queue-lvalue idiom compile on the real TU); score
census GREEN (phase_sky_neutral retired from FOUNDATIONAL_PHASES; bijection now
31↔31, 20 foundational — 9 update + 22 render rows). Pixel-identical rig pending
(Jean's machine — ribbon-on: the possessed pawn snaps onto the flown head exactly
as before; ribbon-off: nothing observable changed).

## C6 — THE BINDING REGISTRY (single-source the binding NUMBERS; glaw1 +
## byte-identity GREEN; HOLD for the launch gate — NOT pixel-only)
Jean stamped all four rulings (recon BINDING_REGISTRY_RECON): (1) constexpr,
GROUP-SCOPED; (2) authored literals, +200 band a static_assert witness only;
(3) Option A — C++ registry + WGSL mirror on the launch gate — IS C6, the third
(WGSL) copy stays a mirror the gate catches; (4) ONE constant PER SITE, named for
the site, never one-per-buffer. Registry-first: single-source the NUMBERS only,
behavior-identical, no husk removed.

WHAT LANDED. A new header realization/binding_registry.hpp carries the single
source of truth: `namespace bind { namespace g0 {…} namespace g1 {…} }`, 89 g0 +
14 g1 = 103 named `inline constexpr uint32_t`, each the binding number authored
ONCE, named for the WGSL variable it mirrors (so the const name is greppable in
both state.hpp and world.wgsl). GROUP-SCOPED is FORCED, not stylistic: binding 22
= terrain_mesh_indices (g0) AND bilinear_sampler (g1); 25 = tile_grid vs
shadow_map; 26/30 likewise — a flat list would fuse distinct slots. state.hpp's
createBindGroups: every `entries[k].binding = <literal>` (all 323, across 24
layout + 24 group blocks + the offscreen gallery group) → `= bind::gX::<name>`.

THE BEHAVIOR-IDENTITY PROOF (§3 contract, mechanically enforced). The rewrite was
a scripted transform that touches ONLY the RHS integer of each `.binding =` line —
block classified g0/g1 by its label ("Texture" ⟺ g1), number → site-name by the
recon's extraction. TWO gates prove identity: (a) the DIFF — exactly 323 lines
changed, every old side `entries[k].binding = <int>;` and every new side
`entries[k].binding = bind::gX::<name>;`, same index k, same trailing comment; NO
array size N, index, order, or resource line touched (grep-verified empty
otherwise). (b) BYTE-IDENTITY — a checker resolved every new `bind::gX::name`
against the registry and confirmed all 323 equal the original integer git HEAD
had. Same integers emitted → the built bind-groups are byte-identical; glaw1
proves the 103 names resolve on the real TU + the +200 witnesses hold.

DISCIPLINE 2 — NO FORKS. Every binding took a single site-named constant cleanly.
The same-buffer-different-number cases (patch_instances 340 / photo_patch_instances
144 / zone_patch_instances 165; render_camera 280 / camera_state 80; the orb
trio) are THREE site names for one buffer — the const names the (group, SLOT), the
shared handle lives in the group entry's `.buffer` (ruling 4), so no collapse and
no computed arithmetic. The 5 WGSL-aliased pairs (config/fc_config, vp_data/fc_vp,
agent_state/fc_agents, camera_state/fc_camera, patch_instances/fc_patches) are ONE
binding number each → ONE constant covers both aliases (the alias is a second WGSL
var name for the same slot). Binding 201/280 binding DIFFERENT handles across
Render-Entity vs Photographer-RE is the same story — one slot const, per-group
handle. No binding needed a computed-at-runtime slot; nothing was forced.

GRAPH EDGE REVEALED — the C++ layout↔group pair is now COMPILER-LINKED. L2-a (the
"binding integer typed twice") collapses 2 of its 3 copies: a layout/group typo is
now an undefined-symbol compile error, not a runtime crash. The frame's GPU
address space, 323 scattered literals, is now a named (group, slot) map of 103
symbols — and it makes two structures legible: the +200 render-mirror band (now a
static_assert-witnessed law, authored not computed) and the same-buffer-different-
slot aliasing (now explicit per-site names). It also readies the §4 husk re-index:
`bind::g0::terrain_state` (20/220) and `bind::g0::pmg_*` (190-192) are now named
symbols the compiler will flag when the follow-on deletes them (vs the blind
literal hunt today). THE CEILING (accepted): the 108 WGSL @binding literals stay a
mirror — the shader can't read a C++ const — kept in lockstep by the launch gate;
closing the third copy (a generated/substituted shader, feasible because
world.wgsl is runtime text) is the named "true pin" follow-on.

GATES: glaw1 GREEN (103 names resolve + witnesses); byte-identity GREEN (323/323
resolve to the original integer); score census GREEN (spine untouched — 9 update
+ 22 render). THE GATE THAT REPLACES PIXEL-ONLY (§5, Jean's ruling): a
mis-single-sourced binding can render identical on the common path and
crash/validate-fail off it, so the rig run MUST exercise every group under Dawn
validation — a SNAPSHOT (photographer + exhibition groups), INDOOR/GALLERY entry
(indoor_shell + gallery entity/texture), and a spawned GoL ZONE (zone + aura +
derive) — with zero validation errors; pixel-identity is the secondary check.
HELD for Jean's gate pass.

## HUSK SWEEP 1/3 — TerrainState (the §4 re-index, compiler-driven; glaw1
## GREEN; HOLD for the crash-aware launch pass)
C6 stamped launch-validated; now the §4 re-index the registry was built to
enable — DELETION on a verified base, and the FIRST cut to change array SIZES
(the churn registry-first deferred, landing here under the crash gate, not a
pixel-only proof). Husk 1: the dead GPUTerrainState buffer — bound at 20/220,
read by no shader since 851ce68.

STILL-DEAD RECONFIRMED AT HEAD (grep, not the old markers): terrainBuffer_/
GPUTerrainState referenced only by decl + alloc + isReady + one boot dead-store
+ the 3 bind entries; world.wgsl terrain_state/render_terrain read by no shader
body (only the decls + residue comments). No live consumer appeared since §6.

COMPILER-DRIVEN, exactly as designed. Deleted bind::g0::terrain_state(20) +
bind::g0::render_terrain(220) + their +200 witness from binding_registry.hpp
FIRST; glaw1 then flagged EXACTLY five dangling refs — state.hpp:3566 (Compute
Entity Layout), :3668 (Render Entity Layout), :4551 (Compute Entity group),
:4649 (Render Entity group), :4971 (Photographer RE group) — no more, no less.
That is the proof the registry converted a blind literal-hunt into a
compiler-checked symbol removal: the compiler enumerated every site.

THE RE-INDEX (array SIZES change here). All five were entries[3]; each block:
removed the entries[3] block, renumbered entries[4..18] -> [3..17], shrank the
std::array 19 -> 18. A scripted transform verified DENSE (every touched block
entries[0..N-1], no gap/dup) — order-independence means the renumber is
behavior-identical (the binding NUMBER is the key; only the array slot moved).
Then the orphaned C++ residue: struct GPUTerrainState + its size static_assert,
the terrainBuffer_ member + its makeBuffer alloc + the isReady term + the boot
dead-store (10 lines writing a buffer no one read). WGSL (glaw1-blind, hand-
verified reader-free): removed struct TerrainState + the two @binding vars +
the stale module-list mention; residue "why-removed" comments kept as history.

GRAPH EDGE REVEALED — the first dead binding-SLOT freed. Group 0 loses numbers
20 and 220; the two 19-entry groups (Compute + Render Entity) and the
Photographer render-entity group are now 18; two 19-entry layouts are 18. The
registry made this a clean symbol deletion end to end — delete the name, follow
the compiler to every site — the exact re-index §4 promised. terrain_state was
also the last wire of the RAYMARCH storage-weld (the DEAD marker's "awaiting
storage-weld removal" is now discharged).

GATES: glaw1 GREEN (compiler drove the C++ deletion — zero dangling refs);
score census GREEN (spine untouched); encodings clean UTF-8/LF, no CR. THE
CRASH-AWARE LAUNCH PASS (same as C6, NOT pixel-only): the shrunk groups (18
entries) must still validate against their layouts + every pipeline that binds
them — re-run the cold paths (GoL, gallery, snapshot) under Dawn validation,
zero errors. The WGSL @binding removal is blind — the launch gate is where the
C++ layout (now 18) and the shader (now missing terrain_state/render_terrain)
are proven consistent. Pixel-identity secondary. HELD for the gate.

## HUSK SWEEP 2/3 — the PMG basket (pyramid mesh-gen; a whole layout+group
## dropped; glaw1 GREEN; HOLD for the crash-aware launch pass)
Husk 2: the pyramid mesh-gen basket — bindings 190/191/192, a dedicated
layout + group + three buffers + a WGSL island — whose entry point
(pyramid_mesh_gen) was cut in the C2 orphan sweep. Unlike husk 1 (one entry
inside live groups), this is a WHOLE dead bind group: nothing dispatched it.

STILL-DEAD RECONFIRMED AT HEAD (grep): no pyramidMeshGenPipeline_, no
dispatch, no draw; pyramidMeshGenBindGroup_ built but never bound; the WGSL
pmg_vertices/pmg_indices write-only, pmg_emit_tri uncalled (a closed island);
the surviving upload_pyramid_mesh_params_slot / set_pyramid_index_count /
pyramid_mesh_gen_pending calls all dead-stores (their reader was cut).

THE CUT (spans 6 files; the LIVE pyramid-as-terrain path stays). Deleted
bind::g0::pmg_params/pmg_vertices/pmg_indices from the registry; the compiler
then flagged the layout + group bind entries. Because ALL three bindings were
dead, the ENTIRE Pyramid Mesh Gen layout block + group block were removed (not
shrunk) — a whole bind group gone. Then the residue:
  - state.hpp: the pyramidVertexBuffer_/IndexBuffer_/MeshParamsBuffer_ members
    + their alloc + zero-init + logging; pyramidIndexCount_; the GPUPyramidMesh-
    Params struct + its sizeof assert; the PMG_* count consts; the vertex/index/
    count accessors + upload_pyramid_mesh_params_slot + the mesh-gen layout/group
    getters.
  - renderer.hpp: the pyramidMeshGenLayout_ member + its assignment.
  - entities.hpp: the pyramid_mesh_gen_pending flag + two dead-store sites
    (evict + clear-all).
  - entity_pipeline.hpp: the "write mesh gen params" tail of the commit path.
  - world.wgsl: the whole §9.0 island (PMG_* consts, PyramidMeshParams, bindings
    190-192, the pmg_* writer/geometry helpers) — 106 lines, spliced out.
KEPT LIVE (the discipline's edge): MAX_PYRAMID_INSTANCES, pyramidInstancesBuffer_
(baked via contrib_pyramids_at — pyramids ARE terrain), pyramidGroundBuffer_
(placement ground-Y), upload_pyramids, upload_pyramid_origins, and every
pyramid INSTANCE field on the live commit/evict paths. createPyramidMesh() now
allocates only the two live buffers.

GRAPH EDGE REVEALED — a whole dead bind group excised, group 0 loses 190/191/192.
This is the §4 re-index at its cleanest: the registry symbol deletion led the
compiler straight to the layout+group; because the group was wholly dead, it
came out entire. The "pyramids are drawn geometry" reading (the mesh-gen basket)
is now fully gone; the "pyramids are terrain" reading (instances + ground) is all
that remains — the recon's L1-a pyramid-CAST ruling, realized. The RENDER_UPDATE
recon's PMG husk (§6) is discharged.

GATES: glaw1 GREEN (compiler drove the C++ deletion across 5 hpp/inl files —
zero dangling); score census GREEN; encodings clean LF, no CR; net −236 lines.
THE CRASH-AWARE LAUNCH PASS (same as husk 1): the surviving groups must still
create + validate — but the specific check here is that REMOVING a whole bind
group left createBindGroups() correct (one fewer group built) and no pipeline
referenced pyramidMeshGenLayout_. Re-run full ROSTER + GoL + gallery + snapshot
under Dawn validation, zero errors; spawn/evict a pyramid (the live commit path
lost its mesh-gen tail). Pixel-identity secondary. HELD for the gate.

## HUSK SWEEP 3/3 — the complexity texel (WGSL-only channel; the ODD ONE
## OUT; glaw1 GREEN + pixel-identity PRIMARY here; HOLD for the rig)
Husk 3, the one that corrects the anchor: complexity is a DATA-CHANNEL husk
riding LIVE buffers (the patch heightfield texture's .w + the height-scratch
+1 slot), NOT a binding-slot one. It frees NO binding, touches NO registry, no
array size — a pure WGSL removal. Marker was LATENT[complexity] (baked +
shipped per-vertex, read by no fragment shader; every palette call hardcodes
0.5). This is why the gate INVERTS: for the binding husks the crash-aware
launch was primary; here PIXEL-IDENTITY is primary (a WGSL varying/heightfield
change), with launch-validation catching the VS→FS interface.

STILL-DEAD RECONFIRMED AT HEAD (grep): no in.complexity read anywhere; the .w
channel sampled only into the (removed) varying; all five palette call sites
still pass 0.5. The dead chain: pass-1 stores complexity to scratch +1 →
pass-2 reads it back → bakes it into the heightfield .w → the patch-terrain VS
samples .w into out.complexity → no FS reads it.

THE CUT (WGSL-only, C++ untouched, behavior-identical). Removed the whole dead
data flow: the PatchTerrainVarying.complexity varying (@location(2), renumbered
patch_uv/layer to 2/3 to keep locations contiguous — the VS out + FS in share
one struct, so they stay matched); out.complexity = height_data.w in the VS;
the pass-2 complexity readback; the pass-1 scratch +1 store; and the bake
becomes vec4(height, grad_x, grad_z, 0.0). KEPT LIVE (the discipline's edge):
terrain_height_and_complexity + ground_formed_with_complexity (they compute
HEIGHT — the complexity is a free byproduct the compiler now DCEs), and the
palette_color_smooth / palette_color / palette_target_color complexity PARAMETER
(a live knob, hardcoded 0.5 today, a future coupling point). The height scratch
stays STRIDE-2 (byte-identical height layout → the +1 slot simply goes
unwritten): no C++ buffer resize, so genuinely WGSL-only.

WHY BEHAVIOR-IDENTICAL: the .w channel and the complexity varying were read by
nothing live (palette hardcodes 0.5), and the height plumbing (scratch base
slots, pass-2 gradients, the .x/.yz heightfield channels) is byte-for-byte
unchanged. The terrain renders the same pixels; only a dead interpolant + a
dead texture channel are gone.

GRAPH EDGE REVEALED — a data channel, not a binding, retired. This husk did NOT
ride the registry (frees no binding) — it corrects RENDER_UPDATE_API §6's
"TerrainState + complexity could ride the registry": TerrainState (husk 1) was
a true binding re-index the registry de-risked; complexity is a parallel WGSL
channel cleanup that needed no registry at all. The heightfield .w is now free
for a future coupling (the LATENT note's "interference density → material") —
but as a clean unused channel, not a shipped-and-ignored one.

GATES: glaw1 GREEN (C++ untouched — WGSL-only); score census GREEN; encodings
clean LF, no CR. THE RIG GATE (inverted from the binding husks): PIXEL-IDENTITY
is primary — the terrain must render byte-identical (the removed varying + .w
were dead) — with the launch validating the changed patch-terrain VS→FS varying
interface (locations 0,1,2,3 matched on both sides) at pipeline creation. The
WGSL is BLIND (glaw1 does not compile it); the rig is the proof. HELD for the
gate. THE SWEEP CLOSES: three husks (TerrainState, PMG, complexity), the recon's
§6 parked baskets, all discharged — two binding re-indexes the registry enabled,
one WGSL channel cleanup beside it.

## THE VEIL — THE CUT (ruled, all six; glaw1 + census GREEN; HOLD for the
## OBSERVABLE-BY-DESIGN gate — the first since b2b)
Jean ruled all six on the recon (VEIL_VISIBILITY_RECON): the point-anchored
fog-wall as the ONE visual visibility authority. What landed, per ruling:

1. PREREQUISITE — render_point_pos() (world.wgsl, beside render_pawn_pos):
   point_camera_hosted() ? render_camera.pos : render_pawn_pos(). The veil
   anchors on THE POINT, never the eye; the eye-fog stays eye-anchored (a
   legitimate view-effect fork, untouched).
2. METRIC — per-fragment distance(world_pos.xz, point.xz): the fragment IS
   the metric. update_entity_draw_visibility DEMOTED to overdraw optimization
   at the EXIST ring (350): the edge margin (25) + per-size inset (0.5/cap 60)
   RETIRED (their visual role is dead); hysteresis (40) stays as a mesh-upload
   toggle damper. Sole law, static_asserted: EXIST ≥ VEIL_FAR + ENTITY_MAX_
   EXTENT (75) — no entity with in-veil fragments is ever CPU-culled.
3. CHAIN — declared ONCE in Dim (state.hpp, the registry pattern):
   VEIL_NEAR_DEFAULT 175 (the old LOD0 ring) / VEIL_FAR_DEFAULT 275 (the old
   draw cylinder) / EXIST_RADIUS 350 (the pregen edge) — reused, not invented —
   with static_asserts PREGEN ≥ EXIST > FAR > NEAR + the cull law. LIVE values
   ride config (veil_near/veil_far, tunable): the CPU band reads the getters,
   the GPU LOD0 gate reads fc_config.veil_near, the fragment veil reads both —
   ONE yardstick on all three sides by construction. V1 FIXED: agents 360→350
   (=EXIST, static_asserted; CPU mirror agents.hpp updated). FLAGGED FORK
   (the ruling's "unless a reason surfaces"): FLOATERS STAY 400 — the in-code
   contract documents why (floaters SPAWN to the 350 pregen edge; sphere pos
   lands ~12u past its anchor + commit lag → a 350/360 line caused near-100%
   eviction-at-spawn; 400 = 350 + 50 headroom). Floaters are SKY objects
   (never stand on unresident ground) and 400 > FAR keeps every eviction
   behind the wall — invisible by construction. Flagged, not forced.
4. MECHANISM — the fog-wall as a NEW term in shade_lit, composed AFTER the
   existing eye-fog: veil = smoothstep(veil_near, veil_far, point_d) *
   veil_strength * veil_scale; return mix(fogged, fog_color, veil). Wall
   color = the existing fog/horizon color (reads as sky, not a cylinder).
   VERIFICATION LINE: inside NEAR the term is EXACTLY zero (smoothstep ≤ edge
   → 0; mix(x, y, 0) = x) — pixel-identical to today by arithmetic.
5. FORKS — orbs/fade overlay: untouched (no veil term reaches them). RIBBON:
   exempt via the ruled veil-scale param — shade_lit gained veil_scale; a new
   ribbon_fs (Entry::RIBBON_FS; the ribbon pipeline's one-line entryPoint
   change) passes 0.0 while entity/terrain/zone pass 1.0. GALLERY/INDOOR: the
   exemption IS the mode — U5 (StageWorld, F_CONFIG, pre-drain) stages
   veil_strength = finite_mode ? 0 : 1 (the same law that makes all patches
   visible in finite mode: walls define the boundary, not fog). The PAWN is
   NOT exempt (ruled): in camera-host the abandoned body fades into the
   horizon like anything else. ZONES join (their FS was found to route
   through shade_lit after all — the recon's "own-FS" note corrected);
   OUTDOOR GALLERY FRAMES join via their own FS (they spawn to the 350
   residency ring — the mode-exemption covers indoor, not them).
6. FOSSILS — retired: PATCH_RENDER_RADIUS/SIDE + RENDER_RADIUS/SIDE +
   VISIBLE_RADIUS(_SQ) + LOD_FULL_RADIUS(_SQ) + VISIBILITY_CYLINDER_*(2) +
   LOD0_CYLINDER_*(2) + GPU FRUSTUM_LOD0_RADIUS_SQ — the 4-spelling LOD0
   group and 3-spelling draw group COLLAPSE into the declared chain (Dim +
   config; the GPU gate now reads the same live value the CPU band uses).
   RENAMED: lod_pawn → lod_point (config field, offset-384 assert intact;
   stage_/upload_ setters; fc_config reads; pawn_wx/pawn_wz locals →
   point_wx/point_wz; all "pawn" comments corrected — the name fossil dies).
   Config grew 400 → 416 bytes (veil_near/far/strength/_veil_pad appended;
   sizeof assert updated; WGSL DesignConfig mirrored field-for-field).

HAND-VERIFICATION (WGSL blind — the law) that CAUGHT A CRASH: the first
gallery veil draft called render_point_pos(), whose call graph statically
references render_agents (binding 260) — but the Gallery Entity layout binds
only {1,201,280,320}: pipeline creation would have FAILED at boot. Fixed:
gallery_frame_fs anchors on config.lod_point_* — the SAME point (the staged
anti-flicker copy the CPU band + GPU gate already share; 1 frame stale, law
E-4, imperceptible across a 100 wu band) — no layout change. All shade_lit
users verified layout-safe (entity/ribbon/shell/terrain/zone/snapshot bind
260+280 already). Mirror verified field-for-field; all 4 shade_lit callers
pass the new arity; render_point_pos defined once (out-of-order module scope
is the shader's existing pattern); the frustum gate reads lod_point +
veil_near².

GRAPH EDGE REVEALED — visibility has ONE author. The 9-authority disagreement
(recon §1) collapses: the veil (fragment fog-wall) is the visual truth for
terrain + every entity_fs family + zones + outdoor gallery; the CPU band and
GPU LOD gate are LOD mechanics on the same chain values; the CPU entity cull
is overdraw hygiene at EXIST; existence eviction sits on (agents) or flagged
above (floaters) the EXIST ring — every number either IS the chain or is
static_asserted against it. The stage now ends where awareness ends, around
the same anchor (§11: the veil is the bubble's visual face).

GATES: glaw1 GREEN; score census GREEN (spine untouched — the strength
staging rides U5's existing F_CONFIG face); encodings clean LF, no CR.
THE OBSERVABLE-BY-DESIGN GATE (Jean's rig, the first since b2b): inside NEAR
pixel-identical (the term is arithmetically zero); walking outward THE POPS
ARE GONE — flora no longer stands on void, arches no longer materialize
inside visible ground, the world condenses continuously with each step;
ribbon still visible far; orbs intact; indoor unchanged (strength 0); and
the camera-host check — fly away from your pawn and the world unveils around
YOU while the body recedes into the wall. HELD for the gate.

## THE VEIL — REEVALUATION FLIP (Jean's re-ruling: THE RING is the DRAW
## authority, fog is ICING; glaw1 + census GREEN; HOLD for the rig)
The gate showed the inversion: the fog-wall concealed only what had terrain
behind it — beyond-FAR geometry painted wall-colored SILHOUETTES against the
orb sky. Re-ruled: the DRAW SET is the authority; fog is cosmetic. KEPT from
376115f: render_point_pos, the chain header/asserts/config plumbing, the
lod_point rename, the fossil retirement, V1 (agents ≤350), the ribbon
veil-scale machinery. FLIPPED:

1. THE RING — one chain constant (Dim::VEIL_RING_DEFAULT = 6.5 patches = 325,
   Jean's enlargement from 5.5/275), the SOLE draw authority, live via
   config.veil_ring. Config quad re-authored: {veil_ring, veil_icing,
   veil_strength, lod0_radius} (the old near/far/pad semantics retired; the
   pad slot now carries lod0 — nothing wasted). ALL draw gates read it:
   - terrain band outer gate (band_patches: d2 ≤ ring², nearest-edge);
   - entity cull (below); flora/zone per-vertex kills; sphere/cube/pawn/
     gallery instance gates. Same anchor everywhere: the STAGED point
     (lod_point — the band's yardstick, so draw membership is synchronized
     with terrain banding BY CONSTRUCTION); the icing alone reads the live
     render_point_pos (a smooth fade; the 1-frame delta is invisible).
2. FLORA'S FIRST DRAW GATE — the mechanism REPORT (ruled "verify the
   derivation; fallback VS; report which"): the patch-list DERIVATION IS
   structurally supported (patches record entity_refs{family,slot}; the
   params-upload + pending-flag toggle verbs exist per family) BUT it costs
   a mesh-gen dispatch per ring crossing + new per-slot draw state × 3
   families. LANDED THE FALLBACK: a per-vertex VS kill (palm/cactus/blade —
   the meshes are baked world-space with no instance channel; beyond-ring
   verts clip to vec4(0,0,-1e4,1) far behind the near plane). POP-FREE BY
   CONSTRUCTION: the kill boundary sits exactly where the icing is 1, so
   any mixed-triangle sliver is fog-colored; the huge kill offset keeps
   sliver screen extent negligible. The derivation stays named if the rig
   disagrees. ZONES gained the same per-vertex kill (their extrusions had
   NO self-fade — the GOL_FADE is the terrain tint, a recon correction).
3. ARCH/COLUMN/ANTENNA — the cull RE-POINTED to the ring as a CORRECTNESS
   gate: draw membership = center − extent ≤ ring (arch extent = half_span;
   column = max(shaft_radius, THIN 5); antenna = THIN 5 — the "center±extent"
   metric replaces the retired inset). HYSTERESIS FLIPPED OUTWARD, wholly
   beyond the ring: show when nearest ≤ ring (entering fragments at icing=1
   → invisible join), hide when nearest > ring+40 (fully iced the whole
   band → invisible exit) — both toggle edges behind the fade.
4. FOG → ICING — shade_lit's term is now smoothstep(ring − δ, ring, d), δ =
   config.veil_icing (default 40, ruled ~25-50, tunable). Composed after the
   untouched eye-fog; wall color still fog/horizon. Gallery FS same (staged-
   point anchor, layout law). NOTHING relies on it for concealment.
5. CHAIN RE-ASSERTED — PREGEN(350) ≥ EXIST(350) > RING(325) > LOD0(175), plus
   RING − ICING > LOD0 (the band sits outside the full-mesh core). The old
   EXIST ≥ FAR+75 law + ENTITY_MAX_EXTENT DISSOLVED (entities draw at the
   ring; per-entity extents are real fields now). FLAGGED, NOT STARTED: the
   thin factory band EXIST−RING = 25 wu — if the rig shows rim-pops under
   fast flight, PREGEN-8 is the named storage-weld follow-on (225→289
   layers, TILE_GRID 17→19, MAX_ACTIVE_PATCHES — a C6-registry-class cut).
   Floaters: STAY 400 existence (the flagged spawn-headroom fork) but now
   DRAW only inside the ring (the sphere/cube instance gates) — the
   silhouette leak they caused is closed at the draw set, not the radius.

NEWLY-SURFACED FOSSILS (flagged → sweep): EntityFamilyTraits.cull_base /
cull_height_scale — ZERO readers (the old per-family cull columns; 9
positional rows across 4 files carry dead values). Marked DEAD in
entity_types.hpp; the row sweep is a trivial compiler-driven follow-on
(removing the fields makes every stale row a too-many-initializers error).

GATES: glaw1 GREEN; score census GREEN; encodings clean LF. HAND-VERIFIED
(WGSL blind): config mirror field-for-field (ring/icing/strength/lod0);
icing at 2 sites (shade_lit + gallery); draw gates at 8 sites (4 instance:
pawn/sphere/monolith/gallery; 4 per-vertex: zone/palm/cactus/blade); the
frustum LOD0 gate reads fc_config.lod0_radius (the same live value as the
CPU band); finite/indoor: rooms (≤200) sit inside the ring → every gate a
no-op; band's finite short-circuit + strength=0 unchanged.
THE OBSERVABLE GATE (Jean's rig): palms/flora VANISH beyond the ring — no
silhouettes against sky; entities and terrain appear TOGETHER at the ring,
inside the icing fade; nothing visible stands on undrawn ground; ribbon
still far-visible; indoor unchanged; the ring at 6.5 vs 5.5 — config-tune
live. HELD for the gate.

## THE RIM — the continuous edge (small cut; glaw1 + census GREEN; HOLD
## for the observable gate)
The flip made flora/zone/entities vanish AT the ring, but terrain still
ended on patch scallops (the banded set is patch-granular). THE RIM makes
the terrain's VISIBLE edge a smooth circle — the last piece of "the horizon
recedes continuously."

1. patch_terrain_fs: discard beyond config.veil_ring, per-fragment — the
   terrain SIBLING of the flora/zone per-vertex kill. Anchor = the STAGED
   point (lod_point), so every hard draw-set edge (terrain FS + flora/zone
   VS + sphere/cube/pawn/gallery instance gates) is CONCENTRIC — ONE circle,
   no scallops, no slab joins, no silhouettes. The patch-granular banded set
   (nearest-edge ≤ ring, so drawn coverage runs to ring + up to a patch
   diagonal) is the circle's invisible SUPERSET — the discard always has
   drawn geometry under it, never a hole.
2. THE TASTE KNOB (config-gated, default OFF = mechanism 1 alone): a new
   config.veil_dither (repurposed a free pulse-pad float — no struct-size
   delta, offset-384 lod_point assert intact). >0.5 → the icing band
   [ring−δ, ring] DITHER-dissolves (world-space stipple noise; geometry
   condenses) instead of tinting to fog, at BOTH icing FS sites (shade_lit +
   gallery_frame_fs). Jean tunes live; set_veil_dither the dirty-gated door.
FLAGGED, NOT CHASED (Jean's call): the shadow passes are DEPTH-ONLY (no FS),
so the per-fragment rim discard has no shadow equivalent — terrain (and
flora/zone, whose shadow VS also omit the ring kill) cast shadows out to the
patch-granular set, ~one patch (≤50wu) beyond the smooth visible rim. Logged
at shadow_patch_terrain_vs; a shadow-side ring gate is a follow-on only if
the rig reads it as a shadow with no caster.

WHY IT'S SAFE: inside ring−δ the frame is unchanged (no discard, icing term
zero — pixel-identical to the flip). The discard uses the same staged point
+ same veil_ring as the CPU band, so the FS never discards a fragment the
band didn't cover (the band is the wider, patch-granular set). The dither
default 0 keeps the tint path (the flip's behavior) exactly.

GATES: glaw1 GREEN; score census GREEN; encodings clean LF. HAND-VERIFIED
(WGSL blind): config mirror field-for-field (veil_dither slots into the old
pulse-pad position on both sides; the tail veil_ring/icing/strength/lod0
aligns); the rim discard reads config.veil_ring + lod_point (concentric with
the flip's kills); the dither branch guards both FS sites; the shadow flag is
a comment only (no shadow VS touched). THE OBSERVABLE GATE (Jean's rig):
walking outward the horizon is a smooth CIRCLE that recedes continuously —
no tile scallops, no slab joins, no silhouettes; the dither knob toggles
tint↔condense live; the shadow rim (~40wu early) is the rig's call.

## TERRAIN-2 — SKIRTS (side excursion; weld #2; own rig-gated commit)
Jean's order: skirt the terrain patch mesh (plain patch edges) to hide
inter-patch cracks — precision AND LOD/T-junction — with ONE mechanism.
GoL extrusion mesh untouched. METHOD: each patch skirts its FULL
perimeter (duplicate edge ring, drop copies by skirt_depth, quad-strip
ring->copy); interior skirts overlap into the gap and hide under
neighbors (no exposed-edge detection). Skirt verts inherit the edge
verts' FINAL COMPOSITED height (read the displaced ring, no recompute) —
overlays come free, the curtain tracks the live surface.
BLAST RADIUS (exceeds the stated world.wgsl-only): the patch mesh is
implicit VS verts (65x65 grid) + a C++-GENERATED index buffer
(state.hpp) — so skirt geometry is state.hpp (index gen, LOD0 + LOD1,
compiler-verified) + world.wgsl (VS branch, blind). BOTH VS functions:
patch_terrain_vs AND shadow_patch_terrain_vs (the shadow pass SHARES the
patch IBs, render_passes.hpp:323/328 — else vi>=grid reads garbage).
MECHANISM: skirt vert (ring index k) = vertex_index PATCH_GRID_VERT_COUNT
(4225) + k; patch_skirt_grid(k) maps k in [0,256) to the perimeter grid
vertex (CW walk bottom/right/top/left), MIRRORED by state.hpp
skirt_grid_index (the two MUST agree). LOD0 appends all 256 ring
segments; LOD1 appends every-`step`th so the skirt top matches the LOD1
interior edge. WINDING (a,b,sa)+(b,sb,sa) DERIVED, not guessed: the
grid's known-front tri (i00,i01,i10) has a +Y normal = front under
cullMode=Back/frontFace=CCW, so outward-normal = front; the skirt quad
is outward on all four edges (per-edge verified). All draw paths use the
dynamic index counts (patch_index_count()/_lod1()/indirect args from
patchIndexCount_) — skirt auto-covered, no hardcoded count.
skirt_depth = 8.0 (start; heightfield curtains only show at the crack or
the finite outer rim, so generous is safe; rig-tunes).
OUTER RIM: skirt every edge (Jean's default); if the finite-world outer
perimeter reads as a wall in the rig, suppress the outermost ring (a
follow-up; the containment clamp may keep the camera off it entirely).
GATES: glaw1 GREEN (verifies the C++ index gen — lambda/constexpr/loops);
score census GREEN; world.wgsl + state.hpp clean UTF-8/LF, no CR. Blind
half = the VS drop (winding derived); the RIG is the proof + tunes
skirt_depth. OBSERVABLE: inter-patch cracks close. HELD for rig.

## TERRAIN-2 (STAGE 1) Phase A — THE MANIFOLD INTERFACE, SPHERE-DIRECTED
## (design on paper; ONE report; STOP for Jean's stamp before any code)

The destination is the SPHERE (+ family — polar/torus/non-planar sheets, the
music-visualizer environments). Stage 1 builds the INTERFACE the sphere plugs
into, filled by the heightfield as the sole placeholder cast, so the sphere
(Stage 3) is a NEW CAST behind a proven interface, not a scattered rewrite.
BINDING intent: the query face + composition fold are shaped by what a
SPHERICAL cast requires, NOT by tidying the heightfield. NO code until the
stamp. Product: audit/TERRAIN2_STAGE1_INTERFACE.md.

A1 THE QUERY SIGNATURE: manifold_resolve(query_pos: vec3, policy: u32, qi:
QueryInputs) -> SurfaceHit{position: vec3, normal: vec3, valid: u32}. INPUT =
a WORLD-SPACE vec3 (the coordinate every consumer already has; QueryInputs.
consumer_pos is already vec3) — the signature does NOT hardcode xz; the CAST
projects world-pos into its own parameter (heightfield reads .xz; sphere takes
normalize(pos-center)). This is the no-templates/no-new-struct answer: a world
position is the universal coordinate, projecting it is the cast's private
business (the intrinsic-parameter GENERATE face the generator walks is a
separate face = one of the four welds Stage 1 does NOT touch; Stage 1 freezes
the RESOLVE face). OUTPUT = a full surface point + TRUE normal RETURNED by the
cast (heightfield fills position=(x,h,z), normal=(-gx,1,-gz) — its current
values, now returned not assumed; the literal 1.0 leaves the caller). BOUNDARY
= carried in the query (Boundary{center,extent}; extent=0=infinite per today's
world_bound(0,0,0,0)); the cast returns valid=0 + boundary-projected position
outside (the pawn/camera clamp becomes a return value). THE SPHERE MAKES
FINITE NATURAL — a closed manifold has no edge, valid always 1; a large sphere
is effectively-infinite-with-seeded-novelty (Jean's model, no boundary
branch). Adopts TERRAIN-1's idle *_gradient scaffold (already returns vec3(h,
dh/dx,dh/dz), zero callers) — named, not invented. Proven: a sphere cast
implements the signature UNCHANGED (walk-through in the report).

A2 THE CAST BOUNDARY: INTERFACE (frozen) = the signature + SurfaceHit/Boundary
+ the fold declaration (ground_architecture.hpp's POLICIES[] masks ARE the
fold) + the consumer call sites. CAST (heightfield now, sphere later) = the
four welds (texel format, mesh VS, normal basis, Y-up/XZ movement+index) + the
base-shape bodies (terrain_height_at/contrib_static_base/tile_modifiers/piers/
pyramids) + the world-pos->parameter projection. Stage 3 replaces exactly the
CAST column, inherits the interface.

A3 THE COMPOSITION FOLD: the unifying insight is a BASE-SHAPE vs OVERLAY
partition = the cast/interface line. BASE-SHAPE authors (the CAST's, replaced
by the sphere) = seed-lattice*tile-amp + tile-bias + piers + pyramids. OVERLAY
authors (UNIVERSAL, fold along the normal onto any cast) = GoL + terrain waves
(voice) + radial pulses + pawn aura. The ONE declared fold: base =
CAST.base_shape(pos); displace along base.normal by the sum of GLOBAL overlays
(same for all) + CONSUMER-LOCAL terms (policy-parameterized: GoL-suppression
center, self-aura exclusion — legitimate divergence, kept). The three-sampler
BUG is only that the baked sampler omits the GLOBAL overlays; that is A4's fix.
The masks already encode this; Stage 1 makes them the ONE declaration + one run
site (kills the shadow-VS hand-fusion drift). The base/overlay line is
load-bearing: the sphere replaces base-shape, keeps overlays untouched.

A4 THE SAMPLER UNIFICATION (the free bug-fix, delta disclosed loud): CPU
estimate -> the CPU cast (ribbon Y becomes accurate, or keep documented-coarse);
baked sample_terrain_y -> cached base-shape (.x->pos, .yz->normal) + the overlay
fold; analytic query_ground_* -> analytic base-shape + fold (no delta). THE
DELTA: today the pawn stands on GoL/aura-deformed ground but placed entities sit
on un-deformed baked ground (sink/float), shadows drop aura+pulses, the
photographer clamps to un-overlaid ground; unifying makes them AGREE (a
correctness fix — paintings already hand-compensate). Visible delta today
confined to GoL zones + the pawn-aura region (waves dead). GATING RECOMMENDATION:
split b2 into b2a (the fold STRUCTURE, every policy keeps its current set ->
PIXEL-IDENTICAL, lands the interface) + b2b (the AGREEMENT flip, baked gains the
overlays -> the disclosed delta, its own rig gate). LEAN: land b2a in Stage 1,
DEFER b2b. Jean rules.

A5 THE tile_world CROSS-CUT SPLIT (TERRAIN-0 Law 2 resolved): TileState ->
TileShape{archetype,height_bias,amp_scale,activation_scale,amp_momentum} = the
CAST's shape authorship (+ the TerrainToken momentum) + TilePopulation{entity_
density,theme_spawn[],theme_idx} = the population concern (population_themes).
The F1-F4 face splits (F1/F2/F4 shape, F3 population). Two clean gifts: the GPU
mirror is already shape-only (population is CPU-only, never crosses), and one
generate_tile_state fills both (only storage+readers separate). Behavior-
identical.

PHASE B shape (sized at the stamp): b1 the interface lands (pixel-identical);
b2a the fold structure (pixel-identical); b2b the agreement flip (gated,
disclosed delta, recommend defer); b3 the finite collapse (6 branches -> one
window=intersect(follow,boundary) rule + boundary-carrying query, behavior-
identical); b4 the cross-cut split (behavior-identical). Each: pixel/behavior
gate + the no-WGSL-compiler hand-verification pass.

FIVE STAMP QUESTIONS: (1) the signature shape; (2) the interface/cast
partition; (3) the fold + base-shape/overlay partition; (4) the sampler-delta
ruling (b2a-now/b2b-defer recommended; ribbon unify-or-coarse); (5) the
cross-cut split. NO CODE until the stamp — the interface is the sphere's
foundation, gotten right on paper first.

## TERRAIN-2 (STAGE 1) b1 — THE INTERFACE LANDS (manifold_resolve;
## the pawn's normal migrated; pixel-identical; world.wgsl-only)

Phase B's first cut, the stamped interface made real. b1 lands the
manifold query face + the heightfield cast behind it, and migrates the
first consumer — the pawn's tilt normal — byte-identical.

THE INTERFACE (world.wgsl, end of the Ground Query API):
- SurfaceHit { position: vec3, normal: vec3, valid: u32 } + Boundary
  { center: vec3, extent: f32 } (extent=0=infinite, mirroring
  config.world_bound's (0,0,0,0) convention). Boundary is DECLARED but
  not yet collapsed — valid is always 1u, no boundary projection runs
  (consumers still clamp); the finite collapse is b3.
- POLICY_* id consts (world.wgsl, above POLICY_*_MASK) mirroring the
  C++ PolicyId enum byte-for-byte (a mirror-law addition; the C++ enum
  gained the pointer-note). manifold_resolve switches on them.
- manifold_height_hf(xz, policy, qi) — the heightfield cast's scalar
  height, DISPATCHING to the existing per-policy query_ground_*
  functions (delegation = byte-identical VALUES by construction). All
  8 resolve policies + a static-base default for CELESTIAL/RENDER (the
  render policy is a fused VS weld with no scalar query; not a resolve
  policy).
- manifold_resolve(query_pos: vec3, policy: u32, qi) -> SurfaceHit —
  position = (x, h0, z); normal = the eps=0.5 finite-diff gradient
  normal normalize(vec3(-dx,1,-dz)) — the Y-up 1.0 now lives IN THE
  CAST (where the sphere replaces it), no longer reconstructed by each
  caller. valid=1u.

THE FIRST CONSUMER MIGRATED: terrain_normal_at (the pawn's tilt normal,
its sole caller update_player_agent) now delegates to
manifold_resolve(vec3(xz.x,0,xz.y), POLICY_WALKER_TILT, qi).normal.
PROVEN BYTE-IDENTICAL: manifold_resolve for WALKER_TILT computes the
exact same three query_ground_walker_tilt samples at eps=0.5, the same
dx/dz, the same normalize(vec3(-dx,1,-dz)) the inline body did;
query_pos.y is ignored (only .xz read). PERF-NEUTRAL: same three
evaluations, no waste (the normal consumer uses all three samples).

DEFERRED TO THE b1 COHORT (follow-on, each a clean .position.y
extraction, single-intent + rig-gated): the position-reader consumers —
camera clamp (world.wgsl:6428), sphere orbit (2930), arch ground
(5737), cube kite (6727/6732), entity flyer (6774), agent snap (5565);
then later placement's multi-sample + photographer (both TEXTURE
readers via sample_terrain_y_at — a distinct cached-cast variant, not
the analytic path) and the pawn's step-climb HEIGHT path (the
perf-tuned paired query, left whole). NOTE for the cohort: a
height-only consumer reading .position.y relies on the compiler DCE-ing
the normal's two extra samples; confirm on the rig, else add a
height-only companion — a perf question, never a pixel one.

NO C++/GPU MIRROR, NO BINDINGS: SurfaceHit/Boundary are function-local
value types (never uploaded); b1 is a pure world.wgsl function addition
+ one C++ comment. That is why b1 is the low-risk interface-landing.

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
147/5; encodings clean UTF-8/LF, no CR. NO-WGSL-COMPILER HAND-
VERIFICATION (the law): definition order verified (POLICY consts <
structs < manifold_height_hf < manifold_resolve < terrain_normal_at
caller — every use after def); manifold_resolve defined once; the
switch total (8 cases + default, all return f32); the pawn-normal
delegation proven byte-identical above. BISECTION ANCHOR: this commit;
bisect on demo=full / demo=minimal pixel-identity at the rig (the pawn
tilts exactly as before; nothing else observable changed).
AWAITING THE RIG: pixel-identical — the pawn's body-lean is unchanged
(the normal flows through the interface but computes the same value).

## TERRAIN-2 (STAGE 1) b1-cohort — THE POSITION-READERS MIGRATE
## (manifold_position added; 7 consumers onto the interface; pixel-
## identical AND perf-neutral by construction)

b1 landed the interface + the pawn's normal (rig-confirmed pixel-
identical). This cohort moves the position-reader consumers onto it,
WITHOUT the DCE gamble b1's ladder flagged.

THE POSITION-ONLY FACE (world.wgsl): manifold_position(query_pos: vec3,
policy: u32, qi) -> vec3 — the surface POINT without its orientation,
for consumers that snap to the surface but don't orient to it. PERF-
NEUTRAL BY CONSTRUCTION: exactly one manifold_height_hf evaluation, no
normal work — no reliance on the compiler DCE-ing an unused normal (the
open question from b1, closed by construction). Cast-agnostic like
manifold_resolve (a sphere returns center+dir*radius here; the tangent-
normal work stays in the full resolve). manifold_resolve refactored to
take its position from manifold_position — the pawn normal stays
BYTE-IDENTICAL (h0 = manifold_position(..).y = the same
manifold_height_hf value b1 shipped). Disclosed: manifold_position is a
small companion beyond the stamped manifold_resolve — the sphere-honest,
perf-neutral answer to "consumers that need only the point"; foldable if
Jean prefers the DCE route.

SEVEN CONSUMERS MIGRATED (each manifold_position(pos3, POLICY, qi).y,
byte-identical: .y = manifold_height_hf(pos3.xz,..) = the original
query_ground_<policy>(xz,..)): the primary camera clamp (POLICY_FLYER),
the sphere orbit clearance, the arch ground, the cube kite home + the
cube anchor home, the entity flyer ground, and the agent ground-snap
(POLICY_WALKER_AGENT). All the analytic flyer/walker_agent scalar
consumers now speak through the interface.

STILL ON THE RAW CAST (deliberate, later): the LATENT
query_ground_flyer_gradient (5 internal samples — an idle scaffold, no
consumer); placement + photographer (TEXTURE readers via
sample_terrain_y_at — the cached-cast variant, a distinct migration);
the pawn step-climb HEIGHT path (pawn_ground_resolve's perf-tuned
paired query, left whole). The cast's own manifold_height_hf dispatch
still calls query_ground_<policy> — that IS the delegation, not a
consumer.

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
147/5; encodings clean UTF-8/LF, no CR. NO-WGSL-COMPILER HAND-
VERIFICATION: manifold_position defined once (world.wgsl:2992) before
all 8 callers (manifold_resolve + the 7 consumers); every migration's
xz equals the original arg (proven per-site); the pawn normal unchanged.
BISECTION ANCHOR: this commit; bisect on demo=full/minimal pixel-
identity + frame-time (perf-neutral by construction — the rig confirms
no regression on the camera/entity/sphere/cube hot paths).
AWAITING THE RIG: pixel-identical (every ground clamp/snap returns the
same value) AND frame-time unchanged (one height eval per consumer, as
before — no wasted normal).

## TERRAIN-2 (STAGE 1) b4 — THE tile_world CROSS-CUT SPLIT (A5;
## TileState -> TileShape + TilePopulation; behavior-identical; C++,
## COMPILER-VERIFIED)

SEQUENCING NOTE (disclosed): taken BEFORE b2/b3 by deliberate risk
choice. b4 is pure C++ — glaw1 COMPILES the real TU, so the split is
compiler-verified, not blind — behavior-identical, contained to
tile_world.hpp, and it isolates the base-shape's tile half the sphere
replaces. b2 (the fold) is the campaign's most intricate BLIND-WGSL
change (FP bit-identity on the hottest inner loop + the aura self/
external forms + GoL suppression + the perf-tuned walker_pair); it gets
a focused careful pass next. b3 (finite collapse) sits between. Order
flagged to Jean; redirectable.

THE SPLIT (TERRAIN-0 Law 2 resolved): TileState carried two welded
concerns — landform SHAPE and entity POPULATION. Split by concern:
- TileShape { archetype, height_bias, amp_scale, activation_scale,
  amp_momentum } — the heightfield CAST's per-tile base-shape
  modulation (the sphere replaces this at Stage 3). Read by the GPU
  tile upload, estimate_terrain_height (F1), tile_archetype (F4), the
  neighbor archetype roll, the terrain-token emission.
- TilePopulation { entity_density, theme_spawn[PopFamily::COUNT],
  theme_idx } — the population/themes concern (NOT terrain shape). Read
  by tile_apply_spawn_mult (F3).
- TileState { TileShape shape; TilePopulation pop; } — the two rolled
  together at ONE generation moment under ONE (gx,gz) key
  (generate_tile_state fills both); only the TYPE separates the
  concern, the readers touch the half they own.

BEHAVIOR-IDENTICAL BY CONSTRUCTION: nesting preserves every field,
default, and computation order; ~19 field accesses re-pointed to
.shape / .pop (writes in generate_tile_state; reads in the GPU upload,
the neighbor roll, the token emission, F1/F3/F4). NO C++/WGSL WIRE
TOUCHED: GPUTileEntry (the GPU mirror) was already shape-only — it
reads it->second.shape.amp_scale now, the same value; the population
half never crosses to the GPU (A5's structural gift, confirmed).
CONTAINED: TileState/tileCache_/generate_tile_state are referenced only
in tile_world.hpp (+ a cartridge.hpp wiring instance); the .archetype/
.theme_idx hits elsewhere (entity_pipeline/spawn_engine/bodies) are
UNRELATED structs (gate/sel/plan), not TileState.

A5 NOTE: this draws the concern LINE in the type (shape vs population,
each reader declaring which it owns). The physical relocation of
TilePopulation into population_themes.hpp (a cross-module move) is a
heavier follow-on, pulled if wanted; the type-split is the behavior-
identical b4 deliverable and is what the sphere needs (TileShape is now
a named, separable thing the base-shape authorship owns).

GATES: glaw1 GREEN full + minimal (COMPILER-VERIFIED — the real TU
through g++, name lookup + syntax proven, not merely hand-checked);
score census GREEN; sentinels 147/5; encodings clean UTF-8/LF, no CR.
Residual flat-access grep: ZERO (every TileState field now via .shape/
.pop). BISECTION ANCHOR: this commit; behavior-identical (same tile
values, same spawn, same terrain — the split is type-only).
AWAITING THE RIG: behavior-identical (terrain shape, spawn density, and
themes all unchanged; the cache holds the same data reorganized).

## TERRAIN-2 (STAGE 1) — CLOSE-OUT (the two rulings; Stage 1 complete;
## the radius cap HELD on the disclosure gate)

Jean ruled the b-handoff's two open decisions; Stage 1 closes. The
manifold interface is landed, the heightfield sits behind it, the
sphere's plug-in point is real. This close-out is comment-only in
world.wgsl (the two rulings as notes) + this record; ONE sub-decision
is held on Jean's own disclosure gate.

b2 — THE FOLD: MET BY b1; BODY-MERGE DEFERRED (ruling, noted at the
manifold_height_hf header). b1's dispatch IS the one declared place the
fold runs, in the POLICIES[]-declared order — b2a's structural goal is
MET; no analytic site hand-copies the fold. The physical merge of the
per-policy query_ground_* bodies into one manifold_fold is NOT pulled
(low-value — they don't drift; high-FP-risk blind shader work); the
real drift hazard lives in the WELDS (bake, patch_terrain_vs,
shadow_patch_terrain_vs) = Stage 3, so the body-merge rides Stage 3's
weld work IF wanted. b2b (the agreement flip) stays deferred by design.

b3 — CONTAINMENT STAYS A SEPARATE SHELL; THE QUERY STAYS PURE (ruling,
noted at the SurfaceHit/Boundary header). world_bound is a containment
SHELL over infinite ground (MOOD_FINITE_OUTDOOR = finite, no walls),
not terrain extent; the code already keeps the surface query ("where is
the ground") separate from containment ("am I allowed here"). RULED
KEEP SEPARATE — do NOT fold the shell into resolve-time valid; the
manifold query stays PURE; containment stays its own layer; consumers
clamp as today. Boundary{center,extent}+valid stay DECLARED-DORMANT —
their real home is the manifold FAMILY's closedness story (a sphere is
closed: no edge, valid always 1; closedness replaces containment on
closed manifolds). The stale "b3 makes this real" comment is CORRECTED
to the ruling. Three converging rationales: the code already separates
them; finiteness-as-enclosure (Jean's indoor-walls instinct) is a
bounding layer not a terrain property; the sphere model makes
containment a flat-manifold-specific shell, belonging AROUND the query
not IN it.

THE INVARIANT SUB-DECISION — HELD ON THE DISCLOSURE GATE. Ruling was:
CAP finite_radius AT 3 (active_radius 7 >= 2*finite_radius holds across
[1,3]; retires the origin-pin workaround; no pregen-radius boot cost),
WITH the gate "if a live mood actually uses 4, report before capping."
FINDING (disclosure triggered): ALL FOUR finite moods declare
finite_radius_max = 4 — MOOD_INDOOR_FLAT / INDOOR_VAULT /
FINITE_OUTDOOR / FINITE_OUTDOOR_REF, all {finite, 1, 4} at
spine_state.hpp:226-229. derive_finite_radius yields {1,2,3,4}, so
radius-4 finite worlds ARE live (~1/4 of finite spawns per mood).
Capping max 4->3 SHRINKS the largest finite worlds: radius-4 (9x9
patches, 450x450 wu) -> radius-3 (7x7, 350x350 wu), a content change to
every finite room. Per Jean's own gate, the cap is HELD for his
confirm: cap at 3 (accept the shrink) OR raise PATCH_PREGEN_RADIUS to
>=8 (preserve radius-4, wider ring for all worlds — small boot cost) OR
another extent ruling. NOT capped blindly. Note: the follow-window
streaming unification (the origin-pin retirement) was NOT part of this
close-out (the rulings are the only code; b3's structural streaming
rewrite is not done — the origin-pin stays, harmless, until/unless the
cap+unification is pursued).

=== STAGE 1: COMPLETE ===
Landed + rig-confirmed: b1 (interface + pawn normal), b1-cohort
(manifold_position + 7 consumers), b4 (tile_world shape/population
split, compiler-verified). Subsumed: b2a (fold met by b1's dispatch).
Ruled + noted: b2 body-merge (deferred to Stage 3), b3 containment
(stays a separate shell; query pure; Boundary declared-dormant).
Deferred by design: b2b (agreement flip). RESOLVED: the finite_radius
bounding decision → DEFERRED TO b3 (see resolution below); no radius
code lands in Stage 1.
THE OUTCOME: terrain presents a manifold interface (manifold_resolve
orient-to-surface, manifold_position snap-to-surface); the heightfield
is the sole cast BEHIND it; the Y-up assumption lives in the cast not
the callers; the cast/interface boundary is drawn (A2); the containment
shell is separate from the pure query. THE SPHERE'S PLUG-IN POINT
EXISTS: Stage 3 writes a spherical cast behind the frozen signature,
rewriting only the four welds (storage, mesh VS, normal basis,
movement/index), inheriting everything above.

=== STANDING (pulled, not pushed) ===
- STAGE 3 THE SPHERE + the manifold family (polar/torus/non-planar):
  the four-weld rewrite as new casts behind the frozen interface —
  pulled when a demo wants a non-heightfield world; the b2 body-merge
  rides this if wanted; the welds are the real cost, paid once under a
  test baseline behind a proven interface.
- b2b THE AGREEMENT FLIP: "baked consumers see the live surface" —
  its own gated cut, pulled when wanted.
- THE FINITENESS-AS-ENCLOSURE QUESTION (Jean's Model B): move
  finiteness ENTIRELY to the occupier/collision layer (walls bound
  you; terrain stays pure infinite manifold), retiring the containment
  shell too. NOT opened now — Stage 1 closes with the shell separate-
  but-present; a candidate next campaign; b3's grounding (shell over
  infinite ground) is its starting evidence.
- THE finite_radius BOUNDING DECISION: RESOLVED → deferred to b3 (see
  the disclosure-gate resolution below). No radius code in Stage 1.
- THE SDF EXCAVATION + dead-code sweep: wire-first-clean-second; the
  TERRAIN-1 M6 disposition table is the standing map.

GATES: glaw1 GREEN full + minimal; score census GREEN; sentinels
147/5; encodings clean UTF-8/LF, no CR. Comment-only world.wgsl (the
two rulings as in-place notes; the stale b3 comment corrected) — no
behavior change, nothing for the rig to re-verify beyond "still builds."

=== DISCLOSURE-GATE RESOLUTION: finite_radius → DEFERRED TO b3 ===
Jean's ruling on the held sub-decision: DEFER to b3, land no radius code
in Stage 1. Two facts completed the disclosure and inverted the intent:
(1) SCOPE — raising PATCH_PREGEN_RADIUS 7->8 touches ONLY open worlds:
in finite mode active_radius is capped to finite_radius
(patch_system.hpp:569-571) and the center is origin-pinned (:558-560),
so the raise is swallowed there; its whole runtime effect is on the
(majority) open worlds — pregen ring 15x15->17x17 (MAX_ACTIVE_PATCHES
225->289, +28%; state.hpp:69), tile grid 17x17->19x19. (2) TIMING —
radius-4 is NOT at risk today: the invariant active_radius >=
2*finite_radius is a precondition of the FOLLOWING-window (= b3, still
deferred), not of current behavior. Origin-pin + radius-cap makes the
finite pregen set exactly the box every frame, so radius-4 (9x9=81
patches) renders fully inside the 225-patch buffer with no popping. The
cap (Option 1) would have BROKEN radius-4; the raise (Option 2) buys
nothing present, only pre-pays open-world memory for a deferred b3.
CONCLUSION: neither cap nor raise is needed for present correctness;
both pre-pay for b3. The radius bounding choice (raise pregen / lower
finite_radius_max / keep origin-pin as the degenerate case) is exactly
b3 finding (a)'s ruling — decided WHEN b3 lands, with the full
following-window in hand. Stage 1 therefore closes with ONLY the two
comment rulings (6ed3520) + b1/b1-cohort/b4; the origin-pin stays,
harmless. b3 inherits the bounding decision intact.

════════════════════════════════════════════════════════════════════
RESIDUE SWEEP — EXECUTION (T0/T1/T2 per the stamped plan,
RESIDUE_SWEEP_RECON §3; §4a/T3 HELD for its own ruling)
════════════════════════════════════════════════════════════════════

T0 — THE COMMENT SWEEP (one commit, zero behavior). All §2 STALE-
BREADCRUMBs discharged: cartridge.hpp rows 1-7 (terrain-mirror divider
dropped; "~400 lines below" prose fixed; six→five adapter pairs +
family_dispatch.inl → "inlined beside the table"; pyramid mesh-gen
narration trimmed to the rationale; ROSTER path → contracts/; the
truncated dispatch-table comment completed; U8's "silently dropped"
failure mode superseded → validate_spine boot law). world.wgsl: the 4
"→ C6"/follow-on pointers dropped (4419/5049/5605/8372 regions), §9
TOC + header now say two families (the pyramid's realization IS the
terrain). Contracts veil tombstones dropped (surface_services, state).
Demos-adjacent: CMake THE_BOARD_DEMO help names the matrix column (not
demos/<name>.hpp), demo_contract_v0.md status note marks full/minimal
retired into the matrix, matrix.hpp drops the "148" magic count.
RIBBON RULING EXECUTED: SPAWN_CHANCE 0.900f is now the AUTHORED value
(TESTING note deleted here + the spawn-engine summary bullet removed;
the distributions pass revisits it properly). Gap-explainer tombstones
KEPT per the stamp (RAYMARCH/SDF, husk-sweep gap markers).

════════════════════════════════════════════════════════════════════
THE PARKED LEDGER (standing section — the ONE place open flags live.
Future sweeps read THIS, not the tree. Consolidated at residue-sweep
T0. Discipline: when a flag lands or dies, strike it HERE in the same
commit.)
════════════════════════════════════════════════════════════════════

P-1  WIDE-TRAITS RULING (residue recon §4a; T3, HELD) —
     EntityFamilyTraits carries 10 zero-reader fields beyond the swept
     cull_* pair (short_name, max_instances, grounded, creates_ground,
     piers_per_entity, has_footprint, spawn_roll_prop, spawn_chance,
     mood_multiplier, gpu_ground_y); the pipeline reads the per-family
     *Config/*Prop constants directly, bypassing traits. Contract-face
     shrink, each field cut re-columns 9 rows. Verify half-wired vs
     vestigial PER FIELD (esp. creates_ground / gpu_ground_y) before
     its own ruling.
P-2  TEST-RIG PIERS (ship checklist) — patch_system.hpp
     TESTING[test-rig-piers]: debug ground fixture (ramp + plateau +
     block at pier slots 0-2), NOT a roster piece (ROSTER-1a §1
     ruling). Mortal retirement: dies at ship.
P-3  PREGEN-8 CONTINGENCY (rig-triggered) — the thin factory band
     EXIST−RING = 25 wu. IF the rig shows rim-pops under fast flight,
     the storage weld is the named follow-on: 225→289 layers,
     TILE_GRID 17→19, MAX_ACTIVE_PATCHES — a C6-registry-class cut.
     Sleeps until the rig says otherwise.
P-4  OPTION-B WGSL PIN (registry follow-on) — C6 pinned the C++ side;
     world.wgsl @binding literals stay hand-mirrored with
     launch-validation as the net (Option A). Option B — ONE authored
     source generating both sides — is the named "true pin"
     (BINDING_REGISTRY_RECON §2).
P-5  SILENT-CLOCK CONTRACT (driver-law hole) — the silent-BPM axis
     (default-BPM-when-music-off) has no owner and no authored default
     (FRAME_CONDUCTOR_RECON, time/clocks row). Wants an owner when the
     driver law next opens.
P-6  TERRAIN-2 b3 (the finite collapse) — parked at Stage-1 close; the
     radius-bounding choice (raise pregen / lower finite_radius_max /
     keep origin-pin degenerate) is b3 finding (a)'s ruling, decided
     WHEN b3 lands with the full following-window in hand.
