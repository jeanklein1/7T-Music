# CONTACT_4 — CAMPAIGN LOG

Campaign: CONTACT_4 (the scale batch — the scale ledger, the three
corrections, the shell instrument; GPU CAMPAIGN / S 0-3). Handoffs
`src/docs/HANDOFFS/GPU CAMPAIGN/S 0-3/`. Execution: on master, metadata
trailers. No new bindings; no new config fields.

## WHY THIS BATCH

CONTACT_2/3's mechanisms are correct (sign, seam, servo, split — all
verified). What is wrong is GEOMETRY: three influence radii were
authored in the wrong space, and no instrument has power over "is 8 wu
the right radius." The three deaths, verified in-tree:

1. **Agents uncatchable.** The point-source flee gate is
   `ppr = personal_radius(30) + point_bubble_radius(20) = 50 wu` (one
   full `PATCH_EXTENT`). Inside it the escape is a velocity FLOOR with
   gain ≥ 1.0 and NO proximity term ⇒ agents hold ~18 wu/s away from a
   15 wu/s player from a patch away, forever. Worse than uncatchable:
   `POSSESSION_RADIUS = 20`, so an agent fleeing from 50 wu can never be
   reached to possess. K2a's `FLEE_SHELL_FRAC` was scoped to the
   body-to-body gate and never touched this path.
2. **Cubes never react.** `CUBE_PART_RADIUS = 8`, but cubes float well
   above the pawn and the C1a gate is 3D (`pd2 = pdx²+pdy²+pdz²`), so
   `pdy` alone exceeds 8 ⇒ `pd2 > 64`, the gate CANNOT FIRE at any
   lateral distance. The cube-vs-pawn contact radius (3.0 + 1.6 = 4.6)
   is deader still. C1a and K2b are each right alone and jointly fatal.
3. **Spheres shove the player.** `update_player_agent`'s sphere loop
   uses `r = contact_radius(1.6) + CONTACT_SPHERE_RADIUS(12) = 13.6` and
   NO mass weighting — the player takes the full impulse. `CONTACT_SPHERE_RADIUS`
   is the sphere's INFLUENCE field (colour/terrain range), not its body
   (~1.2–1.5 wu). That is the T2b spec as written; K1 only made it
   visible.

**THE LAW:** every influence radius is declared beside the world
dimension it derives from, with its arithmetic written out.

---

## S0 — INDEX + PREFLIGHT

### Base

`df3d8e8` (master trunk) — the CONTACT_3 tip (b977e4b) + the S handoff
commit. Baseline glaw1: **G-LAW 1: GREEN**.

### The world's scales (the reference column)

| Scale | Const | Value | file:line |
|---|---|---|---|
| mosaic cell | `PATCH_CELL_SIZE` | 3.125 wu | surface_services.hpp:70 |
| patch | `PATCH_EXTENT` / `Dim::PATCH_EXTENT` | 50 wu | world.wgsl:253 / state.hpp:80 |
| the bubble | `config.point_bubble_radius` | 20 wu | (C3a; = contracts/point.hpp) |
| possess reach | `POSSESSION_RADIUS` | 20 wu | agents.hpp:107 |
| agent eviction | `AGENT_EVICTION_RADIUS` | 350 wu | world.wgsl:7079 |
| floater eviction | `FLOATER_EVICTION_RADIUS` | 400 wu | world.wgsl:7111 |
| veil ring / LOD0 | `config.veil_ring` / `lod0_radius` | 325 / 175 wu | world.wgsl:1667 / 1670 |
| pawn forcefield | `PAWN_FORCEFIELD_RADIUS_*` | 6 / 2 wu | world.wgsl:2163 / 2164 |
| sphere body (per-instance) | `SPHERE_TIERS` BODY_RADIUS μ | ~1.2–1.5 wu | spheres.hpp:136/141 |

### ⚠ THE CRITICAL NUMBER — cube altitude (S2b derives CUBE_PART_RADIUS from it)

Cube `orbit_height` is NOT a literal — it is a per-tier Gaussian
(`CUBE_TIERS`, cube_behaviors.hpp:497), clamped ≥ 3.0:

| Tier | orbit_height μ,σ | body_radius μ | bob μ |
|---|---|---|---|
| SmallCube | **25, 20** | 1.8 | 1.0 |
| MedCube | **45, 30** | 4.0 | 1.5 |
| LargeCube | **75, 45** | 8.0 | 2.0 |
| Monolith | **12, 8** | 3.0 | 1.2 |

**The cap is MOOD-DEPENDENT** (traced through `cube_apply_indoor_rescale`
→ `cap_to_ceiling`, contracts/indoor_module.hpp):
- **Indoor moods only** (`MOOD_INDOOR_FLAT` ceiling 20, `MOOD_INDOOR_VAULT`
  ceiling 25): the cube's TOTAL vertical extent (orbit + half-height +
  bob) is capped to `INDOOR_HEIGHT_CAP_FRACTION(0.75) × ceiling` =
  **15 (flat) / 18.75 (vault)** wu. The cube CENTER (what the 3D gate
  measures, `pdy`) sits strictly below that. So indoors, `H_max < 18.75`.
- **Outdoor moods:** the cap NEVER fires — `orbit_height` is the raw
  Gaussian (means 25/45/75). Outdoor cubes float at **25–75+ wu**.

**PROBLEM SPOTTED (beyond the handoff):** the handoff assumed a single
`H_max ≈ 18` (its worked example: 18 + bob 3 + 10 ⇒ 31). That is the
INDOOR case only. Outdoors, no sane radius reaches a cube at 25–75 wu.

**RESOLUTION (S2b):** derive `CUBE_PART_RADIUS` from the indoor cap (the
handoff's clear intent) = `0.75 × VAULT(25) = 18.75` wu envelope +
~10 wu lateral ⇒ **≈ 30 wu** (between the bubble 20 and the patch 50,
as S3's note expects). Flag the outdoor limitation in the ledger and the
report; the per-instance alternative (`fe.orbit_height + margin`, which
would reach outdoor cubes too) is noted deferred. The S3 shell instrument
makes this scale visible for Jean's judgment.

### The S2/S3 edit sites (verified)

- **flee_gain_player rows** (agents.hpp:174): worker 1.2 · scout 1.4 ·
  sentinel 1.0 · leader 1.1 — all ≥ 1.0 (the catchability bug). S2a →
  {0.70, 0.85, 0.50, 0.60}.
- **The point-source flee block** + the **sphere loops** live in both
  walker kernels (from C3b/S). `update_player_agent`'s sphere loop is
  the ONLY `floating_entities` use in that kernel (verified) ⇒ S2c's
  removal drops binding 100 from its closure (S3b prediction confirmed
  in advance).
- **`CONTACT_SPHERE_RADIUS = 12`** (world.wgsl:2172) — the sphere's
  influence radius, not its body; S2c retires it (grep for other
  readers at edit time).
- **Terrain FS** `patch_terrain_fs` (world.wgsl:4155) + the
  `LIVE_CARD_DEBUG_VIEW` const (world.wgsl:258) — the S3 host.
  `PATCH_EXTENT`, `config.lod_point_x/z`, `in.world_pos` all visible in
  the FS scope.

All resolved → S1. glaw1 baseline GREEN.

---

## S1 — THE SCALE LEDGER

### [S1a] — 11ea14e (documentation only, zero behavior)

THE SCALE LEDGER above the contact/steering const cluster: the world's
scales as the reference column, every influence radius as
{value·reference·derivation}, and THE GATE-FEASIBILITY RULE (a 3D gate
against a body at altitude H fires only if R > H; reach = sqrt(R²−H²)).
The three deaths written as `[DEAD]`/`[UNREACHABLE]`/`[WRONG SPACE]`
rows. Worked derivation comments on every influence-radius const + the
point-source composed radius. `git diff` comment-only (every const VALUE
byte-identical). glaw1 GREEN.

---

## S2 — THE THREE CORRECTIONS

### [S2a] — 7aa84cd (point-source flee: shell, falloff, gains)

`ppr = config.point_bubble_radius` (20, was 30+20=50 — dropped the BODY
shell from a PRESENCE term). Added the proximity falloff
`prox = clamp(1−pd/ppr,0,1)` (full at contact, nil at the edge —
`behavior_flee`'s shape). `flee_gain_player < 1` every row {0.70, 0.85,
0.50, 0.60} — the CATCHABILITY LAW (a floor gain ≥ 1 is uncatchable and
unpossessable). Worked: worker at 6 wu in a 20 bubble → gap closes at
~8.7 wu/s (catchable); at the edge, no reaction. glaw1 GREEN.

### [S2b] — bec106c (cube parting: derived radius, 3D falloff)

`CUBE_PART_RADIUS = 30` DERIVED from the indoor ceiling cap
(`INDOOR_HEIGHT_CAP_FRACTION 0.75 × VAULT 25 = 18.75` wu envelope + ~11
lateral; reach under the tallest indoor cube ≈ 23.4 wu). Falloff now uses
the 3D distance the gate uses (was planar — disagreed with the 3D gate).
Worked: player beneath a cube at ~18 wu → before: gate dead; after
(R=30): prox 0.40, force 6.0 → swings aside. **OUTDOOR CAVEAT** (spotted
at S0): the cap fires INDOORS only; outdoor cubes (raw Gaussian 25/45/75)
stay beyond 30 — per-instance radius is the deferred fix. glaw1 GREEN.

### [S2c] — bedcb2f (spheres: delete player loop; body radius; tombstone)

The player sphere loop DELETED (spheres do not move the point's body —
Jean's ruling). The agents' loop uses `fe.body_radius` (per-instance
~1.2–1.5 wu) instead of `CONTACT_SPHERE_RADIUS` (the INFLUENCE field 12,
~8× the body). `CONTACT_SPHERE_RADIUS` tombstoned (reference-free).
Closes the per-slot-floater-radii deferred item. glaw1 GREEN; Dawn
witness ALL FAMILIES GREEN.

---

## S3 — THE SHELL INSTRUMENT + CLOSEOUT

### [S3a] — c396aea (the shells, drawn on the ground)

`CONTACT_SHELL_DEBUG` (0u) + `SHELL_RING_WIDTH` (0.35). In
`patch_terrain_fs`, after the final colour compose, the rings blend over
the world: bubble (cyan, 20), cube parting (amber, 30), one patch (grey,
50 — the yardstick). Zero bindings, zero layout (uses `config.lod_point_x/z`
+ module consts already in the FS). glaw1 GREEN; witness ALL FAMILIES GREEN.

### [S3b] — closeout

Instruments → `_post_c4`, diffed against the S0 base `df3d8e8`:
- **cc6:** zero substantive layout diffs; flags EMPTY.
- **cc7:** 96 → 96 declarations (+0) — `CONTACT_SPHERE_RADIUS` was a
  const (not a declaration); the shell consts likewise.
- **cc7 mirror:** zero orphans both directions.
- **cc4:** `update_player_agent` **LOST binding 100** (floating_entities)
  with the sphere loop — exactly as predicted; the other three kernels
  unchanged. The player kernel now uses fewer bindings than the CE
  layout provides (allowed; witness confirms).
- **Dawn witness** (`_post_c4`): ALL FAMILIES GREEN, zero module messages.

Encoding sweep: world.wgsl / agents.hpp no-BOM, LF-only; FXC banner
byte-untouched. Final glaw1 GREEN.
