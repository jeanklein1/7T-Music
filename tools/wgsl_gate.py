#!/usr/bin/env python3
# THE WGSL GATE — naga reads the real module.
#
# world.wgsl carries no `requires` directive and no immediate address space,
# so naga parses, scopes and validates the module as shipped. Exit 0 only if
# naga accepts it; an absent naga is a FAILED gate, never a pass (P1).
#
# WHAT IT DOES NOT PROVE. Pipeline-layout conformance, minBindingSize and
# every runtime fact past the type surface: the web boot is the witness of
# record (ATLAS_1revB). A Tint arm lights when T7_TINT names a tint
# executable; there is none in the container and none in the pinned payload.
#
# USAGE
#   python3 tools/wgsl_gate.py        # gate; exit 1 on failure
import os, re, shutil, subprocess, sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORLD_WGSL = os.path.join(REPO, "src", "cartridges", "the_board",
                          "realization", "world.wgsl")

# The LANE, not the English word. `immediate radius` is ordinary prose in
# §3 and predates this gate; only these two tokens are the address space.
LANE = re.compile(r"var<immediate>|immediate_address_space")


def shallow_note():
    """L29 — the environment announces its own depth."""
    if os.path.exists(os.path.join(REPO, ".git", "shallow")):
        print("  [gate] NOTE: shallow clone — history-derived claims are "
              "unsafe until git fetch --unshallow")


def run(tool, path):
    r = subprocess.run([tool, path], capture_output=True, text=True)
    blob = (r.stdout or "") + (r.stderr or "")
    bad = r.returncode != 0 or "error" in blob.lower() or "could not parse" in blob.lower()
    return bad, blob


def main():
    shallow_note()
    if not os.path.exists(WORLD_WGSL):
        print("wgsl-gate: FAIL — %s not found" % WORLD_WGSL)
        return 1
    naga = shutil.which("naga") or os.path.expanduser("~/.cargo/bin/naga")
    if not os.path.exists(naga) and not shutil.which("naga"):
        print("wgsl-gate: FAIL — naga not on PATH and not at ~/.cargo/bin/naga "
              "(cargo install naga-cli). An unrunnable gate is a failed gate.")
        return 1
    with open(WORLD_WGSL, encoding="utf-8") as fh:
        src = fh.read()
    hit = LANE.search(src)
    if hit:
        print("wgsl-gate: FAIL — the module names the immediate lane again "
              "(`%s` in world.wgsl); this gate reads the raw module only"
              % hit.group(0))
        return 1
    bad, blob = run(naga, WORLD_WGSL)
    if bad:
        print("wgsl-gate: FAIL — naga rejected world.wgsl")
        sys.stdout.write(blob)
        return 1
    print("wgsl-gate: PASS — world.wgsl parses, scopes and validates under naga, raw")
    tint = os.environ.get("T7_TINT", "")
    if tint and os.path.isfile(tint):
        tbad, tblob = run(tint, WORLD_WGSL)
        if tbad:
            print("wgsl-gate: FAIL — tint rejected world.wgsl")
            sys.stdout.write(tblob)
            return 1
        print("  [gate] tint arm PASS (%s)" % tint)
    else:
        print("  [gate] tint arm DORMANT — set T7_TINT to a tint executable to light it")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        try:
            sys.stdout.close()
        except Exception:
            pass
        sys.exit(0)
