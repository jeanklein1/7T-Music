#pragma once

// ─── midi_port.hpp  (dev: transport-aware) ───────────────────────
//
// External MIDI input via a system port (loopMIDI / DAW), now also reading
// the DAW's transport. Same note path as the original — incoming note
// events go into a lock-free ring drained by poll() — with one addition:
// the callback also feeds MIDI clock and transport messages to a
// MidiTransport, so the port surfaces the DAW's musical position, play
// state, and tempo.
//
// Change from the original: ignoreTypes no longer drops timing (clock now
// flows), and handle_message offers each message to the transport first;
// only non-transport messages fall through to the note decode. Note
// consumers are unaffected — they still just poll() events.
//
// In Ableton: enable this port's Clock/Sync output. The port then counts
// the 24-per-quarter pulses into beats() — phase-locked, and frozen when
// the DAW is stopped.
//
// Depends on: sources/midi_event.hpp, sources/transport.hpp,
//             external/RtMidi.h, and the standard headers below.

#include "sources/midi_event.hpp"
#include "sources/transport.hpp"
#include "external/RtMidi.h"

#include <array>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace t7 {

// ═══ MIDI PORT ═══════════════════════════════════════════════════

class MidiPort {
public:
    MidiPort() {
        try {
            midi_in_ = std::make_unique<RtMidiIn>();
        } catch (RtMidiError&) {
            midi_in_.reset();
        }
    }

    ~MidiPort() { close(); }

    MidiPort(const MidiPort&) = delete;
    MidiPort& operator=(const MidiPort&) = delete;

    // ── Connection ───────────────────────────────────────────────

    std::vector<std::string> enumerate() const {
        std::vector<std::string> result;
        if (!midi_in_) return result;
        try {
            unsigned int n = midi_in_->getPortCount();
            for (unsigned int i = 0; i < n; ++i)
                result.push_back(midi_in_->getPortName(i));
        } catch (RtMidiError&) {}
        return result;
    }

    bool open(unsigned int port_index) {
        if (!midi_in_) return false;
        if (open_) close();
        try {
            unsigned int n = midi_in_->getPortCount();
            if (port_index >= n) return false;

            midi_in_->openPort(port_index);
            midi_in_->setCallback(&MidiPort::on_rtmidi_callback, this);
            // Keep timing clock (middle = false); drop sysex and active sense.
            midi_in_->ignoreTypes(true, false, true);

            port_name_ = midi_in_->getPortName(port_index);
            open_ = true;
            return true;
        } catch (RtMidiError&) {
            return false;
        }
    }

    bool open_by_name(const std::string& name_substring) {
        if (!midi_in_) return false;
        std::vector<std::string> ports = enumerate();
        for (size_t i = 0; i < ports.size(); ++i)
            if (icontains(ports[i], name_substring))
                return open(static_cast<unsigned int>(i));
        return false;
    }

    void close() {
        if (!midi_in_ || !open_) return;
        try {
            midi_in_->cancelCallback();
            midi_in_->closePort();
        } catch (RtMidiError&) {}
        open_ = false;
        port_name_.clear();
        transport_.reset();
    }

    bool is_open() const { return open_; }
    const std::string& port_name() const { return port_name_; }

    // ── DAW transport (read side) ────────────────────────────────

    bool     playing()     const { return transport_.playing(); }
    double   beats()       const { return transport_.beats(); }
    float    bpm()         const { return transport_.bpm(); }
    bool     ever_synced() const { return transport_.ever_synced(); }
    uint32_t pulses()      const { return transport_.pulses(); }

    // ── POLL — drain note events, stamp with current_beat ─────────

    int poll(float current_beat, MidiEvent* out, int max_out) {
        int count = 0;
        const uint32_t write = write_idx_.load(std::memory_order_acquire);
        uint32_t read = read_idx_.load(std::memory_order_relaxed);
        while (read != write && count < max_out) {
            out[count] = ring_[read & RING_MASK];
            out[count].beat = current_beat;
            ++read;
            ++count;
        }
        read_idx_.store(read, std::memory_order_release);
        return count;
    }

    int pending_count() const {
        const uint32_t write = write_idx_.load(std::memory_order_acquire);
        const uint32_t read = read_idx_.load(std::memory_order_acquire);
        return static_cast<int>(write - read);
    }

private:
    static constexpr uint32_t RING_SIZE = 256;
    static constexpr uint32_t RING_MASK = RING_SIZE - 1;
    static_assert((RING_SIZE & RING_MASK) == 0, "RING_SIZE must be a power of two");

    std::unique_ptr<RtMidiIn> midi_in_;
    bool open_ = false;
    std::string port_name_;

    MidiTransport transport_;

    std::array<MidiEvent, RING_SIZE> ring_{};
    std::atomic<uint32_t> write_idx_{0};
    std::atomic<uint32_t> read_idx_{0};

    // ── CALLBACK (runs on RtMidi's thread) ───────────────────────

    static void on_rtmidi_callback(double deltatime,
                                   std::vector<unsigned char>* msg,
                                   void* user) {
        if (!msg || msg->empty() || !user) return;
        static_cast<MidiPort*>(user)->handle_message(deltatime, *msg);
    }

    void handle_message(double deltatime, const std::vector<unsigned char>& m) {
        // Clock / start / stop / continue / song-position go to the transport.
        if (transport_.feed(deltatime, m)) return;

        // Everything else: the note path, unchanged.
        if (m.size() < 3) return;

        const uint8_t status   = m[0];
        const uint8_t type     = status & 0xF0;
        const uint8_t channel  = status & 0x0F;
        const uint8_t pitch    = m[1];
        const uint8_t velocity = m[2];

        MidiEvent ev;
        if (type == 0x90 && velocity > 0) {
            ev = MidiEvent::note_on(channel, pitch, velocity / 127.0f, 0.0f);
        } else if (type == 0x80 || (type == 0x90 && velocity == 0)) {
            ev = MidiEvent::note_off(channel, pitch, 0.0f);
        } else {
            return;  // CC, pitch bend, aftertouch — ignore
        }
        push(ev);
    }

    void push(const MidiEvent& ev) {
        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
        const uint32_t read  = read_idx_.load(std::memory_order_acquire);
        if ((write - read) >= RING_SIZE) return;   // full — drop
        ring_[write & RING_MASK] = ev;
        write_idx_.store(write + 1, std::memory_order_release);
    }

    // ── Helpers ──────────────────────────────────────────────────

    static bool icontains(const std::string& haystack, const std::string& needle) {
        if (needle.empty()) return true;
        if (haystack.size() < needle.size()) return false;
        auto lower = [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); };
        for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
            bool match = true;
            for (size_t j = 0; j < needle.size(); ++j)
                if (lower(haystack[i+j]) != lower(needle[j])) { match = false; break; }
            if (match) return true;
        }
        return false;
    }
};

} // namespace t7
