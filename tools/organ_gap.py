#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE ORGAN'S GAP — organ_gap.py
#
# A map of what the panel does NOT yet name: it parses the enrollment
# list (src/console/organ_params.inc) for every home struct the macro
# forms address, brace-parses those structs out of the tree, and prints
# each declared member that no enrollment line mentions.
#
# EXIT 0 ALWAYS UNLESS --gate. A member absent from the panel is usually
# absent ON PURPOSE and the reason lives in docs/ORGAN.md, so failing a
# build over one would assert a judgement this tool cannot make.
#
# USAGE   organ_gap.py · --brief for counts · --gate to bite
# ═══════════════════════════════════════════════════════════════════════

# THE THREE BLIND SPOTS, said plainly rather than discovered later:
#   1. IT CANNOT SEE HOMELESS CONSTANTS — an `inline constexpr` with no
#      live home has no struct member to be missing from. This measures
#      the gap between the HOMES and the panel; the ledger measures the
#      gap between the PROGRAM and the panel, which is larger.

#   2. IT TRUSTS ITS OWN FILE TABLE. HOMES below maps each enrolled struct
#      to its declaring file by hand, and a bank with no row there is not
#      scanned — silence that reads like health. So the tool PRINTS ITS
#      TABLE on every run. Update it when a bank is born.

#   3. IT REPORTS AT THE GRANULARITY THE ENROLLMENT ADDRESSES. A line
#      spelling `fog.gain` names the top-level member `fog`, so a partly
#      enrolled nested aggregate reads as fully named. The ledger carries
#      the per-field truth.

# THE READER WITNESS — the one thing here that CAN fail. A GRADUATION IS
# COMPLETE when the design table's only remaining readers are its seed and
# its asserts. Every word-boundary mention of a DESIGN symbol in src/ is
# classified, and anything outside the five lawful classes below is a
# surviving runtime reader. --gate is nonzero on one.

#   definition     the declaring statement itself
#   seed           a statement that also names the LIVE bank
#   static_assert  a proof about the authored table — its second job
#   comment        prose, including a mention inside a message string
#   constexpr      a compile-time derivation: a constexpr context cannot
#                  read a mutable bank, so the constant stays on the table

import os
import re
import sys

# tools/ is sys.path[0] when a tool runs as a script; the insert is for the
# ledger's subprocess runs and for any caller importing a tool from elsewhere.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from organ_parse import MACRO, split_args

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INC  = os.path.join(ROOT, "src", "console", "organ_params.inc")

# ─── THE FILE TABLE (blind spot 2 — keep it current, it is printed) ────
# struct name -> path of the file that DECLARES it, relative to the repo.
HOMES = {
    "GPUDesignConfig":       "src/cartridges/the_board/realization/state.hpp",
    "GPULighting":           "src/cartridges/the_board/realization/state.hpp",
    "GPUAgentRoomConstants": "src/cartridges/the_board/realization/state.hpp",
    "DriverSurface":         "src/cartridges/the_board/contracts/driver_surface.hpp",
    "AtmosphereBank":        "src/cartridges/the_board/contracts/atmosphere_surface.hpp",
    # THE MAP COULD NOT SEE THE GROUND (THE_PANEL I U0). AutomatonBank was
    # born at ONE_SURFACE-II U1, after this table was last written, and a
    # home absent from the table is a gap that reads as zero — blind spot
    # 2's exact failure, which is why that blind spot prints the table it
    # trusted. Not one of its fields is enrolled, so the row it adds is
    # the largest single absence the tool reports.
    "AutomatonBank":         "src/cartridges/the_board/contracts/automaton_surface.hpp",
    "AgentPopulationBank":   "src/cartridges/the_board/contracts/agent_surface.hpp",
    "AgentTierBank":         "src/cartridges/the_board/contracts/agent_tiers.hpp",
    "AgentBehaviorBank":     "src/cartridges/the_board/contracts/agent_tiers.hpp",
    "PawnAuraProfile":       "src/cartridges/the_board/contracts/pawn_surface.hpp",
    "OrbConsole":            "src/cartridges/the_board/contracts/orb_surface.hpp",
    "OrbConfig":         "src/cartridges/the_board/contracts/orb_surface.hpp",
    "PanelSurface":          "src/cartridges/the_board/contracts/control_panel.hpp",
    "RibbonSurface":         "src/cartridges/the_board/contracts/ribbon_surface.hpp",
    "CanvasSurface":         "src/coupling/canvas_surface.hpp",
    "CubeBank":              "src/cartridges/the_board/bodies/cube_behaviors.hpp",
    # the destructive bank
    "RibbonSpawnSurface":    "src/cartridges/the_board/contracts/ribbon_surface.hpp",
}
# MoodProfile, IndoorSurface and WorldDrawSurface stood here with their
# homes; all three left with their subjects at ONE_WORLD-II U2 and U4,
# and their rows printed STRUCT NOT FOUND — blind spot 2 doing its job —
# until U8 cut them. CubeBank is a HOME with NO ENROLLED ROW: the bank
# rose at U1c and cannot be enrolled while it lives in a body, because
# the organ may not include one (L38). It is listed so the reader witness
# can see it; the `*` in the file table is the honest mark for it.

# ─── THE GRADUATED PAIRS (the reader witness) ─────────────────────────
# home struct -> (DESIGN symbol, LIVE bank symbol). A home with no pair
# (the three GPU rooms) was never a graduation and has nothing to witness.
PAIRS = {
    "DriverSurface":     ("DRIVER_TABLE",      "DRIVER_LIVE"),
    "AtmosphereBank":    ("ATMOS_TABLE",       "ATMOS_LIVE"),
    "AutomatonBank":     ("AUTO_TABLE",        "AUTO_LIVE"),
    "AgentPopulationBank": ("AGENTS_TABLE",    "AGENTS_LIVE"),
    "CubeBank":          ("CUBE_TABLE",        "CUBE_LIVE"),
    "AgentTierBank":     ("AGENT_TIER_GAINS",  "TIER_LIVE"),
    "AgentBehaviorBank": ("AGENT_BEHAVIORS",   "BEHAVIOR_LIVE"),
    "PawnAuraProfile":   ("PAWN_AURA_DEFAULT", "PAWN_AURA_LIVE"),
    "OrbConsole":        ("ORB_CONSOLE",       "ORB_CONSOLE_LIVE"),
    "OrbConfig":     ("ORB_TABLE",         "ORB_LIVE"),
    "PanelSurface":      ("PANEL_TABLE",       "PANEL_LIVE"),
    "RibbonSurface":     ("RIBBON_TABLE",      "RIBBON_LIVE"),
    "CanvasSurface":     ("CANVAS_TABLE",      "CANVAS_LIVE"),
    "RibbonSpawnSurface":("RIBBON_SPAWN_TABLE","RIBBON_SPAWN_LIVE"),
}
# THE FOUR BANKS ONE_WORLD-II BUILT are witnessed here for the first
# time at U8. AtmosphereBank and AgentPopulationBank were HOMES with no
# PAIR — the tool called them "(not a graduation)" when each is exactly
# one — and OrbConfig's pair named ORB_MOOD_TABLE, the seven-row
# table U2 took, rather than ORB_TABLE, the design row that seeds
# ORB_LIVE today. A pair naming a symbol that does not exist witnesses
# nothing and reports zero readers for it, which reads as a pass.
SRC_EXT = (".hpp", ".cpp", ".inc", ".h")


def mask(text):
    """Blank comments and string bodies, and say what each byte is.

    Returns (code, kind): `code` is the text with comment and string-literal
    bytes replaced by spaces so a regex sees only real code; kind[i] is one
    of 'code' / 'comment' / 'string', so a hit inside prose can be named as
    prose rather than merely ignored.
    """
    out, kind = [], []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        two = text[i:i + 2]
        if two == "//":
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i)); kind.extend("comment" for _ in range(j - i))
            i = j
        elif two == "/*":
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(" " * (j - i)); kind.extend("comment" for _ in range(j - i))
            i = j
        elif c in "\"'":
            q, j = c, i + 1
            while j < n and text[j] != q:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + " " * (j - i - 2) + (q if j - i >= 2 else ""))
            kind.append("code"); kind.extend("string" for _ in range(j - i - 2))
            if j - i >= 2:
                kind.append("code")
            i = j
        else:
            out.append(c); kind.append("code")
            i += 1
    code = "".join(out)
    # a defensive equalisation: the classifier indexes both by the same i
    if len(code) != n:
        code = (code + " " * n)[:n]
    if len(kind) != n:
        kind = (kind + ["code"] * n)[:n]
    return code, kind


def statement_of(code, at):
    """The declaration a hit sits inside: back to the previous CODE `;`,
    forward to the next one. Deliberately not a parser — an initializer's
    braces and commas carry no semicolons in this tree, so this reaches the
    whole `inline ... = { ... };` a seeding line lives in, which is the one
    thing a cheaper rule would get wrong."""
    a = code.rfind(";", 0, at) + 1
    b = code.find(";", at)
    return code[a:(len(code) if b < 0 else b + 1)]


def classify(code, kind, at, design, live):
    if kind[at] == "comment":
        return "comment"
    if kind[at] == "string":
        return "comment"          # a message is prose that happens to compile
    stmt = statement_of(code, at)
    if "static_assert" in stmt:
        return "static_assert"
    if re.search(r"\b" + re.escape(live) + r"\b", stmt):
        return "seed"
    if re.search(r"constexpr\b[^;=]*\b" + re.escape(design) + r"\s*(\[[^\]]*\])?\s*=", stmt):
        return "definition"
    if re.search(r"\bconstexpr\b", stmt):
        return "constexpr"        # a compile-time consumer keeps its source
    return "violation"


def reader_census():
    """{home: {class: [(path, line, text)]}} for every graduated pair."""
    found = {h: {} for h in PAIRS}
    for base, _dirs, files in os.walk(os.path.join(ROOT, "src")):
        for f in files:
            if not f.endswith(SRC_EXT):
                continue
            path = os.path.join(base, f)
            try:
                text = open(path, encoding="utf-8", errors="replace").read()
            except OSError:
                continue
            code, kind = None, None
            for home, (design, live) in PAIRS.items():
                if design not in text:
                    continue
                if code is None:
                    code, kind = mask(text)
                for m in re.finditer(r"\b" + re.escape(design) + r"\b", text):
                    cls = classify(code, kind, m.start(), design, live)
                    ln = text.count("\n", 0, m.start()) + 1
                    rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
                    line = text.split("\n")[ln - 1].strip()
                    found[home].setdefault(cls, []).append((rel, ln, line))
    return found


# ─── THE ENROLLMENT LIST — the macro forms, and where STRUCT and FIELD sit
#   ORGAN_PARAM     (BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
#   _GEN            the same, declared generational
#   _DEF            the same, + DEFKIND, DEFSTRUCT, DEFFIELD
#   _DEFONLY        (TYPE, MIN, MAX, STEP, GROUP, LABEL, DEFKIND, DEFSTRUCT, DEFFIELD)
#   _RO             (BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)

# Every form has an `_NS` twin whose FIRST argument is the enrolled
# struct's namespace. The two shapes are otherwise identical, so this
# parser reads the suffix and shifts its indices by one rather than
# carrying a second table.


def base_member(field):
    """`tier_gains[2].color_r` -> `tier_gains`; `fog.gain` -> `fog`."""
    return re.split(r"[\[.]", field.strip(), 1)[0]


def enrolled_pairs():
    """{struct: set(top-level member names named by any macro form)}"""
    seen = {}
    with open(INC, encoding="utf-8") as f:
        for raw in f:
            m = MACRO.match(raw.strip())
            if not m:
                continue
            form, ns, args = m.group(1), m.group(2), split_args(m.group(3))
            k = 1 if ns else 0          # the _NS forms shift everything by one
            pairs = []
            if form in ("ORGAN_PARAM", "ORGAN_PARAM_GEN", "ORGAN_PARAM_RO") \
                    and len(args) >= 3 + k:
                pairs.append((args[1 + k], args[2 + k]))
            elif form == "ORGAN_PARAM_DEF" and len(args) >= 12 + k:
                pairs.append((args[1 + k], args[2 + k]))    # the instance
                pairs.append((args[10 + k], args[11 + k]))  # the definition
            elif form == "ORGAN_PARAM_DEFONLY" and len(args) >= 9 + k:
                pairs.append((args[7 + k], args[8 + k]))
            for st, fl in pairs:
                seen.setdefault(st, set()).add(base_member(fl))
    return seen


# ─── THE STRUCTS ──────────────────────────────────────────────────────
DECL = re.compile(r"\bstruct\s+(?:alignas\s*\([^)]*\)\s*)?(\w+)\s*\{")
# one declaration line: <type> <name>[...] ... ;   (no parens = not a function)
MEMBER = re.compile(r"^\s*(?:inline\s+|static\s+|constexpr\s+|mutable\s+)*"
                    r"(?:struct\s+|class\s+|enum\s+(?:class\s+)?|unsigned\s+|signed\s+)?"
                    r"[A-Za-z_][\w:]*\s*(?:<[^;]*>)?\s*[*&]?\s*"
                    r"([A-Za-z_]\w*)\s*(?:\[[^\]]*\])*\s*(?:=[^;]*)?;\s*$")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def struct_body(path, name):
    """Brace-aware extraction of `struct name { ... }` from a file."""
    try:
        with open(path, encoding="utf-8") as f:
            src = f.read()
    except OSError:
        return None
    for m in DECL.finditer(src):
        if m.group(1) != name:
            continue
        i, depth = m.end() - 1, 0
        for j in range(i, len(src)):
            if src[j] == "{":
                depth += 1
            elif src[j] == "}":
                depth -= 1
                if depth == 0:
                    return src[i + 1:j]
    return None


def members_of(body):
    """Top-level declared members, in order, skipping pads and nested bodies.

    A nested `struct { ... } name;` contributes `name` (its own members are
    reached through it, and the enrollment spells them that way)."""
    body = strip_comments(body)
    out, depth, buf = [], 0, []
    i = 0
    while i < len(body):
        ch = body[i]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                # `} name;` — the nested aggregate's own member name
                tail = body[i + 1:body.find(";", i) + 1] if ";" in body[i:] else ""
                nm = re.match(r"\s*(\w+)\s*(?:\[[^\]]*\])*\s*;", tail)
                if nm:
                    out.append(nm.group(1))
                    i = body.find(";", i)
                buf = []
                i += 1
                continue
        if depth == 0:
            buf.append(ch)
            if ch == ";":
                line = "".join(buf)
                mm = MEMBER.match(" " + line.strip())
                if mm:
                    out.append(mm.group(1))
                buf = []
        i += 1
    return [m for m in out if not m.startswith("_pad") and not m.startswith("_")]


def main():
    brief = "--brief" in sys.argv
    gate  = "--gate" in sys.argv
    enrolled = enrolled_pairs()

    print("ORGAN GAP — members of the enrolled homes that the panel does not name")
    print("=" * 72)
    print("A map, not a gate. Reasons live in docs/ORGAN.md.")
    print()
    print("THE FILE TABLE this run trusted (blind spot 2 — stale rows are")
    print("invisible bugs, so they are printed):")
    for st in sorted(HOMES):
        mark = " " if st in enrolled else "*"   # * = enrolled by nothing
        pair = PAIRS.get(st)
        print("  %s %-24s %-52s %s" % (mark, st, HOMES[st],
                                       ("%s -> %s" % pair) if pair else "(not a graduation)"))
    unlisted = sorted(set(enrolled) - set(HOMES))
    if unlisted:
        print()
        print("  ENROLLED BUT NOT IN THE TABLE — not scanned, so not reported:")
        for st in unlisted:
            print("    %s" % st)
    print()

    total_gap = 0
    for st in sorted(HOMES):
        body = struct_body(os.path.join(ROOT, HOMES[st]), st)
        if body is None:
            print("%-24s  STRUCT NOT FOUND in %s" % (st, HOMES[st]))
            continue
        decl = members_of(body)
        named = enrolled.get(st, set())
        gap = [m for m in decl if m not in named]
        total_gap += len(gap)
        print("%-24s  %2d/%2d named   %d absent"
              % (st, len(decl) - len(gap), len(decl), len(gap)))
        if gap and not brief:
            for m in gap:
                print("        %s" % m)
    print()
    print("TOTAL ABSENT FROM THE PANEL, ACROSS THE ENROLLED HOMES: %d" % total_gap)
    print()
    print("Blind spot 1: homeless constants — an authored constexpr with no")
    print("live home — cannot appear above. This tool measures the gap between")
    print("the HOMES and the panel; the ledger measures the gap between the")
    print("PROGRAM and the panel, which is larger.")
    # ─── THE READER WITNESS ──────────────────────────────────────────
    census = reader_census()
    print()
    print("THE READER WITNESS — every mention of a DESIGN symbol, classified.")
    print("A graduation is complete when the design table's only readers are")
    print("its seed and its asserts. Anything else is a surviving runtime")
    print("reader — the class of defect this witness exists to catch.")
    print()
    ORDER = ["definition", "seed", "static_assert", "constexpr", "comment", "violation"]
    violations = []
    for home in sorted(PAIRS):
        design, live = PAIRS[home]
        hits = census[home]
        counts = " ".join("%s=%d" % (c, len(hits[c])) for c in ORDER if hits.get(c))
        flag = "  <-- SURVIVING READER" if hits.get("violation") else ""
        print("  %-20s %-42s %s%s" % (design, counts or "(no mention anywhere)",
                                      "", flag))
        for path, ln, line in hits.get("constexpr", []):
            print("        constexpr derivation  %s:%d  %s" % (path, ln, line[:60]))
        for path, ln, line in hits.get("violation", []):
            print("        VIOLATION  %s:%d  %s" % (path, ln, line[:64]))
            violations.append((design, path, ln, line))
    print()
    print("SURVIVING RUNTIME READERS ACROSS %d GRADUATED PAIRS: %d"
          % (len(PAIRS), len(violations)))
    if gate:
        print("--gate: %s" % ("FAIL" if violations else "PASS"))
    print()
    print("Blind spot 3: a partly-enrolled nested aggregate reads as named —")
    print("`fog.gain` names `fog`. The ledger carries the per-field truth.")
    # THE MAP IS STILL TOOTHLESS. Only --gate can fail, and only on a
    # surviving reader — never on an unenrolled member, which is a
    # judgement the ledger makes and this tool may not.
    return 1 if (gate and violations) else 0


if __name__ == "__main__":
    sys.exit(main())
