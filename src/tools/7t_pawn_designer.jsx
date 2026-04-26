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
const RENDER_SEGS = 48;
const RENDER_RINGS = 32;
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

/* Generate the face list for one pawn — surface revolution + bottom cap + optional disc rings.
   Generic over geometry: caller supplies radiusAt(t) ∈ [0,1] (normalized) plus
   the world-space height and outer radius. `transformVert` is applied to every
   local vertex, so callers control rotation and world placement. Faces include
   lighting and gradient colour, ready for sort+draw. No projection here —
   caller projects into screen space.

   `discs` (optional) is an array of {t, rBelow, rAbove} entries. At each t a
   horizontal disc/annulus is emitted to bridge the radius gap — this is how the
   stacked family represents hard segment transitions (e.g. a `step` profile
   shouldering up to a wider segment). Without this the surface revolution would
   draw an oblique band where the geometry is supposed to step. */
function buildPawnFaces(radiusAt, height, radius, stops, transformVert, discs = null, extra = null) {
  const faces = [];
  const lightDir = normalize3(PAWN_LIGHT_DIR);

  // Surface revolution. radiusAt may depend on theta — we sample
  // per-vertex (4 queries per quad) rather than once per ring.
  for (let r = 0; r < RENDER_RINGS - 1; r++) {
    for (let s = 0; s < RENDER_SEGS; s++) {
      const t0 = r / (RENDER_RINGS - 1), t1 = (r + 1) / (RENDER_RINGS - 1);
      const a0 = (s / RENDER_SEGS) * Math.PI * 2, a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
      const r00 = radiusAt(t0, a0) * radius;
      const r01 = radiusAt(t0, a1) * radius;
      const r10 = radiusAt(t1, a0) * radius;
      const r11 = radiusAt(t1, a1) * radius;
      const y0 = t0 * height - height * 0.45;
      const y1 = t1 * height - height * 0.45;
      const ca0 = Math.cos(a0), sa0 = Math.sin(a0), ca1 = Math.cos(a1), sa1 = Math.sin(a1);
      const verts = [
        transformVert([ca0*r00, y0, sa0*r00]),
        transformVert([ca1*r01, y0, sa1*r01]),
        transformVert([ca1*r11, y1, sa1*r11]),
        transformVert([ca0*r10, y1, sa0*r10]),
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

  // Bottom cap. For axisymmetric families radiusAt(0) returns the base radius
  // for any angle; for theta-dependent families we sample per-segment and emit
  // triangles whose outer edge follows the actual perimeter shape.
  const capY = -height * 0.45;
  const capCol = evalGradient(0, stops);
  for (let s = 0; s < RENDER_SEGS; s++) {
    const a0 = (s / RENDER_SEGS) * Math.PI * 2;
    const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
    const ra = radiusAt(0, a0) * radius;
    const rb = radiusAt(0, a1) * radius;
    const verts = [
      transformVert([0, capY, 0]),
      transformVert([Math.cos(a1)*rb, capY, Math.sin(a1)*rb]),
      transformVert([Math.cos(a0)*ra, capY, Math.sin(a0)*ra]),
    ];
    const ce1 = sub3(verts[1], verts[0]), ce2 = sub3(verts[2], verts[0]);
    const n = normalize3(cross3(ce1, ce2));
    if (n[2] > 0.02) continue;
    const diff = Math.max(0, n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
    const light = 0.28 + 0.72 * diff;
    const avgZ = (verts[0][2] + verts[1][2] + verts[2][2]) / 3;
    faces.push({ verts, light, col: capCol, avgZ, tri: true });
  }

  // Top cap — only emitted when the silhouette at t=1 has non-trivial radius.
  // Without this, any pawn whose top segment doesn't taper to a point reads as
  // an open bottle. For pieces that already taper to ~0 (Bishop's mitre, Queen's
  // orb-to-point, etc.) the work is skipped — there's no hole to plug.
  // Use the angle-0 sample as the closure test (a noisy lobe could give slightly
  // higher r_top in some directions but this is sufficient as a heuristic).
  const topR0 = radiusAt(1, 0) * radius;
  if (topR0 > 1e-4) {
    const topY = height * 0.55; // mirror of capY: t=1 maps to height*0.55
    const topCol = evalGradient(1, stops);
    for (let s = 0; s < RENDER_SEGS; s++) {
      const a0 = (s / RENDER_SEGS) * Math.PI * 2;
      const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
      const ra = radiusAt(1, a0) * radius;
      const rb = radiusAt(1, a1) * radius;
      // Wound CCW viewed from above — opposite winding from the bottom cap so
      // its outward normal points up rather than down.
      const verts = [
        transformVert([0, topY, 0]),
        transformVert([Math.cos(a0)*ra, topY, Math.sin(a0)*ra]),
        transformVert([Math.cos(a1)*rb, topY, Math.sin(a1)*rb]),
      ];
      const te1 = sub3(verts[1], verts[0]), te2 = sub3(verts[2], verts[0]);
      const n = normalize3(cross3(te1, te2));
      if (n[2] > 0.02) continue;
      const diff = Math.max(0, n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
      const light = 0.28 + 0.72 * diff;
      const avgZ = (verts[0][2] + verts[1][2] + verts[2][2]) / 3;
      faces.push({ verts, light, col: topCol, avgZ, tri: true });
    }
  }

  // Disc rings at segment boundaries (stacked geometry hard transitions).
  // Each disc is an annulus from rInner to rOuter, facing up if rOuter>rInner
  // (top of a wider lower segment showing through), or facing down if rInner>rOuter.
  // For axisymmetric families discs come in as scalar rBelow/rAbove pairs; we
  // sample both at angle 0 effectively (the values are constant in theta).
  if (discs) {
    for (const d of discs) {
      const rA = d.rBelow * radius, rB = d.rAbove * radius;
      if (Math.abs(rA - rB) < 1e-5) continue;
      const rInner = Math.min(rA, rB), rOuter = Math.max(rA, rB);
      const yDisc  = d.t * height - height * 0.45;
      const col    = evalGradient(d.t, stops);
      const facingUp = rA > rB;
      const n = facingUp ? [0, 1, 0] : [0, -1, 0];
      const diff = Math.max(0, n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
      const light = 0.28 + 0.72 * diff;
      for (let s = 0; s < RENDER_SEGS; s++) {
        const a0 = (s / RENDER_SEGS) * Math.PI * 2;
        const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
        const ca0 = Math.cos(a0), sa0 = Math.sin(a0), ca1 = Math.cos(a1), sa1 = Math.sin(a1);
        const verts = facingUp ? [
          transformVert([ca0 * rInner, yDisc, sa0 * rInner]),
          transformVert([ca0 * rOuter, yDisc, sa0 * rOuter]),
          transformVert([ca1 * rOuter, yDisc, sa1 * rOuter]),
          transformVert([ca1 * rInner, yDisc, sa1 * rInner]),
        ] : [
          transformVert([ca0 * rInner, yDisc, sa0 * rInner]),
          transformVert([ca1 * rInner, yDisc, sa1 * rInner]),
          transformVert([ca1 * rOuter, yDisc, sa1 * rOuter]),
          transformVert([ca0 * rOuter, yDisc, sa0 * rOuter]),
        ];
        const avgZ = (verts[0][2] + verts[1][2] + verts[2][2] + verts[3][2]) / 4;
        faces.push({ verts, light, col, avgZ });
      }
    }
  }

  // Family-supplied extruded geometry (e.g. Heraldic family's ornamental head).
  // Caller emits raw vertex triples already passed through transformVert; we
  // light, color, and z-sort them here so they participate in the painter's
  // algorithm alongside the surface revolution.
  //
  // These faces are rendered DOUBLE-SIDED: each triangle is emitted twice, once
  // with its original winding and once reversed, so the head reads correctly
  // regardless of viewing angle and regardless of how the family's geometry
  // generator wound its triangles. Lighting uses the absolute dot product so
  // backs aren't pure black. This is the right tradeoff for thin-shell
  // hand-built geometry (heads, ornaments) where doing rigorous winding audits
  // for every shape doesn't pay off.
  if (extra) {
    for (const tri of extra) {
      const [v0, v1, v2] = tri.verts;
      const e1 = sub3(v1, v0), e2 = sub3(v2, v0);
      const n = normalize3(cross3(e1, e2));
      // Two-sided lighting: use |n·L| so we get diffuse on both faces.
      const diff = Math.abs(n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
      const light = 0.28 + 0.72 * diff;
      const col = tri.col || evalGradient(tri.t != null ? tri.t : 1, stops);
      const avgZ = (v0[2] + v1[2] + v2[2]) / 3;
      // Front face.
      faces.push({ verts: [v0, v1, v2], light, col, avgZ, tri: true });
      // Back face — slightly nudge avgZ so the painter's algorithm renders it
      // just behind the front face when both are co-planar (same depth).
      faces.push({ verts: [v0, v2, v1], light, col, avgZ: avgZ + 1e-5, tri: true });
    }
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
   Pan offsets shift the projection in screen pixels (applied after auto-fit).
   Family must provide radiusAt(geom, t) and bounds(geom)→{height,radius}. */
function render3D(ctx, W, H, family, geom, stops, rotY, tilt, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);
  const { height, radius } = family.bounds(geom);
  // envelopeMax tells us the largest normalized-radius the silhouette reaches.
  // Default 1.0 keeps the smooth family unchanged; stacked reports its widest
  // segment so flared bases / overhanging crowns stay on canvas.
  const envMax = family.envelopeMax ? family.envelopeMax(geom) : 1.0;
  const scaleY = H * 0.78 / Math.max(height, 0.0001);
  const scaleX = W * 0.85 / Math.max(radius * 2 * envMax, 0.0001);
  const scale  = Math.min(scaleY, scaleX) * zoom;
  const cy = H * 0.52;
  const transformVert = makeTransform(rotY, tilt, 0, 0);
  const discs = family.discontinuities ? family.discontinuities(geom) : null;
  const extra = family.extraFaces ? family.extraFaces(geom, height, radius, transformVert) : null;
  const faces = buildPawnFaces((t, theta) => family.radiusAt(geom, t, theta), height, radius, stops, transformVert, discs, extra);
  const project = (v) => [W / 2 + panX + v[0] * scale, cy + panY - v[1] * scale];
  drawFaces(ctx, faces, project);
}

/* Comparison 3D: all tiers in a single shared scene. Bases aligned on world y=0,
   pawns spaced along world x, the whole scene rotates as one unit. Faces from all
   pawns are merged into a single depth-sorted list so they occlude each other
   correctly when the scene is rotated. */
function render3DCompare(ctx, W, H, family, tiers, rotY, tilt, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);

  // Each tier's slot is a hair wider than its silhouette so neighbours don't kiss.
  // envelopeMax accounts for segments/lobes that flare beyond the unit envelope —
  // without it, wide stacked pieces would overlap their neighbours in compare mode.
  const tierBounds = tiers.map(t => family.bounds(t.geom));
  const tierEnv    = tiers.map(t => family.envelopeMax ? family.envelopeMax(t.geom) : 1.0);
  const slotWidths = tierBounds.map((b, i) => b.radius * 2 * tierEnv[i] * 1.25);
  const totalW = slotWidths.reduce((a, b) => a + b, 0);
  const maxH   = Math.max(...tierBounds.map(b => b.height));

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
  const yOffsets = tierBounds.map(b => 0.45 * b.height);

  // Collect every face from every pawn, all in the same world space.
  const allFaces = [];
  tiers.forEach((tier, i) => {
    const b = tierBounds[i];
    const transformVert = makeTransform(rotY, tilt, xOffsets[i], yOffsets[i]);
    const discs = family.discontinuities ? family.discontinuities(tier.geom) : null;
    const extra = family.extraFaces ? family.extraFaces(tier.geom, b.height, b.radius, transformVert) : null;
    const faces = buildPawnFaces((t, theta) => family.radiusAt(tier.geom, t, theta), b.height, b.radius, tier.stops, transformVert, discs, extra);
    for (const f of faces) allFaces.push(f);
  });

  const project = (v) => [W / 2 + panX + v[0] * scale, cy + panY - v[1] * scale];
  drawFaces(ctx, allFaces, project);
}

function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b) { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v) { const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l < 1e-8 ? [0,0,1] : [v[0]/l, v[1]/l, v[2]/l]; }

/* ═══ 2D PROFILE CROSS-SECTION ═══ */
function render2D(ctx, W, H, family, geom, stops) {
  ctx.clearRect(0, 0, W, H);
  const mg = { l: 30, r: 10, t: 10, b: 20 };
  const pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  const envMax = family.envelopeMax ? family.envelopeMax(geom) : 1.0;
  const maxR = Math.max(1.05, envMax * 1.05);
  // For axisymmetric families this collapses to a single sample per t. For
  // theta-dependent families take the max across a few angle samples
  // so the side silhouette shows the lobe envelope rather than an arbitrary slice.
  const radiusAt = (t) => {
    let r = 0;
    const N = 16;
    for (let k = 0; k < N; k++) {
      const a = (k / N) * Math.PI * 2;
      const v = family.radiusAt(geom, t, a);
      if (v > r) r = v;
    }
    return r;
  };

  // Background
  ctx.fillStyle = "var(--color-background-secondary)";
  ctx.fillRect(0, 0, W, H);

  // Draw filled silhouette with gradient
  const steps = 200;
  for (let i = 0; i < steps; i++) {
    const t0 = i / steps, t1 = (i + 1) / steps;
    const r0 = radiusAt(t0), r1 = radiusAt(t1);
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
    const r0 = radiusAt(t0), r1 = radiusAt(t1);
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

  // Section boundary lines — family-supplied
  const bounds = family.boundaries ? family.boundaries(geom) : [];
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

  // Profile outline (with hard steps where the family reports radius discontinuities).
  // Without this, the 200-sample sweep would draw an oblique line across a step,
  // making `step`-profile silhouettes look gradient-y instead of cleanly stepped.
  const discs = family.discontinuities ? family.discontinuities(geom) : [];
  ctx.strokeStyle = "var(--color-text-primary)";
  ctx.lineWidth = 1.2;
  for (const sign of [+1, -1]) {
    ctx.beginPath();
    let didMove = false;
    for (let i = 0; i <= steps; i++) {
      const t = i / steps;
      // If we just stepped past a discontinuity, emit a hard horizontal jump first.
      const passed = discs.find(d => i > 0 && (i - 1) / steps < d.t && t >= d.t);
      if (passed) {
        const yJump = mg.t + ph - passed.t * ph;
        const xBelow = mg.l + pw / 2 + sign * (passed.rBelow / maxR) * pw / 2;
        const xAbove = mg.l + pw / 2 + sign * (passed.rAbove / maxR) * pw / 2;
        ctx.lineTo(xBelow, yJump);
        ctx.lineTo(xAbove, yJump);
      }
      const r = radiusAt(t);
      const x = mg.l + pw / 2 + sign * (r / maxR) * pw / 2;
      const y = mg.t + ph - t * ph;
      if (!didMove) { ctx.moveTo(x, y); didMove = true; }
      else ctx.lineTo(x, y);
    }
    ctx.stroke();
  }

  // Axis labels
  ctx.fillStyle = "var(--color-text-tertiary)";
  ctx.font = "9px monospace";
  ctx.textAlign = "right";
  ctx.fillText("1.0", mg.l - 3, mg.t + 5);
  ctx.fillText("0.0", mg.l - 3, mg.t + ph + 4);
}

/* All-tiers 2D: shared scale, bases aligned on the same baseline — mirrors the 3D scene
   so size relationships are read the same way in both views. */
function render2DCompare(ctx, W, H, family, tiers) {
  ctx.clearRect(0, 0, W, H);

  // World layout: same scheme as the 3D compare scene.
  const tierBounds = tiers.map(t => family.bounds(t.geom));
  const tierEnv    = tiers.map(t => family.envelopeMax ? family.envelopeMax(t.geom) : 1.0);
  const slotWidths = tierBounds.map((b, i) => b.radius * 2 * tierEnv[i] * 1.25);
  const totalW = slotWidths.reduce((a, b) => a + b, 0);
  const maxH   = Math.max(...tierBounds.map(b => b.height));

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
    const radiusAt = (t) => {
      let r = 0;
      const N = 16;
      for (let k = 0; k < N; k++) {
        const a = (k / N) * Math.PI * 2;
        const v = family.radiusAt(tier.geom, t, a);
        if (v > r) r = v;
      }
      return r;
    };
    const b = tierBounds[i];
    const stops = tier.stops;
    const cx = xCentre + xOffsets[i] * scale;
    const baseAtT = (t) => baseY - (t * b.height) * scale;

    // Filled silhouette
    for (let k = 0; k < STEPS; k++) {
      const t0 = k / STEPS, t1 = (k + 1) / STEPS;
      const r0 = radiusAt(t0) * b.radius;
      const r1 = radiusAt(t1) * b.radius;
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
      const r0 = radiusAt(t0) * b.radius * 0.85;
      const r1 = radiusAt(t1) * b.radius * 0.85;
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
        const r = radiusAt(t) * b.radius;
        const x = cx + sign * r * scale, y = baseAtT(t);
        if (k === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }

    // Dimensions below
    ctx.fillStyle = "rgba(255,255,255,0.4)";
    ctx.font = "8px monospace";
    ctx.textAlign = "center";
    ctx.fillText(`${b.height.toFixed(1)} × ${b.radius.toFixed(2)}`, cx, H - 5);
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

function Num({ value, onChange, min = 0, max = 1, step = 0.01, w = 46, tip = "" }) {
  const [txt, setTxt] = useState(String(Math.round(value * 10000) / 10000));
  const [slider, setSlider] = useState(false);
  useEffect(() => { setTxt(String(Math.round(value * 10000) / 10000)); }, [value]);
  const commit = () => { const v = parseFloat(txt); if (!isNaN(v)) onChange(Math.max(min, Math.min(max, v))); else setTxt(String(Math.round(value * 10000) / 10000)); };
  // Range string in the tooltip — uses g-precision so we don't print "0.5000".
  const rangeTxt = `range: [${(+min).toPrecision(3).replace(/\.?0+$/, "")}, ${(+max).toPrecision(3).replace(/\.?0+$/, "")}]`;
  const titleTxt = tip ? `${tip}\n${rangeTxt}` : rangeTxt;
  return (
    <span style={{ display: "inline-flex", alignItems: "center", gap: 2, position: "relative" }} title={titleTxt}>
      <span
        onClick={() => setSlider(s => !s)}
        style={{ cursor: "pointer", fontSize: 8, lineHeight: 1, color: slider ? "var(--color-text-info)" : "var(--color-text-tertiary)", userSelect: "none", opacity: slider ? 1 : 0.5, transition: "opacity .15s", width: 8, textAlign: "center" }}
        title="Toggle slider"
      >▸</span>
      {slider && <input type="range" min={min} max={max} step={step} value={value} style={{ width: 56, height: 12, margin: 0 }} onChange={e => onChange(+e.target.value)} />}
      <input type="text" value={txt} onChange={e => setTxt(e.target.value)} onBlur={commit} onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }} style={{ ...ist, width: w }} title={titleTxt} />
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

const STORAGE_KEY = "7t:pawn:families:v1";

/* ═══════════════════════════════════════════════════════════════════════
   FAMILY REGISTRY

   A "family" is a complete pawn geometry plus its tier roster. Each family
   provides:
     - name          : display name (string)
     - tierNames     : array of tier display names
     - defaultTiers  : function → array of {geom, stops}
     - radiusAt      : (geom, t) → number, normalized 0..1
     - bounds        : (geom) → {height, radius}  [world-space]
     - boundaries    : (geom) → [{t, label}] for the 2D dashed lines (optional)

   PawnDesigner is family-agnostic above the FAMILIES level: it picks one
   active family and routes all geometry queries through it.

   Editor panels for each family are defined further down (SmoothPanels,
   StackedPanels) and attached to the registry alongside the renderers.
   ═══════════════════════════════════════════════════════════════════════ */

/* ── SMOOTH FAMILY — profile-of-revolution, the original pawn ───────── */
const SMOOTH_FAMILY = {
  name: "Smooth",
  tierNames: TIER_NAMES,
  defaultTiers: () => TIER_NAMES.map((_, i) => ({
    geom:  TIER_PROFILE_FNS[i](),
    stops: TIER_STOPS_FNS[i](),
  })),
  radiusAt: (geom, t, _theta) => pawnRadius(t, geom),
  bounds:   (geom) => ({ height: geom.height, radius: geom.radius }),
  // Largest normalized radius reached by the silhouette. peak_r is by construction
  // the widest point, but we Math.max over the rest in case the artist sets weird
  // combinations where another field exceeds peak_r.
  envelopeMax: (geom) => Math.max(
    geom.start_r, geom.flare_r, geom.peak_r,
    geom.body_start_r, geom.waist_r, geom.neck_r,
    geom.collar_bulge + geom.neck_r,  // collar can bulge above the neck radius
    geom.head_base_r, geom.head_sphere_r,
    0.0001
  ),
  boundaries: (geom) => [
    { t: geom.base_t,   label: "Base"   },
    { t: geom.body_t,   label: "Body"   },
    { t: geom.neck_t,   label: "Neck"   },
    { t: geom.collar_t, label: "Collar" },
    { t: geom.head_t,   label: "Head"   },
  ],
};

/* ── STACKED FAMILY — 5 fixed segments stacked along height ─────────── */

/* Segment roles, bottom→top. Fixed semantics so the panel UI can label them. */
const STACKED_SEG_NAMES = ["Base", "Shaft", "Waist", "Collar", "Finial"];

/* Per-segment profile shapes — control how r(u∈[0,1]) interpolates between r_bot and r_top.
   All return the *normalized radius* at local u inside the segment.
     linear  : straight frustum
     concave : pinched midpoint (curves inward)
     convex  : barrelled midpoint (curves outward)
     step    : near-cylindrical body (r_bot until u≈0.85, then quick taper to r_top)
*/
const STACKED_SHAPES = {
  linear:  (u, rb, rt) => rb + (rt - rb) * u,
  concave: (u, rb, rt) => {
    // pinched midpoint: dip ~25% below the line midpoint
    const line = rb + (rt - rb) * u;
    const mid  = (rb + rt) * 0.5;
    return line - 0.25 * mid * Math.sin(u * Math.PI);
  },
  convex:  (u, rb, rt) => {
    // bulged midpoint: lift ~25% above the line midpoint
    const line = rb + (rt - rb) * u;
    const mid  = (rb + rt) * 0.5;
    return line + 0.25 * mid * Math.sin(u * Math.PI);
  },
  step:    (u, rb, rt) => {
    if (u < 0.85) return rb;
    return rb + (rt - rb) * ((u - 0.85) / 0.15);
  },
};

/* Compute normalized radius at global t∈[0,1] for a stacked geometry.
   Walks the segment list summing heights until t falls inside segment k,
   then dispatches to that segment's shape function.
   Returned radius is normalized — bounds() multiplies by geom.radius for world space. */
function stackedRadiusAt(geom, t, _theta) {
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  let acc = 0;
  for (let k = 0; k < segs.length; k++) {
    const s = segs[k];
    const segFrac = s.height / totalH;
    if (t <= acc + segFrac || k === segs.length - 1) {
      const u = segFrac < 1e-8 ? 0 : (t - acc) / segFrac;
      const fn = STACKED_SHAPES[s.profile] || STACKED_SHAPES.linear;
      return Math.max(0, fn(Math.max(0, Math.min(1, u)), s.r_bot, s.r_top));
    }
    acc += segFrac;
  }
  return 0;
}

/* Cumulative t value at the top of each segment — used for the 2D dashed lines. */
function stackedBoundaries(geom) {
  const totalH = geom.segments.reduce((a, s) => a + s.height, 0) || 1;
  const out = [];
  let acc = 0;
  for (let k = 0; k < geom.segments.length; k++) {
    acc += geom.segments[k].height / totalH;
    out.push({ t: acc, label: STACKED_SEG_NAMES[k] });
  }
  return out;
}

/* Where the geometry has hard radius jumps between adjacent segments — those
   become disc-ring faces in the 3D renderer so the silhouette looks correctly
   stepped instead of an oblique band between mismatched radii. */
function stackedDiscontinuities(geom) {
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  const out = [];
  let acc = 0;
  for (let k = 0; k < segs.length - 1; k++) {
    acc += segs[k].height / totalH;
    const rBelow = segs[k].r_top;
    const rAbove = segs[k + 1].r_bot;
    if (Math.abs(rBelow - rAbove) > 1e-4) {
      out.push({ t: acc, rBelow, rAbove });
    }
  }
  return out;
}

/* Stacked tier defaults — three starter archetypes spanning the design space. */

/* Tier 0 — Stacked: classical column-like baseline. Wide base, slim shaft, mid waist,
   modest collar, conical finial. Cool slate palette. */
function stackedColumnGeom() {
  return {
    height: 1.8, radius: 0.55,
    segments: [
      { height: 0.20, r_bot: 0.95, r_top: 0.70, profile: "linear"  }, // Base
      { height: 0.45, r_bot: 0.55, r_top: 0.45, profile: "linear"  }, // Shaft
      { height: 0.20, r_bot: 0.50, r_top: 0.60, profile: "concave" }, // Waist
      { height: 0.10, r_bot: 0.75, r_top: 0.65, profile: "convex"  }, // Collar
      { height: 0.20, r_bot: 0.50, r_top: 0.05, profile: "linear"  }, // Finial
    ],
  };
}
function stackedColumnStops() {
  return [
    { t: 0.00, c: [0.32, 0.36, 0.42], label: "Base"   },
    { t: 0.25, c: [0.42, 0.46, 0.52], label: "Shaft"  },
    { t: 0.55, c: [0.50, 0.54, 0.60], label: "Waist"  },
    { t: 0.78, c: [0.60, 0.64, 0.70], label: "Collar" },
    { t: 0.95, c: [0.72, 0.76, 0.82], label: "Finial" },
  ];
}

/* Tier 1 — Totem: ornate, alternating bulged and pinched segments, warm ochre/red palette.
   Reads as ceremonial. */
function stackedTotemGeom() {
  return {
    height: 2.2, radius: 0.60,
    segments: [
      { height: 0.18, r_bot: 1.00, r_top: 0.80, profile: "linear"  }, // Base
      { height: 0.30, r_bot: 0.65, r_top: 0.65, profile: "convex"  }, // Shaft (bulged)
      { height: 0.20, r_bot: 0.55, r_top: 0.55, profile: "concave" }, // Waist (pinched)
      { height: 0.18, r_bot: 0.85, r_top: 0.85, profile: "convex"  }, // Collar (bulged)
      { height: 0.14, r_bot: 0.55, r_top: 0.30, profile: "convex"  }, // Finial (rounded cap)
    ],
  };
}
function stackedTotemStops() {
  return [
    { t: 0.00, c: [0.35, 0.18, 0.10], label: "Base"   },
    { t: 0.25, c: [0.65, 0.32, 0.15], label: "Shaft"  },
    { t: 0.50, c: [0.78, 0.55, 0.22], label: "Waist"  },
    { t: 0.72, c: [0.62, 0.30, 0.18], label: "Collar" },
    { t: 0.92, c: [0.85, 0.70, 0.35], label: "Finial" },
  ];
}

/* Tier 2 — Pylon: austere monument. Tall step segments, near-cylindrical, granite palette. */
function stackedPylonGeom() {
  return {
    height: 3.0, radius: 0.50,
    segments: [
      { height: 0.10, r_bot: 1.00, r_top: 0.85, profile: "step"   }, // Base
      { height: 0.50, r_bot: 0.70, r_top: 0.65, profile: "linear" }, // Shaft (long)
      { height: 0.12, r_bot: 0.65, r_top: 0.60, profile: "step"   }, // Waist (band)
      { height: 0.18, r_bot: 0.78, r_top: 0.72, profile: "step"   }, // Collar
      { height: 0.10, r_bot: 0.50, r_top: 0.30, profile: "linear" }, // Finial
    ],
  };
}
function stackedPylonStops() {
  return [
    { t: 0.00, c: [0.28, 0.27, 0.26], label: "Base"   },
    { t: 0.30, c: [0.40, 0.39, 0.38], label: "Shaft"  },
    { t: 0.65, c: [0.48, 0.47, 0.46], label: "Waist"  },
    { t: 0.82, c: [0.55, 0.54, 0.53], label: "Collar" },
    { t: 0.96, c: [0.70, 0.68, 0.66], label: "Finial" },
  ];
}

/* Tier 3 — Bishop: clerical silhouette. Slim shaft, modest collar with a clear shoulder
   step, and a tall conical mitre finial that tapers to a point. Ivory/cream palette. */
function stackedBishopGeom() {
  return {
    height: 2.4, radius: 0.45,
    segments: [
      { height: 0.14, r_bot: 1.00, r_top: 0.78, profile: "linear"  }, // Base
      { height: 0.42, r_bot: 0.50, r_top: 0.40, profile: "concave" }, // Shaft (slim, narrowing)
      { height: 0.06, r_bot: 0.42, r_top: 0.55, profile: "step"    }, // Waist (small ring)
      { height: 0.12, r_bot: 0.62, r_top: 0.50, profile: "convex"  }, // Collar (rounded shoulder)
      { height: 0.30, r_bot: 0.45, r_top: 0.00, profile: "linear"  }, // Finial (mitre — tall cone to point)
    ],
  };
}
function stackedBishopStops() {
  return [
    { t: 0.00, c: [0.78, 0.74, 0.66], label: "Base"   },
    { t: 0.18, c: [0.88, 0.84, 0.76], label: "Shaft"  },
    { t: 0.65, c: [0.92, 0.88, 0.78], label: "Waist"  },
    { t: 0.74, c: [0.95, 0.92, 0.82], label: "Collar" },
    { t: 0.90, c: [1.00, 0.98, 0.90], label: "Finial" },
  ];
}

/* Tier 4 — Queen: regal silhouette. Tall, slim shaft with strong waist pinch, wide
   crown collar, and a bulbous orb finial. Wine/burgundy palette with gold highlights
   at the crown. */
function stackedQueenGeom() {
  return {
    height: 2.8, radius: 0.55,
    segments: [
      { height: 0.14, r_bot: 1.00, r_top: 0.80, profile: "linear"  }, // Base
      { height: 0.36, r_bot: 0.55, r_top: 0.42, profile: "concave" }, // Shaft (slim, pinched mid)
      { height: 0.10, r_bot: 0.40, r_top: 0.38, profile: "concave" }, // Waist (deep pinch)
      { height: 0.18, r_bot: 0.55, r_top: 0.85, profile: "convex"  }, // Collar (broad crown shoulder)
      { height: 0.22, r_bot: 0.75, r_top: 0.00, profile: "convex"  }, // Finial (bulbous orb to point)
    ],
  };
}
function stackedQueenStops() {
  return [
    { t: 0.00, c: [0.20, 0.06, 0.10], label: "Base"   },
    { t: 0.18, c: [0.42, 0.10, 0.18], label: "Shaft"  },
    { t: 0.58, c: [0.55, 0.14, 0.22], label: "Waist"  },
    { t: 0.72, c: [0.85, 0.65, 0.25], label: "Collar" },
    { t: 0.90, c: [0.95, 0.80, 0.35], label: "Finial" },
  ];
}

/* Tier 5 — King: authoritative silhouette. Wide stable base, sturdy shaft, flat-stepped
   collar, broad crown that flares outward with a small finial peak (the abstracted
   cross on top). Ebony palette with subdued gold trim. */
function stackedKingGeom() {
  return {
    height: 3.2, radius: 0.62,
    segments: [
      { height: 0.16, r_bot: 1.00, r_top: 0.85, profile: "step"   }, // Base (wide, stepped)
      { height: 0.38, r_bot: 0.60, r_top: 0.55, profile: "linear" }, // Shaft (sturdy, slight taper)
      { height: 0.08, r_bot: 0.55, r_top: 0.60, profile: "step"   }, // Waist (subtle band)
      { height: 0.20, r_bot: 0.85, r_top: 0.95, profile: "step"   }, // Collar (broad flared crown)
      { height: 0.18, r_bot: 0.45, r_top: 0.00, profile: "linear" }, // Finial (cross — narrow taper)
    ],
  };
}
function stackedKingStops() {
  return [
    { t: 0.00, c: [0.08, 0.07, 0.08], label: "Base"   },
    { t: 0.20, c: [0.14, 0.12, 0.13], label: "Shaft"  },
    { t: 0.62, c: [0.18, 0.16, 0.16], label: "Waist"  },
    { t: 0.74, c: [0.55, 0.42, 0.18], label: "Collar" },
    { t: 0.92, c: [0.78, 0.62, 0.22], label: "Finial" },
  ];
}

/* Tier 6 — Bauhaus: simple-on-purpose. Near-cylindrical segments with no ornament,
   no waist pinch, no collar bulge — just three honest cylinders and a rounded cap.
   The 'less is more' entry. Cool industrial gray palette. */
function stackedBauhausGeom() {
  return {
    height: 1.6, radius: 0.42,
    segments: [
      { height: 0.18, r_bot: 1.00, r_top: 1.00, profile: "step"   }, // Base (flat disc)
      { height: 0.40, r_bot: 0.55, r_top: 0.55, profile: "linear" }, // Shaft (uniform cylinder)
      { height: 0.04, r_bot: 0.55, r_top: 0.55, profile: "linear" }, // Waist (skipped — placeholder ring)
      { height: 0.06, r_bot: 0.65, r_top: 0.65, profile: "step"   }, // Collar (small disc shelf)
      { height: 0.32, r_bot: 0.65, r_top: 0.00, profile: "convex" }, // Finial (rounded dome to point)
    ],
  };
}
function stackedBauhausStops() {
  return [
    { t: 0.00, c: [0.42, 0.44, 0.46], label: "Base"   },
    { t: 0.22, c: [0.55, 0.57, 0.60], label: "Shaft"  },
    { t: 0.62, c: [0.60, 0.62, 0.65], label: "Waist"  },
    { t: 0.68, c: [0.65, 0.67, 0.70], label: "Collar" },
    { t: 0.90, c: [0.78, 0.80, 0.83], label: "Finial" },
  ];
}

const STACKED_TIER_NAMES = ["Stacked", "Totem", "Pylon", "Bishop", "Queen", "King", "Bauhaus"];
const STACKED_GEOM_FNS  = [stackedColumnGeom, stackedTotemGeom, stackedPylonGeom, stackedBishopGeom, stackedQueenGeom, stackedKingGeom, stackedBauhausGeom];
const STACKED_STOPS_FNS = [stackedColumnStops, stackedTotemStops, stackedPylonStops, stackedBishopStops, stackedQueenStops, stackedKingStops, stackedBauhausStops];

const STACKED_FAMILY = {
  name: "Stacked",
  tierNames: STACKED_TIER_NAMES,
  defaultTiers: () => STACKED_TIER_NAMES.map((_, i) => ({
    geom:  STACKED_GEOM_FNS[i](),
    stops: STACKED_STOPS_FNS[i](),
  })),
  radiusAt: stackedRadiusAt,
  bounds:   (geom) => ({ height: geom.height, radius: geom.radius }),
  // Maximum normalized radius across the silhouette — auto-fit uses this to know
  // how wide the piece really is when segments flare beyond the unit envelope.
  envelopeMax: (geom) => {
    let m = 0;
    for (const s of geom.segments) {
      if (s.r_bot > m) m = s.r_bot;
      if (s.r_top > m) m = s.r_top;
    }
    return Math.max(m, 0.0001);
  },
  boundaries: stackedBoundaries,
  discontinuities: stackedDiscontinuities,
};

/* ════════════════════════════════════════════════════════════════════════
   HERALDIC FAMILY — stacked-style body with a chosen ornamental head.

   Body is 4 stacked segments (Base / Shaft / Waist / Collar), reusing the
   same per-segment vocabulary as the Stacked family — height, r_bot, r_top,
   profile shape. No Finial segment because the head occupies that slot.

   Head is one of a small catalog of named decorative shapes:
     - crescent  : fat crescent moon with sharp tips, optionally tilted
     - cleft     : dome with a deep wedge cut into the front (exaggerated bishop slit)
     - notched   : flattened ring with a gap opening forward (stylized crown)
     - forked    : two prongs rising and splaying outward (twin horns / tongue)

   Each head is a self-contained primitive that emits its own triangles via
   extraFaces. The body's `radiusAt` covers t ∈ [0, body_t_top] where
   body_t_top is body_height / total_height; above that the surface revolution
   produces zero radius and the head takes over.

   Geom shape:
     segments      : 4 segments {height, r_bot, r_top, profile} — identical
                     vocabulary to STACKED_SHAPES
     head_kind     : 'crescent' | 'cleft' | 'notched' | 'forked'
     head_size     : float — head's footprint radius as fraction of the
                     collar's r_top (1 = matches collar exactly; >1 overhangs)
     head_height   : float — head's vertical extent in world units
     head_tilt     : float radians — pitch the head forward (+) or back (-)
     head_offset   : float — translate the head forward (along +x) as
                     fraction of collar r_top
     head_params   : { ... } — shape-specific extra parameters; see each
                     head shape function for the fields it consumes
     height        : world-space body height
     radius        : world-space body radius (segments * radius)
   ════════════════════════════════════════════════════════════════════════ */

/* Body has 4 segments — same segment vocabulary as Stacked, but no Finial. */
const HERALDIC_SEG_NAMES = ["Base", "Shaft", "Waist", "Collar"];

/* Body radius lookup. t is normalized over the BODY only (0 at base, 1 at the
   top of the Collar). Above t=1 we return 0 — head takes over via extraFaces. */
function heraldicBodyRadiusAt(geom, t, _theta) {
  if (t > 1) return 0;
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  let acc = 0;
  for (let k = 0; k < segs.length; k++) {
    const s = segs[k];
    const segFrac = s.height / totalH;
    if (t <= acc + segFrac || k === segs.length - 1) {
      const u = segFrac < 1e-8 ? 0 : (t - acc) / segFrac;
      const fn = STACKED_SHAPES[s.profile] || STACKED_SHAPES.linear;
      return Math.max(0, fn(Math.max(0, Math.min(1, u)), s.r_bot, s.r_top));
    }
    acc += segFrac;
  }
  return 0;
}

/* Where in t-space the body region ends. The body fills the body_height
   fraction of the total height; the head sits in the rest. */
function heraldicBodyTop(geom) {
  const totalH = geom.height + geom.head_height;
  return totalH < 1e-8 ? 1 : geom.height / totalH;
}

/* Heraldic radiusAt sees the WHOLE piece — body + head region. We map t to
   body-local-t and call heraldicBodyRadiusAt; for t past the body top, return
   zero (head is rendered by extraFaces, not by surface revolution). */
function heraldicRadiusAt(geom, t, theta) {
  const top = heraldicBodyTop(geom);
  if (t > top) return 0;
  const tBody = top < 1e-8 ? 0 : t / top;
  return heraldicBodyRadiusAt(geom, tBody, theta);
}

/* Body bounds. Total height includes the head; total width is body radius
   times the maximum segment normalized radius. */
function heraldicBounds(geom) {
  const totalH = geom.height + geom.head_height;
  return { height: totalH, radius: geom.radius };
}

function heraldicEnvelopeMax(geom) {
  let m = 0;
  for (const s of geom.segments) {
    if (s.r_bot > m) m = s.r_bot;
    if (s.r_top > m) m = s.r_top;
  }
  // Head can extend the silhouette outward via head_size and head_offset.
  const headExtent = geom.head_size * (geom.segments[3]?.r_top ?? 1) + Math.abs(geom.head_offset);
  return Math.max(m, headExtent, 0.0001);
}

/* Body segment boundaries, in t-space relative to the WHOLE piece (body+head). */
function heraldicBoundaries(geom) {
  const top = heraldicBodyTop(geom);
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  const out = [];
  let acc = 0;
  for (let k = 0; k < segs.length; k++) {
    acc += segs[k].height / totalH;
    out.push({ t: acc * top, label: HERALDIC_SEG_NAMES[k] });
  }
  // Body-top boundary (where the head begins).
  out.push({ t: top, label: "Head base" });
  return out;
}

function heraldicDiscontinuities(geom) {
  const top = heraldicBodyTop(geom);
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  const out = [];
  let acc = 0;
  for (let k = 0; k < segs.length - 1; k++) {
    acc += segs[k].height / totalH;
    const rBelow = segs[k].r_top, rAbove = segs[k + 1].r_bot;
    if (Math.abs(rBelow - rAbove) > 1e-4) {
      out.push({ t: acc * top, rBelow, rAbove });
    }
  }
  return out;
}

/* ── Head shape catalog ─────────────────────────────────────────────────
   Each shape is a function (geom, height, radius, transformVert) → faces[].
   Returns an array of { verts: [v0, v1, v2], col?, t? } where verts are
   already transformed via transformVert. The face builder lights and sorts
   them automatically.

   All four shapes share a coordinate frame: the head sits on top of the
   collar at local-y = bodyTop * height - height * 0.45, with the body axis
   running through (0, 0). Forward is +x. We tilt forward by head_tilt
   radians (rotation about the z-axis) so the whole head leans.
   ──────────────────────────────────────────────────────────────────────── */

/* Helper: smoothstep — clamps to [0,1] and applies the classic 3t² − 2t³ ease.
   Used by the horns path parameterization for slow-start/slow-end interpolation. */
function _smoothstep(x) {
  const t = Math.max(0, Math.min(1, x));
  return t * t * (3 - 2 * t);
}

/* Helper: rotate (x, y, z) about z by angle alpha (so +x leans toward +y when alpha>0). */
function _rotZ(p, sina, cosa) {
  return [p[0] * cosa - p[1] * sina, p[0] * sina + p[1] * cosa, p[2]];
}

/* Shared boilerplate: returns the world-space transform that places head-local
   coordinates onto the body's collar. Handles tilt and forward offset. */
function _headPlacement(geom, height, radius, transformVert) {
  const top = heraldicBodyTop(geom);
  const yBase = top * height - height * 0.45;
  const sina = Math.sin(geom.head_tilt);
  const cosa = Math.cos(geom.head_tilt);
  const xOff = geom.head_offset * (geom.segments[3]?.r_top ?? 1) * radius;
  // Returns a function: head-local (x, y, z) → world point passed through transformVert.
  return (p) => {
    const rotated = _rotZ(p, sina, cosa);
    return transformVert([rotated[0] + xOff, rotated[1] + yBase, rotated[2]]);
  };
}

/* Crescent / horns head — two mirrored swept tubes growing from the head's
   base, sweeping outward and upward, with the tips curling inward. Reads as
   bull horns or stylized crescent moon depending on parameters.

   Each horn is a tube along a 2D path parameterized by u in [0, 1]:
     u=0 : root, planted on the head's collar, x = ±base_separation/2
     u=1 : tip, after sweep + curl

   The path starts at the root, sweeps outward by `spread`, rises by `rise`,
   then curls inward by `tip_curl` over the last 30% of u. Tube cross-section
   radius is `thickness` at the base and tapers to 0 at the tip — sharp points.

   head_params for crescent (horns):
     base_separation : how far apart the two horn roots are (fraction of
                       head_size). 0 = roots meet at one point. (default 0.20)
     spread          : outward sweep distance (fraction of head_size).
                       Higher = wider horns. (default 0.55)
     rise            : vertical rise (fraction of head_height). 1.0 = tips
                       reach the head's full vertical extent. (default 1.0)
     tip_curl        : how much the tips curl back inward in the last 30% of
                       the path. 0 = straight tips; 0.5 = curl halfway back to
                       the centerline; >0.5 = tips converge or cross.
                       (default 0.45)
     thickness       : tube cross-section radius at the root, as fraction of
                       head_size. (default 0.18) */
function _crescentHead(geom, height, radius, transformVert) {
  const place = _headPlacement(geom, height, radius, transformVert);
  const params = geom.head_params || {};
  const baseSep   = params.base_separation != null ? params.base_separation : 0.20;
  const spread    = params.spread          != null ? params.spread          : 0.55;
  const rise      = params.rise            != null ? params.rise            : 1.0;
  const tipCurl   = params.tip_curl        != null ? params.tip_curl        : 0.45;
  const thickness = params.thickness       != null ? params.thickness       : 0.18;

  const collarR = (geom.segments[3]?.r_top ?? 1) * radius;
  const headR   = geom.head_size * collarR;
  const headH   = geom.head_height;
  const out = [];

  const NLEN = 24;  // samples along each horn
  const NCIR = 10;  // samples around tube cross-section

  // For each horn (left = -1, right = +1), build a parametric path in xy.
  // Path components:
  //   - x_base(u) : sweep outward, dominated by spread; tip_curl pulls back inward at the end
  //   - y_base(u) : rises with u, mostly linear with a slight ease-out
  // We use smoothstep for interpolation so the path has continuous tangents.
  for (const dir of [-1, +1]) {
    // Sample arc points + frame.
    const arcPts = [];
    for (let i = 0; i <= NLEN; i++) {
      const u = i / NLEN;
      // Outward sweep follows a smoothstep curve from 0 to 1 (full spread).
      // Curl-back: in the last `curlStart` fraction of u, we pull x back toward
      // the centerline by `tipCurl × spread`. Lerp between (no curl) and
      // (curled) using a smoothstep over that final region.
      const curlStart = 0.65; // u where curl begins
      const curlT = u <= curlStart ? 0 : _smoothstep((u - curlStart) / (1 - curlStart));
      // Outward distance from centerline.
      const outwardSweep = _smoothstep(Math.min(1, u / curlStart));
      const x_offset = (baseSep / 2) + outwardSweep * spread - curlT * tipCurl * spread;
      // Vertical rise: start a bit slow (horns plant outward first), then accelerate.
      // _smoothstep(u) gives the slow-start-slow-end profile that feels right
      // for horn growth.
      const y = _smoothstep(u) * rise * headH;
      // Place the horn point. Tips angle slightly forward-then-up via the
      // tangent direction.
      const x = dir * x_offset * headR;
      // Compute tangent from finite differences of the path. We'll use the
      // analytic version below to avoid losing the endpoints.
      arcPts.push({ u, pos: [x, y, 0] });
    }
    // Tangents from finite differences (central difference where possible).
    const tans = arcPts.map((_, i) => {
      const a = arcPts[Math.max(0, i - 1)].pos;
      const b = arcPts[Math.min(NLEN, i + 1)].pos;
      const dx = b[0] - a[0], dy = b[1] - a[1], dz = b[2] - a[2];
      const l = Math.sqrt(dx*dx + dy*dy + dz*dz) || 1;
      return [dx/l, dy/l, dz/l];
    });

    // Tube radius along the horn: max at u=0 (root), tapers to 0 at u=1.
    // Use a slight ease (1-u)^1.5 so the tip is sharp without the base looking blunt.
    const tubeR = (u) => Math.pow(1 - u, 1.4) * thickness * headR;

    // Build cross-section rings.
    const rings = arcPts.map(({ u, pos }, i) => {
      const r = tubeR(u);
      const tan = tans[i];
      // Build a coordinate frame: tan + perp1 (in xy) + perp2 (z axis).
      const perp1 = [-tan[1], tan[0], 0];
      const ring = [];
      for (let j = 0; j < NCIR; j++) {
        const phi = (j / NCIR) * Math.PI * 2;
        const a = Math.cos(phi) * r;
        const b = Math.sin(phi) * r;
        ring.push([
          pos[0] + perp1[0] * a,
          pos[1] + perp1[1] * a,
          pos[2] + b,
        ]);
      }
      return ring;
    });

    // Stitch consecutive rings into quad strips.
    for (let i = 0; i < NLEN; i++) {
      const a = rings[i], b = rings[i + 1];
      for (let j = 0; j < NCIR; j++) {
        const jn = (j + 1) % NCIR;
        const v0 = place(a[j]);
        const v1 = place(a[jn]);
        const v2 = place(b[jn]);
        const v3 = place(b[j]);
        out.push({ verts: [v0, v1, v2], t: 1 });
        out.push({ verts: [v0, v2, v3], t: 1 });
      }
    }

    // Cap the root (u=0) so the horn doesn't have an open hole at the base.
    // Triangle fan from the centerline point.
    const baseRing = rings[0];
    const baseCenter = place(arcPts[0].pos);
    for (let j = 0; j < NCIR; j++) {
      const jn = (j + 1) % NCIR;
      out.push({ verts: [baseCenter, place(baseRing[jn]), place(baseRing[j])], t: 1 });
    }
  }

  return out;
}

/* Cleft head — dome with a deep wedge cut into the front. Built as a hemisphere
   sliced by a vertical plane offset from center.

   head_params for cleft:
     cleft_depth  : float in [0, 1], how deep the cut goes (1 = all the way through)
     cleft_width  : float in radians, angular width of the cut */
function _cleftHead(geom, height, radius, transformVert) {
  const place = _headPlacement(geom, height, radius, transformVert);
  const params = geom.head_params || {};
  const cleftDepth = params.cleft_depth != null ? params.cleft_depth : 0.55;
  const cleftWidth = params.cleft_width != null ? params.cleft_width : 0.9;

  const collarR = (geom.segments[3]?.r_top ?? 1) * radius;
  const headR   = geom.head_size * collarR;
  const headH   = geom.head_height;
  const out = [];

  // Hemisphere with a wedge removed. Generate vertex grid in spherical coordinates
  // (θ around y-axis, φ from base to top). Skip quads whose center falls inside
  // the cleft wedge.
  const NLON = 32;
  const NLAT = 14;

  const cleftHalf = cleftWidth / 2;
  // Cleft is centered on +x (theta = 0) and pointing inward. The cut goes from the
  // surface inward to depth cleftDepth*headR.
  const inCleft = (theta) => Math.abs(theta) < cleftHalf;

  // Build vertex grid.
  const grid = [];
  for (let j = 0; j <= NLAT; j++) {
    const phi = (j / NLAT) * (Math.PI / 2); // 0 at base, π/2 at top
    const ringY = Math.sin(phi) * headH;
    const ringR = Math.cos(phi) * headR;
    const row = [];
    for (let i = 0; i <= NLON; i++) {
      const theta = (i / NLON) * Math.PI * 2 - Math.PI; // -π to +π
      // Cleft cut: if inside wedge angles, push the vertex inward by cleftDepth.
      const insideCleft = inCleft(theta);
      const r = insideCleft ? ringR * (1 - cleftDepth) : ringR;
      const x = Math.cos(theta) * r;
      const z = Math.sin(theta) * r;
      row.push({ pt: [x, ringY, z], inCleft: insideCleft, theta });
    }
    grid.push(row);
  }

  // Outer surface quads.
  for (let j = 0; j < NLAT; j++) {
    for (let i = 0; i < NLON; i++) {
      const a = grid[j][i].pt;
      const b = grid[j][i + 1].pt;
      const c = grid[j + 1][i + 1].pt;
      const d = grid[j + 1][i].pt;
      // Skip degenerate quads near the apex.
      out.push({ verts: [place(a), place(b), place(c)], t: 1 });
      out.push({ verts: [place(a), place(c), place(d)], t: 1 });
    }
  }

  // Cleft inner walls — left and right "cliff face" of the wedge.
  // Walk from cleft edge to apex, building strips that close the cut.
  for (let j = 0; j < NLAT; j++) {
    // Left edge (theta = -cleftHalf).
    const phiA = (j / NLAT) * (Math.PI / 2);
    const phiB = ((j + 1) / NLAT) * (Math.PI / 2);
    const yA = Math.sin(phiA) * headH;
    const yB = Math.sin(phiB) * headH;
    const rA = Math.cos(phiA) * headR;
    const rB = Math.cos(phiB) * headR;
    for (const sign of [-1, +1]) {
      const theta = sign * cleftHalf;
      const cosT = Math.cos(theta), sinT = Math.sin(theta);
      // Outer point (full radius) and inner point (radius reduced by cleftDepth).
      const outerA = [cosT * rA, yA, sinT * rA];
      const innerA = [cosT * rA * (1 - cleftDepth), yA, sinT * rA * (1 - cleftDepth)];
      const outerB = [cosT * rB, yB, sinT * rB];
      const innerB = [cosT * rB * (1 - cleftDepth), yB, sinT * rB * (1 - cleftDepth)];
      // Wind so the wall faces into the cleft (i.e. outward from the wedge interior).
      if (sign < 0) {
        out.push({ verts: [place(outerA), place(innerA), place(innerB)], t: 1 });
        out.push({ verts: [place(outerA), place(innerB), place(outerB)], t: 1 });
      } else {
        out.push({ verts: [place(outerA), place(innerB), place(innerA)], t: 1 });
        out.push({ verts: [place(outerA), place(outerB), place(innerB)], t: 1 });
      }
    }
  }

  // Cleft floor — connect inner edges (at the bottom of the wedge) across the cut.
  for (let j = 0; j < NLAT; j++) {
    const phiA = (j / NLAT) * (Math.PI / 2);
    const phiB = ((j + 1) / NLAT) * (Math.PI / 2);
    const yA = Math.sin(phiA) * headH;
    const yB = Math.sin(phiB) * headH;
    const rA = Math.cos(phiA) * headR * (1 - cleftDepth);
    const rB = Math.cos(phiB) * headR * (1 - cleftDepth);
    const cosL = Math.cos(-cleftHalf), sinL = Math.sin(-cleftHalf);
    const cosR = Math.cos(+cleftHalf), sinR = Math.sin(+cleftHalf);
    const lA = [cosL * rA, yA, sinL * rA];
    const rA2 = [cosR * rA, yA, sinR * rA];
    const lB = [cosL * rB, yB, sinL * rB];
    const rB2 = [cosR * rB, yB, sinR * rB];
    out.push({ verts: [place(lA), place(rA2), place(rB2)], t: 1 });
    out.push({ verts: [place(lA), place(rB2), place(lB)], t: 1 });
  }

  return out;
}

/* Notched ring head — flattened torus with a gap on the front side. Reads as
   a stylized open crown.

   head_params for notched:
     gap          : float in radians, angular size of the gap (default 1.2)
     ring_thick   : float, ring thickness as fraction of head_size (default 0.25) */
function _notchedHead(geom, height, radius, transformVert) {
  const place = _headPlacement(geom, height, radius, transformVert);
  const params = geom.head_params || {};
  const gap        = params.gap        != null ? params.gap        : 1.2;
  const ringThick  = params.ring_thick != null ? params.ring_thick : 0.25;

  const collarR = (geom.segments[3]?.r_top ?? 1) * radius;
  const headR   = geom.head_size * collarR;
  const headH   = geom.head_height;
  const out = [];

  // Torus: major radius headR (centerline of the ring), minor radius
  // ringThick * headR. The ring has an arc gap centered on +x.
  const RMaj = headR;
  const RMin = ringThick * headR * Math.min(1, headH / Math.max(0.0001, ringThick * headR));
  // Vertical scale: squash so torus height matches headH (let it be compressed).
  const yScale = headH / Math.max(0.0001, RMin * 2);

  const NMAJ = 36;
  const NMIN = 12;

  const gapHalf = gap / 2;
  // Major angles run from gapHalf to 2π - gapHalf — i.e. skip the gap.
  for (let i = 0; i < NMAJ; i++) {
    const u0 = i / NMAJ, u1 = (i + 1) / NMAJ;
    const t0 = gapHalf + u0 * (Math.PI * 2 - gap);
    const t1 = gapHalf + u1 * (Math.PI * 2 - gap);
    for (let j = 0; j < NMIN; j++) {
      const v0 = (j / NMIN) * Math.PI * 2;
      const v1 = ((j + 1) / NMIN) * Math.PI * 2;
      const buildPt = (T, V) => {
        const cT = Math.cos(T), sT = Math.sin(T);
        const cV = Math.cos(V), sV = Math.sin(V);
        const x = (RMaj + RMin * cV) * cT;
        const z = (RMaj + RMin * cV) * sT;
        const y = RMin * sV * yScale + RMin * yScale; // shift so torus sits on y=0
        return [x, y, z];
      };
      const p00 = buildPt(t0, v0);
      const p10 = buildPt(t1, v0);
      const p11 = buildPt(t1, v1);
      const p01 = buildPt(t0, v1);
      out.push({ verts: [place(p00), place(p10), place(p11)], t: 1 });
      out.push({ verts: [place(p00), place(p11), place(p01)], t: 1 });
    }
  }

  // Cap the two ends of the torus where it was cut by the gap.
  for (const tEnd of [gapHalf, Math.PI * 2 - gapHalf]) {
    const cT = Math.cos(tEnd), sT = Math.sin(tEnd);
    for (let j = 0; j < NMIN; j++) {
      const v0 = (j / NMIN) * Math.PI * 2;
      const v1 = ((j + 1) / NMIN) * Math.PI * 2;
      const cV0 = Math.cos(v0), sV0 = Math.sin(v0);
      const cV1 = Math.cos(v1), sV1 = Math.sin(v1);
      const ctr = [RMaj * cT, RMin * yScale, RMaj * sT];
      const p0 = [(RMaj + RMin * cV0) * cT, RMin * sV0 * yScale + RMin * yScale, (RMaj + RMin * cV0) * sT];
      const p1 = [(RMaj + RMin * cV1) * cT, RMin * sV1 * yScale + RMin * yScale, (RMaj + RMin * cV1) * sT];
      // Wind so the cap faces outward (away from the ring interior).
      out.push({ verts: [place(ctr), place(p0), place(p1)], t: 1 });
    }
  }

  return out;
}

/* Forked head — two prongs rising from a base, splaying outward. Reads as
   serpent's tongue / ram's horns.

   head_params for forked:
     prong_angle    : float radians, how far apart the prongs splay (default 0.5)
     prong_thick    : float, prong base thickness as fraction of head_size (default 0.25)
     prong_taper    : float in [0, 1], fraction of prong base width at the tip (default 0.1) */
function _forkedHead(geom, height, radius, transformVert) {
  const place = _headPlacement(geom, height, radius, transformVert);
  const params = geom.head_params || {};
  const prongAngle = params.prong_angle != null ? params.prong_angle : 0.5;
  const prongThick = params.prong_thick != null ? params.prong_thick : 0.25;
  const prongTaper = params.prong_taper != null ? params.prong_taper : 0.10;

  const collarR = (geom.segments[3]?.r_top ?? 1) * radius;
  const headR   = geom.head_size * collarR;
  const headH   = geom.head_height;
  const out = [];

  // Each prong: a tapered rectangular cross-section, swept along an arc that
  // splays outward by prong_angle from vertical. Two prongs, mirrored across
  // the y-axis. We approximate each as a stack of horizontal cross-sections.
  const NSLICE = 10;
  const NSIDE = 8; // sides of each prong cross-section

  for (const dir of [-1, +1]) {
    // Prong sweeps from straight up at the base to (dir * prong_angle) at the tip.
    const slabs = [];
    for (let i = 0; i <= NSLICE; i++) {
      const v = i / NSLICE;
      // Parametric path of the prong's centerline.
      const angle = v * prongAngle * dir;
      const cx = Math.sin(angle) * headH * 0.7;     // x offset increases as we tilt
      const cy = v * headH;
      // Cross-section radius: tapers from prongThick at base to prongThick*prongTaper at tip.
      const xr = headR * prongThick * (1 - v * (1 - prongTaper));
      const slab = [];
      for (let s = 0; s < NSIDE; s++) {
        const phi = (s / NSIDE) * Math.PI * 2;
        const lx = Math.cos(phi) * xr;
        const lz = Math.sin(phi) * xr;
        slab.push([cx + lx, cy, lz]);
      }
      slabs.push(slab);
    }
    // Connect slab i to i+1 with quads.
    for (let i = 0; i < NSLICE; i++) {
      const a = slabs[i], b = slabs[i + 1];
      for (let s = 0; s < NSIDE; s++) {
        const sn = (s + 1) % NSIDE;
        out.push({ verts: [place(a[s]), place(a[sn]), place(b[sn])], t: 1 });
        out.push({ verts: [place(a[s]), place(b[sn]), place(b[s])], t: 1 });
      }
    }
    // Tip cap (small, points outward).
    const tipSlab = slabs[NSLICE];
    let tcx = 0, tcy = 0, tcz = 0;
    for (const p of tipSlab) { tcx += p[0]; tcy += p[1]; tcz += p[2]; }
    tcx /= tipSlab.length; tcy /= tipSlab.length; tcz /= tipSlab.length;
    const tipCenter = place([tcx, tcy, tcz]);
    for (let s = 0; s < NSIDE; s++) {
      const sn = (s + 1) % NSIDE;
      out.push({ verts: [tipCenter, place(tipSlab[sn]), place(tipSlab[s])], t: 1 });
    }
  }

  return out;
}

const HERALDIC_HEAD_KINDS = {
  crescent: _crescentHead,
  cleft:    _cleftHead,
  notched:  _notchedHead,
  forked:   _forkedHead,
};

function heraldicExtraFaces(geom, height, radius, transformVert) {
  const fn = HERALDIC_HEAD_KINDS[geom.head_kind] || _crescentHead;
  const headFaces = fn(geom, height, radius, transformVert);

  // Always emit a horizontal disc at the body's top to close the collar.
  // Without this, the body has an open ring at its peak and the head doesn't
  // necessarily cover it (e.g. head_size < 1 leaves the platform exposed).
  const top = heraldicBodyTop(geom);
  const yTop = top * height - height * 0.45;
  const collarR = (geom.segments[3]?.r_top ?? 1) * radius;
  const NSEG = 32;
  const platform = [];
  if (collarR > 1e-4) {
    const centre = transformVert([0, yTop, 0]);
    for (let s = 0; s < NSEG; s++) {
      const a0 = (s / NSEG) * Math.PI * 2;
      const a1 = ((s + 1) % NSEG) / NSEG * Math.PI * 2;
      const v1 = transformVert([Math.cos(a0) * collarR, yTop, Math.sin(a0) * collarR]);
      const v2 = transformVert([Math.cos(a1) * collarR, yTop, Math.sin(a1) * collarR]);
      // Wound CCW viewed from above so the normal points up.
      platform.push({ verts: [centre, v1, v2], t: top });
    }
  }
  return [...platform, ...headFaces];
}

/* Tier defaults — 7 archetypes spanning body sizes (Pawn-like → Colossal-like)
   and head shapes. Body proportions deliberately echo the smooth family's Pawn
   (1.5×0.5, dramatic narrow waist, modest collar bulge) and Colossal (3.5×0.75,
   extreme waist pinch, prominent collar bulge, big head). The heraldic
   4-segment layout maps to these:
     Base   : wide convex flare (analog of smooth's start→flare→peak lip)
     Shaft  : tapers from body_start_r down to the waist (concave)
     Waist  : short narrow pinch
     Collar : widens back outward (the collar bulge)
   Then the chosen head shape sits on top. */

/* Tier 0 — Crescent Pawn: pawn body + horns crown. */
function heraldicCrescentPawnGeom() {
  return {
    segments: [
      { height: 0.20, r_bot: 0.95, r_top: 0.70, profile: "convex"  }, // wide flared base → body_start
      { height: 0.30, r_bot: 0.70, r_top: 0.28, profile: "concave" }, // shaft tapers to narrow waist
      { height: 0.10, r_bot: 0.24, r_top: 0.22, profile: "concave" }, // waist (very narrow)
      { height: 0.18, r_bot: 0.40, r_top: 0.45, profile: "convex"  }, // collar bulges outward
    ],
    head_kind: "crescent", head_size: 1.4, head_height: 0.45,
    head_tilt: -0.20, head_offset: 0.0,
    head_params: { base_separation: 0.20, spread: 0.55, rise: 1.0, tip_curl: 0.45, thickness: 0.18 },
    height: 1.05, radius: 0.50,
  };
}

/* Tier 1 — Cleft Bishop: tall slim pawn body, exaggerated cleft crown. */
function heraldicCleftBishopGeom() {
  return {
    segments: [
      { height: 0.18, r_bot: 0.90, r_top: 0.65, profile: "convex"  },
      { height: 0.34, r_bot: 0.60, r_top: 0.22, profile: "concave" },
      { height: 0.10, r_bot: 0.20, r_top: 0.18, profile: "concave" },
      { height: 0.16, r_bot: 0.36, r_top: 0.40, profile: "convex"  },
    ],
    head_kind: "cleft", head_size: 1.05, head_height: 0.55,
    head_tilt: 0.0, head_offset: 0.0,
    head_params: { cleft_depth: 0.55, cleft_width: 0.7 },
    height: 1.35, radius: 0.45,
  };
}

/* Tier 2 — Crescent Colossus: Colossal-flavor body (extreme waist, big collar) + horns. */
function heraldicCrescentColossusGeom() {
  return {
    segments: [
      { height: 0.18, r_bot: 1.00, r_top: 0.78, profile: "convex"  }, // monumental base
      { height: 0.42, r_bot: 0.72, r_top: 0.18, profile: "concave" }, // long shaft, dramatic taper
      { height: 0.10, r_bot: 0.16, r_top: 0.14, profile: "concave" }, // extreme pinch
      { height: 0.20, r_bot: 0.42, r_top: 0.50, profile: "convex"  }, // big collar bulge
    ],
    head_kind: "crescent", head_size: 1.6, head_height: 0.65,
    head_tilt: -0.25, head_offset: 0.0,
    head_params: { base_separation: 0.25, spread: 0.65, rise: 1.1, tip_curl: 0.50, thickness: 0.22 },
    height: 1.95, radius: 0.65,
  };
}

/* Tier 3 — Notched Sovereign: solid wider pawn body + notched crown. */
function heraldicNotchedSovereignGeom() {
  return {
    segments: [
      { height: 0.20, r_bot: 0.98, r_top: 0.78, profile: "convex"  },
      { height: 0.32, r_bot: 0.74, r_top: 0.30, profile: "concave" },
      { height: 0.10, r_bot: 0.28, r_top: 0.26, profile: "concave" },
      { height: 0.20, r_bot: 0.48, r_top: 0.55, profile: "convex"  }, // strong collar (royal)
    ],
    head_kind: "notched", head_size: 1.10, head_height: 0.40,
    head_tilt: 0.0, head_offset: 0.0,
    head_params: { gap: 1.4, ring_thick: 0.28 },
    height: 1.45, radius: 0.62,
  };
}

/* Tier 4 — Forked Druid: pawn body, twin-prong head. */
function heraldicForkedDruidGeom() {
  return {
    segments: [
      { height: 0.20, r_bot: 0.92, r_top: 0.70, profile: "convex"  },
      { height: 0.30, r_bot: 0.68, r_top: 0.24, profile: "concave" },
      { height: 0.10, r_bot: 0.22, r_top: 0.20, profile: "concave" },
      { height: 0.16, r_bot: 0.38, r_top: 0.42, profile: "convex"  },
    ],
    head_kind: "forked", head_size: 1.0, head_height: 0.55,
    head_tilt: 0.0, head_offset: 0.0,
    head_params: { prong_angle: 0.55, prong_thick: 0.22, prong_taper: 0.08 },
    height: 1.30, radius: 0.52,
  };
}

/* Tier 5 — Cleft Monolith: Colossal-flavor body, narrower cleft. */
function heraldicCleftMonolithGeom() {
  return {
    segments: [
      { height: 0.18, r_bot: 1.00, r_top: 0.80, profile: "convex"  },
      { height: 0.42, r_bot: 0.74, r_top: 0.18, profile: "concave" },
      { height: 0.10, r_bot: 0.16, r_top: 0.14, profile: "concave" },
      { height: 0.20, r_bot: 0.42, r_top: 0.48, profile: "convex"  },
    ],
    head_kind: "cleft", head_size: 1.05, head_height: 0.55,
    head_tilt: 0.0, head_offset: 0.0,
    head_params: { cleft_depth: 0.55, cleft_width: 0.55 },
    height: 2.05, radius: 0.70,
  };
}

/* Tier 6 — Crescent Acolyte: small pawn, small horns crown. */
function heraldicCrescentAcolyteGeom() {
  return {
    segments: [
      { height: 0.20, r_bot: 0.90, r_top: 0.62, profile: "convex"  },
      { height: 0.28, r_bot: 0.58, r_top: 0.22, profile: "concave" },
      { height: 0.08, r_bot: 0.20, r_top: 0.18, profile: "concave" },
      { height: 0.14, r_bot: 0.34, r_top: 0.38, profile: "convex"  },
    ],
    head_kind: "crescent", head_size: 1.25, head_height: 0.32,
    head_tilt: -0.20, head_offset: 0.0,
    head_params: { base_separation: 0.18, spread: 0.50, rise: 0.95, tip_curl: 0.55, thickness: 0.16 },
    height: 0.90, radius: 0.42,
  };
}

/* Color stops — one per tier. Each provides a base→head gradient that suits the
   tier's intended palette. */
function heraldicCrescentPawnStops() {
  return [
    { t: 0.00, c: [0.30, 0.32, 0.36], label: "Base"   },
    { t: 0.20, c: [0.55, 0.55, 0.58], label: "Shaft"  },
    { t: 0.55, c: [0.72, 0.70, 0.70], label: "Waist"  },
    { t: 0.78, c: [0.85, 0.82, 0.78], label: "Collar" },
    { t: 1.00, c: [0.96, 0.94, 0.88], label: "Head"   },
  ];
}
function heraldicCleftBishopStops() {
  return [
    { t: 0.00, c: [0.78, 0.74, 0.66], label: "Base"   },
    { t: 0.25, c: [0.88, 0.84, 0.76], label: "Shaft"  },
    { t: 0.55, c: [0.92, 0.88, 0.78], label: "Waist"  },
    { t: 0.74, c: [0.95, 0.92, 0.82], label: "Collar" },
    { t: 1.00, c: [1.00, 0.98, 0.90], label: "Head"   },
  ];
}
function heraldicCrescentColossusStops() {
  return [
    { t: 0.00, c: [0.10, 0.09, 0.09], label: "Base"   },
    { t: 0.20, c: [0.16, 0.13, 0.11], label: "Shaft"  },
    { t: 0.55, c: [0.22, 0.18, 0.14], label: "Waist"  },
    { t: 0.78, c: [0.55, 0.42, 0.16], label: "Collar" },
    { t: 1.00, c: [0.85, 0.65, 0.25], label: "Head"   },
  ];
}
function heraldicNotchedSovereignStops() {
  return [
    { t: 0.00, c: [0.08, 0.10, 0.20], label: "Base"   },
    { t: 0.22, c: [0.15, 0.20, 0.40], label: "Shaft"  },
    { t: 0.55, c: [0.22, 0.30, 0.55], label: "Waist"  },
    { t: 0.78, c: [0.30, 0.40, 0.65], label: "Collar" },
    { t: 1.00, c: [0.92, 0.78, 0.30], label: "Head"   },
  ];
}
function heraldicForkedDruidStops() {
  return [
    { t: 0.00, c: [0.18, 0.22, 0.10], label: "Base"   },
    { t: 0.22, c: [0.25, 0.38, 0.18], label: "Shaft"  },
    { t: 0.55, c: [0.32, 0.50, 0.22], label: "Waist"  },
    { t: 0.78, c: [0.45, 0.62, 0.30], label: "Collar" },
    { t: 1.00, c: [0.60, 0.75, 0.40], label: "Head"   },
  ];
}
function heraldicCleftMonolithStops() {
  return [
    { t: 0.00, c: [0.22, 0.22, 0.22], label: "Base"   },
    { t: 0.22, c: [0.30, 0.30, 0.30], label: "Shaft"  },
    { t: 0.60, c: [0.40, 0.40, 0.40], label: "Waist"  },
    { t: 0.80, c: [0.50, 0.50, 0.50], label: "Collar" },
    { t: 1.00, c: [0.65, 0.64, 0.62], label: "Head"   },
  ];
}
function heraldicCrescentAcolyteStops() {
  return [
    { t: 0.00, c: [0.30, 0.20, 0.08], label: "Base"   },
    { t: 0.22, c: [0.55, 0.40, 0.18], label: "Shaft"  },
    { t: 0.55, c: [0.72, 0.55, 0.25], label: "Waist"  },
    { t: 0.78, c: [0.82, 0.65, 0.30], label: "Collar" },
    { t: 1.00, c: [0.95, 0.82, 0.42], label: "Head"   },
  ];
}

const HERALDIC_TIER_NAMES = [
  "Crescent Pawn", "Cleft Bishop", "Crescent Colossus",
  "Notched Sovereign", "Forked Druid", "Cleft Monolith", "Crescent Acolyte",
];
const HERALDIC_GEOM_FNS  = [
  heraldicCrescentPawnGeom, heraldicCleftBishopGeom, heraldicCrescentColossusGeom,
  heraldicNotchedSovereignGeom, heraldicForkedDruidGeom, heraldicCleftMonolithGeom,
  heraldicCrescentAcolyteGeom,
];
const HERALDIC_STOPS_FNS = [
  heraldicCrescentPawnStops, heraldicCleftBishopStops, heraldicCrescentColossusStops,
  heraldicNotchedSovereignStops, heraldicForkedDruidStops, heraldicCleftMonolithStops,
  heraldicCrescentAcolyteStops,
];

const HERALDIC_FAMILY = {
  name: "Heraldic",
  tierNames: HERALDIC_TIER_NAMES,
  defaultTiers: () => HERALDIC_TIER_NAMES.map((_, i) => ({
    geom:  HERALDIC_GEOM_FNS[i](),
    stops: HERALDIC_STOPS_FNS[i](),
  })),
  radiusAt: heraldicRadiusAt,
  bounds: heraldicBounds,
  envelopeMax: heraldicEnvelopeMax,
  boundaries: heraldicBoundaries,
  discontinuities: heraldicDiscontinuities,
  extraFaces: heraldicExtraFaces,
};

const FAMILIES = {
  smooth:   SMOOTH_FAMILY,
  stacked:  STACKED_FAMILY,
  heraldic: HERALDIC_FAMILY,
};
const FAMILY_KEYS = ["smooth", "stacked", "heraldic"];

/* ── Defaults across all families — full state shape for first launch ── */
function defaultAllFamilies() {
  return {
    smooth:   SMOOTH_FAMILY.defaultTiers(),
    stacked:  STACKED_FAMILY.defaultTiers(),
    heraldic: HERALDIC_FAMILY.defaultTiers(),
  };
}
const DEFAULTS_JSON = JSON.stringify(defaultAllFamilies());

export default function PawnDesigner() {
  /* ── Family-aware state ──
     `families` is the full saved state across both families:
       { smooth: [tier0, tier1, ...], stacked: [tier0, tier1, ...] }
     `familyKey` is which family is currently active in the UI.
     `tierIdx` is which tier within the active family is being edited.
     The camera (rotY/tilt/zoom/panX/panY) is family-agnostic and persists across
     family switches, which makes A/B comparison feel continuous. */
  const [families, setFamilies] = useState(defaultAllFamilies);
  const [familyKey, setFamilyKey] = useState("smooth");
  const [tierIdx, setTierIdx] = useState(0);
  const [loaded, setLoaded] = useState(false);
  const family = FAMILIES[familyKey];
  const tiers  = families[familyKey];
  const T      = tiers[tierIdx];
  const geom   = T.geom;
  const stops  = T.stops;
  const isModified = JSON.stringify(families) !== DEFAULTS_JSON;
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

  /* ── Mutators that target the active family + tier ──
     Each preserves the rest of the families state untouched. */

  // Mutate the current tier's geom via a callback; deep-copy to avoid mutation hazards.
  const upd = fn => setFamilies(prev => {
    const fam = prev[familyKey].map(t => ({ geom: { ...t.geom }, stops: t.stops }));
    fn(fam[tierIdx].geom);
    return { ...prev, [familyKey]: fam };
  });
  // Same as `upd`, but also deep-copies the segments array so callers can mutate
  // segment[i].field directly without aliasing the previous state.
  const updSegments = fn => setFamilies(prev => {
    const fam = prev[familyKey].map(t => ({
      geom: {
        ...t.geom,
        segments: (t.geom.segments || []).map(s => ({ ...s })),
      },
      stops: t.stops,
    }));
    fn(fam[tierIdx].geom);
    return { ...prev, [familyKey]: fam };
  });
  // Update a single color stop in the current tier.
  const updStop = (idx, color) => setFamilies(prev => ({
    ...prev,
    [familyKey]: prev[familyKey].map((t, i) =>
      i === tierIdx ? { ...t, stops: t.stops.map((s, si) => si === idx ? { ...s, c: color } : s) } : t
    ),
  }));
  // Wholesale-replace the current tier's stops (used by gradient presets).
  const setCurrentStops = (newStops) => setFamilies(prev => ({
    ...prev,
    [familyKey]: prev[familyKey].map((t, i) =>
      i === tierIdx ? { ...t, stops: newStops } : t
    ),
  }));
  // Reset just the current tier to its defaults.
  const resetCurrentTier = () => setFamilies(prev => {
    const defaults = FAMILIES[familyKey].defaultTiers();
    return {
      ...prev,
      [familyKey]: prev[familyKey].map((t, i) => i === tierIdx ? defaults[tierIdx] : t),
    };
  });
  // Reset the entire active family to defaults.
  const resetCurrentFamily = () => setFamilies(prev => ({
    ...prev,
    [familyKey]: FAMILIES[familyKey].defaultTiers(),
  }));

  // Load saved state on mount. Migration order, oldest → newest:
  //   1. v0: single-pawn payload  {prof, stops}                  → smooth tier 0
  //   2. v1: smooth-tier array    [{prof,stops},...]            → smooth family
  //   3. v1+: families object     {smooth:[{geom,stops}...]}     → as-is
  // Any unrecognized shape falls through to defaults.
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) {
          const data = JSON.parse(raw);

          if (data && typeof data === "object" && !Array.isArray(data)
              && (data.smooth || data.stacked)) {
            // Families-shaped payload — current schema.
            const merged = defaultAllFamilies();
            for (const fk of FAMILY_KEYS) {
              const incoming = data[fk];
              if (Array.isArray(incoming)) {
                merged[fk] = merged[fk].map((d, i) => {
                  const e = incoming[i];
                  if (!e || !e.geom || !Array.isArray(e.stops)) return d;
                  return e;
                });
              }
            }
            setFamilies(merged);
          } else if (Array.isArray(data) && data.every(t => t?.prof && Array.isArray(t?.stops))) {
            // Legacy v1 smooth-tier array. `prof` was renamed to `geom`.
            setFamilies(prev => {
              const all = defaultAllFamilies();
              all.smooth = all.smooth.map((d, i) => data[i]
                ? { geom: data[i].prof, stops: data[i].stops }
                : d);
              return all;
            });
          } else if (data?.prof && Array.isArray(data?.stops)) {
            // Legacy v0 single-pawn payload — promote into smooth tier 0.
            setFamilies(prev => {
              const all = defaultAllFamilies();
              all.smooth[0] = { geom: data.prof, stops: data.stops };
              return all;
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
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(families)); } catch (e) {} })();
  }, [families, loaded]);

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
    if (compareMode) render3DCompare(ctx, W, H, family, tiers, rotY, tilt, zoom, panX, panY);
    else             render3D(ctx, W, H, family, geom, stops, rotY, tilt, zoom, panX, panY);
  }, [family, tiers, tierIdx, compareMode, geom, stops, rotY, tilt, zoom, panX, panY, resizeTick]);

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
    if (compareMode) render2DCompare(ctx, W, H, family, tiers);
    else             render2D(ctx, W, H, family, geom, stops);
  }, [familyKey, family, tiers, tierIdx, compareMode, geom, stops, resizeTick]);

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

  // Switch active family. Clamp the tier index in case the destination family
  // has fewer tiers. Compare mode is intentionally preserved across switches:
  // the compare view auto-routes through the active family's tiers, so flipping
  // smooth↔stacked while in compare just re-renders the scene with the other
  // family's lineup — which is exactly the comparison the user is making.
  const switchFamily = (fk) => {
    setFamilyKey(fk);
    const n = FAMILIES[fk].tierNames.length;
    if (tierIdx >= n) setTierIdx(0);
  };

  // WGSL export currently only emits for the smooth family. Stacked geometry needs a
  // different exporter (mesh-side rather than shader-side), which we'll wire up later
  // per the user's deferred-export plan.
  const wgslCode = familyKey === "smooth" ? generateWGSL(geom, stops) : "// WGSL export for the Stacked family is not implemented yet.\n";

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  const tierNames = family.tierNames;

  return (
    <div style={{ fontFamily: "'JetBrains Mono', 'SF Mono', 'Fira Code', monospace", color: "var(--color-text-primary)", lineHeight: 1.4, position: "relative", fontSize: 11 }}>
      {/* ── TOP BAR ── */}
      <div style={{ ...rw, marginBottom: 6, padding: "2px 0" }}>
        <span style={{ fontSize: 12, fontWeight: 600, letterSpacing: "0.02em" }}>7T Pawn Designer</span>
        {isModified && <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 6 }}>● modified</span>}
        <div style={{ marginLeft: "auto", display: "flex", gap: 4 }}>
          <button onClick={() => { setDockKey(k => k + 1); setPanelOrder(defaultOrder); }} style={btnStyle}>Dock all</button>
          <button onClick={() => setFamilies(defaultAllFamilies())} style={btnStyle} title="Reset every tier in every family">Reset all</button>
          <button onClick={resetCurrentFamily} style={btnStyle} title={`Reset every tier in ${family.name}`}>Reset family</button>
          {!compareMode && <button onClick={resetCurrentTier} style={btnStyle}>Reset tier</button>}
          <button onClick={() => {
            const r = (v, d=4) => +(+v).toFixed(d);
            const a = arr => "[" + arr.map(v => r(v)).join(", ") + "]";
            let s = "=== 7T PAWN DESIGNER STATE ===\n\n";
            for (const fk of FAMILY_KEYS) {
              const fam = FAMILIES[fk];
              s += `╔═══ FAMILY: ${fam.name} ═══╗\n\n`;
              families[fk].forEach((tier, ti) => {
                s += `── Tier ${ti}: ${fam.tierNames[ti]} ──\n`;
                s += "  geom:\n";
                Object.entries(tier.geom).forEach(([k, v]) => {
                  if (k === "segments") {
                    s += `    segments:\n`;
                    v.forEach((seg, si) => {
                      s += `      [${si}] ${STACKED_SEG_NAMES[si] || ""}: h=${r(seg.height)} r_bot=${r(seg.r_bot)} r_top=${r(seg.r_top)} profile=${seg.profile}\n`;
                    });
                  } else if (typeof v === "number") {
                    s += `    ${k}: ${r(v)}\n`;
                  } else {
                    s += `    ${k}: ${v}\n`;
                  }
                });
                s += "  stops:\n";
                tier.stops.forEach((st) => { s += `    ${st.label} (t=${r(st.t)}): rgb(${a(st.c)})\n`; });
                s += "\n";
              });
            }
            s += "── JSON (for import) ──\n";
            s += JSON.stringify(families, null, 2) + "\n";
            copyText(s); (() => {
              const btn = document.activeElement;
              if (btn) { const orig = btn.textContent; btn.textContent = "Copied!"; setTimeout(() => { btn.textContent = orig; }, 1200); }
            })();
          }} style={{ ...btnStyle, border: "1px solid var(--color-border-success)", color: "var(--color-text-success)", background: "transparent" }}>Save</button>
        </div>
      </div>

      {/* ── FAMILY SWITCHER ── */}
      <div style={{ display: "flex", gap: 3, marginBottom: 4, alignItems: "center" }}>
        <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginRight: 4, letterSpacing: "0.04em", textTransform: "uppercase" }}>Family</span>
        {FAMILY_KEYS.map(fk => {
          const active = fk === familyKey;
          return (
            <button key={fk} onClick={() => switchFamily(fk)} style={{
              fontSize: 10, padding: "3px 10px", borderRadius: 4, cursor: "pointer",
              border: active ? "2px solid var(--color-border-success)" : "1px solid var(--color-border-tertiary)",
              background: active ? "var(--color-background-success)" : "var(--color-background-secondary)",
              color: active ? "var(--color-text-success)" : "var(--color-text-secondary)",
              fontWeight: active ? 600 : 400,
            }}>{FAMILIES[fk].name}</button>
          );
        })}
      </div>

      {/* ── TIER TABS ── */}
      <div style={{ display: "flex", gap: 3, marginBottom: 6, flexWrap: "wrap" }}>
        {tierNames.map((name, i) => {
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
        }} title={`Show all ${family.name} tiers side-by-side at shared scale`}>Compare</button>
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
          >{family.name} · {compareMode ? "Compare" : tierNames[tierIdx]} · {Math.round(zoom * 100)}%</div>
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
        {familyKey === "smooth" && (
          <div style={{ display: "flex", gap: 4, marginBottom: 6, flexWrap: "wrap" }}>
            {Object.keys(PRESETS).map(name =>
              <button key={name} onClick={() => setCurrentStops(PRESETS[name]())} style={{ ...btnStyle, fontSize: 9 }}>{name}</button>
            )}
          </div>
        )}
        {stops.map((s, i) => <ColorStopRow key={i} stop={s} index={i} onChange={updStop} />)}
      </DragPanel>);

        case "profile": return (
      <DragPanel {...dp} title={
        familyKey === "smooth"  ? "Profile Geometry"
        : familyKey === "stacked" ? "Stacked Segments"
        : "Heraldic Geometry"
      }>
        {familyKey === "smooth" ? (
          <SmoothProfilePanel geom={geom} upd={upd} />
        ) : familyKey === "stacked" ? (
          <StackedProfilePanel geom={geom} upd={upd} updSegments={updSegments} />
        ) : (
          <HeraldicProfilePanel geom={geom} upd={upd} updSegments={updSegments} />
        )}
      </DragPanel>);

        case "export": return (
      <DragPanel {...dp} title={familyKey === "smooth" ? "WGSL Export" : "Export (deferred)"}>
        <div style={{ display: "flex", gap: 4, marginBottom: 4 }}>
          <button onClick={() => { copyText(wgslCode); setCopied(true); setTimeout(() => setCopied(false), 1500); }} style={{ ...btnStyle, background: copied ? "var(--color-background-success)" : "var(--color-background-secondary)" }}>
            {copied ? "Copied" : "Copy"}
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

/* ═══ SMOOTH-FAMILY PROFILE PANEL ═══ */
function SmoothProfilePanel({ geom, upd }) {
  return (
    <>
      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>Section boundaries (t)</div>
      <div style={rw}>
        <span style={lb} title="Where the flared base finishes and the body begins">Base end</span>
        <Num value={geom.base_t} min={0.05} max={0.3} w={40} onChange={v => upd(n => { n.base_t = v; })}
             tip="Height (0–1) where the base ends and the body starts. Higher → taller base." />
        <span style={lb} title="Where the body finishes and the neck taper starts">Body end</span>
        <Num value={geom.body_t} min={0.2} max={0.65} w={40} onChange={v => upd(n => { n.body_t = v; })}
             tip="Height (0–1) where the body ends and the neck pinch starts. Higher → longer body, shorter neck region." />
        <span style={lb} title="Where the neck pinch reaches its narrowest point">Neck end</span>
        <Num value={geom.neck_t} min={0.35} max={0.75} w={40} onChange={v => upd(n => { n.neck_t = v; })}
             tip="Height (0–1) where the neck taper bottoms out. Between body_t and collar_t." />
      </div>
      <div style={rw}>
        <span style={lb} title="Where the collar swell ends and the head begins">Collar end</span>
        <Num value={geom.collar_t} min={0.5} max={0.85} w={40} onChange={v => upd(n => { n.collar_t = v; })}
             tip="Height (0–1) where the collar swell ends and the head section starts." />
        <span style={lb} title="Where the rounded head finishes — above this is the tip taper">Head end</span>
        <Num value={geom.head_t} min={0.7} max={0.99} w={40} onChange={v => upd(n => { n.head_t = v; })}
             tip="Height (0–1) where the spherical head ends. Above this the silhouette tapers to the tip." />
      </div>
      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Key radii (normalized)</div>
      <div style={rw}>
        <span style={lb} title="Radius right at the bottom">Start</span>
        <Num value={geom.start_r} min={0.1} max={1} w={38} onChange={v => upd(n => { n.start_r = v; })}
             tip="Radius at t=0 (the very bottom). Smaller → narrower foot." />
        <span style={lb} title="Radius at the bottom of the flare lip">Flare</span>
        <Num value={geom.flare_r} min={0.3} max={1} w={38} onChange={v => upd(n => { n.flare_r = v; })}
             tip="Radius at the bottom of the flared base lip — usually slightly less than peak_r." />
        <span style={lb} title="Widest point of the base flare">Peak</span>
        <Num value={geom.peak_r} min={0.3} max={1.2} w={38} onChange={v => upd(n => { n.peak_r = v; })}
             tip="The widest point of the base flare. Most pieces have this as their max radius." />
      </div>
      <div style={rw}>
        <span style={lb} title="Radius where the body proper starts (after the base lip)">Body start</span>
        <Num value={geom.body_start_r} min={0.1} max={1} w={38} onChange={v => upd(n => { n.body_start_r = v; })}
             tip="Radius at the start of the body, just above the base. Sets how wide the body trunk is." />
        <span style={lb} title="Radius at the narrowest point of the neck">Waist</span>
        <Num value={geom.waist_r} min={0.05} max={0.6} w={38} onChange={v => upd(n => { n.waist_r = v; })}
             tip="Radius at the body's mid-point. Lower → more pinched waist (Colossal-like). Higher → barrel body." />
        <span style={lb} title="Radius at the narrowest neck pinch">Neck</span>
        <Num value={geom.neck_r} min={0.05} max={0.4} w={38} onChange={v => upd(n => { n.neck_r = v; })}
             tip="Radius at the neck pinch (between body and head). Smaller → more dramatic head transition." />
      </div>
      <div style={rw}>
        <span style={lb} title="How much the collar bulges out beyond the neck">Collar bulge</span>
        <Num value={geom.collar_bulge} min={0} max={0.3} w={38} onChange={v => upd(n => { n.collar_bulge = v; })}
             tip="Extra radius added at the collar (a ring above the neck pinch). 0 → flat. Higher → pronounced collar shoulder." />
        <span style={lb} title="Radius where the head's spherical region starts">Head base</span>
        <Num value={geom.head_base_r} min={0.05} max={0.4} w={38} onChange={v => upd(n => { n.head_base_r = v; })}
             tip="Radius where the rounded head begins, above the collar." />
        <span style={lb} title="Radius at the widest part of the rounded head">Head sphere</span>
        <Num value={geom.head_sphere_r} min={0.1} max={0.6} w={38} onChange={v => upd(n => { n.head_sphere_r = v; })}
             tip="Radius of the spherical head at its widest. Higher → bulkier head (Idol-like)." />
      </div>
      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Scale (world units)</div>
      <div style={rw}>
        <span style={lb} title="Total height of the piece in world units">Height</span>
        <Num value={geom.height} min={0.5} max={10} w={40} onChange={v => upd(n => { n.height = v; })}
             tip="Total piece height in world units. WGSL default is 1.5." />
        <span style={lb} title="Outer radius of the piece in world units">Radius</span>
        <Num value={geom.radius} min={0.1} max={4} w={40} onChange={v => upd(n => { n.radius = v; })}
             tip="Outer radius of the piece in world units. Multiplied with the normalized radii above to get final size. WGSL default is 0.5." />
      </div>
      <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>WGSL defaults: height=1.5, radius=0.5</div>
    </>
  );
}

/* ═══ STACKED-FAMILY PROFILE PANEL ═══
   One row per segment with height + r_bot + r_top + shape selector.
   Plus the world-space height/radius scale (same role as smooth's). */
function StackedProfilePanel({ geom, upd, updSegments }) {
  const shapeKeys = Object.keys(STACKED_SHAPES);
  return (
    <>
      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>Segments (bottom → top)</div>
      {geom.segments.map((seg, i) => (
        <div key={i} style={{ ...rw, gap: 4, padding: "2px 0", borderTop: i === 0 ? "none" : "1px dashed var(--color-border-tertiary)" }}>
          <span style={{ ...lb, minWidth: 48, fontWeight: 500 }} title={`Segment ${i}: ${STACKED_SEG_NAMES[i] || ""}`}>{STACKED_SEG_NAMES[i] || `Seg ${i}`}</span>
          <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }} title="Segment height (relative)">h</span>
          <Num value={seg.height} min={0.005} max={5} step={0.01} w={36}
               onChange={v => updSegments(n => { n.segments[i].height = v; })}
               tip="Segment height (relative). All segment heights are normalized to fill the world Height — only the ratio between segments matters." />
          <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }} title="Bottom radius of this segment (normalized)">r↓</span>
          <Num value={seg.r_bot} min={0} max={3} step={0.005} w={36}
               onChange={v => updSegments(n => { n.segments[i].r_bot = v; })}
               tip="Bottom radius of this segment (normalized to world Radius). 1.0 = full envelope; up to 3.0 for flared bases or wide shoulders." />
          <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }} title="Top radius of this segment (normalized)">r↑</span>
          <Num value={seg.r_top} min={0} max={3} step={0.005} w={36}
               onChange={v => updSegments(n => { n.segments[i].r_top = v; })}
               tip="Top radius of this segment (normalized to world Radius). Set to 0 on the finial for a pointed tip." />
          <select
            value={seg.profile}
            onChange={e => updSegments(n => { n.segments[i].profile = e.target.value; })}
            style={{ ...ist, padding: "2px 4px", fontSize: 10, textAlign: "left" }}
            title="Shape of the segment between r_bot and r_top: linear (frustum), concave (pinched), convex (barreled), step (cylinder with sudden taper at top)"
          >
            {shapeKeys.map(k => <option key={k} value={k}>{k}</option>)}
          </select>
        </div>
      ))}
      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Scale (world units)</div>
      <div style={rw}>
        <span style={lb} title="Total piece height in world units">Height</span>
        <Num value={geom.height} min={0.5} max={10} w={40} onChange={v => upd(n => { n.height = v; })}
             tip="Total height in world units. Segment heights are normalized to fill this." />
        <span style={lb} title="Outer radius of the piece in world units">Radius</span>
        <Num value={geom.radius} min={0.1} max={4} w={40} onChange={v => upd(n => { n.radius = v; })}
             tip="Outer radius in world units. Per-segment r_bot / r_top values multiply this." />
      </div>
      <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>
        Segment heights are relative — they are normalized to fill the world-space Height.
      </div>
    </>
  );
}

/* ═══ HERALDIC-FAMILY PROFILE PANEL ═══
   Stacked-style 4-segment body editor (Base/Shaft/Waist/Collar) plus a
   head selector and the parameters specific to the chosen head shape. */
function HeraldicProfilePanel({ geom, upd, updSegments }) {
  const shapeKeys = Object.keys(STACKED_SHAPES);
  const headKinds = Object.keys(HERALDIC_HEAD_KINDS);
  const headTiltDeg = (geom.head_tilt * 180) / Math.PI;
  const params = geom.head_params || {};

  // Helper: update a single field on geom.head_params (deep-copying to avoid mutation).
  const updParam = (key, value) => upd(n => {
    n.head_params = { ...(n.head_params || {}), [key]: value };
  });

  return (
    <>
      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>
        Body segments (bottom → top)
      </div>
      {geom.segments.map((seg, i) => (
        <div key={i} style={{ ...rw, gap: 4, padding: "2px 0", borderTop: i === 0 ? "none" : "1px dashed var(--color-border-tertiary)" }}>
          <span style={{ ...lb, minWidth: 48, fontWeight: 500 }} title={`Segment ${i}: ${HERALDIC_SEG_NAMES[i] || ""}`}>
            {HERALDIC_SEG_NAMES[i] || `Seg ${i}`}
          </span>
          <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }} title="Segment height (relative)">h</span>
          <Num value={seg.height} min={0.005} max={5} step={0.01} w={36}
               onChange={v => updSegments(n => { n.segments[i].height = v; })}
               tip="Segment height (relative). Heights are normalized to fill the body's world Height." />
          <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }} title="Bottom radius">r↓</span>
          <Num value={seg.r_bot} min={0} max={3} step={0.005} w={36}
               onChange={v => updSegments(n => { n.segments[i].r_bot = v; })}
               tip="Segment bottom radius (normalized to world Radius)." />
          <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }} title="Top radius">r↑</span>
          <Num value={seg.r_top} min={0} max={3} step={0.005} w={36}
               onChange={v => updSegments(n => { n.segments[i].r_top = v; })}
               tip="Segment top radius (normalized to world Radius). The Collar's r_top sets the platform that the head sits on." />
          <select
            value={seg.profile}
            onChange={e => updSegments(n => { n.segments[i].profile = e.target.value; })}
            style={{ ...ist, padding: "2px 4px", fontSize: 10, textAlign: "left" }}
            title="Segment profile shape: linear / concave / convex / step"
          >
            {shapeKeys.map(k => <option key={k} value={k}>{k}</option>)}
          </select>
        </div>
      ))}

      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Head</div>
      <div style={rw}>
        <span style={lb} title="Which decorative head shape sits on top of the collar">Kind</span>
        <select
          value={geom.head_kind}
          onChange={e => upd(n => { n.head_kind = e.target.value; })}
          style={{ ...ist, padding: "2px 4px", fontSize: 10, textAlign: "left", minWidth: 90 }}
          title="Head shape: crescent (fat moon with tips), cleft (dome with wedge cut), notched (open ring), forked (twin prongs)"
        >
          {headKinds.map(k => <option key={k} value={k}>{k}</option>)}
        </select>
        <span style={lb} title="Head footprint as fraction of the collar's r_top">Size</span>
        <Num value={geom.head_size} min={0.4} max={2.5} step={0.01} w={42}
             onChange={v => upd(n => { n.head_size = v; })}
             tip="Head footprint radius as a fraction of the collar's r_top. 1.0 = matches collar; >1 overhangs." />
      </div>
      <div style={rw}>
        <span style={lb} title="Vertical extent of the head in world units">Height</span>
        <Num value={geom.head_height} min={0.05} max={3} step={0.01} w={42}
             onChange={v => upd(n => { n.head_height = v; })}
             tip="Vertical extent of the head in world units. Adds to the body height." />
        <span style={lb} title="Pitch the head forward (+) or back (-) in radians">Tilt (°)</span>
        <Num value={headTiltDeg} min={-45} max={45} step={1} w={42}
             onChange={v => upd(n => { n.head_tilt = (v * Math.PI) / 180; })}
             tip="Pitch the head forward (positive) or back (negative). Slider is degrees; stored as radians." />
        <span style={lb} title="Translate head forward as fraction of collar r_top">Offset</span>
        <Num value={geom.head_offset} min={-1} max={1} step={0.01} w={42}
             onChange={v => upd(n => { n.head_offset = v; })}
             tip="Translate the head along +X (forward). Fraction of collar r_top." />
      </div>

      {/* Shape-specific parameters */}
      {geom.head_kind === "crescent" && (
        <>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Horns params</div>
          <div style={rw}>
            <span style={lb} title="Distance between the two horn roots">Base sep</span>
            <Num value={params.base_separation != null ? params.base_separation : 0.20} min={0} max={1.0} step={0.01} w={42}
                 onChange={v => updParam("base_separation", v)}
                 tip="Distance between the two horn roots, as fraction of head_size. 0 = roots meet at one point; higher = horns plant farther apart." />
            <span style={lb} title="Outward sweep distance">Spread</span>
            <Num value={params.spread != null ? params.spread : 0.55} min={0.0} max={1.5} step={0.01} w={42}
                 onChange={v => updParam("spread", v)}
                 tip="How far the horns sweep outward before curling up. Higher = wider horns." />
            <span style={lb} title="Vertical rise of the horns">Rise</span>
            <Num value={params.rise != null ? params.rise : 1.0} min={0.2} max={2.0} step={0.01} w={42}
                 onChange={v => updParam("rise", v)}
                 tip="Vertical reach of the horns, as fraction of head_height. 1.0 = tips reach the head's full extent." />
          </div>
          <div style={rw}>
            <span style={lb} title="How much the tips curl back inward">Tip curl</span>
            <Num value={params.tip_curl != null ? params.tip_curl : 0.45} min={0} max={1} step={0.01} w={42}
                 onChange={v => updParam("tip_curl", v)}
                 tip="How much the tips curl back inward. 0 = straight; 0.5 = halfway back; >0.5 = tips converge or cross." />
            <span style={lb} title="Tube cross-section radius at the root">Thickness</span>
            <Num value={params.thickness != null ? params.thickness : 0.18} min={0.02} max={0.5} step={0.01} w={42}
                 onChange={v => updParam("thickness", v)}
                 tip="Cross-section radius at the root, as fraction of head_size. Tapers to 0 at the tip." />
          </div>
        </>
      )}
      {geom.head_kind === "cleft" && (
        <>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Cleft params</div>
          <div style={rw}>
            <span style={lb} title="How deep the wedge cut goes into the dome">Depth</span>
            <Num value={params.cleft_depth != null ? params.cleft_depth : 0.55} min={0.05} max={0.95} step={0.01} w={42}
                 onChange={v => updParam("cleft_depth", v)}
                 tip="How deep the wedge cuts into the dome. 0.5 = halfway; 0.9 = nearly bisects the head." />
            <span style={lb} title="Angular width of the cut, in radians">Width</span>
            <Num value={params.cleft_width != null ? params.cleft_width : 0.9} min={0.2} max={2.5} step={0.05} w={42}
                 onChange={v => updParam("cleft_width", v)}
                 tip="Angular width of the wedge in radians. 0.5 = narrow slit; 1.5 = broad notch." />
          </div>
        </>
      )}
      {geom.head_kind === "notched" && (
        <>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Notched params</div>
          <div style={rw}>
            <span style={lb} title="Angular size of the gap in radians">Gap</span>
            <Num value={params.gap != null ? params.gap : 1.2} min={0.1} max={3.5} step={0.05} w={42}
                 onChange={v => updParam("gap", v)}
                 tip="Angular gap in the ring, in radians. 0.5 = small opening; 2.5 = mostly open." />
            <span style={lb} title="Ring thickness as fraction of head_size">Ring thick</span>
            <Num value={params.ring_thick != null ? params.ring_thick : 0.25} min={0.05} max={0.6} step={0.01} w={42}
                 onChange={v => updParam("ring_thick", v)}
                 tip="Thickness of the ring tube, as fraction of head radius." />
          </div>
        </>
      )}
      {geom.head_kind === "forked" && (
        <>
          <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Forked params</div>
          <div style={rw}>
            <span style={lb} title="How far apart the prongs splay">Splay</span>
            <Num value={params.prong_angle != null ? params.prong_angle : 0.5} min={0} max={1.4} step={0.02} w={42}
                 onChange={v => updParam("prong_angle", v)}
                 tip="How far the prongs splay outward at the tip, in radians. 0 = parallel; 1.0 = wide V." />
            <span style={lb} title="Prong base thickness">Thickness</span>
            <Num value={params.prong_thick != null ? params.prong_thick : 0.25} min={0.05} max={0.6} step={0.01} w={42}
                 onChange={v => updParam("prong_thick", v)}
                 tip="Prong base thickness as fraction of head radius." />
            <span style={lb} title="Tip thickness as fraction of base thickness">Taper</span>
            <Num value={params.prong_taper != null ? params.prong_taper : 0.10} min={0} max={1} step={0.01} w={42}
                 onChange={v => updParam("prong_taper", v)}
                 tip="Tip thickness as a fraction of the base thickness. 0 = sharp point; 1 = no taper." />
          </div>
        </>
      )}

      <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Scale (world units)</div>
      <div style={rw}>
        <span style={lb} title="Body height in world units (head_height adds on top)">Body H</span>
        <Num value={geom.height} min={0.5} max={10} w={42}
             onChange={v => upd(n => { n.height = v; })}
             tip="Body height in world units. The total piece height is body_height + head_height." />
        <span style={lb} title="Outer radius in world units">Radius</span>
        <Num value={geom.radius} min={0.1} max={4} w={42}
             onChange={v => upd(n => { n.radius = v; })}
             tip="Outer radius in world units. Multiplies the per-segment normalized r_bot/r_top." />
      </div>
      <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>
        Body uses the same segment vocabulary as Stacked. Head sits on top of the Collar's r_top.
      </div>
    </>
  );
}

const btnStyle = {
  fontSize: 10, padding: "2px 8px", borderRadius: 4, cursor: "pointer",
  border: "1px solid var(--color-border-tertiary)",
  background: "var(--color-background-secondary)",
  color: "var(--color-text-secondary)",
};