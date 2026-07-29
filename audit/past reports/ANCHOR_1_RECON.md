# ANCHOR_1 — PART A RECON

Base: `9c46ff63707052b46c8af081f72c6f544a7ce823` (master @ HEAD, clean tree)
`git rev-parse --is-shallow-repository` → `false`
Encoding: LF, no BOM.

Gates: `[G:grep]` CC — Part A. `[G:glaw1]` CC — Part B. `[G:visual]` Jean — UNCLAIMED.

---

## CONTRADICTIONS FIRST

### C1 — `update_cube` ALREADY issues a live-xz ground query. Part B is the *third* one, not the second.

The handoff's WHY states: *"A cube displaced by CurlField, PhaseWave or the
point-source shove hovers at orbit_height above terrain it is no longer over."*

Half true. Downstream of the FIND block, `world.wgsl:7556-7584` already does
exactly what ruling 1 asks — at the live xz, through POLICY_FLYER:

```wgsl
            // ── Terrain-clearance clamp on drift.y ────────────────
            // Query ground at the cube's *actual* xz, not at the home
            // anchor. This matters whenever a behavior moves the cube
            // in xz: CurlField can drift the cube onto terrain that's
            // higher than the home (a pyramid, a hill), and the clamp
            // needs to know about *that* ground, not the flat ground
            // at the anchor. PhaseWave doesn't move xz, so for it the
            // two queries return the same value.
            ...
            let pos_xz = vec2(home.x + fe.drift.x, home.z + fe.drift.z);
            let pos_qi = QueryInputs(vec3(pos_xz.x, 0.0, pos_xz.y), signal.t_seconds);
            let ground = manifold_position(vec3(pos_xz.x, 0.0, pos_xz.y), POLICY_FLYER, pos_qi).y;
            let cube_floor_y = ground + CUBE_TERRAIN_CLEARANCE + fe.body_radius * fe.aspect_y;
            let min_drift_y = cube_floor_y - home.y;
            if (fe.drift.y < min_drift_y) {
                fe.drift.y = min_drift_y;
                if (fe.drift_vel.y < 0.0) { fe.drift_vel.y = 0.0; }
            }
```

So a drifted cube does **not** sink into a rise today. What it fails to do is
*rise to full clearance* over one: `home.y` stays pinned to the birthplace's
ground, and the clamp then floors the cube at
`CUBE_TERRAIN_CLEARANCE + body_radius*aspect_y` above the rise — a scrape, not
a hover.

**The defect is real but its name is different: constant clearance is not
maintained; the ground is not ignored.** Part B remains correct and worth
landing. Two consequences:

- The "NO NEW FXC SURFACE" claim is *stronger* than the handoff states —
  `manifold_position(..., POLICY_FLYER, ...)` is called three times in this
  kernel already (kite arm, anchor arm, clamp), and one of those three is
  already a live-xz call. Part B changes an argument to match a call that is
  already there.
- After Part B the clamp goes slack in the ordinary case (`home.y` already
  tracks the local ground, so `min_drift_y ≈ CLEARANCE + half_extent −
  orbit_height − bob_y`, deeply negative). It is not dead: it still covers the
  one-frame lag (C2/A5) and the kite arm, and it is still the hard floor. Its
  comment is amended in the Part B commit to say so.

### C2 — Part B introduces an F7 toggle-ON discontinuity in `home.y`.

The comment the handoff asked me to quote is the one Part B falsifies. Today,
`world.wgsl:7474-7482`:

```wgsl
            // Y is *always* terrain-relative in both modes. This makes
            // F7 toggle preserve world position cleanly: at the moment
            // of toggle, the home.xz interpretation switches but the
            // ground query underneath gives the same answer (anchor.xz
            // == point.xz + offset.xz at the toggle moment by
            // construction, since offset is captured as cube.cx -
            // point.xz). Cubes feel like balloons leashed to the point
            // — float at orbit_height above whatever terrain they're
            // over, regardless of the point's current altitude.
```

The "balloons" sentence becomes true in both modes, as the handoff predicted.
The **sentence before it becomes false.** Its whole argument is that both arms
query the same xz at the toggle moment. After Part B they do not:

| | anchor arm queries | kite arm queries |
|---|---|---|
| today | `anchor.xz` | `kite_xz == anchor.xz` at toggle |
| after B | `pos.xz == anchor.xz + drift.xz` | `kite_xz == anchor.xz` at toggle |

A cube toggled into kite mode **while drifting, over a slope** steps vertically
by `ground(anchor.xz + drift.xz) − ground(anchor.xz)`. CurlField drifts ~3 wu;
on flat ground this is zero, on a pyramid face it is visible. A cube at rest
has `drift == 0` and is unaffected — the handoff's stationary-cube visual check
still holds exactly.

**Not fixed here.** The fix is ruling 1 applied to the kite arm as well
(`kite_xz → pos.xz`), which is a second edit outside Part B's FIND block.
Proposed as ANCHOR_2. The false comment **is** repaired in the Part B commit —
leaving a comment that argues for a property the same commit removes is the
failure mode this campaign's charter line names.

### C3 — A4's cross-check: the prior "update_sphere ignores GoL height" finding is **STALE**.

See A4. The sphere goes through POLICY_FLYER at its live orbital xz, and
POLICY_FLYER's mask contains `CONTRIB_GOL_ZONES`. The finding was true of some
earlier revision; it is not true of this tree.

### C4 — `ribbon_head_move` does not exist.

A2 names three functions; only two exist. See A2.

### C5 — The CPU mirror of the cube anchor is write-only and goes stale. (pre-existing)

Found while building A6, independent of Part B. Nothing on the CPU ever reads
`GPUFloatingEntityState::anchor` back. The CPU's stand-in is
`activeCubes_[i].cx/cz`, written at spawn (`cube_behaviors.hpp:555`), by corral
(`:345-346`) and by F7 toggle-OFF (`:416-417`) — and **never** refreshed from
either GPU-side anchor write: the plasticity leak (`world.wgsl:7607-7608`) and
the kite release (`:7457`). With `config.cube_plasticity = 1.0` and a non-zero
per-tier `fe.plasticity`, the leak fires every frame, so the mirror diverges
continuously.

The one consumer that matters is the F7 toggle-ON offset capture
(`cube_behaviors.hpp:395-396`), which reads the stale mirror. This compounds
C2: after a period of plasticity leak the captured offset is already wrong by
the leaked amount, and Part B adds the drift term on top. Reported, not fixed.

---

## A1 — WHERE IS THE RIBBON'S SPINE EVALUATED?

**Hybrid, and the CPU half is the head.** The spine is *not* a pure GPU
evaluation from `RibbonState`, and *not* a CPU polyline upload either. The CPU
authors a short head-pose history and uploads it
(`upload_ribbon_head_poses`, `ribbon.hpp:588`); the GPU echoes that history
down the body.

**`RibbonState` field list** (`state.hpp:820-843`, mirrors `world.wgsl:997`):

```cpp
    float anchor[3];            // 0
    float time;                 // 12
    uint32_t cube_count;        // 16
    float cube_size;            // 20
    float height;               // 24
    float checker_scatter;      // 28 — per-cell color jitter amplitude (CONTRAST skin)
    float color[3];             // 32
    float lateral_amp;          // 44
    float lateral_freq;         // 48 (rad/s, head oscillation rate)
    float vertical_amp;         // 52
    float vertical_freq;        // 56
    uint32_t seed;              // 60 — spawn seed (GPU-side per-ribbon hash key)
    float propagation_speed;    // 64 (world units/s; head→tail trail rate)
    uint32_t is_visible;        // 68
    float orientation;          // 72 (heading radians)
    uint32_t color_mode;        // 76
    uint32_t is_roaming;        // 80 (0 = stationary spine = today; 1 = head roams, wired stage 1b)
    float _pad1;                // 84
    float _pad2;                // 88
    float _pad3;                // 92
    float color_b[3];           // 96 — second checker median (CONTRAST)
    float hue_spread;           // 108 — radians; per-cell hue rotation amplitude (CONTRAST skin; 0 = CB-1 look)
                                // 112 total
```

**Entry point** — `world.wgsl:4981-5001`, a compute kernel, one invocation per
ring:

```wgsl
// --- Compute ribbon ring transforms (flying ribbons; no terrain follow)
@compute @workgroup_size(64)
fn compute_ribbon_rings(@builtin(global_invocation_id) gid: vec3<u32>) {
    let ring_idx = gid.x;
    let ribbon = ribbon_state;

    // Early-out for unused rings
    if (ring_idx >= ribbon.cube_count || ribbon.is_visible == 0u || ribbon.cube_count < 2u) {
```

Note the kernel's own header comment: **"no terrain follow."**

Ring transforms land in `ring_xforms` (`GPURibbonRingTransform`, 48 B stride,
bindings 121 / 361) and `ribbon_vs` (`world.wgsl:5030`) indexes them. The spine
position itself is `ribbon_spine_at` (`:4856`) → `ribbon_centerline_at` (`:4813`)
+ `ribbon_displacement_at`.

**Sample-point count** — `state.hpp:65-68`:

```cpp
    constexpr uint32_t RIBBON_MAX_RINGS = 400;
    constexpr uint32_t RIBBON_VERTEX_COUNT = (RIBBON_MAX_RINGS - 1) * RIBBON_TUBE_VERTS_PER_SEG + RIBBON_CAP_VERTS;
```

400 rings max; live count is `ribbon.cube_count`. Dispatch is
`ribbon_ring_workgroups() = (400 + 63) / 64` = 7 workgroups (`state.hpp:2876`).

**Ground query anywhere in the GPU ribbon path: NONE.** `compute_ribbon_rings`,
`ribbon_spine_at`, `ribbon_centerline_at`, `ribbon_displacement_at`,
`ribbon_ring_motor` and `ribbon_vs` contain no `manifold_*`, no
`query_ground_*`. The `GPURibbonRingTransform` slot that once held it is now
explicitly dead (`state.hpp`):

```cpp
            float _pad0;               // ( 4) = 48 — explicit padding. Was
                                       // `terrain_y`, always 0.0 since ribbons
                                       // stopped following terrain: two writers
                                       // in world.wgsl, zero readers anywhere.
```

> **Verdict:** the spine is a GPU compute kernel over 400 rings that reads a
> CPU-authored head-pose history, and it contains no ground query at all — the
> field that used to carry one is now named `_pad0` and documented as dead.

## A2 — WHERE DOES THE RIBBON HEAD MOVE, CPU OR GPU?

**`ribbon_head_move` does not exist** (C4). The head moves **CPU-side**, in
`ribbon_advance_head`. Signatures, all in `bodies/ribbon.hpp` (declarations at
:453-491, definition at :592):

```cpp
void ribbon_advance_head(RibbonState& rs, GPUState& gpuState,          // :453
                         ...);
void ribbon_head_pen(const RibbonState& rs, float& x, float& z, float& heading);   // :460
inline void ribbon_wander_inputs(ActiveRibbon& ar,                     // :491
```

`ribbon_head_pen` (`:782`) is a pure accessor on the CPU mirror:

```cpp
inline void ribbon_head_pen(const RibbonState& rs, float& x, float& z, float& heading) {
    x = rs.head.pos[0];
    z = rs.head.pos[2];
    heading = rs.head.heading;
}
```

All three are CPU. The GPU never moves the head; it receives
`head_poses` through `upload_ribbon_head_poses` (`ribbon.hpp:588`) — the file
header calls that path "a dumb wire" (`:26`).

**Does that site have a CPU ground query? YES — and it is already being
called.** `ribbon.hpp:900-915`:

```cpp
        float rib_gnd;
        bool  rib_gnd_valid;
        {
            const auto& rb = rs.gpu[rs.rendered_slot];
            float gx = rb.anchor[0], gz = rb.anchor[2];
            if (ribbon_head_is(rs, rs.rendered_slot)) {
                float hy, hh; ribbon_head_pose(rs, gx, hy, gz, hh);
            }
            rib_gnd = estimate_terrain_height(c->tile_world_state_, gx, gz);
            rib_gnd_valid = terrain_tile_warm(c->tile_world_state_, gx, gz);
        }
        ribbon_advance_head(rs, c->gpuState_, queue, ...
            rib_gnd, rib_gnd_valid);
```

(the same block again at `:935-942` for the `nearest` slot). Note it queries at
the *live head* xz when `ribbon_head_is` holds — `ribbon_head_pose` overwrites
`gx/gz` — and falls back to `rb.anchor.xz` otherwise. So the query point is
already ruling-1 shaped.

**But the answer is consumed ONCE.** `ribbon.hpp:620-627`:

```cpp
    if (!hd.alt_baked) {
        hd.origin[1]  = ground_y + ribbon.height;   // first truth, once
        hd.pos[1]     = hd.origin[1];
        hd.alt_target = hd.origin[1];
        hd.y_vel      = 0.0f;
        hd.hist_y.fill(hd.pos[1]);   // the body's constant past re-bases with the bake
        hd.alt_baked  = ground_valid;
    }
```

> **Verdict:** the head moves on the CPU, a CPU ground query already exists at
> the live head position (`estimate_terrain_height` + `terrain_tile_warm`), and
> ruling 2's per-frame clamp needs no new query site — only the removal of the
> `alt_baked` latch that currently makes the ground a birthright rather than a
> reading. The caveat: `estimate_terrain_height` samples **terrain only**, so a
> CPU-side ruling 2 gets terrain avoidance but not the piers/pyramids/GoL that
> POLICY_FLYER bundles for free. That is the trade A3 prices.

## A3 — THE PIPELINE LAYOUT QUESTION

**The ribbon does NOT sit on the ground-bearing layout.** `renderer.hpp:1282-1291`:

`liveContribComputeLayout` = `{ computeEntityLayout_, computeTextureLayout_ }` —
a two-group layout. Its users are `update_player_agent`, `update_other_agents`,
`update_camera`, `update_sphere`, `update_cube`. The Group 1
(`computeTextureLayout`) bindings are what let a kernel reach
`query_ground_flyer` / `manifold_position`.

`compute_ribbon_rings` is built at `renderer.hpp:1370-1373` on
`computeLayoutFor(ribbonComputeLayout_)` — a **different, single-group** layout
with no texture group.

**What would have to be added:** `computeTextureLayout_` as Group 1 of the
ribbon compute pipeline layout, plus the matching bind-group creation and a
`SetBindGroup(1, …)` at the ribbon dispatch site — i.e. a new
`ribbonLiveContribComputeLayout`, or a promotion of `ribbonComputeLayout_` to
two groups.

> **Verdict:** GPU-side ruling 2 is the **new-pipeline-layout class**, the
> FXC-sensitive one — not a one-line clamp. CPU-side ruling 2 is free but
> terrain-only (A2). This is the decision the handoff wrote A3 to force, and
> the answer is the expensive one.

## A4 — THE SPHERE'S GROUND READ

`coupling_terrain_to_sphere_orbit_height`, in full (`world.wgsl:3176-3193`):

```wgsl
fn coupling_terrain_to_sphere_orbit_height(sphere_xz: vec2<f32>, base_height: f32) -> f32 {
    if (!coupling_active(COUPLING_TERRAIN_TO_SPHERE_HEIGHT)) {
        return base_height;
    }

    // POLICY_FLYER — sphere rides static base + pyramids + gol zones +
    // terrain waves + radial pulses + pawn aura. No gol_suppression
    // (flyers don't flatten GoL at their own position).
    // consumer_pos is unused by flyer (no consumer-local fields); pass
    // a placeholder Y — only xz matters.
    let qi = QueryInputs(vec3(sphere_xz.x, 0.0, sphere_xz.y), signal.t_seconds);
    let ground = manifold_position(vec3(sphere_xz.x, 0.0, sphere_xz.y), POLICY_FLYER, qi).y;

    // Ensure minimum clearance above ground
    let min_height = ground + SPHERE_MIN_TERRAIN_CLEARANCE;

    return max(base_height, min_height);
}
```

The call site, showing which xz is passed (`world.wgsl:3381-3394`):

```wgsl
fn compose_sphere_from_orbit_pga(t: f32, fe: FloatingEntityState) -> FloatingEntityState {
    // 1. Get the pure PGA orbit (circular motion via Motor)
    var s = dynamics_sphere_motor_orbit(t, fe);

    // 2. Apply terrain coupling (Height adjustment)
    //    This is an "effect" applied after the ideal motion
    let base_height = s.pos.y;  // orbit_height from motor + anchor
    let adjusted_height = coupling_terrain_to_sphere_orbit_height(
        vec2(s.pos.x, s.pos.z),
        base_height
    );

    // Update position with terrain-adjusted height
    s.pos.y = adjusted_height;
```

`vec2(s.pos.x, s.pos.z)` — the **live orbital position**, after the motor ran.

Is the orbit centred on `fe.anchor`? Yes (`world.wgsl:3355-3365`):

```wgsl
    let offset = vec3(fe.orbit_radius, 0.0, 0.0);
    let p_start = point_from_vec3(offset);
    let p_moved = sw_motor_point(m_orbit, p_start);
    let local_pos = point_to_vec3(p_moved);
    s.pos = fe.anchor + vec3(local_pos.x, fe.orbit_height, local_pos.z);
```

> **CROSS-CHECK VERDICT: the prior finding "update_sphere ignores GoL height"
> is STALE.** The sphere *does* go through POLICY_FLYER, whose mask contains
> `CONTRIB_GOL_ZONES` (the function's own comment enumerates "gol zones"), and
> it queries at the live orbital xz, not at `fe.anchor.xz`. The sphere is
> already ruling-1 compliant on its clearance read. The anchor survives only as
> the orbit's *centre* — a force-law constant in the handoff's own sense, which
> ruling 1 does not touch. **No sphere edit is owed.**

## A5 — THE CUBE'S GROUND READ, AND THE DRIFT ORDER

`world.wgsl:7486-7554`, verbatim, bob_y line through the end of the integrator:

```wgsl
            let bob_y = sin(fe.t * 6.283185 / max(fe.bob_period, 0.1)) * fe.bob_amplitude;
            var home: vec3<f32>;
            if (fe.follow_pawn != 0u) {
                let point_p = point_pos();
                let kite_xz = vec2(point_p.x + fe.pawn_offset.x,
                                   point_p.z + fe.pawn_offset.z);
                let kite_qi = QueryInputs(vec3(kite_xz.x, 0.0, kite_xz.y),
                                          signal.t_seconds);
                let ground_k = manifold_position(vec3(kite_xz.x, 0.0, kite_xz.y), POLICY_FLYER, kite_qi).y;
                home = vec3(kite_xz.x, ground_k + fe.orbit_height + bob_y, kite_xz.y);
            } else {
                let home_xz = vec2(fe.anchor.x, fe.anchor.z);
                let qi = QueryInputs(fe.anchor, signal.t_seconds);
                let ground_a = manifold_position(vec3(home_xz.x, 0.0, home_xz.y), POLICY_FLYER, qi).y;
                home = vec3(fe.anchor.x, ground_a + fe.orbit_height + bob_y, fe.anchor.z);
            }

            // ── Drift integrator ──────────────────────────────────
            ...
            let behavior_force = cube_behavior_force(
                fe, signal.t_seconds, point_xz, config.floater_coordination);
            ...
            var push_impulse = vec3(0.0);
            {
                let q_prof = row_cube_push(fe);
                let q_r = influence_response(
                    fe.pos, vec2(0.0), point_pos(), vec2(0.0), q_prof, dt);
                push_impulse = vec3(q_r.x, 0.0, q_r.y);
            }
            let spring_a = -fe.drift * fe.spring_stiffness;
            fe.drift_vel = fe.drift_vel + (spring_a + behavior_force) * dt + push_impulse;
            fe.drift_vel = fe.drift_vel * exp(-fe.drag * dt);
            fe.drift = fe.drift + fe.drift_vel * dt;
```

The FIND block's `} else { … }` arm **matches the handoff byte for byte.**
Confirmed at `world.wgsl:7496-7501`.

**ORDER: the home block comes FIRST; this frame's drift is integrated AFTER it**
(`7486-7501` home, `7503-7554` integrator). So `fe.pos` at the home block is
**last frame's** composed position — one frame of lag on the hover height, as
the handoff anticipated. Reported, not fixed. Three checks that the lag is the
*only* consequence:

- `fe.pos` is live and already trusted at that point: it is read at `:7424`
  for the point-distance eviction test, above the home block.
- It is initialised at spawn to the anchor exactly — `cube_behaviors.hpp:586`:
  `fe.pos[0] = inst.cx; fe.pos[1] = fe.orbit_height; fe.pos[2] = inst.cz;` — so
  frame 1 queries `anchor.xz`, bit-identical to today.
- Kite release (`:7457`) sets `fe.anchor = vec3(fe.pos.x, 0.0, fe.pos.z)`, so on
  a release frame `pos.xz == anchor.xz` and the replacement is again
  bit-identical.
- The plasticity leak preserves `anchor.xz + drift.xz`, and `fe.pos` is composed
  before it (`:7587` vs `:7604`), so the sum `fe.pos.xz` reads stays consistent
  across the leak.

> **Verdict:** FIND matches; order is home-then-drift, so Part B costs exactly
> one frame of lag on the hover height and nothing else; a stationary cube is
> bit-identical because `drift == 0 ⇒ pos.xz == anchor.xz`.

## A6 — WHO ELSE READS `anchor`?

`FloatingEntityState.anchor` — GPU (`world.wgsl`):

| site | what it computes | R/W | cadence |
|---|---|---|---|
| `:3365` `dynamics_sphere_motor_orbit` | `s.pos = fe.anchor + vec3(local.x, orbit_height, local.z)` — the orbit **centre** | R | PER FRAME |
| `:3388` `compose_sphere_from_orbit_pga` | ground query — passes `s.pos.xz`, **not** anchor | (—) | PER FRAME |
| `:7386` `cube_behavior_force` | `rest_xz` → CurlField / PhaseWave field sample point (the spring's **rest** point) | R | PER FRAME |
| `:7457` kite release | `fe.anchor = vec3(fe.pos.x, 0.0, fe.pos.z)` | **W** | ONCE per F7 toggle-OFF |
| `:7497-7500` anchor-mode home | `home.xz` and the clearance query point | R | PER FRAME — **Part B target** |
| `:7607-7608` plasticity leak | `fe.anchor.xz += fe.drift.xz * leak` | **W** | PER FRAME when λ > 0 |

`FloatingEntityState.anchor` — CPU (write-only; nothing reads it back):

| site | what it computes | R/W | cadence |
|---|---|---|---|
| `spheres.hpp:174` `sphere_write_gpu` | `fe.anchor = {inst.cx, 0, inst.cz}` at spawn | W | ONCE |
| `cube_behaviors.hpp:568` `cube_write_gpu` | same, for cubes | W | ONCE |
| `state.hpp:2034` `upload_cube_anchor` | the wire (`offsetof`-targeted partial write) | W | — |
| `cube_behaviors.hpp:374` `tick_cube_corral_animations` | eased corral target → `upload_cube_anchor` | W | PER FRAME while a corral anim is in flight, non-kite only |

CPU **mirror** of the cube anchor — `CubeBehaviorsState::activeCubes_[i].cx/cz`
(this is what the handoff calls "the C++ mirrors"; see C5 — it is never
refreshed from the two GPU-side writes above):

| site | what it computes | R/W | cadence |
|---|---|---|---|
| `cube_behaviors.hpp:555` `cube_register` | `ac.cx = inst.cx; ac.cz = inst.cz` | W | ONCE at spawn |
| `cube_behaviors.hpp:330-331` corral arm | `from_x/from_z` — non-kite start of the glide | R | ONCE per corral command |
| `cube_behaviors.hpp:345-346` corral arm | ring target, non-kite | W | ONCE per corral command |
| `cube_behaviors.hpp:395-396` F7 toggle-**ON** | `ox = cx - px` — **the offset capture** | R | ONCE per toggle |
| `cube_behaviors.hpp:416-417` F7 toggle-**OFF** | `cx = px + pawn_offset` | W | ONCE per toggle |

Different struct, same field name — listed so a reader does not conflate them.
`GPURibbonState::anchor` (`state.hpp:820`) is the ribbon's spawn plan point:
written at `ribbon.hpp:1221-1223` and `state.hpp:5767-5769` (ONCE), read at
`ribbon.hpp:600-601`, `:685-687` (head init, ONCE) and `:904`, `:935` (PER
FRAME, as the ground-query fallback when the slot is not the head — see A2).
`ActiveRibbon::anchor_x/anchor_z` (`ribbon.hpp:1270-1271`, read at `:923-924`
for the player-distance test) is the CPU mirror of that.

> **Verdict:** four per-frame GPU readers of `fe.anchor`. Two are force-law
> constants that ruling 1 explicitly does not touch (the sphere's orbit centre,
> the cube spring's rest point). One is already compliant (the sphere's ground
> read never touches anchor). **Exactly one per-frame reader is a clearance
> evaluation at a fixed patch of land — `:7497-7500` — and that is Part B.**
> The census finds no second site owed the same edit.

## A7 — THE EXISTING SOFT CLAMP

`world.wgsl:7228-7233`, in `update_camera`:

```wgsl
    {
        let min_clearance = 1.5;  // minimum height above terrain
        let qi = QueryInputs(pawn_pos, signal.t_seconds);  // suppression centered on the pawn (matches the pawn body)
        let ground_at_cam = manifold_position(camera.pos, POLICY_WALKER_TILT, qi).y;
        camera.pos.y = max(camera.pos.y, ground_at_cam + min_clearance);
    }
```

There is **no symbol named `SOFT_FLOOR`** anywhere in the tree; the constant is
an unnamed local literal `1.5`.

> **Verdict: it is a HARD max, not a soft blend** — `max(y, ground + c)`, one
> line, no smoothstep, no rate limit. The cube's own clamp (`:7581-7584`) is the
> same shape plus a velocity kill. So ruling 2's clamp has a twin to copy, but
> the twin is hard: if Jean wants ruling 2 *soft*, it will be a new invention
> after all, and there are then three clamps of two different characters
> (camera, cube, ribbon) worth unifying. Also note the camera's policy is
> `POLICY_WALKER_TILT`, not `POLICY_FLYER` — a ribbon clamp copying this shape
> must still pick its own policy, and A3's cost applies to that choice.

---

## PART B

FIND matched byte for byte at `world.wgsl:7496-7501` → **APPLIED**, verbatim.

Landed alongside it in the same commit: the `:7474-7482` comment repair
required by C2, and one sentence on the clearance clamp (`:7556`) recording
that it is now a safety net rather than the kernel's only live-xz reader (C1).
Both are truth-maintenance on comments the edit falsifies; no behavior change.

`[G:glaw1]` — GREEN.
`[G:visual]` — **UNCLAIMED.** Jean's gate. What to look for is in the handoff;
add C2's case to it: toggle F7 **on** while a cube is drifting across a slope
and watch for a vertical step. A cube at rest must be identical.

## PROPOSED NEXT (not in this handoff)

- **ANCHOR_2a** — ruling 1 on the kite arm (`kite_xz → pos.xz`), closing C2.
- **ANCHOR_2b** — refresh the CPU cube-anchor mirror, or delete it and read the
  offset from a GPU readback, closing C5.
- **RIBBON/ruling 2** — decide A3's fork: CPU (free, terrain-only) vs. GPU (new
  pipeline layout, full POLICY_FLYER). A2 shows the CPU query site already
  exists and only the `alt_baked` latch stands in the way.
