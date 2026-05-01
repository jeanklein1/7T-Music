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
    start_r: 0.85, flare_r: 0.90, flare_t: 0.05,
    peak_r: 1.05, peak_t: 0.12,
    base_t: 0.18, body_start_r: 0.85,
    body_t: 0.58, waist_r: 0.25,
    neck_t: 0.60, neck_r: 0.40,
    collar_t: 0.67, collar_bulge: 0.24,
    head_t: 0.93, head_base_r: 0.30, head_sphere_r: 0.20,
    height: 1.9, radius: 0.55,
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
    start_r: 0.50, flare_r: 0.90, flare_t: 0.05,
    peak_r: 0.80, peak_t: 0.18,
    base_t: 0.28, body_start_r: 0.50,
    body_t: 0.50, waist_r: 0.35,
    neck_t: 0.65, neck_r: 0.30,
    collar_t: 0.70, collar_bulge: 0.25,
    head_t: 0.93, head_base_r: 0.25, head_sphere_r: 0.20,
    height: 1.7, radius: 0.50,
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
    collar_t: 0.66, collar_bulge: 0.30,
    head_t: 0.92, head_base_r: 0.10, head_sphere_r: 0.20,
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
function buildPawnFaces(radiusAt, height, radius, stops, transformVert, discs = null, extra = null, topT = 1) {
  const faces = [];
  const lightDir = normalize3(PAWN_LIGHT_DIR);

  // Surface revolution. radiusAt may depend on theta — we sample
  // per-vertex (4 queries per quad) rather than once per ring.
  // The revolution covers t ∈ [0, topT]. For most families topT=1; the
  // Heraldic family passes topT < 1 because its head region is rendered
  // separately via extraFaces, and we don't want degenerate quads for
  // t > topT producing phantom discs.
  for (let r = 0; r < RENDER_RINGS - 1; r++) {
    for (let s = 0; s < RENDER_SEGS; s++) {
      const t0 = (r / (RENDER_RINGS - 1)) * topT;
      const t1 = ((r + 1) / (RENDER_RINGS - 1)) * topT;
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

  // Top cap — only emitted when the silhouette at t=topT has non-trivial radius.
  // Without this, any pawn whose top segment doesn't taper to a point reads as
  // an open bottle. For pieces that already taper to ~0 (Bishop's mitre, Queen's
  // orb-to-point, etc.) the work is skipped — there's no hole to plug.
  // Use the angle-0 sample as the closure test (a noisy lobe could give slightly
  // higher r_top in some directions but this is sufficient as a heuristic).
  const topR0 = radiusAt(topT, 0) * radius;
  if (topR0 > 1e-4) {
    const topY = topT * height - height * 0.45; // matches the surface revolution's last ring
    const topCol = evalGradient(topT, stops);
    for (let s = 0; s < RENDER_SEGS; s++) {
      const a0 = (s / RENDER_SEGS) * Math.PI * 2;
      const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
      const ra = radiusAt(topT, a0) * radius;
      const rb = radiusAt(topT, a1) * radius;
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
  const topT  = family.bodyTopT ? family.bodyTopT(geom) : 1;
  const faces = buildPawnFaces((t, theta) => family.radiusAt(geom, t, theta), height, radius, stops, transformVert, discs, extra, topT);
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
    const topT  = family.bodyTopT ? family.bodyTopT(tier.geom) : 1;
    const faces = buildPawnFaces((t, theta) => family.radiusAt(tier.geom, t, theta), b.height, b.radius, tier.stops, transformVert, discs, extra, topT);
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
    <div ref={dragRef} style={{ marginBottom: pos ? 0 : 7, border: pos ? "1px solid #666" : "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: pos ? "#2a2a30" : "var(--color-background-primary)", boxShadow: pos ? "0 8px 32px rgba(0,0,0,.6)" : "none", ...style }}>
      <div onMouseDown={startDrag} style={{
        padding: "8px 12px",
        fontSize: 12,
        fontWeight: 600,
        letterSpacing: 0.2,
        userSelect: "none",
        display: "flex",
        alignItems: "center",
        gap: 6,
        color: pos ? "#e0ddd8" : "var(--color-text-primary)",
        // Header background: stronger contrast against the body. Translucent so
        // it composes correctly whether the parent is light (docked) or dark
        // (dragged out).
        background: pos ? "#353540" : "rgba(127,127,127,0.16)",
        borderBottom: pos ? "1px solid #555" : "1px solid var(--color-border-tertiary)",
        cursor: "grab",
      }}>
        <span data-notdrag="1" onClick={() => setOpen(!open)} style={{ cursor: "pointer", fontSize: 9, transition: "transform .15s", display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px" }}>▶</span>
        <span style={{ flex: 1 }}>{title}</span>
        {pos && <span data-notdrag="1" onClick={doDock} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}
      </div>
      {open && <div style={{ padding: "8px 12px 10px", maxHeight: pos ? 400 : "none", overflowY: pos ? "auto" : "visible", color: pos ? "#d0cdc8" : undefined }}>{children}</div>}
    </div>
  );
}

/* ═══ SECTION LABEL ═══
   Reusable subtitle pill used inside panels to mark logical groupings —
   "Section boundaries", "Scale", "Color stops", etc. A tinted background
   band with a colored left-bar accent makes section transitions visible at
   a glance without heavy borders. Accepts children for sections that need
   to put a button next to the label (e.g. the segment editor's "+ add"). */
function SectionLabel({ children, style }) {
  return (
    <div style={{
      fontSize: 10,
      fontWeight: 600,
      letterSpacing: 0.3,
      textTransform: "uppercase",
      color: "var(--color-text-secondary)",
      // Translucent overlay reads as a subtle tint on light backgrounds (docked
      // panels) and a soft highlight on dark backgrounds (dragged-out panels).
      background: "rgba(127,127,127,0.10)",
      borderLeft: "2px solid var(--color-text-tertiary, #888)",
      padding: "5px 12px",
      margin: "8px -12px 6px",
      display: "flex",
      alignItems: "center",
      gap: 6,
      ...(style || {}),
    }}>{children}</div>
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

const rw = { display: "flex", flexWrap: "wrap", gap: "4px 9px", alignItems: "center", marginBottom: 5 };
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
     concave : pinched midpoint (curves inward, like a goblet stem)
     convex  : barrelled midpoint (curves outward, like a barrel)
     step    : near-cylindrical body (r_bot until u≈0.85, then quick taper to r_top)
     bell    : narrow at both ends, fat in the middle — overrides r_bot/r_top
               for a true bead/torus-ring look (always returns to r_bot at u=0
               and r_top at u=1, but bulges far above either between)
     flare   : accelerating curve — slow at the bottom, fast at the top, like
               a trumpet bell mouth opening up
     ogee    : S-curve — concave then convex, classic architectural moulding
     sphere  : true semicircle silhouette. r(u) = R · sqrt(1 - (2u-1)²) where
               R = max(r_bot, r_top). Endpoints are exactly 0 — gives a clean
               closure at both top and bottom of the segment regardless of
               r_bot/r_top input. For a true circular sphere (not an ellipsoid),
               the segment's world-height should equal 2·R·world-radius.
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
  bell:    (u, rb, rt) => {
    // bead/torus shape: anchored at endpoints, big bulge in the middle.
    // The bulge magnitude scales with the larger of the two endpoint radii so
    // it reads as "fatter than the segment ends" regardless of rb,rt values.
    const line = rb + (rt - rb) * u;
    const ref  = Math.max(rb, rt, 0.05);
    return line + 0.55 * ref * Math.sin(u * Math.PI);
  },
  flare:   (u, rb, rt) => {
    // accelerating curve: slow growth near u=0, fast near u=1.
    // Quadratic ease-in: u² goes from 0→1 with the action at the top.
    return rb + (rt - rb) * (u * u);
  },
  ogee:    (u, rb, rt) => {
    // S-curve from rb to rt: dips below the line in the lower half, lifts
    // above it in the upper half. Smoothstep gives the S shape; we offset by
    // a sine so the dip and lift are visible as inflections.
    const line = rb + (rt - rb) * u;
    const mid  = (rb + rt) * 0.5;
    return line + 0.18 * mid * Math.sin(u * Math.PI * 2);
  },
  sphere:  (u, rb, rt) => {
    // true semicircle, peak radius = max(rb, rt). Closes to 0 at both ends.
    const R = Math.max(rb, rt);
    const x = 2 * u - 1; // -1 at base, +1 at top
    const arg = Math.max(0, 1 - x * x);
    return R * Math.sqrt(arg);
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
    // Use the profile function's actual endpoint radii rather than the raw
    // stored r_top/r_bot — profiles like 'sphere' close to 0 at both endpoints
    // regardless of stored values, so the boundary isn't a real discontinuity.
    const fnBelow = STACKED_SHAPES[segs[k].profile]     || STACKED_SHAPES.linear;
    const fnAbove = STACKED_SHAPES[segs[k + 1].profile] || STACKED_SHAPES.linear;
    const rBelow  = Math.max(0, fnBelow(1, segs[k].r_bot,     segs[k].r_top));
    const rAbove  = Math.max(0, fnAbove(0, segs[k + 1].r_bot, segs[k + 1].r_top));
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
   STACKED II — pawn-proportioned variants of the seven Stacked presets.

   Same five-segment vocabulary (Base / Shaft / Waist / Collar / Finial),
   same per-tier heights, same Collar and Finial as the originals — but the
   Base, Shaft, and Waist have been re-radialized to read as pawn-like
   silhouettes: a wide flared base (convex curve), a slim shank, a real
   waist pinch, and a flare back out to whatever the original tier's
   collar dictates.

   The geometry shares Stacked's radiusAt / boundaries / discontinuities /
   panel — only the tier defaults differ.
   ════════════════════════════════════════════════════════════════════════ */

/* Tier 0 II — Column: wide flared base, slim concave shank, narrow neck
   that flares up into the original Collar at 0.75. Total height bumped
   from 1.8 to 2.1 to give the wide-collared head more vertical room. */
function stacked2ColumnGeom() {
  return {
    height: 2.1, radius: 0.55,
    segments: [
      { height: 0.20, r_bot: 1.30, r_top: 0.72, profile: "convex"  }, // Base (1.3× scaled foundation)
      { height: 0.45, r_bot: 0.55, r_top: 0.30, profile: "concave" }, // Shaft (narrows toward neck)
      { height: 0.20, r_bot: 0.30, r_top: 0.75, profile: "convex"  }, // Waist (narrow neck flares up)
      { height: 0.10, r_bot: 0.75, r_top: 0.65, profile: "convex"  }, // Collar (unchanged)
      { height: 0.20, r_bot: 0.50, r_top: 0.05, profile: "linear"  }, // Finial (unchanged)
    ],
  };
}

/* Tier 1 II — Totem: dramatically scaled — base widened by 1.95× and height
   stretched 1.5× from 2.6 to 3.9. The piece reads as monumental: a massive
   flared foundation, slim body, and the original wide collar getting plenty
   of room to breathe at the top. */
function stacked2TotemGeom() {
  return {
    height: 3.9, radius: 0.60,
    segments: [
      { height: 0.18, r_bot: 1.95, r_top: 1.21, profile: "convex"  }, // Base (1.95× scaled)
      { height: 0.30, r_bot: 0.62, r_top: 0.32, profile: "concave" },
      { height: 0.20, r_bot: 0.32, r_top: 0.85, profile: "convex"  },
      { height: 0.18, r_bot: 0.85, r_top: 0.85, profile: "convex"  }, // Collar (unchanged)
      { height: 0.14, r_bot: 0.55, r_top: 0.30, profile: "convex"  }, // Finial (unchanged)
    ],
  };
}

/* Tier 2 II — Pylon: wide base, slim concave shank, narrow neck rising
   to the Collar at 0.78. Height bumped from 3.0 to 3.3. */
function stacked2PylonGeom() {
  return {
    height: 3.3, radius: 0.50,
    segments: [
      { height: 0.10, r_bot: 1.30, r_top: 0.75, profile: "convex"  }, // Base (1.3× scaled)
      { height: 0.50, r_bot: 0.58, r_top: 0.32, profile: "concave" },
      { height: 0.12, r_bot: 0.32, r_top: 0.78, profile: "convex"  },
      { height: 0.18, r_bot: 0.78, r_top: 0.72, profile: "step"    }, // Collar (unchanged)
      { height: 0.10, r_bot: 0.50, r_top: 0.30, profile: "linear"  }, // Finial (unchanged)
    ],
  };
}

/* Tier 3 II — Bishop: wide base, slim concave shank, narrow neck rising
   modestly to the Collar at 0.62. Cleanest pawn reading in the set;
   height unchanged since the Collar is moderate. */
function stacked2BishopGeom() {
  return {
    height: 2.4, radius: 0.45,
    segments: [
      { height: 0.14, r_bot: 1.30, r_top: 0.72, profile: "convex"  }, // Base (1.3× scaled)
      { height: 0.42, r_bot: 0.55, r_top: 0.30, profile: "concave" },
      { height: 0.06, r_bot: 0.30, r_top: 0.62, profile: "convex"  },
      { height: 0.12, r_bot: 0.62, r_top: 0.50, profile: "convex"  }, // Collar (unchanged)
      { height: 0.30, r_bot: 0.45, r_top: 0.00, profile: "linear"  }, // Finial (unchanged)
    ],
  };
}

/* Tier 4 II — Queen: narrowest original Collar.r_bot (0.55) lets the
   neck stay genuinely slim — reads as a tall pawn with a queen's crown.
   Height unchanged. */
function stacked2QueenGeom() {
  return {
    height: 2.8, radius: 0.55,
    segments: [
      { height: 0.14, r_bot: 1.30, r_top: 0.72, profile: "convex"  }, // Base (1.3× scaled)
      { height: 0.36, r_bot: 0.55, r_top: 0.30, profile: "concave" },
      { height: 0.10, r_bot: 0.30, r_top: 0.55, profile: "convex"  },
      { height: 0.18, r_bot: 0.55, r_top: 0.85, profile: "convex"  }, // Collar (unchanged)
      { height: 0.22, r_bot: 0.75, r_top: 0.00, profile: "convex"  }, // Finial (unchanged)
    ],
  };
}

/* Tier 5 II — King: wide base, slim concave shank, narrow neck flaring
   into the broad crown at 0.85. Height bumped from 3.2 to 3.6 to give
   the wide-collared crown breathing room above an imposing pawn body. */
function stacked2KingGeom() {
  return {
    height: 3.6, radius: 0.62,
    segments: [
      { height: 0.16, r_bot: 1.30, r_top: 0.81, profile: "convex"  }, // Base (1.3× scaled)
      { height: 0.38, r_bot: 0.62, r_top: 0.32, profile: "concave" },
      { height: 0.08, r_bot: 0.32, r_top: 0.85, profile: "convex"  },
      { height: 0.20, r_bot: 0.85, r_top: 0.95, profile: "step"    }, // Collar (unchanged)
      { height: 0.18, r_bot: 0.45, r_top: 0.00, profile: "linear"  }, // Finial (unchanged)
    ],
  };
}

/* Tier 6 II — Bauhaus: wide base, slim concave shank, narrow neck rising
   to the Collar at 0.65. Height unchanged. */
function stacked2BauhausGeom() {
  return {
    height: 1.6, radius: 0.42,
    segments: [
      { height: 0.18, r_bot: 1.30, r_top: 0.72, profile: "convex"  }, // Base (1.3× scaled)
      { height: 0.40, r_bot: 0.55, r_top: 0.32, profile: "concave" },
      { height: 0.04, r_bot: 0.32, r_top: 0.65, profile: "convex"  },
      { height: 0.06, r_bot: 0.65, r_top: 0.65, profile: "step"    }, // Collar (unchanged)
      { height: 0.32, r_bot: 0.65, r_top: 0.00, profile: "convex"  }, // Finial (unchanged)
    ],
  };
}

/* Stops — re-use the original Stacked stops for each tier so palette
   continuity is preserved. The user wants the visual identity carried
   over, just with new radial proportions. */
const STACKED2_TIER_NAMES = ["Column II", "Totem II", "Pylon II", "Bishop II", "Queen II", "King II", "Bauhaus II"];
const STACKED2_GEOM_FNS  = [stacked2ColumnGeom, stacked2TotemGeom, stacked2PylonGeom, stacked2BishopGeom, stacked2QueenGeom, stacked2KingGeom, stacked2BauhausGeom];
const STACKED2_STOPS_FNS = [stackedColumnStops, stackedTotemStops, stackedPylonStops, stackedBishopStops, stackedQueenStops, stackedKingStops, stackedBauhausStops];

const STACKED2_FAMILY = {
  name: "Stacked II",
  tierNames: STACKED2_TIER_NAMES,
  defaultTiers: () => STACKED2_TIER_NAMES.map((_, i) => ({
    geom:  STACKED2_GEOM_FNS[i](),
    stops: STACKED2_STOPS_FNS[i](),
  })),
  // Geometry/render hooks — Stacked II shares the underlying segment math
  // entirely; only the per-tier defaults differ.
  radiusAt: stackedRadiusAt,
  bounds:   (geom) => ({ height: geom.height, radius: geom.radius }),
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
   HERALDIC FAMILY — variable-length stacked segments, no head primitive.

   Every piece is a stack of 1–7 segments. Each segment has the same
   per-segment vocabulary as Stacked — height, r_bot, r_top, profile shape.
   The segment list is the entire piece end-to-end: there is no separate
   head, no extraFaces, no add-on ornaments. All design expression flows
   through (a) the number and proportions of segments and (b) the seven
   profile shapes available (linear, concave, convex, step, bell, flare,
   ogee).

   The seven default tiers are each named after a celestial body and
   sculpted so its silhouette is recognizable on its own — different segment
   counts (2 to 7), different proportions, different profile vocabularies.

   Geom shape:
     segments  : array of 1–7 {height, r_bot, r_top, profile} entries,
                 ordered bottom → top
     height    : world-space total piece height (segment heights normalize to fill it)
     radius    : world-space radius scale (segment radii multiply this)
   ════════════════════════════════════════════════════════════════════════ */

/* Variable segment count: 1..7. Position-relative labels — bottommost is
   "Base", topmost is "Crown", solo is "Body", anything in between is "Seg N". */
const HERALDIC_MAX_SEGS = 7;
const HERALDIC_MIN_SEGS = 1;
function heraldicSegLabel(i, n) {
  if (n === 1) return "Body";
  if (i === 0) return "Base";
  if (i === n - 1) return "Crown";
  return `Seg ${i + 1}`;
}

/* Body radius lookup. t is normalized over the WHOLE piece (0 at base, 1 at the
   top of the topmost segment). The whole piece is segments — no separate head. */
function heraldicRadiusAt(geom, t, _theta) {
  if (t > 1) return 0;
  const segs = geom.segments;
  if (!segs || segs.length === 0) return 0;
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

function heraldicBounds(geom) {
  return { height: geom.height, radius: geom.radius };
}

function heraldicEnvelopeMax(geom) {
  let m = 0;
  for (const s of geom.segments) {
    if (s.r_bot > m) m = s.r_bot;
    if (s.r_top > m) m = s.r_top;
    // bell profile bulges to ~1.55x the larger endpoint, so account for that
    // when the user has chosen the bell shape.
    if (s.profile === "bell") {
      const ref = Math.max(s.r_bot, s.r_top);
      if (ref * 1.55 > m) m = ref * 1.55;
    }
  }
  return Math.max(m, 0.0001);
}

/* Segment boundaries in t-space [0,1]. Position-relative labels via heraldicSegLabel. */
function heraldicBoundaries(geom) {
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  const out = [];
  let acc = 0;
  for (let k = 0; k < segs.length; k++) {
    acc += segs[k].height / totalH;
    out.push({ t: acc, label: heraldicSegLabel(k, segs.length) });
  }
  return out;
}

function heraldicDiscontinuities(geom) {
  const segs = geom.segments;
  const totalH = segs.reduce((a, s) => a + s.height, 0) || 1;
  const out = [];
  let acc = 0;
  for (let k = 0; k < segs.length - 1; k++) {
    acc += segs[k].height / totalH;
    // Use the profile function's actual endpoint radii — some profiles (sphere)
    // close to 0 at endpoints regardless of stored r_bot/r_top, so the boundary
    // between two segments isn't a real discontinuity even when their stored
    // values disagree.
    const fnBelow = STACKED_SHAPES[segs[k].profile]     || STACKED_SHAPES.linear;
    const fnAbove = STACKED_SHAPES[segs[k + 1].profile] || STACKED_SHAPES.linear;
    const rBelow  = Math.max(0, fnBelow(1, segs[k].r_bot,     segs[k].r_top));
    const rAbove  = Math.max(0, fnAbove(0, segs[k + 1].r_bot, segs[k + 1].r_top));
    if (Math.abs(rBelow - rAbove) > 1e-4) {
      out.push({ t: acc, rBelow, rAbove });
    }
  }
  return out;
}

/* Tier defaults — 7 progressive refinements of one pawn essence.

   Every tier is the same fundamental piece — a Staunton-tradition pawn:
   floating foot moulding, weighted base, concave shank, thin collar disc,
   spherical head. What changes from tier to tier is the degree of
   refinement and ornamentation.

   The progression follows a Saint-Seiya-armor logic: Bronze (raw, basic)
   → Silver (refined profile) → Gold (first ornament) → Steel (more imposing
   proportions) → Crystal (finial detail added) → Star (delicate refinement)
   → Divine (most ornate, pearlescent). Each step strictly adds something
   to the previous, never substituting away from the pawn essence.

   Math note: the sphere segment uses the new "sphere" profile. For a true
   circular silhouette (not stretched ellipse), the sphere segment's relative
   height (as a fraction of the sum of all segment heights, times the piece's
   world height) must equal 2 × sphere_radius × world_radius. With the
   per-tier conventions used here (world H ≈ 1.0, world R = 0.50, sphere
   peak normalized radius ≈ 0.40), the sphere segment relative height is
   ≈ 0.40 of total — adjusted per tier.
*/

/* Tier 0 — Bronze: the pawn essence in its most basic form. 5 segments.
   Foot moulding, base, concave shank, thin collar disc, sphere. Plain
   bronze palette — the unrefined starting point. */
function heraldicBronzeGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.00, r_top: 0.92, profile: "convex"  }, // foot moulding
      { height: 0.10, r_bot: 0.92, r_top: 0.55, profile: "convex"  }, // base
      { height: 0.34, r_bot: 0.50, r_top: 0.30, profile: "concave" }, // shank (pinched)
      { height: 0.04, r_bot: 0.30, r_top: 0.42, profile: "convex"  }, // collar disc
      { height: 0.40, r_bot: 0.00, r_top: 0.40, profile: "sphere"  }, // head sphere
    ],
    height: 1.00, radius: 0.50,
  };
}
function heraldicBronzeStops() {
  return [
    { t: 0.00, c: [0.32, 0.20, 0.10], label: "Foot"   },
    { t: 0.06, c: [0.45, 0.28, 0.14], label: "Base"   },
    { t: 0.20, c: [0.55, 0.36, 0.18], label: "Shank"  },
    { t: 0.55, c: [0.65, 0.42, 0.22], label: "Collar" },
    { t: 1.00, c: [0.78, 0.55, 0.28], label: "Head"   },
  ];
}

/* Tier 1 — Silver: same architecture as Bronze, but the shank is now an
   ogee (S-curve) instead of plain concave — more elegant. Slightly taller
   (1.10) and more graceful. Polished silver palette. */
function heraldicSilverGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.00, r_top: 0.92, profile: "convex" },
      { height: 0.10, r_bot: 0.92, r_top: 0.55, profile: "convex" },
      { height: 0.38, r_bot: 0.50, r_top: 0.28, profile: "ogee"   }, // refined shank
      { height: 0.04, r_bot: 0.28, r_top: 0.42, profile: "convex" },
      { height: 0.40, r_bot: 0.00, r_top: 0.40, profile: "sphere" },
    ],
    height: 1.10, radius: 0.50,
  };
}
function heraldicSilverStops() {
  return [
    { t: 0.00, c: [0.55, 0.55, 0.58], label: "Foot"   },
    { t: 0.06, c: [0.68, 0.68, 0.70], label: "Base"   },
    { t: 0.20, c: [0.78, 0.78, 0.80], label: "Shank"  },
    { t: 0.55, c: [0.85, 0.85, 0.86], label: "Collar" },
    { t: 1.00, c: [0.92, 0.92, 0.94], label: "Head"   },
  ];
}

/* Tier 2 — Gold: adds a single decorative bead between the shank and the
   collar — the Victorian-balcony detail. 6 segments. Refined ogee shank
   (carried forward from Silver). Gold tones. */
function heraldicGoldGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.00, r_top: 0.92, profile: "convex" },
      { height: 0.10, r_bot: 0.92, r_top: 0.55, profile: "convex" },
      { height: 0.36, r_bot: 0.50, r_top: 0.26, profile: "ogee"   },
      { height: 0.04, r_bot: 0.26, r_top: 0.26, profile: "bell"   }, // NEW: bead
      { height: 0.04, r_bot: 0.26, r_top: 0.42, profile: "convex" },
      { height: 0.42, r_bot: 0.00, r_top: 0.42, profile: "sphere" },
    ],
    height: 1.15, radius: 0.50,
  };
}
function heraldicGoldStops() {
  return [
    { t: 0.00, c: [0.42, 0.30, 0.10], label: "Foot"   },
    { t: 0.06, c: [0.62, 0.45, 0.16], label: "Base"   },
    { t: 0.20, c: [0.78, 0.58, 0.20], label: "Shank"  },
    { t: 0.50, c: [0.92, 0.72, 0.28], label: "Bead"   },
    { t: 0.58, c: [0.85, 0.65, 0.24], label: "Collar" },
    { t: 1.00, c: [0.98, 0.82, 0.38], label: "Head"   },
  ];
}

/* Tier 3 — Steel: more imposing version of Gold. Wider base, longer
   shank, thicker bead. Same 6-segment architecture as Gold. Cool steel-blue
   palette with brushed-metal feel. */
function heraldicSteelGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.05, r_top: 0.95, profile: "convex" }, // wider foot
      { height: 0.10, r_bot: 0.95, r_top: 0.58, profile: "convex" }, // wider base
      { height: 0.42, r_bot: 0.52, r_top: 0.24, profile: "ogee"   }, // longer shank
      { height: 0.05, r_bot: 0.24, r_top: 0.24, profile: "bell"   }, // thicker bead
      { height: 0.04, r_bot: 0.24, r_top: 0.42, profile: "convex" },
      { height: 0.42, r_bot: 0.00, r_top: 0.42, profile: "sphere" },
    ],
    height: 1.30, radius: 0.55,
  };
}
function heraldicSteelStops() {
  return [
    { t: 0.00, c: [0.22, 0.26, 0.32], label: "Foot"   },
    { t: 0.06, c: [0.35, 0.42, 0.50], label: "Base"   },
    { t: 0.20, c: [0.50, 0.58, 0.65], label: "Shank"  },
    { t: 0.55, c: [0.62, 0.70, 0.75], label: "Bead"   },
    { t: 0.62, c: [0.58, 0.66, 0.72], label: "Collar" },
    { t: 1.00, c: [0.78, 0.85, 0.90], label: "Head"   },
  ];
}

/* Tier 4 — Crystal: adds a small finial bud crowning the sphere. 7 segments.
   The bud is a tiny bell segment at the very top — like a polished cap. The
   palette shifts to luminous translucent crystal — pale aqua with a hint of
   white. */
function heraldicCrystalGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.05, r_top: 0.95, profile: "convex" },
      { height: 0.10, r_bot: 0.95, r_top: 0.58, profile: "convex" },
      { height: 0.42, r_bot: 0.52, r_top: 0.24, profile: "ogee"   },
      { height: 0.05, r_bot: 0.24, r_top: 0.24, profile: "bell"   },
      { height: 0.04, r_bot: 0.24, r_top: 0.42, profile: "convex" },
      { height: 0.40, r_bot: 0.00, r_top: 0.40, profile: "sphere" },
      { height: 0.06, r_bot: 0.00, r_top: 0.10, profile: "sphere" }, // NEW: finial bud (sphere closes to point)
    ],
    height: 1.35, radius: 0.55,
  };
}
function heraldicCrystalStops() {
  return [
    { t: 0.00, c: [0.30, 0.50, 0.55], label: "Foot"   },
    { t: 0.06, c: [0.45, 0.65, 0.70], label: "Base"   },
    { t: 0.20, c: [0.60, 0.78, 0.82], label: "Shank"  },
    { t: 0.55, c: [0.72, 0.86, 0.88], label: "Bead"   },
    { t: 0.62, c: [0.78, 0.90, 0.92], label: "Collar" },
    { t: 0.92, c: [0.88, 0.96, 0.98], label: "Head"   },
    { t: 1.00, c: [0.98, 1.00, 1.00], label: "Bud"    },
  ];
}

/* Tier 5 — Star: same 7-segment architecture as Crystal, but every detail
   is more delicate. Longer shank, smaller more-refined bead, smaller head
   sphere relative to body. Pearlescent palette with star-white highlights. */
function heraldicStarGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.00, r_top: 0.90, profile: "convex" },
      { height: 0.09, r_bot: 0.90, r_top: 0.52, profile: "convex" },
      { height: 0.48, r_bot: 0.48, r_top: 0.20, profile: "ogee"   }, // even longer shank
      { height: 0.04, r_bot: 0.20, r_top: 0.20, profile: "bell"   }, // small refined bead
      { height: 0.03, r_bot: 0.20, r_top: 0.36, profile: "convex" },
      { height: 0.36, r_bot: 0.00, r_top: 0.36, profile: "sphere" }, // smaller head
      { height: 0.06, r_bot: 0.00, r_top: 0.08, profile: "sphere" },
    ],
    height: 1.45, radius: 0.55,
  };
}
function heraldicStarStops() {
  return [
    { t: 0.00, c: [0.62, 0.60, 0.72], label: "Foot"   },
    { t: 0.06, c: [0.78, 0.75, 0.85], label: "Base"   },
    { t: 0.20, c: [0.88, 0.85, 0.92], label: "Shank"  },
    { t: 0.62, c: [0.92, 0.88, 0.95], label: "Bead"   },
    { t: 0.68, c: [0.95, 0.92, 0.96], label: "Collar" },
    { t: 0.92, c: [0.98, 0.96, 1.00], label: "Head"   },
    { t: 1.00, c: [1.00, 1.00, 1.00], label: "Bud"    },
  ];
}

/* Tier 6 — Divine: the most refined form. Same 7-segment architecture as
   Star but the bead is articulated as a doubled band (taller bell) and the
   proportions are at their most graceful. White-gold palette with luminous
   accents — the celestial crown of the progression. */
function heraldicDivineGeom() {
  return {
    segments: [
      { height: 0.04, r_bot: 1.00, r_top: 0.88, profile: "convex" },
      { height: 0.10, r_bot: 0.88, r_top: 0.50, profile: "convex" },
      { height: 0.50, r_bot: 0.46, r_top: 0.18, profile: "ogee"   },
      { height: 0.07, r_bot: 0.20, r_top: 0.20, profile: "bell"   }, // doubled-band bead
      { height: 0.03, r_bot: 0.20, r_top: 0.38, profile: "convex" },
      { height: 0.38, r_bot: 0.00, r_top: 0.38, profile: "sphere" },
      { height: 0.07, r_bot: 0.00, r_top: 0.10, profile: "sphere" }, // tapered bud (sphere closes to point)
    ],
    height: 1.55, radius: 0.55,
  };
}
function heraldicDivineStops() {
  return [
    { t: 0.00, c: [0.85, 0.78, 0.55], label: "Foot"   },
    { t: 0.06, c: [0.92, 0.85, 0.60], label: "Base"   },
    { t: 0.20, c: [0.96, 0.90, 0.65], label: "Shank"  },
    { t: 0.62, c: [1.00, 0.95, 0.70], label: "Bead"   },
    { t: 0.70, c: [1.00, 0.96, 0.75], label: "Collar" },
    { t: 0.92, c: [1.00, 0.98, 0.85], label: "Head"   },
    { t: 1.00, c: [1.00, 1.00, 0.95], label: "Bud"    },
  ];
}

const HERALDIC_TIER_NAMES = [
  "Bronze", "Silver", "Gold",
  "Steel", "Crystal", "Star", "Divine",
];
const HERALDIC_GEOM_FNS  = [
  heraldicBronzeGeom, heraldicSilverGeom, heraldicGoldGeom,
  heraldicSteelGeom, heraldicCrystalGeom, heraldicStarGeom, heraldicDivineGeom,
];
const HERALDIC_STOPS_FNS = [
  heraldicBronzeStops, heraldicSilverStops, heraldicGoldStops,
  heraldicSteelStops, heraldicCrystalStops, heraldicStarStops, heraldicDivineStops,
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
};

const FAMILIES = {
  smooth:    SMOOTH_FAMILY,
  stacked:   STACKED_FAMILY,
  stacked2:  STACKED2_FAMILY,
  heraldic:  HERALDIC_FAMILY,
};
const FAMILY_KEYS = ["smooth", "stacked", "stacked2", "heraldic"];

/* ── Defaults across all families — full state shape for first launch ── */
function defaultAllFamilies() {
  return {
    smooth:    SMOOTH_FAMILY.defaultTiers(),
    stacked:   STACKED_FAMILY.defaultTiers(),
    stacked2:  STACKED2_FAMILY.defaultTiers(),
    heraldic:  HERALDIC_FAMILY.defaultTiers(),
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
            // Heraldic migration: older saves have head_kind / head_size /
            // head_height / head_tilt / head_offset / head_params fields on
            // each geom. The Heraldic family no longer uses them — every
            // piece is segments only. We strip those fields silently. If a
            // saved geom is missing the segments array entirely (very old
            // Heraldic shape), we drop it and fall back to the default.
            const migrateHeraldicGeom = (geom) => {
              if (!geom || typeof geom !== "object") return null;
              if (!Array.isArray(geom.segments) || geom.segments.length === 0) return null;
              // Keep only fields the new schema uses.
              return {
                segments: geom.segments.map(s => ({
                  height: s.height,
                  r_bot:  s.r_bot,
                  r_top:  s.r_top,
                  profile: s.profile || "linear",
                })),
                height: geom.height,
                radius: geom.radius,
              };
            };
            const merged = defaultAllFamilies();
            for (const fk of FAMILY_KEYS) {
              const incoming = data[fk];
              if (Array.isArray(incoming)) {
                merged[fk] = merged[fk].map((d, i) => {
                  const e = incoming[i];
                  if (!e || !e.geom || !Array.isArray(e.stops)) return d;
                  if (fk === "heraldic") {
                    const migrated = migrateHeraldicGeom(e.geom);
                    if (!migrated) return d;
                    return { geom: migrated, stops: e.stops };
                  }
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
        familyKey === "smooth"   ? "Profile Geometry"
        : familyKey === "stacked"  ? "Stacked Segments"
        : familyKey === "stacked2" ? "Stacked II Segments"
        : "Heraldic Geometry"
      }>
        {familyKey === "smooth" ? (
          <SmoothProfilePanel geom={geom} upd={upd} />
        ) : (familyKey === "stacked" || familyKey === "stacked2") ? (
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
      <SectionLabel>Section boundaries (t)</SectionLabel>
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
      <SectionLabel>Key radii (normalized)</SectionLabel>
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
      <SectionLabel>Scale (world units)</SectionLabel>
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
      <SectionLabel>Segments (bottom → top)</SectionLabel>
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
      <SectionLabel>Scale (world units)</SectionLabel>
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
   Variable-length segment editor (1–7 segments). The piece is purely segments
   end-to-end — no separate head primitive. Each segment has height, r_bot,
   r_top, and a profile shape from STACKED_SHAPES. */
function HeraldicProfilePanel({ geom, upd, updSegments }) {
  const shapeKeys = Object.keys(STACKED_SHAPES);

  return (
    <>
      <SectionLabel>
        <span style={{ flex: 1 }}>Body segments (bottom → top) — {geom.segments.length}/{HERALDIC_MAX_SEGS}</span>
        <button
          onClick={() => updSegments(n => {
            if (n.segments.length >= HERALDIC_MAX_SEGS) return;
            // Insert a new segment just below the current top, copying its values
            // so the new piece slots in cleanly without sudden geometry changes.
            const top = n.segments[n.segments.length - 1] || { height: 0.10, r_bot: 0.50, r_top: 0.50, profile: "linear" };
            n.segments.splice(n.segments.length - 1, 0, {
              height: top.height * 0.5,
              r_bot:  top.r_bot,
              r_top:  top.r_bot,
              profile: "linear",
            });
          })}
          disabled={geom.segments.length >= HERALDIC_MAX_SEGS}
          style={{ ...btnStyle, fontSize: 9, padding: "1px 6px", opacity: geom.segments.length >= HERALDIC_MAX_SEGS ? 0.4 : 1 }}
          title="Add a new segment just below the topmost (Crown) segment. Caps at 7."
        >+ add</button>
      </SectionLabel>
      {geom.segments.map((seg, i) => (
        <div key={i} style={{ ...rw, gap: 4, padding: "2px 0", borderTop: i === 0 ? "none" : "1px dashed var(--color-border-tertiary)" }}>
          <span style={{ ...lb, minWidth: 48, fontWeight: 500 }} title={`Segment ${i + 1} of ${geom.segments.length}`}>
            {heraldicSegLabel(i, geom.segments.length)}
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
               tip="Segment top radius (normalized to world Radius). The topmost (Crown) segment's r_top sets the platform the head sits on." />
          <select
            value={seg.profile}
            onChange={e => updSegments(n => { n.segments[i].profile = e.target.value; })}
            style={{ ...ist, padding: "2px 4px", fontSize: 10, textAlign: "left" }}
            title="Segment profile shape: linear / concave / convex / step"
          >
            {shapeKeys.map(k => <option key={k} value={k}>{k}</option>)}
          </select>
          <button
            onClick={() => updSegments(n => {
              if (n.segments.length <= HERALDIC_MIN_SEGS) return;
              n.segments.splice(i, 1);
            })}
            disabled={geom.segments.length <= HERALDIC_MIN_SEGS}
            style={{ ...btnStyle, fontSize: 9, padding: "0 5px", opacity: geom.segments.length <= HERALDIC_MIN_SEGS ? 0.4 : 1, marginLeft: 2 }}
            title={geom.segments.length <= HERALDIC_MIN_SEGS ? "Can't remove — at least 1 segment is required." : `Remove the ${heraldicSegLabel(i, geom.segments.length)} segment.`}
          >×</button>
        </div>
      ))}

      <SectionLabel>Scale (world units)</SectionLabel>
      <div style={rw}>
        <span style={lb} title="Total piece height in world units">Height</span>
        <Num value={geom.height} min={0.5} max={10} w={42}
             onChange={v => upd(n => { n.height = v; })}
             tip="Total piece height in world units. Segment heights are normalized to fill this." />
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