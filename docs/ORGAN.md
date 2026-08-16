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

### The eligibility rule
A field may carry a definition target only if the mood apply is its
ONLY runtime reader. The atmospheric group — `sun_direction`,
`sun_color`, `sun_intensity`, `sun_ambient`, `clear_color` — passes.
The structural group — `finite`, the radii, `indoor`, `ceiling_type`,
`wall_height`, `terrain_amp_ceiling`, the `allow_*` flags — does not:
world generation reads it, and rewriting it without regenerating the
world means nothing at best and disagrees at worst. The rule is
stated in code beside `MOOD_LIVE`.

### The two modes
`definition` (default) writes the live mood's definition and lets the
mood apply re-run at the frame boundary; the edit survives a mood
change, and only for the LIVE mood — an edit to another mood's
definition waits until the program enters it. `preview` writes the
instance: immediate, and the next author may take it back. A dial
with no definition falls back to the instance under either mode.
O1a's contest readings come from PREVIEW writes only, because a
definition write never touches the instance and so asks the
instrument nothing.

Export keys a definition by mood AND id (`"<mood>/<id>"`) and an
instance value by id alone, so one file can carry several moods and
an import puts each back where it came from.

### Open for ORGAN_2
- Twelve of the sixteen enrolled dials have no definition target.
  Four (atmosphere) have no mood field to define; eight (the tiers)
  have an author that is not a mood, so `MoodProfile` is the wrong
  place to reach for. Any of them that O1a reads as PER-FRAME is a
  dial whose panel write cannot last and has nowhere durable to go —
  that is the finding, and it wants a definition surface of its own,
  not a target invented in `MoodProfile`.
- `clear_color` is the mirror case: a definition with no instance
  dial, because `clearColor_` is not one of the three homes.
- `config.sun_direction` beside `lighting.sun.direction` — two
  apparent homes for one fact, carried in from CHORD.

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

## Access
The panel exists only under `?organ=1`; backtick toggles visibility.
Without the flag, no DOM is built, no export is called, the audience
path is byte-identical. The panel is an instrument, not the art.
