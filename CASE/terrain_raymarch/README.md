# terrain_raymarch

Raymarching render cartridge for 7T Musical Visualizer.

## Overview

Same scene as `terrain_pawn`, different rendering technique:

| Aspect | terrain_pawn | terrain_raymarch |
|--------|--------------|------------------|
| Rendering | Rasterization | Raymarching |
| Geometry | Vertex pulling | SDF evaluation |
| Draw calls | 2 (terrain + pawn) | 1 (fullscreen) |
| Vertex count | ~240,096 | 3 |
| GPU load | Vertex-bound | Fragment-bound |

## Usage

In `main.cpp`, change one line:

```cpp
// Rasterized
#include "cartridges/terrain_pawn/cartridge.hpp"
terrain_pawn::Cartridge renderer;

// Raymarched
#include "cartridges/terrain_raymarch/cartridge.hpp"
terrain_raymarch::Cartridge renderer;
```

Everything else stays identical.

## How It Works

**Fullscreen Triangle**
```wgsl
@vertex
fn fullscreen_vs(@builtin(vertex_index) vid: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(vid & 1u) * 4 - 1);
    let y = f32(i32((vid >> 1u) & 1u) * 4 - 1);
    return vec4<f32>(x, y, 0.0, 1.0);
}
```

**Fragment Shader Raymarches**
```wgsl
@fragment
fn world_fs(@builtin(position) frag_coord: vec4<f32>) -> @location(0) vec4<f32> {
    let ray = compute_ray(frag_coord);
    let hit = raymarch(cam_pos, ray);
    return shade(hit);
}
```

**Scene as SDFs**
```wgsl
fn sdf_terrain(p: vec3<f32>) -> f32 {
    return p.y - terrain_height(p.x, p.z);
}

fn sdf_pawn(p: vec3<f32>) -> f32 {
    return sdf_cone(transform_to_local(p), PAWN_HEIGHT, PAWN_RADIUS);
}
```

## Files

| File | Purpose |
|------|---------|
| `cartridge.hpp` | RenderCartridge implementation |
| `state.hpp` | GPU buffer management (identical to terrain_pawn) |
| `renderer.hpp` | Pipeline creation (simpler - one render pipeline) |
| `world.wgsl` | Raymarching shader |

## Benefits

- **Smooth shapes**: No polygon edges
- **Infinite detail**: Resolution-independent
- **Easy blending**: `smin(sdf_a, sdf_b, k)` for smooth unions
- **Future-ready**: Easy to add volumetrics, soft shadows, AO
