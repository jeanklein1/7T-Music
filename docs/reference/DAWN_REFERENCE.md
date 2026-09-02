# Dawn — build reference

> ARCHIVAL WITH NAMED DRIFT (CANON, 2026-08): FXC is struck (L2, PIVOT_0)
> and the native arms are sunset (SUNSET_0-2) — sections describing FXC as
> the live compiler and native build trees describe a retired world.
> Rewrite pending Jean's reference round.

Documents the ARCHIVED native build (native-sunset, PIVOT_0). Kept as record; nothing here binds the web twin.

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
witness that the standalone copy is in use. The hazard is Visual
Studio's CMake DRIVER, not its shell: a Developer Command Prompt
APPENDS the IDE's toolchain directories — standalone cmake still
resolves first, verified 2026-08. The live statute is docs/LAWS.md L39.

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

> **REWRITTEN AT EMBER_0**, per this file's own "rewrite pending". The
> paragraph here described FXC as the live compiler; the lane of record is
> **Vulkan** (Tint→SPIR-V) and has been since PIVOT_0. What that paragraph
> got right is the configure line, and it is more load-bearing than it
> looked.

`Dawn build and use DXC: OFF` — the line this build still prints, and
EMBER_0's RECON.4/5/6 established what it costs. Dawn wraps
`EnsureDXCLibraries` — the only site in the D3D12 backend that opens
`dxcompiler.dll` and `dxil.dll` — in `#if defined(DAWN_USE_BUILT_DXC)`
with **no `#else`**, and `dawn_native` receives that define only inside
`if (DAWN_USE_BUILT_DXC)`. With the option OFF the loader is absent from
the library, so **the DXC lane cannot be reached by placing DLLs beside
the exe**: there is no code to reach. `C:/dev/dawn/third_party/dxc` is
also absent from disk, so turning the option on begins with a fetch, and
BOTH configurations must then be rebuilt — `dawn_lib_optional` in
7T-Music's `CMakeLists.txt` treats a library present in one config and
absent in the other as fatal. The whole route is scripted:
`tools/ember_route_a.py`.

**The two Windows compilers, as they stand.** FXC reaches the machine
through the system `d3dcompiler_47.dll`, which is present — so the FXC
lane's block is not availability but shader shape (L49, and the status
banner at `kCompilerPlan`). DXC's `dxcompiler.dll` is **not** in
`System32`, which is better news than it sounds: no ambient copy can
shadow a pinned one, so whatever loads DXC will be ours by construction.
FXC's speed is independent of Dawn's build configuration — a Release Dawn
makes Tint (WGSL→HLSL) much faster and does not move FXC at all — and the
FXC constraints of the retired floor are archived in
`docs/FXC_LAWS_RECORD.md`.

`D3D11` and `Vulkan` backends are ON, and **Vulkan is the lane of
record**; D3D12 stays linked and reachable through the adapter scorer. The
Vulkan backend enumerates Vulkan layers at boot, which is visible in the
console.
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
