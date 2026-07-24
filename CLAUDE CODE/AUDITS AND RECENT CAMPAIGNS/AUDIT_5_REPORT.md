# AUDIT-5 — THE COLLISION CENSUS

**Base:** master `3cc67a897880dd201f33fa6f8141a2670e240d45` (the CONTACT_5 merge
`60818b0` + the collision-tidiness handoff commit). Every recipe is relative to
it. **Audit only — no edits merged** (the [A5-5] price build ran on this branch
and was reverted; the tree is unchanged but for these report files + `audit/a5_*.py`).

**Baseline glaw1:** `G-LAW 1: GREEN` (witness reading, not a gate — no edits).

**Verified against the LIVE TREE**, not `BATCH_REPORT_C5.md`. Where they
disagree it is called out (see [A5-8] the flock-radius wire).

## THE FRAME (three laws, one unified)

1. **BODY↔BODY** — `influence_response` + `InfluenceProfile` (world.wgsl:2344).
   Nine call sites, one body. Done at CONTACT_5.
2. **BODY↔WORLD** — `manifold_position` + `POLICY_*` query, and several
   unrelated RESPONSE shapes (revert / pin / steer). **Where the untidiness now
   lives.**
3. **AGGREGATE FIELD** — the orb flock (sep/ali/coh), its own radii/gains/signs.

**Per-section verdict:**

| § | Section | Verdict |
|---|---|---|
| A5-1 | The taxonomy | **FINDING** — ~35 altered-motion sites; 8 fit NONE of the three laws |
| A5-2 | Point sweep → behaviors | **FINDING** — pursuit + flee read the pawn body as the point; degrade to idle in camera-host (proven) |
| A5-3 | Feasibility ledger → every column | **FINDING** — presence caps are frame-hitch guards, not tuning knobs (except the sphere cap) |
| A5-4 | Tweak map | **FINDING** — the table is one file; the VALUES live in FIVE |
| A5-6 | Ground-resolve census | **FINDING** — 3 response shapes + 1 sensor + 1 null; a dead cliff-clamp fn |
| A5-7 | Accumulation law | **FINDING** — 4 fates (3 policy, 1 minor accident); the one-frame point-velocity leak |
| A5-8 | Boundary statements | **PROPOSAL-SURFACE** — name laws 1/2/3 + the follow family; two rulings |
| A5-5 | Profile block + price | **RECOMMEND** — variant B is compile-neutral; PROMOTE approach_floor |
| A5-9 | Disposition table | the deliverable that feeds tonight |

---

## [A5-1] THE TAXONOMY — the census of altered motion

**VERDICT: FINDING.** ~35 sites alter one thing's position/velocity because of
another. Law 1 is closed (9 sites, one body). Law 2 is a sprawl (see A5-6). Law
3 is the orb flock. **Eight sites fit NONE of the three laws** — the real finds:
the follow/leash/mount family (kite cube, sky-mode ribbon mount, camera-follow),
the three point-relative evictions, the CPU spawn-footprint separation (the
CPU body↔body analog of contact), and the shadow-VP snap.

**Recipe:** grep census (anchored to the velocity/position WRITE line), verified
against the live tree at `3cc67a8`. The `pairs/frame` column is the optimization
census — the table already exists when a hard limit arrives.

**Constants pinned:** SPHERE_SLOT_COUNT 8, CUBE_SLOT_COUNT 256, 32 agent slots,
MAX_ORBS 256, PAWN_AURA_N 64 (4096 cells), MAX_FOOTPRINTS 128. Player/sphere/cube
kernels `workgroup_size(1)`; other-agents `(32)`; orb `(64)`.

### Law 1 — BODY↔BODY (the closed law)

| site | file:line | response | gate | pairs/frame |
|---|---|---|---|---|
| agent↔agent contact (player) | world.wgsl:7412 | vel impulse (spring) | 3D sphere Σcontact_r | 1×32 → ~31 |
| agent↔agent flee (player) | :7426 | vel impulse (matador) | 2D flat shell ×0.25 | ~31 |
| spheres push point (player) | :7451 | vel impulse, yield 1 | 3D `fe.influence_radius` | 1×8 → 8 |
| agent↔agent contact (other) | :7530 | vel impulse (mass-wt) | 3D sphere | 32×32 → **1024** |
| agent↔agent flee (other) | :7544 | vel impulse | 2D flat ×0.25 | 32×32 → ~992 |
| agent↔sphere contact (other) | :7562 | vel impulse | 3D contact+body_r | 32×8 → 256 |
| point-source flee (other) | :7589 | vel impulse (soft) | 3D bubble ~20 | 32×1 → 32 |
| spheres push point (camera) | :7673 | pos impulse ×dt | 3D `fe.influence_radius` | 1×8 → 8 (free-fly) |
| point SHOVES cubes | :8065 | drift impulse (presence) | **cylinder** 7 / \|dy\|<85 | 256×1 → 256 |

### Law 2 — BODY↔WORLD (the sprawl; full census in A5-6)

Ground resolve (snap/slide/revert, world.wgsl:6488/6500/6620), gradient steering
(:6595/:6715), the pin family (world-bound :6742, camera terrain :7734 / wall
:7744 / ceiling :7750, sphere clearance :3620, cube clearance :8094, cube home
:8009), the portal sensor (:6811). **Two INVERTED law-2 members — a body moves
the GROUND:** the pawn aura lifts terrain (:4312, field authored by
`compute_pawn_aura` :8945) and GoL-suppression flattens ground under a body
(:2564). Pair cost: aura = 64²=4096 cells + per-vertex; suppression = per walker
ground query (pawn + 32 agents + camera).

### Law 3 — AGGREGATE FIELD (the orb flock)

Separation (:12472), alignment (:12476), cohesion (:12480), all in ONE O(N²)
loop over `orb_config.count` (:12465, ceiling 256) → **256² = 65 536 pair-evals/
frame — the system's largest.** Plus the speed clamp (:12542). Reads
`orb_state_prev` for feedback isolation. (The dome-shell reprojection :12566 is
law 2 — orb↔world-shell.)

### ⚠ FLAGGED — fits NONE of the three laws (the real finds)

| site | file:line | what it is | proposed home |
|---|---|---|---|
| sky-mode pawn snap to ribbon head | world.wgsl:6658 | body MOUNTS a structure (rides the ribbon) | **law 4: ATTACHMENT/FOLLOW** |
| kite-mode cube follows the point | :8004 | cube home LEASHED to point + offset | law 4 |
| camera aim-point lerp → pawn | :7707 | damped position follow (τ=0.30) | law 4 |
| camera orbits pawn aim | :7713 | camera composed from pawn aim | law 4 |
| agent eviction (point-relative) | :7612 | pos-gated lifecycle deactivate | **lifecycle, not motion** |
| sphere eviction | :7775 | pos-gated deactivate (400) | lifecycle |
| cube eviction | :7939 | pos-gated deactivate (400) | lifecycle |
| CPU spawn-footprint reject | spawn_engine.hpp:535 | **the CPU body↔body analog of contact** (own `MIN_SEPARATION` table) | **law 1, spawn-time, CPU** |
| CPU spawn indoor-bounds clamp | spawn_engine.hpp:294 | placement clamp into room | law 2, spawn-time, CPU |
| pawn→sun shadow-VP snap | world.wgsl:3644 | light follows the pawn (grid-snapped) | rendering follow |

The two structural finds: **(a)** a whole ATTACHMENT/FOLLOW family (kite,
sky-mount, camera-follow) exists with no named law; **(b)** the CPU spawn
separation is BODY↔BODY collision (law 1's intent) implemented in a different
place, time, and vocabulary (`MIN_SEPARATION[fam][fam]` + footprint radii) — the
same "the law protects only the surface its invariants name" gap the charter
warns of. `[anchor-1]`

---

## [A5-2] THE POINT SWEEP, EXTENDED TO THE BEHAVIOR LAYER

**VERDICT: FINDING.** CONTACT_5 P2c swept the INFLUENCE reads and stopped.
`behavior_pursuit` and `behavior_flee` open with
`let player = agent_state[config.possessed_slot]` and use it as the player's
POSITION — a POINT-term. In camera-host they gate on the abandoned pawn and
**degrade to idle wander (proven below)**.

**Recipe:** `audit/a5_reader_census.py` (grep of `config.possessed_slot`).

**The four behavior-layer reads:**

| behavior | file:line | reads for | class | conversion |
|---|---|---|---|---|
| biased_walk | world.wgsl:6907 | skip-self (index) | BODY/index | none (non-convertible) |
| **pursuit** | **world.wgsl:7076** | **target position** | **POINT-term** | `let p = point_pos(); let dx = p.x - a.pos_x; let dz = p.z - a.pos_z;` |
| **flee** | **world.wgsl:7119** | **target position** | **POINT-term** | `let p = point_pos(); let dx = a.pos_x - p.x; let dz = a.pos_z - p.z;` |
| flock2d | world.wgsl:7184 | skip-self (index) | BODY/index | none |

**The camera-host degradation — CONFIRMED by arithmetic.** Let C = camera
(= the point in camera-host), P = abandoned pawn, A = a live agent.
- Live ⟹ `|A−C| ≤ AGENT_EVICTION_RADIUS 350` (evicted otherwise, :7612, around
  `point_pos()` = C).
- Pursuit fires ⟹ `|A−P| < 40` (neighbor_radius, agents.hpp:148); flee ⟹ `< 30`.
- Triangle: `|P−C| ≤ |P−A| + |A−C| < 40+350 = 390` (pursuit) / `380` (flee).

**Contrapositive:** once the camera is ≥ 390 wu (pursuit) / ≥ 380 wu (flee) from
the abandoned pawn, the detect shell around the pawn lies entirely outside the
350-wu eviction disk around the camera — it can contain only dead slots. The
gate cannot fire; pursuit → its beat-time wander else-branch (:7089), flee → its
idle wander (:7133). The 40/30-wu shell is anchored on the WRONG body. Routing
:7076 / :7119 through `point_pos()` repairs it. **This is the A5-2 defect, of the
exact shape the charter predicted — a sweep written for one layer, never
extended.** `[anchor-2]`

---

## [A5-3] THE FEASIBILITY LEDGER, EXTENDED TO EVERY COLUMN

**VERDICT: FINDING.** Every PRESENCE cap is UNREACHABLE at 60 Hz **except the
sphere push cap**. The contact + cube caps are **frame-hitch guards**, not tuning
knobs — a constant presented as a knob that is really a hitch guard is the drift
the charter forbids.

**Recipe:** `audit/a5_feasibility.py` (authored values pasted with file:line).

| row | max magnitude @ 60 Hz | cap | reachable @60? | cap-engagement dt (fps) |
|---|---|---|---|---|
| 1/4 agent contact | 4.0·40/60 = **2.67** | 6 | no | 0.0375 s (**26.7 fps**) |
| 3/8 sphere push (μ=8) | 8·40/60 = **5.33** | 6 | no (but close) | 0.0187 s (**53.3 fps**) |
| 3/8 sphere push (r≥9, σ tail) | 9·40/60 = **6.0** | 6 | **YES** | ≤ 60 fps |
| 6 agent↔sphere | 3.5·40/60 = **2.33** | 6 | no | 0.0429 s (23.3 fps) |
| 9 cube push | 7·25/60 = **2.92** | 12 | no (by ~4×) | 0.0686 s (**14.6 fps**) |
| 2/5/7 flees | approach ≈ v_ap·gain ≲ 15 | 1e9 | never | — |

**The finding, precisely.** `CUBE_PART_CAP` and the contact `CONTACT_IMPULSE_CAP`
engage only below 15–27 fps — they are **frame-hitch guards** (they stop a long
frame from teleporting a body through a wall). BUT `CONTACT_IMPULSE_CAP 6` is
ALSO doing double duty: for spheres with `influence_radius ≥ 9 wu` (the σ tail of
Sentinel μ8/σ2, i.e. a common draw) it bites at 60 Hz — there it is a REAL
limiter. So the same const is a hitch guard for contact and a live clamp for
large spheres. The ledger must SAY which, per row.

**Static-assert proposal.** The const-only presence rows (contact, cube push) can
be asserted C++-side where the values live — e.g. in `bodies/agents.hpp` beside
the tier table for contact, and a mirror check for the cube consts:
```cpp
static_assert(4.0f * CONTACT_SPRING / 60.0f < CONTACT_IMPULSE_CAP,
    "agent contact cap is a frame-hitch guard: unreachable at 60 Hz "
    "(max contact_r pair 2.0+2.0). If this fires the cap became a tuning knob.");
static_assert(CUBE_PUSH_RADIUS * CUBE_PUSH_GAIN / 60.0f < CUBE_PART_CAP,
    "cube push cap is a frame-hitch guard: unreachable at 60 Hz.");
```
Placement: the CONTACT_SPRING/CONTACT_IMPULSE_CAP/CUBE_PUSH_* values live as
WGSL consts (no C++ twin today) — so either add a C++ mirror header for them, or
the WGSL-side alternative is a `const_assert` (WGSL 1.0 has `const_assert`,
supported by Tint — verify on target). **What CANNOT be asserted:** the sphere
push, because `influence_radius` is a per-instance SAMPLED value (Gaussian, no
compile-time bound); the σ-tail engagement is a runtime fact. Say so honestly:
the sphere row's cap is documented as a live limiter, not asserted away.
`[anchor-3]`

---

## [A5-4] THE TWEAK MAP — where every number physically lives

**VERDICT: FINDING.** The BATCH_REPORT claim "every knob in one place, one file"
is true for the TABLE (the constructors, all in world.wgsl) and **false for the
VALUES: they live in FIVE files.**

| column | value home · mechanism |
|---|---|
| radius (contact) | `contact_radius` — **agents.hpp** tier table (baked → `AgentTierParams` uniform) |
| radius (flee) | `personal_radius` (agents.hpp tier) × `FLEE_SHELL_FRAC` (world.wgsl const) |
| radius (point flee) | `config.point_bubble_radius` — **contracts/point.hpp** `POINT_BUBBLE_RADIUS` → config upload |
| radius (sphere push) | `fe.influence_radius` — **bodies/spheres.hpp** tier table, per-instance baked GPU field |
| radius (agent↔sphere) | `contact_radius` (agents.hpp) + `fe.body_radius` (spheres.hpp, per-instance) |
| radius/vwindow (cube) | `CUBE_PUSH_RADIUS` / `CUBE_PUSH_VWINDOW` — world.wgsl const |
| presence_gain | `CONTACT_SPRING` / `SPHERE_PUSH_GAIN` / `CUBE_PUSH_GAIN` — world.wgsl const |
| approach_gain | `NONPLAYER_FLEE_GAIN` (world.wgsl const) / `flee_gain_player` (agents.hpp tier) |
| falloff_mix, tangential | literal in the constructor — world.wgsl |
| cap | `CONTACT_IMPULSE_CAP` / `INFLUENCE_NO_CAP` / `CUBE_PART_CAP` — world.wgsl const |
| yield_share | `m_o/(m_s+m_o)` computed inline (world.wgsl) from `contact_mass` (agents.hpp) × `PAWN_CONTACT_MASS_MULT` (world.wgsl) |
| approach_floor (param) | `BUBBLE_PART_SPEED` (world.wgsl const) or 0 literal |
| persistence | `config.cube_plasticity` ← `Idle::CUBE_PLASTICITY_DEFAULT` — **realization/state.hpp** |

**Files the values live in: FIVE** — world.wgsl, bodies/agents.hpp,
contracts/point.hpp, realization/state.hpp, bodies/spheres.hpp. This is the
tweakability answer AND the master-control-panel prerequisite: a real control
panel must reach across five files (or the values must be centralized first).
The distribution is not wrong — each value lives with its table — but the "one
file" claim is a report overstatement; the tree is the truth. `[anchor-4]`

---

## [A5-6] THE GROUND-RESOLVE CENSUS — law 2, made legible (SEED)

**VERDICT: FINDING (seed only, no unification proposed).** Not one vocabulary,
not N unrelated ones: **3 genuine response shapes + 1 sensor + 1 null**, with the
sprawl coming from ONE primitive (the pin/clamp) open-coded at six sites.

**Recipe:** grep census of `POLICY_*`, `manifold_position`, `clamp`/`max`/`min`
on `.pos`, in world.wgsl. Policy enum world.wgsl:2797.

| # | site | policy | response | when blocked | owner |
|---|---|---|---|---|---|
| 1 | walker step-gate + slides + revert (6474-6513) | WALKER + WALKER_TILT | **revert** (block→X-slide→Z-slide→revert, `w=0`) | revert to prev + vel kill (6753) | `pawn_ground_resolve` |
| 2 | agent ground snap (6620) | WALKER_AGENT | **snap** | never blocks | `agent_settle` |
| 3 | cube terrain-clearance floor (8089) | FLYER | **pin** (clamp-from-below + vel.y kill) | floors Y, kills down-drift | `update_cube` |
| 4 | world-bound XZ clamp (6741) | none | **pin** (box) | clamp both axes | `behavior_player_controlled` |
| 5 | camera terrain floor (7730) | WALKER_TILT | **pin** (max-Y) | `max(pos.y, ground+1.5)` | `update_camera` |
| 6 | camera wall clamp (7738) | none | **pin** (box) | clamp ±wall_margin | `update_camera` |
| 7 | camera ceiling (7747) | none | **pin** (min-Y) | `min(pos.y, ceiling−3)` | `update_camera` |
| 8 | camera-host early return (7633) | none | **NULL** (TERRAIN RULE = NONE) | every clamp skipped | `update_camera` |
| 9 | portal vertical gate (6801) | FLYER | **sensor** (sets trigger, no motion) | — | `behavior_player_controlled` |
| 10 | gradient steering (6584 / 6704) | none (baked field) | **steer** (soft level-set deflect) | never blocks (a whisper) | `agent_post_step` / player |
| — | `query_ground_walker_walkable` cliff-clamp (3473) | WALKER | **would-be pin** | **ZERO CALLERS (latent)** | — DEAD |

**The answer.** The verbs (block/slide/clamp/snap/steer) collapse to THREE
decisions: **(1) revert** (only `pawn_ground_resolve` can say *no* and fall back;
the single rich responder), **(2) pin** (snap/clamp on an axis via max/min/clamp
— replicated inline at six sites #2-#7; the code itself flags the sprawl at
world.wgsl:3507 "consumers clamp via the separate containment layer"; its N-ness
is historical accident, one primitive open-coded), **(3) steer** (a level-set
whisper, upstream of the wall, never rejects). Plus the portal SENSOR (#9, no
motion) and the camera-host NULL (#8, deliberate absence). **Latent DELETE:**
`query_ground_walker_walkable` has zero callers (the operative wall is #1's
`PAWN_STEP_HEIGHT` gate). Seed for a "law-2 unification" campaign: collapse the
six pin sites onto one `clamp_axis` / containment primitive. `[anchor-6]`

---

## [A5-7] THE ACCUMULATION LAW — where the response lands

**VERDICT: FINDING — three deliberate policies + one minor accident.**
`influence_response` returns a planar wu/s delta; four things happen to it:

| # | host | fate (verbatim) | verdict |
|---|---|---|---|
| 1 | non-player agent | impulse added AFTER `agent_post_step` (drag/cap/steer run inside the behavior, before the gather) → **escapes that frame's drag, persists** | policy (K1) |
| 2 | player | same add, but `behavior_player_controlled` AUTHORS vel each frame → imposed part **does not accumulate**; only the K1b inline pos-add survives | policy (K1b) |
| 3 | cube | `drift_vel += push_impulse` **then** `drift_vel *= exp(-drag·dt)` → **damped the same frame it lands** | **minor accident** |
| 4 | camera | no vel state; `camera.pos += r·dt` directly | policy (no vel host) |

**Quantify (3).** cube drag `CUBE_DEFAULT_DRAG = 1.5/s` (cube_behaviors.hpp:58),
so `exp(-1.5/60) = 0.9753` — the cube push is **damped ~2.5% the frame it lands**,
whereas the agent impulse (fate 1) escapes that frame's drag entirely. Same word
("impulse"), different treatment, because the cube's drag line follows the push
while the agent's drag (in `agent_post_step`) precedes the gather. It is an
ORDERING accident, not a decision — but the magnitude is 2.5%, so KEEP-WITH-REASON
(disclose it; do not re-order the cube integrator for 2.5%).

**The one-frame leak — CONFIRMED.** Dispatch order
`ribbon → player → others → camera → sphere → cube → vp`. `update_player_agent`
writes back velocity INCLUDING the gather's impulses (e.g. a sphere shove, P2a);
then `update_other_agents` row 7 reads `agent_state[possessed_slot].vel_*`
(world.wgsl:7578) as the POINT's velocity for the crowd's flee. **So a sphere
shoving the pawn is read by the crowd as the point lunging, for one frame**
(magnitude ≤ the sphere push ~2.67–6 wu/s). Next frame the player re-authors
velocity (fate 2), so it does not compound. **Rule: percept, not artifact —
KEEP-WITH-REASON.** It is a one-frame, disclosed softness of the same family as
the flock's racy-neighbor read; arguably a feature (the crowd flinches when the
point is struck). `[anchor-7]`

---

## [A5-8] THE BOUNDARY STATEMENTS

**VERDICT: PROPOSAL-SURFACE.** One sentence per law, for the charter. The
asymmetry the charter names (the point-centering ledger has a boundary statement;
the behavior layer never did → the A5-2 defect) is exactly why these must ship.

- **Law 1 (BODY↔BODY, `influence_response`)** governs one body's velocity/drift
  response to another body's occupancy or approach, per-pair, per GPU frame; it
  does NOT govern spawn-time overlap (that is the CPU footprint separation,
  A5-1), nor a body's response to the world (law 2), nor camera/mount following
  (the follow family). *Because* its invariant is "pairwise, runtime, motion."
- **Law 2 (BODY↔WORLD, `manifold_position` + POLICY_*)** governs how the world
  constrains a body's position (revert / pin / steer); it does NOT govern
  body-body, and the camera-host branch deliberately opts OUT (TERRAIN RULE =
  NONE). *Because* its invariant is "the world is the other party."
- **Law 3 (AGGREGATE FIELD, orb flock)** governs many-body emergent motion via
  separation/alignment/cohesion over a previous-frame snapshot; it does NOT
  share law 1's per-pair response body. *Because* its invariant is "the field is
  the neighborhood, read stale to avoid feedback."
- **Law 4 — NAME IT (the follow/attachment family)** governs a body's position
  SET to track another (kite cube, sky-mode ribbon mount, camera aim-follow); it
  is not collision (no rejection, no response shaping) — it is subordination.
  Today it is unnamed and scattered (A5-1 flagged 4 members).

**RULING 1 — the orb flock.** Separation IS a radius-gated pair repulsion, BUT
its `diff/d²` accumulator is **normalized to a unit direction** (world.wgsl:12488)
before the gain — so its applied magnitude is NOT overlap-proportional like
`influence_response` presence; the 1/d² shapes DIRECTION only. **Do not merge
separation onto the profile table** (it would change its magnitude law); **fence
the whole flock as law 3** and name it. Price: fencing = naming + a boundary
statement (free); merging separation = a new profile magnitude mode + splitting
the shared O(N²) loop + the `orb_state_prev` snapshot (real cost, no win).
Recommend FENCE.

**RULING 2 — `neighbor_radius` vs `personal_radius` (delete-sensitive).** Precise
reader census (`audit/a5_reader_census.py`):
- `b.neighbor_radius` is **LIVE**: biased_walk (25, :6913), pursuit (40, :7080),
  flee (30, :7123 + :7128).
- `g.personal_radius` (tier) is read by: flock2d (:7190), and the two contact
  flee-shells (:7421, :7539).
- **The flock gates on `g.personal_radius`, NOT `b.neighbor_radius`** (:7190) —
  so `AGENT_BEHAVIORS[8].neighbor_radius = 30` (the flock row) is the ONLY DEAD
  value. **Do NOT delete the `neighbor_radius` column** (live for three
  behaviors); the flock row's value is documentation-only. Either wire the flock
  to read `b.neighbor_radius` (then the column is fully live and the tier
  personal_radius is the pure contact radius), or annotate the flock row as
  intentionally-unused. RULE-NEEDED — a wrong delete is a silent flock loss.
  `[anchor-8]`

---

## [A5-5] THE PROFILE BLOCK — proposal + price

**VERDICT: RECOMMEND both.** Variant B is compile-neutral; `approach_floor`
promotion is free and removes the last knob outside the table.

**Proposal.** Collapse the nine scattered `InfluenceProfile(...)` constructors
into ONE contiguous block of `fn row_*(...) -> InfluenceProfile` builders, called
from the sites; dynamic columns (`fe.influence_radius`, `m_o/(m_s+m_o)`, tier
gains) stay parameters. **FXC-safe:** no array, no runtime indexing — the banner
(`SEAM[world.wgsl:fxc-constraints]`) names embedded-array/texture stamps as a
hang class; a function returning a constructed struct is not that shape (Tint
inlines it). Quote when it brushes the banner: functions ✓, `const` array
indexed at runtime ✗.

**Price (built on this branch, reverted; Dawn witness, ±30% SwiftShader noise):**

| | variant A (inline) | variant B (row fns) |
|---|---|---|
| module compile ms | 137 (one 392 cold-cache outlier) | 119 / 120 |
| player kernel ms | 447–758 | 459–526 |
| other kernel ms | 812–962 | 632–685 |
| cube kernel ms | 316–346 | 309–323 |
| verdict | GREEN | GREEN, 0 messages |

**Both GREEN; every delta is within the ±30% noise, and variant B's numbers
trend slightly LOWER** (Tint inlines the builders → equivalent compiled shader).
**RECOMMEND variant B** — a real legibility win (the profile table becomes code
in one place, next to `influence_response`) at zero measured FXC cost. Not a
mandate; if a future kernel adds branches this is worth re-pricing, but at nine
static builders it is free.

**PROMOTE `approach_floor` to the 9th `InfluenceProfile` column.** It is
constructed at the call site exactly like the other dynamic columns
(`BUBBLE_PART_SPEED` camera-host / 0 else), so nothing structural blocks it; it
drops a parameter from the `influence_response` signature and moves the last knob
into the table. Compile-neutral (same inlining). RECOMMEND. `[anchor-5]`

---

## [A5-9] THE DISPOSITION TABLE — the deliverable that feeds tonight

`finding · verdict · size · risk · blocked-by`. Verdicts: DELETE · RENAME ·
PROMOTE · RULE-NEEDED · KEEP-WITH-REASON.

| # | finding | verdict | size | risk | blocked-by |
|---|---|---|---|---|---|
| 1 | `query_ground_walker_walkable` (3473) AND `query_ground_walker_gradient` — BOTH zero callers, documented as "intent" at ground_architecture.hpp:178-180 (live tilt path is `terrain_normal_at`) | **DELETE** (YAGNI forbids "reserved"; the intent is a `gradient=true` policy flag with no live consumer) — or KEEP-WITH-REASON if the gradient path is genuinely near | ~15 lines each | low (latent, no callers; the doc comment is the only ref) | grep-confirm 0 refs; Jean stamp on the "intent" |
| 2 | `approach_floor` is the 9th column outside the struct | **PROMOTE** to `InfluenceProfile` col | small | low (compile-neutral, A5-5) | none |
| 3 | `CUBE_PART_CAP` names a mechanism retired at P2b; now caps a presence impulse AND is a frame-hitch guard | **RENAME** → `CUBE_PUSH_CAP` (+ comment "hitch guard, unreachable at 60 Hz") | rename + comment | low | A5-3 verdict |
| 4 | `AGENT_BEHAVIORS[8].neighbor_radius` (flock row) is DEAD; column LIVE for pursuit/flee/biased_walk | **RULE-NEEDED** — wire flock to it OR annotate unused; do NOT delete the column | 1 line or comment | **high if column deleted** (silent flock loss) | Jean stamp |
| 5 | pursuit + flee read the pawn body as the POINT (world.wgsl:7076/7119); degrade to idle in camera-host | **RULE-NEEDED** — route through `point_pos()`? (changes camera-host feel: agents would chase/flee the point) | 2 lines each | medium (behavior change) | Jean stamp |
| 6 | presence caps are frame-hitch guards, not tuning knobs (except sphere ≥9 wu) | **RULE-NEEDED** — add the static_asserts + label each cap | asserts + comments | low | A5-3; C++ mirror or WGSL const_assert |
| 7 | `CUBE_PUSH_VWINDOW 85` is an accident of the authored MEAN (75+2+8); σ reaches 45 | **RULE-NEEDED** — per-instance vwindow (`fe.orbit_height + fe.bob_amplitude·? + margin`, ONE expression, fields resident at :8058) OR rule the ceiling deliberate | 1 expression OR a comment | low | Jean stamp; verify scope (below) |
| 8 | tombstone comments (`CONTACT_SPHERE_RADIUS`, `CONTACT_CUBE_RADIUS`, `CUBE_PART_RADIUS/GAIN`) describe retired mechanisms | **KEEP-WITH-REASON** — they ARE the gate-feasibility lesson record; charter flags "comment on retired mechanism" but these are cited by future work. Compress to a one-line epitaph each if strict | trim | low | Jean taste |
| 9 | the follow/attachment family (kite, sky-mount, camera-follow) fits no law | **RULE-NEEDED** — name **law 4 (ATTACHMENT/FOLLOW)** in the charter | boundary statement | none (doc) | A5-8 |
| 10 | CPU spawn-footprint separation is the CPU body↔body analog of contact (own `MIN_SEPARATION`) | **KEEP-WITH-REASON** — legitimately CPU spawn-time; NAME it as law-1's spawn-time sibling in the charter | boundary statement | none (doc) | A5-8 |
| 11 | orb separation is direction-only, not overlap-proportional | **KEEP-WITH-REASON** — fence the flock as law 3; do NOT merge onto the profile table | boundary statement | none (doc) | A5-8 ruling 1 |
| 12 | the six law-2 pin sites open-code one clamp primitive | **RULE-NEEDED** (next campaign) — seed a `clamp_axis`/containment unification | (next batch) | medium | A5-6 seed |
| 13 | cube push damped 2.5% same-frame (ordering); one-frame point-velocity leak | **KEEP-WITH-REASON** — both disclosed softnesses, sub-percept | comment | none | A5-7 |
| 14 | the profile VALUES live in 5 files (report says "one") | **KEEP-WITH-REASON** — values live with their tables; fix the report's overstatement, not the tree | doc | none | A5-4 |
| 15 | `CONTACT_SHELL_DEBUG` draws no sphere shell (`fe.influence_radius` per-instance) | **KEEP-WITH-REASON / DECLINE** — per-instance can't be one ring; add a per-sphere loop only if the debug view needs it (low value) | — | none | Jean taste |
| 16 | profile block scattered across 4 kernels | **PROMOTE** — variant B (contiguous `row_*()` builders), compile-neutral | ~40 lines | low | A5-5 |

**Scope verify for #7 (per-instance vwindow).** `fe.orbit_height` (offset 60)
and `fe.bob_amplitude` (offset 104) are per-instance GPU fields resident on
`FloatingEntityState` (world.wgsl:984 struct); at the cube-push constructor
(world.wgsl:8057) `fe` is in scope and `home`/`bob_y` are locals — so a
per-instance vwindow (`fe.orbit_height + bob margin + 8`) is ONE expression, not
a new mechanism. Confirmed. The choice (every cube shoveable vs. very-high cubes
out of reach like balloons) is Jean's; the current 85 is neither — it is the
mean's accident.

**The tonight-batch shortlist (unblocked, mechanical):** #1 DELETE (dead
cliff-clamp), #2 PROMOTE (approach_floor), #3 RENAME (CUBE_PUSH_CAP), #16 PROMOTE
(variant B). The RULE-NEEDED rows (#4, #5, #7, #9-#12) want a Jean stamp before
they move. `[anchor-9]`

---

## STANDING-CONSTRAINT COMPLIANCE

- **No edits merged.** The [A5-5] price build ran on this branch and was reverted
  (`world.wgsl` byte-restored; `git status` clean but for report + `a5_*.py`).
- **Tree is the truth** — every anchor verified live at `3cc67a8`; the one
  report/tree disagreement (the flock radius wire) is called out (A5-8).
- **Counts ship with recipes** — `audit/a5_feasibility.py`, `audit/a5_reader_census.py`.
- **FXC banner is law** — the A5-5 proposal is checked against it (functions ✓,
  runtime-indexed arrays ✗) and priced GREEN.
- **Encoding** — this file + `a5_*.py` are LF-only, no BOM.
- **YAGNI** — every disposition is DELETE or KEEP-WITH-REASON (or a Jean-stamped
  RULE-NEEDED); nothing reserved.
