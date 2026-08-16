#!/bin/sh
# G-LAW 1 — THE STANDALONE COMPILE (LADDER-6 FIX). Runs the whole real
# cartridge TU through a conforming compiler with only the absent SDK
# surface stubbed. Exit 0 = the tree parses, scopes, and looks up clean.
#
# ── GATE_1 REPAIR (2026-08-16) ────────────────────────────────────────
# This gate used to compile against gen_stubs.py's VACUOUS webgpu_cpp.h —
# a stub whose types accept anything and whose enums hold nothing. It had
# been failing on 24 errors for so long that the failure was treated as
# the baseline and the gate was read differentially, which is another way
# of saying it had stopped answering. Every one of those 24 was the STUB's
# gap, not the tree's: pointed at the real header the same TU is clean.
#
# It compiles against the vendored emdawnwebgpu payload now — the same
# pinned generation the web build uses (third_party/emdawnwebgpu/PINNED.md,
# F5F) and the same one the console gate uses — so glaw1 is a pass/fail
# gate again rather than a diff to be interpreted. The Emscripten C surface
# is shared with the console gate rather than duplicated: one stub set, one
# home (tools/gates/console_gate/stubs, each file bannered).
#
# SUNSET_1 is why the Emscripten stubs are needed here at all. With the
# native arms deleted, gallery.hpp's <emscripten.h> and <emscripten/fetch.h>
# are unconditional, so the cartridge TU reaches them on every platform —
# there is one program now, and it is the web one.
#
# The BOUNDARY is unchanged and still stated in tu.cpp: this harness
# certifies OUR names, scope and syntax. Nothing is linked, nothing runs,
# and the web boot remains the witness of record (ATLAS_1revB).
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
PKG="$ROOT/third_party/emdawnwebgpu/emdawnwebgpu_pkg"
STUBS="$ROOT/tools/gates/console_gate/stubs"

if [ ! -f "$PKG/webgpu_cpp/include/webgpu/webgpu_cpp.h" ]; then
    echo "G-LAW 1: the emdawnwebgpu payload is missing at $PKG."
    echo "  This gate compiles against the PIN and never a system or emsdk"
    echo "  copy. Fetch per third_party/emdawnwebgpu/PINNED.md."
    exit 1
fi

CXX="$(command -v clang++ || command -v g++)"
if [ -z "$CXX" ]; then echo "G-LAW 1: no clang++ or g++ on PATH"; exit 1; fi

"$CXX" -std=gnu++20 -w -fsyntax-only -DGLFW_INCLUDE_NONE \
    -I "$ROOT/src" \
    -I "$STUBS" \
    -I "$PKG/webgpu_cpp/include" \
    -I "$PKG/webgpu/include" \
    "$HERE/tu.cpp"
echo "G-LAW 1: GREEN"
