# TIDY_1 — CAMPAIGN LOG (THE AUDIT-5 STAMPS)

Executes the AUDIT-5 `[A5-9]` disposition. Handoff:
`src/docs/HANDOFFS/COLLISION TIDINESS/` (TIDY_1). Base: master `3cc67a8` (the
AUDIT-5 base; the review branch's code is byte-identical to it). Landed
**T1 mechanical → T2 percept → T3 charter.** Execution on the transient review
branch `claude/sync-handoffs-review-5vd7j6`, metadata trailers. Two audit
rulings OVERTURNED here: #7 (cube reach) and half of #6 (the C++-mirror route).

## P0 — THE DUMP PATH: **YES**

The witness harness CAN emit Tint's backend output. Recipe: spawn Chromium
`--headless=new --in-process-gpu --enable-dawn-features=dump_shaders,disable_symbol_renaming
--enable-logging=stderr`, drive a page that compiles each contact pipeline, and
capture the child's stderr — Dawn dumps the WGSL and the **Tint SPIR-V
disassembly** per pipeline (the SwiftShader/Vulkan backend lowers to SPIR-V, not
the Windows D3D12/FXC HLSL — but the principle holds: a backend-output diff pre/
post is the re-expression proof). `disable_symbol_renaming` keeps kernel names
(`OpEntryPoint %update_cube`), so each of the 5 SPIR-V blocks self-identifies.

**The gate is sound.** After normalizing Chrome's `", source: http://…:<port>`
annotation (the only run-varying line), the per-kernel SPIR-V is **deterministic
run-to-run** (two independent baseline dumps byte-identical across all 5 kernels).
So a non-empty pre/post diff is a real signal, not noise. Harness:
`audit/tidy1_spirv_dump.mjs`; per-kernel SPIR-V under the scratchpad.

**The T1 gate:** per-kernel SPIR-V diff, post-commit vs the commit's parent.
Byte-identical is the strong proof. A non-empty diff is a REVIEW SURFACE (read
it; confirm naming/ordering only). A semantic delta = a T1 edit that was not
pure re-expression — caught in seconds.

---

## T1 — THE MECHANICAL BATCH (backend-diff gate per commit)

### [T1a] DELETE the dead cliff-clamp pair + the comments that cite them

`query_ground_walker_walkable` + `query_ground_walker_gradient` — both zero
callers (the live wall is `pawn_ground_resolve`'s `PAWN_STEP_HEIGHT` gate).
Deleted both fns + their `LATENT[policy-surface]` preambles (world.wgsl). Fixed
the three comment sites that named them: the `STEER_GRAD_LO/HI` block re-pointed
to `PAWN_STEP_HEIGHT` (the substance — no shared steepness truth, Jean-tunable —
survives); the manifold API-listing line removed; the walker-height-note
"Walkable variant" line removed. `ground_architecture.hpp` intent-comment
(the `gradient=true is intent` rows naming both fns) removed; the live
`POLICY_WALKER` entry stays. Tree-wide grep of both names: **ZERO**.

**Gate:** SPIR-V diff vs pre = **EMPTY across all 5 kernels** (the delete touched
no live closure — Tint compiles only reachable fns). modcheck 0 messages.

### [T1b] RENAME `CUBE_PART_CAP` → `CUBE_PUSH_CAP` + the honest label

The name outlived the mechanism it named (the approach-parting, retired at P2b);
it now caps a presence impulse. Renamed the const + its two references
(world.wgsl) and gave it the hitch-guard biography: "max Δv per frame on the
cube's drift. A FRAME-HITCH GUARD, not a tuning knob: unreachable at 60 Hz (max
7·25/60 = 2.92); first engages at dt = 0.0686 s (14.6 fps). Raising it changes
nothing you can see." Code refs of the old name: **ZERO** (the remaining
mentions are in historical handoff .txt specs — input, not code).

**Gate:** SPIR-V diff vs [T1a] = **EMPTY** (rename, value 12.0 unchanged);
modcheck 0 messages.

### [T1c] VARIANT B — the profile table as contiguous `row_*()` builders

Collapsed the nine scattered `InfluenceProfile(...)` constructors into six named
`fn row_*() -> InfluenceProfile` builders sited in one block right after
`influence_response`. Dynamic columns (radii, the pair mass weight
`m_other/(m_self+m_other)`, tier gains) stay parameters; the constant columns
live once. The nine sites (contact x2, flee x2, sphere-push x2, agent-sphere,
point-flee, cube-push) now read `row_agent_contact(...)` / `row_agent_flee(...)`
/ `row_sphere_push(fe)` / `row_agent_sphere(...)` / `row_point_flee(...)` /
`row_cube_push()`. STRUCTURE, not VALUES — variant B is the prerequisite for a
master-panel *structure*, irrelevant to the *numbers* (those still live in the
five files A5-4 mapped). FXC-safe: a fn returning a constructed struct is not the
runtime-indexed `const` array the banner forbids.

**Gate (reviewed, not empty — variant B ADDS reachable fns, so growth is
expected):** per-function backend SPIR-V, canonicalized (per-fn SSA renumber,
name-suffix + whitespace normalized), across all 5 kernels:
- `update_sphere`: **byte-identical** (touches no profile — the determinism control).
- The **six row builders**: added pure constructors, each returning the struct
  its inline site built (guaranteed by count-asserted text substitution).
- The **four profile-callers** (`update_player_agent`, `update_other_agents_inner`,
  `update_camera`, `update_cube`): the *only* body change is inline
  `OpCompositeConstruct` → `OpFunctionCall %row_*`, passed to `influence_response`
  in the identical operand slot; every other instruction bit-identical (verified
  on `update_camera` as the representative single-site case).
- **Float-constant set per kernel: IDENTICAL** (guards against a fat-fingered
  column value — none). modcheck 0 messages.

### [T1d] PROMOTE `approach_floor` to the 9th `InfluenceProfile` column

`approach_floor` stops being `influence_response`'s 7th parameter and becomes the
struct's 9th field, constructed at the row: `0.0` for every row except
`row_point_flee`, which takes it as an arg (`BUBBLE_PART_SPEED` camera-host /
`0.0` pawn-host — the branch still decides at the call site, then passes the
local into the builder). The signature drops the param; the body reads
`max(p.approach_floor, dot(other_vel, dir))`. The whole profile now travels as
one value — the table has no out-of-band column.

**Gate (reviewed benign):** per-function backend SPIR-V, canonicalized, T1d vs
T1c. `update_sphere` byte-identical; the changed set is exactly
`influence_response` + the profile rows + the callers. In `influence_response`
the ONLY delta is the consuming line: `NMax %approach_floor_param %dot` →
`%m8 = OpCompositeExtract %p 8` then `NMax %m8 %dot` — **every other
`OpCompositeExtract %p N` keeps its index** (0,2,3,4,5,6,7), so appending member
8 shifted no existing field. Each row's `OpCompositeConstruct` grew 8→9 operands:
`row_point_flee`'s 9th operand is the incoming param, all other rows' is the `0.0`
constant. Float-constant set per kernel IDENTICAL; modcheck 0 messages.

### [T1e] The feasibility ledger — one `const_assert` + the cap ledger

**P-check (does Tint accept `const_assert`?): YES, and it is EVALUATED.** A
passing assert compiles with 0 errors; a *false* one errors `L4:1 const assertion
failed` — so it is a real machine-check, not silently dropped. (Probed on the
target Dawn/Tint via modcheck.)

Shipped two things, both zero-behavior:
1. **The cube `const_assert`** at `CUBE_PUSH_CAP`:
   `const_assert CUBE_PUSH_RADIUS * CUBE_PUSH_GAIN < CUBE_PUSH_CAP * 60.0;`
   (175 < 720). The cube row is the ONLY cap whose three terms are all module
   consts, so it compiles its own hitch-guard proof — the cap binds only on a
   frame hitch (~14.6 fps), never at 60 Hz.
2. **THE CAP LEDGER** by the row builders: CONTACT rows are **unassertable**
   (`contact_radius` is an `agent_tier_gains` UNIFORM field, split from the const
   caps — a uniform×const is not a const-expression; the first concrete argument
   for value consolidation). SPHERE row is a **LIVE limiter** (max impulse
   `influence_radius*SPHERE_PUSH_GAIN*dt` = 5.33 at μ=8/60 Hz, crosses the cap 6
   below 53.3 fps and at μ≥9 at 60 Hz — a real clamp, no value change). FLEE rows
   are **uncapped** (`INFLUENCE_NO_CAP`): pure APPROACH (dt-invariant) cannot run
   away, so there is nothing for a cap to guard. **No C++ mirror** of these
   consts (that reintroduces the ungated cross-language duplication AUDIT-4
   flagged).

**Gate:** SPIR-V diff vs [T1d] = **EMPTY across all 5 kernels** (compile-time
assert + comments = zero runtime effect); modcheck 0 messages.

## END-OF-T1 BUILD GATE

- **glaw1: `G-LAW 1: GREEN`** — the desktop build compiles the modified
  `world.wgsl` (const_assert, variant B, the 9th column) through the desktop
  toolchain. Desktop stays green.
- **Dawn witness: `ALL PIPELINE FAMILIES GREEN`** — module compiles with 0
  messages, no pipeline-family failures under real Dawn/Tint.

---

## T2 — THE PERCEPT BATCH (visual gate on BOTH hosts — Jean)

### [T2a] Route `behavior_pursuit` + `behavior_flee` through `point_pos()`

Both behaviors read the raw possessed slot (`agent_state[config.possessed_slot]`).
Now they read `point_pos()` — the emitter. `neighbor_radius`
(`agent_behaviors[6u]`/`[7u]`) is untouched; only the target moves.

**Scope (backend-confirmed):** SPIR-V changes in **`update_other_agents` ONLY**;
the other 4 kernels byte-identical. Within it, exactly two functions changed —
`behavior_pursuit`, `behavior_flee` — the two edited. modcheck 0 messages.

**Two percepts to visual-gate (both hosts):**

1. **Near-field repair (the fix).** In pawn-host, `point_pos()` ==
   `compute_pawn_pos()` == the possessed slot's position, so pursuit/flee are
   **behaviourally identical** there. The change is **camera-host only**: in
   free-fly the possessed pawn sits idle while the player flies the camera-point
   elsewhere — before T2a, pursuers clustered that abandoned statue and fleers
   avoided it, both ignoring where the player actually was. Now they track the
   point. This is the common failure the point-law repairs.

2. **The standoff annulus (the new interaction).** A PURSUER (behavior 6) is
   pulled INWARD toward the point by `behavior_pursuit` (within
   `agent_behaviors[6].neighbor_radius`) and simultaneously pushed OUTWARD by the
   point-flee bubble (`row_point_flee`, applied to every agent in the gather
   within `config.point_bubble_radius`). Where they balance is a standoff ring.
   **Camera-host:** the bubble carries `approach_floor = BUBBLE_PART_SPEED` (a
   permanent isotropic outward push even when the point is still), so the ring is
   **permanent** — pursuers orbit a fixed standoff rather than reaching the point.
   **Pawn-host:** the floor is 0, so the outward push acts only on real closing
   speed — pursuers **converge when the point is still, scatter when it moves.**
   Fleers (behavior 7) feel both forces outward → no standoff, they just leave
   faster. The exact ring radius is a balance of `neighbor_radius`/`home_pull`
   against `point_bubble_radius`/`flee_gain_player` — Jean's visual call (not
   asserted here).

### [T2b] Cube reach — ceiling + planar sentinel (OVERTURNS audit #7)

The old cube gate was a single vertical window `|dy| < CUBE_PUSH_VWINDOW` (85),
where `dy = fe.pos.y − point.y` and `fe.pos.y = ground_at(xz) + orbit_height +
bob + drift.y` — so **ground relief leaked into eligibility** (audit #7). Split
into two clean tests:
- **Test A — REACH:** `reach_ok = (fe.orbit_height + fe.bob_amplitude) <=
  CUBE_REACH_CEILING` — the AUTHORED mean altitude, terrain-INDEPENDENT (uses
  `bob_amplitude`, not the instantaneous bob). Folded into radius via
  `select(0.0, CUBE_PUSH_RADIUS, reach_ok)` — branchless; out of reach ⇒ radius
  0 ⇒ the gate never opens.
- **Test B — PLANAR:** new sentinel `INFLUENCE_PLANAR_ONLY = 1.0e9` as `vwindow`
  keeps the CYLINDRICAL gate (positive vwindow) with an unbounded vertical
  half-window ⇒ purely planar. `vwindow <= 0` would flip to the SPHERICAL gate
  and reinstate the CONTACT_4 altitude-vs-reach trap — so a huge positive, not a
  zero. Retired `CUBE_PUSH_VWINDOW 85` (inline epitaph).

**Scope (backend-confirmed):** SPIR-V changes in **`update_cube` ONLY**; the
other 4 kernels byte-identical. Within it, exactly `row_cube_push` + the caller
changed. The `row_cube_push` body decompiles to exactly the spec: two field
extracts (`orbit_height`, `bob_amplitude`) → add → `<=` ceiling → `OpSelect`
(7.0 in reach / 0.0 out) → construct `(select, PLANAR_ONLY, GAIN, 0, 1, CAP, 1,
0, 0)`. modcheck 0 messages.

**Percept to visual-gate (both hosts):** with ceiling 30, monoliths (~12) and
small cubes (~25) stay shoveable; medium (~45) and large (~75) become **canopy**
(unresponsive) — before T2b nearly every cube within ±85 vertical responded, so
**fewer** cubes shove now by default (the deliberate reach ceiling, Jean-tunable
— raise toward `INFLUENCE_PLANAR_ONLY` for all-shoveable). The repair: a cube on
a hill vs flat ground now has the SAME eligibility (authored altitude), where the
old `|dy|` gate coupled the terrain beneath it.

## END-OF-T2 BUILD GATE

- **glaw1: `G-LAW 1: GREEN`** — desktop compiles the reformulated `world.wgsl`
  (the `select()` reach gate, `INFLUENCE_PLANAR_ONLY`, the point-routed
  behaviors, the retired `CUBE_PUSH_VWINDOW`).
- **Dawn witness: `ALL PIPELINE FAMILIES GREEN`** — module 0 messages, no
  pipeline-family failures under real Dawn/Tint.

---

## T3 — THE CHARTER + BATCH REPORT (doc-only)

### [T3] `COLLISION_CHARTER.md` + `BATCH_REPORT_TIDY1.md`

Doc-only commit, zero WGSL/C++ change. Two new root docs:

- **`COLLISION_CHARTER.md`** — the four laws of altered motion, each a boundary
  statement + invariants: **Law 1 BODY↔BODY** (`influence_response` +
  `InfluenceProfile`, the 9-column table, the cylindrical-gate invariant, the
  cap→PRESENCE rule) with its **spawn-time sibling** (CPU spawn-footprint
  separation, `spawn_services.hpp`); **Law 2 BODY↔WORLD** (`manifold_position` +
  `POLICY_*`, revert/pin/steer); **Law 3 AGGREGATE FIELD** (agent + orb flock);
  **Law 4 ATTACHMENT/FOLLOW** (kite cube, sky-mode ribbon mount, camera
  aim-follow, sun-VP snap). Plus the **generalized feasibility rule** (a value
  split across rooms cannot be asserted in either → consolidation is enforcement,
  not preference) with its corollary; the **tombstone policy** (one-line epitaph
  at the site, lesson in the charter) with the three TIDY_1 tombstones; and the
  **two disclosed softnesses** (cube ~2.5% same-frame damping; one-frame
  point-velocity leak).
- **`BATCH_REPORT_TIDY1.md`** — P0 dump-path = YES (the recipe + soundness);
  per-T1-commit diff results (T1a/b/e empty, T1c/d reviewed-benign); Tint
  `const_assert` accepted AND evaluated (false errors); the two T2 percepts as
  observed; the two overturns (#7 full, #6 half-declined — no C++ mirror); the
  gates summary; the outstanding visual gate for Jean.

Push follows.
