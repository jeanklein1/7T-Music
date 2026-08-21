#!/usr/bin/env python3
# The enrollment grammar of src/console/organ_params.inc, parsed once.
# organ_gap.py, organ_readers.py and organ_ledger.py import from here; each keeps
# its own row extraction because each wants different columns of the same line.
import re

# ORGAN_PARAM / _GEN / _DEF / _DEFONLY / _RO, each with an optional _NS twin whose
# first argument is the struct's namespace.
MACRO = re.compile(
    r"^(ORGAN_PARAM(?:_GEN|_DEF|_DEFONLY|_RO)?)(_NS)?\s*\((.*)\)\s*$")


def split_args(s):
    """Top-level comma split — brackets and quotes are not separators."""
    out, depth, quo, cur = [], 0, False, []
    for ch in s:
        if quo:
            cur.append(ch)
            if ch == '"':
                quo = False
            continue
        if ch == '"':
            quo = True; cur.append(ch); continue
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            out.append("".join(cur).strip()); cur = []
        else:
            cur.append(ch)
    out.append("".join(cur).strip())
    return out
