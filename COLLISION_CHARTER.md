# THE COLLISION CHARTER — the four laws of altered motion

*Established by TIDY_1 (THE AUDIT-5 STAMPS), formalizing the boundary statements
the CONTACT_1..5 and AUDIT-5 work converged on. This is the doctrine the audited
tree was stamped against; where a mechanism does not fit a law here, that is a
finding, not a licence to invent a fifth.*

Every mechanism that moves an entity against its own volition — every "altered
motion" — belongs to exactly **one** of four laws. The value of the taxonomy is
that it makes miscategorization visible: a body-vs-body shove wearing a
world-response costume (the CONTACT_4 spherical-gate trap) is a category error,
and naming the four categories is how you catch it.

---

## Law 1 — BODY ↔ BODY (the influence law)

**Mechanism.** `influence_response(self, other, profile, dt)` +
`InfluenceProfile`. One body, two response *shapes* selected by the profile
columns:
- **PRESENCE** — a reaction to OCCUPANCY: force ∝ overlap `(r − d)`, dt-scaled
  (the K1 impulse law). The shove.
- **APPROACH** — a reaction to MOTION: force ∝ closing speed, a velocity floor,
  NOT dt-scaled (dt-invariant). The dodge, with the matador tangential split.

**The profile is the contract** (9 columns after TIDY_1): `radius`, `vwindow`,
`presence_gain`, `approach_gain`, `falloff_mix`, `cap`, `yield_share`,
`tangential`, `approach_floor`. Extend it; do not fork it. It is built by the
contiguous `row_*()` builders beside `influence_response` — the profile *table as
code* (TIDY_1 T1c/T1d). Every body-vs-body interaction the demo has — agent
contact, body-to-body flee, sphere-pushes-point, agent-vs-sphere, the point-flee
bubble, cube-push — is one row of that table.

**Invariants.**
- The gate is `vwindow > 0 ⇒ cylindrical` (planar radius, vertical half-window),
  `vwindow ≤ 0 ⇒ spherical`. A hovering body's shell is the *column* beneath it;
  a spherical gate would need a radius larger than the altitude, dragging planar
  reach out with it — the CONTACT_4 trap. **Never gate a floating body
  spherically.**
- Caps guard PRESENCE rows (dt-scaled — a frame hitch can inflate one impulse).
  APPROACH-only rows carry `INFLUENCE_NO_CAP`: a dt-invariant term cannot run
  away, so there is nothing to guard.

**Spawn-time sibling.** The CPU **spawn-footprint separation**
(`spawn_services.hpp` `register_footprint` + the per-family-pair spacing table)
is Law 1's counterpart *before the sim starts*: it keeps entities apart at
BIRTH, the same "no two bodies occupy one place" intent, enforced at spawn rather
than per frame. It is not a fifth law; it is Law 1's spawn tense.

---

## Law 2 — BODY ↔ WORLD (the manifold law)

**Mechanism.** `manifold_position(pos, POLICY_*, query)` + the `POLICY_*` family
(`POLICY_FLYER`, `POLICY_WALKER`, `POLICY_WALKER_TILT`, `POLICY_WALKER_AGENT`).
The world moves the body along the terrain manifold. Three response modes:
**revert** (snap back onto the surface), **pin** (hold at a fixed clearance),
**steer** (deflect along the gradient).

**Invariant.** The world is the authority; the body has no yield share against it
(contrast Law 1's `yield_share`). The manifold query is a READ of static terrain
+ dynamic occupancy masks; it never writes the world. A body-vs-body concern that
tries to ride this path (e.g. "the pawn pushes terrain") is miscategorized —
that is Law 1 or a coupling, not Law 2.

---

## Law 3 — AGGREGATE FIELD (the flock law)

**Mechanism.** Emergent multi-body fields: agent `behavior_flock2d` (Vicsek
alignment/cohesion/separation) and the **orb flock** (`orb_config` tier gains —
`orb_tier_flock_{sep,align,coh}_gain`). Motion arises from the *statistics* of
many neighbors, not any single pair.

**Invariant.** No pair is privileged; the field is a reduction over sampled
neighbors within `neighbor_radius`. This is deliberately NOT Law 1: there is no
`InfluenceProfile`, no yield share, no gate — a flock is a perception, not a
contact. Racy neighbor reads are the disclosed softness the field absorbs.

---

## Law 4 — ATTACHMENT / FOLLOW (the mount law)

**Mechanism.** A follower's transform is SLAVED to a host's, not pushed by a
force: the **kite cube** (cube home tracks `point_pos()` in kite mode), the
**sky-mode ribbon mount** (`sky_mode != 0` — pawn mounted on the ribbon head,
terrain tilt bypassed, `SEAM[ribbon:sky-mode]`), the **camera aim-follow** (the
damped `aim_point` lerp in `update_camera`), and the **sun-VP snap**
(`COUPLING_PAWN_TO_SUN_VP`).

**Invariant.** Follow is a *kinematic* relationship — the follower reads the
host's transform and composes its own; there is no reaction force, no cap, no
yield. If a "follow" needs a spring or a shove, it is Law 1 in disguise.

---

## The feasibility rule (generalized from the TIDY_1 T1e cap ledger)

> **A value split across rooms cannot be asserted in either.** When a constraint's
> terms straddle categories — one a compile-time `const`, another a runtime
> `uniform` field — no `const_assert` can see the whole expression. Therefore
> **consolidation of values that must be checked together is ENFORCEMENT, not
> preference.**

**Concrete instance (T1e).** The cube cap row is `const_assert`able —
`CUBE_PUSH_RADIUS * CUBE_PUSH_GAIN < CUBE_PUSH_CAP * 60` — *because* all three
terms are module consts; that row compiles its own hitch-guard proof. The contact
and sphere rows are NOT assertable, because their reach term (`contact_radius`,
`fe.influence_radius`) lives in a uniform while the caps are consts. That
un-checkability is the first concrete argument for moving the tier radii next to
the caps: not tidiness, but the difference between a machine-checked invariant and
a comment nobody re-derives.

**Corollary.** `const_assert` is available and *evaluated* on the target
Dawn/Tint (a false assertion errors `const assertion failed` — verified T1e), so
any all-const invariant SHOULD ship as one. A prose ledger line is the fallback
only when the terms cannot be brought into one room.

---

## The containment rule (generalized from FIX_TILE_PATCH_CONTAINMENT)

> Where two independently-authored bounds must **nest**, the nesting is a **LAW,
> not a margin**. State which bound contains which, and enforce it
> **structurally** — by construction or by predicate — never by choosing a number
> with slack in it.

Two instances, one shape, a day apart:

- A **lifecycle radius must exceed the spawn radius**, or every entity committed
  at the frontier dies the frame it spawns — `FLOATER_EVICTION_RADIUS` 400 → 800:
  the eviction line has to sit outside the allocation frontier (~400 wu near-edge,
  ~566 at the diagonal corner).
- A **tile's lifetime must contain every patch that stands on it**, or a centre
  jump orphans the patch — `FORGET_RADIUS`. Tile eviction is unbudgeted and
  immediate; patch eviction is budgeted and lags. In steady state the
  `FORGET_RADIUS = PATCH_PREGEN_RADIUS + 2` slack hides the gap; a world-transition
  centre jump consumes it in one unbudgeted call while patch eviction still
  drains. Fixed by a keep-predicate (`build_active_patch_set`) so a load-bearing
  tile is spared however far the centre moved — containment is now a predicate,
  not a slack number.

Both failed **silently**. Both were guarded only by slack, and slack is consumed
by whatever moves fastest. And the retired `FLOATER_EVICTION_RADIUS` comment was
not unreasoned — it justified 400 against a "350 spawn radius + 50 headroom"
premise that had since drifted to 400 near-edge / ~566 corner. A constant reasoned
against a value that moved is the argument for **deriving both numbers rather than
authoring them** (see the feasibility corollary; the derivation is queued in
`DEFERRED_REGISTER.md`).

---

## Tombstone policy

Retirement leaves a **one-line epitaph at the code site**; the full lesson lives
**here**. The epitaph says *what* was retired and points home; the charter
carries *why*. This keeps code comments describing present behavior (a standing
rule) while preserving the reasoning that a future reader needs.

Tombstones to date:
- **`CUBE_PART_CAP` → `CUBE_PUSH_CAP`** (TIDY_1 T1b). The name outlived the
  approach-parting mechanism retired at CONTACT_5 P2b; it now caps a presence
  impulse, and is a frame-hitch guard (unreachable at 60 Hz), not a tuning knob.
- **`query_ground_walker_gradient` / `query_ground_walker_walkable`** (TIDY_1
  T1a). The dead cliff-clamp pair — zero callers; the live wall is
  `pawn_ground_resolve`'s `PAWN_STEP_HEIGHT` gate.
- **`CUBE_PUSH_VWINDOW 85` → `CUBE_REACH_CEILING 30`** (TIDY_1 T2b). The single
  vertical window gated `|dy| = fe.pos.y − point.y`, which folded
  `ground_at(xz)` into eligibility — **terrain relief leaked into reach**
  (audit #7, OVERTURNED). The reformulation is two clean tests: a REACH test on
  the cube's *authored* altitude (`orbit_height + bob_amplitude`,
  terrain-independent) folded into radius via `select`, and a planar-only
  cylinder (`INFLUENCE_PLANAR_ONLY` as vwindow — a huge positive, NOT `≤ 0`,
  which would reinstate the spherical trap).

---

## Disclosed softnesses (named, accepted)

These are known one-frame imperfections the springs/fields absorb. They are
documented, not bugs to chase.

1. **Cube same-frame damping (~2.5%).** In the cube drift integrator the push
   impulse is added to `drift_vel`, then the *whole* `drift_vel` is multiplied by
   `exp(−drag·dt)` the SAME frame — so the fresh impulse is attenuated
   `≈ 1 − drag·dt` (~2.5% at the default drag) the frame it lands. A one-frame
   softness on the first frame of a shove; negligible against the accumulating
   drift while the point stays in the column.

2. **One-frame point-velocity leak.** The APPROACH term reads its host's velocity
   one frame stale (the contact gather runs on last-frame's snapped state), and
   the camera-host point has *no* velocity field at all — it falls back to the
   isotropic `BUBBLE_PART_SPEED` floor (`approach_floor`, now the profile's 9th
   column, TIDY_1 T1d). The deferred `config.point_vel_x/z` closes both: it would
   give the point a real per-frame velocity and retire the floor.
