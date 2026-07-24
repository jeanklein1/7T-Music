# CONTACT_2 — BATCH REPORT

Campaign: CONTACT_2 (the composed-spheres completion — geometry law,
steering, the social split; GPU CAMPAIGN / C 0-4). This is the batch's
witness stand (C4).

## Base + final

- Base: `3252a6d` (master trunk) — the CLOSURE_PAWN 5-commit feature
  series + `Handoffs C 0-4`.
- **Execution note (deviation):** per Jean's direction the batch executed
  **directly on the master trunk**, not a `CONTACT_2` side-branch. Every
  commit carries `Campaign / Stage / Base / Trunk / Gates` metadata
  trailers. Pushed to origin/master at C3b (the stop-hook + ephemeral
  container favored preserving gated work); C4 closeout follows.
- Commit list:

| Commit | Hash | Intent |
|--------|------|--------|
| [C0]  | a752004 | Preflight: anchors a–h verified on the CLOSURE_PAWN-shifted tree; baseline glaw1 GREEN |
| [C1a] | fa690c9 | Geometry law: 3D distance gate, planar response (all 5 landed pairs) |
| [C1b] | 7210fcc | Cube plasticity: drift→anchor leak (λ=0 bit-neutral) |
| [C2a] | 61b849a | Gradient reader `sample_terrain_grad_at` + steering consts |
| [C2b] | 159a18b | Steering term (pawn + agents; branchless) |
| [C3a] | 4aa2555 | Tier shell columns + the bubble's config graduation |
| [C3b] | 7a3b10f | The flee servo, generalized + point-source (sign-corrected) |

## Recount vs expectations (`_post_c2`, diffed against the C0 base)

The instruments are diffed against the **C0 base `3252a6d`**, not the
prior campaign's `_post_tc1` — CLOSURE_PAWN landed between T3 and C0, so
its deltas belong to the base, not this batch.

| Check | Expectation (C4) | Result | Verdict |
|---|---|---|---|
| cc6 flags | EMPTY | EMPTY | **MATCH** |
| cc6 layout deltas | NONE (zero bindings added) | zero substantive diffs vs base | **MATCH** |
| cc7 declarations | +0 (unless the config mirror counts) | 96 → 96 (+0); the config field + tier columns are struct members, not `@binding` declarations | **MATCH** |
| cc7 mirror | tombstones parked; zero orphans | 90 matched, zero orphans both directions | **MATCH** |
| cc4 agent closures | unchanged in bindings | all four kernels byte-identical to the base (the new `point_pos`/`config.point_bubble_radius` refs were already bound) | **MATCH** |
| Dawn witness | ALL FAMILIES GREEN | 19 families / 30 EPs GREEN, zero module messages; limits 10/12/4 match the FXC banner | **MATCH** |
| Final glaw1 | GREEN | `G-LAW 1: GREEN` | **MATCH** |

"No new bindings anywhere in this batch" — confirmed by all four
instruments. The one internal size change is the `agent_tier_gains`
uniform element (32 → 48 B for the two tier columns); it is
sizeof-driven at buffer creation and every bind-group site, so it
auto-resizes with no layout edit.

## The C1a geometry-law deltas

Every pair-test gained a 3D distance GATE (`d2 = dx²+dy²+dz²`) with a
planar RESPONSE (direction via `d_pl`, the xz-only distance; overlap
`(r−d)` keeps the true 3D `d`). Behavior deltas: a pawn atop a MONOLITH
plateau no longer shoves agents at its base; airborne cubes ignore
ground bodies outside their sphere; a cube overhead feels nothing from a
pawn beneath. Ground-level pairs (dy ≈ 0) are unchanged. Applied to both
walker gathers' agent loop **and** sphere loop (handoff line 46) and the
cube-vs-pawn repulsion.

## The λ = 0 bit-neutrality statement

Cube plasticity ships armed but at rest: `plasticity = 0.0` on every
`CUBE_TIER_GAINS` row ⇒ `leak = clamp(0 · dt, 0, 1) = 0` ⇒
`anchor += drift·0` and `drift -= drift·0` are exact no-ops. The leak is
placed AFTER the position compose and is xz-only and anchor-mode-only —
three refinements that make the handoff's stated continuity law ("moves
no pixels") hold, each identically bit-neutral at λ = 0. The elastic
cube is reproduced bitwise; λ > 0 is Jean's tuning dial.

## The flock re-point equal-value proof

`personal_radius` is seeded 30.0 on every tier = the live flock sensing
value `agent_behaviors[FLOCK2D].neighbor_radius = 30.0`. The re-point
`behavior_flock2d: b.neighbor_radius → g.personal_radius` therefore
compares `od2 < 30²` before and after — identical for every agent
regardless of tier (all tiers seed 30.0). Bit-safe by equal value; Jean
tunes per-tier afterward. (ANCHOR-C: the handoff's stated seed 8.0 was
based on a wrong assumption about the sensing value; the stated INTENT —
"behavior-neutral" — required 30.0, which is what shipped.)

## The bubble graduation disclosure

`POINT_BUBBLE_RADIUS` moved from a compile-time WGSL const (no runtime
upload, "must match contracts/point.hpp") to a boot-pinned config field
`config.point_bubble_radius`, filling the checker tail pad so
`GPUDesignConfig` sizeof stays 592 (assert message reworded, not the
number). Its sole reader (the portal's vertical gate) re-points to the
config field; the const becomes the REST-mirror comment. **DISCLOSE:**
the bubble is ONE thing — the portal's vertical gate and the C3b
point-source flee now read the SAME radius, so coupling the bubble's
radius later (the coupling campaign) will breathe portal sensitivity
too. `contracts/point.hpp POINT_BUBBLE_RADIUS` stays the source of truth
(boot-pinned via `set_point_bubble_radius`); rest 20.0 = today.

## The camera-velocity VERIFY outcome — FALLBACK (shipped)

C0 anchor (g) verified: `GPUCameraState` has **no velocity field**
(sizeof 48; pos/azimuth/elevation/distance/pan/aim_point/_pad only). So
in camera-host (free-fly) the point-source flee uses the documented
fallback `v_ap := BUBBLE_PART_SPEED = 4.0` (a constant parting speed),
carrying `TODO(camera-velocity)`. Pawn-host uses the real
`agent_state[possessed_slot]` velocity. **Consequence to note:** the
constant fallback is NOT approach-gated, so a perfectly still free-fly
camera still imparts a parting impulse to bodies inside the bubble —
this diverges from the strict "still world = pre-batch world" clause in
**camera-host only**. It is faithful to the handoff's explicit
constant-fallback instruction and is pre-registered in the DEFERRED
REGISTER below (the camera-velocity upgrade). Pawn-host rest identity is
bitwise-clean.

## The flee-servo sign correction (deviation — adversarially verified)

The handoff's `v_ap = max(0, dot(other.vel, -dir))` (with `dir` =
other→me) measures the **receding** component, so the matador gate
("sprint at a sentinel — it clears you") would never fire. Corrected to
`+dir` (the true approach speed). Worked example: self=(0,0) vel 0,
other=(5,0) vel (−8,0) sprinting in ⇒ `dir=(−1,0)`; `dot(vel,−dir)=−8→0`
(no flee, wrong) vs `dot(vel,+dir)=+8` (flee, right). A **3-lens
adversarial panel** (physics/sign, C4-gate behavior, rest-identity/FXC)
independently re-derived the canonical case and returned **unanimous
CORRECTION_RIGHT, zero defects, rest-identity PRESERVED, C4 gates
SATISFIED, verdict SHIP.** No-double-count verified (the contact-spring
loop keeps the possessed pair; the body-flee excludes it; the pawn flees
only via the point-source term — disjoint sources).

## Deviations

- **D1 — on-master execution** (Jean's direction): no `CONTACT_2`
  side-branch; commits on master with metadata trailers; pushed at C3b.
- **D2 — anchor-c seed 8.0 → 30.0** (equal-value proof above): honored
  the "behavior-neutral" intent over the stated number.
- **D3 — anchor-h target** `CUBE_POPULATIONS` → `CUBE_TIER_GAINS` +
  `FloatingEntityState._pad0` (per the handoff's "or its true name / else
  per the upload shape" hedge).
- **D4 — C1b leak placement + scope**: after the compose, xz-only,
  anchor-mode-only — required to make the handoff's continuity law true
  on this code; all bit-neutral at λ=0.
- **D5 — C2b normalize hardening**: `/max(glen, 0.0001)` fixes the
  handoff's latent flat-ground `normalize(0)` NaN; bit-equivalent where
  glen>0.
- **D6 — C3b flee-sign correction** (`−dir` → `+dir`): the defining
  deviation; adversarially verified SHIP.

## Encoding sweep

world.wgsl / state.hpp / agents.hpp / cube_behaviors.hpp: **no BOM,
LF-only.** The FXC constraints banner (world.wgsl §SEAM fxc-constraints)
is byte-untouched.

## JEAN'S GATE LIST (Windows, one session)

- [ ] **Build + boot.** FXC watch: nothing structural changed — terms in
      existing kernels; the gathers grew by a flee block (still one
      bounded loop each). New: the branchless steering (smoothstep +
      guarded normalize), the flee servo (matador split), the plasticity
      leak, the point-source term.
- [ ] **REST IDENTITY, bitwise:** λ = 0, flee gated on approach,
      steering gated on speed, bubble rest 20 ⇒ a still **pawn-host**
      world is the pre-batch world. (Camera-host free-fly is never
      "still" — the constant parting fallback is the deferred item.)
- [ ] **THE GEOMETRY WALK:** stand atop a MONOLITH plateau — agents at
      the base stay calm; fly a cube overhead of the pawn — it feels
      nothing until spheres truly meet.
- [ ] **THE WHISPER WALK:** walk straight at an antenna — the path bends
      before the clamp ever fires; dead-center approach still stops.
      Watch agents wander AROUND columns.
- [ ] **THE MATADOR:** sprint at a sentinel — it clears you at your
      speed, sideways, then settles back to its gait. Sprint through a
      crowd — a wake, not an explosion. (This gate is exactly what the
      sign correction fixes — if it fails, the sign is the first suspect.)
- [ ] **THE POINT:** free-fly low through a crowd — it parts around the
      camera (the bubble); climb — the parting stops (the vertical gate).
      Portal skim still fires as before (radius unchanged at rest).
- [ ] **PLASTICITY (tuning, not gating):** set one cube tier's λ to 0.5
      locally — shove a cube, watch it stay shifted, the drift spring
      softening the transit; revert or keep per taste.

## DEFERRED REGISTER (named, not lost)

- **camera-velocity upgrade** — the real free-fly camera velocity field
  (GPUCameraState has none today); until then the point-source flee uses
  the constant `BUBBLE_PART_SPEED` fallback in camera-host. **Shipped as
  the fallback this batch.**
- living-plateau steering — needs a GoL gradient channel (the card
  carries no GoL slope; Layer-E-adjacent).
- per-slot floater radii — pending a size field in the fe struct.
- the bubble's coupling wire — the coupling campaign's panel (the bubble
  graduated to config here so the wire has a landing).
- the ribbon floor channel; emissaries (the glider travelers) — their
  own small campaigns.

## Seeds

CONTACT_2 completes the composed-spheres arc: geometry law (3D gates),
steering (the whisper), and the social split (flee is the point's, shove
is the body's). What follows is the coupling campaign on the unified
substrate — the bubble's wire, the band drivers, the presence/paint
cards, and the panel that plays it all.
