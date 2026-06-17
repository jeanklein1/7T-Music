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
// The speaking half: the canvas composes an AnalysisSignal and publishes a
// StatLayout over it. The signal carries the readings as flat float slots; the
// layout names them — which group sits in which channel's band, at which slot,
// of what width and shape — and is the whole contract a render side needs (it
// imports the generic StatGroup, never this header). output() returns the
// signal, stat_layout() the map.
//
// The canvas IS an AnalysisCartridge. It owns its MIDI port, opens it in
// initialize(), and in update(dt) drains the port and advances on the DAW's
// own beat, read from the port — dt is wall-clock, kept only as the signal's
// t_seconds telemetry, never the musical clock. A render side (the_lab, the
// console) runs the canvas through the cartridge interface alone: initialize
// once, update each frame, read output() by stat_layout(). Tests drive the
// same frame directly through route() and advance(beat), the port dormant.
//
// All eight readings flow: the polyphony; the present's content and the
// present-plus-window content — pitch classes by count and by length; the
// spine's line — the current note as a one-hot and the signed distance it
// moved; and the field — which of six declared harmonic fields the music sits
// in. The first seven are pure reads. The field is the one step: the canvas
// holds the bank of six masks in hierarchy order and a held field per channel,
// scores the present-plus-window against the bank once a frame in advance(),
// and moves the incumbent only when a field strictly out-scores it — holding
// through ambiguity and silence, and reading the top of the hierarchy until the
// first field scores. Every pitch-class vector is re-origined to D before it
// ships; lengths are in beats, the unit the clock already counts, so unscaled.
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
//             analysis/analysis_cartridge.hpp, <array>, <cstdint>.

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

namespace t7 {

class Canvas : public AnalysisCartridge {
public:
    // ── The cartridge lifecycle ──────────────────────────────────
    //
    // What a render side calls. initialize() composes the canvas and opens its
    // port; update() drains the port and advances one frame on the DAW's beat;
    // on_input() is unused, the canvas's source being the DAW alone.

    void initialize(const char* asset_path) override {
        (void)asset_path;   // no asset to load; the DAW is the source
        // The composition: two channels, MIDI 0 and MIDI 1, each computing the
        // full packet — present, four-beat window, the spine's line. One reading
        // is published, the field over the union of the two, at the group band.
        // The per-channel readings compute but stay unspoken; selecting the
        // source of what publishes is the published set's whole work.
        ContextSpec s0 = default_spec(/*midi*/ 0, /*window*/ 4.0f);
        ContextSpec s1 = default_spec(/*midi*/ 1, /*window*/ 4.0f);
        s0.crossings.active = true;
        s1.crossings.active = true;
        configure(0, s0);
        configure(1, s1);
        publish_field_global();
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
    // configure() composes one analysis slot; route() ingests one event;
    // advance() is the frame at a given beat. update() is built from the last
    // two; a test reaches them straight, without a port.

    // Configure one analysis slot: store its spec and realize its Context. The
    // slot now computes its full views every frame; what it publishes is a
    // separate act, so configuring a channel and speaking from it are distinct.
    // Realize once into the fresh Context; reconfiguring a used slot would
    // accumulate windows, so a slot is configured a single time.
    bool configure(int slot, const ContextSpec& spec) {
        if (slot < 0 || slot >= MAX_CHANNELS) return false;
        specs_[slot]  = spec;
        realize(spec, contexts_[slot]);
        active_[slot] = true;
        return true;
    }

    // Declare this composition's published reading: the field over the union of
    // the active channels, written to the group band's field slot and named
    // without a voice prefix, since it speaks for the group rather than a
    // channel. The one entry the layout carries while the published set is this.
    void publish_field_global() {
        layout_[layout_count_++] =
            StatGroup{ "field", GROUP_BAND, SLOT_FIELD, 1, StatShape::Scalar };
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
    // from the present and the windows, the field steps once on the rebuilt
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
        return StatLayoutView{ layout_.data(), static_cast<uint32_t>(layout_count_) };
    }

    // ── Read access — the binding, for inspection ────────────────

    bool               active(int slot)  const { return slot >= 0 && slot < MAX_CHANNELS && active_[slot]; }
    const ContextSpec& spec(int slot)    const { return specs_[slot]; }
    const Context&     context(int slot) const { return contexts_[slot]; }
    Context&           context(int slot)       { return contexts_[slot]; }
    bool               is_open()         const { return port_.is_open(); }
    const std::string& port_name()       const { return port_.port_name(); }

private:
    // ── Published slot map (per channel; one 0..STATS_PER_CHANNEL band) ─
    //
    // Where each reading lands in a channel's band. The full packet is declared
    // so the shape is fixed; the names are positional, since a channel is
    // whatever Ableton routes to it. Sixty-three of the hundred-and-twenty-eight
    // slots are spoken for; the rest are room.

    static constexpr int SLOT_PRESENT_COUNT  = 0;    // 12  present pcs, by count
    static constexpr int SLOT_PRESENT_LENGTH = 12;   // 12  present pcs, by length (beats)
    static constexpr int SLOT_WINDOW_COUNT   = 24;   // 12  present+window, by count
    static constexpr int SLOT_WINDOW_LENGTH  = 36;   // 12  present+window, by length (beats)
    static constexpr int SLOT_CURRENT_PC     = 48;   // 12  current note, one-hot
    static constexpr int SLOT_DISTANCE       = 60;   // 1   signed registral interval
    static constexpr int SLOT_FIELD          = 61;   // 1   held field index 1..6
    static constexpr int SLOT_POLYPHONY      = 62;   // 1   present voice count

    // The composition's home — the root every published vector re-origins to —
    // and the size of the field bank.
    static constexpr int PROJECT_PC_ORIGIN = 2;      // D
    static constexpr int BANK_SIZE         = 6;      // the six declared fields
    static constexpr int GROUP_BAND        = MAX_CHANNELS - 1;  // the band group readings publish to

    // Group names, per slot — positional, unique to (channel, reading) so the
    // render side resolves without collision. Static, so the layout may hold
    // pointers into them for the program's life.
    static constexpr const char* NAME_PRESENT_COUNT[MAX_CHANNELS] = {
        "ch0.present_count", "ch1.present_count", "ch2.present_count", "ch3.present_count"
    };
    static constexpr const char* NAME_PRESENT_LENGTH[MAX_CHANNELS] = {
        "ch0.present_length", "ch1.present_length", "ch2.present_length", "ch3.present_length"
    };
    static constexpr const char* NAME_WINDOW_COUNT[MAX_CHANNELS] = {
        "ch0.window_count", "ch1.window_count", "ch2.window_count", "ch3.window_count"
    };
    static constexpr const char* NAME_WINDOW_LENGTH[MAX_CHANNELS] = {
        "ch0.window_length", "ch1.window_length", "ch2.window_length", "ch3.window_length"
    };
    static constexpr const char* NAME_CURRENT_PC[MAX_CHANNELS] = {
        "ch0.current_pc", "ch1.current_pc", "ch2.current_pc", "ch3.current_pc"
    };
    static constexpr const char* NAME_DISTANCE[MAX_CHANNELS] = {
        "ch0.distance", "ch1.distance", "ch2.distance", "ch3.distance"
    };
    static constexpr const char* NAME_FIELD[MAX_CHANNELS] = {
        "ch0.field", "ch1.field", "ch2.field", "ch3.field"
    };
    static constexpr const char* NAME_POLYPHONY[MAX_CHANNELS] = {
        "ch0.polyphony", "ch1.polyphony", "ch2.polyphony", "ch3.polyphony"
    };

    // Declare a channel's full per-reading packet to the layout, in the band's
    // order. The capability a wider published set would call to speak a voice's
    // own readings; unused while the published set is the group field alone, kept
    // so that widening the set is adding a call, not writing the writer.
    void register_channel(int slot) {
        layout_[layout_count_++] = StatGroup{ NAME_PRESENT_COUNT[slot],  slot, SLOT_PRESENT_COUNT,  12, StatShape::Vector };
        layout_[layout_count_++] = StatGroup{ NAME_PRESENT_LENGTH[slot], slot, SLOT_PRESENT_LENGTH, 12, StatShape::Vector };
        layout_[layout_count_++] = StatGroup{ NAME_WINDOW_COUNT[slot],   slot, SLOT_WINDOW_COUNT,   12, StatShape::Vector };
        layout_[layout_count_++] = StatGroup{ NAME_WINDOW_LENGTH[slot],  slot, SLOT_WINDOW_LENGTH,  12, StatShape::Vector };
        layout_[layout_count_++] = StatGroup{ NAME_CURRENT_PC[slot],     slot, SLOT_CURRENT_PC,     12, StatShape::Vector };
        layout_[layout_count_++] = StatGroup{ NAME_DISTANCE[slot],       slot, SLOT_DISTANCE,        1,  StatShape::Scalar };
        layout_[layout_count_++] = StatGroup{ NAME_FIELD[slot],          slot, SLOT_FIELD,           1,  StatShape::Scalar };
        layout_[layout_count_++] = StatGroup{ NAME_POLYPHONY[slot],      slot, SLOT_POLYPHONY,       1,  StatShape::Scalar };
    }

    // Compose the signal for this frame: stamp the clocks, then write the
    // published readings. The published set here is one reading — the group
    // field — so publish writes that alone, at the group band's field slot, the
    // value overwritten every frame so the signal stays fresh without a clear.
    // The beat is the DAW's musical position; t_seconds and dt the wall-clock
    // telemetry carried beside it. The per-channel writer below is the capability
    // a wider set would call; here it sits unused.
    void publish(float beat) {
        output_.t_beats   = beat;
        output_.t_seconds = t_seconds_;
        output_.dt        = dt_;

        output_.set_stat(GROUP_BAND, SLOT_FIELD, static_cast<float>(group_field_index()));
    }

    // Write one channel's full per-reading packet to its band: the present off
    // the playhead, the present-plus-window off the playhead summed with the
    // primary wagon, the line off the spine — current note dressed to D as a
    // one-hot, distance a bare signed scalar, the vectors twelve slots each, the
    // field a one-based rank, the polyphony the present voice count. Paired with
    // register_channel(); both wait for a published set that names the voice.
    void write_channel(int i) {
        const VectorDressing to_D{ /*reorigin*/ true, PROJECT_PC_ORIGIN, VectorDressing::Scale::None };
        const Context&         ctx = contexts_[i];
        const PlayheadReadout&  ph = ctx.playhead();
        const WagonReadout&     wg = ctx.wagon(0);   // the primary trailing window
        const Spine&            sp = ctx.spine();    // the elected line

        write_vector(i, SLOT_PRESENT_COUNT,  dress(pc_count (ph),     to_D));
        write_vector(i, SLOT_PRESENT_LENGTH, dress(pc_length(ph),     to_D));
        write_vector(i, SLOT_WINDOW_COUNT,   dress(pc_count (ph, wg), to_D));
        write_vector(i, SLOT_WINDOW_LENGTH,  dress(pc_length(ph, wg), to_D));
        write_vector(i, SLOT_CURRENT_PC,     dress(current_note(sp),  to_D));
        output_.set_stat(i, SLOT_DISTANCE,   static_cast<float>(line_distance(sp)));
        output_.set_stat(i, SLOT_FIELD,      static_cast<float>(field_index(i)));
        output_.set_stat(i, SLOT_POLYPHONY,  static_cast<float>(ph.current_count));
    }

    // Write a twelve-component vector into a channel's slot band.
    void write_vector(int channel, int slot_base, const PitchClassVector& v) {
        for (int pc = 0; pc < 12; ++pc)
            output_.set_stat(channel, slot_base + pc, v.v[pc]);
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

    // Step the held fields once this frame, after the views rebuild and before
    // publish reads them — the canvas's mutation a frame. Each channel's own
    // field steps on its present-plus-window set (computed and offered, though
    // unpublished here), and the group field steps on the union of the active
    // channels, which is the reading this composition speaks.
    void step_fields() {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            if (!active_[i]) continue;
            const Context& ctx = contexts_[i];
            const PitchClassVector degrees =
                to_degrees(present_set(ctx.playhead(), ctx.wagon(0)), PROJECT_PC_ORIGIN);
            held_field_[i].step(degrees, bank(), BANK_SIZE);
        }
        step_group_field();
    }

    // The group field: union the active channels' present-sets into one set,
    // re-origin to D, and run the very election a single channel runs. The source
    // is widened from a voice to the group; the machinery downstream is untouched,
    // which is the whole reason the union lives as a vector op and not here.
    void step_group_field() {
        PitchClassVector sets[MAX_CHANNELS];
        int n = 0;
        for (int i = 0; i < MAX_CHANNELS; ++i)
            if (active_[i])
                sets[n++] = present_set(contexts_[i].playhead(), contexts_[i].wagon(0));
        const PitchClassVector degrees =
            to_degrees(present_union(sets, n), PROJECT_PC_ORIGIN);
        group_field_.step(degrees, bank(), BANK_SIZE);
    }

    // A held field as a one-based rank, or the top of the hierarchy before any
    // field has scored — so the index is always 1..6 and silence never opens a
    // phantom seventh. The per-channel form reads a voice's incumbent; the group
    // form reads the union's.
    int field_index(int channel) const {
        const int inc = held_field_[channel].incumbent;
        return (inc < 0) ? 1 : inc + 1;
    }

    int group_field_index() const {
        const int inc = group_field_.incumbent;
        return (inc < 0) ? 1 : inc + 1;
    }

    std::array<ContextSpec, MAX_CHANNELS> specs_{};
    std::array<Context,     MAX_CHANNELS> contexts_{};
    std::array<bool,        MAX_CHANNELS> active_{};
    std::array<HeldField,   MAX_CHANNELS> held_field_{};   // per-channel field, offered
    HeldField                             group_field_{};  // the group field, published

    MidiPort port_;             // the DAW's MIDI port, owned and drained each frame
    float    t_seconds_ = 0.0f; // wall-clock accumulation, the signal's telemetry
    float    dt_        = 0.0f; // the last frame's wall-clock delta

    // The published signal and the map naming its groups. The layout grows as
    // channels are configured; it is sized for the full packet on every channel
    // so the view's backing never moves. The signal is value-initialized, so
    // every slot reads zero until a publish writes it.
    AnalysisSignal output_{};
    static constexpr int GROUPS_PER_CHANNEL = 8;
    // Sized for the full per-channel packet on every band plus a band's worth of
    // group readings, so the view's backing never moves however the set widens.
    std::array<StatGroup, MAX_CHANNELS * GROUPS_PER_CHANNEL + GROUPS_PER_CHANNEL> layout_{};
    int layout_count_ = 0;
};

} // namespace t7
