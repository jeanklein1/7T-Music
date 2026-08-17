#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE WGSL GATE (PROBATE_E3) — the per-commit gate of record
#
# WHY THIS EXISTS. The `world.wgsl` COMPILER FLOOR block names naga as
# the per-commit gate. As of naga 30, naga cannot read this program at
# all: it rejects `requires immediate_address_space;` as an unknown
# language extension and stops at line one of the module. Its WGSL front
# end carries no `push_constant` address space either, so there is not
# even a near-synonym to rewrite to. PROBATE's F5 caught this, and
# caught the important half with it — the rejection is NOT new. naga
# refuses the module at `3e18c39`, before PROBATE touched anything, and
# has refused it since DOMESDAY_2 F3-a put the directive in. A gate that
# has been failing-open for a whole campaign is worse than no gate,
# because the campaign believed it had one.
#
# WHAT THIS GATE DOES. One deterministic transform, then naga:
#
#   1. `requires immediate_address_space;` is commented out.
#   2. Each `var<immediate> N: T;` becomes
#      `@group(0) @binding(900+x) var<uniform> N: T;`, x enumerated
#      from 0 in source order.
#
# and the transformed module goes to naga, which parses, scopes and
# validates it whole.
#
# WHAT IT PROVES. Everything about the module except the immediate
# address space itself: syntax, scoping, name resolution, types,
# uniformity, entry-point signatures, workgroup attributes — over every
# line of the real file, with two declarations wearing a different
# address space.
#
# WHAT IT DOES NOT PROVE. The address space it rewrites away. An
# immediates-specific error — a struct illegal as an immediate, a second
# `var<immediate>` reachable from one entry point (WGSL §14.3), a size
# past `maxImmediateSize` — is invisible here BY CONSTRUCTION, and the
# boot is its only witness. That is one generation wider than the blind
# spot `world.wgsl`'s banner already names, and the banner now says so.
#
# THE TRANSFORM IS THE GATE'S PINNED HALF. It is in the tree, in one
# file, with no options and no flags, so every run of this gate is the
# same run. An ad-hoc shim retyped per campaign would be a gate whose
# definition drifts — which is how the naga clause went stale in the
# first place.
#
# RETIRE THIS FILE the day naga reads `immediate_address_space`
# directly: the gate becomes `naga world.wgsl` and this transform
# becomes history. PROBATE_E3's probe records where that stood on
# 2026-08-16.
#
# USAGE
#   python3 tools/wgsl_gate.py        # gate; exit 1 on failure
#
# Exit status is 0 if naga accepts the transformed module, 1 otherwise
# (including naga being absent — an unrunnable gate reports as a failed
# gate, never as a pass).
# ═══════════════════════════════════════════════════════════════════════

import io
import os
import re
import shutil
import subprocess
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORLD_WGSL = os.path.join(REPO, "src", "cartridges", "the_board",
                          "realization", "world.wgsl")

DIRECTIVE = "requires immediate_address_space;"
SHIM_TAG = "// [wgsl_gate shim] "
IMMEDIATE_BASE_BINDING = 900


def transform(src):
    """The pinned transform. Deterministic; same bytes in, same out."""
    out = src.replace(DIRECTIVE, SHIM_TAG + DIRECTIVE)
    seen = []

    def sub(m):
        binding = IMMEDIATE_BASE_BINDING + len(seen)
        seen.append(m.group(1))
        return ("@group(0) @binding(%d) var<uniform> %s: %s;"
                % (binding, m.group(1), m.group(2)))

    out = re.sub(r"var<immediate>\s+(\w+)\s*:\s*([^;]+);", sub, out)
    return out, seen


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

    src = io.open(WORLD_WGSL, encoding="utf-8").read()
    shimmed, immediates = transform(src)

    if DIRECTIVE not in src:
        print("wgsl-gate: note — `%s` is no longer in the module. If the")
        print("  immediates lane is gone, retire this gate for plain naga." % DIRECTIVE)

    tmp = tempfile.mkdtemp(prefix="wgsl_gate_")
    try:
        shim_path = os.path.join(tmp, "world_shimmed.wgsl")
        io.open(shim_path, "w", encoding="utf-8", newline="\n").write(shimmed)
        r = subprocess.run([naga, shim_path], capture_output=True, text=True)
        # naga-cli reports parse/validation errors on stdout/stderr while
        # still exiting 0 in some builds, so BOTH are consulted: any
        # diagnostic text is a failure, whatever the exit status says.
        blob = (r.stdout or "") + (r.stderr or "")
        bad = r.returncode != 0 or "error" in blob.lower() \
            or "could not parse" in blob.lower()
        if bad:
            print("wgsl-gate: FAIL — naga rejected the transformed module")
            print("  (transform: %d immediate(s) rewritten to uniform seats "
                  "at @group(0) @binding(%d..); the `requires` directive "
                  "commented out)"
                  % (len(immediates), IMMEDIATE_BASE_BINDING))
            sys.stdout.write(blob)
            return 1
        print("wgsl-gate: PASS — world.wgsl parses, scopes and validates under "
              "naga through the pinned immediate shim")
        print("  shimmed: %s"
              % (", ".join("%s -> @group(0) @binding(%d)"
                           % (n, IMMEDIATE_BASE_BINDING + i)
                           for i, n in enumerate(immediates)) or "(none)"))
        print("  NOT gated here: the immediate address space itself — the web "
              "boot is its only witness (world.wgsl COMPILER FLOOR block).")

        # ═══ ARM 3 — TINT, THE SHIPPING COMPILER (PROBATE_CLOSE3) ═════
        #
        # The shim arm above gates everything EXCEPT the address space it
        # rewrites away, and that hole is the whole reason the banner
        # names the boot as witness of record. Tint closes it: Tint
        # speaks `immediate_address_space`, so it reads the REAL module —
        # `requires`, `var<immediate>` and all — and it is the same
        # compiler family every supported browser runs.
        #
        # SHIPPED DORMANT. There is no Tint in this container and none in
        # the pinned emdawnwebgpu payload (SEAL EF4 — the package is
        # headers and JS, with no build system). The archived Dawn
        # checkout can build one; until someone spends that hour the arm
        # waits, NAMED. A skip that says its name, never a silent pass —
        # the same law §G1's dev-serve skip answers to.
        tint = os.environ.get("T7_TINT", "")
        if tint and os.path.isfile(tint):
            raw_path = os.path.join(tmp, "world_raw.wgsl")
            io.open(raw_path, "w", encoding="utf-8", newline="\n").write(src)
            t = subprocess.run([tint, raw_path], capture_output=True, text=True)
            tblob = (t.stdout or "") + (t.stderr or "")
            tbad = t.returncode != 0 or "error" in tblob.lower()
            if tbad:
                print("wgsl-gate: FAIL — tint rejected the RAW module "
                      "(no shim; the real address space)")
                sys.stdout.write(tblob)
                return 1
            print("  [gate] tint arm PASS — the RAW module, immediates and all, "
                  "accepted by the shipping compiler family (%s)" % tint)
        else:
            print("  [gate] tint arm DORMANT — set T7_TINT to a tint executable "
                  "to light it")

        # ═══ ARM 4 — THE NAGA TRIPWIRE (PROBATE_CLOSE4) ═══════════════
        #
        # The Firefox question, watched by the tree instead of by
        # somebody's memory. naga is run once more against the RAW
        # module. Today it fails, and that failure is EXPECTED and
        # non-fatal — it is the directive being unknown, which is the
        # entire reason the shim exists and the banner says PENDING.
        #
        # The day it PASSES, naga has shipped the extension, and the
        # gate says so unprompted. A PENDING line nobody is watching
        # decays into a permanent; this is the watch, and it costs one
        # naga invocation per run.
        raw_path = os.path.join(tmp, "world_raw_naga.wgsl")
        io.open(raw_path, "w", encoding="utf-8", newline="\n").write(src)
        rn = subprocess.run([naga, raw_path], capture_output=True, text=True)
        rblob = (rn.stdout or "") + (rn.stderr or "")
        raw_rejected = rn.returncode != 0 or "error" in rblob.lower() \
            or "could not parse" in rblob.lower()
        if raw_rejected:
            print("  [gate] naga-raw REJECTS the module, as expected — "
                  "immediate_address_space is still unknown to it; "
                  "Firefox stays PENDING (world.wgsl banner)")
        else:
            print("  [gate] NOTE: naga now accepts immediate_address_space — "
                  "the Firefox PENDING line is ready for its boot witness "
                  "(world.wgsl banner).")
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        # `wgsl_gate.py | head` closes stdout mid-print, and Python's
        # default response is a traceback — which, printed by a GATE,
        # reads exactly like the gate falling over. The arms added at
        # PROBATE_CLOSE put more lines after the PASS block and made
        # this easy to hit. Exit quietly instead: the verdict is the
        # exit status, and a reader who piped us to `head` asked for
        # the first lines, not for a stack trace.
        try:
            sys.stdout.close()
        except Exception:
            pass
        sys.exit(0)
