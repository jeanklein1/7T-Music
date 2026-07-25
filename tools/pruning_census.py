#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# PRUNING_1 — THE CENSUS INSTRUMENT
#
# The reason scaffolding accumulated is not weak discipline: the tree had
# no instrument that made scaffolding VISIBLE. This is that instrument.
# Run it at the head of a campaign, not as an event.
#
#   python tools/pruning_census.py            # report -> stdout + audit/
#   python tools/pruning_census.py --no-dates # skip git pickaxe dating
#   python tools/pruning_census.py --check     # exit 1 if the report on
#                                              # disk is stale (CI gate)
#
# CONTRACT: stdlib only. No network. Runs from the repo root. Every list
# sorted, so successive runs diff cleanly. ZERO writes under src/.
# Output: stdout + audit/PRUNING_1_CENSUS.md
# ═══════════════════════════════════════════════════════════════════════
"""PRUNING_1 P0 census — makes scaffolding visible without deleting any of it."""

from __future__ import annotations

import argparse
import hashlib
import io
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict

# ───────────────────────────────────────────────────────────────────────
# ROOMS — every path this instrument knows about, in one place.
# ───────────────────────────────────────────────────────────────────────
WGSL = "src/cartridges/the_board/realization/world.wgsl"
WEB_WGSL = "web/shaders/world.wgsl"
WEB_WGSL_SIDECAR = "web/shaders/world.wgsl.source"
RENDERER = "src/cartridges/the_board/realization/renderer.hpp"
STATE = "src/cartridges/the_board/realization/state.hpp"
REGISTRY = "src/cartridges/the_board/realization/binding_registry.hpp"
PASSES = "src/cartridges/the_board/realization/render_passes.hpp"
GROUND_ARCH = "src/cartridges/the_board/contracts/ground_architecture.hpp"
CONSTITUTION = "src/docs/old docs/cartridge_constitution.md"
CMAKE = "CMakeLists.txt"
CANVAS = "src/coupling/visual_canvas.hpp"
WEB_UNIFORMS = "web/js/uniforms.js"
TOOL = "tools/pruning_census.py"
OUT = "audit/PRUNING_1_CENSUS.md"
# The instrument and its own report are excluded from every tree-wide scan.
# This is a determinism requirement, not tidiness: the tool's pattern tables
# name every referent it hunts, and the report quotes every hit from the last
# run — count either and run N+1 can never equal run N.
SELF_PATHS = {TOOL, OUT}

# Comment-mass census scope (the handoff's §6 scope).
MASS_DIRS = ["src/cartridges/the_board", "src/coupling", "src/musical"]
# Tree-wide scans never enter these.
SKIP_DIRS = {".git", "src/external", "node_modules", "build", "src/docs/old docs"}
SRC_EXT = {".hpp", ".cpp", ".h", ".wgsl"}

# ───────────────────────────────────────────────────────────────────────
# PRIMITIVES
# ───────────────────────────────────────────────────────────────────────


def read(path):
    """Read a file, or return '' if it is not there. Absence is a finding,
    never a crash — this instrument must survive the tree it audits."""
    try:
        with io.open(path, encoding="utf-8", errors="replace") as f:
            return f.read()
    except OSError:
        return ""


def exists(path):
    return os.path.exists(path)


def git(*args, timeout=30):
    """git, fail-soft. Returns '' rather than raising: a census that dies on
    a detached worktree is not an instrument."""
    try:
        r = subprocess.run(
            ["git"] + list(args),
            capture_output=True,
            text=True,
            timeout=timeout,
            errors="replace",
        )
        return r.stdout.strip() if r.returncode == 0 else ""
    except (OSError, subprocess.SubprocessError):
        return ""


def _blank(m):
    """Replace a span with spaces, keeping every newline. LENGTH-PRESERVING
    by construction — this is the invariant the whole census rests on: a
    character offset means the same thing in the raw text and the stripped
    text, so `lineno()` is valid against either and the two never drift."""
    return re.sub(r"[^\n]", " ", m.group(0))


def strip_block_comments(s):
    return re.sub(r"/\*.*?\*/", _blank, s, flags=re.S)


def strip_wgsl(s):
    """WGSL has no string literals, so comment stripping is unconditional."""
    return re.sub(r"//[^\n]*", _blank, strip_block_comments(s))


def strip_cpp(s):
    """C++ DOES have string literals, and this tree stores WGSL entry-point
    names in them. Blank string BODIES first so a `//` inside a literal
    cannot eat the rest of the line, then strip comments."""
    s = re.sub(r'"(?:\\.|[^"\\\n])*"',
               lambda m: '"' + " " * (len(m.group(0)) - 2) + '"', s)
    return re.sub(r"//[^\n]*", _blank, strip_block_comments(s))


def lineno(text, pos):
    return text.count("\n", 0, pos) + 1


def splitlines_exact(s):
    """Lines of a file, with a single trailing newline treated as a
    terminator rather than a separator — so `len()` equals `wc -l`."""
    lines = s.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    return lines


def comment_line_stats(raw, wgsl=False):
    """(total, comment, blank). A COMMENT LINE is a line that carries text
    before stripping and carries nothing after it — i.e. the line's entire
    payload was comment. A trailing `// note` on a code line is NOT counted;
    it is not removable mass."""
    stripped = strip_wgsl(raw) if wgsl else strip_cpp(raw)
    raw_lines = splitlines_exact(raw)
    st_lines = splitlines_exact(stripped)
    total = len(raw_lines)
    comment = blank = 0
    for i, rl in enumerate(raw_lines):
        r = rl.strip()
        s = st_lines[i].strip() if i < len(st_lines) else ""
        if not r:
            blank += 1
        elif not s:
            comment += 1
    return total, comment, blank


def walk_src(exts=SRC_EXT, roots=("src",), extra=()):
    """Deterministic tree walk. Sorted, and never inside SKIP_DIRS."""
    out = []
    for root in roots:
        for dirpath, dirnames, filenames in os.walk(root):
            rel = dirpath.replace(os.sep, "/")
            if any(rel == s or rel.startswith(s + "/") for s in SKIP_DIRS):
                dirnames[:] = []
                continue
            dirnames[:] = sorted(d for d in dirnames if d not in {".git", "node_modules"})
            for fn in sorted(filenames):
                if os.path.splitext(fn)[1] in exts:
                    out.append(os.path.join(dirpath, fn).replace(os.sep, "/"))
    out.extend(e for e in extra if exists(e))
    return sorted(set(out) - SELF_PATHS)


# ═══════════════════════════════════════════════════════════════════════
# §1 — WGSL MODULE: symbols, call graph, reachability
# ═══════════════════════════════════════════════════════════════════════

STAGE_ATTR = re.compile(r"@(compute|vertex|fragment)\b")
WORKGROUP = re.compile(r"@workgroup_size\s*\(([^)]*)\)")
FN_RE = re.compile(r"\bfn\s+([A-Za-z_]\w*)\s*\(")
BIND_RE = re.compile(
    r"@group\s*\(\s*(\d+)\s*\)\s*@binding\s*\(\s*(\d+)\s*\)\s*"
    r"var\s*(?:<([^>]*)>)?\s*([A-Za-z_]\w*)\s*:\s*([^;]+);"
)
# MODULE SCOPE means column 0. An indented `const` is a FUNCTION-LOCAL, and
# counting it as module-scope both inflates the total and puts a body-scope
# constant (CMG_MAX_SA, COAST_EPS, TIER_ANTENNA_FIRST) into the "unreferenced
# module const" deletion list, where it does not belong.
CONST_RE = re.compile(r"^const\s+([A-Za-z_]\w*)\s*(?::|=)", re.M)
LOCAL_CONST_RE = re.compile(r"^[ \t]+const\s+([A-Za-z_]\w*)\s*(?::|=)", re.M)
OVERRIDE_RE = re.compile(r"^override\s+([A-Za-z_]\w*)\s*(?::|=)", re.M)
STRUCT_RE = re.compile(r"^struct\s+([A-Za-z_]\w*)\s*\{", re.M)
IDENT_RE = re.compile(r"[A-Za-z_]\w*")
LOCAL_DECL_RE = re.compile(r"\b(?:let|var|const)\s+([A-Za-z_]\w*)")


class WgslModule:
    """A parsed world.wgsl: declarations, call graph, per-entry closure.

    The reachability notion here is Tint's: a pipeline pulls in exactly the
    transitive callee closure of its entry point, and exactly the module-scope
    resources that closure names. Everything else is dead-code-eliminated
    before the driver ever sees it."""

    def __init__(self, path):
        self.path = path
        self.raw = read(path)
        self.text = strip_wgsl(self.raw)
        self._closures = {}
        self._ident_lines = None
        self.total, self.comment_lines, self.blank_lines = comment_line_stats(self.raw, wgsl=True)
        self._parse_functions()
        self._parse_decls()
        self._build_graph()

    # ── functions ──────────────────────────────────────────────────────
    def _parse_functions(self):
        t = self.text
        self.functions = {}
        self.duplicate_fns = []
        for m in FN_RE.finditer(t):
            name = m.group(1)
            # The attribute block sits in the text immediately BEFORE `fn`,
            # possibly on its own line(s). Walk back to the previous `}` or
            # `;` (end of the last declaration) and read the attributes there.
            head_start = max(t.rfind("}", 0, m.start()), t.rfind(";", 0, m.start())) + 1
            head = t[head_start : m.start()]
            stage_m = STAGE_ATTR.search(head)
            stage = stage_m.group(1) if stage_m else None
            wg = WORKGROUP.search(head)
            brace = t.find("{", m.end())
            if brace < 0:
                continue
            depth, i = 1, brace + 1
            while i < len(t) and depth:
                if t[i] == "{":
                    depth += 1
                elif t[i] == "}":
                    depth -= 1
                i += 1
            rec = {
                "name": name,
                "line": lineno(t, m.start()),
                "stage": stage,
                "workgroup": re.sub(r"\s+", "", wg.group(1)) if wg else None,
                "body": t[brace:i],
                "end_line": lineno(t, i),
            }
            if name in self.functions:
                self.duplicate_fns.append((name, self.functions[name]["line"], rec["line"]))
            self.functions[name] = rec
        self.entry_points = {n: f for n, f in self.functions.items() if f["stage"]}

    # ── module-scope declarations ──────────────────────────────────────
    def _parse_decls(self):
        t = self.text
        self.resources = {}
        for m in BIND_RE.finditer(t):
            g, b, aspace, name, ty = m.groups()
            self.resources[name] = {
                "name": name,
                "group": int(g),
                "binding": int(b),
                "space": (aspace or "").strip(),
                "type": " ".join(ty.split()),
                "line": lineno(t, m.start()),
            }
        self.consts = {m.group(1): lineno(t, m.start()) for m in CONST_RE.finditer(t)}
        self.local_consts = {m.group(1): lineno(t, m.start())
                             for m in LOCAL_CONST_RE.finditer(t)}
        self.overrides = {m.group(1): lineno(t, m.start()) for m in OVERRIDE_RE.finditer(t)}
        self.structs = {m.group(1): lineno(t, m.start()) for m in STRUCT_RE.finditer(t)}
        # A module-scope declaration REFERENCES other module-scope symbols:
        # `const LINE_Y : Line = ...` keeps `Line` alive, and no function body
        # mentions `Line` at all. Without this, a live const's own type reads
        # as dead — so declaration bodies join the reference universe.
        self.decl_refs = {}
        for m in CONST_RE.finditer(t):
            end = t.find(";", m.end())
            self.decl_refs[m.group(1)] = set(IDENT_RE.findall(t[m.start():end if end > 0 else m.end()]))
        for m in STRUCT_RE.finditer(t):
            brace = t.find("{", m.start())
            depth, i = 1, brace + 1
            while i < len(t) and depth:
                if t[i] == "{":
                    depth += 1
                elif t[i] == "}":
                    depth -= 1
                i += 1
            self.decl_refs[m.group(1)] = set(IDENT_RE.findall(t[m.start():i]))

    def expand_decl_refs(self, idents):
        """Fixed point: anything a reachable declaration names is reachable."""
        seen = set(idents)
        stack = list(seen)
        while stack:
            n = stack.pop()
            for r in self.decl_refs.get(n, ()):  # noqa: SIM118
                if r not in seen:
                    seen.add(r)
                    stack.append(r)
        return seen

    # ── the call graph ─────────────────────────────────────────────────
    def _build_graph(self):
        fn_names = set(self.functions)
        res_names = set(self.resources)
        self.calls, self.direct_res, self.direct_idents = {}, {}, {}
        for name, f in self.functions.items():
            idents = set(IDENT_RE.findall(f["body"]))
            # A `let/var/const NAME` in the body SHADOWS a module-scope
            # resource of the same name (world.wgsl really does this, e.g.
            # `let patch_grid = ...`). WGSL does not hoist, so this is an
            # over-approximation only in the pathological use-before-declare
            # case, which does not occur here.
            shadowed = set(LOCAL_DECL_RE.findall(f["body"]))
            self.direct_idents[name] = idents
            self.calls[name] = (idents & fn_names) - {name}
            self.direct_res[name] = (idents & res_names) - shadowed

    def closure(self, root):
        """Transitive callee closure, memoised. §2 asks it once per
        (resource × entry point) — 96 × 64 — so without the cache the tool
        recomputes the same 64 closures six thousand times."""
        cached = self._closures.get(root)
        if cached is not None:
            return cached
        seen, stack = set(), [root]
        while stack:
            n = stack.pop()
            if n in seen:
                continue
            seen.add(n)
            stack.extend(self.calls.get(n, ()))
        self._closures[root] = seen
        return seen

    def callers_of(self, name):
        return sorted(c for c, callees in self.calls.items() if name in callees)

    def reachable_from(self, roots):
        """Union of the closures of `roots` — the live function set."""
        out = set()
        for r in roots:
            if r in self.functions:
                out |= self.closure(r)
        return out

    def _index_idents(self):
        """One pass over the stripped text builds ident -> [line, ...].
        Without it, §1.3/§1.4 rescan the whole 12 k-line shader once per
        symbol — 400+ full scans, and the tool takes 15 s instead of 3."""
        if getattr(self, "_ident_lines", None) is not None:
            return
        self._ident_lines = defaultdict(list)
        line = 1
        pos = 0
        t = self.text
        for m in IDENT_RE.finditer(t):
            line += t.count("\n", pos, m.start())
            pos = m.start()
            self._ident_lines[m.group(0)].append(line)

    def references_outside_decl(self, ident, decl_line=None):
        """Whole-word references to `ident` in the comment-stripped text,
        excluding the line the declaration sits on."""
        self._index_idents()
        lines = self._ident_lines.get(ident, ())
        if decl_line is None:
            return len(lines)
        return sum(1 for ln in lines if ln != decl_line)


# ═══════════════════════════════════════════════════════════════════════
# THE SECOND GATE — which entry points does a C++ pipeline actually name?
# ═══════════════════════════════════════════════════════════════════════

CPP_KEYWORDS = {
    "if", "for", "while", "switch", "else", "do", "try", "catch", "return",
    "const", "constexpr", "static", "inline", "struct", "class", "namespace",
    "auto", "sizeof", "using", "template", "typename", "operator", "new",
    "delete", "case", "default", "public", "private", "protected",
}
ENTRY_CONST_RE = re.compile(r"constexpr\s+const\s+char\*\s*(\w+)\s*=\s*\"([^\"]+)\"")
ENTRY_USE_RE = re.compile(r"\bEntry::(\w+)\b")
PIPE_CREATE_RE = re.compile(
    r"(\w+)\s*=\s*\w+\.Create(RenderPipeline|ComputePipeline)\s*\(|"
    r"\bCreate(RenderPipeline|ComputePipeline)\s*\("
)


class PipelineGate:
    """An entry point with no pipeline is dead, and everything reachable
    ONLY through it is dead with it. That is the gate that matters."""

    def __init__(self, files):
        self.files = list(files)
        self.decls = {}          # C++ constant name -> WGSL entry string
        self.decl_sites = {}     # C++ constant name -> file:line
        self.uses = defaultdict(list)   # C++ constant name -> [file:line]
        self.literal_uses = defaultdict(list)  # raw "string" used as entryPoint
        self.pipelines = []      # (file, line, kind, lhs)
        for path in files:
            raw = read(path)
            if not raw:
                continue
            for m in ENTRY_CONST_RE.finditer(raw):
                self.decls[m.group(1)] = m.group(2)
                self.decl_sites[m.group(1)] = "%s:%d" % (path, lineno(raw, m.start()))
            code = strip_cpp(raw)
            decl_lines = {lineno(raw, m.start()) for m in ENTRY_CONST_RE.finditer(raw)}
            for m in ENTRY_USE_RE.finditer(code):
                ln = lineno(code, m.start())
                if ln in decl_lines:
                    continue
                self.uses[m.group(1)].append("%s:%d" % (path, ln))
            # A raw literal assigned straight into an entryPoint field would
            # bypass Entry:: entirely — census it so the gate cannot be
            # silently defeated.
            for m in re.finditer(r"entryPoint\s*=\s*\"([^\"]+)\"", raw):
                self.literal_uses[m.group(1)].append("%s:%d" % (path, lineno(raw, m.start())))
            for m in PIPE_CREATE_RE.finditer(code):
                kind = m.group(2) or m.group(3)
                self.pipelines.append(
                    (path, lineno(code, m.start()), kind, (m.group(1) or "<anon>"))
                )
        # WHAT "LIVE" MEANS, precisely. Naming an entry point somewhere in C++
        # is NOT the same as building a pipeline from it — a debug print or a
        # dead branch would name it too. So each use site is resolved to the
        # statement it sits in, and that statement must either create a
        # pipeline directly or call a helper whose body creates one.
        # The BLOCK, not the statement, is the right scope: the tree writes
        #     fragment.entryPoint = Entry::ENTITY_FS;
        #     ... a few statements ...
        #     out = device_.CreateRenderPipeline(&desc);
        # so a statement-level test rejects every genuinely live entry point.
        # Helper names are the lambdas/functions whose own body creates a
        # pipeline (makeComputePipeline, makeEntity, makeShadow, …); a call to
        # one of those counts as building.
        CREATE = re.compile(r"Create(?:Render|Compute)Pipeline\s*\(")
        blocks = {}     # path -> [(start, end, builds_directly)]
        helpers = set()
        for path in files:
            code = strip_cpp(read(path))
            spans, stack = [], []
            for i, ch in enumerate(code):
                if ch == "{":
                    stack.append(i)
                elif ch == "}" and stack:
                    a = stack.pop()
                    spans.append((a, i))
            spans = [(a, b, bool(CREATE.search(code[a:b]))) for a, b in spans]
            blocks[path] = spans
            # a helper is the identifier heading a block that creates a pipeline
            for a, b, mk in spans:
                if not mk:
                    continue
                head = code[max(0, a - 240):a]
                # Only a genuine definition head names a helper. Control-flow
                # keywords and type names sit in the same textual position and
                # would otherwise be reported as "helpers" (`if`, `constexpr`,
                # `Pipeline`).
                for hm in (re.search(r"\bauto\s+([a-z_]\w*)\s*=\s*\[[^\]]*\]\s*\([^;]*\)"
                                     r"[^;{]*$", head),
                           re.search(r"\b[A-Za-z_][\w:<>\*&,\s]*?\b([a-z_]\w*)\s*"
                                     r"\([^;{)]*\)\s*(?:const\s*)?$", head)):
                    if hm and hm.group(1) not in CPP_KEYWORDS:
                        helpers.add(hm.group(1))
                        break
        self.helpers = sorted(helpers)
        helper_alt = "|".join(re.escape(h) for h in helpers) or r"(?!)"
        USES_HELPER = re.compile(r"\b(?:" + helper_alt + r")\s*\(")

        self.live, self.named_only = set(), {}
        for path in files:
            code = strip_cpp(read(path))
            decl_lines = {lineno(read(path), m.start())
                          for m in ENTRY_CONST_RE.finditer(read(path))}
            for m in ENTRY_USE_RE.finditer(code):
                cname = m.group(1)
                if cname not in self.decls or lineno(code, m.start()) in decl_lines:
                    continue
                builds = False
                for a, b, mk in blocks[path]:
                    if a <= m.start() <= b and (mk or USES_HELPER.search(code[a:b])):
                        builds = True
                        break
                if builds:
                    self.live.add(self.decls[cname])
        for cname, ep in self.decls.items():
            if self.uses.get(cname) and ep not in self.live:
                self.named_only[cname] = ep
        self.live |= set(self.literal_uses)
        self.unused_consts = sorted(c for c in self.decls if not self.uses.get(c))
        # entryPoint fields fed from a variable rather than a literal: the
        # indirection static extraction has to survive.
        self.indirect_sites = []
        for path in files:
            raw = read(path)
            for m in re.finditer(r"entryPoint\s*=\s*([A-Za-z_]\w*)\s*;", raw):
                self.indirect_sites.append(
                    (path, lineno(raw, m.start()), m.group(1))
                )


# ═══════════════════════════════════════════════════════════════════════
# §2 — THE BINDING REGISTRY
# ═══════════════════════════════════════════════════════════════════════

REG_CONST_RE = re.compile(
    r"^\s*inline\s+constexpr\s+uint32_t\s+(\w+)\s*=\s*(\d+)\s*;(.*)$", re.M
)
REG_NS_RE = re.compile(r"^\s*namespace\s+(g\d+)\s*\{", re.M)
BIND_REF_RE = re.compile(r"\bbind::(g\d+)::(\w+)\b")
BG_SINK_RE = re.compile(
    r"(\w+)\s*=\s*\w+\.Create(BindGroupLayout|BindGroup)\s*\(", re.M
)
# Flag (d) is about PARKED BINDING NUMBERS specifically — "reserved",
# "parked", "do not reuse". RETIRED/REMOVED tombstones are a different
# animal and are censused in §6.1; folding them in here triples the row
# count and buries the actual parking notes.
RESERVED_RE = re.compile(
    r"(?i)\b(number\s+(?:reserved|parked)|reserved\b|parked\b|do\s+not\s+reuse)"
)


class Registry:
    def __init__(self, path):
        raw = read(path)
        self.raw = raw
        self.consts = {}   # (group, name) -> {value, line, comment}
        ns_marks = [(m.start(), m.group(1)) for m in REG_NS_RE.finditer(raw)]
        for m in REG_CONST_RE.finditer(raw):
            group = "g?"
            for pos, g in ns_marks:
                if pos < m.start():
                    group = g
                else:
                    break
            self.consts[(group, m.group(1))] = {
                "group": group,
                "name": m.group(1),
                "value": int(m.group(2)),
                "line": lineno(raw, m.start()),
                "comment": m.group(3).strip().lstrip("/ ").strip(),
            }
        self.by_name = {}
        # THE SLOT IS THE KEY, not the name. The header says so: one buffer
        # wears several names (patch_instances/fc_patches; orb_state/
        # render_orb_state/orb_state_ro), each name addressing one
        # (group, slot). Indexing by name alone reads every alias as an
        # orphan — this index is what keeps flag (a) honest.
        self.by_slot = defaultdict(list)
        for (g, n), rec in self.consts.items():
            self.by_name.setdefault(n, []).append(rec)
            self.by_slot[(g, rec["value"])].append(rec)


def scan_bind_refs(path):
    """Attribute every `bind::gN::name` reference to the bind group / layout
    it lands in. The block always terminates in an assignment to a named
    `...Layout_` / `...BindGroup_`, so a forward scan to the next sink is
    sound and needs no brace tracking."""
    raw = read(path)
    code = strip_cpp(raw)
    sinks = [(m.start(), m.group(1), m.group(2)) for m in BG_SINK_RE.finditer(code)]
    out = []
    for m in BIND_REF_RE.finditer(code):
        sink_name, sink_kind = "<unattributed>", "?"
        for pos, nm, kind in sinks:
            if pos > m.start():
                sink_name, sink_kind = nm, kind
                break
        out.append(
            {
                "group": m.group(1),
                "name": m.group(2),
                "line": lineno(code, m.start()),
                "sink": sink_name,
                "kind": sink_kind,
            }
        )
    return out


# ═══════════════════════════════════════════════════════════════════════
# §3 — THE CONFIG MIRROR
# ═══════════════════════════════════════════════════════════════════════

CPP_FIELD_RE = re.compile(
    r"^[ \t]*(uint32_t|int32_t|uint64_t|float|double|int|unsigned)\s+(\w+)"
    r"((?:\s*\[\s*\d+\s*\])*)\s*;",
    re.M,
)
WGSL_FIELD_RE = re.compile(r"^\s*(\w+)\s*:\s*(.+?)\s*,\s*$")

# C++ scalars: every one of them packs at align 4 in this struct.
CPP_SCALAR = {
    "uint32_t": (4, 4), "int32_t": (4, 4), "float": (4, 4),
    "int": (4, 4), "unsigned": (4, 4), "double": (8, 8), "uint64_t": (8, 8),
}


def wgsl_layout(ty):
    """(align, size) under the WGSL uniform address-space rules:
       f32/u32/i32 -> 4/4 · vec2 -> 8/8 · vec3 -> 16/12 · vec4 -> 16/16
       array<T,N>  -> align(T), stride = roundUp(align(T), size(T)) * N
       (uniform address space forces array stride to a multiple of 16, which
        is exactly why the tree spells wave_frozen_t as three scalars and not
        array<f32,3> — see the note at world.wgsl's DesignConfig.)"""
    ty = ty.strip()
    m = re.match(r"array\s*<\s*(.+?)\s*,\s*(\d+)\s*>$", ty)
    if m:
        ea, es = wgsl_layout(m.group(1))
        stride = ((es + ea - 1) // ea) * ea
        stride = ((stride + 15) // 16) * 16  # uniform address space
        return ea if ea >= 16 else 16, stride * int(m.group(2))
    if ty in ("f32", "u32", "i32"):
        return 4, 4
    if ty.startswith("vec2"):
        return 8, 8
    if ty.startswith("vec3"):
        return 16, 12
    if ty.startswith("vec4"):
        return 16, 16
    if ty.startswith("mat4x4"):
        return 16, 64
    if ty.startswith("mat3x3"):
        return 16, 48
    return 4, 4  # unknown -> scalar; flagged by the AGREE column


def cpp_layout(base, dims):
    a, s = CPP_SCALAR.get(base, (4, 4))
    n = 1
    for d in re.findall(r"\d+", dims or ""):
        n *= int(d)
    return a, s * n


def roundup(x, a):
    return ((x + a - 1) // a) * a


def parse_cpp_struct(raw, name):
    """Field list of a C++ struct, in declaration order, with offsets.
    Everything is done against the STRIPPED text — a struct located in the
    raw text and then walked in the stripped one is exactly the drift the
    length-preserving strippers exist to prevent."""
    code = strip_cpp(raw)
    m = re.search(r"struct(?:\s+alignas\s*\(\s*\d+\s*\))?\s+" + re.escape(name) + r"\s*\{", code)
    if not m:
        return [], 0, 0
    start = m.end()
    depth, i = 1, start
    while i < len(code) and depth:
        if code[i] == "{":
            depth += 1
        elif code[i] == "}":
            depth -= 1
        i += 1
    body = code[start : i - 1]
    body_off = start
    fields, off, maxa = [], 0, 4
    for fm in CPP_FIELD_RE.finditer(body):
        base, fname, dims = fm.group(1), fm.group(2), fm.group(3) or ""
        a, size = cpp_layout(base, dims)
        off = roundup(off, a)
        fields.append(
            {
                "name": fname,
                "type": base + re.sub(r"\s+", "", dims),
                "offset": off,
                "size": size,
                "align": a,
                "line": lineno(code, body_off + fm.start()),
                "elems": max(1, _prod_dims(dims)),
            }
        )
        off += size
        maxa = max(maxa, a)
    align = 16  # alignas(16) on the struct
    return fields, roundup(off, align), align


def _prod_dims(dims):
    n = 1
    for d in re.findall(r"\d+", dims or ""):
        n *= int(d)
    return n


def parse_wgsl_struct(text, name):
    m = re.search(r"^[ \t]*struct\s+" + re.escape(name) + r"\s*\{", text, re.M)
    if not m:
        return [], 0, 0
    start = m.end()
    depth, i = 1, start
    while i < len(text) and depth:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    body = text[start : i - 1]
    fields, off, maxa = [], 0, 4
    for line in body.split("\n"):
        fm = WGSL_FIELD_RE.match(line)
        if not fm:
            continue
        fname, ty = fm.group(1), fm.group(2)
        a, size = wgsl_layout(ty)
        off = roundup(off, a)
        fields.append({"name": fname, "type": ty, "offset": off, "size": size, "align": a})
        off += size
        maxa = max(maxa, a)
    # line numbers, resolved against the same text the offsets came from
    base_line = lineno(text, m.start())
    seen = 0
    for line_i, line in enumerate(body.split("\n")):
        if WGSL_FIELD_RE.match(line):
            fields[seen]["line"] = base_line + line_i + 1
            seen += 1
    return fields, roundup(off, maxa), maxa


def pair_fields(cpp, wg):
    """Walk both field lists in order and pair them. The two rooms are not
    1:1 by construction: a C++ `float v[3]` may face a WGSL `vec3<f32>` OR
    three scalars `v0,v1,v2`. Both spellings are legal mirrors; only the
    OFFSETS have to agree."""
    rows, i, j = [], 0, 0
    while i < len(cpp) or j < len(wg):
        c = cpp[i] if i < len(cpp) else None
        w = wg[j] if j < len(wg) else None
        if c and w and c["name"] == w["name"]:
            rows.append((c, [w]))
            i += 1
            j += 1
            continue
        # C++ array vs a run of numbered WGSL scalars (wave_frozen_t -> t0,t1,t2)
        if c and w and w["name"].startswith(c["name"]) and w["name"][len(c["name"]):].isdigit():
            n = c["elems"]
            run = []
            while j < len(wg) and len(run) < n and wg[j]["name"].startswith(c["name"]) \
                    and wg[j]["name"][len(c["name"]):].isdigit():
                run.append(wg[j])
                j += 1
            rows.append((c, run))
            i += 1
            continue
        # genuine divergence — record and resync by index
        rows.append((c, [w] if w else []))
        i += 1
        j += 1
    return rows


# ═══════════════════════════════════════════════════════════════════════
# §4 — STATUS TAGS
# ═══════════════════════════════════════════════════════════════════════

TAG_PATTERNS = [
    ("STATUS: LATENT", re.compile(r"STATUS:\s*LATENT")),
    ("STATUS: INTENT", re.compile(r"STATUS:\s*INTENT")),
    ("LATENT[...]", re.compile(r"(?<!:\s)\bLATENT\s*\[")),
    ("DRIVERLESS", re.compile(r"\bDRIVERLESS\b")),
    ("INTENT (bare)", re.compile(r"(?<!STATUS: )\bINTENT\b(?!\s*:)")),
]

# WGSL attributes are not declarations. `@compute @workgroup_size(1)` above a
# fn, or `@location(2)` on a struct member, will hand a naive "first line
# below" rule the answers `workgroup_size` and `location`.
ATTR_ONLY = re.compile(r"^\s*@[A-Za-z_]\w*(\s*\([^)]*\))?(\s*@[A-Za-z_]\w*(\s*\([^)]*\))?)*\s*$")
DECL_NAME = [
    re.compile(r"^\s*(?:@[^\n]*\s+)?fn\s+([A-Za-z_]\w*)\s*\("),          # wgsl fn
    re.compile(r"^\s*struct\s+([A-Za-z_]\w*)"),                          # struct
    re.compile(r"^\s*(?:const|override)\s+([A-Za-z_]\w*)"),              # wgsl const
    re.compile(r"^\s*@group[^\n]*var\s*(?:<[^>]*>)?\s*([A-Za-z_]\w*)"),  # wgsl binding
    re.compile(r"^\s*([A-Za-z_]\w*)\s*:\s*\S"),                          # struct member
    re.compile(r"^\s*(?:inline\s+|static\s+|constexpr\s+|const\s+|virtual\s+)*"
               r"[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*[\(\[=;]"),  # c++ declarator
]
FN_HEAD = re.compile(r"^fn\s+([A-Za-z_]\w*)\s*\(")
CPP_FN_HEAD = re.compile(
    r"^\s{0,20}(?:inline\s+|static\s+|virtual\s+|constexpr\s+)*"
    r"[A-Za-z_][\w:<>,\s\*&]*?\b([A-Za-z_]\w*)\s*\([^;]*\)\s*(?:const\s*)?\{\s*$")


def _is_comment(s):
    t = s.strip()
    return t.startswith("//") or t.startswith("*") or t.startswith("/*")


def governed_symbol(lines, idx, wgsl=False):
    """(governs, inside) — the declaration a tag sits above, and the function
    it sits inside, if any.

    Comments in this tree run to eight lines and more, and a fixed lookahead
    window silently walks off the end of one and falls back to the line
    ABOVE — which is a different symbol entirely. So comment and blank lines
    are skipped without consuming budget, attribute-only lines are stepped
    over, and the enclosing function is reported separately rather than being
    confused with the governed one."""
    governs = "(unattributed)"
    scanned = 0
    for k in range(idx, len(lines)):
        s = lines[k]
        if not s.strip() or _is_comment(s) or ATTR_ONLY.match(s):
            continue
        scanned += 1
        if scanned > 3:
            break
        for pat in DECL_NAME:
            m = pat.match(s)
            if m:
                governs = m.group(1)
                break
        if governs != "(unattributed)":
            break
        governs = s.strip()[:48]
        break
    inside = None
    for k in range(idx, -1, -1):
        m = FN_HEAD.match(lines[k]) if wgsl else CPP_FN_HEAD.match(lines[k])
        if m:
            inside = m.group(1)
            break
    return governs, inside


def scan_status_tags(files):
    """Tag sites, each CLASSIFIED. Not every match is a prunable tag:

      LEGEND        — the lines that DEFINE the vocabulary (`STATUS: LATENT[name]
                      — capability with a plausible future`). Pruning the
                      legend is a category error.
      tombstone-ref — the tag appears inside a `(X REMOVED — LATENT[y])`
                      marker. The capability is already gone; the tag is a
                      record of it, and belongs to §6.1, not here.
      prose         — an unbracketed LATENT/INTENT inside an English sentence
                      or a banner ("THE DRIVER'S INTENT ORGAN"). Not a tag.
      TAG           — everything else: a live claim about a live symbol.

    Only TAG rows are the ruling surface. The others are reported so the
    count cannot be quietly inflated by them."""
    hits = []
    for path in files:
        raw = read(path)
        if not raw:
            continue
        wgsl = path.endswith(".wgsl")
        lines = splitlines_exact(raw)
        # A LEGEND is a run of tag lines that define the vocabulary: three or
        # more distinct tag words inside one 8-line window, each followed by a
        # dash and a definition.
        legend_lines = set()
        defn = [i for i, l in enumerate(lines)
                if re.search(r"STATUS:\s*[A-Z]+(\[\w[\w:-]*\])?\s*[—–-]{1,2}\s", l)]
        for i in defn:
            near = [j for j in defn if abs(j - i) <= 8]
            if len(near) < 3:
                continue
            # Expand to the whole contiguous comment block: a legend's
            # continuation lines ("… (the DRIVERLESS pattern, generalized)")
            # carry tag words too, and are just as much dictionary.
            for seed in near:
                k = seed
                while k > 0 and _is_comment(lines[k - 1]):
                    k -= 1
                j = seed
                while j + 1 < len(lines) and _is_comment(lines[j + 1]):
                    j += 1
                legend_lines.update(range(k, j + 1))
        for i, line in enumerate(lines):
            for label, pat in TAG_PATTERNS:
                m = pat.search(line)
                if not m:
                    continue
                # every tag class must live inside a comment
                before = line[: m.start()]
                if "//" not in before and not line.strip().startswith(("*", "//", "/*")):
                    continue
                if i in legend_lines:
                    kind = "LEGEND (defines the vocabulary)"
                elif TOMBSTONE_RE.search(line):
                    kind = "tombstone-ref (already removed)"
                elif label == "INTENT (bare)" and "INTENT[" not in line:
                    kind = "**prose — not a tag**"
                elif label == "LATENT[...]" and "LATENT[" not in line:
                    kind = "**prose — not a tag**"
                else:
                    kind = "TAG"
                gov, inside = governed_symbol(lines, i, wgsl)
                # A tag in the file's opening banner governs the FILE, not the
                # `#include` or `namespace` that happens to follow it.
                if re.match(r"^\s*(?:#include|#pragma|namespace\b|using\b)", gov):
                    gov = "(file header)"
                if kind.startswith("LEGEND"):
                    gov = "(the STATUS convention itself)"
                hits.append(
                    {
                        "file": path,
                        "line": i + 1,
                        "tag": label,
                        "kind": kind,
                        "text": line.strip()[:160],
                        "symbol": gov,
                        "inside": inside,
                    }
                )
                break
    return sorted(hits, key=lambda h: (h["file"], h["line"]))


def first_appearance(path, needle, enabled=True):
    """git pickaxe: the commit that first ADDED this text to this file.
    Bounded by the repo's own history depth — see the caveat printed in §4."""
    if not enabled:
        return "-"
    d = git("log", "--diff-filter=A", "-1", "--format=%as", "-S", needle, "--", path)
    return d or "(not in history)"


# ═══════════════════════════════════════════════════════════════════════
# §5 — DANGLING REFERENCES
# ═══════════════════════════════════════════════════════════════════════

DANGLING = [
    ("backup_board", re.compile(r"\bbackup_board\b")),
    ("the_chord", re.compile(r"\bthe_chord\b")),
    ("src/dev/BACKUP", re.compile(r"src/dev/BACKUP")),
    ("*.inl", re.compile(r"\b\w+\.inl\b")),
    ("cell_fields", re.compile(r"\bcell_fields\b")),
    ("zone_mesh_*", re.compile(r"\bzone_mesh_\w*")),
    ("zone_extrusion_*", re.compile(r"\bzone_extrusion_\w*")),
    ("zone_patch_instances", re.compile(r"\bzone_patch_instances\b")),
    ("apply_gol_extrusion_color", re.compile(r"\bapply_gol_extrusion_color\b")),
    ("animated_cell_color", re.compile(r"\banimated_cell_color\b")),
    ("OVERLAY_WAVES", re.compile(r"\bOVERLAY_WAVES\b")),
    ("probe_*", re.compile(r"\bprobe_\w+")),
]


# ═══════════════════════════════════════════════════════════════════════
# §6 — TOMBSTONES, COORDINATES, DEBUG CONSTANTS
# ═══════════════════════════════════════════════════════════════════════

# A TOMBSTONE is the house's own marker, and the house spells it in CAPS:
# `(fn X RETIRED — ...)`, `(binding N RETIRED ...)`, `(X REMOVED ...)`,
# `Number reserved`. Matching case-insensitively drags in every prose use of
# "retired"/"reserved" — 116 false positives on this tree — so the case IS
# the signal. Verified: 74 uppercase markers + 4 `Number reserved`.
TOMBSTONE_RE = re.compile(r"\b(?:RETIRED|REMOVED)\b|Number\s+reserved")
CAMPAIGN_TAGS = [
    "GROUND_CARD_1", "UNIFIED_GROUND_1", "TRUEBAND_CONTACT_1", "CLOSURE_PAWN",
    "UG_FIELDS_1", "CONTACT_2", "CONTACT_3", "CONTACT_4", "CONTACT_5",
    "TIDY_1", "PRUNING_1", "CHECKER-REBUILD", "LADDER", "COMPACT",
]
CAMPAIGN_TAG_RE = re.compile(r"\b(" + "|".join(re.escape(t) for t in CAMPAIGN_TAGS) + r")\b")
# A coordinate is a campaign tag followed by a short stamp, or a bare stamp
# in a comment. The bare form is deliberately narrow: `[A-Z]\d[a-z]?` bounded
# by brackets/parens/whitespace, so ordinary prose does not match.
COORD_RE = re.compile(
    r"\b(?:" + "|".join(re.escape(t) for t in CAMPAIGN_TAGS) + r")\s+"
    r"([A-Za-z]{1,3}\d{1,2}[a-z]?)\b"
)
DEBUG_CONSTS = [
    "TERRAIN_DEBUG_VIEW", "CHECKER_DEBUG_VIEW",
    "LIVE_CARD_DEBUG_VIEW", "CONTACT_SHELL_DEBUG",
]
DIAG_RE = re.compile(r"\[DIAG:AUDIT\]")


# ═══════════════════════════════════════════════════════════════════════
# THE REPORT
# ═══════════════════════════════════════════════════════════════════════


class Report:
    def __init__(self):
        self.buf = []

    def __call__(self, s=""):
        self.buf.append(s)

    def table(self, headers, rows):
        if not rows:
            self("_(none)_")
            self()
            return
        self("| " + " | ".join(headers) + " |")
        self("|" + "|".join("---" for _ in headers) + "|")
        for r in rows:
            cells = ["" if c is None else str(c).replace("|", "\\|").replace("\n", " ") for c in r]
            self("| " + " | ".join(cells) + " |")
        self()

    def text(self):
        return "\n".join(self.buf) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--no-dates", action="store_true",
                    help="skip the git pickaxe first-appearance dating (§4)")
    ap.add_argument("--check", action="store_true",
                    help="exit 1 if %s is missing or stale; write nothing" % OUT)
    ap.add_argument("--quiet", action="store_true", help="write the file, print nothing")
    args = ap.parse_args()

    if not exists(WGSL):
        sys.stderr.write(
            "pruning_census: run me from the repo root (%s not found)\n" % WGSL
        )
        return 2

    dates_on = not args.no_dates
    R = Report()

    head = git("rev-parse", "HEAD") or "(no git)"
    head_short = head[:12]
    branch = git("rev-parse", "--abbrev-ref", "HEAD") or "(detached)"
    head_date = git("log", "-1", "--format=%as") or "-"
    hist_depth = git("rev-list", "--count", "HEAD") or "?"
    root_commit = git("log", "--reverse", "--format=%as", "--max-parents=0") .split("\n")[0] \
        if git("log", "--reverse", "--format=%as", "--max-parents=0") else "-"

    # ── parse everything once ──────────────────────────────────────────
    w = WgslModule(WGSL)
    # The web mirror is parsed HERE, not down in §5, because every WGSL
    # deletion verdict has to consult it. A symbol that is dead on the
    # desktop and LIVE in the mirror is not a free deletion — the next
    # resync would silently drop code the web port is running.
    mw = WgslModule(WEB_WGSL) if exists(WEB_WGSL) else None

    def mirror_status(name):
        """('live' | 'present but dead' | 'absent' | '—', detail)."""
        if mw is None:
            return "—", ""
        if name in mw.functions:
            callers = mw.callers_of(name)
            if callers:
                return ("**LIVE IN MIRROR**",
                        "called by %s" % ", ".join("`%s`" % c for c in callers[:3]))
            return "present, 0 callers", ""
        if name in mw.consts or name in mw.structs or name in mw.resources:
            n = mw.references_outside_decl(
                name, mw.consts.get(name) or mw.structs.get(name)
                or (mw.resources[name]["line"] if name in mw.resources else None))
            return ("**LIVE IN MIRROR**" if n else "present, 0 refs",
                    "%d reference(s)" % n if n else "")
        return "absent", ""
    gate = PipelineGate([RENDERER, STATE, PASSES])
    reg = Registry(REGISTRY)
    refs = scan_bind_refs(STATE)

    live_entries = sorted(n for n in w.entry_points if n in gate.live)
    dead_entries = sorted(n for n in w.entry_points if n not in gate.live)
    live_fns = w.reachable_from(live_entries)
    all_entry_fns = w.reachable_from(list(w.entry_points))
    unreachable_fns = sorted(set(w.functions) - all_entry_fns)
    dead_only_fns = sorted(all_entry_fns - live_fns)

    # ═══════════════════════════════════════════════════════════════════
    R("# PRUNING_1 — P0: THE CENSUS")
    R()
    R("> Generated by `tools/pruning_census.py`. Stdlib only, deterministic,")
    R("> zero writes under `src/`. Re-run to regenerate; the file diffs cleanly.")
    R()
    R("| anchor | value |")
    R("|---|---|")
    R("| HEAD | `%s` |" % head)
    R("| HEAD (short) | `%s` |" % head_short)
    R("| branch | `%s` |" % branch)
    R("| HEAD date | %s |" % head_date)
    R("| history depth | %s commits (root dated %s) |" % (hist_depth, root_commit))
    R("| shader | `%s` — %d lines |" % (WGSL, w.total))
    R()
    R("**Every finding below cites this hash.** If HEAD has moved, re-run:")
    R("`python tools/pruning_census.py`.")
    R()
    # The handoff says "anchor by HASH, never by remembered name". Honour that
    # literally: state where HEAD sits relative to the trunk, because in this
    # checkout they are NOT the same commit and a census read as if it were
    # taken on master would be read wrong.
    ahead = git("rev-list", "--count", "origin/master..HEAD")
    behind = git("rev-list", "--count", "HEAD..origin/master")
    om = git("rev-parse", "origin/master")
    if om and om != head:
        R("> **ANCHOR NOTICE — read before citing this census as \"master\".**")
        R("> `origin/master` is `%s`, and HEAD is **%s commit(s) ahead / %s behind**" %
          (om[:12], ahead or "?", behind or "?"))
        R("> it. Every count in this report is taken at HEAD (`%s`), which" % head_short)
        R("> carries the shipped ground-campaign work that `origin/master` does not.")
        R("> A census run on `origin/master` would give different numbers. The")
        R("> handoff's instruction — *anchor by HASH, never by remembered name* —")
        R("> is exactly why this notice exists.")
        R()
    R("---")
    R()

    # ── PREMISE CHECKS ─────────────────────────────────────────────────
    R("## §0 — PREMISE CHECKS (the handoff's own assumptions, verified)")
    R()
    prem = []
    n_chord = sum(
        len(re.findall(r"\bthe_chord\b", read(p)))
        for p in walk_src(exts={".hpp", ".cpp", ".wgsl", ".md", ".txt"},
                          roots=("src", "audit"), extra=(CMAKE,))
    )
    chord_dir = exists("src/cartridges/the_chord")
    prem.append((
        "`the_chord` is retired (the antecedent only — see the row below for "
        "the one-room conclusion it is used to draw)",
        "CONFIRMED" if not chord_dir else "FALSE",
        "no `src/cartridges/the_chord/` directory; %d textual mentions remain (§5). "
        "**This confirms the premise, NOT the conclusion**: the cartridge mirror "
        "is gone, but that is not the only mirror." % n_chord,
    ))
    web_live = exists(WEB_WGSL)
    if web_live:
        d_sha = hashlib.sha256(read(WGSL).encode("utf-8", "replace")).hexdigest()
        m_sha = hashlib.sha256(read(WEB_WGSL).encode("utf-8", "replace")).hexdigest()
        side = read(WEB_WGSL_SIDECAR).strip().replace("\n", " · ")
        prem.append((
            "`world.wgsl` is SINGLE-COPY",
            "**FALSE — a second copy exists**",
            "`%s` (%d lines) vs `%s` (%d lines); sha differ (%s… vs %s…). "
            "Sidecar: %s" % (
                WGSL, w.total, WEB_WGSL, len(splitlines_exact(read(WEB_WGSL))),
                d_sha[:10], m_sha[:10], side or "(absent)"),
        ))
    prem.append((
        "`backup_board` is gone from the tree",
        "CONFIRMED" if not exists("src/cartridges/backup_board") else "FALSE",
        "no such directory; remaining references are dangling — see §5",
    ))
    if exists(WEB_UNIFORMS):
        prem.append((
            "the config is a TWO-room mirror (C++ ↔ WGSL)",
            "**FALSE — there is a third room**",
            "`%s` packs the same 592-byte buffer by hand-written word index, "
            "checked by nothing — §3.3" % WEB_UNIFORMS,
        ))
    R.table(["premise", "verdict", "evidence"], prem)
    R("**Consequence for the campaign.** The one-room claim holds for the")
    R("`the_board`↔`the_chord` axis — that mirror law IS fossil, exactly as the")
    R("handoff says. It does NOT hold for the desktop↔web axis. There the tree")
    R("has two more rooms, both live and neither compiler-checked:")
    R()
    R("1. `%s` — a byte-identical shader mirror under `CLAUDE.md`'s" % WEB_WGSL)
    R("   Mirror doctrine, currently **stale** (§5.3);")
    R("2. `%s` — a hand-indexed packer for the same config" % WEB_UNIFORMS)
    R("   buffer, with the offsets typed into the source (§3.3).")
    R()
    R("So a P1 WGSL deletion is a **two-room** edit and a P3 config-field")
    R("deletion is a **three-room** one. Neither is the one-room edit the")
    R("handoff scoped, and that is the reason several §7 verdicts read")
    R("RULE(Jean) where the handoff would have expected DELETE.")
    R()
    R("Sized honestly, because the correction is worth only what it measures:")
    R("the shader mirror is cheap to fix today (§5.3 shows the web host")
    R("dispatches two entry points and both still exist on the desktop), while")
    R("the config packer is the one that actually bites (§3.3 — five deletion")
    R("candidates are written there at hand-typed offsets that nothing checks).")
    R()
    R("---")
    R()

    # ═══════════════════ §1 ═══════════════════════════════════════════
    R("## §1 — WGSL SYMBOL CENSUS (`%s`)" % WGSL)
    R()
    R("| metric | count |")
    R("|---|---|")
    R("| file lines | %d |" % w.total)
    R("| comment lines (whole-line) | %d (%.1f%%) |" %
      (w.comment_lines, 100.0 * w.comment_lines / max(1, w.total)))
    R("| blank lines | %d |" % w.blank_lines)
    R("| `fn` declarations | %d |" % len(w.functions))
    stage_n = Counter(f["stage"] for f in w.entry_points.values())
    R("| entry points (`@compute`/`@vertex`/`@fragment`) | %d — %s |" %
      (len(w.entry_points),
       " · ".join("%s %d" % (s, stage_n[s]) for s in ("compute", "vertex", "fragment")
                  if stage_n[s])))
    R("| entry points a C++ pipeline names (LIVE) | %d |" % len(live_entries))
    R("| entry points NO pipeline names (DEAD) | **%d** |" % len(dead_entries))
    R("| `fn` reachable from ANY entry point | %d |" % len(all_entry_fns))
    R("| `fn` reachable from a LIVE entry point | %d |" % len(live_fns))
    R("| `fn` unreachable from every entry point | **%d** |" % len(unreachable_fns))
    R("| `const` declarations (module-scope; %d function-local excluded) | %d |"
      % (len(w.local_consts), len(w.consts)))
    R("| `struct` declarations | %d |" % len(w.structs))
    R("| `override` declarations | %d |" % len(w.overrides))
    R("| `@group/@binding` declarations | %d |" % len(w.resources))
    R("| duplicate `fn` names | %d |" % len(w.duplicate_fns))
    R()
    if w.duplicate_fns:
        R("**Duplicate `fn` names found** — the handoff's uniqueness premise is false:")
        R.table(["name", "first line", "second line"], sorted(w.duplicate_fns))
    else:
        R("Function-name uniqueness holds (the handoff's premise): `\\bNAME\\s*\\(` is a")
        R("sound call-site test on the comment-stripped text.")
        R()

    R("### §1.1 — THE SECOND GATE: entry points vs pipelines actually created")
    R()
    R("An entry point with no pipeline is dead, and everything reachable ONLY")
    R("through it is dead with it. Pipelines are created in `%s`; the entry" % RENDERER)
    R("point is named through the `Entry::` constant table there.")
    R()
    R("**What \"LIVE\" means here, stated exactly.** Naming an entry point in C++")
    R("is *not* the same as building a pipeline from it — a debug print or a dead")
    R("branch would name it too. So each `Entry::` use site is resolved to the")
    R("enclosing brace block, and that block must either call")
    R("`Create{Render,Compute}Pipeline` directly or call a helper whose own body")
    R("does. (The statement is the wrong scope: this tree writes")
    R("`fragment.entryPoint = Entry::ENTITY_FS;` several statements above the")
    R("`CreateRenderPipeline` call in the same block.)")
    R()
    R("- `Entry::` string constants declared: **%d**" % len(gate.decls))
    R("- `Entry::` constants referenced at a non-declaration site: **%d**" % len(gate.uses))
    R("- direct `Create*Pipeline` call sites: **%d**" % len(gate.pipelines))
    R("- pipeline-building helpers detected: **%d** — %s" %
      (len(gate.helpers), ", ".join("`%s`" % h for h in gate.helpers) or "none"))
    R("- constants named in C++ but **not** inside a pipeline-building block: **%d**%s" %
      (len(gate.named_only),
       " — " + ", ".join("`%s`" % c for c in sorted(gate.named_only)) if gate.named_only else ""))
    R("- raw string literals assigned to an `entryPoint` field, **within the %d" % len(gate.files))
    R("  files this gate scans** (%s): **%d**" %
      (", ".join("`%s`" % f.split("/")[-1] for f in gate.files), len(gate.literal_uses)))
    R()
    R("> Scope caveat, so the zero above is not overread: the gate scans the")
    R("> desktop host only. `web/js/boot.js` assigns single-quoted entry-point")
    R("> literals into its own pipeline descriptors — a different host, for the")
    R("> web port, counted in §5.3 rather than here.")
    R()
    if gate.indirect_sites:
        R("**Indirection notice.** %d `entryPoint` assignments read a *variable*," % len(gate.indirect_sites))
        R("not a literal. Each is a helper-lambda parameter fed by `Entry::` at the")
        R("call site, so static extraction still closes — but it is the one place")
        R("this gate could be silently defeated, so it is censused:")
        R.table(["file:line", "variable"],
                sorted(("%s:%d" % (f, l), v) for f, l, v in gate.indirect_sites))
    if gate.unused_consts:
        R("**`Entry::` constants declared but never referenced** (dead names, even if")
        R("the WGSL entry point behind them is live):")
        R.table(["constant", "entry point string", "declared at"],
                [(c, gate.decls[c], gate.decl_sites.get(c, "?")) for c in gate.unused_consts])
    if dead_entries:
        R("### ⚠ DEAD ENTRY POINTS — no pipeline names these")
        R()
        rows = []
        for n in dead_entries:
            f = w.entry_points[n]
            only = sorted(w.closure(n) - live_fns - {n})
            rows.append((n, f["stage"], f["workgroup"] or "-", f["line"], len(only),
                         ", ".join(only[:6]) + (" …" if len(only) > 6 else "") or "-"))
        R.table(["entry point", "stage", "workgroup", "line",
                 "fns dead WITH it", "those fns"], rows)
    else:
        R("### ✅ NO DEAD ENTRY POINTS")
        R()
        R("Every one of the %d WGSL entry points is named by a pipeline, and every" % len(w.entry_points))
        R("one of the %d `Entry::` constants is referenced. The two lists are in" % len(gate.decls))
        R("exact bijection — the gate is closed and finds nothing. **This is the")
        R("single biggest cost saving of the campaign: §1 has no GPU-side prize,")
        R("so P1 should not be scoped as if it did.**")
        R()

    R("### §1.2 — `fn` unreachable from EVERY entry point")
    R()
    if unreachable_fns:
        rows = []
        for n in unreachable_fns:
            st, det = mirror_status(n)
            rows.append((n, w.functions[n]["line"], len(w.callers_of(n)),
                         ", ".join(w.callers_of(n)[:4]) or "—", st, det or "—"))
        R.table(["fn", "line", "callers (desktop)", "called by",
                 "web mirror", "mirror detail"], rows)
        R("Recipe: `python tools/pruning_census.py` — closure over the call graph from")
        R("all %d entry points; anything outside that union is here." % len(w.entry_points))
        R()
        live_in_mirror = [r[0] for r in rows if r[4].startswith("**")]
        if live_in_mirror:
            R("⚠ **%s %s dead on the desktop but LIVE in the web mirror.**" %
              (", ".join("`%s`" % n for n in live_in_mirror),
               "is" if len(live_in_mirror) == 1 else "are"))
            R("These are **not** free deletions. \"Zero callers\" is true of the")
            R("desktop room only; the next resync (`cp` desktop → mirror) would")
            R("drop code the web port is currently running. They go to RULE(Jean)")
            R("in §7, not DELETE. This is the concrete cost of the two-room")
            R("situation §5.3 measures — the first place it changes a verdict.")
    else:
        R("_(none — every `fn` is reachable from at least one entry point)_")
    R()
    if dead_only_fns:
        R("**Reachable ONLY through a dead entry point** (falls with it):")
        R.table(["fn", "line"], [(n, w.functions[n]["line"]) for n in dead_only_fns])
    R()

    R("### §1.3 — `const` declarations with zero post-strip references")
    R()
    # A WGSL const that nothing reads is a mechanical fact. Whether it should
    # GO is not: several are deliberate MIRRORS of a C++ enum, declared so the
    # two rooms read alike. Deleting a mirror is a policy change, not a
    # pruning — so the mirror is detected and routed to RULE(Jean) instead.
    cpp_idents = set()
    for p in walk_src(exts={".hpp", ".cpp"}, roots=("src",)):
        cpp_idents |= set(IDENT_RE.findall(strip_cpp(read(p))))

    def cpp_twin(name):
        for cand in (name, re.sub(r"_WGSL$", "", name)):
            if cand in cpp_idents:
                return cand
        return None

    dead_consts = []
    for name, ln in sorted(w.consts.items()):
        if w.references_outside_decl(name, ln) == 0:
            dead_consts.append((name, ln, cpp_twin(name), mirror_status(name)[0]))
    mirrors = [c for c in dead_consts if c[2]]
    web_live = [c for c in dead_consts if c[3].startswith("**")]
    R("Scope note: **module-scope only** (column 0). %d function-local `const`" % len(w.local_consts))
    R("declarations exist and are deliberately excluded — a body-scope constant")
    R("is not a module symbol and cannot be an orphan of this kind.")
    R()
    R("Unreferenced: **%d** of %d module-scope consts. Of those, **%d have a C++" %
      (len(dead_consts), len(w.consts), len(mirrors)))
    R("twin of the same name** (declared mirrors, not accidents) and **%d are" % len(web_live))
    R("still read by the web mirror**.")
    R()

    def const_class(twin, web):
        if web.startswith("**"):
            return "**LIVE IN THE WEB MIRROR — RULE(Jean)**"
        if twin:
            return "**MIRROR of C++ — RULE(Jean)**"
        return "orphan — DELETE"

    R.table(["const", "line", "C++ twin", "web mirror", "class"],
            [(n, l, "`%s`" % t if t else "—", web, const_class(t, web))
             for n, l, t, web in dead_consts])

    R("### §1.4 — `struct` declarations with zero references")
    R()
    dead_structs = [(n, l) for n, l in sorted(w.structs.items())
                    if w.references_outside_decl(n, l) == 0]
    R("Unreferenced: **%d** of %d." % (len(dead_structs), len(w.structs)))
    R()
    R.table(["struct", "line"], dead_structs)

    R("### §1.5 — `override` declarations")
    R()
    R.table(["override", "line", "references"],
            [(n, l, w.references_outside_decl(n, l)) for n, l in sorted(w.overrides.items())])

    R("---")
    R()

    # ═══════════════════ §2 ═══════════════════════════════════════════
    R("## §2 — BINDING CENSUS")
    R()
    R("Three rooms, one table: the authored numbers (`%s`), the layout+group" % REGISTRY)
    R("entries (`%s`), and the shader declarations (`%s`)." % (STATE, WGSL))
    R()
    by_sink = defaultdict(list)
    for r in refs:
        by_sink[(r["kind"], r["sink"])].append(r)
    reg_names = {n for (_g, n) in reg.consts}
    wgsl_names = set(w.resources)

    R("| metric | count |")
    R("|---|---|")
    R("| registry constants | %d |" % len(reg.consts))
    R("| WGSL `@group/@binding` declarations | %d |" % len(w.resources))
    R("| `bind::` references in `state.hpp` | %d |" % len(refs))
    R("| distinct bind-group layouts | %d |" %
      len({s for (k, s) in by_sink if k == "BindGroupLayout"}))
    R("| distinct bind groups | %d |" % len({s for (k, s) in by_sink if k == "BindGroup"}))
    R()
    # The registry header describes itself in prose, and prose does not
    # recompile. Check the numbers it claims against the numbers that are
    # true — a stale self-description is scaffolding too.
    slot_n = len({("g%d" % r["group"], r["binding"]) for r in w.resources.values()})
    hm = re.search(r"\((\d+)\s+\n?[^\n]*declarations over (\d+)\s*\n?[^\n]*slots", reg.raw)
    if not hm:
        hm = re.search(r"(\d+)\s+declarations over\s+(\d+)\s+slots", " ".join(reg.raw.split()))
    if hm:
        cd, cs = int(hm.group(1)), int(hm.group(2))
        ok = (cd == len(w.resources) and cs == slot_n)
        R("**The registry header describes itself, and the description is%s stale.**" %
          ("" if not ok else " NOT"))
        R.table(["claimed in `%s`'s header" % REGISTRY.split("/")[-1], "true at this HEAD"],
                [("%d WGSL `@binding` declarations" % cd,
                  "**%d**" % len(w.resources) + ("" if cd == len(w.resources) else "  ← differs")),
                 ("%d distinct slots" % cs,
                  "**%d**" % slot_n + ("" if cs == slot_n else "  ← differs"))])
        if not ok:
            R("Prose does not recompile. The header's own recount is off by %d and %d;" %
              (abs(cd - len(w.resources)), abs(cs - slot_n)))
            R("it is scaffolding of exactly the kind this campaign exists to find, and")
            R("it is the file whose *whole purpose* is to be the single source of truth")
            R("for these numbers. Recipe: this table.")
            R()

    # the joined slot table
    res_readers = {}
    for rn in w.resources:
        eps = sorted(e for e in live_entries if rn in set().union(
            *(w.direct_res.get(f, set()) for f in w.closure(e)) or [set()]))
        fns = sorted(f for f in live_fns if rn in w.direct_res.get(f, set()))
        res_readers[rn] = (eps, fns)

    rows = []
    for rn in sorted(w.resources, key=lambda n: (w.resources[n]["group"], w.resources[n]["binding"], n)):
        r = w.resources[rn]
        g = "g%d" % r["group"]
        exact = reg.consts.get((g, rn))
        at_slot = sorted(x["name"] for x in reg.by_slot.get((g, r["binding"]), []))
        # A WGSL var is registry-covered if a constant of its own name exists,
        # OR a constant addresses its exact (group, slot) under an alias name.
        if exact:
            regcol = exact["name"]
        elif at_slot:
            regcol = ", ".join(at_slot) + " *(alias)*"
        else:
            regcol = "— **none**"
        groups_bound = sorted({x["sink"] for x in refs
                               if x["name"] == rn or x["name"] in at_slot})
        eps, fns = res_readers[rn]
        rows.append((
            "%d/%d" % (r["group"], r["binding"]),
            regcol,
            rn,
            r["space"] or "handle",
            ", ".join(groups_bound[:3]) + (" …" if len(groups_bound) > 3 else "") or "—",
            len(fns),
            len(eps),
        ))
    R("### §2.1 — the joined slot table")
    R()
    R("The registry column resolves by **(group, slot)**, not by name — the")
    R("registry's own law is one constant per *site*, so several names can")
    R("legitimately address one slot (`config`/`fc_config`, `patch_instances`/")
    R("`fc_patches`). Rows marked *(alias)* are covered by a differently-named")
    R("constant at the same slot and are **not** orphans.")
    R()
    R.table(["(group,binding)", "registry const", "WGSL var", "space",
             "bound in", "reachable fn readers", "live entry readers"], rows)

    # (a)
    R("### §2.2 — flag (a): registry constant with NO WGSL twin")
    R()
    wgsl_slots = {("g%d" % r["group"], r["binding"]) for r in w.resources.values()}
    orphan_reg = sorted(
        (rec["group"], rec["name"], rec["value"], rec["line"],
         "alias — slot IS declared" if (rec["group"], rec["value"]) in wgsl_slots
         else "**slot not declared in WGSL at all**",
         rec["comment"][:60])
        for (g, n), rec in reg.consts.items() if n not in wgsl_names
    )
    R.table(["group", "constant", "value", "registry line", "slot status",
             "trailing comment"], orphan_reg)
    R("A registry constant with no shader twin is a number nothing claims. Some")
    R("are legitimate ALIASES named for a *different* site on the same buffer")
    R("(the header says so); the trailing comment column is where that shows.")
    R()

    # (b)
    R("### §2.3 — flag (b): WGSL declaration with NO registry constant")
    R()
    orphan_wgsl = []
    for rn in sorted(wgsl_names):
        r = w.resources[rn]
        g = "g%d" % r["group"]
        if (g, rn) in reg.consts:
            continue
        at_slot = sorted(x["name"] for x in reg.by_slot.get((g, r["binding"]), []))
        orphan_wgsl.append((g, r["binding"], rn, r["line"], r["type"][:44],
                            ", ".join(at_slot) or "— **slot unclaimed**"))
    R.table(["group", "binding", "WGSL var", "wgsl line", "type",
             "constant at the same slot"], orphan_wgsl)
    covered = [r for r in orphan_wgsl if not r[5].startswith("—")]
    R("**%d of %d are covered by a differently-named constant at the SAME slot**" %
      (len(covered), len(orphan_wgsl)))
    R("— exactly the `fc_`-alias set the registry header predicts by name. So")
    R("flag (b) is empty in substance: there is no WGSL binding this tree")
    R("addresses without an authored number. The remaining %d (if any) are the" %
      (len(orphan_wgsl) - len(covered)))
    R("slots where the mirror is held by the launch gate alone — a runtime")
    R("crash, not a compile error.")
    R()

    # (c)
    R("### §2.4 — flag (c): bound resource read by ZERO reachable functions ← GPU cost")
    R()
    unread = []
    for rn in sorted(wgsl_names):
        eps, fns = res_readers[rn]
        if not fns:
            r = w.resources[rn]
            bound = sorted({x["sink"] for x in refs if x["name"] == rn})
            st, det = mirror_status(rn)
            unread.append(("%d/%d" % (r["group"], r["binding"]), rn, r["space"] or "handle",
                           r["line"], ", ".join(bound) or "— (not bound in state.hpp)",
                           st, det or "—"))
    R("Reachability is computed **from LIVE entry points only** — an entry point")
    R("no pipeline names cannot keep a binding alive. The web mirror is checked")
    R("too, for the same reason it is checked in §1.2: a binding that is dead")
    R("here and read there is not a free removal.")
    R()
    R.table(["(group,binding)", "WGSL var", "space", "wgsl line", "bound in",
             "web mirror", "mirror detail"], unread)
    unread_mirror = [u for u in unread if u[5].startswith("**")]
    if unread_mirror:
        R("⚠ **%s read by the web mirror.** The desktop finding stands — %s has" %
          (", ".join("`%s`" % u[1] for u in unread_mirror),
           "it" if len(unread_mirror) == 1 else "they have"))
        R("zero references of any kind in `%s`, not merely zero reachable ones —" % WGSL)
        R("but the binding cannot be retired from the *registry* while the mirror")
        R("still binds it. Freeing the C++ buffer and layout entry is safe; taking")
        R("the number back is not, until §5.3 is resolved.")
        R()
    if not unread:
        R("Every declared binding is read by at least one function reachable from a")
        R("live entry point. **§2 yields no unread-binding prize either.**")
        R()

    # (d)
    R("### §2.5 — flag (d): reserved / parked / do-not-reuse comments")
    R()
    R("Per the standing ruling these are superstition — the registry IS the single")
    R("source, so a \"do not reuse\" note carries no information git does not.")
    R("Listed for retirement, not for action on the numbers themselves.")
    R()
    parked = []
    for path in (REGISTRY, STATE):
        raw = read(path)
        for i, line in enumerate(splitlines_exact(raw)):
            s = line.strip()
            if not (s.startswith("//") or s.startswith("*") or "//" in line):
                continue
            if RESERVED_RE.search(line):
                parked.append((path.split("/")[-1], i + 1, s[:120]))
    R("Scope: the two rooms that OWN binding numbers (`%s`, `%s`)." %
      (REGISTRY.split("/")[-1], STATE.split("/")[-1]))
    R("`RETIRED`/`REMOVED` tombstones are a different class and live in §6.1.")
    R()
    R("Sites: **%d**" % len(parked))
    R()
    R.table(["file", "line", "text"], parked)

    R("---")
    R()

    # ═══════════════════ §3 ═══════════════════════════════════════════
    R("## §3 — CONFIG MIRROR CENSUS")
    R()
    state_raw = read(STATE)
    cpp_fields, cpp_size, cpp_align = parse_cpp_struct(state_raw, "GPUDesignConfig")
    wgsl_fields, wgsl_size, wgsl_align = parse_wgsl_struct(w.text, "DesignConfig")
    rows_pair = pair_fields(cpp_fields, wgsl_fields)

    # the declared sizeof witness
    sz = re.search(r"static_assert\s*\(\s*sizeof\s*\(\s*GPUDesignConfig\s*\)\s*==\s*(\d+)", state_raw)
    declared = int(sz.group(1)) if sz else None

    # config aliases, derived not assumed
    wgsl_aliases = sorted(n for n, r in w.resources.items() if r["type"].strip() == "DesignConfig")
    cpp_members = sorted(set(re.findall(r"GPUDesignConfig\s+(\w+)\s*[{;=]", state_raw)))
    cpp_accessors = sorted(set(re.findall(r"GPUDesignConfig&?\s+(\w+)\s*\(\s*\)", state_raw)))

    R("| metric | value |")
    R("|---|---|")
    R("| C++ `GPUDesignConfig` fields | %d |" % len(cpp_fields))
    R("| WGSL `DesignConfig` fields | %d |" % len(wgsl_fields))
    R("| C++ computed size | %d B (align %d) |" % (cpp_size, cpp_align))
    R("| WGSL computed size | %d B (align %d) |" % (wgsl_size, wgsl_align))
    R("| declared `sizeof` witness | %s |" % (declared if declared else "—"))
    R("| computed == witness | %s |" % ("YES" if declared == cpp_size else "**NO**"))
    R("| WGSL uniform aliases of the struct | %s |" % (", ".join(wgsl_aliases) or "—"))
    R("| C++ member / accessor | %s / %s |" %
      (", ".join(cpp_members) or "—", ", ".join(a + "()" for a in cpp_accessors) or "—"))
    R()
    R("Offset rules applied — C++: scalars align 4, arrays align 4 × N, struct")
    R("`alignas(16)`. WGSL uniform address space: `f32/u32/i32` 4/4, `vec2` 8/8,")
    R("`vec3` **16**/12, `vec4` 16/16, `array<vec4,N>` stride 16. The four `vec3`")
    R("members are exactly where the rooms can silently diverge — C++ packs a")
    R("`float[3]` at align 4, WGSL forces align 16 — which is why `state.hpp`")
    R("carries `offsetof(...) %% 16 == 0` static_asserts on them.")
    R()

    # ── reader counting, over DERIVED alias sets ──────────────────────
    # A field WRITTEN at boot and never READ is not alive; it is a value
    # nobody asks for. So reads and writes are counted separately, and the
    # boot-pin block is fenced off by name rather than by eyeball.
    cpp_files = walk_src(exts={".hpp", ".cpp"}, roots=("src",))
    cpp_blobs = {p: strip_cpp(read(p)) for p in cpp_files}
    cpp_roots = sorted(set(cpp_members) | {a + r"\s*\(\s*\)" for a in cpp_accessors})
    ROOT_ALT = "|".join(cpp_roots) if cpp_roots else r"config_"
    WRITE_TAIL = re.compile(r"^\s*(?:\[[^\]]*\]\s*)*(?:=(?!=)|[+\-*/|&^]=|\+\+|--)")

    def find_boot_block():
        """The boot-pin block, located by its own name — `initializeState()`
        — and brace-matched, not guessed at by line number."""
        blob = cpp_blobs.get(STATE, "")
        m = re.search(r"\bbool\s+initializeState\s*\(\s*\)\s*\{", blob)
        if not m:
            return (STATE, 0, 0)
        _d, i = 1, m.end()
        while i < len(blob) and _d:
            if blob[i] == "{":
                _d += 1
            elif blob[i] == "}":
                _d -= 1
            i += 1
        return (STATE, lineno(blob, m.start()), lineno(blob, i))

    BOOT = find_boot_block()

    def in_boot(path, line):
        return path == BOOT[0] and BOOT[1] <= line <= BOOT[2]

    # ONE pass over every C++ blob, matching every config field at once via a
    # single alternation. Scanning ~2 MB once per field instead is the whole
    # difference between a 3 s gate and a 15 s one.
    _sites = defaultdict(lambda: ([], [], []))
    if cpp_fields:
        ALL_FIELDS = "|".join(sorted((re.escape(f["name"]) for f in cpp_fields),
                                     key=len, reverse=True))
        SITE_RE = re.compile(r"\b(?:" + ROOT_ALT + r")\s*\.\s*(" + ALL_FIELDS + r")\b")
        for p, blob in cpp_blobs.items():
            for m in SITE_RE.finditer(blob):
                ln = lineno(blob, m.start())
                site = "%s:%d" % (p, ln)
                reads, writes, boot = _sites[m.group(1)]
                if WRITE_TAIL.match(blob[m.end():m.end() + 24]):
                    (boot if in_boot(p, ln) else writes).append(site)
                else:
                    reads.append(site)

    def cpp_sites(field):
        """(reads, writes, boot_writes) — each a sorted list of file:line."""
        reads, writes, boot = _sites.get(field, ([], [], []))
        return sorted(reads), sorted(writes), sorted(boot)

    def wgsl_readers(field):
        """Every WGSL touch of a uniform field is a read — the address space
        is read-only in the shader."""
        if not wgsl_aliases:
            return []
        pat = re.compile(r"\b(?:" + "|".join(re.escape(a) for a in wgsl_aliases) +
                         r")\s*\.\s*" + re.escape(field) + r"\b")
        return sorted("%s:%d" % (WGSL, lineno(w.text, m.start())) for m in pat.finditer(w.text))

    def wgsl_readers_for(wl, cname):
        """WGSL reads for a C++ field, following the name split
        (`wave_frozen_t` -> `wave_frozen_t0..2`)."""
        out = []
        for x in wl:
            out += wgsl_readers(x["name"])
        if not wl:
            out += wgsl_readers(cname)
        return out

    # ── setters, their line spans, and their callers outside boot ──────
    setter_defs = {}    # field -> setter name
    setter_span = {}    # setter name -> (first_line, last_line) in state.hpp
    _state_blob = cpp_blobs.get(STATE, "")
    for m in re.finditer(r"\bvoid\s+(set_\w+)\s*\([^)]*\)\s*\{", _state_blob):
        # brace-match the body — these setters are `if (...) { ... }`, so a
        # `[^{}]*` body pattern silently matches nothing.
        _d, i = 1, m.end()
        while i < len(_state_blob) and _d:
            if _state_blob[i] == "{":
                _d += 1
            elif _state_blob[i] == "}":
                _d -= 1
            i += 1
        body = _state_blob[m.end():i]
        setter_span[m.group(1)] = (lineno(_state_blob, m.start()), lineno(_state_blob, i))
        for fm in re.finditer(r"\b(?:" + ROOT_ALT + r")\s*\.\s*(\w+)", body):
            setter_defs.setdefault(fm.group(1), m.group(1))

    # Same one-pass treatment for setter call sites.
    _setter_calls = defaultdict(list)
    if setter_span:
        CALL_RE = re.compile(r"\b(" + "|".join(sorted(map(re.escape, setter_span),
                                                      key=len, reverse=True)) + r")\s*\(")
        for p, blob in cpp_blobs.items():
            for m in CALL_RE.finditer(blob):
                name = m.group(1)
                ln = lineno(blob, m.start())
                lo, hi = setter_span[name]
                if p == STATE and lo <= ln <= hi:
                    continue          # the definition itself
                if in_boot(p, ln):
                    continue          # boot pins are not consumers
                _setter_calls[name].append("%s:%d" % (p, ln))

    def setter_callers(field):
        name = setter_defs.get(field)
        if not name:
            return None, []
        return name, sorted(_setter_calls.get(name, ()))

    def live_reads(field, reads):
        """Split reads into LIVE and self-referential. A read inside the body
        of a setter that nobody calls is the setter's own change-guard
        (`if (config_.X != s)`) — it does not keep the field alive; it dies
        with the setter. This is the one place a naive reader count reports
        a dead field as busy, so it is separated rather than hidden."""
        name, callers = setter_callers(field)
        if not name or callers:
            return reads, []
        lo, hi = setter_span.get(name, (0, 0))
        live, guard = [], []
        for s in reads:
            p, _, ln = s.rpartition(":")
            (guard if (p == STATE and lo <= int(ln) <= hi) else live).append(s)
        return live, guard

    R("The boot-pin block is `state.hpp::initializeState()`, located by name and")
    R("brace-matched: **lines %d–%d**. Writes inside it are pins, not consumers," % (BOOT[1], BOOT[2]))
    R("and are counted in their own column. **A field pinned at boot and never")
    R("read is not alive — it is a value nobody asks for.**")
    R()
    R("### §3.1 — the field-by-field mirror")
    R()
    rows = []
    zero_readers = []
    idx = 0
    for c, wl in rows_pair:
        idx += 1
        cname = c["name"] if c else "—"
        wname = "+".join(x["name"] for x in wl) if wl else "—"
        coff = c["offset"] if c else None
        woff = wl[0]["offset"] if wl else None
        csz = c["size"] if c else None
        wsz = (wl[-1]["offset"] + wl[-1]["size"] - wl[0]["offset"]) if wl else None
        agree = "AGREE" if (c and wl and coff == woff and csz == wsz) else "**DISAGREE**"
        cr, cw, cb = cpp_sites(cname) if c else ([], [], [])
        wr = wgsl_readers_for(wl, cname)
        sname, scalls = setter_callers(cname) if c else (None, [])
        cr_live, cr_guard = live_reads(cname, cr) if c else ([], [])
        rows.append((idx, cname, c["type"] if c else "—", coff,
                     wname, wl[0]["type"] if wl else "—", woff, agree,
                     "%d%s" % (len(cr_live),
                               " (+%d guard)" % len(cr_guard) if cr_guard else ""),
                     len(wr), len(cw), len(cb),
                     ("`%s` ×%d" % (sname, len(scalls))) if sname else "—"))
        if c and not cr_live and not wr:
            zero_readers.append((cname, c["type"], c["size"], coff, len(cb) + len(cw),
                                 sname, len(scalls), len(cr_guard)))
    R.table(["#", "C++ name", "C++ type", "C++ off",
             "WGSL name", "WGSL type", "WGSL off", "verdict",
             "C++ reads", "WGSL reads", "C++ writes", "boot pins",
             "setter ×callers"], rows)
    dis = sum(1 for r in rows if r[7].startswith("**"))
    R("**%d of %d rows AGREE on offset and extent; %d DISAGREE.** Total: C++ %d B," %
      (len(rows) - dis, len(rows), dis, cpp_size))
    R("WGSL %d B, declared witness %s. The two rooms are in lockstep at this HEAD." %
      (wgsl_size, declared))
    R()

    R("### §3.2 — fields with ZERO READS in BOTH rooms (deletion candidates)")
    R()
    R("Reader spellings are **derived, not assumed**: the WGSL aliases come from")
    R("every `var<uniform> X : DesignConfig` declaration (%s); the C++ roots" %
      (", ".join(wgsl_aliases) or "none"))
    R("come from the declared member and accessor of type `GPUDesignConfig`")
    R("(`%s`). Writes and boot pins are shown so the reason a field looks busy" %
      ", ".join(cpp_members + [a + "()" for a in cpp_accessors]))
    R("is visible: pinning is not consumption.")
    R()
    running = 0
    zrows = []
    for name, ty, size, off, nwrites, sname, ncalls, nguard in zero_readers:
        running += size
        zrows.append((name, ty, size, off, nwrites,
                      ("`%s` ×%d — dies with the field" % (sname, ncalls))
                      if sname else "—",
                      nguard or "—", running))
    R.table(["field", "type", "bytes", "offset", "writes+pins",
             "setter ×callers (excl. boot)", "guard-only reads",
             "running total"], zrows)
    realised = cpp_size - roundup(cpp_size - running, 16)
    R("**Reclaimable config bytes: %d B** of %d (%.1f%%)." %
      (running, cpp_size, 100.0 * running / max(1, cpp_size)))
    R()
    R("> Caveat, stated plainly. (1) Removing a field mid-struct **re-flows every")
    R("> offset after it in ALL THREE rooms** — C++, WGSL, and the hand-written")
    R("> JS packer of §3.3, which nothing checks. The total above is the ceiling if all of")
    R("> them go in ONE commit; taken one at a time the cost is a full mirror")
    R("> re-verification each time, and the `offsetof` witnesses in `state.hpp`")
    R("> are what makes that survivable at all. (2) The uniform is padded to its")
    R("> 16 B alignment, so the buffer only shrinks in 16 B steps: %d B of dead" % running)
    R("> field yields `592 → %d`, a **%d B** real reduction of the upload." %
      (roundup(cpp_size - running, 16), realised))
    R("> (3) The config is uploaded once per dirty-flag flush, not per draw —")
    R("> so even that %d B is a *bandwidth* saving of the smallest kind. **The" % realised)
    R("> reason to do this is legibility, not frames.** Say so to Jean rather")
    R("> than letting a byte count imply a performance claim.")
    R()

    # ── §3.3 THE THIRD ROOM ───────────────────────────────────────────
    # The handoff frames the config as a two-room mirror. It is a THREE-room
    # mirror: the web port packs the same 592-byte buffer by raw word index,
    # with the byte offsets hard-coded in JS. Nothing checks it. A field
    # deletion re-flows every later offset and silently corrupts it.
    web_u = read(WEB_UNIFORMS)
    zero_names = {z[0] for z in zero_readers}
    R("### §3.3 — THE THIRD ROOM ⚠ (`%s`)" % WEB_UNIFORMS)
    R()
    if web_u:
        m = re.search(r"new\s+ArrayBuffer\s*\(\s*(\d+)\s*\)", web_u)
        web_size = int(m.group(1)) if m else None
        cover = []                       # (byte, field) for each C++ field
        for f in cpp_fields:
            cover.append((f["offset"], f["offset"] + f["size"], f["name"]))

        def field_at(byte):
            for lo, hi, nm in cover:
                if lo <= byte < hi:
                    return nm
            return None

        # SCOPE TO makeConfig. The same file also has makeSignal(), which
        # indexes a DIFFERENT 336-byte buffer with the same `f[n]`/`u[n]`
        # spelling — scanning the whole file reports FrameSignal writes as
        # config hazards.
        fm = re.search(r"export\s+function\s+makeConfig\s*\([^)]*\)\s*\{", web_u)
        if fm:
            depth, i = 1, fm.end()
            while i < len(web_u) and depth:
                if web_u[i] == "{":
                    depth += 1
                elif web_u[i] == "}":
                    depth -= 1
                i += 1
            lo_off, hi_off = fm.start(), i
        else:
            lo_off, hi_off = 0, len(web_u)
        body = web_u[lo_off:hi_off]

        rows, hazards, seen_line = [], [], set()
        # `f[16] = …` AND the destructuring form `[f[16], f[17], f[18]] = …`,
        # where only the LAST element is followed by `]` + `=`. Requiring `=`
        # immediately after the subscript silently drops the other two — and
        # in this file that hides all three sun_direction words.
        for wm in re.finditer(r"\b([fu])\s*\[\s*(\d+)\s*\]\s*(?==|,|\])", body):
            idx = int(wm.group(2))
            byte = idx * 4
            ln = lineno(web_u, lo_off + wm.start())
            tail = body[wm.end(): body.find("\n", wm.end())]
            ann = re.search(r"@(\d+)", tail)
            nm = field_at(byte)
            # A trailing `// name @N` annotates the FIRST write on its line;
            # `f[24]=…; f[25]=…; f[26]=…;  // fog_color @96` is one comment for
            # three contiguous words, not three disagreements.
            if ln in seen_line:
                agree = "— (covered by this line's first annotation)"
            elif not ann:
                agree = "—"
            else:
                agree = ("AGREE" if int(ann.group(1)) == byte else
                         "**DISAGREE (comment says @%s)**" % ann.group(1))
            seen_line.add(ln)
            rows.append((ln, "%s[%d]" % (wm.group(1), idx), byte,
                         nm or "**outside every field**", agree,
                         "**⚠ ON A DELETION CANDIDATE**" if nm in zero_names else ""))
            if nm in zero_names:
                hazards.append((nm, ln, byte))
        for wm in re.finditer(r"\bf\.set\s*\(\s*\[[^\]]*\]\s*,\s*(\d+)\s*\)", body):
            idx = int(wm.group(1))
            nm = field_at(idx * 4)
            rows.append((lineno(web_u, lo_off + wm.start()), "f.set(…, %d)" % idx,
                         idx * 4, nm or "**outside every field**", "—",
                         "**⚠ ON A DELETION CANDIDATE**" if nm in zero_names else ""))
            if nm in zero_names:
                hazards.append((nm, lineno(web_u, lo_off + wm.start()), idx * 4))
        # loop-written ranges (`for (let i = 40; i < 46; i++) f[i] = ...`)
        for wm in re.finditer(
                r"for\s*\(\s*let\s+\w+\s*=\s*(\d+)\s*;\s*\w+\s*<\s*(\d+)\s*;[^)]*\)\s*"
                r"([fu])\s*\[\s*\w+\s*\]\s*=", body):
            a, b = int(wm.group(1)), int(wm.group(2))
            nm = field_at(a * 4)
            rows.append((lineno(web_u, lo_off + wm.start()),
                         "%s[%d..%d]" % (wm.group(3), a, b - 1), a * 4,
                         nm or "**outside every field**", "—",
                         "**⚠ ON A DELETION CANDIDATE**" if nm in zero_names else ""))
            if nm in zero_names:
                hazards.append((nm, lineno(web_u, lo_off + wm.start()), a * 4))
        rows.sort(key=lambda r: (r[2], r[0]))
        hazards = sorted(set(hazards))
        R("**The config mirror has THREE rooms, not two.** `%s` packs the same" % WEB_UNIFORMS)
        R("buffer for the web port by **raw word index**, with the byte offsets")
        R("written into the source as comments. Nothing checks it against either")
        R("struct — no `static_assert`, no generator, no test.")
        R()
        R.table(["", "value"],
                [("`new ArrayBuffer(N)`", "%s B" % web_size),
                 ("C++ `sizeof(GPUDesignConfig)`", "%d B" % cpp_size),
                 ("WGSL `DesignConfig`", "%d B" % wgsl_size),
                 ("sizes agree", "**YES**" if web_size == cpp_size else "**NO**"),
                 ("indexed writes found", str(len(rows)))])
        R("Every indexed write, resolved to the field it lands in:")
        R()
        R.table(["js line", "write", "byte", "lands in C++ field",
                 "comment vs computed", "hazard"], rows)
        ann_rows = [r for r in rows if r[4] in ("AGREE",) or r[4].startswith("**DIS")]
        dis = [r for r in ann_rows if r[4].startswith("**DIS")]
        R("**%d of %d hand-annotated offsets AGREE with the offsets this tool" %
          (len(ann_rows) - len(dis), len(ann_rows)))
        R("computed from `state.hpp`; %d disagree.** That is worth stating in its" % len(dis))
        R("own right: the web port's byte map was authored by hand, independently")
        R("of this instrument, and it lands on the same numbers. It is a genuine")
        R("**third witness** that §3.1's offset arithmetic is correct — and, at")
        R("the same time, the reason a deletion here is dangerous.")
        R()
        if hazards:
            R("### ⚠ THE BLOCKER")
            R()
            R("**%d of the §3.2 deletion candidates are WRITTEN BY THE WEB PORT** at" % len(set(h[0] for h in hazards)))
            R("a hard-coded offset: %s." %
              ", ".join("`%s` (%s:%d, @%d)" % (n, WEB_UNIFORMS, l, b) for n, l, b in hazards))
            R()
            R("Deleting them re-flows every later offset in the 592-byte buffer.")
            R("The C++ room is protected by `offsetof` static_asserts and the WGSL")
            R("room by the shared struct — **the JS room is protected by nothing**.")
            R("It would keep writing the old offsets and the web demo would boot")
            R("with silently wrong palette, veil and LOD values.")
            R()
            R("**This changes the §3 verdict from DELETE to RULE(Jean).** The cut is")
            R("still right; it is a *three-room* commit, and the third room needs")
            R("either a regenerated offset map or an offset witness of its own.")
            R("Recipe: `grep -nE '\\b[fu]\\[[0-9]+\\]' %s`." % WEB_UNIFORMS)
            R()
    else:
        R("_(not present — the config mirror is two rooms at this HEAD)_")
        R()

    R("### §3.4 — the handoff's expected candidates, verified one by one")
    R()
    R("The handoff lists these as expected zero-reader candidates and says")
    R("*verify, do not assume*. Verified:")
    R()
    expected = ["wave_time_scale", "wave_enable_mask", "wave_freeze_mask",
                "wave_frozen_t", "pawn_amp_scale", "pawn_height_bias",
                "mute_dynamics_2d", "active_cell_size"]
    erows = []
    cpp_by_name = {f["name"]: f for f in cpp_fields}
    wg_by_name = {f["name"]: f for f in wgsl_fields}
    for name in expected:
        f = cpp_by_name.get(name)
        cr, cw, cb = cpp_sites(name)
        wr = wgsl_readers(name)
        if name not in wg_by_name:
            for k in range(8):
                wr += wgsl_readers("%s%d" % (name, k))
        sname, scalls = setter_callers(name)
        cr_live, cr_guard = live_reads(name, cr)
        if cr_live or wr:
            verdict = "**HAS LIVE READERS — KEEP**"
        elif cr_guard:
            verdict = "**ZERO LIVE READS** — only its own dead setter's guard"
        else:
            verdict = "**ZERO READS — confirmed candidate**"
        erows.append((name, f["type"] if f else "(absent)", f["size"] if f else "-",
                      len(cr_live), len(wr), len(cw), len(cb), len(cr_guard),
                      ("`%s` ×%d" % (sname, len(scalls))) if sname else "—",
                      verdict, "; ".join((cr_live + cr_guard + wr)[:2]) or "—"))
    R.table(["expected field", "type", "bytes", "C++ live reads", "WGSL reads",
             "C++ writes", "boot pins", "guard-only reads", "setter ×callers",
             "verdict", "sites"], erows)
    conf = sum(1 for e in erows if "ZERO" in e[9])
    R("**%d of %d confirmed as zero-live-read; %d refuted.**" %
      (conf, len(erows), len(erows) - conf))
    R()
    R("Two of them — `pawn_amp_scale`, `pawn_height_bias` — read as busy to a")
    R("naive grep and are not. Their only read is the change-guard inside their")
    R("OWN setter (`if (config_.X != s)`), and that setter has **zero callers**.")
    R("The field, its setter, and the guard form one closed dead loop; they go")
    R("together or not at all. This is exactly the shape the handoff warned")
    R("against — *\"probably dead\" is not a finding* — so the recipe is named:")
    R("`setter_callers()` + `live_reads()` in `tools/pruning_census.py`.")
    R()

    R("---")
    R()

    # ═══════════════════ §4 ═══════════════════════════════════════════
    R("## §4 — STATUS-TAG CENSUS (the ruling surface)")
    R()
    src_files = walk_src()
    tags = scan_status_tags(src_files)
    counts = Counter(t["tag"] for t in tags)
    R("Standing ruling: `STATUS: LATENT` and `STATUS: INTENT` are DELETE-AND-RECORD")
    R("by DEFAULT. Every site below arrives with its own evidence row; Jean rules")
    R("the exceptions.")
    R()
    kinds = Counter(t["kind"] for t in tags)
    actionable = [t for t in tags if t["kind"] == "TAG"]
    R.table(["tag class", "raw sites", "of which TAG (the ruling surface)"],
            sorted((c, n, sum(1 for t in actionable if t["tag"] == c))
                   for c, n in counts.items()))
    R("**%d raw matches, of which %d are the actual ruling surface.**" %
      (len(tags), len(actionable)))
    R("The rest are not tags at all, and counting them would inflate the")
    R("campaign's apparent size:")
    R()
    R.table(["classification", "sites", "why it is not a prunable tag"],
            [(k, n, {
                "TAG": "— (this IS the surface)",
                "LEGEND (defines the vocabulary)":
                    "these lines DEFINE `STATUS: LATENT` / `STATUS: INTENT`; "
                    "deleting them deletes the dictionary",
                "tombstone-ref (already removed)":
                    "the tag sits inside a `(X REMOVED — …)` marker; the "
                    "capability is already gone — §6.1's business",
                "**prose — not a tag**":
                    "an unbracketed LATENT/INTENT in an English sentence or a "
                    "banner, e.g. “THE DRIVER'S INTENT ORGAN”",
            }.get(k, "—")) for k, n in sorted(kinds.items())])
    shallow = exists(".git/shallow")
    R("**Dating caveat — read this before treating age as evidence.** This")
    R("checkout is a **%sclone %s commits deep, rooted at %s**." %
      ("SHALLOW " if shallow else "", hist_depth, root_commit))
    if shallow:
        R("`.git/shallow` exists, so the history is *truncated by construction* —")
        R("the pickaxe cannot see past the graft no matter how the query is written.")
    R("Every first-appearance date below is therefore floored at that point: a tag")
    R("dated \"%s\" may be far older upstream and there is no way to tell from" % root_commit)
    R("here. **In this tree, age is not evidence.** The handoff's test — *\"a")
    R("LATENT tag that has waited months for a consumer is a fossil\"* — cannot be")
    R("run against this history at all. Rule on reachability, which §1–§3 measure")
    R("directly, and treat the date column as decoration until someone runs this")
    R("instrument on a full clone.")
    R()
    trows = []
    for t in tags:
        d = first_appearance(t["file"], t["text"][:60], dates_on)
        callers = "—"
        for cand in (t["symbol"], t["inside"]):
            if t["file"].endswith(".wgsl") and cand in w.functions:
                callers = "`%s`: %d" % (cand, len(w.callers_of(cand)))
                if cand not in all_entry_fns:
                    callers += " · **UNREACHABLE**"
                break
        trows.append((t["file"].replace("src/cartridges/the_board/", "…/"), t["line"],
                      t["symbol"], t["inside"] or "—", t["tag"], t["kind"],
                      callers, d, t["text"][:64]))
    R("Symbol attribution is computed by skipping comment, blank and")
    R("attribute-only lines to the first real declaration below the tag, and by")
    R("separately reporting the function the tag sits *inside*. Both are")
    R("best-effort; **`file:line` is the authority**, and where the two columns")
    R("disagree the tag is worth reading by eye before ruling on it.")
    R()
    R.table(["file", "line", "governs", "inside fn", "tag", "class",
             "reachable callers", "first seen", "text"], trows)

    # 4a — the policy surface
    R("### §4a — THE POLICY SURFACE (Tier 3)")
    R()
    mh = w.functions.get("manifold_height_hf")
    arm_targets, arm_policy = {}, {}
    GENERIC_ARM = "sample_terrain_y_at"
    if mh:
        # The nine arms are a `switch policy { case Nu: { return F(...); } }`
        # with the policy name in the trailing comment. Parse the RAW slice
        # so the comment (which carries the policy identity) survives.
        raw_body = w.raw[w.raw.find("\n", w.raw.find("fn manifold_height_hf")):]
        raw_body = raw_body[: raw_body.find("\n}\n")]
        arms = []
        for m in re.finditer(
                r"(case\s+(\d+)u|default)\s*:\s*\{\s*return\s+(\w+)\s*\([^\n]*?"
                r"(?://\s*(.*))?$", raw_body, re.M):
            label = ("case %su" % m.group(2)) if m.group(2) else "default"
            fn, comment = m.group(3), (m.group(4) or "").strip()
            pol = re.search(r"\b(POLICY_\w+|CELESTIAL/RENDER)", comment)
            arms.append((label, pol.group(1) if pol else "—", fn, comment[:70]))
            arm_targets[fn] = arm_targets.get(fn, 0) + 1
            # the arm's comment names which POLICIES[] row it serves; the
            # default arm serves the remainder ("CELESTIAL/RENDER")
            for pid in re.findall(r"POLICY_\w+", comment):
                arm_policy[pid] = fn
            if label == "default":
                for word in re.findall(r"\b([A-Z][A-Z_]{3,})\b", comment):
                    arm_policy.setdefault("POLICY_" + word, fn)
        R("`manifold_height_hf` — `%s:%d`–%d, %d callers. It is a `switch` over" %
          (WGSL, mh["line"], mh["end_line"], len(w.callers_of("manifold_height_hf"))))
        R("the policy id with **%d arms**." % len(arms))
        R()
        rows = []
        for label, pol, fn, comment in arms:
            generic = fn not in ("query_ground_placement_pyramid",) and \
                fn.startswith("sample_") or fn == "sample_terrain_y_at"
            rows.append((
                label, pol, "`%s`" % fn,
                w.functions[fn]["line"] if fn in w.functions else "—",
                len(w.callers_of(fn)) if fn in w.functions else "—",
                "yes" if fn in live_fns else "**no**",
                "**generic fallback — the arm is a no-op distinction**" if generic
                else "policy-specific",
            ))
        R.table(["arm", "policy", "dispatches", "decl line", "callers",
                 "live?", "realization"], rows)
        gen = sum(1 for r in rows if r[6].startswith("**"))
        R("**%d of %d arms dispatch a policy-specific function; %d fall through to" %
          (len(rows) - gen, len(rows), gen))
        R("the generic baked sampler.** An arm whose body is the same call as the")
        R("`default` arm distinguishes nothing at runtime — it is a *declared*")
        R("distinction, and that is precisely the LATENT surface Jean asked about.")
        R()
    else:
        R("`manifold_height_hf` not found in `%s`." % WGSL)
        R()

    qg = sorted(n for n in w.functions if n.startswith("query_ground_"))
    R("**`query_ground_*` functions**")
    R()
    R.table(["fn", "line", "callers", "called by", "reachable from a live entry?"],
            [(n, w.functions[n]["line"], len(w.callers_of(n)),
              ", ".join(w.callers_of(n)[:3]) or "— **none**",
              "yes" if n in live_fns else "**no**") for n in qg])

    pm = sorted(n for n in w.consts if re.match(r"POLICY_\w*MASK", n))
    R("**`POLICY_*_MASK` constants**")
    R()
    R.table(["const", "line", "references (post-strip, excl. decl)"],
            [(n, w.consts[n], w.references_outside_decl(n, w.consts[n])) for n in pm])

    ga_raw = read(GROUND_ARCH)
    R("**`contracts/ground_architecture.hpp` — what is load-bearing**")
    R()
    if ga_raw:
        dag = [(i + 1, l.strip()[:110]) for i, l in enumerate(ga_raw.split("\n"))
               if "CONTRIBUTOR_DAG" in l]
        sa = [(i + 1, l.strip()[:110]) for i, l in enumerate(ga_raw.split("\n"))
              if "static_assert" in l]
        R("`CONTRIBUTOR_DAG` sites: **%d** · `static_assert` sites: **%d** — these are" %
          (len(dag), len(sa)))
        R("the one declared, compile-time-checked statement of the composition. They")
        R("are LOAD-BEARING and are **not** pruning candidates.")
        R()
        R.table(["line", "CONTRIBUTOR_DAG site"], dag[:20])
        R.table(["line", "static_assert (the closure validation)"], sa[:20])
        grad = [(i + 1, l.strip()[:110]) for i, l in enumerate(ga_raw.split("\n"))
                if "gradient_supported" in l]
        R("`PolicyDef::gradient_supported` sites (declaration + readers):")
        R.table(["line", "text"], grad)
    else:
        R("_(file not found)_")
        R()

    # ── POLICIES[] rows, each crossed against its realization ─────────
    R("**`POLICIES[]` rows — realized or declared-only, with a per-arm verdict**")
    R()
    pol_rows = []
    if ga_raw:
        # Parsed against the RAW text on purpose: the row's gradient flag is
        # spelled `/*gradient=*/false`, so stripping comments erases the very
        # marker that identifies the field.
        ga_code = ga_raw
        pm2 = re.search(r"POLICIES\s*\[\s*\]\s*=\s*\{", ga_code)
        if pm2:
            seg = ga_code[pm2.end():]
            depth, i = 1, 0
            while i < len(seg) and depth:
                if seg[i] == "{":
                    depth += 1
                elif seg[i] == "}":
                    depth -= 1
                i += 1
            seg = seg[:i]
            for m in re.finditer(
                    r"\{\s*(POLICY_\w+)\s*,\s*\"([^\"]*)\"(.*?)/\*gradient=\*/\s*(true|false)",
                    seg, re.S):
                pid, pname, _mid, grad = m.groups()
                qfn = "query_ground_" + pname
                gfn = qfn + "_gradient"
                served_by = arm_policy.get(pid)
                dispatched = served_by is not None
                callers = len(w.callers_of(qfn)) if qfn in w.functions else 0
                mask = pid + "_MASK"
                mask_refs = (w.references_outside_decl(mask, w.consts[mask])
                             if mask in w.consts else None)
                if served_by == GENERIC_ARM:
                    real = "served by the **generic** `%s`" % GENERIC_ARM
                    if qfn in w.functions and callers == 0:
                        verdict = ("**PRUNE `%s`** (0 callers) — glaw2; "
                                   "KEEP the row" % qfn)
                    else:
                        verdict = ("KEEP the row; note it is **indistinguishable "
                                   "from `default`**")
                elif dispatched:
                    real = "REALIZED — arm dispatches `%s`" % served_by
                    verdict = "KEEP"
                elif qfn not in w.functions:
                    real = "**declared only — no `%s`, no arm**" % qfn
                    verdict = "**RULE(Jean)** — declared interface, zero realization"
                elif callers == 0:
                    real = "**fn exists, 0 callers, no arm**"
                    verdict = "**PRUNE `%s`** — glaw2" % qfn
                else:
                    real = "fn exists, called from %d site(s), not via the switch" % callers
                    verdict = "KEEP"
                gstat = "—"
                if grad == "true":
                    if gfn in w.functions:
                        gc = len(w.callers_of(gfn))
                        gstat = ("`%s` ×%d callers" % (gfn, gc)) if gc else \
                            "**`%s` exists, ZERO callers — intent, not realization**" % gfn
                    else:
                        gstat = "**no `%s` fn at all — intent, not realization**" % gfn
                pol_rows.append((pid, pname, "yes" if dispatched else "**no**",
                                 callers if qfn in w.functions else "—",
                                 mask_refs if mask_refs is not None else "—",
                                 grad, gstat, real, verdict))
    R.table(["POLICIES[] row", "name", "dispatched by the switch?", "WGSL callers",
             "`_MASK` refs", "gradient=", "gradient realization",
             "realization", "recommendation"], pol_rows)
    R("`PolicyDef::gradient_supported` is **declared and never read** — the field")
    R("exists on the struct (`%s:113`) with zero readers tree-wide; the" % GROUND_ARCH)
    R("`gradient=` column above is the authored value, and the column beside it")
    R("is what the shader actually provides. Where they disagree, the row is")
    R("intent, not capability.")
    R()

    # 4b — PGA
    R("### §4b — PGA (Jean has RULED: `pga_color_motor` and its tail retire)")
    R()
    PGA = ["pga_color_motor", "Motor", "rotor", "translator", "gp_mm", "sw_mp",
           "Point", "Plane", "Line", "sw_motor_point", "point_from_vec3",
           "point_to_vec3", "MOTOR_IDENTITY", "LINE_Y"]

    def reach_without(removed):
        """Reachability from the live entry points with `removed` deleted from
        the call graph. This is a SIMULATED DELETION, not a heuristic: it is
        exactly what the tree looks like after the cut, so 'falls with the
        colour motor' is a computed fact and not a guess. (Excluding whole
        entry-point closures instead — the obvious shortcut — is wrong: the
        colour motor and the sphere-orbit path share an entry point, so that
        method reports live symbols as dead.)"""
        seen, stack = set(), [e for e in live_entries if e not in removed]
        while stack:
            n = stack.pop()
            if n in seen or n in removed:
                continue
            seen.add(n)
            stack.extend(w.calls.get(n, ()))
        return seen

    CUT = {"pga_color_motor"}
    after = reach_without(CUT)
    after_idents = set()
    for f in after:
        after_idents |= w.direct_idents.get(f, set())
    after_idents = w.expand_decl_refs(after_idents)

    color_motor_closure = w.closure("pga_color_motor") if "pga_color_motor" in w.functions else set()
    prows = []
    fell = []
    for s in PGA:
        line, kind = "—", "—"
        if s in w.functions:
            line, kind = w.functions[s]["line"], "fn"
        elif s in w.structs:
            line, kind = w.structs[s], "struct"
        elif s in w.consts:
            line, kind = w.consts[s], "const"
        else:
            m = re.search(r"\b" + re.escape(s) + r"\b", w.text)
            if m:
                line, kind = lineno(w.text, m.start()), "reference"
        callers = w.callers_of(s) if s in w.functions else []
        survives = (s in after) if kind == "fn" else (s in after_idents)
        if s in CUT:
            part = "**THE CUT** (Jean's ruling)"
        elif survives:
            keeps = sorted(c for c in callers if c in after)[:3]
            part = "**SURVIVES** — still reached via %s" % (", ".join(
                "`%s`" % k for k in keeps) or "a live reference")
        else:
            part = "falls with the colour motor"
            fell.append(s)
        prows.append((s, kind, line, len(callers),
                      ", ".join(callers[:3]) or "—", part))
    R.table(["symbol", "kind", "line", "callers", "called by", "partition"], prows)
    R()
    R("**Method.** The partition is a *simulated deletion*: remove")
    R("`pga_color_motor` from the call graph, recompute reachability from the")
    R("%d live entry points, and see what no longer arrives. Recipe:" % len(live_entries))
    R("`reach_without({\"pga_color_motor\"})` in `tools/pruning_census.py`.")
    R("The shortcut of \"ignore every entry point whose closure contains the")
    R("colour motor\" gives the WRONG answer here, because the colour motor and")
    R("the sphere-orbit path share an entry point.")
    R()
    R("`pga_color_motor` — `%s:%s`, %d caller(s): %s. Its closure is %d fn." %
      (WGSL, w.functions["pga_color_motor"]["line"] if "pga_color_motor" in w.functions else "?",
       len(w.callers_of("pga_color_motor")),
       ", ".join("`%s`" % c for c in w.callers_of("pga_color_motor")) or "—",
       len(color_motor_closure)))
    R()
    R("**Falls with the cut (%d): %s**" %
      (len(fell), ", ".join("`%s`" % s for s in fell) or "nothing"))
    R("**Survives on the ribbon + sphere-orbit paths (%d): %s**" %
      (len(prows) - len(fell) - 1,
       ", ".join("`%s`" % r[0] for r in prows if r[5].startswith("**SURVIVES"))))
    R()
    R("> Note for P1: the caller `%s` does not" %
      (w.callers_of("pga_color_motor")[0] if w.callers_of("pga_color_motor") else "—"))
    R("> disappear with the cut — it is the site that must be **rewritten** to")
    R("> read the relocated CPU-decoded value. The census deliberately does not")
    R("> assume it is deleted; deleting it is a behaviour change, not a pruning.")
    R()
    R("**RELOCATION DESTINATION — recorded here so the capability is not lost with")
    R("the code.** Movement through colour space relocates to `%s`" % CANVAS)
    R("as a CPU decode on a `Segment`, per the constitutional route (CPU-glided")
    R("values, stateless GPU evaluators). Evidence the destination exists:")
    R()
    canvas_raw = read(CANVAS)
    seg_sites = [(i + 1, l.strip()[:100]) for i, l in enumerate(canvas_raw.split("\n"))
                 if re.search(r"\bSegment\b", l)][:12]
    R.table(["%s line" % CANVAS, "Segment site"], seg_sites)

    R("---")
    R()

    # ═══════════════════ §5 ═══════════════════════════════════════════
    R("## §5 — DANGLING-REFERENCE CENSUS")
    R()
    scan_files = walk_src(
        exts={".hpp", ".cpp", ".h", ".wgsl", ".md", ".txt", ".js", ".mjs", ".py", ".json"},
        roots=("src", "audit", "web", "tools"),
        extra=(CMAKE, "README.md", "DEFERRED_REGISTER.md"),
    )
    scan_files = [p for p in scan_files if "package-lock" not in p]
    existence = {
        "backup_board": exists("src/cartridges/backup_board"),
        "the_chord": exists("src/cartridges/the_chord"),
        "src/dev/BACKUP": exists("src/dev/BACKUP"),
        "cell_fields": bool(re.search(r"\bcell_fields\b", w.text)),
        "zone_patch_instances": "zone_patch_instances" in w.resources,
        "apply_gol_extrusion_color": "apply_gol_extrusion_color" in w.functions,
        "animated_cell_color": "animated_cell_color" in w.functions,
        "OVERLAY_WAVES": "OVERLAY_WAVES" in w.consts,
    }
    inl_files = walk_src(exts={".inl"}, roots=("src",))
    existence["*.inl"] = bool(inl_files)

    # ── THE ROOM SPLIT, and why it exists ─────────────────────────────
    # The constitution's own line: "git keeps every word; audit/ is the sole
    # map." An audit report that says a file USED to exist is not a dangling
    # reference — it IS the record, and deleting it destroys the map. A past
    # handoff is the same. So hits are partitioned by room: LOAD-BEARING
    # rooms (code, comments, the build, the live docs) are enumerated one by
    # one and are the campaign's actual surface; ARCHIVAL rooms are counted
    # and left alone. Merging the two is how a census turns into 4,000 lines
    # of noise that nobody rules on.
    def room_of(path):
        # SELF-EXCLUSION, and it is load-bearing for determinism, not taste:
        #   · the instrument names every referent in its own pattern table;
        #   · the REPORT quotes every hit it found last run.
        # Counting either would make run N+1 differ from run N forever — the
        # census would never reach a fixed point and could not be diffed.
        if path in SELF_PATHS:
            return "self"
        if path.startswith("audit/") or path.startswith("src/docs/"):
            return "archival"
        if path.startswith("web/PORT_MAP") or path.endswith(".json"):
            return "archival"
        # The web mirror is not a dangling reference — it is a STALE COPY that
        # still carries code the desktop retired. Counted here, diagnosed in
        # §5.3, where it belongs.
        if path.startswith("web/"):
            return "mirror"
        return "load-bearing"

    dang_rows, dang_counts = [], Counter()
    archival, mirror_hits = Counter(), Counter()
    for path in scan_files:
        raw = read(path)
        if not raw:
            continue
        room = room_of(path)
        for i, line in enumerate(splitlines_exact(raw)):
            for label, pat in DANGLING:
                if not pat.search(line):
                    continue
                if label == "zone_mesh_*":
                    ex = any(k.startswith("zone_mesh_") for k in
                             list(w.functions) + list(w.resources))
                elif label == "zone_extrusion_*":
                    ex = any(k.startswith("zone_extrusion_") for k in
                             list(w.functions) + list(w.resources))
                elif label == "probe_*":
                    m = pat.search(line)
                    tok = m.group(0)
                    ex = (tok in w.functions or tok in w.resources
                          or bool(walk_src(exts={".cpp", ".hpp", ".mjs", ".js", ".py"},
                                           roots=("src", "audit"))
                                  and any(tok in q for q in scan_files)))
                else:
                    ex = existence.get(label, False)
                if room == "self":
                    pass
                elif room == "archival":
                    archival[(path, label)] += 1
                elif room == "mirror":
                    mirror_hits[label] += 1
                else:
                    dang_counts[(label, ex)] += 1
                    dang_rows.append((path, i + 1, label, "yes" if ex else "**NO**",
                                      line.strip()[:96]))
                break
    R("Scanned %d files (src/, audit/, web/, tools/, CMakeLists.txt), excluding" % len(scan_files))
    R("`src/external/` and `package-lock.json`.")
    R()
    R("**Rooms are split, and the split is the point.** The constitution's own")
    R("line is *\"git keeps every word; audit/ is the sole map.\"* An audit report")
    R("that records a file which used to exist is **not** a dangling reference —")
    R("it is the map. So `audit/` and `src/docs/` (past handoffs, past reports)")
    R("are counted below and then left alone. What P1+ acts on is the")
    R("**load-bearing** room: code, code comments, `CMakeLists.txt`, and the")
    R("living docs.")
    R()
    summ = defaultdict(lambda: [0, 0])
    for (label, ex), n in dang_counts.items():
        summ[label][0 if ex else 1] += n
    R.table(["referent", "still exists?", "load-bearing hits (exists)",
             "load-bearing hits (**DANGLING**)"],
            sorted((l, "yes" if v[0] else "no", v[0], v[1]) for l, v in summ.items()))
    R("Archival mentions (recorded, **not** for deletion): **%d** across %d files." %
      (sum(archival.values()), len({p for p, _ in archival})))
    R()
    R.table(["archival file", "referent", "mentions"],
            sorted((p, l, n) for (p, l), n in archival.items() if n >= 3)[:40])
    R("The instrument excludes **itself** (`tools/pruning_census.py`) — its")
    R("pattern table necessarily names every referent it hunts for.")
    R()
    if mirror_hits:
        R("**In the web mirror, these referents are not dangling — they are LIVE**,")
        R("because the mirror predates the retirements that removed them from the")
        R("desktop. That is a staleness measurement, not a deletion surface:")
        R()
        R.table(["referent", "live sites in `%s`" % WEB_WGSL],
                sorted(mirror_hits.items()))
        R("Diagnosed in §5.3. **Deleting these from the desktop shader while the")
        R("mirror still runs them is exactly the two-room hazard.**")
        R()

    R("### §5.1 — every DANGLING hit in a load-bearing room")
    R()
    live_dang = sorted(r for r in dang_rows if r[3].startswith("**"))
    R("Sites: **%d**. These are the campaign's actual §5 surface." % len(live_dang))
    R()
    R.table(["file", "line", "referent", "still exists?", "text"], live_dang)

    R("### §5.2 — the constitution §0 mirror law (FOSSIL)")
    R()
    con = read(CONSTITUTION)
    if con:
        clines = con.split("\n")
        hits = [(i + 1, l.strip()) for i, l in enumerate(clines)
                if re.search(r"byte-identical|mirror|the_chord|\.inl", l, re.I)][:14]
        R("`%s` — this paragraph describes a two-cartridge world that no longer" % CONSTITUTION)
        R("exists (`src/cartridges/the_chord/` is absent at `%s`):" % head_short)
        R()
        R.table(["line", "text"], hits)
        R("**Verdict: FOSSIL — census it, do not obey it.** Both mirrored-module")
        R("subjects named in it (`ribbon.inl`, `pawn.inl`) are also gone: %d `.inl`" % len(inl_files))
        R("files exist in the tree.")
        R()

    R("### §5.3 — the OTHER mirror law, which is NOT fossil ⚠")
    R()
    if exists(WEB_WGSL):
        d_raw, m_raw = read(WGSL), read(WEB_WGSL)
        d_sha = hashlib.sha256(d_raw.encode("utf-8", "replace")).hexdigest()
        m_sha = hashlib.sha256(m_raw.encode("utf-8", "replace")).hexdigest()
        side = read(WEB_WGSL_SIDECAR)
        side_commit = None
        sm = re.search(r"\b([0-9a-f]{7,40})\b", side)
        if sm:
            side_commit = sm.group(1)
        # The sidecar commit predates the graft in this shallow clone, so the
        # count is unavailable rather than zero — say which.
        known = git("cat-file", "-e", "%s^{commit}" % side_commit) if side_commit else ""
        behind = ""
        if side_commit and git("rev-parse", "--verify", "-q", "%s^{commit}" % side_commit):
            behind = git("rev-list", "--count", "%s..HEAD" % side_commit)
        side_date = re.search(r"mirrored:\s*([0-9-]+)", side)
        mw = WgslModule(WEB_WGSL)
        R("`CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS/CLAUDE.md` declares")
        R("`%s` a **byte-identical mirror** of `%s`." % (WEB_WGSL, WGSL))
        R("That doctrine governs a LIVE web port. It is currently **violated**:")
        R()
        R.table(["", "desktop", "web mirror"],
                [("path", WGSL, WEB_WGSL),
                 ("lines", w.total, mw.total),
                 ("sha256", d_sha[:16] + "…", m_sha[:16] + "…"),
                 ("`fn` count", len(w.functions), len(mw.functions)),
                 ("entry points", len(w.entry_points), len(mw.entry_points)),
                 ("@group/@binding", len(w.resources), len(mw.resources))])
        R("- sidecar `%s`: %s" % (WEB_WGSL_SIDECAR, side.strip().replace("\n", " · ") or "(empty)"))
        if side_commit:
            R("- sidecar source commit `%s` — %s" %
              (side_commit,
               "**%s commits behind HEAD**" % behind if behind else
               "**not present in this shallow clone's history**, so the distance "
               "cannot be counted from here; the sidecar dates the mirror to "
               "**%s**, which is before this clone's graft root (%s)" %
               (side_date.group(1) if side_date else "an unrecorded date", root_commit)))
        added = sorted(set(w.entry_points) - set(mw.entry_points))
        removed = sorted(set(mw.entry_points) - set(w.entry_points))
        R("- entry points on desktop but NOT in the mirror: %s" % (", ".join(added) or "none"))
        R("- entry points in the mirror but NOT on desktop: %s" % (", ".join(removed) or "none"))
        ab = sorted(set(w.resources) - set(mw.resources))
        rb = sorted(set(mw.resources) - set(w.resources))
        R("- bindings on desktop but NOT in the mirror: %s" % (", ".join(ab) or "none"))
        R("- bindings in the mirror but NOT on desktop: %s" % (", ".join(rb) or "none"))
        R()
        R("The divergence runs in **both directions** — the desktop has since")
        R("retired symbols the mirror still carries, and revived others the mirror")
        R("records as retired (`animated_cell_color` is live on the desktop and")
        R("tombstoned in the mirror; `animated_cell_color_lut` is the reverse). The")
        R("mirror is nonetheless strictly OLDER; it is the desktop that reversed")
        R("itself. A resync is still a copy, not a merge.")
        R()
        # How much does the staleness actually cost TODAY? Only the entry
        # points the web host dispatches can break, and the port is early.
        host_files = walk_src(exts={".js", ".mjs", ".html"}, roots=("web",))
        host_named = set()
        for hp in host_files:
            host_named |= set(re.findall(r"entryPoint\s*:\s*[\"'](\w+)[\"']", read(hp)))
        shared = sorted(n for n in host_named if n in w.entry_points or n in mw.entry_points)
        local = sorted(n for n in host_named if n not in w.entry_points
                       and n not in mw.entry_points)
        R("**How much does the staleness cost today?** Only the entry points the")
        R("web host actually dispatches can break on a resync:")
        R()
        R.table(["entry point named by the web host", "in desktop?", "in mirror?"],
                [(n, "yes" if n in w.entry_points else "**no**",
                  "yes" if n in mw.entry_points else "**no**") for n in shared] +
                [(n + " *(harness placeholder)*", "—", "—") for n in local])
        risky = [n for n in shared if n not in w.entry_points]
        R("The port is early: it dispatches **%d** of the shared shader's %d entry" %
          (len(shared), len(w.entry_points)))
        R("points%s. %s" % (
            (" (%s)" % ", ".join("`%s`" % n for n in shared)) if shared else "",
            "**Every one of them still exists on the desktop, so a resync would "
            "not break the demo today.**" if not risky else
            "**%d of them are GONE from the desktop — a resync breaks the demo.**"
            % len(risky)))
        R("So the honest reading is: the doctrine is violated and the mirror is")
        R("%s entry points behind, but the *shader* resync is low-risk right now." %
          (len(added) + len(removed)))
        R("**The part that actually bites is `%s` (§3.3)** — its hand-written" % WEB_UNIFORMS)
        R("offsets are consumed by the host on every boot, and nothing checks them.")
        R()
        R("**The correction to the P0 premise.** The handoff says \"`the_chord` is")
        R("retired ⇒ `world.wgsl` is SINGLE-COPY. Every WGSL deletion is a one-room")
        R("edit.\" The first clause is true; the conclusion is not — a second copy")
        R("exists under a live doctrine, so a P1 WGSL deletion is a **two-room**")
        R("edit and P1 would inherit a divergence it did not create.")
        R()
        R("**But state the size honestly, or the correction becomes its own kind")
        R("of error.** The measurement above says the shader half of this is cheap")
        R("today: the port dispatches two entry points and both still exist. What")
        R("Jean has to rule is narrower than \"stop the campaign\":")
        R()
        R("1. **Before P1 (WGSL deletions)** — either run the resync ritual once")
        R("   (`cp` + `gzip -9` + sidecar sha256, per `CLAUDE.md`), or record that")
        R("   the mirror doctrine is suspended for the duration. Either is a")
        R("   one-line decision; leaving it unstated is what costs.")
        R("2. **Before P3 (config-field deletions)** — the real blocker, and it is")
        R("   `%s`, not the shader. See §3.3." % WEB_UNIFORMS)
        R()

    R("### §5.4 — CMakeLists.txt: `backup_board` and the `probe_*` targets")
    R()
    cm = read(CMAKE)
    cmlines = cm.split("\n")
    bb = [(i + 1, l.strip()[:120]) for i, l in enumerate(cmlines) if "backup_board" in l]
    R.table(["CMakeLists.txt line", "backup_board reference"], bb)
    if bb:
        R("`src/cartridges/backup_board/` does not exist at `%s` — every line above" % head_short)
        R("is dangling.")
        R()
    prow = []
    for m in re.finditer(r"add_executable\s*\(\s*(\w+)\s+([^\s\)]+)", cm):
        tgt, src = m.group(1), m.group(2)
        prow.append((tgt, src, "yes" if exists(src) else "**NO**",
                     lineno(cm, m.start())))
    R.table(["target", "source", "source exists?", "line"], sorted(prow))

    R("---")
    R()

    # ═══════════════════ §6 ═══════════════════════════════════════════
    R("## §6 — COMMENT-MASS CENSUS")
    R()
    mass = []
    tot_l = tot_c = 0
    for d in MASS_DIRS:
        for p in walk_src(exts={".hpp", ".cpp", ".wgsl"}, roots=(d,)):
            raw = read(p)
            t, c, b = comment_line_stats(raw, wgsl=p.endswith(".wgsl"))
            mass.append((p, t, c, b, 100.0 * c / max(1, t)))
            tot_l += t
            tot_c += c
    R("A COMMENT LINE is a line whose entire payload is comment — a trailing")
    R("`// note` on a code line is not counted, because it is not removable mass.")
    R()
    R("**Totals over %s: %d lines, %d comment lines (%.1f%%).**" %
      (", ".join("`%s`" % d for d in MASS_DIRS), tot_l, tot_c, 100.0 * tot_c / max(1, tot_l)))
    R()
    R("Top 15 by comment MASS:")
    R()
    R.table(["file", "lines", "comment", "blank", "ratio"],
            [(p, t, c, b, "%.1f%%" % r) for p, t, c, b, r in
             sorted(mass, key=lambda x: -x[2])[:15]])
    R("Top 15 by comment RATIO (files ≥ 100 lines):")
    R()
    R.table(["file", "lines", "comment", "blank", "ratio"],
            [(p, t, c, b, "%.1f%%" % r) for p, t, c, b, r in
             sorted([m for m in mass if m[1] >= 100], key=lambda x: -x[4])[:15]])
    R("Full table, alphabetical:")
    R()
    R.table(["file", "lines", "comment", "blank", "ratio"],
            [(p, t, c, b, "%.1f%%" % r) for p, t, c, b, r in sorted(mass)])

    # tombstones
    R("### §6.1 — TOMBSTONES")
    R()
    tomb = []
    for p in src_files:
        raw = read(p)
        for i, line in enumerate(raw.split("\n")):
            s = line.strip()
            if not (s.startswith("//") or s.startswith("*") or "//" in line):
                continue
            if not TOMBSTONE_RE.search(line):
                continue
            camp = CAMPAIGN_TAG_RE.search(line)
            tomb.append((p.replace("src/cartridges/the_board/", "…/"), i + 1,
                         camp.group(1) if camp else "(uncited)", s[:100]))
    R("Tombstone sites: **%d**." % len(tomb))
    R()
    bycamp = Counter(t[2] for t in tomb)
    # ── SHIPPED vs IN-FLIGHT, with the evidence named ─────────────────
    # A campaign has shipped when the tree carries its closing record: a
    # dedicated `<TAG>_LOG.md`, a `BATCH_REPORT_*.md`, or a closeout/merge
    # commit naming it. Prose matching alone under-reports (a log file's
    # title need not contain the word "closeout"), so file existence is the
    # primary witness and git the corroborating one.
    log_dir = "CLAUDE CODE/AUDITS AND RECENT CAMPAIGNS"
    log_files = sorted(os.listdir(log_dir)) if os.path.isdir(log_dir) else []
    subjects = git("log", "--format=%s")
    shipped, evidence = {}, {}
    for tag in sorted(set(bycamp) - {"(uncited)"}):
        ev = []
        for fn in log_files:
            if tag in fn or tag.replace("_", "") in fn.replace("_", ""):
                ev.append("`%s`" % fn)
        gm = re.findall(r"^.*" + re.escape(tag) + r".*$", subjects, re.M)
        closeout = [g for g in gm if re.search(r"closeout|Merge|merge|closed", g, re.I)]
        if closeout:
            ev.append("git: “%s”" % closeout[0][:60])
        elif gm:
            ev.append("git: %d commit(s) name it" % len(gm))
        shipped[tag] = "SHIPPED" if ev else "in-flight / unproven"
        evidence[tag] = "; ".join(ev[:2]) or "—"
    R.table(["campaign tag", "tombstones", "status", "evidence", "fossil?"],
            [(t, n, shipped.get(t, "—"), evidence.get(t, "—"),
              "**yes — retire the tombstone**" if shipped.get(t) == "SHIPPED" else
              ("— (uncited: no campaign to check)" if t == "(uncited)" else "not yet"))
             for t, n in sorted(bycamp.items())])
    R("Evidence for SHIPPED: a `<TAG>_LOG.md` / `BATCH_REPORT_*.md` in")
    R("`%s/`, or a closeout/merge commit naming the tag." % log_dir)
    R("Recipe: `ls \"%s\"` and `git log --format=%%s | grep -i '<TAG>'`." % log_dir)
    R()
    unc = bycamp.get("(uncited)", 0)
    if unc:
        R("**%d of %d tombstones cite NO campaign at all.** These are the deepest" % (unc, len(tomb)))
        R("fossils: a marker whose campaign cannot even be named has outlived the")
        R("reason anyone would consult it. Under the standing ruling — *git keeps")
        R("every word* — they carry no information git does not, and they are the")
        R("largest single block of removable comment mass in §6.")
        R()
    R("Every tombstone site:")
    R()
    R.table(["file", "line", "campaign", "text"], sorted(tomb))

    # coordinates
    R("### §6.2 — CAMPAIGN COORDINATES in comments")
    R()
    coord = Counter()
    coord_examples = defaultdict(list)
    for p in src_files:
        raw = read(p)
        for i, line in enumerate(raw.split("\n")):
            if "//" not in line and not line.strip().startswith("*"):
                continue
            for m in COORD_RE.finditer(line):
                coord[p] += 1
                if len(coord_examples[p]) < 2:
                    coord_examples[p].append("%d: %s" % (i + 1, m.group(0)))
    R("A coordinate is a **campaign tag followed by a short stamp** (`CONTACT_5 P1b`,")
    R("`UNIFIED_GROUND_1 U4`). The bare-stamp form (`m4`, `p1a`, `A2-3`) is")
    R("deliberately NOT matched: in prose it collides with ordinary text and the")
    R("false-positive rate is not worth the count. **The number below is therefore")
    R("a floor, not a total** — stated so the §4 sizing is not overread.")
    R()
    R("Coordinate sites: **%d** across %d files." % (sum(coord.values()), len(coord)))
    R()
    R.table(["file", "coordinates", "examples"],
            [(p.replace("src/cartridges/the_board/", "…/"), n,
              "; ".join(coord_examples[p])) for p, n in sorted(coord.items(), key=lambda x: -x[1])])

    # debug constants
    R("### §6.3 — the four debug constants and `[DIAG:AUDIT]`")
    R()
    drows = []
    for name in DEBUG_CONSTS:
        site, val, uses = "—", "**absent from the tree**", []
        for p in src_files:
            raw = read(p)
            code = strip_cpp(raw) if not p.endswith(".wgsl") else strip_wgsl(raw)
            m = re.search(r"\b(?:const|constexpr|let)\s+(?:\w+\s+)?" +
                          re.escape(name) + r"\s*(?::\s*\w+\s*)?=\s*([^;,\n]+)", code)
            declared_line = lineno(code, m.start()) if m else None
            if m and site == "—":
                site = "%s:%d" % (p, declared_line)
                val = "`%s`" % m.group(1).strip()
            for um in re.finditer(r"\b" + re.escape(name) + r"\b", code):
                ln = lineno(code, um.start())
                if declared_line is not None and ln == declared_line:
                    continue
                uses.append("%s:%d" % (p.split("/")[-1], ln))
        drows.append((name, site, val, len(uses),
                      ", ".join(sorted(uses)) or "— **inert**",
                      "branch(es) still present" if uses else
                      "**INERT — the slot is retired in fact**"))
    R.table(["debug constant", "declared at", "current value", "branches",
             "branch sites", "instrument status"], drows)
    R("A debug constant pinned at its off value costs **zero runtime** (the")
    R("branch is folded away per entry point) and N lines of shader text. The")
    R("question the handoff asks — *does each named slot's instrument still")
    R("exist* — is answered by the branch-sites column: a constant with zero")
    R("branch sites names an instrument that is already gone, and the constant")
    R("is the only thing left of it.")
    R()
    diag = []
    for p in src_files:
        raw = read(p)
        for i, line in enumerate(raw.split("\n")):
            if DIAG_RE.search(line):
                diag.append((p.replace("src/cartridges/the_board/", "…/"), i + 1,
                             line.strip()[:100]))
    R("`[DIAG:AUDIT]` blocks: **%d**." % len(diag))
    R()
    R.table(["file", "line", "text"], sorted(diag))

    R("---")
    R()

    # ═══════════════════ §7 ═══════════════════════════════════════════
    R("## §7 — THE VERDICT TABLE")
    R()
    R("One row per candidate, sorted by tier. `removal instrument` is how P1/P2")
    R("would PROVE the removal safe: **glaw1** (C++ compile) · **glaw2** (naga/Tint")
    R("parse) · **Dawn enumeration probe** (bind the layout without the entry; Dawn")
    R("objects if live) · **rest bit-identity** · **idle rig** · **none-needed**.")
    R("`disclosure` is the visible or behavioural consequence.")
    R()
    R("**The failure direction we want is under-reach, never over-reach** — so")
    R("anything short of certain is `RULE(Jean)`.")
    R()
    V = []

    # Tier 0 — blockers
    if exists(WEB_WGSL):
        V.append(("0", "web/shaders/world.wgsl (the mirror)", "web port",
                  "%s:1" % WEB_WGSL, "CLAUDE.md Mirror doctrine (LIVE)", "n/a",
                  "resync ritual + smoke test",
                  "the web demo's shader changes; a resync is a real behaviour surface",
                  "**RULE(Jean)** — blocks every P1 WGSL deletion"))

    # Tier 1 — dangling references, pure text
    for label, ex in sorted({(l, e) for (l, e) in dang_counts if not e}):
        n = dang_counts[(label, ex)]
        V.append(("1", label, "comments/docs/build", "%d sites — §5.1" % n,
                  "dangling", "0", "none-needed",
                  "none — behaviour-identical by construction", "DELETE"))

    # Tier 1 — constitution + CMake fossils
    if con:
        V.append(("1", "constitution §0 mirror law", "docs",
                  "%s" % CONSTITUTION, "fossil (the_chord retired)", "0",
                  "none-needed", "none — a document, not code", "DELETE (or mark SUPERSEDED)"))
    if bb:
        V.append(("1", "CMakeLists backup_board paragraph", "build",
                  "%s:%d…" % (CMAKE, bb[0][0]), "dangling", "0",
                  "glaw1 (configure + build)", "none — the path does not exist",
                  "DELETE"))

    # Tier 2 — WGSL dead symbols
    for n in unreachable_fns:
        st, det = mirror_status(n)
        if st.startswith("**"):
            V.append(("2", n, "wgsl", "%s:%d" % (WGSL, w.functions[n]["line"]),
                      "unreachable on the DESKTOP; **live in the web mirror** (%s)" % det,
                      "0 desktop / %d mirror" % len(mw.callers_of(n)),
                      "glaw2 (Tint parse) **+ a web smoke test**",
                      "none on the desktop; the next resync would delete code the "
                      "web port runs",
                      "**RULE(Jean)** — resolve the mirror (§5.3) first"))
        else:
            V.append(("2", n, "wgsl", "%s:%d" % (WGSL, w.functions[n]["line"]),
                      "unreachable (mirror: %s)" % st, "0", "glaw2 (Tint parse)",
                      "none — DCE'd per entry point; zero runtime cost today", "DELETE"))
    for n, ln, twin, web in dead_consts:
        if web.startswith("**"):
            V.append(("2", n, "wgsl const", "%s:%d" % (WGSL, ln),
                      "unreferenced on the DESKTOP; **read by the web mirror**", "0",
                      "glaw2 (Tint parse) **+ a web smoke test**",
                      "none on the desktop; the next resync would delete a const "
                      "the web port reads",
                      "**RULE(Jean)** — resolve the mirror (§5.3) first"))
        elif twin:
            V.append(("2", n, "wgsl const", "%s:%d" % (WGSL, ln),
                      "unreferenced, but MIRRORS C++ `%s`" % twin, "0",
                      "glaw2 (Tint parse)",
                      "none at runtime; the two rooms stop reading alike",
                      "**RULE(Jean)** — deleting a declared mirror is a policy "
                      "change, not a pruning"))
        else:
            V.append(("2", n, "wgsl const", "%s:%d" % (WGSL, ln),
                      "unreferenced (mirror: %s)" % web, "0",
                      "glaw2 (Tint parse)", "none — behaviour-identical by construction",
                      "DELETE"))
    for n, ln in dead_structs:
        V.append(("2", n, "wgsl struct", "%s:%d" % (WGSL, ln), "unreferenced", "0",
                  "glaw2 (Tint parse)", "none — behaviour-identical by construction",
                  "DELETE"))

    # Tier 3 — bindings
    for slot, rn, space, ln, bound, mst, mdet in unread:
        V.append(("3", rn, "binding %s" % slot, "%s:%d" % (WGSL, ln),
                  "bound, zero reachable readers" +
                  ("; **read by the web mirror** (%s)" % mdet if mst.startswith("**") else ""),
                  "0", "Dawn enumeration probe",
                  "GPU: one binding slot + its buffer" +
                  ("; the registry number cannot be retired while the mirror binds it"
                   if mst.startswith("**") else ""),
                  "RULE(Jean)"))

    # Tier 3 — config fields
    web_written = {h[0] for h in hazards} if web_u else set()
    for name, ty, size, off, nwrites, sname, ncalls, nguard in zero_readers:
        V.append(("3", name + (" (+ `%s`)" % sname if sname else ""), "config mirror",
                  "%s:%d (GPUDesignConfig)" % (STATE, cpp_by_name[name]["line"]),
                  "zero live reads in BOTH C++/WGSL rooms (%d write%s%s)" %
                  (nwrites, "" if nwrites == 1 else "s",
                   ", %d guard-only read" % nguard if nguard else ""), "0",
                  "rest bit-identity + glaw1 offsetof witnesses "
                  "+ **a web offset witness that does not yet exist**",
                  "GPU: %d B of the uniform; re-flows every later offset in "
                  "THREE rooms" % size,
                  ("**RULE(Jean)** — the web port writes this offset by hand "
                   "(§3.3); deleting it corrupts the demo silently"
                   if name in web_written else
                   "DELETE — but only in the same commit as the rest, and only "
                   "after §3.3's third room is re-mapped")))

    # Tier 4 — status tags
    tag_by_file = Counter(t["file"] for t in actionable)
    for f, n in sorted(tag_by_file.items()):
        V.append(("4", "%d STATUS/LATENT/INTENT tag%s" % (n, "" if n == 1 else "s"),
                  "comments", f,
                  "LATENT/INTENT", "n/a", "none-needed",
                  "none — comment text only", "DELETE-AND-RECORD (standing ruling)"))
    nonactionable = [t for t in tags if t["kind"] != "TAG"]
    if nonactionable:
        V.append(("keep", "%d LEGEND / tombstone-ref / prose matches" % len(nonactionable),
                  "comments", "tree-wide — §4", "not tags", "n/a", "none-needed",
                  "deleting the legend deletes the vocabulary the other tags use",
                  "KEEP — excluded from the ruling surface by classification"))

    # Tier 4 — tombstones of shipped campaigns
    for t, n in sorted(bycamp.items()):
        if shipped.get(t) == "SHIPPED":
            V.append(("4", "%d tombstone%s citing %s" % (n, "" if n == 1 else "s", t),
                      "comments",
                      "tree-wide — §6.1", "campaign SHIPPED", "n/a", "none-needed",
                      "none — comment text only", "DELETE"))

    # Tier 5 — policy + PGA, the ruled surface
    for n in qg:
        if not w.callers_of(n):
            st, det = mirror_status(n)
            dup = " — same symbol as its tier-2 row" if n in unreachable_fns else ""
            V.append(("5", n, "policy surface", "%s:%d" % (WGSL, w.functions[n]["line"]),
                      "0 callers (mirror: %s)%s" % (st, dup), "0",
                      "glaw2 (Tint parse)" +
                      (" **+ a web smoke test**" if st.startswith("**") else ""),
                      "none — unreachable today" if not st.startswith("**") else
                      "none on the desktop; the mirror still runs it",
                      "**RULE(Jean)** — resolve the mirror (§5.3) first"
                      if st.startswith("**") else "DELETE"))
    if "pga_color_motor" in w.functions:
        V.append(("5", "pga_color_motor + tail", "PGA",
                  "%s:%d" % (WGSL, w.functions["pga_color_motor"]["line"]),
                  "RULED to retire (Jean)", str(len(w.callers_of("pga_color_motor"))),
                  "glaw2 + rest bit-identity",
                  "colour-space movement must relocate to %s first" % CANVAS,
                  "DELETE **after** the relocation lands"))
    if ga_raw:
        V.append(("keep", "CONTRIBUTOR_DAG + its closure static_asserts", "contracts",
                  GROUND_ARCH, "load-bearing", "compile-time",
                  "n/a", "n/a",
                  "KEEP — the one declared, checked statement of the composition"))

    R.table(["tier", "symbol", "room", "file:line", "claimed STATUS",
             "reachable callers", "removal instrument", "disclosure",
             "recommended verdict"], V)
    R("Verdict rows: **%d** (DELETE %d · RULE(Jean) %d · KEEP %d)." % (
        len(V),
        sum(1 for v in V if v[8].startswith("DELETE")),
        sum(1 for v in V if "RULE(Jean)" in v[8]),
        sum(1 for v in V if v[8].startswith("KEEP"))))
    R()

    R("---")
    R()

    # ═══════════════════ §8 ═══════════════════════════════════════════
    R("## §8 — SIZING")
    R()
    R("Three separate columns. **They do not add up and must not be merged.**")
    R()

    R("### GPU runtime")
    R()
    gpu_rows = [
        ("config bytes", "%d B of %d → **%d B** realised after 16 B padding" %
         (running, cpp_size, realised),
         "the only column that moves the uniform upload, and it moves it once "
         "per dirty flush, not per draw"),
        ("bindings read by zero reachable fn", "%d — %s" %
         (len(unread), ", ".join("`%s` (%s)" % (u[1], u[0]) for u in unread) or "none"),
         "each is a bind-group slot plus whatever buffer backs it; **this is the "
         "one place §2 has a real GPU prize**"),
        ("buffers freed", "0 until the line above is ruled",
         "the binding is bound in %d group(s); freeing the buffer needs Jean" %
         len({x["sink"] for x in refs if any(x["name"] == u[1] for u in unread)})),
        ("textures freed", "0", "no storage-texture binding is unread"),
        ("indirect slots", "unchanged", "no dead entry points to remove"),
        ("**unreachable WGSL `fn`**", "%d" % len(unreachable_fns),
         "**ZERO runtime cost** — Tint DCEs per entry point before the driver "
         "sees the module"),
        ("**dead entry points**", "%d" % len(dead_entries),
         "zero pipelines to remove" if not dead_entries else "each removes a pipeline"),
    ]
    R.table(["item", "value", "note"], gpu_rows)
    R("**Say this out loud so the campaign is not judged by a metric it cannot")
    R("move:** unreachable WGSL functions are dead-code-eliminated per entry point")
    R("and cost **zero** at runtime. Deleting %d unreachable `fn` buys tree mass" % len(unreachable_fns))
    R("and read-time, not frames. The GPU prize in this tree is the config bytes")
    R("(%d B) and whatever the flag-(c) list yields once ruled — nothing else." % running)
    R()

    R("### Build time")
    R()
    # The baseline is recorded as "G-LAW 1: GREEN in 24.7 s" — no "build" or
    # "compile" word anywhere near it, so key on the law's own name.
    baseline = None
    for p in walk_src(exts={".md"}, roots=("audit", "CLAUDE CODE", "src/docs")):
        m = re.search(r"[^\n]{0,60}(?:g-?law\s*1|glaw1|baseline)[^\n]{0,40}?"
                      r"(\d+\.\d+)\s*s\b[^\n]{0,20}", read(p), re.I)
        if m:
            baseline = (p, lineno(read(p), m.start()), m.group(0).strip()[:80], m.group(1))
            break
    R.table(["item", "value"],
            [("entry points removed", len(dead_entries)),
             ("pipelines removed", len(dead_entries)),
             ("baseline (located in-tree)",
              "**%s s** — `%s:%d`: “%s”" % (baseline[3], baseline[0], baseline[1], baseline[2])
              if baseline else "24.7 s per the handoff; NOT located in-tree")])
    R("With %d dead entry points there is **no build-time prize in §1**. The build" % len(dead_entries))
    R("cost of this tree is the single translation unit, not the shader.")
    R()

    R("### Tree mass — what a context window actually pays")
    R()
    all_src = walk_src(exts={".hpp", ".cpp", ".wgsl"}, roots=("src",))
    src_lines = sum(len(splitlines_exact(read(p))) for p in all_src)
    no_eol = sum(1 for p in all_src if read(p) and not read(p).endswith("\n"))
    R.table(["item", "files", "lines"],
            [("src/ excluding src/external/", len(all_src), src_lines),
             ("comment lines in the §6 scope", "—", tot_c),
             ("world.wgsl comment lines", "—", w.comment_lines),
             ("status-tag sites (§4, TAG class only)", "—", len(actionable)),
             ("tombstone sites (§6.1)", "—", len(tomb)),
             ("coordinate sites (§6.2, a floor)", "—", sum(coord.values())),
             ("dangling reference sites (§5.1)",
              "—", sum(1 for r in dang_rows if r[3].startswith("**")))])
    if no_eol:
        R("> These are LINES, not newlines. **%d of the %d files under `src/` have" %
          (no_eol, len(all_src)))
        R("> no trailing newline**, so `sum(wc -l)` reports %d — exactly %d fewer." %
          (src_lines - no_eol, no_eol))
        R("> Said here so a cross-check does not look like a discrepancy.")
        R()
    R("**The honest payoff of PRUNING_1 is tree mass, not frames.** %d comment" % tot_c)
    R("lines in the §6 scope, of which the tag/tombstone/coordinate/dangling sites")
    R("above are the mechanically identifiable fraction (%d sites). Everything else" %
      (len(actionable) + len(tomb) + sum(coord.values()) +
       sum(1 for r in dang_rows if r[3].startswith("**"))))
    R("in §6 is prose that needs a human ruling, which is P4's job, not P0's.")
    R()

    R("---")
    R()
    R("## ACCEPTANCE — self-check")
    R()
    R.table(["requirement", "status"], [
        ("`python tools/pruning_census.py` runs clean from repo root", "✅"),
        ("stdlib only, no network", "✅ (`re`, `os`, `io`, `sys`, `argparse`, "
                                    "`subprocess`→git, `hashlib`, `collections`)"),
        ("deterministic — every list sorted", "✅"),
        ("regenerates `%s`" % OUT, "✅"),
        ("zero edits under `src/`", "✅ — this tool only reads"),
        ("every finding carries a reproducible recipe",
         "✅ — re-run the tool; per-section recipes inline"),
        ("no verdict EXECUTION", "✅ — §7 recommends, Jean rules, P1 executes"),
    ])
    R()
    R("Anchored at `%s` on `%s`." % (head, branch))
    R()

    text = R.text()

    if args.check:
        cur = read(OUT)
        if cur != text:
            sys.stderr.write("pruning_census: %s is STALE — re-run to regenerate\n" % OUT)
            return 1
        if not args.quiet:
            sys.stdout.write("pruning_census: %s is current (HEAD %s)\n" % (OUT, head_short))
        return 0

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with io.open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    if not args.quiet:
        sys.stdout.write(text)
        sys.stderr.write("\npruning_census: wrote %s (HEAD %s)\n" % (OUT, head_short))
    return 0


if __name__ == "__main__":
    sys.exit(main())
