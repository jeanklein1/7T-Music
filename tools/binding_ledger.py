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
