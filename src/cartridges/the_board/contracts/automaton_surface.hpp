#pragma once
// ═══ THE AUTOMATON'S BANK (ONE_SURFACE-II U1) ════════════════════════
//
// The Game of Life stopped being an ENTITY and became a PROPERTY OF THE
// GROUND. What was eight discrete zones — each rolling its own tier, its
// own parameters, its own footprint out of a lattice node — is one
// automaton over the whole finite cell grid, and its parameters are
// DIALS on a bank instead of rolls nobody could reach.
//
// ORGAN_3 SHAPE, the tree's standing form for a live bank:
//   AUTO_TABLE  the DESIGN — authored, constexpr, two jobs only:
//               seeding the bank and standing under its asserts
//   AUTO_LIVE   the BANK — what every runtime reader reads
//
// THE PARAMETRIC SPIRIT SURVIVES THE COLLAPSE, and that is the point of
// the spread columns. A zone drew each of its continuous parameters from
// a Gaussian about its tier's mean; the world's automaton draws each of
// its own from a Gaussian about the BANK's centre, once, at world birth,
// from the world seed. Same distribution, one draw instead of eight —
// so two worlds still differ, and the dial moves the distribution rather
// than replacing it. Exactly the atmosphere's shape (ATMOS_TABLE), and
// for the same reason.
//
// WHAT COLLAPSED, NAMED. The TIER machinery: ten Conway rows and four
// Pulse rows, each a size and a parameter set, selected by a weighted
// roll per zone. The world has ONE automaton, so there is nothing for a
// tier to distinguish and nothing for a weight to select between. The
// tables' VALUES survive as this bank's centres and spreads,
// transcribed; the SELECTION dies.
//
// TRANSCRIBED, NOT DERIVED, ON PURPOSE — ATMOS_TABLE's precedent, kept
// verbatim: a `= GOL_TIERS[6]` initializer would read well and then die
// with the table at U2, leaving the literals to be typed at the one
// moment nothing could check them. So they were typed HERE while the
// source still stood, and pinned field by field by a witness at the
// bottom of bodies/gol_zones.hpp.
//
// THAT WITNESS HAS DONE ITS JOB AND GONE (probated at THE_PANEL I U5).
// It died with the file at U2, exactly as designed — an assert that
// proves a transcription has nothing to prove once the source is gone —
// but this paragraph went on describing it in the present tense for
// three units. The numbers below stood under it for one commit, which
// is the whole window in which they could be checked, and that is the
// claim the idiom actually makes.

#include <cstdint>

namespace t7 {
namespace the_board {

    // ── The rule ─────────────────────────────────────────────────
    // L3 MIRROR: world.wgsl AUTO_ALGORITHM_* / AUTO_COLOR_* / PULSE_FIELD_*.
    struct AutomatonAlgorithm {
        static constexpr uint32_t CONWAY = 0;
        static constexpr uint32_t PULSE  = 1;
    };

    // The Pulse rows' field function: which spatial law writes the
    // per-cell target. BREATH is the per-cell sinusoid the algorithm
    // shipped with; SPIRAL is continuous rather than binary.
    struct AutomatonField {
        static constexpr uint32_t BREATH = 0;
        static constexpr uint32_t SPIRAL = 1;
    };

    // The spring's OVERSHOOT law on the visual value in [0,1] — NOT the
    // grid's topology. Read the banner at AUTO_TABLE.boundary_mode.
    struct AutomatonBoundary {
        static constexpr uint32_t REFLECT = 0;
        static constexpr uint32_t WRAP    = 1;
    };

    struct AutomatonColor {
        static constexpr uint32_t NEUTRAL  = 0;  // height-only extrusion, no tint
        static constexpr uint32_t LENS     = 1;  // shift ground toward the target colour
        static constexpr uint32_t BLACKISH = 2;  // darken toward near-black
    };

    // ═══ THE BANK ════════════════════════════════════════════════
    //
    // Centres and spreads, in the order the tier tables authored them.
    // A `_spread` of 0 makes its row a POINT row — the draw returns the
    // centre exactly, the IEEE identity, no hash and no round trip.
    struct AutomatonBank {
        // ─── The rule (selections, not draws) ─────────────────────
        uint32_t algorithm;            // AutomatonAlgorithm::
        uint32_t rule_mask;            // Conway B/S bitset: bit n birth, bit 9+n survival
        uint32_t field_fn;             // AutomatonField:: — Pulse only
        uint32_t color_mode;           // AutomatonColor::
        uint32_t boundary_mode;        // AutomatonBoundary:: — the SPRING's overshoot law

        // ─── Initial conditions ───────────────────────────────────
        float density,             density_spread;

        // ─── Temporal (BEATS — the header already speaks them) ────
        float tick_period,         tick_period_spread;

        // ─── Visual transition ────────────────────────────────────
        // `spring_stiffness` and `wander_radius` STOOD HERE AND DO NOT
        // COME ACROSS. Both were columns on both tier tables, drawn by
        // Gaussian in zone_derive_params, written into GoLZoneConfig —
        // and read by NOTHING, on either side, ever. The spring's actual
        // rate is omega = 3 / (transition_fraction * tick_period), which
        // is the two dials below; a third "stiffness" that no kernel
        // consults is a dial that lies about what it does. The Spiral
        // row's own comment had caught half of it ("the spring_stiffness
        // column beside it is written and never read"); the wander
        // radius was uncaught, and its comment reads as though the value
        // were live ("wander_radius 0 — the spiral centre IS the zone
        // centre and does not move"). Both die here, with the derive
        // kernel that was their only writer.
        float transition_fraction, transition_fraction_spread;

        // ─── Height ───────────────────────────────────────────────
        float alive_height,        alive_height_spread;

        // ─── Per-CELL scatter (amounts, not spreads) ──────────────
        // These are not distributions over the world's one draw; they
        // are how much each CELL differs from its neighbours, and they
        // stay scalar for that reason.
        float spring_variance;         // [0,1] per-cell spring speed scatter
        float phase_randomness;        // [0,1] per-cell phase offset (Pulse)
        float tempo_randomness;        // [0,1] per-cell frequency scatter (Pulse)
        float height_factor_mean;      // per-cell height multiplier, Gaussian
        float height_factor_sigma;
        float height_factor_lo;        // and its clamp
        float height_factor_hi;

        // ─── Colour target ────────────────────────────────────────
        float target[3];
        float target_spread;

        // ─── Eligibility ──────────────────────────────────────────
        // The ONE surviving term of the zone-lattice decision: the
        // automaton lives on DISCRETE ground and nowhere else. Below
        // this interpolated mode value a cell is smooth ground and the
        // automaton does not reach it.
        float mode_threshold;
    };

    // ═══ THE DESIGN ══════════════════════════════════════════════
    //
    // Transcribed from GOL_TIERS[6] "Glacier" — the highest-weight
    // Conway row at 0.21, i.e. the tier the retiring world drew most
    // often, and therefore the closest single answer to "what did the
    // zones look like". Its Pulse-only columns come from
    // GOL_PULSE_TIERS[0] "Breathe", the highest-weight Pulse row at
    // 0.38, so that flipping `algorithm` to PULSE lands on the modal
    // Pulse world rather than on zeros.
    //
    // THE ONE AUTHORED CHOICE IN THIS COMMIT, STATED PLAINLY. Ten tiers
    // collapse to one bank and SOME row has to be the one transcribed;
    // "the modal row" is the only defensible rule, and it is still a
    // choice about how the world looks. It is also the cheapest possible
    // choice to revise, which is the whole point of the campaign: every
    // number below is a dial now. Jean's walk rules it.
    //
    // AND THE SCALE QUESTION THE COLLAPSE OPENS, ALSO STATED. A zone was
    // 24 cells — 75 wu — inside a ~450 wu world. The automaton is the
    // whole 144-cell grid. At Glacier's density 0.12 that is ~2500 live
    // cells lifting alive_height 24 wu across the entire ground, where
    // before it was a few hundred inside eight islands. The VALUES are
    // faithfully transcribed; whether they READ at world scale is a
    // question only the walk can answer, and density and alive_height
    // are the two dials to reach for first.
    inline constexpr AutomatonBank AUTO_TABLE = {
        AutomatonAlgorithm::CONWAY,
        0x1808u,                          // B3/S23 — Conway's own rule
        AutomatonField::BREATH,           // unread while algorithm is CONWAY
        AutomatonColor::LENS,             // the modal colour weight (0.40)
        AutomatonBoundary::REFLECT,       // Conway's, transcribed — see below

        0.12f,  0.03f,                    // density,             ±
        4.0f,   0.0f,                     // tick_period (beats), ± — RELIEF_0: the tick IS the bar, on the bar line
        0.08f,  0.02f,                    // transition_fraction, ±
        24.0f,  7.5f,                     // alive_height,        ±

        0.25f,                            // spring_variance   (Glacier)
        0.15f,                            // phase_randomness  (Breathe)
        0.10f,                            // tempo_randomness  (Breathe)
        1.0f, 0.15f, 0.6f, 1.4f,          // per-cell height factor: mean, sigma, clamp

        { 0.5f, 0.5f, 0.5f }, 0.3f,       // colour target, ± — the uniform
                                          // [0.2, 0.8] the zone derive drew,
                                          // written as centre ± half-range

        0.50f,                            // mode_threshold
    };

    // ── WRAP IS PINNED, AND `boundary_mode` IS NOT THE THING PINNED ──
    //
    // THE TRAP, NAMED SO NOBODY WALKS INTO IT. The campaign pins the
    // TOPOLOGY to WRAP — "the torus is the point". There are two
    // different wraps in this machinery and they are not the same fact:
    //
    //   1. THE GRID'S TOPOLOGY — the neighbour walk's modular index,
    //      `(cell + d + n) % n`, in the evolve kernel. It has always
    //      been toroidal, it is a hardcoded modulus with no dial, and
    //      globalizing it is exactly what makes the world's opposite
    //      edges continuous. THIS is what is pinned, and it is pinned by
    //      being a modulus rather than a choice. It needs no field here.
    //
    //   2. `boundary_mode` — `apply_boundary(visual, mode)`, wrap01 vs
    //      reflect01 on the SPRING's overshoot in [0,1]. A different
    //      fact entirely: it is about a scalar leaving its range, not
    //      about a cell leaving the grid.
    //
    // Setting (2) to WRAP "because the campaign says WRAP" would change
    // how every overshooting cell settles — a silent visual change from
    // a word that meant something else. Conway's zones ran REFLECT and
    // this bank runs REFLECT.
    static_assert(AUTO_TABLE.boundary_mode == AutomatonBoundary::REFLECT,
        "the spring's overshoot law is the Conway zones' REFLECT, transcribed. "
        "The campaign's WRAP pin is the GRID's topology — the evolve kernel's "
        "modular neighbour walk — which is a hardcoded modulus and not this field");

    // THE CONWAY WITNESS. `rule_mask` is a B/S bitset and 0x1808 is
    // B3/S23: birth on 3 (bit 3 = 0x0008), survival on 2 and 3
    // (bits 11, 12 = 0x1800). A mask that is not this is not Conway, and
    // the handoff's "CONWAY boots" is the sentence this assert keeps.
    static_assert(AUTO_TABLE.algorithm == AutomatonAlgorithm::CONWAY
               && AUTO_TABLE.rule_mask == 0x1808u,
        "CONWAY boots: B3/S23. PULSE and every other rule are dials off this row");

    // THE LIVE BANK. Seeded whole — one struct, one copy, no per-row
    // list to forget a member of.
    inline AutomatonBank AUTO_LIVE = AUTO_TABLE;

    // THE COUPLING'S BOUNDS ON THE TWO SCALES stood here
    // (GROUND_SCALE_MIN 0.25 / MAX 4.0, GROUND_VOICE_0 → RELIEF_1):
    // they bounded a coupling that no longer exists. The HAND's bounds
    // are the organ rows' own ([0.1, 4.0] tick, [0.0, 4.0] height). A
    // future coupling aimed at these fields re-authors its bounds here —
    // and must keep the tick a divisor of the bar, or it breaks RELIEF's
    // latch law the way the quicken did.

} // namespace the_board
} // namespace t7
