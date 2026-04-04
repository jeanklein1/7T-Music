// ═══════════════════════════════════════════════════════════════════════════════
// GALLERY CARTRIDGE — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Updated to use Portable Door system.
//
// KEY CHANGES:
//   - PortableDoor struct (96 bytes) with forward/up vectors
//   - door_world_to_local() handles arbitrary orientation
//   - Door hole carving and void rendering work for any placement
//
// SCROLL INDEX:
//   §1    CONFIG
//   §2    CHART
//   §3    PORTABLE DOOR
//   §4    ROOM GEOMETRY
//   §5    ENTITIES
//   §6    COUPLINGS
//   §7    COMPOSE
//   §8    RENDER
//   §9    BINDINGS
//   §10   ENTRY POINTS
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 CONFIG
// ═══════════════════════════════════════════════════════════════════════════════

const COUPLING_INPUT_MOVES_PAWN:        u32 = 1u << 0u;
const COUPLING_PAWN_TO_CAMERA_TARGET:   u32 = 1u << 1u;
const COUPLING_INPUT_ORBITS_CAMERA:     u32 = 1u << 2u;
const COUPLING_INPUT_ZOOMS_CAMERA:      u32 = 1u << 3u;

struct DesignConfig {
    mute_dynamics: u32,
    mute_couplings: u32,
    pawn_speed: f32,
    _pad0: f32,
}

fn coupling_active(bit: u32) -> bool {
    return (config.mute_couplings & bit) == 0u;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CHART
// ═══════════════════════════════════════════════════════════════════════════════

const ROOM_HALF_WIDTH: f32 = 15.0;
const ROOM_HEIGHT: f32 = 6.0;
const ROOM_HALF_DEPTH: f32 = 15.0;

const FLOOR_TILE_SIZE: f32 = 2.0;
const FLOOR_COLOR_LIGHT: vec3<f32> = vec3(0.75, 0.75, 0.75);
const FLOOR_COLOR_DARK: vec3<f32> = vec3(0.65, 0.65, 0.65);


// ═══════════════════════════════════════════════════════════════════════════════
// §3 PORTABLE DOOR — Self-contained door object
// ═══════════════════════════════════════════════════════════════════════════════

const DOOR_SHAPE_RECTANGLE: u32 = 0u;
const DOOR_SHAPE_ARCH: u32 = 1u;
const DOOR_SHAPE_CIRCLE: u32 = 2u;
const DOOR_SHAPE_POINTED_ARCH: u32 = 3u;

const MAX_DOORS: u32 = 8u;
const DOOR_TARGET_NONE: u32 = 0xFFFFFFFFu;

// ─── Structure (96 bytes) — Must match state.hpp ─────────────────────────────

struct PortableDoor {
    // Placement
    position: vec3<f32>,
    _pad0: f32,
    forward: vec3<f32>,     // Direction "through" door
    _pad1: f32,
    up: vec3<f32>,
    _pad2: f32,
    
    // Dimensions
    width: f32,
    height: f32,
    thickness: f32,
    commitment_depth: f32,
    
    // Identity
    shape_type: u32,
    target_id: u32,
    _pad3: u32,
    _pad4: u32,
    
    // Appearance
    color: vec3<f32>,
    glow: f32,
}

struct DoorHeader {
    count: u32,
    _pad0: u32,
    _pad1: u32,
    _pad2: u32,
}

// ─── Coordinate transform ────────────────────────────────────────────────────

fn door_right(door: PortableDoor) -> vec3<f32> {
    return normalize(cross(door.up, door.forward));
}

fn door_world_to_local(p: vec3<f32>, door: PortableDoor) -> vec3<f32> {
    let d = p - door.position;
    let right = door_right(door);
    
    return vec3(
        dot(d, right),
        dot(d, door.up),
        dot(d, door.forward)
    );
}

// ─── 2D SDF shapes ───────────────────────────────────────────────────────────

fn door_sdf_2d_rectangle(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let center = vec2(0.0, h * 0.5);
    let size = vec2(half_w, h * 0.5);
    let d = abs(p - center) - size;
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
}

fn door_sdf_2d_arch(p: vec2<f32>, half_w: f32, h: f32) -> f32 {
    let arch_y = h - half_w;
    if (p.y > arch_y) {
        return length(p - vec2(0.0, arch_y)) - half_w;
    } else {
        let d = abs(p) - vec2(half_w, arch_y);
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
        let d = abs(p) - vec2(half_w, arch_y);
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

// ─── 3D SDF for wall carving ─────────────────────────────────────────────────

fn door_sdf_hole(p: vec3<f32>, door: PortableDoor) -> f32 {
    let local = door_world_to_local(p, door);
    let half_w = door.width * 0.5;
    
    let d_2d = door_sdf_2d(local.xy, half_w, door.height, door.shape_type);
    let d_z = abs(local.z) - door.thickness * 0.5;
    
    return max(d_2d, d_z);
}

// ─── Ray intersection (for void rendering) ───────────────────────────────────

fn door_ray_intersect(ro: vec3<f32>, rd: vec3<f32>, door: PortableDoor) -> f32 {
    let denom = dot(rd, door.forward);
    if (abs(denom) < 0.001) { return -1.0; }
    
    let t = dot(door.position - ro, door.forward) / denom;
    if (t < 0.0) { return -1.0; }
    
    let hit_p = ro + rd * t;
    let local = door_world_to_local(hit_p, door);
    let half_w = door.width * 0.5;
    
    if (door_sdf_2d(local.xy, half_w, door.height, door.shape_type) < 0.0) {
        return t;
    }
    return -1.0;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 ROOM GEOMETRY
// ═══════════════════════════════════════════════════════════════════════════════

const COLOR_WALL: vec3<f32> = vec3(0.92, 0.92, 0.90);
const COLOR_CEILING: vec3<f32> = vec3(0.95, 0.95, 0.95);
const COLOR_DOOR_VOID: vec3<f32> = vec3(0.02, 0.02, 0.03);

fn sdf_box(p: vec3<f32>, b: vec3<f32>) -> f32 {
    let q = abs(p) - b;
    return length(max(q, vec3(0.0))) + min(max(q.x, max(q.y, q.z)), 0.0);
}

fn sdf_room_interior(p: vec3<f32>) -> f32 {
    let size = vec3(ROOM_HALF_WIDTH, ROOM_HEIGHT * 0.5, ROOM_HALF_DEPTH);
    let center = vec3(0.0, ROOM_HEIGHT * 0.5, 0.0);
    return sdf_box(p - center, size);
}

fn sdf_all_door_holes(p: vec3<f32>) -> f32 {
    var min_d: f32 = 1000.0;
    for (var i: u32 = 0u; i < render_door_header.count; i++) {
        let d = door_sdf_hole(p, render_doors[i]);
        min_d = min(min_d, d);
    }
    return min_d;
}

fn sdf_walls(p: vec3<f32>) -> f32 {
    let interior = sdf_room_interior(p);
    
    // Only evaluate door holes if we're near the wall surface
    // This saves ~400 door SDF evaluations per pixel when in open space
    if (interior > -3.0) {
        let door_holes = sdf_all_door_holes(p);
        return -min(interior, door_holes);
    }
    
    return -interior;
}

fn sdf_floor(p: vec3<f32>) -> f32 { return p.y; }
fn sdf_ceiling(p: vec3<f32>) -> f32 { return ROOM_HEIGHT - p.y; }

fn floor_checkerboard(p: vec3<f32>) -> vec3<f32> {
    let tx = floor(p.x / FLOOR_TILE_SIZE);
    let tz = floor(p.z / FLOOR_TILE_SIZE);
    let checker = i32(tx + tz) & 1;
    return select(FLOOR_COLOR_LIGHT, FLOOR_COLOR_DARK, checker == 1);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 ENTITIES
// ═══════════════════════════════════════════════════════════════════════════════

const PAWN_RADIUS: f32 = 0.5;
const PAWN_HEIGHT: f32 = 1.5;
const PAWN_SPEED: f32 = 8.0;
const COLOR_PAWN: vec3<f32> = vec3(0.6, 0.55, 0.7);

struct PawnState {
    pos: vec3<f32>,
    heading: f32,
    orientation: vec4<f32>,
}

const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_EL: f32 = 0.1;
const CAMERA_MAX_EL: f32 = 1.2;
const CAMERA_MIN_DIST: f32 = 3.0;
const CAMERA_MAX_DIST: f32 = 25.0;

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

fn camera_compute_position(aim: vec3<f32>, cam: CameraState) -> vec3<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    let look_at = aim + right * cam.pan_x + up * cam.pan_y;
    let offset = cam.distance * vec3(cos_el * sin_az, sin_el, cos_el * cos_az);
    
    return look_at + offset;
}

fn sdf_round_cone(p: vec3<f32>, r_base: f32, r_tip: f32, h: f32) -> f32 {
    let b = (r_base - r_tip) / h;
    let a = sqrt(1.0 - b * b);
    let q = vec2(length(p.xz), p.y);
    let k = dot(q, vec2(-b, a));
    if (k < 0.0) { return length(q) - r_base; }
    if (k > a * h) { return length(q - vec2(0.0, h)) - r_tip; }
    return dot(q, vec2(a, b)) - r_base;
}

fn quat_inverse(q: vec4<f32>) -> vec4<f32> { return vec4(-q.xyz, q.w); }

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let t = 2.0 * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 COUPLINGS
// ═══════════════════════════════════════════════════════════════════════════════

fn coupling_input_to_world_velocity(input: vec2<f32>, azimuth: f32) -> vec2<f32> {
    let cos_az = cos(azimuth);
    let sin_az = sin(azimuth);
    return vec2(
        input.x * cos_az + input.y * sin_az,
        -input.x * sin_az + input.y * cos_az
    );
}

fn coupling_velocity_to_heading(vel: vec2<f32>, current: f32, dt: f32) -> f32 {
    if (length(vel) < 0.01) { return current; }
    let goal = atan2(vel.x, vel.y);
    var diff = goal - current;
    if (diff > 3.14159) { diff -= 6.28318; }
    if (diff < -3.14159) { diff += 6.28318; }
    return current + diff * min(dt * 8.0, 1.0);
}

fn heading_to_orientation(h: f32) -> vec4<f32> {
    return vec4(0.0, sin(h * 0.5), 0.0, cos(h * 0.5));
}

fn coupling_input_to_camera_orbit(az_delta: f32, el_delta: f32, cam: CameraState) -> CameraState {
    var result = cam;
    result.azimuth += az_delta;
    result.elevation = clamp(cam.elevation + el_delta, CAMERA_MIN_EL, CAMERA_MAX_EL);
    return result;
}

fn coupling_input_to_camera_zoom(zoom_delta: f32, cam: CameraState) -> CameraState {
    var result = cam;
    result.distance = clamp(cam.distance + zoom_delta, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
    return result;
}

fn coupling_input_to_camera_pan(pan_delta: vec2<f32>, cam: CameraState) -> CameraState {
    var result = cam;
    result.pan_x += pan_delta.x * cam.distance * 0.5;
    result.pan_y += pan_delta.y * cam.distance * 0.5;
    return result;
}

fn coupling_pawn_to_camera_target(pawn_pos: vec3<f32>, cam: CameraState) -> vec3<f32> {
    let aim = vec3(pawn_pos.x, pawn_pos.y + PAWN_HEIGHT * 0.5, pawn_pos.z);
    return camera_compute_position(aim, cam);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 COMPOSE
// ═══════════════════════════════════════════════════════════════════════════════

fn compose_update_world(dt: f32) {
    // Pawn movement (GPU side mirrors CPU for rendering)
    var pawn = pawn_state;
    
    if (coupling_active(COUPLING_INPUT_MOVES_PAWN)) {
        let input_dir = vec2(signal.move_x, signal.move_z);
        let world_vel = coupling_input_to_world_velocity(input_dir, camera_state.azimuth);
        let speed = config.pawn_speed;
        
        var new_x = pawn.pos.x + world_vel.x * speed * dt;
        var new_z = pawn.pos.z + world_vel.y * speed * dt;
        
        // Check if in any door volume (simplified: just silhouette check)
        var in_door = false;
        let check_pos = vec3(new_x, PAWN_HEIGHT * 0.5, new_z);
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
    
    // Camera
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
// §8 RENDER
// ═══════════════════════════════════════════════════════════════════════════════

const MAX_STEPS: i32 = 100;
const MAX_DIST: f32 = 60.0;
const SURF_DIST: f32 = 0.005;

const MAT_NONE: u32 = 0u;
const MAT_FLOOR: u32 = 1u;
const MAT_WALL: u32 = 2u;
const MAT_CEILING: u32 = 3u;
const MAT_PAWN: u32 = 4u;
const MAT_DOOR_VOID: u32 = 5u;

struct SceneHit { dist: f32, material: u32, }

const LIGHT_DIR: vec3<f32> = vec3(0.3, 0.9, 0.2);
const AMBIENT: f32 = 0.45;
const DIFFUSE: f32 = 0.55;
const COLOR_FOG: vec3<f32> = vec3(0.88, 0.90, 0.92);
const FOG_DENSITY: f32 = 0.02;

fn sdf_pawn(p: vec3<f32>) -> f32 {
    let local = quat_rotate(quat_inverse(render_pawn.orientation), p - render_pawn.pos);
    return sdf_round_cone(local, PAWN_RADIUS, 0.0, PAWN_HEIGHT);
}

fn sdf_scene(p: vec3<f32>) -> SceneHit {
    var result: SceneHit;
    result.dist = MAX_DIST;
    result.material = MAT_NONE;
    
    let floor_d = sdf_floor(p);
    if (floor_d < result.dist) { result.dist = floor_d; result.material = MAT_FLOOR; }
    
    let ceil_d = sdf_ceiling(p);
    if (ceil_d < result.dist) { result.dist = ceil_d; result.material = MAT_CEILING; }
    
    let wall_d = sdf_walls(p);
    if (wall_d < result.dist) { result.dist = wall_d; result.material = MAT_WALL; }
    
    let pawn_d = sdf_pawn(p);
    if (pawn_d < result.dist) { result.dist = pawn_d; result.material = MAT_PAWN; }
    
    return result;
}

struct RayHit { hit: bool, pos: vec3<f32>, dist: f32, material: u32, }

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    
    var t: f32 = 0.0;
    for (var i: i32 = 0; i < MAX_STEPS; i++) {
        let p = ro + rd * t;
        let scene = sdf_scene(p);
        
        if (scene.dist < SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = t;
            result.material = scene.material;
            return result;
        }
        
        t += scene.dist;
        if (t > MAX_DIST) { break; }
    }
    
    // Check door voids
    var closest_t = MAX_DIST;
    for (var i: u32 = 0u; i < render_door_header.count; i++) {
        let door_t = door_ray_intersect(ro, rd, render_doors[i]);
        if (door_t > 0.0 && door_t < closest_t) {
            closest_t = door_t;
        }
    }
    
    if (closest_t < MAX_DIST) {
        result.hit = true;
        result.pos = ro + rd * closest_t;
        result.dist = closest_t;
        result.material = MAT_DOOR_VOID;
        return result;
    }
    
    result.dist = t;
    return result;
}

fn calc_normal(p: vec3<f32>) -> vec3<f32> {
    let e = 0.001;
    let d = sdf_scene(p).dist;
    return normalize(vec3(
        sdf_scene(p + vec3(e, 0.0, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, e, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, 0.0, e)).dist - d
    ));
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
        let depth = min(hit.dist * 0.05, 1.0);
        return mix(COLOR_DOOR_VOID, COLOR_DOOR_VOID * 0.5, depth);
    }
    
    let n = calc_normal(hit.pos);
    let light = normalize(LIGHT_DIR);
    let ndotl = max(dot(n, light), 0.0);
    
    var base: vec3<f32>;
    if (hit.material == MAT_FLOOR) {
        base = floor_checkerboard(hit.pos);
    } else if (hit.material == MAT_WALL) {
        base = COLOR_WALL;
    } else if (hit.material == MAT_CEILING) {
        base = COLOR_CEILING;
    } else if (hit.material == MAT_PAWN) {
        base = COLOR_PAWN;
    } else {
        base = COLOR_FOG;
    }
    
    let lit = base * (AMBIENT + DIFFUSE * ndotl);
    let fog = 1.0 - exp(-hit.dist * FOG_DENSITY);
    return mix(lit, COLOR_FOG, fog);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §9 BINDINGS
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

// Compute
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read>       config: DesignConfig;
@group(0) @binding(2) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(3) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(4) var<storage, read>       door_header: DoorHeader;
@group(0) @binding(5) var<storage, read>       doors: array<PortableDoor, 8>;

// Render
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_pawn: PawnState;
@group(0) @binding(12) var<storage, read> render_camera: CameraState;
@group(0) @binding(13) var<storage, read> render_door_header: DoorHeader;
@group(0) @binding(14) var<storage, read> render_doors: array<PortableDoor, 8>;


// ═══════════════════════════════════════════════════════════════════════════════
// §10 ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(1)
fn update_world() {
    compose_update_world(signal.dt);
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
