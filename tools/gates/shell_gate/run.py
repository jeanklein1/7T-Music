#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE SHELL'S SMOKE TEST — gates/shell_gate/run.py
#
# It compiles a small main() over console/organ_scene.hpp and
# console/organ_repl.hpp, LINKS it, and RUNS it: five scenes down the
# ROAD, and a scripted session through the HAND. Every other gate in this
# tree reads text; this one executes the shell.
#
# THE NAME IS DELIBERATE AND IT IS A RETARGET, not a revival. A
# `tools/gates/shell_gate/run.py` stood here until WEB_SUNSET: it proved
# the C++ ↔ browser-shell seam, and it left with the shell it proved
# because there was nothing at the other end of it. THE_PANEL II §1.4
# rules that what it alone proved "either retargets to the REPL's smoke
# test or retires claimed". There is a shell again, so it retargets: same
# name, same job, a native other end.
#
# WHAT IT CAN PROVE WITHOUT A DEVICE, AND WHAT IT CANNOT — the split is
# the point, and THE_PANEL II §2 U5 named it in advance ("IF it can run
# deviceless (manifest parse + refusal path)"):
#
#   DEVICELESS, and therefore here:
#     · the parser — a malformed scene is refused BY LINE and nothing
#       lands,
#     · the schema check — a version this build does not read is refused
#       whole, before one key is applied,
#     · the manifest lookup — a key the manifest does not carry is named
#       and the FILE is refused, because a scene must never half-apply,
#     · WHOLE-ID MATCHING, which is the one that would be a real bug:
#       `WORLD.scheme_weights[0]` is a RETIRED world-draw key and
#       `WORLD.next_seed` is live block 15. A prefix match would write the
#       seed dial from a dead portal weight.
#
#   NOT DEVICELESS, and therefore the probe's `+scene` arm:
#     · the APPLY. `block_base` returns null until `bind_home` runs, so
#       every write is refused "the block has no home" — and binding one
#       needs a GPUState, whose wgpu handle members pull Dawn's release
#       symbols at LINK time. Verified, not assumed: the probe below
#       links clean without a GPUState and fails to link with one.
#
# So this gate proves the road REFUSES correctly, and Jean's probe proves
# it APPLIES. Neither claim is made by the other.
#
# USAGE   scene_gate/run.py · --print to see the compile line
# ═══════════════════════════════════════════════════════════════════════

import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
SRC = os.path.join(ROOT, "src")
STUBS = os.path.join(ROOT, "tools", "gates", "console_gate", "stubs")
DAWN_INC = os.path.join(ROOT, "third_party", "dawn_native_headers", "include")
SCENES = os.path.join(HERE, "scenes")

PROBE = """#include "console/organ_scene.hpp"
#include "console/organ_repl.hpp"
#include <iostream>
#include <string>
int main(int argc, char** argv) {
    if (argc < 3) return 2;
    if (std::string(argv[1]) == "scene") {
        t7::organ::SceneResult r = t7::organ::apply_scene(argv[2]);
        std::cout << "VERDICT ok=" << (r.ok ? 1 : 0)
                  << " applied=" << r.applied << " refused=" << r.refused
                  << " unknown=" << r.unknown << " schema=" << r.schema << "\\n";
        return 0;
    }
    // THE SCRIPTED SESSION. Every line after argv[1]=="repl" is one REPL
    // command, run through the same repl_exec the stdin lane calls.
    for (int i = 2; i < argc; ++i) t7::organ::repl_exec(argv[i]);
    // THE PENDING MASK, DRAINED AND PRINTED (WHEEL_0 R7). A door press
    // deviceless does exactly one thing that outlives the print: it ORs a
    // bit into g_doors_pending. take_doors_pending() is the same function
    // organ_flush calls at the frame boundary, so draining it here proves
    // the half of the door path that exists in this binary — the id
    // resolved, the bit landed, and an out-of-range id landed nothing.
    std::cout << "DOORS_PENDING 0x" << std::hex
              << t7::organ::take_doors_pending() << std::dec << "\\n";
    return 0;
}
"""

# scene -> (predicate on the VERDICT line, what the row is asserting)
CASES = [
    ("malformed.json",
     lambda v: v["ok"] == 0 and v["applied"] == 0,
     "a malformed scene is refused and NOTHING lands"),
    ("schema1.json",
     lambda v: v["ok"] == 0 and v["schema"] == 1 and v["applied"] == 0,
     "a scene with no `schema` reads as version 1 and is refused whole"),
    ("retired_key.json",
     lambda v: v["ok"] == 0 and v["unknown"] == 2 and v["applied"] == 0,
     "two retired ids are named and the FILE is refused — including "
     "`WORLD.scheme_weights[0]`, which a PREFIX match would have taken "
     "for live block 15"),
    ("witness.json",
     lambda v: v["ok"] == 1 and v["applied"] == 0,
     "a witness row (`_RO`) resolves in the manifest and is refused at "
     "the write — the meter is not a dial"),
    ("clean.json",
     lambda v: v["ok"] == 1 and v["unknown"] == 0,
     "a well-formed schema-2 scene resolves whole, prose and all"),
]

VERDICT = re.compile(r"VERDICT ok=(\d+) applied=(\d+) refused=(\d+) "
                     r"unknown=(\d+) schema=(-?\d+)")


def main():
    cxx = shutil.which("clang++") or shutil.which("g++")
    if not cxx:
        print("shell-gate: SKIP — no C++ compiler on PATH")
        return 0
    tmp = tempfile.mkdtemp(prefix="shell_gate_")
    try:
        src = os.path.join(tmp, "scene_probe.cpp")
        exe = os.path.join(tmp, "scene_probe")
        with open(src, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(PROBE)
        cmd = [cxx, "-std=gnu++20", "-DGLFW_INCLUDE_NONE",
               "-I", SRC, "-I", STUBS, "-I", DAWN_INC, "-o", exe, src]
        if "--print" in sys.argv:
            print("shell-gate: " + " ".join(cmd))
        p = subprocess.run(cmd, capture_output=True, text=True)
        if p.returncode != 0:
            print("shell-gate: FAIL — the road does not build and link "
                  "deviceless:\n" + (p.stdout or "") + (p.stderr or ""))
            return 1

        failed = 0
        for name, pred, claim in CASES:
            path = os.path.join(SCENES, name)
            r = subprocess.run([exe, "scene", path], capture_output=True, text=True)
            blob = (r.stdout or "") + (r.stderr or "")
            m = VERDICT.search(blob)
            if not m:
                print("  [FAIL] %-18s no verdict line\n%s" % (name, blob))
                failed += 1
                continue
            v = dict(zip(("ok", "applied", "refused", "unknown", "schema"),
                         (int(g) for g in m.groups())))
            if pred(v):
                print("  [PASS] %-18s %s" % (name, claim))
            else:
                print("  [FAIL] %-18s %s\n         got %s\n%s"
                      % (name, claim, v, blob))
                failed += 1

        # ─── THE HAND ────────────────────────────────────────────
        # A scripted session, run through the same repl_exec the stdin
        # lane calls. What it proves is the surface: the manifest is the
        # REPL's whole vocabulary, so every verb below is name-blind and
        # a new dial reaches all of them with no line edited here.
        exported = os.path.join(tmp, "round_trip.json")
        session = [
            "help",
            "list World",                 # a filter over group AND id
            "get WORLD.next_seed",
            "set WORLD.next_seed 12345",  # refused deviceless — no home
            "set LIGHTING.sun.intensity 3",   # refused ALWAYS — a witness
            "set WORLD.next_seed",        # too few lanes, named
            "get NO.SUCH.ROW",            # not in the manifest, named
            "doors",
            "door 3",
            "door 5",                     # the wheel's mode door (WHEEL_0)
            "door 5",                     # ...pressed again: the mask COALESCES
            "door 99",                    # out of range, named
            "export " + exported,
            "wat",                        # an unknown verb, named
        ]
        r = subprocess.run([exe, "repl"] + session, capture_output=True, text=True)
        blob = (r.stdout or "") + (r.stderr or "")
        checks = [
            ("the nine verbs answer `help`", "[REPL] list [filter]" in blob),
            ("`list` derives its sections from the rows' own group strings",
             "── World · Seed" in blob and "── World · Radius" in blob),
            ("`list <filter>` reports shown/total", re.search(r"\[REPL\] 3/\d+ row", blob) is not None),
            ("`get` prints value, range and derived cadence",
             "cadence gen (on respawn)" in blob),
            ("a WITNESS is refused by name — the meter is not a dial",
             "refused: LIGHTING.sun.intensity — a witness, not a dial" in blob),
            ("a row given too few lanes is refused with the count it wants",
             "takes 1 value(s); got 0" in blob),
            ("an id the manifest does not carry is named",
             "no row `NO.SUCH.ROW` in the manifest" in blob),
            ("`doors` prints the roster and `door <n>` presses one",
             "Rebirth the world" in blob and "pressed 3" in blob),
            ("a door id past the roster is refused with the count",
             "no door 99" in blob),
            # ── DOOR 5, THE WHEEL'S (WHEEL_0 R7) ────────────────────
            # WHAT THIS CAN AND CANNOT PROVE, stated because the ruling
            # asks for the limit rather than a claim: `reveal_zoetrope`
            # IS NOT IN THIS BINARY. The door's consumer is
            # organ_flush, which lives inside class Cartridge, which is
            # the only includer of bodies/cube_behaviors.hpp; this TU is
            # two console headers. So the bit is set and never taken,
            # `cbs.formation` never flips, and the `[Wheel] …` line is
            # never printed. That half is the probe's.
            #
            # Note this is NOT the `bind_home` limit the banner above
            # describes — that one stops the WRITE path. The door path
            # never gets far enough to be stopped by it; it stops one
            # tier earlier, because its consumer is not linked.
            #
            # WHAT IS LEFT IS WORTH PINNING ANYWAY, and it is the part
            # that rots: door 5's ROSTER POSITION and its LABEL, which
            # WHEEL_0 U3 changed and promised would not renumber.
            ("door 5 is the wheel's, by id and by label",
             "5  Wheel: take the choir / let it roam" in blob
             and "pressed 5 — Wheel: take the choir / let it roam" in blob),
            # THE MASK IS THE VERB PATH, deviceless. take_doors_pending()
            # is the same drain organ_flush calls, so this asserts that
            # both real presses landed their bits and that door 99
            # landed none: 3 and 5 set, nothing above ORGAN_DOOR_COUNT.
            ("a press lands its bit in the mask the frame boundary drains, "
             "and an out-of-range id lands none",
             re.search(r"DOORS_PENDING 0x([0-9a-f]+)", blob) is not None
             and int(re.search(r"DOORS_PENDING 0x([0-9a-f]+)", blob).group(1), 16)
                 == (1 << 3) | (1 << 5)),
            # THE COALESCE, which is why a "full cycle" is not
            # expressible here: two presses between two frame boundaries
            # are ONE raise by construction (organ_registry.hpp ORs a
            # bit). The session presses door 5 twice and the mask still
            # reads one bit — the acknowledgement prints twice, the
            # program is told once. A scripted cycle would need a frame,
            # which needs the cartridge, which needs Dawn.
            ("two presses between two boundaries are one raise",
             blob.count("pressed 5 — ") == 2),
            ("an unknown verb is named, not swallowed", "`wat`?" in blob),
            ("`export` skips every witness — a meter is not a scene's",
             re.search(r"witness\(es\) skipped", blob) is not None),
        ]
        for claim, ok in checks:
            print("  [%s] %-18s %s" % ("PASS" if ok else "FAIL", "repl", claim))
            if not ok:
                failed += 1
        if failed and "--print" in sys.argv:
            print(blob)

        # ─── THE ROUND TRIP ──────────────────────────────────────────
        # THE_PANEL I U3c found nine keys in the web era's own
        # baseline.json that no organ_set call could ever have accepted:
        # ARRAY members over a live block, written by an export that
        # walked the C++ STRUCT instead of the row list. An export that
        # does not walk the manifest writes a file the program cannot
        # read back. This is that law, checked: every row `export` wrote
        # must RESOLVE on `import`, with zero unknowns.
        if os.path.exists(exported):
            r = subprocess.run([exe, "scene", exported], capture_output=True, text=True)
            m = VERDICT.search((r.stdout or "") + (r.stderr or ""))
            ok = m is not None and int(m.group(4)) == 0 and int(m.group(1)) == 1
            print("  [%s] %-18s %s" % ("PASS" if ok else "FAIL", "round-trip",
                  "every row `export` wrote RESOLVES on `import` — zero "
                  "unknown keys, so the manifest walk cannot write a file "
                  "the program refuses"))
            if not ok:
                failed += 1
        else:
            print("  [FAIL] round-trip        `export` wrote no file")
            failed += 1

        if failed:
            # It said "%d of %d scene(s)" against len(CASES), but `failed`
            # accumulates over the scenes AND the REPL checks AND the round
            # trip — so a REPL-only failure reported "3 of 5 scene(s)" with
            # no scene failing. Counted honestly now (WHEEL_0 R7).
            print("\nshell-gate: FAIL — %d row(s) did not answer as the road "
                  "promises (%d scene(s) + %d repl check(s) + 1 round trip)"
                  % (failed, len(CASES), len(checks)))
            return 1
        print("\nshell-gate: PASS — the shell builds and links "
              "deviceless; %d scene(s) down the road and a scripted "
              "session through the hand answer as promised (the hand "
              "includes door 5, the wheel's — its ROSTER and LABEL and "
              "its bit in the pending mask; the VERB is the probe's, "
              "because reveal_zoetrope is not in this binary). THE APPLY "
              "PATH IS NOT PROVEN HERE and cannot be: block_base returns "
              "null until bind_home, and binding one needs a GPUState, "
              "which pulls Dawn at link time. That half is the probe's "
              "`+scene` arm." % len(CASES))
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
