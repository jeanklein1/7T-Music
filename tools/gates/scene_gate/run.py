#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE SCENE ROAD'S SMOKE TEST — gates/scene_gate/run.py
#
# It compiles a small main() over console/organ_scene.hpp, LINKS it, and
# RUNS it against five scenes. Every other gate in this tree reads text;
# this one executes the road.
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
#include <iostream>
int main(int argc, char** argv) {
    if (argc < 2) return 2;
    t7::organ::SceneResult r = t7::organ::apply_scene(argv[1]);
    std::cout << "VERDICT ok=" << (r.ok ? 1 : 0)
              << " applied=" << r.applied << " refused=" << r.refused
              << " unknown=" << r.unknown << " schema=" << r.schema << "\\n";
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
        print("scene-gate: SKIP — no C++ compiler on PATH")
        return 0
    tmp = tempfile.mkdtemp(prefix="scene_gate_")
    try:
        src = os.path.join(tmp, "scene_probe.cpp")
        exe = os.path.join(tmp, "scene_probe")
        with open(src, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(PROBE)
        cmd = [cxx, "-std=gnu++20", "-DGLFW_INCLUDE_NONE",
               "-I", SRC, "-I", STUBS, "-I", DAWN_INC, "-o", exe, src]
        if "--print" in sys.argv:
            print("scene-gate: " + " ".join(cmd))
        p = subprocess.run(cmd, capture_output=True, text=True)
        if p.returncode != 0:
            print("scene-gate: FAIL — the road does not build and link "
                  "deviceless:\n" + (p.stdout or "") + (p.stderr or ""))
            return 1

        failed = 0
        for name, pred, claim in CASES:
            path = os.path.join(SCENES, name)
            r = subprocess.run([exe, path], capture_output=True, text=True)
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

        if failed:
            print("\nscene-gate: FAIL — %d of %d scene(s) did not answer as "
                  "the road promises" % (failed, len(CASES)))
            return 1
        print("\nscene-gate: PASS — the scene road builds and links "
              "deviceless, and %d scene(s) answer as promised. THE APPLY "
              "PATH IS NOT PROVEN HERE and cannot be: block_base returns "
              "null until bind_home, and binding one needs a GPUState, "
              "which pulls Dawn at link time. That half is the probe's "
              "`+scene` arm." % len(CASES))
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
