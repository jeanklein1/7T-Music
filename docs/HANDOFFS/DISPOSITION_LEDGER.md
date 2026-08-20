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

**DEFER rows that died,** each in the commit that landed it. ORGAN_3
killed the canvas tier (P2), `ORB_MOOD_TABLE` → `ORB_MOOD_LIVE` (P3) and
`FIELD_BEACON_S` (P5); ORGAN_3c killed none, its one target
(`mute_couplings`) having come back with evidence for keeping it; and
**ORGAN_4 killed four of the nine that survived** — the ribbon's four
pipes (P2), the four mood-structural witnesses (P3a), `POSSESSION_RADIUS`
(P3b) and Wave 3's remaining destructive banks (P3d).

**FIVE SURVIVE**, and each names its own owner in place:

| what | owner |
| --- | --- |
| `THEME_BASE_WEIGHT`, `INDOOR_PALETTES[]`, `tierset_id` | a composite editor (D5) |
| `mute_couplings` | a checkbox per bit — a shell feature, not an enrollment line; and ORGAN_3c found that the bits are not there to check |
| `floater_coordination` | a TEMPERAMENT RULING, not a reading. ORGAN_4 P3c read the module and found the deferral's premise false: it is not driven, it is player-cycled. One `ORGAN_PARAM` line lands it the moment D1 rules who owns it. |

None is externally blocked, which is why none of them is in
`docs/OPEN.md`. The one that was — the beacon's assert — is the line
ORGAN_3b P5 killed there.

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
| `mute_couplings` | U32 | `Coupling::NONE` | C1 — a **bitmask** | Debug | — | **DEFER-RANGE (D1d)**, and **ORGAN_3c re-affirmed it on new evidence** — see below. `Coupling::ALL` is `0x1FFFFF`; a slider from 0 to 2 097 151 is not a dial. It wants a checkbox per bit; the census found that the bits are not there to check. |
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

**`POSSESSION_RADIUS`'s row died at ORGAN_4 P3b.** Its condition was met
rather than waived: the `_SQ` twin had exactly one reader, so the square is
derived AT that read from the live value and the second constant is retired.
Nothing survives from Wave 2.

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

**The remaining destructive banks died at ORGAN_4 P3d** — five of the six
landed, and the sixth was retired for a reason the bulk reading could not
have found. `PORTAL_DENSITY`, `FINITE_OUTDOOR_CHANCE`, `SCHEME_WEIGHTS` and
the portal palette became `WORLD_DRAW_LIVE` (block 10,
contracts/mood_constants.hpp); the ribbon's spawn rolls and colour
vocabulary became `RIBBON_SPAWN_LIVE` (block 11,
contracts/ribbon_surface.hpp), a SECOND bank beside `RIBBON_LIVE` because
its temperament differs. **The floater tier weights did not land:**
`SPHERE_BASE_TIER_WEIGHTS` and `CUBE_BASE_TIER_WEIGHTS` have NO READER —
see P3d below. Nothing survives from Wave 3.

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
| `floater_coordination` | **SHARPENED at ORGAN_4 P3c, and the premise below was wrong.** It is not driven at all: `cube_behaviors.hpp:400` is inside `cycle_floater_coordination`, a PLAYER COMMAND. There is no per-frame author. The anatomy and the priced shape are in P3c below; what remains is a temperament ruling, not a reading. |

**The four mood-structural witnesses died at ORGAN_4 P3a** — all four
land in `GPUDesignConfig`, the block the panel already addresses, so each
had the fixed home a witness needs and none needed a mirror minted for it.

**The ribbon's four pipes died at ORGAN_4 P2** — the fourth pipe's blocker
was read rather than worked around: downstream is `st ? st[c2] : 0.0f`, so
the null branch's rest SHAPE is `{0,0,0}`, which is PARAM_LAYOUT's own rest
column for `ribbon.color_stim`. The anatomy this row predicted is the
anatomy that landed, field for field.

---

# THE GAP AT CLOSE

`tools/organ_gap.py --gate`, run against the tree ORGAN_3c leaves. Every
absent member below has a reason in the sections above — the tool
finds the gap, the ledger explains it, and neither pretends to do the
other's job.

```
ORGAN GAP — members of the enrolled homes that the panel does not name
========================================================================
A map, not a gate. Reasons live in docs/HANDOFFS/DISPOSITION_LEDGER.md.

THE FILE TABLE this run trusted (blind spot 2 — stale rows are
invisible bugs, so they are printed):
    AgentBehaviorBank        src/cartridges/the_board/contracts/agent_tiers.hpp   AGENT_BEHAVIORS -> BEHAVIOR_LIVE
    AgentTierBank            src/cartridges/the_board/contracts/agent_tiers.hpp   AGENT_TIER_GAINS -> TIER_LIVE
    CanvasSurface            src/coupling/canvas_surface.hpp                      CANVAS_TABLE -> CANVAS_LIVE
    DriverSurface            src/cartridges/the_board/contracts/driver_surface.hpp DRIVER_TABLE -> DRIVER_LIVE
    GPUAgentRoomConstants    src/cartridges/the_board/realization/state.hpp       (not a graduation)
    GPUDesignConfig          src/cartridges/the_board/realization/state.hpp       (not a graduation)
    GPULighting              src/cartridges/the_board/realization/state.hpp       (not a graduation)
    IndoorSurface            src/cartridges/the_board/contracts/indoor_module.hpp INDOOR_TABLE -> INDOOR_LIVE
    MoodProfile              src/cartridges/the_board/contracts/spine_state.hpp   MOOD_TABLE -> MOOD_LIVE
    OrbConsole               src/cartridges/the_board/contracts/orb_surface.hpp   ORB_CONSOLE -> ORB_CONSOLE_LIVE
    OrbMoodConfig            src/cartridges/the_board/contracts/orb_surface.hpp   ORB_MOOD_TABLE -> ORB_MOOD_LIVE
    PanelSurface             src/cartridges/the_board/contracts/control_panel.hpp PANEL_TABLE -> PANEL_LIVE
    PawnAuraProfile          src/cartridges/the_board/contracts/pawn_surface.hpp  PAWN_AURA_DEFAULT -> PAWN_AURA_LIVE
    RibbonSurface            src/cartridges/the_board/contracts/ribbon_surface.hpp RIBBON_TABLE -> RIBBON_LIVE

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

THE READER WITNESS — every mention of a DESIGN symbol, classified.
A graduation is complete when the design table's only readers are
its seed and its asserts. Anything else is a surviving runtime
reader, and the reason ORGAN_3 shipped seven dead dials.

  AGENT_BEHAVIORS      definition=1 seed=10 comment=7             
  AGENT_TIER_GAINS     definition=1 seed=4 static_assert=2 comment=7 
  CANVAS_TABLE         definition=1 seed=1 comment=1              
  DRIVER_TABLE         definition=1 seed=1 comment=1              
  INDOOR_TABLE         definition=1 seed=1 static_assert=2 comment=1 
  MOOD_TABLE           definition=1 seed=4 static_assert=9 constexpr=4 comment=34 
        constexpr derivation (D7)  src/cartridges/the_board/bodies/gallery.hpp:346  MOOD_TABLE[MOOD_INDOOR_FLAT].finite_radius_max
        constexpr derivation (D7)  src/cartridges/the_board/bodies/gallery.hpp:347  > MOOD_TABLE[MOOD_INDOOR_VAULT].finite_radius_max
        constexpr derivation (D7)  src/cartridges/the_board/bodies/gallery.hpp:348  ? MOOD_TABLE[MOOD_INDOOR_FLAT].finite_radius_max
        constexpr derivation (D7)  src/cartridges/the_board/bodies/gallery.hpp:349  : MOOD_TABLE[MOOD_INDOOR_VAULT].finite_radius_max;
  ORB_CONSOLE          definition=1 seed=1 comment=2              
  ORB_MOOD_TABLE       definition=1 seed=4 comment=7              
  PANEL_TABLE          definition=1 seed=1 static_assert=4 comment=2 
  PAWN_AURA_DEFAULT    definition=1 seed=1 static_assert=1 comment=3 
  RIBBON_TABLE         definition=1 seed=1 static_assert=1 comment=2 

SURVIVING RUNTIME READERS ACROSS 11 GRADUATED PAIRS: 0
--gate: PASS

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

## P1 — the proofs: the program's side of the contract

### The reader witness, and a fifth class the evidence asked for

ORGAN_3 w2 built `PANEL_LIVE`, enrolled it, and left the readers on the
constexprs. Seven dials wrote a bank nothing read, and it took a campaign
and a human sweep to find. That class of defect is mechanical, so it is
now caught mechanically: for each of the **eleven** graduated pairs, every
word-boundary mention of the DESIGN symbol anywhere in `src/` is
classified, and anything outside the lawful classes is a surviving runtime
reader.

D3's four classes held for ten pairs. `MOOD_TABLE` produced a fifth that
the decision table does not name, so it is reported rather than assumed:

| class | count across the tree | verdict |
| --- | --- | --- |
| definition | 11 | lawful — the declaring statement |
| seed | 29 | lawful — the statement also names the LIVE bank |
| static_assert | 19 | lawful — the design table's second job |
| comment | 67 | lawful — prose, including message strings |
| **constexpr** | **4** | **lawful, and printed always** |
| violation | **0** | — |

The four are `gallery.hpp:346-349`, deriving `INDOOR_RADIUS_MAX` from
`MOOD_TABLE`'s `finite_radius_max` rows. This is **D7's already-ruled
case**, and it is lawful for a reason stronger than convenience: a
`constexpr` initialiser *cannot* read a mutable `inline` bank — it is
ill-formed, not merely awkward — and a value fixed at compile time is one
the panel could never move. Calling it an incomplete graduation would be
false; enrolling its source would build the dead dial, not remove one. The
fields it reads are MoodProfile **structural**, already C5 by the standing
eligibility rule. The class is printed on every run rather than folded
into silence, so the day one of these stops being compile-time it is
visible.

**The gate was proved to bite before it was believed.** A probe file
declaring `return DRIVER_TABLE.fog.gain;` produced
`violation=1 <-- SURVIVING READER` and `rc=1`; removing it returned `rc=0`.
A gate that cannot fail is decoration.

`organ_gap.py --gate` is a standing member of the harness family from this
commit. The map itself stays toothless — exit 0 always — because an
unenrolled member is a judgement the ledger makes and the tool may not.

### The kind × type round-trip

Systematic, from the table rather than by example: `def_test` groups every
writable definition entry by (kind × type), writes one representative per
non-empty cell through `organ_set` with a definition target, and reads it
back through `organ_def_get`. A kind or a type that enrolls tomorrow is
covered the day it does, and an empty cell prints `empty` rather than
failing (D4).

```
  kind x type          F32           U32           BOOL          VEC3          VEC4
  NONE                empty         empty         empty         empty         empty
  MOOD                ok            empty         empty         ok            empty
  TIER                ok            empty         empty         ok            empty
  BEHAVIOR            ok            empty         empty         empty         empty
  ORB_MOOD            ok            ok            ok            ok            empty
  9 non-empty cells, 9 proved, 16 empty
  integer lanes on the definition path: 2 cells, all converted ✓
```

The two integer cells are ORGAN_3b's amendment under oath: the write is
never refused, and `7.0f` into a `U32` lane reads back as `7`, not as the
1.08e9 a bit-reinterpretation would produce. The float cells prove nothing
regressed while the path learned to convert.

## P2 — FLAGGED, not built: the mute strip has no bits to check

ORGAN_3c set out to build the cure this ledger named for `mute_couplings`
— *"a checkbox per bit, which is a shell feature"*. The census stopped it
at D1's third branch, and produced a second reason the ledger did not have.

**Finding 1 — the enum is an entangled WGSL mirror, by its own word.**
`Coupling::` opens at `realization/state.hpp` with:

> *"Mirrors world.wgsl's COUPLING_\* bit-flag block. MUST match those bit
> values 1:1 — semantic drift here would corrupt every GPU-side coupling
> read silently."*

D1's third branch is written for exactly this: *generated or entangled
with WGSL mirrors → FLAG, skip the phase*. Minting a name list beside one
half of a hand-kept mirror is how the two halves begin to drift, and the
banner says what drift costs.

**Finding 2 — `ALL` is a blanket, not a roll-call.** This is the one the
ledger did not know, and it is the stronger of the two:

| | |
| --- | --- |
| `Coupling::ALL` | `0x1FFFFF` — **21 bits** |
| named bits | **8** — bits 1, 2, 3, 4, 5, 6, 14, 16 |
| unnamed | bit 0 and twelve others, sparsely |

The specified `static_assert(count == bit-width of ALL)` cannot hold: 8 is
not 21, and the gap is not an oversight to be filled but a mask that was
always wider than its vocabulary. Twenty-one checkboxes would be **thirteen
toggles over bits nothing reads** — the dead-dial defect ORGAN_3b P0 spent
a commit repairing, rebuilt deliberately.

**One more thing the witness script should know.** §6 asks Jean to *"solo
the fog coupling"*. There is no fog bit. The eight named couplings are
terrain→pawn (y, tilt), pawn→camera, the three input couplings,
terrain→sphere-height and pawn→sun-VP. `Coupling::` masks the **program's**
couplings; the music campaign's couplings are a different vocabulary that
does not exist yet. That is worth settling before the strip is built,
because it changes what the strip is for.

### The shape, priced and unbuilt

A roster over the **eight named bits** — `{ id, bits, labels }` emitted by
an `organ_masks()` ABI beside `organ_doors()`, `mute_couplings` enrolled as
a plain `U32` that never renders a slider, and a checkbox grid the shell
draws name-blind from the roster. `COUPLING_NAMES[]` would sit beside the
existing constants with a `static_assert` tying each entry to its own
constant and asserting every named bit lies inside `ALL` — not a count
assert against `ALL`'s width, which is the assert that cannot exist here.

That is a defensible design and it is **not CC's to choose**: it changes
what a hand-kept WGSL mirror is allowed to grow, and D1 reserves that. The
deferral survives with both findings attached; the next sitting starts from
them rather than from the enum.

---

# ORGAN_4 — THE READER ANSWERS

**The law of this campaign:** an enrollment states a belief; only the
reader proves it. Every row this handoff touches ends the phase either
consumed by its reader or retired with its reason ledgered.

## P0 — the census

### The stale-authority gates: all five green

| gate | subject | verdict |
| --- | --- | --- |
| 1 | `configure_orbs` (`bodies/orbs.hpp`) | reads `ORB_CONSOLE_LIVE.noise_floor / .dome_radius / .base_size`; zeros `force_radial`, `color_pulse/converge/surge`; `gpuCfg.motion_rule = os.current_motion_rule` behind the one-time Brownian seed; ends `upload_orb_config(queue, gpuCfg); os.init_pending = true;` — with `log_configure_` after it, a print and nothing else |
| 2 | `realization/state.hpp` | `upload_orb_frame` (dt+t coalesced, `static_assert`ed adjacent) and `upload_orb_noise` (`offsetof(noise_amp)`, 4 bytes) both present |
| 3 | `src/console/organ_params.inc` | `Sky & Light · Dome` = 3 plain `ORGAN_PARAM(ORBS, OrbConsole, …)`; `Orb mood` = 13 and `Orb flock` = 11 `ORGAN_PARAM_DEFONLY(… ORB_MOOD, OrbMoodConfig …)` — 24 total |
| 4 | `src/console/organ_registry.hpp` | `kOrganDoors` carries exactly `ORGAN_DOOR_RESPEAK`; `ORGAN_DOOR_COUNT = 1`; `g_orb_def_dirty` + `take_orb_definition_dirty()` present; the cartridge boundary consumes them and calls `configure_orbs(orbs_state_, &orbs_deps_, ORB_MOOD_LIVE[mood_state_.active % MOOD_COUNT], queue)` |
| 5 | `bodies/ribbon.hpp` | the four pipe reads sit at :824-838 with fallbacks `: 1.0f`, `: 1.0f`, `: 0.0f`, `: nullptr`, and downstream `const float s = st ? st[c2] : 0.0f;` |

Nothing differs materially from the handoff's quotes; D7 does not fire.
Two immaterial drifts, recorded so a successor is not surprised: gate 1's
function ends with a diagnostic print after the upload, and gate 5's block
sits nine lines above the quoted 833-846 (ORGAN_3d's edits moved it).

### FLAG — the entry count is 263, not 262

The handoff sizes the campaign from 262 and expects 257 at close. The tree
carries **263**: ORGAN_3b P5 enrolled `PANEL.beacon.s` after the P3 tally
this file records, and ORGAN_3c's own prose already says *"263 rows read as
a column"*. The five retirements therefore land 258, not 257, before this
campaign's additions. Flagged and continued; the close tally below is the
counted one, not the predicted one.

### C1 — `organ_set`'s instance-write path

`organ_registry.hpp`, the tail of `organ_set`. After the whitelist lookup,
the `ro` refusal, the def-only branch and the definition branch, the
instance path is: take `base`, offset by `e->offset`, convert-or-clamp-and-
write, then three statements —
`g_home->organ_mark_dirty((uint32_t)block);`,
`if (block_has_boundary((uint8_t)block)) g_orb_def_dirty = true;`,
`note_write(*e);`. The per-block hook belongs on the middle statement: it
is the one site that has already succeeded at clamping and writing, and it
is already keyed on the block. D1 satisfied without adding a site.

### C2 — is `OrbMoodConfig.motion_rule` read anywhere at all?

**No. Dead tree-wide.** `git grep -n "motion_rule" -- src | grep -v
current_motion_rule` returns eleven hits and not one of them reads the
config field: three name `OrbsState::motion_rule_initialized`, two name
`cycle_orb_motion_rule`, three name `GPUOrbConfig::motion_rule` (the
struct member, its partial uploader, and the WGSL mirror's dispatch),
one is the field's own declaration, one is a comment, and the last is the
enrollment line itself. `configure_orbs` writes
`gpuCfg.motion_rule = os.current_motion_rule` — the player's, never the
mood's. The dial has been writing a field with no reader since the bank
was born.

### C3 — `ORB_MOOD_TABLE` rows sitting on the sentinel

`drag` 0.4/0.5/0.5/0.4 and every `flock_*` at its authored value: no zeros.
`orbital_base_speed` is **0.0f in all four rows** — so four of four rows
ride `eff()`'s fallback (`ORB_DEFAULT_ORBITAL_SPEED`, 0.15) rather than an
authored value. The four `rule_drag_*` are 0.0f in all four rows too:
sixteen initialisers riding `passthrough()`'s 1.0×. The sentinel is in
live use, which is what D1(d) needs to be true.

### C4 — is there a fixed CPU home for the ribbon's driven quartet?

**No.** `RibbonState` holds `GPURibbonState gpu[MAX_RIBBON_INSTANCES]` and
`uint32_t rendered_slot = UINT32_MAX`. The last-uploaded amps and colour
live at `rs.gpu[rs.rendered_slot]`, whose index moves with eviction and
reads a sentinel when nothing is rendered. There is no `last_uploaded_*`
member and no other candidate. Per **D2**, P2c enrolls **no witness** for
the ribbon seam: *a witness needs a home; a varying slot is not one*, and
minting a mirror to give it one is forbidden by windows-not-homes.

### C5 — the `floater_coordination` driver, read whole

**The ledger's premise is wrong, and this is the correction.** Wave 4 filed
it as *"driven by `cube_behaviors.hpp:400`"*. That line is inside
`cycle_floater_coordination`, a **player command**: it steps
`cbs.coordination_step` through three values of
`FLOATER_COORDINATION_STEPS` and calls `stage_floater_coordination(v)`,
which assigns `config_.floater_coordination` with no dirty raise. There is
no per-frame author anywhere. Its readers are `world.wgsl:8845` (per frame)
and `cartridge.hpp:1143`, where `phase_motion_drivers` reads
`config().floater_coordination` as the beacon's S gain. So it is **not a
C4 seam** — a rest+gain room for it would model a driver that does not
exist. Verdict feeds P3c.

### C6 — the four mood-structural facts and their instance homes

| fact | instance home | the applier that writes it |
| --- | --- | --- |
| `veil_strength` | `GPUDesignConfig.veil_strength` | `cartridge.hpp:1206` — `set_veil_strength(finite_mode ? 0 : 1)` |
| `terrain_amp_ceiling` | `GPUDesignConfig.terrain_amp_ceiling` | `direction/mood.hpp:574` — `set_terrain_amp_ceiling(m.terrain_amp_ceiling)` |
| `ceiling_height` | `GPUDesignConfig.ceiling_height` | `direction/mood.hpp:663/665` — `set_ceiling_height(effective_ceiling)` / `(0.0f)` |
| `indoor_height_cap` | `GPUDesignConfig.indoor_height_cap` | `direction/mood.hpp:581` — `set_indoor_height_cap(…)` |

All four have a FIXED home in the block the panel already addresses
(`ORGAN_BLOCK_CONFIG`), so all four clear **D2** and take witnesses.

### C7 — `POSSESSION_RADIUS` and its `_SQ` twin

`bodies/agents.hpp:93-94` declares the pair; `POSSESSION_RADIUS_SQ` has
**exactly one reader** — `float best_d2 = POSSESSION_RADIUS_SQ;` at :498 —
and `POSSESSION_RADIUS` has exactly one, the "no agent within N units"
diagnostic at :515. Single derivation site, so P3b takes its first branch.

### C8 — the surviving Wave-3 destructive-bank DEFER rows, with homes

| bank | home | its live reader |
| --- | --- | --- |
| `PORTAL_DENSITY` | `direction/mood.hpp:107` | `entity_pipeline.hpp:1034` — `if (portal_roll < PORTAL_DENSITY)` |
| `FINITE_OUTDOOR_CHANCE` | `direction/mood.hpp:1296` | `direction/mood.hpp:1300-1303` — the world-draw ladder |
| `SCHEME_WEIGHTS[4]` | `direction/mood.hpp:279` | `direction/mood.hpp:353` — the indoor light-scheme roll |
| `PORTAL_COLORS[4][3]` + `PORTAL_COLOR_BACK[3]` | `contracts/mood_constants.hpp:35/48` | `portal_color_for()` at :53, called when a portal is built |
| the ribbon's spawn rolls | `bodies/ribbon.hpp:78,87` (`RibbonConfig::SPAWN_CHANCE / POSITION_JITTER`) and `:118-130` (`WANDER_*`) | `select_ribbon_for_patch:1114`, `place_ribbon_from_selection:1177`, `commit_ribbon:1277` and the wander policy — all runtime, none constexpr |
| the ribbon's colour vocabulary | `bodies/ribbon.hpp:132-161` | `fill_ribbon_selection_geometry:1013-1026` |
| the floater tier weights | `contracts/floaters.hpp:41,100` | **NONE — see below** |

**`SPHERE_BASE_TIER_WEIGHTS` and `CUBE_BASE_TIER_WEIGHTS` HAVE NO READER.**
`git grep` finds each symbol exactly once in `src/`: its own declaration.
The generic pipeline rolls a floater's tier from
`adapter.get_tier_profile(t).weight` — `SPHERE_TIERS[t].weight` and
`CUBE_TIERS[t].weight` (`entity_pipeline.hpp:110-116`) — multiplied by
`theme_tier_weights()`. The base arrays were superseded and left standing.
Enrolling them would have built two dead dials on purpose, which is the
exact defect this campaign exists to end, so their share of the Wave-3 row
is **retired, not landed** (P3d).

## P1 — the sky repairs

### P1a — the console mask: a CPU bank whose reader is an EVENT

`ORB_CONSOLE_LIVE`'s only reader is `configure_orbs`, so the three Dome
dials were dead until a mood change; ORGAN_3b P3 cured the *silence* with a
block-wide `g_orb_def_dirty` raise, which cured it by firing the whole
applier — and `configure_orbs` ends `os.init_pending = true`, so every
notch of the dome slider **re-seeded the sky**. Right about WHEN, wrong
about WHAT.

**The idiom, stated once so it can be reused:** a CPU bank whose reader is
an event gets a PER-FIELD MASK consumed at the boundary, and the boundary
routes each field to the cadence its own reader has.

```cpp
inline uint32_t g_orb_console_dirty = 0;   // bit = offsetof/4
```

Three routes, chosen by reading the kernel rather than by taste:

| field | GPU reader | route |
| --- | --- | --- |
| `dome_radius` | `world.wgsl:13218` (init) and `:13576` (the dynamics shell re-projection, every frame) | targeted 4-byte partial — `upload_orb_dome_radius`, minted this phase |
| `noise_floor` | `world.wgsl:13371/13390`, every frame | targeted 4-byte partial — `upload_orb_noise`, **revived** from its gen-1 orphanhood; its comment now names the panel as its living caller |
| `base_size` | `world.wgsl:13323` (init) and `:13263` (recolor) — baked into `orb_state[i].size` | the definition re-speak: bit 1 raises `g_orb_def_dirty` |

**One statement changed in `organ_set`, not added.** D1 asked for the hook
at the single site after the clamp and the write succeed, keyed on the
block. That site already existed — the `block_has_boundary` raise — so the
raise became the mask and no new site was born:

```cpp
if (block == ORGAN_BLOCK_ORBS)
    g_orb_console_dirty |= (1u << (e->offset / 4u));
```

`block_has_boundary` STAYS and keeps its meaning: all three fields still
land at the frame boundary, so all three still read BOUNDARY cadence. What
changed is what the boundary *does*, and that is plumbing, not cadence —
two questions, two mechanisms. A `static_assert` beside the flag pins
dome 0 / base size 4 / noise 8, so a field reordered in `OrbConsole` fails
the build at the flag rather than routing a radius into the noise floor.

The boundary block sits IMMEDIATELY BEFORE `take_orb_definition_dirty()`,
so a base-size raise is consumed in the same frame it is made.

### P1b — five rows retired, each on its reader's word

| row | field | fate |
| --- | --- | --- |
| `base hue` | `OrbMoodConfig.base_hue` | **row dies, field docketed (L26).** Dead BY CONSTRUCTION: every `ORB_PALETTES` row carries `count ≥ 1` (4, 4, 3, 1), so `pack_palette_` never leaves `palette_count` at 0, so `world.wgsl`'s legacy single-hue arm (`if palette_count > 0u` … else) is unreachable. |
| `hue variance` | `OrbMoodConfig.hue_variance` | same branch, same fate. |
| `hue converge target` | `OrbMoodConfig.hue_converge_target` | **row dies, field STAYS.** It is mood-scoped and honest; `configure_orbs` copies it, but `color_converge` is hard-zeroed, so nothing moves when it is turned. **Revival intent ledgered:** the row returns in the commit that gives `color_converge` a gen-2 coupling. |
| `rule (0B 1O 2F 3K)` | `OrbMoodConfig.motion_rule` | **row dies, field docketed (L26).** C2 proved it dead tree-wide. The applier's hardcode is a RULING, restated in the .inc: the rule is player-owned, seeded once to Brownian. Its reachable form is the door. |
| `flock gesture seed` | `OrbMoodConfig.flock_gesture_default` | **row dies, field STAYS.** A first-run seed is its honest job; a boot-only fact wearing a boundary chip misreports, because `apply_mood_first_run_defaults_` refuses it on every run after the first. Its reachable form is the other door. |

The three docketed fields do not die now: `ORB_MOOD_TABLE` is positionally
brace-initialised, so D3 holds the broom back — the twin rooms move in one
commit, and `docs/OPEN.md`'s L26 line carries them until then.

Orb mood 13 → 8, Orb flock 11 (unchanged), ORB_MOOD 24 → 19, entries
263 → **258**.

### P1c — two doors

`ORGAN_DOOR_ORB_RULE` and `ORGAN_DOOR_ORB_GESTURE` call
`cycle_orb_motion_rule` and `cycle_orb_gesture` — the same functions KP_8
and KP_7 press. No author added, no behavior invented, no write path
opened; each keeps its own guard and its own partial upload. The shell
renders one button per manifest row name-blind (`organ_panel.js:457-475`),
so **zero JS edits**: the two buttons appear because the roster grew.

This is what a player-owned fact's enrollment looks like when the panel
tells the truth: not a dial the applier ignores, but the program's own
command with a button on it.

### P1d — thirteen floors, and one correction to the handoff

The floors are one step each, and the correction is arithmetic. The handoff
glossed the four `rule_drag_*` floors as **0.01**, against a step of 0.02;
D4 says *min := exactly one step*. Half a step puts every gridline off the
integers — with min 0.01 and step 0.02 the dial reaches 0.99 and 1.01 and
**never 1.0**, which is exactly the pass-through identity the operator most
needs. D4 governs; the four floors are **0.02**. The harness proves the
consequence directly: for all thirteen rows the authored mood value still
sits on the grid `min + k·step`.

### P1e — the palette temperament, stamped

Stamped above `pack_palette_` verbatim, plus the sentence P1b earned: the
same function is why `base_hue`/`hue_variance` are not dials. Palette is
config-owned (the dial and the mood are the durable authors); rule and
gesture are player-owned (seed once). The asymmetry is deliberate — a dial
exists for `palette_id`, and a config the applier ignores is a dead dial.

### P1f — the harness, run against the compiled registry

Not a browser shim this time: the assertions are about the COMPILED table,
so they were compiled and **executed** natively against the real headers
under the console_gate's own pinned surface (`clang++ -std=gnu++20
-D__EMSCRIPTEN__`, the vendored emdawnwebgpu includes, the gate's stub
set). The harness is a scratchpad instrument; its output is the artifact.

```
ORGAN_4 harness — the compiled registry, executed
  [PASS] entry tally is 258 (263 - 5; the handoff predicted 257)
  [PASS] ORGAN_DOOR_COUNT == 3 / kOrganDoors carries three rows / both labels
  [PASS] five retired ids ABSENT from the manifest (each named)
  [PASS] every def-only entry sits on its own family's sentinel
  [PASS] no (block, offset, type) triple repeats across all 258 entries
  [PASS] find_entry resolves every entry to itself
  [PASS] every section is contiguous in registry order      8 sections
  [PASS] a Dome write raises exactly its own console bit    0 -> 0x1
  [PASS] a Dome write raises exactly its own console bit    4 -> 0x2
  [PASS] a Dome write raises exactly its own console bit    8 -> 0x4
  [PASS]   the take clears the mask, each time
  [PASS]   and NO blanket re-speak is raised (the sky is not re-seeded)
  [PASS] the three writes landed in ORB_CONSOLE_LIVE   dome=640 size=4.5 noise=0.42
  [PASS] three writes coalesce into one mask of three bits  0x7
  [PASS] all three Dome dials still read BOUNDARY cadence
  [PASS] each floor is exactly one step, authored value still reachable  13/13
  [PASS] no sentinel-backed row can be dialled to 0 any more
GREEN — 0 failure(s)
```

One harness bug found and fixed in the harness, not the tree: the first
grid check used a 1e-6 tolerance and failed six rows whose residual is
2e-6 — float representation noise (`0.01f` is `0.00999999977648…`). A
window that tight measures IEEE-754, not the grid.

### The rest of the harness family, at P1 close

| gate | verdict |
| --- | --- |
| `tools/gates/console_gate/run.py` | **PASS** — cartridge.hpp and console.hpp type-check with zero diagnostics; this is the gate that reads every edit above |
| `tools/gates/glaw2/run.py` | **GREEN** — 294 fn, 271 const, 81 struct, 86 binding, 65 entry points |
| `tools/gates/sha256_gate/run.py` | **PASS** |
| `tools/binding_gen.py --check` | PASS on every relation; S-6 (commit integrity) is red only while HEAD is ahead of upstream, by construction |
| `tools/organ_gap.py --gate` | **PASS** — 0 surviving runtime readers across 11 graduated pairs |
| `tools/gates/score/run.py` | **RED, 4 violations — PRE-EXISTING.** Verified by `git stash`: identical at 85b1cd6. `phase_motion_corral` missing from the spine; ribbon's F8 door and orbs' boot config ungated in the manifest; `phase_live_card_write` ungated but not FOUNDATIONAL. Not ORGAN_4's, not repaired here — the spine table is a different jurisdiction. Flagged for the next sitting that opens `UPDATE_SPINE`. |
| `tools/wgsl_gate.py` | FAIL, environmental — `naga` is not on PATH in this container. The gate reports unrunnable as failed by design (P1). No WGSL was touched this campaign. |

## P2 — the ribbon seam

Wave 4's ribbon row named the anatomy and left it: `DriverSurface::Ribbon
{ rest_amp_lat, rest_amp_vert, rest_tint_stim[3], rest_tint_mix, gain }`.
That is the anatomy that landed, field for field. What Wave 4 was missing
was one read, and this is it.

**The fourth pipe's blocker, read rather than worked around.** `color_stim`'s
fallback is a NULL POINTER, so the row asked what a null branch's rest
SHAPE is. The answer was already in the line below it:
`const float s = st ? st[c2] : 0.0f`. The code has always assumed `{0,0,0}`;
`PARAM_LAYOUT` says the same thing independently — `{ "ribbon.color_stim",
6, 3, 0.0f }`. Two witnesses, one shape, no guess.

**The rests ARE the old fallbacks.** `{ 1.0f, 1.0f, {0,0,0}, 0.0f, 1.0f }`
— `PARAM_LAYOUT`'s four rest columns and the seam's four hardcoded
fallbacks agree, which is what made this a rest-and-gain seam rather than
a redesign. One gain for four pipes, as fog's density and colour share
one: the four are one gesture, the canvas moving a ribbon.

The room is **60 → 88 bytes** (22 words: fog 5, aura 4, checker 6, ribbon
7), and the whole-struct `static_assert` the room never had is added with
it — the canvas precedent, which bites, rather than the tautological form.

### Byte-stability, measured rather than claimed

The handoff's sentence is *"gain 1 and the shipped rests reproduce today's
arithmetic exactly."* Half of it is trivially true and half needed a
number, so both were measured:

```
  [PASS] rest 0, gain 1: byte-identical (stim and mix)
         4718592 samples, 0 differences
  [PASS] rest 1, gain 1: byte-identical across the pipe's domain
         1310720 samples over [0.5, 16], 0 differ
         (over the full sweep [1e-9, 32]: differences exist, ALL below
          0.0078 — d − 1 loses the low bits only when d is far under the
          pipe's floor. The swell coupling's multiplier rests at 1.0 and
          its ceiling dial floors at 1.0, so the domain is [1, ceiling].)
  [PASS] gain 0 returns the curator's rest exactly, for every driven value
```

The stim and mix pipes need no Sterbenz argument at all — their rests are
0, and `0 + 1·(d − 0) == d` is exact for every float, the same argument the
checker seam got for free. The two amp pipes rest at 1.0 and so do need
one, and the window is stated above with its evidence instead of assumed.

### No witness, and that is the finding (D2)

C4 asked whether a FIXED CPU home holds the last-uploaded amp/colour
quartet. It does not: they live at `rs.gpu[rs.rendered_slot]`, and
`rendered_slot` moves with eviction and reads `UINT32_MAX` when nothing is
rendered. **A witness needs a home; a varying slot is not one.** Minting a
mirror to give it one is what windows-not-homes forbids, so the section
carries five dials and no meter — and says so in its own banner, where the
next reader will look.

Entries **258 → 263**. `organ_gap` now reads `DriverSurface 4/4 named`.

## P3 — the backlog sweep

**Six landed, one retired unbuilt, one sharpened.** Entries 263 → **305**;
blocks 10 → 12.

### P3a — four structural witnesses

Per C6, all four author into `GPUDesignConfig`, so the block the panel
already addresses IS their fixed home and D2 is satisfied without minting
anything. Sections by SUBJECT, not by home:

| fact | section · group | why a meter and not a dial |
| --- | --- | --- |
| `veil_strength` | Atmosphere · Veil | `cartridge.hpp:1206` writes `finite_mode ? 0 : 1` — a world-shape fact. A slider would fight that author and lose on the next world draw. |
| `terrain_amp_ceiling` | Terrain · Indoor | composed by the mood applier from MoodProfile's STRUCTURAL group, which is C5: world generation reads it, so a dial would edit a world already built. |
| `ceiling_height` | Terrain · Indoor | the same, and it is also a camera clamp. |
| `indoor_height_cap` | Terrain · Indoor | the same, read once per zone birth by `zone_derive_params`. |

The three indoor meters sit beside two GEN dials in one group, which is
legible only because ORGAN_3b moved the cadence chip from the group's NAME
to the ROW. A DRIVEN meter under a heading that once read *"edits the next
spawn"* would have been the misreport this campaign exists to end.

### P3b — POSSESSION_RADIUS, and the pair that stopped being a pair

C7's first branch: `POSSESSION_RADIUS_SQ` had exactly one reader. So the
constant is retired outright and the one site squares the LIVE value:

```cpp
const float reach = PANEL_LIVE.possession.radius;
float best_d2 = reach * reach;
```

The graduation went to `contracts/control_panel.hpp` rather than to a new
header, on that file's own argument — *"a second contracts header per
family would be two homes for one idea"* — and because possession IS input
grammar, which is what the file says it holds. `bodies/agents.hpp` could
never have carried the dial: the ORGAN may not include a body.

Range 0…80 is 4× the authored 20 `[heuristic]`, and the floor is one step:
a zero reach makes the search unwinnable rather than merely small. A
`static_assert` states that beside the design value, the ribbon `r_min`
precedent applied to the second divisor-shaped fact in the tree.

### P3c — floater_coordination: the ledger's own premise was wrong

Wave 4 filed it as *"driven by `cube_behaviors.hpp:400`"* and deferred it
because the module was unread. The module is read now, and **there is no
driver.** Line 400 sits inside `cycle_floater_coordination`, a player
command: it steps `cbs.coordination_step` through the three values of
`FLOATER_COORDINATION_STEPS` and calls `stage_floater_coordination(v)`,
which assigns `config_.floater_coordination` and raises no dirty bit. Its
readers are `world.wgsl:8845` (per frame) and `cartridge.hpp:1143`, where
`phase_motion_drivers` reads it as the beacon's S gain — both READS.

So P3c's first branch does not apply: **a rest-and-gain room for it would
model a driver that does not exist.** Per the handoff's second branch the
anatomy is ledgered and the row is not landed. The anatomy, priced:

- The fact already has a fixed home the panel addresses —
  `GPUDesignConfig.floater_coordination`, block 0 — so a plain
  `ORGAN_PARAM` line is the whole build. No bank, no block, no witness.
- What it needs first is a **temperament ruling**, and it is exactly the
  one P1e stamped for the palette: the key-cycle is a live gesture and the
  dial would be the durable author, or the reverse. The three authored
  steps (0 / 0.5 / 1.0) are a vocabulary the cycle walks; a continuous
  slider over the same range is a strictly larger space, which is either
  the point or a lie about what the world was designed to do.
- D1 reserves that choice. One line lands it the moment it is made.

### P3d — the destructive banks

**Two banks born, thirty-seven rows, no wiring anywhere.** The Wave-3
recipe verbatim: LIVE bank beside its table, `ORGAN_PARAM_GEN` on every
row, the banner naming the next natural event, and NOT ONE flag raised —
because a wrong re-speak tears down a world (D5, and 3b's D4 behind it).

**`WORLD_DRAW_LIVE`** — `contracts/mood_constants.hpp`, block 10, 11 rows.
`PORTAL_DENSITY`, `FINITE_OUTDOOR_CHANCE` and `SCHEME_WEIGHTS` were
`inline constexpr` in `direction/mood.hpp`, which the ORGAN may not
include; the portal palette was already in mood_constants beside the
`PortalDestination` it describes. One bank rather than two, because they
are one question — *what does a fresh world roll?* Rows split by SUBJECT
per the ledger's own placement: `Sky & Light · Schemes` (4),
`Sky & Light · Portals` (5), `Agents · Portals` (2). `SCHEME_COUNT`
travelled with the weights it sizes; the two tables it also sizes stay in
`mood.hpp` and read it unqualified, unchanged.

One reader tightened while it was open: `pick_portal_mood` read
`FINITE_OUTDOOR_CHANCE` four times across one ladder. It reads the live
value ONCE into a local now, so a write landing mid-draw cannot make the
four branches disagree about the same roll.

**`RIBBON_SPAWN_LIVE`** — `contracts/ribbon_surface.hpp`, block 11, 26
rows. A SECOND bank in the same file as `RIBBON_LIVE`, and the reason is
temperament: the head law is read every frame, and every row here is read
ONCE, as a ribbon is drawn. `RibbonConfig` dies with its two members —
two static constants were its whole body. `RIBBON_SMOOTH_PALETTE_COUNT`
and `RIBBON_COLOR_MODE_COUNT` moved to contracts because they SIZE the
bank's rows, and a `static_assert` ties `RibbonColorMode::COUNT` to the
second so a fourth colour mode cannot be born without its weight.

**The floater tier weights did NOT land, and the reason is the campaign's
own law.** `SPHERE_BASE_TIER_WEIGHTS` and `CUBE_BASE_TIER_WEIGHTS` have
**no reader**: `git grep` finds each symbol exactly once in `src/`, at its
own declaration. The generic pipeline rolls a floater's tier from
`adapter.get_tier_profile(t).weight` — the `weight` column of
`SPHERE_TIERS` / `CUBE_TIERS` — multiplied by `theme_tier_weights()`
(`entity_pipeline.hpp:110-116`). The base arrays were superseded and left
standing. Enrolling them would have built **two dead dials on purpose**,
which is the exact defect ORGAN_4 exists to end. Their share of the row is
retired; the constants themselves are a broom question for whichever
sitting next opens `contracts/floaters.hpp`, and they are named here so it
does not have to rediscover them.

### The harness at P3 close

```
  [PASS] the entry tally is the counted one              305 (want 305)
  [PASS] twelve block ids (two destructive banks born)
  [PASS] block 10 -> WORLD_DRAW_LIVE / block 11 -> RIBBON_SPAWN_LIVE
  [PASS] both banks seed byte-for-byte from their design tables
  [PASS] every destructive-bank row reads cadence GEN            39 rows
  [PASS] a destructive write LANDS in its bank
  [PASS]   and raises NO re-speak flag of any kind (D5)
  [PASS] the four mood-structural facts meter as DRIVEN witnesses  4/4
  [PASS] organ_set refuses to write a witness                 4 refusals
  [PASS] the possession reach enrolls, floors at one step, lands, and
         clamps 0 up to the floor rather than zeroing the reach
  [PASS] no (block, offset, type) triple repeats across all 305 entries
  [PASS] every section is contiguous in registry order        8 sections
GREEN — 0 failure(s)
```

`organ_gap` reads **13 graduated pairs, 0 surviving runtime readers** —
`WorldDrawSurface 5/5 named`, `RibbonSpawnSurface 21/21 named`. Its file
table and its pair table both grew, so neither bank is scanned by silence.

## P4 — the reader proof (`tools/organ_readers.py`)

The audit family's newest check, and the organ_gap sibling: stdout only,
**exit 0 always**. `organ_gap` measures the gap between the HOMES and the
panel — which declared members no dial names. This measures the inward
gap: of the dials that DO exist, which write a field nobody reads.

### The match is HANDLE-QUALIFIED, and that is the whole tool

A bare-token match cannot answer this campaign's own flagship question.
`configure_orbs` CONTAINS the token `motion_rule` — in the line
`gpuCfg.motion_rule = os.current_motion_rule;`, which reads the PLAYER's
rule and never the mood's. **A tool that matched the bare token would have
passed the deadest row in the tree.**

So a row is proved only when its leaf is reached through a name that IS
the bank: the LIVE symbol, a parameter whose type is the bank's struct
(`cfg`, `m`), or a reference alias bound to either (`const auto& S =
RIBBON_SPAWN_LIVE`), resolved to a fixed point because aliases chain. The
gap between handle and leaf admits member and index characters only, so a
match is one EXPRESSION and never one line.

### The positive control — a gate that cannot fail is decoration

Run by re-adding the two rows this campaign had just retired:

```
  SUSPECT     1
  ORB_MOOD       motion_rule                   Sky & Light · Orb mood
        MECHANICAL? — named outside the declared readers at:
          …/bodies/orbs.hpp:559  gpuCfg.motion_rule = os.current_motion_rule;
```

`motion_rule` came back a suspect **with the evidence for its ruling
quoted in its own second pass**. `flock_gesture_default` came back PROVED
— correctly: `apply_mood_first_run_defaults_` really does read it, and its
row died for a CADENCE reason, not a reader one. Separating those two is
the only thing that makes the instrument worth having.

### The second pass is what makes a suspect actionable (D6)

For every row the declared readers miss, the tool searches all of `src/`
for the same token, drops the field's own declaration, and prints what is
left:

- **MECHANICAL** — named somewhere the table does not list. The table is
  stale or the reader moved. Fix now.
- **SEMANTIC** — named nowhere but its own declaration. The dial writes a
  fact nothing reads. Ledger the anatomy; Jean and Claude rule.

### The run against the tree ORGAN_4 leaves

```
  proved    231   a declared reader names the field
  SUSPECT     0   no declared reader names it
  witness    17   an _RO meter: the question is inverted (blind spot 5)
  scope      57   GPU-side or whole-struct (blind spots 2, 3)
```

**Zero suspects, and the number is only worth what the control is worth.**
Nothing needed a mechanical fix and nothing needed a ruling — P1b had
already retired the five the instrument would have caught, which is the
campaign working in the order it was written rather than the tool finding
nothing to find.

The table's own census pass names four functions it does not list —
`mood_def`, `organ_flush` (×2) and `init_renderer`. All four hand a whole
row on or return a reference; none reads a field, so none belongs in the
table. They are printed anyway, marked `?`, because a reader the table
forgot looks exactly like this until someone reads it.

### What it deliberately cannot see

Fifty-seven `CONFIG` rows are OUT OF SCOPE, named as such rather than
reported: `config_` ships whole and is read in `world.wgsl`. Proving
GPU-side consumption is the kernel's ledger — a sixth instrument, priced
here and not built: it would parse `world.wgsl`'s uniform structs and ask
the same question of the shader bodies, and it is the one thing that could
have caught `base_hue`/`hue_variance` mechanically instead of by Jean's
eye. Seventeen witnesses are skipped because their question is inverted: a
meter asks who AUTHORS, and that is the contest instrument's job.

## P5 — the fifth ledger (`audit/ORGAN.md`)

`tools/organ_ledger.py` emits it; the header names its generator and its
regen command, the audit family's convention. BINDING, COMMAND, MANIFEST
and MIRROR each keep a book about one of the program's rooms; the organ
had two instruments and no book, and now has one.

**Two reasons, and the second is the larger.** SEARCHABILITY — *"what is
the range on the cohesion radius"* should be answerable by grepping a file
in the tree, not by building the program and opening a panel. And **the
coupling menu**: a coupling is a parameter set into trajectory over time,
so every row with an authored range IS a trajectory domain. The `range`
column is the domain a trajectory would play over and the `cadence` column
says whether playing it would be heard now, at the boundary, or at the
author's next event. That makes this table the music campaign's target
map rather than panel decoration.

### The derivation is restated once, and then CHECKED against the C++

`derived_cadence()` lives in `organ_registry.hpp`; the generator restates
it in Python, which is one more copy of a rule than the compiled-registry
law likes. So the harness prints the same tallies from the COMPILED table
and the two are compared rather than trusted:

```
      cadence   live 120 · gen 39 · boundary 129 · driven 17
      def kind  NONE 179 · MOOD 5 · TIER 32 · BEHAVIOR 70 · ORB_MOOD 19
      witnesses 17 · doors 3 · blocks 12
  [PASS] every row lands in exactly one cadence
  [PASS] witness count equals the DRIVEN count (a meter's cadence)
```

`audit/ORGAN.md` reports the same numbers, field for field. The door table
is PARSED out of `kOrganDoors` rather than restated, for the same reason.

### The tallies at close

| | |
| --- | --- |
| entries | **305** |
| by section | Agents 104 · Ribbon 56 · Terrain 42 · Sky & Light 37 · Atmosphere 23 · Interaction 21 · Pawn 18 · Debug 4 |
| by cadence | live 120 · gen 39 · boundary 129 · driven 17 |
| by macro form | PARAM 123 · PARAM_DEF 105 · PARAM_DEFONLY 21 · PARAM_GEN 39 · PARAM_RO 17 |
| definition kinds | NONE 179 · MOOD 5 · TIER 32 · BEHAVIOR 70 · ORB_MOOD 19 |
| blocks and sentinels | 12 blocks + two sentinels (255 MoodProfile, 254 OrbMoodConfig) |
| namespaces | the_board 290 · canvas 15 |
| doors | 3 |

**The GEN set is 2 → 39.** ORGAN_3b sized it at two and said the machinery
"carries the rest the moment they land, at one macro token each." It did:
thirty-seven rows landed at P3d and not one line of cadence machinery
changed.

The book closes with the verbatim tails of both check tools under
`## THE GAP` and `## THE READERS`, so one file answers what the panel
names, what it does not, and whether the readers agree. Written LF, no
BOM, single trailing newline, proved by byte-level read-back — the
`binding_ledger` G2-eol precedent.

## P6 — the preset layer (`web/presets/`)

**A scene is a file, a boot is a choice.** `presets/index.json` lists what
is on the shelf; `?preset=<name>` picks one at boot and a select in the
panel header picks one by hand. Both walk the SAME import path the file
picker already walked, so **zero new write machinery exists here** — a
partial file applies exactly what it carries, definitions raise their
flags, instances write and mark.

### The one thing the handoff's shape would have got wrong

`organ_panel.js` opens with `if (…get('organ') !== '1') return;`. Building
the preset loader inside `build()` — the obvious place, and where the
handoff's wording points — would have made a preset reachable **only by
someone already holding the panel open**, which is the exact opposite of
what an exhibition boot is for. São Paulo wants the design, not the
instrument.

So the gate now has two reasons to wake and the loader lives at MODULE
scope:

```js
var WANT_PANEL  = Q.get('organ') === '1';
var WANT_PRESET = Q.get('preset');
if (!WANT_PANEL && !WANT_PRESET) return;
```

The audience path keeps its promise exactly: with neither flag the file
still returns on its first statement — no DOM, no stylesheet, no timer, no
ccall. Proved below rather than asserted.

`applyFile` therefore reads the MANIFEST rather than the panel's rows, and
moves a widget only when a row happens to exist. One road, three doors:
the URL, the select, the import button.

### The shelf ships

`tools/web_dist.py` copied five named artifacts and the exhibition; a
folder the panel fetches by relative path would simply not have been
there. It now copies `web/presets/*.json` into `dist/presets/`, weighs them
under the same per-file cap, and COUNTS them — the poster precedent, so
the file count it prints is not a lie. Verified end to end against stub
build outputs: `presets (dist) 10456 bytes, 2 files`, `dist/presets/`
holding both.

### `baseline.json` — captured, and honest about what it is not

Captured by running **the panel's own export walk** (`collect(null)`)
natively against the C++ boot values: 126 definitions keyed `<mood>/` or
`world/`, 105 bank instances keyed by id, 17 witnesses skipped because a
meter is not a setting. **231 keys.**

What it deliberately omits: the **57 boot-pinned `GPUDesignConfig` rows**.
`initializeState` writes them at device init and no offline capture can
run it, so fabricating them would have shipped a preset that zeroes the
config on load. The index entry says so in its `note`, which the select
shows as the option's title. A complete scene comes from the panel's own
export — which is what P6c tells Jean to do anyway.

### The harness — the SHIPPED panel, under a shim, on the REAL manifest

`organ_manifest()` was dumped from the compiled registry and fed to a
hand-rolled DOM; `web/organ_panel.js` was loaded unmodified. Four modes:

```
?organ=1&preset=baseline   panel built · 305 dials · both orb doors render
                           · select in the bar, lists "baseline", carries the
                           note as its title · choosing resets it (a verb,
                           not a state) and walks the same write path
                           · 231 applied, 0 unknown, 0 rejects
?preset=baseline           NO PANEL, no DOM, never says "panel up"
                           · 231 applied, 105 inst / 126 def, 0 rejects
                           · ORBS.dome_radius = 500
(no flag)                  no DOM, no fetch, no timer, no ccall,
                           the program untouched
?preset=probe (synthetic)  2 applied, 1 unknown COUNTED not thrown,
                           1 witness NAMED not rejected, 0 rejects,
                           and the real dial still moved to 777
```

Every mode GREEN, and `localStorage`/`sessionStorage` were instrumented to
count accesses: **0**. The artifact rule holds — a preset is chosen per
boot or per click, and the URL is the only thing that carries a choice
between sessions.

### P6c — the sentence for Jean

> Design a scene on the panel. Press **export** (or a section's own export
> for one voice). Drop the JSON in `web/presets/`, add one line to
> `web/presets/index.json` — `{ "name": "dusk", "file": "dusk.json" }` —
> and São Paulo boots into it with `?preset=dusk`. No panel needed at the
> exhibition; `?organ=1` is only for the desk.

---

## ORGAN_4 AT CLOSE

### The tallies

| | at open | at close |
| --- | --- | --- |
| entries | 263 | **305** |
| dials / witnesses | 250 / 13 | **288 / 17** |
| blocks | 10 | **12** |
| doors | 1 | **3** |
| by cadence | live 114 · gen 2 · boundary 134 · driven 13 | **live 120 · gen 39 · boundary 129 · driven 17** |
| definition kinds | none 132 · MOOD 5 · TIER 32 · BEHAVIOR 70 · ORB_MOOD 24 | **none 179 · MOOD 5 · TIER 32 · BEHAVIOR 70 · ORB_MOOD 19** |
| graduated pairs (organ_gap) | 11 | **13** |
| audit books | 4 | **5** |
| check tools | 1 (`organ_gap`) | **2** (`organ_gap`, `organ_readers`) |
| GPU wallet | — | **UNTOUCHED**: zero buffers, zero bindings, zero WGSL |

By section at close: Agents 104 · Ribbon 56 · Terrain 42 · Sky & Light 37
· Atmosphere 23 · Interaction 21 · Pawn 18 · Debug 4.

**The handoff predicted 257 and the tree carries 305.** Both halves of the
gap are accounted for: it sized the retirement from a stale 262 when the
tree already held 263 (ORGAN_3b P5's `PANEL.beacon.s`), and its expected
shape counted the five deletions without counting its own additions —
P2c's five, P3a's four, P3b's one and P3d's thirty-seven.

### Every row-death this campaign caused

| row | phase | fate |
| --- | --- | --- |
| `OrbMoodConfig.base_hue` | P1b | RETIRED — dead by construction; field docketed on L26 |
| `OrbMoodConfig.hue_variance` | P1b | RETIRED — same branch, same fate |
| `OrbMoodConfig.hue_converge_target` | P1b | RETIRED — field STAYS; the row returns with a gen-2 `color_converge` coupling |
| `OrbMoodConfig.motion_rule` | P1b | RETIRED — dead tree-wide; field docketed; its reachable form is door 1 |
| `OrbMoodConfig.flock_gesture_default` | P1b | RETIRED — field STAYS; a boot-only fact cannot wear a boundary chip; its reachable form is door 2 |
| `POSSESSION_RADIUS_SQ` (a constant, not a row) | P3b | RETIRED outright — the square is derived at its one read from the live value |
| `RibbonConfig` (a struct, not a row) | P3d | RETIRED — two static members were its whole body; both graduated |
| `SPHERE_BASE_TIER_WEIGHTS` / `CUBE_BASE_TIER_WEIGHTS` | P3d | NOT ENROLLED — no reader anywhere; enrolling them would have built two dead dials on purpose |

### Every DEFER row this campaign killed

| row | phase |
| --- | --- |
| the ribbon's four pipes (Wave 4) | P2 |
| `veil_strength`, `terrain_amp_ceiling`, `ceiling_height`, `indoor_height_cap` (Wave 4) | P3a |
| `POSSESSION_RADIUS` (Wave 2 — the last of Wave 2) | P3b |
| the remaining destructive banks (Wave 3 — the last of Wave 3) | P3d |

`floater_coordination` survives with a **corrected** reason: Wave 4 called
it driven; it is player-cycled, and what it needs is a temperament ruling
D1 reserves. Four of the nine surviving rows died; five remain, and
Waves 2 and 3 are now empty.

### What ORGAN_4 built

| | |
| --- | --- |
| one idiom | the CONSOLE MASK — a CPU bank whose reader is an event gets a per-field mask, and the boundary routes each field to its own reader's cadence |
| two doors | `Cycle orb rule`, `Cycle orb gesture` — the reachable form of a player-owned fact |
| one seam | the ribbon's four pipes, rests and one gain, byte-stable at the seeds |
| two banks | `WORLD_DRAW_LIVE` (block 10), `RIBBON_SPAWN_LIVE` (block 11) — destructive, GEN on every row, no wiring |
| one graduation | `POSSESSION_RADIUS` into `contracts/control_panel.hpp`, its `_SQ` twin retired |
| four witnesses | the mood-structural facts, metered against the block the panel already addresses |
| two instruments | `tools/organ_readers.py`, `tools/organ_ledger.py` |
| one audit book | `audit/ORGAN.md` — the fifth, and the music campaign's coupling menu |
| one layer | `web/presets/` — a scene is a file, a boot is a choice |

### The harness at close

Compiled and RUN natively against the real headers under the
console_gate's pinned surface — 53 assertions, **GREEN, 0 failures** —
plus the shipped panel driven under a shim in four query modes, all GREEN.

| gate | verdict |
| --- | --- |
| `tools/gates/console_gate/run.py` | **PASS** — cartridge.hpp and console.hpp, zero diagnostics |
| `tools/gates/glaw2/run.py` | **GREEN** |
| `tools/gates/sha256_gate/run.py` | **PASS** |
| `tools/organ_gap.py --gate` | **PASS** — 13 graduated pairs, 0 surviving runtime readers |
| `tools/organ_readers.py` | 231 proved, **0 suspects**, 17 witnesses, 57 out of scope |
| `tools/binding_gen.py --check` | PASS on every relation |
| `tools/gates/score/run.py` | **RED, 4 violations — PRE-EXISTING** at 85b1cd6, proved by `git stash`. Spine-table jurisdiction, flagged for the next sitting that opens `UPDATE_SPINE`. |
| `tools/wgsl_gate.py` | unrunnable here — no `naga` on PATH. No WGSL was touched. |
| `tools/mirror_census.py` | NOT RUN — frozen at ML-1 per `docs/OPEN.md` |

### What a successor should not trust silently

- **`audit/ORGAN.md` is generated.** Regenerate with
  `python3 tools/organ_ledger.py` after any `.inc` edit, or the book and
  the panel disagree.
- **`organ_readers.py`'s reader table is hand-authored.** It prints itself
  and prints the functions its own census finds that it omits; a row
  reading `proved` against a stale table is the failure mode it was built
  to make visible rather than to make impossible.
- **The kernel's ledger does not exist.** GPU-side consumption is out of
  scope for both instruments. `base_hue` was found by Jean's eye, not by a
  tool, and nothing in the tree would have caught it. That instrument is
  the honest next spend.
- **`baseline.json` is a partial preset by construction** — the 57
  boot-pinned `GPUDesignConfig` rows are written by `initializeState` at
  device init, which no offline capture can run. The index note says so.

---

# ORGAN_5 — THE RULE MADE VISIBLE

**The laws of this campaign:** a dial whose effect depends on a mode
stands next to a truthful readout of that mode; an author re-speaks no
more than the edit requires; and a section is organized for the hand that
plays it, not the struct that stores it.

## P0 — the census

### The stale-authority gates: five green, one MATERIALLY DIFFERENT

| gate | verdict |
| --- | --- |
| 1 · `cartridge.hpp` boundary order | **GREEN**, with one immaterial note. The order is doors → *mood-def take → tier-def take* → console mask → orb-def take → `configure_orbs(orbs_state_, &orbs_deps_, ORB_MOOD_LIVE[mood_state_.active % MOOD_COUNT], queue)`. The two intervening takes are independent flags the handoff's quote simply did not enumerate; every ordering the quote asserts holds. |
| 2 · `configure_orbs` tail and callers | **GREEN**. Tail is `upload_orb_config(queue, gpuCfg); os.init_pending = true;` (then the diagnostic print). Exactly two callers: `cartridge.hpp:1480` (the boundary) and `direction/mood.hpp:701` (the mood apply). |
| 3 · `.inc` orb sections, doors | **GREEN**. Dome 3 · Orb mood 8 · Orb flock 11; `ORGAN_DOOR_COUNT = 3` with three `kOrganDoors` rows. |
| 4 · `write_definition` routes `ORB_MOOD` | **GREEN**. `else if (e.def_kind == ORGAN_DEF_ORB_MOOD) { g_orb_def_dirty = true; }`. |
| 5 · `web/index.html` panel script | **MATERIALLY DIFFERENT — see below.** |
| 6 · `upload_orb_speed_mult` | **GREEN**. A 4-byte partial at `offsetof(GPUOrbConfig, speed_mult)` (state.hpp:3559). |

### GATE 5 — STOPPED ON THE ITEM, AND WHY THE CAMPAIGN CONTINUES

The handoff's premise is that `web/index.html` references `organ_panel.js`
**without** a version query, and P5a's job is to add one. The tree says
otherwise:

```html
var BUILD = '__BUILD_ID__';
…
op.src = 'organ_panel.js?v=' + BUILD;      // index.html:1030
```

**The panel is already versioned, and has been since the script was first
loaded there.** P5a's first half is not work to do; it is work already
done. D6 says a material difference STOPS — so this item stops and is
reported rather than executed, and the rest of the campaign proceeds under
FLAG-AND-FINISH, because the difference makes the prescribed edit
*unnecessary*, never *unsafe*.

P5a's second half — `window.T7_BUILD_ID = "__BUILD_ID__";` — would have
been actively wrong. The shell states its own law three lines above the
placeholder:

> *"The placeholder appears exactly ONCE in this file, on the next line, so
> the substitution has one target and the dist script's refusal-to-ship
> check has one thing to count."*

A second literal `__BUILD_ID__` would break that invariant to obtain a
value the file **already holds in a variable**. P5b's goal — the footer
naming the build — is reached by `window.T7_BUILD_ID = BUILD;`: one line,
no second placeholder, the law intact. That is what P5 lands.

### The mirror branch — deleted locally, REFUSED remotely

`git branch -D claude/campaign-handoff-3h50qd` succeeded. The remote
delete does not: the session's git relay answers **HTTP 403** to a delete
refspec, in every form (`--delete`, `:ref`, `+:refs/heads/…`), while
ordinary pushes to master succeed. This is an environment policy, not a
repository state, and no GitHub tool in this session exposes ref deletion.

**One command for Jean, from his own machine:**
`git push origin --delete claude/campaign-handoff-3h50qd`

### C1 — `OrbMoodConfig` offsets, and the reseed mask

`sizeof(OrbMoodConfig)` is **108 bytes = 27 words**, so the highest bit any
field can claim is **26** — every offset fits a `uint32_t` mask with five
bits to spare.

| field | offset | bit |
| --- | --- | --- |
| `enabled` | 0 | **0** |
| `count` | 4 | **1** |
| `drag` | 20 | **5** |
| `palette_id` | 48 | **12** |

`ORB_RESEED_BITS = 0x00001023`.

The other twenty-one: `base_hue` 2 · `hue_variance` 3 · `brightness` 4 ·
`motion_rule` 6 · `rotation_speed` 7 · `rotation_axis` 8 ·
`orbital_base_speed` 11 · `hue_converge_target` 13 · `tierset_id` 14 ·
`flock_sep_radius` 15 · `flock_align_radius` 16 · `flock_coh_radius` 17 ·
`flock_sep_weight` 18 · `flock_align_weight` 19 · `flock_coh_weight` 20 ·
`flock_max_speed` 21 · `flock_gesture_default` 22 · `rule_drag_*` 23–26.

**One property the mask depends on, stated because it is not obvious.** A
bit is the ENTRY's offset ÷ 4, not every word the entry writes:
`rotation_axis` is a VEC3 covering words 8–10 and raises bit 8 alone. That
is sound here and provably so — it is the struct's only multi-word entry,
and words 9 and 10 belong to no other field, so no write can silently
raise or miss a reseed bit. A future VEC3 whose later lanes overlapped a
reseed word would break that, which is why the harness pins the four
reseed offsets rather than trusting them.

### C2 — how `organ_mood()` gets its answer

`organ_mood()` → `current_mood()` → `g_mood ? *g_mood : 0`, where
`g_mood` is a `const uint32_t*` bound ONCE at boot by
`cartridge.hpp:751 — bind_mood(&mood_state_.active)`. **The registry
borrows a pointer to the spine's own field and keeps no copy**, so the
panel can never show a mood the program has left. That is the pattern P2's
rule window copies — with one difference the rule forces: the rule lives in
`OrbsState`, which the registry may not include (a contract may not reach a
body), so the window is a `uint32_t` the CARTRIDGE writes rather than a
pointer the registry follows. Same law — one home, no second truth —
different plumbing, because the home is one tier further away.

### C3 — the shell's two hook points

- **The poll**: one `setInterval(…, 250)` in `build()`. It refreshes every
  witness meter, every contest marker, and the footer's status line.
- **A door press**: `b.addEventListener('click', function () { C.door(d.i); })`
  in the door strip, one handler per manifest row, name-blind.

### C4 — the U32 write path, end to end

Both widgets already route through **one** `push`:

```js
var set = function (src) {
  return function () {
    v[0] = clamp(parseFloat(src.value) || 0, p);
    push(p, v); sl.value = v[0]; nm.value = v[0];
  };
};
sl.addEventListener('input', set(sl));
nm.addEventListener('input', set(nm));
```

**Two touch-hostile properties, both mechanical and both provable without
a phone:**

1. **`parseFloat(src.value) || 0` commits `p.min` for an EMPTY box.** An
   empty field parses `NaN`, `|| 0` makes it `0`, and `clamp` snaps to the
   floor. Clearing a number box before typing is the ordinary way to enter
   a value on a touch keyboard — so the first keystroke of every phone
   edit writes the minimum. On `count` (min 0) that is `count = 0`, and
   `configure_orbs` **early-returns** on `os.count == 0`: the dome goes
   inactive mid-edit. This is the most economical explanation of
   *"count works on desktop but not his phone"* that the tree can supply
   without the device.
2. **The handler writes `nm.value` back into the box being typed in.** A
   focused number input reassigned on every keystroke fights the caret on
   touch keyboards. There is also no `change`/`blur` commit at all — the
   number box has exactly one path, and it fires per keystroke.

**A reject counter already exists and is already shown.**
`organ_rejected_count()` is cwrapped as `C.rejects()` and the footer reads
`· rejected N`. Per D2, nothing is added: the instrument P5b asks for is
in the tree.

### C5 — `dispatch_orb_dynamics` and the dt/t question

`dispatch_orb_dynamics` calls
`upload_orb_frame(queue, c->time_state_.dt, c->time_state_.seconds)`
immediately before the compute pass, **every frame**.

The handoff asks that the one-frame dt/t stomp be named and not fixed. The
census finds there is **no stomp to name**, and the reason is the frame
order rather than luck: `pawn.cpp:190` runs `organ_flush` at the HEAD of
the frame — before input, before update, before render — and
`phase_orb_sky` runs in the RENDER spine. So a full `upload_orb_config`
zeroes `dt`/`t_seconds` in the uniform and the SAME frame's dispatch
re-authors both before any kernel reads them. When the dome is inactive
the dispatch early-returns, and nothing reads the uniform at all. The
comment P1 lands says exactly this, which is the honest version of what
the handoff asked to have said.

### C6 — `speed_mult`'s whole anatomy

**The expected branch holds: the gen-1 writer is retired and nothing live
moves the value.**

| site | what it is |
| --- | --- |
| `orbs.hpp:281` | `float speed_mult_current = 1.0f;` — the declaration |
| `orbs.hpp:602` | `os.speed_mult_current = 1.0f;` in `teardown_orbs` — a RESET, not an author |
| `orbs.hpp:484` | `gpuCfg.speed_mult = os.speed_mult_current;` in `pack_flocking_`'s tail — the only READ |
| `state.hpp:3559` | `upload_orb_speed_mult` — **zero callers anywhere in `src/`** |

No live writer. The comment beside the field still promises a smoother
("Smoothed on the CPU, uploaded via `upload_orb_speed_mult` only when it
moves") that no longer exists; the field has been pinned at 1.0 since the
gen-1 coupling retired. **D5 does not fire — P3c's retirement is on.**

**The kernel's read, checked rather than assumed.** `speed_mult` scales the
energy source of each rule that HAS one:

| rule | site | what it scales |
| --- | --- | --- |
| BROWNIAN (0) | `world.wgsl:13390` | the noise injection into velocity |
| ORBITAL (1) | `world.wgsl:13436` | the orbital angular speed |
| FLOCKING (3) | `world.wgsl:13548` | `eff_max = flock_max_speed * speed_mult` |
| FROZEN (2) | — | **nothing, and correctly.** The frozen arm has no energy term: it only bleeds velocity through `rule_drag_frozen`. Stillness is the rule's defining property, so a master energy dial has nothing there to master. |

So "all rules" is true in the only sense that means anything: every rule
that produces motion has that motion scaled. The dial's banner says so in
those words rather than in the looser ones.

## P1 — the upload-only path

**An author re-speaks no more than the edit requires.** Before this, every
notch of every one of the nineteen orb-mood dials ran `configure_orbs`,
which ends `os.init_pending = true` — so the init kernel re-seeded the
whole sky. Dragging a flock radius destroyed the flock it was steering,
which is why the dynamics rows did not READ as working even though every
one of them landed.

### The idiom, one level up

ORGAN_4's console mask gave a per-field mask to a bank whose reader is an
event. ORGAN_5 gives one to a bank that already HAD a flag:

> **The FLAG says THAT the bank changed; the MASK says WHAT; the boundary
> decides HOW MUCH re-speak the edit requires.**

```cpp
inline uint32_t g_orb_def_touched = 0;      // bit = offsetof/4
inline uint32_t take_orb_def_touched();
```

One raise, at the one site that already raises the flag — and it uses
`e.def_offset`, not `e.offset`. The write two blocks above lands at
`p + e.def_offset`, so the bit must name the same word. For today's rows
the two are equal (every ORB_MOOD row is definition-only, and a def-only
entry's `offset` IS its `def_offset`); a future `ORGAN_PARAM_DEF` row with
an instance elsewhere would make them differ, and this is the one that
stays right.

### The classification lives in the registry, not at the boundary (D1)

```cpp
inline constexpr uint32_t ORB_RESEED_BITS =
      (1u << (offsetof(the_board::OrbMoodConfig, enabled)    / 4u))
    | (1u << (offsetof(the_board::OrbMoodConfig, count)      / 4u))
    | (1u << (offsetof(the_board::OrbMoodConfig, palette_id) / 4u))
    | (1u << (offsetof(the_board::OrbMoodConfig, drag)       / 4u));
static_assert(ORB_RESEED_BITS == 0x00001023u, …);
```

D1 permits either home. The registry wins because **the bit convention
(offset ÷ 4) is defined there and nowhere else**: the constant that
interprets bits belongs with the constant that produces them — the same
argument the console mask's own assert makes two blocks below. A field
reordered in `OrbMoodConfig` now fails the BUILD at the assert rather than
teaching the boundary to re-seed on the wrong dial.

Why these four and not others: `enabled` and `count` decide whether and
how many orbs exist; `drag` is written per orb at seed time; `palette_id`
colours them at init and recolor. All four are baked into `orb_state`. The
other fifteen are per-frame GPU reads — the uniform upload alone carries
them, and velocities and positions persist under the finger.

### One hazard the prescribed shape left open, and closed

`base_size` is an **OrbConsole** field, so it can raise the orb flag (it is
init-baked) while contributing **no bit** to a mask that indexes
`OrbMoodConfig`. Left alone, a same-frame flock drag would have supplied a
light bit, `tm != 0` would have been true, `(tm & RESEED) == 0` would have
been true, and the size edit would have been **swallowed**.

The console block now carries its own heavy reason down in one local:

```cpp
bool console_reseed = false;
…
if (cm & 2u) { g_orb_def_dirty = true; console_reseed = true; }
…
const bool reseed = console_reseed || (tm == 0u)
                 || ((tm & t7::organ::ORB_RESEED_BITS) != 0u);
```

### `configure_orbs` gains `bool reseed`, with NO default

Two callers, two different answers, so a third that forgets should fail the
build rather than inherit whichever answer happened to be written into a
default. `direction/mood.hpp` passes `/*reseed=*/true` — a mood change is a
new world's sky. The tail never CLEARS a pending init: a light pass leaves
the flag exactly as it found it, so a re-seed already armed by a mood
change still fires.

**The dt/t question, answered rather than deferred.** C5 found there is no
stomp to name. `gpuCfg` is value-initialised, so a whole-struct upload does
carry `dt = t_seconds = 0` — but `organ_flush` runs at the HEAD of the
frame (`pawn.cpp:190`, before input, before update, before render) and
`phase_orb_sky` runs in the RENDER spine, so the same frame's
`dispatch_orb_dynamics` re-authors both before any kernel reads them. With
the dome inactive the dispatch early-returns and nothing reads the uniform
at all. The comment in the tail says that, which is the honest version of
what the handoff asked to have said.

### The harness — with a wgpu floor that RECORDS

`configure_orbs` really uploads, so the harness stubs
`wgpuQueueWriteBuffer` to record the write instead of performing one.
*"The light pass still uploaded the uniform"* is therefore an assertion,
not a hope.

```
  ── P1: the touched mask and the reseed classification ──
  [PASS] ORB_RESEED_BITS is enabled 0 | count 1 | drag 5 | palette_id 12  0x00001023
  [PASS] every OrbMoodConfig offset/4 fits a uint32 mask            27 words
  [PASS] a flock def write raises the flag AND exactly its own bit  mask=0x00020000 (bit 17)
  [PASS]   and the boundary computes reseed = FALSE (the light path)
  [PASS]   the take returns the mask and clears it
  [PASS] a write to `count` computes reseed = TRUE                  mask=0x00000002
  [PASS] each of the four reseed facts computes TRUE alone          4/4
  [PASS] and the other fifteen orb rows all compute FALSE           15/15
  [PASS] door RESPEAK — the flag with NO bits — computes TRUE
  [PASS] a base-size raise forces TRUE even beside a light bit      the hazard, closed
  ── P1b: configure_orbs, called for real ──
  [PASS] a LIGHT pass still uploads the whole uniform               1 write, 480 bytes
  [PASS]   and leaves init_pending down
  [PASS]   a light pass NEVER CLEARS an init already pending
  [PASS] a HEAVY pass raises init_pending
  [PASS] a disabled mood uploads nothing and arms nothing (the early return)
  [PASS] count = 0 early-returns before the upload — the dome goes dark
GREEN — 0 failure(s)
```

The last line is C4's explanation of the phone defect, reproduced
natively: `count = 0` reaches `if (!os.active || os.count == 0) return;`
**before** the upload, so the dome goes dark and stays dark until the next
non-zero write. On a touch keyboard the first keystroke of every edit
writes zero (P5c).

## P2 — the rule made visible

**A dial whose effect depends on a mode stands next to a truthful readout
of that mode.** Fifteen of the orb rows are rule-scoped — the seven flock
rows act only under FLOCKING, the orbital speed only under ORBITAL, each
rule drag only under its own rule — and the panel showed no rule at all.
Turning one and seeing nothing was indistinguishable from a dead dial,
which is the diagnosis ORGAN_4 spent a campaign learning to fear.

### P2a — a window, and why it is a copy where the mood is a pointer

```cpp
inline uint32_t g_orb_rule_view = 0;                       // rule | gesture<<8
inline void set_orb_rule_view(uint32_t rule, uint32_t gesture);
EMSCRIPTEN_KEEPALIVE inline int organ_orb_rule(void);
```

`OrbsState.current_motion_rule` and `gesture_idx[]` remain the only truth
(CHORD's windows-not-homes). C2 found `organ_mood()` borrows a POINTER to
`mood_state_.active`; the rule cannot, and the reason is a tier: the mood
lives in a CONTRACT the registry already includes, and the rule lives in
`OrbsState` — a BODY, which the organ may not include. Same law, one home;
different plumbing, because the home is one tier further away.

**Packed into one `uint32`, and one ABI call**, because the operator reads
a rule and its gesture as one fact and two calls could disagree between
them.

### ONE WRITER, ONE SITE — and NOT the door handlers

The handoff placed the write "after boot's first configure, and after
handling either orb door. One writer, three sites max." The census says
three sites would have been stale for exactly the path Jean tested with:
**`cycle_orb_motion_rule` has two callers** — the door, and `KP_8` in
`direction/input.hpp:267` — and `cycle_orb_gesture` likewise has `KP_7`.
A readout refreshed only beside the doors would sit still while the
keyboard turned the sky.

So the write is one line at the frame boundary, in `organ_flush`, from the
one home:

```cpp
t7::organ::set_orb_rule_view(
    orbs_state_.current_motion_rule,
    orbs_state_.gesture_idx[orbs_state_.current_motion_rule & 3u]);
```

One writer, ONE site, and it cannot go stale whoever turned the rule. The
cost on a frame where nothing moved is two masks and a store.

### P2b — the shell, and what did NOT become a hardcode

D3 rules the four rule names as the one exception to name-blindness, and
the comment names the authority (`bodies/orbs.hpp`'s `RULE_NAMES`, and the
`ORB_RULE_*` constants `world.wgsl`'s own dispatch tests). **Nothing else
became an exception**, and two temptations were refused:

- **Which DOORS cycle the rule** is never asked. The handoff asks for a
  subtitle "on BOTH orb doors", which needs the shell to know that door
  ids 1 and 2 are the orb ones — a C++ NUMBER, and one that would silently
  move the readout onto the wrong button the day the roster is renumbered.
  That is precisely the defect name-blindness exists to prevent, and unlike
  the four names it fails SILENTLY. So the line sits under the whole door
  bar, beside both buttons that cycle the rule, and no id is named.
- **Which GROUPS are rule-scoped** is DERIVED from the group's own name:
  a group ending `"<rule> rule"` is scoped to that rule, and which one is
  read out of the name against `RULE_NAMES`. `Flocking rule` → 3,
  `Orbital rule` → 1, `Motion — all rules` → no match and no line, which is
  correct — those rows act under every rule. Renaming or adding a rule
  group in the `.inc` needs no edit here.

Two shapes, one voice. Under the door bar: `now: brownian · gesture 0`.
Under a rule-scoped group's header, dim when dormant and lit when live:

```
▸ these rows act in the FLOCKING rule — live rule: brownian
▸ these rows are acting NOW — live rule: flocking · gesture 2
```

The dormant line's hover carries the sentence the whole campaign is for:
*"Turning one and seeing nothing is the RULE, not a dead dial — cycle to
flocking to hear them."*

**An unknown rule prints its NUMBER.** A ruled duplication that has fallen
behind should say so, not invent: rule 9 reads `rule 9`, never a guess.

Refreshed at C3's 250 ms poll and 60 ms after ANY door press — name-blind
again, because RESPEAK moves no rule and a no-op refresh is cheaper than
asking which door this is. 60 ms beats the poll to the eye without racing
the program: the boundary has run three times by then at 60 fps, once at
15 fps.

The line is built once and the refresh moves TEXT, never nodes: at four
refreshes a second, replacing children would be churn for a line that
changes on a keypress. The filter hides a scope line with its header — a
scope line over no rows would answer a question the filter just took away.

### The harness

Native, on the compiled window:

```
  ── P2a: the rule window ──
  [PASS] boot reads brownian / gesture 0 — what the program seeds to
  [PASS] rule and gesture pack into one uint32 (rule | gesture<<8)   0x503
  [PASS]   and unpack to the pair the shell reads
  [PASS] each field is masked to a byte — neither can bleed into the other
  [PASS] the window follows the home and never writes back to it
```

Shell, under the shim against the real manifest — and with P3c's regroup
simulated, so the mechanism is proved before the strings that will use it
exist:

```
  [PASS] ORGAN_5 P2 — the live-rule line sits under the door bar
  [PASS]   and reads the packed window                     brownian · gesture 0
  [PASS]   a group named "<rule> rule" grows a scope line            1 scope
  [PASS]   dormant while another rule is live                   rulescope off
  [PASS]   and names its own rule       ▸ these rows act in the FLOCKING rule…
  [PASS]   the line follows the window                     flocking · gesture 2
  [PASS]   and the scope line LIGHTS when its rule goes live
  [PASS]   an unknown rule prints its NUMBER rather than guessing    rule 9
```

## P3 — the rule console

**A section is organized for the hand that plays it, not the struct that
stores it.** The orb rows were grouped by HOME — three under "Dome"
because they live in `OrbConsole`, nineteen under "Orb mood" and "Orb
flock" because they live in `OrbMoodConfig` — and the operator paid twice:
brownian's strength dial sat in the Dome group under the geometry-sounding
name *"noise floor"*, and fifteen rule-scoped rows sat in one
undifferentiated block with nothing saying which rule each acted in.

### P3a — the master motion dial, and an orphan retired

`OrbConsole` gains a fourth field. C6's expected branch held, so **D5 does
not fire**: the retirement is on.

| | before | after |
| --- | --- | --- |
| the value the kernel scales by | `OrbsState.speed_mult_current`, pinned at 1.0 since the gen-1 coupling retired | `ORB_CONSOLE_LIVE.speed_mult`, an authored console fact with a dial |
| where `configure_orbs` reads it | the tail of `pack_flocking_` | the config-build block, beside the other three console reads |
| `upload_orb_speed_mult` | zero callers | the console mask's bit 3 |

Three retirements, all C6-gated: the field, its teardown reset, and a
comment that still promised *"smoothed on the CPU, uploaded via
`upload_orb_speed_mult` only when it moves"* — a promise about a smoother
that no longer existed. The read moved out of `pack_flocking_` because it
was never a flocking fact: the kernel scales brownian's noise, orbital's
angular speed and flocking's speed ceiling by it. FROZEN has no energy
term, which is that rule's defining property and not an omission.

**The floor of 0 is HONEST**, and the distinction matters: the thirteen
ORB_MOOD floors sit one step off zero because `eff()`/`passthrough()` read
0 as *"no opinion"*. Nothing reads this one as a sentinel — the kernel
multiplies by it — so **zero is stillness**, an authored artistic state,
and the dial reaches it.

The console mask grows bit 3, pinned by the same `static_assert` that
pins the other three, and the boundary routes it to a targeted 4-byte
partial: **smooth under the finger, no re-seed.**

> A gen-2 coupling CLAIMS this field through a rest+gain seam when it
> arrives; until then the dial IS the rest. — the field's own banner

### P3b — the noise envelope widens

`noise_floor` max **1.0 → 3.0** `[heuristic]`, step unchanged. A dial must
be able to overshoot salience before Jean's eye can tune it back; the
ceiling is his to rule later.

### P3c — the five groups, by the hand

| group | rows | why it is one group |
| --- | --- | --- |
| `Sky & Light · Dome` | 2 — dome radius, base size | what the sky IS |
| `Sky & Light · Orbs` | 5 — enabled, count, palette id, drag, brightness | what populates it. The first four are EXACTLY `ORB_RESEED_BITS`; brightness rides with them by SUBJECT (what a mote looks like, not how it moves) |
| `Sky & Light · Motion — all rules` | 8 — speed mult, noise floor, rotation speed, rotation axis, drag ×4 | what moves it whatever rule is on |
| `Sky & Light · Orbital rule` | 1 — orbital speed | one row, and it earns a group: the group's NAME is what makes the live-rule line appear |
| `Sky & Light · Flocking rule` | 7 — the boids rows | the seven that made Jean's sweep |

**The layout and the readout stay in step with no table between them.**
P2b derives a group's rule from its own name, so `Orbital rule` → 1 and
`Flocking rule` → 3 light up automatically, and `Motion — all rules`
(plural) correctly grows no line — those rows are never dormant.

**The four rule drags stay together** rather than being split one per rule
group: they are one four-lane fact — the shape of drag across the whole
rule set — and splitting them would hide that shape from the hand
comparing them. Each names its own rule in its label, now spelled
`drag × brownian rule` so the label answers the question the group's
heading no longer does.

D4 held: **no block, offset, type, range, step or mask bit changed** in the
regroup. Only group strings, labels, and row order within the section —
plus the two edits P3a and P3b authorise by name.

### P3d — the books

| | |
| --- | --- |
| entries | 305 → **306** |
| `organ_readers` | 231 → **232 proved, 0 suspects** — `configure_orbs` names `speed_mult`, so the new row is PROVED and not merely enrolled |
| `organ_gap` | `OrbConsole 4/4 named`, 13 pairs, 0 surviving readers |
| `audit/ORGAN.md` | regenerated — Sky & Light 37 → 38 |
| `web/presets/baseline.json` | regenerated — 231 → 232 keys, `ORBS.speed_mult = [1]` |

```
  ── P3a: the master motion dial ──
  [PASS] OrbConsole is four words — the design row grew with the bank
  [PASS] speed_mult sits at offset 12, so its console bit is 3
  [PASS] the authored rest is identity — the dance, unscaled
  [PASS] the master dial enrolls · 0 … 4, the floor HONEST not a sentinel
  [PASS] a speed write raises console bit 3 and lands       mask=0x8 value=2.75
  [PASS]   and asks for NO re-speak — smooth under the finger, no reseed
  [PASS] configure_orbs reads the CONSOLE, not OrbsState (smoother retired)
  ── P3b/P3c: the noise envelope and the regroup ──
  [PASS] noise floor's ceiling widens 1.0 -> 3.0                      max 3
  [PASS] the five orb groups are contiguous and correctly populated     5/5
  [PASS] D4: noise floor changed GROUP, never its block or offset
```

And the shell, against the REAL regrouped manifest — the simulation P2
needed is gone, the strings are in the tree:

```
  [PASS] the two "<rule> rule" groups grow scope lines            2 scope(s)
  [PASS]   both dormant while brownian is live        rulescope off | off
  [PASS]   each names its OWN rule, derived from its group name
  [PASS]   FLOCKING lights and ORBITAL stays dark — one live rule at a time
```

## P4 — the pill

**Minimized is a state, not a scroll.** `hidden` already existed and the
backtick already toggled it; what was missing was a way BACK on a device
with no backtick. On a phone, `?organ=1` opened a panel that could be
hidden and never restored.

**One state, three doors.** A `minimized` session variable, moved by the
header's minimize button, by the pill, and by the key — so they cannot
disagree about whether the panel is up:

```js
function setMinimized(on) {
  minimized = !!on;
  root.className = minimized ? 'hidden' : '';
  pill.className = minimized ? 'on' : '';
}
```

**The pill lives on `<body>`, not inside `#organ`** — `#organ` is what
gets hidden, and a pill inside it would vanish with the thing it exists to
bring back. The harness asserts the parent, because that is the mistake
this design is one line away from.

**44 px, twice.** The minimize button and the pill are both 44 px tall —
the smallest target a thumb hits reliably. The button's negative margins
(`-14px` top and bottom) pull its box back to the header's own 16 px line,
so the hit target is generous while the layout keeps its row; the ORGAN_3c
grid is untouched. The glyph is an em dash: the panel, folded.

**Session only** — no storage of any kind, the same law the width and the
`openMap` already follow.

The header's title changed from `ORGAN — \` to hide` to
`ORGAN — \` toggles`, which is now true in both directions, and its hover
names all three doors for the operator who found the panel before finding
this line.

```
  [PASS] ORGAN_5 P4 — the pill exists
  [PASS]   and lives on <body>, NOT inside the panel it brings back
  [PASS]   hidden while the panel is up
  [PASS]   the minimize button is in the header, sharing one row with the title
  [PASS]   pressing it hides the panel and shows the pill   root="hidden" pill="on"
  [PASS]   tapping the pill brings it back — the phone has its backtick
  [PASS]   and the BACKTICK moves the same state, not a second one
  [PASS]   toggling back agrees with both other doors
```

The four query modes still hold, all GREEN: `?organ=1&preset=baseline`,
`?preset=baseline` (no panel, no pill, the scene still applies), no flag
(no DOM, no fetch, no timer, no ccall), and the synthetic probe.

## P5 — the phone

### P5a — the panel was already versioned; the second placeholder was not built

Gate 5's finding (P0) settles half of this phase before it starts:
`op.src = 'organ_panel.js?v=' + BUILD;` has been in `web/index.html` all
along. *"A stale panel against a fresh program"* was already impossible by
construction.

The handoff's other half — a second `__BUILD_ID__` placeholder — **would
have broken a law to obtain a value the file already holds**. The shell
states it three blocks above:

> *"The placeholder appears exactly ONCE in this file, on the next line, so
> the substitution has one target and the dist script's refusal-to-ship
> check has one thing to count."*

And `web_dist` substitutes with `str.replace`, which rewrites **every**
occurrence — including one inside a comment. (Verified the hard way: the
first draft of the comment defending this invariant spelled the token, and
`grep -c` caught it before it shipped.) What landed is one line:

```js
window.T7_BUILD_ID = BUILD;
```

Proved end to end against stub build outputs: `dist/index.html` carries
`var BUILD = 'f9372e70c562';`, `window.T7_BUILD_ID = BUILD;` and
`organ_panel.js?v=f9372e70c562`, from ONE substitution target.

`web_dist`'s report gains the line the handoff asked for — *"is the panel
versioned too?"* was a question the report left the reader to answer by
opening `index.html`:

```
  build id           f9372e70c562   <- sha256(the_board.wasm)[:12]; the .js/.wasm/.data query
                     Deploy twice without rebuilding and this must not change.
                     organ_panel.js?v=f9372e70c562 too; the panel's footer prints `build f9372e70c562`.
```

### P5b — the footer tells, and so does the pill

`build <id>` leads the footer's status line, and rides the pill's hover
too — on a phone the footer is a scroll away and the pill never is. Two
devices showing different ids are not the same program, and *"count works
here and not there"* stops being a mystery.

**Absent is not an error.** Opening `web/index.html` directly leaves the
placeholder unsubstituted, and a page predating this line has no global at
all; either way the footer says NOTHING rather than printing a lie about
provenance. Both paths are driven in the harness.

**No new counters (D2).** C4 found `organ_rejected_count()` already
cwrapped and already displayed as `· rejected N`. The instrument P5b
offers to add exists; nothing was added.

### P5c — an empty box is not a value

C4's mechanical finding, fixed. The old handler was:

```js
v[0] = clamp(parseFloat(src.value) || 0, p);
push(p, v); sl.value = v[0]; nm.value = v[0];
```

Two defects, both provable without a phone:

1. **An EMPTY field committed `p.min`.** `''` parses `NaN`, `|| 0` makes it
   `0`, `clamp` snaps to the floor. Clearing a number box before typing is
   the ordinary way to enter a value on a touch keyboard, so the FIRST
   keystroke of every phone edit wrote the minimum. On `count` (min 0)
   that is `count = 0` — and `configure_orbs` early-returns on
   `os.count == 0`, so the dome goes dark mid-edit and every later
   keystroke lands on a dead sky. **The C++ half is reproduced natively in
   P1's harness**: `count = 0` early-returns *before* the upload.
2. **The handler rewrote the box being typed in.** Reassigning a focused
   number input on every keystroke fights the caret on touch keyboards.

Now: a non-numeric box commits nothing and the program keeps its last good
value; the handler syncs the OTHER widget only; and `change` (blur/Enter)
is the settle — it normalises the box to the value that actually landed,
so a clamp becomes visible when the hand lets go instead of silently
disagreeing with the program. **Two commit paths, one `push`.**

```
  [PASS] ORGAN_5 P5b — the footer names the build, first   build a1b2c3d4e5f6 · 306 dials
  [PASS]   and the pill carries it too
  [PASS] ORGAN_5 P5b — an absent or unsubstituted id prints NOTHING, not a lie
  [PASS] ORGAN_5 P5c — an EMPTY box commits NOTHING
  [PASS]   and neither does a non-numeric one
  [PASS]   a real value still commits, one write                    1 write
  [PASS]   and the OTHER widget follows it                          64
  [PASS]   while the box being typed in is NOT rewritten under the finger
  [PASS]   `change` normalises the box to the CLAMPED value that landed  256
  [PASS]   and the slider agrees                                    256
```

### The sentence P5c asks for, and it still stands

**Count-on-phone awaits a same-build comparison; the footer's build id is
the instrument.** What P5c fixed is mechanical and device-independent —
the empty-box commit is wrong on every device and merely bites hardest
where clearing the field is how one types. Whether it was the whole of
Jean's phone defect cannot be settled from here: no device, no
reproduction. The next report should name the build id from the phone's
footer and the desktop's, and if they match, the remaining difference is
the device and not the program.

---

## ORGAN_5 AT CLOSE

### The tallies

| | at open | at close |
| --- | --- | --- |
| entries | 305 | **306** |
| dials / witnesses | 288 / 17 | **289 / 17** |
| blocks · doors | 12 · 3 | **12 · 3** (unchanged) |
| by cadence | live 120 · gen 39 · boundary 129 · driven 17 | **live 120 · gen 39 · boundary 130 · driven 17** |
| Sky & Light section | 37 | **38** |
| orb groups | 3 (Dome, Orb mood, Orb flock) | **5** (Dome, Orbs, Motion — all rules, Orbital rule, Flocking rule) |
| `organ_readers` | 231 proved | **232 proved, 0 suspects** |
| GPU wallet | — | **UNTOUCHED**: zero buffers, zero bindings, zero WGSL |

### The P0 census verdicts

Five gates green. **Gate 5 materially different and STOPPED on the item:**
`web/index.html` already versions `organ_panel.js`, so P5a's first half
was work already done and its second half would have broken the shell's
one-placeholder law. Reported, not executed; P5 landed the goal by the
route the tree's own law permits.

C1 offsets and `ORB_RESEED_BITS = 0x1023` · C2 the mood pointer and why
the rule cannot be one · C3 the poll and the door click · C4 two
touch-hostile properties and a reject counter that already exists · C5 no
dt/t stomp to name (the frame order prevents it) · C6 the orphan
confirmed. Each is recorded in full under `ORGAN_5 P0`.

### C6's anatomy, and D5

**D5 did not fire.** `speed_mult_current` was declared, reset in teardown,
read once in `pack_flocking_`, and moved by nothing; `upload_orb_speed_mult`
had zero callers. The gen-1 coupling's writer was retired and left an
authored landing pad with nothing landing on it. So the retirement went
ahead: the field, its teardown reset, and a comment still promising a CPU
smoother that no longer existed. The kernel's read was checked rather than
assumed — brownian's noise, orbital's angular speed and flocking's speed
ceiling all scale by it; FROZEN has no energy term, which is that rule's
defining property and not an omission.

### P5c's sentence, and it stands

**Count-on-phone awaits a same-build comparison; the footer's build id is
the instrument.** P5c fixed what is mechanical and device-independent —
the empty-box commit is wrong everywhere and merely bites hardest where
clearing the field is how one types, and `count = 0` reaching
`configure_orbs`'s early return is reproduced natively. Whether that was
the WHOLE of the defect cannot be settled without the device. The next
report should name the build id from the phone's footer and the desktop's.

### The one thing this session could not do

`git push origin --delete claude/campaign-handoff-3h50qd` is **refused by
the session's git relay with HTTP 403**, in every refspec form, while
ordinary pushes to master succeed; no GitHub tool here exposes ref
deletion. The local branch is gone. **One command for Jean from his own
machine**, and the attic law is satisfied.

### The gates at close

| gate | verdict |
| --- | --- |
| `tools/gates/console_gate/run.py` | **PASS** — cartridge.hpp and console.hpp, zero diagnostics |
| `tools/gates/glaw2/run.py` | **GREEN** — 294 fn, 271 const, 81 struct, 86 binding, 65 entry points |
| `tools/gates/sha256_gate/run.py` | **PASS** |
| `tools/organ_gap.py --gate` | **PASS** — 13 pairs, 0 surviving runtime readers |
| `tools/organ_readers.py` | **232 proved, 0 suspects** — the new master dial is PROVED by `configure_orbs`, not merely enrolled |
| `tools/binding_gen.py --check` | PASS on every relation |
| native harnesses | **56 + 35 = 91 asserts, GREEN** — the ORGAN_4 harness re-run against the new tree, and an ORGAN_5 harness that calls the real `configure_orbs` behind a recording wgpu floor |
| shell shim | **45 asserts, GREEN**, and all four query modes hold |
| `tools/gates/score/run.py` | **RED, 4 violations — PRE-EXISTING** (proved by `git stash` at ORGAN_4). Spine-table jurisdiction. |
| `tools/wgsl_gate.py` | unrunnable here — no `naga`. No WGSL was touched. |
| `tools/mirror_census.py` | NOT RUN — frozen at ML-1 per `docs/OPEN.md` |

### What a successor should not trust silently

- **The harness restates the boundary's reseed expression.** `organ5_harness`
  keeps its own copy of
  `console_reseed || (tm == 0) || (tm & ORB_RESEED_BITS)`, kept in step
  with `cartridge.hpp` by eye. The asserts catch a divergence in the
  CLASSIFICATION; they would not catch the boundary dropping the test
  altogether. A gate that reads the cartridge would.
- **`RULE_NAMES` is four strings in JS (D3).** Their ORDER is the
  contract, and the authorities are `bodies/orbs.hpp` and the kernel's own
  dispatch. An unknown index prints its NUMBER rather than guessing, so a
  stale table says so — but only for indices past the end.
- **The rule window is refreshed every frame from `OrbsState`.** If a
  future rule author writes `current_motion_rule` from somewhere the
  boundary cannot see, the window still catches it; if one writes it
  AFTER the boundary in the same frame, the panel is one frame behind.
  At 250 ms polling that is invisible, and it is the price of one writer.
- **`web/presets/baseline.json` is regenerated, not hand-kept.** It grew
  `ORBS.speed_mult` this campaign. Any `.inc` change wants
  `tools/organ_ledger.py` and a fresh capture, or the shelf drifts from
  the registry.
