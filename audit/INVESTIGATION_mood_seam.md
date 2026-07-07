# INVESTIGATION — mood-transition patch seam / discontinuity

Read-only diagnostic, non-blocking (Part 3 of the GATE RESULT + CADENCE
CHANGE handoff). No code was changed by this investigation. Branch
`FINAL_TOUCH`; findings anchored to the live tree at commit `63a1742`
(campaign B1), which post-dates the campaign edits — the LOD subsystem
named below was untouched by any campaign/closeout stage.

## Charter (as briefed)

> A mood transition leaves a visible discontinuity at patch boundaries.
> "Something gets stale." Worse the further the pawn is from origin.
> Pre-existing (predates the refactor). Two named suspects: **S-A**
> grid-dim / stride twins; **S-B** the five duplicated finite-bound
> copies in mood.inl.

## Method & tree-anchoring note

The parallel read pass initially ran in a worktree that had forked from
an **ancestor** of FINAL_TOUCH (`e82abc2`, "refactoring cartridge
generative algorithm overhauling") rather than the working tip. `e82abc2`
*is* an ancestor of FINAL_TOUCH (it is the merge-base), so the mechanism
transfers, but its line numbers are stale (e.g. mood.inl 262 vs. the
live 536; a pre-refactor copy sits under `4 4 BEFORE REFAC/`). **Every
address below was re-anchored by symbol against the live FINAL_TOUCH
tree** and re-verified after the Stage-3 pop-batch deletion shifted
cartridge.hpp by −245 lines. The LOD subsystem is byte-for-byte the same
system in both trees (verified).

## Symptom, restated precisely

The discontinuity is **geometric**, not a data-staleness artifact. The
"something gets stale" framing is *disconfirmed* in the state sense (see
S-A / S-B / transition verdicts — nothing survives a mood change to go
stale). The mechanism that actually fits all three symptom clauses is a
distance-dependent **level-of-detail seam**, below.

## Suspects examined

| Suspect | Verdict | Evidence |
|---|---|---|
| **S-A** grid-dim / stride twin ("13" vs live) | **RULED OUT** | The "13" survives only as a *stale comment* (state.hpp GPUTileGrid.side annotation). No merge/edge/stride computation reads 13. The bake+render tile stride is the **dynamic** `tile_grid.side` (uploaded; =17 open, `2*(fr+1)+1` finite). Heightfield layers, instance buffer, and MAX_PATCHES are all `PREGEN_SIDE²`=225. No 13-vs-15/17 or 225-vs-289 divergence exists in any *computation*. One latent-but-inert twin: `tileGridSide` is uploaded from runtime `activeRadius_` rather than the const, but always yields the correct value (`activeRadius_`=7 open, capped to `finiteRadius_ ∈ [1,4]`). |
| **S-B** five duplicated finite-bound copies (mood.inl) | **RULED OUT** | All five copies read the *same* live `finiteRadius_` member and the `PATCH_EXTENT` const. `finiteRadius_` is written once per world and never mutated mid-world, so the transition path and the streamer cannot disagree. Duplication is a DRY smell, not a correctness bug. |
| Transition vs. streamer coherence | **AGREE (not the cause)** | A mood change is FADE_OUT → TEARDOWN → FADE_IN. Teardown clears `tileCache_`, zeroes the patch set, and resets `lastCenter = INT32_MAX`, so the next `stream_patches` is a **full regen**. No patch, tile, or modifier survives a transition to carry stale state across it. |

## Root-cause hypothesis (leading; medium-high confidence)

**LOD-0 / LOD-1 T-junction cracks at the pawn-relative LOD boundary,
with no stitching geometry.**

- Terrain patches render at **LOD-0** (64×64) or **LOD-1** (32×32). The
  LOD-1 index buffer is built by stepping through the *same* 65×65 vertex
  grid with `step = 2` — it **skips every other edge vertex**
  (state.hpp:3020).
- There is **no skirt / stitch geometry** anywhere in the terrain path
  (grep for skirt/stitch/T-junction in the_board returns only palm-crown
  foliage params, never terrain). So where a LOD-0 patch abuts a LOD-1
  patch, the LOD-1 edge is a **coarser polyline missing the LOD-0 edge
  vertices** → a T-junction, i.e. a hairline crack / shading seam along
  the shared boundary.
- LOD assignment is **pawn-relative**: patches inside
  `LOD_FULL_RADIUS = 3.5` patches are LOD-0, beyond it LOD-1
  (cartridge.hpp:3812). The boundary is a moving annulus at
  `3.5 × PATCH_EXTENT(50) = 175` world units from the pawn.

### Why it fits every clause

- *"discontinuities at patch boundaries"* = the T-junction crack at the
  LOD-0/LOD-1 boundary.
- *"worse away from origin"* = walking toward the finite boundary pushes
  more of the world across the 3.5-patch ring into LOD-1, so more
  boundaries become mixed-LOD T-junctions that were uniform LOD-0 at
  spawn/origin.
- *"pre-existing"* = pawn-relative LOD + absent skirts predate the
  generative refactor; the mood transition merely *re-reveals* the seam
  after each full regen (it does not create it).

## Corroboration in the live tree

The engine **already documents this exact annulus** as a trouble zone —
cartridge.hpp:3840-3845:

> "Push the CPU's banding pawn so the GPU frustum-cull shader uses the
> same pawn position to apply the LOD0 distance gate. Without this, GPU
> reads the live pawn (1-2 frames ahead of pawnReadback) and disagrees
> with CPU at the LOD0/LOD1 boundary annulus, causing patch flicker
> around ~175 world units from the pawn."

That fix synchronizes **which tier** each patch is assigned to (killing a
CPU/GPU *classification* flicker). It does **not** add stitching, so it
cannot close the **geometric** T-junction between a correctly-classified
LOD-0 patch and its correctly-classified LOD-1 neighbour. The remaining
discontinuity is that geometric gap — distinct from, and downstream of,
the flicker the existing comment addresses. Same ring (~175 wu), two
different defects.

## Live FINAL_TOUCH addresses (re-anchored)

| Symbol / site | Address | Role |
|---|---|---|
| `PATCH_MESH_N = 64` | state.hpp:113 | LOD-0 subdivisions |
| `PATCH_MESH_N_LOD1 = 32` | state.hpp:115 | LOD-1 subdivisions |
| LOD-1 decimation `step = PATCH_MESH_N / PATCH_MESH_N_LOD1  // = 2` | state.hpp:3020 | skips every other edge vertex |
| `stride = PATCH_MESH_N + 1  // 65` | state.hpp:3021 | shared vertex grid both LODs index into |
| `patchIndexBufferLOD1_` | state.hpp:1537 | half-res index buffer (the decimated edges) |
| `LOD_FULL_RADIUS = 3.5f` | cartridge.hpp:2521 | LOD-0/LOD-1 split radius (patches) |
| `LOD0_CYLINDER_RADIUS = 3.5 × PATCH_EXTENT = 175 wu` | cartridge.hpp:2535 | the moving boundary annulus |
| `if (d2 <= LOD0_CYLINDER_RADIUS_SQ) lod0 else lod1` | cartridge.hpp:3812-3817 | per-patch tier assignment |
| LOD band pack + `lod_pawn` sync comment | cartridge.hpp:3824-3848 | the documented ~175 wu flicker fix |
| `PATCH_EXTENT = 50.0f` | state.hpp:103 | world units per patch side (→ 3.5×50=175) |

## Settling test (for Jean — confirm before any fix is scoped)

Launch a finite mood (key 8/9). Stand at origin, note the seam density.
Walk toward the boundary and watch the boundaries **~175 wu out**:

- New cracks appearing along a ring that **tracks the pawn** → LOD
  T-junction **confirmed** (this hypothesis).
- Seams pinned to **fixed world coordinates** (not moving with the pawn)
  → a bake-value issue instead; the next step would be to instrument
  `mark_patches_for_regen` rather than the LOD path.

## Remediation directions (NOT executed — for a future handoff)

Recorded only so the next work order has a starting menu; the charter was
diagnosis, and no fix is authorized here.

1. **Skirt geometry** — extrude a short vertical curtain at every patch
   perimeter so a crack is hidden behind overlapping wall, not open sky.
   Cheapest; hides the seam without solving the topology.
2. **Edge-vertex retention** — keep LOD-1 patches at full resolution on
   any edge shared with a LOD-0 neighbour (constrained decimation), so
   the shared polyline matches. Correct but needs neighbour-LOD awareness
   at index-build time.
3. **Uniform-LOD annulus** — widen or feather the transition so abutting
   patches are never more than a discrete LOD apart, and/or geomorph the
   LOD-1 edge vertices toward their LOD-0 positions across the band.

## Confidence

Medium-high. The mechanism is structurally present and uniquely fits all
three symptom clauses, and the engine independently flags the same 175 wu
annulus. The settling test converts this to a certainty (or redirects to
the bake path) in one launch.
