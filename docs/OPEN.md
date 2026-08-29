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
   Ninja presets set `DAWN_CHECK_CONFIGS` to the single build type, so
   Release alone satisfies `the-board-full-release` — but leaves
   `the-board-full` (Debug) broken. Build both, as DAWN_REFERENCE says.
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
| N3 | `gallery.hpp` | `315d4bc1^` |
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

## THE PRUNING CAMPAIGN AHEAD (not started)

Phase W — the web strip — landed first (docs/LAWS.md: WEB_SUNSET). The
S-units below are unchanged and still FLAGGED.

SUNRISE_0 delivers a functional native second repo with **nothing cut**.
The strip is the next campaign, worked from `attic/full-board`. Its
advance census is already done and is the reason it is a campaign and not
a chore: **three of its four units are FLAGGED.**

| unit | subject | finding |
|---|---|---|
| S1 | snapshot / photograph | no KEEP-row reader — but not separable: it lives inside `gallery.hpp`, which declares `SEAM[gallery:dual-role]` |
| S2 | galleries, paintings, hang | FLAGGED — the veil lift gates on `gallery_state_.authored_staged_count`; `compute_entity_placement` writes `photo_painting_slots` before Y-correcting KEEP flora; `painting_slots` is entry[3] of the shared `shadowStateLayout_` |
| S3 | portals | FLAGGED — `static_assert(!ROSTER.transitions \|\| ROSTER.portal)` in `demos/demo.hpp` forbids the combination outright; `TransitionPhase`'s only ignition is `point_.portal_trigger` |
| S4 | agent pipelines | FLAGGED — `agent_state[possessed_slot]` **is** the point; camera, veil, terrain aura, LOD streaming, the ribbon's sky rule and the shadow box all read it |

`WorldDrawSurface` empties under S2+S3 together, under a `sizeof`
static_assert — that, not binding group 2, is the "room that empties".

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

## THE ABLETON SEAM (held, after N)

Link session (tempo/beat/phase ground truth) and/or DAW loopback as native
audio-in, so the pre-modulation hint gains a real carrier — an explicit
signal over MIDI/OSC/Link rather than inference, which is what Wagon and
Playhead's forward-cue requirement was always waiting for.

The socket is already open: `parse_boot_params(int, char**)` kept its full
signature through the sunset with both parameters unnamed, `main()` still
calls it with real `argc`/`argv`, and N1 restores the
`--seed/--mood/--cap/--msaa` loop behind it.
