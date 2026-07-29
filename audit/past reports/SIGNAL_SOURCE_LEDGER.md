> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# RECON A — THE SIGNAL SOURCE LEDGER (the music half's first survey)

The SOURCE wall of the patch bay: what the analyzer computes, what it ships,
what anything reads. Read-only, verified at HEAD `66f582c`. **STOP.**

---

## §0 HEADLINE

The analyzer is a **MIDI pitch-class engine** — no audio path, no frequency
domain (DFT/FFT: absent everywhere, code AND design). The signal is 4128
bytes (time trio + 8 channels × 128 stat slots); the GPU relay windows
**channel 0, slots 0–63 (the 256-byte block)** — and at HEAD that window is
**read by NO shader**: every WGSL consumer was retired at M1-C (tombstones
below). The only live stats consumers are CPU couplings
(`visual_canvas.tick` — fog + voice/room playheads) and the_lab's
dashboards. Meanwhile the analysis half computes and DISCARDS a coupling
goldmine every frame: onset/release masks + counts, field-election strength
and ambiguity, tempo + play-state, note velocity, spine provenance. Chroma
exists as computation (12-bin pitch-class vectors throughout), absent only
as a label.

## §1 THE PRODUCTION PATH (canvas/port → harvest)

RtMidi thread → `MidiPort::on_rtmidi_callback` (midi_port.hpp:158) →
clock bytes to `MidiTransport::feed` (src/sources/transport.hpp:35 — 0xF8
pulses, `beats() = pulses/24 PPQN` :69, EXACT and phase-locked; `bpm()` :70
is estimate-only, never advances time) · note bytes to the lock-free ring
(midi_port.hpp:186,189) → `Canvas::update` (canvas.hpp:138: beat :141, ring
drain :143) → `route` :232 → `Context::receive` (context.hpp:108,
stream+spine) → `advance` :245 (Playhead/Wagon rebuild) → `step_fields`
:503 → **`publish`** :396 → `output()` :258 → the harness
(incubator_dual.cpp:233 `analysis.update`, :238
`render.update(analysis.output(), …)`; layout handshake
`bind_signal_layout(analysis.stat_layout())` :189) → the_board
`update(signal,…)` cartridge.hpp:926 → U1 `phase_fill_signal` :689.

## §2 THE SIGNAL + THE SLOT MAP

`src/analysis/analysis_signal.hpp:72-105`: `{ t_seconds, t_beats, dt,
_pad0, stats[1024], _pad1[4] }` — 8 channels × 128 slots
(`stat_index = ch·128 + slot`, :64), `sizeof == 4128` static_asserted
(:107). DOC BUG: the :80 comment says "2048 bytes"; the block is 4096.
Channel assignment (canvas.hpp:118-133): **voices 0–6 → channels 0–6;
the group union → channel 7** (`all.*`). Pitch classes re-origined to D
(`PROJECT_PC_ORIGIN=2`) before write (:422-423).

The canonical per-channel slot map (canvas.hpp:283-290) + producers:

| slot | name | contents | units | producer | wired? |
|---|---|---|---|---|---|
| 0-11 | PRESENT_COUNT | sounding notes per pitch class | count | `pc_count(playhead)` pc_count.hpp:38 → write canvas.hpp:417,442 | YES |
| 12-23 | PRESENT_LENGTH | present pcs by provisional length | beats | capability `pc_length(ph)` pc_count.hpp:68 | **NO** (unwired) |
| 24-35 | WINDOW_COUNT | present+window occurrences | count | capability pc_count.hpp:56 | **NO** (unwired) |
| 36-47 | WINDOW_LENGTH | present+window length per pc | beats | pc_count.hpp:88 → write canvas.hpp:418 | YES |
| 48-59 | CURRENT_PC | current line note, one-hot | 0/1 | `current_note(spine)` spine_ops.hpp:36 → write canvas.hpp:416 | YES |
| 60 | DISTANCE | signed prev→current interval | semitones | `line_distance` spine_ops.hpp:46 → write canvas.hpp:429 | YES |
| 61 | FIELD | held harmonic-field rank | index 1..6 | `HeldField::step` field.hpp:115 → write canvas.hpp:414 | YES (group band) |
| 62 | POLYPHONY | present voice count | count | — | **NO WRITER EXISTS** |

Time trio: `t_beats` ← transport beats (exact); `t_seconds` ← wall clock
accumulation (canvas.hpp:140); `dt` ← wall frame delta.

## §3 THE SHIP (what leaves the analysis half)

- The U1 relay (cartridge.hpp:698-699) copies `signal.stats[0..63]` →
  `gpuSignal.stats[64]` — i.e. **CHANNEL 0 (voice 0), slots 0–63**. The
  group aggregates (channel 7 = stats[896..1023]) and voices 1–6 NEVER
  reach the GPU. GPUFrameSignal (state.hpp:307-333): analysis carries
  t_seconds/t_beats/dt/stats[64]; everything else is engine-side (input
  deltas, dt_beats derived, the sky block owned by resync_sky_head —
  upload_signal writes bytes [0,304) only, state.hpp:1650-1653).
- CPU-side, `visual_canvas_.tick(signal)` (U4, cartridge.hpp:739) resolves
  named slots via the StatLayoutView handshake — fog + voice/room playhead
  couplings (visual_canvas.hpp reads via `signal.stat(…)`).

## §4 THE READ WALLS

- **GPU: ZERO live `signal.stats` reads in world.wgsl.** Three tombstones
  only: 1823 ("stats[0] terrain-amplitude coupling, retired M1-C"),
  6695 + 6980 ("DRIVERLESS (M1-C): raw signal.stats[0] substituted").
  **The 256-byte relay is a dead pipe at HEAD** — uploaded every frame,
  consumed by nothing.
- CPU (the_board): exactly cartridge.hpp U1/U3/U7 (time trio →
  gpuSignal/time_state_; the U7 transition timer reads dt) +
  visual_canvas.tick's named couplings. No signal reads anywhere in
  machine/, bodies/, direction/.
- the_lab: the only FULL consumer — StatShape-dispatched dashboards sweep
  channels × slots (the_lab.cpp:180-208, 294/315/332, 450-481).

## §5 CHROMA + DFT VERDICTS

- **Chroma: PRESENT as computation, ABSENT as label.** 12-bin pitch-class
  vectors are the analyzer's native fabric — `PitchClassVector`
  (musical_ops.hpp:102), `pc_count`/`pc_length` (pc_count.hpp:38-88),
  `present_set` (field.hpp:32). Every literal "chroma" hit in the repo is
  render-side COLOR chroma (ribbon/visual_canvas/world.wgsl).
- **DFT/FFT/spectral: ABSENT — implemented AND design.** Zero hits for
  fft/dft/fourier/goertzel anywhere; "spectrum" in this repo means the
  terrain band palette (color), plus ImPlot's built-in colormap name.

## §6 THE DELTAS

COMPUTED BUT NOT SHIPPED (the coupling goldmine — produced every frame,
discarded):
1. **Onset/release masks + counts** (playhead.hpp:58-67,148-176) — the
   percussive axis, unshipped.
2. **Field-election strength + ambiguity** (`FieldChoice.overlap`/`tie`,
   field.hpp:69-94) — only the incumbent index ships.
3. **Tempo + play-state** (`bpm()`, `playing()`, transport.hpp:67-106).
4. **Note velocity** — captured (midi_port.hpp:180, 0-1), consumed by nothing.
5. Wagon aggregates (inside/straddling/overflow, wagon.hpp:77-79);
   spine provenance (Survived/Minted/Chosen, spine.hpp:207-213);
   PreviousEvent (never enabled); `pc_set` (no consumer);
   wagons 1-3 (bank of 4, only wagon(0) read).
6. The three unwired canonical slots (§2) — declared, capable, unwritten.

SHIPPED BUT NEVER READ:
- `stats[64..1023]` (voices 1-6 + the group band) — never relayed to GPU;
  CPU-reachable only via the layout handshake (visual_canvas/the_lab).
- `_pad0`, `_pad1[4]`.
- **The entire GPU stats window** (channel 0, slots 0-63) — relayed, then
  read by no shader (§4). The pipe is pressurized from both ends and
  connected to nothing in the middle.

---

STOP — the SOURCE wall stands. Notable for any future patch-bay ruling:
the wall's strongest signals (onsets, field strength, velocity, tempo) are
exactly the unshipped ones; the shipped ones are the pitch-class field the
render half stopped listening to at M1-C.
