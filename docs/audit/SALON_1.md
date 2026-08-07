# SALON_1 — STAGE A: RECON

Read-only. Zero edits to `src/**` outside this file. Zero builds.
Every claim below was read at the named `path:line` in this tree at
`4483b3b`. Where the handoff names a value, the value was re-read
before being relied on; where it disagrees with the tree, §0 says so.

**Scope read:** `src/cartridges/the_board/**`, `src/incubator_dual.cpp`,
plus `src/console/console.hpp` (device limits), `src/docs/LAWS.md` (L2/L3).
`src/tools/*.jsx` confirmed OUT — and the handoff's ruling is vindicated
concretely. Two files mention "painting" only as prose about Jean's canvases
(`7t_blade_cluster_designer.jsx:8`, `7t_cactus_designer.jsx:7`). One does carry
gallery data: `7t_population_designer.jsx:62-64` holds a `gallery` family row
and `const GALLERY_CHANCE_BY_ARCHETYPE = [0.03, 0.06, 0.30, 0.40]; // gallery.hpp`.
**That copy is stale by ~4×** — the live table is
`{ 0.12f, 0.24f, 0.70f, 0.85f }` (`gallery.hpp:169`), and the same row's
`mood: [1,1,1,1,1,0]` has six entries against `MOOD_COUNT = 4`. A sketch that has
already drifted is not a call site. Nothing in this campaign touches it.

---

## §0 — CORRECTIONS TO THE HANDOFF

Nine findings that change what a later stage must do. Reported, not acted on.

### 0.1 — Stage B is not two edits. The WGSL room holds the same fact three more times.

`Dim::PAINTING_MAX_SLOTS` has a mirror in `world.wgsl`, and a grep for
the *name* finds only one third of it:

| `world.wgsl` | text | what it is |
|---|---|---|
| `:9865` | `const PAINTING_MAX_SLOTS: u32 = 32u;` | the named mirror |
| `:9869` | `@group(1) @binding(50) var<storage, read> painting_slots: array<UnifiedPaintingSlot, 32>;` | **bare literal** — the render-side array |
| `:9325` | `@group(0) @binding(143) var<storage, read_write> photo_painting_slots: array<UnifiedPaintingSlot, 32>;` | **bare literal** — the compute-side array |
| `:9573` | `for (var i = 0u; i < 32u; i++) {` | **bare literal** — `compute_entity_placement`'s painting Y-correction loop |

Three of the four are bare `32`s. This is exactly the case the REGISTER's
"bare substring counts are not evidence" clause exists for.

`LAWS.md:51` (**L3 — THE MIRROR LAW**) governs: *"Edit both rooms in the
same commit."* Nothing in the compiler sees this pair. `audit/tools/glaw2`
does not check it either — its own header (`run.py:20`) states it proves
"no dangling name and no structural break", not values.

**Consequence:** a C++-only Stage B does not raise capacity. It raises the
buffer and the draw counts while the shader keeps reading 32 slots.

Cost note on the third literal: `compute_entity_placement` is **dirty-driven,
not per-frame** — `cartridge.hpp:1602-1605` gates `dispatch_placement_correction`
on `world_state_.placement_dirty`. So `:9573`'s serial 32-iteration loop is a
placement-event cost, not a frame cost, and raising its bound to 256 is cheap.
It still has to be raised, or slots 32+ never get their ground Y corrected.

### 0.2 — Stage B's "expected visual delta: none" does not hold as written.

The two draws are sized *from the constant*, and one of them has no guard.

`renderer.hpp:922` — `pass.Draw(Dim::PAINTING_QUAD_VERTS, Dim::PAINTING_MAX_SLOTS);`
→ instance count. The receiving shader is `gallery_frame_vs`
(`world.wgsl:9909-9910`), which does `let slot = painting_slots[iid];`
(`:9915`) with **no bounds guard**. At 256 instances against
`array<UnifiedPaintingSlot, 32>`, instances 32..255 index out of range.
WGSL does not make this UB — the implementation clamps — so the observable
result is ~224 redundant instances all reading slot 31. If slot 31 is
active, a `TERRAIN_QUAD`, and inside the veil ring, that is 224 coplanar
duplicate quads on an alpha-blended, depth-writing pipeline
(`renderer.hpp:1863-1890`). Not a crash; not nothing either.

`renderer.hpp:938,944` — `pass.Draw(Dim::PAINTING_FRAME_VERTEX_COUNT);`
where `PAINTING_FRAME_VERTEX_COUNT = PAINTING_MAX_SLOTS * 78`
(`state.hpp:261`). Here `wall_painting_vs` **does** guard —
`if (pidx >= PAINTING_MAX_SLOTS)` at `world.wgsl:10183` — so slots 32..255
become degenerate early-outs. Safe, but 2 × 17,472 wasted vertex
invocations per frame.

**Read:** the *wall* path is delta-free at 256. The *terrain-quad* path is
not. The probe isolates the FXC verdict only if both rooms move together,
which returns us to 0.1.

### 0.3 — "Bind-group buffer growth is a named FXC hazard" is not what this tree documents.

`LAWS.md:30` (**L2 — THE FXC LAW**) names four things, verbatim:

1. lean, byte-pinned instance structs in hot loops;
2. no new runtime branching in the collision/ground chain;
3. no texture-array stamps in the collision chain — *"they hang FXC"*;
4. **"Storage buffers per stage = 10. Uniform buffers per stage = 12."**

Buffer *size* growth appears nowhere.

**The phrase the handoff uses has a real precedent, and it means something
else.** `audit/past reports/BATCH_F_REPORT.md:88-94`, verbatim:

> **Cost: storage 5 → 7 of 10** — and the blast radius is the whole shared
> layout … FXC exposure: this is bind-group buffer growth on the agent
> kernels — exactly what the banner names as a hang trigger

Read the cost line. "Bind-group buffer growth" there is **5 → 7 buffers**, a
**count** against L2.4's ceiling of 10 — not bytes. The live code comment that
lesson produced (`renderer.hpp:1283-1290`) is likewise about adding a **group**:
*"agent-side binding growth … never widens their FXC surface."*

Stage B changes no struct layout (`sizeof(GPUPaintingSlot)` stays 128), adds no
branch, adds no texture-array stamp, adds no bind group, and **adds no buffer to
any stage**. Counted against L2.4 the two wall-painting pipelines use **2 storage
buffers in the vertex stage** (`render_vp`, `painting_slots`) and **4 in the
fragment stage** (`render_vp`, `render_camera`, `render_light`, `painting_slots`)
— see §A2.3. Both far under 10, and both unchanged by Stage B.

The device requests full adapter limits (`console.hpp:266-269`), so
`maxStorageBufferBindingSize` is not near 32 KiB on any real adapter. There is
no `maxBufferSize` or `maxUniformBufferBindingSize` discussion anywhere in this
tree.

**Read:** Stage B grows one buffer's *bytes*. The hazard this tree names is
growth in buffer *count* and in *branching*. They are different facts that share
a word. That does not make the probe worthless — it makes it cheap, and it means
a null FXC result is the expected result rather than a reassurance. The *real*
Stage B hazard is §0.1, and it is a correctness hazard, not an FXC one.

### 0.4 — Stage C's return count is three, not two.

The handoff says *"the design session saw them in both the snapshot and
authored paths."* There is a third, above the content branch. Full census
in §C.

### 0.5 — The vault ceiling is already derived from where the paintings hang.

`mood.hpp:601` `vault_crown()` computes the vault's spring height from
painting geometry:

```cpp
static constexpr float PAINT_CENTER_FRACTION = 0.45f;
static constexpr float PAINT_TOP_MARGIN     = 5.5f;
static constexpr float SPRING_MARGIN        = 8.0f;
const float paint_center = ch * PAINT_CENTER_FRACTION;
const float paint_top    = paint_center + PAINT_TOP_MARGIN;
const float spring_h     = paint_top + SPRING_MARGIN;
```

`PAINT_CENTER_FRACTION = 0.45f` is a **second home** for
`WALL_ART.paint_y_frac = 0.45f` (`gallery.hpp:288`). One fact, two homes,
currently agreeing by coincidence, with the identity nowhere stated — the
same shape as the `MAX_PROMOTIONS_PER_FRAME` / `PAINTING_MAX_SLOTS` pair the
handoff already names.

This bites Stage E directly. R4 wants a `top_margin`. The surface it must be
measured against is the **vertical wall top**, which is `ch` for FLAT but
`spring_h` for VAULT — and `spring_h` is a function of the paint fraction.
Change the hang and the room changes shape underneath it.

Numbers, read not guessed (`spine_state.hpp:201-202`, `state.hpp:69`):

| mood | `ch` | wall top | crown |
|---|---|---|---|
| `MOOD_INDOOR_FLAT` | 20.0 | 20.0 (`wall_h = ch`, `mood.hpp:725`) | — |
| `MOOD_INDOOR_VAULT` | 25.0 | 24.75 (`spring_h`) | 47.25 … 92.25 (`spring_h + rise`, rise scales with span) |

`place_wall_paintings` receives `ceiling_h = m.ceiling_height`
(`mood.hpp:713, 841`) — i.e. 20.0 / 25.0. It never sees `spring_h` or
`crown_h`. For VAULT it is handed 25.0 for a wall that stops at 24.75.

### 0.6 — `exhibition_count` has no reader anywhere. Not one.

Full census in §A9. It is incremented at four sites, decremented at two,
zeroed at one, and **read at zero**. Not even in a log line. This is the
single most useful fact for Stage D: the field is free to be repurposed as
a refcount base, or deleted, without breaking a consumer.

Sharpening it: both decrements (`gallery.hpp:1215`, `:1803`) are unfloored
`uint32_t` `--`. The field can already wrap to `0xFFFFFFFF`, and nothing would
notice — which is the proof that no consumer exists, stated the other way
round.

### 0.7 — No shader path assumes a slot↔layer bijection. Stage D's GPU side is already free.

Both sample sites use `slot.texture_layer`, never the slot index:
`world.wgsl:9986` (`out.texture_layer = slot.texture_layer;` → varying →
sampled at `:9998`) and `world.wgsl:10213`
(`textureSample(painting_array, painting_sampler_filt, in.uv, slot.texture_layer)`).
Nothing derives one from the other. The weld is entirely a CPU-side
bookkeeping convention.

### 0.8 — `SHADOW_WALL_PAINTING_VS` is not merely caller-free. It does not exist.

A12's question is answered more strongly than it was asked. See §A12.

### 0.9 — The unguarded edge is the floor, not the ceiling.

The handoff reads `max_bottom_height = 4.0f` as *"a floor-side clamp only. There
is no top margin. Nothing prevents crossing the ceiling plane."* The first two
clauses hold. The third does not describe what the numbers reach.

Swept over every bucket's full `(height, y_offset)` rectangle (§E.1), the
reachable canvas band on a FLAT wall is **`y ∈ [−1.50, 16.00]`**, or
`[−1.95, 16.45]` once the 0.45 frame border is counted, against `ch = 20.0`.

- **The ceiling is never crossed today.** The top is incidentally bounded by
  `min(py + h/2, 4 + h)` — the same clamp, viewed from the other side — leaving
  **3.55 m of dead wall** above the art. Unguarded, but not breached.
- **The floor is crossed.** A `statement` piece at `h = 14`, `y_offset = −3.5`
  puts its frame's bottom edge **1.95 m below `y = 0`**. `max_bottom_height`
  cannot catch it: the clamp fires only when `bottom > 4`, i.e. when a painting
  hangs too *high*.

So R4 needs two new guards, and the one the handoff did not name is the one
that is already failing.

---

## §A1 — `GPUPaintingSlot`, both rooms

**CPU.** `src/cartridges/the_board/realization/state.hpp:1619-1651`, inside
`namespace t7 { namespace the_board { ... }}` (the `Dim` / GPU-mirror region
of `state.hpp`). Declared `struct alignas(16) GPUPaintingSlot`. Pinned by
`static_assert(sizeof(GPUPaintingSlot) == 128, "GPUPaintingSlot must be 128 bytes")`
at `:1651`.

**WGSL.** `world.wgsl:9837-9863`, `struct UnifiedPaintingSlot`, under the
banner at `:9831`: `// --- Unified Painting Slot (must match GPUPaintingSlot in state.hpp, 128 bytes)`.

**Field-by-field. The mirror is exact.**

| off | cpu field | cpu type | wgsl field | wgsl type |
|---:|---|---|---|---|
| 0 | `position[3]` | `float[3]` | `position` | `vec3<f32>` |
| 12 | `texture_layer` | `uint32_t` | `texture_layer` | `u32` |
| 16 | `forward[3]` | `float[3]` | `forward` | `vec3<f32>` |
| 28 | `form_type` | `uint32_t` | `form_type` | `u32` |
| 32 | `up[3]` | `float[3]` | `up` | `vec3<f32>` |
| 44 | `is_active` | `uint32_t` | `is_active` | `u32` |
| 48 | `scale_x` | `float` | `scale_x` | `f32` |
| 52 | `scale_y` | `float` | `scale_y` | `f32` |
| 56 | `uv_scale_x` | `float` | `uv_scale_x` | `f32` |
| 60 | `uv_scale_y` | `float` | `uv_scale_y` | `f32` |
| 64 | `geometry_seed` | `float` | `geometry_seed` | `f32` |
| 68 | `content_source` | `uint32_t` | `content_source` | `u32` |
| 72 | `patch_gx` | `int32_t` | `patch_gx` | `i32` |
| 76 | `patch_gz` | `int32_t` | `patch_gz` | `i32` |
| 80 | `frame_depth` | `float` | `frame_depth` | `f32` |
| 84 | `frame_width` | `float` | `frame_width` | `f32` |
| 88 | `canvas_recess` | `float` | `canvas_recess` | `f32` |
| 92 | `_pad0` | `float` | `_pad0` | `f32` |
| 96 | `frame_color[3]` | `float[3]` | `frame_color` | `vec3<f32>` |
| 108 | `_pad1` | `float` | `_pad1` | `f32` |
| 112 | `_pad2[4]` | `float[4]` | `_pad2` | `vec4<f32>` |
| | **128** | | | |

No mismatch in order, type, or padding. Each `vec3` is followed by a scalar,
so every 16-byte slot is exactly filled and WGSL's `vec3` 16-byte alignment
introduces no implicit gap — the layout is valid in both languages without
adjustment. `alignas(16)` and the trailing `_pad2[4]` are what make that true;
neither is decorative.

**Other mirrors:** none live. `audit/past reports/CC_AUDIT_REPORT.md:205,1440`
records the pair at older line numbers; `src/docs/old docs/mop.patch` carries
historical diff text. No `.jsx` mirror. No codegen step.

**GROWTH LAW note (for HELD).** Adding `uv_offset_x/y` costs two of the four
`_pad2` floats and leaves `sizeof` at 128 — the two-rooms edit is a rename of
padding, and the `static_assert` at `:1651` is the handshake unchanged.

---

## §A2 — The painting-slot buffer, its bind groups, and the FXC question

### A2.1 — Creation

`state.hpp:3276-3278`:

```cpp
paintingSlotsBuffer_ = makeBuffer("Painting Slots",
    sizeof(GPUPaintingSlot) * Dim::PAINTING_MAX_SLOTS,
    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
```

Member declared `state.hpp:1917`. Size today **4,096 B**; at 256, **32,768 B**.
Presence-checked at `:3314`. Zero-filled at boot, `state.hpp:6168-6175`.

Preceded at `:3275` by a `LATENT[gate-a-shared]` note claiming the buffer is
*"exclusive-in-Compute-Entity + Entity-Placement"*. Read against the tree that
is imprecise: the buffer's only two bind-group memberships are **Gallery
Texture** and **Entity Placement** (below). There is no Compute-Entity
membership. Noted, not fixed.

### A2.2 — Every bind group containing it (exactly two)

Complete list of `paintingSlotsBuffer_` occurrences: `state.hpp:1917` (decl),
`2240`, `2244`, `2249` (the three upload doors), `2394`, `3276`, `3314`,
`5609`, `6173`.

**(1) Gallery Texture BindGroup** — `state.hpp:2391-2412`.
Layout `galleryTextureBindGroupLayout_`, built at `state.hpp:4554-4589`,
labelled `"Gallery Texture Layout"`. Bound as **group 1**
(`renderer.hpp:920-921, 936-937, 942-943`).

| binding | resource | type | visibility |
|---|---|---|---|
| 50 `painting_slots` | `paintingSlotsBuffer_` | `ReadOnlyStorage` | Vertex \| Fragment |
| 51 `painting_array` | `exhibitionReadView_` | texture `2DArray`, Float | Fragment |
| 52 `painting_sampler_filt` | `paintingSampler_` | Filtering sampler | Fragment |
| — `bilinear_sampler` | `bilinearSampler_` | Filtering sampler | Vertex |
| — `live_card_read` | `liveCardView_` | texture `2D`, Float | Vertex |

Binding numbers from `binding_registry.hpp:151-153`. The layout sets **no
`minBindingSize`**; only the bind-group *entry* sets `.size`
(`state.hpp:2395`), to the whole buffer.

**(2) Entity Placement compute BindGroup** — `state.hpp:5603-5625+`.
Layout `entityPlacementComputeLayout_`. Bound as **group 0**.

| binding | resource |
|---|---|
| `config` | `configBuffer_` |
| **143 `photo_painting_slots`** | **`paintingSlotsBuffer_`**, `.size = sizeof(GPUPaintingSlot) * Dim::PAINTING_MAX_SLOTS` |
| `photo_heightfield` | `patchHeightfieldArrayReadView_` |
| `photo_sampler` | `bilinearSampler_` |
| `arch_ground` | `archGroundBuffer_` |
| `column_ground` | `columnGroundBuffer_` |
| `plant_ground` | `plantComputeGroundBuffer_` |
| +2 more entries | (9 total) |

Binding 143 from `binding_registry.hpp:70`. This is the **read_write** face
(`world.wgsl:9325`) — the Y-correction pass that seats outdoor paintings on
the live ground.

### A2.3 — Pipelines on those layouts

**Gallery Texture layout (group 1)** is used by exactly three render
pipelines, all built with the two-group layout
`{galleryEntityLayout_, galleryTextureLayout_}`:

| pipeline | member | vs | fs | site |
|---|---|---|---|---|
| `"Gallery Frame"` | `galleryFramePipeline_` | `gallery_frame_vs` | `gallery_frame_fs` | `renderer.hpp:1881-1898` |
| `"Wall Painting Canvas"` | `wallPaintingCanvasPipeline_` | `wall_painting_vs` | `wall_painting_canvas_fs` | `renderer.hpp:1924-1949` |
| `"Wall Painting Frame"` | `wallPaintingFramePipeline_` | `wall_painting_vs` | `wall_painting_frame_fs` | `renderer.hpp:1952-1977` |

All three sit behind `if constexpr (ROSTER.gallery)`.

**Gallery Entity layout (group 0)** — `state.hpp:4526-4552`, 4 entries:
`config` (Uniform, V\|F), `render_vp` (ReadOnlyStorage, V\|F),
`render_camera` (ReadOnlyStorage, F), `render_light` (ReadOnlyStorage, F).

**Entity Placement layout (group 0)** is used by the
`compute_entity_placement` pipeline (`renderer.hpp:1387-1400`), which takes
`{entityPlacementComputeLayout_, computeTextureLayout_}`.

Per-stage storage-buffer counts for the wall-painting pipelines: **vertex 2,
fragment 4** — against L2.4's ceiling of 10.

### A2.4 — What this tree actually says about FXC

Every `FXC` hit in `src/` (excluding `src/docs/old docs/`):

- `LAWS.md:30-49` — the law itself, quoted in §0.3.
- `world.wgsl:14-16` — *"L2 FXC — the Windows D3D12 backend's hard limits,
  honored by structure. READ L2 BEFORE adding a branch to the collision/
  ground chain or a texture-array stamp anywhere near it."*
- `world.wgsl:2405, 2949, 3043, 3763, 6697, 6757, 7489, 7493, 12412, 12464, 12685`
  — all about **branching**, **loop unrolling**, **uniform-bounded chains**,
  and **kernel splitting for compile time**. None about buffer size.
- `renderer.hpp:1288` — *"…never widens their FXC surface"*, about adding a
  **bind group** (a third group to the agent kernels), not about growing one.
- `renderer.hpp:1315-1505` — ~14 `ROSTER-GATE … FXC skipped when disabled`
  markers. The lever this tree uses against FXC is *not compiling the
  pipeline at all*.
- `seed_utils.hpp:12` — the bit-identical randomness mirror.
- `audit/past reports/BATCH_F_REPORT.md:88-94, 211-224` — the one precedent
  that pairs "FXC exposure" with the words "bind-group buffer growth"; it is a
  **buffer count** change (5 → 7 of 10). Quoted in §0.3.

**The per-stage ceilings are machine-measured, not assumed:**
`maxStorageBuffersPerShaderStage = 10`, `maxUniformBuffersPerShaderStage = 12`
(`audit/past reports/probe_results*.json:14-15`, restated `LEDGER_1_REPORT.md:821-822`).

**A boot-time baseline exists for the Stage B witness.**
`src/docs/HANDOFFS/PROCESS_LAWS.md:270-283`: pipeline compile is **55 s** total
post-`NDEBUG`, of which the FXC-heavy pipelines are `patch_terrain` 4.9 s,
`patch_terrain_indirect` 4.8 s, `monolith` 3.7 s, `pawn` 3.5 s. Stage B's
comparison is against the three gallery pipelines' rows in that leaderboard, not
against the total — the total is dominated by terrain and will not move.

**A2.5 — VERDICT. NO.** The painting-slot buffer is not in a bind group
shared with a pipeline this repo's documentation flags as FXC-sensitive. The
FXC-sensitive surface named by L2 and by `world.wgsl`'s own banner is the
collision/ground chain and the agent kernels — neither binds
`paintingSlotsBuffer_`. Group count is unchanged by Stage B; per-stage buffer
counts are unchanged; struct layout is unchanged.

**The witness Stage B asks for already exists.** `renderer.hpp:149-156`:

```cpp
template <typename F>
bool tPipe(const char* label, F&& fn) { ... 
    std::cout << "  [Pipeline] " << label << ": " << ms << " ms\n";
    pipelineTimings_.push_back({label, ms}); ... }
```

Every render and compute pipeline is created through it, and a sorted
leaderboard is printed at `renderer.hpp:341`. Stage B needs to add only the
`sizeof(...) * Dim::PAINTING_MAX_SLOTS` print, not a timing harness.

---

## §A3 — The texture arrays and their exact cost

All four painting textures are built by one lambda, `state.hpp:2323-2334`:

```cpp
auto makeTextureArray = [&](const char* label, uint32_t layers,
    wgpu::TextureUsage usage) -> wgpu::Texture {
        wgpu::TextureDescriptor desc{};
        desc.label = label;
        desc.size = { Dim::PAINTING_RESOLUTION, Dim::PAINTING_RESOLUTION, layers };
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.format = colorFormat;
        desc.usage = usage;
        return device_.CreateTexture(&desc);
    };
```

`colorFormat` is the **swapchain colour format**, passed into
`initOffscreenResources` (`state.hpp:2321-2322`) — `BGRA8Unorm` or
`RGBA8Unorm`, either way **4 B/texel**. `mipLevelCount` and `sampleCount` are
left at descriptor defaults, i.e. **1**. No mip generation exists anywhere in
the painting path.

| texture | site | layers | usage | bytes | MiB |
|---|---|---:|---|---:|---:|
| Snapshot Staging | `2344-2350` | 16 | `CopyDst \| CopySrc` | 1024·1024·16·4 = 67,108,864 | **64.0** |
| Authored Staging | `2352-2358` | 16 | `CopyDst \| CopySrc` | 67,108,864 | **64.0** |
| Exhibition | `2360-2366` | 32 | `CopyDst \| TextureBinding` | 1024·1024·32·4 = 134,217,728 | **128.0** |
| Offscreen Colour | `2368-2377` | 1 | `RenderAttachment \| CopySrc` | 4,194,304 | **4.0** |
| Offscreen Depth | `2379-2388` | 1 | `RenderAttachment` | see below | **≈4.0** |

Offscreen depth format is `wgpu::TextureFormat::Depth24Plus`
(`state.hpp:2383`) — a *format family*, not a byte width. Dawn resolves it to
a 32-bit depth format on D3D12 and Vulkan, so 4 B/texel is the working figure;
it is implementation-defined and should be read as ≈4.0 MiB, not exactly.

**The handoff's "~128 MiB (32 × 1024² × 4 B)" for the exhibition array is
EXACT.** 134,217,728 B = 128.0 MiB precisely.

**Painting-subsystem texture VRAM today: ≈264 MiB.**

At `EXHIBITION_LAYERS = 256` the exhibition array alone becomes
1024·1024·256·4 = 1,073,741,824 B = **1024.0 MiB**; subsystem total ≈ **1160
MiB**, a delta of **+896 MiB**. That is the measurement HELD exists to avoid.

**Stage B does not touch any of this.** It moves `paintingSlotsBuffer_` from
4 KiB to 32 KiB and nothing else. `EXHIBITION_LAYERS` stays 32.

---

## §A4 — `world.wgsl`: how the wall shaders consume the slot

### A4.1 — The three entry points

| entry | lines | shape |
|---|---|---|
| `wall_painting_vs` | `10178-10206` | `@vertex`, **not instanced** — decodes `vid` |
| `wall_painting_canvas_fs` | `10208-10220` | `@fragment` |
| `wall_painting_frame_fs` | `10222-10230` | `@fragment` |

Geometry helper `compute_wall_painting_geometry` at `10054-10176`;
`PAINTING_FRAME_VERTS_PER: u32 = 78u` at `10038` (the WGSL twin of
`state.hpp:260`, and it is a **third** unmirrored literal pair — but 78 is
not a value any stage moves, so it is noted only).

### A4.2 — The decode and the guard

```wgsl
fn wall_painting_vs(@builtin(vertex_index) vid: u32) -> WallPaintingVarying {
    let pidx = vid / PAINTING_FRAME_VERTS_PER;
    let local_vid = vid % PAINTING_FRAME_VERTS_PER;

    if (pidx >= PAINTING_MAX_SLOTS) { ... return out; }   // :10183

    let slot = painting_slots[pidx];                       // :10191

    if (slot.is_active == 0u || slot.form_type != FORM_WALL_FRAME) { ... }
```

The overflow branch returns a fully-zeroed varying with
`out.clip_pos = vec4(0.0)` — a degenerate vertex, discarded at
clip/rasterise. It is a correct, cheap early-out.

Vertex budget: `local_vid < 6` → canvas quad (6 verts); `< 54` → four frame
sides × 12; else → four bevel rings × 6. 6 + 48 + 24 = **78**. Matches
`PAINTING_FRAME_VERTS_PER`.

`world_pos.y += sample_live_card(out.world_pos.xz).x;` at `:10203` — **wall
frames ride the live card's ground term.** Indoor frames are lifted by
whatever the card holds under them. Relevant to Stage E: R4's floor margin is
not measured against a flat y=0.

### A4.3 — `texture_layer` and `uv_scale`

`uv_scale` is consumed once, in the canvas branch (`world.wgsl:10085`):

```wgsl
out.uv = uvs[ci] * vec2(s.uv_scale_x, s.uv_scale_y);
```

The frame branches write `out.uv = vec2(0.0)`. So `uv_scale_x/y` scale the
canvas UV rect only — which is exactly the mechanism HELD's sub-rect packing
would extend with `uv_offset_x/y`. The concept is already load-bearing for
authored aspect correction (`gallery.hpp:1359-1360` sets
`uv_scale = dst_w/RES, dst_h/RES` from the letterboxed disk image).

`texture_layer` is consumed at `world.wgsl:10213`:

```wgsl
let slot = painting_slots[in.painting_index];
let tex_color = textureSample(painting_array, painting_sampler_filt, in.uv, slot.texture_layer);
```

Note the fragment shader **re-reads the slot** rather than carrying the layer
in the varying — so `painting_slots` is bound `Vertex | Fragment`
(`state.hpp:4559`).

### A4.4 — THE KEY QUESTION: any slot↔layer bijection assumption?

**None. In any entry point.** Complete `texture_layer` census in `world.wgsl`:

| line | site | use |
|---|---|---|
| `9840` | `UnifiedPaintingSlot` | field decl |
| `9884` | `GalleryVarying` | `@interpolate(flat)` varying decl |
| `9928` | `gallery_frame_vs` | culled path writes `0u` |
| `9986` | `gallery_frame_vs` | `out.texture_layer = slot.texture_layer;` |
| `9998` | `gallery_frame_fs` | `textureSample(..., in.texture_layer)` |
| `10213` | `wall_painting_canvas_fs` | `textureSample(..., slot.texture_layer)` |

Every sample takes the layer **from the slot's field**. Nothing samples with
`iid`, `pidx`, or `in.painting_index` as the array layer. Nothing derives a
layer from a slot index or vice versa. Nothing sizes the exhibition array from
the slot count.

Also checked and cleared: `compute_entity_placement` (`world.wgsl:9573-9603`) is
the **only** shader path touching `photo_painting_slots`, and it reads only
`is_active`, `patch_gx`, `position`, `scale_y`, `form_type`, `frame_width` — it
never reads `texture_layer` and never samples `painting_array`. (The other two
`for (var i = 0u; i < 32u; i++)` loops nearby, `:9607` and `:9642`, are the
column and blade ground loops over unrelated buffers — see the replace-all trap
in §A10 Q5.)

**Verdict: the GPU is already sharing-safe.** Stage D is a pure CPU-side
change. See §0.7.

**One masking coincidence, named because Stage D is where it would bite.**
`Dim::PAINTING_MAX_SLOTS = 32` (`state.hpp:254`) and
`Dim::EXHIBITION_LAYERS = 32` (`state.hpp:257`) are **equal today**. So if a
slot index were ever mistakenly used as a layer — in C++ or in a future shader
edit — it would never index out of range. It would silently sample the wrong
picture, with no validation error and no crash. Stage B breaks that equality
(256 vs 32), which converts the same mistake from *wrong image* into
*out-of-range* — better, but only because the numbers stop matching.

### A4.5 — The terrain-quad path

`gallery_frame_vs` (`:9909-9930`) is **instanced**: `let slot = painting_slots[iid];`
with **no bounds guard** — the hazard in §0.2. It culls on
`slot.is_active == 0u || slot.form_type != FORM_TERRAIN_QUAD || !frame_in_ring`,
where `frame_in_ring` is the veil-ring draw authority (`:9920-9922`).
`uv_scale` is **not** applied on this path — outdoor snapshot quads always set
`uv_scale = 1.0` (`gallery.hpp:1163-1164`), and outdoor *authored* paintings go
through `fill_slot_wall_frame` (i.e. they are `WALL_FRAME` form, drawn by the
wall pipeline, not this one).

---

## §A5 — `fill_slot_wall_frame`, full body

`gallery.hpp:578-607`:

```cpp
inline void fill_slot_wall_frame(
    GPUPaintingSlot& s,
    float x, float y, float z,
    float nx, float ny, float nz,
    float aspect_ratio, float base_height,
    uint32_t layer, uint32_t content,
    float uv_sx, float uv_sy,
    const FrameStyle& frame,
    int32_t gx, int32_t gz
) {
    s = {};
    s.position[0] = x; s.position[1] = y; s.position[2] = z;
    s.forward[0] = nx; s.forward[1] = ny; s.forward[2] = nz;
    s.up[0] = 0.0f; s.up[1] = 1.0f; s.up[2] = 0.0f;
    s.form_type = FormType::WALL_FRAME;
    s.is_active = 1;
    s.scale_x = base_height * aspect_ratio;
    s.scale_y = base_height;
    s.texture_layer = layer;
    s.content_source = content;
    s.uv_scale_x = uv_sx;
    s.uv_scale_y = uv_sy;
    s.frame_depth = frame.depth;
    s.frame_width = frame.width;
    s.canvas_recess = frame.recess;
    s.frame_color[0] = frame.color[0];
    s.frame_color[1] = frame.color[1];
    s.frame_color[2] = frame.color[2];
    s.patch_gx = gx; s.patch_gz = gz;
}
```

Opens with `s = {}` — value-initialises all 128 bytes, so every field it does
not name is deterministically zero. **Fields not set:** `geometry_seed`,
`_pad0`, `_pad1`, `_pad2[4]`. `geometry_seed` is the only meaningful one; it
drives `deform_gallery_frame` (`world.wgsl:9888-9906`) which is read **only by
the terrain-quad path** — wall frames are undeformed by construction. The
outdoor authored path patches it in after the call
(`gallery.hpp:1122`: `s.geometry_seed = cpu_hash_f(p_seed, ...)`), the indoor
paths do not.

`scale_x = base_height * aspect_ratio` — **the helper derives width from the
real aspect.** This is the drift's other end; see §C.6.

**Note for Stage E:** this is the *only* slot-fill door the indoor path uses,
both content branches (`gallery.hpp:1713`, `:1767`). It takes `x,y,z` already
resolved. A two-tier hang adds no new fill function — it changes what is
passed. That is the "manipulate what exists" the handoff asks for.

**Frame styles** (`gallery.hpp:572-574`): `FRAME_AUTHORED` and
`FRAME_SNAPSHOT` are **byte-identical** — `{0.30, 0.45, 0.09, {0.25,0.15,0.08}}`
— with the comment *"same museum frame (content is different, ceremony is the
same)"*. D1 says hierarchy is carried by position and resolution. If Stage E
wants the fill tier to read as lesser, the frame is a third available axis and
it is currently unused.

---

## §A6 — `clear_wall_paintings`, full body

`gallery.hpp:1796-1810`:

```cpp
inline void clear_wall_paintings(GalleryState& gs, GalleryDeps* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
        if (gs.painting_slots[i].is_active != 0 &&
            gs.painting_slots[i].form_type == FormType::WALL_FRAME) {
            uint32_t exh = gs.painting_slots[i].texture_layer;
            if (exh < Dim::EXHIBITION_LAYERS) {
                gs.exhibition_occupied[exh] = false;
                gs.exhibition_count--;
            }
            gs.painting_slots[i].is_active = 0;
            c->gpuState_.deactivate_painting_slot(queue, i);
        }
    }
    gs.wall_frame_count = 0;
}
```

**Does it free exhibition layers? Yes** — one per cleared slot, by the rule
*"the layer this slot's `texture_layer` names"*, bounds-checked against
`EXHIBITION_LAYERS`.

**What it does not do**, against `teardown_gallery` (`gallery.hpp:1865-1893`):
does not reset `active_painting_count`, does not touch `gallery_centers`, does
not clear `pending_promotion_count`, does not clear `staging_reserved`, does
not reset `snapshot_staging[].consumed` or `authored_staging[].consumed`, does
not zero the *whole* slot (only `is_active`), does not rotate authored staging.

**Latent defect, reported not fixed.** The predicate is
`form_type == WALL_FRAME` with **no patch filter**. Outdoor *authored* gallery
paintings are also `WALL_FRAME` — `commit_gallery:1115` fills them through
`fill_slot_wall_frame` with real `patch_gx/gz`, increments `wall_frame_count`
(`:1128`) **and** `active_painting_count` (`:1177`). So a
`clear_wall_paintings` running while outdoor authored monuments are alive
would deactivate them, free their exhibition layers, leave their
`gallery_centers` entry active and their footprint registered, and leak
`active_painting_count` (never decremented here).

**Reachability today: none.** Both call sites are downstream of `apply_mood`
(`gallery.hpp:1527` inside `place_wall_paintings`; `mood.hpp:685` inside
`clear_indoor_shell`), and `apply_mood` has exactly two callers —
`cartridge.hpp:537` (boot, which resolves to `MOOD_OPEN_SUNSET` and so never
reaches the place arm — see §A7) and `cartridge.hpp:1021`, which runs **after**
`teardown_gallery` at `cartridge.hpp:987`. So the slot array is always empty
when either fires. The defect is armed, not firing.

It arms harder under R1, because R1 makes the wall-frame population large and
the `active_painting_count` leak proportional to it — and because Stage D's
whole premise is that a `WALL_FRAME` slot's `texture_layer` may be shared, which
turns this function's per-slot free into the §A9 Q3 defect as well. Two reasons
to look at `clear_wall_paintings` again before Stage E, neither of them Stage
A's to act on.

Corroboration that the form-type overlap is real, from the tree itself:
`evict_paintings_for_patch` carries an explicit
`if (... form_type == FormType::WALL_FRAME) gs.wall_frame_count--;` branch
(`gallery.hpp:1218-1220`) keyed on `patch_gx/gz` — reachable **only** for slots
`commit_gallery` produced, since both `place_wall_paintings` fill sites pass
`INT32_MAX, INT32_MAX` (`gallery.hpp:1720`, `:1774`). The outdoor path knowingly
makes `WALL_FRAME` slots. `clear_wall_paintings` is the one site that filters on
form type **without** also filtering on patch.

---

## §A7 — Every caller of `place_wall_paintings` / `clear_wall_paintings`

Complete census, whole repo:

| path:line | symbol | kind |
|---|---|---|
| `gallery.hpp:136` | `place_wall_paintings` | comment |
| `gallery.hpp:518` | `place_wall_paintings` | declaration |
| `gallery.hpp:520` | `clear_wall_paintings` | declaration |
| `gallery.hpp:1525` | `place_wall_paintings` | **definition** |
| `gallery.hpp:1527` | `clear_wall_paintings` | **CALL** (from `place_wall_paintings`, defensive self-clear) |
| `gallery.hpp:1796` | `clear_wall_paintings` | **definition** |
| `mood.hpp:685` | `clear_wall_paintings` | **CALL** — in `clear_indoor_shell` |
| `mood.hpp:841` | `place_wall_paintings` | **CALL** — in `generate_indoor_shell` |

**The one live placement call**, `mood.hpp:841`:

```cpp
place_wall_paintings(gallery_state, &gallery_deps, queue, bmin, bmax, ch);
```

**Argument provenance** — all three are locals of `generate_indoor_shell`,
`mood.hpp:711-713`:

```cpp
float bmin = -(float)c->world_state_.finite_radius * Dim::PATCH_EXTENT;
float bmax = ((float)c->world_state_.finite_radius + 1.0f) * Dim::PATCH_EXTENT;
float ch = m.ceiling_height;
```

- `Dim::PATCH_EXTENT = 50.0f` (`state.hpp:69`).
- `finite_radius` from `derive_finite_radius` (`mood.hpp:1187`):
  `mood.finite_radius_min + cpu_hash(seed, 77u) % range`. Both indoor moods
  carry `finite_radius_min = 1, finite_radius_max = 4`
  (`spine_state.hpp:201-202`), so **`finite_radius ∈ {1,2,3,4}`, uniform**.
- `m.ceiling_height` — **20.0f** for `MOOD_INDOOR_FLAT`, **25.0f** for
  `MOOD_INDOOR_VAULT`, pinned by `static_assert` at `spine_state.hpp:224-225`.

**The resulting room, exactly:**

| `finite_radius` | `bmin` | `bmax` | `span` | `wall_center` | `usable_span` |
|---:|---:|---:|---:|---:|---:|
| 1 | −50 | 100 | 150 | 25 | 126 |
| 2 | −100 | 150 | 250 | 25 | 226 |
| 3 | −150 | 200 | 350 | 25 | 326 |
| 4 | −200 | 250 | 450 | 25 | 426 |

`usable_span = max(span − 2·12, span·0.3)` (`gallery.hpp:1609-1610`) — the
`span − 24` arm always wins; the `0.3` floor is dead at every legal radius.

**`wall_center = 25.0` at every radius.** The room is not centred on the
origin — `bmin = −50r`, `bmax = 50r + 50`. Stage E's centre-reserved sub-rect
must use `wall_center`, not 0.

**Control flow.** `place_wall_paintings` runs **exactly once per indoor mood
entry**, and only when the mood carries a ceiling:

```
apply_mood                              mood.hpp:649
 └─ if constexpr (ROSTER.indoor_shell)  mood.hpp:669
     └─ apply_mood_indoor_shell         mood.hpp:621
         ├─ if (m.indoor && m.ceiling_type != CeilingType::NONE)
         │   └─ generate_indoor_shell   mood.hpp:708
         │       └─ place_wall_paintings  mood.hpp:841   ← once
         └─ else
             └─ clear_indoor_shell      mood.hpp:682
                 └─ clear_wall_paintings  mood.hpp:685
```

The two branches are exclusive, there is no loop, and `apply_mood` has two
callers (`cartridge.hpp:537` boot, `cartridge.hpp:1021` transition). It cannot
run twice for one entry.

**And the boot caller never reaches it.** `cartridge.hpp:537` passes
`mood_state_.active`, authored at `cartridge.hpp:411` from `DEMO.boot_mood`;
both demo columns carry `MOOD_OPEN_SUNSET`
(`matrix.hpp:105-108`), pinned by `static_assert` at `matrix.hpp:154-159`. That
mood is `indoor = false`, so boot always takes the `clear_indoor_shell` arm.

**Therefore `place_wall_paintings` runs from exactly one live path: a mood
transition — and `cartridge.hpp:987` calls `teardown_gallery` before
`cartridge.hpp:1021` calls `apply_mood`.** The slot array is always empty when
the wall is hung. That is the fact §A6's latent defect rests on, and the fact
Stage E can rely on: a placement event starts from a clean slot pool every time.

`bmin`/`bmax` are **runtime**, not compile-time — `world_state_.finite_radius`
(default 2, `surface_services.hpp:45`) is written at `cartridge.hpp:966` from
`pendingDestination_.finite_radius`, inside the same teardown arm and therefore
already live when `generate_indoor_shell` reads it. `ceiling_h` is compile-time.

**How a transition is armed, and how Jean will reach the visual gate.**
`apply_mood` still has exactly the two callers above — but the *transition* one
is armed from two doors, and one of them is the keyboard:

- **ENTRY door #1**, `request_mood_transition` (`mood.hpp:1148-1173`, so
  labelled at `:1149`) — bound to **`GLFW_KEY_6` → `MOOD_INDOOR_FLAT`** and
  **`GLFW_KEY_7` → `MOOD_INDOOR_VAULT`** (`input.hpp:246-247`; `5` and `8` take
  the two outdoor moods).
- **ENTRY door #2**, the portal path (`cartridge.hpp:1247-1258`).

Both only set `pendingDestination_` and `phase = TransitionPhase::FADE_OUT`
(`mood.hpp:1161`, `cartridge.hpp:1258`); neither calls `apply_mood`. Both then
run the *same* block at `cartridge.hpp:960-1040` — `teardown_gallery` at `:987`,
`apply_mood` at `:1021`. **The teardown-before-place ordering is unconditional.**

Two consequences worth having before Stage E's visual gate:

- Keys **6** and **7** are the fastest route into a hung wall. No portal needed.
- Each keypress rolls a **fresh** `finite_radius` (`mood.hpp:1159`), so the room
  size changes every time — which is what makes the §E.1 coverage table's
  spread visible by just pressing 6 repeatedly. The **back portal** is the
  exception: it replays the saved radius (`mood.hpp:944`, `:973`, captured at
  `cartridge.hpp:962`), so returning through one reproduces the same room —
  the reproducible test case for an A/B look.

**Why `src/incubator_dual.cpp` is in scope:** it holds no gallery or painting
code at all (grepped: zero hits for `gallery|painting|Gallery|Painting`). It is
the harness door — `incubator_dual.cpp:183` calls
`render.init_renderer(console.color_format(), console.depth_format())`, which is
the `cartridge.hpp:492` function that runs `initOffscreenResources` (§A3) and
the boot `apply_mood`. It is the natural home for Stage B's `sizeof` witness
print, and nothing else in this campaign touches it.

**Indoor-ness** is `MoodProfile::indoor` (`spine_state.hpp:174`) — `true` for
`MOOD_INDOOR_FLAT (1)` and `MOOD_INDOOR_VAULT (2)`, `false` for
`MOOD_OPEN_SUNSET (0)` and `MOOD_FINITE_OUTDOOR (3)` (`spine_state.hpp:200-203`).
Both indoor rows carry a non-`NONE` `ceiling_type`, so for those two moods the
`place` branch always wins.

---

## §A8 — `PhotographerCaptureConfig`, and what supply costs

`gallery.hpp:140-164`, verbatim:

```cpp
struct PhotographerCaptureConfig {
    static constexpr float TRIGGER_DISTANCE_MEAN = 50.0f;
    static constexpr float TRIGGER_DISTANCE_SIGMA = 8.0f;
    static constexpr float TRIGGER_DISTANCE_FLOOR = 20.0f;

    static constexpr float BURST_WEIGHT_1 = 0.40f;
    static constexpr float BURST_WEIGHT_2 = 0.70f;
    static constexpr float BURST_WEIGHT_3 = 0.90f;
    static constexpr uint32_t BURST_MAX = 4;
    static constexpr uint32_t BURST_COOLDOWN_FRAMES = 12;

    static constexpr float DISTANCE_FLOOR = 0.5f;
    static constexpr float ELEVATION_FLOOR = 0.005f;
    static constexpr float FOV_FLOOR = 15.0f;

    static constexpr float WIDE_LENS_CHANCE = 0.10f;
    static constexpr float WIDE_LENS_FOV_LO = 90.0f;
    static constexpr float WIDE_LENS_FOV_HI = 110.0f;
};
```

**`BURST_COOLDOWN_FRAMES = 12`.** Burst distribution
(`sample_shot_count`, `gallery.hpp:382-388`): the weights are **cumulative
thresholds**, so P(1)=0.40, P(2)=0.30, P(3)=0.20, P(4)=0.10.

**E[shots per trigger] = 1(0.40) + 2(0.30) + 3(0.20) + 4(0.10) = 2.00.**

Trigger cadence: `TRIGGER_DISTANCE_MEAN = 50.0` world units, gaussian σ 8,
floored at 20, **modulated by archetype pace** `{0.7, 0.8, 1.5, 1.5}`
(`GalleryConfig::PHOTO_PACE_BY_ARCHETYPE`, `gallery.hpp:222`, applied at
`gallery.hpp:653-657`).

**To fill 16 staging layers: 8 trigger events ≈ 400 world units walked**
(at pace 1.0; 280 in mountain terrain, 600 in basin/pool). A burst of 4 takes
36 frames to drain (12-frame cooldown between shots, `gallery.hpp:634-639`).
Steps > 5.0 wu in one frame are discarded (`gallery.hpp:627`), so teleports do
not bank distance.

**The freshness window, and the number that actually bounds Stage E.**
`snapshot_staging` is `Dim::STAGING_LAYERS = 16` deep, circular, unconditional
overwrite (`gallery.hpp:686-697`). The indoor path draws by **first-fit linear
scan** over `valid && !consumed` (`gallery.hpp:1687-1692`). Authored staging is
also 16 (`load_authored_textures:1436` loads `min(manifest_size, 16)`).

**So the maximum number of DISTINCT images an indoor site can hang today is
16 + 16 = 32** — exactly `EXHIBITION_LAYERS`. That is not a coincidence, and
it is the ceiling the HELD stage's trigger condition is written against.

**One nuance the handoff does not name.** `gs.snapshot_count` is a
**high-water mark**, not a live count: `if (gs.snapshot_count < Dim::STAGING_LAYERS)
gs.snapshot_count++` (`gallery.hpp:697`) and it is never decremented. The
indoor site-type gate reads it (`gallery.hpp:1543,1547`: `&& gs.snapshot_count > 0`),
as does the outdoor pool gate (`gallery.hpp:737`: `< MIN_POOL_SIZE`). Both can
therefore pass with **zero unconsumed records** available. The
`snap_stg == UINT32_MAX` fallback at `gallery.hpp:1693` exists precisely
because of this. Untouched by this campaign (supply is out of scope) but Stage
E should not read `snapshot_count` as available supply.

---

## §A9 — THE WELD CENSUS (Stage D gate)

Complete. Every occurrence of all six symbols, whole repo. Everything lives in
`gallery.hpp` except one drain call.

### (a) `exhibition_occupied` — 9 occurrences

| path:line | kind | enclosing | what |
|---|---|---|---|
| `gallery.hpp:480` | DECLARATION | `GalleryState` | `bool exhibition_occupied[Dim::EXHIBITION_LAYERS]{}` |
| `gallery.hpp:536` | READ | `find_free_exhibition_layer` | first-fit scan |
| `gallery.hpp:1124` | WRITE `true` | `commit_gallery` (authored) | claim |
| `gallery.hpp:1167` | WRITE `true` | `commit_gallery` (snapshot) | claim |
| `gallery.hpp:1214` | **WRITE `false`** | `evict_paintings_for_patch` | **free** |
| `gallery.hpp:1722` | WRITE `true` | `place_wall_paintings` (snapshot) | claim |
| `gallery.hpp:1776` | WRITE `true` | `place_wall_paintings` (authored) | claim |
| `gallery.hpp:1802` | **WRITE `false`** | `clear_wall_paintings` | **free** |
| `gallery.hpp:1889` | WRITE `false` ×N | `teardown_gallery` | blanket clear |

### (b) `exhibition_count` — 8 occurrences, **zero reads**

| path:line | kind | enclosing |
|---|---|---|
| `gallery.hpp:481` | DECLARATION | `GalleryState` |
| `gallery.hpp:1125` | WRITE (`++`) | `commit_gallery` (authored) |
| `gallery.hpp:1168` | WRITE (`++`) | `commit_gallery` (snapshot) |
| `gallery.hpp:1215` | WRITE (`--`) | `evict_paintings_for_patch` |
| `gallery.hpp:1723` | WRITE (`++`) | `place_wall_paintings` (snapshot) |
| `gallery.hpp:1777` | WRITE (`++`) | `place_wall_paintings` (authored) |
| `gallery.hpp:1803` | WRITE (`--`) | `clear_wall_paintings` |
| `gallery.hpp:1890` | WRITE (`= 0`) | `teardown_gallery` |

### (c) `find_free_exhibition_layer` — 5

`:534` definition; calls at `:1105` (commit/authored), `:1138`
(commit/snapshot), `:1698` (wall/snapshot), `:1750` (wall/authored).

### (d) `queue_promotion` — 5

`:540` definition; calls at `:1127`, `:1170`, `:1725`, `:1779` — the same four
placement sites, each immediately after its `exhibition_occupied[exh] = true`.

### (e) `drain_gallery_promotions` — 3

`:525` declaration, `:1898` definition, **`cartridge.hpp:1720` the single
call** (per frame, in the encoder, after `render_snapshot_pass` per the O-7
ordering note at `:1896`).

### (f) `texture_layer` (C++) — 5

| path:line | kind | enclosing |
|---|---|---|
| `state.hpp:1622` | DECLARATION | `GPUPaintingSlot` |
| `gallery.hpp:596` | **WRITE** | `fill_slot_wall_frame` (`s.texture_layer = layer`) |
| `gallery.hpp:1159` | **WRITE** | `commit_gallery` snapshot path (`s.texture_layer = exh`) |
| `gallery.hpp:1212` | **READ** | `evict_paintings_for_patch` |
| `gallery.hpp:1800` | **READ** | `clear_wall_paintings` |

Two writers, two readers. WGSL side in §A4.4 — six sites, all
layer-from-field.

### Q1 — Is `exhibition_count` ever read?

**No. Never. Not once.** The table in (b) is complete and contains no READ
row. It appears in no `std::cout`, no comparison, no return, no assert. The
three gallery log lines (`gallery.hpp:725`, `:1196`, `:1791`) print
`snapshot_count`, `placed`/`painting_count`, and `wall_frame_count`
respectively — never `exhibition_count`.

It is a write-only counter. Under sharing it would become meaningless; since
nothing reads it, nothing breaks.

### Q2 — Which sites free a layer, and could they free a shared one?

| site | rule | shared-safe? |
|---|---|---|
| `evict_paintings_for_patch:1214` | for each active slot with matching `patch_gx/gz`, free `slot.texture_layer` | **NO** — frees per-slot with no check that another live slot names the same layer |
| `clear_wall_paintings:1802` | for each active `WALL_FRAME` slot, free `slot.texture_layer` | **NO** — same |
| `teardown_gallery:1889` | blanket: all `EXHIBITION_LAYERS` to `false` | **YES** — it also zeroes every slot (`:1881-1883`) and re-uploads an empty array (`:1885-1887`), so no slot survives to dangle |

Under today's bijection all three are correct because the layer has exactly
one owner. The bijection is enforced **only** by the occupancy array — nothing
in `GPUPaintingSlot` or in either write site requires `texture_layer` to be
unique.

### Q3 — Per-site verdict if many slots may share one layer

| site | verdict |
|---|---|
| `find_free_exhibition_layer:534` | **needs a new meaning.** "free" must become "referenced by nothing", which is a refcount question, not a bool question. |
| `queue_promotion:540` | correct as-is; but the *caller* must stop issuing one promotion per slot. |
| `commit_gallery:1124/1167` claims | correct as-is (claim is idempotent). |
| `place_wall_paintings:1722/1776` claims | correct as-is. |
| **`evict_paintings_for_patch:1214`** | **INCORRECT.** Frees a layer another patch's slot may still point at → that slot renders a layer that will be overwritten by the next promotion. Outdoor path; perturbed by Stage D *by sharing*, per the handoff's own OUT OF SCOPE clause. |
| **`clear_wall_paintings:1802`** | **INCORRECT.** Same failure, indoor. Worse under R1: the whole point of Stage D is that a wall's frames share layers, so clearing one wall frees layers another wall still uses. |
| `teardown_gallery:1889` | **CORRECT under sharing**, unchanged — it destroys slots and layers together. |
| `exhibition_count` ± | meaningless under sharing, harmless (no reader). |
| WGSL sample sites | **CORRECT under sharing**, unchanged (§0.7). |

**Two sites, both frees.** That is the whole surface. The reader set does not
by itself decide between a refcount and a placement-scoped map — that is
Jean's ruling — but the census says the choice is small: a refcount would need
increments at four claim sites and decrements at exactly the two frees above,
plus the blanket reset. A placement-scoped map would cover
`place_wall_paintings` alone and leave `evict_paintings_for_patch` on the
current bijection, which is consistent only if outdoor placement never shares.
**Reported. Not chosen.**

---

## §A10 — THE SLOT-COUNT CENSUS (Stage B gate)

### (a) `PAINTING_MAX_SLOTS` — every occurrence, C++ and WGSL

| path:line | kind | what changes at 256 |
|---|---|---|
| `state.hpp:254` | **DEFINITION** `constexpr uint32_t PAINTING_MAX_SLOTS = 32;` | the edit |
| `state.hpp:261` | READ (composes `PAINTING_FRAME_VERTEX_COUNT`) | 2,496 → 19,968 |
| `state.hpp:2395` | BUFFER-SIZE (bind-group entry, Gallery Texture) | 4 KiB → 32 KiB |
| `state.hpp:2942` | READ (`painting_max_slots()` accessor) | **dead — no caller** |
| `state.hpp:3277` | BUFFER-SIZE (`makeBuffer` size arg) | 4 KiB → 32 KiB |
| `state.hpp:5610` | BUFFER-SIZE (bind-group entry, Entity Placement) | 4 KiB → 32 KiB |
| `state.hpp:6169` | **STACK ARRAY** `GPUPaintingSlot emptySlots[...]` | 4 KiB → 32 KiB stack |
| `state.hpp:6170` | LOOP-BOUND (boot zero-fill) | boot only |
| `state.hpp:6174` | BUFFER-SIZE (`WriteBuffer` length) | 4 KiB → 32 KiB |
| `renderer.hpp:922` | **DRAW-ARG** (instance count) | 32 → 256 instances; **see §0.2** |
| `gallery.hpp:486` | MEMBER ARRAY `GPUPaintingSlot painting_slots[...]` | 4 KiB → 32 KiB in `GalleryState` |
| `gallery.hpp:552` | LOOP-BOUND `find_free_painting_slot` | per painting placed |
| `gallery.hpp:754` | LOOP-BOUND `select_gallery_for_patch` idempotency scan | **per patch-selection attempt** — the hot one |
| `gallery.hpp:1207` | LOOP-BOUND `evict_paintings_for_patch` | per eviction |
| `gallery.hpp:1797` | LOOP-BOUND `clear_wall_paintings` | per indoor entry |
| `gallery.hpp:1881` | LOOP-BOUND `teardown_gallery` | per transition |
| `gallery.hpp:1885` | **STACK ARRAY** `GPUPaintingSlot empty[...]` | 4 KiB → 32 KiB stack |
| `gallery.hpp:1886` | COUNT-ARG (`upload_painting_slots`) | 32 → 256 |
| **`world.wgsl:9865`** | **DEFINITION (WGSL)** `const PAINTING_MAX_SLOTS: u32 = 32u;` | **not moved by a C++ edit** |
| **`world.wgsl:10183`** | GUARD `if (pidx >= PAINTING_MAX_SLOTS)` | still clamps at 32 |
| **`world.wgsl:9869`** | **BARE `32`** — `array<UnifiedPaintingSlot, 32>` | shader still sees 32 slots |
| **`world.wgsl:9325`** | **BARE `32`** — `array<UnifiedPaintingSlot, 32>` (compute) | Y-correction still sees 32 |
| **`world.wgsl:9573`** | **BARE `32`** — `for (var i = 0u; i < 32u; i++)` | Y-correction loop still 32 |
| `src/docs/HANDOFFS/A FEW TWEAKS/cc_handoff_tweaks_A.txt:202` | doc prose | — |

### (b) `PAINTING_FRAME_VERTEX_COUNT` and the vertex constants

| path:line | kind |
|---|---|
| `state.hpp:258` | DEFINITION `PAINTING_QUAD_N = 8` |
| `state.hpp:259` | DEFINITION `PAINTING_QUAD_VERTS = 8·8·6 = 384` |
| `state.hpp:260` | DEFINITION `PAINTING_FRAME_VERTS_PER = 78` |
| `state.hpp:261` | DEFINITION `PAINTING_FRAME_VERTEX_COUNT = MAX_SLOTS · 78` = **2,496** today |
| `state.hpp:2941` | accessor `painting_quad_verts()` — **dead, no caller** |
| `state.hpp:2943` | accessor `painting_frame_vertex_count()` — **dead, no caller** |
| `renderer.hpp:922` | DRAW-ARG (vertex count 384) |
| `renderer.hpp:938` | DRAW-ARG (canvas pass) |
| `renderer.hpp:944` | DRAW-ARG (frame pass) |
| `world.wgsl:9875` | WGSL twin `GALLERY_QUAD_N: u32 = 8u` |
| `world.wgsl:9933` | read |
| `world.wgsl:10038` | WGSL twin `PAINTING_FRAME_VERTS_PER: u32 = 78u` |
| `world.wgsl:10180-10181` | the `vid` decode |

`PAINTING_QUAD_N`/`GALLERY_QUAD_N` and `PAINTING_FRAME_VERTS_PER` (both rooms)
are a second and third unmirrored literal pair. Neither moves in this
campaign; recorded for completeness.

### (c) `MAX_PROMOTIONS_PER_FRAME` — 3 occurrences, all in `gallery.hpp`

| path:line | kind |
|---|---|
| `:428` | **DEFINITION** `inline constexpr uint32_t MAX_PROMOTIONS_PER_FRAME = 32;` |
| `:483` | MEMBER ARRAY `PendingPromotion pending_promotions[MAX_PROMOTIONS_PER_FRAME]{}` |
| `:542` | GUARD `if (gs.pending_promotion_count < MAX_PROMOTIONS_PER_FRAME)` — **the silent drop** |

### Q1 — Stack arrays

Two, both `GPUPaintingSlot[Dim::PAINTING_MAX_SLOTS]`, element size 128 B:

| site | 32 | 256 |
|---|---:|---:|
| `gallery.hpp:1885` (`teardown_gallery`) | 4,096 B | 32,768 B |
| `state.hpp:6169` (boot zero-fill) | 4,096 B | 32,768 B |

Neither is near 1 MiB. Nothing else is stack-allocated from this constant.
`GalleryState::painting_slots` (`gallery.hpp:486`) is a **member**, not stack —
32 KiB inside `GalleryState`, which lives in the cartridge object.

### Q2 — Buffer sizes and draw counts

| what | 32 | 256 |
|---|---:|---:|
| `paintingSlotsBuffer_` | 4,096 B | 32,768 B |
| Gallery Texture binding size | 4,096 B | 32,768 B |
| Entity Placement binding size | 4,096 B | 32,768 B |
| `draw_gallery_frames` | 384 verts × 32 inst = 12,288 | 384 × 256 = 98,304 |
| `draw_wall_paintings` (×2 passes) | 2,496 each = 4,992 | 19,968 each = 39,936 |
| **total painting vertex invocations/frame** | **17,280** | **138,240** |

Against terrain, still noise — the handoff's "the cost of this campaign is
VRAM and supply, not frame time" holds. But note the added 120,960
invocations at 256 are **all wasted** unless WGSL moves too (§0.2).

### Q3 — Linear scans

| site | bound | frequency |
|---|---|---|
| `gallery.hpp:552` `find_free_painting_slot` | 256 | once per painting placed |
| **`gallery.hpp:754`** idempotency scan | 256 | **per gallery-selection attempt** — the spawn engine calls this per candidate patch |
| `gallery.hpp:1207` `evict_paintings_for_patch` | 256 | per patch eviction |
| `gallery.hpp:1797` `clear_wall_paintings` | 256 | per indoor mood entry |
| `gallery.hpp:1881` `teardown_gallery` | 256 | per mood transition |
| `state.hpp:6170` | 256 | boot |

`gallery.hpp:754` is the only per-frame-ish one, and 256 iterations of two
int compares is nothing. Flagged for the record, not as a concern.

### Q4 — Is the WGSL constant synced by any mechanism?

**No. Definitively no.** Evidence:

- No codegen: `world.wgsl` is loaded as a source file
  (`renderer.hpp:142-143`, `shaderSource_`/`shaderPath_`); there is no
  generation or substitution step.
- No `static_assert` can reach it — it is in a different language.
- No test: `audit/tools/glaw2/run.py` is the only WGSL checker; its own header
  (`run.py:16-23`) states it proves delimiters balance, names resolve, and the
  entry-point set matches baseline — *"types, address spaces, override-expression
  legality, uniform layout rules"* explicitly excluded. `PAINTING_MAX_SLOTS`
  appears in `baseline.json:289` only as a declared-symbol name.
- No comment ties them. `world.wgsl:9831` says the *struct* must match
  `state.hpp`; nothing says the *count* must.
- The general rule that governs is **L3** (`LAWS.md:51`), which is written
  down precisely because *"the WGSL room is held by nothing the compiler can
  see"*.

**If C++ goes to 256 and WGSL stays at 32:**

1. `paintingSlotsBuffer_` becomes 32 KiB; both bind-group entries bind all
   32 KiB. Dawn validates a fixed-size WGSL array binding as
   `bindingSize ≥ arraySize`; larger is legal. **No validation error, no
   warning.**
2. CPU places up to 256 paintings and uploads all of them.
3. Slots 32..255 are **invisible** — `painting_slots` and `photo_painting_slots`
   are `array<..., 32>`, and `compute_entity_placement`'s loop stops at 32, so
   slots 32+ never even get their ground Y corrected.
4. `wall_painting_vs` guards at 32 → those frames are silently degenerate.
5. `gallery_frame_vs` does **not** guard → instances 32..255 clamp to slot 31
   and re-draw it (§0.2).

**Net: a silent capacity no-op with a terrain-quad duplication artefact.**

### Q5 — Verbatim anchors

`state.hpp:254`:
```
            constexpr uint32_t PAINTING_MAX_SLOTS = 32;       // max exhibited paintings
```
(12 leading spaces; `PAINTING_MAX_SLOTS = 32` appears once in `state.hpp`.)

`gallery.hpp:428`:
```
inline constexpr uint32_t MAX_PROMOTIONS_PER_FRAME = 32;
```
(column 0; unique in the repo.)

The WGSL anchors, if Stage B is amended to include them:
`world.wgsl:9865` `const PAINTING_MAX_SLOTS: u32 = 32u;` (unique);
`world.wgsl:9869` and `:9325` both end `array<UnifiedPaintingSlot, 32>;` —
**that substring appears exactly twice**, distinguished only by the full
binding line, so no blind replace-all;
`world.wgsl:9573` `    for (var i = 0u; i < 32u; i++) {` — the pattern
`for (var i = 0u; i < 32u; i++)` appears **five times** in `world.wgsl`
(`:2484, :9573, :9607, :9642`, and the two-space variant), and only `:9573` is
the painting loop. `:9607` and `:9642` are the column and plant loops, which
are 32 for `MAX_COLUMN_INSTANCES`, an unrelated 32. **A blind replace-all here
would corrupt two other subsystems.**

### The identity the handoff names, confirmed

Every `queue_promotion` call (§A9(d): `:1127, :1170, :1725, :1779`) is
immediately preceded by a `find_free_exhibition_layer` and immediately
followed or preceded by exactly one slot fill. There is no path that promotes
without filling a slot, and none that fills a slot without promoting. So
**promotions per frame ≤ total slots** is a theorem, not a coincidence, and
`MAX_PROMOTIONS_PER_FRAME = Dim::PAINTING_MAX_SLOTS` states it. Confirmed.

Cost of that identity at 256: `pending_promotions` grows from 32 to 256
entries of `PendingPromotion` (`bool` + 2 × `uint32_t`, 12 B padded) — from
384 B to 3,072 B inside `GalleryState`. Negligible.

---

## §A11 — Draw shape

### Definitions

`renderer.hpp:911-923`:
```cpp
void draw_gallery_frames(pass, galleryEntityBindGroup, galleryTextureBindGroup,
                         uint32_t activePaintingCount) {
    if constexpr (!(ROSTER.gallery)) return;
    if (activePaintingCount == 0) return;
    pass.SetPipeline(galleryFramePipeline_);
    pass.SetBindGroup(0, galleryEntityBindGroup);
    pass.SetBindGroup(1, galleryTextureBindGroup);
    pass.Draw(Dim::PAINTING_QUAD_VERTS, Dim::PAINTING_MAX_SLOTS);
}
```

`renderer.hpp:925-945`:
```cpp
void draw_wall_paintings(pass, galleryEntityBindGroup, galleryTextureBindGroup,
                         uint32_t wallFrameCount) {
    if constexpr (!(ROSTER.gallery)) return;
    if (wallFrameCount == 0) return;
    pass.SetPipeline(wallPaintingCanvasPipeline_);
    pass.SetBindGroup(0, ...); pass.SetBindGroup(1, ...);
    pass.Draw(Dim::PAINTING_FRAME_VERTEX_COUNT);
    pass.SetPipeline(wallPaintingFramePipeline_);
    pass.SetBindGroup(0, ...); pass.SetBindGroup(1, ...);
    pass.Draw(Dim::PAINTING_FRAME_VERTEX_COUNT);
}
```

| | draws | kind | verts | instances | pipelines |
|---|---:|---|---:|---:|---|
| `draw_gallery_frames` | **1** | `Draw` (non-indexed, **instanced**) | 384 | 32 | 1 |
| `draw_wall_paintings` | **2** | `Draw` (non-indexed, **non-instanced**) | 2,496 | 1 | 2 |

Neither is indexed. Neither is indirect. No vertex buffers
(`desc.vertex.bufferCount = 0` at `renderer.hpp:1886, 1936, 1964`) — all
geometry is procedural from `vertex_index` / `instance_index`.

The **two-pass split** for wall paintings is a fragment-shader fork over the
same vertex shader: `wall_painting_canvas_fs` discards when `is_canvas == 0`,
`wall_painting_frame_fs` discards when `is_canvas == 1`. Both passes run the
full 2,496-vertex geometry. **Half of each pass's raster work is discarded by
construction.** Not a defect — it is how one VS serves two materials without a
material index — but it doubles the cost of any slot-count growth.

### Call sites

Both called exactly once, from `render_passes.hpp:482-495`, in the main render
pass, in the "FORKS — the specials" block after `draw_table(..., DRAW_MAIN)`
and before orbs and the fade overlay. Gating:
`c->gallery_state_.wall_frame_count` and
`c->gallery_state_.active_painting_count` respectively (zero → early return).

### Pipeline state

All three painting pipelines: `PrimitiveTopology::TriangleList`,
`CullMode::None` (commented *"visible from both sides (outdoor monuments)"*),
`FrontFace::CCW`, depth `format = depthFormat_`, `depthWriteEnabled = true`,
`depthCompare = Less`. `"Gallery Frame"` carries an explicit alpha blend
(`SrcAlpha`/`OneMinusSrcAlpha`, `renderer.hpp:1864-1868`); the two wall
pipelines set **no** blend state (opaque), consistent with
`render_passes.hpp:476-478` calling them opaque.

### Bind-group budget

All three use **2 groups** (`galleryEntityLayout_` = 0, `galleryTextureLayout_` = 1),
`renderer.hpp:1852-1857` and `1907-1912`. The device requests full adapter
limits (`console.hpp:266-269`), and WebGPU's guaranteed `maxBindGroups` is 4.

**So there are at least 2 free bind-group slots on every painting pipeline**,
and per-stage storage-buffer headroom of 8 (vertex) / 6 (fragment) against
L2.4's 10. If the reallocation campaign frees binding numbers, this family can
spend them without touching group count. Recorded as asked; no stage in this
campaign needs them.

### Q6 — All slots, or an active count?

**All slots, unconditionally.** Both draws are sized from
`Dim::PAINTING_MAX_SLOTS`; the `activePaintingCount` / `wallFrameCount`
parameters are used **only** as a zero-check, never as a count. Culling of
inactive slots happens in the shader (`world.wgsl:9923` and `:10194`).

That is why Stage B's draw cost scales 8× with the constant even when nothing
more is placed, and why §0.2's terrain-quad hazard exists at all.

---

## §A12 — `SHADOW_WALL_PAINTING_VS`

**Verdict: it has no live caller — because it has no definition, no
declaration, and no shader entry point. It was cut.**

Complete hit list, whole repo:

| path:line | kind |
|---|---|
| `renderer.hpp:72` | **cut-marker comment** — `// SHADOW_WALL_PAINTING_VS CUT — caller-free shadow` |
| `renderer.hpp:66` | sibling cut-marker — `// SHADOW_GALLERY_FRAME_VS CUT — caller-free shadow` |
| `renderer.hpp:1140` | comment — `// draw_shadow_wall_paintings + draw_shadow_gallery_frames CUT` |
| `renderer.hpp:1158` | comment — `if (!(ROSTER.gallery)) n += 4;  // was 6; shadow_gallery_frame + shadow_wall_painting cut` |
| `renderer.hpp:1900` | comment — `// Shadow Gallery Frame pipeline CUT — caller-free` |
| `renderer.hpp:1979` | comment — `// Shadow Wall Painting pipeline CUT — caller-free` |
| `src/docs/old docs/mop.patch:1436-1438` | historical diff text |
| `audit/past reports/LADDER.md:3292,3391,3399`, `RENDER_UPDATE_API_RECON.md:69`, `CC_AUDIT_REPORT.md:130`, `TERRAIN_DOSSIER.md:509` | prior reports |

**Zero hits in `world.wgsl`.** The entry point `shadow_wall_painting_vs` does
not exist in the shader. `renderer.hpp:69-71` declares only the three live
names (`WALL_PAINTING_VS`, `WALL_PAINTING_CANVAS_FS`, `WALL_PAINTING_FRAME_FS`);
line 72 is where the fourth used to be.

No pipeline is created from it (`renderer.hpp:1979` marks the site), no draw
function exists, and `renderer.hpp:1158` records the pipeline-count arithmetic
already adjusted for the removal (6 → 4).

Corroborating: `gallery.hpp:35-39`, `NOTE[gallery:shadows-missing]` —
*"paintings (terrain quads) and wall frames are not currently drawn in the
shadow pass (draw_shadow_all). They render in the main pass via
draw_wall_paintings + draw_gallery_frames but cast no shadows."*

**Consequence for the campaign, as the handoff read it: confirmed.** Wall
density does not multiply by active spot count. Per painting the main-pass
cost is `PAINTING_FRAME_VERTS_PER` = 78 across two passes, plus the terrain
quad's 384 for outdoor forms. The handoff's "462" is the sum of both forms'
budgets; a wall frame alone is 78 × 2 passes = 156 vertex invocations.

---

## §A13 — The outdoor consequence of 256 slots

### A13.1 — Outdoor readers of the bound

| site | via | effect at 256 |
|---|---|---|
| `gallery.hpp:754` | direct loop | idempotency scan 8× longer, same answer |
| `gallery.hpp:1055` | `find_free_painting_slot` | succeeds 8× deeper before returning `UINT32_MAX` |
| `gallery.hpp:1207` | direct loop | eviction scan 8× longer |
| `gallery.hpp:1881,1885,1886` | `teardown_gallery` | 8× the clear + a 32 KiB stack array + a 32 KiB upload |

None of these change outdoor *behaviour*. They change how deep the pool goes
before it is empty.

### A13.2 — Does anything outdoors assume 32 implicitly?

Searched `machine/spawn_engine.hpp`, `machine/entity_pipeline.hpp`,
`contracts/spawn_services.hpp`, the footprint registry, and `roster.hpp`.
**No painting-count constant lives outside `Dim::`.** The outdoor caps that
exist are their own facts:

- `MAX_GALLERIES = 48` (`gallery.hpp:437`) — gallery *centres*, not paintings.
- `GalleryConfig::PAINTINGS_MAX_BY_ARCHETYPE[4] = {8, 10, 12, 12}`
  (`gallery.hpp:176`).
- `GalleryConfig::PAINTINGS_MEAN = 5.0`, `SIGMA = 2.0`, `MIN = 2`.

Only `src/docs/HANDOFFS/A FEW TWEAKS/cc_handoff_tweaks_A.txt:202` names the
ratio `Dim::PAINTING_MAX_SLOTS / GalleryConfig::PAINTINGS_MEAN` as *"the true
ceiling on simultaneous galleries"* — a doc, not code. At 32/5 that ceiling is
~6 galleries; at 256/5 it is ~51, which now exceeds `MAX_GALLERIES = 48`.
**So 256 moves the outdoor binding constraint from slots to gallery centres.**
That is a behaviour change outdoors, produced by a change that touches no
outdoor code. Reported.

### A13.3 — With slots at 256 and `EXHIBITION_LAYERS` at 32

`commit_gallery`'s loop stops at the **first** layer exhaustion:

- authored path: `gallery.hpp:1105-1106`
  `uint32_t exh = find_free_exhibition_layer(gs); if (exh == UINT32_MAX) break;`
- snapshot path: `gallery.hpp:1138-1139`, identical.

`break` exits the painting loop cleanly; `placed` is whatever was achieved;
`placed == 0` releases the footprint and the centre (`:1188-1193`); otherwise
the gallery stands, partially populated, and the log at `:1196-1203` prints
`paintings=<placed>/<painting_count>`.

**What the user sees: galleries that thin out.** The first ~32 exhibited
paintings in the world look normal; after that, every new gallery is short of
its planned count, and eventually every new gallery places zero and vanishes
before it is drawn. No error, no crash. The already-registered footprint is
correctly released, so no ghost ground is claimed.

**The exhibition array, not the slot array, is the real capacity today.**
Raising slots to 256 alone raises nothing that a user can see. This is the
same conclusion as §0.2 arriving from the other side, and it is the honest
reason Stage B has no visual delta.

### A13.4 — The pool is shared, and it is shared *in the same room*

**Proof the pool is shared:** one array, `GalleryState::painting_slots`
(`gallery.hpp:486`); one allocator, `find_free_painting_slot` (`:551`); called
by `commit_gallery:1055` (outdoor) and `place_wall_paintings:1649` (indoor).
One `exhibition_occupied` array (`:480`), one allocator
`find_free_exhibition_layer` (`:534`), four callers across both paths.

**And they contend simultaneously.** `MOOD_SPAWN_MULT`
(`population_themes.hpp:38-43`) rests at **identity — 1.0 for gallery in both
indoor moods** — so the gallery family is not vetoed indoors, and
`indoor_module.hpp:76` gives gallery `{IndoorSize::NATURAL, IndoorBounds::FULL}`
with the comment *"sand-standing exhibits wholly inside"*. Outdoor terrain
galleries therefore spawn **inside the indoor room**, clamped by
`indoor_bounds_clamp` (`gallery.hpp:905`) to fit whole.

Ordering saves the wall today: `teardown_gallery` clears everything, then
`apply_mood` → `place_wall_paintings` takes its ≤20 slots, and only afterwards
does patch streaming let galleries claim the rest.

**Under R1 that ordering becomes the problem, not the protection.** Wall
frames would take 200+ of 256 first, and every indoor terrain gallery would
then find `find_free_painting_slot` empty and place zero. Whether that is
correct is a taste question for Jean — the handoff's OUT OF SCOPE clause says
outdoor is perturbed *by sharing, not by design*, and this is precisely such a
perturbation. **Reported; no edit proposed.**

Worst-case outdoor demand today: `MAX_GALLERIES` 48 × mean 5 ≈ 240 paintings
wanted against 32 available — outdoor is already starved by ~7×. It does not
read as starvation only because `place_gallery_from_selection` reserves against
*content* (`gallery.hpp:876-884, 893-895`) and content is capped at 16 + 16
long before slots are.

### A13.5 — Silent promotion drops

The drop site is `gallery.hpp:542`, and it is unconditional on path — outdoor
and indoor both go through `queue_promotion`.

**How many promotions can one frame queue?** Every promotion accompanies one
slot fill, so the per-frame ceiling is (galleries committed this frame ×
paintings each) + (one indoor placement event, if a mood transition landed
this frame).

- A mood transition frame runs `place_wall_paintings` once: **≤ 20 today**
  (4 walls × `per_wall_count_hi` 5), **≤ 200+ under R1**.
- Commit frames run `commit_gallery` per placement drained by
  `commit_entity_queue`; each can place up to `plan.reserved_count` ≤ 12.

Today the sum cannot exceed 32 because the whole slot array is 32. **That is
the only thing holding the invariant** — and it is exactly the handoff's
reading. At 256 slots with `MAX_PROMOTIONS_PER_FRAME` left at 32, a single R1
placement event queues ~200 promotions, **168 of which are dropped silently**,
while `exhibition_occupied[exh] = true` is still set and
`upload_painting_slot` still runs. Result: 168 frames pointing at exhibition
layers that were never written — undefined texture contents, and the layer
permanently marked occupied.

**Confirmed load-bearing. The two constants must move together or not at all.**

---

## §C — THE ABANDONMENT CENSUS (Stage C gate)

`place_wall_paintings` spans `gallery.hpp:1525-1794`. Loop structure:

```
for (aw = 0; aw < active_wall_count; aw++)          :1599   ← the wall loop
    for (p = 0; p < effective_count; p++)           :1618   ← the width pre-pass
    while (effective_count > 1 && ...)              :1637   ← the trim
    for (p = 0; p < effective_count; p++)           :1647   ← the placement loop
        for (i = 0; i < STAGING_LAYERS; i++)        :1687   ← the snapshot scan
        for (a = 0; a < STAGING_LAYERS; a++)        :1739   ← the authored rescan
```

### C.1 — Every `return`. **Expected count: 3.**

| # | line | verbatim | nesting | condition | abandons |
|---|---|---|---|---|---|
| 1 | **1650** | `            if (slot == UINT32_MAX) return;` | placement loop | painting slots exhausted | **all remaining walls** |
| 2 | **1699** | `                    if (exh == UINT32_MAX) return;` | placement loop, snapshot branch | exhibition layers exhausted | **all remaining walls** |
| 3 | **1751** | `                if (exh == UINT32_MAX) return;` | placement loop, authored branch | exhibition layers exhausted | **all remaining walls** |

**Proof of what each abandons:** all three sit lexically inside the `for (p …)`
at `:1647`, which is inside the `for (aw …)` at `:1599`. `return` leaves
`place_wall_paintings` entirely. The wall loop never advances. Walls
`aw+1 … active_wall_count-1` are never visited. The function's only remaining
work is the log at `:1791`, which is also skipped — so a resource-exhausted run
prints nothing at all, which is why this has stayed invisible.

**The handoff undercounts.** It names the snapshot and authored paths (2 and 3
above). **#1 at `:1650` sits above the content branch** — it is the *slot*
exhaustion, not the *layer* exhaustion — and it is the one that fires first
under R1, because slots (256) and layers (32) diverge the moment Stage B lands
without Stage D. Confirm the count is **3** before editing.

### C.2 — Every `break`. All correct; do not touch.

| line | verbatim | nesting | exits |
|---|---|---|---|
| 1690 | `                        break;` | snapshot scan `for i` | the scan, having found `snap_stg` |
| 1707 | `                    if (cursor + paint_width > wall_right) break;` | placement loop | **this wall only** — correct |
| 1761 | `                if (cursor + paint_width > wall_right) break;` | placement loop | **this wall only** — correct |

`:1707` and `:1761` are the width-overflow guards, and they are already the
shape the three returns should take. That is the local precedent.

### C.3 — Every `continue`. All correct.

| line | verbatim | effect |
|---|---|---|
| 1694 | `                    if (count_unused_authored(gs, usedAuthored) == 0) continue;` | skip this painting (no content either way) |
| 1730 | `                    continue;` | snapshot placed successfully → next painting |
| 1736 | `                if (auth_stg == UINT32_MAX) continue;` | no authored staging → skip this painting |
| 1746 | `                    if (best == UINT32_MAX) continue;` | all authored already used → skip this painting |

### C.4 — Anchors, and the replace-all trap

Indentation, read with `cat -A`:

- `:1650` — **12 spaces**, `if (slot == UINT32_MAX) return;`.
  The string `if (slot == UINT32_MAX)` appears **twice** in `gallery.hpp`:
  `:1056` (`… break;`, in `commit_gallery`) and `:1650`. Distinct suffixes, so
  `if (slot == UINT32_MAX) return;` is unique. Preceding line is
  `uint32_t slot = find_free_painting_slot(gs);`.
- `:1699` — **20 spaces**. Preceded by
  `                    uint32_t exh = find_free_exhibition_layer(gs);`.
- `:1751` — **16 spaces**. Preceded by
  `                uint32_t exh = find_free_exhibition_layer(gs);`.

**`if (exh == UINT32_MAX)` appears FOUR times in `gallery.hpp`:**
`:1106` (`break`, commit/authored), `:1139` (`break`, commit/snapshot),
`:1699` (`return`), `:1751` (`return`).
**`if (exh == UINT32_MAX) return;` appears exactly twice**, at 20 and 16
spaces of indentation respectively — so the two must be anchored by
indentation or by including the preceding line. **No blind replace-all.**

### C.5 — `commit_gallery`, the outdoor twin: NOT defective

`gallery.hpp:941-1205`. One `return` (`:992`) — the residual zero-content
abort, which is **before** the painting loop and correctly releases the
footprint and the centre first (`:990-991`). Every in-loop exit is a `break`:
`:1056` (slot exhaustion), `:1082` (no content of either kind), `:1106`
(authored layer exhaustion), `:1139` (snapshot layer exhaustion), `:1174`
(`!placed_this`).

**The outdoor path already has the correct form at every corresponding site.**
Stage C is bringing the indoor path up to the shape the outdoor path already
holds — which is a strong argument that `break` is the intended form and the
three `return`s are the drift.

### C.6 — The width-estimate drift (Stage E fold-in)

**The plan**, `gallery.hpp:1618-1634`:
```cpp
uint32_t scale_idx = select_tier(p_seed, WallPaintingProp::SCALE_ROLL, w, INDOOR_SCALE_COUNT);
float h = INDOOR_SCALES[scale_idx]->height_lo
        + cpu_hash_f(p_seed, WallPaintingProp::HEIGHT_JITTER) * (…height_hi − …height_lo);
painting_heights[p] = h;

// Estimate width from typical aspect ratio (~1.3)
float est_aspect = 0.8f + cpu_hash_f(p_seed, WallPaintingProp::ASPECT_ESTIMATE) * 0.8f;  // [0.8, 1.6]
painting_widths[p] = h * est_aspect;
total_width += painting_widths[p];
if (p > 0) total_width += WALL_ART.painting_gap;
```

**The placement**, `gallery.hpp:1703` (snapshot) and `:1757` (authored):
```cpp
paint_width = height * snap.aspect_ratio;   // :1703
paint_width = height * img.aspect_ratio;    // :1757
…
cursor += paint_width + WALL_ART.painting_gap;   // :1727, :1781
```

**Height does not drift.** `select_tier` and the height jitter both key off the
same `p_seed` (`:1619` and `:1652` compute it identically), and `select_tier`
is pure (`seed_utils.hpp:106-109`). So `painting_heights[p]` is exactly the
height used.

**Width drifts, in both directions.** The plan uses `est_aspect ∈ [0.8, 1.6]`,
uniform. Reality is:
- snapshot: `snap.aspect_ratio`, sampled per tier from `SHOT_PARAMS`
  (`gallery.hpp:104-112`), spanning **[0.56, 2.39]** with a
  weight-averaged mean near **1.60**;
- authored: `img.aspect_ratio = width/height` of the disk image
  (`gallery.hpp:1358`), unbounded.

So `est_aspect` is biased **low by roughly 1.60 / 1.20 ≈ 33 %** against
snapshots, and arbitrary against authored images. `group_start` is computed
from the planned total (`:1644`), `cursor` advances by the real widths
(`:1727, :1781`), and the two diverge monotonically to the right. The
`wall_right` guards at `:1707`/`:1761` are the containment — they `break` the
wall short, which is why the visible symptom today is "the last painting on a
wall sometimes doesn't appear" rather than "paintings run off the edge".

**The fix the handoff names — resolve the content decision above the width
pre-pass — is structurally available.** The content decision at
`gallery.hpp:1674-1680` depends only on `site_type`, `p_seed`, and
`count_unused_authored(gs, usedAuthored)`. The first two are available in the
pre-pass (`p_seed` is computed identically at `:1619`). The third is not: it
mutates as the placement loop consumes staging. That is the one real obstacle,
and it is why the reorder belongs in Stage E's rewrite rather than as a
standalone fix. **Reported, not designed.**

---

## §E-INPUTS — measurements Stage E will need

Recon-grade facts only. No mechanism proposed.

### E.1 — The wall rect today, and how little of it is used

**Vertical band, exactly**, `MOOD_INDOOR_FLAT` (`ch = 20`).
`paint_y_base = ch · 0.45 = 9.0` (`gallery.hpp:1534`); `py = paint_y_base + y_offset`
with per-bucket `y_offset` from `gallery.hpp:292-294`; then the clamp at
`:1669-1671` sets `bottom = 4` whenever `bottom > 4`. Swept over each bucket's
full `(h, y_offset)` rectangle:

| bucket | h | `y_offset` | reachable bottom | reachable top |
|---|---|---|---:|---:|
| intimate | [6, 11] | [0.0, +2.0] | 3.50 | 15.00 |
| standard | [8, 12] | [−1.5, +1.5] | 1.50 | 16.00 |
| statement | [10, 14] | [−3.5, −1.5] | **−1.50** | 14.50 |

**Canvas band `y ∈ [−1.50, 16.00]`; with the 0.45 frame border, `[−1.95, 16.45]`
of a 20.0 m wall.**

Two things follow, and both are R4's business:

- **There is no top clamp** — `max_bottom_height` guards only the *floor* side
  (it pushes a too-high painting down). The top is bounded only incidentally, by
  `min(py + h/2, 4 + h)`, which today tops out at 16.45. **A 3.55 m dead band at
  the ceiling is left by accident, not by design.** Stage E's `top_margin` should
  make that deliberate.
- **The floor side already breaches.** A statement piece at `h = 14`,
  `y_offset = −3.5` puts the frame's bottom edge **1.95 m below `y = 0`**. The
  clamp cannot catch it — it only fires when `bottom > 4`. Whether this shows
  depends on the live-card ground term added at `world.wgsl:10203`, but nothing
  in the placement math prevents it. Stage E's `floor_margin` is not a
  refinement here; it is the first floor-side guard in the direction that
  matters.

Expected art area per painting: E[h] = 0.25·8.5 + 0.50·10.0 + 0.25·12.0 =
**10.125**; E[aspect] ≈ **1.60** (weighted over `SHOT_PARAMS`) → E[w] ≈ 16.2 →
**≈ 164 m² per painting**. Mean count per wall = 3 (uniform [1,5]).

| radius | wall rect (usable_span × ch, FLAT) | typical art (3 × 164) | **coverage** | best case (5 statement) |
|---:|---:|---:|---:|---:|
| 1 | 126 × 20 = 2,520 | 492 | **19.5 %** | 45.7 % |
| 2 | 226 × 20 = 4,520 | 492 | **10.9 %** | 25.5 % |
| 3 | 326 × 20 = 6,520 | 492 | **7.5 %** | 17.7 % |
| 4 | 426 × 20 = 8,520 | 492 | **5.8 %** | 13.5 % |

**R1's "majority" is unreachable at any per-wall count the current code
permits** — `painting_widths[8]` / `painting_heights[8]` (`gallery.hpp:1614-1615`)
and `effective_count = std::min(count, 8u)` (`:1616`) cap a wall at 8 pieces,
which is ≤ 52 % even in the smallest room.

### E.2 — What R3 actually has to be, arithmetically

Taking D1 literally — the fill tier strictly below every painting bucket, so
`snapshot_height ∈ [3, 5]`, E[h] = 4.0, E[w] = 4.0 · 1.60 = 6.4, art area
25.6 m² — and a `snapshot_gap` of 1.5 on both axes, the cell is
7.9 × 5.5 = 43.45 m², giving a packing efficiency of **0.589**. That is also
the **ceiling on achievable coverage** at that gap: tighten `snapshot_gap` to
1.0 and efficiency rises to 0.692.

Frames needed for `target_coverage = 0.55`, four walls, FLAT:

| radius | per wall | **four walls** |
|---:|---:|---:|
| 1 | 54 | **216** |
| 2 | 97 | **389** |
| 3 | 140 | **560** |
| 4 | 183 | **732** |

Minus the centre-band paintings' footprints and their `painting_clearance`
moat — call it 20–25 fewer per wall.

**Read on the 256 derivation:** 256 covers `finite_radius = 1` at majority on
all four walls with headroom, `radius = 2` on roughly two and a half walls, and
`radius = 3`/`4` not at all. Since radius is uniform over {1,2,3,4}, **256
delivers R1 in about one room in four.** The alternatives are Jean's to weigh:
a larger constant (with the `EXHIBITION_LAYERS` cost in §A3 if distinctness is
to follow), a `target_coverage` that is per-radius rather than global, or R1
read as "the walls the eye lands on" rather than all four. **No
recommendation. The arithmetic is the report.**

### E.3 — The coupling Stage E must not break

Repeating §0.5 because it is the one thing that can silently invalidate a
landed hang: `vault_crown()` (`mood.hpp:601-616`) derives the vault's spring
height from `PAINT_CENTER_FRACTION = 0.45f` + `PAINT_TOP_MARGIN = 5.5f` +
`SPRING_MARGIN = 8.0f`. Move the hang upward without moving these and the vault
springs below the art. And `PAINT_CENTER_FRACTION` is a duplicate of
`WALL_ART.paint_y_frac`.

The vertical extent available to a wall is therefore:

| mood | `ceiling_h` passed to `place_wall_paintings` | true vertical wall top |
|---|---:|---|
| FLAT | 20.0 | 20.0 (`wall_h = ch`, `mood.hpp:725`) |
| VAULT | 25.0 | 24.75 (`spring_h`) — **not** the 47.25+ crown |

Plus: `wall_painting_vs` adds `sample_live_card(world_pos.xz).x` to every
vertex (`world.wgsl:10203`), so the floor a frame is measured from is the live
card's, not y = 0.

---

## LEDGER

| Stage | State | Commit | Note |
|---|---|---|---|
| A — Recon | **landed** | this commit | 13/13 answered; 9 corrections in §0 |
| B — Slot count | **held** | — | land-gated. **Re-spec required: §0.1 (3 more WGSL literals, L3), §0.2 (unguarded instanced draw), §0.3 (the named FXC hazard is not this tree's hazard). Witness `tPipe` already exists (§A2.5).** |
| C — Abandonment | **held** | — | gate: Stage A stamped. Census done: **3 returns**, not 2 — `gallery.hpp:1650, 1699, 1751`. Anchors + replace-all trap in §C.4. |
| D — Weld | **held** | — | A9 census landed. Two incorrect sites only: `evict_paintings_for_patch:1214`, `clear_wall_paintings:1802`. GPU already sharing-safe (§0.7). `exhibition_count` has no reader (§0.6). Mechanism NOT chosen. |
| E — Salon hang | **held** | — | visual gate. Inputs in §E. **Vault-crown coupling (§0.5) and the live floor breach (§0.9) are prerequisite findings.** |
| HELD — Packing | held | — | trigger: Jean's eye on E. Cost of the alternative measured: `EXHIBITION_LAYERS` 32 → 256 is **+896 MiB** (§A3). |

**Stage A asks for a stamp before any stage proceeds. Nothing below A has been
touched.**
