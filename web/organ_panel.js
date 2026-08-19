// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE PANEL SHELL (ORGAN_0c)
//
// A VIEW of the program (docs/ORGAN.md). Every row here is drawn from the
// manifest the C++ registry emits, so this file knows no parameter names,
// no ranges and no offsets — enroll a dial in organ_params.inc and it
// appears; remove one and it stops. There is nothing to keep in step.
//
// ACCESS. Without ?organ=1 this file returns on its second line: no DOM, no
// stylesheet, no timer, no ccall. The audience path is byte-identical, and
// that is why the file may ship unconditionally.
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
  if (new URLSearchParams(location.search).get('organ') !== '1') return;

  var F32 = 0, U32 = 1, BOOL = 2, VEC3 = 3, VEC4 = 4;
  // The manifest's "def" column names the definition FAMILY: 0 none,
  // 1 a mood's meaning, 2 the world's tiers (ORGAN_2b), 3 the world's
  // behaviours (ORGAN_3). DEFONLY is the sentinel block of an entry that
  // has a definition and no instance at all.
  //
  // WHAT THE SHELL ACTUALLY NEEDS TO KNOW is not the family but its
  // SCOPE: a per-mood definition keys its export by mood, a world
  // definition keys it "world/" and appears once. isWorldDef is that
  // question, asked in one place so a fourth family answers it by being
  // added here rather than by being forgotten at three call sites.
  var DEF_MOOD = 1, DEF_TIER = 2, DEF_BEHAVIOR = 3, DEFONLY = 255;
  function isWorldDef(p) { return p.def === DEF_TIER || p.def === DEF_BEHAVIOR; }

  // ORGAN_3b — CADENCE. The manifest's "cad" says WHEN a stop sounds; the
  // C++ derives it, this file only prints it. Live is SILENT on purpose:
  // silence is the default state of a working dial, and a chip on every
  // row would be noise rather than information. The other three each
  // answer a question the operator would otherwise have to ask the source.
  var CAD = [null, 'on respawn', 'boundary', 'driven'];
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
  var W_MIN = 2 * G.pad + G.body + G.lblmin + 3 * G.hdgap
                        + G.sw + G.mk + G.chip;
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
    'position:fixed;top:0;right:0;width:' + px(W_DEF) + ';max-height:100vh;overflow-y:auto;' +
    'background:#0d0f12;color:#c8ccd2;font:11px/1.45 ui-monospace,Menlo,Consolas,monospace;' +
    'border-left:1px solid #262b33;z-index:9999;padding:8px ' + px(G.pad) + ' 14px}' +
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
    // ORGAN_3c P0b — the resize grip, on the panel's INNER edge.
    '#organ .grip{position:fixed;top:0;bottom:0;width:6px;cursor:col-resize;z-index:10000;' +
    'background:transparent;border-left:1px solid transparent}' +
    '#organ .grip:hover,#organ .grip.drag{border-left-color:#5c93c4}' +
    '#organ button{background:#14181d;color:#c8ccd2;border:1px solid #303742;font:inherit;' +
    'padding:3px 9px;cursor:pointer}' +
    '#organ button:hover{border-color:#5c93c4}' +
    '#organ button.on{background:#1d2a36;border-color:#5c93c4;color:#cfe0ef}' +
    '#organ .star{color:#5c93c4}' +
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
    '#organ details.sec>summary .sx:hover{color:#5c93c4}';

  var C = null;                 // the cwrap'd ABI
  var rows = [];                // {p, apply(values)} per manifest entry
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
    var target = (p.block === DEFONLY || definitionMode) ? C.mood() : -1;
    // A witness is never marked: organ_set refuses it, so the panel never got
    // to ask the contest question and must not claim a reading (ORGAN_2a).
    if (!(definitionMode && p.def) && !p.ro) touched[p.i] = 1;
    C.set(p.block, p.offset, p.type, v[0] || 0, v[1] || 0, v[2] || 0, v[3] || 0,
          target);
  }
  function pushDef(p, v, mood) {
    C.set(p.block, p.offset, p.type, v[0] || 0, v[1] || 0, v[2] || 0, v[3] || 0,
          mood);
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
    if (p.def && !p.ro) {
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
    var set = function (src) {
      return function () {
        v[0] = clamp(parseFloat(src.value) || 0, p);
        push(p, v); sl.value = v[0]; nm.value = v[0];
      };
    };
    sl.addEventListener('input', set(sl));
    nm.addEventListener('input', set(nm));
    ln1.appendChild(sl); ln1.appendChild(nm);
    return finish({ p: p, mk: mk,
             show: function (nv) { v = nv.slice(); sl.value = v[0]; nm.value = v[0]; },
             read: function () { return v; } }, nodes);
  }

  function build(manifest) {
    var st = document.createElement('style'); st.textContent = CSS;
    document.head.appendChild(st);

    var root = document.createElement('div'); root.id = 'organ';
    var h = document.createElement('h1'); h.textContent = 'ORGAN — ` to hide';
    root.appendChild(h);

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
        b.addEventListener('click', function () { C.door(d.i); });
        doorBar.appendChild(b);
      });
      root.appendChild(doorBar);
    }

    // ── ORGAN_3b P4a — THE FILTER ────────────────────────────────────
    // 262 rows are a library, not a page. One field, matched against
    // id + label + group lowercased, so the operator can reach a stop by
    // any of the three names it already has. No debounce: this is a dev
    // instrument and a keystroke's worth of work is 262 substring tests.
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
      }
      var r = buildRow(p, host);
      // The haystack is the three names a stop already answers to.
      r.hay = (p.id + ' ' + p.label + ' ' + p.group).toLowerCase();
      r.sec = cur; r.on = true;
      cur.rows.push(r);
      if (curGroup) curGroup.rows.push(r);
      rows.push(r);
    });

    // ── ORGAN_3b P4a/P4b — the filter, applied ───────────────────────
    // A row hides when its haystack lacks the needle; a group header hides
    // when it has no visible row; a section hides when it has none either.
    // The section tally reads `hits/total` while filtering, so the operator
    // can see how much of a voice a word touches without opening it.
    function vis(node, on) { node.style.display = on ? '' : 'none'; }
    function applyFilter() {
      var q = (find.value || '').toLowerCase().trim();
      filtering = q.length > 0;
      secs.forEach(function (s2) {
        var live = 0;
        s2.rows.forEach(function (r) {
          r.on = !filtering || r.hay.indexOf(q) >= 0;
          r.nodes.forEach(function (n) { vis(n, r.on); });
          if (r.on) live++;
        });
        s2.groups.forEach(function (g) {
          var any = false;
          g.rows.forEach(function (r) { if (r.on) any = true; });
          vis(g.h2, any);
        });
        vis(s2.det, live > 0);
        s2.det.open = filtering ? live > 0 : !!openMap[s2.name];
        s2.tally.textContent = '  ' +
          (filtering ? live + '/' + s2.rows.length : String(s2.rows.length));
      });
    }
    find.addEventListener('input', applyFilter);

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
    // A grip on the panel's INNER edge, pointer events, vanilla. Width
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
      grip.style.right = (width - 3) + 'px';   // straddles the edge
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
        if (r.p.def) {
          var n = lanes(r.p.type), d = [];
          for (var l = 0; l < n; l++) d.push(C.defGet(r.p.i, m, l));
          out[(isWorldDef(r.p) ? 'world' : m) + '/' + r.p.id] = d;
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
          var obj, applied = 0, skipped = 0, witnesses = 0;
          try { obj = JSON.parse(rd.result); }
          catch (e) { importNote = 'import: not JSON'; return; }
          var byId = {};
          rows.forEach(function (r) { byId[r.p.id] = r; });
          // IMPORT KNOWS WHAT EXPORT KNOWS (ORGAN_2b): a witness is not a
          // setting. A file cut before ORGAN_2a still carries the four
          // driven values by id; sending them would only be refused in the
          // C++, so they are counted and named instead of rung up as
          // rejections.
          Object.keys(obj).forEach(function (k) {
            var cut = k.indexOf('/');
            var scope = cut > 0 ? k.slice(0, cut) : null;
            var r = byId[cut > 0 ? k.slice(cut + 1) : k];
            if (!r)     { skipped++;   return; }   // an id this build does not enroll
            if (r.p.ro) { witnesses++; return; }   // a meter is not a setting
            if (scope === null) { r.apply([].concat(obj[k])); applied++; return; }
            // "world/<id>" is the world's bank, which belongs to no mood:
            // the live mood is sent and the C++ kind lets it fall away.
            var mood = (scope === 'world') ? C.mood() : parseInt(scope, 10);
            if (r.p.def && mood >= 0) { r.setDef(mood, [].concat(obj[k])); applied++; }
            else skipped++;                       // no definition behind that dial
          });
          importNote = 'import: ' + applied + ' applied, ' + skipped + ' unknown'
                     + (witnesses ? ', skipped ' + witnesses + ' witnesses' : '');
        };
        rd.readAsText(f.files[0]);
      });
      f.click();
    });

    // ── the panel carries its own witnesses ──────────────────────────
    setInterval(function () {
      var contested = 0;
      rows.forEach(function (r) {
        // A witness reads the home itself, every tick — that IS the row.
        if (r.ro) {
          var n = lanes(r.p.type), out = [];
          for (var l = 0; l < n; l++) out.push(g4(C.get(r.p.block, r.p.offset, l)));
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
      status.textContent = rows.length + ' dials  ·  mood ' + C.mood() +
                           '  ·  ' + (definitionMode ? 'definition' : 'preview') +
                           '  ·  reconciled ' + C.flushes() +
                           '  ·  rejected ' + C.rejects() +
                           '  ·  contested ' + contested + '/' + rows.length +
                           (importNote ? '  ·  ' + importNote : '');
    }, 250);

    // ── backtick, and the canvas keeps its keys ──────────────────────
    window.addEventListener('keydown', function (e) {
      if (e.key === '`') { root.classList.toggle('hidden'); e.preventDefault(); }
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
        flushes:  M.cwrap('organ_flush_count', 'number', []),
        count:    M.cwrap('organ_param_count', 'number', []),
        contest:       M.cwrap('organ_contest', 'number', ['number']),
        contestFrames: M.cwrap('organ_contest_frames', 'number', ['number']),
        mood:          M.cwrap('organ_mood', 'number', []),
        defGet:        M.cwrap('organ_def_get', 'number', ['number','number','number']),
        get:           M.cwrap('organ_get', 'number', ['number','number','number']),
        doors:         M.cwrap('organ_doors', 'string', []),
        door:          M.cwrap('organ_door', null, ['number'])
      };
      if (C.count() <= 0) return;          // registry not bound yet
      manifest = JSON.parse(C.manifest());
    } catch (e) { return; }                 // not ready; ask again
    if (!manifest || !manifest.length) return;
    clearInterval(wait);
    build(manifest);
    console.log('[ORGAN] panel up — ' + manifest.length + ' dials');
  }, 500);
})();
