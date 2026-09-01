#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════
# THE AUTOMATON CENSUS (GOL_ROWS_3 F1; repointed at ONE_SURFACE-II U3)
# — what a rule mask actually does
#
# WHY THIS EXISTS. A rule mask plus a handful of authored floats, and
# NOTHING in the tree tells you what the pair does. The old tier rows'
# comments said "terminal", "plateaus", "walls"; three of those claims
# were false when GOL_RULES_1 wrote them, and the campaign that found
# out did it with a harness that lived in a session transcript. The next
# rule campaign would rebuild that harness and rebuild its
# approximations differently — which already happened once: between
# GOL_ROWS_1 and GOL_ROWS_2 the seeding gained a Gaussian it had been
# missing and a headline dark count moved from 9 to 12.
#
# So the harness lives here, reads the artifact, and is one thing.
#
# ITS SUBJECT CHANGED AND ITS QUESTION DID NOT (ONE_SURFACE-II).
# It censused FOURTEEN TIER ROWS, each a candidate for an island. There
# is one automaton and one rule now, over the whole ground — so the
# question "does this mask go dark, saturate, or stay structured" stopped
# being a comparison between rows and became THE question about the
# world. And THE_PANEL is about to put `rule_mask` on a dial, which makes
# it askable by anyone. That is why this tool was repointed rather than
# attic'd.
#
# TWO THINGS THE MOVE CHANGED IN THE ANSWERS, and both matter:
#   · THE GRID IS THE WORLD'S. Rows were censused at their tier's 8..32
#     cells; the automaton runs at (2R+1) * PATCH_CELL_N, up to 144. A
#     mask that goes dark on a 24-cell torus need not on a 144-cell one,
#     and vice versa. Old numbers do not transfer.
#   · THE VALUES ARE READ, NOT PARSED. The driver #includes
#     contracts/automaton_surface.hpp, so AUTO_TABLE's fields are the
#     program's own values — one taxonomy row better than the parse this
#     replaced.
#
# WHAT IS REAL AND WHAT IS TRANSLITERATED
#   REAL      the hashes and the bucket walk — this tool compiles a
#             driver that #includes primitives/seed_utils.hpp, so
#             cpu_lattice_node_seed, cpu_hash_f and cpu_sample_gaussian
#             are the program's own, not a Python copy of them.
#   REAL      AUTO_TABLE — #included, not parsed. Edit the bank, rerun,
#             get the new answer.
#   MIRRORED  coupling_gol_next_state, pulse_cell_target's SPIRAL
#             branch, and automaton_evolve's spring + apply_boundary, all
#             transliterated from world.wgsl. If that file's versions
#             move, these must move with them — the census is a mirror
#             of the shader (L3 MIRROR), and it is not gate-covered.
#
# THE STATED LIMITATION — READ THIS BEFORE QUOTING A NUMBER
#   This tool does not reproduce automaton_seed's MASK — the birth
#   kernel multiplies the per-cell height factor by
#   discrete_visibility_rest. Evaluating it needs the whole colour and
#   field system. That kernel ONLY EVER REMOVES live cells at birth, so
#   every dark count this tool reports is a LOWER BOUND on the real one.
#   Comparisons BETWEEN candidates are sound — the omission applies to
#   all of them alike. A candidate's absolute distance from a band is
#   NOT sound, and a row that sits at the edge of one here is outside it
#   in the world.
#
# USAGE
#   python3 tools/gol_census.py                    # census every Conway row
#   python3 tools/gol_census.py --spiral           # Pulse: Spiral coherence
#   python3 tools/gol_census.py --seeds 512        # widen the sample
#   python3 tools/gol_census.py --gens 4000        # run them longer
#   python3 tools/gol_census.py --candidate 'Vote:0x3E1E0:0.50:0.06:32'
#         # name:mask:density_mean:density_sigma:cells — a what-if row,
#         # censused beside the real ones without touching the tree.
#         # Repeatable.
#
# WHAT THE CONWAY COLUMNS MEAN
#   dark        no live cell at the end. The zone renders nothing — no
#               height, and the tint's `color_val > 0.01` fails — while
#               still holding its footprint against every other zone.
#               Dead ground. This is the failure column.
#   saturated   every cell live. A solid raised block at the row's
#               alive_height. A legitimate object, not a failure, but
#               one object.
#   structured  neither extreme.
#   froze       reached a fixed point inside the generation budget. A
#               row that never freezes is a boil; whether that is right
#               is the row's business, but it should be on purpose.
# ═══════════════════════════════════════════════════════════════════════
"""GOL_ROWS_3: census what each GoL tier row does, from the tree's own tables."""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
BANK = os.path.join(REPO, "src", "cartridges", "the_board", "contracts",
                    "automaton_surface.hpp")
STATE = os.path.join(REPO, "src", "cartridges", "the_board", "realization",
                     "state.hpp")
PRIMS = os.path.join(REPO, "src", "cartridges", "the_board", "primitives")
INC = os.path.join(REPO, "src", "cartridges")

# The property index automaton_seed rolls aliveness on. Mirrors
# AUTO_PROP_DENSITY (world.wgsl) — a CELL draw, band AUTO_SEED_BAND.
PROP_DENSITY = 960

# ── THE TABLE PARSER IS GONE, AND THAT IS AN UPGRADE ─────────────────
#
# _num / _table / _names / read_tables stood here: a positional parser
# over GOL_TIERS[GOL_TIER_COUNT] and GOL_PULSE_TIERS[...] in
# bodies/gol_zones.hpp, with a name-array cross-check because a
# positional parse of a fourteen-column initializer is exactly as fragile
# as it sounds. The file is gone (ONE_SURFACE-II U2) and so is the
# fragility: the driver #includes contracts/automaton_surface.hpp and
# reads AUTO_TABLE's fields as VALUES. Nothing is transliterated, nothing
# is column-counted, and editing the bank changes the answer with no
# parser to keep in step.

# ── The driver ────────────────────────────────────────────────────────
#
# Everything below the include is a transliteration of world.wgsl. The
# spawn path above it is select_gol_zone's and seed_gol_zone's.

DRIVER = r'''
#include "the_board/primitives/seed_utils.hpp"
#include "the_board/contracts/automaton_surface.hpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <set>
#include <string>
#include <vector>
using namespace t7::the_board;

static const float PI_ = 3.14159265359f;
// AUTO_SEED_BAND / AUTO_PROP_DENSITY (world.wgsl, surface/automaton.hpp)
static const uint32_t SEED_BAND = 260u;
static const uint32_t PROP_DENSITY = 960u;

// THE BANK'S OWN VALUES, not a transliteration of them. This is the
// upgrade the parser's death bought: `bank` prints what AUTO_TABLE
// actually holds, and the Python side packs its census row from that.
static int print_bank() {
    using namespace t7::the_board;
    std::printf("%X %f %f %f %f %f %f %f %u %u\n",
        AUTO_TABLE.rule_mask,
        AUTO_TABLE.density, AUTO_TABLE.density_spread,
        AUTO_TABLE.tick_period, AUTO_TABLE.transition_fraction,
        AUTO_TABLE.phase_randomness, AUTO_TABLE.tempo_randomness,
        AUTO_TABLE.spring_variance,
        AUTO_TABLE.boundary_mode,
        AUTO_TABLE.field_fn);
    return 0;
}

// world.wgsl §3.7 — coupling_gol_next_state
static float next_state(bool alive, int neighbors, uint32_t rule_mask) {
    uint32_t n = (uint32_t)neighbors;
    uint32_t bit = alive ? (9u + n) : n;
    return (rule_mask & (1u << bit)) != 0u ? 1.0f : 0.0f;
}

// world.wgsl — zone_gol_evolve's Conway branch: Moore neighbourhood,
// wrapped with `% gs` on both axes.
static void step(const std::vector<float>& a, std::vector<float>& b,
                 int N, uint32_t rule) {
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
        int c = 0;
        for (int dy = -1; dy <= 1; dy++) for (int dx = -1; dx <= 1; dx++) {
            if (!dx && !dy) continue;
            if (a[((y + dy + N) % N) * N + ((x + dx + N) % N)] > 0.5f) c++;
        }
        b[y * N + x] = next_state(a[y * N + x] > 0.5f, c, rule);
    }
}

// world.wgsl — gol_cell_hash / gol_cell_variation
static uint32_t cell_hash(uint32_t cx, uint32_t cy) {
    return cx * 374761393u + cy * 668265263u;
}
static float cell_var(uint32_t h) { return (float)(h & 0xFFFFu) / 65535.0f; }

// world.wgsl — pulse_cell_target, SPIRAL branch. config.mode_gol_tick_scale
// is pinned at 1.0 by the boot block and has no driver, so it is 1 here.
static float spiral(uint32_t cx, uint32_t cy, float t_beats, float tick_period,
                    float phase_randomness, float tempo_randomness,
                    uint32_t grid_size) {
    uint32_t h = cell_hash(cx, cy);
    float cell_phase = cell_var(h) * phase_randomness * 2.0f * PI_;
    uint32_t h2 = cell_hash(cx + 137u, cy + 251u);
    float tempo_jitter = 1.0f + (cell_var(h2) - 0.5f) * tempo_randomness;
    float n = (float)grid_size;
    float px = (float)cx + 0.5f - n * 0.5f, py = (float)cy + 0.5f - n * 0.5f;
    float r = std::sqrt(px * px + py * py);
    float th = std::atan2(py, px) / (2.0f * PI_);
    float u = th * 2.0f + r / (n * 0.5f)
            - t_beats * tempo_jitter / std::fmax(tick_period * 1.0f, 0.01f);
    return 0.5f + 0.5f * std::cos(u * 2.0f * PI_ + cell_phase);
}
// the same field with every scatter term removed — the shape to hold against
static float spiral_ideal(uint32_t cx, uint32_t cy, float t_beats,
                          float tick_period, uint32_t grid_size) {
    float n = (float)grid_size;
    float px = (float)cx + 0.5f - n * 0.5f, py = (float)cy + 0.5f - n * 0.5f;
    float r = std::sqrt(px * px + py * py);
    float th = std::atan2(py, px) / (2.0f * PI_);
    return 0.5f + 0.5f * std::cos((th * 2.0f + r / (n * 0.5f)
                 - t_beats / std::fmax(tick_period, 0.01f)) * 2.0f * PI_);
}
static float wrap01(float x) { return x - std::floor(x); }   // apply_boundary WRAP

// select_gol_zone: the per-zone density is a Gaussian draw, clamped, and
// seed_gol_zone then rolls each cell against THAT — not against the mean.
static float zone_density(uint32_t seed, float mean, float sigma) {
    return std::max(0.05f, std::min(0.9f,
        cpu_sample_gaussian(seed, PROP_DENSITY, mean, sigma)));
}

static float population(const std::vector<float>& a) {
    float p = 0; for (float v : a) p += v; return p;
}

// ── Conway census ────────────────────────────────────────────────────
static void census(const char* name, uint32_t rule, float dm, float ds,
                   int N, int seeds, int gens) {
    int dark = 0, full = 0, structured = 0, froze = 0;
    double live = 0, dens = 0;
    for (uint32_t k = 0; k < (uint32_t)seeds; k++) {
        uint32_t seed = cpu_lattice_node_seed(9000u + k, (int32_t)k, 13, SEED_BAND);
        float d = zone_density(seed, dm, ds);
        dens += d;
        std::vector<float> a(N * N), b(N * N);
        for (int i = 0; i < N * N; i++)
            a[i] = cpu_hash_f(seed + i, PROP_DENSITY) < d ? 1.0f : 0.0f;
        int at = -1;
        for (int g = 1; g <= gens; g++) {
            step(a, b, N, rule);
            bool same = (a == b);
            a.swap(b);
            if (same && at < 0) at = g;
        }
        if (at > 0) froze++;
        float lv = 100.0f * population(a) / (N * N);
        live += lv;
        if (lv < 0.5f) dark++; else if (lv > 99.5f) full++; else structured++;
    }
    printf("  %-16s 0x%-6X %.2f/%.2f %2d | %4d | %4d | %4d | %5.1f%% | %4d | %5.1f%%\n",
           name, rule, dm, ds, N, dark, full, structured,
           live / seeds, froze, 100.0 * dark / seeds);
}

// ── Spiral coherence ─────────────────────────────────────────────────
// Shape correlation: the best match against the scatter-free spiral over
// ALL phase offsets. The spring lags the drive by a fixed phase, and a
// rotating spiral shifted in phase is the same spiral — an un-lagged
// correlation reads a large constant negative and means nothing.
static double shape_corr(const std::vector<float>& v, int N, float t, float tick) {
    double best = -2;
    for (int k = 0; k < 180; k++) {
        float tt = t - tick * (float)k / 180.0f;
        double sa=0, sb=0, saa=0, sbb=0, sab=0; int n=0;
        for (uint32_t y = 0; y < (uint32_t)N; y++)
        for (uint32_t x = 0; x < (uint32_t)N; x++) {
            double A = v[y * N + x], B = spiral_ideal(x, y, tt, tick, (uint32_t)N);
            sa+=A; sb+=B; saa+=A*A; sbb+=B*B; sab+=A*B; n++;
        }
        double ma = sa/n, mb = sb/n;
        double den = std::sqrt((saa/n - ma*ma) * (sbb/n - mb*mb));
        if (den > 1e-12) { double c = (sab/n - ma*mb) / den; if (c > best) best = c; }
    }
    return best;
}

static void spiral_run(float tick, float trans, float phase, float tempo,
                       float sv, int N, int bnd) {
    printf("  row: tick %.1f, trans %.2f, phase %.2f, tempo %.2f, sv %.2f, "
           "%d cells, %s\n", tick, trans, phase, tempo, sv, N,
           bnd ? "WRAP" : "REFLECT");
    printf("  omega = 3 / (%.2f x %.1f) = %.2f   (transition_fraction x "
           "tick_period IS the spring)\n\n", trans, tick, 3.0f / (trans * tick));
    printf("    t_beats  at 120bpm   shape corr   neighbour step   distinct   rails\n");
    std::vector<float> vis(N * N, 0.0f), vel(N * N, 0.0f);
    float dt = 1.0f / 60.0f, t = 0.0f;
    const int marks[] = {20, 75, 150, 300, 900, 1800, 3600};
    int mi = 0;
    while (mi < 7) {
        t += dt * 2.0f;                                  // beats, at 120bpm
        for (uint32_t y = 0; y < (uint32_t)N; y++)
        for (uint32_t x = 0; x < (uint32_t)N; x++) {
            int i = y * N + x;
            float tgt = spiral(x, y, t, tick, phase, tempo, (uint32_t)N);
            float om = 3.0f / std::fmax(trans * tick, 0.01f);
            if (sv > 0.001f) {
                float j = 1.0f + (cell_var(cell_hash(x + 53u, y + 97u)) - 0.5f) * sv;
                om = om / std::fmax(j, 0.3f);
            }
            float od = om * dt, e = std::exp(-od), d = vis[i] - tgt;
            float nv = tgt + (d * (1.0f + od) + vel[i] * dt) * e;
            float nvel = (vel[i] * (1.0f - od) - d * om * om * dt) * e;
            if (std::fabs(nv - tgt) < 0.001f && std::fabs(nvel) < 0.01f) {
                vis[i] = tgt; vel[i] = 0.0f;
            } else {
                vis[i] = bnd ? wrap01(nv) : nv;
                vel[i] = (nv < 0.0f || nv > 1.0f) ? 0.0f : nvel;
            }
        }
        if (t >= marks[mi]) {
            std::set<int> buckets; int rails = 0; double stepsum = 0; int m = 0;
            for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
                float A = vis[y * N + x];
                buckets.insert((int)(A * 1000));
                if (A < 0.01f || A > 0.99f) rails++;
                if (x + 1 < N) { stepsum += std::fabs(A - vis[y * N + x + 1]); m++; }
            }
            printf("    %7d  %6.1f min      %6.3f          %.3f          %4zu      %4d\n",
                   marks[mi], marks[mi] / 120.0, shape_corr(vis, N, t, tick),
                   stepsum / m, buckets.size(), rails);
            mi++;
        }
    }
}

// argv: MODE then packed rows.
//   conway  seeds gens   then name:mask:dm:ds:cells per row
//   spiral  tick:trans:phase:tempo:sv:cells:bnd
int main(int argc, char** argv) {
    std::string mode = argv[1];
    if (mode == "spiral") {
        float f[7]; std::string s = argv[2]; size_t p = 0; int i = 0;
        while (i < 7) {
            size_t q = s.find(':', p);
            f[i++] = atof(s.substr(p, q == std::string::npos ? q : q - p).c_str());
            if (q == std::string::npos) break;
            p = q + 1;
        }
        spiral_run(f[0], f[1], f[2], f[3], f[4], (int)f[5], (int)f[6]);
        return 0;
    }
    if (std::string(argv[1]) == "bank") { return print_bank(); }
    int seeds = atoi(argv[2]), gens = atoi(argv[3]);
    for (int a = 4; a < argc; a++) {
        std::string s = argv[a];
        std::vector<std::string> f; size_t p = 0, q;
        while ((q = s.find(':', p)) != std::string::npos) {
            f.push_back(s.substr(p, q - p)); p = q + 1;
        }
        f.push_back(s.substr(p));
        census(f[0].c_str(), (uint32_t)strtoul(f[1].c_str(), 0, 16),
               (float)atof(f[2].c_str()), (float)atof(f[3].c_str()),
               atoi(f[4].c_str()), seeds, gens);
    }
    return 0;
}
'''


def build(tmp):
    cxx = shutil.which("clang++") or shutil.which("g++")
    if not cxx:
        raise SystemExit("gol-census: no clang++ or g++ on PATH")
    src = os.path.join(tmp, "census.cpp")
    with open(src, "w", encoding="utf-8") as fh:
        fh.write(DRIVER)
    exe = os.path.join(tmp, "census")
    c = subprocess.run([cxx, "-std=c++20", "-O2", "-I", INC, src, "-o", exe],
                       capture_output=True, text=True)
    if c.returncode != 0:
        sys.stdout.write(c.stdout + c.stderr)
        raise SystemExit("gol-census: the driver did not compile against "
                         "primitives/seed_utils.hpp")
    return exe


def decode(mask):
    born = "".join(str(n) for n in range(9) if int(mask) & (1 << n))
    surv = "".join(str(n) for n in range(9) if int(mask) & (1 << (9 + n)))
    return "B%s/S%s" % (born, surv)


def read_bank(exe):
    """AUTO_TABLE's fields, from the compiled driver — values, not text."""
    out = subprocess.run([exe, "bank"], capture_output=True, text=True)
    if out.returncode != 0 or not out.stdout.strip():
        raise SystemExit("gol-census: the driver could not print AUTO_TABLE")
    f = out.stdout.split()
    return {
        "rule_mask":           int(f[0], 16),
        "density_mean":        float(f[1]),
        "density_sigma":       float(f[2]),
        "tick_period_mean":    float(f[3]),
        "transition_fraction_mean": float(f[4]),
        "phase_randomness_mean":    float(f[5]),
        "tempo_randomness_mean":    float(f[6]),
        "spring_variance":     float(f[7]),
        "boundary_mode":       int(f[8]),
        "field_fn":            int(f[9]),
        # THE ONE THING STILL READ AS TEXT, and it is one integer with an
        # unambiguous anchor rather than a fourteen-column initializer.
        # Dim lives in realization/state.hpp, which pulls the instruments
        # dial and the whole wgpu surface behind it — far too much to
        # compile for one number. Grepping a `constexpr uint32_t
        # AUTO_GRID_MAX = <n>;` is the honest trade, and it fails LOUD.
        "grid_cells":          read_grid_max(),
    }


GRID_RE = re.compile(r"constexpr\s+uint32_t\s+AUTO_GRID_MAX\s*=\s*(\d+)\s*;")


def read_grid_max():
    with open(STATE, encoding="utf-8") as fh:
        m = GRID_RE.search(fh.read())
    if not m:
        raise SystemExit("gol-census: no `constexpr uint32_t AUTO_GRID_MAX = "
                         "<n>;` in realization/state.hpp — the automaton's "
                         "grid capacity moved or was renamed")
    return int(m.group(1))


def main():
    ap = argparse.ArgumentParser(
        description="Census what the automaton's rule mask actually does.")
    ap.add_argument("--seeds", type=int, default=32,
                    help="world seeds (default 32)")
    ap.add_argument("--gens", type=int, default=2000,
                    help="generations per world (default 2000)")
    ap.add_argument("--candidate", action="append", default=[],
                    help="a what-if rule not in the bank: "
                         "name:mask:density_mean:density_sigma:cells. "
                         "THIS IS THE POINT OF THE TOOL NOW — the bank has "
                         "one rule and THE_PANEL puts it on a dial, so the "
                         "question is always 'what would THIS mask do'.")
    ap.add_argument("--spiral", action="store_true",
                    help="Pulse instead: the SPIRAL field's arm coherence")
    args = ap.parse_args()

    tmp = tempfile.mkdtemp(prefix="gol_census_")
    try:
        exe = build(tmp)
        bank = read_bank(exe)

        if args.spiral:
            print("── the SPIRAL field's arm coherence, from AUTO_TABLE ──\n")
            packed = "%f:%f:%f:%f:%f:%d:%d" % (
                bank["tick_period_mean"], bank["transition_fraction_mean"],
                bank["phase_randomness_mean"], bank["tempo_randomness_mean"],
                bank["spring_variance"], bank["grid_cells"],
                bank["boundary_mode"])
            subprocess.run([exe, "spiral", packed])
            print("\n  shape corr 1.00 = the arms are intact. Tempo scatter is a "
                  "per-cell FREQUENCY\n  multiplier and its phase error integrates "
                  "in t_beats; phase scatter is a bounded\n  static offset. Only "
                  "one of the two can ever cost coherence.")
            return 0

        packed = ["%s:%X:%.4f:%.4f:%d" % ("AUTO_TABLE", bank["rule_mask"],
                                          bank["density_mean"],
                                          bank["density_sigma"],
                                          bank["grid_cells"])]
        for c in args.candidate:
            f = c.split(":")
            if len(f) != 5:
                raise SystemExit("gol-census: --candidate wants "
                                 "name:mask:density_mean:density_sigma:cells")
            packed.append("%s:%X:%.4f:%.4f:%d"
                          % (f[0].replace(" ", "_"), int(f[1], 0),
                             float(f[2]), float(f[3]), int(f[4])))

        print("── the automaton's rule, %d world seeds each, %d generations ──"
              % (args.seeds, args.gens))
        print("   the grid is the WORLD's (%d cells/side), not a tier's 8..32 — "
              "old tier numbers do NOT transfer." % bank["grid_cells"])
        print("   dark counts are a LOWER BOUND: the birth mask is not modelled "
              "(see the header).\n")
        print("   %-16s %s   (AUTO_TABLE)" % ("AUTO_TABLE", decode(bank["rule_mask"])))
        for c in args.candidate:
            f = c.split(":")
            print("   %-16s %s   (candidate, not in the bank)"
                  % (f[0], decode(int(f[1], 0))))
        print()
        print("  %-16s %-8s %-9s %-2s | %-4s | %-4s | %-4s | %-6s | %-4s | %s"
              % ("row", "mask", "dens m/s", "N", "dark", "satu", "strc",
                 "live", "frze", "dark%"))
        print("  " + "-" * 88)
        subprocess.run([exe, "conway", str(args.seeds), str(args.gens)] + packed)
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
