import { useState, useEffect, useRef, useCallback } from "react";

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


/* ═══════════════════════════════════════════════════════════════════════
   7T COLUMN DESIGNER — v3
   Surface-of-revolution geometry workbench — 3 classical column tiers.
   Profile builder ported from world.wgsl column_mesh_gen.
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Pillar", "Doric", "Ornate"];

/* ═══ PALETTES (from cartridge.hpp) ═══ */
const COLUMN_PALETTE = [
  { name: "Light grey",   c: [0.82, 0.80, 0.78] },
  { name: "Warm limestone", c: [0.88, 0.83, 0.72] },
  { name: "Blue-grey slate", c: [0.55, 0.58, 0.63] },
  { name: "Terracotta",   c: [0.72, 0.45, 0.32] },
  { name: "Marble",       c: [0.90, 0.87, 0.82] },
  { name: "Dark basalt",  c: [0.35, 0.33, 0.32] },
  { name: "Mossy grey",   c: [0.52, 0.58, 0.48] },
];
const SANDSTONE = { c: [0.75, 0.68, 0.60], variance: 0.04 };

/* ═══ TIER DEFAULTS — mirrors cartridge.hpp COLUMN_TIERS[0..2] exactly ═══ */
function defaultTier(idx) {
  //                     h     σ    rad   σ    tap   σ    ent   σ    bL   σ    bH   σ    bO   σ    cL   σ    cH   σ    cO   σ    sp   σ    sH   σ    eb   σ    col%  bur  segA shR   wt
  const raw = [
    /* PILLAR  */  [  6.5, 1.2, 1.80,0.30, 1.00,0.00, 0.00,0.00,  1.0,0.3, 0.50,0.10, 0.20,0.05,  1.0,0.3, 0.40,0.08, 0.15,0.04,  0.30,0.08, 1.50,0.30, 0.40,0.08,  0.15,0.25,  16, 4, 0.05 ],
    /* DORIC   */  [  6.4, 1.2, 0.38,0.06, 0.85,0.03, 0.02,0.01,  0.0,0.0, 0.00,0.00, 0.00,0.00,  2.0,0.5, 0.50,0.10, 0.15,0.04,  0.20,0.05, 1.00,0.20, 0.30,0.05,  0.25,0.20,  20, 8, 0.20 ],
    /* ORNATE  */  [ 16.8, 2.8, 1.35,0.19, 0.82,0.03, 0.04,0.01,  2.0,0.5, 1.20,0.25, 0.30,0.08,  3.0,0.5, 1.50,0.30, 0.40,0.10,  0.40,0.10, 1.50,0.30, 0.50,0.10,  0.35,0.20,  28,12, 0.18 ],
  ];
  const r = raw[idx];
  return {
    height: r[0], height_s: r[1], shaft_r: r[2], shaft_r_s: r[3],
    taper: r[4], taper_s: r[5], entasis: r[6], entasis_s: r[7],
    base_layers: r[8], base_layers_s: r[9], base_h: r[10], base_h_s: r[11], base_oh: r[12], base_oh_s: r[13],
    cap_layers: r[14], cap_layers_s: r[15], cap_h: r[16], cap_h_s: r[17], cap_oh: r[18], cap_oh_s: r[19],
    solid_pad: r[20], solid_pad_s: r[21], solid_h: r[22], solid_h_s: r[23], edge_blend: r[24], edge_blend_s: r[25],
    color_over: r[26], burial: r[27], segs_around: r[28], shaft_rings: r[29], weight: r[30],
    color: [...SANDSTONE.c],
  };
}

/* ═══ PROFILE BUILDER — port from world.wgsl column_mesh_gen (classical branch) ═══ */
function buildProfile(T) {
  const prof = [];
  const col = T.color;
  const burial = T.burial * Math.max(T.solid_h, T.height * 0.02);
  const bl = Math.round(T.base_layers), cl = Math.round(T.cap_layers);
  const topR = T.shaft_r * T.taper;
  const baseTopY = T.base_h, shaftTopY = T.height - T.cap_h;
  const sr = Math.max(T.shaft_rings, 4);

  if (bl > 0 && T.base_h > 0.001) {
    for (let i = 0; i < bl; i++) {
      const t0 = i / bl, t1 = (i + 1) / bl;
      const r0 = T.shaft_r + T.base_oh * (1 - t0);
      prof.push({ r: r0, y: t0 * T.base_h - burial, color: col });
      prof.push({ r: r0, y: t1 * T.base_h - burial, color: col });
    }
    prof.push({ r: T.shaft_r, y: baseTopY - burial, color: col });
  } else {
    prof.push({ r: T.shaft_r, y: -burial, color: col });
  }

  for (let i = 0; i <= sr; i++) {
    const t = i / sr;
    const y = baseTopY + t * (shaftTopY - baseTopY);
    let r = T.shaft_r + (topR - T.shaft_r) * t;
    r += T.entasis * T.shaft_r * Math.sin(t * Math.PI);
    prof.push({ r, y: y - burial, color: col });
  }

  if (cl > 0 && T.cap_h > 0.001) {
    for (let i = 0; i < cl; i++) {
      const t0 = i / cl, t1 = (i + 1) / cl;
      const r0 = topR + T.cap_oh * t0;
      prof.push({ r: r0, y: shaftTopY + t0 * T.cap_h - burial, color: col });
      prof.push({ r: r0, y: shaftTopY + t1 * T.cap_h - burial, color: col });
    }
    prof.push({ r: topR + T.cap_oh, y: T.height - burial, color: col });
  } else {
    prof.push({ r: topR, y: T.height - burial, color: col });
  }

  return prof;
}

/* ═══ 3D RENDERER ═══ */
const RENDER_SEGS = 28;
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }
function lerpC(a, b, t) { return [a[0]+(b[0]-a[0])*t, a[1]+(b[1]-a[1])*t, a[2]+(b[2]-a[2])*t]; }

function render3D(ctx, W, H, profile, rotY, tilt) {
  ctx.clearRect(0, 0, W, H);
  if (profile.length < 2) return;
  const faces = [];
  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY), cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  let maxR=0, minY=Infinity, maxY=-Infinity;
  for (const p of profile) { maxR=Math.max(maxR,p.r); minY=Math.min(minY,p.y); maxY=Math.max(maxY,p.y); }
  const totalH=maxY-minY||1, midY=(minY+maxY)/2, extent=Math.max(maxR*2,totalH);
  const scale=Math.min(W,H)*0.44/(extent*0.5), cy=H*0.5;

  const rotV = (v) => { const vy=v[1]-midY; const x1=v[0]*cosR+v[2]*sinR,z1=-v[0]*sinR+v[2]*cosR; return [x1, vy*cosT-z1*sinT, vy*sinT+z1*cosT]; };
  const project=(v)=>[W/2+v[0]*scale, cy-v[1]*scale];

  const addTri = (v0, v1, v2, col) => {
    const verts = [v0, v1, v2];
    const e1 = sub3(v1, v0), e2 = sub3(v2, v0);
    const n = normalize3(cross3(e1, e2));
    if (n[2] > 0.02) return;
    const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
    faces.push({ verts, light: 0.25+0.75*diff, col, avgZ: (v0[2]+v1[2]+v2[2])/3, tri: true });
  };

  for (let r=0; r<profile.length-1; r++) {
    const p0=profile[r], p1=profile[r+1];
    if (p0.r<0.0001&&p1.r<0.0001) continue;
    for (let s=0; s<RENDER_SEGS; s++) {
      const a0=(s/RENDER_SEGS)*Math.PI*2, a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;
      const ca0=Math.cos(a0),sa0=Math.sin(a0),ca1=Math.cos(a1),sa1=Math.sin(a1);
      const verts=[rotV([ca0*p0.r,p0.y,sa0*p0.r]),rotV([ca1*p0.r,p0.y,sa1*p0.r]),rotV([ca1*p1.r,p1.y,sa1*p1.r]),rotV([ca0*p1.r,p1.y,sa0*p1.r])];
      const e1=sub3(verts[1],verts[0]),e2=sub3(verts[3],verts[0]),n=normalize3(cross3(e1,e2));
      if (n[2]>0.02) continue;
      const diff=Math.max(0,n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
      faces.push({ verts, light:0.25+0.75*diff, col:lerpC(p0.color,p1.color,0.5), avgZ:(verts[0][2]+verts[1][2]+verts[2][2]+verts[3][2])/4 });
    }
  }

  const topP=profile[profile.length-1];
  if (topP.r>0.001) for (let s=0;s<RENDER_SEGS;s++){
    const a0=(s/RENDER_SEGS)*Math.PI*2,a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;
    addTri(rotV([0,topP.y,0]), rotV([Math.cos(a0)*topP.r,topP.y,Math.sin(a0)*topP.r]), rotV([Math.cos(a1)*topP.r,topP.y,Math.sin(a1)*topP.r]), topP.color);
  }

  const botP=profile[0];
  if (botP.r>0.001) for (let s=0;s<RENDER_SEGS;s++){
    const a0=(s/RENDER_SEGS)*Math.PI*2,a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;
    addTri(rotV([0,botP.y,0]), rotV([Math.cos(a1)*botP.r,botP.y,Math.sin(a1)*botP.r]), rotV([Math.cos(a0)*botP.r,botP.y,Math.sin(a0)*botP.r]), botP.color);
  }

  for (let i=0; i<profile.length-1; i++) {
    const p0=profile[i], p1=profile[i+1];
    const dy = Math.abs(p1.y - p0.y);
    const dr = Math.abs(p1.r - p0.r);
    if (dy < 0.001 && dr > 0.001) {
      const rInner = Math.min(p0.r, p1.r), rOuter = Math.max(p0.r, p1.r);
      const yy = p0.y;
      const isTop = (p1.r > p0.r);
      for (let s=0; s<RENDER_SEGS; s++) {
        const a0=(s/RENDER_SEGS)*Math.PI*2, a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;
        const ca0=Math.cos(a0),sa0=Math.sin(a0),ca1=Math.cos(a1),sa1=Math.sin(a1);
        const v = isTop ? [
          rotV([ca0*rInner,yy,sa0*rInner]), rotV([ca1*rInner,yy,sa1*rInner]),
          rotV([ca1*rOuter,yy,sa1*rOuter]), rotV([ca0*rOuter,yy,sa0*rOuter]),
        ] : [
          rotV([ca0*rInner,yy,sa0*rInner]), rotV([ca0*rOuter,yy,sa0*rOuter]),
          rotV([ca1*rOuter,yy,sa1*rOuter]), rotV([ca1*rInner,yy,sa1*rInner]),
        ];
        const e1=sub3(v[1],v[0]),e2=sub3(v[3],v[0]),n=normalize3(cross3(e1,e2));
        if (n[2]>0.02) continue;
        const diff=Math.max(0,n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
        faces.push({ verts:v, light:0.22+0.68*diff, col:p0.color, avgZ:(v[0][2]+v[2][2])/2 });
      }
    }
  }

  faces.sort((a, b) => a.avgZ - b.avgZ);
  for (const f of faces) {
    const pts2 = f.verts.map(project);
    ctx.fillStyle = rgb01(f.col[0]*f.light, f.col[1]*f.light, f.col[2]*f.light);
    ctx.beginPath(); ctx.moveTo(pts2[0][0], pts2[0][1]);
    for (let i = 1; i < pts2.length; i++) ctx.lineTo(pts2[i][0], pts2[i][1]);
    ctx.closePath(); ctx.fill();
  }
}

/* ═══ 2D FRONT ELEVATION ═══ */
function render2D(ctx, W, H, profile) {
  ctx.clearRect(0,0,W,H); ctx.fillStyle="#111114"; ctx.fillRect(0,0,W,H);
  if (profile.length<2) return;
  let maxR=0,minY=Infinity,maxY=-Infinity;
  for(const p of profile){maxR=Math.max(maxR,p.r);minY=Math.min(minY,p.y);maxY=Math.max(maxY,p.y);}
  const mg=16,extent=Math.max(maxR*2.2,(maxY-minY)*1.1);
  const sc=Math.min((W-mg*2)/extent,(H-mg*2)/(maxY-minY||1));
  const cx=W/2,baseY=H-mg;
  const toS=(r,y)=>[cx+r*sc,baseY-(y-minY)*sc];

  ctx.lineWidth=1.5;
  for(let side=0;side<2;side++){const sign=side===0?1:-1;for(let i=0;i<profile.length-1;i++){const p0=profile[i],p1=profile[i+1];ctx.strokeStyle=rgb01(...lerpC(p0.color,p1.color,0.5));ctx.beginPath();const[x0,y0]=toS(p0.r*sign,p0.y);const[x1,y1]=toS(p1.r*sign,p1.y);ctx.moveTo(x0,y0);ctx.lineTo(x1,y1);ctx.stroke();}}

  ctx.fillStyle="rgba(200,154,108,0.6)";
  for(const p of profile){const[sx,sy]=toS(p.r,p.y);ctx.beginPath();ctx.arc(sx,sy,2,0,Math.PI*2);ctx.fill();}

  const[gx0,gy]=toS(-maxR*1.1,0);const[gx1]=toS(maxR*1.1,0);
  ctx.strokeStyle="rgba(100,200,100,0.25)";ctx.lineWidth=1;ctx.setLineDash([4,3]);ctx.beginPath();ctx.moveTo(gx0,gy);ctx.lineTo(gx1,gy);ctx.stroke();ctx.setLineDash([]);

  ctx.fillStyle="rgba(255,255,255,0.3)";ctx.font="9px monospace";ctx.textAlign="center";
  const[,topSy]=toS(0,maxY);
  ctx.fillText(`h=${(maxY-minY).toFixed(1)}`,cx,topSy-4);
  ctx.fillText(`r=${maxR.toFixed(2)}`,cx+maxR*sc*0.5,baseY+12);
}

/* ═══ UI COMPONENTS ═══ */
const ist={padding:"2px 3px",fontSize:11,fontFamily:"monospace",borderRadius:4,border:"1px solid var(--color-border-tertiary)",background:"var(--color-background-primary)",color:"var(--color-text-primary)",textAlign:"right"};

function Num({value,onChange,min=0,max=10,step=0.01,w=46}){
  const[txt,setTxt]=useState(String(Math.round(value*10000)/10000));
  const[focused,setFocused]=useState(false);
  useEffect(()=>{setTxt(String(Math.round(value*10000)/10000));},[value]);
  const commit=()=>{const v=parseFloat(txt);if(!isNaN(v))onChange(Math.max(min,Math.min(max,v)));else setTxt(String(Math.round(value*10000)/10000));setFocused(false);};
  return(<span style={{position:"relative",display:"inline-block"}}><input type="text" value={txt} title={`${min} – ${max}`} onChange={e=>setTxt(e.target.value)} onFocus={()=>setFocused(true)} onBlur={commit} onKeyDown={e=>{if(e.key==="Enter")e.target.blur();}} style={{...ist,width:w,outline:focused?"1.5px solid var(--color-border-info)":"none"}}/></span>);
}

function DragPanel({title,children,ini=false,id,resetKey,onDock}){
  const[open,setOpen]=useState(ini);const[pos,setPos]=useState(null);const[dragging,setDragging]=useState(false);
  const dragRef=useRef(null);const offsetRef=useRef({x:0,y:0});
  useEffect(()=>{setPos(null);},[resetKey]);
  const startDrag=useCallback(e=>{if(e.target.closest("[data-notdrag]"))return;e.preventDefault();const rect=dragRef.current.getBoundingClientRect();offsetRef.current={x:e.clientX-rect.left,y:e.clientY-rect.top};setDragging(true);},[]);
  useEffect(()=>{if(!dragging)return;const onMove=e=>{const p=dragRef.current.parentElement.getBoundingClientRect();setPos({x:e.clientX-p.left-offsetRef.current.x,y:e.clientY-p.top-offsetRef.current.y});};const onUp=()=>setDragging(false);window.addEventListener("mousemove",onMove);window.addEventListener("mouseup",onUp);return()=>{window.removeEventListener("mousemove",onMove);window.removeEventListener("mouseup",onUp);};},[dragging]);
  const doDock=()=>{setPos(null);if(onDock)onDock(id);};
  const style=pos?{position:"absolute",left:pos.x,top:pos.y,zIndex:100,maxWidth:520,minWidth:280}:{};
  return(<div ref={dragRef} style={{marginBottom:pos?0:5,border:pos?"1px solid #666":"1px solid var(--color-border-tertiary)",borderRadius:8,overflow:"hidden",background:pos?"#2a2a30":"var(--color-background-primary)",boxShadow:pos?"0 8px 32px rgba(0,0,0,.6)":"none",...style}}><div onMouseDown={startDrag} style={{padding:"5px 10px",fontSize:12,fontWeight:500,userSelect:"none",display:"flex",alignItems:"center",gap:5,color:pos?"#e0ddd8":"var(--color-text-primary)",background:pos?"#353540":"var(--color-background-secondary)",cursor:"grab"}}><span data-notdrag="1" onClick={()=>setOpen(!open)} style={{cursor:"pointer",fontSize:9,transition:"transform .15s",display:"inline-block",transform:open?"rotate(90deg)":"none",padding:"4px 2px"}}>▶</span><span style={{flex:1}}>{title}</span>{pos&&<span data-notdrag="1" onClick={doDock} style={{fontSize:9,cursor:"pointer",opacity:.5,padding:"2px 4px"}}>dock</span>}</div>{open&&<div style={{padding:"5px 10px",maxHeight:pos?400:"none",overflowY:pos?"auto":"visible",color:pos?"#d0cdc8":undefined}}>{children}</div>}</div>);
}

const rw={display:"flex",flexWrap:"wrap",gap:"3px 8px",alignItems:"center",marginBottom:3};
const lb={fontSize:10,color:"var(--color-text-secondary)",minWidth:56};
const btnStyle={fontSize:10,padding:"2px 8px",borderRadius:4,cursor:"pointer",border:"1px solid var(--color-border-tertiary)",background:"var(--color-background-secondary)",color:"var(--color-text-secondary)"};

function MuSigma({label, mu, muSet, sigma, sigmaSet, muMin=0, muMax=10, sMax=5, muW=50, sW=38}) {
  return (<div style={{display:"grid",gridTemplateColumns:"90px auto 16px auto",gap:"2px 4px",alignItems:"center",marginBottom:3}}>
    <span style={{fontSize:10,color:"var(--color-text-secondary)"}}>{label} μ</span>
    <Num value={mu} min={muMin} max={muMax} w={muW} onChange={muSet}/>
    <span style={{fontSize:10,color:"var(--color-text-tertiary)",textAlign:"center"}}>σ</span>
    <Num value={sigma} min={0} max={sMax} w={sW} onChange={sigmaSet}/>
  </div>);
}

function PaletteSwatches({palette, onPick, label}) {
  return (<div style={{marginBottom:4}}>
    <div style={{fontSize:9,color:"var(--color-text-tertiary)",marginBottom:2}}>{label}</div>
    <div style={{display:"flex",gap:2,flexWrap:"wrap"}}>
      {palette.map((p,i)=>(
        <span key={i} title={p.name} onClick={()=>onPick(p.c.slice())} style={{width:18,height:18,borderRadius:3,cursor:"pointer",background:rgb01(...p.c),border:"1px solid var(--color-border-tertiary)",display:"inline-block"}}/>
      ))}
    </div>
  </div>);
}

/* ═══ C++ EXPORT ═══ */
function genCpp(tiers) {
  const r=(v,d=2)=>(+v).toFixed(d)+"f";
  const lines = ["static constexpr ColumnTierParams COLUMN_TIERS[] = {"];
  tiers.forEach((T, i) => {
    lines.push(`    /* ${TIER_NAMES[i].toUpperCase().replace(/[. ]/g,"_").padEnd(10)} */  { ${r(T.height)}, ${r(T.height_s)},  ${r(T.shaft_r)}, ${r(T.shaft_r_s)},  ${r(T.taper)}, ${r(T.taper_s)},  ${r(T.entasis)}, ${r(T.entasis_s)},  `
      +`${r(T.base_layers,1)}, ${r(T.base_layers_s,1)},  ${r(T.base_h)}, ${r(T.base_h_s)},  ${r(T.base_oh)}, ${r(T.base_oh_s)},  `
      +`${r(T.cap_layers,1)}, ${r(T.cap_layers_s,1)},  ${r(T.cap_h)}, ${r(T.cap_h_s)},  ${r(T.cap_oh)}, ${r(T.cap_oh_s)},  `
      +`${r(T.solid_pad)}, ${r(T.solid_pad_s)},  ${r(T.solid_h)}, ${r(T.solid_h_s)},  ${r(T.edge_blend)}, ${r(T.edge_blend_s)},  `
      +`${r(T.color_over)}, ${r(T.burial)},  ${T.segs_around}, ${T.shaft_rings},  ${r(T.weight)} },`);
  });
  lines.push("};");
  return lines.join("\n");
}

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

const STORAGE_KEY = "7t:column:tiers";
const DEFAULTS = () => TIER_NAMES.map((_, i) => defaultTier(i));
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

/* ═══ MAIN ═══ */
export default function ColumnDesigner() {
  const [tierIdx, setTierIdx] = useState(1);
  const [tiers, setTiers] = useState(DEFAULTS);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const [rotY, setRotY] = useState(0.5);
  const [tilt, setTilt] = useState(0.12);
  const [dockKey, setDockKey] = useState(0);
  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);
  const defaultOrder = ["geometry", "base_cap", "solid", "appearance", "quality", "export"];
  const [panelOrder, setPanelOrder] = useState(defaultOrder);
  const [resizeTick, setResizeTick] = useState(0);

  const cv3dRef=useRef(null), cv2dRef=useRef(null), c3dRef=useRef(null), c2dRef=useRef(null);
  const upd = fn => setTiers(prev => { const n = JSON.parse(JSON.stringify(prev)); fn(n[tierIdx]); return n; });

  // Load saved tiers on mount
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) { const data = JSON.parse(raw); if (Array.isArray(data) && data.length === tiers.length) setTiers(data); }
      } catch (e) {}
      setLoaded(true);
    })();
  }, []);

  // Auto-save on every edit after initial load
  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(tiers)); } catch (e) {} })();
  }, [tiers, loaded]);
  const profile = buildProfile(T);

  useEffect(()=>{const ro=new ResizeObserver(()=>setResizeTick(t=>t+1));if(c3dRef.current)ro.observe(c3dRef.current);if(c2dRef.current)ro.observe(c2dRef.current);return()=>ro.disconnect();},[]);
  useEffect(()=>{const cv=cv3dRef.current;if(!cv)return;const dpr=window.devicePixelRatio||1;const W=cv.clientWidth,H=cv.clientHeight;cv.width=W*dpr;cv.height=H*dpr;const ctx=cv.getContext("2d");ctx.scale(dpr,dpr);render3D(ctx,W,H,profile,rotY,tilt);},[profile,rotY,tilt,resizeTick]);
  useEffect(()=>{const cv=cv2dRef.current;if(!cv)return;const dpr=window.devicePixelRatio||1;const W=cv.clientWidth,H=cv.clientHeight;cv.width=W*dpr;cv.height=H*dpr;const ctx=cv.getContext("2d");ctx.scale(dpr,dpr);render2D(ctx,W,H,profile);},[profile,resizeTick]);
  const onPointerDown3D=useCallback(e=>{const sx=e.clientX,sy=e.clientY,sr=rotY,st=tilt;const onMove=ev=>{setRotY(sr+(ev.clientX-sx)*0.01);setTilt(st-(ev.clientY-sy)*0.008);};const onUp=()=>{window.removeEventListener("pointermove",onMove);window.removeEventListener("pointerup",onUp);};window.addEventListener("pointermove",onMove);window.addEventListener("pointerup",onUp);},[rotY,tilt]);

  const resetAll = () => { setTiers(DEFAULTS()); setDockKey(k => k + 1); };
  const resetTier = () => setTiers(prev => { const n = [...prev]; n[tierIdx] = defaultTier(tierIdx); return n; });
  const cppCode=genCpp(tiers);

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{fontFamily:"'JetBrains Mono','SF Mono',monospace",color:"var(--color-text-primary)",lineHeight:1.4,position:"relative",fontSize:11}}>
      {/* TOP BAR */}
      <div style={{...rw,marginBottom:6,padding:"2px 0"}}>
        <span style={{fontSize:12,fontWeight:600,letterSpacing:"0.02em"}}>7T Column Designer</span>
        {isModified && <span style={{fontSize:9,color:"var(--color-text-tertiary)",marginLeft:6}}>● modified</span>}
        <div style={{marginLeft:"auto",display:"flex",gap:4}}>
          <button onClick={()=>{setDockKey(k=>k+1);setPanelOrder(defaultOrder);}} style={btnStyle}>Dock all</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
        </div>
      </div>

      {/* TIER SELECTOR */}
      <div style={{display:"flex",gap:3,marginBottom:6}}>
        {TIER_NAMES.map((name,i)=>(
          <button key={i} onClick={()=>setTierIdx(i)} style={{fontSize:10,padding:"3px 8px",borderRadius:4,cursor:"pointer",border:i===tierIdx?"2px solid var(--color-border-info)":"1px solid var(--color-border-tertiary)",background:i===tierIdx?"var(--color-background-info)":"var(--color-background-secondary)",color:i===tierIdx?"var(--color-text-info)":"var(--color-text-secondary)",fontWeight:i===tierIdx?600:400}}>{name}</button>
        ))}
      </div>

      {/* CANVASES */}
      <div style={{display:"flex",gap:6,marginBottom:6}}>
        <div ref={c3dRef} style={{flex:2,position:"relative",border:"1px solid var(--color-border-tertiary)",borderRadius:8,overflow:"hidden",background:"#0a0a0e",resize:"vertical",minHeight:180,height:380}}>
          <canvas ref={cv3dRef} onPointerDown={onPointerDown3D} style={{width:"100%",height:"100%",display:"block",cursor:"grab"}}/>
          <div style={{position:"absolute",bottom:4,left:8,fontSize:9,color:"rgba(255,255,255,0.3)"}}>drag to rotate · resize ↘</div>
          <div style={{position:"absolute",top:4,right:8,fontSize:9,color:"rgba(255,255,255,0.3)"}}>{TIER_NAMES[tierIdx]} · {profile.length} pts</div>
        </div>
        <div ref={c2dRef} style={{flex:1,minWidth:140,border:"1px solid var(--color-border-tertiary)",borderRadius:8,overflow:"hidden",resize:"vertical",minHeight:180,height:380}}>
          <canvas ref={cv2dRef} style={{width:"100%",height:"100%",display:"block"}}/>
        </div>
      </div>

      {/* PANELS */}
      {panelOrder.map(pid=>{
        const dp={key:pid,id:pid,resetKey:dockKey,onDock:(id)=>setPanelOrder(prev=>{const w=prev.filter(p=>p!==id);const oi=defaultOrder.indexOf(id);const at=w.findIndex(p=>defaultOrder.indexOf(p)>oi);return at===-1?[...w,id]:[...w.slice(0,at),id,...w.slice(at)];})};
        switch(pid){

        case "geometry": return (<DragPanel {...dp} title="Shaft Geometry" ini={true}>
          <MuSigma label="Height" mu={T.height} muSet={v=>upd(n=>{n.height=v;})} sigma={T.height_s} sigmaSet={v=>upd(n=>{n.height_s=v;})} muMin={1} muMax={200} sMax={50}/>
          <MuSigma label="Shaft radius" mu={T.shaft_r} muSet={v=>upd(n=>{n.shaft_r=v;})} sigma={T.shaft_r_s} sigmaSet={v=>upd(n=>{n.shaft_r_s=v;})} muMin={0.05} muMax={10} sMax={3}/>
          <MuSigma label="Taper" mu={T.taper} muSet={v=>upd(n=>{n.taper=v;})} sigma={T.taper_s} sigmaSet={v=>upd(n=>{n.taper_s=v;})} muMin={0.5} muMax={1.2} sMax={0.2}/>
          <MuSigma label="Entasis" mu={T.entasis} muSet={v=>upd(n=>{n.entasis=v;})} sigma={T.entasis_s} sigmaSet={v=>upd(n=>{n.entasis_s=v;})} muMax={0.15} sMax={0.05}/>
        </DragPanel>);

        case "base_cap": return (<DragPanel {...dp} title="Base & Capital">
          <div style={{fontSize:10,fontWeight:500,color:"var(--color-text-secondary)",marginBottom:3}}>Base</div>
          <MuSigma label="Layers" mu={T.base_layers} muSet={v=>upd(n=>{n.base_layers=Math.round(v);})} sigma={T.base_layers_s} sigmaSet={v=>upd(n=>{n.base_layers_s=v;})} muMax={4} sMax={1} muW={34}/>
          <MuSigma label="Height" mu={T.base_h} muSet={v=>upd(n=>{n.base_h=v;})} sigma={T.base_h_s} sigmaSet={v=>upd(n=>{n.base_h_s=v;})} muMax={3} sMax={1}/>
          <MuSigma label="Overhang" mu={T.base_oh} muSet={v=>upd(n=>{n.base_oh=v;})} sigma={T.base_oh_s} sigmaSet={v=>upd(n=>{n.base_oh_s=v;})} muMax={1} sMax={0.3}/>
          <div style={{fontSize:10,fontWeight:500,color:"var(--color-text-secondary)",margin:"6px 0 3px"}}>Capital</div>
          <MuSigma label="Layers" mu={T.cap_layers} muSet={v=>upd(n=>{n.cap_layers=Math.round(v);})} sigma={T.cap_layers_s} sigmaSet={v=>upd(n=>{n.cap_layers_s=v;})} muMax={5} sMax={1} muW={34}/>
          <MuSigma label="Height" mu={T.cap_h} muSet={v=>upd(n=>{n.cap_h=v;})} sigma={T.cap_h_s} sigmaSet={v=>upd(n=>{n.cap_h_s=v;})} muMax={3} sMax={1}/>
          <MuSigma label="Overhang" mu={T.cap_oh} muSet={v=>upd(n=>{n.cap_oh=v;})} sigma={T.cap_oh_s} sigmaSet={v=>upd(n=>{n.cap_oh_s=v;})} muMax={1} sMax={0.3}/>
        </DragPanel>);

        case "solid": return (<DragPanel {...dp} title="Collision Solid">
          <MuSigma label="Padding" mu={T.solid_pad} muSet={v=>upd(n=>{n.solid_pad=v;})} sigma={T.solid_pad_s} sigmaSet={v=>upd(n=>{n.solid_pad_s=v;})} muMax={5} sMax={1}/>
          <MuSigma label="Height" mu={T.solid_h} muSet={v=>upd(n=>{n.solid_h=v;})} sigma={T.solid_h_s} sigmaSet={v=>upd(n=>{n.solid_h_s=v;})} muMax={15} sMax={5}/>
          <MuSigma label="Edge blend" mu={T.edge_blend} muSet={v=>upd(n=>{n.edge_blend=v;})} sigma={T.edge_blend_s} sigmaSet={v=>upd(n=>{n.edge_blend_s=v;})} muMax={3} sMax={1}/>
          <div style={{fontSize:9,color:"var(--color-text-tertiary)",marginTop:2}}>Collision solid = invisible heightfield baked around the column base. Padding = extra radius beyond base. Edge blend = smoothstep falloff width.</div>
        </DragPanel>);

        case "appearance": return (<DragPanel {...dp} title="Appearance">
          <PaletteSwatches palette={COLUMN_PALETTE} label="Column palette (click to set color)" onPick={c=>upd(n=>{n.color=c;})}/>
          <PaletteSwatches palette={[{name:"Sandstone",c:SANDSTONE.c}]} label="Default sandstone" onPick={c=>upd(n=>{n.color=c;})}/>
          <div style={rw}>
            <span style={lb}>Color</span>
            {["R","G","B"].map((ch,i)=><><span key={"l"+i} style={{fontSize:9,color:"var(--color-text-tertiary)"}}>{ch}</span><Num key={i} value={T.color[i]} min={0} max={1} w={34} onChange={v=>upd(n=>{n.color[i]=v;})}/></>)}
            <span style={{width:18,height:18,borderRadius:4,display:"inline-block",background:rgb01(...T.color),border:"1px solid var(--color-border-tertiary)"}}/>
          </div>
          <div style={rw}>
            <span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v=>upd(n=>{n.color_over=v;})}/>
            <span style={lb}>Burial</span><Num value={T.burial} min={0} max={0.5} w={38} onChange={v=>upd(n=>{n.burial=v;})}/>
          </div>
          <div style={{fontSize:9,color:"var(--color-text-tertiary)",marginTop:2}}>Override = probability that this column uses a palette color instead of terrain sandstone.</div>
        </DragPanel>);

        case "quality": return (<DragPanel {...dp} title="Quality & Selection">
          <div style={rw}>
            <span style={lb}>Segs around</span><Num value={T.segs_around} min={8} max={48} step={1} w={34} onChange={v=>upd(n=>{n.segs_around=Math.round(v);})}/>
            <span style={lb}>Shaft rings</span><Num value={T.shaft_rings} min={2} max={16} step={1} w={34} onChange={v=>upd(n=>{n.shaft_rings=Math.round(v);})}/>
            <span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v=>upd(n=>{n.weight=v;})}/>
          </div>
          <div style={{fontSize:9,color:"var(--color-text-tertiary)",marginTop:2}}>Segs around = revolution segments (visual quality). Shaft rings = vertical subdivisions (affects entasis smoothness). Weight = tier selection probability (all must sum to 1.0).</div>
        </DragPanel>);

        case "export": return (<DragPanel {...dp} title="C++ Export">
          <div style={{display:"flex",gap:4,marginBottom:4}}>
            <button onClick={()=>{copyText(cppCode);setCopied(true);setTimeout(()=>setCopied(false),1500);}} style={{...btnStyle,background:copied?"var(--color-background-success)":"var(--color-background-secondary)"}}>{copied?"Copied":"Copy C++"}</button>
            <button onClick={()=>setShowCode(!showCode)} style={btnStyle}>{showCode?"Hide":"Show"} code</button>
          </div>
          {showCode&&<textarea readOnly value={cppCode} style={{width:"100%",height:80,fontSize:10,fontFamily:"'JetBrains Mono',monospace",background:"var(--color-background-primary)",color:"var(--color-text-primary)",border:"1px solid var(--color-border-tertiary)",borderRadius:4,padding:6,resize:"vertical",lineHeight:1.4,whiteSpace:"pre"}}/>}
        </DragPanel>);

        default: return null;
        }
      })}
    </div>
  );
}
