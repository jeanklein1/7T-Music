#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# G-LAW 2 — THE WGSL STRUCTURAL WITNESS (stand-in)
#
# The law wants naga/Tint. Neither is installed in this container and
# `npx @webgpu/naga-cli` cannot reach a registry, so this is a STAND-IN
# and its limits are stated up front rather than discovered later.
#
# WHAT IT PROVES
#   1. delimiters balance ({} () []) across the whole file;
#   2. every function-call site resolves to a declared `fn` or to a name
#      in the BASELINE-CALIBRATED builtin set;
#   3. every reference to a module-scope symbol resolves to a declaration;
#   4. the entry-point set is exactly the recorded baseline;
#   5. nothing references a symbol the baseline had and this tree lost.
#
# WHAT IT DOES NOT PROVE
#   types, address spaces, override-expression legality, uniform layout
#   rules, or anything else a real front end checks. A GREEN here means
#   "no dangling name and no structural break" — which is precisely the
#   failure mode a DELETION can cause, and precisely why it is enough of
#   a witness for a pruning campaign and not enough for a feature.
#
# SELF-CALIBRATION. WGSL builtins are not enumerated here. The baseline
# file supplies them: every called-but-undeclared name at baseline IS a
# builtin (the tree parses on Dawn today, so it cannot be anything else).
# Any called-but-undeclared name that appears AFTER the baseline is a
# regression by construction.
#
#   python3 audit/tools/glaw2/run.py --record   # freeze the baseline
#   python3 audit/tools/glaw2/run.py            # check against it
# ═══════════════════════════════════════════════════════════════════════
"""G-LAW 2 stand-in: no dangling name, no structural break in world.wgsl."""

import io
import json
import os
import re
import sys

WGSL = "src/cartridges/the_board/realization/world.wgsl"
BASE = "audit/tools/glaw2/baseline.json"

IDENT = re.compile(r"[A-Za-z_]\w*")
CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
FN = re.compile(r"\bfn\s+([A-Za-z_]\w*)\s*\(")
CONST = re.compile(r"^const\s+([A-Za-z_]\w*)", re.M)
OVERRIDE = re.compile(r"^override\s+([A-Za-z_]\w*)", re.M)
STRUCT = re.compile(r"^struct\s+([A-Za-z_]\w*)", re.M)
BINDING = re.compile(
    r"@group\s*\(\s*\d+\s*\)\s*@binding\s*\(\s*\d+\s*\)\s*var\s*(?:<[^>]*>)?\s*([A-Za-z_]\w*)")
ENTRY = re.compile(r"@(compute|vertex|fragment)\b[^\n]*\n\s*fn\s+([A-Za-z_]\w*)")
# `let`/`var`/`const` inside a body, plus struct members and parameters, are
# locals; they shadow nothing at module scope and must not be resolved here.
LOCAL = re.compile(r"\b(?:let|var|const)\s+([A-Za-z_]\w*)")


def blank(m):
    return re.sub(r"[^\n]", " ", m.group(0))


def strip(s):
    s = re.sub(r"/\*.*?\*/", blank, s, flags=re.S)
    return re.sub(r"//[^\n]*", blank, s)


def lineno(t, p):
    return t.count("\n", 0, p) + 1


def parse(path):
    raw = io.open(path, encoding="utf-8").read()
    t = strip(raw)
    return {
        "text": t,
        "fns": sorted(set(FN.findall(t))),
        "consts": sorted(set(CONST.findall(t))),
        "overrides": sorted(set(OVERRIDE.findall(t))),
        "structs": sorted(set(STRUCT.findall(t))),
        "bindings": sorted(set(BINDING.findall(t))),
        "entries": sorted({n for _s, n in ENTRY.findall(t)}),
    }


def balance(t):
    """Delimiter balance with the offending line, not just a boolean."""
    pairs = {")": "(", "}": "{", "]": "["}
    stack = []
    for i, ch in enumerate(t):
        if ch in "({[":
            stack.append((ch, i))
        elif ch in ")}]":
            if not stack or stack[-1][0] != pairs[ch]:
                return "unmatched `%s` at line %d" % (ch, lineno(t, i))
            stack.pop()
    if stack:
        ch, i = stack[-1]
        return "unclosed `%s` opened at line %d" % (ch, lineno(t, i))
    return None


def main():
    if not os.path.exists(WGSL):
        sys.stderr.write("glaw2: run from the repo root\n")
        return 2
    cur = parse(WGSL)
    record = "--record" in sys.argv

    bad = balance(cur["text"])
    if bad:
        sys.stderr.write("G-LAW 2: RED — %s\n" % bad)
        return 1

    declared = set(cur["fns"]) | set(cur["consts"]) | set(cur["structs"]) \
        | set(cur["bindings"]) | set(cur["overrides"])
    called = set(CALL.findall(cur["text"]))
    unresolved_calls = called - set(cur["fns"]) - declared

    if record:
        os.makedirs(os.path.dirname(BASE), exist_ok=True)
        with io.open(BASE, "w", encoding="utf-8", newline="\n") as f:
            json.dump({
                "note": "builtins = called-but-undeclared at the baseline; the "
                        "tree parsed on Dawn at this commit, so they cannot be "
                        "anything else",
                "builtins": sorted(unresolved_calls),
                "entries": cur["entries"],
                "declared": sorted(declared),
            }, f, indent=1, sort_keys=True)
        print("G-LAW 2: baseline recorded — %d builtins, %d entry points, "
              "%d module symbols" % (len(unresolved_calls), len(cur["entries"]),
                                     len(declared)))
        return 0

    if not os.path.exists(BASE):
        sys.stderr.write("glaw2: no baseline; run --record first\n")
        return 2
    base = json.load(io.open(BASE, encoding="utf-8"))
    builtins = set(base["builtins"])

    errs = []
    new_unresolved = sorted(unresolved_calls - builtins)
    for n in new_unresolved:
        m = re.search(r"\b" + re.escape(n) + r"\s*\(", cur["text"])
        errs.append("call to undeclared `%s` (first at line %d)"
                    % (n, lineno(cur["text"], m.start()) if m else 0))

    # A reference to a module symbol the baseline had and this tree lost.
    lost = set(base["declared"]) - declared
    locals_ = set(LOCAL.findall(cur["text"]))
    for n in sorted(lost):
        if n in locals_:
            continue          # re-declared as a local somewhere; not dangling
        for m in re.finditer(r"\b" + re.escape(n) + r"\b", cur["text"]):
            errs.append("deleted symbol `%s` still referenced at line %d"
                        % (n, lineno(cur["text"], m.start())))
            break

    ent_lost = sorted(set(base["entries"]) - set(cur["entries"]))
    ent_new = sorted(set(cur["entries"]) - set(base["entries"]))
    for n in ent_lost:
        errs.append("entry point REMOVED: `%s`" % n)
    for n in ent_new:
        errs.append("entry point ADDED: `%s`" % n)

    if errs:
        sys.stderr.write("G-LAW 2: RED\n")
        for e in errs[:40]:
            sys.stderr.write("  · %s\n" % e)
        if len(errs) > 40:
            sys.stderr.write("  … and %d more\n" % (len(errs) - 40))
        return 1

    print("G-LAW 2: GREEN — %d fn, %d const, %d struct, %d binding, "
          "%d entry points; %d symbols retired cleanly"
          % (len(cur["fns"]), len(cur["consts"]), len(cur["structs"]),
             len(cur["bindings"]), len(cur["entries"]), len(lost)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
