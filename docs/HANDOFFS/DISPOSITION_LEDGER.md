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
| `FIELD_BEACON_S` | F32 | 200.0 | C2 | Interaction · Beacon | — | **DEFER — the assert hazard below** |

**THE BEACON'S ASSERT — a standing hazard this campaign must not break.**
`control_panel.hpp` closes with

```cpp
static_assert(FIELD_BEACON_S < FIELD_K,
    "the beacon's pull must lose to field repulsion at the ring — "
    "otherwise the gather clots instead of spacing (FIELD_4's ruling)");
```

A compile-time proof cannot guard a runtime dial. `FIELD_K` enrolls above
and its range only ever *raises* the winning side, so the ruling survives
every value that dial can reach. `FIELD_BEACON_S` is DEFERRED: enrolling
it live would let the panel push the beacon's pull past `field_k` and
clot the gather, with the static_assert still reading true and no runtime
witness. What lifts it: a clamp at the writer (`S = min(S, field_k)`), or
the ruling restated as a paired range. Priced, not promised.

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
