import { useState, useMemo, useRef, useEffect, useCallback } from "react";

/* ═══════════════════════════════════════════════════════════════════════
   7T TERRAIN COLOR DESIGNER — the color stack's write surface (STEP 2c).

   Authors the GRADUATED palette tables (STEP 2b: config.palette_center /
   palette_light / palette_weight — the medians a color voice moves) and
   previews the mode_* uniforms. Two-layer fidelity, declared:
   · the PALETTE LAYER is an EXACT port — hash_property, lattice seeds,
     palette_weights_at_node (roll 500u, dominant 0.85/0.05),
     palette_field_at (300-wu Hermite lattice), palette_color_smooth —
     the map shows the real spatial palette geography for the seed.
   · the COMPOSITE PREVIEW runs the exact composite_cell_color /
     discrete-tier color math, with the mode/style/sparse FIELD VALUES
     set by sliders (the lattices behind them are structural, not
     couplable — the engine keeps them; this panel authors colors).
   C++ export: set_palette_* calls + the boot-init REST block.
   Constants mirror the tree @ STEP 2b; the engine stays the authority.
   ═══════════════════════════════════════════════════════════════════════ */

/* ═══ EXACT PORTS — world.wgsl §1.5 + §2.2 ═══ */
function hashProp(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return (((h >>> 16) ^ h) >>> 0) / 0xFFFFFFFF;
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
const colorLatticeSeed = (worldSeed, nx, nz, band) => latticeNodeSeed(worldSeed, nx, nz, band + 100);
const smooth01 = (f) => f * f * (3 - 2 * f);

const PALETTE_LATTICE_SPACING = 300;
function paletteWeightsAtNode(worldSeed, nx, nz, weight) {
  const roll = hashProp(colorLatticeSeed(worldSeed, nx, nz, 0), 500);
  let dominant = 3, cumul = 0;
  for (let i = 0; i < 4; i++) { cumul += weight[i]; if (roll < cumul) { dominant = i; break; } }
  const w = [0.05, 0.05, 0.05, 0.05]; w[dominant] = 0.85;
  return w;
}
function paletteFieldAt(worldSeed, x, z, weight) {
  const gx = x / PALETTE_LATTICE_SPACING, gz = z / PALETTE_LATTICE_SPACING;
  const bx = Math.floor(gx), bz = Math.floor(gz);
  const wx = smooth01(gx - bx), wz = smooth01(gz - bz);
  const out = [0, 0, 0, 0];
  for (let dz = 0; dz <= 1; dz++) for (let dx = 0; dx <= 1; dx++) {
    const nw = paletteWeightsAtNode(worldSeed, bx + dx, bz + dz, weight);
    const t = (dx ? wx : 1 - wx) * (dz ? wz : 1 - wz);
    for (let i = 0; i < 4; i++) out[i] += nw[i] * t;
  }
  return out;
}
function paletteColorSmooth(w, complexity, centers, lights) {
  const c = [0, 0, 0];
  for (let i = 0; i < 4; i++)
    for (let k = 0; k < 3; k++)
      c[k] += (lights[i][k] + (centers[i][k] - lights[i][k]) * complexity) * w[i];
  return c;
}

/* exact discrete/composite color math (fields arrive as values) */
function discreteTierColor(cellSeed, gx, gz, region, monoTendency, chessTendency, tintStrength) {
  const chessJitter = (hashProp(cellSeed, 815) - 0.5) * 0.03;
  if (chessTendency + chessJitter > 0.45) {
    const parity = (gx + gz) & 1;
    if (chessTendency > 0.65) {
      const c = parity ? region.slice(0, 3) : region.map((v) => 1 - v).slice(0, 3);
      return c;
    }
    return parity ? [0.88, 0.88, 0.88] : [0.10, 0.10, 0.10];
  }
  const effectiveMono = monoTendency;
  const rgbMean = region.slice(0, 3);
  if (effectiveMono > 0.35) {
    const bw = hashProp(cellSeed, 830) > 0.5 ? 0.85 : 0.12;
    return [bw, bw, bw];
  }
  if (effectiveMono > 0.20) {
    const bw = hashProp(cellSeed, 830) > 0.5 ? 0.85 : 0.12;
    return rgbMean.map((m) => bw + (m - bw) * tintStrength);
  }
  const variance = region[3];
  return rgbMean.map((m, i) =>
    Math.max(0, Math.min(1, m + (hashProp(cellSeed, 840 + i) - 0.5) * 2 * variance)));
}
const MODE_DISCRETE_THRESHOLD = 0.70;
function compositeCellColor(smoothC, discreteC, mode, style, sparse, cellRoll, sparseRoll) {
  const sstep = (lo, hi, x) => { const t = Math.max(0, Math.min(1, (x - lo) / (hi - lo))); return t * t * (3 - 2 * t); };
  const blendT = sstep(MODE_DISCRETE_THRESHOLD - 0.15, MODE_DISCRETE_THRESHOLD + 0.05, mode);
  const blend = smoothC.map((s, i) => s + (discreteC[i] - s) * blendT);
  const survival = sstep(MODE_DISCRETE_THRESHOLD - 0.35, MODE_DISCRETE_THRESHOLD + 0.05, mode);
  const scatter = cellRoll < survival ? discreteC : smoothC;
  const modeColor = blend.map((b, i) => b + (scatter[i] - b) * style);
  const sparseSurvival = sstep(0.22, 0.57, sparse);
  const showCell = (sparseRoll < sparseSurvival) && !(mode > MODE_DISCRETE_THRESHOLD - 0.35);
  return showCell ? discreteC : modeColor;
}

/* ═══ REST VALUES (the 2b boot literals) ═══ */
const REST = {
  centers: [[0.85, 0.70, 0.50], [0.88, 0.58, 0.48], [0.45, 0.58, 0.38], [0.82, 0.55, 0.42]],
  lights:  [[0.92, 0.82, 0.65], [0.95, 0.72, 0.62], [0.62, 0.72, 0.52], [0.92, 0.72, 0.58]],
  weight:  [0.42, 0.28, 0.04, 0.26],
};
const PAL_NAMES = ["sand", "salmon", "green", "warm"];
const RETIRED_VARIANCE = [0.08, 0.14, 0.20, 0.12]; // design space (retired with palette_color, 2b)

const toHex = (c) => "#" + c.map((v) => Math.round(Math.max(0, Math.min(1, v)) * 255).toString(16).padStart(2, "0")).join("");
const fromHex = (h) => [1, 3, 5].map((i) => parseInt(h.slice(i, i + 2), 16) / 255);

function cppExport(st) {
  const L = [];
  L.push("// ── 7T TERRAIN COLOR DESIGNER EXPORT — the graduated palette mirror ──");
  L.push("// state.hpp initializeState boot block (or live set_palette_* calls).");
  L.push("{");
  L.push("    const float centers[4][3] = { " + st.centers.map((c) => `{${c.map((v) => v.toFixed(2) + "f").join(",")}}`).join(", ") + " };");
  L.push("    const float lights[4][3]  = { " + st.lights.map((c) => `{${c.map((v) => v.toFixed(2) + "f").join(",")}}`).join(", ") + " };");
  L.push("    for (uint32_t i = 0; i < 4; i++) {");
  L.push("        gpuState_.set_palette_center(i, centers[i][0], centers[i][1], centers[i][2]);");
  L.push("        gpuState_.set_palette_light(i,  lights[i][0],  lights[i][1],  lights[i][2]);");
  L.push("    }");
  L.push(`    gpuState_.set_palette_weight(${st.weight.map((w) => w.toFixed(2) + "f").join(", ")});  // sums ${st.weight.reduce((a, b) => a + b, 0).toFixed(2)}`);
  L.push("}");
  L.push("// mode_* preview values (drivers pinned at boot — a voice writes these):");
  L.push(`//   set_mode_color_shift(${st.modeShift.toFixed(2)}f); set_mode_checker_scatter(${st.checkerScatter.toFixed(2)}f);`);
  L.push("// (PALETTE_VARIANCE retired 2b; design-space values were {" + RETIRED_VARIANCE.join(", ") + "})");
  return L.join("\n");
}

/* ═══ THE TOOL ═══ */
const MAP_W = 420, MAP_H = 280, WU_PER_PX = 4;   // 1680×1120 wu window

export default function TerrainColorDesigner() {
  const [seed, setSeed] = useState(42);
  const [centers, setCenters] = useState(REST.centers.map((c) => [...c]));
  const [lights, setLights] = useState(REST.lights.map((c) => [...c]));
  const [weight, setWeight] = useState([...REST.weight]);
  const [complexity, setComplexity] = useState(0.5);   // the pinned literal, as a preview dial
  const [modeShift, setModeShift] = useState(0);       // mode_color_shift preview
  const [checkerScatter, setCheckerScatter] = useState(0);
  const [mode, setMode] = useState(0.4);               // slider-set field values (composite preview)
  const [style, setStyle] = useState(0.5);
  const [sparse, setSparse] = useState(0.1);
  const [tintStrength, setTintStrength] = useState(0.15);
  const [view, setView] = useState("smooth");          // smooth | composite
  const [showExport, setShowExport] = useState(false);
  const canvasRef = useRef(null);

  const wSum = weight.reduce((a, b) => a + b, 0);

  /* paint the map */
  useEffect(() => {
    const cv = canvasRef.current; if (!cv) return;
    const ctx = cv.getContext("2d");
    const img = ctx.createImageData(MAP_W, MAP_H);
    const effMode = Math.max(0, Math.min(1, mode + modeShift));
    const effSparseThresholdShift = checkerScatter; // preview of :7275 (threshold-lowering)
    for (let py = 0; py < MAP_H; py++) {
      for (let px = 0; px < MAP_W; px++) {
        const x = px * WU_PER_PX, z = py * WU_PER_PX;
        const w = paletteFieldAt(seed, x, z, weight);
        let c = paletteColorSmooth(w, complexity, centers, lights);
        if (view === "composite") {
          const cgx = Math.floor(x / 3.125), cgz = Math.floor(z / 3.125);
          const cellSeed = latticeNodeSeed(seed, cgx, cgz, 7);
          const region = [hashProp(cellSeed ^ 1, 800), hashProp(cellSeed ^ 1, 801), hashProp(cellSeed ^ 1, 802), 0.02 + hashProp(cellSeed ^ 1, 803) * 0.23];
          const disc = discreteTierColor(cellSeed, cgx, cgz, region, 0.15, 0.2, tintStrength);
          const cellRoll = hashProp(cellSeed, 555), sparseRoll = hashProp(cellSeed, 556);
          c = compositeCellColor(c, disc, effMode, style, Math.min(1, sparse + effSparseThresholdShift), cellRoll, sparseRoll);
        }
        const o = (py * MAP_W + px) * 4;
        img.data[o] = c[0] * 255; img.data[o + 1] = c[1] * 255; img.data[o + 2] = c[2] * 255; img.data[o + 3] = 255;
      }
    }
    ctx.putImageData(img, 0, 0);
  }, [seed, centers, lights, weight, complexity, view, mode, style, sparse, modeShift, checkerScatter, tintStrength]);

  const setC = useCallback((pi, which, hex) => {
    const rgb = fromHex(hex);
    (which === "center" ? setCenters : setLights)((arr) => arr.map((c, i) => (i === pi ? rgb : c)));
  }, []);

  return (
    <div style={S.root}>
      <div style={S.bar}>
        <label style={S.lbl}>seed <input style={S.num} type="number" value={seed} onChange={(e) => setSeed(+e.target.value >>> 0)} /></label>
        <label style={S.lbl}>view
          <select style={S.sel} value={view} onChange={(e) => setView(e.target.value)}>
            <option value="smooth">smooth (palette geography — EXACT)</option>
            <option value="composite">composite preview (fields by slider)</option>
          </select>
        </label>
        <button style={S.reset} onClick={() => { setCenters(REST.centers.map((c) => [...c])); setLights(REST.lights.map((c) => [...c])); setWeight([...REST.weight]); }}>rest values</button>
        <button style={S.export} onClick={() => setShowExport(!showExport)}>{showExport ? "close export" : "C++ export"}</button>
      </div>

      <div style={S.cols}>
        <div style={S.knobs}>
          <div style={S.h}>THE PALETTES — center is the couplable MEDIAN</div>
          {PAL_NAMES.map((name, i) => (
            <div key={name} style={S.palRow}>
              <span style={{ ...S.palName }}>{name}</span>
              <label style={S.swatchL}>μ<input type="color" value={toHex(centers[i])} onChange={(e) => setC(i, "center", e.target.value)} /></label>
              <label style={S.swatchL}>lt<input type="color" value={toHex(lights[i])} onChange={(e) => setC(i, "light", e.target.value)} /></label>
              <input style={S.range} type="range" min="0" max="1" step="0.01" value={weight[i]}
                onChange={(e) => setWeight((w) => w.map((v, k) => (k === i ? +e.target.value : v)))} />
              <span style={S.wv}>{weight[i].toFixed(2)}</span>
            </div>
          ))}
          <div style={{ ...S.note, color: Math.abs(wSum - 1) > 0.01 ? "#e0a050" : "#776f66" }}>
            weight Σ = {wSum.toFixed(2)} {Math.abs(wSum - 1) > 0.01 ? "— engine normalizes in the cumulative pick, but authored tables ship Σ=1" : "✓"}
          </div>

          <div style={S.h}>MIX + MODE (the driverless uniforms, previewed)</div>
          <Knob label={`complexity (pinned 0.5 today)`} v={complexity} set={setComplexity} />
          <Knob label="mode_color_shift (μ-shift)" v={modeShift} set={setModeShift} min={-0.5} max={0.5} />
          <Knob label="mode_checker_scatter" v={checkerScatter} set={setCheckerScatter} />
          <Knob label="tint_strength (pinned 0.15)" v={tintStrength} set={setTintStrength} />

          <div style={S.h}>COMPOSITE PREVIEW FIELDS (slider-set — the lattices stay the engine's)</div>
          <Knob label="mode field" v={mode} set={setMode} />
          <Knob label="style (blend↔scatter)" v={style} set={setStyle} />
          <Knob label="sparse" v={sparse} set={setSparse} />
        </div>

        <div>
          <canvas ref={canvasRef} width={MAP_W} height={MAP_H} style={S.canvas} />
          <div style={S.note}>
            {MAP_W * WU_PER_PX}×{MAP_H * WU_PER_PX} wu · smooth view = exact port (hash → lattice → dominant 0.85/0.05 → Hermite → mix(light, center, complexity)) ·
            composite view = exact color/composite math, field values from the sliders
          </div>
          <div style={S.swatches}>
            {PAL_NAMES.map((n, i) => (
              <div key={n} style={S.swCol}>
                <div style={{ ...S.sw, background: toHex(lights[i]) }} />
                <div style={{ ...S.sw, background: toHex(paletteColorSmooth([+(i===0),+(i===1),+(i===2),+(i===3)], complexity, centers, lights)) }} />
                <div style={{ ...S.sw, background: toHex(centers[i]) }} />
                <span style={S.swName}>{n}</span>
              </div>
            ))}
          </div>
        </div>
      </div>

      {showExport && (
        <textarea style={S.exportBox} readOnly onFocus={(e) => e.target.select()}
          value={cppExport({ centers, lights, weight, modeShift, checkerScatter })} />
      )}
    </div>
  );
}

function Knob({ label, v, set, min = 0, max = 1 }) {
  return (
    <label style={S.knob}>
      <span style={S.knobLbl}>{label}</span>
      <input style={S.range} type="range" min={min} max={max} step="0.01" value={v} onChange={(e) => set(+e.target.value)} />
      <span style={S.wv}>{(+v).toFixed(2)}</span>
    </label>
  );
}

const S = {
  root: { fontFamily: "'SF Mono', Menlo, monospace", background: "#1a1816", color: "#ccc", minHeight: "100%", padding: 10, fontSize: 12 },
  bar: { display: "flex", gap: 10, alignItems: "center", marginBottom: 10, flexWrap: "wrap" },
  lbl: { display: "flex", gap: 4, alignItems: "center", color: "#888" },
  num: { width: 80, background: "#26221e", color: "#ccc", border: "1px solid #3a3630", borderRadius: 3, padding: "3px 6px", fontFamily: "inherit" },
  sel: { background: "#26221e", color: "#ccc", border: "1px solid #3a3630", borderRadius: 3, padding: "3px 6px", fontFamily: "inherit", fontSize: 11 },
  reset: { background: "#3a3630", color: "#ccc", border: "none", borderRadius: 3, padding: "4px 10px", cursor: "pointer", fontFamily: "inherit" },
  export: { marginLeft: "auto", background: "#3a5a3a", color: "#cfc", border: "none", borderRadius: 3, padding: "4px 10px", cursor: "pointer", fontFamily: "inherit" },
  cols: { display: "flex", gap: 14, alignItems: "flex-start" },
  knobs: { width: 330, flexShrink: 0 },
  h: { color: "#d4a040", margin: "12px 0 6px", fontSize: 11, letterSpacing: 1 },
  palRow: { display: "flex", alignItems: "center", gap: 6, margin: "4px 0" },
  palName: { width: 52, color: "#aaa" },
  swatchL: { display: "flex", alignItems: "center", gap: 2, color: "#776f66", fontSize: 10 },
  range: { flex: 1 },
  wv: { width: 36, textAlign: "right", color: "#eee" },
  note: { color: "#776f66", fontSize: 10.5, margin: "6px 0", lineHeight: 1.5, maxWidth: MAP_W },
  knob: { display: "flex", alignItems: "center", gap: 6, margin: "3px 0" },
  knobLbl: { width: 170, fontSize: 10.5, color: "#999", overflow: "hidden", whiteSpace: "nowrap", textOverflow: "ellipsis" },
  canvas: { border: "1px solid #3a3630", imageRendering: "pixelated" },
  swatches: { display: "flex", gap: 10, marginTop: 8 },
  swCol: { display: "flex", flexDirection: "column", gap: 2, alignItems: "center" },
  sw: { width: 44, height: 18, borderRadius: 2, border: "1px solid #3a3630" },
  swName: { fontSize: 10, color: "#776f66" },
  exportBox: { width: "100%", height: 220, marginTop: 12, background: "#141210", color: "#b8d8b8", border: "1px solid #3a3630", borderRadius: 4, padding: 10, fontFamily: "inherit", fontSize: 11, whiteSpace: "pre" },
};
