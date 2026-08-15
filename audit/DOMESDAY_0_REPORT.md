# DOMESDAY_0 — the batch report

One CC session, two streams, ten units plus the closing ledger pass.
Stream A landed on `master`; Stream B waits on `claude/domesday-0` for
Jean's gates (glaw1 + visual + merge). Expected visual delta of the
entire branch: **none.** `binding_gen.py --check`,
`binding_ledger.py --check` and `command_census.py --check` are fully
green at both tips. No compile ran in this environment (no emsdk, no
Dawn checkout); the machine gates that ran are the witnesses, and the
Pixel boot after merge is also the batch's compile gate.

## §1 — unit table

| unit | verdict | where |
|---|---|---|
| A1 — MANIFEST view | **LANDED** `2491b65` | `master` — emitter + witness M-1 in `binding_gen.py`; `audit/MANIFEST.md` |
| A2 — feature ids get names | **FLAGGED** | expected an authoritative `WGPUFeatureName` enum header in-tree; found none — the web twin takes WebGPU from the `emdawnwebgpu` remote port fetched at build time (`CMakeLists.txt:576-583`) and the native twin points at an out-of-tree Dawn checkout `C:/dev/dawn` (`CMakeLists.txt:96-97`). The handoff bars transcribing from memory, so no edit was made. The print site is `src/console/console.hpp:643` (`adapter offers`), ready for the map once a header is vendored or the port pinned to a local release. |
| A3 — maxImmediateSize probe row | **LANDED** `1356750` | `master` — one additive row at the granted-vs-floor print, `src/console/console.hpp` |
| A4 — COMMAND_LEDGER | **LANDED** `3c22376` | `master` — `tools/command_census.py` + `audit/COMMAND_LEDGER.md`; witness C-1 holds (6 storeOp tokens, each attributed to exactly one of 20 pass rows) |
| A5 — staging census | **LANDED** (this report, §2) | facts only, no ruling |
| A6 — law census | **LANDED** (this report, §3) | quotes with anchors |
| B1 — render_vp → uniform | **LANDED** `42f78ad` | `claude/domesday-0` |
| B2 — render_camera → uniform | **LANDED** `6ceec64` | `claude/domesday-0` |
| B3 — visible_patch_indices → instance attribute | **LANDED** `c4c537e` | `claude/domesday-0` |
| B4 — main depth storeOp discard | **ALREADY IN-TREE** (zero commits) | A4's census computed the finding — (a) no, (b) no — and found the edit already made: `depthStoreOp = wgpu::StoreOp::Discard` since DISCARD_0 / PASS_0 F1 (`render_passes.hpp` `render_main_pass`), with the snapshot twin at F2 (`gallery.hpp`). No stencil aspect exists. COMMAND_LEDGER §4 carries the finding line and the STATUS verbatim. |
| B-ledger — regenerate | **LANDED** `d7d5043` | `claude/domesday-0` — content hashes were already current from B3 (S-1 required the regeneration there); the closing pass restamped the two source-commit handles |

Global stops: none tripped. Working tree was clean on current `master`
(`bec480c`), and `--check` passed before any edit.

## §2 — A5, the staging census (facts, no ruling)

Both pools are created once, in `GPUState::initOffscreenResources`
(`state.hpp`), called from the cartridge's `init_renderer`
(`cartridge.hpp:631`): `512 × 512 × 32 layers` in the swapchain color
format — `PAINTING_RESOLUTION = 512`, `STAGING_LAYERS = 32`
(`state.hpp` Dim) — 1 MiB per layer, **32 MiB each**, exactly the two
rows the boot budget names.

**Authored Staging** (`authoredStagingTexture_`):

1. **Allocation** — `makeTextureArray("Authored Staging",
   Dim::STAGING_LAYERS, CopyDst | CopySrc)`, `state.hpp`
   `initOffscreenResources`. Boot-owned: created once; teardown keeps
   it ("staging persists across worlds", `teardown_gallery`,
   `gallery.hpp`); never recreated.
2. **Writer** — `GPUState::upload_authored_painting` (`state.hpp`):
   `queue.WriteTexture` of CPU RGBA bytes, with a CPU R↔B swap when
   the array is BGRA. The bytes come from
   `authored_stage_decoded_image` (`gallery.hpp`), which CPU
   scale-to-fits the decode into a 512² padded box; the decode itself
   is stb_image — native `stbi_load` from disk, web
   `emscripten_fetch` → `stbi_load_from_memory` **in wasm**
   (`authored_image_onsuccess`, `gallery.hpp`). No ImageBitmap, no
   browser decode.
3. **Reader** — `GPUState::promote_to_exhibition` (`state.hpp`):
   `encoder.CopyTextureToTexture`, full 512² layer, staging layer →
   `exhibitionTexture_` layer, driven by `drain_gallery_promotions`
   (`gallery.hpp`).
4. **Ordering** — a mailbox across frames. Writes ride the queue
   timeline from fetch callbacks (web) or the boot/world-change load
   (`load_authored_textures`); promotion drains inside a later
   frame's encoder; the CPU-side `rec.valid` flag is the
   consumability gate; `rotate_authored_staging` refills consumed
   slots on world change.

**Snapshot Staging** (`snapshotStagingTexture_`):

1. **Allocation** — same site, `makeTextureArray("Snapshot Staging",
   Dim::STAGING_LAYERS, CopyDst | CopySrc)`. Boot-owned, never
   recreated.
2. **Writer** — the tail of `render_snapshot_pass` (`gallery.hpp`):
   the photographer renders into `offscreenColorTexture_`
   (`RenderAttachment | CopySrc`), and after `pass.End()` the same
   encoder copies it into the staging layer
   (`encoder.CopyTextureToTexture`). A GPU pass source, not a CPU
   upload.
3. **Reader** — the same `promote_to_exhibition` copy into the
   Exhibition array.
4. **Ordering** — same frame. `drain_gallery_promotions` runs after
   `render_snapshot_pass` by the O-7 order note ("after
   render_snapshot_pass, so the snapshot staging texture holds this
   frame's shot") — one encoder carries offscreen render → copy to
   staging → copy to exhibition.

**The facts the −64 MiB question (a _2 matter) turns on:** the
authored path's source is web-fetched **encoded** image bytes decoded
in wasm — `copyExternalImageToTexture` could instead hand decode and
write to the browser (ImageBitmap → destination array layer directly,
browser-owned staging), which would retire the wasm decode, the CPU
scale/pad box, the BGRA swap, and the Authored Staging array in one
motion. The snapshot path's destination layer view could be a render
target directly only if `exhibitionTexture_` gained
`RenderAttachment` (today: `CopyDst | TextureBinding`); rendering to a
single-layer 2D view of an array texture is legal WebGPU, and the
full-layer-overwrite invariant that layer reuse depends on
(`promote_to_exhibition`'s banner) would then need a new home. This
unit states the facts; it does not rule.

## §3 — A6, the law census (quotes, with anchors)

**L23′, full text** — `docs/LAWS.md:577` (the schema's
`REGISTRY_INVARIANTS` and the layout prose carry condensed mirrors;
the law's home is LAWS.md):

> ## L23′ — THE SCOPE LAW (supersedes L23)
>
> Within one synchronization scope, a buffer presents ONE writability.
> A render pass is one scope, WHOLE. A compute dispatch is one scope
> over its FULL bound groups. Neither is filtered by shader-stage
> visibility or by static use — Dawn merges every entry of every bound
> group, touched or not (Jean's boot log at the U4 gate is the
> evidence; L23's "compute validates per dispatch over what it uses"
> was the half of the truth that survived one gate).
>
> So: **mixed-writability faces of one buffer never share a layout and
> are never co-bound in one scope.** A stratum serving a scope carries
> only the faces that scope may legally see — FRAME split by consumer
> mode (FRAME_R render / FRAME_C compute, A8a); ORBS carries its face
> partition in two layouts (A8b); GALLERY/PHOTO_K stand from A7.
>
> Witness: `P-scope`, both arms — the render arm per pass span, the
> compute arm per dispatch site over the full bound groups, plus the
> group-local law (no bind group backs one buffer through entries of
> mixed writability). **Pessimism is the law: no relaxation of the rule
> may ever be committed on a citation — only on a witnessed Dawn
> behavior test.**
>
> Paid for twice: A7's gallery working set in the render passes, then
> A8's FRAME ro windows and collapsed orb faces at the compute
> dispatches — the same law, learned one scope at a time.

**The orb defended sites** — the prose adjacent to the four binding
constants and the copy kernel, before any swap ruling:

`world.wgsl:12905-12916` (beside the g2:120/122/123/124 declarations):

> ```
> // Previous-frame snapshot (read-only view in main layout). Written
> // by orb_state_prev_copy before each frame's dynamics dispatch so
> // flocking can query neighbors against a stable previous frame.
> @group(2) @binding(122) var<storage, read> orb_state_prev: array<OrbState>;
> // Inverse-access views used only by orb_state_prev_copy. They
> // reference the same physical buffers through a dedicated copy
> // layout. WebGPU requires each shader declaration to match exactly
> // one layout access mode, so 410/412 (bound read_write/read in the
> // main layout) can't be re-used here with swapped access modes.
> @group(2) @binding(123) var<storage, read>       orb_state_ro:      array<OrbState>;
> @group(2) @binding(124) var<storage, read_write> orb_state_prev_rw: array<OrbState>;
> ```

`world.wgsl:13055-13059` (the kernel's own header):

> ```
> // Snapshot orb_state → orb_state_prev so the dynamics kernel can
> // read last frame's positions/velocities while writing the new ones.
> // Uses the dedicated copy layout's bindings (413 read, 414 read_write)
> // rather than 410/412, which are bound read_write/read in the main
> // layout — see the binding-layout comment above.
> ```

`orbsBStateLayout_` creation prose (schema `LAYOUTS`, emitted at
`binding_surface.gen.inc:547-548`):

> ```
> // Orbs B State Layout — stratum 2, ORBS_B face set (A8b): orb_state_ro,
> // orb_config, orb_state_prev_rw — the prev-copy kernel.
> ```

and the A face's twin (schema `LAYOUTS['orbsAStateLayout_']`):

> ```
> // Orbs A State Layout — stratum 2, ORBS_A face set (A8b): orb_state rw,
> // orb_config, orb_state_prev ro — init / dynamics / recolor. One buffer,
> // one writability per scope (L23-prime); the faces partition disjointly.
> ```

The four registry constants themselves (`binding_registry.hpp`, ORBS
band 120–139) carry **no trailing prose** — the band banner
`// ORBS (120–139)` is all the registry says.

**The reconfigure trigger** — one home, `audit/COMMAND_LEDGER.md` §3,
quoted verbatim there from `src/console/console.hpp:1291`
(`Console::begin_frame`): a bare not-equal on the capped framebuffer
size whose branch reconfigures the surface, recreates the depth
buffer, and fires the `[FRAME_1]` print, with no settling window.
The boot-time `Configure` in `initSurface` (`console.hpp:1177`) is
the only other site.

## §4 — wallet before / after

MANIFEST's worst rows, program-wide (each cell used / limit):

| lane | batch start (`2491b65`, = LEDGER Table B at `37bc4de`) | batch end (`claude/domesday-0` tip) |
|---|---|---|
| uniform | 11 / 12 — `updatePlayerAgentPipeline_` C (+3 more, the agents compute family) | 11 / 12 — unchanged (the panel's coalescence relieves that one, not this batch) |
| storage | 6 / 8 — `patchTerrainPipeline_` V (+25 more render rows) | 5 / 8 — `updatePlayerAgentPipeline_` C (+7 more; **no render row above 4**) |
| sampled | 6 / 16 | 6 / 16 |
| samplers | 3 / 16 | 3 / 16 |
| storagetex | 2 / 4 | 2 / 4 |
| immediates (bytes) | 0 / 64 | 0 / 64 — the lane exists to be visible; A3 prints the grant |

Per-family, the rows the handoff predicted: scene family V is storage
**3 / 8** and uniform **9 / 12** at batch end (was 6 and 7); F is
uniform 6 / 12 (was 4). One movement the handoff did not name: the
**shadow family** V also fell 6 → 4 storage, because B1/B2 demoted the
two FRAME windows every render family shares. The program-wide worst
storage row now sits on the compute side (5), not render.

## §5 — anything unexpected, one line each

- **B4's edit predates the batch**: the main depth `storeOp` has read
  `Discard` since DISCARD_0 / PASS_0 F1, snapshot twin F2 — the census
  confirmed the finding's two facts and had nothing to edit.
- **No WebGPU enum header exists in-tree** (A2's flag): both twins take
  their headers from outside the repository.
- **A4's named scan set didn't match the tree**: `renderer.hpp` encodes
  zero passes (witness C-4 pins the zero); passes live in the
  cartridge, four bodies files and the patch surface; the frame submit
  in `incubator_dual.cpp`; the trigger in `console.hpp`. The census
  scans where the facts are and discloses the widening.
- **`frameRLayout_` has a second instantiation**: `framePhotographerGroup_`
  seats the same g1:3/g1:4 slots with `photographerVPBuffer_` /
  `photographerCameraBuffer_`, so B1/B2 added `Uniform` usage to those
  two backings as well (their kernel faces g2:161/162 stay storage).
- **The plan A/B/C scene groups became identical** when B3 retired the
  seat whose segment windows were their only difference; they keep
  their three names and per-slot binds — the collapse is a queued
  ruling, not taken here.
- **`binding_ledger.py` needed teaching for B3** (instrument
  maintenance, disclosed in the commit): `@location` entry-point params
  now classify as vertex-attribute provenance, W2-3's positive control
  re-anchored on `patch_instances[actual_id]`, finding 9 recorded as
  spent.
- **A3 is compile-gated on the port's `wgpu::Limits` carrying
  `maxImmediateSize`** — no emsdk in this environment, so Jean's Pixel
  boot is the first compile this row sees.
- **Stale prose observed, untouched**: the world.wgsl orb banner
  (`world.wgsl:12713-12722`) still describes the orb bind topology by
  retired pre-recut numbers (410–414, 201, 280); and the
  `cullStateLayout_` seat 2 trailing comment still glosses `fc_visible`
  by the retired sibling name `visible_patch_indices` (as a content
  description it remains true). Residue for a sweep, not forced here.

One line for the spirit: **the survey paid for itself on the way
through — three storage seats and a lane we'd never driven, recovered
while the census walked the estate.**
