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
