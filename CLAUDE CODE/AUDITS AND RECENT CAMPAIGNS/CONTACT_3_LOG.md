# CONTACT_3 — CAMPAIGN LOG

Campaign: CONTACT_3 (the imposed/voluntary split + the tuning pass;
GPU CAMPAIGN / K 0-3). Handoffs `src/docs/HANDOFFS/GPU CAMPAIGN/k 0-3/`.
Execution: **on master per the standing direction** (the K0 handoff says
so), metadata trailers (Campaign/Stage/Base/Trunk/Gates).

## WHY THIS BATCH (it reframes every gate)

CONTACT_2's impulses are applied AFTER `agent_post_step`, whose order is
drag → speed cap → steering → **position integration** → ground snap. So
an imposed impulse (a) moves nothing the frame it is applied —
integration already ran — and (b) meets the drag + speed cap at the top
of the NEXT frame before integration. A worker fleeing a player at 15
wu/s asks for 18 and receives its voluntary cap (4.5), one frame late.
**THE CAP IS THE ENEMY OF ALL IMPOSED MOTION** — shove and flee alike.
The design error is the handoff's (CONTACT_2's), not the execution's:
"apply after the cap" is, in a loop, "apply before the next cap".

**THE LAW THIS BATCH INSTALLS:** the speed cap governs INTENT, not
imposition. A body chooses how fast to walk; it does not choose how fast
it is displaced by another body.

**GATE PHILOSOPHY (changed, deliberately):** this is a TUNING batch.
Terrain/pixel rest identity still gates. Movable-body behavior is MEANT
to change — do not defend the old feel.

---

## K0 — INDEX + PREFLIGHT

### Base

`2f5f94a` (master trunk) — the CONTACT_2 tip (663f805) + the K/F handoff
commit. Anchor source: the K0/K1/K2 handoff text, re-verified in-tree.

### Baseline gate

glaw1 at base 2f5f94a, before any edit: **G-LAW 1: GREEN**.

### Anchor table (verified against the live trunk)

| # | Anchor | Verdict | Finding (live refs) |
|---|--------|---------|---------------------|
| a | `fn agent_post_step` + call sites | PASS | def @6318; the tail to split (position integration → ground snap → heading-from-velocity, 6360–6376) is cleanly delineated after the C2b steering block. **9** `return agent_post_step(...)` call sites (handoff estimated ~10) — all unchanged by K1a. |
| b | `fn update_other_agents` | PASS | behavior switch → gather (contact/flee/point-source) → eviction; the K1a `agent_settle` call lands after the gather close (@7349), before eviction. |
| c | `fn update_player_agent` | PASS | gather @7119–7196, write-back @7200; K1b snapshots voluntary velocity before the gather, adds the imposed delta to position after. |
| d | `fn behavior_player_controlled` | PASS | integrates inline (`pos += world_vel*speed*dt`) + ground-resolves at its tail → the player has no settle step (K1b applies imposed motion to position directly). |
| e | `fn update_cube` | PASS | cube-vs-pawn `contact_force` @7762, point-source `parting_force` @7783, drift integration @7802, plasticity leak @7839 — all present from CONTACT_2 (K2b clamps the forces; K2c multiplies the leak by the master). |
| f | contact/flee consts | PASS | `CONTACT_SPRING 40` @2169, `CONTACT_IMPULSE_CAP 6` @2170, `CONTACT_CUBE_RADIUS 3` @2172, `NONPLAYER_FLEE_GAIN 0.8` @2185, `BUBBLE_PART_SPEED 4.0` @2186; `personal_radius` 6 uses in world.wgsl. |
| g | `GPUDesignConfig` | PASS + room | after C3a's `point_bubble_radius` the second checker 16-B slot holds variance + bubble = 8 B, leaving **8 B tail pad** → room for K2c's `cube_plasticity` (4 B); **sizeof stays 592** (assert @1456 message reword, not the number). |
| h | `CUBE_TIER_GAINS` + bake | PASS | `plasticity` column present (0.0 all rows); baked per-instance in `cube_write_gpu` (@598–601). K2c retargets the column to per-tier relative `{1.0, 0.8, 1.2, 0.5}` and multiplies by the live `config.cube_plasticity` master. |

All pasted/resolved → K1. No new bindings; one config field (K2c).

---

## K1 — THE IMPOSED/VOLUNTARY SPLIT

### [K1a] — 7894bb6 (split post_step)

`agent_post_step`'s tail (position integration, ground snap, heading-
from-velocity) moved VERBATIM into a new `agent_settle(agent)`;
`agent_post_step` keeps drag + speed cap + the C2b steering block
(velocity shaping ONLY). `update_other_agents` calls `agent = agent_settle(agent)`
AFTER the contact gather → order is behavior (shaped) → gather (imposed)
→ settle (integrate + snap) → evict. The 9 `return agent_post_step(...)`
behavior call sites are unchanged (contract narrowed). `sp2` recomputed
in `agent_settle` from the post-gather velocity. `let t` dropped from
`agent_post_step` (now unused). DISCLOSE: the gather reads a pre-settle
`pos_y` (last frame's snapped height) for the 3D gate — one-frame-stale
y, immaterial to a soft field; named, accepted.

### [K1b] — d9c35a9 (the player's imposed motion)

`behavior_player_controlled` integrates inline, so the player has no
settle step. `update_player_agent` snapshots the voluntary velocity
(`imp_v0`) before the gather and adds the imposed delta
`(agent.vel − imp_v0) * dt` to position after — the handoff's per-term
paired pos-add unified by linearity (Σ deltas · dt), robust against the
vel-lines being textually shared with `update_other_agents`. DISCLOSE:
the player's imposed displacement bypasses this frame's ground resolve,
snapped next frame; small (the pawn's 4× mass), named, accepted.
glaw1 + Dawn witness GREEN.

---

## K2 — THE TUNING PASS

### [K2a] — a2bed80 (the flee shell)

`FLEE_SHELL_FRAC 0.25`: body-to-body flee triggers at
`(personal_radius_sum) * 0.25` = 15 wu (was 60), in both walker kernels'
body-flee gate ONLY (not the spring, flock sense, or point source).
Stops the crowd fleeing itself.

### [K2b] — 0dd5a1a (the cube parting)

`CUBE_PART_RADIUS 8.0` (was 3+20 bubble = 23), `CUBE_PART_CAP 12.0`,
`CUBE_PART_GAIN 1.0` (was CONTACT_SPRING = 40). The cube point-parting
force is now `min(v_ap · CUBE_PART_GAIN · falloff, CUBE_PART_CAP)` with a
planar linear falloff; the cube-vs-pawn repulsion is clamped to
CUBE_PART_CAP (was uncapped, T2c). Parting, not launching.

### [K2c] — dcebd3c (plasticity live dial)

`CUBE_TIER_GAINS.plasticity` → per-tier relative character
`{1.0, 0.8, 1.2, 0.5}`; `GPUDesignConfig`+`DesignConfig` gain
`cube_plasticity` (the live master, fills the checker tail pad — sizeof
stays 592, assert message reworded); boot-pinned from
`Idle::CUBE_PLASTICITY_DEFAULT = 0.6` via a new `set_cube_plasticity`.
The leak multiplies both: `lam = fe.plasticity · config.cube_plasticity`
(the three C1b refinements kept). DISCLOSE: default 0.6 is a DELIBERATE
change — shoved cubes stay shoved; cube rest identity dropped (terrain/
pixel rest identity still gates — this batch's philosophy).

---

## K3 — CLOSEOUT

Instruments → `_post_c3`, diffed against the K0 base `2f5f94a`:
- **cc6:** zero substantive layout diffs; flags EMPTY.
- **cc7:** 96 → 96 declarations (+0) — `cube_plasticity` is a struct
  member, not a `@binding` declaration.
- **cc7 mirror:** zero orphans both directions.
- **cc4:** all four agent/cube kernel closures byte-identical to the base
  (no new bindings).
- **Dawn witness** (`_post_c3`): ALL FAMILIES GREEN, zero module messages.
- **Pipeline-timing delta (the K1 dividend):** `update_other_agents`
  pipeline creation dropped **1587 ms → 799 ms** (~2×) on SwiftShader/Tint
  — the ground chain moved from ~9 inlined sites to one. Module compile
  165 ms → 143 ms. (Indicative of relative compile cost; FXC's word is
  Jean's Windows build.)

Encoding sweep: world.wgsl / state.hpp / agents.hpp / cube_behaviors.hpp
all no-BOM, LF-only; FXC banner byte-untouched. Final glaw1 GREEN.
