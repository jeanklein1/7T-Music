# OPEN — the register of open state

7T-Music's own open state, and nothing else. Reset at SUNRISE_0 N0: this
repo forked from 7T at `de4b8b6f`, and 7T's register went with 7T. Items
belonging to the sibling — the ledger provenance stamp, the guard-count
drift in its own records, every campaign whose subject is the web line —
were **deleted, not annotated** (L30, L32). They are in that repo's OPEN.md
and in the shared history below the fork point.

## THE FORK

| | |
|---|---|
| forked from | `jeanklein1/7T-Pawns` at `de4b8b6f` |
| recovery tag | `attic/full-board` — the full board, as inherited |
| transplant record | `native-sunset` -> `29cec46b` |
| independence | no `upstream` remote; no merge path back |

`attic/full-board` is the base the pruning campaign works from. It is the
one tag that must never be deleted.

## N-a — DAWN ACQUISITION (open)

Target state, self-verifying: `CMakeLists.txt`'s `dawn_lib` guard passes —
`C:/dev/dawn/out/src/dawn/native/<CFG>/dawn_native.lib` and its siblings
exist for every configuration in `DAWN_CHECK_CONFIGS`.

| | |
|---|---|
| pin | Dawn `56f332d7` (`v20260814.182433`) — the web twin's pin |
| source | `C:\dev\dawn` · build `C:\dev\dawn\out` |
| lane | Vulkan (`kCompilerPlan = CompilerPlan::Vulkan`); D3D12 stays linked and reachable via the adapter scorer |

The recipe is `docs/reference/DAWN_REFERENCE.md` section "Configure and
build", which survived the sunset intact. Three deltas found at N-a and
not yet paid:

1. **depot_tools is not needed.** `DAWN_FETCH_DEPENDENCIES=ON` is
   documented at the pin as *"Use fetch_dawn_dependencies.py as an
   alternative to using depot_tools"*. A plain `git clone` of Dawn plus
   the recorded CMake invocation is the whole route. L39's depot_tools
   prescription was written for a gclient fetch this build does not use;
   following it would add a PATH hazard for nothing.
2. **Both configurations, or neither.** `dawn_lib_optional` treats a
   library present in one config and absent in the other as fatal. The
   Ninja presets pin Release, so Release alone satisfies them — but the
   VS lane is multi-config and proves BOTH configs at configure time,
   and its Debug is the diagnostic build (`the-board-vs-debug`). Build
   both, as DAWN_REFERENCE says.
3. **DAWN_REFERENCE's shader-compiler section is the retired world.** Its
   own CANON banner says so: it describes FXC via `d3dcompiler_47.dll` and
   "only D3D12 is used", both struck by PIVOT_0/L2 before the tag chose
   Vulkan. `DAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON` exists for that FXC path.
   Keep the flag — D3D12 remains reachable — but its stated reason no
   longer describes the selected lane. **Open:** rewrite that section once
   native boots, per the banner's own "rewrite pending".

## N-b — ARM RE-INSERTION (open)

Reverse the sunset diffs onto the living files; never `git checkout` an old
file wholesale over the drift. Paste source is per-file — the parent of the
commit that deleted that file's arm.

| unit | file(s) | source |
|---|---|---|
| N1 | `the_board.cpp`, `boot_params.hpp` | `315d4bc1^` |
| N2 | `console.hpp` | `315d4bc1^` |
| N4 | `CMakeLists.txt`, `CMakePresets.json` | the tag |

### Drift adaptations made during N-b

> DRIFT NOTE (WEB_SUNSET): the first and third bullets describe a twin
> that is attic'd. `apply_pace_once()`, its guard and `g_present_pace`
> were deleted whole at W3a·2 (the pace surface had one reader, the
> `[METER]` window header); `web/index.html` burned at W4b, so the
> `Controls:` print's placement is now the program's own and nothing
> reads it across a seam. Stamped minutes, not rewritten (L28) — the
> record of what N-b did stands; only its live claims are retired.

Recorded because they are **not** restorations and have no source to be
checked against:

- `apply_pace_once()` (`the_board.cpp`) was authored after the sunset
  (WRAP_0 U2) and never had a native arm. Its
  `emscripten_set_main_loop_timing` call is now `#ifdef __EMSCRIPTEN__`,
  with the `g_present_pace` record left on both twins so the meter's window
  header still names the pace.
- `<filesystem>` and `<system_error>` left the TU with `FileWatcher` and
  return inside its guard.
- The `Controls:` print did **not** return to `init_world()`. OVERTURE_0
  moved it into `offer_controls_when_ready()`, and `web/index.html` lifts
  its veil on exactly that line — restoring it verbatim would lift the veil
  before the world is ready, on both twins.

## THE pwsh POST-BUILD FAILURE — NOT THIS TREE'S (closed as a finding)

A `pwsh.exe` invocation fails during the native build. The essentials land
anyway — the DXC pair and the assets folder both arrive beside the exe, the
link completes, the binary runs. It was investigated as a porting job (find
the failing call, swap it for `powershell.exe` or `cmake -E`) and there is
**nothing here to port**. Recorded so the next session does not hunt for it
in `CMakeLists.txt`.

THE EVIDENCE, three independent probes, all negative:

1. **Tracked content names no shell.** `git grep -in "pwsh\|powershell"`
   returns only four prose lines in `docs/LAWS.md` L39, about the emsdk
   `.bat` and PowerShell's `where` alias. No build file, no tool.

2. **The native build emits exactly four commands, and all four are
   shell-free** —

   | site | command |
   |---|---|
   | `execute_process`, configure-time | `${T7_PYTHON} tools/build_stamp.py` |
   | `t7_build_stamp` target | the same, and `VERBATIM`, which bypasses shell interpretation by construction |
   | POST_BUILD | `${CMAKE_COMMAND} -E copy_directory` — the assets |
   | POST_BUILD | `${CMAKE_COMMAND} -E copy_if_different` — the DXC pair |

   `tools/build_stamp.py` itself shells out only to `git describe`. The two
   `cmake -E` copies are precisely the steps whose outputs are observed to
   land, which is the same fact seen from the other side: they never touch a
   shell, so no shell can fail them.

3. **The generated projects are clean.**
   `findstr /S /I /M "pwsh" out\build\the-board-full-release\*.vcxproj*`
   returns nothing across all six generated projects — ALL_BUILD,
   t7_build_stamp, the_board, ZERO_CHECK, VCTargetsPath, CompilerIdCXX.
   The probe is live, not vacuous: `dir /s /b ...\*.vcxproj` lists those
   six. CMake wrote no pwsh call from this tree.

CONCLUSION: the invocation belongs to MSBuild's own VS 2026 toolset, not to
7T-Music. It is an environment behaviour this repo neither causes nor can
fix from inside `CMakeLists.txt`, and the build is complete and correct
despite it.

WHAT WOULD REOPEN IT. If a post-build step ever stops landing — the DXC pair
or `assets/` missing beside the exe — this becomes load-bearing and the
place to look is MSBuild's log at `-verbosity:detailed`, NOT this tree's
four commands. The verbatim error text was never carried into the container
session, so this entry rests on the negative probes above rather than on a
diagnosis of the message itself; anyone who has the text and wants to close
it properly should paste it here.

## THE PRUNING CAMPAIGN AHEAD (S1 + S2 CLOSED at PRUNE_1)

Phase W — the web strip — landed first (docs/LAWS.md: WEB_SUNSET).
**S1 and S2 are CLOSED**: PRUNE_1 took the gallery organ whole — the
photographer, the outdoor galleries and the indoor wall art — and the
findings that flagged them are answered below. **PRUNE_2 landed** after
it and took the grounded five (BLADE, CACTUS, PALM, COLUMN, ANTENNA);
its own close is below. S3 is IN FLIGHT under ONE_WORLD-I; S4 stands,
unchanged and still FLAGGED.

| unit | subject | finding |
|---|---|---|
| S1 | snapshot / photograph | **CLOSED (PRUNE_1).** Not separable, and it was not separated: the whole organ left in one campaign, `SEAM[gallery:dual-role]` with it. |
| S2 | galleries, paintings, hang | **CLOSED (PRUNE_1).** Each flag answered: the veil lift now fires on the elapsed-since-`world_live` condition alone; `compute_entity_placement` lost only its painting loop and Y-corrects the KEEP flora exactly as before; `painting_slots` left `shadowStateLayout_` and the seats behind it re-indexed through `binding_schema.py`, the tool's job, verified by `binding_gen.py --check`. |
| S3 | portals | **IN FLIGHT (ONE_WORLD-I).** Both flags are answered at U1: the `!ROSTER.transitions \|\| ROSTER.portal` edge left with the `transitions` bit it gated, and `TransitionPhase` left together with its only ignition — the machine, the request door, the six keys, the console's mood door and the `portal_trigger` wire (C++ **and** WGSL) went in one commit. U2 then took the doors themselves: the force-spawn channel, the arch's portal identity (`is_portal` / `is_back_portal` / `destination`), `PortalDestination`, `portal_color_for` and the whole destination law (`portal_density`, `mood_weights`, the palette, `pick_portal_mood` / `pick_open_mood`), the `portal` roster bit and the census's portal column. `GPUPortalArray` and the WGSL portal room outlive it — the PASSER route still reads them — and empty at U4. **U3 then took the ARCH family whole**: `PopFamily` 6 → 5 with all eleven positional tables re-columned, the four arch pipelines, the GPU mesh-gen scratch (WGSL §9 and the converged MESHGEN trio 180/181/182, now unallocated), the entity ground atlas and its `compute_entity_placement` writer with both spine rows, `GPUAgentRoomConstants`'s `occupier_amg` window (2864 → 1584 B), and the field/ribbon occupier loops that read it. |
| S4 | agent pipelines | FLAGGED — `agent_state[possessed_slot]` **is** the point; camera, veil, terrain aura, LOD streaming, the ribbon's sky rule and the shadow box all read it |

`WorldDrawSurface` was to empty under S2+S3 together, under a `sizeof`
static_assert. S2 has landed and it did not empty: the fields the
gallery held were never `WorldDrawSurface`'s — the surviving mentions
there are the "Gallery" indoor LIGHT SCHEME (`scheme_weights[2]`) and
the portal dials, which are S3's. Whether the room empties is S3's
question alone now.

### PRUNE_1 — WHAT LEFT, AND THE ARGUMENT THAT MADE IT SAFE

The organ: `bodies/gallery.hpp` whole, six pipelines and their entry
points, the painting/photographer buffers, the three 512-square texture
arrays (snapshot staging, authored staging, exhibition) and the
offscreen snapshot targets, four bind groups and their layouts, the
WGSL §8.1–§8.3 sections and the photographer's own half of §8.0,
`PopFamily::GALLERY`, the `GallerySelection` / `GalleryPlacement` DTOs,
the roster bit, and 57 painting files.

THE TAIL-TRUNCATION ARGUMENT. `PopFamily::GALLERY` was 11, the LAST
family, so `COUNT` 12 → 11 is pure truncation: no surviving family
renumbers, no surviving table column moves relative to another, and
placement priority among the survivors is unchanged. F-1's fear is a
RENUMBERING fear and a tail cut does not trigger it. Nine positional
tables each lost exactly their final element with every other number
byte-identical — including `INDOOR_TREATMENT`, a ninth table the
advance census had not named, found by grepping `PopFamily::COUNT`.

PARKED, not done:
- **The offer's timing.** `OVERTURE_READY_TIMEOUT_S` and its 5.0 s
  stand; the floor term left with the tally it counted. Whether the
  offer should now be immediate is a taste call and Jean's.
- **The `photo_` prefix.** `photo_heightfield` / `photo_sampler` STAY —
  the one patch-grid walk samples them for every room, not just the
  photographer's. The prefix is a legacy of who introduced them; the
  rename is a separate, behaviour-free edit.
- **`assets/atrium/`** (4 files, 1,114,018 B) has NO reader anywhere and
  had none before this campaign — the loader only ever walked
  `assets/paintings` for `PAINTING_*.jpg|.jpeg`, and the `ATRIUM_n`
  names in the code are record indices into that manifest, not these
  files. Left untouched under PRUNE_1 R7; it wants its own tombstone
  ruling (L30), like `presets/` above.
- **The ECONOMY_1 E1 curtain pair.** `patch_index_buffer_lod0_live` /
  `patch_index_count_lod0_live` had ZERO readers BEFORE this campaign;
  `curtainsActive_` is written every frame and read only by them. The
  snapshot pass was the last carrier the prose named, so the notes are
  corrected and the code is left standing — retiring it is its own
  reading, not a prune's side effect.
- **`SpawnClamp::NONE`'s absent clamp.** The value left with its one
  consumer; the sub-ruling that the absent clamp was carried as data,
  and that ruling a clamp IN is a separate taste gate, is now moot.

### PRUNE_2 — THE GROUNDED FIVE (landed)

Five entity families left whole — BLADE, CACTUS, PALM, COLUMN,
ANTENNA — with their roster bits, spawn columns, mesh-gen kernels,
render and shadow pipelines, GPU rooms and the plants' shared organ.
`PopFamily` went from eleven members to six and the surviving five
families were RE-COLUMNED across all eleven positional tables in their
own subjects' commits — the mid-table renumbering F-1 exists to catch,
machine-verified cell by cell against the pre-campaign tree.
- **The MESHGEN convergence.** Five kernels shared bindings 180/181/182
  through their own scratch trios; the trio converged on ARCH alone and
  `binding_schema.py`'s recut regex narrowed with it.
- **`occupier_cmg`.** Deleted on the argument that every reader loop
  opens `if (cm.is_active == 0u) continue;` and nothing writes
  `is_active` once the families are gone — behaviour-identical, not
  merely unreachable.
- **The sweep's two misses, found by verification, fixed in-campaign.**
  A `sky_shell` orphan the WGSL scan would have caught, and an ERASED
  RULING — a deletion hunk that ran into a SURVIVOR's doc block and
  took a re-ruling with it. Both are law now: a prune may take a dead
  subject's prose, never a live one's law, and the orphan sweep
  includes a WGSL reference scan.

## NATIVE PRESET INGESTION (open, born at WEB_SUNSET)

`presets/` holds the authored scenes the web panel used to fetch
(index.json + one file per scene). They currently have NO READER — kept
against L30's letter on the Phase W ruling [F2], because scene recall is
the performance instrument's obvious next organ. The consumer to build:
a boot flag (`--preset=<name>`) walking the same organ_set road
`?preset=` walked, and later the control surface's own load. If that
consumer is refused, this folder goes to the attic with a tombstone.

## THE BUDGET AFTER THE AUDIENCE (open, born at WEB_SUNSET)

L14's storage-8/uniform-12 floor bound because browsers at core defaults
were the audience. Native Dawn can request adapter limits above it.
Untouched at WEB_SUNSET on purpose: raising the floor is a shader-shape
decision with its own witnesses, not a strip's side effect.

## THE DEBUG ARMING (open, born at KEEL_0)

Native Debug currently arms Dawn's own Debug assertions and backend
validation, and nothing program-side: PORT_2c's web arming (SAFE_HEAP
and kin) went with the twin, and no MSVC equivalent was chosen to
replace it. Candidates when a session wants one: /RTC1,
/fsanitize=address, Dawn's extra-validation toggles. Unpriced; arm
deliberately, not by default.

## THE LIGATURE (closed at LIGATURE_1)

The couplings were a two-sided named-resolution system with one hop
missing. `the_board.cpp` fed the render side `BeatClock::stat_layout()`,
which returns `StatLayoutView{ nullptr, 0 }` by construction, so all twelve
source names `VisualCanvas::bind` resolves missed and every pipe idled on
its rest — while the whole target half (resolve, tick, flush, setter, GPU
field, shader read) ran correctly every frame on those rests.

LIGATURE_1 U1 spliced `canvas_1` into that socket and deleted
`src/analysis/beat_clock.hpp`. The twelve names bind against the 55
`canvas_1` publishes, checked name by name before the edit: `all.field`,
`all.present_count`, `all.window_length`, `ch1.present_count`,
`ch1.window_length`, and `ch0.onset` through `ch6.onset`.
`SignalLayout::misses()` is therefore 0, so the release twin's
`[SignalLayout] N sources unbound` line no longer prints and the
`[Zoetrope] ears bound:` line reads 7 of 7.

Nothing was resurrected. LIGATURE_0 §1 established that the analysis arm
returned byte-identical at "bringing back the music"; this was a wiring
change and a stub deletion, nothing more.

**Not closed by it:** Jean's acceptance run. Observables are listed in
`docs/LIGATURE_1_REPORT.md` (U2.5).

## THE RADIAL PULSE RING (open — gen-1 retired, decision unmade)

`pulse_count` and `pulse_data[8]` exist in `world.wgsl`'s config struct and
behind `GPUState::set_pulse_data` in `state.hpp`, and the only caller in the
tree writes the REST — `terrain_looks::REST_PULSE_COUNT` and a zero-filled
array, at the cartridge's boot. There is no CPU middle: LIGATURE_0 §6 walked
the slice and found Hop 2, the ring-buffer write, GONE, with both ends
saying so in their own comments. This is a gen-1 coupling that was retired,
not a hop the splice broke, and LIGATURE_1 R1 scoped it out deliberately.

The GPU end is DRIVERLESS and its comments name a driver that does not
exist.

**The decision, open:** a gen-2 coupling — the onset ears already fold into
`zoetrope_rows_`, but a pawn-origin pulse needs a readback path the coupling
layer does not have — or delete the WGSL contributor and its config seats.
Jean's, with a design pass.

## CUT_1c LEFTOVERS NOT RESTORED (open, low)

"Bringing back the music" restored the musical/analysis/sources arm and
`RtMidi` byte-identically. It did not restore everything `CUT_1c` deleted:

| path | at `1a52f2db^` |
|---|---|
| `src/the_lab.cpp` | 668 lines, blob `3cd1a13fe66e` |
| `src/external/imgui/` + `src/external/implot/` | 18 vendored files |

Restore only if the lab returns. Recovery is `git show 1a52f2db^:<path>`.

## THE TIME SOURCE AFTER THE SPLICE (open — Jean-observed)

`AnalysisSignal::t_beats` now advances from the DAW transport
(`Canvas::update` reads `MidiPort::beats()`), not from wall time, and every
reader inherits that. The census is in `docs/LIGATURE_1_REPORT.md` §U2.3
verbatim; the load-bearing consumers are `visual_canvas.hpp`'s envelope
clock, `cartridge.hpp`'s `time_state_.beats` and the `dt_beats` that gates
`step_trigger`, the zoetrope strike, `state.hpp`'s GPU signal header, and
four `world.wgsl` read sites.

With the transport stopped `t_beats` holds, so `dt_beats` is 0 and every
beat-derived motion freezes. **No fallback was added** — LIGATURE_1's ruling
is that no mechanism is built until a measurement asks for one. What
actually freezes is Jean's second acceptance pass to observe.

## DOC NITS FOUND AT LIGATURE_1 (open, small)

* `CLAUDE.md` names `the-board-vs` as though it were a build preset. It is a
  **configure** preset; the build presets on that lane are
  `the-board-vs-release` and `the-board-vs-debug`.
* `docs/reference/RELEASE_CONSOLE.md` records a boot transcript containing
  `Clock:    BeatClock` and `[Incubator] BeatClock ready (bpm 100)`. Both
  are now doubly stale — the driver has printed `[The Board]` since before
  this campaign, and the analysis line is now `Analysis: canvas_1
  (loopMIDI)`. Left as a record and not edited: whether a recorded
  transcript is a live claim or an artifact is a ruling, not a nit.
* `audit/MIRROR_LEDGER.md` does not match its own tool at master.
  Regenerating it on a pristine `79adfa4d` worktree changes two lines — its
  sweep boundary states 60 `*.hpp` and 1 `.cpp/.h/.cc` under `src/`, where
  master has 83 and 8. The staleness predates LIGATURE_1 and CLAUDE.md's L33
  standing witness (delete the five `audit/` files, run the five tools, get
  a byte-identical tree) therefore does not hold at master. LIGATURE_1 did
  not regenerate it: that is the tool's job, run deliberately, not a
  campaign's side effect.

## THE ABLETON SEAM (held, after N)

Link session (tempo/beat/phase ground truth) and/or DAW loopback as native
audio-in, so the pre-modulation hint gains a real carrier — an explicit
signal over MIDI/OSC/Link rather than inference, which is what Wagon and
Playhead's forward-cue requirement was always waiting for.

> LIGATURE_1: half of this arrived. `canvas_1` opens loopMIDI and reads the
> DAW's transport for beat and tempo, so a MIDI-carried beat/phase signal is
> live and `t_beats` is transport-driven rather than inferred. What stays
> open is Link as the session-level ground truth, and audio-in. The
> `parse_boot_params` socket below is untouched and still unclaimed.

The socket is already open: `parse_boot_params(int, char**)` kept its full
signature through the sunset with both parameters unnamed, `main()` still
calls it with real `argc`/`argv`, and N1 restores the
`--seed/--mood/--cap/--msaa` loop behind it.
