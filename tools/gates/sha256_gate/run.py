#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE SHA256 GATE (PROBATE_SEAL2) — both ends compute the same function
#
# WHY THIS EXISTS. The serve witness compares a digest computed by
# `tools/web_dist.py` (Python's hashlib) against one computed by the
# program at boot (src/core/sha256.hpp). If those two disagree by even a
# padding rule, the witness reports MISMATCH on a perfectly good serve —
# and a witness that cries wolf gets disabled, which is how the corridor
# went unwatched in the first place.
#
# So the agreement is not asserted, it is WITNESSED: this gate compiles
# the header, runs it over real inputs including the actual world.wgsl,
# and holds every digest against hashlib's.
#
# The vectors: the two FIPS 180-4 examples, the empty string, and the
# three lengths where SHA-256's padding rule changes behaviour (55, 56,
# 64 bytes — the block that just fits, the one that spills, the exact
# block). Then world.wgsl itself, which is the only input that matters
# in production and the one whose length no test vector predicts.
#
# USAGE
#   python3 tools/gates/sha256_gate/run.py        # gate; exit 1 on failure
# ═══════════════════════════════════════════════════════════════════════
"""PROBATE_SEAL2: hold src/core/sha256.hpp to hashlib, over real bytes."""

import hashlib
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
HEADER = os.path.join(REPO, "src", "core", "sha256.hpp")
WORLD = os.path.join(REPO, "src", "cartridges", "the_board", "realization",
                     "world.wgsl")

DRIVER = r'''
#include "sha256.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
int main(int argc, char** argv) {
    if (argc > 1) {                       // hash a FILE, byte for byte
        std::ifstream f(argv[1], std::ios::binary);
        std::stringstream b; b << f.rdbuf();
        std::string s = b.str();
        std::printf("%s\n", t7::sha256_hex(s).c_str());
        return 0;
    }
    std::string line;                     // else: one hex digest per stdin line
    while (std::getline(std::cin, line))
        std::printf("%s\n", t7::sha256_hex(line).c_str());
    return 0;
}
'''

VECTORS = [
    b"",
    b"abc",
    b"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    b"a" * 55,   # the last length that fits its own padding block
    b"a" * 56,   # the first that spills into a second block
    b"a" * 64,   # exactly one block
    b"a" * 1000,
]


def main():
    cxx = shutil.which("clang++") or shutil.which("g++")
    if not cxx:
        print("sha256-gate: FAIL — no clang++ or g++ on PATH")
        return 1
    for p in (HEADER, WORLD):
        if not os.path.exists(p):
            print("sha256-gate: FAIL — missing %s" % p)
            return 1

    tmp = tempfile.mkdtemp(prefix="sha256_gate_")
    try:
        src = os.path.join(tmp, "driver.cpp")
        with open(src, "w", encoding="utf-8") as fh:
            fh.write(DRIVER)
        exe = os.path.join(tmp, "driver")
        c = subprocess.run([cxx, "-std=c++20", "-O1", "-I",
                            os.path.dirname(HEADER), src, "-o", exe],
                           capture_output=True, text=True)
        if c.returncode != 0:
            print("sha256-gate: FAIL — %s did not compile the header" % cxx)
            sys.stdout.write(c.stdout + c.stderr)
            return 1

        # Line-fed vectors (no embedded newlines, so getline round-trips).
        feed = [v for v in VECTORS if b"\n" not in v]
        r = subprocess.run([exe], input=b"\n".join(feed) + b"\n",
                           capture_output=True)
        got = r.stdout.decode().split()
        bad = 0
        for v, g in zip(feed, got):
            want = hashlib.sha256(v).hexdigest()
            if g != want:
                bad += 1
                print("  MISMATCH on %d-byte vector: got %s want %s"
                      % (len(v), g, want))
        if len(got) != len(feed):
            print("  MISMATCH — %d digests for %d vectors" % (len(got), len(feed)))
            bad += 1

        # The input that actually ships.
        r = subprocess.run([exe, WORLD], capture_output=True)
        got_world = r.stdout.decode().strip()
        with open(WORLD, "rb") as fh:
            raw = fh.read()
        want_world = hashlib.sha256(raw).hexdigest()
        if got_world != want_world:
            bad += 1
            print("  MISMATCH on world.wgsl: got %s want %s"
                  % (got_world, want_world))

        if bad:
            print("sha256-gate: FAIL — %d disagreement(s) with hashlib" % bad)
            return 1
        print("sha256-gate: PASS — src/core/sha256.hpp agrees with hashlib on "
              "%d vectors and on world.wgsl (%d bytes, sha=%s) [%s]"
              % (len(feed), len(raw), want_world[:8], os.path.basename(cxx)))
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
