# HANDOFF RELEASE_1 — the config axis, and symbols in Release

## GIT LAW
Trunk-based development on master only. Commit and push directly to master;
no transient `claude/*` branches. Separate commits per logical unit for
bisection. History lives in commits. CC starts shallow: run
`git fetch --unshallow` before any ancestry claim. The proxy blocks
`refs/tags` and branch deletion; Jean pushes tags from the design machine.

## ENCODING LAW
The entire tree is LF-only, no BOM anywhere. Every edit preserves this.

## CC REGISTER
REPORT findings; never improvise fixes. Verify every named anchor verbatim
before editing; STOP on mismatch. Comments describe present behavior only.

## CONTEXT (why this handoff exists)
Dawn now exists at `C:/dev/dawn/out` in exactly two configurations — Debug
and Release — built by standalone CMake 4.4.1, with `/Z7` debug information
embedded in the Release libraries. 7t's `CMakeLists.txt` hardcodes `Debug`
inside every Dawn library path, so 7t cannot link Release at all. This
handoff makes the config a build-time choice (`--config Release`), puts
symbols into 7t's own Release objects so crash stacks on foreign hardware
resolve end to end, and deletes what this campaign proved dead. All builds
and runs are Jean's gates; CC edits and commits only.

Scope: `CMakeLists.txt` at the repo root, plus the repo's Dawn reference
document if one exists. Nothing under `src/` changes.

---

# PHASE A — CENSUS (read-only; hard gates; zero edits)

## A1 — state
```
git fetch --unshallow    (if shallow)
git status
git log --oneline -3
```
Require: on master, clean working tree. REPORT the head commit.

## A2 — counts (the count law: every count has a disambiguating boundary)
On `CMakeLists.txt`:

- **Count A** = lines containing `dawn_lib(`   (open paren is the boundary;
  this does NOT match `dawn_lib_optional(` and does NOT match
  `function(dawn_lib ` — verify both non-matches explicitly)
- **Count B** = lines containing `dawn_lib_optional(`
- **Count C** = occurrences of `/Debug/` (slash-bounded, case-sensitive)
- **Count D** = occurrences of bare `Debug` NOT part of `/Debug/`
  (comments, strings) — report each with line number, no gate

**GATE A2: C == A + B exactly**, and every `/Debug/` occurrence sits on a
`dawn_lib(` or `dawn_lib_optional(` line with exactly one occurrence per
line. Advisory expectation: A + B lands in the 75–95 range. If C ≠ A + B,
REPORT the extra or missing sites verbatim with line numbers and STOP.

## A3 — anchor verification (each must appear exactly once, verbatim)
1. `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)`
   (NOTE: this line is live — it feeds compile_commands.json to the three
   Ninja trees under out/build/. Do not delete it, now or ever, on leanness
   grounds.)
2. The required-library function, exactly:
```
function(dawn_lib VAR PATH)
    set(FULL "${DAWN_BUILD}/${PATH}")
    if(NOT EXISTS "${FULL}")
        message(FATAL_ERROR "Missing Dawn library: ${FULL}")
    endif()
    set(${VAR} "${FULL}" PARENT_SCOPE)
endfunction()
```
3. The optional-library function, exactly:
```
function(dawn_lib_optional VAR PATH)
    set(FULL "${DAWN_BUILD}/${PATH}")
    if(EXISTS "${FULL}")
        set(${VAR} "${FULL}" PARENT_SCOPE)
        message(STATUS "  Found optional: ${PATH}")
    else()
        set(${VAR} "" PARENT_SCOPE)
    endif()
endfunction()
```
4. `set(MSVC_COMPILE_OPTS /W3 /wd4251 /wd4275 /wd4244 /wd4267 /EHsc)`
5. `target_compile_options(incubator_dual PRIVATE ${MSVC_COMPILE_OPTS})`
6. `target_compile_options(the_lab PRIVATE ${MSVC_COMPILE_OPTS})`
7. `target_compile_options(${PROJECT_NAME} PRIVATE ${MSVC_COMPILE_OPTS})`
8. `message(STATUS "  incubator_dual -> render: ${INCUBATOR_DUAL_RENDER_CARTRIDGE} analysis: ${INCUBATOR_DUAL_ANALYSIS_CARTRIDGE}")`
9. For each of the five names below, exactly TWO hits in the file — one
   `dawn_lib_optional(` call line and one bare entry inside the
   `foreach(lib ... )` OPTIONAL_LIBS block:
   `LIB_TINT_LANG_CORE_IR_BINARY`, `LIB_TINT_LANG_HLSL_VALIDATE`,
   `LIB_ABSEIL_LOW_LEVEL_HASH`, `LIB_ABSEIL_STATUS`, `LIB_ABSEIL_STATUSOR`.
   **Boundary hazard, named:** `LIB_ABSEIL_STATUS` is a prefix of
   `LIB_ABSEIL_STATUSOR`. Count `LIB_ABSEIL_STATUS` with a trailing
   delimiter (space, quote, or newline) so STATUSOR does not inflate it.

REPORT the file's total line count and the line number of every anchor
(ceiling values) before binding any edit. Any anchor count ≠ 1 (or ≠ 2 for
the A3.9 pairs): STOP.

## A4 — reads (REPORT verbatim; no edits in this handoff)
- `CMakePresets.json` — full contents.
- `CMakeSettings.json` — full contents.
- Locate the Dawn reference doc in the repo:
  `git ls-files | findstr /I dawn_reference` (or equivalent). REPORT the
  path(s), or "absent from repo". These reads decide Phase F and a later
  tree-consolidation ruling; presets are not edited here.

**GATE A: all of A1–A3 pass exactly → proceed to Phase B without waiting.
Any mismatch → STOP, report, no edits.**

---

# PHASE B — COMMIT 1: the config axis (atomic)

All four edits in ONE commit. A partial state makes the guard fire on the
first alphabetical path instead of describing the defect.

## B1 — declare the config set (insert AFTER anchor A3.1)
```
# ── CONFIG AXIS ──────────────────────────────────────────────────────────────
# 7t links Dawn's prebuilt static libraries, which exist in exactly two
# configurations: Debug and Release. The config set here mirrors that fact.
# Multi-config generators (Visual Studio) choose per build via --config;
# single-config generators (Ninja trees) choose per tree via CMAKE_BUILD_TYPE.
# DAWN_CHECK_CONFIGS is the list the library guards verify at configure time.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING
        "Configs provided by the Dawn build" FORCE)
    set(DAWN_CHECK_CONFIGS Debug Release)
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Build type: Debug or Release (the configs Dawn provides)" FORCE)
    endif()
    if(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
        message(FATAL_ERROR
            "CMAKE_BUILD_TYPE '${CMAKE_BUILD_TYPE}' — Dawn provides only Debug and Release")
    endif()
    set(DAWN_CHECK_CONFIGS ${CMAKE_BUILD_TYPE})
endif()
```

## B2 — REPLACE the dawn_lib function (FIND = anchor A3.2, count 1)
```
# Verifies the library exists for every config in DAWN_CHECK_CONFIGS at
# configure time; the $<CONFIG> in PATH resolves per build.
function(dawn_lib VAR PATH)
    foreach(CFG ${DAWN_CHECK_CONFIGS})
        string(REPLACE "$<CONFIG>" "${CFG}" CONCRETE "${DAWN_BUILD}/${PATH}")
        if(NOT EXISTS "${CONCRETE}")
            message(FATAL_ERROR "Missing Dawn library: ${CONCRETE}")
        endif()
    endforeach()
    set(${VAR} "${DAWN_BUILD}/${PATH}" PARENT_SCOPE)
endfunction()
```
If the original carries a `# Function to find a required Dawn library`
comment line directly above, replace that line's text with the new comment
above; do not stack old and new comments.

## B3 — REPLACE the dawn_lib_optional function (FIND = anchor A3.3, count 1)
```
# An optional present in some configs but not all is an error: the optional
# set must be identical across configs, or the link truth differs per config.
function(dawn_lib_optional VAR PATH)
    set(FOUND_ALL TRUE)
    set(FOUND_ANY FALSE)
    foreach(CFG ${DAWN_CHECK_CONFIGS})
        string(REPLACE "$<CONFIG>" "${CFG}" CONCRETE "${DAWN_BUILD}/${PATH}")
        if(EXISTS "${CONCRETE}")
            set(FOUND_ANY TRUE)
        else()
            set(FOUND_ALL FALSE)
        endif()
    endforeach()
    if(FOUND_ALL)
        set(${VAR} "${DAWN_BUILD}/${PATH}" PARENT_SCOPE)
        message(STATUS "  Found optional: ${PATH}")
    elseif(FOUND_ANY)
        message(FATAL_ERROR
            "Optional Dawn library exists in some configs but not all: ${PATH}")
    else()
        set(${VAR} "" PARENT_SCOPE)
    endif()
endfunction()
```
Same comment rule as B2 for any existing comment line above it.

## B4 — the substitution (after B2/B3, so the functions contain no /Debug/)
Global, whole-file: `/Debug/` → `/$<CONFIG>/`
Legal only because GATE A2 proved every `/Debug/` sits on a dawn_lib* call
line. Post-edit verification, both required:
- occurrences of `/Debug/` in the file == 0
- occurrences of `/$<CONFIG>/` in the file == A + B (the A2 counts)
Mismatch → `git checkout -- CMakeLists.txt`, REPORT, STOP.

## B5 — commit
```
cmake: config axis — Dawn paths bind $<CONFIG>; guards verify Debug+Release
```

---

# PHASE C — COMMIT 2: symbols in Release (7t side)

Dawn's Release libs already carry /Z7. This puts /Z7 into 7t's own Release
objects and makes the final link produce a PDB while keeping Release
codegen identical to a symbol-less Release (/OPT:REF /OPT:ICF restored;
/DEBUG alone would silently disable them).

## C1 — REPLACE anchor A3.4 with:
```
set(MSVC_COMPILE_OPTS /W3 /wd4251 /wd4275 /wd4244 /wd4267 /EHsc
    "$<$<CONFIG:Release>:/Z7>")
# Release links produce a full PDB with codegen unchanged: /OPT:REF and
# /OPT:ICF are the plain-Release defaults that /DEBUG would otherwise
# disable. The PDB never ships; it is archived beside the tagged exe.
set(MSVC_LINK_OPTS
    "$<$<CONFIG:Release>:/DEBUG:FULL>"
    "$<$<CONFIG:Release>:/OPT:REF>"
    "$<$<CONFIG:Release>:/OPT:ICF>"
    "$<$<CONFIG:Release>:/INCREMENTAL:NO>")
```

## C2 — after each of anchors A3.5, A3.6, A3.7 (inside their existing
if(MSVC) blocks), insert the matching line:
```
target_link_options(incubator_dual PRIVATE ${MSVC_LINK_OPTS})
target_link_options(the_lab PRIVATE ${MSVC_LINK_OPTS})
target_link_options(${PROJECT_NAME} PRIVATE ${MSVC_LINK_OPTS})
```
(Each after its own target's compile-options line. The probe/check targets
link no Dawn and take no symbols change.)

## C3 — commit
```
cmake: /Z7 + /DEBUG:FULL(+OPT:REF,ICF) on Release for the Dawn-linking targets
```

---

# PHASE D — COMMIT 3: the demo enters the banner

REPLACE anchor A3.8 with:
```
message(STATUS "  incubator_dual -> render: ${INCUBATOR_DUAL_RENDER_CARTRIDGE} analysis: ${INCUBATOR_DUAL_ANALYSIS_CARTRIDGE} demo: ${THE_BOARD_DEMO}")
```
One line, one commit:
```
cmake: banner names THE_BOARD_DEMO — the demo column travels with every configure
```

---

# PHASE E — COMMIT 4: five never-built optionals deleted

For each of the five A3.9 names: delete the entire `dawn_lib_optional(...)`
call line AND the entire bare-name line inside the `foreach(lib ...)` block.
Ten line deletions total. These libraries exist in neither Dawn config;
their variables resolve empty and contribute nothing — this is dead-line
deletion with zero link-behavior change. Respect the STATUS/STATUSOR
boundary from A3.9.

Post-edit: each of the five names occurs 0 times in the file.
Commit:
```
cmake: delete five optional Dawn libs no config builds
```
(NOT touched here: `tint_utils_bytes` — it is in the REQUIRED list and is a
held post-link experiment, Jean's gate.)

---

# PHASE F — COMMIT 5: the Dawn reference doc (conditional on A4)

If A4 found no Dawn reference in the repo: skip, REPORT "F skipped — doc
not in repo." (Copies in the Claude project knowledge are Jean's to
refresh, not CC's.)

If present, edit that file:
1. Build Environment table: Configuration row → `Debug + Release (/Z7 in Release)`.
   Add row: `CMake | 4.4.1 standalone — C:\Program Files\CMake\bin (NOT the
   VS-bundled copy; that copy moves with IDE updates and orphans build trees)`.
2. Build Commands section → replace with the current truth:
```powershell
cd C:\dev\dawn
cmake -S . -B out -G "Visual Studio 18 2026" -A x64 ^
    -DDAWN_FETCH_DEPENDENCIES=ON ^
    -DDAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON ^
    -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG /Z7" ^
    -DCMAKE_C_FLAGS_RELEASE="/O2 /Ob2 /DNDEBUG /Z7"
cmake --build out --config Release --target dawn_native dawn_glfw dawn_proc dawn_system_utils tint_utils_bytes --parallel
cmake --build out --config Debug   --target dawn_native dawn_glfw dawn_proc dawn_system_utils tint_utils_bytes --parallel
```
3. Global in this file: `/Debug/` → `/<CFG>/`, and one sentence above the
   Library Reference tables: "`<CFG>` is `Debug` or `Release`; both exist."
   REPORT the replacement count.
4. Delete any per-file BOM/CRLF-era or otherwise superseded config notes
   encountered — delete rather than annotate.
Commit:
```
docs: DAWN_REFERENCE — two-config paths, /Z7 Release, standalone CMake 4.4.1
```

---

# PHASE G — push and final REPORT
`git push` (direct to master). REPORT: five (or four, if F skipped) commit
hashes, the A2/A3 census table, the B4 verification counts, and the A4
verbatim reads (both JSON files + doc location). No builds attempted.

---

# JEAN'S GATES (after CC lands — none of these are CC's)

1. **Configure** (plain cmd, `cmake --version` → 4.4.1 first):
   `cmake -S . -B build` from `C:\dev\7t`.
   Expect: guard passes both configs; ten "Found optional" lines; banner
   shows `demo: full`. A FATAL_ERROR here names a real path defect — paste it.
2. **The gate**: `cmake --build build --config Release --target incubator_dual --parallel`
   First Release link ever. Unresolved externals would name a genuinely
   missing library — paste, don't guess.
3. **Run** `full`, machine-clean, METER_1 table. Expectations: CPU rows
   collapse hard; GPU `main_pass`/`shadow_pass` roughly hold (that
   asymmetry re-confirms geometry-bound); boot FXC unchanged — Tint got
   faster, the FXC cliff did not move. NDEBUG is live: any assert with a
   side effect is now a behavior change — suspect 7t before Dawn if
   anything misbehaves.
4. **Regression**: `cmake --build build --config Debug --target incubator_dual --parallel` still links and boots.
5. **Archive discipline**: tag the state; keep `incubator_dual.exe` +
   `incubator_dual.pdb` together with the tag name. The PDB never ships,
   never dies.
6. **Cleanup, only after 2–4 pass**: delete `C:\dev\dawn\out.cmake41` and
   `C:\dev\7t\build.cmake43`. Still open, Jean's call: pin Visual Studio's
   CMake to the standalone 4.4.1 (the three Ninja trees remain on the
   IDE's 4.3 and die at its next update); `x64-Debug` tree's fate after
   the A4 presets read; PATH Move-Up of `C:\Program Files\CMake\bin`.
7. **Held experiments, unblocked by gate 2**: remove `tint_utils_bytes`
   from the REQUIRED list and see whether the link still closes; the
   `webgpu_dawn` monolithic-library consolidation (80 paths → ~3) as its
   own future campaign; `the_lab` Release and `minimal` Release
   (`-DTHE_BOARD_DEMO=minimal`, remembering the cache holds it until
   flipped back) — ideal, not gating.
