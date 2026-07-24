# UNIFIED_GROUND_1 — BATCH REPORT

Campaign: UNIFIED_GROUND_1 (campaign v2 Stage 5, designed on AUDIT-2;
handoffs `src/docs/HANDOFFS/U 0-6/`). This is the batch's witness stand (U6).

## Base + final

- Base: `b180e2f9491d042f65800351846446e0ac8b51ea` (GROUND_CARD_1_AUDIT2
  tip — Jean's "trunk that suits you" designation; `src/cartridges`
  byte-identical to GROUND_CARD_1 HEAD `1240bece`, the AUDIT-2 map +
  patches in-tree).
- Commit list:

| Commit | Hash | Intent |
|--------|------|--------|
| [U0]  | 71d42c6 | Preflight: anchors a–i all PASS; baseline glaw1 GREEN |
| [U1a] | e639e87 | The two certified corpses (144 + trajectories) — A2_P1 applied hunk-verified |
| [U1b] | 838fe89 | The stale six fold to truth; census re-counted (100 decls / 97 slots) |
| [U2a] | 3b4d580 | Dim::UG_* + WGSL mirror band constants; the band static_assert |
| [U2b] | 226e29f | LOD0 IB rebuild: caps + curtains + re-aimed skirt = 50,688 exact; LOD1 byte-untouched |
| [U3a] | f185c0f | One suppression form + UgVert/ug_decode + ug_cell_lift |
| [U3b] | e3f38e8 | Both patch VS ride the decode + lift; POLICY_TERRAIN_RENDER gains GoL (both rooms) |
| [U4a] | 98982c1 | WGSL retirement (5 entry points + 8 cascade helpers + 6 decls) + registry tombstones |
| [U4b] | 0cb2090 | GoL layout 14→5, derive re-pin, buffers/draw-path/dispatch half out |
| [U5a] | 7a6cd62 | Door extraction (bit-exact) + zone_seed_mask + Zone Mask layout + flush dispatch |
| [U5b] | 2ef49a0 | grid_cells tier column; derive-authored tier size; extent const tombstoned |

(Log commits 92c49e5 + the U6 log carry UNIFIED_GROUND_1_LOG.md.)

## Recount vs expectations (cc6/cc7/cc4 — `_post_ug1` outputs)

| Check | Expectation (u6) | Result | Verdict |
|---|---|---|---|
| cc6 flags | EMPTY | EMPTY | **MATCH** |
| GoL Zone layout | 5 entries: 2 storage / 2 uniform / 1 storage-tex | exactly that | **MATCH** |
| Zone Mask layout | new | 5 entries: 2 storage / 3 uniform | **MATCH** |
| Compute Entity / Placement | unchanged from GROUND_CARD_1 | 12 (5s/5u) / 9 (5s+g1) | **MATCH** |
| cc7 declarations | "write the true count" | **94** (102 pre-batches − 3 H1 − 2 U1a[101+its own] + 2 GC1 − … recipe: 100 at Stage-5 opening − 6 retired in U4a) | recorded |
| cc7 mirror | tombstones parked; zero orphans | zero orphans in BOTH directions | **MATCH** |
| cc4 agent sets | unchanged | identical to the [5b] machine-gate sets | **MATCH** |
| Retired set references | none | NO entry point references {163,164,165,167,168,169} | **MATCH** |
| Dawn witness (U roster + zone_mask family) | ALL GREEN | ALL PIPELINE FAMILIES GREEN, zero module messages | **MATCH** |

## The U3 rest-identity proof (restated, arithmetic)

At rest the card's `.a` is zero everywhere ⇒ `ug_cell_lift` returns 0 ⇒
`lift = 0`. Cap-band verts decode to the same `(vx,vz)` grid positions
as legacy verts at the same location ⇒ same uv ⇒ same heightfield/card/
aura samples ⇒ identical composited `world_pos`. Base-band verts
coincide with their cap twins (lift 0, drop 0) ⇒ every curtain quad is
zero-area ⇒ rasterizes nothing. Skirt copies keep their exact legacy
decode + drop. Every emitted cap quad covers the same surface the legacy
grid quads covered (cell borders lie on mesh vertex lines —
static_assert'd). ⇒ the frame is the pre-batch raster, bitwise, modulo
nothing. Duplicated cell-border verts decode to identical positions ⇒
no cracks between equally-lifted neighbors; unequal neighbors shear at
the transit — by design (the zone walkthrough gate).

The composite color path is bit-exact by construction: [U5a] extracted
the door math without touching the composite's mix chain (the JUDGMENT
logged in the commit — the spec's one-lerp re-plumb would have changed
float order and forfeited the bitwise gate).

## The retirement census diff vs A2-3 (line-for-line)

Everything A2-3 enumerated landed, plus the deltas the rehearsal and
the live tree demanded:

- A2-3a kill list (5 entry points + apply_gol_extrusion_color,
  zone_mesh_gen_cell, zone_emit_quad, zone_sample_baked_terrain_y):
  **removed** exactly.
- A2-3b cascade (query_ground_baked_heightfield,
  contrib_static_base_at, contrib_terrain_waves_at, terrain_height_at):
  **removed** exactly; contrib_pyramids_at verified bake-only
  post-batch (cc4).
- A2-3c bindings/buffers (163/164/165, 167/168/169 + zone mesh buffers
  + Dim::MAX_ZONE_MESH_*): **removed**; registry numbers **parked** as
  tombstones (the 149 precedent) — the A2 census said "removal", the
  house precedent says park; parked.
- A2-3d C++ rows: all named rows/calls removed; the sync/evolve/derive
  half + GolZoneCompute spine row **stay**, derive **re-pinned** to the
  shared GoL layout (the P2 shape). The FIFTH DrawBind site
  (bodies/gallery.hpp — rehearsal lesson #1) included.
- Deltas beyond A2-3: the five entry points' attribute lines cut with
  them (rehearsal lesson #2, zero orphans machine-checked); the freed
  indexed-indirect budget lines named at both draw_table sites.

## Deviations log

1. **[U0]** Base = the audit-branch tip (Jean: "the trunk that suits
   you"); anchors d/g read as definition-site anchors (bare-string
   counts logged in the U0 table).
2. **[U2b]** UG_CAP_STRIDE_C (=5) added C++-side so the emission never
   hardcodes the cap row stride (the WGSL twin is UG_CAP_STRIDE).
3. **[U3a]** pawn_gol_suppression takes pawn_xz (two params) — the
   spec's single-param point_pos() form cannot serve both stages;
   callers pass their stage source (compute: qi.consumer_pos.xz;
   render: render_pawn_pos().xz). Behavior-identical by construction.
4. **[U4a]** The manifold switch's 3u arm needed no repoint — it has
   ridden the texture form since GROUND_CARD_1 [5b].
5. **[U5a]** The one-derivation refactor stops at the door level; the
   composite's mix chain is untouched (bitwise rest identity beats the
   letter of "call the same helper" — the doors ARE one body now).
6. **[U5a] FINDING (gate a)** The handoff's fixed-32 plane stride
   premise is refuted: every consumer indexes the life planes with
   `y * zp.grid_size + x` (dense), uniformly; the CPU seed fills the
   plane flat (prefix-safe under smaller grids). zone_seed_mask mirrors
   the dense convention; zero consumer fixes.
7. **[U5a]** Zone Mask zone pair is Storage, not the spec'd RO — the
   declarations are `var<storage, read_write>` (the H-batch precedent
   the spec itself cites for zone_life applies to both).
8. **[U5b] (gate b)** zone_config is derive-authored only → no host
   edit. The host's spawn-selection spacing still assumes 100 wu
   (conservative for smaller tiers) — Jean-tunable follow-on, not a
   correctness issue.
9. **ABORTs**: none.

## Encoding sweep

LF-only, no BOM across every touched file; the world.wgsl FXC banner
block byte-untouched. glaw1 GREEN at base, after every handoff, and at
batch end.

─────────────────────────────────────────────────────────────────
JEAN'S GATE LIST (Windows, one session):
[ ] Windows build + boot. FXC watch: the unified decode in BOTH
    patch VS (select-only — A2-5 blessed the shape on Tint; FXC's
    word lands here), zone_seed_mask (standalone-writer family),
    and the ABSENCE of the retired pipelines.
[ ] REST IDENTITY, bitwise: stills A/B vs pre-batch at rest — the
    arithmetic proof made pixels.
[ ] THE ZONE WALKTHROUGH: a Conway zone's birth (ground cracks
    upward — the transit shear, no coplanar shimmer); plateau
    joins between equal neighbors (seamless); the pawn bow-wave
    (now VISIBLE in the terrain — walk into a zone); cells wear
    the ground's own colors + tint; mask silhouettes (smooth cells
    inside a zone stay flat — organic coastline shapes); tier
    sizes (small pulse zones ~25 wu; big Conway 100 wu); LOD1
    distance look (soft steps, no curtains — by design).
[ ] The five carried-over batch gates (idle rig / debug eye /
    motion motif / walkabout / frustum full-window) — one pass
    covers both campaigns.
─────────────────────────────────────────────────────────────────

## Seeds

AUDIT-3 = the same instrument set post-merge + the parked-number census
(29/30, 149, 163–165, 167–169, and U5b's ZONE_DERIVE_EXTENT). Then
Stage 6 (the true-band swap) and Stage 7 (contact collision) per
campaign v2 — both handoff-ready on this tree.
