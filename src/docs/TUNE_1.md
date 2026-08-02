# TUNE_1 — SIX TABLE EDITS, NO NEW MECHANISMS

Trunk-based, master only. One commit per task, in Jean's order — A4, A1, A3,
A8, A5, A10 — then this ledger. Cut from master `def19d7`.

**All six landed. Nothing held.**

| # | commit | task | verdict |
|---|--------|------|---------|
| 1 | `b1c0d73` | A4 mute pawn aura tint | LANDED — second branch |
| 2 | `0c153da` | A1 per-channel face hue | LANDED |
| 3 | `5909381` | A3 FPV eye = figure × ratio | LANDED — pad was free |
| 4 | `3d3f068` | A8 cactus + blade join the cap | LANDED — gate passed |
| 5 | `25bde69` | A5 terrain local/detail | LANDED — no CPU twin |
| 6 | `57407ae` | A10 Conway weights skewed slow | LANDED — twin, both rooms |

Census scope throughout: `src/cartridges/the_board/**` and
`src/incubator_dual.cpp`. Counts are boundary-delimited. `world.wgsl` is the
repo's only shader, which is what makes the "zero reads" claims in A4 and A1
closed rather than partial.

Jean holds all build and visual gates. What is asserted below is what the code
says and what the arithmetic gives; **nothing here claims a render was seen.**

### What was mechanically checked here, and what was not

A full build is **not possible in this container** — `state.hpp` includes
`<webgpu/webgpu_cpp.h>` and Dawn is not vendored, so every header that reaches
the GPU wire (`state.hpp`, `cartridge.hpp`, `grounded.hpp`, `gol_zones.hpp`,
`pawn.hpp`) cannot be isolated into a translation unit. That is a missing
dependency, not a signal about the edits. What *was* run:

- `contracts/indoor_module.hpp` and `bodies/pawn_figures.hpp` are
  self-contained and **compile clean**, `g++ -fsyntax-only -std=c++17`. That
  covers both new `static_assert`s in A3 and the edited table in A8.
- The A3 ratio was **executed**, not just asserted — see the numbers below.
- The A8 `INDOOR_TREATMENT` table was compiled and printed after the edit.
- The A10 twin was compared column by column between the two rooms by script.
- The A3 `DesignConfig` mirror was diffed field by field, `def19d7` vs `HEAD`,
  in both rooms.
- L1: every edited file re-checked for BOM and CRLF. All clean, LF only.

Everything else in this ledger is a reading of the code, quoted verbatim.

---

## A4 — MUTE THE AURA TINT

**Anchor verified.** `PAWN_AURA_DEFAULT` at `bodies/pawn.hpp:47`, verbatim as
found:

```
    0.5f,              // tint_strength
    0.4f, 0.2f, 0.5f, // tint RGB (purple)
    PawnAuraDeltaMode::CONVERGENT,
    0.3f,              // delta_magnitude (used in random mode)
    0x3u,              // effect_mask: color tint + height
    3.0f,              // height_scale
```

### BRANCH FIRED: the second one — `tint_strength 0.5f → 0.0f`

**`effect_mask` has ZERO reads.** The full census is five hits, and every one
is a declaration or the upload assignment:

- `realization/world.wgsl:6209` — `    effect_mask: u32,` (struct field)
- `realization/state.hpp:1206` — the C++ field
- `bodies/pawn.hpp:43` — the profile field
- `bodies/pawn.hpp:56` — the authored `0x3u`
- `bodies/pawn.hpp:163` — `auraCfg.effect_mask = ap.effect_mask;`

No masking operator is applied to any aura config value anywhere in the
shader. Bit 0 therefore does **not** gate the color write, and the first
branch — `0x3u → 0x2u` — would have been a **no-op**. Only one branch fired.

**The color write is unconditional.** `world.wgsl:9249-9256`, verbatim:

```
    if (cell.intensity > 0.001) {
        let color_blend = cell.intensity * pawn_aura_cfg.tint_strength * cell.color_osc;
        let height_blend = select(cell.intensity * cell.height_delta, 0.0, pawn_aura_cfg.height_scale < 0.01);
        textureStore(pawn_aura_tex_write, vec2<i32>(gid.xy),
            vec4(height_blend,
                 cell.delta_r * color_blend,
                 cell.delta_g * color_blend,
                 cell.delta_b * color_blend));
```

The only enclosing guard is the per-cell activity test at 9249. Walking
outward, the only earlier early-out in `compute_pawn_aura` is the grid-bounds
`if (sx >= N || sz >= N) { return; }`. With `tint_strength` at 0,
`color_blend` is 0, the GBA channels store 0, and the terrain FS add
`base_color = clamp(base_color + aura.gba, ...)` at `world.wgsl:4667`
contributes nothing — **on GoL cells or anywhere**, because that add is the
single site where the aura's color reaches terrain.

**Height untouched, and it is a genuinely separate path.** Height rides
`pawn_aura_cfg.height_scale` (line 9251), not a mask bit; the extrusion rides
`config.pawn_aura_height` at `world.wgsl:4432`; and the R-channel brightening
at `world.wgsl:4669-4670` is `+vec3(height_boost)` — achromatic, so it cannot
put purple anywhere.

**Staging proven.** `pawn.hpp:165` is
`auraCfg.tint_strength = std::min(ap.tint_strength * p, 1.0f)` — multiplicative,
so 0 stays 0 at every presence value. The only other writers of that buffer are
the per-frame `dt` / `t_beats` pokes, which cannot reach the field.

`tint_r/g/b` and `delta_magnitude` left at their authored values. Nothing
deleted.

### Correction carried in the same commit

Three comments claimed a gate that does not exist — `bit 0=color, bit
1=height` in `pawn.hpp`, `state.hpp`, and `world.wgsl`. Re-tagged
`STATUS: INTENT` per L9, naming the live gates instead. `aura_n` (declared at
`world.wgsl:6210`) is dead the same way — `sample_pawn_aura` uses the WGSL
constant — and now says so.

### Standing finding, not acted on

`effect_mask` is a dead uniform field in both rooms. Removing it is a struct
change and no task named one, so it stays. Flagged for a future pruning pass.

---

## A1 — CUBE FACES GET THEIR OWN HUES

**Anchor verified.** `world.wgsl:5168-5170` before the edit, verbatim:

```
    let face_hash = hash_property(fe.entity_seed, 500u + face_idx);
    let face_delta = (face_hash - 0.5) * 2.0 * fe.face_variance;
    let face_color = clamp(fe.color + vec3(face_delta, face_delta * 0.7, face_delta * 0.5), vec3(0.0), vec3(1.0));
```

One hash into three channels at fixed ratios — monochromatic by construction,
exactly as the ruling states. Jean's BIND drops the `0.7`/`0.5` ratios; they
were the mechanism, not a separate dial. The landed code is Jean's BIND text.

### The property block — 500–517, now one purpose

| channel | hash | indices |
|---------|------|---------|
| R | `500u + face_idx` | 500–505 — **unchanged** |
| G | `506u + face_idx` | 506–511 — new |
| B | `512u + face_idx` | 512–517 — new |

`face_idx` is 0..5, confirmed closed: the dominant-normal chain at
`world.wgsl:5160-5167` assigns `{0,1}` on the X arm, `{2,3}` on Y, `{4,5}` on
Z, and every path assigns. Each run is exactly 6 wide; the three runs are
disjoint.

### CENSUS 506–517 in the ENTITY-seed space — ZERO occupants

The entity-seed space has exactly two consumers in scope:

- `world.wgsl:5168` — `hash_property(fe.entity_seed, 500u + face_idx)` → 500–505
- `world.wgsl:4319` — `hash_property(entity_seed, 913u)`, the mosaic batch

`GAUSSIAN_PAIR_OFFSET` is 1000, so a `sample_gaussian` on property *P* also
consumes *P*+1000; no entity-seed `sample_gaussian` exists at all, so neither
path reaches 506–517. **No STOP. The block was free.**

**NOT collisions, as ruled.** The 500u / 501u / 510u / 520u / 521u / 540u uses
at `world.wgsl:1153`, `1190`, `1223`, `1255`, `1281`, `1287` take a **lattice
node seed** — the palette / mode / transition / sparse lattices — which is a
different seed space entirely. They are recorded here so the next census does
not re-litigate them.

**R is bit-identical**: same seed, same property, and the BIND gives R an
implicit weight of 1.0, which is what `face_delta` carried before. At
`face_variance == 0` all three channels are bit-identical. The change is
provably additive on G and B.

`CUBE_TIERS` `FACE_VARIANCE` column not touched, as ruled.

### Ledger note — the live coupling on this dial

Delivered `face_variance` is **not** the tier draw:

- `bodies/cube_behaviors.hpp:765-767` — `cube_write_gpu` adds
  `ZOETROPE_FACE_SPLAY` × cell intensity onto it
- `bodies/cube_behaviors.hpp:1018-1019` — `zoetrope_project_slot` writes
  `face_variance * ZOETROPE_FACE_REST + ZOETROPE_FACE_SPLAY * I`
- `bodies/cube_behaviors.hpp:154` — `ZOETROPE_FACE_SPLAY = 1.50f`

So delivered `face_variance` reaches ~2.0 under a full-intensity strike and
each delta spans well past the `[0,1]` clamp. Per-channel independence changes
the **character** of that railing — independent instead of lockstep — but not
its magnitude. If controlled hue rather than clipped hue is ever wanted, the
dial is the splay, not `CUBE_TIERS`.

---

## A3 — EYE HEIGHT FOLLOWS THE FIGURE

**Anchors verified.** `world.wgsl:1941` `const PAWN_HEIGHT: f32 = 1.5;`;
`world.wgsl:1977` `const FPV_EYE_HEIGHT: f32 = PAWN_HEIGHT + 0.2;  // Camera at
eye level`; and the single consumer at `world.wgsl:7894`:
`        camera.pos = pawn_pos + vec3(0.0, FPV_EYE_HEIGHT, 0.0);`

### FPV_EYE_RATIO = 1.7f / 1.5f = **1.1333333254f**

**The conventional figure is figure 0, by population and not by index.**
`FIGURE_SHARES` gives `FAM_REGULAR` 70.0% across a **single** member, against
15% split six ways (2.50% each) for smooth and 15% split seven ways (2.14%
each) for heraldic. Seven bodies in ten are figure 0, height 1.50.

The full roster, and the eye height each figure now gets:

| # | figure | family | height | share | eye |
|---|--------|--------|--------|-------|-----|
| 0 | Pawn | regular | 1.50 | **70.00%** | **1.700000** |
| 1 | Squat | smooth | 1.90 | 2.50% | 2.153333 |
| 2 | Colossal | smooth | 3.50 | 2.50% | 3.966667 |
| 3 | Acorn | smooth | 2.00 | 2.50% | 2.266667 |
| 4 | Spire | smooth | 3.80 | 2.50% | 4.306666 |
| 5 | Idol | smooth | 2.30 | 2.50% | 2.606667 |
| 6 | Stele | smooth | 3.00 | 2.50% | 3.400000 |
| 7 | Bronze | heraldic | 1.50 | 2.14% | 1.700000 |
| 8 | Silver | heraldic | 1.61 | 2.14% | 1.824667 |
| 9 | Gold | heraldic | 2.26 | 2.14% | 2.561333 |
| 10 | Steel | heraldic | 1.93 | 2.14% | 2.187333 |
| 11 | Crystal | heraldic | 1.75 | 2.14% | 1.983333 |
| 12 | Star | heraldic | 1.45 | 2.14% | 1.643333 |
| 13 | Divine | heraldic | 1.88 | 2.14% | 2.130667 |

The possessed agent resolves through
`agent_state_.slots[player_.possessed_slot].skin_id`, the same lookup
`set_pawn_tilt_tau` already uses; `pawn_vs` clamps an out-of-range `skin_id`
to 0, and the CPU staging mirrors that fallback.

### The exactness assertion — checked, not assumed

float32 rounds the ratio, so the round trip had to be measured. Compiled:

```
FPV_EYE_RATIO = 1.1333333254
eye(fig0)     = 1.7000000477
```

`1.7000000477` is exactly what `PAWN_HEIGHT + 0.2` evaluated to
(`1.5f + 0.2f`, where `0.2f` is `0.20000000298023224`). **Bit-identical.** The
static_assert in `pawn_figures.hpp` pins it, and a second static_assert pins
figure 0's height at 1.50 so a future rescale of the conventional figure
cannot silently move the anchor.

### The compute stage CANNOT reach `agent_figure_profiles` — answer: NO

`world.wgsl:921` declares
`@group(0) @binding(112) var<uniform> agent_figure_profiles: array<PawnFigure, 14>;`
and `binding_registry.hpp:59` marks it `// uniform: PawnFigure[14] (render VS
only)`. Its readers are `pawn_vs` (`world.wgsl:4983`) and the monolith VS path
(`5195`) — both render. It is **absent from update_camera's compute layout**,
and adding it would be a new binding, which this round forbids. Hence the CPU
derives the value. This is the same reason `pawn_tilt_tau` already travels the
same door.

### A pad was free — NO sizeof delta, so the GROWTH LAW branch never fired

`GPUDesignConfig` ends in three declared tail pads, `_pad592_0/1/2` (MOSAIC_2
residue: six dials + two pads became five + three). The **first** is reused in
place as `fpv_eye_height` — **both rooms, same commit, same position, same
type**. The `sizeof` witness stays **592** and was not edited. This is growth
law (1), the `possessed_slot` / `veil_dither` / `indoor_height_cap` precedent.
No `vec3` moved, so L4 is not engaged.

**Twin rooms touched:** `realization/state.hpp` (`GPUDesignConfig`) and
`realization/world.wgsl` (`DesignConfig`).

**Mirror verified mechanically**, by extracting both field lists at `def19d7`
and at `HEAD` and diffing them:

```
WGSL DesignConfig:    70 fields before, 70 after
   #67: _pad592_0: f32     ->  fpv_eye_height: f32
C++ GPUDesignConfig:  71 fields before, 71 after
   #68: _pad592_0: float   ->  fpv_eye_height: float
```

Nothing added, removed, reordered, or retyped in either room — the sole delta
is the rename, at the same position. (The one-field index offset between the
rooms is pre-existing and is L4 working as designed: the C++ room declares
`_pad_sun` / `_pad_fog[2]` / `_pad_pier_retired` where WGSL gets that padding
implicitly from `vec3` alignment. That offset is unchanged by this commit.)

### Nothing new was created

Staging goes through the **existing** `GPUState::config()` accessor and the
**existing** `mark_config_dirty()`, guarded on change exactly as
`set_pawn_tilt_tau` guards — so the per-frame call costs nothing while the
figure stays put, and no setter had to be born. `mark_config_dirty()` had no
callers before this commit; it has one now. The only new symbol is
`FPV_EYE_RATIO`, which the task named.

Boot pins the rest beside the tilt pin (L10). Unlike tilt, this does **not**
match zero-init — a zero would drop the camera to the pawn's feet on frame 1 —
so the pin is load-bearing rather than decorative.

The WGSL const is deleted rather than left dead (L8), and the site says what
replaced it. No binding-closure change (L7): `update_camera` already reads
`config` through `fpv_mode_active()`.

---

## A8 — PLANTS UNDER THE CEILING

### STOP GATE PASSED

`contracts/indoor_module.hpp:37`, verbatim:

```
inline constexpr float INDOOR_HEIGHT_CAP_FRACTION = 0.75f; // Jean's law
```

**0.75.** Untouched — it is shared by the other eight capped families and the
GoL lift, and neither a global change nor a private fraction for plants is
authorized.

The two rows, before and after:

```
    /* cactus  */ { IndoorSize::NATURAL, IndoorBounds::MARGIN },  // Jean: keeps size
    /* blade   */ { IndoorSize::NATURAL, IndoorBounds::MARGIN },  // Jean: keeps size
→   /* cactus  */ { IndoorSize::CAP,     IndoorBounds::MARGIN },  // TUNE_1 A8: was NATURAL
→   /* blade   */ { IndoorSize::CAP,     IndoorBounds::MARGIN },  // TUNE_1 A8: was NATURAL
```

Verified after the edit by compiling the table and printing it: cactus and
blade read CAP, column stays EXACT, gol and gallery stay NATURAL, fraction
still 0.75. The two "Jean: keeps size" comments are gone — that ruling is
reversed, and no other site still carries the claim.

**Neither family had a rescale hook or a param list before this commit** —
both adapter slots held `nullptr` with a `NATURAL … keeps size` comment.

### The pattern being mirrored

```
inline void palm_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    cap_to_ceiling(inst, ceiling_h, INDOOR_HEIGHT_CAP_FRACTION,
        /*current_h*/ inst.params[PalmIdx::HEIGHT],
        PALM_INDOOR_RESCALE_PARAMS);
}
```

`cap_to_ceiling` (`indoor_module.hpp:98`) returns early unless `current_h`
exceeds `cap_fraction * ceiling_h`, so **it only ever shrinks**. Outdoor sizes
and every indoor plant already under the cap are bit-identical.

### Vertical extents — read off the mesh generators, not the names

| family | `current_h` | why |
|--------|-------------|-----|
| cactus | `CactusIdx::HEIGHT` | trunk ring walk sets `y = t * p.height` |
| blade | `BladeIdx::BLADE_H` | blade walks to `dist = t * blade_h`, lifts by `cos(splay) ≤ 1` |

For cactus the arms fork at `p.height * fork_frac` and rise by at most
`arm_length`, which stays under the trunk top in **all three tiers** at their
means: FINGER 9.0 vs 3.6+2.0, SAGUARO 13.0 vs 5.9+4.5, CANDELABRA 20.0 vs
8.0+7.0. The trunk is the family's tallest point in every row.

### LENGTHS — classification from the code, not the column name

**cactus** — `HEIGHT`, `RADIUS`, `ARM_LENGTH`, `ARM_RADIUS`.
Excluded, with the reason: `TAPER` / `CAP_ROUND` / `RIB_DEPTH` are multipliers
applied to a radius (`r_base * rib_mod * cap_scale`); `RIBS` / `ARM_COUNT` are
counts; `LEAN` is a **fraction of height** (`lean_mag = p.lean * p.height * t
* t`), so it follows the scale for free; `LEAN_DIR` and `ARM_CURVE` are an
angle and a blend.

> Noted, not corrected: the `CACTUS_TIERS` UNITS comment calls `LEAN` radians.
> The mesh generator uses it as a fraction of height. Either reading excludes
> it from a length list, so the edit does not turn on it.

**blade** — `BLADE_H`, `BLADE_W`.
Excluded: `BLADE_COUNT` is a count; `BLADE_H_VAR` is a fraction of `BLADE_H`
(`h_mult = 1.0 + (hash - 0.5) * p.blade_h_var * 2.0`); `CURVE` multiplies
`blade_h` (`curve_off = p.curve * blade_h * t * t`) and so scales for free;
`SPLAY` / `TWIST` are angles; `TAPER` is a width exponent
(`pow(t, p.taper * 2.5 + 0.5)`).

**Solid extents stay honest** without a separate edit: `generic_select` runs
the indoor hook **before** `compute_solid_half`, by design and with a comment
saying so, so the footprint derives from the scaled params.

---

## A5 — TERRAIN IMMEDIACY

### STOP GATE PASSED — all four live values matched the ruling

**No CPU twin.** `TERRAIN_BANDS` is GPU-only: no `TerrainBand` struct or band
table exists anywhere in scope. `surface/terrain_looks.hpp:135` only *points*
at it ("ROW 7 THE MOVEMENT THIRD: the TRUE bands (TERRAIN_BANDS)"), and
`ribbon.hpp`'s `lateral_amp_sigma` / `vertical_amp_sigma` are a different
family's fields. Unlike GOL_TIERS there is no second room to edit.

```
    //              spacing  freq_μ  freq_σ  amp_μ  amp_σ  damp_μ  damp_σ  damp_min activ  t_freq
    TerrainBand(     30.0,   0.200,  0.060,  1.2,   0.5,   0.040,  0.020,  0.020,  0.60,  0.20  ),  // 2: local
    TerrainBand(     12.0,   0.500,  0.150,  0.4,   0.2,   0.080,  0.040,  0.040,  0.55,  0.40  ),  // 3: detail
→   TerrainBand(     30.0,   0.200,  0.060,  1.2,   1.0,   0.040,  0.020,  0.020,  0.78,  0.20  ),  // 2: local
→   TerrainBand(     12.0,   0.500,  0.150,  0.4,   0.45,  0.080,  0.040,  0.040,  0.72,  0.40  ),  // 3: detail
```

Bands 0/1/5 (patch-and-larger, vetoed) and band 4 (rubble) untouched.
`amp_mean`, `freq_*`, `damping_*`, `spacing`, `temporal_freq` unchanged in
every row. **`damping` held in reserve and not touched.**

### The two instruments, at their consumers

`world.wgsl:557` — the activation gate, spatially coherent:

```
    if (hash_property(node_seed, WAVE_PROP_ACTIVE) > band.activation) {
        return vec2(0.0, 0.0);
    }
```

Raising activation fires more lattice nodes. This is what plains are made of.

`world.wgsl:564` — amplitude is `abs(Gaussian)`:

```
    var amp = abs(sample_gaussian(node_seed, WAVE_PROP_AMP, band.amp_mean, band.amp_sigma));
```

Widening σ yields **both** more faint nodes and rare tall ones from one knob.
Box-Muller truncates at ±3σ (`world.wgsl:406`), so the tail is bounded.

### No unnamed side effect

`activation` also accumulates a per-node `complexity` channel
(`world.wgsl:680-681`), which **would** have been a colour coupling. It is
dead: `PALETTE_COMPLEXITY` is a pinned `0.5` that every palette call site reads
instead (`world.wgsl:1780`, and the comment there names all five sites), and
the patch generator states outright that the complexity slot "is no longer
written — no reader". These two dials move height and nothing else.

Collision follows the risers: the pawn and camera read the same
`TERRAIN_BANDS` walk, through `terrain_height_at` /
`ground_formed_with_complexity`, that the render path bakes.

### For the ledger, not edited

`WAVE_THRESHOLD[6]` = `0.85, 0.70, 0.50, 0.35, 0.20, 0.90`, with
`WAVE_THRESHOLD_SOFTNESS = 0.15`. Bands 2 and 3 sit at **0.50** and **0.35** —
both wake at moderate pool activity, so the gate these dials answer to is
already open across most of the map.

### RESERVE — do not apply without a ruling

`damping_mean` on the same two bands shortens reach and makes risers more
isolated. Second pass if this one reads too broad.

---

## A10 — GoL SKEWED SLOW

### STOP GATE PASSED — live weights matched the ruling's left column, in both rooms

**CPU/GPU twin, both rooms, one commit:** `bodies/gol_zones.hpp:177` and
`realization/world.wgsl:2071`.

| # | tier | tick_period (beats) | old w | new w |
|---|------|--------------------:|------:|------:|
| 0 | Pillars | 8.00 | 0.10 | **0.14** |
| 1 | Sparse | 2.00 | 0.20 | **0.22** |
| 2 | Moderate | 1.00 | 0.18 | **0.12** |
| 3 | Dense | 0.50 | 0.10 | **0.04** |
| 4 | Flash | 0.25 | 0.17 | **0.03** |
| 5 | Monolith | 12.00 | 0.12 | **0.15** |
| 6 | Glacier | 4.00 | 0.13 | **0.30** |

**Arithmetic, recomputed from the tables after the edit rather than taken from
the ruling:**

- sum old = **1.000000**, sum new = **1.000000**
- weighted mean period **3.4325 → 4.7075** beats
- modal tier is now **Glacier** at 0.30, clear of Sparse at 0.22

**Twin verified mechanically** after the edit: all twelve float columns
compared row by row across the two tables — identical on all seven rows, as
are `force_no_height` and `grid_cells`.

`tick_period` is in **BEATS** at its consumer: `world.wgsl:2042` declares
`tick_period_mean: f32,  // beats between generations`, and the value is drawn
through `sample_gaussian(seed, ZONE_PROP_TICK_PERIOD, …)` into the zone config
the tick reads. Weights are consumed as a normalized weighted pick — the
selector walks `cumul += GOL_TIERS[t].weight` against a hash roll, in both
rooms (`world.wgsl:6407`, `8774`; `gol_zones.hpp:436`).

### Note, named rather than hidden

**Flash is the only `force_no_height` row.** Starving it 0.17 → 0.03 makes flat
GoL zones rare. That is a consequence of the weight shift, not a separate
ruling, and it is stated here so it is not discovered later as a surprise.

### Pulse untouched

`GOL_PULSE_TIERS` is a separate three-row table (Breathe / Sparkle / Drift)
with its own profile type and its own selection roll, reached only through
`GOL_PULSE_ALGORITHM_CHANCE`. Byte-identical after this campaign, in both
rooms.

---

## WHAT JEAN STILL HOLDS

Every gate below is visual or build, and none of them was run here.

- **A4** — no purple under the pawn, on GoL cells or anywhere; the aura's
  height extrusion behaves as today.
- **A1** — cubes read as polychrome; the near-uniform minority survives.
- **A3** — conventional-pawn FPV pixel-identical to today (the float
  arithmetic is witnessed above, but the *render* is not); taller and shorter
  figures sit proportionally.
- **A8** — no cactus or blade pierces or crowds a ceiling; outdoor sizes
  unchanged; nothing inflates.
- **A5** — fewer dead-flat stretches; isolated sharp risers at the 30 wu and
  12 wu scales; no uniform rubble; large landforms unchanged; **walk a
  riser — collision follows.**
- **A10** — GoL mostly slow, occasionally quick; the 4-beat tier is plainly
  the common case; Pulse zones unchanged.

## CARRIED FORWARD

1. **A4** — `effect_mask` is a dead uniform field in both rooms, now tagged
   `STATUS: INTENT`. So is `aura_n`. Removing them is a struct change; no task
   named one. For a pruning pass.
2. **A1** — the `ZOETROPE_FACE_SPLAY` coupling drives delivered
   `face_variance` to ~2.0 under a full-intensity strike, well past the
   `[0,1]` clamp. The dial for controlled hue is the splay, not `CUBE_TIERS`.
3. **A5** — `damping_mean` on bands 2 and 3 is the named second pass.
4. **A8** — the `CACTUS_TIERS` UNITS comment calls `LEAN` radians where the
   mesh uses it as a fraction of height. Harmless to A8; worth a look.
