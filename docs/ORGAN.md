# ORGAN — the master control panel

The Organ is a VIEW of the program, not a second copy of it. Enrollment turns a
struct member the program already reads into a dial; the shell draws whatever
the registry emits; a door presses machinery the program already has. Nothing
here is an author the program did not have, and nothing here is a home for a
fact that has one elsewhere.

**THE CHANNEL IS argv (THE_PANEL I U5).** `--seed=N` pins the world, `--msaa=`
picks the sample count, `--scene=<file>` applies a scene and watches it
(THE_PANEL II U1), `--probe=N` runs the device gate and exits on the
device's own verdict, `--probe-backend=` picks its adapter ladder. Read once at
boot before the device request, never re-read, never mutated mid-run; anything
accepted prints one `[Params]` line, because a switch that fired should be
visible. `src/core/boot_params.hpp` is the one home.

*This paragraph read `?organ=1` opens the panel (the backtick folds it; on a
phone, the pill). `?preset=<name>` boots a scene with no panel. `?seed=N` pins
the world.* Not one of those roads exists: the URL channel, the pixel cap and
the pace lever went with the web twin at tag `web-sunset`, and their only
consumers were its presentation layer. It was the doc tree's largest surviving
falsehood and it was in this document's second paragraph.

## The compiled registry

`src/console/organ_params.inc` is the enrollment list. `src/console/organ_registry.hpp`
compiles it into `kOrganParams[]` and exports the panel's C ABI. Every offset is
`offsetof` against the declaration the program reads, so a rename fails the build
at the enrollment line: there is no second copy to drift from.

Five forms, each with an `_NS` twin whose first argument is the struct's
namespace (the plain form supplies `the_board`):

| form | enrolls |
| --- | --- |
| `ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)` | a dial on a home that is the only truth |
| `ORGAN_PARAM_GEN(… the same …)` | the same, landing at the author's next natural event — chip `on respawn` |
| `ORGAN_PARAM_DEF(… the same …, DEFKIND, DEFSTRUCT, DEFFIELD)` | a dial whose home an author re-produces from a definition; names that definition |
| `ORGAN_PARAM_DEFONLY(TYPE, MIN, MAX, STEP, GROUP, LABEL, DEFKIND, DEFSTRUCT, DEFFIELD)` | a definition with no addressable instance |
| `ORGAN_PARAM_RO(BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)` | a witness: metered, never written, no range |

`id` is `block.field` — stable across relabelling; export and import key on it.
`GROUP` is a path, `"Section · Group"`: the first token is the section and the
shell splits on the first separator only. The file is kept ordered by section
path because a section must be contiguous. A dial sits under its SUBJECT, never
under its implementation home. Types are `F32 U32 BOOL VEC3 VEC4`; a `VEC3` over
0..1 renders as a colour.

The manifest is the whitelist: `organ_set` refuses any (block, offset, type)
triple that is not an entry, counts the refusal, and names it.

## Blocks and the sovereignty boundary

| block | home | reaches the GPU by |
| --- | --- | --- |
| 0 `CONFIG` | `GPUDesignConfig config_` | the spine's `upload_config`, off `configDirty_` |
| 1 `LIGHTING` | `GPULighting lightingStage_` | `GPUState::organ_flush` |
| 2 `AGENT_ROOM` | `GPUAgentRoomConstants agentRoomStage_` | `GPUState::organ_flush` |
| 3 `DRIVERS` | `DRIVER_LIVE` — the drivers' room: rests and gains at the seams | the seams that read it each tick |
| 4 `PAWN` | `PAWN_AURA_LIVE` | its readers |
| 5 `ORBS` | `ORB_CONSOLE_LIVE` | the console mask |
| 6 `PANEL` | `PANEL_LIVE` | its readers |
| 7 `RIBBON` | `RIBBON_LIVE` | its readers |
| 8 | — a HOLE. `INDOOR_LIVE` retired at ONE_WORLD-II U4; block ids are the console's wire contract and every stored preset key is one, so the id is never re-packed | — |
| 9 `CANVAS` | `canvas::CANVAS_LIVE` (one tier below the cartridge) | its readers |
| 10 | — a HOLE. `WORLD_DRAW_LIVE` retired at ONE_WORLD-II U4, same law as 8 | — |
| 11 `RIBBON_SPAWN` | `RIBBON_SPAWN_LIVE` | the next ribbon spawn — destructive |
| 12 `ATMOS` | `ATMOS_LIVE` — the world's one sky, as a distribution | the boundary re-draws from it |
| 13 `ORB_BANK` | `ORB_LIVE` — the sky's one orb row | `configure_orbs`, per-field mask |
| 14 `AGENTS` | `AGENTS_LIVE` — how many walk this world, and what they are | the next spawn — destructive |
| 15 `WORLD` | `WORLD_LIVE` — what a world is CHOSEN by: the seed the next one is drawn from, and the range its radius is drawn within | the next rebirth — destructive; the REBIRTH door is that event |
| 16 `AUTOMATON` | `AUTO_LIVE` — the ground's own vocabulary: rule, draw, per-cell grain | the next world's birth — destructive; `draw_automaton` is its one reader |

Blocks 0–2 are reached through three accessors on `GPUState` and no others. GPU
truth — positions, camera, simulation state — has no accessor and therefore no
block id; a panel that cannot name a thing cannot write it. Blocks 3–16 are
contracts-tier CPU banks whose base is the instance itself. There is
deliberately no Camera section: pose is GPU truth, and the camera's dials under
`Interaction · Camera` are input grammar.

**The 255/254 sentinels are gone (ONE_WORLD-II U6a).** They stood in the block
slot for definition-only rows so two families' offsets could not collide at
the same number. Every row in the tree addresses an instance now, so the
sentinels, the `ORGAN_PARAM_DEFONLY` macro pair and the `is_defonly` predicate
all left; a future family with no instance re-founds the convention
descending from 255.

## Definition and preview

The default write is a DEFINITION: it changes what the WORLD means and lets
the program's own applier produce the instance at the frame boundary. PREVIEW
writes the instance; the next author may take it back. A row with no definition
writes the instance under either mode.

An instance row whose home an applier re-produces is a window, not a dial:
it is enrolled as a witness (`_RO`), and the authored dial sits on the
applier's INPUT. The sun is the case of record — `GPULighting::sun` is drawn
by `stage_sky` at every world's birth and every atmosphere re-speak, so its
four rows are witnesses and its brightness is authored at `Atmosphere · Sky`.

| kind | struct · bank | scope | raises | applier at the boundary |
| --- | --- | --- | --- | --- |
| `TIER` | `AgentTierBank` · `TIER_LIVE` | the world's | `g_tier_def_dirty` | `upload_agent_registries_to_gpu` |
| `BEHAVIOR` | `AgentBehaviorBank` · `BEHAVIOR_LIVE` | the world's | `g_tier_def_dirty` — same author as TIER | the same |

**Two kinds, not four (ONE_WORLD-II).** `MOOD` (`MoodProfile` · `MOOD_LIVE`)
and `ORB_MOOD` (`OrbMoodConfig` · `ORB_MOOD_LIVE` — the type is `OrbConfig`
since ONE_SURFACE-I) were MOOD-SELECTED: one row
per mood, the write's target choosing which. U1a gave the atmosphere an
instance and U1b gave the orbs one, so both became ordinary instance blocks
(12 and 13); the target parameter left with the last selecting kind at U6b,
and a definition target above 0 is now REFUSED OUT LOUD — a stored preset
keyed `"<mood>/<param>"` names a world that no longer exists, and aliasing it
onto the one bank would be a silent lie.

A field is a definition only if its applier is the field's one runtime reader.
A kind shares a flag when it shares an author. The definition path CONVERTS
integer lanes and never reinterprets them. The manifest emits `cad`, `inst` and
`scope` derived from the entry; the shell holds no block or kind number.

## Cadence — when does my edit land

| cadence | chip | derived from |
| --- | --- | --- |
| `LIVE` | silent | the default |
| `GEN` | `on respawn` | stored — the one fact the row must volunteer |
| `BOUNDARY` | `boundary` | a definition kind or `block_has_boundary` |
| `DRIVEN` | `driven` | `ro` |

`derived_cadence()` is the one place the rule lives. Temperament: an idempotent
author re-speaks at the boundary when its bank changes; a destructive author
(spawn, world generation) does not — its rows are `GEN` and its group banner
says so. Where one fact has both temperaments, the stricter governs.

## The write path and the masks

`organ_set` clamps (or converts), writes, marks the block dirty, and returns.
Dirty blocks flush once per frame in `GPUState::organ_flush`; a slider drag is
many events and one upload. `config_` raises its own `configDirty_` and the
spine uploads it.

The flag says THAT a bank changed; a mask says WHAT; the boundary decides how
much re-speak the edit requires. Bit = offset / 4, pinned by `static_assert`
beside each mask.
- `g_orb_console_dirty` (block 5): dome radius, noise floor and speed
  multiplier take targeted partial uploads; base size raises the orb re-speak.
- `g_orb_def_touched` (over `OrbConfig`) against `ORB_RESEED_BITS` —
  `enabled`, `count`, `palette_id`, `drag`: only those four re-seed the sky; the
  other fifteen ride the uniform upload. A raise with no bits means everything.

The frame boundary itself — doors, the re-speaks, the mask routing, the rule
window, the flush — is `src/cartridges/the_board/organ_boundary.inc`, member
functions of the cartridge, taken once per frame from `the_board.cpp`.

## Doors

A door raises a flag the boundary already consumes and adds no author.
`organ_doors()` emits the roster; the shell renders one button per row; presses
coalesce in a bitmask.
- `RESPEAK` — raise every definition flag at once.
- `ORB_RULE`, `ORB_GESTURE` — the commands keys KP_8 / KP_7 press; the sky's
  two player-owned facts, which is why they are doors and not dials.
- `CUBE_BEHAVIOR`, `ZOETROPE`, `CUBE_KITE` (THE_PANEL I U4) — the three
  orphan console verbs, graduated. Each pressed machinery the program still
  had and each was reachable from no key and no door since ONE_WORLD-II
  parked them; a door is the caller each was missing. `ROSTER.cube`-gated
  at the boundary, like every other cube call site in the spine.
- `REBIRTH` (THE_PANEL I U1) — presses `rebirth_world` with block 15's
  `next_seed`. The only door with no key behind it: the six keys that once
  ignited a world change left with the transition machine (ONE_WORLD-I), and
  this is their replacement rather than a duplicate of one. It is also the
  only DESTRUCTIVE door, which is why the seed is a `GEN` dial and the press
  is separate — a dial that rebuilt the world on every event of a drag would
  destroy it once per event on the way to the number the hand wanted. Taken
  FIRST at the boundary, before the re-speaks: every other line there makes
  the standing world match the banks, and after a rebirth the standing world
  is a different one.

The mood door (`organ_go_mood`) stood here until ONE_WORLD-I: it pressed
`request_mood_transition`, and door and request left together with the
transition machine. `organ_mood`, `organ_regime` and `organ_mood_names` were
what remained of it, and they left at ONE_WORLD-II U2/U6b with the fact they
read — an export that would return a fact that no longer exists dies with the
fact rather than returning a lie. `organ_host` / `organ_go_host` (RIBBON_1)
are the surviving door-with-a-parameter, on the shape the mood door founded.

The rule window (`organ_orb_rule`) packs rule, gesture, lit and count out of
`OrbsState`, refreshed at the boundary and nowhere else, so it cannot go stale
whoever turned the rule.

## The ABI

`organ_manifest`, `organ_doors` (JSON); `organ_set`, `organ_door`,
`organ_go_host` (writes); `organ_get`, `organ_def_get`, `organ_orb_rule`,
`organ_host` (reads, by manifest index where one applies);
`organ_param_count`, `organ_rejected_count`, `organ_last_reject`,
`organ_flush_count`, `organ_build_stamp` (the status line). The manifest is
emitted in table order, so its index is the row's index. The ABI is inert until
`bind_home` and `bind_point` run at the end of cartridge init; every entry
point returns harmlessly before that, and a consumer polls until
`organ_param_count` is nonzero.

The ABI's consumer is the native control surface to come (docs/OPEN.md,
THE ABLETON SEAM); the browser panel that drove it is attic'd at tag
`web-sunset`.

## The shell

**IT EXISTS (THE_PANEL II).** The road and the hand are native, both on the
tree's one-road law — *one write path, many doors onto it* — and both
name-blind by construction. The acceptance test is the banner and it holds:

> **A new dial is one line in `organ_params.inc` and zero lines in the shell.**

Nothing in `console/organ_scene.hpp` or `console/organ_repl.hpp` names a
dial, a block, a type, a range or a section. `list` derives its sections by
splitting a row's own group string; `set` reads the lane count off the row's
type; `get` prints the cadence `derived_cadence()` computes. Add a row to the
enrollment list and every verb carries it on the next build, unedited.

### THE ROAD — `--scene=<file>`, watched

`console/organ_scene.hpp`. Parses a scene's `block.field` keys, resolves each
against the manifest, applies through `organ_set`, presses `RESPEAK` once per
file when a definition landed. `--scene=` at boot, and a **second
`FileWatcher` instance** — the same class the shader reload uses, one file,
one stat per check, no dependency — re-applies the whole file on every save.
Live editing is any text editor.

**Three laws it keeps, each paid for by a finding:**
- **A scene never half-applies.** Two passes: resolve every key writing
  nothing, and only a file that resolves WHOLE reaches the write. A file that
  half-lands leaves behind a world nobody authored.
- **Whole-id matching, never a prefix.** Block *ids* are never re-packed, but
  block *names* are reusable and one has been reused — `WORLD.*` meant the
  retired world-draw block before it meant block 15.
- **Loud, by name, by line.** A parse failure names its line; an unknown key
  names itself and takes the file with it; a refusal is `organ_set`'s own.

The road opens **after the renderer**, not at `parse_boot_params`: the ABI is
inert until `bind_home`, so a scene applied earlier would write nothing and
say it had.

### THE HAND — a stdin REPL

`console/organ_repl.hpp`. Nine verbs: `list [filter]`, `get <id>`,
`set <id> <v…>`, `doors`, `door <n>`, `export <file>`, `import <file>`,
`probe <N>`, `help`. Polled once per frame with `poll()` on fd 0 —
non-blocking, because the frame loop is a busy `while (running())` with a
blocking `present()` and `getline` on the render thread would stall the world
between keystrokes. It drains the pipe each turn, so a pasted session does not
cost one frame per line, and it sits before `begin_frame` so a `set` typed
this frame is reconciled at THIS frame's boundary.

`probe <N>` arms the device gate from the hand, writing the same
`boot_params` the flag writes — a probe asked for here and one asked for at
argv are the same run.

**POSIX today.** The tree's Windows lane is the Visual Studio build, where the
equivalent is `WaitForSingleObject` on the console handle: a second `#ifdef`
arm of one function and no other line. It is absent and says so rather than
shipping an arm nobody can run.

### THE SMOKE TEST

`tools/gates/shell_gate/run.py` — the name is a **retarget, not a revival**.
A `shell_gate` proved the C++ ↔ browser seam until WEB_SUNSET and left with
the shell it proved. II §1.4 ruled that what it alone proved "either
retargets to the REPL's smoke test or retires claimed"; there is a shell
again, so it retargets. It **compiles, links and RUNS** the shell: five
scenes down the road, a scripted session through the hand, and the export →
import round trip. Every other gate in this tree reads text; this one
executes.

**What it cannot prove, and this is structural.** The APPLY path is not
deviceless: `block_base` returns null until `bind_home`, and binding a home
needs a `GPUState`, whose wgpu handle members pull Dawn at LINK time. So the
gate proves the shell REFUSES correctly and the probe proves it APPLIES.
Neither claim is made by the other.

### PARKED SKINS — Jean-gated, and written where he will find them

Neither is started, and both are in `docs/OPEN.md`'s PANEL section:
- **the graphical overlay** — a skin over this same manifest and this same
  road, adding no author;
- **the Ableton CC map** — CC# → `block.field` through the same `organ_set`.
  It is the panel meeting the music, so it belongs beside the Ableton seam
  and waits for the coupling campaign.

## Presets

The shelf moved to `/presets` at WEB_SUNSET — authored scenes, **still
reader-less**, held for the native ingestion (docs/OPEN.md, NATIVE PRESET
INGESTION). The `?preset=` boot road went with the panel and `--scene=` is
THE_PANEL II's to build.

**THE MIGRATION RECORD LANDED AHEAD OF THE READER (THE_PANEL I U3c).**
`presets/index.json` is **schema 2** and carries the refusal contract plus a
`retired_ids` ledger — data, not code, so the importer is written against a
census rather than a guess. **49 of `baseline.json`'s 232 keys name families
that no longer exist**, and that file stays stamped schema 1 deliberately:
re-stamping it would make the version a lie and hide 49 refusals an importer
should print.

Two things in that ledger are law for whoever writes the reader:
- **A stale key is refused OUT LOUD, BY NAME, and the file with it** — never
  skipped, never aliased. A scene that half-applies leaves behind a world
  nobody authored. The precedent is `organ_set`'s own refusal of a definition
  target above 0.
- **Match ids WHOLE, never by prefix.** Block *ids* are never re-packed (8 and
  10 are permanent holes), but block *NAMES* are reusable and one has been
  reused: `WORLD.*` in a schema-1 file means the dead world-draw block, while
  `WORLD.next_seed` is block 15 and alive.

**And an export must walk the MANIFEST, not the struct.** Nine of those stale
keys are `RIBBON_SPAWN` ARRAY members over a block that is very much alive —
the manifest has no row shape for an array, so no `organ_set` call could ever
have accepted them. The web panel's export walked the struct and wrote a file
the program cannot read back.

## The tuning loop

`--seed=N` pins the world; the panel's dials address the banks that world was
drawn from, and the boundary re-draws WITH the dial rather than re-rolling
(same seed, shifted centre, same offset). The `[Atmos]` witness prints
`seed= int= amb= sun el= az= fog=` once per WORLD, so a drag is silent and a
new world announces itself.

It was `?organ=1&mood=N&seed=N` into one of seven moods, with a lens that
followed the regime the seed drew, and a preset file carrying every mood's
definitions keyed `"<mood>/<param>"`. The regime roll left at U1, the moods at
U2, and the witness lost a term at each: it printed `(mood, seed, regime)` and
prints one seed now.

*This paragraph ended "A preset shelved before those cuts is REFUSED by key,
loudly and by name."* **No program does that, because no program reads a
preset** — the refusal is a CONTRACT written down (see Presets above) and its
enforcement is THE_PANEL II's. `organ_set` does refuse a definition target
above 0 by name, which is the precedent the contract descends from, and that
one is live.

## The world

ONE WORLD, ONE SKY. A world is a SEED and nothing else: `--seed=` on argv
or a drawn one, and every fact the program used to look up per mood is a
LIVE BANK the panel owns — `ATMOS_LIVE` (the sky as a distribution),
`ORB_LIVE`, `AGENTS_LIVE`, and since ONE_SURFACE-II `AUTO_LIVE`, the ground's
own vocabulary. The world is pinned FINITE and its radius draws under the same
seed from **`WORLD_LIVE.radius_min/max`** (THE_PANEL I U3), a range the panel
turns, bounded by the compile-time `FINITE_RADIUS_MAX` that sizes the
automaton's life buffer and the frustum-cull segments.

**`CUBE_LIVE` was named in this list and is NOT a bank the panel owns.** It has
no enrolled row: `CubeBank` lives in a body, and the organ may not include one
(L38), so enrolling it needs a contracts seat founded for it first — the move
`agent_surface.hpp` was. `organ_gap` reports it as a home with zero named
members. Flagged, not forced, and now said here instead of implied.

**AND A WORLD CAN BE RE-DRAWN WITHOUT RESTARTING (THE_PANEL I U1).** Block 15
carries `next_seed`; `ORGAN_DOOR_REBIRTH` presses `rebirth_world` with it.
Turn the seed and press, and the standing world is torn down and another
drawn; press without turning and the same world is rebuilt from the same
number.

`## Moods` stood here until ONE_WORLD-II U7. It described seven variants,
their positional tables (`MOOD_TABLE`, `ORB_MOOD_TABLE`,
`AGENT_POPULATIONS`, `MOOD_SPAWN_MULT`, `CUBE_POPULATIONS`), a portal
colour, a weight in `WORLD_DRAW_TABLE`, a name in `MOOD_NAMES`, and the
destination law that walked `WORLD_DRAW_LIVE.mood_weights` by id. Most of
that was ALREADY STALE when this campaign found it — `pick_portal_mood`,
`pick_open_mood` and the portal colours left with the doors at
ONE_WORLD-I. All of it is gone now.


## Instruments

| instrument | claim |
| --- | --- |
| `tools/organ_gap.py` | the map of declared members the panel does not name; `--gate` fails on a design table with a surviving runtime reader |
| `tools/organ_readers.py` | a declared reader names every enrolled field |
| `tools/organ_ledger.py` | `audit/ORGAN.md` — every row with range, step and cadence; the music campaign's target map |
| `tools/gates/console_gate/run.py` | the translation units type-check, two tiers, zero diagnostics |
| `tools/gates/shell_gate/run.py` | **it RUNS the shell** — five scenes down the road, a scripted session through the hand, the export→import round trip. The one gate here that executes rather than reads |
| `tools/mirror_offsets.py` | not an organ tool, named here because it is the other half of L3: every mirrored member's offset derived from `world.wgsl` and asserted by the C++ compiler |

`tools/gates/shell_gate/run.py` stood in this table, left with the browser
panel at WEB_SUNSET because there was no shell at the other end of it, and
**is back at THE_PANEL II U2 with a native one** — same path, same name, same
job. The three organ tools share `tools/organ_parse.py`.

**EVERY INSTRUMENT IN THIS TABLE READS TEXT, AND THE DEVICE IS A GATE OF ITS
OWN (L48).** They parse the enrollment list, type-check the TUs and diff
idioms; not one of them runs a dial against a device. `the-board --probe=N`
boots, runs N frames and exits on the device's own verdict, and it is the row
that RUNS. A dial that is green here and refused there is a defect no reading
finds.

## What has no dial, and why

An enrollment states a belief and only the reader proves it; the converse also
has a register. Four facts survived every wave of the disposition survey
without earning a row, and each names its own owner rather than waiting on a
campaign. **Two of the four have since died with their subjects
(ONE_WORLD-II): `THEME_BASE_WEIGHT` with the theme engine at U3, and
`INDOOR_PALETTES[]` with the rooms at U4.** Their rows are struck rather than
deleted, because the REASON each was not a dial is the register's content and
outlives the fact.

| what | why not a dial | its owner |
| --- | --- | --- |
| ~~`THEME_BASE_WEIGHT`~~ | it was one scalar over a 5×N weight construction; the honest dial was the whole `MOOD_SPAWN_MULT` matrix, D5-large — **struck, ONE_WORLD-II U3** | a composite editor (D5) |
| ~~`INDOOR_PALETTES[]`~~ | mixed-shape rows, count read from `INDOOR_PALETTE_COUNT` (D5) — **struck, ONE_WORLD-II U4** | a composite editor (D5) |
| `tierset_id` | its "none" value is `0xFFFFFFFF`, and a 0…1 slider cannot express a sentinel without lying (D1(d)). `organ_gap` reports it, by name, as the one absent member of `OrbConfig` | a composite editor (D5) |
| `mute_couplings` | EXCISED at STRIKE_0 U1 — the row this table refused was the honest half; the field was boot-NONE, writer-less, row-less, and the eight couplings it gated are unconditional facts now. The checkbox grid this row once priced is moot: there is no bitmask to check. | none — a pad holds the offset (both rooms) |

`D5` is the composite-editor deferral: a fact whose honest control is a grid
rather than a slider, priced rather than promised. `D1(d)` is the range
deferral: a value whose domain a slider cannot state without lying.

None is externally blocked, which is why none of them is in `docs/OPEN.md`:
OPEN is the register of open STATE, and a deferral any sitting can lift by
reading a module is unfinished survey, not open state.

**The strip question, settled by the tree's own walk.** A sitting once
had to choose which couplings a mute strip would be for — `Coupling::`'s
program couplings, or a music vocabulary that did not yet exist. Both
halves have since answered. The program couplings became unconditional
facts (STRIKE_0 U1 — a mute nothing could turn came out, and what cannot
be turned off needs no strip). The music vocabulary now exists and
carries its own mutes: the drivers' room's gains, where 0 is hands off
by law — fog, checker, ribbon, the cube light, the ground's three — with
the aura's `intent` as the one literal switch. The strip is built and it
is the DRIVERS block; no other strip is owed.

## The absence roll — every gap, its reason

`organ_gap.py` is a map and this is its legend: every member of an
enrolled home that the panel does not name, and why the record says so.
A row here is a REASON on file, not a parking ticket — when a reason
dies, its row must die with it (the mute's row above is the cautionary
tale). Standing at the R-round's close: **40 absent, 40 reasoned, 0
unexplained.**

| home · field | reason |
|---|---|
| `AgentPopulationBank` · `behavior_weights`, `tier_weights` | weight VECTORS — one slider cannot say one; the composite editor (D5) is the future, priced at `tierset_id`'s row |
| `CubeBank` · `behavior_weights` | same class: a weight vector awaiting D5 |
| `AutomatonBank` · `mode_threshold` | LAW, not a dial — hardware mirror of `AUTO_MODE_THRESHOLD` (world.wgsl), transport skips it by design; blessed by Jean at STRIKE_0 R |
| `GPUDesignConfig` · `world_seed` | the seed the world IS; its dial is `WORLD.next_seed` + the REBIRTH door (the seed door), its witness the boot line |
| `GPUDesignConfig` · `sun_direction` | the transported value of `ATMOS.sun_direction` — the dial exists upstream; this is the wire |
| `GPUDesignConfig` · `world_bound_min`, `world_bound_max`, `placement_patch_count` | world geometry the seed and radius author at birth — facts of the world, not knobs on it |
| `GPUDesignConfig` · `pulse_count`, `pulse_data` | the ring's seat — DRIVEN by the strike writer since STRIKE_0; the hand's dial is `ground.ring_gain` |
| `GPUDesignConfig` · `possessed_slot`, `point_host`, `cull_point_x`, `cull_point_z` | the program's own hands — possession and camera state, authored per frame, never a dial |
| `GPUDesignConfig` · `draw_ring`, `cube_plasticity` | dark by ruling (STRIKE_0 U3f): rows retired, fields await a relayout campaign |
| `OrbConfig` · `motion_rule` | dead column — the applier ignores it: `configure_orbs` seeds Brownian hardcoded and the rule is player-owned after (`current_motion_rule`, the door's fact); excision or a reader is a future ruling |
| `OrbConfig` · `flock_gesture_default` | a first-run SEED — "the world seeds once, player wins after": read once in `apply_first_run_defaults_`, then the ORB_GESTURE door owns the fact; a dial would be a second hand after the first frame |
| `OrbConfig` · `base_hue`, `hue_variance` | dead columns — faithfully copied into the legacy single-hue arm the palette law made unreachable ("the enrollment stated a belief, the reader refused it" — the applier's own banner); excision or a reachable arm is a future ruling |
| `OrbConfig` · `hue_converge_target` | a live per-world read at the convergence walk — the seed's character, the automaton-spread precedent |
| `OrbConfig` · `tierset_id` | (standing row above — the sentinel a 0…1 slider cannot say; kept there, referenced here) |
| `PawnAuraProfile` · `effect_mask` | a bitmask, mirrored and unread (STATUS: INTENT) — the checkbox-grid class, unbuilt; a reader or an excision is a future ruling |
| `RibbonSurface` · sixteen movement fields | MOVEMENT, whole — the headline gap, parked by ruling for the movement campaign (ORGAN_REST, reaffirmed at the STRIKE_0 freeze-lift) |

## The tally

**THE HAND-WRITTEN TALLY IS GONE (THE_PANEL I U5), and that is the fix.**

It read *"310 entries — 295 dials and 15 witnesses"* under a line saying "Cut
from `audit/ORGAN.md`, which `tools/organ_ledger.py` regenerates" — and it had
drifted on FOUR of its numbers at once. The recon that found it counted three;
its own adversarial pass found the fourth (`live`: the doc said 164, the tool
said 162). A hand-written tally beside a generated one is the thing this unit
exists to end: the copy is the one that drifts (L46), and a tally is nothing
BUT a copy.

**The tallies live in `audit/ORGAN.md` and nowhere else.** Its header block
carries entries, by section, by cadence, by macro form, definition kinds,
witnesses, blocks used and namespaces, and it is regenerated by
`python3 tools/organ_ledger.py` from the enrollment list. It is committed and
searchable, which is why this document should point at it rather than quote it.

What belongs HERE is what a tally cannot say — the shape of the surface and
why it has that shape — and that is the rest of this document.

**Two numbers ARE worth stating in prose, because they are claims and not
counts.** Every enrolled row has a declared reader (`organ_readers.py`: NO
SUSPECTS), and every graduated design table has no surviving runtime reader
(`organ_gap.py --gate`: PASS). Those are the two things the tally was really
being used to imply.
