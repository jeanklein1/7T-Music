# CONTACT_5 — BATCH REPORT (THE POINT LAW)

Campaign: CONTACT_5 — one response body, one profile table, the point as
emitter. This is the batch's witness stand (P3). It turns the AUDIT-4
findings into code: the audit's `proximity_response` becomes
`influence_response`; the point-source flee and cube parting move onto it;
spheres come back host-agnostic; cubes become shoveable.

## Base + final

- Base: `be747e8` (origin/master — the merged AUDIT-4 report + the P 0-3
  handoffs). Executed on the review branch `claude/sync-handoffs-review-5vd7j6`
  (cut from base, cleanly fast-forward-mergeable to master — Jean merges, as
  with AUDIT-4), metadata trailers.
- Commit list:

| Commit | Hash | Intent |
|--------|------|--------|
| [P1a] | e826200 | The InfluenceProfile struct + influence_response body |
| [P1b] | 2fe91f5 | The 6 call sites collapse to profiles (behavior-preserving) |
| [P1]  | db56a8f | Final bit-preservation verdict (agents exact; 1-ULP cube dev.) |
| [P2a] | 802bc35 | Spheres push the POINT (restore, host-agnostic) |
| [P2b] | 0ec6635 | Cubes — presence, cylinder gate, persistence λ=1 |
| [P2c] | 7728597 | The point-centering sweep (audit + ledger comment) |

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
> THE TWO RESPONSE SHAPES: APPROACH (the dodge) — force ∝ v_ap; a reaction
> to MOTION; velocity floor, NOT dt-scaled (K1). PRESENCE (the shove) —
> force ∝ overlap (r−d); a reaction to OCCUPANCY; impulse, dt-scaled (K1).
>
> THE AUTHORITY TABLE: agents — approach, authority below the point, no
> persistence (FROZEN this batch). cubes — presence, authority far below,
> FULL persistence. spheres — presence, authority ABOVE the point: the
> POINT yields (same law, inequality reversed).

## THE PROFILE TABLE (as shipped — the single control surface)

One row per call site. Columns are the `InfluenceProfile` fields in order:
**radius · vwindow · presence · approach · falloff · cap · yield · tang**.
`world.wgsl` line = the profile constructor. This table replaces C3's tuning
table as the one place "how bodies react" is authored.

| # | site | radius | vwin | presence | approach | fall | cap | yield | tang | line | status |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | agent↔agent contact (player) | `contact_r+contact_r` | 0 | `CONTACT_SPRING` | 0 | 0 | `IMPULSE_CAP` | `m_o/(m_s+m_o)` | 0 | 7406 | **FROZEN** |
| 2 | agent↔agent flee (player) | `(pers+pers)·FLEE_SHELL_FRAC` | 0 | 0 | `NONPLAYER_FLEE_GAIN` | **0** | `NO_CAP` | 1 | 0.6 | 7420 | **FROZEN** |
| 3 | **sphere push (player)** | `fe.influence_radius` | 0 | `SPHERE_PUSH_GAIN` | 0 | 0 | `IMPULSE_CAP` | 1 | 0 | 7445 | NEW P2a |
| 4 | agent↔agent contact (other) | `contact_r+contact_r` | 0 | `CONTACT_SPRING` | 0 | 0 | `IMPULSE_CAP` | `m_o/(m_s+m_o)` | 0 | 7524 | **FROZEN** |
| 5 | agent↔agent flee (other) | `(pers+pers)·FLEE_SHELL_FRAC` | 0 | 0 | `NONPLAYER_FLEE_GAIN` | **0** | `NO_CAP` | 1 | 0.6 | 7538 | **FROZEN** |
| 6 | agent↔sphere contact (other) | `contact_r+fe.body_radius` | 0 | `CONTACT_SPRING` | 0 | 0 | `IMPULSE_CAP` | 1 | 0 | 7556 | **FROZEN** |
| 7 | point-source flee (other) | `config.point_bubble_radius` | 0 | 0 | `flee_gain_player` | **1** | `NO_CAP` | 1 | 0.6 | 7582 | **FROZEN** |
| 8 | **sphere push (camera)** | `fe.influence_radius` | 0 | `SPHERE_PUSH_GAIN` | 0 | 0 | `IMPULSE_CAP` | 1 | 0 | 7668 | NEW P2a |
| 9 | **cube push (cube)** | `CUBE_PUSH_RADIUS` | `CUBE_PUSH_VWINDOW` | `CUBE_PUSH_GAIN` | 0 | 1 | `CUBE_PART_CAP` | 1 | 0 | 8057 | NEW P2b |

The knob values (all authored in one place now): `CONTACT_SPRING 40`,
`CONTACT_IMPULSE_CAP 6`, `NONPLAYER_FLEE_GAIN 0.8`, `FLEE_SHELL_FRAC 0.25`,
`flee_gain_player 0.70/0.85/0.50/0.60` (agents.hpp tier column),
`config.point_bubble_radius 20`, `SPHERE_PUSH_GAIN 40`, `CUBE_PUSH_RADIUS 7`,
`CUBE_PUSH_VWINDOW 85`, `CUBE_PUSH_GAIN 25`, `CUBE_PART_CAP 12`,
`INFLUENCE_NO_CAP 1e9`, `config.cube_plasticity 1.0` (raised P2b).

## THE KNOWN DRIFT, NOW A VISIBLE COLUMN (Jean's call)

The audit found the same flee reflex with a falloff in one copy and not
another. CONTACT_5 does NOT silently resolve it — it ships as the `fall`
column: **the agent↔agent flee (rows 2, 5) ships `falloff_mix = 0`** (a flat
shell — full-strength inside, nothing at the edge), while **the point-source
flee (row 7) ships `falloff_mix = 1`** (the S2a proximity reflex — soft at the
edge). Both are now one edit away from each other in one table. Whether the
crowd's body-to-body flee should also soften is a TUNABLE, not a defect —
change row 2/5's `fall` to 1 and it does.

## P1 — THE BIT-PRESERVATION PROOF (the bisection line)

[P1] is behavior-preserving by construction; it is the known-good line the
whole campaign leans on. The lift went through an adversarial panel (one
skeptic per row, each refuting with f32 counterexamples). The first panel
found FOUR real divergences the symbolic proof had glossed; all four were
fixed in the body and a second panel confirmed:

1. **Squared gate** — inline sites gate `d2 < r*r`; the first lift gated
   `sqrt(d2) < r`, flipping a full-magnitude impulse on the flat flee shell at
   the f32 edge band. Fixed: gate in squared space.
2. **Explicit `d2`** — `d3.x*d3.x+d3.y*d3.y+d3.z*d3.z`, not `dot(d3,d3)` (fma).
3. **Scalar-first** — `esc*(min*yield)`, matching the inline `dir*(scalar)`
   grouping (f32 `*` non-associative; was 1 ULP on ~15% of pairs).
4. **Conditional normalize** — the matador split runs only for `tangential>0`
   (the flees); radial parting keeps the RAW `dir` (the first lift normalized
   → macroscopic divergence when a cube sat overhead).

**Final verdict: the FOUR AGENT rows (contact ×2, flee ×2 — really rows
1,2,4,5 + sphere contact 6 + point flee 7) are BIT-IDENTICAL to CONTACT_4.**
The one residual is a 1-ULP divergence in the (non-agent) cube parting's
pawn-host `v_ap` — OLD divided the raw-coordinate dot once, the shared body
uses `dot(other_vel, dir)`; an unavoidable trade to keep the agent point-flee
exact. It was superseded at P2b (the cube parting became a presence push with
no `v_ap`). Full per-row proof in CONTACT_5_LOG.

## P2 — THE DELIBERATE CHANGES (with worked examples)

- **P2a — spheres push the POINT (restore, host-agnostic).** A sphere's
  presence moves the point's HOST body (`yield_share 1.0`, no mass weight = the
  reversed inequality — the point takes the whole response, the sphere none).
  Player: the impulse lands on velocity, the K1b inline pos-add carries it.
  Camera-host: the same profile on `camera.pos`, `self_vel = 0` (presence needs
  no velocity — the camera-velocity gap is moot), `camera.pos += r*dt`. Both
  hosts net `(r−d)·SPRING·dt²` per frame. Shell = `fe.influence_radius`
  (per-instance, authored 6–8 wu). **Worked:** Sentinel influence 8, point 4 wu
  from centre → overlap 4 → `4·40·(1/60) = 2.67`, capped at 6 → eased out at
  ~2.67 wu/s, stops the instant it clears; standing still inside still pushed.
- **P2b — cubes shoveable (presence, cylinder, λ=1).** The cube parting
  (approach) becomes a PRESENCE push through a CYLINDRICAL gate (planar
  `CUBE_PUSH_RADIUS 7`, `|dy| < CUBE_PUSH_VWINDOW 85`) — a hovering body's shell
  is the column beneath it, escaping the CONTACT_4 spherical trap. The push is
  an IMPULSE (dt-scaled inside, added straight to `drift_vel` — the K1 law).
  **Worked:** cube 18 wu up, pawn 3 wu planar beneath → `|dy|=18 < 85`; overlap
  4, `fall = 1−3/7 = 0.57` → `4·25·(1/60)·0.57 ≈ 0.95` wu/s of drift velocity
  per frame, accumulating while under the column, STOPPING when you step out;
  standing still still pushes. Persistence: `CUBE_PLASTICITY_DEFAULT` raised
  0.6 → **1.0** so a shove RELOCATES (λ=1). Cube-vs-pawn CONTACT retired +
  `CONTACT_CUBE_RADIUS` tombstoned (a body contact never reached a hovering
  cube).
- **P2c — the point-centering sweep.** Audited every influence read of
  `config.possessed_slot`: nothing to convert — P1b/P2b already route every
  point-side POSITION through `point_pos()`. The possessed slot is read directly
  only for genuine BODY emanation (agent↔agent pairs, the pawn mass weight) and
  the point's VELOCITY host-routing. Added the point-centering ledger comment.

## ⚠ PROBLEMS SPOTTED (beyond the handoff)

1. **The handoff's P1 draft body had two agent-gate-breaking defects** (the
   double falloff, the missing tangential column) + needed a camera-host
   approach-floor. Caught by the VERIFY-AND-LOG proof + the adversarial panel;
   all fixed. Without them the agent point-flee would have weakened by ×prox —
   an agent-visible change the P1 gate forbids.
2. **The cube push is an IMPULSE, not a force.** The handoff's 0.95 wu/s worked
   example only holds if PRESENCE is dt-scaled inside and added straight to
   `drift_vel`. Passing `dt=1.0` with the old `×dt` accumulation would have
   capped the FORCE at 12 → ~0.2/frame and made the soft falloff moot. Fixed to
   the impulse form (the K1 law names presence an impulse).
3. **The S3a shell instrument drew the stale `CUBE_PART_RADIUS` (30).** The real
   planar reach is now `CUBE_PUSH_RADIUS 7`; repointed so the debug ring stays
   truthful.
4. **Sphere `influence_radius` ships at 6–8 wu, not the handoff's assumed
   8–15.** Authored + near-band, so `fe.influence_radius` ships (the worked
   example's 12 was illustrative); the shipped worked example is recomputed.

## Recount vs expectations (`_post_c5`, vs the base `be747e8`)

| Check | Expectation | Result | Verdict |
|---|---|---|---|
| cc6 layouts / flags | no deltas / EMPTY | structurally identical (only line shifts) | **MATCH** |
| cc7 declarations | +0 | structurally identical (only line shifts) | **MATCH** |
| cc7 mirror | zero orphans | zero both directions | **MATCH** |
| cc4 closures | player+camera REGAIN floating_entities; cube may change | player **+[100]**, camera **+[100]** (sphere push); cube **−[111]** (agent_tier_gains, contact retired); others unchanged | **MATCH** |
| Dawn witness | ALL GREEN | ALL PIPELINE FAMILIES GREEN, 0 module messages | **MATCH** |
| glaw1 | GREEN | GREEN (state.hpp default change) | **MATCH** |
| No new bindings / config fields | true | true (only new WGSL consts + one C++ default value) | **MATCH** |

Every new binding in a closure (100 for player/camera) is provided by that
kernel's existing layout (Compute Entity Layout) — the witness confirms the
kernels validate. The cube using one fewer binding (111) than its layout
provides is allowed (the CONTACT_4 precedent for `update_player_agent`).

## Pipeline timings (`_post_c5` vs `_post_c4`, SwiftShader create-ms — noisy)

| kernel | _post_c4 | _post_c5 | note |
|---|---|---|---|
| update_player_agent | 371 | 420 | gained the sphere loop; within create-time noise |
| update_other_agents | 627 | 847 | the shared-body inlining; noisy — see caveat |
| update_camera | 123 | 129 | ~flat |
| update_sphere | 186 | 147 | ↓ |
| update_cube | 385 | 277 | ↓ (contact retired; one push, not contact+parting) |

**Caveat:** these are pipeline-CREATION (compile-complexity) times under
SwiftShader/Tint, ~±30% run-to-run — NOT runtime performance. The cube kernel
clearly simplified; the others sit within noise. The true runtime axis stays
unmeasured (timestamp queries — the deferred frame-time instrument). Module
compile is stable (120 → 125 ms).

## Deviations

- **D1 — review-branch execution** (Jean merges, as with AUDIT-4), not a direct
  master push; the harness scopes pushes to the designated branch.
- **D2 — the 4 P1-verify body fixes** vs the handoff's draft (squared gate,
  explicit d2, scalar-first, conditional normalize) + the `approach_floor`
  param + the `tangential` profile column. All required for the agent gate;
  proven in CONTACT_5_LOG.
- **D3 — the cube push is an impulse** (dt inside, direct add), not the
  handoff-draft force (dt=1.0, ×dt). Required for the 0.95 wu/s worked example
  and the soft falloff (spotted problem #2).
- **D4 — sphere shell = fe.influence_radius (6–8 wu shipped)**; the handoff's
  8–15 assumption ran high (spotted problem #4).
- **D5 — the S3a shell ring repointed** to CUBE_PUSH_RADIUS (spotted problem
  #3); CUBE_PART_RADIUS/GAIN tombstoned (reference-free after the swap).

## Encoding sweep

world.wgsl / state.hpp: **no BOM, LF-only.** The FXC constraints banner
(lines 44–56, `SEAM[world.wgsl:fxc-constraints]`) is byte-untouched — the diff
begins at line 2215.

## JEAN'S GATE LIST (Windows)

- [ ] **Build + boot.** FXC watch: `influence_response` is called from four
      kernels; it is small and branchless-ish, but this is the first shared
      body in the contact chain. Pipeline create-times reported above.
- [ ] **AGENTS UNCHANGED** (the P1 gate, felt): walk at an agent, sprint a
      cluster, possess one. It must feel EXACTLY like CONTACT_4. If it does not,
      P1 is at fault, not P2 — bisect there (the four agent rows are proven
      bit-identical, so this should hold).
- [ ] **THE SPHERES, restored:** walk into a sphere's path — it eases you out
      and keeps pushing while you stand inside its shell. Then press 4
      (camera-only) and fly INTO a sphere — it should push the camera too. In
      free-fly the spheres are the only solid things there; that is the design.
- [ ] **THE CUBES, shoveable:** walk under a floating cube — it moves ahead of
      you while you are in its column, STOPS when you step out, and STAYS where
      you left it (λ=1). Herd one across a patch. Then try to shove one from 20
      wu away — nothing should happen (the column is small on purpose). Outdoor
      caveat: cubes whose sampled altitude exceeds ~85 wu (the σ tail) sit above
      the column and stay unreachable — a per-instance vwindow is the deferred
      refinement.
- [ ] **THE PROFILE TABLE:** open it (above) and confirm every knob you would
      want to turn is in one place, in one file.

## DEFERRED REGISTER (two items CLOSED by this batch)

- **CLOSED:** the four-copy flee drift (one body now; the drift is a visible
  column).
- **CLOSED:** sphere-pushes-the-player (restored, host-agnostic; the
  camera-velocity gap is moot for it — presence needs no velocity).
- **still open:** the presence CARD (build it when a consumer needs a FIELD —
  many emitters or persistence; AUDIT-4 probe evidence banked: writer is the
  aura's shape, `rg16float` not storage-writable → `rgba16float`) · the
  1-ULP cube-parting v_ap (dissolved at P2b) · the dead flock `neighbor_radius`
  knob · the two ungated eviction mirrors (promote to config) ·
  `config.point_vel_x/z` (now only the agents' dodge in camera-host + the
  point-flee velocity host-routing) · the `tile_apply_spawn_mult` contract
  ruling · figure-scaled contact radius · aggregate impulse clamp · AgentState
  pad debt · living-plateau steering · the ribbon floor · emissaries · the
  frame-time instrument (still the one unmeasured axis).

## Seeds

CONTACT_5 makes the influence law ONE body with a profile table — the audit's
`proximity_response`, ratified and shipped. The point is the emitter; spheres
and cubes are back on the field, host-agnostic; the agents are byte-frozen.
The contact quartet + the point law (CONTACT_1→5) is closed. Still queued:
the F campaign (sign + mask + topology split), the presence card when a
consumer needs a field, and the deferred register above.
