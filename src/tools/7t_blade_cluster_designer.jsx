import { useState, useEffect, useRef, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T BLADE CLUSTER DESIGNER
   Ground-level leaf clusters — 3-7 thick pointed blades from a single
   ground point. No stem/trunk. Sparse vertical green accents.
   3 tiers: Sprout / Clump / Thicket.
   Visual reference: the small green plants in Jean's 7T paintings.
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Sprout", "Clump", "Thicket"];

/* ═══ HASH — exact port from cartridge.hpp ═══ */
function cpuH(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return ((h >>> 16) ^ h) >>> 0;
}
function hf(s, p) { return cpuH(s, p) / 0xFFFFFFFF; }

/* ═══ PALETTES ═══ */
const BLADE_PALETTE = [
  { name: "Lush green",    c: [0.28, 0.52, 0.22] },
  { name: "Dark green",    c: [0.18, 0.38, 0.18] },
  { name: "Olive",         c: [0.38, 0.45, 0.22] },
  { name: "Yellow-green",  c: [0.45, 0.52, 0.22] },
  { name: "Sage",          c: [0.35, 0.45, 0.30] },
  { name: "Forest",        c: [0.15, 0.32, 0.15] },
];
const BLADE_AGED_PALETTE = [
  { name: "Dry olive",     c: [0.48, 0.45, 0.28] },
  { name: "Straw",         c: [0.55, 0.50, 0.30] },
  { name: "Brown tip",     c: [0.45, 0.38, 0.25] },
  { name: "Warm aged",     c: [0.50, 0.42, 0.28] },
];

/* ═══ PER-INSTANCE COLOR ═══ */
function resolveColors(T, seed) {
  const v = T.color_var || 0;
  let bc = [...T.blade_color];
  let ac = [...(T.blade_color_aged || T.blade_color)];
  if (hf(seed, 950) < T.color_over) {
    bc = [...BLADE_PALETTE[cpuH(seed, 955) % BLADE_PALETTE.length].c];
  }
  bc[0] += (hf(seed, 951) - 0.5) * v * 2;
  bc[1] += (hf(seed, 952) - 0.5) * v * 2;
  bc[2] += (hf(seed, 953) - 0.5) * v * 2;
  if (hf(seed, 960) < T.color_over) {
    ac = [...BLADE_AGED_PALETTE[cpuH(seed, 965) % BLADE_AGED_PALETTE.length].c];
  }
  ac[0] += (hf(seed, 961) - 0.5) * v * 2;
  ac[1] += (hf(seed, 962) - 0.5) * v * 2;
  ac[2] += (hf(seed, 963) - 0.5) * v * 2;
  return {
    bladeCol: bc.map(c => Math.max(0, Math.min(1, c))),
    agedCol: ac.map(c => Math.max(0, Math.min(1, c))),
  };
}

/* ═══ TIER DEFAULTS ═══ */
function defaultTier(idx) {
  const D = [
    // Sprout — tiny, 3-4 blades, ground punctuation
    { blade_count: 3, blade_count_s: 0.5,
      blade_h: 1.8, blade_h_s: 0.4,
      blade_h_var: 0.35, blade_h_var_s: 0.08,
      blade_w: 0.30, blade_w_s: 0.06,
      splay: 0.18, splay_s: 0.06,
      curve: 0.12, curve_s: 0.04,
      twist: 0.05, twist_s: 0.02,
      taper: 0.85, taper_s: 0.05,
      color_over: 0.15, color_var: 0.06,
      blade_segs: 5,
      weight: 0.50,
      blade_color: [0.28, 0.52, 0.22],
      blade_color_aged: [0.48, 0.45, 0.28] },
    // Clump — classic painting bush, 4-6 blades
    { blade_count: 5, blade_count_s: 1,
      blade_h: 3.2, blade_h_s: 0.6,
      blade_h_var: 0.40, blade_h_var_s: 0.10,
      blade_w: 0.45, blade_w_s: 0.08,
      splay: 0.25, splay_s: 0.08,
      curve: 0.18, curve_s: 0.06,
      twist: 0.08, twist_s: 0.03,
      taper: 0.82, taper_s: 0.05,
      color_over: 0.20, color_var: 0.06,
      blade_segs: 6,
      weight: 0.35,
      blade_color: [0.25, 0.48, 0.20],
      blade_color_aged: [0.50, 0.45, 0.28] },
    // Thicket — taller, denser, 5-7 blades
    { blade_count: 6, blade_count_s: 1,
      blade_h: 5.5, blade_h_s: 1.2,
      blade_h_var: 0.45, blade_h_var_s: 0.10,
      blade_w: 0.55, blade_w_s: 0.10,
      splay: 0.30, splay_s: 0.10,
      curve: 0.22, curve_s: 0.08,
      twist: 0.10, twist_s: 0.04,
      taper: 0.80, taper_s: 0.06,
      color_over: 0.25, color_var: 0.08,
      blade_segs: 7,
      weight: 0.15,
      blade_color: [0.18, 0.42, 0.18],
      blade_color_aged: [0.45, 0.40, 0.25] },
  ];
  return JSON.parse(JSON.stringify(D[idx]));
}

/* ═══ 3D MATH ═══ */
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }
function lerpC(a, b, t) { return [a[0]+(b[0]-a[0])*t, a[1]+(b[1]-a[1])*t, a[2]+(b[2]-a[2])*t]; }
function lerp(a, b, t) { return a + (b - a) * t; }

/* ═══ BLADE GEOMETRY ═══
   Each blade: flat quad strip along a curved midrib.
   Wide at base, tapers to sharp point. Gentle curve outward.
   Per-blade height variation from seed gives asymmetric silhouette.
   Golden angle azimuth packing. */
function bladeGeometry(T, bladeIdx, totalBlades, seed) {
  const quads = [];
  const segs = Math.max(3, T.blade_segs);
  const GA = Math.PI * (3 - Math.sqrt(5));

  // Azimuth: golden angle packing
  const azimuth = bladeIdx * GA;
  const ca = Math.cos(azimuth), sa = Math.sin(azimuth);

  // Per-blade height variation (hash-driven for determinism)
  const hVar = T.blade_h_var || 0;
  const hMult = 1.0 + (hf(seed, 970 + bladeIdx) - 0.5) * hVar * 2;
  const bladeH = T.blade_h * Math.max(0.4, hMult);

  // Splay: how far the blade tilts outward from vertical
  // Outer blades (lower index) splay more
  const rank = bladeIdx / Math.max(1, totalBlades - 1);
  const splayAng = T.splay * (0.6 + 0.4 * (1.0 - rank));
  // Per-blade splay jitter
  const splayJ = (hf(seed, 980 + bladeIdx) - 0.5) * 0.15;
  const finalSplay = splayAng + splayJ;

  // Forward direction: tilted outward from vertical
  const cosS = Math.cos(finalSplay), sinS = Math.sin(finalSplay);
  const fwd = [ca * sinS, cosS, sa * sinS];

  // Perpendicular for blade width (cross fwd × up-ish)
  let right;
  if (cosS > 0.95) {
    right = normalize3([-sa, 0, ca]);
  } else {
    right = normalize3(cross3(fwd, [0, 1, 0]));
  }

  // Per-blade twist (rotation of width axis along length)
  const twistDir = (bladeIdx % 2 === 0) ? 1 : -1;
  const twistAmt = T.twist * twistDir;

  // Color: blend young→aged by rank
  const aged = T.blade_color_aged || T.blade_color;
  const baseCol = lerpC(T.blade_color, aged, (1.0 - rank) * 0.5);

  const midrib = [];
  for (let i = 0; i <= segs; i++) {
    const t = i / segs;
    const dist = t * bladeH;

    // Curve: gentle outward arc (quadratic)
    const curveOff = T.curve * bladeH * t * t;

    // Position along fwd direction + curve displacement
    const x = fwd[0] * dist + ca * curveOff;
    const y = fwd[1] * dist;
    const z = fwd[2] * dist + sa * curveOff;

    // Width: wide at base, taper to sharp point
    const baseFrac = 0.3 + 0.7 * Math.min(1, t * 4); // ramp in at root
    const tipFrac = 1.0 - Math.pow(t, T.taper * 2.5 + 0.5); // taper to point
    const w = T.blade_w * baseFrac * tipFrac;

    // Twist the width axis along the blade
    const twistAngle = twistAmt * t * Math.PI;
    const ct = Math.cos(twistAngle), st2 = Math.sin(twistAngle);
    const rx = right[0] * ct + fwd[0] * st2;
    const ry = right[1] * ct + fwd[1] * st2;
    const rz = right[2] * ct + fwd[2] * st2;

    const perpX = rx * w * 0.5;
    const perpY = ry * w * 0.5;
    const perpZ = rz * w * 0.5;

    // Color: darker at base, lighter mid, tip trends toward aged
    const shade = 0.7 + 0.3 * Math.sin(t * Math.PI * 0.8);
    const tipAge = t * t * 0.3;
    const col = lerpC(baseCol, aged, tipAge).map(c => Math.min(1, c * shade));

    midrib.push({ x, y, z, perpX, perpY, perpZ, color: col });
  }

  for (let i = 0; i < midrib.length - 1; i++) {
    const a = midrib[i], b = midrib[i + 1];
    quads.push({
      verts: [
        [a.x + a.perpX, a.y + a.perpY, a.z + a.perpZ],
        [b.x + b.perpX, b.y + b.perpY, b.z + b.perpZ],
        [b.x - b.perpX, b.y - b.perpY, b.z - b.perpZ],
        [a.x - a.perpX, a.y - a.perpY, a.z - a.perpZ],
      ],
      color: lerpC(a.color, b.color, 0.5),
    });
  }
  return quads;
}

/* ═══ 3D RENDERER ═══ */
function render3D(ctx, W, H, T, rotY, tilt, seed) {
  ctx.clearRect(0, 0, W, H);
  const ld = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  const extent = Math.max(T.blade_h * 1.3, T.blade_w * 4);
  const scale = Math.min(W, H) * 0.36 / (extent * 0.5);
  const midY = T.blade_h * 0.45;

  const rv = (v) => {
    const vy = v[1] - midY;
    const x1 = v[0] * cosR + v[2] * sinR;
    const z1 = -v[0] * sinR + v[2] * cosR;
    return [x1, vy * cosT - z1 * sinT, vy * sinT + z1 * cosT];
  };
  const pj = (v) => [W / 2 + v[0] * scale, H / 2 - v[1] * scale];

  const faces = [];
  const addQ = (a, b, c, d, col) => {
    const ra = rv(a), rb = rv(b), rc = rv(c), rd = rv(d);
    const n = normalize3(cross3(sub3(rb, ra), sub3(rc, ra)));
    const dt = -(n[0] * ld[0] + n[1] * ld[1] + n[2] * ld[2]);
    const li = Math.max(0.2, Math.min(1, 0.3 + dt * 0.7));
    faces.push({ v: [ra, rb, rc, rd], col, li, z: (ra[2] + rb[2] + rc[2] + rd[2]) / 4 });
  };

  // Blades
  const nBlades = Math.max(2, Math.round(T.blade_count));
  for (let b = 0; b < nBlades; b++) {
    const quads = bladeGeometry(T, b, nBlades, seed);
    for (const q of quads) {
      addQ(q.verts[0], q.verts[1], q.verts[2], q.verts[3], q.color);
    }
  }

  // Ground disc (subtle)
  const gr = T.blade_w * 1.5;
  const gSegs = 12;
  for (let s = 0; s < gSegs; s++) {
    const a0 = (s / gSegs) * Math.PI * 2;
    const a1 = ((s + 1) / gSegs) * Math.PI * 2;
    const ra = rv([Math.cos(a0) * gr, 0, Math.sin(a0) * gr]);
    const rb = rv([Math.cos(a1) * gr, 0, Math.sin(a1) * gr]);
    const rc = rv([0, 0, 0]);
    const n = normalize3(cross3(sub3(rb, ra), sub3(rc, ra)));
    const dt = -(n[0] * ld[0] + n[1] * ld[1] + n[2] * ld[2]);
    const li = Math.max(0.15, 0.2 + dt * 0.3);
    faces.push({ v: [ra, rb, rc], col: [0.35, 0.30, 0.22], li, z: (ra[2] + rb[2] + rc[2]) / 3 });
  }

  faces.sort((a, b) => a.z - b.z);
  for (const f of faces) {
    const pts = f.v.map(pj);
    ctx.fillStyle = rgb01(f.col[0] * f.li, f.col[1] * f.li, f.col[2] * f.li);
    ctx.beginPath(); ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath(); ctx.fill();
  }

  // Ground line
  ctx.strokeStyle = "rgba(100,200,100,0.15)"; ctx.lineWidth = 1;
  const gY = H / 2 + midY * scale * cosT;
  ctx.beginPath(); ctx.moveTo(0, gY); ctx.lineTo(W, gY); ctx.stroke();
}

/* ═══ 2D PROFILE ═══ */
function render2D(ctx, W, H, T, seed) {
  ctx.clearRect(0, 0, W, H);
  ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const nBlades = Math.max(2, Math.round(T.blade_count));
  const mg = { l: 16, r: 16, t: 10, b: 10 }, pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  const maxH = T.blade_h * 1.5, maxW = T.blade_h * T.splay + T.blade_w;
  const scY = ph / maxH, scX = pw / 2 / (maxW * 1.1 || 1);
  const sc = Math.min(scY, scX), cx = W / 2, baseY = mg.t + ph;
  const toS = (x, y) => [cx + x * sc, baseY - y * sc];

  const GA = Math.PI * (3 - Math.sqrt(5));
  for (let b = 0; b < nBlades; b++) {
    const az = b * GA;
    const cosAz = Math.cos(az);
    const rank = b / Math.max(1, nBlades - 1);
    const splayAng = T.splay * (0.6 + 0.4 * (1.0 - rank));
    const hMult = 1.0 + (hf(seed, 970 + b) - 0.5) * (T.blade_h_var || 0) * 2;
    const bladeH = T.blade_h * Math.max(0.4, hMult);
    const aged = T.blade_color_aged || T.blade_color;
    const col = lerpC(T.blade_color, aged, (1.0 - rank) * 0.5);
    const alpha = 0.3 + 0.4 * Math.abs(cosAz);

    ctx.strokeStyle = `rgba(${Math.round(col[0] * 255)},${Math.round(col[1] * 255)},${Math.round(col[2] * 255)},${alpha.toFixed(2)})`;
    ctx.lineWidth = Math.max(1, T.blade_w * sc * 0.5);
    ctx.beginPath();
    const segs = 12;
    for (let i = 0; i <= segs; i++) {
      const t = i / segs;
      const dist = t * bladeH;
      const cosS = Math.cos(splayAng), sinS = Math.sin(splayAng);
      const dx = cosAz * sinS * dist + cosAz * T.curve * bladeH * t * t;
      const dy = cosS * dist;
      const [sx, sy] = toS(dx, dy);
      if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
    }
    ctx.stroke();

    // Width envelope
    ctx.strokeStyle = `rgba(${Math.round(col[0] * 200)},${Math.round(col[1] * 200)},${Math.round(col[2] * 200)},${(alpha * 0.3).toFixed(2)})`;
    ctx.lineWidth = 0.5;
    for (const side of [-1, 1]) {
      ctx.beginPath();
      for (let i = 0; i <= segs; i++) {
        const t = i / segs;
        const dist = t * bladeH;
        const cosS = Math.cos(splayAng), sinS = Math.sin(splayAng);
        const dx = cosAz * sinS * dist + cosAz * T.curve * bladeH * t * t;
        const dy = cosS * dist;
        const baseFrac = 0.3 + 0.7 * Math.min(1, t * 4);
        const tipFrac = 1.0 - Math.pow(t, T.taper * 2.5 + 0.5);
        const w = T.blade_w * baseFrac * tipFrac * side;
        const [sx, sy] = toS(dx + w * 0.5 * (1 - Math.abs(cosAz)), dy);
        if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
      }
      ctx.stroke();
    }
  }

  // Ground
  const [gx0, gy] = toS(-maxW * 1.1, 0);
  const [gx1] = toS(maxW * 1.1, 0);
  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(gx0, gy); ctx.lineTo(gx1, gy); ctx.stroke(); ctx.setLineDash([]);

  ctx.fillStyle = "rgba(255,255,255,0.3)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  ctx.fillText(`h=${T.blade_h.toFixed(1)}`, cx, mg.t + 8);
}

/* ═══ C++ EXPORT ═══ */
function genCpp(tiers) {
  const f = (v, d = 2) => { const s = v.toFixed(d); return s.includes(".") ? s + "f" : s + ".0f"; };
  const lines = [];
  lines.push("// ─── Blade Cluster Tier Definitions ───────────────────────────────────");
  lines.push("//");
  lines.push("//                         bC_μ  σ    bH_μ  σ    hVar  σ    bW_μ  σ    sply  σ    crv   σ    twst  σ    tpr   σ     col%  cVar  bSeg wt");
  lines.push("static constexpr BladeClusterTierParams BLADE_TIERS[] = {");
  tiers.forEach((T, i) => {
    const vals = [
      T.blade_count, T.blade_count_s, T.blade_h, T.blade_h_s,
      T.blade_h_var, T.blade_h_var_s, T.blade_w, T.blade_w_s,
      T.splay, T.splay_s, T.curve, T.curve_s,
      T.twist, T.twist_s, T.taper, T.taper_s,
      T.color_over, T.color_var,
    ].map(v => f(v));
    vals.push(String(T.blade_segs), f(T.weight));
    lines.push(`    /* ${TIER_NAMES[i].padEnd(10)} */  { ${vals.join(", ")} },`);
  });
  lines.push("};");
  return lines.join("\n");
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
function MuSigma({ label, mu, muSet, sigma, sigmaSet, muMin = 0, muMax = 100, sMax = 20, muW = 46 }) {
  return (<div style={rw}><span style={lb}>{label}</span><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>μ</span><Num value={mu} min={muMin} max={muMax} w={muW} onChange={muSet} /><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>σ</span><Num value={sigma} max={sMax} w={38} onChange={sigmaSet} /></div>);
}
function PaletteSwatches({ palette, label, onPick }) {
  return (<div style={{ ...rw, marginBottom: 5 }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginRight: 4 }}>{label}</span>{palette.map((p, i) => (<span key={i} title={p.name} onClick={() => onPick(p.c.slice())} style={{ width: 14, height: 14, borderRadius: 3, cursor: "pointer", background: rgb01(...p.c), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />))}</div>);
}
function DragPanel({ title, children, ini = false, id, resetKey, onDock }) {
  const [open, setOpen] = useState(ini); const [pos, setPos] = useState(null); const [dragging, setDragging] = useState(false);
  const dragRef = useRef(null); const offsetRef = useRef({ x: 0, y: 0 });
  useEffect(() => { setPos(null); }, [resetKey]);
  const startDrag = useCallback(e => { if (e.target.closest("[data-notdrag]")) return; e.preventDefault(); const rect = dragRef.current.getBoundingClientRect(); offsetRef.current = { x: e.clientX - rect.left, y: e.clientY - rect.top }; setDragging(true); }, []);
  useEffect(() => { if (!dragging) return; const onMove = e => { const p = dragRef.current.parentElement.getBoundingClientRect(); setPos({ x: e.clientX - p.left - offsetRef.current.x, y: e.clientY - p.top - offsetRef.current.y }); }; const onUp = () => setDragging(false); window.addEventListener("mousemove", onMove); window.addEventListener("mouseup", onUp); return () => { window.removeEventListener("mousemove", onMove); window.removeEventListener("mouseup", onUp); }; }, [dragging]);
  const doDock = () => { setPos(null); if (onDock) onDock(id); };
  const style = pos ? { position: "absolute", left: pos.x, top: pos.y, zIndex: 100, maxWidth: 520, minWidth: 280 } : {};
  return (<div ref={dragRef} style={{ marginBottom: pos ? 0 : 5, border: pos ? "1px solid #666" : "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: pos ? "#2a2a30" : "var(--color-background-primary)", boxShadow: pos ? "0 8px 32px rgba(0,0,0,.6)" : "none", ...style }}><div onMouseDown={startDrag} style={{ padding: "5px 10px", fontSize: 12, fontWeight: 500, userSelect: "none", display: "flex", alignItems: "center", gap: 5, color: pos ? "#e0ddd8" : "var(--color-text-primary)", background: pos ? "#353540" : "var(--color-background-secondary)", cursor: "grab" }}><span data-notdrag="1" onClick={() => setOpen(!open)} style={{ cursor: "pointer", fontSize: 9, transition: "transform .15s", display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px" }}>▶</span><span style={{ flex: 1 }}>{title}</span>{pos && <span data-notdrag="1" onClick={doDock} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}</div>{open && <div style={{ padding: "5px 10px", maxHeight: pos ? 400 : "none", overflowY: pos ? "auto" : "visible", color: pos ? "#d0cdc8" : undefined }}>{children}</div>}</div>);
}

const rw = { display: "flex", flexWrap: "wrap", gap: "3px 8px", alignItems: "center", marginBottom: 3 };
const lb = { fontSize: 10, color: "var(--color-text-secondary)", minWidth: 56 };
const btnStyle = { fontSize: 10, padding: "2px 8px", borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-secondary)" };

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

const STORAGE_KEY = "7t:blade-cluster:tiers";
const DEFAULTS = () => [0, 1, 2].map(defaultTier);
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

/* ═══ MAIN COMPONENT ═══ */
export default function BladeClusterDesigner() {
  const [tierIdx, setTierIdx] = useState(1);
  const [tiers, setTiers] = useState(DEFAULTS);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const upd = (fn) => setTiers(prev => { const n = JSON.parse(JSON.stringify(prev)); fn(n[tierIdx]); return n; });

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

  const [seed, setSeed] = useState(42);
  const resolved = resolveColors(T, seed);
  const RT = { ...T, blade_color: resolved.bladeCol, blade_color_aged: resolved.agedCol };

  const [rotY, setRotY] = useState(0.5);
  const [tilt, setTilt] = useState(0.2);
  const [autoRot, setAutoRot] = useState(true);
  const cv3dRef = useRef(null);
  const cv2dRef = useRef(null);
  const animRef = useRef(null);

  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);
  const defaultOrder = ["blades", "shape", "appearance", "quality", "export"];
  const [panelOrder, setPanelOrder] = useState([...defaultOrder]);
  const [dockKey, setDockKey] = useState(0);
  const resetAll = () => { setTiers(DEFAULTS()); setDockKey(k => k + 1); };
  const resetTier = () => setTiers(prev => { const n = [...prev]; n[tierIdx] = defaultTier(tierIdx); return n; });
  const cppCode = genCpp(tiers);


  useEffect(() => {
    let frame = 0;
    const tick = () => {
      const c3 = cv3dRef.current, c2 = cv2dRef.current;
      if (c3) {
        const rect = c3.parentElement.getBoundingClientRect();
        c3.width = rect.width * devicePixelRatio;
        c3.height = rect.height * devicePixelRatio;
        const ctx = c3.getContext("2d");
        ctx.setTransform(devicePixelRatio, 0, 0, devicePixelRatio, 0, 0);
        const r = autoRot ? rotY + frame * 0.008 : rotY;
        render3D(ctx, rect.width, rect.height, RT, r, tilt, seed);
      }
      if (c2) {
        const rect = c2.parentElement.getBoundingClientRect();
        c2.width = rect.width * devicePixelRatio;
        c2.height = rect.height * devicePixelRatio;
        const ctx = c2.getContext("2d");
        ctx.setTransform(devicePixelRatio, 0, 0, devicePixelRatio, 0, 0);
        render2D(ctx, rect.width, rect.height, RT, seed);
      }
      frame++;
      animRef.current = requestAnimationFrame(tick);
    };
    tick();
    return () => cancelAnimationFrame(animRef.current);
  }, [RT, rotY, tilt, autoRot, seed]);

  // Camera. Attached via the canvas's onPointerDown JSX prop, NOT
  // addEventListener — React binds at render time, so the listener cannot be
  // lost to the Loading… placeholder's mount ordering (see the note on the
  // wheel effect). Left/middle drag rotates.
  const onPointerDown3D = useCallback(e => {
    const sx = e.clientX, sy = e.clientY;
    const sr = rotY, st = tilt;
    setAutoRot(false);
    const onMove = ev => {
      setRotY(sr + (ev.clientX - sx) * 0.01);
      setTilt(st - (ev.clientY - sy) * 0.008);
    };
    const onUp = () => {
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
    };
    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
  }, [rotY, tilt]);

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "#555" }}>Loading…</div>;

  return (
    <div style={{ display: "flex", gap: 8, height: "100vh", padding: 8, fontFamily: "'JetBrains Mono', monospace", fontSize: 11, overflow: "hidden", background: "var(--color-background-primary)", color: "var(--color-text-primary)", boxSizing: "border-box", position: "relative" }}>
      <div style={{ flex: "1 1 55%", display: "flex", flexDirection: "column", gap: 6, minWidth: 0 }}>
        <div style={{ display: "flex", gap: 4, alignItems: "center", flexWrap: "wrap" }}>
          {TIER_NAMES.map((name, i) => (
            <button key={i} onClick={() => setTierIdx(i)} style={{ ...btnStyle, background: i === tierIdx ? "var(--color-background-info)" : "var(--color-background-secondary)", color: i === tierIdx ? "var(--color-text-on-info)" : "var(--color-text-secondary)", fontWeight: i === tierIdx ? 600 : 400 }}>{name}</button>
          ))}
          <span style={{ flex: 1 }} />
          <button onClick={() => setAutoRot(!autoRot)} style={{ ...btnStyle, opacity: autoRot ? 1 : 0.5 }}>{autoRot ? "◉ spin" : "○ spin"}</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
          {isModified && <span style={{ fontSize: 9, color: "#666", marginLeft: 4 }}>● modified</span>}
        </div>
        <div style={{ display: "flex", gap: 6, alignItems: "center", fontSize: 10 }}>
          <span style={{ color: "var(--color-text-secondary)" }}>seed</span>
          <Num value={seed} min={0} max={99999} step={1} w={52} onChange={v => setSeed(Math.round(v))} />
          <button onClick={() => setSeed(Math.floor(Math.random() * 99999))} style={btnStyle}>reroll</button>
          <span style={{ color: "var(--color-text-tertiary)", marginLeft: 4 }}>blade</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.bladeCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
          <span style={{ color: "var(--color-text-tertiary)" }}>aged</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.agedCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
        </div>
        <div style={{ flex: "1 1 65%", background: "#1a1a1e", borderRadius: 8, overflow: "hidden", cursor: "grab", minHeight: 180 }}>
          <canvas
            ref={cv3dRef}
            onPointerDown={onPointerDown3D}
            onContextMenu={e => e.preventDefault()}
            style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }}
          />
        </div>
        <div style={{ flex: "0 0 28%", background: "#111114", borderRadius: 8, overflow: "hidden", minHeight: 100 }}>
          <canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} />
        </div>
      </div>

      <div style={{ flex: "0 0 310px", overflowY: "auto", overflowX: "hidden", paddingRight: 2 }}>
        {panelOrder.map(pid => {
          const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };
          switch (pid) {

            case "blades": return (<DragPanel {...dp} title="Blade Count & Size" ini={true}>
              <MuSigma label="Blade count" mu={T.blade_count} muSet={v => upd(n => { n.blade_count = Math.round(v); })} sigma={T.blade_count_s} sigmaSet={v => upd(n => { n.blade_count_s = v; })} muMin={2} muMax={10} sMax={3} muW={34} />
              <MuSigma label="Blade height" mu={T.blade_h} muSet={v => upd(n => { n.blade_h = v; })} sigma={T.blade_h_s} sigmaSet={v => upd(n => { n.blade_h_s = v; })} muMin={0.5} muMax={12} sMax={4} />
              <MuSigma label="Height var" mu={T.blade_h_var} muSet={v => upd(n => { n.blade_h_var = v; })} sigma={T.blade_h_var_s} sigmaSet={v => upd(n => { n.blade_h_var_s = v; })} muMax={0.8} sMax={0.2} />
              <MuSigma label="Blade width" mu={T.blade_w} muSet={v => upd(n => { n.blade_w = v; })} sigma={T.blade_w_s} sigmaSet={v => upd(n => { n.blade_w_s = v; })} muMin={0.05} muMax={2} sMax={0.5} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Height var = per-blade height randomization (hash-driven). Gives the cluster an asymmetric silhouette — each blade is a different height from the same base parameters.</div>
            </DragPanel>);

            case "shape": return (<DragPanel {...dp} title="Shape & Posture" ini={true}>
              <MuSigma label="Splay" mu={T.splay} muSet={v => upd(n => { n.splay = v; })} sigma={T.splay_s} sigmaSet={v => upd(n => { n.splay_s = v; })} muMax={1.2} sMax={0.3} />
              <MuSigma label="Curve" mu={T.curve} muSet={v => upd(n => { n.curve = v; })} sigma={T.curve_s} sigmaSet={v => upd(n => { n.curve_s = v; })} muMax={0.8} sMax={0.2} />
              <MuSigma label="Twist" mu={T.twist} muSet={v => upd(n => { n.twist = v; })} sigma={T.twist_s} sigmaSet={v => upd(n => { n.twist_s = v; })} muMax={0.5} sMax={0.15} />
              <MuSigma label="Taper" mu={T.taper} muSet={v => upd(n => { n.taper = v; })} sigma={T.taper_s} sigmaSet={v => upd(n => { n.taper_s = v; })} muMin={0.3} muMax={1.0} sMax={0.2} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Splay = outward tilt from vertical (0=straight up, ~0.3=natural spread). Curve = quadratic outward arc along blade length. Twist = blade surface rotation along its axis. Taper = how quickly the blade narrows to tip (higher=sharper point).</div>
            </DragPanel>);

            case "appearance": return (<DragPanel {...dp} title="Appearance">
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>Blade (young / inner)</div>
              <PaletteSwatches palette={BLADE_PALETTE} label="Blade palette" onPick={c => upd(n => { n.blade_color = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"b" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.blade_color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.blade_color[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.blade_color), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Blade (aged / outer)</div>
              <PaletteSwatches palette={BLADE_AGED_PALETTE} label="Aged palette" onPick={c => upd(n => { n.blade_color_aged = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"a" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.blade_color_aged[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.blade_color_aged[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.blade_color_aged), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={rw}>
                <span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v => upd(n => { n.color_over = v; })} />
                <span style={lb}>Variance</span><Num value={T.color_var} min={0} max={0.3} w={38} onChange={v => upd(n => { n.color_var = v; })} />
              </div>
            </DragPanel>);

            case "quality": return (<DragPanel {...dp} title="Quality & Selection">
              <div style={rw}>
                <span style={lb}>Blade segs</span><Num value={T.blade_segs} min={3} max={12} step={1} w={34} onChange={v => upd(n => { n.blade_segs = Math.round(v); })} />
                <span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v => upd(n => { n.weight = v; })} />
              </div>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Blade segs = quads per blade midrib. Weight = tier selection probability.</div>
            </DragPanel>);

            case "export": return (<DragPanel {...dp} title="C++ Export">
              <div style={{ display: "flex", gap: 4, marginBottom: 4 }}>
                <button onClick={() => { navigator.clipboard?.writeText(cppCode); setCopied(true); setTimeout(() => setCopied(false), 1500); }} style={{ ...btnStyle, background: copied ? "var(--color-background-success)" : "var(--color-background-secondary)" }}>{copied ? "Copied" : "Copy C++"}</button>
                <button onClick={() => setShowCode(!showCode)} style={btnStyle}>{showCode ? "Hide" : "Show"} code</button>
              </div>
              {showCode && <textarea readOnly value={cppCode} style={{ width: "100%", height: 100, fontSize: 10, fontFamily: "'JetBrains Mono',monospace", background: "var(--color-background-primary)", color: "var(--color-text-primary)", border: "1px solid var(--color-border-tertiary)", borderRadius: 4, padding: 6, resize: "vertical", lineHeight: 1.4, whiteSpace: "pre" }} />}
            </DragPanel>);

            default: return null;
          }
        })}
      </div>
    </div>
  );
}
