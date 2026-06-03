#pragma once

// ─── midi_event.hpp ──────────────────────────────────────────────
//
// Plain value type for event routing. A MidiEvent is a simple value
// representing a note on or off. Events are produced by sources
// (MidiFile, KeyboardMidi) and consumed by streams (MidiStream); this
// decouples the concept of a note from the mechanism of delivery, so
// events can be buffered, sorted, logged, or routed freely.
//
// Size: 12 bytes (fits in a register on 64-bit systems).
//
// Depends on: <cstdint>.

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
    
    // ── Factory Methods ──────────────────────────────────────────
    
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
