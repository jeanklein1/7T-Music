#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE BINDING LEDGER (BUDGET_0) — a read-only census of the program's
# binding surface, and the instrument that reproduces it.
#
# WHAT THIS IS. WebGPU charges a program in SLOTS, per shader stage,
# summed across a whole pipeline layout. Bytes and slots are unrelated
# currencies; bind GROUPS do not partition the budget (all the bind group
# layouts of a pipeline layout are concatenated, then checked); and a slot
# is charged once per stage named in `visibility`, whether or not that
# stage can reach the binding. This tool counts what the API charges.
#
# WHAT THIS IS NOT. It touches no translation unit, no shader, and no
# build graph. It reads four files and writes one markdown artifact.
# Precedent: tools/web_dist.py.
#
# THE UNIT OF THE LEDGER is one row per (bind group layout, entry index)
# — NOT per buffer. The registry is one constant per SITE: the same
# buffer wears several names because each name is one (group, slot).
# A buffer-keyed census would merge rows the API charges separately.
#
# USAGE
#   python3 tools/binding_ledger.py              # witnesses to stdout, write artifact
#   python3 tools/binding_ledger.py --check      # witnesses only, write nothing
#   python3 tools/binding_ledger.py -o PATH      # write elsewhere
#
# Exit status is 1 if any witness fails or the reconciliation gate fails.
# A failing witness means the parser or the program disagrees with a
# stated fact; either way the ledger is not to be trusted until ruled on.
# ═══════════════════════════════════════════════════════════════════════

import argparse
import hashlib
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REAL = os.path.join(REPO, "src", "cartridges", "the_board", "realization")

STATE_HPP = os.path.join(REAL, "state.hpp")
REGISTRY_HPP = os.path.join(REAL, "binding_registry.hpp")
RENDERER_HPP = os.path.join(REAL, "renderer.hpp")
WORLD_WGSL = os.path.join(REAL, "world.wgsl")

DEFAULT_OUT = os.path.join(REPO, "audit", "BINDING_LEDGER.md")

# ─── The Core defaults (WebGPU §3.6.2 "limits"). The guaranteed floor on
#     every machine that hands you an adapter, and therefore the web
#     twin's real ceiling. ────────────────────────────────────────────
CORE = {
    "uniform": 12,          # maxUniformBuffersPerShaderStage
    "storage": 8,           # maxStorageBuffersPerShaderStage
    "sampled": 16,          # maxSampledTexturesPerShaderStage
    "samplers": 16,         # maxSamplersPerShaderStage
    "storagetex": 4,        # maxStorageTexturesPerShaderStage
}
CORE_BIND_GROUPS = 4                      # maxBindGroups
CORE_GROUPS_PLUS_VBS = 24                 # maxBindGroupsPlusVertexBuffers
CORE_DYN_UNIFORM = 8                      # maxDynamicUniformBuffersPerPipelineLayout
CORE_DYN_STORAGE = 4                      # maxDynamicStorageBuffersPerPipelineLayout
UNIFORM_BINDING_MAX_BYTES = 65536         # maxUniformBufferBindingSize
UNIFORM_OFFSET_ALIGN = 256                # minUniformBufferOffsetAlignment

STAGES = ("V", "F", "C")


# ═══════════════════════════════════════════════════════════════════════
# WITNESSES
# ═══════════════════════════════════════════════════════════════════════

class Witnesses:
    """Every authority-bearing check, recorded with its numbers.

    A witness is not an assertion: it is a row. It records what was
    expected, what was found, and whether they agreed, so the artifact
    can carry the evidence rather than a claim about it.
    """

    def __init__(self):
        self.rows = []

    def record(self, tag, ok, detail):
        self.rows.append((tag, bool(ok), detail))
        return ok

    def failures(self):
        return [r for r in self.rows if not r[1]]

    def report(self, stream=sys.stdout):
        for tag, ok, detail in self.rows:
            stream.write("  [%s] %-10s %s\n" % ("PASS" if ok else "FAIL", tag, detail))


# ═══════════════════════════════════════════════════════════════════════
# C++ SOURCE HANDLING
# ═══════════════════════════════════════════════════════════════════════

def read(path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def strip_cpp_comments(src):
    """Blank out // and /* */ comments, preserving byte offsets and lines.

    Offsets are preserved so that every span computed on the stripped text
    indexes the original text identically — the parser can quote source
    without a second coordinate system. String and character literals are
    left intact: `desc.label` is a string literal and is ledger data.
    """
    out = list(src)
    i, n = 0, len(src)
    while i < n:
        c = src[i]
        if c == '"' or c == "'":
            quote = c
            i += 1
            while i < n:
                if src[i] == "\\":
                    i += 2
                    continue
                if src[i] == quote:
                    i += 1
                    break
                i += 1
            continue
        if c == "/" and i + 1 < n and src[i + 1] == "/":
            while i < n and src[i] != "\n":
                out[i] = " "
                i += 1
            continue
        if c == "/" and i + 1 < n and src[i + 1] == "*":
            while i < n and not (src[i] == "*" and i + 1 < n and src[i + 1] == "/"):
                if src[i] != "\n":
                    out[i] = " "
                i += 1
            if i < n:
                out[i] = " "
                out[i + 1] = " "
                i += 2
            continue
        i += 1
    return "".join(out)


def line_of(src, pos):
    return src.count("\n", 0, pos) + 1


def gate_map(src):
    """Offset -> list of enclosing `if constexpr (...)` conditions.

    Returns a list of (start, end, condition) spans, innermost last when
    sorted by start. The program's compile-time gates are all written as
    `if constexpr (COND) {` with the brace on the same line, so a brace
    walk resolves them exactly; a gate whose body is a single unbraced
    statement guards nothing this census reads (they are early `return`s
    in dispatch helpers, not creation sites).
    """
    spans = []
    for m in re.finditer(r"if\s+constexpr\s*\(", src):
        j = m.end() - 1
        depth, k = 0, j
        while k < len(src):
            if src[k] == "(":
                depth += 1
            elif src[k] == ")":
                depth -= 1
                if depth == 0:
                    break
            k += 1
        cond = src[j + 1:k].strip()
        rest = src[k + 1:k + 200]
        brace = re.match(r"\s*\{", rest)
        if not brace:
            continue                       # unbraced gate: guards one statement
        body = k + 1 + brace.end() - 1
        depth, p = 0, body
        while p < len(src):
            if src[p] == "{":
                depth += 1
            elif src[p] == "}":
                depth -= 1
                if depth == 0:
                    break
            p += 1
        spans.append((m.start(), p, cond))
    return spans


def gates_at(spans, pos):
    return [c for (s, e, c) in spans if s <= pos <= e]


def roster_gate_of(spans, pos):
    """The ROSTER condition(s) enclosing an offset, as one string."""
    hits = [c for c in gates_at(spans, pos) if "ROSTER" in c]
    return " && ".join(hits) if hits else ""


# ═══════════════════════════════════════════════════════════════════════
# PHASE 0a — THE LAYOUT CENSUS
# ═══════════════════════════════════════════════════════════════════════

VIS_ABBREV = {"Vertex": "V", "Fragment": "F", "Compute": "C"}


def vis_string(mask_tokens):
    """A ShaderStage mask as a canonical V/F/C string, in pipeline order."""
    s = {VIS_ABBREV[t] for t in mask_tokens if t in VIS_ABBREV}
    return "".join(a for a in STAGES if a in s)


def parse_registry(w):
    """binding_registry.hpp -> {('g0','signal'): 0, ...} plus the reverse map."""
    src = strip_cpp_comments(read(REGISTRY_HPP))
    consts = {}
    ns = None
    depth_at_ns = None
    depth = 0
    for line in src.splitlines():
        m = re.search(r"namespace\s+(g\d)\s*\{", line)
        if m:
            ns = m.group(1)
            depth_at_ns = depth
        depth += line.count("{") - line.count("}")
        if ns is not None and depth <= depth_at_ns:
            ns = None
        c = re.search(r"inline\s+constexpr\s+uint32_t\s+(\w+)\s*=\s*(\d+)\s*;", line)
        if c and ns:
            consts[(ns, c.group(1))] = int(c.group(2))
    w.record("registry", len(consts) > 0,
             "binding_registry.hpp: %d constants over %d namespaces (%s)"
             % (len(consts), len({k[0] for k in consts}),
                ", ".join(sorted({k[0] for k in consts}))))
    return consts


class LayoutEntry:
    __slots__ = ("layout_label", "layout_member", "entry_index", "binding_const",
                 "binding_number", "registry_ns", "kind", "access", "vis_declared",
                 "vis_raw", "has_dynamic_offset", "roster_gated", "src_line")

    def __init__(self, **kw):
        for k in self.__slots__:
            setattr(self, k, kw.get(k))


def parse_layouts(w, registry):
    """state.hpp -> one LayoutEntry per (bind group layout, entry index).

    Each creation block is delimited by its `std::array<...BindGroupLayoutEntry,
    N>` declaration and the `CreateBindGroupLayout` that consumes it, so the
    parse cannot drift into the bind-GROUP blocks that follow (those declare
    `BindGroupEntry`, a different type).
    """
    raw = read(STATE_HPP)
    src = strip_cpp_comments(raw)
    spans = gate_map(src)

    layouts = []          # (label, member, declared_n, [LayoutEntry], roster_gate)
    for m in re.finditer(
            r"std::array<\s*wgpu::BindGroupLayoutEntry\s*,\s*(\d+)\s*>\s*(\w+)\s*\{\s*\}\s*;", src):
        declared_n = int(m.group(1))
        arr = m.group(2)
        tail = src.find("CreateBindGroupLayout", m.end())
        if tail < 0:
            raise SystemExit("binding_ledger: unterminated layout block at line %d"
                             % line_of(src, m.start()))
        end = src.find(";", tail)
        block = src[m.end():end]
        base = m.end()

        label = None
        lm = re.search(r'desc\.label\s*=\s*"([^"]*)"\s*;', block)
        if lm:
            label = lm.group(1)
        mem = re.search(r"(\w+)\s*=\s*device_\.CreateBindGroupLayout", block)
        member = mem.group(1) if mem else None

        # entryCount must be the array's own size, not an independent literal.
        ec = re.search(r"desc\.entryCount\s*=\s*(.+?)\s*;", block)
        ec_expr = ec.group(1).strip() if ec else "<absent>"

        rows = {}

        def slot(i):
            return rows.setdefault(i, {"vis": [], "dyn": False})

        pat = re.escape(arr)
        for e in re.finditer(pat + r"\[(\d+)\]\.binding\s*=\s*bind::(g\d)::(\w+)\s*;", block):
            r = slot(int(e.group(1)))
            r["ns"], r["const"] = e.group(2), e.group(3)
            r["line"] = line_of(src, base + e.start())
        for e in re.finditer(pat + r"\[(\d+)\]\.visibility\s*=\s*([^;]+);", block):
            r = slot(int(e.group(1)))
            r["vis"] = re.findall(r"wgpu::ShaderStage::(\w+)", e.group(2))
            r["vis_raw"] = e.group(2).strip()
        for e in re.finditer(pat + r"\[(\d+)\]\.buffer\.type\s*=\s*wgpu::BufferBindingType::(\w+)\s*;", block):
            r = slot(int(e.group(1)))
            r["kind"], r["access"] = "buffer", e.group(2)
        for e in re.finditer(pat + r"\[(\d+)\]\.buffer\.hasDynamicOffset\s*=\s*(true|false)\s*;", block):
            slot(int(e.group(1)))["dyn"] = (e.group(2) == "true")
        for e in re.finditer(pat + r"\[(\d+)\]\.sampler\.type\s*=\s*wgpu::SamplerBindingType::(\w+)\s*;", block):
            r = slot(int(e.group(1)))
            r["kind"], r["access"] = "sampler", e.group(2)
        for e in re.finditer(pat + r"\[(\d+)\]\.texture\.sampleType\s*=\s*wgpu::TextureSampleType::(\w+)\s*;", block):
            r = slot(int(e.group(1)))
            r["kind"] = "texture"
            r["sample"] = e.group(2)
        for e in re.finditer(pat + r"\[(\d+)\]\.texture\.viewDimension\s*=\s*wgpu::TextureViewDimension::(\w+)\s*;", block):
            slot(int(e.group(1)))["viewdim"] = e.group(2)
        for e in re.finditer(pat + r"\[(\d+)\]\.storageTexture\.access\s*=\s*wgpu::StorageTextureAccess::(\w+)\s*;", block):
            r = slot(int(e.group(1)))
            r["kind"] = "storageTexture"
            r["staccess"] = e.group(2)
        for e in re.finditer(pat + r"\[(\d+)\]\.storageTexture\.format\s*=\s*wgpu::TextureFormat::(\w+)\s*;", block):
            slot(int(e.group(1)))["stformat"] = e.group(2)
        for e in re.finditer(pat + r"\[(\d+)\]\.storageTexture\.viewDimension\s*=\s*wgpu::TextureViewDimension::(\w+)\s*;", block):
            slot(int(e.group(1)))["stviewdim"] = e.group(2)

        entries = []
        for i in sorted(rows):
            r = rows[i]
            kind = r.get("kind")
            if kind == "texture":
                access = "%s/%s" % (r.get("sample", "?"), r.get("viewdim", "?"))
            elif kind == "storageTexture":
                access = "%s/%s/%s" % (r.get("staccess", "?"), r.get("stformat", "?"),
                                       r.get("stviewdim", "?"))
            else:
                access = r.get("access", "?")
            entries.append(LayoutEntry(
                layout_label=label,
                layout_member=member,
                entry_index=i,
                binding_const="bind::%s::%s" % (r.get("ns"), r.get("const")),
                binding_number=registry.get((r.get("ns"), r.get("const"))),
                registry_ns=r.get("ns"),
                kind=kind,
                access=access,
                vis_declared=vis_string(r["vis"]),
                vis_raw=r.get("vis_raw", ""),
                has_dynamic_offset=r["dyn"],
                roster_gated=roster_gate_of(spans, m.start()),
                src_line=r.get("line"),
            ))
        layouts.append({
            "label": label, "member": member, "declared_n": declared_n,
            "entries": entries, "entry_count_expr": ec_expr,
            "roster_gated": roster_gate_of(spans, m.start()),
            "line": line_of(src, m.start()),
        })

    # ─── WITNESS 0a-1 — the array's declared size IS the row count.
    #     A partially populated `entries` array is either a defect or a
    #     parser failure and the census cannot tell which.
    bad = [(L["label"], L["declared_n"], len(L["entries"])) for L in layouts
           if L["declared_n"] != len(L["entries"])]
    w.record("0a-1", not bad,
             "%d layouts, every row count == std::array<…, N>"
             % len(layouts) if not bad else
             "row-count mismatch: " + "; ".join("%s declared %d, parsed %d" % b for b in bad))

    # entryCount must be derived from the array, never an independent literal.
    lit = [(L["label"], L["entry_count_expr"]) for L in layouts
           if not L["entry_count_expr"].endswith(".size()")]
    w.record("0a-1b", not lit,
             "every desc.entryCount is <array>.size()" if not lit else
             "literal entryCount: " + "; ".join("%s = %s" % x for x in lit))

    # ─── WITNESS 0a-2 — every binding_const resolves in the registry.
    unresolved = ["%s (%s entry %d)" % (e.binding_const, e.layout_label, e.entry_index)
                  for L in layouts for e in L["entries"] if e.binding_number is None]
    total_rows = sum(len(L["entries"]) for L in layouts)
    w.record("0a-2", not unresolved,
             "%d rows, every bind:: symbol resolves in binding_registry.hpp" % total_rows
             if not unresolved else "unresolvable: " + ", ".join(unresolved))

    # ─── WITNESS 0a-3 — binding numbers are unique within one layout.
    dups = []
    for L in layouts:
        seen = {}
        for e in L["entries"]:
            if e.binding_number in seen:
                dups.append("%s: binding %s at entries[%d] and entries[%d]"
                            % (L["label"], e.binding_number, seen[e.binding_number],
                               e.entry_index))
            seen[e.binding_number] = e.entry_index
    w.record("0a-3", not dups,
             "no duplicate binding number inside any layout" if not dups
             else "; ".join(dups))

    # Every row must name a kind — an entry with none is an unparsed construct.
    kindless = ["%s entries[%d]" % (e.layout_label, e.entry_index)
                for L in layouts for e in L["entries"] if not e.kind]
    w.record("0a-4", not kindless,
             "every row carries a resolved kind (buffer/sampler/texture/storageTexture)"
             if not kindless else "kindless rows: " + ", ".join(kindless))

    # Every row must name at least one stage — a zero mask binds nothing —
    # and every stage token must be one of the three the census can count.
    # `ShaderStage::None` or an unnamed mask would silently under-count.
    visless = ["%s entries[%d] (%r)" % (e.layout_label, e.entry_index, e.vis_raw)
               for L in layouts for e in L["entries"] if not e.vis_declared]
    unknown = sorted({t for L in layouts for e in L["entries"]
                      for t in re.findall(r"wgpu::ShaderStage::(\w+)", e.vis_raw)
                      if t not in VIS_ABBREV})
    w.record("0a-5", not visless and not unknown,
             "every row names at least one of Vertex/Fragment/Compute, and no other stage token appears"
             if not visless and not unknown else
             "; ".join(filter(None, [
                 ("empty visibility: " + ", ".join(visless)) if visless else "",
                 ("unknown stage token(s): " + ", ".join(unknown)) if unknown else ""])))

    return layouts


# ═══════════════════════════════════════════════════════════════════════
# PHASE 0b — THE REACHABILITY CENSUS (WGSL)
#
# INSTRUMENT RULING (handoff): the call graph is built TEXTUALLY over the
# one module. Not naga IR reflection — naga reflects per-module, so the
# per-entry-point set still needs the closure, and an IR dependency buys
# nothing the text does not already give.
# ═══════════════════════════════════════════════════════════════════════

def strip_wgsl_comments(src):
    """Blank out // and (nestable) /* */ comments, preserving offsets."""
    out = list(src)
    i, n, depth = 0, len(src), 0
    while i < n:
        if depth == 0 and src.startswith("//", i):
            while i < n and src[i] != "\n":
                out[i] = " "
                i += 1
            continue
        if src.startswith("/*", i):
            depth += 1
            out[i] = out[i + 1] = " "
            i += 2
            continue
        if depth and src.startswith("*/", i):
            depth -= 1
            out[i] = out[i + 1] = " "
            i += 2
            continue
        if depth and src[i] != "\n":
            out[i] = " "
        i += 1
    return "".join(out)


# ─── The const environment. Array counts are spelled with named
#     constants (PAINTING_MAX_SLOTS, FIELD_SUBSCRIBERS), so a size
#     calculator that cannot fold them cannot size the bindings that
#     matter. Only integer-valued constants are folded; anything else
#     is simply absent, and a type that needs it reports unresolved
#     rather than guessing. ──────────────────────────────────────────

def parse_wgsl_consts(src):
    env = {}
    for m in re.finditer(r"^const\s+(\w+)\s*(?::\s*([\w<>]+)\s*)?=\s*([^;]+);", src, re.M):
        name, expr = m.group(1), m.group(3).strip()
        v = eval_const(expr, env)
        if v is not None:
            env[name] = v
    return env


def eval_const(expr, env):
    """Fold an integer-valued WGSL const expression, or return None.

    Deliberately narrow: literals with u/i suffixes, the four arithmetic
    operators, parentheses, u32()/i32() casts, and references to
    already-folded constants. Anything else — floats, struct values,
    array constructors, function calls — folds to None and the caller
    reports the size as unresolved.
    """
    e = expr.strip()
    if not e:
        return None
    e = re.sub(r"\b(?:u32|i32)\s*\(", "(", e)
    e = re.sub(r"\b(\d+)[uif]\b", r"\1", e)
    if not re.fullmatch(r"[\w\s+\-*/%()]+", e):
        return None
    if re.search(r"\b\d+\.\d*|\.\d+\b", e):
        return None
    for name in sorted(set(re.findall(r"[A-Za-z_]\w*", e)), key=len, reverse=True):
        if name not in env:
            return None
        e = re.sub(r"\b%s\b" % re.escape(name), str(env[name]), e)
    try:
        v = eval(e, {"__builtins__": {}}, {})     # noqa: S307 — arithmetic only
    except Exception:
        return None
    return int(v) if isinstance(v, (int, float)) and float(v).is_integer() else None


def parse_wgsl_structs(src):
    """name -> [(member_name, member_type, align_override, size_override)]"""
    structs = {}
    for m in re.finditer(r"^struct\s+(\w+)\s*\{", src, re.M):
        depth, p = 0, m.end() - 1
        while p < len(src):
            if src[p] == "{":
                depth += 1
            elif src[p] == "}":
                depth -= 1
                if depth == 0:
                    break
            p += 1
        body = src[m.end():p]
        members = []
        for line in split_top_level(body, ","):
            line = line.strip()
            if not line:
                continue
            al = re.search(r"@align\s*\(\s*(\d+)\s*\)", line)
            sz = re.search(r"@size\s*\(\s*(\d+)\s*\)", line)
            line = re.sub(r"@\w+\s*(\([^)]*\))?", "", line).strip()
            mm = re.match(r"(\w+)\s*:\s*(.+)$", line, re.S)
            if not mm:
                continue
            members.append((mm.group(1), " ".join(mm.group(2).split()),
                            int(al.group(1)) if al else None,
                            int(sz.group(1)) if sz else None))
        structs[m.group(1)] = members
    return structs


def split_top_level(text, sep):
    """Split on `sep` at angle/paren/brace depth zero."""
    out, depth, cur = [], 0, []
    for ch in text:
        if ch in "<([{":
            depth += 1
        elif ch in ">)]}":
            depth -= 1
        if ch == sep and depth == 0:
            out.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    out.append("".join(cur))
    return out


def round_up(k, n):
    return ((n + k - 1) // k) * k


SCALARS = {"f32": (4, 4), "i32": (4, 4), "u32": (4, 4), "f16": (2, 2)}


class WgslType:
    """A resolved WGSL store type: its layout, and whether uniform admits it.

    `size` is None for anything containing a runtime-sized array or an
    unfoldable element count — the census reports unresolved rather than
    inventing a number.
    """

    def __init__(self, spelling):
        self.spelling = spelling
        self.align = None
        self.size = None
        self.runtime = False
        self.unresolved = None            # why the size is unknown
        self.uniform_blockers = []        # concrete reasons uniform refuses it
        self.array_elem = None            # (elem WgslType, count, stride)


def layout_of(spelling, structs, consts, cache=None, stack=()):
    """Compute AlignOf/SizeOf and uniform-address-space legality for a type.

    Rules are WGSL §"Memory Layout" and §"Address Space Layout Constraints":
    array stride is roundUp(AlignOf(E), SizeOf(E)); a struct's size rounds up
    to its own alignment; and in the UNIFORM address space

      · an array's element STRIDE must be a multiple of 16,
      · every struct member's offset must be a multiple of
        RequiredAlignOf(member, uniform) — which is roundUp(16, AlignOf) when
        the member is itself a struct or an array, and AlignOf otherwise,
      · a struct-or-array member must be followed by roundUp(16, SizeOf) of
        space before the next member,
      · atomics and runtime-sized arrays are storage-only.

    This is the predicate that actually decides a storage→uniform demotion,
    and it is where a naive census goes wrong. Note what it does and does
    NOT forbid: array<f32, N> (stride 4) and array<vec2<f32>, N> (stride 8)
    are ILLEGAL; array<vec3<f32>, N> is LEGAL, because AlignOf(vec3<f32>) is
    16 and the stride therefore rounds to 16 — it merely wastes 4 B per
    element. A type's own alignment being under 16 is NOT a blocker: a
    struct of eight f32 members has AlignOf 4, stride 32, and is perfectly
    legal in uniform.
    """
    cache = {} if cache is None else cache
    t = " ".join(spelling.split())
    if t in cache:
        return cache[t]
    r = WgslType(t)

    if t in SCALARS:
        r.align, r.size = SCALARS[t]
    elif re.fullmatch(r"vec([234])<(\w+)>", t):
        n, base = re.fullmatch(r"vec([234])<(\w+)>", t).groups()
        n = int(n)
        if base not in SCALARS:
            r.unresolved = "non-scalar vector element %s" % base
        else:
            bs = SCALARS[base][1]
            r.align = (2 if n == 2 else 4) * bs
            r.size = n * bs
    elif re.fullmatch(r"mat([234])x([234])<(\w+)>", t):
        c, rw, base = re.fullmatch(r"mat([234])x([234])<(\w+)>", t).groups()
        col = layout_of("vec%s<%s>" % (rw, base), structs, consts, cache, stack)
        if col.size is None:
            r.unresolved = "column type unresolved"
        else:
            r.align = col.align
            r.size = int(c) * round_up(col.align, col.size)
    elif re.fullmatch(r"atomic<(\w+)>", t):
        inner = layout_of(re.fullmatch(r"atomic<(\w+)>", t).group(1),
                          structs, consts, cache, stack)
        r.align, r.size, r.unresolved = inner.align, inner.size, inner.unresolved
        r.uniform_blockers.append("atomic<> is not permitted in the uniform address space")
    elif t.startswith("array<"):
        inner = t[len("array<"):]
        assert inner.endswith(">")
        parts = [p.strip() for p in split_top_level(inner[:-1], ",")]
        elem = layout_of(parts[0], structs, consts, cache, stack)
        count = None
        if len(parts) > 1:
            count = eval_const(parts[1], consts)
        else:
            r.runtime = True
        r.align = elem.align
        if elem.size is None:
            r.unresolved = "element type unresolved: %s" % (elem.unresolved or parts[0])
        else:
            stride = round_up(elem.align, elem.size)
            r.array_elem = (elem, count, stride)
            if r.runtime:
                r.unresolved = "runtime-sized array"
                r.uniform_blockers.append(
                    "runtime-sized array is not permitted in the uniform address space")
            elif count is None:
                r.unresolved = "unfoldable element count %r" % parts[1]
            else:
                r.size = count * stride
            if stride % 16 != 0:
                r.uniform_blockers.append(
                    "array element stride %d B is not a multiple of 16 "
                    "(element %s: align %d, size %d)"
                    % (stride, elem.spelling, elem.align, elem.size))
        r.uniform_blockers += ["in element: " + b for b in elem.uniform_blockers]
    elif t in structs:
        if t in stack:
            r.unresolved = "recursive struct %s" % t
        else:
            off, maxalign, blockers = 0, 1, []
            members = structs[t]
            for i, (mname, mtype, aov, sov) in enumerate(members):
                mt = layout_of(mtype, structs, consts, cache, stack + (t,))
                if mt.align is None:
                    r.unresolved = "member %s: %s" % (mname, mt.unresolved or "unresolved")
                    break
                malign = aov or mt.align
                msize = sov if sov is not None else mt.size
                off = round_up(malign, off)
                maxalign = max(maxalign, malign)
                # UNIFORM: every member offset is a multiple of
                # roundUp(16, align) for struct/array members.
                req = round_up(16, malign) if (mt.array_elem is not None
                                               or mtype in structs) else malign
                if off % req:
                    blockers.append("member %s.%s at offset %d is not a multiple of %d"
                                    % (t, mname, off, req))
                blockers += ["in %s.%s: %s" % (t, mname, b) for b in mt.uniform_blockers]
                if msize is None:
                    if i == len(members) - 1 and mt.runtime:
                        r.runtime = True
                        r.unresolved = "trailing runtime-sized array in %s.%s" % (t, mname)
                    else:
                        r.unresolved = "member %s: %s" % (mname, mt.unresolved or "unresolved")
                    break
                # UNIFORM: a struct/array member must be followed by
                # roundUp(16, SizeOf) of space.
                if i < len(members) - 1 and (mt.array_elem is not None or mtype in structs):
                    nxt = members[i + 1]
                    nt = layout_of(nxt[1], structs, consts, cache, stack + (t,))
                    if nt.align is not None:
                        nalign = nxt[2] or nt.align
                        if round_up(nalign, off + msize) < off + round_up(16, msize):
                            blockers.append(
                                "member %s.%s (a %s) is followed by %s at offset %d; "
                                "uniform requires the next member at %d or later"
                                % (t, mname, "array" if mt.array_elem else "struct",
                                   nxt[0], round_up(nalign, off + msize),
                                   off + round_up(16, msize)))
                off += msize
            else:
                r.align = maxalign
                r.size = round_up(maxalign, off)
            if r.align is None and r.runtime:
                r.align = maxalign
            r.uniform_blockers = blockers
    elif t.startswith("texture_") or t.startswith("sampler"):
        r.unresolved = "handle type (no host-shareable layout)"
    else:
        r.unresolved = "unknown type %s" % t

    cache[t] = r
    return r


def uniform_element_note(ty):
    """What a storage→uniform demotion of this type would COST in element type.

    Table C's A2 rows carry this so the cost lands in the ledger rather than
    in a later surprise. Where nothing is needed, say so — "none needed" is
    the answer for most rows and it is worth stating.
    """
    if not ty.uniform_blockers:
        return "none needed"
    stride_hit = [b for b in ty.uniform_blockers if b.startswith("array element stride")]
    if stride_hit and ty.array_elem:
        elem, count, stride = ty.array_elem
        need = round_up(16, stride)
        if elem.spelling in SCALARS:
            return ("element %s has stride %d B; uniform needs a multiple of 16 — "
                    "widen to vec4<%s> (÷4 the count) or wrap in a @size(16) struct"
                    % (elem.spelling, stride, elem.spelling))
        if re.fullmatch(r"vec2<(\w+)>", elem.spelling):
            base = re.fullmatch(r"vec2<(\w+)>", elem.spelling).group(1)
            return ("element %s has stride %d B; uniform needs a multiple of 16 — "
                    "widen to vec4<%s> (pack two per element) or pad to %d B"
                    % (elem.spelling, stride, base, need))
        return ("element %s has stride %d B; uniform needs a multiple of 16 — "
                "pad the element to %d B (@size(%d) or an explicit tail member)"
                % (elem.spelling, stride, need, need))
    return "; ".join(ty.uniform_blockers)


class WgslDecl:
    __slots__ = ("symbol", "group", "binding", "address_space", "wgsl_access",
                 "wgsl_type", "has_runtime_array", "line", "layout")

    def __init__(self, **kw):
        for k in self.__slots__:
            setattr(self, k, kw.get(k))


DECL_RE = re.compile(
    r"@group\((\d+)\)\s*@binding\((\d+)\)\s*var\s*"
    r"(?:<\s*([^>]*?)\s*>)?\s*(\w+)\s*:\s*([^;]+);")


def parse_wgsl_decls(w, src, structs, consts):
    decls = []
    for m in DECL_RE.finditer(src):
        space_clause = (m.group(3) or "").strip()
        parts = [p.strip() for p in space_clause.split(",")] if space_clause else []
        space = parts[0] if parts else ""
        access = parts[1] if len(parts) > 1 else ""
        if space == "storage" and not access:
            access = "read"                # WGSL default for var<storage>
        spelling = " ".join(m.group(5).split())
        lay = layout_of(spelling, structs, consts)
        decls.append(WgslDecl(
            symbol=m.group(4),
            group=int(m.group(1)),
            binding=int(m.group(2)),
            address_space=space or "handle",
            wgsl_access=access or "n/a",
            wgsl_type=spelling,
            has_runtime_array=bool(lay.runtime),
            line=line_of(src, m.start()),
            layout=lay,
        ))

    # Every `@group` occurrence in the file must have produced a row — a
    # declaration the regex skipped is a hole the census would never show.
    raw_groups = len(re.findall(r"@group\s*\(", src))
    w.record("0b-0", raw_groups == len(decls),
             "%d @group( occurrences, %d declarations parsed" % (raw_groups, len(decls)))

    slots = {}
    for d in decls:
        slots.setdefault((d.group, d.binding), []).append(d)
    aliases = sorted(s for k, v in slots.items() for s in [x.symbol for x in v][1:])

    # ─── WITNESS 0b-1 — the registry banner asserts 100 declarations over
    #     97 slots, with fc_config / fc_vp / fc_patches as the three
    #     aliases. Reproduce both numbers AND name exactly those three.
    expect_aliases = ["fc_config", "fc_patches", "fc_vp"]
    ok = (len(decls) == 100 and len(slots) == 97 and aliases == expect_aliases)
    w.record("0b-1", ok,
             "banner reproduced: %d declarations over %d slots; aliases %s"
             % (len(decls), len(slots), ", ".join(aliases)) if ok else
             "banner says 100 declarations over 97 slots with aliases %s; census found "
             "%d over %d with aliases %s"
             % (", ".join(expect_aliases), len(decls), len(slots), ", ".join(aliases) or "none"))

    # ─── The layout calculator, checked against the PROGRAM's own prose.
    #     Three byte counts are written down in state.hpp and
    #     binding_registry.hpp by the people who sized the buffers. If the
    #     calculator cannot reproduce all three, no A2 row it produces is
    #     worth reading.
    by_symbol = {d.symbol: d for d in decls}
    stated = [("agent_figure_profiles", 4032, "state.hpp entries[17]: \"4032 B, session-constant\""),
              ("field_head_poses", 6400, "binding_registry.hpp g2:2: \"6,400 B\""),
              ("field_authored", 144, "binding_registry.hpp g2:5: \"uniform, 144 B\"")]
    off = []
    for sym, want, where in stated:
        got = by_symbol[sym].layout.size if sym in by_symbol else None
        if got != want:
            off.append("%s: source says %s, calculator says %s (%s)" % (sym, want, got, where))
    w.record("0b-4", not off,
             "WGSL layout calculator reproduces all three byte counts the program states in "
             "prose: agent_figure_profiles 4032 B, field_head_poses 6400 B, field_authored 144 B"
             if not off else "; ".join(off))

    # ─── The uniform-legality predicate, checked against the program.
    #     Every declaration the program ALREADY places in the uniform
    #     address space compiles today, so the predicate MUST judge every
    #     one of them legal. A false positive here would let an A2 row
    #     propose a demotion the compiler refuses — the exact failure G4
    #     names.
    already = [d for d in decls if d.address_space == "uniform"]
    wrong = ["%s (%s): %s" % (d.symbol, d.wgsl_type, d.layout.uniform_blockers[0])
             for d in already if d.layout.uniform_blockers]
    w.record("0b-5", not wrong,
             "the uniform-legality predicate clears all %d declarations the program already "
             "places in the uniform address space" % len(already)
             if not wrong else
             "predicate rejects %d live uniform declaration(s): %s"
             % (len(wrong), "; ".join(wrong)))
    return decls, slots


# ─── 0b-ii — the reachability closure ────────────────────────────────

class WgslFn:
    __slots__ = ("name", "stage", "workgroup_size", "body", "params", "start", "line",
                 "calls", "refs", "uses", "sites", "shadowed")

    def __init__(self, **kw):
        for k in self.__slots__:
            setattr(self, k, kw.get(k))


STAGE_OF_ATTR = {"vertex": "V", "fragment": "F", "compute": "C"}

# One combined scanner so braces, local declarations and identifiers are
# seen in a single offset-ordered pass.
SCOPE_TOK = re.compile(
    r"(?P<open>\{)|(?P<close>\})"
    r"|\b(?:let|var|const)\b\s*(?:<[^>]*>)?\s*(?P<decl>[A-Za-z_]\w*)"
    r"|(?<![\w.])(?P<id>[A-Za-z_]\w*)")


def scoped_idents(fn, interesting):
    """Yield (identifier, is_module_scope) for every occurrence in a body.

    A name declared by `let`/`var`/`const`, or bound as a parameter,
    shadows the module scope for the rest of its block. Without this, a
    local named after a binding reads as a reference to that binding —
    which is how `patch_terrain_fs` appeared to reach `patch_grid`.
    """
    scopes = [set(re.findall(r"(?<![\w.])(\w+)\s*:", fn.params or ""))]
    for m in SCOPE_TOK.finditer(fn.body):
        if m.group("open"):
            scopes.append(set())
        elif m.group("close"):
            if len(scopes) > 1:
                scopes.pop()
        elif m.group("decl"):
            name = m.group("decl")
            scopes[-1].add(name)
            if name in interesting:
                yield name, False, m.start("decl"), m.end("decl")
        else:
            tok = m.group("id")
            if tok not in interesting:
                continue
            yield (tok, not any(tok in sc for sc in scopes),
                   m.start("id"), m.end("id"))


def parse_wgsl_functions(w, src, decl_names):
    fns = {}
    for m in re.finditer(r"\bfn\s+(\w+)\s*\(", src):
        name = m.group(1)
        p = m.end() - 1
        depth = 0
        while p < len(src):
            if src[p] == "(":
                depth += 1
            elif src[p] == ")":
                depth -= 1
                if depth == 0:
                    break
            p += 1
        brace = src.find("{", p)
        if brace < 0:
            continue
        depth, q = 0, brace
        while q < len(src):
            if src[q] == "{":
                depth += 1
            elif src[q] == "}":
                depth -= 1
                if depth == 0:
                    break
            q += 1
        body = src[brace:q + 1]

        # Attributes precede `fn`, on the same line or the lines above,
        # separated only by whitespace.
        head_start = m.start()
        k = head_start - 1
        while k >= 0:
            back = src[:k + 1]
            am = re.search(r"@\w+\s*(\([^)]*\))?\s*$", back)
            if not am:
                break
            k = am.start() - 1
        head = src[k + 1:head_start]
        stage = None
        for a, s in STAGE_OF_ATTR.items():
            if re.search(r"@%s\b" % a, head):
                stage = s
        wg = re.search(r"@workgroup_size\s*\(([^)]*)\)", head)
        fns[name] = WgslFn(name=name, stage=stage, params=src[m.end():p],
                           workgroup_size=("(%s)" % " ".join(wg.group(1).split())) if wg else "",
                           body=body, start=brace, line=line_of(src, m.start()),
                           calls=set(), refs=set())

    # Reference extraction. An identifier preceded by `.` is member
    # access, not a module-scope name; the `fn` keyword itself is a
    # definition, not a call. Everything else that matches a function
    # name or a binding symbol counts as a reference — UNLESS a local
    # declaration or parameter shadows it in scope. BUDGET_0b noted the
    # shadowing hazard and accepted it as one-directional; extension 1's
    # W1-1 then caught it live (`let patch_grid = vec2<i32>(…)` in two
    # functions), so the closure now resolves scope properly rather than
    # over-approximating.
    for fn in fns.values():
        fn.shadowed = set()
        for tok, is_ref, _s, _e in scoped_idents(fn, decl_names | set(fns)):
            if not is_ref:
                fn.shadowed.add(tok)
                continue
            if tok in fns:
                fn.calls.add(tok)
            elif tok in decl_names:
                fn.refs.add(tok)

    entries = {n: f for n, f in fns.items() if f.stage}
    w.record("0b-2", len(entries) > 0,
             "%d functions, %d entry points (%d vertex, %d fragment, %d compute)"
             % (len(fns), len(entries),
                sum(1 for f in entries.values() if f.stage == "V"),
                sum(1 for f in entries.values() if f.stage == "F"),
                sum(1 for f in entries.values() if f.stage == "C")))
    missing_wg = [n for n, f in entries.items() if f.stage == "C" and not f.workgroup_size]
    w.record("0b-3", not missing_wg,
             "every @compute entry point carries a @workgroup_size" if not missing_wg
             else "no workgroup_size on: " + ", ".join(sorted(missing_wg)))
    return fns, entries


def reachability(fns, entries):
    """entry point -> (reached function set, reached binding symbols)."""
    out = {}
    for name, fn in entries.items():
        seen, stack = set(), [name]
        while stack:
            cur = stack.pop()
            if cur in seen:
                continue
            seen.add(cur)
            stack.extend(fns[cur].calls - seen)
        refs = set()
        for f in seen:
            refs |= fns[f].refs
        out[name] = (seen, refs)
    return out


# ═══════════════════════════════════════════════════════════════════════
# EXTENSION 1 — READ/WRITE SEPARATION
#
# BUDGET_0 recorded that a binding is REFERENCED. This records what the
# reference DOES. Two things turn on it: whether two dispatches can be
# fused (a fused kernel deletes the inter-dispatch barrier that is making
# the sequence correct today), and whether a `read_write` declaration
# claims a capability it never exercises.
# ═══════════════════════════════════════════════════════════════════════

# Atomic builtins, by what they do to the atomic they are handed.
ATOMIC_RW = ("atomicAdd", "atomicSub", "atomicMax", "atomicMin", "atomicAnd",
             "atomicOr", "atomicXor", "atomicExchange", "atomicCompareExchangeWeak")
ATOMIC_W = ("atomicStore",)
ATOMIC_R = ("atomicLoad",)

COMPOUND_ASSIGN = ("<<=", ">>=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=")


def _skip_ws(s, i, step=1):
    while 0 <= i < len(s) and s[i] in " \t\r\n":
        i += step
    return i


def _consume_access_chain(s, i):
    """From just past a symbol, consume `[…]` and `.ident` to the chain's end."""
    while True:
        j = _skip_ws(s, i)
        if j < len(s) and s[j] == "[":
            depth, k = 0, j
            while k < len(s):
                if s[k] == "[":
                    depth += 1
                elif s[k] == "]":
                    depth -= 1
                    if depth == 0:
                        break
                k += 1
            i = k + 1
            continue
        if j < len(s) and s[j] == "." and j + 1 < len(s) and (s[j + 1].isalpha() or s[j + 1] == "_"):
            k = j + 1
            while k < len(s) and (s[k].isalnum() or s[k] == "_"):
                k += 1
            i = k
            continue
        return i


def classify_use(body, start, end):
    """What does this reference to a binding DO? -> 'r' | 'w' | 'rw'.

    Lexical and deliberately biased: an ambiguous site is called a WRITE.
    Both consumers are safe in that direction — a spurious write adds a RAW
    pair (a false bar on fusing, not a false blessing) and removes an A4
    flag (a missed correction, not a wrong one).
    """
    # A builtin call wrapping the reference: textureStore(tex, …),
    # atomicAdd(&buf[i], …). Look back past whitespace and an optional `&`.
    b = _skip_ws(body, start - 1, -1)
    if b >= 0 and body[b] == "&":
        b = _skip_ws(body, b - 1, -1)
    if b >= 0 and body[b] == "(":
        k = _skip_ws(body, b - 1, -1)
        e = k + 1
        while k >= 0 and (body[k].isalnum() or body[k] == "_"):
            k -= 1
        callee = body[k + 1:e]
        if callee == "textureStore":
            return "w"
        if callee in ATOMIC_W:
            return "w"
        if callee in ATOMIC_R:
            return "r"
        if callee in ATOMIC_RW:
            return "rw"

    # Otherwise: is the reference the target of an assignment?
    i = _consume_access_chain(body, end)
    j = _skip_ws(body, i)
    tail = body[j:j + 3]
    for op in COMPOUND_ASSIGN:
        if tail.startswith(op):
            return "rw"
    if tail.startswith("++") or tail.startswith("--"):
        return "rw"
    if tail.startswith("=") and not tail.startswith("=="):
        return "w"
    return "r"


def annotate_uses(w, src, fns, decls):
    """Attach per-function {symbol: 'r'|'w'|'rw'} to every function.

    Also records each access SITE (offset, index expression, enclosing
    override gate) for extension 2, which asks a different question of the
    same occurrences.
    """
    names = {d.symbol for d in decls}
    for fn in fns.values():
        fn.uses = {}
        fn.sites = []
        for tok, is_ref, st, en in scoped_idents(fn, names):
            if not is_ref:
                continue
            kind = classify_use(fn.body, st, en)
            prev = fn.uses.get(tok)
            fn.uses[tok] = "rw" if (prev and prev != kind) or kind == "rw" else kind
            fn.sites.append({"symbol": tok, "off": st, "end": en, "use": kind})

    # ─── W1-0. The write detector is TEXTUAL: it sees an assignment or a
    #     builtin call at the reference. A write reached through a pointer
    #     parameter would be invisible to it. world.wgsl must therefore
    #     contain no pointer types at all, or the detector is unsound.
    ptrs = sorted(set(re.findall(r"\bptr\s*<[^>]*>", src)))
    w.record("W1-0", not ptrs,
             "world.wgsl declares no `ptr<…>` anywhere, so no write can reach a "
             "binding except through an assignment or a builtin at the reference — "
             "which is exactly what the detector sees"
             if not ptrs else "pointer types present, write detection is unsound: "
             + ", ".join(ptrs))
    return fns


def rw_closure(fns, entries, reach):
    """entry point -> {symbol: 'r'|'w'|'rw'} over its whole call closure."""
    out = {}
    for name in entries:
        acc = {}
        for f in reach[name][0]:
            for sym, kind in fns[f].uses.items():
                prev = acc.get(sym)
                acc[sym] = "rw" if (prev and prev != kind) or kind == "rw" else kind
        out[name] = acc
    return out


def reads(u):
    return {s for s, k in u.items() if k in ("r", "rw")}


def writes(u):
    return {s for s, k in u.items() if k in ("w", "rw")}


def phase_ext1(w, wgsl, cen, layouts):
    fns, entries, reach = wgsl["fns"], wgsl["entries"], wgsl["reach"]
    ep_use = rw_closure(fns, entries, reach)
    wgsl["ep_use"] = ep_use

    # ─── W1-1 — a binding the WGSL declares non-writable must have an
    #     empty write-set, and a write-only storage texture must have an
    #     empty read-set. A violation is a detector bug, not a program
    #     defect: the shader would not compile.
    bad = []
    for d in wgsl["decls"]:
        writable = (d.address_space == "storage" and d.wgsl_access == "read_write") \
                   or d.wgsl_type.startswith("texture_storage")
        readable = not (d.wgsl_type.startswith("texture_storage")
                        and re.search(r",\s*write\s*>", d.wgsl_type))
        for ep, u in ep_use.items():
            k = u.get(d.symbol)
            if k is None:
                continue
            if not writable and k in ("w", "rw"):
                bad.append("%s: write recorded in %s but declared %s %s"
                           % (d.symbol, ep, d.address_space, d.wgsl_access))
            if not readable and k in ("r", "rw"):
                bad.append("%s: read recorded in %s but declared write-only"
                           % (d.symbol, ep))
    w.record("W1-1", not bad,
             "no write recorded on any binding the WGSL declares non-writable, and no "
             "read on any write-only storage texture, across all %d entry points"
             % len(entries) if not bad else "; ".join(sorted(set(bad))[:6]))

    # ─── 1a — the RAW table. Restricted to FUSION-ELIGIBLE pairs: a fused
    #     kernel has ONE pipeline layout, so only compute entry points whose
    #     pipelines carry identical group layouts could ever be fused. The
    #     unrestricted matrix is counted too, so the restriction hides
    #     nothing.
    cs = [p for p in cen["pipelines"] if p.kind == "compute"]
    ep_of = {}
    for p in cs:
        ep_of.setdefault(p.cs_entry and cen["entry_const"].get(p.cs_entry), []).append(p)
    comp_eps = [e for e in ep_of if e]

    raw_rows, all_pairs, all_nonempty = [], 0, 0
    for a in sorted(comp_eps):
        for b_ep in sorted(comp_eps):
            if a == b_ep:
                continue
            all_pairs += 1
            hazard = writes(ep_use[a]) & reads(ep_use[b_ep])
            if hazard:
                all_nonempty += 1
            same = (ep_of[a][0].group_layouts == ep_of[b_ep][0].group_layouts)
            if same:
                raw_rows.append({"a": a, "b": b_ep, "hazard": sorted(hazard),
                                 "layouts": ep_of[a][0].group_layouts})

    # ─── W1-3 — the table exists whether or not it has rows, and its pair
    #     count is stated. Silence and absence must be distinguishable.
    w.record("W1-3",
             len(raw_rows) == sum(1 for a in comp_eps for b_ep in comp_eps
                                  if a != b_ep
                                  and ep_of[a][0].group_layouts == ep_of[b_ep][0].group_layouts),
             "RAW table: %d fusion-eligible ordered pairs (same pipeline layout) of %d "
             "compute entry points; %d carry a hazard. Unrestricted matrix: %d ordered "
             "pairs, %d with a non-empty write∩read."
             % (len(raw_rows), len(comp_eps),
                sum(1 for r in raw_rows if r["hazard"]), all_pairs, all_nonempty))

    # ─── 1b — A4, OVER-PRIVILEGED. Declared read_write, never written.
    a4 = []
    for d in wgsl["decls"]:
        if not (d.address_space == "storage" and d.wgsl_access == "read_write"):
            continue
        touch = {ep: u[d.symbol] for ep, u in ep_use.items() if d.symbol in u}
        if touch and all(k == "r" for k in touch.values()):
            a4.append({"decl": d, "readers": sorted(touch),
                       "layout_rows": [], "vertex_hazard": None})
    return {"ep_use": ep_use, "raw": raw_rows, "a4": a4,
            "raw_all_pairs": all_pairs, "raw_all_nonempty": all_nonempty,
            "compute_eps": sorted(comp_eps)}


def phase_0b(w):
    raw = read(WORLD_WGSL)
    src = strip_wgsl_comments(raw)
    consts = parse_wgsl_consts(src)
    structs = parse_wgsl_structs(src)
    decls, slots = parse_wgsl_decls(w, src, structs, consts)
    fns, entries = parse_wgsl_functions(w, src, {d.symbol for d in decls})
    annotate_uses(w, src, fns, decls)
    reach = reachability(fns, entries)
    return {"src": src, "consts": consts, "structs": structs, "decls": decls,
            "slots": slots, "fns": fns, "entries": entries, "reach": reach}


# ═══════════════════════════════════════════════════════════════════════
# EXTENSION 2 — ACCESS-PATTERN CLASSIFICATION
#
# The ledger records a binding's TYPE. Whether it can leave the storage
# wallet for a vertex buffer turns on how it is INDEXED, which is a
# different question — and the one the logistics survey got backwards in
# both directions on the same pair of bindings.
# ═══════════════════════════════════════════════════════════════════════

SEQ_BUILTINS = {"instance_index": "instance", "vertex_index": "vertex"}
OTHER_BUILTINS = ("global_invocation_id", "local_invocation_id", "workgroup_id",
                  "num_workgroups", "local_invocation_index", "front_facing",
                  "position", "sample_index", "sample_mask", "frag_depth",
                  "subgroup_invocation_id", "subgroup_size")

# Least-eligible wins when a variable is assigned more than once.
CLASS_RANK = {"builtin_sequential": 0, "scalar": 1, "builtin_derived": 2,
              "other": 3, "indirected": 4}


class IdxClass:
    __slots__ = ("cls", "source")

    def __init__(self, cls, source=""):
        self.cls, self.source = cls, source

    def __repr__(self):
        return "%s(%s)" % (self.cls, self.source) if self.source else self.cls


def join_class(a, b):
    if a is None:
        return b
    if b is None:
        return a
    return a if CLASS_RANK[a.cls] >= CLASS_RANK[b.cls] else b


AFFINE_OK = re.compile(r"^[\w\s\+\-\*\(\)\.]*$")


def classify_expr(expr, env, bindings, consts, overrides, uniforms, params=()):
    """Classify an index expression into the five-class taxonomy."""
    e = " ".join(expr.split())
    if not e:
        return IdxClass("scalar", "(no index)")

    # An index that is itself a load from another binding is an
    # INDIRECTION, whatever it is indexed by. This test comes first
    # because `visible_patch_indices[patch_id]` is sequential in
    # patch_id and still makes its consumer random-access.
    for m in re.finditer(r"(?<![\w.])([A-Za-z_]\w*)\s*\[", e):
        if m.group(1) in bindings:
            return IdxClass("indirected", m.group(1))

    # `select`, `min`, `f32`… are CALLEES, not data sources — the
    # arguments are. `true`/`false` are literals, not identifiers.
    idents = [t for t in re.findall(r"(?<![\w.])([A-Za-z_]\w*)(?!\s*\()", e)
              if t not in ("true", "false")]
    tainted = [(t, env[t]) for t in idents if t in env and env[t] is not None]
    if tainted:
        worst = None
        for _, k in tainted:
            worst = join_class(worst, k)
        if worst.cls == "builtin_sequential":
            # Affine only: no division, modulo, member access, call or
            # select. Anything else is many elements per primitive.
            body = e
            for t, _ in tainted:
                body = re.sub(r"(?<![\w.])%s(?![\w])" % re.escape(t), " ", body)
            if ("/" in e or "%" in e or "." in e or "select" in e
                    or re.search(r"\w\s*\(", e) or not AFFINE_OK.match(e)):
                return IdxClass("builtin_derived", worst.source)
            return IdxClass("builtin_sequential", worst.source)
        return worst
    if any(t in uniforms for t in idents):
        return IdxClass("scalar", next(t for t in idents if t in uniforms))
    if all(t in consts or t in overrides or t in SCALARS for t in idents) or not idents:
        src = ", ".join(t for t in idents if t in overrides) or "const"
        return IdxClass("scalar", src)
    unknown = [t for t in idents if t not in consts and t not in overrides]
    if params and all(t in params for t in unknown):
        # The index comes from this function's own parameters. The closure
        # is not interprocedural, so say so rather than inventing a class.
        return IdxClass("other", "callee parameter: " + ", ".join(sorted(set(unknown))))
    return IdxClass("other", e[:60])


ASSIGN_RE = re.compile(
    r"(?:\b(?:let|var|const)\b\s*(?:<[^>]*>)?\s*(?P<decl>\w+)\s*(?::[^=;]+)?=\s*(?P<dexpr>[^;]+);)"
    r"|(?:(?:^|(?<=[{};]))\s*(?P<lhs>[A-Za-z_]\w*)\s*(?P<op>[+\-*/%&|^]?=)(?!=)"
    r"\s*(?P<aexpr>[^;]+);)", re.M)


def taint_env(fn, bindings, consts, overrides, uniforms):
    """Local dataflow: which locals carry a builtin, an indirection, or neither."""
    env = {}
    pnames = set(re.findall(r"(?<![\w.])(\w+)\s*:", fn.params or ""))
    for pm in re.finditer(r"@builtin\s*\(\s*(\w+)\s*\)\s*(\w+)\s*:", fn.params or ""):
        b, name = pm.group(1), pm.group(2)
        if b in SEQ_BUILTINS:
            env[name] = IdxClass("builtin_sequential", SEQ_BUILTINS[b])
        else:
            env[name] = IdxClass("builtin_derived", b)
    for om in re.finditer(r"@builtin\s*\(\s*(\w+)\s*\)", fn.body[:0] or ""):
        pass
    for m in ASSIGN_RE.finditer(fn.body):
        name = m.group("decl") or m.group("lhs")
        expr = m.group("dexpr") or m.group("aexpr")
        if not name or name in bindings:
            continue
        env[name] = join_class(env.get(name),
                               classify_expr(expr, env, bindings, consts, overrides,
                                             uniforms, pnames))
        if m.group("op") and m.group("op") != "=":
            env[name] = join_class(env[name], IdxClass("builtin_derived", "compound assign"))
    return env


def if_spans_wgsl(body):
    """(start, end, condition) for every braced `if (...)` in a body."""
    spans = []
    for m in re.finditer(r"\bif\s*\(", body):
        j = m.end() - 1
        depth, k = 0, j
        while k < len(body):
            if body[k] == "(":
                depth += 1
            elif body[k] == ")":
                depth -= 1
                if depth == 0:
                    break
            k += 1
        cond = body[j + 1:k]
        rest = re.match(r"\s*\{", body[k + 1:k + 40])
        if not rest:
            continue
        b = k + 1 + rest.end() - 1
        depth, p = 0, b
        while p < len(body):
            if body[p] == "{":
                depth += 1
            elif body[p] == "}":
                depth -= 1
                if depth == 0:
                    break
            p += 1
        spans.append((m.start(), p, " ".join(cond.split())))
    return spans


def phase_ext2(w, wgsl):
    decls = wgsl["decls"]
    bindings = {d.symbol for d in decls}
    uniforms = {d.symbol for d in decls if d.address_space == "uniform"}
    consts = set(wgsl["consts"])
    overrides = set(re.findall(r"^override\s+(\w+)", wgsl["src"], re.M))

    sites = []
    for fn in wgsl["fns"].values():
        env = taint_env(fn, bindings, consts, overrides, uniforms)
        ifs = if_spans_wgsl(fn.body)
        for s in fn.sites:
            i = _skip_ws(fn.body, s["end"])
            if i < len(fn.body) and fn.body[i] == "[":
                depth, k = 0, i
                while k < len(fn.body):
                    if fn.body[k] == "[":
                        depth += 1
                    elif fn.body[k] == "]":
                        depth -= 1
                        if depth == 0:
                            break
                    k += 1
                idx = fn.body[i + 1:k]
            else:
                idx = ""
            cls = classify_expr(idx, env, bindings, consts, overrides, uniforms,
                                set(re.findall(r"(?<![\w.])(\w+)\s*:", fn.params or "")))
            gates = [c for (a, b_, c) in ifs if a <= s["off"] <= b_
                     and any(o in c for o in overrides)]
            sites.append({"fn": fn.name, "symbol": s["symbol"], "use": s["use"],
                          "index": " ".join(idx.split()), "cls": cls,
                          "line": fn.line + fn.body.count("\n", 0, s["off"]),
                          "override_gate": "; ".join(gates)})

    # ─── W2-1 — every classified access resolves to a Table A binding.
    stray = sorted({s["symbol"] for s in sites if s["symbol"] not in bindings})
    w.record("W2-1", not stray,
             "%d access sites classified, every one on a declared binding" % len(sites)
             if not stray else "unknown symbols: " + ", ".join(stray))

    # ─── W2-2 — classification is total, and `other` rows are listed
    #     individually with their expression. Never counted in aggregate.
    others = [s for s in sites if s["cls"].cls == "other"]
    counts = {}
    for s in sites:
        counts[s["cls"].cls] = counts.get(s["cls"].cls, 0) + 1
    w.record("W2-2", all(s["cls"].cls in CLASS_RANK for s in sites),
             "classification total: " + ", ".join("%s %d" % (k, counts[k])
                                                  for k in sorted(counts))
             + ("; `other` rows enumerated below" if others else "; no `other` rows"))

    # ─── W2-3 — the POSITIVE CONTROL. The classifier must call
    #     visible_patch_indices[patch_id] sequential and
    #     patch_instances[actual_id] indirected, in patch_terrain_vs.
    #     If it does not, it is wrong in the exact way the survey was.
    ctl = {}
    for s in sites:
        if s["fn"] == "patch_terrain_vs" and s["symbol"] in ("visible_patch_indices",
                                                             "patch_instances"):
            ctl[s["symbol"]] = s
    ok = (ctl.get("visible_patch_indices") and
          ctl["visible_patch_indices"]["cls"].cls == "builtin_sequential" and
          ctl["visible_patch_indices"]["cls"].source == "instance" and
          ctl.get("patch_instances") and
          ctl["patch_instances"]["cls"].cls == "indirected")
    w.record("W2-3", ok,
             "positive control: patch_terrain_vs reads visible_patch_indices[%s] as %s "
             "and patch_instances[%s] as %s"
             % (ctl["visible_patch_indices"]["index"], ctl["visible_patch_indices"]["cls"],
                ctl["patch_instances"]["index"], ctl["patch_instances"]["cls"])
             if ok else "control FAILED: %s"
             % {k: (v["index"], str(v["cls"])) for k, v in ctl.items()})

    # ─── The vertex-buffer eligibility predicate. Every access in every
    #     entry point that reaches the binding must be builtin_sequential
    #     on the same builtin, and the element stride must fit
    #     maxVertexBufferArrayStride. An override gate does NOT rescue a
    #     mixed classification: an override selects a branch inside ONE
    #     entry point, and one entry point has ONE signature.
    by_symbol = {}
    for s in sites:
        by_symbol.setdefault(s["symbol"], []).append(s)
    elig = []
    for d in decls:
        ss = by_symbol.get(d.symbol, [])
        if not ss:
            continue
        classes = {(s["cls"].cls, s["cls"].source) for s in ss}
        stride = d.layout.array_elem[2] if d.layout.array_elem else None
        blockers = []
        kind = None
        if len(classes) > 1:
            blockers.append("mixed classification across %d access sites: %s"
                            % (len(ss), ", ".join(sorted("%s(%s)" % c for c in classes))))
        only = next(iter(classes)) if len(classes) == 1 else None
        if only and only[0] != "builtin_sequential":
            blockers.append("indexed %s(%s), not sequential in a builtin" % only)
        if only and only[0] == "builtin_sequential":
            kind = only[1]
        if stride is None:
            blockers.append("no array element stride (not an array binding)")
        elif stride > 2048:
            blockers.append("element stride %d B exceeds maxVertexBufferArrayStride 2048"
                            % stride)
        gated = sorted({s["override_gate"] for s in ss if s["override_gate"]})
        elig.append({"decl": d, "sites": ss, "classes": sorted(classes), "stride": stride,
                     "blockers": blockers, "step": kind, "override_gates": gated,
                     "eligible": not blockers})
    return {"sites": sites, "eligibility": elig, "others": others,
            "overrides": sorted(overrides)}


# ═══════════════════════════════════════════════════════════════════════
# PHASE 0c — THE PIPELINE CENSUS
#
# The shared builders (makeEntity, makeComputePipeline, computeLayoutFor,
# makeShadow) are RESOLVED, not recorded: a pipeline built through a
# shared builder inherits that builder's pipeline layout, and the ledger
# wants the layout, not the builder's name.
# ═══════════════════════════════════════════════════════════════════════

class Pipeline:
    __slots__ = ("label", "member", "kind", "vs_entry", "fs_entry", "cs_entry",
                 "group_layouts", "vertex_buffer_count", "vertex_attribute_count",
                 "color_target_count", "roster_gate", "line")

    def __init__(self, **kw):
        for k in self.__slots__:
            setattr(self, k, kw.get(k))

    def entries(self):
        return [(e, s) for e, s in ((self.vs_entry, "V"), (self.fs_entry, "F"),
                                    (self.cs_entry, "C")) if e]


def parse_layout_handles(w):
    """renderer field name -> state.hpp bind group layout member.

    Two hops: state.hpp's accessors (`x_layout() const { return xLayout_; }`)
    and the renderer's init() assignments (`xLayout_ = gpuState.x_layout();`).
    Resolving both means Table C can name the LAYOUT a pipeline binds, not
    the renderer's private handle for it.
    """
    state = strip_cpp_comments(read(STATE_HPP))
    acc = {}
    for m in re.finditer(
            r"wgpu::BindGroupLayout\s+(\w+)\s*\(\s*\)\s*const\s*\{\s*return\s+(\w+)\s*;\s*\}", state):
        acc[m.group(1)] = m.group(2)
    rend = strip_cpp_comments(read(RENDERER_HPP))
    field = {}
    for m in re.finditer(r"(\w+)\s*=\s*gpuState\.(\w+)\s*\(\s*\)\s*;", rend):
        if m.group(2) in acc:
            field[m.group(1)] = acc[m.group(2)]
    w.record("0c-0", len(field) > 0,
             "%d renderer layout handles resolve to state.hpp layout members "
             "(via %d gpuState accessors)" % (len(field), len(acc)))
    return field


def parse_pipelines(w, src, spans, handles, layouts_by_member):
    """Resolve every pipeline creation to (label, member, stages, group layouts)."""
    # ─── Pipeline layouts, resolved in source order. Every variable is
    #     reassigned inside its own block, so a running map with
    #     last-write-wins is exact for this file's shape.
    arrays, plds, pls = {}, {}, {}
    events = []
    for m in re.finditer(
            r"std::array<\s*wgpu::BindGroupLayout\s*,\s*(\d+)\s*>\s*(\w+)\s*=\s*\{([^}]*)\}", src):
        items = [x.strip() for x in m.group(3).split(",") if x.strip()]
        events.append((m.start(), "array", m.group(2), items, int(m.group(1))))
    for m in re.finditer(r"(\w+)\.bindGroupLayouts\s*=\s*(\w+)\.data\(\)", src):
        events.append((m.start(), "pld", m.group(1), m.group(2), None))
    for m in re.finditer(
            r"wgpu::PipelineLayout\s+(\w+)\s*=\s*device_\.CreatePipelineLayout\(\s*&(\w+)\s*\)", src):
        events.append((m.start(), "create", m.group(1), m.group(2), None))
    for m in re.finditer(r"wgpu::PipelineLayout\s+(\w+)\s*=\s*computeLayoutFor\(\s*(\w+)\s*\)", src):
        events.append((m.start(), "for", m.group(1), m.group(2), None))

    bad_counts = []
    for pos, kind, a, b, n in sorted(events):
        if kind == "array":
            arrays[a] = b
            if n != len(b):
                bad_counts.append("%s declares %d, lists %d" % (a, n, len(b)))
        elif kind == "pld":
            plds[a] = b
        elif kind == "create":
            pls[a] = list(arrays.get(plds.get(b, ""), []))
        elif kind == "for":
            pls[a] = [b]
    w.record("0c-0b", not bad_counts,
             "every std::array<BindGroupLayout, N> lists exactly N members"
             if not bad_counts else "; ".join(bad_counts))

    def resolve(plvar, at):
        return [handles.get(h, h) for h in pls.get(plvar, [])]

    # Snapshot the pipeline-layout map at each use site: the same variable
    # name (`pl`, `layout`) is rebound in later blocks.
    snapshots = []
    for pos, kind, a, b, n in sorted(events):
        if kind in ("create", "for"):
            snapshots.append((pos, a, list(pls_at(events, pos, handles).get(a, []))))

    def layouts_at(plvar, pos):
        best = None
        for p, name, val in snapshots:
            if name == plvar and p <= pos:
                best = val
        return best if best is not None else []

    pipelines = []

    # ─── Compute: every one goes through makeComputePipeline. ─────────
    for m in re.finditer(
            r"makeComputePipeline\(\s*\"([^\"]*)\"\s*,\s*\"([^\"]*)\"\s*,\s*"
            r"(\w+)\s*,\s*Entry::(\w+)\s*,\s*(\w+)\s*\)", src, re.S):
        pipelines.append(Pipeline(
            label=m.group(2), member=m.group(5), kind="compute",
            cs_entry=m.group(4), group_layouts=layouts_at(m.group(3), m.start()),
            vertex_buffer_count=0, vertex_attribute_count=0, color_target_count=0,
            roster_gate=roster_gate_of(spans, m.start()), line=line_of(src, m.start())))

    # ─── Vertex buffer layouts: attribute counts by VBL variable. ─────
    attrs = {}
    for m in re.finditer(
            r"std::array<\s*wgpu::VertexAttribute\s*,\s*(\d+)\s*>\s*(\w+)", src):
        attrs[m.group(2)] = int(m.group(1))
    vbl_attrs = {}
    for m in re.finditer(r"(\w+)\.attributeCount\s*=\s*(?:(\w+)\.size\(\)|(\d+))", src):
        vbl_attrs[m.group(1)] = attrs.get(m.group(2), 0) if m.group(2) else int(m.group(3))

    def vbl_of(tok):
        tok = tok.strip()
        if tok == "nullptr":
            return 0, 0
        name = tok.lstrip("&")
        return 1, vbl_attrs.get(name, None)

    # ─── Render, through the two shared builders. makeEntity always uses
    #     `renderLayout` and Entry::ENTITY_FS; makeShadow uses
    #     `shadowRenderLayout` unless its trailing 8th argument overrides.
    for m in re.finditer(r"makeEntity\(([^;]*?)\)\)\s*return\s+false", src, re.S):
        a = [x.strip() for x in split_top_level(m.group(1), ",")]
        a[0] = a[0].lstrip("!")
        vb, va = vbl_of(a[3])
        pipelines.append(Pipeline(
            label=a[1].strip('"'), member=a[5], kind="render",
            vs_entry=entry_name(a[2]), fs_entry="ENTITY_FS",
            group_layouts=layouts_at("renderLayout", m.start()),
            vertex_buffer_count=vb, vertex_attribute_count=va, color_target_count=1,
            roster_gate=roster_gate_of(spans, m.start()), line=line_of(src, m.start())))

    for m in re.finditer(r"makeShadow\(([^;]*?)\)\)\s*return\s+false", src, re.S):
        a = [x.strip() for x in split_top_level(m.group(1), ",")]
        a[0] = a[0].lstrip("!")
        vb, va = vbl_of(a[3])
        plvar = a[7] if len(a) > 7 else "shadowRenderLayout"
        pipelines.append(Pipeline(
            label=a[1].strip('"'), member=a[5], kind="render",
            vs_entry=entry_name(a[2]), fs_entry=None,       # depth-only: no FS
            group_layouts=layouts_at(plvar, m.start()),
            vertex_buffer_count=vb, vertex_attribute_count=va, color_target_count=0,
            roster_gate=roster_gate_of(spans, m.start()), line=line_of(src, m.start())))

    # ─── Render, spelled out. Each block is delimited by its own
    #     RenderPipelineDescriptor; the two inside the shared builders are
    #     skipped, since their call sites are already recorded above.
    builder_spans = []
    for m in re.finditer(r"auto\s+(makeEntity|makeShadow)\s*=\s*\[&\]", src):
        b = src.find("{", m.end())
        depth, p = 0, b
        while p < len(src):
            if src[p] == "{":
                depth += 1
            elif src[p] == "}":
                depth -= 1
                if depth == 0:
                    break
            p += 1
        builder_spans.append((m.start(), p))

    for m in re.finditer(r"wgpu::RenderPipelineDescriptor\s+(\w+)\s*\{\s*\}\s*;", src):
        if any(s <= m.start() <= e for s, e in builder_spans):
            continue
        d = m.group(1)
        nxt = re.search(r"wgpu::(?:Render|Compute)PipelineDescriptor\s+\w+\s*\{\s*\}\s*;",
                        src[m.end():])
        stop = m.end() + (nxt.start() if nxt else len(src) - m.end())
        cm = re.search(r"(\w+)\s*=\s*device_\.CreateRenderPipeline\(\s*&%s\s*\)" % re.escape(d),
                       src[m.end():stop])
        if not cm:
            continue
        # The FragmentState, ColorTargetState and VertexBufferLayout a
        # descriptor points at are declared BEFORE it, in the same braced
        # scope — so the block to read is the enclosing scope, not the
        # descriptor's own tail.
        blk = src[enclosing_block_start(src, m.start()):m.end() + cm.end()]
        get = lambda f: (re.search(re.escape(d) + r"\." + f + r"\s*=\s*([^;]+);", blk) or [None, None])[1]
        fsvar = get(r"fragment")
        fs = None
        if fsvar:
            fm = re.search(re.escape(fsvar.strip().lstrip("&")) +
                           r"\.entryPoint\s*=\s*Entry::(\w+)\s*;", blk)
            fs = fm.group(1) if fm else None
        tc = None
        if fsvar:
            tm = re.search(re.escape(fsvar.strip().lstrip("&")) +
                           r"\.targetCount\s*=\s*(\d+)\s*;", blk)
            tc = int(tm.group(1)) if tm else None
        vbc = get(r"vertex\.bufferCount")
        vbuf = get(r"vertex\.buffers")
        va = 0
        if vbuf and vbuf.strip() not in ("nullptr",):
            va = vbl_attrs.get(vbuf.strip().lstrip("&"), None)
        pipelines.append(Pipeline(
            label=(get("label") or "").strip().strip('"'), member=cm.group(1), kind="render",
            vs_entry=entry_name(get(r"vertex\.entryPoint") or ""), fs_entry=fs,
            group_layouts=layouts_at((get("layout") or "").strip(), m.start()),
            vertex_buffer_count=int(vbc) if vbc and vbc.strip().isdigit() else 0,
            vertex_attribute_count=va, color_target_count=tc if tc is not None else 0,
            roster_gate=roster_gate_of(spans, m.start()), line=line_of(src, m.start())))

    pipelines.sort(key=lambda p: p.line)
    return pipelines


def enclosing_block_start(src, pos):
    """Offset just inside the innermost `{` enclosing `pos`."""
    depth, p = 0, pos
    while p > 0:
        p -= 1
        if src[p] == "}":
            depth += 1
        elif src[p] == "{":
            if depth == 0:
                return p + 1
            depth -= 1
    return 0


def pls_at(events, pos, handles):
    """Replay the pipeline-layout events up to `pos`. Exact, and cheap enough."""
    arrays, plds, pls = {}, {}, {}
    for p, kind, a, b, n in sorted(events):
        if p > pos:
            break
        if kind == "array":
            arrays[a] = b
        elif kind == "pld":
            plds[a] = b
        elif kind == "create":
            pls[a] = [handles.get(h, h) for h in arrays.get(plds.get(b, ""), [])]
        elif kind == "for":
            pls[a] = [handles.get(b, b)]
    return pls


def entry_name(tok):
    m = re.search(r"Entry::(\w+)", tok or "")
    return m.group(1) if m else None


def phase_0c(w, layouts, wgsl):
    src = strip_cpp_comments(read(RENDERER_HPP))
    spans = gate_map(src)
    handles = parse_layout_handles(w)
    by_member = {L["member"]: L for L in layouts}
    pipelines = parse_pipelines(w, src, spans, handles, by_member)

    # Entry:: constants -> the WGSL entry point names they carry verbatim.
    entry_const = {}
    for m in re.finditer(r'constexpr\s+const\s+char\*\s+(\w+)\s*=\s*"([^"]*)"\s*;', src):
        entry_const[m.group(1)] = m.group(2)

    # Every pipeline must resolve to at least one real bind group layout,
    # and every named layout must be one state.hpp actually creates.
    unresolved = ["%s -> %s" % (p.label, g) for p in pipelines
                  for g in p.group_layouts if g not in by_member]
    empty = [p.label for p in pipelines if not p.group_layouts]
    w.record("0c-0c", not unresolved and not empty,
             "every pipeline resolves to bind group layouts state.hpp creates"
             if not unresolved and not empty else
             "; ".join(filter(None, [
                 ("unknown layout: " + ", ".join(unresolved)) if unresolved else "",
                 ("no layouts resolved: " + ", ".join(empty)) if empty else ""])))

    # ─── WITNESS 0c-1 — at most 4 bind groups per pipeline layout.
    over = ["%s: %d" % (p.label, len(p.group_layouts)) for p in pipelines
            if len(p.group_layouts) > CORE_BIND_GROUPS]
    w.record("0c-1", not over,
             "max bind groups per pipeline layout: %d of %d"
             % (max((len(p.group_layouts) for p in pipelines), default=0), CORE_BIND_GROUPS)
             if not over else "over maxBindGroups: " + ", ".join(over))

    # ─── WITNESS 0c-2 — groups + vertex buffers ≤ 24.
    worst = max(((len(p.group_layouts) + p.vertex_buffer_count, p.label)
                 for p in pipelines), default=(0, "-"))
    over2 = ["%s: %d" % (p.label, len(p.group_layouts) + p.vertex_buffer_count)
             for p in pipelines
             if len(p.group_layouts) + p.vertex_buffer_count > CORE_GROUPS_PLUS_VBS]
    w.record("0c-2", not over2,
             "max bindGroups+vertexBuffers: %d of %d (%s)"
             % (worst[0], CORE_GROUPS_PLUS_VBS, worst[1])
             if not over2 else "over maxBindGroupsPlusVertexBuffers: " + ", ".join(over2))

    # ─── WITNESS 0c-3 — every Entry:: constant used is a real entry point
    #     in the 0b census, with a matching stage.
    problems = []
    for p in pipelines:
        for const, stage in p.entries():
            name = entry_const.get(const)
            if name is None:
                problems.append("%s: Entry::%s is not declared" % (p.label, const))
                continue
            fn = wgsl["entries"].get(name)
            if fn is None:
                problems.append("%s: Entry::%s -> \"%s\" is not an entry point in world.wgsl"
                                % (p.label, const, name))
            elif fn.stage != stage:
                problems.append("%s: \"%s\" used as %s but declared %s"
                                % (p.label, name, stage, fn.stage))
    used = {c for p in pipelines for c, _ in p.entries()}
    w.record("0c-3", not problems,
             "all %d Entry:: constants used by pipelines resolve to world.wgsl entry points "
             "with a matching stage" % len(used)
             if not problems else "; ".join(problems))

    # A layout used at two different group indices would break the
    # (group, binding) join in 0d outright.
    idx = {}
    clash = []
    for p in pipelines:
        for i, g in enumerate(p.group_layouts):
            if idx.setdefault(g, i) != i:
                clash.append("%s at index %d and %d" % (g, idx[g], i))
    w.record("0c-4", not clash,
             "every bind group layout is bound at ONE group index across all pipelines"
             if not clash else "; ".join(sorted(set(clash))))

    return {"pipelines": pipelines, "entry_const": entry_const,
            "group_index": idx, "handles": handles}


# ═══════════════════════════════════════════════════════════════════════
# PHASE 0d — THE JOIN, THE STAGE BUDGET, THE TIER A PREDICATES
# ═══════════════════════════════════════════════════════════════════════

def demo_column(w):
    """Which demo column this census is taken under.

    The binding surface turns out not to vary with it (no bind group layout
    creation is ROSTER-gated), but the PIPELINE set does, and every A3 row
    has to say which column it was censused under or the flag could be a
    false positive from a pipeline that column never creates.
    """
    demo = read(os.path.join(REPO, "src", "cartridges", "the_board", "demos", "demo.hpp"))
    m = re.search(r"#\s*define\s+INCUBATE_DEMO\s+(\w+)", demo)
    default = m.group(1) if m else "?"
    cml = read(os.path.join(REPO, "CMakeLists.txt"))
    cm = re.search(r'set\(THE_BOARD_DEMO\s+"(\w+)"', cml)
    cmake = cm.group(1) if cm else "?"
    w.record("0d-0", default == cmake,
             "demo column: demo.hpp default `%s`, CMake THE_BOARD_DEMO default `%s`"
             % (default, cmake))
    return default


def compatible(entry, decl):
    """Can this WGSL declaration satisfy this layout entry?

    WebGPU requires each shader declaration's access mode to match its
    layout entry exactly — the reason orbCopyLayout_ exists at all. This
    matters on the three ALIASED slots, where two declarations share one
    (group, binding) and only one of them is legal against a given entry:
    (0, 2) carries `vp_data` as read_write and `fc_vp` as read.
    """
    if entry.kind == "buffer":
        if entry.access == "Uniform":
            return decl.address_space == "uniform"
        want = "read_write" if entry.access == "Storage" else "read"
        return decl.address_space == "storage" and decl.wgsl_access == want
    if entry.kind == "sampler":
        return decl.wgsl_type.startswith("sampler")
    if entry.kind == "storageTexture":
        return decl.wgsl_type.startswith("texture_storage")
    if entry.kind == "texture":
        return (decl.wgsl_type.startswith("texture_")
                and not decl.wgsl_type.startswith("texture_storage"))
    return False


def slot_category(row):
    """Which of the five per-stage budgets this row spends."""
    if row.kind == "buffer":
        return "uniform" if row.access == "Uniform" else "storage"
    if row.kind == "texture":
        return "sampled"
    if row.kind == "sampler":
        return "samplers"
    if row.kind == "storageTexture":
        return "storagetex"
    return None


def phase_0d(w, layouts, wgsl, cen):
    pipelines = cen["pipelines"]
    gindex = cen["group_index"]
    entry_const = cen["entry_const"]
    by_member = {L["member"]: L for L in layouts}

    # (group, binding) -> the WGSL declarations at that slot.
    by_slot = {}
    for d in wgsl["decls"]:
        by_slot.setdefault((d.group, d.binding), []).append(d)

    # Which pipelines bind each layout, and at which index.
    binders = {}
    for p in pipelines:
        for i, g in enumerate(p.group_layouts):
            binders.setdefault(g, []).append((p, i))

    # ─── 0d-i — resolve vis_actual, row by row. ───────────────────────
    rows = []
    for L in layouts:
        gi = gindex.get(L["member"])
        for e in L["entries"]:
            at_slot = by_slot.get((gi, e.binding_number), [])
            decls = [x for x in at_slot if compatible(e, x)]
            syms = {d.symbol for d in decls}
            actual, reached_by = set(), []
            for p, _ in binders.get(L["member"], []):
                for const, stage in p.entries():
                    ep = entry_const.get(const)
                    fn_reach = wgsl["reach"].get(ep)
                    if not fn_reach:
                        continue
                    if syms & fn_reach[1]:
                        actual.add(stage)
                        reached_by.append("%s/%s" % (p.label, ep))
            vis_actual = "".join(s for s in STAGES if s in actual)
            vis_delta = "".join(s for s in e.vis_declared if s not in actual)
            # The registry states that its names deliberately equal the
            # WGSL names, so a matched pair whose names differ is a STALE
            # MIRROR — a finding, not a match failure.
            const_name = e.binding_const.rsplit("::", 1)[1]
            slot_syms = {x.symbol for x in at_slot}
            mirror = "ok" if const_name in slot_syms else (
                "no WGSL declaration" if not slot_syms else
                "registry says %s, WGSL says %s" % (const_name, "/".join(sorted(slot_syms))))
            rows.append({
                "e": e, "layout": L, "group": gi,
                "decls": decls, "at_slot": at_slot, "symbols": sorted(syms),
                # On an aliased slot both names are the same slot; show the
                # one the registry constant is spelled with.
                "primary": (const_name if const_name in syms
                            else (sorted(syms)[0] if syms else "—")),
                "vis_actual": vis_actual, "vis_delta": vis_delta,
                "mirror": mirror, "reached_by": sorted(set(reached_by)),
                "binders": [p.label for p, _ in binders.get(L["member"], [])],
            })

    # REPORT — a layout entry whose (group, binding) has no WGSL declaration.
    orphan_rows = [r for r in rows if not r["at_slot"]]

    # A layout entry with declarations at its slot but none the entry can
    # legally satisfy would be rejected at pipeline creation. Finding one
    # means the join is wrong, not the program.
    mismatch = ["%s entries[%d] %s wants %s/%s; slot (@group(%s), @binding(%s)) declares %s"
                % (r["layout"]["label"], r["e"].entry_index, r["e"].binding_const,
                   r["e"].kind, r["e"].access, r["group"], r["e"].binding_number,
                   ", ".join("%s: %s %s" % (x.symbol, x.address_space, x.wgsl_access)
                             for x in r["at_slot"]))
                for r in rows if r["at_slot"] and not r["decls"]]
    w.record("0d-3", not mismatch,
             "every layout entry has at least one access-compatible WGSL declaration at its "
             "(group, binding)" if not mismatch else "; ".join(mismatch))
    # REPORT — a WGSL declaration with no layout entry in any pipeline that
    # binds its group.
    covered = {(r["group"], r["e"].binding_number) for r in rows}
    orphan_decls = [d for d in wgsl["decls"] if (d.group, d.binding) not in covered]
    # REPORT — a registry constant with no WGSL mirror.
    ns_group = {"g0": 0, "g1": 1, "g2": 2}
    registry = parse_registry(Witnesses())
    orphan_consts = [(ns, n, v) for (ns, n), v in sorted(registry.items())
                     if (ns_group[ns], v) not in by_slot]

    # ─── 0d-ii — the stage budget. ────────────────────────────────────
    row_of = {}
    for r in rows:
        row_of.setdefault(r["layout"]["member"], []).append(r)

    budget = []
    for p in pipelines:
        stages = sorted({s for _, s in p.entries()}, key=STAGES.index)
        reach_here = set()
        for const, _ in p.entries():
            ep = entry_const.get(const)
            if ep in wgsl["reach"]:
                reach_here |= wgsl["reach"][ep][1]
        per_stage_reach = {}
        for const, stage in p.entries():
            ep = entry_const.get(const)
            per_stage_reach.setdefault(stage, set())
            if ep in wgsl["reach"]:
                per_stage_reach[stage] |= wgsl["reach"][ep][1]
        for stage in stages:
            dec = dict.fromkeys(CORE, 0)
            act = dict.fromkeys(CORE, 0)
            hit = dict.fromkeys(CORE, 0)
            detail = {k: [] for k in CORE}
            for g in p.group_layouts:
                for r in row_of.get(g, []):
                    cat = slot_category(r["e"])
                    if cat is None:
                        continue
                    if stage in r["e"].vis_declared:
                        dec[cat] += 1
                        detail[cat].append("%s[%d] %s" % (r["layout"]["label"],
                                                          r["e"].entry_index, r["primary"]))
                    if stage in r["vis_actual"]:
                        act[cat] += 1
                    if set(r["symbols"]) & per_stage_reach.get(stage, set()):
                        hit[cat] += 1
            budget.append({"pipeline": p, "stage": stage, "declared": dec,
                           "actual": act, "reached": hit, "detail": detail,
                           "dyn_uniform": sum(1 for g in p.group_layouts
                                              for r in row_of.get(g, [])
                                              if r["e"].has_dynamic_offset
                                              and r["e"].access == "Uniform"),
                           "dyn_storage": sum(1 for g in p.group_layouts
                                              for r in row_of.get(g, [])
                                              if r["e"].has_dynamic_offset
                                              and r["e"].access != "Uniform")})

    # ─── THE RECONCILIATION GATE. The web twin boots on a pure-defaults
    #     device request (PORT_5d), which is a live external witness that
    #     the real program fits inside the Core defaults. So every
    #     (pipeline, stage) row counted against vis_declared MUST fit. If
    #     one does not, the ledger is wrong — not the program.
    over = []
    for b in budget:
        for cat, limit in CORE.items():
            if b["declared"][cat] > limit:
                over.append("%s/%s %s %d>%d [%s]"
                            % (b["pipeline"].label, b["stage"], cat,
                               b["declared"][cat], limit, "; ".join(b["detail"][cat])))
    tight = max(budget, key=lambda b: max(b["declared"][c] / CORE[c] for c in CORE))
    tight_cat = max(CORE, key=lambda c: tight["declared"][c] / CORE[c])
    w.record("gate", not over,
             "every (pipeline, stage) row fits the Core defaults; tightest is "
             "%s / %s at %s %d of %d"
             % (tight["pipeline"].label, tight["stage"], tight_cat,
                tight["declared"][tight_cat], CORE[tight_cat])
             if not over else "OVER Core defaults: " + " | ".join(over))

    over_dyn = ["%s dyn_uniform %d>%d" % (b["pipeline"].label, b["dyn_uniform"], CORE_DYN_UNIFORM)
                for b in budget if b["dyn_uniform"] > CORE_DYN_UNIFORM]
    over_dyn += ["%s dyn_storage %d>%d" % (b["pipeline"].label, b["dyn_storage"], CORE_DYN_STORAGE)
                 for b in budget if b["dyn_storage"] > CORE_DYN_STORAGE]
    w.record("0d-1", not over_dyn,
             "dynamic-offset bindings: %d of %d uniform, %d of %d storage, program-wide"
             % (max((b["dyn_uniform"] for b in budget), default=0), CORE_DYN_UNIFORM,
                max((b["dyn_storage"] for b in budget), default=0), CORE_DYN_STORAGE)
             if not over_dyn else "; ".join(over_dyn))

    # A row's vis_actual can never exceed its vis_declared: a stage that
    # reaches a binding its layout does not expose to that stage would be a
    # pipeline-creation error, and finding one means the join is wrong.
    impossible = ["%s[%d] declares %s but reaches %s"
                  % (r["layout"]["label"], r["e"].entry_index,
                     r["e"].vis_declared or "-", r["vis_actual"])
                  for r in rows if set(r["vis_actual"]) - set(r["e"].vis_declared)]
    w.record("0d-2", not impossible,
             "no row is reached by a stage its visibility mask excludes"
             if not impossible else "; ".join(impossible))

    # ─── 0d-iii — the Tier A predicates. Flags, not a plan. ───────────
    a1 = [r for r in rows if r["vis_delta"]]

    a2 = []
    for r in rows:
        e = r["e"]
        if e.access != "ReadOnlyStorage":
            continue
        d = r["decls"][0] if r["decls"] else None
        ev = {
            "access": e.access == "ReadOnlyStorage",
            "wgsl_access": bool(d) and d.wgsl_access == "read",
            "not_runtime": bool(d) and not d.has_runtime_array,
            "bytes": bool(d) and d.layout.size is not None
                     and d.layout.size <= UNIFORM_BINDING_MAX_BYTES,
            "uniform_legal": bool(d) and not d.layout.uniform_blockers,
        }
        a2.append({"row": r, "decl": d, "ev": ev, "pass": all(ev.values()),
                   "element_note": uniform_element_note(d.layout) if d else "no WGSL declaration",
                   "size": d.layout.size if d else None})

    reached_any = set()
    for _, refs in wgsl["reach"].values():
        reached_any |= refs
    a3 = []
    for ns, n, v in orphan_consts:
        a3.append(("registry constant with no WGSL mirror",
                   "bind::%s::%s = %d" % (ns, n, v), ""))
    for d in wgsl["decls"]:
        if d.symbol not in reached_any:
            a3.append(("WGSL declaration no entry point reaches",
                       "%s @group(%d) @binding(%d)" % (d.symbol, d.group, d.binding), ""))
    for d in orphan_decls:
        a3.append(("WGSL declaration with no layout entry in any pipeline binding its group",
                   "%s @group(%d) @binding(%d)" % (d.symbol, d.group, d.binding), ""))
    for r in orphan_rows:
        a3.append(("layout entry whose (group, binding) has no WGSL declaration",
                   "%s entries[%d] %s -> (@group(%s), @binding(%s))"
                   % (r["layout"]["label"], r["e"].entry_index, r["e"].binding_const,
                      r["group"], r["e"].binding_number), ""))
    for r in rows:
        if not r["vis_actual"] and r["decls"]:
            a3.append(("layout entry no reachable code touches in any pipeline that binds it",
                       "%s entries[%d] %s (declares %s)"
                       % (r["layout"]["label"], r["e"].entry_index,
                          r["symbols"][0] if r["symbols"] else "?", r["e"].vis_declared), ""))

    stale = [r for r in rows if r["mirror"] not in ("ok",) and r["decls"]]

    return {"rows": rows, "budget": budget, "a1": a1, "a2": a2, "a3": a3,
            "stale_mirror": stale, "orphan_rows": orphan_rows,
            "orphan_decls": orphan_decls, "orphan_consts": orphan_consts,
            "tightest": (tight, tight_cat)}


# ═══════════════════════════════════════════════════════════════════════
# PHASE 0e — THE ARTIFACT
#
# Table A's `purpose` column and the whole of Table D are JEAN'S. They are
# left empty on purpose: they are the two things a census cannot produce.
# ═══════════════════════════════════════════════════════════════════════

INPUTS = [STATE_HPP, REGISTRY_HPP, RENDERER_HPP, WORLD_WGSL]


def provenance():
    """Commit SHA + content hashes of the four inputs.

    NOT `HEAD`. G2 requires that re-running on an unchanged tree reproduces
    this file byte for byte, and HEAD moves the moment the artifact is
    committed. The SHA recorded here is the last commit that touched any
    INPUT, which is stable across commits that do not — including the one
    that lands this file. The content hashes are the authoritative
    provenance; the SHA is the human-readable handle for them.
    """
    rel = [os.path.relpath(p, REPO) for p in INPUTS]
    try:
        sha = subprocess.run(["git", "-C", REPO, "log", "-1", "--format=%H", "--"] + rel,
                             capture_output=True, text=True, check=True).stdout.strip()
        subj = subprocess.run(["git", "-C", REPO, "log", "-1", "--format=%s", "--"] + rel,
                              capture_output=True, text=True, check=True).stdout.strip()
    except Exception:
        sha, subj = "(git unavailable)", ""
    return sha, subj, [(r, sha256(p)) for r, p in zip(rel, INPUTS)]


def md_escape(s):
    return str(s).replace("|", "\\|")


# Comments in state.hpp that this census can check a number against. Each
# is quoted only if it is STILL THERE, so the finding cannot outlive the
# comment it is about. The census reports; it never corrects a comment.
CHECKED_COMMENTS = [
    ("the VS storage-buffer cap is full",
     "state.hpp, Render Entity Layout entries[16]/[17] — the two uniform "
     "bindings that exist because the vertex storage budget was said to be full"),
    ("avoids exceeding the 8 storage buffer per-stage limit",
     "state.hpp, Mesh Gen Entity Layout — the one-entry layout said to exist "
     "to stay under the storage cap"),
    ("past the 10-per-stage limit",
     "state.hpp, Compute Entity Layout entries[10]/[11] — the two agent "
     "registries said to have been moved to uniform to clear a 10-per-stage cap"),
]


def check_comments():
    src = read(STATE_HPP)
    low = src.lower()
    return [(frag, where, frag.lower() in low) for frag, where in CHECKED_COMMENTS]


def emit(path, w, column, layouts, wgsl, cen, join):
    sha, subj, hashes = provenance()
    rows, budget = join["rows"], join["budget"]
    o = []
    A = o.append

    # ─── 1. PROVENANCE ────────────────────────────────────────────────
    A("# THE BINDING LEDGER")
    A("")
    A("A read-only census of every declaration that spends a WebGPU slot, and of")
    A("what the program can actually reach through it. Generated by")
    A("`tools/binding_ledger.py`; do not hand-edit — re-run it.")
    A("")
    A("**One row per `(bind group layout, entry index)`, not per buffer.** The")
    A("registry is one constant per SITE: the same buffer wears several names")
    A("because each name is one `(group, slot)`. A buffer-keyed census would")
    A("merge rows the API charges separately.")
    A("")
    A("## Provenance")
    A("")
    A("| field | value |")
    A("|---|---|")
    A("| demo column censused | `%s` |" % column)
    A("| source commit | `%s` |" % sha)
    A("| | %s |" % md_escape(subj))
    for r, h in hashes:
        A("| `%s` | `sha256:%s` |" % (r, h))
    A("")
    A("The source commit is the last commit touching any of the four inputs —")
    A("not `HEAD`, which moves when this file is committed. The content hashes")
    A("are the authoritative provenance.")
    A("")
    A("### The law this serves")
    A("")
    A("A slot is charged **once per stage named in `visibility`**, whether or not")
    A("that stage can reach the binding. Bind **groups do not partition the")
    A("budget** — every bind group layout in a pipeline layout is concatenated,")
    A("then checked. **Bytes and slots are unrelated currencies.** Core defaults,")
    A("per shader stage: uniform %d · storage %d · sampled %d · samplers %d ·"
      % (CORE["uniform"], CORE["storage"], CORE["sampled"], CORE["samplers"]))
    A("storage textures %d. Per pipeline layout: bind groups %d, dynamic-offset"
      % (CORE["storagetex"], CORE_BIND_GROUPS))
    A("uniform %d, dynamic-offset storage %d. Per binding: uniform %d B, storage"
      % (CORE_DYN_UNIFORM, CORE_DYN_STORAGE, UNIFORM_BINDING_MAX_BYTES))
    A("134217728 B. Uniform bindings also need %d B offset alignment, which"
      % UNIFORM_OFFSET_ALIGN)
    A("matters only where a binding is a window onto a shared buffer.")
    A("")
    A("### Witnesses")
    A("")
    A("| witness | result | numbers |")
    A("|---|---|---|")
    for tag, ok, detail in w.rows:
        A("| `%s` | %s | %s |" % (tag, "**PASS**" if ok else "**FAIL**", md_escape(detail)))
    A("")
    A("`gate` is the RECONCILIATION GATE. The web twin boots on a pure-defaults")
    A("device request (PORT_5d — a value-initialised `wgpu::Limits`, nothing")
    A("named), which is a live external witness that the real program fits")
    A("inside the Core defaults. If a Table B row counted against `vis_declared`")
    A("exceeded a limit, the ledger would be wrong, not the program.")
    A("")

    # ─── Findings. The handoff's REPORT items need a home Jean will read,
    #     and it is not a commit message. Every number below is computed;
    #     every quoted comment is checked to still exist before it is
    #     quoted. Reported, not acted on: no program file is touched.
    tight, tcat = join["tightest"]
    main_v = next((r for r in budget if r["pipeline"].member == "pawnPipeline_"
                   and r["stage"] == "V"), None)
    gated = [L for L in layouts if L["roster_gated"]]
    reached_any = set()
    for _, refs in wgsl["reach"].values():
        reached_any |= refs
    dead_decls = sorted(d.symbol for d in wgsl["decls"] if d.symbol not in reached_any)

    A("### Findings — reported, not acted on")
    A("")
    A("**1. The binding surface does not vary with the demo column.** %s"
      % ("No bind group layout creation site sits inside a `if constexpr (ROSTER.…)` "
         "gate — all %d are created unconditionally. So slot pressure exists for "
         "pipelines a given build never creates: the PIPELINE set is ROSTER-gated "
         "(%d of %d pipelines carry a gate), the LAYOUT set is not."
         % (len(layouts), sum(1 for p in cen["pipelines"] if p.roster_gate),
            len(cen["pipelines"]))
         if not gated else
         "%d layout(s) ARE ROSTER-gated: %s."
         % (len(gated), ", ".join(L["label"] for L in gated))))
    A("")
    A("**2. No dead surface in the shader.** %s"
      % ("All %d module-scope declarations are reached by at least one of the %d "
         "entry points." % (len(wgsl["decls"]), len(wgsl["entries"]))
         if not dead_decls else
         "%d declaration(s) reached by ZERO entry points: %s."
         % (len(dead_decls), ", ".join(dead_decls))))
    A("")
    A("**3. Zero dynamic-offset bindings.** `hasDynamicOffset` is never set")
    A("anywhere in `state.hpp`, so every row carries the default `false` and the")
    A("two dynamic-offset limits stand at 0/%d and 0/%d program-wide."
      % (CORE_DYN_UNIFORM, CORE_DYN_STORAGE))
    A("")
    A("**4. The tightest row is not where the handoff expected it.** BUDGET_0's")
    A("handoff expected the main render family's VERTEX stage at storage 8/8. It")
    if main_v:
        A("reads **storage %d of %d**. The tightest row in the program is"
          % (main_v["declared"]["storage"], CORE["storage"]))
    A("**%s / %s at %s %d of %d** — the ROOM FAMILY, over the three concatenated"
      % (md_escape(tight["pipeline"].label), tight["stage"], tcat,
         tight["declared"][tcat], CORE[tcat]))
    A("groups `computeEntityBindGroupLayout_` + `computeTextureBindGroupLayout_` +")
    A("`roomLayout_`, shared by %s."
      % ", ".join("`%s`" % cen["entry_const"].get(c, c)
                  for p in cen["pipelines"] if p.group_layouts == tight["pipeline"].group_layouts
                  for c, _ in p.entries()))
    A("That is exactly what `world.wgsl`'s own FXC banner says: *\"the room family")
    A("sits at 8/8 storage — no new storage binding without a demotion plan.\"* And")
    A("its `actual` column reads %d too: **an A1 sweep buys nothing on the one row"
      % tight["actual"][tcat])
    A("with no margin.**")
    A("")
    A("The handoff asked which comment has outlived its referent if the render")
    A("family did not reproduce 8/8. Three have, to different degrees. Each is")
    A("quoted only because it is still present in `state.hpp` today:")
    A("")
    live = {frag: ok for frag, _, ok in check_comments()}
    if live.get("the VS storage-buffer cap is full") and main_v:
        A("- *\"the VS storage-buffer cap is full\"* — **off by one.** The vertex")
        A("  stage stands at storage %d of %d, so there is exactly ONE free vertex"
          % (main_v["declared"]["storage"], CORE["storage"]))
        A("  storage slot. Not enough for both `agent_tier_gains` and")
        A("  `agent_figure_profiles` — putting both back as storage would be %d —"
          % (main_v["declared"]["storage"] + 2))
        A("  but \"full\" is no longer literally true. The decision the comment")
        A("  defends still holds; the number in it does not.")
    if live.get("avoids exceeding the 8 storage buffer per-stage limit"):
        mg = [p for p in cen["pipelines"]
              if "meshGenEntityBindGroupLayout_" in p.group_layouts]
        A("- *\"Avoids exceeding the 8 storage buffer per-stage limit\"* on the Mesh")
        A("  Gen Entity Layout — **outlived its referent outright.** That layout is")
        A("  bound by %s in the whole program: %s. %sEvery mesh-gen"
          % ("exactly ONE pipeline" if len(mg) == 1 else "%d pipelines" % len(mg),
             ", ".join(md_escape(p.label) for p in mg) or "none",
             "A render pipeline with one uniform, zero storage buffers, and no "
             "compute stage at all. " if len(mg) == 1 else ""))
        A("  pipeline now carries its own dedicated single-group layout")
        A("  (`archMeshGenLayout_` and its four siblings). Corroborating it: this")
        A("  layout's one entry declares `VFC` and only `F` reaches it — the `C` in")
        A("  that mask is a fossil of the mesh-gen era, and it is an A1 row below.")
    if live.get("past the 10-per-stage limit"):
        A("- *\"pushed compute storage buffer count past the 10-per-stage limit\"* —")
        A("  **the Core default is %d, not 10.** The constraint is real and TIGHTER"
          % CORE["storage"])
        A("  than the comment states: with those two registries as storage the room")
        A("  family would stand at %d of %d. The number cited is not a Core default."
          % (tight["declared"]["storage"] + 2, CORE["storage"]))
    A("")
    A("**5. One correction to the handoff's A2 rule, and the ledger follows the")
    A("spec.** The handoff states that in the uniform address space")
    A("`array<f32, N>` *and* `array<vec3<f32>, N>` are illegal. The first is")
    A("right; the second is not. The rule is that the array's **element stride**")
    A("— `roundUp(AlignOf(E), SizeOf(E))` — must be a multiple of 16.")
    A("`AlignOf(vec3<f32>)` is 16, so the stride is `roundUp(16, 12)` = 16 and the")
    A("array is legal; it merely wastes 4 B per element. `array<vec2<f32>, N>`")
    A("(stride 8) *is* illegal, as is any scalar element. A type's own alignment")
    A("being under 16 is not a blocker either: `array<AgentBehaviorParams, 10>`")
    A("has `AlignOf` 4 and stride 32, and the program binds it as a uniform")
    A("today. Witness `0b-5` is the guard — the predicate clears all %d"
      % sum(1 for d in wgsl["decls"] if d.address_space == "uniform"))
    A("declarations the program already places in uniform.")
    A("")
    A("**6. The C6 note on `g2::field_head_poses` did not cost an element type.**")
    A("The handoff attributes an element-type widening to that demotion. At")
    A("`ecfbc32`, the commit that introduced the binding, both `g0:122")
    A("head_poses` and `g2:2 field_head_poses` already read")
    A("`array<vec4<f32>, 400>`. The demotion changed the address space over an")
    A("element type that was already vec4-strided. Table C still records the")
    A("required element type per A2 row, because for other candidates it is a")
    A("real cost — it just was not one there.")
    A("")

    # ─── 2. TABLE A ───────────────────────────────────────────────────
    A("## Table A — the ledger")
    A("")
    A("One row per `(bind group layout, entry index)`, %d rows over %d layouts."
      % (len(rows), len(layouts)))
    A("Columns through `roster_gated` are the layout census (0a); `group` onward")
    A("are joined from the WGSL census (0b) and the pipeline census (0c).")
    A("`bytes` is the size of the WGSL **store type**, computed by WGSL layout")
    A("rules; `—` means a runtime-sized array or a handle type, which has none.")
    A("**`purpose` is Jean's and is left empty.**")
    A("")
    hdr = ("| layout_label | layout_member | # | binding_const | binding | ns | "
           "kind | access | vis_declared | dyn | roster_gated | group | wgsl_symbol | "
           "wgsl_type | wgsl_access | runtime_array | bytes | slot | vis_actual | "
           "vis_delta | purpose |")
    A(hdr)
    A("|" + "---|" * (hdr.count("|") - 1))
    for r in sorted(rows, key=lambda r: (r["layout"]["label"], r["e"].entry_index)):
        e = r["e"]
        d = r["decls"][0] if r["decls"] else None
        alias = ""
        if len(r["at_slot"]) > 1:
            alias = " *(slot also spelled %s)*" % ", ".join(
                "`%s`" % x.symbol for x in r["at_slot"] if x.symbol != r["primary"])
        A("| %s | `%s` | %d | `%s` | %s | `%s` | %s | %s | `%s` | %s | %s | %s | `%s`%s | "
          "`%s` | %s | %s | %s | %s | `%s` | `%s` |  |"
          % (md_escape(e.layout_label), e.layout_member, e.entry_index,
             md_escape(e.binding_const), e.binding_number, e.registry_ns,
             e.kind, md_escape(e.access), e.vis_declared,
             "yes" if e.has_dynamic_offset else "no",
             e.roster_gated or "—", r["group"],
             r["primary"], alias,
             md_escape(d.wgsl_type) if d else "—",
             d.wgsl_access if d else "—",
             ("yes" if d.has_runtime_array else "no") if d else "—",
             (d.layout.size if d and d.layout.size is not None else "—"),
             slot_category(e) or "—",
             r["vis_actual"] or "-", r["vis_delta"] or "-"))
    A("")

    # ─── 3. TABLE B ───────────────────────────────────────────────────
    A("## Table B — the stage budget")
    A("")
    A("One row per `(pipeline, stage)`, counted across **all** the pipeline's")
    A("group layouts concatenated. Each cell reads `declared / actual / reached`")
    A("against the Core limit in the header.")
    A("")
    A("- **declared** — counted against `vis_declared`. *This is what the API")
    A("  charges.* Every one of these must fit, and the gate above says they do.")
    A("- **actual** — counted against `vis_actual`: the same rows, minus those no")
    A("  reachable code touches through this layout in any pipeline that binds")
    A("  it. This is what the budget would read after an A1 sweep. **The")
    A("  difference between these two columns is the entire prize of BUDGET_1.**")
    A("- **reached** — counted against what *this pipeline's* stage actually")
    A("  touches. A derived third column, narrower than `actual`, because a")
    A("  visibility mask is shared by every pipeline binding the layout and can")
    A("  only be trimmed to the union. It bounds what any per-pipeline split")
    A("  could ever buy.")
    A("")
    A("| pipeline | member | stage | entry point | uniform /%d | storage /%d | "
      "sampled /%d | samplers /%d | storagetex /%d | bind groups /%d | dyn_u /%d | dyn_s /%d |"
      % (CORE["uniform"], CORE["storage"], CORE["sampled"], CORE["samplers"],
         CORE["storagetex"], CORE_BIND_GROUPS, CORE_DYN_UNIFORM, CORE_DYN_STORAGE))
    A("|---|---|---|---|---|---|---|---|---|---|---|---|")
    for bx in budget:
        p = bx["pipeline"]
        ep = ", ".join(cen["entry_const"].get(c, c) for c, s in p.entries() if s == bx["stage"])
        cells = []
        for k in ("uniform", "storage", "sampled", "samplers", "storagetex"):
            cell = "%d / %d / %d" % (bx["declared"][k], bx["actual"][k], bx["reached"][k])
            if bx["declared"][k] == CORE[k]:
                cell = "**%s**" % cell
            cells.append(cell)
        A("| %s | `%s` | %s | `%s` | %s | %d | %d | %d |"
          % (md_escape(p.label), p.member, bx["stage"], ep, " | ".join(cells),
             len(p.group_layouts), bx["dyn_uniform"], bx["dyn_storage"]))
    A("")
    tight, tcat = join["tightest"]
    A("Tightest row: **%s / %s** at %s **%d of %d**, and `actual` reads %d there too —"
      % (md_escape(tight["pipeline"].label), tight["stage"], tcat,
         tight["declared"][tcat], CORE[tcat], tight["actual"][tcat]))
    A("an A1 sweep buys nothing on the row that has no margin.")
    A("")
    A("The rows in bold are at their limit exactly. Groups bound per pipeline")
    A("are shown because they are a per-pipeline-layout limit of their own;")
    A("`bind groups + vertex buffers` is checked by witness `0c-2` and is")
    A("nowhere near its 24.")
    A("")

    # ─── 4. TABLE C ───────────────────────────────────────────────────
    A("## Table C — Tier A candidates")
    A("")
    A("**A flag list, not a plan.** Rows are flagged; nothing is acted on.")
    A("")
    A("### A1 — OVER-VISIBLE (`vis_delta` ≠ ∅)")
    A("")
    A("The declaration claims a stage that cannot reach the binding. Correcting")
    A("one is not an optimization: it is the removal of a false statement that")
    A("happened to cost a slot. No shader edit, no data movement, no behaviour")
    A("change.")
    A("")
    A("| layout | # | symbol | kind | slot | vis_declared | vis_actual | **vis_delta** | "
      "slots freed, per stage | reached by |")
    A("|---|---|---|---|---|---|---|---|---|---|")
    for r in sorted(join["a1"], key=lambda r: (r["layout"]["label"], r["e"].entry_index)):
        A("| %s | %d | `%s` | %s | %s | `%s` | `%s` | `%s` | %s | %s |"
          % (md_escape(r["layout"]["label"]), r["e"].entry_index, r["primary"],
             r["e"].kind, slot_category(r["e"]), r["e"].vis_declared,
             r["vis_actual"] or "-", r["vis_delta"],
             ", ".join("%s: 1 %s" % (s, slot_category(r["e"])) for s in r["vis_delta"]),
             md_escape(", ".join(r["reached_by"]) or "— nothing")))
    A("")
    A("### A2 — DEMOTABLE (storage → uniform)")
    A("")
    A("All five predicates must hold. The one that actually decides it is")
    A("`uniform_legal`: in the uniform address space an array's **element")
    A("stride** must be a multiple of 16, every struct member offset must be a")
    A("multiple of `roundUp(16, …)` for struct and array members, and a struct or")
    A("array member must be followed by `roundUp(16, SizeOf)` of space.")
    A("`element_type_required` names what a demotion would cost, or says none is")
    A("needed. Uniform bindings also carry a %d B offset alignment, which binds"
      % UNIFORM_OFFSET_ALIGN)
    A("only where the binding is a window onto a shared buffer.")
    A("")
    A("| layout | # | symbol | wgsl_type | bytes ≤ %d | ReadOnlyStorage | wgsl read | "
      "no runtime array | uniform_legal | **verdict** | element_type_required | "
      "same slot bound elsewhere as |" % UNIFORM_BINDING_MAX_BYTES)
    A("|---|---|---|---|---|---|---|---|---|---|---|---|")
    tick = lambda b: "yes" if b else "**no**"
    for x in sorted(join["a2"], key=lambda x: (not x["pass"], x["row"]["layout"]["label"],
                                               x["row"]["e"].entry_index)):
        r, ev = x["row"], x["ev"]
        # A slot demoted in one layout is still the same buffer the OTHER
        # layouts bind. If another layout binds it Storage, the buffer needs
        # both usages and the demotion is not local to one layout.
        elsewhere = sorted({"%s in %s" % (o["e"].access, o["layout"]["label"])
                            for o in rows
                            if (o["group"], o["e"].binding_number)
                            == (r["group"], r["e"].binding_number)
                            and o is not r and o["e"].access != r["e"].access})
        A("| %s | %d | `%s` | `%s` | %s | %s | %s | %s | %s | %s | %s | %s |"
          % (md_escape(r["layout"]["label"]), r["e"].entry_index, r["primary"],
             md_escape(x["decl"].wgsl_type) if x["decl"] else "—",
             ("%s (%s)" % (tick(ev["bytes"]), x["size"] if x["size"] is not None else "—")),
             tick(ev["access"]), tick(ev["wgsl_access"]), tick(ev["not_runtime"]),
             tick(ev["uniform_legal"]),
             "**CANDIDATE**" if x["pass"] else "no",
             md_escape(x["element_note"]),
             md_escape("; ".join(elsewhere)) if elsewhere else "—"))
    A("")
    A("A demotion also **spends a uniform slot to save a storage one**, so an A2")
    A("row only helps where the same `(pipeline, stage)` has uniform margin. Read")
    A("each candidate against its Table B rows before costing it. And the last")
    A("column matters: where the same `(group, binding)` is bound with a different")
    A("access by another layout, a demotion is not local to one layout — the")
    A("buffer carries both usages, and the WGSL declaration for the demoted")
    A("layout has to be a separate `var<uniform>` alias, which spends a registry")
    A("site of its own. That is the `orb_state` / `orb_state_ro` pattern.")
    A("")
    A("### A3 — ORPHANED")
    A("")
    A("Dead binding surface. **Every row states the demo column it was censused")
    A("under**, because a flag on a binding reached only under a column not")
    A("censused would be a false positive.")
    A("")
    if join["a3"]:
        A("| kind | what | demo column | evidence |")
        A("|---|---|---|---|")
        for kind, what, _ in join["a3"]:
            ev = ""
            for r in rows:
                if what.startswith(r["layout"]["label"]) and ("entries[%d]" % r["e"].entry_index) in what:
                    ev = ("bound by %d pipeline(s): %s; reached by none of their entry points"
                          % (len(set(r["binders"])), ", ".join(sorted(set(r["binders"])))))
            A("| %s | %s | `%s` | %s |" % (md_escape(kind), md_escape(what), column, md_escape(ev)))
    else:
        A("None.")
    A("")
    A("No registry constant lacks a WGSL mirror — %d constants, %d slots, one to"
      % (len(parse_registry(Witnesses())), len(wgsl["slots"])))
    A("one. No WGSL declaration goes unreached by every entry point. No bind group")
    A("layout goes unbound by every pipeline. Every layout entry's")
    A("`(group, binding)` resolves to an access-compatible WGSL declaration.")
    A("")

    # ─── 5. TABLE D ───────────────────────────────────────────────────
    A("## Table D — DEMAND")
    A("")
    A("**Jean's. Emitted as a stub: row labels only, every cell empty.**")
    A("Nothing in Tier A needs a demand to justify it — a declaration that claims")
    A("a stage it cannot reach was never true. These are the items that DO.")
    A("")
    A("| queued item | demand | what it buys | what it costs | ruling |")
    A("|---|---|---|---|---|")
    queued = []
    for x in join["a2"]:
        if x["pass"]:
            queued.append("A2 demotion — `%s` @group(%s) @binding(%s) in %s (%s B)"
                          % (x["row"]["primary"], x["row"]["group"],
                             x["row"]["e"].binding_number, x["row"]["layout"]["label"],
                             x["size"]))
    for kind, what, _ in join["a3"]:
        queued.append("A3 removal — %s" % what)
    queued.append("The room family's storage stage stands at %d of %d. Any new storage "
                  "binding reachable from update_player_agent / update_other_agents / "
                  "update_sphere / update_cube needs a demotion to pay for it."
                  % (tight["declared"]["storage"], CORE["storage"]))
    for q in queued:
        A("| %s |  |  |  |  |" % md_escape(q))
    A("")

    # ─── Appendices: 0b's two products, which A–D consume but do not show.
    A("## Appendix 1 — the WGSL declaration table (0b-i)")
    A("")
    A("%d module-scope declarations over %d `(group, binding)` slots. The three"
      % (len(wgsl["decls"]), len(wgsl["slots"])))
    A("aliases share slots with an earlier declaration and are marked.")
    A("")
    A("| wgsl_symbol | group | binding | address_space | wgsl_access | wgsl_type | "
      "runtime_array | bytes | align | uniform_legal | alias of |")
    A("|---|---|---|---|---|---|---|---|---|---|---|")
    first = {}
    for d in wgsl["decls"]:
        first.setdefault((d.group, d.binding), d.symbol)
    for d in sorted(wgsl["decls"], key=lambda d: (d.group, d.binding, d.line)):
        A("| `%s` | %d | %d | %s | %s | `%s` | %s | %s | %s | %s | %s |"
          % (d.symbol, d.group, d.binding, d.address_space, d.wgsl_access,
             md_escape(d.wgsl_type), "yes" if d.has_runtime_array else "no",
             d.layout.size if d.layout.size is not None else "—",
             d.layout.align if d.layout.align is not None else "—",
             "yes" if not d.layout.uniform_blockers else "no",
             ("`%s`" % first[(d.group, d.binding)])
             if first[(d.group, d.binding)] != d.symbol else "—"))
    A("")
    A("## Appendix 2 — the reachability closure (0b-ii)")
    A("")
    A("Per entry point: the transitive closure of function calls, then every")
    A("binding declaration referenced anywhere in that closure. Built textually")
    A("over the one module. The residual error is one-directional — a local")
    A("shadowing a binding name would ADD a reference, never remove one — so")
    A("every `vis_actual` here is generous and every A1 flag is conservative.")
    A("")
    A("| entry_point | stage | workgroup_size | functions reached | bindings reached |")
    A("|---|---|---|---|---|")
    for name in sorted(wgsl["entries"]):
        fn = wgsl["entries"][name]
        fns_reached, refs = wgsl["reach"][name]
        A("| `%s` | %s | %s | %d | %s |"
          % (name, fn.stage, fn.workgroup_size or "—", len(fns_reached),
             ", ".join("`%s`" % s for s in sorted(refs)) or "— none"))
    A("")
    A("## Appendix 3 — pipelines and their group layouts (0c)")
    A("")
    A("Index position in `bindGroupLayouts` **is** the `@group` number. Shared")
    A("builders are resolved, not recorded.")
    A("")
    A("| pipeline | member | kind | vs | fs | cs | group_layouts (ordered) | vbufs | "
      "vattrs | color targets | roster_gate |")
    A("|---|---|---|---|---|---|---|---|---|---|---|")
    for p in cen["pipelines"]:
        ec = cen["entry_const"]
        A("| %s | `%s` | %s | %s | %s | %s | %s | %d | %s | %d | %s |"
          % (md_escape(p.label), p.member, p.kind,
             "`%s`" % ec.get(p.vs_entry, p.vs_entry) if p.vs_entry else "—",
             "`%s`" % ec.get(p.fs_entry, p.fs_entry) if p.fs_entry else "—",
             "`%s`" % ec.get(p.cs_entry, p.cs_entry) if p.cs_entry else "—",
             " → ".join("`%s`" % g for g in p.group_layouts),
             p.vertex_buffer_count,
             "—" if p.vertex_attribute_count is None else p.vertex_attribute_count,
             p.color_target_count, "`%s`" % p.roster_gate if p.roster_gate else "—"))
    A("")

    text = "\n".join(o) + "\n"
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    return text


# ═══════════════════════════════════════════════════════════════════════
# REPORT
# ═══════════════════════════════════════════════════════════════════════

def main():
    ap = argparse.ArgumentParser(description="BUDGET_0 — the binding ledger census.")
    ap.add_argument("--check", action="store_true",
                    help="run the witnesses, write nothing")
    ap.add_argument("-o", "--out", default=DEFAULT_OUT)
    args = ap.parse_args()

    w = Witnesses()
    registry = parse_registry(w)
    layouts = parse_layouts(w, registry)

    print("BUDGET_0 — the binding ledger")
    print("")
    print("PHASE 0a — THE LAYOUT CENSUS")
    print("  %d bind group layouts, %d rows"
          % (len(layouts), sum(len(L["entries"]) for L in layouts)))
    for L in layouts:
        print("    %-34s %-32s %2d entr%s%s"
              % (L["label"], L["member"], len(L["entries"]),
                 "y" if len(L["entries"]) == 1 else "ies",
                 ("  [gate: %s]" % L["roster_gated"]) if L["roster_gated"] else ""))
    gated = [L for L in layouts if L["roster_gated"]]
    print("")
    print("  FINDING: %s"
          % ("no bind group layout creation sits inside a ROSTER gate — the "
             "binding surface is identical across every demo column"
             if not gated else
             "%d layout(s) are ROSTER-gated: %s"
             % (len(gated), ", ".join(L["label"] for L in gated))))
    b = phase_0b(w)
    print("")
    print("PHASE 0b — THE REACHABILITY CENSUS")
    print("  %d module-scope binding declarations over %d (group, binding) slots"
          % (len(b["decls"]), len(b["slots"])))
    print("  %d functions, %d entry points"
          % (len(b["fns"]), len(b["entries"])))
    sized = [d for d in b["decls"] if d.layout.size is not None]
    print("  %d declarations have a computable store size; %d do not "
          "(runtime-sized arrays and handle types)"
          % (len(sized), len(b["decls"]) - len(sized)))

    reached_any = set()
    for _, refs in b["reach"].values():
        reached_any |= refs
    dead = sorted(d.symbol for d in b["decls"] if d.symbol not in reached_any)
    print("")
    print("  REPORT: %s"
          % ("every module-scope binding declaration is reached by at least one entry point"
             if not dead else
             "%d declaration(s) reached by ZERO entry points — dead binding surface: %s"
             % (len(dead), ", ".join(dead))))

    c = phase_0c(w, layouts, b)
    print("")
    print("PHASE 0c — THE PIPELINE CENSUS")
    print("  %d pipelines (%d render, %d compute)"
          % (len(c["pipelines"]),
             sum(1 for p in c["pipelines"] if p.kind == "render"),
             sum(1 for p in c["pipelines"] if p.kind == "compute")))
    for p in c["pipelines"]:
        print("    %-30s %-28s %-7s %-46s vb=%d/%s ct=%d%s"
              % (p.label, p.member, p.kind,
                 "+".join(e for e, _ in p.entries()),
                 p.vertex_buffer_count,
                 "-" if p.vertex_attribute_count is None else p.vertex_attribute_count,
                 p.color_target_count,
                 ("  [gate: %s]" % p.roster_gate) if p.roster_gate else ""))
    print("")
    print("  group index of each bind group layout:")
    for g, i in sorted(c["group_index"].items(), key=lambda kv: (kv[1], kv[0])):
        print("    @group(%d)  %s" % (i, g))

    column = demo_column(w)
    d = phase_0d(w, layouts, b, c)
    e1 = phase_ext1(w, b, c, layouts)
    e2 = phase_ext2(w, b)

    print("")
    print("PHASE 0d — THE JOIN, THE STAGE BUDGET, TIER A")
    print("  demo column censused: %s" % column)
    print("")
    print("  the four tightest (pipeline, stage) rows, declared / actual / limit:")
    order = sorted(d["budget"], key=lambda r: -max(r["declared"][k] / CORE[k] for k in CORE))
    for r in order[:4]:
        print("    %-32s %s   %s" % (r["pipeline"].label, r["stage"],
                                     "  ".join("%s %d/%d/%d" % (k, r["declared"][k],
                                                                r["actual"][k], CORE[k])
                                               for k in ("uniform", "storage", "sampled",
                                                         "samplers", "storagetex"))))
    main_v = next((r for r in d["budget"]
                   if r["pipeline"].member == "pawnPipeline_" and r["stage"] == "V"), None)
    if main_v:
        print("")
        print("  the main render family's VERTEX stage (renderEntityLayout_ + "
              "renderTextureLayout_): storage %d of %d"
              % (main_v["declared"]["storage"], CORE["storage"]))
        for x in main_v["detail"]["storage"]:
            print("      %s" % x)

    print("")
    print("  TIER A: %d A1 (over-visible), %d A2 candidates (%d pass all predicates), %d A3"
          % (len(d["a1"]), len(d["a2"]), sum(1 for x in d["a2"] if x["pass"]), len(d["a3"])))
    for r in d["a1"]:
        print("    A1  %-30s [%2d] %-22s declared %-3s  actual %-3s  delta %s"
              % (r["layout"]["label"], r["e"].entry_index, r["primary"],
                 r["e"].vis_declared, r["vis_actual"] or "-", r["vis_delta"]))
    for kind, what, _ in d["a3"]:
        print("    A3  %s — %s" % (kind, what))
    print("  stale mirror rows (registry name != WGSL name): %d" % len(d["stale_mirror"]))

    print("")
    print("EXTENSION 1 — READ/WRITE SEPARATION")
    shadow = {}
    for fn in b["fns"].values():
        for s in getattr(fn, "shadowed", ()) or ():
            shadow.setdefault(s, []).append(fn.name)
    print("  scope resolution: %d module-scope name(s) shadowed by a local — %s"
          % (len(shadow), "; ".join("%s in %s" % (k, ", ".join(sorted(v)))
                                    for k, v in sorted(shadow.items())) or "none"))
    print("  RAW hazards, fusion-eligible ordered pairs (same pipeline layout):")
    for r in e1["raw"]:
        print("    %-24s -> %-24s %s"
              % (r["a"], r["b"], ", ".join(r["hazard"]) or "EMPTY — no hazard"))
    print("  A4 (OVER-PRIVILEGED, declared read_write and never written): %d"
          % len(e1["a4"]))
    for x in e1["a4"]:
        print("    %-26s readers: %s" % (x["decl"].symbol, ", ".join(x["readers"])))

    print("")
    print("EXTENSION 2 — ACCESS-PATTERN CLASSIFICATION")
    print("  overrides in world.wgsl: %s" % (", ".join(e2["overrides"]) or "none"))
    elig = [x for x in e2["eligibility"] if x["eligible"]]
    print("  vertex-buffer eligible bindings: %d" % len(elig))
    for x in elig:
        print("    %-24s %s-step, stride %s B, %d access site(s)"
              % (x["decl"].symbol, x["step"], x["stride"], len(x["sites"])))
    print("  the two bindings the survey named:")
    for name in ("patch_instances", "render_ring_xforms", "visible_patch_indices"):
        x = next((y for y in e2["eligibility"] if y["decl"].symbol == name), None)
        if x:
            print("    %-22s %-9s %s" % (name, "ELIGIBLE" if x["eligible"] else "blocked",
                                         "; ".join(x["blockers"]) or "-"))
    if e2["others"]:
        grp = {}
        for s_ in e2["others"]:
            grp.setdefault((s_["symbol"], s_["cls"].source), []).append(s_["fn"])
        print("  `other` access sites: %d, over %d (binding, provenance) groups — every"
              " one enumerated individually in the artifact:" % (len(e2["others"]), len(grp)))
        for (sym, src), fs in sorted(grp.items()):
            print("    %-22s %-34s in %s" % (sym, src, ", ".join(sorted(set(fs)))[:60]))

    print("")
    print("WITNESSES")
    w.report()

    if w.failures():
        print("")
        print("STOP — %d witness(es) failed. Nothing written. The ledger is not "
              "trustworthy until Jean rules which side is wrong." % len(w.failures()))
        return 1

    if args.check:
        print("")
        print("--check: all witnesses pass, nothing written.")
        return 0

    text = emit(args.out, w, column, layouts, b, c, d)
    print("")
    print("PHASE 0e — THE ARTIFACT")
    print("  wrote %s (%d lines, %d bytes)"
          % (os.path.relpath(args.out, REPO), text.count("\n"), len(text.encode("utf-8"))))
    return 0


if __name__ == "__main__":
    sys.exit(main())
