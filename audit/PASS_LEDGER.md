# THE PASS LEDGER (PASS_0) — the frame economy a tile-based GPU is charged

> **POST-CAMPAIGN — 2026-08-13.** ATLAS_1revB, DISCARD_0 and FORMAT_1
> falsified **Q2 and Q3**: no `LoadOp::Load` survives anywhere in `src/`,
> two readerless depth stores are now `Discard`, the shadow maps are
> `Depth16Unorm`, and the indoor 4-light arm moves 16 MiB/frame of shadow
> attachment traffic, not 96. **ESTATE §2 carries the live numbers.**
> Q5–Q6 stand — with both shadow rows 8 MiB, textures 237.9, total 251.1.

A read-only census of what the audience's hardware pays per frame:
render passes, attachment load/store traffic, per-frame buffer writes,
and resident bytes. The binding ledger (`audit/BINDING_LEDGER.md`)
censused SLOTS; this one censuses BYTES MOVED and BYTES HELD.

**Zero source edits.** Every claim carries file + symbol; line numbers
are hints and go stale, symbols do not. Structured for regeneration:
the table shapes are fixed, so a later run diffs cleanly against this one.

## Provenance

| field | value |
|---|---|
| HEAD | `a690f7ad87ec483349720ce0281df0c09af4b326` |
| base | `origin/master` |
| binding ledger cross-referenced | `audit/BINDING_LEDGER.md` @ source commit `6710ac2` |
| demo column | `full` (`DemoCol::full` — "the golden twin — every tickable ON", `demos/matrix.hpp`) |
| subject note | TIDY_0d-i's `draw_indexed_mesh` reroute is unbuilt at this HEAD and is censused **as written** |

### The four primary read sites

| file | role in this census |
|---|---|
| `realization/render_passes.hpp` | shadow pass, main pass, frustum cull, ground entries |
| `bodies/gallery.hpp` | snapshot pass |
| `realization/state.hpp` | every texture/buffer creation, every upload setter, the GPU budget |
| `console/console.hpp` | surface configuration, host-owned depth |

---

## Q1 — THE PASS ROSTER

### Q1.1 Render passes (attachment traffic bearers)

Four `BeginRenderPass` sites exist in the whole tree. Two of them are the
two arms of one spine phase, so the frame presents **two or three**
distinct render passes depending on arm and capture state.

| # | label (verbatim) | owning function | file | cadence | arm |
|---|---|---|---|---|---|
| P1 | `Shadow Atlas Tile` | `render_shadow_pass` (indoor arm) | `render_passes.hpp` | **per light**, 1 pass per light, `li < count && li < MAX_SPOT_LIGHTS(4)` | indoor only |
| P2 | `Shadow Pass` | `render_shadow_pass` (outdoor arm) | `render_passes.hpp` | once per frame | outdoor only |
| P3 | `Rasterized Scene` | `render_main_pass` | `render_passes.hpp` | once per frame | both |
| P4 | `Photographer Snapshot` | `render_snapshot_pass` | `bodies/gallery.hpp` | on demand — early-returns unless `gs.pending_snapshot.active` | both |

The arm selector is one predicate, `render_passes.hpp`:

```
if (c->mood_state_.spot_light_active && cpuSpotLights_.count > 0)   → P1 (loop)
else                                                                 → P2 (single)
```

P1 and P2 are **mutually exclusive**: the indoor arm never runs the
single sun pass, and the outdoor arm never runs a tile. The spine row is
one (`RPhase::ShadowPass`, gate `true`), so exactly one arm runs each frame.

### Q1.2 Compute passes (no attachment traffic — listed for completeness)

Sixteen `BeginComputePass` sites. A compute pass has no attachments, so
it moves no load/store bytes; it is listed so the roster is total.

| label | owning function | file | cadence |
|---|---|---|---|
| `Photographer VP Compute` | `render_snapshot_pass` | `bodies/gallery.hpp` | on demand (with P4) |
| `Frustum Cull Patches` | `dispatch_frustum_cull` | `render_passes.hpp` | per frame |
| (3 sites) | `dispatch_compute`, `dispatch_placement_correction`, `dispatch_live_card_write` | `render_passes.hpp` | per frame |
| (2 sites) | GoL zone kernels | `bodies/gol_zones.hpp` | per frame, `ROSTER.gol` |
| (1 site) | GoL derive flush | `bodies/gol_zones.hpp` | per frame, `ROSTER.gol` |
| (4 sites) | orb kernels | `bodies/orbs.hpp` | per frame, `ROSTER.orbs` |
| (1 site) | pawn aura | `bodies/pawn.hpp` | wall-clock, `ROSTER.pawn_aura` |
| (1 site) | entity mesh gen | `cartridge.hpp` | per frame |
| (2 sites) | patch heightfield / cell colour | `surface/patch_system.hpp` | on patch stream |

### Q1.3 The canvas / swapchain configure site — one row, verbatim

`src/console/console.hpp`, `initSurface()`:

```
colorFormat_ = caps.formats[0];

surfaceConfig_.device      = device_;
surfaceConfig_.format      = colorFormat_;
surfaceConfig_.width       = currentWidth_;
surfaceConfig_.height      = currentHeight_;
surfaceConfig_.presentMode = wgpu::PresentMode::Fifo;
surfaceConfig_.alphaMode   = wgpu::CompositeAlphaMode::Opaque;
surface_.Configure(&surfaceConfig_);
```

| field | value |
|---|---|
| surface format | **runtime** — `caps.formats[0]`, the adapter's first advertised format. Never named in source. |
| `alphaMode` | `CompositeAlphaMode::Opaque` |
| `presentMode` | `PresentMode::Fifo` |
| `colorSpace` | **not set anywhere in the tree** — a repo-wide grep for `colorSpace` returns zero hits |
| `toneMapping` | **not set anywhere in the tree** — zero hits for `toneMapping` / `SurfaceColorManagement` |

Both absences are defaults-by-omission, not choices recorded anywhere.
`colorFormat_` also feeds four offscreen textures (Snapshot Staging,
Authored Staging, Exhibition, Offscreen Snapshot Color), so the adapter's
format choice sets 105 MiB of resident bytes as a side effect (Q5).

---

## Q2 — THE ATTACHMENT TABLE

Per render pass, per attachment. `clearValue` provenance is the last
column. Every op in the tree is enumerated here: a repo-wide grep for
`LoadOp::` / `StoreOp::` returns **12 assignments**, all listed below.

| pass | attachment | texture symbol | format | dimensions | `loadOp` | `storeOp` | resolve | clearValue from |
|---|---|---|---|---|---|---|---|---|
| P1 `Shadow Atlas Tile`, `li` even | depth | `shadowMapTexture_` (`li<2`) / `spotShadowMapTexture_` (`li>=2`) | `Depth32Float` | 2048 × 2048 × 1 | **`Clear`** | `Store` | — | literal `1.0f` |
| P1 `Shadow Atlas Tile`, `li` odd | depth | `shadowMapTexture_` (`li<2`) / `spotShadowMapTexture_` (`li>=2`) | `Depth32Float` | 2048 × 2048 × 1 | **`Load`** | `Store` | — | (unused under `Load`) |
| P2 `Shadow Pass` | depth | `shadowMapTexture_` | `Depth32Float` | 2048 × 2048 × 1 | `Clear` | `Store` | — | literal `1.0f` |
| P3 `Rasterized Scene` | color 0 | swapchain backbuffer (host-owned) | `colorFormat_` (runtime) | W × H × 1 | `Clear` | `Store` | none | `clearColor_[0..2]`, alpha literal `1.0` — call-site parameter, `cartridge.hpp` |
| P3 `Rasterized Scene` | depth | `depthTexture_` (`console.hpp`) | **`Depth24Plus`** | W × H × 1 | `Clear` | `Store` | — | literal `1.0f` |
| P4 `Photographer Snapshot` | color 0 | `offscreenColorTexture_` | `colorFormat_` (runtime) | 512 × 512 × 1 | `Clear` | `Store` | none | `c->clearColor_[0..2]`, alpha `1.0` |
| P4 `Photographer Snapshot` | depth | `offscreenDepthTexture_` | **`Depth24Plus`** | 512 × 512 × 1 | `Clear` | `Store` | — | literal `1.0f` |

**No `StoreOp::Discard` exists anywhere in the tree.** Every attachment
in every pass stores. No pass uses a resolve target; no pass is
multisampled.

### Q2.1 The dynamic branch (stop-condition disclosure)

One pass descriptor builds its ops from a runtime value rather than a
constant. Per the campaign's stop condition it is reported as **both
branches**, not collapsed:

```
render_passes.hpp, render_shadow_pass, indoor arm:
    uint32_t within = li % 2;   // 0 = left half, 1 = right half
    depthAttachment.depthLoadOp = (within == 0) ? wgpu::LoadOp::Clear
                                                : wgpu::LoadOp::Load;
    depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
```

The view is also branch-selected in the same descriptor:

```
    bool use_sun_map = (li < 2);
    depthAttachment.view = use_sun_map ? c->gpuState_.shadow_map_view()
                                       : c->gpuState_.spot_shadow_map_view();
```

Resolved per light index, the four reachable rows are:

| `li` | texture | `within` | `loadOp` | `storeOp` |
|---|---|---|---|---|
| 0 | `shadowMapTexture_` | 0 | `Clear` | `Store` |
| 1 | `shadowMapTexture_` | 1 | **`Load`** | `Store` |
| 2 | `spotShadowMapTexture_` | 0 | `Clear` | `Store` |
| 3 | `spotShadowMapTexture_` | 1 | **`Load`** | `Store` |

The `Load` on odd tiles exists **only** to preserve the even tile's
contents, because each tile is a separate render pass against the same
texture. Nothing in the odd tile reads what the load brings in. This is
the structure Q3 prices.

### Q2.2 The shadow maps' creation sites — verbatim

Carried forward from REFOUND_0 U2 and re-read at this HEAD. Both
descriptors are value-initialized in their own scope, so no field carries
over; `makeTexture` sets only `desc.label` and never touches format or
usage.

```
src/cartridges/the_board/realization/state.hpp, shadowMapTexture_:
    desc.size   = { Dim::SHADOW_MAP_SIZE, Dim::SHADOW_MAP_SIZE, 1 };
    desc.format = wgpu::TextureFormat::Depth32Float;
    desc.usage  = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    shadowMapTexture_ = makeTexture("Shadow Map", desc);

src/cartridges/the_board/realization/state.hpp, spotShadowMapTexture_:
    desc.size   = { Dim::SHADOW_MAP_SIZE, Dim::SHADOW_MAP_SIZE, 1 };
    desc.format = wgpu::TextureFormat::Depth32Float;
    desc.usage  = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    spotShadowMapTexture_ = makeTexture("Spot Shadow Atlas", desc);
```

`Dim::SHADOW_MAP_SIZE = 2048`. One symbol, one value each: `Depth32Float`,
`RenderAttachment | TextureBinding`. No `CopySrc`, no `CopyDst`, no mips,
default views (`CreateView()` with no descriptor).

The whole sampling stack is coherent with the format: layout entries
declare `TextureSampleType::Depth`, the sampler is
`SamplerBindingType::Comparison` backed by `CompareFunction::Less` with
`FilterMode::Linear` on both min and mag, and WGSL declares
`texture_depth_2d` + `sampler_comparison` at `@group(1) @binding(25/26/27)`.

### Q2.3 The main pass depth — is it ever sampled?

**No, and it provably cannot be.**

```
src/console/console.hpp, createDepthBuffer():
    depthDesc.label  = "Depth Texture";
    depthDesc.size   = { w, h, 1 };
    depthDesc.format = depthFormat_;                          // Depth24Plus
    depthDesc.usage  = wgpu::TextureUsage::RenderAttachment;  // <- no TextureBinding
```

The usage mask is `RenderAttachment` alone. Without `TextureBinding` the
texture cannot legally be placed in a bind group, so no shader in the
program can sample it regardless of what any WGSL declares. It is also
never a `CopySrc`, so nothing reads it back either.

**FINDING F1 — a stored attachment with no reader.** `P3`'s depth is
`storeOp: Store` on a texture that is physically unsamplable and never
copied. Under the tiler model of Q3 that is `W·H·4` bytes written back to
main memory every frame, for a resource whose contents are dead the
instant the pass ends. `StoreOp::Discard` is the exact op for this case
and appears nowhere in the tree.

**FINDING F2 — the same shape, offscreen.** `offscreenDepthTexture_` is
created `RenderAttachment` only (`state.hpp`), and `P4` stores it. 1.0 MiB
written back per capture with no reader. Lower stakes than F1 because P4
is on demand, but it is the same defect and the same one-word fix.

Both findings are reported, not acted on: this campaign edits no source.

---

## Q3 — THE TRAFFIC ARITHMETIC

### Q3.0 The model — stated as a model, not as a witness

This section is **arithmetic under a stated model**, not a measurement.
Nothing here was observed on hardware; every number is derived from the
descriptors in Q2 and the dimensions in Q5.

The model:

1. `loadOp: Load` reads the **whole attachment subresource** into tile
   memory: `+bytes`.
2. `storeOp: Store` writes the **whole attachment subresource** back:
   `+bytes`.
3. `Clear` and `Discard` are approximately free — no traffic to main
   memory.
4. **Viewport and scissor do not reduce either.** WebGPU load/store ops
   are defined per attachment over the whole subresource; a scissored
   pass still loads and stores the full texture. This is the pivot of
   the whole section: the indoor arm scissors each tile to half width
   and pays for the full 2048 × 2048 anyway.
5. Attachment bytes = `width × height × bytesPerTexel`, no mips, no
   multisample, no driver padding.

Where the model is generous or harsh:

- **Generous to the program**: a real tiler may compress depth, and
  a `Clear` still costs the tile-memory clear itself.
- **Harsh to the program**: it assumes no driver-side elision of a
  store whose result is never read — which for F1 a driver cannot
  perform, because it cannot know the texture is unsampled.

Per-texture constants used below:

| attachment | bytes | MiB |
|---|---|---|
| Shadow map (2048 × 2048 × 4, `Depth32Float`) | 16,777,216 | 16.000 |
| Spot shadow atlas (identical) | 16,777,216 | 16.000 |
| Main color (W × H × 4) | `4·W·H` | — |
| Main depth (W × H × 4, `Depth24Plus`) | `4·W·H` | — |
| Snapshot color (512 × 512 × 4) | 1,048,576 | 1.000 |
| Snapshot depth (512 × 512 × 4) | 1,048,576 | 1.000 |

### Q3.1 The shadow arm — the table the campaign was called for

Per frame, shadow attachments only.

| arm | passes | tiles `Clear`+`Store` | tiles `Load`+`Store` | loaded MiB | stored MiB | **total MiB** | × outdoor |
|---|---|---|---|---|---|---|---|
| **outdoor** (`P2`) | 1 | 1 | 0 | 0 | 16 | **16** | 1.00× |
| **indoor, 2 lights** (Gallery) | 2 | 1 (`li0`) | 1 (`li1`) | 16 | 32 | **48** | 3.00× |
| **indoor, 3 lights** (Cathedral) | 3 | 2 (`li0`,`li2`) | 1 (`li1`) | 16 | 48 | **64** | 4.00× |
| **indoor, 4 lights** (Quartet) | 4 | 2 (`li0`,`li2`) | 2 (`li1`,`li3`) | 32 | 64 | **96** | 6.00× |

Worked, tile by tile, at 4 lights:

| `li` | texture | op pair | load MiB | store MiB | running MiB |
|---|---|---|---|---|---|
| 0 | Shadow Map | `Clear`+`Store` | 0 | 16 | 16 |
| 1 | Shadow Map | **`Load`**+`Store` | 16 | 16 | 48 |
| 2 | Spot Shadow Atlas | `Clear`+`Store` | 0 | 16 | 64 |
| 3 | Spot Shadow Atlas | **`Load`**+`Store` | 16 | 16 | 96 |

**The Clear-left / Load-right structure, priced.** Of the 96 MiB an
indoor Quartet frame moves in shadow attachments, **32 MiB is pure
preservation traffic** — the two `Load`s that exist only because the
right-hand tile is a separate pass against a texture the left-hand tile
already wrote. Nothing in an odd tile reads a loaded texel. A further
32 MiB is the odd tiles' `Store` of a texture that was already stored
one pass earlier and only half-modified since.

Scaled to a 60 Hz frame, shadow attachments alone:

| arm | MiB/frame | GiB/s at 60 Hz |
|---|---|---|
| outdoor | 16 | 0.94 |
| indoor 2 | 48 | 2.81 |
| indoor 3 | 64 | 3.75 |
| indoor 4 | 96 | 5.63 |

### Q3.2 Whole-frame attachment traffic

The main pass is resolution-dependent and the canvas size is runtime
(`surfaceConfig_.width/height` from `currentWidth_/currentHeight_`), so
the main-pass term stays symbolic and two illustrative resolutions are
worked below. They are **examples, not measurements** — no canvas size is
recorded in the tree.

Main pass, both arms, always: `4·W·H` (color, `Clear`+`Store`) +
`4·W·H` (depth, `Clear`+`Store`) = **`8·W·H` bytes stored, 0 loaded**.

| arm | shadow MiB | main pass | total (symbolic) |
|---|---|---|---|
| outdoor | 16 | `8·W·H` B | `16 MiB + 8·W·H` |
| indoor 2 | 48 | `8·W·H` B | `48 MiB + 8·W·H` |
| indoor 3 | 64 | `8·W·H` B | `64 MiB + 8·W·H` |
| indoor 4 | 96 | `8·W·H` B | `96 MiB + 8·W·H` |

Worked at two illustrative canvas sizes:

| arm | @ 1920 × 1080 (15.82 MiB main) | @ 1080 × 2400 (19.78 MiB main) | shadow share @1920×1080 |
|---|---|---|---|
| outdoor | 31.8 MiB | 35.8 MiB | 50% |
| indoor 2 | 63.8 MiB | 67.8 MiB | 75% |
| indoor 3 | 79.8 MiB | 83.8 MiB | 80% |
| indoor 4 | 111.8 MiB | 115.8 MiB | **86%** |

The snapshot pass adds **2.0 MiB** (1.0 color + 1.0 depth, both
`Clear`+`Store`) on the frames where a capture is pending, in either arm.

**The headline.** In an indoor Quartet frame the shadow atlas is 86% of
all attachment traffic, and a third of that shadow traffic is the
preservation `Load`/`Store` pairs that the two-pass-per-texture tiling
forces. The main pass — the pass that actually produces the image — is
the minority cost.

### Q3.3 What the two open rulings would price

Derived under the same model; recorded here because Q3 is where the
arithmetic lives, and again in the precondition matrix at the close.

| arm | today | after ATLAS_1 | after FORMAT_1 | after both |
|---|---|---|---|---|
| outdoor | 16 | 16 | 8 | 8 |
| indoor 2 | 48 | 16 | 24 | 8 |
| indoor 3 | 64 | 32 | 32 | 16 |
| indoor 4 | **96** | 32 | 48 | **16** |

MiB per frame, shadow attachments only.

- **ATLAS_1** (one pass per shadow texture, per-light viewports inside
  the pass) removes every `Load` and every duplicate `Store`: each
  texture is cleared once and stored once. At 4 lights, 96 → 32 MiB,
  a **64 MiB/frame** saving (3.75 GiB/s at 60 Hz).
- **FORMAT_1** (`depth16unorm` shadow maps) halves every shadow byte:
  2 B/texel instead of 4, so each texture is 8 MiB not 16.
- Together at 4 lights: **96 → 16 MiB, a 6× reduction**, which is
  exactly the outdoor arm's present cost.

---

## Q4 — PER-FRAME BUFFER TRAFFIC

Reachable from the frame loop, boot excluded. Byte sizes are the ledger's
Table A column where the buffer is bound, and the `sizeof` at the
creation site otherwise. Cadence is read from the spine table
(`RENDER_SPINE` / `UPDATE_SPINE`, `cartridge.hpp`).

### Q4.1 `WriteBuffer` — CPU → GPU

| symbol | bytes | cadence | gated? | site |
|---|---|---|---|---|
| `plantComputeGroundBuffer_` (`plant_ground`) | 2,432 | per frame | **unconditional** | `upload_ground_entries`, `render_passes.hpp` |
| `columnGroundBuffer_` (`cmg_column_ground`) | 1,024 | per frame | **unconditional** | `upload_ground_entries` → `upload_column_origins` |
| `archGroundBuffer_` (`arch_ground`) | 512 | per frame | **unconditional** | `upload_ground_entries` → `upload_arch_origins` |
| `frustumComputeBuffer_` (`fc_indirect`) | 60 | per frame | **unconditional** | `reset_frustum_indirect`, `FC_ARGS_BYTES = 15·4` |
| `signalBuffer_` (`signal`) | 48 | per frame | **unconditional** | `upload_signal` — writes `offsetof(GPUFrameSignal, sky_mode)` = 48 of the 80-byte binding |
| `configBuffer_` (`config`) | 624 | per frame | **dirty-gated** — `if (!configDirty_ && !configDynamic_) return;` | `upload_config` |
| `drawPlanBuffer_` (`fc_draw_plan`) | 144 | per frame | **dirty-gated** — `memcmp` against `lastDrawPlan_` | `upload_draw_plan` |
| `spotVPStagingBuffer_` | 64 × count | **on mood change**, not per frame | conditional on light count | `stage_spot_vps`, called from `derive_indoor_lights` in `direction/mood.hpp` |
| `signalBuffer_` sky block | 32 | per frame when the pawn rides the ribbon | conditional | `resync_sky_head` |
| `ribbonBuffer_` (`render_ribbon`) | 4 / 12 / 4+4 | per frame, `ROSTER.ribbon` | targeted sub-range writes | `upload_ribbon_time` / `_color` / `_wave_amps` |
| `headPosesBuffer_` (`head_poses`) | ≤ 6,400 | per frame, `ROSTER.ribbon` | conditional | `upload_ribbon_head_poses` |
| `pawnAuraConfigBuffer_` (`pawn_aura_cfg`) | 4 + 4 | wall-clock, `ROSTER.pawn_aura` | conditional | `upload_pawn_aura_frame` |
| `orbConfigBuffer_` (`orb_config`) | 4–24 per field | per frame, `ROSTER.orbs` | per-field targeted writes | `upload_orb_frame` and siblings |

### Q4.2 `CopyBufferToBuffer` — GPU → GPU

| source → dest | bytes | cadence | gated? | site |
|---|---|---|---|---|
| `floatingEntityBuffer_` → readback staging | **54,912** | per frame while `floaterReadbackState_ == IDLE` | state-machine gated | `phase_witness_capture`, `cartridge.hpp` |
| `agentStateBuffer_` → readback staging | 3,072 | per frame while `pawnReadbackState_ == IDLE` | state-machine gated | `phase_witness_capture` |
| `frustumComputeBuffer_` → `frustumIndirectLOD0_` | 60 | per frame | **unconditional** | `dispatch_frustum_cull`, `FC_ARGS_BYTES` |
| `spot_vp_staging[li·64]` → `vp_buffer[light_vp_offset]` | **64 × N lights** | **per light, per frame** | indoor arm only | `render_shadow_pass`, inside the tile loop |
| meter resolve → readback staging | `16 · pairs` | per frame | `if constexpr (INSTRUMENTS.frame_meter)` — **off by default** | `cartridge.hpp` |
| patch staging → patch params | varies | on patch stream | conditional | `surface/patch_system.hpp` |

### Q4.3 The spot VP copy dance, priced

The indoor arm's per-light copy is the smallest item in this census and
the most structural. Inside the tile loop, before each `BeginRenderPass`:

```
encoder.CopyBufferToBuffer(
    c->gpuState_.spot_vp_staging(), li * 64,
    c->gpuState_.vp_buffer(), GPUState::light_vp_offset(),
    GPUState::light_vp_size());
```

64 bytes, N times per frame — 256 B/frame at 4 lights. It is *why* the
tiles are separate passes: the VP buffer holds **one** light matrix slot,
so each tile must rewrite it before its pass, and a copy cannot be
recorded inside a render pass. Any ATLAS_1 that merges the tiles into one
pass must first widen `vp_buffer`'s light slot to an array indexed by the
draw, or pass the light index another way. **This 64-byte copy, not the
attachment ops, is the actual blocker on ATLAS_1** — the load/store
structure is a consequence of it.

### Q4.4 `render_floating` against its wall — verbatim

The brief records 54,912; re-read at this HEAD it is unchanged.

| field | value |
|---|---|
| binding | `bind::g0::render_floating = 300`, `binding_registry.hpp` |
| WGSL | `render_floating` : `FloatingEntityArray`, uniform, visibility `VF` |
| ledger row (verbatim) | `` | Render Entity Layout | `renderEntityBindGroupLayout_` | 4 | `bind::g0::render_floating` | 300 | `g0` | buffer | Uniform | `VF` | no | — | 0 | `render_floating` | `FloatingEntityArray` | n/a | no | 54912 | uniform | `VF` | `-` | `` |
| **current size** | **54,912 B** |
| **the wall** | **65,536 B** (`maxUniformBufferBindingSize`, WebGPU Core default) |
| headroom | 10,624 B |
| derivation | `TOTAL_FLOATING_SLOTS (264) × sizeof(GPUFloatingEntityState) (208) = 54,912` — `264 = MAX_SPHERE_INSTANCES (8) + MAX_CUBE_INSTANCES (256)` |
| headroom in slots | **51 slots** (`10,624 / 208 = 51.08`) |
| occupancy | **83.8%** of the wall |

The same buffer is also bound as storage (`floating_entities`, 54,912 B)
in the compute band, where the wall is `maxStorageBufferBindingSize`
(128 MiB Core) and irrelevant. It is the **uniform** binding that is at
84% of a hard limit. Adding one field to `GPUFloatingEntityState` costs
264 × the field's aligned size: a single `vec4` (16 B) would add 4,224 B
and take the binding to 59,136 (90.2%); a second would breach at 63,360
plus alignment. The structure is 208 bytes and 16-aligned, so growth is
quantised at 264 × 16 = 4,224 B per `vec4`. **Two more `vec4`s fit; three
do not.**

### Q4.5 The three largest unconditional per-frame writes

"Unconditional" = no dirty flag and no roster gate; state-machine gated
readbacks are included with the gate named, because they fire on the
large majority of frames.

| rank | item | bytes/frame | kind |
|---|---|---|---|
| 1 | `floatingEntityBuffer_` → readback staging | **54,912** | `CopyBufferToBuffer`, gated on `IDLE` |
| 2 | `agentStateBuffer_` → readback staging | 3,072 | `CopyBufferToBuffer`, gated on `IDLE` |
| 3 | `plant_ground` | 2,432 | `WriteBuffer`, ungated |

Restricted to `WriteBuffer` alone, the three are `plant_ground` (2,432),
`cmg_column_ground` (1,024), and `arch_ground` (512) — all three inside
the same function, `upload_ground_entries`, all three rewriting the full
high-water array every frame regardless of how many instances are active.

**The comparison that matters.** Total unconditional per-frame buffer
traffic is **62,376 B (60.9 KiB)** — 4,076 B of `WriteBuffer` + 58,044 B
of `CopyBufferToBuffer` + 256 B of spot-VP copies at 4 lights. Against
Q3's 16–96 MiB of attachment traffic, buffer traffic is **three orders of
magnitude smaller**. The frame economy is attachment-bound. No upload
optimisation in this table can matter next to one `StoreOp::Discard` or
one merged shadow pass.

---

## Q5 — THE RESIDENT TABLE

### Q5.1 Reconciliation against the boot `[GPU Budget]` block

The boot block reports: buffers 13.2 MiB, textures 253.9 MiB, total
267.1 MiB. Recomputed here from the creation sites in `state.hpp`, using
the program's own byte model (`texel_bytes`, `state.hpp`: `RGBA16Float` 8,
`RGBA8Unorm` 4, `BGRA8Unorm` 4, `R32Float` 4, `Depth32Float` 4,
`Depth24Plus` 4).

| side | boot block | recomputed here | verdict |
|---|---|---|---|
| textures | 253.9 MiB | 266,242,048 B = **253.908 MiB** | **RECONCILES** |
| buffers | 13.2 MiB | not independently recomputed — see note | — |
| total | 267.1 MiB | 253.908 + 13.2 = **267.108 MiB** | **RECONCILES** |

Note on the buffer side: buffer sizes are `sizeof()` expressions over
~100 `makeBuffer` call sites and cannot be summed without evaluating C++
constant expressions, which this read-only census does not do. The four
buffers ≥ 1 MiB are enumerated in Q5.3 and no buffer reaches the boot
block's top-five cutoff, which is consistent with a 13.2 MiB total.

The block's own stated exclusions are honoured and are not findings:
*"Excludes the surface backbuffer and the console depth texture
(host-owned)."* Both are host-allocated in `console.hpp`, so `noteAlloc`
never sees them.

**No allocation appears on one side and not the other.** The
stop-condition FINDING for a budget mismatch is **not triggered**.

### Q5.2 Textures — every allocation, ≥ 1 MiB marked

`mips` is the mip level count; every texture in the program is created
without `mipLevelCount`, so all are 1. A **sampled** texture without mips
is a bandwidth fact: minification falls to a single level, so a
magnified-to-minified sample walks the full-resolution texels every tap.
The `sampled?` column marks which ones that applies to.

| texture symbol | label | dims | layers | format | B/texel | bytes | **MiB** | mips | sampled? | usage |
|---|---|---|---|---|---|---|---|---|---|---|
| `patchHeightfieldArrayTexture_` | Patch Heightfield Array | 256 × 256 | 225 | `RGBA16Float` | 8 | 117,964,800 | **112.500** | 1 | yes | `StorageBinding \| TextureBinding` |
| `exhibitionTexture_` | Exhibition | 512 × 512 | 40 | `colorFormat` | 4 | 41,943,040 | **40.000** | 1 | yes | `CopyDst \| TextureBinding` |
| `snapshotStagingTexture_` | Snapshot Staging | 512 × 512 | 32 | `colorFormat` | 4 | 33,554,432 | **32.000** | 1 | no | `CopyDst \| CopySrc` |
| `authoredStagingTexture_` | Authored Staging | 512 × 512 | 32 | `colorFormat` | 4 | 33,554,432 | **32.000** | 1 | no | `CopyDst \| CopySrc` |
| `shadowMapTexture_` | Shadow Map | 2048 × 2048 | 1 | `Depth32Float` | 4 | 16,777,216 | **16.000** | 1 | yes (comparison) | `RenderAttachment \| TextureBinding` |
| `spotShadowMapTexture_` | Spot Shadow Atlas | 2048 × 2048 | 1 | `Depth32Float` | 4 | 16,777,216 | **16.000** | 1 | yes (comparison) | `RenderAttachment \| TextureBinding` |
| `liveCardTexture_` | Live Card | 640 × 640 | 1 | `RGBA16Float` | 8 | 3,276,800 | **3.125** | 1 | yes | `StorageBinding \| TextureBinding` |
| `offscreenColorTexture_` | Offscreen Snapshot Color | 512 × 512 | 1 | `colorFormat` | 4 | 1,048,576 | **1.000** | 1 | no | `RenderAttachment \| CopySrc` |
| `offscreenDepthTexture_` | Offscreen Snapshot Depth | 512 × 512 | 1 | `Depth24Plus` | 4 | 1,048,576 | **1.000** | 1 | **no — F2** | `RenderAttachment` |
| `patchCellColorArrayTexture_` | Patch Cell Color Array | 16 × 16 | 225 | `RGBA8Unorm` | 4 | 230,400 | 0.220 | 1 | yes | `StorageBinding \| TextureBinding` |
| `zoneLifeTexture_` | GoL Zone Life | 32 × 32 | 8 | `R32Float` | 4 | 32,768 | 0.031 | 1 | yes | `StorageBinding \| TextureBinding` |
| `pawnAuraTexture_` | Pawn Aura | 64 × 64 | 1 | `RGBA16Float` | 8 | 32,768 | 0.031 | 1 | yes | `StorageBinding \| TextureBinding` |
| `entityGroundAtlasTexture_` | Entity Ground Atlas | 256 × 1 | 1 | `R32Float` | 4 | 1,024 | 0.001 | 1 | yes | `StorageBinding \| TextureBinding` |
| | | | | | | **266,242,048** | **253.908** | | | |

Nine allocations are ≥ 1 MiB. Host-owned and outside the block by design:
the swapchain backbuffer (`W · H · 4`) and `depthTexture_` (`W · H · 4`,
`Depth24Plus`).

**Census note — the top-five tie.** `GPU_TOP_N = 5`, so the boot block's
"largest single allocations" list stops at Shadow Map (16.0 MiB) and
silently omits **Spot Shadow Atlas, which is exactly the same 16.0 MiB**.
The list is truthful (it says "largest", and 5 is the cap) but a reader
sizing the shadow subsystem from the boot block will see 16 MiB where the
program holds 32. Not a reconciliation failure — the byte totals close —
and therefore not the stop-condition FINDING; recorded because the whole
point of a census is that the reader not be surprised.

### Q5.3 Buffers ≥ 1 MiB

| buffer symbol | label | derivation | bytes | MiB |
|---|---|---|---|---|
| `columnVertexBuffer_` | Column VB (GPU mesh gen) | `CMG_TOTAL_VERTICES (48,000) × sizeof(ArchVertex) (40)` | 1,920,000 | 1.831 |
| `archVertexBuffer_` | Arch VB (GPU mesh gen) | `AMG_TOTAL_VERTICES (32,000) × 40` | 1,280,000 | 1.221 |
| `cactusVertexBuffer_` | Cactus VB (GPU mesh gen) | `CACTUSG_TOTAL_VERTICES (20 × 1,500) × 40` | 1,200,000 | 1.144 |
| `palmVertexBuffer_` | Palm VB (GPU mesh gen) | `PALMG_TOTAL_VERTICES (28,800) × 40` | 1,152,000 | 1.099 |

All four are high-water allocations sized by `MAX_*_INSTANCES ×
MAX_VERTS_PER_SLOT`, resident regardless of how many instances are live.
Next largest: Column IB 0.732 MiB, Blade VB 0.610 MiB, Cactus IB
0.610 MiB, Palm IB 0.549 MiB, Patch Height Scratch 0.500 MiB.

### Q5.4 The Patch Heightfield Array — are four channels load-bearing?

The census question the brief posed, answered from the shader.

| field | value |
|---|---|
| allocation | 225 layers × 256 × 256, `RGBA16Float`, **112.5 MiB — the largest single allocation in the program, 44% of all texture bytes** |
| write binding | `@group(0) @binding(24) var patch_heightfield_array_write: texture_storage_2d_array<rgba16float, write>` |
| read binding | `@group(1) @binding(28) var patch_heightfield_array_read: texture_2d_array<f32>` |

**Channel semantics**, from the one and only `textureStore` in the
program (`world.wgsl`, inside the `generate_patch_gradients` kernel):

```
textureStore(patch_heightfield_array_write, texel, layer, vec4(height, grad_x, grad_z, 0.0));
```

| channel | carries | load-bearing? |
|---|---|---|
| `.r` | `height` | **yes** — read as `height_data.x` at both sampling sites |
| `.g` | `grad_x` | **yes** — the gradient pair, read as `.yz` |
| `.b` | `grad_z` | **yes** |
| `.a` | **literal `0.0`** | **NO** |

The shader documents the answer itself, at the sampling site:

```
// .x = height, .yz = gradients, .w = unused (was complexity — swept)
```

**Writing kernels** (2): `generate_patch_heights` and
`generate_patch_gradients`, both `@compute`, dispatched from
`surface/patch_system.hpp` on patch stream.

**Reading entry points** (9, across two stages — see Q6.2 for the
two-binding split): `patch_terrain_vs` and `shadow_patch_terrain_vs`
read it as `patch_heightfield_array_read` (`@group(1) @binding(28)`,
Vertex) through the two `textureSampleLevel` calls in `world.wgsl`;
`compute_entity_placement`, `compute_photographer_vp`, `update_camera`,
`update_cube`, `update_other_agents`, `update_player_agent`, and
`update_sphere` read the same view as `photo_heightfield`
(`@group(0) @binding(145)`, Compute).

**FINDING F3 — one dead channel, 28.125 MiB.** Three of four channels are
load-bearing; the fourth is a hard-coded `0.0` that no reader consumes,
and the shader's own comment records it as swept. At 2 bytes per half
float, `225 × 256 × 256 × 2 = 29,491,200 B = 28.125 MiB` of the program's
largest allocation is a constant zero — **11% of all resident texture
bytes**, and 10.5% of the whole 267.1 MiB budget.

The obvious move (`rgba16float` → `rg16float` + a third channel, or a
packed `rgba16float` → `rgb`-shaped alternative) is **not available as
written**: WebGPU has no three-channel 16-bit storage format, and
`texture_storage_2d_array` write access constrains the choice further.
The realisable variants are `rg16float` (height + one gradient, 56.25 MiB,
requires deriving the second gradient) or splitting into two textures.
Either way the edit must satisfy **nine reading entry points across two
shader stages** and two separate bind-group slots, not the two vertex
shaders a reader of the render passes alone would count. Priced, not
proposed — this census does not rule.

### Q5.5 Paintings — censused as facts only

Per the brief, the painting allocations are **FENCED**: Jean's
improvisation room, recorded and not acted on.

| allocation | MiB | note |
|---|---|---|
| Exhibition | 40.000 | 40 layers × 512², `colorFormat`, `CopyDst \| TextureBinding` — the resident gallery |
| Snapshot Staging | 32.000 | 32 layers, `CopyDst \| CopySrc` — photographer writes here |
| Authored Staging | 32.000 | 32 layers, `CopyDst \| CopySrc` — disk images load here |
| Offscreen Snapshot Color | 1.000 | the photographer's render target |
| Offscreen Snapshot Depth | 1.000 | F2 lives here |
| **total** | **106.000** | **41.7% of all texture bytes** |

Fact, not proposal: the two staging arrays (64 MiB combined) are
`CopyDst | CopySrc` only — never `TextureBinding`, never sampled. They
are pure transfer buffers held resident for the life of the program.

---

## Q6 — HOT vs COLD

Built from the binding ledger's Appendix 2 (the reachability closure,
which resolves each entry point's transitive function calls to the
bindings it actually touches) crossed with the draw roster
(`realization/drawable_table.hpp` and the per-pass forks).

### Q6.1 The draw roster, per pass

`DRAWABLES` (`drawable_table.hpp`), canonical order, with membership:

| drawable | shadow | main | snapshot |
|---|---|---|---|
| pawn | ● | ● | ● |
| sphere | ● | ● | ● |
| monolith | ● | ● | — |
| ribbon | ● | ● | ● |
| arch | ● | ● | ● |
| column | ● | ● | ● |
| palm | ● | ● | — |
| cactus | ● | ● | — |
| blade | ● | ● | — |
| shell | ● | ● | ● |

Plus per-pass forks outside the table: terrain (all three passes, three
different draw shapes), wall paintings and gallery frames (all three
passes), orbs and the fade overlay (main only).

### Q6.2 SAMPLED-PER-FRAME — the bandwidth-relevant set

| texture | MiB | sampled by | shadow | main | snapshot |
|---|---|---|---|---|---|
| `patchHeightfieldArrayTexture_` | 112.500 | `patch_terrain_vs`, `shadow_patch_terrain_vs` (g1 `patch_heightfield_array_read`) — **plus 7 compute kernels** via g0 `photo_heightfield`, see below | ● **outdoor only** | ● | ● |
| `exhibitionTexture_` | 40.000 | `painting_array` — wall painting + gallery frame FS | — | ● | ● |
| `shadowMapTexture_` | 16.000 | `entity_fs`, `patch_terrain_fs`, `ribbon_fs` | — | ● | ● |
| `spotShadowMapTexture_` | 16.000 | `entity_fs`, `patch_terrain_fs`, `ribbon_fs` (indoor) | — | ● indoor | ● indoor |
| `liveCardTexture_` | 3.125 | `patch_terrain_vs`, every plant/arch/column VS, both shadow and main | ● | ● | ● |
| `patchCellColorArrayTexture_` | 0.220 | `patch_terrain_fs` | — | ● | ● |
| `zoneLifeTexture_` | 0.031 | terrain FS | — | ● | ● |
| `pawnAuraTexture_` | 0.031 | terrain FS | — | ● | ● |
| `entityGroundAtlasTexture_` | 0.001 | every plant/arch/column VS, both shadow and main | ● | ● | ● |
| **hot subtotal** | **187.908** | | | | |

**The heightfield is reached through two different bindings, and the
second one is not a render pass at all.** The same texture view
(`patchHeightfieldArrayReadView_`) is bound twice with different
visibilities:

| binding | group/slot | stage | readers |
|---|---|---|---|
| `patch_heightfield_array_read` | `@group(1) @binding(28)` | Vertex | `patch_terrain_vs`, `shadow_patch_terrain_vs` |
| `photo_heightfield` | `@group(0) @binding(145)` | **Compute** | `compute_entity_placement`, `compute_photographer_vp`, `update_camera`, `update_cube`, `update_other_agents`, `update_player_agent`, `update_sphere` |

So the program's largest allocation is sampled by **two vertex shaders
and seven compute kernels**, the latter every frame in both arms
(`update_*` are the agent kernels in `dispatch_compute`). Any FORMAT
ruling against the heightfield — including the F3 dead-channel finding —
must satisfy nine consumers across two stages, not two.

**The shadow pass samples almost nothing.** Exactly three textures are
reachable from any shadow entry point, per Appendix 2:

| shadow entry point | textures reached |
|---|---|
| `shadow_patch_terrain_vs` | `patch_heightfield_array_read`, `live_card_read` |
| `shadow_arch_vs`, `shadow_column_vs`, `shadow_palm_vs`, `shadow_cactus_vs`, `shadow_blade_cluster_vs` | `entity_ground_atlas`, `live_card_read` |
| `shadow_wall_painting_vs` | `live_card_read` |
| `shadow_pawn_vs`, `shadow_sphere_vs`, `shadow_monolith_vs`, `shadow_ribbon_vs`, `shadow_shell_vs`, `shadow_gallery_frame_vs` | **none** |

And **indoors it samples less still**: `draw_shadow_all(c, pass,
cast_terrain=false)` for every atlas tile (the UMBRA_4 spot caster cut),
so `shadow_patch_terrain_vs` never runs in the indoor arm and the
112.5 MiB heightfield is untouched by the indoor shadow pass. The indoor
shadow arm's entire sampled footprint is `live_card_read` (3.125 MiB) and
`entity_ground_atlas` (1 KiB) — while paying 96 MiB of attachment traffic.

That asymmetry is the census's sharpest single reading: **the indoor
shadow pass moves 96 MiB of attachment bytes to sample 3.1 MiB of
texture.**

### Q6.3 RESIDENT-ONLY — the RAM-relevant set

Never sampled by any shader in any pass. These cost memory, not bandwidth.

| texture | MiB | why it is resident | reachable usage |
|---|---|---|---|
| `snapshotStagingTexture_` | 32.000 | photographer's write target, copy source for promotion | `CopyDst \| CopySrc` — no `TextureBinding` |
| `authoredStagingTexture_` | 32.000 | disk images land here, copy source for promotion | `CopyDst \| CopySrc` — no `TextureBinding` |
| `offscreenColorTexture_` | 1.000 | P4's color target; copied out, never sampled | `RenderAttachment \| CopySrc` |
| `offscreenDepthTexture_` | 1.000 | P4's depth target — **F2**, no reader at all | `RenderAttachment` |
| **cold subtotal** | **66.000** | | |

### Q6.4 The split

| set | MiB | share of texture bytes |
|---|---|---|
| hot — sampled per frame | 187.908 | 74.0% |
| cold — resident only | 66.000 | 26.0% |
| **total** | **253.908** | 100% |

Of the cold 66 MiB, 64 MiB is the two painting staging arrays — FENCED
per the brief, recorded and untouched.

---

## ANNEX — THE GRANTS CENSUS

What the audience's hardware actually grants. Row one is recorded from a
live console; the remaining rows are left empty by design and are filled
when Jean pastes them.

| # | date | device / browser | source | adapter | features granted | notable absences |
|---|---|---|---|---|---|---|
| 1 | 2026-08-13 | Pixel 8 / Chrome | `everexpandingboard.com` console | `arm \| valhall` | `bgra8unorm-storage`, `core-features-and-limits`, `depth-clip-control`, `depth32float-stencil8`, `dual-source-blending`, `float32-blendable`, `float32-filterable`, `indirect-first-instance`, `primitive-index`, `rg11b10ufloat-renderable`, `shader-f16`, `subgroups`, `texture-component-swizzle`, `texture-compression-astc`, `texture-compression-astc-sliced-3d`, `texture-compression-etc2`, `texture-formats-tier1`, `texture-formats-tier2`, `timestamp-query` | `texture-compression-bc`, `clip-distances` |
| 2 | | iPhone / Safari | | | | |
| 3 | | desktop / Chrome | | | | |
| 4 | | desktop / Firefox | | | | |

### What row one already licenses

Read against this census, and against nothing else — one row is one
device, not a population.

- **`timestamp-query` is granted.** The frame meter's
  `meter_arm_render` / `ResolveQuerySet` path is live hardware on this
  device, not a desktop-only instrument. `INSTRUMENTS.frame_meter` is off
  by default; the grant says the switch has somewhere to land.
- **`texture-compression-astc` and `-etc2` are granted;
  `texture-compression-bc` is not.** ASTC is the mobile-side lever for
  the 106 MiB of painting bytes. Fenced by the brief; recorded because
  the grants row is where the precondition would be checked.
- **`float32-filterable` is granted.** Not currently used: the shadow
  maps are comparison-sampled (`sampler_comparison` + `CompareFunction::Less`),
  which needs no filterable-float grant, and the census found no
  non-comparison depth sampling anywhere.
- **`core-features-and-limits` is granted**, so the Core defaults the
  binding ledger's gate row prices are the right ceiling for this
  device, and the `maxUniformBufferBindingSize` = 65,536 wall in Q4.4 is
  the real wall here.
- **Nothing in row one bears on FORMAT_1.** `depth16unorm` is a core
  WebGPU format requiring no feature grant, so it needs no row in this
  table to be legal.

---

## THE PRECONDITION MATRIX

What each ruling this census prices would need before it could be taken.
This is a census, not a ruling: these are preconditions, not proposals,
and every one of them is a source edit that PASS_0 does not make.

### ATLAS_1 — one-pass spot atlas

**Priced at:** 96 → 32 MiB/frame at 4 lights (Q3.3); 48 → 16 at 2 lights.

| # | precondition | site | why |
|---|---|---|---|
| A1 | `vp_buffer`'s light slot must hold **all** active light VPs, not one | `GPUState::light_vp_offset()` / `light_vp_size()`, `state.hpp` | the per-tile `CopyBufferToBuffer` (Q4.3) cannot be recorded inside a render pass, so merging the tiles requires every light's matrix resident before the pass opens |
| A2 | the shadow VS must select its light by index | `shadow_*_vs` in `world.wgsl` | with one pass, the draw — not the pass — identifies the light |
| A3 | the tile rectangle must move from `SetViewport`/`SetScissorRect` per pass to per draw inside the pass | `render_shadow_pass` | viewport is a pass-encoder call and is legal mid-pass; this part is free |
| A4 | the two-texture split (`li<2` sun map, `li>=2` spot map) must survive or be retired deliberately | `render_shadow_pass` | it exists because the sun map is idle indoors; one pass per texture keeps it, one pass total does not |
| A5 | `MAX_SPOT_LIGHTS = 4` and the 1×2-per-texture tiling must be restated as one geometry | `state.hpp`, `sample_spot_shadow_pcf` banner | the shader's PCF already names the retired 2×2 scheme; a new tiling must not silently revive it |

### FORMAT_1 — `depth16unorm` shadow maps

**Priced at:** halves every shadow byte — 16 → 8 MiB per texture; with
ATLAS_1, 96 → 16 MiB/frame at 4 lights (Q3.3).

| # | precondition | site | why |
|---|---|---|---|
| F1 | four coupled `Depth32Float` spellings must move together | `state.hpp` (× 2 descriptors), `renderer.hpp` (`shadowDepth.format`, the `DepthStencilState` every shadow pipeline shares), `state.hpp` (`texel_bytes`) | none of the four carries a twin marker, a shared constant, or a `static_assert`; a partial edit fails at pipeline validation **at runtime**, not at compile time |
| F2 | `texel_bytes` must gain a `Depth16Unorm` case | `GPUState::texel_bytes`, `state.hpp` | it returns 0 for unknown formats, which increments `gpuUnknownFormats_` and makes the boot budget print `WARNING: ... total is an UNDERCOUNT`. Q5's reconciliation would break silently. |
| F3 | the depth-bias reasoning must be re-derived | `renderer.hpp`, the PENUMBRA_1 P2 comment block | `depthBias` was **deleted** (not zeroed) because under `Depth32Float` it is a ULP multiple of the primitive's max depth. Under a **unorm** format it becomes a fixed fraction of the range — the deletion's whole justification inverts. The live instruments are `slopeScale`, `depthBiasClamp`, and the normal offset. |
| F4 | precision must be checked against the sun frustum | `renderer.hpp` (frustum depth 599.9), `sample_shadow_pcf` | 16 bits over a 599.9-deep frustum is ~0.0092 world units per step; the post-UMBRA_5 texel is 0.2051 wu. Ratio is favourable but unverified — this census does not measure acne. |
| F5 | the comparison-sampling stack must be confirmed unaffected | `state.hpp` layout entries, `world.wgsl` declarations | `TextureSampleType::Depth` + `sampler_comparison` + `texture_depth_2d` are format-agnostic across depth formats; expected to be a no-op, but it is the stack that would break loudest |

### Painting compression — **FENCED**

| status | **FENCED** |
|---|---|
| scope | Exhibition 40.0 + Snapshot Staging 32.0 + Authored Staging 32.0 + Offscreen Color 1.0 = **105 MiB**, 41.3% of all texture bytes |
| grant | `texture-compression-astc` **is** granted on the Pixel 8 / valhall row; `texture-compression-bc` is **not** |
| authority | the brief: *"Paintings are censused as facts only: they are Jean's improvisation room and fenced from action until he opens them."* |
| this census's action | **none.** Recorded in Q5.5 and Q6.3 as facts. No precondition list is offered, because offering one is the first step of acting. |

---

## FINDINGS — reported, not acted on

| # | finding | where | scale |
|---|---|---|---|
| **F1** | `P3`'s depth is `storeOp: Store` on a texture created `RenderAttachment` only — no `TextureBinding`, no `CopySrc`. It cannot be sampled or read back by anything. `StoreOp::Discard` appears nowhere in the tree. | `console.hpp` `createDepthBuffer`, `render_passes.hpp` `render_main_pass` | `4·W·H` bytes/frame — 7.9 MiB/frame at 1920 × 1080 |
| **F2** | Same shape offscreen: `offscreenDepthTexture_` is `RenderAttachment` only and `P4` stores it. | `state.hpp`, `bodies/gallery.hpp` | 1.0 MiB per capture |
| **F3** | The Patch Heightfield Array's `.a` channel is a hard-coded `0.0` that no reader consumes; the shader's own comment records it as swept. | `world.wgsl` `generate_patch_gradients` + the sampling comment | 28.125 MiB resident — 11% of all texture bytes |
| **F4** | The indoor shadow arm moves 96 MiB of attachment traffic per frame (4 lights) to sample 3.1 MiB of texture, because the terrain caster cut (UMBRA_4) removed the only large sampler from that pass without changing the attachment structure. | `render_passes.hpp` `render_shadow_pass` + `draw_shadow_all` | 32 MiB/frame of it is pure `Load` preservation |

Observations that are **not** findings, recorded so the reader is not
surprised: the boot block's top-five omits Spot Shadow Atlas at an equal
16.0 MiB (`GPU_TOP_N = 5`, Q5.2); the surface format, `colorSpace`, and
`toneMapping` are all unset or runtime-derived (Q1.3).

## CLOSE

Reconciliation closed: textures recompute to 253.908 MiB against the boot
block's 253.9, and the total to 267.108 against 267.1. No allocation
appears on one side and not the other, so the campaign's budget-mismatch
stop condition was not triggered. One pass descriptor builds its ops
dynamically; both branches are reported in Q2.1 rather than collapsed.
Read-only throughout — no source file was edited.
