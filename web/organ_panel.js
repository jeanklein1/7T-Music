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

  var CSS =
    '#organ{position:fixed;top:0;right:0;width:330px;max-height:100vh;overflow-y:auto;' +
    'background:#0d0f12;color:#c8ccd2;font:11px/1.45 ui-monospace,Menlo,Consolas,monospace;' +
    'border-left:1px solid #262b33;z-index:9999;padding:8px 10px 14px}' +
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
    '#organ .row{display:flex;align-items:center;gap:6px;margin:2px 0}' +
    '#organ .lbl{flex:0 0 118px;color:#96a0ab;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}' +
    '#organ input[type=range]{flex:1 1 auto;min-width:0;accent-color:#5c93c4;height:14px}' +
    '#organ input[type=number]{flex:0 0 62px;background:#14181d;color:#c8ccd2;border:1px solid #262b33;' +
    'font:inherit;padding:1px 3px}' +
    '#organ input[type=color]{flex:0 0 34px;height:18px;background:#14181d;border:1px solid #262b33;padding:0}' +
    '#organ .lane{flex:0 0 108px}' +
    '#organ .ro{flex:1 1 auto;text-align:right;color:#8fa3b8}' +
    // ORGAN_3b — the cadence chip. One rule, three modifiers, no colour
    // louder than the existing accent; the panel is an instrument.
    '#organ .cad{flex:0 0 auto;font-size:9px;letter-spacing:.06em;color:#4f5761;' +
    'border:1px solid #262b33;border-radius:2px;padding:0 3px;margin-left:2px}' +
    '#organ .cad.gen{color:#b0954e;border-color:#3a3226}' +
    '#organ .cad.boundary{color:#5c93c4;border-color:#24313d}' +
    '#organ .cad.driven{color:#8fa3b8;border-color:#262b33}' +
    '#organ .mk{flex:0 0 58px;text-align:right;font-size:10px;color:#4f5761}' +
    '#organ .mk.free{color:#5f8f6a}' +
    '#organ .mk.event{color:#b0954e}' +
    '#organ .mk.frame{color:#b0644e}' +
    '#organ .legend{color:#4f5761;margin-top:3px}' +
    '#organ .sub{margin-left:118px}' +
    '#organ button{background:#14181d;color:#c8ccd2;border:1px solid #303742;font:inherit;' +
    'padding:3px 9px;cursor:pointer}' +
    '#organ button:hover{border-color:#5c93c4}' +
    '#organ button.on{background:#1d2a36;border-color:#5c93c4;color:#cfe0ef}' +
    '#organ .star{color:#5c93c4}' +
    '#organ .foot{margin-top:12px;padding-top:6px;border-top:1px solid #1d222a;color:#6b7480}' +
    '#organ .bar{display:flex;gap:6px;margin-bottom:6px}';

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
  function finish(r) {
    r.apply = function (nv) { r.show(nv); push(r.p, nv); };
    r.setDef = function (mood, nv) {
      pushDef(r.p, nv, mood);
      if (mood === C.mood()) r.show(nv);
    };
    return r;
  }

  function buildRow(p, host) {
    var n = lanes(p.type), v = p.v.slice();
    var row = document.createElement('div'); row.className = 'row';
    var lbl = document.createElement('span'); lbl.className = 'lbl';
    lbl.textContent = p.label; lbl.title = p.id;
    if (p.def && !p.ro) {
      var star = document.createElement('span'); star.className = 'star';
      star.textContent = ' *';
      star.title = 'has a mood definition';
      lbl.appendChild(star);
    }
    row.appendChild(lbl);
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

    // ORGAN_2a — A WITNESS IS A METER, NOT A DIAL. A driven value carries no
    // input of any kind: the dials that move it are its driver's, enrolled
    // above it in the same group. The 250 ms loop below fills this span, so
    // the operator watches the driven value breathe beside the rests and
    // gains that shape it. No star either — a witness has no definition to
    // write, and organ_set would refuse the write anyway.
    if (p.ro) {
      var meter = document.createElement('span'); meter.className = 'ro';
      row.appendChild(meter); row.appendChild(mk);
      if (cad) row.appendChild(cad);
      host.appendChild(row);
      return finish({ p: p, mk: mk, ro: meter,
               show: function (nv) { v = nv.slice(); },
               read: function () { return v; } });
    }

    if (p.type === BOOL) {
      var cb = document.createElement('input'); cb.type = 'checkbox';
      cb.checked = v[0] > 0.5;
      cb.addEventListener('input', function () { v[0] = cb.checked ? 1 : 0; push(p, v); });
      row.appendChild(cb); row.appendChild(mk);
      if (cad) row.appendChild(cad);
      host.appendChild(row);
      return finish({ p: p, mk: mk,
               show: function (nv) { v = nv.slice(); cb.checked = v[0] > 0.5; },
               read: function () { return v; } });
    }

    if (n > 1) {
      // A colour when the range says so; the fine sliders sit under it
      // because a colour input alone cannot be nudged one step.
      var col = document.createElement('input'); col.type = 'color';
      var isCol = (p.min === 0 && p.max === 1);
      if (isCol) {
        col.value = hex(v);
        col.addEventListener('input', function () {
          var c = unhex(col.value);
          v[0] = c[0]; v[1] = c[1]; v[2] = c[2];
          push(p, v); sync();
        });
        row.appendChild(col);
      }
      row.appendChild(mk);
      if (cad) row.appendChild(cad);
      host.appendChild(row);
      var sliders = [];
      for (var i = 0; i < n; i++) (function (li) {
        var r2 = document.createElement('div'); r2.className = 'row sub';
        var s = document.createElement('input'); s.type = 'range'; s.className = 'lane';
        s.min = p.min; s.max = p.max; s.step = p.step; s.value = v[li];
        var num = document.createElement('input'); num.type = 'number';
        num.min = p.min; num.max = p.max; num.step = p.step; num.value = v[li];
        var on = function (src) {
          return function () {
            v[li] = clamp(parseFloat(src.value) || 0, p);
            push(p, v); sync();
          };
        };
        s.addEventListener('input', on(s));
        num.addEventListener('input', on(num));
        r2.appendChild(s); r2.appendChild(num); host.appendChild(r2);
        sliders.push({ s: s, num: num });
      })(i);
      var sync = function () {
        for (var k = 0; k < n; k++) { sliders[k].s.value = v[k]; sliders[k].num.value = v[k]; }
        if (isCol) col.value = hex(v);
      };
      return finish({ p: p, mk: mk,
               show: function (nv) { v = nv.slice(); sync(); },
               read: function () { return v; } });
    }

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
    row.appendChild(sl); row.appendChild(nm); row.appendChild(mk);
    if (cad) row.appendChild(cad);
    host.appendChild(row);
    return finish({ p: p, mk: mk,
             show: function (nv) { v = nv.slice(); sl.value = v[0]; nm.value = v[0]; },
             read: function () { return v; } });
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
    var bx = document.createElement('button'); bx.textContent = 'export';
    var bi = document.createElement('button'); bi.textContent = 'import';
    bar.appendChild(bd); bar.appendChild(bp);
    bar.appendChild(bx); bar.appendChild(bi); root.appendChild(bar);

    // ── ORGAN_3: the group string is a PATH ──────────────────────────
    // "Section · Group". The first token is the operator's VOICE and becomes
    // a collapsible block; the remainder is the group header inside it, as
    // before. A group with no separator is a section of its own — the shell
    // stays name-blind either way, and Jean renames by editing group strings.
    // Two levels, never three: only the FIRST separator splits.
    var group = null, section = null, host = root, count = 0, tally = null;
    manifest.forEach(function (p, i) {
      p.i = i;   // the manifest is emitted in registry order: index IS the key
      var cut = p.group.indexOf(SEP);
      var sec = cut < 0 ? p.group : p.group.slice(0, cut);
      var grp = cut < 0 ? null    : p.group.slice(cut + SEP.length);
      if (sec !== section) {
        section = sec; group = null;
        var det = document.createElement('details'); det.className = 'sec';
        var sum = document.createElement('summary'); sum.textContent = sec;
        tally = document.createElement('span'); tally.className = 'n';
        count = 0; sum.appendChild(tally);
        det.appendChild(sum);
        host = document.createElement('div'); host.className = 'body';
        det.appendChild(host); root.appendChild(det);
      }
      count++; tally.textContent = '  ' + count;
      if (grp !== null && grp !== group) {
        group = grp;
        var h2 = document.createElement('h2'); h2.textContent = grp;
        host.appendChild(h2);
      }
      rows.push(buildRow(p, host));
    });

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
    bx.addEventListener('click', function () {
      var m = C.mood();
      var out = {};
      rows.forEach(function (r) {
        if (r.p.ro) return;   // witnesses export nothing: a meter is not a setting (ORGAN_2a)
        if (r.p.def) {
          var n = lanes(r.p.type), d = [];
          for (var l = 0; l < n; l++) d.push(C.defGet(r.p.i, m, l));
          out[(isWorldDef(r.p) ? 'world' : m) + '/' + r.p.id] = d;
        } else {
          out[r.p.id] = r.read();
        }
      });
      var blob = new Blob([JSON.stringify(out, null, 1)], { type: 'application/json' });
      var a = document.createElement('a');
      a.href = URL.createObjectURL(blob);
      a.download = 'organ.json';
      a.click();
      URL.revokeObjectURL(a.href);
    });
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
        get:           M.cwrap('organ_get', 'number', ['number','number','number'])
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
