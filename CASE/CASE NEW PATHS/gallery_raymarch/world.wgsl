// ═══════════════════════════════════════════════════════════════════════════════
// GALLERY CARTRIDGE — GPU Scroll (Refactored)
// ═══════════════════════════════════════════════════════════════════════════════
//
// ARCHITECTURAL CHANGE: "Carved Solid" instead of "Assembled Planes"
//
// The room is now modeled as NEGATIVE SPACE carved from infinite solid material.
// This gives us:
//   - Natural corners where floor meets wall (shared SDF edges)
//   - Single material assignment based on which surface is closest
//   - Floor tiling as pure shading, not geometry
//   - Door passages as proper CSG subtractions
//
// TEXTURE ARCHITECTURE:
//   room_field (128×128, RGBA16Float):
//     .r = room interior SDF (negative inside, positive in walls)
//     .g = nearest surface ID (1=floor, 2=wall, 3=ceiling) encoded
//     .b = reserved
//     .a = reserved
//
// SCROLL INDEX:
//   §1    CONFIG
//   §2    STRUCTURES (UNCHANGED - must match state.hpp)
//   §3    DOOR SDF (UNCHANGED)
//   §4    ROOM FIELD COMPUTE (REFACTORED)
//   §5    ENTITIES (UNCHANGED)
//   §6    COUPLINGS (UNCHANGED)
//   §7    RENDER (REFACTORED - unified room SDF)
//   §8    BINDINGS (UNCHANGED)
//   §9    ENTRY POINTS (UNCHANGED)
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 CONFIG
// ═══════════════════════════════════════════════════════════════════════════════

const PI: f32 = 3.14159265359;

const COUPLING_INPUT_MOVES_PAWN:        u32 = 1u << 0u;
const COUPLING_PAWN_TO_CAMERA_TARGET:   u32 = 1u << 1u;
const COUPLING_INPUT_ORBITS_CAMERA:     u32 = 1u << 2u;
const COUPLING_INPUT_ZOOMS_CAMERA:      u32 = 1u << 3u;

// Room geometry - single source of truth
const ROOM_HALF_WIDTH: f32 = 15.0;
const ROOM_HEIGHT: f32 = 6.0;
const ROOM_HALF_DEPTH: f32 = 15.0;
const WALL_THICKNESS: f32 = 0.5;

const FIELD_SIZE: f32 = 128.0;

// Floor tiling (shading only, not geometry)
const FLOOR_TILE_SIZE: f32 = 2.0;
const FLOOR_COLOR_LIGHT: vec3<f32> = vec3(0.75, 0.65, 0.55);
const FLOOR_COLOR_DARK: vec3<f32> = vec3(0.25, 0.15, 0.35);

// Pawn
const PAWN_RADIUS: f32 = 0.5;
const PAWN_HEIGHT: f32 = 1.5;
const COLOR_PAWN: vec3<f32> = vec3(0.2, 0.35, 0.65);


// ═══════════════════════════════════════════════════════════════════════════════
// §2 STRUCTURES — Must match state.hpp exactly (UNCHANGED)
// ═══════════════════════════════════════════════════════════════════════════════

struct FrameSignal {
    t_seconds: f32,
    dt: f32,
    aspect_ratio: f32,
    _pad0: f32,
    move_x: f32,
    move_z: f32,
    look_az_delta: f32,
    look_el_delta: f32,
    zoom_delta: f32,
    pan_x_delta: f32,
    pan_y_delta: f32,
    _pad1: f32,
}

struct DesignConfig {
    mute_dynamics: u32,
    mute_couplings: u32,
    pawn_speed: f32,
    _pad0: f32,
}

struct PawnState {
    pos: vec3<f32>,
    heading: f32,
    orientation: vec4<f32>,
}

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

struct PortableDoor {
    position: vec3<f32>,
    _pad0: f32,
    forward: vec3<f32>,
    _pad1: f32,
    up: vec3<f32>,
    _pad2: f32,
    
    width: f32,
    height: f32,
    thickness: f32,
    commitment_depth: f32,
    
    shape_type: u32,
    target_id: u32,
    _pad3: u32,
    _pad4: u32,
    
    color: vec3<f32>,
    glow: f32,
}

struct DoorHeader {
    count: u32,
    _pad0: u32,
    _pad1: u32,
    _pad2: u32,
}

struct RoomParams {
    room_half_width: f32,
    room_height: f32,
    room_half_depth: f32,
    field_world_size: f32,
}

fn coupling_active(bit: u32) -> bool {
    return (config.mute_couplings & bit) == 0u;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 DOOR SDF — Shape functions for compute pass
// ═══════════════════════════════════════════════════════════════════════════════
//
// FIX NOTES (Phantom Roof):
//   The arch and pointed_arch SDFs had a geometric discontinuity at the seam
//   between the rectangular base and the curved top. The rectangle's top bound
//   (p.y - arch_y) created a surface at y=arch_y, but just above it the curve
//   says "inside". This manifested as a faint horizontal plane.
//
//   Solution: Remove the top bound from the rectangle. The if/else switch already
//   handles which formula applies based on p.y. The rectangle extends infinitely
//   upward in theory, but the curve takes over before that matters.
//

const DOOR_SHAPE_RECTANGLE: u32 = 0u;
const DOOR_SHAPE_ARCH: u32 = 1u;
const DOOR_SHAPE_CIRCLE: u32 = 2u;
const DOOR_SHAPE_POINTED_ARCH: u32 = 3u;
const MAX_DOORS: u32 = 8u;

fn door_right(door: PortableDoor) -> vec3<f32> {
    return normalize(cross(door.up, door.forward));
}

fn door_world_to_local(p: vec3<f32>, door: PortableDoor) -> vec3<f32> {
    let d = p - door.position;
    let right = door_right(door);
    return vec3(dot(d, right), dot(d, door.up), dot(d, door.forward));
}

fn door_sdf_2d_rectangle(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let center = vec2(0.0, h * 0.5);
    let size = vec2(half_w, h * 0.5);
    let d = abs(p - center) - size;
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
}

fn door_sdf_2d_arch(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let arch_y = h - half_w;
    if (p.y > arch_y) {
        // Semicircle at top
        return length(p - vec2(0.0, arch_y)) - half_w;
    } else {
        // Rectangle from y=0 upward
        // FIX: Only bound at bottom (y >= 0). The top is handled by the semicircle branch.
        //      Removing the top bound eliminates the phantom surface at the seam.
        let dx = abs(p.x) - half_w;
        let dy = -p.y;  // Distance below y=0 (bottom bound only)
        let d = vec2(dx, dy);
        return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
    }
}

fn door_sdf_2d_circle(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let radius = min(half_w, h * 0.5);
    return length(p - vec2(0.0, radius)) - radius;
}

fn door_sdf_2d_pointed_arch(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let arch_y = h * 0.6;
    if (p.y > arch_y) {
        let radius = half_w * 1.5;
        let d_left = length(p - vec2(-half_w * 0.5, arch_y)) - radius;
        let d_right = length(p - vec2(half_w * 0.5, arch_y)) - radius;
        return max(d_left, d_right);
    } else {
        // Rectangle from y=0 upward
        // FIX: Only bound at bottom (y >= 0). The top is handled by the pointed arch branch.
        //      Same fix as door_sdf_2d_arch.
        let dx = abs(p.x) - half_w;
        let dy = -p.y;  // Distance below y=0 (bottom bound only)
        let d = vec2(dx, dy);
        return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
    }
}

fn door_sdf_2d(p: vec2<f32>, half_w: f32, h: f32, shape_type: u32) -> f32 {
    switch shape_type {
        case DOOR_SHAPE_RECTANGLE: { return door_sdf_2d_rectangle(p, half_w, h); }
        case DOOR_SHAPE_ARCH: { return door_sdf_2d_arch(p, half_w, h); }
        case DOOR_SHAPE_CIRCLE: { return door_sdf_2d_circle(p, half_w, h); }
        case DOOR_SHAPE_POINTED_ARCH: { return door_sdf_2d_pointed_arch(p, half_w, h); }
        default: { return door_sdf_2d_rectangle(p, half_w, h); }
    }
}

fn door_sdf_hole(p: vec3<f32>, door: PortableDoor) -> f32 {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    let d_2d = door_sdf_2d(local.xy, half_w, door.height, door.shape_type);
    let d_z = abs(local.z) - door.thickness * 0.5;
    return max(d_2d, d_z);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 ROOM FIELD COMPUTE — Precompute unified room SDF (REFACTORED)
// ═══════════════════════════════════════════════════════════════════════════════
//
// KEY INSIGHT: We store the room INTERIOR SDF, not wall shell.
// - Negative values = inside the room (walkable space)
// - Positive values = inside walls (solid material)
// - Zero = wall surface
//
// The fragment shader reconstructs 3D by combining:
//   - Cached 2D interior (XZ plane)
//   - Floor plane (y = 0)
//   - Ceiling plane (y = height)
//   - Per-pixel door carving

fn texel_to_world_xz(texel: vec2<u32>) -> vec2<f32> {
    let uv = (vec2<f32>(texel) + 0.5) / FIELD_SIZE;
    let world_size = room_params.field_world_size;
    return (uv - 0.5) * world_size;
}

// 2D signed distance to room interior (negative inside)
fn sdf_room_interior_2d(p_xz: vec2<f32>) -> f32 {
    let half_size = vec2(room_params.room_half_width, room_params.room_half_depth);
    let d = abs(p_xz) - half_size;
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
}

// Compute which wall is nearest (for material hints)
// Returns: 1.0 = X walls (left/right), 2.0 = Z walls (front/back)
fn nearest_wall_axis(p_xz: vec2<f32>) -> f32 {
    let half_size = vec2(room_params.room_half_width, room_params.room_half_depth);
    let dist_x = half_size.x - abs(p_xz.x);
    let dist_z = half_size.y - abs(p_xz.y);
    return select(2.0, 1.0, dist_x < dist_z);
}

fn compose_update_room_field(texel: vec2<u32>) {
    let p_xz = texel_to_world_xz(texel);
    
    // Room interior SDF (negative inside room, positive in walls)
    let interior = sdf_room_interior_2d(p_xz);
    
    // Wall axis hint (for potential future use - corner detection, etc.)
    let wall_hint = nearest_wall_axis(p_xz);
    
    // Store in texture
    // .r = interior SDF
    // .g = wall axis hint
    // .b = reserved
    // .a = reserved
    textureStore(room_field_write, texel, vec4(interior, wall_hint, 0.0, 0.0));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 ENTITIES — Pawn and Camera (UNCHANGED)
// ═══════════════════════════════════════════════════════════════════════════════

const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_ELEVATION: f32 = 0.1;
const CAMERA_MAX_ELEVATION: f32 = 1.4;
const CAMERA_MIN_DISTANCE: f32 = 3.0;
const CAMERA_MAX_DISTANCE: f32 = 25.0;

fn heading_to_orientation(heading: f32) -> vec4<f32> {
    let half_angle = heading * 0.5;
    return vec4(0.0, sin(half_angle), 0.0, cos(half_angle));
}

fn coupling_velocity_to_heading(vel: vec2<f32>, current: f32, dt: f32) -> f32 {
    let speed = length(vel);
    if (speed < 0.01) { return current; }
    
    let target_heading = atan2(vel.x, -vel.y);
    let diff = target_heading - current;
    let wrapped = atan2(sin(diff), cos(diff));
    
    let rate = 8.0;
    return current + wrapped * min(rate * dt, 1.0);
}

fn coupling_input_to_camera_orbit(az_delta: f32, el_delta: f32, cam: CameraState) -> CameraState {
    var c = cam;
    c.azimuth += az_delta;
    c.elevation = clamp(c.elevation + el_delta, CAMERA_MIN_ELEVATION, CAMERA_MAX_ELEVATION);
    return c;
}

fn coupling_input_to_camera_pan(delta: vec2<f32>, cam: CameraState) -> CameraState {
    var c = cam;
    c.pan_x += delta.x * cam.distance * 0.3;
    c.pan_y += delta.y * cam.distance * 0.3;
    return c;
}

fn coupling_input_to_camera_zoom(delta: f32, cam: CameraState) -> CameraState {
    var c = cam;
    c.distance = clamp(c.distance + delta, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
    return c;
}

fn coupling_pawn_to_camera_target(pawn_pos: vec3<f32>, cam: CameraState) -> vec3<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    let look_target = pawn_pos + vec3(0.0, PAWN_HEIGHT * 0.7, 0.0);
    let look_at = look_target + right * cam.pan_x + up * cam.pan_y;
    let offset = cam.distance * vec3(cos_el * sin_az, sin_el, cos_el * cos_az);
    
    var cam_pos = look_at + offset;
    
    // Clamp camera to room bounds (with margin for wall thickness)
    let margin = 1.0;
    cam_pos.x = clamp(cam_pos.x, -ROOM_HALF_WIDTH + margin, ROOM_HALF_WIDTH - margin);
    cam_pos.z = clamp(cam_pos.z, -ROOM_HALF_DEPTH + margin, ROOM_HALF_DEPTH - margin);
    cam_pos.y = clamp(cam_pos.y, 0.5, ROOM_HEIGHT - 0.5);
    
    return cam_pos;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 COUPLINGS — World state update (UNCHANGED)
// ═══════════════════════════════════════════════════════════════════════════════

fn compose_update_world(dt: f32) {
    var pawn = pawn_state;
    
    if (coupling_active(COUPLING_INPUT_MOVES_PAWN)) {
        let cam = camera_state;
        let cos_az = cos(cam.azimuth);
        let sin_az = sin(cam.azimuth);
        
        let world_vx = signal.move_x * cos_az + signal.move_z * sin_az;
        let world_vz = -signal.move_x * sin_az + signal.move_z * cos_az;
        let world_vel = vec2(world_vx, world_vz);
        
        var new_x = pawn.pos.x + world_vx * config.pawn_speed * dt;
        var new_z = pawn.pos.z + world_vz * config.pawn_speed * dt;
        
        // Check if in any door volume
        let check_y = PAWN_HEIGHT * 0.5;
        let check_pos = vec3(new_x, check_y, new_z);
        var in_door = false;
        
        for (var i: u32 = 0u; i < door_header.count; i++) {
            let local = door_world_to_local(check_pos, doors[i]);
            let half_w = doors[i].width * 0.5;
            if (door_sdf_2d(local.xy, half_w, doors[i].height, doors[i].shape_type) < 0.0 &&
                abs(local.z) < doors[i].thickness * 0.5) {
                in_door = true;
                break;
            }
        }
        
        if (!in_door) {
            let margin = PAWN_RADIUS + 0.1;
            new_x = clamp(new_x, -ROOM_HALF_WIDTH + margin, ROOM_HALF_WIDTH - margin);
            new_z = clamp(new_z, -ROOM_HALF_DEPTH + margin, ROOM_HALF_DEPTH - margin);
        }
        
        pawn.pos.x = new_x;
        pawn.pos.z = new_z;
        pawn.pos.y = 0.0;
        
        pawn.heading = coupling_velocity_to_heading(world_vel, pawn.heading, dt);
        pawn.orientation = heading_to_orientation(pawn.heading);
    }
    
    pawn_state = pawn;
    
    var camera = camera_state;
    
    if (coupling_active(COUPLING_INPUT_ORBITS_CAMERA)) {
        camera = coupling_input_to_camera_orbit(signal.look_az_delta, signal.look_el_delta, camera);
        camera = coupling_input_to_camera_pan(vec2(signal.pan_x_delta, signal.pan_y_delta), camera);
    }
    
    if (coupling_active(COUPLING_INPUT_ZOOMS_CAMERA)) {
        camera = coupling_input_to_camera_zoom(signal.zoom_delta, camera);
    }
    
    if (coupling_active(COUPLING_PAWN_TO_CAMERA_TARGET)) {
        camera.pos = coupling_pawn_to_camera_target(pawn_state.pos, camera);
    }
    
    camera_state = camera;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 RENDER — Unified room SDF with proper corners (REFACTORED)
// ═══════════════════════════════════════════════════════════════════════════════
//
// FIX NOTES (Light Leaks):
//   The original code used boolean logic to conditionally render floor/ceiling:
//       if (in_room_xz || in_door_passage) { ... floor ... }
//   
//   This is incorrect for SDFs. At the exact boundary (interior_2d ≈ 0), sub-pixel
//   sampling differences cause the condition to flicker, creating gaps where the
//   void shows through.
//
//   Solution: Always evaluate floor and ceiling SDFs. The min() operation will
//   naturally select the correct surface. The wall SDF already clips properly
//   via its below_floor and above_ceiling terms.
//

const MAX_STEPS: i32 = 128;
const MAX_DIST: f32 = 60.0;
const SURF_DIST: f32 = 0.002;

// Material IDs
const MAT_NONE: u32 = 0u;
const MAT_FLOOR: u32 = 1u;
const MAT_WALL: u32 = 2u;
const MAT_CEILING: u32 = 3u;
const MAT_PAWN: u32 = 4u;
const MAT_DOOR_VOID: u32 = 5u;

// Colors
const COLOR_WALL: vec3<f32> = vec3(0.72, 0.90, 0);
const COLOR_CEILING: vec3<f32> = vec3(0.95, 0.75, 0.85);
const COLOR_DOOR_VOID: vec3<f32> = vec3(0.12, 0.32, 0.23);
const COLOR_FOG: vec3<f32> = vec3(0.7, 0.70, 0.92);

// Lighting
const LIGHT_DIR: vec3<f32> = vec3(0.3, 0.9, 0.2);
const AMBIENT: f32 = 0.45;
const DIFFUSE: f32 = 0.55;
const FOG_DENSITY: f32 = 0.02;

struct RoomHit {
    dist: f32,
    material: u32,
}

struct SceneHit {
    dist: f32,
    material: u32,
}

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
    material: u32,
    door_index: u32,
}

// ─── Texture Sampling ───────────────────────────────────────────────────────

fn world_to_uv(p_xz: vec2<f32>) -> vec2<f32> {
    let world_size = render_room_params.field_world_size;
    return (p_xz / world_size) + 0.5;
}

fn sample_room_field(p_xz: vec2<f32>) -> vec2<f32> {
    let uv = world_to_uv(p_xz);
    let sample = textureSampleLevel(room_field_read, bilinear_sampler, uv, 0.0);
    return sample.rg;  // .r = interior SDF, .g = wall hint
}

// ─── Unified Room SDF ───────────────────────────────────────────────────────
//
// Returns distance to nearest room surface and which surface it is.
// Uses the same SDF semantics as original but with unified material assignment.

fn sdf_room_shell(p: vec3<f32>) -> RoomHit {
    var result: RoomHit;
    result.dist = MAX_DIST;
    result.material = MAT_NONE;
    
    // Sample cached 2D interior SDF (negative inside room, positive outside)
    let field = sample_room_field(p.xz);
    let interior_2d = field.r;
    
    // FIX: Floor and ceiling are now evaluated UNCONDITIONALLY.
    // The min() chain naturally selects the correct surface.
    // This eliminates the light leak at room boundaries caused by boolean logic.
    
    // Floor — always evaluate
    let floor_d = p.y;
    if (floor_d < result.dist) {
        result.dist = floor_d;
        result.material = MAT_FLOOR;
    }
    
    // Ceiling — always evaluate
    let ceil_d = render_room_params.room_height - p.y;
    if (ceil_d < result.dist) {
        result.dist = ceil_d;
        result.material = MAT_CEILING;
    }
    
    // Walls — use the cached 2D field + vertical bounds
    // The wall SDF clips properly via below_floor and above_ceiling terms,
    // so it won't incorrectly override floor/ceiling materials.
    let wall_xz = -interior_2d;  // Positive inside room, negative in wall solid
    let below_floor = -p.y;
    let above_ceiling = p.y - render_room_params.room_height;
    var wall_d = max(wall_xz, max(below_floor, above_ceiling));
    
    // Door hole carving (inline for walls only)
    if (wall_d < 2.0 - 0.01) {
        for (var i: u32 = 0u; i < render_door_header.count; i++) {
            let door_d = door_sdf_hole(p, render_doors[i]);
            wall_d = max(wall_d, -door_d);
        }
    }
    
    if (wall_d < result.dist) {
        result.dist = wall_d;
        result.material = MAT_WALL;
    }
    
    return result;
}

// ─── Complete Scene SDF ─────────────────────────────────────────────────────

fn sdf_round_cone(p: vec3<f32>, r1: f32, r2: f32, h: f32) -> f32 {
    let q = vec2(length(p.xz), p.y);
    let b = (r1 - r2) / h;
    let a = sqrt(1.0 - b * b);
    let k = dot(q, vec2(-b, a));
    
    if (k < 0.0) { return length(q) - r1; }
    if (k > a * h) { return length(q - vec2(0.0, h)) - r2; }
    return dot(q, vec2(a, b)) - r1;
}

fn quat_inverse(q: vec4<f32>) -> vec4<f32> {
    return vec4(-q.xyz, q.w);
}

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let t = 2.0 * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}

fn sdf_pawn(p: vec3<f32>) -> f32 {
    let local = quat_rotate(quat_inverse(render_pawn.orientation), p - render_pawn.pos);
    return sdf_round_cone(local, PAWN_RADIUS, 0.0, PAWN_HEIGHT);
}

fn sdf_scene(p: vec3<f32>) -> SceneHit {
    var result: SceneHit;
    result.dist = MAX_DIST;
    result.material = MAT_NONE;
    
    // Unified room shell (floor + walls + ceiling, doors already carved)
    let room = sdf_room_shell(p);
    
    if (room.dist < result.dist) {
        result.dist = room.dist;
        result.material = room.material;
    }
    
    // Pawn
    let pawn_d = sdf_pawn(p);
    if (pawn_d < result.dist) {
        result.dist = pawn_d;
        result.material = MAT_PAWN;
    }
    
    return result;
}

// ─── Door Ray Intersection (for void rendering) ─────────────────────────────

fn door_ray_intersect(ro: vec3<f32>, rd: vec3<f32>, door: PortableDoor) -> f32 {
    let denom = dot(rd, door.forward);
    if (abs(denom) < 0.001) { return -1.0; }
    
    // Move void plane to the BACK of the carved passage (thickness/2 behind door plane)
    // This gives visual depth and prevents the "ghost plane" (back wall of carved hole)
    let void_depth = door.thickness * 0.5 - 0.1;  // Slightly inside to ensure void renders first
    let void_plane_pos = door.position + door.forward * void_depth;
    
    let t = dot(void_plane_pos - ro, door.forward) / denom;
    if (t < 0.0) { return -1.0; }
    
    let hit_p = ro + rd * t;
    let local = door_world_to_local(hit_p, door);
    let half_w = door.width * 0.5;
    
    if (door_sdf_2d(local.xy, half_w, door.height, door.shape_type) < 0.0) {
        return t;
    }
    return -1.0;
}

// ─── Raymarching ────────────────────────────────────────────────────────────

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    result.door_index = 0u;
    
    // Pre-compute door void intersections FIRST
    var door_t = MAX_DIST;
    var door_idx: u32 = 0u;
    for (var i: u32 = 0u; i < render_door_header.count; i++) {
        let t = door_ray_intersect(ro, rd, render_doors[i]);
        if (t > 0.0 && t < door_t) {
            door_t = t;
            door_idx = i;
        }
    }
    
    var t: f32 = 0.0;
    for (var i: i32 = 0; i < MAX_STEPS; i++) {
        let p = ro + rd * t;
        
        // If we've passed the door void plane, return door void
        if (t > door_t) {
            result.hit = true;
            result.pos = ro + rd * door_t;
            result.dist = door_t;
            result.material = MAT_DOOR_VOID;
            result.door_index = door_idx;
            return result;
        }
        
        let scene = sdf_scene(p);
        
        if (scene.dist < SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = t;
            result.material = scene.material;
            return result;
        }
        
        t += scene.dist * 0.9;
        if (t > MAX_DIST) { break; }
    }
    
    // Fallback: return door void if we have one
    if (door_t < MAX_DIST) {
        result.hit = true;
        result.pos = ro + rd * door_t;
        result.dist = door_t;
        result.material = MAT_DOOR_VOID;
        result.door_index = door_idx;
    }
    
    return result;
}

// ─── Normal Calculation ─────────────────────────────────────────────────────

fn calc_normal(p: vec3<f32>) -> vec3<f32> {
    let e = 0.002;
    let dx = sdf_scene(p + vec3(e, 0.0, 0.0)).dist - sdf_scene(p - vec3(e, 0.0, 0.0)).dist;
    let dy = sdf_scene(p + vec3(0.0, e, 0.0)).dist - sdf_scene(p - vec3(0.0, e, 0.0)).dist;
    let dz = sdf_scene(p + vec3(0.0, 0.0, e)).dist - sdf_scene(p - vec3(0.0, 0.0, e)).dist;
    return normalize(vec3(dx, dy, dz));
}

// ─── Shading ────────────────────────────────────────────────────────────────

fn floor_checkerboard(p: vec3<f32>) -> vec3<f32> {
    let tx = floor(p.x / FLOOR_TILE_SIZE);
    let tz = floor(p.z / FLOOR_TILE_SIZE);
    let checker = i32(tx + tz) & 1;
    return select(FLOOR_COLOR_LIGHT, FLOOR_COLOR_DARK, checker == 1);
}

fn get_ray_direction(uv: vec2<f32>) -> vec3<f32> {
    let cam = render_camera;
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    let orbital = vec3(cos_el * sin_az, sin_el, cos_el * cos_az);
    let look_dir = -orbital;
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(orbital, right);
    
    let fov = tan(CAMERA_FOV * 0.5);
    return normalize(look_dir + right * uv.x * fov * render_signal.aspect_ratio + up * uv.y * fov);
}

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) { return COLOR_FOG; }

    if (hit.material == MAT_DOOR_VOID) {
        let door_color = render_doors[hit.door_index].color;
        
        // Dark void at bottom, fade to fog above
        let void_height = 1.5;
        let fade_height = 1.0;
        
        if (hit.pos.y < void_height) {
            let base_color = door_color * 0.12;
            let deep_color = door_color * 0.04;
            let depth = min(hit.dist * 0.05, 1.0);
            return mix(base_color, deep_color, depth);
        } else if (hit.pos.y < void_height + fade_height) {
            let t = (hit.pos.y - void_height) / fade_height;
            let void_color = door_color * 0.12;
            return mix(void_color, COLOR_FOG, t);
        } else {
            return COLOR_FOG;
        }
    }
    
    let n = calc_normal(hit.pos);
    let light = normalize(LIGHT_DIR);
    let ndotl = max(dot(n, light), 0.0);
    
    var base: vec3<f32>;
    switch hit.material {
        case MAT_FLOOR: {
            base = floor_checkerboard(hit.pos);
        }
        case MAT_WALL: {
            base = COLOR_WALL;
        }
        case MAT_CEILING: {
            base = COLOR_CEILING;
        }
        case MAT_PAWN: {
            base = COLOR_PAWN;
        }
        default: {
            base = COLOR_FOG;
        }
    }
    
    let lit = base * (AMBIENT + DIFFUSE * ndotl);
    let fog = 1.0 - exp(-hit.dist * FOG_DENSITY);
    return mix(lit, COLOR_FOG, fog);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §8 BINDINGS — GPU memory (UNCHANGED)
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Compute (group 0: buffers) ───────────────────────────────────────────────
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read>       config: DesignConfig;
@group(0) @binding(2) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(3) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(4) var<storage, read>       door_header: DoorHeader;
@group(0) @binding(5) var<storage, read>       doors: array<PortableDoor, 8>;
@group(0) @binding(6) var<storage, read>       room_params: RoomParams;

// ─── Compute (group 1: textures) ──────────────────────────────────────────────
@group(1) @binding(0) var room_field_write: texture_storage_2d<rgba16float, write>;

// ─── Render (group 0: buffers) ────────────────────────────────────────────────
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_pawn: PawnState;
@group(0) @binding(12) var<storage, read> render_camera: CameraState;
@group(0) @binding(13) var<storage, read> render_door_header: DoorHeader;
@group(0) @binding(14) var<storage, read> render_doors: array<PortableDoor, 8>;
@group(0) @binding(15) var<storage, read> render_room_params: RoomParams;

// ─── Render (group 1: textures) ───────────────────────────────────────────────
@group(1) @binding(0) var room_field_read: texture_2d<f32>;
@group(1) @binding(1) var bilinear_sampler: sampler;


// ═══════════════════════════════════════════════════════════════════════════════
// §9 ENTRY POINTS (UNCHANGED)
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(1)
fn update_world() {
    compose_update_world(signal.dt);
}

@compute @workgroup_size(8, 8)
fn update_room_field(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= 128u || id.y >= 128u) {
        return;
    }
    compose_update_room_field(id.xy);
}

@vertex
fn fullscreen_vs(@builtin(vertex_index) vid: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(vid & 1u) * 4 - 1);
    let y = f32(i32((vid >> 1u) & 1u) * 4 - 1);
    return vec4(x, y, 0.0, 1.0);
}


@fragment
fn world_fs(@builtin(position) frag_coord: vec4<f32>) -> @location(0) vec4<f32> {
    let res = vec2(render_signal.aspect_ratio * 720.0, 720.0);
    let uv = (frag_coord.xy / res) * 2.0 - 1.0;
    let corrected = vec2(uv.x, -uv.y);
    
    let rd = get_ray_direction(corrected);
    let hit = raymarch(render_camera.pos, rd);
    let color = shade(hit, rd);
    
    return vec4(color, 1.0);
}
