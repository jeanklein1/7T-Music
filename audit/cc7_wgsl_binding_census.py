#!/usr/bin/env python3
"""[CC-7] WGSL @group/@binding literal census for world.wgsl.

Extracts every module-scope resource declaration `@group(G) @binding(B) var... name : type`,
counts the literals, reports duplicates (same (group,binding) used by more than one
declaration), and reports occupancy of specific slots of interest (g0:31, g1:34).

The design plan expects a binding_registry of C++ constants mirroring these
literals; at this tree's HEAD no such registry exists (see AUDIT_REPORT.md CC-7),
so the mirror is checked against the BINDING MAP comment in state.hpp instead.

Usage: python3 audit/cc7_wgsl_binding_census.py [path/to/world.wgsl]
"""
import re
import sys
import json
from collections import defaultdict

SRC = sys.argv[1] if len(sys.argv) > 1 else "src/cartridges/the_board/world.wgsl"
text = open(SRC, encoding="utf-8").read()

decl_re = re.compile(
    r"@group\((\d+)\)\s*@binding\((\d+)\)\s*"
    r"var\s*(?:<([^>]*)>)?\s*([A-Za-z_]\w*)\s*:\s*([^;]+);"
)

decls = []
for m in decl_re.finditer(text):
    g, b, aspace, name, typ = m.groups()
    line = text[: m.start()].count("\n") + 1
    decls.append({
        "group": int(g), "binding": int(b), "address_space": (aspace or "").strip(),
        "name": name, "type": " ".join(typ.split()), "line": line,
    })

# Sanity: count raw literals independently of the full-declaration regex.
raw_group_literals = len(re.findall(r"@group\(\d+\)", text))
raw_binding_literals = len(re.findall(r"@binding\(\d+\)", text))

by_slot = defaultdict(list)
for d in decls:
    by_slot[(d["group"], d["binding"])].append(d)

dups = {f"g{g}:b{b}": [f'{d["name"]} @ line {d["line"]} ({d["address_space"] or "handle"})'
                       for d in ds]
        for (g, b), ds in sorted(by_slot.items()) if len(ds) > 1}

groups = defaultdict(list)
for d in decls:
    groups[d["group"]].append(d)

slots_of_interest = {}
for slot in [(0, 31), (1, 34), (0, 34), (1, 31)]:
    ds = by_slot.get(slot, [])
    slots_of_interest[f"g{slot[0]}:b{slot[1]}"] = (
        [f'{d["name"]} @ line {d["line"]}' for d in ds] if ds else "FREE (no declaration)"
    )

out = {
    "source": SRC,
    "declaration_count": len(decls),
    "raw_group_literal_count": raw_group_literals,
    "raw_binding_literal_count": raw_binding_literals,
    "per_group_counts": {f"g{g}": len(ds) for g, ds in sorted(groups.items())},
    "slots_of_interest": slots_of_interest,
    "duplicate_slots": dups,
    "declarations": sorted(decls, key=lambda d: (d["group"], d["binding"], d["line"])),
}
print(json.dumps(out, indent=2))
