# CONTACT_4 — BATCH REPORT

Campaign: CONTACT_4 (the scale batch — the ledger, the three
corrections, the shell instrument; GPU CAMPAIGN / S 0-3). This is the
batch's witness stand (S3).

## Base + final

- Base: `df3d8e8` (master trunk) — the CONTACT_3 tip (b977e4b) + the S
  handoff commit. Executed on master, metadata trailers.
- Commit list:

| Commit | Hash | Intent |
|--------|------|--------|
| [S0]  | 945b2b5 | Preflight: the scale diagnosis; anchors a–f; the cube-altitude finding; baseline glaw1 |
| [S1a] | 11ea14e | The scale ledger (comment-only, zero behavior) |
| [S2a] | 7aa84cd | Point-source flee: bubble shell + proximity falloff + catchable gains |
| [S2b] | bec106c | Cube parting: derived radius (30) + 3D falloff |
| [S2c] | bedcb2f | Spheres: delete player loop; agents use `fe.body_radius`; tombstone |
| [S3a] | c396aea | The shell instrument (rings drawn on the terrain) |

## The diagnosis (three radii in the wrong space)

CONTACT_2/3's mechanisms are correct — the sign, seam, servo, split are
all verified. The failure was GEOMETRY: three influence radii were
authored in the wrong space, and no instrument had power over "is 8 wu
the right radius."

1. **Agents uncatchable** — the point-source flee gate was
   `personal_radius(30) + point_bubble_radius(20) = 50 wu` (a whole
   patch), a constant floor with gain ≥ 1 and no proximity ⇒ agents fled
   a walking player from a patch away, and `POSSESSION_RADIUS = 20`
   meant they could never even be reached to possess.
2. **Cubes unreachable** — `CUBE_PART_RADIUS = 8` with a 3D gate against
   cubes floating well above the pawn: `pdy` alone exceeded 8, so the
   gate could NEVER fire.
3. **Spheres shoved the player** — the player's sphere loop used
   `CONTACT_SPHERE_RADIUS = 12` (the sphere's INFLUENCE field, ~8× its
   ~1.5 wu body), unweighted.

**THE LAW installed:** every influence radius is declared beside the
world dimension it derives from, with its arithmetic written out (the
scale ledger, S1).

## The three corrections (each with its worked example)

- **S2a — the point-source flee.** `ppr = config.point_bubble_radius`
  (20, the presence shell; `personal_radius` is a body shell and was
  double-counted). Added the proximity falloff `prox = clamp(1−pd/ppr,
  0, 1)` (full at contact, nil at the edge — `behavior_flee`'s shape).
  `flee_gain_player < 1` every row (the CATCHABILITY LAW). Worked:
  worker at 6 wu in a 20 bubble → demand `15·0.70·0.70 = 7.35`, recedes
  ~6.3 while the player advances 15 ⇒ gap closes ~8.7 wu/s (catchable);
  at the edge, prox 0 ⇒ no reaction. Before: constant 18 wu/s out to 50.
- **S2b — the cube parting.** `CUBE_PART_RADIUS = 30`, DERIVED from the
  indoor ceiling cap (`INDOOR_HEIGHT_CAP_FRACTION 0.75 × VAULT 25 =
  18.75` wu envelope + ~11 lateral; reach under the tallest indoor cube
  ≈ 23.4 wu). Falloff switched to the 3D distance the gate uses. Worked:
  player beneath a cube at ~18 wu → before: `pd2 = 324 > 64`, gate dead;
  after (R=30): `prox 0.40`, `force 6.0` → swings aside.
- **S2c — the spheres.** The player sphere loop DELETED (spheres do not
  move the point's body). The agents' loop uses `fe.body_radius`
  (per-instance ~1.5 wu) instead of the retired `CONTACT_SPHERE_RADIUS`
  (12, the influence field). Tombstoned. Closes the per-slot-floater-
  radii deferred item.

## ⚠ PROBLEM SPOTTED (beyond the handoff) — the outdoor cube caveat

The handoff assumed a single `H_max ≈ 18` for the cube altitude (its
example: 18 + bob 3 + 10 ⇒ 31). Tracing the spawn arithmetic showed cube
`orbit_height` is a per-tier Gaussian (means **25 / 45 / 75 / 12 wu**),
and the ceiling cap that bounds it to ~18.75 wu **only fires in indoor
moods** (`MOOD_INDOOR_FLAT/VAULT`). So:
- **Indoors:** `H_max < 18.75` ⇒ `CUBE_PART_RADIUS = 30` reaches every
  cube. The handoff's intent holds.
- **Outdoors:** the cap never runs ⇒ cubes float at 25–75+ wu, beyond 30.

`CUBE_PART_RADIUS` was derived from the indoor cap (the handoff's clear
intent, matching its ~31 example) and the outdoor limitation flagged in
the ledger, this report, and — visibly — the S3 shell instrument. The
clean fix for outdoor cubes is a **per-instance radius**
(`fe.orbit_height + margin`, which reaches a cube at its own altitude);
it is in the deferred register.

## Recount vs expectations (`_post_c4`, vs the S0 base `df3d8e8`)

| Check | Expectation | Result | Verdict |
|---|---|---|---|
| cc6 flags / layouts | EMPTY / no deltas | EMPTY / zero substantive diffs | **MATCH** |
| cc7 declarations | +0 | 96 → 96 (the retired const + shell consts are not declarations) | **MATCH** |
| cc7 mirror | zero orphans | zero both directions | **MATCH** |
| cc4 closures | unchanged EXCEPT `update_player_agent` loses floating_entities/100 | exactly that: player LOST [100]; other/cube/sphere UNCHANGED | **MATCH** |
| Dawn witness | ALL GREEN | 19 families / 30 EPs GREEN, zero module messages | **MATCH** |
| Final glaw1 | GREEN | GREEN | **MATCH** |

`update_player_agent` now uses fewer bindings than the Compute Entity
layout provides (binding 100 unused) — allowed, and the witness confirms
the slimmer kernel validates. "No new bindings; no new config fields" —
confirmed.

## Deviations

- **D1 — on-master execution** (standing direction); metadata trailers.
- **D2 — cube radius derived from the INDOOR cap** (18.75), not a raw
  tier max: the handoff's own worked example (~31) confirms the indoor
  intent; outdoor cubes are the flagged limitation + deferred per-instance
  fix (see the caveat above).
- **D3 — S2b falloff authored 3D** (`sqrt(qd2)`), replacing K2b's planar
  falloff which disagreed with the 3D gate — per S2b item 5.

## Encoding sweep

world.wgsl / agents.hpp: **no BOM, LF-only.** The FXC constraints banner
is byte-untouched.

## THE UPDATED TUNING TABLE (now with a REFERENCE column — reviewable)

| Knob | Value | Reference | Where |
|---|---|---|---|
| point-source `ppr` | 20 (`config.point_bubble_radius`) | bubble/possess 20 | world.wgsl (S2a) |
| proximity falloff | `1 − pd/ppr` | — (reflex shape) | world.wgsl (S2a) |
| `flee_gain_player` (×4) | 0.70 / 0.85 / 0.50 / 0.60 | catchability (<1) | agents.hpp:174 |
| `FLEE_SHELL_FRAC` | 0.25 | bubble/possess 20 → 15 wu | world.wgsl:2251 |
| `personal_radius` (×4) | 30.0 | flock neighbor_radius 30 | agents.hpp:170 |
| `NONPLAYER_FLEE_GAIN` | 0.8 | dimensionless | world.wgsl:2241 |
| `CONTACT_SPRING` / `_IMPULSE_CAP` | 40 / 6 | units (Δv) | world.wgsl:2210 / 2211 |
| `contact_radius` / `contact_mass` (×4) | 1.6/1.4/2.0/1.8 · 1.0/0.8/1.5/1.2 | body | agents.hpp:168–169 |
| `PAWN_CONTACT_MASS_MULT` | 4.0 | dimensionless | world.wgsl:2226 |
| sphere loop radius | `contact_radius + fe.body_radius` | sphere body ~1.5 | world.wgsl (S2c) |
| `CUBE_PART_RADIUS` / `_CAP` / `_GAIN` | 30 / 12 / 1.0 | cube alt (indoor cap 18.75) | world.wgsl:2265–2267 |
| `CUBE_PLASTICITY_DEFAULT` (master) | 0.6 | dimensionless | state.hpp:306 |
| `CUBE_TIER_GAINS.plasticity` (×4) | 1.0 / 0.8 / 1.2 / 0.5 | per-tier λ character | cube_behaviors.hpp:92 |
| `STEER_LOOKAHEAD` / `_GAIN` / `_GRAD_LO` / `_HI` | 4 / 3 / 0.7 / 1.4 | cell 3.125 (lookahead) | world.wgsl:2231–2237 |
| `config.pawn_speed` (the flee's input) | `Idle::PAWN_SPEED` 15 | — | state.hpp:303 |
| `CONTACT_SHELL_DEBUG` / `SHELL_RING_WIDTH` | 0u / 0.35 | instrument | world.wgsl:264–265 |

## JEAN'S GATE LIST (Windows)

- [ ] **Build + boot.** Nothing structural changed; the player kernel got
      SMALLER (one loop removed).
- [ ] **THE SHELLS:** flip `CONTACT_SHELL_DEBUG = 1u`, stand still, look
      down. Read the rings against the grey patch ring (bubble 20 well
      inside; cube 30 between it and the patch 50). Revert after.
- [ ] **THE APPROACH:** walk at an agent — it should ignore you until the
      bubble, then dart aside, and you should be able to WALK IT DOWN and
      possess it (Caps Lock). The gate S2a exists for; if you still
      cannot close, report `flee_gain_player` and your pawn speed.
- [ ] **THE CROWD:** sprint through a cluster — a wake that settles; at
      rest, no reaction at all.
- [ ] **THE CUBES:** walk beneath one — it should swing aside and (with
      λ) keep some displacement; sweep a drone show and rearrange it.
      (Indoor cubes; outdoor cubes above ~30 wu remain beyond reach —
      the flagged caveat.)
- [ ] **THE SPHERES:** walk into a sphere's path — it must NOT push you.
      Watch an agent near one — it still gives way, now at the sphere's
      true body size.
- [ ] The whisper + geometry gates, unchanged.

## DEFERRED REGISTER (two items CLOSED by this batch)

- **CLOSED:** per-slot floater radii (S2c item 8 — the agents' sphere
  loop now uses `fe.body_radius`).
- **CLOSED:** the sphere-pushes-player behaviour (S2c item 7).
- **NEW / open:** outdoor-cube reachability — a per-instance cube parting
  radius (`fe.orbit_height + margin`) reaches cubes at their own altitude
  (the outdoor caveat above).
- still open: figure-scaled contact radius (binding 112 → COMPUTE
  visibility) · CPU-authored point velocity (`config.point_vel_x/z` —
  retires `BUBBLE_PART_SPEED`, restores "a still camera parts no one";
  the camera-host path also has no proximity-falloff input until this
  lands) · aggregate impulse clamp (the per-pair cap has no total) ·
  AgentState pad debt (96→112) · living-plateau steering · the bubble's
  coupling wire · the ribbon floor · emissaries · the frame-time
  instrument (timestamp queries — still the one unmeasured axis).

## Seeds

CONTACT_4 makes the influence radii honest — each declared beside its
reference scale, the three deaths fixed, and the shell instrument to
read scale in one frame. The contact quartet (CONTACT_1→4) is closed.
Still queued: the F campaign (sign + mask + topology split), the
coupling campaign, and the deferred register above.
