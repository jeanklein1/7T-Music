import { useState, useEffect, useRef, useCallback } from "react";

/* ═══ HASH — exact port from cartridge.hpp ═══ */
function cpuH(seed, prop) {
  let h = (Math.imul(seed >>> 0, 747796405) + Math.imul(prop >>> 0, 2891336453) + 1) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return ((h >>> 16) ^ h) >>> 0;
}
function hf(s, p) { return cpuH(s, p) / 0xFFFFFFFF; }
function tseed(m, gx, gz) {
  let h = m >>> 0;
  h = (h ^ Math.imul((gx & 0xFFFFFFFF) >>> 0, 73856093)) >>> 0;
  h = (h ^ Math.imul((gz & 0xFFFFFFFF) >>> 0, 19349663)) >>> 0;
  h = Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769) >>> 0;
  return (Math.imul(((h >>> 16) ^ h) >>> 0, 2654435769)) >>> 0;
}
function gauss(seed, prop, mu, sig) {
  const u1 = Math.max(hf(seed, prop), 1e-6);
  const u2 = hf(seed, prop + 1000);
  const z = Math.max(-3, Math.min(3, Math.sqrt(-2 * Math.log(u1)) * Math.cos(2 * Math.PI * u2)));
  return mu + z * sig;
}
function selW(seed, prop, wts) {
  const roll = hf(seed, prop); let tot = 0; for (const w of wts) tot += w;
  let c = 0; for (let i = 0; i < wts.length; i++) { c += wts[i] / tot; if (roll < c) return i; }
  return wts.length - 1;
}

/* ═══ CONSTANTS ═══ */
const PE = 50;
const SPAWN_RADIUS = 7;
const RENDER_RADIUS = 5;
const EVICT_RADIUS = SPAWN_RADIUS + 2;

const TN = { p: ["Obelisk", "Temple", "Colossus"], a: ["Doorway", "Standard", "Monumental"], c: ["Pillar", "Doric", "Ornate"], n: ["Antenna", "Squat", "Colossal"] };
const TS = { p: [.35, .6, 1], a: [.1, .55, .95], c: [.15, .3, .5], n: [.6, .45, .85] };
const FN = ["Pyramid", "Arch", "Column", "Antenna"];
const FK = ["p", "a", "c", "n"];
const FC = ["#d4713b", "#4a90c4", "#6aaa5c", "#b07acc"];
const AC_COL = "#b07acc";
const THN = ["Transition", "Monumental", "Colonnade", "Antenna", "Barren"];
const THC = ["rgba(180,180,170,.18)", "rgba(200,140,80,.22)", "rgba(100,160,200,.22)", "rgba(160,200,100,.22)", "rgba(220,210,190,.15)"];
const AC = ["#c49a6c", "#b8b090", "#a0b8a0", "#8ab0c8"];
const AN = ["Mount", "Varied", "Basin", "Pool"];
const BASE_WEIGHT = 10;

const SPAWN_PROP = [800, 600, 700, 900];
const TIER_PROP  = [804, 604, 703, 903];
const POS_X_PROP = [801, 601, 701, 901];
const POS_Z_PROP = [802, 602, 702, 902];
const ROT_PROP   = [803, 603, 355, 355];
const FORM_PROP  = [360, 350, 356, 358];
const JITTER     = [0.25, 0.35, 0.35, 0.35];

/* ═══ FORMATION SLOT HELPER ═══ */
const EMPTY_SLOT = { ch: 0, di: 0, ds: 0, an: 0, as: 0, rm: 2, dr: 0, tm: 7 };
function getSlot(fr) { return fr.slots[String(fr.anchor)] || EMPTY_SLOT; }

function initState() {
  return {
    seed: 42,
    aw: [1.8, 1.3, 1, 0],
    sc: { p: 0.020, a: 0.020, c: 0.010, n: 0.020 },
    tw: { p: [.5, .25, .25], a: [.5, .15, .15], c: [.05, .2, .18], n: [.1, .22, .13] },
    sep: [[60, 50, 30, 50], [50, 100, 60, 60], [30, 100, 40, 60], [50, 60, 50, 80]],
    mute: [false, false, false, false],
    solo: -1,
    th: [
      // 0: TRANSITION
      { sp: [0.4, 0.3, 0.01, 0.7], tp: [1, 1, 1], ta: [1, 0.3, 1], tc: [0.1, 0.2, 0.3], tn: [0.1, 2, 0.7],
        spike: 150, sustain: 20, decay: 3, cooldown: 0,
        f: [{ anchor: -1, slots: {} },
            { anchor: -1, slots: {} },
            { anchor: -1, slots: { "-1": { ch: 0, di: 0, ds: 0, an: 0, as: 0, rm: 2, dr: 0, tm: 7 } } },
            { anchor: -1, slots: {} }] },
      // 1: MONUMENTAL
      { sp: [1.5, 1, 0.1, 1], tp: [.2, .5, 3], ta: [2, .1, 3], tc: [.01, .01, 1], tn: [.5, 1.5, .5],
        spike: 150, sustain: 10, decay: 10, cooldown: 8,
        f: [{ anchor: -1, slots: {} },
            { anchor: -1, slots: {
              "0": { ch: 0, di: 0, ds: 0, an: 0, as: 0, rm: 0, dr: 0, tm: 7 },
              "-1": { ch: .9, di: 100, ds: 0, an: 1.571, as: .5, rm: 2, dr: .01, tm: 7 } } },
            { anchor: 1, slots: { "1": { ch: .40, di: 25, ds: 5, an: 1.571, as: .30, rm: 0, dr: .05, tm: 7 } } },
            { anchor: -1, slots: {} }] },
      // 2: COLONNADE
      { sp: [1, .5, 10, .1], tp: [1, 1, 1], ta: [3, .5, 1], tc: [.3, 3, 5], tn: [.2, .1, .1],
        spike: 150, sustain: 15, decay: 6, cooldown: 6,
        f: [{ anchor: -1, slots: {} },
            { anchor: -1, slots: { "-1": { ch: .94, di: 150, ds: 50, an: 1.57, as: .1, rm: 0, dr: .10, tm: 7 } } },
            { anchor: 2, slots: {
              "1": { ch: 0, di: 100, ds: 3, an: 1.571, as: .15, rm: 0, dr: .05, tm: 7 },
              "-1": { ch: .98, di: 100, ds: 0, an: 2.094, as: .1, rm: 0, dr: 0, tm: 7 },
              "2": { ch: .97, di: 50, ds: 10, an: 1.57, as: 0, rm: 1, dr: 0, tm: 6 } } },
            { anchor: -1, slots: {} }] },
      // 3: ANTENNA
      { sp: [1, .5, .001, 4], tp: [1, .05, 2], ta: [1, .1, .1], tc: [.1, .3, .3], tn: [.5, 3.5, 1],
        spike: 180, sustain: 10, decay: 5, cooldown: 5,
        f: [{ anchor: -1, slots: {} },
            { anchor: -1, slots: {} },
            { anchor: -1, slots: {} },
            { anchor: -1, slots: { "-1": { ch: .95, di: 100, ds: 5, an: 0, as: .20, rm: 1, dr: .10, tm: 7 } } }] },
      // 4: BARREN
      { sp: [.4, .3, .3, .5], tp: [2, .5, .2], ta: [1, 1, 1], tc: [.2, .5, .5], tn: [1, 1, 1],
        spike: 100, sustain: 12, decay: 3, cooldown: 4,
        f: [{ anchor: -1, slots: {} },
            { anchor: -1, slots: {} },
            { anchor: -1, slots: {} },
            { anchor: -1, slots: {} }] },
    ],
  };
}

/* ═══ PERSISTENT WORLD ═══ */

function createWorld(P) {
  return {
    patches: new Map(),
    archetypes: new Map(),
    env: { active: -1, elapsed: 0, cooldowns: P.th.map(t => t.spike <= 0 ? t.cooldown : 0) },
    tips: [null, null, null, null],
    placed: [],
    queue: [],       // entities selected but not yet placed
    patchSeq: 0,
    entSeq: 0,
    initSeq: -1,
  };
}

function envelopeWeight(theme, elapsed) {
  if (elapsed < theme.sustain) return theme.spike;
  if (elapsed < theme.sustain + theme.decay) {
    const t = (elapsed - theme.sustain) / theme.decay;
    return theme.spike + (BASE_WEIGHT - theme.spike) * t;
  }
  return BASE_WEIGHT;
}

function getArchetype(world, P, gx, gz) {
  const key = `${gx},${gz}`;
  if (world.archetypes.has(key)) return world.archetypes.get(key);
  const sd = tseed(P.seed, gx, gz), roll = hf(sd, 300);
  const nc = [0, 0, 0, 0];
  for (let dz = -1; dz <= 1; dz++) for (let dx = -1; dx <= 1; dx++) {
    if (!dx && !dz) continue;
    const k = `${gx + dx},${gz + dz}`;
    if (world.archetypes.has(k)) nc[world.archetypes.get(k)]++;
  }
  const wts = P.aw.map((aw, i) => { let w = aw; const n = nc[i]; if (n >= 4) w *= .2; else if (n >= 2) w *= 1.5; else if (n >= 1) w *= 2; return w; });
  let tot = 0; for (const w of wts) tot += w; let cum = 0, ar = 3;
  for (let a = 0; a < 4; a++) { cum += wts[a] / tot; if (roll < cum) { ar = a; break; } }
  world.archetypes.set(key, ar);
  return ar;
}

function checkSep(world, P, px, pz, fam, placingRad = 0) {
  for (const p of world.placed) {
    const md = P.sep[fam][p.fam]; if (md <= 0) continue;
    const centerDist = Math.sqrt((px - p.x) ** 2 + (pz - p.z) ** 2);
    if (centerDist - (p.rad || 0) - placingRad < md) return false;
  }
  return true;
}

function spawnPatch(world, P, gx, gz) {
  const key = `${gx},${gz}`;
  if (world.patches.has(key)) return;
  for (let dz = -1; dz <= 1; dz++) for (let dx = -1; dx <= 1; dx++)
    getArchetype(world, P, gx + dx, gz + dz);
  const ar = getArchetype(world, P, gx, gz);
  const sd = tseed(P.seed, gx, gz);

  // ── Theme envelope (unchanged) ──
  let thIdx;
  if (P.solo >= 0) {
    thIdx = P.solo;
  } else {
    const env = world.env;
    const w = P.th.map((th, i) => {
      if (env.cooldowns[i] > 0) return 0;
      if (i === env.active) return envelopeWeight(th, env.elapsed);
      return BASE_WEIGHT;
    });
    let wTot = 0; for (const v of w) wTot += v;
    if (wTot < 0.001) wTot = 1;
    const thRoll = hf(sd, 370);
    thIdx = P.th.length - 1; let thCum = 0;
    for (let i = 0; i < P.th.length; i++) { thCum += w[i] / wTot; if (thRoll < thCum) { thIdx = i; break; } }

    if (thIdx !== env.active) {
      if (env.active >= 0) env.cooldowns[env.active] = P.th[env.active].cooldown;
      env.active = thIdx; env.elapsed = 0;
      // New theme — clear formation tips so formations don't bridge across themes
      world.tips = [null, null, null, null];
    } else { env.elapsed++; }
    if (env.active >= 0) {
      const th = P.th[env.active];
      if (env.elapsed >= th.sustain + th.decay) {
        env.cooldowns[env.active] = th.cooldown;
        env.active = -1; env.elapsed = 0;
      }
    }
    for (let i = 0; i < env.cooldowns.length; i++) if (env.cooldowns[i] > 0) env.cooldowns[i]--;
  }

  const theme = P.th[thIdx];

  // ── Selection: decide what exists, push to queue ──
  for (let fam = 0; fam < 4; fam++) {
    if (P.mute[fam]) continue;
    const chance = Math.min(P.sc[FK[fam]] * theme.sp[fam], 1);
    if (hf(sd, SPAWN_PROP[fam]) >= chance) continue;

    const bw = [...P.tw[FK[fam]]];
    const tb = [theme.tp, theme.ta, theme.tc, theme.tn][fam];
    for (let i = 0; i < bw.length; i++) bw[i] *= (tb[i] || 1);
    const tier = selW(sd, TIER_PROP[fam], bw);

    world.queue.push({ fam, tier, thm: thIdx, sd, gx, gz });
  }

  // Create patch shell (entities added during drainQueue)
  world.patches.set(key, { gx, gz, ar, ti: thIdx, ents: [], forms: [], seq: world.patchSeq++ });
}

// ═══════════════════════════════════════════════════════════════════
// DRAIN QUEUE — placement phase
//
// Processes all queued entities. For each:
//   1. Check formation rule from its theme
//   2. If formation active + anchor tip exists → step from tip
//   3. If formed position fails separation → fall back to jitter
//   4. If no formation → jitter position
//   5. Separation check → commit or discard
//   6. Assign entity to the patch it physically occupies
//   7. Update tip for this family
// ═══════════════════════════════════════════════════════════════════

function drainQueue(world, P) {
  if (world.queue.length === 0) return;

  // Drain in selection order (FIFO).
  // Entities were selected in distance-sorted patch order.
  // Each entity steps from the tip of its anchor family.
  // The chain builds naturally — no family priority imposed.
  for (const q of world.queue) {
    const { fam, tier, thm, sd, gx, gz } = q;
    const theme = P.th[thm];
    const fr = theme.f[fam];
    const slot = fr.slots[String(fr.anchor)];

    // Jitter fallback position (on originating patch)
    let cx = (gx + .5) * PE + (hf(sd, POS_X_PROP[fam]) - .5) * PE * JITTER[fam];
    let cz = (gz + .5) * PE + (hf(sd, POS_Z_PROP[fam]) - .5) * PE * JITTER[fam];
    let rot = hf(sd, ROT_PROP[fam]) * Math.PI * 2;
    let formed = false;
    let formLine = null;
    const sc = TS[FK[fam]][tier] || .5;
    const placingRad = sc * 12;

    // Formation attempt — only if slot exists, ch > 0, and tier passes mask
    const tierBit = (1 << tier);
    if (slot && slot.ch > 0 && (tierBit & (slot.tm ?? 7)) && hf(sd, FORM_PROP[fam]) < slot.ch) {
      const anchorFam = fr.anchor >= 0 ? fr.anchor : fam;
      const tip = world.tips[anchorFam];
      if (tip) {
        const di = (tip.rad || 0) + Math.max(0, gauss(sd, 340, slot.di, slot.ds));
        const ang = tip.rot + slot.an + gauss(sd, 342, 0, slot.as);
        const fx = tip.x + Math.cos(ang) * di;
        const fz = tip.z + Math.sin(ang) * di;
        let frot = rot;
        if (slot.rm === 0) frot = tip.rot + gauss(sd, 344, 0, slot.dr);
        else if (slot.rm === 1) frot = Math.atan2(fz - tip.z, fx - tip.x) + gauss(sd, 344, 0, slot.dr);
        if (checkSep(world, P, fx, fz, fam, placingRad)) {
          formLine = { x1: tip.x, z1: tip.z, x2: fx, z2: fz, fam, sibFam: anchorFam };
          cx = fx; cz = fz; rot = frot; formed = true;
        }
      }
    }

    // Separation check on fallback position
    if (!formed && !checkSep(world, P, cx, cz, fam, placingRad)) continue;

    // ── Commit ──
    const ent = { x: cx, z: cz, rot, fam, ti: tier,
      tn: TN[FK[fam]][tier], sc,
      thm, formed, seq: world.entSeq++ };
    world.placed.push({ x: cx, z: cz, fam, rad: sc * 12 });

    // Update tip only if this entity could participate in formation.
    // Ineligible tiers spawning at random positions must not break the chain.
    const tierEligible = !slot || !slot.ch || ((1 << tier) & (slot.tm ?? 7));
    if (tierEligible) {
      world.tips[fam] = { x: cx, z: cz, rot, rad: sc * 12 };
    }

    // Assign to host patch (where entity physically sits)
    const hostGx = Math.floor(cx / PE);
    const hostGz = Math.floor(cz / PE);
    const host = world.patches.get(`${hostGx},${hostGz}`);
    if (host) {
      host.ents.push(ent);
      if (formLine) host.forms.push(formLine);
    } else {
      // No host patch — assign to originating patch
      const origin = world.patches.get(`${gx},${gz}`);
      if (origin) {
        origin.ents.push(ent);
        if (formLine) origin.forms.push(formLine);
      }
    }
  }

  world.queue.length = 0;
}

function evictBeyond(world, cx, cz) {
  for (const [key, p] of [...world.patches]) {
    if (Math.abs(p.gx - cx) > EVICT_RADIUS || Math.abs(p.gz - cz) > EVICT_RADIUS) {
      for (const e of p.ents) {
        // Entity's physical grid cell — where it actually sits
        const eGx = Math.floor(e.x / PE);
        const eGz = Math.floor(e.z / PE);
        const hostKey = `${eGx},${eGz}`;
        const hostPatch = world.patches.get(hostKey);
        if (hostPatch && hostKey !== key) {
          // Entity physically belongs to a different surviving patch — migrate
          hostPatch.ents.push(e);
        } else {
          // Entity is genuinely on the dying patch — remove from separation list
          const idx = world.placed.findIndex(pl => pl.x === e.x && pl.z === e.z && pl.fam === e.fam);
          if (idx >= 0) world.placed.splice(idx, 1);
        }
      }
      // Formation lines are visual-only — no need to migrate
      world.patches.delete(key);
    }
  }
  for (const key of [...world.archetypes.keys()]) {
    const [gx, gz] = key.split(",").map(Number);
    if (Math.abs(gx - cx) > EVICT_RADIUS + 2 || Math.abs(gz - cz) > EVICT_RADIUS + 2)
      world.archetypes.delete(key);
  }
}

function updateWorld(world, P, px, pz) {
  evictBeyond(world, px, pz);

  // Collect new patches within spawn radius, sorted by distance from pawn
  // (closest first) — matches C++ spawn scan order
  const pending = [];
  for (let gz = pz - SPAWN_RADIUS; gz <= pz + SPAWN_RADIUS; gz++)
    for (let gx = px - SPAWN_RADIUS; gx <= px + SPAWN_RADIUS; gx++)
      if (!world.patches.has(`${gx},${gz}`))
        pending.push({ gx, gz, d2: (gx - px) * (gx - px) + (gz - pz) * (gz - pz) });

  pending.sort((a, b) => a.d2 - b.d2);
  for (const p of pending) spawnPatch(world, P, p.gx, p.gz);

  // All selections complete — now place entities from queue
  drainQueue(world, P);

  if (world.initSeq < 0) world.initSeq = world.patchSeq;
}

function collectVisible(world, px, pz) {
  const tiles = [], ents = [], forms = [];
  let minSeq = Infinity, maxSeq = 0;
  for (let gz = pz - SPAWN_RADIUS; gz <= pz + SPAWN_RADIUS; gz++)
    for (let gx = px - SPAWN_RADIUS; gx <= px + SPAWN_RADIUS; gx++) {
      const inRender = Math.abs(gx - px) <= RENDER_RADIUS && Math.abs(gz - pz) <= RENDER_RADIUS;
      const p = world.patches.get(`${gx},${gz}`);
      if (p) {
        tiles.push({ ...p, pregen: !inRender });
        if (p.seq < minSeq) minSeq = p.seq; if (p.seq > maxSeq) maxSeq = p.seq;
        for (const e of p.ents) ents.push({ ...e, pregen: !inRender });
        for (const f of p.forms) forms.push(f);
      } else {
        const ar = world.archetypes.get(`${gx},${gz}`) ?? 1;
        tiles.push({ gx, gz, ar, ti: -1, ents: [], forms: [], seq: 0, pregen: !inRender });
      }
    }
  return { tiles, ents, forms, pawnGx: px, pawnGz: pz, minSeq, maxSeq, initSeq: world.initSeq };
}

/* ═══ UI ═══ */
const ist = { padding: "2px 3px", fontSize: 11, fontFamily: "monospace", borderRadius: 4, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)", textAlign: "right" };

function Num({ value, onChange, min = 0, max = 10, step = 0.01, w = 46 }) {
  const [txt, setTxt] = useState(String(Math.round(value * 10000) / 10000));
  const [focused, setFocused] = useState(false);
  useEffect(() => { setTxt(String(Math.round(value * 10000) / 10000)); }, [value]);
  const commit = () => { const v = parseFloat(txt); if (!isNaN(v)) onChange(Math.max(min, Math.min(max, v))); else setTxt(String(Math.round(value * 10000) / 10000)); setFocused(false); };
  return (
    <span style={{ position: "relative", display: "inline-block" }}>
      <input type="text" value={txt} title={`${min} - ${max}`}
        onChange={e => setTxt(e.target.value)}
        onFocus={() => setFocused(true)} onBlur={commit}
        onKeyDown={e => { if (e.key === "Enter") e.target.blur(); }}
        style={{ ...ist, width: w, outline: focused ? "1.5px solid var(--color-border-info)" : "none" }} />
      {focused && <span style={{ position: "absolute", left: 0, top: "100%", marginTop: 2, fontSize: 9, fontFamily: "monospace", color: "var(--color-text-tertiary)", whiteSpace: "nowrap", pointerEvents: "none", zIndex: 10, background: "var(--color-background-primary)", padding: "1px 3px", borderRadius: 3, border: "1px solid var(--color-border-tertiary)" }}>{min} - {max}</span>}
    </span>
  );
}
function SliderNum({ value, onChange, min = 0, max = 100, step = 1, sw = 70, nw = 44, label }) {
  return (<div style={{ display: "flex", alignItems: "center", gap: 4 }}>
    {label && <span style={{ fontSize: 10, color: "var(--color-text-secondary)", minWidth: 36 }}>{label}</span>}
    <input type="range" min={min} max={max} step={step} value={value} style={{ width: sw }} onChange={e => onChange(+e.target.value)} />
    <Num value={value} min={min} max={max} step={step} w={nw} onChange={onChange} />
  </div>);
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
    const onMove = e => { const parent = dragRef.current.parentElement.getBoundingClientRect(); setPos({ x: e.clientX - parent.left - offsetRef.current.x, y: e.clientY - parent.top - offsetRef.current.y }); };
    const onUp = () => setDragging(false);
    window.addEventListener("mousemove", onMove); window.addEventListener("mouseup", onUp);
    return () => { window.removeEventListener("mousemove", onMove); window.removeEventListener("mouseup", onUp); };
  }, [dragging]);
  const doDock = () => { setPos(null); if (onDock) onDock(id); };
  const style = pos ? { position: "absolute", left: pos.x, top: pos.y, zIndex: 100, maxWidth: 420, minWidth: 280 } : {};
  return (
    <div ref={dragRef} style={{ marginBottom: pos ? 0 : 5, border: pos ? "1px solid rgba(0,0,0,.2)" : "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "var(--color-background-primary)", boxShadow: pos ? "0 4px 20px rgba(0,0,0,.3), 0 1px 4px rgba(0,0,0,.15)" : "none", ...style }}>
      <div onMouseDown={startDrag} style={{ padding: "5px 10px", fontSize: 12, fontWeight: 500, userSelect: "none", display: "flex", alignItems: "center", gap: 5, color: "var(--color-text-primary)", background: "var(--color-background-secondary)", cursor: "grab" }}>
        <span data-notdrag="1" onClick={() => setOpen(!open)} style={{ cursor: "pointer", fontSize: 9, transition: "transform .15s", display: "inline-block", transform: open ? "rotate(90deg)" : "none", padding: "4px 2px" }}>&#9654;</span>
        <span style={{ flex: 1 }}>{title}</span>
        {pos && <span data-notdrag="1" onClick={doDock} style={{ fontSize: 9, cursor: "pointer", opacity: .5, padding: "2px 4px" }}>dock</span>}
      </div>
      {open && <div style={{ padding: "5px 10px", maxHeight: pos ? 400 : "none", overflowY: pos ? "auto" : "visible" }}>{children}</div>}
    </div>
  );
}

const rw = { display: "flex", flexWrap: "wrap", gap: "3px 8px", alignItems: "center", marginBottom: 3 };
const lb = { fontSize: 10, color: "var(--color-text-secondary)", minWidth: 36 };
const RM = ["Inherit", "Follow", "Indep."];
const thd = { padding: "1px 3px", fontWeight: 500, color: "var(--color-text-secondary)", textAlign: "right", borderBottom: "1px solid var(--color-border-tertiary)", fontSize: 9 };
const tdd = { padding: "1px 2px", textAlign: "right", borderBottom: "1px solid var(--color-border-tertiary)" };
const tdl = { padding: "1px 4px", color: "var(--color-text-secondary)", fontSize: 10, borderBottom: "1px solid var(--color-border-tertiary)" };

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

const STORAGE_KEY = "7t:theme:state";
const DEFAULTS_JSON = JSON.stringify(initState());

export default function App() {
  const [P, setP] = useState(initState);
  const [loaded, setLoaded] = useState(false);
  const isModified = JSON.stringify(P) !== DEFAULTS_JSON;
  const [vis, setVis] = useState({ th: true, fo: true, gr: true, age: true, clean: false, nums: false });
  const [view, setView] = useState(null);
  const [hov, setHov] = useState(null);
  const [selTh, setSelTh] = useState(0);
  const [dockKey, setDockKey] = useState(0);
  const [pawnPos, setPawnPos] = useState([0, 0]);
  const [showDump, setShowDump] = useState(false);
  const cvRef = useRef(null);
  const worldRef = useRef(null);
  const upd = fn => setP(prev => { const n = JSON.parse(JSON.stringify(prev)); fn(n); return n; });
  const defaultOrder = ["theme", "arch", "spawn", "tier", "sep"];
  const [panelOrder, setPanelOrder] = useState(defaultOrder);

  // Load saved state on mount
  useEffect(() => {
    (async () => {
      try {
        const raw = await store.get(STORAGE_KEY);
        if (raw) { const data = JSON.parse(raw); if (data && typeof data === "object" && data.th) setP(data); }
      } catch (e) {}
      setLoaded(true);
    })();
  }, []);

  // Auto-save on every edit after initial load
  useEffect(() => {
    if (!loaded) return;
    (async () => { try { await store.set(STORAGE_KEY, JSON.stringify(P)); } catch (e) {} })();
  }, [P, loaded]);

  // Full reset on parameter change
  useEffect(() => {
    worldRef.current = createWorld(P);
    updateWorld(worldRef.current, P, pawnPos[0], pawnPos[1]);
    setView(collectVisible(worldRef.current, pawnPos[0], pawnPos[1]));
  }, [P]); // eslint-disable-line

  // Incremental update on pawn movement
  useEffect(() => {
    if (!worldRef.current) return;
    updateWorld(worldRef.current, P, pawnPos[0], pawnPos[1]);
    setView(collectVisible(worldRef.current, pawnPos[0], pawnPos[1]));
  }, [pawnPos]); // eslint-disable-line

  useEffect(() => {
    const onKey = e => {
      if (e.target.tagName === "INPUT" || e.target.tagName === "SELECT" || e.target.tagName === "TEXTAREA") return;
      const map = { ArrowUp: [0, -1], ArrowDown: [0, 1], ArrowLeft: [-1, 0], ArrowRight: [1, 0] };
      const d = map[e.key];
      if (d) { e.preventDefault(); setPawnPos(p => [p[0] + d[0], p[1] + d[1]]); }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  useEffect(() => {
    if (!view || !cvRef.current) return;
    const cv = cvRef.current, ctx = cv.getContext("2d");
    const dpr = window.devicePixelRatio || 1;
    const W = cv.clientWidth, H = cv.clientHeight;
    cv.width = W * dpr; cv.height = H * dpr; ctx.scale(dpr, dpr);

    const viewSide = (SPAWN_RADIUS * 2 + 1) * PE;
    const mg = 8;
    const sc = Math.min((W - mg * 2) / viewSide, (H - mg * 2) / viewSide);
    const cenX = (view.pawnGx + .5) * PE, cenZ = (view.pawnGz + .5) * PE;
    const ox = W / 2 - cenX * sc, oz = H / 2 - cenZ * sc;
    const tp = (x, z) => [ox + x * sc, oz + z * sc];

    ctx.fillStyle = getComputedStyle(document.documentElement).getPropertyValue("--color-background-primary") || "#fff";
    ctx.fillRect(0, 0, W, H);
    const pw = PE * sc;

    // Step-based age: all patches from the same arrow press share one shade
    const seqRange = view.maxSeq - view.minSeq;
    const hasWalked = view.maxSeq > view.initSeq;

    for (const t of view.tiles) {
      const [sx, sz] = tp(t.gx * PE, t.gz * PE);
      if (vis.clean) {
        ctx.fillStyle = "#d5cfc5"; ctx.fillRect(sx, sz, pw, pw);
      } else {
        ctx.fillStyle = AC[t.ar]; ctx.fillRect(sx, sz, pw, pw);
        if (vis.th && t.ti >= 0) { ctx.fillStyle = THC[t.ti]; ctx.fillRect(sx, sz, pw, pw); }
      }
      if (vis.age) {
        const freshness = seqRange > 0 ? (t.seq - view.minSeq) / seqRange : 0.5;
        // Full range: oldest = black, newest = white
        const grey = Math.round(freshness * 255);
        ctx.fillStyle = `rgb(${grey},${grey},${grey})`;
        ctx.fillRect(sx, sz, pw, pw);

        if (t.pregen) {
          ctx.fillStyle = "rgba(0,0,0,0.4)";
          ctx.fillRect(sx, sz, pw, pw);
        }

        if (vis.nums) {
          const displayNum = view.maxSeq - t.seq + 1;
          // Dark text on bright tiles, bright text on dark tiles
          const textGrey = freshness > 0.5 ? 0 : 255;
          ctx.fillStyle = `rgba(${textGrey},${textGrey},${textGrey},0.6)`;
          ctx.font = `bold ${Math.max(7, pw * 0.2)}px monospace`;
          ctx.textAlign = "center"; ctx.textBaseline = "middle";
          ctx.fillText(displayNum, sx + pw / 2, sz + pw / 2);
        }
      } else if (t.pregen) {
        ctx.fillStyle = "rgba(0,0,0,0.45)";
        ctx.fillRect(sx, sz, pw, pw);
      }
      if (vis.gr) { ctx.strokeStyle = t.pregen ? "rgba(0,0,0,.12)" : "rgba(0,0,0,.07)"; ctx.lineWidth = .5; ctx.strokeRect(sx, sz, pw, pw); }
    }

    // Render radius boundary
    {
      const rMinGx = view.pawnGx - RENDER_RADIUS, rMaxGx = view.pawnGx + RENDER_RADIUS;
      const rMinGz = view.pawnGz - RENDER_RADIUS, rMaxGz = view.pawnGz + RENDER_RADIUS;
      const [bx0, bz0] = tp(rMinGx * PE, rMinGz * PE);
      const [bx1, bz1] = tp((rMaxGx + 1) * PE, (rMaxGz + 1) * PE);
      ctx.strokeStyle = "rgba(255,255,255,0.5)"; ctx.lineWidth = 2; ctx.setLineDash([]);
      ctx.strokeRect(bx0, bz0, bx1 - bx0, bz1 - bz0);
    }

    // Init zone boundary
    if (vis.age && hasWalked) {
      ctx.strokeStyle = "rgba(80,140,220,0.5)"; ctx.lineWidth = 2; ctx.setLineDash([4, 3]);
      for (const t of view.tiles) {
        if (t.seq >= view.initSeq || t.pregen) continue;
        const [sx, sz] = tp(t.gx * PE, t.gz * PE);
        const check = (dgx, dgz) => {
          const nb = view.tiles.find(n => n.gx === t.gx + dgx && n.gz === t.gz + dgz);
          return !nb || nb.seq >= view.initSeq;
        };
        if (check(0, -1)) { ctx.beginPath(); ctx.moveTo(sx, sz); ctx.lineTo(sx + pw, sz); ctx.stroke(); }
        if (check(0, 1))  { ctx.beginPath(); ctx.moveTo(sx, sz + pw); ctx.lineTo(sx + pw, sz + pw); ctx.stroke(); }
        if (check(-1, 0)) { ctx.beginPath(); ctx.moveTo(sx, sz); ctx.lineTo(sx, sz + pw); ctx.stroke(); }
        if (check(1, 0))  { ctx.beginPath(); ctx.moveTo(sx + pw, sz); ctx.lineTo(sx + pw, sz + pw); ctx.stroke(); }
      }
      ctx.setLineDash([]);
    }

    if (vis.fo) {
      ctx.lineCap = "round";
      for (const f of view.forms) {
        const [x1, z1] = tp(f.x1, f.z1), [x2, z2] = tp(f.x2, f.z2);
        const cross = f.fam !== f.sibFam;
        ctx.setLineDash(cross ? [] : [5, 3]);
        ctx.lineWidth = cross ? 2 : 2.5;
        ctx.strokeStyle = FC[f.fam] + (cross ? "cc" : "99");
        ctx.beginPath(); ctx.moveTo(x1, z1); ctx.lineTo(x2, z2); ctx.stroke();
        if (cross) { ctx.fillStyle = FC[f.sibFam]; ctx.beginPath(); ctx.arc(x1, z1, 3, 0, Math.PI * 2); ctx.fill(); }
      }
      ctx.setLineDash([]); ctx.lineCap = "butt";
    }

    for (const e of view.ents) {
      const [ex, ez] = tp(e.x, e.z), sz = Math.max((3 + e.sc * 7) * sc * 3, 1.5);
      ctx.save(); ctx.translate(ex, ez); ctx.rotate(-e.rot);
      if (e.pregen) {
        ctx.globalAlpha = 0.25;
      } else if (vis.age) {
        const freshness = seqRange > 0 ? (e.seq - view.minSeq) / seqRange : 0.5;
        ctx.globalAlpha = 0.2 + freshness * 0.8;
      }
      if (e.fam === 0) { ctx.fillStyle = FC[0]; ctx.beginPath(); ctx.moveTo(0, -sz); ctx.lineTo(-sz * .7, sz * .6); ctx.lineTo(sz * .7, sz * .6); ctx.closePath(); ctx.fill(); }
      else if (e.fam === 1) { ctx.strokeStyle = FC[1]; ctx.lineWidth = Math.max(1.5, sz * .25); ctx.lineCap = "round"; ctx.beginPath(); ctx.arc(0, 0, sz * .8, Math.PI, 0); ctx.stroke(); }
      else { const cc = FC[e.fam]; ctx.fillStyle = cc; ctx.beginPath(); ctx.arc(0, 0, sz * .45, 0, Math.PI * 2); ctx.fill(); ctx.strokeStyle = cc; ctx.lineWidth = 1; ctx.beginPath(); ctx.moveTo(0, 0); ctx.lineTo(0, -sz * .6); ctx.stroke(); }
      ctx.restore();
    }

    {
      const [px, pz] = tp((pawnPos[0] + .5) * PE, (pawnPos[1] + .5) * PE);
      ctx.fillStyle = "#e03030"; ctx.beginPath(); ctx.arc(px, pz, 5, 0, Math.PI * 2); ctx.fill();
      ctx.strokeStyle = "#fff"; ctx.lineWidth = 1.5; ctx.beginPath(); ctx.arc(px, pz, 5, 0, Math.PI * 2); ctx.stroke();
      ctx.strokeStyle = "#e03030"; ctx.lineWidth = 1;
      ctx.beginPath(); ctx.moveTo(px - 8, pz); ctx.lineTo(px + 8, pz); ctx.moveTo(px, pz - 8); ctx.lineTo(px, pz + 8); ctx.stroke();
    }
  }, [view, vis, pawnPos]);

  const onMove = useCallback(e => {
    if (!view || !cvRef.current) return;
    const rect = cvRef.current.getBoundingClientRect();
    const W = rect.width, H = rect.height;
    const viewSide = (SPAWN_RADIUS * 2 + 1) * PE, mg = 8;
    const sc = Math.min((W - mg * 2) / viewSide, (H - mg * 2) / viewSide);
    const ox2 = W / 2 - (view.pawnGx + .5) * PE * sc;
    const oz2 = H / 2 - (view.pawnGz + .5) * PE * sc;
    const wx = (e.clientX - rect.left - ox2) / sc, wz = (e.clientY - rect.top - oz2) / sc;
    let best = null, bd = 300;
    for (const en of view.ents) { const d = (en.x - wx) ** 2 + (en.z - wz) ** 2; if (d < bd) { bd = d; best = en; } }
    setHov(best);
  }, [view, pawnPos]);

  const ct = P.th[selTh];
  const stats = view ? {
    t: view.ents.length, p: view.ents.filter(e => e.fam === 0).length,
    a: view.ents.filter(e => e.fam === 1).length, c: view.ents.filter(e => e.fam === 2).length,
    ant: view.ents.filter(e => e.fam === 3).length,
    formed: view.ents.filter(e => e.formed).length, patches: view.tiles.length,
  } : null;

  if (!loaded) return <div style={{ padding: 20, fontFamily: "monospace", fontSize: 11, color: "var(--color-text-tertiary)" }}>Loading…</div>;

  return (
    <div style={{ fontFamily: "var(--font-sans)", color: "var(--color-text-primary)", lineHeight: 1.4, position: "relative" }}>
      <div style={rw}>
        <SliderNum label="Seed" value={P.seed} min={0} max={99999} step={1} sw={60} nw={54} onChange={v => upd(n => { n.seed = v | 0; })} />
        {[["th", "Themes"], ["fo", "Form."], ["gr", "Grid"], ["age", "Age"], ["clean", "Clean"], ["nums", "#"]].map(([k, l]) =>
          <label key={k} style={{ fontSize: 10, color: "var(--color-text-secondary)", cursor: "pointer", marginLeft: 4 }}>
            <input type="checkbox" checked={vis[k]} onChange={e => setVis(prev => ({ ...prev, [k]: e.target.checked }))} /> {l}
          </label>)}
        <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 8 }}>Arrows move ({pawnPos[0]},{pawnPos[1]})</span>
        <button onClick={() => { setDockKey(k => k + 1); setPanelOrder(defaultOrder); }} style={{ fontSize: 10, padding: "2px 8px", marginLeft: 8, borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-secondary)" }}>Dock all</button>
        <button onClick={() => { setP(initState()); setSelTh(0); }} style={{ fontSize: 10, padding: "2px 8px", marginLeft: 4, borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-secondary)" }}>Reset all</button>
        {isModified && <span style={{ fontSize: 9, color: "var(--color-text-tertiary)", marginLeft: 4 }}>● modified</span>}
        <button onClick={() => setShowDump(d => !d)}
          style={{ fontSize: 10, padding: "2px 8px", marginLeft: 4, borderRadius: 4, cursor: "pointer", border: "1px solid " + (showDump ? "var(--color-border-info)" : "var(--color-border-success)"), background: showDump ? "var(--color-background-info)" : "transparent", color: showDump ? "var(--color-text-info)" : "var(--color-text-success)" }}>{showDump ? "Hide" : "Save"}</button>
      </div>

      {showDump && (() => {
        const r = (v, d=4) => +(+v).toFixed(d);
        const a = arr => "[" + arr.map(v => r(v)).join(", ") + "]";
        const RM_N = ["inherit", "follow", "independent"];
        const ANC_N = ["Pyramid", "Arch", "Column", "Antenna"];
        const fmtSlot = (sl, fi) => { const tm = sl.tm ?? 7; const tiers = TN[FK[fi]].filter((_, ti) => (tm >> ti) & 1).map(n => n.slice(0,3)).join("+"); return "ch:" + r(sl.ch) + " dist:" + r(sl.di,1) + "/" + r(sl.ds,1) + " ang:" + r(sl.an,3) + " a\u03C3:" + r(sl.as) + " rot:" + RM_N[sl.rm] + " drift:" + r(sl.dr) + " tiers:" + tiers; };
        const fmtF = (f, fname, fi) => {
          const anchorName = f.anchor < 0 ? FN[fi] : ANC_N[f.anchor];
          const activeKey = String(f.anchor);
          const keys = Object.keys(f.slots);
          if (keys.length === 0) return "    " + fname + ":   anchor:" + anchorName + " (no formation)";
          let out = "";
          for (const k of keys) {
            const isActive = k === activeKey;
            const sl = f.slots[k];
            const slotAnchor = +k < 0 ? FN[fi] : ANC_N[+k];
            out += "    " + fname + ":   anchor:" + slotAnchor + (isActive ? " *" : "  ") + " " + fmtSlot(sl, fi) + "\n";
          }
          return out.trimEnd();
        };
        let s = "=== 7T THEME TOOL STATE ===\n";
        s += "seed: " + P.seed + "  solo: " + (P.solo >= 0 ? THN[P.solo] : "off") + "\n\n";
        s += "archetype weights: " + a(P.aw) + "\n\n";
        s += "base spawn chances:\n";
        s += "  pyramid: " + r(P.sc.p) + "\n  arch:    " + r(P.sc.a) + "\n  column:  " + r(P.sc.c) + "\n  antenna: " + r(P.sc.n) + "\n\n";
        s += "base tier weights:\n";
        s += "  pyramid: " + a(P.tw.p) + "\n  arch:    " + a(P.tw.a) + "\n  column:  " + a(P.tw.c) + "\n  antenna: " + a(P.tw.n) + "\n\n";
        s += "separation matrix:\n";
        s += "          Pyr   Arch  Col   Ant\n";
        s += "  Pyr:  " + a(P.sep[0]) + "\n  Arch: " + a(P.sep[1]) + "\n  Col:  " + a(P.sep[2]) + "\n  Ant:  " + a(P.sep[3]) + "\n\n";
        s += "mute: pyr:" + P.mute[0] + " arch:" + P.mute[1] + " col:" + P.mute[2] + " ant:" + P.mute[3] + "\n\n";
        P.th.forEach((t, i) => {
          s += "--- Theme " + i + ": " + THN[i] + " ---\n";
          s += "  envelope: spike:" + r(t.spike,0) + " sustain:" + t.sustain + " decay:" + t.decay + " cooldown:" + t.cooldown + "\n";
          s += "  spawn: " + a(t.sp) + "\n";
          s += "  tier_pyr: " + a(t.tp) + "  tier_arch: " + a(t.ta) + "\n";
          s += "  tier_col: " + a(t.tc) + "  tier_ant: " + a(t.tn) + "\n";
          s += "  formation:\n";
          s += fmtF(t.f[0], "Pyramid", 0) + "\n";
          s += fmtF(t.f[1], "Arch   ", 1) + "\n";
          s += fmtF(t.f[2], "Column ", 2) + "\n";
          s += fmtF(t.f[3], "Antenna", 3) + "\n\n";
        });
        const taId = "save-dump-ta";
        return (<div style={{ position: "relative", marginBottom: 6 }}>
          <textarea id={taId} readOnly value={s} onFocus={e => e.target.select()} style={{ width: "100%", height: 200, fontSize: 10, fontFamily: "monospace", padding: 6, borderRadius: 6, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-secondary)", color: "var(--color-text-primary)", resize: "vertical" }} />
          <button onClick={(ev) => {
            const ta = document.getElementById(taId);
            if (ta) { ta.select(); ta.setSelectionRange(0, ta.value.length); }
            try { document.execCommand("copy"); ev.target.textContent = "Copied!"; setTimeout(() => { ev.target.textContent = "Copy"; }, 1200); }
            catch(e) { ev.target.textContent = "Select all \u2192 Ctrl+C"; setTimeout(() => { ev.target.textContent = "Copy"; }, 2000); }
          }} style={{ position: "absolute", top: 4, right: 4, fontSize: 9, padding: "2px 8px", borderRadius: 4, cursor: "pointer", border: "1px solid var(--color-border-success)", background: "var(--color-background-primary)", color: "var(--color-text-success)" }}>Copy</button>
        </div>);
      })()}

      <div style={{ position: "relative", border: "1px solid var(--color-border-tertiary)", borderRadius: 8, overflow: "hidden", background: "var(--color-background-primary)", marginBottom: 6 }}>
        <canvas ref={cvRef} onMouseMove={onMove} style={{ width: "100%", height: 460, display: "block", cursor: "crosshair" }} />
        {hov && <div style={{ position: "absolute", top: 4, right: 4, background: "var(--color-background-primary)", border: "1px solid var(--color-border-tertiary)", borderRadius: 6, padding: "4px 8px", fontSize: 10, lineHeight: 1.5, pointerEvents: "none" }}>
          <span style={{ fontWeight: 500, color: FC[hov.fam] }}>{FN[hov.fam]}</span> {" - "} {hov.tn} ({hov.sc.toFixed(2)})<br />{THN[hov.thm]} | {Math.round(hov.rot * 180 / Math.PI)}{"\u00B0"} {hov.formed && "\u26A1"}
        </div>}
        {worldRef.current && (() => {
          const env = worldRef.current.env;
          const active = env.active;
          const th = active >= 0 ? P.th[active] : null;
          let phase = "idle", progress = 0, total = 1;
          if (th) {
            if (env.elapsed < th.sustain) { phase = "sustain"; progress = env.elapsed; total = th.sustain; }
            else if (env.elapsed < th.sustain + th.decay) { phase = "decay"; progress = env.elapsed - th.sustain; total = th.decay; }
            else { phase = "expired"; }
          }
          const barW = 80, barH = 4;
          const fill = total > 0 ? Math.min(1, progress / total) : 0;
          const coolStrs = env.cooldowns.map((c, i) => c > 0 ? THN[i][0] + c : null).filter(Boolean);
          return (
            <div style={{ position: "absolute", bottom: 4, left: 4, background: "rgba(0,0,0,0.55)", borderRadius: 5, padding: "3px 8px", fontSize: 9, lineHeight: 1.5, pointerEvents: "none", color: "#ccc", display: "flex", alignItems: "center", gap: 6 }}>
              <span style={{ fontWeight: 600, color: active >= 0 ? "#e8e0d0" : "#888" }}>{active >= 0 ? THN[active] : "No theme"}</span>
              {th && <span style={{ color: phase === "sustain" ? "#8c8" : phase === "decay" ? "#ca8" : "#888" }}>{phase}</span>}
              {th && <span style={{ display: "inline-block", width: barW, height: barH, background: "rgba(255,255,255,0.12)", borderRadius: 2, overflow: "hidden", verticalAlign: "middle" }}>
                <span style={{ display: "block", width: fill * barW, height: barH, background: phase === "sustain" ? "#6a6" : "#b85", borderRadius: 2 }} />
              </span>}
              {coolStrs.length > 0 && <span style={{ color: "#866", fontSize: 8 }}>cool: {coolStrs.join(" ")}</span>}
            </div>
          );
        })()}
      </div>

      {stats && <div style={{ fontSize: 10, color: "var(--color-text-secondary)", marginBottom: 6, display: "flex", gap: 6, flexWrap: "wrap", alignItems: "center" }}>
        <span>{stats.t} ents / {stats.patches} patches ({(stats.t / Math.max(stats.patches, 1)).toFixed(2)}/p) {"\u00B7"} {stats.formed} formed</span>
        {[
          { name: "Pyramid", count: stats.p, fi: 0, col: FC[0] },
          { name: "Arch", count: stats.a, fi: 1, col: FC[1] },
          { name: "Column", count: stats.c, fi: 2, col: FC[2] },
          { name: "Antenna", count: stats.ant, fi: 3, col: FC[3] },
        ].map(({ name, count, fi, col }, idx) => {
          const muted = P.mute[fi];
          return (<button key={idx} onClick={() => upd(n => { n.mute[fi] = !n.mute[fi]; })}
            title={muted ? "Show " + FN[fi] + "s" : "Mute " + FN[fi] + "s"}
            style={{ display: "flex", alignItems: "center", gap: 3, fontSize: 10, padding: "1px 6px", borderRadius: 4, cursor: "pointer",
              border: "1px solid " + (muted ? "var(--color-border-tertiary)" : col),
              background: muted ? "var(--color-background-secondary)" : "transparent",
              color: muted ? "var(--color-text-tertiary)" : col,
              opacity: muted ? 0.5 : 1, textDecoration: muted ? "line-through" : "none" }}>
            {name} {count}
          </button>);
        })}
        <span style={{ marginLeft: "auto", display: "flex", gap: 5 }}>{AN.map((n, i) => P.aw[i] > 0 ? <span key={i} style={{ display: "flex", alignItems: "center", gap: 2 }}><span style={{ width: 7, height: 7, borderRadius: 2, background: AC[i], display: "inline-block" }} />{n}</span> : null)}</span>
      </div>}

      {panelOrder.map(pid => {
        const dp = { key: pid, id: pid, resetKey: dockKey, onDock: (id) => setPanelOrder(prev => [...prev.filter(p => p !== id), id]) };
        switch (pid) {
        case "theme": return (
          <DragPanel {...dp} title={"Theme: " + THN[selTh] + (P.solo >= 0 ? " [SOLO: " + THN[P.solo] + "]" : "")} ini={true}>
            <div style={{ display: "flex", gap: 3, marginBottom: 6, flexWrap: "wrap", alignItems: "center" }}>
              {THN.map((name, i) => {
                const isSoloed = P.solo === i;
                return (<div key={i} style={{ display: "flex", gap: 1 }}>
                  <button onClick={() => setSelTh(i)} style={{ fontSize: 10, padding: "2px 7px", borderRadius: 4, cursor: "pointer", display: "flex", alignItems: "center", gap: 4,
                    border: i === selTh ? "2px solid var(--color-border-info)" : "1px solid var(--color-border-tertiary)",
                    background: i === selTh ? "var(--color-background-info)" : "var(--color-background-secondary)",
                    color: i === selTh ? "var(--color-text-info)" : "var(--color-text-secondary)",
                    opacity: P.solo >= 0 && !isSoloed ? 0.35 : 1 }}>
                    {name}
                  </button>
                </div>);
              })}
            </div>
            <div style={{ display: "flex", gap: 4, marginBottom: 6, alignItems: "center" }}>
              <button onClick={() => upd(n => { n.solo = n.solo === selTh ? -1 : selTh; })}
                style={{ fontSize: 10, padding: "3px 10px", borderRadius: 4, cursor: "pointer", fontWeight: 600,
                  border: P.solo === selTh ? "2px solid #e03030" : "1px solid var(--color-border-tertiary)",
                  background: P.solo === selTh ? "rgba(224,48,48,.12)" : "var(--color-background-secondary)",
                  color: P.solo === selTh ? "#e03030" : "var(--color-text-secondary)" }}>
                {P.solo === selTh ? "\u25CF Solo ON" : "Solo"}
              </button>
              {P.solo >= 0 && <span style={{ fontSize: 9, color: "#e03030" }}>Only {THN[P.solo]} — all others excluded</span>}
            </div>
            <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", marginBottom: 2 }}>Envelope</div>
            <div style={rw}>
              <span style={lb}>Spike</span><Num value={ct.spike} min={0} max={500} step={10} w={42} onChange={v => upd(n => { n.th[selTh].spike = v; })} />
              <span style={lb}>Sustain</span><Num value={ct.sustain} min={0} max={99} step={1} w={34} onChange={v => upd(n => { n.th[selTh].sustain = Math.round(v); })} />
              <span style={lb}>Decay</span><Num value={ct.decay} min={0} max={30} step={1} w={34} onChange={v => upd(n => { n.th[selTh].decay = Math.round(v); })} />
              <span style={lb}>Cool</span><Num value={ct.cooldown} min={0} max={99} step={1} w={34} onChange={v => upd(n => { n.th[selTh].cooldown = Math.round(v); })} />
            </div>
            <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "4px 0 2px" }}>Spawn mult.</div>
            <div style={rw}>{FN.map((name, fi) => <label key={fi} style={{ fontSize: 10, display: "flex", gap: 2, alignItems: "center", color: FC[fi] }}>{name} <Num value={ct.sp[fi]} min={0} max={10} w={38} onChange={v => upd(n => { n.th[selTh].sp[fi] = v; })} /></label>)}</div>
            <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "4px 0 2px" }}>Tier bias</div>
            {[["Pyr", "tp", TN.p], ["Arch", "ta", TN.a], ["Col", "tc", TN.c], ["Ant", "tn", TN.n]].map(([label, key, names]) => <div key={key} style={{ marginBottom: 2 }}><span style={{ fontSize: 9, color: "var(--color-text-tertiary)" }}>{label}:</span><div style={rw}>{names.map((tn, ti) => <label key={ti} style={{ fontSize: 10, display: "flex", gap: 2, alignItems: "center" }}>{tn} <Num value={(ct[key] && ct[key][ti]) || 1} min={0} max={10} w={34} onChange={v => upd(n => { n.th[selTh][key][ti] = v; })} /></label>)}</div></div>)}
            <div style={{ fontSize: 10, fontWeight: 500, color: "var(--color-text-secondary)", margin: "4px 0 2px" }}>Formation (differential tip)</div>
            <div style={{ overflowX: "auto" }}>
              <table style={{ fontSize: 10, borderCollapse: "collapse", width: "100%" }}>
                <thead><tr>{["", "Anchor", "Ch", "Dist", "\u03C3", "Ang", "A\u03C3", "Rot", "Drft", "Tiers"].map((h, i) => <th key={i} style={{ ...thd, textAlign: i ? "right" : "left" }}>{h}</th>)}</tr></thead>
                <tbody>{FN.map((name, fi) => { const fr = ct.f && ct.f[fi]; if (!fr) return null;
                  const slot = getSlot(fr);
                  const active = slot.ch > 0;
                  const dim = { opacity: active ? 1 : 0.3, pointerEvents: active ? "auto" : "none" };
                  const uAnc = (v) => upd(n => { n.th[selTh].f[fi].anchor = v; });
                  const uSlot = (k, v) => upd(n => {
                    const f = n.th[selTh].f[fi];
                    const key = String(f.anchor);
                    if (!f.slots[key]) f.slots[key] = { ...EMPTY_SLOT };
                    f.slots[key][k] = v;
                  });
                  const tierNames = TN[FK[fi]];
                  const tm = slot.tm ?? 7;
                  return (<tr key={fi}><td style={tdl}>{name}</td>
                    <td style={tdd}><select value={fr.anchor} onChange={e => uAnc(+e.target.value)} style={{ fontSize: 9, padding: "1px", borderRadius: 3, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)" }}><option value={-1}>{name}</option>{FN.map((n, i) => i !== fi ? <option key={i} value={i}>{n}</option> : null)}</select></td>
                    <td style={tdd}><Num value={slot.ch} min={0} max={1} w={30} onChange={v => uSlot("ch", v)} /></td>
                    <td style={{ ...tdd, ...dim }}><Num value={slot.di} min={0} max={300} step={1} w={34} onChange={v => uSlot("di", v)} /></td>
                    <td style={{ ...tdd, ...dim }}><Num value={slot.ds} min={0} max={50} step={1} w={28} onChange={v => uSlot("ds", v)} /></td>
                    <td style={{ ...tdd, ...dim }}><Num value={slot.an} min={0} max={6.28} w={30} onChange={v => uSlot("an", v)} /></td>
                    <td style={{ ...tdd, ...dim }}><Num value={slot.as} min={0} max={2} w={28} onChange={v => uSlot("as", v)} /></td>
                    <td style={{ ...tdd, ...dim }}><select value={slot.rm} onChange={e => uSlot("rm", +e.target.value)} style={{ fontSize: 9, padding: "1px", borderRadius: 3, border: "1px solid var(--color-border-tertiary)", background: "var(--color-background-primary)", color: "var(--color-text-primary)", opacity: active ? 1 : 0.3, pointerEvents: active ? "auto" : "none" }}>{RM.map((n, i) => <option key={i} value={i}>{n}</option>)}</select></td>
                    <td style={{ ...tdd, ...dim }}><Num value={slot.dr} min={0} max={1} w={28} onChange={v => uSlot("dr", v)} /></td>
                    <td style={{ ...tdd, ...dim, padding: "2px 4px", minWidth: 70 }}><div style={{ display: "flex", gap: 2, justifyContent: "flex-end" }}>{tierNames.map((tn, ti) => {
                      const on = (tm >> ti) & 1;
                      return (<span key={ti} data-notdrag="1" onClick={() => uSlot("tm", tm ^ (1 << ti))} title={tn} style={{ fontSize: 8, padding: "1px 4px", borderRadius: 3, cursor: "pointer", border: on ? "1px solid rgba(100,180,100,0.7)" : "1px solid rgba(120,120,120,0.3)", background: on ? "rgba(80,160,80,0.25)" : "rgba(80,80,80,0.15)", color: on ? "#8c8" : "#666", lineHeight: "14px", userSelect: "none", fontWeight: on ? 600 : 400 }}>{tn.slice(0, 3)}</span>);
                    })}</div></td>
                  </tr>);})}</tbody>
              </table>
            </div>
          </DragPanel>);
        case "arch": return (
          <DragPanel {...dp} title="Archetype weights">
            <div style={rw}>{AN.map((name, i) => <label key={i} style={{ fontSize: 10, display: "flex", alignItems: "center", gap: 3 }}><span style={{ width: 7, height: 7, borderRadius: 2, background: AC[i], display: "inline-block" }} />{name} <Num value={P.aw[i]} min={0} max={5} w={38} onChange={v => upd(n => { n.aw[i] = v; })} /></label>)}</div>
          </DragPanel>);
        case "spawn": return (
          <DragPanel {...dp} title="Base spawn chances">
            <table style={{ fontSize: 10, borderCollapse: "collapse", width: "100%" }}>
              <thead><tr><th style={{ ...thd, textAlign: "left" }}>Family</th><th style={thd}>Chance</th></tr></thead>
              <tbody>{FK.map((fk, fi) => <tr key={fk}><td style={tdl}>{FN[fi]}</td>
                <td style={tdd}><Num value={P.sc[fk]} min={0} max={1} step={.001} w={52} onChange={v => upd(n => { n.sc[fk] = v; })} /></td>
              </tr>)}</tbody>
            </table>
          </DragPanel>);
        case "tier": return (
          <DragPanel {...dp} title="Base tier weights">
            {FK.map((fk, fi) => <div key={fk} style={{ marginBottom: 4 }}><span style={{ fontSize: 9, fontWeight: 500, color: "var(--color-text-secondary)" }}>{FN[fi]}</span><div style={rw}>{TN[fk].map((tn, ti) => <label key={ti} style={{ fontSize: 10, display: "flex", gap: 2, alignItems: "center" }}>{tn} <Num value={P.tw[fk][ti]} min={0} max={2} w={38} onChange={v => upd(n => { n.tw[fk][ti] = v; })} /></label>)}</div></div>)}
          </DragPanel>);
        case "sep": return (
          <DragPanel {...dp} title="Separation matrix (wu)">
            <table style={{ fontSize: 10, borderCollapse: "collapse" }}>
              <thead><tr><th style={{ ...thd, textAlign: "left" }}>{"\u2193 / \u2192"}</th>{FN.map((n, i) => <th key={i} style={thd}>{n}</th>)}</tr></thead>
              <tbody>{P.sep.map((row, ri) => <tr key={ri}><td style={tdl}>{FN[ri]}</td>{row.map((v, ci) => <td key={ci} style={tdd}><Num value={v} min={0} max={200} step={1} w={38} onChange={nv => upd(n => { n.sep[ri][ci] = nv; })} /></td>)}</tr>)}</tbody>
            </table>
          </DragPanel>);
        default: return null;
        }
      })}
    </div>
  );
}