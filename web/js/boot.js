/* ============================================================================
   7T WEB — boot
   Terrain family live: mirror module (validated clean = PORT_MAP §5 stride
   assertion) → explicit layouts from the §2c census → async pipeline creation
   (each timed, boot report is cumulative) → streaming conductor → frustum
   cull → LOD0-indirect + LOD1-direct terrain draw under an interim fixed
   camera (camera/vp is next in lift order).

   Boot staging: stage 1 first pixel = clear (pre-terrain), stage 2 = first
   frame with terrain draws. SwiftShader timings are rehearsal; Jean's Chrome
   numbers are what PORT_MAP records.
   ========================================================================== */

import { createState, clearShadowMaps, D } from './state.js';
import { makeConfig, makeSignal, writeInterimCamera, writeLights } from './uniforms.js';
import { TerrainStreamer, encodeCullAndDraw } from './passes/terrain.js';

const statusEl = document.getElementById('status');
const canvas   = document.getElementById('stage');
const CAPTURE  = new URLSearchParams(location.search).has('capture');
const SKIP     = new URLSearchParams(location.search).get('skip') || '';   // dev bisect: draw|cull|lod0|lod1
let finishCapture = null;

const statusLines = [];
function show(line) {
  statusLines.push(line);
  statusEl.textContent = statusLines.slice(-28).join('\n');
}
function log(line) { console.log('[7T] ' + line); show(line); }
function fail(line) {
  console.error('[7T] BOOT FAIL — ' + line);
  show('BOOT FAIL — ' + line);
  throw new Error(line);
}

const bootReport = [];
function timed(step, ms, note = '') {
  bootReport.push({ step, ms: +ms.toFixed(1), note });
  log(`${step}: ${ms.toFixed(1)} ms${note ? '  (' + note + ')' : ''}`);
}

/* --- palette tier → JS ------------------------------------------------------ */
const css = getComputedStyle(document.documentElement);
const hex = (name) => {
  const h = css.getPropertyValue(name).trim().replace('#', '');
  return [parseInt(h.slice(0, 2), 16) / 255, parseInt(h.slice(2, 4), 16) / 255, parseInt(h.slice(4, 6), 16) / 255];
};
const PALETTE = { ink: hex('--ink'), sky: hex('--sky'), gold: hex('--gold'), orb: hex('--orb-red') };

/* --- device ----------------------------------------------------------------- */
if (!('gpu' in navigator)) fail('WebGPU not available. Open in Chrome/Edge over http(s) or localhost.');

const tBoot = performance.now();
let t0 = performance.now();
const adapter = await navigator.gpu.requestAdapter();
if (!adapter) fail('no GPU adapter');
timed('adapter request', performance.now() - t0);

const L = adapter.limits;
log(`adapter limits — storageBufs/stage: ${L.maxStorageBuffersPerShaderStage}` +
    `  storageTex/stage: ${L.maxStorageTexturesPerShaderStage}` +
    `  texArrayLayers: ${L.maxTextureArrayLayers}  bindings/group: ${L.maxBindingsPerBindGroup}`);
if (adapter.info) log(`adapter info — ${adapter.info.vendor || '?'} / ${adapter.info.architecture || '?'}`);

/* Census §2c: default limits suffice (≤8 storage/stage; 225 ≤ 256 tex layers). */
t0 = performance.now();
const device = await adapter.requestDevice();
timed('device request', performance.now() - t0);
/* Surface GPU failures in the on-page overlay too — a lost device otherwise
   looks like a silently blank canvas while the JS keeps narrating. */
device.lost.then((info) => {
  console.error('[7T] device lost: ' + info.message);
  show('*** DEVICE LOST (' + info.reason + '): ' + info.message + ' ***');
});
device.addEventListener('uncapturederror', (e) => {
  console.error('[7T] uncaptured error: ' + e.error.message);
  show('*** GPU ERROR: ' + e.error.message.slice(0, 160) + ' ***');
});

/* --- surface ----------------------------------------------------------------- */
const ctx    = canvas.getContext('webgpu');
const format = navigator.gpu.getPreferredCanvasFormat();
ctx.configure({ device, format, alphaMode: 'opaque' });
log(`surface configured — format: ${format}`);

let depthTex = null;
function resize() {
  const dpr = Math.min(window.devicePixelRatio || 1, 2);
  const w = Math.max(1, Math.floor(canvas.clientWidth * dpr));
  const h = Math.max(1, Math.floor(canvas.clientHeight * dpr));
  if (canvas.width !== w || canvas.height !== h) {
    canvas.width = w; canvas.height = h;
    depthTex?.destroy();
    depthTex = device.createTexture({
      label: 'main depth', format: 'depth24plus',
      size: { width: w, height: h },
      usage: GPUTextureUsage.RENDER_ATTACHMENT,
    });
  }
}
window.addEventListener('resize', resize);
resize();

/* Present clear frames IMMEDIATELY — staged boot stage 1. The canvas must
   never sit configured-but-unpresented while pipelines compile (8 s on an
   iGPU): the first present after a long gap is exactly where the device
   loss lands on both test machines. `frame` is hoisted; its full path is
   gated on `ready`, armed at the end of init below. */
let ready = false;
let firstClearLogged = false;
let last = performance.now();
let firstTerrainFrame = true;
let streamed = 0;
/* ?skip=present defers all presents until after init — validation-only smoke
   for environments whose compositor kills the instance on first present. */
if (!SKIP.includes('present')) requestAnimationFrame(frame);

/* --- mirror module (the stride assertion) ------------------------------------ */
t0 = performance.now();
const resp = await fetch('./shaders/world.wgsl');
if (!resp.ok) fail(`mirror fetch: HTTP ${resp.status}`);
const wgsl = await resp.text();
timed('mirror fetch', performance.now() - t0, `${(wgsl.length / 1024).toFixed(0)} KiB`);

t0 = performance.now();
const worldModule = device.createShaderModule({ label: 'world.wgsl (mirror)', code: wgsl });
const compInfo = await worldModule.getCompilationInfo();
timed('module create + validate', performance.now() - t0, 'whole mirror, roster-independent');
const errors = compInfo.messages.filter((m) => m.type === 'error');
const warnings = compInfo.messages.filter((m) => m.type === 'warning');
for (const m of compInfo.messages.slice(0, 12)) {
  console[m.type === 'error' ? 'error' : 'warn'](`[7T] wgsl ${m.type} ${m.lineNum}:${m.linePos} — ${m.message}`);
}
if (errors.length || warnings.length) {
  fail(`mirror module: ${errors.length} error(s), ${warnings.length} warning(s) — 0-warning standard`);
}
log('STRIDE ASSERTION PASS — mirror validated clean (0 errors, 0 warnings)');

/* --- state + pipelines (async; creation IS the mechanical census check) ------ */
t0 = performance.now();
const R = createState(device);
timed('state create', performance.now() - t0, 'buffers/textures/layouts/bind groups');

async function computePipe(name, entryPoint, layout) {
  const t = performance.now();
  const p = await device.createComputePipelineAsync({
    label: entryPoint, layout: device.createPipelineLayout({ bindGroupLayouts: [layout] }),
    compute: { module: worldModule, entryPoint },
  });
  timed(`pipeline ${entryPoint}`, performance.now() - t);
  return p;
}
async function terrainRenderPipe(label, indirection) {
  const t = performance.now();
  const p = await device.createRenderPipelineAsync({
    label,
    layout: device.createPipelineLayout({ bindGroupLayouts: [R.renderEntityLayout, R.renderTextureLayout] }),
    vertex: {
      module: worldModule, entryPoint: 'patch_terrain_vs',
      constants: indirection ? { USE_PATCH_INDIRECTION: 1 } : {},
    },
    fragment: { module: worldModule, entryPoint: 'patch_terrain_fs', targets: [{ format }] },
    primitive: { topology: 'triangle-list', cullMode: 'back', frontFace: 'ccw' },
    depthStencil: { format: 'depth24plus', depthWriteEnabled: true, depthCompare: 'less' },
  });
  timed(`pipeline ${label}`, performance.now() - t);
  return p;
}

[R.pipeHeights, R.pipeGradients, R.pipeCells, R.pipeCull, R.pipeTerrainIndirect, R.pipeTerrainDirect] =
  await Promise.all([
    computePipe('heights', 'generate_patch_heights', R.patchGenLayout),
    computePipe('gradients', 'generate_patch_gradients', R.patchGenLayout),
    computePipe('cells', 'generate_patch_cells', R.patchGenLayout),
    computePipe('cull', 'frustum_cull_patches', R.frustumLayout),
    terrainRenderPipe('patch_terrain (indirect)', true),
    terrainRenderPipe('patch_terrain (direct LOD1)', false),
  ]);
log('terrain pipelines created — census check passed (explicit layouts validated)');

/* --- init writes -------------------------------------------------------------- */
clearShadowMaps(device, R);
const config = makeConfig(device, R.config);
const signal = makeSignal(device, R.signal, PALETTE);
R.aspect = canvas.width / Math.max(canvas.height, 1);
const EYE_Y = writeInterimCamera(device, R)[1];
writeLights(device, R);
config.flush();

const terrain = new TerrainStreamer(device, R, config);

ready = true;   // stage 2 arms: next frame streams and draws terrain
log('boot ok — streaming terrain…');
console.log('[7T] BOOT OK');
if (SKIP.includes('present')) requestAnimationFrame(frame);

/* --- diagnostics (dev; runs once when the window completes) -------------------
   Reads back (a) the frustum-cull instance count — a broken VP culls everything,
   and (b) heightfield texels around the origin — tells us if the interim camera
   sits inside the terrain. f16 decode is inline (rgba16float texels). */
function f16(bits) {
  const s = (bits & 0x8000) ? -1 : 1, e = (bits >> 10) & 0x1f, m = bits & 0x3ff;
  if (e === 0) return s * m * 2 ** -24;
  if (e === 31) return m ? NaN : s * Infinity;
  return s * (1 + m / 1024) * 2 ** (e - 15);
}
async function runDiagnostics() {
  try {
    await runDiagnosticsInner();
  } catch (e) {
    console.warn('[7T] diag skipped: ' + e.message);
    finishCapture?.();
  }
}
async function runDiagnosticsInner() {
  const cullStg = device.createBuffer({ label: 'diag cull', size: 20, usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST });
  const texStg  = device.createBuffer({ label: 'diag tex', size: 256, usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST });
  const layer = terrain.patches.get('0,0')?.layer ?? 0;
  const enc = device.createCommandEncoder({ label: 'diag' });
  enc.copyBufferToBuffer(R.frustumCompute, 0, cullStg, 0, 20);
  enc.copyTextureToBuffer(
    { texture: R.heightfield, origin: { x: 0, y: 0, z: layer } },
    { buffer: texStg, bytesPerRow: 256 }, { width: 32, height: 1 });
  device.queue.submit([enc.finish()]);
  await Promise.all([cullStg.mapAsync(GPUMapMode.READ), texStg.mapAsync(GPUMapMode.READ)]);
  const cull = new Uint32Array(cullStg.getMappedRange());
  const tx = new Uint16Array(texStg.getMappedRange());
  const heights = [0, 8, 16, 24, 31].map((i) => f16(tx[i * 4]).toFixed(2));
  log(`diag — cull kept ${cull[1]} of ${terrain.counts.lod0} lod0; ` +
      `heights near origin: ${heights.join(', ')} (eye y ${EYE_Y.toFixed(1)})`);
  cullStg.unmap(); texStg.unmap();
  // dev capture (?capture=1): downscaled canvas → data URL in the console, so
  // the headless harness (or a bug report) carries the actual pixels
  if (CAPTURE) {
    setTimeout(() => {
      const c2 = document.createElement('canvas');
      c2.width = 320; c2.height = 200;
      c2.getContext('2d').drawImage(canvas, 0, 0, 320, 200);
      console.log('[7T] CAPTURE ' + c2.toDataURL('image/png'));
      finishCapture?.();
    }, 800);
  }
}

/* --- frame -------------------------------------------------------------------- */
if (SKIP.includes('gen')) setTimeout(runDiagnostics, 4000);   // dev: probe device survival with no work submitted

function frame(now) {
  const dt = Math.min((now - last) / 1000, 0.05); last = now;
  resize();
  if (canvas.width === 0 || canvas.height === 0) { requestAnimationFrame(frame); return; }

  if (!ready) {
    // stage 1: color-only clear present while the mirror compiles
    const enc = device.createCommandEncoder({ label: 'boot clear' });
    enc.beginRenderPass({
      colorAttachments: [{
        view: ctx.getCurrentTexture().createView(),
        clearValue: { r: PALETTE.ink[0], g: PALETTE.ink[1], b: PALETTE.ink[2], a: 1 },
        loadOp: 'clear', storeOp: 'store',
      }],
    }).end();
    device.queue.submit([enc.finish()]);
    if (!firstClearLogged) {
      firstClearLogged = true;
      timed('time to first pixel — stage 1 (clear)', performance.now() - tBoot);
    }
    requestAnimationFrame(frame);
    return;
  }

  signal.frame(now / 1000, dt, canvas.width / Math.max(canvas.height, 1));

  const enc = device.createCommandEncoder();
  const n = SKIP.includes('gen') ? 0 : terrain.encode(enc);   // stream_patches: params copy + 3 dispatches per patch
  config.flush();                          // placement_patch_count / lod_point after banding
  encodeCullAndDraw(device, R, enc,
    ctx.getCurrentTexture().createView(), depthTex.createView(),
    { r: PALETTE.ink[0], g: PALETTE.ink[1], b: PALETTE.ink[2], a: 1 },
    terrain.counts, SKIP);
  device.queue.submit([enc.finish()]);

  if (n > 0) {
    streamed += n;
    if (streamed >= terrain.patches.size && terrain.pending.length === 0) {
      log(`terrain window complete — ${streamed} patches (lod0 ${terrain.counts.lod0}, ` +
          `render ${terrain.counts.render}, all ${terrain.counts.all})`);
      runDiagnostics();
    }
  }
  if (firstTerrainFrame && terrain.counts.render > 0) {
    firstTerrainFrame = false;
    timed('time to first pixel — stage 2 (terrain)', performance.now() - tBoot);
    console.table(bootReport);
    console.log('[7T] TERRAIN DRAWING');
  }
  requestAnimationFrame(frame);
}

/* ?capture=1 holds the module (and so the page load event) open until the
   canvas capture has been logged — the headless harness exits on load, and
   an early exit tears the device down mid-diagnostics. Normal loads skip
   this entirely. */
if (CAPTURE) {
  await new Promise((res) => {
    finishCapture = res;
    setTimeout(res, 60000);   // safety: never hold load forever
  });
}
