> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# C6 — THE BINDING REGISTRY — RECON (read-only; the single-source-of-truth plan)

Campaign: RENDER/UPDATE API cleanup, Tier-3 item **C6** (RENDER_UPDATE_API_RECON
§6). This recon plans the single-source-of-truth for **binding NUMBERS only** —
behavior-identical, **NO husk deletions** (those are the verified follow-on, §4).
Anchor: RENDER_UPDATE_API_RECON §2 (the 24 layout/group pairs, the "binding
integer typed twice" hazard L2-a) + the three parked husks (§6). Line numbers are
HEAD (post-`06a403d`); any future cut re-greps. **Nothing moved; `git status`
clean but for this file.**

METHOD: two exhaustive read-only extractors (the C++ bind-group surface in
`state.hpp`; the WGSL `@binding` surface in `world.wgsl`) + one husk-confirmation
pass, synthesized here. The `.binding =` census reconciles exactly (**323** =
152 layout + 168 group + 3 offscreen-recreate).

---

## §0 THE ANSWER TO THE CORE QUESTION (stated first)

**Single-source = `constexpr` binding constants, group-scoped. NOT a runtime
registry object.** The lean hypothesis is CONFIRMED against the actual arrays:

- The binding number is a **compile-time value on BOTH sides** — a C++ integer
  literal and a WGSL `@binding` const-expression. A named `constexpr` referenced
  by the layout entry *and* the group entry single-sources the C++ pair; a typo
  is an **undefined symbol → compile error**, which glaw1 catches. The same
  integer is emitted, so the built bind-group is **byte-identical** →
  behavior-identical. Zero runtime machinery.
- A runtime **registry object** (a map/vector queried at build time) is strictly
  *more* machinery — runtime state + indirection — and buys **no** compile-time
  typo protection unless it is itself `constexpr`. It only earns its keep if
  bindings must be *discovered/iterated dynamically*, or if **one** description
  emits layout + group together. But layout and group entries carry **different**
  payloads (layout: visibility + resource *type*; group: the resource *handle* +
  size), so a single object emitting both sides must encode
  visibility/type/size/handle — that is the **C3-class per-binding descriptor
  table**, a far larger refactor that *re-touches the array structure*. That is
  not registry-first. **Runtime object: REFUTED as over-machinery.**

**The one non-obvious constraint the arrays impose: the registry must be
GROUP-SCOPED, not a flat list.** Binding numbers are per-`@group` keys, and this
code reuses low numbers across the two groups with *different meanings*:
`22` = `terrain_mesh_indices` (group 0) **and** `bilinear_sampler` (group 1);
`25` = `tile_grid` (g0) **and** `shadow_map` (g1); `26` = `pier_instances` (g0)
**and** `shadow_sampler` (g1); `30` = `pyramid_instances` (g0) **and**
`cell_fields_read` (g1). A flat `constexpr uint32_t X = 22` list would collapse
two distinct slots. The minimal shape is **two namespaces** (e.g. `bind::g0::…`
and `bind::g1::…`), each a flat list of named `constexpr uint32_t`.

**Discipline-2 sub-question to FLAG for the stamp (authored, not computed).** The
numbers are banded and carry a `render = compute + 200` arithmetic relation
(signal 0→200, vp 2→201, terrain 20→220, agents 60→260, camera 80→280, floating
100→300, ribbon 120→360/361, patch 340, cull 500/501). Two registry shapes:
  - **(a) AUTHORED** — every binding a named literal (`g0::render_terrain = 220`).
    Behavior-identical, no cleverness, each number authored once. **RECOMMENDED**
    — it matches the Frame-Spine law (AUTHORED order, CHECKED by validation,
    never COMPUTED).
  - **(b) COMPUTED** — `g0::render_terrain = g0::terrain + 200`. Collapses two
    names to one root but makes bindings a *computed* quantity (a distinction
    turned into a mechanism). If the +200 band is worth pinning, it belongs as a
    `static_assert` (CHECKED), **never** as the source of the value.
  LEAN: author the literals; optionally add asserts that *witness* the +200 band.
  Registry-first should not encode the arithmetic.

---

## §1 THE BINDING SURFACE (the ground truth of what is duplicated)

`createBindGroups()` (`state.hpp:3547`, called from `GPUState::init` @1593) builds
**24 layout blocks then 24 group blocks**, each a `std::array<…,N> entries{}` with
per-entry `entries[k].binding = <literal>`. One **25th group** ("Gallery Texture")
is (re)built on the offscreen path at `:1991`, outside `createBindGroups`.

**Layout side** declares `binding + visibility + resource TYPE` (the buffer/
texture/sampler handle is *not* named — it appears only in a `//` comment).
**Group side** declares `binding + the resource handle + size`. The two are kept
in sync **only by the shared array index `k` carrying the same literal** — and
the array index is *not* the binding number (WebGPU validates the binding *set*,
order-independent; e.g. Photographer Compute puts binding 80 physically at
`entries[9]` between entries 1 and 2, `state.hpp:4069/5046`).

| # | pair (layout `:ln` → group `:ln`) | N | layout member | bindings (ascending) | typed twice? | const or literal | shared layout? |
|---|---|---|---|---|---|---|---|
| 1 | Compute Entity `3551`→`4536` | 19 | `computeEntityBindGroupLayout_` | 0,1,2,20,25,26,30,60,62,80,100,101,110,111,145,146,152,160,161 | yes | literal | — |
| 2 | Render Entity `3653`→`4634` | 19 | `renderEntityBindGroupLayout_` | 1,25,111,200,201,220,260,280,300,320,321,322,340,360,361,390,391,400,411 | yes | literal | **REUSED by G11** |
| 3 | Mesh Gen Entity `3755`→`4731` | 1 | `meshGenEntityBindGroupLayout_` | 1 | yes | literal | — |
| 4 | Shadow Texture `3774`→`4748` | 3 | `shadowTextureBindGroupLayout_` | 22,23,28 | yes | literal | — |
| 5 | Render Texture `3801`→`4770` | 11 | `renderTextureBindGroupLayout_` | 22,23,25,26,27,28,29,30,31,32,33 | yes | literal | — |
| 6 | Compute Texture `3869`→`4817` | 3 | `computeTextureBindGroupLayout_` | 22,23,33 | yes | literal | — |
| 7 | Terrain Index Gen `3896`→`4839` | 1 | `terrainIndexGenLayout_` | 22 | yes | literal | — |
| 8 | Patch Gen `3914`→`4856` | 9 | `patchGenLayout_` | 1,23,24,25,26,27,28,29,30 | yes | literal | — |
| 9 | Ribbon Compute `3970`→`4902` | 5 | `ribbonComputeLayout_` | 25,26,120,121,122 | yes | literal | — |
| 10 | Gallery Entity `4002`→`4935` | 4 | `galleryEntityBindGroupLayout_` | 1,201,280,320 | yes | literal | — |
| 11 | Gallery Texture `4030`→`1991` | 3 | `galleryTextureBindGroupLayout_` | 50,51,52 | yes | literal | group at `:1991` (offscreen) |
| 12 | Photographer Compute `4059`→`5039` | 10 | `photographerComputeLayout_` | 1,60,80,140,141,142,144,145,146,152 | yes | literal | — |
| 13 | Entity Placement `4117`→`5080` | 14 | `entityPlacementComputeLayout_` | 1,60,143,144,145,146,147,148,149,150,151,152,160,161 | yes | literal | — |
| 14 | Frustum Cull `4189`→`5138` | 7 | `frustumCullLayout_` | 1,2,60,80,340,500,501 | yes | literal | — |
| 15 | GoL Zone `4233`→`5172` | 14 | `zoneGolComputeLayout_` | 1,25,26,30,160,161,162,163,164,165,166,167,168,169 | yes | literal | — |
| 16 | Pawn Aura `4308`→`5235` | 5 | `pawnAuraComputeLayout_` | 1,60,170,171,172 | yes | literal | — |
| 17 | Orb Compute `4345`→`5267` | 3 | `orbComputeLayout_` | 410,411,412 | yes | literal | — |
| 18 | Orb Copy `4375`→`5295` | 3 | `orbCopyLayout_` | 411,413,414 | yes | literal | — |
| 19 | Pyramid Mesh Gen `4400`→`5320` | 3 | `pyramidMeshGenLayout_` | 190,191,192 | yes | literal | — |
| 20 | Arch Mesh Gen `4424`→`5345` | 3 | `archMeshGenLayout_` | 193,194,195 | yes | literal | — |
| 21 | Column Mesh Gen `4448`→`5370` | 3 | `columnMeshGenLayout_` | 196,197,198 | yes | literal | — |
| 22 | Palm Mesh Gen `4472`→`5395` | 3 | `palmMeshGenLayout_` | 180,181,182 | yes | literal | — |
| 23 | Cactus Mesh Gen `4496`→`5420` | 3 | `cactusMeshGenLayout_` | 183,184,185 | yes | literal | — |
| 24 | Blade Mesh Gen `4516`→`5441` | 3 | `bladeMeshGenLayout_` | 186,187,188 | yes | literal | — |

**Every** binding in **every** block is a hand-typed literal — **no named C++
constant maps a symbol to a binding slot** anywhere in the file (grep-confirmed;
the only `constexpr`/`Dim::` values near this code size the `entries[k].size`
expressions, never the binding index).

**Two 19-entry pairs are the worst** (Compute Entity #1, Render Entity #2) — 38
binding literals each typed twice, plus the Render Entity layout is **REUSED** by
a 25th group, "Photographer Render Entity" (`state.hpp:4960`, 19 entries), which
diverges from G2 only at binding 201 (`photographerVPBuffer_` vs `vpBuffer_`) and
280 (`photographerCameraBuffer_` vs `cameraBuffer_`).

**Same buffer, DIFFERENT binding numbers across layouts** (no registry — the
"which number is this buffer here?" ambiguity):

| buffer member | binding → group |
|---|---|
| `signalBuffer_` | 0 (Compute Entity) · 200 (Render Entity, Photographer RE) |
| `vpBuffer_` | 2 (Compute Entity, Frustum Cull) · 201 (Render Entity, Gallery Entity) |
| `terrainBuffer_` | 20 (Compute Entity) · 220 (Render Entity, Photographer RE) |
| `agentStateBuffer_` | 60 (Compute Entity, Photographer C, Entity Placement, Frustum Cull, Pawn Aura) · 260 (Render Entity, Photographer RE) |
| `cameraBuffer_` | 80 (Compute Entity, Photographer C, Frustum Cull) · 280 (Render Entity, Gallery Entity) |
| `floatingEntityBuffer_` | 100 (Compute Entity) · 300 (Render Entity, Photographer RE) |
| `zoneConfigBuffer_` | 160 (Compute Entity, Entity Placement, GoL Zone) · 32 (Render Texture) |
| `patchInstancesBuffer_` | 340 (Render Entity, Photographer RE, Frustum Cull) · 144 (Photographer C, Entity Placement) · 165 (GoL Zone) |
| `visiblePatchIndicesBuffer_` | 391 (Render Entity, Photographer RE) · 500 (Frustum Cull) |
| `orbStateBuffer_` | 400 (Render Entity, Photographer RE) · 410 (Orb Compute) · 413 (Orb Copy) |
| `ribbonBuffer_` / `ringTransformsBuffer_` | 360/361 (Render Entity) · 120/121 (Ribbon Compute) |
| `patchHeightfieldArrayReadView_` | 145 (g0 entity/photog/placement) · 28 (g1 shadow/render tex) · 163 (GoL Zone) |
| `bilinearSampler_` | 146 (g0) · 22 (g1 shadow/render/compute tex) · 164 (GoL Zone) |

This is the single fact the registry names: today "binding 340" and "binding 144"
and "binding 165" are three unrelated literals that all mean *"patch_instances,
here."* Named, they become `g0::patch_instances_render` / `…_photog` / `…_zone`
(or one name if the design unifies them — a *ruling*, not a mechanical step).

---

## §2 THE C++/WGSL PIN (how a registry-first cut keeps the shader in lockstep)

The binding number lives in **THREE** hand-typed places, none compiler-linked:
1. the C++ **layout** entry (`state.hpp`),
2. the C++ **group** entry (`state.hpp`),
3. the WGSL **`@binding(N)`** attribute (`world.wgsl`).

**The WGSL surface** (extractor): **108** real `@binding` declarations — **94 in
`@group(0)`, 14 in `@group(1)`; no group 2+.** Every `@group`/`@binding` value is
a **bare integer literal** — a search for any alphabetic/`const`/`override`
binding reference returned zero. Group 0 = the big everything-group (uniforms +
storage sim state + write storage-textures + mesh-gen scratch + the `render_*`
read mirrors), numeric range 0–501. Group 1 = shared samplers + read-side sampled
textures, range 22–52. The WGSL declares a **superset**; each pipeline's C++
layout picks a **subset** — so **one WGSL binding number participates in several
C++ layouts.** Five group-0 numbers are declared **twice** in WGSL under
different names + access modes (main block vs the frustum-cull `fc_*` block):
binding 1 (`config`/`fc_config`), 2 (`vp_data` rw / `fc_vp` ro), 60 (`agent_state`
rw / `fc_agents` ro), 80 (`camera_state` rw / `fc_camera` ro), 340
(`patch_instances`/`fc_patches`) — legal because no single entry point uses both
aliases.

**The hard truth for registry-first:** a C++ `constexpr` **cannot** be read by the
shader (WGSL has no `#include` of C++). So a C++-only constant registry
single-sources sites **1 and 2** (the L2-a "typed twice" hazard — the two C++
copies) but leaves site **3**, the 108 WGSL literals, as an **independent third
copy**. Registry-first must be honest about this: **it closes 2 of the 3 copies;
the WGSL stays a mirror.** How the mirror is kept in lockstep — options this
codebase can support, ranked by machinery:

- **Option A — mirror + comment convention + the crash-aware gate (MINIMAL,
  registry-first).** Keep the WGSL `@binding` literals; the C++ layout comments
  already name the WGSL var (`// zone_config (storage — matches var<storage,
  read_write>)`). A mismatch is caught at **bind-group / pipeline validation**
  (§5), not silently. This is the *same* discipline the codebase already uses for
  C++↔WGSL scalar constants (`AGENT_EVICTION_RADIUS` is "MIRRORED MANUALLY in
  world.wgsl… the compiler cannot" — `bodies/agents.inl`). **This is the C6 cut:
  single-source the C++ pair; the WGSL mirror rides the launch gate.**
- **Option B — a generated block, one authored source → both sides (FOLLOW-ON).**
  world.wgsl is **read from disk as a runtime string** (`std::ifstream` →
  `shaderSource_` → `wgslSource.code` → `CreateShaderModule`,
  `renderer.hpp:1219-1286`) — so a build/boot step *could* emit both the C++
  `constexpr` header and a WGSL prelude (or substitute `@binding($NAME)` tokens
  from the single list) before compile. Feasible *here* precisely because the
  shader is text, not embedded. But it is a codegen/preprocess step — **more
  machinery than registry-first**; name it, don't build it now.
- **Option C — WGSL module-const bindings (WITHIN-WGSL single-source only).** WGSL
  allows `const B: u32 = 22; @binding(B) …`. This single-sources *within* the
  shader but still does **not** link to C++. Low value for the cross-language pin;
  noted for completeness.

RECOMMENDATION: **Option A for C6.** The registry is C++-side; the WGSL mirror is
gated by launch-validation (§5). Option B is the eventual "true pin" — a named
follow-on, sequenced after the registry exists (it also unblocks a WGSL-side
re-index).

---

## §3 THE FIXED-ARRAY CONSTRAINT (the line that keeps this behavior-identical)

**Registry-first does NOT re-index and does NOT touch array sizes.** It rewrites
exactly one thing: the **right-hand integer** of each `entries[k].binding =
<literal>;` → `entries[k].binding = bind::g0::<name>;`. It leaves untouched:

- the array **size** `N` (`std::array<wgpu::BindGroupLayoutEntry, 19>`),
- the array **index** `k` and entry **order**,
- every other field — `.visibility`, `.buffer.type`, `.buffer`, `.textureView`,
  `.sampler`, `.size`.

`N` is the **entry count** (how many bindings the group has), orthogonal to the
binding **integers**. `N` changes only when a binding is **added or removed** —
which is **husk removal = the re-index follow-on (§4), explicitly not in this
cut.** So the precise line: **registry-first single-sources the binding-integer
*assignments*; it never alters an array size, index, order, or resource.** Same
`N` entries, same order, same handles, same integers emitted → the built
bind-group is byte-identical; glaw1 proves the names resolve. That is the whole
behavior-identity argument.

---

## §4 THE RE-INDEX STRATEGY (planned, NOT executed here)

Once the numbers are named, husk removal (§6) becomes mechanical. Today, removing
a dead binding means hunting the literal `20` — which collides with array indices,
`entries[k].size` sub-expressions, and every unrelated `20` — a **glaw1-blind**
edit (the RAYMARCH-husk pain). With the registry:

1. Delete the husk's `bind::g0::<name>` from the registry header.
2. The **compiler flags every dangling `bind::g0::<name>`** — both the layout and
   group entry lines, plus any pipeline-layout reference. A literal hunt becomes a
   **symbol-removal the compiler drives.**
3. At each flagged site: delete the `entries[k]` block, **decrement `N` by one**
   (order-independence means no re-pack — just remove and shrink), and delete the
   matching WGSL `@binding` declaration + its `var`.
4. Re-run the crash-aware gate (§5): the shrunk group still validates against its
   layout and every pipeline that binds it.

Net: the registry converts a blind literal-hunt-and-renumber into a
**compiler-checked symbol removal + a bounded WGSL edit**. This is the recipe the
three husks inherit; it is defined here and run **nowhere** in this cut.

---

## §5 THE CRASH-AWARE GATE (this cut's gate; pixel-identity is NOT sufficient)

A mis-single-sourced binding can render **identical on the common path and
crash/validation-fail off it** (a wrong number in a rarely-bound group). So the
gate is *bind-group + pipeline validation coverage*, not pixels:

1. **Launch full-ROSTER with Dawn validation ON** (the debug device). The app
   boots the_board (incubator).
2. **All 24 layouts + all 25 groups create non-null.** `createBindGroups()`
   (`state.hpp:3547`) returns true — every block ends in `if (!x) return false`.
   WebGPU validates the binding *set* at `CreateBindGroup`: a binding in the group
   but not the layout, a duplicate, or a layout/group **type** disagreement fails
   **here**. (The offscreen "Gallery Texture" group `:1991` is built later, in
   `initOffscreenResources` — it must also succeed.)
3. **Every bind group is actually BOUND at least once under validation.**
   `SetBindGroup` validates the group against the *pipeline's* layout at
   dispatch/draw. So the run must exercise the **off-common-path** groups, or a
   wrong number there never gets checked:
   - **Photographer Compute (10) + Photographer Render Entity (19, reuses the
     render-entity layout)** + the **Gallery/exhibition texture group (`:1991`)** →
     **trigger a snapshot** (the photographer + gallery paths).
   - **GoL Zone (14) + Pawn Aura (5)** → **spawn a GoL zone / pawn-aura frame**
     (a derive frame).
   - **Entity Placement (14), Frustum Cull (7), the six Mesh-Gen groups (3 each),
     Ribbon Compute (5), Gallery Entity (4)** → covered by a **full-ROSTER**
     walkthrough (each family's mesh-gen dispatches; cull runs every frame).
4. **Zero Dawn validation errors** logged across all of the above.

**The concrete checklist:** launch full ROSTER → walk the world (all families
mesh-gen + draw + cull) → **fire a snapshot** (photographer + exhibition groups) →
**enter an indoor/gallery scene** (indoor_shell, gallery entity/texture) → **spawn
a GoL zone** (zone + aura + derive). If every one of those binds clean under the
validation layer, the single-source held. Pixel-identity is then a *secondary*
confirmation (necessary, not sufficient).

---

## §6 STILL-DEAD CONFIRMATION (the three husks at HEAD — flagged, deleted nowhere)

Confirmed at HEAD (grep, not the old markers). **All three remain writer/
reader-free of any LIVE consumer** — no husk gained a reference since parking. A
sharp distinction that matters for how each rides C6:

**Husk 1 — TerrainState — STILL DEAD, and a BINDING-SLOT husk (the registry's
direct target).** Def: `state.hpp:427` `struct GPUTerrainState` (assert @1262);
`world.wgsl:668` `struct TerrainState`. Marker: `state.hpp:426` ("DEAD: no
writer/reader post-851ce68; awaiting storage-weld removal"), `world.wgsl:5522-25`.
GPU side is reader/writer-free: the buffer is **still BOUND** at binding **20**
(`terrain_state`, `world.wgsl:4874`) and **220** (`render_terrain`,
`world.wgsl:4937`) — declared, but **no shader body reads or writes either.**
C++ residue = the storage-weld the marker names: alloc `state.hpp:2679`, isReady
`:2787`, one boot **dead-store** `WriteBuffer` `:5532-5541`, and **3 bind entries
`state.hpp:4551 / 4649 / 4971`** (inside the Compute-Entity #1 / Render-Entity #2
/ Photographer-RE groups — my §1 bindings 20 and 220). **This is the paradigm
bound-but-dead husk:** removing it is exactly the §4 re-index — free bindings
20/220, drop 3 entries, shrink the two 19-entry groups to 18. It is *why* C6
exists.

**Husk 2 — PMG (pyramid mesh-gen) basket — STILL DEAD, also a BINDING-SLOT husk.**
Def: `state.hpp:120-124` (`PMG_*` counts); `world.wgsl:8816-8850` (`PMG_*` consts,
`PyramidMeshParams`, bindings **190/191/192**, helpers `pmg_*`). Markers:
`world.wgsl:8811-14` + `8908-16` ("write-only husk with no entry point →
C6 layout-weld"), `state.hpp:2364-2380` (`DEAD (C2)`), `renderer.hpp:138`. The
`pyramid_mesh_gen` compute entry point is **CUT** — no pipeline, no dispatch, no
draw, no reader of the generated VB/IB (`pmg_vertices`/`pmg_indices` are
write-only, `pmg_emit_tri` has no caller — a closed dead island). Yet the **layout
+ group still build** (Pyramid Mesh Gen, my §1 #19, `state.hpp:4400/5320`,
bindings 190/191/192) and the buffers still allocate (`state.hpp:3182-3226`) — a
group constructed but never dispatched. Surviving C++ refs are dead-stores only
(`upload_pyramid_mesh_params_slot` @`entities.hpp:772/1554`,
`entity_pipeline.hpp:823`; `*_pending = true` written, never read). **Bound-but-
dead → the §4 re-index frees 190/191/192 and drops group #19.** (Adjacent, NOT
part of the basket: `GROUND_ATLAS_PYRAMID = 48` `world.wgsl:4966` is a separate
write-only atlas-slot husk — same orphan-sweep lineage, flagged for completeness.)

**Husk 3 — complexity texel — STILL DEAD (reader-free), but a DATA-CHANNEL husk,
NOT a binding-slot one — it corrects the anchor.** In-code marker is
**`LATENT[complexity]`**, not `DEAD` (same meaning: baked, shipped, unread). Bake:
the `.w` channel of `textureStore(patch_heightfield_array_write, …,
vec4(height, grad_x, grad_z, complexity))` `world.wgsl:7067`, carried into the
`PatchTerrainVarying.complexity` varying (`:3667`, written `:3770`). Markers:
`world.wgsl:3767-69`, `7064-66`. **No reader:** `in.complexity` is read nowhere;
all five palette calls hardcode `0.5` (`world.wgsl:5147/7096/7238/7266/7283`).
**Key difference:** complexity has **no separable GPU resource** — it rides *live*
buffers (the `patchHeightScratchBuffer_` stride-2 slot `state.hpp:1388` and the
live heightfield texture's `.w` channel, produced free by the LIVE gradient
pass). So its removal is **WGSL-only** (drop the `.w` store + the scratch slot +
the varying field) and **frees no binding number.** It does **not** ride the
binding registry the way husks 1 and 2 do. This refines RENDER_UPDATE_API_RECON
§6's "TerrainState husk + complexity texel could ride this": TerrainState is a
true binding re-index the registry de-risks; **complexity is a parallel WGSL
channel cleanup that needs no registry.**

**Bottom line for the follow-on:** husks 1 & 2 are the binding-slot re-index the
registry unblocks (bindings 20/220 and 190/191/192 freed; the two 19-groups → 18
and group #19 dropped); husk 3 is a separate WGSL-channel removal. All three are
**flagged, deleted NOWHERE in this cut.**

---

## §7 DISCIPLINE

Read-only. No binding renamed, no array resized, no husk removed, no struct
touched. `git status` clean but for this file. **Full stop at the report for the
stamp** — the ruling wanted: (1) constexpr vs runtime object (§0: constexpr,
group-scoped); (2) authored vs computed bands (§0 sub-question: recommend
authored); (3) the WGSL-pin option for C6 (§2: Option A now, Option B named
follow-on); (4) whether the same-buffer-different-number cases (§1) get one name
or several (a design ruling, not mechanical). Nothing executes until the stamp.
