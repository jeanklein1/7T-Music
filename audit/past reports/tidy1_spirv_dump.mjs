// TIDY_1 T1 GATE — dump Tint SPIR-V for the 5 live/contact kernels of a given
// world.wgsl via Dawn dump_shaders (--in-process-gpu). Associates each of the 5
// SPIR-V blocks by its `OpEntryPoint GLCompute %<kernel>` name. Writes one
// normalized .spv.txt per kernel. Diff pre/post = the re-expression proof.
// Usage: node spirv_dump.mjs <world.wgsl> <cc6.json> <outdir>
import http from 'node:http';
import { spawn } from 'node:child_process';
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';

const [wgslPath, cc6Path, outdir] = process.argv.slice(2);
mkdirSync(outdir, { recursive: true });
const wgsl = readFileSync(wgslPath, 'utf8');
const cc6 = JSON.parse(readFileSync(cc6Path, 'utf8'));
const KERNELS = ['update_player_agent', 'update_other_agents', 'update_camera', 'update_sphere', 'update_cube'];
const GROUPS = ['Compute Entity Layout', 'Compute Texture Layout'];

const PAGE = `<!doctype html><title>spv</title><script>
const VIS={Compute:GPUShaderStage.COMPUTE,Vertex:GPUShaderStage.VERTEX,Fragment:GPUShaderStage.FRAGMENT};
const BUF={Storage:'storage',ReadOnlyStorage:'read-only-storage',Uniform:'uniform'};
const SAMP={Filtering:'filtering',NonFiltering:'non-filtering',Comparison:'comparison'};
const TEX={Float:'float',UnfilterableFloat:'unfilterable-float',Depth:'depth',Uint:'uint',Sint:'sint'};
const FMT={RGBA16Float:'rgba16float',RGBA8Unorm:'rgba8unorm',RG32Float:'rg32float',R32Float:'r32float',RGBA32Float:'rgba32float'};
function layoutDesc(lay){const entries=[];for(const e of lay.entries){const ent={binding:e.binding,visibility:e.stages.reduce((a,s)=>a|VIS[s],0)};
 if(e.class==='storage_buffer'||e.class==='uniform_buffer'||e.class==='other_buffer'){ent.buffer={type:BUF[e.detail]??'uniform'};}
 else if(e.class==='sampler'){ent.sampler={type:SAMP[e.detail]??'filtering'};}
 else if(e.class==='sampled_texture'){ent.texture={sampleType:TEX[e.detail]??'float',viewDimension:e.viewDimension??'2d'};}
 else if(e.class==='storage_texture'){ent.storageTexture={access:'write-only',format:FMT[e.detail],viewDimension:e.viewDimension??'2d'};}
 entries.push(ent);}return{entries};}
(async()=>{try{const wgsl=await(await fetch('/wgsl')).text();const cc6=await(await fetch('/cc6')).json();
 const a=await navigator.gpu.requestAdapter();const d=await a.requestDevice();
 const layByLabel=Object.fromEntries(cc6.layouts.map(l=>[l.label,l]));
 const module=d.createShaderModule({code:wgsl});await module.getCompilationInfo();
 const bgls=${JSON.stringify(GROUPS)}.map(l=>d.createBindGroupLayout(layoutDesc(layByLabel[l])));
 const plo=d.createPipelineLayout({bindGroupLayouts:bgls});
 for(const ep of ${JSON.stringify(KERNELS)}){await d.createComputePipelineAsync({layout:plo,compute:{module,entryPoint:ep}});}
 navigator.sendBeacon('/done','ok');}catch(e){navigator.sendBeacon('/done','ERR '+e.message);}})();</script>`;

let doneResolve; const done = new Promise(r => (doneResolve = r));
const server = http.createServer((req, res) => {
  if (req.url === '/wgsl') { res.setHeader('content-type','text/plain'); res.end(wgsl); return; }
  if (req.url === '/cc6') { res.setHeader('content-type','application/json'); res.end(JSON.stringify(cc6)); return; }
  if (req.url === '/done') { let b=''; req.on('data',c=>b+=c); req.on('end',()=>{res.end('ok');doneResolve(b);}); return; }
  res.setHeader('content-type','text/html'); res.end(PAGE);
});
await new Promise(r => server.listen(0, '127.0.0.1', r));
const port = server.address().port;

const chunks = [];
const child = spawn('/opt/pw-browsers/chromium', [
  '--headless=new','--no-sandbox','--disable-gpu-sandbox','--in-process-gpu',
  '--enable-unsafe-webgpu','--enable-features=Vulkan','--use-webgpu-adapter=swiftshader',
  '--enable-dawn-features=dump_shaders,disable_symbol_renaming','--enable-logging=stderr','--v=0',
  `http://127.0.0.1:${port}/`,
], { env: { ...process.env, PLAYWRIGHT_BROWSERS_PATH: '/opt/pw-browsers' } });
child.stdout.on('data', d => chunks.push(d.toString()));
child.stderr.on('data', d => chunks.push(d.toString()));
const beacon = await Promise.race([done, new Promise(r => setTimeout(() => r('TIMEOUT'), 60000))]);
await new Promise(r => setTimeout(r, 2500));
child.kill('SIGKILL'); server.close();

const raw = chunks.join('');
// Each SPIR-V block: from a "; SPIR-V" line to the next "Dumped"/"; SPIR-V"/end.
// Strip Chrome log-prefixed lines. Key by `OpEntryPoint GLCompute %<kernel>`.
const isChromeLog = l => /^\[\d+[:.]\d+[:/]/.test(l) || /:INFO:CONSOLE|:VERBOSE|:WARNING|:ERROR/.test(l);
const idx = [];
const lines = raw.split('\n');
for (let i = 0; i < lines.length; i++) if (/^\s*; SPIR-V\s*$/.test(lines[i]) || /; SPIR-V\s*$/.test(lines[i])) idx.push(i);
const result = {};
for (let b = 0; b < idx.length; b++) {
  const start = idx[b];
  const end = (b + 1 < idx.length) ? idx[b + 1] : lines.length;
  let block = lines.slice(start, end).filter(l => !isChromeLog(l));
  // trim trailing non-spirv tail after the block (next "Dumped WGSL"/"Dumped SPIRV" console)
  const body = block.join('\n');
  const clean = body.slice(body.indexOf('; SPIR-V'));
  const m = clean.match(/OpEntryPoint\s+GLCompute\s+%(\w+)/);
  if (!m) continue;
  const ep = m[1];
  // Normalize the origin-hashed entry-point string so it can never drift a diff.
  const norm = clean.replace(/dawn_entry_point_[0-9a-f]+/g, 'dawn_entry_point_EP')
                    .split('\n')
                    .filter(l => !/source:\s*http/.test(l) && l.trim() !== '"')
                    .map(l => l.replace(/\s+$/,''))
                    .join('\n').trim();
  result[ep] = norm;
  writeFileSync(`${outdir}/${ep}.spv.txt`, norm + '\n');
}
const summary = KERNELS.map(k => `${k}: ${result[k] ? result[k].split('\n').length + ' lines' : 'MISSING'}`);
console.log(JSON.stringify({ beacon, spirv_blocks: idx.length, kernels: Object.keys(result), summary }, null, 2));
