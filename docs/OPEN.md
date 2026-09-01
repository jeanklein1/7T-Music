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
| S3 | portals (cont.) | **CLOSED (ONE_WORLD-I U4).** The rooms emptied: `GPUPortalArray` / `GPUPortalEntry` and the WGSL `PortalArray` / `PortalEntry` are gone, and with them the PASSER round — the array's last reader, a behaviour whose whole purpose was walking between doors. `GPUAgentRoomConstants` went 1584 → 512 B. `WorldDrawSurface` did NOT empty: `scheme_weights` survives, so the room S2 predicted would empty under S2+S3 stands on the indoor light scheme alone. |
| S4 | agent pipelines | FLAGGED — `agent_state[possessed_slot]` **is** the point; camera, veil, terrain aura, LOD streaming, the ribbon's sky rule and the shadow box all read it |

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
drove it. `rebirth_world` has no caller by ruling; the panel's seed dial
is the caller it waits for. FLAGGED for Jean's visual gate. The verb now
stands **marked** — `SEAM[spine:P8]`, with its latent chain named at the
verb; the forward cue is THE PANEL below.

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

**Parked with THE PANEL (below), not done here:** the orphan console verbs
the sweep surfaced — `cycle_cube_behavior_override`, `reveal_zoetrope`,
`toggle_cube_kite_mode`, `unrecord_entity` and roughly forty accessor leads —
each reachable from no key and no door. None is this campaign's subject; they
await the panel's own recon, which is the sitting that can say what a control
surface needs.

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
- `OrbMoodConfig` still wears the moods' name. Its rows stopped being seven at
  U1b; the type is the world's one orb row now. A type rename is Jean's.
- The frustum-cull flag (`useIndirectTerrainPipeline_`) is WRITTEN once at
  every world's birth and READ BY NOBODY — OPT_1 found it latent, and the
  column that fed it died at U2, so the write is now the literal `true`. The
  cut wants a build.
- Five WGSL orphans that predate this campaign and are not its subjects:
  `row_occupier`, `seg_closest`, `signal_active`, `translator` (defined,
  referenced only from prose) and `CARD_NODES_4` (a documented exclusion).
- `docs/FXC_LAWS_RECORD.md` cites `GPUSpotLightArray`'s `static_assert` in
  `state.hpp` as a live example; the assert left at U4. It is a stamped
  record, so the sweep reports it rather than editing it.

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
`--seed/--cap/--msaa` loop behind it. (`--mood` was the fourth until
ONE_WORLD-II U2 — a world is chosen by its seed alone now.)

## THE PANEL (held — the campaign that gives the rebirth a caller)

A seed dial the player turns: one gen-cadence, C3-destructive control that
re-draws the standing world under a new seed. It is the named future caller
of `rebirth_world`, which stands uncalled and **marked `SEAM[spine:P8]` —
explicit latent infrastructure** since ONE_WORLD-I. Boot-as-caller was
**refused, not deferred**: a birth from nothing and a rebirth are different
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
* **The rebirth transcript.** The verb keeps its `[World] Rebirth complete`
  line — the honest voice for a caller that does not exist yet, and it never
  prints at boot. A side-by-side transcript witness can only be written once
  something turns the dial, so the duty parks here rather than going stale
  in a campaign that cannot exercise it.
* **The voice gate** (print-literal probate against each campaign's kill
  vocabulary). Parked as a PANEL-era candidate by the rulings after
  ONE_WORLD-I: no gate sees a print literal today, and the standing cover is
  the narration rule — narration dies with its subject, in the subject's own
  commit. ONE_WORLD-II's close payload may restate this line; it is recorded
  here now because the rebirth transcript parks beside it.
