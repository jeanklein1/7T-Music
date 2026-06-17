#pragma once

// ─── context_spec.hpp ────────────────────────────────────────────
//
// The composition of one channel's context, written as data. A Context
// owns the windows and memories a channel reads through; this spec says
// which ones, and with what parameters, in a form the canvas can read
// before it builds anything. The canvas holds one spec per channel, and
// before it runs an operation it asks the spec whether the channel
// supplies what the operation needs — so the description has to stand on
// its own, ahead of the Context it gives rise to.
//
// A composition divides in two, because a channel's material does. The
// view layer is the stream read as a slice: the present, the notes
// sounding now, and a small bank of windows, each a trailing depth of the
// completed past. A view keeps nothing of its own — snapshot it from the
// stream at any instant and it is whole — so it is taken fresh each frame.
// The memory layer is the stream read as a succession that outlasts the
// slice: the previous-event, which latches the last onset-group, and the
// spine, which latches the elected previous voice of a line. A memory
// cannot be snapshotted into being; it is fed the event flow and
// accumulates, and it holds its value across the silence a slice would
// simply find empty.
//
// The default floor is a present and one window, the memory layer empty:
// enough for the measures and selections that read only the present and
// its window, and silent about succession until a channel asks for it.
// The window's depth is a per-channel quantity — there is no shared span
// constant; one channel's bar is not another's.
//
// One clock. Every quantity here is in beats, the clock the Context
// already runs on: the windows clip in beats, the stream retains in beats,
// and the two memories group in beats — the width within which onsets
// fuse into one event, or offsets into one cohort. The spine was first
// built and probed on a separate clock of arrival seconds; owned by the
// Context it is fed the beat clock with the rest, and its grouping becomes
// musical rather than perceptual. That this makes simultaneity scale with
// tempo is deliberate, of a piece with windows measured in beats; a
// perceptual seconds-grouping is the alternative the slot leaves open.
//
// This file is data only — it includes none of the views or memories it
// describes. The realizer that turns a spec into a live Context, and the
// table that says which operation needs which layer, are separate.
//
// Usage:
//   ContextSpec s = default_spec(/*channel*/ 2, /*window beats*/ 4.0f);
//   s.add_window(8.0f, 4.0f);          // a second, deeper, offset window
//   s.crossings.active = true;         // turn the spine on (beats default)
//   if (s.has_crossings()) { /* the canvas may run a step here */ }
//
// Depends on: <array>, <cstdint>.

#include <array>
#include <cstdint>

namespace t7 {

// ═══ LIMITS ══════════════════════════════════════════════════════

// A channel carries at most this many windows. Matches CONTEXT_MAX_WAGONS;
// the realizer maps each active window onto one Wagon.
constexpr int SPEC_MAX_WINDOWS = 4;

// ═══ VIEW LAYER ══════════════════════════════════════════════════

// A window the channel reads: a trailing slice of the completed stream,
// clipped to [anchor - offset - span, anchor - offset]. The span is the
// window's depth and is a per-channel quantity, never a shared constant;
// the offset slides the whole window back from the present. Both in beats,
// to match Context::add_wagon(span_beats, offset_beats).
struct WindowSpec {
    float span_beats   = 0.0f;   // the window's depth
    float offset_beats = 0.0f;   // how far back the window's near edge sits
    bool  active       = false;  // whether this slot is in use
};

// ═══ MEMORY LAYER ════════════════════════════════════════════════

// The binding the spine consults when the music alone cannot decide the
// previous — the last key of the cascade, selecting one note from a tie.
// Lowest is the only binding drawn so far; the slot is open by design, and
// the alternatives are named here as they are bound, never left implicit.
// The realizer maps a binding to one of the spine's injectable oracles.
enum class OracleBinding : uint8_t {
    Lowest = 0,   // the lowest pitch of the tie (Spine::oracle_lowest)
    // Highest, Loudest, Newest, ... — open, not yet bound.
};

// The previous-event memory: a latch over onset-groups. Onsets falling
// within `tolerance_beats` of one another fuse into a single event, and
// the latch holds the last such group across the silence that follows.
struct EventMemorySpec {
    bool  active         = false;
    float tolerance_beats = 0.1f;   // the onset-grouping (simultaneity) width
};

// The spine: a latch over the line. It elects the previous voice at the
// crossings through cardinality one and holds the resolved election across
// silence. `tolerance_beats` is the grouping width the final-offset cohort
// and the synchronous-onset test share; `oracle` is the last-resort
// binding consulted when neither survival nor a recorded mint decides.
struct CrossingMemorySpec {
    bool          active         = false;
    float         tolerance_beats = 0.05f;                // cohort / synchronous width
    OracleBinding oracle         = OracleBinding::Lowest;
};

// ═══ THE COMPOSITION ═════════════════════════════════════════════

// The composition of one channel: a view layer (a present, a bank of
// windows) and a memory layer (the previous-event, the spine). All of it
// is data; the queries near the end are what the canvas asks to learn
// whether a channel provides what an operation requires, before the
// operation is allowed to run.
struct ContextSpec {
    // The MIDI channel this composition reads.
    int channel = 0;

    // How much completed history the stream retains, in beats. It must
    // cover the deepest window; required_retention_beats() reports that
    // floor, and default_spec() seeds it from the windows declared.
    float stream_retention_beats = 0.0f;

    // ── View layer ───────────────────────────────────────────────
    // The present has no parameters — it simply reads the notes sounding
    // now — so it is a flag, not a struct. It is the floor, and droppable:
    // a window-only channel is coherent, purely retrospective, but it
    // forfeits the present half of every combine.
    bool present = true;
    std::array<WindowSpec, SPEC_MAX_WINDOWS> windows{};

    // ── Memory layer ─────────────────────────────────────────────
    // Both off by default. Either may be turned on; each is then fed the
    // complete event flow — onsets and offsets, in arrival order, on the
    // beat clock — and holds across silence.
    EventMemorySpec    event;       // the previous-event latch
    CrossingMemorySpec crossings;   // the spine

    // ── Construction helper ──────────────────────────────────────

    // Fill the next free window slot. Returns false if the bank is full.
    bool add_window(float span_beats, float offset_beats = 0.0f) {
        for (auto& w : windows) {
            if (!w.active) {
                w = WindowSpec{span_beats, offset_beats, true};
                return true;
            }
        }
        return false;
    }

    // ── Queries the canvas checks an operation against ───────────

    bool has_present()   const { return present; }
    bool has_event()     const { return event.active; }
    bool has_crossings() const { return crossings.active; }

    int window_count() const {
        int n = 0;
        for (const auto& w : windows) if (w.active) ++n;
        return n;
    }
    bool has_window() const { return window_count() > 0; }

    // The retention the windows demand: the farthest near-edge plus depth.
    float required_retention_beats() const {
        float deepest = 0.0f;
        for (const auto& w : windows) {
            if (w.active) {
                const float reach = w.offset_beats + w.span_beats;
                if (reach > deepest) deepest = reach;
            }
        }
        return deepest;
    }
};

// ═══ DEFAULT FLOOR ═══════════════════════════════════════════════

// The default composition: a present and a single window, no memories.
// The window's depth is supplied per channel — there is no built-in span.
inline ContextSpec default_spec(int channel, float window_span_beats) {
    ContextSpec s;
    s.channel = channel;
    s.present = true;
    s.add_window(window_span_beats);
    s.stream_retention_beats = s.required_retention_beats();
    return s;
}

} // namespace t7
