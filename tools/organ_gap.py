#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE ORGAN'S GAP (ORGAN_3 P3) — organ_gap.py
#
# WHAT THIS IS. A map of what the panel does NOT yet name. It parses the
# enrollment list (src/console/organ_params.inc) for every home struct
# the four macro forms address, brace-parses those structs out of the
# tree, and prints each declared member that no enrollment line mentions.
#
# CHECK-FAMILY, AND DELIBERATELY TOOTHLESS. stdout only, exit 0 ALWAYS.
# This is a map, not a gate: a member absent from the panel is usually
# absent ON PURPOSE (structural, driven, a pad, an RNG salt), and the
# reason lives in docs/ORGAN.md — "The disposition" for the class, and
# "What has no dial, and why" for the four facts that survived every wave
# of the survey. A tool that failed a build over an unenrolled field would
# be asserting a judgement it cannot make. binding_gen.py --check is a
# gate because the schema IS the authority there; here the DOC is the
# authority and this tool only reports.
#
# ITS TWO BLIND SPOTS, said plainly rather than discovered later:
#
#   1. IT CANNOT SEE HOMELESS CONSTANTS. A design parameter that lives as
#      an `inline constexpr` in a module and has no live home at all is
#      invisible here — there is no struct member for it to be missing
#      from. Those are the ledger's C2 rows, and the ledger is the only
#      instrument that finds them. This tool measures the gap between the
#      HOMES and the panel; the ledger measures the gap between the
#      PROGRAM and the panel, which is larger.
#
#   2. IT TRUSTS ITS OWN FILE TABLE. HOMES below maps each enrolled
#      struct to the file that declares it, by hand. A bank born without
#      a row here is simply not scanned, and the tool would report no gap
#      for it — silence that reads like health. So the tool PRINTS ITS
#      TABLE on every run: staleness is visible in the output rather than
#      hidden in the source. Update it when a bank is born.
#
#   3. IT REPORTS AT THE GRANULARITY THE ENROLLMENT ADDRESSES. A line
#      spelling `fog.gain` names the top-level member `fog`, so a bank
#      whose nested aggregate is PARTLY enrolled reads as fully named.
#      DriverSurface says 3/3 because fog, aura and checker each have at
#      least one dial — not because every field inside them does. The
#      same holds for `t[2].color_r` and `b[3].speed_cap`. Recursing
#      would be a bigger tool than the job wants; the ledger carries the
#      per-field truth for those five banks, and this line is here so the
#      count is never read as more than it is.
#
# THE READER WITNESS (ORGAN_3c P1) — the one thing here that CAN fail.
#
# A GRADUATION IS COMPLETE when the design table's only remaining readers
# are its own seed and its own asserts. ORGAN_3 w2 built PANEL_LIVE,
# enrolled it, and left the readers on the constexprs — seven dials that
# wrote a bank nothing read, found by hand a campaign later. That class of
# defect is mechanical to detect, so from ORGAN_3c it is detected
# mechanically: for every pair in PAIRS below, every word-boundary mention
# of the DESIGN symbol anywhere in src/ is classified, and anything that is
# not one of the lawful classes is a surviving runtime reader.
#
#   definition   the declaring statement itself
#   seed         a statement that also names the LIVE bank (the seeding)
#   static_assert  a proof about the authored table — its second job
#   comment      prose, including a mention inside a message string
#   constexpr    a COMPILE-TIME derivation (array sizing, a bound). Not in
#                the handoff's D3 list, and added here on the evidence:
#                gallery.hpp derives INDOOR_RADIUS_MAX from MOOD_TABLE's
#                structural rows. A constexpr context CANNOT read a mutable
#                inline bank — it is ill-formed, not merely awkward — and a
#                value fixed at compile time is one the panel could never
#                move, so calling it an incomplete graduation would be
#                false. D7 already rules this case for the canvas: the
#                constant stays on the design table. Printed always, never
#                silent, and never a gate failure.
#
# --gate turns the witness into a check the harness family runs. The MAP
# stays toothless (exit 0 always); only --gate can fail, and only on a
# surviving reader — a judgement the tree makes, not one this tool makes.
#
# USAGE
#   python3 tools/organ_gap.py            # the map
#   python3 tools/organ_gap.py --brief    # counts only
#   python3 tools/organ_gap.py --gate     # nonzero on a surviving reader
# ═══════════════════════════════════════════════════════════════════════

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INC  = os.path.join(ROOT, "src", "console", "organ_params.inc")

# ─── THE FILE TABLE (blind spot 2 — keep it current, it is printed) ────
# struct name -> path of the file that DECLARES it, relative to the repo.
HOMES = {
    "GPUDesignConfig":       "src/cartridges/the_board/realization/state.hpp",
    "GPULighting":           "src/cartridges/the_board/realization/state.hpp",
    "GPUAgentRoomConstants": "src/cartridges/the_board/realization/state.hpp",
    "DriverSurface":         "src/cartridges/the_board/contracts/driver_surface.hpp",
    "AgentTierBank":         "src/cartridges/the_board/contracts/agent_tiers.hpp",
    "AgentBehaviorBank":     "src/cartridges/the_board/contracts/agent_tiers.hpp",
    "MoodProfile":           "src/cartridges/the_board/contracts/spine_state.hpp",
    "PawnAuraProfile":       "src/cartridges/the_board/contracts/pawn_surface.hpp",
    "OrbConsole":            "src/cartridges/the_board/contracts/orb_surface.hpp",
    "OrbMoodConfig":         "src/cartridges/the_board/contracts/orb_surface.hpp",
    "PanelSurface":          "src/cartridges/the_board/contracts/control_panel.hpp",
    "RibbonSurface":         "src/cartridges/the_board/contracts/ribbon_surface.hpp",
    "IndoorSurface":         "src/cartridges/the_board/contracts/indoor_module.hpp",
    "CanvasSurface":         "src/coupling/canvas_surface.hpp",
    # ORGAN_4 P3d — the two destructive banks the Wave-3 row was holding.
    "WorldDrawSurface":      "src/cartridges/the_board/contracts/mood_constants.hpp",
    "RibbonSpawnSurface":    "src/cartridges/the_board/contracts/ribbon_surface.hpp",
}

# ─── THE GRADUATED PAIRS (ORGAN_3c — the reader witness) ──────────────
# home struct -> (DESIGN symbol, LIVE bank symbol). A home with no pair
# (the three GPU rooms) was never a graduation and has nothing to witness.
PAIRS = {
    "MoodProfile":       ("MOOD_TABLE",        "MOOD_LIVE"),
    "DriverSurface":     ("DRIVER_TABLE",      "DRIVER_LIVE"),
    "AgentTierBank":     ("AGENT_TIER_GAINS",  "TIER_LIVE"),
    "AgentBehaviorBank": ("AGENT_BEHAVIORS",   "BEHAVIOR_LIVE"),
    "PawnAuraProfile":   ("PAWN_AURA_DEFAULT", "PAWN_AURA_LIVE"),
    "OrbConsole":        ("ORB_CONSOLE",       "ORB_CONSOLE_LIVE"),
    "OrbMoodConfig":     ("ORB_MOOD_TABLE",    "ORB_MOOD_LIVE"),
    "PanelSurface":      ("PANEL_TABLE",       "PANEL_LIVE"),
    "RibbonSurface":     ("RIBBON_TABLE",      "RIBBON_LIVE"),
    "IndoorSurface":     ("INDOOR_TABLE",      "INDOOR_LIVE"),
    "CanvasSurface":     ("CANVAS_TABLE",      "CANVAS_LIVE"),
    "WorldDrawSurface":  ("WORLD_DRAW_TABLE",  "WORLD_DRAW_LIVE"),
    "RibbonSpawnSurface":("RIBBON_SPAWN_TABLE","RIBBON_SPAWN_LIVE"),
}
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
        return "constexpr"        # D7: a compile-time consumer keeps its source
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


# ─── THE ENROLLMENT LIST ──────────────────────────────────────────────
# The macro forms, and where the STRUCT and FIELD sit in each:
#   ORGAN_PARAM        (BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL)
#   ORGAN_PARAM_GEN    (the same, declared generational — ORGAN_3b)
#   ORGAN_PARAM_DEF    (BLOCK, STRUCT, FIELD, ... , DEFKIND, DEFSTRUCT, DEFFIELD)
#   ORGAN_PARAM_DEFONLY(TYPE, MIN, MAX, STEP, GROUP, LABEL, DEFKIND, DEFSTRUCT, DEFFIELD)
#   ORGAN_PARAM_RO     (BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)
#
# ORGAN_3b P2 gave every form an `_NS` twin whose FIRST argument is the
# enrolled struct's namespace. The two shapes are otherwise identical, so
# this parser reads the suffix and shifts its indices by one rather than
# carrying a second table — a namespace parameter that is invisible to the
# enrollment lines should be nearly invisible here too.
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
    # ─── THE READER WITNESS (ORGAN_3c) ───────────────────────────────
    census = reader_census()
    print()
    print("THE READER WITNESS — every mention of a DESIGN symbol, classified.")
    print("A graduation is complete when the design table's only readers are")
    print("its seed and its asserts. Anything else is a surviving runtime")
    print("reader, and the reason ORGAN_3 shipped seven dead dials.")
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
            print("        constexpr derivation (D7)  %s:%d  %s" % (path, ln, line[:60]))
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
