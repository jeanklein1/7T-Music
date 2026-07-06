# THE CARTRIDGE CONSTITUTION
Branch FINAL_LAPS · applies to both cartridges · repo home: src/docs/
Ratification = CC audits every claim here against the tree (§5's counts,
§3's order) and the document ships same-commit with any change it governs.

## §0 — PREAMBLE: WHAT A CARTRIDGE IS
A cartridge is the composition root and the ambient world: the one place
where entities are assembled into an exhibit, and the environment every
module breathes. It is swappable as a whole — the_chord (the lab) and
the_board (the exhibit) are two instances of one anatomy, mirrors except
where a divergence is authored and declared. Nothing musical happens in
it except through the canvas; nothing entity-shaped lives in it except
assembly.

## §1 — THE SINGLE-ORGANISM LAW
Modules are class-body includes; the cartridge is one class whose
chapters see each other. This is design, not accident: it buys order-free
wiring (chapters reference chapters without forward ceremony) and
complete-class visibility (a conductor can be handed the whole organism
through one pointer). The boundary that keeps it honest: ambient SIGHT is
free, ambient WRITING is not — cross-cutting writes pass through declared
seams (the keyhole, the signal, the bank), never through casual reaches.
A stranger reading this codebase will mistake the single translation unit
for a hack precisely once: this section exists so the mistake dies on
first contact.

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

## §3 — THE CONDUCTOR CONTRACT: update() AS THE SCORE
update() reads top to bottom as the frame's score, in fixed cascade
order: clocks adopt (seconds, beats, and the TEMPO FOLLOWER — beat_rate
held-last through stops, defaulting to the 100 BPM CALIBRATION ANCHOR at
which authored idle motion equals wall-clock feel; periodic idles
converge to it as their modules are touched) → analysis is read → the
canvas ticks (all coupling decodes, nowhere else) → entity conductors
flush and advance, ONE CALL PER ENTITY → consumers consume (camera,
mounts — reads, never laws) → the frame signal fills FROM POST-ADVANCE
STATE (the SNAP-1 ordering: what the kernels snap to and what the rings
render must derive from the same advance) → uploads ship. THE KEYHOLE: a
conductor may reach through its cartridge pointer for declared services —
the clocks, the canvas's params, the GPU wires, its own resolved bindings
— and for nothing entity-private of a peer. Consumers consume;
conductors orchestrate; the cartridge itself merely calls.

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
realizes geometry; no CPU law duplicates a shader law.

## §5 — THE EXCEPTION LEDGER (the living section)
An exception is a tagged deviation with a named decision and a
retirement condition. This section IS the census; it updates same-commit
with any change to its entries. Classes and counts as of this commit:
- COMPAT (1 debt, two files): pawn.inl's aura ramp holds the legacy
  Trajectory; trajectory.hpp's COMPAT section exists for it. Dies at M3.
- TESTING (1): ribbon SPAWN_CHANCE 0.9. Dies at ship.
- DIAG-unwrapped (1): [Ribbon] SPAWN stdout. Wrapped at ship.
- DRIVERLESS (11 shader tags): capabilities whose drivers the demolition
  removed (terrain amplitude, sphere/floater color, band motion, mode
  uniforms, GoL scales, pulses). Each dies by revive-or-delete when its
  region is next worked.
- BOOT-NEUTRAL WRITES (1 block): init writes retired uniforms to their
  neutral values so driverless capabilities rest true. Dies with the
  last driverless entry it serves.
- TEARDOWN BULK SWEEPS (2): mood-transition teardown clears by sweep
  rather than per-entity eviction. Documented; dies if eviction ever
  unifies.
- EVICTION THUNKS (1 class): dispatch_evict_* live cartridge-side though
  eviction is lifecycle (a §2 trespass). Die as entities absorb their
  evictors on next touch.
- WHITELIST (1): the_lab.cpp reads the contract directly — the permanent
  instrument-panel exception, by charter.
- NAMED TODO (1): world.wgsl seam-map binding 144 cleanup.
- HISTORICAL NARRATION (33 tags): Scope-B/DONE archaeology. PENDING
  RULING F3 (strip and keep SEAM[], recommended).
Anything deviant and untagged is a bug in this document first.

## §6 — BOOT, HOT-RELOAD, TEARDOWN
BOOT: construct state → init GPU → compile pipelines (loaders guard
missing files; arrays sized to contents — the crash law) → bind the
signal layout (all canvas resolves happen once, here; resolve fails soft
and the bind log is the truth) → boot-neutral writes → first adoption.
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
