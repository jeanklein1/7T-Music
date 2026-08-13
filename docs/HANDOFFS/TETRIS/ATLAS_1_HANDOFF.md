# ATLAS_1 — ONE PASS PER SHADOW TEXTURE

The ruling PASS_0 priced. The indoor arm today runs one render pass per
light against two shared textures, with `Load` on every odd tile purely
to preserve the even tile — 96 MiB/frame of shadow attachment traffic at
4 lights, of which 32 MiB is preservation `Load`s and 32 MiB duplicate
`Store`s (Q3.1). After this campaign each shadow texture is **cleared
once, drawn for all its lights under per-light viewports, stored once**:
96 → 32 MiB at 4 lights, 48 → 16 at 2 (Q3.3). Outdoor is unchanged. Tile
geometry (1024 × 2048 halves) and every line of PCF math are unchanged —
this campaign restructures passes, not texels.

## THE DESIGN, RULED

- **D1 — pass-per-texture.** Indoor: one pass on `shadowMapTexture_`
  covering lights 0–1, one on `spotShadowMapTexture_` covering 2–3; a
  pass is opened only if it owns ≥ 1 active light. `Clear` at open,
  `Store` at close, no `Load` anywhere. The `li < 2` texture split (A4)
  survives deliberately.
- **D2 — the light slot becomes an array.** `vp_buffer`'s `light_vp`
  window (64 B, one `mat4x4`) widens to
  `array<mat4x4<f32>, MAX_SPOT_LIGHTS>` (256 B; array stride 64 —
  uniform-legal). Mirror law: both rooms, same commit, witnesses
  updated.
- **D3 — the draw names its light.** Every shadow draw passes
  `firstInstance = li` (`DrawIndexed(count, 1, 0, 0, li)` — core for
  direct draws; the `indirect-first-instance` feature gates only
  indirect buffers). Each shadow VS gains
  `@builtin(instance_index) li : u32` and reads `light_vps[li]` where it
  read `light_vp`. Outdoor draws pass 0 and the sun VP lives in
  `light_vps[0]`, preserving today's one-slot semantics as the array's
  first element.
- **D4 — the copy dance dies.** `derive_indoor_lights` writes all
  active VPs into the widened slot directly (one `WriteBuffer` at mood
  change); the per-tile `CopyBufferToBuffer` and, if U0 confirms no
  other reader, `spotVPStagingBuffer_` itself are retired (its registry
  number is freed per L6.5).
- **D5 — untouched by decree:** `SHADOW_MAP_SIZE`, `TILE_W/TILE_H`, the
  viewport/scissor rectangles per light, `sample_spot_shadow_pcf` and
  its banner, UMBRA_4's `cast_terrain=false` for spot tiles, and the
  outdoor arm's structure.

## U0 — RECON GATE (census before any edit; STOP on any mismatch)

1. Git hygiene; record HEAD. Confirm DISCARD_0 landed or note its
   absence (independent either way).
2. Read `render_shadow_pass` whole. Confirm the indoor loop matches
   PASS_0 Q2.1 verbatim (the `within`/`use_sun_map` branches, the
   per-tile copy of Q4.3).
3. **The outdoor assumption is load-bearing:** confirm the outdoor arm
   delivers the sun VP through the same `light_vp` slot the shadow VSes
   read. If the sun VP reaches shadow VSes any other way → **STOP and
   report**; D3's `light_vps[0]` convention depends on it.
4. Enumerate every WGSL reader of `light_vp` with an expected count
   (PASS_0 names 13 shadow vertex entry points; the read-site count may
   exceed 13 — report the number found, then use it as the edit count).
5. Read `GPUState::light_vp_offset()/light_vp_size()` and the C++ twin
   of the slot; enumerate every C++ writer/reader of the slot and of
   `spotVPStagingBuffer_` (expected: `stage_spot_vps`, the per-tile
   copy, the outdoor sun write — anything else is a STOP).
6. Read the `sample_spot_shadow_pcf` banner (A5) and confirm nothing in
   it assumes one-pass-per-tile.

## THE UNITS (one commit each, naga after every WGSL-touching commit)

- **U1 — widen the slot.** Both rooms, same commit: C++ slot size/offset
  + witnesses; WGSL `light_vp` → `light_vps: array<mat4x4<f32>, 4>`
  with all read sites updated to `light_vps[li]` and each shadow VS
  gaining the `instance_index` builtin. (Slot widening and read-site
  rewrite are one logical unit — the tree never compiles with a
  half-mirrored slot.)
- **U2 — thread `firstInstance`.** `draw_shadow_all` gains a light
  parameter; the 13 shadow `draw_*` calls pass it into `DrawIndexed`'s
  `firstInstance`. Outdoor call sites pass 0.
- **U3 — restructure the passes.** Indoor arm: per texture, one
  `BeginRenderPass` (`Clear`/`Store`), then per owned light:
  `SetViewport` + `SetScissorRect` + `draw_shadow_all(c, pass, false,
  li)`. Delete the per-tile copy. Outdoor arm: unchanged except the
  explicit 0.
- **U4 — retire the staging buffer** (only if U0.5 found no other
  reader): buffer, setter, registry constant. `derive_indoor_lights`
  writes `light_vps[0..count)` directly.
- **U5 — ledger regen** (`tools/binding_ledger.py`): the slot's byte
  column moves 64 → 256; nothing else should. Any other delta is a
  FINDING.

## WITNESSES

naga per shader commit (CC). glaw1 + boot + **three visual gates**
(brief ruling 7's walk): outdoor with terrain shadows; indoor 2 lights
(Gallery); indoor 4 lights (Cathedral/Quartet) — shadows must be
pixel-plausible in all three, and the odd tiles must not vanish (the
symptom of a resurrected mid-pass clear). Then preview deploy + one
Pixel window per the spend governor. Observable predictions: indoor
`shadow_pass` **cpu** falls from ~0.29–0.41 toward the outdoor ~0.15–0.2
(fewer pass records — the one column that provably scaled with passes);
GPU row compared against itself only. The 96 → 32 MiB is the model's
number; the meter cannot see it directly and the handoff does not claim
it will.

## STOP CONDITIONS

- Any U0 verbatim mismatch, count mismatch, or extra reader/writer.
- The outdoor sun-VP path bypassing the slot (U0.3).
- Anything in the PCF banner or tiling constants that a one-pass
  structure contradicts (U0.6).
- Any edit that would change tile rectangles, PCF, caster sets, or
  texture formats — those belong to FORMAT_1 and later rounds, not here.
