# ARENA_0 — MESH-ARENA RECON (READ-ONLY)

Answers the seven questions TETRIS ARENA_0 asks of the five mesh-gen
families — arch, column, palm, cactus, blade — ahead of a future shared-arena
ruling. **Zero source edits.** No recommendation beyond the GO/NO-GO matrix;
Jean rules the arena.

Base: `cf57353` on `claude/tetris-handoff-setup-eugf7e`, whose source-file
content for every file cited here is `ed982e0` (master) plus TETRIS
WALLET_0, the bind-group HOTFIX and ORB_V. None of the three touched a
mesh-gen family except WALLET_0's demotion of the two *occupier windows*
onto `columnMeshParamsBuffer_` / `archMeshParamsBuffer_`, which is called
out in Q1 because it changes those two buffers' usage flags.

Every claim carries file + symbol. Line numbers are hints; the symbols are
the authority (P2). Absences were verified over whole files, untruncated
(P11).

---

## Q1 — The fifteen buffer creation sites

All fifteen are in `state.hpp`, in `createArchMesh` / `createColumnMesh` /
`createPalmMesh` / `createCactusMesh` / `createBladeMesh`, all through the
same `makeBuffer(label, bytes, usage)` helper.

| species | member | bytes | usage flags |
|---|---|---|---|
| arch | `archVertexBuffer_` | `AMG_TOTAL_VERTICES * sizeof(ArchVertex)` | `Vertex \| CopyDst \| Storage` |
| arch | `archIndexBuffer_` | `AMG_TOTAL_INDICES * sizeof(uint32_t)` | `Index \| CopyDst \| Storage` |
| arch | `archMeshParamsBuffer_` | `MAX_ARCH_INSTANCES * sizeof(GPUArchMeshParams)` | `Storage \| Uniform \| CopyDst` |
| column | `columnVertexBuffer_` | `CMG_TOTAL_VERTICES * sizeof(ArchVertex)` | `Vertex \| CopyDst \| Storage` |
| column | `columnIndexBuffer_` | `CMG_TOTAL_INDICES * sizeof(uint32_t)` | `Index \| CopyDst \| Storage` |
| column | `columnMeshParamsBuffer_` | `MAX_COLUMN_INSTANCES * sizeof(GPUColumnMeshParams)` | `Storage \| Uniform \| CopyDst` |
| palm | `palmVertexBuffer_` | `PALMG_TOTAL_VERTICES * sizeof(ArchVertex)` | `Vertex \| CopyDst \| Storage` |
| palm | `palmIndexBuffer_` | `PALMG_TOTAL_INDICES * sizeof(uint32_t)` | `Index \| CopyDst \| Storage` |
| palm | `palmMeshParamsBuffer_` | `MAX_PALM_INSTANCES * sizeof(GPUPalmMeshParams)` | `Storage \| CopyDst` |
| cactus | `cactusVertexBuffer_` | `CACTUSG_TOTAL_VERTICES * sizeof(ArchVertex)` | `Vertex \| CopyDst \| Storage` |
| cactus | `cactusIndexBuffer_` | `CACTUSG_TOTAL_INDICES * sizeof(uint32_t)` | `Index \| CopyDst \| Storage` |
| cactus | `cactusMeshParamsBuffer_` | `MAX_CACTUS_INSTANCES * sizeof(GPUCactusMeshParams)` | `Storage \| CopyDst` |
| blade | `bladeVertexBuffer_` | `BLADEG_TOTAL_VERTICES * sizeof(ArchVertex)` | `Storage \| Vertex \| CopyDst` |
| blade | `bladeIndexBuffer_` | `BLADEG_TOTAL_INDICES * sizeof(uint32_t)` | `Storage \| Index \| CopyDst` |
| blade | `bladeMeshParamsBuffer_` | `MAX_BLADE_INSTANCES * sizeof(GPUBladeClusterMeshParams)` | `Storage \| CopyDst` |

**Uniform across all five, and this is the arena's foundation:** every
vertex buffer is `Vertex + Storage + CopyDst`; every index buffer is
`Index + Storage + CopyDst`; every params buffer is `Storage + CopyDst`.
The blade pair spells the same set in a different order — a spelling
difference, not a flag difference.

**The one asymmetry**, and it is not a mesh-gen fact: `archMeshParamsBuffer_`
and `columnMeshParamsBuffer_` additionally carry `Uniform`, added by TETRIS
WALLET_0 so `occupier_amg` / `occupier_cmg` — the room family's read-only
windows onto these same buffers — can ride the uniform address space. The
mesh-gen kernels still bind them as `ReadOnlyStorage`. Palm, cactus and
blade have no occupier row and so no second usage.

The `Dim::` constants, all in `state.hpp`:

| species | slots | verts/slot | indices/slot |
|---|---|---|---|
| arch | `MAX_ARCH_INSTANCES` 16 | `AMG_MAX_VERTS_PER_SLOT` 2000 | `AMG_MAX_INDICES_PER_SLOT` 7500 |
| column | `MAX_COLUMN_INSTANCES` 32 | `CMG_MAX_VERTS_PER_SLOT` 1500 | `CMG_MAX_INDICES_PER_SLOT` 6000 |
| palm | `MAX_PALM_INSTANCES` 24 | `PALMG_MAX_VERTS_PER_SLOT` 1200 | `PALMG_MAX_INDICES_PER_SLOT` 6000 |
| cactus | `MAX_CACTUS_INSTANCES` 20 | `CACTUSG_MAX_VERTS_PER_SLOT` 1500 | `CACTUSG_MAX_INDICES_PER_SLOT` 7998 |
| blade | `MAX_BLADE_INSTANCES` 32 | `BLADEG_MAX_VERTS_PER_SLOT` 500 | `BLADEG_MAX_INDICES_PER_SLOT` 1998 |

Column's 32 slots are two families on one buffer: `MAX_COLUMN_ONLY` 16
classical columns at slots 0–15, `MAX_ANTENNA_ONLY` 16 antennas at 16–31,
with `ANTENNA_SLOT_OFFSET` 16 and a `static_assert` holding the sum. An
arena inherits that sub-partition unchanged.

---

## Q2 — Render-side vertex layout per species

**All five are byte-identical, and not merely equal — they are the SAME
OBJECT.** `renderer.hpp` builds one `wgpu::VertexBufferLayout archVBL` and
passes `&archVBL` to all five `makeEntity(…)` calls:

```
makeEntity("arch",   "Catenary Arch (Rasterized)",   Entry::ARCH_VS,   &archVBL, CullMode::Back, archPipeline_)
makeEntity("column", "Generative Column (Rasterized)", Entry::COLUMN_VS, &archVBL, CullMode::None, columnPipeline_)
makeEntity("palm",   "Palm Tree (Rasterized)",       Entry::PALM_VS,   &archVBL, CullMode::None, palmPipeline_)
makeEntity("cactus", "Cactus (Rasterized)",          Entry::CACTUS_VS, &archVBL, CullMode::None, cactusPipeline_)
makeEntity("blade",  "Blade Cluster (Rasterized)",   Entry::BLADE_VS,  &archVBL, CullMode::None, bladePipeline_)
```

`archVBL`: `arrayStride = 40`, `stepMode = Vertex`, 4 attributes —

| # | format | offset | shaderLocation | `ArchVertex` member |
|---|---|---|---|---|
| 0 | `Float32x3` | 0 | 0 | `float pos[3]` |
| 1 | `Float32x3` | 12 | 1 | `float normal[3]` |
| 2 | `Float32x3` | 24 | 2 | `float color[3]` |
| 3 | `Float32` | 36 | 3 | `uint32_t arch_index` |

Note attribute 3: the C++ member is `uint32_t` and the vertex format is
`Float32`, so the vertex stage receives the index channel's bits
reinterpreted as float, not converted. This is the MOSAIC_1 encoding
channel (`enc = mosaic_seed * 64 + slot`) and it is deliberate — but it is
a correspondence held by nothing in the tree, and an arena that changes
how vertices are addressed will be reading it. Flagged, not judged.

The five differ in exactly two things, neither of them the layout: the
vertex **entry point** (`ARCH_VS` / `COLUMN_VS` / `PALM_VS` / `CACTUS_VS` /
`BLADE_VS`) and the **cull mode** (arch `Back`, the other four `None` —
single-sided quads).

---

## Q3 — Index format and today's `DrawIndexed` arguments

Index format is `wgpu::IndexFormat::Uint32` for all five, set inside each
`draw_*` in `renderer.hpp`.

The five draw functions are the same function five times over:

```
pass.SetPipeline(<species>Pipeline_);
pass.SetVertexBuffer(0, vertexBuffer);
pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
pass.DrawIndexed(indexCount);
```

`DrawIndexed(indexCount)` takes Dawn's defaults for every remaining
argument, so **`firstIndex` = 0 and `baseVertex` = 0 — confirmed, as the
handoff expected**, along with `instanceCount` = 1 and `firstInstance` = 0.
Each species is drawn as a single non-instanced indexed draw over its own
buffers.

Callers, all five in `drawable_table.hpp`, each passing that species' own
vertex buffer, index buffer and count:

```
r.draw_arch  (p, g.arch_vertex_buffer(),   g.arch_index_buffer(),   g.arch_index_count());
r.draw_column(p, g.column_vertex_buffer(), g.column_index_buffer(), g.column_index_count());
r.draw_palm  (p, g.palm_vertex_buffer(),   g.palm_index_buffer(),   g.palm_index_count());
r.draw_cactus(p, g.cactus_vertex_buffer(), g.cactus_index_buffer(), g.cactus_index_count());
r.draw_blade (p, g.blade_vertex_buffer(),  g.blade_index_buffer(),  g.blade_index_count());
```

**indexCount source.** `archIndexCount_` and its four siblings are plain
`uint32_t` members of `GPUState`, written only through
`set_<species>_index_count`. Every write is in `bodies/grounded.hpp`, in
`prepare_<species>_mesh_gen`, and every one has the same shape — a
high-water scan over the species' slots:

```
c->gpuState_.set_arch_index_count(anyActive
    ? (maxSlot + 1) * Dim::AMG_MAX_INDICES_PER_SLOT : 0);
```

So the drawn range is always a **prefix** `[0, (maxSlot+1) * INDICES_PER_SLOT)`
of the species' own index buffer, never a middle window. `prepare_column_mesh_gen`
scans both sub-families and takes the max across them, which is why it uses
`ANTENNA_SLOT_OFFSET`. The five teardown paths in the same file set all five
counts to 0.

**This is the fact that matters most for an arena.** A shared arena moves
each species to a non-zero base, and a prefix-shaped count is precisely
what does NOT survive that move unchanged: the draw would need either a
`firstIndex` of the species' arena base (and a `baseVertex` to match), or
per-species offset buffer views. Today's `DrawIndexed(indexCount)` with
both defaulted to 0 is correct only because each species owns its buffer
from byte 0. Nothing in the draw path is offset-parameterized — unlike the
kernels (Q4).

---

## Q4 — Kernel write addressing

**Confirmed arena-favorable, and more strongly than the handoff's Table F
note suggested.** The base offsets are not merely "flowing as callee
parameters"; every one of the five kernels derives its base from the slot
index in its first three statements, and the derivation is the same
expression in all five:

| kernel | base derivation |
|---|---|
| `arch_mesh_gen` | `let slot_vb = slot * AMG_MAX_VERTS_PER_SLOT;`<br>`let slot_ib = slot * AMG_MAX_INDICES_PER_SLOT;` |
| `column_mesh_gen` | `let slot_vb = slot * CMG_MAX_VERTS_PER_SLOT;`<br>`let slot_ib = slot * CMG_MAX_INDICES_PER_SLOT;` |
| `palm_mesh_gen` | `let vb_base = slot * PALMG_MAX_VERTS_PER_SLOT;`<br>`let ib_base = slot * PALMG_MAX_INDICES_PER_SLOT;` |
| `cactus_mesh_gen` | `let vb_base = slot * CACTUSG_MAX_VERTS_PER_SLOT;`<br>`let ib_base = slot * CACTUSG_MAX_INDICES_PER_SLOT;` |
| `blade_cluster_mesh_gen` | `let vb_base = slot * BLADEG_MAX_VERTS_PER_SLOT;`<br>`let ib_base = slot * BLADEG_MAX_INDICES_PER_SLOT;` |

`slot` is `gid.x` in all five. Those bases then flow downward as callee
parameters — `abs_idx` into the per-vertex writers
(`amg_write_vertex` / `palmg_write_vertex` / `cactusg_write_vertex` /
`bladeg_write_vertex`, each computing `abs_idx * <SPECIES>_FLOATS_PER_VERTEX`),
and `vb_start` / `ib_start` into the arch and column sub-mesh emitters,
where the local cursors `var vi = vb_start` and `var ii = ib_start` walk
forward.

**Where a base would come from in an arena:** exactly one place per
species, the `slot * <CONST>` line above. An arena needs `arena_base +
slot * <CONST>`, with `arena_base` a per-species constant or a config
field. Nothing else in any kernel names an absolute address.

Two details a merge must carry:

- The **inactive path** writes degenerate indices, and it writes them
  pointing at the slot's own first vertex, not at 0 —
  `bladeg_indices[ib_base + i] = vb_base;` carries the in-tree comment
  `// NOT 0u!`, and all five do the same. In an arena `vb_base` becomes an
  arena-absolute vertex index, which is what the degenerate triangle
  should point at anyway; the invariant survives, but it is the kind of
  line that gets "simplified" to 0 by someone who has not read the comment.
- `arch_mesh_gen` is the only 2D kernel: it takes `sub_mesh = gid.y` over
  4 sub-meshes, and its inactive path zeroes only `AMG_MAX_INDICES_PER_SLOT / 4`
  — its own quarter. An arena keeps that quartering.

---

## Q5 — The five params structs side by side

Sizes confirmed as the handoff states: **80 / 128 / 128 / 128 / 80 B**,
counting the WGSL fields (all members are 4 B scalars, so size = 4 × field
count: arch 20, column 32, palm 32, cactus 32, blade 20).

**Present in all five (the common core, 7 fields):**
`center_x`, `center_z` — planar placement; `is_active` — the liveness gate
every kernel tests first; and a 3-float body colour, though it is spelled
differently in each (`color_r/g/b` in arch and column, `trunk_*` in palm,
`body_*` in cactus, `blade_*` in blade).

**Present in four of five:** a burial depth (`burial` — arch, column, palm;
cactus and blade have none). Present in three: a lean pair (`lean`,
`lean_dir` — palm, cactus; arch has `rotation` instead, which is the same
idea with one field).

**Divergence, by species:**

| species | its own fields | shape of the divergence |
|---|---|---|
| arch | `rotation`, `half_span`, `rise`, `depth`, `thickness`, `pier_height`, `catenary_a`, `segs_u`, `segs_v` | a catenary curve + its piers; two-axis tessellation |
| column | `height`, `shaft_radius`, `taper`, `entasis`, `base_height/overhang`, `capital_height/overhang`, `base_layers`, `capital_layers`, `segs_around`, `shaft_rings`, `tier`, and **nine drum-colour floats** | by far the widest; the 9 antenna drum colours are a third of the struct and exist for half its slots |
| palm | `base_r`, `top_r`, `bark_rings`, `bark_depth`, `frond_count/len/width/droop/arch`, `crown_spread`, `crown_skirt`, `trunk_segs`, `frond_segs`, plus `frond_*` and `aged_*` colour triples | trunk + crown as two sub-bodies, each with its own colour |
| cactus | `radius`, `taper`, `ribs`, `rib_depth`, `cap_round`, `arm_count`, `arm_height/length/radius/curve`, `trunk_segs`, `arm_segs`, `seed`, plus a `rib_*` colour triple | body + arms as two sub-bodies |
| blade | `blade_count`, `blade_h`, `blade_h_var`, `blade_w`, `splay`, `curve`, `twist`, `taper`, `blade_segs`, `seed`, plus an `aged_*` triple | the only one whose "mesh" is a *population* of sub-bodies rather than one body |

**Type divergence worth naming:** the segment counts are `u32` in arch,
column and blade but **`f32`** in palm and cactus (`trunk_segs`, `arm_segs`
are `u32`, but `ribs`, `frond_count`, `arm_count`, `blade_count` are
`f32`). A single arena params struct would have to pick, and the picking is
a behaviour change, not a layout change.

**Two families carry a MOSAIC_1 encoding field and three do not:** arch has
`mosaic_seed` (the GROWTH 64 → 80 that made it 80 B) and column repurposed
`_pad128_0` for the same; palm, cactus and blade have no mosaic channel.
Cactus and blade instead carry a plain `seed`. These are not the same
field and must not be unified by name.

Padding tails: arch `_pad80_0/1/2`, column `_pad128_1/2`, palm `_pad0/1/2`,
cactus `_pad0`…`_pad6`, blade `_pad0`. Every struct is already padded to
its 16-multiple, which is why WALLET_0's uniform demotion of the arch and
column params cost no element-type change.

---

## Q6 — Arithmetic: what an arena would have to allocate

`ArchVertex` is 40 B (`pos[3]` + `normal[3]` + `color[3]` + `arch_index`);
indices are `uint32_t`, 4 B.

| species | slots | vertices | vertex bytes | indices | index bytes | params bytes |
|---|---|---|---|---|---|---|
| arch | 16 | 32,000 | 1,280,000 | 120,000 | 480,000 | 1,280 |
| column | 32 | 48,000 | 1,920,000 | 192,000 | 768,000 | 4,096 |
| palm | 24 | 28,800 | 1,152,000 | 144,000 | 576,000 | 3,072 |
| cactus | 20 | 30,000 | 1,200,000 | 159,960 | 639,840 | 2,560 |
| blade | 32 | 16,000 | 640,000 | 63,936 | 255,744 | 2,560 |
| **TOTAL** | **124** | **154,800** | **6,192,000** | **679,896** | **2,719,584** | **13,568** |

- **Vertex arena: 6,192,000 B = 5.905 MiB.**
- **Index arena: 2,719,584 B = 2.594 MiB.**
- Combined: 8,911,584 B = 8.499 MiB. Params, if also merged: 13,568 B.

Against `maxStorageBufferBindingSize` — 128 MiB, the figure the program
itself cites in `console/console.hpp`'s PORT_5d census ("the largest
storage binding is Live Card Scratch at ~3.3 MiB of 128 MiB") — the vertex
arena is **4.61%** and the index arena **2.03%**; together **6.64%**.

**Stated anyway, as the handoff asks: this is nothing.** The largest single
arena a full merge would create is under twice the size of a binding the
program already allocates, and both sit two orders of magnitude under the
cap. Size is not a precondition for the arena; it is not even a
consideration. If the arena is refused it will be refused on the draw path
(Q3) or on the params divergence (Q5), never on bytes.

---

## Q7 — Dispatch shapes, and whether anything couples the five

| species | dispatch | `@workgroup_size` | shape |
|---|---|---|---|
| arch | `DispatchWorkgroups(Dim::MAX_ARCH_INSTANCES, 4, 1)` | `(1, 1, 1)` | 2D: 16 slots × 4 sub-meshes |
| column | `DispatchWorkgroups(Dim::MAX_COLUMN_INSTANCES, 1, 1)` | `(1, 1, 1)` | 1D: 32 slots |
| palm | `DispatchWorkgroups(Dim::MAX_PALM_INSTANCES, 1, 1)` | `(1, 1, 1)` | 1D: 24 slots |
| cactus | `DispatchWorkgroups(Dim::MAX_CACTUS_INSTANCES, 1, 1)` | `(1, 1, 1)` | 1D: 20 slots |
| blade | `DispatchWorkgroups(Dim::MAX_BLADE_INSTANCES, 1, 1)` | `(1)` | 1D: 32 slots |

Each dispatch is CAPACITY-shaped — the full slot count every time, with the
kernel's own `if (slot >= <SPECIES>_MAX_SLOTS) { return; }` and
`is_active` gate doing the narrowing. None reads a live count.

**Nothing couples them. Four independent confirmations:**

1. **Five dedicated bind group layouts**, one per species —
   `archMeshGenLayout_` (3 entries), `columnMeshGenLayout_` (5),
   `palmMeshGenLayout_` (3), `cactusMeshGenLayout_` (3),
   `bladeMeshGenLayout_` (3). No mesh-gen pipeline shares a layout with
   another.
2. **Five entry points, five pipelines, five `dispatch_*` functions**, each
   taking its own bind group as a parameter.
3. **No RAW hazard between any pair of them** — Table E of
   `BINDING_LEDGER.md` lists no ordered pair among the five mesh-gen
   kernels. They write disjoint buffers and read disjoint params.
4. **Five independent ROSTER gates** — `ROSTER.arch`,
   `ROSTER.column || ROSTER.antenna`, `ROSTER.palm`, `ROSTER.cactus`,
   `ROSTER.blade`, each gating its own pipeline creation and its own
   dispatch.

So the arena keeps five entry points and five dispatches, as the handoff
anticipated. **One shared resource exists and it is not a mesh buffer:**
`plantComputeGroundBuffer_` is co-owned by palm, cactus and blade (the
three `LATENT[gate-a-shared]` comments in `state.hpp` name it), bound in
the entity-placement path rather than in any mesh-gen group. It does not
couple the mesh-gen dispatches, but it is the one place the word "plant"
already means three species at once, and an arena ruling should know it is
there.

---

## GO / NO-GO MATRIX

Per precondition. **No recommendation beyond this table.**

| # | precondition | verdict | evidence |
|---|---|---|---|
| 1 | Buffer usage flags are uniform across the five, so one arena buffer can carry every species' role | **GO** | Q1: all VBs `Vertex+Storage+CopyDst`, all IBs `Index+Storage+CopyDst`, all params `Storage+CopyDst`. Arch and column params carry an extra `Uniform` from WALLET_0 — additive, and irrelevant to a vertex/index arena |
| 2 | Render-side vertex layouts are identical, so one arena needs one layout | **GO** | Q2: not merely identical — all five pipelines are handed `&archVBL`, the same object |
| 3 | Index format is common | **GO** | Q3: `Uint32` in all five `draw_*` |
| 4 | Kernel writes are offset-parameterized, so re-basing is local | **GO** | Q4: one `slot * <CONST>` line per kernel is the only absolute address; everything downstream is a parameter |
| 5 | Arena size fits `maxStorageBufferBindingSize` | **GO** | Q6: 5.905 MiB vertices, 2.594 MiB indices — 4.61% and 2.03% of 128 MiB |
| 6 | The five dispatches are independent, so the arena keeps five entry points | **GO** | Q7: five layouts, five pipelines, five ROSTER gates, zero RAW pairs |
| 7 | The draw path can address a species at a non-zero arena base | **NO-GO as it stands** | Q3: all five call `DrawIndexed(indexCount)` with `firstIndex` and `baseVertex` defaulted to 0, and the count is a *prefix* high-water from `prepare_*_mesh_gen`. Correct only while each species owns its buffer from byte 0. A merge requires either both draw arguments carrying the species' arena base, or per-species buffer views. This is the work item the arena actually costs |
| 8 | The five params structs can share one home | **NO-GO for a merged params struct; not required for a vertex/index arena** | Q5: 7 common fields against 9–20 divergent ones per species; segment counts are `u32` in three species and `f32` in two; two species carry a MOSAIC_1 `mosaic_seed` and two carry an unrelated plain `seed`. Merging params is a separate ruling from merging vertices and indices, and nothing in Q1–Q7 makes it a precondition of the latter |
| 9 | No hidden cross-species coupling | **GO, with one named exception** | Q7: `plantComputeGroundBuffer_` is co-owned by palm, cactus and blade, but it lives outside the mesh-gen groups and couples no dispatch |

Two rows short of clean, and they are different in kind: **row 7 is a
mechanical change with a known shape** (two draw arguments and their
sources), while **row 8 is a design question** about whether five
authored-geometry vocabularies should become one. Rows 1–6 and 9 are
unconditional GOs.

One correspondence held by nothing in the tree, surfaced by Q2 and
recorded here because an arena will be reading it: `ArchVertex::arch_index`
is `uint32_t` in C++ and `Float32` in the vertex layout, carrying the
MOSAIC_1 `enc = mosaic_seed * 64 + slot` channel as reinterpreted bits.
