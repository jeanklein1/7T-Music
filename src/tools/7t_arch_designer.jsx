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
   7T ARCH DESIGNER
   Catenary barrel vault geometry — 3 tiers (Doorway / Standard / Monumental).
   Profile math ported from world.wgsl arch_mesh_gen + cartridge.hpp solve_catenary_a.
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Doorway", "Standard", "Monumental"];

/* ═══ PALETTES (from cartridge.hpp) ═══ */
const ARCH_PALETTE = [
  { name: "Light grey stone", c: [0.82, 0.80, 0.78] },
];
const ARCH_SANDSTONE = { name: "Sandstone", c: [0.75, 0.68, 0.60], variance: 0.04 };

function defaultTier(idx) {
  const D = [
    // Doorway
    { span: 12.0, span_s: 2.4, rise: 12.0, rise_s: 2.4, depth: 4.5, depth_s: 0.9, thickness: 1.2, thickness_s: 0.18,
      pier_h: 1.5, pier_h_s: 0.9, pier_pad: 0.9, pier_pad_s: 0.3, edge_blend: 0.9, edge_blend_s: 0.15,
      color_over: 0.15, burial: 0.20, weight: 0.50, segs_u: 16, segs_v: 4,
      color: [0.75, 0.68, 0.60] },
    // Standard
    { span: 50.0, span_s: 15.0, rise: 42.0, rise_s: 7.0, depth: 5.6, depth_s: 1.1, thickness: 1.4, thickness_s: 0.21,
      pier_h: 5.6, pier_h_s: 2.1, pier_pad: 0.7, pier_pad_s: 0.3, edge_blend: 0.7, edge_blend_s: 0.14,
      color_over: 0.25, burial: 0.20, weight: 0.35, segs_u: 32, segs_v: 8,
      color: [0.75, 0.68, 0.60] },
    // Monumental
    { span: 60.0, span_s: 10.0, rise: 80.0, rise_s: 12.0, depth: 10.0, depth_s: 2.0, thickness: 2.5, thickness_s: 0.30,
      pier_h: 8.0, pier_h_s: 2.5, pier_pad: 1.0, pier_pad_s: 0.3, edge_blend: 0.8, edge_blend_s: 0.15,
      color_over: 0.35, burial: 0.20, weight: 0.15, segs_u: 48, segs_v: 12,
      color: [0.75, 0.68, 0.60] },
  ];
  return JSON.parse(JSON.stringify(D[idx]));
}

/* ═══ CATENARY MATH ═══ */
function solveCatenaryA(halfSpan, targetH) {
  let aLo = 0.1, aHi = Math.max(halfSpan * 10, 5);
  let a = halfSpan;
  for (let i = 0; i < 50; i++) {
    a = 0.5 * (aLo + aHi);
    const val = a * (Math.cosh(halfSpan / a) - 1);
    if (val > targetH) aLo = a; else aHi = a;
  }
  return a;
}

function catenaryProfile(halfSpan, rise, thickness, pierH, burial, segsU) {
  const a = solveCatenaryA(halfSpan, rise);
  const H = a * (Math.cosh(halfSpan / a) - 1);
  const pts = { outer: [], inner: [] };
  for (let i = 0; i <= segsU; i++) {
    const u = i / segsU;
    const t = -halfSpan + 2 * halfSpan * u;
    const y = H - a * (Math.cosh(t / a) - 1);
    const sh = Math.sinh(t / a);
    const plen = Math.sqrt(sh * sh + 1);
    const pnx = sh / plen, pny = 1 / plen;
    const ht = thickness * 0.5;
    pts.outer.push({ x: t + pnx * ht, y: pierH + y + pny * ht - burial });
    pts.inner.push({ x: t - pnx * ht, y: pierH + y - pny * ht - burial });
  }
  return { pts, a, H };
}

/* ═══ 3D RENDERER — barrel vault with catenary cross-section ═══ */
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }

function render3D(ctx, W, H, T, rotY, tilt, zoom = 1, panX = 0, panY = 0, view = null) {
  if (!view?.noClear) ctx.clearRect(0, 0, W, H);
  const halfSpan = T.span / 2;
  const segsU = Math.min(T.segs_u, 48);
  const segsV = Math.min(T.segs_v, 12);
  const { pts, H: catH } = catenaryProfile(halfSpan, T.rise, T.thickness, T.pier_h, T.burial * T.pier_h, segsU);

  const faces = [];
  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  const totalH = T.pier_h + catH + T.thickness;
  const extent = Math.max(T.span, T.depth, totalH);
  const midY = totalH / 2;
  const scale = (view?.scale ?? Math.min(W, H) * 0.42 / (extent * 0.5)) * zoom;

  // Subtract midY BEFORE rotation so the arch spins around its center
  const rotVert = (v) => { const vy=v[1]-midY; const x1=v[0]*cosR+v[2]*sinR; const z1=-v[0]*sinR+v[2]*cosR; return [x1, vy*cosT-z1*sinT, vy*sinT+z1*cosT]; };
  const project = (v) => [W/2 + panX + v[0]*scale, H/2 + panY - v[1]*scale];
  const halfD = T.depth / 2;
  const col = T.color;

  // Build shell quads (outer and inner)
  for (let shell = 0; shell < 2; shell++) {
    const p = shell === 0 ? pts.outer : pts.inner;
    for (let iv = 0; iv < segsV; iv++) {
      for (let iu = 0; iu < segsU; iu++) {
        const v0 = iv / segsV, v1 = (iv+1) / segsV;
        const z0 = -halfD + T.depth * v0, z1 = -halfD + T.depth * v1;
        const p0 = p[iu], p1 = p[iu+1];
        // Outer shell: [p0z0, p1z0, p1z1, p0z1] → cross gives inward normal ✓
        // Inner shell: reverse winding so cross also gives inward normal
        const verts = shell === 0 ? [
          rotVert([p0.x, p0.y, z0]), rotVert([p1.x, p1.y, z0]),
          rotVert([p1.x, p1.y, z1]), rotVert([p0.x, p0.y, z1]),
        ] : [
          rotVert([p0.x, p0.y, z0]), rotVert([p0.x, p0.y, z1]),
          rotVert([p1.x, p1.y, z1]), rotVert([p1.x, p1.y, z0]),
        ];
        const e1 = sub3(verts[1], verts[0]), e2 = sub3(verts[3], verts[0]);
        const n = normalize3(cross3(e1, e2));
        if (n[2] > 0.02) continue;
        const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
        const light = (shell === 0 ? 0.25 : 0.18) + (shell === 0 ? 0.75 : 0.55) * diff;
        faces.push({ verts, light, col, avgZ: (verts[0][2]+verts[1][2]+verts[2][2]+verts[3][2])/4 });
      }
    }
  }

  // Front and back cap faces — quad strip connecting outer to inner at each depth end
  // Mirrors WGSL amg_gen_cap (sub-meshes 2 and 3)
  for (let capSide = 0; capSide < 2; capSide++) {
    const zPos = capSide === 0 ? -halfD : halfD;
    for (let iu = 0; iu < segsU; iu++) {
      const o0 = pts.outer[iu], o1 = pts.outer[iu+1];
      const i0 = pts.inner[iu], i1 = pts.inner[iu+1];
      // Front cap: [o0, i0, i1, o1] → cross gives +z (inward) ✓
      // Back cap:  [o0, o1, i1, i0] → cross gives -z (inward) ✓
      const verts = capSide === 0 ? [
        rotVert([o0.x, o0.y, zPos]), rotVert([i0.x, i0.y, zPos]),
        rotVert([i1.x, i1.y, zPos]), rotVert([o1.x, o1.y, zPos]),
      ] : [
        rotVert([o0.x, o0.y, zPos]), rotVert([o1.x, o1.y, zPos]),
        rotVert([i1.x, i1.y, zPos]), rotVert([i0.x, i0.y, zPos]),
      ];
      const e1 = sub3(verts[1], verts[0]), e2 = sub3(verts[3], verts[0]);
      const n = normalize3(cross3(e1, e2));
      if (n[2] > 0.02) continue;
      const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
      const light = 0.22 + 0.68 * diff;
      faces.push({ verts, light, col, avgZ: (verts[0][2]+verts[1][2]+verts[2][2]+verts[3][2])/4 });
    }
  }

  // Pier blocks (two rectangular solids at the base)
  const pierW = T.thickness + T.pier_pad * 2;
  for (let side = 0; side < 2; side++) {
    const cx = side === 0 ? -halfSpan : halfSpan;
    const hw = pierW / 2, hd = halfD;
    const y0 = -T.burial * T.pier_h, y1 = T.pier_h - T.burial * T.pier_h;
    const corners = [[-hw,y0,-hd],[hw,y0,-hd],[hw,y0,hd],[-hw,y0,hd],[-hw,y1,-hd],[hw,y1,-hd],[hw,y1,hd],[-hw,y1,hd]];
    const faceIdx = [[0,3,2,1],[4,5,6,7],[0,1,5,4],[2,3,7,6],[0,4,7,3],[1,2,6,5]];
    for (let fi = 0; fi < 6; fi++) {
      const verts = faceIdx[fi].map(ci => { const c = corners[ci]; return rotVert([c[0]+cx, c[1], c[2]]); });
      // Compute normal from cross product (inward, consistent with shell faces)
      const e1 = sub3(verts[1], verts[0]), e2 = sub3(verts[3], verts[0]);
      const n = normalize3(cross3(e1, e2));
      if (n[2] > 0.02) continue;
      const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
      faces.push({ verts, light: 0.22 + 0.65 * diff, col: [col[0]*0.85, col[1]*0.85, col[2]*0.85], avgZ: (verts[0][2]+verts[2][2])/2 });
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
function render2D(ctx, W, H, T) {
  ctx.clearRect(0, 0, W, H); ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const halfSpan = T.span / 2;
  const bur = T.burial * T.pier_h;
  const { pts, H: catH } = catenaryProfile(halfSpan, T.rise, T.thickness, T.pier_h, bur, 60);
  const totalH = T.pier_h + catH + T.thickness;
  const extent = Math.max(T.span + T.thickness * 2 + T.pier_pad * 4, totalH);
  const mg = 20;
  const sc = Math.min((W - mg*2) / extent, (H - mg*2) / totalH);
  const cx = W / 2, baseY = H - mg;
  const toS = (x, y) => [cx + x * sc, baseY - (y + bur) * sc];

  // Fill between outer curves
  ctx.fillStyle = "rgba(130,120,100,0.2)";
  ctx.beginPath();
  const o0 = toS(pts.outer[0].x, pts.outer[0].y);
  ctx.moveTo(o0[0], o0[1]);
  for (const p of pts.outer) { const [sx, sy] = toS(p.x, p.y); ctx.lineTo(sx, sy); }
  for (let i = pts.inner.length - 1; i >= 0; i--) { const [sx, sy] = toS(pts.inner[i].x, pts.inner[i].y); ctx.lineTo(sx, sy); }
  ctx.closePath(); ctx.fill();

  // Outlines
  ctx.lineWidth = 1.5; ctx.strokeStyle = rgb01(T.color[0], T.color[1], T.color[2]);
  for (const curve of [pts.outer, pts.inner]) {
    ctx.beginPath();
    const [fx, fy] = toS(curve[0].x, curve[0].y);
    ctx.moveTo(fx, fy);
    for (let i = 1; i < curve.length; i++) { const [sx, sy] = toS(curve[i].x, curve[i].y); ctx.lineTo(sx, sy); }
    ctx.stroke();
  }

  // Pier blocks
  ctx.fillStyle = "rgba(130,120,100,0.12)"; ctx.strokeStyle = "rgba(200,154,108,0.4)"; ctx.lineWidth = 1;
  for (let side = -1; side <= 1; side += 2) {
    const pierX = side * halfSpan;
    const hw = (T.thickness + T.pier_pad * 2) / 2;
    const [x0, y0] = toS(pierX - hw, -bur);
    const [x1, y1] = toS(pierX + hw, T.pier_h - bur);
    ctx.fillRect(x0, y1, x1 - x0, y0 - y1);
    ctx.strokeRect(x0, y1, x1 - x0, y0 - y1);
  }

  // Ground line
  const [gx0, gy] = toS(-extent / 2, 0);
  const [gx1] = toS(extent / 2, 0);
  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(gx0, gy); ctx.lineTo(gx1, gy); ctx.stroke(); ctx.setLineDash([]);

  // Dimensions
  ctx.fillStyle = "rgba(255,255,255,0.3)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  const [, topY] = toS(0, T.pier_h + catH + T.thickness / 2);
  ctx.fillText(`span=${T.span.toFixed(1)}  rise=${T.rise.toFixed(1)}`, cx, topY - 6);
  ctx.fillText(`depth=${T.depth.toFixed(1)}  thick=${T.thickness.toFixed(2)}`, cx, topY + 6);
}

/* ═══ UI COMPONENTS ═══ */
const ist = { padding: "2px 3px", fontSize: 11, fontFamily: "monospace", borderRadius: 4, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)", textAlign: "right" };
function Num({ value, onChange, min = 0, max = 10, step = 0.01, w = 46 }) {
  const [txt, setTxt] = useState(String(Math.round(value * 10000) / 10000));
  const [focused, setFocused] = useState(false);
  useEffect(() => { setTxt(String(Math.round(value * 10000) / 10000)); }, [value]);
  const commit = () => { const v = parseFloat(txt); if (!isNaN(v)) onChange(Math.max(min, Math.min(max, v))); else setTxt(String(Math.round(value * 10000) / 10000)); setFocused(false); };
  return (<span style={{ position: "relative", display: "inline-block" }}><input type="text" value={txt} title={`${min} – ${max}`} onChange={e => setTxt(e.target.value)} onFocus={() => setFocused(true)} onBlur={commit} onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }} style={{ ...ist, width: w, outline: focused ? "1.5px solid var(--color-border-info)" : "none" }} /></span>);
}

function DragPanel({ title, children, ini = false, id, resetKey, onDock }) {
  const [open, setOpen] = useState(ini);
  const [pos, setPos] = useState(null);
  const [dragging, setDragging] = useState(false);
  const dragRef = useRef(null);
  const offsetRef = useRef({ x: 0, y: 0 });
  useEffect(() => { setPos(null); }, [resetKey]);
  const startDrag = useCallback(e => { if (e.target.closest("[data-notdrag]")) return; e.preventDefault(); const rect = dragRef.current.getBoundingClientRect(); offsetRef.current = { x: e.clientX - rect.left, y: e.clientY - rect.top }; setDragging(true); }, []);
  useEffect(() => { if (!dragging) return; const onMove = e => { const p = dragRef.current.parentElement.getBoundingClientRect(); setPos({ x: e.clientX - p.left - offsetRef.current.x, y: e.clientY - p.top - offsetRef.current.y }); }; const onUp = () => setDragging(false); window.addEventListener("mousemove", onMove); window.addEventListener("mouseup", onUp); return () => { window.removeEventListener("mousemove", onMove); window.removeEventListener("mouseup", onUp); }; }, [dragging]);
  const doDock = () => { setPos(null); if (onDock) onDock(id); };
  const style = pos ? { position: "absolute", left: pos.x, top: pos.y, zIndex: 100, maxWidth: 520, minWidth: 280 } : {};
  return (<div ref={dragRef} style={{ marginBottom: pos ? 0 : 5, border: pos ? "1px solid #666" : "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: pos ? "#2a2a30" : "var(--color-background-primary)", boxShadow: pos ? "0 8px 32px rgba(0,0,0,.6)" : "none", ...style }}><div onMouseDown={startDrag} style={{ padding: "5px 10px", fontSize: 12, fontWeight: 500, userSelect: "none", display: "flex", alignItems: "center", gap: 5, color: pos ? "#e0ddd8" : "var(--color-text-primary)", background: pos ? "#353540" : "var(--color-background-secondary)", cursor: "grab" }}><span data-notdrag="1" onClick={() => setOpen(!open)} style={{ cursor: "pointer", fontSize: 9, transition: "transform .15s", display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px" }}>▶</span><span style={{ flex: 1 }}>{title}</span>{pos && <span data-notdrag="1" onClick={doDock} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}</div>{open && <div style={{ padding: "5px 10px", maxHeight: pos ? 400 : "none", overflowY: pos ? "auto" : "visible", color: pos ? "#d0cdc8" : undefined }}>{children}</div>}</div>);
}

const rw = { display: "flex", flexWrap: "wrap", gap: "3px 8px", alignItems: "center", marginBottom: 3 };
const ms = { display: "grid", gridTemplateColumns: "90px auto 16px auto", gap: "2px 4px", alignItems: "center", marginBottom: 3 };
const lb = { fontSize: 10, color: "var(--color-text-secondary)", minWidth: 56 };
const sLb = { fontSize: 10, color: "var(--color-text-tertiary)", textAlign: "center" };
const btnStyle = { fontSize: 10, padding: "2px 8px", borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-secondary)" };

/* ═══ C++ EXPORT ═══ */
function genCpp(tiers) {
  const r = (v, d=2) => v.toFixed(d) + "f";
  const lines = ["static constexpr ArchTierParams ARCH_TIERS[] = {"];
  tiers.forEach((T, i) => {
    lines.push(`    /* ${TIER_NAMES[i].toUpperCase().padEnd(12)} */  { ${r(T.span)}, ${r(T.span_s)},  ${r(T.rise)}, ${r(T.rise_s)},  ${r(T.depth)}, ${r(T.depth_s)},  ${r(T.thickness)}, ${r(T.thickness_s)},  ${r(T.pier_h)}, ${r(T.pier_h_s)},  ${r(T.pier_pad)}, ${r(T.pier_pad_s)},  ${r(T.edge_blend)}, ${r(T.edge_blend_s)},  ${r(T.color_over)}, ${r(T.burial)},  ${T.segs_u}, ${T.segs_v},  ${r(T.weight)} },`);
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

const STORAGE_KEY = "7t:arch:tiers";
const DEFAULTS = () => TIER_NAMES.map((_, i) => defaultTier(i));
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

/* ═══ MAIN ═══ */
export default function ArchDesigner() {
  const [tierIdx, setTierIdx] = useState(1);
  const [tiers, setTiers] = useState(DEFAULTS);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const [rotY, setRotY] = useState(0.6);
  const [tilt, setTilt] = useState(0.15);
  const [zoom, setZoom] = useState(1.0);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);
  const [dockKey, setDockKey] = useState(0);
  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);
  const defaultOrder = ["shell", "piers", "appearance", "export"];
  const [panelOrder, setPanelOrder] = useState(defaultOrder);
  const [resizeTick, setResizeTick] = useState(0);

  const cv3dRef = useRef(null), cv2dRef = useRef(null);
  const c3dRef = useRef(null), c2dRef = useRef(null);

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

  useEffect(() => { const ro = new ResizeObserver(() => setResizeTick(t => t + 1)); if (c3dRef.current) ro.observe(c3dRef.current); if (c2dRef.current) ro.observe(c2dRef.current); return () => ro.disconnect(); }, []);

  useEffect(() => { const cv = cv3dRef.current; if (!cv) return; const dpr = window.devicePixelRatio || 1; const W = cv.clientWidth, H = cv.clientHeight; cv.width = W * dpr; cv.height = H * dpr; const ctx = cv.getContext("2d"); ctx.scale(dpr, dpr); render3D(ctx, W, H, T, rotY, tilt, zoom, panX, panY); }, [T, rotY, tilt, resizeTick, zoom, panX, panY]);
  useEffect(() => { const cv = cv2dRef.current; if (!cv) return; const dpr = window.devicePixelRatio || 1; const W = cv.clientWidth, H = cv.clientHeight; cv.width = W * dpr; cv.height = H * dpr; const ctx = cv.getContext("2d"); ctx.scale(dpr, dpr); render2D(ctx, W, H, T); }, [T, resizeTick]);

  const onPointerDown3D = useCallback(e => { const sx = e.clientX, sy = e.clientY; if (e.button === 2) { e.preventDefault(); const spx = panX, spy = panY; const onMoveP = ev => { setPanX(spx + (ev.clientX - sx)); setPanY(spy + (ev.clientY - sy)); }; const onUpP = () => { window.removeEventListener("pointermove", onMoveP); window.removeEventListener("pointerup", onUpP); }; window.addEventListener("pointermove", onMoveP); window.addEventListener("pointerup", onUpP); return; } const sr = rotY, st = tilt; const onMove = ev => { setRotY(sr + (ev.clientX - sx) * 0.01); setTilt(st - (ev.clientY - sy) * 0.008); }; const onUp = () => { window.removeEventListener("pointermove", onMove); window.removeEventListener("pointerup", onUp); }; window.addEventListener("pointermove", onMove); window.addEventListener("pointerup", onUp); }, [rotY, tilt, panX, panY]);
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
  const cppCode = genCpp(tiers);

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{ fontFamily: "'JetBrains Mono', 'SF Mono', monospace", color: "var(--color-text-primary)", lineHeight: 1.4, position: "relative", fontSize: 11 }}>
      {/* TOP BAR */}
      <div style={{ ...rw, marginBottom: 6, padding: "2px 0" }}>
        <span style={{ fontSize: 12, fontWeight: 600, letterSpacing: "0.02em" }}>7T Arch Designer</span>
        {isModified && <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 6 }}>● modified</span>}
        <div style={{ marginLeft: "auto", display: "flex", gap: 4 }}>
          <button onClick={() => { setDockKey(k => k + 1); setPanelOrder(defaultOrder); }} style={btnStyle}>Dock all</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
        </div>
      </div>

      {/* TIER SELECTOR */}
      <div style={{ display: "flex", gap: 3, marginBottom: 6 }}>
        {TIER_NAMES.map((name, i) => (
          <button key={i} onClick={() => setTierIdx(i)} style={{ fontSize: 10, padding: "3px 8px", borderRadius: 4, cursor: "pointer", border: i === tierIdx ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)", background: i === tierIdx ? "var(--color-background-info)" : "var(--color-background-secondary)", color: i === tierIdx ? "var(--color-text-info)" : "var(--color-text-secondary)", fontWeight: i === tierIdx ? 600 : 400 }}>{name}</button>
        ))}
      </div>

      {/* CANVASES */}
      <div style={{ display: "flex", gap: 6, marginBottom: 6 }}>
        <div ref={c3dRef} style={{ flex: 2, position: "relative", border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "#0a0a0e", resize: "vertical", minHeight: 180, height: 380 }}>
          <canvas ref={cv3dRef} onPointerDown={onPointerDown3D} onContextMenu={e => e.preventDefault()} style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }} />
          <div
            onClick={() => { setZoom(1); setPanX(0); setPanY(0); }}
            title="Click to reset view"
            style={{ position: "absolute", top: 4, right: 8, fontSize: 9,
                     color: "rgba(255,255,255,0.4)", cursor: "pointer",
                     userSelect: "none", padding: "2px 4px" }}
          >{Math.round(zoom * 100)}%</div>
          <div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9,
                        color: "rgba(255,255,255,0.3)" }}>drag to rotate · right-drag to pan · scroll to zoom</div>
          <div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9, color: "rgba(255,255,255,0.3)" }}>drag to rotate · resize ↘</div>
        </div>
        <div ref={c2dRef} style={{ flex: 1, minWidth: 160, border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", resize: "vertical", minHeight: 180, height: 380 }}>
          <canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} />
        </div>
      </div>

      {/* PANELS */}
      {panelOrder.map(pid => {
        const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };
        switch (pid) {
        case "shell": return (<DragPanel {...dp} title="Shell Geometry" ini={true}>
          <div style={ms}><span style={lb}>Span μ</span><Num value={T.span} min={2} max={120} w={50} onChange={v => upd(n => { n.span = v; })} /><span style={sLb}>σ</span><Num value={T.span_s} min={0} max={30} w={38} onChange={v => upd(n => { n.span_s = v; })} /></div>
          <div style={ms}><span style={lb}>Rise μ</span><Num value={T.rise} min={2} max={120} w={50} onChange={v => upd(n => { n.rise = v; })} /><span style={sLb}>σ</span><Num value={T.rise_s} min={0} max={20} w={38} onChange={v => upd(n => { n.rise_s = v; })} /></div>
          <div style={ms}><span style={lb}>Depth μ</span><Num value={T.depth} min={0.5} max={30} w={50} onChange={v => upd(n => { n.depth = v; })} /><span style={sLb}>σ</span><Num value={T.depth_s} min={0} max={5} w={38} onChange={v => upd(n => { n.depth_s = v; })} /></div>
          <div style={ms}><span style={lb}>Thickness μ</span><Num value={T.thickness} min={0.1} max={6} w={50} onChange={v => upd(n => { n.thickness = v; })} /><span style={sLb}>σ</span><Num value={T.thickness_s} min={0} max={1} w={38} onChange={v => upd(n => { n.thickness_s = v; })} /></div>
          <div style={rw}><span style={lb}>Segs U</span><Num value={T.segs_u} min={4} max={64} step={1} w={34} onChange={v => upd(n => { n.segs_u = Math.round(v); })} /><span style={lb}>Segs V</span><Num value={T.segs_v} min={2} max={16} step={1} w={34} onChange={v => upd(n => { n.segs_v = Math.round(v); })} /></div>
        </DragPanel>);
        case "piers": return (<DragPanel {...dp} title="Pier Solids">
          <div style={ms}><span style={lb}>Pier height μ</span><Num value={T.pier_h} min={0} max={30} w={50} onChange={v => upd(n => { n.pier_h = v; })} /><span style={sLb}>σ</span><Num value={T.pier_h_s} min={0} max={8} w={38} onChange={v => upd(n => { n.pier_h_s = v; })} /></div>
          <div style={ms}><span style={lb}>Padding μ</span><Num value={T.pier_pad} min={0} max={3} w={50} onChange={v => upd(n => { n.pier_pad = v; })} /><span style={sLb}>σ</span><Num value={T.pier_pad_s} min={0} max={1} w={38} onChange={v => upd(n => { n.pier_pad_s = v; })} /></div>
          <div style={ms}><span style={lb}>Edge blend μ</span><Num value={T.edge_blend} min={0} max={3} w={50} onChange={v => upd(n => { n.edge_blend = v; })} /><span style={sLb}>σ</span><Num value={T.edge_blend_s} min={0} max={0.5} w={38} onChange={v => upd(n => { n.edge_blend_s = v; })} /></div>
        </DragPanel>);
        case "appearance": return (<DragPanel {...dp} title="Appearance">
          <div style={{ marginBottom: 4 }}>
            <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginBottom: 2 }}>Presets (click to set color)</div>
            <div style={{ display: "flex", gap: 3, flexWrap: "wrap", marginBottom: 4 }}>
              <button onClick={() => upd(n => { n.color = ARCH_SANDSTONE.c.slice(); })} style={{ ...btnStyle, fontSize: 9, display: "flex", alignItems: "center", gap: 3 }}>
                <span style={{ width: 10, height: 10, borderRadius: 2, background: rgb01(...ARCH_SANDSTONE.c), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
                Sandstone
              </button>
              {ARCH_PALETTE.map((p, i) => (
                <button key={i} onClick={() => upd(n => { n.color = p.c.slice(); })} style={{ ...btnStyle, fontSize: 9, display: "flex", alignItems: "center", gap: 3 }}>
                  <span style={{ width: 10, height: 10, borderRadius: 2, background: rgb01(...p.c), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
                  {p.name}
                </button>
              ))}
            </div>
          </div>
          <div style={rw}>
            <span style={lb}>Color</span>
            {["R","G","B"].map((ch, i) => <><span key={"l"+i} style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num key={i} value={T.color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.color[i] = v; })} /></>)}
            <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(T.color[0], T.color[1], T.color[2]), border: "1px solid var(--color-border-tertiary)" }} />
          </div>
          <div style={rw}><span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v => upd(n => { n.color_over = v; })} /><span style={lb}>Burial</span><Num value={T.burial} min={0} max={0.5} w={38} onChange={v => upd(n => { n.burial = v; })} /><span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v => upd(n => { n.weight = v; })} /></div>
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 3 }}>Override % = probability of using palette color vs sandstone. Engine adds ±{ARCH_SANDSTONE.variance} per-channel noise at spawn.</div>
        </DragPanel>);
        case "export": return (<DragPanel {...dp} title="C++ Export">
          <div style={{ display: "flex", gap: 4, marginBottom: 4 }}><button onClick={() => { copyText(cppCode); setCopied(true); setTimeout(() => setCopied(false), 1500); }} style={{ ...btnStyle, background: copied ? "var(--color-background-success)" : "var(--color-background-secondary)" }}>{copied ? "Copied" : "Copy C++"}</button><button onClick={() => setShowCode(!showCode)} style={btnStyle}>{showCode ? "Hide" : "Show"}</button></div>
          {showCode && <textarea readOnly value={cppCode} style={{ width: "100%", height: 60, fontSize: 10, fontFamily: "'JetBrains Mono', monospace", background: "var(--color-background-primary)", color: "var(--color-text-primary)", border: "1px solid var(--color-border-tertiary)", borderRadius: 4, padding: 6, resize: "vertical", lineHeight: 1.4, whiteSpace: "pre" }} />}
        </DragPanel>);
        default: return null;
        }
      })}
    </div>
  );
}
