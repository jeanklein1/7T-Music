# CONTACT_2 — CAMPAIGN LOG

Campaign: CONTACT_2 (the composed-spheres completion — geometry law,
steering, the social split; GPU CAMPAIGN / C 0-4). Handoffs
`src/docs/HANDOFFS/GPU CAMPAIGN/C 0-4/`.

**Trunk / execution note (deviation from handoff default):** the C0
handoff names `Branch: CONTACT_2`, but Jean directed execution **directly
on the master trunk** (2026-07-23: "You can work on the current master
trunk. Just make sure you detail metadata so we can track history
easily."). So the `[C0]`–`[C4]` commits land on `master` with rich
metadata trailers (Campaign/Stage/Base/Trunk/Gates) rather than on a
side-branch. Push at C4 per the handoff cadence.

---

## C0 — INDEX + PREFLIGHT

### Base

`3252a6d` (master trunk) — carries the CLOSURE_PAWN 5-commit feature
series (pawn figure registry / skin_id / figure geometry) + `Handoffs
C 0-4`. Anchor source: the C0/C1/C2/C3 handoff text, re-verified in-tree
(the tree is the law — CLOSURE_PAWN shifted it under the handoff).

### Baseline gate

glaw1 at base 3252a6d, before any edit: **G-LAW 1: GREEN**.

### Anchor table (verified against the live trunk)

| # | Anchor | Verdict | Finding (live refs) |
|---|--------|---------|---------------------|
| a | `CONTACT_SPRING` gathers | PASS (line-shifted) | def @2161 + 5 uses (7069/7086/7154/7171/7598). The 3 T2 blocks are byte-identical; `pos_y`/`vel_y` exist ⇒ C1a 3D gate feasible. C1a applies to the sphere sub-loops too (handoff: "agent loop + sphere loop"). |
| b | `fn update_cube` | PASS | 1 def @7462; plasticity leak inserts after `fe.drift = fe.drift + fe.drift_vel * dt;` (@7605), true names `fe.anchor`/`fe.drift`. Guard: CLOSURE_PAWN's separate `PawnFigure.drift` (HSV color) is NOT the cube's drift. |
| c | flock sensing radius | RESOLVABLE w/ correction | No "sensing-radius const" — it's the uniform field `agent_behaviors[8].neighbor_radius` (literal 30.0 in agents.hpp:150), read @6869. C3a seeds `personal_radius = 30.0` and re-points **that line** (field shared by biased_walk/pursuit/flee — leave those). `PAWN_FORCEFIELD_RADIUS` is two consts (STATIONARY 6.0 @2154 + MOVING 2.0 @2155). |
| d | `POINT_BUBBLE_RADIUS` | PASS census / doctrine note | def @7023 + exactly 1 code reader @6495 (portal vertical gate) + 3 comment mentions. C3a appends `config.point_bubble_radius` (crosses WGSL/host boundary) and boot-pins from `contracts/point.hpp` (point.hpp stays source-of-truth); the REST-mirror comment must state the value now flows via boot upload. |
| e | `fn point_pos` | PASS | 1 def @5548, param-free `-> vec3<f32>`, host-switched; already called from `update_other_agents` @7185 and `update_cube` @7466 — lacks no binding. Distinct from render `render_point_pos` @5573. |
| f | `GPUDesignConfig` | PASS + correction | append `point_bubble_radius` @offset 580 into the existing checker tail pad. **sizeof does NOT rise — stays 592** (584 rounds to 592). Growth-law "bump the witness" is a message reword, not a number bump; assert @1450 stays `==592`; WGSL `DesignConfig` mirror gains the 4th scalar. |
| g | camera velocity | **FALLBACK (decisive)** | No velocity field in `GPUCameraState` (@742-751, sizeof 48). C3b uses the documented fallback `v_ap := BUBBLE_PART_SPEED` (synthesized constant, NOT a struct read). **C4 must report this.** The scalar `Config.point_fly_speed` @507 is not a camera velocity vector. |
| h | cube tier table | RESOLVABLE w/ correction | Named `CUBE_POPULATIONS` is per-MOOD (6 rows, no GPU path) — wrong target. Real per-tier home = `CUBE_TIER_GAINS` (4 rows). No GPU cube-tier array + "no new bindings" ⇒ plasticity bakes **per-instance** into `FloatingEntityState`/`GPUFloatingEntityState` free pad `_pad0` (world.wgsl:1013 / state.hpp:796); 208 B size pin untouched; `update_cube` reads `fe.plasticity`. |

Verdict: **RESOLVABLE_WITH_NOTES → ready for C1** (all anchors resolve;
the "notes" are the handoff's own hedges — "or its true name", "else per
the upload shape", "agent loop + sphere loop" — landing on concrete
sites of the CLOSURE_PAWN-shifted tree).

### Base-shift (CLOSURE_PAWN landed under the handoff — risk: LOW)

- `AgentState._pad0` @offset 92 is **gone** → repurposed to `skin_id`
  (carries live figure data, preserved through possession). This batch's
  plasticity rides `FloatingEntityState`, not `AgentState` — no direct
  bite, but no AgentState pad remains to squat.
- `@binding(112)` is now `agent_figure_profiles` (uniform, vertex-only);
  next free is 113. This batch adds **no bindings** — moot, noted.
- All world.wgsl line numbers shifted down ~261; the T2 kernels /
  `AgentTierParams` / `GPUAgentTierDef` / `GPUCameraState` / cube row are
  semantically byte-untouched. Every C edit re-anchors by symbol.

### Reconciliations carried into execution

1. Re-anchor by symbol, never by handoff-era line number.
2. C1a: the sphere sub-loops get the 3D gate too (handoff line 46).
3. C1b: target `CUBE_TIER_GAINS` + `FloatingEntityState._pad0`
   (`fe.plasticity`), not `CUBE_POPULATIONS`/`tier.plasticity`.
4. C3a: re-point the flock uniform field read @6869; `personal_radius`
   seed = 30.0; carry the `config.point_bubble_radius` WGSL+host append
   and the boot pin; `GPUDesignConfig` sizeof stays 592 (message reword).
5. C3b: camera-host flee source uses the `BUBBLE_PART_SPEED` FALLBACK
   (no camera velocity field) — reported at C4.
