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

   Preview matches engine pyramid_mesh_gen (world.wgsl §9.0):
   • truncation < 0.01 → pointed: 4 triangular sides + 2-tri bottom cap (6 tris)
   • truncation ≥ 0.01 → truncated: 4 quad sides + top cap + bottom cap (12 tris)

   2D cross-section overlays the terrain heightfield from evaluate_pyramid()
   (world.wgsl ~L1880) so edge_blend's effect on the base is visible.
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

/* ═══ MATH HELPERS ═══ */
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }
function smoothstep(a, b, x) { const t = Math.max(0, Math.min(1, (x - a) / (b - a))); return t * t * (3 - 2 * t); }

/* ═══ TERRAIN HEIGHTFIELD (mirrors world.wgsl evaluate_pyramid) ═══
   At each (lx, lz) in pyramid-local space, returns the terrain Y
   contribution. The visible mesh is hard-edged at hx/hz; the heightfield
   adds a subtle softening at the base in the [hx-blend, hx+blend] band
   via the smoothstep mask. */
function evaluatePyramid(lx, lz, T) {
  const hx = T.base_half;
  const hz = T.base_half * T.aspect;
  const blend = Math.max(T.edge_blend, 0);
  const aLx = Math.abs(lx), aLz = Math.abs(lz);
  if (aLx > hx + blend || aLz > hz + blend) return 0;
  let mask = 1;
  if (blend > 0.001) {
    const fx_lo = smoothstep(-hx - blend, -hx + blend, lx);
    const fx_hi = 1 - smoothstep(hx - blend, hx + blend, lx);
    const fz_lo = smoothstep(-hz - blend, -hz + blend, lz);
    const fz_hi = 1 - smoothstep(hz - blend, hz + blend, lz);
    mask = fx_lo * fx_hi * fz_lo * fz_hi;
  } else {
    if (aLx > hx || aLz > hz) return 0;
  }
  if (mask < 0.001) return 0;
  const cheb = Math.max(aLx / Math.max(hx, 0.001), aLz / Math.max(hz, 0.001));
  const taper = Math.max(0, Math.min(1, (1 - cheb) / Math.max(1 - T.trunc, 0.001)));
  return T.height * taper * mask;
}

/* ═══ 3D RENDERER ═══
   Rebuilds the engine's exact mesh topology:
   • pointed (trunc < 0.01) → 4 tri sides + 2 bottom-cap tris (6 tris, 18 idx)
   • truncated (trunc ≥ 0.01) → 4 quad sides + top + bottom (12 tris, 36 idx)
   Winding matches pyramid_mesh_gen (world.wgsl §9.0). */
function render3D(ctx, W, H, T, rotY, tilt, zoom = 1, panX = 0, panY = 0, view = null) {
  if (!view?.noClear) ctx.clearRect(0, 0, W, H);
  const bx = T.base_half, bz = T.base_half * T.aspect;
  const h = T.height;
  const col = T.color;
  const blend = Math.max(T.edge_blend, 0);
  const isPointed = T.trunc < 0.01;

  // Base corners (y = 0, ground-relative — engine VS adds ground_y)
  const b00 = [-bx, 0, -bz];
  const b10 = [ bx, 0, -bz];
  const b11 = [ bx, 0,  bz];
  const b01 = [-bx, 0,  bz];

  // Build face list (each face: { verts, shadeMul })
  const faces = [];
  if (isPointed) {
    // 4 triangular sides — winding from engine: bases[(f+1)%4], bases[f], apex
    const apex = [0, h, 0];
    faces.push({ verts: [b10, b00, apex], shadeMul: 0.92 }); // -z
    faces.push({ verts: [b11, b10, apex], shadeMul: 0.85 }); // +x
    faces.push({ verts: [b01, b11, apex], shadeMul: 0.92 }); // +z
    faces.push({ verts: [b00, b01, apex], shadeMul: 0.85 }); // -x
    // Bottom cap (CW from above → -Y normal)
    faces.push({ verts: [b00, b11, b01], shadeMul: 0.45 });
    faces.push({ verts: [b00, b10, b11], shadeMul: 0.45 });
  } else {
    const tx = bx * T.trunc, tz = bz * T.trunc;
    const t00 = [-tx, h, -tz];
    const t10 = [ tx, h, -tz];
    const t11 = [ tx, h,  tz];
    const t01 = [-tx, h,  tz];
    // Side quads as triangle pairs (matches engine emit_tri ordering)
    faces.push({ verts: [b10, b00, t00], shadeMul: 0.90 }); faces.push({ verts: [b10, t00, t10], shadeMul: 0.90 });
    faces.push({ verts: [b11, b10, t10], shadeMul: 0.85 }); faces.push({ verts: [b11, t10, t11], shadeMul: 0.85 });
    faces.push({ verts: [b01, b11, t11], shadeMul: 0.90 }); faces.push({ verts: [b01, t11, t01], shadeMul: 0.90 });
    faces.push({ verts: [b00, b01, t01], shadeMul: 0.85 }); faces.push({ verts: [b00, t01, t00], shadeMul: 0.85 });
    // Top cap (CCW from above → +Y normal)
    faces.push({ verts: [t00, t01, t11], shadeMul: 1.00 }); faces.push({ verts: [t00, t11, t10], shadeMul: 1.00 });
    // Bottom cap (CW from above → -Y normal)
    faces.push({ verts: [b00, b11, b01], shadeMul: 0.45 }); faces.push({ verts: [b00, b10, b11], shadeMul: 0.45 });
  }

  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  // Include blend in framing extent so the edge_blend zone outline stays visible
  const extent = Math.max((bx + blend) * 2, (bz + blend) * 2, h);
  const scale = (view?.scale ?? Math.min(W, H) * 0.40 / (extent * 0.5)) * zoom;
  const midY = h / 2;
  const rotVert = (v) => { const vy = v[1] - midY; const x1 = v[0]*cosR + v[2]*sinR; const z1 = -v[0]*sinR + v[2]*cosR; return [x1, vy*cosT - z1*sinT, vy*sinT + z1*cosT]; };
  const project = (v) => [W/2 + panX + v[0]*scale, H/2 + panY - v[1]*scale];

  // Process faces: cull, shade, sort
  const renderFaces = [];
  for (const f of faces) {
    const fv = f.verts.map(rotVert);
    const e1 = sub3(fv[1], fv[0]), e2 = sub3(fv[2], fv[0]);
    const n = normalize3(cross3(e1, e2));
    if (n[2] > 0.02) continue; // backface cull
    const diff = Math.max(0, n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
    const light = 0.22 + 0.78 * diff * f.shadeMul;
    const avgZ = fv.reduce((s, v) => s + v[2], 0) / fv.length;
    renderFaces.push({ verts: fv, light, avgZ });
  }
  renderFaces.sort((a, b) => a.avgZ - b.avgZ);

  // Optional: edge_blend zone outline at y=0 (extended footprint, hx+blend)
  // This is purely informational — the engine's mask attenuates terrain
  // height in the [hx-blend, hx+blend] band (taper=0 outside hx, so visible
  // softening is on the inside of the mesh base).
  if (blend > 0.001) {
    const ringMesh = [
      [-bx, 0, -bz], [ bx, 0, -bz], [ bx, 0,  bz], [-bx, 0,  bz]
    ].map(v => project(rotVert(v)));
    const ringExt = [
      [-bx-blend, 0, -bz-blend], [ bx+blend, 0, -bz-blend],
      [ bx+blend, 0,  bz+blend], [-bx-blend, 0,  bz+blend]
    ].map(v => project(rotVert(v)));
    ctx.strokeStyle = "rgba(255,200,100,0.20)"; ctx.lineWidth = 1; ctx.setLineDash([3, 3]);
    ctx.beginPath();
    ctx.moveTo(ringExt[0][0], ringExt[0][1]);
    for (let i = 1; i < ringExt.length; i++) ctx.lineTo(ringExt[i][0], ringExt[i][1]);
    ctx.closePath(); ctx.stroke();
    ctx.setLineDash([]);
    // Inner edge of softening band (hx - blend) — only if blend < hx
    if (blend < bx && blend < bz) {
      const ringInner = [
        [-bx+blend, 0, -bz+blend], [ bx-blend, 0, -bz+blend],
        [ bx-blend, 0,  bz-blend], [-bx+blend, 0,  bz-blend]
      ].map(v => project(rotVert(v)));
      ctx.strokeStyle = "rgba(255,200,100,0.10)"; ctx.lineWidth = 0.5; ctx.setLineDash([2, 4]);
      ctx.beginPath();
      ctx.moveTo(ringInner[0][0], ringInner[0][1]);
      for (let i = 1; i < ringInner.length; i++) ctx.lineTo(ringInner[i][0], ringInner[i][1]);
      ctx.closePath(); ctx.stroke();
      ctx.setLineDash([]);
    }
  }

  // Draw mesh faces (back to front)
  for (const f of renderFaces) {
    const pts = f.verts.map(project);
    ctx.fillStyle = rgb01(col[0]*f.light, col[1]*f.light, col[2]*f.light);
    ctx.beginPath(); ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath(); ctx.fill();
    ctx.strokeStyle = rgb01(col[0]*f.light*0.7, col[1]*f.light*0.7, col[2]*f.light*0.7);
    ctx.lineWidth = 0.5; ctx.stroke();
  }

  // Topology label
  ctx.fillStyle = "rgba(255,255,255,0.45)"; ctx.font = "9px monospace"; ctx.textAlign = "left";
  ctx.fillText(isPointed ? "mesh: pointed · 6 tris (4 sides + bottom)" : "mesh: truncated · 12 tris (4 sides + top + bottom)", 8, 14);
}

/* ═══ 2D CROSS-SECTION ═══
   Shows mesh outline (solid) overlaid on the terrain heightfield curve
   (orange) sampled along z=0. Where they diverge near the base reveals
   the edge_blend effect — the terrain dips below the mesh in the
   [hx-blend, hx] band where mask < 1.0. */
function render2D(ctx, W, H, T) {
  ctx.clearRect(0, 0, W, H); ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const bx = T.base_half;
  const tx = bx * T.trunc;
  const h = T.height;
  const blend = Math.max(T.edge_blend, 0);
  const isPointed = T.trunc < 0.01;
  const xExtent = bx + blend + 1;
  const mg = 20;
  const sc = Math.min((W - mg*2) / (xExtent * 2.2), (H - mg*2) / (h * 1.15));
  const cx = W / 2, baseY = H - mg;
  const toS = (x, y) => [cx + x * sc, baseY - y * sc];

  // ── Terrain heightfield curve (orange) ─────────────────────────────
  const samples = 240;
  const xMin = -xExtent, xMax = xExtent;
  // Filled region under the curve
  ctx.fillStyle = "rgba(255,180,80,0.10)";
  ctx.beginPath();
  ctx.moveTo(...toS(xMin, 0));
  for (let i = 0; i <= samples; i++) {
    const x = xMin + (xMax - xMin) * i / samples;
    const y = evaluatePyramid(x, 0, T);
    ctx.lineTo(...toS(x, y));
  }
  ctx.lineTo(...toS(xMax, 0));
  ctx.closePath(); ctx.fill();
  // Curve outline
  ctx.strokeStyle = "rgba(255,180,80,0.55)"; ctx.lineWidth = 1;
  ctx.beginPath();
  let first = true;
  for (let i = 0; i <= samples; i++) {
    const x = xMin + (xMax - xMin) * i / samples;
    const y = evaluatePyramid(x, 0, T);
    const [sx, sy] = toS(x, y);
    if (first) { ctx.moveTo(sx, sy); first = false; } else { ctx.lineTo(sx, sy); }
  }
  ctx.stroke();

  // ── Mesh outline (sandstone) ───────────────────────────────────────
  // Pointed: triangle. Truncated: trapezoid.
  const drawMeshPath = () => {
    ctx.beginPath();
    if (isPointed) {
      const [x0, y0] = toS(-bx, 0);   ctx.moveTo(x0, y0);
      const [x1, y1] = toS( bx, 0);   ctx.lineTo(x1, y1);
      const [x2, y2] = toS(  0, h);   ctx.lineTo(x2, y2);
    } else {
      const [x0, y0] = toS(-bx, 0);   ctx.moveTo(x0, y0);
      const [x1, y1] = toS( bx, 0);   ctx.lineTo(x1, y1);
      const [x2, y2] = toS( tx, h);   ctx.lineTo(x2, y2);
      const [x3, y3] = toS(-tx, h);   ctx.lineTo(x3, y3);
    }
    ctx.closePath();
  };
  ctx.fillStyle = "rgba(130,120,100,0.18)"; drawMeshPath(); ctx.fill();
  ctx.strokeStyle = rgb01(T.color[0], T.color[1], T.color[2]); ctx.lineWidth = 1.5; drawMeshPath(); ctx.stroke();

  // ── Ground line ────────────────────────────────────────────────────
  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(...toS(-xExtent*1.05, 0)); ctx.lineTo(...toS(xExtent*1.05, 0)); ctx.stroke();
  ctx.setLineDash([]);

  // ── Edge_blend zone tick marks at -bx-e, -bx, -bx+e, bx-e, bx, bx+e ─
  if (blend > 0.001) {
    ctx.strokeStyle = "rgba(255,180,80,0.30)"; ctx.lineWidth = 1; ctx.setLineDash([2, 2]);
    [-bx - blend, -bx + blend, bx - blend, bx + blend].forEach(x => {
      const [sx, sy] = toS(x, 0);
      ctx.beginPath(); ctx.moveTo(sx, sy - 6); ctx.lineTo(sx, sy + 4); ctx.stroke();
    });
    ctx.setLineDash([]);
  }

  // ── Dimensions label ───────────────────────────────────────────────
  ctx.fillStyle = "rgba(255,255,255,0.4)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  ctx.fillText(`h=${h.toFixed(1)}  base=${bx.toFixed(1)}  trunc=${(T.trunc*100).toFixed(0)}%  blend=${blend.toFixed(1)}`, cx, toS(0, h)[1] - 6);
  ctx.textAlign = "left";
  ctx.fillStyle = "rgba(255,180,80,0.55)"; ctx.font = "8px monospace";
  ctx.fillText("─ terrain heightfield (evaluate_pyramid)", 6, 10);
  ctx.fillStyle = rgb01(T.color[0]*0.9, T.color[1]*0.9, T.color[2]*0.9);
  ctx.fillText(isPointed ? "─ mesh: pointed (trunc < 0.01)" : "─ mesh: truncated", 6, 20);
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
  const [zoom, setZoom] = useState(1.0);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);
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
        <div ref={c3dRef} style={{ flex: 2, position: "relative", border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "#0a0a0e", resize: "vertical", minHeight: 180, height: 340 }}><canvas ref={cv3dRef} onPointerDown={onPointerDown3D} onContextMenu={e => e.preventDefault()} style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }} /><div onClick={() => { setZoom(1); setPanX(0); setPanY(0); }} title="Click to reset view" style={{ position: "absolute", top: 4, right: 8, fontSize: 9, color: "rgba(255,255,255,0.4)", cursor: "pointer", userSelect: "none", padding: "2px 4px" }}>{Math.round(zoom * 100)}%</div><div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9, color: "rgba(255,255,255,0.3)" }}>drag to rotate · right-drag to pan · scroll to zoom · resize ↘</div></div>
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
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Truncation &lt; 0.01 → engine builds 4-sided pointed mesh. ≥ 0.01 → truncated mesh with flat top at trunc·base.</div>
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Edge blend softens the terrain heightfield in the [base−blend, base+blend] band (mesh edges stay hard).</div>
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