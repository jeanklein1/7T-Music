import { useState, useEffect, useRef, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T PALM DESIGNER
   Procedural palm tree — tapered trunk with lean + radial frond crown.
   3 tiers: Sapling / Coastal / Royal.
   Follows cartridge.hpp entity conventions: μ/σ tier params, hash pipeline,
   deterministic spawning, collision solid, C++ export.
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Sapling", "Coastal", "Royal"];

/* ═══ HASH — exact port from cartridge.hpp ═══ */
function cpuH(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return ((h >>> 16) ^ h) >>> 0;
}
function hf(s, p) { return cpuH(s, p) / 0xFFFFFFFF; }

/* ═══ PALETTES ═══ */
const TRUNK_PALETTE = [
  { name: "Warm bark",     c: [0.52, 0.40, 0.28] },
  { name: "Grey bark",     c: [0.48, 0.45, 0.40] },
  { name: "Dark bark",     c: [0.35, 0.28, 0.20] },
  { name: "Bleached",      c: [0.62, 0.58, 0.50] },
  { name: "Reddish",       c: [0.55, 0.35, 0.25] },
];
const FROND_PALETTE = [
  { name: "Lush green",    c: [0.28, 0.52, 0.22] },
  { name: "Olive",         c: [0.40, 0.48, 0.25] },
  { name: "Dark green",    c: [0.18, 0.38, 0.18] },
  { name: "Yellow-green",  c: [0.48, 0.55, 0.22] },
  { name: "Dusty sage",    c: [0.38, 0.48, 0.35] },
  { name: "Dry frond",     c: [0.58, 0.52, 0.32] },
];
const SANDSTONE = { c: [0.52, 0.40, 0.28], variance: 0.04 };

/* ═══ PER-INSTANCE COLOR — hash-driven palette selection ═══
   Same pattern as cartridge.hpp column/arch/pyramid color logic.
   seed drives deterministic palette pick + per-channel jitter.
   Property indices: 920=trunk override roll, 921-923=trunk RGB var,
                     930=frond override roll, 931-933=frond RGB var,
                     940=aged frond override roll, 941-943=aged RGB var,
                     925=trunk palette idx, 935=frond palette idx, 945=aged palette idx. */
function resolveColors(T, seed) {
  const tVar = T.trunk_var || 0;
  const fVar = T.frond_var || 0;
  let tc = [...T.trunk_color];
  let fc = [...T.frond_color];
  let fa = [...(T.frond_color_aged || T.frond_color)];
  // Trunk: palette override or base + variance
  if (hf(seed, 920) < T.color_over) {
    const pi = cpuH(seed, 925) % TRUNK_PALETTE.length;
    tc = [...TRUNK_PALETTE[pi].c];
  }
  tc[0] += (hf(seed, 921) - 0.5) * tVar * 2;
  tc[1] += (hf(seed, 922) - 0.5) * tVar * 2;
  tc[2] += (hf(seed, 923) - 0.5) * tVar * 2;
  // Frond (young/inner): palette override or base + variance
  if (hf(seed, 930) < T.color_over) {
    const pi = cpuH(seed, 935) % FROND_PALETTE.length;
    fc = [...FROND_PALETTE[pi].c];
  }
  fc[0] += (hf(seed, 931) - 0.5) * fVar * 2;
  fc[1] += (hf(seed, 932) - 0.5) * fVar * 2;
  fc[2] += (hf(seed, 933) - 0.5) * fVar * 2;
  // Frond (aged/outer): independent palette roll + variance
  if (hf(seed, 940) < T.color_over) {
    const pi = cpuH(seed, 945) % FROND_PALETTE.length;
    fa = [...FROND_PALETTE[pi].c];
  }
  fa[0] += (hf(seed, 941) - 0.5) * fVar * 2;
  fa[1] += (hf(seed, 942) - 0.5) * fVar * 2;
  fa[2] += (hf(seed, 943) - 0.5) * fVar * 2;
  return {
    trunkCol: tc.map(v => Math.max(0, Math.min(1, v))),
    frondCol: fc.map(v => Math.max(0, Math.min(1, v))),
    frondAgedCol: fa.map(v => Math.max(0, Math.min(1, v))),
  };
}

/* ═══ TIER DEFAULTS ═══ */
function defaultTier(idx) {
  const D = [
    // Sapling — short, thin, few fronds
    { height: 10.0, height_s: 2.0,
      base_r: 0.35, base_r_s: 0.06,
      top_r: 0.18, top_r_s: 0.03,
      lean: 0.08, lean_s: 0.04,
      bark_rings: 12, bark_rings_s: 3,
      bark_depth: 0.04, bark_depth_s: 0.01,
      frond_count: 6, frond_count_s: 1,
      frond_len: 5.0, frond_len_s: 1.0,
      frond_width: 0.8, frond_width_s: 0.15,
      frond_droop: 0.55, frond_droop_s: 0.10,
      frond_arch: 0.25, frond_arch_s: 0.08,
      crown_spread: 0.65, crown_spread_s: 0.10,
      crown_skirt: 0.20, crown_skirt_s: 0.08,
      solid_pad: 0.2, solid_pad_s: 0.05,
      solid_h: 1.2, solid_h_s: 0.3,
      edge_blend: 0.3, edge_blend_s: 0.05,
      color_over: 0.15, burial: 0.15,
      trunk_var: 0.06, frond_var: 0.06,
      trunk_segs: 12, frond_segs: 8,
      weight: 0.45,
      trunk_color: [0.52, 0.40, 0.28],
      frond_color: [0.28, 0.52, 0.22],
      frond_color_aged: [0.52, 0.48, 0.28] },
    // Coastal — classic palm proportions
    { height: 22.0, height_s: 4.0,
      base_r: 0.55, base_r_s: 0.08,
      top_r: 0.28, top_r_s: 0.04,
      lean: 0.14, lean_s: 0.06,
      bark_rings: 20, bark_rings_s: 4,
      bark_depth: 0.06, bark_depth_s: 0.015,
      frond_count: 10, frond_count_s: 2,
      frond_len: 8.0, frond_len_s: 1.5,
      frond_width: 1.0, frond_width_s: 0.20,
      frond_droop: 0.65, frond_droop_s: 0.12,
      frond_arch: 0.35, frond_arch_s: 0.10,
      crown_spread: 0.75, crown_spread_s: 0.12,
      crown_skirt: 0.40, crown_skirt_s: 0.10,
      solid_pad: 0.3, solid_pad_s: 0.08,
      solid_h: 1.5, solid_h_s: 0.3,
      edge_blend: 0.4, edge_blend_s: 0.08,
      color_over: 0.20, burial: 0.15,
      trunk_var: 0.06, frond_var: 0.06,
      trunk_segs: 16, frond_segs: 10,
      weight: 0.35,
      trunk_color: [0.52, 0.40, 0.28],
      frond_color: [0.28, 0.52, 0.22],
      frond_color_aged: [0.55, 0.50, 0.30] },
    // Royal — tall, dramatic crown
    { height: 42.0, height_s: 8.0,
      base_r: 0.90, base_r_s: 0.12,
      top_r: 0.40, top_r_s: 0.06,
      lean: 0.06, lean_s: 0.03,
      bark_rings: 30, bark_rings_s: 5,
      bark_depth: 0.08, bark_depth_s: 0.02,
      frond_count: 16, frond_count_s: 3,
      frond_len: 12.0, frond_len_s: 2.5,
      frond_width: 1.4, frond_width_s: 0.25,
      frond_droop: 0.75, frond_droop_s: 0.12,
      frond_arch: 0.40, frond_arch_s: 0.12,
      crown_spread: 0.85, crown_spread_s: 0.10,
      crown_skirt: 0.50, crown_skirt_s: 0.12,
      solid_pad: 0.5, solid_pad_s: 0.10,
      solid_h: 2.0, solid_h_s: 0.5,
      edge_blend: 0.5, edge_blend_s: 0.10,
      color_over: 0.25, burial: 0.20,
      trunk_var: 0.08, frond_var: 0.08,
      trunk_segs: 20, frond_segs: 12,
      weight: 0.20,
      trunk_color: [0.52, 0.40, 0.28],
      frond_color: [0.18, 0.38, 0.18],
      frond_color_aged: [0.50, 0.45, 0.28] },
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

/* ═══ TRUNK PROFILE ═══
   Surface of revolution with progressive lean offset.
   Bark rings modulate radius slightly at regular intervals. */
function trunkProfile(T) {
  const pts = [];
  const rings = Math.max(8, Math.round(T.bark_rings));
  const steps = Math.max(rings * 2, 20);
  for (let i = 0; i <= steps; i++) {
    const t = i / steps;
    // Radius: linear taper from base to top
    let r = lerp(T.base_r, T.top_r, t);
    // Bark ring modulation (sinusoidal bumps)
    const ringPhase = t * rings * Math.PI * 2;
    r += Math.sin(ringPhase) * T.bark_depth * (1.0 - t * 0.5); // less pronounced at top
    // Lean: quadratic offset in X, increasing with height
    const leanX = T.lean * T.height * t * t;
    const y = t * T.height;
    // Color: darken at base, lighter at top
    const shade = 0.85 + 0.15 * t;
    const col = T.trunk_color.map(c => c * shade);
    pts.push({ r: Math.max(0.01, r), y, leanX, color: col });
  }
  return pts;
}

/* ═══ FROND GEOMETRY ═══
   Each frond radiates from the crown along a 3D direction vector.
   Golden-angle azimuth packing. Elevation ranges from -crown_skirt
   (outer/old fronds dipping below horizontal) to +crown_spread
   (inner/young fronds pointing upward). Midrib follows the initial
   direction; gravity (droop) pulls the tip downward absolutely.
   Returns array of quads [{verts,color}]. */
function frondGeometry(T, frondIdx, totalFronds, crownY, crownLeanX) {
  const quads = [];
  const segs = Math.max(4, T.frond_segs);

  // Golden angle azimuth — natural Fibonacci packing
  const goldenAngle = Math.PI * (3 - Math.sqrt(5));
  const baseAngle = frondIdx * goldenAngle;

  // Elevation: outer (old) fronds dip below horizontal, inner (young) point up
  const rank = frondIdx / Math.max(1, totalFronds - 1); // 0=outer/old → 1=inner/young
  const elevTop = (T.crown_spread || 0) * Math.PI * 0.42;     // ~75° at spread=1.0
  const elevBot = -(T.crown_skirt || 0) * Math.PI * 0.25;     // ~-45° at skirt=1.0
  const elevation = elevBot + (elevTop - elevBot) * rank;
  // Inner fronds droop less (young, stiff) and are slightly shorter
  const droopScale = 0.25 + 0.75 * (1.0 - rank);
  const lenScale = 0.6 + 0.4 * (1.0 - rank);            // inner = 60-100% length

  // 3D forward direction from crown point
  const cosAz = Math.cos(baseAngle), sinAz = Math.sin(baseAngle);
  const cosEl = Math.cos(elevation), sinEl = Math.sin(elevation);
  const fwd = [cosAz * cosEl, sinEl, sinAz * cosEl];

  // Perpendicular for frond width (cross forward × world-up)
  let right;
  if (sinEl > 0.95) {
    // Nearly vertical — fall back to azimuth-perpendicular
    right = [-sinAz, 0, cosAz];
  } else {
    right = normalize3(cross3(fwd, [0, 1, 0]));
  }

  // Midrib curve points
  const midrib = [];
  const frondLen = T.frond_len * lenScale;
  for (let i = 0; i <= segs; i++) {
    const t = i / segs;
    const dist = t * frondLen;

    // Follow forward direction + absolute vertical displacement
    const archUp = T.frond_arch * frondLen * Math.sin(t * Math.PI * 0.4);
    const droopDown = T.frond_droop * frondLen * t * t * t * droopScale;
    const dy = archUp - droopDown;

    const x = crownLeanX + fwd[0] * dist;
    const y = crownY + fwd[1] * dist + dy;
    const z = fwd[2] * dist;

    // Width: tapers to tip, ramps in at base
    const w = T.frond_width * (1.0 - t * 0.85) * (0.3 + 0.7 * Math.min(1, t * 3));
    const perpX = right[0] * w * 0.5;
    const perpY = right[1] * w * 0.5;
    const perpZ = right[2] * w * 0.5;

    // Color: blend young→aged by rank, plus tip aging and per-frond hue shift
    const aged = T.frond_color_aged || T.frond_color;
    const baseCol = lerpC(aged, T.frond_color, rank);       // outer=aged, inner=young
    const tipAge = Math.min(1, t * t * (1.0 - rank * 0.7)); // tips age faster on old fronds
    const segCol = lerpC(baseCol, aged, tipAge * 0.3);      // subtle tip tint
    const shade = 0.75 + 0.25 * t;
    const frondVar = (frondIdx % 3) * 0.03;
    const col = segCol.map((c, ci) => Math.min(1, c * shade + (ci === 1 ? frondVar : -frondVar * 0.5)));
    midrib.push({ x, y, z, perpX, perpY, perpZ, color: col });
  }

  // Build quads along midrib
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
/* Rotated WORLD space only — never projects, draws, clears, or sees W/H/zoom/pan.
   Scale-free by design: that is what lets tiers merge into one sorted list. */
function buildFaces(T, rotY, tilt, xOff, midY) {
  const RENDER_SEGS = Math.min(T.trunk_segs, 24);
  const profile = trunkProfile(T);
  const lightDir = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);

  const rotV = (v) => {
    const vx = v[0] + xOff;          // world-space row offset, BEFORE the Y-rotation
    const vy = v[1] - midY;
    const x1 = vx * cosR + v[2] * sinR;
    const z1 = -vx * sinR + v[2] * cosR;
    return [x1, vy * cosT - z1 * sinT, vy * sinT + z1 * cosT];
  };

  const faces = [];
  const addTri = (a, b, c, col) => {
    const n = normalize3(cross3(sub3(b, a), sub3(c, a)));
    const d = -(n[0] * lightDir[0] + n[1] * lightDir[1] + n[2] * lightDir[2]);
    const light = Math.max(0.25, Math.min(1.0, 0.35 + d * 0.65));
    const avgZ = (a[2] + b[2] + c[2]) / 3;
    faces.push({ verts: [a, b, c], col, light, avgZ });
  };
  const addQuad = (a, b, c, d, col) => { addTri(a, b, c, col); addTri(a, c, d, col); };

  // ── Trunk (surface of revolution with lean) ──
  for (let i = 0; i < profile.length - 1; i++) {
    const p0 = profile[i], p1 = profile[i + 1];
    for (let s = 0; s < RENDER_SEGS; s++) {
      const a0 = (s / RENDER_SEGS) * Math.PI * 2;
      const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
      const ca0 = Math.cos(a0), sa0 = Math.sin(a0);
      const ca1 = Math.cos(a1), sa1 = Math.sin(a1);
      const v00 = rotV([p0.leanX + ca0 * p0.r, p0.y, sa0 * p0.r]);
      const v10 = rotV([p1.leanX + ca0 * p1.r, p1.y, sa0 * p1.r]);
      const v11 = rotV([p1.leanX + ca1 * p1.r, p1.y, sa1 * p1.r]);
      const v01 = rotV([p0.leanX + ca1 * p0.r, p0.y, sa1 * p0.r]);
      const col = lerpC(p0.color, p1.color, 0.5);
      addQuad(v00, v10, v11, v01, col);
    }
  }

  // ── Crown cap (small hemisphere at trunk top) ──
  const topP = profile[profile.length - 1];
  const crownR = T.top_r * 1.3;
  const crownY = topP.y;
  const crownLeanX = topP.leanX;
  const crownCol = lerpC(T.trunk_color, T.frond_color, 0.4);
  for (let s = 0; s < RENDER_SEGS; s++) {
    const a0 = (s / RENDER_SEGS) * Math.PI * 2;
    const a1 = ((s + 1) % RENDER_SEGS) / RENDER_SEGS * Math.PI * 2;
    const tip = rotV([crownLeanX, crownY + crownR * 0.6, 0]);
    const b0 = rotV([crownLeanX + Math.cos(a0) * crownR, crownY, Math.sin(a0) * crownR]);
    const b1 = rotV([crownLeanX + Math.cos(a1) * crownR, crownY, Math.sin(a1) * crownR]);
    addTri(tip, b0, b1, crownCol);
  }

  // ── Fronds ──
  const nFronds = Math.max(3, Math.round(T.frond_count));
  for (let f = 0; f < nFronds; f++) {
    const quads = frondGeometry(T, f, nFronds, crownY + crownR * 0.3, crownLeanX);
    for (const q of quads) {
      const rv = q.verts.map(v => rotV(v));
      addQuad(rv[0], rv[1], rv[2], rv[3], q.color);
    }
  }

  return faces;
}

/* ═══ CAMERA PRIMITIVES — world→screen, and the depth-sorted fill ═══ */
function makeProject(W, H, scale, panX, panY) {
  return (v) => [W / 2 + panX + v[0] * scale, H / 2 + panY - v[1] * scale];
}

function drawFaces(ctx, faces, project) {
  faces.sort((a, b) => a.avgZ - b.avgZ);
  for (const f of faces) {
    const pts = f.verts.map(project);
    ctx.fillStyle = rgb01(f.col[0] * f.light, f.col[1] * f.light, f.col[2] * f.light);
    ctx.beginPath();
    ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath();
    ctx.fill();
  }
}

function tierExtent(T) { return Math.max(T.height, T.frond_len * 2, T.base_r * 4); }
function tierHeight(T) { return T.height; }

/* Single view — keeps render3D's original public signature, so the call sites
   and effect deps from [2] are untouched. */
function render3D(ctx, W, H, T, rotY, tilt, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);
  const scale = Math.min(W, H) * 0.38 / (tierExtent(T) * 0.5) * zoom;
  const midY = T.height * 0.5;
  drawFaces(ctx, buildFaces(T, rotY, tilt, 0, midY), makeProject(W, H, scale, panX, panY));
}

/* All tiers in ONE shared scene: bases on a common line, spaced along world x,
   the row rotating as a unit. Every tier's faces merge into a single
   depth-sorted list so they occlude correctly at any angle. Scale is shared and
   taken from the largest tier — the RELATIVE SIZE IS THE COMPARISON, so a tier
   is never normalised to its own slot. */
function render3DCompare(ctx, W, H, tiers, rotY, tilt, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);
  const exts = tiers.map(tierExtent);
  const maxExt = Math.max(...exts, 1e-4);
  const maxH = Math.max(...tiers.map(tierHeight), 1e-4);

  // Slot per tier: its own footprint plus a constant gutter, so a large tier
  // gets more room and neighbours never touch.
  const slotW = exts.map(e => e * 0.7 + maxExt * 0.18);
  const totalW = slotW.reduce((a, b) => a + b, 0);
  const scale = Math.min(W * 0.92 / totalW, H * 0.72 / maxExt) * zoom;

  let cum = -totalW / 2;
  const xOffs = slotW.map(w => { const c = cum + w / 2; cum += w; return c; });

  const midY = maxH * 0.5;                       // shared → bases align
  const all = [];
  tiers.forEach((T, i) => all.push(...buildFaces(T, rotY, tilt, xOffs[i], midY)));
  drawFaces(ctx, all, makeProject(W, H, scale, panX, panY));
}

/* ═══ 2D PROFILE ═══ */
function render2D(ctx, W, H, T) {
  ctx.clearRect(0, 0, W, H);
  ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const profile = trunkProfile(T);
  if (profile.length < 2) return;
  const mg = { l: 20, r: 20, t: 12, b: 12 }, pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  let maxR = 0, maxLean = 0, minY = Infinity, maxY = -Infinity;
  for (const p of profile) {
    maxR = Math.max(maxR, p.r + Math.abs(p.leanX));
    maxLean = Math.max(maxLean, Math.abs(p.leanX));
    minY = Math.min(minY, p.y);
    maxY = Math.max(maxY, p.y);
  }
  // Include frond extent (account for upward spread + downward skirt)
  const elevTop2D = (T.crown_spread || 0) * Math.PI * 0.42;
  const elevBot2D = (T.crown_skirt || 0) * Math.PI * 0.25;
  maxR = Math.max(maxR, T.frond_len + maxLean);
  maxY = Math.max(maxY, T.height + T.frond_len * Math.sin(elevTop2D) + T.frond_arch * T.frond_len);
  minY = Math.min(minY, T.height - T.frond_len * Math.sin(elevBot2D) - T.frond_droop * T.frond_len);

  const totalH = maxY - minY || 1, scaleY = ph / totalH, scaleR = pw / 2 / (maxR * 1.1 || 1);
  const sc = Math.min(scaleY, scaleR), cx = W / 2, baseY = mg.t + ph - (0 - minY) * sc;
  const toS = (x, y) => [cx + x * sc, baseY - (y - minY) * sc];

  // Trunk silhouette
  ctx.fillStyle = "rgba(82,64,40,0.15)";
  ctx.beginPath();
  const [fx, fy] = toS(profile[0].leanX + profile[0].r, profile[0].y);
  ctx.moveTo(fx, fy);
  for (let i = 1; i < profile.length; i++) {
    const [sx, sy] = toS(profile[i].leanX + profile[i].r, profile[i].y);
    ctx.lineTo(sx, sy);
  }
  for (let i = profile.length - 1; i >= 0; i--) {
    const [sx, sy] = toS(profile[i].leanX - profile[i].r, profile[i].y);
    ctx.lineTo(sx, sy);
  }
  ctx.closePath(); ctx.fill();

  // Trunk outline
  ctx.lineWidth = 1.5;
  for (let side = 0; side < 2; side++) {
    const sign = side === 0 ? 1 : -1;
    for (let i = 0; i < profile.length - 1; i++) {
      const p0 = profile[i], p1 = profile[i + 1];
      ctx.strokeStyle = rgb01(...lerpC(p0.color, p1.color, 0.5));
      ctx.beginPath();
      const [x0, y0] = toS(p0.leanX + p0.r * sign, p0.y);
      const [x1, y1] = toS(p1.leanX + p1.r * sign, p1.y);
      ctx.moveTo(x0, y0); ctx.lineTo(x1, y1); ctx.stroke();
    }
  }

  // Frond midribs (2D side projection — golden angle + elevation)
  const topP = profile[profile.length - 1];
  const nFronds = Math.max(3, Math.round(T.frond_count));
  const goldenAngle2D = Math.PI * (3 - Math.sqrt(5));
  ctx.lineWidth = 1;
  for (let f = 0; f < nFronds; f++) {
    const azimuth = f * goldenAngle2D;
    const cosAz = Math.cos(azimuth);
    const rank = f / Math.max(1, nFronds - 1);
    const eTop2 = (T.crown_spread || 0) * Math.PI * 0.42;
    const eBot2 = -(T.crown_skirt || 0) * Math.PI * 0.25;
    const elev = eBot2 + (eTop2 - eBot2) * rank;
    const cosEl = Math.cos(elev), sinEl = Math.sin(elev);
    const droopSc = 0.25 + 0.75 * (1.0 - rank);
    const lenSc = 0.6 + 0.4 * (1.0 - rank);
    const fLen = T.frond_len * lenSc;
    // Project 3D direction into side view (X-Y): horizontal component = cosAz * cosEl
    const fwdX = cosAz * cosEl, fwdY = sinEl;
    const alpha = 0.25 + 0.35 * Math.abs(cosAz); // fade fronds facing away
    const aged2D = T.frond_color_aged || T.frond_color;
    const blended = lerpC(aged2D, T.frond_color, rank);
    ctx.strokeStyle = `rgba(${Math.round(blended[0] * 255)},${Math.round(blended[1] * 255)},${Math.round(blended[2] * 255)},${alpha.toFixed(2)})`;
    ctx.beginPath();
    const segs2 = 12;
    for (let i = 0; i <= segs2; i++) {
      const t = i / segs2;
      const dist = t * fLen;
      const archUp = T.frond_arch * fLen * Math.sin(t * Math.PI * 0.4);
      const droopDown = T.frond_droop * fLen * t * t * t * droopSc;
      const dy = fwdY * dist + archUp - droopDown;
      const dx = fwdX * dist;
      const [sx, sy] = toS(topP.leanX + dx, topP.y + dy);
      if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
    }
    ctx.stroke();
  }

  // Ground line
  const [gx0, gy] = toS(-maxR * 1.1, 0);
  const [gx1] = toS(maxR * 1.1, 0);
  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(gx0, gy); ctx.lineTo(gx1, gy); ctx.stroke(); ctx.setLineDash([]);

  // Labels
  ctx.fillStyle = "rgba(255,255,255,0.3)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  const [, topSy] = toS(0, maxY);
  ctx.fillText(`h=${T.height.toFixed(1)}`, cx, topSy - 4);
  ctx.fillText(`r=${T.base_r.toFixed(2)}`, cx + T.base_r * sc * 0.5, baseY + 12);
}

/* ═══ C++ EXPORT ═══ */
function genCpp(tiers) {
  const f = (v, d = 2) => { const s = v.toFixed(d); return s.includes(".") ? s + "f" : s + ".0f"; };
  const lines = [];
  lines.push("// ─── Palm Tier Definitions ────────────────────────────────────────────");
  lines.push("//");
  lines.push("//                         h_μ    σ    bR_μ  σ    tR_μ  σ    lean   σ    bkR   σ    bkD   σ    fC_μ  σ    fL_μ  σ    fW_μ  σ    fDr   σ    fAr   σ    cSp   σ    cSk   σ    sp_μ  σ    sH_μ  σ    eb_μ  σ     col%  bur  tVar fVar  tSeg fSeg wt");
  lines.push("static constexpr PalmTierParams PALM_TIERS[] = {");
  tiers.forEach((T, i) => {
    const vals = [
      T.height, T.height_s, T.base_r, T.base_r_s, T.top_r, T.top_r_s,
      T.lean, T.lean_s, T.bark_rings, T.bark_rings_s, T.bark_depth, T.bark_depth_s,
      T.frond_count, T.frond_count_s, T.frond_len, T.frond_len_s,
      T.frond_width, T.frond_width_s, T.frond_droop, T.frond_droop_s,
      T.frond_arch, T.frond_arch_s, T.crown_spread, T.crown_spread_s,
      T.crown_skirt, T.crown_skirt_s,
      T.solid_pad, T.solid_pad_s, T.solid_h, T.solid_h_s, T.edge_blend, T.edge_blend_s,
      T.color_over, T.burial, T.trunk_var, T.frond_var,
    ].map(v => f(v));
    vals.push(String(T.trunk_segs), String(T.frond_segs), f(T.weight));
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

const STORAGE_KEY = "7t:palm:tiers";
const DEFAULTS = () => [0, 1, 2].map(defaultTier);
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

/* ═══ MAIN COMPONENT ═══ */
export default function PalmDesigner() {
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
  // Resolved tier: hash-derived colors applied for preview rendering
  const resolved = resolveColors(T, seed);
  const RT = { ...T, trunk_color: resolved.trunkCol, frond_color: resolved.frondCol, frond_color_aged: resolved.frondAgedCol };
  // Compare renders the whole row, so resolve each tier's colours the same way RT does.
  const cmpTiers = tiers.map(t => { const rc = resolveColors(t, seed); return { ...t, trunk_color: rc.trunkCol, frond_color: rc.frondCol, frond_color_aged: rc.frondAgedCol }; });

  const [rotY, setRotY] = useState(0.4);
  const [tilt, setTilt] = useState(0.15);
  const [autoRot, setAutoRot] = useState(true);
  const [compareMode, setCompareMode] = useState(false);
  const [zoom, setZoom] = useState(1.0);
  const [panX, setPanX] = useState(0);
  const [panY, setPanY] = useState(0);
  const cv3dRef = useRef(null);
  const cv2dRef = useRef(null);
  const animRef = useRef(null);

  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);

  const defaultOrder = ["trunk", "crown", "solid", "appearance", "quality", "export"];
  const [panelOrder, setPanelOrder] = useState([...defaultOrder]);
  const [dockKey, setDockKey] = useState(0);
  const resetAll = () => { setTiers(DEFAULTS()); setDockKey(k => k + 1); };
  const resetTier = () => setTiers(prev => { const n = [...prev]; n[tierIdx] = defaultTier(tierIdx); return n; });
  const cppCode = genCpp(tiers);


  // 3D render loop
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
        if (compareMode) render3DCompare(ctx, rect.width, rect.height, cmpTiers, r, tilt, zoom, panX, panY);
        else render3D(ctx, rect.width, rect.height, RT, r, tilt, zoom, panX, panY);
      }
      if (c2) {
        const rect = c2.parentElement.getBoundingClientRect();
        c2.width = rect.width * devicePixelRatio;
        c2.height = rect.height * devicePixelRatio;
        const ctx = c2.getContext("2d");
        ctx.setTransform(devicePixelRatio, 0, 0, devicePixelRatio, 0, 0);
        render2D(ctx, rect.width, rect.height, RT);
      }
      frame++;
      animRef.current = requestAnimationFrame(tick);
    };
    tick();
    return () => cancelAnimationFrame(animRef.current);
  }, [RT, cmpTiers, compareMode, rotY, tilt, autoRot, zoom, panX, panY]);

  // Camera. Attached via the canvas's onPointerDown JSX prop, NOT
  // addEventListener — React binds at render time, so the listener cannot be
  // lost to the Loading… placeholder's mount ordering.
  const onPointerDown3D = useCallback(e => {
    const sx = e.clientX, sy = e.clientY;
    // Right button pans in screen pixels, independent of zoom/scale.
    if (e.button === 2) {
      e.preventDefault();
      const spx = panX, spy = panY;
      const onMoveP = ev => { setPanX(spx + (ev.clientX - sx)); setPanY(spy + (ev.clientY - sy)); };
      const onUpP = () => {
        window.removeEventListener("pointermove", onMoveP);
        window.removeEventListener("pointerup", onUpP);
      };
      window.addEventListener("pointermove", onMoveP);
      window.addEventListener("pointerup", onUpP);
      return;
    }
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
  }, [rotY, tilt, panX, panY]);

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

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "#555" }}>Loading…</div>;

  return (
    <div style={{ display: "flex", gap: 8, height: "100vh", padding: 8, fontFamily: "'JetBrains Mono', monospace", fontSize: 11, overflow: "hidden", background: "var(--color-background-primary)", color: "var(--color-text-primary)", boxSizing: "border-box", position: "relative" }}>

      {/* LEFT: 3D + 2D canvases */}
      <div style={{ flex: "1 1 55%", display: "flex", flexDirection: "column", gap: 6, minWidth: 0 }}>
        {/* Tier tabs + controls */}
        <div style={{ display: "flex", gap: 4, alignItems: "center", flexWrap: "wrap" }}>
          {TIER_NAMES.map((name, i) => (
            <button key={i} onClick={() => { setTierIdx(i); setCompareMode(false); }} style={{ ...btnStyle, background: i === tierIdx ? "var(--color-background-info)" : "var(--color-background-secondary)", color: i === tierIdx ? "var(--color-text-on-info)" : "var(--color-text-secondary)", fontWeight: i === tierIdx ? 600 : 400 }}>{name}</button>
          ))}
          <button onClick={() => setCompareMode(c => !c)} style={{
            ...btnStyle,
            border: compareMode ? "2px solid var(--color-border-info)" : undefined,
            background: compareMode ? "var(--color-background-info)" : "var(--color-background-secondary)",
            color: compareMode ? "var(--color-text-on-info)" : "var(--color-text-secondary)",
            fontWeight: compareMode ? 600 : 400,
          }} title="Show all tiers side-by-side at shared scale">Compare</button>
          <span style={{ flex: 1 }} />
          <button onClick={() => setAutoRot(!autoRot)} style={{ ...btnStyle, opacity: autoRot ? 1 : 0.5 }}>{autoRot ? "◉ spin" : "○ spin"}</button>
          <button onClick={resetAll} style={btnStyle}>Reset all</button>
          <button onClick={resetTier} style={btnStyle}>Reset tier</button>
          {isModified && <span style={{ fontSize: 9, color: "#666", marginLeft: 4 }}>● modified</span>}
        </div>
        {/* Seed + resolved color preview */}
        <div style={{ display: "flex", gap: 6, alignItems: "center", fontSize: 10 }}>
          <span style={{ color: "var(--color-text-secondary)" }}>seed</span>
          <Num value={seed} min={0} max={99999} step={1} w={52} onChange={v => setSeed(Math.round(v))} />
          <button onClick={() => setSeed(Math.floor(Math.random() * 99999))} style={btnStyle}>reroll</button>
          <span style={{ color: "var(--color-text-tertiary)", marginLeft: 4 }}>trunk</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.trunkCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
          <span style={{ color: "var(--color-text-tertiary)" }}>frond</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.frondCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
          <span style={{ color: "var(--color-text-tertiary)" }}>aged</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.frondAgedCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
        </div>
        {/* 3D viewport */}
        <div style={{ flex: "1 1 65%", position: "relative", background: "#1a1a1e", borderRadius: 8, overflow: "hidden", cursor: "grab", minHeight: 180 }}>
          <canvas
            ref={cv3dRef}
            onPointerDown={onPointerDown3D}
            onContextMenu={e => e.preventDefault()}
            style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }}
          />
          <div
            onClick={() => { setZoom(1); setPanX(0); setPanY(0); }}
            title="Click to reset view"
            style={{ position: "absolute", top: 4, right: 8, fontSize: 9,
                     color: "rgba(255,255,255,0.4)", cursor: "pointer",
                     userSelect: "none", padding: "2px 4px" }}
          >{Math.round(zoom * 100)}%</div>
          <div style={{ position: "absolute", bottom: 4, left: 8, fontSize: 9,
                        color: "rgba(255,255,255,0.3)" }}>drag to rotate · right-drag to pan · scroll to zoom</div>
        </div>
        {/* 2D profile */}
        <div style={{ flex: "0 0 28%", background: "#111114", borderRadius: 8, overflow: "hidden", minHeight: 100 }}>
          <canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} />
        </div>
      </div>

      {/* RIGHT: parameter panels */}
      <div style={{ flex: "0 0 310px", overflowY: "auto", overflowX: "hidden", paddingRight: 2 }}>

        {panelOrder.map(pid => {
          const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };

          switch (pid) {

            case "trunk": return (<DragPanel {...dp} title="Trunk Geometry" ini={true}>
              <MuSigma label="Height" mu={T.height} muSet={v => upd(n => { n.height = v; })} sigma={T.height_s} sigmaSet={v => upd(n => { n.height_s = v; })} muMin={2} muMax={80} sMax={20} />
              <MuSigma label="Base radius" mu={T.base_r} muSet={v => upd(n => { n.base_r = v; })} sigma={T.base_r_s} sigmaSet={v => upd(n => { n.base_r_s = v; })} muMin={0.05} muMax={3} sMax={1} />
              <MuSigma label="Top radius" mu={T.top_r} muSet={v => upd(n => { n.top_r = v; })} sigma={T.top_r_s} sigmaSet={v => upd(n => { n.top_r_s = v; })} muMin={0.02} muMax={2} sMax={0.5} />
              <MuSigma label="Lean" mu={T.lean} muSet={v => upd(n => { n.lean = v; })} sigma={T.lean_s} sigmaSet={v => upd(n => { n.lean_s = v; })} muMax={0.5} sMax={0.2} />
              <MuSigma label="Bark rings" mu={T.bark_rings} muSet={v => upd(n => { n.bark_rings = Math.round(v); })} sigma={T.bark_rings_s} sigmaSet={v => upd(n => { n.bark_rings_s = v; })} muMin={3} muMax={50} sMax={10} muW={34} />
              <MuSigma label="Bark depth" mu={T.bark_depth} muSet={v => upd(n => { n.bark_depth = v; })} sigma={T.bark_depth_s} sigmaSet={v => upd(n => { n.bark_depth_s = v; })} muMax={0.3} sMax={0.1} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Lean = quadratic offset (fraction of height). Bark rings = sinusoidal radius modulation count. Bark depth = amplitude of ring bumps.</div>
            </DragPanel>);

            case "crown": return (<DragPanel {...dp} title="Frond Crown" ini={true}>
              <MuSigma label="Frond count" mu={T.frond_count} muSet={v => upd(n => { n.frond_count = Math.round(v); })} sigma={T.frond_count_s} sigmaSet={v => upd(n => { n.frond_count_s = v; })} muMin={3} muMax={24} sMax={5} muW={34} />
              <MuSigma label="Frond length" mu={T.frond_len} muSet={v => upd(n => { n.frond_len = v; })} sigma={T.frond_len_s} sigmaSet={v => upd(n => { n.frond_len_s = v; })} muMin={1} muMax={25} sMax={6} />
              <MuSigma label="Frond width" mu={T.frond_width} muSet={v => upd(n => { n.frond_width = v; })} sigma={T.frond_width_s} sigmaSet={v => upd(n => { n.frond_width_s = v; })} muMin={0.1} muMax={4} sMax={1} />
              <MuSigma label="Droop" mu={T.frond_droop} muSet={v => upd(n => { n.frond_droop = v; })} sigma={T.frond_droop_s} sigmaSet={v => upd(n => { n.frond_droop_s = v; })} muMax={1.5} sMax={0.4} />
              <MuSigma label="Arch" mu={T.frond_arch} muSet={v => upd(n => { n.frond_arch = v; })} sigma={T.frond_arch_s} sigmaSet={v => upd(n => { n.frond_arch_s = v; })} muMax={1.0} sMax={0.3} />
              <MuSigma label="Crown spread" mu={T.crown_spread} muSet={v => upd(n => { n.crown_spread = v; })} sigma={T.crown_spread_s} sigmaSet={v => upd(n => { n.crown_spread_s = v; })} muMax={1.0} sMax={0.3} />
              <MuSigma label="Crown skirt" mu={T.crown_skirt} muSet={v => upd(n => { n.crown_skirt = v; })} sigma={T.crown_skirt_s} sigmaSet={v => upd(n => { n.crown_skirt_s = v; })} muMax={1.0} sMax={0.3} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Spread = how high inner fronds reach (0=horizontal, 1=~75° up). Skirt = how low outer fronds dip below horizontal (0=flat, 1=~45° down). Together they define the elevation range: oldest fronds hang in the skirt, youngest point skyward. Golden-angle azimuth packing.</div>
            </DragPanel>);

            case "solid": return (<DragPanel {...dp} title="Collision Solid">
              <MuSigma label="Padding" mu={T.solid_pad} muSet={v => upd(n => { n.solid_pad = v; })} sigma={T.solid_pad_s} sigmaSet={v => upd(n => { n.solid_pad_s = v; })} muMax={3} sMax={1} />
              <MuSigma label="Height" mu={T.solid_h} muSet={v => upd(n => { n.solid_h = v; })} sigma={T.solid_h_s} sigmaSet={v => upd(n => { n.solid_h_s = v; })} muMax={8} sMax={2} />
              <MuSigma label="Edge blend" mu={T.edge_blend} muSet={v => upd(n => { n.edge_blend = v; })} sigma={T.edge_blend_s} sigmaSet={v => upd(n => { n.edge_blend_s = v; })} muMax={2} sMax={0.5} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Cylindrical solid around trunk base. Pawn blocked by step-height on contact. Padding = extra radius beyond trunk base.</div>
            </DragPanel>);

            case "appearance": return (<DragPanel {...dp} title="Appearance">
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>Trunk</div>
              <PaletteSwatches palette={TRUNK_PALETTE} label="Trunk palette" onPick={c => upd(n => { n.trunk_color = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"g" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.trunk_color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.trunk_color[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.trunk_color), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Fronds (young / inner)</div>
              <PaletteSwatches palette={FROND_PALETTE} label="Frond palette" onPick={c => upd(n => { n.frond_color = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"f" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.frond_color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.frond_color[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.frond_color), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Fronds (aged / outer)</div>
              <PaletteSwatches palette={FROND_PALETTE} label="Aged palette" onPick={c => upd(n => { n.frond_color_aged = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"a" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.frond_color_aged[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.frond_color_aged[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.frond_color_aged), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={rw}>
                <span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v => upd(n => { n.color_over = v; })} />
                <span style={lb}>Burial</span><Num value={T.burial} min={0} max={0.5} w={38} onChange={v => upd(n => { n.burial = v; })} />
              </div>
              <div style={rw}>
                <span style={lb}>Trunk var</span><Num value={T.trunk_var} min={0} max={0.3} w={38} onChange={v => upd(n => { n.trunk_var = v; })} />
                <span style={lb}>Frond var</span><Num value={T.frond_var} min={0} max={0.3} w={38} onChange={v => upd(n => { n.frond_var = v; })} />
              </div>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Override = probability of palette pick vs base color (per spawn). Variance = per-channel RGB jitter (±var). Change seed above to preview different spawn rolls. Each tree gets a unique color from hash(seed, prop).</div>
            </DragPanel>);

            case "quality": return (<DragPanel {...dp} title="Quality & Selection">
              <div style={rw}>
                <span style={lb}>Trunk segs</span><Num value={T.trunk_segs} min={6} max={32} step={1} w={34} onChange={v => upd(n => { n.trunk_segs = Math.round(v); })} />
                <span style={lb}>Frond segs</span><Num value={T.frond_segs} min={4} max={16} step={1} w={34} onChange={v => upd(n => { n.frond_segs = Math.round(v); })} />
                <span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v => upd(n => { n.weight = v; })} />
              </div>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Trunk segs = revolution segments. Frond segs = segments per frond midrib. Weight = tier selection probability.</div>
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
