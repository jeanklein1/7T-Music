#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE CONSOLE GATE (GATE_1) — console.hpp, compiled per commit
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
# WHAT IT PROVES. console.hpp's WEB arm parses, scopes, resolves every
# name, and type-checks — against the REAL API surface the web build will
# use: the vendored emdawnwebgpu payload (F5F's pin, never a system or
# emsdk copy) and the GLFW header the contrib port actually wraps. If
# console.hpp names a symbol those headers lack, this fails.
#
# WHAT IT DOES NOT PROVE. Nothing is linked, nothing is run, no semantics
# are checked, and the Emscripten C surface behind the stubs is vacuous by
# construction. Pipeline-layout conformance and minBindingSize remain
# invisible here exactly as they are to naga — ATLAS_1revB's law stands:
# THE WEB BOOT IS THE WITNESS OF RECORD for everything past the type
# surface. This gate closes the gap between "nobody read it" and "it
# type-checks", and claims no more ground than that.
#
# THE NATIVE ARM IS NOT COMPILED, and after SUNSET_1 there is none to
# compile. Before that unit it could not be: it named Dawn enumerators
# that exist in the archived native generation and not in the pinned web
# package, which was itself a symptom of dead code (SUNSET_0's flag).
#
# USAGE
#   python3 tools/gates/console_gate/run.py           # gate; exit 1 on failure
#   python3 tools/gates/console_gate/run.py --print   # also echo the command
#
# Provenance of every vendored/stubbed header: PROVENANCE.md beside this file.
# ═══════════════════════════════════════════════════════════════════════
"""GATE_1: compile console.hpp's web arm against the vendored API surface."""

import os
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

# The subject. One line, on purpose: the gate's question is whether this
# header stands up on its own, not whether some caller happens to include
# something first.
TU = '#include "console/console.hpp"\n'


def main() -> int:
    for path, what in ((SRC, "src/"),
                       (STUBS, "the stub dir"),
                       (WEBGPU_CPP_INC, "the vendored emdawnwebgpu C++ headers"),
                       (WEBGPU_C_INC, "the vendored emdawnwebgpu C headers")):
        if not os.path.isdir(path):
            print("console-gate: MISSING %s at %s" % (what, path))
            if PKG in path:
                print("  The emdawnwebgpu payload is not in the tree. The gate "
                      "compiles against the PIN and never against a system or "
                      "emsdk copy — fetch per third_party/emdawnwebgpu/PINNED.md.")
            return 1

    cxx = shutil.which("clang++") or shutil.which("g++")
    if cxx is None:
        print("console-gate: no clang++ or g++ on PATH")
        return 1

    tmp = tempfile.mkdtemp(prefix="console_gate_")
    try:
        tu_path = os.path.join(tmp, "tu_console_web.cpp")
        with open(tu_path, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(TU)

        cmd = [
            cxx, "-fsyntax-only", "-std=gnu++20", "-w",
            # SUNSET_1 deleted every __EMSCRIPTEN__ guard, so this define no
            # longer selects an arm — no source reads it. It stays for
            # FIDELITY: emcc always defines it, the Emscripten headers may
            # branch on it themselves, and a gate that compiles under
            # different macros than the build is answering a different
            # question. GLFW_INCLUDE_NONE keeps glfw3.h from reaching for an
            # OpenGL header this tree never uses.
            "-D__EMSCRIPTEN__", "-DGLFW_INCLUDE_NONE",
            "-I", SRC,
            "-I", STUBS,
            "-I", WEBGPU_CPP_INC,
            "-I", WEBGPU_C_INC,
            tu_path,
        ]
        if "--print" in sys.argv:
            print("console-gate: " + " ".join(cmd))

        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            sys.stdout.write(proc.stdout)
            sys.stderr.write(proc.stderr)
            print("console-gate: FAIL — console.hpp does not compile against "
                  "the vendored API surface (%s)" % os.path.basename(cxx))
            return 1

        if proc.stderr.strip():
            sys.stderr.write(proc.stderr)
        print("console-gate: PASS — console.hpp (web arm) parses, scopes and "
              "type-checks against the pinned emdawnwebgpu surface [%s]"
              % os.path.basename(cxx))
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
