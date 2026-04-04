import { useState, useRef, useEffect, useCallback, useMemo } from "react";

/* ═══ CLIPBOARD HELPER (sandbox-safe) ═══ */
function copyText(text) {
  if (navigator.clipboard && navigator.clipboard.writeText) {
    navigator.clipboard.writeText(text).catch(() => fallbackCopy(text));
  } else { fallbackCopy(text); }
}
function fallbackCopy(text) {
  const ta = document.createElement("textarea");
  ta.value = text; ta.style.position = "fixed"; ta.style.opacity = "0";
  document.body.appendChild(ta); ta.select();
  try { document.execCommand("copy"); } catch(e) {}
  document.body.removeChild(ta);
}


// ═══════════════════════════════════════════════════════════════════
// 7T STAMP GENERATOR v5
// No B&W. User-driven color mixing: 1–3 medians × per-color variance.
// Smoothness gradient. Scatter density. Field / Grid / Voronoi.
// ═══════════════════════════════════════════════════════════════════

// ── HASH PIPELINE (identical to world.wgsl) ─────────────────────
function hp(seed, prop) {
  let h = Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1;
  h = h >>> 0; h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769); h = h >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769); h = h >>> 0;
  return (((h >>> 16) ^ h) >>> 0) / 0xFFFFFFFF;
}
function lns(ms, nx, nz, band) {
  let h = ms >>> 0; h = (h ^ Math.imul(nx >>> 0, 73856093)) >>> 0;
  h = (h ^ Math.imul(nz >>> 0, 19349663)) >>> 0; h = (h ^ Math.imul(band >>> 0, 83492791)) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769); h = h >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769); h = h >>> 0; return h;
}
const C = v => Math.max(0, Math.min(1, v));
const L = (a, b, t) => [a[0]*(1-t)+b[0]*t, a[1]*(1-t)+b[1]*t, a[2]*(1-t)+b[2]*t];
const hm = t => t * t * (3 - 2 * t);
function cls(nx, nz, band, ms) { return lns(ms, nx, nz, band + 100); }
function h2r(hex) { const m = hex.match(/^#?([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$/i); return m ? [parseInt(m[1],16)/255, parseInt(m[2],16)/255, parseInt(m[3],16)/255] : [.5,.5,.5]; }
function r2h(rgb) { return "#" + rgb.map(v => Math.round(C(v)*255).toString(16).padStart(2,"0")).join(""); }

// ═══ COLOR ENGINE ═══
function colorEngineCell(gx, gz, seed, cfg) {
  const cs = lns(seed, gx, gz, 200), colors = cfg.colors, n = colors.length;
  let idx;
  if (cfg.pattern === "checker") { if (n===1) idx=0; else if (n===2) idx=(gx+gz)&1; else idx=((gx%3)+(gz%3))%3; }
  else { idx = Math.min(Math.floor(hp(cs,700)*n), n-1); }
  const med = colors[idx].rgb, vari = colors[idx].variance;
  const nr=(hp(cs,840)-.5)*2, ng=(hp(cs,841)-.5)*2, nb=(hp(cs,842)-.5)*2;
  const discrete = [C(med[0]+nr*vari), C(med[1]+ng*vari), C(med[2]+nb*vari)];
  const smooth = [0,0,0]; for (let i=0;i<n;i++) for (let ch=0;ch<3;ch++) smooth[ch]+=colors[i].rgb[ch]/n;
  return { discrete, smooth, cs };
}
function fillColor(cfg) {
  if (cfg.fillMode==="custom"&&cfg.fillCustom) return cfg.fillCustom;
  if (cfg.fillMode==="color1"&&cfg.colors[0]) return cfg.colors[0].rgb;
  if (cfg.fillMode==="color2"&&cfg.colors[1]) return cfg.colors[1].rgb;
  if (cfg.fillMode==="color3"&&cfg.colors[2]) return cfg.colors[2].rgb;
  const n=cfg.colors.length, avg=[0,0,0]; for(let i=0;i<n;i++) for(let ch=0;ch<3;ch++) avg[ch]+=cfg.colors[i].rgb[ch]/n; return avg;
}
function evalGridCell(gx, gz, gridN, seed, cfg) {
  const { discrete, cs } = colorEngineCell(gx, gz, seed, cfg); const fill = fillColor(cfg);
  let smoothT = 0;
  if (cfg.smoothness > 0) {
    const fx=(gx+.5)/gridN, fz=(gz+.5)/gridN; let edgeness;
    if (cfg.smoothShape==="radial") { const dx=fx-.5,dz=fz-.5; edgeness=Math.min(1,Math.sqrt(dx*dx+dz*dz)*2); }
    else { const rad=cfg.smoothAngle*Math.PI/180; edgeness=C(fx*Math.cos(rad)+fz*Math.sin(rad)); }
    smoothT = edgeness * cfg.smoothness;
  }
  const color = L(discrete, fill, smoothT);
  if (cfg.scatter>0) { if (hp(cs,900)>1-cfg.scatter) return {color:fill, visible:false}; }
  return { color, visible: true };
}

// ═══ FIELD MODE ═══
function fieldPaletteAt(wx, wz, cfg) {
  const sp=cfg.palSpacing, gx=wx/sp, gz=wz/sp, bx=Math.floor(gx), bz=Math.floor(gz), fx=hm(gx-bx), fz=hm(gz-bz);
  const colors=cfg.colors, n=colors.length; const smooth=[0,0,0]; for(let i=0;i<n;i++) for(let ch=0;ch<3;ch++) smooth[ch]+=colors[i].rgb[ch]/n;
  let wIdx=[0,0,0]; for(let dz=0;dz<2;dz++) for(let dx=0;dx<2;dx++) { const s=cls(bx+dx,bz+dz,0,cfg.seed), roll=hp(s,500), dom=Math.min(Math.floor(roll*n),n-1), w=(dx?fx:1-fx)*(dz?fz:1-fz); wIdx[dom]=(wIdx[dom]||0)+w; }
  const result=[0,0,0]; let totalW=0; for(let i=0;i<n;i++) totalW+=wIdx[i]||0; if(totalW<.001) return smooth;
  for(let i=0;i<n;i++){const w=(wIdx[i]||0)/totalW; for(let ch=0;ch<3;ch++) result[ch]+=colors[i].rgb[ch]*w;} return result;
}
function fieldModeAt(wx, wz, cfg) {
  const sp=cfg.modeSpacing, gx=wx/sp, gz=wz/sp, bx=Math.floor(gx), bz=Math.floor(gz), fx=hm(gx-bx), fz=hm(gz-bz);
  let r=0; for(let dz=0;dz<2;dz++) for(let dx=0;dx<2;dx++) r+=Math.pow(hp(cls(bx+dx,bz+dz,1,cfg.seed),501),cfg.modeExp)*((dx?fx:1-fx)*(dz?fz:1-fz)); return r;
}
function fieldDiscreteColor(wx, wz, cs, cfg) {
  const colors=cfg.colors, n=colors.length, roll=hp(cs,700), idx=Math.min(Math.floor(roll*n),n-1);
  const med=colors[idx].rgb, vari=colors[idx].variance;
  const nr=(hp(cs,840)-.5)*2, ng=(hp(cs,841)-.5)*2, nb=(hp(cs,842)-.5)*2;
  return [C(med[0]+nr*vari), C(med[1]+ng*vari), C(med[2]+nb*vari)];
}
function evalFieldPixel(wx, wz, cfg) {
  const cs_=cfg.cellSize, gx=Math.floor(wx/cs_), gz=Math.floor(wz/cs_), cs=lns(cfg.seed,gx,gz,200);
  const smooth=fieldPaletteAt(wx,wz,cfg), discrete=fieldDiscreteColor(wx,wz,cs,cfg);
  let mode=C(fieldModeAt(wx,wz,cfg)+cfg.modeBias); const T=cfg.modeThreshold;
  const S=(e0,e1,x)=>{const t=C((x-e0)/(e1-e0));return t*t*(3-2*t);}; const blendT=S(T-.15,T+.05,mode);
  let color=L(smooth,discrete,blendT);
  if(cfg.borders&&cfg.borderWidth>0){const fx=(wx/cs_)-gx,fz=(wz/cs_)-gz,bw=cfg.borderWidth;if(fx<bw||fx>1-bw||fz<bw||fz>1-bw) color=L(color,cfg.borderColor,cfg.borderOpacity);}
  return color;
}
function localCS(fx,fy,cfg){if(!cfg.varScale)return cfg.cellSize;const rad=cfg.scaleAngle*Math.PI/180;const t=C(fx*Math.cos(rad)+fy*Math.sin(rad));return cfg.csMin+t*(cfg.csMax-cfg.csMin);}
const BZ=.18; function tw(t){const s=(e0,e1,x)=>{const v=C((x-e0)/(e1-e0));return v*v*(3-2*v);};if(t<BZ)return 1-s(0,BZ,t);if(t>1-BZ)return s(1-BZ,1,t);return 0;}

// ═══ VORONOI ═══
function genVorPts(seed,count,ext){const pts=[];for(let i=0;i<count;i++)pts.push({x:hp(seed+i,1000)*ext,z:hp(seed+i,1001)*ext,cs:lns(seed,i,0,300),idx:i});return pts;}
function buildBuckets(pts,ext,bN){const bk=Array.from({length:bN*bN},()=>[]);for(const p of pts){const bx=Math.min(Math.floor(p.x/ext*bN),bN-1),bz=Math.min(Math.floor(p.z/ext*bN),bN-1);bk[bz*bN+bx].push(p);}return bk;}
function vorNearest(wx,wz,bk,bN,ext){const cbx=Math.min(Math.floor(C(wx/ext)*bN),bN-1),cbz=Math.min(Math.floor(C(wz/ext)*bN),bN-1);let best=1e12,bestPt=null,second=1e12;for(let dz=-2;dz<=2;dz++)for(let dx=-2;dx<=2;dx++){const nx=cbx+dx,nz=cbz+dz;if(nx<0||nx>=bN||nz<0||nz>=bN)continue;for(const p of bk[nz*bN+nx]){const d=(p.x-wx)*(p.x-wx)+(p.z-wz)*(p.z-wz);if(d<best){second=best;best=d;bestPt=p;}else if(d<second)second=d;}}return{pt:bestPt,edge:Math.sqrt(second)-Math.sqrt(best)};}
function vorCellColor(pt,cfg){const colors=cfg.colors,n=colors.length,roll=hp(pt.cs,700);const idx=cfg.pattern==="checker"?pt.idx%n:Math.min(Math.floor(roll*n),n-1);const med=colors[idx].rgb,vari=colors[idx].variance;const nr=(hp(pt.cs,840)-.5)*2,ng=(hp(pt.cs,841)-.5)*2,nb=(hp(pt.cs,842)-.5)*2;return[C(med[0]+nr*vari),C(med[1]+ng*vari),C(med[2]+nb*vari)];}

// ═══ RENDERERS ═══
function renderGrid(canvas,gridN,seed,cfg,res,gapPx){const ctx=canvas.getContext("2d");canvas.width=res;canvas.height=res;const fill=fillColor(cfg);ctx.fillStyle=`rgb(${Math.round(fill[0]*255)},${Math.round(fill[1]*255)},${Math.round(fill[2]*255)})`;ctx.fillRect(0,0,res,res);const tg=gapPx*(gridN+1),cPx=(res-tg)/gridN;for(let gz=0;gz<gridN;gz++)for(let gx=0;gx<gridN;gx++){const{color,visible}=evalGridCell(gx,gz,gridN,seed,cfg);if(!visible&&gapPx>0)continue;ctx.fillStyle=`rgb(${Math.round(color[0]*255)},${Math.round(color[1]*255)},${Math.round(color[2]*255)})`;ctx.fillRect(gapPx+gx*(cPx+gapPx),gapPx+gz*(cPx+gapPx),cPx,cPx);}}
function renderField(canvas,cfg,res,tileable,onP,onD){const ctx=canvas.getContext("2d");canvas.width=res;canvas.height=res;const img=ctx.createImageData(res,res),d=img.data;let row=0;const CH=Math.max(2,Math.floor(48/(res/256)));function chunk(){const end=Math.min(row+CH,res);for(let py=row;py<end;py++)for(let px=0;px<res;px++){const fx=px/res,fy=py/res,ext=cfg.worldExtent;let color;if(tileable){const wx0=cfg.originX+fx*ext,wz0=cfg.originZ+fy*ext,bx=tw(fx),bz=tw(fy),lcs=localCS(fx,fy,cfg),cfgL={...cfg,cellSize:lcs},eval_=(wx,wz)=>evalFieldPixel(wx,wz,cfgL);if(bx<.001&&bz<.001)color=eval_(wx0,wz0);else{const offX=fx<.5?ext:-ext,offZ=fy<.5?ext:-ext,c00=eval_(wx0,wz0);if(bx>.001&&bz>.001)color=L(L(c00,eval_(wx0+offX,wz0),bx),L(eval_(wx0,wz0+offZ),eval_(wx0+offX,wz0+offZ),bx),bz);else if(bx>.001)color=L(c00,eval_(wx0+offX,wz0),bx);else color=L(c00,eval_(wx0,wz0+offZ),bz);}}else{const lcs=localCS(fx,fy,cfg);color=evalFieldPixel(cfg.originX+fx*ext,cfg.originZ+fy*ext,{...cfg,cellSize:lcs});}const idx=(py*res+px)*4;d[idx]=Math.round(color[0]*255);d[idx+1]=Math.round(color[1]*255);d[idx+2]=Math.round(color[2]*255);d[idx+3]=255;}row=end;if(onP)onP(Math.round(row/res*100));if(row<res)setTimeout(chunk,0);else{ctx.putImageData(img,0,0);if(onD)onD();}}chunk();}
function renderVoronoi(canvas,cfg,res,onP,onD){const ctx=canvas.getContext("2d");canvas.width=res;canvas.height=res;const img=ctx.createImageData(res,res),d=img.data;const ext=cfg.vorExtent,pts=genVorPts(cfg.seed,cfg.vorCount,ext),bN=Math.max(4,Math.floor(Math.sqrt(cfg.vorCount)*1.5)),bk=buildBuckets(pts,ext,bN),bCol=cfg.borderColor;let row=0;const CH=Math.max(2,Math.floor(48/(res/256)));function chunk(){const end=Math.min(row+CH,res);for(let py=row;py<end;py++)for(let px=0;px<res;px++){const wx=(px/res)*ext,wz=(py/res)*ext;const{pt,edge}=vorNearest(wx,wz,bk,bN,ext);let color=pt?vorCellColor(pt,cfg):[.5,.5,.5];if(cfg.borders&&cfg.borderWidth>0){const eT=cfg.borderWidth*ext*.01;if(edge<eT)color=L(color,bCol,(1-edge/eT)*cfg.borderOpacity);}if(cfg.scatter>0&&pt){if(hp(pt.cs,900)>1-cfg.scatter)color=fillColor(cfg);}const idx=(py*res+px)*4;d[idx]=Math.round(color[0]*255);d[idx+1]=Math.round(color[1]*255);d[idx+2]=Math.round(color[2]*255);d[idx+3]=255;}row=end;if(onP)onP(Math.round(row/res*100));if(row<res)setTimeout(chunk,0);else{ctx.putImageData(img,0,0);if(onD)onD();}}chunk();}
function genGridSVG(gridN,seed,cfg,gap){const sz=100,gm=gap?.3:0,tg=gm*(gridN+1),cm=(sz-tg)/gridN;const fill=fillColor(cfg);let r=gap?`<rect x="0" y="0" width="${sz}" height="${sz}" fill="rgb(${Math.round(fill[0]*255)},${Math.round(fill[1]*255)},${Math.round(fill[2]*255)})"/>\n`:"";for(let gz=0;gz<gridN;gz++)for(let gx=0;gx<gridN;gx++){const{color,visible}=evalGridCell(gx,gz,gridN,seed,cfg);if(!visible&&gap)continue;const w=gap?cm.toFixed(3):(cm+.015).toFixed(3);r+=`<rect x="${(gm+gx*(cm+gm)).toFixed(3)}" y="${(gm+gz*(cm+gm)).toFixed(3)}" width="${w}" height="${w}" fill="rgb(${Math.round(color[0]*255)},${Math.round(color[1]*255)},${Math.round(color[2]*255)})"/>`;}return `<?xml version="1.0" encoding="UTF-8"?>\n<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${sz} ${sz}" width="${sz}mm" height="${sz}mm">\n${r}\n</svg>`;}

// ═══ VARIANCE SWATCH ═══
function VarianceSwatch({hex,variance,width="100%",height=16}){const canvasRef=useRef(null);useEffect(()=>{const cv=canvasRef.current;if(!cv)return;const ctx=cv.getContext("2d");const w=cv.width=120,h=cv.height=16;const med=h2r(hex);for(let x=0;x<w;x++){const t=(x/(w-1))*2-1;ctx.fillStyle=`rgb(${Math.round(C(med[0]+t*variance)*255)},${Math.round(C(med[1]+t*variance)*255)},${Math.round(C(med[2]+t*variance)*255)})`;ctx.fillRect(x,0,1,h);}ctx.fillStyle="rgba(255,255,255,0.5)";ctx.fillRect(w/2-.5,0,1,3);ctx.fillRect(w/2-.5,h-3,1,3);},[hex,variance]);return <canvas ref={canvasRef} style={{width,height,borderRadius:3,border:"1px solid #2a2520",display:"block"}}/>;}

// ═══════════════════════════════════════════════════════════════════
// REACT UI
// ═══════════════════════════════════════════════════════════════════

/* ═══ PORTABLE STORAGE — artifact window.storage → localStorage fallback ═══ */
const store = {
  async get(key) {
    try { if (window.storage) { const r = await window.storage.get(key); return r?.value ?? null; } } catch {}
    try { return localStorage.getItem(key); } catch {}
    return null;
  },
  async set(key, value) {
    try { if (window.storage) await window.storage.set(key, value); } catch {}
    try { localStorage.setItem(key, value); } catch {}
  },
};
const STORAGE_KEY = "7t:stamp:state";

export default function StampGenerator() {
  const gridRef=useRef(null),fieldRef=useRef(null),vorRef=useRef(null),tileRef=useRef(null);
  const [rendering,setRendering]=useState(false);const [progress,setProgress]=useState(0);
  const [mode,setMode]=useState("grid");const [tab,setTab]=useState("design");
  const [loaded,setLoaded]=useState(false);
  const [colorCount,setColorCount]=useState(2);
  const [c1Hex,setC1Hex]=useState("#c06838");const [c1Var,setC1Var]=useState(0.12);
  const [c2Hex,setC2Hex]=useState("#3868a0");const [c2Var,setC2Var]=useState(0.15);
  const [c3Hex,setC3Hex]=useState("#48884a");const [c3Var,setC3Var]=useState(0.10);
  const [pattern,setPattern]=useState("checker");const [fillMode,setFillMode]=useState("blend");const [fillCustomHex,setFillCustomHex]=useState("#8a7050");
  const [smoothness,setSmoothness]=useState(0);const [smoothShape,setSmoothShape]=useState("radial");const [smoothAngle,setSmoothAngle]=useState(90);
  const [scatter,setScatter]=useState(0);
  const [gridSeed,setGridSeed]=useState(42);const [gridN,setGridN]=useState(16);const [gridRes,setGridRes]=useState(512);const [gridGap,setGridGap]=useState(true);const [gridGapPx,setGridGapPx]=useState(2);
  const [fieldSeed,setFieldSeed]=useState(42);const [fieldRes,setFieldRes]=useState(512);const [worldExtent,setWorldExtent]=useState(400);const [cellSize,setCellSize]=useState(3.0);const [modeBias,setModeBias]=useState(0.1);const [originX,setOriginX]=useState(0);const [originZ,setOriginZ]=useState(0);const [tileable,setTileable]=useState(false);const [showTile,setShowTile]=useState(false);
  const [varScale,setVarScale]=useState(false);const [scaleAngle,setScaleAngle]=useState(90);const [csMin,setCsMin]=useState(1.0);const [csMax,setCsMax]=useState(6.0);
  const [borders,setBorders]=useState(false);const [borderW,setBorderW]=useState(0.05);const [borderO,setBorderO]=useState(0.85);const [borderHex,setBorderHex]=useState("#0a0806");
  const [vorSeed,setVorSeed]=useState(42);const [vorCount,setVorCount]=useState(256);const [vorExtent,setVorExtent]=useState(100);const [vorRes,setVorRes]=useState(512);
  const [vorBorders,setVorBorders]=useState(true);const [vorBW,setVorBW]=useState(0.15);const [vorBO,setVorBO]=useState(0.9);const [vorBHex,setVorBHex]=useState("#0a0806");

  const DEFAULTS_JSON = JSON.stringify(defaultSnapshot());
  const snapshotJSON = JSON.stringify(getSnapshot());
  const isModified = snapshotJSON !== DEFAULTS_JSON;

  // Load saved state on mount
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) { const data = JSON.parse(raw); if (data && typeof data === "object" && data.c1Hex) applySnapshot(data); }
      } catch (e) {}
      setLoaded(true);
    })();
  }, []);

  // Auto-save on every edit after initial load
  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, snapshotJSON); } catch (e) {} })();
  }, [snapshotJSON, loaded]);
  const getSnapshot = () => ({
    colorCount,c1Hex,c1Var,c2Hex,c2Var,c3Hex,c3Var,pattern,fillMode,fillCustomHex,
    smoothness,smoothShape,smoothAngle,scatter,
    gridSeed,gridN,gridRes,gridGap,gridGapPx,
    fieldSeed,fieldRes,worldExtent,cellSize,modeBias,originX,originZ,tileable,
    varScale,scaleAngle,csMin,csMax,borders,borderW,borderO,borderHex,
    vorSeed,vorCount,vorExtent,vorRes,vorBorders,vorBW,vorBO,vorBHex,
  });
  const applySnapshot = (s) => {
    setColorCount(s.colorCount);setC1Hex(s.c1Hex);setC1Var(s.c1Var);setC2Hex(s.c2Hex);setC2Var(s.c2Var);setC3Hex(s.c3Hex);setC3Var(s.c3Var);
    setPattern(s.pattern);setFillMode(s.fillMode);setFillCustomHex(s.fillCustomHex);
    setSmoothness(s.smoothness);setSmoothShape(s.smoothShape);setSmoothAngle(s.smoothAngle);setScatter(s.scatter);
    setGridSeed(s.gridSeed);setGridN(s.gridN);setGridRes(s.gridRes);setGridGap(s.gridGap);setGridGapPx(s.gridGapPx);
    setFieldSeed(s.fieldSeed);setFieldRes(s.fieldRes);setWorldExtent(s.worldExtent);setCellSize(s.cellSize);setModeBias(s.modeBias);setOriginX(s.originX);setOriginZ(s.originZ);setTileable(s.tileable);
    setVarScale(s.varScale);setScaleAngle(s.scaleAngle);setCsMin(s.csMin);setCsMax(s.csMax);setBorders(s.borders);setBorderW(s.borderW);setBorderO(s.borderO);setBorderHex(s.borderHex);
    setVorSeed(s.vorSeed);setVorCount(s.vorCount);setVorExtent(s.vorExtent);setVorRes(s.vorRes);setVorBorders(s.vorBorders);setVorBW(s.vorBW);setVorBO(s.vorBO);setVorBHex(s.vorBHex);
  };
  const defaultSnapshot = () => ({
    colorCount:2,c1Hex:"#c06838",c1Var:0.12,c2Hex:"#3868a0",c2Var:0.15,c3Hex:"#48884a",c3Var:0.10,
    pattern:"checker",fillMode:"blend",fillCustomHex:"#8a7050",smoothness:0,smoothShape:"radial",smoothAngle:90,scatter:0,
    gridSeed:42,gridN:16,gridRes:512,gridGap:true,gridGapPx:2,
    fieldSeed:42,fieldRes:512,worldExtent:400,cellSize:3.0,modeBias:0.1,originX:0,originZ:0,tileable:false,
    varScale:false,scaleAngle:90,csMin:1.0,csMax:6.0,borders:false,borderW:0.05,borderO:0.85,borderHex:"#0a0806",
    vorSeed:42,vorCount:256,vorExtent:100,vorRes:512,vorBorders:true,vorBW:0.15,vorBO:0.9,vorBHex:"#0a0806",
  });

  const buildColors=useCallback(()=>{const arr=[{rgb:h2r(c1Hex),variance:c1Var}];if(colorCount>=2)arr.push({rgb:h2r(c2Hex),variance:c2Var});if(colorCount>=3)arr.push({rgb:h2r(c3Hex),variance:c3Var});return arr;},[colorCount,c1Hex,c1Var,c2Hex,c2Var,c3Hex,c3Var]);
  const buildGridCfg=useCallback(()=>({colors:buildColors(),pattern,scatter,smoothness,smoothShape,smoothAngle,fillMode,fillCustom:h2r(fillCustomHex)}),[buildColors,pattern,scatter,smoothness,smoothShape,smoothAngle,fillMode,fillCustomHex]);
  const buildFieldCfg=useCallback(()=>({seed:fieldSeed,colors:buildColors(),worldExtent,cellSize,modeBias,originX,originZ,palSpacing:300,modeSpacing:120,modeThreshold:.7,modeExp:5,varScale,scaleAngle,csMin,csMax,borders,borderWidth:borderW,borderOpacity:borderO,borderColor:h2r(borderHex)}),[fieldSeed,buildColors,worldExtent,cellSize,modeBias,originX,originZ,varScale,scaleAngle,csMin,csMax,borders,borderW,borderO,borderHex]);
  const buildVorCfg=useCallback(()=>({seed:vorSeed,colors:buildColors(),pattern,vorCount,vorExtent,scatter,borders:vorBorders,borderWidth:vorBW,borderOpacity:vorBO,borderColor:h2r(vorBHex),fillMode,fillCustom:h2r(fillCustomHex)}),[vorSeed,buildColors,pattern,vorCount,vorExtent,scatter,vorBorders,vorBW,vorBO,vorBHex,fillMode,fillCustomHex]);
  const doGenGrid=useCallback(()=>{if(!gridRef.current)return;renderGrid(gridRef.current,gridN,gridSeed,buildGridCfg(),gridRes,gridGap?gridGapPx:0);},[gridN,gridSeed,buildGridCfg,gridRes,gridGap,gridGapPx]);
  const doGenField=useCallback(()=>{if(!fieldRef.current||rendering)return;setRendering(true);setProgress(0);requestAnimationFrame(()=>setTimeout(()=>{renderField(fieldRef.current,buildFieldCfg(),fieldRes,tileable,p=>setProgress(p),()=>{setRendering(false);setProgress(100);});},20));},[buildFieldCfg,fieldRes,tileable,rendering]);
  const doGenVor=useCallback(()=>{if(!vorRef.current||rendering)return;setRendering(true);setProgress(0);requestAnimationFrame(()=>setTimeout(()=>{renderVoronoi(vorRef.current,buildVorCfg(),vorRes,p=>setProgress(p),()=>{setRendering(false);setProgress(100);});},20));},[buildVorCfg,vorRes,rendering]);
  const doGen=()=>{if(mode==="grid")doGenGrid();else if(mode==="field")doGenField();else doGenVor();};
  useEffect(()=>{if(mode==="grid")doGenGrid();},[mode,doGenGrid]);
  useEffect(()=>{if(!showTile||!fieldRef.current||!tileRef.current||rendering||mode!=="field")return;const s=fieldRef.current,d=tileRef.current,sz=s.width;if(!sz)return;d.width=sz*2;d.height=sz*2;const ctx=d.getContext("2d");ctx.drawImage(s,0,0);ctx.drawImage(s,sz,0);ctx.drawImage(s,0,sz);ctx.drawImage(s,sz,sz);ctx.strokeStyle="rgba(196,154,108,0.3)";ctx.lineWidth=1;ctx.setLineDash([6,4]);ctx.beginPath();ctx.moveTo(sz,0);ctx.lineTo(sz,sz*2);ctx.stroke();ctx.beginPath();ctx.moveTo(0,sz);ctx.lineTo(sz*2,sz);ctx.stroke();},[showTile,rendering,mode]);
  useEffect(()=>{doGenGrid();},[]);
  useEffect(()=>{setTab(mode==="field"?"spatial":"design");},[mode]);
  const dlPNG=(ref,name)=>{if(!ref.current)return;const a=document.createElement("a");a.download=name;a.href=ref.current.toDataURL("image/png");a.click();};
  const dlSVG=(svg,name)=>{const b=new Blob([svg],{type:"image/svg+xml"});const a=document.createElement("a");a.download=name;a.href=URL.createObjectURL(b);a.click();};

  const sL={fontSize:10,color:"#5a5045",letterSpacing:"0.1em",textTransform:"uppercase",marginBottom:5,fontFamily:"monospace"};
  const Sl=({label,value,onChange,min,max,step,unit=""})=>(<div style={{marginBottom:6}}><div style={{display:"flex",justifyContent:"space-between"}}><span style={{fontSize:10,color:"#8a7e6e",fontFamily:"monospace"}}>{label}</span><span style={{fontSize:10,color:"#c4a882",fontFamily:"monospace"}}>{step<1?value.toFixed(2):value}{unit}</span></div><input type="range" min={min} max={max} step={step} value={value} onChange={e=>onChange(Number(e.target.value))} style={{width:"100%",accentColor:"#c49a6c",height:2,cursor:"pointer"}}/></div>);
  const Tg=({label,checked,onChange})=>(<label style={{display:"flex",alignItems:"center",gap:6,cursor:"pointer",marginBottom:5,fontSize:10,color:"#a89878"}}><div onClick={e=>{e.preventDefault();onChange(!checked)}} style={{width:26,height:13,borderRadius:7,position:"relative",background:checked?"#c49a6c":"#2a2520",border:`1px solid ${checked?"#d4aa7c":"#3a3025"}`,cursor:"pointer"}}><div style={{width:9,height:9,borderRadius:5,background:checked?"#1a1714":"#5a5045",position:"absolute",top:1,left:checked?14:1,transition:"left 0.15s"}}/></div><span style={{fontFamily:"monospace"}}>{label}</span></label>);
  const Ci=({label,value,onChange})=>(<div style={{display:"flex",gap:4,alignItems:"center",marginBottom:5}}><input type="color" value={value} onChange={e=>onChange(e.target.value)} style={{width:26,height:20,border:"1px solid #2a2520",borderRadius:2,cursor:"pointer",background:"#0f0d0b",padding:1}}/><span style={{fontSize:9,color:"#7a6e5e",fontFamily:"monospace",width:42}}>{label}</span><input type="text" value={value} onChange={e=>onChange(e.target.value)} style={{flex:1,padding:"2px 4px",fontSize:10,fontFamily:"JetBrains Mono",background:"#0f0d0b",border:"1px solid #2a2520",color:"#c4a882",borderRadius:2,outline:"none"}}/></div>);

  const ColorSlot=({idx,hex,setHex,variance,setVariance,removable,onRemove})=>(<div style={{marginBottom:10,padding:"8px 10px",background:"#1e1b18",borderRadius:4,border:"1px solid #2a2520"}}><div style={{display:"flex",alignItems:"center",gap:6,marginBottom:6}}><input type="color" value={hex} onChange={e=>setHex(e.target.value)} style={{width:30,height:22,border:"1px solid #2a2520",borderRadius:3,cursor:"pointer",background:"#0f0d0b",padding:1}}/><span style={{fontSize:10,color:"#c4a882",fontFamily:"monospace",flex:1}}>Color {idx+1}</span><input type="text" value={hex} onChange={e=>setHex(e.target.value)} style={{width:72,padding:"2px 4px",fontSize:10,fontFamily:"JetBrains Mono",background:"#0f0d0b",border:"1px solid #2a2520",color:"#8a7e6e",borderRadius:2,outline:"none"}}/>{removable&&<button onClick={onRemove} style={{width:18,height:18,fontSize:11,fontFamily:"monospace",background:"transparent",border:"1px solid #2a2520",color:"#5a4540",borderRadius:2,cursor:"pointer",lineHeight:"16px",padding:0}}>×</button>}</div><div style={{marginBottom:3}}><div style={{display:"flex",justifyContent:"space-between",marginBottom:2}}><span style={{fontSize:9,color:"#6a5e4e",fontFamily:"monospace"}}>VARIANCE</span><span style={{fontSize:9,color:"#8a7e6e",fontFamily:"monospace"}}>±{(variance*100).toFixed(0)}% spread</span></div><input type="range" min={0} max={0.45} step={0.01} value={variance} onChange={e=>setVariance(Number(e.target.value))} style={{width:"100%",accentColor:"#c49a6c",height:2,cursor:"pointer",marginBottom:3}}/></div><VarianceSwatch hex={hex} variance={variance} height={14}/><div style={{display:"flex",justifyContent:"space-between",marginTop:2}}><span style={{fontSize:8,color:"#3a3530",fontFamily:"monospace"}}>darkest</span><span style={{fontSize:8,color:"#3a3530",fontFamily:"monospace"}}>median</span><span style={{fontSize:8,color:"#3a3530",fontFamily:"monospace"}}>lightest</span></div></div>);

  const FillPresets=()=>{const presets=[{id:"blend",label:"Median Blend"},{id:"color1",label:"Color 1",swatch:c1Hex}];if(colorCount>=2)presets.push({id:"color2",label:"Color 2",swatch:c2Hex});if(colorCount>=3)presets.push({id:"color3",label:"Color 3",swatch:c3Hex});presets.push({id:"custom",label:"Custom"});return(<div style={{marginBottom:6}}><div style={{display:"flex",justifyContent:"space-between",marginBottom:3}}><span style={{fontSize:9,color:"#6a5e4e",fontFamily:"monospace",textTransform:"uppercase",letterSpacing:"0.08em"}}>Fill Color</span><span style={{fontSize:8,color:"#4a4035",fontFamily:"monospace"}}>smooth zones + dropout</span></div><div style={{display:"flex",flexWrap:"wrap",gap:3}}>{presets.map(p=>(<button key={p.id} onClick={()=>setFillMode(p.id)} style={{padding:"3px 6px",fontSize:9,fontFamily:"monospace",background:fillMode===p.id?"#3a3025":"transparent",border:`1px solid ${fillMode===p.id?"#c49a6c":"#2a2520"}`,color:fillMode===p.id?"#e8d5b0":"#5a5045",borderRadius:2,cursor:"pointer",display:"flex",alignItems:"center",gap:3}}>{p.swatch&&<div style={{width:8,height:8,borderRadius:2,background:p.swatch,border:"1px solid #2a2520"}}/>}{p.label}</button>))}</div>{fillMode==="custom"&&<div style={{marginTop:4}}><Ci label="Fill" value={fillCustomHex} onChange={setFillCustomHex}/></div>}</div>);};

  const ColorsPanel=()=>(<><div style={sL}>Color Medians</div><ColorSlot idx={0} hex={c1Hex} setHex={setC1Hex} variance={c1Var} setVariance={setC1Var} removable={false}/>{colorCount>=2&&<ColorSlot idx={1} hex={c2Hex} setHex={setC2Hex} variance={c2Var} setVariance={setC2Var} removable={colorCount>1} onRemove={()=>setColorCount(c=>c-1)}/>}{colorCount>=3&&<ColorSlot idx={2} hex={c3Hex} setHex={setC3Hex} variance={c3Var} setVariance={setC3Var} removable onRemove={()=>setColorCount(2)}/>}{colorCount<3&&<button onClick={()=>setColorCount(c=>c+1)} style={{width:"100%",padding:"5px 0",fontSize:10,fontFamily:"monospace",marginBottom:10,background:"transparent",border:"1px dashed #2a2520",color:"#5a5045",borderRadius:3,cursor:"pointer"}}>+ Add Color</button>}<div style={{...sL,marginTop:4}}>Assignment</div><div style={{display:"flex",gap:3,marginBottom:8}}>{["checker","random"].map(p=>(<button key={p} onClick={()=>setPattern(p)} style={{flex:1,padding:"4px 0",fontSize:10,fontFamily:"monospace",textTransform:"capitalize",background:pattern===p?"#3a3025":"transparent",border:`1px solid ${pattern===p?"#c49a6c":"#2a2520"}`,color:pattern===p?"#e8d5b0":"#5a5045",borderRadius:2,cursor:"pointer"}}>{p}</button>))}</div><FillPresets/></>);

  const tabs=mode==="field"?["spatial","colors","export"]:["design","export"];
  const hdrBtn={padding:"3px 8px",fontSize:9,fontFamily:"monospace",letterSpacing:"0.04em",background:"transparent",borderRadius:3,cursor:"pointer"};

  if (!loaded) return <div style={{padding:20,fontFamily:"monospace",fontSize:11,color:"#5a5045"}}>Loading…</div>;

  return (
    <div style={{height:"100vh",background:"#1a1714",color:"#e8ddd0",fontFamily:"'DM Sans',sans-serif",display:"flex",flexDirection:"column",overflow:"hidden"}}>
      <link href="https://fonts.googleapis.com/css2?family=DM+Sans:wght@300;400;500&family=JetBrains+Mono:wght@300;400&display=swap" rel="stylesheet"/>

      {/* Header */}
      <div style={{padding:"9px 18px 7px",borderBottom:"1px solid #2a2520",display:"flex",alignItems:"center",gap:8,flexShrink:0}}>
        <span style={{fontSize:14,fontWeight:500,letterSpacing:"0.18em",color:"#c49a6c"}}>7T</span>
        <span style={{fontSize:11,fontWeight:300,color:"#6a5e4e"}}>Stamp Generator</span>
        {isModified && <span style={{fontSize:9,color:"#5a5045",marginLeft:4}}>● modified</span>}
        <div style={{marginLeft:14,display:"flex",borderRadius:4,overflow:"hidden",border:"1px solid #2a2520"}}>
          {[["grid","Grid"],["field","Field"],["voronoi","Voronoi"]].map(([id,lb])=>(<button key={id} onClick={()=>setMode(id)} style={{padding:"3px 10px",fontSize:9,fontFamily:"monospace",letterSpacing:".05em",textTransform:"uppercase",border:"none",cursor:"pointer",background:mode===id?"#c49a6c":"#1a1714",color:mode===id?"#1a1714":"#6a5e4e",fontWeight:mode===id?600:400}}>{lb}</button>))}
        </div>
        {mode==="field"&&<div style={{marginLeft:"auto",display:"flex",gap:8}}><Tg label="Tileable" checked={tileable} onChange={setTileable}/><Tg label="2×2" checked={showTile} onChange={setShowTile}/></div>}
        <div style={{marginLeft:mode==="field"?0:"auto",display:"flex",gap:4,alignItems:"center"}}>
          <button onClick={()=>applySnapshot(defaultSnapshot())} style={{...hdrBtn,border:"1px solid #5a3030",color:"#c06050"}}>Reset</button>
          <button onClick={()=>{const r=(v,d=4)=>+(+v).toFixed(d);const s=getSnapshot();let txt="=== 7T STAMP v5 STATE ===\n\n";txt+="── Colors ──\n";txt+=`  count: ${s.colorCount}  pattern: ${s.pattern}  fill: ${s.fillMode}\n`;txt+=`  color1: ${s.c1Hex} var:${r(s.c1Var)}\n`;if(s.colorCount>=2)txt+=`  color2: ${s.c2Hex} var:${r(s.c2Var)}\n`;if(s.colorCount>=3)txt+=`  color3: ${s.c3Hex} var:${r(s.c3Var)}\n`;txt+=`  smoothness: ${r(s.smoothness)} (${s.smoothShape} ${s.smoothAngle}°)  scatter: ${r(s.scatter)}\n\n`;txt+="── Grid ──\n";txt+=`  seed: ${s.gridSeed}  cells: ${s.gridN}  gap: ${s.gridGap} (${s.gridGapPx}px)\n\n`;txt+="── Field ──\n";txt+=`  seed: ${s.fieldSeed}  extent: ${s.worldExtent}  cellSize: ${r(s.cellSize)}\n`;txt+=`  modeBias: ${r(s.modeBias)}  origin: (${s.originX}, ${s.originZ})  tileable: ${s.tileable}\n`;txt+="\n── Voronoi ──\n";txt+=`  seed: ${s.vorSeed}  count: ${s.vorCount}  extent: ${s.vorExtent}\n`;txt+="\n── JSON ──\n"+JSON.stringify(s,null,2)+"\n";copyText(txt); (()=>{const btn=document.activeElement;if(btn){const orig=btn.textContent;btn.textContent="Copied!";setTimeout(()=>{btn.textContent=orig;},1200);}})();}} style={{...hdrBtn,border:"1px solid #3a5a3a",color:"#6aaa5c"}}>Save</button>
        </div>
      </div>

      <div style={{display:"flex",flex:1,overflow:"hidden"}}>
        <div style={{width:268,borderRight:"1px solid #2a2520",display:"flex",flexDirection:"column",flexShrink:0}}>
          <div style={{display:"flex",borderBottom:"1px solid #2a2520",flexShrink:0}}>{tabs.map(id=>(<button key={id} onClick={()=>setTab(id)} style={{flex:1,padding:"5px 0",fontSize:9,fontFamily:"monospace",letterSpacing:".06em",textTransform:"uppercase",background:"transparent",border:"none",borderBottom:tab===id?"2px solid #c49a6c":"2px solid transparent",color:tab===id?"#e8d5b0":"#5a5045",cursor:"pointer"}}>{id}</button>))}</div>
          <div style={{padding:"10px 14px",overflowY:"auto",flex:1}}>
            {mode==="grid"&&tab==="design"&&<><div style={{marginBottom:8}}><div style={sL}>Seed</div><div style={{display:"flex",gap:4}}><input type="number" value={gridSeed} onChange={e=>setGridSeed(Number(e.target.value))} style={{flex:1,padding:"3px 5px",fontSize:12,fontFamily:"JetBrains Mono",background:"#0f0d0b",border:"1px solid #2a2520",color:"#e8d5b0",borderRadius:2,outline:"none"}}/><button onClick={()=>setGridSeed(Math.floor(Math.random()*99999))} style={{padding:"3px 7px",fontSize:9,fontFamily:"monospace",background:"#2a2520",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:2,cursor:"pointer"}}>DICE</button></div></div><ColorsPanel/><div style={sL}>Grid</div><Sl label="Cells per side" value={gridN} onChange={v=>setGridN(Math.round(v))} min={2} max={64} step={1}/><Tg label="Show gaps" checked={gridGap} onChange={setGridGap}/>{gridGap&&<Sl label="Gap width" value={gridGapPx} onChange={setGridGapPx} min={0} max={8} step={1} unit="px"/>}<div style={{marginTop:6,paddingTop:6,borderTop:"1px solid #2a2520"}}><div style={sL}>Smoothness</div><Sl label="Blend amount" value={smoothness} onChange={setSmoothness} min={0} max={1} step={0.01}/>{smoothness>0&&<><div style={{display:"flex",gap:3,marginBottom:6}}>{["radial","directional"].map(s=>(<button key={s} onClick={()=>setSmoothShape(s)} style={{flex:1,padding:"3px 0",fontSize:9,fontFamily:"monospace",textTransform:"capitalize",background:smoothShape===s?"#3a3025":"transparent",border:`1px solid ${smoothShape===s?"#c49a6c":"#2a2520"}`,color:smoothShape===s?"#e8d5b0":"#5a5045",borderRadius:2,cursor:"pointer"}}>{s}</button>))}</div>{smoothShape==="directional"&&<Sl label="Angle" value={smoothAngle} onChange={setSmoothAngle} min={0} max={360} step={5} unit="°"/>}</>}</div><div style={{marginTop:4,paddingTop:4,borderTop:"1px solid #2a2520"}}><div style={sL}>Scatter</div><Sl label="Cell dropout" value={scatter} onChange={setScatter} min={0} max={0.9} step={0.01}/></div></>}
            {mode==="grid"&&tab==="export"&&<><div style={{marginBottom:12}}><div style={sL}>PNG</div><div style={{display:"flex",gap:3,marginBottom:5}}>{[256,512,1024,2048].map(r=>(<button key={r} onClick={()=>setGridRes(r)} style={{flex:1,padding:"4px 0",fontSize:10,fontFamily:"monospace",background:gridRes===r?"#3a3025":"transparent",border:`1px solid ${gridRes===r?"#c49a6c":"#2a2520"}`,color:gridRes===r?"#e8d5b0":"#5a5045",borderRadius:2,cursor:"pointer"}}>{r}</button>))}</div><button onClick={()=>dlPNG(gridRef,`7t-grid-${gridSeed}-${gridRes}.png`)} style={{width:"100%",padding:"5px 0",fontSize:10,fontFamily:"monospace",letterSpacing:".06em",textTransform:"uppercase",background:"transparent",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:3,cursor:"pointer"}}>Download PNG</button></div><div><div style={sL}>SVG</div><button onClick={()=>dlSVG(genGridSVG(gridN,gridSeed,buildGridCfg(),gridGap),`7t-grid-${gridSeed}.svg`)} style={{width:"100%",padding:"5px 0",fontSize:10,fontFamily:"monospace",letterSpacing:".06em",textTransform:"uppercase",background:"transparent",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:3,cursor:"pointer"}}>Download SVG</button></div></>}
            {mode==="field"&&tab==="spatial"&&<><div style={{marginBottom:8}}><div style={sL}>Seed</div><div style={{display:"flex",gap:4}}><input type="number" value={fieldSeed} onChange={e=>setFieldSeed(Number(e.target.value))} style={{flex:1,padding:"3px 5px",fontSize:12,fontFamily:"JetBrains Mono",background:"#0f0d0b",border:"1px solid #2a2520",color:"#e8d5b0",borderRadius:2,outline:"none"}}/><button onClick={()=>setFieldSeed(Math.floor(Math.random()*99999))} style={{padding:"3px 7px",fontSize:9,fontFamily:"monospace",background:"#2a2520",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:2,cursor:"pointer"}}>DICE</button></div></div><div style={sL}>Geometry</div><Sl label="World Extent" value={worldExtent} onChange={setWorldExtent} min={50} max={2000} step={10} unit=" wu"/><Sl label="Cell Size" value={cellSize} onChange={setCellSize} min={0.5} max={10} step={0.25} unit=" wu"/><Sl label="Origin X" value={originX} onChange={setOriginX} min={-2000} max={2000} step={10}/><Sl label="Origin Z" value={originZ} onChange={setOriginZ} min={-2000} max={2000} step={10}/><div style={{...sL,marginTop:6}}>Mode</div><Sl label="Smooth ↔ Discrete" value={modeBias} onChange={setModeBias} min={-0.5} max={0.8} step={0.01}/><div style={{marginTop:6,paddingTop:6,borderTop:"1px solid #2a2520"}}><Tg label="Variable Cell Scale" checked={varScale} onChange={setVarScale}/>{varScale&&<><Sl label="Angle" value={scaleAngle} onChange={setScaleAngle} min={0} max={360} step={5} unit="°"/><Sl label="Min" value={csMin} onChange={setCsMin} min={0.5} max={10} step={0.25} unit=" wu"/><Sl label="Max" value={csMax} onChange={setCsMax} min={0.5} max={15} step={0.25} unit=" wu"/></>}</div><div style={{marginTop:4,paddingTop:4,borderTop:"1px solid #2a2520"}}><Tg label="Cell Outlines" checked={borders} onChange={setBorders}/>{borders&&<><Sl label="Width" value={borderW} onChange={setBorderW} min={0.01} max={0.15} step={0.005}/><Sl label="Opacity" value={borderO} onChange={setBorderO} min={0} max={1} step={0.05}/><Ci label="Color" value={borderHex} onChange={setBorderHex}/></>}</div></>}
            {mode==="field"&&tab==="colors"&&<ColorsPanel/>}
            {mode==="field"&&tab==="export"&&<><div style={{marginBottom:12}}><div style={sL}>PNG</div><div style={{display:"flex",gap:3,marginBottom:5}}>{[256,512,1024,2048].map(r=>(<button key={r} onClick={()=>setFieldRes(r)} style={{flex:1,padding:"4px 0",fontSize:10,fontFamily:"monospace",background:fieldRes===r?"#3a3025":"transparent",border:`1px solid ${fieldRes===r?"#c49a6c":"#2a2520"}`,color:fieldRes===r?"#e8d5b0":"#5a5045",borderRadius:2,cursor:"pointer"}}>{r}</button>))}</div><button onClick={()=>dlPNG(fieldRef,`7t-field-${fieldSeed}-${fieldRes}.png`)} style={{width:"100%",padding:"5px 0",fontSize:10,fontFamily:"monospace",letterSpacing:".06em",textTransform:"uppercase",background:"transparent",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:3,cursor:"pointer"}}>Download PNG</button></div></>}
            {mode==="voronoi"&&tab==="design"&&<><div style={{marginBottom:8}}><div style={sL}>Seed</div><div style={{display:"flex",gap:4}}><input type="number" value={vorSeed} onChange={e=>setVorSeed(Number(e.target.value))} style={{flex:1,padding:"3px 5px",fontSize:12,fontFamily:"JetBrains Mono",background:"#0f0d0b",border:"1px solid #2a2520",color:"#e8d5b0",borderRadius:2,outline:"none"}}/><button onClick={()=>setVorSeed(Math.floor(Math.random()*99999))} style={{padding:"3px 7px",fontSize:9,fontFamily:"monospace",background:"#2a2520",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:2,cursor:"pointer"}}>DICE</button></div></div><ColorsPanel/><div style={sL}>Tessellation</div><Sl label="Cell count" value={vorCount} onChange={v=>setVorCount(Math.round(v))} min={16} max={1024} step={1}/><Sl label="Extent" value={vorExtent} onChange={setVorExtent} min={10} max={500} step={5}/><div style={{marginTop:6,paddingTop:6,borderTop:"1px solid #2a2520"}}><Tg label="Edge Outlines" checked={vorBorders} onChange={setVorBorders}/>{vorBorders&&<><Sl label="Width" value={vorBW} onChange={setVorBW} min={0.01} max={0.5} step={0.01}/><Sl label="Opacity" value={vorBO} onChange={setVorBO} min={0} max={1} step={0.05}/><Ci label="Color" value={vorBHex} onChange={setVorBHex}/></>}</div><div style={{marginTop:4,paddingTop:4,borderTop:"1px solid #2a2520"}}><div style={sL}>Scatter</div><Sl label="Cell dropout" value={scatter} onChange={setScatter} min={0} max={0.9} step={0.01}/></div></>}
            {mode==="voronoi"&&tab==="export"&&<><div style={{marginBottom:12}}><div style={sL}>PNG</div><div style={{display:"flex",gap:3,marginBottom:5}}>{[256,512,1024,2048].map(r=>(<button key={r} onClick={()=>setVorRes(r)} style={{flex:1,padding:"4px 0",fontSize:10,fontFamily:"monospace",background:vorRes===r?"#3a3025":"transparent",border:`1px solid ${vorRes===r?"#c49a6c":"#2a2520"}`,color:vorRes===r?"#e8d5b0":"#5a5045",borderRadius:2,cursor:"pointer"}}>{r}</button>))}</div><button onClick={()=>dlPNG(vorRef,`7t-voronoi-${vorSeed}-${vorRes}.png`)} style={{width:"100%",padding:"5px 0",fontSize:10,fontFamily:"monospace",letterSpacing:".06em",textTransform:"uppercase",background:"transparent",border:"1px solid #3a3025",color:"#c49a6c",borderRadius:3,cursor:"pointer"}}>Download PNG</button></div></>}
          </div>
          <div style={{padding:"8px 14px",borderTop:"1px solid #2a2520",flexShrink:0}}><button onClick={doGen} disabled={rendering&&mode!=="grid"} style={{width:"100%",padding:"8px 0",fontSize:11,fontFamily:"monospace",letterSpacing:"0.1em",textTransform:"uppercase",fontWeight:500,background:rendering&&mode!=="grid"?"#2a2520":"linear-gradient(135deg, #c49a6c, #a87a4c)",border:"none",color:rendering&&mode!=="grid"?"#6a5e4e":"#1a1714",borderRadius:3,cursor:rendering&&mode!=="grid"?"wait":"pointer"}}>{rendering&&mode!=="grid"?`Computing... ${progress}%`:"Generate"}</button></div>
        </div>

        <div style={{flex:1,display:"flex",alignItems:"center",justifyContent:"center",background:"#131110",padding:16,overflow:"auto"}}>
          <div style={{position:"relative"}}>
            {mode==="grid"&&<canvas ref={gridRef} style={{maxWidth:"calc(100vw - 330px)",maxHeight:"calc(100vh - 70px)",imageRendering:"pixelated",borderRadius:2,boxShadow:"0 6px 30px rgba(0,0,0,.5)"}}/>}
            {mode==="field"&&<><canvas ref={fieldRef} style={{maxWidth:"calc(100vw - 330px)",maxHeight:"calc(100vh - 70px)",imageRendering:"pixelated",borderRadius:2,boxShadow:"0 6px 30px rgba(0,0,0,.5)",display:showTile?"none":"block"}}/>{showTile&&<canvas ref={tileRef} style={{maxWidth:"calc(100vw - 330px)",maxHeight:"calc(100vh - 70px)",imageRendering:"pixelated",borderRadius:2,boxShadow:"0 6px 30px rgba(0,0,0,.5)"}}/>}</>}
            {mode==="voronoi"&&<canvas ref={vorRef} style={{maxWidth:"calc(100vw - 330px)",maxHeight:"calc(100vh - 70px)",imageRendering:"pixelated",borderRadius:2,boxShadow:"0 6px 30px rgba(0,0,0,.5)"}}/>}
            <div style={{position:"absolute",top:6,left:6,padding:"2px 6px",background:"rgba(26,23,20,.85)",borderRadius:3,border:"1px solid #2a2520"}}><span style={{fontSize:9,fontFamily:"monospace",color:"#c49a6c",letterSpacing:".05em"}}>{mode==="grid"?`GRID ${gridN}×${gridN}`:mode==="field"?"FIELD COMPOSITE":`VORONOI ${vorCount}`}{colorCount>0&&<span style={{color:"#5a5045"}}> — {colorCount} color{colorCount>1?"s":""}</span>}</span></div>
            {rendering&&mode!=="grid"&&<div style={{position:"absolute",inset:0,display:"flex",alignItems:"center",justifyContent:"center",background:"rgba(19,17,16,.85)",borderRadius:2}}><div style={{textAlign:"center"}}><div style={{color:"#c49a6c",fontFamily:"monospace",fontSize:11,letterSpacing:".08em",marginBottom:8}}>{mode==="voronoi"?"COMPUTING NEAREST NEIGHBOR":"EVALUATING LATTICE FIELDS"}</div><div style={{width:140,height:2,background:"#2a2520",borderRadius:1,overflow:"hidden"}}><div style={{height:"100%",background:"#c49a6c",width:`${progress}%`,transition:"width .15s"}}/></div></div></div>}
          </div>
        </div>
      </div>
    </div>
  );
}