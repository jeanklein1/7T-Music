#pragma once

/**
 * ANALYSIS CARTRIDGE - Interface for Musical Analysis Modules
 * ===========================================================
 * 
 * An analysis cartridge transforms MIDI events into musical statistics.
 * It owns the musical interpretation: which channels to listen to,
 * what analyzers to run, what stats to compute.
 * 
 * THE CATEGORICAL BOUNDARY
 * ------------------------
 * 
 * The analysis cartridge knows about music. It knows what polyphony means,
 * what a note onset is, how to compute pitch centroid.
 * 
 * It does NOT know about visualization. It has no concept of terrain,
 * cameras, or vertices. It just produces numbers in slots.
 * 
 * LIFECYCLE
 * ---------
 * 
 *   1. Console creates cartridge
 *   2. Console calls initialize()
 *   3. Per frame:
 *      a. Console routes input events via on_input()
 *      b. Console calls update(dt)
 *      c. Console reads output() and passes to render cartridges
 * 
 * WHAT THE CARTRIDGE OWNS
 * -----------------------
 * 
 *   - Clock (converts dt to beats)
 *   - MidiStream(s)
 *   - Sources (MidiFile, KeyboardMidi) that feed streams
 *   - Analyzers (Playhead, Wagon)
 *   - Train (stat computation)
 *   - The mapping of stats to slots
 */

#include "analysis/analysis_signal.hpp"
#include "core/input_event.hpp"

namespace t7 {

// =============================================================================
// ANALYSIS CARTRIDGE INTERFACE
// =============================================================================

class AnalysisCartridge {
public:
    virtual ~AnalysisCartridge() = default;
    
    // ─── LIFECYCLE ──────────────────────────────────────────────────────────
    
    /**
     * Initialize the cartridge.
     * Called once after construction.
     * 
     * @param asset_path  Path to assets folder (for loading MIDI files, etc.)
     */
    virtual void initialize(const char* asset_path) = 0;
    
    /**
     * Update the cartridge.
     * Called once per frame.
     * 
     * @param dt  Delta time in seconds (from console's wall clock)
     */
    virtual void update(float dt) = 0;
    
    // ─── INPUT ──────────────────────────────────────────────────────────────
    
    /**
     * Handle an input event.
     * The cartridge decides which events it cares about (e.g., musical keys).
     * 
     * @param event  The input event
     */
    virtual void on_input(const InputEvent& event) = 0;
    
    // ─── OUTPUT ─────────────────────────────────────────────────────────────
    
    /**
     * Get the current analysis output.
     * Valid after update() has been called.
     *
     * @return  Reference to the analysis signal (time + stats)
     */
    virtual const AnalysisSignal& output() const = 0;

    /**
     * Publish this cartridge's stat layout — the slot map that names which
     * (channel, slot_base, count) each stat group occupies. The render side
     * receives this once and resolves coupling sources by name, without ever
     * including the analysis cartridge's own headers.
     *
     * @return  Non-owning view over the cartridge's static STAT_LAYOUT.
     */
    virtual StatLayoutView stat_layout() const = 0;
};

} // namespace t7
