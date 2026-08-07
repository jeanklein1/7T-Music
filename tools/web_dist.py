#!/usr/bin/env python3
# ─── tools/web_dist.py ───────────────────────────────────────────
#
# SHIP_0 U4 — assemble the deployable folder and decide the host BY THE
# NUMBERS, not by preference.
#
# CC never builds, so this script is the executable half of U4: Jean
# builds, runs this, and it reports the inventory, applies the host
# rule, and writes dist/. It reads sizes off disk — nothing here is a
# remembered number.
#
#   python tools/web_dist.py            # inventory + verdict + write dist/
#   python tools/web_dist.py --check    # inventory + verdict only
#
# THE HOST RULE (SHIP_0 U4, in its own words): every file <= 25 MiB ->
# Cloudflare Pages; else -> GitHub Pages (~100 MiB/file). If neither
# fits, that is a RESOLVE — the script says so and stops rather than
# repackaging on its own authority.
#
# WebGPU requires a secure context. HTTPS is mandatory and a LAN IP is
# not it; both hosts below serve HTTPS by default, which is the whole
# reason they are the candidates.

import argparse
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
WEB = os.path.join(ROOT, "web")
DIST = os.path.join(ROOT, "dist")

# index.html is SOURCE (tracked); the other three are build output
# (.gitignore'd). All four ship.
ARTIFACTS = ["index.html", "the_board.js", "the_board.wasm", "the_board.data"]

CF_LIMIT = 25 * 1024 * 1024        # Cloudflare Pages per-file
GH_LIMIT = 100 * 1024 * 1024       # GitHub Pages per-file (soft, ~100 MiB)


def mib(n):
    return n / (1024.0 * 1024.0)


def main():
    ap = argparse.ArgumentParser(description="SHIP_0 U4 — web dist assembly + host verdict")
    ap.add_argument("--check", action="store_true", help="inventory and verdict only; write nothing")
    args = ap.parse_args()

    missing = [f for f in ARTIFACTS if not os.path.isfile(os.path.join(WEB, f))]
    present = [f for f in ARTIFACTS if f not in missing]

    print("web output inventory  (%s)" % WEB)
    print("  %-18s %14s  %9s" % ("file", "bytes", "MiB"))
    total = 0
    sizes = {}
    for f in present:
        n = os.path.getsize(os.path.join(WEB, f))
        sizes[f] = n
        total += n
        print("  %-18s %14d  %9.2f" % (f, n, mib(n)))
    for f in missing:
        print("  %-18s %14s  %9s" % (f, "MISSING", "-"))
    print("  %-18s %14d  %9.2f" % ("TOTAL", total, mib(total)))

    if missing:
        print("")
        print("BUILD FIRST — %d artifact(s) absent." % len(missing))
        print("  cmake --preset the-board-web && cmake --build --preset the-board-web")
        print("(the three build outputs land in web/ beside the tracked index.html;")
        print(" .gitignore keeps them out of the tree on purpose.)")
        return 2

    # THE DOWNLOAD COST, called out on its own line because it is the
    # mobile number that matters and U4 asks for it explicitly. Recorded,
    # NOT optimized — Jean's directive parks that until the phone verdict.
    data = sizes["the_board.data"]
    print("")
    print("MOBILE DOWNLOAD COST")
    print("  the_board.data   %d bytes  (%.2f MiB)   <- the preloaded assets + world.wgsl" % (data, mib(data)))
    print("  wasm + js        %d bytes  (%.2f MiB)" % (
        sizes["the_board.wasm"] + sizes["the_board.js"],
        mib(sizes["the_board.wasm"] + sizes["the_board.js"])))
    print("  first visit      %d bytes  (%.2f MiB) uncompressed" % (total, mib(total)))
    print("  Both hosts serve br/gzip for js/html; .wasm and .data compress well over the")
    print("  wire, so the number a phone actually pays is lower than the figure above.")
    print("  RECORD IT. Do not optimize it (SHIP_0 U4).")

    biggest = max(sizes, key=lambda k: sizes[k])
    print("")
    print("HOST VERDICT (by the numbers)")
    print("  largest single file: %s at %.2f MiB" % (biggest, mib(sizes[biggest])))
    if sizes[biggest] <= CF_LIMIT:
        print("  -> CLOUDFLARE PAGES. Every file is within its 25 MiB per-file limit.")
        host = "cloudflare"
    elif sizes[biggest] <= GH_LIMIT:
        print("  -> GITHUB PAGES. %s exceeds Cloudflare's 25 MiB; GitHub's ~100 MiB holds."
              % biggest)
        host = "github"
    else:
        print("  -> RESOLVE. %s exceeds BOTH limits (%.2f MiB)." % (biggest, mib(sizes[biggest])))
        print("     Not repackaging on my own authority. Options, for Jean's ruling:")
        print("       a. split the preload — ship a starter asset set, fetch the rest at runtime")
        print("       b. drop the preload and fetch assets over HTTP (needs a loader path)")
        print("       c. a host with no per-file cap (S3/R2 + CDN)")
        return 3

    if args.check:
        print("")
        print("(--check: nothing written)")
        return 0

    if os.path.isdir(DIST):
        shutil.rmtree(DIST)
    os.makedirs(DIST)
    for f in ARTIFACTS:
        shutil.copy2(os.path.join(WEB, f), os.path.join(DIST, f))
    print("")
    print("WROTE %s  (%d files)" % (DIST, len(ARTIFACTS)))

    print("")
    print("DEPLOY — exact commands")
    print("")
    if host == "cloudflare":
        print("  Cloudflare Pages, first time:")
        print("    npm install -g wrangler")
        print("    wrangler login")
        print("    wrangler pages project create the-board --production-branch main")
        print("    wrangler pages deploy dist --project-name the-board")
        print("")
        print("  Every time after:")
        print("    wrangler pages deploy dist --project-name the-board")
        print("")
        print("  Or without the CLI: dash.cloudflare.com -> Workers & Pages -> Create ->")
        print("  Pages -> Upload assets -> drag the dist/ FOLDER in. The URL is")
        print("  https://the-board.pages.dev and it is the QR destination.")
    else:
        print("  GitHub Pages, gh-pages branch convention: create an orphan branch that")
        print("  holds ONLY the deployable files, push it, and point Pages at its root")
        print("  (Settings -> Pages -> Source: Deploy from a branch -> gh-pages / (root)).")
        print("  From a clean tree:")
        print("    git switch --orphan gh-pages")
        print("    git rm -rf . >NUL 2>&1")
        print("    cp dist/* .            # or: xcopy dist\\* . /Y  on cmd")
        print("    git add -A && git commit -m \"web build\"")
        print("    git push -u origin gh-pages")
        print("    git switch -")
        print("  The URL is https://jeanklein1.github.io/7T-Pawns/ .")
    print("")
    print("  HEADERS: none. The build is single-threaded, so no COOP/COEP is needed")
    print("  and adding them would only risk breaking the load.")
    print("")
    print("  HTTPS is not optional — WebGPU needs a secure context. Both hosts give it;")
    print("  a LAN IP does not, which is why 'just serve it locally' is not a phone test.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
