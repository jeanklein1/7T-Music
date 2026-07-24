# CONTACT_3 — BATCH REPORT

Campaign: CONTACT_3 (the imposed/voluntary split + the tuning pass;
GPU CAMPAIGN / K 0-3). This is the batch's witness stand (K3).

## Base + final

- Base: `2f5f94a` (master trunk) — the CONTACT_2 tip (663f805) + the K/F
  handoff commit. Executed **directly on master** per the standing
  direction; commits carry `Campaign/Stage/Base/Trunk/Gates` trailers.
- Commit list:

| Commit | Hash | Intent |
|--------|------|--------|
| [K0]  | 9985011 | Preflight: the imposed/voluntary diagnosis; anchors a–h; baseline glaw1 GREEN |
| [K1a] | 7894bb6 | Split `agent_post_step` → `agent_settle` (integrate/snap/heading after the gather) |
| [K1b] | d9c35a9 | The player's imposed motion lands on position this frame |
| [K2a] | a2bed80 | The flee shell: `FLEE_SHELL_FRAC` (stop the crowd fleeing itself) |
| [K2b] | 0dd5a1a | The cube parting: radius 8 + cap 12 + falloff (stop the fling) |
| [K2c] | dcebd3c | Plasticity live dial: per-tier character × `config.cube_plasticity` |

## The diagnosis restated (why the batch exists)

CONTACT_2's contact/flee impulses were applied AFTER `agent_post_step`,
whose order was drag → cap → steering → **integrate** → snap. So an
imposed impulse (a) moved nothing the frame it was applied — integration
had already run — and (b) met the drag + speed cap at the top of the
next frame. A worker fleeing at 15 wu/s asked for 18 and received its
voluntary cap (4.5), one frame late. **The cap ate every imposed
motion.** The design error was CONTACT_2's ("apply after the cap" is, in
a loop, "apply before the next cap"), and K corrects it: **the speed cap
governs INTENT, not imposition.**

## The split (K1)

`agent_post_step` was cleaved into two: it keeps drag + cap + steering
(velocity shaping), and a new `agent_settle` holds integration + ground
snap + heading. The kernel calls `agent_settle` AFTER the contact gather,
so imposed motion lands the same frame and is never capped. The 9
behavior call sites are unchanged. The player (which integrates inline in
`behavior_player_controlled`) instead snapshots its voluntary velocity
before the gather and adds the imposed delta to position after (K1b).

**Worked example (the new law):** worker cap 4.5, player approaching at
15 wu/s, gain 1.2, dt 1/60. Gather: v_ap = 15 → demand 18; vel·dir = 0 →
deficit 18 → vel += 18·esc. Settle: displacement = 18/60 = 0.30 wu this
frame. Next frame: drag + cap trim vel to 4.5 → gather sees vel·dir = 4.5
→ deficit 13.5 → tops back to 18. Sustained escape, self-correcting, no
runaway momentum. Before K1: 4.5 wu/s, one frame late.

**The compile dividend:** moving the ground-resolve chain from ~9 inlined
sites (one per behavior) to ONE dropped `update_other_agents` pipeline
creation from **1587 ms → 799 ms** (~2×, SwiftShader/Tint); module
compile 165 ms → 143 ms. Indicative of relative compile cost; FXC's word
is the Windows build.

## The three tuning changes

- **K2a — the flee shell.** `personal_radius = 30` on every tier made
  body-to-body flee trigger at the SUM (60 wu), so a 32-agent cluster
  fled itself into permanent nervousness. `FLEE_SHELL_FRAC = 0.25` makes
  the flee expression a fraction of the sensing one: 30+30 → **15 wu**
  trigger (body-flee gate only). Worked: a walked-through cluster now
  parts locally instead of milling.
- **K2b — the cube parting.** The C3b parting was radius 23 (self +
  bubble) with force ∝ the player's full speed × 40, uncapped, into a
  spring integrator — it flung. Now radius **8**, force
  `min(v_ap · GAIN · falloff, 12)` with a planar linear falloff; the
  cube-vs-pawn repulsion clamped to 12 too. Worked: player at 15 wu/s,
  5 wu from a cube — before radius 23 / force ∝ 600 / uncapped; after
  radius 8 / force ≤ 12 / falloff-shaped. Parting, not launching.
- **K2c — plasticity live dial.** Cubes returned because λ = 0 on every
  row (C1b bit-neutrality) and λ bakes at spawn. K2c splits it into
  per-tier CHARACTER (`{1.0, 0.8, 1.2, 0.5}`, respawn to change) × a live
  global MASTER (`config.cube_plasticity`, default **0.6**). The leak
  multiplies both. **DELIBERATE behavior change:** shoved cubes now stay
  shoved; cube rest identity is dropped (terrain/pixel rest identity
  still gates — this batch's declared philosophy).

## Recount vs expectations (`_post_c3`, vs the K0 base `2f5f94a`)

| Check | Expectation | Result | Verdict |
|---|---|---|---|
| cc6 flags / layouts | EMPTY / no deltas | EMPTY / zero substantive diffs | **MATCH** |
| cc7 declarations | +0 | 96 → 96 | **MATCH** |
| cc7 mirror | zero orphans | zero both directions | **MATCH** |
| cc4 closures | unchanged | all four agent/cube kernels byte-identical | **MATCH** |
| Dawn witness | ALL GREEN | 19 families / 30 EPs GREEN, zero module messages | **MATCH** |
| Final glaw1 | GREEN | GREEN (the 592 assert passes — `cube_plasticity` fits the tail pad) | **MATCH** |

"No new bindings; one config field" — confirmed; `cube_plasticity` is a
`GPUDesignConfig`/`DesignConfig` struct member (config buffer is
sizeof-driven, 592 unchanged), not a `@binding` declaration.

## Adversarial verification (K1 split + rest identity + K1b)

A 3-lens panel (split data-flow, rest-identity, player snapshot-delta)
independently checked the batch and reached **unanimous SHIP, zero
confirmed defects**: terrain/pixel rest identity PRESERVED, the split
correct (agent_settle called exactly once, no double/missing
integration, eviction sees the settled position, `sp2` recomputed
post-gather), and the K1b snapshot-delta proven algebraically equal to
per-term paired pos-adds (linearity). One low-severity **non-defect**
noted: an agent with behavior 0 / no-op in a *non-possessed* slot (a
state the code documents as "should never appear" — the possessed slot
is skipped, every legitimate active agent carries behavior 1–9) now gets
integrated by `agent_settle` where pre-K1 it froze; harmless for every
reachable state (idle behavior-1–9 agents settle `pos += 0` and re-snap,
which correctly tracks deforming terrain). Not fixed — bug-only path.

## Deviations

- **D1 — on-master execution** (standing direction); metadata trailers.
- **D2 — K1b snapshot-delta** instead of literal per-term paired
  pos-adds: mathematically identical (Σ deltas·dt by linearity), robust
  against the vel-lines being textually shared with `update_other_agents`
  (where `agent_settle` integrates them instead).
- **D3 — K2b falloff authored**: the handoff said "keep the existing
  falloff shape," but the C3b parting had none — a planar linear falloff
  (1 at center → 0 at the radius edge) was authored to match the
  "falloff-shaped" intent.
- **D4 — 9 behavior call sites**, not the handoff's estimated ~10.

## Encoding sweep

world.wgsl / state.hpp / agents.hpp / cube_behaviors.hpp: **no BOM,
LF-only.** The FXC constraints banner is byte-untouched.

## THE TUNING TABLE (every knob, one place)

| Knob | Value | Where |
|---|---|---|
| `FLEE_SHELL_FRAC` | 0.25 | world.wgsl:2192 |
| `personal_radius` (×4 tiers) | 30.0 all | agents.hpp:170 (`AGENT_TIER_GAINS` @174) |
| `flee_gain_player` (×4) | 1.2 / 1.4 / 1.0 / 1.1 | agents.hpp:171 (rows @174) |
| `NONPLAYER_FLEE_GAIN` | 0.8 | world.wgsl:2186 |
| `CONTACT_SPRING` / `CONTACT_IMPULSE_CAP` | 40.0 / 6.0 | world.wgsl:2170 / 2171 |
| `contact_radius` / `contact_mass` (×4) | 1.6/1.4/2.0/1.8 · 1.0/0.8/1.5/1.2 | agents.hpp:168–169 (rows @174) |
| `PAWN_CONTACT_MASS_MULT` | 4.0 | world.wgsl:2174 |
| `CUBE_PART_RADIUS` / `_CAP` / `_GAIN` | 8.0 / 12.0 / 1.0 | world.wgsl:2197 / 2198 / 2199 |
| `CUBE_PLASTICITY_DEFAULT` (live master) | 0.6 | state.hpp:306 (`Idle`) |
| `CUBE_TIER_GAINS.plasticity` (×4 character) | 1.0 / 0.8 / 1.2 / 0.5 | cube_behaviors.hpp:92 (rows) |
| `STEER_LOOKAHEAD_WU` / `_GAIN` / `_GRAD_LO` / `_HI` | 4.0 / 3.0 / 0.7 / 1.4 | world.wgsl:2177 / 2178 / 2182 / 2183 |
| `config.pawn_speed` (the flee's input) | `Idle::PAWN_SPEED` 15.0 default | state.hpp:303 (runtime `config.pawn_speed`) |

## JEAN'S GATE LIST (Windows)

- [ ] **Build + boot.** FXC watch: `agent_settle` is a MOVE of existing
      code, not new code; the gathers are unchanged in shape. Expect the
      `update_other_agents` compile cost to DROP (the ground chain is now
      one site, not nine).
- [ ] **TERRAIN REST IDENTITY (still a gate):** a still world's terrain
      is unchanged — the split touches no terrain path.
- [ ] **THE MATADOR, retried:** sprint at a sentinel. It should now clear
      you at YOUR speed and settle back to its gait. This is the gate K1
      exists for. If it still fails, the sign and the seam are both
      verified, so the next suspect is `config.pawn_speed` (15) vs the
      agents' `speed_cap` being far apart — report the two numbers.
- [ ] **THE CROWD:** walk (don't sprint) through a cluster. Agents should
      hold formation and part locally — no permanent nervousness. If they
      still mill, drop `FLEE_SHELL_FRAC` further.
- [ ] **THE CUBES, three things:** shoulder one — it parts (not flings)
      and STAYS moved; walk past at range — nothing until ~8 wu; sweep
      through the drone show and rearrange it. Tune
      `CUBE_PLASTICITY_DEFAULT` to taste (0 = old elastic behavior).
- [ ] The whisper + the geometry gates from C2, unchanged.

## DEFERRED REGISTER (carried + new)

- **figure-scaled contact radius** (CLOSURE_PAWN interaction: a Colossal
  and a Spire share a tier's body radius) — the clean fix is widening
  binding 112's visibility to COMPUTE.
- **CPU-authored point velocity** (`config.point_vel_x/z` from the P5
  harvest) — retires the camera-host `BUBBLE_PART_SPEED` fallback and
  restores "a still camera parts no one".
- **the player's gather above the ground-resolve tail** — if a shove sink
  is ever visible (the K1b follow-on, not this cut).
- **AgentState pad debt (96→112)** — blocks per-agent individuality.
- living-plateau steering · per-slot floater radii · the bubble's
  coupling wire · the ribbon floor · emissaries.

## Seeds

CONTACT_3 makes imposed motion real (the cap governs intent, not
imposition) and hands Jean the tuning table. The contact trilogy is
closed. What remains queued: the F campaign (also just pushed — sign +
mask + topology split), the coupling campaign, and the deferred register
above.
