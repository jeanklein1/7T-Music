// ═══════════════════════════════════════════════════════════════════════════════
// PAWN RASTERIZE — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Rasterized pawn viewer. Mesh comes from C++, we just transform and shade.
// Hot reload enabled — edit this file, see changes instantly.
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ Chart:     Void space                                                       │
// │ Subject:   Pawn mesh (vertices from C++)                                    │
// │ Camera:    Full spherical orbit                                             │
// │ Lighting:  Studio three-point                                               │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// WHAT YOU CAN TWEAK HERE (hot reload):
//   - Colors (SUBJECT_COLOR, VOID_COLOR)
//   - Lighting (directions, intensities)
//   - Effects (rim, fresnel, etc.)
//
// WHAT LIVES IN C++ (requires rebuild):
//   - Mesh geometry (PawnProfile in state.hpp)
//
// SCROLL INDEX:
//   §1    STRUCTURES ..................... line 40
//   §2    CAMERA ......................... line 70
//   §3    LIGHTING ....................... line 140
//   §4    BINDINGS ....................... line 180
//   §5    ENTRY POINTS ................... line 200
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 STRUCTURES — Must match state.hpp exactly
// ═══════════════════════════════════════════════════════════════════════════════

struct FrameSignal {
    t_seconds: f32,
    dt: f32,
    aspect_ratio: f32,
    _pad0: f32,
    look_az_delta: f32,
    look_el_delta: f32,
    zoom_delta: f32,
    pan_x_delta: f32,
    pan_y_delta: f32,
    _pad1: f32,
    _pad2: f32,
    _pad3: f32,
}

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
}

struct VertexOutput {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) world_pos: vec3<f32>,
    @location(1) world_normal: vec3<f32>,
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CAMERA — Orbital camera with matrices
// ═══════════════════════════════════════════════════════════════════════════════

const CAMERA_FOV: f32 = 1.0;
const CAMERA_NEAR: f32 = 0.1;
const CAMERA_FAR: f32 = 100.0;
const CAMERA_MIN_ELEVATION: f32 = -1.48;
const CAMERA_MAX_ELEVATION: f32 = 1.48;
const CAMERA_MIN_DISTANCE: f32 = 1.5;
const CAMERA_MAX_DISTANCE: f32 = 20.0;

// Pawn center height (should match C++ PawnProfile::total_height / 2)
const AIM_POINT: vec3<f32> = vec3(0.0, 0.9, 0.0);

fn camera_compute_position(aim_point: vec3<f32>, cam: CameraState) -> vec3<f32> {
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

fn camera_view_matrix(cam: CameraState, aim_point: vec3<f32>) -> mat4x4<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    let look_at = aim_point + right * cam.pan_x + up * cam.pan_y;
    
    let tx = -dot(right, cam.pos);
    let ty = -dot(up, cam.pos);
    let tz = -dot(-forward, cam.pos);
    
    return mat4x4<f32>(
        vec4(right.x, up.x, -forward.x, 0.0),
        vec4(right.y, up.y, -forward.y, 0.0),
        vec4(right.z, up.z, -forward.z, 0.0),
        vec4(tx, ty, tz, 1.0)
    );
}

fn camera_projection_matrix(aspect: f32) -> mat4x4<f32> {
    let f = 1.0 / tan(CAMERA_FOV * 0.5);
    let nf = 1.0 / (CAMERA_NEAR - CAMERA_FAR);
    
    return mat4x4<f32>(
        vec4(f / aspect, 0.0, 0.0, 0.0),
        vec4(0.0, f, 0.0, 0.0),
        vec4(0.0, 0.0, CAMERA_FAR * nf, -1.0),
        vec4(0.0, 0.0, CAMERA_FAR * CAMERA_NEAR * nf, 0.0)
    );
}

fn compose_update_camera() {
    var cam = camera_state;
    
    cam.azimuth += signal.look_az_delta;
    cam.elevation = clamp(
        cam.elevation + signal.look_el_delta,
        CAMERA_MIN_ELEVATION,
        CAMERA_MAX_ELEVATION
    );
    cam.distance = clamp(
        cam.distance + signal.zoom_delta,
        CAMERA_MIN_DISTANCE,
        CAMERA_MAX_DISTANCE
    );
    cam.pan_x += signal.pan_x_delta * cam.distance * 0.5;
    cam.pan_y += signal.pan_y_delta * cam.distance * 0.5;
    
    cam.pos = camera_compute_position(AIM_POINT, cam);
    
    camera_state = cam;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 LIGHTING — Tweak these for instant visual changes
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Colors (edit these!) ─────────────────────────────────────────────────────

const SUBJECT_COLOR: vec3<f32> = vec3(0.92, 0.89, 0.85);  // Warm ivory
const VOID_COLOR: vec3<f32> = vec3(0.02, 0.02, 0.03);

// ─── Light directions ─────────────────────────────────────────────────────────

const LIGHT_KEY: vec3<f32> = vec3(0.6, 0.8, 0.5);    // Main light (front-right-above)
const LIGHT_FILL: vec3<f32> = vec3(-0.4, 0.3, 0.6);  // Fill (left)
const LIGHT_RIM: vec3<f32> = vec3(0.0, 0.2, -0.8);   // Rim (behind)

// ─── Intensities ──────────────────────────────────────────────────────────────

const AMBIENT: f32 = 0.15;
const KEY_STRENGTH: f32 = 0.65;
const FILL_STRENGTH: f32 = 0.25;
const RIM_STRENGTH: f32 = 0.35;
const SPECULAR_POWER: f32 = 32.0;
const SPECULAR_STRENGTH: f32 = 0.3;

fn shade_surface(
    normal: vec3<f32>,
    view_dir: vec3<f32>,
    base_color: vec3<f32>
) -> vec3<f32> {
    let key_dir = normalize(LIGHT_KEY);
    let fill_dir = normalize(LIGHT_FILL);
    
    // Diffuse
    let key = max(dot(normal, key_dir), 0.0) * KEY_STRENGTH;
    let fill = max(dot(normal, fill_dir), 0.0) * FILL_STRENGTH;
    
    // Rim (fresnel-ish)
    let rim = pow(max(1.0 - dot(normal, view_dir), 0.0), 3.0) * RIM_STRENGTH;
    
    // Specular (Blinn-Phong)
    let half_vec = normalize(key_dir + view_dir);
    let spec = pow(max(dot(normal, half_vec), 0.0), SPECULAR_POWER) * SPECULAR_STRENGTH;
    
    let light = AMBIENT + key + fill + rim;
    var color = base_color * light;
    color += vec3(spec);  // White specular highlight
    
    return color;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 BINDINGS — GPU memory layout
// ═══════════════════════════════════════════════════════════════════════════════

// Compute
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> camera_state: CameraState;

// Render
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_camera: CameraState;


// ═══════════════════════════════════════════════════════════════════════════════
// §5 ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(1)
fn update_camera() {
    compose_update_camera();
}

@vertex
fn mesh_vs(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    // Model matrix: identity (pawn at origin)
    let world_pos = in.position;
    let world_normal = in.normal;
    
    // View-projection
    let view = camera_view_matrix(render_camera, AIM_POINT);
    let proj = camera_projection_matrix(render_signal.aspect_ratio);
    let vp = proj * view;
    
    out.clip_pos = vp * vec4(world_pos, 1.0);
    out.world_pos = world_pos;
    out.world_normal = world_normal;
    
    return out;
}

@fragment
fn mesh_fs(in: VertexOutput) -> @location(0) vec4<f32> {
    let normal = normalize(in.world_normal);
    let view_dir = normalize(render_camera.pos - in.world_pos);
    
    let color = shade_surface(normal, view_dir, SUBJECT_COLOR);
    
    return vec4(color, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// END OF SCROLL
// ═══════════════════════════════════════════════════════════════════════════════
