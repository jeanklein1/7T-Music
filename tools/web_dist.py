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
# EXHIBIT_0 U1 — THE EXHIBITION LEAVES THE BUNDLE. The four build files
# above are THE PROGRAM: rebuilt only when the program changes. The
# paintings, the soundtrack, and exhibition.json are THE EXHIBITION:
# plain files beside the program, deployed alone when the exhibition
# changes. Daily curation must never wake the compiler — which is why
# they are assembled here, by this script, and not by --preload-file.
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
import json
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
WEB = os.path.join(ROOT, "web")
DIST = os.path.join(ROOT, "dist")

SRC_PAINTINGS = os.path.join(ROOT, "assets", "paintings")
SRC_MUSIC = os.path.join(ROOT, "assets", "music")
DIST_PAINTINGS = os.path.join(DIST, "paintings")
DIST_MUSIC = os.path.join(DIST, "music")

# index.html is SOURCE (tracked); the other three are build output
# (.gitignore'd). All four ship.
ARTIFACTS = ["index.html", "the_board.js", "the_board.wasm", "the_board.data"]

# THE SAME NUMBER THE PROGRAM STAGES AT. The authored loader scales every
# painting to fit Dim::PAINTING_RESOLUTION (src/cartridges/the_board/
# realization/state.hpp) and pads to that square before upload, and it
# never scales UP (`if (scale > 1.0f) scale = 1.0f`). A long side above
# this value is therefore bytes over the wire that the GPU throws away.
# Capping here is the only place that saves them. If PAINTING_RESOLUTION
# moves, this moves with it.
PAINTING_CAP = 512
PAINTING_QUALITY = 82

# THE OTHER NUMBER THE PROGRAM PINS, and unlike PAINTING_CAP this one is
# a CEILING rather than a target. rotate_authored_staging (src/cartridges/
# the_board/bodies/gallery.hpp) tracks which paintings are already in a
# staging slot with `bool disk_in_use[256]`, and every read of it is
# guarded `disk_idx < 256 && disk_in_use[disk_idx]` — so a manifest index
# at or past the array does not overflow it, it SHORT-CIRCUITS to "not in
# use". The dedupe fails open.
#
# It was a safe assumption while the manifest was a directory this repo
# owned. EXHIBIT_0 made the manifest a deploy-time input, so the number
# is worth stating on the side that can actually count the files. It is
# declared here to be compared, not to be enforced: the array in
# gallery.hpp is the source, this is its echo, and they move together.
MANIFEST_DEDUPE_CAP = 256

# The scan the program does, restated: name starts PAINTING_, extension
# .jpg or .jpeg case-insensitively (gallery.hpp, scan_paintings_folder).
PAINTING_EXTS = (".jpg", ".jpeg")

# exhibition.json, NOT manifest.json — the root manifest.json name stays
# reserved for the PWA web manifest.
EXHIBITION_JSON = "exhibition.json"

CF_LIMIT = 25 * 1024 * 1024        # Cloudflare Pages per-file
GH_LIMIT = 100 * 1024 * 1024       # GitHub Pages per-file (soft, ~100 MiB)


def mib(n):
    return n / (1024.0 * 1024.0)


def extract_number(name):
    """The C++ sort key, restated in Python.

    gallery.hpp takes the stem, finds the FIRST '_', and hands the rest to
    std::stoi — which reads leading digits and stops, and whose throw is
    caught and turned into 0. `int(...)` would reject "1_v2" where stoi
    returns 1, so the leading-digit match is the faithful form.
    """
    stem = os.path.splitext(name)[0]
    pos = stem.find("_")
    if pos == -1 or pos + 1 >= len(stem):
        return 0
    m = re.match(r"\s*([+-]?\d+)", stem[pos + 1:])
    return int(m.group(1)) if m else 0


def list_paintings():
    """Source paintings in the program's numeric order (PAINTING_2 < PAINTING_10)."""
    if not os.path.isdir(SRC_PAINTINGS):
        return []
    names = [
        f for f in os.listdir(SRC_PAINTINGS)
        if os.path.isfile(os.path.join(SRC_PAINTINGS, f))
        and f.startswith("PAINTING_")
        and os.path.splitext(f)[1].lower() in PAINTING_EXTS
    ]
    # Name is the tiebreak so the order is total, not merely numeric —
    # std::sort is unstable and the browser re-sorts on arrival anyway,
    # but a manifest that reorders between runs is noise in a diff.
    names.sort(key=lambda f: (extract_number(f), f))
    return names


def list_music():
    if not os.path.isdir(SRC_MUSIC):
        return []
    names = [
        f for f in os.listdir(SRC_MUSIC)
        if os.path.isfile(os.path.join(SRC_MUSIC, f))
        and os.path.splitext(f)[1].lower() == ".mp3"
    ]
    names.sort()
    return names


def dir_bytes(paths):
    return sum(os.path.getsize(p) for p in paths if os.path.isfile(p))


def write_paintings(names):
    """Re-encode into dist/paintings/, capped and requantized.

    THE OPTIONAL MUST NOT BE ABLE TO FAIL THE DIST. Pillow is a
    convenience — without it every painting still ships, just at its
    authored size. The hint prints once, the copy happens anyway.
    """
    os.makedirs(DIST_PAINTINGS, exist_ok=True)
    try:
        from PIL import Image
    except ImportError:
        print("  (Pillow absent — copying paintings verbatim, uncapped.")
        print("   pip install pillow   for the %dpx / q%d re-encode.)"
              % (PAINTING_CAP, PAINTING_QUALITY))
        for f in names:
            shutil.copy2(os.path.join(SRC_PAINTINGS, f), os.path.join(DIST_PAINTINGS, f))
        return [os.path.join(DIST_PAINTINGS, f) for f in names]

    # The ORIGINAL filename is kept, .jpeg and .jpg alike: exhibition.json
    # names the file that exists in dist/, the program sniffs content not
    # extension (stb_image), and both extensions serve as image/jpeg.
    for f in names:
        src = os.path.join(SRC_PAINTINGS, f)
        dst = os.path.join(DIST_PAINTINGS, f)
        try:
            with Image.open(src) as im:
                im = im.convert("RGB")
                w, h = im.size
                if max(w, h) > PAINTING_CAP:
                    im.thumbnail((PAINTING_CAP, PAINTING_CAP), Image.LANCZOS)
                im.save(dst, "JPEG", quality=PAINTING_QUALITY, optimize=True)
        except Exception as e:
            # One unreadable file is not a reason to ship no exhibition.
            print("  (re-encode failed for %s: %s — copied verbatim)" % (f, e))
            shutil.copy2(src, dst)
    return [os.path.join(DIST_PAINTINGS, f) for f in names]


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

    # THE EXHIBITION, COUNTED SEPARATELY BECAUSE IT SHIPS SEPARATELY.
    # These bytes are not in the four files above and never will be
    # again — they are fetched by URL at runtime. Source sizes here;
    # the re-encoded dist figures print after the write.
    paintings = list_paintings()
    music = list_music()
    paintings_src_bytes = dir_bytes([os.path.join(SRC_PAINTINGS, f) for f in paintings])
    music_src_bytes = dir_bytes([os.path.join(SRC_MUSIC, f) for f in music])
    print("")
    print("exhibition inventory  (%s)" % os.path.join(ROOT, "assets"))
    print("  %-18s %14s  %9s  %7s" % ("kind", "bytes", "MiB", "files"))
    print("  %-18s %14d  %9.2f  %7d"
          % ("paintings (src)", paintings_src_bytes, mib(paintings_src_bytes), len(paintings)))
    print("  %-18s %14d  %9.2f  %7d"
          % ("music", music_src_bytes, mib(music_src_bytes), len(music)))
    print("  %-18s %14d  %9.2f  %7d"
          % ("TOTAL", paintings_src_bytes + music_src_bytes,
             mib(paintings_src_bytes + music_src_bytes), len(paintings) + len(music)))
    if not paintings:
        print("  (no PAINTING_*.jpg|.jpeg under assets/paintings — the dist will")
        print("   carry an empty exhibition, which the program reads as no paintings.)")

    if len(paintings) > MANIFEST_DEDUPE_CAP:
        over = len(paintings) - MANIFEST_DEDUPE_CAP
        print("")
        print("  !! %d PAINTINGS — %d PAST THE ROTATION'S DEDUPE CAP OF %d"
              % (len(paintings), over, MANIFEST_DEDUPE_CAP))
        print("     Nothing here fails and the dist is still written: every painting")
        print("     ships and every one of them hangs. What stops working is the")
        print("     rotation's no-duplicates rule, and it stops SILENTLY.")
        print("     rotate_authored_staging guards its bookkeeping with")
        print("     `disk_idx < %d && disk_in_use[disk_idx]`, so an index past the"
              % MANIFEST_DEDUPE_CAP)
        print("     array reads as 'not in use' — fail-OPEN, not fail-safe. The")
        print("     cursor can then hand painting #%d+ to two staging slots at once"
              % MANIFEST_DEDUPE_CAP)
        print("     and the same canvas hangs twice on one wall.")
        print("     Curate under %d, or raise the array in gallery.hpp and this"
              % MANIFEST_DEDUPE_CAP)
        print("     constant together — they are one number in two files.")

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
    print("  the_board.data   %d bytes  (%.2f MiB)   <- world.wgsl (the program's shader)" % (data, mib(data)))
    print("  wasm + js        %d bytes  (%.2f MiB)" % (
        sizes["the_board.wasm"] + sizes["the_board.js"],
        mib(sizes["the_board.wasm"] + sizes["the_board.js"])))
    print("  first visit      %d bytes  (%.2f MiB) uncompressed" % (total, mib(total)))
    print("  exhibition       %d bytes  (%.2f MiB) source, fetched AFTER first paint"
          % (paintings_src_bytes + music_src_bytes, mib(paintings_src_bytes + music_src_bytes)))
    print("  Both hosts serve br/gzip for js/html; .wasm and .data compress well over the")
    print("  wire, so the number a phone actually pays is lower than the figure above.")
    print("  RECORD IT. Do not optimize it (SHIP_0 U4).")

    # THE RULE SAYS EVERY FILE, AND dist/ IS NO LONGER FOUR FILES. The
    # exhibition ships in the same folder under the same per-file cap, so
    # it is weighed by the same three-way branch — not printed beside it
    # as a remark the verdict ignores. Source sizes, because the verdict
    # runs before the write and must also hold under --check: music is
    # copied verbatim so its source size IS its dist size, and paintings
    # only ever shrink, so a source size is a safe upper bound.
    verdict_sizes = dict(sizes)
    for f in paintings:
        verdict_sizes["paintings/" + f] = os.path.getsize(os.path.join(SRC_PAINTINGS, f))
    for f in music:
        verdict_sizes["music/" + f] = os.path.getsize(os.path.join(SRC_MUSIC, f))

    biggest = max(verdict_sizes, key=lambda k: verdict_sizes[k])
    biggest_n = verdict_sizes[biggest]
    print("")
    print("HOST VERDICT (by the numbers)")
    print("  largest single file: %s at %.2f MiB   (of %d files in dist/)"
          % (biggest, mib(biggest_n), len(verdict_sizes) + 1))
    if biggest_n <= CF_LIMIT:
        print("  -> CLOUDFLARE PAGES. Every file is within its 25 MiB per-file limit.")
        host = "cloudflare"
    elif biggest_n <= GH_LIMIT:
        print("  -> GITHUB PAGES. %s exceeds Cloudflare's 25 MiB; GitHub's ~100 MiB holds."
              % biggest)
        host = "github"
    else:
        print("  -> RESOLVE. %s exceeds BOTH limits (%.2f MiB)." % (biggest, mib(biggest_n)))
        print("     Not repackaging on my own authority. Options, for Jean's ruling:")
        print("       a. split the preload — ship a starter asset set, fetch the rest at runtime")
        print("       b. drop the preload and fetch assets over HTTP (needs a loader path)")
        print("       c. a host with no per-file cap (S3/R2 + CDN)")
        print("     If the offender is an exhibition file, a fourth option exists that the")
        print("     other three did not have: re-encode or split THAT FILE. It is not")
        print("     welded into the bundle any more.")
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
    print("EXHIBITION — assembling %d painting(s), %d track(s)" % (len(paintings), len(music)))
    painting_paths = write_paintings(paintings)
    os.makedirs(DIST_MUSIC, exist_ok=True)
    for f in music:
        shutil.copy2(os.path.join(SRC_MUSIC, f), os.path.join(DIST_MUSIC, f))
    music_paths = [os.path.join(DIST_MUSIC, f) for f in music]

    # THE MANIFEST IS FILENAMES, NOT PATHS. The program joins the folder
    # itself (paintings/<name>, music/<name>), so a path here would be a
    # second place that decides the layout.
    # ensure_ascii=False, and it is load-bearing. The default escapes any
    # non-ASCII character as \uXXXX, and the program's manifest parse is
    # a by-hand string scan with no unescaping — it would hand
    # "PAINTING_café.jpeg" to the fetch verbatim and take a 404 for
    # a file that is sitting right there. The handle is already utf-8 and
    # the C++ side reads raw bytes.
    with open(os.path.join(DIST, EXHIBITION_JSON), "w", encoding="utf-8", newline="\n") as fh:
        json.dump({"paintings": paintings, "music": music}, fh, indent=2, ensure_ascii=False)
        fh.write("\n")

    paintings_dist_bytes = dir_bytes(painting_paths)
    music_dist_bytes = dir_bytes(music_paths)
    file_count = len(ARTIFACTS) + len(painting_paths) + len(music_paths) + 1

    print("  %-18s %14d  %9.2f  %7d" % ("paintings (dist)", paintings_dist_bytes,
                                        mib(paintings_dist_bytes), len(painting_paths)))
    print("  %-18s %14d  %9.2f  %7d" % ("music (dist)", music_dist_bytes,
                                        mib(music_dist_bytes), len(music_paths)))
    if paintings_src_bytes and paintings_dist_bytes:
        print("  paintings re-encode: %.2f MiB -> %.2f MiB  (%.0f%% of source)"
              % (mib(paintings_src_bytes), mib(paintings_dist_bytes),
                 100.0 * paintings_dist_bytes / paintings_src_bytes))
    print("  %s: %d painting(s), %d track(s)" % (EXHIBITION_JSON, len(paintings), len(music)))

    # The verdict above already weighed these, from source sizes. This is
    # the confirmation on the bytes actually written — and it is a hard
    # stop, not a remark: a dist/ that cannot be deployed must not print
    # deploy instructions.
    exhibition_paths = painting_paths + music_paths
    if exhibition_paths:
        biggest_ex = max(exhibition_paths, key=os.path.getsize)
        ex_n = os.path.getsize(biggest_ex)
        host_limit = CF_LIMIT if host == "cloudflare" else GH_LIMIT
        print("  largest exhibition file (written): %s at %.2f MiB"
              % (os.path.basename(biggest_ex), mib(ex_n)))
        if ex_n > host_limit:
            print("")
            print("  -> RESOLVE. %s is %.2f MiB, past the %.0f MiB per-file limit of the"
                  % (os.path.basename(biggest_ex), mib(ex_n), mib(host_limit)))
            print("     host this run chose. The verdict above weighed SOURCE sizes; this")
            print("     is the written byte count, and it disagrees.")
            print("     dist/ is written but NOT deployable as-is. Re-encode or split")
            print("     that file — it is an exhibition file, not the bundle, so that")
            print("     costs a copy and not a compile.")
            return 3

    print("")
    print("WROTE %s  (%d files)" % (DIST, file_count))

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
        print("    cp -r dist/. .         # or: xcopy dist\\* . /E /Y  on cmd")
        print("                           # -r / /E: dist/ has paintings\\ and music\\ now")
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
