import { useState, useMemo, useRef, useEffect, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T POPULATION DESIGNER — the spawn-probability stack as an instrument.

   Born from audit/COMPOSITION_RECON.md (R2b ruling): the §2 worked-example
   trace as a LIVE readout. Pick a family + mood + tile; watch the stack
   multiply out through the same law the engine runs
   (compose_spawn_chance — mood → global → tile(F3) → [proximity] →
   base × adj → clamp → roll). Spatial-density map, per-factor sliders,
   C++ export in the tables' own shapes.

   All constants mirror the tree @ the R1 collapse (commit 86a9899); each
   block cites its C++ home. Integer hashes are bit-exact ports
   (Math.imul); float math runs in doubles — display parity, the engine
   stays the authority.
   ═══════════════════════════════════════════════════════════════════════ */

/* ═══ HASHES — exact ports of primitives/seed_utils.hpp ═══ */
function cpuHash(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return ((h >>> 16) ^ h) >>> 0;
}
const hashF = (s, p) => cpuHash(s, p) / 0xFFFFFFFF;
function tileSeed(master, gx, gz) {
  let h = master >>> 0;
  h = (h ^ Math.imul(gx | 0, 73856093)) >>> 0;
  h = (h ^ Math.imul(gz | 0, 19349663)) >>> 0;
  h = Math.imul(((h ^ (h >>> 16)) >>> 0), 2654435769) >>> 0;
  h = Math.imul(((h ^ (h >>> 16)) >>> 0), 2654435769) >>> 0;
  return h;
}
function latticeNodeSeed(master, nx, nz, band) {
  let h = master >>> 0;
  h = (h ^ Math.imul(nx | 0, 73856093)) >>> 0;
  h = (h ^ Math.imul(nz | 0, 19349663)) >>> 0;
  h = (h ^ Math.imul(band >>> 0, 83492791)) >>> 0;
  h = Math.imul(((h ^ (h >>> 16)) >>> 0), 2654435769) >>> 0;
  h = Math.imul(((h ^ (h >>> 16)) >>> 0), 2654435769) >>> 0;
  return h;
}

/* ═══ FAMILY FACTS — PopFamily order (roster.hpp, F-1 pinned) ═══
   base: per-family SPAWN_CHANCE (gallery = archetype-indexed, see below)
   roll: SPAWN_ROLL property id · mood: MOOD_MULTIPLIER row
   clamp: 'MIN1' | 'RANGE01' | 'NONE' · prox: proximity ON in the law
   seedDomain: 'tile' | 'lattice' (GoL rolls per lattice node, band 250) */
const FAMILIES = [
  { key: "pyramid", name: "Pyramid", base: 0.030, roll: 800,  mood: [1,1,1,1,1,0], clamp: "MIN1", prox: true,  color: "#c4a040" },
  { key: "arch",    name: "Arch",    base: 0.030, roll: 600,  mood: [1,1,1,1,1,1], clamp: "MIN1", prox: true,  color: "#c4786a" },
  { key: "column",  name: "Column",  base: 0.030, roll: 700,  mood: [1,1,1,1,1,0], clamp: "MIN1", prox: true,  color: "#b07acc" },
  { key: "antenna", name: "Antenna", base: 0.025, roll: 900,  mood: [1,1,1,1,1,0], clamp: "MIN1", prox: true,  color: "#8a8aaa" },
  { key: "palm",    name: "Palm",    base: 0.200, roll: 950,  mood: [1,1,1,1,1,0], clamp: "MIN1", prox: true,  color: "#3a8a3a" },
  { key: "cactus",  name: "Cactus",  base: 0.100, roll: 1000, mood: [1,1,1,1,1,0], clamp: "MIN1", prox: true,  color: "#2a6a4a" },
  { key: "blade",   name: "Blade",   base: 0.025, roll: 1100, mood: [1,1,1,1,1,0], clamp: "MIN1", prox: true,  color: "#4a7a3a" },
  { key: "sphere",  name: "Sphere",  base: 0.015, roll: 100,  mood: [1,1,0,0,1,0], clamp: "MIN1", prox: true,  color: "#c0a0d0" },
  { key: "ribbon",  name: "Ribbon",  base: 0.900, roll: 400,  mood: [1,1,0,0,1,0], clamp: "MIN1", prox: true,  color: "#5ab0c4" },
  { key: "cube",    name: "Cube",    base: 0.600, roll: 130,  mood: [1,1,0,0,1,0], clamp: "MIN1", prox: true,  color: "#9a8ab0" },
  { key: "gol",     name: "GoL",     base: 0.150, roll: 920,  mood: [1,1,0,0,1,0], clamp: "RANGE01", prox: false, color: "#70c890", seedDomain: "lattice" },
  { key: "gallery", name: "Gallery", base: null,  roll: 500,  mood: [1,1,1,1,1,0], clamp: "NONE", prox: false, color: "#d4a0a0" },
];
const GALLERY_CHANCE_BY_ARCHETYPE = [0.03, 0.06, 0.30, 0.40]; // gallery.hpp
const ARCH_NAMES = ["mountain", "varied", "basin", "pool"];
const MOOD_NAMES = ["open_default", "open_sunset", "indoor_flat", "indoor_vault", "finite_outdoor", "finite_ref"];

/* ═══ THEMES — population_themes.hpp (spawn_weight in PopFamily order) ═══ */
const THEME_DEFS = [
  { name: "TRANSITION", w: 0.21, density: 1.0, sw: [0.4,0.3,0.7,0.3,0.3,0.3,0.5,0.3,1.0,0.5,0.5,0.5], env: [150,20,3,0],  color: "#b4b4aa" },
  { name: "MONUMENTAL", w: 0.30, density: 1.0, sw: [1.5,1.0,1.0,0.5,0.2,0.2,0.5,0.3,1.0,0.3,0.3,0.3], env: [150,10,10,8], color: "#c88c50" },
  { name: "COLONNADE",  w: 0.31, density: 1.0, sw: [0.3,1.0,4.0,0.5,0.3,0.3,0.5,0.3,1.0,0.3,0.5,0.4], env: [150,15,6,6],  color: "#64a0c8" },
  { name: "ANTENNA",    w: 0.18, density: 1.0, sw: [0.5,0.5,1.0,4.0,0.5,0.5,0.5,0.3,1.0,0.3,0.3,0.3], env: [180,10,5,5],  color: "#a0c864" },
  { name: "BARREN",     w: 0.04, density: 1.0, sw: [0.4,0.3,0.5,0.3,0.2,0.2,0.1,0.3,1.0,0.1,0.2,0.6], env: [100,12,3,4],  color: "#dcd2be" },
];

/* ═══ LATTICE / DENSITY CONSTANTS — population_themes.hpp:20-32 ═══ */
const PATCH_EXTENT = 50;
const THEME_LATTICE_SPACING = 500, THEME_SEED_BAND = 170, THEME_PICK_PROP = 370;
const DENSITY_DEFAULTS = { spacing: 250, band: 160, exponent: 0.6, min: 1.0, max: 1.0, noiseProp: 350 };

/* ═══ PROXIMITY — spawn_engine.hpp tables (PopFamily order) ═══ */
const PROX = {
  radius:    [0,0,60,0,150,120,120,0,0,0,0,0],
  maxBoost:  [1,1,2,1,3,3,3,1,1,1,1,1],
  threshold: [0,0,2,0,1,1,1,0,0,0,0,0],
  selfAff:   [0,0,0.4,0,0.65,0.5,0.5,0,0,0,0,0], // AFFINITY[f][f]
};
const GLOBAL_ENTITY_DENSITY_DEFAULT = 1.0;

/* ═══ THE TILE LAYER — exact port of generate_tile_population ═══ */
function smooth(f) { return f * f * (3 - 2 * f); }
function selectThemeAtNode(nodeSeed, themeW) {
  const roll = hashF(nodeSeed, THEME_PICK_PROP);
  let total = 0; for (const w of themeW) total += w;
  let c = 0;
  for (let t = 0; t < themeW.length; t++) { c += themeW[t] / total; if (roll < c) return t; }
  return themeW.length - 1;
}
function tilePop(seed, gx, gz, themes, densityCfg) {
  const cx = (gx + 0.5) * PATCH_EXTENT, cz = (gz + 0.5) * PATCH_EXTENT;
  // density noise band (DEAD today: min==max — the wake preview lives here)
  const dlx = cx / densityCfg.spacing, dlz = cz / densityCfg.spacing;
  const dbx = Math.floor(dlx), dbz = Math.floor(dlz);
  const dwx = smooth(dlx - dbx), dwz = smooth(dlz - dbz);
  let noise = 0;
  for (let dz = 0; dz <= 1; dz++) for (let dx = 0; dx <= 1; dx++) {
    const ns = latticeNodeSeed(seed, dbx + dx, dbz + dz, densityCfg.band);
    const shaped = Math.pow(hashF(ns, densityCfg.noiseProp), densityCfg.exponent);
    noise += shaped * ((dx ? dwx : 1 - dwx) * (dz ? dwz : 1 - dwz));
  }
  let entityDensity = densityCfg.min + noise * (densityCfg.max - densityCfg.min);
  // theme field: 4-node bilinear blend
  const tlx = cx / THEME_LATTICE_SPACING, tlz = cz / THEME_LATTICE_SPACING;
  const tbx = Math.floor(tlx), tbz = Math.floor(tlz);
  const twx = smooth(tlx - tbx), twz = smooth(tlz - tbz);
  const spatial = new Array(12).fill(0);
  let blendedDensity = 0;
  const nodes = [];
  const themeW = themes.map((t) => t.w);
  for (let dz = 0; dz <= 1; dz++) for (let dx = 0; dx <= 1; dx++) {
    const ns = latticeNodeSeed(seed, tbx + dx, tbz + dz, THEME_SEED_BAND);
    const tidx = selectThemeAtNode(ns, themeW);
    const w = (dx ? twx : 1 - twx) * (dz ? twz : 1 - twz);
    for (let f = 0; f < 12; f++) spatial[f] += themes[tidx].sw[f] * w;
    blendedDensity += themes[tidx].density * w;
    nodes.push({ nx: tbx + dx, nz: tbz + dz, theme: tidx, w });
  }
  entityDensity *= blendedDensity;
  return { entityDensity, spatial, nodes };
}

/* ═══ THE COMPOSITION LAW — exact port of compose_spawn_chance ═══
   mood → global → tile(F3) → [proximity] → base × adj → clamp        */
function proximityBoost(fi, neighborCount, tables) {
  if (!FAMILIES[fi].prox) return { boost: 1, note: "law: proximity OFF" };
  const radius = tables.radius[fi];
  if (radius <= 0) return { boost: 1, note: "radius 0 — never scans" };
  if (neighborCount < tables.threshold[fi]) return { boost: 1, note: `below threshold (${neighborCount}<${tables.threshold[fi]})` };
  const boost = Math.min(1 + tables.selfAff[fi] * neighborCount, tables.maxBoost[fi]);
  return { boost, note: `min(1+${tables.selfAff[fi]}·${neighborCount}, ${tables.maxBoost[fi]})` };
}
function composeChance({ fi, mood, base, globalDensity, pop, neighborCount, proxTables }) {
  const fam = FAMILIES[fi];
  const rows = [];
  let adj = fam.mood[mood];
  rows.push({ name: `mood ×  (${MOOD_NAMES[mood]})`, v: fam.mood[mood], run: adj });
  const vetoed = (fam.clamp !== "MIN1") && adj <= 0; // bespoke funnels early-return
  adj *= globalDensity;
  rows.push({ name: "GLOBAL_ENTITY_DENSITY", v: globalDensity, run: adj, identity: globalDensity === 1 });
  adj *= pop.entityDensity;
  rows.push({ name: "tile entity_density (F3)", v: pop.entityDensity, run: adj, identity: Math.abs(pop.entityDensity - 1) < 1e-9 });
  adj *= pop.spatial[fi];
  rows.push({ name: "tile spatial_density[fam] (F3)", v: pop.spatial[fi], run: adj });
  const pr = proximityBoost(fi, neighborCount, proxTables);
  if (fam.prox) { adj *= pr.boost; }
  rows.push({ name: `proximity boost — ${pr.note}`, v: fam.prox ? pr.boost : 1, run: adj, off: !fam.prox });
  let chance = base * adj;
  rows.push({ name: `base × adj  (base ${base.toFixed(3)})`, v: base, run: chance });
  let clampNote = "";
  if (fam.clamp === "MIN1") { const c = Math.min(chance, 1); clampNote = "min(·,1)"; chance = c; }
  else if (fam.clamp === "RANGE01") { const c = Math.max(0, Math.min(1, chance)); clampNote = "max(0,min(1,·))"; chance = c; }
  else clampNote = "NONE (carried as data — gallery sub-ruling)";
  rows.push({ name: `clamp ${fam.clamp} — ${clampNote}`, v: null, run: chance });
  return { rows, chance, vetoed };
}

/* ═══ C++ EXPORT — the tables' own shapes ═══ */
function cppExport(state) {
  const { globalDensity, themes, density, baseOverrides } = state;
  const L = [];
  L.push("// ── 7T POPULATION DESIGNER EXPORT — paste-ready, table-shaped ──");
  L.push("// (values only; homes cited per block. Waking a dead knob is a");
  L.push("//  BEHAVIOR change — rig-gated per R3.)");
  L.push("");
  L.push("// contracts/spawn_services.hpp");
  L.push(`inline constexpr float GLOBAL_ENTITY_DENSITY = ${globalDensity.toFixed(3)}f;`);
  L.push("");
  L.push("// surface/population_themes.hpp — DENSITY band" + (density.min === density.max ? "  (still identity)" : "  (WOKEN: min ≠ max)"));
  L.push(`inline constexpr float DENSITY_EXPONENT = ${density.exponent.toFixed(2)}f;`);
  L.push(`inline constexpr float DENSITY_MIN = ${density.min.toFixed(2)}f;`);
  L.push(`inline constexpr float DENSITY_MAX = ${density.max.toFixed(2)}f;`);
  L.push("");
  L.push("// surface/population_themes.hpp — THEMES spawn_weight rows (PopFamily order)");
  themes.forEach((t) => {
    L.push(`/* ${t.name.padEnd(10)} */ { ${t.sw.map((v) => v.toFixed(1) + "f").join(", ")} },  // density_mult ${t.density.toFixed(1)}f`);
  });
  L.push("");
  L.push("// per-family SPAWN_CHANCE (each family's Config struct)");
  FAMILIES.forEach((f, i) => {
    if (f.base === null) return;
    const v = baseOverrides[i] ?? f.base;
    const changed = Math.abs(v - f.base) > 1e-9 ? "   // CHANGED" : "";
    L.push(`static constexpr float SPAWN_CHANCE = ${v.toFixed(3)}f;   // ${f.name}${changed}`);
  });
  L.push(`static constexpr float GALLERY_CHANCE_BY_ARCHETYPE[4] = { ${GALLERY_CHANCE_BY_ARCHETYPE.map((v) => v.toFixed(2) + "f").join(", ")} };`);
  return L.join("\n");
}

/* ═══ THE TOOL ═══ */
const MAP_W = 44, MAP_H = 28, CELL = 15;

export default function PopulationDesigner() {
  const [seed, setSeed] = useState(42);
  const [fi, setFi] = useState(2);              // COLUMN — the recon's worked example
  const [mood, setMood] = useState(0);
  const [tile, setTile] = useState({ gx: 8, gz: 7 });
  const [neighbors, setNeighbors] = useState(0);
  const [globalDensity, setGlobalDensity] = useState(GLOBAL_ENTITY_DENSITY_DEFAULT);
  const [density, setDensity] = useState({ ...DENSITY_DEFAULTS });
  const [themes, setThemes] = useState(THEME_DEFS.map((t) => ({ ...t, sw: [...t.sw] })));
  const [baseOverrides, setBaseOverrides] = useState({});
  const [mapMode, setMapMode] = useState("chance"); // chance | spatial | themes | passes
  const [showExport, setShowExport] = useState(false);
  const canvasRef = useRef(null);

  const fam = FAMILIES[fi];
  const base = fam.base === null ? null : (baseOverrides[fi] ?? fam.base);
  const densityCfg = density;

  /* selected-tile trace */
  const trace = useMemo(() => {
    const pop = tilePop(seed, tile.gx, tile.gz, themes, densityCfg);
    const b = fam.base === null
      ? GALLERY_CHANCE_BY_ARCHETYPE[1] /* archetype 'varied' default — map has no terrain */
      : base;
    const composed = composeChance({ fi, mood, base: b, globalDensity, pop, neighborCount: neighbors, proxTables: PROX });
    const ts = tileSeed(seed, tile.gx, tile.gz);
    const roll = hashF(ts, fam.roll);
    return { pop, composed, ts, roll, pass: !composed.vetoed && roll < composed.chance, baseUsed: b };
  }, [seed, tile, fi, mood, globalDensity, themes, densityCfg, neighbors, base, fam]);

  /* map field */
  const field = useMemo(() => {
    const cells = [];
    for (let gz = 0; gz < MAP_H; gz++) for (let gx = 0; gx < MAP_W; gx++) {
      const pop = tilePop(seed, gx, gz, themes, densityCfg);
      const b = fam.base === null ? GALLERY_CHANCE_BY_ARCHETYPE[1] : base;
      const { chance, vetoed } = composeChance({ fi, mood, base: b, globalDensity, pop, neighborCount: neighbors, proxTables: PROX });
      const roll = hashF(tileSeed(seed, gx, gz), fam.roll);
      cells.push({ gx, gz, pop, chance: vetoed ? 0 : chance, pass: !vetoed && roll < chance });
    }
    return cells;
  }, [seed, fi, mood, globalDensity, themes, densityCfg, neighbors, base, fam]);

  /* canvas paint */
  useEffect(() => {
    const cv = canvasRef.current; if (!cv) return;
    const ctx = cv.getContext("2d");
    ctx.fillStyle = "#141210"; ctx.fillRect(0, 0, cv.width, cv.height);
    const maxSpatial = 4.5;
    for (const c of field) {
      const x = c.gx * CELL, y = c.gz * CELL;
      if (mapMode === "chance") {
        const v = Math.min(c.chance / Math.max(0.001, (base ?? 0.06) * maxSpatial), 1);
        ctx.fillStyle = `rgba(${60 + 180 * v | 0},${90 + 120 * v | 0},${200 * v | 0},1)`;
      } else if (mapMode === "spatial") {
        const v = Math.min(c.pop.spatial[fi] / maxSpatial, 1);
        ctx.fillStyle = `rgba(${40 + 200 * v | 0},${60 + 140 * v | 0},${40 | 0},1)`;
      } else if (mapMode === "themes") {
        let best = 0, bw = -1;
        c.pop.nodes.forEach((n) => { if (n.w > bw) { bw = n.w; best = n.theme; } });
        ctx.fillStyle = THEME_DEFS[best].color + "cc";
      } else { // passes
        ctx.fillStyle = c.pass ? fam.color : "#1c1a18";
      }
      ctx.fillRect(x, y, CELL - 1, CELL - 1);
      if (mapMode === "passes" && c.pass) { ctx.fillStyle = "#fff"; ctx.fillRect(x + CELL / 2 - 1, y + CELL / 2 - 1, 2, 2); }
    }
    // theme lattice nodes
    ctx.lineWidth = 2;
    const drawn = new Set();
    for (const c of field) for (const n of c.pop.nodes) {
      const k = n.nx + ":" + n.nz; if (drawn.has(k)) continue; drawn.add(k);
      const px = (n.nx * THEME_LATTICE_SPACING) / PATCH_EXTENT * CELL, pz = (n.nz * THEME_LATTICE_SPACING) / PATCH_EXTENT * CELL;
      if (px < -CELL || pz < -CELL || px > cv.width + CELL || pz > cv.height + CELL) continue;
      ctx.strokeStyle = THEME_DEFS[n.theme].color;
      ctx.beginPath(); ctx.arc(px, pz, 6, 0, 6.2832); ctx.stroke();
    }
    // selected tile
    ctx.strokeStyle = "#fff"; ctx.lineWidth = 2;
    ctx.strokeRect(tile.gx * CELL - 1, tile.gz * CELL - 1, CELL + 1, CELL + 1);
  }, [field, mapMode, tile, fi, base, fam]);

  const onMapClick = useCallback((e) => {
    const r = canvasRef.current.getBoundingClientRect();
    const gx = Math.floor((e.clientX - r.left) / CELL), gz = Math.floor((e.clientY - r.top) / CELL);
    if (gx >= 0 && gx < MAP_W && gz >= 0 && gz < MAP_H) setTile({ gx, gz });
  }, []);

  const setThemeSw = (ti, v) => setThemes((th) => th.map((t, i) => i === ti ? { ...t, sw: t.sw.map((s, f) => f === fi ? v : s) } : t));
  const passCount = field.filter((c) => c.pass).length;

  return (
    <div style={S.root}>
      {/* ── header bar ── */}
      <div style={S.bar}>
        {FAMILIES.map((f, i) => (
          <button key={f.key} style={{ ...S.chip, background: i === fi ? f.color : "#26221e", color: i === fi ? "#111" : "#bbb" }}
            onClick={() => setFi(i)}>{f.name}</button>
        ))}
        <span style={S.sep} />
        <select style={S.select} value={mood} onChange={(e) => setMood(+e.target.value)}>
          {MOOD_NAMES.map((m, i) => <option key={m} value={i}>{m}</option>)}
        </select>
        <label style={S.lbl}>seed <input style={S.num} type="number" value={seed} onChange={(e) => setSeed(+e.target.value >>> 0)} /></label>
        <label style={S.lbl}>map
          <select style={S.select} value={mapMode} onChange={(e) => setMapMode(e.target.value)}>
            <option value="chance">chance</option><option value="spatial">spatial_density</option>
            <option value="themes">theme field</option><option value="passes">rolls that PASS</option>
          </select>
        </label>
        <button style={S.export} onClick={() => setShowExport(!showExport)}>{showExport ? "close export" : "C++ export"}</button>
      </div>

      <div style={S.cols}>
        {/* ── knobs ── */}
        <div style={S.knobs}>
          <div style={S.h}>BASE — {fam.name}</div>
          {fam.base !== null ? (
            <Knob label={`SPAWN_CHANCE  (authored ${fam.base})`} v={base} min={0} max={1} step={0.005}
              onChange={(v) => setBaseOverrides((o) => ({ ...o, [fi]: v }))} />
          ) : (
            <div style={S.note}>gallery: archetype-indexed base {"{"}{GALLERY_CHANCE_BY_ARCHETYPE.join(", ")}{"}"} ({ARCH_NAMES.join("/")}) — trace uses 'varied'</div>
          )}
          <div style={S.note}>mood row [{fam.mood.join(",")}] · clamp {fam.clamp} · proximity {fam.prox ? "ON" : "OFF"}{fam.seedDomain === "lattice" ? " · lattice-seed roll" : ""}</div>

          <div style={S.h}>MASTER</div>
          <Knob label={`GLOBAL_ENTITY_DENSITY ${globalDensity === 1 ? "(identity)" : "(WOKEN)"}`} v={globalDensity} min={0} max={3} step={0.05} onChange={setGlobalDensity} />

          <div style={S.h}>THEMES — spawn_weight[{fam.name}]</div>
          {themes.map((t, ti) => (
            <Knob key={t.name} label={t.name} v={t.sw[fi]} min={0} max={6} step={0.1} onChange={(v) => setThemeSw(ti, v)} accent={t.color} />
          ))}

          <div style={S.h}>DENSITY BAND {density.min === density.max ? "(dead — wake by min≠max)" : "(WOKEN — rig-gated, R3)"}</div>
          <Knob label="DENSITY_MIN" v={density.min} min={0} max={2} step={0.05} onChange={(v) => setDensity((d) => ({ ...d, min: v }))} />
          <Knob label="DENSITY_MAX" v={density.max} min={0} max={2} step={0.05} onChange={(v) => setDensity((d) => ({ ...d, max: v }))} />
          <Knob label="DENSITY_EXPONENT" v={density.exponent} min={0.1} max={2} step={0.05} onChange={(v) => setDensity((d) => ({ ...d, exponent: v }))} />

          <div style={S.h}>CLUSTER HYPOTHESIS</div>
          <Knob label={`neighbors within ${PROX.radius[fi] || "—"} wu`} v={neighbors} min={0} max={6} step={1} onChange={setNeighbors} />
          <div style={S.note}>post-roll (not in chance): slot cap · MIN_SEPARATION at PLACE · footprints 128</div>
        </div>

        {/* ── map ── */}
        <div style={S.mapWrap}>
          <canvas ref={canvasRef} width={MAP_W * CELL} height={MAP_H * CELL} style={S.canvas} onClick={onMapClick} />
          <div style={S.mapFoot}>
            {MAP_W}×{MAP_H} patches ({MAP_W * PATCH_EXTENT}×{MAP_H * PATCH_EXTENT} wu) · rings = theme lattice nodes ·
            {" "}{passCount} tiles PASS for {fam.name} ({(100 * passCount / (MAP_W * MAP_H)).toFixed(1)}%)
          </div>
        </div>

        {/* ── the live stack (the recon §2 table, alive) ── */}
        <div style={S.stack}>
          <div style={S.h}>THE STACK — {fam.name} @ ({tile.gx},{tile.gz}) · {MOOD_NAMES[mood]}</div>
          <table style={S.tbl}><tbody>
            {trace.composed.rows.map((r, i) => (
              <tr key={i} style={{ opacity: r.off ? 0.4 : 1 }}>
                <td style={S.tdN}>{r.name}{r.identity ? "  · identity" : ""}</td>
                <td style={S.tdV}>{r.v === null ? "" : (typeof r.v === "number" ? r.v.toFixed(3) : r.v)}</td>
                <td style={S.tdR}>{r.run.toFixed(4)}</td>
              </tr>
            ))}
          </tbody></table>
          <div style={S.verdict}>
            {trace.composed.vetoed
              ? <span style={{ color: "#e07050" }}>MOOD VETO (bespoke early-return)</span>
              : <>chance <b>{trace.composed.chance.toFixed(4)}</b> · roll(seed {trace.ts >>> 0}, prop {fam.roll}) = <b>{trace.roll.toFixed(4)}</b> →{" "}
                <span style={{ color: trace.pass ? "#7ac47a" : "#c47a7a", fontWeight: 700 }}>{trace.pass ? "SPAWN" : "no spawn"}</span></>}
          </div>
          <div style={S.note}>
            tile nodes: {trace.pop.nodes.map((n) => `${THEME_DEFS[n.theme].name}·w${n.w.toFixed(2)}`).join("  ")}<br />
            spatial_density[{fam.name}] = {trace.pop.spatial[fi].toFixed(3)} · entity_density = {trace.pop.entityDensity.toFixed(3)}
          </div>
        </div>
      </div>

      {showExport && (
        <textarea style={S.exportBox} readOnly value={cppExport({ globalDensity, themes, density, baseOverrides })} onFocus={(e) => e.target.select()} />
      )}
    </div>
  );
}

function Knob({ label, v, min, max, step, onChange, accent }) {
  return (
    <label style={S.knob}>
      <span style={{ ...S.knobLbl, color: accent || "#999" }}>{label}</span>
      <input type="range" min={min} max={max} step={step} value={v} onChange={(e) => onChange(+e.target.value)} style={S.range} />
      <span style={S.knobV}>{(+v).toFixed(step >= 1 ? 0 : 2)}</span>
    </label>
  );
}

const S = {
  root: { fontFamily: "'SF Mono', Menlo, monospace", background: "#1a1816", color: "#ccc", minHeight: "100%", padding: 10, fontSize: 12 },
  bar: { display: "flex", flexWrap: "wrap", gap: 4, alignItems: "center", marginBottom: 10 },
  chip: { border: "none", borderRadius: 3, padding: "4px 8px", cursor: "pointer", fontSize: 11, fontFamily: "inherit" },
  sep: { width: 12 },
  select: { background: "#26221e", color: "#ccc", border: "1px solid #3a3630", borderRadius: 3, padding: "3px 6px", fontFamily: "inherit", fontSize: 11 },
  lbl: { display: "flex", gap: 4, alignItems: "center", color: "#888" },
  num: { width: 80, background: "#26221e", color: "#ccc", border: "1px solid #3a3630", borderRadius: 3, padding: "3px 6px", fontFamily: "inherit" },
  export: { marginLeft: "auto", background: "#3a5a3a", color: "#cfc", border: "none", borderRadius: 3, padding: "4px 10px", cursor: "pointer", fontFamily: "inherit" },
  cols: { display: "flex", gap: 12, alignItems: "flex-start" },
  knobs: { width: 290, flexShrink: 0 },
  h: { color: "#d4a040", margin: "12px 0 6px", fontSize: 11, letterSpacing: 1 },
  knob: { display: "flex", alignItems: "center", gap: 6, margin: "3px 0" },
  knobLbl: { width: 150, fontSize: 10.5, overflow: "hidden", textOverflow: "ellipsis", whiteSpace: "nowrap" },
  range: { flex: 1 },
  knobV: { width: 40, textAlign: "right", color: "#eee" },
  note: { color: "#776f66", fontSize: 10.5, margin: "4px 0", lineHeight: 1.5 },
  mapWrap: { flexShrink: 0 },
  canvas: { border: "1px solid #3a3630", cursor: "crosshair", imageRendering: "pixelated" },
  mapFoot: { color: "#776f66", fontSize: 10.5, marginTop: 4, maxWidth: MAP_W * CELL },
  stack: { flex: 1, minWidth: 320 },
  tbl: { borderCollapse: "collapse", width: "100%" },
  tdN: { padding: "3px 6px", borderBottom: "1px solid #26221e", color: "#aaa" },
  tdV: { padding: "3px 6px", borderBottom: "1px solid #26221e", textAlign: "right", color: "#888", width: 60 },
  tdR: { padding: "3px 6px", borderBottom: "1px solid #26221e", textAlign: "right", color: "#e8d8b0", width: 70, fontWeight: 600 },
  verdict: { margin: "8px 0", padding: 8, background: "#211e1b", borderRadius: 4 },
  exportBox: { width: "100%", height: 260, marginTop: 12, background: "#141210", color: "#b8d8b8", border: "1px solid #3a3630", borderRadius: 4, padding: 10, fontFamily: "inherit", fontSize: 11, whiteSpace: "pre" },
};
