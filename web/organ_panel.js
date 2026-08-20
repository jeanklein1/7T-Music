// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE PANEL SHELL (ORGAN_0c)
//
// A VIEW of the program (docs/ORGAN.md). Every row here is drawn from the
// manifest the C++ registry emits, so this file knows no parameter names,
// no ranges and no offsets — enroll a dial in organ_params.inc and it
// appears; remove one and it stops. There is nothing to keep in step.
//
// ACCESS. Without ?organ=1 AND without ?preset=<name> this file returns on
// its first statement: no DOM, no stylesheet, no timer, no ccall. The
// audience path is byte-identical, and that is why the file may ship
// unconditionally. ORGAN_4 P6 added the second flag and kept that promise:
// an exhibition boot names a scene and gets no panel, a plain visit names
// neither and pays nothing.
//
// DEFINITION OR PREVIEW (O1b). A dial marked * has a mood DEFINITION
// behind its home. In definition mode — the default — writing it changes
// what the live mood MEANS and lets the program's own mood apply produce
// the picture; the edit survives the next mood change. In preview mode the
// write goes to the instance, which is immediate and temporary. Dials
// without a * have no definition to write and behave the same either way.
//
// CONTEST MARKERS (O1a). Every row carries the C++ instrument's reading of
// whether the panel's last word on that dial still stands — free / event /
// frame, with the number of frames it stood. The panel does not compute
// this and cannot: only the program can see its own homes between frames.
// A row shows a dot until this session has written it, because an unasked
// question has no answer.
//
// AESTHETIC BOUND. This is an instrument, not the art. Monospace, one dark
// column, no animation, no branding. The art stays on the canvas.
// ═══════════════════════════════════════════════════════════════════════
(function () {
  'use strict';
  // ORGAN_4 P6 — TWO REASONS TO WAKE, NOT ONE. `?organ=1` opens the panel;
  // `?preset=<name>` applies a scene. An EXHIBITION boots with the second
  // and not the first — São Paulo wants the design, not the instrument —
  // so gating the whole file on `organ=1` would have made the preset layer
  // reachable only by someone already holding the panel open, which is the
  // opposite of what it is for. A plain visitor still pays nothing: with
  // neither flag this file returns on its second statement, as before.
  var Q = new URLSearchParams(location.search);
  var WANT_PANEL = Q.get('organ') === '1';
  var WANT_PRESET = Q.get('preset');
  if (!WANT_PANEL && !WANT_PRESET) return;

  var F32 = 0, U32 = 1, BOOL = 2, VEC3 = 3, VEC4 = 4;
  // ORGAN_6 — THE SHELL KNOWS NO NUMBERS. It asked two questions of the
  // manifest's raw columns and answered them here, in JS, from copies of
  // C++ enum values: which blocks are def-only sentinels, and which kinds
  // are world-scoped. Both are now DERIVED in organ_registry.hpp and
  // EMITTED — `inst` and `scope` — the same move derived_cadence already
  // made for `cad`, and for the same reason: a rule with two homes drifts,
  // and this one did. A sixth definition family costs one line there and
  // none here.
  //
  //   p.inst   1 the panel may address an instance · 0 definition only
  //   p.scope  0 no definition · 1 mood-selected · 2 the world's

  // ORGAN_3b — CADENCE. The manifest's "cad" says WHEN a stop sounds; the
  // C++ derives it, this file only prints it. Live is SILENT on purpose:
  // silence is the default state of a working dial, and a chip on every
  // row would be noise rather than information. The other three each
  // answer a question the operator would otherwise have to ask the source.
  var CAD = [null, 'on respawn', 'boundary', 'driven'];

  // ── ORGAN_5 P2b — THE ONE RULED EXCEPTION TO NAME-BLINDNESS (D3) ──
  // Four strings. THE AUTHORITY IS bodies/orbs.hpp — `RULE_NAMES` beside
  // cycle_orb_motion_rule, and the ORB_RULE_* constants the kernel's own
  // dispatch tests (`orb_config.motion_rule == 0u` … in world.wgsl).
  // Their ORDER is the contract; a fifth rule adds a string here and a
  // branch there, and both fail loudly rather than silently.
  //
  // Everything else about the rule readout stays name-blind: which GROUPS
  // are rule-scoped is derived from the group's own name below, not
  // listed, and which DOORS cycle the rule is never asked — the readout
  // sits under the whole door bar, so renumbering a door cannot move it
  // onto the wrong button.
  var RULE_NAMES = ['brownian', 'orbital', 'frozen', 'flocking'];

  // ── ORGAN_5 P5b — WHICH BUILD IS THIS? ────────────────────────────
  // index.html sets window.T7_BUILD_ID from the same `BUILD` variable it
  // versions every script URL with (BUILDID_0), so the footer names the
  // bytes the visitor is actually running. Two devices showing different
  // ids are not the same program, and "count works here and not there"
  // stops being a mystery and becomes a fact one glance settles.
  //
  // ABSENT IS NOT AN ERROR. Opening web/index.html directly leaves the
  // placeholder unsubstituted, and a page that predates this line has no
  // global at all; either way the footer simply says nothing rather than
  // printing a lie about provenance.
  var BUILD_ID = (typeof window !== 'undefined' && window.T7_BUILD_ID) || '';
  var BUILD_OK = BUILD_ID && BUILD_ID.indexOf('_') < 0;   // not the placeholder
  var BUILD_TAG = BUILD_OK ? ('build ' + BUILD_ID + '  \u00b7  ') : '';

  // A group whose name ends in "<rule> rule" is scoped to that rule, and
  // the shell learns which one from the name itself. "Flocking rule" -> 3,
  // "Orbital rule" -> 1, "Motion — all rules" -> none (and no line, which
  // is correct: those rows act under every rule).
  function ruleOfGroup(name) {
    var m = /(^|\s)([A-Za-z]+)\s+rule$/.exec(name || '');
    if (!m) return -1;
    return RULE_NAMES.indexOf(m[2].toLowerCase());
  }
  // ATMOS_1b/2 — a group whose name ends "Regime N" is scoped to that
  // regime, read from the name as ruleOfGroup reads its rule. N is the
  // label's number; the draw's regime is the Atmosphere.regime[] index,
  // so "Regime 1" -> 0. Adding a regime group in the .inc needs no edit
  // here; the operator never sees the index.
  function regimeOfGroup(name) {
    var m = /\bRegime (\d)$/.exec(name || '');
    return m ? parseInt(m[1], 10) - 1 : -1;
  }
  var SEP = ' \u00b7 ';         // ORGAN_3 — the group path's separator
  var lanes = function (t) { return t === VEC3 ? 3 : t === VEC4 ? 4 : 1; };

  // ── ORGAN_3c P0 — THE GRID'S FIXED PARTS, IN ONE HOME ─────────────
  // Every fixed width the row grid uses is a CSS custom property on
  // #organ, and W_MIN below is COMPUTED from the same numbers. The
  // stylesheet and the resize clamp therefore cannot disagree, which is
  // the only way "nothing overlaps at any lawful width" stays true (D2)
  // — a hardcoded minimum is a guess, and a guess is how overlap returns.
  var G = {
    pad:    10,   // #organ horizontal padding, per side
    grip:    6,   // ORGAN_3d — the resize gutter, ADDED to the left padding
    border:  1,   // #organ border-left — inside the width, see box-sizing
    body:    2,   // details.sec .body padding-left
    hdgap:   4,   // line 1 column gap
    gap:     6,   // line 2 column gap
    lblmin: 110,  // the label column's floor — below this an ellipsis lies
    sw:      28,  // the colour swatch, when a row has one
    mk:      58,  // the contest column ("event 1200" at 10px monospace)
    chip:    62,  // the cadence chip ("on respawn" at 9px, plus its pill)
    slmin:   90,  // the slider's floor — THIS is the acceptance test
    val:     56   // the value box: ~7ch, right-aligned, plus border+padding
  };
  // Line 1 governs the minimum; line 2 is narrower by construction.
  //
  // ORGAN_3d — THE MODEL IS NOW EXACT, NOT MERELY CONSERVATIVE. #organ is
  // box-sizing:border-box, so `width` is the whole panel: border, both
  // paddings and the content column. Before this the panel was content-box
  // and `width` named only the column, which is why the grip — positioned
  // off `width` — landed 21px inside the content it was meant to sit
  // beside, and why `min(640, 50vw)` was quietly a 661px panel. The grip
  // gutter is ADDED to the left padding rather than replacing it, so the
  // rows keep the breathing room the right side has.
  var W_MIN = G.border + (G.pad + G.grip) + G.pad + G.body
                       + G.lblmin + 3 * G.hdgap + G.sw + G.mk + G.chip;
  var W_DEF = 330;
  function wMax() {
    var half = Math.floor((window.innerWidth || 1280) / 2);
    return Math.max(W_MIN, Math.min(640, half));   // D2
  }
  var px = function (n) { return n + 'px'; };

  var CSS =
    '#organ{' +
    '--hdgap:' + px(G.hdgap) + ';--gap:' + px(G.gap) + ';' +
    '--lblmin:' + px(G.lblmin) + ';--sw:' + px(G.sw) + ';--mk:' + px(G.mk) + ';' +
    '--chip:' + px(G.chip) + ';--slmin:' + px(G.slmin) + ';--val:' + px(G.val) + ';' +
    '--pad:' + px(G.pad) + ';--grip-w:' + px(G.grip) + ';' +
    'box-sizing:border-box;position:fixed;top:0;right:0;width:' + px(W_DEF) + ';' +
    'max-height:100vh;overflow-y:auto;' +
    'background:#0d0f12;color:#c8ccd2;font:11px/1.45 ui-monospace,Menlo,Consolas,monospace;' +
    'border-left:' + px(G.border) + ' solid #262b33;z-index:9999;' +
    // ORGAN_3d — the gutter is the grip's alone. The left padding carries
    // it in ADDITION to the panel's own inset, so the content column is
    // never under the strip and the rows keep the right side's margin.
    'padding:8px var(--pad) 14px calc(var(--pad) + var(--grip-w))}' +
    '#organ.hidden{display:none}' +
    '#organ h1{font-size:11px;letter-spacing:.14em;color:#7d8894;margin:2px 0 10px;font-weight:400}' +
    '#organ h2{font-size:10px;letter-spacing:.1em;color:#5c93c4;margin:12px 0 4px;font-weight:400;' +
    'border-bottom:1px solid #1d222a;padding-bottom:2px}' +
    // ORGAN_3 — SECTIONS. Native <details>, so the open/closed state is the
    // browser's and this file keeps none: no JS, no animation, no third level.
    '#organ details.sec{margin:10px 0 0;border-top:1px solid #262b33;padding-top:4px}' +
    '#organ details.sec>summary{font-size:11px;letter-spacing:.16em;color:#8fa3b8;' +
    'cursor:pointer;padding:2px 0;text-transform:uppercase;outline:none}' +
    '#organ details.sec>summary::-webkit-details-marker{color:#4f5761}' +
    '#organ details.sec[open]>summary{color:#cfe0ef}' +
    '#organ details.sec>summary .n{color:#4f5761;letter-spacing:0;font-size:10px}' +
    '#organ details.sec .body{padding-left:2px}' +
    '#organ details.sec .body h2:first-child{margin-top:6px}' +
    // ── ORGAN_3c P0 — THE ROW GRID ───────────────────────────────────
    // Two lines, both grids, columns placed EXPLICITLY so a row with no
    // swatch and no chip still lines its markers up with the row above.
    //   line 1  [ label ……………………………  sw  mk  chip ]
    //   line 2  [ slider ——————————————————— | value ]
    // The label ellipsis is the only thing that gives, and it hands what
    // it hid to the title. Nothing else may shrink past its floor, so a
    // slider can never end up behind the number — Jean's acceptance test.
    '#organ .hd{display:grid;align-items:center;column-gap:var(--hdgap);margin:5px 0 0;' +
    'grid-template-columns:minmax(var(--lblmin),1fr) var(--sw) var(--mk) var(--chip)}' +
    '#organ .ln{display:grid;align-items:center;column-gap:var(--gap);margin:1px 0 2px;' +
    'grid-template-columns:minmax(var(--slmin),1fr) var(--val)}' +
    '#organ .lbl{grid-column:1;min-width:0;color:#96a0ab;' +
    'overflow:hidden;text-overflow:ellipsis;white-space:nowrap}' +
    '#organ .ln input[type=range]{grid-column:1;width:100%;min-width:var(--slmin);' +
    'accent-color:#5c93c4;height:14px;margin:0}' +
    '#organ .ln input[type=number]{grid-column:2;width:var(--val);box-sizing:border-box;' +
    'text-align:right;background:#14181d;color:#c8ccd2;border:1px solid #262b33;' +
    'font:inherit;padding:1px 3px}' +
    '#organ .ln input[type=checkbox]{grid-column:1;justify-self:start;margin:0}' +
    '#organ input[type=color]{grid-column:2;width:var(--sw);height:15px;box-sizing:border-box;' +
    'background:#14181d;border:1px solid #262b33;padding:0}' +
    '#organ .ro{grid-column:2;text-align:right;color:#8fa3b8;' +
    'overflow:hidden;text-overflow:ellipsis;white-space:nowrap}' +
    // ORGAN_3b — the cadence chip. One rule, three modifiers, no colour
    // louder than the existing accent; the panel is an instrument.
    // ORGAN_3c moved it to the label line: it never again competes with
    // a control for width, so it costs the slider nothing.
    '#organ .cad{grid-column:4;box-sizing:border-box;width:var(--chip);text-align:center;' +
    'font-size:9px;letter-spacing:.06em;color:#4f5761;overflow:hidden;white-space:nowrap;' +
    'border:1px solid #262b33;border-radius:2px;padding:0 2px}' +
    '#organ .cad.gen{color:#b0954e;border-color:#3a3226}' +
    '#organ .cad.boundary{color:#5c93c4;border-color:#24313d}' +
    '#organ .cad.driven{color:#8fa3b8;border-color:#262b33}' +
    '#organ .mk{grid-column:3;width:var(--mk);text-align:right;font-size:10px;color:#4f5761;' +
    'overflow:hidden;white-space:nowrap}' +
    '#organ .mk.free{color:#5f8f6a}' +
    '#organ .mk.event{color:#b0954e}' +
    '#organ .mk.frame{color:#b0644e}' +
    '#organ .legend{color:#4f5761;margin-top:3px}' +
    // ── ORGAN_3d — THE GRIP TAKES ITS OWN GUTTER ─────────────────────
    // THE VISIBLE LINE IS THE PANEL'S OWN BORDER. The grip draws nothing:
    // it is an invisible hit strip flush inside that border, in a gutter
    // the padding reserves for it, so it shares no pixel with any control.
    // ORGAN_3c positioned it off `width` while the panel was content-box,
    // which put it 21px into the content column — it ate the sliders'
    // leftmost pixels and clipped every label's first glyph. Nothing is
    // computed here any more; CSS puts it where it belongs.
    //
    // IT STAYS FIXED, and that is deliberate. #organ scrolls (263 rows of
    // two lines), and an absolutely-positioned child of a scroll container
    // rides the scrolled content — the gutter would be grabbable only near
    // the top. Fixed keeps it whole at every scroll position, which is the
    // one thing ORGAN_3c's grip did get right. Under box-sizing:border-box
    // its horizontal placement is no longer a guess: the panel spans
    // viewport-right 0..width, the border owns the last pixel of that, and
    // the gutter is the --grip-w beneath it. #organ is already
    // position:fixed, so it needs no position:relative to hold a child.
    '#organ .grip{position:fixed;top:0;bottom:0;width:var(--grip-w);' +
    'cursor:ew-resize;z-index:10000;background:transparent}' +
    // The affordance is the border brightening, plus the cursor. One line,
    // the existing accent, no new colour and no second drawn edge.
    '#organ:has(>.grip:hover),#organ:has(>.grip.drag){border-left-color:#5c93c4}' +
    '#organ button{background:#14181d;color:#c8ccd2;border:1px solid #303742;font:inherit;' +
    'padding:3px 9px;cursor:pointer}' +
    '#organ button:hover{border-color:#5c93c4}' +
    '#organ button.on{background:#1d2a36;border-color:#5c93c4;color:#cfe0ef}' +
    '#organ .star{color:#5c93c4}' +
    // LENS_1 — the kin marker: under ALL, a label whose regimes disagree.
    '#organ .hd.kin .lbl::after{content:" \\2260";color:#5c93c4}' +
    '#organ .foot{margin-top:12px;padding-top:6px;border-top:1px solid #1d222a;color:#6b7480}' +
    '#organ .bar{display:flex;gap:6px;margin-bottom:6px}' +
    '#organ .bar.doors{margin:-2px 0 8px;flex-wrap:wrap}' +
    '#organ .bar.doors button{color:#9fb3c8;border-color:#2c3a46}' +
    '#organ .bar.doors button:hover{border-color:#5c93c4;color:#cfe0ef}' +
    // ORGAN_3b P4 — NAVIGATION. A filter field and a per-section export
    // affordance; nothing else, because a dev instrument that grows a
    // chrome grows a maintenance bill the artwork never asked for.
    '#organ .find{display:block;width:100%;box-sizing:border-box;margin:0 0 6px;' +
    'background:#14181d;color:#c8ccd2;border:1px solid #262b33;font:inherit;padding:3px 5px}' +
    '#organ .find:focus{outline:none;border-color:#5c93c4}' +
    '#organ details.sec>summary .sx{float:right;background:none;border:0;color:#4f5761;' +
    'font:inherit;padding:0 2px;cursor:pointer;letter-spacing:0}' +
    '#organ details.sec>summary .sx:hover{color:#5c93c4}' +
    // ORGAN_4 P6 — a select in a bar, dressed as the buttons beside it so
    // the bar reads as one row rather than as a native control dropped in.
    // ATMOS_1 — the law was always about a select in a bar, not about
    // presets, so the selector says that and the mood door inherits it.
    '#organ .bar select{background:#14181d;color:#c8ccd2;' +
    'border:1px solid #303742;font:inherit;padding:3px 6px;cursor:pointer}' +
    '#organ .bar select:hover{border-color:#5c93c4}' +
    // ORGAN_5 P2b — THE RULE READOUT. Two shapes, one voice: a line under
    // the door bar (the sky's live rule, beside the buttons that cycle
    // it) and a line under each rule-scoped group's header (which rule
    // or regime THESE rows act in, and whether it is the live one —
    // the regime line since ATMOS_1b). Dim when the group is dormant, lit
    // when it is the rule in force — the operator
    // learns "nothing is moving because this rule is not on" at a glance,
    // which is the whole defect this phase exists to end.
    '#organ .rulenow{margin:-4px 0 8px;color:#7d8894;font-size:10px;' +
    'letter-spacing:.04em}' +
    '#organ .rulenow b{color:#cfe0ef;font-weight:400}' +
    '#organ .rulescope{margin:-2px 0 5px;font-size:10px;line-height:1.5;' +
    'color:#5b636d;letter-spacing:.03em}' +
    '#organ .rulescope.on{color:#8fb98f}' +
    '#organ .rulescope.off{color:#6b7480}' +
    // ── ORGAN_5 P4 — MINIMIZED IS A STATE, NOT A SCROLL ──────────────
    // The header becomes a row so the title and the minimize button share
    // one line; the button is 44x44 because that is the smallest target a
    // thumb hits reliably, and the panel is now used on a phone. Its GLYPH
    // is small — the target is generous, the mark is quiet.
    '#organ .head{display:flex;align-items:center;gap:6px;margin:2px 0 10px}' +
    '#organ .head h1{margin:0;flex:1 1 auto;overflow:hidden;' +
    'text-overflow:ellipsis;white-space:nowrap}' +
    '#organ .min{flex:0 0 auto;width:44px;height:44px;margin:-14px -6px -14px 0;' +
    'background:none;border:0;color:#7d8894;font:inherit;font-size:15px;' +
    'line-height:1;cursor:pointer;padding:0}' +
    '#organ .min:hover{color:#cfe0ef}' +
    // THE PILL IS THE PHONE'S BACKTICK. A touch device has no key to press,
    // so with ?organ=1 this is the way in and out. Bottom-right, clear of
    // the panel's own edge, and 44px tall for the same reason.
    '#organ-pill{position:fixed;right:10px;bottom:10px;z-index:9999;' +
    'min-width:64px;height:44px;padding:0 14px;' +
    'background:#0d0f12;color:#8fa3b8;border:1px solid #2c3a46;' +
    'font:11px/44px ui-monospace,Menlo,Consolas,monospace;letter-spacing:.14em;' +
    'cursor:pointer;display:none}' +
    '#organ-pill.on{display:block}' +
    '#organ-pill:hover{border-color:#5c93c4;color:#cfe0ef}';

  var C = null;                 // the cwrap'd ABI
  var rows = [];                // {p, apply(values)} per manifest entry
  var MANIFEST = null;          // ORGAN_4 P6 — bound at boot, panel or not
  var rowsById = {};            // id -> row, filled by build() when it runs
  // LENS_1 — the regime lens. `lens` is 'live', 'all' or a regime index;
  // `regimeKin` maps "<group sans Regime N>|<label>" to the rows that are
  // the same dial in every regime (filled by build()); `lensAll` is the
  // fan-out switch push() reads. Session state, never storage.
  var lens = 'live';
  var lensAll = false;
  var regimeKin = {};
  var importNote = '';
  var touched = {};             // manifest index -> this session has written it
  var CLASS = ['free', 'event', 'frame'];   // organ_contest's three readings
  var definitionMode = true;    // O1b — the durable write is the default one

  function clamp(v, p) { return v < p.min ? p.min : v > p.max ? p.max : v; }
  // %.4g, so a meter reads like the manifest's own numbers. JS has no printf:
  // C picks the exponential form at exponent < -4 or >= the precision, then
  // strips the trailing zeros from whichever form it chose. Spelled out.
  function g4(x) {
    if (!isFinite(x)) return String(x);
    if (x === 0) return '0';
    var e = Math.floor(Math.log10(Math.abs(x)));
    var s = (e < -4 || e >= 4) ? x.toExponential(3) : x.toPrecision(4);
    var cut = s.indexOf('e');
    var man = cut < 0 ? s : s.slice(0, cut);
    if (man.indexOf('.') >= 0) man = man.replace(/0+$/, '').replace(/\.$/, '');
    if (cut < 0) return man;
    var ex = s.slice(cut + 1);                       // "+5" / "-07"
    var sg = ex.charAt(0);
    var dg = ex.slice(1);
    while (dg.length < 2) dg = '0' + dg;             // C pads the exponent
    return man + 'e' + sg + dg;
  }
  function hex(v) {
    var b = function (f) {
      var n = Math.round(Math.max(0, Math.min(1, f)) * 255).toString(16);
      return n.length < 2 ? '0' + n : n;
    };
    return '#' + b(v[0]) + b(v[1]) + b(v[2]);
  }
  function unhex(s) {
    return [parseInt(s.substr(1, 2), 16) / 255,
            parseInt(s.substr(3, 2), 16) / 255,
            parseInt(s.substr(5, 2), 16) / 255];
  }
  // TARGET: -1 writes the instance, a mood id writes that mood's
  // definition. A dial with no definition falls back to the instance in the
  // C++ — the panel does not need to know which, and asking here would put
  // the routing in two places.
  function push(p, v) {
    // The write is also the question the contest instrument answers: until
    // the panel has said something, there is nothing for another author to
    // contradict. A definition write is NOT that question — it never
    // touches the instance — so only the preview path marks the dial.
    //
    // A DEFINITION-ONLY DIAL HAS NO PREVIEW (ORGAN_2b, block NONE): there
    // is no instance for one to show, and −1 would only ring the reject
    // counter. It targets the live mood whatever the toggle says.
    var target = (!p.inst || definitionMode) ? C.mood() : -1;
    // ORGAN_6 — MARK ON THE TARGET, NOT ON THE MODE. The contest question
    // is asked by an INSTANCE write and by nothing else: a definition
    // write never touches the home, so there is nothing for another author
    // to contradict. These were two statements of one rule and the second
    // was wrong for a definition-only row under preview — it claimed a
    // reading for a write that had just been refused. `target < 0` IS the
    // instance write, so the two can no longer disagree.
    // A witness is never marked either: organ_set refuses it (ORGAN_2a).
    if (target < 0 && !p.ro) touched[p.i] = 1;
    C.set(p.block, p.offset, p.type, v[0] || 0, v[1] || 0, v[2] || 0, v[3] || 0,
          target);
    fanRegimes(p, v);   // LENS_1 — under ALL, every regime
  }
  function pushDef(p, v, mood) {
    C.set(p.block, p.offset, p.type, v[0] || 0, v[1] || 0, v[2] || 0, v[3] || 0,
          mood);
  }

  // ── LENS_1 — KIN: the same dial in every regime, found by NAME ────
  // A regime row's siblings are the rows whose group is the same with
  // "Regime N" struck out and whose label is the same. Derived from the
  // group string and the label — the shell's one permitted kind of
  // knowledge about a dial (ORGAN_5 P2b, ATMOS_1b) — never from the id.
  function kinKey(p) {
    var m = /^(.*)\bRegime \d+$/.exec(p.group || '');
    return m ? (m[1] + '|' + p.label) : null;
  }
  // Under ALL, a write to a regime row lands in the same row of every
  // other regime: the same definition write, the same mood, the
  // sibling's own manifest index. setDef shows the sibling too, so the
  // hidden rows' caches stay true and the ≠ marker clears on the next
  // tick because the kin now agree.
  function fanRegimes(p, v) {
    if (!lensAll) return;
    var k = kinKey(p); if (!k) return;
    var kin = regimeKin[k] || [], mood = C.mood();
    kin.forEach(function (s) {
      if (s.p.id !== p.id) s.setDef(mood, v.slice());
    });
  }

  // Every row is built the same way at the end: show() moves the widgets,
  // apply() moves them and writes wherever the mode points, setDef() writes
  // one named mood's definition and only shows it when that mood is live.
  // ORGAN_3b P4 — `nodes` is every element this row put in the section body
  // (a VEC3 is a header plus one line per lane), so the filter hides a row
  // by hiding what it built rather than by guessing at the DOM's shape.
  function finish(r, nodes) {
    r.nodes = nodes || [];
    r.apply = function (nv) { r.show(nv); push(r.p, nv); };
    r.setDef = function (mood, nv) {
      pushDef(r.p, nv, mood);
      if (mood === C.mood()) r.show(nv);
    };
    return r;
  }

  function buildRow(p, host) {
    var n = lanes(p.type), v = p.v.slice();
    var nodes = [];
    var add = function (el) { nodes.push(el); host.appendChild(el); return el; };

    // ── ORGAN_3c P0a — LINE 1: the label, and the markers pinned right ──
    var hd = document.createElement('div'); hd.className = 'hd';
    var lbl = document.createElement('span'); lbl.className = 'lbl';
    lbl.textContent = p.label;
    // THE HOVER ANSWERS WHAT THE ELLIPSIS HID. A truncated label without a
    // title is a name the panel took away; with one it is a name the panel
    // is holding for you. The id rides along because the label alone does
    // not say which home a stop lives in.
    lbl.title = p.label + '\n' + p.id;
    if (p.scope && !p.ro) {
      var star = document.createElement('span'); star.className = 'star';
      star.textContent = ' *';
      star.title = 'has a mood definition';
      lbl.appendChild(star);
    }
    hd.appendChild(lbl);
    var mk = document.createElement('span'); mk.className = 'mk';
    mk.textContent = '\u00b7';
    mk.title = 'contest: does the panel\u2019s last word on this dial still stand?';

    // ORGAN_3b — the cadence chip rides beside the contest marker, because
    // the two answer the operator's two questions about one row: WHEN does
    // my edit land, and DOES it still stand.
    var cad = null;
    if (CAD[p.cad]) {
      cad = document.createElement('span');
      cad.className = 'cad ' + ['live', 'gen', 'boundary', 'driven'][p.cad];
      cad.textContent = CAD[p.cad];
      cad.title = ['',
        'the author\u2019s next natural event applies this \u2014 a spawn, a world init. ' +
        'Dragging it and seeing nothing IS the dial working.',
        'a re-speak at the frame boundary applies this \u2014 it lands within a frame.',
        'a per-frame author writes this; the row is a meter, not a dial.'][p.cad];
    }
    // Called once per row, after any arm-specific header content (the
    // colour swatch) so the markers stay in their own columns.
    var closeHead = function () {
      hd.appendChild(mk);
      if (cad) hd.appendChild(cad);
      add(hd);
    };
    // ── LINE 2: one per control. A VEC row makes one per lane. ──
    var line = function () {
      var ln = document.createElement('div'); ln.className = 'ln';
      return add(ln);
    };

    // ORGAN_2a — A WITNESS IS A METER, NOT A DIAL. A driven value carries no
    // input of any kind: the dials that move it are its driver's, enrolled
    // above it in the same group. The 250 ms loop below fills this span, so
    // the operator watches the driven value breathe beside the rests and
    // gains that shape it. No star either — a witness has no definition to
    // write, and organ_set would refuse the write anyway. The meter sits in
    // the VALUE column, so a witness reads down the same edge as a dial.
    if (p.ro) {
      closeHead();
      var meter = document.createElement('span'); meter.className = 'ro';
      line().appendChild(meter);
      return finish({ p: p, mk: mk, ro: meter,
               show: function (nv) { v = nv.slice(); },
               read: function () { return v; } }, nodes);
    }

    if (p.type === BOOL) {
      closeHead();
      var cb = document.createElement('input'); cb.type = 'checkbox';
      cb.checked = v[0] > 0.5;
      cb.addEventListener('input', function () { v[0] = cb.checked ? 1 : 0; push(p, v); });
      line().appendChild(cb);
      return finish({ p: p, mk: mk,
               show: function (nv) { v = nv.slice(); cb.checked = v[0] > 0.5; },
               read: function () { return v; } }, nodes);
    }

    if (n > 1) {
      // A colour when the range says so. The swatch sits on the LABEL line
      // beside the markers (ORGAN_3c) and the fine sliders stack below it,
      // full width, because a colour input alone cannot be nudged one step.
      var col = document.createElement('input'); col.type = 'color';
      var isCol = (p.min === 0 && p.max === 1);
      if (isCol) {
        col.value = hex(v);
        col.addEventListener('input', function () {
          var c = unhex(col.value);
          v[0] = c[0]; v[1] = c[1]; v[2] = c[2];
          push(p, v); sync();
        });
        hd.appendChild(col);
      }
      closeHead();
      var sliders = [];
      for (var i = 0; i < n; i++) (function (li) {
        var ln = line();
        var sl2 = document.createElement('input'); sl2.type = 'range';
        sl2.min = p.min; sl2.max = p.max; sl2.step = p.step; sl2.value = v[li];
        var num = document.createElement('input'); num.type = 'number';
        num.min = p.min; num.max = p.max; num.step = p.step; num.value = v[li];
        sl2.title = num.title = p.label + ' \u2014 lane ' + li;
        var on = function (src) {
          return function () {
            v[li] = clamp(parseFloat(src.value) || 0, p);
            push(p, v); sync();
          };
        };
        sl2.addEventListener('input', on(sl2));
        num.addEventListener('input', on(num));
        ln.appendChild(sl2); ln.appendChild(num);
        sliders.push({ s: sl2, num: num });
      })(i);
      var sync = function () {
        for (var k = 0; k < n; k++) { sliders[k].s.value = v[k]; sliders[k].num.value = v[k]; }
        if (isCol) col.value = hex(v);
      };
      return finish({ p: p, mk: mk,
               show: function (nv) { v = nv.slice(); sync(); },
               read: function () { return v; } }, nodes);
    }

    closeHead();
    var ln1 = line();
    var sl = document.createElement('input'); sl.type = 'range';
    sl.min = p.min; sl.max = p.max; sl.step = p.step; sl.value = v[0];
    var nm = document.createElement('input'); nm.type = 'number';
    nm.min = p.min; nm.max = p.max; nm.step = p.step; nm.value = v[0];
    // ── ORGAN_5 P5c — AN EMPTY BOX IS NOT A VALUE ───────────────────
    //
    // The old form was `clamp(parseFloat(src.value) || 0, p)`, and it
    // committed p.min for an EMPTY field: '' parses NaN, `|| 0` makes it
    // 0, and clamp snaps to the floor. Clearing a number box before
    // typing is the ordinary way to enter a value on a touch keyboard, so
    // the FIRST keystroke of every phone edit wrote the minimum. On
    // `count` (min 0) that is count = 0, and configure_orbs early-returns
    // on `os.count == 0` — the dome goes dark mid-edit and every later
    // keystroke lands on a dead sky. This is the most economical
    // explanation the tree can give for "count works on desktop but not
    // his phone", and the harness reproduces the C++ half natively.
    //
    // A non-numeric box now commits NOTHING and the program keeps its
    // last good value.
    //
    // AND THE HANDLER NO LONGER WRITES BACK INTO THE ELEMENT UNDER THE
    // FINGER. Reassigning a focused number input on every keystroke
    // fights the caret on touch keyboards; the OTHER widget is synced, so
    // the slider still follows the box and the box still follows the
    // slider, but neither overwrites the one being used.
    //
    // TWO COMMIT PATHS, ONE PUSH. `input` is the live one — the drag and
    // the keystroke — and `change` is the settle: it fires on blur and on
    // Enter, and it NORMALISES the box to the value that actually landed,
    // so a clamp becomes visible the moment the hand lets go instead of
    // silently disagreeing with the program.
    var commit = function (src) {
      return function () {
        var raw = parseFloat(src.value);
        if (!isFinite(raw)) return;      // an empty box is not a value
        v[0] = clamp(raw, p);
        push(p, v);
        if (src !== sl) sl.value = v[0];
        if (src !== nm) nm.value = v[0];
      };
    };
    sl.addEventListener('input', commit(sl));
    nm.addEventListener('input', commit(nm));
    nm.addEventListener('change', function () {
      commit(nm)();
      nm.value = v[0];                   // the settle: show what landed
    });
    ln1.appendChild(sl); ln1.appendChild(nm);
    return finish({ p: p, mk: mk,
             show: function (nv) { v = nv.slice(); sl.value = v[0]; nm.value = v[0]; },
             read: function () { return v; } }, nodes);
  }

  // ── ORGAN_4 P6 — THE PRESET LAYER ──────────────────────────────────
  //
  // A SCENE IS A FILE, A BOOT IS A CHOICE. `web/presets/index.json` lists
  // what is on the shelf; `?preset=<name>` picks one at boot and the
  // panel's select picks one by hand. Both walk the SAME import path — a
  // partial file applies exactly what it carries, definitions raise their
  // flags and instances write and mark — so ZERO new write machinery
  // exists here and none was needed.
  //
  // IT LIVES AT MODULE SCOPE, NOT INSIDE build(), and that is the whole
  // point: an exhibition boots with `?preset=` and NO panel. A preset
  // reachable only from an open panel would be an instrument feature
  // wearing an exhibition's name.
  //
  // NO STORAGE OF ANY KIND. The artifact rule, and the same law the width
  // and the openMap already follow: a dev instrument that remembers is a
  // dev instrument that surprises. A preset is chosen per boot or per
  // click, and the URL is the only thing that carries a choice between
  // sessions.
  //
  // AN UNKNOWN ID IS NOT AN ERROR. The import path counts it and the
  // status line names the count — the standing behaviour, which is why a
  // preset cut against another build degrades instead of failing.

  // THE IMPORT WALK, LIFTED OUT OF THE FILE READER. It reads the MANIFEST
  // rather than the panel's rows, so it works with the panel closed; when
  // a row does exist, the widget is moved too, exactly as `r.apply` and
  // `r.setDef` would have.
  function applyFile(obj, what) {
    var applied = 0, skipped = 0, witnesses = 0;
    if (!MANIFEST) { importNote = what + ': the registry is not bound'; return importNote; }
    var byId = {};
    MANIFEST.forEach(function (p) { byId[p.id] = p; });
    // IMPORT KNOWS WHAT EXPORT KNOWS (ORGAN_2b): a witness is not a
    // setting. A file cut before ORGAN_2a still carries the four driven
    // values by id; sending them would only be refused in the C++, so they
    // are counted and named instead of rung up as rejections.
    Object.keys(obj).forEach(function (k) {
      var cut = k.indexOf('/');
      var scope = cut > 0 ? k.slice(0, cut) : null;
      var id = cut > 0 ? k.slice(cut + 1) : k;
      var p = byId[id], r = rowsById[id];
      if (!p)     { skipped++;   return; }   // an id this build does not enroll
      if (p.ro)   { witnesses++; return; }   // a meter is not a setting
      var v = [].concat(obj[k]);
      if (scope === null) {
        if (r) r.show(v);
        push(p, v);
        applied++;
        return;
      }
      // "world/<id>" is the world's bank, which belongs to no mood: the
      // live mood is sent and the C++ kind lets it fall away.
      var mood = (scope === 'world') ? C.mood() : parseInt(scope, 10);
      if (p.scope && mood >= 0) {
        pushDef(p, v, mood);
        if (r && mood === C.mood()) r.show(v);
        applied++;
      } else skipped++;                      // no definition behind that dial
    });
    importNote = what + ': ' + applied + ' applied, ' + skipped + ' unknown'
               + (witnesses ? ', skipped ' + witnesses + ' witnesses' : '');
    return importNote;
  }

  // The index, fetched ONCE however many doors ask for it.
  var shelfPromise = null;
  function shelf() {
    if (shelfPromise) return shelfPromise;
    shelfPromise = fetch('presets/index.json')
      .then(function (res) {
        if (!res.ok) throw new Error(res.status + ' ' + res.statusText);
        return res.json();
      })
      .then(function (list) {
        var byName = {};
        if (Array.isArray(list)) list.forEach(function (e) {
          if (e && e.name && e.file) byName[e.name] = e;
        });
        return byName;
      })
      .catch(function (e) {
        // No shelf is not a defect: presets are a layer, not a dependency.
        console.log('[ORGAN] no preset index (' + e.message + ')');
        return {};
      });
    return shelfPromise;
  }

  function loadPreset(entry) {
    return fetch('presets/' + entry.file)
      .then(function (res) {
        if (!res.ok) throw new Error(res.status + ' ' + res.statusText);
        return res.json();
      })
      .then(function (obj) {
        console.log('[ORGAN] preset "' + entry.name + '" \u2014 '
                    + applyFile(obj, 'preset ' + entry.name));
      })
      .catch(function (e) {
        importNote = 'preset ' + entry.name + ': ' + e.message;
        console.log('[ORGAN] preset "' + entry.name + '" did not load: ' + e.message);
      });
  }

  function build(manifest) {
    var st = document.createElement('style'); st.textContent = CSS;
    document.head.appendChild(st);

    var root = document.createElement('div'); root.id = 'organ';
    var head = document.createElement('div'); head.className = 'head';
    var h = document.createElement('h1');
    h.textContent = 'ORGAN \u2014 ` toggles';
    h.title = 'the backtick minimizes and restores; on a touch device the '
            + 'button beside this does, and the pill brings it back';
    var minBtn = document.createElement('button');
    minBtn.className = 'min';
    minBtn.textContent = '\u2014';          // an em dash: the panel, folded
    minBtn.title = 'minimize to a pill (or press `)';
    head.appendChild(h); head.appendChild(minBtn);
    root.appendChild(head);

    var bar = document.createElement('div'); bar.className = 'bar';
    var bd = document.createElement('button'); bd.textContent = 'definition';
    var bp = document.createElement('button'); bp.textContent = 'preview';
    bd.title = 'write what the live mood MEANS — the edit survives a mood change';
    bp.title = 'write the instance — immediate, and the next author may take it back';
    var setMode = function (def) {
      definitionMode = def;
      bd.className = def ? 'on' : '';
      bp.className = def ? '' : 'on';
    };
    bd.addEventListener('click', function () { setMode(true); });
    bp.addEventListener('click', function () { setMode(false); });
    setMode(true);
    // ── ORGAN_3b — THE DOOR STRIP ────────────────────────────────────
    // One button per door the build carries, read from the program rather
    // than named here: the shell stays name-blind about doors exactly as
    // it is about dials. No confirmation dialogs — the labels carry the
    // warning, and this is an instrument for an operator, not a consumer
    // UI. Presses coalesce in the C++ bitmask, so a double-click is one
    // raise.
    // ORGAN_5 P2b — declared HERE and not beside the section loop: the
    // door bar below assigns ruleNow, and a `var` further down would be
    // hoisted and then overwrite it with null when execution reached it.
    var ruleScopes = [];      // {el, rule} per rule-scoped group
    var regimeScopes = [];    // {el, regime} per regime-scoped group (ATMOS_1b/2)
    var ruleNow = null;       // the line under the door bar
    var ruleNowVal = null;    // its <b>, the only part that changes
    var domeLine = null;      // ORGAN_6 — the dome's own line, beside it

    var doorRoster = [];
    try { doorRoster = JSON.parse(C.doors()); } catch (e) { doorRoster = []; }

    var bx = document.createElement('button'); bx.textContent = 'export';
    var bi = document.createElement('button'); bi.textContent = 'import';
    bar.appendChild(bd); bar.appendChild(bp);
    bar.appendChild(bx); bar.appendChild(bi); root.appendChild(bar);

    if (doorRoster.length) {
      var doorBar = document.createElement('div'); doorBar.className = 'bar doors';
      doorRoster.forEach(function (d) {
        var b = document.createElement('button');
        b.textContent = d.l;
        b.title = 'a door presses the program\u2019s own frame boundary \u2014 ' +
                  'it adds no author';
        b.addEventListener('click', function () {
          C.door(d.i);
          // ORGAN_5 P2b — a door is consumed at the NEXT frame boundary,
          // so the window it moves is one frame behind this click. 60ms
          // beats the 250ms poll to the operator's eye without racing the
          // program: at 60fps the boundary has run three times by then,
          // and on a 15fps phone it has run once. Every door refreshes,
          // name-blind — RESPEAK moves no rule and the refresh is a
          // no-op, which is cheaper than asking which door this is.
          setTimeout(refreshRule, 60);
        });
        doorBar.appendChild(b);
      });
      root.appendChild(doorBar);
      // ORGAN_5 P2b — the live rule, under the bar that cycles it. NOT on
      // the buttons themselves: which door is the rule door is a C++
      // NUMBER, and a shell that hardcoded it would silently move this
      // readout onto the wrong button the day the roster is renumbered.
      // Under the bar the association is unambiguous and stays so.
      ruleNow = document.createElement('div');
      ruleNow.className = 'rulenow';
      // Built once; the refresh moves TEXT and never nodes. At four
      // refreshes a second, replacing children would be churn for a line
      // that changes on a keypress.
      var rnLbl = document.createElement('span'); rnLbl.textContent = 'now: ';
      ruleNowVal = document.createElement('b');
      ruleNow.appendChild(rnLbl); ruleNow.appendChild(ruleNowVal);
      ruleNow.title = 'the sky\u2019s motion rule and the active rule\u2019s '
                    + 'gesture \u2014 both PLAYER-OWNED (KP_8 / KP_7 or the '
                    + 'doors above); the panel reads them, it does not own them';
      root.appendChild(ruleNow);

      // ── ORGAN_6 — IS THE DOME LIT? ───────────────────────────────────
      // configure_orbs early-returns on !active || count == 0, so with the
      // dome dark every other orb dial is inert and the panel said nothing —
      // the operator drags a cohesion radius against an empty sky and reads
      // it as a broken panel. That is the defect the rule readout and the
      // regime readout each answer; this is the third place that needed one.
      // It rides beside the rule readout rather than under a group, because
      // deriving WHICH groups are orb groups would be a fourth name-string
      // in a file whose whole argument is that it holds none.
      domeLine = document.createElement('div');
      domeLine.className = 'rulescope';
      domeLine.title = 'configure_orbs returns early on enabled 0 or count 0, '
                     + 'so with the dome dark every other orb dial is inert \u2014 '
                     + 'the dial is not broken, the sky is empty';
      root.appendChild(domeLine);
    }

    // ── ATMOS_1 — THE MOOD DOOR: a door with a parameter ──────────────
    // Which mood is the program's to say: names from organ_mood_names(),
    // id = index, so a new mood appears here with zero JS edits. The
    // press walks request_mood_transition (keys 5-9's own door) at the
    // next boundary. Editing a non-live mood's definition-only rows needs
    // you IN it — this select is the road there.
    var moodSel = null;
    if (C.goMood && C.moodNames) {
      var names = [];
      try { names = JSON.parse(C.moodNames()); } catch (e) { names = []; }
      if (names.length) {
        var goBar = document.createElement('div'); goBar.className = 'bar doors';
        moodSel = document.createElement('select'); moodSel.className = 'mood';
        names.forEach(function (n, i) {
          var o = document.createElement('option'); o.value = String(i); o.textContent = n; moodSel.appendChild(o);
        });
        moodSel.value = String(C.mood());
        var go = document.createElement('button'); go.textContent = 'enter mood';
        go.title = 'request a transition to the selected mood \u2014 the same door keys 5-9 press; ignored while one is in flight';
        go.addEventListener('click', function () { C.goMood(parseInt(moodSel.value, 10)); });
        goBar.appendChild(moodSel); goBar.appendChild(go);
        root.appendChild(goBar);
      }
    }

    // ── ORGAN_3b P4a — THE FILTER ────────────────────────────────────
    // The manifest is a library, not a page. One field, matched against
    // id + label + group lowercased, so the operator can reach a stop by
    // any of the three names it already has. No debounce: this is a dev
    // instrument and a keystroke's worth of work is one substring test
    // per row (audit/ORGAN.md carries the count).
    var find = document.createElement('input');
    find.type = 'text'; find.className = 'find';
    find.placeholder = 'filter \u2014 id, label or section';
    root.appendChild(find);

    // ── ORGAN_3: the group string is a PATH ──────────────────────────
    // "Section · Group". The first token is the operator's VOICE and becomes
    // a collapsible block; the remainder is the group header inside it, as
    // before. A group with no separator is a section of its own — the shell
    // stays name-blind either way, and Jean renames by editing group strings.
    // Two levels, never three: only the FIRST separator splits.
    var group = null, section = null, host = root, count = 0, tally = null;
    var secs = [], cur = null, curGroup = null;
    var filtering = false;    // true while a needle is in the field
    var regimeCount = 0;      // LENS_1 — read from the group names, not declared
    var openMap = {};         // section name -> the operator's own choice
    var width = W_DEF;        // ORGAN_3c P0b — the hand's width, same law:
                              // a session variable, never storage
    manifest.forEach(function (p, i) {
      p.i = i;   // the manifest is emitted in registry order: index IS the key
      var cut = p.group.indexOf(SEP);
      var sec = cut < 0 ? p.group : p.group.slice(0, cut);
      var grp = cut < 0 ? null    : p.group.slice(cut + SEP.length);
      if (sec !== section) {
        section = sec; group = null; curGroup = null;
        var det = document.createElement('details'); det.className = 'sec';
        det.open = false;      // ORGAN_3b P4b — the panel opens as a table of
                               // contents; the filter and the hand open it
        var sum = document.createElement('summary'); sum.textContent = sec;
        tally = document.createElement('span'); tally.className = 'n';
        count = 0; sum.appendChild(tally);
        // ORGAN_3b P4c — a voice is a file. The section's own export writes
        // only its rows; import needs nothing new, because a partial file
        // has always applied exactly what it carries.
        var sx = document.createElement('button'); sx.className = 'sx';
        sx.textContent = '\u2913';
        sx.title = 'export this section alone \u2014 imports back as what it carries';
        (function (name) {
          sx.addEventListener('click', function (e) {
            if (e && e.preventDefault) e.preventDefault();
            if (e && e.stopPropagation) e.stopPropagation();
            download(collect(function (r) { return r.sec.name === name; }),
                     'organ-' + slug(name) + '.json');
          });
        })(sec);
        sum.appendChild(sx);
        det.appendChild(sum);
        host = document.createElement('div'); host.className = 'body';
        det.appendChild(host); root.appendChild(det);
        cur = { name: sec, det: det, tally: tally, rows: [], groups: [] };
        secs.push(cur);
        // ORGAN_3b P4b — the operator's own choice, remembered for the
        // session only. The filter opens what it finds; when the filter
        // clears, this map is what the panel goes back to, so a search
        // never silently rearranges the desk.
        (function (s2, d2) {
          d2.addEventListener('toggle', function () {
            if (!filtering) openMap[s2] = !!d2.open;
          });
        })(sec, det);
      }
      count++; tally.textContent = '  ' + count;
      if (grp !== null && grp !== group) {
        group = grp;
        var h2 = document.createElement('h2'); h2.textContent = grp;
        host.appendChild(h2);
        curGroup = { h2: h2, rows: [] };
        cur.groups.push(curGroup);
        // ORGAN_5 P2b — a group whose name ends "<rule> rule" says so,
        // and says whether that rule is the live one. Derived from the
        // group's own name, so renaming or adding a rule group in the
        // .inc needs no edit here.
        var gr = ruleOfGroup(grp);
        if (gr >= 0) {
          var rs = document.createElement('div');
          rs.className = 'rulescope';
          host.appendChild(rs);
          curGroup.scope = rs;     // the filter hides it with its header
          ruleScopes.push({ el: rs, rule: gr });
        }
        // ATMOS_1b/2 — the same line for a regime-scoped group. It wears
        // the rule line's dress: the class names a scope line, not a rule.
        var gg = regimeOfGroup(grp);
        if (gg >= 0) {
          var gs = document.createElement('div');
          gs.className = 'rulescope';
          host.appendChild(gs);
          curGroup.scope = gs;
          curGroup.regime = gg;                       // LENS_1 — the lens hides by this
          if (gg + 1 > regimeCount) regimeCount = gg + 1;
          regimeScopes.push({ el: gs, regime: gg });
        }
      }
      var r = buildRow(p, host);
      // The haystack is the three names a stop already answers to.
      r.hay = (p.id + ' ' + p.label + ' ' + p.group).toLowerCase();
      r.sec = cur; r.on = true;
      cur.rows.push(r);
      if (curGroup) curGroup.rows.push(r);
      rows.push(r);
      rowsById[p.id] = r;   // ORGAN_4 P6 — the preset road moves widgets too
      // LENS_1 — a row in a regime group carries its regime (−1 otherwise)
      // and joins its kin across regimes.
      r.regime = (curGroup && curGroup.regime !== undefined) ? curGroup.regime : -1;
      var kk = kinKey(p);
      if (kk) (regimeKin[kk] = regimeKin[kk] || []).push(r);
    });

    // ── ORGAN_3b P4a/P4b — the filter, applied ───────────────────────
    // A row hides when its haystack lacks the needle; a group header hides
    // when it has no visible row; a section hides when it has none either.
    // The section tally reads `hits/total` while filtering, so the operator
    // can see how much of a voice a word touches without opening it.
    function vis(node, on) { node.style.display = on ? '' : 'none'; }
    // LENS_1 — the lens is a second predicate, AND-ed with the needle.
    // A row outside any regime group is always admitted. LIVE and ALL
    // admit the regime this world is drawn into; a number admits that
    // regime, live or dormant — the scope line says which.
    function lensAdmits(regime) {
      if (regime < 0) return true;
      if (lens === 'live' || lens === 'all') return regime === (C.regime ? C.regime() : 0);
      return regime === lens;
    }
    function applyFilter() {
      var q = (find.value || '').toLowerCase().trim();
      filtering = q.length > 0;
      secs.forEach(function (s2) {
        var live = 0;
        s2.rows.forEach(function (r) {
          r.on = (!filtering || r.hay.indexOf(q) >= 0) && lensAdmits(r.regime);
          r.nodes.forEach(function (n) { vis(n, r.on); });
          if (r.on) live++;
        });
        s2.groups.forEach(function (g) {
          var any = false;
          g.rows.forEach(function (r) { if (r.on) any = true; });
          vis(g.h2, any);
          // ORGAN_5 P2b — a scope line — rule or tier — is part of its
          // header, so it hides and shows with it. A scope line over no
          // rows would be an answer to a question the filter just took
          // away.
          if (g.scope) vis(g.scope, any);
        });
        vis(s2.det, live > 0);
        s2.det.open = filtering ? live > 0 : !!openMap[s2.name];
        // LENS_1 — shown/total whenever they differ, whichever instrument
        // hid the rest: a section that says 73 while showing 25 is a lie.
        s2.tally.textContent = '  ' +
          (live !== s2.rows.length ? live + '/' + s2.rows.length : String(s2.rows.length));
      });
    }
    find.addEventListener('input', applyFilter);

    // ── LENS_1 — THE REGIME LENS ─────────────────────────────────────
    // A regime is an AXIS, not a group: four groups of twelve rows are one
    // set of twelve looked at four ways. The lens picks the way. LIVE
    // follows the world's draw; a number is the operator's word; ALL
    // shows the live regime's rows and fans a write to every regime. It
    // is built after the loop because the count is read from the group
    // names (regimeOfGroup), not declared here. Beside the mood select
    // when there is one: the two selects are the panel's two axes.
    var lensSel = null;
    if (regimeCount > 0 && C.regime) {
      lensSel = document.createElement('select'); lensSel.className = 'mood';
      var addOpt = function (value, text) {
        var o = document.createElement('option'); o.value = value; o.textContent = text; lensSel.appendChild(o);
      };
      addOpt('live', 'regime: this world\u2019s');
      for (var ri = 0; ri < regimeCount; ri++) addOpt(String(ri), 'regime ' + (ri + 1));
      addOpt('all', 'all regimes \u2014 write to every one');
      lensSel.value = 'live';
      lensSel.title = 'which regime\u2019s rows to show. "this world\u2019s" follows the draw; '
                    + 'a number shows that regime whether or not this world is in it (the scope '
                    + 'line says); "all" shows this world\u2019s rows and a write there lands in '
                    + 'every regime \u2014 a \u2260 on a label means the regimes disagree.';
      lensSel.addEventListener('change', function () {
        var val = lensSel.value;
        lens = (val === 'live' || val === 'all') ? val : parseInt(val, 10);
        lensAll = (val === 'all');
        applyFilter();
        refreshRegime();
      });
      var lensHost = goBar || (function () {
        var b = document.createElement('div'); b.className = 'bar doors';
        root.insertBefore(b, find); return b;
      })();
      lensHost.appendChild(lensSel);
    }
    applyFilter();   // the lens's default (LIVE) takes effect before the first paint

    var foot = document.createElement('div'); foot.className = 'foot';
    var status = document.createElement('div');
    var legend = document.createElement('div'); legend.className = 'legend';
    legend.textContent =
      'contest: free = the panel\u2019s word stands  \u00b7  ' +
      'event = lost on an occasion  \u00b7  frame = lost at once  ' +
      '(n = frames it stood). Only PREVIEW writes ask the question \u2014 a ' +
      'definition write never touches the instance, so a starred dial reads ' +
      '\u00b7 until you drag it in preview.';
    foot.appendChild(status); foot.appendChild(legend);
    root.appendChild(foot);
    document.body.appendChild(root);

    // ── ORGAN_3c P0b — THE RESIZE ────────────────────────────────────
    // A grip in the panel's own gutter, pointer events, vanilla. Width
    // clamps to [W_MIN, wMax()] — W_MIN computed from the grid's own
    // fixed parts (D2), so every width inside the clamp is a width the
    // grid can lay out without hiding anything. Narrow squeezes the
    // sliders toward their floor; wide gives them the room. Double-click
    // is home. The width lives beside openMap and dies with the session,
    // because a dev instrument that remembers is a dev instrument that
    // surprises.
    var grip = document.createElement('div'); grip.className = 'grip';
    root.appendChild(grip);
    function setWidth(w) {
      var max = wMax();
      width = w < W_MIN ? W_MIN : (w > max ? max : w);
      root.style.width = width + 'px';
      // Flush INSIDE the border, in the gutter the left padding reserves:
      // right edge under the border, left edge --grip-w further in. Both
      // terms come from the same table the stylesheet renders from.
      grip.style.right = (width - G.border - G.grip) + 'px';
    }
    setWidth(width);
    var drag = null;
    grip.addEventListener('pointerdown', function (e) {
      drag = { x: e.clientX, w: width };
      grip.className = 'grip drag';
      if (grip.setPointerCapture && e.pointerId !== undefined)
        grip.setPointerCapture(e.pointerId);
      if (e.preventDefault) e.preventDefault();
    });
    grip.addEventListener('pointermove', function (e) {
      if (!drag) return;
      setWidth(drag.w + (drag.x - e.clientX));   // the panel is on the right
    });
    var endDrag = function () { drag = null; grip.className = 'grip'; };
    grip.addEventListener('pointerup', endDrag);
    grip.addEventListener('pointercancel', endDrag);
    grip.addEventListener('dblclick', function (e) {
      if (e && e.preventDefault) e.preventDefault();
      setWidth(W_DEF);
    });
    window.addEventListener('resize', function () { setWidth(width); });

    // ── export / import ──────────────────────────────────────────────
    // A DEFINITION BELONGS TO A MOOD, so its key names one: "<mood>/<id>".
    // An instance value has no mood and keys by id alone. One file can
    // therefore carry several moods' definitions, and importing it puts
    // each back where it came from rather than into whichever mood happens
    // to be live at the time.
    //
    // A WORLD DEFINITION BELONGS TO NO MOOD (ORGAN_2b), so it keys
    // "world/<id>" and appears once. Reading it still goes through defGet:
    // the C++ switch sends a TIER entry to the one bank and lets the mood
    // argument fall on the floor, so the panel needs no second reader.
    //
    // ORGAN_3b P4c — one walk, an optional predicate. A section export is
    // the same walk narrowed, so witnesses stay skipped and the world/mood
    // keying stays identical: a partial file is a real file, not a dialect.
    function collect(pred) {
      var m = C.mood();
      var out = {};
      rows.forEach(function (r) {
        if (r.p.ro) return;   // witnesses export nothing: a meter is not a setting (ORGAN_2a)
        if (pred && !pred(r)) return;
        if (r.p.scope) {
          var n = lanes(r.p.type), d = [];
          for (var l = 0; l < n; l++) d.push(C.defGet(r.p.i, m, l));
          out[(r.p.scope === 2 ? 'world' : m) + '/' + r.p.id] = d;
        } else {
          out[r.p.id] = r.read();
        }
      });
      return out;
    }
    function slug(name) {
      return name.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
    }
    function download(out, name) {
      var blob = new Blob([JSON.stringify(out, null, 1)], { type: 'application/json' });
      var a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = name;
      a.click();
      URL.revokeObjectURL(a.href);
    }
    bx.addEventListener('click', function () { download(collect(null), 'organ.json'); });
    bi.addEventListener('click', function () {
      var f = document.createElement('input'); f.type = 'file'; f.accept = '.json,application/json';
      f.addEventListener('change', function () {
        if (!f.files || !f.files[0]) return;
        var rd = new FileReader();
        rd.onload = function () {
          var obj;
          try { obj = JSON.parse(rd.result); }
          catch (e) { importNote = 'import: not JSON'; return; }
          applyFile(obj, 'import');
        };
        rd.readAsText(f.files[0]);
      });
      f.click();
    });

    // ── ORGAN_4 P6b — THE SHELF, IN THE HEADER ───────────────────────
    // A small select beside export/import, filled from the same index the
    // boot reads. Choosing walks the same road `?preset=` walks, which is
    // the same road `import` walks — one write path, three doors onto it.
    var sel = document.createElement('select');
    sel.className = 'preset';
    sel.title = 'apply a preset \u2014 the same import path, so a partial file '
              + 'applies exactly what it carries';
    var opt0 = document.createElement('option');
    opt0.value = ''; opt0.textContent = 'preset\u2026';
    sel.appendChild(opt0);
    bar.appendChild(sel);

    shelf().then(function (byName) {
      var names = Object.keys(byName);
      if (!names.length) { sel.style.display = 'none'; return; }
      names.forEach(function (n) {
        var o = document.createElement('option');
        o.value = n; o.textContent = n;
        if (byName[n].note) o.title = byName[n].note;
        sel.appendChild(o);
      });
      sel.addEventListener('change', function () {
        var e = byName[sel.value];
        sel.value = '';                 // the select is a verb, not a state
        if (e) loadPreset(e);
      });
    });

    // ── ORGAN_5 P2b — THE RULE READOUT, refreshed from the window ────
    // One reader, two shapes. `organ_orb_rule()` packs `rule | gesture<<8`
    // — one ABI call, because the operator reads a rule and its gesture
    // as one fact and two calls could disagree between them.
    //
    // A rule the shell has no name for prints its NUMBER rather than
    // guessing: the four names are a ruled duplication (D3), and a
    // duplication that has fallen behind should say so, not invent.
    function ruleName(r) {
      return RULE_NAMES[r] || ('rule ' + r);
    }
    function refreshRule() {
      var packed = C.orbRule();
      var rule = packed & 0xFF, gesture = (packed >> 8) & 0xFF;
      if (ruleNowVal) {
        ruleNowVal.textContent = ruleName(rule) + ' \u00b7 gesture ' + gesture;
      }
      if (domeLine) {
        var lit = (packed >> 16) & 1, motes = (packed >> 17) & 0x1FF;
        domeLine.className = 'rulescope ' + (lit && motes ? 'on' : 'off');
        domeLine.textContent = (lit && motes)
          ? '\u25b8 the dome is lit \u2014 ' + motes + ' motes'
          : '\u25b8 the dome is DARK ('
            + (!lit ? 'enabled 0' : 'count 0')
            + ') \u2014 every orb dial below is inert until it is lit';
      }
      ruleScopes.forEach(function (rs) {
        var on = rs.rule === rule;
        rs.el.className = 'rulescope ' + (on ? 'on' : 'off');
        rs.el.textContent = on
          ? '\u25b8 these rows are acting NOW \u2014 live rule: '
            + ruleName(rule) + ' \u00b7 gesture ' + gesture
          : '\u25b8 these rows act in the ' + RULE_NAMES[rs.rule].toUpperCase()
            + ' rule \u2014 live rule: ' + ruleName(rule);
        rs.el.title = on
          ? 'the live rule IS this group\u2019s rule: turning these moves the sky'
          : 'the live rule is ' + ruleName(rule) + ', so these rows are dormant. '
            + 'Turning one and seeing nothing is the RULE, not a dead dial \u2014 '
            + 'cycle to ' + RULE_NAMES[rs.rule] + ' to hear them.';
      });
    }
    refreshRule();

    // ── ATMOS_1b/2 — THE REGIME READOUT, the rule readout's law again ─
    // A dial whose effect depends on a mode stands next to a truthful
    // readout of that mode. The fifty-two regime rows are mode-scoped: a
    // world is drawn into ONE regime by its seed, and the other regimes'
    // centres and spreads move nothing here until a world lands in them.
    // organ_regime() reads the spine's own field through the pointer
    // organ_mood() follows; the text shows the LABEL's number, never the
    // index.
    var lastLensRegime = -1;   // LENS_1 — the regime the lens last followed
    function refreshRegime() {
      if (!C.regime) return;
      var g = C.regime();
      // LENS_1 — the lens follows the draw at LIVE and ALL: a transition
      // or a weight edit that re-rolls moves the shown groups on this tick.
      if (g !== lastLensRegime) {
        lastLensRegime = g;
        if (lens === 'live' || lens === 'all') applyFilter();
      }
      regimeScopes.forEach(function (gs) {
        var on = gs.regime === g;
        gs.el.className = 'rulescope ' + (on ? 'on' : 'off');
        gs.el.textContent = (on && lensAll)
          ? '\u25b8 ALL \u2014 showing regime ' + (g + 1) + ', this world\u2019s; '
            + 'a write here lands in every regime'
          : on
          ? '\u25b8 this world is drawn into regime ' + (g + 1)
            + ' \u2014 these rows act NOW'
          : '\u25b8 these rows act in worlds drawn into regime ' + (gs.regime + 1)
            + ' \u2014 this world is in regime ' + (g + 1);
        gs.el.title = on
          ? 'the seed drew this regime: turning these moves the sky at the next boundary'
          : 'the seed drew regime ' + (g + 1) + ', and RESPEAK keeps the seed, so the '
            + 'centres and spreads here move nothing until a world lands in regime '
            + (gs.regime + 1) + ' (?seed= pins which). The weight rows \u2014 Atmosphere '
            + '\u00b7 Regimes, above \u2014 are the exception: they move where this '
            + 'world\u2019s roll lands, and can bring this world here.';
      });

      // LENS_1 — ≠: under ALL the shown row carries the live regime's
      // value; if its kin disagree, the label says so and the hover lists
      // them. Read from the rows' caches (kept true by show/setDef and the
      // follow-the-mood refresh), so this is arithmetic, not ABI calls.
      Object.keys(regimeKin).forEach(function (k) {
        var kin = regimeKin[k];
        kin.forEach(function (r) {
          var hd = r.nodes[0]; if (!hd) return;
          if (!lensAll || !r.on) { hd.classList.remove('kin'); hd.title = ''; return; }
          var mine = r.read(), differ = false, lines = [];
          kin.forEach(function (s) {
            var theirs = s.read();
            var same = theirs.length === mine.length && theirs.every(function (x, i) { return x === mine[i]; });
            if (!same) differ = true;
            lines.push('regime ' + (s.regime + 1) + ': ' + theirs.map(g4).join(', '));
          });
          hd.classList.toggle('kin', differ);
          hd.title = differ ? 'the regimes disagree \u2014 a write here makes them agree\n' + lines.join('\n') : '';
        });
      });
    }
    refreshRegime();

    // ── the panel carries its own witnesses ──────────────────────────
    // ATMOS_1 — and it follows the MOOD. A mood-selected definition row
    // shows the LIVE mood's definition, so a transition (this select, a
    // key, a portal) leaves it showing the mood the panel was built in.
    // The manifest's `scope` is the one home for "is this row
    // mood-selected", so TIER and BEHAVIOR — one bank, the target ignored
    // — are correctly left alone. r.show() reassigns each row's cached
    // lane vector as
    // well as its widgets, so the next drag starts from the shown value
    // with nothing further to update. Instance rows refresh here too
    // (ATMOS_1b): the apply re-authors the sun's homes at every entry,
    // and a panel that is a VIEW shows what the program wrote.
    var lastMood = C.mood();
    setInterval(function () {
      var contested = 0;
      var m = C.mood();
      if (m !== lastMood) {
        lastMood = m;
        if (moodSel) moodSel.value = String(m);
        rows.forEach(function (r) {
          if (r.p.ro) return;                    // witnesses refresh every tick below
          if (r.p.scope) {
            if (r.p.scope === 2) return;         // TIER/BEHAVIOR: one bank, the mood ignored
            var n = lanes(r.p.type), d = [];     // a mood-selected definition: the new mood's
            for (var l = 0; l < n; l++) d.push(C.defGet(r.p.i, m, l));
            r.show(d);
          } else {
            // An instance: the apply may have re-authored its home, so
            // read the HOME, not the row's cache. r.read() returns the
            // cached lane vector (`v`) — the same value already shown —
            // so it could not carry the new mood's numbers. C.get is the
            // reader the witness poll below already uses, and it reads
            // through the registry to the home itself (ATMOS_1b).
            var ni = lanes(r.p.type), iv = [];
            for (var li = 0; li < ni; li++) iv.push(C.get(r.p.i, li));
            r.show(iv);
          }
        });
      }
      rows.forEach(function (r) {
        // A witness reads the home itself, every tick — that IS the row.
        if (r.ro) {
          var n = lanes(r.p.type), out = [];
          for (var l = 0; l < n; l++) out.push(g4(C.get(r.p.i, l)));
          r.ro.textContent = out.join(' ');
        }
        var k = C.contest(r.p.i);
        if (k > 0) contested++;
        // An untouched dial reads as free because nothing has contradicted
        // it, which is not the same as knowing it holds. Say so with a dot
        // rather than claiming a reading the program was never asked for.
        r.mk.className = 'mk' + (touched[r.p.i] ? ' ' + CLASS[k] : '');
        r.mk.textContent = touched[r.p.i]
          ? CLASS[k] + ' ' + C.contestFrames(r.p.i)
          : '\u00b7';
      });
      refreshRule();
      refreshRegime();
      status.textContent = BUILD_TAG + rows.length + ' dials  ·  mood ' + C.mood() +
                           (C.regime ? '  ·  regime ' + (C.regime() + 1) : '') +
                           '  ·  ' + (definitionMode ? 'definition' : 'preview') +
                           '  ·  reconciled ' + C.flushes() +
                           '  ·  rejected ' + C.rejects() +
                           (C.rejects() ? '  ·  last: ' + C.lastReject() : '') +
                           '  ·  contested ' + contested + '/' + rows.length +
                           (importNote ? '  ·  ' + importNote : '');
    }, 250);

    // ── ORGAN_5 P4 — THE PILL, and the one state behind both doors ───
    //
    // MINIMIZED IS A STATE, NOT A SCROLL. `hidden` already existed and
    // the backtick already toggled it; what was missing was a way BACK on
    // a device with no backtick. The pill is that way, and it is the same
    // state — one variable, three ways to move it (the button, the pill,
    // the key), so they can never disagree about whether the panel is up.
    //
    // SESSION ONLY, like the width and the openMap: no storage of any
    // kind (the artifact rule). A dev instrument that remembers is a dev
    // instrument that surprises.
    //
    // The pill lives on <body> and not inside #organ, because #organ is
    // what gets hidden — a pill inside it would vanish with the thing it
    // is meant to bring back.
    var pill = document.createElement('div');
    pill.id = 'organ-pill';
    pill.textContent = 'organ';
    // ORGAN_5 P5b — the build id rides the pill's hover as well as the
    // footer: on a phone the footer is a scroll away and the pill never is.
    pill.title = 'restore the panel' + (BUILD_OK ? '  \u00b7  build ' + BUILD_ID : '');
    document.body.appendChild(pill);

    var minimized = false;
    // ONE STATE, and `hidden` is the only class #organ ever carries — so
    // an assignment is exact here and needs no classList dance.
    function setMinimized(on) {
      minimized = !!on;
      root.className = minimized ? 'hidden' : '';
      pill.className = minimized ? 'on' : '';
    }
    minBtn.addEventListener('click', function (e) {
      if (e && e.preventDefault) e.preventDefault();
      setMinimized(true);
    });
    pill.addEventListener('click', function (e) {
      if (e && e.preventDefault) e.preventDefault();
      setMinimized(false);
    });

    // ── backtick, and the canvas keeps its keys ──────────────────────
    window.addEventListener('keydown', function (e) {
      if (e.key === '`') { setMinimized(!minimized); e.preventDefault(); }
    });
    // Anything typed at the panel is the panel's; the world never hears it.
    ['keydown', 'keyup', 'keypress'].forEach(function (t) {
      root.addEventListener(t, function (e) { if (e.key !== '`') e.stopPropagation(); });
    });
  }

  // The program may still be compiling when this runs, and the boot path in
  // index.html is delicate — so wait by asking, rather than by hooking into
  // its choreography.
  var tries = 0;
  var wait = setInterval(function () {
    if (++tries > 1200) { clearInterval(wait); return; }   // ~10 min, then give up quietly
    var M = window.Module;
    if (!M || typeof M.cwrap !== 'function') return;
    var manifest;
    try {
      C = {
        manifest: M.cwrap('organ_manifest', 'string', []),
        set:      M.cwrap('organ_set', null, ['number','number','number','number','number','number','number','number']),
        rejects:  M.cwrap('organ_rejected_count', 'number', []),
        lastReject:    M.cwrap('organ_last_reject', 'string', []),
        flushes:  M.cwrap('organ_flush_count', 'number', []),
        count:    M.cwrap('organ_param_count', 'number', []),
        contest:       M.cwrap('organ_contest', 'number', ['number']),
        contestFrames: M.cwrap('organ_contest_frames', 'number', ['number']),
        mood:          M.cwrap('organ_mood', 'number', []),
        defGet:        M.cwrap('organ_def_get', 'number', ['number','number','number']),
        get:           M.cwrap('organ_get', 'number', ['number','number']),
        orbRule:       M.cwrap('organ_orb_rule', 'number', []),
        doors:         M.cwrap('organ_doors', 'string', []),
        door:          M.cwrap('organ_door', null, ['number']),
        goMood:        M.cwrap('organ_go_mood', null, ['number']),
        moodNames:     M.cwrap('organ_mood_names', 'string', []),
        regime:        M.cwrap('organ_regime', 'number', [])
      };
      if (C.count() <= 0) return;          // registry not bound yet
      manifest = JSON.parse(C.manifest());
    } catch (e) { return; }                 // not ready; ask again
    if (!manifest || !manifest.length) return;
    clearInterval(wait);
    MANIFEST = manifest;
    if (WANT_PANEL) {
      build(manifest);
      console.log('[ORGAN] panel up — ' + manifest.length + ' dials');
    }
    // ORGAN_4 P6 — THE BOOT CHOICE, read once from the URL and applied
    // through the same road the select and the import button walk. It runs
    // whether or not the panel is up, because an exhibition boots with the
    // scene and not the instrument.
    if (WANT_PRESET) {
      shelf().then(function (byName) {
        if (byName[WANT_PRESET]) return loadPreset(byName[WANT_PRESET]);
        console.log('[ORGAN] ?preset=' + WANT_PRESET
                    + ' is not in presets/index.json');
      });
    }
  }, 500);
})();
