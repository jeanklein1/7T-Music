import { useState, useEffect, useRef, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T CACTUS DESIGNER
   Columnar cacti — ribbed surface of revolution + optional forking arms.
   3 tiers: Finger / Saguaro / Candelabra.
   Visual reference: tall green vertical accents in Jean's 7T paintings.
   ═══════════════════════════════════════════════════════════════════════ */

const TIER_NAMES = ["Finger", "Saguaro", "Candelabra"];

/* ═══ HASH — exact port from cartridge.hpp ═══ */
function cpuH(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return ((h >>> 16) ^ h) >>> 0;
}
function hf(s, p) { return cpuH(s, p) / 0xFFFFFFFF; }

/* ═══ PALETTES ═══ */
const BODY_PALETTE = [
  { name: "Classic green",  c: [0.22, 0.42, 0.18] },
  { name: "Grey-green",     c: [0.30, 0.40, 0.28] },
  { name: "Blue-green",     c: [0.20, 0.38, 0.30] },
  { name: "Olive",          c: [0.35, 0.42, 0.22] },
  { name: "Dark cactus",    c: [0.15, 0.30, 0.15] },
  { name: "Pale sage",      c: [0.38, 0.48, 0.35] },
];
const RIB_PALETTE = [
  { name: "Rib highlight",  c: [0.30, 0.52, 0.25] },
  { name: "Rib warm",       c: [0.35, 0.48, 0.22] },
  { name: "Rib pale",       c: [0.40, 0.50, 0.32] },
];

/* ═══ PER-INSTANCE COLOR ═══ */
function resolveColors(T, seed) {
  const v = T.color_var || 0;
  let bc = [...T.body_color];
  let rc = [...T.rib_color];
  if (hf(seed, 1020) < T.color_over) {
    bc = [...BODY_PALETTE[cpuH(seed, 1025) % BODY_PALETTE.length].c];
  }
  bc[0] += (hf(seed, 1021) - 0.5) * v * 2;
  bc[1] += (hf(seed, 1022) - 0.5) * v * 2;
  bc[2] += (hf(seed, 1023) - 0.5) * v * 2;
  if (hf(seed, 1030) < T.color_over) {
    rc = [...RIB_PALETTE[cpuH(seed, 1035) % RIB_PALETTE.length].c];
  }
  rc[0] += (hf(seed, 1031) - 0.5) * v * 2;
  rc[1] += (hf(seed, 1032) - 0.5) * v * 2;
  rc[2] += (hf(seed, 1033) - 0.5) * v * 2;
  return {
    bodyCol: bc.map(c => Math.max(0, Math.min(1, c))),
    ribCol: rc.map(c => Math.max(0, Math.min(1, c))),
  };
}

/* ═══ TIER DEFAULTS ═══ */
function defaultTier(idx) {
  const D = [
    // Finger — small single column, no arms
    { height: 4.0, height_s: 1.0,
      radius: 0.30, radius_s: 0.06,
      taper: 0.70, taper_s: 0.08,
      ribs: 8, ribs_s: 1,
      rib_depth: 0.12, rib_depth_s: 0.03,
      lean: 0.06, lean_s: 0.03,
      cap_round: 0.60, cap_round_s: 0.10,
      arm_count: 0, arm_count_s: 0,
      arm_height: 0.4, arm_height_s: 0.1,
      arm_length: 1.5, arm_length_s: 0.3,
      arm_radius: 0.18, arm_radius_s: 0.03,
      arm_curve: 0.60, arm_curve_s: 0.15,
      color_over: 0.15, color_var: 0.06,
      trunk_segs: 12, arm_segs: 8,
      weight: 0.45,
      body_color: [0.22, 0.42, 0.18],
      rib_color: [0.30, 0.52, 0.25] },
    // Saguaro — medium-tall, 1-2 arms
    { height: 10.0, height_s: 2.5,
      radius: 0.55, radius_s: 0.10,
      taper: 0.75, taper_s: 0.06,
      ribs: 12, ribs_s: 2,
      rib_depth: 0.15, rib_depth_s: 0.04,
      lean: 0.04, lean_s: 0.02,
      cap_round: 0.65, cap_round_s: 0.10,
      arm_count: 1, arm_count_s: 0.5,
      arm_height: 0.45, arm_height_s: 0.12,
      arm_length: 3.5, arm_length_s: 0.8,
      arm_radius: 0.30, arm_radius_s: 0.06,
      arm_curve: 0.70, arm_curve_s: 0.15,
      color_over: 0.20, color_var: 0.06,
      trunk_segs: 14, arm_segs: 10,
      weight: 0.35,
      body_color: [0.22, 0.42, 0.18],
      rib_color: [0.30, 0.52, 0.25] },
    // Candelabra — tall, 2-4 arms
    { height: 18.0, height_s: 4.0,
      radius: 0.75, radius_s: 0.12,
      taper: 0.80, taper_s: 0.05,
      ribs: 14, ribs_s: 2,
      rib_depth: 0.18, rib_depth_s: 0.04,
      lean: 0.03, lean_s: 0.02,
      cap_round: 0.70, cap_round_s: 0.10,
      arm_count: 3, arm_count_s: 1,
      arm_height: 0.40, arm_height_s: 0.10,
      arm_length: 5.0, arm_length_s: 1.2,
      arm_radius: 0.35, arm_radius_s: 0.06,
      arm_curve: 0.75, arm_curve_s: 0.12,
      color_over: 0.25, color_var: 0.08,
      trunk_segs: 16, arm_segs: 10,
      weight: 0.20,
      body_color: [0.20, 0.38, 0.16],
      rib_color: [0.28, 0.48, 0.22] },
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

/* ═══ RIBBED COLUMN GEOMETRY ═══
   Generates quads for a ribbed columnar cactus segment.
   Returns faces: [{v:[rotated verts], col, li, z}] ready for sorting.
   centerX/Y/Z = base position, dir = [dx,dy,dz] growth direction,
   h = height along dir, r = base radius, taper = top/bottom ratio,
   ribs/ribDepth = rib modulation, segsV = vertical subdivisions. */
function ribbedColumn(T, cx, cy, cz, dirX, dirY, dirZ, h, r, taper, ribs, ribDepth, segsV, segsA, bodyCol, ribCol, lean) {
  const quads = [];
  const steps = Math.max(4, segsV);
  const around = Math.max(6, segsA || Math.max(ribs * 2, 12));
  const dl = normalize3([dirX, dirY, dirZ]);

  // Perpendicular axes for the column cross-section
  let up = [0, 1, 0];
  if (Math.abs(dl[1]) > 0.95) up = [1, 0, 0];
  const right = normalize3(cross3(dl, up));
  const fwd = normalize3(cross3(right, dl));

  for (let j = 0; j < steps; j++) {
    const t0 = j / steps, t1 = (j + 1) / steps;
    for (let i = 0; i < around; i++) {
      const a0 = (i / around) * Math.PI * 2;
      const a1 = ((i + 1) / around) * Math.PI * 2;
      const pts = [];
      for (const [t, a] of [[t0, a0], [t1, a0], [t1, a1], [t0, a1]]) {
        const d = t * h;
        const rad = r * lerp(1.0, taper, t);
        // Rib modulation
        const ribPhase = a * ribs / (Math.PI * 2);
        const ribMod = 1.0 + Math.cos(ribPhase * Math.PI * 2) * ribDepth;
        const finalR = rad * ribMod;
        // Cap rounding at top
        const capT = Math.max(0, (t - (1.0 - T.cap_round * 0.3)) / (T.cap_round * 0.3));
        const capScale = capT > 0 ? Math.cos(capT * Math.PI * 0.5) : 1.0;
        const rr = finalR * capScale;
        // Lean offset
        const leanOff = (lean || 0) * h * t * t;
        const ca = Math.cos(a), sa = Math.sin(a);
        const x = cx + dl[0] * d + (right[0] * ca + fwd[0] * sa) * rr + right[0] * leanOff;
        const y = cy + dl[1] * d + (right[1] * ca + fwd[1] * sa) * rr;
        const z = cz + dl[2] * d + (right[2] * ca + fwd[2] * sa) * rr + right[2] * leanOff * 0.3;
        pts.push([x, y, z]);
      }
      // Color: ribs lighter on crests
      const ribFrac = (Math.cos((((a0 + a1) / 2) * ribs / (Math.PI * 2)) * Math.PI * 2) + 1) * 0.5;
      const shade = 0.85 + 0.15 * ((t0 + t1) / 2);
      const col = lerpC(bodyCol, ribCol, ribFrac * 0.6).map(c => c * shade);
      quads.push({ verts: pts, color: col });
    }
  }
  return quads;
}

/* ═══ 3D RENDERER ═══ */
/* Rotated WORLD space only — never projects, draws, clears, or sees W/H/zoom/pan.
   Scale-free by design: that is what lets tiers merge into one sorted list. */
function buildFaces(T, rotY, tilt, xOff, midY, seed) {
  const ld = normalize3([-0.6, -0.7, -0.3]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt), sinT = Math.sin(tilt);
  const armCount = Math.max(0, Math.round(T.arm_count));

  const rv = (v) => {
    const vx = v[0] + xOff;          // world-space row offset, BEFORE the Y-rotation
    const vy = v[1] - midY;
    const x1 = vx * cosR + v[2] * sinR;
    const z1 = -vx * sinR + v[2] * cosR;
    return [x1, vy * cosT - z1 * sinT, vy * sinT + z1 * cosT];
  };

  const faces = [];
  const addQuads = (quads) => {
    for (const q of quads) {
      const rv0 = rv(q.verts[0]), rv1 = rv(q.verts[1]), rv2 = rv(q.verts[2]), rv3 = rv(q.verts[3]);
      const n = normalize3(cross3(sub3(rv1, rv0), sub3(rv2, rv0)));
      const dt = -(n[0] * ld[0] + n[1] * ld[1] + n[2] * ld[2]);
      const li = Math.max(0.2, Math.min(1, 0.3 + dt * 0.7));
      faces.push({ v: [rv0, rv1, rv2, rv3], col: q.color, li, z: (rv0[2] + rv1[2] + rv2[2] + rv3[2]) / 4 });
    }
  };

  const ribs = Math.max(4, Math.round(T.ribs));
  const around = Math.max(ribs * 2, 12);

  // Main trunk
  const trunkQuads = ribbedColumn(T, 0, 0, 0, 0, 1, 0, T.height, T.radius, T.taper, ribs, T.rib_depth, T.trunk_segs, around, T.body_color, T.rib_color, T.lean);
  addQuads(trunkQuads);

  // Arms
  const GA = Math.PI * (3 - Math.sqrt(5));
  for (let a = 0; a < armCount; a++) {
    const armAz = a * GA + hf(seed, 1050 + a) * 0.5;
    const forkY = T.height * (T.arm_height + (hf(seed, 1060 + a) - 0.5) * 0.15);
    const leanOff = T.lean * T.height * (forkY / T.height) * (forkY / T.height);
    const forkX = Math.cos(armAz) * T.radius * T.taper * 0.9 + leanOff * 0.3;
    const forkZ = Math.sin(armAz) * T.radius * T.taper * 0.9;

    // Arm grows outward then curves upward
    // We render it as a column along a direction that starts horizontal and bends up
    const armLen = T.arm_length * (0.8 + hf(seed, 1070 + a) * 0.4);
    const armSegs = Math.max(4, T.arm_segs);
    const armR = T.arm_radius * (0.85 + hf(seed, 1080 + a) * 0.3);
    const outX = Math.cos(armAz), outZ = Math.sin(armAz);

    // Build arm as series of short column segments along a curve
    const armQuads = [];
    const aSteps = armSegs;
    let px = forkX, py = forkY, pz = forkZ;
    for (let s = 0; s < aSteps; s++) {
      const t0 = s / aSteps, t1 = (s + 1) / aSteps;
      const tMid = (t0 + t1) / 2;
      // Direction blends from horizontal-outward to vertical-up
      const blend = tMid * T.arm_curve;
      const dx = outX * (1.0 - blend);
      const dy = blend;
      const dz = outZ * (1.0 - blend);
      const dl = Math.sqrt(dx * dx + dy * dy + dz * dz);
      const segLen = armLen / aSteps;
      const nx = px + (dx / dl) * segLen;
      const ny = py + (dy / dl) * segLen;
      const nz = pz + (dz / dl) * segLen;

      const segR = armR * lerp(1.0, 0.7, tMid);
      const segRibs = Math.max(4, ribs - 2);
      const segAround = Math.max(segRibs * 2, 8);
      const sq = ribbedColumn(T, px, py, pz, dx / dl, dy / dl, dz / dl, segLen, segR, 0.95, segRibs, T.rib_depth * 0.8, 1, segAround, T.body_color, T.rib_color, 0);
      armQuads.push(...sq);
      px = nx; py = ny; pz = nz;
    }
    // Arm cap (small hemisphere)
    const capR = armR * 0.7;
    const capSegs = 4;
    for (let ci = 0; ci < capSegs; ci++) {
      const ca0 = (ci / capSegs) * Math.PI * 2;
      const ca1 = ((ci + 1) / capSegs) * Math.PI * 2;
      armQuads.push({ verts: [
        [px, py, pz],
        [px + Math.cos(ca0) * capR, py + capR * 0.4, pz + Math.sin(ca0) * capR],
        [px + Math.cos(ca1) * capR, py + capR * 0.4, pz + Math.sin(ca1) * capR],
        [px, py + capR * 0.6, pz],
      ], color: lerpC(T.body_color, T.rib_color, 0.3) });
    }
    addQuads(armQuads);
  }

  return faces;
}

/* ═══ CAMERA PRIMITIVES — world→screen, and the depth-sorted fill ═══ */
function makeProject(W, H, scale, panX, panY) {
  return (v) => [W / 2 + panX + v[0] * scale, H / 2 + panY - v[1] * scale];
}

function drawFaces(ctx, faces, project) {
  faces.sort((a, b) => a.z - b.z);
  for (const f of faces) {
    const pts = f.v.map(project);
    ctx.fillStyle = rgb01(f.col[0] * f.li, f.col[1] * f.li, f.col[2] * f.li);
    ctx.beginPath(); ctx.moveTo(pts[0][0], pts[0][1]);
    for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i][0], pts[i][1]);
    ctx.closePath(); ctx.fill();
  }
}

function tierExtent(T) { return Math.max(T.height * 1.2, T.arm_length * 2 + T.radius * 2); }
function tierHeight(T) { return T.height; }

/* Single view — keeps render3D's original public signature, so the call sites
   and effect deps from [2] are untouched. */
function render3D(ctx, W, H, T, rotY, tilt, seed, zoom = 1, panX = 0, panY = 0) {
  ctx.clearRect(0, 0, W, H);
  const scale = Math.min(W, H) * 0.36 / (tierExtent(T) * 0.5) * zoom;
  const midY = T.height * 0.45;
  drawFaces(ctx, buildFaces(T, rotY, tilt, 0, midY, seed), makeProject(W, H, scale, panX, panY));
}

/* All tiers in ONE shared scene: bases on a common line, spaced along world x,
   the row rotating as a unit. Every tier's faces merge into a single
   depth-sorted list so they occlude correctly at any angle. Scale is shared and
   taken from the largest tier — the RELATIVE SIZE IS THE COMPARISON, so a tier
   is never normalised to its own slot. */
function render3DCompare(ctx, W, H, tiers, rotY, tilt, seed, zoom = 1, panX = 0, panY = 0) {
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
  tiers.forEach((T, i) => all.push(...buildFaces(T, rotY, tilt, xOffs[i], midY, seed)));
  drawFaces(ctx, all, makeProject(W, H, scale, panX, panY));
}

/* ═══ 2D PROFILE ═══ */
function render2D(ctx, W, H, T, seed) {
  ctx.clearRect(0, 0, W, H);
  ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const mg = { l: 16, r: 16, t: 10, b: 10 }, pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  const maxExt = Math.max(T.height, T.arm_length * 2 + T.radius * 4);
  const sc = Math.min(ph / (T.height * 1.3), pw / 2 / (maxExt * 0.6));
  const cx = W / 2, baseY = mg.t + ph;
  const toS = (x, y) => [cx + x * sc, baseY - y * sc];

  // Trunk silhouette
  const steps = 20;
  ctx.fillStyle = "rgba(34,66,28,0.15)";
  ctx.beginPath();
  for (let i = 0; i <= steps; i++) {
    const t = i / steps;
    const r = T.radius * lerp(1.0, T.taper, t);
    const capT = Math.max(0, (t - (1.0 - T.cap_round * 0.3)) / (T.cap_round * 0.3));
    const capScale = capT > 0 ? Math.cos(capT * Math.PI * 0.5) : 1.0;
    const leanOff = T.lean * T.height * t * t;
    const [sx, sy] = toS(leanOff + r * capScale, t * T.height);
    if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
  }
  for (let i = steps; i >= 0; i--) {
    const t = i / steps;
    const r = T.radius * lerp(1.0, T.taper, t);
    const capT = Math.max(0, (t - (1.0 - T.cap_round * 0.3)) / (T.cap_round * 0.3));
    const capScale = capT > 0 ? Math.cos(capT * Math.PI * 0.5) : 1.0;
    const leanOff = T.lean * T.height * t * t;
    const [sx, sy] = toS(leanOff - r * capScale, t * T.height);
    ctx.lineTo(sx, sy);
  }
  ctx.closePath(); ctx.fill();

  // Trunk outline
  ctx.strokeStyle = rgb01(...T.body_color); ctx.lineWidth = 1.5;
  for (const sign of [1, -1]) {
    ctx.beginPath();
    for (let i = 0; i <= steps; i++) {
      const t = i / steps;
      const r = T.radius * lerp(1.0, T.taper, t);
      const capT = Math.max(0, (t - (1.0 - T.cap_round * 0.3)) / (T.cap_round * 0.3));
      const capScale = capT > 0 ? Math.cos(capT * Math.PI * 0.5) : 1.0;
      const leanOff = T.lean * T.height * t * t;
      const [sx, sy] = toS(leanOff + r * capScale * sign, t * T.height);
      if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
    }
    ctx.stroke();
  }

  // Arms (2D projection)
  const armCount = Math.max(0, Math.round(T.arm_count));
  const GA = Math.PI * (3 - Math.sqrt(5));
  ctx.lineWidth = 1;
  for (let a = 0; a < armCount; a++) {
    const armAz = a * GA + hf(seed, 1050 + a) * 0.5;
    const cosAz = Math.cos(armAz);
    const forkFrac = T.arm_height + (hf(seed, 1060 + a) - 0.5) * 0.15;
    const forkY = T.height * forkFrac;
    const leanOff = T.lean * T.height * forkFrac * forkFrac;
    const armLen = T.arm_length * (0.8 + hf(seed, 1070 + a) * 0.4);
    const armR = T.arm_radius * (0.85 + hf(seed, 1080 + a) * 0.3);
    const alpha = 0.3 + 0.4 * Math.abs(cosAz);
    ctx.strokeStyle = `rgba(${Math.round(T.body_color[0] * 255)},${Math.round(T.body_color[1] * 255)},${Math.round(T.body_color[2] * 255)},${alpha.toFixed(2)})`;
    const aSegs = 12;
    for (const sign of [1, -1]) {
      ctx.beginPath();
      let px = leanOff + T.radius * T.taper * 0.9 * cosAz * sign, py = forkY;
      for (let s = 0; s <= aSegs; s++) {
        const t = s / aSegs;
        const blend = t * T.arm_curve;
        const dx = cosAz * (1.0 - blend) * sign;
        const dy = blend;
        const dl = Math.sqrt(dx * dx + dy * dy);
        px += (dx / dl) * (armLen / aSegs);
        py += (dy / dl) * (armLen / aSegs);
        const r = armR * lerp(1.0, 0.7, t) * ((sign === 1) ? 1 : -1) * 0.3;
        const [sx, sy] = toS(px + r, py);
        if (s === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
      }
      ctx.stroke();
    }
  }

  // Ground
  const [gx0, gy] = toS(-maxExt * 0.6, 0);
  const [gx1] = toS(maxExt * 0.6, 0);
  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(gx0, gy); ctx.lineTo(gx1, gy); ctx.stroke(); ctx.setLineDash([]);
  ctx.fillStyle = "rgba(255,255,255,0.3)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  ctx.fillText(`h=${T.height.toFixed(1)} r=${T.radius.toFixed(2)}`, cx, mg.t + 8);
}

/* ═══ C++ EXPORT ═══ */
function genCpp(tiers) {
  const f = (v, d = 2) => { const s = v.toFixed(d); return s.includes(".") ? s + "f" : s + ".0f"; };
  const lines = [];
  lines.push("// ─── Cactus Tier Definitions ──────────────────────────────────────────");
  lines.push("static constexpr CactusTierParams CACTUS_TIERS[] = {");
  tiers.forEach((T, i) => {
    const vals = [
      T.height, T.height_s, T.radius, T.radius_s, T.taper, T.taper_s,
      T.ribs, T.ribs_s, T.rib_depth, T.rib_depth_s,
      T.lean, T.lean_s, T.cap_round, T.cap_round_s,
      T.arm_count, T.arm_count_s, T.arm_height, T.arm_height_s,
      T.arm_length, T.arm_length_s, T.arm_radius, T.arm_radius_s,
      T.arm_curve, T.arm_curve_s,
      T.color_over, T.color_var,
    ].map(v => f(v));
    vals.push(String(T.trunk_segs), String(T.arm_segs), f(T.weight));
    lines.push(`    /* ${TIER_NAMES[i].padEnd(12)} */  { ${vals.join(", ")} },`);
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

const STORAGE_KEY = "7t:cactus:tiers";
const DEFAULTS = () => [0, 1, 2].map(defaultTier);
const DEFAULTS_JSON = JSON.stringify(DEFAULTS());

/* ═══ MAIN COMPONENT ═══ */
export default function CactusDesigner() {
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
  const RT = { ...T, body_color: resolved.bodyCol, rib_color: resolved.ribCol };
  // Compare renders the whole row, so resolve each tier's colours the same way RT does.
  const cmpTiers = tiers.map(t => { const rc = resolveColors(t, seed); return { ...t, body_color: rc.bodyCol, rib_color: rc.ribCol }; });

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
  const defaultOrder = ["trunk", "arms", "appearance", "quality", "export"];
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
        c3.width = rect.width * devicePixelRatio; c3.height = rect.height * devicePixelRatio;
        const ctx = c3.getContext("2d");
        ctx.setTransform(devicePixelRatio, 0, 0, devicePixelRatio, 0, 0);
        const r3 = autoRot ? rotY + frame * 0.008 : rotY;
        if (compareMode) render3DCompare(ctx, rect.width, rect.height, cmpTiers, r3, tilt, seed, zoom, panX, panY);
        else render3D(ctx, rect.width, rect.height, RT, r3, tilt, seed, zoom, panX, panY);
      }
      if (c2) {
        const rect = c2.parentElement.getBoundingClientRect();
        c2.width = rect.width * devicePixelRatio; c2.height = rect.height * devicePixelRatio;
        const ctx = c2.getContext("2d");
        ctx.setTransform(devicePixelRatio, 0, 0, devicePixelRatio, 0, 0);
        render2D(ctx, rect.width, rect.height, RT, seed);
      }
      frame++; animRef.current = requestAnimationFrame(tick);
    };
    tick(); return () => cancelAnimationFrame(animRef.current);
  }, [RT, cmpTiers, compareMode, rotY, tilt, autoRot, seed, zoom, panX, panY]);

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
      <div style={{ flex: "1 1 55%", display: "flex", flexDirection: "column", gap: 6, minWidth: 0 }}>
        <div style={{ display: "flex", gap: 4, alignItems: "center", flexWrap: "wrap" }}>
          {TIER_NAMES.map((name, i) => (<button key={i} onClick={() => { setTierIdx(i); setCompareMode(false); }} style={{ ...btnStyle, background: i === tierIdx ? "var(--color-background-info)" : "var(--color-background-secondary)", color: i === tierIdx ? "var(--color-text-on-info)" : "var(--color-text-secondary)", fontWeight: i === tierIdx ? 600 : 400 }}>{name}</button>))}
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
        <div style={{ display: "flex", gap: 6, alignItems: "center", fontSize: 10 }}>
          <span style={{ color: "var(--color-text-secondary)" }}>seed</span>
          <Num value={seed} min={0} max={99999} step={1} w={52} onChange={v => setSeed(Math.round(v))} />
          <button onClick={() => setSeed(Math.floor(Math.random() * 99999))} style={btnStyle}>reroll</button>
          <span style={{ color: "var(--color-text-tertiary)", marginLeft: 4 }}>body</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.bodyCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
          <span style={{ color: "var(--color-text-tertiary)" }}>rib</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.ribCol), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
        </div>
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
        <div style={{ flex: "0 0 28%", background: "#111114", borderRadius: 8, overflow: "hidden", minHeight: 100 }}>
          <canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} />
        </div>
      </div>
      <div style={{ flex: "0 0 310px", overflowY: "auto", overflowX: "hidden", paddingRight: 2 }}>
        {panelOrder.map(pid => {
          const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => { const w = prev.filter(p => p !== id); const oi = defaultOrder.indexOf(id); const at = w.findIndex(p => defaultOrder.indexOf(p) > oi); return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)]; }) };
          switch (pid) {
            case "trunk": return (<DragPanel {...dp} title="Trunk" ini={true}>
              <MuSigma label="Height" mu={T.height} muSet={v => upd(n => { n.height = v; })} sigma={T.height_s} sigmaSet={v => upd(n => { n.height_s = v; })} muMin={1} muMax={40} sMax={10} />
              <MuSigma label="Radius" mu={T.radius} muSet={v => upd(n => { n.radius = v; })} sigma={T.radius_s} sigmaSet={v => upd(n => { n.radius_s = v; })} muMin={0.1} muMax={3} sMax={1} />
              <MuSigma label="Taper" mu={T.taper} muSet={v => upd(n => { n.taper = v; })} sigma={T.taper_s} sigmaSet={v => upd(n => { n.taper_s = v; })} muMin={0.3} muMax={1} sMax={0.2} />
              <MuSigma label="Ribs" mu={T.ribs} muSet={v => upd(n => { n.ribs = Math.round(v); })} sigma={T.ribs_s} sigmaSet={v => upd(n => { n.ribs_s = v; })} muMin={4} muMax={24} sMax={4} muW={34} />
              <MuSigma label="Rib depth" mu={T.rib_depth} muSet={v => upd(n => { n.rib_depth = v; })} sigma={T.rib_depth_s} sigmaSet={v => upd(n => { n.rib_depth_s = v; })} muMax={0.5} sMax={0.15} />
              <MuSigma label="Lean" mu={T.lean} muSet={v => upd(n => { n.lean = v; })} sigma={T.lean_s} sigmaSet={v => upd(n => { n.lean_s = v; })} muMax={0.3} sMax={0.1} />
              <MuSigma label="Cap round" mu={T.cap_round} muSet={v => upd(n => { n.cap_round = v; })} sigma={T.cap_round_s} sigmaSet={v => upd(n => { n.cap_round_s = v; })} muMax={1} sMax={0.3} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Ribs = vertical ridges around the column. Rib depth = how deep the valleys between ridges. Cap round = hemisphere rounding at the top. Lean = quadratic tilt.</div>
            </DragPanel>);
            case "arms": return (<DragPanel {...dp} title="Arms" ini={true}>
              <MuSigma label="Arm count" mu={T.arm_count} muSet={v => upd(n => { n.arm_count = Math.round(v); })} sigma={T.arm_count_s} sigmaSet={v => upd(n => { n.arm_count_s = v; })} muMax={6} sMax={2} muW={34} />
              <MuSigma label="Fork height" mu={T.arm_height} muSet={v => upd(n => { n.arm_height = v; })} sigma={T.arm_height_s} sigmaSet={v => upd(n => { n.arm_height_s = v; })} muMin={0.15} muMax={0.8} sMax={0.2} />
              <MuSigma label="Arm length" mu={T.arm_length} muSet={v => upd(n => { n.arm_length = v; })} sigma={T.arm_length_s} sigmaSet={v => upd(n => { n.arm_length_s = v; })} muMin={0.5} muMax={12} sMax={3} />
              <MuSigma label="Arm radius" mu={T.arm_radius} muSet={v => upd(n => { n.arm_radius = v; })} sigma={T.arm_radius_s} sigmaSet={v => upd(n => { n.arm_radius_s = v; })} muMin={0.05} muMax={1.5} sMax={0.4} />
              <MuSigma label="Arm curve" mu={T.arm_curve} muSet={v => upd(n => { n.arm_curve = v; })} sigma={T.arm_curve_s} sigmaSet={v => upd(n => { n.arm_curve_s = v; })} muMax={1} sMax={0.3} />
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>Fork height = fraction of trunk height where arms emerge. Arm curve = how quickly the arm bends from horizontal to vertical (0=straight out, 1=sharp elbow up). Set arm count to 0 for a simple finger cactus.</div>
            </DragPanel>);
            case "appearance": return (<DragPanel {...dp} title="Appearance">
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 3 }}>Body (valleys)</div>
              <PaletteSwatches palette={BODY_PALETTE} label="Body palette" onPick={c => upd(n => { n.body_color = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"b" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.body_color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.body_color[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.body_color), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>Ribs (crests)</div>
              <PaletteSwatches palette={RIB_PALETTE} label="Rib palette" onPick={c => upd(n => { n.rib_color = c; })} />
              <div style={rw}>
                <span style={lb}>Color</span>
                {["R", "G", "B"].map((ch, i) => <span key={"r" + i} style={{ display: "contents" }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span><Num value={T.rib_color[i]} min={0} max={1} w={34} onChange={v => upd(n => { n.rib_color[i] = v; })} /></span>)}
                <span style={{ width: 18, height: 18, borderRadius: 4, display: "inline-block", background: rgb01(...T.rib_color), border: "1px solid var(--color-border-tertiary)" }} />
              </div>
              <div style={rw}>
                <span style={lb}>Override %</span><Num value={T.color_over} min={0} max={1} w={38} onChange={v => upd(n => { n.color_over = v; })} />
                <span style={lb}>Variance</span><Num value={T.color_var} min={0} max={0.3} w={38} onChange={v => upd(n => { n.color_var = v; })} />
              </div>
            </DragPanel>);
            case "quality": return (<DragPanel {...dp} title="Quality & Selection">
              <div style={rw}>
                <span style={lb}>Trunk segs</span><Num value={T.trunk_segs} min={6} max={24} step={1} w={34} onChange={v => upd(n => { n.trunk_segs = Math.round(v); })} />
                <span style={lb}>Arm segs</span><Num value={T.arm_segs} min={4} max={16} step={1} w={34} onChange={v => upd(n => { n.arm_segs = Math.round(v); })} />
                <span style={lb}>Weight</span><Num value={T.weight} min={0} max={1} w={38} onChange={v => upd(n => { n.weight = v; })} />
              </div>
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
