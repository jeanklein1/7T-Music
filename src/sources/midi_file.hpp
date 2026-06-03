#pragma once

// ─── midi_file.hpp ───────────────────────────────────────────────
//
// Event producer: reads MIDI files and produces events via poll(). No
// router coupling — composition routes events to streams.
//
// Inert construction: the file can be default-constructed; loading and
// playback are separate explicit operations, which enables fixed storage
// and clear lifecycle management.
//
// Poll model: instead of pushing events to a router, the file produces
// events via poll() — the caller provides a time window and receives the
// events that fall within it. Loop handling is internal: if the file
// loops, poll() handles the wrap automatically, producing events from
// both sides of the boundary.
//
// Usage:
//   MidiFile midi;
//   midi.load("song.mid");
//   midi.set_channel(2);  // output channel for events
//   midi.set_loop(true);
//   MidiEvent events[64];
//   int count = midi.poll(prev_beat, current_beat, events, 64);
//   for (int i = 0; i < count; ++i)
//       streams[events[i].channel].receive(events[i]);
//
// Depends on: sources/midi_event.hpp, <string>, <vector>.

#include "sources/midi_event.hpp"
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>

namespace t7 {

// ═══ CONSTANTS ═══════════════════════════════════════════════════

constexpr int MIDI_FILE_CHANNEL_COUNT = 16;
constexpr int MIDI_FILE_NOTE_COUNT = 128;

// ═══ MIDI FILE READER (Internal) ═════════════════════════════════

namespace midi_internal {

inline uint32_t read_vlq(std::ifstream& file) {
    uint32_t value = 0;
    uint8_t byte;
    do {
        file.read(reinterpret_cast<char*>(&byte), 1);
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    return value;
}

inline uint16_t read_be16(std::ifstream& file) {
    uint8_t buf[2];
    file.read(reinterpret_cast<char*>(buf), 2);
    return static_cast<uint16_t>((buf[0] << 8) | buf[1]);
}

inline uint32_t read_be32(std::ifstream& file) {
    uint8_t buf[4];
    file.read(reinterpret_cast<char*>(buf), 4);
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

struct RawEvent {
    float beat;
    int type;
    int note;
    int velocity;
};

inline bool parse_file(const std::string& filepath,
                       std::vector<RawEvent>& out_events,
                       std::string& out_name,
                       int& out_note_count) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "MidiFile: Cannot open: " << filepath << "\n";
        return false;
    }

    char header[4];
    file.read(header, 4);
    if (std::strncmp(header, "MThd", 4) != 0) {
        std::cerr << "MidiFile: Invalid MIDI (no MThd): " << filepath << "\n";
        return false;
    }

    uint32_t header_length = read_be32(file);
    uint16_t format = read_be16(file);
    uint16_t num_tracks = read_be16(file);
    uint16_t ticks_per_beat = read_be16(file);

    (void)header_length;
    (void)format;

    if (ticks_per_beat == 0) ticks_per_beat = 480;

    out_name = "";
    out_note_count = 0;

    for (int track_idx = 0; track_idx < num_tracks; ++track_idx) {
        char track_header[4];
        file.read(track_header, 4);
        if (std::strncmp(track_header, "MTrk", 4) != 0) break;

        uint32_t track_length = read_be32(file);
        std::streampos track_end = file.tellg() + std::streampos(track_length);

        uint32_t running_ticks = 0;
        uint8_t running_status = 0;

        while (file.tellg() < track_end && file.good()) {
            uint32_t delta = read_vlq(file);
            running_ticks += delta;

            uint8_t status;
            file.read(reinterpret_cast<char*>(&status), 1);

            if (status < 0x80) {
                file.seekg(-1, std::ios::cur);
                status = running_status;
            } else {
                running_status = status;
            }

            float beat = static_cast<float>(running_ticks) / ticks_per_beat;

            if ((status & 0xF0) == 0x90) {
                uint8_t note, vel;
                file.read(reinterpret_cast<char*>(&note), 1);
                file.read(reinterpret_cast<char*>(&vel), 1);

                RawEvent evt;
                evt.beat = beat;
                evt.type = (vel > 0) ? 0x90 : 0x80;
                evt.note = note;
                evt.velocity = vel;
                out_events.push_back(evt);

                if (vel > 0) out_note_count++;

            } else if ((status & 0xF0) == 0x80) {
                uint8_t note, vel;
                file.read(reinterpret_cast<char*>(&note), 1);
                file.read(reinterpret_cast<char*>(&vel), 1);

                RawEvent evt;
                evt.beat = beat;
                evt.type = 0x80;
                evt.note = note;
                evt.velocity = 0;
                out_events.push_back(evt);

            } else if ((status & 0xF0) == 0xA0) {
                file.seekg(2, std::ios::cur);
            } else if ((status & 0xF0) == 0xB0) {
                file.seekg(2, std::ios::cur);
            } else if ((status & 0xF0) == 0xC0) {
                file.seekg(1, std::ios::cur);
            } else if ((status & 0xF0) == 0xD0) {
                file.seekg(1, std::ios::cur);
            } else if ((status & 0xF0) == 0xE0) {
                file.seekg(2, std::ios::cur);
            } else if (status == 0xFF) {
                uint8_t meta_type;
                file.read(reinterpret_cast<char*>(&meta_type), 1);
                uint32_t meta_len = read_vlq(file);

                if (meta_type == 0x03 && meta_len > 0) {
                    std::vector<char> name_buf(meta_len + 1, 0);
                    file.read(name_buf.data(), meta_len);
                    if (out_name.empty()) {
                        out_name = name_buf.data();
                    }
                } else {
                    file.seekg(meta_len, std::ios::cur);
                }
            } else if (status == 0xF0 || status == 0xF7) {
                uint32_t len = read_vlq(file);
                file.seekg(len, std::ios::cur);
            }
        }

        file.seekg(track_end);
    }

    return true;
}

} // namespace midi_internal

// ═══ TRACK INFO ══════════════════════════════════════════════════

struct MidiTrackInfo {
    char name[64] = {0};
    char filepath[256] = {0};
    int channel = -1;           // -1 = not loaded
    int note_count = 0;

    bool loaded() const { return channel >= 0; }
};

// ═══ MIDI FILE ═══════════════════════════════════════════════════

class MidiFile {
public:
    /**
     * Default constructor - inert, no external coupling.
     */
    MidiFile() {
        clear_open_notes();
    }

    // ── Loading ──────────────────────────────────────────────────

    /**
     * Load a MIDI file.
     * 
     * @param filepath Path to .mid file
     * @return true if loaded successfully
     */
    bool load(const std::string& filepath) {
        std::vector<midi_internal::RawEvent> raw_events;
        std::string track_name;
        int note_count = 0;

        if (!midi_internal::parse_file(filepath, raw_events, track_name, note_count)) {
            return false;
        }

        // Clear existing data
        messages_.clear();
        
        // Convert raw events to messages
        for (const auto& raw : raw_events) {
            Message msg{};  // Zero-initialize all fields including _pad
            msg.beat = raw.beat;
            msg.note = static_cast<uint8_t>(raw.note);
            msg.velocity = static_cast<uint8_t>((raw.type == 0x90) ? raw.velocity : 0);
            messages_.push_back(msg);
        }

        // Sort by beat time
        std::sort(messages_.begin(), messages_.end(),
            [](const Message& a, const Message& b) { return a.beat < b.beat; });

        // Update duration
        duration_beats_ = 0.0f;
        if (!messages_.empty()) {
            duration_beats_ = messages_.back().beat + 4.0f;
        }

        // Store track info
        track_info_.note_count = note_count;
        std::strncpy(track_info_.name, 
                     track_name.empty() ? "Untitled" : track_name.c_str(),
                     sizeof(track_info_.name) - 1);
        std::strncpy(track_info_.filepath, filepath.c_str(), 
                     sizeof(track_info_.filepath) - 1);
        track_info_.channel = channel_;

        loaded_ = true;

        std::cout << "MidiFile: Loaded '" << filepath << "' (" 
                  << note_count << " notes, " << duration_beats_ << " beats)\n";

        return true;
    }

    /**
     * Clear all loaded data.
     */
    void clear() {
        messages_.clear();
        track_info_ = MidiTrackInfo{};
        clear_open_notes();
        reset_playback();
        duration_beats_ = 0.0f;
        loaded_ = false;
    }

    // ── Configuration ────────────────────────────────────────────

    void set_channel(int channel) { 
        channel_ = channel; 
        track_info_.channel = channel;
    }
    int channel() const { return channel_; }

    void set_loop(bool loop) { loop_ = loop; }
    bool loop() const { return loop_; }

    // ── POLL - Produce events for a time window ──────────────────

    /**
     * Poll for events in the time window (prev_beat, current_beat].
     * 
     * Handles loop wrapping internally - if the window crosses the loop
     * point, events from both sides of the boundary are produced.
     * 
     * @param prev_beat    Start of window (exclusive)
     * @param current_beat End of window (inclusive)
     * @param out          Output buffer for events
     * @param max_out      Maximum events to write
     * @return Number of events written
     */
    int poll(float prev_beat, float current_beat, MidiEvent* out, int max_out) {
        if (!loaded_ || messages_.empty() || max_out <= 0) {
            return 0;
        }

        int written = 0;

        if (loop_ && duration_beats_ > 0.0f) {
            // Compute effective beats within loop
            float eff_prev = std::fmod(prev_beat, duration_beats_);
            float eff_curr = std::fmod(current_beat, duration_beats_);
            
            if (eff_prev < 0.0f) eff_prev += duration_beats_;
            if (eff_curr < 0.0f) eff_curr += duration_beats_;

            // Detect wrap (current < prev means we crossed the loop point)
            if (eff_curr < eff_prev) {
                // First: flush open notes at loop boundary
                written += flush_open_notes_to_buffer(duration_beats_, out + written, max_out - written);
                
                // Poll from prev to end
                written += poll_range(eff_prev, duration_beats_, out + written, max_out - written);
                
                // Reset playback state for new loop iteration
                reset_playback();
                
                // Poll from start to current
                written += poll_range(0.0f, eff_curr, out + written, max_out - written);
            } else {
                // Normal case - no wrap
                written = poll_range(eff_prev, eff_curr, out, max_out);
            }
        } else {
            // No loop - simple range poll
            written = poll_range(prev_beat, current_beat, out, max_out);
        }

        return written;
    }

    // ── State ────────────────────────────────────────────────────

    bool is_loaded() const { return loaded_; }
    float duration_beats() const { return duration_beats_; }
    const MidiTrackInfo& track_info() const { return track_info_; }

    /**
     * Reset playback to beginning.
     */
    void reset() {
        reset_playback();
    }

private:
    // ── Message Format ───────────────────────────────────────────

    struct Message {
        float beat;
        uint8_t note;
        uint8_t velocity;   // 0 = note off
        uint8_t _pad[2];
    };

    static_assert(sizeof(Message) == 8, "Message should be 8 bytes");

    // ── State ────────────────────────────────────────────────────

    std::vector<Message> messages_;     // Loaded once
    MidiTrackInfo track_info_;
    
    // Open notes tracking: open_notes_[note] = onset beat, or -1 if not open
    std::array<float, MIDI_FILE_NOTE_COUNT> open_notes_;

    size_t msg_pointer_ = 0;
    float duration_beats_ = 0.0f;
    int channel_ = 0;
    bool loop_ = true;
    bool loaded_ = false;

    // ── Internal ─────────────────────────────────────────────────

    void clear_open_notes() {
        open_notes_.fill(-1.0f);
    }

    void reset_playback() {
        msg_pointer_ = 0;
        clear_open_notes();
    }

    /**
     * Poll events in range (start, end] and write to output buffer.
     * Updates open_notes_ tracking.
     */
    int poll_range(float start, float end, MidiEvent* out, int max_out) {
        if (max_out <= 0) return 0;
        
        int written = 0;

        // Find starting position if needed
        if (msg_pointer_ < messages_.size() && messages_[msg_pointer_].beat <= start) {
            // Fast forward to start
            while (msg_pointer_ < messages_.size() && messages_[msg_pointer_].beat <= start) {
                const auto& msg = messages_[msg_pointer_];
                // Track open notes even when skipping
                if (msg.velocity > 0) {
                    open_notes_[msg.note] = msg.beat;
                } else {
                    open_notes_[msg.note] = -1.0f;
                }
                ++msg_pointer_;
            }
        }

        // Process messages in range
        while (msg_pointer_ < messages_.size() && written < max_out) {
            const auto& msg = messages_[msg_pointer_];

            if (msg.beat > end) break;

            if (msg.velocity > 0) {
                out[written++] = MidiEvent::note_on(
                    channel_, msg.note, msg.velocity / 127.0f, msg.beat);
                open_notes_[msg.note] = msg.beat;
            } else {
                out[written++] = MidiEvent::note_off(channel_, msg.note, msg.beat);
                open_notes_[msg.note] = -1.0f;
            }

            ++msg_pointer_;
        }

        return written;
    }

    /**
     * Generate note_off events for all currently open notes.
     * Used at loop boundary to prevent stuck notes.
     */
    int flush_open_notes_to_buffer(float beat, MidiEvent* out, int max_out) {
        int written = 0;
        
        for (int note = 0; note < MIDI_FILE_NOTE_COUNT && written < max_out; ++note) {
            if (open_notes_[note] >= 0.0f) {
                out[written++] = MidiEvent::note_off(channel_, note, beat);
                open_notes_[note] = -1.0f;
            }
        }
        
        return written;
    }
};

} // namespace t7
