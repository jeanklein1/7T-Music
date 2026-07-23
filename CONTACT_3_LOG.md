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
