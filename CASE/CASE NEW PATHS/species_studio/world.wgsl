// ═══════════════════════════════════════════════════════════════════════════════
// SPECIES STUDIO — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Live-editable pawn species designer.
// All shape, material, and texture parameters come from the design_params buffer.
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ Chart:     Void space with subtle ground reference                          │
// │ Subject:   Parameterized chess piece profile                                │
// │ Camera:    Full spherical orbit                                             │
// │ Lighting:  Studio three-point + environment                                 │
// │ Textures:  Procedural (grain, marble, bands, cells)                         │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// SCROLL INDEX:
//   §1    STRUCTURES ..................... line 30
//   §2    CAMERA ......................... line 75
//   §3    NOISE .......................... line 130
//   §4    SUBJECT PROFILE ................ line 200
//   §5    TEXTURES ....................... line 300
//   §6    RENDER ......................... line 400
//   §7    BINDINGS ....................... line 550
//   §8    ENTRY POINTS ................... line 580
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

struct DesignParams {
    // Profile
    base_radius: f32,
    base_rim_radius: f32,
    base_height: f32,
    body_bottom_radius: f32,
    body_taper: f32,
    body_height: f32,
    collar_bulge: f32,
    collar_height: f32,
    neck_radius: f32,
    head_radius: f32,
    head_height: f32,
    smoothing: f32,
    
    // Material
    color_r: f32,
    color_g: f32,
    color_b: f32,
    metallic: f32,
    roughness: f32,
    accent_r: f32,
    accent_g: f32,
    accent_b: f32,
    accent_blend: f32,
    _mat_pad0: f32,
    _mat_pad1: f32,
    _mat_pad2: f32,
    
    // Texture
    pattern_type: f32,
    pattern_scale: f32,
    pattern_intensity: f32,
    pattern_distort: f32,
    grain_direction: f32,
    noise_octaves: f32,
    _tex_pad0: f32,
    _tex_pad1: f32,
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CAMERA — Full spherical orbit
// ═══════════════════════════════════════════════════════════════════════════════

const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_ELEVATION: f32 = -1.48;
const CAMERA_MAX_ELEVATION: f32 = 1.48;
const CAMERA_MIN_DISTANCE: f32 = 1.5;
const CAMERA_MAX_DISTANCE: f32 = 20.0;

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
    
    // Aim at center of piece (approximate)
    let piece_center_y = params.base_height + params.body_height * 0.5;
    cam.pos = camera_compute_position(vec3(0.0, piece_center_y, 0.0), cam);
    
    camera_state = cam;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 NOISE — Procedural texture foundation
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Hash functions ──────────────────────────────────────────────────────────

fn hash21(p: vec2<f32>) -> f32 {
    var p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

fn hash31(p: vec3<f32>) -> f32 {
    var p3 = fract(p * 0.1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

fn hash33(p: vec3<f32>) -> vec3<f32> {
    var p3 = fract(p * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yxz + 33.33);
    return fract((p3.xxy + p3.yxx) * p3.zyx);
}

// ─── Value noise ─────────────────────────────────────────────────────────────

fn noise3(p: vec3<f32>) -> f32 {
    let i = floor(p);
    let f = fract(p);
    let u = f * f * (3.0 - 2.0 * f);
    
    return mix(
        mix(
            mix(hash31(i + vec3(0.0, 0.0, 0.0)), hash31(i + vec3(1.0, 0.0, 0.0)), u.x),
            mix(hash31(i + vec3(0.0, 1.0, 0.0)), hash31(i + vec3(1.0, 1.0, 0.0)), u.x),
            u.y
        ),
        mix(
            mix(hash31(i + vec3(0.0, 0.0, 1.0)), hash31(i + vec3(1.0, 0.0, 1.0)), u.x),
            mix(hash31(i + vec3(0.0, 1.0, 1.0)), hash31(i + vec3(1.0, 1.0, 1.0)), u.x),
            u.y
        ),
        u.z
    );
}

// ─── Fractal Brownian Motion ─────────────────────────────────────────────────

fn fbm(p: vec3<f32>, octaves: i32) -> f32 {
    var value = 0.0;
    var amplitude = 0.5;
    var frequency = 1.0;
    var pos = p;
    
    for (var i = 0; i < octaves; i++) {
        value += amplitude * noise3(pos * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

// ─── Voronoi / Cellular ──────────────────────────────────────────────────────

fn voronoi(p: vec3<f32>) -> vec2<f32> {
    let n = floor(p);
    let f = fract(p);
    
    var min_dist = 8.0;
    var second_dist = 8.0;
    
    for (var k = -1; k <= 1; k++) {
        for (var j = -1; j <= 1; j++) {
            for (var i = -1; i <= 1; i++) {
                let g = vec3(f32(i), f32(j), f32(k));
                let o = hash33(n + g);
                let r = g + o - f;
                let d = dot(r, r);
                
                if (d < min_dist) {
                    second_dist = min_dist;
                    min_dist = d;
                } else if (d < second_dist) {
                    second_dist = d;
                }
            }
        }
    }
    
    return vec2(sqrt(min_dist), sqrt(second_dist));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 SUBJECT PROFILE — Parameterized chess piece
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Derived heights (computed from params) ──────────────────────────────────

fn get_body_top_y() -> f32 {
    return render_params.base_height + render_params.body_height;
}

fn get_collar_top_y() -> f32 {
    return get_body_top_y() + render_params.collar_height;
}

fn get_neck_top_y() -> f32 {
    return get_collar_top_y() + render_params.collar_height * 0.5;
}

fn get_head_center_y() -> f32 {
    return get_neck_top_y() + render_params.head_height - render_params.head_radius;
}

fn get_total_height() -> f32 {
    return get_head_center_y() + render_params.head_radius;
}

// ─── Profile function ────────────────────────────────────────────────────────
//
// Returns desired radius at given height.

fn piece_profile(y: f32) -> f32 {
    let p = render_params;
    
    // Key heights
    let base_rim_y = p.base_height * 0.5;
    let base_top_y = p.base_height;
    let body_top_y = base_top_y + p.body_height;
    let collar_mid_y = body_top_y + p.collar_height * 0.5;
    let collar_top_y = body_top_y + p.collar_height;
    let neck_top_y = collar_top_y + p.collar_height * 0.5;
    let head_center_y = neck_top_y + p.head_height - p.head_radius;
    
    // Body taper: radius decreases as we go up
    let body_top_radius = p.body_bottom_radius * (1.0 - p.body_taper);
    
    // Below base
    if (y < 0.0) { return 0.0; }
    
    // Base foot to rim
    if (y < base_rim_y) {
        let t = y / base_rim_y;
        let s = t * t * (3.0 - 2.0 * t);
        return mix(p.base_radius, p.base_rim_radius, s);
    }
    
    // Base rim to top
    if (y < base_top_y) {
        let t = (y - base_rim_y) / (base_top_y - base_rim_y);
        let s = t * t * (3.0 - 2.0 * t);
        return mix(p.base_rim_radius, p.body_bottom_radius, s);
    }
    
    // Body (elegant taper)
    if (y < body_top_y) {
        let t = (y - base_top_y) / p.body_height;
        // Use smooth curve for organic taper
        let s = t * t * (3.0 - 2.0 * t);
        return mix(p.body_bottom_radius, body_top_radius, s);
    }
    
    // Collar approach (pinch before bulge)
    if (y < collar_mid_y) {
        let t = (y - body_top_y) / (collar_mid_y - body_top_y);
        let s = t * t * (3.0 - 2.0 * t);
        let pinch = body_top_radius * 0.85;
        return mix(body_top_radius, p.collar_bulge, s);
    }
    
    // Collar peak to collar top (back down)
    if (y < collar_top_y) {
        let t = (y - collar_mid_y) / (collar_top_y - collar_mid_y);
        let s = t * t * (3.0 - 2.0 * t);
        return mix(p.collar_bulge, p.neck_radius * 1.1, s);
    }
    
    // Neck
    if (y < neck_top_y) {
        let t = (y - collar_top_y) / (neck_top_y - collar_top_y);
        let s = t * t * (3.0 - 2.0 * t);
        return mix(p.neck_radius * 1.1, p.neck_radius, s);
    }
    
    // Head attachment zone - taper to zero for sphere blend
    if (y < head_center_y) {
        let t = (y - neck_top_y) / (head_center_y - neck_top_y);
        return mix(p.neck_radius, 0.0, t);
    }
    
    // Above head center - sphere only
    return 0.0;
}

// ─── Profile body SDF ────────────────────────────────────────────────────────

fn sdf_profile_body(p: vec3<f32>) -> f32 {
    let r = length(p.xz);
    let y = p.y;
    
    let profile_r = piece_profile(y);
    
    // If profile radius is zero, body doesn't exist here
    // (prevents the Y-axis from becoming a surface to infinity)
    if (profile_r <= 0.0) {
        return 1000.0;
    }
    
    var d = r - profile_r;
    
    // Cap bottom
    d = max(d, -y);
    
    return d;
}

// ─── Head sphere SDF ─────────────────────────────────────────────────────────

fn sdf_head(p: vec3<f32>) -> f32 {
    let head_center = vec3(0.0, get_head_center_y(), 0.0);
    return length(p - head_center) - render_params.head_radius;
}

// ─── Smooth min ──────────────────────────────────────────────────────────────

fn smooth_min(a: f32, b: f32, k: f32) -> f32 {
    let h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * 0.25;
}

// ─── Complete subject SDF ────────────────────────────────────────────────────

fn sdf_subject(p: vec3<f32>) -> f32 {
    let body = sdf_profile_body(p);
    let head = sdf_head(p);
    
    let k = render_params.smoothing * 4.0;
    return smooth_min(body, head, max(k, 0.01));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 TEXTURES — Procedural patterns
// ═══════════════════════════════════════════════════════════════════════════════

const PATTERN_NONE: i32 = 0;
const PATTERN_GRAIN: i32 = 1;
const PATTERN_MARBLE: i32 = 2;
const PATTERN_BANDS: i32 = 3;
const PATTERN_CELLS: i32 = 4;

const GRAIN_VERTICAL: i32 = 0;
const GRAIN_RADIAL: i32 = 1;
const GRAIN_SPIRAL: i32 = 2;

// ─── Wood grain ──────────────────────────────────────────────────────────────

fn texture_grain(p: vec3<f32>, normal: vec3<f32>) -> f32 {
    let params = render_params;
    let scale = params.pattern_scale;
    let distort = params.pattern_distort;
    let octaves = i32(params.noise_octaves);
    let direction = i32(params.grain_direction);
    
    var coord: f32;
    
    if (direction == GRAIN_VERTICAL) {
        // Vertical grain (like turned wood)
        coord = p.y * scale;
        coord += fbm(p * scale * 0.5, octaves) * distort * 2.0;
    } else if (direction == GRAIN_RADIAL) {
        // Radial grain (like tree rings)
        let r = length(p.xz);
        coord = r * scale;
        coord += fbm(p * scale * 0.3, octaves) * distort * 2.0;
    } else {
        // Spiral grain
        let angle = atan2(p.z, p.x);
        let r = length(p.xz);
        coord = (angle * 0.5 + r + p.y * 0.3) * scale;
        coord += fbm(p * scale * 0.4, octaves) * distort * 2.0;
    }
    
    // Create grain lines
    let grain = sin(coord * 6.28318) * 0.5 + 0.5;
    return pow(grain, 2.0);
}

// ─── Marble veining ──────────────────────────────────────────────────────────

fn texture_marble(p: vec3<f32>, normal: vec3<f32>) -> f32 {
    let params = render_params;
    let scale = params.pattern_scale;
    let distort = params.pattern_distort;
    let octaves = i32(params.noise_octaves);
    
    // Distorted coordinates
    let distorted = p + vec3(
        fbm(p * scale, octaves),
        fbm(p * scale + vec3(5.2, 1.3, 2.8), octaves),
        fbm(p * scale + vec3(1.7, 9.2, 3.1), octaves)
    ) * distort;
    
    // Veining pattern
    let vein = sin((distorted.x + distorted.y + distorted.z) * scale * 2.0);
    
    // Add some turbulence
    let turb = fbm(distorted * scale * 2.0, octaves);
    
    return clamp(vein * 0.5 + 0.5 + turb * 0.3, 0.0, 1.0);
}

// ─── Horizontal bands ────────────────────────────────────────────────────────

fn texture_bands(p: vec3<f32>, normal: vec3<f32>) -> f32 {
    let params = render_params;
    let scale = params.pattern_scale;
    let distort = params.pattern_distort;
    let octaves = i32(params.noise_octaves);
    
    // Height-based bands with noise distortion
    var y = p.y * scale;
    y += fbm(p * scale * 0.5, octaves) * distort;
    
    let band = sin(y * 6.28318) * 0.5 + 0.5;
    return pow(band, 1.5);
}

// ─── Cellular pattern ────────────────────────────────────────────────────────

fn texture_cells(p: vec3<f32>, normal: vec3<f32>) -> f32 {
    let params = render_params;
    let scale = params.pattern_scale;
    let distort = params.pattern_distort;
    
    // Distort position
    let distorted = p * scale + vec3(
        noise3(p * scale * 2.0),
        noise3(p * scale * 2.0 + vec3(17.0, 0.0, 0.0)),
        noise3(p * scale * 2.0 + vec3(0.0, 31.0, 0.0))
    ) * distort;
    
    let v = voronoi(distorted);
    
    // Edge detection (difference between closest and second closest)
    let edge = v.y - v.x;
    
    return clamp(edge * 2.0, 0.0, 1.0);
}

// ─── Pattern dispatcher ──────────────────────────────────────────────────────

fn get_texture_value(p: vec3<f32>, normal: vec3<f32>) -> f32 {
    let pattern = i32(render_params.pattern_type);
    
    switch pattern {
        case PATTERN_GRAIN: {
            return texture_grain(p, normal);
        }
        case PATTERN_MARBLE: {
            return texture_marble(p, normal);
        }
        case PATTERN_BANDS: {
            return texture_bands(p, normal);
        }
        case PATTERN_CELLS: {
            return texture_cells(p, normal);
        }
        default: {
            return 0.0;
        }
    }
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 RENDER — Raymarching and shading
// ═══════════════════════════════════════════════════════════════════════════════

const MAX_STEPS: i32 = 100;
const MAX_DIST: f32 = 50.0;
const SURF_DIST: f32 = 0.002;

// ─── Environment ─────────────────────────────────────────────────────────────

const VOID_COLOR: vec3<f32> = vec3(0.02, 0.02, 0.03);
const GROUND_COLOR: vec3<f32> = vec3(0.08, 0.08, 0.09);
const FOG_DENSITY: f32 = 0.02;

// ─── Lighting ────────────────────────────────────────────────────────────────

const LIGHT_KEY: vec3<f32> = vec3(0.6, 0.8, 0.5);
const LIGHT_FILL: vec3<f32> = vec3(-0.4, 0.3, 0.6);
const LIGHT_RIM: vec3<f32> = vec3(0.0, 0.2, -0.8);

const AMBIENT: f32 = 0.15;
const KEY_STRENGTH: f32 = 0.6;
const FILL_STRENGTH: f32 = 0.25;
const RIM_STRENGTH: f32 = 0.3;

// ─── Ground plane ────────────────────────────────────────────────────────────

fn sdf_ground(p: vec3<f32>) -> f32 {
    return p.y + 0.01;  // Just below y=0
}

// ─── Scene ───────────────────────────────────────────────────────────────────

const MAT_NONE: u32 = 0u;
const MAT_SUBJECT: u32 = 1u;
const MAT_GROUND: u32 = 2u;

struct SceneHit {
    dist: f32,
    material: u32,
}

fn sdf_scene(p: vec3<f32>) -> SceneHit {
    var result: SceneHit;
    
    let subject_d = sdf_subject(p);
    let ground_d = sdf_ground(p);
    
    if (subject_d < ground_d) {
        result.dist = subject_d;
        result.material = MAT_SUBJECT;
    } else {
        result.dist = ground_d;
        result.material = MAT_GROUND;
    }
    
    return result;
}

// ─── Raymarch ────────────────────────────────────────────────────────────────

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
    material: u32,
}

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    result.dist = 0.0;
    result.material = MAT_NONE;
    
    var t = 0.0;
    
    for (var i = 0; i < MAX_STEPS; i++) {
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
    
    result.dist = t;
    return result;
}

// ─── Normal calculation ──────────────────────────────────────────────────────

fn calc_normal(p: vec3<f32>) -> vec3<f32> {
    let eps = 0.001;
    let d = sdf_scene(p).dist;
    return normalize(vec3(
        sdf_scene(p + vec3(eps, 0.0, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, eps, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, 0.0, eps)).dist - d
    ));
}

// ─── Material color ──────────────────────────────────────────────────────────

fn get_material_color(p: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let params = render_params;
    
    // Base color
    let base = vec3(params.color_r, params.color_g, params.color_b);
    let accent = vec3(params.accent_r, params.accent_g, params.accent_b);
    
    // Get texture value
    let tex = get_texture_value(p, normal);
    let tex_influence = tex * params.pattern_intensity;
    
    // Blend base with darkened version based on texture
    var color = mix(base, base * 0.6, tex_influence);
    
    // Blend in accent color
    color = mix(color, accent, params.accent_blend * (1.0 - tex * 0.5));
    
    return color;
}

// ─── Shading ─────────────────────────────────────────────────────────────────

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return VOID_COLOR;
    }
    
    let normal = calc_normal(hit.pos);
    
    // Material
    var base_color: vec3<f32>;
    if (hit.material == MAT_SUBJECT) {
        base_color = get_material_color(hit.pos, normal);
    } else {
        // Ground: subtle grid
        let grid = step(0.98, fract(hit.pos.x * 2.0)) + step(0.98, fract(hit.pos.z * 2.0));
        base_color = mix(GROUND_COLOR, GROUND_COLOR * 1.5, min(grid, 1.0) * 0.3);
    }
    
    // Three-point lighting
    let key_dir = normalize(LIGHT_KEY);
    let fill_dir = normalize(LIGHT_FILL);
    let rim_dir = normalize(LIGHT_RIM);
    
    let key = max(dot(normal, key_dir), 0.0) * KEY_STRENGTH;
    let fill = max(dot(normal, fill_dir), 0.0) * FILL_STRENGTH;
    let rim = pow(max(1.0 - dot(normal, -rd), 0.0), 3.0) * RIM_STRENGTH;
    
    // Metallic influence
    let metallic = render_params.metallic;
    let roughness = render_params.roughness;
    
    // Fresnel (more pronounced for metallic materials)
    let fresnel = pow(1.0 - max(dot(normal, -rd), 0.0), 3.0 + metallic * 2.0);
    
    // Combine lighting
    var light = AMBIENT + key + fill;
    
    // Add specular highlight (stronger for metallic/smooth)
    let half_vec = normalize(key_dir - rd);
    let spec = pow(max(dot(normal, half_vec), 0.0), 32.0 / (roughness + 0.1));
    light += spec * (0.3 + metallic * 0.5);
    
    // Rim light
    light += rim * (1.0 + metallic);
    
    var color = base_color * light;
    
    // Metallic reflection tint
    color = mix(color, base_color * light * 1.5, metallic * fresnel);
    
    // Fog
    let fog = 1.0 - exp(-hit.dist * FOG_DENSITY);
    color = mix(color, VOID_COLOR, fog);
    
    return color;
}

// ─── Ray direction ───────────────────────────────────────────────────────────

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
// §7 BINDINGS — GPU memory layout
// ═══════════════════════════════════════════════════════════════════════════════

// Compute
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(2) var<storage, read>       params: DesignParams;

// Render
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_camera: CameraState;
@group(0) @binding(12) var<storage, read> render_params: DesignParams;


// ═══════════════════════════════════════════════════════════════════════════════
// §8 ENTRY POINTS
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
