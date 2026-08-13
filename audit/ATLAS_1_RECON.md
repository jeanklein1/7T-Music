# ATLAS_1 — U0 RECON GATE (read-only)

The gate the handoff mandates before any edit, run whole. **It does not
clear.** One stop condition is met and one ruled design decision (D2)
collides with a writer the handoff's U0.5 expected to find in C++ and
that does not exist there. No `src/` file was touched by ATLAS_1.

DISCARD_0 landed first and is unaffected by everything below.

## Provenance

| field | value |
|---|---|
| HEAD at gate | `17a0faa99ccc5c538e422daca0faec3b3fcf307e` |
| DISCARD_0 | **landed** — `76bb00b` (U1, main depth), `17a0faa` (U2, snapshot depth) |
| handoff | `docs/HANDOFFS/TETRIS/ATLAS_1_HANDOFF.md` |
| cross-referenced | `audit/PASS_LEDGER.md` Q2.1 / Q3.1 / Q4.3, `audit/BINDING_LEDGER.md` Tables A/B |
| compile gate available here | **none** — `CMakeLists.txt` pins `DAWN_DIR` to a Windows prebuilt (`C:/dev/dawn`) and there is no `emcc`. No naga, no glaw1, no boot. |

## VERDICT TABLE

| step | result |
|---|---|
| U0.1 git hygiene, HEAD, DISCARD_0 status | **PASS** |
| U0.2 indoor loop matches PASS_0 Q2.1 verbatim | **PASS** |
| U0.3 outdoor sun VP reaches the shadow VSes through the slot | **PASS, with a qualification that turns out to be the whole story** |
| U0.4 WGSL reader census (expected ≥ 13) | **PASS — 19 references, 14 functional reads** |
| U0.5 C++ writers/readers of the slot and of `spotVPStagingBuffer_` | **STOP — expectation mismatch** |
| U0.6 `sample_spot_shadow_pcf` banner assumes nothing about pass structure | **PASS — and it supplies the resolution** |

---

## U0.2 — the indoor loop, verbatim

Matches PASS_0 Q2.1 exactly; nothing has moved since the census.

```
render_passes.hpp, render_shadow_pass, indoor arm:
    uint32_t within = li % 2;
    bool use_sun_map = (li < 2);
    depthAttachment.view = use_sun_map ? shadow_map_view() : spot_shadow_map_view();
    depthAttachment.depthLoadOp  = (within == 0) ? LoadOp::Clear : LoadOp::Load;
    depthAttachment.depthStoreOp = StoreOp::Store;
```

The per-tile copy of Q4.3 is present and unchanged:

```
    encoder.CopyBufferToBuffer(
        c->gpuState_.spot_vp_staging(), li * 64,
        c->gpuState_.vp_buffer(), GPUState::light_vp_offset(),
        GPUState::light_vp_size());
```

## U0.4 — the read-site census

19 textual references to `light_vp` in `world.wgsl`. Classified:

| class | count | sites |
|---|---|---|
| struct field declaration | 1 | `light_vp: mat4x4<f32>` in the VP struct |
| **shadow VS reads** | **13** | `out.clip_pos = render_vp.light_vp * …` — the 13 shadow vertex entry points PASS_0 named |
| **fragment read** | **1** | `sample_shadow_pcf` — `let light_clip = render_vp.light_vp * vec4(world_pos + offset_w, 1.0)` |
| **GPU writes** | **2** | `compute_vp`: `vp_data.light_vp = coupling_pawn_to_sun_vp(...)`; `compute_photographer_vp`: `photographer_vp.light_vp = coupling_pawn_to_sun_vp(...)` |
| comments | 2 | — |

**The count found is 14 functional reads, not 13.** The handoff
anticipated an overshoot and asked for the number found; the 14th is a
**fragment** read in `sample_shadow_pcf`, not a shadow VS, so it is not
covered by D3's `@builtin(instance_index)` mechanism at all. Under D3 it
would have to become `light_vps[0]` by hand.

## U0.5 — the writer census: **STOP**

### `spotVPStagingBuffer_` — clean, D4's precondition is satisfied

| toucher | file | role |
|---|---|---|
| `stage_spot_vps` | `state.hpp` | the only writer (`WriteBuffer`, 64 B × count) |
| the per-tile copy | `render_passes.hpp` | the only reader |
| `spot_vp_staging()` accessor | `state.hpp` | — |
| creation + null-check | `state.hpp` | — |

**No other reader exists.** D4's conditional ("only if U0.5 found no
other reader") is met: the buffer, its setter, and its registry constant
can be retired.

### The `light_vp` slot — the mismatch

U0.5 expected three writers: `stage_spot_vps`, the per-tile copy, and
**the outdoor sun write**. Enumerated at HEAD:

| toucher of `vpBuffer_` / the slot | kind |
|---|---|
| the per-tile `CopyBufferToBuffer` | GPU copy, `render_passes.hpp` |
| `light_vp_offset()` / `light_vp_size()` | accessors, `state.hpp` |
| `makeBuffer("VP Matrix", sizeof(GPUVPMatrix), …)` | creation |
| four bind-group entries at `sizeof(GPUVPMatrix)` | binding |

**There is no C++ writer of the sun VP. There is no CPU write to
`vpBuffer_` at all.** The outdoor sun write the handoff expected to
enumerate in C++ lives on the **GPU**: `compute_vp`, a
`@compute @workgroup_size(1)` kernel that runs every frame in
`dispatch_compute` (R10).

That is the U0.5 stop condition — "anything else is a STOP" — met not by
an extra writer but by the expected writer being somewhere the design
did not account for. It matters because of what follows.

---

## THE COLLISION — why D2 + D3 + D4 cannot ship as ruled

### The mechanism today

`light_vp` is a **single slot, time-multiplexed within the frame**:

1. R10 `dispatch_compute` → `compute_vp` writes the **sun** VP into
   `vp_data.light_vp`, gated only on
   `coupling_active(COUPLING_PAWN_TO_SUN_VP)`.
2. R18 indoor: before each tile, the per-tile copy **overwrites** the
   slot with spot light `li`'s VP; the tile draws; repeat.
3. R19 main pass: `calc_directional_light` **skips the sun PCF entirely**
   when spots are active. The shader says why, verbatim:

```
    // Shadow: skip when spot lights are active — light_vp is being used
    // for spot atlas tiles, so the directional PCF would sample wrong.
    var shadow = 1.0;
    if (render_lighting.spots.count == 0u) {
        shadow = sample_shadow_pcf(world_pos, geo_normal);
    }
```

So the per-tile copy is not only how each tile gets its matrix — **it is
also what protects light 0's tile from `compute_vp`**, by running after
it, every frame.

### What D4 does to that

(Naming note: D4 names `derive_indoor_lights` as the writer. That
function derives the lights; the VP computation and upload actually sit
in its caller, `apply_mood_spot_lights` — `direction/mood.hpp`. The
collision is the same wherever the write is spelled.)

D4 retires the per-tile copy. D3 puts the sun in `light_vps[0]` and spot
light `li` in `light_vps[li]` — so **spot light 0 and the sun are the
same slot**, with two owners at different cadences:

| owner | writes | cadence |
|---|---|---|
| `compute_vp` (GPU) | the sun VP | **every frame**, R10 |
| the CPU mood path (per D4) | spot light 0's VP | at mood change |

R10 precedes R18 in the spine. With the copy gone, every indoor frame
would leave the **sun's** VP in `light_vps[0]`, and light 0's tile would
render the scene from the sun's viewpoint into the left half of the sun
map. That is a correctness break, not a regression in degree.

### The coupling gate does not save it

`coupling_active` reads `config.mute_couplings`, which is `Coupling::NONE`
at init and is touched only by the debug freeze/release pair. **No
mood, arm, or indoor path mutes bit 16 anywhere in the tree.** The sun-VP
write is live in both arms.

### Scope note

D3 also does not cover the 14th read site. `sample_shadow_pcf` is a
fragment function with no `instance_index`; under D2 it must be
hand-pinned to `light_vps[0]`, which is the colliding slot.

---

## U0.6 — the banner is clean, and it hands over the resolution

`sample_spot_shadow_pcf` assumes nothing about pass structure. Its only
tiling assumption is the one D5 preserves:

```
    // NDC to UV (flip Y), then scale + offset into atlas tile.
    // Each texture holds 2 tiles side by side (left/right halves).
    // Tile = half width (0.5), full height (1.0).
```

The banner's own arithmetic (`SHADOW_MAP_SIZE * 0.5` for the X texel
count, the non-square 2× note, the `spot_f` invariant) is per-light and
per-fragment and is indifferent to how many passes wrote the atlas.

**But its first line is the finding:**

```
fn sample_spot_shadow_pcf(world_pos: vec3<f32>, geo_normal: vec3<f32>, light_index: u32) -> f32 {
    let light = render_lighting.spots.lights[light_index];
    ...
    let light_clip = light.view_proj * vec4(world_pos + offset_w, 1.0);
```

**Every spot light's shadow VP is already resident on the GPU, per
light, in a 4-element array, and the fragment PCF already indexes it by
light index.** Confirmed on the C++ side:

```
struct alignas(16) GPUSpotLight {
    ...
    float view_proj[16];   // perspective shadow VP for this light
};
static_assert(sizeof(GPUSpotLight) == 128, ...);

struct alignas(16) GPULighting {
    GPUDirectionalLight sun;      // offset   0, 48 B
    GPUPointLightArray  points;   // offset  48, 272 B
    GPUSpotLightArray   spots;    // offset 320, 528 B   <- the VPs are in here
};
static_assert(sizeof(GPULighting) == 848, ...);
```

Both destinations are filled from the same CPU array, in two functions
of `direction/mood.hpp`:

- `apply_mood_spot_lights` runs `compute_spot_light_vp` for every
  `i < cpuSpotLights_.count` into `cpuSpotLights_.lights[i].view_proj`,
  then calls `stage_spot_vps(queue, c->cpuSpotLights_)` →
  `spotVPStagingBuffer_`, which the per-tile copy reads.
- `upload_lights` does `lighting.spots = c->cpuSpotLights_;` — a
  whole-struct copy carrying every `view_proj[16]` — then
  `upload_lighting(queue, lighting)` → `lightingBuffer_`, which the
  fragment PCF reads.

**`spotVPStagingBuffer_` is a 256-byte duplicate of bytes already
uploaded to `lightingBuffer_` at offset 320**, filled from the same
source array in the same mood application.

---

## D2′ — the alternative the gate found, priced

Same outcome as ATLAS_1 (96 → 32 MiB at 4 lights), no struct widening, no
collision.

| | D2 as ruled | D2′ |
|---|---|---|
| storage added | `GPUVPMatrix` 128 → 320 B, on **two** buffers (`vpBuffer_` and `photographerVPBuffer_` share the struct) | **none** — the VPs already exist |
| bind-group entries resized | 8 (`sizeof(GPUVPMatrix)` sites) | 0 |
| slot-0 ownership | **two owners, colliding** | unchanged — `light_vp` stays the sun's, single owner `compute_vp` |
| 14th read site (`sample_shadow_pcf`) | must be hand-pinned to `light_vps[0]` | **untouched** |
| photographer buffer | widens to carry a 4-light array it never uses | untouched |
| ledger delta | `render_vp` + photographer VP rows 128 → 320 B | one visibility cell, `F` → `VF` |

**The shape.** Each shadow VS gains `@builtin(instance_index) li : u32`
and selects its matrix:

- outdoor / sun tile → `render_vp.light_vp` (exactly today's read)
- spot tile → `render_lighting.spots.lights[li].view_proj` (exactly what
  the fragment PCF already reads for the same light)

`spotVPStagingBuffer_`, `stage_spot_vps`, the per-tile copy, and the
registry constant all still die, as D4 rules.

**Its one cost.** `render_lighting` is `Uniform`, visibility **`F`** on
the Render Entity Layout. A shadow VS cannot read it without a visibility
change to `VF`. That is one word in one layout entry, and the budget
absorbs it: every shadow pipeline's V stage stands at **uniform 5 of 12**
(Table B, charged/declared/actual `5 / 5 / 1`), going to 6 of 12. The
ledger's gate row — `Update Player Agent (0D, 1 thread) / C at uniform 10
of 12` — is a **compute** row on the Compute Entity Layout and is not
touched by a Vertex-visibility change.

**A latent consequence, flagged and NOT taken here.** Once `light_vp` is
no longer clobbered by the tiles, the reason
`calc_directional_light` skips the sun PCF indoors evaporates — the sun
shadow could be restored indoors by deleting the
`spots.count == 0u` guard. That changes texels, which D5 forbids and
which this gate has no mandate for. It is recorded as a future ruling,
not performed.

---

## WHY THIS GATE STOPPED, AND WHAT WAS NOT DONE

Three reasons, in order of weight:

1. **A ruled decision cannot ship as written.** D2/D3/D4 together produce
   a wrong render in the indoor arm. Resolving it means re-ruling D2 —
   its byte count is stated in the handoff (`256 B; array stride 64`) —
   and that is Jean's call, not a recon gate's.
2. **The mandated witness cannot run.** The handoff requires "naga after
   every WGSL-touching commit". There is no naga, no Dawn, and no `emcc`
   in this container. ATLAS_1 adds a builtin to 13 vertex entry points,
   rewrites 14 read sites, and changes a bind-group visibility; landing
   that on master unverified would put the live site at risk for a
   perf win the meter cannot even see directly.
3. **The stop conditions say so.** U0.5's "anything else is a STOP" is
   met.

**Not done:** no `src/` file was modified by ATLAS_1. U1–U5 are
untouched. `spotVPStagingBuffer_` still lives.

**Ready for the next session:** U0.2, U0.3, U0.4 and U0.6 are complete
and need no re-running unless HEAD moves. The single open question is
whether Jean rules D2′ in place of D2. If yes, the campaign shrinks:
U1 becomes a visibility flip plus the VS matrix selection, U2 is
unchanged, U3 is unchanged, U4 is unchanged, and U5's predicted ledger
delta becomes one visibility cell rather than two byte columns.
