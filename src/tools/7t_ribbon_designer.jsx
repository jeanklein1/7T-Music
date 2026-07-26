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
   7T RIBBON DESIGNER
   Animated cube-chain spine with 3 compound waves (lateral, vertical, twist).
   Harmonic ratio palettes for vertical/twist cycles.
   3 tiers: Serpentine / Helix / Streamer.
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Serpentine", "Helix", "Streamer"];

/* ═══ PALETTES & COLOR MODES (from cartridge.hpp) ═══ */
const RIBBON_SMOOTH_PALETTE = [
  { name: "Warm sandstone", c: [0.82, 0.75, 0.62] },
  { name: "Sky blue",       c: [0.55, 0.65, 0.78] },
  { name: "Golden",         c: [0.85, 0.78, 0.58] },
  { name: "Sage green",     c: [0.50, 0.68, 0.55] },
];
const RIBBON_COLOR_MODES = [
  { name: "Smooth", weight: 0.40 },
  { name: "Tinted", weight: 0.35 },
  { name: "Contrast", weight: 0.25 },
];

const VERTICAL_RATIOS = [
  { ratio: 1/3, name: "1:3", weight: 0.15 },
  { ratio: 1/2, name: "1:2", weight: 0.35 },
  { ratio: 2/3, name: "2:3", weight: 0.30 },
  { ratio: 1/1, name: "1:1", weight: 0.20 },
];
const TWIST_RATIOS = [
  { ratio: 1/4, name: "1:4", weight: 0.15 },
  { ratio: 1/3, name: "1:3", weight: 0.30 },
  { ratio: 1/2, name: "1:2", weight: 0.35 },
  { ratio: 2/3, name: "2:3", weight: 0.20 },
];

function defaultTier(idx) {
  const D = [
    // Serpentine
    { cubes: 350, cubes_s: 60, cube_size: 4.0, cube_size_s: 1.0, height: 60, height_s: 15,
      lat_amp: 8.4, lat_amp_s: 0.4, lat_cyc: 1.5, lat_cyc_s: 0.4, lat_spd: 0.25, lat_spd_s: 0.075,
      vert_amp: 3.0, vert_amp_s: 0.5, vert_cyc: 0.8, vert_cyc_s: 0.2, vert_spd: 0.20, vert_spd_s: 0.06, vert_ratio_idx: 1,
      twist_amp: 0.6, twist_amp_s: 0.2, twist_cyc: 0.5, twist_cyc_s: 0.2, twist_spd: 0.15, twist_spd_s: 0.05, twist_ratio_idx: 2,
      weight: 0.45, color: [0.82, 0.75, 0.62] },
    // Helix
    { cubes: 150, cubes_s: 40, cube_size: 2.5, cube_size_s: 0.6, height: 55, height_s: 12,
      lat_amp: 1.5, lat_amp_s: 0.4, lat_cyc: 3.0, lat_cyc_s: 0.8, lat_spd: 0.60, lat_spd_s: 0.175,
      vert_amp: 1.0, vert_amp_s: 0.3, vert_cyc: 2.5, vert_cyc_s: 0.6, vert_spd: 0.50, vert_spd_s: 0.15, vert_ratio_idx: 1,
      twist_amp: 6.0, twist_amp_s: 0.8, twist_cyc: 2.0, twist_cyc_s: 0.5, twist_spd: 0.20, twist_spd_s: 0.05, twist_ratio_idx: 2,
      weight: 0.30, color: [0.55, 0.65, 0.78] },
    // Streamer
    { cubes: 250, cubes_s: 50, cube_size: 3.0, cube_size_s: 0.8, height: 70, height_s: 20,
      lat_amp: 4.0, lat_amp_s: 0.6, lat_cyc: 2.0, lat_cyc_s: 0.5, lat_spd: 0.40, lat_spd_s: 0.10,
      vert_amp: 6.0, vert_amp_s: 1.0, vert_cyc: 1.2, vert_cyc_s: 0.3, vert_spd: 0.325, vert_spd_s: 0.075, vert_ratio_idx: 2,
      twist_amp: 1.6, twist_amp_s: 0.6, twist_cyc: 1.5, twist_cyc_s: 0.4, twist_spd: 0.25, twist_spd_s: 0.08, twist_ratio_idx: 1,
      weight: 0.25, color: [0.85, 0.78, 0.58] },
  ];
  return JSON.parse(JSON.stringify(D[idx]));
}

/* ═══ SPINE EVALUATION (mirrors WGSL ribbon_spine_at) ═══ */
function spineAt(t, T, time) {
  const vr = VERTICAL_RATIOS[T.vert_ratio_idx].ratio;
  const tr = TWIST_RATIOS[T.twist_ratio_idx].ratio;

  // Length of the physical chain (cube_count × cube_size)
  const totalLength = T.cubes * T.cube_size;
  const along = (t - 0.5) * totalLength;

  // Lateral wave (horizontal sway perpendicular to chain)
  const latPhase = time * T.lat_spd + t * T.lat_cyc * Math.PI * 2;
  const lateral = Math.sin(latPhase) * T.lat_amp;

  // Vertical wave (breathing — oscillates around Y=0 in preview, altitude in engine)
  const vertPhase = time * T.lat_spd * vr + t * T.lat_cyc * vr * Math.PI * 2;
  const vertical = Math.sin(vertPhase) * T.vert_amp;

  // Twist: helical displacement perpendicular to spine (matches WGSL 0.4/0.3 factors)
  const twistPhase = time * T.lat_spd * tr + t * T.lat_cyc * tr * Math.PI * 2;
  const twistDepth = Math.sin(twistPhase) * 0.4 * T.twist_amp;
  const twistVert  = Math.cos(twistPhase) * 0.3 * T.twist_amp;

  // orientation = 0 → along is X, lateral is Z (matches WGSL with cos(0)=1, sin(0)=0)
  return {
    x: along,
    y: vertical + twistVert,
    z: lateral + twistDepth,
  };
}

/* ═══ 3D RENDERER ═══ */
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }
function rgb01(r, g, b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }

function render3D(ctx, W, H, T, rotY, tilt, time, zoom, panX = 0, panY = 0, view = null) {
  if (!view?.noClear) ctx.clearRect(0, 0, W, H);
  const N = Math.min(Math.round(T.cubes), 400);
  const cubeHalf = T.cube_size * 0.5;
  const col = T.color;

  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  // Ribbon is a horizontal chain: length along X, sway along Z, breathing along Y
  const totalLength = T.cubes * T.cube_size;
  const maxLateral = T.lat_amp + 0.4 * T.twist_amp;
  const maxVertical = T.vert_amp + 0.3 * T.twist_amp;
  const extent = Math.max(totalLength, maxLateral * 2 + T.cube_size, maxVertical * 2 + T.cube_size);
  const scale = (view?.scale ?? Math.min(W, H) * 0.40 / (extent * 0.5)) * (zoom || 1);
  const midY = 0; // vertical wave centers on 0
  const rotVert = (v) => { const vy=v[1]-midY; const x1=v[0]*cosR+v[2]*sinR; const z1=-v[0]*sinR+v[2]*cosR; return [x1, vy*cosT-z1*sinT, vy*sinT+z1*cosT]; };
  const project = (v) => [W/2 + panX + v[0]*scale, H/2 + panY - v[1]*scale];

  const faces = [];
  // Face indices for standard box convention:
  //   bottom 0-3: (-x,-y,-z), (+x,-y,-z), (+x,-y,+z), (-x,-y,+z)
  //   top    4-7: (-x,+y,-z), (+x,+y,-z), (+x,+y,+z), (-x,+y,+z)
  const faceIdx = [[0,3,2,1],[4,5,6,7],[0,1,5,4],[2,3,7,6],[1,2,6,5],[0,4,7,3]];

  // Sample every few cubes for performance
  const step = Math.max(1, Math.floor(N / 120));
  for (let i = 0; i < N; i += step) {
    const t = i / (N - 1);
    const s = spineAt(t, T, time);
    const ch = cubeHalf;

    // 8 box corners in standard convention (bottom then top)
    const corners = [
      rotVert([s.x - ch, s.y - ch, s.z - ch]),
      rotVert([s.x + ch, s.y - ch, s.z - ch]),
      rotVert([s.x + ch, s.y - ch, s.z + ch]),
      rotVert([s.x - ch, s.y - ch, s.z + ch]),
      rotVert([s.x - ch, s.y + ch, s.z - ch]),
      rotVert([s.x + ch, s.y + ch, s.z - ch]),
      rotVert([s.x + ch, s.y + ch, s.z + ch]),
      rotVert([s.x - ch, s.y + ch, s.z + ch]),
    ];

    for (let fi = 0; fi < 6; fi++) {
      const fv = faceIdx[fi].map(vi => corners[vi]);
      const e1 = sub3(fv[1], fv[0]), e2 = sub3(fv[3], fv[0]);
      const n = normalize3(cross3(e1, e2));
      if (n[2] > 0.02) continue;
      const diff = Math.max(0, n[0]*lightDir[0]+n[1]*lightDir[1]+n[2]*lightDir[2]);
      const light = 0.22 + 0.78 * diff;
      // Slight color variation along spine
      const cv = 0.85 + 0.15 * t;
      const avgZ = fv.reduce((s, v) => s + v[2], 0) / fv.length;
      faces.push({ verts: fv, light, col: [col[0]*cv, col[1]*cv, col[2]*cv], avgZ });
    }
  }

  faces.sort((a, b) => a.avgZ - b.avgZ);
  for (const f of faces) {
    const pts = f.verts.map(project);
    ctx.fillStyle = rgb01(f.col[0]*f.light, f.col[1]*f.light, f.col[2]*f.light);
    ctx.beginPath(); ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath(); ctx.fill();
  }
}

/* ═══ 2D SPINE WAVES ═══ */
function render2D(ctx, W, H, T, time) {
  ctx.clearRect(0, 0, W, H); ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const mg = { l: 30, r: 10, t: 10, b: 20 };
  const pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;

  const vr = VERTICAL_RATIOS[T.vert_ratio_idx].ratio;
  const tr = TWIST_RATIOS[T.twist_ratio_idx].ratio;
  const N = 200;
  const third = ph / 3;

  // Compute isolated wave components (not combined spine position)
  function wavesAt(t) {
    const latPhase = time * T.lat_spd + t * T.lat_cyc * Math.PI * 2;
    const vertPhase = time * T.lat_spd * vr + t * T.lat_cyc * vr * Math.PI * 2;
    const twistPhase = time * T.lat_spd * tr + t * T.lat_cyc * tr * Math.PI * 2;
    return {
      lateral: Math.sin(latPhase) * T.lat_amp,
      vertical: Math.sin(vertPhase) * T.vert_amp,
      twistDepth: Math.sin(twistPhase) * 0.4 * T.twist_amp,
      twistVert: Math.cos(twistPhase) * 0.3 * T.twist_amp,
    };
  }

  const maxLat = T.lat_amp * 1.2 || 1;
  const maxVert = (T.vert_amp + 0.3 * T.twist_amp) * 1.2 || 1;
  const maxTwist = (0.4 * T.twist_amp) * 1.2 || 1;

  // Lateral wave (horizontal sway)
  ctx.strokeStyle = "rgba(200,154,108,0.8)"; ctx.lineWidth = 1.5;
  ctx.beginPath();
  for (let i = 0; i <= N; i++) {
    const t = i / N;
    const w = wavesAt(t);
    const sx = mg.l + t * pw;
    const sy = mg.t + third * 0.5 - (w.lateral / maxLat) * (third * 0.5 - 4);
    i === 0 ? ctx.moveTo(sx, sy) : ctx.lineTo(sx, sy);
  }
  ctx.stroke();

  // Vertical wave (breathing + twist vertical component)
  ctx.strokeStyle = "rgba(108,170,200,0.8)"; ctx.lineWidth = 1.5;
  ctx.beginPath();
  for (let i = 0; i <= N; i++) {
    const t = i / N;
    const w = wavesAt(t);
    const sx = mg.l + t * pw;
    const sy = mg.t + third * 1.5 - ((w.vertical + w.twistVert) / maxVert) * (third * 0.5 - 4);
    i === 0 ? ctx.moveTo(sx, sy) : ctx.lineTo(sx, sy);
  }
  ctx.stroke();

  // Twist corkscrew (depth displacement)
  ctx.strokeStyle = "rgba(170,120,200,0.8)"; ctx.lineWidth = 1.5;
  ctx.beginPath();
  for (let i = 0; i <= N; i++) {
    const t = i / N;
    const w = wavesAt(t);
    const sx = mg.l + t * pw;
    const sy = mg.t + third * 2.5 - (w.twistDepth / maxTwist) * (third * 0.5 - 4);
    i === 0 ? ctx.moveTo(sx, sy) : ctx.lineTo(sx, sy);
  }
  ctx.stroke();

  // Labels
  ctx.font = "9px monospace"; ctx.textAlign = "left";
  ctx.fillStyle = "rgba(200,154,108,0.5)";
  ctx.fillText("lateral", mg.l, mg.t + 10);
  ctx.fillStyle = "rgba(108,170,200,0.5)";
  ctx.fillText("vertical", mg.l, mg.t + third + 10);
  ctx.fillStyle = "rgba(170,120,200,0.5)";
  ctx.fillText("twist", mg.l, mg.t + third * 2 + 10);

  // Center lines
  ctx.strokeStyle = "rgba(255,255,255,0.1)"; ctx.lineWidth = 0.5; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(mg.l, mg.t + third * 0.5); ctx.lineTo(mg.l + pw, mg.t + third * 0.5); ctx.stroke();
  ctx.beginPath(); ctx.moveTo(mg.l, mg.t + third * 1.5); ctx.lineTo(mg.l + pw, mg.t + third * 1.5); ctx.stroke();
  ctx.beginPath(); ctx.moveTo(mg.l, mg.t + third * 2.5); ctx.lineTo(mg.l + pw, mg.t + third * 2.5); ctx.stroke();
  ctx.setLineDash([]);
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

function RatioButtons({ ratios, value, onChange }) {
  return (<div style={{ display: "flex", gap: 2, flexWrap: "wrap" }}>{ratios.map((r, i) => (
    <button key={i} onClick={() => onChange(i)} style={{ fontSize: 9, padding: "2px 6px", borderRadius: 3, cursor: "pointer", fontFamily: "monospace",
      border: i === value ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)",
      background: i === value ? "var(--color-background-info)" : "transparent",
      color: i === value ? "var(--color-text-info)" : "var(--color-text-tertiary)", fontWeight: i === value ? 600 : 400 }}>
      {r.name} <span style={{ fontSize: 8, opacity: 0.6 }}>{(r.weight*100).toFixed(0)}%</span>
    </button>
  ))}</div>);
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

const STORAGE_KEY = "7t:ribbon:tiers";
const DEFAULTS = () => TIER_NAMES.map((_, i) => defaultTier(i));
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

export default function RibbonDesigner() {
  const [tierIdx, setTierIdx] = useState(0);
  const [tiers, setTiers] = useState(DEFAULTS);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const [rotY, setRotY] = useState(0.4); const [tilt, setTilt] = useState(0.1);
  const [zoom, setZoom] = useState(1);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);
  const [playing, setPlaying] = useState(true);
  const [dockKey, setDockKey] = useState(0);
  const [showCode, setShowCode] = useState(false);
  const defaultOrder = ["geometry", "lateral", "vertical", "twist", "appearance", "export"];
  const [panelOrder, setPanelOrder] = useState(defaultOrder);
  const [resizeTick, setResizeTick] = useState(0);
  const [time, setTime] = useState(0);
  const cv3dRef = useRef(null), cv2dRef = useRef(null), c3dRef = useRef(null), c2dRef = useRef(null);
  const animRef = useRef(null);
  const upd = fn => setTiers(prev => { const n = JSON.parse(JSON.stringify(prev)); fn(n[tierIdx]); return n; });

  function genCpp(tiers) {
    const r = (v, d=2) => v.toFixed(d) + "f";
    const lines = ["static constexpr RibbonTierParams RIBBON_TIERS[] = {"];
    tiers.forEach((T, i) => {
      lines.push(`    /* ${TIER_NAMES[i].toUpperCase().padEnd(12)} */  { ${r(T.cubes,1)}, ${r(T.cubes_s,1)},  ${r(T.cube_size)}, ${r(T.cube_size_s)},  ${r(T.height,1)}, ${r(T.height_s,1)},  ${r(T.lat_amp)}, ${r(T.lat_amp_s)},  ${r(T.lat_cyc)}, ${r(T.lat_cyc_s)},  ${r(T.lat_spd)}, ${r(T.lat_spd_s,3)},  ${r(T.vert_amp)}, ${r(T.vert_amp_s)},  ${r(T.vert_cyc)}, ${r(T.vert_cyc_s)},  ${r(T.vert_spd)}, ${r(T.vert_spd_s)},  ${r(T.twist_amp)}, ${r(T.twist_amp_s)},  ${r(T.twist_cyc)}, ${r(T.twist_cyc_s)},  ${r(T.twist_spd)}, ${r(T.twist_spd_s)},  ${r(T.weight)} },`);
    });
    lines.push("};");
    return lines.join("\n");
  }
  const cppCode = genCpp(tiers);

  // Load saved tiers on mount
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) { const data = JSON.parse(raw); if (Array.isArray(data) && data.length === TIER_NAMES.length) setTiers(data); }
      } catch (e) {}
      setLoaded(true);
    })();
  }, []);

  // Auto-save on every edit after initial load
  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(tiers)); } catch (e) {} })();
  }, [tiers, loaded]);

  useEffect(() => { const ro = new ResizeObserver(() => setResizeTick(t => t + 1)); if (c3dRef.current) ro.observe(c3dRef.current); if (c2dRef.current) ro.observe(c2dRef.current); return () => ro.disconnect(); }, [loaded]);

  // Animation loop
  useEffect(() => {
    if (!playing) { if (animRef.current) cancelAnimationFrame(animRef.current); return; }
    let last = performance.now();
    const tick = (now) => {
      const dt = (now - last) / 1000; last = now;
      setTime(t => t + dt);
      animRef.current = requestAnimationFrame(tick);
    };
    animRef.current = requestAnimationFrame(tick);
    return () => { if (animRef.current) cancelAnimationFrame(animRef.current); };
  }, [playing]);

  useEffect(() => { const cv = cv3dRef.current; if (!cv) return; const dpr = window.devicePixelRatio || 1; const W = cv.clientWidth, H = cv.clientHeight; cv.width = W * dpr; cv.height = H * dpr; const ctx = cv.getContext("2d"); ctx.scale(dpr, dpr); render3D(ctx, W, H, T, rotY, tilt, time, zoom, panX, panY); }, [T, rotY, tilt, time, resizeTick, zoom, panX, panY]);
  useEffect(() => { const cv = cv2dRef.current; if (!cv) return; const dpr = window.devicePixelRatio || 1; const W = cv.clientWidth, H = cv.clientHeight; cv.width = W * dpr; cv.height = H * dpr; const ctx = cv.getContext("2d"); ctx.scale(dpr, dpr); render2D(ctx, W, H, T, time); }, [T, time, resizeTick]);

  const onPointerDown3D = useCallback(e => { const sx = e.clientX, sy = e.clientY; if (e.button === 2) { e.preventDefault(); const spx = panX, spy = panY; const onMoveP = ev => { setPanX(spx + (ev.clientX - sx)); setPanY(spy + (ev.clientY - sy)); }; const onUpP = () => { window.removeEventListener("pointermove", onMoveP); window.removeEventListener("pointerup", onUpP); }; window.addEventListener("pointermove", onMoveP); window.addEventListener("pointerup", onUpP); return; } const sr = rotY, st = tilt, sz = zoom; const sens = 0.01 / Math.max(sz, 0.3); const onMove = ev => { setRotY(sr + (ev.clientX - sx) * sens); setTilt(st - (ev.clientY - sy) * sens * 0.8); }; const onUp = () => { window.removeEventListener("pointermove", onMove); window.removeEventListener("pointerup", onUp); }; window.addEventListener("pointermove", onMove); window.addEventListener("pointerup", onUp); }, [rotY, tilt, zoom, panX, panY]);

  // Scroll-to-zoom: non-passive listener so preventDefault() works
  useEffect(() => { const cv = cv3dRef.current; if (!cv) return; const handler = e => { e.preventDefault(); setZoom(z => Math.max(0.1, Math.min(20, z * (e.deltaY > 0 ? 0.9 : 1.1)))); }; cv.addEventListener("wheel", handler, { passive: false }); return () => cv.removeEventListener("wheel", handler); }, [loaded]);

  const resetAll = () => { setTiers(DEFAULTS()); setDockKey(k => k + 1); };
  const resetTier = () => setTiers(prev => { const n = [...prev]; n[tierIdx] = defaultTier(tierIdx); return n; });

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{ fontFamily: "'JetBrains Mono', 'SF Mono', monospace", color: "var(--color-text-primary)", lineHeight: 1.4, position: "relative", fontSize: 11 }}>
      <div style={{ ...rw, marginBottom: 6, padding: "2px 0" }}>
        <span style={{ fontSize: 12, fontWeight: 600 }}>7T Ribbon Designer</span>
        {isModified && <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 6 }}>● modified</span>}
        <button onClick={() => setPlaying(!playing)} style={{ ...btnStyle, marginLeft: 8, border: `1px solid ${playing ? "var(--color-border-success)" : "var(--color-border-warning)"}`, color: playing ? "var(--color-text-success)" : "var(--color-text-warning)" }}>{playing ? "⏸ Pause" : "▶ Play"}</button>
        <div style={{ marginLeft: "auto", display: "flex", gap: 4 }}>
          <button onClick={() => { setDockKey(k => k + 1); setPanelOrder(defaultOrder); }} style={btnStyle}>Dock all</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
        </div>
      </div>

      <div style={{ display: "flex", gap: 3, marginBottom: 6 }}>{TIER_NAMES.map((name, i) => (<button key={i} onClick={() => setTierIdx(i)} style={{ fontSize: 10, padding: "3px 8px", borderRadius: 4, cursor: "pointer", border: i === tierIdx ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)", background: i === tierIdx ? "var(--color-background-info)" : "var(--color-background-secondary)", color: i === tierIdx ? "var(--color-text-info)" : "var(--color-text-secondary)", fontWeight: i === tierIdx ? 600 : 400 }}>{name}</button>))}</div>

      <div style={{ display: "flex", gap: 6, marginBottom: 6 }}>
        <div ref={c3dRef} style={{ flex: 2, position: "relative", border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "#0a0a0e", resize: "vertical", minHeight: 180, height: 420 }}><canvas ref={cv3dRef} onPointerDown={onPointerDown3D} onContextMenu={e => e.preventDefault()} style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }} /><div onClick={() => { setZoom(1); setPanX(0); setPanY(0); }} title="Click to reset view" style={{ position: "absolute", top: 4, right: 8, fontSize: 9, color: "rgba(255,255,255,0.4)", cursor: "pointer", userSelect: "none", padding: "2px 4px" }}>{Math.round(zoom * 100)}%</div><div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9, color: "rgba(255,255,255,0.3)" }}>drag to rotate · right-drag to pan · scroll to zoom · resize ↘</div></div>
        <div ref={c2dRef} style={{ flex: 1, minWidth: 160, border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", resize: "vertical", minHeight: 180, height: 420 }}><canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} /></div>
      </div>

      {panelOrder.map(pid => {
        const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };
        switch (pid) {
        case "geometry": return (<DragPanel {...dp} title="Geometry" ini={true}>
          <div style={ms}><span style={lb}>Cubes μ</span><Num value={T.cubes} min={20} max={500} step={1} w={44} onChange={v => upd(n => { n.cubes = Math.round(v); })} /><span style={sLb}>σ</span><Num value={T.cubes_s} min={0} max={100} step={1} w={38} onChange={v => upd(n => { n.cubes_s = v; })} /></div>
          <div style={ms}><span style={lb}>Cube size μ</span><Num value={T.cube_size} min={0.5} max={8} w={44} onChange={v => upd(n => { n.cube_size = v; })} /><span style={sLb}>σ</span><Num value={T.cube_size_s} min={0} max={3} w={38} onChange={v => upd(n => { n.cube_size_s = v; })} /></div>
          <div style={ms}><span style={lb}>Height μ</span><Num value={T.height} min={10} max={150} w={44} onChange={v => upd(n => { n.height = v; })} /><span style={sLb}>σ</span><Num value={T.height_s} min={0} max={30} w={38} onChange={v => upd(n => { n.height_s = v; })} /></div>
        </DragPanel>);
        case "lateral": return (<DragPanel {...dp} title="Lateral Wave (fundamental)">
          <div style={ms}><span style={lb}>Amplitude μ</span><Num value={T.lat_amp} min={0} max={20} w={44} onChange={v => upd(n => { n.lat_amp = v; })} /><span style={sLb}>σ</span><Num value={T.lat_amp_s} min={0} max={5} w={38} onChange={v => upd(n => { n.lat_amp_s = v; })} /></div>
          <div style={ms}><span style={lb}>Cycles μ</span><Num value={T.lat_cyc} min={0.1} max={8} w={44} onChange={v => upd(n => { n.lat_cyc = v; })} /><span style={sLb}>σ</span><Num value={T.lat_cyc_s} min={0} max={2} w={38} onChange={v => upd(n => { n.lat_cyc_s = v; })} /></div>
          <div style={ms}><span style={lb}>Speed μ</span><Num value={T.lat_spd} min={0} max={2} w={44} onChange={v => upd(n => { n.lat_spd = v; })} /><span style={sLb}>σ</span><Num value={T.lat_spd_s} min={0} max={0.5} w={38} onChange={v => upd(n => { n.lat_spd_s = v; })} /></div>
        </DragPanel>);
        case "vertical": return (<DragPanel {...dp} title="Vertical Wave (breathing)">
          <div style={ms}><span style={lb}>Amplitude μ</span><Num value={T.vert_amp} min={0} max={15} w={44} onChange={v => upd(n => { n.vert_amp = v; })} /><span style={sLb}>σ</span><Num value={T.vert_amp_s} min={0} max={3} w={38} onChange={v => upd(n => { n.vert_amp_s = v; })} /></div>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "4px 0 3px" }}>Harmonic ratio (cycles = lateral × ratio)</div>
          <RatioButtons ratios={VERTICAL_RATIOS} value={T.vert_ratio_idx} onChange={v => upd(n => { n.vert_ratio_idx = v; })} />
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 3 }}>Current: vert cycles = {T.lat_cyc.toFixed(1)} × {VERTICAL_RATIOS[T.vert_ratio_idx].ratio.toFixed(3)} = {(T.lat_cyc * VERTICAL_RATIOS[T.vert_ratio_idx].ratio).toFixed(2)}</div>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-tertiary)", margin: "6px 0 3px", fontStyle: "italic" }}>Legacy (pre-override seeds for Gaussian draw)</div>
          <div style={ms}><span style={lb}>Cycles μ</span><Num value={T.vert_cyc} min={0.1} max={8} w={44} onChange={v => upd(n => { n.vert_cyc = v; })} /><span style={sLb}>σ</span><Num value={T.vert_cyc_s} min={0} max={2} w={38} onChange={v => upd(n => { n.vert_cyc_s = v; })} /></div>
          <div style={ms}><span style={lb}>Speed μ</span><Num value={T.vert_spd} min={0} max={2} w={44} onChange={v => upd(n => { n.vert_spd = v; })} /><span style={sLb}>σ</span><Num value={T.vert_spd_s} min={0} max={0.5} w={38} onChange={v => upd(n => { n.vert_spd_s = v; })} /></div>
        </DragPanel>);
        case "twist": return (<DragPanel {...dp} title="Twist Wave (corkscrew)">
          <div style={ms}><span style={lb}>Amplitude μ</span><Num value={T.twist_amp} min={0} max={12} w={44} onChange={v => upd(n => { n.twist_amp = v; })} /><span style={sLb}>σ</span><Num value={T.twist_amp_s} min={0} max={3} w={38} onChange={v => upd(n => { n.twist_amp_s = v; })} /></div>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "4px 0 3px" }}>Harmonic ratio (cycles = lateral × ratio)</div>
          <RatioButtons ratios={TWIST_RATIOS} value={T.twist_ratio_idx} onChange={v => upd(n => { n.twist_ratio_idx = v; })} />
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 3 }}>Current: twist cycles = {T.lat_cyc.toFixed(1)} × {TWIST_RATIOS[T.twist_ratio_idx].ratio.toFixed(3)} = {(T.lat_cyc * TWIST_RATIOS[T.twist_ratio_idx].ratio).toFixed(2)}</div>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-tertiary)", margin: "6px 0 3px", fontStyle: "italic" }}>Legacy (pre-override seeds for Gaussian draw)</div>
          <div style={ms}><span style={lb}>Cycles μ</span><Num value={T.twist_cyc} min={0.1} max={8} w={44} onChange={v => upd(n => { n.twist_cyc = v; })} /><span style={sLb}>σ</span><Num value={T.twist_cyc_s} min={0} max={2} w={38} onChange={v => upd(n => { n.twist_cyc_s = v; })} /></div>
          <div style={ms}><span style={lb}>Speed μ</span><Num value={T.twist_spd} min={0} max={2} w={44} onChange={v => upd(n => { n.twist_spd = v; })} /><span style={sLb}>σ</span><Num value={T.twist_spd_s} min={0} max={0.5} w={38} onChange={v => upd(n => { n.twist_spd_s = v; })} /></div>
        </DragPanel>);
        case "appearance": return (<DragPanel {...dp} title="Appearance">
          <div style={{ marginBottom: 4 }}>
            <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginBottom: 2 }}>Smooth palette (click to set color)</div>
            <div style={{ display: "flex", gap: 3, flexWrap: "wrap", marginBottom: 4 }}>
              {RIBBON_SMOOTH_PALETTE.map((p, i) => (
                <button key={i} onClick={() => upd(n => { n.color = p.c.slice(); })} style={{ ...btnStyle, fontSize: 9, display: "flex", alignItems: "center", gap: 3 }}>
                  <span style={{ width: 10, height: 10, borderRadius: 2, background: rgb01(...p.c), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
                  {p.name}
                </button>
              ))}
            </div>
          </div>
          <div style={rw}><span style={lb}>Color</span>{["R","G","B"].map((ch, i) => <><span key={"l"+i} style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num key={i} value={T.color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.color[i] = v; })} /></>)}<span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(T.color[0], T.color[1], T.color[2]), border: "1px solid var(--color-border-tertiary)" }} /></div>
          <div style={rw}><span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v => upd(n => { n.weight = v; })} /></div>
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 4 }}>Color mode at spawn: {RIBBON_COLOR_MODES.map(m => `${m.name} ${(m.weight*100).toFixed(0)}%`).join(", ")}. Smooth uses this palette; Tinted/Contrast are hash-derived.</div>
        </DragPanel>);
        case "export": return (<DragPanel {...dp} title="C++ Export">
          <div style={{ display: "flex", gap: 4, marginBottom: 4 }}>
            <button onClick={() => { copyText(cppCode); const btn = document.activeElement; if (btn) { const orig = btn.textContent; btn.textContent = "Copied!"; setTimeout(() => { btn.textContent = orig; }, 1200); } }} style={btnStyle}>Copy C++</button>
            <button onClick={() => { const s = JSON.stringify({ tierIdx, tier: TIER_NAMES[tierIdx], vertRatio: VERTICAL_RATIOS[T.vert_ratio_idx].name, twistRatio: TWIST_RATIOS[T.twist_ratio_idx].name, ...T }, null, 2); copyText(s); const btn = document.activeElement; if (btn) { const orig = btn.textContent; btn.textContent = "Copied!"; setTimeout(() => { btn.textContent = orig; }, 1200); } }} style={btnStyle}>Copy JSON</button>
            <button onClick={() => setShowCode(!showCode)} style={btnStyle}>{showCode ? "Hide" : "Show"}</button>
          </div>
          {showCode && <textarea readOnly value={cppCode} style={{ width: "100%", height: 50, fontSize: 10, fontFamily: "'JetBrains Mono', monospace", background: "var(--color-background-primary)", color: "var(--color-text-primary)", border: "1px solid var(--color-border-tertiary)", borderRadius: 4, padding: 6, resize: "vertical", lineHeight: 1.4, whiteSpace: "pre" }} />}
          <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 3 }}>C++ line matches RIBBON_TIERS[] field order. All 25 values.</div>
        </DragPanel>);
        default: return null;
        }
      })}
    </div>
  );
}