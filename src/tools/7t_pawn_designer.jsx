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
   7T PAWN DESIGNER — Color gradient & profile geometry workbench
   
   Designs a pawn_profile_color(t) function that maps height parameter
   to RGB, extending the current flat COLOR_PAWN in world.wgsl.
   Also allows profile geometry tweaks (section boundaries + radii).
   ═══════════════════════════════════════════════════════════════════════ */

/* ═══ PROFILE (port from world.wgsl pawn_profile_radius) ═══ */
function pawnRadius(t, P) {
  if (t < P.flare_t) return lerp(P.start_r, P.flare_r, t / P.flare_t);
  if (t < P.peak_t)  { const u = (t - P.flare_t) / (P.peak_t - P.flare_t); return lerp(P.flare_r, P.peak_r, Math.sin(u * Math.PI * 0.5)); }
  if (t < P.base_t)  { const u = (t - P.peak_t) / (P.base_t - P.peak_t); return lerp(P.peak_r, P.body_start_r, u * u); }
  if (t < P.body_t)  { const u = (t - P.base_t) / (P.body_t - P.base_t); const e = u * u * (3 - 2 * u); return lerp(P.body_start_r, P.waist_r, e); }
  if (t < P.neck_t)  { const u = (t - P.body_t) / (P.neck_t - P.body_t); return lerp(P.waist_r, P.neck_r, u); }
  if (t < P.collar_t){ const u = (t - P.neck_t) / (P.collar_t - P.neck_t); return P.neck_r + P.collar_bulge * Math.sin(u * Math.PI); }
  if (t < P.head_t)  { const u = (t - P.collar_t) / (P.head_t - P.collar_t); const y = u * 2 - 1; const sr = Math.sqrt(Math.max(0, 1 - y * y)); return P.head_base_r + P.head_sphere_r * sr; }
  const u = (t - P.head_t) / (1 - P.head_t); return lerp(P.head_base_r, 0, u * u * (3 - 2 * u));
}

function lerp(a, b, t) { return a + (b - a) * t; }
function clamp01(v) { return Math.max(0, Math.min(1, v)); }
function lerpC(a, b, t) { return [lerp(a[0],b[0],t), lerp(a[1],b[1],t), lerp(a[2],b[2],t)]; }

/* ═══ COLOR GRADIENT ═══ */
// Evaluate gradient at height t. Stops are sorted by t position.
function evalGradient(t, stops) {
  if (stops.length === 0) return [0.8, 0.5, 0.8];
  if (t <= stops[0].t) return stops[0].c;
  if (t >= stops[stops.length - 1].t) return stops[stops.length - 1].c;
  for (let i = 0; i < stops.length - 1; i++) {
    if (t >= stops[i].t && t <= stops[i + 1].t) {
      const u = (t - stops[i].t) / (stops[i + 1].t - stops[i].t);
      return lerpC(stops[i].c, stops[i + 1].c, u);
    }
  }
  return stops[stops.length - 1].c;
}

function rgb01(r, g, b) {
  return `rgb(${Math.round(clamp01(r)*255)},${Math.round(clamp01(g)*255)},${Math.round(clamp01(b)*255)})`;
}
function hex2rgb(h) {
  h = h.replace('#', '');
  return [parseInt(h.slice(0,2),16)/255, parseInt(h.slice(2,4),16)/255, parseInt(h.slice(4,6),16)/255];
}
function rgb2hex(c) {
  return '#' + c.map(v => Math.round(clamp01(v)*255).toString(16).padStart(2,'0')).join('');
}

/* ═══ INITIAL STATE ═══ */
const SECTION_NAMES = ["Base", "Body", "Neck", "Collar", "Head", "Tip"];
const SECTION_KEYS  = ["base","body","neck","collar","head","tip"];
const TIER_NAMES    = ["Pawn", "Squat", "Colossal", "Acorn", "Spire", "Idol", "Stele"];

/* Tier 0 — classic pawn (current default). */
function initProfile() {
  return {
    start_r: 0.75, flare_r: 0.88, flare_t: 0.04,
    peak_r: 0.92, peak_t: 0.10,
    base_t: 0.14, body_start_r: 0.70,
    body_t: 0.50, waist_r: 0.22,
    neck_t: 0.62, neck_r: 0.18,
    collar_t: 0.70, collar_bulge: 0.08,
    head_t: 0.95, head_base_r: 0.15, head_sphere_r: 0.35,
    height: 1.5, radius: 0.5,
  };
}

function initStops() {
  return [
    { t: 0.00, c: [0.80, 0.50, 0.80], label: "Base" },
    { t: 0.14, c: [0.80, 0.50, 0.80], label: "Body" },
    { t: 0.50, c: [0.80, 0.50, 0.80], label: "Neck" },
    { t: 0.62, c: [0.80, 0.50, 0.80], label: "Collar" },
    { t: 0.70, c: [0.80, 0.50, 0.80], label: "Head" },
    { t: 0.95, c: [0.80, 0.50, 0.80], label: "Tip" },
  ];
}

/* Tier 1 — Squat: stout, mushroom-like sentinel. Short, wide, flatter head. */
function squatProfile() {
  return {
    start_r: 0.85, flare_r: 0.95, flare_t: 0.05,
    peak_r: 1.00, peak_t: 0.12,
    base_t: 0.18, body_start_r: 0.85,
    body_t: 0.55, waist_r: 0.42,
    neck_t: 0.65, neck_r: 0.32,
    collar_t: 0.72, collar_bulge: 0.04,
    head_t: 0.92, head_base_r: 0.30, head_sphere_r: 0.20,
    height: 0.9, radius: 0.65,
  };
}
function squatStops() {
  return [
    { t: 0.00, c: [0.55, 0.42, 0.30], label: "Base" },
    { t: 0.18, c: [0.68, 0.55, 0.40], label: "Body" },
    { t: 0.55, c: [0.78, 0.66, 0.50], label: "Neck" },
    { t: 0.65, c: [0.82, 0.71, 0.55], label: "Collar" },
    { t: 0.72, c: [0.85, 0.74, 0.58], label: "Head" },
    { t: 0.92, c: [0.92, 0.84, 0.68], label: "Tip" },
  ];
}

/* Tier 2 — Colossal: monumental, towering. Sharper waist, prominent collar, large dome head. */
function colossalProfile() {
  return {
    start_r: 0.78, flare_r: 0.92, flare_t: 0.05,
    peak_r: 0.98, peak_t: 0.12,
    base_t: 0.16, body_start_r: 0.72,
    body_t: 0.48, waist_r: 0.16,
    neck_t: 0.60, neck_r: 0.14,
    collar_t: 0.68, collar_bulge: 0.13,
    head_t: 0.96, head_base_r: 0.18, head_sphere_r: 0.42,
    height: 3.5, radius: 0.75,
  };
}
function colossalStops() {
  return [
    { t: 0.00, c: [0.10, 0.09, 0.09], label: "Base" },
    { t: 0.16, c: [0.13, 0.11, 0.11], label: "Body" },
    { t: 0.48, c: [0.18, 0.15, 0.13], label: "Neck" },
    { t: 0.60, c: [0.55, 0.42, 0.16], label: "Collar" },
    { t: 0.68, c: [0.78, 0.62, 0.22], label: "Head" },
    { t: 0.96, c: [0.92, 0.80, 0.38], label: "Tip" },
  ];
}

/* Tier 3 — Acorn: small oval marker. Compact body, modest cap, earthy brown. */
function acornProfile() {
  return {
    start_r: 0.75, flare_r: 0.85, flare_t: 0.05,
    peak_r: 0.92, peak_t: 0.18,
    base_t: 0.28, body_start_r: 0.85,
    body_t: 0.55, waist_r: 0.55,
    neck_t: 0.65, neck_r: 0.40,
    collar_t: 0.72, collar_bulge: 0.05,
    head_t: 0.88, head_base_r: 0.35, head_sphere_r: 0.20,
    height: 0.7, radius: 0.45,
  };
}
function acornStops() {
  return [
    { t: 0.00, c: [0.30, 0.18, 0.08], label: "Base" },
    { t: 0.28, c: [0.48, 0.30, 0.16], label: "Body" },
    { t: 0.55, c: [0.55, 0.36, 0.20], label: "Neck" },
    { t: 0.65, c: [0.62, 0.42, 0.24], label: "Collar" },
    { t: 0.72, c: [0.42, 0.28, 0.14], label: "Head" },
    { t: 0.88, c: [0.22, 0.13, 0.06], label: "Tip" },
  ];
}

/* Tier 4 — Spire: tall and narrow, near-needle. Cool steel-blue palette. */
function spireProfile() {
  return {
    start_r: 0.50, flare_r: 0.55, flare_t: 0.05,
    peak_r: 0.60, peak_t: 0.15,
    base_t: 0.20, body_start_r: 0.50,
    body_t: 0.65, waist_r: 0.30,
    neck_t: 0.75, neck_r: 0.20,
    collar_t: 0.82, collar_bulge: 0.05,
    head_t: 0.92, head_base_r: 0.18, head_sphere_r: 0.15,
    height: 2.8, radius: 0.35,
  };
}
function spireStops() {
  return [
    { t: 0.00, c: [0.18, 0.20, 0.25], label: "Base" },
    { t: 0.20, c: [0.30, 0.35, 0.42], label: "Body" },
    { t: 0.65, c: [0.45, 0.52, 0.60], label: "Neck" },
    { t: 0.75, c: [0.55, 0.62, 0.70], label: "Collar" },
    { t: 0.82, c: [0.62, 0.70, 0.78], label: "Head" },
    { t: 0.92, c: [0.78, 0.85, 0.92], label: "Tip" },
  ];
}

/* Tier 5 — Idol: head-heavy ornate. Pronounced collar bulge + dominant head sphere, jade. */
function idolProfile() {
  return {
    start_r: 0.50, flare_r: 0.62, flare_t: 0.05,
    peak_r: 0.68, peak_t: 0.10,
    base_t: 0.16, body_start_r: 0.55,
    body_t: 0.45, waist_r: 0.20,
    neck_t: 0.55, neck_r: 0.20,
    collar_t: 0.66, collar_bulge: 0.20,
    head_t: 0.92, head_base_r: 0.30, head_sphere_r: 0.55,
    height: 1.8, radius: 0.55,
  };
}
function idolStops() {
  return [
    { t: 0.00, c: [0.10, 0.22, 0.20], label: "Base" },
    { t: 0.16, c: [0.15, 0.32, 0.28], label: "Body" },
    { t: 0.45, c: [0.20, 0.45, 0.40], label: "Neck" },
    { t: 0.55, c: [0.30, 0.55, 0.50], label: "Collar" },
    { t: 0.66, c: [0.45, 0.70, 0.62], label: "Head" },
    { t: 0.92, c: [0.65, 0.85, 0.75], label: "Tip" },
  ];
}

/* Tier 6 — Stele: austere vertical marker, near-cylindrical. Limestone palette. */
function steleProfile() {
  return {
    start_r: 0.78, flare_r: 0.82, flare_t: 0.04,
    peak_r: 0.85, peak_t: 0.10,
    base_t: 0.14, body_start_r: 0.78,
    body_t: 0.60, waist_r: 0.55,
    neck_t: 0.72, neck_r: 0.38,
    collar_t: 0.80, collar_bulge: 0.03,
    head_t: 0.95, head_base_r: 0.35, head_sphere_r: 0.20,
    height: 2.0, radius: 0.40,
  };
}
function steleStops() {
  return [
    { t: 0.00, c: [0.55, 0.52, 0.48], label: "Base" },
    { t: 0.14, c: [0.62, 0.60, 0.55], label: "Body" },
    { t: 0.60, c: [0.72, 0.70, 0.66], label: "Neck" },
    { t: 0.72, c: [0.78, 0.76, 0.72], label: "Collar" },
    { t: 0.80, c: [0.82, 0.80, 0.76], label: "Head" },
    { t: 0.95, c: [0.88, 0.86, 0.82], label: "Tip" },
  ];
}

const TIER_PROFILE_FNS = [initProfile, squatProfile, colossalProfile, acornProfile, spireProfile, idolProfile, steleProfile];
const TIER_STOPS_FNS   = [initStops,   squatStops,   colossalStops,   acornStops,   spireStops,   idolStops,   steleStops];

function defaultPawnTier(idx) {
  return { prof: TIER_PROFILE_FNS[idx](), stops: TIER_STOPS_FNS[idx]() };
}
function defaultAllTiers() {
  return TIER_NAMES.map((_, i) => defaultPawnTier(i));
}

const PRESETS = {
  "Lavender (default)": () => initStops(),
  "Sandstone": () => [
    { t: 0.00, c: [0.72, 0.58, 0.42], label: "Base" },
    { t: 0.14, c: [0.78, 0.65, 0.50], label: "Body" },
    { t: 0.50, c: [0.82, 0.72, 0.58], label: "Neck" },
    { t: 0.62, c: [0.85, 0.75, 0.62], label: "Collar" },
    { t: 0.70, c: [0.90, 0.82, 0.70], label: "Head" },
    { t: 0.95, c: [0.95, 0.90, 0.82], label: "Tip" },
  ],
  "Obsidian & Gold": () => [
    { t: 0.00, c: [0.12, 0.10, 0.10], label: "Base" },
    { t: 0.14, c: [0.15, 0.12, 0.12], label: "Body" },
    { t: 0.50, c: [0.18, 0.15, 0.14], label: "Neck" },
    { t: 0.62, c: [0.65, 0.50, 0.18], label: "Collar" },
    { t: 0.70, c: [0.75, 0.60, 0.22], label: "Head" },
    { t: 0.95, c: [0.85, 0.72, 0.30], label: "Tip" },
  ],
  "Deep Ocean": () => [
    { t: 0.00, c: [0.08, 0.12, 0.25], label: "Base" },
    { t: 0.14, c: [0.10, 0.18, 0.40], label: "Body" },
    { t: 0.50, c: [0.15, 0.30, 0.55], label: "Neck" },
    { t: 0.62, c: [0.20, 0.42, 0.65], label: "Collar" },
    { t: 0.70, c: [0.35, 0.60, 0.78], label: "Head" },
    { t: 0.95, c: [0.65, 0.82, 0.92], label: "Tip" },
  ],
  "Ember": () => [
    { t: 0.00, c: [0.30, 0.05, 0.02], label: "Base" },
    { t: 0.14, c: [0.55, 0.10, 0.03], label: "Body" },
    { t: 0.50, c: [0.80, 0.25, 0.05], label: "Neck" },
    { t: 0.62, c: [0.90, 0.45, 0.08], label: "Collar" },
    { t: 0.70, c: [0.95, 0.65, 0.15], label: "Head" },
    { t: 0.95, c: [1.00, 0.90, 0.50], label: "Tip" },
  ],
  "Marble": () => [
    { t: 0.00, c: [0.82, 0.80, 0.78], label: "Base" },
    { t: 0.14, c: [0.88, 0.86, 0.83], label: "Body" },
    { t: 0.50, c: [0.90, 0.88, 0.85], label: "Neck" },
    { t: 0.62, c: [0.85, 0.83, 0.80], label: "Collar" },
    { t: 0.70, c: [0.92, 0.90, 0.88], label: "Head" },
    { t: 0.95, c: [0.95, 0.94, 0.92], label: "Tip" },
  ],
  "Patina Bronze": () => [
    { t: 0.00, c: [0.22, 0.35, 0.28], label: "Base" },
    { t: 0.14, c: [0.30, 0.42, 0.32], label: "Body" },
    { t: 0.50, c: [0.45, 0.38, 0.22], label: "Neck" },
    { t: 0.62, c: [0.55, 0.42, 0.20], label: "Collar" },
    { t: 0.70, c: [0.65, 0.50, 0.25], label: "Head" },
    { t: 0.95, c: [0.38, 0.50, 0.35], label: "Tip" },
  ],
};

/* ═══ 3D RENDERER — Painter's algorithm, diffuse + ambient ═══ */
const RENDER_SEGS = 32;
const RENDER_RINGS = 28;
const PAWN_LIGHT_DIR = [-0.6, -0.7, -0.3];

/* Build a vertex transform: world-space offset (xOff, yOff), then Y rotation, then X tilt.
   Used both for single pawns (xOff=yOff=0) and for the comparison scene (each pawn placed
   at its own world offset so the whole scene rotates as one unit). */
function makeTransform(rotY, tilt, xOff = 0, yOff = 0) {
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);
  return (v) => {
    const x0 = v[0] + xOff, y0 = v[1] + yOff, z0 = v[2];
    const x1 = x0 * cosR + z0 * sinR;
    const z1 = -x0 * sinR + z0 * cosR;
    const y2 = y0 * cosT - z1 * sinT;
    const z2 = y0 * sinT + z1 * cosT;
    return [x1, y2, z2];
  };
}

/* Generate the face list for one pawn — surface revolution + bottom cap.
   `transformVert` is applied to every local vertex, so callers control rotation
   and world placement. Faces include lighting and gradient colour, ready for sort+draw.
   No projection here — caller projects into screen space. */
function buildPawnFaces(prof, stops, transformVert) {
  const faces = [];
  const lightDir = normalize3(PAWN_LIGHT_DIR);

  // Surface revolution
  for (let r = 0; r < RENDER_RINGS - 1; r++) {
    for (let s = 0; s < RENDER_SEGS; s++) {
      const t0 = r / (RENDER_RINGS - 1), t1 = (r + 1) / (RENDER_RINGS - 1);
      const a0 = (s / RENDER_SEGS) * Math.PI * 2, a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
      const rad0 = pawnRadius(t0, prof) * prof.radius;
      const rad1 = pawnRadius(t1, prof) * prof.radius;
      const y0 = t0 * prof.height - prof.height * 0.45;
      const y1 = t1 * prof.height - prof.height * 0.45;
      const ca0 = Math.cos(a0), sa0 = Math.sin(a0), ca1 = Math.cos(a1), sa1 = Math.sin(a1);
      const verts = [
        transformVert([ca0*rad0, y0, sa0*rad0]),
        transformVert([ca1*rad0, y0, sa1*rad0]),
        transformVert([ca1*rad1, y1, sa1*rad1]),
        transformVert([ca0*rad1, y1, sa0*rad1]),
      ];
      const e1 = sub3(verts[1], verts[0]), e2 = sub3(verts[3], verts[0]);
      const n = normalize3(cross3(e1, e2));
      if (n[2] > 0.02) continue; // backface cull
      const diff = Math.max(0, n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
      const light = 0.28 + 0.72 * diff;
      const tMid = (t0 + t1) / 2;
      const col = evalGradient(tMid, stops);
      const avgZ = (verts[0][2] + verts[1][2] + verts[2][2] + verts[3][2]) / 4;
      faces.push({ verts, light, col, avgZ });
    }
  }

  // Bottom cap
  const capR = pawnRadius(0, prof) * prof.radius;
  const capY = -prof.height * 0.45;
  const capCol = evalGradient(0, stops);
  for (let s = 0; s < RENDER_SEGS; s++) {
    const a0 = (s / RENDER_SEGS) * Math.PI * 2;
    const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
    const verts = [
      transformVert([0, capY, 0]),
      transformVert([Math.cos(a1)*capR, capY, Math.sin(a1)*capR]),
      transformVert([Math.cos(a0)*capR, capY, Math.sin(a0)*capR]),
    ];
    const ce1 = sub3(verts[1], verts[0]), ce2 = sub3(verts[2], verts[0]);
    const n = normalize3(cross3(ce1, ce2));
    if (n[2] > 0.02) continue;
    const diff = Math.max(0, n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
    const light = 0.28 + 0.72 * diff;
    const avgZ = (verts[0][2] + verts[1][2] + verts[2][2]) / 3;
    faces.push({ verts, light, col: capCol, avgZ, tri: true });
  }

  return faces;
}

/* Project a face list into screen space and draw it (painter's algorithm). */
function drawFaces(ctx, faces, project) {
  faces.sort((a, b) => a.avgZ - b.avgZ);
  for (const f of faces) {
    const pts = f.verts.map(project);
    ctx.fillStyle = rgb01(f.col[0]*f.light, f.col[1]*f.light, f.col[2]*f.light);
    ctx.beginPath();
    ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath();
    ctx.fill();
  }
}

/* Single-pawn 3D: auto-fits to canvas, then user zoom multiplies on top.
   Pan offsets shift the projection in screen pixels (applied after auto-fit). */
function render3D(ctx, W, H, prof, stops, rotY, tilt, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);
  const scaleY = H * 0.78 / Math.max(prof.height, 0.0001);
  const scaleX = W * 0.85 / Math.max(prof.radius * 2, 0.0001);
  const scale  = Math.min(scaleY, scaleX) * zoom;
  const cy = H * 0.52;
  const transformVert = makeTransform(rotY, tilt, 0, 0);
  const faces = buildPawnFaces(prof, stops, transformVert);
  const project = (v) => [W / 2 + panX + v[0] * scale, cy + panY - v[1] * scale];
  drawFaces(ctx, faces, project);
}

/* Comparison 3D: all tiers in a single shared scene. Bases aligned on world y=0,
   pawns spaced along world x, the whole scene rotates as one unit. Faces from all
   pawns are merged into a single depth-sorted list so they occlude each other
   correctly when the scene is rotated. */
function render3DCompare(ctx, W, H, tiers, rotY, tilt, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);

  // Each tier's slot is a hair wider than its silhouette so neighbours don't kiss.
  const slotWidths = tiers.map(t => t.prof.radius * 2 * 1.25);
  const totalW = slotWidths.reduce((a, b) => a + b, 0);
  const maxH   = Math.max(...tiers.map(t => t.prof.height));

  // Shared scale that fits the whole row at default rotation.
  // World y=0 is the shared base line; world x=0 is the row centre.
  const scaleY = H * 0.82 / Math.max(maxH, 0.0001);
  const scaleX = W * 0.92 / Math.max(totalW, 0.0001);
  const scale  = Math.min(scaleY, scaleX) * zoom;
  const cy = H * 0.85; // baseline near the bottom of the canvas

  // Cumulative slot edges → centred world x for each pawn.
  let cumX = -totalW / 2;
  const xOffsets = slotWidths.map(sw => { const c = cumX + sw / 2; cumX += sw; return c; });
  // Local geometry has y from -0.45h to +0.55h; shift up by 0.45h so each base sits at y=0.
  const yOffsets = tiers.map(t => 0.45 * t.prof.height);

  // Collect every face from every pawn, all in the same world space.
  const allFaces = [];
  tiers.forEach((tier, i) => {
    const transformVert = makeTransform(rotY, tilt, xOffsets[i], yOffsets[i]);
    const faces = buildPawnFaces(tier.prof, tier.stops, transformVert);
    for (const f of faces) allFaces.push(f);
  });

  const project = (v) => [W / 2 + panX + v[0] * scale, cy + panY - v[1] * scale];
  drawFaces(ctx, allFaces, project);
}

function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }

/* ═══ 2D PROFILE CROSS-SECTION ═══ */
function render2D(ctx, W, H, prof, stops) {
  ctx.clearRect(0, 0, W, H);
  const mg = { l: 30, r: 10, t: 10, b: 20 };
  const pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  const maxR = 1.05;

  // Background
  ctx.fillStyle = "var(--color-background-secondary)";
  ctx.fillRect(0, 0, W, H);

  // Draw filled silhouette with gradient
  const steps = 200;
  for (let i = 0; i < steps; i++) {
    const t0 = i / steps, t1 = (i + 1) / steps;
    const r0 = pawnRadius(t0, prof), r1 = pawnRadius(t1, prof);
    const col = evalGradient((t0 + t1) / 2, stops);
    const x0l = mg.l + pw / 2 - (r0 / maxR) * pw / 2;
    const x0r = mg.l + pw / 2 + (r0 / maxR) * pw / 2;
    const x1l = mg.l + pw / 2 - (r1 / maxR) * pw / 2;
    const x1r = mg.l + pw / 2 + (r1 / maxR) * pw / 2;
    const y0 = mg.t + ph - t0 * ph, y1 = mg.t + ph - t1 * ph;
    ctx.fillStyle = rgb01(col[0] * 0.85, col[1] * 0.85, col[2] * 0.85);
    ctx.beginPath();
    ctx.moveTo(x0l, y0); ctx.lineTo(x0r, y0); ctx.lineTo(x1r, y1); ctx.lineTo(x1l, y1);
    ctx.closePath(); ctx.fill();
  }

  // Lit side (right half gets more light)
  for (let i = 0; i < steps; i++) {
    const t0 = i / steps, t1 = (i + 1) / steps;
    const r0 = pawnRadius(t0, prof), r1 = pawnRadius(t1, prof);
    const col = evalGradient((t0 + t1) / 2, stops);
    const cx = mg.l + pw / 2;
    const x0r = cx + (r0 / maxR) * pw / 2 * 0.85;
    const x1r = cx + (r1 / maxR) * pw / 2 * 0.85;
    const y0 = mg.t + ph - t0 * ph, y1 = mg.t + ph - t1 * ph;
    ctx.fillStyle = rgb01(col[0], col[1], col[2]);
    ctx.globalAlpha = 0.5;
    ctx.beginPath();
    ctx.moveTo(cx, y0); ctx.lineTo(x0r, y0); ctx.lineTo(x1r, y1); ctx.lineTo(cx, y1);
    ctx.closePath(); ctx.fill();
    ctx.globalAlpha = 1;
  }

  // Section boundary lines
  const bounds = [
    { t: prof.base_t, label: "Base" },
    { t: prof.body_t, label: "Body" },
    { t: prof.neck_t, label: "Neck" },
    { t: prof.collar_t, label: "Collar" },
    { t: prof.head_t, label: "Head" },
  ];
  ctx.setLineDash([3, 3]);
  ctx.lineWidth = 0.5;
  for (const b of bounds) {
    const y = mg.t + ph - b.t * ph;
    ctx.strokeStyle = "var(--color-text-tertiary)";
    ctx.beginPath(); ctx.moveTo(mg.l, y); ctx.lineTo(W - mg.r, y); ctx.stroke();
    ctx.fillStyle = "var(--color-text-tertiary)";
    ctx.font = "9px monospace";
    ctx.textAlign = "right";
    ctx.fillText(b.t.toFixed(2), mg.l - 3, y + 3);
  }
  ctx.setLineDash([]);

  // Profile outline
  ctx.strokeStyle = "var(--color-text-primary)";
  ctx.lineWidth = 1.2;
  ctx.beginPath();
  for (let i = 0; i <= steps; i++) {
    const t = i / steps;
    const r = pawnRadius(t, prof);
    const x = mg.l + pw / 2 + (r / maxR) * pw / 2;
    const y = mg.t + ph - t * ph;
    i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
  }
  ctx.stroke();
  ctx.beginPath();
  for (let i = 0; i <= steps; i++) {
    const t = i / steps;
    const r = pawnRadius(t, prof);
    const x = mg.l + pw / 2 - (r / maxR) * pw / 2;
    const y = mg.t + ph - t * ph;
    i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y);
  }
  ctx.stroke();

  // Axis labels
  ctx.fillStyle = "var(--color-text-tertiary)";
  ctx.font = "9px monospace";
  ctx.textAlign = "right";
  ctx.fillText("1.0", mg.l - 3, mg.t + 5);
  ctx.fillText("0.0", mg.l - 3, mg.t + ph + 4);
}

/* All-tiers 2D: shared scale, bases aligned on the same baseline — mirrors the 3D scene
   so size relationships are read the same way in both views. */
function render2DCompare(ctx, W, H, tiers) {
  ctx.clearRect(0, 0, W, H);

  // World layout: same scheme as the 3D compare scene.
  const slotWidths = tiers.map(t => t.prof.radius * 2 * 1.25);
  const totalW = slotWidths.reduce((a, b) => a + b, 0);
  const maxH   = Math.max(...tiers.map(t => t.prof.height));

  const mg = { l: 8, r: 8, t: 14, b: 18 }; // top room for labels, bottom for h × r
  const pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  const scaleY = ph / Math.max(maxH, 0.0001);
  const scaleX = pw / Math.max(totalW, 0.0001);
  const scale  = Math.min(scaleY, scaleX);

  // Centre the row horizontally; baseline sits at the bottom of the plot area.
  const xCentre = mg.l + pw / 2;
  const baseY   = mg.t + ph;
  let cumX = -totalW / 2;
  const xOffsets = slotWidths.map(sw => { const c = cumX + sw / 2; cumX += sw; return c; });

  // Faint baseline so the eye can read aligned bases at a glance.
  ctx.strokeStyle = "rgba(255,255,255,0.10)";
  ctx.setLineDash([3, 3]);
  ctx.lineWidth = 1;
  ctx.beginPath(); ctx.moveTo(mg.l, baseY); ctx.lineTo(W - mg.r, baseY); ctx.stroke();
  ctx.setLineDash([]);

  // Each pawn: filled silhouette using its own gradient, then outline.
  const STEPS = 120;
  tiers.forEach((tier, i) => {
    const prof = tier.prof, stops = tier.stops;
    const cx = xCentre + xOffsets[i] * scale;
    const baseAtT = (t) => baseY - (t * prof.height) * scale;

    // Filled silhouette
    for (let k = 0; k < STEPS; k++) {
      const t0 = k / STEPS, t1 = (k + 1) / STEPS;
      const r0 = pawnRadius(t0, prof) * prof.radius;
      const r1 = pawnRadius(t1, prof) * prof.radius;
      const col = evalGradient((t0 + t1) / 2, stops);
      const y0 = baseAtT(t0), y1 = baseAtT(t1);
      ctx.fillStyle = rgb01(col[0] * 0.85, col[1] * 0.85, col[2] * 0.85);
      ctx.beginPath();
      ctx.moveTo(cx - r0 * scale, y0);
      ctx.lineTo(cx + r0 * scale, y0);
      ctx.lineTo(cx + r1 * scale, y1);
      ctx.lineTo(cx - r1 * scale, y1);
      ctx.closePath(); ctx.fill();
    }
    // Lit side (right half)
    for (let k = 0; k < STEPS; k++) {
      const t0 = k / STEPS, t1 = (k + 1) / STEPS;
      const r0 = pawnRadius(t0, prof) * prof.radius * 0.85;
      const r1 = pawnRadius(t1, prof) * prof.radius * 0.85;
      const col = evalGradient((t0 + t1) / 2, stops);
      const y0 = baseAtT(t0), y1 = baseAtT(t1);
      ctx.globalAlpha = 0.5;
      ctx.fillStyle = rgb01(col[0], col[1], col[2]);
      ctx.beginPath();
      ctx.moveTo(cx, y0); ctx.lineTo(cx + r0 * scale, y0);
      ctx.lineTo(cx + r1 * scale, y1); ctx.lineTo(cx, y1);
      ctx.closePath(); ctx.fill();
      ctx.globalAlpha = 1;
    }
    // Outline
    ctx.strokeStyle = "rgba(255,255,255,0.55)";
    ctx.lineWidth = 1;
    for (const sign of [+1, -1]) {
      ctx.beginPath();
      for (let k = 0; k <= STEPS; k++) {
        const t = k / STEPS;
        const r = pawnRadius(t, prof) * prof.radius;
        const x = cx + sign * r * scale, y = baseAtT(t);
        if (k === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }

    // Tier label above each pawn
    ctx.fillStyle = "rgba(255,255,255,0.7)";
    ctx.font = "bold 9px monospace";
    ctx.textAlign = "center";
    ctx.fillText(TIER_NAMES[i], cx, mg.t - 2);
    // Dimensions below
    ctx.fillStyle = "rgba(255,255,255,0.4)";
    ctx.font = "8px monospace";
    ctx.fillText(`${prof.height.toFixed(1)} × ${prof.radius.toFixed(2)}`, cx, H - 5);
  });
}

/* ═══ GRADIENT BAR ═══ */
function renderGradientBar(ctx, W, H, stops) {
  for (let x = 0; x < W; x++) {
    const t = x / W;
    const c = evalGradient(t, stops);
    ctx.fillStyle = rgb01(c[0], c[1], c[2]);
    ctx.fillRect(x, 0, 1, H);
  }
  // Stop markers
  for (const s of stops) {
    const x = s.t * W;
    ctx.strokeStyle = "var(--color-text-primary)";
    ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke();
    ctx.fillStyle = "var(--color-text-primary)";
    ctx.beginPath(); ctx.moveTo(x, H); ctx.lineTo(x - 3, H + 5); ctx.lineTo(x + 3, H + 5); ctx.closePath(); ctx.fill();
  }
}

/* ═══ WGSL CODE GENERATION ═══ */
function generateWGSL(prof, stops) {
  const f = (v) => v.toFixed(3);
  let code = `// ─── Pawn Profile Color (generated by 7t Pawn Designer) ───\n`;
  code += `// Replace entity_color = COLOR_PAWN with:\n`;
  code += `//   out.pawn_t = t;  (in pawn_vs, add @location(3) pawn_t: f32 to EntityVarying)\n`;
  code += `//   out.entity_color = pawn_profile_color(in.pawn_t);  (in pawn_fs)\n\n`;
  code += `fn pawn_profile_color(t: f32) -> vec3<f32> {\n`;
  for (let i = 0; i < stops.length - 1; i++) {
    const s0 = stops[i], s1 = stops[i + 1];
    const cond = i === 0 ? `    if (t < ${f(s1.t)})` : `    if (t < ${f(s1.t)})`;
    code += `${cond} {\n`;
    code += `        let u = saturate((t - ${f(s0.t)}) / ${f(s1.t - s0.t)});\n`;
    code += `        return mix(vec3(${f(s0.c[0])}, ${f(s0.c[1])}, ${f(s0.c[2])}),\n`;
    code += `                   vec3(${f(s1.c[0])}, ${f(s1.c[1])}, ${f(s1.c[2])}), u);\n`;
    code += `    }\n`;
  }
  const last = stops[stops.length - 1];
  code += `    return vec3(${f(last.c[0])}, ${f(last.c[1])}, ${f(last.c[2])});\n`;
  code += `}\n`;

  // Also generate modified profile if changed from defaults
  const def = initProfile();
  const changed = Object.keys(def).some(k => Math.abs(prof[k] - def[k]) > 0.001);
  if (changed) {
    code += `\n// ─── Modified Pawn Profile Radius ───\n`;
    code += `fn pawn_profile_radius(t: f32) -> f32 {\n`;
    code += `    if (t < ${f(prof.flare_t)}) { return mix(${f(prof.start_r)}, ${f(prof.flare_r)}, t / ${f(prof.flare_t)}); }\n`;
    code += `    if (t < ${f(prof.peak_t)}) { let u = (t - ${f(prof.flare_t)}) / ${f(prof.peak_t - prof.flare_t)}; return mix(${f(prof.flare_r)}, ${f(prof.peak_r)}, sin(u * PI * 0.5)); }\n`;
    code += `    if (t < ${f(prof.base_t)}) { let u = (t - ${f(prof.peak_t)}) / ${f(prof.base_t - prof.peak_t)}; return mix(${f(prof.peak_r)}, ${f(prof.body_start_r)}, u * u); }\n`;
    code += `    if (t < ${f(prof.body_t)}) { let u = (t - ${f(prof.base_t)}) / ${f(prof.body_t - prof.base_t)}; let e = smoothstep(0.0, 1.0, u); return mix(${f(prof.body_start_r)}, ${f(prof.waist_r)}, e); }\n`;
    code += `    if (t < ${f(prof.neck_t)}) { let u = (t - ${f(prof.body_t)}) / ${f(prof.neck_t - prof.body_t)}; return mix(${f(prof.waist_r)}, ${f(prof.neck_r)}, u); }\n`;
    code += `    if (t < ${f(prof.collar_t)}) { let u = (t - ${f(prof.neck_t)}) / ${f(prof.collar_t - prof.neck_t)}; return ${f(prof.neck_r)} + ${f(prof.collar_bulge)} * sin(u * PI); }\n`;
    code += `    if (t < ${f(prof.head_t)}) { let u = (t - ${f(prof.collar_t)}) / ${f(prof.head_t - prof.collar_t)}; let y = u * 2.0 - 1.0; let sr = sqrt(max(0.0, 1.0 - y * y)); return ${f(prof.head_base_r)} + ${f(prof.head_sphere_r)} * sr; }\n`;
    code += `    let u = (t - ${f(prof.head_t)}) / ${f(1 - prof.head_t)}; return mix(${f(prof.head_base_r)}, 0.0, smoothstep(0.0, 1.0, u));\n`;
    code += `}\n`;
  }

  return code;
}

/* ═══ UI PRIMITIVES (matching theme tool) ═══ */
const ist = { padding: "2px 3px", fontSize: 11, fontFamily: "monospace", borderRadius: 4, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)", textAlign: "right" };

function Num({ value, onChange, min = 0, max = 1, step = 0.01, w = 46 }) {
  const [txt, setTxt] = useState(String(Math.round(value * 10000) / 10000));
  const [slider, setSlider] = useState(false);
  useEffect(() => { setTxt(String(Math.round(value * 10000) / 10000)); }, [value]);
  const commit = () => { const v = parseFloat(txt); if (!isNaN(v)) onChange(Math.max(min, Math.min(max, v))); else setTxt(String(Math.round(value * 10000) / 10000)); };
  return (
    <span style={{ display: "inline-flex", alignItems: "center", gap: 2, position: "relative" }}>
      <span
        onClick={() => setSlider(s => !s)}
        style={{ cursor: "pointer", fontSize: 8, lineHeight: 1, color: slider ? "var(--color-text-info)" : "var(--color-text-tertiary)", userSelect: "none", opacity: slider ? 1 : 0.5, transition: "opacity .15s", width: 8, textAlign: "center" }}
        title="Toggle slider"
      >▸</span>
      {slider && <input type="range" min={min} max={max} step={step} value={value} style={{ width: 56, height: 12, margin: 0 }} onChange={e => onChange(+e.target.value)} />}
      <input type="text" value={txt} onChange={e => setTxt(e.target.value)} onBlur={commit} onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }} style={{ ...ist, width: w }} />
    </span>
  );
}

/* ═══ DRAGGABLE PANEL (synced with theme tool) ═══ */
function DragPanel({ title, children, ini = false, id, resetKey, onDock }) {
  const [open, setOpen] = useState(ini);
  const [pos, setPos] = useState(null);
  const [dragging, setDragging] = useState(false);
  const dragRef = useRef(null);
  const offsetRef = useRef({ x: 0, y: 0 });

  useEffect(() => { setPos(null); }, [resetKey]);

  const startDrag = useCallback(e => {
    if (e.target.closest("[data-notdrag]")) return;
    e.preventDefault();
    const rect = dragRef.current.getBoundingClientRect();
    offsetRef.current = { x: e.clientX - rect.left, y: e.clientY - rect.top };
    setDragging(true);
  }, []);

  useEffect(() => {
    if (!dragging) return;
    const onMove = e => {
      const parent = dragRef.current.parentElement.getBoundingClientRect();
      setPos({ x: e.clientX - parent.left - offsetRef.current.x, y: e.clientY - parent.top - offsetRef.current.y });
    };
    const onUp = () => setDragging(false);
    window.addEventListener("mousemove", onMove);
    window.addEventListener("mouseup", onUp);
    return () => { window.removeEventListener("mousemove", onMove); window.removeEventListener("mouseup", onUp); };
  }, [dragging]);

  const doDock = () => { setPos(null); if (onDock) onDock(id); };
  const style = pos ? { position: "absolute", left: pos.x, top: pos.y, zIndex: 100, maxWidth: 480, minWidth: 280 } : {};

  return (
    <div ref={dragRef} style={{ marginBottom: pos ? 0 : 5, border: pos ? "1px solid #666" : "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: pos ? "#2a2a30" : "var(--color-background-primary)", boxShadow: pos ? "0 8px 32px rgba(0,0,0,.6)" : "none", ...style }}>
      <div onMouseDown={startDrag} style={{ padding: "5px 10px", fontSize: 12, fontWeight: 500, userSelect: "none", display: "flex", alignItems: "center", gap: 5, color: pos ? "#e0ddd8" : "var(--color-text-primary)", background: pos ? "#353540" : "var(--color-background-secondary)", cursor: "grab" }}>
        <span data-notdrag="1" onClick={() => setOpen(!open)} style={{ cursor: "pointer", fontSize: 9, transition: "transform .15s", display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px" }}>▶</span>
        <span style={{ flex: 1 }}>{title}</span>
        {pos && <span data-notdrag="1" onClick={doDock} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}
      </div>
      {open && <div style={{ padding: "5px 10px", maxHeight: pos ? 400 : "none", overflowY: pos ? "auto" : "visible", color: pos ? "#d0cdc8" : undefined }}>{children}</div>}
    </div>
  );
}

/* ═══ COLOR STOP EDITOR ═══ */
function ColorStopRow({ stop, index, onChange }) {
  const colorRef = useRef(null);
  return (
    <div style={{ display: "flex", alignItems: "center", gap: 5, marginBottom: 3 }}>
      <span style={{ fontSize: 10, color: "var(--color-text-secondary)", minWidth: 38, fontWeight: 500 }}>{stop.label}</span>
      <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", minWidth: 28, textAlign: "right", fontFamily: "monospace" }}>{stop.t.toFixed(2)}</span>
      <div
        onClick={() => colorRef.current?.click()}
        style={{ width: 22, height: 16, borderRadius: 3, border: "1px solid var(--color-border-tertiary)", background: rgb01(stop.c[0], stop.c[1], stop.c[2]), cursor: "pointer", flexShrink: 0 }}
      />
      <input ref={colorRef} type="color" value={rgb2hex(stop.c)} onChange={e => onChange(index, hex2rgb(e.target.value))} style={{ position: "absolute", opacity: 0, width: 0, height: 0, pointerEvents: "none" }} />
      <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>R</span>
      <Num value={stop.c[0]} min={0} max={1} w={34} onChange={v => { const c = [...stop.c]; c[0] = v; onChange(index, c); }} />
      <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>G</span>
      <Num value={stop.c[1]} min={0} max={1} w={34} onChange={v => { const c = [...stop.c]; c[1] = v; onChange(index, c); }} />
      <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>B</span>
      <Num value={stop.c[2]} min={0} max={1} w={34} onChange={v => { const c = [...stop.c]; c[2] = v; onChange(index, c); }} />
    </div>
  );
}

const rw = { display: "flex", flexWrap: "wrap", gap: "3px 8px", alignItems: "center", marginBottom: 3 };
const lb = { fontSize: 10, color: "var(--color-text-secondary)", minWidth: 56 };

/* ═══ MAIN APP ═══ */
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

const STORAGE_KEY = "7t:pawn:tiers:v1";
const DEFAULTS_JSON = JSON.stringify(defaultAllTiers());

export default function PawnDesigner() {
  const [tiers, setTiers] = useState(defaultAllTiers);
  const [tierIdx, setTierIdx] = useState(0);
  const [loaded, setLoaded] = useState(false);
  const T = tiers[tierIdx];
  const prof = T.prof;
  const stops = T.stops;
  const isModified = JSON.stringify(tiers) !== DEFAULTS_JSON;
  const [rotY, setRotY] = useState(0.5);
  const [tilt, setTilt] = useState(0.15);
  const [zoom, setZoom] = useState(1);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);
  const [compareMode, setCompareMode] = useState(false);
  const [dockKey, setDockKey] = useState(0);
  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);
  const defaultOrder = ["color", "profile", "export"];
  const [panelOrder, setPanelOrder] = useState(defaultOrder);

  const cv3dRef = useRef(null);
  const cv2dRef = useRef(null);
  const cvBarRef = useRef(null);
  const dragRef = useRef(null);
  const preview3dContainerRef = useRef(null);
  const preview2dContainerRef = useRef(null);
  const [resizeTick, setResizeTick] = useState(0);

  // Mutate the current tier's profile via a callback; deep-copy to avoid mutation hazards.
  const upd = fn => setTiers(prev => {
    const n = prev.map(t => ({ prof: { ...t.prof }, stops: t.stops }));
    fn(n[tierIdx].prof);
    return n;
  });
  // Update a single color stop in the current tier.
  const updStop = (idx, color) => setTiers(prev => prev.map((t, i) =>
    i === tierIdx ? { ...t, stops: t.stops.map((s, si) => si === idx ? { ...s, c: color } : s) } : t
  ));
  // Wholesale-replace the current tier's stops (used by gradient presets).
  const setCurrentStops = (newStops) => setTiers(prev => prev.map((t, i) =>
    i === tierIdx ? { ...t, stops: newStops } : t
  ));
  // Reset just the current tier to its defaults.
  const resetCurrentTier = () => setTiers(prev => prev.map((t, i) =>
    i === tierIdx ? defaultPawnTier(tierIdx) : t
  ));

  // Load saved state on mount (tolerates older single-pawn schema by promoting it to tier 0).
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) {
          const data = JSON.parse(raw);
          if (Array.isArray(data) && data.every(t => t?.prof && Array.isArray(t?.stops))) {
            // Tier-array payload. Length might mismatch if we've added (or removed) tiers
            // since the save — keep what we have, fill any extras from defaults.
            const merged = defaultAllTiers().map((d, i) => data[i] || d);
            setTiers(merged);
          } else if (data?.prof && Array.isArray(data?.stops)) {
            // Legacy single-tier payload — promote into tier 0, leave the rest as defaults.
            setTiers(prev => {
              const n = defaultAllTiers();
              n[0] = { prof: data.prof, stops: data.stops };
              return n;
            });
          }
        }
      } catch (e) {}
      setLoaded(true);
    })();
  }, []);

  // Auto-save on every edit after initial load.
  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(tiers)); } catch (e) {} })();
  }, [tiers, loaded]);

  // Re-render canvases when preview containers resize
  useEffect(() => {
    const ro = new ResizeObserver(() => setResizeTick(t => t + 1));
    if (preview3dContainerRef.current) ro.observe(preview3dContainerRef.current);
    if (preview2dContainerRef.current) ro.observe(preview2dContainerRef.current);
    return () => ro.disconnect();
  }, []);

  // 3D canvas rendering — dispatches between single-tier and compare-all modes.
  useEffect(() => {
    const cv = cv3dRef.current; if (!cv) return;
    const dpr = window.devicePixelRatio || 1;
    const W = cv.clientWidth, H = cv.clientHeight;
    cv.width = W * dpr; cv.height = H * dpr;
    const ctx = cv.getContext("2d");
    ctx.scale(dpr, dpr);
    if (compareMode) render3DCompare(ctx, W, H, tiers, rotY, tilt, zoom, panX, panY);
    else             render3D(ctx, W, H, prof, stops, rotY, tilt, zoom, panX, panY);
  }, [tiers, tierIdx, compareMode, prof, stops, rotY, tilt, zoom, panX, panY, resizeTick]);

  // Scroll-wheel zoom on the 3D preview. The [loaded] dep is critical: at first mount
  // `loaded` is false and the canvas isn't in the DOM yet (the component returns a
  // Loading… placeholder), so the ref is null. When the load completes and the canvas
  // mounts, this effect re-runs and binds the listener for real. Non-passive so we can
  // preventDefault and stop the page from scrolling.
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

  // 2D profile rendering — dispatches between single-tier and compare-all modes.
  useEffect(() => {
    const cv = cv2dRef.current; if (!cv) return;
    const dpr = window.devicePixelRatio || 1;
    const W = cv.clientWidth, H = cv.clientHeight;
    cv.width = W * dpr; cv.height = H * dpr;
    const ctx = cv.getContext("2d");
    ctx.scale(dpr, dpr);
    if (compareMode) render2DCompare(ctx, W, H, tiers);
    else             render2D(ctx, W, H, prof, stops);
  }, [tiers, tierIdx, compareMode, prof, stops, resizeTick]);

  // Gradient bar
  useEffect(() => {
    const cv = cvBarRef.current; if (!cv) return;
    const dpr = window.devicePixelRatio || 1;
    const W = cv.clientWidth, H = cv.clientHeight;
    cv.width = W * dpr; cv.height = H * dpr;
    const ctx = cv.getContext("2d");
    ctx.scale(dpr, dpr);
    renderGradientBar(ctx, W, H, stops);
  }, [stops]);

  // Mouse drag on the 3D canvas. Right button (button === 2) pans the camera in
  // screen pixels; anything else (left, middle) rotates as before. Pan offset is
  // applied straight to the projection, independent of zoom/scale.
  const onPointerDown3D = useCallback(e => {
    const startX = e.clientX, startY = e.clientY;
    if (e.button === 2) {
      // Pan
      e.preventDefault();
      const startPanX = panX, startPanY = panY;
      const onMove = ev => {
        setPanX(startPanX + (ev.clientX - startX));
        setPanY(startPanY + (ev.clientY - startY));
      };
      const onUp = () => {
        window.removeEventListener("pointermove", onMove);
        window.removeEventListener("pointerup", onUp);
      };
      window.addEventListener("pointermove", onMove);
      window.addEventListener("pointerup", onUp);
    } else {
      // Rotate
      const startRot = rotY, startTilt = tilt;
      const onMove = ev => {
        setRotY(startRot + (ev.clientX - startX) * 0.01);
        setTilt(startTilt - (ev.clientY - startY) * 0.008);
      };
      const onUp = () => {
        window.removeEventListener("pointermove", onMove);
        window.removeEventListener("pointerup", onUp);
      };
      window.addEventListener("pointermove", onMove);
      window.addEventListener("pointerup", onUp);
    }
  }, [rotY, tilt, panX, panY]);

  const wgslCode = generateWGSL(prof, stops);

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{ fontFamily: "'JetBrains Mono', 'SF Mono', 'Fira Code', monospace", color: "var(--color-text-primary)", lineHeight: 1.4, position: "relative", fontSize: 11 }}>
      {/* ── TOP BAR ── */}
      <div style={{ ...rw, marginBottom: 6, padding: "2px 0" }}>
        <span style={{ fontSize: 12, fontWeight: 600, letterSpacing: "0.02em" }}>7T Pawn Designer</span>
        {isModified && <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 6 }}>● modified</span>}
        <div style={{ marginLeft: "auto", display: "flex", gap: 4 }}>
          <button onClick={() => { setDockKey(k => k + 1); setPanelOrder(defaultOrder); }} style={btnStyle}>Dock all</button>
          <button onClick={() => setTiers(defaultAllTiers())} style={btnStyle}>Reset all</button>
          {!compareMode && <button onClick={resetCurrentTier} style={btnStyle}>Reset tier</button>}
          <button onClick={() => {
            const r = (v, d=4) => +(+v).toFixed(d);
            const a = arr => "[" + arr.map(v => r(v)).join(", ") + "]";
            let s = "=== 7T PAWN DESIGNER STATE ===\n\n";
            tiers.forEach((tier, ti) => {
              s += `══ Tier ${ti}: ${TIER_NAMES[ti]} ══\n\n`;
              s += "── Profile geometry ──\n";
              Object.entries(tier.prof).forEach(([k, v]) => { s += `  ${k}: ${r(v)}\n`; });
              s += "\n── Color gradient stops ──\n";
              tier.stops.forEach((st) => { s += `  ${st.label} (t=${r(st.t)}): rgb(${a(st.c)})\n`; });
              s += "\n";
            });
            s += "── JSON (for import) ──\n";
            s += JSON.stringify(tiers, null, 2) + "\n";
            copyText(s); (() => {
              const btn = document.activeElement;
              if (btn) { const orig = btn.textContent; btn.textContent = "Copied!"; setTimeout(() => { btn.textContent = orig; }, 1200); }
            })();
          }} style={{ ...btnStyle, border: "1px solid var(--color-border-success)", color: "var(--color-text-success)", background: "transparent" }}>Save</button>
        </div>
      </div>

      {/* ── TIER TABS ── */}
      <div style={{ display: "flex", gap: 3, marginBottom: 6, flexWrap: "wrap" }}>
        {TIER_NAMES.map((name, i) => {
          const active = i === tierIdx && !compareMode;
          return (
            <button key={i} onClick={() => { setTierIdx(i); setCompareMode(false); }} style={{
              fontSize: 10, padding: "3px 8px", borderRadius: 4, cursor: "pointer",
              border: active ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)",
              background: active ? "var(--color-background-info)" : "var(--color-background-secondary)",
              color: active ? "var(--color-text-info)" : "var(--color-text-secondary)",
              fontWeight: active ? 600 : 400,
            }}>{name}</button>
          );
        })}
        <button onClick={() => setCompareMode(true)} style={{
          fontSize: 10, padding: "3px 8px", borderRadius: 4, cursor: "pointer", marginLeft: 6,
          border: compareMode ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)",
          background: compareMode ? "var(--color-background-info)" : "var(--color-background-secondary)",
          color: compareMode ? "var(--color-text-info)" : "var(--color-text-secondary)",
          fontWeight: compareMode ? 600 : 400,
        }} title="Show all tiers side-by-side at shared scale">Compare</button>
      </div>

      {/* ── CANVAS ROW ── */}
      <div style={{ display: "flex", gap: 6, marginBottom: 6 }}>
        {/* 3D preview — resizable */}
        <div ref={preview3dContainerRef} style={{ flex: 2, position: "relative", border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "#16191f", resize: "vertical", minHeight: 180, height: 340 }}>
          <canvas
            ref={cv3dRef}
            onPointerDown={onPointerDown3D}
            onContextMenu={e => e.preventDefault()}
            style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }}
          />
          <div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9, color: "rgba(255,255,255,0.3)" }}>drag to rotate · right-drag to pan · scroll to zoom · resize ↘</div>
          <div
            onClick={() => { setZoom(1); setPanX(0); setPanY(0); }}
            title="Click to reset view"
            style={{ position: "absolute", top: 4, right: 8, fontSize: 9, color: "rgba(255,255,255,0.4)", cursor: "pointer", userSelect: "none", padding: "2px 4px" }}
          >{compareMode ? "Compare" : TIER_NAMES[tierIdx]} · {Math.round(zoom * 100)}%</div>
        </div>
        {/* 2D profile — resizable */}
        <div ref={preview2dContainerRef} style={{ flex: 1, minWidth: 120, border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", resize: "vertical", minHeight: 180, height: 340 }}>
          <canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} />
        </div>
      </div>

      {/* ── COMPARE-MODE HINT ── */}
      {compareMode && (
        <div style={{ padding: "10px 14px", border: "1px dashed var(--color-border-tertiary)", borderRadius: 6, fontSize: 10, color: "var(--color-text-tertiary)", textAlign: "center", marginBottom: 6 }}>
          Compare view — pick a tier above to edit its profile and colors.
        </div>
      )}

      {/* ── GRADIENT BAR ── */}
      {!compareMode && (
        <div style={{ marginBottom: 6 }}>
          <div style={{ fontSize: 10, color: "var(--color-text-secondary)", marginBottom: 2 }}>t = 0 → 1 (base → tip)</div>
          <div style={{ border: "1px solid var(--color-border-tertiary)", borderRadius: 4, overflow: "hidden", height: 18 }}>
            <canvas ref={cvBarRef} style={{ width: "100%", height: 18, display: "block" }} />
          </div>
        </div>
      )}

      {/* ═══ PANELS — ordered, draggable ═══ */}

      {!compareMode && panelOrder.map(pid => {
        const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };
        switch (pid) {

        case "color": return (
      <DragPanel {...dp} title="Color Gradient" ini={true}>
        <div style={{ display: "flex", gap: 4, marginBottom: 6, flexWrap: "wrap" }}>
          {Object.keys(PRESETS).map(name =>
            <button key={name} onClick={() => setCurrentStops(PRESETS[name]())} style={{ ...btnStyle, fontSize: 9 }}>{name}</button>
          )}
        </div>
        {stops.map((s, i) => <ColorStopRow key={i} stop={s} index={i} onChange={updStop} />)}
      </DragPanel>);

        case "profile": return (
      <DragPanel {...dp} title="Profile Geometry">
        <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>Section boundaries (t)</div>
        <div style={rw}>
          <span style={lb}>Base end</span><Num value={prof.base_t} min={0.05} max={0.3} w={40} onChange={v => upd(n => { n.base_t = v; })} />
          <span style={lb}>Body end</span><Num value={prof.body_t} min={0.2} max={0.65} w={40} onChange={v => upd(n => { n.body_t = v; })} />
          <span style={lb}>Neck end</span><Num value={prof.neck_t} min={0.35} max={0.75} w={40} onChange={v => upd(n => { n.neck_t = v; })} />
        </div>
        <div style={rw}>
          <span style={lb}>Collar end</span><Num value={prof.collar_t} min={0.5} max={0.85} w={40} onChange={v => upd(n => { n.collar_t = v; })} />
          <span style={lb}>Head end</span><Num value={prof.head_t} min={0.7} max={0.99} w={40} onChange={v => upd(n => { n.head_t = v; })} />
        </div>
        <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Key radii (normalized)</div>
        <div style={rw}>
          <span style={lb}>Start</span><Num value={prof.start_r} min={0.1} max={1} w={38} onChange={v => upd(n => { n.start_r = v; })} />
          <span style={lb}>Flare</span><Num value={prof.flare_r} min={0.3} max={1} w={38} onChange={v => upd(n => { n.flare_r = v; })} />
          <span style={lb}>Peak</span><Num value={prof.peak_r} min={0.3} max={1.2} w={38} onChange={v => upd(n => { n.peak_r = v; })} />
        </div>
        <div style={rw}>
          <span style={lb}>Body start</span><Num value={prof.body_start_r} min={0.1} max={1} w={38} onChange={v => upd(n => { n.body_start_r = v; })} />
          <span style={lb}>Waist</span><Num value={prof.waist_r} min={0.05} max={0.6} w={38} onChange={v => upd(n => { n.waist_r = v; })} />
          <span style={lb}>Neck</span><Num value={prof.neck_r} min={0.05} max={0.4} w={38} onChange={v => upd(n => { n.neck_r = v; })} />
        </div>
        <div style={rw}>
          <span style={lb}>Collar bulge</span><Num value={prof.collar_bulge} min={0} max={0.3} w={38} onChange={v => upd(n => { n.collar_bulge = v; })} />
          <span style={lb}>Head base</span><Num value={prof.head_base_r} min={0.05} max={0.4} w={38} onChange={v => upd(n => { n.head_base_r = v; })} />
          <span style={lb}>Head sphere</span><Num value={prof.head_sphere_r} min={0.1} max={0.6} w={38} onChange={v => upd(n => { n.head_sphere_r = v; })} />
        </div>
        <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Scale (world units)</div>
        <div style={rw}>
          <span style={lb}>Height</span><Num value={prof.height} min={0.5} max={4} w={40} onChange={v => upd(n => { n.height = v; })} />
          <span style={lb}>Radius</span><Num value={prof.radius} min={0.1} max={2} w={40} onChange={v => upd(n => { n.radius = v; })} />
        </div>
        <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>WGSL defaults: height=1.5, radius=0.5</div>
      </DragPanel>);

        case "export": return (
      <DragPanel {...dp} title="WGSL Export">
        <div style={{ display: "flex", gap: 4, marginBottom: 4 }}>
          <button onClick={() => { copyText(wgslCode); setCopied(true); setTimeout(() => setCopied(false), 1500); }} style={{ ...btnStyle, background: copied ? "var(--color-background-success)" : "var(--color-background-secondary)" }}>
            {copied ? "Copied" : "Copy WGSL"}
          </button>
          <button onClick={() => setShowCode(!showCode)} style={btnStyle}>{showCode ? "Hide" : "Show"} code</button>
        </div>
        {showCode && (
          <textarea
            readOnly
            value={wgslCode}
            style={{
              width: "100%", height: 220, fontSize: 10, fontFamily: "'JetBrains Mono', monospace",
              background: "var(--color-background-primary)", color: "var(--color-text-primary)",
              border: "1px solid var(--color-border-tertiary)", borderRadius: 4, padding: 6,
              resize: "vertical", lineHeight: 1.4, whiteSpace: "pre",
            }}
          />
        )}
      </DragPanel>);

        default: return null;
        }
      })}
    </div>
  );
}

const btnStyle = {
  fontSize: 10, padding: "2px 8px", borderRadius: 4, cursor: "pointer",
  border: "1px solid var(--color-border-tertiary)",
  background: "var(--color-background-secondary)",
  color: "var(--color-text-secondary)",
};