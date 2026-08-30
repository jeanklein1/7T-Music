# LIGATURE_1 — completion report

The splice landed. `the_board.cpp` no longer hands the render side an empty
stat layout; it hands it `canvas_1`'s published one, and all twelve source
names the couplings resolve now bind. The stub is deleted, the F-9 organ
collateral is cleaned, and `docs/OPEN.md` carries the closed row and five
new open ones.

Jean's acceptance run is the part this report does not close. Everything
below it is static or gate evidence taken in a Linux container; the program
build and the Ableton run are his gates and were not attempted.

## 0. Anchors

| | |
|---|---|
| campaign branch | `claude/ligature-1-splice` |
| base | `79adfa4d` (master at campaign start) |
| tip | `732b9b48` |
| commits | `fbf4f00f` (U1), `c1ba230d` (U3), `732b9b48` (U4) |
| cumulative | 8 files, +226 / -1152 |

Blob SHAs after edit:

| path | blob |
|---|---|
| `src/the_board.cpp` | `9f43e266ca0cf9d9f3b75b77d161153d8ae13ce6` |
| `src/console/organ_registry.hpp` | `3047070e199df57c2a7cd6d8f75cf028ec48b817` |
| `src/console/console.hpp` | `a7106e3c65c73fa3bf366ac53b3c9a9cfaf50b16` |
| `src/coupling/visual_canvas.hpp` | `f157ae5c1b0a7f1591515d6407891854362b28d4` |
| `CMakeLists.txt` | `adafeced6165b7d8b010245f435f23c5035501a8` |
| `docs/OPEN.md` | `bd5910f7f43e9d5fd0feb54ef114121a477e39f2` |
| `src/analysis/beat_clock.hpp` | DELETED (was `b10038ff5069783c6be15e1e8d885d36238f7354`) |
| `src/coupling/organ_registry.hpp` | DELETED (was `3047070e199df57c2a7cd6d8f75cf028ec48b817`) |

## 1. U0 — the gates

All three passed; nothing stopped.

**U0.1 preflight.** The clone was already un-shallowed in this session
(`git rev-parse --is-shallow-repository` -> `false`, 2228 commits). No
history claim below rests on a graft boundary.

**U0.2 reachability.** Both present on master:

```
git cat-file -e origin/master:src/analysis/canvas_1/canvas.hpp   -> PRESENT
git cat-file -e origin/master:src/external/RtMidi.cpp            -> PRESENT
```

**U0.3 stale authority.** All eleven anchor blobs at `origin/master` equal
the recon anchors — zero mismatches. Master was still exactly `79adfa4d`,
the commit `docs/LIGATURE_0_RECON.md` was taken at, so the recon's authority
held without qualification.

## 2. U1 — the splice

`src/the_board.cpp`, eight anchored edits, plus the deletion of
`src/analysis/beat_clock.hpp`. Every edit was located by anchor text and
asserted unique before substitution; no edit was located by line number.

| step | what changed |
|---|---|
| 1 | file-header prose: the analysis side is now named as `canvas_1`, not the BeatClock |
| 2 | `#include "analysis/beat_clock.hpp"` -> `#include "analysis/canvas_1/canvas.hpp"` |
| 3 | App member `t7::BeatClock clock;` -> `t7::canvas_1::Canvas analysis;` with the handoff's comment block |
| 4 | `init_world()` ready line reports `canvas_1` and loopMIDI open/closed via `Canvas::is_open()` |
| 5 | the socket: `bind_signal_layout(app->analysis.stat_layout())`, comment per the handoff |
| 6 | frame loop: `analysis.update(dt)` and `render.update(analysis.output(), ...)` |
| 7 | banner: `Clock:    BeatClock` -> `Analysis: canvas_1 (loopMIDI)` |
| 8 | boot site: `analysis.initialize("assets")` replaces the retired clock block |

**Ordering, checked.** `initialize()` builds the layout, so it must precede
the bind. In `main()` it sits at line 398 and `init_world()` — which calls
`bind_signal_layout` — at line 401. On the native lane `init_world()` runs
once from `main()` before the loop, so `frame()`'s `!world_ready` path never
re-enters it.

**R3 honoured.** The member is `analysis`, the pre-CUT_1c noun. The CUT_1c
diff on `src/incubator_dual.cpp` confirms it: `AnalysisCartridge analysis;`
/ `render.bind_signal_layout(analysis.stat_layout())` is exactly what CUT_1c
replaced with the clock, and this splice is its inverse.

**Not restored, deliberately.** CUT_1c also removed the `INCUBATE_ANALYSIS`
selection machinery (`ANALYSIS_HEADER`, `namespace analysis_ns`, the
`ANALYSIS_NAME` display constant). The handoff specifies a direct include
and a direct type, so the selection dial stays retired; `CMakeLists.txt`
says so in prose after U3.

## 3. U2 — witnesses

### U2.1 — `rg -n 'BeatClock|beat_clock' src/`

At the end of U1, one survivor:

```
src/coupling/visual_canvas.hpp:351:            // resolve above happens here, and with the BeatClock's empty
```

Enclosing symbol: `t7::VisualCanvas::bind`, in the PORT_4c socket-witness
block. FLAGGED at U1 (see F-2), repaired at U3. After U3 the census is:

```
$ rg -n 'BeatClock|beat_clock' src/
(zero hits)
```

### U2.2 — `rg -n 'canvas_1|beat_clock' docs/ CLAUDE.md`

```
docs/reference/ATTIC.md:102:| `attic/MIGRATION_canvas_1` | 2026-06-18 | n | the_lab: one playhead heatmap per channel, on a real time axis |  |
```

That is an attic branch name, not a claim about the feed point. No edit.

The literal grep cannot see a doc that misstates the feed point in other
words, so it was widened:

```
$ rg -n 'BeatClock|analysis side|empty stat layout|audio socket|CUT_1c' docs/ CLAUDE.md
docs/reference/RELEASE_CONSOLE.md:9:(index):27   Clock:    BeatClock
docs/reference/RELEASE_CONSOLE.md:17:(index):27 [Incubator] BeatClock ready (bpm 100)
docs/reference/RELEASE_CONSOLE.md:287:  Clock:    BeatClock
docs/reference/RELEASE_CONSOLE.md:306:[Incubator] BeatClock ready (bpm 100)
```

Four lines in a recorded boot transcript. FLAGGED, not edited — see F-5.

### U2.3 — `rg -n '\.t_beats' src/`, verbatim

```
src/coupling/visual_canvas.hpp:367:            const float beat = signal.t_beats;
src/analysis/canvas_1/probe_canvas.cpp:183:            const float beat = canvas.output().t_beats;
src/analysis/canvas_1/canvas.hpp:442:        output_.t_beats   = beat;
src/cartridges/the_board/bodies/pawn.hpp:154:            auraCfg.t_beats = c->time_state_.beats;
src/cartridges/the_board/cartridge.hpp:960:                gpuSignal.t_beats = signal.t_beats;
src/cartridges/the_board/cartridge.hpp:980:                gpuSignal.dt_beats = signal.t_beats - time_state_.prev_beats;  // beats since last frame -> step_trigger
src/cartridges/the_board/cartridge.hpp:1062:                time_state_.beats = signal.t_beats;
src/cartridges/the_board/cartridge.hpp:1069:                    const float db = signal.t_beats - time_state_.prev_beats;
src/cartridges/the_board/cartridge.hpp:1072:                    time_state_.prev_beats = signal.t_beats;
src/cartridges/the_board/cartridge.hpp:1169:                    world_state_.active_seed, visual_canvas_.zoetrope_rows(), signal.t_beats);
src/cartridges/the_board/cartridge.hpp:1171:                    world_state_.active_seed, signal.t_beats, signal.dt,
src/cartridges/the_board/realization/state.hpp:3633:                header.t_beats = t_beats;
src/cartridges/the_board/realization/world.wgsl:7912:    let t_beats   = signal.t_beats;
src/cartridges/the_board/realization/world.wgsl:8374:    let t_beats   = signal.t_beats;
src/cartridges/the_board/realization/world.wgsl:10927:            zone_config.t_beats,
src/cartridges/the_board/realization/world.wgsl:11218:    let t_beats = pawn_aura_cfg.t_beats;
```

Sixteen sites. `canvas.hpp:442` is now the writer, and it writes
`port_.beats()` — the DAW's clock, not wall time. The consequence a stopped
transport produces is visible at `cartridge.hpp:980`: `dt_beats` is
`t_beats - prev_beats`, so a held `t_beats` makes it 0 and `step_trigger`
never fires. `visual_canvas.hpp:367` takes the same clock for every Segment
envelope, so a glide in flight holds where it is rather than completing.

No fallback was added. The ruling is that no mechanism is built until a
measurement asks, and the measurement is Jean's.

### U2.4 — predicted boot transcript delta

The twelve names were checked against the 55 `canvas_1` publishes before the
splice, by extracting `RIBBON_VOICE`, `CHECKER_VOICE` and the
`ZOETROPE_EARS` mask from `visual_canvas.hpp` and the `NAME_*` tables and
`publish_reading` group rows from `canvas.hpp`:

```
ZOETROPE_EARS = 0b0111'1111 -> 7 ears (ch0..ch6)
RIBBON_VOICE = ch1   CHECKER_VOICE = ch1   VOICES = 7
canvas_1 publishes 55 names; VisualCanvas::bind resolves 12:

  BINDS   all.field            BINDS   ch1.onset
  BINDS   all.present_count    BINDS   ch1.present_count
  BINDS   all.window_length    BINDS   ch1.window_length
  BINDS   ch0.onset            BINDS   ch2.onset
  BINDS   ch3.onset            BINDS   ch4.onset
  BINDS   ch5.onset            BINDS   ch6.onset

=> misses() would be 0 — every source binds
```

So, for Jean's run:

**Release twin.** The `[SignalLayout] N sources unbound (no audio source)`
line **disappears** — `VisualCanvas::bind` prints it only under
`if (signal_layout_.misses() > 0)`, and misses is now 0. The
`[Zoetrope] ears bound:` line reads **7 of 7 (mask 0x7F)**, where it read
0 of 7 before.

**Debug twin.** `SignalLayout::resolve`'s per-source `#ifndef NDEBUG` warn
fires only on a miss, so the twelve previously-printed miss lines
**disappear** as well. (The handoff predicted twelve per-source *bind* lines
here; there is no such print — `resolve` is silent on success. Recorded as
F-7 rather than silently differing.)

**Both twins, new.** `[canvas] loopMIDI open=<0|1>` on stderr from
`Canvas::initialize`, then `[The Board] canvas_1 ready (loopMIDI
open|closed)`. The banner's analysis line reads `Analysis: canvas_1 (loopMIDI)`
where it read `Clock:    BeatClock`.

Order at boot: banner -> `[canvas] loopMIDI open=` -> `[The Board] canvas_1
ready` -> `[The Board] the_board renderer ready` -> `[Zoetrope] ears bound:`
-> `[The Board] Hot reload enabled:`.

### U2.5 — Jean's acceptance run (his gate; listed, not executed)

Build `the-board-full-release`; Ableton -> loopMIDI; transport playing.

* fog density and tint step across a held-field change and drift on
  `fog_span`
* ribbon amp swells on ch1 sustains, releases to the seed dance in silence
* room tint mixes while the room sounds, fades in silence
* checker resultant colours with ch1 window activity, releasing linearly
  over 8 beats
* cube zoetrope strikes on per-channel onsets

Second pass with the transport **stopped**: note what freezes. `t_beats`
holds; §U2.3 above bounds what that reaches. Record; decide nothing.

## 4. U3 — organ hygiene

* `src/console/organ_registry.hpp` restored to the W3e blob `3047070e`.
  The delta against the reverted `70d09e96` is prose: of 161 changed lines
  153 are comments or blanks, and the remaining rows are two trailing
  comments (on the `core/instruments.hpp` include and on `g_rejected`) plus
  one `static_assert` message string on `kOrganDoors`. No code changed.
* `src/coupling/organ_registry.hpp` deleted — zero readers, per LIGATURE_0
  §7's three-strand proof.
* `src/console/console.hpp` — `kCompilerPlan` collapsed to one line; value
  unchanged (`CompilerPlan::Vulkan`).
* `CMakeLists.txt` — trailing newline restored (the file ended `)` at
  `0x29`).
* Three stale BeatClock comments repaired (F-2).

**U3.5 witness.** `rg -n 'organ_registry' src/ tools/` returns one include
— `cartridge.hpp` naming the console path — and the hardcode at
`tools/organ_ledger.py:41`. The other hits are prose mentions of the bare
filename in `organ_params.inc`, `organ_boundary.inc` and
`organ_readers.py`'s suffix test. Nothing names `coupling/organ_registry`.

## 5. U4 — OPEN.md

`docs/OPEN.md` read end to end first, then: **THE LIGATURE** added as
closed; **THE RADIAL PULSE RING**, **CUT_1c LEFTOVERS NOT RESTORED**, **THE
TIME SOURCE AFTER THE SPLICE** and **DOC NITS FOUND AT LIGATURE_1** added as
open; **THE ABLETON SEAM** amended to record that its MIDI/transport half
arrived and that Link and audio-in stay open.

## 6. Gates

Every row of CLAUDE.md's gate table, run on the pushed tip `732b9b48` with a
clean tree:

| gate | verdict |
|---|---|
| G-LAW 1 | **GREEN** |
| G-LAW 2 | **GREEN** — 312 fn, 326 const, 82 struct, 81 binding, 55 entry points |
| TU gate | **PASS** — tier CARTRIDGE and tier CONSOLE, zero diagnostics |
| score census | **GREEN** — 8 update + 20 render rows, bijection both directions |
| WGSL gate | **PASS** — parses, scopes and validates under naga, raw |
| binding surface | **PASS** — all relations agree, all witnesses pass (S-6 included) |
| organ gap | **PASS** |
| organ ledger | **NO SUSPECTS** |
| mirror census | **PASS** — all witnesses pass, nothing written |

Two notes on how that table was obtained, so it is not read as more than it
is:

* **The WGSL gate was unrunnable when first invoked** — naga was absent from
  this container, and the gate calls an unrunnable gate a failed gate. Rather
  than record a red row for an environment gap, `cargo install naga-cli` was
  run and the gate then passed. `world.wgsl` is byte-identical to master
  (`5b36243d` both sides): this campaign touched no WGSL, exactly as the
  handoff required.
* **S-6 read red mid-campaign** and green at the end. It checks porcelain
  cleanliness and HEAD-versus-pushed-tip, not content, so it is red by
  construction while a campaign is in flight. Every other binding row passed
  throughout.

## 7. Flags

**F-1 — Branch.** The handoff's R7 puts each unit on `claude/ligature-1-*`;
this session's harness designates `claude/ligature-0-recon-hcrix0`. The
handoff is the more specific and later instruction and names the branch
explicitly, so the campaign branch is `claude/ligature-1-splice`. The
LIGATURE_0 report remains on its own branch, unmerged; neither is on master.
Merging is Jean's.

**F-2 — U1 reached past its stated file scope, in U3.** U1 is specified as
"`src/the_board.cpp` only". Deleting `beat_clock.hpp` left three comments
describing a file and a type that no longer exist, one of them citing the
deleted path outright and one — the PORT_4c socket note in
`coupling/visual_canvas.hpp` — asserting that every source resolve misses,
which the splice made false. R5 says comments state present behaviour, so
all three were rewritten. They landed in **U3**, not U1, to keep U1's stated
scope honest. The three: `CMakeLists.txt` twice, `visual_canvas.hpp` once.

**F-3 — `(void)argc; (void)argv;` removed.** The pair sat in the retired
clock block and was justified by its comment ("No command-line input
either"). `parse_boot_params(argc, argv)` at the top of `main()` uses both,
so neither is an unused parameter and the casts were dead. Removed with the
block.

**F-4 — The handoff's debug-twin prediction does not match the code.** U2.4
predicts "twelve per-source bind lines" on the debug twin. `SignalLayout::
resolve` prints only on a *miss*, under `#ifndef NDEBUG`; there is no
success print. The real delta is that the twelve miss lines **stop**
appearing. Recorded rather than quietly substituted.

**F-5 — `docs/reference/RELEASE_CONSOLE.md` is now doubly stale.** It
records a boot transcript containing `Clock:    BeatClock` and `[Incubator]
BeatClock ready (bpm 100)`. The `[Incubator]` prefix was already stale
before this campaign — the driver prints `[The Board]` — which is the
evidence that the file is a *record* of a past run rather than a live claim.
Whether a recorded transcript should be refreshed or left as an artifact is
a ruling, not a one-word fix, so it was FLAGGED and left. Row added to
OPEN.md.

**F-6 — `audit/MIRROR_LEDGER.md` does not match its own tool at master, and
was not regenerated here.** `tools/mirror_census.py` writes by default;
invoking it per CLAUDE.md's table (without `--check`) during the gate sweep
regenerated the artifact and produced a two-line diff — its sweep boundary
states 60 `*.hpp` and 1 `.cpp/.h/.cc` under `src/`, where master has 83 and
8. To establish whether this campaign caused it, the ledger was regenerated
on a **pristine `79adfa4d` worktree**: the same two-line diff appears. The
staleness therefore predates LIGATURE_1 — it is the file count from before
"bringing back the music" — and CLAUDE.md's L33 standing witness does not
hold at master. The accidental write was reverted with `git checkout --`,
and the ledger is byte-unchanged by this campaign. Regenerating it is the
tool's job run deliberately, with its own ruling; a row is in OPEN.md.

**F-7 — The gates ran on Linux/clang++, not on Jean's build.** The TU gate
compiles `the_board.cpp` and therefore the whole new include chain —
`canvas_1/canvas.hpp` -> `sources/midi_port.hpp` -> `external/RtMidi.h` —
with zero diagnostics. It sets only `-DGLFW_INCLUDE_NONE`, so
`__WINDOWS_MM__` is **not** defined for it: the pass proves the headers
type-check, not that RtMidi's Windows Multimedia backend in `RtMidi.cpp`
compiles. That translation unit is unchanged by this campaign and has been
in the target since `e0e22e46`, so the risk is pre-existing rather than
introduced — but the real build is Jean's gate and this report does not
stand in for it.

**F-8 — Nothing was run.** No program was executed, no MIDI port opened, no
frame rendered. Every behavioural claim above is derived from the source and
the gates. The prediction in §U2.4 is a prediction.
