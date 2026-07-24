# TIDY_1 — BATCH REPORT (THE AUDIT-5 STAMPS)

Executes the AUDIT-5 `[A5-9]` disposition against the audited tree. Base master
`3cc67a8`; developed on the transient review branch
`claude/sync-handoffs-review-5vd7j6`. Eight commits: **T1 mechanical (5) → T2
percept (2) → T3 charter (1)**. Two audit rulings **OVERTURNED** here: **#7**
(cube reach) in full, and **half of #6** (the C++-mirror route — declined).

| stage | commit  | what | gate result |
|-------|---------|------|-------------|
| T1a | `2c84d4a` | delete dead cliff-clamp pair + citing comments | SPIR-V **empty** 5/5 |
| T1b | `78b9e99` | rename `CUBE_PART_CAP`→`CUBE_PUSH_CAP` + hitch-guard label | SPIR-V **empty** 5/5 |
| T1c | `c1f526d` | variant B — 9 inline ctors → 6 `row_*()` builders | benign (construct→call) |
| T1d | `edb5d7c` | promote `approach_floor` to the 9th profile column | benign (member-8 extract) |
| T1e | `54c1d93` | `const_assert` + the cap ledger | SPIR-V **empty** 5/5 |
| T2a | `e8f5b6e` | route pursuit/flee through `point_pos()` | scoped to `update_other_agents` |
| T2b | `0f4d248` | cube reach ceiling + planar sentinel (overturns #7) | scoped to `update_cube` |
| T3  | *(this)* | the charter + this report | doc-only |

---

## P0 — THE DUMP PATH: **YES**

The witness harness CAN emit Tint's backend output, so a mechanical edit can be
proven a pure re-expression by a **backend-output diff**. Recipe: spawn Chromium
`--headless=new --in-process-gpu --enable-dawn-features=dump_shaders,disable_symbol_renaming
--enable-logging=stderr`, drive a page that builds each contact pipeline, capture
the child's stderr. Dawn dumps the **Tint SPIR-V disassembly** per pipeline (the
SwiftShader/Vulkan backend lowers to SPIR-V, not Windows D3D12/FXC HLSL — but the
principle holds: a backend-output diff pre/post is the re-expression proof).
`disable_symbol_renaming` keeps kernel names so each of the 5 SPIR-V blocks
self-identifies by `OpEntryPoint GLCompute %<kernel>`.

**The gate is sound.** After normalizing Chrome's per-run `", source: http://…"`
annotation, the per-kernel SPIR-V is **deterministic run-to-run**. So a non-empty
pre/post diff is real signal. Harness: `audit/tidy1_spirv_dump.mjs`; the
per-function canonicalizer (`spv_fncmp.py`) matches functions by name and
canonicalizes SSA IDs per-function so renumbering + name-suffix churn does not
mask a real semantic delta. `update_sphere` (touches no profile) stayed
byte-identical across every T1/T2 step — the determinism control.

---

## T1 — per-commit backend-diff results

- **T1a** (delete): SPIR-V **EMPTY across all 5 kernels** — the deleted closure
  had zero callers, so Tint (which compiles only reachable fns) emitted nothing
  different.
- **T1b** (rename): SPIR-V **EMPTY** — a rename with value `12.0` unchanged.
- **T1c** (variant B): **benign, reviewed.** Variant B *adds* reachable fns, so
  growth is expected. `update_sphere` byte-identical; the six `row_*()` builders
  are added pure constructors; the four profile-callers change ONLY inline
  `OpCompositeConstruct` → `OpFunctionCall %row_*`, passed to `influence_response`
  in the identical operand slot; every other instruction bit-identical (verified
  on `update_camera`). Float-constant set per kernel identical.
- **T1d** (9th column): **benign, reviewed.** In `influence_response` the only
  delta is the consuming line — `NMax %approach_floor_param %dot` becomes an
  `OpCompositeExtract` of member 8 then `NMax`; **every other
  `OpCompositeExtract %p N` keeps its index** (0,2,3,4,5,6,7), so appending
  member 8 shifted no existing field. Each row's construct grew 8→9 operands:
  `row_point_flee`'s 9th operand is the incoming param, every other row's is the
  `0.0` constant.
- **T1e** (ledger): SPIR-V **EMPTY** — a compile-time `const_assert` + comments,
  zero runtime effect.

**Does Tint accept `const_assert`? YES — and it is EVALUATED.** A passing assert
compiles with 0 errors; a *false* one errors `L4:1 const assertion failed` on the
target Dawn/Tint (probed via `modcheck`). So the cube-row assertion is a real
machine-check, not silently dropped.

---

## T2 — the two percepts, as observed

Both T2 edits are behavioral; their backend scope was confirmed surgical (T2a →
`update_other_agents` only, functions `behavior_pursuit`+`behavior_flee`; T2b →
`update_cube` only, `row_cube_push`+caller). The **visual gate is PENDING** —
Claude cannot see the canvas; Jean runs both hosts.

- **T2a — point-routed pursuit/flee.** In **pawn-host** the change is
  behaviourally identical (`point_pos()` == the possessed slot's pos there). The
  observable change is **camera-host only**: pursuers/fleers now track the flown
  point instead of the idle possessed statue (the near-field repair). Second
  percept to watch: a **standoff annulus** for pursuers — pulled inward by
  `behavior_pursuit`, pushed outward by the point-flee bubble; **permanent** in
  camera-host (the `BUBBLE_PART_SPEED` floor is a standing outward push),
  **conditional** in pawn-host (floor 0 → converge when the point is still,
  scatter when it moves). Fleers feel both forces outward → no standoff.
- **T2b — cube reach ceiling (overturns #7).** With `CUBE_REACH_CEILING = 30`,
  monoliths (~12) and small cubes (~25) stay shoveable; medium (~45) and large
  (~75) become **canopy** (unresponsive). So **fewer** cubes shove by default
  than the old ±85 window — the deliberate reach ceiling (Jean-tunable; raise
  toward `INFLUENCE_PLANAR_ONLY` for all-shoveable). The repair: a cube on a hill
  vs flat ground now has the SAME eligibility (authored altitude), where the old
  `|dy|` gate coupled the terrain beneath it. `row_cube_push` decompiles to
  exactly the spec: two field extracts → add → `<=` ceiling → `OpSelect`
  (7.0 in reach / 0.0 out) → construct `(select, PLANAR_ONLY, GAIN, 0, 1, CAP, 1,
  0, 0)`.

---

## The two overturns

- **#7 (cube reach) — OVERTURNED in full (T2b).** The audit's per-instance
  vertical window was buggy: `|dy|` coupled `ground_at(xz)`, so terrain relief
  under the cube changed its eligibility. Replaced by the terrain-independent
  authored-altitude reach test + a planar-only cylinder.
- **#6 (the C++-mirror route) — HALF DECLINED.** The consolidation *intent* is
  adopted (the T1e cap ledger + the charter's feasibility rule name value-split
  as the enforcement problem), but the proposed **C++ mirror of the WGSL consts
  is declined** — it reintroduces the ungated cross-language duplication AUDIT-4
  flagged. The `const_assert` lives with the WGSL that uses it; consolidation
  (bringing tier radii next to the caps) is the enforcement path, not a mirror.

Also **declined** (from `[A5-9]`, unchanged): #15 (sphere shell in
`CONTACT_SHELL_DEBUG`); #12 (the six Law-2 pin sites — an A5-6 seed, not this
batch).

---

## Gates summary

- **modcheck: 0 messages** after every commit (real Dawn `createShaderModule` +
  `getCompilationInfo`).
- **Per-kernel backend SPIR-V:** empty (T1a/T1b/T1e) or reviewed-benign
  (T1c/T1d); T2 scoped to the single owning kernel each.
- **END-OF-T1 and END-OF-T2 build gate:** `glaw1 → G-LAW 1: GREEN` (the desktop
  build compiles the modified `world.wgsl` through its own toolchain — mirror
  doctrine upstream stays green) and **Dawn witness → `ALL PIPELINE FAMILIES
  GREEN`** (module 0 messages, no pipeline-family failures) both times.
- **Discipline:** LF-only, no BOM; the FXC constraints banner byte-untouched;
  every edit an anchored, count-asserted script; comments describe present
  behavior; retirements leave one-line epitaphs (lessons in `COLLISION_CHARTER.md`).

## Outstanding

The **visual parity gate for T2a + T2b on both hosts** (pawn + free-fly) is
Jean's — the two percepts above are the disclosure to check against. No behavior
was invented; where the reach ceiling (30) or the standoff radius is a judgment
call, it is flagged as Jean-tunable, not asserted.
