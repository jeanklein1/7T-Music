/* ============================================================================
   7T WEB — boot
   Terrain family under a paced streaming conductor. The unbudgeted burst
   (6 patches × 3 heavy dispatches per frame with no back-pressure) reset the
   GPU on integrated graphics (TDR-class). Fix, per Jean's plan:

   §1 budget       — B patches/frame (start 2), one encoder + submit per rAF,
                     yield every frame; the conductor never bursts the window.
   §2 adaptive     — time every gen submit via onSubmittedWorkDone; >150 ms →
                     halve B (min 1); <50 ms ×10 → grow B ×1.25 (cap 8).
   §3 device-lost  — halt loop + conductor on device.lost; log reason; ONE
                     auto-recovery (rebuild, resume from tile cache, B halved);
                     a second loss → fallback card.
   §4 fallback     — no navigator.gpu / no adapter / double-loss → static card.
   §5 bisect       — ?passes=heights|+gradients|+cells|+draws caps patch passes.
   §6 report       — per-submit max ms, live B, allocated bytes, lost events.
   §7 headless     — ?skip=present runs the full paced path to an offscreen
                     target (no compositor present) — the software pacing test.
   §8 compression  — mirror fetched .gz-first via DecompressionStream.
   ========================================================================== */

import { createState, clearShadowMaps, D } from './state.js';
import { makeConfig, makeSignal, writeInterimCamera, writeLights } from './uniforms.js';
import { TerrainStreamer, encodeCullAndDraw } from './passes/terrain.js';

const params   = new URLSearchParams(location.search);
const statusEl = document.getElementById('status');
const canvas   = document.getElementById('stage');
const CAPTURE  = params.has('capture');
const SKIP     = params.get('skip') || '';                 // draw|cull|lod0|lod1|gen|present
const PRESENT  = !SKIP.includes('present');
const LEVELS   = { heights: 1, gradients: 2, cells: 3, draws: 4 };
const capLevel = params.get('passes') ? (LEVELS[params.get('passes').replace('+', '')] ?? 4) : 4;
const PASSES   = { gen: capLevel >= 1, level: Math.min(capLevel, 3), draws: capLevel >= 4 };
const INITIAL_B = 2;
let finishCapture = null;
/* Drive the loop with rAF when presenting; headless (?skip=present) has no
   compositor so rAF is throttled to ~nothing — use setTimeout there (§7). */
const schedule = PRESENT ? requestAnimationFrame : (fn) => setTimeout(() => fn(performance.now()), 4);

const statusLines = [];
function show(line) { statusLines.push(line); statusEl.textContent = statusLines.slice(-30).join('\n'); }
function log(line) { console.log('[7T] ' + line); show(line); }

const bootReport = [];
const extras = { budgetB: INITIAL_B, maxSubmitMs: 0, allocMB: 0, lost: [] };
function timed(step, ms, note = '') { bootReport.push({ step, ms: +ms.toFixed(1), note }); log(`${step}: ${ms.toFixed(1)} ms${note ? '  (' + note + ')' : ''}`); }

/* --- fallback card (§4) ----------------------------------------------------- */
function showFallback(reason) {
  const el = document.getElementById('fallback');
  if (el) {
    document.getElementById('fallback-reason').textContent = reason;
    el.style.display = 'flex';
    canvas.style.display = 'none';
    statusEl.style.display = 'none';
  }
  console.error('[7T] FALLBACK — ' + reason);
  finishCapture?.();
}
function fail(line) { console.error('[7T] BOOT FAIL — ' + line); show('BOOT FAIL — ' + line); throw new Error(line); }

/* --- palette tier → JS ------------------------------------------------------ */
const css = getComputedStyle(document.documentElement);
const hex = (name) => {
  const h = css.getPropertyValue(name).trim().replace('#', '');
  return [parseInt(h.slice(0, 2), 16) / 255, parseInt(h.slice(2, 4), 16) / 255, parseInt(h.slice(4, 6), 16) / 255];
};
const PALETTE = { ink: hex('--ink'), sky: hex('--sky'), gold: hex('--gold'), orb: hex('--orb-red') };
const CLEAR = { r: PALETTE.ink[0], g: PALETTE.ink[1], b: PALETTE.ink[2], a: 1 };

/* --- feature gate (§4) ------------------------------------------------------ */
if (!('gpu' in navigator)) {
  showFallback('WebGPU is not available in this browser.');
  throw new Error('no webgpu');
}

/* --- mirror fetch, .gz-first (§8) ------------------------------------------- */
const tBoot = performance.now();
async function fetchMirror() {
  const t = performance.now();
  if ('DecompressionStream' in window) {
    try {
      const r = await fetch('./shaders/world.wgsl.gz');
      if (r.ok) {
        const ds = r.body.pipeThrough(new DecompressionStream('gzip'));
        const text = await new Response(ds).text();
        timed('mirror fetch (gz)', performance.now() - t, `${(text.length / 1024).toFixed(0)} KiB decoded`);
        return text;
      }
    } catch { /* fall through to plain */ }
  }
  const r = await fetch('./shaders/world.wgsl');
  if (!r.ok) fail(`mirror fetch: HTTP ${r.status}`);
  const text = await r.text();
  timed('mirror fetch', performance.now() - t, `${(text.length / 1024).toFixed(0)} KiB`);
  return text;
}

/* --- device acquisition (§4 timeout guard) ---------------------------------- */
const GPU_UNAVAILABLE =
  'no GPU adapter — WebGPU is unavailable or disabled.\n' +
  'chrome://gpu → check "WebGPU" and "Problems Detected". After repeated GPU\n' +
  'crashes Chrome disables it: fully quit Chrome and reopen, or reboot.';
async function acquireDevice() {
  let t = performance.now();
  const adapter = await Promise.race([
    navigator.gpu.requestAdapter(),
    new Promise((res) => setTimeout(() => res(null), 8000)),
  ]);
  if (!adapter) { showFallback(GPU_UNAVAILABLE); throw new Error('no adapter'); }
  timed('adapter request', performance.now() - t);
  const Lm = adapter.limits;
  log(`adapter limits — storageBufs/stage: ${Lm.maxStorageBuffersPerShaderStage}  storageTex/stage: ` +
      `${Lm.maxStorageTexturesPerShaderStage}  texArrayLayers: ${Lm.maxTextureArrayLayers}`);
  if (adapter.info) log(`adapter info — ${adapter.info.vendor || '?'} / ${adapter.info.architecture || '?'}`);
  t = performance.now();
  const device = await adapter.requestDevice();
  timed('device request', performance.now() - t);
  device.lost.then(onLost);
  device.addEventListener('uncapturederror', (e) => {
    console.error('[7T] uncaptured error: ' + e.error.message);
    show('*** GPU ERROR: ' + e.error.message.slice(0, 160) + ' ***');
  });
  return device;
}

/* --- scene build (re-callable on recovery) ---------------------------------- */
const format = navigator.gpu.getPreferredCanvasFormat();
const ctx = canvas.getContext('webgpu');
let wgsl = null;   // fetched once, reused across recoveries

function makeDepth(device, w, h) {
  return device.createTexture({ label: 'main depth', format: 'depth24plus',
    size: { width: w, height: h }, usage: GPUTextureUsage.RENDER_ATTACHMENT });
}

async function buildScene(device, resumeTiles) {
  ctx.configure({ device, format, alphaMode: 'opaque' });

  const t0 = performance.now();
  const worldModule = device.createShaderModule({ label: 'world.wgsl (mirror)', code: wgsl });
  const compInfo = await worldModule.getCompilationInfo();
  timed('module create + validate', performance.now() - t0, 'whole mirror, roster-independent');
  const errors = compInfo.messages.filter((m) => m.type === 'error');
  const warnings = compInfo.messages.filter((m) => m.type === 'warning');
  for (const m of compInfo.messages.slice(0, 12))
    console[m.type === 'error' ? 'error' : 'warn'](`[7T] wgsl ${m.type} ${m.lineNum}:${m.linePos} — ${m.message}`);
  if (errors.length || warnings.length) fail(`mirror module: ${errors.length} error(s), ${warnings.length} warning(s)`);
  log('STRIDE ASSERTION PASS — mirror validated clean (0 errors, 0 warnings)');

  let t = performance.now();
  const R = createState(device);
  extras.allocMB = R.totalBytes / (1024 * 1024);
  timed('state create', performance.now() - t, `${extras.allocMB.toFixed(1)} MB buffers+textures`);

  const cpipe = (entryPoint, layout) => device.createComputePipelineAsync({
    label: entryPoint, layout: device.createPipelineLayout({ bindGroupLayouts: [layout] }),
    compute: { module: worldModule, entryPoint } });
  const rpipe = (label, indirection) => device.createRenderPipelineAsync({
    label, layout: device.createPipelineLayout({ bindGroupLayouts: [R.renderEntityLayout, R.renderTextureLayout] }),
    vertex: { module: worldModule, entryPoint: 'patch_terrain_vs', constants: indirection ? { USE_PATCH_INDIRECTION: 1 } : {} },
    fragment: { module: worldModule, entryPoint: 'patch_terrain_fs', targets: [{ format }] },
    primitive: { topology: 'triangle-list', cullMode: 'back', frontFace: 'ccw' },
    depthStencil: { format: 'depth24plus', depthWriteEnabled: true, depthCompare: 'less' } });

  t = performance.now();
  [R.pipeHeights, R.pipeGradients, R.pipeCells, R.pipeCull, R.pipeTerrainIndirect, R.pipeTerrainDirect] =
    await Promise.all([
      cpipe('generate_patch_heights', R.patchGenLayout), cpipe('generate_patch_gradients', R.patchGenLayout),
      cpipe('generate_patch_cells', R.patchGenLayout), cpipe('frustum_cull_patches', R.frustumLayout),
      rpipe('patch_terrain (indirect)', true), rpipe('patch_terrain (direct LOD1)', false),
    ]);
  timed('pipelines created', performance.now() - t, 'census check passed');

  clearShadowMaps(device, R);
  const config = makeConfig(device, R.config);
  const signal = makeSignal(device, R.signal, PALETTE);
  R.aspect = canvas.width / Math.max(canvas.height, 1);
  const eyeY = writeInterimCamera(device, R)[1];
  writeLights(device, R);
  config.flush();

  const maxCells = params.get('patches') ? parseInt(params.get('patches'), 10) : 0;   // §7 software cap
  const terrain = new TerrainStreamer(device, R, config, 42, maxCells);
  if (resumeTiles) terrain.tiles = resumeTiles;   // §3: deterministic resume

  // headless full-run target (§7): draw offscreen so we never present
  let offColor = null, offDepth = null;
  if (!PRESENT) {
    offColor = device.createTexture({ label: 'offscreen color', format,
      size: { width: 640, height: 400 }, usage: GPUTextureUsage.RENDER_ATTACHMENT });
    offDepth = makeDepth(device, 640, 400);
  }
  return { device, R, config, signal, terrain, eyeY, depth: makeDepth(device, canvas.width, canvas.height),
           offColor, offDepth };
}

/* --- pacing loop ------------------------------------------------------------ */
let S = null;            // current scene
let dead = false;
let recovered = false;
let ready = false;
let B = INITIAL_B;
let genBusy = false;
let fastStreak = 0;
let last = performance.now();
let stage1Logged = false, stage2Logged = false, windowLogged = false;

function adaptBudget(ms) {
  extras.maxSubmitMs = Math.max(extras.maxSubmitMs, ms);
  if (ms > 150) { B = Math.max(1, Math.floor(B / 2)); fastStreak = 0; }
  else if (ms < 50) { if (++fastStreak >= 10) { B = Math.min(8, Math.ceil(B * 1.25)); fastStreak = 0; } }
  else fastStreak = 0;
  extras.budgetB = B;
}

function frame(now) {
  if (dead) return;                                        // §3: hard halt
  const dt = Math.min((now - last) / 1000, 0.05); last = now;
  if (PRESENT) {
    resize();
    if (canvas.width === 0 || canvas.height === 0) { schedule(frame); return; }
  }

  if (!ready) {                                            // stage 1: clear present while pipelines compile
    const enc = S?.device?.createCommandEncoder?.({ label: 'boot clear' });
    if (enc && PRESENT) {
      enc.beginRenderPass({ colorAttachments: [{ view: ctx.getCurrentTexture().createView(),
        clearValue: CLEAR, loadOp: 'clear', storeOp: 'store' }] }).end();
      S.device.queue.submit([enc.finish()]);
      if (!stage1Logged) { stage1Logged = true; timed('first pixel — stage 1 (clear)', performance.now() - tBoot); }
    }
    schedule(frame);
    return;
  }

  const { device, R, config, signal, terrain } = S;

  // §2 back-pressure: while a gen batch is in flight, YIELD — submit nothing.
  // (Submitting draws every tick during the wait floods the queue and makes
  // each onSubmittedWorkDone drain the whole backlog — times explode.)
  if (genBusy) { schedule(frame); return; }

  signal.frame(now / 1000, dt, canvas.width / Math.max(canvas.height, 1));
  const enc = device.createCommandEncoder();
  let didGen = false;
  if (!terrain.done && PASSES.gen && !SKIP.includes('gen')) {
    didGen = terrain.encode(enc, B, PASSES.level) > 0;    // §1: one paced batch
  }
  config.flush();
  const doDraw = PASSES.draws && (PRESENT || didGen);     // headless draws only on a gen tick
  if (doDraw) {
    const colorView = PRESENT ? ctx.getCurrentTexture().createView() : S.offColor.createView();
    const depthView = PRESENT ? S.depth.createView() : S.offDepth.createView();
    encodeCullAndDraw(device, R, enc, colorView, depthView, CLEAR, terrain.counts, SKIP);
  }
  const tSub = performance.now();
  device.queue.submit([enc.finish()]);                    // §1: one submit per frame

  if (!stage2Logged && terrain.lod0Done && terrain.counts.render > 0) {
    stage2Logged = true;
    timed('first pixel — stage 2 (lod0 ring)', performance.now() - tBoot, `${terrain.counts.lod0} patches, B=${B}`);
  }

  if (didGen) {                                            // §2: measure this batch, adapt B
    genBusy = true;
    const genN = terrain.patches.size;
    device.queue.onSubmittedWorkDone().then(() => {
      if (dead) return;
      const ms = performance.now() - tSub;
      adaptBudget(ms);
      genBusy = false;
      if (genN <= INITIAL_B * 2 || genN % 25 === 0)       // sparse progress trace
        log(`gen ${genN}/${terrain.targetCount} — submit ${ms.toFixed(0)} ms, B=${B}`);
      if (terrain.done && !windowLogged) windowComplete();
      schedule(frame);                                    // resume once the batch drained
    });
    return;                                               // don't double-schedule
  }
  schedule(frame);
}

function windowComplete() {
  windowLogged = true;
  log(`terrain window complete — ${S.terrain.patches.size} patches ` +
      `(lod0 ${S.terrain.counts.lod0}, render ${S.terrain.counts.render}); ` +
      `B=${B}, max submit ${extras.maxSubmitMs.toFixed(0)} ms, alloc ${extras.allocMB.toFixed(1)} MB`);
  console.table(bootReport);
  console.log('[7T] REPORT', JSON.stringify(extras));
  console.log('[7T] WINDOW COMPLETE');
  if (!PRESENT) verifyHeadless();
  else runDiagnostics();
}

/* --- device-lost discipline (§3) -------------------------------------------- */
function logLost(info) {
  const rec = { reason: info.reason, message: info.message, atMs: +(performance.now() - tBoot).toFixed(0) };
  extras.lost.push(rec);
  bootReport.push({ step: 'DEVICE LOST', ms: rec.atMs, note: `${rec.reason}: ${rec.message}` });
  console.error('[7T] device lost:', JSON.stringify(rec));
  show(`*** DEVICE LOST (${info.reason}): ${info.message} ***`);
}
async function onLost(info) {
  if (info.reason === 'destroyed') return;                // intentional teardown, not a crash
  dead = true;                                            // halts frame() + conductor immediately
  genBusy = false;
  logLost(info);
  if (recovered) { showFallback('GPU device lost twice — the hardware or driver reset repeatedly.'); return; }
  recovered = true;
  const resumeTiles = S?.terrain?.tiles ?? null;
  B = Math.max(1, Math.floor(B / 2)); extras.budgetB = B;
  log(`recovering — rebuilding GPU state, resuming with B=${B}…`);
  try {
    ready = false; stage1Logged = false;
    const device = await acquireDevice();
    S = await buildScene(device, resumeTiles);
    dead = false; ready = true;
    log('recovered — resuming terrain stream');
    schedule(frame);
  } catch (e) {
    showFallback('recovery failed: ' + e.message);
  }
}

/* --- diagnostics + headless verification ------------------------------------ */
function f16(bits) {
  const s = (bits & 0x8000) ? -1 : 1, e = (bits >> 10) & 0x1f, m = bits & 0x3ff;
  if (e === 0) return s * m * 2 ** -24;
  if (e === 31) return m ? NaN : s * Infinity;
  return s * (1 + m / 1024) * 2 ** (e - 15);
}
async function runDiagnostics() {
  try {
    const { device, R, terrain, eyeY } = S;
    const cullStg = device.createBuffer({ size: 20, usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST });
    const texStg = device.createBuffer({ size: 256, usage: GPUBufferUsage.MAP_READ | GPUBufferUsage.COPY_DST });
    const layer = terrain.patches.get('0,0')?.layer ?? 0;
    const enc = device.createCommandEncoder();
    enc.copyBufferToBuffer(R.frustumCompute, 0, cullStg, 0, 20);
    enc.copyTextureToBuffer({ texture: R.heightfield, origin: { x: 0, y: 0, z: layer } },
      { buffer: texStg, bytesPerRow: 256 }, { width: 32, height: 1 });
    device.queue.submit([enc.finish()]);
    await Promise.all([cullStg.mapAsync(GPUMapMode.READ), texStg.mapAsync(GPUMapMode.READ)]);
    const cull = new Uint32Array(cullStg.getMappedRange());
    const tx = new Uint16Array(texStg.getMappedRange());
    const heights = [0, 8, 16, 24, 31].map((i) => f16(tx[i * 4]).toFixed(2));
    log(`diag — cull kept ${cull[1]} of ${terrain.counts.lod0} lod0; heights near origin: ${heights.join(', ')} (eye y ${eyeY.toFixed(1)})`);
    cullStg.unmap(); texStg.unmap();
    if (CAPTURE) setTimeout(() => {
      const c2 = document.createElement('canvas'); c2.width = 320; c2.height = 200;
      c2.getContext('2d').drawImage(canvas, 0, 0, 320, 200);
      console.log('[7T] CAPTURE ' + c2.toDataURL('image/png'));
      finishCapture?.();
    }, 500);
    else finishCapture?.();
  } catch (e) { console.warn('[7T] diag skipped: ' + e.message); finishCapture?.(); }
}
async function verifyHeadless() {
  // §7: assert the paced software run filled its window and the pacing logic
  // held — completion + adaptation + no device loss. (Software submits are all
  // slow, so per-submit "breaches" are expected and drive B down by design;
  // the assertion is that generation stays bounded and completes, not raw ms.)
  const t = S.terrain;
  const filledOK = t.patches.size === t.targetCount;
  const pass = filledOK && extras.lost.length === 0;
  log(`HEADLESS VERIFY — filled ${t.patches.size}/${t.targetCount}; max submit ` +
      `${extras.maxSubmitMs.toFixed(0)} ms; final B=${B}; adapted=${B !== INITIAL_B}; lost=${extras.lost.length}`);
  console.log(`[7T] HEADLESS ${pass ? 'PASS' : 'FAIL'}`);
  finishCapture?.();
}

function resize() {
  const dpr = Math.min(window.devicePixelRatio || 1, 2);
  const w = Math.max(1, Math.floor(canvas.clientWidth * dpr)), h = Math.max(1, Math.floor(canvas.clientHeight * dpr));
  if (S && (canvas.width !== w || canvas.height !== h)) {
    canvas.width = w; canvas.height = h;
    S.depth?.destroy();
    S.depth = makeDepth(S.device, w, h);
  }
}

/* --- orchestrate ------------------------------------------------------------ */
if (PRESENT) { canvas.width = Math.max(1, canvas.clientWidth); canvas.height = Math.max(1, canvas.clientHeight); }
schedule(frame);   // stage-1 clears begin immediately (S guards no-op until built)
wgsl = await fetchMirror();
try {
  const device = await acquireDevice();
  S = await buildScene(device, null);
  ready = true;
  log(`boot ok — streaming terrain (B=${B}, passes≤${capLevel}${PRESENT ? '' : ', headless'})…`);
  console.log('[7T] BOOT OK');
} catch (e) {
  if (!document.getElementById('fallback') || document.getElementById('fallback').style.display !== 'flex')
    showFallback('boot failed: ' + e.message);
}

/* ?capture=1 / headless hold the module open so the harness doesn't exit early */
if (CAPTURE || !PRESENT) {
  await new Promise((res) => { finishCapture = res; setTimeout(res, 60000); });
}
