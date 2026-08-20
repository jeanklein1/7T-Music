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
Tier 1 is live. One registry, three tiers, never three systems — but
the columns for tiers 2 and 3 arrive with the campaign that fills them,
not six campaigns early. ORGAN_0 reserved them and ORGAN_6 CODA spent
them: both sat at zero on every row for six campaigns, and a placeholder
cannot tell the music campaign what shape the column it needs actually
has. The charter's argument is about WHERE a coupling column belongs,
which is here; it is not an argument for shipping an empty one.
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
read the room each tick are its flush. Since ATMOS_1 the fog's REST is
the mood's, and since ATMOS_2 one column of the drawn REGIME
(`Regime.fog_density` / `.fog_color`, drawn per world into
`MoodState.fog_rest_*`); the room keeps the gain, the canvas emits a
deviation from its anchor row, and the seam reads
`rest + gain · deviation`.

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
| frustum cull, GoL gate, aura policy | `shape.allow_*` | renderer / GoL / pawn |
| sun direction, colour, intensity, ambient | `atmos` (through `draw_atmosphere`) | `apply_mood_lighting` → the deps → `upload_lights` → `GPULighting` |
| clear colour | `atmos.regime[r].clear_color` | `clearColor_` |
| fog rest | `atmos.regime[r].fog_*` | `mood_state_.fog_rest_*` → the U4 seam |
| terrain amp ceiling, indoor height cap | `shape.terrain_amp_ceiling`, `shape.wall_height` | GPU config |
| spot lights | `shape.indoor`, `shape.wall_height`, `shape.ceiling_type` | the spot array |
| indoor shell, camera ceiling | `shape.indoor`, `shape.ceiling_type`, `shape.wall_height` | the shell + the clamp |
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
1. DEFINITION — `MOOD_LIVE[mood].atmos` — the distribution;
   `draw_atmosphere(seed, ·)` turns it into layer 2 at every apply.
   What the mood means.
2. INSTANCE OF RECORD — `sunDirection_` / `sunColor_` /
   `mood_state_.sun_*` / `mood_state_.regime`, written by
   `apply_mood_lighting`.
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
ONLY runtime reader. Since ATMOS_1 the split is the type's own: the
whole of `Atmosphere` — the sun's bearing and the regimes — passes,
because `apply_mood_lighting`
reads it in one place, through `draw_atmosphere`. The whole of
`WorldShape` — `finite`, the radii, `indoor`, `ceiling_type`,
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
`BEHAVIOR` (ORGAN_3) answers "what does a behaviour DO" — one
`AgentBehaviorBank`, target ignored, same as `TIER`. `ORB_MOOD`
(ORGAN_3b) answers "what is this mood's sky" — one `OrbMoodConfig` per
mood, so like `MOOD` and unlike the other two it is **mood-selected**
and the write's target picks the row.
`definition_base` is the one place that mapping lives; a further family
costs an enum value and nothing else. Five kinds exist: `NONE`, `MOOD`,
`TIER`, `BEHAVIOR` and `ORB_MOOD` — and any prose that stops at the
fourth predates ORGAN_3b. The manifest's `def` column carries the kind,
so the shell can key an export by it without learning any parameter's
name; what the shell actually needs is not the family but its SCOPE, and
since ORGAN_6 the registry derives that and emits it. The shell asks the
question by reading a column, which is once by construction rather than
once by discipline — see *The manifest answers* below.

A DEFINITION-ONLY ENTRY is the case where a fact has a definition the
panel may write and no instance the panel may address. Its block is
a sentinel, `block_base` answers null, and
`organ_set` routes it straight to the definition path — preview is
refused, because there is nothing a preview could show. Reading it
reads the LIVE mood's definition, which is what the manifest's opening
values show. `clear_color` is the case that asked for it.

**ONE SENTINEL PER FAMILY, DERIVED FROM THE KIND.** The block/offset/type
triple identifies an entry, so two def-only families sharing a sentinel
collide the moment their offsets meet — which is exactly what happened at
ORGAN_3b when `MoodProfile.clear_color` and `OrbMoodConfig.rotation_axis`
both claimed `(255, 32, VEC3)`. `ORGAN_PARAM_DEFONLY` now pastes
`ORGAN_DEFONLY_BLOCK_##DEFKIND`, so the sentinel follows from the kind and
a new family cannot land on a used one without adding its own line.
The convention descends: **255** MoodProfile, **254** OrbMoodConfig, a
third family takes 253. `is_defonly()` answers for all of them.

**THE DEFINITION PATH CONVERTS; IT NEVER REINTERPRETS.** ORGAN_1 refused
`U32` and `BOOL` on the definition path, stating the reason as
reinterpretation — writing a float's bits into an integer field. But
`organ_set`'s instance path has *converted* since ORGAN_0, and conversion
is not reinterpretation. ORGAN_3b taught both definition paths the same
conversion; the rule that survives is **never reinterpret**, which is what
was actually being defended.

## The manifest answers (ORGAN_6)
> *A rule restated in a second language is a rule with two homes.*

`derived_cadence()` exists for exactly one reason: the shell must not
restate a rule the C++ owns, so the C++ derives `cad` and emits it. Two
rules were left un-derived and the shell restated both in JavaScript —
**which blocks are def-only sentinels**, and **which definition kinds are
world-scoped**. The first drifted the moment ORGAN_3b P3 minted a second
sentinel (254) beside the first (255): `web/organ_panel.js` kept one
number, so every one of the nineteen `ORB_MOOD` rows read as an ordinary
block, and in PREVIEW mode `push()` sent `target = -1`, which `organ_set`
refuses on a definition-only row. **Nineteen dials died silently the
moment the operator pressed `preview` and came back when he pressed
`definition`** — for two campaigns, behind a live reject counter.

Two derived columns close it, beside `derived_cadence()` and for its
reason:

| column | question | values |
| --- | --- | --- |
| `inst` | may the panel address this row's INSTANCE? | `1` yes · `0` definition only |
| `scope` | how is its DEFINITION addressed? | `0` none · `1` mood-selected · `2` the world's |

`def` STAYS: it is the FACT — which family — and `scope`/`inst` are the
DERIVATIONS. A manifest carries both, exactly as it carries `block` and
`offset` beside `cad`.

**The shell holds no C++ number.** Not a block sentinel, not a def-kind.
A sixth definition family answers in `derived_scope()` and the shell
learns nothing. `push()` also stated one rule twice — it chose its target
from the mode and then marked the contest from the mode AGAIN, and the two
disagreed for a definition-only row under preview, claiming a contest
reading for a write that had just been refused.

**And the first fix was wrong, which the CODA says here rather than
elsewhere.** P2 replaced the mark with `target < 0` and called that the
instance write; it is not, because a row with no definition in definition
mode carries a mood target, `write_definition` refuses it on
`ORGAN_DEF_NONE`, and the write falls through to the instance — 172
writable rows stopped being asked a question the contest instrument was
still answering. The predicate is now `!(target >= 0 && p.scope)`, which
is `organ_set`'s own early return transcribed rather than a second rule
that happens to agree with it: `p.scope > 0` holds exactly when
`write_definition` succeeds, so the two cannot disagree because one IS the
other.

### The two mood-selected families keep one discipline
`MOOD` and `ORB_MOOD` are both mood-selected: the write's target picks the
row. `MOOD` has recorded WHICH mood since O1b, and the boundary drops a
stale write with it, because populating this world from one we are not in
is not a preview, it is a wrong answer. `ORB_MOOD` recorded nothing and
re-spoke the live mood regardless — so a write aimed at a dormant mood
landed in its own row and then made the boundary re-speak a *different*
one: the edit lost with the reject counter untouched, and its touched bits
spent re-seeding a sky the edit was never about.
`g_orb_def_dirty_mood` is the slot it was missing, and
`raise_orb_definition(mood)` is now the only way to raise the flag. The
touched mask is taken BEFORE the mood test and unconditionally: leaving
bits standing would let a dormant-mood edit reseed the live sky one frame
later, which is the same defect through a slower door.

### A rejection gets a name
The panel WAS reporting the sentinel defect the whole time. It printed
`rejected 19`, and the number carried no information — not which row, not
why. **A count is not a diagnosis.** `g_last_reject` is one string written
at every refusal site in `organ_set` and read by the status line beside
the count; `++g_rejected` appears exactly once in the tree, inside
`note_reject`, so a fifth refusal site cannot forget to say why.

### The dome says whether it is lit
`configure_orbs` early-returns on `!os.active || os.count == 0`, so with
`enabled 0` or `count 0` every other orb dial is inert and the panel said
nothing — the operator drags a cohesion radius against a dark sky and
reads it as a broken panel. That is the defect the rule readout (ORGAN_5
P2b) and the regime readout (ATMOS_1b) each answer, in a third place that
never got one. The rule window WIDENS rather than a second window being
minted — its own argument, extended: `active` takes bit 16 and `count`
bits 17..25 of the same `uint32`, one ABI call, nothing to keep in step.
The line rides beside the rule readout under the door bar, because
deriving WHICH groups are orb groups would be a fourth name-string in a
file whose whole argument is that it holds none.

### The shell gets a standing witness
`tools/gates/shell_gate/run.py` (L33). The C++ organ is proved by
`organ_gap`, `organ_readers`, `organ_ledger`, `console_gate` and a native
harness; the shell was proved by a shim rebuilt each campaign and thrown
away with it, so nothing in the tree could have caught the sentinel drift
and nothing would have caught its successor. Six checks, both sides parsed
from the tree, no browser and no node: the shell names no block sentinel;
the type enum, the cadence table's length, the scope constants and
`RULE_NAMES` agree with the C++ they copy; and every `M.cwrap` reaches a
real `EMSCRIPTEN_KEEPALIVE` export at the matching arity — the last of
which catches a change made on one side of the ABI only.

It reads TEXT, not behaviour: it cannot see a value passed wrongly, only a
name or an arity that disagrees, and it proves the SEAM rather than the
shell. The web boot remains the witness of record for everything past it.

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
has, and every one it does not. `audit/ORGAN.md` is the first — generated
per commit from the compiled registry — and *What has no dial, and why*
below is the second. ORGAN_3's own working ledger was the survey that
produced both; it is spent and retired to git (L31, ORGAN_6 CODA C3). A
coupling is a parameter set into trajectory over time, so every enrolled
dial — with its authored (min, max) — is a trajectory domain the music
campaign will play. The panel is that campaign's target map, not
decoration.

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

Where one fact has both temperaments, the **stricter governs** — a bank
with one idempotent reader and one destructive reader gets no boundary
wiring at all, because a re-speak that is safe for one and ruinous for
the other is not safe.
`INDOOR_HEIGHT_CAP_FRACTION` is the exemplar: one idempotent reader
(`apply_mood_lighting`) and nine destructive ones (`cap_to_ceiling` at
every spawn), so its bank has no boundary wiring at all.

A KIND IS NOT A FLAG. Two kinds share a flag when they share an AUTHOR,
because the flag names the occasion and the occasion is the author
speaking. `BEHAVIOR` raises `TIER`'s flag for exactly that reason.

### Graduation completeness (ORGAN_3c)
**A graduation is complete when the design table's only remaining readers
are its seed and its asserts.** A bank that is built, enrolled and left
unread is worse than no bank: the panel offers a dial, the dial writes, the
write lands, and the world does not move — which reads as a broken
instrument rather than as an unfinished one. ORGAN_3 w2 shipped exactly
that, seven times over, when it built `PANEL_LIVE` and left the beacon and
camera readers on the constexprs; it took a campaign and a human sweep to
find. `organ_gap.py --gate` is the standing witness that it stays fixed —
it classifies every mention of each design symbol and fails on any that is
not a definition, a seed, a `static_assert`, prose, or a **compile-time
derivation** (D7's case: a `constexpr` initialiser cannot read a mutable
bank at all, so its source rightly stays on the design table). The map
around it is still toothless; only the gate can fail, and only on this.

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
`tools/organ_gap.py` — check-family, stdout only, **exit 0 always** unless
`--gate` is passed (ORGAN_3c: the reader witness above is the one thing
here that can fail). It
parses the enrollment list, brace-parses the enrolled home structs, and
prints every declared member the panel does not name. A map, not a gate:
a member absent from the panel is usually absent on purpose, and the
reason lives in the ledger. Three blind spots, printed on every run:
it cannot see homeless constants (the ledger's job, and the larger gap);
it trusts its own file table (so it prints it); and it reports at the
granularity the enrollment addresses, so a partly-enrolled nested
aggregate reads as named.

## Cadence (ORGAN_3b)
> *A dial that edits the future must say so where the hand is.*
> — Jean's first sweep, which found the one defect the disposition
> inherited rather than created: a generational dial read as a dead dial,
> because the panel did not yet say WHEN a stop sounds.

Four cadences, answering *when does my edit land*:

| cadence | chip | meaning |
| --- | --- | --- |
| `LIVE` | *(silent)* | the write is the change |
| `GEN` | `on respawn` | the author's next natural event applies it — a spawn, a world init |
| `BOUNDARY` | `boundary` | a re-speak at the frame boundary applies it; it lands within a frame |
| `DRIVEN` | `driven` | a per-frame author writes this; the row is a meter, not a dial |

**Only `GEN` is stored.** `derived_cadence()` answers the other three from
facts the entry already carries — a witness is `DRIVEN`, a definition or a
block with a boundary re-speak is `BOUNDARY`, everything else is `LIVE` or
the stored `GEN`. A cadence column that could disagree with the ro flag or
the def kind would be a fourth place to forget something; there are three
already.

The chip rides beside the contest marker, because the two answer the
operator's two questions about one row: WHEN does my edit land, and DOES
it still stand. `LIVE` is silent by design — the common case earns no ink.

## Doors
A door is the panel pressing **the program's own machinery**: it raises a
flag the frame boundary already consumes, and adds no author the program
did not have. That is the whole sovereignty argument applied to verbs
rather than to values — the panel may ask the program to do a thing it
already does, and may not do that thing itself.

Door 0, **Re-speak definitions**, raises every definition flag at once, so
an edit made while its bank was quiet lands on the click rather than on
the next mood change. It writes no home and no dial; the harness proves
both. Presses coalesce in a `uint32_t` bitmask, so a double-click is one
raise, and the roster is read from the program (`organ_doors()`) rather
than named in the shell — the shell stays name-blind about doors exactly
as it is about dials.

**The mood door** (`organ_go_mood`, ATMOS_1) is a door with a parameter:
the shell's select, built from `organ_mood_names()`, asks the program to
enter a mood by id; the boundary walks `request_mood_transition`, which
keeps its own guards. Editing a non-live mood's definition rows requires
being in it; this is the road.

**Molt is priced, not built** — and it no longer has a number, because
ORGAN_4 spent ids 1 and 2 on the sky's two player-owned facts (see *The
reader answers* below). It was written here as "door 1" when 1 was the
next free id; the id is a slot, not a claim, and the price is what
survives. `NEEDS_REGEN` re-runs the
heightfield only; `request_recenter` re-evaluates the window only;
`evict_patch` is a loop. The bill is the risk, not the lines: evicting the
point's own patch, a budgeted respawn, and the free-layer stack. D3's
threshold — one existing authored function plus ≤ ~20 lines of glue using
deps the cartridge already holds — is not met, so it is recorded as a
price.

## Navigation (ORGAN_3b P4)
The manifest is a library, not a page. The panel opens **collapsed**, as a
table of contents. One filter field matches `id + label + group`
lowercased — the three names a stop already answers to — hiding rows,
then group headers with no visible row, then sections with none either;
a filtered section's tally reads `hits/total`. The filter opens what it
finds, but an auto-open is not a choice: only what the hand opened is
remembered, and clearing the filter returns the desk to that. Session
only, no storage.

Each section header carries its own export — the same walk narrowed by a
predicate, so witnesses stay skipped and the `world/` and `<mood>/` keying
is identical. Import needed nothing: a partial file has always applied
exactly what it carries. **A voice is a file.**

### The row grid, and the width (ORGAN_3c)
A row is two lines, both grids, every cell placed by explicit
`grid-column` rather than auto-placement — so a row with no colour swatch
and no cadence chip still lines its markers up with the row above it, and
the rows read as a column rather than as a list.

```
line 1  [ label ……………………………  sw   mk   chip ]
line 2  [ slider ————————————————— | value ]
```

The label is the only cell that gives; what its ellipsis hides it hands to
the `title`, label and id both. VEC lanes stack full-width. Witnesses keep
their meter in the value column, so a meter reads down the same edge as a
dial. The chip moved up to the label line and never competes with a
control again.

**Every fixed width is a CSS custom property on `#organ`, and the resize
minimum is computed from the same numbers** — 292px, line 1 governing;
`MAX = min(640px, 50vw)`. A hardcoded minimum is a guess, and a guess is
how overlap comes back. The grip rides the panel's inner edge; double-click
is home.

A hand-set width is a **choice**, so it lives beside `openMap` and obeys
the same law: session only, never storage. The filter may open a section it
found and the grid may squeeze a slider toward its floor, but neither
rewrites what the hand set.

## The tally at ATMOS_2's close
365 enrolled entries — 348 dials and 17 read-only witnesses — across
eight sections and twelve blocks. Re-cut at ATMOS_2's close from the
generated book, as every campaign that moves the count re-cuts it at its
end.

THIS TABLE IS NO LONGER THE AUTHORITY. `audit/ORGAN.md` is generated from
the enrollment list on demand (ORGAN_4 P5) and carries every row with its
range, its step and its cadence. What stays here is the shape, because a
shape is a thing to argue with and a table of 365 rows is not.

| section | entries |
| --- | --- |
| Agents | 109 |
| Atmosphere | 73 |
| Ribbon | 56 |
| Terrain | 42 |
| Sky & Light | 41 |
| Interaction | 22 |
| Pawn | 18 |
| Debug | 4 |

By cadence: 122 live, 46 gen, 180 boundary, 17 driven.

Definition kinds: 189 none, 55 MOOD, 32 TIER, 70 BEHAVIOR, 19 ORB_MOOD;
74 of them definition-only — 55 under sentinel 255 (MoodProfile) and 19
under 254 (OrbMoodConfig). Blocks 0-11: config, lighting, agent room,
drivers, pawn aura, orb console, the panel, the ribbon, indoor, canvas,
the world draw, the ribbon's spawn bank — the twelve the campaign set as
its consolidation threshold, now all in use.


## The reader answers (ORGAN_4)

**An enrollment states a belief; only the reader proves it.** Every row
ORGAN_4 touched ends the campaign either consumed by its reader or retired
with its reason ledgered.

That law exists because of a defect the panel could not see. Jean's sweep
of the orbs found eight of the sky's twenty-seven rows dead and thirteen
more sitting on a sentinel — every one shipped through a green harness,
because **the manifest's chip derives from the ENROLLMENT'S FORM while the
truth lives in the READER'S BODY.** A registry that cannot drift about
offsets can still be entirely wrong about consumption.

### The console-mask idiom

A CPU BANK WHOSE READER IS AN EVENT GETS A PER-FIELD MASK, consumed at the
frame boundary, and the boundary routes each field to the cadence its own
reader has.

`ORB_CONSOLE_LIVE`'s only reader is `configure_orbs` — an event — so the
three Dome dials were dead until a mood change. ORGAN_3b cured the silence
with a block-wide re-speak flag, which cured it by firing the whole
applier; `configure_orbs` ends `os.init_pending = true`, so every notch of
the dome slider **re-seeded the sky**. Right about WHEN, wrong about WHAT.

```cpp
inline uint32_t g_orb_console_dirty = 0;   // bit = offsetof/4
```

The raise costs one shift at a site that already had the offset in hand —
`organ_set`'s single post-clamp statement, keyed on the block (D1). A
`static_assert` beside the flag pins the three offsets, so a field
reordered in the struct fails the BUILD rather than routing a radius into
a noise floor. `block_has_boundary` stays and keeps its meaning: all three
fields still land at the boundary, so all three still read BOUNDARY
cadence. **A cadence question and a plumbing question are two questions.**

### The three doors

A door is the panel pressing the program's OWN machinery. ORGAN_3b built
one; ORGAN_4 added two, and they are what a PLAYER-OWNED fact's enrollment
looks like when the panel tells the truth:

| id | label | what it presses |
| --- | --- | --- |
| 0 | Re-speak definitions | every definition flag at once |
| 1 | Cycle orb rule | `cycle_orb_motion_rule` — the same function KP_8 presses |
| 2 | Cycle orb gesture | `cycle_orb_gesture` — the same function KP_7 presses |

`OrbMoodConfig.motion_rule` had a dial and no reader: `configure_orbs`
writes `os.current_motion_rule`, the player's, and never looks at the
mood's. That hardcode is a RULING — the rule is player-owned, seeded once
to Brownian — so the dial died and the ruling's reachable form is the
door. `flock_gesture_default` is the same shape for a different reason: a
BOOT-ONLY fact wearing a boundary chip misreports, because the applier
refuses it on every run after the first.

Zero JS edits were needed. The shell renders one button per manifest row,
name-blind, exactly as it does for dials.

### The thirteen floors — D1(d)'s second application

THE TIERSET PRECEDENT, APPLIED VERBATIM: **a slider cannot express a
sentinel without lying.** `configure_orbs` reads 0 as "no opinion"
(`eff()`) for `drag`, `orbital_base_speed` and the seven `flock_*`, and
`passthrough()` maps the four `rule_drag_*` zeros to 1.0×. Thirteen rows
whose minimum was 0 therefore had a value the applier discards. Each min
is now **exactly one step** of its own row (D4).

One step and not less, and the arithmetic is the argument: with min 0.01
against a step of 0.02 a dial reaches 0.99 and 1.01 and **never 1.0** —
the pass-through identity itself. Half a step puts every gridline off the
integers. At one step the authored mood value stays on the grid
`min + k·step` for all thirteen, which the harness proves rather than
assumes.

### The palette temperament

Stamped above `pack_palette_`, because the same function answers two
questions:

> palette is config-owned — the dial and the mood are the durable authors
> and `pack_palette_` re-speaks them; the KP palette-cycle is a live
> gesture that lasts until the next configure. Rule and gesture are the
> OPPOSITE (player-owned, seed once): that asymmetry is deliberate — a
> dial exists for `palette_id`, and a config the applier ignores is a dead
> dial.

Player-protecting the palette field would have killed the `palette id`
dial the exact way the rule dial died. And the same function is why
`base_hue` and `hue_variance` are not dials: every `ORB_PALETTES` row
carries `count ≥ 1`, so `palette_count` is never 0 and the kernel's legacy
single-hue arm is unreachable. Two fields the applier faithfully copies
into a branch the GPU cannot take.

### `tools/organ_readers.py` — does your reader name you?

The audit family's newest check and the `organ_gap` sibling: stdout only,
**exit 0 always**. `organ_gap` measures the gap between the HOMES and the
panel; this measures the inward gap — of the dials that DO exist, which
write a field nobody reads.

**The match is HANDLE-QUALIFIED, and that is the whole tool.**
`configure_orbs` CONTAINS the token `motion_rule`, in
`gpuCfg.motion_rule = os.current_motion_rule;`. A bare-token match would
have passed the deadest row in the tree. A row is proved only when its
leaf is reached through a name that IS the bank — the LIVE symbol, a
parameter of the bank's struct type, or a reference alias bound to either.

Its blind spots, stated in its own header rather than discovered later:
helper indirection and whole-struct copies produce false POSITIVES; a
colliding leaf token produces a false NEGATIVE, which is worse and is why
the match had to be handle-qualified; **GPU-side consumption is out of
scope** — that is the kernel's ledger, a sixth instrument, and the one
that could have caught `base_hue` mechanically instead of by Jean's eye.
A witness is skipped because its question is inverted: a meter asks who
AUTHORS, which is the contest instrument's job.

### `audit/ORGAN.md` — the fifth ledger

BINDING, COMMAND, MANIFEST and MIRROR each keep a generated book about one
of the program's rooms. `tools/organ_ledger.py` writes the panel's: every
row with its section, label, id, block, type, range, step, cadence,
def-kind and `ro`, then the tallies, then the verbatim tails of both check
tools.

**Every row with an authored range is a trajectory domain.** A coupling is
a parameter set into trajectory over time, so this table is the music
campaign's target map — the range column is the domain a trajectory would
play over, and the cadence column says whether playing it would be heard
now, at the boundary, or at the author's next event.

`derived_cadence()` is restated once in the generator, which is one more
copy of a rule than the compiled-registry law likes — so the harness
prints the same tallies from the COMPILED table and the two are compared
rather than trusted.

### The preset layer (`web/presets/`)

**A scene is a file, a boot is a choice.** `index.json` is the shelf,
`?preset=<name>` picks one at boot, a select in the panel header picks one
by hand. All three walk the SAME import path: a partial file applies
exactly what it carries, an unknown id is counted rather than thrown, and
no new write machinery exists.

THE LOADER LIVES AT MODULE SCOPE, NOT INSIDE `build()`, and that is the
whole point: an exhibition boots with `?preset=` and no panel. A preset
reachable only from an open panel would be an instrument feature wearing
an exhibition's name. The audience path keeps its promise exactly — with
neither flag the file still returns on its first statement.

No storage of any kind, the same law the width and the `openMap` follow:
the URL is the only thing that carries a choice between sessions.

> **For Jean.** Design a scene on the panel. Press export (or a section's
> own export for one voice). Drop the JSON in `web/presets/`, add one line
> to `web/presets/index.json`, and São Paulo boots into it with
> `?preset=<name>`.


## What has no dial, and why (ORGAN_3–6, closed)
An enrollment states a belief and only the reader proves it; the converse
also has a register. Four facts survived every wave of the disposition
survey without earning a row, and each names its own owner rather than
waiting on a campaign.

| what | why not a dial | its owner |
| --- | --- | --- |
| `THEME_BASE_WEIGHT` | it is one scalar over a 5×N weight construction; the honest dial is the whole `MOOD_SPAWN_MULT` matrix, D5-large | a composite editor (D5) |
| `INDOOR_PALETTES[]` | mixed-shape rows, count read from `INDOOR_PALETTE_COUNT` (D5) | a composite editor (D5) |
| `tierset_id` | its "none" value is `0xFFFFFFFF`, and a 0…1 slider cannot express a sentinel without lying (D1(d)). `organ_gap` reports it, by name, as the one absent member of `OrbMoodConfig` | a composite editor (D5) |
| `mute_couplings` | a bitmask: `Coupling::ALL` is `0x1FFFFF` and a slider from 0 to 2 097 151 is not a dial. It wants a checkbox per bit — a shell feature, not an enrollment line — and ORGAN_3c found **the bits are not there to check**: 21 bits wide, 8 of them named, so twenty-one checkboxes would be thirteen toggles over bits nothing reads. `Coupling::` is also a hand-kept mirror of `world.wgsl`'s `COUPLING_*` block, which D1's third branch reserves | a checkbox grid over a roster the C++ emits — priced, unbuilt, and **not CC's to choose** |

`D5` is the composite-editor deferral: a fact whose honest control is a
grid rather than a slider, priced rather than promised. `D1(d)` is the
range deferral: a value whose domain a slider cannot state without lying.

None is externally blocked, which is why none of them is in
`docs/OPEN.md`: OPEN is the register of open STATE, and a deferral any
sitting can lift by reading a module is unfinished survey, not open state.
The survey that produced them — ORGAN_3's disposition ledger — is spent
and retired to git (L31); this table is what outlived it.

**And one thing the next sitting should settle before it builds the
strip.** `Coupling::` masks the PROGRAM's couplings — terrain→pawn (y,
tilt), pawn→camera, the three input couplings, terrain→sphere-height and
pawn→sun-VP. There is no fog bit. The music campaign's couplings are a
different vocabulary that does not exist yet, and which one the strip is
for changes what the strip is.

## The rule made visible (ORGAN_5)

Three laws, and each answers a defect Jean found by playing the built
panel rather than by reading it:

> **A dial whose effect depends on a mode stands next to a truthful
> readout of that mode.**
> **An author re-speaks no more than the edit requires.**
> **A section is organized for the hand that plays it, not the struct
> that stores it.**

### The touched mask — the console idiom, one level up

ORGAN_4 gave a per-field mask to a bank whose reader is an EVENT. ORGAN_5
gives one to a bank that already HAD a flag, and the division of labour is
the whole idea:

> **The FLAG says THAT the bank changed. The MASK says WHAT. The BOUNDARY
> decides HOW MUCH re-speak the edit requires.**

`configure_orbs` ends `os.init_pending = true`, so before this every notch
of every orb-mood dial re-seeded the sky: dragging a flock radius
destroyed the flock it was steering. Now `g_orb_def_touched` records
`offsetof/4` at the one site that already raises the flag, and the
boundary tests it against `ORB_RESEED_BITS` — the four facts the init
kernel bakes into `orb_state` (`enabled`, `count`, `palette_id`, `drag`).
The other fifteen are per-frame GPU reads: the uniform upload alone
carries them and velocities persist under the finger.

**A raise with NO bits means everything.** That is the RESPEAK door, which
promises exactly that, and it is also the safe answer for a future caller
that did not say — a caller that says nothing must not be handed the light
path by default.

**The classification lives with the mask, not with the boundary.** The bit
convention is defined in the registry and nowhere else, so the constant
that INTERPRETS bits belongs beside the constant that PRODUCES them. Two
`static_assert`s pin it: the literal `0x1023`, and that the struct's word
count still fits a `uint32`.

**A mask that indexes one struct cannot speak for another.** `base_size`
is an `OrbConsole` field that is init-baked, so it raises the orb flag
while contributing no bit to a mask over `OrbMoodConfig`. The console
block therefore carries its own heavy reason down in one local — without
it a same-frame flock drag would have supplied a light bit and swallowed
the size edit.

### The rule window

`organ_orb_rule()` returns `rule | gesture << 8` — a WINDOW onto
`OrbsState`, which stays the only home (CHORD). `organ_mood()` and
`organ_light_tier()` borrow ONE POINTER to the spine's mood organ
(ATMOS_1b); the rule cannot, and the difference is a
tier: the mood lives in a CONTRACT the registry includes, the rule in a
BODY the organ may not. **Same law, different plumbing, because the home
is one tier further away.**

**One writer, one site, and not the event.** The obvious placement is
beside the door handlers — and it would be stale for exactly the path the
defect was found on, because `cycle_orb_motion_rule` has two callers: the
door and `KP_8`. The window is refreshed at the frame boundary from the
one home, which cannot go stale whoever turned the rule.

### By the hand, not by the struct

The orb rows were grouped by HOME — three under "Dome" because they live
in `OrbConsole`, nineteen under "Orb mood"/"Orb flock" because they live
in `OrbMoodConfig`. Brownian's strength dial therefore sat in a geometry
group under the name *"noise floor"*, and fifteen rule-scoped rows sat in
one block with nothing saying which rule each acted in.

Five groups now: **Dome** (what the sky is) · **Orbs** (what populates it
— exactly the four reseed facts plus brightness) · **Motion — all rules**
· **Orbital rule** · **Flocking rule**.

**The layout and the readout stay in step with no table between them.** A
group whose name ends `"<rule> rule"` grows a live-rule line under its
header, and WHICH rule is read out of the name itself. `Motion — all
rules` is plural and correctly grows none: those rows are never dormant.
Adding a rule group later needs no shell edit.

Only the four rule NAMES are duplicated in JS (D3), with the authority
named. Two further hardcodes were refused: which DOORS cycle the rule
(a C++ number that would silently move the readout onto the wrong button
on a renumber — so the line sits under the whole door bar), and which
GROUPS are rule-scoped (derived, as above).

### The speed dial's claim note

`OrbConsole.speed_mult` is the master motion strength: the kernel scales
brownian's noise, orbital's angular speed and flocking's speed ceiling by
it. FROZEN has no energy term, which is that rule's defining property.

> A gen-2 coupling CLAIMS this field through a rest+gain seam when it
> arrives; until then the dial IS the rest.

Its floor of **0 is HONEST**, and the distinction is worth keeping: the
thirteen orb-mood floors sit one step off zero because `eff()` reads 0 as
*"no opinion"*. Nothing reads this one as a sentinel — the kernel
multiplies by it — so **zero is stillness**, an authored artistic state,
and the dial reaches it.

It replaced an ORPHAN. `OrbsState.speed_mult_current` was the gen-1
coupling's CPU smoother; the coupling's writer retired and left the field
declared, reset in teardown, read once, and pinned at 1.0 forever — with a
comment still promising a smoother that no longer existed. A GPU field, an
uploader with zero callers, and no author: an authored landing pad with
nothing landing on it.

### The pill

Minimized is a STATE, not a scroll: one session variable, three doors —
the header's minimize button, the pill, and the backtick. The pill lives
on `<body>` and not inside `#organ`, because `#organ` is what gets hidden
and a pill inside it would vanish with the thing it exists to bring back.
Both targets are 44 px, because the panel is used on a phone and that is
the smallest a thumb hits reliably. No storage — the width and the
`openMap`'s law, extended.

### Two authors, one home (the CODA's row)

`CONFIG.floater_coordination` is the campaign's 307th row and the first
enrolled on a field that another author already writes. Both are writers
and NEITHER is an applier: `cube_behaviors.hpp`'s
`cycle_floater_coordination` steps the field through
`FLOATER_COORDINATION_STEPS { 0, 0.5, 1 }`, the dial writes the same
`config_` member through `organ_set`, and nothing re-speaks it at any
boundary. So coexistence is lawful and LAST WRITER WINS — the palette
situation's exact opposite, where an applier stood over the panel's word
and took it back on the next mood. The contest column is the narrator:
it already asks whether the panel's last word on a dial still stands, and
here that question has a second author to answer about.

The range is the command's own set, `[0, 1]`, and the step 0.005 puts
every one of its three stops on the dial's grid — so the dial can say
everything the command can say and 198 values besides.

And the census that priced it found the second author UNREACHABLE:
`cycle_floater_coordination` is declared and defined in
`cube_behaviors.hpp` and called from nowhere in the tree, so the contest
is LATENT. Today the panel is this field's only reachable author — which
is why the row was worth landing rather than a reason to hold it. When
the command is bound, the contest column starts narrating and no line
here has to change.


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


## The persistence ladder (ATMOS_1)

Every parameter stands on one rung, and the rung says who takes it back
and when.

1. **The instrument's registration** — every `*_LIVE` bank, the drivers'
   gains, the canvas's envelopes. Written by the panel and the presets;
   no program author touches it. Held through everything.
2. **The player's preferences** — orb rule, flock gesture, aura intent,
   palette, the possessed body, render radius. Seeded once by a mood's
   first run, then the player's. Held through transitions.
3. **The environment's instance** — `sunDirection_`, `sunColor_`,
   `mood_state_.sun_*`, `clearColor_`, `mood_state_.fog_rest_*`, and
   `mood_state_.regime`, rolled by `apply_mood_regime` from rung 1 and
   the seed. Authored by `apply_mood` at every entry from rung 1 and the
   seed; re-spoken at the frame boundary when rung 1 is edited (RESPEAK).
4. **The world's draw** — terrain, spawns, the finite radius, the indoor
   palette, and the atmosphere's draw (`draw_atmosphere`). Reborn at
   teardown; the same seed is the same world.
5. **The drivers' output** — fog, the checker field, the ribbon pipes,
   aura presence, the fade. Re-authored every frame as
   `rest (3) + gain (1) · deviation`; never held.
6. **The live simulation** — orb pos/vel/colour, agents, GoL cells, the
   aura grid, the camera. Advances per frame; a discrete command changes
   the law, not the state; reborn only at teardown.

A transition holds 1–2, re-speaks 3, reborns 4 and 6; 5 continues over
the new rest. "Held regardless" is rungs 1 and 2. "A custom environment"
is a rung-1 row plus its rung-4 draw.

## The atmosphere is a distribution (ATMOS_1, ATMOS_2)

A mood is `{ WorldShape shape; Atmosphere atmos; }` (contracts/
spine_state.hpp). The shape is structural — generation reads it, the
eligibility rule bars dials. The atmosphere is a distribution: a sun
bearing (centre, azimuth and elevation spreads) and up to four REGIMES,
weighted by the mood's own `regime_weight[]` (REGIME_1). **A regime is a
state of the world; the atmosphere's row for it is the whole sky** — the
sun's colour, the light's intensity and ambient, the fog's rest and
colour, the clear colour, each a centre with its own spread — and the
seed picks one regime for the whole world. "The light
according to this fog" is therefore written, not coupled: they are in the
same row. A colour's spread is a ± on brightness over the whole triple,
hue kept. `draw_atmosphere(seed, atmos)` is a pure deriver
(direction/mood.hpp): the world's seed draws one instance at every apply,
so the same seed draws the same sky and the back portal keeps its
promise. A DEFONLY dial on any of it re-draws the live mood at the
boundary with the same seed: the instance moves with the dial rather
than re-rolling. Spread 0 draws the centre exactly (a colour multiplies
by exactly 1.0f), which is how the four pre-ATMOS_1 moods stayed
bit-identical: one regime at weight 1, every spread 0.

Three designs were weighed for "the light according to each fog"
(ATMOS_2). Independent rolls — a light table and a fog table — buy
combinations and cannot say it: the tables never meet. Per-parameter
subscription — each parameter naming which roll picks its row — is the
general form, but a row's liveness then depends on a selector the shell
cannot read from a name, and the readout law would need an ABI per row.
Flat regimes are the leaf form of both: any tree of tiers and sub-tiers
flattens to its leaves, and what a tree adds over the leaves is SHARING.
Sharing is an economy — a per-parameter "mood-wide" flag — priced in
OPEN.md; today a parameter wanted the same in every regime is set equal
in every regime.

### The subscription law (REGIME_1)

The regime is the world's. `MoodProfile.regime_weight[REGIME_COUNT]` is
the mood's law — how often it draws each regime — and `apply_mood_regime`
rolls it first in `apply_mood` and first in the boundary's mood re-speak,
writing `MoodState.regime` before any applier reads it. `draw_atmosphere`
takes that index as a parameter and obeys.

A family subscribes by carrying `[REGIME_COUNT]` columns in its own
definition and indexing them with `MoodState.regime` at its own apply.
There is never a second roll: "moonless, thick, six slow walkers, dim
orbs" is one index read by four families. The atmosphere is the first
subscriber; its columns are the `Atmosphere · Regime N` groups and its
law's four rows are `Atmosphere · Regimes`, at the head of the section so
the hand finds the law beside the rows it governs.

The eligibility rule is the gate on who may subscribe: a family's columns
must be read by its applier and by nothing else at runtime. The orb mood
bank passes today; the agent populations need enrolling first; the
ribbon, the terrain and the canvas are read per frame and would need
per-mood rests before they could carry a column; `WorldShape` can only
ever take effect at the next world, a temperament question rather than a
dial question.

The first second subscriber owes one thing: the weight rows raise the
MOOD definition flag and no other, so a weight edit re-speaks the sky
alone. When the orbs subscribe, the edit must raise their flag too, or
the sky changes regime and the orbs keep the old one for a frame or for
a world (OPEN.md prices it).

Variants are moods: `open_sunset`, `open_night`, `open_noon` share one
`SHAPE_OPEN` and differ only in atmosphere. A new mood is one `SHAPE_`
(or a shared one) and one `ATMOS_` constant, one row in each positional
per-mood table (`MOOD_TABLE`, `ORB_MOOD_TABLE`, `AGENT_POPULATIONS`,
`MOOD_SPAWN_MULT`, `CUBE_POPULATIONS`), a portal colour and a weight in
`WORLD_DRAW_TABLE`, and a name in `MOOD_NAMES` — every table's assert
names the commit it expects.

The destination law is one weighted table, `WORLD_DRAW_LIVE.mood_weights`,
walked by id (`pick_portal_mood`; `pick_open_mood` restricts the walk to
open shapes — the triad's way out of a room). A weight of 0 shuts a door
without unmaking the mood.

### The regime readout (ATMOS_1b, ATMOS_2)

The regime rows are mode-scoped: a world is drawn into one regime by its
seed, and the other regimes' centres and spreads move nothing in it. So
the rule readout's law applies and the shell applies it the same way — a
group whose name ends "Regime N" grows a live line under its header,
derived from the name, lit when this world was drawn into that regime
and dim otherwise. `organ_regime()` answers through the pointer
`organ_mood()` already borrows: `bind_mood` hands the registry the
spine's mood organ, and the two ABI calls are two windows on one home.
The operator never sees an index: the scope line, the status line and
the `[Atmos]` witness all print the label's number.

The weight rows (`Atmosphere · Regimes`) are the one dormant-regime edit
that can move the live world: the roll is fixed by the seed, the
thresholds are the weights, so raising a regime's weight can bring this
world into it without a transition. The scope line's hover says so.

### The regime lens (LENS_1)

A regime is an axis, not a group, and the panel's rows are one set looked
at four ways. The lens — a select beside the mood select — picks the way:
**this world's** (the default) shows the regime the world was drawn into
and follows the draw; **regime N** shows that regime whether or not this
world is in it, the scope line saying which; **all** shows this world's
rows and fans a write to the same row in every regime. Kin are found by
name — the group with "Regime N" struck out, plus the label — the shell's
one permitted kind of knowledge about a dial. Under *all*, a label wears
`≠` when its kin disagree and the hover lists them; a write clears it.
The lens is a view: export and import see every row. A section's tally
reads `shown/total` whenever they differ, whichever instrument hid the
rest.

### The tuning loop

`?organ=1&mood=4&seed=N` boots straight into the night (DOMESDAY_1 B9's
`?mood=` took the two new ids for free, range-checked against
`MOOD_COUNT`); the seed pins which regime. Export; drop the JSON in
`web/presets/` and add its line to `index.json`;
`?preset=<name>&mood=4&seed=N` boots the tuned sky. The `[Atmos]` witness
speaks once per regime — `(mood, seed, regime)` — so a drag is silent and
a regime change under a weight dial is announced.
