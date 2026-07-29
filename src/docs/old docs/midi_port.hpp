// **ARCHIVAL** — this document describes the tree as of its own
// date and is superseded. Do not cite as current. Live facts live
// in the code; adjudications live in `audit/` and
// `docs/HANDOFFS/OPTIMIZATION 1/`.

#pragma once

/**
 * MIDI PORT - External MIDI Input via System Port (loopMIDI / DAW)
 * ================================================================
 *
 * Connects to a system MIDI input port (typically a loopMIDI virtual port
 * fed by Ableton or another DAW) and queues incoming note events for the
 * analysis cartridge to drain via poll().
 *
 * INERT CONSTRUCTION
 * ------------------
 *
 * Constructing a MidiPort never fails, even if the MIDI subsystem is
 * unavailable. is_open() reports false until open() succeeds. This lets
 * the analysis cartridge include it unconditionally — the port simply
 * stays silent if no DAW is connected.
 *
 * THREADING
 * ---------
 *
 * RtMidi runs its own input thread and invokes a callback when MIDI
 * arrives. The callback decodes the bytes and pushes a MidiEvent into
 * a single-producer / single-consumer ring buffer. The main thread
 * drains the ring via poll(). Lock-free, no allocations during runtime.
 *
 * BEAT STAMPING
 * -------------
 *
 * The callback runs on RtMidi's thread without access to our Clock,
 * so events are stamped with the caller-provided beat at poll time.
 * Timing error is bounded by frame duration (~16ms at 60fps), well
 * within the Playhead's 50ms chord tolerance.
 *
 * USAGE
 * -----
 *
 *     MidiPort port;
 *     port.open_by_name("loopMIDI");   // case-insensitive substring match
 *
 *     // Each frame:
 *     MidiEvent events[64];
 *     int count = port.poll(current_beat, events, 64);
 *     for (int i = 0; i < count; ++i) {
 *         stream.receive(events[i]);
 *     }
 */

#include "sources/midi_event.hpp"
#include "external/RtMidi.h"

#include <array>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace t7 {

// =============================================================================
// MIDI PORT
// =============================================================================

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

    // =========================================================================
    // CONNECTION
    // =========================================================================

    /**
     * List available input port names.
     * Safe to call before open().
     */
    std::vector<std::string> enumerate() const {
        std::vector<std::string> result;
        if (!midi_in_) return result;
        try {
            unsigned int n = midi_in_->getPortCount();
            for (unsigned int i = 0; i < n; ++i) {
                result.push_back(midi_in_->getPortName(i));
            }
        } catch (RtMidiError&) {}
        return result;
    }

    /**
     * Open a port by index. Returns true on success.
     */
    bool open(unsigned int port_index) {
        if (!midi_in_) return false;
        if (open_) close();

        try {
            unsigned int n = midi_in_->getPortCount();
            if (port_index >= n) return false;

            midi_in_->openPort(port_index);
            midi_in_->setCallback(&MidiPort::on_rtmidi_callback, this);
            // Drop noise: sysex, MIDI clock/timing, active sense
            midi_in_->ignoreTypes(true, true, true);

            port_name_ = midi_in_->getPortName(port_index);
            open_ = true;
            return true;
        } catch (RtMidiError&) {
            return false;
        }
    }

    /**
     * Open the first port whose name contains the given substring
     * (case-insensitive). Returns true on success.
     */
    bool open_by_name(const std::string& name_substring) {
        if (!midi_in_) return false;

        std::vector<std::string> ports = enumerate();
        for (size_t i = 0; i < ports.size(); ++i) {
            if (icontains(ports[i], name_substring)) {
                return open(static_cast<unsigned int>(i));
            }
        }
        return false;
    }

    /**
     * Close the port. Safe to call even if not open.
     */
    void close() {
        if (!midi_in_ || !open_) return;
        try {
            midi_in_->cancelCallback();
            midi_in_->closePort();
        } catch (RtMidiError&) {
            // Closing - swallow
        }
        open_ = false;
        port_name_.clear();
    }

    bool is_open() const { return open_; }
    const std::string& port_name() const { return port_name_; }

    // =========================================================================
    // POLL - Retrieve pending events
    // =========================================================================

    /**
     * Drain pending events. Each event is stamped with current_beat.
     *
     * @param current_beat Beat to stamp events with
     * @param out          Output buffer
     * @param max_out      Maximum events to write
     * @return Number of events written
     */
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

    /**
     * Approximate count of events pending in the ring.
     */
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

    // SPSC ring buffer
    std::array<MidiEvent, RING_SIZE> ring_{};
    std::atomic<uint32_t> write_idx_{0};  // producer: RtMidi callback thread
    std::atomic<uint32_t> read_idx_{0};   // consumer: main thread

    // =========================================================================
    // CALLBACK (runs on RtMidi's thread)
    // =========================================================================

    static void on_rtmidi_callback(double /*deltatime*/,
                                   std::vector<unsigned char>* msg,
                                   void* user) {
        if (!msg || msg->empty() || !user) return;
        static_cast<MidiPort*>(user)->handle_message(*msg);
    }

    void handle_message(const std::vector<unsigned char>& m) {
        if (m.size() < 3) return;

        const uint8_t status   = m[0];
        const uint8_t type     = status & 0xF0;
        const uint8_t channel  = status & 0x0F;
        const uint8_t pitch    = m[1];
        const uint8_t velocity = m[2];

        MidiEvent ev;
        if (type == 0x90 && velocity > 0) {
            // Note On
            ev = MidiEvent::note_on(channel, pitch, velocity / 127.0f, 0.0f);
        } else if (type == 0x80 || (type == 0x90 && velocity == 0)) {
            // Note Off (explicit, or running-status note-on with velocity 0)
            ev = MidiEvent::note_off(channel, pitch, 0.0f);
        } else {
            return;  // Not a note message — ignore CCs, pitch bend, etc.
        }

        push(ev);
    }

    void push(const MidiEvent& ev) {
        const uint32_t write = write_idx_.load(std::memory_order_relaxed);
        const uint32_t read  = read_idx_.load(std::memory_order_acquire);

        if ((write - read) >= RING_SIZE) {
            // Ring full — drop event. At musical event rates this is
            // effectively impossible (256 events per frame would be ~15kHz).
            return;
        }

        ring_[write & RING_MASK] = ev;
        write_idx_.store(write + 1, std::memory_order_release);
    }

    // =========================================================================
    // HELPERS
    // =========================================================================

    static bool icontains(const std::string& haystack, const std::string& needle) {
        if (needle.empty()) return true;
        if (haystack.size() < needle.size()) return false;

        auto lower = [](unsigned char c) -> char {
            return static_cast<char>(std::tolower(c));
        };

        for (size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
            bool match = true;
            for (size_t j = 0; j < needle.size(); ++j) {
                if (lower(haystack[i+j]) != lower(needle[j])) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        return false;
    }
};

} // namespace t7
