> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# THE GROUND CARD — Campaign v2 (STAMPED)

Status: **approved by Jean, 2026-07-21** — supersedes ground_card_campaign_v1.md.
Trunk: **CLOSURE_GPU** @ `bd405d92` (hash-anchored; the SWEEP name in v1's
handoff was a naming error — new law: *handoffs anchor tree identity by
hash, never by remembered branch name*). Audit: AUDIT_REPORT.md on
`claude/ground-card-audit-exb0m5`, artifacts under `audit/`. The design
snapshot matched HEAD on every load-bearing line (29/30 tombstones,
cmg 190/191, TILE_GRID_CAPACITY 1024) — the CC-1 delta measured the
mis-named reference, not the design's inputs.

**Preconditions to Stage 2 (the only ones):**
1. The Windows witness run — `audit/probe_dawn_witness.mjs` in Chrome on the
   design machine (the FXC half of CC-5), plus the
   `readonly_and_readwrite_storage_textures` feature query there (decides
   paint-card v2 residency later; nothing in this campaign depends on it).
2. glaw1 + boot as ever (audit baseline: GREEN, 24.7 s).

---

## §1 The one sentence

Everything this campaign touches lives on the same address — world
position — and the campaign's content is giving that address one
substrate. Geometry, color, life, spawning, and collision become channels
of one ground, not five systems that each rediscovered it.

Sorting law: position-indexed → cards; identity-indexed → buffers.
Continuity law: goals may leap, values may only walk, state outlives its
law. Constitutional route: all time-variation is CPU-glided; the GPU holds
static seeds and stateless evaluators.

## §2 Decisions in force (the ledger)

| decision | status |
|---|---|
| Live Card: R = waves+pulses Δh, G/B = ∂x/∂z, A = raw GoL lift | stamped |
| Card residency: 512² RGBA16F over 800 wu, centered on config.lod_point, window origin snapped to the 3.125 cell grid (nearest fetch is cell-exact for .a), fully rewritten per frame, writer CALLS existing evaluators | stamped |
| Card addresses: g0::live_card_write = 31, g1::live_card_read = 34 | CC-7 certified free, both rooms |
| THE UNIFIED GROUND (GoL): retire zone extrusion mesh; re-topologize terrain into cap tiles + cell-granular curtains; land rises wearing its own colors | stamped |
| GoL mask: written into height_factor at derive (bake's own doors re-run, one-derivation law); dynamic form waits for Layer E | stamped |
| GoL size: quantized in cells — grid_size ∈ {8..32}, extent = grid_size × 3.125; >100 wu = stride edge, out of scope | stamped |
| True-band terrain animation: writer content swap, bands 0–3 live, band 4 static by ruling; gen-1 wires (band_blend / band_phase_origin) re-aim from overlay to the real instrument | stamped, Stage 6 |
| Collision, the Profile Law: contact via tier-registry radius/mass columns + post-step gather; pawn heavy (soft nudge, not immovable); sphere-vs-walker and cube-vs-pawn included; cube-vs-cube deferred | stamped, Stage 7 |
| Mode-field couplings (drift / morph / breath), Layer E, tile card C, presence card, paint card, atmosphere card | designed destinations — next campaign (§9) |

## §3 Audit absorption (what changed between v1 and v2)

**Corrections taken:** GoL pier binding is LIVE via
`zone_gol_mesh_gen → zone_sample_baked_terrain_y →
query_ground_baked_heightfield → contrib_static_base_at →
structure_height_at` — its removal moves to Stage 5, where it is
pre-certified (all life kernels validate clean without it). Aura tile_grid
is LIVE (`compute_pawn_aura → gol_composite_cell_color →
evaluate_cell_fields`) — cleanup seed withdrawn; the aura kernel is hereby
a named Layer C/E customer. The agents are FULLY ANALYTIC today —
`sample_terrain_y_at`'s only readers are photographer and placement; the
entries[13] comment was fossil. Stage 4 therefore includes wiring the
baked path into the agent kernels.

**Gifts taken:** the dead-entry harvest (CC-3 cross, each Dawn-verified
removable): Compute Entity {trajectories 101, photo_heightfield 145,
photo_sampler 146, patch_grid 152}; Placement {agent_state 60,
photo_patch_instances 144}; Frustum Cull {fc_agents 60, fc_camera 80};
Photographer {144}; Compute Texture {nearest_sampler 23}. A LIVE BUG:
`dispatch_frustum_cull` hardcodes 4 workgroups (256 threads) against
MAX_ACTIVE_PATCHES 289 — slots 256–288 never culled at full window;
correct count 5. Banner "108" is stale: 102 declarations / 97 slots,
zero mirror orphans, five documented fc_ aliases.

**Confirmations banked:** funnel theorem tree-wide (CC-2, CC-4's 25
verbatim errors — all five evictees × all five agent kernels, compute_vp
empty); budgets machine-counted (CC-6; Ribbon corrected to 3/2,
head_poses); radians-per-wu verbatim (Nyquist analysis stands: worst band
λ ≈ 5.2 wu ≥ 3.3 texels at 1.5625 wu/texel); writer patterns compile in
single-digit ms on Tint; world.wgsl clean in 116–336 ms.

## §4 Consumer rewiring (per policy, post-Stage-4)

`card(xz)` = one textureSampleLevel of the live card; `baked(xz)` = the
sample_terrain_y_at path (patch_grid + heightfield array — NEWLY wired
into the agent kernels); `supp(d)` = the existing distance smoothstep.

| consumer | composition |
|---|---|
| placement | baked + card.**a** (ruling preserved: no waves/pulses) |
| patch VS / shadow patch VS | heightfield + card.**r** (+aura); normals from .yz + card.**gb**; **+ card.a·(1−supp)** at Stage 5 |
| entity/wall VS ×12 + zone-era sites | atlas/heightfield + card.**r** (replaces the 6-wave loop) |
| WALKER (pawn resolve) | baked + card.r + card.a·(1−supp) + self-aura scalar |
| WALKER_TILT (camera) | baked + card.r + card.a·(1−supp) |
| WALKER_AGENT / FLYER (agents, sphere, cubes, floater-arch) | baked + card.r + card.a + aura-tex |
| CELESTIAL | 0 (unchanged) |

The analytic query_ground_* bodies remain the manifold's definition; the
switch arms rewire. The bake stays analytic (it fills the baked card).

## §5 Binding arithmetic (audit-grade; storage/uniform per stage)

| layout | today (CC-6) | after S1 | after S4 | after S5 |
|---|---|---|---|---|
| Compute Entity | 9/7 (+1 tex, 1 smp) | 7/7 (0 tex, 0 smp) | **5/5** (+heightfield tex, sampler, patch_grid back as LIVE; card via g1) | — |
| Compute Texture (g1) | 1 tex / 2 smp | 1/1 | **2/1** (+card read) | — |
| Entity Placement | 9/1 | 7/1 | **5/1** (+card read; sampler present) | — |
| Frustum Cull | 6/1 | **4/1** + dispatch fix 4→5 | — | — |
| Ribbon | 3/2 | **2/1** | — | — |
| Photographer | 6/2 | **5/2** | — | — |
| GoL Zone | 7/4 | — | — | **2/2** (mesh trio, pier, zone_patch_instances, zone_heightfield+sampler retire; tile_grid+pyramids uniforms drop — their only reach was the mesh path's fallback chain) |
| Render Entity | 8 VS / 7+1 FS | unchanged | unchanged | unchanged — Unified Ground uses the card's nearest fetch, so the Grade-B zone_params VS widen is AVOIDED; render relief arrives with C/E |

Banner law 10/12 per stage: satisfied everywhere with margin restored on
every wall. Card memory: 2 MB. Patch mesh grows 4,481 → 10,496 verts,
indices ≈ 2× (one shared instanced mesh — trivial bytes).

## §6 The staged arc

**Stage 0 — DONE.** Audit delivered; preconditions above outstanding.

**Stage 1 — the sweep.** Ribbon dead pair; the harvest (CE: trajectories +
the three future-live entries per strict YAGNI — removed now, re-added
live at S4; Placement 60/144; Cull 60/80; Photographer 144; CT
nearest_sampler); **frustum-cull dispatch fix (4→5)**; text fold: 225-labels,
banner 108→actual, ribbon terrain_y residue (field removal deferred — VS
input stride — comment-only now), spine-banner staleness. Aura seed
WITHDRAWN. Gate: glaw1 + boot + idle rig. Disclosure: the cull fix may
visibly restore patches at full window — a correctness delta.

**Stage 2 — card plumbing, instrument first.** Texture + views + registry
(31/34) + writer layout/pipeline (signal, config, zone_config, zone_life,
storage-tex — the zone pair's future sole home) + spine row LiveCardWrite
between R9/R10 + two witnesses (card-before-DispatchCompute,
card-before-PlacementCorrection; preserves today's pre-evolve zone read,
R10<R13) + writer at OVERLAY parity + TERRAIN_DEBUG_VIEW slot painting the
card. Nothing reads it yet. Gate: build + boot + debug view under music.

**Stage 3 — render rewire.** The 14 VS sites + zone-era sites: card sample
replaces the wave loop; normals take .gb. Gate: rest bit-identity (card
zeros at rest by construction) + motion motif review + debug cross-check.

**Stage 4 — compute rewire + evictions.** Three contrib call-sites → §4
compositions; manifold switch arms follow; **wire baked path into agent
kernels** (re-add 145/146/152 as live); evict piers + zone pair from CE,
zone pair from Placement; tile_grid + pyramids uniforms out of CE. Gate:
walkabout (pawn step/slide, camera clamp, agent snap, cube kite/hover,
sphere clearance) + cc6 recount matching §5.

**Stage 5 — THE UNIFIED GROUND.** Topology: per-cell 5×5 cap tiles +
cell-border curtains generalizing the patch skirt (patch perimeter absorbs
into it; 41 verts/cell, 10,496/patch; index-gen + VS vertex_index decode
in lockstep, curtains degenerate at rest). Lift: card.a nearest +
per-vertex supp — the bow-wave becomes visible; the land rises wearing its
own colors (terrain FS is the one color law; tint stack unchanged).
Retirements: zone-mesh kernels ×2, entry points ×3, mesh buffer trio,
pier entry (pre-certified), zone_patch_instances, zone_heightfield +
sampler, apply_gol_extrusion_color, drawable-table zone row, the indirect
slot returned, suppression triple → pair. Mask: derive writes vocabulary
predicate into height_factor. Size: grid_size per tier ∈ {8..32}. LOD1:
old topology + card.a lift, no curtains (distance hides the slope).
Gates: bitwise rest identity; zone walkthrough (birth, transit shear,
pawn bow-wave, plateau joins, mask silhouettes); retirement census.

**Stage 6 — the true-band swap.** Writer R becomes
Σ(bands 0–3) gate(blend_driven) × band_act × (moving − frozen), computed
by the existing evaluator's own pieces at texel centers (one derivation;
phase-origin anti-teleport inherited; the seeded activity field shapes
WHERE a woken band breathes, music decides THAT it breathes); pulses stay
in R; gradients by the bake's two-pass shared-tile pattern; band 4 static
by ruling. Gen-1 wires re-aim; rest = today's stillness, bit-frozen.
Overlay retirement joins the cleanup fold (call sites already collapsed at
S3). Gate: motif review under music + rest stillness law. Separate
commit; cleanly bisectable.

**Stage 7 — contact collision (parallel-eligible after S4).** Profile
columns (radius, mass) beside the tier registries (agent_tier_gains
already bound in the very kernels); post-step gather over pawn + 32
agents; capped spring impulse along the pair axis, mass-ratio weighted,
pawn heavy; sphere-vs-walker + cube-vs-pawn through the same columns;
cube-vs-cube deferred. Optional pad-float individuality (one free
AgentState pad) — follow-on, not this cut. Disclosed softness: one-frame
dispatch asymmetry, absorbed by the springs. FXC shape: uniform-bounded
gather outside the resolve chain (flock2d precedent); optional CC probe.
Gate: walkabout + feel review.

## §7 Behavior deltas (the disclosure register)

1. Pulse-shaded normals (upgrade). 2. Walker/camera/agents on the sampled
surface the pixels stand on (agreement closes; agents additionally move
analytic→baked at S4 — same surface as rendered, the deeper agreement).
3. One evaluation moment for the deformation field (VS/compute clock
unify). 4. Rest identity bit-exact at every stage gate. 5. Window
covenant: card covers ±400; all live consumers inside EXIST; edge-clamp
named. 6. S5: bow-wave visible; lifted cells wear terrain color (the ask,
verbatim); transit shear replaces coplanar shimmer. 7. S1: frustum fix
may restore never-culled patches at full window. 8. S6: terrain breathes
its own low bands; overlay accent retires. 9. S7: bodies nudge; player
barely feels it.

## §8 Risk register

| risk | net |
|---|---|
| FXC | on-device witness precondition; writer = proven standalone pattern; no new branching in resolve; textureSampleLevel everywhere incl. VS; banner laws untouched |
| baseline drift | hash-anchored handoffs (new law); Stage-1 branch cut from CLOSURE_GPU @ bd405d92 or its verified descendant |
| hidden consumers | funnel PASS + Dawn enumeration; any new removal repeats the probe pattern |
| pixel regression | rest bit-identity per stage + debug view + motif gates |
| topology (S5) | duplicated-border decode identity at rest is the correctness proof; index-gen and VS decode land in one lockstep commit |

## §9 Named destinations (the coupling campaign, next)

Over the unified substrate: mode-field **drift / morph / breath** (the
morph as the pre-modulation hint's visual twin); **Layer E** — the field
card (per-fragment live-recolor collapse; dynamic GoL mask — coastlines
and living land breathe together; aura kernel as customer; render-side
tile_grid relief); **tile card C**; **presence card** (bodies weigh;
spawn samples gaps); **paint card v2** (persistent authored fields;
residency per the FXC feature query; idleness = decay to the derived
field); **3D atmosphere card**. Open rulings that remain open, tracked:
E timing · C timing · pad-float individuality · cube-vs-cube.

## §10 Handoff plan

Implementation session: CC handoffs H1..Hn per stage, FIND/REPLACE form,
hash-anchored, verify-first, one build per batch, commits sized for
bisection. Stage gates as listed. Seed docs: this file + AUDIT_REPORT.md
+ the audit artifact index + session_bridge_terrain_era.md +
7t_program_theory_v3.md.
