#pragma once

/**
 * MIDI EVENT - Plain Value Type for Event Routing
 * ================================================
 * 
 * A MidiEvent is a simple value representing a note on or off.
 * Events are produced by sources (MidiFile, KeyboardMidi) and
 * consumed by streams (MidiStream).
 * 
 * This decouples the concept of a note from the mechanism of delivery.
 * Events can be buffered, sorted, logged, or routed freely.
 * 
 * SIZE: 12 bytes (fits in a register on 64-bit systems)
 */

#include <cstdint>

namespace t7 {

struct MidiEvent {
    enum Type : uint8_t { 
        NOTE_ON, 
        NOTE_OFF 
    };
    
    Type type;
    uint8_t channel;
    uint8_t pitch;
    uint8_t _pad;
    float velocity;  // 0-1 for NOTE_ON, ignored for NOTE_OFF
    float beat;      // When this event occurred
    
    // =========================================================================
    // FACTORY METHODS
    // =========================================================================
    
    static MidiEvent note_on(int channel, int pitch, float velocity, float beat) {
        MidiEvent e;
        e.type = NOTE_ON;
        e.channel = static_cast<uint8_t>(channel);
        e.pitch = static_cast<uint8_t>(pitch);
        e._pad = 0;
        e.velocity = velocity;
        e.beat = beat;
        return e;
    }
    
    static MidiEvent note_off(int channel, int pitch, float beat) {
        MidiEvent e;
        e.type = NOTE_OFF;
        e.channel = static_cast<uint8_t>(channel);
        e.pitch = static_cast<uint8_t>(pitch);
        e._pad = 0;
        e.velocity = 0.0f;
        e.beat = beat;
        return e;
    }
};

static_assert(sizeof(MidiEvent) == 12, "MidiEvent should be 12 bytes");

} // namespace t7
