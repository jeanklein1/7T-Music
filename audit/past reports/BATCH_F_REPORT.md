# BATCH F — THE SWEEP DIES + THE CONTACT ROWS — report

Cartridge: `the_board` (`incubator_dual`). Master-direct. Deviations are
REPORT, never improvisation — **and this batch ends on one: F0-a's
precondition FAILED, so per the handoff's own instruction the batch STOPPED
after Part 0.** F1/F2 did not land. The options and their binding-budget
costs are below, as instructed; no side table of radii was improvised.

**Preflight.** Not shallow. Base: `fe680886af0eb1cb482c4e41c96bbc10f44a0034`,
`git rev-list --count HEAD` = 1001 at base. LF-only, no BOM, no CR
introduced. glaw1 GREEN at base and at every commit. Symbols, never
FILE:LINE. Part 0 written whole; the only edits this batch are F-STEP-0's
merge (instructed to precede Part 0) and this report.

---

## [F-STEP-0] THE C5 MERGE — done first, as instructed

| fact | value |
|---|---|
| merged commit | `822c56e` (the Batch E rebase of SPAWN_C5) |
| merge commit | `c08666c` — "MERGE C5: the sweep dies — release is by owner everywhere, one law" |
| prediction | recorded in the merge commit message verbatim per the handoff: ribn delta −1 structurally impossible outside mood 5; the mood-5 anchor ribbon is the sole surviving −1 and retires at REQUEST_1; any other −1 in a future census is a NEW finding, not noise |
| glaw1 on merged head | **GREEN** |
| `unregister_footprints_for_patch` | **gone** — zero references in code (definition, declaration, call site all dead) |
| local branch | deleted |
| remote branch | **DEVIATION, reported:** `git push origin --delete` fails through this environment's git proxy ("the remote end hung up unexpectedly", three attempts with backoff; ordinary pushes succeed). The remote `claude/batch-c5-prepared` still exists at `822c56e` — which is now fully contained in master via the merge, so it is a harmless stale pointer of already-merged history. It needs a UI-side delete (one click in GitHub's branches page). |

---

## PART 0 — READ-ONLY CENSUS

### [F0-a] The agent kernels' reach — **THE STOP**

`update_player_agent` and `update_other_agents` are both created with
`liveContribComputeLayout` = **{ `computeEntityLayout_`,
`computeTextureLayout_` }** (renderer, pipeline 1b/1c).

**Group 0 — `computeEntityLayout_` ("Compute Entity Layout"), 12 entries:**

| binding | name | type |
|---|---|---|
| 0 | signal | uniform |
| 1 | config | uniform |
| 2 | vp_data | storage |
| 60 | agent_state | storage |
| 80 | camera_state | storage |
| 100 | floating_entities | storage |
| 62 | portal_array | uniform |
| 145 | photo_heightfield | texture 2d-array |
| 146 | photo_sampler | sampler (filtering) |
| 152 | patch_grid | read-only storage |
| 110 | agent_behaviors | uniform |
| 111 | agent_tier_gains | uniform |

**Group 1 — `computeTextureLayout_`, 4 entries:** bilinear_sampler,
nearest_sampler, pawn_aura_read, live_card_read — two samplers, two
textures, **zero buffers**.

**The mesh params are NOT reachable.** `cmg_params` (`@binding(196)`,
`array<ColumnMeshParams, 32>`) and `amg_params` (`@binding(193)`,
`array<ArchMeshParams, 16>`) live only in the dedicated mesh-gen layouts
(the arch trio 193–195, the column trio 196–198). Neither appears in either
group the agent kernels bind. **The handoff's condition fires: STOP after
Part 0, report the options.**

**The current budget, counted:** the agent kernels' compute stage binds
**5 storage buffers** (vp_data, agent_state, camera_state,
floating_entities, patch_grid) of the 10-per-stage adapter limit, and
**5 uniforms** of the 12. The history in the layout's own comment is the
cautionary tale: the agent registries were first tried as ReadOnlyStorage
and **pushed the stage past 10** — that is why they are uniforms today.
Headroom exists but has been exhausted once before.

**THE OPTIONS (decision is Jean's):**

- **Option A — grow the shared layout.** Add bindings 196 + 193 to
  `computeEntityLayout_` as ReadOnlyStorage, Compute visibility. The rows
  then read the SAME buffers the mesh-gen kernels read — one fact, one home,
  zero new buffers, zero new WGSL declarations (the module-scope
  declarations already exist; a pipeline layout that includes them makes
  them reachable from the agent entry points).
  **Cost: storage 5 → 7 of 10** — and the blast radius is the whole shared
  layout: `liveContribComputeLayout` also serves `update_camera`,
  `update_sphere`, `update_cube`, and the group-0-only `computeLayout`
  serves `compute_vp`, so all six pipelines' stages go to 7 (all still
  under 10; unused bindings in a layout are legal and free at runtime).
  FXC exposure: this is bind-group buffer growth on the agent kernels —
  exactly what the banner names as a hang trigger — gated by [G:shader].
- **Option B — a third bind group for the agent kernels only.** A new
  2-entry layout { 196, 193 } appended as group 2 of a new agent-only
  pipeline layout. Same per-stage cost (**7 of 10** — the limit is
  per-stage, not per-group), but the growth is confined to the two agent
  pipelines; sphere/cube/camera/vp stay at 5. Price: a new pipeline layout,
  a new bind group, boot-time validation rows, and the L6 registry gains a
  group-2 namespace for compute — more plumbing, same budget.
- **Option C — a side table of radii: FORBIDDEN** by the handoff, and
  rightly — it would be a second home for authored geometry, the
  two-copies-of-one-fact shape this campaign has now buried eight times.

**Recommendation, offered not landed:** Option A. The budget survives it
(7 of 10), the one-fact-one-home law is exactly its shape, and Option B's
isolation buys nothing the budget needs while adding a third bind-group
namespace to maintain. [G:shader] gates either equally.

### [F0-b] The fields — confirmed sufficient for radial body tests

**`GPUColumnMeshParams`** (pinned; mirrored as `world.wgsl::ColumnMeshParams`;
one array of 32 = columns 0–15 + antennas 16–31 via `ANTENNA_SLOT_OFFSET` —
the "same struct, thinner population" the handoff names, distinguished by
`tier` 0=Pillar 1=Doric 2=Ornate 3=Antenna):

```cpp
struct alignas(16) GPUColumnMeshParams {
    float center_x;          // ← body center x
    float center_z;          // ← body center z
    float height;
    float shaft_radius;      // ← THE lateral half-extent
    float taper; float entasis;
    float base_height; float base_overhang;
    float capital_height; float capital_overhang;
    float burial;
    float color_r, color_g, color_b;
    uint32_t base_layers, capital_layers, segs_around, shaft_rings;
    uint32_t is_active;      // ← gates the test
    uint32_t tier;
    float drum_color_r1..b3; // antenna drum colors
};
```

No rotation field — a surface of revolution needs none; the radial test
needs none. `shaft_radius` is the honest lateral half (taper/entasis only
narrow it above the foot; `base_overhang` widens the foot slightly — a
choice for F1's skin constant, noted for the row author).

**`GPUArchMeshParams`** (64 B pinned; mirrored as `ArchMeshParams`):

```cpp
struct alignas(16) GPUArchMeshParams {
    float center_x;          // ← arch center x
    float center_z;          // ← arch center z
    float rotation;          // ← leg placement needs it
    float half_span;         // ← leg centers = center ± half_span rotated
    float rise;
    float depth;             // ← leg lateral half (with thickness)
    float thickness;         // ← leg lateral half (with depth)
    float pier_height; float burial; float catenary_a;
    uint32_t segs_u, segs_v;
    float color_r, color_g, color_b;
    uint32_t is_active;      // ← gates the test
};
```

Everything the two-legs test needs exists: leg centers at
`center ± rotate(half_span, 0)`, radius from `thickness/2` and `depth/2`
(the leg's cross-section halves). The SPAN stays open by construction —
only the two leg bodies exist.

### [F0-c] The row grammar — the house dialect, quoted

The profile struct and one row of each shape the handoff named:

```wgsl
struct InfluenceProfile {
    radius:        f32,   // shell radius (see vwindow)
    vwindow:       f32,   // <= 0 : spherical gate; > 0 : cylindrical, |dy| < vwindow
    presence_gain: f32,   // overlap term (r-d)*gain, dt-scaled, 0 = off
    approach_gain: f32,   // approach term v_ap*gain, NOT dt-scaled, 0 = off
    falloff_mix:   f32,   // 0 = flat across the shell, 1 = linear (1-d/r)
    cap:           f32,   // max magnitude of the summed response
    yield_share:   f32,   // 0..1 -- how much of it THIS body takes
    tangential:    f32,   // matador split coefficient
    approach_floor: f32,
}

fn row_agent_contact(g_self: AgentTierParams, og: AgentTierParams, m_self: f32, m_other: f32) -> InfluenceProfile {
    return InfluenceProfile(g_self.contact_radius + og.contact_radius, 0.0,
                            CONTACT_SPRING, 0.0, 0.0, CONTACT_IMPULSE_CAP,
                            m_other / (m_self + m_other), 0.0, 0.0);
}
fn row_agent_sphere(g_self: AgentTierParams, fe: FloatingEntityState) -> InfluenceProfile {
    return InfluenceProfile(g_self.contact_radius + fe.body_radius, 0.0,
                            CONTACT_SPRING, 0.0, 0.0, CONTACT_IMPULSE_CAP, 1.0, 0.0, 0.0);
}
```

`row_agent_sphere` is the **immovable-authority pattern** F1 needs verbatim:
`yield_share = 1.0` — the agent takes all of it, the other body none. The
grammar note above the rows is itself load-bearing: *"a fn returning a
constructed struct is not the runtime-indexed const array the banner
forbids"* — FXC-safe by structure. The occupier rows, when they land, are
three more of exactly these: `row_agent_column(g_self, cm)`,
`row_agent_antenna` (same fn, antenna population), `row_agent_arch_leg`,
each `radius = g_self.contact_radius + <lateral half> +
OCCUPIER_CONTACT_SKIN`, `yield 1.0`, presence-only.

### [F0-d] Loop bounds + FXC, as it bears on the agent kernels

The occupier scan per agent per frame: `MAX_COLUMN_ONLY` (16) +
`MAX_ANTENNA_ONLY` (16) — one 32-slot pass over `cmg_params` — plus
`MAX_ARCH_INSTANCES` (16) × 2 legs = **64 bounded tests**, every one behind
an `is_active` gate, every loop bounded by a `Dim::` cap (compile-time
constants, the FXC-friendly shape `structure_height_at` already models with
its "keep FXC happy" bound).

What the law says about THESE kernels specifically: L2's no-new-branching
clause names the collision/**ground** chain (`pawn_ground_resolve` and
below). The agent kernels are the CONTACT chain — governed instead by the
grammar note quoted in F0-c (rows as constructed-struct fns are FXC-safe;
runtime-indexed const arrays are what the banner forbids) and by the split
that already exists for compile-time reasons (the agent kernel is two
kernels precisely because FXC inlines every behavior body — the post-step
helper was pulled out "so FXC compiles the common epilogue once per kernel
rather than ten times"). Bounded uniform loops are the established pattern:
both kernels already run the 32-slot agent-agent gather. The named hazards
for F1 are therefore (1) the bind-group buffer growth of F0-a — the actual
stop — and (2) total inlined loop count per kernel, which is why the
handoff's designed fallback (splitting the occupier scan into its own tiny
pass) exists.

### [F0-e] Who collides — one grammar, two gather sites

The possessed slot and the free agents do **not** share one gather call
site — they share the ROWS. `update_player_agent` and `update_other_agents`
each carry a structurally identical inline gather block (same 32-slot loop,
same `row_agent_contact` → `influence_response` → `vel += r` sequence,
disclosed as deliberate: the split kernels are the FXC accommodation). The
pawn's weight is already in the grammar at both sites — the player kernel
sets `m_self = contact_mass * PAWN_CONTACT_MASS_MULT`; the other-agents
kernel applies the same mult to `m_other` when `k == possessed_slot`.

**Consequence for F1, stated:** one row set serves both, as the handoff
assumed — but the gather insertion is **two call sites**, not one. For
immovable occupiers (`yield 1.0`, no mass pair) the two insertions are
byte-identical, so the duplication is mechanical, not semantic.

---

## ANCHOR VERIFICATION

| anchor | status |
|---|---|
| `update_player_agent` / `update_other_agents` (the two kernels) | **MATCHED** |
| `liveContribComputeLayout` = { computeEntityLayout_, computeTextureLayout_ } | **MATCHED** (renderer, pipelines 1b/1c) |
| `cmg_params` @binding(196) / `amg_params` @binding(193) in mesh-gen groups only | **MATCHED** — and ABSENT from the agent groups, which is the finding |
| `row_agent_contact` / `row_agent_sphere` / `InfluenceProfile` | **MATCHED** byte-for-byte (quoted above) |
| `PAWN_CONTACT_MASS_MULT` in both gathers | **MATCHED** |
| Dim caps 16/16/16 (column/antenna/arch) | **MATCHED** (`MAX_COLUMN_ONLY` = `MAX_ANTENNA_ONLY` = 16, `MAX_ARCH_INSTANCES` = 16) |

---

## [F3] THE ERASURE BATCH'S FINAL SHAPE (report-only — updated by what F0 learned)

*Premise shift, honestly stated: the handoff expected F1 to have landed and
proven the rows before this shape was drawn. F1 stopped at the binding
ruling, so this is the shape as F0 + E0-d can draw it — the erasure stays
TWO batches out (rows first, then this), and nothing below is an edit.*

**What dies when the piers die:**

- **The bake loop**: `structure_height_at` + `evaluate_pier` and their call
  in `ground_formed_with_complexity` — E0-d's reclaim: 65,536 texels ×
  `pier_count` invocations per patch bake, two transcendentals each ahead of
  the spatial reject (~786 K invocations / ~1.6 M transcendentals per bake
  at a typical live count of 12), multiplied by every patch each pier write
  dirties.
- **The regen storm**: the two pier-driven `mark_patches_for_regen` sites
  (column/antenna post-commit AABB, arch pier-pair AABB) — each pier write
  currently re-bakes every patch its inflated box touches.
- **The write/clear machinery**: `write_pier` (6 callers), `clear_pier`
  (7 sites incl. the reset_surface sweep), `recompute_and_upload_pier_count`
  + `pier_count_dirty` + the high-water `config.pier_count`,
  `cpuPiers_[68]`, the pier slot vocabulary (`PIER_TOTAL` 68,
  `PIER_ARCH_BASE` 4, `PIER_COLUMN_BASE` 36).
- **The GPU residue**: the 3,264 B `pier_instances` buffer, **binding g0:26
  freed** (currently in the terrain-bake layouts; it left the agents' layout
  at GROUND_CARD_1 [5c] already), the `PierInstance` WGSL mirror + its
  48-byte C++ pin, and the FXC-sensitive bounded pier loop leaves the bake
  entirely.
- **The comments that die with them**: the pier writers' banner
  (patch_system), the boot no-op note in `reset_surface` (its clear_pier
  paragraph), the L2 example naming `GPUPierInstance` as the byte-pinned
  hot-loop struct (the EXAMPLE dies, the law stays), and every "the wall"
  reference that survives F2's truth-fix — F2, when it lands with F1, will
  have already re-pointed "piers are the sole hard-body mechanism" prose to
  the rows.

**What must NOT die with them, named now so the erasure author doesn't cut
it:** `mark_patches_for_regen` itself (pyramids may still need it —
`contrib_pyramids_at` is a separate contributor), the ground-entries dirty
protocol (`ground_entries_dirty` has non-pier writers), and the doorway
geometry consumers of the arch vocabulary (`pier_height` feeds the mesh, not
just the pier).

**Order of proof the handoff set, restated:** the rows land (F1, pending the
binding ruling) → Jean's four runtime moves prove the shaft stop engages
BEFORE the pier wall → piers stand one more batch as the control group →
then the erasure, with the numbers above as its budget claim.

---

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| MERGE C5: the sweep dies — release is by owner everywhere, one law | `c08666c` | **GREEN** | LF, no BOM, no CR |
| BATCH F: Part 0, the stop, and the options | *(this report's commit)* | **GREEN** | LF, no BOM, no CR |

No F1/F2 commits — the batch stopped where its own law said to stop.

## CARRIED REGISTER (unchanged, no edits)

- [point:stale-readback-after-transition] → POINT_1 (the harvest protocol
  owns the fix).
- Forced mood-5 anchor ribbon (the last unruled author) → REQUEST_1.
- [ribbon:stale-tip-ref] (successor's premature death) → REQUEST_1.
- J2 (the adapted probe, one run) — Jean-side, non-blocking; the verified
  diff is in audit/BATCH_E_REPORT.md.
- **NEW: the F0-a binding ruling** — Option A vs B above; F1/F2 land the
  batch after one word.
- **NEW: the stale remote branch** `claude/batch-c5-prepared` — already-
  merged history; needs a UI-side delete (the proxy refuses delete pushes).
