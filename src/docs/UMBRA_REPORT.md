# UMBRA_1 — RECON REPORT

Read 2026-07-30 against the tree at `0466346`. Scope: `src/cartridges/the_board/**`
and `src/incubator_dual.cpp`. Generated C++ text inside `src/tools/*.jsx` excluded.
Nothing compiled, nothing run — Jean holds the gates.

Every item below was read from a **descriptor or a call site**. Where a label and a
descriptor disagreed, the descriptor won and the label is filed under R11.

---

## THE HEADLINE — the campaign's model and the tree disagree in one structural way

**The sun view/ortho pair is built on the GPU, in WGSL, not CPU-side.**
`coupling_pawn_to_sun_vp` (`world.wgsl`, `— [COUPLING:pawn→sun:view_proj]`) builds
both matrices and returns `proj * view`. There is **no** CPU-side sun ortho anywhere
in scope. R10 asked which file+function builds them CPU-side; the answer is *none*,
and the exhaustive search behind that claim is recorded under R10.

This does **not** fire the global STOP condition. That condition reads *"the sun
frustum construction is not reducible to center + radius + near/far."* It **is** so
reducible — center = `point_pos()`, radius = `SUN_HALF_EXTENT`, near/far =
`SUN_NEAR`/`SUN_FAR` — the construction simply lives in the other room. UMBRA_2's
site moves from R10-CPU to R10-WGSL and its arithmetic is unchanged.

Two consequences ripple forward, both good:

- **UMBRA_2** is a WGSL edit, not a C++ edit. It is still FXC-clean: no arrays, no
  runtime branching added, no new bindings.
- **UMBRA_7's `TEXEL_WORLD` needs no uniform and no layout growth.** Both of its
  factors — `SUN_HALF_EXTENT` and `SHADOW_MAP_SIZE` — are already module-scope WGSL
  `const`s *in the same file as the sampling function*. `TEXEL_WORLD` is therefore a
  const-expression, which is strictly better than the handoff's preference (a): no
  GROWTH LAW, no padding hunt, no STOP. See R8.

---

## R1 — MAPS

**Sun shadow map** — `state.hpp`, `createTextures`, block labelled `"Shadow Map"`:

```cpp
wgpu::TextureDescriptor desc{};
desc.label = "Shadow Map";
desc.size = { Dim::SHADOW_MAP_SIZE, Dim::SHADOW_MAP_SIZE, 1 };
desc.format = wgpu::TextureFormat::Depth32Float;
desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
```

`Dim::SHADOW_MAP_SIZE` = `2048` (`state.hpp`, `namespace Dim`, "Lighting" block).
So: **2048 × 2048, Depth32Float, one texture, no array, no mips.**

**Spot shadow maps** — `state.hpp`, `createTextures`, block labelled
`"Spot Shadow Atlas"`: same dimensions, same format, **one** texture.

But the spot system uses **two** textures, not one — and the second is the sun map:

```
//   shadow_map      (repurposed sun map) → lights 0, 1
//   spot_shadow_map                      → lights 2, 3
```
(`world.wgsl`, banner above `sample_spot_shadow_pcf`)

Confirmed at the pass, not just the comment — `render_passes.hpp`,
`render_shadow_pass`: `bool use_sun_map = (li < 2);` selects the attachment, and
`within = li % 2` selects the left/right half via `SetViewport`/`SetScissorRect`
with `TILE_W = Dim::SHADOW_MAP_SIZE / 2`, `TILE_H = Dim::SHADOW_MAP_SIZE`.

So the live shape is **two textures × two half-width tiles = 4 spot slots**,
`MAX_SPOT_LIGHTS = 4` (`state.hpp`), tile = 1024 × 2048 today.

**This is the fact that governs UMBRA_5.** The sun map's size is not the sun map's
alone — the sun map *is* spot storage during indoor moods. `Dim::SHADOW_MAP_SIZE`
is one fact with one C++ home serving both roles by design.

**Depth format is float.** UMBRA_6 flagged this in advance: the constant-bias unit
under `Depth32Float` is ULP-relative, not a fixed fraction. Quantified in R4.

**Every C++ read of `Dim::SHADOW_MAP_SIZE`:** the two texture descriptors above, and
`TILE_W`/`TILE_H` in `render_shadow_pass`. Four sites, one constant.

**Every WGSL read of `SHADOW_MAP_SIZE`** (`world.wgsl`, `const SHADOW_MAP_SIZE: f32
= 2048.0`): `SHADOW_BIAS_MIN`, `SHADOW_BIAS_MAX`, `texel_size` in
`sample_shadow_pcf`, `SPOT_DEPTH_BIAS`, `SPOT_SLOPE_BIAS_MAX`, `texel_size` in
`sample_spot_shadow_pcf`. Six sites, one constant. Four of the six die in UMBRA_6.

---

## R2 — FRUSTUM

Site: `world.wgsl`, `fn coupling_pawn_to_sun_vp`. Parameters, verbatim:

```wgsl
const SUN_ALTITUDE: f32 = 250.0;
const SUN_HALF_EXTENT: f32 = 300.0;
const SUN_NEAR: f32 = 0.1;
const SUN_FAR: f32 = 600.0;
const SHADOW_SNAP_SIZE: f32 = 2.0;   // world units — shadow VP snaps to this grid
```

**Centers on:** THE POINT, not the pawn. `compute_vp` passes `point_pos()`, which is
`camera_state.pos` when the camera hosts and the possessed body otherwise. The
in-file comment is explicit and, unusually, correct:

```wgsl
    // Sun VP: kite coupling — the sun orbits THE POINT at fixed
    // offset (was the pawn; the 300-unit shadow box must cover
    // what the eye sees, so it follows the point's host — identical
    // when the pawn hosts, tracks the camera in free-fly).
```

- **Radius (R0):** `SUN_HALF_EXTENT` = 300.0 world units, symmetric ortho ±R0.
- **Near/far:** 0.1 / 600.0. Sun sits `SUN_ALTITUDE` = 250 up-light of the center.
- **Refit cadence:** every frame, unconditionally, in the `compute_vp` compute entry
  point — gated only by `coupling_active(COUPLING_PAWN_TO_SUN_VP)`. **There is no
  per-frame refit of radius/near/far to delete: they are already frozen constants.**
  UMBRA_2 step 1 is therefore already satisfied by construction, and the "delete the
  refit code" instruction has no referent. Reported, not improvised around.

**Existing snapping — and why it is the wrong snap.** Verbatim:

```wgsl
    var snapped = pawn_pos;
    snapped.x = round(pawn_pos.x / SHADOW_SNAP_SIZE) * SHADOW_SNAP_SIZE;
    snapped.z = round(pawn_pos.z / SHADOW_SNAP_SIZE) * SHADOW_SNAP_SIZE;
```

This quantizes **world XZ**, on a 2.0-unit grid. The light-space sample grid is
axis-aligned to *world* XZ only when the sun points straight down. For every other
sun direction — which is every mood — the snapped center still lands at an arbitrary
sub-texel offset in light space, and the sample grid slides exactly as if nothing
were snapped. The grid it does quantize (2.0 wu) is also **6.8× coarser than a texel**
(0.293 wu today), so even in the degenerate straight-down case it is snapping to the
wrong lattice.

This is the fire UMBRA_2 was written to put out, and the existing snap is not a
partial fix — it is a fix aimed at the wrong space. UMBRA_2 replaces it.

**Second caller.** `coupling_pawn_to_sun_vp` is called twice: `compute_vp` (the live
sun VP) and the photographer kernel (`photographer_vp.light_vp = coupling_pawn_to_sun_vp(point_p, cfg.sun_direction);`).
Both get UMBRA_2's snap for free — the edit is inside the function.

---

## R3 — LIGHT DIRECTION

**Static per mood. Not animated per frame.** The only writer is
`apply_mood_lighting` (`direction/mood.hpp`), which copies `MoodProfile::sun_direction`
into `sunDirection_` and pushes a normalized copy to the GPU config:

```cpp
    c->gpuState_.set_sun_direction(m.sun_direction[0] / len,
                                m.sun_direction[1] / len,
                                m.sun_direction[2] / len);
```

`set_sun_direction` (`state.hpp`) is itself change-guarded — it compares before
writing and only dirties the config on an actual change. There is no lerp, no
per-frame drive, no time term anywhere on the path. `gallery.hpp` re-normalizes the
same member into the photographer config; it is a reader, not a second writer.

**Consequence for UMBRA_2:** the snap is total, not partial. The residual-tremble
caveat in UMBRA_2 step 3 does not apply and will not be written into the commit body
— the sun rotates only across a mood transition, which is already covered by a fade.

**Consequence for UMBRA_2's arithmetic:** `config.sun_direction` reaches the shader
**pre-normalized at every writer**. The light basis `{right, true_up, fwd}` is
therefore genuinely orthonormal, which is what makes the light-space snap exact
without a `normalize()` added inside the named edit.

---

## R4 — SHADOW PIPELINES

**One builder, one depth-stencil state, every shadow pipeline.** `renderer.hpp`,
inside `createPipelines`, the block introduced by "THE SHARED BUILDER (shadow/depth
category)". Verbatim:

```cpp
wgpu::DepthStencilState shadowDepth{};
shadowDepth.format = wgpu::TextureFormat::Depth32Float;
shadowDepth.depthWriteEnabled = true;
shadowDepth.depthCompare = wgpu::CompareFunction::Less;
```

- `depthBias` — **ABSENT** (defaulted `0`).
- `depthBiasSlopeScale` — **ABSENT** (defaulted `0.0f`).
- `depthBiasClamp` — **ABSENT** (defaulted `0.0f`).

A repo-wide grep for `depthBias` across the census scope returns **nothing**. There
is no rasterizer bias in this program at all today. UMBRA_6's step 1 is a
**three-line insertion at exactly one site** that reaches every shadow pipeline —
the best possible shape for it.

`makeShadow` forks only on `(label, dbgLabel, vsEntry, vbl, cullMode, out)`;
topology is `TriangleList`, `frontFace` is `CCW`, `fragment` is `nullptr`
(depth-only), layout is `shadowRenderLayout` — shared by all.

**Cull modes, verbatim from the call sites:**

| pipeline | `cullMode` |
|---|---|
| `shadowPatchTerrainPipeline_` | `Back` |
| `shadowPawnPipeline_` | `None` |
| `shadowSpherePipeline_` | `Back` |
| `shadowMonolithPipeline_` | `Back` |
| `shadowArchPipeline_` | `Back` |
| `shadowColumnPipeline_` | `None` |
| `shadowPalmPipeline_` | `None` |
| `shadowCactusPipeline_` | `None` |
| `shadowBladePipeline_` | `None` |
| `shadowShellPipeline_` | `None` |
| `shadowRibbonPipeline_` | `None` |

**No pipeline uses `CullMode::Front`.** UMBRA_6 step 3 is therefore a no-op:
"If back or none, leave it." Left. (`shadow_pyramid` was cut by an earlier orphan
sweep; the comment saying so is accurate.)

**The spot pass reuses these same pipelines.** `render_shadow_pass` calls
`draw_shadow_all` from both branches. So UMBRA_6's rasterizer bias governs the spot
atlas writes too — which is the correct outcome, and it is what licenses deleting
the spot-side shader bias in the same commit.

**The float-format flag UMBRA_6 asked for.** Under `Depth32Float`, WebGPU computes
the constant bias as `r = 2^(exp(max depth in primitive) − mantissa bits)`, i.e. in
**ULP**, not in fixed depth units. The shader constants it replaces are
`SHADOW_BIAS_MIN` = 2.0e-4 and `SHADOW_BIAS_MAX` = 4.0e-3 *at the pre-campaign
RES = 2048* — **three to four orders of magnitude larger** than the constant.

> **Corrected after the P3 pass — see finding 6 in the ledger.** The figures I first
> gave here were wrong in two ways, and both are worth keeping visible rather than
> quietly overwriting. (a) I read the ULP at `z ≈ 1`, giving 1.2e-7. This scene's
> sun-map geometry sits near **z = 0.417** (the light is `SUN_ALTITUDE` = 250 above
> ground in a 599.9-deep frustum), where one ULP is 2^-25 = 2.98e-8 — so
> `depthBias = 2` buys **6.0e-8 NDC**, four times less. (b) The slope term matches
> the old `mix()` at 45° and *only* near there: new/old runs 0.00 at normal
> incidence, ≈1.1 through 30–60°, then 2.3 / 4.3 / 10.1 at 80 / 85 / 88° and
> unbounded beyond, since `depthBiasClamp = 0.0` means **no clamp**. Reporting the
> 45° agreement as general would have been this campaign's own failure mode.

The slope term is where the work lands in the mid-range. The sun ortho maps 599.9
world units of depth onto [0,1], so at post-UMBRA_5 texel size 0.2051 wu a 45°
receiver gives `dz/dtexel = 3.4e-4`; times `depthBiasSlopeScale = 2.0` that is
**6.8e-4**, against the old term's 6.6e-4 there. It is correctly zero on surfaces
facing the light — which is the intended shape *except* where caster and receiver are
different tessellations, and that exception is real here (finding 2).
**`depthBias = 2` is kept as the start, flagged as the handoff instructed.**
*(Superseded: PENUMBRA_1 P2 deleted `depthBias` outright. On a float depth format it
is a ULP multiple, so 2 bought 6.0e-8 NDC — not a dial in steps of one. This line is
a decision statement inside a dated recon, so it is struck rather than edited.)*

---

## R5 — SHADER-SIDE NUDGES

**Write path.** Eleven shadow VS entry points exist:
`shadow_patch_terrain_vs`, `shadow_pawn_vs`, `shadow_sphere_vs`,
`shadow_monolith_vs`, `shadow_arch_vs`, `shadow_column_vs`, `shadow_shell_vs`,
`shadow_ribbon_vs`, `shadow_palm_vs`, `shadow_cactus_vs`, `shadow_blade_cluster_vs`.

**Exactly one of them nudges.** `shadow_pawn_vs`:

```wgsl
    // Lift pawn above terrain in shadow map. With perspective projection
    // from a ceiling light, 0.01 is invisible in the depth buffer at 19+
    // units distance. 0.3 gives enough depth separation to clear the
    // terrain without visibly displacing the shadow shape from overhead.
    var shadow_pos = world_pos;
    shadow_pos.y += 0.3;
```

The other ten are clean. Their `ground_y` / `sample_live_card(...)` terms are
**geometry, not bias** — each is the identical term its colour-pass twin applies, so
the shadow silhouette tracks the visible body. Not nudges; not deleted.

**Compare path — sun**, `sample_shadow_pcf`:

```wgsl
const SHADOW_BIAS_MIN: f32 = 0.4096 / SHADOW_MAP_SIZE;
const SHADOW_BIAS_MAX: f32 = 8.192 / SHADOW_MAP_SIZE;
```
```wgsl
    let light_dir = -render_light.direction;
    let cos_theta = max(dot(normal, light_dir), 0.0);
    let bias = mix(SHADOW_BIAS_MAX, SHADOW_BIAS_MIN, cos_theta);

    let current_depth = light_ndc.z - bias;
```

**Compare path — spot**, `sample_spot_shadow_pcf`:

```wgsl
const SPOT_DEPTH_BIAS: f32 = 6.144 / SHADOW_MAP_SIZE;    // base bias, scaled by 1/clip.w
const SPOT_SLOPE_BIAS_MAX: f32 = 20.48 / SHADOW_MAP_SIZE;  // extra bias at grazing angles
```
```wgsl
    let light_dir = normalize(light.position - world_pos);
    let cos_theta = max(dot(normal, light_dir), 0.001);
    let slope_bias = SPOT_SLOPE_BIAS_MAX * (1.0 - cos_theta) / cos_theta;
    let total_bias = (SPOT_DEPTH_BIAS + slope_bias) / max(light_clip.w, 1.0);

    let current_depth = light_ndc.z - total_bias;
```

**Not a nudge, and kept:** the `+ 0.5` inside `sample_spot_shadow_pcf`'s tap offsets
(`vec2(f32(x) + 0.5, f32(y) + 0.5) * texel_size`). That is *sample centering*, not
depth bias — and its absence from the sun kernel is a genuine defect, filed at R6.

**Note for UMBRA_6.** The sun bias is *already* slope-scaled in the shader
(`mix(MAX, MIN, cos_theta)`). UMBRA_6's stated intent — "one constant is doing two
jobs and failing both" — describes a single-constant bias this tree does not have.
The edit still stands on its own merits: the rasterizer computes slope from the
*actual* screen-space depth gradient rather than from a receiver-normal proxy, it
applies at write time so it costs no fragment work, and it gives bias one home
instead of two. Executed as named, with the reasoning corrected here rather than
silently.

---

## R6 — SAMPLING SITE

Site: `world.wgsl`, `fn sample_shadow_pcf(world_pos: vec3<f32>, normal: vec3<f32>) -> f32`.

- **Stage:** fragment. Reached from `calc_directional_light`, itself called from the
  lighting composition in the FS. **Light-space coords are built in the fragment
  stage** — `let light_clip = render_vp.light_vp * vec4(world_pos, 1.0);` — not in a
  vertex stage, and not carried in a varying.
- **Receiver normal:** **available, as a direct function parameter** (`normal`),
  world-space, threaded from the FS through `calc_directional_light`. **UMBRA_7's
  precondition is satisfied with no varying, no struct, and no layout change.**
  This is the strongest possible form of that precondition.
- **Sampler:** `@group(1) @binding(26) var shadow_sampler: sampler_comparison;` —
  **already a comparison sampler**, created with `desc.compare = wgpu::CompareFunction::Less`
  and `magFilter`/`minFilter` = `Linear`, `ClampToEdge` on both axes
  (`state.hpp`, `createSamplers`, `"Shadow Sampler (PCF comparison)"`). The layout
  entry is `wgpu::SamplerBindingType::Comparison`. **UMBRA_8 step 1 needs no
  retyping and no growth — the binding is already what it asks for.**
- **Taps:** 16, as a 4×4 double loop with `textureSampleCompare`.

**A defect, found by reading the offsets rather than the comment.** Verbatim:

```wgsl
    for (var y: i32 = -2; y <= 1; y++) {
        for (var x: i32 = -2; x <= 1; x++) {
```

That yields offsets `{-2,-1,0,1}` on each axis. The kernel's centroid sits at
**−0.5 texels in both x and y** — every sun shadow in the program is displaced half
a texel up-left of its caster, permanently. The spot kernel has the same loop bounds
but corrects for it with the `+ 0.5` term R5 quotes; the sun kernel has no such term.

UMBRA_8's symmetric 3×3 (`-1, 0, 1`) **fixes this as a side effect**, and that is a
second, unbudgeted reason to prefer nine centred taps to sixteen off-centre ones.

**Callers:** one — `calc_directional_light`, which passes its own `normal` parameter
through unchanged, and which gates the whole call:

```wgsl
    var shadow = 1.0;
    if (render_spot_lights.count == 0u) {
        shadow = sample_shadow_pcf(world_pos, normal);
    }
```

So the sun PCF is skipped entirely whenever spot lights are live — the sun map is
holding spot tiles then, and sampling it as a sun map would read garbage. Correct as
written; noted because it means **UMBRA_7 and UMBRA_8 change outdoor moods only.**

---

## R7 — CASTER LISTS

The sun shadow pass and the spot shadow pass call **the same body**,
`draw_shadow_all` (`render_passes.hpp`). Draw-for-draw they are today *identical*.
The list, in order:

1. **Terrain, band 0** — `draw_shadow_patch_terrain(..., patch_index_buffer_lod1(),
   patch_index_count_lod1(), lod0_patch_count)`.
2. **Terrain, band 1** — a bare `pass.DrawIndexed(patch_index_count_lod1(), ...)`
   reusing the already-bound LOD1 IB.
3. **The drawable table**, filtered to `DRAW_SHADOW`: `pawn`, `sphere`, `monolith`,
   `ribbon`, `arch`, `column`, `palm`, `cactus`, `blade`, `shell` — ten rows, each
   further gated by its `ROSTER` flag at pipeline-build time.

### (a) Curtains — already out, and the label says otherwise

**Curtain geometry is terrain**: the curtain band of the LOD0 index buffer, 16 quads
per cell × 256 cells, welding cap-perimeter verts to their curtain-bottom twins to
seal discontinuous cell-lift seams (`state.hpp`, `build_lod0_ib`).

`cartridge.hpp` states the scope law: *"curtains exist ONLY in the LOD0 index
buffer."* Read at the builder, that is true — the LOD1 IB is built from the interior
grid at `step = 2` plus a coarse skirt ring, and **emits no curtain band at all**.

The shadow pass draws **both** terrain bands through `patch_index_buffer_lod1()`.
**Therefore the sun caster list already contains zero curtain geometry**, and has
since ECONOMY_1 E2 pinned the shadow terrain to LOD1 density.

UMBRA_3's step 2 is **already landed**. There is no boolean to add and nothing to
exclude. What *is* left is a stale label, which contradicts the tree at the very site
UMBRA_3 would have edited — `shadow_patch_terrain_vs`:

```wgsl
    // Same unified decode as patch_terrain_vs — the shadow pass shares
    // the patch index buffers (cap + curtain + skirt bands).
```

The shadow pass shares the *decode*; it does not share the curtain band. Filed at R11.

### (b) Caster LOD — already pinned, and not eye-driven

**The ladder is two rungs**, both terrain-mesh densities of a 50 wu patch
(`Dim::PATCH_EXTENT = 50.0f`):

| rung | subdivisions | quad edge |
|---|---|---|
| LOD0 | `PATCH_MESH_N = 64` | 0.78125 wu |
| LOD1 | `PATCH_MESH_N_LOD1 = 32` | 1.5625 wu |

**Nothing selects between them for shadow casters.** `draw_shadow_all` passes the
LOD1 buffer unconditionally, for both bands — no eye distance, no pawn distance, no
per-patch test. The eye/point-driven banding (`lod0_radius` = 175 wu, `lod_point_x/z`)
governs the **main** pass only.

**Against UMBRA_3's rule**, computed with post-UMBRA_5 values as instructed
(R1 = 1.4 × 300 = 420, RES1 = 2 × 2048 = 4096, so `texelWorld` = 840/4096 =
**0.2051 wu**, and 2 × `texelWorld` = **0.4102 wu**):

- LOD0 edge 0.78125 wu = 3.8 texels — **finer than the target can resolve**: pure cost.
- LOD1 edge 1.5625 wu = 7.6 texels — coarser than the target.

*No* rung satisfies "edge ≤ 2 × texelWorld", so the rule as literally written selects
nothing. The handoff anticipated this: *"If the ladder makes the rule ambiguous, STOP
and report the ladder — Jean rules."* **The ladder is reported above.** The
disposition is that the rule's *intent* — never draw a caster denser than the map can
resolve — is already fully satisfied, because the pin is already at the ladder's
coarsest rung and there is nothing coarser to move to. **UMBRA_3 lands no edit.**

### (c) Terrain in the spot lists — yes, and this is UMBRA_4's whole target

Because `draw_shadow_all` is shared, **every spot light re-draws the entire terrain**,
once per light, up to `MAX_SPOT_LIGHTS = 4`. Indoors, under a shell, with a ceiling
lamp whose cone never reaches the horizon. That is 4 × (two terrain draws at LOD1
density over the full active patch set) of pure waste inside the known bottleneck.

**After UMBRA_4's cut, a spot list is:** the ten drawable-table rows, roster-gated —
`pawn`, `sphere`, `monolith`, `ribbon`, `arch`, `column`, `palm`, `cactus`, `blade`,
`shell` — and nothing else. The indoor scene's actual occluders, which is the whole
of what an indoor spot can meaningfully shadow.

---

## R8 — BIND STRUCTURE

| resource | group | binding | owner |
|---|---|---|---|
| `shadow_map` | 1 | 25 | `renderTextureBindGroupLayout_` |
| `shadow_sampler` | 1 | 26 | `renderTextureBindGroupLayout_` |
| `spot_shadow_map` | 1 | 27 | `renderTextureBindGroupLayout_` |

Numbers authored in `binding_registry.hpp`, `namespace bind::g1`; the WGSL
`@binding` literals are its mirror (L6). Verified equal at both rooms.

**The owning layout is shared, and heavily.** `"Render Texture Layout"` carries
**11 entries**: both samplers, the two shadow textures + comparison sampler, the
patch heightfield array, the patch cell-colour array, the GoL zone life texture, the
GoL zone params buffer, the pawn aura texture, and the live card. It is the main
render pass's texture group for every drawable.

**Growing it fires the global STOP condition.** So UMBRA_7 must not put
`TEXEL_WORLD` there, and UMBRA_8 must not add a sampler. Neither needs to:

- **UMBRA_8** — `shadow_sampler` is *already* `sampler_comparison` with `Linear`
  filtering and `compare = Less` (R6). Replacement, not growth; in fact not even
  replacement. Nothing to do but change the call.
- **UMBRA_7** — `TEXEL_WORLD` is **a const-expression**, not a uniform:

  ```
  2 * SUN_HALF_EXTENT / SHADOW_MAP_SIZE
  ```

  Both operands are module-scope WGSL `const`s declared *above* the sampling
  function in the same file. There is no fact without a home here, so the handoff's
  option (a)/(b) ladder — spare padding, else STOP — is never entered. This is
  strictly better than (a): a const costs no bytes, cannot drift from the descriptor
  it derives from, and needs no `sizeof` witness. **Recorded under P7** as a
  structural choice taken inside a named edit: the minimal form, reported.

**Uniform structs reaching the sampling shader,** for completeness, since R8 asked:
`render_vp: VPMatrix` (`@group(0) @binding(201)`, `{ m, light_vp }`, two mat4 — no
padding), `render_light: DirectionalLight` (`@binding(320)`; carries `_pad0` and
`_pad1/_pad2/_pad3` — 4 spare floats, and thus *would* have been a legal home under
(a) had one been needed), `render_point_lights` (`@binding(321)`),
`render_spot_lights` (`@binding(322)`), and `config`. All are group-0 storage/uniform
bindings, all already reach the fragment stage. **None is grown by this campaign.**

---

## R9 — FXC

The handoff asked for "the `world.wgsl` banner FXC block, verbatim". The banner's FXC
block is a **pointer**, not the constraints — reproduced verbatim:

```
//   L2  FXC — the Windows D3D12 backend's hard limits, honored by
//       structure. READ L2 BEFORE adding a branch to the collision/
//       ground chain or a texture-array stamp anywhere near it.
```

The banner names its own delegation elsewhere in the same block: *"the operational
home of the specifics is the world.wgsl FXC banner — the banner owns the
constraints"* — but the constraints themselves live in `src/docs/LAWS.md` L2, which
the banner points to. The two rooms disagree about which of them owns the rules; the
rules exist in exactly one of them. Filed at R11 as a documentation seam, not chased
here.

**L2 — THE FXC LAW**, verbatim, since it is what actually binds these edits:

```
1. Instance structs in hot loops stay lean and byte-pinned — the pattern's
   live exemplar is the `GPUSpotLightArray` pin (`static_assert` in
   `state.hpp`: `16 + MAX_SPOT_LIGHTS * 128`).
2. The collision/ground chain admits **no new runtime branching**. The live
   exemplar: the pyramid loop bounds itself by a uniform —
   `min(pyramid_instances.count, MAX_PYRAMID_INSTANCES)` in world.wgsl —
   and dispatch is by uniform function choice, never by branch.
3. Texture-array stamps in the collision chain **hang FXC**. Do not add one.
4. Storage buffers per stage = 10. Uniform buffers per stage = 12.
```

**Compliance of this campaign, item by item.** UMBRA_2 adds arithmetic to a compute
entry point — no branch, no array, no texture stamp. UMBRA_7 adds a normal-offset
term — pure arithmetic in the fragment stage, nowhere near the collision/ground
chain. UMBRA_8 **removes** a double loop and replaces it with nine straight-line
calls at const offsets, which strictly reduces FXC's unrolling work; and
`textureSampleCompareLevel` carries no implicit derivatives, so no uniformity
diagnostic can fire in any stage. **No edit adds a binding, so items 1 and 4 are
untouched.** Net FXC surface across the campaign: negative.

---

## R10 — MATRIX SITE

**Exactly one site constructs the sun ortho, and it is in WGSL.**
`world.wgsl`, `fn coupling_pawn_to_sun_vp`, returning `proj * view` where:

```wgsl
    let proj = mat4x4<f32>(
        vec4(1.0 / he,  0.0,       0.0,                   0.0),
        vec4(0.0,        1.0 / he,  0.0,                   0.0),
        vec4(0.0,        0.0,      -r_depth,               0.0),
        vec4(0.0,        0.0,      -SUN_NEAR * r_depth,    1.0)
    );
```

**The search behind "exactly one", so the claim is falsifiable rather than asserted:**
`ortho`, `orthographic`, `half_extent`, `SUN_`, `light_vp`, `lightView`,
`compute_sun`, `sun_matrices` were each grepped across the whole census scope. The
only projection constructions found are: this one (sun, ortho, WGSL);
`build_view_projection_matrix` (camera, **perspective**, WGSL); `build_lookat_vp`
(photographer, **perspective**, WGSL); and `compute_spot_light_vp`
(`render_passes.hpp`, spot, **perspective**, C++). No CPU-side sun ortho exists.

`compute_spot_light_vp` is therefore **not** a second ortho construction — it is a
perspective VP for a cone light, which is the correct projection for a point source,
and it is CPU-side because spot lights are CPU-authored per mood. **No STOP.**

**How `render_vp.light_vp` is fed, per pass** — this is the fact that makes the
sun/spot sharing legible:

- **Sun pass:** `compute_vp` writes `vp_data.light_vp` on the GPU each frame.
- **Spot pass:** `render_shadow_pass` overwrites the *same 64 bytes* with a
  `CopyBufferToBuffer` from `spot_vp_staging()` before each per-light sub-pass —
  `GPUState::light_vp_offset()`, `light_vp_size()`. One slot, two writers, never
  concurrently.

That is also why `calc_directional_light` must skip the sun PCF when spot lights are
live (R6): the slot is holding a spot matrix.

---

## R11 — STALE LABELS

Verified each against the descriptor or the array it describes. Disposition per P5.

**1. `world.wgsl`, above the group-1 texture declarations — REGISTRY WORK, corrected.**

```wgsl
// --- Render textures (Group 1: bindings 22-23, 25-26)
```
Three lines below it, `@binding(27) var spot_shadow_map` is declared. The C++ twin
(`state.hpp`, `"Render texture layout"`) reads `bindings 22-23, 25-29, 31-34` and is
**correct**. The WGSL comment enumerates bindings — registry work — so it is
corrected to `22-23, 25-27`, not deleted. **Folded into UMBRA_8's commit** (the
commit that touches this sampling neighbourhood).

**2. `world.wgsl`, `shadow_patch_terrain_vs` — BEHAVIOUR CLAIM, corrected.**

```wgsl
    // the patch index buffers (cap + curtain + skirt bands).
```
False since ECONOMY_1 E2: the shadow pass binds the **LOD1** IB, which has a grid
band and a coarse skirt ring and **no curtain band**. Evidence at R7(a). Corrected in
UMBRA_3's commit — the handoff's "delete-not-annotate" applied to the wrong clause,
not to the whole comment, whose first sentence is true and load-bearing.

**3. `state.hpp`, the `SHADOW_MAP_SIZE` TWIN comment — TRUE, and load-bearing.**

```cpp
            // TWIN: world.wgsl `const SHADOW_MAP_SIZE: f32` (— Shadow
            // constants). The WGSL twin feeds BOTH PCF texel_size reads.
```
Verified true in both rooms. Not stale. **It must be edited by UMBRA_5 anyway**,
because it says "BOTH PCF texel_size reads" and after UMBRA_6 deletes four of the six
WGSL readers, "both" becomes the only two that remain — the sentence stays true by
accident. Left alone; noted so a future reader knows it was checked, not missed.

**4. `world.wgsl` banner vs `LAWS.md` L2 — circular delegation.**
The banner points at L2 for the FXC specifics; L2 says *"the operational home of the
specifics is the world.wgsl FXC banner"*. Each names the other as the owner. The
constraints live in L2. Not corrected here — `LAWS.md` is outside this campaign's
scope and P8 says a law exists only where it is committed. **Flagged for the next
campaign that opens `LAWS.md`.**

**5. Comments checked and found TRUE** (recorded so the absence of a finding is
evidence, not silence): the two-texture spot atlas banner; the `"(repurposed sun
map)"` claim; `"curtains exist ONLY in the LOD0 index buffer"`; the `"Shadow Texture
Layout ... NO shadow map"` comment and its 4-entry array; the `"Render texture
layout"` 11-entry binding list; `"shadow_pyramid pipeline CUT"`; the kite-coupling
comment on the sun VP centering on THE POINT.

---

## STOP CONDITIONS — none fired

| condition | status |
|---|---|
| A named anchor absent, or a different count than stated | **Clear.** Every anchor verified verbatim. Two anchors are *already satisfied* rather than absent — R2's frozen frustum, R7's curtain exclusion — reported as findings, per the register. |
| An edit requires growing a shared bind-group layout | **Clear.** No edit grows any layout. `TEXEL_WORLD` is a const (R8); the comparison sampler already exists (R6). |
| An edit conflicts with the FXC banner | **Clear.** Net FXC surface is negative (R9). |
| Sun frustum not reducible to center + radius + near/far | **Clear.** It reduces exactly. It is built in WGSL rather than C++ — a site correction, not a shape violation (R10). |

---

## WHAT THE CAMPAIGN ACTUALLY BUYS, given the tree as read

The handoff named two suspects for "shadows compose as we approach": a sliding
sample grid, and caster silhouettes that change with eye distance. The recon
**acquits the second and convicts the first**.

Caster LOD does not vary with the eye at all (R7b) — it is pinned, unconditionally,
and has been since ECONOMY_1 E2. So no silhouette changes as the camera moves.

The sample grid, meanwhile, slides freely: the existing snap quantizes world XZ on a
lattice that is neither the light's nor a texel's (R2). Every static edge in the
scene re-rasterizes at a shifted threshold on every frame the point moves. That is
the fire, it is the whole fire, and UMBRA_2 is the water.

Two further defects surfaced that the handoff did not predict, and both are fixed by
edits already in it:

- **The sun PCF kernel is off-centre by half a texel** (R6) — every sun shadow in the
  program sits half a texel up-light of its caster. UMBRA_8's symmetric 3×3 removes it.
- **Terrain is redrawn once per spot light, indoors, under a shell** (R7c) — UMBRA_4.

## COSTS, NAMED IN ADVANCE

**UMBRA_5's VRAM.** `Dim::SHADOW_MAP_SIZE` sizes *both* depth textures, because the
sun map is deliberately reused as spot storage indoors (R1). Doubling it per side is
therefore 4× on both:

| | today (2048) | after (4096) |
|---|---|---|
| sun map | 16.8 MB | 67.1 MB |
| spot atlas | 16.8 MB | 67.1 MB |
| **total** | **33.6 MB** | **134.2 MB** |

**+100.7 MB.** Named here rather than discovered at a gate. The spot half of that
spend is not waste — it doubles indoor tile resolution to 2048 × 4096 for free — but
it is real, and it is Jean's to accept or to claw back via the tuning ladder's radius
step.

**Why the constant is not split.** "One fact, one home" would seem to argue for
separate sun and spot sizes. It argues the other way: the sun map *is* the spot
atlas's first texture, so a split would give the two halves of one atlas different
tile widths and force per-branch texel arithmetic in `sample_spot_shadow_pcf`. One
size is genuinely one fact here. **Not split. Reported per P7.**

**The C++/WGSL twin is not unified either, and cannot be cheaply.** `world.wgsl` is
read from disk at runtime (`renderer.hpp`, the shader loader) — there is no
compile-time bridge across that seam, exactly as the L3 MIRROR comment at
`state.hpp` says. Unifying would need either a WGSL `override` plumbed through every
pipeline that reads the constant (and overrides cannot initialise the `const`s that
depend on it), or source-patching the shader text at load. Both are structural
changes UMBRA_5 did not name. **The minimal form is taken: both rooms edited in the
same commit, as L3 requires.** Filed as a horizon item below.

**The highest-risk single deletion in the campaign** is `shadow_pos.y += 0.3` in
`shadow_pawn_vs` (UMBRA_6). It is the pawn's only depth separation from terrain in
the **spot** path — and the spot path is deliberately normal-offset-free
("No normal offset — it breaks contact shadows"), so UMBRA_7 does not backfill it
there. Outdoors UMBRA_7 covers it; indoors, rasterizer slope bias covers it alone.
Flagged at the top of the tuning ladder.

> **Closed by PENUMBRA_1 P5.** The spot path now has its own normal offset, derived
> per fragment because a perspective frustum has no constant texel size. The quoted
> "No normal offset" ruling was true of a *depth* lift, which is what that path had
> when it was written; a normal offset moves the sample position instead. The comment
> it quotes no longer exists.

---

# CAMPAIGN LEDGER

| Handoff | Commit | State | Notes |
|---|---|---|---|
| UMBRA_1 | `umbra: recon report` | **landed** | This document. No STOP fired. Sun VP is WGSL-side, not CPU-side — UMBRA_2's site corrected, UMBRA_7's growth risk dissolved. |
| UMBRA_2 | `umbra: freeze and snap sun frustum` | **landed** | Freeze already true (named constants, no refit) — nothing to delete. Snap **replaced**, not added: world-XZ → light-space texels. `SHADOW_SNAP_SIZE` deleted, `SHADOW_TEXEL_WORLD` introduced. No animated-sun caveat (R3). |
| UMBRA_3 | `umbra: pin shadow caster LOD; curtains out of sun casters` | **dead by prior work** | Both halves already landed by ECONOMY_1 E2, proven at the vertex-index level. No boolean added — the exclusion is structural, not conditional. Ladder + arithmetic committed to the tree at `draw_shadow_all`, per the handoff's STOP-and-report. Label corrections only. |
| UMBRA_4 | `umbra: terrain out of spot caster lists` | **landed** | One `cast_terrain` argument, two call sites. Up to 4 full terrain redraws per frame removed indoors. Spot lists keep the ten drawable-table rows. |
| UMBRA_5 | `umbra: sun map RES→2x per side, radius 1.4x` | **landed** | 2048→4096 (at the cap), 300→420. Both twin rooms, one commit. +100.7 MB VRAM, named in advance. Constant deliberately **not** split — the sun map *is* the spot atlas's first texture. |
| UMBRA_6 | `umbra: bias to rasterizer state; shader nudges deleted` | **landed, with four flagged risks** | One depth-stencil site reaches all 11 shadow pipelines. Float-format flag raised with arithmetic: `depthBias = 2` = 6.0e-8 NDC vs the 1.0e-4…2.0e-3 it replaces. No `CullMode::Front` anywhere — step 3 a no-op. The P3 refuters caught the most in this commit: see findings 1, 2 and 6 below. |
| UMBRA_7 | `umbra: normal-offset receiver sampling` | **landed** | `TEXEL_WORLD` as a const-expression — no uniform, no growth, no STOP, and better than the handoff's preferred rung. Receiver normal was already a parameter at the site; verified unit at every caller. |
| UMBRA_8 | `umbra: 3x3 PCF + edge fade` | **landed** | Comparison sampler already correct — nothing to retype. Nine unrolled `textureSampleCompareLevel` taps at const offsets. Fixes the half-texel kernel offset found at R6. Edge fade is insurance at today's radius (see below). |
| — | `umbra: campaign close` | **landed** | This ledger, plus three P5 corrections the campaign's own refuters found — two of them in the campaign's own output. |

## WHAT THE ADVERSARIAL PASSES CHANGED

The P3 pass on UMBRA_6 and the recon refuters were not ceremony. Five findings
survived verification, three of them against work this campaign had just written.

1. **A stale number in my own comment.** The UMBRA_6 comment quoted the deleted
   constants as `2e-4..4e-3` — their values at RES=2048, before UMBRA_5 doubled it.
   At 4096 they are `1.0e-4..2.0e-3`, exactly half. This is precisely the failure mode
   the campaign exists to fix (a label restating a number that moved), committed *by*
   the campaign. Caught before it landed.

2. **The caster/receiver tessellation gap.** The shadow pass draws terrain at LOD1
   while the main pass draws near terrain at LOD0 — a chord over 1.5625 wu against a
   surface sampled at 0.78125. In a concave dip the chord rides above the true
   surface and the receiver reads as self-shadowed. **Slope-scale cannot compensate
   this**: it corrects a primitive's own gradient, not a difference between two
   meshes, and the error peaks exactly where slope, and therefore the slope term,
   goes to zero. The deleted `SHADOW_BIAS_MIN` was the only term covering it. Named
   at the site, with its own ladder rung.

3. **An over-claim in UMBRA_3's ruling.** I wrote that nothing in `draw_shadow_all`
   reads eye distance, so caster silhouettes cannot change as the camera approaches.
   True of **density**; false of the **set**. The instance counts come from
   `band_patches`, which partitions against `lod0_radius` and the veil ring measured
   from THE POINT — and in camera-host mode the point is the eye. Corrected in the
   tree: *which* patches cast tracks the viewer, even though *how finely* they cast
   does not. The acquittal stands, but only at the density level.

4. **Binding-registry drift.** `binding_registry.hpp` claimed "95 declarations over 92
   slots"; the tree has **96 over 93**. Verified pre-existing (the same counts hold at
   `0466346`), so not campaign-caused — but it is the file that calls itself the single
   source of truth for binding numbers, and the count does registry work. Corrected
   under P5, not de-numbered.

5. **A stale tap-count label**, `"Shadow Sampling with 4x4 PCF"`, left standing by
   UMBRA_8 over the kernel it had just made 3×3. Corrected, and scoped — the *spot*
   kernel below it is still 4×4 and its label is right.

6. **The bias arithmetic was computed at the wrong depth**, and the tuning ladder's
   remedy for the campaign's own highest-risk deletion **pointed the wrong way.**
   Both are corrected; both mattered.

   *The depth:* `depthBias` on a float format is a ULP multiple of the primitive's
   max depth, so the exponent decides the answer. I computed it at z ≈ 1. This
   scene's sun-map geometry sits near **z = 0.417** — the light is `SUN_ALTITUDE` =
   250 above the ground in a 599.9-deep frustum. One ULP there is 2^-25 = 2.98e-8,
   not 2^-23, so `depthBias = 2` buys **6.0e-8 NDC, not 2.4e-7**. The dial's
   landmarks move with it: restoring the old floor is **~3355**, not ~840.

   *The direction:* bias pushes the **stored caster** depth away from the light, so
   more bias means more lit means a **weaker** shadow. The ladder told Jean to
   *raise* `depthBiasSlopeScale` when the pawn's shadow detaches indoors — which
   would have made detachment worse, on the exact symptom the campaign flagged as
   its most likely regression. Detachment is *too much* bias. Corrected, and the
   ladder now opens with the direction table rather than assuming it.

   *And the equivalence was scoped.* "Slope-scale replaces the old mix()" is true in
   the mid-range and false at both ends. Measured, new/old by incidence:
   0° **0.00** · 30° 1.11 · 45° **1.04** · 60° 1.13 · 80° 2.32 · 85° 4.26 · 88°
   **10.13** · → ∞. It is *zero* where the old term had a floor, and *unbounded*
   where the old term had a ceiling — because `depthBiasClamp = 0.0` means **no
   clamp**, not *clamp at zero*. Quoting the 45° agreement as general would have been
   the campaign's own failure mode a third time. Both ends are now named at the site,
   with the `CullMode::None` thin-sheet case that the unbounded end can actually
   break.

## THE RESULT UMBRA_5 ACTUALLY BOUGHT, which the handoff did not predict

The veil ring — the draw authority, past which no terrain is drawn at all — is
`6.5 × PATCH_EXTENT` = **325 wu**.

- **Before:** sun half-extent 300 wu. The shadow map ran out **25 wu inside the drawn
  world**. There was a ring of visible terrain that received no sun shadow, and its
  edge was a hard line. That is the "composing at the far edge" artifact, and it was
  not a subtlety — it was geometric.
- **After:** 420 wu. Coverage now extends **95 wu past** the last drawn thing.

The far-edge artifact is **structurally eliminated, not merely pushed out**. Radius was
the whole of it, and 1.4× happened to be more than enough.

One honest consequence: **UMBRA_8's edge fade is insurance at these numbers, not an
active effect.** Its band begins at `0.88 × 420` = 369.6 wu, well beyond the 325 wu
ring, so it never touches drawn ground. It becomes live only if the tuning ladder
claws radius back below 369.3 — the first −10% step (→378) is still clear, the second
(→340) is where it starts working. Recorded at the site so nobody tunes against an
effect that is currently dormant, and kept because it is what makes that ladder step
safe.

## R11, SECOND PASS — four stale labels the first pass missed

The full recon fan-out landed after the campaign did, and its refuters found four
label defects in the shadow chain that my own R11 pass had not caught. All four are
live at HEAD, all four are corrected. Recording the miss as well as the fix, because
"the report found nothing there" and "the report did not look" are different claims
and only one of them was true.

1. **`world.wgsl`, `compute_vp`** — *"the 300-unit shadow box must cover what the eye
   sees"*. I quoted this comment in R2 and called it correct. It **was** correct about
   the centering, and it carried a number that UMBRA_5 then falsified three commits
   later. De-numbered (P5: it merely restates `SUN_HALF_EXTENT`).

2. **`state.hpp`, the spot atlas descriptor** — *"2×2 tiled for up to 4 spot lights"*.
   **False.** The tiling is **1×2 per texture across two textures**: lights 0–1 in the
   sun map, 2–3 here, each a half-width full-height tile. Three witnesses —
   `use_sun_map = (li < 2)`, `within = li % 2` driving a half-width viewport, and the
   `light_index < 2u` branch in the shader. R1 got the *shape* right by reading the
   pass; it failed to then turn round and convict the label. The 2×2 grid is the
   retired scheme, which the WGSL banner correctly calls "the old single-texture 2×2
   grid" — so the tree contained its own refutation the whole time.

3. **`state.hpp`, the sun map descriptor** — *"directional light depth"*. Incomplete
   in a way that matters: this texture is also the spot atlas's **first** texture
   indoors. A reader pruning "directional" work could delete half the spot atlas.
   Widened.

4. **`state.hpp`, the ECONOMY_1 E1 flag pair** — *"Every LOD0 carrier (indirect reset,
   indoor direct, shadow band 0, snapshot) draws through these two"*. **Three of the
   four are false.** The only remaining caller of `patch_index_buffer_lod0_live()` is
   the snapshot pass; `reset_frustum_indirect` writes the two counts as separate plan
   slots, and the shadow pass draws both bands through the LOD1 buffer — so "shadow
   band 0" never read these at all. Not campaign-caused: it is drift from ECONOMY_1's
   own closing arm, and `render_main_pass` already documents the truth. Two rooms,
   one fact, and they had disagreed since before UMBRA opened.

## TWO CONSEQUENCES RECORDED HERE — BOTH SINCE RESOLVED BY PENUMBRA_1

- **The pawn aura now moves the shadow sample, not just the shading.** The terrain FS
  perturbs its normal by up to ~17° where the aura is active
  (`normal = normalize(normal + vec3(0.0, aura.r * 0.3, 0.0))`), and after UMBRA_7
  that normal steers the offset as well as the lighting. The effect is sub-texel and
  arguably correct — a normal that claims the ground is raised should sample as if it
  were — but it is a coupling that did not exist before this campaign, and it is not
  in the handoff. Named so it is not rediscovered as a mystery.

  > **Closed by PENUMBRA_1 P4**, and the reasoning above was too generous. The aura
  > lookup snaps to an exact texel centre, so its bilinear filter degenerates to
  > nearest and the value *steps* at every cell boundary. It was not a smooth
  > "arguably correct" nudge; it was the one per-cell discontinuity in the whole
  > terrain normal chain, and it was steering the shadow sample. The shadow path now
  > reads the pre-aura geometric normal.

- **`sample_spot_shadow_pcf`'s depth clamp is now dead code.** `clamped_depth =
  clamp(current_depth, 0.0, 1.0)` was there to contain the bias UMBRA_6 deleted; with
  raw `light_ndc.z` going in, and `out_of_bounds` already rejecting outside [0,1], it
  is the identity on every surviving fragment. Residue of my own edit. Left standing
  rather than cut, because removing it is shader logic outside any named handoff and
  it costs nothing — but it is residue, and it should go with the next edit that opens
  that function.

  > **Wrong, and PENUMBRA_1 P6 was written to act on it before the error surfaced.**
  > The clamp is not residue. It is the manual clamp a *floating-point* depth
  > resource requires — `SampleCmp` does not auto-clamp the reference on
  > `Depth32Float`, only on unorm formats — and it is the only NaN scrubber on the
  > path, because every ordered comparison against NaN is false and `out_of_bounds`
  > therefore *passes* NaN through. Without the clamp a NaN fragment reads fully
  > black instead of fully lit. The word "surviving" in the sentence above is exactly
  > where the reasoning failed. P6 kept it and documented it.

**One refuter claim checked and rejected:** that the spot kernel's horizontal tap
spacing is 2 tile-texels against 1 vertical, because it scales offsets by
`1.0 / SHADOW_MAP_SIZE` while sampling a half-width tile. It does not. The tile is a
*sub-rectangle* of the texture, not a scaled copy, so a texel there is a texel; a step
of `1/SHADOW_MAP_SIZE` in `uv.x` is exactly one texel inside the tile. The kernel is
correct as written and was not touched.

# HORIZON

**The `SHADOW_MAP_SIZE` twin.** Two rooms, no compile-time bridge, held only by L3
and a comment. Unification needs a WGSL `override` plumbed through every pipeline
that reads it — and overrides cannot initialise dependent `const`s, so
`SHADOW_TEXEL_WORLD` would have to become an override too, and so on up the chain.
Deferred with this campaign as its dated owner (2026-07-30); it rides the next
campaign that opens the pipeline-creation path, rather than paying for it twice.

**The FXC ownership loop.** `world.wgsl`'s banner and `LAWS.md` L2 each name the
other as the home of the FXC specifics. The specifics are in L2. One of the two
sentences should be cut; `LAWS.md` is outside this campaign's scope.

**The rim, still flagged and still not chased.** `shadow_patch_terrain_vs` carries
a standing note that the depth-only shadow pass has no equivalent of the visible
rim's per-fragment discard, so terrain casts ~one patch beyond the smooth visible
rim. UMBRA_8's edge fade attenuates the *sampling* edge, not this *casting* edge.
Untouched, deliberately.

# TUNING LADDER (Jean's dial, at the visual gate)

*Rewritten at PENUMBRA_1 P7 against the instruments that now exist. The previous
version sent Jean to `depthBias` three times — a field P2 deleted — and told him to
raise bias to cure detachment, which deepens it. Both were caught by adversarial
passes, not by review.*

**Read the direction first.** Depth bias pushes the **stored caster** depth *away
from the light*. More bias → more lit → **weaker** shadow. So:

| symptom | meaning | direction |
|---|---|---|
| acne, self-shadow stippling | too little bias | bias **UP** |
| peter-panning, shadow off contact | too much bias | bias **DOWN** |

**The levers no longer live at one site.** Two rooms now, and knowing which is which
is half the diagnosis:

- `renderer.hpp`, `shadowDepth` in the shared shadow builder — **depth** bias
  (`depthBiasSlopeScale`, `depthBiasClamp`). One edit reaches all eleven shadow
  pipelines, sun and spot alike.
- `world.wgsl` — **the normal offset** and **the filter footprint**
  (`PCF_RADIUS_TEXELS`, `SPOT_PCF_RADIUS_TEXELS`, `TEXEL_UV`, and the `0.33`/`0.67`
  blend in each sampler). Sun and spot have separate footprint constants on purpose.

**`depthBias` is not on this ladder and must not be put back.** Under `Depth32Float`
it is a ULP multiple of the primitive's max depth; at this scene's z ≈ 0.417 a value
of 2 bought 6.0e-8 NDC, ~3,355× short of the floor it was nominally covering. It is
not a dial in steps of one. P2 deleted it.

| symptom | instrument | direction |
|---|---|---|
| acne on slopes | `depthBiasSlopeScale` | **up**, +0.5 per step |
| acne on flat sun-facing ground | the offset **floor** (the `0.33` term) | **up**, +0.15 per step |
| shadow off contact at the pawn's feet | the offset **ceiling** (the `0.67` term) | **down** |
| shadow detaches on thin sheets only | `depthBiasClamp` | **down** from 2.8e-3 |
| penumbra too hard, caster steps visible | **tap count** (`PCF_RADIUS_TEXELS` follows) | up — see below |
| penumbra too soft near the camera | `SUN_HALF_EXTENT` | **down** — see the reserve below |
| indoor pawn shadow off its feet | the **spot** offset (same `0.33`/`0.67`, in `sample_spot_shadow_pcf`) | **down** |

Two rungs need their reasons, because both look like simple knobs and are not.

**Spacing is not a dial at all — it is pinned at 1, and width comes from tap count.**
PENUMBRA_1 P3 argued the opposite here (taps at −2/0/+2 "cover [−3,−1] [−1,1] [1,3]
contiguously") and shipped visible banding for it. A bilinear comparison tap is a
**tent, not a box**: weight 1 at its centre falling linearly to zero at ±1 texel. Tents
at spacing 2 land on each other's zeros — summed weight `1 0 1 0 1` at successive
texels, half the map never read, a comb of period 2 texels. Touching supports is not
coverage; the condition is on the weights. **PENUMBRA_2 N1 reverted it.**

Width therefore comes from tap count alone, at spacing 1. **Quote the VISIBLE column at
a gate** — support is which texels get read, but an edge sweeps the response 0→1 over
the tap-centre span plus one texel of ramp:

| kernel | taps | support | **visible penumbra** |
|---|---|---|---|
| UMBRA_8 3×3 | 9 | 0.820 wu | 0.615 wu |
| P3 3×3 spacing 2 | 9 | 1.230 wu | 1.025 wu *(banded — withdrawn)* |
| **N1 4×4 (current)** | **16** | **1.025 wu** | **0.820 wu** |
| N4 5×5 *(held)* | 25 | 1.230 wu | 1.025 wu |
| pre-campaign 4×4 @2048 | 16 | 1.465 wu | 1.171 wu |

The next rung up is N4 — 25 taps — and it lands only once a measurement asks.

**The crispness reserve — a control-panel fact, not a tuning tip.**
`SUN_HALF_EXTENT` is a single WGSL const from which texel world-size, both normal
offsets' magnitude, the frustum snap lattice and the edge-fade radius all derive.
Lowering it buys sharpness everywhere at once. There are two regimes:

| range | what happens | cost |
|---|---|---|
| **420 → 369 wu** | up to ~12% finer texels | none — the fade stays dormant and no shadow horizon is ever visible |
| **369 → 325 wu** | up to ~23% finer texels | the fade goes live and softens a horizon that now falls inside the 325 wu veil ring |

**Therefore UMBRA_8's edge fade is not dormant-and-deletable.** It is precisely what
makes the reserve *spendable* below 369 wu. Recorded here because a reader who
measured it at today's radius would find it never firing and conclude it was dead
code. It is a purchased option, not residue.

# CONTROL-PANEL RECORD

**One dial, two jobs — the pattern this campaign family exists to break, still
standing.** `Dim::SHADOW_MAP_SIZE` is a single fact serving two roles: the sun map's
resolution *and* the spot atlas's. UMBRA_5 paid +100.7 MB of VRAM to double it, and
half that spend went to the spot atlas as an unavoidable side effect. Splitting the
constant would let the sun go finer for less VRAM than UMBRA_5 paid.

It is not split, and the reason is the reason it is hard: the sun map **is** the spot
atlas's first texture during indoor moods, so a split gives the two halves of one
atlas different tile widths and forces per-branch texel arithmetic in
`sample_spot_shadow_pcf`. **HORIZON, dated 2026-07-31, not chased here.**

**The spot tile is 1:2 and its projection is 1:1.** `compute_spot_light_vp` has no
aspect term (`proj[0] == proj[5] == f`) while the tile is 2048 × 4096, so spot shadow
texels are non-square by exactly 2×. P5 works around it by taking the coarser axis;
the projection itself is untouched. **HORIZON, dated 2026-07-31.**

---
---

# PENUMBRA_1 — RECON

Read 2026-07-31 against `32a2ccd`. Five gated questions. Same register as UMBRA_1:
descriptors and call sites, never labels. Where an answer contradicts the handoff's
premise, the contradiction is the finding and it is stated first.

An adversarial fan-out was launched over the same five questions and had not returned
when this section was written; its findings will land as an amendment, the way UMBRA's
R11 second pass did. Every claim below is from a direct read of the source, cited.

## P1-A — `depthBiasClamp` legality → **P2 IS CLEARED TO LAND**

**No compatibility mode is requested anywhere in the tree.** Adapters come from
`instance_->EnumerateAdapters()` (`console.hpp`) called with **no
`RequestAdapterOptions` at all** — so no `featureLevel`, no `compatibilityMode`, no
`FeatureLevel::Compatibility`. Dawn's native default is core.

The device asks for exactly one feature, conditionally:

```cpp
wgpu::FeatureName requiredFeatures[1] = { wgpu::FeatureName::TimestampQuery };
if (adapter.HasFeature(wgpu::FeatureName::TimestampQuery)) {
    deviceDesc.requiredFeatures = requiredFeatures;
    deviceDesc.requiredFeatureCount = 1;
}
```

`core-features-and-limits` is **never named** in the repo. Under the current WebGPU
spec that is the expected shape: a non-compat device carries it implicitly; it is
something you *check*, not something you request. Limits are requested at full adapter
capacity (`deviceDesc.requiredLimits = &adapterLimits`).

**P1 (ASSERT-AND-GUARD) discipline on this answer.** What the repo *requests* is a repo
fact and is settled above. What the device *grants* is a runtime fact beyond the repo
boundary, and this report does not assert it. **The guard already exists and needs no
new code** — `console.hpp` prints the adapter's full enumerated feature list at boot:

```cpp
std::cout << "[Console] Adapter features (" << feats.featureCount << "):";
```

So the hypothesis is falsifiable from any boot log Jean already has. **P2 lands**; if a
boot ever shows a compat adapter, `depthBiasClamp` is the first thing to revert and the
fallback is capping `depthBiasSlopeScale` instead, which is Jean's ruling.

## P1-B — terrain normal derivation → **the handoff's checker mechanism is REFUTED**

**The shading normal is continuous, not per-cell and not faceted.** One line builds it,
in the terrain FS:

```wgsl
    var normal = normalize(vec3(-in.gradients.x, 1.0, -in.gradients.y));
```

`gradients` is `@location(1) gradients: vec2<f32>` — **no `@interpolate(flat)`**, so it
is smoothly interpolated across every triangle. The VS writes it as

```wgsl
    out.gradients = height_data.yz + live.yz;
```

where `height_data` is a **bilinear-filtered** heightfield fetch
(`textureSampleLevel(patch_heightfield_array_read, bilinear_sampler, …)`) and `live.yz`
is the live card's full-Δ gradient. Both terms are continuous. Nothing per-cell reaches
the normal: `cell_local` *is* `@interpolate(flat)`, but it feeds cell colour, not
shading normals.

**So "per-cell flat normal ⇒ per-cell offset jump" is not the mechanism.** Per the
handoff's own checker discriminator, that pushes the ruling to Jean's glance — and if
the checker is equally strong on open lit ground far from any shadow, it is the
cell-colour aesthetic and never was ours.

**But a different discontinuity is real, and it is the one that matters here.** The cell
lift moves **position** and contributes **nothing** to the gradient:

```wgsl
    let lift = ug_cell_lift(pi.origin, pi.extent, d.cellx, d.cellz)
             * (1.0 - pawn_gol_suppression(world_pos.xz, render_pawn_pos().xz));
    world_pos.y += lift * d.lift_scale - d.drop;
```
```wgsl
    out.gradients = height_data.yz + live.yz;     // no lift term
```

A lifted GoL cell is a slab whose top face **and vertical curtain walls** carry the
*unlifted* terrain's normal. On a curtain wall the shading normal points **up** while
the surface faces sideways. Post-UMBRA_7 that normal also steers the shadow sample, so
the offset on a curtain wall pushes along a normal that is geometrically wrong by ~90°.

This is a position/normal mismatch, not a faceted-normal one — a different fault from
the one the handoff hypothesised, at the same site. **P4 does not fix it** (P4 is about
the aura). Filed as a HORIZON item below; no edit is authorised for it here.

## P1-C — sun sampler bounds rejection → **yes, and the clip/NDC distinction is moot**

`sample_shadow_pcf` rejects out-of-range depth, testing the post-divide value:

```wgsl
    let out_of_bounds = shadow_uv.x < 0.0 || shadow_uv.x > 1.0 ||
                        shadow_uv.y < 0.0 || shadow_uv.y > 1.0 ||
                        light_ndc.z < 0.0 || light_ndc.z > 1.0;
```

**For the sun the divide is the identity.** Every column of the ortho carries `.w = 0`
except the last, which carries `1.0`:

```wgsl
        vec4(1.0 / he,  0.0,       0.0,                   0.0),
        vec4(0.0,        1.0 / he,  0.0,                   0.0),
        vec4(0.0,        0.0,      -r_depth,               0.0),
        vec4(0.0,        0.0,      -SUN_NEAR * r_depth,    1.0)
```

so `light_clip.w == 1.0` identically and `light_ndc.z == light_clip.z`. Testing NDC z is
testing clip z. **The second sphere-square hypothesis — a spuriously in-range sample —
cannot occur on the sun path.** That leaves P1-E's mechanism as the live one.

**Two findings the question asked for.**

1. **The two samplers disagree on what out-of-bounds means.** Sun returns `1.0` —
   *fully lit*: `return select(lit, 1.0, out_of_bounds);`. Spot returns `0.0` — *fully
   shadowed*: `return select(shadow / 16.0, 0.0, out_of_bounds);`. Opposite conventions
   for the same condition, in the same file, ten lines of scroll apart. The spot's
   choice is masked in practice because `cone_falloff` multiplies its result to ~0
   outside the cone, but it is a genuine inconsistency and nothing documents it.

2. **The spot path has a latent sign hazard the sun path cannot have.** Its projection
   row 3 is `(0, 0, -1, 0)`, so `light_clip.w = −view_z`: for a fragment *behind* the
   light `w < 0`, the divide flips every sign, and an out-of-frustum fragment can land
   inside [0,1] and be sampled. Masked by the same `cone_falloff`. Reported, not fixed —
   no handoff authorises it.

## P1-D — spot projection aspect → **uncompensated; spot texels are non-square by exactly 2×**

`compute_spot_light_vp` builds a **perspective** projection with **no aspect term at
all**. Verbatim:

```cpp
    const float outer_half = std::acos(std::max(light.outer_cone, -0.95f));
    const float fov = std::min(2.0f * outer_half + 0.2f, 2.8f);
    const float near_plane = 1.0f;
    const float far_plane = light.range + 5.0f;
    float f = 1.0f / std::tan(fov * 0.5f);
    float nf = 1.0f / (near_plane - far_plane);

    float proj[16] = {
        f, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, far_plane * nf, -1.0f,
        0.0f, 0.0f, far_plane * near_plane * nf, 0.0f
    };
```

`proj[0] == proj[5] == f`. Equal angular half-extent on both axes — rendered into a
tile of `TILE_W = SHADOW_MAP_SIZE / 2 = 2048` by `TILE_H = SHADOW_MAP_SIZE = 4096`.

**So the same angle maps onto 2048 texels horizontally and 4096 vertically: a spot
shadow texel is exactly 2× wider in angle than it is tall.** A 1:2 tile with a 1:1
projection. This is a finding in its own right, as the handoff anticipated; it is
reported and **not** fixed here.

**What P5 needs, and the problem with how the handoff asked for it.** The handoff wants
`SPOT_TAN_HALF_FOV` and `SPOT_TILE_TEXELS` taken "from P1-D verbatim; do not invent
them." Verbatim, they do not exist:

- The FOV is **per-light**, derived on the CPU from `light.outer_cone`, and is **never
  uploaded** — the shader receives only the finished `view_proj` matrix. There is no
  constant to quote.
- The tile size is C++-side (`TILE_W`/`TILE_H` are `static constexpr` locals inside
  `render_shadow_pass`), but both derive from `Dim::SHADOW_MAP_SIZE`, whose WGSL twin
  `SHADOW_MAP_SIZE` is already in the shader.

Rather than invent a constant or mirror the CPU formula into WGSL (a new L3 mirror, and
exactly the two-rooms-one-fact pattern this campaign family exists to break), both fall
out of data the shader already holds:

- `f = length(vec3(m[0][0], m[1][0], m[2][0]))` where `m = light.view_proj` — the first
  matrix *row* is `f × right`, and `right` is unit, so its length is `f`. Then
  `tan(halfFOV) = 1 / f`.
- axial distance to the fragment is `light_clip.w` directly, which is what the
  handoff's formula calls `light_dist`.
- tile texels: `SHADOW_MAP_SIZE * 0.5` on x, `SHADOW_MAP_SIZE` on y.

**P5 will therefore use the X axis** — the coarser one, 2048 texels — which is the
handoff's own instruction for the uncompensated-aspect case ("land the offset using the
**larger** of the two texel world-sizes"). Recorded here under P7 of the process laws as
a structural choice taken inside a named edit: the minimal form, reported.

## P1-E — terrain rim → **the sphere-square's first hypothesis is quantitatively supported**

**The visible rim is smooth and round.** `shade_lit`:

```wgsl
    let point_d = distance(world_pos.xz, render_point_pos().xz);
    let veil = smoothstep(config.veil_ring - config.veil_icing, config.veil_ring, point_d)
             * config.veil_strength * veil_scale;
    if (config.veil_dither > 0.5) {
        if (veil_dither_noise(world_pos.xz) < veil) { discard; }
        return fogged;
    }
    return mix(fogged, config.fog_color, veil);
```

Radius source: `config.veil_ring`, defaulted from `Dim::VEIL_RING_DEFAULT = 6.5f *
PATCH_EXTENT` = **325 wu**, with `Dim::VEIL_ICING_DEFAULT = 40.0f` — a 40 wu fade band
from 285 to 325 — measured from **THE POINT**, and `veil_strength` is 0 in
finite/indoor.

**The shadow caster boundary is square-cornered and larger.** `shadow_patch_terrain_vs`
has **no rim path at all**, and cannot: the shadow pass is depth-only, `desc.fragment =
nullptr`, so there is no fragment stage in which to `discard`. The tree already says so
and is correct.

What selects the caster set is `band_patches`:

```cpp
        float d2 = patch_distance_sq(point_wx, point_wz, ox, oz, half);
        if (c->world_state_.finite_mode || d2 <= ring_sq) {
```

and `patch_distance_sq` is a **point-to-AABB** distance, not point-to-centre:

```cpp
    float dx = std::max(0.0f, std::abs(px - origin_x) - half);
    float dz = std::max(0.0f, std::abs(pz - origin_z) - half);
    return dx * dx + dz * dz;
```

A patch is therefore included when its **nearest edge** is within `veil_ring`. With
`Dim::PATCH_EXTENT = 50.0f`, cast geometry extends up to roughly **one patch beyond the
visible rim**, along a boundary that is **quantised to the 50 wu patch grid** — square
corners, not a circle. And `point_wx/point_wz` are `c->point_`, THE POINT, so the whole
boundary **translates with the viewer** (and in camera-host mode the point is the eye).

`SUN_HALF_EXTENT` is 420 wu, so that entire caster set sits **inside** the shadow
frustum and all of it casts.

| boundary | shape | radius from THE POINT |
|---|---|---|
| visible terrain (rim) | smooth circle, 40 wu fade | 325 wu |
| shadow-casting terrain | **square-cornered, 50 wu granular** | ~325 → ~375 wu |
| sun shadow frustum | square, snapped to texels | 420 wu |

**Conclusion.** There is a band of terrain, up to ~50 wu wide, that is invisible and
still casts, bounded by right angles, tracking the viewer. That is precisely a
square-edged shadow of a caster with no visible geometry. It makes the handoff's *first*
sphere-square hypothesis the strongly favoured one before Jean's discriminator is even
run — and P1-C has independently removed the *second* (a spuriously in-range sample
cannot happen under an orthographic w ≡ 1). **No edit here; this campaign does not
authorise one.**

## P1 STOP CONDITIONS — none fired

| condition | status |
|---|---|
| named anchor absent / different count | **Clear**, with two premise corrections reported above: P1-B's faceted-normal mechanism does not exist, and P1-D's `SPOT_TAN_HALF_FOV` / `SPOT_TILE_TEXELS` do not exist as constants. |
| edit would grow a shared bind-group layout | **Clear.** Nothing in P2–P7 adds a binding. |
| edit conflicts with the FXC banner / L2 | **Clear.** Every P2–P7 shader edit is arithmetic or const-expression offsets. |
| `depthBiasClamp` illegal on this device | **Clear.** No compatibility mode is requested anywhere (P1-A). |

---

## PENUMBRA_1 — RECON AMENDMENT (adversarial pass)

The fan-out over P1's five questions returned after the campaign's edits had begun.
Most of what it reports as "wrong" is **baseline drift I caused**: I launched the
recon, said I would wait for it, and then began landing P2–P5 while its agents were
still reading `world.wgsl`. Their line numbers and verbatim quotes are correct
against `32a2ccd` and stale against HEAD. That is a process failure, not a finding,
and it is the *second* time this campaign family has made it — UMBRA's recon drifted
the same way. **The rule that would have prevented it: a report-only handoff may land
while a recon runs; an edit handoff may not.**

Three findings survive the drift and are recorded here.

**1. The obvious fix for the spot aspect is wrong, and would clip every cone.**
P1-D found the projection carries no aspect term against a 1:2 tile. The textbook
correction — `proj[0] = f / aspect` with `aspect = 2048/4096 = 0.5`, i.e. `proj[0] =
2f` — makes texels square by *halving the horizontal angular coverage*:

| authored fov | half-angle X today | X after the "fix" | coverage kept |
|---|---|---|---|
| 1.2 rad | 34.4° | 18.9° | 55% |
| 2.0 rad | 57.3° | 37.9° | 66% |
| 2.8 rad | 80.2° | 71.0° | 88% |

A spot cone is **circular**. A 1:2 tile cannot hold a circular cone with square
texels — square texels force a 1:2 angular frustum, which clips the cone in X. **The
tile shape is the defect, not the projection**, and the tile shape comes from the
atlas packing (two half-width tiles per texture). Anyone who acts on the aspect
HORIZON item by scaling `proj[0]` will lose light, not gain sharpness.

**2. `fov` is not a constant across lights.** It ranges roughly 1.2–2.8 rad depending
on the scheme, derived per light from a sampled `outer_cone`. P5's per-fragment
recovery of `f` from the matrix handles that correctly *because* it is per-light; a
mirrored constant would not have.

**3. P1-A's runtime settle is weaker than P1 claimed, and the correction already rode
in P2's commit.** The boot print is the **adapter's** feature list, not the device's
(`device_.GetFeatures` is absent tree-wide), and it prints **bare integers** with no
Dawn headers vendored to decode them. No recorded boot captures that line at all —
though `Adapter selected: index=2` *is* recorded, which means ≥3 adapters enumerate
on the shipping box, so the unfiltered first-max-wins pick is genuinely exercised.
The live guard is the uncaptured-error callback: a compat device rejects non-zero
`depthBiasClamp` at pipeline creation and fails **loudly**. P2 landed on that basis.

---

# PENUMBRA_1 — CAMPAIGN LEDGER

| handoff | commit | state | notes |
|---|---|---|---|
| P1 | `penumbra: P1 recon; PROCESS_LAWS P9` | **landed** | Two of five questions overturned the handoff's own premise (P1-B's faceted normal, P1-D's non-existent constants). PROCESS_LAWS gained P9 — fetch before you claim. |
| P2 | `penumbra: depthBiasClamp restores the SHADOW_BIAS_MAX ceiling` | **landed** | Ceiling restored at 2.8e-3, derived via texel-world ratio — which exposed that ECONOMY_1 E6's "free carry" tracked resolution only and would have been 1.40× short. `depthBias` deleted as inert. |
| P3 | `penumbra: PCF_SPACING drives tap offsets and normal-offset magnitude` | **landed, then REVERTED by PENUMBRA_2 N1** | The offset floor stands and was the durable half. The spacing-2 widening was wrong: bilinear comparison taps are tents, so spacing 2 combed at a 2-texel period and banded every shadow. |
| P4 | `penumbra: shadow offset uses geometric normal` | **landed** | Aura's shading fiction no longer steers the shadow sample. Stronger than the handoff knew: the aura lookup snaps to a texel centre, so it was a per-cell *step*, not a smooth nudge. Dormant by default. |
| P5 | `penumbra: normal offset on the spot path` | **landed** | `f` recovered from the matrix rather than mirrored; X axis taken per the uncompensated-aspect rule; `SPOT_PCF_RADIUS_TEXELS` kept separate from the sun's. |
| P6 | `penumbra: P6 — the dead clamp is not dead; R11 sweep instead` | **deletion DEAD; sweep landed** | The clamp is the manual clamp a float depth format requires *and* the only NaN scrubber (`out_of_bounds` passes NaN by construction). Kept and documented. The R11 sweep corrected ~15 labels, most created by this campaign. |
| P7 | `penumbra: tuning ladder rewritten against the new instruments` | **landed** | Four ladder rungs were actively wrong, one of them a 1.40× regression if followed. Levers now split by room. Crispness reserve and both HORIZON items recorded. |
| P8 | — | **HELD** | Releases only on Jean's word, after P2's visual gate. |

## WHAT THE ADVERSARIAL PASSES CAUGHT THIS TIME

Every one of these was in work this campaign had just written:

1. **P6's whole premise.** The deletion would have removed a documented format
   requirement and the path's only NaN scrubber, on the reasoning that it was our own
   residue. `P3 — REFUTER FOR DELETIONS` paid for itself twice over.
2. **Four wrong ladder rungs**, including one that told Jean to set `depthBiasClamp`
   to the exact value P2's derivation rejects as 1.40× short.
3. **~15 stale labels**, including `sample_pawn_aura`'s comment that P4's commit body
   convicted *in prose* and then left standing in the file — the campaign's named
   failure mode, committed by the campaign.
4. **`renderer.hpp` contradicting itself across 70 lines** of one comment block,
   describing `depthBias` as a live dial fifty lines above the paragraph explaining it
   had been deleted.

The pattern is consistent and worth naming: the errors are not in the code, which is
small and was checked. They are in the *prose about* the code, which is large, and
which no compiler reads.

### AMENDMENT A — P1-E was under-verified, and the correction inverts half of it

Two errors in the P1-E section above, both mine, both found by its refuter.

**1. The caster reach is 395.71 wu, not "~375".** The supremum is exact, not
approximate: maximising a patch's far corner subject to
`(|cx|−25)² + (|cz|−25)² ≤ 325²` gives `325 + 50√2 = 395.71` wu. I estimated "about
one patch beyond" as +50; the diagonal costs +50√2. That leaves only **24.29 wu** of
lateral margin against `SUN_HALF_EXTENT` = 420, not the comfortable 45 I implied.

**2. I tested the frustum's lateral extent and never its depth range — and the depth
range is the tighter bound in every mood.** The sun eye sits `SUN_ALTITUDE` = 250 wu
up-light of the centre and the ortho keeps only `[SUN_NEAR, SUN_FAR]` = [0.1, 600]
along the light axis. A caster's axial coordinate is `dot(d, P − C) + 250`, so the
horizontal component of the sun direction converts lateral distance into depth:

| mood | \|horizontal(d)\| | FAR clips up-sun beyond | NEAR clips down-sun beyond | binds at |
|---|---|---|---|---|
| `MOOD_FINITE_OUTDOOR` | 0.704 | 497.1 wu | **354.9 wu** | 354.9 |
| `MOOD_OPEN_SUNSET` | 0.966 | 362.4 wu | **258.7 wu** | 258.7 |

Both are below the 395.71 wu lateral reach. **The near plane, not the map's width, is
what actually ends the sun's caster set** — and `SUN_ALTITUDE` is what sets it.

**The consequence inverts half of my finding.** I reported one defect: an invisible
overhang that casts. There are two, and they point opposite ways:

- **Beyond the rim, inside the depth range** — terrain that is invisible and *does*
  cast, square-cornered at 50 wu granularity. As reported.
- **Inside the rim, outside the depth range** — at `MOOD_OPEN_SUNSET` everything more
  than ~259 wu down-sun of the point is clipped by the near plane while the visible
  rim runs to 325 wu. That is a band of **visible terrain that casts nothing at all**,
  ~66 wu wide, and it moves with the sun's azimuth.

For the sphere-square discriminator this matters: a shadow that *ends* along a
straight line partway across visible ground is the near-plane clip, not a caster
silhouette and not bias. Distinguish it from the overhang by which side of the point
it lies on — the clip is **down-sun**, the overhang is all around.

**Not fixed here.** The lever would be `SUN_ALTITUDE` (raise it to push the near plane
back) or `SUN_NEAR`/`SUN_FAR`, and no PENUMBRA handoff authorises touching the sun
frustum's depth. Raising `SUN_ALTITUDE` alone is not free either — it moves the whole
`[near, far]` window along the light axis and can clip the *far* side instead.
**HORIZON, dated 2026-07-31.**
