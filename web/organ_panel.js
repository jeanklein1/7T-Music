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
// AESTHETIC BOUND. This is an instrument, not the art. Monospace, one dark
// column, no animation, no branding. The art stays on the canvas.
// ═══════════════════════════════════════════════════════════════════════
(function () {
  'use strict';
  if (new URLSearchParams(location.search).get('organ') !== '1') return;

  var F32 = 0, U32 = 1, BOOL = 2, VEC3 = 3, VEC4 = 4;
  var lanes = function (t) { return t === VEC3 ? 3 : t === VEC4 ? 4 : 1; };

  var CSS =
    '#organ{position:fixed;top:0;right:0;width:330px;max-height:100vh;overflow-y:auto;' +
    'background:#0d0f12;color:#c8ccd2;font:11px/1.45 ui-monospace,Menlo,Consolas,monospace;' +
    'border-left:1px solid #262b33;z-index:9999;padding:8px 10px 14px}' +
    '#organ.hidden{display:none}' +
    '#organ h1{font-size:11px;letter-spacing:.14em;color:#7d8894;margin:2px 0 10px;font-weight:400}' +
    '#organ h2{font-size:10px;letter-spacing:.1em;color:#5c93c4;margin:12px 0 4px;font-weight:400;' +
    'border-bottom:1px solid #1d222a;padding-bottom:2px}' +
    '#organ .row{display:flex;align-items:center;gap:6px;margin:2px 0}' +
    '#organ .lbl{flex:0 0 118px;color:#96a0ab;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}' +
    '#organ input[type=range]{flex:1 1 auto;min-width:0;accent-color:#5c93c4;height:14px}' +
    '#organ input[type=number]{flex:0 0 62px;background:#14181d;color:#c8ccd2;border:1px solid #262b33;' +
    'font:inherit;padding:1px 3px}' +
    '#organ input[type=color]{flex:0 0 34px;height:18px;background:#14181d;border:1px solid #262b33;padding:0}' +
    '#organ .lane{flex:0 0 108px}' +
    '#organ .sub{margin-left:118px}' +
    '#organ button{background:#14181d;color:#c8ccd2;border:1px solid #303742;font:inherit;' +
    'padding:3px 9px;cursor:pointer}' +
    '#organ button:hover{border-color:#5c93c4}' +
    '#organ .foot{margin-top:12px;padding-top:6px;border-top:1px solid #1d222a;color:#6b7480}' +
    '#organ .bar{display:flex;gap:6px;margin-bottom:6px}';

  var C = null;                 // the cwrap'd ABI
  var rows = [];                // {p, apply(values)} per manifest entry
  var importNote = '';

  function clamp(v, p) { return v < p.min ? p.min : v > p.max ? p.max : v; }
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
  function push(p, v) {
    C.set(p.block, p.offset, p.type, v[0] || 0, v[1] || 0, v[2] || 0, v[3] || 0);
  }

  function buildRow(p, host) {
    var n = lanes(p.type), v = p.v.slice();
    var row = document.createElement('div'); row.className = 'row';
    var lbl = document.createElement('span'); lbl.className = 'lbl';
    lbl.textContent = p.label; lbl.title = p.id;
    row.appendChild(lbl);

    if (p.type === BOOL) {
      var cb = document.createElement('input'); cb.type = 'checkbox';
      cb.checked = v[0] > 0.5;
      cb.addEventListener('input', function () { v[0] = cb.checked ? 1 : 0; push(p, v); });
      row.appendChild(cb); host.appendChild(row);
      return { p: p, apply: function (nv) { v = nv.slice(); cb.checked = v[0] > 0.5; push(p, v); },
               read: function () { return v; } };
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
      return { p: p, apply: function (nv) { v = nv.slice(); push(p, v); sync(); },
               read: function () { return v; } };
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
    row.appendChild(sl); row.appendChild(nm); host.appendChild(row);
    return { p: p, apply: function (nv) { v = nv.slice(); sl.value = v[0]; nm.value = v[0]; push(p, v); },
             read: function () { return v; } };
  }

  function build(manifest) {
    var st = document.createElement('style'); st.textContent = CSS;
    document.head.appendChild(st);

    var root = document.createElement('div'); root.id = 'organ';
    var h = document.createElement('h1'); h.textContent = 'ORGAN — ` to hide';
    root.appendChild(h);

    var bar = document.createElement('div'); bar.className = 'bar';
    var bx = document.createElement('button'); bx.textContent = 'export';
    var bi = document.createElement('button'); bi.textContent = 'import';
    bar.appendChild(bx); bar.appendChild(bi); root.appendChild(bar);

    var group = null;
    manifest.forEach(function (p) {
      if (p.group !== group) {
        group = p.group;
        var h2 = document.createElement('h2'); h2.textContent = group;
        root.appendChild(h2);
      }
      rows.push(buildRow(p, root));
    });

    var foot = document.createElement('div'); foot.className = 'foot';
    root.appendChild(foot);
    document.body.appendChild(root);

    // ── export / import ──────────────────────────────────────────────
    bx.addEventListener('click', function () {
      var out = {};
      rows.forEach(function (r) { out[r.p.id] = r.read(); });
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
          var obj, applied = 0, skipped = 0;
          try { obj = JSON.parse(rd.result); }
          catch (e) { importNote = 'import: not JSON'; return; }
          var byId = {};
          rows.forEach(function (r) { byId[r.p.id] = r; });
          Object.keys(obj).forEach(function (k) {
            if (byId[k]) { byId[k].apply([].concat(obj[k])); applied++; }
            else skipped++;   // an id this build does not enroll
          });
          importNote = 'import: ' + applied + ' applied, ' + skipped + ' unknown';
        };
        rd.readAsText(f.files[0]);
      });
      f.click();
    });

    // ── the panel carries its own witnesses ──────────────────────────
    setInterval(function () {
      foot.textContent = rows.length + ' dials  ·  flushed ' + C.flushes() +
                         '  ·  rejected ' + C.rejects() +
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
        set:      M.cwrap('organ_set', null, ['number','number','number','number','number','number','number']),
        rejects:  M.cwrap('organ_rejected_count', 'number', []),
        flushes:  M.cwrap('organ_flush_count', 'number', []),
        count:    M.cwrap('organ_param_count', 'number', [])
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
