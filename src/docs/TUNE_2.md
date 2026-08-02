# TUNE_2 — TWO PERCEPTS, TABLE WORK

Trunk-based, master only. One commit per task, in order — B1, B2 — then this
ledger. Cut from master `4d502df` (the TUNE_1 corrections commit).

**Both landed.** One STOP clause fired and was handled as its own text
instructs (report, then bind); the round's single named exception went
**unused**, because the site it was written for no longer exists.

| # | commit | task | verdict |
|---|--------|------|---------|
| 1 | `e8d170d` | B1 silence every pawn ground tint | LANDED — exception unused |
| 2 | `e1fe2df` | B2 rescale cube face excursion | LANDED — REST ≠ 1.0, reported |

Census scope: `src/cartridges/the_board/**` and `src/incubator_dual.cpp`,
boundary-delimited. Jean holds all build and visual gates. No build is
possible in this container (Dawn is not vendored) and **no render was seen**.

---

# B1 — THE PAWN DOES NOT TINT THE GROUND

TUNE_1 A4 silenced one producer and claimed the percept. This censuses the
**effect** first, then binds.

## THE PRODUCER LIST

Everything in the tree that puts a pawn-proximate colour into a ground colour
value:

| # | producer | constant | strength dial | status |
|---|----------|----------|---------------|--------|
| 1 | Pawn FF tint, `patch_terrain_fs` | `ZONE_PAWN_TINT` | `ZONE_PAWN_TINT_STRENGTH` | **SILENCED here** 0.6 → 0.0 |
| 2 | Pawn aura tint, aura kernel → `aura.gba` | `tint_r/g/b` | `PAWN_AURA_DEFAULT.tint_strength` | already 0.0f (TUNE_1 A4), **confirmed still 0.0f** |
| 3 | `zone_extrusion_fs`'s own copy of (1) | `ZONE_PAWN_TINT` | `ZONE_PAWN_TINT_STRENGTH` | **RETIRED** in `98982c1` [U4a] |

**Producer 1 — the live one.** `world.wgsl`:

```
base_color = mix(base_color, ZONE_PAWN_TINT, pawn_ff * ZONE_PAWN_TINT_STRENGTH * color_val);
```

Guard chain, outermost first: `tag_alpha > 0.001` → `CELL_ANIM_GOL` bit set →
camera `fade > 0.01` (`GOL_FADE_NEAR`/`GOL_FADE_FAR`) → fragment inside an
active zone's bounds → `color_val > 0.01` → `pawn_ff > 0.01`. It is a
subordinate tenant of GoL, scaled by `color_val`, exactly as the charter's C6
table describes.

**Producer 2 — confirmed still silent.** `PAWN_AURA_DEFAULT.tint_strength` is
`0.0f`. It is multiplied into `color_blend` in the aura compute kernel, so the
GBA channels the texture carries are exactly zero, and the FS add
`base_color + aura.gba` contributes nothing.

**Producer 3 — retired, not silenced.** See the C6-F1 verdict below.

## NOT A TINT, AND DELIBERATELY STILL LIVE

`patch_terrain_fs` also does this, inside the aura block:

```
    let height_boost = aura.r * 0.15;
    base_color = clamp(base_color + vec3(height_boost), vec3(0.0), vec3(1.0));
    normal = normalize(normal + vec3(0.0, aura.r * 0.3, 0.0));
```

It adds the **same amount to R, G and B**, so it moves brightness and not hue
— it is not a tint, and it is part of the height effect this ruling preserves
("the height extrusion behaves as today"). Named here so it is not mistaken
for a fourth tint by the next reader who sees the pawn change the ground's
appearance.

> **Reported, not treated as a STOP.** The `0.15` and `0.3` literals have no
> named dial — the charter flagged them "FS magic 0.15/0.3 unpaneled" and that
> is still true. B1's STOP is for a producer of the *tint* that no dial gates;
> this produces no hue, so it does not trip it. If Jean wants the pawn to make
> **no** visible difference to the ground at all, this is the remaining item
> and it needs a ruling.

## C6-F1 — CLOSED BY DELETION, AND THE CHARTER IS STALE TWICE

The charter (`src/docs/old docs/terrain_program_charter.md` §C6) says:

> **C6-F1:** … The extrusion FS applies the same tints UNSCALED (L8139/8145) —
> an inconsistency to rule on.

**Both halves fail against the live tree.**

1. **There is no extrusion FS.** `98982c1 [U4a]` retired `zone_extrusion_fs`,
   `zone_extrusion_vs`, `shadow_zone_extrusion_vs`, `apply_gol_extrusion_color`,
   `zone_mesh_gen_cell`, `zone_gol_mesh_gen` and the rest of the separate
   extrusion mesh path. GoL is the ground now, so there is no second fragment
   shader to keep in step. `block_color` — the variable that block held — has
   **zero hits** in the tree.
2. **It was not unscaled by the end anyway.** The retired site's final form,
   from that commit's diff:

   ```
   -        block_color = mix(block_color, ZONE_PAWN_TINT, pawn_ff * ZONE_PAWN_TINT_STRENGTH * color_val);
   -        block_color = mix(block_color, ZONE_SPHERE_TINT, sphere_ff * ZONE_SPHERE_TINT_STRENGTH * color_val);
   ```

   Both read their strength constants. The charter's own parenthetical says
   C6-F1 "was harmonized the same commit"; that harmonization is real, and the
   flag above it was never updated.

## THE NAMED EXCEPTION WENT UNUSED

B1 authorized exactly one creation this round: inserting
`ZONE_PAWN_TINT_STRENGTH` into an extrusion mix that applied the tint without
it. **No such mix exists**, on two independent grounds above. Nothing was
inserted. The round's one exception is spent on nothing, and `world.wgsl` is
one changed number plus a comment.

## THE UNAPPLIED SPHERE MIRROR

The instruction asked for the one-line edit that would mirror the exception on
the sphere side, reported and unapplied. **The mirror as literally specified
does not exist** — it presupposed the extrusion site. The analogous *live*
edit, in the same block, is:

```
const ZONE_SPHERE_TINT_STRENGTH: f32 = 0.5;   →   const ZONE_SPHERE_TINT_STRENGTH: f32 = 0.0;
```

That would silence the gold near the sphere. **Not applied. Not ruled.** The
sphere keeps its 0.5.

## THE FOLD

`ZONE_PAWN_TINT_STRENGTH` is a compile-time `const`, so the mix becomes
`mix(base_color, TINT, x * 0.0 * y)` = `mix(c, t, 0.0)` = `c` — an identity
assignment. The store dies, the `if` body empties, `pawn_ff` becomes unused,
and the `zone_pawn_ff` call goes with it. `pawn_ff` derives from a
`smoothstep` and so cannot be NaN, which is what makes the `x * 0` fold
legitimate rather than a fast-math assumption.

**Expected: an absent term, not dead per-pixel work.** Not verified by
disassembly — no build is possible here, so this is a reading of the code and
of what any optimizer must do with it, not a measurement.

## FOR THE LEDGER — NO EDIT

```
const PAWN_FORCEFIELD_RADIUS_STATIONARY: f32 = 6.0;  // Radius when not moving
const PAWN_FORCEFIELD_RADIUS_MOVING: f32 = 2.0;      // Radius at max speed
const PAWN_FORCEFIELD_FALLOFF: f32 = 2.0;            // Edge softness (smoothstep width)
const PAWN_FORCEFIELD_SPEED_SCALE: f32 = 1.0;        // How quickly radius shrinks with speed
```

`radius = mix(STATIONARY, MOVING, speed_factor)`, so the radius runs from
**2.0 while walking to 6.0 at a standstill** — a 3× widening, on a smoothstep
±2.0 wide. That is why the pool grew when Jean stopped. Untouched.

## OTHER PAWN-PROXIMITY READERS — CHECKED, NOT TINTS

`pawn_gol_suppression` (three callers) scales the VS **cell lift** only —
`lift * (1.0 - suppression)`, height, never colour. `patch_terrain_fs` is the
only site in the tree that composes terrain colour: `apply_gol_color` has one
caller, and every `base_color` past line 9000 belongs to orbs.

---

# B2 — CUBE FACE COLOR, TASTE PASS

## STOP CLAUSE 1 — PASSED

All five listed values matched the live tree before the edit: SmallCube
`{0.40, 0.12}`, MedCube `{0.45, 0.15}`, LargeCube `{0.35, 0.10}`, Monolith
`{0.45, 0.12}`, and `ZOETROPE_FACE_SPLAY = 1.50f`.

## STOP CLAUSE 2 — FIRED, REPORTED, THEN BOUND

```
inline constexpr float ZOETROPE_FACE_SPLAY = 1.50f;  // added face_variance at full I
inline constexpr float ZOETROPE_FACE_REST  = 1.20f;  // × the spawn draw, in formation
```

**`ZOETROPE_FACE_REST` is 1.20f, not 1.0.** The clause said to report it and
the recomputed rest-delivered figures *before* binding, which is what the
commit message and this section do.

The assumption it guards — "the tier draw arrives at rest substantially
intact" — is **wrong in the safe direction**: in formation the draw arrives
**20% hot**, amplified rather than attenuated. Recomputed with the real 1.20
(the `rest formation` rows below), the new column still clips at 0.0% at μ, so
the ruling's premise survives its own broken assumption. Bound as instructed.

## DELIVERED face_variance IS ASSEMBLED AT TWO SITES, AND THEY DIFFER

```
cube_write_gpu:          fe.face_variance = inst.params[CubeIdx::FACE_VARIANCE]
                                          + ZOETROPE_FACE_SPLAY * zoetrope_cell_intensity(...)

zoetrope_project_slot:   gpu.upload_cube_face_variance(queue, slot,
                             cbs.activeCubes_[slot].face_variance * ZOETROPE_FACE_REST
                             + ZOETROPE_FACE_SPLAY * I);
```

`zoetrope_project_slot` early-returns in `Formation::ROAM`, so a **roaming**
cube carries the bare draw and only a cube **in formation** takes the ×1.20.
`I = min(1, excite + 0.45 × pigment)`, so `I ∈ [0, 1]` and full strike is
`I = 1`.

## THE NUMBER THIS PASS WAS AIMED AT

The per-channel delta is uniform in `[-V, +V]` where `V` is delivered
`face_variance`, applied to base colours from `cube_compute_colors`:

```
inst.colors[0] = hash * 0.55f + 0.35f;   // R ∈ [0.35, 0.90]
inst.colors[1] = hash * 0.50f + 0.30f;   // G ∈ [0.30, 0.80]
inst.colors[2] = hash * 0.60f + 0.20f;   // B ∈ [0.20, 0.80]
```

`clip` = P(a draw hits the `[0,1]` clamp); `keep` = the fraction of the
intended excursion magnitude that survives it. Both averaged over the three
channels at a mid-range base.

| tier | state | V before | clip | keep | V after | clip | keep |
|---|---|---:|---:|---:|---:|---:|---:|
| SmallCube | rest ROAM μ | 0.400 | 1.0% | 99.9% | 0.180 | 0.0% | 100.0% |
| SmallCube | rest ROAM μ+3σ | 0.760 | 34.2% | 87.3% | 0.360 | 0.0% | 100.0% |
| SmallCube | rest formation μ | 0.480 | 4.7% | 99.1% | 0.216 | 0.0% | 100.0% |
| SmallCube | rest formation μ+3σ | 0.912 | 45.2% | 78.9% | 0.432 | 2.2% | 99.7% |
| SmallCube | **strike ROAM μ** | 1.900 | 73.7% | 45.5% | 0.580 | 15.1% | 96.4% |
| SmallCube | **strike formation μ** | 1.980 | 74.7% | 44.0% | 0.616 | 19.1% | 94.9% |
| SmallCube | strike formation μ+3σ | 2.412 | 79.3% | 37.1% | 0.832 | 39.9% | 83.2% |
| MedCube | rest ROAM μ | 0.450 | 2.8% | 99.5% | 0.200 | 0.0% | 100.0% |
| MedCube | rest ROAM μ+3σ | 0.900 | 44.4% | 79.5% | 0.410 | 1.4% | 99.9% |
| MedCube | rest formation μ | 0.540 | 10.3% | 97.8% | 0.240 | 0.0% | 100.0% |
| MedCube | rest formation μ+3σ | 1.080 | 53.7% | 70.6% | 0.492 | 5.4% | 98.9% |
| MedCube | **strike ROAM μ** | 1.950 | 74.4% | 44.5% | 0.600 | 17.4% | 95.6% |
| MedCube | **strike formation μ** | 2.040 | 75.5% | 42.9% | 0.640 | 21.9% | 93.7% |
| MedCube | strike formation μ+3σ | 2.580 | 80.6% | 34.9% | 0.892 | 43.9% | 79.9% |
| LargeCube | rest ROAM μ | 0.350 | 0.0% | 100.0% | 0.160 | 0.0% | 100.0% |
| LargeCube | rest ROAM μ+3σ | 0.650 | 23.1% | 93.2% | 0.310 | 0.0% | 100.0% |
| LargeCube | rest formation μ | 0.420 | 1.8% | 99.8% | 0.192 | 0.0% | 100.0% |
| LargeCube | rest formation μ+3σ | 0.780 | 35.9% | 86.1% | 0.372 | 0.0% | 100.0% |
| LargeCube | **strike ROAM μ** | 1.850 | 73.0% | 46.6% | 0.560 | 12.6% | 97.2% |
| LargeCube | **strike formation μ** | 1.920 | 74.0% | 45.1% | 0.592 | 16.5% | 95.9% |
| LargeCube | strike formation μ+3σ | 2.280 | 78.1% | 38.9% | 0.772 | 35.2% | 86.6% |
| Monolith | rest ROAM μ | 0.450 | 2.8% | 99.5% | 0.200 | 0.0% | 100.0% |
| Monolith | rest ROAM μ+3σ | 0.810 | 38.3% | 84.4% | 0.380 | 0.2% | 100.0% |
| Monolith | rest formation μ | 0.540 | 10.3% | 97.8% | 0.240 | 0.0% | 100.0% |
| Monolith | rest formation μ+3σ | 0.972 | 48.6% | 75.8% | 0.456 | 3.2% | 99.5% |
| Monolith | **strike ROAM μ** | 1.950 | 74.4% | 44.5% | 0.600 | 17.4% | 95.6% |
| Monolith | **strike formation μ** | 2.040 | 75.5% | 42.9% | 0.640 | 21.9% | 93.7% |
| Monolith | strike formation μ+3σ | 2.472 | 79.8% | 36.3% | 0.856 | 41.6% | 81.9% |

**The defect was exactly where the ruling said it was.** At rest the old
numbers were only mildly over — 1–3% clipping at μ. Under a **strike**, three
draws in four hit the clamp and **less than half** the intended excursion
survived. A delta that clips on two channels and not the third is precisely
"clipped RGB is primaries". After the bind a strike clips 13–22% and keeps
94–97%; rest clips essentially not at all, and even the μ+3σ tail in formation
is down from 45–54% to 0–5%.

## THE BIND

```
CUBE_TIERS FACE_VARIANCE {μ, σ}          ZOETROPE_FACE_SPLAY
  0 SmallCube  {0.40, 0.12} → {0.18, 0.06}    1.50f → 0.40f
  1 MedCube    {0.45, 0.15} → {0.20, 0.07}
  2 LargeCube  {0.35, 0.10} → {0.16, 0.05}
  3 Monolith   {0.45, 0.12} → {0.20, 0.06}
```

Both in one commit — one percept, one excursion. Nothing else in `CUBE_TIERS`
moved: the diff is the **9th `{μ, σ}` pair** of each row (confirmed
`CubeIdx::FACE_VARIANCE == 8`) and one constant. The `monolith_vs` arithmetic
from TUNE_1 A1 is untouched — the mechanism was right, only its scale was
wrong.

### σ:μ — one correction of record

The ruling says μ:σ ratios are preserved on purpose. They are preserved
**approximately, not exactly**:

| tier | σ/μ before | σ/μ after | μ scale |
|---|---:|---:|---:|
| SmallCube | 0.3000 | 0.3333 | 0.450 |
| MedCube | 0.3333 | 0.3500 | 0.444 |
| LargeCube | 0.2857 | 0.3125 | 0.457 |
| Monolith | 0.2667 | 0.3000 | 0.444 |

μ scales by a near-uniform 0.444–0.457, but σ/μ **rises in every row**. The
drift runs the helpful way: a wider *relative* spread means more low draws, so
the near-uniform minority Jean asked to keep is preserved and very slightly
enlarged.

## RETUNE

Two knobs, one line each, both linear in the delta:

- **rest** → the `FACE_VARIANCE` column in `CUBE_TIERS`
- **strike** → `ZOETROPE_FACE_SPLAY`

---

## WHAT JEAN STILL HOLDS

- **B1** — walk onto a live GoL zone and stop. No purple, on cells or
  extrusions, at any speed. Toggle the aura on (numpad 3): still no purple,
  and the height extrusion behaves as today.
- **B2** — at rest, faces read as plainly different hues, muted rather than
  primary, with occasional near-uniform cubes surviving. Under a strike, faces
  splay further without going to pure primaries, white, or black.

## CARRIED FORWARD

1. **The aura's height brighten is unpaneled.** `aura.r * 0.15` and the
   `aura.r * 0.3` normal perturb have no named dial. Achromatic, so B1's
   ruling does not reach them — but they are the only remaining way the pawn
   changes the ground's appearance. Needs a ruling if "no pawn-proximate
   effect on ground colour" is meant literally.
2. **`ZONE_SPHERE_TINT_STRENGTH` stays 0.5.** The sphere's gold is unruled;
   the one-line edit is recorded above, unapplied.
3. **The charter's C6 section is stale.** C6-F1 describes a shader that no
   longer exists and an inconsistency that was harmonized before deletion.
   The charter is in `old docs/` and archived by its folder, so this is a note,
   not a defect.
4. **`ZOETROPE_FACE_REST = 1.20f` is a third, unruled contributor** to
   delivered `face_variance` — it amplifies the tier draw by 20% for cubes in
   formation. B2 moved the other two. If rest-in-formation still reads hot
   after the visual gate, this is the knob nobody has touched.
