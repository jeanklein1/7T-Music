# CENSUS_1 — SEED ADDENDUM DISPOSITIONS (seeds 7–10)

Verify, classify, dispose **on paper**. Hands off the tree — no code was touched
by this census. Every claim carries file + line + verbatim quote; inferences are
marked `[INFERRED]`.

**PARENT NOTE.** The addendum (`src/docs/HANDOFFS/OPTIMIZATION 1/
cc_handoff_census_1_addendum.txt`) says "append to [5]". The parent CENSUS_1
handoff — and therefore seeds 1–6 and the parent's section numbering — is **not
in this tree** (searched: `grep -rn "CENSUS_1"` over the repo; only the addendum
and unrelated older censuses match). This file therefore carries the addendum's
four seeds standalone, numbered 7–10 as given, for Jean to fold into the parent
census wherever it lives. Classification scheme used, reconstructed from the
addendum's own usage: **CLASS A** = prose asserting something the code does not
do; **CLASS B** = dead code (buffer/kernel/pipeline with no live consumer);
**CLASS C** = one fact with two+ homes kept true only by manual lockstep.

---

## SEED 7 — `SHADOW_MAP_SIZE`: A DIM FACT WITH TWO HOMES AND NO LOCKSTEP MECHANISM

**VERIFIED.** The two homes:

```
state.hpp:207    constexpr uint32_t SHADOW_MAP_SIZE = 4096;        (under // Lighting)
world.wgsl:3567  const SHADOW_MAP_SIZE: f32 = 4096.0;              (under // --- Shadow constants)
```

Both PCF kernels derive `texel_size` from the WGSL twin: sun
`sample_shadow_pcf` at `world.wgsl:3598`, spot `sample_spot_shadow_pcf` at
`world.wgsl:3715` — both `let texel_size = 1.0 / SHADOW_MAP_SIZE;`. So the
addendum's warning is exact: changing `Dim::` alone resizes the texture and the
atlas tiles while every PCF world-footprint silently shrinks — a *different*
shadow, not a cheaper one.

**Full census of `\bSHADOW_MAP_SIZE\b`, scope `src/**` minus `src/tools`
(the standing exclusion): 8 sites, 3 files.**

| Site | Quote | Class |
|---|---|---|
| `state.hpp:207` | declaration | HOME 1 |
| `world.wgsl:3567` | declaration | HOME 2 |
| `render_passes.hpp:261` | `TILE_W = Dim::SHADOW_MAP_SIZE / 2;  // 2048` | derives |
| `render_passes.hpp:262` | `TILE_H = Dim::SHADOW_MAP_SIZE;      // 4096` | derives |
| `state.hpp:3800` | sun depth texture `desc.size = { Dim::SHADOW_MAP_SIZE, ... }` | derives |
| `state.hpp:3812` | spot depth texture `desc.size = { Dim::SHADOW_MAP_SIZE, ... }` | derives |
| `world.wgsl:3598` | sun PCF `1.0 / SHADOW_MAP_SIZE` | derives (from HOME 2) |
| `world.wgsl:3715` | spot PCF `1.0 / SHADOW_MAP_SIZE` | derives (from HOME 2) |

No third independent home. No independent functional 4096/2048 literal serving a
shadow purpose (LEDGER_1 F-9 re-confirmed). Three **prose** restatements exist:
the trailing `// 2048` / `// 4096` at `render_passes.hpp:261-262` and the atlas
comment `world.wgsl:3657` ("Two 4096×4096 depth textures, each split left/right
into 2048×4096 tiles") — these go stale under any resize.

**THE AGGRAVATION, verified:** nothing keeps the two homes in lockstep — no
`static_assert`, no codegen, and **not even a mirror note**. The tree has a
named convention for exactly this situation and it is absent here:
`TILE_GRID_CAPACITY` (`state.hpp:137-141`) carries *"twin: world.wgsl
TILE_GRID_CAPACITY ... Raise it in BOTH rooms or glaw1/Dawn objects"*, and
`PATCH_CELL_SIZE` / `LIVE_CARD_SIZE` carry explicit *"L3 MIRROR"* notes
(`state.hpp:79, 93-94`). `SHADOW_MAP_SIZE` alone among the mirrored Dim facts is
unmarked on both sides. `[INFERRED]` note: a compile-time assert cannot span
this seam anyway — world.wgsl is runtime-loaded source — so the available
mechanisms are the mirror-note pattern or piping a pipeline-override constant
from `Dim::`.

**CLASS: C** (in spirit even while the numbers agree, per the addendum — and
the census adds: C *without even the mirror note*, the weakest form in the tree).

**DISPOSITION (paper):** PROBE_1 rev2 C2 edits both rooms in lockstep — done on
`claude/probe1-c2`, both homes, one commit. The **permanent** home is a
landing-time ruling for Jean, two arms: (a) pipeline-override constant piped
from `Dim::SHADOW_MAP_SIZE` at pipeline creation — one home, mechanical
lockstep, at the cost of one more override plumb; (b) keep the twin pair and add
the `TILE_GRID_CAPACITY`-pattern mirror note to BOTH declarations — zero code
motion, lockstep stays manual but becomes *named*. Either way the three prose
restatements (`render_passes.hpp:261-262` trailing comments, `world.wgsl:3657`)
should be de-numbered or derived at the same landing.

---

## SEED 8 — "ONE INDIRECT DRAW PER PASS": SEVEN HOMES, ONE OF THEM FALSE, UNRECORDED PROVENANCE, ARCHITECTURE-BEARING

**VERIFIED — the home count: 7 design-side homes in 5 files.** A first sweep
with `grep "one indirect|One DrawIndexedIndirect|only one indirect"` found only
5 — the canonical home escaped because its backticks break the phrase
(`One \`DrawIndexedIndirect\` per`). This is precisely the corollary-boundary
trap the addendum warned about; the count below is from the widened sweep.

The canonical LAW home (1):
```
LAWS.md:43             4. One `DrawIndexedIndirect` per render pass, maximum.        (L2 item 4)
```

States the limit (3):
```
render_passes.hpp:395  // Terrain LOD1 — always direct (Dawn D3D12 limit: only one indirect per pass)
state.hpp:1838         // GPU frustum culling — LOD0 only (Dawn D3D12 limit: one indirect draw per pass).
state.hpp:3106         // GPU frustum culling — LOD0 only (Dawn D3D12 limitation: one indirect draw per pass).
```

The corollary — "budget line" bookkeeping that only makes sense under the
limit (2):
```
render_passes.hpp:342  // One DrawIndexedIndirect budget line free in this pass (L2.4).   (draw_shadow_all)
render_passes.hpp:414  // One DrawIndexedIndirect budget line free in this pass (L2.4).   (main pass draw_table)
```

The L2 pointer (1):
```
world.wgsl:17          //  READ L2 BEFORE adding ... a second indirect draw in one pass.
```

Plus echoes outside code: `audit/LEDGER_1_REPORT.md:855` (which itself says
"three code sites" — now known to be an undercount) and the archival
`src/docs/old docs/the_board_seam_map.md:2613`,
`audit/past reports/CC_AUDIT_REPORT.md:153`.

**ONE OF THE SEVEN IS LITERALLY FALSE TODAY.** The main-pass corollary at
`render_passes.hpp:414` ("One DrawIndexedIndirect budget line free in this
pass") contradicts `render_passes.hpp:374-382`: outdoors,
`use_indirect_terrain()` is true and the main pass **consumes** its one
indirect line on the LOD0 terrain draw — the budget line is not free there.
The shadow-pass twin at `:342` is unconditionally true (that pass issues zero
indirect draws). Treating `:414` as license to add an indirect draw to the
main pass would violate L2.4 outdoors. Do not edit one twin without ruling on
the other.

**A related flag, reported, not fixed:** `cartridge.hpp:1742` —
`static_assert(FrustumCull < ShadowPass, "O-7: frustum cull before the shadow
pass")` — orders the cull before a pass that consumes no cull output (the
shadow VS reads `patch_instances` directly). The ordering assert is broader
than the data dependency; it becomes load-bearing only if a shadow-side
indirect path ever lands.

**Load-bearing: confirmed at both named consequences.** (a) LOD1 draws its full
annulus direct-and-uncullable in the eye pass — `render_passes.hpp:395` is the
stated reason the LOD1 draw never went indirect. (b) The shadow pass culls
nothing and the `:342` comment treats its indirect budget as a scarce resource
(one line, "free"). The actual encoder-call census: **exactly one**
`DrawIndexedIndirect` in the tree (`renderer.hpp:736`,
`draw_patch_terrain_lod0_indirect`), zero `DrawIndirect`.

**What the census cannot settle in-tree, stated plainly:** whether ANY Dawn
version this tree ever ran actually had the limitation. The tree has **no Dawn
version pin** — `CMakeLists.txt:13` points at an unpinned local checkout
(`set(DAWN_DIR "C:/dev/dawn" ...)`) — and `git log -S "one indirect draw per
pass"` dates only the comments' arrival in this repo, not the Dawn they were
written against. The design-side reading (received, not verified here) is that
the WebGPU spec and dawn.json place `drawIndexedIndirect` as an ordinary
per-call encoder method, and that a one-per-CALL batching limit belongs to the
multi-draw FEATURE (value 50) — i.e. the claim as written matches no spec
construct. **PROBE_1 C1's R0 boot log is the instrument that settles the
adjacent live question** (whether the current Dawn exposes multi-draw); whether
the historical limitation ever existed would need the Dawn changelog, which is
outside this tree.

**CLASS: A-or-fossil prose in 7 homes (one already false), ranked at the top beside seed 2**
(per the addendum's own ranking instruction; seed 2 is parent-side, not
visible here). Wrong ruling it produced if false: the draw-submission
architecture itself — LOD1 uncullable, shadow uncled, one cull result serving
one draw (LEDGER_1 H7/F-2 quantify the cost).

**DISPOSITION (paper):** do not edit the comments — the claim must be
**adjudicated, not reworded**. R0 (C1's log) answers whether multi-draw exists
on the shipping Dawn; a five-line held probe issuing TWO plain
`DrawIndexedIndirect` calls in one pass would answer whether the base
limitation exists today. If both come back permissive, all five homes and the
budget-line bookkeeping retire together, and the LOD1/shadow indirect
architecture unlocks (LEDGER_1 L-3/L-7 become schedulable). If the limitation
is real on D3D12, the seven homes should collapse to the ONE law that already
exists (`LAWS.md:43`, L2.4) with pointers — six independent restatements of an
unverifiable constraint, one of them already false, is how fossils breed. The
`:414` falsity should be corrected at the same ruling regardless of which way
the adjudication goes.

---

## SEED 9 — `terrainIndexBuffer_`: DEAD BUFFER + LIVE KERNEL, CARRYING A FALSE "READ EVERY FRAME"

**VERIFIED — the claim:**

```
state.hpp:3132  // Terrain index buffer -- filled once by compute shader, read every frame
state.hpp:3133  terrainIndexBuffer_ = makeBuffer("Terrain IB",
state.hpp:3134      Dim::TERRAIN_INDEX_COUNT * 4,
state.hpp:3135      wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
```

**VERIFIED — the reader set is EMPTY.** With word boundaries:
- `terrainIndexBuffer_` appears at exactly 5 sites: member (`state.hpp:1686`),
  accessor definition (`:2552`), creation (`:3133`, `:3136`), bind-group entry
  (`:5151`, the *writer's* bind group). That is: creation + write plumbing only.
- The accessor `terrain_index_buffer()` has **zero callers** (`grep -rn
  "terrain_index_buffer()"` → only its definition).
- **No `SetIndexBuffer` site in the tree binds it.** All 14 index-buffer binds
  were enumerated; every terrain draw uses `patch_index_buffer()` /
  `patch_index_buffer_lod1()` (`render_passes.hpp:328, 333, 380, 389, 401`;
  `gallery.hpp:1275` — the snapshot pass included).
- The `BufferUsage::Index` flag has therefore never been exercised; the buffer
  is written once by `generate_terrain_indices` (`world.wgsl:7863-7879`, via
  `terrain_mesh_indices` `@group(0) @binding(22)`, `world.wgsl:5457`) and never
  read by anything, on either the GPU or the CPU.

"Filled once by compute shader" — TRUE (the one-shot dispatch,
`cartridge.hpp:502-516`, `"Terrain Index Gen (one-shot)"`). "Read every frame"
— **FALSE.** The patch system replaced the consumer: the live index buffers are
`patchIndexBuffer_` / `patchIndexBufferLOD1_` (CPU-built, `state.hpp:3159-3278`).

**THE RIDE-ALONGS (the full dead cluster, if the buffer is ruled dead):**
`terrainIndexBuffer_` + accessor (`state.hpp:1686, 2552`); the
`generate_terrain_indices` kernel (`world.wgsl:7862-7879`) and its constants'
kernel-side uses (`TERRAIN_MESH_N`/`TERRAIN_MESH_STRIDE`, `world.wgsl:215-216` —
declaration shared, only this kernel consumes them); the storage binding
`@group(0) @binding(22)` (`world.wgsl:5457`); the pipeline (one of the boot FXC
compiles — the ~2.4 s figure is the addendum's, **unverifiable in this
container**: no build here); the Terrain Index Gen layout + bind group
(`state.hpp:4213, 5155`) and dispatch helper (`renderer.hpp` /
`state.hpp:2952` `terrain_mesh_workgroups()`); the one-shot boot dispatch block
(`cartridge.hpp:502-516`); `Dim::TERRAIN_INDEX_COUNT` / `TERRAIN_MESH_VERTS`
(`state.hpp:50-52`) — checked: no other consumer.

**CLASS: B** (buffer + kernel + pipeline + bind group + boot dispatch, no live
consumer) **carrying a CLASS A claim** ("read every frame"), exactly as the
addendum suspected.

**DISPOSITION (paper):** retirement candidate, whole-cluster — the recipe is
mechanical because every member is exclusive to the cluster (g0 binding 22
shares nothing; g1:22 is the bilinear sampler, a different group — not a
collision). Savings if retired: one boot pipeline compile (the FXC
ride-along), one storage buffer (`TERRAIN_INDEX_COUNT × 4` = 1,572,864 B), one
bind group + layout, one binding slot in group 0.

**Three retirement-recipe cautions, so the ruling sees them up front:**
1. **L6 registry governance**: `binding_registry.hpp:16` witnesses the count
   *"96 declarations over 93 slots"* in world.wgsl, and `LAWS.md:113` uses
   **g0:22 as its worked example**. Deleting the `@binding(22)` declaration
   stales both — the removal must update the registry count and the law's
   example together.
2. **Boot gate**: `state.hpp:3136` `if (!terrainIndexBuffer_) return false;`
   makes the dead buffer a boot-success gate inside `createMeshBuffers()`.
   Buffer removal must remove this check; draw-side-only removal changes
   nothing.
3. **Timing brackets**: `cartridge.hpp:498/:516` `t1`/`t2`
   high_resolution_clock stamps bracket the one-shot dispatch — trace their
   consumer before deleting the block (not traced by this census).

The census reports; the campaign decides — note this is the same
`generate_terrain_indices` LEDGER_1 F-1 examined when ruling out per-frame IB
rebuild mechanisms, so retiring it also removes a standing source of confusion
between the dead legacy grid IB and the live patch IBs.

---

## SEED 10 — THE SHADOW-SKIP SENTENCE: THE ENABLER EXISTS, THE SKIP WAS NEVER BUILT

**VERIFIED — the claim, verbatim:**

```
world.wgsl:3251  // Snap pawn XZ to shadow grid for temporal stability.
world.wgsl:3252  // Shadow map content is pixel-perfect between grid crossings,
world.wgsl:3253  // enabling the CPU to skip the shadow pass on idle frames.
```

with `SHADOW_SNAP_SIZE = 2.0` (`world.wgsl:3248`) and the snap itself at
`world.wgsl:3255-3256` (`round(pawn_pos / SNAP) * SNAP` on X and Z).

**VERIFIED — no CPU site exercises the skip.** `render_shadow_pass` has exactly
one caller: `phase_shadow_pass` (`cartridge.hpp:1573-1576`), and it is
**unconditional** — no movement gate, no dirty flag, no grid-crossing
detection. Searched: `grep -rni "shadow_dirty|skip.*shadow|shadow.*skip|
grid_cross"` over `src/cartridges/` and `src/console/` → **zero hits**. The
sun VP is likewise recomputed every frame (`compute_vp`, in the per-frame
compute block). The addendum's METER observation (ShadowPass paying full cost
through resting windows) is consistent with, and now explained by, the code:
the pass runs every frame by construction.

**Precision of the verdict:** the sentence is the (b) case — the *enabling
half* is real (the snap exists and does what it says; between grid crossings
the light VP is bit-stable, so map content would indeed be reusable), and the
*enabled half* (the CPU skip) **was never implemented**. The prose reads as a
description of a shipped capability; what shipped is the precondition. The
call sits in the RENDER_SPINE with a constexpr row gate of `true`
(`cartridge.hpp:1637`), and `render_shadow_pass`'s body has no idle/no-op path
— its only branch selects indoor-atlas vs outdoor.

**A correctness bound on the claim itself `[INFERRED]`:** "pixel-perfect
between grid crossings" holds only for **static** casters. Every dynamic
caster in the drawable table (pawn, spheres, cubes, ribbon, agents) moves the
map's content between crossings, so the honest capability was always
"skip when idle AND nothing shadowed moved" — the invalidation set in
disposition arm (b) is not optional garnish, it is the claim's missing half.

**CLASS: A** (a capability presented as enabled), **LEVER-BEARING**: this
sentence, grown up, is GEOMETRY_2's rate-aligned shadow map — cache
static-caster depth, redraw movers only. The quantified upside is LEDGER_1's
~10 ms shadow pass at rest.

**DISPOSITION (paper):** two arms for the campaign to rank, neither taken here.
(a) The honest-comment fix: reword `:3253` to "which would let the CPU skip the
shadow pass on idle frames (skip not yet implemented)" — CLASS A cured for one
comment's cost. (b) The lever: implement the skip — CPU-side, hash the inputs
that invalidate the map (snapped pawn cell, sun direction, any caster
moved/spawned/despawned, GoL tick, spot-light set) and elide
`render_shadow_pass` when unchanged. Arm (b) subsumes arm (a) and is the
cheapest of the GEOMETRY_2 family because it needs no shader work; its risk is
the invalidation set (a missed invalidator = a frozen shadow), which is why it
belongs to the campaign, not this census.

---

## SCOPE NOTE (relayed, not ruled)

The addendum's optional ruling — adding `src/console/**` to the census scope —
costs ~one file and is supported by this census's own evidence: the adapter
finding (PROBE_1) showed the host layer's prose carries the same disease. All
four seeds above fall inside the existing scope regardless. Jean rules.
