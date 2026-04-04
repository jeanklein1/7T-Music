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
// │ Subject:   Chess piece at origin (pawn-bishop hybrid)                       │
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
//   §3    SUBJECT ......................... line 125
//   §4    RENDER .......................... line 290
//   §5    BINDINGS ........................ line 395
//   §6    ENTRY POINTS .................... line 420
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
    cam.pos = camera_compute_position(vec3(0.0, 1.0, 0.0), cam);  // Aim at piece center
    
    camera_state = cam;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 SUBJECT — Chess Piece (Pawn-Bishop Hybrid)
// ═══════════════════════════════════════════════════════════════════════════════
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ PROFILE REVOLUTION                                                          │
// │                                                                             │
// │ The piece is defined by a 2D profile curve revolved around the Y axis.     │
// │ This mimics how real chess pieces are lathed from wood or carved.          │
// │                                                                             │
// │       ●      ← head (sphere)                                                │
// │      ╱│╲                                                                    │
// │     │ │ │    ← neck                                                         │
// │    ╱  │  ╲   ← collar                                                       │
// │   │   │   │  ← upper body                                                   │
// │   │   │   │                                                                 │
// │   │   │   │  ← bishop body (tall, elegant)                                  │
// │   │   │   │                                                                 │
// │    ╲  │  ╱   ← body taper                                                   │
// │   ──┴─┴─┴──  ← base                                                         │
// │                                                                             │
// │ Adjust the PROFILE constants below to reshape the piece.                    │
// └─────────────────────────────────────────────────────────────────────────────┘

// ─── [SUBJECT:position_and_color] ──────────────────────────────────────────

const SUBJECT_POS: vec3<f32> = vec3(0.0, 0.0, 0.0);
const SUBJECT_COLOR: vec3<f32> = vec3(0.85, 0.82, 0.75);  // Ivory/bone

// ─── [SUBJECT:profile_control_points] ──────────────────────────────────────
//
// Each control point: (height, radius)
// Heights are from bottom (0) to top.
// Tweak these to change the piece's silhouette.

// Base
const P_BASE_FOOT: vec2<f32>      = vec2(0.00, 0.50);   // Bottom of base
const P_BASE_RIM: vec2<f32>       = vec2(0.08, 0.55);   // Decorative rim
const P_BASE_TOP: vec2<f32>       = vec2(0.15, 0.45);   // Top of base platform

// Body (bishop-like: tall and elegant)
const P_BODY_BOTTOM: vec2<f32>    = vec2(0.25, 0.38);   // Body starts
const P_BODY_LOWER: vec2<f32>     = vec2(0.60, 0.32);   // Lower body taper
const P_BODY_MID: vec2<f32>       = vec2(1.10, 0.28);   // Mid body
const P_BODY_UPPER: vec2<f32>     = vec2(1.50, 0.24);   // Upper body

// Collar (decorative ring)
const P_COLLAR_BOTTOM: vec2<f32>  = vec2(1.58, 0.22);   // Below collar
const P_COLLAR_PEAK: vec2<f32>    = vec2(1.65, 0.28);   // Collar bulge
const P_COLLAR_TOP: vec2<f32>     = vec2(1.72, 0.20);   // Above collar

// Neck and head attachment
const P_NECK: vec2<f32>           = vec2(1.82, 0.18);   // Thin neck
const P_HEAD_BASE: vec2<f32>      = vec2(1.92, 0.22);   // Where head meets neck

// Head (sphere parameters)
const HEAD_CENTER_Y: f32 = 2.12;
const HEAD_RADIUS: f32 = 0.25;

// Total height for reference
const PIECE_HEIGHT: f32 = 2.37;  // HEAD_CENTER_Y + HEAD_RADIUS

// ─── [SUBJECT:smoothing] ───────────────────────────────────────────────────

const PROFILE_SMOOTHING: f32 = 0.02;  // Rounds sharp transitions

// ─── [SUBJECT:interpolation_helpers] ───────────────────────────────────────

// Smooth interpolation between two control points
fn lerp_profile(y: f32, p0: vec2<f32>, p1: vec2<f32>) -> f32 {
    let t = clamp((y - p0.x) / (p1.x - p0.x), 0.0, 1.0);
    // Smoothstep for organic feel
    let s = t * t * (3.0 - 2.0 * t);
    return mix(p0.y, p1.y, s);
}

// Check if y is in range [p0.x, p1.x]
fn in_range(y: f32, p0: vec2<f32>, p1: vec2<f32>) -> bool {
    return y >= p0.x && y < p1.x;
}

// ─── [SUBJECT:profile_function] ────────────────────────────────────────────
//
// Returns the radius at a given height.
// This defines the 2D silhouette of the piece.

fn piece_profile(y: f32) -> f32 {
    // Below base
    if (y < P_BASE_FOOT.x) {
        return 0.0;
    }
    
    // Base section
    if (in_range(y, P_BASE_FOOT, P_BASE_RIM)) {
        return lerp_profile(y, P_BASE_FOOT, P_BASE_RIM);
    }
    if (in_range(y, P_BASE_RIM, P_BASE_TOP)) {
        return lerp_profile(y, P_BASE_RIM, P_BASE_TOP);
    }
    
    // Base to body transition
    if (in_range(y, P_BASE_TOP, P_BODY_BOTTOM)) {
        return lerp_profile(y, P_BASE_TOP, P_BODY_BOTTOM);
    }
    
    // Body (the tall bishop-like section)
    if (in_range(y, P_BODY_BOTTOM, P_BODY_LOWER)) {
        return lerp_profile(y, P_BODY_BOTTOM, P_BODY_LOWER);
    }
    if (in_range(y, P_BODY_LOWER, P_BODY_MID)) {
        return lerp_profile(y, P_BODY_LOWER, P_BODY_MID);
    }
    if (in_range(y, P_BODY_MID, P_BODY_UPPER)) {
        return lerp_profile(y, P_BODY_MID, P_BODY_UPPER);
    }
    
    // Collar
    if (in_range(y, P_BODY_UPPER, P_COLLAR_BOTTOM)) {
        return lerp_profile(y, P_BODY_UPPER, P_COLLAR_BOTTOM);
    }
    if (in_range(y, P_COLLAR_BOTTOM, P_COLLAR_PEAK)) {
        return lerp_profile(y, P_COLLAR_BOTTOM, P_COLLAR_PEAK);
    }
    if (in_range(y, P_COLLAR_PEAK, P_COLLAR_TOP)) {
        return lerp_profile(y, P_COLLAR_PEAK, P_COLLAR_TOP);
    }
    
    // Neck
    if (in_range(y, P_COLLAR_TOP, P_NECK)) {
        return lerp_profile(y, P_COLLAR_TOP, P_NECK);
    }
    if (in_range(y, P_NECK, P_HEAD_BASE)) {
        return lerp_profile(y, P_NECK, P_HEAD_BASE);
    }
    
    // Above head base - taper to zero so sphere takes over
    if (y >= P_HEAD_BASE.x) {
        // Above head center: no body, sphere only
        if (y > HEAD_CENTER_Y) {
            return 0.0;
        }
        // Taper from head base radius down to 0 at head center
        let t = (y - P_HEAD_BASE.x) / (HEAD_CENTER_Y - P_HEAD_BASE.x);
        return mix(P_HEAD_BASE.y, 0.0, t);
    }
    
    return 0.0;
}

// ─── [SUBJECT:sdf_profile_body] ────────────────────────────────────────────
//
// SDF for the revolved profile (everything except the head sphere)

fn sdf_profile_body(p: vec3<f32>) -> f32 {
    let r = length(p.xz);          // Radial distance from Y axis
    let y = p.y;                    // Height
    
    // Get desired radius at this height
    let profile_r = piece_profile(y);
    
    // Distance to the revolved surface
    // Positive = outside, negative = inside
    var d = r - profile_r;
    
    // Cap the top and bottom
    let d_bottom = -y;  // Below y=0
    let d_top = y - P_HEAD_BASE.x;  // Above head base (sphere takes over)
    
    // Union with caps (we want the inside, so max with negative of caps)
    d = max(d, d_bottom);
    
    return d;
}

// ─── [SUBJECT:sdf_head] ────────────────────────────────────────────────────

fn sdf_head(p: vec3<f32>) -> f32 {
    let head_center = vec3(0.0, HEAD_CENTER_Y, 0.0);
    return length(p - head_center) - HEAD_RADIUS;
}

// ─── [SUBJECT:smooth_union] ────────────────────────────────────────────────

fn smooth_min(a: f32, b: f32, k: f32) -> f32 {
    let h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}

// ─── [SUBJECT:sdf] ─────────────────────────────────────────────────────────
//
// Final subject SDF: body profile + head sphere, smoothly joined

fn sdf_subject(p: vec3<f32>) -> f32 {
    let local = p - SUBJECT_POS;
    
    let body = sdf_profile_body(local);
    let head = sdf_head(local);
    
    // Smooth union of body and head
    return smooth_min(body, head, PROFILE_SMOOTHING * 4.0);
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
