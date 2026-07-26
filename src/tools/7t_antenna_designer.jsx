import { useState, useEffect, useRef, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T ANTENNA DESIGNER — Antenna entities (Antenna, Squat, Colossal)
   Surface-of-revolution geometry workbench — 3 tiers.
   Profile builder ported from world.wgsl column_mesh_gen (antenna branch).
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Antenna", "Squat", "Colossal"];
const CPP_NAMES = ["ANTENNA", "ANT_SQT", "ANT_COL"];

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
const DRUM_PALETTE = [
  { name: "Terracotta",   c: [0.85, 0.55, 0.35] },
  { name: "Steel blue",   c: [0.45, 0.60, 0.70] },
  { name: "Ochre",        c: [0.70, 0.65, 0.45] },
  { name: "Sage",         c: [0.55, 0.70, 0.55] },
  { name: "Dusty rose",   c: [0.75, 0.50, 0.55] },
  { name: "Lavender grey", c: [0.60, 0.55, 0.68] },
];

/* ═══ TIER DEFAULTS — mirrors cartridge.hpp COLUMN_TIERS[3..5] ═══ */
function defaultTier(idx) {
  const raw = [
    /* ANTENNA */  [ 17.5, 3.5, 0.30,0.05, 0.85,0.05, 0.00,0.00,  2.0,0.5, 2.10,0.42, 1.50,0.30,  0.0,0.0, 1.50,0.30, 0.00,0.00,  0.20,0.05, 1.00,0.20, 0.30,0.05,  0.40,0.20,  16, 6, 0.10 ],
    /* ANT_SQT */  [ 32.5, 6.5, 0.90,0.15, 0.85,0.05, 0.00,0.00,  2.0,0.5, 2.00,0.40, 6.00,1.20,  0.0,0.0, 1.50,0.30, 0.00,0.00,  0.40,0.10, 1.50,0.30, 0.40,0.08,  0.40,0.20,  16, 6, 0.22 ],
    /* ANT_COL */  [125.0,25.0, 3.00,0.50, 0.85,0.05, 0.00,0.00,  2.0,0.5, 7.50,1.50,17.50,3.50,  0.0,0.0, 7.50,1.50, 0.00,0.00,  1.95,0.39,12.00,2.40, 1.00,0.20,  0.40,0.20,  20, 8, 0.13 ],
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
    drum_colors: [DRUM_PALETTE[0].c.slice(), DRUM_PALETTE[1].c.slice(), DRUM_PALETTE[2].c.slice()],
  };
}

/* ═══ PROFILE BUILDER — antenna (post + stacked drums) ═══ */
function buildProfile(T) {
  const prof = [];
  const col = T.color;
  const burial = T.burial * Math.max(T.solid_h, T.height * 0.02);
  const postR = T.shaft_r;
  const drumCount = Math.min(Math.round(T.base_layers), 3);
  const drumH = T.base_h, drumOH = T.base_oh, spacerH = T.cap_h, drumTaper = T.taper;
  const dc = T.drum_colors || [col, col, col];
  const contentH = drumCount * drumH + Math.max(drumCount - 1, 0) * spacerH;
  const drumStartY = T.height - Math.max(contentH, 0);

  prof.push({ r: postR, y: -burial, color: col });
  prof.push({ r: postR, y: drumStartY - burial, color: col });

  for (let d = 0; d < drumCount; d++) {
    const dyBase = drumStartY + d * (drumH + spacerH);
    const bottomR = postR + drumOH * (0.6 + 0.4 * ((d * 0.618 + T.segs_around * 0.1) % 1));
    const topR = bottomR * drumTaper;
    const dCol = dc[d] || col;
    prof.push({ r: bottomR, y: dyBase - burial, color: dCol });
    for (let ri = 1; ri <= 3; ri++) {
      const t = ri / 4;
      prof.push({ r: bottomR + (topR - bottomR) * t, y: dyBase + t * drumH - burial, color: dCol });
    }
    prof.push({ r: topR, y: dyBase + drumH - burial, color: dCol });
    if (d + 1 < drumCount) {
      prof.push({ r: postR, y: dyBase + drumH - burial, color: col });
      prof.push({ r: postR, y: dyBase + drumH + spacerH - burial, color: col });
    }
  }
  prof.push({ r: postR, y: T.height - burial, color: col });
  return prof;
}

/* ═══ 3D RENDERER ═══ */
const RENDER_SEGS = 28;
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }
function lerpC(a, b, t) { return [a[0]+(b[0]-a[0])*t, a[1]+(b[1]-a[1])*t, a[2]+(b[2]-a[2])*t]; }

function render3D(ctx, W, H, profile, rotY, tilt, zoom = 1, panX = 0, panY = 0, view = null) {
  if (!view?.noClear) ctx.clearRect(0, 0, W, H);
  if (profile.length < 2) return;
  const faces = [];
  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY), cosT = Math.cos(tilt), sinT = Math.sin(tilt);
  let maxR=0, minY=Infinity, maxY=-Infinity;
  for (const p of profile) { maxR=Math.max(maxR,p.r); minY=Math.min(minY,p.y); maxY=Math.max(maxY,p.y); }
  const totalH=maxY-minY||1, midY=(minY+maxY)/2, extent=Math.max(maxR*2,totalH);
  const scale=(view?.scale ?? Math.min(W,H)*0.44/(extent*0.5))*zoom, cy=H*0.5;
  const rotV = (v) => { const vy=v[1]-midY; const x1=v[0]*cosR+v[2]*sinR,z1=-v[0]*sinR+v[2]*cosR; return [x1, vy*cosT-z1*sinT, vy*sinT+z1*cosT]; };
  const project=(v)=>[W/2+panX+v[0]*scale, cy+panY-v[1]*scale];
  const addTri = (v0, v1, v2, col) => {
    const e1 = sub3(v1, v0), e2 = sub3(v2, v0), n = normalize3(cross3(e1, e2));
    if (n[2] > 0.02) return;
    const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
    faces.push({ verts: [v0, v1, v2], light: 0.25+0.75*diff, col, avgZ: (v0[2]+v1[2]+v2[2])/3 });
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
  if (topP.r>0.001) for (let s=0;s<RENDER_SEGS;s++){const a0=(s/RENDER_SEGS)*Math.PI*2,a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;addTri(rotV([0,topP.y,0]),rotV([Math.cos(a0)*topP.r,topP.y,Math.sin(a0)*topP.r]),rotV([Math.cos(a1)*topP.r,topP.y,Math.sin(a1)*topP.r]),topP.color);}
  const botP=profile[0];
  if (botP.r>0.001) for (let s=0;s<RENDER_SEGS;s++){const a0=(s/RENDER_SEGS)*Math.PI*2,a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;addTri(rotV([0,botP.y,0]),rotV([Math.cos(a1)*botP.r,botP.y,Math.sin(a1)*botP.r]),rotV([Math.cos(a0)*botP.r,botP.y,Math.sin(a0)*botP.r]),botP.color);}
  for (let i=0; i<profile.length-1; i++) {
    const p0=profile[i], p1=profile[i+1];
    if (Math.abs(p1.y-p0.y)<0.001 && Math.abs(p1.r-p0.r)>0.001) {
      const ri=Math.min(p0.r,p1.r),ro=Math.max(p0.r,p1.r),y=p0.y,col=lerpC(p0.color,p1.color,0.5),facesUp=p1.r<p0.r;
      for (let s=0;s<RENDER_SEGS;s++){const a0=(s/RENDER_SEGS)*Math.PI*2,a1=((s+1)%RENDER_SEGS)/RENDER_SEGS*Math.PI*2;const ca0=Math.cos(a0),sa0=Math.sin(a0),ca1=Math.cos(a1),sa1=Math.sin(a1);const vi=rotV([ca0*ri,y,sa0*ri]),vo=rotV([ca0*ro,y,sa0*ro]),vi1=rotV([ca1*ri,y,sa1*ri]),vo1=rotV([ca1*ro,y,sa1*ro]);if(facesUp){addTri(vi,vo,vo1,col);addTri(vi,vo1,vi1,col);}else{addTri(vi,vo1,vo,col);addTri(vi,vi1,vo1,col);}}
    }
  }
  faces.sort((a,b)=>a.avgZ-b.avgZ);
  for (const f of faces) { const pts=f.verts.map(project); ctx.fillStyle=rgb01(f.col[0]*f.light,f.col[1]*f.light,f.col[2]*f.light); ctx.beginPath(); ctx.moveTo(pts[0][0],pts[0][1]); for(let i=1;i<pts.length;i++) ctx.lineTo(pts[i][0],pts[i][1]); ctx.closePath(); ctx.fill(); }
}

function render2D(ctx, W, H, profile) {
  ctx.clearRect(0,0,W,H); ctx.fillStyle="#111114"; ctx.fillRect(0,0,W,H);
  if (profile.length<2) return;
  const mg={l:20,r:20,t:12,b:12}, pw=W-mg.l-mg.r, ph=H-mg.t-mg.b;
  let maxR=0,minY=Infinity,maxY=-Infinity;
  for(const p of profile){maxR=Math.max(maxR,p.r);minY=Math.min(minY,p.y);maxY=Math.max(maxY,p.y);}
  const totalH=maxY-minY||1, scaleY=ph/totalH, scaleR=pw/2/(maxR*1.1||1);
  const sc=Math.min(scaleY,scaleR), cx=W/2, baseY=mg.t+ph-(0-minY)*sc;
  const toS=(r,y)=>[cx+r*sc, baseY-(y-minY)*sc];
  ctx.fillStyle="rgba(130,120,100,0.15)"; ctx.beginPath();
  const [fx,fy]=toS(profile[0].r,profile[0].y); ctx.moveTo(fx,fy);
  for(let i=1;i<profile.length;i++){const[sx,sy]=toS(profile[i].r,profile[i].y);ctx.lineTo(sx,sy);}
  for(let i=profile.length-1;i>=0;i--){const[sx,sy]=toS(-profile[i].r,profile[i].y);ctx.lineTo(sx,sy);}
  ctx.closePath(); ctx.fill();
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
function Num({value,onChange,min=0,max=10,step=0.01,w=46}){const[txt,setTxt]=useState(String(Math.round(value*10000)/10000));const[focused,setFocused]=useState(false);useEffect(()=>{setTxt(String(Math.round(value*10000)/10000));},[value]);const commit=()=>{const v=parseFloat(txt);if(!isNaN(v))onChange(Math.max(min,Math.min(max,v)));else setTxt(String(Math.round(value*10000)/10000));setFocused(false);};return(<span style={{position:"relative",display:"inline-block"}}><input type="text" value={txt} title={`${min} – ${max}`} onChange={e=>setTxt(e.target.value)} onFocus={()=>setFocused(true)} onBlur={commit} onKeyDown={e=>{if(e.key==="Enter")e.target.blur();}} style={{...ist,width:w,outline:focused?"1.5px solid var(--color-border-info)":"none"}}/></span>);}
function DragPanel({title,children,ini=false,id,resetKey,onDock}){const[open,setOpen]=useState(ini);const[pos,setPos]=useState(null);const[dragging,setDragging]=useState(false);const dragRef=useRef(null);const offsetRef=useRef({x:0,y:0});useEffect(()=>{setPos(null);},[resetKey]);const startDrag=useCallback(e=>{if(e.target.closest("[data-notdrag]"))return;e.preventDefault();const rect=dragRef.current.getBoundingClientRect();offsetRef.current={x:e.clientX-rect.left,y:e.clientY-rect.top};setDragging(true);},[]);useEffect(()=>{if(!dragging)return;const onMove=e=>{const p=dragRef.current.parentElement.getBoundingClientRect();setPos({x:e.clientX-p.left-offsetRef.current.x,y:e.clientY-p.top-offsetRef.current.y});};const onUp=()=>setDragging(false);window.addEventListener("mousemove",onMove);window.addEventListener("mouseup",onUp);return()=>{window.removeEventListener("mousemove",onMove);window.removeEventListener("mouseup",onUp);};},[dragging]);const doDock=()=>{setPos(null);if(onDock)onDock(id);};const style=pos?{position:"absolute",left:pos.x,top:pos.y,zIndex:100,maxWidth:520,minWidth:280}:{};return(<div ref={dragRef} style={{marginBottom:pos?0:5,border:pos?"1px solid #666":"1px solid var(--color-border-tertiary)",borderRadius:8,overflow:"hidden",background:pos?"#2a2a30":"var(--color-background-primary)",boxShadow:pos?"0 8px 32px rgba(0,0,0,.6)":"none",...style}}><div onMouseDown={startDrag} style={{padding:"5px 10px",fontSize:12,fontWeight:500,userSelect:"none",display:"flex",alignItems:"center",gap:5,color:pos?"#e0ddd8":"var(--color-text-primary)",background:pos?"#353540":"var(--color-background-secondary)",cursor:"grab"}}><span data-notdrag="1" onClick={()=>setOpen(!open)} style={{cursor:"pointer",fontSize:9,transition:"transform .15s",display:"inline-block",transform:open?"rotate(90deg)":"none",padding:"4px 2px"}}>▶</span><span style={{flex:1}}>{title}</span>{pos&&<span data-notdrag="1" onClick={doDock} style={{fontSize:9,cursor:"pointer",opacity:.5,padding:"2px 4px"}}>dock</span>}</div>{open&&<div style={{padding:"5px 10px",maxHeight:pos?400:"none",overflowY:pos?"auto":"visible",color:pos?"#d0cdc8":undefined}}>{children}</div>}</div>);}
const rw={display:"flex",flexWrap:"wrap",gap:"3px 8px",alignItems:"center",marginBottom:3};
const lb={fontSize:10,color:"var(--color-text-secondary)",minWidth:56};
const btnStyle={fontSize:10,padding:"2px 8px",borderRadius:4,cursor:"pointer",border:"1px solid var(--color-border-tertiary)",background:"var(--color-background-secondary)",color:"var(--color-text-secondary)"};
function MuSigma({label, mu, muSet, sigma, sigmaSet, muMin=0, muMax=10, sMax=5, muW=50, sW=38}) {return (<div style={{display:"grid",gridTemplateColumns:"90px auto 16px auto",gap:"2px 4px",alignItems:"center",marginBottom:3}}><span style={{fontSize:10,color:"var(--color-text-secondary)"}}>{label} μ</span><Num value={mu} min={muMin} max={muMax} w={muW} onChange={muSet}/><span style={{fontSize:10,color:"var(--color-text-tertiary)",textAlign:"center"}}>σ</span><Num value={sigma} min={0} max={sMax} w={sW} onChange={sigmaSet}/></div>);}
function PaletteSwatches({palette, onPick, label}) {return (<div style={{marginBottom:4}}><div style={{fontSize:9,color:"var(--color-text-tertiary)",marginBottom:2}}>{label}</div><div style={{display:"flex",gap:2,flexWrap:"wrap"}}>{palette.map((p,i)=>(<span key={i} title={p.name} onClick={()=>onPick(p.c.slice())} style={{width:18,height:18,borderRadius:3,cursor:"pointer",background:rgb01(...p.c),border:"1px solid var(--color-border-tertiary)",display:"inline-block"}}/>))}</div></div>);}

/* ═══ C++ EXPORT ═══ */
function genCpp(tiers) {
  const r=(v,d=2)=>(+v).toFixed(d)+"f";
  const lines = ["// ─── Antenna Tier Definitions (COLUMN_TIERS[3..5]) ──────────────────────"];
  tiers.forEach((T, i) => {
    lines.push(`    /* ${CPP_NAMES[i].padEnd(10)} */  { ${r(T.height)}, ${r(T.height_s)},  ${r(T.shaft_r)}, ${r(T.shaft_r_s)},  ${r(T.taper)}, ${r(T.taper_s)},  ${r(T.entasis)}, ${r(T.entasis_s)},  `
      +`${r(T.base_layers,1)}, ${r(T.base_layers_s,1)},  ${r(T.base_h)}, ${r(T.base_h_s)},  ${r(T.base_oh)}, ${r(T.base_oh_s)},  `
      +`${r(T.cap_layers,1)}, ${r(T.cap_layers_s,1)},  ${r(T.cap_h)}, ${r(T.cap_h_s)},  ${r(T.cap_oh)}, ${r(T.cap_oh_s)},  `
      +`${r(T.solid_pad)}, ${r(T.solid_pad_s)},  ${r(T.solid_h)}, ${r(T.solid_h_s)},  ${r(T.edge_blend)}, ${r(T.edge_blend_s)},  `
      +`${r(T.color_over)}, ${r(T.burial)},  ${T.segs_around}, ${T.shaft_rings},  ${r(T.weight)} },`);
  });
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

const STORAGE_KEY = "7t:antenna:tiers";
const DEFAULTS = () => TIER_NAMES.map((_, i) => defaultTier(i));
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

/* ═══ MAIN ═══ */
export default function AntennaDesigner() {
  const [tierIdx, setTierIdx] = useState(0);
  const [tiers, setTiers] = useState(DEFAULTS);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const [rotY, setRotY] = useState(0.5);
  const [tilt, setTilt] = useState(0.12);
  const [zoom, setZoom] = useState(1.0);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);
  const [dockKey, setDockKey] = useState(0);
  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);
  const defaultOrder = ["geometry", "drums", "solid", "appearance", "quality", "export"];
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
  useEffect(()=>{const cv=cv3dRef.current;if(!cv)return;const dpr=window.devicePixelRatio||1;const W=cv.clientWidth,H=cv.clientHeight;cv.width=W*dpr;cv.height=H*dpr;const ctx=cv.getContext("2d");ctx.scale(dpr,dpr);render3D(ctx,W,H,profile,rotY,tilt,zoom,panX,panY);},[profile,rotY,tilt,resizeTick,zoom,panX,panY]);
  useEffect(()=>{const cv=cv2dRef.current;if(!cv)return;const dpr=window.devicePixelRatio||1;const W=cv.clientWidth,H=cv.clientHeight;cv.width=W*dpr;cv.height=H*dpr;const ctx=cv.getContext("2d");ctx.scale(dpr,dpr);render2D(ctx,W,H,profile);},[profile,resizeTick]);
  const onPointerDown3D=useCallback(e=>{const sx=e.clientX,sy=e.clientY;if(e.button===2){e.preventDefault();const spx=panX,spy=panY;const onMoveP=ev=>{setPanX(spx+(ev.clientX-sx));setPanY(spy+(ev.clientY-sy));};const onUpP=()=>{window.removeEventListener("pointermove",onMoveP);window.removeEventListener("pointerup",onUpP);};window.addEventListener("pointermove",onMoveP);window.addEventListener("pointerup",onUpP);return;}const sr=rotY,st=tilt;const onMove=ev=>{setRotY(sr+(ev.clientX-sx)*0.01);setTilt(st-(ev.clientY-sy)*0.008);};const onUp=()=>{window.removeEventListener("pointermove",onMove);window.removeEventListener("pointerup",onUp);};window.addEventListener("pointermove",onMove);window.addEventListener("pointerup",onUp);},[rotY,tilt,panX,panY]);
  // Scroll-wheel zoom on the 3D preview. The [loaded] dep is critical: at first
  // mount `loaded` is false and the canvas isn't in the DOM yet (the component
  // returns a Loading… placeholder), so the ref is null. When the load completes
  // and the canvas mounts, this effect re-runs and binds the listener for real.
  // Non-passive so we can preventDefault and stop the page from scrolling.
  useEffect(() => {
    const cv = cv3dRef.current;
    if (!cv) return;
    const onWheel = e => {
      e.preventDefault();
      setZoom(z => Math.max(0.1, Math.min(20, z * (e.deltaY > 0 ? 0.9 : 1.1))));
    };
    cv.addEventListener("wheel", onWheel, { passive: false });
    return () => cv.removeEventListener("wheel", onWheel);
  }, [loaded]);
  const resetAll = () => { setTiers(DEFAULTS()); setDockKey(k => k + 1); };
  const resetTier = () => setTiers(prev => { const n = [...prev]; n[tierIdx] = defaultTier(tierIdx); return n; });
  const cppCode=genCpp(tiers);

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{fontFamily:"'JetBrains Mono','SF Mono',monospace",color:"var(--color-text-primary)",lineHeight:1.4,position:"relative",fontSize:11}}>
      <div style={{...rw,marginBottom:6,padding:"2px 0"}}>
        <span style={{fontSize:12,fontWeight:600,letterSpacing:"0.02em"}}>7T Antenna Designer</span>
        {isModified && <span style={{fontSize:9,color:"var(--color-text-tertiary)",marginLeft:6}}>● modified</span>}
        <div style={{marginLeft:"auto",display:"flex",gap:4}}>
          <button onClick={()=>{setDockKey(k=>k+1);setPanelOrder(defaultOrder);}} style={btnStyle}>Dock all</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
        </div>
      </div>
      <div style={{display:"flex",gap:3,marginBottom:6,flexWrap:"wrap"}}>
        {TIER_NAMES.map((name,i)=>(<button key={i} onClick={()=>setTierIdx(i)} style={{fontSize:10,padding:"3px 8px",borderRadius:4,cursor:"pointer",border:i===tierIdx?"2px solid var(--color-border-info)":"1px solid var(--color-border-tertiary)",background:i===tierIdx?"var(--color-background-info)":"var(--color-background-secondary)",color:i===tierIdx?"var(--color-text-info)":"var(--color-text-secondary)",fontWeight:i===tierIdx?600:400}}>{name}</button>))}
      </div>
      <div style={{display:"flex",gap:6,marginBottom:6}}>
        <div ref={c3dRef} style={{flex:2,position:"relative",border:"1px solid var(--color-border-tertiary)",borderRadius:8,overflow:"hidden",background:"#0a0a0e",resize:"vertical",minHeight:180,height:380}}>
          <canvas ref={cv3dRef} onPointerDown={onPointerDown3D} onContextMenu={e=>e.preventDefault()} style={{width:"100%",height:"100%",display:"block",cursor:"grab"}}/>
          <div
            onClick={() => { setZoom(1); setPanX(0); setPanY(0); }}
            title="Click to reset view"
            style={{ position: "absolute", top: 4, right: 8, fontSize: 9,
                     color: "rgba(255,255,255,0.4)", cursor: "pointer",
                     userSelect: "none", padding: "2px 4px" }}
          >{Math.round(zoom * 100)}%</div>
          <div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9,
                        color: "rgba(255,255,255,0.3)" }}>drag to rotate · right-drag to pan · scroll to zoom</div>
          <div style={{position:"absolute",bottom:4,left:8,fontSize:9,color:"rgba(255,255,255,0.3)"}}>drag to rotate · resize ↘</div>
          <div style={{position:"absolute",top:4,right:8,fontSize:9,color:"rgba(255,255,255,0.3)"}}>{TIER_NAMES[tierIdx]} · {profile.length} pts</div>
        </div>
        <div ref={c2dRef} style={{flex:1,minWidth:140,border:"1px solid var(--color-border-tertiary)",borderRadius:8,overflow:"hidden",resize:"vertical",minHeight:180,height:380}}>
          <canvas ref={cv2dRef} style={{width:"100%",height:"100%",display:"block"}}/>
        </div>
      </div>
      {panelOrder.map(pid=>{
        const dp={key:pid,id:pid,resetKey:dockKey,onDock:(id)=>setPanelOrder(prev=>{const w=prev.filter(p=>p!==id);const oi=defaultOrder.indexOf(id);const at=w.findIndex(p=>defaultOrder.indexOf(p)>oi);return at===-1?[...w,id]:[...w.slice(0,at),id,...w.slice(at)];})};
        switch(pid){
        case "geometry": return (<DragPanel {...dp} title="Post Geometry" ini={true}>
          <MuSigma label="Height" mu={T.height} muSet={v=>upd(n=>{n.height=v;})} sigma={T.height_s} sigmaSet={v=>upd(n=>{n.height_s=v;})} muMin={1} muMax={200} sMax={50}/>
          <MuSigma label="Post radius" mu={T.shaft_r} muSet={v=>upd(n=>{n.shaft_r=v;})} sigma={T.shaft_r_s} sigmaSet={v=>upd(n=>{n.shaft_r_s=v;})} muMin={0.05} muMax={10} sMax={3}/>
          <MuSigma label="Drum taper" mu={T.taper} muSet={v=>upd(n=>{n.taper=v;})} sigma={T.taper_s} sigmaSet={v=>upd(n=>{n.taper_s=v;})} muMin={0.5} muMax={1.2} sMax={0.2}/>
        </DragPanel>);
        case "drums": return (<DragPanel {...dp} title="Drum Configuration">
          <MuSigma label="Count" mu={T.base_layers} muSet={v=>upd(n=>{n.base_layers=Math.round(v);})} sigma={T.base_layers_s} sigmaSet={v=>upd(n=>{n.base_layers_s=v;})} muMin={1} muMax={3} sMax={1} muW={34}/>
          <MuSigma label="Drum height" mu={T.base_h} muSet={v=>upd(n=>{n.base_h=v;})} sigma={T.base_h_s} sigmaSet={v=>upd(n=>{n.base_h_s=v;})} muMin={0.5} muMax={20} sMax={5}/>
          <MuSigma label="Overhang" mu={T.base_oh} muSet={v=>upd(n=>{n.base_oh=v;})} sigma={T.base_oh_s} sigmaSet={v=>upd(n=>{n.base_oh_s=v;})} muMin={0.1} muMax={25} sMax={5}/>
          <MuSigma label="Spacer height" mu={T.cap_h} muSet={v=>upd(n=>{n.cap_h=v;})} sigma={T.cap_h_s} sigmaSet={v=>upd(n=>{n.cap_h_s=v;})} muMin={0.1} muMax={10} sMax={3}/>
          <div style={{fontSize:10,fontWeight:500,color:"var(--color-text-secondary)",margin:"6px 0 3px"}}>Drum Colors</div>
          <PaletteSwatches palette={DRUM_PALETTE} label="Drum palette (click to apply to selected drum)" onPick={()=>{}}/>
          {[0,1,2].filter(i=>i<Math.round(T.base_layers)).map(i=>(<div key={i} style={rw}>
            <span style={{...lb,minWidth:44}}>Drum {i+1}</span>
            {["R","G","B"].map((ch,ci)=><><span key={"l"+ci} style={{fontSize:9,color:"var(--color-text-tertiary)"}}>{ch}</span><Num key={ci} value={T.drum_colors[i][ci]} min={0} max={1} w={34} onChange={v=>upd(n=>{n.drum_colors[i][ci]=v;})}/></>)}
            <span style={{width:14,height:14,borderRadius:3,display:"inline-block",background:rgb01(...T.drum_colors[i]),border:"1px solid var(--color-border-tertiary)"}}/>
            <div style={{display:"flex",gap:1,marginLeft:4}}>{DRUM_PALETTE.map((dp,pi)=>(<span key={pi} title={dp.name} onClick={()=>upd(n=>{n.drum_colors[i]=dp.c.slice();})} style={{width:10,height:10,borderRadius:2,cursor:"pointer",background:rgb01(...dp.c),border:"1px solid var(--color-border-tertiary)",display:"inline-block"}}/>))}</div>
          </div>))}
        </DragPanel>);
        case "solid": return (<DragPanel {...dp} title="Collision Solid">
          <MuSigma label="Padding" mu={T.solid_pad} muSet={v=>upd(n=>{n.solid_pad=v;})} sigma={T.solid_pad_s} sigmaSet={v=>upd(n=>{n.solid_pad_s=v;})} muMax={5} sMax={1}/>
          <MuSigma label="Height" mu={T.solid_h} muSet={v=>upd(n=>{n.solid_h=v;})} sigma={T.solid_h_s} sigmaSet={v=>upd(n=>{n.solid_h_s=v;})} muMax={15} sMax={5}/>
          <MuSigma label="Edge blend" mu={T.edge_blend} muSet={v=>upd(n=>{n.edge_blend=v;})} sigma={T.edge_blend_s} sigmaSet={v=>upd(n=>{n.edge_blend_s=v;})} muMax={3} sMax={1}/>
        </DragPanel>);
        case "appearance": return (<DragPanel {...dp} title="Appearance">
          <PaletteSwatches palette={COLUMN_PALETTE} label="Post palette (click to set color)" onPick={c=>upd(n=>{n.color=c;})}/>
          <PaletteSwatches palette={[{name:"Sandstone",c:SANDSTONE.c}]} label="Default sandstone" onPick={c=>upd(n=>{n.color=c;})}/>
          <div style={rw}><span style={lb}>Color</span>{["R","G","B"].map((ch,i)=><><span key={"l"+i} style={{fontSize:9,color:"var(--color-text-tertiary)"}}>{ch}</span><Num key={i} value={T.color[i]} min={0} max={1} w={34} onChange={v=>upd(n=>{n.color[i]=v;})}/></>)}<span style={{width:18,height:18,borderRadius:4,display:"inline-block",background:rgb01(...T.color),border:"1px solid var(--color-border-tertiary)"}}/></div>
          <div style={rw}><span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v=>upd(n=>{n.color_over=v;})}/><span style={lb}>Burial</span><Num value={T.burial} min={0} max={0.5} w={38} onChange={v=>upd(n=>{n.burial=v;})}/></div>
        </DragPanel>);
        case "quality": return (<DragPanel {...dp} title="Quality & Selection">
          <div style={rw}><span style={lb}>Segs around</span><Num value={T.segs_around} min={8} max={48} step={1} w={34} onChange={v=>upd(n=>{n.segs_around=Math.round(v);})}/><span style={lb}>Shaft rings</span><Num value={T.shaft_rings} min={2} max={16} step={1} w={34} onChange={v=>upd(n=>{n.shaft_rings=Math.round(v);})}/><span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v=>upd(n=>{n.weight=v;})}/></div>
        </DragPanel>);
        case "export": return (<DragPanel {...dp} title="C++ Export">
          <div style={{display:"flex",gap:4,marginBottom:4}}>
            <button onClick={()=>{navigator.clipboard?.writeText(cppCode);setCopied(true);setTimeout(()=>setCopied(false),1500);}} style={{...btnStyle,background:copied?"var(--color-background-success)":"var(--color-background-secondary)"}}>{copied?"Copied":"Copy C++"}</button>
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
