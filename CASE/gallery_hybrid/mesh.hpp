#pragma once

/**
 * GALLERY MESH GENERATION
 * =======================
 *
 * Generates room geometry with door cutouts.
 * Each wall is broken into quads around door openings.
 *
 * COORDINATE SYSTEM:
 *   Y = up
 *   Room centered at origin
 *   Walls at ±ROOM_HALF_WIDTH (X), ±ROOM_HALF_DEPTH (Z)
 *
 * MATERIALS (encoded in vertex):
 *   0 = Floor
 *   1 = Ceiling
 *   2 = Wall
 *   3 = Door void (black quad in doorway)
 */

#include <vector>
#include <cmath>
#include <algorithm>

namespace t7 {
namespace gallery {

// ═══════════════════════════════════════════════════════════════════════════════
// §1 ROOM DIMENSIONS
// ═══════════════════════════════════════════════════════════════════════════════

constexpr float ROOM_HALF_WIDTH = 15.0f;
constexpr float ROOM_HEIGHT = 6.0f;
constexpr float ROOM_HALF_DEPTH = 15.0f;

// Material IDs
constexpr uint32_t MAT_FLOOR = 0;
constexpr uint32_t MAT_CEILING = 1;
constexpr uint32_t MAT_WALL = 2;
constexpr uint32_t MAT_DOOR_VOID = 3;
constexpr uint32_t MAT_PAWN = 4;


// ═══════════════════════════════════════════════════════════════════════════════
// §2 VERTEX FORMAT
// ═══════════════════════════════════════════════════════════════════════════════

struct Vertex {
    float position[3];
    float normal[3];
    float uv[2];
    uint32_t material;
    uint32_t _pad[3];  // Align to 48 bytes
};

static_assert(sizeof(Vertex) == 48, "Vertex must be 48 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §3 DOOR INFO (simplified for mesh generation)
// ═══════════════════════════════════════════════════════════════════════════════

struct DoorInfo {
    float center_x;       // Position along wall
    float width;
    float height;
    
    float left() const { return center_x - width * 0.5f; }
    float right() const { return center_x + width * 0.5f; }
};


// ═══════════════════════════════════════════════════════════════════════════════
// §4 MESH BUILDER
// ═══════════════════════════════════════════════════════════════════════════════

class MeshBuilder {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // ─── Quad helper ──────────────────────────────────────────────────────────
    // Adds a quad with 4 vertices and 6 indices (2 triangles)
    // Vertices should be specified counter-clockwise when viewed from front
    
    void add_quad(
        float x0, float y0, float z0,
        float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3,
        float nx, float ny, float nz,
        uint32_t material
    ) {
        uint32_t base = static_cast<uint32_t>(vertices.size());
        
        // Calculate UVs based on world position (for tiling)
        auto calc_uv = [](float x, float y, float z, float nx, float ny, float nz) -> std::pair<float, float> {
            if (std::abs(ny) > 0.5f) {
                // Floor/ceiling: use XZ
                return { x * 0.5f, z * 0.5f };
            } else if (std::abs(nx) > 0.5f) {
                // Side walls: use ZY
                return { z * 0.5f, y * 0.5f };
            } else {
                // Front/back walls: use XY
                return { x * 0.5f, y * 0.5f };
            }
        };
        
        auto [u0, v0] = calc_uv(x0, y0, z0, nx, ny, nz);
        auto [u1, v1] = calc_uv(x1, y1, z1, nx, ny, nz);
        auto [u2, v2] = calc_uv(x2, y2, z2, nx, ny, nz);
        auto [u3, v3] = calc_uv(x3, y3, z3, nx, ny, nz);
        
        vertices.push_back({{ x0, y0, z0 }, { nx, ny, nz }, { u0, v0 }, material, {0,0,0}});
        vertices.push_back({{ x1, y1, z1 }, { nx, ny, nz }, { u1, v1 }, material, {0,0,0}});
        vertices.push_back({{ x2, y2, z2 }, { nx, ny, nz }, { u2, v2 }, material, {0,0,0}});
        vertices.push_back({{ x3, y3, z3 }, { nx, ny, nz }, { u3, v3 }, material, {0,0,0}});
        
        // Two triangles (CCW winding)
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
};


// ═══════════════════════════════════════════════════════════════════════════════
// §5 ROOM GENERATION
// ═══════════════════════════════════════════════════════════════════════════════

namespace RoomGen {

// ─── Floor ───────────────────────────────────────────────────────────────────

inline void add_floor(MeshBuilder& mb) {
    // Floor at y=0, normal pointing up
    // Vertices CCW when viewed from above (from inside room looking down)
    mb.add_quad(
        -ROOM_HALF_WIDTH, 0.0f, -ROOM_HALF_DEPTH,  // 0: back-left
         ROOM_HALF_WIDTH, 0.0f, -ROOM_HALF_DEPTH,  // 1: back-right
         ROOM_HALF_WIDTH, 0.0f,  ROOM_HALF_DEPTH,  // 2: front-right
        -ROOM_HALF_WIDTH, 0.0f,  ROOM_HALF_DEPTH,  // 3: front-left
        0.0f, 1.0f, 0.0f,  // Normal up
        MAT_FLOOR
    );
}

// ─── Ceiling ─────────────────────────────────────────────────────────────────

inline void add_ceiling(MeshBuilder& mb) {
    // Ceiling at y=ROOM_HEIGHT, normal pointing down
    // Vertices CCW when viewed from below (from inside room looking up)
    mb.add_quad(
        -ROOM_HALF_WIDTH, ROOM_HEIGHT, -ROOM_HALF_DEPTH,  // 0: back-left
        -ROOM_HALF_WIDTH, ROOM_HEIGHT,  ROOM_HALF_DEPTH,  // 1: front-left
         ROOM_HALF_WIDTH, ROOM_HEIGHT,  ROOM_HALF_DEPTH,  // 2: front-right
         ROOM_HALF_WIDTH, ROOM_HEIGHT, -ROOM_HALF_DEPTH,  // 3: back-right
        0.0f, -1.0f, 0.0f,  // Normal down
        MAT_CEILING
    );
}

// ─── Wall with door cutout ───────────────────────────────────────────────────
//
// Wall is split into up to 3 quads:
//   - Left section (if door doesn't touch left edge)
//   - Right section (if door doesn't touch right edge)
//   - Top section (above door)
//
//  +--------+------+--------+
//  |        | TOP  |        |
//  |  LEFT  +------+ RIGHT  |
//  |        |DOOR  |        |
//  |        |      |        |
//  +--------+------+--------+

inline void add_wall_with_door(
    MeshBuilder& mb,
    float wall_min, float wall_max,  // Wall extent (X or Z depending on orientation)
    float wall_pos,                   // Wall position (Z for back/front, X for left/right)
    float nx, float ny, float nz,     // Normal
    bool is_x_wall,                   // true = back/front wall (varies in X), false = side wall (varies in Z)
    const DoorInfo* door              // nullptr if no door
) {
    auto add_wall_quad = [&](float min_u, float max_u, float min_v, float max_v) {
        float x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
        
        if (is_x_wall) {
            // Back or front wall (normal in ±Z)
            x0 = min_u; y0 = min_v; z0 = wall_pos;
            x1 = min_u; y1 = max_v; z1 = wall_pos;
            x2 = max_u; y2 = max_v; z2 = wall_pos;
            x3 = max_u; y3 = min_v; z3 = wall_pos;
        } else {
            // Left or right wall (normal in ±X)
            x0 = wall_pos; y0 = min_v; z0 = min_u;
            x1 = wall_pos; y1 = max_v; z1 = min_u;
            x2 = wall_pos; y2 = max_v; z2 = max_u;
            x3 = wall_pos; y3 = min_v; z3 = max_u;
        }
        
        mb.add_quad(x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3, nx, ny, nz, MAT_WALL);
    };
    
    if (!door) {
        // No door: full wall
        add_wall_quad(wall_min, wall_max, 0.0f, ROOM_HEIGHT);
        return;
    }
    
    float door_left = door->left();
    float door_right = door->right();
    float door_top = door->height;
    
    // Left section
    if (door_left > wall_min + 0.01f) {
        add_wall_quad(wall_min, door_left, 0.0f, ROOM_HEIGHT);
    }
    
    // Right section
    if (door_right < wall_max - 0.01f) {
        add_wall_quad(door_right, wall_max, 0.0f, ROOM_HEIGHT);
    }
    
    // Top section (above door)
    if (door_top < ROOM_HEIGHT - 0.01f) {
        add_wall_quad(door_left, door_right, door_top, ROOM_HEIGHT);
    }
}

// ─── Door void (black quad in doorway) ───────────────────────────────────────

inline void add_door_void(
    MeshBuilder& mb,
    float wall_pos,
    float nx, float ny, float nz,
    bool is_x_wall,
    const DoorInfo& door
) {
    float offset = 0.01f;  // Slight offset to avoid z-fighting
    float void_pos = wall_pos - nz * offset - nx * offset;  // Push into doorway
    
    float x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
    
    if (is_x_wall) {
        x0 = door.left();  y0 = 0.0f;         z0 = void_pos;
        x1 = door.left();  y1 = door.height;  z1 = void_pos;
        x2 = door.right(); y2 = door.height;  z2 = void_pos;
        x3 = door.right(); y3 = 0.0f;         z3 = void_pos;
    } else {
        x0 = void_pos; y0 = 0.0f;         z0 = door.left();
        x1 = void_pos; y1 = door.height;  z1 = door.left();
        x2 = void_pos; y2 = door.height;  z2 = door.right();
        x3 = void_pos; y3 = 0.0f;         z3 = door.right();
    }
    
    mb.add_quad(x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3, nx, ny, nz, MAT_DOOR_VOID);
}

} // namespace RoomGen


// ═══════════════════════════════════════════════════════════════════════════════
// §6 PAWN MESH
// ═══════════════════════════════════════════════════════════════════════════════

namespace PawnGen {

inline void add_cone(
    MeshBuilder& mb,
    float base_radius,
    float tip_radius,
    float height,
    int segments
) {
    uint32_t base_center_idx = static_cast<uint32_t>(mb.vertices.size());
    
    // Base center
    mb.vertices.push_back({{ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f }, MAT_PAWN, {0,0,0}});
    
    // Base ring
    uint32_t base_ring_start = static_cast<uint32_t>(mb.vertices.size());
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = base_radius * std::cos(angle);
        float z = base_radius * std::sin(angle);
        float u = 0.5f + 0.5f * std::cos(angle);
        float v = 0.5f + 0.5f * std::sin(angle);
        mb.vertices.push_back({{ x, 0.0f, z }, { 0.0f, -1.0f, 0.0f }, { u, v }, MAT_PAWN, {0,0,0}});
    }
    
    // Base triangles
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        mb.indices.push_back(base_center_idx);
        mb.indices.push_back(base_ring_start + next);
        mb.indices.push_back(base_ring_start + i);
    }
    
    // Tip
    uint32_t tip_idx = static_cast<uint32_t>(mb.vertices.size());
    mb.vertices.push_back({{ 0.0f, height, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f }, MAT_PAWN, {0,0,0}});
    
    // Side vertices with proper normals
    uint32_t side_ring_start = static_cast<uint32_t>(mb.vertices.size());
    float slope = base_radius / height;
    float ny = slope / std::sqrt(1.0f + slope * slope);
    float nxz = 1.0f / std::sqrt(1.0f + slope * slope);
    
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = base_radius * std::cos(angle);
        float z = base_radius * std::sin(angle);
        float nx = nxz * std::cos(angle);
        float nz_val = nxz * std::sin(angle);
        mb.vertices.push_back({{ x, 0.0f, z }, { nx, ny, nz_val }, { (float)i / segments, 0.0f }, MAT_PAWN, {0,0,0}});
    }
    
    // Side triangles
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        mb.indices.push_back(side_ring_start + i);
        mb.indices.push_back(side_ring_start + next);
        mb.indices.push_back(tip_idx);
    }
}

} // namespace PawnGen


// ═══════════════════════════════════════════════════════════════════════════════
// §7 FULL ROOM GENERATION
// ═══════════════════════════════════════════════════════════════════════════════

struct RoomConfig {
    DoorInfo* back_door = nullptr;   // At z = -ROOM_HALF_DEPTH
    DoorInfo* front_door = nullptr;  // At z = +ROOM_HALF_DEPTH
    DoorInfo* left_door = nullptr;   // At x = -ROOM_HALF_WIDTH
    DoorInfo* right_door = nullptr;  // At x = +ROOM_HALF_WIDTH
};

inline void generate_room(MeshBuilder& mb, const RoomConfig& config) {
    using namespace RoomGen;
    
    // Floor and ceiling
    add_floor(mb);
    add_ceiling(mb);
    
    // Back wall (z = -ROOM_HALF_DEPTH, normal = +Z)
    add_wall_with_door(mb, -ROOM_HALF_WIDTH, ROOM_HALF_WIDTH, -ROOM_HALF_DEPTH,
                       0.0f, 0.0f, 1.0f, true, config.back_door);
    if (config.back_door) {
        add_door_void(mb, -ROOM_HALF_DEPTH, 0.0f, 0.0f, 1.0f, true, *config.back_door);
    }
    
    // Front wall (z = +ROOM_HALF_DEPTH, normal = -Z)
    add_wall_with_door(mb, -ROOM_HALF_WIDTH, ROOM_HALF_WIDTH, ROOM_HALF_DEPTH,
                       0.0f, 0.0f, -1.0f, true, config.front_door);
    if (config.front_door) {
        add_door_void(mb, ROOM_HALF_DEPTH, 0.0f, 0.0f, -1.0f, true, *config.front_door);
    }
    
    // Left wall (x = -ROOM_HALF_WIDTH, normal = +X)
    add_wall_with_door(mb, -ROOM_HALF_DEPTH, ROOM_HALF_DEPTH, -ROOM_HALF_WIDTH,
                       1.0f, 0.0f, 0.0f, false, config.left_door);
    if (config.left_door) {
        add_door_void(mb, -ROOM_HALF_WIDTH, 1.0f, 0.0f, 0.0f, false, *config.left_door);
    }
    
    // Right wall (x = +ROOM_HALF_WIDTH, normal = -X)
    add_wall_with_door(mb, -ROOM_HALF_DEPTH, ROOM_HALF_DEPTH, ROOM_HALF_WIDTH,
                       -1.0f, 0.0f, 0.0f, false, config.right_door);
    if (config.right_door) {
        add_door_void(mb, ROOM_HALF_WIDTH, -1.0f, 0.0f, 0.0f, false, *config.right_door);
    }
}

inline void generate_pawn(MeshBuilder& mb) {
    PawnGen::add_cone(mb, 0.5f, 0.0f, 1.5f, 16);
}


} // namespace gallery
} // namespace t7
