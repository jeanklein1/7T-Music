#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE MIRROR'S PER-FIELD WITNESS — mirror_offsets.py
#
# It emits src/cartridges/the_board/realization/mirror_offsets.gen.inc:
# one `static_assert(offsetof(GPUFoo, member) == N)` per member of every
# struct the WGSL room registers with a `BYTE-FOR-BYTE (N B` marker, with
# N computed from world.wgsl by the WGSL layout rules. The C++ COMPILER
# then proves the two rooms agree field by field, and the TU gate is where
# a divergence stops.
#
# WHY IT EXISTS, AND IT IS NOT HYPOTHETICAL (THE_PANEL I U6). The two
# rooms had exactly two witnesses over a mirrored struct: a C++ `sizeof`
# static_assert, and the binding ledger's `0b-4`, which recomputes the
# WGSL room's SIZE and compares it to the marker. THE_PANEL I U2 proved by
# injection what those two cannot see between them:
#
#   remove a MID-STRUCT pad from ONE room only, before a 16-aligned
#   member, and the other room's implicit padding absorbs it. BOTH sizes
#   stay identical. Every offset between the cut and that member is now
#   four bytes apart across the rooms, and NOTHING SAYS SO.
#
# U5 then hit that case for real, one unit later, deleting `grain_band`
# from the WGSL DesignConfig while the C++ one kept a pad. It was caught
# by reading, because U2 had written the hazard down. This is the
# instrument that catches it by building.
#
# THE MARKER IS THE ENROLLMENT, exactly as it is for 0b-4: a struct joins
# by stating its size in its own banner, which L3 already asks for. The
# C++ twin is `GPU` + the WGSL name, and a struct whose twin is not found
# is REPORTED, never skipped in silence.
#
# WHAT IT DOES NOT ASSERT, said plainly rather than discovered later:
#   1. A member the C++ room does not declare by that name is SKIPPED —
#      that is how the C++ room's explicit alignment pads (`_pad_sun`,
#      `_pad_fog`) are handled, since the WGSL room gets them implicitly
#      and declares nothing. A skip is printed with its reason.
#   2. It proves OFFSETS, not TYPES. `float x[3]` against `vec3<f32>` and
#      `float x[4]` against `vec4<f32>` both land a following member at
#      the same place in one of the two rooms; the offsets of everything
#      after them are what this catches, and a same-size wrong type is
#      still the reader's job.
#   3. A struct nested inside another is walked at the TOP level only.
#      Its own members are asserted when it carries its own marker.
#
# USAGE   mirror_offsets.py · --check to diff against the tree, no write
# ═══════════════════════════════════════════════════════════════════════

import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from binding_ledger import (parse_wgsl_consts, parse_wgsl_structs, layout_of,
                            round_up, strip_wgsl_comments)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WGSL = os.path.join(ROOT, "src", "cartridges", "the_board", "realization", "world.wgsl")
STATE = os.path.join(ROOT, "src", "cartridges", "the_board", "realization", "state.hpp")
OUT = os.path.join(ROOT, "src", "cartridges", "the_board", "realization",
                   "mirror_offsets.gen.inc")

MARKER = re.compile(r"BYTE-FOR-BYTE[^)]*?\(\s*(\d+)\s*B\b", re.S)
STRUCT = re.compile(r"^\s*struct\s+(\w+)\s*\{", re.M)


def member_offsets(name, structs, consts):
    """The WGSL room's offset for each top-level member, by its own rules.

    The same arithmetic layout_of() runs internally — align each member,
    round the offset up to it, advance by its size. Restated here because
    layout_of returns a struct's SIZE and this needs its interior.
    """
    out = []
    off = 0
    for mname, mtype, aov, sov in structs[name]:
        mt = layout_of(mtype, structs, consts)
        if mt.align is None or mt.size is None:
            return None, "member %s (%s) is unresolved" % (mname, mtype)
        malign = aov or mt.align
        msize = sov if sov is not None else mt.size
        off = round_up(malign, off)
        out.append((mname, off, msize, mtype))
        off += msize
    return out, None


def cpp_members(twin, src):
    """Every member name the C++ twin declares, in order."""
    m = re.search(r"struct(?:\s+alignas\(\d+\))?\s+" + re.escape(twin) + r"\s*\{", src)
    if not m:
        return None
    i = m.end()
    depth, j = 1, i
    while j < len(src) and depth:
        if src[j] == "{":
            depth += 1
        elif src[j] == "}":
            depth -= 1
        j += 1
    body = src[i:j - 1]
    body = re.sub(r"//[^\n]*", " ", body)
    body = re.sub(r"/\*.*?\*/", " ", body, flags=re.S)
    names = []
    for line in body.split(";"):
        mm = re.search(r"([A-Za-z_]\w*)\s*((?:\[[^\]]*\])*)\s*$", line.strip())
        if mm and mm.group(1) not in ("struct", "public", "private"):
            names.append(mm.group(1))
    return names


def build():
    wsrc = open(WGSL, encoding="utf-8").read()
    csrc = open(STATE, encoding="utf-8").read()
    # THE MARKERS ARE READ FROM THE RAW TEXT AND THE TYPES FROM THE
    # STRIPPED TEXT, which is binding_ledger's own split and not a
    # preference: a marker LIVES in a comment, while parse_wgsl_structs
    # splits members on top-level commas and a comment containing one
    # silently swallows the members around it. Reading types from raw
    # source made DesignConfig parse as THIRTEEN members and 52 bytes on
    # the first run of this tool — a lossy parse that reports a confident
    # number, which is the worst shape a census can take.
    stripped = strip_wgsl_comments(wsrc)
    consts = parse_wgsl_consts(stripped)
    structs = parse_wgsl_structs(stripped)

    rows, notes, problems = [], [], []
    for m in MARKER.finditer(wsrc):
        nxt = STRUCT.search(wsrc, m.end())
        line = wsrc[:m.start()].count("\n") + 1
        if not nxt:
            problems.append("marker at world.wgsl:%d registers no struct" % line)
            continue
        name = nxt.group(1)
        twin = "GPU" + name
        cm = cpp_members(twin, csrc)
        if cm is None:
            problems.append("%s (world.wgsl:%d): no C++ twin `%s` in state.hpp"
                            % (name, line, twin))
            continue
        offs, err = member_offsets(name, structs, consts)
        if offs is None:
            problems.append("%s: %s" % (name, err))
            continue
        have = set(cm)
        asserts, skipped = [], []
        for mname, off, msize, mtype in offs:
            if mname in have:
                asserts.append((twin, mname, off, mtype))
            else:
                skipped.append(mname)
        rows.append((name, twin, asserts))
        if skipped:
            notes.append("%s: %d member(s) not declared under the same name in "
                         "%s and therefore not asserted: %s"
                         % (name, len(skipped), twin, ", ".join(skipped)))
    return rows, notes, problems


HEADER = """// GENERATED by tools/mirror_offsets.py — DO NOT HAND-EDIT (L28).
//
// THE MIRROR'S PER-FIELD WITNESS. Every line below is one member of a
// struct that world.wgsl registers with a `BYTE-FOR-BYTE (N B` marker,
// asserted at the offset THE WGSL ROOM puts it at. The number comes from
// world.wgsl and the check runs in the C++ compiler, so the two rooms
// prove each other rather than each proving itself.
//
// WHAT THIS CATCHES THAT NOTHING ELSE DID: a pad removed from one room
// and not the other, upstream of a 16-aligned member. Both rooms' SIZES
// stay identical — each pads implicitly back to the boundary — while
// every offset in between sits apart. The `sizeof` asserts cannot see it
// and neither can the binding ledger's 0b-4. THE_PANEL I U2 proved that
// by injection; U5 then hit it for real one unit later.
//
// Included from state.hpp after the mirrored structs are declared.
"""


def render(rows, notes):
    out = [HEADER]
    for name, twin, asserts in rows:
        out.append("\n// ── %s  ↔  %s  (%d members asserted) ──"
                   % (name, twin, len(asserts)))
        for twin_, mname, off, mtype in asserts:
            out.append('static_assert(offsetof(%s, %s) == %d,\n'
                       '    "%s.%s is at %d in world.wgsl (%s); the two rooms '
                       'must agree field for field");'
                       % (twin_, mname, off, name, mname, off, mtype))
    if notes:
        out.append("\n// SKIPPED, WITH THE REASON (see the tool's banner, note 1):")
        for n in notes:
            out.append("//   " + n)
    return "\n".join(out) + "\n"


def main():
    check = "--check" in sys.argv
    rows, notes, problems = build()
    text = render(rows, notes)
    total = sum(len(a) for _, _, a in rows)
    for n in notes:
        print("  note: " + n)
    for p in problems:
        print("  PROBLEM: " + p)
    if check:
        cur = open(OUT, encoding="utf-8").read() if os.path.exists(OUT) else None
        if cur == text and not problems:
            print("mirror-offsets: PASS — %d member offset(s) across %d "
                  "marker-registered struct(s), and the tree carries them"
                  % (total, len(rows)))
            return 0
        if problems:
            print("mirror-offsets: FAIL — the census could not be built")
            return 1
        print("mirror-offsets: FAIL — %s is stale; re-run without --check" % OUT)
        return 1
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print("wrote %s (%d member offset(s), %d struct(s))"
          % (os.path.relpath(OUT, ROOT), total, len(rows)))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
