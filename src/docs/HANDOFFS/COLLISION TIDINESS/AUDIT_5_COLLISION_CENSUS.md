# AUDIT-5 — THE COLLISION CENSUS

Campaign: AUDIT-5. **Audit only — no edits.** Every finding lands in the
disposition table [A5-9], which is the input to the dead-code batch that
follows tonight.

Successor to AUDIT-4 (`AUDIT_REPORT.md`) and to CONTACT_1→5. The contact
quartet plus the point law closed the BODY↔BODY question. This batch asks what
"collision" still means everywhere else, and prices the tidiness.

---

## THE FRAME (the ruling this batch is built on)

"Collision" in this program is **three laws**, and only one is unified:

1. **BODY ↔ BODY** — `influence_response` + `InfluenceProfile`. Nine call
   sites, one body. Done at CONTACT_5.
2. **BODY ↔ WORLD** — `pawn_ground_resolve`, `agent_settle`'s snap, the cube's
   `CUBE_TERRAIN_CLEARANCE` clamp, the world-bound clamp, the pawn-host camera
   clamps vs `TERRAIN RULE = NONE` in camera-host, the portal's vertical gate.
   ONE query API (`manifold_position` + `POLICY_*`) and several unrelated
   RESPONSE shapes. **This is where the untidiness now lives.**
3. **AGGREGATE FIELD** — the orb flock (sep/ali/coh), its own radii, gains,
   signs, per-rule drag multipliers.

Naming the three is most of the win. Every item below hangs off it.

**The discipline this batch enforces:** a law protects exactly the surface its
invariants name. CONTACT_5 had the strongest invariant in the project's history
and still shipped rows nobody checked, because the check was written for radii
and never extended. Tidiness here is not housekeeping — it is the difference
between a system that can still be reasoned about at the deadline and one that
can only be poked.

---

## P0 — BASE + PREFLIGHT

- **Pin the base.** Report `git rev-parse HEAD` on `master` after the CONTACT_5
  merge. Do NOT assume `be747e8` — that was C5's base, not this one. State the
  hash at the top of the report; every recipe is relative to it.
- Baseline `glaw1`. Report GREEN/RED. (No edits this batch, so this is a
  witness reading, not a gate.)
- **Verify against the LIVE TREE, not against `BATCH_REPORT_C5.md`.** The
  report is a witness statement. Where report and tree disagree, the tree wins
  and the disagreement is a finding.
- Every count ships with its recipe (`audit/a5_*.py`, outputs beside them —
  the AUDIT-4 pattern).
- Every quotation verbatim, with file:line.
- Branch: transient `claude/*` review branch per the git law; merged and deleted
  same day. No pushes to master.

---

## [A5-1] THE TAXONOMY — the census of altered motion

**WHAT.** Enumerate EVERY site in the live cartridge where one thing's position
or velocity is altered because of another thing. Not just the contact chain:
the ground resolve, the clamps, the flock, the aura, the ribbon, the gallery,
anything.

**COLUMNS.** `site · file:line · law (1/2/3/none) · response shape · gate shape ·
pairs evaluated per frame`.

The last column is the **optimization census**. When a hard limit arrives, the
table already exists and nobody re-derives it under pressure. Give the formula
AND the worst case (e.g. `update_other_agents`: 32 threads × 32 slots = 1024
pair evaluations/frame, of which N active).

**FLAG** any site that fits none of the three laws. Those are the real finds.

**DELIVERABLE.** One table. `[anchor-1]`

---

## [A5-2] THE POINT SWEEP, EXTENDED TO THE BEHAVIOR LAYER

**WHAT.** CONTACT_5 P2c swept every *influence* read of
`config.possessed_slot` and concluded nothing was left to convert. It never
touched the *behavior* reads.

Census every remaining read of `config.possessed_slot` and of
`agent_state[config.possessed_slot]` in the tree. Classify each:

- **BODY emanation** (legitimate — the pawn as a body): agent↔agent pairs, the
  `PAWN_CONTACT_MASS_MULT` weight.
- **POINT term** (must route through `point_pos()`): anything expressing the
  player's *presence*.

**PRE-REGISTERED EXPECTATION — confirm or refute, do not assume.**
`behavior_pursuit` and `behavior_flee` each open with
`let player = agent_state[config.possessed_slot];` and gate on
`b.neighbor_radius`. In camera-host the pawn stands idle wherever it was
abandoned while the population lives under the camera by point-centered
eviction — so the detect/alarm gate should never fire and both behaviors should
degrade silently to idle wander. **Prove it with the gate arithmetic**: pawn
position at abandonment vs `AGENT_EVICTION_RADIUS`, vs `b.neighbor_radius`.
If it fires, say so — the refutation is as valuable as the confirmation.

Check every other behavior for the same shape.

**DELIVERABLE.** Reader table + verdict per site + the one-line conversion for
each POINT-term site. `[anchor-2]`

---

## [A5-3] THE FEASIBILITY LEDGER, EXTENDED TO EVERY COLUMN

**WHAT.** The `GATE-FEASIBILITY RULE` banner in `world.wgsl` covers RADII: *a
3D gate against a body at altitude H can only fire if the radius exceeds H.*
It is mechanical, it is falsifiable, and it caught every C2/C3 defect
retroactively. It was never extended past radii.

Extend it to **every profile column**. For each of the nine rows compute the
maximum attainable magnitude from the AUTHORED ranges (tier tables, per-instance
bands, consts) and compare against that row's `cap`.

**PRE-REGISTERED ARITHMETIC — confirm or refute each.**

| row | max magnitude at dt = 1/60 | cap | expectation |
|---|---|---|---|
| 1 / 4 agent contact | `contact_r` max pair 2.0+2.0 = 4.0 → `4.0·40/60 = 2.67`, then ×`yield_share` ≤ 1 | 6 | unreachable |
| 3 / 8 sphere push | `influence_radius` μ 6–8 → `8·40/60 = 5.33` | 6 | unreachable at the means; engages at `r ≥ 9` (σ tail) |
| 6 agent↔sphere | `contact_r + body_radius` ≈ 2.0+1.5 → `3.5·40/60 = 2.33` | 6 | unreachable |
| 9 cube push | `7·25/60 = 2.92` (fall = 1 at centre) | 12 | unreachable by ~4× |
| 2 / 5 / 7 flees | approach term, uncapped | `1e9` | n/a |

**THE LIKELY VERDICT, and the reason this item matters.** If every presence-row
cap is unreachable at 60 Hz, the caps are not tuning knobs — they are
**frame-hitch guards**. At `dt = 0.1 s` row 1 gives `4.0·40·0.1 = 16 > 6` and
row 9 gives `7·25·0.1 = 17.5 > 12`; both engage. So compute and report a **"cap
engagement dt" column**: the dt at which each cap first bites. If that is what
the caps are, the ledger must SAY so — a constant presented as a tuning knob
that is really a hitch guard is exactly the drift the charter forbids.

**THEN.** Propose the **static-assert form**. This check belongs to the
compiler, not to a comment above the constants. C++-side over the authored
tables where the values live; state honestly what cannot be asserted C++-side
(per-instance sampled values) and what the WGSL-side alternative is, if any.

**DELIVERABLE.** Extended ledger table + the engagement-dt column + the assert
proposal with its exact placement. `[anchor-3]`

---

## [A5-4] THE TWEAK MAP — where every number physically lives

**WHAT.** For all nine rows × eight `InfluenceProfile` columns, plus the
`approach_floor` parameter: name the physical home and the mechanism of every
number.

**MECHANISM CLASSES.** compile-time WGSL const · C++ `Idle::` default → config
upload · per-instance baked GPU field (written where?) · tier table (which
file?) · computed inline at the call site.

**PRE-REGISTERED EXPECTATION.** The BATCH_REPORT claims "every knob in one
place, in one file." Expect that to be false for the VALUES even though it is
true for the TABLE: `flee_gain_player` in `bodies/agents.hpp`,
`point_bubble_radius` uploaded from `contracts/point.hpp`, `cube_plasticity`
defaulted at `Idle::CUBE_PLASTICITY_DEFAULT` in `realization/state.hpp`,
`influence_radius` written per-instance by `bodies/spheres.hpp` from the sphere
tier table, the rest as `world.wgsl` consts. Count the files. Report the number.

This table is simultaneously the tweakability answer and the master-control-panel
prerequisite. `[anchor-4]`

---

## [A5-5] THE PROFILE BLOCK — proposal + price, NOT a mandate

**WHAT.** The profile table exists as prose in a markdown report; the code has
nine constructors scattered across four kernels. Proposal: collapse them into ONE
contiguous authored section of named `fn row_*(...) -> InfluenceProfile` bodies,
called from the sites, with dynamic columns (`fe.influence_radius`,
`m_o/(m_s+m_o)`, tier gains) staying parameters.

**HARD CONSTRAINT.** No array, no runtime indexing. The `world.wgsl` FXC banner
(`SEAM[world.wgsl:fxc-constraints]`) forbids new runtime branching in this chain
and names texture/embedded-array stamps as a hang class. A `const` array of
profiles indexed at runtime is exactly that shape. Functions returning
constructed structs are not.

**PRICE IT.** Build both variants on the review branch, run the Dawn witness,
report per-kernel pipeline-creation deltas and module compile time. State the
±30% SwiftShader noise caveat. **Then recommend** — including "don't" if the
price is real. A tidiness change that costs FXC time is not tidiness.

**ALSO PRICE.** Promoting `approach_floor` to the ninth `InfluenceProfile`
column. It is constructed at the call site exactly like the other dynamic
columns, so nothing structural blocks it; it removes the last knob outside the
table and drops a parameter from the signature.

**DELIVERABLE.** Two proposals, two prices, one recommendation each. `[anchor-5]`

---

## [A5-6] THE GROUND-RESOLVE CENSUS — law 2, documented not fixed

**WHAT.** Table every BODY↔WORLD site: `site · policy (POLICY_*) · response
shape · behavior when blocked · who owns the response`.

Known members to start from, not an exhaustive list — find the rest:
`pawn_ground_resolve` (step-height gate, axis-aligned slides, revert),
`agent_settle`'s snap, the cube's `CUBE_TERRAIN_CLEARANCE` drift.y clamp with
its `drift_vel.y` kill, the world-bound clamp, the camera's pawn-host clamps vs
the camera-host early return (`TERRAIN RULE = NONE`), the portal vertical gate,
the C2b gradient steering (`STEER_*` — a whisper, not a wall: classify it).

**THE QUESTION TO ANSWER.** Is there ONE response vocabulary here (block /
slide / clamp / snap / steer), or N unrelated ones? If one, name it. If N, say
how many and which are genuinely distinct decisions versus historical accident.

This is a SEED for the next campaign, not this batch's work. Do not propose the
unification — just make it legible. `[anchor-6]`

---

## [A5-7] THE ACCUMULATION LAW — where the response lands

**WHAT.** `influence_response` returns a planar velocity delta in wu/s. Four
different things happen to it in the tree. Verify each verbatim and rule
whether it is policy or accident.

**PRE-REGISTERED, verify each:**

1. **Non-player agent** — `agent_post_step` (drag, cap, steering) runs INSIDE
   the behavior, BEFORE the kernel's gather; the impulse is added after, so it
   **escapes that frame's drag** and persists into the next frame.
2. **Player** — same add, but `behavior_player_controlled` AUTHORS velocity
   each frame (`agent.vel_x = world_vel.x * speed`, or zeroes it), so the
   imposed part **does not accumulate**; only the K1b inline pos-add survives.
3. **Cube** — `drift_vel += push_impulse` then `drift_vel *= exp(-drag·dt)`,
   so it is **damped the same frame it lands**.
4. **Camera** — no velocity state; `camera.pos += r · dt` directly.

**VERDICT REQUIRED.** Four deliberate policies, or one policy and three
accidents? Quantify (3) — the same-frame damping at authored cube drag.

**ALSO VERIFY — the one-frame leak.** Dispatch order is
`ribbon → player → others → camera → sphere → cube → vp`. The player kernel
writes back velocity INCLUDING the gather's impulses; row 7 then reads
`agent_state[config.possessed_slot].vel_*` as the POINT's velocity. Expectation:
a sphere shoving the pawn is read by the crowd as the point lunging, for one
frame. Confirm, quantify, and rule: percept or artifact? `[anchor-7]`

---

## [A5-8] THE BOUNDARY STATEMENTS

**WHAT.** One sentence per law: what it governs, what it does not, and why.
These go in the charter, not in prose comments. Every future unification ships
one — that asymmetry (the point-centering ledger has one; the behavior layer
never did) is exactly where the A5-2 defect landed.

**TWO RULINGS TO PREPARE (recommend; Jean stamps).**

- **The orb flock.** Separation IS a pair-presence response wearing flock
  clothes (`1/d²`-weighted, radius-gated). Alignment and cohesion are not —
  they are aggregate-field terms. Merge separation onto the profile table, or
  fence the whole flock as law 3? Price both. Note the constraint: it reads
  `orb_state_prev` to avoid in-pass feedback, and it is O(N²) over
  `MAX_ORBS = 256`, so its pair count belongs in [A5-1].
- **`neighbor_radius` vs `personal_radius`.** Two radii for one percept, read
  by different layers. **This one matters tonight.** The deferred register
  calls `neighbor_radius` dead; the tree appears to read it in
  `behavior_pursuit`, `behavior_flee`, and the home-pull/alarm path. Produce
  the **precise reader census before anything is deleted.** A wrong delete here
  is a silent behavior loss, not a compile error. `[anchor-8]`

---

## [A5-9] THE DISPOSITION TABLE — the deliverable that feeds tonight

Every finding from every item, one row:
`finding · verdict · size · risk · blocked-by`.

**VERDICTS.** `DELETE` · `RENAME` · `PROMOTE` · `RULE-NEEDED` · `KEEP-WITH-REASON`.

Seeds already known (verify, then dispose):

- `CUBE_PART_CAP` names a mechanism retired at P2b; it now caps a presence
  impulse → RENAME `CUBE_PUSH_CAP` (or whatever [A5-3] proves it to be).
- `approach_floor` is the ninth column sitting outside the struct → PROMOTE.
- The tombstone comments (`CONTACT_SPHERE_RADIUS`, `CONTACT_CUBE_RADIUS`,
  `CUBE_PART_RADIUS/GAIN`) — KEEP or DELETE? They are the record of the
  gate-feasibility lesson. Recommend, with reasoning.
- `CUBE_PUSH_VWINDOW 85` is an accident of the authored MEAN (`75 + 2 + 8`),
  while σ reaches 45. `fe.orbit_height` (offset 60) and `fe.bob_amplitude`
  (offset 104) are per-instance fields already resident, and `bob_y` and `home`
  are locals in scope at the profile constructor — so the per-instance form is
  ONE expression, not a mechanism. Verify that scope claim, then present both
  dispositions: per-instance vwindow (every cube shoveable), or rule the
  ceiling deliberate (very high cubes are out of reach, like real balloons).
  Jean stamps; the current 85 is neither.
- The `CONTACT_SHELL_DEBUG` instrument: does it draw every shell the table now
  has? It draws the bubble, `CUBE_PUSH_RADIUS`, and one patch. The sphere shell
  (`fe.influence_radius`) is per-instance and absent. Propose or decline.

`[anchor-9]`

---

## STANDING CONSTRAINTS

- **No edits.** Proposals live in [A5-9]. The only exception is the [A5-5]
  price build, which happens on the review branch and is not merged.
- **The tree is the truth.** Reports are witness statements.
- **Counts ship with their recipes.** `audit/a5_*.py`, outputs beside them.
- **The FXC banner is law.** `SEAM[world.wgsl:fxc-constraints]` — do not
  propose anything it forbids, and quote it when a proposal brushes it.
- **Encoding.** LF-only, no BOM, tree-wide. If a new file is created, it obeys.
- **YAGNI.** Nothing is reserved. A finding is DELETE or it is KEEP-WITH-REASON.
- **Comments describe present behavior only.** Any comment found describing a
  retired mechanism is a finding.

## DELIVERABLES

1. `AUDIT_5_REPORT.md` — one section per `[A5-N]`, verdict first, then recipe,
   then evidence. The AUDIT-4 shape.
2. `audit/a5_*.py` + outputs.
3. The disposition table [A5-9], standalone enough to drive tonight's dead-code
   batch without re-reading the whole report.

## LAND ORDER

`[A5-1]` → `[A5-2]` → `[A5-3]` → `[A5-4]` → `[A5-6]` → `[A5-7]` → `[A5-8]` →
`[A5-5]` (the price build last; it is the only one that compiles) → `[A5-9]`.

`[A5-1]` first because everything else indexes into its table.
