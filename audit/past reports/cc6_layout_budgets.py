#!/usr/bin/env python3
"""[CC-6] Machine budget counts.

Parses every BindGroupLayout entry block inside state.hpp's createBindGroups()
(and anywhere else a `std::array<wgpu::BindGroupLayoutEntry, N>` block appears)
and counts, per layout label and per shader stage:
  storage buffers (Storage + ReadOnlyStorage), uniform buffers,
  sampled textures, storage textures, samplers.

Binding numbers may be raw integers or bind::gN::name constants; constants
are resolved from binding_registry.hpp (second CLI arg).

Crosses the counts against per-stage limits:
  - WebGPU default / older-Dawn law: 8 storage, 12 uniform per stage
  - world.wgsl banner FXC law:       10 storage, 12 uniform per stage

Usage: python3 audit/cc6_layout_budgets.py [state.hpp] [binding_registry.hpp]
"""
import re
import sys
import json

SRC = sys.argv[1] if len(sys.argv) > 1 else "src/cartridges/the_board/realization/state.hpp"
REG = sys.argv[2] if len(sys.argv) > 2 else "src/cartridges/the_board/realization/binding_registry.hpp"
text = open(SRC, encoding="utf-8").read()
lines = text.splitlines()

# Registry constants: namespace g0 { inline constexpr uint32_t name = N; } etc.
registry = {}
try:
    reg_text = open(REG, encoding="utf-8").read()
    for gm in re.finditer(r"namespace\s+(g\d)\s*\{(.*?)\n\s*\}", reg_text, re.S):
        gname, body = gm.group(1), gm.group(2)
        for cm in re.finditer(r"inline constexpr uint32_t\s+(\w+)\s*=\s*(\d+)\s*;", body):
            registry[f"bind::{gname}::{cm.group(1)}"] = int(cm.group(2))
except OSError:
    pass

def resolve_binding(expr):
    expr = expr.strip()
    if expr.isdigit():
        return int(expr)
    key = expr if expr.startswith("bind::") else "bind::" + expr.split("bind::")[-1]
    if key in registry:
        return registry[key]
    raise SystemExit(f"unresolvable binding expression: {expr!r}")

# Find each layout-entry block: from the std::array<wgpu::BindGroupLayoutEntry,..>
# declaration to the desc.label that names it.
block_re = re.compile(
    r"std::array<wgpu::BindGroupLayoutEntry,\s*(\d+)>\s*entries\{\};(.*?)"
    r'desc\.label\s*=\s*"([^"]+)"',
    re.S,
)

entry_re = re.compile(
    r"entries\[(\d+)\]\.binding\s*=\s*([^;]+);(.*?)(?=entries\[\d+\]\.binding|wgpu::BindGroupLayoutDescriptor)",
    re.S,
)

STAGES = ["Compute", "Vertex", "Fragment"]

def classify(chunk):
    if ".buffer.type" in chunk:
        m = re.search(r"BufferBindingType::(\w+)", chunk)
        t = m.group(1)
        if t in ("Storage", "ReadOnlyStorage"):
            return "storage_buffer", t
        if t == "Uniform":
            return "uniform_buffer", t
        return "other_buffer", t
    if ".storageTexture" in chunk:
        m = re.search(r"TextureFormat::(\w+)", chunk)
        return "storage_texture", m.group(1) if m else "?"
    if ".texture.sampleType" in chunk:
        m = re.search(r"TextureSampleType::(\w+)", chunk)
        return "sampled_texture", m.group(1) if m else "?"
    if ".sampler.type" in chunk:
        m = re.search(r"SamplerBindingType::(\w+)", chunk)
        return "sampler", m.group(1) if m else "?"
    return "UNCLASSIFIED", "?"

def view_dimension(chunk):
    m = re.search(r"TextureViewDimension::e(\w+)", chunk)
    if not m:
        return None  # WebGPU default: "2d"
    return {"1D": "1d", "2D": "2d", "2DArray": "2d-array", "3D": "3d",
            "Cube": "cube", "CubeArray": "cube-array"}.get(m.group(1), m.group(1))

def stages_of(chunk):
    m = re.search(r"visibility\s*=\s*([^;]+);", chunk)
    vis = m.group(1) if m else ""
    return [s for s in STAGES if s in vis]

CLASSES = ["storage_buffer", "uniform_buffer", "sampled_texture", "storage_texture", "sampler"]
report = []
for m in block_re.finditer(text):
    declared_n, body, label = int(m.group(1)), m.group(2), m.group(3)
    line_no = text[: m.start()].count("\n") + 1
    entries = []
    counts = {s: {c: 0 for c in CLASSES} for s in STAGES}
    for em in entry_re.finditer(body):
        idx, binding, chunk = int(em.group(1)), resolve_binding(em.group(2)), em.group(3)
        klass, detail = classify(chunk)
        st = stages_of(chunk)
        for s in st:
            if klass in counts[s]:
                counts[s][klass] += 1
        entries.append({"index": idx, "binding": binding, "class": klass,
                        "detail": detail, "stages": st,
                        "viewDimension": view_dimension(chunk)})
    report.append({
        "label": label,
        "state_hpp_line": line_no,
        "declared_entry_count": declared_n,
        "parsed_entry_count": len(entries),
        "entries": entries,
        "per_stage_counts": counts,
    })

# Cross against the two candidate laws.
LAWS = {"webgpu_default(8s/12u)": (8, 12), "plan_banner(10s/12u)": (10, 12)}
flags = []
for lay in report:
    for stage, c in lay["per_stage_counts"].items():
        for law, (smax, umax) in LAWS.items():
            if c["storage_buffer"] > smax:
                flags.append(f'{lay["label"]} [{stage}]: {c["storage_buffer"]} storage buffers > {smax} ({law})')
            if c["uniform_buffer"] > umax:
                flags.append(f'{lay["label"]} [{stage}]: {c["uniform_buffer"]} uniform buffers > {umax} ({law})')
    if lay["declared_entry_count"] != lay["parsed_entry_count"]:
        flags.append(f'{lay["label"]}: declared {lay["declared_entry_count"]} entries, parser saw {lay["parsed_entry_count"]} — PARSER GAP')

print(json.dumps({"source": SRC, "layouts": report, "flags": sorted(set(flags))}, indent=2))
