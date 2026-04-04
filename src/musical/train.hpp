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
 * USAGE
 * -----
 * 
 *     // Setup
 *     MidiStream stream(router, 0);
 *     Playhead playhead;
 *     Wagon wagon(4.0f);
 *     Train train;
 *     
 *     // Define computations
 *     auto HUE = train.define([](const TrainContext& ctx) {
 *         float pitch = playhead_centroid(ctx.playhead(0)) / 127.0f;
 *         float density = wagon_coverage(ctx.wagon(0));
 *         return pitch * (0.5f + density * 0.5f);
 *     });
 *     
 *     // Runtime
 *     stream.update(beat);
 *     auto snap = stream.snapshot();
 *     playhead.update(snap, stream.history());
 *     wagon.update(snap, stream.history());
 *     
 *     train.set_playhead(0, playhead.readout());
 *     train.set_wagon(0, wagon.readout());
 *     train.update(beat);
 *     
 *     float hue = train.get(HUE);
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

constexpr int TRAIN_MAX_PLAYHEADS = 8;
constexpr int TRAIN_MAX_WAGONS = 8;
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
    
    // --- SET READOUTS (values, copied) ---
    
    Train& set_playhead(int slot, const PlayheadReadout& readout) {
        if (slot >= 0 && slot < TRAIN_MAX_PLAYHEADS) {
            context_.playheads_[slot] = readout;
            context_.playhead_valid_[slot] = true;
        }
        return *this;
    }
    
    Train& clear_playhead(int slot) {
        if (slot >= 0 && slot < TRAIN_MAX_PLAYHEADS) {
            context_.playhead_valid_[slot] = false;
        }
        return *this;
    }
    
    Train& set_wagon(int slot, const WagonReadout& readout) {
        if (slot >= 0 && slot < TRAIN_MAX_WAGONS) {
            context_.wagons_[slot] = readout;
            context_.wagon_valid_[slot] = true;
        }
        return *this;
    }
    
    Train& clear_wagon(int slot) {
        if (slot >= 0 && slot < TRAIN_MAX_WAGONS) {
            context_.wagon_valid_[slot] = false;
        }
        return *this;
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
     * Recompute all stats.
     * Call after setting all readouts for this frame.
     */
    void update(float current_beat) {
        current_beat_ = current_beat;
        context_.beat_ = current_beat;
        
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
};

// =============================================================================
// SIZE VERIFICATION
// =============================================================================

// ComputeFn: 32 (buffer) + 24 (pointers) = 56 bytes

// TrainContext memory:
//   playheads_[8]      = 8 * 536 = 4288 bytes
//   wagons_[8]         = 8 * 3100 = 24800 bytes
//   valid flags        = 16 bytes
//   beat_              = 4 bytes
//   Total              ~ 29 KB

// Train memory:
//   context_           ~ 29 KB
//   compute_fns_[32]   = 32 * 56 = 1792 bytes
//   stat_values_[32]   = 128 bytes
//   scalars            ~ 8 bytes
//   Total              ~ 31 KB

} // namespace t7
