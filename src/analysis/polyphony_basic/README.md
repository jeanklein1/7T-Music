# Polyphony Basic - Analysis Cartridge

A minimal analysis cartridge that tracks polyphony (number of active notes).

## Purpose

This cartridge serves as:
1. **The testbed** — Used during infrastructure development
2. **A template** — Copy this folder to create new analysis cartridges

## Sources

- **MidiFile** — Plays `assets/default.mid` in a loop
- **KeyboardMidi** — Computer keyboard as piano (A-Z keys, plus `;`, `[`, `]`)

## Analysis Pipeline

```
MidiFile ──────┐
               ├──► MidiStream ──► Playhead ──► Train ──► polyphony
KeyboardMidi ──┘                                              │
                                                              ▼
                                                       AnalysisSignal
```

## Output Format

| Slot | Name | Range | Description |
|------|------|-------|-------------|
| `stats[0]` | polyphony | 0.0 – ~16.0 | Count of currently sounding notes |

## Usage

```cpp
#include "cartridges/polyphony_basic/canvas.hpp"

// Create and initialize
t7::polyphony_basic::Canvas analysis;
analysis.initialize("assets");

// Per frame
analysis.update(dt);
const t7::AnalysisSignal& signal = analysis.output();

// Read polyphony
float poly = signal.stat(0, t7::polyphony_basic::STAT_POLYPHONY);
```

## Creating a New Analysis Cartridge

1. Copy this folder to `cartridges/your_cartridge_name/`
2. Rename `Canvas` class if desired
3. Modify:
   - Sources (add/remove MidiFile, KeyboardMidi, etc.)
   - Analyzers (add Wagons, multiple Playheads, etc.)
   - Stats (define new Train computations)
   - Output format (document which slots mean what)
4. Update `CMakeLists.txt` to include your new cartridge
