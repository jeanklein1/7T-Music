#pragma once

// ─── context.hpp ─────────────────────────────────────────────────
//
// Per-channel context: the stateful home for one channel's reading. It
// owns the channel's MidiStream and reads through it in two layers. The
// view layer is the stream as a slice — the Playhead (the present) and a
// small bank of Wagons (trailing windows of completed history) — rebuilt
// each frame from the stream. The memory layer is the stream as a
// succession — the PreviousEvent (the latched prior onset-group) and the
// Spine (the elected previous voice of a line) — fed the event flow and
// holding its value across silence. Both memories are opt-in; a bare
// context is views only.
//
// The two layers update on different occasions, because they are
// different kinds. A view is stateless: it is rebuilt from the stream in
// update(), each frame, whole. A memory accumulates: it is fed in
// receive(), per event, and update() leaves it untouched. So the present
// and the windows refresh every frame while the memories simply persist
// between the events that move them.
//
// Routing feeds the whole flow. Every event reaches the stream; onsets
// and offsets both reach whichever memories are enabled — onsets open and
// group, offsets record the releases the elections read. The previous
// event needs its offsets to know which voice lingered to the seam; the
// spine needs both onsets and offsets to see its crossings.
//
// This is where the Train's ownership role lands now that the Train is
// removed: the Context owns the windows and the memories. Composition
// (the operations DAG) and shipping (the serializer) live elsewhere —
// operation attachment is deliberately NOT here yet, and the spec-to-
// context realizer is its own concern (context_realize.hpp).
//
// Per-frame cycle (driven by the canvas):
//     for each event this frame:  ctx.receive(event)
//     ctx.update(beat)
//     ... read ctx.playhead(), ctx.previous(), ctx.spine(), ctx.wagon(i) ...
//
// One clock: every time the context handles is the beat clock the canvas
// runs on. The windows clip in beats, and the two memories group in beats.
//
// Depends on: sources/midi_event.hpp, musical/midi_stream.hpp,
//             musical/playhead.hpp, musical/previous_event.hpp,
//             musical/wagon.hpp, musical/spine.hpp, <array>.

#include "sources/midi_event.hpp"
#include "musical/midi_stream.hpp"
#include "musical/playhead.hpp"
#include "musical/previous_event.hpp"
#include "musical/wagon.hpp"
#include "musical/spine.hpp"

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

    // Add a Wagon of the given span/offset. Returns its index, or -1 if full.
    int add_wagon(float span_beats, float offset_beats = 0.0f) {
        if (wagon_count_ >= CONTEXT_MAX_WAGONS) return -1;
        const int slot = wagon_count_++;
        wagons_[slot].set_span(span_beats);
        wagons_[slot].set_offset(offset_beats);
        return slot;
    }

    int wagon_count() const { return wagon_count_; }

    // Turn on the previous-event memory (off by default) and set its
    // onset-grouping tolerance, in beats.
    void enable_previous(float tolerance_beats) {
        previous_active_ = true;
        previous_.set_tolerance(tolerance_beats);
    }

    // Turn on the spine memory (off by default), set its grouping
    // tolerance in beats, and bind its oracle (lowest by default).
    void enable_spine(float tolerance_beats,
                      Spine::Oracle oracle = &Spine::oracle_lowest) {
        spine_active_ = true;
        spine_.set_tolerance(tolerance_beats);
        spine_.set_oracle(oracle);
    }

    // ── Per-frame: receive events, then update ───────────────────

    // Route one event for this channel. The stream takes every event; the
    // enabled memories take onsets and offsets both — onsets to open and
    // group, offsets to record the releases their elections read.
    void receive(const MidiEvent& ev) {
        stream_.receive(ev);

        const bool is_on = (ev.type == MidiEvent::NOTE_ON);

        if (previous_active_) {
            if (is_on) previous_.on_onset(ev.pitch, ev.velocity, ev.beat);
            else       previous_.on_offset(ev.pitch, ev.beat);
        }
        if (spine_active_) {
            if (is_on) spine_.on_onset(ev.pitch, ev.beat);
            else       spine_.on_offset(ev.pitch, ev.beat);
        }
    }

    // Advance the stream, then rebuild the views. Call once per frame
    // after routing this frame's events. The memories are not rebuilt
    // here — they moved in receive() and hold otherwise.
    void update(float beat) {
        stream_.update(beat);

        // A backward time jump clears the stream; the views and memories
        // must follow so they do not carry stale state across the
        // discontinuity.
        if (stream_.had_time_discontinuity()) {
            playhead_.clear();
            previous_.clear();
            spine_.clear();
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
    const Spine&           spine() const { return spine_; }

    const WagonReadout& wagon(int i) const {
        static const WagonReadout empty{};
        return (i >= 0 && i < CONTEXT_MAX_WAGONS) ? wagons_[i].readout() : empty;
    }

    const MidiStream& stream() const { return stream_; }

    // Which memories this context keeps — the read side checks these the
    // way the canvas checks a spec, before reading a memory's readout.
    bool has_previous() const { return previous_active_; }
    bool has_spine() const { return spine_active_; }

    // ── Reset ────────────────────────────────────────────────────

    void clear() {
        stream_.clear();
        playhead_.clear();
        previous_.clear();
        spine_.clear();
        for (int i = 0; i < wagon_count_; ++i) wagons_[i].clear();
    }

private:
    MidiStream    stream_;
    Playhead      playhead_;
    PreviousEvent previous_;
    Spine         spine_;

    std::array<Wagon, CONTEXT_MAX_WAGONS> wagons_{};
    int  wagon_count_ = 0;

    bool previous_active_ = false;
    bool spine_active_    = false;
};

} // namespace t7
