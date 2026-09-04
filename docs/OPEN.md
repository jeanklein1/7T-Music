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

### THE CLAUSE FIRED (EMBER_0) — and the answer is a witness, not a hunt

The condition above is met. On the certified MSBuild lane,
`dir /s /b out\build\the-board-full-release\*.dll` returned **File Not
Found** with the tree present — no `dxcompiler.dll`, no `dxil.dll` — after
a build whose log printed both post-build COMMENT lines. The Ninja tree
holds both. So the entry's central claim, *"the essentials land anyway"*,
is **false for the DXC half on the VS lane**, and it was believed because
a COMMENT prints when a step is REACHED, not when it succeeds.

**Consequence today: nil.** EMBER_0's RECON.4 proved the pair is inert at
this Dawn build regardless (see EMBER_0 below), so nothing was lost. The
class is real all the same, and it becomes load-bearing the moment route
(a) lands.

**What was done about it, and what was not.** Not a hunt through MSBuild's
log: the last POST_BUILD step now prints a manifest of what is actually
beside the exe, so a silent non-landing reads as a visible `ABSENT` line
(`tools/post_build_manifest.cmake`). It reports and never fails. The
*cause* is still undiagnosed and still not this tree's — the three
negative probes above stand — and `assets/` was never checked on that
lane, which the manifest now answers on every build. **Open for Jean:**
one `dir /b out\build\the-board-full-release\Release` says whether
`assets\` landed there too; the next VS build answers it for free.

## EMBER_0 — THE WINDOWS COMPILER LANES (open; the rebuild waits on Jean)

Three values in `kCompilerPlan`, three statuses, all now written at the
constant per **L49**. The resident plan is **Vulkan** and it carries the
music. What EMBER_0 settled, by recon rather than assumption:

| lane | status | the block, named |
|---|---|---|
| `Vulkan` | **SUPPORTED** | resident; Tint→SPIR-V; the boot of record |
| `D3D12_Dxc` | **NOT YET TRUE** | Dawn at pin `56f332d7` wraps `EnsureDXCLibraries` — the only site opening `dxcompiler.dll`/`dxil.dll` — in `#if defined(DAWN_USE_BUILT_DXC)` with **no `#else`**, and `C:/dev/dawn/out` is built with that option `OFF`. The loader is absent from the linked library; no DLL placement reaches code never compiled |
| `D3D12_Fxc` | **BLOCKED — shader shape** | six uniform blocks carry an array of structs subscripted by a non-constant expression. Worst: `TileGrid`, `array<TileGridEntry, 1024>`, **16,400 B** |

**RULING.4, and its anchor is not the one the handoff was written with.**
The FXC block was stamped on WALLET_0's occupier windows — and those left
the tree at PRUNE_2 U4 (`occupier_cmg`) and ONE_WORLD-I U3
(`occupier_amg`) before EMBER executed. The RECON.8 **refresh**
re-established the finding at master against the cliff's mechanism rather
than its old address, surveying all fourteen `var<uniform>` blocks with
every claim adversarially refuted by an independent reader:

| block | bytes | the array |
|---|---|---|
| `TileGrid` | 16,400 | `array<TileGridEntry, 1024>`, read at `tile_grid.entries[lz * s + lx]`, `s` runtime |
| `SceneConstants` | 4,336 | `array<PawnFigure, 14>` @ 288 B, indexed by a storage-derived skin id in `pawn_vs` **and** `shadow_pawn_vs` |
| `DesignConfig` | 672 | `pulse_data[8]` + two palette arrays |
| `AgentRoomConstants` | 512 | `behaviors[10]` + `tier_gains[4]` |
| `PyramidArray` | 272 | `instances[8]` |
| `DrawPlanParams` | 144 | `rects[8]` |

Six blocks hold no arrays at all; `FieldBus` is dynamically indexed but its
array is `array<vec4<f32>, 8>` — a native cbuffer indexed load, not the
pathological struct expansion — and one claim (`DesignConfig` at its
`fc_config` binding) was refuted and is not counted. **So the lane was
never briefly unblocked when the occupier windows left; no window was
missed.** The fork does not reshape its shaders to court a legacy
compiler: the shader shape is the program's law and FXC is the old road.
If a stage ever demands FXC, that is a design campaign entered with the
cliff on the map.

**Stated honestly, because the FXC record demands it.** 20,227 ms was
MEASURED. These six are shape-matched PREDICTIONS. The measurement would
be a boot this ruling declines to spend (L49's evidence clause).

**What is open, and it is one thing: Jean's button.**
`python tools\ember_route_a.py` discovers in seconds and changes nothing;
`--go` fetches `third_party/dxc` at Dawn's own DEPS pin, configures with
`DAWN_USE_BUILT_DXC=ON`, rebuilds **both** configs, and reports the
`dawn_lib` manifest delta by N6's method — a report, never an edit,
because whether `dxcompiler`'s shared linkage leaves an import library
this repo must link is a ruling. Overnight is its natural home.

**Then, and only then, UNIT.1.** `kCompilerPlan = D3D12_Dxc`, and the
witness is a boot log showing `Compiler plan (request): DXC` **and**
`use_dxc` inside the `GetTogglesUsed` line — both, or the lane is not
true. That pair is the exact negative-space of the PIVOT_0a defect, and
`--probe=120` is what turns it into an exit code. **UNIT.1 is halted until
the rebuild lands**: selecting DXC today reproduces PIVOT_0a's signature
for a third reason — not mis-chained, not driver-refused, compiled out —
and would burn a stop condition on an answered question.

**One deferred amendment, stamped.** `use_dxc` is `ToggleStage::Adapter`
at this pin (RECON.3 re-read the registry; L21's citation is sound), but
both arms chain on the **instance** descriptor, one inheritance hop above
that stage — and L21 itself closes debt 12 as MOOT with that hop never
witnessed carrying a toggle. The hopless root is `RequestAdapterOptions`,
which this program does not construct (`EnumerateAdapters()` is called
bare, deliberately unfiltered so the boot log lists every adapter).
Re-siting both arms onto it is **UNIT.1's work, with a boot to prove it** —
a boot-path change no witness runs is precisely what PIVOT_0a was.

**Closed at EMBER_0, no longer open:** the Windows SDK glob in E3 (an
unpinned ambient provider that resolved first on any host with an SDK —
N10's phantom in a second costume; excised while removing it could break
nothing); E3's "HYPOTHESIS, not a checked fact" banner (checked, false,
rewritten); the FXC arm resting on Dawn's default for `use_dxc` (it now
chains `disabledToggles` explicitly — a default is Dawn's to flip, a
disabled toggle is ours).

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
| S3 | portals (cont.) | **CLOSED (ONE_WORLD-I U4).** The rooms emptied: `GPUPortalArray` / `GPUPortalEntry` and the WGSL `PortalArray` / `PortalEntry` are gone, and with them the PASSER round — the array's last reader, a behaviour whose whole purpose was walking between doors. `GPUAgentRoomConstants` went 1584 → 512 B. `WorldDrawSurface` did NOT empty: `scheme_weights` survives, so the room S2 predicted would empty under S2+S3 stands on the indoor light scheme alone. |
| S4 | agent pipelines | FLAGGED — `agent_state[possessed_slot]` **is** the point; camera, the ring, terrain aura, the ribbon's sky rule and the shadow box all read it. (It read "LOD streaming" until ONE_SURFACE-I: there is no LOD and no streaming, and the finding is unchanged by that — the point is still what every one of them reads.) |
| S5 | the streaming conductor | **CLOSED (ONE_SURFACE-I).** Not a prune's subject and never on the S-strip: it was the machine that made an endless plane possible, and ONE_WORLD-II's pin is what made it answerable. `stream_patches` and its budgets, the recenter, the eviction lane, the veil's strength and the LOD split all left across U2–U5; `build_world` is what remains. |

`WorldDrawSurface` was to empty under S2+S3 together, under a `sizeof`
static_assert. S2 has landed and it did not empty: the fields the
gallery held were never `WorldDrawSurface`'s — the surviving mentions
there are the "Gallery" indoor LIGHT SCHEME (`scheme_weights[2]`) and
the portal dials, which are S3's. Whether the room empties is S3's
question alone now.

**ANSWERED, and not by S3 (ONE_WORLD-II U4).** The room never emptied
under a prune; it died with the rooms. `scheme_weights` fed the light-scheme
roll whose only declared reader was the indoor deriver, so `WorldDrawSurface`,
`WORLD_DRAW_LIVE` and organ block 10 all left together. The block id is a
HOLE, never re-packed — a stored preset key is a block id, and re-packing
would silently re-point every one.

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

### ONE_WORLD-I — THE DOORS (landed)

The transition machine, the portals, the ARCH family and the rooms they
left behind, in six commits. **S3 is CLOSED** (the table above carries the
per-finding answers); the world is one world now, and nothing in the
program moves between worlds.

| unit | what left |
|---|---|
| U1 | `TransitionPhase` and the machine, `phase_portal_trigger`, `request_mood_transition` and the six keys, the console's mood door, the `portal_trigger` wire in all four rooms, the `transitions` roster bit. The TEARDOWN arm GRADUATED to `rebirth_world` — the one survivor. |
| U1a/U1b | Two stale citations the unit's own sweep missed; then the fade overlay whole (the rider ruled it in scope) and the `Controls:` line that still offered the dead mood keys. |
| U2 | The force-spawn channel, the arch's portal identity, `PortalDestination`, the destination law (`portal_density`, `mood_weights`, the palette, the weighted walk) and sixteen organ dials, the `portal` roster bit, the census's portal column. |
| U3 | The ARCH family whole. `PopFamily` 6 → 5 with every positional table re-columned, four pipelines, the GPU mesh-gen scratch (the converged MESHGEN trio now unallocated), the entity ground atlas and its two spine rows, `occupier_amg`. |
| U4 | The portal room itself and the PASSER round that walked it; `GPUAgentRoomConstants` 2864 → 512 B. |
| U5 | The proximity subsystem (five tables, unexercised since PRUNE_2), the PASSER's `route` field, the arch slack dial, and the prose probate. |

**A REBIRTH IS A HARD CUT** — the crossfade left with the machine that
drove it. FLAGGED for Jean's visual gate, and the flag is LIVE now: the
verb waited two campaigns marked `SEAM[spine:P8]` with no caller, and
**THE_PANEL I U1 gave it one** — block 15's seed dial and the REBIRTH
door. The mark is struck at the verb, tombstoned rather than deleted,
because the forward cue it carried came true verbatim. The hard cut has
never been seen; Jean's walk is the first thing that will watch a world
die.

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

### ONE_WORLD-II — THE WEATHER (landed)

The seven moods, the theme engine and the indoor rooms, in nine commits.
**The world is one world and wears one weather**: a world is a SEED and
nothing else, every fact the program used to look up per mood is a LIVE
BANK the panel owns, and the world is pinned FINITE with its radius drawn
from the seed between two dials.

| unit | what left, and what rose |
|---|---|
| U1 / U1a / U1b / U1c | **The banks rose first, and the moods still stood.** `ATMOS_LIVE` (flat — a bearing plus twelve centre/spread fields, `REGIME_COUNT` and the weighted roll gone), `ORB_LIVE`, `CUBE_LIVE`, `AGENTS_LIVE`, each transcribed from the sunset row and pinned by a witness that spent itself against the still-standing table. Every runtime applier rewired to a bank; the mood tables became single-purpose seeds with no runtime reader. The organ's atmosphere rows went 52 → 12. |
| U2 | **The moods fell.** `MOOD_TABLE` / `MOOD_LIVE` / `mood_def` / `MoodProfile` / `MOOD_NAMES` / `WorldShape` / `Regime` / `Atmosphere` / `CeilingType`, `--mood`, `DEMO_BOOT_MOOD`, `mood_constants.hpp`, `MoodState` → `SkyState`, `apply_mood` → `stage_world_birth`. Cut as an ENUMERATION of six death-verified symbols, not as a banner span — the clause that made that mandatory is below. |
| U3 | **The theme engine fell** and `population_themes.hpp` survives as the population panel proper. `THEMES`, the lattice, the envelopes, the cooldowns, `theme_tier_weights`, `theme_short_name`, `MOOD_SPAWN_MULT` and traits' `mood_multiplier`; `compose_spawn_chance` is `global → base × adj → clamp`. |
| U4 | **The rooms fell, whole.** The shell mesh + `ShellVertex` + `Dim::SHELL_*` + its pipeline, `INDOOR_PALETTES`, `INDOOR_LIVE` / `IndoorSurface`, `WORLD_DRAW_LIVE` / `WorldDrawSurface`, the spot lights (array, atlas half, pipelines, `SPOT_PCF_*`, WGSL), the per-family indoor sizing hook and its policy table, the three structural pads. `GPULighting` 848 → 48 B and `GPUFrameR` 1040 → 240 B in both rooms. Organ blocks 8 and 10 are HOLES. |
| U5 | **The pin.** `WORLD_FINITE` boots true and nothing unsets it; the radius derives from the seed between `FINITE_RADIUS_MIN/MAX`. `become_world` prints the world it made. |
| U6a / U6b | **The organ re-homed.** The definition-only mechanism retired whole — the 255/254 sentinels, the `ORGAN_PARAM_DEFONLY` pair, `is_defonly` — because every row addresses an instance now. Blocks 12/13/14 (`ATMOS`, `ORB_BANK`, `AGENTS`) enrolled; the selecting kinds' target parameter left, and a definition target above 0 is REFUSED OUT LOUD by name. |
| U7 | **The sweep.** Prose probate across `src/`, `tools/` and `docs/`; three orphaned `*_INDOOR_RESCALE_PARAMS` tables U4 left behind; a stale `spotShadowMapTexture_` row in `binding_schema.py`; `mood.hpp` → `sky.hpp`, `apply_mood_lighting` → `stage_sky`, `apply_aura_mood_policy` → `apply_aura_policy`; struck-notes on L10 and L44; the ledgers' hand-copied tally corrected against the tool. |
| U8 | Instruments, the audit room, the L33 witness and the full battery. The G-LAW 2 record found `kFrameSlotZero` — a constant three live sentences called the offset every group-1 bind passes, referenced by nothing since U4 took the dynamic seat. `tools/organ_gap.py` carried three dead pairs that witnessed nothing and reported zero readers for them, which reads as a pass; the three banks this campaign built were HOMES with no pair at all. Thirteen pairs, all real, zero surviving readers. |

**AMENDMENT B GREW A THIRD CLAUSE, and it was paid for three times.**
Deletion regions anchor on **symbols, never on banners**; a banner-to-banner
span is lawful only after every symbol between the banners has been
individually death-verified and appears on the cut's enumeration, and
anything found between them that is not on the list survives the cut and is
flagged. The persistence ladder had to be relocated OUT of the mood region
before U2 could cut it; a banner span in `world.wgsl` took `veil_dither_noise`
and `veil_t`; one in `orb_surface.hpp` took `ORB_TABLE`. All three were caught
— two by a gate, one by the compiler — and a between-banner orphan with no
callers would have passed every gate in silence.

**AMENDMENT D, the three rooms of a fact.** A fact has its definition (the
bank), its applier (the read) and its transport (organ rows, C-ABI exports,
preset keys); when a commit moves any room, every room moves in that commit.
Paid for at U1, where the ruling deferred transport to U6 and the panel spent
five units writing `MOOD_LIVE[mood].atmos.*` while the draw read `ATMOS_LIVE`
— an atmosphere dial turned and nothing happened, and no gate in the tree
could see it.

**Parked with THE PANEL, and DISPOSED AT THE_PANEL I U4.** The orphan
console verbs this sweep surfaced — `cycle_cube_behavior_override`,
`reveal_zoetrope`, `toggle_cube_kite_mode`, `unrecord_entity` and roughly
forty accessor leads — were each reachable from no key and no door, and
were left for "the panel's own recon, which is the sitting that can say
what a control surface needs". That sitting ran; its verdicts are in THE
PANEL's section below, and the short form is: three verbs graduated to
doors, one had already died, twenty-nine leaves were cut, and the rest got
a verdict naming why they are not this campaign's.

**The close.** Every gate green at the pushed tip: TU PASS/PASS · G-LAW 2
GREEN · WGSL PASS · score census GREEN · binding surface all witnesses (S-6
on a clean tree at the pushed tip) · organ gap PASS · organ ledger NO
SUSPECTS · mirror census all witnesses · command census all witnesses. L33's
standing witness was RUN at the closing commit — the five files in `audit/`
deleted, the five tools re-run, all five byte-identical. **G-LAW 1 and the
visual gate are Jean's and are not claimed here**, and neither is the boot
transcript the rider's §C.2 asks for: it comes from a build, and a build is
Jean's gate. The `[Atmos]` line to expect is one field shorter than the
target on record — `seed= int= amb= sun el= az= fog=`, no `mood=`, no
`regime=` — and `[World] Born FINITE radius=R (SxS patches)` is new.

**Flagged, not taken (naming and build are Jean's gates):**
- ~~`OrbMoodConfig` still wears the moods' name.~~ **TAKEN at ONE_SURFACE-I
  U0** on Jean's word: the type is `OrbConfig`. Wire-safe — a row's id is
  `#BLOCK "." #FIELD`, so no struct name reaches a stored preset key.
- The frustum-cull flag (`useIndirectTerrainPipeline_`) is WRITTEN once at
  every world's birth and READ BY NOBODY — OPT_1 found it latent, and the
  column that fed it died at U2, so the write is now the literal `true`. The
  cut wants a build.
- Five WGSL orphans that predate this campaign and are not its subjects:
  `row_occupier`, ~~`seg_closest`~~, ~~`signal_active`~~, `translator`
  (defined, referenced only from prose) and `CARD_NODES_4` (a documented
  exclusion). **TWO TAKEN at TENSE_0 U1**, each with a one-line tombstone;
  `signal_active` took `config.mute_signal`'s last reader with it, so the
  field retired to a named pad in both rooms at U2b. `row_occupier`,
  `translator` and `CARD_NODES_4` still stand — the first is on the
  PROTECT LIST pending the occupier-limb ruling, and TENSE_0 repaired its
  prose only.
- `docs/FXC_LAWS_RECORD.md` cites `GPUSpotLightArray`'s `static_assert` in
  `state.hpp` as a live example; the assert left at U4. It is a stamped
  record, so the sweep reports it rather than editing it.

### ONE_SURFACE-I — THE STILLNESS (landed)

The finite world is built ONCE, at birth, and never streams again. Seven
commits. **After this, the ground is still**; ONE_SURFACE-II makes it
alive.

| unit | what left, and what rose |
|---|---|
| U0 housekeeping | The two names ONE_WORLD-II flagged and could not take: `OrbMoodConfig` → `OrbConfig` (wire-safe — a row id is `#BLOCK "." #FIELD`, so no struct name reaches a preset key), and LATENT[frustum_cull_opt_out] taken whole with `SkyDeps::renderer_` and the fwd it existed for. |
| U1 | **`build_world`.** The whole (2R+1)² grid allocated, spawned, baked, banded and uploaded at birth, at both doors. It stages the seed and bounds and drains the config ITSELF — `phase_stage_upload` has not run when a boot builder bakes. One batch per submit (LATTICE_1); the params buffer holds 225 records against 81 at the widest radius, so the loop runs exactly once. |
| U2 | **The conductor falls.** `stream_patches`, `request_recenter`, the two windows, the six budgets, the alloc scan, `active_radius`, the `[STREAM]` line, `set_render_radius` and the [ ] keys. R3 becomes `surface_visibility` — banding and the entity cull are functions of a moving POINT, and only the window stopped moving. It also found a bug U1 had shipped: `upload_tile_grid_now` sized its window on the STREAMING radius, correct only because the conductor capped that radius around its own call. |
| U3 | **The eviction lane**, under its guard — traced verb by verb first: no teardown path reads `entity_refs`; every one sweeps by owner. `evict_patch`, `evict_patch_entities`, `free_layer`, the registry, `FamilyDispatch::evict_slot` and the five family evictors. A new positional net on `FamilyDispatch`, proven to bite. The ribbon's two-tip reference protocol came with it; its REJECT gate stayed. |
| U4 | **The veil's strength**, and the fold table that overturned the unit's premise. The icing, the RIM knob and `veil_scale` die. The RING does NOT: four gates read it ungated by strength, and at radius 4 the box diagonal is 636 wu against a ring of 342, so it still culls inside the wall. The grain does not fold to a constant either — it is a smoothstep over [300, 342], constant only at radius 1. |
| U5 | **The LOD fold.** One band, one density; `lod0_radius`, the plan's `lod0_count`, FC_SEG_C, the third args slot, the TERRAIN_C bit and two chain asserts. `lod_point` → `cull_point` (wire-safe: not enrolled). The A/B split SURVIVES and is not LOD — it is zone overlap. The LOD1 index buffer survives its band: the SHADOW pass draws at LOD1 density by ECONOMY_1 E2. |
| U6 | **The sweep**, and what it swept was mostly older than this campaign: `ground_entries_dirty`, `placement_dirty` and `entities_culled` (zero readers, naming two functions that exist only in comments), `update_entity_draw_visibility` (a stub returning 0 since ONE_WORLD-I U3), `mesh_gen_settled` and `MESH_GEN_SETTLE_S` (no callers), `world_young` (whose only reader they were), `mark_patches_for_regen` and `PatchPhase::NEEDS_REGEN` (structurally unreachable), `PATCH_GRID_RADIUS`/`SIDE`. Both SEAM banners rewritten. |
| U7 | Instruments, the audit room, the L33 witness, the full battery. |

**THE ONE DISCLOSED BEHAVIOUR DELTA, and it is one seed in four.**
`derive_finite_radius` draws R ∈ {1,2,3,4}. The conductor's fullRegen arm
spawned everything inside the PRIORITY window — `PATCH_GRID_RADIUS` = 3,
a 7×7 — so for R ≤ 3 birth was ALREADY a one-shot build and no budget
ever fired. Only R = 4 leaves a 32-patch outer ring, which the conductor
served over later frames ordered from the RIBBON_4 look-ahead point.
`build_world` takes all 81 in one nearest-first pass from the point, and
the two orders differ: (4,0) is 200 wu out and (3,3) is 212, so one pass
serves a patch outside the old priority window before one inside it.
Entity SELECTION is seed-driven and unchanged; what can differ is which
of two entities wins ground both want, since footprints register at PLACE
in candidate order.

**Flagged, not taken — AND NOW RULED (ONE_SURFACE's close).** All three
were raised as findings during the campaign and held for a decision. The
decisions are below, so nothing on this list is still waiting on an
opinion; two are parked with a named destination and one is closed.

- **`veil_ring` / `veil_icing` — TAKEN AT THE_PANEL I U3.** They carried
  the veil's name for what are now the draw authority and the grain's
  band. A CONFIG field's name IS its organ row id (`ORGAN_PARAM_NS`, one
  row id per `#BLOCK "." #FIELD`), and a row id is a stored preset's key,
  so the parking waited for a migration moment. This was it.
  They are **`draw_ring`** and **`grain_band`** now, in both rooms, with
  `VEIL_RING_DEFAULT`/`VEIL_ICING_DEFAULT` and `GPUState::veil_ring()`
  renamed with them — a half-rename that left the constants saying `VEIL`
  would be the drift the parking was avoiding. The group moved from
  `Atmosphere · Veil` to `Atmosphere · Ring & grain`.
  **The names are TRANSCRIBED, not invented**: both rooms already called
  these two "THE DRAW AUTHORITY" and "the grain's band" in the prose
  beside them. **NAMING IS JEAN'S GATE** — overruling costs one line per
  room plus one alias row, and the ledger that makes such a change cheap
  landed with it.
  **AND THE BREAKAGE THE PARKING FEARED WAS NOT THERE.** `baseline.json`
  — the tree's only stored preset — contains ZERO `veil_*` keys, because
  the web panel's export walk excluded the boot-pinned GPUDesignConfig
  rows and these two are among them. The premise was right in principle
  and the instance was empty. What the census DID find is below.
- **`FINITE_RADIUS_MIN/MAX` — PARKED TO THE PANEL, and WEATHER's promise
  is amended to say so.** ONE_WORLD-II §1.7 said the pin would enroll
  them; it did not, and ONE_SURFACE §1.4 then presumed they were dials
  whose gen-cadence edits reach the world through rebirth. Neither is
  true today: they are `constexpr`. Enrolling them is an ORGAN_3
  graduation — a constructive change neither handoff scoped — and the
  only thing that could ever exercise one is THE PANEL, which is also the
  only thing that can call `rebirth_world`. **The promise is not
  abandoned, it is re-dated**: an unexercisable dial is an enrollment that
  states a belief nothing proves (L45), so the graduation rides the
  campaign that gives it a caller. The pin works meanwhile; nothing in
  the world is waiting on it.
  **LANDED AT THE_PANEL I U3, and it is NOT quite an ordinary ORGAN_3
  graduation.** U1 gave `rebirth_world` a caller, which made a
  gen-cadence radius dial exercisable for the first time; U3 enrolled the
  pair as `WORLD.radius_min` / `WORLD.radius_max` (block 15, GEN, C3
  destructive) and pointed `derive_finite_radius` at the bank. The draw
  and its salt are unchanged, so a given seed under the boot range draws
  exactly the radius it always drew.
  **What is not ordinary: the design constant KEEPS a runtime reader, on
  purpose.** `FINITE_RADIUS_MAX` is also a CAPACITY — three static_asserts
  bind it to frustum-cull segments A and B, to the layer pool and, through
  `Dim::AUTO_GRID_MAX`, to the automaton's life buffer, which is
  `(2*MAX+1)*PATCH_CELL_N` cells square. A world wider than the pin does
  not look wrong; it indexes past that buffer's end on its last row, every
  frame. So the constants stay compile-time and the dial is fenced three
  times — the enrollment line names the constant as its ceiling, `organ_set`
  clamps a consumer to it, and `derive_finite_radius` clamps again at the
  last line before the number becomes an index. `organ_gap`'s PAIRS map is
  deliberately NOT given this pair: a PAIR asserts the design symbol has no
  surviving runtime reader, and here one is doing its job. The reason is
  written at the tool so a successor does not "fix" it.
  The pair moved home with the enrollment: they are declared in
  `contracts/world_surface.hpp` now, beside the bank they seat, and
  `surface_services.hpp` keeps the capacity asserts that need `Dim::`.
- **`world_box_clamp_xz`'s `has_bounds` — KEEP, and closed.** It reads
  like a finiteness test the pin makes constant. **It is not.** It is the
  uninitialised-config guard wearing a finiteness costume:
  `GPUState::initializeState` zeroes the bounds and uploads them
  unconditionally, so a boot frame really does carry (0,0), and removing
  the guard turns `clamp(p.x, 0+m, 0-m)` — low above high, which WGSL
  leaves undefined — into a body test. The costume was the whole hazard,
  so the cure is the comment: `world_box_clamp_xz`'s banner now says what
  the guard is for and what must die with it. **No further action; this
  bullet is a record, not a task.**

## THE DEVICE GATE (open — the probe is unproven code)

`--probe=N` landed with L48. It boots, runs N frames through the ordinary
loop, and exits on the device's verdict: `PROBE GREEN` and 0, or the first
uncaptured error verbatim and nonzero. It exists because ONE_SURFACE-I U5
moved one room of an L3 MIRROR pair, nine text-reading gates went green,
and every frame of the first native boot failed validation.

**RATIFIED at the UNBLOCK RIDER: both deviations stand.** Not-headless is
right — L10's one loop outranks the commission's "headless" spec; the
probe gives the loop a reason to stop, not a second loop. And
`--probe-backend` defaulting to `any` is right — the known-boots
configuration is the one worth probing, and an instrument that will not
boot is worse than none. One run with `=null` settles the rest, at Jean's
leisure. Nothing below is a re-litigation of either.

**AND IT IS NOW LOAD-BEARING.** ONE_SURFACE-II's U1 is not "landed" until
`--probe=120` passes on it; THE_PANEL's U1 (the seed door, the campaign's
riskiest live path) is the same. The probe went from an instrument to a
precondition in one rider.

**IT HAS NEVER RUN.** CC's environment carries `third_party/dawn_native_headers`
and no built Dawn, no GLFW and no display; the TU gate type-checks
`console.hpp` and `the_board.cpp` and that is the whole of what CC can
witness about it. Its first run is Jean's, and it is the pre-walk for
every constructive GPU commit from here.

**UNTIL BOTH RUNS LAND, THE PROBE GUARDS NOTHING BUT HOPE**
(AFTER_AUTOMATON §2). A clean run at the tip is the tip's evidence in
spirit; it is the RED run at `2905ed68` that proves the instrument bites.
An untested gate reporting GREEN is indistinguishable from a gate that
cannot fail.

**THE AMENDMENT-A INJECTION IS ORDERED, AND IT IS TWO RUNS, NOT ONE.**
Before the probe's first load-bearing use it must be proven to bite: run
it at the tip (must be GREEN) **and** at `2905ed68` (must be RED, with the
binding-size error as its first line). One run alone proves nothing — a
probe that always says GREEN passes the first test and fails the job. An
instrument is trusted only after it bites.

Three things the first runs settle, in order:

1. **Does `--probe=120` boot and print a verdict at all?** The likely
   surprises are the exit path (the loop leaves through `running()`, not
   through a close request, so the window is still open when `main`
   returns) and whether 120 frames is long enough to reach the states
   worth validating.
2. **Does it catch the bug it was built for?** The honest way to know is
   to run it against `2905ed68` — the tip whose log opened the commission.
   It must print RED with the binding-size error as its first line. A gate
   that has never been proven to bite is a gate nobody should trust
   (Amendment A), and this one has a ready-made injection sitting in
   history.
3. **Does `--probe-backend=null` boot?** If Dawn's null backend can serve
   this console's real GLFW surface, the probe becomes a validate-only run
   that needs no GPU — and a CI-shaped gate becomes possible. If it cannot,
   the switch is a dead letter and should be retired with a tombstone
   saying so, rather than left as an option nobody can take.

### A SECOND, STATIC HALF OF THE SAME CLASS — **BUILT AND BITING**

> **COMMISSIONED at the UNBLOCK RIDER, LANDED at ONE_SURFACE-II U4** —
> `binding_gen --check`'s **S-8**, in exactly the measured shape: a
> relation row inside an existing gate, not a tenth gate.
>
> **IT CHECKS ELEVEN SEATS AND COVERS EVERYTHING.** The measurement below
> feared partial coverage; the reduction turned out to reach every case
> that occurs — six declarations across eleven group entries, with
> `nothing uncovered` in its own verdict line. A case it could not judge
> would be REPORTED BY NAME rather than skipped, which is the discipline
> the hole itself argued for.
>
> **PROVEN TO BITE** (Amendment A), by the injection the commission
> named — `fc_indirect`'s store type set back to `2905ed68`'s
> `array<atomic<u32>, 15>` against today's `FC_ARGS_BYTES`:
>
>     [FAIL] S-8 ... fc_indirect@cullStateGroup_[3]:
>                    WGSL extent 15 x 4 B = 60 B vs C++ 40 B (FC_ARGS_BYTES)
>
> **That is Dawn's own sentence, in arithmetic, from text.** The device
> said "bound with size 40 ... requires at least 60 bytes"; the gate now
> says 60 vs 40 before the shader is ever compiled.
>
> **The control holds too**: under that same injection `mirror_census`
> still reports zero failures, which is the hole reproduced on demand and
> the proof that S-8 is the only thing that closes it.
>
> **WHAT IT DOES NOT CLOSE**, so nobody mistakes its scope: only FIXED
> extents. A wrong size behind a runtime-sized `array<T>` has no extent to
> compare, and neither has a rejected layout, an overrunning copy, or an
> index past the end of a map. Those stay the probe's, and L48 says so.
>
> The measurement below is kept as written — it is what the witness was
> built from, and the record of how a device error, read carefully after
> the fact, turned into a text gate.

A sweep after the hotfix asked whether `fc_indirect` was alone. It measured
this, at this commit:

**Six declarations in `world.wgsl` carry a FIXED array extent** —
`agent_state` and `render_agents` (`array<AgentState, 32>`), `ring_xforms`
and `render_ring_xforms` (`array<RibbonRingTransform, 400>`),
`ribbon_spine` (`array<vec4<f32>, 402>`), and `fc_indirect`
(`array<atomic<u32>, 10>`). **Every one of them agrees with the C++
constant behind its seat**: `Dim::MAX_AGENTS` is 32,
`Dim::RIBBON_MAX_RINGS` is 400, `Dim::RIBBON_SPINE_SLOTS` is 402, and
`FC_ARGS_SLOTS` is 10. The tree is clean; `fc_indirect` was the only one
that had drifted.

**But nothing checks that, and here is exactly why it slipped.** Two gates
each hold one half and neither holds the pair:

- `mirror_census` pins the SCHEMA's store-type spelling against the WGSL
  declaration. At `2905ed68` both said `array<atomic<u32>, 15>` — in
  perfect agreement, and both wrong.
- `binding_gen --check` pins the seat's `size_expr` against the C++ tree.
  It said `FC_ARGS_BYTES`, which was correct C++ for 40 bytes.

So the schema agreed with the shader, the C++ agreed with itself, and the
one comparison nobody makes — the WGSL EXTENT against the C++ COUNT — was
the whole defect. It is a static fact about two files, and a text-reading
gate can hold it.

**The shape it should take**, if it is ruled in: a witness inside
`binding_gen --check` (which already owns schema-vs-tree agreement, so this
is a row in an existing gate rather than a tenth gate). For every
declaration whose store type is `array<T, N>` with N literal, resolve the
backing seat's `size_expr` and assert it equals N elements. The reduction
that makes it exact rather than heuristic: where the C++ spells
`<count> * sizeof(GPUFoo)` and `GPUFoo` is the declared C++ mirror of the
WGSL `Foo`, `mirror_census` already proves those two are byte-identical, so
the byte equation reduces to `count == N` with no WGSL layout arithmetic at
all. Scalar and vector elements resolve numerically. All six declarations
above are inside that reduction.

**And it can be proven to bite before it is trusted** (Amendment A), with
no injection to invent: run it against `2905ed68`, where the schema said 15
and `FC_ARGS_BYTES` said 40. It must go red there and green here.

**Left as a commission rather than built.** Adding a witness changes the
battery, and the battery is not CC's to change on its own reading — the
triangle puts rulings with Claude. The measurement above is the whole of
what CC owes the decision: the gap is named, its two halves are named, the
tree is proven clean at this commit, the implementation shape is worked
out, and the injection that would prove it exists in history.

**The headless half of the commission is NOT built, and deliberately.**
§3 asked for a headless boot; this console boots GLFW and configures a real
window surface, and a headless path would be a second boot path through the
console — a second door, which L10 forbids. Making the probe headless is a
console change (an offscreen colour target replacing the swapchain, chosen
at one seam rather than at two boot paths), and it belongs to whoever wants
the probe in CI. Nothing in the current campaign needs it.

## THE MOSAIC IS UNREACHABLE — THE DEFAULT FIRED (closed at THE_PANEL I U5)

**IT ARRIVED UNSPOKEN, AND SILENCE EXECUTED THE DEFAULT.** Jean's ruling at
ONE_SURFACE's close set it; AFTER_AUTOMATON §2 dated it to this unit —
"MOSAIC_2 RETIRES IN THE_PANEL-I U5 unless Jean says *keep the grain*
before that unit runs". No such word arrived. The apparatus retired.

**WHY IT WAS UNREACHABLE.** `entity_fs` guarded the mosaic on
`if (in.mosaic_seed != 0u && config.mosaic_enable > 0.5)`, and **no vertex
entry in `world.wgsl` ever wrote `mosaic_seed`.** WGSL zero-inits `var out:
EntityVarying;`, so the seed was 0 at every fragment and the branch never
ran. On the CPU side `EntityInstance::mosaic_seed` was declared and never
read or written anywhere in `src/` or `tools/`. Dead on both sides, and it
PREDATED ONE_SURFACE — whatever used to write the seed left before either
campaign.

**WHAT LEFT.** In `world.wgsl`: 211 lines — `MOSAIC_MEDIANS`,
`mosaic_cell_seed`, `MosaicShard`, `mosaic_shard`, `MosaicPassage`,
`mosaic_pcell`, `mosaic_passage_at`, `mosaic_far`, `MosaicSample`,
`mosaic_sample`, property run 900–921 — plus `veil_t`, `EntityVarying`'s
`paint_y` and `mosaic_seed`, the `entity_fs` branch, and six `config`
fields. In C++: `EntityInstance::mosaic_seed`, five `Dim::` rests,
`GRAIN_BAND_DEFAULT`, six boot pins, six organ rows and six struct fields.
`docs/LAWS.md` L11 and L12 take struck-notes; both laws are kept WHOLE,
because the reasoning in each outlives the one mechanism that illustrated
it.

**GPUDesignConfig 688 → 672**, and the arithmetic split two ways, which is
the interesting half:
* The **five mosaic dials** were reclaimed — 20 bytes from BELOW
  `checker_resultant`, the struct's last 16-aligned member, so nothing
  after them had a boundary to lose. One fresh pad carried 668 back to 672.
* **`grain_band` could NOT be** and is a named pad. It sits upstream of
  `palette_center`, which is `array<vec4<f32>,4>`; the stretch between
  `pulse_data` and it now holds THREE retired pads, 12 bytes, and twelve is
  not sixteen. If a fourth field in that stretch ever dies, all four come
  out together. That is the whole collection that stretch will ever offer.

**AND THE NEAR-MISS IS THE UNIT'S REAL FINDING.** The first cut of this
work deleted `grain_band` from the WGSL struct and left a pad in the C++
one. **Both structs stay 688 bytes under that edit** — WGSL rounds its
size up to the struct's 16-byte alignment and `alignas(16)` does the same —
while every offset from `veil_ring` to `palette_center` sits four bytes
apart between the rooms. The `sizeof` witness cannot see it. Neither can
`0b-4`. **U2 documented that exact blind spot by injecting against it ONE
UNIT EARLIER**, which is the only reason it was caught here by reading
rather than by Dawn. Both rooms declare the pad now.

**AND `0b-4` FIRED FOR REAL, one unit after it was written.** U2 enrolled
DesignConfig in the marker convention at 688; this unit shrank it and the
ledger said, by name: *"DesignConfig (world.wgsl:1649): prose says 688,
calculator says 672"*. The marker reads 672 now, derived from `world.wgsl`
by the layout calculator, beside a `static_assert` that reads 672, derived
from `state.hpp` by the compiler. Two instruments, two files, one number.

**REVIVAL IS ONE LINE PER ENTITY VS** — `out.mosaic_seed = <something>` —
and everything is intact in history at this commit's parent. That is what
made a default safe.

**PROBE-PENDING.** Two hundred lines of shader left and every offset in the
config struct moved. The static battery is green in both rooms; only the
device can say the world still draws.

### A CORRECTION TO ONE_SURFACE-I U4's COMMIT MESSAGE

U4 kept the grain's smoothstep and its two dials, and argued for it from
arithmetic: the band is [300, 342], a finite world's box diagonal runs
212 / 354 / 495 / 636 wu at radii 1..4, so the grain varies for three
radii in four and could not be folded to a constant. **The arithmetic is
right and the conclusion is right — bodies do draw out to the ring at
342, so the grain WOULD vary if it were evaluated — but it is never
evaluated, for the reason above.** What U4 shipped is unchanged and
remains behaviour-identical, which is what its acceptance test asked; the
justification was one layer short of the truth.

Two smaller corrections from the same source, both already true in the
tree and neither changing what shipped:
- the ring gates anchor on `config.cull_point_x/z` (staged, one frame
  stale by law E-4) while `veil_t` anchors on ~~`render_point_pos()`
  (live)~~. Two anchors, which is precisely the condition L12's closing
  paragraph says one band exists to avoid.
  **AMENDED at TENSE_0 U1, and the amendment strengthens the correction
  rather than weakening it.** `render_point_pos()` had NO CALLER — one
  occurrence in all of `src/`, its own definition — so it was struck. The
  second anchor was never evaluated by the running program; the "two
  anchors" condition was already moot when this correction was written,
  and what the veil actually anchors on is a question this row leaves
  open. The word that misled was `(live)`.
- `ribbon_vs` has no ring gate at all — U4 noted this as the ribbon's
  surviving exemption, and it is worth restating that the exemption is
  now the ONLY thing distinguishing `ribbon_fs` from `entity_fs`.

## ONE_SURFACE-II — THE AUTOMATON (U1-U3 landed; the close is Jean's)

> **THE UNITS ARE IN.** U1 the switchover, U2 the family, U3 this sweep.
> What remains of the campaign is U4's instrument commit (the extent
> witness the rider commissioned) and then the close: glaw1, build, **the
> probe**, and the walk — the ground alive everywhere, wrap seams
> invisible because a torus has no edge, the world otherwise itself.
>
> **THE SCALE QUESTION U1 FLAGGED IS ANSWERED, AND MEASURED.** U1 warned
> that Glacier's density 0.12 over 20,736 cells might read as a forest
> where it read as an island. `tools/gol_census.py`, repointed at
> AUTO_TABLE by this sweep, ran the actual rule on the actual 144-cell
> torus for 16 world seeds x 2000 generations:
>
>     AUTO_TABLE  B3/S23  0.12/0.03  N=144 | dark 0 | satu 0 | strc 16
>                                          | live 3.1% | frze 0 | dark% 0.0
>
> **3.1% live, not 12%.** The seeded density is a transient; Conway thins
> it to ~640 live cells of the 20,736. Zero worlds went dark, zero
> saturated, all sixteen stayed structured, none froze. So the ground is
> a sparse structured field that never flattens and never fills — which
> is what the transcription was hoping for and is now evidence rather
> than hope. The one number the census cannot give is whether ~640 cells
> lifting 24 wu LOOKS right; that is the walk's, and `density` and
> `alive_height` are the two dials to reach for.
>
> (The dark count is a LOWER BOUND — the tool does not model the birth
> mask, which only ever removes cells. At 0 dark over 16 seeds the bound
> is not close to binding.)

### THE ORIGINAL HEADER (the hold, and how it resolved)

The Game of Life stops being an ENTITY and becomes a PROPERTY OF THE
GROUND: one automaton over the whole finite cell grid, WRAP topology, the
GOL family retired. U0's recon is done and is below.

**THE HOLD IS RESOLVED.** The Stillness closed at Jean's device; the
precondition ("RUN AFTER THE STILLNESS CLOSES GREEN") is met. What
replaces it is narrower and harder: **U1 lands only probe-GREEN.** The
switchover commit is not "landed" until `--probe=120` passes on it, and
before that first load-bearing use the probe itself takes its Amendment-A
injection — Jean's two runs, GREEN at the tip and RED at `2905ed68`. See
THE DEVICE GATE above.

The reason the bar is a device and not a battery is unchanged, and worth
restating because it is the whole shape of this unit: every unit of
ONE_SURFACE-I was a REMOVAL whose behaviour could be reasoned to exactly
— a fold table, a reader census, an arithmetic bound. **U1 is
CONSTRUCTIVE**: a new world-sized field, two new kernels, a reshaped bind
group, a swapped contributor row. "It type-checks and the gates are green"
is much weaker evidence about a shader that has never run — which is the
sentence the hotfix round paid for.

### THE RIDER'S AMENDMENTS, FOLDED (the handoff is not reissued)

- **§1.5 is answered precisely and the answer is BOTH**, in the two
  distinct mechanisms enumerated below: the pedestal is a LIVE-CARD
  contributor (the `.a` channel), the tint is a separate draw-time
  array-layer sample. **U1 globalizes both AS FOUND** — same mechanisms,
  world-sized. No new visual behaviour is invented; the dials inherit
  today's values.
- **The live-card ruling is already law** (recorded below) and U1
  executes it deliberately rather than discovering it.
- Everything else in ONE_SURFACE-II stands as written: WRAP pinned,
  CONWAY boots with PULSE dialable, AUTO_LIVE transcribed-and-pinned, the
  derive-submit seam dying with its cause, teardown reshaped under P8's
  law.
- **The extent witness rides U4's instrument commit** (commissioned at
  the rider; specification under THE DEVICE GATE). The automaton reshapes
  bind groups and should land under that witness, not before it.

### THE ABLETON SEAM GAINS ITS CUE (§1.2, recorded at U3)

The automaton's `tick_period` is **in BEATS** — it always was, because
`GolDeps` carried `TimeState` and the config header read beats rather
than seconds. What changed is that there is now ONE period instead of
eight rolled ones, and it is a dial on a bank.

**So stepping the world on Ableton's bar is one dial away, and the dial
already exists.** `upload_automaton_header`'s gate is
`floor(beats / tick_period)` crossing an integer; point `beats` at a Link
or MIDI-clock transport instead of the internal follower and the ground
advances on the music's bar. Nothing in the automaton has to move for
that — the wiring is the COUPLING campaign's, beside the seam below, and
this line exists so that campaign does not have to rediscover that the
hard half is already done.

### THE LIVE CARD'S REST LAW — RULED (for U1)

U1 must not carry this decision as an open question, so it is made here.

`live_card_is_live()` (cartridge.hpp) is a three-term disjunction:
`pulse_count > 0`, `terrain_time > 0`, and **any GoL zone active**. The
third term dies with the zones — there are no zones in the automaton
world, only the ground. **U1 takes that conjunct out with the zone scan
it reads, and does not replace it**: an automaton over the whole cell
grid is live by construction, every frame, everywhere.

**Which means the rest-close optimization retires**, and the ruling says
so rather than leaving a skip that can never fire. OPT_1a's whole
apparatus — `liveCardRestClean_`, the entering-rest clearing write, the
`live_card_state_label` two-word vocabulary — goes with it, replaced by a
TOMBSTONE at the site citing GROUND_CARD_1, saying what the skip was, what
made it unreachable, and the one condition that brings it back.

**Jean's boot log already shows this is the state.** `[Card] live-card
field: LIVE — writer runs every frame (boot)` — before U1, on the world
as it stands. The optimization is not being taken away from a program that
was using it; it is being retired from a program that had already stopped
reaching it. That is the difference between a cut and a loss, and it is
worth stating in the commit that makes it.

**And it comes back if idleness does.** If the panel era ever adds an
automaton pause dial, "the ground is not advancing" becomes a real state
again and the rest law is the right answer to it — one condition, at the
tombstone, which is why the tombstone names it. Decided on purpose, not
by omission.

### §1.5 IS ANSWERED, and the answer is BOTH — with a precision the
handoff's phrasing leaves open.

**(a) The pedestal is a LIVE-CARD contributor, not a patch-bake one.**
`contrib_gol_zones_at(world_xz)` is evaluated inside the live-card
kernel and stored in the card's **`.a` channel**
(`textureStore(live_card_write, ..., vec4(height, grad_x, grad_z,
contrib_gol_zones_at(p_here)))`). Ground consumers then compose
`h += contrib_gol_zones_at(xz); h -= contrib_gol_suppression_at(xz,
consumer_pos)`. So the contributor DAG row the handoff names is real, and
it feeds the LIVE CARD — the patch heightfield bake is a different lane.

**(b) The tint is a draw-time texture sample.** `zone_life_read`
(`texture_2d_array<f32>`, g3:102) sampled in the terrain FS at
`i32(z)` — the ZONE INDEX is the array layer — feeding
`apply_gol_color(base_color, zp, cx, cy, blend)`. Globalizing means the
layer index has nothing to select and the sample becomes a single plane.

### THE HAZARD U0 FOUND, and the handoff does not mention it

**THE LIVE CARD'S REST LAW HAS A CONJUNCT THE AUTOMATON WOULD FALSIFY
EVERYWHERE.** The card is at rest — and `phase_live_card_write` returns
before its dispatch, gated by `liveCardRestClean_` — only when ALL of:

  (1) `config.terrain_time <= 0`      [MUSICAL]
  (2) the pulse ring is empty         [MUSICAL]
  (3) **no zone covers the texel**    [NOT MUSICAL]

The shader's own note says why (3) is the awkward one: *"silence the
music and a living zone still lifts."* With eight zones that is a LOCAL
condition and the card reaches rest whenever they are quiet or absent.
A world-sized automaton makes conjunct (3) false wherever ANY cell is
alive — which, at a seeded density over the whole grid, is essentially
always. The rest gate would never close again, and the live-card
dispatch would run every frame for the life of the program.

That is not a reason not to do it. It was a decision U1 had to make ON
PURPOSE rather than discover afterwards that an optimisation quietly
stopped firing — and **it is made: the rest law RETIRES and the card is
per-frame, priced.** (The ruling is above; the rider made it law.)

**The alternative that was NOT taken, recorded at the tombstone rather
than lost:** re-found the rest law on the automaton's own tick — the card
is clean BETWEEN ticks, so a tick-cadence gate would still close, often,
in exactly the world the automaton makes. It was not taken because it is
not a clean win: conjuncts (1) and (2) are musical and move on their own
clock, so a tick gate would have to compose with them rather than replace
them, and a per-frame card that is honestly per-frame beats a three-clock
gate nobody can reason about. If the price is ever measured and found
real, this is the second revival path beside the pause dial, and the
tombstone names both.

### The rest of U0's enumeration

- **Zone-shaped WGSL** (dies as indexing, survives as algorithm):
  `zone_config` / `zone_life` (g2:101/102, read_write storage),
  `zone_life_tex_write` / `zone_life_read` (g3:101/102),
  `GoLZoneConfig`, `GoLZoneArray`, `ZoneDeriveRequest` and the derive
  kernel, `apply_gol_color`, `contrib_gol_zones_at`'s
  `for z in 0..zone_config.count` walk. The ALGORITHMS — the neighbour
  rule, the pulse fields, the birth mask, `apply_boundary` — are
  index-free and survive whole.
- **The zone organ** (C++): `GoLZoneState[MAX_GOL_ZONES]`,
  `active_slot_count`, `gol_tier_extent`, `GoLZoneProp`'s per-zone
  Gaussian bands (930-938), `GOL_PULSE_ALGORITHM_CHANCE` (0.35),
  `pending_derive_requests`.
- **The derive-submit seam dies with its cause**, as §1.6 predicts:
  `SEAM[gol:derive-submit]` exists because runtime zone SPAWNS must
  derive mid-frame. With seeding birth-only there are no runtime spawns.
  Note that ONE_SURFACE-I already removed the only thing that created
  patches after birth, so the seam's premise is already half gone.
- **The roster's last excision** is the GOL family: PYRAMID 0, SPHERE 1,
  RIBBON 2, CUBE 3, COUNT 4. `FamilyDispatch` now carries the positional
  net U3 added and proved, so a re-columning failure lands at the
  contract.
- **The beat header** is already what §1.2 wants: `GolDeps` carries
  `TimeState` and the config header reads BEATS, so `tick_period`
  becomes one global dial in units the header already speaks.

## NATIVE PRESET INGESTION (open, born at WEB_SUNSET)

`presets/` holds the authored scenes the web panel used to fetch
(index.json + one file per scene). They currently have NO READER — kept
against L30's letter on the Phase W ruling [F2], because scene recall is
the performance instrument's obvious next organ. The consumer to build:
a boot flag (`--preset=<name>`) walking the same organ_set road
`?preset=` walked, and later the control surface's own load. If that
consumer is refused, this folder goes to the attic with a tombstone.

### THE MIGRATION RECORD LANDED FIRST (THE_PANEL I U3c)

`index.json` is **schema 2** and carries the refusal contract and a
`retired_ids` ledger. It is DATA, not code: it records what an importer
must refuse and what to say when it does, so PANEL-II's importer is
written against a census rather than a guess.

**49 of `baseline.json`'s 232 keys name families that no longer exist** —
and the file is left stamped **schema 1** deliberately. Re-stamping it 2
would make the version a lie and hide 49 refusals the importer should
print. The other 183 keys are live and correct.

| retired family | keys | left at |
|---|---|---|
| `MoodProfile.*` (with a `<target>/` mood prefix) | 9 | ONE_WORLD-II U2 |
| `OrbMoodConfig.*` (same prefix) | 19 | ONE_WORLD-II U1b/U6b |
| `WORLD.*` — the world-DRAW block 10 | 10 | ONE_WORLD-II U4 |
| `RIBBON_SPAWN.<array>` — array members the manifest has no row shape for | 9 | the web-era export walk |
| `INDOOR.*` — block 8 | 2 | ONE_WORLD-II U4 |

**THE HAZARD, NAMED LOUDLY BECAUSE IT IS NEW.** Block *ids* are never
re-packed — 8 and 10 are permanent holes for exactly this reason. Block
*NAMES* turn out to be reusable, and one has just been reused:
`WORLD.*` in that file means the dead world-draw block, while
`WORLD.next_seed` / `WORLD.radius_min` / `WORLD.radius_max` are block 15
and very much alive. No individual key collides — the field names are
disjoint — but a family-level alias or a PREFIX match would, and would
write the seed dial from a dead portal colour. **Match ids WHOLE.** This
is the first time a block name has been reused, and the ledger says so at
the row.

**The `RIBBON_SPAWN` row is a finding of its own**: those keys were
written by the web panel's export walk, which walked the STRUCT rather
than the manifest, so it emitted keys no `organ_set` call could ever have
accepted. The manifest is the whitelist; an export that does not walk it
writes a file the program cannot read back. PANEL-II's export must walk
the manifest — the re-import test in its close is the proof.

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
`[Zoetrope] ears bound:` line read 7 of 7. **CHOIR_0 retired the ears**:
the twelve bound names are six now, the seven onsets went back on the
shelf, `ch6.present_count` joined, and the boot line at that seam reads
`[CHOIR] ear bound: ch6.present_count (36 lanes)`. The misses count is
still 0 and its silence still means the same thing.

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

**The decision, open:** a gen-2 coupling — a pawn-origin pulse needs a
readback path the coupling layer does not have — or delete the WGSL
contributor and its config seats. Jean's, with a design pass. (This entry
once offered the onset ears' fold into `zoetrope_rows_` as the near half of
the wiring; CHOIR_0 retired both, so the seven `chN.onset` publications are
now unread capability on the shelf and the pulse would take them from
there. The readback half is unchanged and is still what makes this a
decision rather than a wiring job.)

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
`step_trigger`, `state.hpp`'s GPU signal header, and four `world.wgsl`
read sites. (The zoetrope STRIKE was on that list until CHOIR_0 excised
it; the choir's envelope reads the same `t_beats` through the canvas's
envelope clock, already named above, so the census loses a row rather than
gaining one.)

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

## THE COUPLING ATLAS (landed — `docs/COUPLING_ATLAS.md`)

Commissioned by AFTER_AUTOMATON §3 as the music campaign's design
substrate: five tables, every row by symbol, **no rewiring**. Jean
composes on it; the coupling campaign builds what he composes.

**The reassurance held and the tree proved it**: eight pipes, twelve
bindings, two seams, one composition law, all live. The atlas corrected
two premises of its own commission —

- **there are no misses.** LIGATURE_1 retired `BeatClock`, so all twelve
  sources resolve; Jean's boot log is the witness (`ears bound: 7 of 7`,
  and no `[SignalLayout] … unbound` line, which prints only on a miss).
  The seam's shape is one bit further out: `loopMIDI open=0/1`.
- **cadence is FOUR, not three** — LIVE / GEN / BOUNDARY / DRIVEN, and
  the fourth is load-bearing: a DRIVEN row is a METER and `organ_set`
  refuses it, so it can never carry a voice.

**The finding that reframes the campaign**: the layer's problem is not
rot, it is REACH. canvas_1 publishes 55 names and **12 are heard** — the
entire pc-DFT capability (16 names) and every voice's `distance` contour
have no consumer anywhere in the tree. Meanwhile the automaton adds 15
candidate dials of which NONE is enrolled. 43 unheard publications on one
side, 15 unreachable dials on the other, and a 256-slot bank that is 6%
allocated.

Also settled there, for PANEL-I U4: **`reveal_zoetrope` is a DOOR, not a
death** — the ears are bound 7 of 7 and the rows are struck every tick,
so it is an honest lever on live machinery.

## THE GoL VOCABULARY OUTLIVED THE GoL (open, named at THE_PANEL I U5)

The family left at ONE_SURFACE-II U2 and the ground took its place. Three
of its NAMES are still load-bearing, and each is a rename with a mirror on
the other side of it — so they are priced here rather than taken in a
sweep that was not commissioned to name things (naming is Jean's gate).

| what | where | what it actually is | cost |
|---|---|---|---|
| `mode_gol_tick_scale`, `mode_gol_height_scale` | `GPUDesignConfig` + `world.wgsl`, and two enrolled organ rows | the two per-frame COUPLING multipliers over the automaton's drawn tick period and alive height. Live cadence, GPU-read. | both rooms, one commit, plus two row ids. **Wire-checked: `baseline.json` names neither** — its 16 `CONFIG` keys are all `ribbon_*` — so nothing stored breaks, exactly as the veil pair turned out. |
| `gol_composite_cell_color` | `world.wgsl` | a TERRAIN-COLOUR PROBE. Its one caller is `compute_pawn_aura`; it is NOT the tint funnel, and ONE_SURFACE-II U3's addendum exists because a sweep once said it was. | WGSL only — G-LAW 2 is the whole verification |
| `AUTO_SEED_BAND`, `AutoProp::*` | already native | no debt; listed so the reader can see where the line was drawn |

The organ GROUP was renamed at U3b (`Terrain · GoL` → `Terrain · Automaton
couplings`) because a group string is prose and costs nothing. The FIELD
names were not, and the difference is the mirror.

## TENSE_0 — THE TREE IN THE PRESENT TENSE (landed; three tails open)

The prose campaign landed on `master` at `63b87186`, full account in
`docs/TENSE_0_REPORT.md`, entered in `docs/LAWS.md` beside PRUNE_1. Nine
text gates green, the room rebuilds byte-identically (L33), S-6 green at
the pushed tip. **This row exists because a landed campaign that leaves
open state owes that state a home here and nowhere else (L32).** The
handoff scoped OPEN.md's EXISTING rows out of the campaign; it did not
address the state the campaign itself created, and a campaign report is
not a docket.

**1 · `CellIdentity` — JEAN'S RULING, evidence gathered, nothing edited.**
The struct is 22 fields / 34 scalars, built once per cell and passed BY
VALUE across four function boundaries. Eight fields have been
authority-internal since P2 and **no consumer reads any of them** —
member-access census (`\.<field>\b`) returns ZERO for all eight. The
handoff's proposed test (`rg -n 'parity'`) returns **22** and would have
misled the ruling; it collides with locals in
`discrete_cell_color_at_tier`, with orb code and with the ribbon's tick
parity. The freight is **18 of 34 scalars, 52.9%** — larger than the
handoff claimed. U7 did not execute and is not in the tree.

**2 · The occupier limb — deferred, named so it is not lost.** Whether
`row_occupier` / `occupier_contact` and the three `OCCUPIER_*` consts
keep standing for a future family or retire with the arch. TENSE_0 U3b
repaired their PROSE only, which is honest either way, so the question
costs nothing while it waits.

**3 · The probe.** U1 struck three WGSL functions and U2a renamed a
`DesignConfig` field, so the shader's token stream moved. Nine green text
rows are not the acceptance (L48): `the-board --probe=N` at the tip is,
and it is Jean's. G-LAW 1 likewise was not run here.

**And the name is provisional.** TENSE_0 is F1-class; the naming gate is
Jean's.

## ORGAN_REST — THE ORGAN'S PAPERWORK (landed; four tails open)

Landed on `master`, full account in `docs/ORGAN_REST_REPORT.md`. The ruling
was right and the diagnosis mostly was not: three of the campaign's four
named taxes do not exist in this tree, and the tree said so in files the
handoff could not read when it was written. **The organ was never the drag;
the belief about it was.** Same reason this row exists as TENSE_0's: a landed
campaign that leaves open state owes that state a home here (L32), and the
handoff's U7 asked only for a report.

**1 · U4 — QUARANTINED, and it is Jean's to overrule.** "Regenerate
`audit/ORGAN.md` at release tags only" did not run, on three independent
grounds. The book is one of the five files L33's rebuild ritual deletes and
regenerates to prove the room is byte-reproducible, so a stale one fails a
standing witness. `git tag` returns three tags and all three are attic
markers — this repo has never released, so the cadence anchor does not exist
and "release tags only" means never again. And the cost is zero: the book
carries no provenance stamp, is a pure function of the enrolled rows, and
went byte-identical across this campaign's own three comment-only commits.

**2 · The U6 census — Jean's ruling, nothing removed.** 324 rows classified
A 5 / B 46 / C 14 / D 259, D being the default bucket. Row ids are
stored-preset keys and block ids are the seam's wire contract, so trimming is
cheap now and expensive once anything outside this tree has saved a scene.
The census recommends nothing and this row does not either.

**3 · U1 and U3 are DRAFTS awaiting Jean's pen**, marked so in their commit
subjects. They move charter prose into the enrollment file's voice, which is
a naming-class act.

**4 · Two defects found in passing, neither this campaign's subject.** The
chain constraint at `draw_ring`'s row reads `EXIST(350) > ring > lod0, and
ring − band > lod0` — a LIVE range constrained against two RETIRED terms,
whose homes are now `_pad_lod0_radius_retired` and `_pad_grain_band_retired`.
And `organ_registry.hpp` says "The five forwards below are the price" above
four forwards.

**The name is provisional.** ORGAN_REST is a proposal; naming is Jean's gate.

## HEM_1 — THE HEM REACHES THE WALKERS (landed; two deferrals, one ruling)

Landed on `master`. The pawn obeyed two laws the thirty-one free agents did
not — the wall (`world_box_clamp_xz`, inset by the body's own radius) and the
slope law (`pawn_ground_resolve` → `slope_passable`). HEM_1 gives both to the
agents, and fixes the thing that made the first survivable. **Zero mirrors,
zero bindings, zero organ rows**: every radius and constant it needed was
already bound where it was needed, and the four "must be a no-op" gates
(`binding_gen --check`, `mirror_offsets --check`, `organ_gap --gate`,
`organ_ledger --check`) were the structural proof.

**1 · THE ASYMMETRY THAT WAS.** The pawn clamped to the box at
`config.pawn_body_radius` and resolved its ground through the slope law; an
agent integrated, snapped its `y` to whatever the ground answered, and walked
on. A 3.125 wu cell lifting `alive_height` (~24 wu) is a grade of ~7.7 against
`PAWN_MAX_SLOPE` 1.75 — the pawn is stopped by it, an agent teleported up its
face. Agents now stop, slide, or revert on the same law with the same
constants; blocked zeroes velocity, and heading is deliberately NOT recomputed
so a blocked agent keeps facing what stopped it.

**2 · THE SPAWN ANNULUS WAS DRAWN ENTIRELY OUTSIDE THE WALL.** The annulus is
200–340 wu from the point; the box's half-width is 75 at `finite_radius` 1 and
225 at radius 4. At radius 1 **no sampled position was inside the world at
all**. Landing the clamp alone would have slammed thirty-one figures onto the
wall and held them there — which is why the placement fix shipped in the same
commit as the clamp, not as a follow-up. The rule is SCALE, THEN CLAMP: scale
the annulus by the box's inradius, then clamp with the same margin the shader
insets by. Rejection sampling was refused — the salt is frozen and a variable
number of draws is not a frozen salt.

**3 · PLACEMENT DETERMINISM MOVED, KNOWINGLY.** `ONE_WORLD-II U1c` recorded
that agent placement is bit-for-bit against the frozen salt. In a FINITE world
every agent now lands somewhere else; that is the unit, not a regression. The
infinite arm (`finite_mode == false`) is byte-identical — `walled` is false,
`fit` is 1.0, and no clamp runs.

**4 · THE EVICTION IS ACCEPTED, NOT FIXED.** `AGENT_EVICTION_RADIUS` is 350
against a box whose diagonal is **212 wu at radius 1 and 636 at radius 4**. Once
the wall holds, eviction cannot fire in a small world and fires only near the
far corners of a large one. The spawn-far / walk-in / evict-out economy is
replaced by a RESIDENT POPULATION, which is the coherent reading of a walled
world: nobody leaves, so nobody needs replacing. Scaling the radius by the fit
factor would need that factor GPU-side — a `GPUDesignConfig` field, a mirror
growth and an organ row — which is the one property this campaign held. A
future reader wondering why `respawn_evicted_agents` is quiet finds the answer
here rather than a bug.

**5 · DEFERRED — THE SUPPRESSION CENTRES. Jean's, unmade.** Agents do not carve
the cell field. The pawn is mobile through it because it carries a suppression
bubble and the picture carves the same bubble in the two patch VS, so floor and
picture agree. The carve supports exactly TWO centres (the pawn and the eye,
`max()`-ed); thirty-two would mean a per-vertex loop over the agent array in
both patch VS **and** in `automaton_evolve`'s gather. **Suppression is a
PRIVILEGE, not a constraint** — Jean asked for the constraints. Agents obey the
field the picture already draws them standing on, and THE FLOOR IS THE PICTURE
holds for them unchanged.

**6 · DEFERRED — GoL IN THE WHISPER. Named, not scheduled.**
`sample_terrain_grad_at` reads the BAKED heightfield — static base and pyramids
only. Cell walls are invisible to the steering, for pawn and agent alike, so
agents meet cells with no anticipation by construction. A cell-aware whisper
would need a gradient of the live card.

**7 · DEFERRED — PER-FIGURE AGENT RADIUS.** The wall margin is the tier's
`contact_radius`, not `PAWN_FIGURES[skin_id].radius`. The latter is the truer
twin but `scene_constants` is `@group(2) @binding(200)` VERTEX-only and
unreachable from compute; reaching it means a new binding or a mirror growth,
refused on this campaign's zero-surface property. If `scene_constants` ever
reaches compute for another reason, the margin should move and become the
pawn's exact twin.

## SKIRT_WELD_1 — THE PERIMETER SKIRT HANGS FROM THE BASE BAND (landed; one seam held)

Landed on `master`. The patch perimeter skirt hung its top edge on **cap**
verts, and cap verts are CELL-OWNED (UNIFIED_GROUND_1): ring slot `k` and
`k+1` straddle a cell seam at every multiple of `UG_QUADS_PER_CELL`, so one
live cell turned that quad into a ramp from `ground + alive_height` down to
`ground` — an extra triangle dragged up by a cell it does not belong to, over
a curtain that had already sealed the same edge. The ring now hangs from the
**base** band, whose twins carry `wall = 1 → lift_scale = 0` (WALL_1) and
never lift. **Zero mirrors, zero bindings, zero organ rows, and the four index
counts are byte-identical** — the emission pushes six indices per quad either
way, so only the index VALUES moved.

**1 · THE INVERSE IS ASSERTED, NOT ASSUMED.** `cell_perimeter_slot` (the new
`(lx,lz) → k` inverse of `cell_perimeter`) is `constexpr` and carries a
sixteen-term `static_assert` that is simultaneously a round-trip check and a
bijection proof: the sixteen results are `0..15`, each exactly once. The
right-hand column is `cell_perimeter`'s own emission, stated as literals
because `cell_perimeter` is a runtime lambda and cannot be called from a
constant expression without editing it, which the campaign froze.

**2 · BOTH LODs GOT THE SAME RE-AIM.** `build_lod0_ib` (zoned and cap-only)
and the LOD1 stride-2 ring are one lambda between them; the curtain-less
variants are the lift-conservative switch, not dead alternates, and a fix
landed on only the zoned buffers would have passed every gate.

**3 · THE TINT'S VARIATION IS A GAIN NOW, NOT AN OFFSET.**
`apply_automaton_color`'s BLACKISH branch added `vec3(r_shift, g_shift,
-r_shift)` **after** the darkening multiply. On near-black ground the base
term is ~0, so the offset became the whole colour and the `clamp` beneath it
deleted G and B asymmetrically — a cell holding max `r_shift` painted
saturated red out of black ground. As a gain the scatter is proportional to
what is there. The clamp is EXCISED rather than kept: the ceiling is
`0.55 * 1.05 = 0.578` against a base ≤ 1, and the floor is ≥ 0 because every
factor is, so the clamp had become a lie about what the line can produce.
Neither `_SHIFT_RANGE` value moved — zeroing them would have hidden the class
and left the census reading clean on a lie.

**4 · TWO RESIDUALS, NAMED AND LEFT.**
- `in.skirt` was `0 → 1` across a skirt quad (cap top, ring-copy bottom) and
  is now the constant `1` (both ends carry `wall = 1`). `DEBUG_VIEW = 3u`
  therefore paints the whole skirt quad magenta including its top edge —
  truer as an instrument, and a changed picture in that debug view.
- The per-triangle `cell_local` step at a cell seam SURVIVES. `cell_local` is
  `@interpolate(flat)`, whose provoking vertex is the primitive's first, and
  the emission is `a, b, sa` / `b, sb, sa` — so triangle 1 provokes from `a`
  and triangle 2 from `b`. At a seam slot those two belong to adjacent cells,
  and the two halves of one quad shade from two different cells. It was true
  before the re-aim and is true after; what the re-aim removed is the ramp
  the step was riding on.

**5 · HELD — THE SHADING NORMAL EXCLUDES THE AUTOMATON'S LIFT.**
`patch_terrain_vs` sums `out.gradients = height_data.yz + live.yz`; the GoL
lift rides the card's `.w`, nearest-sampled at the cell centre, and
contributes nothing to the gradient. Every cap and curtain is therefore lit
with the flat ground's normal. Real, second-order, and superseded by
SKIRT_WELD_1 — the picture must be re-shot with the ramps gone before this is
scoped, because the ramps were producing tone splits of their own.

**6 · THE RECORD RITUAL WAS NOT RUN, DELIBERATELY.** The handoff asks for
`glaw2 --record`. One name retires here — `skirt_cap_index` — and it is
**C++**, which glaw2 does not see; the campaign adds no WGSL entry point and
no WGSL const, and glaw2 is GREEN without a re-record. `--record` rewrites the
whole baseline including the `declared` retirement ledger (RETRACT_0 R10 flags
it Jean-gated and destructive), so running it here would have rewritten a
ledger over a campaign that moved nothing glaw2 tracks. The tombstone for
`skirt_cap_index` is the diff's.

## CHOIR_0 — THE LIGHT INSIDE THE CUBES (landed; one divergence, three flags)

Landed on `master`. **Working name — naming is Jean's gate.** The function
that lights the cubes with the music is rewritten from the ground up. `ch6`
is cast as the cube voice; its `present_count` — twelve lanes, the count of
sounding notes per pitch class — plays a keyboard of `CUBE_CHOIR_N = 36`
cubes read as stacked pianos, key `k` being rank `k/12` of raw pitch class
`k%12`. Each active key lights one cube from within: a saturating attack
(τ = `light_plateau`/4, ≈ 0.982 at the 8-beat plateau, steepest at
switch-on) and a fixed-slope release (full brightness to dark in
`light_release` beats). **Activation and deactivation only** — no
held-length book is kept, because the envelope IS the memory.

**KEY k = SLOT k, BY CONSTRUCTION.** `run_spawn_preamble` reserves the
lowest free slot, so capping `CUBE_TRAITS.max_instances` at `CUBE_CHOIR_N`
keeps the population dense in `0..N−1` and an evicted key's refill relights
the same dark key. There is no mapping table and no registry: the identity
is the law. Flipping to two ranks is the one token `CUBE_CHOIR_N = 24`; the
static_assert pair refuses anything else.

**ZERO WGSL EDITS. `world.wgsl` IS BYTE-IDENTICAL.** Colour and face
variance are buffer facts written through the existing partial-write doors
(`upload_cube_color`, `upload_cube_face_variance`). `canvas_1` already
published `PresentCount` for every voice — the coupling simply took one of
the atlas's unheard names. `Reading::PresentLength` stays an orphan by
Jean's ruling; no `publish_reading` was added.

**ZERO ANALYSIS MECHANISM EDITS, AND ONE COMMENT.** §0(a) said "ZERO
EDITS" of the analysis side and explained it as "no `publish_reading`, no
writer case, nothing" — and no reading, slot, writer or publication moved,
not one executable token. **One comment did**: `canvas_1`'s `[ONSET]`
witness justified itself as one half of a pair with the board's
`[ZOETROPE]` strike line, which this campaign deleted. It is named in the
report as D7 rather than done quietly, because the handoff's word was
"zero".

**WHAT DIED.** The zoetrope's LATTICE, whole: `ZoetropeCell`, `cells[]`,
`cell_scratch[]`, `wdir[][]`, `primed`, `last_tick_beat`; the AUTOMATON
console band with its composite-retention law; `zoetrope_strike` and its
write head; the tick's diffusion/decay pass, the prime pass, both clock
guards and the per-tick projector flush inside `zoetrope_service`;
`zoetrope_cell_intensity`, `project_cell_color`, `zoetrope_project_slot`;
`slot_of_cell`; `ZOETROPE_PIGMENT_R/G/B/WEIGHT`; `ZOETROPE_FACE_SPLAY` and
`ZOETROPE_FACE_REST`; and canvas-side `ZOETROPE_EARS`, `ZOETROPE_ROW_OF_PC`,
`zoetrope_rows_[7]` and the seven-ear resolve. **The seven onset
publications STAY published** (capability doctrine): they lost their only
reader, not their right to exist.

**WHAT SURVIVED, AND IT IS MOST OF THE FILE.** The formation machine
entire — `reveal_zoetrope` (Door 5), the two stations, the walk, the reseat
watch, `stage_wait`, `repaint_all`, the settle law, the hand-back. Doors
4/5/6 and the kite law. `LATTICE_ROWS/COLS/CELLS`, the helix constants and
BOTH their static_asserts (the bijection is still proved; only the inverse
WALK retired with the strike that used it). The screen bands, `SWELL_GAIN`,
`REST_DIM`. `zoetrope_service` keeps its name because the zoetrope's BODY
is the formation machine; only its SUBSTRATE was the lattice.

**1 · THE DIVERGENCE — THE PIPE'S BASE IS 15, NOT 16.** The handoff says
`{ "cube.light", 16, 36, 0.0f }` and, in the same sentence, "the first free
run" and "15/256 → 51/256 allocated". The terrain run ends at slot 14, so
the first free slot is 15, and 15 + 36 = 51 is the stated total; base 16
would leave slot 15 an unexplained hole in a hand-laid register map and
total 52. Two of the three signals say 15 and they are the two that are
internally consistent, so the pipe sits at 15. **Trivially movable if the
16 was deliberate** — one number in `PARAM_LAYOUT` and the cartridge
re-resolves by name.

**2 · FLAG — THE FACE VARIANCE'S REST MOVED, VISIBLY.** The old law was
`draw · FACE_REST + FACE_SPLAY · I` with `FACE_REST = 1.20`, so a cube
standing dark in ANY formation wore 1.2× its spawn draw; the new law is
`draw · (1 − I)`, so it wears 1.0×. This is the commission's own law
(`var = spawn_draw · (1 − I)`, and FACE_REST explicitly retires), and it is
what makes the law self-restoring at I = 0 with no restore pass — but it is
a **percept change on a standing screen with the music silent**, not only
on a lit one. Jean's visual desk owns it. Restoring the boost would cost
the self-restoring property, which is why it was not quietly kept.

**3 · FLAG — THE POKE GATE REPLACES A TICK.** The lattice's flush hid
behind a 0.25-beat tick; the choir has no tick, so `choir_project` runs
every frame and gates on the LIGHT (`CHOIR_FLUSH_EPS = 1e-3`). At 120 BPM /
60 fps the attack stops poking about two thirds through its plateau and the
release pokes every frame it lasts, so the worst case is one poke per
SOUNDING key per frame — 36 twelve-byte colour writes against the old
≤252-slot sweep every quarter beat. **Silence pokes nothing.** The birth
path seeds the gate (`cube_write_gpu`) and `clear_cubes` resets it, which
together are what make "the same dark key relights" true at the GPU as well
as at the slot.

**4 · FLAG — TWO DIALS PARKED, NOT ENROLLED.** `CANVAS_LIVE.light_plateau`
/ `.light_release` (15 → 17 floats) and `DRIVER_LIVE.cube.light_color` /
`.gain` (18 → 22 words) are authored in their structs with design values
and carry NO `organ_params.inc` row — the ORGAN_REST registry freeze holds
until the Ableton seam campaign. Four `ORGAN_PARAM` lines when it lifts:
two `ORGAN_PARAM_NS(canvas, CANVAS, …)` beside the checker cadence rows,
and two `ORGAN_PARAM(DRIVERS, …)` beside the checker's gain. `organ_gap
--gate` and `organ_ledger --check` are both PASS with them unenrolled — an
unenrolled field is not a suspect, it is simply not a dial yet.

**5 · THE RECORD RITUAL, PER SKIRT_WELD_1's PRECEDENT.** The handoff asks
for `glaw2 --record`. **Every retired name here is C++**, which glaw2 does
not see: no WGSL entry point, const or struct moved, and glaw2 is GREEN
without a re-record (`18 symbols retired cleanly`, unchanged). `--record`
rewrites the whole baseline including the `declared` retirement ledger
(RETRACT_0 R10: Jean-gated and destructive), so it was not run. **Every
retired name is claimed by a tombstone in the diff** — the ritual's actual
requirement — and the WGSL gate is a genuine no-op, stated rather than
assumed: `world.wgsl` is byte-identical.

**6 · THE POKE GATE COST ONE REAL BUG, CAUGHT AND FIXED.** An adversarial
pass over the finished diff found it: the climb ends by snapping
`body_radius` to the BARE pixel, and the SWELL lives in the projector, which
is gated on the light MOVING — so a cube arriving on the screen **under a
held chord** would stand unswollen until its key next changed. The lattice's
flush hid the whole class (it ran unconditionally every 0.25-beat tick).
**The arrival now declares itself**: the settle raises `repaint_all`, and a
newborn under a STANDING screen is dressed with the swell as it is dressed
with the light. Recorded here because it is the general shape of what a
cheaper flush buys — **any state change that alters a cube's look without
moving its light must raise the force**, and there are now three that do:
the dim's two edges and the arrival.

**7 · OPEN — THE PROBE.** Every text gate CC can run is green (see the
report), and green is not a world that draws. `the-board --probe=N` and the
visual desk are Jean's: the light colour triple, the attack's shape at
τ = 2, the variance convergence, and the **sparse screen percept** — 36
cubes seated through the helix across a 7×36 geometry is a much emptier
screen than 252 was, and that is the cap's intended consequence, not a bug.

## THE HANDOVER LIST (THE_PANEL's close — the coupling campaign's table)

**THE WRAP ORDER §2.3 asks that the next campaign's handoff be authorable
from the wrap report and the atlas ALONE, with no fresh recon round.** This
is the durable half of that report: the same list, in the register that
outlives a chat.

### 1 · THE TWO DOCUMENTS THAT ARE THE SUBSTRATE

| what | where | what it carries |
|---|---|---|
| **THE COUPLING ATLAS** | `docs/COUPLING_ATLAS.md` | five tables by symbol — SOURCES (55 published, **6 heard, 49 unheard** since CHOIR_0; it was 12/43), PIPES (PARAM_LAYOUT's **nine**), THE CHOIR (one voice, 36 keys — the table that was EARS, the zoetrope's seven), THE UNCOUPLED NEW WORLD (every candidate dial **with its cadence**), THE ORPHANED |
| **THE SEAM SCOUT** | the atlas's own appendix | the native analysis side as it stands — the clock end to end, the loopMIDI lane, `StatLayoutView`'s publish/bind seam, `AnalysisSignal`'s shape — and the three transports with costs. **No transport chosen: that is Jean's.** |

### 2 · THE LIVE DIAL INVENTORY, WITH CADENCES

**326 enrolled rows across 15 blocks.** By cadence: **boundary 106 · live
156 · gen 50 · driven 14**. The generated book is `audit/ORGAN.md` and it
is the authority; this table is the shape a composer needs.

| block | rows | cadence | what a voice into it would be |
|---|---|---|---|
| `AGENT_ROOM` | 102 | boundary | the agents' tier/behaviour definitions |
| `CONFIG` | 82 | live 72 · driven 10 | the GPU design mirror — **the largest live surface** |
| `AUTOMATON` | 22 | **gen** | the ground's whole vocabulary |
| `RIBBON_SPAWN` | 20 | **gen** | the next ribbon drawn |
| `ORB_BANK` | 19 | boundary | the sky's one orb row |
| `ATMOS` | 15 | boundary | the sky as a distribution |
| `CANVAS` | 15 | live | the coupling layer's own dials |
| `DRIVERS` | 14 | live | the rests and gains at the seams |
| `PAWN` / `PANEL` | 9 / 9 | live | the aura; the field and the input grammar |
| `AGENTS` | 5 | **gen** | how many walk this world |
| `LIGHTING` | 4 | driven | witnesses — the drawn sun |
| `ORBS` | 4 | boundary | the orb console |
| `WORLD` | 3 | **gen** | the seed and the radius range |
| `RIBBON` | 3 | live | reference bpm, the mount's eases |

**CADENCE IS THE COMPOSER'S REAL CONSTRAINT, and it is DERIVED, not
chosen** (`derived_cadence()`, one home). A voice into a `gen` row is
heard once per world and needs a DOOR to land; a `driven` row refuses a
voice outright. **Every automaton dial is `gen`** — so the ground is a
composer's instrument, not a performer's, until a door is bound to an
event.

**SEVEN DOORS**, and they are the layer's unexplored mechanism: every
coupling today is a continuous value, and a door is where it could speak
in EVENTS. `RESPEAK` · `ORB_RULE` · `ORB_GESTURE` · **`REBIRTH`** ·
`CUBE_BEHAVIOR` · `ZOETROPE` · `CUBE_KITE`.

### 3 · THE SURFACE THE CAMPAIGN INHERITS

- **One write road**: `organ_set`, whitelisted by the manifest, clamping
  floats AND integers, naming every refusal. The REPL, the scene file and
  any future CC map are doors onto it — none is a second author.
- **Two consumers already built**: `--scene=<file>` (watched) and a stdin
  REPL. Both name-blind: **a new dial is one line in `organ_params.inc`
  and zero lines in the shell.**
- **A gate that RUNS**: `tools/gates/shell_gate/run.py`.
- **39 named absences** remain across the enrolled homes (`organ_gap.py`),
  the largest being `RibbonSurface`'s 16 — and **every one of those moves
  MOVEMENT**.

### 4 · THE PARKED CC-MAP SPEC

Stated in full at THE PANEL's parked-skins block below, and its shape is
settled by the seam scout: **CC# → `block.field` through `organ_set`**, a
map plus a callback on the existing MIDI lane, no new dependency, zero GPU
seams. **The design question it must answer is cadence** (see 2 above),
and the atlas tags every candidate dial for exactly that.

### 5 · WHAT IS STILL OPEN, EVERY LINE

| section | state |
|---|---|
| **N-a DAWN ACQUISITION** · **N-b ARM RE-INSERTION** | open — Jean's, and upstream of everything |
| **THE DEVICE GATE** | open — the probe is unproven code until Jean runs GREEN at the tip and RED at `2905ed68` |
| **ONE_SURFACE-II — THE AUTOMATON** | the close is Jean's |
| **NATIVE PRESET INGESTION** | the reader is BUILT (`--scene=`); what remains is whether `presets/` earns its keep once scenes are exported from the REPL |
| **THE BUDGET AFTER THE AUDIENCE** · **THE DEBUG ARMING** | open, both Jean-gated |
| **THE RADIAL PULSE RING** | open — gen-1 retired, decision unmade |
| **CUT_1c LEFTOVERS** · **DOC NITS** | open, low |
| **THE TIME SOURCE AFTER THE SPLICE** | open — Jean-observed |
| **THE GoL VOCABULARY OUTLIVED THE GoL** | open — three renames priced, wire-checked, naming is Jean's |
| **THE ABLETON SEAM** | held — and it is what the next campaign collects |
| **THE PANEL** | I and II landed; three PROBE-PENDING flags await one verdict |

**What remains after the probe verdict and the walk is exactly the musical
horizon** — the seam, the CC map, the coupling vocabulary. Which is the
reason the fork exists.

## THE ABLETON SEAM (held, after N)

> **THE MAP FOR THIS SEAM IS `docs/COUPLING_ATLAS.md`** — five tables,
> every row by symbol, cadence on every candidate dial, and **THE SEAM
> SCOUT as its appendix**: the native analysis side as it stands (the
> clock end to end, the loopMIDI lane, `StatLayoutView`'s publish/bind
> seam, `AnalysisSignal`'s shape) and the three candidate transports —
> Ableton **Link**, **MIDI** through the existing lane, **DAW audio
> loopback** — each with what exists in-tree, what it adds, which seams
> it touches, and its cost. Linked, not restated (L46): the atlas is the
> one home for what the coupling layer is, and the campaign that collects
> this seam is authored on it. **No transport is chosen there; that is
> Jean's, on those facts.**
>
> **The one fact worth carrying up here**, because it changes what the
> seam IS: **the world already has counted, phase-locked musical position
> from an external timeline.** `MidiTransport` counts 0xF8 pulses, 24 per
> quarter, and `transport.hpp` says outright that "tempo is estimated only
> for display and is never used to advance the beat". So the seam is not
> about getting TIME. The gaps are the RETURN DIRECTION (the MIDI lane
> reads and never writes) and the LISTENERS (43 of 55 published sources
> are unheard). Both are reachable without a new dependency.
>
> **A CUE ARRIVED FROM ONE_SURFACE-II** (recorded in full at that
> section): the automaton's `tick_period` is in BEATS and is one dial, so
> stepping the ground on the bar is a matter of pointing `beats` at a
> transport this seam opens. Named here so the coupling campaign finds it
> where it will be looking.


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
`--seed/--cap/--msaa` loop behind it. (`--mood` was the fourth until
ONE_WORLD-II U2 — a world is chosen by its seed alone now.)

## THE PANEL (the plan's last movement — TWO HANDOFFS IN HAND)

### U0 · RECON — DONE. **U1+ HELD ON JEAN'S PROBE.**

The handoff's own precondition is "RUN AFTER ONE_SURFACE-II CLOSES GREEN
(probe included)", and U0's first line is "verify the automaton's close
(probe green at its tip)". **CC cannot verify that**, for the reason
THE DEVICE GATE states: no built Dawn, no display. So the recon is done
and recorded — it needs no probe — and every constructive unit waits.
Same shape as ONE_SURFACE-II's own hold, which the rider resolved by
Jean running the thing.

**THE CENSUS, item by item as §1 asks:**

**1. The web shell is ALREADY GONE.** `web/` does not exist — it left
whole at WEB_SUNSET, not just its build. So the "room full of lies"
U0 was told to look for (an `organ_panel.js` whose cwrap list names dead
exports like `organ_mood` and `organ_regime`) IS NOT THERE, and
**Handoff II's §1.4 — "The web shell dies (if standing)" — is discharged
before it begins.** II's U3 has no subject. What it should do instead is
the half that survives: the shell-gate witness §1.4 mentions, if any
gate still names the file. (None does; `web/` appears nowhere in
`tools/`.)

**2. THE ABI AS IT IS — fourteen exports, all `extern "C"`, all
KEEPALIVE'd** (`src/console/organ_registry.hpp`):

    organ_manifest        organ_doors           organ_set
    organ_get             organ_def_get         organ_param_count
    organ_rejected_count  organ_last_reject     organ_flush_count
    organ_build_stamp     organ_door            organ_host
    organ_go_host         organ_orb_rule

No dead export among them: `organ_mood` and `organ_regime` retired with
their facts (ONE_WORLD-II). `bind_home(&gpuState_)` is called once at
cartridge init, and it is the gate that makes `organ_set` a dial rather
than a memory editor. **THE MANIFEST IS THE WHITELIST** — `organ_set`
refuses any (block, offset) the manifest does not carry, by name, into
`organ_last_reject`.

**THREE DOORS**, `kOrganDoors[]`, one row per id with a static_assert
holding the count: `Re-speak definitions`, `Cycle orb rule`,
`Cycle orb gesture`. Presses coalesce through a bitmask by construction.

**Handoff II's acceptance test is therefore already TRUE of the ABI and
only untrue of the SHELL**: `organ_manifest` emits the whole vocabulary
and a name-blind reader needs no per-dial code. The road and the hand
are what is missing, not the surface.

**3. THE ENROLLMENT GAP — 311 enrolled rows and 53 named absences, of
which 15 were invisible until this recon fixed the map.**

Rows by block: AGENT_ROOM 102, CONFIG 88, RIBBON_SPAWN 20, ORB_BANK 19,
canvas 15, ATMOS 15, DRIVERS 14, PAWN 9, PANEL 9, AGENTS 5, ORBS 4,
LIGHTING 4, RIBBON 3.

`tools/organ_gap.py` reports **53 absent across the enrolled homes**:

| home | named / total | the absences |
|---|---|---|
| **`AutomatonBank`** | **0/15** | **15 — the whole ground.** algorithm, rule_mask, field_fn, color_mode, boundary_mode, density(+spread), tick_period(+spread), transition_fraction(+spread), alive_height(+spread), spring_variance, phase_randomness, tempo_randomness, height_factor mean/sigma/lo/hi, target, mode_threshold |
| `RibbonSurface` | 3/19 | **16** — yaw_rate, max_speed, r_min, climb_rate, floor_margin, alt_smooth_dist, alt_stiff, mount_setback, sky_yaw_tau, lookahead, clear_head, clear_body, roam_radius, wander_soft, wander_yaw_max, wander_arrive |
| `GPUDesignConfig` | 79/91 | 12 — and most are NOT dials: world_seed, world_bound_min/max, cull_point_x/z, placement_patch_count, possessed_slot, point_host, pulse_count, pulse_data are STATE the program authors, not knobs a hand turns |
| `OrbConfig` | 18/24 | 6 — base_hue, hue_variance, motion_rule, hue_converge_target, tierset_id, flock_gesture_default |
| `AgentPopulationBank` | 5/7 | 2 — behavior_weights, tier_weights (arrays) |
| `PawnAuraProfile` | 8/9 | 1 — effect_mask (STATUS: INTENT, mirrored and never read) |
| `CubeBank` | 0/1 | 1 — behavior_weights |

**THE RIBBON IS THE HEADLINE**: sixteen absent of nineteen, and every
one of them moves MOVEMENT — yaw rate, climb rate, roam radius, wander.
"Every single knob involved in the parametric generation of geometry,
coloring and movement" is Jean's sentence, and this is the largest
single block of movement knobs the panel does not name.

**THE BIGGEST GAP WAS INVISIBLE TO THE TOOL, AND THIS RECON FIXED THAT
BEFORE REPORTING IT.** `AutomatonBank` was not in `organ_gap.py`'s file
table at all — it was born at ONE_SURFACE-II U1, after that table was
last written, and **a home absent from the table is a gap that reads as
ZERO.** That is blind spot 2's exact failure mode, which is why the tool
prints the table it trusted; printing it is what made this findable.
The row is added (the tool is a MAP, not a gate — its own header says
so), and the total moved 38 -> 53.

Not one of the bank's fields is enrolled: the rule mask, the density,
the tick period (which is the Ableton cue's own dial), the transition
fraction, the alive height, the colour mode and target, the three
per-cell scatters, the height-factor draw. **The ground's whole
vocabulary.** Enrolling it is U3's largest single item.

**Candidates are FACTS for Jean's gate, not automatic enrollments** —
"every single knob" is his sentence to apply, and several of the 38 are
correctly absent (state, not knobs; INTENT mirrors; arrays the manifest
has no row shape for).

**4. THE DESIGNCONFIG PAD MAP — COLLECTED AND STRUCK AT U2.** The recon
read it as "704 bytes, at least nine retired pads" and listed them:
`_pad_pier_retired`, `_pad_terrain_amp_ceiling_retired`,
`_pad_ceiling_height_retired`, `_pad_veil_dither_retired`,
`_pad_indoor_height_cap_retired`, `_pad_veil_strength_retired`,
`_pad_lod0_radius_retired`, `_pad_arch_slack_retired`, `_pad720_1/2`.

**FOUR COULD GO, AND THE ARITHMETIC — NOT TASTE — PICKED THEM.**
`704 -> 688`, both rooms, one commit. The four are the two ONE_WORLD-II
U4 pads and the two repurposed pulse-pad floats: they are 16 CONTIGUOUS
bytes between `placement_patch_count` and `pulse_data`, which is
`array<vec4<f32>,8>` in the WGSL room and therefore 16-aligned, so
removing all four moves it by exactly one stride and every 16-byte
boundary in the struct survives. Removing two would have moved it by 8
and parted the mirror silently.

**THE OTHER FIVE ARE NOT DEBT, and the recon's "at least nine" was
optimistic about how much headroom the alignment allows.** Three of them
— `_pad_sun`, `_pad_fog[2]`, `_pad_pier_retired` — ARE the WGSL room's
implicit vec3/vec2 padding written down; removing them parts the mirror
outright, and `_pad_pier_retired`'s name is the only thing about it that
reads as a retirement. `_pad_veil_strength_retired` and
`_pad_lod0_radius_retired` are 8 bytes between `pulse_data` and
`palette_center`, so taking them pushes `palette_center` to 8 mod 16.
And the tail's `_pad_arch_slack_retired` + `_pad720_1/2` cannot shrink
the struct at all: `alignas(16)` rounds whatever is left back up, so the
declared pads would simply become invisible ones.

**THE NAMED HAZARD WAS REAL AND WORSE THAN NAMED.** The pin read
`offsetof(cull_point_x) == 336`, with the reason "cull_point_x offset
must be 336 for targeted upload" — and that reason was FALSE: the
destination offset is taken by `offsetof` on the next line, so 336
pinned nothing the call needed and would have fired on any relayout as a
false alarm. What `upload_cull_point` actually assumes is ADJACENCY — it
writes EIGHT bytes from `&cull_point_x` — and NOTHING asserted that. The
pin is re-derived as the relation, and **proven to bite**: splitting the
pair fails the build with the adjacency message while an absolute pin on
`cull_point_x`, injected beside it as the control, stays green.

**AND THE MIRROR GAINED A WITNESS IT NEVER HAD.** The binding ledger's
`0b-4` runs a WGSL layout calculator over every struct a `BYTE-FOR-BYTE
(N B` marker registers. Six structs carried the marker; **DesignConfig
— the largest mirror in the tree and the one every campaign since
PRUNING_1 has relayouted — did not**, so the `sizeof` assert pinned it
in ONE room and nothing in the battery computed the other room's answer.
The marker is written now: both numbers read 688, derived by two
instruments from two files.

### AND WHAT 0b-4 STILL CANNOT SEE (found by injecting against it, U2)

Three WGSL-only injections, run against the new marker:

| injection | verdict | why |
|---|---|---|
| append a field | **FIRES** — "prose says 688, calculator says 704" | the growth case, which is what the GROWTH LAW is about |
| drop a TRAILING pad | blind, and correctly so | WGSL rounds the struct up to its 16-byte alignment and `alignas(16)` does the same, so both rooms absorb it identically and no divergence exists |
| drop a MID-STRUCT pad before a 16-aligned member | **blind, and this one is a real break** | WGSL re-pads implicitly to put `palette_center` back on 16, so the SIZE is unchanged while every offset from `veil_ring` to `palette_center` differs between the rooms |

**The third row was the open finding, and U6 CLOSED IT.**
`tools/mirror_offsets.py` derives every member's offset from `world.wgsl`
by the WGSL layout rules and emits one `static_assert(offsetof(...) == N)`
per member into `mirror_offsets.gen.inc`, which `state.hpp` includes after
the mirrored structs are declared. **128 member offsets across the seven
marker-registered structs**, checked by the C++ compiler — so the two rooms
prove each other rather than each proving itself against a number a person
typed. The marker is the enrollment, exactly as it is for `0b-4`: a struct
joins by stating its size in its own banner, which L3 already asks for.

**Proven to bite, on the exact injection that defined the hole.** Delete
`_pad_grain_band_retired` from the WGSL room only: `0b-4` PASSES,
`static_assert(sizeof(GPUDesignConfig) == 672)` compiles clean, and the
per-field witness fails the build on the two members between the cut and
`palette_center`. **U5 hit that case for real one unit after U2 wrote it
down**, and it was caught by reading. It is caught by building now.

**What it does NOT assert, said here rather than discovered later:** a
member the C++ room does not declare under the same name is SKIPPED and
the skip is printed with its reason (this is how the C++ room's explicit
alignment pads are handled — the WGSL room gets them implicitly and
declares nothing); it proves OFFSETS, not TYPES, so a same-size wrong type
is still the reader's job; and a nested struct is walked at the top level
only, its own members asserted when it carries its own marker.

**5. THE PRESET PATH** stands with NO READER: `presets/index.json` and
`presets/baseline.json`, kept against L30's letter by the Phase W ruling
[F2]. That is Handoff II's `--scene=` road, and its own OPEN.md section
(NATIVE PRESET INGESTION) already names the walk.

**6. THE ORPHAN VERBS — DISPOSED AT U4, every lead with a verdict.**

**THE THREE CUBE VERBS GRADUATED TO DOORS.** `cycle_cube_behavior_override`
(id 4), `reveal_zoetrope` (id 5) and `toggle_cube_kite_mode` (id 6) each
press machinery the program still has, each is already a complete
transaction with its own guards and its own printed line, and each was
missing only a caller. Nothing inside them moved to receive one. They are
`ROSTER.cube`-gated at the boundary, like every other cube call site in
the spine: with the family off there are no active cubes and all three
would print "0 cube(s)" — an honest no-op, but a door does not get an
exemption from the rule that a disabled family is not walked.

**`unrecord_entity` WAS ALREADY DEAD, and the parking was stale.** It left
with the entity-ref registry at ONE_SURFACE-I U3 — `EntityRef`,
`entity_refs[]`, `entity_ref_count`, `record_entity` and it, all together,
because their one consumer walked a registry of patches that do not die in
a world built once. The parking was written at ONE_WORLD-II and named a
verb that had already gone. **Verdict: no subject; the line dies here.**

**THE "~40 ACCESSOR LEADS" WERE NEVER ENUMERATED, so U4 enumerated them.**
The census is mechanical, and it was RUN rather than estimated: every
function DEFINED under `src/` whose name appears nowhere else in `src/`
outside comments and string literals. **983 definitions scanned, 127 with
zero call sites.** The verdicts, by subject:

| leads | verdict |
|---|---|
| **26** GPUState accessors (`state.hpp`) | **CUT.** Listed by name at the tombstone in the class. |
| **2** `entity_spread` / `entity_tint` (`entity_pipeline.hpp`) | **CUT**, with their two constants. |
| **1** `cpu_smoothstep` (`seed_utils.hpp`) | **CUT** — the one leaf of that header with no reader. |
| **1** `drawable_table_encoder_witness` | **KEEP, MARKED.** It exists to be uncalled: taking the address of both `draw_table` instantiations forces the TU gate to type-check the bundle encoder's arm. Its own banner says "Never called; emits no code". |
| **14** the organ ABI exports | **KEEP.** Being called from outside the tree is what an ABI IS. A census of `src/` cannot see its callers by construction — this is the census's own blind spot, printed rather than acted on. |
| **6** `main` | entry points. |
| **~10** `core/types.hpp` + `core/cartridge_manager.hpp` + `render/render_cartridge.hpp` + `console/console.hpp` | **NOT THIS CAMPAIGN'S.** A math vocabulary (`dot`, `cross`, `lerp`, `saturate`, `xyz`, `length_squared`), the multi-cartridge framework's own latent surface (`transition_to`, `return_to_hub`, `return_to_previous`, `active_id`), and framework virtuals (`supports_backspace`, `get_pending_transition`). The_board is one cartridge in a frame built for several; cutting the frame's surface because the one cartridge does not use it is the sweep exceeding its subject. |
| **~50** `musical/`, `sources/`, `analysis/` | **NOT THIS CAMPAIGN'S, and NAMED AS THE NEXT ONE'S.** This is the analysis side, and `docs/COUPLING_ATLAS.md` already published exactly this census in its own terms: 55 published names, 12 heard, 43 unheard. An unheard accessor there is not dead code, it is an unconnected pipe — the coupling campaign's whole subject. Cutting them would delete the seam before it is built. |
| **~15** `external/RtMidi.cpp` | vendored third party. Out of scope by the same law that keeps `third_party/` out of every sweep. |

**THE ONE FINDING WORTH THE SITTING.** `entity_spread` / `entity_tint`
carried a banner reading "PRUNE_2 excised that family and the pyramid is
now the only consumer". **The pyramid had stopped calling them too, and
nothing said so.** A probate exists to catch exactly that, and this one
outlived two campaigns because no instrument in the battery asks "does
anything call this". The census that found it is one 40-line script; it is
recorded here rather than added to the battery, because a zero-call-site
scan over a tree with an ABI and a framework tier produces judgement calls,
not verdicts, and a gate that needs judgement is a gate that gets waived.

**7. `rebirth_world`'s P8 SEAM is UNCHANGED and still uncalled** —
`SEAM[spine:P8]`, explicit latent infrastructure. U1's seed door is its
first caller — **and U1 has now landed it, so this recon line is
answered**; the chain's membership moved at ONE_SURFACE-II U1
(`teardown_gol` -> `teardown_automaton`) with one INVERSION recorded
there: `upload_automaton_config` has two callers now, so it is LIVE and
a sweep must not read it as latent.

**Baseline battery at this recon**: every row green at the pushed tip,
L33 rebuilds byte-identically, S-6 clean, and S-8 among the witnesses.

### A DOC-TALLY DRIFT FOR U5 (found by the U1 census's adversarial pass)

`docs/ORGAN.md` and the generated `audit/ORGAN.md` disagree on **four**
tallies, not the three previously recorded — the fourth is `live`, where
the doc says **164** and the tool says **162**. U5's whole unit is
rewriting `docs/ORGAN.md` to native truth, so it is recorded here rather
than patched piecemeal: a hand-written tally beside a generated one is
the thing that unit exists to end.

### HANDOFF II · U0 — ALSO DONE, and it needs no probe either

II's U0 asks four questions and three of them are answerable now. All
four are recorded here rather than in a second sitting, because the
answers change what II builds.

**1. THE CONSOLE TIER'S STDIN STATE: EMPTY.** Nothing in `src/` reads
`std::cin`, `getline`, `stdin`, `read(0, …)` or polls fd 0. **The REPL
has nothing to fight** — the concern §1's U0 raises does not arise. What
it will have to answer is that the frame loop is a busy `while
(console.running())` with a blocking `present()`, so the read must be
non-blocking or on its own thread; that is a design choice for U2, not a
conflict.

**2. THE WATCHER'S PLATFORM SEAM IS ALREADY BUILT — AND IT IS EXACTLY
§1.1's SPEC.** `FileWatcher` (`src/the_board.cpp`) is `watch(path)` +
`check()` over `std::filesystem::last_write_time` with the `error_code`
overload — **one file, one stat per check, no dependency**, which is
word for word what II asks the scene watcher to be. It is polled from
the frame loop every ~30 frames beside the shader reload, and it is
GENERIC: it takes any path.
**So the scene watcher is a SECOND INSTANCE of an existing class, not a
new mechanism.** II's U1 does not build a watcher; it builds an
`apply_scene(path)` and hangs it off one.

**3. `organ_manifest` IS NATIVELY CALLABLE, DIRECTLY.** It is an
`inline` function in a header the cartridge already includes, `extern
"C"` for name reach and `EMSCRIPTEN_KEEPALIVE`'d against linker GC —
and the macro **self-defines away** off the web, which its own banner
notes is why the ABI cost the sunset nothing. It returns a `const char*`
into a `static std::string`, built lazily and carrying the CURRENT value
of every entry so a surface opens showing the PROGRAM rather than its
own defaults. **No cwrap, no marshalling, no bridge**: the REPL calls it
like any other function.

**4. THE WEB SHELL: absent** — answered above, and it discharges §1.4.

**WHAT II'S UNITS ACTUALLY REDUCE TO, after this recon.** U1's road is
an import walk plus a `FileWatcher` instance. U2's hand is a parser over
`organ_manifest`'s JSON with a write to `organ_set` — both of which
exist and are law. U3 has no subject. **The campaign is smaller than its
handoff, and it is smaller because the ABI was built to survive exactly
this.**


> **SCHEDULED.** THE_PANEL arrived as two handoffs and runs after
> ONE_SURFACE-II closes green, probe included.
>
> **I — THE DIALS** finishes the parametric surface: the seed door (**U1
> LANDED**, PROBE-PENDING — the P8 seam's first caller; the transcript
> witness awaits Jean's run, not a caller), the deliberate DesignConfig
> relayout (**U2 LANDED**, PROBE-PENDING — 704 -> 688, four pads, both
> rooms; the cull-point net re-derived and bite-proven; DesignConfig
> enrolled in the mirror's byte-count witness at last), enrollment
> completion + the preset migration (U3, where the two parked
> preset-wire breakers below finally move), the orphan verbs disposed
> (U4), ORGAN.md rewritten native (U5), instruments + battery + probe
> (U6).
>
> **II — THE SHELL — LANDED.** `--scene=<file>` plus a watched scene file
> as THE ROAD (`console/organ_scene.hpp`), a stdin REPL as THE HAND
> (`console/organ_repl.hpp`), the web shell's fate DISCHARGED (it was not
> standing — `web/` left whole at the sunset), the shell gate RETARGETED
> to the REPL's smoke test, ORGAN.md's shell sections written native.
> **Its acceptance test holds: a new dial is one line in
> `organ_params.inc` and zero lines in the shell.** Nothing in either
> file names a dial, a block, a type, a range or a section.
> **What is Jean's and only Jean's**: the apply path. `block_base`
> returns null until `bind_home`, and binding a home needs a `GPUState`
> that pulls Dawn at link time — so the gate proves the shell REFUSES
> correctly and the probe proves it APPLIES.
>
> **THE PARKED SKINS, and neither is started (II §1.5).**
> * **The graphical overlay** — a skin over the SAME manifest and the SAME
>   road, adding no author. The ABI already emits every row with its
>   current value and every door with its label; an overlay is a renderer
>   over `organ_manifest`, not a second surface.
> * **The Ableton CC map** — CC# → `block.field` through the same
>   `organ_set`. It is the panel meeting the music, so it belongs beside
>   THE ABLETON SEAM and waits for the coupling campaign; the atlas
>   (`docs/COUPLING_ATLAS.md`) is what it will be designed on, and the
>   cadence tag on every candidate dial is there because a CC that turns
>   a GEN row and a CC that turns a LIVE one are different instruments.
> Both are **Jean-gated**: how the knobs look and what plays them are his
> next sentences, and the campaign that made every knob addressable is
> deliberately not the campaign that answers them.
> What remains open after that campaign is exactly the musical horizon,
> which is the reason the fork exists.



### LANDED AT THE_PANEL I U1 — THE SEED DOOR IS BUILT (PROBE-PENDING)

Everything below described a socket. It is a plug now, and the paragraphs
are kept because what they PROMISED is exactly what arrived, which is the
evidence a forward cue is worth writing:

* **Block 15, `WORLD_LIVE.next_seed`** (`contracts/world_surface.hpp`) —
  one row, GEN cadence, C3 destructive, no boundary wiring (L44). The one
  enrolled bank with no design TABLE: a world's seed was never authored,
  so the composition root seats it from the seed DRAW_0 actually drew.
* **`ORGAN_DOOR_REBIRTH`** — pressed at the frame boundary, FIRST of the
  doors, calling `rebirth_world(WORLD_LIVE.next_seed, queue)`. The only
  door with no key behind it, and the only destructive one.
* **The P8 mark is struck** at both its sites in `cartridge.hpp`, kept as
  a tombstone. The ten-verb latent chain has a live caller; the note is an
  ownership map now, not a shield.
* **PROBE-PENDING.** No device has run this. THE WRAP ORDER §0 rules that
  the flag rides the report and Jean's verdict at the tip converts it; a
  RED verdict fires the standing quarantine on this unit.
* **The rebirth transcript witness is AUTHORABLE and NOT AUTHORED** — see
  its own bullet below, which is re-dated rather than struck.

A seed dial the player turns: one gen-cadence, C3-destructive control that
re-draws the standing world under a new seed. It is the named future caller
of `rebirth_world`, which stood uncalled and **marked `SEAM[spine:P8]` —
explicit latent infrastructure** from ONE_WORLD-I to THE_PANEL I U1.
Boot-as-caller was **refused, not deferred**: a birth from nothing and a rebirth are different
operations sharing one door (`become_world`, L10), and one wrapper over both
loses to the PRIME INVARIANT's byte-for-byte boot order.

The socket is already open, the same way the Ableton seam's is: the verb is
whole and spine-resident, it walks the machine's own fixed sequence (O-3),
and ten teardown/reseed verbs plus one transitive `GPUState` upload are
latent with it — every one named at the verb, so no sweep reads them as
orphans. Seven of the ten are **gate-held**: the score census asserts each
roster bit's gated teardown call site in `cartridge.hpp`, and those sites are
the seam's. The dial has to author a seed and call one verb; nothing else in
the program has to move first.

**Parked here, for the campaign that lands the dial:**
* **The rebirth transcript — RE-DATED, NOT STRUCK (THE_PANEL I U1).** The
  verb keeps its `[World] Rebirth complete` line, and it prints for a living
  reason now; it still never prints at boot, which is what makes it a
  REBIRTH witness rather than a birth one. The duty parked here because
  nothing could turn the dial. **That blocker is gone and a second one is
  named in its place**: the transcript is a RUN, and CC has no built Dawn
  and no display (THE DEVICE GATE). So it moves from "cannot be authored"
  to "awaits the one hand that can author it" — one mid-run rebirth quoted
  beside one boot, from Jean's walk, at the same sitting as the probe
  verdict that converts U1's PROBE-PENDING flag.
* **The voice gate** (print-literal probate against each campaign's kill
  vocabulary). Parked as a PANEL-era candidate by the rulings after
  ONE_WORLD-I: no gate sees a print literal today, and the standing cover is
  the narration rule — narration dies with its subject, in the subject's own
  commit. ONE_WORLD-II's close payload may restate this line; it is recorded
  here now because the rebirth transcript parks beside it.
* **Two ONE_SURFACE parkings, ruled at its close** — the
  `FINITE_RADIUS_MIN/MAX` enrollment and the `veil_ring`/`veil_icing`
  rename. Both are stated in full under ONE_SURFACE-I's "Flagged, not
  taken — AND NOW RULED"; they are named here and not restated (L46),
  because this is where their caller arrives. The common shape is worth
  seeing: one is a dial nothing can exercise until the panel exists, the
  other a row-id rename that breaks stored preset keys and therefore wants
  the panel's enrollment-and-migration moment. Both are cheap once there is
  a panel and expensive in every campaign before it.

## THE CONTINUITY LEDGER (entered at the wrap, from THE BATON)

**Why this section exists.** A context ends; the tree does not. THE BATON's
Part II asked that OPEN.md carry the week's whole truth so the next session
can read the tree and know everything, and the baton itself dies at the
close it describes (L31). This is that entry.

**It is an INDEX, not a copy.** L46 — a rule restated in a second language
is a rule with two homes, and two homes drift. Everything below that already
lives somewhere is POINTED AT, never repeated; only what had no home is
written out here.

### The week, as campaigns

| campaign | subject | state |
|---|---|---|
| PRUNE_1 | gallery + snapshot organs out | LANDED |
| PRUNE_2 | blade / cactus / palm / column / antenna excised | LANDED |
| ONE_WORLD I — THE DOORS | transitions, portals, arch; the rebirth verb (P8) | LANDED |
| ONE_WORLD II — THE WEATHER | moods, themes, indoor → ATMOS / ORB / AGENTS / CUBE banks; world pinned FINITE | LANDED |
| ONE_SURFACE I — THE STILLNESS | streaming conductor out; the world built once; the probe commissioned | LANDED |
| ONE_SURFACE II — THE AUTOMATON | the ground IS the GoL, WRAP torus; GOL family out; the S-8 witness | LANDED |
| LIGATURE 0/1 | the recon, then canvas_1 fills the socket | LANDED |
| THE COUPLING ATLAS + SEAM SCOUT | the map, and three transports with costs | LANDED |
| THE_PANEL I — THE KNOBS | enrollment, the relayout, the seed door | LANDED |
| THE_PANEL II — THE HAND | `--scene=`, the watcher, the stdin REPL, the shell gate | LANDED |
| SUNRISE_0 · KEEL_0 · HELM_0 | the fork, the build constitution, the preset surface | LANDED |
| CHOIR_0 | the cube lattice out; `ch6` cast, 36 keys, one enveloped light per key | LANDED — **Jean's probe + visual desk open** (see CHOIR_0 above) |
| EMBER_0 | the D3D12 lanes | **OPEN — on Jean's button** (see EMBER_0 above) |

Roster: PYRAMID, SPHERE, RIBBON, CUBE. The ground: one automaton, ~3.1%
live at measure. Every mood-authored fact is now a live bank. Ableton:
tested working, couplings operating — the vocabulary rethink is design work
on the atlas, not repair.

### The doctrine — one pointer, deliberately

`docs/PROCESS_LAWS.md`, **THE DOCTRINE STACK**. Fifteen rules of method,
each the generalization of one failure this week produced and caught: §0
forward motion, A nets bite-proven, B1/B2/B3 on cuts, C compile coherence
over choreography, D the dial joins its fact, the record ritual,
transcribe-and-pin, refuse-loudly accessors, KEEPALIVE, P8's latency
corollary, the probe as acceptance gate, map-as-acceptance, and the web
twin not being a witness. It lives there because that file is the laws of
METHOD and this one is the register of OPEN STATE — doctrine is not open
state, and filing it here would be the second home L46 forbids.

### Standing rulings still armed — where each one lives

| ruling | its home |
|---|---|
| MOSAIC_2 retires unless "keep the grain" arrives | *THE MOSAIC IS UNREACHABLE*, above |
| banks follow ownership | the bank banners in `state.hpp`; ONE_WORLD-II's commits |
| the SDK glob is dead; E3 tells the truth | **CLOSED at EMBER_0** — `CMakeLists.txt`, the E3 banner |
| FXC is documented-unsupported; shader shape is law | **EMBER_0** above, L49, and the `kCompilerPlan` banner |
| UNIT.1 (the `D3D12_Dxc` boot) halted until route (a) lands | **EMBER_0** above |
| orphan verbs default to DOORS | THE_PANEL I U4's commit and `docs/ORGAN.md` |
| no constructive GPU work ships unprobed | **L48**, and `CLAUDE.md`'s gate table |

### Jean's open gates

Not work items — decisions only he can make, gathered so none is lost
between contexts.

1. **The probe ritual** — GREEN at tip, RED at `2905ed68`, the pair that
   proves the probe bites (*THE DEVICE GATE*, above).
2. **The knob walk** — the hand exists; walking it is what tells the next
   campaign where to widen.
3. **Three campaign names** — provisional until his gate (F1-class).
4. **EMBER's overnight button** — `python tools\ember_route_a.py --go`,
   then the manifest-delta stamp (**EMBER_0**, above).
5. **The assets check** — `dir /b out\build\the-board-full-release\Release`,
   or simply the next VS build, now that the post-build manifest reports it
   for free (*THE CLAUSE FIRED*, above).
6. **The seam transport** — the scout landed three with costs and chose
   none, deliberately. Prior stated at the wrap: **Link**, because the clock
   already thinks in beats (*THE ABLETON SEAM*, above).
7. **"Keep the grain"** — if ever, and it was due before THE_PANEL I U5.

### The horizon, after the wrap

**THE SEAM** — the Ableton connection collected; musical witnesses join the
battery. **THE VOICES** — Jean composes the coupling score on the atlas, by
cadence: frame-rate dials are continuous voices, gen-rate dials are
bar-scale events (the seed on a phrase, the ground on the bar, orbs leaving
neutral); Claude orchestrates, CC wires. **THE STAGE** — scenes as setlist,
then performance, skins, other machines.

From the seam onward, every close gate is **played, not walked**.

### The last line

The refactor ends where the instrument begins. The tree is minimal,
readable, self-contained, gated and honest — and it remembers everything
this ledger says, which is the point: the next hands to hold this need only
the tree, the doctrine, and the music.
