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
# reason lives in docs/HANDOFFS/DISPOSITION_LEDGER.md. A tool that failed
# a build over an unenrolled field would be asserting a judgement it
# cannot make. binding_gen.py --check is a gate because the schema IS the
# authority there; here the LEDGER is the authority and this tool only
# reports.
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
# USAGE
#   python3 tools/organ_gap.py            # the map
#   python3 tools/organ_gap.py --brief    # counts only
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
}

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
    enrolled = enrolled_pairs()

    print("ORGAN GAP — members of the enrolled homes that the panel does not name")
    print("=" * 72)
    print("A map, not a gate. Reasons live in docs/HANDOFFS/DISPOSITION_LEDGER.md.")
    print()
    print("THE FILE TABLE this run trusted (blind spot 2 — stale rows are")
    print("invisible bugs, so they are printed):")
    for st in sorted(HOMES):
        mark = " " if st in enrolled else "*"   # * = enrolled by nothing
        print("  %s %-24s %s" % (mark, st, HOMES[st]))
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
    print("Blind spot 3: a partly-enrolled nested aggregate reads as named —")
    print("`fog.gain` names `fog`. The ledger carries the per-field truth.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
