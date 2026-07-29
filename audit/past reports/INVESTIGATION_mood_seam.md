> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# INVESTIGATION — mood-transition patch seam / discontinuity

Read-only diagnostic, non-blocking. **This revision supersedes the earlier
T-junction hypothesis**, per Jean's redirect: three reported conditions
(the crack is *intermittent*, *appears after mood transitions*, and
*correlates with leaving the initialization area* — near origin, no
transition, it does not occur) a static geometric seam cannot produce.
A fixed LOD T-junction would render *always and everywhere* the boundary
is drawn; these conditions point at **state / world-space precision that
degrades as the world travels from its init origin**, with the LOD
boundary being where it becomes *visible*, not its cause.

No code changed. Anchored to `FINAL_TOUCH @ c55a67b` (HEAD verified).

## Method & tree-anchoring note

The first read pass ran in a worktree forked from an *ancestor* of
FINAL_TOUCH (`e82abc2`, the merge-base) with stale line numbers. All
addresses below were re-anchored by symbol against the live tree and
re-verified after the Stage-3 pop-batch deletion shifted cartridge.hpp by
−245 lines. (Standing rule going forward: isolated-worktree agents launch
pinned to a named commit and print/confirm HEAD before reading.)

## Symptom, corrected

A **distance-correlated, intermittent** discontinuity along patch
boundaries — not a static geometric gap. It is absent near origin, grows
as the pawn leaves the init area, and (per the added test) is expected to
*reset* when the pawn returns near origin. That signature is the
fingerprint of **absolute-world-space evaluation losing float32 precision
with distance** — not stale mesh topology.

## Suspects still ruled out (unchanged, and consistent with the redirect)

| Suspect | Verdict |
|---|---|
| **S-A** grid-dim / stride "13" twin | RULED OUT — the "13" is a stale comment; every computation uses the dynamic `tile_grid.side`. No dimension mismatch exists. |
| **S-B** five duplicated finite-bound copies | RULED OUT — all read the same single-source `finiteRadius_`, written once per world, never mutated mid-world. |
| Transition vs. streamer coherence | AGREE — TEARDOWN clears `tileCache_`, zeroes patches, `last_center = INT32_MAX`; the next stream is a full regen. Nothing carries stale state *across* the transition. (This is why the seam appears *after* the pawn moves, not at the transition instant.) |

## Redirected suspects — findings against (a) / (b) / (c)

### (a) World-space precision — LEADING (structurally confirmed)

**There is no floating-origin / recenter of the terrain onto the pawn.**
Patches are keyed and baked in *absolute* world coordinates that grow
without bound as the pawn walks away from (0,0):

- `make_patch_params` (cartridge.hpp:2474-2475):
  `p.origin = ((gx + 0.5) * PATCH_EXTENT, (gz + 0.5) * PATCH_EXTENT)` —
  `gx/gz` are absolute grid indices; nothing rebases them to a local
  frame. At gx≈1000 the origin is ≈50 000 wu.
- The GPU bake evaluates each texel's world position **per patch,
  independently** (world.wgsl:6711-6715):
  `world_xz = patch_params.origin + (uv − 0.5) * extent`, then
  `ground_formed_with_complexity(world_xz)`.

**The crack mechanism.** Two adjacent patches share an edge. Patch A's
far edge is `origin_A + 0.5·extent`; patch B's near edge is
`origin_B − 0.5·extent`. Algebraically equal (`origin_B = origin_A +
extent`). In **float32 they are not** — `(origin_A + 0.5·extent)` and
`((origin_A + extent) − 0.5·extent)` round differently, and the gap is
≈1 ULP *at the magnitude of the origin*. Near (0,0) the ULP is ~1e-5 wu
(sub-visible); at 50 000 wu it is ~4e-3 wu. Each patch then samples the
high-frequency terrain function at a slightly *different* world point
along the shared edge, so the edge heights disagree → a hairline crack at
the patch boundary whose size scales with the local gradient × the ULP
gap. This is **intermittent** (whether the two roundings coincide depends
on the exact origin bits), **worsens with distance from origin** (ULP
grows), and **vanishes near origin** (ULP → sub-pixel).

**Why "after a mood transition."** TEARDOWN *resets the pawn to origin*
and installs a new seed (cartridge.hpp:2973 `active_seed = …`;
2983-2984 `readback_x/z = 0`; 3002 pawn at `Idle::PAWN_POS 0,0`). So a
transition returns you to a fresh origin with cracks gone; they reappear
only once you **leave** the new world's init area — precisely the
reported ordering. The transition doesn't *create* the seam; it resets
the reference point the seam is measured from.

**Why it shows at the LOD boundary.** There is exactly **one heightfield
+ gradient texture per patch, shared by both LOD meshes** (LOD differs
only in the index buffer). So the boundary crack is between *adjacent
patches*, and the LOD-0/LOD-1 transition merely makes an existing
patch-boundary discrepancy more visible (a full-res edge abutting a
decimated one). Jean's read — "the LOD boundary is where it becomes
visible, not the cause" — is correct.

### (b) LOD height/gradient regeneration lag — SECONDARY (transient only)

There is no separate per-LOD height data to fall out of sync (one
heightfield per patch, above), so the literal "one LOD lags" cannot
happen. The nearest real variant: `init_patch_system` (cartridge.hpp:2370)
on teardown clears CPU bookkeeping and `tileCache_` but **does not zero
the GPU heightfield layers**; each layer re-bakes when its patch
re-streams, over a **budgeted** several-frame window. Any resulting
old-world/new-world boundary is a *transient* during FADE_IN (masked by
the fade), not a persistent, distance-correlated seam. Does not fit the
symptom triad.

### (c) Stale LOD-boundary center after transition — SECONDARY

The LOD split reads the pawn readback (`centerX/Z =
floor(readback / PATCH_EXTENT)`, cartridge.hpp:3506-3507) and the band
pack measures from `pawn_wx/pawn_wz`, pushed to the GPU as `lod_pawn`
(3846). A stale center makes CPU and GPU disagree about *which tier* a
patch is — a **classification flicker**, already mitigated by the
`lod_pawn` sync (documented at 3840-3845, "~175 wu from the pawn"). That
is a flicker of tier assignment, not a height discontinuity, and it is
pawn-relative (moves with the pawn), not origin-relative. Not the
distance-from-origin seam.

## Important caveat — finite vs. open world (decides (a) vs. a modifier bug)

Suspect (a) needs **large** absolute coordinates. In an **open** world
the pawn walks to arbitrary distance → (a) fully applies. In a **small
finite** world (radius 2-4 ⇒ ≤ ~200 wu, the key 8/9 case), the ULP gap is
~1e-5 wu — **too small to be visible**. So:

- If the seam reproduces while walking far in an **open** world → (a)
  world-space precision, high confidence.
- If it reproduces in a **small finite** world (≤ a few hundred wu from
  origin) → precision is *insufficient*; the cause is more likely a
  **modifier / tile-grid reference that shifts on recenter** (the
  heightfield gen reads per-tile modifiers from the tile grid whose
  origin is `last_center`; a patch baked against one grid origin abutting
  a neighbor baked against another would crack independent of absolute
  magnitude) or the transient regen-lag (b). This is the one fork the
  static read cannot close without the runtime.

## Settling test (refined — for Jean)

Launch, stand near origin (note seam density), then:

1. **Open world:** walk far. Cracks appear along patch boundaries and
   **worsen with distance**, densest at the LOD-0/LOD-1 ring but present
   on plain boundaries too → **(a) confirmed.**
2. **Return toward origin:** cracks **thin out / vanish** as coordinates
   shrink → confirms the origin-relative (precision) signature and rules
   out a pawn-relative (LOD-topology) cause.
3. **Small finite world (key 8/9):** if the seam is still obvious at ≤200
   wu from origin, precision is too small — redirect to the tile-grid /
   modifier-origin path (the finite-fork above), not (a).

Discriminator in one line: **origin-relative (worse far, resets near
init) ⇒ (a) precision; pawn-relative (tracks the pawn at ~175 wu) ⇒ LOD
classification; transient (fades after settling) ⇒ regen-lag.**

## Remediation directions (NOT executed — parked per ruling)

Recorded only. The defect is *disagreement between neighbors at a shared
edge*, so any fix that makes both neighbors compute the shared edge from
an **identical expression** closes the crack regardless of absolute
magnitude:

1. **Canonical sample lattice** — derive each texel's `world_xz` from a
   *global integer lattice index* (`f32(global_ix) * texel_size`) instead
   of `per-patch origin + offset`. Neighbors share the same integer index
   on the shared edge → bit-identical world position → identical height.
   Most targeted; closes the seam without a broader refactor.
2. **Floating origin / world rebase** — periodically recenter the terrain
   frame on the pawn so bake coordinates stay small. Fixes precision
   globally (also helps distant entities), larger change.
3. **Skirt/curtain geometry** — cosmetic fallback that hides any residual
   crack behind overlapping wall; does not address the root disagreement.

## Confidence

**(a) world-space precision is the leading cause for open worlds** —
mechanism structurally confirmed and it uniquely fits intermittent +
worse-with-distance + resets-near-origin + appears-after-transition (via
the origin reset). The single open fork is the small-finite-world case,
which the refined settling test resolves in one launch. It remains
cosmetic, pre-existing, and blocks nothing; a geometry/precision fix is a
separate stage only if the test confirms the origin-relative signature.
