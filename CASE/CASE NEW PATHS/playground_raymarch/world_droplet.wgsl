// ═══════════════════════════════════════════════════════════════════════════════
// PLAYGROUND RAYMARCH — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Minimal sculpture viewer. One object floating in void.
// Full spherical camera rotation for viewing from any angle.
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ Chart:     Void space, no boundaries                                        │
// │ Subject:   Pawn at origin (the shape we iterate on)                         │
// │ Camera:    Full spherical orbit (azimuth 360°, elevation ±85°)              │
// │ Lighting:  Studio three-point                                                │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// PURPOSE:
//   This is the creative workbench. Modify §3 SUBJECT to experiment with
//   procedural shapes. When satisfied, copy the SDF to other cartridges.
//
// SCROLL INDEX:
//   §1    SIGNAL .......................... line 35
//   §2    CAMERA .......................... line 55
//   §3    SUBJECT ......................... line 115
//   §4    RENDER .......................... line 175
//   §5    BINDINGS ........................ line 280
//   §6    ENTRY POINTS .................... line 305
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 SIGNAL — Frame data from CPU
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


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CAMERA — Full spherical orbit
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [CAMERA:limits] ────────────────────────────────────────────────────────
//
// Full spherical rotation:
//   Azimuth: unlimited (wraps naturally)
//   Elevation: ±85° (avoid gimbal lock at poles)

const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_ELEVATION: f32 = -1.48;   // ~-85° (looking up from below)
const CAMERA_MAX_ELEVATION: f32 = 1.48;    // ~+85° (looking down from above)
const CAMERA_MIN_DISTANCE: f32 = 1.5;      // Close inspection
const CAMERA_MAX_DISTANCE: f32 = 20.0;     // Far overview

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

// ─── [CAMERA:compute_position] ──────────────────────────────────────────────

fn camera_compute_position(aim_point: vec3<f32>, cam: CameraState) -> vec3<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    // Forward direction (toward aim_point)
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    // Apply pan offset in screen space
    let look_at = aim_point + right * cam.pan_x + up * cam.pan_y;
    
    // Orbital offset
    let offset = cam.distance * vec3(cos_el * sin_az, sin_el, cos_el * cos_az);
    
    return look_at + offset;
}

// ─── [CAMERA:update] ────────────────────────────────────────────────────────

fn compose_update_camera() {
    var cam = camera_state;
    
    // Apply input deltas
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
    
    // Compute position (orbiting around origin)
    cam.pos = camera_compute_position(vec3(0.0, 0.75, 0.0), cam);  // Aim slightly above origin
    
    camera_state = cam;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 SUBJECT — The shape we're sculpting
// ═══════════════════════════════════════════════════════════════════════════════
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ THIS IS THE CREATIVE CANVAS                                                 │
// │                                                                             │
// │ Modify sdf_subject() to experiment with procedural shapes.                  │
// │ The rest of the scroll is frozen infrastructure.                            │
// └─────────────────────────────────────────────────────────────────────────────┘

// ─── [SUBJECT:parameters] ───────────────────────────────────────────────────

const SUBJECT_POS: vec3<f32> = vec3(0.0, 0.0, 0.0);
const SUBJECT_COLOR: vec3<f32> = vec3(0.6, 0.55, 0.7);  // Muted purple

// ─── [SUBJECT:primitives] ───────────────────────────────────────────────────

fn sdf_round_cone(p: vec3<f32>, r_base: f32, r_tip: f32, h: f32) -> f32 {
    let b = (r_base - r_tip) / h;
    let a = sqrt(1.0 - b * b);
    let q = vec2(length(p.xz), p.y);
    let k = dot(q, vec2(-b, a));
    if (k < 0.0) { return length(q) - r_base; }
    if (k > a * h) { return length(q - vec2(0.0, h)) - r_tip; }
    return dot(q, vec2(a, b)) - r_base;
}

fn sdf_sphere(p: vec3<f32>, r: f32) -> f32 {
    return length(p) - r;
}

fn sdf_box(p: vec3<f32>, b: vec3<f32>) -> f32 {
    let q = abs(p) - b;
    return length(max(q, vec3(0.0))) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// ─── [SUBJECT:sdf] ──────────────────────────────────────────────────────────
//
// Current subject: Pawn (round cone)
// Modify this function to try different shapes.

fn sdf_subject(p: vec3<f32>) -> f32 {
    let local = p - SUBJECT_POS;
    
    // Pawn: round cone
    // Base radius 0.5, tip radius 0.0, height 1.5
    return sdf_round_cone(local, 0.5, 0.0, 1.5);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 RENDER — Raymarching and shading
// ═══════════════════════════════════════════════════════════════════════════════

const MAX_STEPS: i32 = 100;
const MAX_DIST: f32 = 50.0;
const SURF_DIST: f32 = 0.002;

// ─── [RENDER:lighting] ──────────────────────────────────────────────────────
//
// Studio three-point lighting for sculptural clarity.

const LIGHT_KEY: vec3<f32> = vec3(0.6, 0.8, 0.5);      // Main light (warm, upper right)
const LIGHT_FILL: vec3<f32> = vec3(-0.4, 0.3, 0.6);    // Fill light (cool, left)
const LIGHT_RIM: vec3<f32> = vec3(0.0, 0.2, -0.8);     // Rim light (behind)

const AMBIENT: f32 = 0.15;
const KEY_STRENGTH: f32 = 0.6;
const FILL_STRENGTH: f32 = 0.25;
const RIM_STRENGTH: f32 = 0.3;

// ─── [RENDER:void] ──────────────────────────────────────────────────────────
//
// Background void color and fog.

const VOID_COLOR: vec3<f32> = vec3(0.02, 0.02, 0.03);
const FOG_DENSITY: f32 = 0.03;

// ─── [RENDER:raymarch] ──────────────────────────────────────────────────────

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
}

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    result.dist = 0.0;
    
    var t: f32 = 0.0;
    
    for (var i: i32 = 0; i < MAX_STEPS; i++) {
        let p = ro + rd * t;
        let d = sdf_subject(p);
        
        if (d < SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = t;
            return result;
        }
        
        t += d;
        
        if (t > MAX_DIST) {
            break;
        }
    }
    
    result.dist = t;
    return result;
}

// ─── [RENDER:normal] ────────────────────────────────────────────────────────

fn calc_normal(p: vec3<f32>) -> vec3<f32> {
    let eps = 0.001;
    let d = sdf_subject(p);
    return normalize(vec3(
        sdf_subject(p + vec3(eps, 0.0, 0.0)) - d,
        sdf_subject(p + vec3(0.0, eps, 0.0)) - d,
        sdf_subject(p + vec3(0.0, 0.0, eps)) - d
    ));
}

// ─── [RENDER:shade] ─────────────────────────────────────────────────────────

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return VOID_COLOR;
    }
    
    let normal = calc_normal(hit.pos);
    
    // Three-point lighting
    let key_dir = normalize(LIGHT_KEY);
    let fill_dir = normalize(LIGHT_FILL);
    let rim_dir = normalize(LIGHT_RIM);
    
    let key = max(dot(normal, key_dir), 0.0) * KEY_STRENGTH;
    let fill = max(dot(normal, fill_dir), 0.0) * FILL_STRENGTH;
    let rim = pow(max(1.0 - dot(normal, -rd), 0.0), 3.0) * RIM_STRENGTH;
    
    let light = AMBIENT + key + fill + rim;
    var color = SUBJECT_COLOR * light;
    
    // Fog toward void
    let fog = 1.0 - exp(-hit.dist * FOG_DENSITY);
    color = mix(color, VOID_COLOR, fog);
    
    return color;
}

// ─── [RENDER:ray_direction] ─────────────────────────────────────────────────

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
    
    let fov_factor = tan(CAMERA_FOV * 0.5);
    return normalize(
        look_dir +
        right * uv.x * fov_factor * render_signal.aspect_ratio +
        up * uv.y * fov_factor
    );
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 BINDINGS — GPU memory layout
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [BINDINGS:compute] ─────────────────────────────────────────────────────

@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> camera_state: CameraState;

// ─── [BINDINGS:render] ──────────────────────────────────────────────────────

@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_camera: CameraState;


// ═══════════════════════════════════════════════════════════════════════════════
// §6 ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(1)
fn update_camera() {
    compose_update_camera();
}

@vertex
fn fullscreen_vs(@builtin(vertex_index) vid: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(vid & 1u) * 4 - 1);
    let y = f32(i32((vid >> 1u) & 1u) * 4 - 1);
    return vec4(x, y, 0.0, 1.0);
}

@fragment
fn world_fs(@builtin(position) frag_coord: vec4<f32>) -> @location(0) vec4<f32> {
    let resolution = vec2(render_signal.aspect_ratio * 720.0, 720.0);
    let uv = (frag_coord.xy / resolution) * 2.0 - 1.0;
    let corrected_uv = vec2(uv.x, -uv.y);
    
    let rd = get_ray_direction(corrected_uv);
    let hit = raymarch(render_camera.pos, rd);
    let color = shade(hit, rd);
    
    return vec4(color, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// END OF SCROLL
// ═══════════════════════════════════════════════════════════════════════════════
