#pragma once

/**
 * POLYPHONY BASIC - Analysis Cartridge
 * =====================================
 * 
 * A minimal analysis cartridge that tracks polyphony (note count).
 * 
 * SOURCES
 * -------
 * - MidiFile: Plays a MIDI file in a loop
 * - KeyboardMidi: Computer keyboard as piano
 * - MidiPort: External MIDI input via system port (loopMIDI / DAW)
 * 
 * ANALYSIS
 * --------
 * - Single MidiStream (channel 0)
 * - Single Playhead (point-in-time observation)
 * - Single stat: polyphony (count of active notes)
 * 
 * OUTPUT FORMAT
 * -------------
 * - stats[0] = polyphony (0.0 to ~16.0)
 * 
 * This cartridge serves as:
 * 1. The testbed for infrastructure development
 * 2. A template for creating new analysis cartridges
 */

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

#include <iostream>
#include <string>

namespace t7 {
namespace polyphony_basic {

// =============================================================================
// STAT SLOT DEFINITIONS
// =============================================================================
// 
// This cartridge's output format. Visualization cartridges must know
// this mapping to interpret the signal.

constexpr int STAT_POLYPHONY = 0;

// =============================================================================
// CANVAS - The Analysis Cartridge Implementation
// =============================================================================

class Canvas : public AnalysisCartridge {
public:
    Canvas() = default;
    
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    
    // ─── LIFECYCLE ──────────────────────────────────────────────────────────
    
    void initialize(const char* asset_path) override {
        clock_.set_bpm(120.0f);

        // Define stats
        polyphony_stat_ = train_.define([](const TrainContext& ctx) {
            return static_cast<float>(ctx.playhead(0).current_count);
        });

        // Try to load default MIDI
        if (asset_path) {
            std::string midi_path = std::string(asset_path) + "/SWEET_CLIP_3.mid";
            load_midi(midi_path.c_str());
        }

        // Try to auto-connect to a loopMIDI port (Ableton -> us)
        if (midi_port_.open_by_name("loopmidi")) {
            std::cout << "Connected to MIDI port: "
                      << midi_port_.port_name() << "\n";
        }
    }
    
    void update(float dt) override {
        clock_.tick(dt);
        float beat = clock_.t_beats();
        
        // Route events from sources to stream
        route_midi_file_events(beat);
        route_keyboard_events();
        route_midi_port_events(beat);
        
        // Update stream (prunes old history)
        stream_.update(beat);
        
        // Update analyzers
        StreamSnapshot snap = stream_.snapshot();
        const CompletedRing& history = stream_.history();
        playhead_.update(snap, history);
        
        // Wire playhead to train and compute stats
        train_.set_playhead(0, playhead_.readout());
        train_.update(beat);
        
        // Build output signal
        finalize_output()
            ;
        
        prev_beat_ = beat;
    }
    
    // ─── INPUT ──────────────────────────────────────────────────────────────
    
    void on_input(const InputEvent& event) override {
        // We only care about key events for musical input
        if (event.type == InputEvent::Type::KeyDown) {
            on_music_key_down(event.character);
        } else if (event.type == InputEvent::Type::KeyUp) {
            on_music_key_up(event.character);
        }
    }
    
    // ─── OUTPUT ─────────────────────────────────────────────────────────────
    
    const AnalysisSignal& output() const override { 
        return output_; 
    }
    
    // ─── CONFIGURATION ──────────────────────────────────────────────────────
    
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
    
    /**
     * Get the clock (for external inspection).
     */
    const Clock& clock() const { return clock_; }
    
private:
    // ─── TIME ───────────────────────────────────────────────────────────────
    Clock clock_;
    float prev_beat_ = 0.0f;
    
    // ─── SOURCES ────────────────────────────────────────────────────────────
    MidiFile midi_file_;
    bool midi_loaded_ = false;
    KeyboardMidi keyboard_{ 0, 100 };  // channel 0, max 100 events
    MidiPort midi_port_;
    
    // ─── STREAM + ANALYZERS ─────────────────────────────────────────────────
    MidiStream stream_;
    Playhead playhead_;
    Train train_;
    TrainStatId polyphony_stat_;
    
    // ─── OUTPUT ─────────────────────────────────────────────────────────────
    AnalysisSignal output_;
    
    // ─── EVENT ROUTING ──────────────────────────────────────────────────────
    
    void route_midi_file_events(float beat) {
        if (!midi_loaded_) return;
        
        MidiEvent events[128];
        int count = midi_file_.poll(prev_beat_, beat, events, 128);
        
        for (int i = 0; i < count; ++i) {
            stream_.receive(events[i]);
        }
    }
    
    void route_keyboard_events() {
        MidiEvent events[64];
        int count = keyboard_.poll(events, 64);

        for (int i = 0; i < count; ++i) {
            stream_.receive(events[i]);
        }
    }

    void route_midi_port_events(float beat) {
        if (!midi_port_.is_open()) return;

        MidiEvent events[64];
        int count = midi_port_.poll(beat, events, 64);

        for (int i = 0; i < count; ++i) {
            stream_.receive(events[i]);
        }
    }
    
    // ─── KEYBOARD INPUT ─────────────────────────────────────────────────────
    
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
    
    // ─── OUTPUT FINALIZATION ────────────────────────────────────────────────
    
    void finalize_output() {
        output_.t_seconds = clock_.t_seconds();
        output_.t_beats = clock_.t_beats();
        output_.dt = clock_.dt();
        
        // Write stats to output slots
        output_.set_stat(0, STAT_POLYPHONY, train_.get(polyphony_stat_));
    }
};

} // namespace polyphony_basic
} // namespace t7
