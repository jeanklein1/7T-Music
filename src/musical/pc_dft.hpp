#pragma once

// ─── pc_dft.hpp ──────────────────────────────────────────────────
//
// THE PC-DFT CAPABILITY (the compound stratum): the discrete Fourier
// transform of a PUBLISHED pitch-class vector — the six interval
// families f1..f6 as magnitude + phase. One stratum above pc_count:
// where pc_count reads the VIEWS (playhead/wagon), pc_dft reads a
// published pc-vector — the same 12 floats the PRESENT_COUNT slots
// carry, re-origined as shipped. The DFT reads what ships, not raw
// state.
//
// The families (Fourier-phase literature): f1 chromatic cluster axis ·
// f2 tritone-pair axis · f3 minor-third/octatonic · f4 major-third/
// hexatonic · f5 the FIFTHS family (diatonic/triadic sets peak here —
// a major triad's argmax) · f6 the whole-tone family (a whole-tone
// cluster puts ALL its energy here).
//
// UNITS (the slot-map contract):
//   mag[k−1]   normalized [0,1] — |X_k| ÷ L1(v); L1 = 0 → 0 (safe;
//              for non-negative v the triangle inequality caps it at 1).
//   phase[k−1] radians [−π,π] on the pc circle (atan2 convention);
//              origin = the PUBLISHED origin (D after the canvas dress).
//
// REST (declared here, held by the publisher): zero vector → mags 0;
// phases HOLD-LAST per channel AT THE PUBLISH SITE — a consumer fading
// on mag never sees phase snap. The pure fn itself returns phase 0 for
// the zero vector; holding is the caller's stateful half (the same
// entry-owned pattern as the held field).
//
// Depends on: musical/musical_ops.hpp (PitchClassVector) + <cmath>.

#include <cmath>
#include "musical/musical_ops.hpp"

namespace t7 {

struct PcDft {
    float mag[6];     // families f1..f6, L1-normalized [0,1]
    float phase[6];   // radians [−π,π]; 0 for the zero vector
};

// The 12-point real DFT, families k = 1..6 (k=0 is the L1 itself; k>6
// mirrors by conjugate symmetry). X_k = Σ_n v[n]·e^(−i·2πkn/12).
inline PcDft pc_dft(const PitchClassVector& v) {
    PcDft out{};
    float l1 = 0.0f;
    for (int n = 0; n < 12; ++n)
        l1 += (v.v[n] < 0.0f ? -v.v[n] : v.v[n]);
    constexpr float TAU = 6.28318530717958647692f;
    for (int k = 1; k <= 6; ++k) {
        float re = 0.0f, im = 0.0f;
        for (int n = 0; n < 12; ++n) {
            const float a = TAU * (float)(k * n) / 12.0f;
            re += v.v[n] * std::cos(a);
            im -= v.v[n] * std::sin(a);
        }
        out.mag[k - 1]   = (l1 > 0.0f) ? std::sqrt(re * re + im * im) / l1 : 0.0f;
        out.phase[k - 1] = (l1 > 0.0f) ? std::atan2(im, re) : 0.0f;
    }
    return out;
}

} // namespace t7
