# ONE_WORLD · HANDOFF II — THE WEATHER · revB (supersedes revA whole)

> **SUBJECT unchanged:** the mood system, whole — seven moods, the
> theme engine, the indoor organs, the per-mood banks — replaced by
> three live banks and a pinned FINITE world. **revB is revA
> re-authored from a fresh reading of the synced tree**: the tripwire
> is answered, the regime question is settled by the tree's own
> assert, and the banks land on institutions the tree already built.
> Where revA and revB disagree, revB governs. Doctrine: Handoff I's
> §0 with Amendments A, B, C; the P8 ruling's "latency is not
> exemption"; symbols not lines; U0's census outranks this text.
>
> **CORPUS WARNING for CC:** the project knowledge Claude read is
> mixed-era (old and current files coexist). Every anchor below was
> era-dated by content before use, but the tree remains the only
> witness — verify by symbol, as always.

---

## §1 — SETTLED FACTS (fresh recon; each names its witness)

1. **Every mood weights one regime.** `MOOD_TABLE` rows all carry
   `{1,0,0,0}` and the tree asserts it ("EVERY MOOD WEIGHTS ONE
   REGIME TODAY"). The console capture's `regime=1` was 1-based
   display. **The dominant-weight amendment is retired**: ATMOS_LIVE
   seeds from the boot row's `regime[0]`, exactly.
2. **The regime ARRAY dies with the roll.** With one regime ever worn
   and the roll dead, `Regime regime[REGIME_COUNT]` is four slots for
   one fact. ATMOS_LIVE is **flat**: the sun's bearing
   (`sun_direction` + az/el spreads) + the twelve regime fields
   (centres + spreads), one struct, no array, `REGIME_COUNT` and
   `regime_weight` gone. The parked "mood-wide flag" price in OPEN.md
   dies moot with them.
3. **The DRAW survives; only the ROLL dies.** `draw_atmosphere` — the
   seed's draw of one sky from a distribution, spread-aware,
   short-circuit-on-point — is the bank's gen-cadence applier. The
   tree's own banner already promises the end state: *"The panel
   writes the distribution, and the draw moves WITH the dial rather
   than re-rolling."* II makes that sentence true.
4. **The tripwire is answered: the shell dies whole.** The finite
   boundary is the **containment clamp** (`world_bound`, the b3
   ruling: "an invisible wall, not terrain extent —
   MOOD_FINITE_OUTDOOR is finite with no walls"). The shell mesh
   (walls + ceilings + `INDOOR_PALETTES`) is indoor-only
   (`CeilingType::NONE` = no shell geometry). No wall graduates;
   U4 takes the whole organ. `veil_strength`'s finite staging (0 —
   "walls define the boundary, not fog") keeps its behavior; its
   comment is probated to name containment.
5. **The banks land on standing institutions, not new ones.**
   `contracts/control_panel.hpp` is "THE PANEL — one home, every
   room" with `PANEL_LIVE` as the live-bank precedent (ORGAN_3);
   `population_themes.hpp` is by its own banner "the population
   panel." So: **ATMOS_LIVE** stands as the atmosphere panel
   (contracts tier, ORGAN_3 shape — own header, it is a large
   family); **POP_LIVE** lands by **refounding the population panel**
   — the theme engine inside `population_themes.hpp` dies, the
   file's panel role survives as the home of POP_LIVE + the five
   families' spawn dials + `GLOBAL_ENTITY_DENSITY`; **ORB_LIVE**
   follows the same ORGAN_3 shape at the orbs' contracts home.
6. **The persistence ladder amends, in its documented home.** Rung 3
   ("authored by apply_mood at entry") collapses into rung 1 (the
   banks) + rung 4 (the world's draw). U7 amends the ladder prose in
   `docs/ORGAN.md`'s section and every citation of it — an amendment
   with this campaign named, never a deletion.
7. **`finite_radius_min/max` are the pin's dials.** The shape draws
   the radius from a seed-ranged pair; the pin keeps the draw and
   enrolls **min and max** (two dials), radius per-world as the
   parametric spirit wants. `WorldShape` itself dies with the moods;
   the two facts that survive it are `finite=true` (the pin) and the
   radius range (the dials) — rehomed beside `finite_mode` at
   `WorldState`, minimal form.

---

## §2 — SCOPE (revA's, with the settled deltas)

**KILL:** the seven moods, `MOOD_TABLE`/`MOOD_LIVE`/`mood_def`/
`MOOD_NAMES`/`MoodState` remains/`--mood`/`DEMO_BOOT_MOOD`; the F-3
row pins and column witnesses and carry witness (they die WITH their
table — the new bank gets its own witnesses only where an invariant
survives, per Amendment A); `apply_mood` and the per-mood appliers
(collapse to `stage_world_birth`); the regime roll, `REGIME_COUNT`,
`regime_weight`; `MOOD_SPAWN_MULT` + traits' `mood_multiplier` (T7
pins updated same commit); `GoLState.mood_allowed`; the theme ENGINE
(themes, lattice, envelopes, cooldowns, `theme_tier_weights`,
`theme_short_name`); the indoor organs whole — shell mesh + pipeline
+ `ShellVertex` + `Dim::SHELL_*`, `CeilingType`, `INDOOR_PALETTES`,
`INDOOR_LIVE` + `IndoorSize` (incl. `EXACT`), spot lights (array,
atlas half, pipelines, `SPOT_PCF_*`, WGSL), wall-frame residue if
reader-less; `WorldShape` + `SHAPE_*` + `SCHEME_ROLL`/`PALETTE_ROLL`
sentinels (the indoor light-scheme/palette rolls die with their
subjects); the ribbon's dormant indoor treatment (flagged); the
mood/scheme facts of `WORLD_DRAW_LIVE`; `organ_mood_names`,
`ORGAN_SCOPE_MOOD`, the DEFONLY-MOOD sentinel and every
`MoodProfile.*` DEFONLY row (re-homed, U6).

**BUILD:** ATMOS_LIVE (flat, §1.2, seeded from
`MOOD_TABLE[MOOD_OPEN_SUNSET]` — sunset remains the ruled seed
absent Jean's word), ORB_LIVE, POP_LIVE (§1.5 homes;
`spawn_population`/`respawn_evicted_agents` shed `mood_id`;
`wanderers` roster bit stays); the FINITE pin (§1.7; boots true,
loud boot print, nothing unsets it; open-mode code paths untouched —
campaign 3's subject).

**KEEP / NOT IN SCOPE:** streaming, veil chain, LOD, `world_young`,
`reset_surface`, `rebirth_world` (P8 — **first-class reader**: its
mood reads rewire to the banks like any live caller's, its narration
survives), the coupling layer (fog rest re-anchors to ATMOS_LIVE,
same composition law), GoL zones, agents' behaviors/tiers, pyramids,
sphere, ribbon, cube, orbs' commands, `GLOBAL_ENTITY_DENSITY`.

**Parked with THE PANEL (OPEN.md, one line, U7):** the orphan console
verbs CC's sweep surfaced (`cycle_cube_behavior_override`,
`reveal_zoetrope`, `toggle_cube_kite_mode`, `unrecord_entity`, ~40
accessor leads) await the panel's own recon — none is II's subject
except `theme_short_name`, which dies with its engine in U3.

---

## §3 — U0 · RECON (no edits, plus two housekeeping commits)

1. Verify I's close (roster COUNT 5, no `TransitionPhase`, battery
   green) and land the **P8 marking commit** if CC's `99c2891f` line
   is not already merged where II bases.
2. **DOORS-escape check:** the `Sky & Light · Portals` organ rows
   (`WORLD.portal_colors`, `portal_color_back`) must already be dead;
   if any linger, take them as flagged housekeeping — DOORS scope,
   II's broom.
3. Baseline battery; verdicts pasted.
4. Enumerate by symbol (the settled facts above narrow this list):
   `apply_mood`'s full fan and every reader of
   `mood_state_.active` / `MOOD_LIVE` / `mood_def`; the ORB_MOOD
   table + `configure_orbs` signature; `AGENT_POPULATIONS` + its twin
   sum tables; the organ's MOOD-kind and ORB_MOOD-kind row counts and
   both DEFONLY sentinels; preset export/import keying and the
   refusal precedent; the shell's build path (`mood.hpp`'s shell
   generator, `[Shell]` print) and the spot half of the shadow atlas
   (exact split); the `--seed/--cap/--msaa/--mood` boot loop's shape;
   `WORLD_DRAW_LIVE`'s mood/scheme facts; the ribbon's dormant
   indoor cells; every citation of the persistence ladder.

## §4 — THE UNITS (revA's spine, revB's content)

**U1 — THE BANKS RISE** (constructive; one commit). Declare
ATMOS_LIVE (flat, §1.2) / ORB_LIVE / POP_LIVE at their §1.5 homes,
seeded constexpr from the sunset rows; rewire every runtime applier —
`draw_atmosphere` (now the bank's draw), lighting, `configure_orbs`,
spawn/respawn, the fog rest — to the banks. The mood tables become
single-purpose seeds with no runtime reader. `rebirth_world`'s apply
tail rewires in this commit (latency is not exemption). Green
mid-state, a graduation, no zero slots.

**U2 — MOODS FALL** (one commit). §2's mood list whole; boot's
sequence becomes `stage_world_birth` (atmosphere draw from
ATMOS_LIVE, lights, `configure_orbs(ORB_LIVE, reseed)`, population
from POP_LIVE); `--mood` leaves the loop; `mood_constants.hpp` dies
or shrinks to survivors; the finite facts rehome per §1.7. Twin
rooms + asserts per commit.

**U3 — THEMES FALL, THE PANEL REFOUNDED** (one commit). The engine
dies inside `population_themes.hpp`; the file survives as the
population panel proper (§1.5) — banner rewritten, POP_LIVE + spawn
dials resident. `compose_spawn_chance` simplifies. **The report
tables the five survivors' post-theme effective spawn numbers** — no
tuning, numbers only (the silent-survivor check; `sph` showed 0 in
the old world).

**U4 — INDOOR FALLS, WHOLE** (one commit; tripwire answered — no
graduation). Shell + `ShellVertex` + `Dim::SHELL_*` + pipeline +
`[Shell]` narration, `CeilingType`, `INDOOR_PALETTES`, spot organs
and the atlas's spot half (recon's split), `SCHEME_ROLL`/
`PALETTE_ROLL`, Lighting collapses to sun + ambient in both rooms.
`veil_strength`'s comment probates to containment (§1.4).

**U5 — THE PIN** (one commit, small and loud). `finite_mode` boots
true; `[World] Born FINITE radius=…` per the rider's §B; min/max
radius dials enroll (§1.7); open-mode branches untouched.

**U6 — THE ORGAN RE-HOME** (one commit). MoodProfile's DEFONLY rows
→ ATMOS_LIVE live rows (blocks renamed from "Atmosphere · Regime N"
to the one atmosphere; four-regime rows collapse to one set);
ORB_MOOD rows → ORB_LIVE; POP_LIVE enrolls; `ORGAN_SCOPE_MOOD`,
DEFONLY-MOOD sentinel, `organ_mood_names`, follow-the-mood machinery
die; preset keys drop the mood prefix and stale keys **refuse loudly
by name** (the `mood_def` precedent); `derived_scope`/
`derived_cadence` simplify.

**U7 — SWEEP** (one commit). Prose probate of
mood/theme/indoor/spot/atrium/shape referents (Amendment B, WGSL
orphan scan mandatory); the persistence-ladder amendment (§1.6);
LAWS whose subject died get struck-notes naming this campaign, each
flagged, none deleted; stamped minutes untouched; indoor reference
docs to the attic; OPEN.md — the close line, the panel parking line
(§2), the mood-wide-flag price line retired (§1.2).

**U8 — INSTRUMENTS + AUDIT + BATTERY** (two commits). Score rows for
dead phases/subjects; `KNOWN_FAMILIES` −spot_lights −indoor_shell;
`organ_readers.py` applier lists become the bank appliers; glaw2
`--record`, pure-deletion check; regenerate the room; L33 witness;
full battery, verdicts verbatim. **Close with the rider's §C.2:**
the native boot log quoted beside the target transcript — Jean's
build supplies it at the walk.

## §5 — DEPENDENCY MAP (unchanged from revA)

U1←U0 · U2←U1 · U3←U0 · U4←U0(+U2 for mood-named bits) · U5←U0 ·
U6←U1,U2 · U7←landed · U8←landed. Quarantines convert to flags per
the standing table; nothing halts the campaign.

## §6 — COMMITS, REPORT, CLOSE

`ONE_WORLD-II U<n>: <subject> — the weather`; tombstones list paths
(L30). Report adds the **bank-seeding proof** (booted bank values
byte-equal sunset's row, spreads included) and the **applier-rewire
proof** (no runtime mood-table reader survives U1). Close: glaw1 +
build + the walk (sunset look unchanged; no rooms, no spot pools;
FINITE announced; orbs and agents present; boot log vs §B). Naming,
merge, tags, flag ledger — Jean's. If committed under
`docs/HANDOFFS/`, the close deletes the round's files (L31).
