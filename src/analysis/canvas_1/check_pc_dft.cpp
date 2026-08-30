// ─── check_pc_dft — the pure-fn gate for the pc-DFT capability ────
// Known vectors → known families (CORRECTED against the math at the
// gate's first run: the major triad peaks f3 — |X3| = √5, the
// triadicity coefficient — NOT f5 as the handoff sketched; f5's
// signature set is the DIATONIC SCALE, |X5| ≈ 3.73 with every other
// family ≤ 1): a whole-tone cluster puts all energy in f6; a single
// pc → all mags equal (1.0); the zero vector rests at mags 0 /
// phases 0; mags are transposition-invariant; phases stay in [−π,π].
// No canvas, no port, no RtMidi — pc_dft.hpp + musical_ops.hpp alone.

#include <cassert>
#include <cmath>
#include <cstdio>
#include "musical/pc_dft.hpp"

using t7::PitchClassVector;
using t7::PcDft;
using t7::pc_dft;

static bool near(float a, float b, float eps = 1e-5f) { return std::fabs(a - b) < eps; }

int main() {
    // 1. Single pc → every family's mag is exactly 1 (|X_k| = L1 = 1).
    {
        PitchClassVector v; v[0] = 1.0f;
        const PcDft d = pc_dft(v);
        for (int k = 0; k < 6; ++k) assert(near(d.mag[k], 1.0f));
    }

    // 2. Whole-tone cluster {0,2,4,6,8,10} → all energy in f6, none elsewhere.
    {
        PitchClassVector v;
        for (int pc = 0; pc < 12; pc += 2) v[pc] = 1.0f;
        const PcDft d = pc_dft(v);
        assert(near(d.mag[5], 1.0f));
        for (int k = 0; k < 5; ++k) assert(near(d.mag[k], 0.0f));
    }

    // 3a. C major triad {0,4,7} → argmax is f3 (index 2): |X3| = √5 ≈ 2.236,
    //     the triadicity coefficient (f5 runs second at ≈ 1.932).
    {
        PitchClassVector v; v[0] = 1.0f; v[4] = 1.0f; v[7] = 1.0f;
        const PcDft d = pc_dft(v);
        int argmax = 0;
        for (int k = 1; k < 6; ++k) if (d.mag[k] > d.mag[argmax]) argmax = k;
        assert(argmax == 2);
        assert(near(d.mag[2], std::sqrt(5.0f) / 3.0f, 1e-4f));
    }

    // 3b. The DIATONIC SCALE {0,2,4,5,7,9,11} → argmax is f5 (index 4):
    //     |X5| ≈ 3.732 while every other family sits at ≤ 1 — the fifths
    //     family's signature set.
    {
        PitchClassVector v;
        for (int pc : {0, 2, 4, 5, 7, 9, 11}) v[pc] = 1.0f;
        const PcDft d = pc_dft(v);
        int argmax = 0;
        for (int k = 1; k < 6; ++k) if (d.mag[k] > d.mag[argmax]) argmax = k;
        assert(argmax == 4);
        for (int k = 0; k < 6; ++k) if (k != 4) assert(d.mag[k] < d.mag[4] * 0.5f);
    }

    // 4. Zero vector → the declared REST of the pure half: mags 0, phases 0.
    {
        PitchClassVector v;
        const PcDft d = pc_dft(v);
        for (int k = 0; k < 6; ++k) { assert(near(d.mag[k], 0.0f)); assert(near(d.phase[k], 0.0f)); }
    }

    // 5. Transposition invariance of mags (rotation only moves phase).
    {
        PitchClassVector a; a[0] = 1.0f; a[4] = 1.0f; a[7] = 1.0f;
        PitchClassVector b; b[3] = 1.0f; b[7] = 1.0f; b[10] = 1.0f;   // +3 semitones
        const PcDft da = pc_dft(a), db = pc_dft(b);
        for (int k = 0; k < 6; ++k) assert(near(da.mag[k], db.mag[k]));
    }

    // 6. Phase range [−π,π] over a spread of vectors.
    {
        for (int seed = 0; seed < 12; ++seed) {
            PitchClassVector v; v[seed] = 1.0f; v[(seed * 5 + 3) % 12] = 2.0f;
            const PcDft d = pc_dft(v);
            for (int k = 0; k < 6; ++k) assert(d.phase[k] >= -3.14159266f && d.phase[k] <= 3.14159266f);
        }
    }

    std::printf("check_pc_dft: GREEN — triad peaks f3, diatonic scale peaks f5, "
                "whole-tone peaks f6, single pc uniform, zero rests, "
                "mags transposition-invariant\n");
    return 0;
}
