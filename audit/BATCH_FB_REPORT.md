# BATCH F-B — THE AGENTS' ROOM + THE OCCUPIER ROWS — report

Cartridge: `the_board` (`incubator_dual`). RULING STAMPED: **Option B** — the
agents' room. Executed as stamped; deviations are REPORT, never
improvisation.

**Preflight.** Not shallow. Base:
`2bcba6266bbedbb3be91a247756f07d172906e21` (the Batch F head,
post-C5-merge), `git rev-list --count HEAD` = 1004 at base. Master-direct.
LF-only, no BOM, no CR introduced. glaw1 GREEN at base and every commit.
Symbols, never FILE:LINE. Part 0 written whole before the first edit. The
stale remote `claude/batch-c5-prepared` remains Jean's one click — noted,
not touched.

---

## PART 0 — READ-ONLY

### [FB0-a] Group 2 is virgin

`rg "@group(2)"` over world.wgsl: **zero hits.** The namespace is free; the
layout index 2 is the room's.

### [FB0-b] The types + access

The two existing declarations, verbatim:

```wgsl
@group(0) @binding(193) var<storage, read>       amg_params: array<ArchMeshParams, 16>;
@group(0) @binding(196) var<storage, read>       cmg_params: array<ColumnMeshParams, 32>;
```

Both are already `var<storage, read>` — ReadOnlyStorage is not merely
declarable, it is their existing access mode, so the group-2 windows are
byte-identical declarations at a different (group, binding). FB2 reuses the
struct types `ColumnMeshParams` / `ArchMeshParams` unchanged; the buffers
stay the single home (two windows, one buffer each).

### [FB0-c] The house patterns, named

- ***Layout_ members**: declared in the GPUState private section beside
  their kin (the mesh-gen family `archMeshGenLayout_ // bindings 193-195`
  etc.); `agentOccupierLayout_` + `agentOccupierBindGroup_` join that
  family with the same comment style.
- **Layouts and groups built** in `GPUState::createBindGroups()`: a
  `std::array<wgpu::BindGroupLayoutEntry, N>` block with per-entry
  binding/visibility/type, a labeled descriptor, create + false-return on
  failure; the bind group later in the same function from the existing
  buffers with explicit sizes (the "Arch Mesh Gen BindGroup" block is the
  copied model — it already binds `archMeshParamsBuffer_`).
- **Exposure**: the getter pair convention `agent_occupier_layout()` /
  `agent_occupier_group()` beside `compute_entity_layout()` /
  `compute_entity_group()`.
- **The renderer** fetches layouts once at init
  (`computeEntityLayout_ = gpuState.compute_entity_layout();`) and builds
  pipeline layouts from arrays of bind-group layouts; the
  `dispatch_update_*` signature pattern takes bind groups as parameters and
  `SetBindGroup(n, …)`s them; the call sites in the compute pass pass
  `c->gpuState_.*_group()`.
- **Boot validation convention**: creation-failure propagation — every
  layout, group, and pipeline creation is checked and false-returns out of
  boot (the registry's "boot-time bind-group and pipeline validation" is
  this chain plus Dawn's own layout↔shader validation at
  CreateComputePipeline, which will refuse a pipeline whose WGSL bindings
  the layout does not cover — the lockstep witness for L6).
- **The L6 home**: `binding_registry.hpp`, where `namespace g2` opens after
  `g1` with the file's law (numbers group-scoped, authored, one constant
  per site, names equal the WGSL variable names for greppability).

### [FB0-d] Re-anchor at the new base

The diff between the Batch F base and this base touched only the C5-merge
files (patch_system, spawn_engine, spawn_services, grounded,
entity_pipeline — comment/release-path lines) and audit/. **Neither
world.wgsl nor state.hpp nor agents.hpp moved**, so:

| anchor | status |
|---|---|
| the inline contact gather in `update_player_agent` (32-slot loop, `row_agent_contact` → `influence_response`, then the sphere-push loop) | **MATCHED** |
| the inline contact gather in `update_other_agents` (same block, `m_other` possession mult, then the agent-vs-sphere loop) | **MATCHED** |
| Batch F report's struct-field verification (column `shaft_radius`, no rotation; arch `half_span`/`thickness`/`depth`/`rotation`; antennas as slots 16–31 of the one 32 array) | **CITED AS STANDING** — the files did not move |
| `row_agent_sphere` (the immovable-authority pattern, yield 1.0) | **MATCHED** |
| `InfluenceProfile.vwindow` — `> 0` = cylindrical gate, `INFLUENCE_PLANAR_ONLY = 1.0e9` as the planar-only sentinel (`row_cube_push` precedent) | **MATCHED** — the occupier rows use it: a column is a vertical body; its gate is a cylinder, not a ball |

One authored value Part 0 had to select (recorded here before the edit):
**`OCCUPIER_CONTACT_SKIN = 1.6`**. In `row_agent_sphere` the shell is
`g_self.contact_radius + fe.body_radius`; the stamped occupier signature
`occupier_contact(self_p, dt)` is deliberately body-agnostic (zero
per-kernel variation), so the skin plays the `contact_radius` role for
every tier. The tier radii are 1.4–2.0 wu (worker 1.6); 1.6 is the
population's center. Jean-tunable, authored once beside the CONTACT_* block.

---

## THE COMMITS

### [FB1] The room (C++)

- `bind::g2` opens in the L6 registry with its two rows:
  `occupier_cmg = 0`, `occupier_amg = 1` (names equal the WGSL vars, per
  the registry's law).
- `agentOccupierLayout_`: exactly two ReadOnlyStorage entries, Compute
  visibility. Nothing else — the room grows only when a named tenant
  arrives.
- `agentOccupierBindGroup_`: created once at boot from
  `columnMeshParamsBuffer_` and `archMeshParamsBuffer_` — the same buffers
  the mesh-gen groups bind; one fact, one home, two windows.
- The renderer's three-group pipeline layout
  `{computeEntityLayout_, computeTextureLayout_, agentOccupierLayout_}` is
  used by `update_player_agent` and `update_other_agents` ONLY; the other
  four live-contributor pipelines keep the two-group layout untouched
  (Option B's dividend: the FXC hang class is confined to exactly the two
  pipelines being changed).
- `dispatch_update_player_agent` / `dispatch_update_other_agents` gain the
  third bind-group parameter; the two call sites pass
  `agent_occupier_group()`; `SetBindGroup(2, …)`.

### [FB2] The rows (WGSL)

- Declarations beside the CONTACT rows, reusing the existing struct types:

  ```wgsl
  @group(2) @binding(0) var<storage, read> occupier_cmg: array<ColumnMeshParams, 32>;
  @group(2) @binding(1) var<storage, read> occupier_amg: array<ArchMeshParams, 16>;
  ```

- **One shared function**, `occupier_contact(self_p, dt) -> vec2`, called
  from BOTH gather sites (inserted after each kernel's sphere-push loop —
  the same immovable-pushes-agent family). Two bounded loops inside:
  SHAFTS (32 slots — columns 0–15, antennas 16–31, one field one law;
  radius = `shaft_radius + OCCUPIER_CONTACT_SKIN`) and ARCH LEGS (16 × 2
  bodies at `center ± half_span` rotated; radius =
  `max(thickness, depth)/2 + skin`; the SPAN stays open). Every test
  `is_active`-gated; `yield 1.0` on the agent, zero on the occupier — the
  `row_agent_sphere` immovable-authority pattern; the gate is CYLINDRICAL
  (`INFLUENCE_PLANAR_ONLY`) because a column is a vertical body — an agent
  at any height meets the shaft, exactly the `row_cube_push` precedent.
- The named fallback (inline-dup on FXC objection) was NOT needed at
  authoring; it remains [G:shader]'s designed retreat.
- Truth-fixes ride the commit: the pier prose that claimed sole hard-body
  authority now names the rows.

**PIERS REMAIN UNTOUCHED** — the control group, one batch from the grave;
the erasure's shape is already drawn in the Batch F report's F3.

---

## THE AGENT STAGE'S POST-BATCH BINDING LEDGER (for the week's couplings)

| group | layout | entries | storage bufs | uniform bufs |
|---|---|---|---|---|
| 0 | computeEntityLayout_ (shared, untouched) | 12 | 5 (vp_data, agent_state, camera_state, floating_entities, patch_grid) | 5 (signal, config, portal_array, agent_behaviors, agent_tier_gains) |
| 1 | computeTextureLayout_ (shared, untouched) | 4 | 0 | 0 (2 samplers + 2 textures) |
| 2 | **agentOccupierLayout_ — THE AGENTS' ROOM** | **2** | **2** (occupier_cmg, occupier_amg — read-only windows onto the mesh-param buffers) | 0 |

**Stage totals after this batch: 7 storage of 10, 5 uniform of 12.**
Headroom for the couplings arriving this week: **3 storage buffers,
7 uniforms** on the agent stage — and the room is where they land, with
binding numbers 2, 3, … in `bind::g2`, without touching the six pipelines
that share the entity layout. The other five compute pipelines' stages are
unchanged at 5 storage.

## ANCHOR VERIFICATION (landed against)

| anchor | status |
|---|---|
| `@group(2)` virgin in world.wgsl | **MATCHED** (zero hits pre-edit; the room's now) |
| `amg_params` / `cmg_params` declarations (`var<storage, read>`, 193/196) | **MATCHED** byte-for-byte |
| the "Arch Mesh Gen Layout" / "Arch Mesh Gen BindGroup" blocks (the copied model) | **MATCHED** |
| both kernels' sphere loops (the insertion points) | **MATCHED** |
| the C++ arch pier leg math (`center ± half_span` via cos/sin — the row's geometry twin) | **MATCHED** — the WGSL legs use the identical form |
| `row_agent_sphere` / `row_cube_push` (the two patterns the row composes) | **MATCHED** |

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| BATCH F-B: Part 0 — the room is virgin, the patterns named | `48e77ce` | GREEN (base) | LF, no BOM, no CR |
| AGENTS_ROOM: group 2 opens — the occupier windows (FB1) | `f812701` | **GREEN** | LF, no BOM, no CR |
| OCCUPIER_ROWS: the standing bodies' word (FB2) | `6de02a8` | **GREEN** (C++ surface) | LF, no BOM, no CR |
| BATCH F-B: the report | *(this commit)* | **GREEN** | LF, no BOM, no CR |

Base `2bcba62`, master-direct throughout. The named FXC fallbacks were not
needed at authoring and remain [G:shader]'s designed retreats: inline-dup on
a shared-fn objection (naming its twin), loop-split into a tiny dedicated
pass on a count objection.

## GATE STATUS

- **[G:glaw1]** — table above, all GREEN.
- **[G:shader]** — Jean's full FXC recompile. The hang-risk surface is
  exactly the two agent pipelines (Option B's dividend). The rows are
  arithmetic + two bounded `is_active`-gated loops (32 + 16×2 = 64 tests)
  through constructed-struct profiles — the FXC-safe shape the grammar note
  itself certifies.
- **[G:runtime-J]** — the four moves at a fixed seed: (1) column face-on —
  soft radial stop at the shaft, engaging BEFORE the legacy pier wall (the
  row's shell is `shaft_radius + 1.6`; the pier wall sits at
  `shaft + padding` in the bake — the push meets you first); (2) strafe —
  smooth slide, no snag (radial presence has no tangential term);
  (3) the arch — clean through the span, refused softly at either leg;
  (4) the Batch E dune at two frame rates — unchanged; the rows touch
  velocities only and never enter `pawn_ground_resolve`.
- **[G:visual]** — nothing moves a pixel; the rows move only velocities.

## JEAN RUNBOOK (standing)

- One click: delete remote `claude/batch-c5-prepared` (merged history; the
  proxy refuses delete pushes from here).
- The four moves above.
- J2 (the adapted probe, one run) — still open, still non-blocking; the
  verified diff is in audit/BATCH_E_REPORT.md.
