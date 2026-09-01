# THE COUPLING ATLAS

> **A MAP, NOT A REWIRING.** Commissioned by AFTER_AUTOMATON §3 as the
> music campaign's design substrate. Nothing here is wired; Jean composes
> on it and the coupling campaign builds what he composes. Every row is
> cited by symbol.
>
> **THE REASSURANCE IS TRUE, AND THE TREE PROVES IT.** The coupling layer
> is not broken. Amendment D dragged every rest through every campaign as
> its facts moved — `fog_rest_density` re-homed to `sky_state_`, written
> by `stage_sky` from the atmosphere instance, in the commit the bank
> rose. The composition law `out = rest + gain · (driven − rest)` survives
> verbatim at both seams. `PARAM_LAYOUT`'s overlap witness still holds the
> bank at compile time. **What changed is the target world**: smaller,
> stiller, and far more *dialed* than the one the couplings were composed
> against.

---

## §0 — TWO CORRECTIONS TO THE COMMISSION, BEFORE THE TABLES

The directive frames the atlas around two premises the tree no longer
carries. Both corrections make the map better, so they are stated first
rather than quietly worked around.

**(a) THERE ARE NO MISSES. The "BeatClock's empty layout" is gone.**
§3.1 asks which sources "resolve and which miss on the BeatClock's empty
layout, the misses ARE the Ableton seam's shape". That was true and
LIGATURE_1 closed it: `BeatClock` no longer exists in the tree, and
`the_board.cpp` binds `app->analysis.stat_layout()` — canvas_1's real
published layout. **All twelve resolve.**

Jean's own boot log is the evidence: `[Zoetrope] ears bound: 7 of 7`, and
NO `[SignalLayout] N sources unbound` line — that line prints only on a
miss, and its absence is the witness.

**So the seam's shape is not a set of misses.** It is one bit further
out: `port_.open_by_name("loopMIDI")`, and `[canvas] loopMIDI open=0/1`
is where the world learns whether anything is playing. Every binding
below is live and reading zeros when the port is shut.

**(b) THE CADENCE VOCABULARY IS FOUR, NOT THREE.** §3.4 asks for "frame,
boundary, or gen". The tree's own enum (`organ_registry.hpp`) is
**LIVE / GEN / BOUNDARY / DRIVEN**, and the fourth is load-bearing here:

| cadence | meaning | can it carry a voice? |
|---|---|---|
| `ORGAN_CAD_LIVE` | the home is read where it is needed | **yes** — the frame sees the edit |
| `ORGAN_CAD_BOUNDARY` | derived from def-kind / sentinel / block | **yes**, at the boundary's rate |
| `ORGAN_CAD_GEN` | the author's next natural event (STORED — the one bit the registry cannot infer) | **only on that event** — a per-frame voice into a gen row is a voice nobody hears until a rebirth |
| `ORGAN_CAD_DRIVEN` | derived from `ro`: **the row is a METER** | **NO.** A driven row is read-only by construction; `organ_set` refuses it |

Cadence is DERIVED, not hand-painted (`derived_cadence()` is the one home
of the rule). The census: **boundary 106 · driven 14 · gen 25 · live 162**.

---

## §1 — SOURCES: what the analysis side publishes, and who listens

`canvas_1::initialize` publishes **55 names**: seven readings over each of
seven voices (49) plus six group readings over `Source::group({0..6})`.

**Twelve are heard. Forty-three are published into silence.**

That is the real shape of the uncoupled analysis side — not misses, but
**unheard publications**. Every one is a source a coupling could take
today with no analysis work at all.

### The twelve that are read

| # | source name | reading | resolved in | drives |
|---|---|---|---|---|
| 1 | `all.field` | `Reading::Field` | `fog_field_` | fog density + fog colour (a table index into `FOG_BY_FIELD`) |
| 2 | `ch1.present_count` | `Reading::PresentCount` | `voice_playhead_` | the sustain swell's hold clock (`RIBBON_VOICE`) |
| 3 | `all.window_length` | `Reading::WindowLength` | `room_wagon_` | the room tint's HUE (pitch-class centre of mass) |
| 4 | `all.present_count` | `Reading::PresentCount` | `room_playhead_` | the room tint's MIX gate (sounding vs silent) |
| 5 | `ch1.window_length` | `Reading::WindowLength` | `checker_win_` | the checker resultant colour (`CHECKER_VOICE`) |
| 6–12 | `ch0.onset` … `ch6.onset` | `Reading::Onset` | `zoetrope_ears_[0..6]` | the zoetrope's seven row impulses (`ZOETROPE_EARS = 0b0111'1111`) |

Both named voices are `ch1` — `RIBBON_VOICE` and `CHECKER_VOICE` are the
same wire, the chordal piano. **One voice carries two of the five
couplings**; the other five MIDI channels are heard only as onsets.

### The forty-three that are published and unheard

| family | names | what a coupling could take |
|---|---|---|
| `chN.current_pc` (7) | the one-hot current note per voice | a per-voice melodic pointer — nothing reads a single voice's note |
| `chN.distance` (7) | the line's signed distance | **contour**: whether a voice is rising or falling. No consumer at all |
| `chN.dft_mag` (7) + `chN.dft_phase` (7) | the six interval families per voice | **the whole pc-DFT capability is unheard on every voice** |
| `all.current_pc` | per-pc voice count | the room's note density |
| `all.dft_mag` + `all.dft_phase` | the room's interval families | consonance / dissonance as a scalar the world could wear |
| `chN.present_count` (6 of 7) | the Playhead per voice | only `ch1` is heard |
| `chN.window_length` (6 of 7) | the Wagon per voice | only `ch1` is heard |
| `chN.onset` — all 7 heard | — | the one reading with full coverage |

**The DFT is the headline.** Fourteen published names (`dft_mag` /
`dft_phase` across seven voices) plus two group names, computed every
frame by the compound stratum, and **not one has a reader**. The tree
calls it "the pc-DFT capability"; it is capability with no consumer.

---

## §2 — PIPES: `PARAM_LAYOUT`'s eight

The register map is hand-laid and compile-time-checked: a `static_assert`
lambda proves no pipe leaves the bank and no two overlap. Rests are the
canvas's own; the WORLD's rest is composed in at the seam.

| # | pipe | base/count | rest | composition seam | live reader | status |
|---|---|---|---|---|---|---|
| 1 | `fog.density` | 0 / 1 | `0.0` (a DEVIATION) | `cartridge.hpp`, `set_fog`: `max(0, ms.fog_rest_density + drv.gain · fp.get(...))` | `GPUState::set_fog` | **LIVE.** `DRIVER_LIVE.fog.gain = 0.63` — the desk dialled it back |
| 2 | `fog.color` | 1 / 3 | `0.0` per channel | same seam, per lane, `clamp(rest + gain·dev, 0, 1)` | `GPUState::set_fog` | **LIVE**, same gain |
| 3 | `ribbon.amp_lateral_mult` | 4 / 1 | `1.0` (identity) | `ribbon.hpp`: `ml_raw = valid ? vp.get(base) : R.rest_amp_lat` | the ribbon's dance | **LIVE.** Swell 1→2× over 8 beats |
| 4 | `ribbon.amp_vertical_mult` | 5 / 1 | `1.0` | same | same | **LIVE**, same law |
| 5 | `ribbon.color_stim` | 6 / 3 | `0.0` | `ribbon.hpp`: `st = valid ? vp.run(base) : nullptr` | the ribbon's tint | **LIVE.** Hue from the Wagon's pc centre of mass |
| 6 | `ribbon.color_mix` | 9 / 1 | `0.0` | `ribbon.hpp`: `mix_raw = valid ? vp.get(base) : R.rest_tint_mix` | the ribbon's tint | **LIVE.** Gated by the room Playhead |
| 7 | `terrain.checker_mean` | 10 / 3 | `0.0` | `cartridge.hpp`: per lane against `ck.rest_resultant[]` | `set_checker_color_field` | **LIVE.** Sample-and-hold on the beat grid |
| 8 | `terrain.checker_var` | 13 / 2 | `0.0` | same seam: `[0]` amount, `[1]` variance | `set_checker_color_field` | **LIVE.** Amount 0 → each cell's SEED colour, not gray |

**All eight are live and all eight have a reader.** The eight pipes
occupy slots 0–14 of a **256-slot** bank (`VISUAL_PARAM_SLOTS`), so the
register map is **6% allocated**. Room is not the constraint on this
layer; reach is.

**The gain rows live in `DRIVER_LIVE`** (18 words: fog 1, aura 4, checker
6, ribbon 7) — enrolled, panel-visible, and the thing that makes every
coupling dialable to silence without unwiring it. **`DRIVER_LIVE.aura`
carries `intent = 0`**: the aura's coupling is authored and OFF, and its
own comment records that it rested ON for exactly one commit.

**Every pipe carries a `Segment`** and moves through
`trajectory_release(seg, goal, beat, span)` — spans in BEATS, never
seconds. That is the composition law's second half and it is uniform
across all eight.

---

## §3 — EARS: the zoetrope's seven rows

| fact | value |
|---|---|
| source | `chN.onset` for each set bit of `ZOETROPE_EARS = 0b0111'1111` — channels 0–6 |
| fold | `zoetrope_rows_[ZOETROPE_ROW_OF_PC[(i + 2) % 12]] += w` |
| the un-dressing | published vectors ship DRESSED to D (index 0 = D, the canvas contract `PC_COLOR` also binds); `ZOETROPE_ROW_OF_PC` is authored by RAW pitch class (0 = C), so `+2` un-dresses |
| shape | **impulses, not an accumulator** — overwritten every tick; the lattice integrates |
| accessor | `VisualCanvas::zoetrope_rows()` → `const float*`, seven wide |
| **its ONE consumer** | `cartridge.hpp:1100` — passed with `world_state_.active_seed` and `signal.t_beats` |
| witness | `[Zoetrope] ears bound: N of M (mask 0x%02X)` — doctrine, printed always, so a deaf zoetrope names its fault |

### The `reveal_zoetrope` tie-in for PANEL-I U4

`reveal_zoetrope` (`bodies/cube_behaviors.hpp`, declared and defined)
is one of the orphan console verbs — **reachable from no key and no
door**. It is a lever on machinery the program still has: the ears are
bound 7 of 7 and the rows are being struck every tick.

**Atlas verdict for U4: this is a DOOR, not a death.** U4's pre-resolved
default says an honest lever on surviving machinery graduates to an organ
door, and this is the clearest case in the parked list — the thing it
reveals is live, listening, and currently invisible.

---

## §4 — THE UNCOUPLED NEW WORLD: live-bank dials a voice could take

Cadence decides what kind of voice a dial can carry, so it is the second
column. **A `gen` row cannot wear a per-frame voice** — the edit lands at
the author's next natural event, so a modulation into one is heard once
per rebirth. **A `driven` row cannot wear a voice at all** — it is a
meter, and `organ_set` refuses it.

### 4a — THE AUTOMATON (born ONE_SURFACE-II U1; **nothing enrolled yet**)

`AUTO_LIVE` is 0/15 named by `organ_gap.py`. Enrolling it is PANEL-I U3's
largest item; until then no cadence exists, so the column below is the
cadence each field WOULD need.

| dial | would-be cadence | the voice it could carry |
|---|---|---|
| **`tick_period`** | **live** | **THE ABLETON CUE.** Already in BEATS — `upload_automaton_header`'s gate is `floor(beats / tick_period)` crossing an integer. Point `beats` at a Link/MIDI transport and **the ground steps on the bar.** The hard half is done |
| `density` | **gen** | the world's aliveness — drawn at birth, so a voice here is a per-rebirth voice. Census: 0.12 seed → 3.1% live |
| `alive_height` | live | how far the ground lifts. A per-frame voice would breathe the whole surface |
| `rule_mask` | gen | which Life. A voice would be a *rule change*, which is a world change |
| `transition_fraction` | live | the spring's duty cycle — how much of a tick the cell spends in transit |
| `color_mode`, `target[3]` | live | the alive-colour policy and its target |
| `spring_variance`, `phase_randomness`, `tempo_randomness` | live | per-cell scatter — the field's *texture* rather than its shape |

### 4b — ATMOSPHERE beyond fog (`ATMOS_LIVE`, 15/15 enrolled)

Fog is the only atmospheric fact with a coupling. Every other field is
enrolled, dialable, and voiceless:

`sun_direction` (+ `sun_az_spread_deg`, `sun_el_spread_deg`),
`sun_color` (+ spread), `intensity` (+ spread), `ambient` (+ spread),
`clear_color` (+ spread). **`intensity` and `ambient` are the two a room
would feel first**; `clear_color` is the sky itself.

Note the shape: every one is a CENTRE with a SPREAD, and the spreads are
all `0` today (the point-row witness). A voice can take the centre; the
spread is a per-world draw and belongs to the seed, not the music.

### 4c — THE ORBS (`ORB_TABLE` / `ORB_CONSOLE`, 18/24 named)

The named six absences are `base_hue`, `hue_variance`, `motion_rule`,
`hue_converge_target`, `tierset_id`, `flock_gesture_default`.
**`motion_rule` and `flock_gesture_default` already have DOORS**
(`ORGAN_DOOR_ORB_RULE`, `ORGAN_DOOR_ORB_GESTURE`) — so the orb rule is
already pressable, and a coupling would be a voice pressing a door rather
than turning a dial. That is a different mechanism and worth Jean's
attention: **the door is the only place the coupling layer could speak in
EVENTS rather than in continuous values.**

### 4d — WEIGHT ARRAYS (`AgentPopulationBank`, `CubeBank`)

`behavior_weights` and `tier_weights` (agents), `behavior_weights`
(cube) — absent from the panel because **the manifest has no row shape
for an array of weights**. They are the most musical thing in the
population system (what KIND of thing the world spawns) and the least
reachable. A row shape for them is a real PANEL-I question.

### 4e — THE SEED DOOR ITSELF (PANEL-I U1)

The seed is `gen` by definition — it is the world's identity. A voice on
the seed is **a voice that ends worlds**, which is a composition decision
of a different order from every other row here. The atlas names it and
takes no position.

---

## §5 — THE ORPHANED: coupling-side code whose reader or source died

| symbol | where | verdict |
|---|---|---|
| `Reading::Polyphony` | `canvas.hpp` enum | **PUBLISHED BY NOTHING.** The enumerator exists; no `publish_reading` call names it |
| `Reading::PresentLength` | same | **PUBLISHED BY NOTHING** |
| `Reading::WindowCount` | same | **PUBLISHED BY NOTHING** |
| the 43 unheard names | §1 | published, computed, **read by nothing** — capability, not orphans |
| `DRIVER_LIVE.aura` | `driver_surface.hpp` | authored with `intent = 0`. Its four words are live, dialable, and currently inert BY THE DIAL, not by a broken wire — the honest state, and its comment says so |
| `reveal_zoetrope` | `cube_behaviors.hpp` | reachable from no key and no door — **a door candidate, not an orphan** (§3) |

**Three enumerators with no publisher is the only true orphan class
here**, and it is small. The coupling layer's problem is not rot; it is
**reach**: 43 of 55 published names and 15 of 15 automaton dials have no
voice, while all eight pipes and all twelve bindings work.

---

## §6 — WHAT THE MAP SAYS

1. **Nothing is broken.** Eight pipes, twelve bindings, two seams, one
   composition law, all live. The gain rows can dial any of it to silence
   without unwiring it.
2. **The analysis side is far richer than the world hears.** 43 unheard
   publications, and the entire pc-DFT capability among them.
3. **The world got much more dialable while the couplings stood still.**
   The automaton alone adds 15 candidate dials, one of which
   (`tick_period`, in beats) is the Ableton seam's own dial.
4. **Cadence is the composer's real constraint**, and it is derived, not
   chosen: a voice into a `gen` row is heard once per world, and a
   `driven` row refuses a voice outright.
5. **The doors are the unexplored mechanism.** Every coupling today is a
   continuous value; `organ_door` is where the layer could speak in
   events, and two orb doors already exist.

**The one wiring fact worth stating plainly**: `AUTO_LIVE.tick_period` is
already in beats, and the automaton's tick gate already crosses on
`floor(beats / tick_period)`. The ground stepping on Ableton's bar is a
transport away — no automaton change at all.

---

# APPENDIX — THE SEAM SCOUT (THE WRAP ORDER §1.4)

**Probe-free, no builds.** The transport choice is Jean's and is made on
these facts. What follows is the analysis side AS IT STANDS, then the
three candidate transports at the level the wrap order asked for: *what
exists in-tree, what each adds, which seams each touches.*

## A — THE NATIVE ANALYSIS SIDE, AS IT STANDS

### A.1 The correction the scout's own commission needed

The wrap order's §1.4 asks the scout to enumerate "the **BeatClock's empty
layout**". **There is no `BeatClock`.** It was retired at LIGATURE_1, and
the atlas already corrected the same stale premise in its §0(a): every one
of the twelve heard sources RESOLVES. There are no misses, and therefore
no misses that "ARE the Ableton seam's shape". The seam's shape is the
**forty-three unheard publications** of §1 — a much better fact, because a
miss is a bug and an unheard publication is a pipe waiting for a listener.

### A.2 The clock, end to end — the single most important fact here

| step | where | what it is |
|---|---|---|
| MIDI bytes | `MidiPort` (`sources/midi_port.hpp`) | RtMidi input thread, one system port, opened by name at startup |
| pulses | `MidiTransport::feed` (`sources/transport.hpp`) | **counts 0xF8 timing clocks, 24 per quarter.** Also 0xFA start / 0xFB continue / 0xFC stop / 0xF2 song-position |
| beats | `MidiTransport::beats()` | `pulses / 24`, a `double`. **Exact and phase-locked** |
| the frame's beat | `canvas_1::advance` | `const float beat = (float)port_.beats();  // the DAW's clock` |
| the signal | `AnalysisSignal::t_beats` | stamped in `publish(beat)` |
| the world | `phase_advance_clock` | `time_state_.beats = signal.t_beats` |
| the ground | `upload_automaton_header` | `floor(beats / tick_period)` crosses → one tick |

**POSITION IS COUNTED, TEMPO IS ESTIMATED, AND THE TWO ARE NOT THE SAME
FACT.** `transport.hpp` says it outright: *"tempo is estimated only for
display and is never used to advance the beat."* `bpm()` exists and **has
no reader in the render side at all**. The world's own `TimeState::
beat_rate` is derived independently, downstream, as `db/dt` per frame,
held through silence, defaulting to 100 BPM.

**This is the fact every transport question turns on.** The world already
runs on a counted, phase-locked musical position from an external
timeline. It does not need a beat clock. What it lacks is not TIME.

### A.3 The loopMIDI lane, as it stands

- **Opens by name at startup**, in `canvas_1::initialize`; prints
  `[canvas] loopMIDI open=<0|1>` and `[The Board] canvas_1 ready (loopMIDI
  open|closed)`.
- **Degrades silently and correctly**: closed port → `beats()` stays 0 →
  `t_beats` 0 → the tempo follower holds its 100 BPM default and the world
  runs on authored idle motion. Nothing in the render side tests
  `is_open()`.
- **One direction only.** It READS. Nothing in the tree writes MIDI.
- Channels 0–6 are the seven voices; `ch1` is the chordal piano and
  carries two of the five couplings.

### A.4 `StatLayoutView` — who publishes it and who binds it

- **Published by** `canvas_1::stat_layout()` (an `AnalysisCartridge`
  override), a non-owning view over `layout_[]`, filled by
  `publish_reading(...)` — **55 calls, and the count IS the census** of
  §1.
- **Bound by** `render.bind_signal_layout(...)`, once, in `init_world()`,
  before the first frame. The render side **resolves by NAME**
  (`musical/signal_layout.hpp`); an absent name takes the graceful path —
  one stderr warning, `valid=false`, that coupling disabled.
- **A new source is one `publish_reading` line and one name to bind.**
  This is the analysis side's exact analogue of the organ's "one line in
  `organ_params.inc`", and it is why §1's 43 unheard names are cheap to
  hear.

### A.5 `AnalysisSignal` — the shape

`alignas(16)`, mirrored into the GPU's `GPUFrameSignal` (80 B, its own
`static_assert`). Carries `t_beats`, `t_seconds`, `dt`, and the slot array
every published reading writes into by address. **The slots are a fixed
grid, not a list**: an unpublished reading is neither written nor
computed, and its slot stays value-initialised at zero.

## B — THE THREE TRANSPORTS

Read A.2 first. The world has musical POSITION already; each transport
below is judged on what it adds beyond that.

### B.1 — Ableton **Link** (tempo / beat / phase)

**In-tree today:** nothing. Link is a third-party C++ library (header +
source, Apache-2.0) and would be the tree's **second** vendored dependency
after RtMidi.

**What it adds:** peer-to-peer tempo and **phase** on a LAN, with no DAW
transport required and no cable. Its distinctive fact is **phase** — where
in the bar you are — shared across applications, plus a `quantum`.

**What it adds OVER what exists:** less than it first appears. MIDI clock
already gives counted position, and phase within a bar is
`fmod(beats, quantum)` from a number the tree already has. **Link's real
gain is that it needs no DAW and no virtual cable** — two machines, or
Live plus this program, agree with zero setup.

**Seams touched:** ONE. `canvas_1::advance`'s `port_.beats()` becomes a
choice of clock source. Nothing downstream moves — not the signal, not
`TimeState`, not the automaton's tick gate.

**Cost:** a vendored dependency, a build-system change (Link wants
platform socket code), a clock-source seam, and a runtime choice between
two transports. **Verdict: the highest ceiling and the highest cost.** It
is the right answer if the instrument is ever played beside Live on
another machine, and an expensive way to get a number the tree already
counts if it is not.

### B.2 — **MIDI** through the existing lane

**In-tree today:** all of it. `MidiPort`, `MidiTransport`, seven voices,
the clock, start/stop/continue/song-position, and 55 published readings.

**What it adds:** **the return direction, which is the actual gap.** The
lane reads and never writes. Two things become possible with no new
mechanism:
1. **CC IN → `organ_set`.** A CC# → `block.field` map over the manifest,
   through the same one write road the REPL and the scene file use. This
   is the CC MAP already parked in OPEN.md's PANEL section, and after
   THE_PANEL II it is *a map plus a callback* — the road exists.
2. **CC/note OUT** — the world speaking back to the DAW. Nothing in the
   tree does this and nothing asks for it yet.

**Seams touched:** the MIDI callback (a second consumer beside
`transport.feed`), and `organ_set` — which is already the one write road
and needs no change. **Zero GPU seams. Zero analysis seams.**

**Cost: by far the lowest, and it is the only one that touches the PANEL
rather than the clock.** The dependency is vendored, the port is open, the
manifest is the whitelist, and the refusal path is built and gated.
**Verdict: the cheapest real capability in the list, and the one the two
PANEL handoffs were the groundwork for.**

**The one design question it raises**, and it is a genuine one for Jean:
**cadence.** A CC turning a `live` row is an instrument. A CC turning a
`gen` row (the seed, the whole automaton) is a *trigger* — it moves
nothing until a door fires. A CC turning a `driven` row is refused
outright. The atlas tags every candidate dial with its cadence for exactly
this reason, and a CC map that ignores cadence produces knobs that appear
dead.

### B.3 — **DAW audio loopback**

**In-tree today:** nothing. No audio input, no FFT, no device layer.

**What it adds:** the one thing MIDI structurally cannot give — **the
sound itself**: amplitude envelopes, spectral content, the character of a
synth rather than the notes behind it. It also hears material that has no
MIDI representation (audio clips, external instruments, anything
recorded).

**What it costs:** an audio device layer (a third dependency, or platform
APIs), a real-time audio thread with its own hard latency budget, an FFT
and the whole feature-extraction question, and a **second analysis
vocabulary** beside the pc/DFT one canvas_1 already publishes. The atlas's
§1 shows the existing vocabulary is 43/55 UNHEARD — so this adds sources
to a world that is not listening to the ones it has.

**Seams touched:** the largest set — a new source tier under `sources/`, a
new analysis path, new `publish_reading` names, plus the same clock
question B.1 raises (audio gives no beat; it would still need MIDI or Link
for position).

**Verdict: the highest capability and the wrong next step.** It is a
campaign, not a seam, and it is the only one of the three that does not
make the existing 43 unheard publications any more heard.

## C — WHAT THE SCOUT CONCLUDES (facts, not a choice)

1. **The world already has phase-locked musical position from an external
   timeline.** No transport is needed to make the ground step on the bar —
   `AUTO_LIVE.tick_period` is in beats and the tick gate already crosses
   on `floor(beats / tick_period)`. **That coupling is a wiring line, not
   a transport.**
2. **The gap is not INPUT, it is the RETURN DIRECTION and the LISTENERS.**
   43 published sources are unheard; the MIDI lane cannot be written to;
   the panel has no player. All three are addressed by B.2 and none needs
   a new dependency.
3. **B.1 buys independence from the cable**, not information. Its case is
   "played beside Live, without loopMIDI", and that is a real case — but
   it is a convenience case, not a capability one.
4. **B.3 buys a genuinely new sense** and costs a campaign, into a world
   already not listening to most of what it is told.
5. **Cadence is the constraint any of them meets**, and the atlas already
   tags it. This is the fact most likely to surprise: the automaton's
   entire vocabulary is `gen`, so a CC over it is a *composer's* control,
   not a *performer's* — until a door is bound to a note.

**No transport is chosen here. No build is proposed.**
