#pragma once

// ─── canvas.hpp ──────────────────────────────────────────────────
//
// The canvas: the per-channel assembly, and the surface the program meets
// the eye through. For each channel it reads it holds the spec that composes
// that channel and the Context realized from it; it routes the event flow to
// the right channel by the channel stamped on each event; and it publishes
// one signal — the named slot map a visual binds to. Reading the music and
// speaking the result are its two halves.
//
// The reading half: a channel is configured from a spec, its Context realized
// to match, the flow routed in, the views advanced each frame. The binding
// reads back — which spec, with which windows and memories, on which channel
// — so the composition is visible before a note flows.
//
// The speaking half is a declared list of published readings. The signal is
// fixed in size — MAX_CHANNELS bands of STATS_PER_CHANNEL slots — but the
// layout is the curated surface a render side binds to. The canvas holds the
// full capability to compute every reading; the published list is the
// contract, and what runs serves what is published, not the reverse: publish()
// walks the list, computes each declared reading from its source, and writes it
// to its address — nothing outside that closure is computed.
//
// A published reading names three things: the reading (Field, present_count,
// …), its source, and its address in the signal. Declaring one — publish_reading
// — gates two things together, its layout group and its slot-write, so the
// signal and the layout never disagree about what is present. Two rules hold
// the slots steady. A reading's slot is canonical (Field is slot 61 whether or
// not it is published, so opting one in or out never moves another); and a
// reading may be declared only if the analysis makes it available — the line
// readings require the spine, the window readings a wagon — so a declaration
// the spec cannot feed is refused.
//
// A source is one channel or a union of channels. A per-channel reading draws
// from a single channel's views and lands in that channel's band; a group
// reading draws from the union of several — combined by presence one level out,
// the same shape feeding the same field machinery untouched — and lands in a
// reserved band (the last index) carrying cross-channel readings, named without
// a voice prefix. So a reading can rise: rather than each channel publishing its
// own, a group publishes one, taken across the texture.
//
// The first instance: two voices subscribed (slot 0 ← MIDI 0, slot 1 ← MIDI 1),
// each a present and a four-beat window with the spine off — the spec pruning
// what the published set does not ask for. One reading is published: the field,
// source the union {0, 1}, computed from both voices' present-and-window sets,
// named `field` in the group band. No per-channel field; the per-channel held
// fields are never stepped. The other seven readings stay as capability in the
// modules, computed by nothing this round — each one a single publish_reading
// away. Every pitch-class vector is re-origined to D; lengths are in beats.
//
// The canvas IS an AnalysisCartridge. It owns its MIDI port, opens it in
// initialize(), and in update(dt) drains the port and advances on the DAW's
// own beat, read from the port — dt is wall-clock, kept only as the signal's
// t_seconds telemetry, never the musical clock. A render side (the_lab, the
// console) runs the canvas through the cartridge interface alone: initialize
// once, update each frame, read output() by stat_layout(). Tests drive the
// same frame directly through configure(), publish_reading(), route() and
// advance(beat), the port dormant.
//
// A channel here is an analysis slot, 0..MAX_CHANNELS-1, the same channel the
// signal publishes under. A slot's spec names the MIDI channel it listens to,
// which need not equal the slot; routing matches on that.
//
// Depends on: musical/context.hpp, musical/context_spec.hpp,
//             musical/context_realize.hpp, musical/pc_count.hpp,
//             musical/spine_ops.hpp, musical/field.hpp,
//             musical/vector_dressing.hpp, sources/midi_event.hpp,
//             sources/midi_port.hpp, analysis/analysis_signal.hpp,
//             analysis/analysis_cartridge.hpp, <array>, <cstdint>,
//             <initializer_list>.

#include "musical/context.hpp"
#include "musical/context_spec.hpp"
#include "musical/context_realize.hpp"
#include "musical/pc_count.hpp"
#include "musical/spine_ops.hpp"
#include "musical/field.hpp"
#include "musical/vector_dressing.hpp"
#include "sources/midi_event.hpp"
#include "sources/midi_port.hpp"
#include "analysis/analysis_signal.hpp"
#include "analysis/analysis_cartridge.hpp"

#include <array>
#include <cstdint>
#include <initializer_list>

namespace t7 {
namespace canvas_1 {

class Canvas : public AnalysisCartridge {
public:
    // ── The cartridge lifecycle ──────────────────────────────────
    //
    // What a render side calls. initialize() composes the canvas and opens its
    // port; update() drains the port and advances one frame on the DAW's beat;
    // on_input() is unused, the canvas's source being the DAW alone.

    void initialize(const char* asset_path) override {
        (void)asset_path;   // no asset to load; the DAW is the source
        // Two voices: slot 0 reads MIDI channel 0, slot 1 reads MIDI channel 1.
        // Each keeps the present and a four-beat trailing window; the spine is
        // left off (default_spec turns no memory on), since the published field
        // reads the present-and-window set, not the line — the spec pruning what
        // the published set does not ask for, so the spine is never built.
        configure(0, default_spec(/*midi*/ 0, /*window*/ 4.0f));
        configure(1, default_spec(/*midi*/ 1, /*window*/ 4.0f));

        // The sole published reading: the field, taken across the union of both
        // voices and published once in the reserved group band — no per-channel
        // field. The union is the contract's first and only tenant this round.
        publish_reading(Reading::Field, Source::group({0, 1}), "field");

        port_.open_by_name("loopMIDI");   // the DAW's virtual port
    }

    void update(float dt) override {
        dt_         = dt;          // wall-clock delta, telemetry only
        t_seconds_ += dt;
        const float beat = static_cast<float>(port_.beats());   // the DAW's clock
        MidiEvent ev[256];
        const int n = port_.poll(beat, ev, 256);
        for (int i = 0; i < n; ++i) route(ev[i]);
        advance(beat);
    }

    void on_input(const InputEvent&) override {
        // The canvas reads the DAW's MIDI port. The keyboard-debug source is
        // not part of the lean cartridge, so device input is ignored.
    }

    // ── Composition and frame (the seam tests drive directly) ────
    //
    // configure() composes one analysis slot; publish_reading() declares a
    // reading for publication; route() ingests one event; advance() is the
    // frame at a given beat. update() is built from the last two; a test reaches
    // them straight, without a port.

    // The readings the canvas can publish, and a reading's source: one channel,
    // or a union of several. The vocabulary of the published-set contract — a
    // composer (initialize, or a test) names a reading and a source to
    // publish_reading; a render side never needs them, resolving by name.
    enum class Reading : uint8_t {
        PresentCount, PresentLength, WindowCount, WindowLength,
        CurrentPC, Distance, Field, Polyphony
    };

    struct Source {
        uint32_t mask = 0;   // bit i set ⇒ channel i contributes

        static Source channel(int c) { return Source{ 1u << c }; }
        static Source group(std::initializer_list<int> cs) {
            uint32_t m = 0;
            for (int c : cs) m |= (1u << c);
            return Source{ m };
        }
        int  count() const {
            int n = 0;
            for (int i = 0; i < MAX_CHANNELS; ++i) if (mask & (1u << i)) ++n;
            return n;
        }
        bool is_group() const { return count() > 1; }
        int  first()    const {
            for (int i = 0; i < MAX_CHANNELS; ++i) if (mask & (1u << i)) return i;
            return -1;
        }
    };

    // Configure one analysis slot: store its spec and realize its Context. What
    // the slot publishes is declared separately, by publish_reading, so the
    // contract is opt-in. Realize once into the slot's fresh Context;
    // reconfiguring a used slot would accumulate windows, so a slot is
    // configured a single time.
    bool configure(int slot, const ContextSpec& spec) {
        if (slot < 0 || slot >= MAX_CHANNELS) return false;
        specs_[slot]  = spec;
        realize(spec, contexts_[slot]);
        active_[slot] = true;
        return true;
    }

    // Declare a reading for publication. The opt-in: it gates the layout group
    // and the slot-write together, so the signal and its map never disagree.
    // Refused — and nothing added — if the source's analysis cannot feed the
    // reading (a window or a line the spec did not build), the reading's writer
    // is not yet wired into the publish path, the source is empty, or the list is
    // full — so a declaration that could not be both computed AND written never
    // enters the contract. A per-channel source lands the reading in that
    // channel's band; a union lands it in the reserved group band. The name is
    // the render side's handle and must be static storage.
    bool publish_reading(Reading r, const Source& src, const char* name) {
        if (src.count() == 0)                  return false;
        if (!available(r, src))                return false;
        if (!writer_wired(r))                  return false;
        if (published_count_ >= MAX_PUBLISHED) return false;

        const ReadingSpec rs = reading_spec(r);
        const int band = src.is_group() ? GROUP_BAND : src.first();

        layout_[published_count_]    = StatGroup{ name, band, rs.slot, rs.width, rs.shape };
        published_[published_count_] = Published{ r, src.mask, band, HeldField{} };
        ++published_count_;
        return true;
    }

    // Route one event to the slot whose spec names its MIDI channel. The
    // dynamic half of the wiring; an event on a channel no slot listens to is
    // dropped. Both update()'s drain and a test's injection enter here.
    void route(const MidiEvent& ev) {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (active_[i] && specs_[i].channel == ev.channel) {
                contexts_[i].receive(ev);
                return;
            }
        }
    }

    // Advance every configured channel's views to `beat`, step the held fields,
    // then publish. The memories already moved in route(), so the views rebuild
    // from the present and the windows, the fields step once on the rebuilt
    // views, and publish() reads everything into the signal.
    void advance(float beat) {
        for (int i = 0; i < MAX_CHANNELS; ++i)
            if (active_[i]) contexts_[i].update(beat);
        step_fields();
        publish(beat);
    }

    // ── Speak — the published signal and its map ─────────────────
    //
    // The signal as last composed, and the layout naming the groups in it. A
    // render side receives the layout once and reads the signal each frame by
    // it. These two are the AnalysisCartridge output.

    const AnalysisSignal& output() const override { return output_; }

    StatLayoutView stat_layout() const override {
        return StatLayoutView{ layout_.data(), static_cast<uint32_t>(published_count_) };
    }

    // ── Read access — the binding, for inspection ────────────────

    bool               active(int slot)  const { return slot >= 0 && slot < MAX_CHANNELS && active_[slot]; }
    const ContextSpec& spec(int slot)    const { return specs_[slot]; }
    const Context&     context(int slot) const { return contexts_[slot]; }
    Context&           context(int slot)       { return contexts_[slot]; }
    bool               is_open()         const { return port_.is_open(); }
    const std::string& port_name()       const { return port_.port_name(); }

private:
    // ── The published-set contract (the canonical slot map) ──────
    //
    // A reading's canonical address-within-a-band and shape, the same whether
    // the reading is published or not — opting one in or out never moves
    // another. Sixty-three of the hundred-and-twenty-eight slots are spoken
    // for; the rest are room. The names are positional, since a channel is
    // whatever Ableton routes to it; the name a group is published under is
    // supplied at declaration, without a voice prefix.

    static constexpr int SLOT_PRESENT_COUNT  = 0;    // 12  present pcs, by count
    static constexpr int SLOT_PRESENT_LENGTH = 12;   // 12  present pcs, by length (beats)
    static constexpr int SLOT_WINDOW_COUNT   = 24;   // 12  present+window, by count
    static constexpr int SLOT_WINDOW_LENGTH  = 36;   // 12  present+window, by length (beats)
    static constexpr int SLOT_CURRENT_PC     = 48;   // 12  current note, one-hot
    static constexpr int SLOT_DISTANCE       = 60;   // 1   signed registral interval
    static constexpr int SLOT_FIELD          = 61;   // 1   held field index 1..6
    static constexpr int SLOT_POLYPHONY      = 62;   // 1   present voice count

    struct ReadingSpec { int slot; int width; StatShape shape; };

    static ReadingSpec reading_spec(Reading r) {
        switch (r) {
            case Reading::PresentCount:  return { SLOT_PRESENT_COUNT,  12, StatShape::Vector };
            case Reading::PresentLength: return { SLOT_PRESENT_LENGTH, 12, StatShape::Vector };
            case Reading::WindowCount:   return { SLOT_WINDOW_COUNT,   12, StatShape::Vector };
            case Reading::WindowLength:  return { SLOT_WINDOW_LENGTH,  12, StatShape::Vector };
            case Reading::CurrentPC:     return { SLOT_CURRENT_PC,     12, StatShape::Vector };
            case Reading::Distance:      return { SLOT_DISTANCE,        1, StatShape::Scalar };
            case Reading::Field:         return { SLOT_FIELD,           1, StatShape::Scalar };
            case Reading::Polyphony:     return { SLOT_POLYPHONY,       1, StatShape::Scalar };
        }
        return { 0, 1, StatShape::Scalar };   // unreachable; quiets the compiler
    }

    // What an analysis must supply for a reading to be available. The present is
    // always maintained, so present-only readings need nothing; the window
    // readings — and the field, which reads the present-and-window set — need a
    // wagon; the line readings need the spine.
    static bool reading_needs_window(Reading r) {
        return r == Reading::WindowCount || r == Reading::WindowLength
            || r == Reading::Field;
    }
    static bool reading_needs_spine(Reading r) {
        return r == Reading::CurrentPC || r == Reading::Distance;
    }

    // Whether a reading's value-writer is built. Only the field is wired into the
    // publish path this round (the lean scope); declaring a reading whose writer
    // does not exist yet would advertise a layout group whose slot never fills,
    // so the contract refuses it until write_reading handles it — keeping the
    // gate of layout and write a single, honest act. A new reading's case in
    // write_reading and its line here are added together.
    static bool writer_wired(Reading r) { return r == Reading::Field; }

    // One published reading: its kind, the channels it draws from, the band it
    // lands in, and — when it is a field — the held incumbent that is its one
    // piece of state. The held field lives with the entry, so only a published
    // field holds and steps; an unpublished one neither exists nor moves.
    struct Published {
        Reading   reading     = Reading::Field;
        uint32_t  source_mask = 0;
        int       band        = 0;
        HeldField field{};     // stateful; used only when reading == Field
    };

    // The reserved band for cross-channel (group) readings: the last index, the
    // home a group reading takes since it belongs to no single voice.
    static constexpr int GROUP_BAND = MAX_CHANNELS - 1;

    // The composition's home — the root every published vector re-origins to —
    // and the size of the field bank.
    static constexpr int PROJECT_PC_ORIGIN = 2;      // D
    static constexpr int BANK_SIZE         = 6;      // the six declared fields

    // The most readings publishable at once: each canonical slot, once per band.
    static constexpr int MAX_PUBLISHED = MAX_CHANNELS * 8;

    // Whether every channel a reading draws from supplies what it needs. A
    // source channel must be active and its spec must build the window or the
    // spine the reading reads; the present is always there.
    bool available(Reading r, const Source& src) const {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (!(src.mask & (1u << i))) continue;
            if (!active_[i]) return false;
            if (reading_needs_window(r) && !specs_[i].has_window())    return false;
            if (reading_needs_spine(r)  && !specs_[i].has_crossings()) return false;
        }
        return true;
    }

    // ── The frame's speaking ─────────────────────────────────────

    // Compose the signal for this frame: stamp the clocks, then write every
    // published reading to its address. The beat is the DAW's musical position;
    // t_seconds and dt are the wall-clock telemetry carried alongside it. Only
    // the published list is walked — an unpublished reading is neither written
    // nor computed, so the signal's other slots simply stay at their value-
    // initialized zero.
    void publish(float beat) {
        output_.t_beats   = beat;
        output_.t_seconds = t_seconds_;
        output_.dt        = dt_;

        for (int k = 0; k < published_count_; ++k)
            write_reading(published_[k]);
    }

    // Write one published reading to its band and canonical slot. Only the field
    // is wired this round — the lean scope: the other readings stay as
    // capability in the modules (pc_count, spine_ops, vector_dressing) and gain
    // their line here when first published. The field ships as a one-based rank;
    // silence reads as the top of the hierarchy, never zero.
    void write_reading(const Published& p) {
        const int slot = reading_spec(p.reading).slot;
        switch (p.reading) {
            case Reading::Field:
                output_.set_stat(p.band, slot, static_cast<float>(field_index(p.field)));
                break;
            default:
                // Unreachable: publish_reading refuses any reading not wired here
                // (see writer_wired). The other readings stay as capability in the
                // modules and gain their case when first published.
                break;
        }
    }

    // Write a twelve-component vector into a band's slot run — the writer the
    // vector readings use once they are published.
    void write_vector(int band, int slot_base, const PitchClassVector& v) {
        for (int pc = 0; pc < 12; ++pc)
            output_.set_stat(band, slot_base + pc, v.v[pc]);
    }

    // ── The field — the one step ─────────────────────────────────
    //
    // The bank: the six declared harmonic fields in hierarchy order (earlier
    // ranks higher; the order is the tiebreaker). Their degree-sets are the
    // conventional modes, root-relative. Composition policy, shared by every
    // channel, built once on first use.
    static const Field* bank() {
        static const Field b[BANK_SIZE] = {
            { "phrygian_dominant", field_mask({0, 1, 4, 5, 7, 8, 10}) },
            { "mixolydian",        field_mask({0, 2, 4, 5, 7, 9, 10}) },
            { "major",             field_mask({0, 2, 4, 5, 7, 9, 11}) },
            { "dorian",            field_mask({0, 2, 3, 5, 7, 9, 10}) },
            { "harmonic_minor",    field_mask({0, 2, 3, 5, 7, 8, 11}) },
            { "lydian_sharp2",     field_mask({0, 3, 4, 6, 7, 9, 11}) },
        };
        return b;
    }

    // Step each published field once this frame: union its source channels'
    // present-and-window sets into one set by presence, re-origin to D, and move
    // that field's held incumbent on a strict win. The canvas's one mutation a
    // frame; called in advance() after the views rebuild and before publish()
    // reads the incumbents. A per-channel field would step on a single channel's
    // set — none is published, so none steps.
    void step_fields() {
        for (int k = 0; k < published_count_; ++k) {
            Published& p = published_[k];
            if (p.reading != Reading::Field) continue;
            const PitchClassVector degrees =
                to_degrees(union_present_set(p.source_mask), PROJECT_PC_ORIGIN);
            p.field.step(degrees, bank(), BANK_SIZE);
        }
    }

    // The union of a source's present-and-window sets: gather each active source
    // channel's present_set and join them with present_union — the presence-OR
    // that lives beside the field. Binary in, binary out, the same shape a single
    // channel returns, so the field machinery one level out is untouched; the
    // rise from a voice to a group is invisible past this point.
    PitchClassVector union_present_set(uint32_t mask) const {
        PitchClassVector sets[MAX_CHANNELS];
        int n = 0;
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (!(mask & (1u << i)) || !active_[i]) continue;
            sets[n++] = present_set(contexts_[i].playhead(), contexts_[i].wagon(0));
        }
        return present_union(sets, n);
    }

    // The published field index: the incumbent as a one-based rank, or the top
    // of the hierarchy before any field has scored — so the index is always
    // 1..6 and silence never opens a phantom seventh.
    int field_index(const HeldField& hf) const {
        return (hf.incumbent < 0) ? 1 : hf.incumbent + 1;
    }

    std::array<ContextSpec, MAX_CHANNELS> specs_{};
    std::array<Context,     MAX_CHANNELS> contexts_{};
    std::array<bool,        MAX_CHANNELS> active_{};

    MidiPort port_;             // the DAW's MIDI port, owned and drained each frame
    float    t_seconds_ = 0.0f; // wall-clock accumulation, the signal's telemetry
    float    dt_        = 0.0f; // the last frame's wall-clock delta

    // The published signal and the contract over it: the list of published
    // readings (each carrying its own field state) and the parallel layout
    // naming their groups. The two grow together in publish_reading and share an
    // index, so the map and the writes can never disagree. The signal is
    // value-initialized, so every unpublished slot reads zero until written.
    AnalysisSignal output_{};
    std::array<Published, MAX_PUBLISHED> published_{};
    std::array<StatGroup, MAX_PUBLISHED> layout_{};
    int published_count_ = 0;
};

} // namespace canvas_1
} // namespace t7
