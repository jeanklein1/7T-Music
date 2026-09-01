#!/usr/bin/env python3
"""One-shot census: functions defined in src/ whose name appears nowhere
else in src/ outside comments — i.e. zero call sites."""
import os, re, sys
ROOT = "/home/user/7T-Music/src"
EXT = (".hpp", ".cpp", ".inc")
files = []
for d, _, fs in os.walk(ROOT):
    for f in fs:
        if f.endswith(EXT): files.append(os.path.join(d, f))

def strip_comments(t):
    t = re.sub(r"/\*.*?\*/", " ", t, flags=re.S)
    t = re.sub(r"//[^\n]*", " ", t)
    t = re.sub(r'"(\\.|[^"\\])*"', '""', t)
    return t

texts = {}
for f in files:
    texts[f] = strip_comments(open(f, encoding="utf-8", errors="replace").read())

DEF = re.compile(
    r"^\s*(?:template<[^\n]*>\s*)?(?:inline\s+|static\s+|constexpr\s+|virtual\s+|explicit\s+)*"
    r"(?:[A-Za-z_][\w:<>,\s\*&\[\]]*?[\s\*&])"
    r"([A-Za-z_]\w*)\s*\([^;{]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?\{",
    re.M)

KEYWORDS = {"if","for","while","switch","return","catch","sizeof","else","do","new","delete"}
defs = {}
for f, t in texts.items():
    for m in DEF.finditer(t):
        n = m.group(1)
        if n in KEYWORDS or n.startswith("operator"): continue
        defs.setdefault(n, []).append(f)

orphans = []
for n, where in sorted(defs.items()):
    uses = 0
    pat = re.compile(r"\b" + re.escape(n) + r"\b")
    for f, t in texts.items():
        uses += len(pat.findall(t))
    if uses <= len(where):            # only its own definition line(s)
        orphans.append((n, where))
print("DEFINITIONS SCANNED:", len(defs))
print("ZERO-CALL-SITE:", len(orphans))
for n, where in orphans:
    print("  %-42s %s" % (n, ", ".join(w.replace(ROOT+"/", "") for w in where)))
