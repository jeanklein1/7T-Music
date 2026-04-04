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
   7T PYRAMID DESIGNER
   Truncated quad solid — 3 tiers (Obelisk / Temple / Colossus).
   Parameters from cartridge.hpp PYRAMID_TIERS[].
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Obelisk", "Temple", "Colossus"];

/* ═══ COLOR (from cartridge.hpp — no palette, sandstone only) ═══ */
const PYRAMID_SANDSTONE = { name: "Sandstone", c: [0.80, 0.72, 0.58], variance: 0.05 };

function defaultTier(idx) {
  const D = [
    { height: 28.0, height_s: 6.0, base_half: 16.0, base_half_s: 3.0, aspect: 1.0, aspect_s: 0.15, trunc: 0.00, trunc_s: 0.00, edge_blend: 1.5, edge_blend_s: 0.3, color_over: 0.10, color_var: 0.04, weight: 0.50, color: [0.80, 0.72, 0.58] },
    { height: 45.0, height_s: 8.0, base_half: 40.0, base_half_s: 6.0, aspect: 1.0, aspect_s: 0.20, trunc: 0.25, trunc_s: 0.08, edge_blend: 3.0, edge_blend_s: 0.75, color_over: 0.15, color_var: 0.04, weight: 0.35, color: [0.80, 0.72, 0.58] },
    { height: 78.0, height_s: 14.4, base_half: 60.0, base_half_s: 9.6, aspect: 1.0, aspect_s: 0.10, trunc: 0.05, trunc_s: 0.04, edge_blend: 3.6, edge_blend_s: 1.0, color_over: 0.20, color_var: 0.04, weight: 0.15, color: [0.80, 0.72, 0.58] },
  ];
  return JSON.parse(JSON.stringify(D[idx]));
}

/* ═══ 3D RENDERER ═══ */
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }

function render3D(ctx, W, H, T, rotY, tilt) {
  ctx.clearRect(0, 0, W, H);
  const bx = T.base_half, bz = T.base_half * T.aspect;
  const tx = bx * T.trunc, tz = bz * T.trunc;
  const h = T.height;
  const col = T.color;

  // 8 corners: base (y=0) and top (y=h)
  const verts = [
    [-bx, 0, -bz], [bx, 0, -bz], [bx, 0, bz], [-bx, 0, bz],  // base
    [-tx, h, -tz], [tx, h, -tz], [tx, h, tz], [-tx, h, tz],      // top
  ];

  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  const extent = Math.max(bx * 2, bz * 2, h);
  const scale = Math.min(W, H) * 0.40 / (extent * 0.5);
  const midY = h / 2;
  // Subtract midY BEFORE rotation so the pyramid spins around its center
  const rotVert = (v) => { const vy=v[1]-midY; const x1=v[0]*cosR+v[2]*sinR; const z1=-v[0]*sinR+v[2]*cosR; return [x1, vy*cosT-z1*sinT, vy*sinT+z1*cosT]; };
  const project = (v) => [W/2 + v[0]*scale, H/2 - v[1]*scale];

  const faceIdx = [[0,3,2,1],[4,5,6,7],[0,1,5,4],[2,3,7,6],[1,2,6,5],[0,4,7,3]];
  const faceShade = [0.5, 1.0, 0.85, 0.85, 0.9, 0.9]; // base darker

  const faces = [];
  for (let fi = 0; fi < 6; fi++) {
    const fv = faceIdx[fi].map(ci => rotVert(verts[ci]));
    const e1 = sub3(fv[1], fv[0]), e2 = sub3(fv[3], fv[0]);
    const n = normalize3(cross3(e1, e2));
    if (n[2] > 0.02) continue;
    const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
    const light = 0.22 + 0.78 * diff * faceShade[fi];
    const avgZ = fv.reduce((s, v) => s + v[2], 0) / fv.length;
    faces.push({ verts: fv, light, avgZ });
  }

  faces.sort((a, b) => a.avgZ - b.avgZ);
  for (const f of faces) {
    const pts = f.verts.map(project);
    ctx.fillStyle = rgb01(col[0]*f.light, col[1]*f.light, col[2]*f.light);
    ctx.beginPath(); ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath(); ctx.fill();
    ctx.strokeStyle = rgb01(col[0]*f.light*0.7, col[1]*f.light*0.7, col[2]*f.light*0.7);
    ctx.lineWidth = 0.5; ctx.stroke();
  }
}

/* ═══ 2D CROSS-SECTION ═══ */
function render2D(ctx, W, H, T) {
  ctx.clearRect(0, 0, W, H); ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const bx = T.base_half, tx = bx * T.trunc, h = T.height;
  const extent = Math.max(bx * 2, h); const mg = 20;
  const sc = Math.min((W - mg*2) / (bx * 2.2), (H - mg*2) / (h * 1.1));
  const cx = W / 2, baseY = H - mg;
  const toS = (x, y) => [cx + x * sc, baseY - y * sc];

  // Fill
  ctx.fillStyle = "rgba(130,120,100,0.15)";
  ctx.beginPath();
  const [x0, y0] = toS(-bx, 0); ctx.moveTo(x0, y0);
  const [x1, y1] = toS(bx, 0); ctx.lineTo(x1, y1);
  const [x2, y2] = toS(tx, h); ctx.lineTo(x2, y2);
  const [x3, y3] = toS(-tx, h); ctx.lineTo(x3, y3);
  ctx.closePath(); ctx.fill();

  // Outline
  ctx.strokeStyle = rgb01(T.color[0], T.color[1], T.color[2]); ctx.lineWidth = 1.5;
  ctx.beginPath(); ctx.moveTo(x0, y0); ctx.lineTo(x1, y1); ctx.lineTo(x2, y2); ctx.lineTo(x3, y3); ctx.closePath(); ctx.stroke();

  // Ground line
  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(toS(-bx*1.1, 0)[0], toS(0, 0)[1]); ctx.lineTo(toS(bx*1.1, 0)[0], toS(0, 0)[1]); ctx.stroke(); ctx.setLineDash([]);

  // Dims
  ctx.fillStyle = "rgba(255,255,255,0.3)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  ctx.fillText(`h=${h.toFixed(1)}  base=${bx.toFixed(1)}  trunc=${(T.trunc*100).toFixed(0)}%`, cx, toS(0, h)[1] - 6);
}

/* ═══ UI ═══ */
const ist = { padding: "2px 3px", fontSize: 11, fontFamily: "monospace", borderRadius: 4, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)", textAlign: "right" };
function Num({ value, onChange, min = 0, max = 10, step = 0.01, w = 46 }) {
  const [txt, setTxt] = useState(String(Math.round(value * 10000) / 10000));
  const [focused, setFocused] = useState(false);
  useEffect(() => { setTxt(String(Math.round(value * 10000) / 10000)); }, [value]);
  const commit = () => { const v = parseFloat(txt); if (!isNaN(v)) onChange(Math.max(min, Math.min(max, v))); else setTxt(String(Math.round(value * 10000) / 10000)); setFocused(false); };
  return (<span style={{ position: "relative", display: "inline-block" }}><input type="text" value={txt} title={`${min} – ${max}`} onChange={e => setTxt(e.target.value)} onFocus={() => setFocused(true)} onBlur={commit} onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }} style={{ ...ist, width: w, outline: focused ? "1.5px solid var(--color-border-info)" : "none" }} /></span>);
}

function DragPanel({ title, children, ini = false, id, resetKey, onDock }) {
  const [open, setOpen] = useState(ini); const [pos, setPos] = useState(null); const [dragging, setDragging] = useState(false);
  const dragRef = useRef(null); const offsetRef = useRef({ x: 0, y: 0 });
  useEffect(() => { setPos(null); }, [resetKey]);
  const startDrag = useCallback(e => { if (e.target.closest("[data-notdrag]")) return; e.preventDefault(); const rect = dragRef.current.getBoundingClientRect(); offsetRef.current = { x: e.clientX - rect.left, y: e.clientY - rect.top }; setDragging(true); }, []);
  useEffect(() => { if (!dragging) return; const onMove = e => { const p = dragRef.current.parentElement.getBoundingClientRect(); setPos({ x: e.clientX - p.left - offsetRef.current.x, y: e.clientY - p.top - offsetRef.current.y }); }; const onUp = () => setDragging(false); window.addEventListener("mousemove", onMove); window.addEventListener("mouseup", onUp); return () => { window.removeEventListener("mousemove", onMove); window.removeEventListener("mouseup", onUp); }; }, [dragging]);
  const style = pos ? { position: "absolute", left: pos.x, top: pos.y, zIndex: 100, maxWidth: 520, minWidth: 280 } : {};
  return (<div ref={dragRef} style={{ marginBottom: pos ? 0 : 5, border: pos ? "1px solid #666" : "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: pos ? "#2a2a30" : "var(--color-background-primary)", boxShadow: pos ? "0 8px 32px rgba(0,0,0,.6)" : "none", ...style }}><div onMouseDown={startDrag} style={{ padding: "5px 10px", fontSize: 12, fontWeight: 500, userSelect: "none", display: "flex", alignItems: "center", gap: 5, color: pos ? "#e0ddd8" : "var(--color-text-primary)", background: pos ? "#353540" : "var(--color-background-secondary)", cursor: "grab" }}><span data-notdrag="1" onClick={() => setOpen(!open)} style={{ cursor: "pointer", fontSize: 9, transition: "transform .15s", display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px" }}>▶</span><span style={{ flex: 1 }}>{title}</span>{pos && <span data-notdrag="1" onClick={() => { setPos(null); if (onDock) onDock(id); }} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}</div>{open && <div style={{ padding: "5px 10px", maxHeight: pos ? 400 : "none", overflowY: pos ? "auto" : "visible", color: pos ? "#d0cdc8" : undefined }}>{children}</div>}</div>);
}

const rw = { display: "flex", flexWrap: "wrap", gap: "3px 8px", alignItems: "center", marginBottom: 3 };
const ms = { display: "grid", gridTemplateColumns: "90px auto 16px auto", gap: "2px 4px", alignItems: "center", marginBottom: 3 };
const lb = { fontSize: 10, color: "var(--color-text-secondary)", minWidth: 56 };
const sLb = { fontSize: 10, color: "var(--color-text-tertiary)", textAlign: "center" };
const btnStyle = { fontSize: 10, padding: "2px 8px", borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-secondary)" };

function genCpp(tiers) {
  const r = (v, d=2) => v.toFixed(d) + "f";
  const lines = ["static constexpr PyramidTierParams PYRAMID_TIERS[] = {"];
  tiers.forEach((T, i) => {
    lines.push(`    /* ${TIER_NAMES[i].toUpperCase().padEnd(10)} */  { ${r(T.height)}, ${r(T.height_s)},  ${r(T.base_half)}, ${r(T.base_half_s)},  ${r(T.aspect)}, ${r(T.aspect_s)},  ${r(T.trunc)}, ${r(T.trunc_s)},  ${r(T.edge_blend)}, ${r(T.edge_blend_s)},  ${r(T.color_over)}, ${r(T.color_var)},  ${r(T.weight)} },`);
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

const STORAGE_KEY = "7t:pyramid:tiers";
const DEFAULTS = () => TIER_NAMES.map((_, i) => defaultTier(i));
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

export default function PyramidDesigner() {
  const [tierIdx, setTierIdx] = useState(1);
  const [tiers, setTiers] = useState(DEFAULTS);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const [rotY, setRotY] = useState(0.5); const [tilt, setTilt] = useState(0.2);
  const [dockKey, setDockKey] = useState(0);
  const [showCode, setShowCode] = useState(false); const [copied, setCopied] = useState(false);
  const defaultOrder = ["geometry", "appearance", "export"];
  const [panelOrder, setPanelOrder] = useState(defaultOrder);
  const [resizeTick, setResizeTick] = useState(0);
  const cv3dRef = useRef(null), cv2dRef = useRef(null), c3dRef = useRef(null), c2dRef = useRef(null);
  const upd = fn => setTiers(prev => { const n = JSON.parse(JSON.stringify(prev)); fn(n[tierIdx]); return n; });

  // Load saved tiers on mount
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) {
          const data = JSON.parse(raw);
          if (Array.isArray(data) && data.length === TIER_NAMES.length) setTiers(data);
        }
      } catch (e) { /* no saved data */ }
      setLoaded(true);
    })();
  }, []);

  // Auto-save on every edit after initial load
  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(tiers)); } catch (e) {} })();
  }, [tiers, loaded]);

  useEffect(() => { const ro = new ResizeObserver(() => setResizeTick(t => t + 1)); if (c3dRef.current) ro.observe(c3dRef.current); if (c2dRef.current) ro.observe(c2dRef.current); return () => ro.disconnect(); }, []);
  useEffect(() => { const cv = cv3dRef.current; if (!cv) return; const dpr = window.devicePixelRatio || 1; const W = cv.clientWidth, H = cv.clientHeight; cv.width = W * dpr; cv.height = H * dpr; const ctx = cv.getContext("2d"); ctx.scale(dpr, dpr); render3D(ctx, W, H, T, rotY, tilt); }, [T, rotY, tilt, resizeTick]);
  useEffect(() => { const cv = cv2dRef.current; if (!cv) return; const dpr = window.devicePixelRatio || 1; const W = cv.clientWidth, H = cv.clientHeight; cv.width = W * dpr; cv.height = H * dpr; const ctx = cv.getContext("2d"); ctx.scale(dpr, dpr); render2D(ctx, W, H, T); }, [T, resizeTick]);
  const onPointerDown3D = useCallback(e => { const sx = e.clientX, sy = e.clientY, sr = rotY, st = tilt; const onMove = ev => { setRotY(sr + (ev.clientX - sx) * 0.01); setTilt(st - (ev.clientY - sy) * 0.008); }; const onUp = () => { window.removeEventListener("pointermove", onMove); window.removeEventListener("pointerup", onUp); }; window.addEventListener("pointermove", onMove); window.addEventListener("pointerup", onUp); }, [rotY, tilt]);

  const resetAll = () => { setTiers(DEFAULTS()); setDockKey(k => k + 1); };
  const resetTier = () => setTiers(prev => { const n = [...prev]; n[tierIdx] = defaultTier(tierIdx); return n; });
  const cppCode = genCpp(tiers);

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{ fontFamily: "'JetBrains Mono', 'SF Mono', monospace", color: "var(--color-text-primary)", lineHeight: 1.4, position: "relative", fontSize: 11 }}>
      <div style={{ ...rw, marginBottom: 6, padding: "2px 0" }}>
        <span style={{ fontSize: 12, fontWeight: 600 }}>7T Pyramid Designer</span>
        {isModified && <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 6 }}>● modified</span>}
        <div style={{ marginLeft: "auto", display: "flex", gap: 4 }}>
          <button onClick={() => { setDockKey(k => k + 1); setPanelOrder(defaultOrder); }} style={btnStyle}>Dock all</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
        </div>
      </div>
      <div style={{ display: "flex", gap: 3, marginBottom: 6 }}>{TIER_NAMES.map((name, i) => (<button key={i} onClick={() => setTierIdx(i)} style={{ fontSize: 10, padding: "3px 8px", borderRadius: 4, cursor: "pointer", border: i === tierIdx ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)", background: i === tierIdx ? "var(--color-background-info)" : "var(--color-background-secondary)", color: i === tierIdx ? "var(--color-text-info)" : "var(--color-text-secondary)", fontWeight: i === tierIdx ? 600 : 400 }}>{name}</button>))}</div>
      <div style={{ display: "flex", gap: 6, marginBottom: 6 }}>
        <div ref={c3dRef} style={{ flex: 2, position: "relative", border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "#0a0a0e", resize: "vertical", minHeight: 180, height: 340 }}><canvas ref={cv3dRef} onPointerDown={onPointerDown3D} style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }} /><div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9, color: "rgba(255,255,255,0.3)" }}>drag to rotate · resize ↘</div></div>
        <div ref={c2dRef} style={{ flex: 1, minWidth: 140, border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", resize: "vertical", minHeight: 180, height: 340 }}><canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} /></div>
      </div>
      {panelOrder.map(pid => {
        const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };
        switch (pid) {
        case "geometry": return (<DragPanel {...dp} title="Geometry" ini={true}>
          <div style={ms}><span style={lb}>Height μ</span><Num value={T.height} min={2} max={150} w={50} onChange={v => upd(n => { n.height = v; })} /><span style={sLb}>σ</span><Num value={T.height_s} min={0} max={30} w={38} onChange={v => upd(n => { n.height_s = v; })} /></div>
          <div style={ms}><span style={lb}>Base half μ</span><Num value={T.base_half} min={2} max={100} w={50} onChange={v => upd(n => { n.base_half = v; })} /><span style={sLb}>σ</span><Num value={T.base_half_s} min={0} max={20} w={38} onChange={v => upd(n => { n.base_half_s = v; })} /></div>
          <div style={ms}><span style={lb}>Aspect μ</span><Num value={T.aspect} min={0.3} max={3} w={50} onChange={v => upd(n => { n.aspect = v; })} /><span style={sLb}>σ</span><Num value={T.aspect_s} min={0} max={0.5} w={38} onChange={v => upd(n => { n.aspect_s = v; })} /></div>
          <div style={ms}><span style={lb}>Truncation μ</span><Num value={T.trunc} min={0} max={0.8} w={50} onChange={v => upd(n => { n.trunc = v; })} /><span style={sLb}>σ</span><Num value={T.trunc_s} min={0} max={0.2} w={38} onChange={v => upd(n => { n.trunc_s = v; })} /></div>
          <div style={ms}><span style={lb}>Edge blend μ</span><Num value={T.edge_blend} min={0} max={10} w={50} onChange={v => upd(n => { n.edge_blend = v; })} /><span style={sLb}>σ</span><Num value={T.edge_blend_s} min={0} max={3} w={38} onChange={v => upd(n => { n.edge_blend_s = v; })} /></div>
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Truncation 0 = pointed apex. 0.25 = flat platform at 25% of base.</div>
        </DragPanel>);
        case "appearance": return (<DragPanel {...dp} title="Appearance">
          <div style={{ marginBottom: 4 }}>
            <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginBottom: 2 }}>Preset (click to set color)</div>
            <div style={{ display: "flex", gap: 3, marginBottom: 4 }}>
              <button onClick={() => upd(n => { n.color = PYRAMID_SANDSTONE.c.slice(); })} style={{ ...btnStyle, fontSize: 9, display: "flex", alignItems: "center", gap: 3 }}>
                <span style={{ width: 10, height: 10, borderRadius: 2, background: rgb01(...PYRAMID_SANDSTONE.c), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
                Sandstone
              </button>
            </div>
          </div>
          <div style={rw}><span style={lb}>Color</span>{["R","G","B"].map((ch, i) => <><span key={"l"+i} style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num key={i} value={T.color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.color[i] = v; })} /></>)}<span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(T.color[0], T.color[1], T.color[2]), border: "1px solid var(--color-border-tertiary)" }} /></div>
          <div style={rw}><span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v => upd(n => { n.color_over = v; })} /><span style={lb}>Variance</span><Num value={T.color_var} min={0} max={0.2} w={38} onChange={v => upd(n => { n.color_var = v; })} /><span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v => upd(n => { n.weight = v; })} /></div>
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 3 }}>No palette — pyramids use sandstone only. Engine adds ±{PYRAMID_SANDSTONE.variance} per-channel noise at spawn.</div>
        </DragPanel>);
        case "export": return (<DragPanel {...dp} title="C++ Export">
          <div style={{ display: "flex", gap: 4, marginBottom: 4 }}><button onClick={() => { copyText(cppCode); setCopied(true); setTimeout(() => setCopied(false), 1500); }} style={{ ...btnStyle, background: copied ? "var(--color-background-success)" : "var(--color-background-secondary)" }}>{copied ? "Copied" : "Copy C++"}</button><button onClick={() => setShowCode(!showCode)} style={btnStyle}>{showCode ? "Hide" : "Show"}</button></div>
          {showCode && <textarea readOnly value={cppCode} style={{ width: "100%", height: 50, fontSize: 10, fontFamily: "'JetBrains Mono', monospace", background: "var(--color-background-primary)", color: "var(--color-text-primary)", border: "1px solid var(--color-border-tertiary)", borderRadius: 4, padding: 6, resize: "vertical", lineHeight: 1.4, whiteSpace: "pre" }} />}
        </DragPanel>);
        default: return null;
        }
      })}
    </div>
  );
}
