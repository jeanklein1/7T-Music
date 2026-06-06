#pragma once

// ─── context.hpp ─────────────────────────────────────────────────
//
// Per-channel context: the stateful home for one channel's reading
// windows. Owns the channel's MidiStream and the windows over it — the
// Playhead (present), the PreviousEvent (held prior onset-group), and a
// small bank of Wagons (spans of completed history). Routes incoming
// events and drives the per-frame update.
//
// This is where the Train's ownership role lands now that the Train is
// removed: the Context owns the windows. Composition (the operations DAG)
// and shipping (the serializer) live elsewhere — operation attachment is
// deliberately NOT here yet.
//
// Per-frame cycle (driven by the canvas):
//     for each event this frame:  ctx.receive(event)
//     ctx.update(beat)
//     ... read ctx.playhead(), ctx.previous(), ctx.wagon(i) ...
//
// Depends on: sources/midi_event.hpp, musical/midi_stream.hpp,
//             musical/playhead.hpp, musical/previous_event.hpp,
//             musical/wagon.hpp, <array>.

#include "sources/midi_event.hpp"
#include "musical/midi_stream.hpp"
#include "musical/playhead.hpp"
#include "musical/previous_event.hpp"
#include "musical/wagon.hpp"

#include <array>

namespace t7 {

constexpr int CONTEXT_MAX_WAGONS = 4;

class Context {
public:
    Context() = default;

    // Owns a MidiStream (non-copyable); the Context is non-copyable, movable.
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    // ── Configuration (setup time) ───────────────────────────────

    void set_channel(int channel) { stream_.set_channel(channel); }
    int  channel() const { return stream_.channel(); }

    void set_retention_beats(float beats) { stream_.set_retention_beats(beats); }
    void set_previous_tolerance(float beats) { previous_.set_tolerance(beats); }

    // Add a Wagon of the given span/offset. Returns its index, or -1 if full.
    int add_wagon(float span_beats, float offset_beats = 0.0f) {
        if (wagon_count_ >= CONTEXT_MAX_WAGONS) return -1;
        const int slot = wagon_count_++;
        wagons_[slot].set_span(span_beats);
        wagons_[slot].set_offset(offset_beats);
        return slot;
    }

    int wagon_count() const { return wagon_count_; }

    // ── Per-frame: receive events, then update ───────────────────

    // Route one event for this channel. Feeds the stream; a note-on also
    // feeds the PreviousEvent latch (which keys on onsets).
    void receive(const MidiEvent& ev) {
        stream_.receive(ev);
        if (ev.type == MidiEvent::NOTE_ON) {
            previous_.on_onset(ev.pitch, ev.velocity, ev.beat);
        }
    }

    // Advance the stream, then rebuild the windows. Call once per frame
    // after routing this frame's events.
    void update(float beat) {
        stream_.update(beat);

        // A backward time jump clears the stream; the held windows must
        // follow so they do not carry stale state across the discontinuity.
        if (stream_.had_time_discontinuity()) {
            playhead_.clear();
            previous_.clear();
        }

        const StreamSnapshot snap = stream_.snapshot();
        playhead_.update(snap);

        for (int i = 0; i < wagon_count_; ++i) {
            wagons_[i].update(stream_.history(), beat);
        }
    }

    // ── Readouts (read side) ─────────────────────────────────────

    const PlayheadReadout& playhead() const { return playhead_.readout(); }
    const PreviousEvent&   previous() const { return previous_; }

    const WagonReadout& wagon(int i) const {
        static const WagonReadout empty{};
        return (i >= 0 && i < CONTEXT_MAX_WAGONS) ? wagons_[i].readout() : empty;
    }

    const MidiStream& stream() const { return stream_; }

    // ── Reset ────────────────────────────────────────────────────

    void clear() {
        stream_.clear();
        playhead_.clear();
        previous_.clear();
        for (int i = 0; i < wagon_count_; ++i) wagons_[i].clear();
    }

private:
    MidiStream    stream_;
    Playhead      playhead_;
    PreviousEvent previous_;

    std::array<Wagon, CONTEXT_MAX_WAGONS> wagons_{};
    int wagon_count_ = 0;
};

} // namespace t7
