import { useState, useEffect, useRef, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T MAU5 DESIGNER — mouth-focused single-preset version
   
   Geometry: one connected mesh. The head is a UV sphere tessellated in
   (lat, lon). The mouth region is defined IN the sphere's own angular
   coordinates — so the cut boundary lies exactly along the tessellation
   grid. Quads whose centre falls inside the mouth region get emitted on
   an inner sphere (radius = head_r − mouth_depth); the rest sit on the
   outer sphere (head_r). Wherever two neighbouring quads disagree about
   inside/outside, we emit a rim wall quad connecting the outer vertex to
   the inner vertex at that shared edge — vertices are shared, so the mesh
   is a single connected piece. No quad-culling, no painter's-algo zebra,
   no jagged rim. The inner surface curves with the sphere the way an
   orange wedge cross-section does.
   
   Tiers have been dropped for this pass. Get one geometry right first.
   ═══════════════════════════════════════════════════════════════════════ */

/* ═══ HASH — same pipeline as other 7T designers ═══ */
function cpuH(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return ((h >>> 16) ^ h) >>> 0;
}
function hf(s, p) { return cpuH(s, p) / 0xFFFFFFFF; }

/* ═══ PALETTES — retained for per-instance colour variation at runtime ═══ */
const HEAD_PALETTE = [
  { name: "Classic black",  c: [0.05, 0.05, 0.06] },
  { name: "Charcoal",       c: [0.18, 0.18, 0.20] },
  { name: "Stage red",      c: [0.72, 0.10, 0.12] },
  { name: "Bubblegum",      c: [0.85, 0.45, 0.65] },
  { name: "Electric cyan",  c: [0.20, 0.80, 0.95] },
  { name: "Acid green",     c: [0.45, 0.85, 0.25] },
  { name: "Royal purple",   c: [0.35, 0.20, 0.70] },
  { name: "Pearl white",    c: [0.92, 0.92, 0.95] },
];
const GLOW_PALETTE = [
  { name: "Cyan rim",       c: [0.20, 0.80, 1.00] },
  { name: "Magenta rim",    c: [1.00, 0.25, 0.90] },
  { name: "Gold rim",       c: [1.00, 0.75, 0.20] },
  { name: "Green rim",      c: [0.35, 0.95, 0.45] },
  { name: "Off",            c: [0.05, 0.05, 0.07] },
];
const EYE_PALETTE = [
  { name: "Light yellow",     c: [0.95, 0.92, 0.60] },
  { name: "Black",            c: [0.05, 0.05, 0.06] },
  { name: "White",            c: [0.95, 0.95, 0.95] },
  { name: "Very light purple",c: [0.82, 0.72, 0.92] },
  { name: "Charcoal",         c: [0.15, 0.15, 0.17] },
  { name: "Cyan",             c: [0.20, 0.80, 0.95] },
];
const MOUTH_PALETTE = [
  { name: "Void black",  c: [0.02, 0.02, 0.025] },
  { name: "Dark grey",   c: [0.12, 0.12, 0.14] },
  { name: "Deep red",    c: [0.35, 0.05, 0.05] },
  { name: "Blood",       c: [0.55, 0.08, 0.08] },
  { name: "Electric",    c: [0.10, 0.70, 0.90] },
  { name: "Magenta",     c: [0.80, 0.15, 0.65] },
];

/* ═══ DEFAULTS — a single preset (no tiers) ═══
   Mouth is described entirely in spherical coordinates so the cut aligns
   with the head's UV tessellation:
     mouth_lon_half   azimuthal half-width of the opening, in radians.
                      The mouth is centred at lon = π/2 (front of the head).
     mouth_lat_center latitude of the band centre at t=0 (negative = lower
                      face). In radians.
     mouth_lat_upper  how far the upper lip extends above band centre.
     mouth_lat_lower  how far the lower lip extends below band centre.
     mouth_smile      latitude rise at the corners (t=±1). The band centre
                      follows  lat_c(t) = mouth_lat_center + mouth_smile·t²
                      so both edges rise in parallel — a pure smile.
     mouth_depth      radial depth in world units. inner_r = head_r − depth.
   
   Tessellation defaults (head_lat=48, head_lon=72) are higher than the
   previous tier values because the rim's apparent smoothness depends on
   grid density: each lat step ≈ 3.75°, each lon step = 5°. */
const DEFAULTS = {
  head_r: 1.20,
  head_squash: 1.00,
  head_lat: 48,
  head_lon: 72,

  ear_r: 1.00,
  ear_thick: 0.60,
  ear_offset_x: 1.46,
  ear_lift_y: 1.05,
  ear_offset_z: 0.00,
  ear_tilt: 0.00,
  ear_yaw: 0.00,
  ear_segs: 24,

  eye_r: 0.19,
  eye_offset_x: 0.58,
  eye_offset_y: 0.70,
  eye_protrude: 0.00,
  eye_segs: 12,

  // Mouth defaults: BIG — occupies most of the lower-front ("south
  // hemisphere" when viewed frontally). All user-tunable in the panel.
  // With marching squares the rim is smooth at any smile/lat_half ratio.
  // Mouth. mouth_lon_half is the HORIZONTAL WIDTH — the azimuthal half-span
  // from the front-centre (lon=π/2) to each side. 1.20 rad ≈ ±69° → the
  // opening spans 138° across the face. Increase it to broaden the smile.
  mouth_lon_half:   1.20,   // ≈ 69° half-width → 138° total horizontal span
  mouth_lat_center: -0.55,  // ≈ -31°, vertical centre on the lower face
  mouth_lat_upper:  0.30,   // ≈ 17°, how far the upper lip extends above centre
  mouth_lat_lower:  0.50,   // ≈ 29°, how far the lower lip extends below centre
  mouth_smile:      0.18,   // ≈ 10° corner-rise (band centre curves upward)
  mouth_depth:      0.15,   // inner sphere at 1.05 when head_r=1.20

  color_over: 0.30,
  head_var: 0.05, face_var: 0.02, ear_var: 0.05, glow_var: 0.08, eye_var: 0.03,
  head_color:  [0.05, 0.05, 0.06],
  face_color:  [0.95, 0.95, 0.92],
  ear_color:   [0.05, 0.05, 0.06],
  eye_color:   [0.12, 0.12, 0.14],
  glow_color:  [0.25, 0.80, 0.95],
  mouth_color: [0.03, 0.03, 0.035],
};

/* ═══ PER-INSTANCE COLOR — hash-driven palette selection ═══
   Prop IDs 960-995 match cartridge.hpp's property band registry. */
function resolveColors(T, seed) {
  const asRGB = (c, fb) => Array.isArray(c) && c.length >= 3
    ? [c[0] ?? fb[0], c[1] ?? fb[1], c[2] ?? fb[2]]
    : [...fb];
  let hc = asRGB(T.head_color,  [0.05, 0.05, 0.06]);
  let fc = asRGB(T.face_color,  [0.95, 0.95, 0.92]);
  let ec = asRGB(T.ear_color,   [0.05, 0.05, 0.06]);
  let yc = asRGB(T.eye_color,   [0.12, 0.12, 0.14]);
  let gc = asRGB(T.glow_color,  [0.25, 0.80, 0.95]);
  const mc = asRGB(T.mouth_color,[0.03, 0.03, 0.035]);

  if (hf(seed, 960) < T.color_over) { const pi = cpuH(seed, 965) % HEAD_PALETTE.length; hc = [...HEAD_PALETTE[pi].c]; }
  hc[0] += (hf(seed, 961) - 0.5) * T.head_var * 2;
  hc[1] += (hf(seed, 962) - 0.5) * T.head_var * 2;
  hc[2] += (hf(seed, 963) - 0.5) * T.head_var * 2;

  fc[0] += (hf(seed, 971) - 0.5) * T.face_var * 2;
  fc[1] += (hf(seed, 972) - 0.5) * T.face_var * 2;
  fc[2] += (hf(seed, 973) - 0.5) * T.face_var * 2;

  ec[0] += (hf(seed, 981) - 0.5) * T.ear_var * 2;
  ec[1] += (hf(seed, 982) - 0.5) * T.ear_var * 2;
  ec[2] += (hf(seed, 983) - 0.5) * T.ear_var * 2;

  const ev = T.eye_var || 0.03;
  yc[0] += (hf(seed, 985) - 0.5) * ev * 2;
  yc[1] += (hf(seed, 986) - 0.5) * ev * 2;
  yc[2] += (hf(seed, 987) - 0.5) * ev * 2;

  if (hf(seed, 990) < T.color_over) { const pi = cpuH(seed, 995) % GLOW_PALETTE.length; gc = [...GLOW_PALETTE[pi].c]; }
  gc[0] += (hf(seed, 991) - 0.5) * T.glow_var * 2;
  gc[1] += (hf(seed, 992) - 0.5) * T.glow_var * 2;
  gc[2] += (hf(seed, 993) - 0.5) * T.glow_var * 2;

  const clip = c => c.map(v => Math.max(0, Math.min(1, v)));
  return { headCol: clip(hc), faceCol: clip(fc), earCol: clip(ec), eyeCol: clip(yc), glowCol: clip(gc), mouthCol: clip(mc) };
}

/* ═══ 3D MATH ═══ */
function cross3(a, b) { return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]; }
function sub3(a, b)   { return [a[0]-b[0], a[1]-b[1], a[2]-b[2]]; }
function normalize3(v){ const l = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); return l<1e-8?[0,0,1]:[v[0]/l,v[1]/l,v[2]/l]; }
function rgb01(r,g,b) { return `rgb(${Math.round(Math.max(0,Math.min(1,r))*255)},${Math.round(Math.max(0,Math.min(1,g))*255)},${Math.round(Math.max(0,Math.min(1,b))*255)})`; }

/* Signed distance from (lat, lon) to the mouth-region boundary.
   Negative inside, positive outside, zero on boundary.
   
   Band CENTRE:  lat_c(t) = mouth_lat_center + mouth_smile·t²
   Upper edge:   lat_c(t) + mouth_lat_upper
   Lower edge:   lat_c(t) - mouth_lat_lower
   
   These are independent, so upper and lower lip can be shaped separately. */
function mouthDist(lat, lon, T) {
  let dlon = lon - Math.PI / 2;
  while (dlon > Math.PI)  dlon -= 2*Math.PI;
  while (dlon < -Math.PI) dlon += 2*Math.PI;
  const t  = Math.max(-1, Math.min(1, dlon / T.mouth_lon_half));
  const bandC = T.mouth_lat_center + T.mouth_smile * t * t;
  const ox = Math.abs(dlon) - T.mouth_lon_half;
  // Asymmetric vertical: different distance above vs below centre
  const above = lat - bandC;
  const oy = above >= 0
    ? above - T.mouth_lat_upper   // positive when outside above
    : -above - T.mouth_lat_lower; // positive when outside below
  return Math.max(ox, oy);
}

/* ═══ 3D RENDERER ═══
   Head + mouth built as a single connected mesh (see file header). */
function render3D(ctx, W, H, T, rotY, tilt, roll = 0, zoom = 1.0) {
  ctx.clearRect(0, 0, W, H);
  const lightDir = normalize3([-0.5, -0.6, -0.6]);
  const cosR = Math.cos(rotY), sinR = Math.sin(rotY);
  const cosT = Math.cos(tilt),  sinT = Math.sin(tilt);
  const cosL = Math.cos(roll),  sinL = Math.sin(roll);

  const headY   = T.head_r * T.head_squash;
  const topY    = headY + T.ear_lift_y + T.ear_r;
  const widest  = Math.max(T.ear_offset_x + T.ear_r, T.head_r * 1.05);
  const extent  = Math.max(topY, widest * 2);
  const scale   = Math.min(W, H) * 0.40 / (extent * 0.55) * zoom;
  const midY    = headY; // rotation pivots on head centre

  const rotV = (v) => {
    const vy = v[1] - midY;
    // 1) Yaw around world Y (horizontal drag)
    const x1 =  v[0]*cosR + v[2]*sinR;
    const z1 = -v[0]*sinR + v[2]*cosR;
    // 2) Pitch around world X (vertical drag)
    const y2 = vy*cosT - z1*sinT;
    const z2 = vy*sinT + z1*cosT;
    // 3) Roll around the camera's forward (Z) axis — rotates the XY plane.
    //    This gives a true third rotational degree of freedom on top of the
    //    yaw+pitch that already covers every possible viewing direction.
    const x3 = x1*cosL - y2*sinL;
    const y3 = x1*sinL + y2*cosL;
    return [x3, y3, z2];
  };
  const project = (v) => [W/2 + v[0]*scale, H/2 - v[1]*scale];

  const faces = [];
  const addTri = (a, b, c, col, emissive = 0) => {
    const n = normalize3(cross3(sub3(b, a), sub3(c, a)));
    const d = -(n[0]*lightDir[0] + n[1]*lightDir[1] + n[2]*lightDir[2]);
    const diff = Math.max(0.28, Math.min(1.0, 0.38 + d*0.62));
    const light = emissive > 0 ? Math.max(diff, 0.85 + emissive*0.15) : diff;
    faces.push({ verts: [a,b,c], col, light, avgZ: (a[2]+b[2]+c[2])/3 });
  };
  const addQuad = (a,b,c,d,col,emi=0) => { addTri(a,b,c,col,emi); addTri(a,c,d,col,emi); };

  /* ── Unified head + mouth via marching squares ───────────────────────
     For each (lat, lon) cell:
       1. Evaluate mouthDist() at the 4 corners.
       2. Classify by the 4-bit inside/outside pattern.
       3. If all-in or all-out, emit a regular quad on innerR or outerR.
       4. Otherwise, interpolate the exact boundary crossings on the cell's
          edges using the signed distance, then emit:
            · an inside polygon (on innerR) in mouth colour
            · an outside polygon (on outerR) in head colour
            · one or two rim-wall quads connecting the outer/inner
              crossings at identical (lat, lon) — topologically connected.
     The rim follows the analytic smile curve, not the grid, so it's smooth
     at any tessellation. The 16 corner patterns reduce to 14 cases (two
     diagonal patterns — 0101, 1010 — are ambiguous and resolved by a
     cell-centre sample; those are rare for simply-connected mouth shapes). */
  const latN = Math.max(12, Math.round(T.head_lat));
  const lonN = Math.max(16, Math.round(T.head_lon));
  const sy = T.head_squash;
  const outerR = T.head_r;
  const innerR = Math.max(0.05, T.head_r - T.mouth_depth);

  const sphPt = (R, lat, lon) => [
    R * Math.cos(lat) * Math.cos(lon),
    headY + R * sy * Math.sin(lat),
    R * Math.cos(lat) * Math.sin(lon),
  ];
  const emitPoly = (pts, R, col) => {
    if (pts.length < 3) return;
    const v3d = pts.map(([la, lo]) => rotV(sphPt(R, la, lo)));
    for (let k = 1; k < v3d.length - 1; k++) addTri(v3d[0], v3d[k], v3d[k + 1], col);
  };
  const emitRim = (pA, pB) => {
    // Rim walls glow with glow_color — this is the visible "LED rim" in
    // the designer preview. In the engine, the glow shader does the full
    // bloom; here we just tint the rim and mark it emissive.
    const oA = rotV(sphPt(outerR, pA[0], pA[1]));
    const oB = rotV(sphPt(outerR, pB[0], pB[1]));
    const iA = rotV(sphPt(innerR, pA[0], pA[1]));
    const iB = rotV(sphPt(innerR, pB[0], pB[1]));
    addQuad(oA, iA, iB, oB, T.glow_color, 0.7);
  };

  for (let i = 0; i < latN; i++) {
    const lat0 = (i / latN - 0.5) * Math.PI;
    const lat1 = ((i + 1) / latN - 0.5) * Math.PI;
    for (let j = 0; j < lonN; j++) {
      const lon0 = (j / lonN) * 2 * Math.PI;
      const lon1 = ((j + 1) / lonN) * 2 * Math.PI;

      // Corners 0..3: bottom-left, bottom-right, top-right, top-left
      const cor = [[lat0, lon0], [lat0, lon1], [lat1, lon1], [lat1, lon0]];
      const d   = [mouthDist(lat0, lon0, T), mouthDist(lat0, lon1, T),
                   mouthDist(lat1, lon1, T), mouthDist(lat1, lon0, T)];
      const inM = [d[0] <= 0, d[1] <= 0, d[2] <= 0, d[3] <= 0];
      const pat = (inM[0]?1:0) | (inM[1]?2:0) | (inM[2]?4:0) | (inM[3]?8:0);

      if (pat === 0)  { emitPoly(cor, outerR, T.head_color); continue; }
      if (pat === 15) { emitPoly(cor, innerR, T.mouth_color); continue; }

      // Edge crossings (linear interpolation of signed distance).
      // Edge k is between corner k and corner (k+1)%4.
      const e = [null, null, null, null];
      for (let k = 0; k < 4; k++) {
        const a = k, b = (k + 1) % 4;
        if (inM[a] !== inM[b]) {
          const tt = d[a] / (d[a] - d[b]);
          e[k] = [cor[a][0] + tt * (cor[b][0] - cor[a][0]),
                  cor[a][1] + tt * (cor[b][1] - cor[a][1])];
        }
      }

      switch (pat) {
        case 1: // c0 in
          emitPoly([cor[0], e[0], e[3]], innerR, T.mouth_color);
          emitPoly([e[0], cor[1], cor[2], cor[3], e[3]], outerR, T.head_color);
          emitRim(e[0], e[3]);
          break;
        case 2: // c1 in
          emitPoly([cor[1], e[1], e[0]], innerR, T.mouth_color);
          emitPoly([cor[0], e[0], e[1], cor[2], cor[3]], outerR, T.head_color);
          emitRim(e[1], e[0]);
          break;
        case 3: // c0, c1 in (bottom)
          emitPoly([cor[0], cor[1], e[1], e[3]], innerR, T.mouth_color);
          emitPoly([e[3], e[1], cor[2], cor[3]], outerR, T.head_color);
          emitRim(e[1], e[3]);
          break;
        case 4: // c2 in
          emitPoly([cor[2], e[2], e[1]], innerR, T.mouth_color);
          emitPoly([cor[0], cor[1], e[1], e[2], cor[3]], outerR, T.head_color);
          emitRim(e[2], e[1]);
          break;
        case 6: // c1, c2 in (right)
          emitPoly([e[0], cor[1], cor[2], e[2]], innerR, T.mouth_color);
          emitPoly([cor[0], e[0], e[2], cor[3]], outerR, T.head_color);
          emitRim(e[2], e[0]);
          break;
        case 7: // all but c3
          emitPoly([cor[0], cor[1], cor[2], e[2], e[3]], innerR, T.mouth_color);
          emitPoly([e[3], e[2], cor[3]], outerR, T.head_color);
          emitRim(e[2], e[3]);
          break;
        case 8: // c3 in
          emitPoly([cor[3], e[3], e[2]], innerR, T.mouth_color);
          emitPoly([cor[0], cor[1], cor[2], e[2], e[3]], outerR, T.head_color);
          emitRim(e[3], e[2]);
          break;
        case 9: // c0, c3 in (left)
          emitPoly([cor[0], e[0], e[2], cor[3]], innerR, T.mouth_color);
          emitPoly([e[0], cor[1], cor[2], e[2]], outerR, T.head_color);
          emitRim(e[0], e[2]);
          break;
        case 11: // all but c2
          emitPoly([cor[0], cor[1], e[1], e[2], cor[3]], innerR, T.mouth_color);
          emitPoly([e[1], cor[2], e[2]], outerR, T.head_color);
          emitRim(e[1], e[2]);
          break;
        case 12: // c2, c3 in (top)
          emitPoly([e[3], e[1], cor[2], cor[3]], innerR, T.mouth_color);
          emitPoly([cor[0], cor[1], e[1], e[3]], outerR, T.head_color);
          emitRim(e[3], e[1]);
          break;
        case 13: // all but c1
          emitPoly([cor[0], e[0], e[1], cor[2], cor[3]], innerR, T.mouth_color);
          emitPoly([e[0], cor[1], e[1]], outerR, T.head_color);
          emitRim(e[0], e[1]);
          break;
        case 14: // all but c0
          emitPoly([e[0], cor[1], cor[2], cor[3], e[3]], innerR, T.mouth_color);
          emitPoly([cor[0], e[0], e[3]], outerR, T.head_color);
          emitRim(e[3], e[0]);
          break;
        case 5:  // diagonal c0, c2 — ambiguous
        case 10: { // diagonal c1, c3 — ambiguous
          // Disambiguate by centre-sample. No rim emitted for this cell
          // (very rare for simply-connected mouth shapes; adjacent cells
          // still contribute rim segments at their own shared edges).
          const latC = (lat0 + lat1) * 0.5;
          const lonC = (lon0 + lon1) * 0.5;
          if (mouthDist(latC, lonC, T) <= 0) emitPoly(cor, innerR, T.mouth_color);
          else emitPoly(cor, outerR, T.head_color);
          break;
        }
      }
    }
  }

  /* ── Ears ── Twin thick discs.
     Tilt (Rx) leans the disc up/down.
     Yaw (Ry, mirrored) turns each ear outward/inward — facial symmetry.
     ear_offset_z shifts forward/backward. ── */
  for (const side of [+1, -1]) {
    const cx = side * T.ear_offset_x;
    const cy = headY + T.ear_lift_y;
    const cz = T.ear_offset_z || 0;

    // Compose tilt (Rx) then yaw (Ry, mirrored by side).
    // Start: U=[1,0,0], V=[0,1,0], N=[0,0,1].
    // After tilt Rx(tilt):  U=[1,0,0], V=[0,c,-s], N=[0,s,c]  (where c=cos, s=sin of tilt — but
    // our tilt convention has N=[0,-sinE,cosE] so s is negated for N.y)
    const tiltA = T.ear_tilt;
    const cT = Math.cos(tiltA), sT = Math.sin(tiltA);
    // After tilt:
    let uX = 1, uY = 0, uZ = 0;
    let vX = 0, vY = cT, vZ = sT;
    let nX = 0, nY = -sT, nZ = cT;

    // Yaw around Y by (side * ear_yaw): right ear turns right, left turns left.
    const yawA = side * (T.ear_yaw || 0);
    const cY = Math.cos(yawA), sY = Math.sin(yawA);
    // Ry: x' = x*cY + z*sY,  z' = -x*sY + z*cY,  y' = y
    const applyYaw = (x, y, z) => [x*cY + z*sY, y, -x*sY + z*cY];
    [uX, uY, uZ] = applyYaw(uX, uY, uZ);
    [vX, vY, vZ] = applyYaw(vX, vY, vZ);
    [nX, nY, nZ] = applyYaw(nX, nY, nZ);

    const axisU = [uX, uY, uZ];
    const axisV = [vX, vY, vZ];
    const axisN = [nX, nY, nZ];

    const n = Math.max(8, T.ear_segs);
    const r = T.ear_r;
    const half = T.ear_thick * 0.5;
    for (let s = 0; s < n; s++) {
      const a0 = (s / n) * Math.PI * 2;
      const a1 = ((s + 1) % n) / n * Math.PI * 2;
      const c0 = Math.cos(a0), s0 = Math.sin(a0);
      const c1 = Math.cos(a1), s1 = Math.sin(a1);
      // Ring points in world coords, offset ±half along face normal
      const rAx = axisU[0]*c0*r + axisV[0]*s0*r;
      const rAy = axisU[1]*c0*r + axisV[1]*s0*r;
      const rAz = axisU[2]*c0*r + axisV[2]*s0*r;
      const rBx = axisU[0]*c1*r + axisV[0]*s1*r;
      const rBy = axisU[1]*c1*r + axisV[1]*s1*r;
      const rBz = axisU[2]*c1*r + axisV[2]*s1*r;
      const oA = rotV([cx+rAx+axisN[0]*half, cy+rAy+axisN[1]*half, cz+rAz+axisN[2]*half]);
      const oB = rotV([cx+rBx+axisN[0]*half, cy+rBy+axisN[1]*half, cz+rBz+axisN[2]*half]);
      const iA = rotV([cx+rAx-axisN[0]*half, cy+rAy-axisN[1]*half, cz+rAz-axisN[2]*half]);
      const iB = rotV([cx+rBx-axisN[0]*half, cy+rBy-axisN[1]*half, cz+rBz-axisN[2]*half]);
      addQuad(oA, iA, iB, oB, T.ear_color);
    }
    for (let capSide = 0; capSide < 2; capSide++) {
      const cH = capSide === 0 ? half : -half;
      const capC = rotV([cx + axisN[0]*cH, cy + axisN[1]*cH, cz + axisN[2]*cH]);
      for (let s = 0; s < n; s++) {
        const a0 = (s / n) * Math.PI * 2;
        const a1 = ((s + 1) % n) / n * Math.PI * 2;
        const c0 = Math.cos(a0), s0 = Math.sin(a0);
        const c1 = Math.cos(a1), s1 = Math.sin(a1);
        const p0 = rotV([cx + axisU[0]*c0*r + axisV[0]*s0*r + axisN[0]*cH,
                         cy + axisU[1]*c0*r + axisV[1]*s0*r + axisN[1]*cH,
                         cz + axisU[2]*c0*r + axisV[2]*s0*r + axisN[2]*cH]);
        const p1 = rotV([cx + axisU[0]*c1*r + axisV[0]*s1*r + axisN[0]*cH,
                         cy + axisU[1]*c1*r + axisV[1]*s1*r + axisN[1]*cH,
                         cz + axisU[2]*c1*r + axisV[2]*s1*r + axisN[2]*cH]);
        if (capSide === 0) addTri(capC, p0, p1, T.ear_color);
        else               addTri(capC, p1, p0, T.ear_color);
      }
    }
  }

  /* ── Eyes ── Full spheres on the front hemisphere. ── */
  {
    const latE = Math.max(5, Math.round(T.eye_segs));
    const lonE = Math.max(6, Math.round(T.eye_segs * 1.4));
    const r = T.eye_r;
    for (const side of [+1, -1]) {
      const ex = side * T.eye_offset_x;
      const ey = T.eye_offset_y;
      const remSq = T.head_r*T.head_r - ex*ex - (ey/sy)*(ey/sy);
      const ez = Math.sqrt(Math.max(0.01, remSq));
      const nrm = normalize3([ex, ey, ez]);
      const cx = ex + nrm[0]*T.eye_protrude;
      const cy = headY + ey + nrm[1]*T.eye_protrude;
      const cz = ez + nrm[2]*T.eye_protrude;
      for (let i = 0; i < latE; i++) {
        const lat0 = (i / latE - 0.5) * Math.PI;
        const lat1 = ((i + 1) / latE - 0.5) * Math.PI;
        const y0 = r*Math.sin(lat0), y1 = r*Math.sin(lat1);
        const r0 = r*Math.cos(lat0), r1 = r*Math.cos(lat1);
        for (let j = 0; j < lonE; j++) {
          const lon0 = (j / lonE) * 2 * Math.PI;
          const lon1 = ((j + 1) / lonE) * 2 * Math.PI;
          const c0 = Math.cos(lon0), s0 = Math.sin(lon0);
          const c1 = Math.cos(lon1), s1 = Math.sin(lon1);
          const v00 = rotV([cx + r0*c0, cy + y0, cz + r0*s0]);
          const v01 = rotV([cx + r0*c1, cy + y0, cz + r0*s1]);
          const v10 = rotV([cx + r1*c0, cy + y1, cz + r1*s0]);
          const v11 = rotV([cx + r1*c1, cy + y1, cz + r1*s1]);
          addQuad(v00, v10, v11, v01, T.eye_color);
        }
      }
    }
  }

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

/* ═══ 2D FRONT PROFILE ═══ */
function render2D(ctx, W, H, T) {
  ctx.clearRect(0, 0, W, H);
  ctx.fillStyle = "#111114"; ctx.fillRect(0, 0, W, H);
  const headY = T.head_r * T.head_squash;
  const topY = headY + T.ear_lift_y + T.ear_r;
  const widest = Math.max(T.ear_offset_x + T.ear_r, T.head_r * 1.05);
  const mg = { l: 20, r: 20, t: 12, b: 12 }, pw = W - mg.l - mg.r, ph = H - mg.t - mg.b;
  const scaleY = ph / (topY * 1.05 || 1);
  const scaleR = pw / 2 / (widest * 1.15 || 1);
  const sc = Math.min(scaleY, scaleR), cx = W / 2, baseY = mg.t + ph;
  const toS = (x, y) => [cx + x * sc, baseY - y * sc];

  ctx.strokeStyle = "rgba(100,200,100,0.25)"; ctx.lineWidth = 1; ctx.setLineDash([4, 3]);
  ctx.beginPath(); ctx.moveTo(mg.l, baseY); ctx.lineTo(W - mg.r, baseY); ctx.stroke(); ctx.setLineDash([]);

  ctx.fillStyle = rgb01(...T.head_color);
  ctx.beginPath();
  ctx.ellipse(...toS(0, headY), T.head_r * sc, T.head_r * T.head_squash * sc, 0, 0, Math.PI*2);
  ctx.fill();

  for (const side of [+1, -1]) {
    const [ex, ey] = toS(side * T.ear_offset_x, headY + T.ear_lift_y);
    ctx.fillStyle = rgb01(...T.ear_color);
    ctx.beginPath();
    ctx.arc(ex, ey, T.ear_r * sc, 0, Math.PI * 2);
    ctx.fill();
  }

  // Eyes
  for (const side of [+1, -1]) {
    const [ex, ey] = toS(side * T.eye_offset_x, headY + T.eye_offset_y);
    const base = T.eye_color;
    ctx.fillStyle = rgb01(base[0]*0.85, base[1]*0.85, base[2]*0.85);
    ctx.beginPath();
    ctx.arc(ex, ey, T.eye_r * sc, 0, Math.PI * 2);
    ctx.fill();
    const hl = 0.35;
    ctx.fillStyle = rgb01(base[0]+(1-base[0])*hl, base[1]+(1-base[1])*hl, base[2]+(1-base[2])*hl);
    ctx.beginPath();
    ctx.arc(ex - T.eye_r * sc * 0.25, ey - T.eye_r * sc * 0.25, T.eye_r * sc * 0.25, 0, Math.PI * 2);
    ctx.fill();
  }

  // Mouth — trace projected outline. Centre = lat_c(t) = lat_centre + smile·t²,
  // Upper edge = lat_c + lat_upper, lower edge = lat_c - lat_lower.
  {
    const NSEG = 40;
    ctx.fillStyle = rgb01(...T.mouth_color);
    ctx.beginPath();
    for (let i = 0; i <= NSEG; i++) {
      const t = -1 + 2*i/NSEG;
      const lon = Math.PI/2 + t * T.mouth_lon_half;
      const lat = T.mouth_lat_center + T.mouth_smile * t * t + T.mouth_lat_upper;
      const Xw = T.head_r * Math.cos(lat) * Math.cos(lon);
      const Yw = T.head_r * T.head_squash * Math.sin(lat);
      const [sx, sy] = toS(Xw, headY + Yw);
      if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
    }
    for (let i = NSEG; i >= 0; i--) {
      const t = -1 + 2*i/NSEG;
      const lon = Math.PI/2 + t * T.mouth_lon_half;
      const lat = T.mouth_lat_center + T.mouth_smile * t * t - T.mouth_lat_lower;
      const Xw = T.head_r * Math.cos(lat) * Math.cos(lon);
      const Yw = T.head_r * T.head_squash * Math.sin(lat);
      ctx.lineTo(...toS(Xw, headY + Yw));
    }
    ctx.closePath();
    ctx.fill();
    ctx.strokeStyle = rgb01(T.glow_color[0], T.glow_color[1], T.glow_color[2]);
    ctx.lineWidth = 1.1;
    ctx.beginPath();
    for (let i = 0; i <= NSEG; i++) {
      const t = -1 + 2*i/NSEG;
      const lon = Math.PI/2 + t * T.mouth_lon_half;
      const lat = T.mouth_lat_center + T.mouth_smile * t * t + T.mouth_lat_upper;
      const Xw = T.head_r * Math.cos(lat) * Math.cos(lon);
      const Yw = T.head_r * T.head_squash * Math.sin(lat);
      const [sx, sy] = toS(Xw, headY + Yw);
      if (i === 0) ctx.moveTo(sx, sy); else ctx.lineTo(sx, sy);
    }
    ctx.stroke();
  }

  ctx.fillStyle = "rgba(255,255,255,0.3)"; ctx.font = "9px monospace"; ctx.textAlign = "center";
  ctx.fillText(`r=${T.head_r.toFixed(2)}`, cx, toS(0, headY)[1] + 3);
}

/* ═══ C++ EXPORT — single struct, no tier array ═══ */
function genCpp(T) {
  const f = (v, d = 3) => { const s = Number(v).toFixed(d); return s.includes(".") ? s + "f" : s + ".0f"; };
  const c = a => `{${f(a[0])}, ${f(a[1])}, ${f(a[2])}}`;
  return [
    "// ─── Mau5 Params (Deadmau5 tribute entity) ─────────────────────────",
    "//",
    "// Single-preset parameters. Mouth is carved in the sphere's own (lat,",
    "// lon) coordinates so the hole boundary aligns with the tessellation",
    "// grid; the inner-sphere mouth interior and the outer-sphere head are",
    "// one connected mesh, joined by rim walls along grid edges.",
    "",
    "struct Mau5Params {",
    "  float head_r, head_squash;",
    "  float ear_r, ear_thick, ear_offset_x, ear_lift_y, ear_offset_z, ear_tilt, ear_yaw;",
    "  float eye_r, eye_offset_x, eye_offset_y, eye_protrude;",
    "  // Mouth in spherical coords (radians except depth). Band centre",
    "  //   lat_c(t) = mouth_lat_center + mouth_smile · t²",
    "  // Band half-thickness",
      "  float mouth_lon_half;",
    "  float mouth_lat_center;",
    "  float mouth_lat_upper;",
    "  float mouth_lat_lower;",
    "  float mouth_smile;",
      "  float mouth_depth;",
    "  // Appearance",
    "  float color_over, head_var, face_var, ear_var, glow_var, eye_var;",
    "  float head_color[3], face_color[3], ear_color[3], eye_color[3], glow_color[3], mouth_color[3];",
    "  // Tessellation",
    "  uint32_t head_lat, head_lon, ear_segs, eye_segs;",
    "};",
    "",
    "static constexpr Mau5Params MAU5_PARAMS = {",
    `  ${f(T.head_r)}, ${f(T.head_squash)},`,
    `  ${f(T.ear_r)}, ${f(T.ear_thick)}, ${f(T.ear_offset_x)}, ${f(T.ear_lift_y)}, ${f(T.ear_offset_z||0)}, ${f(T.ear_tilt)}, ${f(T.ear_yaw||0)},`,
    `  ${f(T.eye_r)}, ${f(T.eye_offset_x)}, ${f(T.eye_offset_y)}, ${f(T.eye_protrude)},`,
    `  ${f(T.mouth_lon_half)}, ${f(T.mouth_lat_center)}, ${f(T.mouth_lat_upper)}, ${f(T.mouth_lat_lower)}, ${f(T.mouth_smile)}, ${f(T.mouth_depth)},`,
    `  ${f(T.color_over)}, ${f(T.head_var)}, ${f(T.face_var)}, ${f(T.ear_var)}, ${f(T.glow_var)}, ${f(T.eye_var || 0.03)},`,
    `  ${c(T.head_color)}, ${c(T.face_color)}, ${c(T.ear_color)}, ${c(T.eye_color)}, ${c(T.glow_color)}, ${c(T.mouth_color)},`,
    `  ${T.head_lat|0}u, ${T.head_lon|0}u, ${T.ear_segs|0}u, ${T.eye_segs|0}u`,
    "};",
  ].join("\n");
}

async function copyText(text) {
  try {
    if (navigator.clipboard?.writeText) { await navigator.clipboard.writeText(text); return true; }
  } catch {}
  try {
    const ta = document.createElement("textarea");
    ta.value = text;
    ta.style.position = "fixed"; ta.style.top = "-1000px";
    ta.setAttribute("readonly", "");
    document.body.appendChild(ta);
    ta.select();
    document.execCommand("copy");
    document.body.removeChild(ta);
    return true;
  } catch { return false; }
}

/* ═══ UI COMPONENTS ═══ */
const ist = { padding: "2px 3px", fontSize: 11, fontFamily: "monospace", borderRadius: 4, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)", textAlign: "right" };
const rw  = { display: "flex", flexWrap: "wrap", gap: "3px 8px", alignItems: "center", marginBottom: 3 };
const lb  = { fontSize: 10, color: "var(--color-text-secondary)", minWidth: 110 };
const btn = { fontSize: 10, padding: "2px 8px", borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-secondary)" };

function Num({ value, onChange, min = 0, max = 10, w = 56 }) {
  const [txt, setTxt] = useState(String(Math.round(value * 10000) / 10000));
  const [focused, setFocused] = useState(false);
  useEffect(() => { setTxt(String(Math.round(value * 10000) / 10000)); }, [value]);
  const commit = () => {
    const v = parseFloat(txt);
    if (!isNaN(v)) onChange(Math.max(min, Math.min(max, v)));
    else setTxt(String(Math.round(value * 10000) / 10000));
    setFocused(false);
  };
  return (
    <input type="text" value={txt}
      title={`${min} – ${max}`}
      onChange={e => setTxt(e.target.value)}
      onFocus={() => setFocused(true)}
      onBlur={commit}
      onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }}
      style={{ ...ist, width: w, outline: focused ? "1.5px solid var(--color-border-info)" : "none" }} />
  );
}
function Row({ label, children }) {
  return <div style={rw}><span style={lb}>{label}</span>{children}</div>;
}
function PaletteSwatches({ palette, label, onPick }) {
  return (
    <div style={{ ...rw, marginBottom: 5 }}>
      <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginRight: 4 }}>{label}</span>
      {palette.map((p, i) => (
        <span key={i} title={p.name} onClick={() => onPick(p.c.slice())}
          style={{ width: 14, height: 14, borderRadius: 3, cursor: "pointer",
                   background: rgb01(...p.c), border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
      ))}
    </div>
  );
}

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
      const p = dragRef.current.parentElement.getBoundingClientRect();
      setPos({ x: e.clientX - p.left - offsetRef.current.x, y: e.clientY - p.top - offsetRef.current.y });
    };
    const onUp = () => setDragging(false);
    window.addEventListener("mousemove", onMove);
    window.addEventListener("mouseup", onUp);
    return () => { window.removeEventListener("mousemove", onMove); window.removeEventListener("mouseup", onUp); };
  }, [dragging]);
  const doDock = () => { setPos(null); if (onDock) onDock(id); };
  const style = pos ? { position: "absolute", left: pos.x, top: pos.y, zIndex: 100, maxWidth: 520, minWidth: 280 } : {};
  return (
    <div ref={dragRef} style={{
      marginBottom: pos ? 0 : 5,
      border: pos ? "1px solid #666" : "1px solid var(--color-border-tertiary)",
      borderRadius: 8, overflow: "hidden",
      background: pos ? "#2a2a30" : "var(--color-background-primary)",
      boxShadow: pos ? "0 8px 32px rgba(0,0,0,.6)" : "none",
      ...style
    }}>
      <div onMouseDown={startDrag} style={{
        padding: "5px 10px", fontSize: 12, fontWeight: 500, userSelect: "none",
        display: "flex", alignItems: "center", gap: 5,
        color: pos ? "#e0ddd8" : "var(--color-text-primary)",
        background: pos ? "#353540" : "var(--color-background-secondary)",
        cursor: "grab"
      }}>
        <span data-notdrag="1" onClick={() => setOpen(!open)} style={{
          cursor: "pointer", fontSize: 9, transition: "transform .15s",
          display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px"
        }}>▶</span>
        <span style={{ flex: 1 }}>{title}</span>
        {pos && <span data-notdrag="1" onClick={doDock} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}
      </div>
      {open && <div style={{
        padding: "5px 10px",
        maxHeight: pos ? 400 : "none",
        overflowY: pos ? "auto" : "visible",
        color: pos ? "#d0cdc8" : undefined
      }}>{children}</div>}
    </div>
  );
}

/* ═══ PORTABLE STORAGE ═══ */
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
const STORAGE_KEY = "7t:mau5:v5";  // single-preset, spherical-coord mouth

const DEFAULTS_JSON = JSON.stringify(DEFAULTS);

/* ═══ MAIN COMPONENT ═══ */
export default function Mau5Designer() {
  const [T, setT] = useState(() => ({ ...DEFAULTS }));
  const [loaded, setLoaded] = useState(false);
  const isModified = JSON.stringify(T) !== DEFAULTS_JSON;
  const upd = (fn) => setT(prev => { const n = JSON.parse(JSON.stringify(prev)); fn(n); return n; });

  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) {
          const data = JSON.parse(raw);
          if (data && typeof data === "object") {
            // Defensive merge: unknown saved fields are ignored, missing ones fall back to defaults.
            setT({ ...DEFAULTS, ...data });
          }
        }
      } catch {}
      setLoaded(true);
    })();
  }, []);

  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(T)); } catch {} })();
  }, [T, loaded]);

  const [seed, setSeed] = useState(42);
  const resolved = resolveColors(T, seed);
  const RT = { ...T,
    head_color:  resolved.headCol,
    face_color:  resolved.faceCol,
    ear_color:   resolved.earCol,
    eye_color:   resolved.eyeCol,
    glow_color:  resolved.glowCol,
    mouth_color: resolved.mouthCol };

  const [rotY, setRotY] = useState(0.4);
  const [tilt, setTilt] = useState(0.15);
  const [roll, setRoll] = useState(0);
  const [zoom, setZoom] = useState(1.0);
  const [autoRot, setAutoRot] = useState(true);
  const cv3dRef = useRef(null);
  const cv2dRef = useRef(null);
  const animRef = useRef(null);

  const [showCode, setShowCode] = useState(false);
  const [copied, setCopied] = useState(false);

  const defaultOrder = ["mouth", "head", "ears", "eyes", "appearance", "tessellation", "export"];
  const [panelOrder, setPanelOrder] = useState([...defaultOrder]);
  const [dockKey, setDockKey] = useState(0);
  const resetAll = () => { setT({ ...DEFAULTS }); setDockKey(k => k + 1); };
  const cppCode = genCpp(T);

  // Render loop
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
        render3D(ctx, rect.width, rect.height, RT, r, tilt, roll, zoom);
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
  }, [RT, rotY, tilt, roll, zoom, autoRot]);

  // Camera: drag rotates, shift+drag rolls, wheel zooms.
  // Uses useCallback + React onPointerDown (ribbon designer pattern) —
  // the handler is attached via JSX prop, not addEventListener.
  const onPointerDown3D = useCallback(e => {
    const sx = e.clientX, sy = e.clientY;
    const sr = rotY, st = tilt, sl = roll, sz = zoom;
    const shift = e.shiftKey;
    const sens = 0.012 / Math.max(sz, 0.3);
    setAutoRot(false);
    const onMove = ev => {
      const dx = ev.clientX - sx, dy = ev.clientY - sy;
      if (shift) {
        setRoll(sl - dx * sens);
      } else {
        setRotY(sr + dx * sens);   // +dx = turntable (ribbon)
      }
      setTilt(st + dy * sens * 0.7); // +dy = drag-up tilts face toward you
    };
    const onUp = () => {
      window.removeEventListener("pointermove", onMove);
      window.removeEventListener("pointerup", onUp);
    };
    window.addEventListener("pointermove", onMove);
    window.addEventListener("pointerup", onUp);
  }, [rotY, tilt, roll, zoom]);

  // Scroll-to-zoom: non-passive listener so preventDefault() works.
  useEffect(() => {
    const cv = cv3dRef.current;
    if (!cv) return;
    const handler = e => {
      e.preventDefault();
      setZoom(z => Math.max(0.3, Math.min(5.0, z * (e.deltaY > 0 ? 0.90 : 1.11))));
    };
    cv.addEventListener("wheel", handler, { passive: false });
    return () => cv.removeEventListener("wheel", handler);
  }, []);

  const onCopy = async () => {
    const ok = await copyText(cppCode);
    if (ok) { setCopied(true); setTimeout(() => setCopied(false), 1500); }
  };

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "#555" }}>Loading…</div>;

  const deg = (rad) => `${(rad * 180 / Math.PI).toFixed(1)}°`;

  return (
    <div style={{ display: "flex", gap: 8, height: "100vh", padding: 8,
                  fontFamily: "'JetBrains Mono', monospace", fontSize: 11,
                  overflow: "hidden", background: "var(--color-background-primary)",
                  color: "var(--color-text-primary)", boxSizing: "border-box", position: "relative" }}>

      <div style={{ flex: "1 1 55%", display: "flex", flexDirection: "column", gap: 6, minWidth: 0 }}>
        <div style={{ display: "flex", gap: 4, alignItems: "center", flexWrap: "wrap" }}>
          <span style={{ fontSize: 12, fontWeight: 500, color: "var(--color-text-primary)" }}>Mau5 Designer</span>
          <span style={{ flex: 1 }} />
          <button onClick={() => setAutoRot(!autoRot)} style={{ ...btn, opacity: autoRot ? 1 : 0.5 }}>{autoRot ? "◉ spin" : "○ spin"}</button>
          <button onClick={() => setZoom(z => Math.max(0.3, z * 0.8))} style={btn} title="Zoom out">−</button>
          <button onClick={() => { setZoom(1.0); setRotY(0.4); setTilt(0.15); setRoll(0); }} style={btn} title="Reset camera">
            ⊙ {zoom.toFixed(2)}×
          </button>
          <button onClick={() => setZoom(z => Math.min(5.0, z * 1.25))} style={btn} title="Zoom in">+</button>
          <button onClick={resetAll} style={btn}>Reset</button>
          {isModified && <span style={{ fontSize: 9, color: "#666", marginLeft: 4 }}>● modified</span>}
        </div>
        <div style={{ display: "flex", gap: 6, alignItems: "center", fontSize: 10, flexWrap: "wrap" }}>
          <span style={{ color: "var(--color-text-secondary)" }}>seed</span>
          <Num value={seed} min={0} max={99999} w={52} onChange={v => setSeed(Math.round(v))} />
          <button onClick={() => setSeed(Math.floor(Math.random() * 99999))} style={btn}>reroll</button>
          <span style={{ color: "var(--color-text-tertiary)", marginLeft: 4 }}>head</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.headCol),
                         border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
          <span style={{ color: "var(--color-text-tertiary)" }}>mouth</span>
          <span style={{ width: 16, height: 16, borderRadius: 3, background: rgb01(...resolved.mouthCol),
                         border: "1px solid var(--color-border-tertiary)", display: "inline-block" }} />
        </div>
        <div style={{ flex: "1 1 65%", background: "#1a1a1e", borderRadius: 8, overflow: "hidden",
                      cursor: "grab", minHeight: 180, position: "relative" }}>
          <canvas ref={cv3dRef} onPointerDown={onPointerDown3D} style={{ width: "100%", height: "100%", display: "block", cursor: "grab" }} />
          <div style={{ position: "absolute", bottom: 6, left: 8, right: 8,
                        display: "flex", justifyContent: "space-between",
                        fontSize: 9, color: "rgba(255,255,255,0.45)",
                        fontFamily: "monospace", pointerEvents: "none", userSelect: "none" }}>
            <span>drag rotate · shift+drag roll · wheel zoom</span>
            <span>yaw {deg(rotY)} · pitch {deg(tilt)} · roll {deg(roll)}</span>
          </div>
        </div>
        <div style={{ flex: "0 0 28%", background: "#111114", borderRadius: 8, overflow: "hidden", minHeight: 100 }}>
          <canvas ref={cv2dRef} style={{ width: "100%", height: "100%", display: "block" }} />
        </div>
      </div>

      <div style={{ flex: "0 0 320px", overflowY: "auto", overflowX: "hidden", paddingRight: 2 }}>
        {panelOrder.map(pid => {
          const dp = { key: pid, id: pid, resetKey: dockKey,
            onDock: (id) => setPanelOrder(prev => {
              const w = prev.filter(p => p !== id);
              const oi = defaultOrder.indexOf(id);
              const at = w.findIndex(p => defaultOrder.indexOf(p) > oi);
              return at === -1 ? [...w, id] : [...w.slice(0, at), id, ...w.slice(at)];
            }) };

          switch (pid) {

            case "mouth": return (<DragPanel {...dp} title="Mouth (Carved Smile)" ini={true}>
              <Row label={`Width, horizontal (${deg(T.mouth_lon_half * 2)} total)`}>
                <Num value={T.mouth_lon_half} min={0.05} max={Math.PI} onChange={v => upd(n => { n.mouth_lon_half = v; })} />
              </Row>
              <Row label={`Lat. centre (${deg(T.mouth_lat_center)})`}>
                <Num value={T.mouth_lat_center} min={-1.5} max={1.5} onChange={v => upd(n => { n.mouth_lat_center = v; })} />
              </Row>
              <Row label={`Upper lip (${deg(T.mouth_lat_upper)})`}>
                <Num value={T.mouth_lat_upper} min={0.01} max={1.5} onChange={v => upd(n => { n.mouth_lat_upper = v; })} />
              </Row>
              <Row label={`Lower lip (${deg(T.mouth_lat_lower)})`}>
                <Num value={T.mouth_lat_lower} min={0.01} max={1.5} onChange={v => upd(n => { n.mouth_lat_lower = v; })} />
              </Row>
              <Row label={`Smile rise (${deg(T.mouth_smile)})`}>
                <Num value={T.mouth_smile} min={-1.0} max={1.0} onChange={v => upd(n => { n.mouth_smile = v; })} />
              </Row>
              <Row label="Depth (world)">
                <Num value={T.mouth_depth} min={0.01} max={1.2} onChange={v => upd(n => { n.mouth_depth = v; })} />
              </Row>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 4, lineHeight: 1.4 }}>
                Mouth region in spherical coords: a band on the front hemisphere. Band centre follows <i>lat_c(t) = lat_centre + smile·t²</i>. Upper and lower lip extents are independent — the upper lip reaches <i>lat_c + lat_upper</i>, the lower lip reaches <i>lat_c − lat_lower</i>. The rim glows with the Glow colour.
                <br/><br/>
                <b>Smooth at any tessellation:</b> boundary crossings are linearly interpolated via marching squares on a Chebyshev signed distance function, so the rim lies on the analytic smile curve rather than snapping to grid rows.
              </div>
            </DragPanel>);

            case "head": return (<DragPanel {...dp} title="Head Sphere">
              <Row label="Radius">
                <Num value={T.head_r} min={0.1} max={10} onChange={v => upd(n => { n.head_r = v; })} />
              </Row>
              <Row label="Y-squash">
                <Num value={T.head_squash} min={0.6} max={1.4} onChange={v => upd(n => { n.head_squash = v; })} />
              </Row>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2 }}>
                UV sphere, floating (engine positions spawn height). Y-squash = 1 is a true round sphere.
              </div>
            </DragPanel>);

            case "ears": return (<DragPanel {...dp} title="Ears">
              <Row label="Radius"><Num value={T.ear_r} min={0.1} max={5} onChange={v => upd(n => { n.ear_r = v; })} /></Row>
              <Row label="Thickness"><Num value={T.ear_thick} min={0.02} max={1.5} onChange={v => upd(n => { n.ear_thick = v; })} /></Row>
              <Row label="Offset X"><Num value={T.ear_offset_x} min={0.1} max={6} onChange={v => upd(n => { n.ear_offset_x = v; })} /></Row>
              <Row label="Lift Y"><Num value={T.ear_lift_y} min={0} max={5} onChange={v => upd(n => { n.ear_lift_y = v; })} /></Row>
              <Row label="Tilt (rad)"><Num value={T.ear_tilt} min={-0.5} max={0.8} onChange={v => upd(n => { n.ear_tilt = v; })} /></Row>
              <Row label="Yaw (rad)"><Num value={T.ear_yaw || 0} min={-1.2} max={1.2} onChange={v => upd(n => { n.ear_yaw = v; })} /></Row>
              <Row label="Offset Z"><Num value={T.ear_offset_z || 0} min={-2} max={2} onChange={v => upd(n => { n.ear_offset_z = v; })} /></Row>
            </DragPanel>);

            case "eyes": return (<DragPanel {...dp} title="Eyes (Bumps)">
              <Row label="Radius"><Num value={T.eye_r} min={0.02} max={2} onChange={v => upd(n => { n.eye_r = v; })} /></Row>
              <Row label="Offset X"><Num value={T.eye_offset_x} min={0.05} max={3} onChange={v => upd(n => { n.eye_offset_x = v; })} /></Row>
              <Row label="Offset Y"><Num value={T.eye_offset_y} min={-2} max={2} onChange={v => upd(n => { n.eye_offset_y = v; })} /></Row>
              <Row label="Protrude"><Num value={T.eye_protrude} min={-0.5} max={1} onChange={v => upd(n => { n.eye_protrude = v; })} /></Row>
            </DragPanel>);

            case "appearance": return (<DragPanel {...dp} title="Colours" ini={true}>
              {[
                { label: "Head",  key: "head_color",  pal: HEAD_PALETTE },
                { label: "Mouth", key: "mouth_color",  pal: MOUTH_PALETTE },
                { label: "Ears",  key: "ear_color",   pal: HEAD_PALETTE },
                { label: "Eyes",  key: "eye_color",   pal: EYE_PALETTE },
                { label: "Glow",  key: "glow_color",  pal: GLOW_PALETTE },
              ].map(({ label, key, pal }) => (
                <div key={key} style={{ marginBottom: 6 }}>
                  <div style={{ display: "flex", alignItems: "center", gap: 5, marginBottom: 2 }}>
                    <span style={{ width: 16, height: 16, borderRadius: 3,
                                   background: rgb01(...(T[key] || [0,0,0])),
                                   border: "1px solid var(--color-border-tertiary)", display: "inline-block", flexShrink: 0 }} />
                    <span style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)" }}>{label}</span>
                  </div>
                  {pal && <PaletteSwatches palette={pal} label="" onPick={c => upd(n => { n[key] = c; })} />}
                  <Row label="">
                    {["R","G","B"].map((ch, i) => <span key={key+ch} style={{ display: "contents" }}>
                      <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span>
                      <Num value={(T[key]||[0,0,0])[i]} min={0} max={1} w={40} onChange={v => upd(n => {
                        if (!n[key]) n[key] = [0,0,0];
                        n[key][i] = v;
                      })} />
                    </span>)}
                  </Row>
                </div>
              ))}
              <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "6px 0 3px" }}>
                Face (engine audio-envelope slot)
              </div>
              <Row label="">
                {["R","G","B"].map((ch, i) => <span key={"f"+i} style={{ display: "contents" }}>
                  <span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{ch}</span>
                  <Num value={T.face_color[i]} min={0} max={1} w={40} onChange={v => upd(n => { n.face_color[i] = v; })} />
                </span>)}
              </Row>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 4, lineHeight: 1.4 }}>
                Per-instance palette override % and per-channel variance are applied at runtime by the hash system. Override % = chance a spawned instance picks a random palette entry instead of the base colour.
              </div>
              <Row label="Override %"><Num value={T.color_over} min={0} max={1} w={48} onChange={v => upd(n => { n.color_over = v; })} /></Row>
              <Row label="Head var"><Num value={T.head_var} min={0} max={0.3} w={48} onChange={v => upd(n => { n.head_var = v; })} /></Row>
              <Row label="Ear var"><Num value={T.ear_var} min={0} max={0.3} w={48} onChange={v => upd(n => { n.ear_var = v; })} /></Row>
              <Row label="Eye var"><Num value={T.eye_var || 0.03} min={0} max={0.3} w={48} onChange={v => upd(n => { n.eye_var = v; })} /></Row>
            </DragPanel>);

            case "tessellation": return (<DragPanel {...dp} title="Tessellation">
              <Row label="Head lat"><Num value={T.head_lat} min={12} max={96} w={48} onChange={v => upd(n => { n.head_lat = Math.round(v); })} /></Row>
              <Row label="Head lon"><Num value={T.head_lon} min={16} max={144} w={48} onChange={v => upd(n => { n.head_lon = Math.round(v); })} /></Row>
              <Row label="Ear segs"><Num value={T.ear_segs} min={8} max={36} w={48} onChange={v => upd(n => { n.ear_segs = Math.round(v); })} /></Row>
              <Row label="Eye segs"><Num value={T.eye_segs} min={5} max={20} w={48} onChange={v => upd(n => { n.eye_segs = Math.round(v); })} /></Row>
              <div style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginTop: 2, lineHeight: 1.4 }}>
                Higher lat/lon = smoother mouth rim (the rim follows grid lines). 48×72 is the sweet spot for this mouth shape; bump to 72×108 for pixel-perfect smile curves on the Stage hero shot.
              </div>
            </DragPanel>);

            case "export": return (<DragPanel {...dp} title="C++ Export">
              <div style={{ display: "flex", gap: 4, marginBottom: 4 }}>
                <button onClick={onCopy} style={{ ...btn, background: copied ? "var(--color-background-success)" : "var(--color-background-secondary)" }}>
                  {copied ? "Copied" : "Copy C++"}
                </button>
                <button onClick={() => setShowCode(!showCode)} style={btn}>{showCode ? "Hide" : "Show"} code</button>
              </div>
              {showCode && <textarea readOnly value={cppCode} style={{
                width: "100%", height: 180, fontSize: 10,
                fontFamily: "'JetBrains Mono',monospace",
                background: "var(--color-background-primary)",
                color: "var(--color-text-primary)",
                border: "1px solid var(--color-border-tertiary)",
                borderRadius: 4, padding: 6, resize: "vertical",
                lineHeight: 1.4, whiteSpace: "pre"
              }} />}
            </DragPanel>);

            default: return null;
          }
        })}
      </div>
    </div>
  );
}
