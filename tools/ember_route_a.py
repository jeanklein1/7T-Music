#!/usr/bin/env python3
"""EMBER_0 · ROUTE (a) — make DXC true on this machine, in one attended start.

    python tools\\ember_route_a.py            # DISCOVER only. Safe. Seconds.
    python tools\\ember_route_a.py --go       # the whole route. Hours. Overnight.
    python tools\\ember_route_a.py --reconcile-only   # just the manifest delta

WHY THIS SCRIPT EXISTS. EMBER_0's RECON.4 established that Dawn at pin
56f332d7 compiles EnsureDXCLibraries — the only site that opens
dxcompiler.dll and dxil.dll — inside `#if defined(DAWN_USE_BUILT_DXC)`
with no `#else`, and that C:/dev/dawn/out is built with that option OFF.
So the DXC lane has exactly one legal acquisition: rebuild the machine
Dawn with the option ON. RULING.2 stamped that rebuild as SCRIPTED,
SCHEDULED work — "a button plus a delta review" rather than a multi-hour
unknown — and this is the button.

THE FOUR THINGS IT DOES, in order:
  1. FETCH third_party/dxc AT A PINNED REVISION. Dawn's own DEPS carries
     that pin; this script reads it, PRINTS it, and records it in the
     report. An unpinned substrate would be a brand-new N10 — the fork
     pins its substrate, always — so if the pin cannot be read, the
     script STOPS rather than fetch whatever HEAD happens to be.
  2. CONFIGURE the Dawn build tree with DAWN_USE_BUILT_DXC=ON.
  3. BUILD BOTH CONFIGS. Not one. dawn_lib_optional() in this repo's
     CMakeLists treats a library present in one config and absent in the
     other as FATAL, so a Release-only rebuild would leave the VS lane's
     Debug configure broken — the trap RECON.5 named.
  4. RECONCILE the dawn_lib manifest against the rebuilt tree's ground
     truth and EMIT THE DELTA AS A REPORT, NEVER AN EDIT. Adding DXC may
     leave an import library this repo must link (dawn_native links
     dxcompiler, declared shared). Whether it does is a RULING, and
     rulings are Jean's. The reconciliation method is N6's, which found
     100 KEEP / 1 RETARGET / 2 DROP / 16 ADD the first time it ran.

WHY --go IS REQUIRED. The bare invocation discovers and reports in
seconds; --go spends hours of machine time and rewrites a build tree
outside this repo. Making the destructive half explicit costs one word
and buys a preview of exactly what will happen. That is the "attended
start" of the ruling; everything after --go is unattended.

NOTHING HERE TOUCHES 7T-MUSIC's TREE. It reads this repo's CMakeLists
and writes one report file. The Dawn checkout is the only thing it
changes, and only under --go.
"""

import argparse
import os
import re
import subprocess
import sys
import time
from datetime import datetime, timezone

CONFIGS = ("Release", "Debug")
DAWN_DIR_DEFAULT = "C:/dev/dawn"
DAWN_BUILD_DEFAULT = "C:/dev/dawn/out"
REPORT_DEFAULT = "ember_route_a_report.txt"

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

_lines = []


def say(msg=""):
    """Print and retain. An overnight run is read from the log, not watched."""
    stamp = time.strftime("%H:%M:%S")
    line = f"[{stamp}] {msg}" if msg else ""
    print(line, flush=True)
    _lines.append(line)


def stop(why, *detail):
    say()
    say("=" * 70)
    say("STOP — " + why)
    for d in detail:
        say("  " + d)
    say("=" * 70)
    say("Nothing was changed by this stage. Report the lines above.")
    write_report()
    sys.exit(1)


def run(cmd, cwd=None, what=""):
    """Run a command, streaming nothing, capturing everything, decoding UTF-8.

    encoding is pinned for the reason SUNRISE_0 N9 paid for: a default
    decode on a Windows console codepage turns a tool's own output into a
    UnicodeDecodeError and the failure gets blamed on the tool.
    """
    say(f"  $ {' '.join(cmd)}")
    try:
        p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True,
                           encoding="utf-8", errors="replace")
    except FileNotFoundError:
        return None, f"{cmd[0]} not found on PATH"
    if p.returncode != 0:
        tail = "\n".join((p.stdout or "").splitlines()[-15:]
                         + (p.stderr or "").splitlines()[-15:])
        return None, f"{what or cmd[0]} exited {p.returncode}\n{tail}"
    return p.stdout or "", None


# ── STAGE 0 · DISCOVER ───────────────────────────────────────────────

def read_dxc_pin(dawn_dir):
    """Find the dxc dependency's pinned revision in Dawn's DEPS.

    Deliberately not a DEPS parser. DEPS is Python-shaped and its schema
    is Dawn's to change; a regex that either finds a 40-hex revision on a
    dxc line or admits it could not is honest, where a half-parser that
    returns the wrong entry is not.
    """
    deps = os.path.join(dawn_dir, "DEPS")
    if not os.path.isfile(deps):
        return None, f"no DEPS at {deps}"
    text = open(deps, encoding="utf-8", errors="replace").read()
    hits = []
    for m in re.finditer(r"[^\n]*dxc[^\n]*", text, re.IGNORECASE):
        line = m.group(0)
        start = max(0, m.start() - 400)
        window = text[start:m.end() + 400]
        for rev in re.finditer(r"\b([0-9a-fA-F]{40})\b", window):
            hits.append((line.strip(), rev.group(1)))
    if not hits:
        return None, ("DEPS holds no line matching 'dxc' with a 40-hex "
                      "revision within 400 characters")
    seen, uniq = set(), []
    for line, rev in hits:
        if rev not in seen:
            seen.add(rev)
            uniq.append((line, rev))
    return uniq, None


def discover(args):
    say("=" * 70)
    say("STAGE 0 — DISCOVER (read-only)")
    say("=" * 70)

    if not os.path.isdir(args.dawn_dir):
        stop(f"no Dawn checkout at {args.dawn_dir}",
             "Pass --dawn-dir if it lives elsewhere.")

    out, err = run(["git", "-C", args.dawn_dir, "rev-parse", "HEAD"],
                   what="git rev-parse")
    if err:
        stop("could not read the Dawn checkout's revision", err)
    dawn_pin = out.strip()
    say(f"  Dawn checkout HEAD : {dawn_pin}")

    cache = os.path.join(args.dawn_build, "CMakeCache.txt")
    built_dxc = "unknown (no CMakeCache.txt — the tree is not configured)"
    if os.path.isfile(cache):
        for line in open(cache, encoding="utf-8", errors="replace"):
            if line.startswith("DAWN_USE_BUILT_DXC:"):
                built_dxc = line.strip()
                break
        else:
            built_dxc = "absent from the cache"
    say(f"  DAWN_USE_BUILT_DXC : {built_dxc}")

    dxc_dir = os.path.join(args.dawn_dir, "third_party", "dxc")
    say(f"  third_party/dxc    : {'present' if os.path.isdir(dxc_dir) else 'ABSENT'}")

    pins, err = read_dxc_pin(args.dawn_dir)
    if err:
        say(f"  dxc pin            : NOT FOUND — {err}")
    else:
        for line, rev in pins:
            say(f"  dxc pin            : {rev}")
            say(f"                       from DEPS: {line[:90]}")

    fetch = os.path.join(args.dawn_dir, "tools", "fetch_dawn_dependencies.py")
    say(f"  fetch tool         : {'present' if os.path.isfile(fetch) else 'ABSENT'}")

    say()
    say("  THE PLAN UNDER --go:")
    say("    1. fetch third_party/dxc at the pin above")
    say("    2. cmake -S <dawn> -B <out> -DDAWN_USE_BUILT_DXC=ON")
    say(f"    3. cmake --build <out> --config {CONFIGS[0]}  (then {CONFIGS[1]})")
    say("    4. reconcile this repo's dawn_lib rows against the result")
    say()
    return dawn_pin, pins, fetch


# ── STAGES 1-3 · FETCH, CONFIGURE, BUILD ─────────────────────────────

def do_fetch(args, pins, fetch):
    say("=" * 70)
    say("STAGE 1 — FETCH third_party/dxc AT ITS PIN")
    say("=" * 70)
    if pins is None:
        stop("refusing to fetch an unpinned substrate",
             "Dawn's DEPS did not yield a dxc revision, and fetching",
             "whatever HEAD happens to be would plant exactly the",
             "undeclared-generation hazard N10 was built to kill.",
             "Clone third_party/dxc by hand at a recorded revision, then",
             "re-run with --reconcile-only after building.")
    if not os.path.isfile(fetch):
        stop("Dawn's fetch tool is absent",
             f"expected {fetch}",
             "Clone third_party/dxc by hand at the pin printed above.")

    out, err = run([sys.executable, fetch, args.dawn_dir],
                   what="fetch_dawn_dependencies")
    if err:
        say("  fetch reported a failure — continuing to the existence check,")
        say("  because a partial fetch that landed dxc is still a landed dxc.")
        say("  " + err.splitlines()[0])

    dxc_dir = os.path.join(args.dawn_dir, "third_party", "dxc")
    if not os.path.isdir(dxc_dir):
        stop("third_party/dxc is still absent after the fetch",
             "Dawn's fetch script did not bring this dependency — it may be",
             "gated on the option being ON, or listed as optional in DEPS.",
             f"The pin to clone by hand is printed in STAGE 0, into {dxc_dir}.")
    say(f"  third_party/dxc present at {dxc_dir}")
    say()


def do_build(args):
    say("=" * 70)
    say("STAGE 2 — CONFIGURE Dawn with DAWN_USE_BUILT_DXC=ON")
    say("=" * 70)
    out, err = run(["cmake", "-S", args.dawn_dir, "-B", args.dawn_build,
                    "-DDAWN_USE_BUILT_DXC=ON"], what="cmake configure")
    if err:
        stop("the Dawn configure failed", err)
    for line in (out or "").splitlines():
        if "DXC" in line:
            say("  " + line.strip())
    say()

    say("=" * 70)
    say("STAGE 3 — BUILD BOTH CONFIGS (a one-config build is the trap)")
    say("=" * 70)
    for cfg in CONFIGS:
        say(f"  building {cfg} — this is the long one")
        t0 = time.time()
        _, err = run(["cmake", "--build", args.dawn_build, "--config", cfg,
                      "--parallel"], what=f"cmake --build {cfg}")
        if err:
            stop(f"the Dawn {cfg} build failed", err)
        say(f"  {cfg} done in {int(time.time() - t0) // 60} min")
    say()


# ── STAGE 4 · RECONCILE (N6's method, ground truth walked in process) ─

def walk_ground_truth(build_root):
    """Every .lib under the build tree, keyed by $<CONFIG>-templated path.

    This is `dir /s /b <out>\\*.lib` with the same two rules N6 applied:
    drop anything under a *.dir/ (MSBuild intermediates, not link inputs),
    and template the config segment so Debug and Release collapse to one
    row carrying the set of configs that hold it.
    """
    rel = {}
    for dirpath, _dirnames, filenames in os.walk(build_root):
        for fn in filenames:
            if not fn.lower().endswith(".lib"):
                continue
            full = os.path.join(dirpath, fn).replace(os.sep, "/")
            root = build_root.replace(os.sep, "/").rstrip("/")
            if not full.lower().startswith(root.lower() + "/"):
                continue
            r = full[len(root) + 1:]
            if ".dir/" in r.lower():
                continue
            parts = r.split("/")
            cfg = next((c for c in parts if c in CONFIGS), None)
            if cfg is None:
                continue
            tmpl = "/".join("$<CONFIG>" if s == cfg else s for s in parts)
            rel.setdefault(tmpl, set()).add(cfg)
    return rel


def load_entries(cmakelists):
    ents = []
    for i, line in enumerate(
            open(cmakelists, encoding="utf-8").read().split("\n"), 1):
        m = re.match(r'\s*dawn_lib(_optional)?\((\w+)\s+"([^"]+)"\s*\)', line)
        if m:
            ents.append(dict(line=i,
                             kind="optional" if m.group(1) else "required",
                             var=m.group(2), path=m.group(3),
                             lib=m.group(3).rsplit("/", 1)[-1]))
    return ents


def varname(lib):
    stem = lib[:-4] if lib.lower().endswith(".lib") else lib
    if stem.startswith("absl_"):
        return "LIB_ABSEIL_" + stem[5:].upper()
    if stem.startswith("tint_"):
        return "LIB_TINT_" + stem[5:].upper()
    if stem.startswith("dawn_"):
        return "LIB_DAWN_" + stem[5:].upper()
    return "LIB_" + re.sub(r"[^A-Za-z0-9]", "_", stem).upper()


def level_of(lib):
    s = lib[:-4] if lib.lower().endswith(".lib") else lib
    if s.startswith("absl_"):
        return 7
    if s.startswith("SPIRV"):
        return 6
    if s.startswith("tint_utils"):
        return 5
    if s.startswith("tint_lang_core"):
        return 4
    if s.startswith("tint_lang") or s.startswith("tint_api"):
        return 3
    if s.startswith("dawn_"):
        return 1
    if s.startswith("glfw"):
        return 8
    return 0


LEVEL_LABEL = {
    1: "LEVEL 1 Dawn core", 3: "LEVEL 3 Tint backends",
    4: "LEVEL 4 Tint core", 5: "LEVEL 5 Tint utils",
    6: "LEVEL 6 SPIRV-Tools", 7: "LEVEL 7 Abseil", 8: "LEVEL 8 GLFW",
    0: "*** UNCLASSIFIED — place by hand ***",
}


def reconcile(args):
    say("=" * 70)
    say("STAGE 4 — RECONCILE the dawn_lib manifest (report, never an edit)")
    say("=" * 70)

    cml = os.path.join(REPO, "CMakeLists.txt")
    rel = walk_ground_truth(args.dawn_build)
    ents = load_entries(cml)

    both = {p for p, c in rel.items() if set(CONFIGS) <= c}
    one = {p: sorted(c) for p, c in rel.items() if not set(CONFIGS) <= c}
    by_lib = {}
    for p in both:
        by_lib.setdefault(p.rsplit("/", 1)[-1], []).append(p)

    keep, retarget, drop, onecfg = [], [], [], []
    for e in ents:
        if e["path"] in both:
            keep.append(e)
        elif e["path"] in one:
            onecfg.append((e, one[e["path"]]))
        else:
            cands = by_lib.get(e["lib"], [])
            if len(cands) == 1:
                retarget.append((e, cands[0]))
            elif len(cands) > 1:
                retarget.append((e, None))       # ambiguous — flag, never pick
            else:
                drop.append(e)

    listed = {e["path"] for e in keep} | {n for _, n in retarget if n}
    adds = sorted(p for p in both if p not in listed)

    say(f"  .lib rows found, both configs : {len(both)}")
    say(f"  .lib rows found, one config   : {len(one)}   <-- flag, never choose")
    for p, cfgs in sorted(one.items()):
        say(f"      {p}   [{', '.join(cfgs)}]")
    say()
    say(f"  KEEP     {len(keep)}")
    say(f"  RETARGET {len(retarget)}")
    say(f"  DROP     {len(drop)}")
    say(f"  ADD      {len(adds)}")
    say()
    for e, n in retarget:
        say(f"  RETARGET {e['var']:<34} {e['path']}")
        say(f"           {'':<34} -> {n or '*** AMBIGUOUS — multiple candidates ***'}")
    for e in drop:
        say(f"  DROP     {e['var']:<34} {e['path']}   (absent from the tree)")
    for e, cfgs in onecfg:
        say(f"  ONE-CFG  {e['var']:<34} {e['path']}   [{', '.join(cfgs)}]")

    if adds:
        say()
        say("  NEW dawn_lib DECLARATIONS (place beside their family):")
        for p in adds:
            say(f'    dawn_lib({varname(p.rsplit("/", 1)[-1]):<34} "{p}")')
        say()
        say("  DAWN_LIBS PLACEMENT, by documented level:")
        lv = {}
        for p in adds:
            lv.setdefault(level_of(p.rsplit("/", 1)[-1]), []).append(p)
        for k in sorted(lv):
            say(f"    {LEVEL_LABEL[k]}")
            for p in lv[k]:
                say(f"      ${{{varname(p.rsplit('/', 1)[-1])}}}")

    say()
    say("  LINT")
    final = sorted(listed | set(adds))
    missing = [p for p in final if p not in both]
    say(f"    emitted entries                      {len(final)}")
    say(f"    emitted paths absent from the tree   {len(missing)}   (must be 0)")
    say(f"    emitted paths under a *.dir/         "
        f"{sum(1 for p in final if '.dir/' in p.lower())}   (must be 0)")
    for p in missing:
        say(f"      MISSING {p}")
    say()

    dxc_rows = sorted(p for p in both if "dxc" in p.lower())
    say("  THE DXC QUESTION — the reason this rebuild happened:")
    if dxc_rows:
        for p in dxc_rows:
            say(f"    {p}   <-- may need a dawn_lib row; RULING")
    else:
        say("    no dxc*.lib in either config. dawn_native links dxcompiler as")
        say("    SHARED and the DLLs are loaded through GetProcAddress, so it")
        say("    is legitimate for no import library to appear. If the exe")
        say("    then links clean, nothing is owed.")
    say()
    return len(keep), len(retarget), len(drop), len(adds), dxc_rows


def write_report(path=None):
    path = path or os.path.join(REPO, REPORT_DEFAULT)
    try:
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write("EMBER_0 · ROUTE (a) — run report\n")
            f.write(datetime.now(timezone.utc).strftime(
                "generated %Y-%m-%dT%H:%MZ\n\n"))
            f.write("\n".join(_lines) + "\n")
    except OSError as e:
        print(f"(could not write the report: {e})", flush=True)
        return None
    return path


def main():
    ap = argparse.ArgumentParser(add_help=True, description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--go", action="store_true",
                    help="actually fetch, configure and rebuild (hours)")
    ap.add_argument("--reconcile-only", action="store_true",
                    help="skip to the manifest delta against the current tree")
    ap.add_argument("--dawn-dir", default=DAWN_DIR_DEFAULT)
    ap.add_argument("--dawn-build", default=DAWN_BUILD_DEFAULT)
    args = ap.parse_args()

    say("EMBER_0 · ROUTE (a) — DXC acquisition")
    say(f"  Dawn source : {args.dawn_dir}")
    say(f"  Dawn build  : {args.dawn_build}")
    say()

    counts = None
    if args.reconcile_only:
        counts = reconcile(args)
    else:
        _pin, pins, fetch = discover(args)
        if not args.go:
            say("=" * 70)
            say("DISCOVERY ONLY. Nothing was changed.")
            say("Re-run with --go to fetch, configure and rebuild both configs.")
            say("=" * 70)
            p = write_report()
            if p:
                say(f"report: {p}")
            return
        do_fetch(args, pins, fetch)
        do_build(args)
        counts = reconcile(args)

    say("=" * 70)
    say("WHAT LANDED, AND WHAT A HUMAN MUST NOW RULE ON")
    say("=" * 70)
    if counts:
        k, r, d, a, dxc_rows = counts
        say(f"  landed : Dawn rebuilt with DXC, both configs; manifest walked")
        say(f"  ruling : {r} RETARGET · {d} DROP · {a} ADD "
            f"({k} rows need no decision)")
        if dxc_rows:
            say(f"  ruling : {len(dxc_rows)} dxc library row(s) — link or not")
        if r == 0 and d == 0 and a == 0:
            say("  ruling : NONE. The manifest already describes the rebuilt")
            say("           tree exactly. Configure and build 7T-Music, then")
            say("           set kCompilerPlan = D3D12_Dxc for UNIT.1's boot.")
    say()
    say("  NEXT, either way: UNIT.1's witness is a boot whose log shows")
    say("  `Compiler plan (request): DXC` AND use_dxc INSIDE the")
    say("  GetTogglesUsed line. Both, or the lane is not true — that pair is")
    say("  the exact negative-space of the PIVOT_0a defect. Then --probe=120.")
    p = write_report()
    if p:
        say(f"report: {p}")


if __name__ == "__main__":
    main()
