# THE DISPOSITION LEDGER — ORGAN_3

The document naming every stop the instrument has, and every one it does
not. One table per module. A **coupling is a parameter set into
trajectory over time**, so every ENROLL row here — with its authored
(min, max) — is a trajectory domain the music campaign will play. This
is the coupling campaign's target map, not panel decoration.

**Fate of this file.** It stays in `docs/HANDOFFS` as ORGAN_3's working
file; Jean and Claude rule its final form at the close. Until then it is
the campaign's resumption point (Law 5): ledger + phase pushes are the
complete handoff to a successor.

**DEFER rows that died this campaign,** each in the commit that landed it:
the canvas tier (P2), `ORB_MOOD_TABLE` → `ORB_MOOD_LIVE` (P3), and
`FIELD_BEACON_S` (P5 — the only one that needed a ruling rather than a
reading). **Nine survive**, and each names its own owner in place:
`THEME_BASE_WEIGHT`, `INDOOR_PALETTES[]` and `tierset_id` want a
composite editor (D5); `mute_couplings` wants a checkbox per bit, which
is a shell feature, not an enrollment line; `POSSESSION_RADIUS` wants its
derived `_SQ` twin read first; Wave 3's remaining destructive banks and
Wave 4's three seams (the ribbon's four pipes, `floater_coordination`,
the four mood-structural witnesses) are bulk and pattern, not difficulty
— the `INDOOR_LIVE` and checker-seam precedents are already in the tree.
None is externally blocked, which is why none of them is in
`docs/OPEN.md`. The one that was — the beacon's assert — is the line P5
killed there.

**Why the deferrals are here and not in `docs/OPEN.md`.** OPEN is the
register of open STATE, one line per item, and its law is that a line
dies when its item closes. A DEFER that any future sitting can lift by
reading a module is not open state — it is unfinished survey, and it
belongs with the survey. Only an **externally blocked** deferral earns an
OPEN line. ORGAN_3 produced exactly one such: the beacon's static_assert,
which needs a ruling before any sitting can lift it.

## The five classes

| class | signature | recipe |
| --- | --- | --- |
| **C1 LIVE** | reachable home; no runtime author after boot (boot pins allowed) | one `ORGAN_PARAM` line |
| **C2 GRADUATE** | authored constexpr in a module; no live home | table stays; LIVE bank in contracts; readers move; enroll against the bank |
| **C3 EVENT** | a named author re-speaks it at an event | LIVE bank + definition kind + boundary re-speak (idempotent) or deferral (destructive) |
| **C4 DRIVEN** | a per-frame author writes it | rests + gains at the seam, `_RO` witnesses on the driven values |
| **C5 BOOT** | structural: world generation reads it, or a compile-time law | never enrolled live; recorded here with a one-line reason |

Range evidence tags (D1): `[tree]` the tree's own word · `[identity]`
physical identity · `[heuristic]` the authored value's envelope — the tag
Jean's eye visits first.

Verdicts: `ENROLL w<n>` (wave) · `BOOT <reason>` · `DEFER <reason>`.

---

# 1 · `contracts/`

The contracts are already panels; several are already *transported* into
`GPUDesignConfig` by the boot-pin block (`state.hpp initializeState`),
which makes them **C1 against the config home** rather than C2 — the
graduation the tier vocabulary needed has, for these, already happened.

## contracts/control_panel.hpp — THE FIELD

Every one of these is boot-pinned into `config_.field_*` /
`config_.point_bubble_radius` (state.hpp:4867-4874, 4861), so the home
exists and the panel line is one `ORGAN_PARAM` against `GPUDesignConfig`.
Boot pin is the only author; no runtime writer exists (`set_*` for these
fields: none).

| param | home | shape | authored | authors | readers | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `FIELD_SLACK` | `GPUDesignConfig.field_slack` | F32 | 3.0 | boot pin only | world.wgsl field_pair, ribbon.hpp CPU sum | C1 | Interaction · Field | 0 … 12 `[heuristic]` | ENROLL w1 |
| `FIELD_K` | `.field_k` | F32 | 300.0 | boot pin | both dialects | C1 | Interaction · Field | 0 … 1200 `[heuristic]` | ENROLL w1 |
| `FIELD_FMAX` | `.field_fmax` | F32 | 600.0 | boot pin | both dialects | C1 | Interaction · Field | 0 … 2400 `[heuristic]` | ENROLL w1 |
| `FIELD_OCCUPIER_GAIN` | `.field_occupier_gain` | F32 | 1.0 | boot pin | both | C1 | Interaction · Field | 0 … 4 `[heuristic]` | ENROLL w1 |
| `FIELD_AUTHORED_GAIN` | `.field_authored_gain` | F32 | 1.0 | boot pin | both | C1 | Interaction · Field | 0 … 4 `[heuristic]` | ENROLL w1 |
| `FIELD_GAIN_CUBE` | `.field_gain_cube` | F32 | 4.0 | boot pin | WGSL subscriber gain | C1 | Interaction · Field | 0 … 16 `[heuristic]` | ENROLL w1 |
| `FIELD_GAIN_SPHERE` | `.field_gain_sphere` | F32 | 1.0 | boot pin | WGSL | C1 | Interaction · Field | 0 … 4 `[heuristic]` | ENROLL w1 |
| `FIELD_GAIN_AGENT` | `.field_gain_agent` | F32 | 4.0 | boot pin | WGSL | C1 | Interaction · Field | 0 … 16 `[heuristic]` | ENROLL w1 |
| `POINT_BUBBLE_RADIUS` (point.hpp) | `.point_bubble_radius` | F32 | 20.0 | boot pin (state.hpp:4861) | contact chain, WGSL | C1 | Interaction · Point | 0 … 80 `[heuristic]` | ENROLL w1 |

### The beacon (FIELD_4) — verified, not assumed

The four beacon constants have **no live home**: `phase_motion_drivers`
(cartridge.hpp:1126-1136) composes `GPUFieldAuthored` from the constexprs
*every frame* and uploads it. So the transport is per-frame but the
*value* is authored — **C2** with a per-frame transport, not C4: there is
no driven value to witness, only a rest the writer re-sends. One dial is
already gained live — `fa.rows[0][3] = FIELD_BEACON_S * coord`, where
`coord` is `config.floater_coordination`.

| param | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- |
| `FIELD_BEACON_R0` | F32 | 25.0 | C2 | Interaction · Beacon | 0 … 100 `[heuristic]` | ENROLL w2 |
| `FIELD_BEACON_R` | F32 | 120.0 | C2 | Interaction · Beacon | 0 … 480 `[heuristic]` | ENROLL w2 |
| `FIELD_BEACON_LIFT` | F32 | 20.0 | C2 | Interaction · Beacon | 0 … 80 `[heuristic]` | ENROLL w2 |
| `FIELD_BEACON_S` | F32 | 200.0 | C2 | Interaction · Beacon | 0 … `FIELD_K − 1` `[tree]` | ENROLL P5 — **the DEFER died**; see the adjudication below |

**THE BEACON'S ASSERT — a standing hazard this campaign must not break.**
`control_panel.hpp` closes with

```cpp
static_assert(FIELD_BEACON_S < FIELD_K,
    "the beacon's pull must lose to field repulsion at the ring — "
    "otherwise the gather clots instead of spacing (FIELD_4's ruling)");
```

A compile-time proof cannot guard a runtime dial. `FIELD_BEACON_S` was
DEFERRED for exactly that: enrolling it live would let the panel push the
beacon's pull past `field_k` and clot the gather, with the static_assert
still reading true and no runtime witness. Two ways out were priced — a
clamp at the writer, or the ruling restated as a paired range.

**ADJUDICATED AT ORGAN_3b P5: Jean stamped the clamp,** because it guards
every future author — a coupling included — and not merely the panel.
`FIELD_BEACON_S` enrolls with `FIELD_BEACON_S_MAX = FIELD_K − 1` as its
max, so the range and the clamp agree by construction at the authored k;
and the writer clamps against config's **live** `field_k`, not the
constexpr.

That second half is not decoration. This ledger said above that `FIELD_K`
*"only ever raises the winning side, so the ruling survives every value
that dial can reach"* — **that was wrong, and the `.inc` carried the same
sentence.** `field_k`'s enrolled min is 0; lowering it under S breaks the
same ruling from the other end, and nothing guarded that. Reading the live
`field_k` at the writer closes both ends at once. Both stale sentences are
corrected in place; the harness now walks every k the dial can reach and
proves `S < k` at each.

## contracts/driver_surface.hpp — the drivers' room (ORGAN_2a)

Fully enrolled at 2a: `fog.rest_density`, `fog.rest_color`, `fog.gain`,
`aura.intent`, `aura.attack`, `aura.release`, `aura.height_gain`. **No
gaps.** It is the one contracts bank that legitimately spans subjects,
and Wave 4 grows it.

## contracts/agent_tiers.hpp — the world's bank (ORGAN_2b)

`TIER_LIVE` enrolled for `color_r`(VEC3) and `speed_gain` per tier — 8 of
the bank's 48 live fields. The gap, per tier: `step_gain`,
`persist_gain`, `contact_radius`, `contact_mass`, `personal_radius`,
`flee_gain_player` (`id`/`name` are identity, never dials).

| param (×4 tiers) | shape | authored (t0..t3) | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- |
| `step_gain` | F32 | 1.0 / 1.8 / 0.6 / 1.2 | C3 TIER | Agents · Tier N | 0 … 4 `[heuristic]` | ENROLL w3 |
| `persist_gain` | F32 | 1.0 / 0.4 / 1.2 / 0.9 | C3 TIER | Agents · Tier N | 0 … 4 `[heuristic]` | ENROLL w3 |
| `contact_radius` | F32 | 1.6 / 1.4 / 2.0 / 1.8 | C3 TIER | Agents · Tier N | 0 … 8 `[heuristic]` | ENROLL w3 |
| `contact_mass` | F32 | 1.0 / 0.8 / 1.5 / 1.2 | C3 TIER | Agents · Tier N | 0 … 4 `[heuristic]` | ENROLL w3 |
| `personal_radius` | F32 | 30.0 all | C3 TIER | Agents · Tier N | 0 … 120 `[heuristic]` | ENROLL w3 |
| `flee_gain_player` | F32 | 0.70 / 0.85 / 0.50 / 0.60 | C3 TIER | Agents · Tier N | 0 … 0.99 `[tree]` | ENROLL w3 |

**THE CATCHABILITY LAW** (CONTACT_4, quoted in `agent_tiers.hpp`) is the
tree's own word on `flee_gain_player`'s max: *"a gain >= 1.0 means the
agent matches or beats the player's radial speed and can NEVER be
approached (nor possessed) … keep every row < 1.0."* Max `0.99`, tagged
`[tree]` — the one range in this campaign a law dictates rather than a
heuristic, and the clearest case of the range column doing real work: a
panel that let this reach 1.0 would silently make the world uncatchable.

The tier bank rides the **existing** TIER re-speak, so these six cost one
enrollment line each and no new machinery.

## contracts/spine_state.hpp — MoodProfile

The **atmospheric group** (`sun_direction`, `sun_color`, `sun_intensity`,
`sun_ambient`, `clear_color`) is fully enrolled through ORGAN_1/2b/2c.
The **structural group** (`finite`, `finite_radius_min/max`, `indoor`,
`ceiling_type`, `wall_height`, `terrain_amp_ceiling`, `allow_*`) is
**C5** by the standing eligibility rule stated beside `MOOD_LIVE`. Not
relitigated (§2's instruction).

## contracts/indoor_module.hpp — the first destructive bank

| param | shape | authored | authors / readers | class | verdict |
| --- | --- | --- | --- | --- | --- |
| `INDOOR_HEIGHT_CAP_FRACTION` | F32 | 0.75 | **two temperaments**: `apply_mood_lighting` → `set_indoor_height_cap` (idempotent) AND `cap_to_ceiling` at every entity spawn — entity_pipeline ×3, grounded ×3, spheres, cube_behaviors, ribbon (destructive) | C3 | ENROLL w3 — Terrain · Indoor, 0 … 1 `[identity]`, **destructive banner** |
| `RIBBON_INDOOR_SCALE` | F32 | 0.15 | ribbon.hpp:1150-1153, the indoor selection scale ("Jean's dial — tune on sight") | C3 destructive | ENROLL w3 — Ribbon · Indoor, 0 … 0.6 `[heuristic]` |
| `INDOOR_ENTITY_WALL_MARGIN` | F32 | 20.0 | spawn placement | C5 | BOOT — world generation reads it at placement |
| `COLUMN_MIN_INDOOR_HEIGHT` | F32 | 1.0 | column build | C5 | BOOT — a geometry floor, read at build |
| `INDOOR_TREATMENT[PopFamily::COUNT]` | table | per-family size + bounds | spawn | C5 | BOOT — mixed-shape rows (D5). A composite editor would need a size enum and a bounds pair per family; pricing, not promising. |

`INDOOR_HEIGHT_CAP_FRACTION` is the campaign's **exemplar of the
temperament law**: one constant with an idempotent author and nine
destructive ones. The stricter temperament governs, so the group banner
says *edits the next spawn* — the mood-apply half will pick it up at the
next mood change without any boundary wiring, exactly as a non-live
mood's definition already waits.

## contracts/mood_constants.hpp

`MOOD_COUNT` and the four mood ids are **C5** (compile-time law;
`MOOD_LIVE`'s static_assert stands on `MOOD_COUNT`). `PORTAL_COLORS[4][3]`
and `PORTAL_COLOR_BACK[3]` have one live consumer — `portal_color_for()`
(mood_constants.hpp:53), called from `grounded.hpp` when a portal is
built — so they are **C3 destructive**: a 5-row colour table, ≤8 rows by
D5, `Sky & Light · Portals`, lanes `0 … 1 [identity]`, ENROLL w3 with the
*edits the next spawn* banner.

## contracts/floaters.hpp

Read whole. The live vocabulary is `ActiveSphere`/`ActiveCube` plus two
tier registries: `SPHERE_TIER_COUNT` / `CUBE_TIER_COUNT` and their
`*_TIER_NAMES` (**C5** — counts and identity), `SPAWN_PROTECTION_S`
(0.10, **C5** — a spawn-race guard, not a dial), and
`SPHERE_BASE_TIER_WEIGHTS` / `CUBE_BASE_TIER_WEIGHTS` — spawn-roll
weights, **C3 destructive**, ≤8 rows, `Agents · Floater mix`,
`0 … 1 [identity]`, ENROLL w3. The remainder of the file's constants are
per-tier geometry read during spawn: **C5**.

## contracts/entity_types.hpp · ground_architecture.hpp · roster.hpp · spawn_services.hpp · surface_services.hpp

Surveyed: **C5 wholesale**, one reason — these are the machine's decl
tier. Pipeline contracts, boundary DTOs, the dispatch row/table, the
ground contributor DAG with its compile-time checks, `MIN_SEPARATION`,
the patch registry and its budgets. Every constant here is read during
world generation or stands under a `static_assert`; none has a runtime
home, and giving one a live dial would mean nothing at best (the world is
already built) and disagree at worst.

---

# 2 · `surface/`

## surface/terrain_looks.hpp — THE TERRAIN_LOOKS PANEL

The single richest **C1** vein in the tree. Every ROW 1 and ROW 2 value
is written into `GPUDesignConfig` by boot pins (ROW 1 in
`GPUState::initializeState`, ROW 2 through the setters in
`Cartridge::initialize`, cartridge.hpp:585-620) and — verified by
writer census — **nothing else authors them**. The panel's own comments
carry the ranges, so almost every tag below is `[tree]`, not
`[heuristic]`: the file was written to be read this way.

The mode trio's status is the panel's own word: *"the mode trio is
DRIVERLESS since gen-1 retirement — driver-ready, held at rest by this
row."* Enrolling them is exactly what "driver-ready" was waiting for.

### ROW 1 — the palette quartet

| param | home | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `PALETTE_CENTER_REST[0..3]` | `config.palette_center[i]` | VEC3 ×4 | sand/salmon/green/warm | C1 | Terrain · Palette N | 0 … 1 `[tree]` "rgb 0-1" | ENROLL w1 |
| `PALETTE_LIGHT_REST[0..3]` | `config.palette_light[i]` | VEC3 ×4 | ditto | C1 | Terrain · Palette N | 0 … 1 `[tree]` | ENROLL w1 |
| `PALETTE_WEIGHT_REST[0..3]` | `config.palette_weight[i]` | F32 ×4 | .42/.28/.04/.26 | C1 | Terrain · Palette mix | 0 … 1 `[tree]` "selection probability" | ENROLL w1 |

`palette_weight` "sums 1.0" is a *convention*, not a clamp — the panel
cannot enforce it and should not pretend to. Four independent lanes;
the WGSL normalises. Noted so no future sitting mistakes the sum for a
law.

### ROW 2 — motion & mode rest pins

| param | home | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `REST_TERRAIN_TIME` | `config.terrain_time` | F32 | 0.0 (frozen) | C1 | Terrain · Motion | 0 … 64 `[heuristic]` (beats; ≤0 freezes) | ENROLL w1 |
| `REST_BAND_BLEND[0..5]` | `config.band_blend_N` | F32 ×6 | −1 (inactive) | C1 | Terrain · Bands | −1 … 1 `[tree]` "[0,1] per band, −1 = inactive sentinel" | ENROLL w1 |
| `REST_BAND_PHASE_ORIGIN[0..5]` | `config.band_phase_origin_N` | F32 ×6 | 0 | C1 | Terrain · Bands | 0 … 64 `[heuristic]` (beats) | ENROLL w1 |
| `REST_MODE_COLOR_SHIFT` | `config.mode_color_shift` | F32 | 0 | C1 | Terrain · Modes | −1 … 1 `[tree]` "mode-field bias [−1,1]" | ENROLL w1 |
| `REST_MODE_CHECKER_SCATTER` | `config.mode_checker_scatter` | F32 | 0 | C1 | Terrain · Modes | −1 … 1 `[tree]` | ENROLL w1 |
| `REST_MODE_PALETTE_DRIFT_TARGET` | `config.mode_palette_target` | F32 | 0 | C1 | Terrain · Modes | 0 … 3 `[tree]` "[0,3] target palette idx" | ENROLL w1 |
| `…_INTENSITY` | `config.mode_palette_intensity` | F32 | 0 | C1 | Terrain · Modes | 0 … 1 `[tree]` | ENROLL w1 |
| `…_TIER` | `config.mode_discrete_tier` | F32 | 0 | C1 | Terrain · Modes | 0 … 4 `[tree]` "[0,4] discrete tier" | ENROLL w1 |
| (GoL scales) | `config.mode_gol_tick_scale` | F32 | 1.0 | C1 | Terrain · GoL | 0.1 … 4 `[heuristic]` | ENROLL w1 |
| (GoL scales) | `config.mode_gol_height_scale` | F32 | 1.0 | C1 | Terrain · GoL | 0 … 4 `[heuristic]` | ENROLL w1 |
| `REST_CHECKER_RESULTANT/_AMOUNT/_VARIANCE` | `config.checker_*` | VEC3+2×F32 | 0 | **C4** | Atmosphere · Checker | — | witnesses, w4 (the U4 flush) |
| `REST_PULSE_COUNT` + `pulse_data[32]` | `config.pulse_*` | U32 + 32×F32 | 0 | C5 | — | — | BOOT — a ring buffer is not a dial (D5: 8 rows × 4 mixed fields) |

ROWS 3-8 live in the WGSL room by the file's own two-rooms rule: **C5**,
counted in §7 below.

## surface/population_themes.hpp

| param | shape | authored | class | verdict |
| --- | --- | --- | --- | --- |
| `THEME_LATTICE_SPACING` | F32 | 500.0 | C5 | BOOT — world generation lattices on it |
| `THEME_SEED_BAND` | U32 | 170 | C5 | BOOT — an RNG address, never a dial |
| `THEME_COUNT` | U32 | 5 | C5 | BOOT — a count |
| `THEME_BASE_WEIGHT` | F32 | 10.0 | C3 destructive | ENROLL w3 candidate → **DEFER**: it is one scalar over a 5×N weight construction; the honest dial is the whole `MOOD_SPAWN_MULT` matrix, D5-large |
| `MOOD_SPAWN_MULT[4][PopFamily::COUNT]` | table | per-mood family multipliers | C5 | BOOT (D5 — larger than 8 rows once flattened). A composite editor would need a mood × family grid; pricing, not promising |
| `THEMES[THEME_COUNT]` | table | mixed (names + weights + family sets) | C5 | BOOT — mixed shapes (D5) |

---

# 3 · `direction/`

## direction/input.hpp — CameraControls

| param | shape | authored | authors / readers | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `MOVE_SPEED` | F32 | 30.0 | boot pin → `config.point_fly_speed` (cartridge.hpp:605) | C1 | Interaction · Camera | 0 … 120 `[heuristic]` | ENROLL w1 |
| `LOOK_SENS_INIT` | F32 | 0.005 | the look-sensitivity clamp's anchor; keys step it | C2 | Interaction · Camera | 0.000625 … 0.04 `[tree]` (init ÷8 … init ×8) | ENROLL w2 |
| `LOOK_SENS_STEP` | F32 | 1.25 | per keypress, multiplicative | C2 | Interaction · Camera | 1.01 … 2 `[heuristic]` | ENROLL w2 |
| `LOOK_SENS_RANGE` | F32 | 8.0 | the clamp's half-width | C2 | Interaction · Camera | 1 … 32 `[heuristic]` | ENROLL w2 |
| `SCROLL_ZOOM_SCALE` | F32 | 2.0 | orbit distance per wheel notch | C2 | Interaction · Camera | 0 … 8 `[heuristic]` | ENROLL w2 |

`LOOK_SENS_RANGE`'s `[tree]` range on `LOOK_SENS_INIT` is the file's own
clamp — *"init ÷R … init ×R"* — read off the code, not guessed.

**Note on the missing Camera section.** §0 rules there is deliberately no
`Camera` section: camera pose is GPU truth, the sovereignty boundary made
visible. These four are *controls*, not pose — they live under
`Interaction · Camera`, which is the operator's word for the input
grammar, not a home. Runner-up section (D3): `Debug`. Noted, not
deliberated.

## direction/input.hpp — the key doors

`set_fpv_mode` (key) and `set_veil_dither` (key) write config fields that
no *program* author touches. They are **C1 key-shared**: the panel and
the keyboard are two operators on one dial, and the contest column will
read EVENT when the key is used — the instrument working as designed
(the aura-intent precedent, ORGAN_2a). `config.fpv_mode` and
`config.veil_dither` ENROLL w1 as BOOL under `Debug` and `Atmosphere ·
Veil` respectively.

## direction/mood.hpp

| param | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- | 
| `PORTAL_DENSITY` | F32 | 1.00 ("fraction of Doorway arches that become portals") | C3 destructive | Agents · Portals | 0 … 1 `[tree]` "fraction" | ENROLL w3 |
| `FINITE_OUTDOOR_CHANCE` | F32 | 0.10 | C3 destructive (world draw) | Agents · Portals | 0 … 1 `[identity]` | ENROLL w3 |
| `SCHEME_WEIGHTS[4]` | F32 ×4 | .42/.43/.10/.05 | C3 destructive (indoor light scheme roll) | Sky & Light · Schemes | 0 … 1 `[identity]` | ENROLL w3 |
| `INDOOR_PALETTES[]` | table | wall/ceiling colour sets | C3 destructive | Sky & Light · Indoor | — | DEFER — mixed-shape rows, count read from `INDOOR_PALETTE_COUNT` (D5) |
| `LIGHT_SCHEMES[SCHEME_COUNT]` | table | per-scheme slot arrays | C5 | — | — | BOOT — nested slot tables (D5); the per-slot field ids (`LATERAL`…`AIM_YAW`) are RNG addresses |
| `SCHEME`/`WALL_PAIR`/`ANCHOR_PICK`/`SLOT_BASE` + field ids | U32 | 1100-1110 | C5 | — | — | BOOT — **RNG salts are addresses, not dials** (§0) |
| `SCHEME_NAMES` / `ANCHOR_NAMES` | strings | — | C5 | — | — | BOOT — identity |
| `VAULT_RISE_FRACTION` (0.30), `MIN_RISE_FLOOR` (5.0), `JOINT_OVERLAP` (3.0), `WALL_FLOOR` (−50), `VAULT_N` (32), `MAX_OUTER_HALF` (1.3) | F32/U32 | — | C5 | — | — | BOOT — indoor shell geometry, read while the world is built |

---

# 4 · `bodies/`

The entity-module pattern is the spine — the ribbon's own banner names
it: *tuning console → registry → tiers → runtime state*. Four modules
were **read whole** and are surveyed at parameter granularity below
(`pawn`, `agents`, `orbs`, `ribbon`). Five were surveyed **by sweep**
and are flagged, not enrolled — see §4.6.

## 4.1 bodies/pawn.hpp

Fully dispositioned by ORGAN_2a. `PawnAuraProfile`'s remaining fields
are the module's own facts, deliberately out of the drivers' room:

| param | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- |
| `influence_radius` | F32 | 20.0 | C2 | Pawn · Aura profile | 0 … 80 `[heuristic]` | ENROLL w2 |
| `attack_stiffness` | F32 | 12.0 | C2 | Pawn · Aura profile | 0 … 48 `[heuristic]` | ENROLL w2 |
| `attack_damping` | F32 | 0.7 | C2 | Pawn · Aura profile | 0 … 2.8 `[heuristic]` | ENROLL w2 |
| `release_rate` | F32 | 1.5 | C2 | Pawn · Aura profile | 0.05 … 8 `[tree]` (matches the ramp's own range, 2a) | ENROLL w2 |
| `tint_strength` | F32 | 0.0 (MUTED, TUNE_1 A4) | C2 | Pawn · Aura profile | 0 … 1 `[identity]` | ENROLL w2 |
| `tint_r/g/b` | VEC3 | 0.4/0.2/0.5 | C2 | Pawn · Aura profile | 0 … 1 `[identity]` | ENROLL w2 |
| `delta_magnitude` | F32 | 0.3 | C2 | Pawn · Aura profile | 0 … 1.2 `[heuristic]` | ENROLL w2 |
| `height_scale` | F32 | 3.0 | C2 | Pawn · Aura profile | 0 … 12 `[heuristic]` | ENROLL w2 |
| `delta_mode` | U32 | CONVERGENT(0) | C2 | Pawn · Aura profile | 0 … 1 `[tree]` (0 convergent, 1 random) | ENROLL w2 |
| `effect_mask` | U32 | 0x3 | C5 | — | — | BOOT — **STATUS: INTENT**, uploaded and never read (TUNE_1 A4 census). A dial on an unread field would lie. |

`tint_strength` is the sharpest instance of the panel's value: the tree
says *"MUTED … 0 silences the terrain tint outright"*. Enrolling it hands
Jean back a whole effect that is currently switched off in source.

## 4.2 bodies/agents.hpp — AGENT_BEHAVIORS

`AGENT_BEHAVIORS[AGENT_BEHAVIOR_COUNT]` is 10 rows × 7 floats, read by
`upload_agent_registries_to_gpu` — **the same author as the tier bank**.
So it rides the *existing* TIER re-speak: a `BEHAVIOR_LIVE` bank beside
`TIER_LIVE`, one flag, one boundary. **C3 idempotent.**

10 rows exceeds D5's ≤8, but D5's cut is about *composite* editing; these
are 7 uniform float columns with a stable row identity, so the honest
shape is per-row enrollment under `Agents · <behavior name>`. Recorded
as an explicit D5 exception with its reason.

| column (×10 rows) | authored span | class | range [ev] | verdict |
| --- | --- | --- | --- | --- |
| `step_rate` | 0 … 0.8 | C3 idem | 0 … 1 `[tree]` (span +25%) | ENROLL w3 |
| `step_size` | 0 … 8.0 | C3 idem | 0 … 10 `[tree]` | ENROLL w3 |
| `persistence` | 0 … 0.85 | C3 idem | 0 … 1 `[identity]` | ENROLL w3 |
| `drag` | 0 … 3.0 | C3 idem | 0 … 3.75 `[tree]` | ENROLL w3 |
| `home_pull` | 0 … 8.0 | C3 idem | 0 … 10 `[tree]` | ENROLL w3 |
| `neighbor_radius` | 0 … 40.0 | C3 idem | 0 … 50 `[tree]` | ENROLL w3 |
| `speed_cap` | 0 … 8.0 | C3 idem | 0 … 10 `[tree]` | ENROLL w3 |

Other agents.hpp constants: `AGENT_EVICTION_RADIUS` (350, **C5** — it
stands under `static_assert(… == Dim::EXIST_RADIUS)`, the VEIL CHAIN
ruling), `AGENT_CENSUS_INTERVAL` (30 s, **C5** — a diagnostic cadence),
`POSSESSION_RADIUS` (20, **C2**, `Interaction · Possession`,
`0 … 80 [heuristic]`, ENROLL w2 — note its `_SQ` twin is derived and must
be recomputed at the read, or the pair drifts; flagged for w2's edit),
`PLAYER_SLOT` (0, **C5** — identity), `AGENT_TIER_NAMES` (**C5**).

## 4.3 bodies/orbs.hpp — the expected first Wave-3 instance

`ORB_MOOD_TABLE[MOOD_COUNT]` is read at `mood.hpp:694` —
`configure_orbs(orbs_state, &orbs_deps, ORB_MOOD_TABLE[mood], queue)` —
inside `apply_mood`. That is the **idempotent** applier the handoff
predicted: re-running it rebuilds the orb population from the config, so
a bank change gets a frame-boundary re-speak, the 2b pattern verbatim.

Bank: `ORB_MOOD_LIVE[MOOD_COUNT]`, kind `ORB_MOOD`, definition-only
entries (there is no instance the panel may address — `configure_orbs`
consumes the config into GPU state). One line per **field**, the target
selecting the mood, exactly as `MoodProfile` works today.

| field | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- |
| `count` | U32 | per mood | C3 idem | Sky & Light · Orbs | 0 … `Dim::MAX_ORBS` `[tree]` (the config's own clamp) | ENROLL w3 |
| `base_hue` | F32 | 0.08 | C3 idem | Sky & Light · Orbs | 0 … 1 `[identity]` | ENROLL w3 |
| `hue_variance` | F32 | 0.05 | C3 idem | Sky & Light · Orbs | 0 … 1 `[identity]` | ENROLL w3 |
| `brightness` | F32 | 0.8 | C3 idem | Sky & Light · Orbs | 0 … 1 `[identity]` | ENROLL w3 |
| `hue_converge_target` | F32 | 0.12 | C3 idem | Sky & Light · Orbs | 0 … 1 `[identity]` | ENROLL w3 |
| `drag` | F32 | 0.5 | C3 idem | Sky & Light · Orbs | 0 … 2 `[heuristic]` | ENROLL w3 |
| `rotation_speed` | F32 | 0.0 rad/s | C3 idem | Sky & Light · Orbs | −2 … 2 `[heuristic]` | ENROLL w3 |
| `rotation_axis[3]` | VEC3 | (0,1,0) | C3 idem | Sky & Light · Orbs | −1 … 1 `[identity]` (normalised in `configure_orbs`) | ENROLL w3 |
| `orbital_base_speed` | F32 | 0.0 rad/s | C3 idem | Sky & Light · Orbs | 0 … 1 `[heuristic]` | ENROLL w3 |
| `flock_sep_radius` | F32 | 50 | C3 idem | Sky & Light · Orb flock | 0 … 200 `[heuristic]` | ENROLL w3 |
| `flock_align_radius` | F32 | 120 | C3 idem | Sky & Light · Orb flock | 0 … 480 `[heuristic]` | ENROLL w3 |
| `flock_coh_radius` | F32 | 200 | C3 idem | Sky & Light · Orb flock | 0 … 800 `[heuristic]` | ENROLL w3 |
| `flock_sep_weight` | F32 | 30 | C3 idem | Sky & Light · Orb flock | 0 … 120 `[heuristic]` | ENROLL w3 |
| `flock_align_weight` | F32 | 8 | C3 idem | Sky & Light · Orb flock | 0 … 32 `[heuristic]` | ENROLL w3 |
| `flock_coh_weight` | F32 | 15 | C3 idem | Sky & Light · Orb flock | 0 … 60 `[heuristic]` | ENROLL w3 |
| `flock_max_speed` | F32 | 60 | C3 idem | Sky & Light · Orb flock | 0 … 240 `[heuristic]` | ENROLL w3 |
| `rule_drag_brownian/orbital/frozen/flocking` | F32 ×4 | 0.0 (pass-through) | C3 idem | Sky & Light · Orb flock | 0 … 4 `[tree]` (0 = pass-through 1.0×) | ENROLL w3 |
| `motion_rule` | U32 | 0..3 | C3 idem | Sky & Light · Orbs | 0 … 3 `[tree]` (Brownian/Orbital/Frozen/Flocking) | ENROLL w3 |
| `palette_id` | U32 | 0..3 | C3 idem | Sky & Light · Orbs | 0 … 3 `[tree]` (`ORB_PAL_COUNT`) | ENROLL w3 |
| `tierset_id` | U32 | `ORB_TIERSET_NONE` | C3 idem | Sky & Light · Orbs | — | **DEFER** — its "none" value is `0xFFFFFFFF`; a 0…1 slider cannot express a sentinel without lying (D1(d)) |
| `flock_gesture_default` | U32 | 0 | C3 idem | Sky & Light · Orbs | 0 … 7 `[tree]` (`ORB_FLOCK_GESTURE_COUNT`) | ENROLL w3 |
| `enabled` | **C++ `bool`** | per mood | C3 idem | Sky & Light · Orbs | 0 … 1 | ENROLL w3 **via D2** — CPU-only struct being graduated, so the bank's field is `uint32_t` from birth (the aura-intent precedent) |

Module console: `ORB_DOME_RADIUS` (500, **C2**, and its comment says
*"Jean's dial"* — `Sky & Light · Dome`, `0 … 2000 [heuristic]`, ENROLL
w2), `ORB_BASE_SIZE` (3.0, **C2**, `0 … 12 [heuristic]`, ENROLL w2),
`ORB_NOISE_FLOOR` (0.3, **C2**, `0 … 1 [identity]`, ENROLL w2). The nine
`ORB_DEFAULT_*` are *fallbacks the mood config already overrides* —
**C5**, one reason: a dial on a default that every live path replaces
would never be seen to move.

Registries: `ORB_PALETTES` (4 palettes × 4 entries, mixed shapes),
`ORB_TIERSETS`, and the three gesture registries (8 flock / 6 brownian /
4 orbital) — **C5** by D5, with the composite-editor price named: a
palette editor needs 4 hue/value/weight triples per palette, and the
gesture registries need a per-gesture parameter block. Pricing, not
promising.

## 4.4 bodies/ribbon.hpp

**The four pipes are already wired C4 seams** — `ribbon.hpp:833-846`
reads them per frame with hard-coded fallbacks that *are* the rests:

```cpp
const float ml  = c->ribbon_amp_lat_dst_.valid  ? vp.get(...)  : 1.0f;
const float mv  = c->ribbon_amp_vert_dst_.valid ? vp.get(...)  : 1.0f;
const float mix = c->ribbon_tint_mix_dst_.valid ? vp.get(...)  : 0.0f;
const float* st = c->ribbon_tint_stim_dst_.valid ? vp.run(...) : nullptr;
```

Wave 4's recipe applies verbatim: those four literals become
`DriverSurface::Ribbon` rests, each gains a gain, and the driven values
witness. Ranges from `PARAM_LAYOUT`'s own rest column (`amp_*` rest 1.0
= identity, `color_stim` 0, `color_mix` 0).

**Head control law** — the file's own words: *"All control-panel
material."* Read per frame in `ribbon_advance_head` from constexpr, so
**C2** with a `RIBBON_LIVE` bank:

| param | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- |
| `RIBBON_YAW_RATE` | 1.0 rad/s | C2 | Ribbon · Head | 0 … 4 `[heuristic]` | ENROLL w2 |
| `RIBBON_MAX_SPEED` | 40.0 | C2 | Ribbon · Head | 0 … 160 `[heuristic]` | ENROLL w2 |
| `RIBBON_R_MIN` | 40.0 | C2 | Ribbon · Head | 1 … 160 `[heuristic]` | ENROLL w2 |
| `RIBBON_CLIMB_RATE` | 15.0 | C2 | Ribbon · Head | 0 … 60 `[heuristic]` | ENROLL w2 |
| `RIBBON_FLOOR_MARGIN` | 25.0 | C2 | Ribbon · Head | 0 … 100 `[heuristic]` | ENROLL w2 |
| `RIBBON_ALT_SMOOTH_DIST` | 180.0 | C2 | Ribbon · Head | 0 … 720 `[heuristic]` | ENROLL w2 |
| `RIBBON_ALT_STIFF` | 0.36 | C2 | Ribbon · Head | 0 … 1.44 `[heuristic]` | ENROLL w2 |
| `RIBBON_MOUNT_SETBACK` | 1.5 | C2 | Ribbon · Head | 0 … 6 `[heuristic]` | ENROLL w2 |
| `RIBBON_SKY_YAW_TAU` | 0.6 s | C2 | Ribbon · Head | 0 … 2.4 `[heuristic]` | ENROLL w2 |
| `RIBBON_REFERENCE_BPM` | 100.0 | C2 | Ribbon · Head | 40 … 240 `[heuristic]` | ENROLL w2 |
| `WANDER_STEER_SOFT` | 0.5 rad | C2 | Ribbon · Wander | 0 … 2 `[heuristic]` | ENROLL w2 |
| `WANDER_YAW_MAX` | 0.15 | C2 | Ribbon · Wander | 0 … 0.6 `[heuristic]` | ENROLL w2 |
| `WANDER_YAW_TAU` | 2.0 s | C2 | Ribbon · Wander | 0 … 8 `[heuristic]` | ENROLL w2 |
| `WANDER_ARRIVE_RADIUS` | 120.0 | C2 | Ribbon · Wander | 0 … 480 `[heuristic]` | ENROLL w2 |

`RIBBON_YAW_RATE` and `RIBBON_R_MIN` are coupled by the file's own steering
law — *"available yaw rate is min(RIBBON_YAW_RATE, speed / RIBBON_R_MIN)"*
— which is a `min`, not an assert, so both dials stay honest at every value.
`RIBBON_R_MIN`'s min is **1, not 0**: zero would divide the turn radius to
nothing. That floor is the range column earning its keep a second time.

**Spawn-authored — C3 destructive**, banner *edits the next spawn*:
`RibbonConfig::SPAWN_CHANCE` (0.900, `0 … 1 [identity]`),
`RibbonConfig::POSITION_JITTER` (0.3), `WANDER_CHANCE` (0.30),
`WANDER_CRUISE_BASE/SIGMA/MIN/MAX`, `WANDER_LEG_MIN/MAX`,
`WANDER_SPREAD`, `WANDER_RETARGET_MIN/VAR`, `WANDER_HATCH_LEG`,
`RibbonColorMode::WEIGHTS[3]`, `RIBBON_SMOOTH_PALETTE[4][3]`,
`SMOOTH_VAR_RANGE/BIAS/G_SCALE/B_SCALE`, `TINTED_RANGE[3]`,
`TINTED_BASE[3]` — all ENROLL w3 against a `RIBBON_SPAWN_LIVE` bank.

**C5**: `MOUNT_TANGENT_ALIGN` / `MOUNT_BANK_GAIN` / `MOUNT_BANK_MAX` are
**LOCKSTEP MIRRORS of world.wgsl** — the L3 hazard the panel exists to
avoid; never a dial while the mirror is hand-kept.
`MIN_CUBE_COUNT/SIZE`, `MIN_ADDED_HEIGHT`, `FOOTPRINT_RADIUS`,
`ORIENTATION_SPREAD`, the `CheckerPair` tables and
`RIBBON_SMOOTH_PALETTE_COUNT` — spawn geometry and counts, BOOT.

## 4.5 bodies/pawn_figures.hpp

`PAWN_FIGURES` — the figure table, read per frame for `tilt_tau`,
`height` (→ `fpv_eye_height`) and body radius. Those three config fields
are **C4 witnesses** (§6). The table itself is mixed-shape per-figure
geometry: **C5**, D5, with the composite price named — a figure editor
needs height/radius/tilt/share per row.

## 4.6 FLAGGED — surveyed by sweep, not read whole

Per FLAG-AND-FINISH, these five were swept (constant census, author
grep) but **not read whole**, so nothing from them is enrolled and their
rows are not claimed. What the sweep found, and what a successor needs:

| module | constants | what the sweep says | why deferred |
| --- | --- | --- | --- |
| `bodies/grounded.hpp` | 240 | dominated by per-family placement geometry, column/antenna/arch build laws, `COLUMN_PALETTE` | the largest single console in the tree; a full read is its own sitting. Expect mostly C3-destructive (spawn) + C5 (build geometry) |
| `bodies/gallery.hpp` | 92 | painting slots, exhibition layers, snapshot cadence, wall-frame budgets | intertwined with `Dim::EXHIBITION_LAYERS` and the SALON open items in `docs/OPEN.md`; enrolling before that register clears would cross a live deferral |
| `bodies/cube_behaviors.hpp` | 65 | floater behaviour laws; `stage_floater_coordination` is its one live write | `floater_coordination` itself is surveyed in §6 as C4; the behaviour constants need the module read whole first |
| `bodies/gol_zones.hpp` | 47 | zone birth/derive params; the GoL keeps its own panel by `terrain_looks` ROW 9 | jurisdiction: ROW 9 points at it as a separate panel, so it wants its own sitting |
| `bodies/spheres.hpp` | 14 | sphere spawn + cap | small; deferred only because its family (`floaters.hpp` weights) is already w3 and should land together |

This is the campaign's honest edge: **four modules read whole and
enrolled, five surveyed and flagged.** The flag is the deliverable for
those five — the next sitting starts here, not from zero.

---

# 5 · `coupling/visual_canvas.hpp`

The canvas is where couplings already live, so this file is mostly **C4
seam material** and the tables its couplings read.

## PARAM_LAYOUT — the pipes

Slot numbers and widths are **C5 always** (§0: "`PARAM_LAYOUT` slot
numbers are C5"). The **rest** column is C4 seam material — three of the
eight pipes are already dispositioned:

| pipe | rest | status |
| --- | --- | --- |
| `fog.density` / `fog.color` | `FOG_DENSITY_NONE` / placeholder | **DONE** — the drivers' room, ORGAN_2a |
| `ribbon.amp_lateral_mult` | 1.0 | C4 — Wave 4, `DriverSurface::Ribbon` |
| `ribbon.amp_vertical_mult` | 1.0 | C4 — Wave 4 |
| `ribbon.color_stim` (3) | 0.0 | C4 — Wave 4 |
| `ribbon.color_mix` | 0.0 | C4 — Wave 4 |
| `terrain.checker_mean` (3) | 0.0 | C4 — Wave 4, the U4 flush |
| `terrain.checker_var` (2) | 0.0 | C4 — Wave 4, the U4 flush |

## The coupling tables and envelopes

| param | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- |
| `FOG_BY_FIELD[7]` / `FOG_COLOR_BY_FIELD[7][3]` | per-field density/tint | C5 | — | — | BOOT — the coupling's authored compile-time panel, ruled out of scope at ORGAN_2a and unchanged |
| `FOG_SPAN` | 2.0 beats | C2 | Atmosphere · Fog | 0 … 8 `[heuristic]` | ENROLL w2 |
| `RIBBON_SWELL_CEILING` | 2.00 × idle (**ruled**) | C2 | Ribbon · Swell | 1 … 4 `[heuristic]` | ENROLL w2 |
| `RIBBON_SWELL_RAMP` | 8.0 beats (**ruled**) | C2 | Ribbon · Swell | 0 … 32 `[heuristic]` | ENROLL w2 |
| `RIBBON_SWELL_ATTACK` / `_RELEASE` | 0.35 / 2.0 beats | C2 | Ribbon · Swell | 0.01 … 8 `[heuristic]` | ENROLL w2 |
| `TINT_LUMA` / `TINT_CHROMA` / `TINT_MIX_MAX` | 0.55 / 0.35 / 0.85 | C2 | Ribbon · Tint | 0 … 1 `[identity]` | ENROLL w2 |
| `TINT_MIX_ATTACK` / `_RELEASE` / `_HUE_SPAN` | 0.5 / 3.0 / 2.0 beats | C2 | Ribbon · Tint | 0.01 … 12 `[heuristic]` | ENROLL w2 |
| `PITCH_VEC_ORIGIN` | 0.0 rad — "rotates the hue seating" | C2 | Ribbon · Tint | 0 … 6.2832 `[identity]` (a full turn) | ENROLL w2 |
| `CHECKER_READ_SPAN` / `_ATTACK` / `_RELEASE` | 4 / 2 / 8 beats | C2 | Atmosphere · Checker | 0.25 … 32 `[heuristic]` | ENROLL w2 |
| `TIDE_SHIFT_MIN/MAX`, `RAIN_SCATTER_MIN/MAX` | −0.65…0.75, −0.80…0.25 | C2 | Terrain · Modes | −1 … 1 `[tree]` (the file names both ends) | ENROLL w2 |
| `TINT_D1[3]` / `TINT_D2[3]` | orthonormal chroma basis | C5 | — | — | BOOT — a basis, not a dial; editing one lane un-orthogonalises the pair |
| `PC_COLOR[12][3]` | Jean's twelve hues | C5 | — | — | BOOT by D5 (12 rows > 8) — **but see the note below** |
| `ZOETROPE_EARS`, `ZOETROPE_ROW_OF_PC[12]`, `RIBBON_VOICE`, `CHECKER_VOICE` | bitmask / index map / voice names | C5 | — | — | BOOT — addresses and identity, never dials |

**`PC_COLOR` is the largest near-miss in the campaign.** Twelve rows of
three lanes, explicitly *"PC_COLOR IS JEAN'S — twelve hues, one per pitch
class. Tune it here."* — an invitation to a panel if ever the tree wrote
one. D5 puts it over the line at 12 > 8, and it is the honest place to
hold: 36 lanes is a colour *editor*, not a dial strip. What a composite
editor would need: twelve swatches keyed by pitch-class name, with the
dressed-pc offset (`0 = D`) shown, since the row order is not chromatic
from C. Pricing, not promising.

---

# 6 · `realization/state.hpp` — GPUDesignConfig, field by field

The single richest vein, and now fully censused: every field, every
setter, every setter's call sites and cadence. The verdicts below rest on
that census, not on the field comments.

## C1 — no runtime author (boot pins only)

Already covered above: the eight `field_*`, `point_bubble_radius`, the
palette mirror (12 entries), the ROW 2 motion/mode pins (14 entries),
`point_fly_speed`. Additional C1 found only here:

| field | shape | authored | class | section·group | range [ev] | verdict |
| --- | --- | --- | --- | --- | --- | --- |
| `pawn_speed` | F32 | `Idle::PAWN_SPEED` | C1 — no writer at all | Interaction · Pawn | 0 … 4× rest `[heuristic]` | ENROLL w1 |
| `freeze_sphere` | U32 | 0 | C1 | Debug | 0 … 1 `[tree]` BOOL | ENROLL w1 |
| `cube_plasticity` | F32 | **1.0** (`Idle::CUBE_PLASTICITY_DEFAULT`, raised 0.6→1.0 at CONTACT_5 P2b) | C1 | Interaction · Cubes | 0 … 1 `[identity]` (a λ master) | ENROLL w1 — **adjudicated at P5: the comment was stale**, and in three places, not one |
| `veil_ring` | F32 | `Dim::VEIL_RING_DEFAULT` (325) | C1 — "Live-tunable" says the tree | Atmosphere · Veil | **265 … 349** `[tree]` — see THE VEIL CHAIN below | ENROLL w1 |
| `veil_icing` | F32 | `Dim::VEIL_ICING_DEFAULT` (40) | C1 | Atmosphere · Veil | **0 … 60** `[tree]` | ENROLL w1 |
| `lod0_radius` | F32 | `Dim::LOD0_RADIUS_DEFAULT` (175) | C1 | Atmosphere · Veil | **0 … 175** `[tree]` | ENROLL w1 |
| `veil_dither` | F32 | 0.0 | C1 key-shared | Atmosphere · Veil | 0 … 1 `[tree]` (">0.5 = dither") | ENROLL w1 |
| `mosaic_enable` | F32 | 1.0 | C1 | Terrain · Mosaic | 0 … 1 `[tree]` (a gate) | ENROLL w1 |
| `mosaic_shard_size` | F32 | `Dim::MOSAIC_SHARD_SIZE_DEFAULT` | C1 | Terrain · Mosaic | 0 … 4× rest `[heuristic]` | ENROLL w1 |
| `mosaic_passage_scale` | F32 | `Dim::MOSAIC_PASSAGE_DEFAULT` | C1 | Terrain · Mosaic | 0 … 4× rest `[heuristic]` | ENROLL w1 |
| `mosaic_blend` | F32 | `Dim::MOSAIC_BLEND_DEFAULT` | C1 | Terrain · Mosaic | 0 … 1 `[identity]` | ENROLL w1 |
| `mosaic_facet` | F32 | `Dim::MOSAIC_FACET_DEFAULT` | C1 | Terrain · Mosaic | 0 … 4× rest `[heuristic]` | ENROLL w1 |
| `mute_dynamics_0d` | U32 | 0 | C1 (D2 — a config toggle word enrolls BOOL directly) | Debug | 0 … 1 | ENROLL w1 |
| `mute_signal` | U32 | 0 | C1 | Debug | 0 … 1 | ENROLL w1 |
| `mute_couplings` | U32 | `Coupling::NONE` | C1 — a **bitmask** | Debug | — | **DEFER-RANGE (D1d)** — `Coupling::ALL` is `0x1FFFFF`; a slider from 0 to 2 097 151 is not a dial. It wants a checkbox per bit, which is a shell feature, not an enrollment line. |
| `fpv_mode` | U32 | 0 | C1 key-shared | Debug | 0 … 1 | ENROLL w1 |

`mute_signal` and `mute_couplings` have one *program* writer pair —
`enter_design_mode` / `enter_performance_mode` — with **zero external
callers**. They are mode presets nothing invokes today, so the fields are
C1 in practice. Recorded so a future sitting that wires those presets
knows it is creating a second author.

## C4 — a per-frame author writes it (witnesses, Wave 4)

| field | author | cadence | verdict |
| --- | --- | --- | --- |
| `fog_density`, `fog_color` | `phase_motion_drivers` → `set_fog` | per frame | **DONE** — ORGAN_2a witnesses |
| `aura_enabled`, `pawn_aura_height` | `tick_pawn_couplings` | per frame | **DONE** — ORGAN_2a witnesses |
| `checker_resultant`, `checker_music_amount`, `checker_music_variance` | `phase_motion_drivers` → `set_checker_color_field` (cartridge.hpp:1080) | per frame | ENROLL w4 — rests+gains in the drivers' room, three witnesses |
| `pawn_tilt_tau` | cartridge.hpp:982, from the possessed figure | per frame | ENROLL w4 — witness only (the *dial* is `PAWN_FIGURES.tilt_tau`, D5-table) |
| `pawn_body_radius` | cartridge.hpp:997 | per frame | ENROLL w4 — witness only |
| `fpv_eye_height` | cartridge.hpp:1015, `FPV_EYE_RATIO × figure height` | per frame | ENROLL w4 — witness only |
| `fade_alpha`, `fade_color` | cartridge.hpp:1332, the transition | per frame | ENROLL w4 — witnesses. **Never dials**: ORGAN_0's own enrollment banner already ruled a dial here "would fight an author and lose confusingly" |
| `floater_coordination` | `cube_behaviors.hpp:400` | per frame | ENROLL w4 — witness; the *dial* is in the flagged `cube_behaviors` module |
| `terrain_amp_ceiling`, `ceiling_height`, `indoor_height_cap` | `apply_mood_lighting` | per mood | ENROLL w4 — witnesses (their definitions are MoodProfile-structural, C5) |
| `veil_strength` | cartridge.hpp:1165, world init | per world | ENROLL w4 — witness |

## C5 — structural

`world_seed` (world generation reads it everywhere), `world_bound_min/max`
(set at world init from the mood's radii), `placement_patch_count`,
`lod_point_x/z` (the CPU's banding point — one yardstick with the GPU
gate by construction; a dial would split it), `possessed_slot` (a host
pointer), `pulse_count` + `pulse_data[32]` (a ring buffer), `sun_direction`
(ORGAN_2c: a window, ruled), and every `_pad*`.

---

# 7 · `world.wgsl` — the TUNING SURFACE DIRECTORY

**C5 BOOT wholesale**, by the standing law the file states itself:

> *Purely visual constants prefer WGSL-side residence for the
> edit-save-look loop: a number that only changes what the eye sees wants
> the shortest path from edit to sight, and that path is here.*

and by `terrain_looks.hpp`'s two-rooms rule — *every VALUE lives in
exactly ONE room* — which is what makes the L3 mirror hazard impossible
for these. One ledger section, no per-const rows, per §2's instruction.

The directory's own chapters: Spatial Field Lattices (10 consts), Palette
Composition, Composite Cuts & Edges, the Movement Third (`TERRAIN_BANDS`,
`WAVE_THRESHOLD`), the checker dials (`CHECKER_WANDER`,
`CHECKER_VAR_PER_NOTE`, `CHECKER_VAR_MAX`, `DEBUG_VIEW`), GoL internals,
and the entity behaviour consts.

**One flag, per §2's "unless a const is plainly a live candidate wrongly
stranded":** the ROW 5 checker dials (`CHECKER_WANDER`,
`CHECKER_VAR_PER_NOTE`, `CHECKER_VAR_MAX`) sit *downstream of a live
config wire* — `terrain_looks` ROW 2 routes the checker field through
`config.checker_*` while these three stay WGSL-resident. That is not
wrong (they are the GPU's own consumption shape, not the coupling's
output), but it is the one place a future sitting might reasonably ask
for a config word. Recorded, not proposed.


---

# THE VEIL CHAIN — a runtime hazard the ranges carry

`state.hpp` proves the veil's ordering at compile time:

```cpp
static_assert(EXIST_RADIUS > VEIL_RING_DEFAULT,          "EXIST > RING");
static_assert(VEIL_RING_DEFAULT > LOD0_RADIUS_DEFAULT,   "RING > LOD0");
static_assert(VEIL_RING_DEFAULT - VEIL_ICING_DEFAULT > LOD0_RADIUS_DEFAULT,
              "the icing band sits wholly outside the LOD0 core");
```

Three of Wave 1's dials are exactly those constants' live homes, and a
static_assert cannot guard a runtime dial — the same hazard the beacon's
assert has, met a second time. Independent clamps cannot enforce a joint
invariant, so the boxes are cut **so that every combination inside them
satisfies the chain**:

| dial | box | worst case |
| --- | --- | --- |
| `veil_ring` | 265 … 349 | max 349 < 350 = `EXIST_RADIUS` ✓ |
| `veil_icing` | 0 … 60 | min ring − max icing = 265 − 60 = 205 |
| `lod0_radius` | 0 … 175 | 205 > 175 = max lod0 ✓ ; and 265 > 175 ✓ |

Widening any one of the three requires re-proving the chain against the
other two. That sentence is in the `.inc` beside the lines, because the
next person to widen a range will read the `.inc`, not this file.

**Two asserts, two answers.** The beacon's (`FIELD_BEACON_S < FIELD_K`)
could not be carried by ranges — `S` had no live home and giving it one
would put both sides of the proof on sliders — so it was DEFERRED, and
ORGAN_3b P5 lifted it with a clamp at the writer instead. The
veil's could, because two of its three bounds are fixed constants
(`EXIST_RADIUS`) and the third is a dial whose box can be cut clear.
Same hazard, different disposition, and the difference is stated rather
than papered over.

---

# WAVE 2 — THE GRADUATIONS, AS BUILT

Four banks, four block ids. **Block budget: 8 of the 12 the handoff set
as the STOP threshold** (0 config, 1 lighting, 2 agent room, 3 drivers,
4 pawn, 5 orbs, 6 panel, 7 ribbon). Wave 3's C3 banks cost **no** block
ids — a definition-only entry lives in the sentinel block, which is the
lesson ORGAN_2b already paid for.

| block | bank | file | dials | design table |
| --- | --- | --- | --- | --- |
| 4 | `PAWN_AURA_LIVE` | `contracts/pawn_surface.hpp` | 9 | `PAWN_AURA_DEFAULT` |
| 5 | `ORB_CONSOLE_LIVE` | `contracts/orb_surface.hpp` | 3 | `ORB_CONSOLE` |
| 6 | `PANEL_LIVE` | `contracts/control_panel.hpp` | 7 | `PANEL_TABLE` |
| 7 | `RIBBON_LIVE` | `contracts/ribbon_surface.hpp` | 14 | `RIBBON_TABLE` |

Each bank seeds byte-for-byte from its design table (harness `memcmp`),
each is reachable through `block_base`, and each rides the CPU-bank flush
idiom — no upload, no dirty flag, the module's own per-tick read is the
flush. `organ_flush` counts them in one arm because they share one
reason; a bank that ever needs an upload earns its own arm and its own
sentence.

## Two dead facts found and retired

- **`PawnState::active_aura_profile`** was described as *"swappable by
  landmarks/commands"* and had **no writer anywhere in the tree** — a
  copy of a constant nothing changed. A panel edit to the profile could
  never have reached it. The two readers now read `PAWN_AURA_LIVE` and
  the field is gone, the same disposition `PawnState::aura_enabled` took
  at ORGAN_2a.
- **`CameraControls`' four control constants** had no transport at all;
  `control_panel.hpp`'s own banner predicted this sitting — *"they join
  when the panel proper is designed"* — and ORGAN_3 is the panel proper.

## DEFERRED from Wave 2, with the reason

**`FIELD_BEACON_S`'s row died at ORGAN_3b P5** — the clamp at the writer,
adjudicated by Jean. What remains:

| what | why |
| --- | --- |
| `POSSESSION_RADIUS` | its `_SQ` twin is a derived constant; enrolling the radius without recomputing the square at the read would let the two disagree silently. Small, real, and wants the read site changed first. |

---

# WAVE 3 — THE DEFINITIONS, AS BUILT

211 entries. One new kind, one new block, and **the temperament law is
now code, not prose.**

| what | kind | machinery it cost | dials |
| --- | --- | --- | --- |
| the tier bank's remaining six columns × 4 tiers | `TIER` (existing) | **none** — it rides the boundary ORGAN_2b already built | 24 |
| `AGENT_BEHAVIORS` → `BEHAVIOR_LIVE` | `BEHAVIOR` (new) | one enum value + one `definition_base` case. **No new flag and no new boundary**: it shares TIER's, because it shares TIER's author | 70 |
| `INDOOR_LIVE` (block 8) | — (a plain bank) | one block id. **No boundary at all, by law** | 2 |

## A kind is not a flag

`upload_agent_registries_to_gpu` reads *both* the tier table and the
behaviour table in one function, and the frame boundary already
re-speaks it. So `ORGAN_DEF_BEHAVIOR` raises `g_tier_def_dirty` — one
flag, one boundary, two banks. The flag names the **occasion**, and the
occasion is one author speaking; a second flag would have been a second
name for one event. Harness: *two banks, two edits, ONE boundary
re-speak.*

## The temperament law, in code

`INDOOR_LIVE` is the exemplar. `INDOOR_HEIGHT_CAP_FRACTION` has ten
readers — one idempotent (`apply_mood_lighting`) and nine destructive
(`cap_to_ceiling` at every entity spawn: entity_pipeline ×3, grounded
×3, spheres, cube_behaviors, ribbon). **The stricter temperament
governs**, so the bank has no boundary wiring whatever: the mood half
picks the edit up at the next mood change, the spawn half at the next
spawn, and neither needs a flag. The group is named *"Terrain · Indoor
(edits the next spawn)"* because a dial that edits the future must say
so — otherwise the operator reads a working panel as a broken one.

Harness: *destructive bank: value lands (0.42), NO re-speak flag raised.*

## THE CATCHABILITY LAW now holds at runtime

`flee_gain_player` enrolls with max `0.99` from CONTACT_4's own words.
The harness pushes it to 1.5 and watches `organ_set`'s clamp bring it to
0.99. Before this campaign the law was enforced by a comment asking
authors to "keep every row < 1.0"; it is now enforced by the panel that
could otherwise have broken it.

## A bug the harness caught

The shell keyed exports by `p.def === DEF_TIER ? 'world' : mood`. Kind
`BEHAVIOR` is 3, so its 70 definitions exported as `0/AGENT_ROOM.…` —
claiming a mood ownership they do not have. The C++ ignores the target
for world kinds, so it would have *round-tripped* correctly and lied in
the file. Fixed by asking the question once — `isWorldDef(p)` — so a
fourth family answers it by being added in one place rather than being
forgotten at three.

## Wave 3's DEFERRALS

**One row died here.** `ORB_MOOD_TABLE` → `ORB_MOOD_LIVE` was deferred
because the boundary was a guess; ORGAN_3b P3 read the module and landed
it. What remains:

| what | why |
| --- | --- |
| the remaining destructive banks — `PORTAL_DENSITY`, `FINITE_OUTDOOR_CHANCE`, `SCHEME_WEIGHTS`, `PORTAL_COLORS`, ribbon's spawn rolls and colour vocabulary, the floater tier weights | Each is a small bank with a banner and no wiring — mechanical, and the pattern is now proven by `INDOOR_LIVE`. Deferred as bulk, not as difficulty. |

---

# WAVE 4 — THE SEAMS, AS BUILT

**223 entries, 13 witnesses.** The drivers' room grew by one seam and by
five meters that will never be dials.

## The checker field's seam — the 2a recipe verbatim

`DriverSurface` gains `Checker { rest_resultant[3], rest_amount,
rest_variance, gain }`; `phase_motion_drivers` blends
`rest + gain·(driven − rest)` per lane and the three driven values enroll
as witnesses. The room is 36 → 60 bytes.

The rests are **not taste**. `terrain_looks` ROW 2 calls them law:
*"RESTS are law: amount 0 (the GPU maps that to each cell's seed color)
and variance 0 — a return to seed, not gray."* Dial the gain to 0 and the
terrain returns to its seed colours, which is the authored silence, not
an absence.

The seam grew a **headless arm**, as fog's did: with no bindings the rest
alone speaks, so the dials reach the picture with the music silent.
`set_checker_color_field` guards, so that arm costs no dirty (D2).

**Equivalence proved, and more cheaply than fog's.** With every rest at
0, `0 + 1·(d − 0) == d` is exact for *every* float — no Sterbenz argument
is needed, unlike the fog seam, whose rests are non-zero. The harness
checks 8 M samples at the authored rests (0 differences), confirms gain 0
returns the rest exactly, and re-checks the Sterbenz window for a
curator-moved rest of 0.5 (0 differences).

## Five meters that are witnesses on purpose

| witness | why there is no dial | 
| --- | --- |
| `fade_alpha`, `fade_color` | ORGAN_0's own enrollment banner already ruled it: *"a dial there would fight an author and lose confusingly."* The transition system authors them per frame. |
| `pawn_tilt_tau`, `pawn_body_radius`, `fpv_eye_height` | re-derived every frame from the **possessed figure's** row in `PAWN_FIGURES`. The dial is that table — a D5 composite, priced in §4.5 — not a slider on the output. Metering them keeps the contest column truthful about who is writing. |

Enrolling a witness where a dial would lie is the panel telling the truth
about its own limits, which was ORGAN_2a's whole argument and is now
applied without being asked twice.

## Wave 4's DEFERRALS

| seam | why |
| --- | --- |
| the ribbon's four pipes (`ribbon.amp_lateral_mult`, `amp_vertical_mult`, `color_stim`, `color_mix`) | The rests are already at the seam as hard-coded fallbacks (`: 1.0f`, `: 0.0f`, `nullptr`) inside `ribbon.hpp:833-846` — but the **`color_stim` fallback is a null pointer**, not a value, and the code branches on it downstream. Moving it into the room means giving the null branch a rest *shape* first, which is a read of the tint path this sitting did not make. The other three are mechanical; they were held back with the fourth so the ribbon's seam lands as one coherent unit rather than three-quarters of one. Anatomy: `DriverSurface::Ribbon { rest_amp_lat, rest_amp_vert, rest_tint_stim[3], rest_tint_mix, gain }`. |
| `floater_coordination` | driven by `cube_behaviors.hpp:400`, which is one of the five modules flagged unread in §4.6. Witness-only would be honest, but its *dial* lives in that module and the pair should land together. |
| `veil_strength`, `terrain_amp_ceiling`, `ceiling_height`, `indoor_height_cap` | driven per-world/per-mood from MoodProfile's **structural** group, which is C5 by the standing eligibility rule. A witness on each is defensible and cheap; deferred only because the four belong to one reading of the mood applier that Wave 3's ORB deferral already named. |

---

# THE GAP AT CLOSE

`tools/organ_gap.py`, run against the tree ORGAN_3b leaves. Every
absent member below has a reason in the sections above — the tool
finds the gap, the ledger explains it, and neither pretends to do the
other's job.

```
ORGAN GAP — members of the enrolled homes that the panel does not name
========================================================================
A map, not a gate. Reasons live in docs/HANDOFFS/DISPOSITION_LEDGER.md.

THE FILE TABLE this run trusted (blind spot 2 — stale rows are
invisible bugs, so they are printed):
    AgentBehaviorBank        src/cartridges/the_board/contracts/agent_tiers.hpp
    AgentTierBank            src/cartridges/the_board/contracts/agent_tiers.hpp
    CanvasSurface            src/coupling/canvas_surface.hpp
    DriverSurface            src/cartridges/the_board/contracts/driver_surface.hpp
    GPUAgentRoomConstants    src/cartridges/the_board/realization/state.hpp
    GPUDesignConfig          src/cartridges/the_board/realization/state.hpp
    GPULighting              src/cartridges/the_board/realization/state.hpp
    IndoorSurface            src/cartridges/the_board/contracts/indoor_module.hpp
    MoodProfile              src/cartridges/the_board/contracts/spine_state.hpp
    OrbConsole               src/cartridges/the_board/contracts/orb_surface.hpp
    OrbMoodConfig            src/cartridges/the_board/contracts/orb_surface.hpp
    PanelSurface             src/cartridges/the_board/contracts/control_panel.hpp
    PawnAuraProfile          src/cartridges/the_board/contracts/pawn_surface.hpp
    RibbonSurface            src/cartridges/the_board/contracts/ribbon_surface.hpp

AgentBehaviorBank          1/ 1 named   0 absent
AgentTierBank              1/ 1 named   0 absent
CanvasSurface             15/15 named   0 absent
DriverSurface              3/ 3 named   0 absent
GPUAgentRoomConstants      2/ 5 named   3 absent
        portals
        occupier_cmg
        occupier_amg
GPUDesignConfig           60/77 named   17 absent
        mute_couplings
        world_seed
        sun_direction
        world_bound_min
        world_bound_max
        placement_patch_count
        terrain_amp_ceiling
        ceiling_height
        floater_coordination
        pulse_count
        possessed_slot
        indoor_height_cap
        pulse_data
        lod_point_x
        lod_point_z
        point_host
        veil_strength
GPULighting                1/ 3 named   2 absent
        points
        spots
IndoorSurface              2/ 2 named   0 absent
MoodProfile                5/15 named   10 absent
        finite
        finite_radius_min
        finite_radius_max
        indoor
        ceiling_type
        wall_height
        terrain_amp_ceiling
        allow_gol_zones
        allow_pawn_aura
        allow_frustum_cull
OrbConsole                 3/ 3 named   0 absent
OrbMoodConfig             23/24 named   1 absent
        tierset_id
PanelSurface               2/ 2 named   0 absent
PawnAuraProfile            8/ 9 named   1 absent
        effect_mask
RibbonSurface             14/14 named   0 absent

TOTAL ABSENT FROM THE PANEL, ACROSS THE ENROLLED HOMES: 34

Blind spot 1: homeless constants — an authored constexpr with no
live home — cannot appear above. This tool measures the gap between
the HOMES and the panel; the ledger measures the gap between the
PROGRAM and the panel, which is larger.
Blind spot 3: a partly-enrolled nested aggregate reads as named —
`fog.gain` names `fog`. The ledger carries the per-field truth.
```

## Reading the gap

Not one of the 33 is an oversight. Grouped by the reason already given:

| absent | count | reason |
| --- | --- | --- |
| `MoodProfile`'s structural group | 10 | C5 by the standing eligibility rule beside `MOOD_LIVE` — world generation reads it |
| `GPUDesignConfig` structural (`world_seed`, bounds, `placement_patch_count`, `lod_point_*`, `possessed_slot`, `pulse_*`, `point_host`) | 10 | C5 — seeds, GPU yardsticks, host pointers, a ring buffer |
| `GPUDesignConfig` driven, witness deferred (`terrain_amp_ceiling`, `ceiling_height`, `indoor_height_cap`, `veil_strength`, `floater_coordination`) | 5 | Wave 4's named deferrals |
| `GPUAgentRoomConstants` windows (`portals`, `occupier_cmg`, `occupier_amg`) | 3 | C5 — windows onto other homes (CHORD's windows-not-homes ruling) |
| `GPULighting` (`points`, `spots`) | 2 | C5 — light arrays, D5 composite |
| `config.sun_direction` | 1 | ORGAN_2c ruled it a window, not a home |
| `config.mute_couplings` | 1 | DEFER-RANGE (D1d) — a bitmask wants checkboxes, not a slider |
| `PawnAuraProfile::effect_mask` | 1 | C5 — STATUS INTENT, uploaded and never read |

The number to carry forward is not 33. It is **blind spot 1**: the
constants with no home at all, which this tool cannot see and the ledger
counts in its C2 and C3-destructive rows — the ribbon's spawn vocabulary,
the orb registries, `PC_COLOR`, the canvas envelopes, and the five
modules of §4.6 that were surveyed and not read. That is the next
campaign's opening census, and it is larger than this one's close.

---

# ORGAN_3b — THE INSTRUMENT LEARNS TIME

## P0 repair — seven dead dials, found by the cadence lens

Before cadence could be added, a defect ORGAN_3 *created* had to go.
Wave 2 built `PANEL_LIVE`, enrolled its seven dials, and **never moved the
readers**: the beacon writer still read `FIELD_BEACON_*` and `input.hpp`
still read `CameraControls::LOOK_SENS_*`. Seven dials wrote a bank nothing
read — the exact "dead dial" class Jean's sweep named, one campaign
upstream of the one that named it. `PANEL_TABLE`'s own comment asserted a
retirement that had not happened; it is true now.

Fixed: the beacon reads `PANEL_LIVE.beacon`, `input.hpp` reads
`PANEL_LIVE.camera`, and `CameraControls`' four constants retired (the
design row is `PANEL_TABLE`, which holds the literals). The three beacon
constants stay — `PANEL_TABLE` seeds *from* them under a static_assert.

## P0 — cadence

| cadence | count | how it is known |
| --- | --- | --- |
| LIVE | 101 | derived (nothing else applies) — and **silent** in the shell |
| GEN | 2 | **stored** — the only cadence an entry cannot infer about itself |
| BOUNDARY | 107 | derived from `def_kind`, the sentinel block, or `block_has_boundary` |
| DRIVEN | 13 | derived from `ro` |

`derived_cadence()` is the one place the rule lives, so the manifest
emitter and the harness cannot disagree. The harness restates the rule
independently and checks all 223 rows against it, plus two invariants:
stored cadence is only ever LIVE or GEN, and **a witness is never GEN** —
a meter has no edit to defer.

### The GEN set is 2, not the 10–30 the handoff expected

The handoff sized it from the ledger's destructive rows. But ORGAN_3 w3
**enrolled only one** destructive bank (`INDOOR_LIVE`) and deferred the
rest as bulk — `PORTAL_DENSITY`, `FINITE_OUTDOOR_CHANCE`,
`SCHEME_WEIGHTS`, `PORTAL_COLORS`, the ribbon's spawn vocabulary, the
floater weights. Those rows are still DEFER, so there is nothing to tag.
The `_GEN` machinery is built and proven on the two that exist; it carries
the rest the moment they land, at one macro token each.

| GEN id | group |
| --- | --- |
| `INDOOR.height_cap_fraction` | Terrain · Indoor |
| `INDOOR.ribbon_scale` | Terrain · Indoor |

### One fact, one home — the group's name gave the chip its job

`"Terrain · Indoor (edits the next spawn)"` became `"Terrain · Indoor"`.
The warning moved from the group's NAME to the ROW, which is where the
hand is — Jean's diagnosis, applied. Two homes for one fact would have
been the smaller of the two errors, but still one.

### A cadence the panel currently states imprecisely — flagged

`ORB_CONSOLE_LIVE`'s three dials (dome radius, base size, noise floor) are
read **only inside `configure_orbs`**, which runs only from the mood
applier. Their true cadence today is "the next mood change" — neither LIVE
nor GEN nor boundary. They read LIVE, which understates the wait.

Not tagged GEN, because D5 forbids inventing a third phrasing and *"on
respawn"* would be a **wrong** word for a mood change — a wrong word is
worse than a missing one. **P3 fixes it properly**: once the orb boundary
and flag exist, block 5's writes share the orb author's flag and
`block_has_boundary` gains the block, making these three genuinely
BOUNDARY. `block_has_boundary()` is already in place, returning false,
so P3 is a one-line addition.

## P1 — doors

**Door 0, `Re-speak definitions` — BUILT.** The registry gains
`g_doors_pending` (a bitmask, so presses coalesce by construction — three
clicks between two boundaries are one raise, the same reconciliation
philosophy as the flush itself), `organ_door(id)`, `take_doors_pending()`,
a `kOrganDoors[]` roster of ids and labels only, and an `organ_doors()`
ABI so the shell stays name-blind about doors exactly as it is about
dials. The cartridge's `organ_flush` consumes the mask **before** the
existing flag consumption and raises `g_def_dirty` (at the live mood) and
`g_tier_def_dirty`. Nothing else. The lines below it then do what they
already do every frame.

Harness: unknown ids ignored without touching the reject counter (a
rejection means the manifest forbade something; a stale shell is not
that); three presses → one mask, taken once; **a door writes no home and
no dial**; and the raise/consume pair transcribed, each flag taken exactly
once.

**Door 1, `Molt` — PRICED, NOT BUILT (D3).**

| what exists | what it does | why it is not molt |
| --- | --- | --- |
| `request_recenter(WorldState&)` | sets `last_center_*` to `INT32_MAX`, so the next `stream_patches` takes the `fullRegen` path | re-evaluates the patch *window* and re-bootstraps the tile cache. Already-`GENERATED` patches stay generated; **no entity respawns** |
| `mark_patches_for_regen(MachineCtx*, box, home)` | marks patches in a world box `NEEDS_REGEN` | `NEEDS_REGEN` feeds `generate_selected_patches` — the **heightfield** path. `spawn_selected_patches` is a different collector, so terrain regenerates and population does not |
| `evict_patch(MachineCtx*, pi, queue)` + `evict_patch_entities` | drops one patch and its entities; streaming re-spawns later | the only real candidate, and it is a **loop**, not a call |

So no single authored function applies a GEN dial. The build would be
~15 lines — inside D3's line budget — and it is the **risk**, not the
length, that makes this a price:

- it would evict patches the point stands on or beside, while the pawn's
  ground resolve reads that heightfield the same frame;
- re-spawn is budgeted (`SPAWN_BUDGET_PER_FRAME`,
  `patches_budget_this_frame`), so the world would visibly dissolve and
  refill over many frames rather than molt;
- a burst of `evict_patch` calls exercises the free-layer stack in a way
  `reset_surface` — *"THE ONE SURFACE RESET"* — exists to do correctly,
  and duplicating a fraction of it at the boundary is how a second author
  gets born.

**A near-miss worth naming:** a door calling `request_recenter` alone
would be one line and honest about *something* — but labelling it "Molt
(respawns nearby)" would be a lie, and a door whose label overstates it is
worse than no door. If a future sitting wants it, the honest label is
"Recenter (rebuild the patch window)" and it is a different door.

What lifts the price: an authored *molt* verb owned by the surface —
`reset_surface`'s sibling, scoped to a radius, respecting the home patch
and the layer stack — at which point door 1 is one call.

## P2 — the canvas tier, price paid

**The canvas DEFER row is dead.** Its price was quoted as *"one extra
macro parameter, four call sites, no new block"*; the actual bill was one
parameter, **five** call sites (`_GEN` was minted in between), and one
block. The parameter is invisible: every form gained an `_NS` twin taking
the namespace first, and the old name became a one-line forward supplying
`the_board`. All 238 lines written before this commit are untouched, and a
line that cares writes `_NS` and says which namespace.

`coupling/canvas_surface.hpp` — `t7::canvas`, beside its subject. A nested
namespace on purpose: it gives the macro a clean `NS::STRUCT` token
without the organ having to spell `t7::` from inside `t7::organ`.

**Block 9 of twelve.** Three remain before the consolidation threshold.

### The Batch Law changed the answer

The ledger's §5 listed **19** canvas constants as ENROLL w2. Reading every
reader — which is what the Batch Law is for — cut it to **15**:

| constant | verdict |
| --- | --- |
| `TIDE_SHIFT_MIN` / `_MAX`, `RAIN_SCATTER_MIN` / `_MAX` | **NOT ENROLLED — no readers anywhere in the tree.** The file calls them *"DOOR AXES (Movement 1 harvest) … the Movement-2 coupling maps goals into these. Dials — nudge by taste"* — a future coupling's held spans. §5's verdict was taken from that "Dials" comment without a reader census. Enrolling them would have built four more dials that write a home nothing reads, which is the exact defect P0 spent a commit repairing. They enroll the day Movement 2 reads them. |

### What the bank is, and what it is not

These are **envelope authorities** — spans, cadences, ceilings and rates
the couplings read while shaping a signal into a parameter. They are *not*
rests: a pipe's rest lives in `PARAM_LAYOUT` (the register map) and a
pipe's rest at the seam lives in the drivers' room. Three homes, three
different facts, stated in the bank's banner so the next sitting does not
merge them.

`swell_ceiling` and `swell_ramp` carry `(ruled)` in the module. The ranges
keep the ruling reachable so Jean can re-rule it with his hand instead of
a rebuild — which is this campaign's whole argument, applied to a value
that had already been argued once.

### The gap tool caught its own staleness

`organ_gap.py` reported `CanvasSurface 0/15 named` and flagged it `*`.
That is **blind spot 2 working as designed** — the tool prints its file
table so staleness is visible rather than silent — and blind spot 2 turned
out to have a sibling: the parser knew four macro forms and the tree now
has ten (`_GEN`, and an `_NS` twin for each). Taught it both by reading
the suffix and shifting indices by one, rather than carrying a second
table. Back to 33 absent, with `CanvasSurface` and `IndoorSurface` fully
named.

## P3 — the orb mood bank, and a second sentinel

**262 entries.** The Wave-3 deferral said the bank and the kind were
straightforward and the boundary was not. That reading held: the bank and
the kind cost the pattern already proven twice, and the boundary cost a
whole read of `direction/mood.hpp` before a single line moved.

`OrbMoodConfig`, `ORB_MOOD_TABLE` and the id constants its default
initialisers name move to `contracts/orb_surface.hpp` — the agent_tiers
precedent, for the same reason: the organ must name the definition and
**the organ may not include a body**. `bodies/orbs.hpp` keeps its
registries, its gestures and its impl, and already included the contract.

### The boundary, read rather than guessed

`configure_orbs` is called from inside `apply_mood` at one site. The
re-speak re-enters **that call alone** — not `apply_mood` whole, which
would re-run the lighting, the shell and the spawn policy on every orb
edit. The applier site now reads `ORB_MOOD_LIVE[mood]` instead of
`ORB_MOOD_TABLE[mood]`, so a mood change and a dial edit reach the orbs
through the same door; the boundary in `organ_flush` passes
`ORB_MOOD_LIVE[active]`, the **live** mood only, for the reason the mood
re-apply already states — populating this sky from a world we are not in
is not a preview, it is a wrong answer.

`configure_orbs` is idempotent (it rebuilds the population from the
config), so the temperament law grants it the frame-boundary re-speak.
Door 0 now raises three families.

### A kind that is mood-selected, and the collision D4 predicted

`ORGAN_DEF_ORB_MOOD = 4` is the **second** mood-selected kind: like
`MOOD` and unlike `TIER`/`BEHAVIOR`, its bank has a row per mood and the
write's target picks it. It carries its own flag, because its author is
`configure_orbs` and not `upload_agent_registries_to_gpu` — a kind shares
a flag when it shares an author, and these do not.

Its 24 lines are definition-only: `configure_orbs` consumes the config
into GPU buffers, so there is no instance the panel may address.
`ORGAN_PARAM_DEFONLY` hard-coded `ORGAN_BLOCK_NONE`, so both def-only
families landed at 255 and `MoodProfile.clear_color` collided with
`OrbMoodConfig.rotation_axis` at `(255, 32, VEC3)` — **the same triple**,
which `find_entry` resolves by first match. D4 anticipated exactly this;
the macro realised it. Fixed at the macro rather than at the call sites:

    #define ORGAN_DEFONLY_BLOCK_MOOD     ORGAN_BLOCK_NONE
    #define ORGAN_DEFONLY_BLOCK_ORB_MOOD ORGAN_BLOCK_NONE_ORB

pasted as `ORGAN_DEFONLY_BLOCK_##DEFKIND`, so **the sentinel is derived
from the kind** and a third def-only family cannot land on a used one
without adding its own line. `is_defonly()` answers for both, and
`def_test` now proves the mapping across all 262 entries as well as the
no-duplicate-triple law that first caught it.

### An ORGAN_1 rule amended, not broken

`write_definition` and `read_definition` refused `U32` and `BOOL`, so the
orb `count`, `motion_rule`, `palette_id`, `flock_gesture_default` and
`enabled` writes were silently rejected. ORGAN_1's stated reason was
**reinterpretation** — writing a float's bits into an integer field. But
`organ_set`'s instance path has *converted* since ORGAN_0, and conversion
is not reinterpretation. Both definition paths were taught the same
conversion the instance path already uses, and the enum banner's
"FLOAT LANES ONLY" became **"NEVER REINTERPRET"** — the rule that
survives is the one that was actually being defended.

`enabled` widens from C++ `bool` to `uint32_t` by D2: the ABI's BOOL
write is four bytes, a `bool` is one, and this struct has no GPU mirror,
so the widen is lawful and the table's `true`/`false` rows initialise a
`u32` unchanged. The aura-intent precedent, applied without argument.

`tierset_id` stays deferred, and for the reason §4.3 already gave: its
"none" value is `0xFFFFFFFF`, and a 0…1 slider cannot express a sentinel
without lying (D1(d)). The gap tool reports it, by name, as the one
absent member of `OrbMoodConfig`.

### The tally at P3

| | |
| --- | --- |
| entries | **262** |
| by section | Sky & Light 33 · Atmosphere 22 · Terrain 39 · Pawn 18 · Ribbon 25 · Agents 102 · Interaction 19 · Debug 4 |
| by cadence | live 113 · gen 2 · boundary 134 · driven 13 |
| definition kinds | NONE 131 · MOOD 5 · TIER 32 · BEHAVIOR 70 · ORB_MOOD 24 |
| definition-only | 255 → 2 (MoodProfile) · 254 → 24 (OrbMoodConfig) |
| blocks used | 0…9 of twelve |

## P4 — shell navigation

262 rows are a library, not a page. Three additions, no fourth.

**The filter** matches `id + label + group` lowercased — the three names a
stop already answers to, so the operator reaches it by whichever one they
remember. A row hides by hiding the elements it built (a VEC3 is a header
plus one line per lane, which is why `finish()` now carries the row's
nodes rather than the shell guessing at the DOM's shape); a group header
hides when it has no visible row; a section hides when it has none either.
While filtering, a section's tally reads `hits/total`, so a word's reach
across a voice is legible without opening it.

**Collapsed by default, with the hand's choice kept.** The panel opens as
a table of contents. The filter opens what it finds — but an auto-open is
not a choice, so it is not recorded: `openMap` holds only what the
operator opened by hand, and clearing the filter returns the desk to that.
Session-only, no storage; the artifact rule and the instrument's
ephemerality agree here.

**Per-section export** — one affordance on each section header, the same
export walk narrowed by a predicate. Witnesses stay skipped, `world/` and
`<mood>/` keying is unchanged, and import needed nothing at all: a partial
file has always applied exactly what it carries. *A voice is a file.*

Harness: 8 sections all closed at boot · `fog` narrows 262 → 6 and the
count agrees with the manifest's own substring test · 7 sections hide,
Atmosphere opens and reads `6/22` · clearing restores 262 and gives back
the one section the hand had opened · `Sky & Light` exports 32 keys —
its non-witness rows, keyed exactly as the whole-panel export keys them —
to `organ-sky-light.json`, without toggling its own section.

## P5 — the adjudications, and a claim this ledger got wrong

### The beacon: the clamp, and the end nobody was watching

Jean stamped the clamp at the writer over the paired-range restatement,
and for the reason that made it the better of two working cures: a clamp
at the writer guards **every** author — the couplings this campaign is a
target map for, included — while a range guards only the panel. That is
the `flee_gain_player` precedent (THE CATCHABILITY LAW), applied a second
time without argument.

Building it turned up the part the ruling had not been asked about. This
ledger and the `.inc` both said `FIELD_K` *"only ever raises the winning
side, so the ruling survives every value that dial can reach."* It does
not. `field_k`'s enrolled min is **0**; lowering it under S breaks
`S < FIELD_K` from the other end, and nothing guarded that — the deferral
had been watching one door of a room with two.

So the clamp reads config's **live** `field_k` rather than the constexpr:

```cpp
const float ceiling = gpuState_.config().field_k - 1.0f;
float s = bcn.s;
if (s > ceiling) s = ceiling;
if (s < 0.0f)    s = 0.0f;
```

`FIELD_BEACON_S_MAX = FIELD_K − 1` is the panel's half — S enrolls with it
as its max, so the range and the clamp agree by construction at the
authored k — and the writer's live read is the other half. The
`static_assert` stays, unchanged: it still proves the authored pair, and a
second `static_assert` now proves the authored S is reachable by its own
dial, so the enrolled range can never silently clamp the design's own
value. Three proofs of one ruling, each speaking about what it can see.

The harness walks every k the `field_k` dial can reach — 0 to 1200 in
half-steps — and asserts `S < k` at each. Both stale sentences are
corrected in place.

### The plasticity comment: stale in three places, not one

`Idle::CUBE_PLASTICITY_DEFAULT` was raised 0.6 → 1.0 at CONTACT_5 P2b and
the constant's own comment says so. Three downstream comments did not:
`state.hpp`'s struct member, `state.hpp`'s boot-pin line, and
`world.wgsl`'s field declaration — the C++ and WGSL twins of one fact,
both stale in the same way. All three amended. The dial itself enrolled in
Wave 1 and needed nothing; Jean's eye holds the final word between 0.6 and
1.0 now that both are one drag apart, which is the instrument finishing an
argument two comments started.

### What P5 did not touch

The nine surviving DEFER rows keep their reasons and their owners, listed
in this file's header. None is externally blocked; `docs/OPEN.md` carries
no ORGAN line at all now that the beacon's is dead, which is the register's
own law working — a line dies when its item closes.

---

# ORGAN_3c — THE POLISH AND THE PROOFS

## P0 — the row grid, and a minimum that is computed

The flex row had one line and five claimants on it: label, control, value,
contest marker, chip. Flex resolves that by shrinking whatever will shrink,
and at 330px what shrank was the slider — behind the number, which is the
defect Jean named.

**Two lines, both grids, columns placed explicitly.**

```
line 1  [ label ……………………………  sw   mk   chip ]
line 2  [ slider ————————————————— | value ]
```

Explicit `grid-column` on every cell, not auto-placement: a row with no
swatch and no chip still lines its markers up with the row above it, which
is what makes 263 rows read as a column rather than as a list. The label
is the only cell that gives, and what its ellipsis hides it hands to the
`title` — the label and the id, so the hover answers both "what is this"
and "where does it live".

VEC3/VEC4 lanes are now **stacked**: one full-width line each, the colour
swatch up on the label line beside the markers. Witnesses keep their meter
in the value column, so a meter reads down the same edge as a dial.

### The minimum is computed, because a guessed minimum is how overlap returns

Every fixed width the grid uses is a CSS custom property on `#organ`, and
`W_MIN` is computed from the same numbers (D2):

```js
var W_MIN = 2*G.pad + G.body + G.lblmin + 3*G.hdgap + G.sw + G.mk + G.chip;
```

**292px.** Line 1 governs; line 2 needs only 174. `W_MAX = min(640, 50vw)`.
The grip rides the panel's inner edge on pointer events; double-click is
home; the width lives beside `openMap` and dies with the session, by the
same law — a dev instrument that remembers is a dev instrument that
surprises.

### The harness parses the stylesheet rather than a copy of it

The shim has no layout engine, so it reads the grid's fixed parts **out of
the shipped CSS** — the custom properties the rules themselves use — lays
the columns out at MIN, default and MAX, and asserts three things per line:
the flexible column never sinks below its floor, no two boxes overlap in
order, and nothing runs past the content box. Then it drives the grip and
checks the shell's own clamp equals the independently computed `W_MIN`, and
that one pixel under MIN the label sinks below its floor — so MIN is proved
*binding*, not merely present.

```
MIN     W=292: label=110px  slider=208px   no overlap ✓
default W=330: label=148px  slider=246px   no overlap ✓
MAX     W=640: label=458px  slider=556px   no overlap ✓
```

What that proves is a property of the CSS that ships, not of a table
maintained beside it.
