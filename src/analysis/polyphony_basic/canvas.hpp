#pragma once

// ─── canvas.hpp ──────────────────────────────────────────────────
//
// polyphony_basic — analysis cartridge. Embodies the pitch-class
// foundation: the lowest note (the reign) and the pitch class set (the
// union of windowed territory) are the current reading; legacy polyphony
// is retired to a keyboard debug path.
//
// Sources:
//   MidiPort    — external MIDI input via system port (loopMIDI / DAW)
//   MidiFile    — plays a MIDI file in a loop
//   KeyboardMidi— computer keyboard as piano (legacy debug)
//
// Channels:
//   "abbott"   on Ableton 1 (internal 0) — reign + territory
//   "costello" on Ableton 2 (internal 1) — reign + territory
//   "louise"   on Ableton 3 (internal 2) — territory only (no lowest)
//   "keyboard" on Ableton 4 (internal 3) — legacy debug, polyphony only
//
// The reads (all D-relative, PROJECT_PC_ORIGIN):
//   lowest-PC one-hot (abbott, costello) — published per channel
//   windowed pc length (all three)       — INTERMEDIATE; its presence
//                                          feeds the set, not published
//   the set (union)                      — active basis vectors: 1.0 if a
//                                          pitch class sounds in ANY of the
//                                          three windows, else 0.0; the
//                                          lossy compression is the closing
//                                          step, after all channels read
//   polyphony (keyboard)                 — published, debug
//
// Each channel owns a MidiStream and a list of attached Trains. Every
// incoming event is dispatched to the channel matching its MIDI channel
// byte; events for unregistered channels are silently dropped.
//
// Output format is declared by STAT_LAYOUT (the single source of truth
// for slot assignments and shapes, consumed by the_lab).
//
// Depends on: analysis/analysis_cartridge.hpp, analysis/analysis_signal.hpp,
//             core/clock.hpp, the sources/*, musical/midi_stream.hpp,
//             musical/playhead.hpp, musical/train.hpp, musical/musical_ops.hpp.

#include "analysis/analysis_cartridge.hpp"
#include "analysis/analysis_signal.hpp"
#include "core/clock.hpp"
#include "sources/midi_event.hpp"
#include "sources/midi_file.hpp"
#include "sources/keyboard_midi.hpp"
#include "sources/midi_port.hpp"
#include "musical/midi_stream.hpp"
#include "musical/playhead.hpp"
#include "musical/train.hpp"
#include "musical/musical_ops.hpp"

#include <array>
#include <iostream>
#include <string>

namespace t7 {
namespace polyphony_basic {

// ═══ CONSTANTS ═══════════════════════════════════════════════════

constexpr int MAX_NAMED_CHANNELS     = 4;
constexpr int MAX_TRAINS_PER_CHANNEL = 4;

constexpr float TERRITORY_WAGON_SPAN_BEATS   = 4.0f;  // 1 bar in 4/4
constexpr float TERRITORY_WAGON_PERIOD_BEATS = 1.0f;  // update per beat

constexpr int PROJECT_PC_ORIGIN = 2;  // D (C-origin index of D)

constexpr int PC_COUNT = 12;  // the twelve pitch classes (the basis)

// ═══ STAT SLOT DEFINITIONS ═══════════════════════════════════════
//
// This cartridge's published output format. Visualization cartridges
// must know this mapping to interpret the signal.
//
// The set is the union of the three channels' windowed pitch classes —
// 12 components stored in channel-0 spare slots (after the lowest-PC
// one-hot). The name "set" carries its meaning, not the channel.

constexpr int STAT_LOWEST_PC_BASE  = 0;   // 12 slots, one-hot of lowest PC
constexpr int STAT_SET_BASE        = 12;  // 12 slots, union of windowed pcs
constexpr int STAT_POLYPHONY       = 0;   // scalar (keyboard, channel 3)

constexpr int KEYBOARD_INTERNAL_CHANNEL = 3;  // Ableton 4

// ═══ CANVAS — The Analysis Cartridge Implementation ══════════════

class Canvas : public AnalysisCartridge {
public:
    Canvas() = default;

    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;

    // ── Lifecycle ────────────────────────────────────────────────

    void initialize(const char* asset_path) override {
        clock_.set_bpm(120.0f);

        // Connect to loopMIDI if available
        if (midi_port_.open_by_name("loopmidi")) {
            std::cout << "Connected to MIDI port: "
                      << midi_port_.port_name() << "\n";
        }

        // Register channels (Ableton 1, 2, 3, 4)
        register_channel("abbott",   1);
        register_channel("costello", 2);
        register_channel("louise",   3);
        register_channel("keyboard", 4);

        // Configure each Train and attach to its channel.
        // territory (windowed pcs) on all three musical channels;
        // reign (lowest one-hot) on abbott and costello only.
        setup_territory(abbott_train_,   abbott_);
        setup_reign(abbott_train_,       abbott_);
        setup_territory(costello_train_, costello_);
        setup_reign(costello_train_,     costello_);
        setup_territory(louise_train_,   louise_);
        setup_keyboard(keyboard_train_);

        attach_train("abbott",   &abbott_train_);
        attach_train("costello", &costello_train_);
        attach_train("louise",   &louise_train_);
        attach_train("keyboard", &keyboard_train_);

        // Try to load default MIDI as fallback
        if (asset_path) {
            std::string midi_path = std::string(asset_path) + "/default.mid";
            load_midi(midi_path.c_str());
        }
    }

    void update(float dt) override {
        clock_.tick(dt);
        float beat = clock_.t_beats();

        // Drain sources into channels
        route_midi_port_events(beat);
        route_midi_file_events(beat);
        route_keyboard_events();

        // For each active channel, snapshot the stream and run
        // every attached Train against it.
        for (int i = 0; i < channel_count_; ++i) {
            auto& ch = channels_[i];
            if (!ch.active) continue;

            ch.stream.update(beat);
            StreamSnapshot snap = ch.stream.snapshot();
            const CompletedRing& hist = ch.stream.history();

            for (int t = 0; t < ch.train_count; ++t) {
                ch.trains[t]->update(snap, hist, beat);
            }
        }

        finalize_output();
        prev_beat_ = beat;
    }

    // ── Input ────────────────────────────────────────────────────

    void on_input(const InputEvent& event) override {
        // We only care about key events for musical input
        if (event.type == InputEvent::Type::KeyDown) {
            on_music_key_down(event.character);
        } else if (event.type == InputEvent::Type::KeyUp) {
            on_music_key_up(event.character);
        }
    }

    // ── Output ───────────────────────────────────────────────────

    const AnalysisSignal& output() const override {
        return output_;
    }

    // ── Configuration ────────────────────────────────────────────

    /**
     * Load a MIDI file as a source.
     */
    bool load_midi(const char* path) {
        if (!midi_file_.load(path)) {
            midi_loaded_ = false;
            return false;
        }
        midi_file_.set_loop(true);
        midi_loaded_ = true;
        prev_beat_ = 0.0f;
        return true;
    }

    /**
     * Set the tempo (BPM).
     */
    void set_bpm(float bpm) {
        clock_.set_bpm(bpm);
    }

    void set_keyboard_enabled(bool on) { keyboard_enabled_ = on; }
    bool keyboard_enabled() const { return keyboard_enabled_; }

    /**
     * Get the clock (for external inspection).
     */
    const Clock& clock() const { return clock_; }

    // Single source of truth for this cartridge's slot map and shapes.
    // Consumed by the_lab; supersedes the prose output-format comment.
    //
    // Only the set is published as the shared field; the two lowest-PC
    // one-hots are published per channel (their election is deferred to
    // the bridge). The per-channel windowed reads stay internal.
    static constexpr StatGroup STAT_LAYOUT[] = {
        { "abbott.lowest_pc",   0, STAT_LOWEST_PC_BASE, PC_COUNT, StatShape::Vector },
        { "costello.lowest_pc", 1, STAT_LOWEST_PC_BASE, PC_COUNT, StatShape::Vector },
        { "set",                0, STAT_SET_BASE,       PC_COUNT, StatShape::Vector },
        { "keyboard.polyphony", 3, STAT_POLYPHONY,      1,        StatShape::Scalar },
    };
    static constexpr int STAT_LAYOUT_COUNT =
        sizeof(STAT_LAYOUT) / sizeof(STAT_LAYOUT[0]);

    // Publish the layout as a non-owning view (the "key" the render side
    // receives once to resolve coupling sources by name). STAT_LAYOUT is
    // static constexpr, so the view stays valid for the program lifetime.
    StatLayoutView stat_layout() const override {
        return StatLayoutView{ STAT_LAYOUT,
                               static_cast<uint32_t>(STAT_LAYOUT_COUNT) };
    }

private:
    // ── Channel Instance ─────────────────────────────────────────
    //
    // One per registered name. Owns its MidiStream; holds non-owning
    // pointers to attached Trains (the Trains themselves are owned by
    // the Canvas as direct members).

    struct ChannelInstance {
        std::string name;
        int channel = -1;   // 0-15 internal (Ableton 1-16 minus 1)
        MidiStream stream;
        std::array<Train*, MAX_TRAINS_PER_CHANNEL> trains{};
        int train_count = 0;
        bool active = false;

        void attach(Train* t) {
            if (train_count < MAX_TRAINS_PER_CHANNEL) {
                trains[train_count++] = t;
            }
        }
    };

    // ── Reading Stats ────────────────────────────────────────────
    //
    // The stat-ids for one musical channel's reading: the windowed
    // territory (per-pitch-class length, intermediate — feeds the set)
    // and the reign (lowest-PC one-hot, published per channel). A
    // territory-only channel (louise) leaves the reign ids unset.

    struct ReadingStats {
        std::array<TrainStatId, PC_COUNT> territory{};  // wagon_pc_length per pc
        std::array<TrainStatId, PC_COUNT> reign{};      // lowest one-hot per pc
        bool has_reign = false;
    };

    // ── Registration API ─────────────────────────────────────────

    /**
     * Register a named channel. Ableton channels are 1-indexed
     * (matching the Ableton UI). Returns nullptr on failure
     * (capacity reached, or duplicate name).
     */
    ChannelInstance* register_channel(const std::string& name,
                                      int ableton_channel) {
        if (channel_count_ >= MAX_NAMED_CHANNELS) return nullptr;
        if (find_channel(name)) return nullptr;  // duplicate

        auto& ch = channels_[channel_count_];
        ch.name = name;
        ch.channel = ableton_channel - 1;  // Ableton 1-16 -> internal 0-15
        ch.train_count = 0;
        ch.active = true;
        ++channel_count_;
        return &ch;
    }

    /**
     * Attach a Train to a named channel. The Canvas does not own
     * the Train — it must outlive the Canvas (typically a member
     * of the same enclosing scope).
     */
    bool attach_train(const std::string& channel_name, Train* train) {
        auto* ch = find_channel(channel_name);
        if (!ch || !train) return false;
        ch->attach(train);
        return true;
    }

    ChannelInstance* find_channel(const std::string& name) {
        for (int i = 0; i < channel_count_; ++i) {
            if (channels_[i].active && channels_[i].name == name) {
                return &channels_[i];
            }
        }
        return nullptr;
    }

    // ── Time ─────────────────────────────────────────────────────
    Clock clock_;
    float prev_beat_ = 0.0f;

    // ── Sources ──────────────────────────────────────────────────
    MidiFile midi_file_;
    bool midi_loaded_ = false;
    KeyboardMidi keyboard_{ KEYBOARD_INTERNAL_CHANNEL, 100 };  // Ableton 4, max 100 events
    MidiPort midi_port_;

    // DEBUG AID — computer keyboard as MIDI into the keyboard channel
    // (internal 3). Toggle at runtime via set_keyboard_enabled(); delete
    // this flag, keyboard_, the keyboard channel/Train, route_keyboard_
    // events(), and on_music_key_* together when the prototype ships.
    bool keyboard_enabled_ = true;

    // ── Channels + Trains ────────────────────────────────────────
    std::array<ChannelInstance, MAX_NAMED_CHANNELS> channels_;
    int channel_count_ = 0;

    // ═══ CURRENT — the reading ═══════════════════════════════════
    //
    // territory (windowed pcs) on all three musical channels; reign
    // (lowest one-hot) on abbott and costello. The set is the union of
    // the three territories, assembled in finalize_output.

    Train abbott_train_;
    Train costello_train_;
    Train louise_train_;
    ReadingStats abbott_;
    ReadingStats costello_;
    ReadingStats louise_;

    // ── Output ───────────────────────────────────────────────────
    AnalysisSignal output_;

    // ── Train Setup (current reading) ────────────────────────────

    // Territory: the windowed pitch classes (length-weighted per pc,
    // D-relative). Intermediate — its presence feeds the set; the
    // per-class lengths are not published.
    void setup_territory(Train& train, ReadingStats& stats) {
        int wg = train.add_wagon(
            TERRITORY_WAGON_SPAN_BEATS,
            0.0f,
            false,
            false,
            TERRITORY_WAGON_PERIOD_BEATS);

        for (int pc = 0; pc < PC_COUNT; ++pc) {
            stats.territory[pc] = train.define(
                [wg, pc](const TrainContext& ctx) -> float {
                    return wagon_pc_length(ctx.wagon(wg), pc, PROJECT_PC_ORIGIN);
                });
        }
    }

    // Reign: the lowest sounding note as a one-hot choice out of twelve
    // (D-relative). Published per channel.
    void setup_reign(Train& train, ReadingStats& stats) {
        int ph = train.add_playhead();
        for (int pc = 0; pc < PC_COUNT; ++pc) {
            stats.reign[pc] = train.define(
                [ph, pc](const TrainContext& ctx) -> float {
                    return playhead_lowest_pc_one_hot_current(ctx.playhead(ph), pc, PROJECT_PC_ORIGIN);
                });
        }
        stats.has_reign = true;
    }

    // ═══ LEGACY / DEBUG — polyphony ══════════════════════════════

    TrainStatId keyboard_polyphony_stat_;
    Train keyboard_train_;

    void setup_keyboard(Train& train) {
        int ph = train.add_playhead();
        keyboard_polyphony_stat_ = train.define(
            [ph](const TrainContext& ctx) {
                return float(playhead_polyphony(ctx.playhead(ph)));
            });
    }

    // ── Event Routing ────────────────────────────────────────────

    void route_midi_port_events(float beat) {
        if (!midi_port_.is_open()) return;

        MidiEvent events[64];
        int count = midi_port_.poll(beat, events, 64);

        for (int i = 0; i < count; ++i) {
            dispatch_event(events[i]);
        }
    }

    void route_midi_file_events(float beat) {
        if (!midi_loaded_) return;

        MidiEvent events[128];
        int count = midi_file_.poll(prev_beat_, beat, events, 128);

        for (int i = 0; i < count; ++i) {
            dispatch_event(events[i]);
        }
    }

    void route_keyboard_events() {
        if (!keyboard_enabled_) return;

        MidiEvent events[64];
        int count = keyboard_.poll(events, 64);

        for (int i = 0; i < count; ++i) {
            dispatch_event(events[i]);
        }
    }

    /**
     * Find the named channel matching the event's channel and
     * deliver the event to its stream. Events with no matching
     * registered channel are silently dropped.
     */
    void dispatch_event(const MidiEvent& ev) {
        for (int i = 0; i < channel_count_; ++i) {
            auto& ch = channels_[i];
            if (ch.active && ch.channel == ev.channel) {
                ch.stream.receive(ev);
                return;
            }
        }
    }

    // ── Keyboard Input ───────────────────────────────────────────

    void on_music_key_down(char key) {
        if (key >= 'A' && key <= 'Z') {
            keyboard_.on_key_press(key, clock_.t_beats());
        } else if (key == ';' || key == '[' || key == ']') {
            keyboard_.on_key_press(key, clock_.t_beats());
        }
    }

    void on_music_key_up(char key) {
        if (key >= 'A' && key <= 'Z') {
            keyboard_.on_key_release(key, clock_.t_beats());
        } else if (key == ';' || key == '[' || key == ']') {
            keyboard_.on_key_release(key, clock_.t_beats());
        }
    }

    // ── Output Finalization ──────────────────────────────────────
    //
    // Each cycle: write abbott/costello lowest one-hots; assemble the
    // set as the union of the three windowed territories (active basis
    // vectors — 1.0 if a pitch class sounds in ANY window, else 0.0);
    // write keyboard polyphony. The set's lossy compression is the
    // closing step, after every channel has been read.

    void finalize_output() {
        output_.t_seconds = clock_.t_seconds();
        output_.t_beats   = clock_.t_beats();
        output_.dt        = clock_.dt();

        // Reign: lowest-PC one-hots, published per channel (D-relative).
        for (int pc = 0; pc < PC_COUNT; ++pc) {
            output_.set_stat(0, STAT_LOWEST_PC_BASE + pc,
                             abbott_train_.get(abbott_.reign[pc]));
            output_.set_stat(1, STAT_LOWEST_PC_BASE + pc,
                             costello_train_.get(costello_.reign[pc]));
        }

        // The set: union of the three windowed territories. A pitch class
        // is in play if its windowed length is nonzero in ANY channel.
        for (int pc = 0; pc < PC_COUNT; ++pc) {
            bool present =
                abbott_train_.get(abbott_.territory[pc])     > 0.0f ||
                costello_train_.get(costello_.territory[pc]) > 0.0f ||
                louise_train_.get(louise_.territory[pc])     > 0.0f;
            output_.set_stat(0, STAT_SET_BASE + pc, present ? 1.0f : 0.0f);
        }

        // Legacy debug: keyboard polyphony.
        output_.set_stat(KEYBOARD_INTERNAL_CHANNEL, STAT_POLYPHONY,
                         keyboard_train_.get(keyboard_polyphony_stat_));
    }
};

} // namespace polyphony_basic
} // namespace t7
