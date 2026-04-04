#pragma once

/**
 * KEYBOARD MIDI - Computer Keyboard to MIDI Events
 * =================================================
 * 
 * Converts computer keyboard input to MIDI-style note events.
 * Events are queued internally and retrieved via poll().
 * 
 * INERT CONSTRUCTION
 * ------------------
 * 
 * No router coupling - events are queued and retrieved by the caller.
 * This enables fixed storage and visible control flow.
 * 
 * DATA STRUCTURES
 * ---------------
 * 
 * Fixed-size arrays indexed by ASCII character (256 entries).
 * Zero allocation after initialization.
 * 
 * LAYOUT (US QWERTY)
 * ------------------
 * 
 *     Black: W  E     T  Y  U     O  P
 *           C# D#    F# G# A#    C# D#
 * 
 *     White: A  S  D  F  G  H  J  K  L  ;
 *            C4 D4 E4 F4 G4 A4 B4 C5 D5 E5
 * 
 *     Lower: Z  X  C  V  B  N  M
 *            C3 D3 E3 F3 G3 A3 B3
 * 
 *     Controls: [ ] = Octave shift down/up
 * 
 * USAGE
 * -----
 * 
 *     KeyboardMidi keyboard(2, 100);  // channel 2, velocity 100
 *     
 *     // In key callback
 *     keyboard.on_key_press('A', current_beat);
 *     keyboard.on_key_release('A', current_beat);
 *     
 *     // Each frame, retrieve pending events
 *     MidiEvent events[32];
 *     int count = keyboard.poll(events, 32);
 *     
 *     // Route to streams
 *     for (int i = 0; i < count; ++i) {
 *         streams[events[i].channel].receive(events[i]);
 *     }
 */

#include "sources/midi_event.hpp"
#include <array>
#include <string>
#include <cctype>
#include <algorithm>

namespace t7 {

// =============================================================================
// KEYBOARD MIDI
// =============================================================================

class KeyboardMidi {
public:
    /**
     * Construct keyboard MIDI controller.
     * 
     * @param channel  MIDI channel (0-15)
     * @param velocity Default velocity (1-127)
     */
    KeyboardMidi(int channel = 0, int velocity = 100)
        : channel_(channel)
        , velocity_(std::clamp(velocity, 1, 127))
    {
        key_to_note_.fill(-1);   // -1 = not mapped
        held_note_.fill(-1);     // -1 = not held
        pending_count_ = 0;
        
        init_piano_layout();
    }
    
    // =========================================================================
    // CONFIGURATION
    // =========================================================================
    
    void set_channel(int channel) { channel_ = channel; }
    int channel() const { return channel_; }
    
    void set_velocity(int v) { velocity_ = std::clamp(v, 1, 127); }
    int velocity() const { return velocity_; }
    
    void set_octave_shift(int s) { octave_shift_ = std::clamp(s, -3, 3); }
    int octave_shift() const { return octave_shift_; }
    
    // =========================================================================
    // KEY EVENTS
    // =========================================================================
    
    /**
     * Handle key press. Returns true if key was handled.
     * Queues event internally - retrieve via poll().
     */
    bool on_key_press(char key, float current_beat) {
        key = normalize_key(key);
        uint8_t key_idx = static_cast<uint8_t>(key);
        
        // Octave shift controls
        if (key == '[') {
            octave_shift_ = std::max(-3, octave_shift_ - 1);
            return true;
        }
        if (key == ']') {
            octave_shift_ = std::min(3, octave_shift_ + 1);
            return true;
        }
        
        // Check if key is mapped
        int base_note = key_to_note_[key_idx];
        if (base_note < 0) return false;
        
        // Already held?
        if (held_note_[key_idx] >= 0) return true;
        
        // Compute actual note with octave shift
        int midi_note = std::clamp(base_note + octave_shift_ * 12, 0, 127);
        
        // Store held note and queue event
        held_note_[key_idx] = static_cast<int8_t>(midi_note);
        queue_event(MidiEvent::note_on(channel_, midi_note, velocity_ / 127.0f, current_beat));
        
        return true;
    }
    
    /**
     * Handle key release. Returns true if key was handled.
     * Queues event internally - retrieve via poll().
     */
    bool on_key_release(char key, float current_beat) {
        key = normalize_key(key);
        uint8_t key_idx = static_cast<uint8_t>(key);
        
        // Get held note
        int midi_note = held_note_[key_idx];
        if (midi_note < 0) return false;
        
        // Clear held state and queue note off
        held_note_[key_idx] = -1;
        queue_event(MidiEvent::note_off(channel_, midi_note, current_beat));
        
        return true;
    }
    
    /**
     * Release all currently held notes.
     */
    void release_all(float current_beat) {
        for (int i = 0; i < 256; ++i) {
            if (held_note_[i] >= 0) {
                queue_event(MidiEvent::note_off(channel_, held_note_[i], current_beat));
                held_note_[i] = -1;
            }
        }
    }
    
    // =========================================================================
    // POLL - Retrieve pending events
    // =========================================================================
    
    /**
     * Retrieve pending events and clear the queue.
     * 
     * @param out     Output buffer for events
     * @param max_out Maximum events to write
     * @return Number of events written
     */
    int poll(MidiEvent* out, int max_out) {
        int count = std::min(pending_count_, max_out);
        
        for (int i = 0; i < count; ++i) {
            out[i] = pending_[i];
        }
        
        // If we couldn't return all events, shift remaining to front
        if (count < pending_count_) {
            for (int i = count; i < pending_count_; ++i) {
                pending_[i - count] = pending_[i];
            }
            pending_count_ -= count;
        } else {
            pending_count_ = 0;
        }
        
        return count;
    }
    
    /**
     * Check how many events are pending.
     */
    int pending_count() const { return pending_count_; }
    
    // =========================================================================
    // ACCESSORS
    // =========================================================================
    
    int held_count() const {
        int count = 0;
        for (int i = 0; i < 256; ++i) {
            if (held_note_[i] >= 0) count++;
        }
        return count;
    }
    
private:
    static constexpr int MAX_PENDING = 64;
    
    int channel_;
    int velocity_;
    int octave_shift_ = 0;
    
    // Fixed-size lookup tables (indexed by ASCII value)
    std::array<int8_t, 256> key_to_note_;   // ASCII -> base MIDI note (-1 = unmapped)
    std::array<int8_t, 256> held_note_;     // ASCII -> currently held note (-1 = not held)
    
    // Pending events queue
    std::array<MidiEvent, MAX_PENDING> pending_;
    int pending_count_ = 0;
    
    void queue_event(const MidiEvent& event) {
        if (pending_count_ < MAX_PENDING) {
            pending_[pending_count_++] = event;
        }
        // If queue is full, drop oldest events (shouldn't happen in normal use)
    }
    
    static char normalize_key(char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    
    void init_piano_layout() {
        // White keys (home row): C4 - E5
        key_to_note_[static_cast<uint8_t>('A')] = 60;  // C4
        key_to_note_[static_cast<uint8_t>('S')] = 62;  // D4
        key_to_note_[static_cast<uint8_t>('D')] = 64;  // E4
        key_to_note_[static_cast<uint8_t>('F')] = 65;  // F4
        key_to_note_[static_cast<uint8_t>('G')] = 67;  // G4
        key_to_note_[static_cast<uint8_t>('H')] = 69;  // A4
        key_to_note_[static_cast<uint8_t>('J')] = 71;  // B4
        key_to_note_[static_cast<uint8_t>('K')] = 72;  // C5
        key_to_note_[static_cast<uint8_t>('L')] = 74;  // D5
        key_to_note_[static_cast<uint8_t>(';')] = 76;  // E5
        
        // Black keys (top row)
        key_to_note_[static_cast<uint8_t>('W')] = 61;  // C#4
        key_to_note_[static_cast<uint8_t>('E')] = 63;  // D#4
        key_to_note_[static_cast<uint8_t>('T')] = 66;  // F#4
        key_to_note_[static_cast<uint8_t>('Y')] = 68;  // G#4
        key_to_note_[static_cast<uint8_t>('U')] = 70;  // A#4
        key_to_note_[static_cast<uint8_t>('O')] = 73;  // C#5
        key_to_note_[static_cast<uint8_t>('P')] = 75;  // D#5
        
        // Lower octave (Z row): C3 - B3
        key_to_note_[static_cast<uint8_t>('Z')] = 48;  // C3
        key_to_note_[static_cast<uint8_t>('X')] = 50;  // D3
        key_to_note_[static_cast<uint8_t>('C')] = 52;  // E3
        key_to_note_[static_cast<uint8_t>('V')] = 53;  // F3
        key_to_note_[static_cast<uint8_t>('B')] = 55;  // G3
        key_to_note_[static_cast<uint8_t>('N')] = 57;  // A3
        key_to_note_[static_cast<uint8_t>('M')] = 59;  // B3
    }
};

// =============================================================================
// UTILITY
// =============================================================================

inline std::string midi_note_name(int midi_note) {
    static const char* names[] = {
        "C", "C#", "D", "D#", "E", "F", 
        "F#", "G", "G#", "A", "A#", "B"
    };
    int octave = (midi_note / 12) - 1;
    int pc = midi_note % 12;
    return std::string(names[pc]) + std::to_string(octave);
}

} // namespace t7
