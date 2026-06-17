// check_field_union.cpp — prove the cross-channel presence union in isolation,
// before any canvas wiring leans on it. Two claims:
//
//   1. The op is a presence join. The union of several per-channel present-sets
//      marks a class iff it sounds in ANY of them; binary in, binary out; the
//      empty set is the identity, and a set joined with itself is unchanged.
//
//   2. It feeds the field unchanged, and that is the point. A bare D-major triad
//      on one channel is ambiguous; two foreign tones on another are ambiguous;
//      their union resolves to ONE field, clear and untied -- read by the same
//      machinery that reads a single voice, with nothing in it altered.

#include "musical/field.hpp"

#include <cstdio>
#include <cassert>
#include <initializer_list>

using namespace t7;

static constexpr int D = 2;   // PROJECT_PC_ORIGIN: the home note

// The canvas's bank, inline, so the election here matches the live one.
static const Field BANK[6] = {
    { "phrygian_dominant", field_mask({0, 1, 4, 5, 7, 8, 10}) },
    { "mixolydian",        field_mask({0, 2, 4, 5, 7, 9, 10}) },
    { "major",             field_mask({0, 2, 4, 5, 7, 9, 11}) },
    { "dorian",            field_mask({0, 2, 3, 5, 7, 9, 10}) },
    { "harmonic_minor",    field_mask({0, 2, 3, 5, 7, 8, 11}) },
    { "lydian_sharp2",     field_mask({0, 3, 4, 6, 7, 9, 11}) },
};

static PitchClassVector pcs(std::initializer_list<int> classes) {
    PitchClassVector s;
    for (int c : classes) s[((c % 12) + 12) % 12] = 1.0f;
    return s;
}

static void print_set(const char* label, const PitchClassVector& s) {
    std::printf("  %-12s { ", label);
    for (int i = 0; i < 12; ++i) if (s.v[i] > 0.0f) std::printf("%d ", i);
    std::printf("}\n");
}

static void expect_set(const PitchClassVector& s, std::initializer_list<int> classes) {
    PitchClassVector want = pcs(classes);
    for (int i = 0; i < 12; ++i) assert(s.v[i] == want.v[i]);
}

int main() {
    // 1. The op is a presence join. -------------------------------------------
    {
        PitchClassVector a = pcs({2, 6});            // ch0: D, F#
        PitchClassVector b = pcs({9, 0});            // ch1: A, C
        PitchClassVector sets[2] = {a, b};
        PitchClassVector u = present_union(sets, 2);
        print_set("ch0", a);
        print_set("ch1", b);
        print_set("union", u);
        expect_set(u, {0, 2, 6, 9});                 // sounding in either
        std::printf("  -> union is the presence-OR of the two\n\n");
    }

    // identity and idempotence: the join laws.
    {
        PitchClassVector a = pcs({2, 6, 9});
        PitchClassVector empty;
        PitchClassVector with_empty[2] = {a, empty};
        PitchClassVector with_self[2]  = {a, a};
        expect_set(present_union(with_empty, 2), {2, 6, 9});   // a + 0 = a
        expect_set(present_union(with_self, 2),  {2, 6, 9});   // a + a = a
        std::printf("  identity (a + empty = a) and idempotence (a + a = a) hold\n\n");
    }

    // 2. The union feeds the field, and decides what neither voice does. ------
    {
        PitchClassVector ch0 = pcs({2, 6, 9});       // D major triad: D F# A
        PitchClassVector ch1 = pcs({0, 4});          // C, E -- the b7 and the 9

        FieldChoice e0 = elect_field(to_degrees(ch0, D), BANK, 6);
        FieldChoice e1 = elect_field(to_degrees(ch1, D), BANK, 6);
        std::printf("  ch0 (D F# A)   -> %-16s overlap %.0f, tie %d   ambiguous\n",
                    BANK[e0.index].name, e0.overlap, e0.tie);
        std::printf("  ch1 (C E)      -> %-16s overlap %.0f, tie %d   ambiguous\n",
                    BANK[e1.index].name, e1.overlap, e1.tie);
        assert(e0.tie > 1);
        assert(e1.tie > 1);

        PitchClassVector sets[2] = {ch0, ch1};
        PitchClassVector u = present_union(sets, 2);
        FieldChoice eu = elect_field(to_degrees(u, D), BANK, 6);
        print_set("union", u);
        std::printf("  union          -> %-16s overlap %.0f, tie %d   clear\n",
                    BANK[eu.index].name, eu.overlap, eu.tie);
        expect_set(u, {0, 2, 4, 6, 9});              // D E F# A C
        assert(eu.index == 1);                       // mixolydian
        assert(eu.tie == 1);                         // untied

        HeldField hf;                                // and the held field takes it
        int idx = hf.step(to_degrees(u, D), BANK, 6);
        assert(idx == 1);
        std::printf("  -> held field bootstraps to %s\n", BANK[idx].name);
    }

    std::printf("\nOK -- the union is a presence join, and feeds the field exactly as one voice does;\n");
    std::printf("      the combination resolves to one field what each voice alone leaves open.\n");
    return 0;
}
