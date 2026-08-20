#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE ORGAN'S OWN BOOK (ORGAN_4 P5) — organ_ledger.py
#
# WHAT THIS IS. The audit family's FIFTH member. BINDING, COMMAND,
# MANIFEST and MIRROR each keep a generated book about one of the
# program's rooms; the organ had instruments (organ_gap, organ_readers)
# and no book. This emits it: audit/ORGAN.md, one committed markdown
# table over every enrolled row, the tallies that summarise it, and the
# tails of both check tools underneath.
#
# WHY A BOOK AND NOT JUST THE TOOLS. Two reasons, and the second is the
# larger one.
#
#   · SEARCHABILITY. `audit/MANIFEST.md` and `audit/BINDING_LEDGER.md`
#     are law and stay searchable (CLAUDE.md). A question like "what is
#     the range on the cohesion radius" should be answerable by grepping
#     a file in the tree, not by building the program and opening a
#     panel.
#   · THE COUPLING MENU. A coupling is a parameter set into trajectory
#     over time, so EVERY ROW WITH AN AUTHORED RANGE IS A TRAJECTORY
#     DOMAIN. This table is the music campaign's target map — the (min,
#     max) column is the domain a trajectory would play over, and the
#     cadence column says whether playing it would be heard now, at a
#     boundary, or at the author's next event.
#
# THE PARSER IS THE ONE organ_gap.py AND organ_readers.py USE, for the
# reason the compiled registry exists at all: three copies of the same
# reading can drift, and the .inc is the one authority. What is added
# here is the DERIVATION the C++ does — `derived_cadence()` restated
# once, in Python, against the same rules organ_registry.hpp states in
# C++, so the book and the manifest cannot disagree about what a row
# means. If they ever do, this file is wrong and the header is where to
# look first.
#
# THE DOOR TABLE is parsed out of organ_registry.hpp's `kOrganDoors`
# rather than restated, for the same reason.
#
# USAGE
#   python3 tools/organ_ledger.py              # write audit/ORGAN.md
#   python3 tools/organ_ledger.py --check      # print, write nothing
#   python3 tools/organ_ledger.py -o PATH      # write elsewhere
#
# LF, no BOM, single trailing newline — verified by read-back, the
# binding_ledger G2-eol precedent.
# ═══════════════════════════════════════════════════════════════════════

import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INC = os.path.join(ROOT, "src", "console", "organ_params.inc")
REG = os.path.join(ROOT, "src", "console", "organ_registry.hpp")
OUT = os.path.join(ROOT, "audit", "ORGAN.md")

# ─── THE ENROLLMENT LIST (the shared parser) ──────────────────────────
MACRO = re.compile(
    r"^(ORGAN_PARAM(?:_GEN|_DEF|_DEFONLY|_RO)?)(_NS)?\s*\((.*)\)\s*$")

# The sentinel a definition-only family lands on. Mirrors
# ORGAN_DEFONLY_BLOCK_##DEFKIND in organ_registry.hpp; the convention
# DESCENDS from 255 and a third family adds one line there and one here.
DEFONLY_BLOCK = {"MOOD": "NONE (255)", "ORB_MOOD": "NONE_ORB (254)"}


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


def num(tok):
    """`0.005f` -> `0.005` · `the_board::FIELD_BEACON_S_MAX` -> itself.

    A range bound that is a NAMED CONSTANT stays named: the beacon's
    ceiling is `FIELD_K − 1` and printing 299 would hide the law it
    carries.
    """
    t = tok.strip()
    m = re.fullmatch(r"(-?[0-9]*\.?[0-9]+)f?", t)
    if not m:
        return t.replace("the_board::", "")
    v = float(m.group(1))
    return ("%g" % v)


def derived_cadence(row):
    """organ_registry.hpp's `derived_cadence()`, restated once.

    Order matters, exactly as it does there: a witness is DRIVEN even if
    its block has a boundary, because the row is a meter and a meter's
    cadence is its author's.
    """
    if row["ro"]:
        return "driven"
    if row["defkind"] != "NONE" or row["defonly"] or row["block"] == "ORBS":
        return "boundary"
    return row["cad"]


def rows():
    out = []
    with open(INC, encoding="utf-8") as f:
        for n, raw in enumerate(f, 1):
            m = MACRO.match(raw.strip())
            if not m:
                continue
            form, ns, args = m.group(1), m.group(2), split_args(m.group(3))
            k = 1 if ns else 0
            r = {"line": n, "form": form, "ns": (args[0] if ns else "the_board"),
                 "ro": False, "defonly": False, "defkind": "NONE",
                 "cad": "live", "minv": "", "maxv": "", "step": ""}
            if form in ("ORGAN_PARAM", "ORGAN_PARAM_GEN") and len(args) >= 9 + k:
                r.update(block=args[0 + k], struct=args[1 + k], field=args[2 + k],
                         type=args[3 + k].replace("ORGAN_", ""),
                         minv=num(args[4 + k]), maxv=num(args[5 + k]),
                         step=num(args[6 + k]),
                         group=args[7 + k].strip('"'), label=args[8 + k].strip('"'))
                if form == "ORGAN_PARAM_GEN":
                    r["cad"] = "gen"
            elif form == "ORGAN_PARAM_RO" and len(args) >= 6 + k:
                r.update(block=args[0 + k], struct=args[1 + k], field=args[2 + k],
                         type=args[3 + k].replace("ORGAN_", ""),
                         group=args[4 + k].strip('"'), label=args[5 + k].strip('"'),
                         ro=True)
            elif form == "ORGAN_PARAM_DEF" and len(args) >= 12 + k:
                r.update(block=args[0 + k], struct=args[1 + k], field=args[2 + k],
                         type=args[3 + k].replace("ORGAN_", ""),
                         minv=num(args[4 + k]), maxv=num(args[5 + k]),
                         step=num(args[6 + k]),
                         group=args[7 + k].strip('"'), label=args[8 + k].strip('"'),
                         defkind=args[9 + k],
                         defstruct=args[10 + k], deffield=args[11 + k])
            elif form == "ORGAN_PARAM_DEFONLY" and len(args) >= 9 + k:
                # TYPE MIN MAX STEP GROUP LABEL DEFKIND DEFSTRUCT DEFFIELD —
                # no BLOCK/STRUCT/FIELD at all, because there is no instance.
                r.update(block=DEFONLY_BLOCK.get(args[6 + k], "NONE?"),
                         struct=args[7 + k], field=args[8 + k],
                         type=args[0 + k].replace("ORGAN_", ""),
                         minv=num(args[1 + k]), maxv=num(args[2 + k]),
                         step=num(args[3 + k]),
                         group=args[4 + k].strip('"'), label=args[5 + k].strip('"'),
                         defkind=args[6 + k], defstruct=args[7 + k],
                         deffield=args[8 + k], defonly=True)
            else:
                continue
            # the id, exactly as organ_manifest builds it
            r["id"] = ("%s.%s" % (r["struct"], r["field"])) if r["defonly"] \
                else ("%s.%s" % (r["block"].split(" ")[0], r["field"]))
            r["section"] = r["group"].split(" · ")[0]
            r["cadence"] = derived_cadence(r)
            out.append(r)
    return out


# ─── THE DOORS (parsed, not restated) ─────────────────────────────────
def doors():
    text = open(REG, encoding="utf-8").read()
    m = re.search(r"kOrganDoors\[\]\s*=\s*\{(.*?)\n\};", text, re.S)
    if not m:
        return []
    return re.findall(r"\{\s*(ORGAN_DOOR_\w+)\s*,\s*\"([^\"]*)\"\s*\}", m.group(1))


def blocks_used(rs):
    return sorted({r["block"] for r in rs})


def tail_of(tool, keep):
    """The last `keep` lines of a check tool's stdout, verbatim."""
    try:
        p = subprocess.run([sys.executable, os.path.join(ROOT, "tools", tool)],
                           capture_output=True, text=True, timeout=600)
    except (OSError, subprocess.SubprocessError) as e:
        return ["(%s did not run: %s)" % (tool, e)]
    lines = (p.stdout or "").rstrip("\n").split("\n")
    return lines[-keep:] if len(lines) > keep else lines


def tally(rs, key):
    counts = {}
    for r in rs:
        counts[r[key]] = counts.get(r[key], 0) + 1
    return counts


def emit():
    rs = rows()
    L = []
    A = L.append

    A("# ORGAN — the organ's own book")
    A("")
    A("<!-- GENERATED by tools/organ_ledger.py from src/console/organ_params.inc")
    A("     and src/console/organ_registry.hpp — do not hand-edit.")
    A("     Regenerate: python3 tools/organ_ledger.py -->")
    A("")
    A("The audit family's fifth member. BINDING, COMMAND, MANIFEST and MIRROR")
    A("each keep a book about one of the program's rooms; this is the panel's.")
    A("")
    A("**A coupling is a parameter set into trajectory over time**, so every")
    A("row below with an authored range is a TRAJECTORY DOMAIN. This table is")
    A("the music campaign's target map, not panel decoration: the range column")
    A("is the domain a trajectory would play over, and the cadence column says")
    A("whether playing it would be heard now, at the frame boundary, or at the")
    A("author's next natural event.")
    A("")
    A("Cadence is DERIVED, never stored — the rule lives once in")
    A("`organ_registry.hpp::derived_cadence()` and is restated once in this")
    A("generator, so the book and the manifest cannot disagree. `driven` is an")
    A("`_RO` witness: the panel meters it and `organ_set` refuses to write it.")
    A("")

    A("## Every enrolled row")
    A("")
    A("| section | label | id | block / family | type | range | step | cadence | def-kind | ro |")
    A("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for r in rs:
        rng = "—" if r["ro"] else "%s … %s" % (r["minv"], r["maxv"])
        stp = "—" if r["ro"] else r["step"]
        group = r["group"].split(" · ")
        grp = group[1] if len(group) > 1 else ""
        A("| %s · %s | %s | `%s` | %s | %s | %s | %s | %s | %s | %s |"
          % (r["section"], grp, r["label"], r["id"], r["block"], r["type"],
             rng, stp, r["cadence"], r["defkind"].lower(),
             "•" if r["ro"] else ""))
    A("")

    A("## The tallies")
    A("")
    A("| | |")
    A("| --- | --- |")
    A("| entries | **%d** |" % len(rs))
    A("| by section | %s |" % " · ".join(
        "%s %d" % (k, v) for k, v in sorted(tally(rs, "section").items(),
                                            key=lambda kv: -kv[1])))
    A("| by cadence | %s |" % " · ".join(
        "%s %d" % (k, v) for k, v in sorted(tally(rs, "cadence").items())))
    A("| by macro form | %s |" % " · ".join(
        "%s %d" % (k.replace("ORGAN_PARAM", "PARAM"), v)
        for k, v in sorted(tally(rs, "form").items())))
    A("| definition kinds | %s |" % " · ".join(
        "%s %d" % (k, v) for k, v in sorted(tally(rs, "defkind").items())))
    A("| witnesses (`ro`) | %d |" % sum(1 for r in rs if r["ro"]))
    A("| blocks and sentinels used | %s |" % ", ".join(blocks_used(rs)))
    A("| namespaces | %s |" % " · ".join(
        "%s %d" % (k, v) for k, v in sorted(tally(rs, "ns").items())))
    A("")
    A("### Doors")
    A("")
    A("A door is the panel pressing the program's OWN machinery: it raises")
    A("flags the frame boundary already consumes and adds no author. Parsed")
    A("from `kOrganDoors`, never restated.")
    A("")
    A("| id | label |")
    A("| --- | --- |")
    for i, (sym, label) in enumerate(doors()):
        A("| %d `%s` | %s |" % (i, sym, label))
    A("")

    A("## THE GAP")
    A("")
    A("`tools/organ_gap.py` — members of the enrolled homes the panel does")
    A("NOT name, and the reader witness over every graduated pair. The tail")
    A("of its run, verbatim:")
    A("")
    A("```")
    for line in tail_of("organ_gap.py", 28):
        A(line)
    A("```")
    A("")

    A("## THE READERS")
    A("")
    A("`tools/organ_readers.py` — does your reader name you? An enrollment")
    A("states a belief; only the reader proves it. The tail of its run,")
    A("verbatim:")
    A("")
    A("```")
    for line in tail_of("organ_readers.py", 16):
        A(line)
    A("```")
    # exactly one trailing newline, so the byte check below can prove it
    return "\n".join(L).rstrip("\n") + "\n"


def main():
    text = emit()
    if "--check" in sys.argv:
        sys.stdout.write(text)
        return 0
    out = OUT
    if "-o" in sys.argv:
        out = sys.argv[sys.argv.index("-o") + 1]
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    # G2-eol, the binding_ledger precedent: the writer pins the terminator
    # and a byte-level read-back proves it, so no host can translate it.
    raw = open(out, "rb").read()
    print("wrote %s (%d lines, %d bytes)"
          % (os.path.relpath(out, ROOT).replace(os.sep, "/"),
             raw.count(b"\n"), len(raw)))
    print("  byte check: %d CRLF, %d bare CR, %d LF, BOM %s, single trailing LF %s -> %s"
          % (raw.count(b"\r\n"), raw.count(b"\r") - raw.count(b"\r\n"),
             raw.count(b"\n"), raw[:3] == b"\xef\xbb\xbf",
             raw.endswith(b"\n") and not raw.endswith(b"\n\n"),
             "LF-CLEAN" if b"\r" not in raw and raw[:3] != b"\xef\xbb\xbf"
             else "DIRTY"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
