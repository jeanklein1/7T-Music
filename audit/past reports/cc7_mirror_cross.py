#!/usr/bin/env python3
# **ARCHIVAL** — this document describes the tree as of its own
# date and is superseded. Do not cite as current. Live facts live
# in the code; adjudications live in `audit/` and
# `docs/HANDOFFS/OPTIMIZATION 1/`.

"""[CC-7] C6 mirror check: world.wgsl @group/@binding literals vs the
binding_registry.hpp constants (the real registry at CLOSURE_GPU HEAD).

For every WGSL resource declaration and every registry constant, matches by
(group, number) and by name, reporting:
  - WGSL literals with NO registry constant at that (group, number)
  - registry constants with NO WGSL literal at that (group, number)
  - name mismatches at matched numbers (documented aliases like
    vp_data // aka fc_vp surface here, deliberately)
Also reports occupancy of the proposed slots g0:31 and g1:34.

Usage: python3 audit/cc7_mirror_cross.py  (expects cc7_output.json beside it)
"""
import json
import os
import re

here = os.path.dirname(os.path.abspath(__file__))
REG = os.path.join(here, "..", "src", "cartridges", "the_board",
                   "realization", "binding_registry.hpp")
cc7 = json.load(open(os.path.join(here, "cc7_output.json")))

reg_text = open(REG, encoding="utf-8").read()
registry = {}  # (group, number) -> [names]
for gm in re.finditer(r"namespace\s+g(\d)\s*\{(.*?)\n\s*\}", reg_text, re.S):
    g, body = int(gm.group(1)), gm.group(2)
    for cm in re.finditer(r"inline constexpr uint32_t\s+(\w+)\s*=\s*(\d+)\s*;", body):
        registry.setdefault((g, int(cm.group(2))), []).append(cm.group(1))

wgsl = {}  # (group, number) -> [names]
for d in cc7["declarations"]:
    wgsl.setdefault((d["group"], d["binding"]), []).append(d["name"])

all_slots = sorted(set(registry) | set(wgsl))
lit_no_reg, reg_no_lit, name_mismatch, clean = [], [], [], 0
for slot in all_slots:
    g, b = slot
    w, r = wgsl.get(slot), registry.get(slot)
    if w and not r:
        lit_no_reg.append(f"g{g}:b{b} wgsl={w}")
    elif r and not w:
        reg_no_lit.append(f"g{g}:b{b} registry={r}")
    else:
        if set(w) == set(r):
            clean += 1
        else:
            name_mismatch.append(f"g{g}:b{b} wgsl={w} registry={r}")

out = {
    "registry_path": os.path.relpath(REG, os.path.join(here, "..")),
    "registry_constant_count": sum(len(v) for v in registry.values()),
    "registry_slot_count": len(registry),
    "wgsl_declaration_count": sum(len(v) for v in wgsl.values()),
    "wgsl_slot_count": len(wgsl),
    "slots_matching_by_number_and_name": clean,
    "wgsl_literal_without_registry_constant": lit_no_reg,
    "registry_constant_without_wgsl_literal": reg_no_lit,
    "name_mismatches_at_matched_numbers": name_mismatch,
    "proposed_slots": {
        "g0:b31": {"wgsl": wgsl.get((0, 31), "FREE"), "registry": registry.get((0, 31), "FREE")},
        "g1:b34": {"wgsl": wgsl.get((1, 34), "FREE"), "registry": registry.get((1, 34), "FREE")},
    },
}
print(json.dumps(out, indent=2))
