Reading the tree before any edit — the recon and the three gates.

Recon anchors — re-running with correct regex (my first pass used `-F`, which made `^` literal):

All three gates pass. Recording and executing.

U2 — the preset ancestry. Note: the handoff says replacement content is "supplied alongside," but no content arrived with the message; the prose specifies the ancestry completely, so I'm authoring it from that spec and reporting it as a RESOLVE.

cmake 3.28.3 is present — I can run the U2 acceptance directly (listing presets only; no configure, no build):

Acceptance met exactly — 4 presets without EMSDK, 5 with.

Push rejected — the remote moved. Fetching to see what landed:

Jean pushed `0b07c6c "CMake json updated"` — touching the same file U2 rewrote. Reading it before I merge anything:

Jean's commit contains the "replacement content" the handoff referenced. Comparing it against what I authored:

My authored file is **byte-identical** to Jean's supplied replacement — the RESOLVE reconstructed it exactly. Rebasing onto his commit; U2 will go empty, so I'll keep it as the unit's binding record:

U2 is retained as an empty commit. Rewriting its message to record honestly that Jean's push landed the identical content first:

# PORT_1d — THE BUILD CROSSES THE GUARD — campaign report

**Four units, all EXECUTE, no STOP. Pushed: `0b07c6c..7be03c7`.** Every expected line number in the handoff was wrong (the tree had moved), so everything below was bound by content.

| unit | commit | file |
|---|---|---|
| U1 dial above the guard | `c006e94` | `CMakeLists.txt` |
| U2 preset ancestry | `30912ff` (empty — see below) | `CMakePresets.json` |
| U3 artifacts ignored | `c676f71` | `.gitignore` |
| U4 message truth | `7be03c7` | `CMakeLists.txt` |

## RECON — anchors, quoted

```
38:  if(NOT EMSCRIPTEN)
488: endif() # NOT EMSCRIPTEN — end of the native Dawn section
270: # ACTIVE CARTRIDGE SELECTION
285: set(INCUBATOR_DUAL_RENDER_CARTRIDGE   "the_board"  CACHE STRING …)
286: set(THE_BOARD_DEMO                    "full"       CACHE STRING …)
287: set(T7_INSTRUMENTS                    "off"        CACHE STRING …)
19:      set(DAWN_CHECK_CONFIGS Debug Release)
29:      set(DAWN_CHECK_CONFIGS ${CMAKE_BUILD_TYPE})
```

The premise holds exactly: the block sat at 269–293, **inside** 38…488. Your diagnosis of the failure mode is confirmed — `T7_INSTRUMENTS=` arrives defined-and-empty, `#ifndef` never fires, the paste names a nonexistent enumerator. I found **two more consumers** failing the same way, both fixed by the same move: the header `file(GLOB_RECURSE)` collapsed to `src/cartridges//*.hpp` (matching nothing), and the `source_group(TREE …)` root.

## R1 — the interval read: CLEAN

Reads of the three dials across the whole file: 285–287 (the sets), 291–292 (the status lines, inside the block itself), then 507, 515, 536–538, 561–563, 592 — **every one at or after the guard's `endif()` at 488**. Zero reads in the vacated interval [31, 284]. The file contains **no** `if(DEFINED …)`, no `STREQUAL`, and no `NOT`-test against any of the three, anywhere. Nothing could observe the earlier declaration; the move is inert for native.

## R2 — the web target name: AGREES, no correction needed

One `add_executable(incubator_dual …)` at 518, **outside** the guard — the target is `incubator_dual` on both twins. The Emscripten branch only sets properties on it: `OUTPUT_NAME "the_board"`, `SUFFIX ".js"`. The build preset's `"targets": [ "incubator_dual" ]` was already correct, so no RESOLVE fired and build presets stayed untouched.

## R3 — the output directory: U3 FIRES

`RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/web"`. `web/index.html` exists, is tracked, and loads `<script src="the_board.js">`. `git check-ignore` confirmed all three artifacts were reachable by `git add` — `[Oo]ut/`, `build/`, `out/build/` never covered them.

## RESOLVEs taken

- **U1 insertion point** (granted): after the CONFIG AXIS `endif()`, **above** the DAWN CONFIGURATION banner — the banner documents the guard and belongs with it. Moved by byte-slicing the file, never retyped: the U+2550 runs (`e2 95 90`) are preserved, and the block travelled whole, so the web configure now prints the same `Active cartridges:` report.
- **U2 replacement content**: the handoff said it was "supplied alongside," but nothing accompanied the message. Rather than STOP, I authored it from the prose spec (which names every preset, parent, and payload). **Mid-round you pushed `0b07c6c` carrying the real file — my version is byte-identical to yours, `diff` clean.** The rebase left U2 with nothing to apply; I kept it as an empty commit so the unit's bindings and acceptance survive, and its subject says so plainly.

## Acceptance, executed here (cmake 3.28.3, listing only — no configure, no build)

```
cmake --list-presets   without EMSDK → 4 presets, the-board-web ABSENT
cmake --list-presets   with EMSDK    → 5 presets, the-board-web present
```

Native presets were also resolved through the new ancestry field-by-field: all four keep exactly their previous effective cache sets. `the-board-web` is the only one that changes, by exactly the two `DAWN_*` variables it should never have had.

## Your build path, unambiguous

- **target** `incubator_dual` → **output** `web/the_board.js` + `.wasm` + `.data` → **ignored** by the three new `.gitignore` lines; `web/index.html` stays tracked and already loads `the_board.js`.
- `cmake --preset the-board-web && cmake --build --preset the-board-web`, then serve `web/` and open it.

## Recorded, not fixed

- A build preset is **not** disabled by its configure preset's condition — `--list-presets=build` shows `the-board-web` in both shells. Harmless (no binaryDir without a configure), and touching it would exceed U2's radius.
- The CONFIG AXIS **header comment** and the `CMAKE_BUILD_TYPE` cache docstring still carry the native-only "configs Dawn provides" framing. U4's radius was the message string alone; a comment-truth pass can take all three together.
- Preset restatement (`the-board` duplicating dials the CMakeLists now supplies to both twins) stands as you recorded it — behavior-neutral, left for a leanness pass.

One thing I did not do: **nothing here was compiled.** U1 removes the only *known* web compile-stopper, but the first real Emscripten build is still the first time this tree meets clang — `glaw1` proves native is unchanged, and the web configure+build is the gate that finds what's next.