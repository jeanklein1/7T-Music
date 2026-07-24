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

---

## C1 — GEOMETRY LAW + PLASTICITY

### [C1a] — fa690c9 (3D gates, planar response)

All 5 landed pair-tests (both walker gathers' agent loop + sphere loop,
+ cube-vs-pawn) gained a 3D distance GATE with a planar RESPONSE:
`dy = self.pos_y - other.pos_y` folds into `d2 = dx*dx+dy*dy+dz*dz`;
overlap `(r-d)` keeps the true 3D `d = sqrt(d2)`, direction uses a new
planar `d_pl = sqrt(max(dx*dx+dz*dz, 0.0001))`. Cube: `cdy`/`cd_pl` the
same. Influence is vertically bounded (a pawn atop a plateau no longer
shoves agents below; airborne cubes ignore ground bodies outside their
sphere). Ground-level pairs (dy≈0) unchanged. 8 edits count-verified;
sphere loops included per handoff line 46. glaw1 GREEN.

### [C1b] — 7210fcc (cube plasticity; λ=0 bit-neutral)

`CUBE_TIER_GAINS` gained a `plasticity` column (λ, 0.0 all rows). ANCHOR-H
reconciliation: the handoff's `CUBE_POPULATIONS` is per-MOOD (6 rows, no
GPU path) — the real per-tier home is `CUBE_TIER_GAINS` (4 rows). No GPU
cube-tier array + "no new bindings" ⇒ plasticity bakes per-instance at
spawn (`cube_write_gpu`) into the `FloatingEntityState._pad0` slot →
`plasticity` (u32→f32, size stays 208). `update_cube` leaks drift→anchor.
THREE refinements make the handoff's stated continuity law ("moves no
pixels") TRUE on this code, all bit-neutral at λ=0: (1) placed AFTER the
compose `fe.pos = home + fe.drift` (home is a local from the pre-leak
anchor); (2) XZ-only (home.y is terrain-relative, not anchor.y); (3)
anchor mode only (kite mode's home tracks the point). The handoff's
"zone..." leak placeholder resolved to λ*dt (no zone input for cubes).
glaw1 GREEN.

---

## C2 — GRADIENT STEERING

### [C2a] — 61b849a (the gradient reader + consts)

`sample_terrain_grad_at` mirrors `sample_terrain_y_at`'s patch_grid/uv
math, returning the baked `.yz` slope (out-of-window ⇒ vec2(0)). Adds NO
binding — patch_grid(152)/photo_heightfield(145)/photo_sampler(146)
already in both agent kernels' closures (cc4-verified). Consts:
STEER_LOOKAHEAD_WU 4.0, STEER_GAIN 3.0, STEER_GRAD_LO 0.7 / HI 1.4
(authored fresh — anchor-c: the walkable clamp gates on a height STEP,
not a gradient const). glaw1 GREEN; reader defined-but-unused until C2b.

### [C2b] — 159a18b (the steering term)

Potential-field steering at each pre-integration seam. Other agents: in
the shared `agent_post_step`, after the speed cap and before position
integration + ground snap. Player: in `behavior_player_controlled`,
steering the input `world_vel` (now a var) before it integrates, so the
PATH bends this frame. Deflects along the level-set (side = 2D cross of
velocity×uphill). BRANCHLESS (FXC sanctum): smoothstep is 0 below LO,
min(1,sp2) quiets standstill. HARDENING: the handoff's
`normalize(vec2(-g.y,g.x))` is a latent flat-ground NaN — realized as
`/max(glen, 0.0001)` (bit-equivalent where glen>0). glaw1 GREEN; Dawn
witness ALL FAMILIES GREEN.

---

## C3 — THE SOCIAL SPLIT

### [C3a] — 4aa2555 (tier shell columns + bubble config field)

AGENT_TIER_GAINS (+ GPU + WGSL mirrors) gained `personal_radius` and
`flee_gain_player` {1.2/1.4/1.0/1.1}. ANCHOR-C RECONCILIATION: the
handoff seeded personal_radius = 8.0 "equal to the flock sensing const
for a behavior-neutral re-point", but the real flock sensing is
`agent_behaviors[FLOCK2D].neighbor_radius = 30.0` — so personal_radius
seeds **30.0** (honoring the INTENT); `behavior_flock2d` re-points
`b.neighbor_radius`→`g.personal_radius` (30==30, bit-safe). Pads gone
since T2 ⇒ AgentTierParams/GPUAgentTierDef grow 32→48 (2 columns + 2
pads; uniform array stride 16-bounded; buffer sizeof-driven, auto-
resizes; assert 32→48). `point_bubble_radius` graduates const→config:
GPUDesignConfig + WGSL DesignConfig gain the field (fills the checker
tail pad, sizeof stays 592); the sole reader (portal vertical gate)
re-points to `config.point_bubble_radius`; boot-pinned from
contracts/point.hpp POINT_BUBBLE_RADIUS (now #included) via
set_point_bubble_radius; the WGSL const becomes the REST-mirror comment.
DOCTRINE FLIP disclosed (compile-time→boot upload; point.hpp stays
source of truth). 17 edits, applied atomically. glaw1 + witness GREEN.

### [C3b] — 7a3b10f (the flee servo + point-source)

Body-to-body flee after the contact spring in both walker kernels
(personal-shell radii, NONPLAYER_FLEE_GAIN 0.8 <1 contracts; 3D gate,
planar response); the pawn pair (possessed slot) is skipped for the body
flee (keeps only the spring). update_other_agents adds THE POINT SOURCE
(agents part around `point_pos()` within personal+bubble, gain
flee_gain_player, host-routed velocity). update_cube gains a matching
parting drift force ∝ v_ap. **SIGN CORRECTION (deviation, flagged):** the
handoff's `v_ap = dot(other.vel, -dir)` measures RECEDING speed — the
matador gate would never fire. Corrected to `+dir` (approach speed);
worked example self(0,0)/other(5,0) vel(-8,0): -dir→0 (no flee) vs
+dir→8 (flee). **Independently verified** by a 3-lens adversarial panel:
unanimous CORRECTION_RIGHT, zero defects, rest-identity PRESERVED, C4
gates SATISFIED, verdict SHIP. CAMERA-HOST FALLBACK: no GPUCameraState
velocity field ⇒ v_ap := BUBBLE_PART_SPEED (deferred camera-velocity
upgrade). glaw1 GREEN; Dawn witness ALL FAMILIES GREEN.

---

## C4 — CLOSEOUT

Instruments → `_post_c2`, diffed against the C0 base `3252a6d` (NOT
post_tc1 — CLOSURE_PAWN landed between T3 and C0; those deltas are the
base, not this batch):
- **cc6:** zero substantive layout diffs; flags EMPTY. (The
  agent_tier_gains buffer element grew 32→48 B, but that is a
  sizeof-driven allocation, not a binding/layout change.)
- **cc7:** 96 → 96 declarations (+0). The config field + tier columns are
  struct members, not `@binding` declarations.
- **cc7 mirror:** 90 matched, zero orphans both directions.
- **cc4:** all four agent-kernel closures byte-identical to the C0 base
  (the new `point_pos`/`config.point_bubble_radius` refs were already
  bound). The "no new bindings anywhere" law holds.
- **Dawn witness** (`_post_c2`): 19 families / 30 entry points, ALL
  PIPELINE FAMILIES GREEN, zero module messages; limits 10 storage /
  12 uniform / 4 storage-tex match the FXC banner.

Encoding sweep: world.wgsl / state.hpp / agents.hpp / cube_behaviors.hpp
all no-BOM, LF-only; the FXC constraints banner byte-untouched. Final
glaw1 GREEN. On-master execution (Jean's direction); pushed at C4.
