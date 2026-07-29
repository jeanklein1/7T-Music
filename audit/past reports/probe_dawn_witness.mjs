// **ARCHIVAL** — this document describes the tree as of its own
// date and is superseded. Do not cite as current. Live facts live
// in the code; adjudications live in `audit/` and
// `docs/HANDOFFS/OPTIMIZATION 1/`.

// [CC-3 / CC-4 / CC-5] Dawn validation witness, run through headless Chromium.
//
// Chromium's WebGPU implementation IS Dawn, so createComputePipeline errors
// here are genuine Dawn validation output ("each names the entry points still
// referencing the binding"). This harness reconstructs the app's bind group
// layouts from audit/cc6_output.json (machine-parsed from
// realization/state.hpp, including registry-resolved binding numbers and
// texture view dimensions), then:
//
//   BASELINE  : full layouts -> every compute pipeline must succeed.
//   PROBE A   : remove the three claimed-dead entries (ribbon tile_grid+
//               pier_instances; GoL pier_instances; pawn-aura tile_grid) and
//               recreate each family's pipelines.
//   PROBE B   : Compute Entity layout minus ONE evictee at a time
//               (30, 25, 26, 160, 161 — spec order; restored between
//               sub-probes) -> create all six CE-family pipelines, capture
//               verbatim Dawn errors. patch_grid (152) NOT probed (spec).
//   PROBE C   : standalone storage-texture kernels (read_write r32float,
//               ping-pong pair core+feature forms, rgba16float writer) with
//               module + pipeline creation timing.
//
// DELTA vs the handoff spec: this is Dawn on Vulkan/SwiftShader (Linux), not
// the project's native Windows D3D12/FXC device, and it witnesses pipeline
// creation, not a full app boot. The C++ side of each probe is gated by the
// tree's own glaw1 (audit/tools/glaw1/run.sh), which DOES run here.
//
// Usage: node audit/probe_dawn_witness.mjs   (from repo root)
import { createRequire } from 'node:module';
import { readFileSync, writeFileSync } from 'node:fs';
import http from 'node:http';

const require = createRequire(
  process.env.PW_DIR ?? '/tmp/claude-0/-home-user-7T-Pawns/d9fc9340-1bed-5e06-bc4e-4b8dd2c215b7/scratchpad/node_modules/'
);
const { chromium } = require('playwright-core');

const wgsl = readFileSync('src/cartridges/the_board/realization/world.wgsl', 'utf8');
const cc6 = JSON.parse(readFileSync('audit/cc6_output.json', 'utf8'));

// Pipeline families at CLOSURE_GPU HEAD (renderer.hpp createComputePipelines).
// groups = ordered bind group layout labels for the pipeline layout.
const LIVE = ['update_player_agent', 'update_other_agents', 'update_camera', 'update_sphere', 'update_cube'];
const FAMILIES = {
  live_contrib: { groups: ['Compute Entity Layout', 'Compute Texture Layout'], eps: LIVE },
  compute_vp: { groups: ['Compute Entity Layout'], eps: ['compute_vp'] },
  terrain_index_gen: { groups: ['Terrain Index Gen Layout'], eps: ['generate_terrain_indices'] },
  patch_gen: { groups: ['Patch Gen Layout'], eps: ['generate_patch_heights', 'generate_patch_gradients', 'generate_patch_cells'] },
  ribbon: { groups: ['Ribbon Compute Layout'], eps: ['compute_ribbon_rings'] },
  photographer: { groups: ['Photographer Compute Layout'], eps: ['compute_photographer_vp'] },
  entity_placement: { groups: ['Entity Placement Compute Layout'], eps: ['compute_entity_placement'] },
  frustum_cull: { groups: ['Frustum Cull Compute Layout'], eps: ['frustum_cull_patches'] },
  pawn_aura: { groups: ['Pawn Aura Compute Layout'], eps: ['compute_pawn_aura'] },
  orb: { groups: ['Orb Compute Layout'], eps: ['orb_init', 'orb_dynamics', 'orb_recolor'] },
  orb_copy: { groups: ['Orb Copy Layout'], eps: ['orb_state_prev_copy'] },
  gol_zone: { groups: ['GoL Zone Compute Layout'], eps: ['zone_gol_sync', 'zone_gol_evolve', 'zone_gol_mesh_reset', 'zone_gol_mesh_gen', 'zone_derive_params'] },
  arch_mesh: { groups: ['Arch Mesh Gen Layout'], eps: ['arch_mesh_gen'] },
  column_mesh: { groups: ['Column Mesh Gen Layout'], eps: ['column_mesh_gen'] },
  palm_mesh: { groups: ['Palm Mesh Gen Layout'], eps: ['palm_mesh_gen'] },
  cactus_mesh: { groups: ['Cactus Mesh Gen Layout'], eps: ['cactus_mesh_gen'] },
  blade_mesh: { groups: ['Blade Mesh Gen Layout'], eps: ['blade_cluster_mesh_gen'] },
};

const PROBE_C_KERNELS = {
  rw_r32float: `
    requires readonly_and_readwrite_storage_textures;
    @group(0) @binding(0) var card: texture_storage_2d<r32float, read_write>;
    @compute @workgroup_size(8, 8)
    fn probe_rw(@builtin(global_invocation_id) gid: vec3<u32>) {
      let v = textureLoad(card, vec2<i32>(gid.xy));
      textureStore(card, vec2<i32>(gid.xy), vec4<f32>(v.x + 1.0, 0.0, 0.0, 0.0));
    }`,
  pingpong_core: `
    @group(0) @binding(0) var src: texture_2d<f32>;
    @group(0) @binding(1) var dst: texture_storage_2d<r32float, write>;
    @compute @workgroup_size(8, 8)
    fn probe_pingpong(@builtin(global_invocation_id) gid: vec3<u32>) {
      let v = textureLoad(src, vec2<i32>(gid.xy), 0);
      textureStore(dst, vec2<i32>(gid.xy), vec4<f32>(v.x * 0.5 + 0.1, 0.0, 0.0, 0.0));
    }`,
  pingpong_feature: `
    requires readonly_and_readwrite_storage_textures;
    @group(0) @binding(0) var src: texture_storage_2d<r32float, read>;
    @group(0) @binding(1) var dst: texture_storage_2d<r32float, write>;
    @compute @workgroup_size(8, 8)
    fn probe_pingpong_ro(@builtin(global_invocation_id) gid: vec3<u32>) {
      let v = textureLoad(src, vec2<i32>(gid.xy));
      textureStore(dst, vec2<i32>(gid.xy), vec4<f32>(v.x * 0.5 + 0.1, 0.0, 0.0, 0.0));
    }`,
  rgba16f_writer: `
    @group(0) @binding(0) var card: texture_storage_2d<rgba16float, write>;
    @compute @workgroup_size(8, 8)
    fn probe_writer(@builtin(global_invocation_id) gid: vec3<u32>) {
      textureStore(card, vec2<i32>(gid.xy), vec4<f32>(0.25, 0.5, 0.75, 1.0));
    }`,
};

const server = http.createServer((_req, res) => {
  res.setHeader('content-type', 'text/html');
  res.end('<!doctype html><title>dawn-witness</title><body>ok</body>');
});
await new Promise(r => server.listen(0, '127.0.0.1', r));
const port = server.address().port;

const browser = await chromium.launch({
  executablePath: '/opt/pw-browsers/chromium',
  args: ['--no-sandbox', '--enable-unsafe-webgpu', '--enable-features=Vulkan',
         '--use-webgpu-adapter=swiftshader', '--disable-gpu-sandbox'],
});
const page = await browser.newPage();
await page.goto(`http://127.0.0.1:${port}/`);

const result = await page.evaluate(async ({ wgsl, layouts, families, probeC }) => {
  const out = { env: {}, module: {}, baseline: {}, probeA: {}, probeB: {}, probeC: {} };
  const adapter = await navigator.gpu.requestAdapter();
  out.env.adapterInfo = { vendor: adapter.info?.vendor, architecture: adapter.info?.architecture };
  out.env.wgslLanguageFeatures = [...navigator.gpu.wgslLanguageFeatures];
  out.env.adapterLimits = {
    maxStorageBuffersPerShaderStage: adapter.limits.maxStorageBuffersPerShaderStage,
    maxUniformBuffersPerShaderStage: adapter.limits.maxUniformBuffersPerShaderStage,
    maxStorageTexturesPerShaderStage: adapter.limits.maxStorageTexturesPerShaderStage,
  };
  // Mirror console.hpp: request full adapter capacity.
  const device = await adapter.requestDevice({
    requiredLimits: {
      maxStorageBuffersPerShaderStage: adapter.limits.maxStorageBuffersPerShaderStage,
      maxStorageTexturesPerShaderStage: adapter.limits.maxStorageTexturesPerShaderStage,
      maxSampledTexturesPerShaderStage: adapter.limits.maxSampledTexturesPerShaderStage,
      maxUniformBuffersPerShaderStage: adapter.limits.maxUniformBuffersPerShaderStage,
      maxBindingsPerBindGroup: adapter.limits.maxBindingsPerBindGroup,
    },
  });

  const VIS = { Compute: GPUShaderStage.COMPUTE, Vertex: GPUShaderStage.VERTEX, Fragment: GPUShaderStage.FRAGMENT };
  const BUF = { Storage: 'storage', ReadOnlyStorage: 'read-only-storage', Uniform: 'uniform' };
  const SAMP = { Filtering: 'filtering', NonFiltering: 'non-filtering', Comparison: 'comparison' };
  const TEX = { Float: 'float', UnfilterableFloat: 'unfilterable-float', Depth: 'depth', Uint: 'uint', Sint: 'sint' };
  const FMT = { RGBA16Float: 'rgba16float', RGBA8Unorm: 'rgba8unorm', RG32Float: 'rg32float', R32Float: 'r32float', RGBA32Float: 'rgba32float' };

  function layoutDesc(lay, removeSet = new Set()) {
    const entries = [];
    for (const e of lay.entries) {
      if (removeSet.has(e.binding)) continue;
      const ent = { binding: e.binding, visibility: e.stages.reduce((a, s) => a | VIS[s], 0) };
      if (e.class === 'storage_buffer' || e.class === 'uniform_buffer' || e.class === 'other_buffer') {
        ent.buffer = { type: BUF[e.detail] ?? 'uniform' };
      } else if (e.class === 'sampler') {
        ent.sampler = { type: SAMP[e.detail] ?? 'filtering' };
      } else if (e.class === 'sampled_texture') {
        ent.texture = { sampleType: TEX[e.detail] ?? 'float', viewDimension: e.viewDimension ?? '2d' };
      } else if (e.class === 'storage_texture') {
        ent.storageTexture = { access: 'write-only', format: FMT[e.detail], viewDimension: e.viewDimension ?? '2d' };
      }
      entries.push(ent);
    }
    return { entries };
  }

  const t0 = performance.now();
  const module = device.createShaderModule({ code: wgsl, label: 'world.wgsl (audit witness)' });
  const info = await module.getCompilationInfo();
  out.module.create_plus_info_ms = +(performance.now() - t0).toFixed(1);
  out.module.messages = info.messages.map(m => `${m.type} L${m.lineNum}: ${m.message}`);

  const layByLabel = Object.fromEntries(layouts.map(l => [l.label, l]));

  // removals: { label: [bindings...] } applied to the named group's layout.
  async function tryPipelines(tag, fam, removals, bucket) {
    const rec = { groups: fam.groups, removed: removals, results: {} };
    let plo;
    try {
      const bgls = fam.groups.map(label =>
        device.createBindGroupLayout(layoutDesc(layByLabel[label], new Set(removals[label] ?? []))));
      plo = device.createPipelineLayout({ bindGroupLayouts: bgls });
    } catch (e) {
      rec.layout_error = String(e);
      bucket[tag] = rec;
      return;
    }
    for (const ep of fam.eps) {
      const t = performance.now();
      try {
        await device.createComputePipelineAsync({ layout: plo, compute: { module, entryPoint: ep } });
        rec.results[ep] = { ok: true, ms: +(performance.now() - t).toFixed(1) };
      } catch (e) {
        rec.results[ep] = { ok: false, error: e.message ?? String(e) };
      }
    }
    bucket[tag] = rec;
  }

  // BASELINE — full layouts, every family.
  for (const [name, fam] of Object.entries(families)) {
    await tryPipelines(name, fam, {}, out.baseline);
  }

  // PROBE A — the three claimed-dead removals (spec CC-3).
  await tryPipelines('A1_ribbon_minus_tile_grid_25_pier_26', families.ribbon,
    { 'Ribbon Compute Layout': [25, 26] }, out.probeA);
  await tryPipelines('A2_gol_minus_pier_26', families.gol_zone,
    { 'GoL Zone Compute Layout': [26] }, out.probeA);
  await tryPipelines('A3_aura_minus_tile_grid_25', families.pawn_aura,
    { 'Pawn Aura Compute Layout': [25] }, out.probeA);
  // Machine-found dead entries beyond the plan (removable-clean checks):
  await tryPipelines('A4_CE_minus_101_145_146_152', families.live_contrib,
    { 'Compute Entity Layout': [101, 145, 146, 152] }, out.probeA);
  await tryPipelines('A4b_vp_CE_minus_101_145_146_152', families.compute_vp,
    { 'Compute Entity Layout': [101, 145, 146, 152] }, out.probeA);
  await tryPipelines('A5_CT_minus_23', families.live_contrib,
    { 'Compute Texture Layout': [23] }, out.probeA);
  await tryPipelines('A6_photographer_minus_144', families.photographer,
    { 'Photographer Compute Layout': [144] }, out.probeA);
  await tryPipelines('A7_placement_minus_60_144', families.entity_placement,
    { 'Entity Placement Compute Layout': [60, 144] }, out.probeA);
  await tryPipelines('A8_frustum_minus_60_80', families.frustum_cull,
    { 'Frustum Cull Compute Layout': [60, 80] }, out.probeA);

  // PROBE B — Compute Entity minus one evictee at a time (spec order;
  // both CE-family pipeline layouts affected: live_contrib + compute_vp).
  const evictees = [['pyramid_instances', 30], ['tile_grid', 25], ['pier_instances', 26],
                    ['zone_config', 160], ['zone_life', 161]];
  for (const [name, b] of evictees) {
    await tryPipelines(`B_${name}_minus_${b}`, families.live_contrib,
      { 'Compute Entity Layout': [b] }, out.probeB);
    await tryPipelines(`B_${name}_minus_${b}_vp`, families.compute_vp,
      { 'Compute Entity Layout': [b] }, out.probeB);
  }

  // PROBE C — standalone storage-texture kernels with creation timing.
  const hasRW = out.env.wgslLanguageFeatures.includes('readonly_and_readwrite_storage_textures');
  for (const [name, code] of Object.entries(probeC)) {
    if ((name === 'rw_r32float' || name === 'pingpong_feature') && !hasRW) {
      out.probeC[name] = { skipped: 'readonly_and_readwrite_storage_textures not exposed' };
      continue;
    }
    const r = {};
    try {
      let t = performance.now();
      const m = device.createShaderModule({ code });
      const ci = await m.getCompilationInfo();
      r.module_ms = +(performance.now() - t).toFixed(1);
      r.messages = ci.messages.map(msg => `${msg.type}: ${msg.message}`);
      const entries = name === 'rw_r32float'
        ? [{ binding: 0, visibility: GPUShaderStage.COMPUTE, storageTexture: { access: 'read-write', format: 'r32float', viewDimension: '2d' } }]
        : name === 'pingpong_core'
          ? [{ binding: 0, visibility: GPUShaderStage.COMPUTE, texture: { sampleType: 'unfilterable-float', viewDimension: '2d' } },
             { binding: 1, visibility: GPUShaderStage.COMPUTE, storageTexture: { access: 'write-only', format: 'r32float', viewDimension: '2d' } }]
          : name === 'pingpong_feature'
            ? [{ binding: 0, visibility: GPUShaderStage.COMPUTE, storageTexture: { access: 'read-only', format: 'r32float', viewDimension: '2d' } },
               { binding: 1, visibility: GPUShaderStage.COMPUTE, storageTexture: { access: 'write-only', format: 'r32float', viewDimension: '2d' } }]
            : [{ binding: 0, visibility: GPUShaderStage.COMPUTE, storageTexture: { access: 'write-only', format: 'rgba16float', viewDimension: '2d' } }];
      const plo = device.createPipelineLayout({ bindGroupLayouts: [device.createBindGroupLayout({ entries })] });
      const epName = { rw_r32float: 'probe_rw', pingpong_core: 'probe_pingpong', pingpong_feature: 'probe_pingpong_ro', rgba16f_writer: 'probe_writer' }[name];
      const t2 = performance.now();
      await device.createComputePipelineAsync({ layout: plo, compute: { module: m, entryPoint: epName } });
      r.pipeline_ms = +(performance.now() - t2).toFixed(1);
      r.ok = true;
    } catch (e) {
      r.ok = false;
      r.error = e.message ?? String(e);
    }
    out.probeC[name] = r;
  }

  return out;
}, { wgsl, layouts: cc6.layouts, families: FAMILIES, probeC: PROBE_C_KERNELS });

writeFileSync('audit/probe_results.json', JSON.stringify(result, null, 2));
console.log(JSON.stringify({
  module_ms: result.module.create_plus_info_ms,
  module_messages: result.module.messages,
  baseline_failures: Object.entries(result.baseline).flatMap(([k, v]) =>
    Object.entries(v.results ?? {}).filter(([, r]) => !r.ok).map(([ep]) => `${k}:${ep}`)),
}, null, 2));
console.log('full results -> audit/probe_results.json');
await browser.close();
server.close();
