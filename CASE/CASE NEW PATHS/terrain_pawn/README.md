# Terrain Pawn - Render Cartridge

A visualization showing a procedural terrain with a movable pawn.

## Purpose

This cartridge serves as:
1. **The testbed** — Used during infrastructure development
2. **A template** — Copy this folder to create new render cartridges

## Input Consumed

| Source | Data | Usage |
|--------|------|-------|
| AnalysisSignal | `t_seconds`, `t_beats`, `dt` | Time for animations |
| AnalysisSignal | `stats[0]` (polyphony) | Terrain amplitude modulation |
| Console | `aspect_ratio` | Camera projection |
| Arrow keys | Movement intent | Pawn movement |
| Mouse drag (left) | Look delta | Camera orbit |
| Mouse drag (right) | Pan delta | Camera pan |
| Scroll | Zoom delta | Camera distance |

## GPU Signal

The cartridge combines all inputs into `GPUFrameSignal` (304 bytes) matching WGSL:

```
Offset  Field           Source
0       t_seconds       AnalysisSignal
4       t_beats         AnalysisSignal  
8       dt              AnalysisSignal
12      aspect_ratio    Console
16      stats[64]       AnalysisSignal
272     move_x          Arrow keys
276     move_z          Arrow keys
280     look_az_delta   Mouse left drag
284     look_el_delta   Mouse left drag
288     zoom_delta      Scroll
292     pan_x_delta     Mouse right drag
296     pan_y_delta     Mouse right drag
300     _pad1           (padding)
```

## Files

| File | Role |
|------|------|
| `cartridge.hpp` | RenderCartridge implementation |
| `state.hpp` | GPU buffer management |
| `renderer.hpp` | Pipeline and mesh creation |
| `world.wgsl` | Shader (compute + render) |

## Usage

```cpp
#include "cartridges/terrain_pawn/cartridge.hpp"

// Create and initialize
t7::terrain_pawn::Cartridge renderer;
renderer.initialize(device);
renderer.init_renderer(colorFormat, depthFormat, "shaders/world.wgsl");

// Per frame
renderer.update(analysisSignal, aspectRatio, queue);
renderer.render(encoder, backbuffer, depth);
```

## Creating a New Render Cartridge

1. Copy this folder to `cartridges/your_cartridge_name/`
2. Modify:
   - `state.hpp`: GPU structs and buffers
   - `renderer.hpp`: Pipelines and meshes
   - `*.wgsl`: Shaders
   - `cartridge.hpp`: Input handling, signal composition
3. Update `CMakeLists.txt` to include your new cartridge
