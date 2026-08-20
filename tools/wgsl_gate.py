#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE WGSL GATE (PROBATE_E3, retired to plain naga by REGAIN_1)
#
# WHAT THIS GATE DOES. It runs naga over `world.wgsl`. That is the whole
# of it: one file, one compiler, no rewriting in between.
#
# WHY THAT SENTENCE IS NEW. PROBATE_E3 could not write it. The module
# carried `requires immediate_address_space;`, and naga 30 rejects that
# directive as an unknown language extension — it stopped at line one and
# read nothing. So this file carried a PINNED TRANSFORM: the directive
# commented out, each `var<immediate>` rewritten to a uniform seat at
# @group(0) @binding(900+x), and naga run on the result. It gated
# everything about the module EXCEPT the address space it rewrote away,
# and that hole was structural — an immediates-specific error was
# invisible here BY CONSTRUCTION, and the boot was its only witness.
#
# REGAIN_1 RETIRED THE IMMEDIATES LANE. `patch_params` and `shadow_slot`
# are dynamic-offset uniforms now, the directive is gone, and the module
# is core WGSL with no language extension in it. There is nothing left to
# rewrite, so the transform, its SHIM_TAG, its base binding and the ARM 4
# watcher that waited on naga shipping the extension all go with it. This
# file's own banner asked for this retirement by name on 2026-08-16; this
# is it.
#
# THE GATE'S OWN BLIND SPOT IS CLOSED. What naga reads now is the file
# the browser is served, byte for byte.
#
# WHAT STILL IS NOT GATED HERE, and never was this file's to gate: naga
# witnesses the MODULE. Pipeline-layout conformance, minBindingSize and
# dynamic-offset alignment are Dawn's checks at pipeline creation, so the
# web boot witnesses those and no source-level tool can. `world.wgsl`'s
# COMPILER FLOOR block names that boundary; it is unchanged by this
# retirement and is a different fact from the hole that just closed.
#
# USAGE
#   python3 tools/wgsl_gate.py        # gate; exit 1 on failure
#
# Exit status is 0 if naga accepts the module, 1 otherwise (including
# naga being absent — an unrunnable gate reports as a failed gate, never
# as a pass; P1, and the whole reason this file exists is a gate that
# failed open unnoticed for a campaign).
# ═══════════════════════════════════════════════════════════════════════

import os
import shutil
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORLD_WGSL = os.path.join(REPO, "src", "cartridges", "the_board",
                          "realization", "world.wgsl")


def shallow_note():
    """L29 — the environment announces its own depth.

    A shallow clone answers "absent" for everything below its graft, and
    that answer is indistinguishable from the truth from inside. Every
    gate that touches git history prints this before it speaks, so
    nobody has to remember to ask. One line, no cost, no judgement.
    """
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    if os.path.exists(os.path.join(root, ".git", "shallow")):
        print("  [gate] NOTE: shallow clone — history-derived claims are "
              "unsafe until git fetch --unshallow")


def rejected(proc):
    """naga-cli reports parse/validation errors on stdout/stderr while
    still exiting 0 in some builds, so BOTH are consulted: any
    diagnostic text is a failure, whatever the exit status says."""
    blob = (proc.stdout or "") + (proc.stderr or "")
    bad = proc.returncode != 0 or "error" in blob.lower() \
        or "could not parse" in blob.lower()
    return bad, blob


def main():
    shallow_note()   # L29
    if not os.path.exists(WORLD_WGSL):
        print("wgsl-gate: FAIL — %s not found" % WORLD_WGSL)
        return 1

    naga = shutil.which("naga") or os.path.expanduser("~/.cargo/bin/naga")
    if not os.path.exists(naga) and not shutil.which("naga"):
        print("wgsl-gate: FAIL — naga not on PATH and not at ~/.cargo/bin/naga.")
        print("  Install with `cargo install naga-cli`. An unrunnable gate")
        print("  reports as FAILED, never as passed (P1) — the whole reason")
        print("  this file exists is a gate that failed open unnoticed.")
        return 1

    bad, blob = rejected(subprocess.run([naga, WORLD_WGSL],
                                        capture_output=True, text=True))
    if bad:
        print("wgsl-gate: FAIL — naga rejected world.wgsl")
        sys.stdout.write(blob)
        return 1
    print("wgsl-gate: PASS — world.wgsl parses, scopes and validates under "
          "naga, whole and unmodified")
    print("  no shim: the module naga read is the module the browser is "
          "served (REGAIN_1 retired the transform with the immediates lane)")
    print("  NOT gated here: pipeline-layout conformance, minBindingSize and "
          "dynamic-offset alignment — Dawn's checks at pipeline creation, "
          "witnessed by the web boot (world.wgsl COMPILER FLOOR block).")

    # ═══ THE TINT ARM (PROBATE_CLOSE3) ════════════════════════════════
    #
    # Kept, and its reason restated. PROBATE_CLOSE3 lit this arm to close
    # the shim's hole — Tint spoke the extension naga did not, so it read
    # the real module. That hole is closed by the module itself now, and
    # this arm's remaining value is a different and smaller one: Tint is
    # the compiler family every supported browser actually runs, so a
    # Tint pass sits one step closer to the boot than a naga pass does.
    #
    # SHIPPED DORMANT. There is no Tint in this container and none in the
    # pinned emdawnwebgpu payload (SEAL EF4 — the package is headers and
    # JS, with no build system). The archived Dawn checkout can build one;
    # until someone spends that hour the arm waits, NAMED. A skip that
    # says its name, never a silent pass — the same law §G1's dev-serve
    # skip answers to.
    tint = os.environ.get("T7_TINT", "")
    if tint and os.path.isfile(tint):
        t = subprocess.run([tint, WORLD_WGSL], capture_output=True, text=True)
        tblob = (t.stdout or "") + (t.stderr or "")
        if t.returncode != 0 or "error" in tblob.lower():
            print("wgsl-gate: FAIL — tint rejected world.wgsl")
            sys.stdout.write(tblob)
            return 1
        print("  [gate] tint arm PASS — the same module, accepted by the "
              "shipping compiler family (%s)" % tint)
    else:
        print("  [gate] tint arm DORMANT — set T7_TINT to a tint executable "
              "to light it")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        # `wgsl_gate.py | head` closes stdout mid-print, and Python's
        # default response is a traceback — which, printed by a GATE,
        # reads exactly like the gate falling over. Exit quietly instead:
        # the verdict is the exit status, and a reader who piped us to
        # `head` asked for the first lines, not for a stack trace.
        try:
            sys.stdout.close()
        except Exception:
            pass
        sys.exit(0)
