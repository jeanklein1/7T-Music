#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE TU GATE (GATE_1; widened by GATEHOUSE_0) — the tree's translation
# units, compiled per commit, with their WARNINGS READ
#
# The directory is still named console_gate: its stubs and PROVENANCE.md
# live here and are cited elsewhere, and a rename would cost every one of
# those references to buy a tidier path. The gate is the script.
#
# WHY THIS EXISTS. glaw1's translation unit is cartridge.hpp, and
# cartridge.hpp does not include console.hpp — only incubator_dual.cpp
# does, and nothing compiles that here. So for the whole life of the tree
# glaw1 answered GREEN to every console edit without reading one line of
# it. That is not a weak witness; it is an absent one wearing a witness's
# name, and it is how two defects shipped:
#
#   · the SetImmediates hole — console.hpp called an API the emsdk's own
#     emdawnwebgpu generation did not carry, and nothing said so until an
#     actual build failed (F5F);
#   · t7::g_dropped_submits used in console.hpp with no include of
#     core/instruments.hpp — a missing include that no gate could see
#     because no gate opened the file (ACQ round).
#
# GATEHOUSE_0 WIDENED IT TWICE, and both widenings were paid for by the
# same two characters.
#
# FIRST, THE SUBJECT. PROBATE_SEAL edited renderer.hpp heavily and
# discovered that NOTHING in the container had ever compiled it — the
# gate's TU was console.hpp alone, and cartridge.hpp (the one glaw1
# actually builds) was checked ad hoc, by hand, once. Ad hoc is not a
# gate. cartridge.hpp is a TU here now.
#
# SECOND, THE TESTIMONY. That ad-hoc run passed `-w`, and read
# `-fsyntax-only`'s EXIT STATUS. Warnings exit zero. So the run was
# green while clang was holding two `-Winvalid-pp-token` diagnostics
# about EM_ASM bodies in renderer.hpp — the gate asked the compiler a
# question, was told the answer, and read only the verdict. Jean's build
# printed both warnings on the first try.
#
# WARNING PARITY, and the flag set is chosen rather than assumed: the
# emcc line in CMakeLists passes NO -W flags at all (T7_WEB_OPT is -O0/-g
# or -O2; T7_WEB_DIAG is emscripten -s settings only), so the build runs
# on clang's DEFAULTS — which is exactly where -Winvalid-pp-token lives.
# Parity therefore means passing no -W flags here either, and above all
# not passing -w. Any line matching `warning:` is red, same posture as
# an error.
#
# WHAT IT PROVES. Each TU parses, scopes, resolves every name and
# type-checks against the REAL API surface the web build will use: the
# vendored emdawnwebgpu payload (F5F's pin, never a system or emsdk copy)
# and the GLFW header the contrib port actually wraps — and does so
# without a single diagnostic.
#
# WHAT IT DOES NOT PROVE. Nothing is linked, nothing is run, no semantics
# are checked, and the Emscripten C surface behind the stubs is vacuous by
# construction. Pipeline-layout conformance and minBindingSize remain
# invisible here exactly as they are to naga — ATLAS_1revB's law stands:
# THE WEB BOOT IS THE WITNESS OF RECORD for everything past the type
# surface. This gate closes the gap between "nobody read it" and "it
# type-checks clean", and claims no more ground than that.
#
# THE NATIVE ARM IS NOT COMPILED, and after SUNSET_1 there is none to
# compile.
#
# USAGE
#   python3 tools/gates/console_gate/run.py           # gate; exit 1 on failure
#   python3 tools/gates/console_gate/run.py --print   # also echo the commands
#
# Provenance of every vendored/stubbed header: PROVENANCE.md beside this file.
# ═══════════════════════════════════════════════════════════════════════
"""GATE_1: compile the tree's TUs against the vendored API surface, warnings read."""

import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))

SRC = os.path.join(ROOT, "src")
STUBS = os.path.join(HERE, "stubs")
PKG = os.path.join(ROOT, "third_party", "emdawnwebgpu", "emdawnwebgpu_pkg")
WEBGPU_CPP_INC = os.path.join(PKG, "webgpu_cpp", "include")
WEBGPU_C_INC = os.path.join(PKG, "webgpu", "include")

# The subjects. One include each, on purpose: the gate's question is
# whether each header stands up ON ITS OWN, not whether some caller
# happens to include something first.
#
# cartridge.hpp is glaw1's own TU and reaches renderer.hpp, state.hpp and
# the whole realization family. console.hpp is the one glaw1 never sees.
# Between them every hand-edited header in the program is compiled.
TUS = [
    ("cartridge", '#include "cartridges/the_board/cartridge.hpp"\n'),
    ("console", '#include "console/console.hpp"\n'),
]

def main() -> int:
    for path, what in ((SRC, "src/"),
                       (STUBS, "the stub dir"),
                       (WEBGPU_CPP_INC, "the vendored emdawnwebgpu C++ headers"),
                       (WEBGPU_C_INC, "the vendored emdawnwebgpu C headers")):
        if not os.path.isdir(path):
            print("tu-gate: MISSING %s at %s" % (what, path))
            if PKG in path:
                print("  The emdawnwebgpu payload is not in the tree. The gate "
                      "compiles against the PIN and never against a system or "
                      "emsdk copy — fetch per third_party/emdawnwebgpu/PINNED.md.")
            return 1

    cxx = shutil.which("clang++") or shutil.which("g++")
    if cxx is None:
        print("tu-gate: no clang++ or g++ on PATH")
        return 1

    tmp = tempfile.mkdtemp(prefix="tu_gate_")
    failed = []
    try:
        for name, tu in TUS:
            tu_path = os.path.join(tmp, "tu_%s_web.cpp" % name)
            with open(tu_path, "w", encoding="utf-8", newline="\n") as fh:
                fh.write(tu)

            cmd = [
                cxx, "-fsyntax-only", "-std=gnu++20",
                # NO -w, and no -W either. WARNING PARITY: the emcc line
                # in CMakeLists passes no -W flags, so the build runs on
                # clang's defaults and so does this. -w is what hid two
                # -Winvalid-pp-token diagnostics through a whole round.
                #
                # SUNSET_1 deleted every __EMSCRIPTEN__ guard, so this
                # define no longer selects an arm — no source reads it. It
                # stays for FIDELITY: emcc always defines it, the
                # Emscripten headers may branch on it themselves, and a
                # gate that compiles under different macros than the build
                # is answering a different question. GLFW_INCLUDE_NONE
                # keeps glfw3.h from reaching for an OpenGL header this
                # tree never uses.
                "-D__EMSCRIPTEN__", "-DGLFW_INCLUDE_NONE",
                "-I", SRC,
                "-I", STUBS,
                "-I", WEBGPU_CPP_INC,
                "-I", WEBGPU_C_INC,
                tu_path,
            ]
            if "--print" in sys.argv:
                print("tu-gate: " + " ".join(cmd))

            proc = subprocess.run(cmd, capture_output=True, text=True)
            blob = (proc.stdout or "") + (proc.stderr or "")
            # READ THE TESTIMONY, not just the verdict.
            # `warning:` is red. A warning exits 0, which is precisely
            # how this gate reported green while clang was naming two
            # defects in the file the round had just rewritten.
            warns = [l for l in blob.split("\n") if re.search(r"\bwarning:", l)]
            if proc.returncode != 0 or warns:
                sys.stdout.write(blob)
                why = "errors" if proc.returncode != 0 else "%d warning(s)" % len(warns)
                print("tu-gate: FAIL — %s.hpp TU: %s (%s)"
                      % (name, why, os.path.basename(cxx)))
                failed.append(name)

        if failed:
            print("tu-gate: FAIL — %d of %d TU(s) not clean: %s"
                  % (len(failed), len(TUS), ", ".join(failed)))
            return 1

        print("tu-gate: PASS — %d TU(s) parse, scope and type-check against the "
              "pinned emdawnwebgpu surface with ZERO diagnostics [%s]"
              % (len(TUS), os.path.basename(cxx)))
        print("  TUs: %s" % ", ".join("%s.hpp" % n for n, _ in TUS))
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
