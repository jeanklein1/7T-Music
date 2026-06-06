#pragma once

// ─── wagon.hpp ───────────────────────────────────────────────────
//
// Completed-pairs-within-a-span context. The Wagon registers a temporary
// history of finished notes over a window [anchor − span, anchor], each
// clipped to the window so its in-window span is exact. Completed-only —
// a note is either active or completed, never both, so the Wagon (past)
// and the Playhead (present) are disjoint and their contributions add
// without double-counting.
//
// The active contribution to any windowed read comes from the Playhead,
// summed at the combine — NOT from the Wagon. There is no include_active
// path here: reaching into active notes would double-count against the
// Playhead and break the additive separation.
//
// Straddling completed notes (started before the window, ended inside) are
// the Wagon's job: they are included and clipped, which is what lets held
// notes that have since ended register their in-window length. A note still
// sounding is active, and belongs to the Playhead, not here.
//
// Reworked against target spec §6. Removed from the prior version:
// include_active (the trap), include_straddling (overlap + clip is now the
// single behaviour; inside-only, if ever wanted, is a downstream filter),
// and update_period (rebuild every frame — cheap, and no stale readout to
// complicate the sync point).
//
// Depends on: musical/stream_data.hpp (CompletedRing, CompletedNote),
//             <array>, <cstdint>.

#include "musical/stream_data.hpp"
#include <array>
#include <cstdint>

namespace t7 {

constexpr int WAGON_MAX_NOTES = 128;

// ═══ WINDOW NOTE ═════════════════════════════════════════════════
//
// A completed note seen through the window. window_onset / window_offset
// are the original times clipped to the window bounds; window_duration is
// the exact in-window span that length-weighted reads use.

struct WindowNote {
    uint8_t pitch = 0;
    uint8_t _pad[3] = {0, 0, 0};
    float   velocity = 0.0f;
    float   onset_beat = 0.0f;
    float   offset_beat = 0.0f;
    float   window_onset = 0.0f;     // max(onset_beat, window_start)
    float   window_offset = 0.0f;    // min(offset_beat, window_end)

    float duration() const { return offset_beat - onset_beat; }
    float window_duration() const { return window_offset - window_onset; }
    int   pitch_class() const { return pitch % 12; }
    int   octave() const { return pitch / 12; }

    bool straddles_start() const { return window_onset  > onset_beat; }
    bool straddles_end()   const { return window_offset < offset_beat; }
    bool entirely_inside() const { return !straddles_start() && !straddles_end(); }
};

static_assert(sizeof(WindowNote) == 24, "WindowNote should be 24 bytes");

// ═══ WAGON READOUT ═══════════════════════════════════════════════

struct WagonReadout {
    float anchor_beat = 0.0f;    // window end  (current_beat − offset)
    float window_start = 0.0f;   // anchor − span
    float window_end = 0.0f;     // anchor
    float span = 0.0f;
    float offset = 0.0f;

    std::array<WindowNote, WAGON_MAX_NOTES> notes{};
    int  note_count = 0;
    bool overflow = false;       // more than WAGON_MAX_NOTES in the window
    int  entirely_inside_count = 0;
    int  straddling_count = 0;

    bool empty() const { return note_count == 0; }
    bool has_notes() const { return note_count > 0; }
    bool has_overflow() const { return overflow; }

    void clear() {
        anchor_beat = window_start = window_end = span = offset = 0.0f;
        note_count = 0;
        overflow = false;
        entirely_inside_count = straddling_count = 0;
    }
};

// ═══ WAGON ═══════════════════════════════════════════════════════

class Wagon {
public:
    Wagon() { readout_.clear(); }

    explicit Wagon(float span, float offset = 0.0f)
        : span_(span), offset_(offset) {
        readout_.clear();
    }

    void  set_span(float beats) { span_ = beats; }
    float span() const { return span_; }

    void  set_offset(float beats) { offset_ = beats; }
    float offset() const { return offset_; }

    // Rebuild the window from history. Reads completed notes only.
    void update(const CompletedRing& history, float current_beat) {
        const float anchor = current_beat - offset_;
        rebuild(history, anchor);
    }

    const WagonReadout& readout() const { return readout_; }

    void clear() { readout_.clear(); }

private:
    float span_ = 0.0f;
    float offset_ = 0.0f;
    WagonReadout readout_;

    void rebuild(const CompletedRing& history, float anchor) {
        const float ws = anchor - span_;
        const float we = anchor;

        readout_.anchor_beat = anchor;
        readout_.window_start = ws;
        readout_.window_end = we;
        readout_.span = span_;
        readout_.offset = offset_;
        readout_.note_count = 0;
        readout_.overflow = false;
        readout_.entirely_inside_count = 0;
        readout_.straddling_count = 0;

        history.for_each_overlapping_window(ws, we, [&](const CompletedNote& cn) {
            add_note(cn, ws, we);
        });
    }

    void add_note(const CompletedNote& cn, float ws, float we) {
        if (readout_.note_count >= WAGON_MAX_NOTES) {
            readout_.overflow = true;
            return;
        }
        WindowNote& wn   = readout_.notes[readout_.note_count];
        wn.pitch         = cn.pitch;
        wn.velocity      = cn.velocity;
        wn.onset_beat    = cn.onset_beat;
        wn.offset_beat   = cn.offset_beat;
        wn.window_onset  = cn.onset_beat  < ws ? ws : cn.onset_beat;
        wn.window_offset = cn.offset_beat > we ? we : cn.offset_beat;

        if (wn.straddles_start() || wn.straddles_end()) ++readout_.straddling_count;
        else                                            ++readout_.entirely_inside_count;

        ++readout_.note_count;
    }
};

} // namespace t7
