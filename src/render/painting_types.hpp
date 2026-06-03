#pragma once

// ─── painting_types.hpp ──────────────────────────────────────────
//
// Painting type definitions: reusable painting infrastructure for
// gallery cartridges. Supports framed paintings with configurable depth,
// aspect-ratio preservation, and shadow casting.
//
// Coordinate system: paintings are specified by wall + offset,
// automatically computing world position and normal; the system derives
// the right vector from normal × up for local-to-world transforms.
//
// Frame geometry: the canvas sits recessed within the frame, creating an
// authentic shadow-box effect. The frame has 8 visible faces — 4 outer
// (facing room) + 4 inner (facing canvas).
//
// Depends on: <cstdint>, <cmath>.

#include <cstdint>
#include <cmath>

namespace t7 {
namespace painting {


// ═══ §1 CONSTANTS ════════════════════════════════════════════════

constexpr uint32_t MAX_PAINTINGS = 16;
constexpr uint32_t PAINTING_CANVAS_SIZE = 2048;  // Texture array slice resolution

// Vertex counts
constexpr uint32_t PAINTING_CANVAS_VERTICES = 6;                    // 1 quad
constexpr uint32_t PAINTING_FRAME_FACES = 8;                        // 4 outer + 4 inner
constexpr uint32_t PAINTING_FRAME_VERTICES = PAINTING_FRAME_FACES * 6;  // 48 per painting

// Wall offset to avoid z-fighting
constexpr float WALL_OFFSET = 0.005f;


// ═══ §2 WALL ENUM — For intuitive painting placement ═════════════

enum class Wall : uint32_t {
    BACK  = 0,  // -Z wall, normal = (0, 0, 1)
    FRONT = 1,  // +Z wall, normal = (0, 0, -1)
    LEFT  = 2,  // -X wall, normal = (1, 0, 0)
    RIGHT = 3   // +X wall, normal = (-1, 0, 0)
};


// ═══ §3 GPU STRUCTURE — Must match world.wgsl exactly ════════════

struct alignas(16) GPUPainting {
    float position[3];      // Center point on wall surface (world space)
    float _pad0;
    float normal[3];        // Points into room (perpendicular to wall)
    float _pad1;
    float up[3];            // Usually (0,1,0)
    float _pad2;

    float width;            // Canvas width in world units
    float height;           // Canvas height in world units
    float frame_depth;      // How far frame protrudes from wall
    float frame_width;      // Frame border thickness (around canvas)

    float canvas_recess;    // Depth of canvas behind frame front
    uint32_t texture_layer; // Index into texture array
    float uv_scale_x;       // Image region within canvas (for aspect ratio)
    float uv_scale_y;

    float frame_color[3];   // RGB frame color
    float _pad3;
};

struct alignas(16) GPUPaintingHeader {
    uint32_t count;
    uint32_t _pad0;
    uint32_t _pad1;
    uint32_t _pad2;
};

static_assert(sizeof(GPUPainting) == 96, "GPUPainting must be 96 bytes");
static_assert(sizeof(GPUPaintingHeader) == 16, "GPUPaintingHeader must be 16 bytes");


// ═══ §4 CPU-SIDE PAINTING — With derived data ════════════════════

class Painting {
public:
    GPUPainting gpu;
    float right[3];  // Derived: cross(up, normal), normalized

    void compute_derived() {
        // right = up × normal (for door-local coordinate system)
        right[0] = gpu.up[1] * gpu.normal[2] - gpu.up[2] * gpu.normal[1];
        right[1] = gpu.up[2] * gpu.normal[0] - gpu.up[0] * gpu.normal[2];
        right[2] = gpu.up[0] * gpu.normal[1] - gpu.up[1] * gpu.normal[0];

        float len = std::sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
        if (len > 0.0001f) {
            right[0] /= len;
            right[1] /= len;
            right[2] /= len;
        }
    }
};


// ═══ §5 PAINTING BUILDER — Fluent API for painting configuration ═

class PaintingBuilder {
public:
    PaintingBuilder() {
        p_.gpu = {};
        p_.gpu.up[1] = 1.0f;           // Default up = Y
        p_.gpu.normal[2] = 1.0f;       // Default normal = +Z (back wall)
        p_.gpu.width = 2.0f;
        p_.gpu.height = 1.5f;
        p_.gpu.frame_depth = 0.06f;
        p_.gpu.frame_width = 0.08f;
        p_.gpu.canvas_recess = 0.04f;  // Canvas slightly behind frame front
        p_.gpu.texture_layer = 0;
        p_.gpu.uv_scale_x = 1.0f;
        p_.gpu.uv_scale_y = 1.0f;
        p_.gpu.frame_color[0] = 0.15f;
        p_.gpu.frame_color[1] = 0.12f;
        p_.gpu.frame_color[2] = 0.10f;
    }

    // ── Position Methods ─────────────────────────────────────────

    // Place on specific wall with offset along wall and height
    PaintingBuilder& on_wall(Wall wall, float offset_along, float height,
                             float room_half_width, float room_half_depth) {
        switch (wall) {
            case Wall::BACK:
                p_.gpu.position[0] = offset_along;
                p_.gpu.position[1] = height;
                p_.gpu.position[2] = -room_half_depth;
                p_.gpu.normal[0] = 0.0f;
                p_.gpu.normal[1] = 0.0f;
                p_.gpu.normal[2] = 1.0f;
                break;
            case Wall::FRONT:
                p_.gpu.position[0] = offset_along;
                p_.gpu.position[1] = height;
                p_.gpu.position[2] = room_half_depth;
                p_.gpu.normal[0] = 0.0f;
                p_.gpu.normal[1] = 0.0f;
                p_.gpu.normal[2] = -1.0f;
                break;
            case Wall::LEFT:
                p_.gpu.position[0] = -room_half_width;
                p_.gpu.position[1] = height;
                p_.gpu.position[2] = offset_along;
                p_.gpu.normal[0] = 1.0f;
                p_.gpu.normal[1] = 0.0f;
                p_.gpu.normal[2] = 0.0f;
                break;
            case Wall::RIGHT:
                p_.gpu.position[0] = room_half_width;
                p_.gpu.position[1] = height;
                p_.gpu.position[2] = offset_along;
                p_.gpu.normal[0] = -1.0f;
                p_.gpu.normal[1] = 0.0f;
                p_.gpu.normal[2] = 0.0f;
                break;
        }
        return *this;
    }

    // Direct position and normal (for non-axis-aligned walls)
    PaintingBuilder& at(float x, float y, float z) {
        p_.gpu.position[0] = x;
        p_.gpu.position[1] = y;
        p_.gpu.position[2] = z;
        return *this;
    }

    PaintingBuilder& facing(float nx, float ny, float nz) {
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        p_.gpu.normal[0] = nx / len;
        p_.gpu.normal[1] = ny / len;
        p_.gpu.normal[2] = nz / len;
        return *this;
    }

    // ── Size Methods ─────────────────────────────────────────────

    // Set canvas size directly
    PaintingBuilder& size(float w, float h) {
        p_.gpu.width = w;
        p_.gpu.height = h;
        return *this;
    }

    // Set width, derive height from aspect ratio
    PaintingBuilder& width_with_aspect(float w, float aspect_ratio) {
        p_.gpu.width = w;
        p_.gpu.height = w / aspect_ratio;
        return *this;
    }

    // ── Frame Methods ────────────────────────────────────────────

    PaintingBuilder& frame(float depth, float border_width, float recess) {
        p_.gpu.frame_depth = depth;
        p_.gpu.frame_width = border_width;
        p_.gpu.canvas_recess = recess;
        return *this;
    }

    PaintingBuilder& frame_color(float r, float g, float b) {
        p_.gpu.frame_color[0] = r;
        p_.gpu.frame_color[1] = g;
        p_.gpu.frame_color[2] = b;
        return *this;
    }

    // ── Texture Methods ──────────────────────────────────────────

    PaintingBuilder& texture(uint32_t layer, float uv_scale_x = 1.0f, float uv_scale_y = 1.0f) {
        p_.gpu.texture_layer = layer;
        p_.gpu.uv_scale_x = uv_scale_x;
        p_.gpu.uv_scale_y = uv_scale_y;
        return *this;
    }

    // ── Build ────────────────────────────────────────────────────

    Painting build() {
        p_.compute_derived();
        return p_;
    }

private:
    Painting p_;
};


} // namespace painting
} // namespace t7
