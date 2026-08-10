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
    __slots__ = ("name", "stage", "workgroup_size", "body", "start", "line", "calls", "refs")

    def __init__(self, **kw):
        for k in self.__slots__:
            setattr(self, k, kw.get(k))


STAGE_OF_ATTR = {"vertex": "V", "fragment": "F", "compute": "C"}


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
        fns[name] = WgslFn(name=name, stage=stage,
                           workgroup_size=("(%s)" % " ".join(wg.group(1).split())) if wg else "",
                           body=body, start=brace, line=line_of(src, m.start()),
                           calls=set(), refs=set())

    # Reference extraction. An identifier preceded by `.` is member
    # access, not a module-scope name; the `fn` keyword itself is a
    # definition, not a call. Everything else that matches a function
    # name or a binding symbol counts as a reference. The direction of
    # any residual error is OVER-approximation (a shadowing local named
    # after a binding would add a reference, never remove one), which
    # makes vis_actual generous and every A1 flag conservative.
    ident = re.compile(r"(?<![\w.])([A-Za-z_]\w*)")
    for fn in fns.values():
        for im in ident.finditer(fn.body):
            tok = im.group(1)
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


def phase_0b(w):
    raw = read(WORLD_WGSL)
    src = strip_wgsl_comments(raw)
    consts = parse_wgsl_consts(src)
    structs = parse_wgsl_structs(src)
    decls, slots = parse_wgsl_decls(w, src, structs, consts)
    fns, entries = parse_wgsl_functions(w, src, {d.symbol for d in decls})
    reach = reachability(fns, entries)
    return {"src": src, "consts": consts, "structs": structs, "decls": decls,
            "slots": slots, "fns": fns, "entries": entries, "reach": reach}


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

    print("")
    print("WITNESSES")
    w.report()

    if w.failures():
        print("")
        print("STOP — %d witness(es) failed. The ledger is not trustworthy until "
              "Jean rules which side is wrong." % len(w.failures()))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
