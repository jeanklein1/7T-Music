#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE SHELL GATE — the seam between the C++ organ and its panel, proved
# per commit. Both sides are parsed from the tree; neither is trusted.
# TOOTHED (L33, gates under tools/gates/): any violation exits nonzero.
#
# THE SIX CHECKS, each a FACT the C++ owns and the shell copies:
#   1. the shell names no block sentinel      — it asks the manifest instead
#   2. the type enum agrees                   — ORGAN_F32 … ORGAN_VEC4
#   3. the cadence table is the right length  — a fifth ORGAN_CAD_* fails here
#   4. the scope constants agree              — ORGAN_SCOPE_WORLD, the one
#                                               number the shell compares
#   5. RULE_NAMES is the right length         — the one ruled exception to
#                                               name-blindness, proven
#   6. every cwrap reaches a real export      — at the matching arity
# ═══════════════════════════════════════════════════════════════════════

# WHAT IT DOES NOT PROVE, said plainly rather than discovered later:

#   · IT READS TEXT, NOT BEHAVIOUR. Nothing is compiled, nothing is run,
#     no browser opens. A name that agrees can still be used wrongly.

#   · IT CANNOT SEE A VALUE PASSED WRONGLY. Check 6 counts parameters and
#     has no opinion about WHICH index or WHICH lane the shell puts in
#     them: organ_get(block, offset) and organ_get(index, lane) are both
#     two numbers.

#   · CHECK 1 READS A GRAMMAR, NOT AN INTEGER. It flags a sentinel value
#     that is BOUND (`= 255`, `[255, 254]`) or COMPARED (`=== 255`), and
#     not one reached by arithmetic or carried in a string — the price of
#     not firing on `f * 255` in the shell's hex-colour helpers.

#   · IT IS DELIBERATELY NARROW. It proves the SEAM, not the shell: the
#     panel's layout, its refresh, its export keying and every line of its
#     DOM are outside this gate's claim, and the web boot remains the
#     witness of record for all of it.
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
HEADER = os.path.join(ROOT, "src", "console", "organ_registry.hpp")
SHELL = os.path.join(ROOT, "web", "organ_panel.js")
ORBS = os.path.join(ROOT, "src", "cartridges", "the_board", "bodies", "orbs.hpp")


def read(path):
    with open(path, encoding="utf-8") as fh:
        return fh.read()


# ─── THE SHELL, WITH ITS PROSE REMOVED ────────────────────────────────
# Check 1 asks whether the shell NAMES a number, and a comment that
# discusses one is not the shell naming it. So comments, strings and regex
# literals come out first — a scanner rather than a regex, because
# `/[^a-z0-9]+/g` and `'it\'s'` both defeat the naive version.
def strip_js(src):
    out = []
    i, n = 0, len(src)
    prev = ""          # last significant character emitted
    while i < n:
        c = src[i]
        two = src[i:i + 2]
        if two == "//":
            j = src.find("\n", i)
            i = n if j < 0 else j
            continue
        if two == "/*":
            j = src.find("*/", i + 2)
            i = n if j < 0 else j + 2
            out.append(" ")
            continue
        if c in "'\"`":
            q, j = c, i + 1
            while j < n:
                if src[j] == "\\":
                    j += 2
                    continue
                if src[j] == q:
                    break
                j += 1
            i = j + 1
            out.append(" ")
            prev = "'"
            continue
        if c == "/" and (prev == "" or prev in "(,=:[!&|?{};+-*%~^<>"):
            # a regex literal, not a division
            j = i + 1
            cls = False
            while j < n:
                if src[j] == "\\":
                    j += 2
                    continue
                if src[j] == "[":
                    cls = True
                elif src[j] == "]":
                    cls = False
                elif src[j] == "/" and not cls:
                    break
                j += 1
            i = j + 1
            while i < n and src[i].isalpha():   # the flags
                i += 1
            out.append(" ")
            prev = "/"
            continue
        out.append(c)
        if not c.isspace():
            prev = c
        i += 1
    return "".join(out)


# ─── THE HEADER'S OWN NUMBERS ─────────────────────────────────────────
def enum_values(src, prefix):
    """Every `PREFIX_NAME = N` in the tree, as {name: int}.

    Two spellings, because the tree uses both: an enumerator (`= 255,`)
    and an `inline constexpr uint32_t ... = 0u;`. Reading only the first
    is how this gate's own check 5 reported zero ORB_RULE_* constants on
    its first run — a check that finds nothing and calls it agreement is
    the decoration this gate exists not to be.
    """
    out = {}
    for m in re.finditer(
            r"\b(" + prefix + r"\w*)\s*=\s*(\d+)[uU]?\s*[,;}]", src):
        out.setdefault(m.group(1), int(m.group(2)))
    return out


def keepalive_arity(src):
    """Every `EMSCRIPTEN_KEEPALIVE inline <ret> name(params)` → arity."""
    out = {}
    for m in re.finditer(
            r"EMSCRIPTEN_KEEPALIVE\s+inline\s+[\w:*&\s]+?\b(\w+)\s*\(", src):
        name = m.group(1)
        # walk to the matching close paren: a parameter list may wrap lines
        depth, j = 1, m.end()
        while j < len(src) and depth:
            if src[j] == "(":
                depth += 1
            elif src[j] == ")":
                depth -= 1
            j += 1
        params = src[m.end():j - 1].strip()
        if params in ("", "void"):
            out[name] = 0
        else:
            out[name] = len([p for p in params.split(",") if p.strip()])
    return out


def js_array_len(src, name):
    """The element count of `var NAME = [ … ];` in the shell."""
    m = re.search(r"\bvar\s+" + re.escape(name) + r"\s*=\s*\[", src)
    if not m:
        return None
    depth, j = 1, m.end()
    while j < len(src) and depth:
        if src[j] == "[":
            depth += 1
        elif src[j] == "]":
            depth -= 1
        j += 1
    body = src[m.end():j - 1].strip()
    if not body:
        return 0
    # top-level commas only — every table here is flat, but say so anyway
    depth, count = 0, 1
    for ch in body:
        if ch in "[({":
            depth += 1
        elif ch in "])}":
            depth -= 1
        elif ch == "," and depth == 0:
            count += 1
    return count


def main():
    for path in (HEADER, SHELL, ORBS):
        if not os.path.exists(path):
            print("shell-gate: FAIL — missing %s" % os.path.relpath(path, ROOT))
            return 1

    hdr = read(HEADER)
    shell_raw = read(SHELL)
    shell = strip_js(shell_raw)
    orbs = read(ORBS)

    violations = []

    def check(ok, num, claim, detail):
        print("  [%s] %d. %s — %s" % ("PASS" if ok else "FAIL", num, claim, detail))
        if not ok:
            violations.append("%d. %s — %s" % (num, claim, detail))

    print("── SHELL GATE: the seam between organ_registry.hpp "
          "and organ_panel.js ──")

    # ── 1. THE SHELL NAMES NO BLOCK SENTINEL ──────────────────────────
    # Every ORGAN_BLOCK_NONE* value is a C++ number, and the shell asks the
    # manifest (`inst`) rather than holding one. NAMING has a grammar: the
    # number is BOUND to something (`var DEFONLY = 255`, `[255, 254]`) or
    # COMPARED against something (`p.block === 255`).

    # Arithmetic is NOT naming: `f * 255` and `parseInt(...) / 255` are the
    # eight-bit colour maximum in this file's hex helpers, and a check that
    # read those as block sentinels would cry wolf on its first run. The
    # blind spot is the cost, and the header states it.
    sentinels = enum_values(hdr, "ORGAN_BLOCK_NONE")
    named = []
    for name, val in sorted(sentinels.items(), key=lambda kv: -kv[1]):
        hits = []
        for m in re.finditer(r"(?<![\w.$])" + str(val) + r"(?![\w.])", shell):
            before = shell[max(0, m.start() - 24):m.start()]
            after = shell[m.end():m.end() + 24]
            # BOUND: a lone `=` (assignment), or a container/argument
            # position. COMPARED: any relational or equality operator on
            # either side. The lone-`=` lookbehind separates
            # `var DEFONLY = 255` from `p.block === 255`; both are hits.
            CMP = r"(?:[=!]=+|[<>]=?)"
            bound = bool(re.search(r"(?<![=!<>])=\s*$", before)
                         or re.search(r"[\[,(]\s*$", before))
            compared = bool(re.search(CMP + r"\s*$", before)
                            or re.match(r"\s*" + CMP, after))
            if bound or compared:
                hits.append(shell.count("\n", 0, m.start()) + 1)
        if hits:
            named.append("%s (%d) at line(s) %s"
                         % (name, val, ", ".join(str(h) for h in hits)))
    check(not named and bool(sentinels), 1,
          "the shell names no block sentinel",
          "%d sentinel(s) %s; shell names %s"
          % (len(sentinels),
             sorted(sentinels.values(), reverse=True),
             "; ".join(named) if named else "none"))

    # ── 2. THE TYPE ENUM AGREES ───────────────────────────────────────
    types = {k[len("ORGAN_"):]: v for k, v in enum_values(hdr, "ORGAN_").items()
             if k in ("ORGAN_F32", "ORGAN_U32", "ORGAN_BOOL",
                      "ORGAN_VEC3", "ORGAN_VEC4")}
    js_types = {}
    m = re.search(r"\bvar\s+F32\s*=\s*\d+[^;]*;", shell)
    if m:
        for mm in re.finditer(r"\b(F32|U32|BOOL|VEC3|VEC4)\s*=\s*(\d+)", m.group(0)):
            js_types[mm.group(1)] = int(mm.group(2))
    bad = sorted(k for k in set(types) | set(js_types)
                 if types.get(k) != js_types.get(k))
    check(bool(types) and not bad, 2,
          "the type enum agrees",
          "C++ %s vs shell %s%s"
          % (sorted(types.items()), sorted(js_types.items()),
             "" if not bad else "; disagree on " + ", ".join(bad)))

    # ── 3. THE CADENCE TABLE IS THE RIGHT LENGTH ──────────────────────
    # A fifth cadence in C++ fails here rather than printing `undefined`
    # on a row nobody thought to look at.
    cads = enum_values(hdr, "ORGAN_CAD_")
    js_cad = js_array_len(shell, "CAD")
    check(bool(cads) and js_cad == len(cads), 3,
          "the cadence table is the right length",
          "%d ORGAN_CAD_* enumerator(s) vs CAD[%s]"
          % (len(cads), "absent" if js_cad is None else js_cad))

    # ── 4. THE SCOPE CONSTANTS AGREE ──────────────────────────────────
    # ORGAN_SCOPE_WORLD is the one number the shell still compares
    # against, so it is the one number that can drift.
    scopes = enum_values(hdr, "ORGAN_SCOPE_")
    world = scopes.get("ORGAN_SCOPE_WORLD")
    js_cmp = sorted(set(int(x) for x in
                        re.findall(r"\.scope\s*===\s*(\d+)", shell)))
    ok4 = world == 2 and bool(js_cmp) and all(v == world for v in js_cmp)
    check(ok4, 4,
          "the scope constants agree",
          "ORGAN_SCOPE_WORLD=%s; shell compares .scope === %s"
          % (world, js_cmp if js_cmp else "nothing"))

    # ── 5. RULE_NAMES IS THE RIGHT LENGTH ─────────────────────────────
    # THE ONE RULED EXCEPTION to name-blindness: four strings whose ORDER
    # is the contract with bodies/orbs.hpp and world.wgsl.
    rules = enum_values(orbs, "ORB_RULE_")
    js_rules = js_array_len(shell, "RULE_NAMES")
    check(bool(rules) and js_rules == len(rules), 5,
          "RULE_NAMES is the right length",
          "%d ORB_RULE_* constant(s) vs RULE_NAMES[%s]"
          % (len(rules), "absent" if js_rules is None else js_rules))

    # ── 6. EVERY CWRAP REACHES A REAL EXPORT, AT THE RIGHT ARITY ──────
    # A change made on one side only — an export losing a parameter while
    # the shell still spells the old count — fails here rather than in the
    # build.
    arity = keepalive_arity(hdr)
    wraps = []
    for m in re.finditer(
            r"cwrap\(\s*'(\w+)'\s*,\s*(?:'(\w+)'|null)\s*,\s*\[([^\]]*)\]\s*\)",
            shell_raw):
        args = [a for a in m.group(3).split(",") if a.strip()]
        wraps.append((m.group(1), len(args)))
    missing = [n for n, _ in wraps if n not in arity]
    mismatched = ["%s(shell %d, C++ %d)" % (n, k, arity[n])
                  for n, k in wraps if n in arity and arity[n] != k]
    check(bool(wraps) and not missing and not mismatched, 6,
          "every cwrap reaches a real export, at the right arity",
          "%d cwrap(s) checked against %d KEEPALIVE export(s)%s%s"
          % (len(wraps), len(arity),
             "; unreachable: " + ", ".join(missing) if missing else "",
             "; arity: " + ", ".join(mismatched) if mismatched else ""))

    print("──")
    if violations:
        for v in violations:
            print("FAIL:", v)
        print("SHELL GATE: RED — %d violation(s)" % len(violations))
        return 1
    print("SHELL GATE: GREEN — 0 violation(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
