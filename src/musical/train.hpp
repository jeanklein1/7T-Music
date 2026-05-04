#pragma once

/**
 * TRAIN - Musical Analysis Composition Canvas
 * ============================================
 * 
 * The Train is where Playheads and Wagons combine into compound analysis.
 * It's a canvas for writing small programs that read analyzer outputs
 * and produce derived statistics.
 * 
 * ARCHITECTURE
 * ------------
 * 
 *     MidiStream --> snapshot() --+--> Playhead.update() --> PlayheadReadout
 *                                 |
 *                                 +--> Wagon.update() -----> WagonReadout
 *                                                                  |
 *                                                                  v
 *                                                         Train.update(readouts)
 *                                                                  |
 *                                                                  v
 *                                                         Computed Stats
 * 
 * SNAPSHOT-BASED
 * --------------
 * 
 * The Train receives readouts as values rather than holding pointers to
 * analyzers. This enables parallel analysis and clean data flow.
 * 
 * ALLOCATION GUARANTEE
 * --------------------
 * 
 * All operations are allocation-free after initialization. Lambda captures
 * are stored inline (up to COMPUTE_FN_BUFFER_SIZE bytes). Lambdas with
 * larger captures will fail to compile.
 * 
 * OWNED ANALYZERS
 * ---------------
 *
 * The Train owns its Playheads and Wagons. Attach them via add_playhead()
 * / add_wagon() at setup time; the Train runs them against the snapshot
 * each frame inside update(). Stat lambdas read their readouts via
 * ctx.playhead(slot) / ctx.wagon(slot).
 *
 * USAGE
 * -----
 *
 *     MidiStream stream(...);
 *     Train train;
 *
 *     int ph_slot = train.add_playhead();
 *     int wg_slot = train.add_wagon(4.0f);
 *
 *     auto STAT = train.define([=](const TrainContext& ctx) {
 *         return float(ctx.playhead(ph_slot).current_count);
 *     });
 *
 *     // Per frame
 *     stream.update(beat);
 *     train.update(stream.snapshot(), stream.history(), beat);
 *     float v = train.get(STAT);
 */

#include "musical/playhead.hpp"
#include "musical/wagon.hpp"
#include <array>
#include <cstdint>
#include <new>
#include <type_traits>

namespace t7 {

// =============================================================================
// CONSTANTS
// =============================================================================

constexpr int TRAIN_MAX_PLAYHEADS = 2;
constexpr int TRAIN_MAX_WAGONS = 2;
constexpr int TRAIN_MAX_STATS = 32;

// Buffer size for inline lambda storage (no heap allocation if capture fits)
// 32 bytes accommodates most typical lambdas (4 pointers or 8 floats)
constexpr size_t COMPUTE_FN_BUFFER_SIZE = 32;

// =============================================================================
// STAT ID - Handle for train outputs
// =============================================================================

struct TrainStatId {
    uint16_t index = 0xFFFF;
    
    constexpr TrainStatId() = default;
    constexpr explicit TrainStatId(uint16_t i) : index(i) {}
    
    constexpr bool valid() const { return index != 0xFFFF; }
    constexpr operator bool() const { return valid(); }
};

// Forward declaration
class TrainContext;

// =============================================================================
// SMALL FUNCTION - Allocation-free callable wrapper
// =============================================================================

/**
 * A small inline function wrapper that stores lambdas without heap allocation.
 * 
 * Unlike std::function, this guarantees no heap allocation by storing the
 * callable inline. Lambdas with captures larger than COMPUTE_FN_BUFFER_SIZE
 * will fail to compile with a clear error message.
 */
class ComputeFn {
public:
    using InvokeFn = float(*)(const void*, const TrainContext&);
    using DestroyFn = void(*)(void*);
    using CopyFn = void(*)(void*, const void*);
    
    ComputeFn() = default;
    
    template<typename F>
    ComputeFn(F&& fn) {
        static_assert(sizeof(F) <= COMPUTE_FN_BUFFER_SIZE,
            "Lambda capture too large for ComputeFn. "
            "Reduce captures or increase COMPUTE_FN_BUFFER_SIZE.");
        static_assert(std::is_trivially_copyable_v<F> || std::is_copy_constructible_v<F>,
            "Lambda must be copyable");
        
        // Store the callable in the buffer
        new (buffer_) F(std::forward<F>(fn));
        
        // Store type-erased operations
        invoke_ = [](const void* buf, const TrainContext& ctx) -> float {
            return (*static_cast<const F*>(buf))(ctx);
        };
        destroy_ = [](void* buf) {
            static_cast<F*>(buf)->~F();
        };
        copy_ = [](void* dst, const void* src) {
            new (dst) F(*static_cast<const F*>(src));
        };
    }
    
    ComputeFn(const ComputeFn& other) {
        if (other.invoke_) {
            other.copy_(buffer_, other.buffer_);
            invoke_ = other.invoke_;
            destroy_ = other.destroy_;
            copy_ = other.copy_;
        }
    }
    
    ComputeFn& operator=(const ComputeFn& other) {
        if (this != &other) {
            if (invoke_) {
                destroy_(buffer_);
            }
            if (other.invoke_) {
                other.copy_(buffer_, other.buffer_);
                invoke_ = other.invoke_;
                destroy_ = other.destroy_;
                copy_ = other.copy_;
            } else {
                invoke_ = nullptr;
                destroy_ = nullptr;
                copy_ = nullptr;
            }
        }
        return *this;
    }
    
    ComputeFn(ComputeFn&& other) noexcept {
        if (other.invoke_) {
            other.copy_(buffer_, other.buffer_);
            invoke_ = other.invoke_;
            destroy_ = other.destroy_;
            copy_ = other.copy_;
            // Clear other
            other.destroy_(other.buffer_);
            other.invoke_ = nullptr;
        }
    }
    
    ComputeFn& operator=(ComputeFn&& other) noexcept {
        if (this != &other) {
            if (invoke_) {
                destroy_(buffer_);
            }
            if (other.invoke_) {
                other.copy_(buffer_, other.buffer_);
                invoke_ = other.invoke_;
                destroy_ = other.destroy_;
                copy_ = other.copy_;
                other.destroy_(other.buffer_);
                other.invoke_ = nullptr;
            } else {
                invoke_ = nullptr;
            }
        }
        return *this;
    }
    
    ~ComputeFn() {
        if (invoke_) {
            destroy_(buffer_);
        }
    }
    
    float operator()(const TrainContext& ctx) const {
        return invoke_(buffer_, ctx);
    }
    
    explicit operator bool() const { return invoke_ != nullptr; }
    
private:
    alignas(8) char buffer_[COMPUTE_FN_BUFFER_SIZE]{};
    InvokeFn invoke_ = nullptr;
    DestroyFn destroy_ = nullptr;
    CopyFn copy_ = nullptr;
};

// =============================================================================
// TRAIN CONTEXT - Read-only view passed to user programs
// =============================================================================

class TrainContext {
public:
    /**
     * Access playhead readout by slot index.
     * Returns empty readout if slot is not set.
     */
    const PlayheadReadout& playhead(int slot) const {
        if (slot < 0 || slot >= TRAIN_MAX_PLAYHEADS || !playhead_valid_[slot]) {
            return empty_playhead_readout_;
        }
        return playheads_[slot];
    }
    
    /**
     * Access wagon readout by slot index.
     * Returns empty readout if slot is not set.
     */
    const WagonReadout& wagon(int slot) const {
        if (slot < 0 || slot >= TRAIN_MAX_WAGONS || !wagon_valid_[slot]) {
            return empty_wagon_readout_;
        }
        return wagons_[slot];
    }
    
    /**
     * Check if a playhead slot has valid data.
     */
    bool has_playhead(int slot) const {
        return slot >= 0 && slot < TRAIN_MAX_PLAYHEADS && playhead_valid_[slot];
    }
    
    /**
     * Check if a wagon slot has valid data.
     */
    bool has_wagon(int slot) const {
        return slot >= 0 && slot < TRAIN_MAX_WAGONS && wagon_valid_[slot];
    }
    
    /**
     * Current beat.
     */
    float beat() const { return beat_; }
    
private:
    friend class Train;
    
    std::array<PlayheadReadout, TRAIN_MAX_PLAYHEADS> playheads_{};
    std::array<WagonReadout, TRAIN_MAX_WAGONS> wagons_{};
    std::array<bool, TRAIN_MAX_PLAYHEADS> playhead_valid_{};
    std::array<bool, TRAIN_MAX_WAGONS> wagon_valid_{};
    float beat_ = 0.0f;
    
    static inline const PlayheadReadout empty_playhead_readout_{};
    static inline const WagonReadout empty_wagon_readout_{};
};

// =============================================================================
// TRAIN CLASS
// =============================================================================

class Train {
public:
    Train() {
        context_.playhead_valid_.fill(false);
        context_.wagon_valid_.fill(false);
        stat_values_.fill(0.0f);
    }
    
    // --- ATTACH ANALYZERS ---

    /**
     * Attach a Playhead to this Train.
     * Returns the slot index for use in lambdas via ctx.playhead(slot).
     * Returns -1 if at capacity.
     */
    int add_playhead() {
        if (playhead_count_ >= TRAIN_MAX_PLAYHEADS) return -1;
        return playhead_count_++;
    }

    /**
     * Attach a Wagon to this Train with the given span (in beats).
     * Returns the slot index for use in lambdas via ctx.wagon(slot).
     * Returns -1 if at capacity.
     */
    int add_wagon(float span_beats,
                  float offset_beats = 0.0f,
                  bool include_straddling = false,
                  bool include_active = false) {
        if (wagon_count_ >= TRAIN_MAX_WAGONS) return -1;
        int slot = wagon_count_++;
        wagons_[slot].set_span(span_beats);
        wagons_[slot].set_offset(offset_beats);
        wagons_[slot].set_include_straddling(include_straddling);
        wagons_[slot].set_include_active(include_active);
        return slot;
    }

    // --- DEFINE COMPUTATIONS ---
    
    /**
     * Define a computation that produces a stat.
     * Returns a handle for reading the result.
     * 
     * The lambda is stored inline (no heap allocation) as long as captures
     * fit within COMPUTE_FN_BUFFER_SIZE (32 bytes). Larger lambdas will
     * fail to compile with a clear error.
     */
    template<typename F>
    TrainStatId define(F&& fn) {
        if (stat_count_ >= TRAIN_MAX_STATS) {
            return TrainStatId{};
        }
        
        TrainStatId id(static_cast<uint16_t>(stat_count_));
        compute_fns_[stat_count_] = ComputeFn(std::forward<F>(fn));
        ++stat_count_;
        return id;
    }
    
    // --- UPDATE ---
    
    /**
     * Update the Train: run owned analyzers against the snapshot,
     * publish their readouts into the context, then evaluate all
     * defined stat lambdas.
     */
    void update(const StreamSnapshot& snap,
                const CompletedRing& history,
                float current_beat) {
        current_beat_ = current_beat;
        context_.beat_ = current_beat;

        // Run owned Playheads and publish readouts
        for (int i = 0; i < playhead_count_; ++i) {
            playheads_[i].update(snap, history);
            context_.playheads_[i] = playheads_[i].readout();
            context_.playhead_valid_[i] = true;
        }

        // Run owned Wagons and publish readouts
        for (int i = 0; i < wagon_count_; ++i) {
            wagons_[i].update(snap, history);
            context_.wagons_[i] = wagons_[i].readout();
            context_.wagon_valid_[i] = true;
        }

        // Evaluate stat lambdas
        for (int i = 0; i < stat_count_; ++i) {
            if (compute_fns_[i]) {
                stat_values_[i] = compute_fns_[i](context_);
            }
        }
    }
    
    // --- READ RESULTS ---
    
    float get(TrainStatId id) const {
        if (id.index < stat_count_) {
            return stat_values_[id.index];
        }
        return 0.0f;
    }
    
    float operator[](TrainStatId id) const {
        return get(id);
    }
    
    const std::array<float, TRAIN_MAX_STATS>& stat_values() const { 
        return stat_values_; 
    }
    
    int stat_count() const { return stat_count_; }
    const TrainContext& context() const { return context_; }
    float current_beat() const { return current_beat_; }
    
private:
    TrainContext context_;

    std::array<ComputeFn, TRAIN_MAX_STATS> compute_fns_;
    std::array<float, TRAIN_MAX_STATS> stat_values_;
    int stat_count_ = 0;

    float current_beat_ = 0.0f;

    // Owned analyzers (Train owns its own observers)
    std::array<Playhead, TRAIN_MAX_PLAYHEADS> playheads_;
    std::array<Wagon,    TRAIN_MAX_WAGONS>    wagons_;
    int playhead_count_ = 0;
    int wagon_count_    = 0;
};

// =============================================================================
// SIZE VERIFICATION
// =============================================================================

// ComputeFn: 32 (buffer) + 24 (pointers) = 56 bytes

// TrainContext memory:
//   playheads_[2]      = 2 * 536  = 1072 bytes
//   wagons_[2]         = 2 * 3100 = 6200 bytes
//   valid flags        = 4 bytes
//   beat_              = 4 bytes
//   Total              ~ 7.3 KB

// Train memory:
//   context_           ~ 7.3 KB
//   compute_fns_[32]   = 32 * 56  = 1792 bytes
//   stat_values_[32]   = 128 bytes
//   playheads_[2]      = 2 * 536  = 1072 bytes  (owned analyzers)
//   wagons_[2]         = 2 * 3100 = 6200 bytes  (owned analyzers)
//   scalars            ~ 16 bytes
//   Total              ~ 16.5 KB

} // namespace t7
