# THE CARTRIDGE CONSTITUTION
Branch FINAL_LAPS · applies to both cartridges · repo home: src/docs/
Ratification = CC audits every claim here against the tree (§5's counts,
§3's order) and the document ships same-commit with any change it governs.

## §0 — PREAMBLE: WHAT A CARTRIDGE IS
A cartridge is the composition root and the ambient world: the one place
where entities are assembled into an exhibit, and the environment every
module breathes. It is swappable as a whole — the_chord (the lab) and
the_board (the exhibit) are two instances of one anatomy, mirrors except
where a divergence is authored and declared. Mirror conventions (the
practiced law): mirrored-module deltas are byte-identical (ribbon.inl,
pawn.inl); ribbon.inl carries a UTF-8 BOM; world.wgsl is BOM-free LF;
world.wgsl deltas are byte-identical between cartridges. Nothing musical
happens in it except through the canvas; nothing entity-shaped lives in
it except assembly.

## §1 — THE COMPOSITION LAW (amended 2026-07-11; supersedes the
## single-organism law)
THE TARGET: modules are real headers that own their state structs
and laws; the cartridge is the COMPOSITION ROOT — it declares the
member instances, includes the headers at file scope, calls the
conductors, and owns assembly, nothing else. Sight is granted by
explicit parameters (state references, the keyhole pointer,
declared services) — never by ambient membership. What this section
once kept honest by discipline (sight free, writing through
declared seams) the boundary now keeps honest by scope: a converted
module's every dependency is visible in its includes and its
signatures.
THE TRANSITIONAL REGIME: during the header ladder two regimes
coexist lawfully. A CONVERTED module lives at file scope under the
target law and sees nothing ambiently. An UNCONVERTED module
remains a class-body include under the prior law, unchanged. Each
ladder stage moves exactly ONE module across, behavior-identical,
golden-gated; the cartridge remains ONE translation unit throughout
— the change is boundary honesty, not compilation strategy.
THE MISTAKE CLAUSE (held, doubled): a stranger will mistake the
single TU for a hack precisely once, and the remaining class-body
includes for the destination precisely once; this section exists so
both mistakes die on first contact.
COMPLETION: when the last module converts, the transitional clause
is struck and §1 becomes the composition law alone.

## §2 — CHAPTERS AND RESIDENCY
The cartridge is organized in banner-titled chapters; a chapter earns a
banner when it owns a concept (state block, dispatch family, lifecycle
phase), and chapters order from state toward lifecycle. THE RESIDENCY
LAW: what lives here is assembly — composition of modules, orchestration
(update), cross-entity wiring, boot, teardown, and the frame-signal fill.
What must NOT live here: entity laws, entity state, coupling decodes
(the canvas's), GPU realization (the shader's). Known trespasses are not
hidden; they are §5 entries with retirement conditions (the eviction
thunks are the standing example).

## §3 — THE CONDUCTOR CONTRACT: THE FRAME AS A SCORE
The frame's score is fixed-order across TWO MOVEMENTS — update() then
render(), the host calling them in that order every frame.
update(): the frame signal builds from analysis (clocks and stats
copied; the sky words ship NEUTRAL ZEROS — see the SNAP-1 clause) →
clocks adopt (seconds, beats, and the TEMPO FOLLOWER — beat_rate
held-last through stops, defaulting to the 100 BPM CALIBRATION ANCHOR
at which authored idle motion equals wall-clock feel; periodic idles
converge to it as their modules are touched) → the canvas ticks (all
coupling decodes, nowhere else) → the pawn conductor flushes → the
signal uploads.
render(): the ribbon conductor advances (ONE CALL PER ENTITY holds for
both conductors) → the sky words are REWRITTEN from post-advance state
(the SNAP-1 clause: resync_sky_head is the authoritative author,
ordered after the tick and before dispatch; queue writes land in
submission order, so what the kernels snap to and what the rings render
derive from the same advance — the neutral fill exists so losing the
resync fails loud, not silently one frame late) → kernels dispatch →
passes encode.
THE KEYHOLE: a conductor may reach through its cartridge pointer for
declared services — the clocks, the canvas's params, the GPU wires, its
own resolved bindings, the spine-owned player/flight state and raw
input (the sky author's seat; its two writes are SEAM-tagged in place),
and the spine's terrain and spawn-engine services — and for nothing
entity-private of a peer. Consumers consume; conductors orchestrate;
the cartridge itself merely calls.

## §4 — THE DOORS LAW
Boundaries are defined by transport, one door per source kind:
THE WORLD enters through LAYOUTS — any kernel may read the substrate
(terrain, atlases) through its own sanctioned bindings; reading the world
is every entity's charter.
PEERS enter through THE SIGNAL — inter-entity state crosses frames only
via the frame-signal block, CPU-composed (the sky_* block is the worked
example; the law was minted when the pawn's saddle joined the ribbon's
frame: one composer for pawn orientation, two doors for its ingredients).
MUSIC enters through THE BANK — entities read pipes; they never learn
what drove them.
Beneath all three, GPU SOVEREIGNTY: the CPU authors intent, the GPU
realizes geometry; no CPU law duplicates a shader law except as a
DECLARED TWIN under §6's hot-reload rule (the MOUNT_* frame mirrors are
the standing instance — lockstep-commented both sides, drift test
stated).

## §5 — THE EXCEPTION LEDGER (the living section)
An exception is a tagged deviation with a named decision and a
retirement condition. This section IS the census; it updates same-commit
with any change to its entries. Classes and counts as of this commit:
Counting convention: mirrored pairs count once; counts are per
cartridge unless stated.
- TESTING (2): ribbon SPAWN_CHANCE 0.9; test-rig piers (ramp/plateau/block
  at pier slots 0-2, TESTING[test-rig-piers], setup_test_rig_piers — a
  debug ground fixture, not a roster piece, ROSTER-1a). Both die at ship.
- DIAG-unwrapped (6 sites, tag-greppable as 'DIAG-unwrapped' plus the
  ribbon's SEAM[ribbon:L1]): [Ribbon] SPAWN; the periodic agent census
  + [Player] pos block (cartridge.hpp); [Photographer] Capture,
  [Gallery] slot, [Authored] Rotated, [WallPainting] Placed
  (gallery.inl). Autonomous stdout, all tagged in place. Wrapped at
  ship. (Key-command feedback prints are the instrument's UI, not
  census members.)
- DRIVERLESS (11 shader tags + 1 CPU landing site): capabilities whose
  drivers the demolition removed (terrain amplitude, sphere/floater
  color, band motion, mode uniforms, GoL scales, pulses, orb inputs,
  the stats-array infrastructure; CPU: orbs speed_mult). Each dies by
  revive-or-delete when its region is next worked.
- BOOT-NEUTRAL WRITES (1 block): init writes retired uniforms to their
  neutral values so driverless capabilities rest true. Dies with the
  last driverless entry it serves.
- TEARDOWN BULK SWEEPS (2 sites in the transition path): teardown_world
  clears every family by slot loops, and the finite-mode ribbon sweep
  inside TEARDOWN. By sweep rather than per-entity eviction;
  documented; dies if eviction ever unifies.
- EVICTION THUNKS (1 class, 12 functions in the_board; 13 in the_chord,
  which keeps a dispatch_evict_noop): dispatch_evict_* live
  cartridge-side though eviction is lifecycle (a §2 trespass). Die as
  entities absorb their evictors on next touch.
- WHITELIST (1): the_lab.cpp reads the contract directly — the permanent
  instrument-panel exception, by charter.
- NAMED TODO (3): world.wgsl seam-map binding 144 cleanup; entities.inl
  spawn-rules placeholder (dies with the object vocabulary); world.wgsl
  is_roaming retained field (dies at the next struct relayout).
- LATENT[gate-a-shared] (14 sites, tag-greppable in state.hpp): the
  ROSTER-1b gate-(a) SHARED pieces — 9 SH·mb (an exclusive buffer/texture
  bound into a megabind; retire by re-sectioning the named group) + 5
  SH·dc (a co-owned instance store plus a forward draw that isn't
  self-count-gated; retire by a behavior-identical `if(indexCount==0)
  return` draw self-gate, then skip). Created-but-pristine under the
  roster's Rider A while disabled; each dies when its retirement is paid.
  Only indoor_shell (SEPARABLE) skips creation today. Full cost table:
  audit/ROSTER_GATE_A.md.
- HEADER LADDER (§1 amended 2026-07-11): the single-organism law is
  superseded by the composition law under a two-regime transitional
  clause (a CONVERTED file-scope header vs an UNCONVERTED class-body
  include coexist lawfully). Converted leaves so far: seed_utils
  (LADDER-1 c1). The remaining modules stay class-body includes under
  the prior law until their stage. This transitional entry dies when the
  last module converts and §1's transitional clause is struck. Ladder
  record: audit/LADDER.md.
- HISTORICAL NARRATION (33 tags): 'Scope B' / DONE[] archaeology — 22
  primary DONE[] tags + 11 'Scope B migration' banners, cross-references
  excluded. PENDING RULING F3 (strip and keep SEAM[], recommended).
Anything deviant and untagged is a bug in this document first.

## §6 — BOOT, HOT-RELOAD, TEARDOWN
BOOT: construct state → init GPU (the boot-neutral writes land here,
immediately after, so driverless capabilities rest true before any
pipeline exists) → compile pipelines (loaders guard missing files;
arrays sized to contents — the crash law) → bind the signal layout (all
canvas resolves happen once, here; resolve fails soft and the bind log
is the truth) → first adoption.
HOT-RELOAD: world.wgsl is live; purely visual constants prefer WGSL-side
residence for the edit-save-look loop. Where a CPU twin must exist (the
MOUNT_* frame mirrors), the GPU set is the TUNING AUTHORITY, the mirror
copies settled values, both sides carry lockstep comments, and the drift
test is stated (the rider is the frame law's). Rebuild-tier dials live in
module consoles.
TEARDOWN: the sweeps of §5, plus the rule that teardown restores
rest-state truthfully — a torn-down world re-boots to identity.

## §7 — AMENDMENT
This document changes in the same commit as the change that would
violate it; a doc may lead the code only under an explicit
PENDING[stage] marker. Jean is the ruling authority; CC audits claims
against the tree at each amendment; disagreements between this document
and the tree are resolved by fixing whichever one is lying.
