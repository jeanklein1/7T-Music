#!/usr/bin/env python3
# **ARCHIVAL** — this document describes the tree as of its own
# date and is superseded. Do not cite as current. Live facts live
# in the code; adjudications live in `audit/` and
# `docs/HANDOFFS/OPTIMIZATION 1/`.

"""[CC-2c / CC-3 / CC-4] WGSL static-usage analyzer for world.wgsl.

Computes, for every entry point, the set of module-scope resource bindings it
*statically* references (directly or through the function call graph). This is
the same reachability notion Dawn/Tint uses when validating a pipeline against
its bind group layouts, so the output predicts exactly which pipelines fail
when a binding is removed from a layout ("compiler as witness", run statically).

Method:
  1. Parse module-scope resource declarations (@group/@binding var ...).
  2. Parse every `fn name(...)` with brace-matched bodies.
  3. Build the call graph (identifier references between function bodies,
     comments and strings stripped).
  4. For each entry point (@compute/@vertex/@fragment), take the transitive
     closure of callees and collect every resource var referenced.

Usage: python3 audit/cc4_wgsl_static_usage.py [world.wgsl] [--var NAME ...]
  --var NAME  also print the reverse view: every entry point reaching NAME.
"""
import re
import sys
import json

args = [a for a in sys.argv[1:] if not a.startswith("--")]
SRC = args[0] if args else "src/cartridges/the_board/world.wgsl"
want_vars = []
if "--var" in sys.argv:
    i = 0
    argv = sys.argv[1:]
    while i < len(argv):
        if argv[i] == "--var" and i + 1 < len(argv):
            want_vars.append(argv[i + 1])
            i += 2
        else:
            i += 1

raw = open(SRC, encoding="utf-8").read()

def strip_comments(s):
    s = re.sub(r"//[^\n]*", "", s)
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
    return s

text = strip_comments(raw)

# 1. Module-scope resource declarations.
decl_re = re.compile(
    r"@group\((\d+)\)\s*@binding\((\d+)\)\s*var\s*(?:<([^>]*)>)?\s*([A-Za-z_]\w*)\s*:"
)
resources = {}
for m in decl_re.finditer(text):
    g, b, aspace, name = m.groups()
    resources[name] = {"group": int(g), "binding": int(b),
                       "address_space": (aspace or "").strip()}

# 2. Functions with brace-matched bodies (on comment-stripped text).
fn_re = re.compile(r"(@compute[^\n]*|@vertex[^\n]*|@fragment[^\n]*)?\s*fn\s+([A-Za-z_]\w*)\s*\(")
functions = {}
for m in fn_re.finditer(text):
    stage_attr, name = m.groups()
    brace = text.find("{", m.end())
    if brace < 0:
        continue
    depth, i = 1, brace + 1
    while i < len(text) and depth:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    body = text[brace:i]
    stage = None
    if stage_attr:
        for s in ("compute", "vertex", "fragment"):
            if "@" + s in stage_attr:
                stage = s
    # line number in the ORIGINAL file
    line = raw[: raw.find("fn " + name + "(")].count("\n") + 1 if ("fn " + name + "(") in raw else None
    functions[name] = {"stage": stage, "body": body, "line": line}

ident_re = re.compile(r"[A-Za-z_]\w*")
local_decl_re = re.compile(r"\b(?:let|var|const)\s+([A-Za-z_]\w*)")

# 3. Direct references: called functions + resource vars per function.
# A `let/var/const NAME` inside the body declares a LOCAL that shadows any
# module-scope resource of the same name (e.g. `let patch_grid = ...` in
# generate_patch_cells vs the g0:152 patch_grid buffer), so shadowed names
# are excluded. Approximation: WGSL doesn't hoist, so a use *before* the
# local declaration would still hit module scope — no such case exists in
# world.wgsl at this HEAD, and the Dawn witness is the final authority.
calls, direct_res = {}, {}
fn_names = set(functions)
res_names = set(resources)
for name, f in functions.items():
    idents = set(ident_re.findall(f["body"]))
    shadowed = set(local_decl_re.findall(f["body"]))
    calls[name] = (idents & fn_names) - {name}
    direct_res[name] = (idents & res_names) - shadowed

# 4. Transitive closure per entry point.
def closure(root):
    seen, stack = set(), [root]
    while stack:
        n = stack.pop()
        if n in seen:
            continue
        seen.add(n)
        stack.extend(calls.get(n, ()))
    return seen

entry_points = {n: f for n, f in functions.items() if f["stage"]}
usage = {}
for name, f in sorted(entry_points.items(), key=lambda kv: kv[1]["line"] or 0):
    reach = closure(name)
    res = sorted(set().union(*(direct_res.get(fn, set()) for fn in reach)))
    usage[name] = {
        "stage": f["stage"], "line": f["line"],
        "bindings": [
            {"name": r, "group": resources[r]["group"], "binding": resources[r]["binding"]}
            for r in sorted(res, key=lambda r: (resources[r]["group"], resources[r]["binding"]))
        ],
    }

out = {"source": SRC, "entry_point_count": len(usage), "function_count": len(functions),
       "usage": usage}

if want_vars:
    reverse = {}
    for v in want_vars:
        readers = [ep for ep, u in usage.items() if any(b["name"] == v for b in u["bindings"])]
        direct = sorted(fn for fn, res in direct_res.items() if v in res)
        reverse[v] = {"entry_points_reaching": readers, "functions_referencing_directly": direct}
    out["reverse"] = reverse

print(json.dumps(out, indent=2))
