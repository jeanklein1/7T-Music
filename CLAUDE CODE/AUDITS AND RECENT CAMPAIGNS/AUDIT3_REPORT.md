# AUDIT-3 REPORT — post-UNIFIED_GROUND_1 pin + Stage-6/7 groundwork

- **Base:** `origin/UNIFIED_GROUND_1` HEAD =
  `f3c61fa7a65325ee3661f12b677607348050cf16` (Jean's designation by
  push placement — the handoff + the WGSL-spec resource landed on this
  branch; its `src/cartridges` tree is identical to the U6 closeout
  commit 6a50738).
- **Audit branch:** `UNIFIED_GROUND_1_AUDIT3` (this report; no mainline
  edits anywhere).
- **Probe branch:** `UNIFIED_GROUND_1_A3_P1` (shape probes only — the
  probe script + raw results as audit artifacts; no removals, never
  merged).
- **Resource noted:** `src/docs/HANDOFFS/HANDOFF AUDIT 3/WebGPU Shading
  Language.pdf` — the WGSL spec; already earned its keep (see A3-5's
  reserved-keyword catch).

---

## [A3-1] BASELINE PIN — VERDICT: PASS

| Instrument | vs `_post_ug1` |
|---|---|
| cc6_layout_budgets | **BYTE-IDENTICAL** |
| cc7_wgsl_binding_census | **BYTE-IDENTICAL** |
| cc7_mirror_cross | **BYTE-IDENTICAL** |
| cc4_wgsl_static_usage | **BYTE-IDENTICAL** |
| Dawn witness (U roster) | **Structurally identical** (only ms timing fields differ); ALL PIPELINE FAMILIES GREEN again |

**Jean's UG1 gate outcomes** (per this handoff's header; no re-runs):

| Gate | Outcome |
|---|---|
| Windows build | **PASS** (reported) |
| Boot (FXC watch: unified decode ×2, zone_seed_mask, retired-pipeline absence) | **PASS** (reported) |
| Rest identity, bitwise | UNKNOWN |
| The zone walkthrough | UNKNOWN |
| The five carried-over batch gates | UNKNOWN |

---

## [A3-2] THE PARKED-NUMBER LEDGER — VERDICT: PASS (zero stragglers)

Tree-wide greps (src/, comment tombstones excluded from "straggler"):

| Parked | Precedent | Tombstone site(s) | Stragglers |
|---|---|---|---|
| g0:29 cell_fields_write | Commit C (LUT retirement) | world.wgsl:5476, registry:66 | NONE (the live `@group(1) @binding(29)` patch_cell_color_array_read is a DIFFERENT slot space — g1 — not a straggler) |
| g1:30 cell_fields_read | Commit C | world.wgsl:5485, registry:180 | NONE |
| g0:149 pyramid_ground | the 149 precedent itself | world.wgsl:8263, state.hpp:4287, registry:95 | NONE |
| g0:163/164/165 (zone terrain-eval trio) | 149 precedent, UNIFIED_GROUND_1 U4 | registry:105–107 | NONE |
| g0:167/168/169 (zone mesh trio) | 149 precedent, UNIFIED_GROUND_1 U4 | registry:109–111 | NONE |
| ZONE_DERIVE_EXTENT | U5b (const parked) | world.wgsl:5794 | NONE |
| Dim::PATCH_INDEX_COUNT | doc-only note (U2b) | state.hpp:142 (definition) | confirmed ZERO runtime readers; no edit |

Zero WGSL `@binding` literals exist for any parked number in its parked
group (machine grep). The bookkeeping closes clean.

## [A3-3] STAGE-6 FACTS — the true-band swap raw material — VERDICT: PASS

All quotes at base f3c61fa7a65325ee3661f12b677607348050cf16.


### (a) the band machinery post-retirement

`src/cartridges/the_board/realization/world.wgsl:430-460` — TerrainBand struct + the 6-band table (λ ordering: tectonic 500 > continental 200 > regional 80 > local 30 > detail 12 > FINE 5 — band 4 "fine" is the fine ripple the Stage-6 ruling keeps static)

```
}


// §1.6 TERRAIN HEIGHT FUNCTION
// Stateless terrain height evaluation. Given (world_xz, master_seed, time),
struct TerrainBand {
    spacing: f32,        // lattice spacing (world units between nodes)
    freq_mean: f32,      // μ for spatial frequency (cycles per unit)
    freq_sigma: f32,     // σ for spatial frequency
    amp_mean: f32,       // μ for amplitude (world units of height)
    amp_sigma: f32,      // σ for amplitude
    damping_mean: f32,   // μ for damping coefficient (Gaussian draw)
    damping_sigma: f32,  // σ for damping coefficient
    damping_min: f32,    // floor — no wave extends beyond ~3/damping_min units
    activation: f32,     // probability that a lattice node contributes at all
    temporal_freq: f32,  // per-band multiplier on pool beat frequency
}

const TERRAIN_BAND_COUNT: u32 = 6u;

const TERRAIN_BANDS = array<TerrainBand, 6>(
    //              spacing  freq_μ  freq_σ  amp_μ  amp_σ  damp_μ  damp_σ  damp_min activ  t_freq
    //                                                                      reach≈3/min
    TerrainBand(    200.0,   0.030,  0.010,  8.0,   3.0,   0.008,  0.004,  0.005,  0.70,  0.05  ),  // 0: continental  reach≈600
    TerrainBand(     80.0,   0.080,  0.025,  3.0,   1.5,   0.020,  0.010,  0.010,  0.65,  0.10  ),  // 1: regional     reach≈300
    TerrainBand(     30.0,   0.200,  0.060,  1.2,   0.5,   0.040,  0.020,  0.020,  0.60,  0.20  ),  // 2: local        reach≈150
    TerrainBand(     12.0,   0.500,  0.150,  0.4,   0.2,   0.080,  0.040,  0.040,  0.55,  0.40  ),  // 3: detail       reach≈75
    TerrainBand(      5.0,   1.200,  0.350,  0.12,  0.05,  0.150,  0.075,  0.060,  0.50,  0.80  ),  // 4: fine         reach≈50
    TerrainBand(    500.0,   0.012,  0.004,  15.0,  6.0,   0.004,  0.002,  0.003,  0.75,  0.02  ),  // 5: tectonic     reach≈1000
);

```

`src/cartridges/the_board/realization/world.wgsl:474-485` — the activity gate (band_act source; WAVE_THRESHOLD read)

```
// SPATIAL FIELD MANIFEST — seed plumbing only. The tunable activity/
// band values (ACTIVITY_LATTICE_SPACING, ACTIVITY_BEAT_FREQ_LO/HI,
// WAVE_THRESHOLD[6] + softness) moved to THE TERRAIN_LOOKS PANEL
// ROW 7 (§2.2) — the movement third of the surface voice.
const ACTIVITY_SEED_BAND: u32       = 50u;      // lattice seed band (separate from terrain)
const ACTIVITY_PROP_LEVEL: u32      = 220u;     // property index: activity intensity
const ACTIVITY_PROP_BEAT_FREQ: u32  = 221u;     // property index: beat frequency

fn band_activity_level(raw_activity: f32, band_index: u32) -> f32 {
    let threshold = WAVE_THRESHOLD[band_index];
    return smoothstep(threshold, threshold + WAVE_THRESHOLD_SOFTNESS, raw_activity);
}
```

`src/cartridges/the_board/realization/world.wgsl:529-545`

```
fn evaluate_directional_wave(
    world_xz: vec2<f32>,
    node_world: vec2<f32>,
    freq: f32,
    amp: f32,
    damping: f32,
    dir: vec2<f32>,
    phase: f32,
) -> f32 {
    // Spatial phase: globally coherent ridges along `dir`.
    // Damping: envelope decays with perpendicular distance from the line
    // through node_world in direction `dir`.
    let offset = world_xz - node_world;
    let perp_dist = abs(offset.x * dir.y - offset.y * dir.x);
    let envelope = exp(-damping * perp_dist);
    return amp * envelope * sin(dot(dir, world_xz) * freq + phase);
}
```

`src/cartridges/the_board/realization/world.wgsl:547-560`

```
fn evaluate_radial_wave(
    world_xz: vec2<f32>,
    center: vec2<f32>,
    freq: f32,
    amp: f32,
    damping: f32,
    phase: f32,
) -> f32 {
    // Concentric rings from `center`.
    // Damping: envelope decays with distance from center.
    let dist = length(world_xz - center);
    let envelope = exp(-damping * dist);
    return amp * envelope * sin(dist * freq + phase);
}
```

`src/cartridges/the_board/realization/world.wgsl:560-612` — the frozen path, the MOVING path (phase_moving), the frozen↔moving mix gated by band_act

```
}

// --- Per-Node Wave Evaluation
fn evaluate_lattice_wave(
    world_xz: vec2<f32>,
    node: vec2<i32>,
    node_seed: u32,
    band: TerrainBand,
    band_act: f32,       // activity level for this band [0,1] (from hierarchy)
    beat_freq: f32,      // cycles per beat from activity field
    t_beats: f32,        // current time in beats
) -> f32 {
    // Activation gate: if draw exceeds band activation, this node is silent.
    // Spatially coherent — a silent node is silent for all query points.
    if (hash_property(node_seed, WAVE_PROP_ACTIVE) > band.activation) {
        return 0.0;
    }

    // Derive parameters from seed via Gaussian / uniform draws
    let is_radial = hash_property(node_seed, WAVE_PROP_TYPE) > 0.5;
    let freq = abs(sample_gaussian(node_seed, WAVE_PROP_FREQ, band.freq_mean, band.freq_sigma));
    var amp = abs(sample_gaussian(node_seed, WAVE_PROP_AMP, band.amp_mean, band.amp_sigma));
    // Indoor amplitude ceiling: clamp large waves to keep terrain gentle
    if (config.terrain_amp_ceiling > 0.0) {
        amp = min(amp, config.terrain_amp_ceiling);
    }
    let damping = max(abs(sample_gaussian(node_seed, WAVE_PROP_DAMPING, band.damping_mean, band.damping_sigma)), band.damping_min);
    let phase_base = hash_property(node_seed, WAVE_PROP_PHASE) * 2.0 * PI;

    // Two phases: frozen is the reference shape, moving advances with beats.
    // band.temporal_freq scales the pool's beat_freq per band:
    //   fine bands ripple fast, continental bands swell slowly.
    let phase_frozen = phase_base;
    let phase_moving = phase_base + t_beats * beat_freq * band.temporal_freq * 2.0 * PI;

    // Node world position: center of this lattice cell
    let node_world = (vec2<f32>(node) + 0.5) * band.spacing;

    if (is_radial) {
        let offset_angle = hash_property(node_seed, WAVE_PROP_DIR_ANGLE) * 2.0 * PI;
        let offset_r = hash_property(node_seed, WAVE_PROP_CENTER_R) * band.spacing * 0.3;
        let center = node_world + vec2(cos(offset_angle), sin(offset_angle)) * offset_r;
        let val_frozen = evaluate_radial_wave(world_xz, center, freq, amp, damping, phase_frozen);
        let val_moving = evaluate_radial_wave(world_xz, center, freq, amp, damping, phase_moving);
        return mix(val_frozen, val_moving, band_act);
    } else {
        let dir_angle = hash_property(node_seed, WAVE_PROP_DIR_ANGLE) * 2.0 * PI;
        let dir = vec2(cos(dir_angle), sin(dir_angle));
        let val_frozen = evaluate_directional_wave(world_xz, node_world, freq, amp, damping, dir, phase_frozen);
        let val_moving = evaluate_directional_wave(world_xz, node_world, freq, amp, damping, dir, phase_moving);
        return mix(val_frozen, val_moving, band_act);
    }
}
```

`src/cartridges/the_board/realization/world.wgsl:627-639` — the phase-origin read

```
}

fn get_band_phase_origin(band_index: u32) -> f32 {
    switch(band_index) {
        case 0u: { return config.band_phase_origin_0; }
        case 1u: { return config.band_phase_origin_1; }
        case 2u: { return config.band_phase_origin_2; }
        case 3u: { return config.band_phase_origin_3; }
        case 4u: { return config.band_phase_origin_4; }
        case 5u: { return config.band_phase_origin_5; }
        default: { return 0.0; }
    }
}
```

`src/cartridges/the_board/realization/world.wgsl:686-705` — the fused bake body (the sole surviving whole-stack evaluator)

```
}

// --- Total Height
// (fn terrain_height_at RETIRED — UNIFIED_GROUND_1 U4; A2-3 census)

// --- Height + Complexity
fn terrain_height_and_complexity(world_xz: vec2<f32>, master_seed: u32, t_beats: f32) -> vec2<f32> {
    let af = terrain_activity_at(world_xz, master_seed);
    let raw_activity = af.x;
    let beat_freq = af.y;

    var height: f32 = 0.0;
    var complexity: f32 = 0.0;
    for (var b: u32 = 0u; b < TERRAIN_BAND_COUNT; b++) {
        let hc = terrain_band_contribution(world_xz, master_seed, t_beats, b, raw_activity, beat_freq);
        height += hc.x;
        complexity += hc.y;
    }
    return vec2(height, complexity / f32(TERRAIN_BAND_COUNT));
}
```


### (b) gradient forms — the bake TWO-PASS pattern (the card writer model)

No true-band with-gradient variant exists — gradients are bake-side only (pass 2 central differences).

`src/cartridges/the_board/realization/world.wgsl:7426-7447` — pass 1: heights to scratch

```
//   No terrain evaluation at all.
//
// The compute pass boundary between them provides the storage buffer
// barrier. Net effect: 1 terrain eval per texel total, not 6.

@compute @workgroup_size(16, 16)
fn generate_patch_heights(@builtin(global_invocation_id) id: vec3<u32>) {
    let res = patch_params.resolution;
    if (id.x >= res || id.y >= res) { return; }

    let res_f = f32(res);
    let uv = vec2<f32>(id.xy) / (res_f - 1.0);
    let world_xz = vec2<f32>(
        patch_params.origin.x + (uv.x - 0.5) * patch_params.extent,
        patch_params.origin.y + (uv.y - 0.5) * patch_params.extent
    );

    let hc = ground_formed_with_complexity(world_xz);
    let base = (id.y * res + id.x) * 2u;
    patch_height_scratch[base]      = hc.x;   // height (stride-2 layout kept; the
    // +1 complexity slot is no longer written — no reader)
}
```

`src/cartridges/the_board/realization/world.wgsl:7451-7540` — pass 2: scratch neighborhood -> gradients

```

@compute @workgroup_size(16, 16)
fn generate_patch_gradients(
    @builtin(global_invocation_id) id: vec3<u32>,
    @builtin(local_invocation_id) lid: vec3<u32>,
    @builtin(workgroup_id) wid: vec3<u32>
) {
    let res = patch_params.resolution;
    let res_i = i32(res);

    // ── Cooperative tile load: 20×20 from global scratch ────────────
    // 256 threads load 400 cells via stride. Halo cells outside the
    // 256×256 grid clamp to boundary (safe: edge stencils only read
    // inward from the boundary, never into clamped halo).
    let thread_id = lid.y * 16u + lid.x;
    let tile_origin_x = i32(wid.x * 16u) - 2;
    let tile_origin_y = i32(wid.y * 16u) - 2;

    for (var t = thread_id; t < 400u; t += 256u) {
        let tx = i32(t % 20u);
        let ty = i32(t / 20u);
        let gx = clamp(tile_origin_x + tx, 0, res_i - 1);
        let gy = clamp(tile_origin_y + ty, 0, res_i - 1);
        sh_height[t] = patch_height_scratch[(u32(gy) * res + u32(gx)) * 2u];
    }
    workgroupBarrier();

    // ── Bounds check AFTER barrier (all threads must participate in load) ─
    if (id.x >= res || id.y >= res) { return; }

    // ── Read center height from shared (complexity readback REMOVED) ─
    let cx = lid.x + 2u;
    let cy = lid.y + 2u;
    let height = sh_height[cy * 20u + cx];

    let texel = vec2<i32>(id.xy);
    let layer = i32(patch_params.layer);
    let res_f = f32(res);
    let eps = patch_params.extent / (res_f - 1.0);

    let ix = id.x;
    let iy = id.y;
    let max_i = res - 1u;

    // Gradient computation: central difference in interior,
    // 3-point one-sided stencil at edges for matching O(eps²) accuracy.
    //   Forward:  (-3h[0] + 4h[1] - h[2]) / (2*eps)
    //   Backward: ( 3h[N] - 4h[N-1] + h[N-2]) / (2*eps)

    // ── Gradient X: all reads from shared tile ──────────────────────
    var grad_x: f32;
    if (ix == 0u) {
        let h0 = height;
        let h1 = sh_height[cy * 20u + cx + 1u];
        let h2 = sh_height[cy * 20u + cx + 2u];
        grad_x = (-3.0 * h0 + 4.0 * h1 - h2) / (2.0 * eps);
    } else if (ix == max_i) {
        let h0 = height;
        let h1 = sh_height[cy * 20u + cx - 1u];
        let h2 = sh_height[cy * 20u + cx - 2u];
        grad_x = (3.0 * h0 - 4.0 * h1 + h2) / (2.0 * eps);
    } else {
        let h_px = sh_height[cy * 20u + cx + 1u];
        let h_mx = sh_height[cy * 20u + cx - 1u];
        grad_x = (h_px - h_mx) / (2.0 * eps);
    }

    // ── Gradient Z: all reads from shared tile ──────────────────────
    var grad_z: f32;
    if (iy == 0u) {
        let h0 = height;
        let h1 = sh_height[(cy + 1u) * 20u + cx];
        let h2 = sh_height[(cy + 2u) * 20u + cx];
        grad_z = (-3.0 * h0 + 4.0 * h1 - h2) / (2.0 * eps);
    } else if (iy == max_i) {
        let h0 = height;
        let h1 = sh_height[(cy - 1u) * 20u + cx];
        let h2 = sh_height[(cy - 2u) * 20u + cx];
        grad_z = (3.0 * h0 - 4.0 * h1 + h2) / (2.0 * eps);
    } else {
        let h_pz = sh_height[(cy + 1u) * 20u + cx];
        let h_mz = sh_height[(cy - 1u) * 20u + cx];
        grad_z = (h_pz - h_mz) / (2.0 * eps);
    }

    // The .w channel is unused (was LATENT[complexity], removed by the husk
    // sweep — no consumer ever read it; palette calls read the pinned
    // PALETTE_COMPLEXITY, TERRAIN_LOOKS ROW 3).
    textureStore(patch_heightfield_array_write, texel, layer, vec4(height, grad_x, grad_z, 0.0));
}
```

`src/cartridges/the_board/realization/world.wgsl:5477-5479` — the scratch declaration (g0:28)

```
//  retirement. Number reserved; do not reuse.)
@group(0) @binding(28) var<storage, read_write> patch_height_scratch: array<f32>;

```

`src/cartridges/the_board/realization/state.hpp:4083-4129` — the Patch Gen layout (scratch entry + tile sharing)

```
                }

                // -- Patch heightfield gen layout (Group 0) -- bindings 23-24 -
                // Per-patch compute pass: fills one heightfield layer.
                // Dispatched when a patch enters the active set.
                {
                    std::array<wgpu::BindGroupLayoutEntry, 8> entries{};

                    entries[0].binding = bind::g0::config;    // config (uniform — world_seed for cell color gen)
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].buffer.type = wgpu::BufferBindingType::Uniform;

                    entries[1].binding = bind::g0::patch_params;
                    entries[1].visibility = wgpu::ShaderStage::Compute;
                    entries[1].buffer.type = wgpu::BufferBindingType::Uniform;

                    entries[2].binding = bind::g0::patch_heightfield_array_write;
                    entries[2].visibility = wgpu::ShaderStage::Compute;
                    entries[2].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                    entries[2].storageTexture.format = wgpu::TextureFormat::RGBA16Float;
                    entries[2].storageTexture.viewDimension = wgpu::TextureViewDimension::e2DArray;

                    entries[3].binding = bind::g0::tile_grid;
                    entries[3].visibility = wgpu::ShaderStage::Compute;
                    entries[3].buffer.type = wgpu::BufferBindingType::Uniform;

                    entries[4].binding = bind::g0::pier_instances;   // pier_instances (storage, read)
                    entries[4].visibility = wgpu::ShaderStage::Compute;
                    entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    entries[5].binding = bind::g0::patch_cell_color_array_write;
                    entries[5].visibility = wgpu::ShaderStage::Compute;
                    entries[5].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                    entries[5].storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
                    entries[5].storageTexture.viewDimension = wgpu::TextureViewDimension::e2DArray;

                    entries[6].binding = bind::g0::pyramid_instances;  // pyramid_instances
                    entries[6].visibility = wgpu::ShaderStage::Compute;
                    entries[6].buffer.type = wgpu::BufferBindingType::Uniform;

                    entries[7].binding = bind::g0::patch_height_scratch;  // patch_height_scratch (two-pass heightfield gen)
                    entries[7].visibility = wgpu::ShaderStage::Compute;
                    entries[7].buffer.type = wgpu::BufferBindingType::Storage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Patch Gen Layout";
                    desc.entryCount = entries.size();
```

`src/cartridges/the_board/realization/renderer.hpp:447-476` — the dispatch pair

```
            void dispatch_generate_patch_heights(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generatePatchHeightsPipeline_);
                pass.SetBindGroup(0, patchGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            // Pass 2: read stored heights from neighbors, compute gradients + complexity.
            void dispatch_generate_patch_gradients(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generatePatchGradientsPipeline_);
                pass.SetBindGroup(0, patchGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            void dispatch_generate_patch_cells(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generatePatchCellsPipeline_);
                pass.SetBindGroup(0, patchGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }
```


### (c) the wires

`src/cartridges/the_board/realization/world.wgsl:1510-1530` — DesignConfig band fields (WGSL mirror)

```
    ceiling_height: f32,          // indoor ceiling Y for camera clamp (0 = no ceiling)
    terrain_time: f32,            // t_beats for terrain evaluation (0 = frozen)
    // ─── Polyphony-driven band motion ────────────────────────────
    // Per-band blend: -1 = inactive sentinel (the evaluators skip the band); [0,1] = activation.
    band_blend_0: f32,            // continental
    band_blend_1: f32,            // regional
    band_blend_2: f32,            // local
    band_blend_3: f32,            // detail
    band_blend_4: f32,            // fine
    band_blend_5: f32,            // tectonic
    // Per-band phase origin (t_beats at activation).
    band_phase_origin_0: f32,
    band_phase_origin_1: f32,
    band_phase_origin_2: f32,
    band_phase_origin_3: f32,
    band_phase_origin_4: f32,
    band_phase_origin_5: f32,
    // ─── Musical animation modes ─────────────────────────────────
    mode_color_shift: f32,        // SIGNED axis on the mode field; rest 0 is the CENTER (− retreats, + advances); range graduates at Movement 1 close
    mode_checker_scatter: f32,    // SIGNED axis on sparse survival; rest 0 the center (− extinguishes, + populates); range graduates at Movement 1 close
    mode_palette_target: f32,     // [0,3] target palette (0=sand 1=salmon 2=green 3=warm)
```

`src/cartridges/the_board/realization/state.hpp:440-458` — GPUDesignConfig band fields (C++ room)

```
            float terrain_time;               // t_beats for terrain evaluation (0 = frozen, >0 = animated)

            // ─── Polyphony-driven band motion ────────────────────────────
            float band_blend_0;               // [0] continental
            float band_blend_1;               // [1] regional
            float band_blend_2;               // [2] local
            float band_blend_3;               // [3] detail
            float band_blend_4;               // [4] fine
            float band_blend_5;               // [5] tectonic
            float band_phase_origin_0;        // [0] continental
            float band_phase_origin_1;        // [1] regional
            float band_phase_origin_2;        // [2] local
            float band_phase_origin_3;        // [3] detail
            float band_phase_origin_4;        // [4] fine
            float band_phase_origin_5;        // [5] tectonic

            // ─── Musical animation modes ─────────────────────────────────
            // Each mode is an independently toggleable coupling circuit.
            // Values are [0,1] intensity, driven by polyphony through trajectory ramp.
```

`src/cartridges/the_board/realization/state.hpp:2272-2279`

```
            void set_terrain_time(float t) {
                if (config_.terrain_time != t) {
                    config_.terrain_time = t;
                    configDirty_ = true;
                }
            }
            void set_band_motion(const float blend[6], const float phase_origin[6]) {
                config_.band_blend_0 = blend[0];
```

`src/cartridges/the_board/realization/state.hpp:2278-2291`

```
            void set_band_motion(const float blend[6], const float phase_origin[6]) {
                config_.band_blend_0 = blend[0];
                config_.band_blend_1 = blend[1];
                config_.band_blend_2 = blend[2];
                config_.band_blend_3 = blend[3];
                config_.band_blend_4 = blend[4];
                config_.band_blend_5 = blend[5];
                config_.band_phase_origin_0 = phase_origin[0];
                config_.band_phase_origin_1 = phase_origin[1];
                config_.band_phase_origin_2 = phase_origin[2];
                config_.band_phase_origin_3 = phase_origin[3];
                config_.band_phase_origin_4 = phase_origin[4];
                config_.band_phase_origin_5 = phase_origin[5];
                configDirty_ = true;
```

`src/cartridges/the_board/cartridge.hpp:411-427` — the boot pin (terrain_looks ROW 2 REST_*)

```
                auto tGpu1 = std::chrono::high_resolution_clock::now();
                std::cout << "[Cartridge] GPUState init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tGpu1 - tGpu0).count()
                    << " ms\n";

                {
                    // The surface voice's terrain rows read THE
                    // TERRAIN_LOOKS PANEL ROW 2 (surface/
                    // terrain_looks.hpp) — the rest column lives where
                    // the parameters live. Values unchanged: blend -1
                    // = inactive, everything else 0.
                    gpuState_.set_band_motion(terrain_looks::REST_BAND_BLEND,
                        terrain_looks::REST_BAND_PHASE_ORIGIN);
                    gpuState_.set_terrain_time(terrain_looks::REST_TERRAIN_TIME);
                    gpuState_.set_mode_color_shift(terrain_looks::REST_MODE_COLOR_SHIFT);
                    gpuState_.set_mode_checker_scatter(terrain_looks::REST_MODE_CHECKER_SCATTER);
                    gpuState_.set_mode_palette_drift(terrain_looks::REST_MODE_PALETTE_DRIFT_TARGET,
```

visual_canvas TIDE/RAIN coupling sites (cartridge.hpp lines): [418, 424, 1463] — the re-aim landing zone; first site quoted:

`src/cartridges/the_board/cartridge.hpp:412-428`

```
                std::cout << "[Cartridge] GPUState init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tGpu1 - tGpu0).count()
                    << " ms\n";

                {
                    // The surface voice's terrain rows read THE
                    // TERRAIN_LOOKS PANEL ROW 2 (surface/
                    // terrain_looks.hpp) — the rest column lives where
                    // the parameters live. Values unchanged: blend -1
                    // = inactive, everything else 0.
                    gpuState_.set_band_motion(terrain_looks::REST_BAND_BLEND,
                        terrain_looks::REST_BAND_PHASE_ORIGIN);
                    gpuState_.set_terrain_time(terrain_looks::REST_TERRAIN_TIME);
                    gpuState_.set_mode_color_shift(terrain_looks::REST_MODE_COLOR_SHIFT);
                    gpuState_.set_mode_checker_scatter(terrain_looks::REST_MODE_CHECKER_SCATTER);
                    gpuState_.set_mode_palette_drift(terrain_looks::REST_MODE_PALETTE_DRIFT_TARGET,
                        terrain_looks::REST_MODE_PALETTE_DRIFT_INTENSITY,
```


### (d) the overlay retirement surface

Callers of terrain_wave_overlay_with_gradient (all occurrences incl. def/comments): lines [2800, 8056]. 
Comment-stripped code callers: write_live_card ONLY (verified below) — the Stage-6 swap retires the overlay whole.

`src/cartridges/the_board/realization/world.wgsl:1800-1875` — WAVE_THRESHOLD + OverlayWave + OVERLAY_WAVES (the retirement set tables)

```
// stays with the field fn at §1.6).
const ACTIVITY_LATTICE_SPACING: f32 = 400.0;    // world units between activity nodes
const ACTIVITY_BEAT_FREQ_LO: f32    = 0.25;     // lowest beat frequency (cycles/beat)
const ACTIVITY_BEAT_FREQ_HI: f32    = 2.0;      // highest beat frequency (cycles/beat)

// Per-band activity thresholds — which terrain bands respond at what intensity.
// Index matches TERRAIN_BANDS: 0=continental, 5=tectonic.
const WAVE_THRESHOLD = array<f32, 6>(
    0.85,  // 0: continental — only the most active pools move the bones
    0.70,  // 1: regional
    0.50,  // 2: local
    0.35,  // 3: detail
    0.20,  // 4: fine — responds to even mild pools
    0.90,  // 5: tectonic — almost geological, only extreme activity animates
);
const WAVE_THRESHOLD_SOFTNESS: f32 = 0.15;      // crossfade width at threshold boundary

// ─── Overlay Wave Design Matrix ─────────────────────────────────────
// (moved from the wave evaluators' side, §1.6; the two consumers —
//  contrib_terrain_waves_at / terrain_wave_overlay_with_gradient —
//  stay with the deformation machinery.)
//
//   amp        World-unit displacement at full blend.
//   freq       Spatial frequency (cycles per world unit). Higher = tighter ripples.
//   period     Temporal period in beats. One full sine cycle per this many beats.
//   direction  Propagation angle (radians). 0 = +X, π/2 = +Z. Negative = seed-derived.
//   amp_jit    Amplitude jitter range. Seed scales amp by (1 ± jit/2).
//   freq_jit   Frequency jitter range. Seed scales freq by (1 ± jit/2).

struct OverlayWave {
    amp: f32,
    freq: f32,
    period: f32,
    direction: f32,
    amp_jit: f32,
    freq_jit: f32,
}

const OVERLAY_WAVE_COUNT: u32 = 6u;

//                                amp    freq    period  dir     amp_jit  freq_jit
const OVERLAY_WAVES = array<OverlayWave, 6>(
    OverlayWave(                  0.12,  1.00,   3.0,   -1.0,   0.4,     0.4  ),  // 0: fine ripple
    OverlayWave(                  0.25,  0.50,   4.5,   -1.0,   0.4,     0.4  ),  // 1: detail
    OverlayWave(                  0.50,  0.25,   6.0,   -1.0,   0.4,     0.4  ),  // 2: local swell
    OverlayWave(                  1.00,  0.12,   9.0,   -1.0,   0.4,     0.4  ),  // 3: regional
    OverlayWave(                  2.00,  0.06,  13.0,   -1.0,   0.4,     0.4  ),  // 4: broad
    OverlayWave(                  3.50,  0.03,  18.0,   -1.0,   0.4,     0.4  ),  // 5: tectonic
);

// Overlay-wave seed properties (900-band — clear of entity 0–156 and
// wave-lattice 200–221; stride separates the six bands).
const OVERLAY_PROP_DIR_ANGLE: u32 = 900u;
const OVERLAY_PROP_FREQ_JIT:  u32 = 901u;
const OVERLAY_PROP_AMP_JIT:   u32 = 902u;
const OVERLAY_PROP_STRIDE:    u32 = 10u;

// ── ROW 8 — GOVERNING EXPRESSIONS ───────────────────────────────────
// The palette's governing expression lives in-room (below). The
// composite's governing contract — composite_cell_color(id, discrete):
// blend smooth→discrete at the ROW 5 mode edges; scatter-survive by
// cell_roll; mix the two styles by the transition field; sparse cells
// survive outside mode zones. Phase 1: the doors are FIELD outputs
// (door_values, fed from evaluate_cell_fields — the ONE derivation,
// bake and live alike since Commit C retired the LUT reconstruction;
// the biased twin deleted at Phase 2).

// Smooth palette color: weighted blend modulated by complexity only.
// No per-cell noise — produces continuous gradients. (Relocated from
// the field-function neighborhood — TERRAIN_LOOKS gather.)
fn palette_color_smooth(weights: vec4<f32>, complexity: f32) -> vec3<f32> {
    var color = vec3(0.0);
    let w = array<f32, 4>(weights.x, weights.y, weights.z, weights.w);
    for (var i: u32 = 0u; i < 4u; i++) {
        color += mix(config.palette_light[i].rgb, config.palette_center[i].rgb, complexity) * w[i];
    }
```

`src/cartridges/the_board/realization/world.wgsl:2767-2788` — overlay_band_params (dies with the table)

```
    freq: f32,
    amp:  f32,
}

fn overlay_band_params(i: u32, seed: u32) -> OverlayBandParams {
    let ow = OVERLAY_WAVES[i];

    // Direction: explicit angle or seed-derived when negative
    var angle: f32;
    if (ow.direction < 0.0) {
        angle = hash_property(seed, OVERLAY_PROP_DIR_ANGLE + i * OVERLAY_PROP_STRIDE) * 2.0 * PI;
    } else {
        angle = ow.direction;
    }
    let dir = vec2(cos(angle), sin(angle));

    // Seed-derived jitter on frequency and amplitude
    let freq = ow.freq * (1.0 + (hash_property(seed, OVERLAY_PROP_FREQ_JIT + i * OVERLAY_PROP_STRIDE) - 0.5) * ow.freq_jit);
    let amp  = ow.amp  * (1.0 + (hash_property(seed, OVERLAY_PROP_AMP_JIT + i * OVERLAY_PROP_STRIDE) - 0.5) * ow.amp_jit);

    return OverlayBandParams(dir, freq, amp);
}
```

`src/cartridges/the_board/realization/world.wgsl:615-627` — get_band_blend (band_blend reads — Stage 6 re-aims, does not retire)

```
// DRIVERLESS since gen-1 retirement — held at neutral by the boot
// block; revive via a gen-2 coupling or delete on the next pass here.
fn get_band_blend(band_index: u32) -> f32 {
    switch(band_index) {
        case 0u: { return config.band_blend_0; }
        case 1u: { return config.band_blend_1; }
        case 2u: { return config.band_blend_2; }
        case 3u: { return config.band_blend_3; }
        case 4u: { return config.band_blend_4; }
        case 5u: { return config.band_blend_5; }
        default: { return -1.0; }
    }
}
```

`src/cartridges/the_board/realization/world.wgsl:2796-2834` — the overlay evaluator itself

```
// Fused height + analytical gradient for the wave overlay.
// Returns vec3(height, dh/dx, dh/dz).
// Replaces the 5x finite-difference approach with 1x loop + analytical derivatives.
// Used by patch_terrain_vs where per-vertex cost dominates frame time.
fn terrain_wave_overlay_with_gradient(world_xz: vec2<f32>) -> vec3<f32> {
    if (config.terrain_time <= 0.0) { return vec3(0.0); }

    let seed = config.world_seed;
    var h: f32 = 0.0;
    var gx: f32 = 0.0;
    var gz: f32 = 0.0;

    for (var i: u32 = 0u; i < OVERLAY_WAVE_COUNT; i++) {
        let blend = get_band_blend(i);
        if (blend <= 0.0) { continue; }

        let ow = OVERLAY_WAVES[i];
        let origin = get_band_phase_origin(i);
        let t = config.terrain_time - origin;

        let bp = overlay_band_params(i, seed);

        let temporal = (2.0 * PI / ow.period) * t;
        let phase    = bp.freq * dot(bp.dir, world_xz) + temporal;

        // h  += B * A * sin(phase)
        // dh/dx = B * A * freq * dir.x * cos(phase)
        // dh/dz = B * A * freq * dir.y * cos(phase)
        let ba = blend * bp.amp;
        let s  = sin(phase);
        let c  = cos(phase);

        h  += ba * s;
        gx += ba * bp.freq * bp.dir.x * c;
        gz += ba * bp.freq * bp.dir.y * c;
    }

    return vec3(h, gx, gz);
}
```

§2.2 ROW 7 rows (TERRAIN_LOOKS panel lines mentioning ROW 7): [92, 477, 1616, 1662, 1792, 2752, 2761, 2763]


### (e) card scratch budget

g0:32 and g0:33 are FREE in both rooms (registry g0 jumps 31 → 60; zero `@group(0) @binding(32|33)` declarations) — the binding-number candidates verified. The writer layout today: 5 entries (2 uniform, 2 storage, 1 storage-texture). A 512² scratch adds 1 storage-texture write (pass 1) + 1 texture read (pass 2) — well inside both cc6 laws (8s/12u; storage-texture budget 4+ per stage) and the A3-5(i) probe validates the exact two-pass shape below.
## [A3-4] STAGE-7 FACTS — contact collision raw material — VERDICT: PASS


### (a) agent_tier_gains — both rooms + rows + upload

`src/cartridges/the_board/realization/state.hpp:634-660` — the C++ struct (32 B, static_asserts nearby)

```
        // Counts mirror the AGENT_BEHAVIOR_COUNT / AGENT_TIER_COUNT
        // enums in bodies/agents.hpp. Kept here so state.hpp can size
        // its registry buffers and bind-group entries without depending
        // on bodies/agents.hpp (which is included after state.hpp, at
        // file scope in the cartridge cohort). Asserts in
        // bodies/agents.hpp verify these stay in sync.
        static constexpr uint32_t GPU_AGENT_BEHAVIOR_COUNT = 10;
        static constexpr uint32_t GPU_AGENT_TIER_COUNT = 4;

        //
        // Field layout matches WGSL `struct AgentTierParams` exactly so
        // the WGSL side can read this buffer with the same struct shape
        // it had when AGENT_TIER_GAINS_WGSL was a const array literal.
        struct alignas(16) GPUAgentTierDef {
            float step_gain;       //  0 — multiplies behavior.step_size impulse
            float persist_gain;    //  4 — multiplies behavior.persistence (and home_pull)
            float speed_gain;      //  8 — multiplies behavior.speed_cap
            float color_r;         // 12 — vertex shader entity color (world.wgsl §6.3)
            float color_g;         // 16
            float color_b;         // 20
            float _pad[2];         // 24-31 — pad to 32 bytes (16-byte alignment)
        };                         // 32 total (16-byte aligned)

        struct alignas(16) GPUCameraState {
            float pos[3];
            float azimuth;
            float elevation;
```

`src/cartridges/the_board/bodies/agents.hpp:161-191` — the authored rows

```
    float       step_gain;       // multiplies step_size
    float       persist_gain;    // multiplies persistence
    float       speed_gain;      // multiplies speed_cap
    float       color_r;
    float       color_g;
    float       color_b;
};

inline constexpr AgentTierDef AGENT_TIER_GAINS[AGENT_TIER_COUNT] = {
    //  id                     name        step  persist  speed  color
    { AGENT_TIER_WORKER,   "worker",   1.0f, 1.0f, 1.0f, 0.60f, 0.62f, 0.65f },  // slate gray
    { AGENT_TIER_SCOUT,    "scout",    1.8f, 0.4f, 1.4f, 0.85f, 0.65f, 0.40f },  // bronze
    { AGENT_TIER_SENTINEL, "sentinel", 0.6f, 1.2f, 0.5f, 0.30f, 0.40f, 0.70f },  // deep blue
    { AGENT_TIER_LEADER,   "leader",   1.2f, 0.9f, 1.1f, 0.95f, 0.85f, 0.55f },  // pale gold
};

static_assert(sizeof(AGENT_TIER_GAINS) / sizeof(AGENT_TIER_GAINS[0]) == AGENT_TIER_COUNT,
              "AGENT_TIER_GAINS must declare one row per AgentTierId");

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════

struct AgentPopulationDef {
    uint32_t mood_id;
    uint32_t count;                                          // 0..Dim::MAX_AGENTS-1
    std::array<float, AGENT_BEHAVIOR_COUNT> behavior_weights;
    std::array<float, AGENT_TIER_COUNT>     tier_weights;
    float    spawn_inner_radius;                             // world units (annulus inner)
    float    spawn_radius;                                   // world units (annulus outer)
    float    home_seeding_radius;                            // world units from spawn point
};

```

`src/cartridges/the_board/realization/state.hpp:1783-1807` — the upload path

```
            // Called once at world-init from the cartridge — values are
            // constexpr-equivalent (sourced from AGENT_BEHAVIORS /
            // AGENT_TIER_GAINS in bodies/agents.hpp) and never change
            // during a session. Source data is passed as raw pointers
            // because the C++ tables are defined inside the cartridge
            // class scope and aren't visible from state.hpp; the cartridge
            // has both a translation step (CPU table → GPU struct) and
            // the queue access, so it owns the call site.
            void upload_agent_registries(wgpu::Queue& queue,
                const GPUAgentBehaviorDef* behaviors,
                uint32_t behavior_count,
                const GPUAgentTierDef* tiers,
                uint32_t tier_count) {
                writeArray(queue, agentBehaviorsBuffer_, behaviors, behavior_count);
                writeArray(queue, agentTierGainsBuffer_, tiers, tier_count);
            }

            void upload_config(wgpu::Queue& queue) {
                if (!configDirty_ && !configDynamic_) return;
                configDirty_ = false;
                writeStruct(queue, configBuffer_, config_);   // Shape A, dirty-driven — the canonical cadence
            }

            // Targeted 4-byte upload of pier_count only — called from write_pier/clear_pier.
            // Bypasses the config dirty flag since pier changes happen mid-frame during spawn.
```

`src/cartridges/the_board/realization/world.wgsl:821-855` — the WGSL mirror struct (field-for-field)

```
//     step_gain       — multiplies behavior.step_size impulse
//     persist_gain    — multiplies behavior.persistence (and home_pull)
//     speed_gain      — multiplies behavior.speed_cap
//     color_r/g/b     — vertex shader entity color

struct AgentBehaviorParams {
    step_rate:       f32,  // steps/beat (musical time)
    step_size:       f32,  // world units/step
    persistence:     f32,  // [0,1] biased-walk angle persistence
    drag:            f32,  // 1/s velocity decay
    home_pull:       f32,  // 1/s² spring toward home
    neighbor_radius: f32,  // flock neighbor search
    speed_cap:       f32,  // max speed
    _pad:            f32,  // pad to 32 bytes (matches GPUAgentBehaviorDef)
}

const AGENT_BEHAVIOR_COUNT_WGSL: u32 = 10u;

@group(0) @binding(110) var<uniform> agent_behaviors: array<AgentBehaviorParams, 10>;

struct AgentTierParams {
    step_gain:     f32,
    persist_gain:  f32,
    speed_gain:    f32,
    color_r:       f32,
    color_g:       f32,
    color_b:       f32,
    _pad0:         f32,  // pad to 32 bytes (matches GPUAgentTierDef)
    _pad1:         f32,
}

const AGENT_TIER_COUNT_WGSL: u32 = 4u;

@group(0) @binding(111) var<uniform> agent_tier_gains: array<AgentTierParams, 4>;

```


### (b) AgentState — the free pad float

`src/cartridges/the_board/realization/world.wgsl:762-792` — WGSL struct

```
struct AgentState {
    pos_x: f32,
    pos_y: f32,
    pos_z: f32,
    t: f32,         // reserved (per-agent local clock; padding to vec4)
    vel_x: f32,
    vel_y: f32,
    vel_z: f32,
    heading: f32,
    home_x: f32,
    home_y: f32,
    home_z: f32,
    seed: u32,
    behavior_id: u32,
    tier_idx: u32,
    is_active: u32,
    portal_trigger: i32,   // only meaningful on possessed slot; -1 = none
    orient_x: f32,
    orient_y: f32,
    orient_z: f32,
    orient_w: f32,
    color_r: f32,   // per-agent body color (palette pick at spawn; 0 = tier fallback)
    color_g: f32,
    color_b: f32,
    _pad0: f32,
}

// ═══ AGENT REGISTRIES (read-only uniform buffers) ═══════════════════════
//
// PAIRED DECLARATIONS — KEEP IN SYNC:
//   AgentBehaviorParams (WGSL, here)   ↔ GPUAgentBehaviorDef (C++, state.hpp)
```

`src/cartridges/the_board/realization/state.hpp:583-614` — C++ struct (the pad float + offset visible here)

```
        struct alignas(16) GPUAgentState {
            float pos_x;           //  0
            float pos_y;           //  4
            float pos_z;           //  8
            float t;               // 12 — personal clock
            float vel_x;           // 16
            float vel_y;           // 20
            float vel_z;           // 24
            float heading;         // 28
            float home_x;          // 32
            float home_y;          // 36
            float home_z;          // 40
            uint32_t seed;         // 44 — stable noise source
            uint32_t behavior_id;  // 48 — AgentBehaviorId (see bodies/agents.hpp)
            uint32_t tier_idx;     // 52 — AgentTierId     (see bodies/agents.hpp)
            uint32_t is_active;    // 56 — 0 = inactive (collapsed in VS + skipped in update)
            int32_t  portal_trigger; // 60 — only meaningful on the possessed slot (-1 = none)
            float orient_x;        // 64 — heading ⊗ tilt quaternion
            float orient_y;        // 68
            float orient_z;        // 72
            float orient_w;        // 76
            float color_r;         // 80 — per-agent body color (palette pick at spawn)
            float color_g;         // 84
            float color_b;         // 88
            float _pad0;           // 92
        };                         // 96 total

        // ─── Agent registry GPU structs ──────────────────────────────
        //
        // The C++ side is uploaded as a uniform buffer (bindings 110/111);
        // the WGSL side reads from those bindings. Field count, field
        // order, and total size MUST match exactly. A field-order
```


### (c) insertion points — the two agent kernels post-step

`src/cartridges/the_board/realization/world.wgsl:6851-6875` — update_player_agent — the gather loop lands after the manifold snap, before the write-back

```

// ─── Player kernel ───────────────────────────────────────────────
// Single thread, runs once per frame on config.possessed_slot.
// Contains the full walker policy (pawn_ground_resolve,
// terrain_normal_at, portal trigger).
@compute @workgroup_size(1)
fn update_player_agent() {
    if (!dynamics_0d_active()) { return; }

    let slot = config.possessed_slot;
    if (slot >= 32u) { return; }

    var agent = agent_state[slot];
    if (agent.is_active == 0u) { return; }

    // The player is always behavior 0 (PlayerControlled). Other
    // behavior ids in the possessed slot are a bug — treat as no-op.
    if (agent.behavior_id == 0u) {
        agent = behavior_player_controlled(agent);
    }

    // The player is never evicted — their slot is the reference
    // frame for eviction, not subject to it.
    agent_state[slot] = agent;
}
```

`src/cartridges/the_board/realization/world.wgsl:6878-6924` — update_other_agents — same seam; per-agent pos/tier_idx/active in registers

```
// 32 threads, one per slot. Skips the possessed slot (handled by
// update_player_agent). Runs algorithmic behaviors only — the heavy
// walker-policy path never inlines here.
@compute @workgroup_size(32)
fn update_other_agents(@builtin(global_invocation_id) gid: vec3<u32>) {
    if (!dynamics_0d_active()) { return; }

    let slot = gid.x;
    if (slot >= 32u) { return; }
    if (slot == config.possessed_slot) { return; }   // handled separately

    var agent = agent_state[slot];
    if (agent.is_active == 0u) { return; }

    switch agent.behavior_id {
        // Behavior 0 (PlayerControlled) should never appear in a
        // non-possessed slot. If it does (stale state, bug), treat
        // as no-op rather than run the heavy walker path from the
        // wrong kernel.
        case 0u: { /* no-op */ }
        case 1u: { agent = behavior_random_walk(agent); }
        case 2u: { agent = behavior_biased_walk(agent); }
        case 3u: { agent = behavior_wanderer(agent); }
        case 4u: { agent = behavior_home_seeker(agent); }
        case 5u: { agent = behavior_slow_patrol(agent); }
        case 6u: { agent = behavior_pursuit(agent); }
        case 7u: { agent = behavior_flee(agent); }
        case 8u: { agent = behavior_flock2d(agent); }
        case 9u: { agent = behavior_levy_flight(agent); }
        default: { /* unknown behavior — no-op */ }
    }

    // Point-centered eviction (was the possessed slot).
    // Non-player agents that wander too far from THE POINT are
    // deactivated; the CPU readback path detects them on the next
    // frame and respawns fresh agents in a disk around the point.
    // Pawn-host identical (the point IS the possessed slot's pos
    // there); in free-fly the population lives under the camera.
    let pp = point_pos();
    let dx = agent.pos_x - pp.x;
    let dz = agent.pos_z - pp.z;
    if (dx * dx + dz * dz > AGENT_EVICTION_RADIUS_SQ) {
        agent.is_active = 0u;
    }

    agent_state[slot] = agent;
}
```


### (d) the precedent loop — flock2d neighbor gather (FXC-safe bounded shape)

`src/cartridges/the_board/realization/world.wgsl:6655-6727`

```
// for those within neighbor_radius, accumulate (a) their position
// (centroid → cohesion) and (b) their velocity vector (heading →
// alignment). Blend agent's own velocity toward the average.
//
// Aesthetic: birds, fish, schools. The most visually striking
// behavior when populations are dense enough for emergent flow.
fn behavior_flock2d(agent_in: AgentState) -> AgentState {
    var a = agent_in;
    let dt        = signal.dt;

    let b = agent_behaviors[8u];
    let tier = min(a.tier_idx, AGENT_TIER_COUNT_WGSL - 1u);
    let g = agent_tier_gains[tier];

    // Step trigger — flock decisions are beat-gated.
    let s = step_trigger(b.step_rate);
    if (s.fired) {
        // Random direction noise (small — alignment dominates).
        let theta = hash_property(a.seed, 7400u + s.step_idx) * 6.28318530718;
        let noise_arc = (1.0 - clamp(b.persistence * g.persist_gain, 0.0, 1.0)) * 0.78;
        let noisy_theta = theta * noise_arc;
        let impulse_n = b.step_size * g.step_gain * 0.15;
        a.vel_x += cos(noisy_theta) * impulse_n;
        a.vel_z += sin(noisy_theta) * impulse_n;

        // Sample 4 neighbors for cohesion + alignment.
        // Wider sample improves chance of finding flock mates with
        // small populations.
        var cx = 0.0;
        var cz = 0.0;
        var ax = 0.0;
        var az = 0.0;
        var n  = 0u;
        for (var k = 0u; k < 4u; k = k + 1u) {
            let other_slot = (a.seed + s.step_idx * 31u + k * 7919u) % 32u;
            if (other_slot == config.possessed_slot) { continue; }
            let other = agent_state[other_slot];
            if (other.is_active == 0u) { continue; }
            let odx = other.pos_x - a.pos_x;
            let odz = other.pos_z - a.pos_z;
            let od2 = odx * odx + odz * odz;
            if (od2 < b.neighbor_radius * b.neighbor_radius && od2 > 0.001) {
                cx = cx + other.pos_x;
                cz = cz + other.pos_z;
                ax = ax + other.vel_x;
                az = az + other.vel_z;
                n = n + 1u;
            }
        }
        if (n > 0u) {
            let inv_n = 1.0 / f32(n);
            // Cohesion pull toward centroid.
            let centroid_x = cx * inv_n;
            let centroid_z = cz * inv_n;
            let pdx = centroid_x - a.pos_x;
            let pdz = centroid_z - a.pos_z;
            let plen = sqrt(pdx * pdx + pdz * pdz);
            if (plen > 0.001) {
                let cohesion = b.step_size * g.step_gain * 0.5;
                a.vel_x = a.vel_x + (pdx / plen) * cohesion;
                a.vel_z = a.vel_z + (pdz / plen) * cohesion;
            }
            // Alignment — strongly blend toward average velocity.
            let avg_vx = ax * inv_n;
            let avg_vz = az * inv_n;
            let align_strength = 0.75;
            a.vel_x = mix(a.vel_x, avg_vx, align_strength);
            a.vel_z = mix(a.vel_z, avg_vz, align_strength);
        }
    }

    return agent_post_step(a, b.drag, b.speed_cap, g.speed_gain);
}
```


### (e) floaters — radii + candidate insertion points

`src/cartridges/the_board/realization/world.wgsl:2078-2095` — pawn forcefield radii constants

```
const GOL_PULSE_ALGORITHM_CHANCE: f32 = 0.35;

// --- Pawn Safety Force Field
const PAWN_FORCEFIELD_ENABLED: bool = true;

// --- Compile-time feature gates
// These prune heavy dependency chains from update_player_agent's pipeline compilation.
// Set to false to cut compile time when iterating on unrelated features.
const PAWN_GOL_GROUND_ENABLED: bool = false;    // Pawn walks on GoL extrusions
const PAWN_FORCEFIELD_RADIUS_STATIONARY: f32 = 6.0;  // Radius when not moving
const PAWN_FORCEFIELD_RADIUS_MOVING: f32 = 2.0;      // Radius at max speed
const PAWN_FORCEFIELD_FALLOFF: f32 = 2.0;            // Edge softness (smoothstep width)
const PAWN_FORCEFIELD_SPEED_SCALE: f32 = 1.0;        // How quickly radius shrinks with speed


// §2.3 MUTING CONTROL

// --- Coupling bit flags
```

`src/cartridges/the_board/realization/state.hpp:296-301` — Idle:: sphere constants (C++ room)

```
            constexpr float SPHERE_ORBIT_RADIUS = 25.0f;
            constexpr float SPHERE_ORBIT_SPEED = 0.3f;
            constexpr float SPHERE_HOVER_HEIGHT = 8.0f;
            constexpr float SPHERE_INFLUENCE_RADIUS = 12.0f;
            constexpr float WAVE_TIME_SCALE = 1.0f;
            constexpr float PAWN_SPEED = 15.0f;
```

`src/cartridges/the_board/realization/world.wgsl:7038-7088` — update_sphere head (insertion: after its ground resolve, before write-back — full body runs to L7092)

```
    camera_state = camera;
}

@compute @workgroup_size(1)
fn update_sphere() {
    if (!dynamics_0d_active()) { return; }

    let dt = signal.dt;
    let point_xz = point_pos().xz;

    // Update sphere slots only (orbital motion)
    for (var slot = 0u; slot < SPHERE_SLOT_COUNT; slot++) {
        var fe = floating_entities.entities[slot];
        if (fe.is_active == 0u) { continue; }

        // Lifecycle: point-distance eviction (was the pawn —
        // floaters follow the point, Jean's ruling). A sphere stays
        // alive within FLOATER_EVICTION_RADIUS of THE POINT.
        // Patch eviction no longer touches floaters (commit path skips
        // entity_refs for sphere/cube), so this is the sole death path.
        let to_point = fe.pos.xz - point_xz;
        if (dot(to_point, to_point) > FLOATER_EVICTION_RADIUS_SQ) {
            floating_entities.entities[slot].is_active = 0u;
            continue;
        }

        if (!sphere_frozen()) {
            fe.t = fe.t + dt;

            // Orbit: PGA motor around anchor
            let updated = compose_sphere_from_orbit_pga(fe.t, fe);
            fe.pos = updated.pos;
            fe.orientation = updated.orientation;

            floating_entities.entities[slot] = fe;
        }

        if (signal_active() && coupling_active(COUPLING_POLYPHONY_TO_SPHERE_COLOR)) {
            // DRIVERLESS (M1-C): raw signal.stats[0] substituted with the
            // neutral 0.0 — color rests at base_color.
            floating_entities.entities[slot].color = coupling_signal_polyphony_to_sphere_color(
                0.0,
                floating_entities.entities[slot].color,
                floating_entities.entities[slot].base_color,
                dt
            );
        }
    }

    // RAYMARCH/SDF EXCAVATION: the terrain-tint dead store removed. This
    // whole nearest-sphere search wrote only terrain_state.tint (the dead
```

`src/cartridges/the_board/realization/world.wgsl:7189-7239` — update_cube head (same seam; full body to L7371)

```
    }
}

@compute @workgroup_size(1)
fn update_cube() {
    if (!dynamics_0d_active()) { return; }

    let dt = signal.dt;
    let point_xz = point_pos().xz;

    // Update cube slots — drift integrator on top of analytical home.
    //
    // Decomposition:
    //   home  = analytical rest position (anchor.xz, ground + orbit_height + bob)
    //   pos   = home + drift
    //
    // drift integrates spring-to-zero plus per-slot behavior forces.
    // Stationary baseline: behavior force is zero. With drift and
    // drift_vel starting at zero, the spring sees no error, the
    // integrator adds nothing, and pos == home every frame — exact
    // visual parity with the pre-substrate hover-bob. Future behaviors
    // (CurlField, PhaseWave, …) push drift around without touching
    // the analytical home.
    let cube_end = CUBE_SLOT_OFFSET + CUBE_SLOT_COUNT;
    for (var slot = CUBE_SLOT_OFFSET; slot < cube_end; slot++) {
        var fe = floating_entities.entities[slot];
        if (fe.is_active == 0u) { continue; }

        // Lifecycle: point-distance eviction (was the pawn —
        // floaters follow the point). Cube stays alive as long as its
        // current position (home + drift) is within range of THE
        // POINT. Patch eviction no longer touches cubes — see the
        // matching test in update_sphere for the lifecycle rationale.
        let to_point = fe.pos.xz - point_xz;
        if (dot(to_point, to_point) > FLOATER_EVICTION_RADIUS_SQ) {
            floating_entities.entities[slot].is_active = 0u;
            continue;
        }

        if (!sphere_frozen()) {
            fe.t = fe.t + dt;

            // ── Kite-release transition ───────────────────────────
            // If follow_pawn == 2u, the CPU requested kite-mode release:
            // freeze the cube's CURRENT world xz as the new anchor,
            // zero drift, and switch to anchor mode (follow_pawn = 0).
            // This guarantees the cube's visible position is preserved
            // exactly across the toggle — anchor-mode home re-derives
            // ground at the new anchor.xz, drift = 0, so pos = home,
            // and the cube stays where the player saw it.
            //
```


### (f) dispatch order truth (the seven dispatches as coded)

`src/cartridges/the_board/realization/render_passes.hpp:170-224` — ribbon → player → others → camera → sphere → cube → vp; campaign v2 §7.9 one-frame asymmetry cites THIS order

```
inline void dispatch_compute(MachineCtx* c, wgpu::CommandEncoder& encoder) {
    wgpu::ComputePassDescriptor desc{};
    desc.label = "Compute Phase";
    wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);

    if (c->ribbon_state_.rendered_slot != UINT32_MAX) {
        c->renderer_.dispatch_compute_ribbon_rings(
            compute,
            c->gpuState_.ribbon_compute_group(),
            GPUState::ribbon_ring_workgroups()
        );
    }

    // RAYMARCH/SDF EXCAVATION: dispatch_update_terrain_config removed (dead
    // writer of the TerrainState buffer — no reader).

    c->renderer_.dispatch_update_player_agent(
        compute,
        c->gpuState_.compute_entity_group(),
        c->gpuState_.compute_texture_group()   // aura + sampler for POLICY_WALKER
    );

    c->renderer_.dispatch_update_other_agents(
        compute,
        c->gpuState_.compute_entity_group(),
        c->gpuState_.compute_texture_group()   // aura + sampler for POLICY_WALKER_AGENT
    );

    c->renderer_.dispatch_update_camera(
        compute,
        c->gpuState_.compute_entity_group(),
        c->gpuState_.compute_texture_group()   // aura + sampler for POLICY_FLYER
    );

    c->renderer_.dispatch_update_sphere(
        compute,
        c->gpuState_.compute_entity_group(),
        c->gpuState_.compute_texture_group()   // aura + sampler for POLICY_FLYER
    );

    c->renderer_.dispatch_update_cube(
        compute,
        c->gpuState_.compute_entity_group(),
        c->gpuState_.compute_texture_group()   // aura + sampler for POLICY_FLYER
    );

    c->renderer_.dispatch_compute_vp(
        compute,
        c->gpuState_.compute_entity_group()
    );

    compute.End();
}

inline void dispatch_frustum_cull(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
```


### (g) mass authority

No pawn-mass constant exists anywhere in the agent rooms (tree-wide grep: the only `mass` fields are the FLOATER vocabulary columns — tier0..2_mass_mult in the floater tier table, world.wgsl ~10956/11027+ — a different population). The "pawn heavy" ratio is authored FRESH. RECOMMENDED HOME per the panel law: a `mass` (or `contact_mass`) column in AGENT_TIER_GAINS — the authored table already mirrored field-for-field in both rooms with static_assert size pins and an upload path (section (a)); population_themes carries THEME palette values, not physics scalars. Radius likewise: a `contact_radius` column beside it (the same insertion shape (a) documents).

---

## [A3-5] SHAPE PROBES — VERDICT: PASS (timings tabled; nothing near the 10 s flag)

Witness Dawn (headless Chromium = real Dawn; **Tint/Vulkan on
SwiftShader — the FXC word stays with Jean's machine**). Probe source +
raw output live on `UNIFIED_GROUND_1_A3_P1` (`audit/probe_shapes_a3.mjs`
+ `audit/probe_shapes_a3_results.json`).

| Probe | Shape | module | pipeline(s) | Messages |
|---|---|---|---|---|
| (i) two-pass card writer | pass 1: 4-band delta heights → 512² storage scratch; pass 2: scratch central-difference neighborhood → `vec4(h, gx, gz, a)` rgba16float store — the bake's two-pass pattern at card size | 2.2 ms | 12.5 / 14.3 ms | none |
| (ii) true-band delta kernel | Σ bands 0..3 of (moving − frozen): bounded band × 3×3-node loops, per-node hash/dir/damp, frozen & moving phases, band_act mix — the lattice-wave evaluator's loop shape | 1.5 ms | 17.9 ms | none |
| (iii) contact gather | 33-slot uniform-bounded pair loop, tier-table radius source, distance test + capped-spring accumulate, mass-ratio weighting — flock2d's shape | 1.2 ms | 22.6 ms | none |

FLAG check (>10 s): nothing flagged — all three shapes are 2–3 orders
of magnitude under the line on Tint.

**Bonus catch (the pushed WGSL spec earning its keep):** the first
contact-probe draft named an agent field `active` — Tint rejects it:
**`active` is a reserved WGSL keyword**. The live AgentState struct does
not use it, but Stage-7's new columns/fields must not either (the probe
renamed to `alive`). Recorded so the Stage-7 handoff author avoids the
trap.

---

## [A3-6] THE FEATURE LINE — VERDICT: PENDING

No Chrome `wgslLanguageFeatures` line from either Windows session has
been supplied. **PENDING** — blocks nothing in Stages 6–7. (Container
observation stands as recorded in AUDIT-2: SwiftShader/Tint exposes
`readonly_and_readwrite_storage_textures`; the design machine's line is
the one that decides paint-card residency.)

---

## Verdict summary

| Section | Verdict |
|---|---|
| A3-1 baseline pin | **PASS** — all instruments byte-identical; witness ALL GREEN |
| A3-2 parked-number ledger | **PASS** — zero stragglers; bookkeeping closed |
| A3-3 Stage-6 facts | **PASS** — band machinery, bake two-pass, wires, overlay retirement set (write_live_card is the sole overlay caller), scratch budget verified (g0:32/33 free) |
| A3-4 Stage-7 facts | **PASS** — tier table, AgentState pad, insertion seams, flock2d precedent, floaters, dispatch order, mass authority (authored fresh; tier-column home recommended) |
| A3-5 shape probes | **PASS** — all three validate in 12–23 ms; `active` reserved-keyword trap recorded |
| A3-6 feature line | **PENDING** (blocks nothing) |

Stage 6 (the true-band swap) and Stage 7 (contact collision) are
handoff-ready on this material.
