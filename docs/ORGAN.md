# ORGAN — the master control panel

The panel is a VIEW of the program, never a second copy. Its subjects
are the persistent CPU homes CHORD built — one struct per cadence
block, one write path per change. The panel puts dials on those homes.

## The three tiers of a parameter
Every enrolled parameter has, in principle, three tiers:
1. CONSTANT — its design value (a stop, drawn or not).
2. IDLE — its oscillation around the design value when uncoupled or
   unstimulated: amplitude, period, waveform (the tremulant).
3. COUPLED — the ear that drives it (one of SignalLayout's seven),
   with gain and shape (the keys).
ORGAN_0 ships tier 1 live and reserves columns for tiers 2 and 3 in
the same registry — one registry, three tiers, never three systems.
Idleness law restated: a parameter without a coupling in this run is
a control-panel design constant; any procedural parameter is a
coupling candidate by design.

## The tier determines the dial (ORGAN_2)
THE RULING. A parameter's tier decides what kind of dial it may wear.
A driven parameter gets dials on its DRIVER — rest value, gain,
range — and never a dial on the value. Putting a tier-1 dial on a
tier-3 parameter builds a control that argues with the driver every
frame and loses every frame; the operator sees a slider that snaps
back and reads it as a broken panel rather than as a parameter that
already has an author.

WHAT THIS SETTLES. O1a's per-frame readings are not defects and want
no repair. `fog_density`, `fog_color`, `aura_enabled` and
`pawn_aura_height` are tier-3 parameters wearing tier-1 dials — the
enrollment was wrong about the tier, not the plumbing. The write path
reaches them correctly (O1d proved the upload is honest); their
drivers simply speak again a millisecond later, which is what a
driver is for.

WHAT IT DOES NOT SETTLE. A driven parameter still needs somewhere
durable to put a rest value and a gain, and that surface does not
exist yet. Naming the ruling is not the same as building it.

## The drivers' room (ORGAN_2a)
The surface the ruling named. contracts/driver_surface.hpp holds
DRIVER_TABLE (the authored code panel) and DRIVER_LIVE (block 3, the
panel's fourth home — the exception that proves the sovereignty
boundary: a contracts-tier CPU surface built FOR the panel, no
GPUState accessor needed). The room owns rests and gains at the
seams, never a module's facts: out = rest + gain·(driven − rest) at
the fog flush; intent/attack/release/height_gain at the aura tick,
where the intent is the presence ramp's rest target, moved home from
PawnState. Driven values enroll as read-only witnesses (ro column):
the panel meters them beside their drivers' dials, organ_set refuses
them, export skips them. No upload and no dirty flag: the seams that
read the room each tick are its flush.

## The compiled-registry law
The registry is COMPILED, not parsed. Enrollment is one macro line
beside the field it enrolls; the compiler produces the manifest from
the same declaration the program reads. A registry the compiler did
not build can drift; this one cannot. (The lineage: 0b-4's markers,
the schema as one home, witnesses that read their subjects.)

## The sovereignty boundary
The panel writes CPU-authored homes only: DesignConfig, the CHORD
blocks (agent_room, scene_constants, frame_r's lighting), field_bus's
authored rows. GPU truth (positions, vp, camera, simulation state) is
never a panel subject — the panel may one day DISPLAY it, but the
write path does not exist by law.

## Instance and definition (ORGAN_1)
A dial the panel can write is not yet a dial the panel owns. Where a
home has a second author, the panel's write is an INSTANCE — true
until that author next speaks — and the durable fact is the
DEFINITION the author reads. ORGAN writes definitions by default and
lets the program's own author produce the instance, because a panel
that produced the instance itself would be the second copy the
charter forbids.

### The census: the mood author
`[Mood] Applied` is printed by `apply_mood`
(direction/mood.hpp), the orchestrator behind every mood change and
every boot. It reads ONE profile and fans it out:

| what it authors | from | reaches |
| --- | --- | --- |
| frustum cull, GoL gate, aura policy | `allow_*` | renderer / GoL / pawn |
| sun direction, colour, intensity, ambient | `sun_*` | `apply_mood_lighting` → the deps → `upload_lights` → `GPULighting` |
| clear colour | `clear_color` | `clearColor_` |
| terrain amp ceiling, indoor height cap | `terrain_amp_ceiling`, `wall_height` | GPU config |
| spot lights | `indoor`, `wall_height`, `ceiling_type` | the spot array |
| indoor shell, camera ceiling | `indoor`, `ceiling_type`, `wall_height` | the shell + the clamp |
| sky dome | `ORB_MOOD_TABLE[mood]` | the orbs |

The profile it reads used to be `MOOD_TABLE`, a constexpr table no
runtime edit could reach. It now reads `mood_def(mood)` —
`MOOD_LIVE`, seeded from `MOOD_TABLE` at load. `MOOD_TABLE` keeps two
jobs, seeding that array and standing under its asserts; every
runtime reader in the tree moved with the apply, so there is one
in-force home and not two. (The constexpr readers in
bodies/gallery.hpp stay on the design table on purpose: they are
compile-time budgets, and a wall's allowance is not a live dial.)

### The three layers, for the sun
1. DEFINITION — `MOOD_LIVE[mood].sun_*`. What the mood means.
2. INSTANCE OF RECORD — `sunDirection_` / `sunColor_` /
   `mood_state_.sun_*`, written by `apply_mood_lighting`.
3. HOME — `lightingStage_.sun`, written by `upload_lights` and
   recorded by `upload_lighting`. This is the panel's subject.
A write to 3 is undone the next time 1 is read. A write to 1 flows
through 2 and 3 by the program's own path, with nothing added.

And the fan, made explicit by 2c's evidence: from the instance of
record the author fills every window the fact has — `config.sun_direction`
(the compute face's shadow VP; frame_r is unreachable there by A8a),
`lightingStage_.sun` (fragment shading), and `PhotographerConfig.sun_direction`
(filled at the photographer's own site, on its own cadence). Windows,
not homes: each is written from the record by the author's fan, and the
panel's durable subject for direction is therefore the DEFINITION alone
(2c) — the instance enrolls as a witness beside it.

### The eligibility rule
A field may carry a definition target only if the mood apply is its
ONLY runtime reader. The atmospheric group — `sun_direction`,
`sun_color`, `sun_intensity`, `sun_ambient`, `clear_color` — passes.
The structural group — `finite`, the radii, `indoor`, `ceiling_type`,
`wall_height`, `terrain_amp_ceiling`, the `allow_*` flags — does not:
world generation reads it, and rewriting it without regenerating the
world means nothing at best and disagrees at worst. The rule is
stated in code beside `MOOD_LIVE`.

THE WORLD ANALOG (ORGAN_2b). A tier field may take a TIER definition
only if `upload_agent_registries_to_gpu` is `TIER_LIVE`'s only
runtime reader; compile-time budget readers stay on the design table
`AGENT_TIER_GAINS`, the same exception the gallery's wall allowances
take against `MOOD_TABLE`. That rule is stated in code beside
`TIER_LIVE` (contracts/agent_tiers.hpp).

### The two modes
`definition` (default) writes the live mood's definition and lets the
mood apply re-run at the frame boundary; the edit survives a mood
change, and only for the LIVE mood — an edit to another mood's
definition waits until the program enters it. `preview` writes the
instance: immediate, and the next author may take it back. A dial
with no definition falls back to the instance under either mode. A
definition-only dial has no preview at all: its write is always a
definition, and the shell targets the live mood for it regardless of
the toggle.
O1a's contest readings come from PREVIEW writes only, because a
definition write never touches the instance and so asks the
instrument nothing.

Export keys a definition by mood AND id (`"<mood>/<id>"`) and an
instance value by id alone, so one file can carry several moods and
an import puts each back where it came from. A world definition
belongs to no mood, so it exports once, keyed `world/<id>`; import
skips witnesses and says how many.

### Open for ORGAN_2
Twelve of the sixteen enrolled dials had no definition target, and
they divided into two different problems. (Counted before ORGAN_2a.
All four bullets are closed; the enrollment stands at twenty-five —
eighteen dials, two definition-only, five witnesses.)

- THE FOUR ATMOSPHERE DIALS are DRIVEN, and the ruling above is their
  answer: they must not carry a dial on the value at all. What they
  need is a driver surface — rest value, gain, range on
  `phase_motion_drivers`' canvas pipe and on `tick_pawn_couplings`'
  presence ramp. Inventing a `MoodProfile` field for them would give
  a tier-3 parameter a tier-1 definition and lose the same argument
  one layer up. RESOLVED (ORGAN_2a): built as the drivers' room —
  contracts/driver_surface.hpp, block 3, rests and gains at the
  seams; the four driven values converted to read-only witnesses.
  The remaining three bullets were ORGAN_2b's.
- THE EIGHT AGENT-TIER DIALS have an author that is not a mood
  (`upload_agent_registries`, once at world init), so `MoodProfile`
  is the wrong place to reach for. They need a non-mood definition
  surface — a definition that belongs to the world rather than to the
  mood. RESOLVED (ORGAN_2b): the vocabulary graduated to
  contracts/agent_tiers.hpp; TIER_LIVE is the world's definition;
  ORGAN_DEF_TIER routes it; the frame boundary re-speaks
  upload_agent_registries_to_gpu.
- `clear_color` is the mirror case: a definition with no home on the
  panel's side, because `clearColor_` is not one of the three homes.
  RESOLVED (ORGAN_2b): a definition-only dial (block NONE) — the
  existing MOOD path writes it, apply_mood_lighting re-runs it, no
  instance exists and preview is refused.
- `config.sun_direction` beside `lighting.sun.direction` — two
  apparent homes for one fact, carried in from CHORD. It finds four
  homes, not two, and one of them unreachable from the other's room.
  RESOLVED BY RULING (ORGAN_2c): the pair dissolves under
  windows-not-homes — one definition, one instance of record
  (`sunDirection_`), one author, four windows; the A8a split is WHY the
  config window exists, and removal was priced (asymmetric mirror
  shrink, three witnesses) and refused. The one real defect was preview
  partiality on `sun.direction`, repaired by converting the dial to
  definition-only with a read-only witness on the instance. Evidence:
  docs/HANDOFFS/ORGAN_2c_RECON.md, consumed at c80b74d (the file's last
  commit — U0 records it).

## Definition kinds (ORGAN_2b)
A definition answers a question, and which question is the KIND.
`MOOD` answers "what does this mood mean" — one `MoodProfile` per
mood, and the write's target selects which. `TIER` answers "what does
this WORLD mean by its tiers" — one `AgentTierBank` (`TIER_LIVE`), so
the target is ignored by design rather than by oversight.
`definition_base` is the one place that mapping lives; a third family
costs an enum value and nothing else. The manifest's `def` column
carries the kind, so the shell can key an export by it without
learning any parameter's name.

A DEFINITION-ONLY ENTRY is the case where a fact has a definition the
panel may write and no instance the panel may address. Its block is
the sentinel `ORGAN_BLOCK_NONE`, `block_base` answers null, and
`organ_set` routes it straight to the definition path — preview is
refused, because there is nothing a preview could show. Reading it
reads the LIVE mood's definition, which is what the manifest's opening
values show. `clear_color` is the case that asked for it.

## The write path
A panel edit in preview mode writes the home struct member and marks
the block dirty; in definition mode it writes the mood definition and
raises one re-apply flag instead.
Dirty blocks flush once, at the frame boundary, through the block's
existing upload — the same reconciliation philosophy as ACQ_0 and
CAP_2: intent asserted per frame, not per event. A slider drag is
many events and one WriteBuffer.

## The contested-dial instrument (O1a)
A dial the panel CAN write is not yet a dial the panel OWNS. Some
homes have a second author — a mood apply, a per-frame updater — and
there the panel's word is not wrong, only temporary. Which dials
those are is DISCOVERED, never hand-censused: organ_set shadows the
bytes that land in the home, and once a frame, at the flush boundary,
the program re-reads the home and asks whether it still says them.
Never disagreed is FREE; stood a while and then lost it is EVENT;
lost it at once is PER-FRAME. The evidence behind a reading is the
survival count — the frames the panel's word stood. The instrument
reports and does not act: what a reading MEANS for the panel is a
separate ruling, and that ruling wants a census as its evidence.

### Staged or composed, and one dirty bit per home (O1d)
A panel write survives its upload only if the home is what the upload
SENDS. Censused, for the three enrolled homes:

| home | how it reaches the GPU | staged or composed |
| --- | --- | --- |
| `config_` | `upload_config` → `writeStruct(configBuffer_, config_)`; two targeted writes also read `config_` | STAGED — sent as it stands |
| `agentRoomStage_` | `upload_agent_registries`, the portal write, `organ_flush` — all write FROM the stage | STAGED — the sovereign CPU copy |
| `lightingStage_` | `upload_lights` (mood.hpp) COMPOSES a fresh `GPULighting` from the deps and hands it to `upload_lighting` | COMPOSED — and `upload_lighting` stores it back into the home, so the home always carries what the GPU last received |

Every writer of all three GPU blocks was checked: no site writes any of
them from anything but its home, and no site writes the frame-R lighting
region except `upload_lighting`. So no home is composed-at-upload WITHOUT
storing through the home, and the remedy `lightingStage_` carries is the
only one needed.

ONE DIRTY BIT PER HOME. `config_` already had one — `configDirty_`, the
flag `upload_config` tests and the spine's own per-frame upload consumes.
ORGAN raises that flag rather than keeping a second beside it, and
`organ_flush` writes nothing for config: a second upload there would be a
second writer for one fact with nothing new to say. The routing lives in
`organ_mark_dirty`, in the home, because which flag a home uses is the
home's knowledge and not the panel's. `organTouched_` is named for what
it is — for lighting and the agents' room it IS the dirty bit, and for
config it is only the witness's record. The panel's status line says
`reconciled` for the same reason: it counts what the panel caused, not
which function did the writing.

A CONFIG dial that loses its value therefore loses it to a CPU AUTHOR, not
to the upload. `fog_density` / `fog_color` are rewritten every frame by
`phase_motion_drivers` from the visual canvas; `aura_enabled` /
`pawn_aura_height` every frame by `tick_pawn_couplings`. Nothing in the
write path can fix that, and nothing should try: by the ORGAN_2 ruling
these are tier-3 parameters wearing tier-1 dials, and the repair is a
driver surface, not a definition for the value. ORGAN_2a built it —
see "The drivers' room" above; all four now enroll as witnesses, and
the dials moved to the seams that drive them.

## Access
The panel exists only under `?organ=1`; backtick toggles visibility.
Without the flag, no DOM is built, no export is called, the audience
path is byte-identical. The panel is an instrument, not the art.

## The disposition (ORGAN_3)
An organ's DISPOSITION is the document naming every stop the instrument
has. `docs/HANDOFFS/DISPOSITION_LEDGER.md` is ours. A coupling is a
parameter set into trajectory over time, so every enrolled dial — with
its authored (min, max) — is a trajectory domain the music campaign will
play. The panel is that campaign's target map, not decoration.

### The five classes
Every design parameter falls in exactly one.

| class | signature | recipe |
| --- | --- | --- |
| **C1 LIVE** | reachable home, no runtime author after boot (boot pins allowed) | one `ORGAN_PARAM` line |
| **C2 GRADUATE** | authored constexpr in a module, no live home | design table stays; LIVE bank in contracts; readers move; enroll against the bank |
| **C3 EVENT** | a named author re-speaks it at an event | LIVE bank + a definition kind, then the temperament law below |
| **C4 DRIVEN** | a per-frame author writes it | rests and gains at the seam, `_RO` witnesses on the driven values |
| **C5 BOOT** | structural — world generation reads it, or it is a compile-time law | never enrolled live; recorded in the ledger with one line of reason |

Shapes: floats and float runs are F32/VEC3/VEC4; a C++ `bool` cannot
enroll (one byte against the ABI's four) — a CPU-only struct being
graduated takes `uint32_t` from birth, a GPU-mirrored struct is never
relaid out for a dial; enums and index choices are U32 with their
meanings named in the line's comment; tables of ≤8 rows enroll per-row,
larger ones stay C5 with the composite editor's price named; RNG salts
are addresses, not dials, always C5.

### The temperament law
An **idempotent** author — re-speaking it reproduces the same world
state from its definitions (the mood apply, the registry upload) —
re-speaks at the frame boundary when its bank changes. A **destructive**
author — respawn, world generation; re-speaking tears down live state —
does **not** get a boundary re-speak: its definitions take effect at the
author's next natural event, exactly as a non-live mood's definition
already waits. Such a bank's group banner says so — *"edits the next
spawn"* — because a dial that edits the future must say it edits the
future, or the operator reads a working panel as a broken one.

Where one fact has both temperaments, the **stricter governs**.
`INDOOR_HEIGHT_CAP_FRACTION` is the exemplar: one idempotent reader
(`apply_mood_lighting`) and nine destructive ones (`cap_to_ceiling` at
every spawn), so its bank has no boundary wiring at all.

A KIND IS NOT A FLAG. Two kinds share a flag when they share an AUTHOR,
because the flag names the occasion and the occasion is the author
speaking. `BEHAVIOR` raises `TIER`'s flag for exactly that reason.

### The sections
The group string is a path: `"Section · Group"`. The shell splits on the
FIRST separator — two levels, never three — and renders each section as a
collapsible block. Sections are the program's VOICES, the operator's
taxonomy rather than the homes': `Sky & Light`, `Atmosphere`, `Terrain`,
`Pawn`, `Ribbon`, `Agents`, `Interaction`, `Debug`.

A dial sits under its SUBJECT, never under its implementation home: the
fog's rests live under `Atmosphere` beside their witnesses, not under a
"Drivers" section, because implementation homes are not menus.

**There is deliberately no Camera section.** Camera pose is GPU truth,
and its absence is the sovereignty boundary made visible in the one place
an operator would look for it. The camera's *controls* — sensitivity,
step, zoom — are input grammar and live under `Interaction · Camera`.

A section must be CONTIGUOUS in registry order, since the shell opens a
new block whenever the first token changes; the harness asserts it,
because the `.inc`'s order is the panel's table of contents.

### The gap tool
`tools/organ_gap.py` — check-family, stdout only, **exit 0 always**. It
parses the enrollment list, brace-parses the enrolled home structs, and
prints every declared member the panel does not name. A map, not a gate:
a member absent from the panel is usually absent on purpose, and the
reason lives in the ledger. Three blind spots, printed on every run:
it cannot see homeless constants (the ledger's job, and the larger gap);
it trusts its own file table (so it prints it); and it reports at the
granularity the enrollment addresses, so a partly-enrolled nested
aggregate reads as named.

### The tally at close
223 enrolled entries — 210 dials and 13 read-only witnesses — across
eight sections and nine blocks.

| section | entries |
| --- | --- |
| Agents | 102 |
| Terrain | 39 |
| Interaction | 19 |
| Atmosphere | 18 |
| Pawn | 18 |
| Ribbon | 14 |
| Sky & Light | 9 |
| Debug | 4 |

Definition kinds: 116 none, 5 MOOD, 32 TIER, 70 BEHAVIOR; two of them
definition-only. Blocks 0-8: config, lighting, agent room, drivers, pawn
aura, orb console, the panel, the ribbon, indoor — nine of the twelve the
campaign set as its consolidation threshold.


## ORGAN_2 — the close (the campaign minute)
SETTLED. Four findings, four fates: the atmosphere dials got the
drivers' room (2a — rests and gains at the seams; driven values became
witnesses); the tier dials got the world's definition (2b — the
vocabulary graduated, TIER_LIVE, the boundary re-speaks the author);
clear_color got the definition-only entry (2b); the sun mirror
DISSOLVED by ruling (2c — windows, not homes; the config window is the
compute face's only eye on the sun and stays). The enrollment stands
at twenty-five; adding a dial is one line in one file, which was the
point.

RESIDUES, recorded not repaired: the dead aura boot seed
(state.hpp's `config_.aura_enabled = 1.0f`, overwritten within a
frame by the tick — OPEN's docket); gain-1 exactness at the fog seam
is Sterbenz-bounded (bit-exact across the authored range, numerically
indistinguishable beyond it; known, unguarded); exports cut before 2c
carry `<mood>/LIGHTING.sun.direction` keys that now import as a
WITNESS SKIP — the id is still enrolled, only no longer writable, so
the note reads "skipped 5 witnesses" and the reject counter never
moves; the photographer window refreshes
at its own fill site, so a definition edit while the photographer is
live lands there on its next fill, not this frame.

PRICED AND UNSPENT: removing `config.sun_direction` (refused — the
bytes work); Firefox regain (held at OPEN); tiers 2 and 3 — the
tremulant and the keys — stay reserved columns in this registry until
the music-coupling campaign claims them beside the VOICE bus.
