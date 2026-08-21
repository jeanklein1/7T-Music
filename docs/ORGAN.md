# ORGAN — the master control panel

The Organ is a VIEW of the program, not a second copy of it. Enrollment turns a
struct member the program already reads into a dial; the shell draws whatever
the registry emits; a door presses machinery the program already has. Nothing
here is an author the program did not have, and nothing here is a home for a
fact that has one elsewhere.

`?organ=1` opens the panel (the backtick folds it; on a phone, the pill).
`?preset=<name>` boots a scene with no panel. `?mood=N` and `?seed=N` pin the
world. With none of these the shell returns on its second statement and the
audience path is byte-identical.

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
| 8 `INDOOR` | `INDOOR_LIVE` | the next spawn — destructive |
| 9 `CANVAS` | `canvas::CANVAS_LIVE` (one tier below the cartridge) | its readers |
| 10 `WORLD` | `WORLD_DRAW_LIVE` | the next world draw — destructive |
| 11 `RIBBON_SPAWN` | `RIBBON_SPAWN_LIVE` | the next ribbon spawn — destructive |
| 255 / 254 | sentinels for definition-only rows: `MoodProfile` / `OrbMoodConfig` | — |

Blocks 0–2 are reached through three accessors on `GPUState` and no others. GPU
truth — positions, camera, simulation state — has no accessor and therefore no
block id; a panel that cannot name a thing cannot write it. Blocks 3–11 are
contracts-tier CPU banks whose base is the instance itself. There is
deliberately no Camera section: pose is GPU truth, and the camera's dials under
`Interaction · Camera` are input grammar. A third definition-only family takes 253.

## Definition and preview

The default write is a DEFINITION: it changes what a mood (or the world) MEANS
and lets the program's own applier produce the instance at the frame boundary.
PREVIEW writes the instance; the next author may take it back. A row with no
definition writes the instance under either mode; a definition-only row refuses
preview.

| kind | struct · bank | scope | raises | applier at the boundary |
| --- | --- | --- | --- | --- |
| `MOOD` | `MoodProfile` · `MOOD_LIVE` | mood-selected | `g_def_dirty` + mood | `apply_mood_regime`, `apply_mood_lighting` — live mood only |
| `TIER` | `AgentTierBank` · `TIER_LIVE` | the world's | `g_tier_def_dirty` | `upload_agent_registries_to_gpu` |
| `BEHAVIOR` | `AgentBehaviorBank` · `BEHAVIOR_LIVE` | the world's | `g_tier_def_dirty` — same author as TIER | the same |
| `ORB_MOOD` | `OrbMoodConfig` · `ORB_MOOD_LIVE` | mood-selected | `g_orb_def_dirty` + mood + touched mask | `configure_orbs` — live mood only |

A field is a definition only if its applier is the field's one runtime reader.
A kind shares a flag when it shares an author. The definition path CONVERTS
integer lanes and never reinterprets them. The manifest emits `cad`, `inst` and
`scope` derived from the entry; the shell holds no block or kind number.

## Cadence — when does my edit land

| cadence | chip | derived from |
| --- | --- | --- |
| `LIVE` | silent | the default |
| `GEN` | `on respawn` | stored — the one fact the row must volunteer |
| `BOUNDARY` | `boundary` | a definition kind, a sentinel block, or `block_has_boundary` |
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
- `g_orb_def_touched` (over `OrbMoodConfig`) against `ORB_RESEED_BITS` —
  `enabled`, `count`, `palette_id`, `drag`: only those four re-seed the sky; the
  other fifteen ride the uniform upload. A raise with no bits means everything.

The frame boundary itself — doors, the re-speaks, the mask routing, the rule
window, the flush — is `src/cartridges/the_board/organ_boundary.inc`, member
functions of the cartridge, taken once per frame from `pawn.cpp`.

## Doors

A door raises a flag the boundary already consumes and adds no author.
`organ_doors()` emits the roster; the shell renders one button per row; presses
coalesce in a bitmask.
- `RESPEAK` — raise every definition flag for the live mood.
- `ORB_RULE`, `ORB_GESTURE` — the commands keys KP_8 / KP_7 press; the sky's
  two player-owned facts, which is why they are doors and not dials.
- the mood door, `organ_go_mood` — `request_mood_transition`, with its own guards.

The rule window (`organ_orb_rule`) packs rule, gesture, lit and count out of
`OrbsState`, refreshed at the boundary and nowhere else, so it cannot go stale
whoever turned the rule.

## The ABI

`organ_manifest`, `organ_doors`, `organ_mood_names` (JSON); `organ_set`,
`organ_door`, `organ_go_mood` (writes); `organ_get`, `organ_def_get`,
`organ_mood`, `organ_regime`, `organ_orb_rule` (reads, by manifest index);
`organ_param_count`, `organ_rejected_count`, `organ_last_reject`,
`organ_flush_count` (the status line). The manifest is emitted in table order,
so its index is the row's index. The ABI is inert until `bind_home` and `bind_mood` run at
the end of cartridge init; every entry point returns harmlessly before that,
and the shell polls until `organ_param_count` is nonzero.

## The shell

`web/organ_panel.js` is name-blind: it knows no parameter, range, offset, block
or kind. Its one ruled exception is `RULE_NAMES`, whose authority is
`bodies/orbs.hpp`; the only other thing it may know about a dial is its group
string and its label.
- Sections collapse; the panel opens as a table of contents. One filter over
  id + label + group. A section's tally reads `shown/total` whenever they differ.
- A group named `<rule> rule` or `Regime N` grows a scope line saying whether
  its rows act now. The regime lens — *this world's* / *regime N* / *all* —
  picks which regime's rows show; under *all* a write fans to every regime's
  kin (found by group name + label) and `≠` marks kin that disagree. The weight
  rows (`Atmosphere · Regimes`) are the one dormant-regime edit that moves the
  live world: a write re-rolls under the same seed at the boundary; `RESPEAK`
  keeps the seed.
- The row grid: label · swatch · chip on line one, slider · value on line two;
  every fixed width is a CSS custom property on `#organ` and the resize minimum
  is computed from them. A witness keeps its meter in the value column.
- Export — the whole desk or one section — keys `<id>` for instances and
  `<mood>/<id>` or `world/<id>` for definitions; witnesses export nothing.
  Import applies exactly what a file carries and counts unknown ids.
- The status line: refusals with the last one named, blocks reconciled at the
  last boundary, the build id.
- No storage of any kind. Width, open sections, lens and mode are session
  state; the URL carries a choice between sessions.

## Presets

`web/presets/index.json` is the shelf; `?preset=<name>` picks one at boot; a
select in the header picks one by hand. All three walk the import path. The
loader lives at module scope so an exhibition boots the scene without the panel.

For Jean: design on the panel, export, drop the JSON in `web/presets/`, add its
line to `index.json`. A preset is for DESIGN TIME. At SHIP TIME the values are
transcribed into the C++ design tables and the JSON is spent — one fact, one
home. A boot preset lands after the first world is drawn, so `GEN` rows reach
the first world only through the tables.

## The tuning loop

`?organ=1&mood=N&seed=N` boots into a mood under a pinned seed; the lens
follows the regime the seed drew. Export, shelve, and
`?preset=<name>&mood=N&seed=N` boots the tuned sky. The `[Atmos]` witness
prints `(mood, seed, regime)` once per regime, so a drag is silent and a regime
change under a weight dial is announced.

## Moods

Variants are moods: `open_sunset`, `open_night`, `open_noon` share one
`SHAPE_OPEN` and differ only in atmosphere. A new mood is one `SHAPE_` (or a
shared one) and one `ATMOS_` constant, one row in each positional per-mood
table (`MOOD_TABLE`, `ORB_MOOD_TABLE`, `AGENT_POPULATIONS`, `MOOD_SPAWN_MULT`,
`CUBE_POPULATIONS`), a portal colour and a weight in `WORLD_DRAW_TABLE`, and a
name in `MOOD_NAMES` — every table's assert names the commit it expects.

The destination law is one weighted table, `WORLD_DRAW_LIVE.mood_weights`,
walked by id (`pick_portal_mood`; `pick_open_mood` restricts the walk to open
shapes — the triad's way out of a room). A weight of 0 shuts a door without
unmaking the mood.

## Instruments

| instrument | claim |
| --- | --- |
| `tools/organ_gap.py` | the map of declared members the panel does not name; `--gate` fails on a design table with a surviving runtime reader |
| `tools/organ_readers.py` | a declared reader names every enrolled field |
| `tools/organ_ledger.py` | `audit/ORGAN.md` — every row with range, step and cadence; the music campaign's target map |
| `tools/gates/shell_gate/run.py` | the C++ ↔ shell seam: no sentinel in the shell; type enum, cadence length, scope constants and `RULE_NAMES` agree; every cwrap reaches an export at matching arity |
| `tools/gates/console_gate/run.py` | the translation units type-check against the pinned emdawnwebgpu surface |

The three organ tools share `tools/organ_parse.py`. Every instrument reads
text; the web boot is the witness of record for everything past the type
surface.

## What has no dial, and why

An enrollment states a belief and only the reader proves it; the converse also
has a register. Four facts survived every wave of the disposition survey
without earning a row, and each names its own owner rather than waiting on a
campaign.

| what | why not a dial | its owner |
| --- | --- | --- |
| `THEME_BASE_WEIGHT` | it is one scalar over a 5×N weight construction; the honest dial is the whole `MOOD_SPAWN_MULT` matrix, D5-large | a composite editor (D5) |
| `INDOOR_PALETTES[]` | mixed-shape rows, count read from `INDOOR_PALETTE_COUNT` (D5) | a composite editor (D5) |
| `tierset_id` | its "none" value is `0xFFFFFFFF`, and a 0…1 slider cannot express a sentinel without lying (D1(d)). `organ_gap` reports it, by name, as the one absent member of `OrbMoodConfig` | a composite editor (D5) |
| `mute_couplings` | a bitmask: `Coupling::ALL` is `0x1FFFFF` and a slider from 0 to 2 097 151 is not a dial. It wants a checkbox per bit — a shell feature, not an enrollment line — and **the bits are not there to check**: 21 bits wide, 8 of them named, so twenty-one checkboxes would be thirteen toggles over bits nothing reads. `Coupling::` is also a hand-kept mirror of `world.wgsl`'s `COUPLING_*` block, which D1's third branch reserves | a checkbox grid over a roster the C++ emits — priced, unbuilt, and **not CC's to choose** |

`D5` is the composite-editor deferral: a fact whose honest control is a grid
rather than a slider, priced rather than promised. `D1(d)` is the range
deferral: a value whose domain a slider cannot state without lying.

None is externally blocked, which is why none of them is in `docs/OPEN.md`:
OPEN is the register of open STATE, and a deferral any sitting can lift by
reading a module is unfinished survey, not open state.

**And one thing the next sitting should settle before it builds the strip.**
`Coupling::` masks the PROGRAM's couplings — terrain→pawn (y, tilt),
pawn→camera, the three input couplings, terrain→sphere-height and pawn→sun-VP.
There is no fog bit. The music campaign's couplings are a different vocabulary
that does not exist yet, and which one the strip is for changes what the strip
is.

## The tally

**365 entries — 348 dials and 17 witnesses.** Cut from `audit/ORGAN.md`, which
`tools/organ_ledger.py` regenerates from the enrollment list.

| section | rows |
| --- | --- |
| Agents | 109 |
| Atmosphere | 73 |
| Ribbon | 56 |
| Terrain | 42 |
| Sky & Light | 41 |
| Interaction | 22 |
| Pawn | 18 |
| Debug | 4 |

By cadence: `boundary` 180 · `live` 122 · `gen` 46 · `driven` 17.
By definition kind: none 189 · BEHAVIOR 70 · MOOD 55 · TIER 32 · ORB_MOOD 19.
