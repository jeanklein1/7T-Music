#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE TU GATE (GATE_1; widened by GATEHOUSE_0; tiered by WEB_SUNSET) —
# the tree's translation units, compiled per commit, with their WARNINGS
# READ
#
# The directory is still named console_gate: its stubs and PROVENANCE.md
# live here and are cited elsewhere, and a rename would cost every one of
# those references to buy a tidier path. The gate is the script.
#
# WHY THIS EXISTS. glaw1's translation unit is cartridge.hpp, and
# cartridge.hpp does not include console.hpp — only the_board.cpp
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
# build passes NO -W flags of its own, so it runs on the compiler's
# DEFAULTS — which is exactly where -Winvalid-pp-token lived. Parity
# therefore means passing no -W flags here either, and above all not
# passing -w. Any line matching `warning:` is red, same posture as an
# error.
#
# WHAT IT PROVES, POST-WEB_SUNSET — IN TWO TIERS, EACH NAMED IN ITS OWN
# VERDICT LINE. Tier CARTRIDGE: cartridge.hpp parses, scopes and
# type-checks as the native program against the emdawnwebgpu surface —
# proven sufficient for that cohort, and the first native type witness
# the realization family ever had. Tier CONSOLE: console.hpp and
# the_board.cpp against third_party/dawn_native_headers — Dawn's NATIVE
# header generation at the same revision (L37), because the web-target
# generation does not carry the native extensions those arms use
# (Adapter::CreateDevice closed that door; a stub cannot add a member).
# A dormant tier says so out loud and points at its OPEN item; it never
# passes silently.
#
# WHAT IT DOES NOT PROVE. Nothing is linked, nothing is run. THE NATIVE
# BOOT IS THE WITNESS OF RECORD past the type surface.
#
# USAGE
#   python3 tools/gates/console_gate/run.py           # gate; exit 1 on failure
#   python3 tools/gates/console_gate/run.py --print   # also echo the commands
#
# Provenance of every vendored/stubbed header: PROVENANCE.md beside this file.
# ═══════════════════════════════════════════════════════════════════════
"""GATE_1: compile the tree's TUs, in two tiers, against their pinned surfaces."""

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

# WEB_SUNSET R-A — the native surface. Dawn's NATIVE header generation at
# the same revision as the emdawnwebgpu payload; its own PINNED.md is the
# receipt. Absent is legal and LOUD: see tier CONSOLE below.
DAWN_NATIVE = os.path.join(ROOT, "third_party", "dawn_native_headers")
DAWN_NATIVE_INC = os.path.join(DAWN_NATIVE, "include")

# ── THE TIERS (WEB_SUNSET R-C) ──────────────────────────────────────
# One include line per tier, because the two cohorts do not share a
# surface. cartridge.hpp reaches renderer.hpp, state.hpp and the whole
# realization family and needs only webgpu_cpp; console.hpp and
# the_board.cpp reach Dawn's native extensions and need the native
# generation. Compiling both against one surface is what the web-mode
# gate did, and it is why the console arms went unread for the whole
# life of the tree.
TIER_CARTRIDGE = [
    ("cartridge", "cartridge.hpp",
     '#include "cartridges/the_board/cartridge.hpp"\n'),
]

# PANORAMA_1: the harness itself. the_board.cpp is the program's ONLY real
# translation unit — it holds main(), the frame driver and the READY offer
# — and until GATEHOUSE_0 nothing here opened it. It needs no -D: the file
# defaults INCUBATE_RENDER itself.
TIER_CONSOLE = [
    ("console", "console.hpp", '#include "console/console.hpp"\n'),
    ("the_board", "the_board.cpp", '#include "the_board.cpp"\n'),
]

# P-16 — the dormant-tier line. A named absence, never a silent one.
DORMANT_LINES = [
    "tu-gate: tier CONSOLE DORMANT — third_party/dawn_native_headers absent.",
    "  console.hpp and the_board.cpp are NOT type-witnessed by this run.",
    "  Supply recipe: docs/OPEN.md, THE NATIVE HEADER SURFACE.",
]


def shallow_note():
    """L29 — the environment announces its own depth.

    A shallow clone answers "absent" for everything below its graft, and
    that answer is indistinguishable from the truth from inside. Every
    gate that touches git history prints this before it speaks, so
    nobody has to remember to ask. One line, no cost, no judgement.
    """
    # the file's own root constant, not a re-derived depth: the
    # first form here was one dirname short and the note never fired.
    if os.path.exists(os.path.join(ROOT, ".git", "shallow")):
        print("  [gate] NOTE: shallow clone — history-derived claims are "
              "unsafe until git fetch --unshallow")


def compile_tu(cxx, tmp, name, source, includes):
    """One TU, one verdict. Returns (ok, blob)."""
    tu_path = os.path.join(tmp, "tu_%s.cpp" % name)
    with open(tu_path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(source)

    cmd = [cxx, "-fsyntax-only", "-std=gnu++20"]
    # NO -w, and no -W either — see WARNING PARITY above. GLFW_INCLUDE_NONE
    # keeps glfw3.h from reaching for an OpenGL header this tree never uses.
    cmd += ["-DGLFW_INCLUDE_NONE"]
    for inc in includes:
        cmd += ["-I", inc]
    cmd += [tu_path]

    if "--print" in sys.argv:
        print("tu-gate: " + " ".join(cmd))

    proc = subprocess.run(cmd, capture_output=True, text=True)
    blob = (proc.stdout or "") + (proc.stderr or "")
    # READ THE TESTIMONY, not just the verdict. `warning:` is red. A
    # warning exits 0, which is precisely how this gate reported green
    # while clang was naming two defects in the file the round had just
    # rewritten.
    warns = [l for l in blob.split("\n") if re.search(r"\bwarning:", l)]
    return (proc.returncode == 0 and not warns), blob, len(warns), proc.returncode


def run_tier(cxx, tmp, label, tus, includes):
    """Compile a tier's TUs. Returns the list of failed TU names."""
    failed = []
    for name, shown, source in tus:
        ok, blob, nwarn, rc = compile_tu(cxx, tmp, name, source, includes)
        if not ok:
            sys.stdout.write(blob)
            why = "errors" if rc != 0 else "%d warning(s)" % nwarn
            print("tu-gate: FAIL — tier %s, %s: %s (%s)"
                  % (label, shown, why, os.path.basename(cxx)))
            failed.append(shown)
    return failed


def main() -> int:
    shallow_note()   # L29
    for path, what in ((SRC, "src/"),
                       (STUBS, "the stub dir"),
                       (WEBGPU_CPP_INC, "the vendored emdawnwebgpu C++ headers"),
                       (WEBGPU_C_INC, "the vendored emdawnwebgpu C headers")):
        if not os.path.isdir(path):
            print("tu-gate: MISSING %s at %s" % (what, path))
            if PKG in path:
                print("  The emdawnwebgpu payload is not in the tree. Tier "
                      "CARTRIDGE compiles against the PIN and never against a "
                      "system copy — fetch per "
                      "third_party/emdawnwebgpu/PINNED.md.")
            return 1

    cxx = shutil.which("clang++") or shutil.which("g++")
    if cxx is None:
        print("tu-gate: no clang++ or g++ on PATH")
        return 1

    tmp = tempfile.mkdtemp(prefix="tu_gate_")
    rc = 0
    try:
        # ── TIER CARTRIDGE — required green ─────────────────────────
        cart_inc = [SRC, STUBS, WEBGPU_CPP_INC, WEBGPU_C_INC]
        cart_failed = run_tier(cxx, tmp, "CARTRIDGE", TIER_CARTRIDGE, cart_inc)
        if cart_failed:
            print("tu-gate: FAIL — tier CARTRIDGE: %s" % ", ".join(cart_failed))
            rc = 1
        else:
            print("tu-gate: PASS — tier CARTRIDGE: %s type-check(s) as the "
                  "native program against the pinned emdawnwebgpu surface, "
                  "ZERO diagnostics [%s]"
                  % (", ".join(s for _, s, _ in TIER_CARTRIDGE),
                     os.path.basename(cxx)))

        # ── TIER CONSOLE — required green when its surface is present ─
        if not os.path.isdir(DAWN_NATIVE):
            for line in DORMANT_LINES:
                print(line)
        else:
            cons_inc = [SRC, STUBS, DAWN_NATIVE_INC]
            cons_failed = run_tier(cxx, tmp, "CONSOLE", TIER_CONSOLE, cons_inc)
            if cons_failed:
                print("tu-gate: FAIL — tier CONSOLE: %s" % ", ".join(cons_failed))
                rc = 1
            else:
                print("tu-gate: PASS — tier CONSOLE: %s type-check(s) against "
                      "third_party/dawn_native_headers, ZERO diagnostics [%s]"
                      % (", ".join(s for _, s, _ in TIER_CONSOLE),
                         os.path.basename(cxx)))
        return rc
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
