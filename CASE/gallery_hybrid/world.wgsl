// ═══════════════════════════════════════════════════════════════════════════════
// GALLERY HYBRID — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Simple rasterized room rendering.
// Room geometry generated on CPU, transformed and shaded on GPU.
//
// PASSES:
//   1. Compute: Update camera from input
//   2. Render: Rasterize room mesh
//   3. Render: Rasterize pawn mesh (separate draw call with transform)
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 TYPES
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

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

struct PawnState {
    pos: vec3<f32>,
    heading: f32,
    orientation: vec4<f32>,
}

// Vertex input from mesh
struct VertexIn {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
    @location(3) material: u32,
}

struct VertexOut {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) world_pos: vec3<f32>,
    @location(1) normal: vec3<f32>,
    @location(2) uv: vec2<f32>,
    @location(3) @interpolate(flat) material: u32,
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

const PI: f32 = 3.14159265359;

// Camera
const CAMERA_FOV: f32 = 1.0;
const CAMERA_NEAR: f32 = 0.1;
const CAMERA_FAR: f32 = 100.0;
const CAMERA_MIN_ELEVATION: f32 = 0.1;
const CAMERA_MAX_ELEVATION: f32 = 1.2;
const CAMERA_MIN_DISTANCE: f32 = 3.0;
const CAMERA_MAX_DISTANCE: f32 = 25.0;

// Pawn
const PAWN_HEIGHT: f32 = 1.5;
const PAWN_SPEED: f32 = 8.0;
const PAWN_TURN_SPEED: f32 = 8.0;

// Materials
const MAT_FLOOR: u32 = 0u;
const MAT_CEILING: u32 = 1u;
const MAT_WALL: u32 = 2u;
const MAT_DOOR_VOID: u32 = 3u;
const MAT_PAWN: u32 = 4u;

// Colors
const COLOR_FLOOR_LIGHT: vec3<f32> = vec3(0.75, 0.75, 0.75);
const COLOR_FLOOR_DARK: vec3<f32> = vec3(0.65, 0.65, 0.65);
const COLOR_CEILING: vec3<f32> = vec3(0.95, 0.95, 0.95);
const COLOR_WALL: vec3<f32> = vec3(0.92, 0.92, 0.90);
const COLOR_DOOR_VOID: vec3<f32> = vec3(0.02, 0.02, 0.03);
const COLOR_PAWN: vec3<f32> = vec3(0.6, 0.55, 0.7);
const COLOR_FOG: vec3<f32> = vec3(0.88, 0.90, 0.92);

// Lighting
const LIGHT_DIR: vec3<f32> = vec3(0.3, 0.9, 0.2);
const AMBIENT: f32 = 0.45;
const DIFFUSE: f32 = 0.55;
const FOG_DENSITY: f32 = 0.02;

// Floor tile
const TILE_SIZE: f32 = 2.0;


// ═══════════════════════════════════════════════════════════════════════════════
// §3 BINDINGS
// ═══════════════════════════════════════════════════════════════════════════════

// Compute bindings
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(2) var<storage, read_write> pawn_state: PawnState;

// Render bindings
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_camera: CameraState;
@group(0) @binding(12) var<storage, read> render_pawn: PawnState;


// ═══════════════════════════════════════════════════════════════════════════════
// §4 MATH HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

fn build_view_matrix(cam: CameraState) -> mat4x4<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    // Camera basis vectors
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    // View matrix (transpose of rotation, then translate)
    return mat4x4<f32>(
        vec4(right.x, up.x, -forward.x, 0.0),
        vec4(right.y, up.y, -forward.y, 0.0),
        vec4(right.z, up.z, -forward.z, 0.0),
        vec4(-dot(right, cam.pos), -dot(up, cam.pos), dot(forward, cam.pos), 1.0)
    );
}

fn build_projection_matrix(aspect: f32) -> mat4x4<f32> {
    let f = 1.0 / tan(CAMERA_FOV * 0.5);
    let nf = 1.0 / (CAMERA_NEAR - CAMERA_FAR);
    
    return mat4x4<f32>(
        vec4(f / aspect, 0.0, 0.0, 0.0),
        vec4(0.0, f, 0.0, 0.0),
        vec4(0.0, 0.0, CAMERA_FAR * nf, -1.0),
        vec4(0.0, 0.0, CAMERA_FAR * CAMERA_NEAR * nf, 0.0)
    );
}

fn quat_from_axis_angle(axis: vec3<f32>, angle: f32) -> vec4<f32> {
    let half_angle = angle * 0.5;
    return vec4(axis * sin(half_angle), cos(half_angle));
}

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let u = q.xyz;
    let s = q.w;
    return 2.0 * dot(u, v) * u + (s * s - dot(u, u)) * v + 2.0 * s * cross(u, v);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 COMPUTE — Update camera and pawn
// ═══════════════════════════════════════════════════════════════════════════════

fn compute_camera_position(aim_point: vec3<f32>, cam: CameraState) -> vec3<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    let look_at = aim_point + right * cam.pan_x + up * cam.pan_y;
    let offset = cam.distance * vec3(cos_el * sin_az, sin_el, cos_el * cos_az);
    
    return look_at + offset;
}

@compute @workgroup_size(1)
fn update_world() {
    let dt = signal.dt;
    
    // ─── Update Pawn ──────────────────────────────────────────────────────────
    var pawn = pawn_state;
    
    let input_dir = vec2(signal.move_x, signal.move_z);
    let cos_az = cos(camera_state.azimuth);
    let sin_az = sin(camera_state.azimuth);
    let world_vel = vec2(
        input_dir.x * cos_az + input_dir.y * sin_az,
        -input_dir.x * sin_az + input_dir.y * cos_az
    );
    
    pawn.pos.x += world_vel.x * PAWN_SPEED * dt;
    pawn.pos.z += world_vel.y * PAWN_SPEED * dt;
    pawn.pos.y = 0.0;
    
    // Update heading
    let speed_sq = dot(world_vel, world_vel);
    if (speed_sq > 0.001) {
        let goal_heading = atan2(world_vel.x, world_vel.y);
        var diff = goal_heading - pawn.heading;
        if (diff > PI) { diff -= 2.0 * PI; }
        else if (diff < -PI) { diff += 2.0 * PI; }
        
        let max_turn = PAWN_TURN_SPEED * dt;
        pawn.heading += clamp(diff, -max_turn, max_turn);
        
        if (pawn.heading > PI) { pawn.heading -= 2.0 * PI; }
        else if (pawn.heading < -PI) { pawn.heading += 2.0 * PI; }
    }
    
    pawn.orientation = quat_from_axis_angle(vec3(0.0, 1.0, 0.0), pawn.heading);
    pawn_state = pawn;
    
    // ─── Update Camera ────────────────────────────────────────────────────────
    var cam = camera_state;
    
    cam.azimuth += signal.look_az_delta;
    cam.elevation = clamp(cam.elevation + signal.look_el_delta, CAMERA_MIN_ELEVATION, CAMERA_MAX_ELEVATION);
    cam.distance = clamp(cam.distance + signal.zoom_delta, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
    cam.pan_x += signal.pan_x_delta * cam.distance * 0.5;
    cam.pan_y += signal.pan_y_delta * cam.distance * 0.5;
    
    // Follow pawn
    let aim = vec3(pawn_state.pos.x, pawn_state.pos.y + PAWN_HEIGHT * 0.5, pawn_state.pos.z);
    cam.pos = compute_camera_position(aim, cam);
    
    camera_state = cam;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 VERTEX SHADER — Room
// ═══════════════════════════════════════════════════════════════════════════════

@vertex
fn room_vs(in: VertexIn) -> VertexOut {
    var out: VertexOut;
    
    let view = build_view_matrix(render_camera);
    let proj = build_projection_matrix(render_signal.aspect_ratio);
    
    let world_pos = vec4(in.position, 1.0);
    out.clip_pos = proj * view * world_pos;
    out.world_pos = in.position;
    out.normal = in.normal;
    out.uv = in.uv;
    out.material = in.material;
    
    return out;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 VERTEX SHADER — Pawn
// ═══════════════════════════════════════════════════════════════════════════════

@vertex
fn pawn_vs(in: VertexIn) -> VertexOut {
    var out: VertexOut;
    
    let view = build_view_matrix(render_camera);
    let proj = build_projection_matrix(render_signal.aspect_ratio);
    
    // Transform by pawn orientation and position
    let rotated = quat_rotate(render_pawn.orientation, in.position);
    let world_pos = rotated + render_pawn.pos;
    
    out.clip_pos = proj * view * vec4(world_pos, 1.0);
    out.world_pos = world_pos;
    out.normal = quat_rotate(render_pawn.orientation, in.normal);
    out.uv = in.uv;
    out.material = in.material;
    
    return out;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §8 FRAGMENT SHADER
// ═══════════════════════════════════════════════════════════════════════════════

fn floor_checkerboard(pos: vec3<f32>) -> vec3<f32> {
    let tx = floor(pos.x / TILE_SIZE);
    let tz = floor(pos.z / TILE_SIZE);
    let checker = i32(tx + tz) & 1;
    return select(COLOR_FLOOR_LIGHT, COLOR_FLOOR_DARK, checker == 1);
}

@fragment
fn main_fs(in: VertexOut) -> @location(0) vec4<f32> {
    var base_color: vec3<f32>;
    
    switch in.material {
        case MAT_FLOOR: {
            base_color = floor_checkerboard(in.world_pos);
        }
        case MAT_CEILING: {
            base_color = COLOR_CEILING;
        }
        case MAT_WALL: {
            base_color = COLOR_WALL;
        }
        case MAT_DOOR_VOID: {
            // Door void is unlit black
            return vec4(COLOR_DOOR_VOID, 1.0);
        }
        case MAT_PAWN: {
            base_color = COLOR_PAWN;
        }
        default: {
            base_color = vec3(1.0, 0.0, 1.0);  // Debug magenta
        }
    }
    
    // Simple diffuse lighting
    let light_dir = normalize(LIGHT_DIR);
    let ndotl = max(dot(in.normal, light_dir), 0.0);
    var color = base_color * (AMBIENT + DIFFUSE * ndotl);
    
    // Distance fog
    let dist = length(in.world_pos - render_camera.pos);
    let fog = 1.0 - exp(-dist * FOG_DENSITY);
    color = mix(color, COLOR_FOG, fog);
    
    return vec4(color, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// END OF SCROLL
// ═══════════════════════════════════════════════════════════════════════════════
