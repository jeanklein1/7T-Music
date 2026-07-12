#!/bin/sh
# G-LAW 1 — THE STANDALONE COMPILE (LADDER-6 FIX). Runs the whole real
# cartridge TU through a conforming compiler with only the absent SDK
# surface stubbed. Exit 0 = the tree parses, scopes, and looks up clean.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
python3 "$HERE/gen_stubs.py"
g++ -std=c++20 -w -fsyntax-only -I "$HERE/../../../src" -I "$HERE/stubs" "$HERE/tu.cpp"
echo "G-LAW 1: GREEN"
