// ═══════════════════════════════════════════════════════════════════════════════
// PLAYGROUND HYBRID — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Branching architecture with precomputed distance field.
// Heavy work done once in compute, fragment just samples.
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ branch_field (256²):  Precomputed wall data per texel                       │
// │   .r = distance to nearest wall centerline                                  │
// │   .g = wall height at this point                                            │
// │   .b = wall thickness                                                       │
// │   .a = interference value                                                   │
// │ Camera:  Full spherical orbit                                               │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// SCROLL INDEX:
//   §1    STRUCTURES ..................... GPU data layout
//   §2    CAMERA ......................... Orbital view system
//   §3    BRANCH FIELD COMPUTE ........... Precompute distance/height
//   §4    SDF FROM TEXTURE ............... Fast wall lookup
//   §5    RAYMARCH ....................... Ray traversal
//   §6    SHADING ........................ Surface appearance
//   §7    BINDINGS ....................... GPU memory
//   §8    ENTRY POINTS ................... Shader stages
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

struct BranchSegment {
    start_x: f32,
    start_z: f32,
    end_x: f32,
    end_z: f32,
    height_start: f32,
    height_end: f32,
    _pad0: f32,
    _pad1: f32,
}

struct WaveSource {
    x: f32,
    z: f32,
    phase: f32,
    _pad0: f32,
}

struct BranchParams {
    wave_frequency: f32,
    height_amplitude: f32,
    thickness_base: f32,
    thickness_amplitude: f32,
    min_thickness: f32,
    max_thickness: f32,
    min_height: f32,
    max_height: f32,
    perimeter_radius: f32,
    center_x: f32,
    center_z: f32,
    segment_count: u32,
    wave_count: u32,
    world_size: f32,
    _pad0: f32,
    _pad1: f32,
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CAMERA — Full spherical orbit
// ═══════════════════════════════════════════════════════════════════════════════

const PI: f32 = 3.14159265359;
const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_ELEVATION: f32 = -0.3;
const CAMERA_MAX_ELEVATION: f32 = 1.5;
const CAMERA_MIN_DISTANCE: f32 = 5.0;
const CAMERA_MAX_DISTANCE: f32 = 80.0;

const AIM_POINT: vec3<f32> = vec3(0.0, 5.0, 0.0);

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
// §3 BRANCH FIELD COMPUTE — Precompute distance/height per texel
// ═══════════════════════════════════════════════════════════════════════════════

const FIELD_SIZE: f32 = 256.0;

// Convert texel coords to world position
fn texel_to_world(texel: vec2<u32>) -> vec2<f32> {
    let uv = (vec2<f32>(texel) + 0.5) / FIELD_SIZE;
    let world_size = branch_params.world_size;
    return (uv - 0.5) * world_size;
}

// Distance from point to line segment in 2D
fn distance_to_segment(p: vec2<f32>, a: vec2<f32>, b: vec2<f32>) -> vec2<f32> {
    let pa = p - a;
    let ba = b - a;
    let len_sq = dot(ba, ba);
    
    if (len_sq < 0.0001) {
        return vec2(length(pa), 0.0);
    }
    
    let t = clamp(dot(pa, ba) / len_sq, 0.0, 1.0);
    let closest = a + ba * t;
    return vec2(length(p - closest), t);
}

// Compute interference at world position
fn compute_interference_at(p2d: vec2<f32>) -> f32 {
    let wave_count = min(branch_params.wave_count, 12u);
    
    var total = 0.0;
    
    for (var i = 0u; i < wave_count; i++) {
        let src = wave_sources[i];
        let src_pos = vec2(src.x, src.z);
        let dist = length(p2d - src_pos);
        
        if (dist > 40.0) {
            continue;
        }
        
        let wave = sin(dist * branch_params.wave_frequency + src.phase);
        let falloff = 1.0 / (1.0 + dist * 0.12);
        
        total += wave * falloff;
    }
    
    if (wave_count > 0u) {
        total /= f32(wave_count) * 0.4;
    }
    
    return clamp(total, -1.0, 1.0);
}

// Query all branches for closest distance and height
fn query_branches_compute(p2d: vec2<f32>) -> vec2<f32> {
    let segment_count = branch_params.segment_count;
    
    var min_dist = 999999.0;
    var best_height = 0.0;
    
    for (var i = 0u; i < segment_count; i++) {
        let seg = branch_segments[i];
        let seg_start = vec2(seg.start_x, seg.start_z);
        let seg_end = vec2(seg.end_x, seg.end_z);
        
        let result = distance_to_segment(p2d, seg_start, seg_end);
        let dist = result.x;
        let t = result.y;
        
        if (dist < min_dist) {
            min_dist = dist;
            best_height = mix(seg.height_start, seg.height_end, t);
        }
    }
    
    return vec2(min_dist, best_height);
}

fn compose_update_branch_field(texel: vec2<u32>) {
    let p2d = texel_to_world(texel);
    
    // Query branches
    let branch_data = query_branches_compute(p2d);
    let dist_to_centerline = branch_data.x;
    let base_height = branch_data.y;
    
    // Compute interference
    let interference = compute_interference_at(p2d);
    
    // Modulate height and thickness
    var wall_height = base_height + interference * branch_params.height_amplitude;
    wall_height = clamp(wall_height, branch_params.min_height, branch_params.max_height);
    
    var thickness = branch_params.thickness_base + interference * branch_params.thickness_amplitude * 0.5;
    thickness = clamp(thickness, branch_params.min_thickness, branch_params.max_thickness);
    
    // Store: distance, height, thickness, interference
    textureStore(branch_field_write, texel, vec4(
        dist_to_centerline,
        wall_height,
        thickness,
        interference
    ));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 SDF FROM TEXTURE — Fast wall lookup
// ═══════════════════════════════════════════════════════════════════════════════

const FLOOR_Y: f32 = 0.0;

fn world_to_uv(p2d: vec2<f32>) -> vec2<f32> {
    let world_size = render_params.world_size;
    return (p2d / world_size) + 0.5;
}

fn sample_branch_field(p2d: vec2<f32>) -> vec4<f32> {
    let uv = world_to_uv(p2d);
    return textureSampleLevel(branch_field_read, bilinear_sampler, uv, 0.0);
}

fn sdf_walls(p: vec3<f32>) -> f32 {
    let p2d = vec2(p.x, p.z);
    let data = sample_branch_field(p2d);
    
    let dist_to_centerline = data.r;
    let wall_height = data.g;
    let thickness = data.b;
    
    // 2D distance to wall surface
    let d_xz = dist_to_centerline - thickness * 0.5;
    
    // Vertical bounds
    let d_below = -p.y + FLOOR_Y;
    let d_above = p.y - wall_height;
    
    return max(d_xz, max(d_below, d_above));
}

fn sdf_floor(p: vec3<f32>) -> f32 {
    return p.y - FLOOR_Y;
}

fn sdf_scene(p: vec3<f32>) -> f32 {
    let walls = sdf_walls(p);
    let floor = sdf_floor(p);
    
    // Smooth blend
    let k = 0.3;
    let h = clamp(0.5 + 0.5 * (floor - walls) / k, 0.0, 1.0);
    return mix(floor, walls, h) - k * h * (1.0 - h);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 RAYMARCH — Ray traversal
// ═══════════════════════════════════════════════════════════════════════════════

const MAX_STEPS: i32 = 80;
const MAX_DIST: f32 = 150.0;
const SURF_DIST: f32 = 0.005;

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
}

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    result.dist = 0.0;
    
    // Bounding sphere early-out
    let bounds_center = vec3(render_params.center_x, render_params.max_height * 0.5, render_params.center_z);
    let bounds_radius = render_params.perimeter_radius + render_params.max_height;
    
    let oc = ro - bounds_center;
    let b = dot(oc, rd);
    let c = dot(oc, oc) - bounds_radius * bounds_radius;
    let discriminant = b * b - c;
    
    if (discriminant < 0.0) {
        result.dist = MAX_DIST;
        return result;
    }
    
    var ray_t = 0.0;
    if (c > 0.0) {
        ray_t = max(0.0, -b - sqrt(discriminant));
    }
    
    for (var i = 0; i < MAX_STEPS; i++) {
        let p = ro + rd * ray_t;
        let d = sdf_scene(p);
        
        if (d < SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = ray_t;
            return result;
        }
        
        ray_t += d * 0.9;
        
        if (ray_t > MAX_DIST) {
            break;
        }
    }
    
    result.dist = ray_t;
    return result;
}

fn calc_normal(p: vec3<f32>) -> vec3<f32> {
    let e = 0.005;
    let d = sdf_scene(p);
    return normalize(vec3(
        sdf_scene(p + vec3(e, 0.0, 0.0)) - d,
        sdf_scene(p + vec3(0.0, e, 0.0)) - d,
        sdf_scene(p + vec3(0.0, 0.0, e)) - d
    ));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 SHADING — Surface appearance
// ═══════════════════════════════════════════════════════════════════════════════

const SKY_COLOR_TOP: vec3<f32> = vec3(0.65, 0.78, 0.95);
const SKY_COLOR_HORIZON: vec3<f32> = vec3(0.85, 0.88, 0.92);
const FOG_DENSITY: f32 = 0.008;

const WALL_COLOR_BASE: vec3<f32> = vec3(0.92, 0.89, 0.84);
const WALL_COLOR_ACCENT: vec3<f32> = vec3(0.82, 0.76, 0.68);
const FLOOR_COLOR: vec3<f32> = vec3(0.78, 0.75, 0.70);

fn get_sky_color(rd: vec3<f32>) -> vec3<f32> {
    let sky_t = rd.y * 0.5 + 0.5;
    return mix(SKY_COLOR_HORIZON, SKY_COLOR_TOP, sky_t);
}

fn get_material_color(p: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let is_floor = smoothstep(0.85, 0.95, normal.y);
    
    // Sample interference from texture for color variation
    let p2d = vec2(p.x, p.z);
    let data = sample_branch_field(p2d);
    let interference = data.a;
    
    let height_factor = clamp(p.y / render_params.max_height, 0.0, 1.0);
    let pattern = interference * 0.5 + 0.5;
    let wall_col = mix(WALL_COLOR_BASE, WALL_COLOR_ACCENT, pattern * 0.3 + height_factor * 0.2);
    
    return mix(wall_col, FLOOR_COLOR, is_floor);
}

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return get_sky_color(rd);
    }
    
    let normal = calc_normal(hit.pos);
    
    let light_dir = normalize(vec3(0.5, 0.8, 0.3));
    let ndotl = max(dot(normal, light_dir), 0.0);
    let fill = max(normal.y, 0.0) * 0.2;
    
    let base_color = get_material_color(hit.pos, normal);
    
    let ambient = 0.35;
    let diffuse = ndotl * 0.5;
    var color = base_color * (ambient + diffuse + fill);
    
    let rim = pow(1.0 - max(dot(normal, -rd), 0.0), 3.0) * 0.1;
    color += rim * SKY_COLOR_TOP;
    
    let fog = 1.0 - exp(-hit.dist * FOG_DENSITY);
    color = mix(color, get_sky_color(rd), fog);
    
    return color;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 BINDINGS — GPU memory
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Compute (group 0: buffers) ───────────────────────────────────────────────
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(2) var<storage, read>       branch_segments: array<BranchSegment>;
@group(0) @binding(3) var<storage, read>       wave_sources: array<WaveSource>;
@group(0) @binding(4) var<storage, read>       branch_params: BranchParams;

// ─── Compute (group 1: textures) ──────────────────────────────────────────────
@group(1) @binding(0) var branch_field_write: texture_storage_2d<rgba16float, write>;

// ─── Render (group 0: buffers) ────────────────────────────────────────────────
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_camera: CameraState;
@group(0) @binding(12) var<storage, read> render_params: BranchParams;

// ─── Render (group 1: textures) ───────────────────────────────────────────────
@group(1) @binding(0) var branch_field_read: texture_2d<f32>;
@group(1) @binding(1) var bilinear_sampler: sampler;


// ═══════════════════════════════════════════════════════════════════════════════
// §8 ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(1)
fn update_camera() {
    compose_update_camera();
}

@compute @workgroup_size(8, 8)
fn update_branch_field(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= 256u || id.y >= 256u) {
        return;
    }
    compose_update_branch_field(id.xy);
}

@vertex
fn terrain_vs(@builtin(vertex_index) vid: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(vid & 1u) * 4 - 1);
    let y = f32(i32((vid >> 1u) & 1u) * 4 - 1);
    return vec4(x, y, 0.0, 1.0);
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
    
    let fov_factor = tan(CAMERA_FOV * 0.5);
    return normalize(
        look_dir +
        right * uv.x * fov_factor * render_signal.aspect_ratio +
        up * uv.y * fov_factor
    );
}

@fragment
fn terrain_fs(@builtin(position) frag_coord: vec4<f32>) -> @location(0) vec4<f32> {
    let resolution = vec2(render_signal.aspect_ratio * 720.0, 720.0);
    let uv = (frag_coord.xy / resolution) * 2.0 - 1.0;
    let corrected_uv = vec2(uv.x, -uv.y);
    
    let rd = get_ray_direction(corrected_uv);
    let hit = raymarch(render_camera.pos, rd);
    let color = shade(hit, rd);
    
    let gamma_corrected = pow(color, vec3(1.0 / 2.2));
    
    return vec4(gamma_corrected, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// END OF SCROLL
// ═══════════════════════════════════════════════════════════════════════════════
