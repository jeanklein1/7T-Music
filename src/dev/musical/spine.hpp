#pragma once

// ─── spine.hpp ───────────────────────────────────────────────────
//
// The previous-note line and its election. A comparison needs two
// terms; the previous term must be a single note, elected from the
// previous event by a total order. Time gives a partial order; this
// machine completes it, by the cascade law, in order of authority:
//
//   1  survived — the last note to go offset; the music performed it.
//   2  minted   — the current term of a recorded succession; the
//                 music stated it.
//   3  chosen   — the oracle, an injected criterion; we declared it.
//
// Facts about "previous" are facts about the spine, and the spine
// records them only at crossings through cardinality one:
//
//   1 → 2  mints succession: a note entering over exactly one sounding
//          note ticks the line — the ground becomes its previous, the
//          entrant becomes the holder. Entering over a chord mints
//          nothing; synchronous onsets mint nothing (a chord entered,
//          not a note) and void the mint they interrupt.
//   2 → 1  is the handover: the survivor is handed the line. If the
//          line already rests on the survivor, nothing moves.
//
// The vertical mints nothing; it only eliminates. At poly → 0 the
// event ends and the election runs over the final offset cohort
// (offsets within tolerance of the last): a sole survivor is elected
// SURVIVED; a tied cohort elects the holder, MINTED, unless the
// holder was installed by the dying tie itself; otherwise the oracle,
// CHOSEN. The resolved value seeds the next event: a note entering
// silence takes it as its previous, carrying its provenance.
//
// The first event has no previous: resolved is invalid until the
// first election. Parked by decision, not oversight.
//
// Clock contract: the mechanism is agnostic — it asks only for a
// monotonic clock, with tolerance in the same units. The interval probe
// fed it arrival seconds; owned by a Context it is fed the beat clock
// with everything else, and its grouping is musical. Same contract as
// PreviousEvent.
//
// Depends on: <array>, <cstdint>.

#include <array>
#include <cstdint>

namespace t7 {

    // ═══ PROVENANCE ══════════════════════════════════════════════════

    enum class Provenance : uint8_t {
        None,       // no election yet
        Survived,   // the music performed it (sole last offset)
        Minted,     // the music stated it (recorded succession)
        Chosen      // we declared it (the oracle)
    };

    inline const char* provenance_name(Provenance p) {
        switch (p) {
        case Provenance::Survived: return "survived";
        case Provenance::Minted:   return "minted";
        case Provenance::Chosen:   return "chosen";
        default:                   return "none";
        }
    }

    // ═══ SPINE ═══════════════════════════════════════════════════════

    class Spine {
    public:
        static constexpr int MAX_SOUNDING = 16;
        static constexpr int MAX_COHORT = 16;

        // The oracle: completes the order when time is silent. Receives
        // the tied pitches; returns the elected one. Lowest by default —
        // one binding of the slot, not the slot.
        using Oracle = int (*)(const int* pitches, int count);

        static int oracle_lowest(const int* pitches, int count) {
            int best = pitches[0];
            for (int i = 1; i < count; ++i)
                if (pitches[i] < best) best = pitches[i];
            return best;
        }

        explicit Spine(float tolerance = 0.05f)
            : tolerance_(tolerance), oracle_(&oracle_lowest) {
        }

        void  set_tolerance(float seconds) { tolerance_ = seconds; }
        float tolerance() const { return tolerance_; }
        void  set_oracle(Oracle o) { if (o) oracle_ = o; }

        // ── Feed ─────────────────────────────────────────────────────

        /**
         * A note begins sounding at time t (grouping clock, seconds).
         */
        void on_onset(int pitch, float t) {
            // Retrigger of a sounding pitch: refresh its onset, no crossing.
            for (int i = 0; i < n_sounding_; ++i) {
                if (sounding_[i].pitch == pitch) {
                    sounding_[i].onset = t;
                    return;
                }
            }

            const int n = n_sounding_;

            if (n == 0) {
                // Entering silence: the resolved election is the previous.
                if (resolved_.valid) {
                    prev_ = LineNote{ resolved_.pitch, resolved_.prov, true };
                }
                else {
                    prev_.valid = false;   // first event — no previous (parked)
                }
                holder_ = Holder{ pitch, t, true };
            }
            else if (n == 1) {
                if (holder_.valid && (t - holder_.set_t) <= tolerance_) {
                    // Synchronous with a just-installed holder: a chord is
                    // being born, not a note arriving. The line is illegible.
                    holder_.valid = false;
                    prev_.valid = false;
                }
                else if (holder_.valid) {
                    // MINT at 1 → 2: ground becomes previous, entrant holds.
                    stash_holder_ = holder_;
                    stash_prev_ = prev_;
                    prev_ = LineNote{ holder_.pitch, Provenance::Minted, true };
                    holder_ = Holder{ pitch, t, true };
                }
                // holder invalid at n==1 cannot persist (handover re-arms it),
                // but if it occurs, the vertical rule applies: mint nothing.
            }
            else {
                // n >= 2: the vertical mints nothing. One correction only:
                // a partner arriving within tolerance of a fresh mint voids
                // it — the "entrant" was a chord. Revert to the pre-mint line.
                if (holder_.valid && (t - holder_.set_t) <= tolerance_) {
                    holder_ = stash_holder_;
                    prev_ = stash_prev_;
                }
            }

            if (n_sounding_ < MAX_SOUNDING) {
                sounding_[n_sounding_++] = SoundingNote{ pitch, t };
            }
        }

        /**
         * A note stops sounding at time t (grouping clock, seconds).
         */
        void on_offset(int pitch, float t) {
            // Remove from the sounding set (swap with last).
            int idx = -1;
            for (int i = 0; i < n_sounding_; ++i) {
                if (sounding_[i].pitch == pitch) { idx = i; break; }
            }
            if (idx < 0) return;   // offset of a note we never saw — ignore
            sounding_[idx] = sounding_[--n_sounding_];

            // The final-offset cohort: pitches whose offsets tie within
            // tolerance of the latest. An offset beyond tolerance resets it.
            if (n_cohort_ > 0 && (t - last_offset_t_) <= tolerance_) {
                if (n_cohort_ < MAX_COHORT) cohort_[n_cohort_++] = pitch;
            }
            else {
                cohort_[0] = pitch;
                n_cohort_ = 1;
            }
            last_offset_t_ = t;

            if (n_sounding_ == 1) {
                // HANDOVER at 2 → 1: the survivor is handed the line.
                // If the line already rests on it, nothing moves — the
                // holder keeps its original installation time.
                const int survivor = sounding_[0].pitch;
                if (!(holder_.valid && holder_.pitch == survivor)) {
                    holder_ = Holder{ survivor, t, true };
                    prev_.valid = false;
                }
            }
            else if (n_sounding_ == 0) {
                elect();
            }
        }

        /**
         * Forget everything, including the resolved election.
         */
        void clear() {
            n_sounding_ = 0;
            n_cohort_ = 0;
            holder_.valid = prev_.valid = resolved_.valid = false;
            stash_holder_.valid = stash_prev_.valid = false;
        }

        // ── The line (live) ──────────────────────────────────────────

        bool line_legible() const { return holder_.valid && n_sounding_ > 0; }
        int  line_holder() const { return holder_.pitch; }
        bool has_line_previous() const { return prev_.valid; }
        int  line_previous() const { return prev_.pitch; }
        Provenance line_previous_provenance() const { return prev_.prov; }

        // ── The election (resolved) ──────────────────────────────────

        bool       has_resolved() const { return resolved_.valid; }
        int        resolved_pitch() const { return resolved_.pitch; }
        Provenance resolved_provenance() const { return resolved_.prov; }

        // ── The index ────────────────────────────────────────────────

        int sounding_count() const { return n_sounding_; }

    private:
        struct SoundingNote { int pitch; float onset; };
        struct Holder { int pitch = 0; float set_t = 0.0f; bool valid = false; };
        struct LineNote { int pitch = 0; Provenance prov = Provenance::None; bool valid = false; };

        float  tolerance_;
        Oracle oracle_;

        std::array<SoundingNote, MAX_SOUNDING> sounding_{};
        int n_sounding_ = 0;

        std::array<int, MAX_COHORT> cohort_{};
        int   n_cohort_ = 0;
        float last_offset_t_ = 0.0f;

        Holder   holder_;
        LineNote prev_;
        Holder   stash_holder_;     // pre-mint line, for voided mints
        LineNote stash_prev_;
        LineNote resolved_;         // the elected previous event

        // ── The cascade, run at poly → 0 ─────────────────────────────

        void elect() {
            if (n_cohort_ == 1) {
                // Key 1 — survived: a sole last offset; the music selected.
                resolved_ = LineNote{ cohort_[0], Provenance::Survived, true };
            }
            else {
                // Key 2 — minted: the holder, if it is among the tied
                // survivors and was not installed by the dying tie itself.
                bool holder_in_tie = false;
                if (holder_.valid && (last_offset_t_ - holder_.set_t) > tolerance_) {
                    for (int i = 0; i < n_cohort_; ++i)
                        if (cohort_[i] == holder_.pitch) { holder_in_tie = true; break; }
                }
                if (holder_in_tie) {
                    resolved_ = LineNote{ holder_.pitch, Provenance::Minted, true };
                }
                else {
                    // Key 3 — chosen: time said nothing; the oracle completes.
                    resolved_ = LineNote{ oracle_(cohort_.data(), n_cohort_),
                                         Provenance::Chosen, true };
                }
            }

            holder_.valid = prev_.valid = false;
            stash_holder_.valid = stash_prev_.valid = false;
            n_cohort_ = 0;
        }
    };

} // namespace t7