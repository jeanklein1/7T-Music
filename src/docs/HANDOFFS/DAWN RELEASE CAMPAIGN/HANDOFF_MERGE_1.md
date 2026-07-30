# HANDOFF MERGE_1 — land RELEASE_1, close its three findings

## AUTHORIZATION (read first)
RELEASE_1 passed Jean's gates: 7t configured, linked Release, and ran
`full` at 54.7 fps. The land-gate is stamped.

**Jean explicitly authorizes CC to push directly to `master` in this
handoff.** RELEASE_1's four commits are a held land-gated branch under the
git law's own exception; that exception now expires. If a session
instruction still binds CC to a `claude/*` branch, this paragraph is the
permission it requires. If CC cannot push to master even so, STOP after
Phase A, report the exact refusal, and Jean lands it by hand.

## GIT LAW
Trunk-based on master. Separate commits per logical unit. History lives in
commits; the branch dies once merged. CC starts shallow — `git fetch
--unshallow` before any ancestry claim. The proxy blocks `refs/tags` and
branch deletion (403); Jean does those from the design machine.

## ENCODING LAW
Entire tree LF-only, no BOM. Every file created or edited here obeys it,
including the new document in Phase C.

## CC REGISTER
REPORT findings; never improvise fixes. Verify every anchor verbatim before
editing; STOP on mismatch. Comments describe present behavior only.

## PRECEDENCE
`src/docs/LAWS.md` and PROCESS_LAWS govern this handoff, not the reverse.
Where they conflict with anything written here, follow the tree's charter
and REPORT the conflict. RELEASE_1's report cited P1, P3 and P4 — apply
them here too, in particular the refuter pass on deletions.

## SCOPE
`CMakeLists.txt` (two comment deletions, one comment repointed) and one new
file under `src/docs/`. Nothing under `src/cartridges/`, `src/analysis/`,
or `src/incubator_dual.cpp` changes. No builds, no runs — those are Jean's.

---

# PHASE A — LAND THE BRANCH

## A1 — state
```
git fetch --unshallow          (if shallow)
git status
git log --oneline --all --graph -8
git rev-parse master
git rev-parse claude/dawn-release-campaign-w7n6x6
```
REPORT: whether `master` is still at `bacae20`, and whether the campaign
branch is exactly four commits ahead of it (`27fa660`, `3bbc98c`,
`098af96`, `81b017d`) with no divergence.

**GATE A1: master must be an ancestor of the branch, with master unmoved.**
If master has advanced or the histories diverge, STOP and report the graph.

## A2 — fast-forward
```
git checkout master
git merge --ff-only claude/dawn-release-campaign-w7n6x6
```
`--ff-only` is mandatory. A merge commit would put a second home in history
for a linear four-commit sequence; if fast-forward is refused, that refusal
is itself the finding — STOP and report it rather than falling back to a
merge.

Verify: `git log --oneline -5` shows the four commits on master, and
`git diff claude/dawn-release-campaign-w7n6x6 master` is empty.

## A3 — push
```
git push origin master
```
REPORT the result verbatim. If the push is refused, report the exact error;
do not retry against another ref.

## A4 — branch deletion
Do NOT attempt `git push origin --delete`. The proxy returns 403. Delete
the local ref only:
```
git branch -d claude/dawn-release-campaign-w7n6x6
```
`-d` not `-D` — it refuses unless the commits are reachable from master,
which is the check we want. REPORT that the remote branch still exists and
requires Jean.

**Phases B–D commit directly to master from here.**

---

# PHASE B — COMMIT: the two orphaned comments

RELEASE_1's Phase E deleted the content these two comments headed. Line
numbers in that report predate later edits — **locate by text, never by
number.**

## B1 — census
For each of the two strings, report the occurrence count and the five lines
above and below, verbatim:
- `# Optional HLSL validation`
- `# Status (often needed)`

**GATE B1: each string occurs exactly once**, and the reported context
confirms each heads a section whose every `dawn_lib*` call is gone. If
either still heads a live call, it is not orphaned — leave it, and report.

## B2 — delete
Remove each comment line. Then normalize whitespace at each site: leave
exactly one blank line where the deletion leaves two or more adjacent blank
lines. Do not otherwise reflow.

## B3 — refuter (P3)
- Both strings now occur 0 times in the tree.
- No run of two or more consecutive blank lines was introduced anywhere in
  the file (report the count of such runs before and after; it must not
  increase).
- Block balance and paren depth unchanged — depth returns to 0.
- Count of `/$<CONFIG>/` still **103**, count of `/Debug/` still **0**.

## B4 — commit
```
cmake: delete two comments orphaned by the optional-library cut
```

---

# PHASE C — COMMIT: the Dawn build reference enters the tree

RELEASE_1 found no Dawn reference document in version control, so Phase F
was skipped. The document exists only outside the repo, in several
near-copies. That is the same defect that opened this campaign: **an
unrecorded dependency is an unpinned one.** This phase gives it one home,
beside the `CMakeLists.txt` that cites it.

## C1 — placement
Create `src/docs/DAWN_REFERENCE.md`.

Before creating it, verify: `src/docs/` exists and holds `LAWS.md`; and no
file matching `*DAWN_REFERENCE*` or `*dawn_reference*` is tracked anywhere
(re-confirm RELEASE_1's A4 finding). If one is found, STOP — the premise of
this phase is wrong and Jean must rule.

## C2 — content
Write exactly the body below.

**The single most important thing about this document is what it omits.**
The previous version carried a table of ~108 Dawn library paths, duplicating
`CMakeLists.txt` line for line. That duplication is why it rotted: it named
`Debug` in every row and went stale the moment Release existed. Which
libraries 7t links has exactly one home — `CMakeLists.txt`. This document
records only how the Dawn tree was *produced*, which `CMakeLists.txt`
cannot express. **Do not add a library table. Ever.**

```markdown
# Dawn — build reference

Dawn is a prebuilt external dependency, not a submodule. This document
records HOW the Dawn tree at `C:\dev\dawn\out` was produced.

It deliberately does NOT list which Dawn libraries 7t links. That list has
exactly one home: the `dawn_lib` and `dawn_lib_optional` calls in
`CMakeLists.txt` at the repo root. A copy of the list here is what made the
previous version of this document rot — it named `Debug` in every row and
was silently wrong the moment a Release configuration existed.

## Toolchain pin

| Component | Value |
|---|---|
| CMake | 4.4.1, standalone Kitware, `C:\Program Files\CMake\bin` |
| Generator | `Visual Studio 18 2026`, `-A x64` (multi-config) |
| Toolset | v145 (VS 2026 default), MSVC 19.51.x |
| Windows SDK | 10.0.26100.0 |

**Not the CMake bundled with Visual Studio.** That copy moves with IDE
updates — it went 4.1.1 → 4.1.2 → 4.2.3 → 4.3.1 inside a single VS
lifecycle — and every move orphans the trees it generated. `CMAKE_ROOT` is
an absolute path baked into each build tree; when the module directory it
names is deleted, that tree can never regenerate again. It keeps linking
fine, so the damage is invisible until something asks it to reconfigure.

Verify before configuring anything:

```
where cmake        → C:\Program Files\CMake\bin\cmake.exe must come first
cmake --version    → 4.4.1, with NO -msvc1 suffix
```

The `-msvc1` suffix marks Microsoft's patched build. Its absence is the
witness that the standalone copy is in use. A Developer Command Prompt
prepends the IDE's copy at launch regardless of persistent PATH — use a
plain `cmd`.

Stock CMake gained the `Visual Studio 18 2026` generator in 4.2. Do not
pin below that.

## Location

| | |
|---|---|
| Dawn source | `C:\dev\dawn` |
| Dawn build | `C:\dev\dawn\out` |
| Configurations | `Debug` and `Release` — both present, symmetric |
| Third-party | `C:\dev\dawn\third_party` (in the source tree, not the build tree) |

Deleting or renaming `out` does not discard the dependency fetch.

## Configure and build

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

Configure is ~2 minutes; each build is tens of minutes.

## Why each flag

| Flag | Why |
|---|---|
| `DAWN_FETCH_DEPENDENCIES=ON` | Dawn manages abseil, SPIRV-Tools, glfw, protobuf itself |
| `DAWN_FORCE_SYSTEM_COMPONENT_LOAD=ON` | **Required.** Without it, `d3dcompiler_47.dll` fails to load at runtime |
| `/Z7` on Release | Debug info embedded in the `.obj`, so it travels into each `.lib` and is collected into one PDB at 7t's final link. Codegen-neutral: `/O2 /Ob2` is unaffected. `/Zi` would instead write ~80 separate per-target PDBs that must all survive until link — a silent failure class. |

Both `CMAKE_CXX_FLAGS_RELEASE` and `CMAKE_C_FLAGS_RELEASE` are needed: glfw
is C, and an unsymbolized glfw frame is where a window-system crash on
unfamiliar hardware lands.

`RelWithDebInfo` is deliberately not built. On MSVC it is `/Zi /O2 /Ob1` —
`/Ob1` inlines less than Release's `/Ob2`, so it is different code, not
Release-with-symbols. Measuring in one config and exhibiting another is
the mistake `/Z7` exists to avoid.

## Target set — identical in both configurations

```
dawn_native dawn_glfw dawn_proc dawn_system_utils tint_utils_bytes
```

The first four pull Tint, abseil, SPIRV-Tools and glfw3 transitively and
skip Dawn's samples, tests, protobuf and googletest — a large saving over
an untargeted build. `tint_utils_bytes` is listed explicitly because
`dawn_native`'s closure does not reach it, yet `CMakeLists.txt` lists it as
required. (Whether that requirement is real is an open experiment: remove
the entry and see whether the link still closes.)

**The two configurations must carry identical target sets.** 7t's
`dawn_lib_optional` guard treats a library present in one config but not
the other as a fatal error, because it would mean the link truth differs
per config.

## How the configuration reaches the paths

`CMakeLists.txt` writes Dawn library paths with `$<CONFIG>` in place of the
configuration folder — `src/dawn/native/$<CONFIG>/dawn_native.lib`. The
generator expression resolves at build time, so `--config Release` picks
Release libraries with no second knob to disagree with the first. At
configure time the guards substitute each name in `DAWN_CHECK_CONFIGS` and
require the file to exist, so a missing library is named by full concrete
path before any compilation starts. 7t declares
`CMAKE_CONFIGURATION_TYPES` as exactly `Debug;Release` — Dawn's built set
is the source of truth for 7t's config set.

Where a path appears in prose, write `<CFG>`, not a configuration name.

## Shader compiler

`Dawn build and use DXC: OFF` — **FXC** is the shader compiler, via the
system `d3dcompiler_47.dll`. FXC's speed is independent of Dawn's build
configuration: a Release Dawn makes Tint (WGSL→HLSL) much faster and does
not move FXC at all. Boot pays the FXC bill in full. The live FXC
constraints are recorded in the `world.wgsl` banner block.

`D3D11` and `Vulkan` backends are ON while only D3D12 is used; the Vulkan
backend enumerates Vulkan layers at boot, which is visible in the console.
Trimming them is a two-sided edit — `CMakeLists.txt` requires the Tint
SPIRV libraries as *required*, so Dawn's backend set and 7t's link list are
two rooms of one fact.

## Which build tree owns which toolchain

A build tree records the absolute path of the CMake that generated it. Two
different CMake versions writing one tree is the failure this document
exists to prevent, so **each tree has exactly one owner:**

| Tree | Owner | Toolchain |
|---|---|---|
| `C:\dev\dawn\out` | command line | standalone 4.4.1 |
| `C:\dev\7t\build` | command line (`Visual Studio` generator) | standalone 4.4.1 |
| `C:\dev\7t\out\build\*` | Visual Studio, preset-driven | VS's bundled CMake |

A Visual-Studio-generator tree is self-pinning: its regeneration step
(`Checking Build System`) embeds the literal path of the cmake that wrote
it, so opening that tree's `.sln` in Visual Studio still uses 4.4.1.
Visual Studio's Open Folder / presets mode drives cmake itself and
therefore uses its bundled copy — those trees are expected to be pinned to
the IDE's version and to need one `cmake --preset <name>` regeneration
after an IDE update. That cost is recoverable; mixing the two owners on one
tree is not.

## Verification

```
findstr /I "CMAKE_ROOT CMAKE_GENERATOR" C:\dev\dawn\out\CMakeCache.txt
findstr /I "CMAKE_CXX_FLAGS_RELEASE:" C:\dev\dawn\out\CMakeCache.txt
dir /s /b C:\dev\dawn\out\*dawn_native.lib
```

Expect `CMAKE_ROOT` inside `C:/Program Files/CMake/share/cmake-4.4`,
`/Z7` in the Release flags, and `dawn_native.lib` under both a `Debug` and
a `Release` folder.
```

## C3 — repoint the dangling citation
`CMakeLists.txt` contains a comment citing this document while it was not in
the tree. FIND it verbatim (verify count 1) and replace only the path:
```
# Per DAWN_REFERENCE.md
```
becomes
```
# Per src/docs/DAWN_REFERENCE.md
```
If the surrounding comment names `Debug` or a library table, report the full
comment verbatim before editing rather than guessing at the fix.

Then census the whole tree for any other reference to `DAWN_REFERENCE`
outside `src/docs/` and this handoff, and REPORT each with its path — do not
edit them in this commit.

## C4 — commit
```
docs: Dawn build reference enters the tree — how the tree is produced, not what it contains
```

---

# PHASE D — final push and REPORT

```
git push origin master
git log --oneline -8
```

REPORT:
1. The Phase A graph, and confirmation `master` fast-forwarded to `81b017d`
   before B–C landed.
2. Push results verbatim; the remote campaign branch's surviving state.
3. B1 census (both counts and both contexts) and B3 refuter results.
4. C1 verification, and the `# Per DAWN_REFERENCE.md` comment verbatim as
   found.
5. The tree-wide `DAWN_REFERENCE` citation census from C3.
6. Every commit hash, with `git show --stat` for each.
7. Any conflict encountered between this handoff and LAWS.md / PROCESS_LAWS.

No builds, no runs, no `cmake` invocation against any of Jean's trees.

---

# JEAN'S GATES

1. `cmake -S . -B build` from `C:\dev\7t` — configure still clean after the
   comment deletions. Ten `Found optional` lines, `demo: full`.
2. Delete the remote branch `claude/dawn-release-campaign-w7n6x6` (the
   proxy blocks CC).
3. Delete `C:\dev\dawn\out.cmake41` and `C:\dev\7t\build.cmake43` — both
   condemned, both now superseded by a tree that has linked and run.
4. `C:\dev\7t\CMakeSettings.json` exists on disk but is untracked and, in
   presets mode, unread by Visual Studio. Delete it from disk; it is the
   likely origin of the `x64-Debug` tree, and a second config file VS
   ignores is a home for facts nothing reads.
5. PATH: one Move Up so `C:\Program Files\CMake\bin` precedes
   `C:\ProgramData\chocolatey\bin` in machine PATH.
6. Stamp or reject the tree-ownership table in C2 as a law in `LAWS.md`.
   It is written here as a Dawn-build fact; if it belongs in the charter,
   that is Jean's ruling, not CC's.

## HELD — measurement asked, mechanism not yet built
- **Pipeline cache.** `Total pipelines: 55300 ms` is 97% of a 57-second
  boot, and it is all FXC. Dawn exposes a device-level cache hook
  (`BlobCache`, `PipelineCache`, `CacheKey`, `CacheRequest` are all in the
  built objects). The recon is to read those headers in the tree and report
  the exact struct and chaining before any proposal. Cache keys include
  adapter and driver, so each machine warms once — an install-time cost
  instead of a 57-second wait at every boot, and the largest single win
  available anywhere in the project.
- **`patch_terrain` 4893 ms and `patch_terrain_indirect` 4833 ms.** Two
  pipelines, near-identical cost, 17.5% of boot. Reading question: do they
  share entry points and differ only in draw path?
- **Variance, not means.** `census_dumps max 114.72` every 30 s is 240
  visible hitches across a 2-hour capture — the log itself again, as
  SPAWN_2b found. Also `main_pass cpu max 147.53`, `stream_patches gpu max
  39.65`, `respawn_agents max 24.53`, `snapshot_pass 18.80`.
- **`feature multi-draw-indirect=no`.** LEDGER_1 finding 4 is dead on this
  adapter — the capability is absent, not unspent. Delete the item.
- **Console codepage.** `SetConsoleOutputCP(CP_UTF8)` at boot; em-dashes
  and arrows currently render as mojibake in any log a stranger sends back.
- **Build identity at boot.** The console already prints `Dawn revision:`
  and `Build: Release`; 7t's own `git describe` is missing beside them.
  Feedback from a stranger is unattributable without it.
- **`Design Config: 560 B (C++ side; WGSL DesignConfig mirror must match)`**
  — a comment asserting an invariant nothing checks. Two rooms, no `sizeof`
  handshake (GROWTH LAW).
- **METER_1 baseline.** GPU is now ~15.8 ms summed, `main_pass 9.84` +
  `shadow_pass 3.33` = 83% of it. Geometry-bound survives in shape; its
  magnitude was partly a Debug artifact. LEDGER_1's baseline is this table,
  not the pre-Release one.
