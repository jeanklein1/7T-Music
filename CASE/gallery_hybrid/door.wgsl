// ═══════════════════════════════════════════════════════════════════════════════
// PORTABLE DOOR — GPU Shader Code
// ═══════════════════════════════════════════════════════════════════════════════
//
// Copy this into your world.wgsl or include via preprocessor.
// Must match door.hpp GPUPortableDoor exactly.
//
// PROVIDES:
//   - PortableDoor struct
//   - 2D SDF shape library
//   - 3D queries (silhouette, volume, commitment)
//   - Door hole carving for wall SDFs
//   - Door void rendering
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 DATA STRUCTURE — Must match door.hpp exactly
// ═══════════════════════════════════════════════════════════════════════════════

struct PortableDoor {
    // Placement (48 bytes)
    position: vec3<f32>,
    _pad0: f32,
    forward: vec3<f32>,
    _pad1: f32,
    up: vec3<f32>,
    _pad2: f32,
    
    // Dimensions (16 bytes)
    width: f32,
    height: f32,
    thickness: f32,
    commitment_depth: f32,
    
    // Identity (16 bytes)
    shape_type: u32,
    target_id: u32,
    _pad3: u32,
    _pad4: u32,
    
    // Appearance (16 bytes)
    color: vec3<f32>,
    glow: f32,
}

// Shape type constants
const DOOR_SHAPE_RECTANGLE: u32 = 0u;
const DOOR_SHAPE_ARCH: u32 = 1u;
const DOOR_SHAPE_CIRCLE: u32 = 2u;
const DOOR_SHAPE_POINTED_ARCH: u32 = 3u;

const DOOR_TARGET_NONE: u32 = 0xFFFFFFFFu;


// ═══════════════════════════════════════════════════════════════════════════════
// §2 COORDINATE TRANSFORM
// ═══════════════════════════════════════════════════════════════════════════════

// Compute right vector from door's forward and up
fn door_right(door: PortableDoor) -> vec3<f32> {
    return normalize(cross(door.up, door.forward));
}

// Transform world position to door-local space
// Local: X = across, Y = up, Z = through (positive = exit side)
fn door_world_to_local(p: vec3<f32>, door: PortableDoor) -> vec3<f32> {
    let d = p - door.position;
    let right = door_right(door);
    
    return vec3(
        dot(d, right),
        dot(d, door.up),
        dot(d, door.forward)
    );
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 2D SDF SHAPES — In door's XY plane
// ═══════════════════════════════════════════════════════════════════════════════

fn door_sdf_2d_rectangle(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let center = vec2(0.0, h * 0.5);
    let size = vec2(half_w, h * 0.5);
    let d = abs(p - center) - size;
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
}

fn door_sdf_2d_arch(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let arch_y = h - half_w;
    
    if (p.y > arch_y) {
        // Semicircle region
        let arch_center = vec2(0.0, arch_y);
        return length(p - arch_center) - half_w;
    } else {
        // Rectangle region
        let d = abs(p) - vec2(half_w, arch_y);
        return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
    }
}

fn door_sdf_2d_circle(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let radius = min(half_w, h * 0.5);
    let center = vec2(0.0, radius);
    return length(p - center) - radius;
}

fn door_sdf_2d_pointed_arch(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let arch_y = h * 0.6;
    
    if (p.y > arch_y) {
        // Gothic arch: intersection of two circles
        let radius = half_w * 1.5;
        let left_center = vec2(-half_w * 0.5, arch_y);
        let right_center = vec2(half_w * 0.5, arch_y);
        
        let d_left = length(p - left_center) - radius;
        let d_right = length(p - right_center) - radius;
        
        return max(d_left, d_right);
    } else {
        // Rectangle below
        let d = abs(p) - vec2(half_w, arch_y);
        return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
    }
}

// ─── Shape dispatcher ────────────────────────────────────────────────────────

fn door_sdf_2d(p: vec2<f32>, half_w: f32, h: f32, shape_type: u32) -> f32 {
    switch shape_type {
        case DOOR_SHAPE_RECTANGLE: {
            return door_sdf_2d_rectangle(p, half_w, h);
        }
        case DOOR_SHAPE_ARCH: {
            return door_sdf_2d_arch(p, half_w, h);
        }
        case DOOR_SHAPE_CIRCLE: {
            return door_sdf_2d_circle(p, half_w, h);
        }
        case DOOR_SHAPE_POINTED_ARCH: {
            return door_sdf_2d_pointed_arch(p, half_w, h);
        }
        default: {
            return door_sdf_2d_rectangle(p, half_w, h);
        }
    }
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 3D QUERIES
// ═══════════════════════════════════════════════════════════════════════════════

// Is point inside the door's 2D silhouette (ignoring Z)?
fn door_point_in_silhouette(p: vec3<f32>, door: PortableDoor) -> bool {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    return door_sdf_2d(local.xy, half_w, door.height, door.shape_type) < 0.0;
}

// Is point inside the door's passable volume?
fn door_point_in_volume(p: vec3<f32>, door: PortableDoor) -> bool {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    
    if (door_sdf_2d(local.xy, half_w, door.height, door.shape_type) >= 0.0) {
        return false;
    }
    return abs(local.z) < door.thickness * 0.5;
}

// Is point on the exit side of the door?
fn door_point_on_exit_side(p: vec3<f32>, door: PortableDoor) -> bool {
    let local = door_world_to_local(p, door);
    return local.z > 0.0;
}

// Is point past commitment threshold?
fn door_point_committed(p: vec3<f32>, door: PortableDoor) -> bool {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    
    if (door_sdf_2d(local.xy, half_w, door.height, door.shape_type) >= 0.0) {
        return false;
    }
    return local.z > door.commitment_depth;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 SDF FOR WALL CARVING
// ═══════════════════════════════════════════════════════════════════════════════
//
// Use this to subtract door holes from walls.
// Returns negative inside the door hole volume.

fn door_sdf_hole(p: vec3<f32>, door: PortableDoor) -> f32 {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    
    // 2D shape distance
    let d_2d = door_sdf_2d(local.xy, half_w, door.height, door.shape_type);
    
    // Z-axis thickness (how deep the hole goes)
    let d_z = abs(local.z) - door.thickness * 0.5;
    
    // 3D: intersection of 2D shape and depth slab
    return max(d_2d, d_z);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 RAY-DOOR INTERSECTION
// ═══════════════════════════════════════════════════════════════════════════════
//
// For rendering the void through the door (what you see when looking through).

fn door_ray_intersect(ro: vec3<f32>, rd: vec3<f32>, door: PortableDoor) -> f32 {
    // Intersect ray with door plane (at door.position, normal = door.forward)
    let denom = dot(rd, door.forward);
    
    // If ray is parallel or facing away, no hit
    if (abs(denom) < 0.001) {
        return -1.0;
    }
    
    let t = dot(door.position - ro, door.forward) / denom;
    if (t < 0.0) {
        return -1.0;
    }
    
    // Check if hit point is inside door shape
    let hit_p = ro + rd * t;
    let local = door_world_to_local(hit_p, door);
    let half_w = door.width * 0.5;
    
    if (door_sdf_2d(local.xy, half_w, door.height, door.shape_type) < 0.0) {
        return t;
    }
    
    return -1.0;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 DOOR FRAME SDF (OPTIONAL)
// ═══════════════════════════════════════════════════════════════════════════════
//
// For rendering a visible door frame around the opening.
// Returns distance to the frame geometry.

const DOOR_FRAME_WIDTH: f32 = 0.15;
const DOOR_FRAME_DEPTH: f32 = 0.1;

fn door_sdf_frame(p: vec3<f32>, door: PortableDoor) -> f32 {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    
    // Inner opening
    let d_inner = door_sdf_2d(local.xy, half_w, door.height, door.shape_type);
    
    // Outer boundary (frame surrounds opening)
    let d_outer = door_sdf_2d(local.xy, half_w + DOOR_FRAME_WIDTH, 
                               door.height + DOOR_FRAME_WIDTH, door.shape_type);
    
    // Frame is the shell between inner and outer
    let d_2d_frame = max(-d_inner, d_outer);
    
    // Extrude in Z
    let d_z = abs(local.z) - DOOR_FRAME_DEPTH;
    
    return max(d_2d_frame, d_z);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §8 SHADING HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

const DOOR_VOID_COLOR: vec3<f32> = vec3(0.02, 0.02, 0.03);

fn door_shade_void(hit_dist: f32, door: PortableDoor) -> vec3<f32> {
    // Gradient from door color at edge to pure void at depth
    let depth_factor = min(hit_dist * 0.1, 1.0);
    let base = mix(door.color * 0.3, DOOR_VOID_COLOR, depth_factor);
    
    // Add glow if active
    let glow_color = door.color * door.glow * 0.5;
    return base + glow_color;
}

fn door_shade_frame(normal: vec3<f32>, door: PortableDoor) -> vec3<f32> {
    // Simple diffuse shading on frame
    let light_dir = normalize(vec3(0.3, 0.9, 0.2));
    let ndotl = max(dot(normal, light_dir), 0.0);
    return door.color * (0.4 + 0.6 * ndotl);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §9 USAGE EXAMPLE — Integrate into your scene
// ═══════════════════════════════════════════════════════════════════════════════
//
// // In your bindings:
// @group(0) @binding(N) var<storage, read> doors: array<PortableDoor, 8>;
// @group(0) @binding(M) var<uniform> door_count: u32;
//
// // Carve all door holes from a wall SDF:
// fn carve_doors_from_wall(wall_sdf: f32, p: vec3<f32>) -> f32 {
//     var result = wall_sdf;
//     for (var i: u32 = 0u; i < door_count; i++) {
//         let hole = door_sdf_hole(p, doors[i]);
//         result = max(result, -hole);  // Subtract hole from wall
//     }
//     return result;
// }
//
// // Check for door void hits in raymarch:
// fn check_door_voids(ro: vec3<f32>, rd: vec3<f32>) -> f32 {
//     var closest_t = 9999.0;
//     for (var i: u32 = 0u; i < door_count; i++) {
//         let t = door_ray_intersect(ro, rd, doors[i]);
//         if (t > 0.0 && t < closest_t) {
//             closest_t = t;
//         }
//     }
//     return closest_t;
// }


// ═══════════════════════════════════════════════════════════════════════════════
// END OF PORTABLE DOOR SHADER CODE
// ═══════════════════════════════════════════════════════════════════════════════
