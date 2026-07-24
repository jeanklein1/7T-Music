# CONTACT_5 — CAMPAIGN LOG (THE POINT LAW)

Campaign: CONTACT_5 — one response body, one profile table, the point as
emitter. Handoffs `src/docs/HANDOFFS/GPU CAMPAIGN/P 0-3/`. Execution: on
the designated review branch `claude/sync-handoffs-review-5vd7j6` (cut from
`origin/master` be747e8, cleanly fast-forwardable — Jean merges, as with
AUDIT-4), metadata trailers. No new bindings. No new config fields.

## THE ARCHITECTURE (Jean's ruling — verbatim)

> UNIFICATION MEANS ONE BODY, NOT ONE EMITTER. The influence law is
> written ONCE and called by every site. What differs per family is not
> the law but the PROFILE: which term is active, how much of the response
> this body takes, and what the displacement leaves behind.
>
> THE POINT IS THE EMITTER — not the pawn. The pawn is one HOST of the
> point; the camera is another. Every point-side term reads point_pos()
> (host-routed) so the system is host-agnostic by construction. PRESENCE
> FOLLOWS THE POINT; EMANATION STAYS THE BODY'S.
>
> THE TWO RESPONSE SHAPES: APPROACH (the dodge) — force ∝ the other's
> closing speed (v_ap); a reaction to MOTION; velocity-floor, NOT
> dt-scaled (K1). PRESENCE (the shove) — force ∝ overlap (r−d); a
> reaction to OCCUPANCY; impulse, dt-scaled (K1).
>
> THE AUTHORITY TABLE: agents — approach, authority below the point, no
> persistence (FROZEN this batch). cubes — presence, authority far below,
> FULL persistence. spheres — presence, authority ABOVE the point: the
> POINT yields (same law, inequality reversed).

## LAND ORDER: [P0] → [P1] → [P2] → [P3]

[P1] behavior-preserving (collapse only); [P2] the deliberate change.
Separate = the bisection line: if feel breaks at P2, P1 is known-good.

---

## P0 — INDEX + PREFLIGHT

### Base

`be747e8` (origin/master — the merged AUDIT-4 report + the P handoffs).
Baseline glaw1: **G-LAW 1: GREEN**.

### a. The agent gathers (verbatim, world.wgsl @ be747e8)

**`update_player_agent` gather** (7242-7309) — agent loop (contact spring +
body flee), NO floater loop (the sphere loop was deleted at CONTACT_4 S2c):
```
for (var k = 0u; k < 32u; k++) {
    if (k == slot) { continue; }
    let other = agent_state[k];
    if (other.is_active == 0u) { continue; }
    let og = agent_tier_gains[min(other.tier_idx, 3u)];
    var m_other = og.contact_mass;
    if (k == config.possessed_slot) { m_other *= PAWN_CONTACT_MASS_MULT; }
    let dx = agent.pos_x - other.pos_x; ... dz ...
    let d2 = dx*dx + dy*dy + dz*dz;
    let r  = g_self.contact_radius + og.contact_radius;
    if (d2 < r * r && d2 > 0.0001) {
        let d = sqrt(d2);
        let d_pl = sqrt(max(dx*dx + dz*dz, 0.0001));
        let push = min((r - d) * CONTACT_SPRING * signal.dt, CONTACT_IMPULSE_CAP)
                 * (m_other / (m_self + m_other));
        agent.vel_x += (dx / d_pl) * push;  agent.vel_z += (dz / d_pl) * push;
    }
    // flee servo:
    let pr = (g_self.personal_radius + og.personal_radius) * FLEE_SHELL_FRAC;
    if (d2 < pr * pr && d2 > 0.0001 && k != config.possessed_slot) {
        let fdpl = sqrt(max(dx*dx + dz*dz, 0.0001));
        let dir = vec2(dx, dz) / fdpl;
        let v_ap = max(0.0, dot(vec2(other.vel_x, other.vel_z), dir));
        let deficit = v_ap * NONPLAYER_FLEE_GAIN - dot(vec2(agent.vel_x, agent.vel_z), dir);
        if (v_ap > 0.001 && deficit > 0.0) {
            let tang = vec2(-dir.y, dir.x) * sign(other.vel_x*dir.y - other.vel_z*dir.x + 0.000001);
            let esc = normalize(dir + tang * 0.6);
            agent.vel_x += esc.x * deficit;  agent.vel_z += esc.y * deficit;
        }
    }
}
```
m_self = `g_self.contact_mass * PAWN_CONTACT_MASS_MULT`. Player integrates
inline (K1b): `agent.pos_x += (agent.vel_x - imp_v0.x)*signal.dt` (7316).

**`update_other_agents` gather** (7357-7486) — agent loop (identical spring +
flee to above but m_self = `g_self.contact_mass`), THEN the sphere loop, THEN
the point-source block:
```
// sphere loop (7415-7436):
for (var sph = 0u; sph < SPHERE_SLOT_COUNT; sph++) {
    let fe = floating_entities.entities[sph];
    if (fe.is_active == 0u) { continue; }
    let dx = agent.pos_x - fe.pos.x; ... dz ...
    let d2 = dx*dx + dy*dy + dz*dz;
    let r  = g_self.contact_radius + fe.body_radius;
    if (d2 < r * r && d2 > 0.0001) {
        let d = sqrt(d2);  let d_pl = sqrt(max(dx*dx + dz*dz, 0.0001));
        let push = min((r - d) * CONTACT_SPRING * signal.dt, CONTACT_IMPULSE_CAP);
        agent.vel_x += (dx / d_pl) * push;  agent.vel_z += (dz / d_pl) * push;
    }
}
// point-source block (7447-7485):
let pt = point_pos();
let pdx = agent.pos_x - pt.x; ... pdz ...  let pd2 = pdx*pdx + pdy*pdy + pdz*pdz;
let ppr = config.point_bubble_radius;
if (pd2 < ppr * ppr && pd2 > 0.0001) {
    let pdpl = sqrt(max(pdx*pdx + pdz*pdz, 0.0001));
    let pdir = vec2(pdx, pdz) / pdpl;
    var pvel = vec2(0.0);  var v_ap = BUBBLE_PART_SPEED;
    if (!point_camera_hosted()) {
        let pawn = agent_state[config.possessed_slot];
        pvel = vec2(pawn.vel_x, pawn.vel_z);  v_ap = max(0.0, dot(pvel, pdir));
    }
    let pd = sqrt(pd2);
    let prox = clamp(1.0 - pd / ppr, 0.0, 1.0);
    let deficit = v_ap * g_self.flee_gain_player * prox - dot(vec2(agent.vel_x, agent.vel_z), pdir);
    if (v_ap > 0.001 && deficit > 0.0) {
        let tang = vec2(-pdir.y, pdir.x) * sign(pvel.x*pdir.y - pvel.y*pdir.x + 0.000001);
        let esc = normalize(pdir + tang * 0.6);
        agent.vel_x += esc.x * deficit;  agent.vel_z += esc.y * deficit;
    }
}
```

### b. `update_cube` blocks (verbatim, world.wgsl 7908-8012)

- **cube↔pawn contact** (7908-7922): `cr = CONTACT_CUBE_RADIUS + pg.contact_radius`;
  3D gate `cd2 < cr*cr`; `mag = min((cr-cd)*CONTACT_SPRING, CUBE_PART_CAP)`
  **(no dt inside)**; `contact_force = vec3((cdx/cd_pl)*mag, 0, (cdz/cd_pl)*mag)`.
- **parting** (7929-7950): `qpr = CUBE_PART_RADIUS`; 3D gate `qd2 < qpr*qpr`;
  v_ap = pawn approach or BUBBLE_PART_SPEED; `falloff = clamp(1-sqrt(qd2)/CUBE_PART_RADIUS,0,1)`;
  `f = min(v_ap*CUBE_PART_GAIN*falloff, CUBE_PART_CAP)` **(no self_vel term, no
  tangential split)**; `parting_force = vec3(qdx/qdpl,0,qdz/qdpl)*f`.
- **drift integration** (7952-7955): `spring_a = -fe.drift*fe.spring_stiffness`;
  `drift_vel += (spring_a + behavior_force + contact_force + parting_force)*dt`;
  `drift_vel *= exp(-fe.drag*dt)`; `drift += drift_vel*dt`. **Both contact_force
  and parting_force are FORCES (accelerations) — dt applied HERE, at integration.**
- **plasticity leak** (8005-8012, anchor-mode only): `lam = fe.plasticity *
  config.cube_plasticity`; `leak = clamp(lam*dt,0,1)`; anchor.xz += drift.xz*leak;
  drift.xz -= drift.xz*leak.

### c. `update_camera` camera-host branch (verbatim, 7527-7550)

Self-contained, EARLY RETURN — the terrain/indoor clamps below (7587-7623) are
in the pawn-host path and are SKIPPED here (TERRAIN RULE = NONE):
```
if (point_camera_hosted()) {
    camera.azimuth += signal.look_az_delta;
    camera.elevation = clamp(camera.elevation + signal.look_el_delta, FPV_MIN_ELEVATION, FPV_MAX_ELEVATION);
    let cos_el = cos(camera.elevation); ... fly_forward/fly_right/fly_up ...
    let fly_speed = select(PAWN_SPEED, config.point_fly_speed, config.point_fly_speed > 0.0);
    camera.pos += (fly_forward*(-signal.move_z) + fly_right*signal.move_x)*fly_speed*signal.dt;   // input → position
    camera.pos += (fly_right*signal.pan_x_delta + fly_up*signal.pan_y_delta)*camera.distance*0.5; // pan
    camera_state = camera;
    return;   // ← P2a sphere push inserts BEFORE this; no later writer overrides camera.pos in this branch
}
```
`point_camera_hosted()` (2321), `point_pos()` (5672: camera_state.pos if
camera-hosted else compute_pawn_pos()) — spellings confirmed.

### d. FloatingEntityState fields + sphere authored ranges

WGSL `FloatingEntityState` (984-1027) exposes BOTH `body_radius` (off 12) and
`influence_radius` (off 32) as baked per-instance GPU fields (C++ twin
`GPUFloatingEntityState` state.hpp:763; sphere writes spheres.hpp:194/198).
- **sphere `influence_radius`** μ band **6.0–8.0** (Sentinel 8.0 / Anomaly 6.0),
  σ 1.5–2.0, floor 3.0. (Below the handoff's assumed 8–15; NOT wildly out.
  RULING: ship `fe.influence_radius` per the handoff's primary instruction —
  authored, per-instance, "a celestial object's influence IS its reach." The
  handoff worked example's "12" is illustrative; the shipped band is 6–8.)
- **sphere `body_radius`** μ band **1.2–1.5**, σ 0.2–0.3, floor 0.5.

### e. Cube orbit_height authored range (P2b vwindow derivation)

`CUBE_TIERS` (cube_behaviors.hpp:497): orbit_height μ = 25 / 45 / **75** / 12
(SmallCube/MedCube/LargeCube/Monolith), floor 3.0, no ceiling; bob μ = 1.0 /
1.5 / **2.0** / 1.2. **Max authored orbit_height μ = 75 (LargeCube), bob 2.0.**
⇒ **CUBE_PUSH_VWINDOW = max_orbit_height(75) + bob(2) + 8 = 85.0 wu** (the
handoff formula). Tail note: σ is large (up to 45); cubes sampled above ~85
(the high tail) sit above the cylinder and stay unreachable — disclosed (a
softer echo of the CONTACT_4 outdoor caveat; 85 now covers every authored mean).

### f. The const cluster + AGENT_TIER_GAINS (world.wgsl / agents.hpp)

`CONTACT_SPRING 40` (2210), `CONTACT_IMPULSE_CAP 6` (2211), `CONTACT_CUBE_RADIUS 3`
(2223), `PAWN_CONTACT_MASS_MULT 4` (2226), `NONPLAYER_FLEE_GAIN 0.8` (2241),
`BUBBLE_PART_SPEED 4` (2243), `FLEE_SHELL_FRAC 0.25` (2251), `CUBE_PART_RADIUS 30`
(2265), `CUBE_PART_CAP 12` (2266), `CUBE_PART_GAIN 1` (2267). `config.point_bubble_radius`
20 (state.hpp:556). AGENT_TIER_GAINS (agents.hpp:174): contact_radius
1.6/1.4/2.0/1.8, contact_mass 1.0/0.8/1.5/1.2, personal_radius 30 (all),
flee_gain_player 0.70/0.85/0.50/0.60. `Idle::CUBE_PLASTICITY_DEFAULT 0.6` (state.hpp).

---

## THE BIT-PRESERVATION DESIGN (P1a VERIFY-AND-LOG — the proof)

The handoff's draft `influence_response` body has **two defects** that would
BREAK the P1 agent gate, plus a **dt-semantics reconciliation** for cubes. All
three are exactly the "order the profile's terms to match / this is the
bit-preservation proof" work the handoff delegates. Corrections:

**DEFECT 1 — the double falloff (breaks the agent point-flee, a HARD-gate site).**
The draft returns `esc * min(mag * fall, cap) * yield` while ALSO folding `fall`
into the approach deficit (`v_ap * approach_gain * fall`). For the point-source
row (falloff_mix = 1) that applies `prox` TWICE: the draft yields
`esc·(v_ap·gain·prox − dot(self,dir))·prox`, but the current site yields
`esc·(v_ap·gain·prox − dot(self,dir))` (prox once, on the v_ap term only).
The draft would weaken the agent dodge by a factor of `prox` — an agent-visible
change, forbidden by the P1 gate. **FIX:** apply `fall` to the presence term
(`(r−d)·gain·dt·fall`) and to the v_ap sub-term of the deficit, and NOT again
at the return. (Presence rows are `fall = 1`, so contact is untouched; the P2b
cube-push presence with falloff_mix = 1 gets its rim-soft `fall` here, matching
the handoff's own worked example `4·25·(1/60)·0.57`.)

**DEFECT 2 — the missing tangential column (would give cube-parting a matador
split it lacks).** The draft sets `esc = normalize(dir + tang·0.6)` whenever the
approach term fires. The agent flees DO split (0.6); the **cube parting is a
straight radial push (no tangential)**. Unifying with a hardcoded 0.6 changes
cube behavior. **FIX:** add a `tangential` profile column (0.6 for the agent
flees, 0 for cube parting → `esc = dir`). This makes the agent-vs-cube approach
shape a VISIBLE COLUMN — the handoff's own "drift preserved as a column" law.

**RECONCILIATION 3 — dt placement (impulse vs force).** Agent contact is a
dt-scaled impulse added straight to velocity; the cube forces are ACCELERATIONS
(`drift_vel += force·dt` at integration). One body serves both by making `dt` a
PARAMETER: `influence_response` dt-scales the PRESENCE term by its `dt` arg and
leaves APPROACH as a velocity floor (K1). **Agent callers pass the real dt and
add the result directly to velocity. The cube caller passes dt = 1.0 (so the
presence term returns a raw FORCE) and applies the real ×dt at its existing
drift-integration line.** This bit-preserves BOTH cube sites exactly (the cap
still binds on the pre-dt force, as today).

**RECONCILIATION 4 — the camera-host approach fallback (a hard-gate site).**
Both the point-flee (agents, site 4) and the cube parting (site 6) fall back to
a constant `v_ap = BUBBLE_PART_SPEED` when the point is CAMERA-hosted (no camera
velocity field). That isotropic constant cannot be produced by
`max(0, dot(other_vel, dir))` from any single `other_vel` (it is
direction-agnostic). Dropping it would stop agents dodging the free-fly camera —
an agent-visible change (Jean uses free-fly; the P1 gate forbids it). **FIX:**
add an `approach_floor` PARAMETER: `v_ap = max(approach_floor, dot(other_vel,
dir))`. Pawn-host and body-to-body pass `0` (real closing speed via `other_vel`);
camera-host passes `BUBBLE_PART_SPEED` with `other_vel = 0`. This is the exact
current fallback; the deferred `config.point_vel_x/z` would retire it (floor→0).

**THE CORRECTED BODY (shipped in P1a):**
```
fall = mix(1, clamp(1 - d_gate/radius, 0, 1), clamp(falloff_mix, 0, 1))
var mag = (radius - d_gate) * presence_gain * dt * fall;   // PRESENCE (dt-scaled, fall-weighted)
var esc = dir;
if (approach_gain > 0) {
    v_ap = max(approach_floor, dot(other_vel, dir));         // isotropic camera-host fallback
    if (v_ap > 0.001) {
        deficit = v_ap * approach_gain * fall - dot(self_vel, dir);   // APPROACH (fall on v_ap term)
        if (deficit > 0) { mag += deficit; esc = normalize(dir + tang * tangential); }
    }
}
return esc * min(mag, cap) * yield_share;
```
plus a degenerate guard reproducing every site's `d2 > 0.0001` skip. Signature:
`influence_response(self_pos, self_vel, other_pos, other_vel, p, dt, approach_floor)`.

**THE PER-ROW PROOF (each profile reproduces TODAY exactly):**

| Row | dt arg | profile (r · vwin · pres · appr · fmix · cap · yield · tang) | reproduces |
|---|---|---|---|
| agent↔agent contact ×2 | real | cr_sum · 0 · SPRING · 0 · 0 · IMPULSE_CAP · m_o/(m_s+m_o) · 0 | `dir·min((r−d)·SPRING·dt, IMP_CAP)·mass` ✓ (cap before mass) |
| agent↔agent flee ×2 | real | pers_sum·FRAC · 0 · 0 · NONPLAYER_FLEE_GAIN · 0 · NO_CAP · 1 · 0.6 | `esc·(v_ap·GAIN − dot(self,dir))` ✓ (fall=1) |
| agent↔sphere contact | real | contact_r+fe.body_radius · 0 · SPRING · 0 · 0 · IMPULSE_CAP · 1 · 0 | `dir·min((r−d)·SPRING·dt, IMP_CAP)` ✓ |
| point-source flee | real | point_bubble_radius · 0 · 0 · flee_gain_player · 1 · NO_CAP · 1 · 0.6 | `esc·(v_ap·gain·prox − dot(self,dir))` ✓ (prox ONCE — defect 1 fixed) |
| cube↔pawn contact | **1.0** | CONTACT_CUBE_RADIUS+contact_r · 0 · SPRING · 0 · 0 · CUBE_PART_CAP · 1 · 0 | caller ×dt ⇒ `dir·min((cr−cd)·SPRING, CAP)·dt` ✓ |
| cube parting | **1.0** | CUBE_PART_RADIUS · 0 · 0 · CUBE_PART_GAIN · 1 · CUBE_PART_CAP · 1 · **0** | caller ×dt, self_vel=0 ⇒ `dir·min(v_ap·gain·falloff, CAP)·dt` ✓ (radial — defect 2 fixed) |

`INFLUENCE_NO_CAP` = a named large const (the uncapped flee rows have no empty
cell). Agent rows (1,2,3,4) — the HARD gate. Cube rows (5,6) match via the dt=1
+ caller-×dt convention.

## P1 VERIFY — the adversarial panel + the 4 fixes it forced

A 6-skeptic adversarial panel (one per row, each told to REFUTE bit-identity
with a concrete f32 counterexample) ran against the FIRST P1b lift. It did its
job — it found **four** real divergence sources the symbolic proof had glossed,
all in the shared body, all now fixed and re-verified:

1. **Square-vs-root gate (all rows).** The inline sites gate `d2 < r*r`
   (squared); the first body gated `sqrt(d2) < radius` (root). These disagree in
   a thin f32 band at the shell edge. Harmless for the contact rows (there
   `(r−d)≈0`, so the magnitude is ≈0 anyway — the cube-contact row was already
   bit-identical) and for the point flee (falloff_mix=1 tapers the edge to ≈0),
   but on the **flat** body-to-body flee shell (falloff_mix=0) a gate flip toggles
   a FULL ~1.6 wu/s impulse on/off. **FIX:** gate in squared space
   `d2_gate >= radius*radius`.
2. **`dot(d3,d3)` vs the explicit sum.** `dot()` may lower to a different fma
   contraction than the inline `dx*dx + dy*dy + dz*dz`. **FIX:** explicit sum.
3. **Product reassociation.** The inline sites fold the scalar first
   (`dir*(min*yield)`); the first body wrote `esc*min(...)*yield` →
   `(dir*min)*yield`. f32 `*` is non-associative ⇒ 1 ULP on ~15% of contact
   pairs (mass-weighted). **FIX:** `let s = min(mag,cap)*yield; return esc*s;`.
4. **Cube-parting re-normalization (macroscopic).** The inline cube parting uses
   the RAW `dir = (qdx/qdpl, qdz/qdpl)` — which is NOT unit when the point sits
   nearly under the cube (planar dist < 0.01 ⇒ `qdpl` floors to 0.01 ⇒ `|dir|<1`).
   The first body ran every approach through `normalize(dir+tang*tangential)`,
   so a near-overhead cube got a full unit push instead of the tiny raw one — a
   MACROSCOPIC divergence. **FIX:** normalize ONLY when `tangential > 0` (the
   flees); radial parting (tangential 0) keeps the raw `dir`.

The point-flee "double falloff" worry (the P1a Defect 1 I had pre-empted) was
confirmed CLEAN by the panel — `fall` multiplies the v_ap term once, never again
at the return.

### FINAL PANEL VERDICT (after the 4 fixes)

A fresh 6-skeptic panel re-tested every prior counterexample against the
corrected body:

| Row | bit_identical | prior counterexample now matches |
|---|---|---|
| agent-agent contact | **TRUE** | yes (the 1-ULP reassociation is gone) |
| agent-agent flee | **TRUE** | yes (the flat-shell gate flip is gone) |
| agent-sphere contact | **TRUE** | yes (explicit sum + squared gate) |
| point-source flee | **TRUE** | yes (both hosts; fall applied once) |
| cube-pawn contact | **TRUE** | yes |
| cube parting | **1-ULP dev.** | prior FIXED; a *new*, deeper residual found |

**THE FOUR AGENT ROWS ARE BIT-IDENTICAL — the P1 hard gate (agents frozen) is
fully met.** The lone residual is on the CUBE parting, non-agent:

**ACCEPTED DEVIATION — cube-parting pawn-host v_ap (1 ULP).** OLD computed
`v_ap = (vx*qdx + vz*qdz) / qdpl` — the raw-coordinate dot divided ONCE. The
unified body computes `dot(other_vel, dir)` where `dir = (qdx/qdpl, qdz/qdpl)` —
the division baked per-component into `dir` (divide-then-sum). Algebraically
equal, ~1 ULP apart in f32 (~24% of pawn-host inputs, last-bit). This is an
UNAVOIDABLE trade: the point-source flee — an AGENT hard-gate site — used OLD
`dot(pvel, pdir)` (the divide-then-sum form), so the one shared v_ap MUST take
that form to keep agents bit-exact. That leaves the cube parting's idiosyncratic
single-division form 1 ULP off. The right call is agents-exact; the cube pays 1
ULP. It is non-agent, imperceptible on a drift force, and **P2b replaces the cube
parting with a PRESENCE profile (no v_ap at all)** — so the deviation exists for
exactly the P1b→P2b span and then dissolves. Camera-host cube parting is fully
bit-identical (other_vel = 0 ⇒ dot = 0 exactly). Recorded, not fixed.

The gate is the campaign's bisection line and it holds: agents are byte-for-byte
CONTACT_4; only a soon-retired cube path moves, by 1 ULP.
