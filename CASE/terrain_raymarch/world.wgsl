// world.wgsl
// ═══════════════════════════════════════════════════════════════════════════════
// RAYMARCHING WORLD — 7T Musical Visualizer
// ═══════════════════════════════════════════════════════════════════════════════
//
// Native raymarching grammar. Objects defined in canonical form, 
// transformed explicitly, composed clearly.
//
// STRUCTURE:
//   1. SIGNAL & STATE     - Input from CPU, persistent GPU state
//   2. PRIMITIVES         - Canonical SDFs (at origin, axis-aligned)
//   3. DOMAIN OPERATIONS  - Transform space before evaluating primitive
//   4. COMBINATION OPS    - Combine multiple distance values
//   5. WAVE FUNCTIONS     - Musical displacement field
//   6. SCENE OBJECTS      - Primitives + Transformations
//   7. SCENE COMPOSITION  - Final assembly
//   8. RAYMARCHER         - Core algorithm
//   9. SHADING            - Materials and lighting
//  10. DYNAMICS           - Compute shader (simulation)
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// 1. SIGNAL & STATE
// ═══════════════════════════════════════════════════════════════════════════════

const STATS_PER_CHANNEL: i32 = 16;
const STAT_POLYPHONY: i32 = 0;

fn stat_index(channel: i32, stat: i32) -> i32 {
    return channel * STATS_PER_CHANNEL + stat;
}

fn polyphony() -> f32 { 
    return signal.stats[stat_index(0, STAT_POLYPHONY)]; 
}

struct FrameSignal {
    t_seconds: f32,
    t_beats: f32,
    dt: f32,
    aspect_ratio: f32,
    stats: array<f32, 64>,
    move_x: f32,
    move_z: f32,
    look_az_delta: f32,
    look_el_delta: f32,
    zoom_delta: f32,
    pan_x_delta: f32,
    pan_y_delta: f32,
    _pad1: f32,
}

struct Trajectory {
    value: f32,
    velocity: f32,
    _pad0: f32,
    _pad1: f32,
}

struct TerrainState {
    amplitude_scale: f32,
    max_amplitude: f32,
    size: f32,
    _pad: f32,
}

struct PawnState {
    pos_x: f32,
    pos_y: f32,
    pos_z: f32,
    heading: f32,
    orientation: vec4<f32>,
}

struct CameraState {
    pos_x: f32,
    pos_y: f32,
    pos_z: f32,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
    view_proj: mat4x4<f32>,
}

// Compute bindings
@group(0) @binding(0) var<storage, read> signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> terrain_state: TerrainState;
@group(0) @binding(2) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(3) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(4) var<storage, read_write> trajectories: array<Trajectory, 16>;

// Render bindings (same buffers, read-only)
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_terrain: TerrainState;
@group(0) @binding(12) var<storage, read> render_pawn: PawnState;
@group(0) @binding(13) var<storage, read> render_camera: CameraState;


// ═══════════════════════════════════════════════════════════════════════════════
// CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

// Pawn geometry
const PAWN_RADIUS: f32 = 0.5;
const PAWN_HEIGHT: f32 = 1.5;
const PAWN_SPEED: f32 = 15.0;

// Camera
const CAMERA_MIN_ELEVATION: f32 = -0.5;
const CAMERA_MAX_ELEVATION: f32 = 1.5;
const CAMERA_MIN_DISTANCE: f32 = 5.0;
const CAMERA_MAX_DISTANCE: f32 = 100.0;
const CAMERA_FOV: f32 = 1.0;

// Raymarching
const MAX_STEPS: i32 = 128;
const MAX_DIST: f32 = 200.0;
const SURF_DIST: f32 = 0.01;

// Checkerboard
const TILE_SIZE: f32 = 4.0;

// Colors
const COLOR_TILE_A: vec3<f32> = vec3<f32>(0.15, 0.4, 0.6);   // Deep blue
const COLOR_TILE_B: vec3<f32> = vec3<f32>(0.6, 0.35, 0.15);  // Warm orange
const COLOR_PAWN: vec3<f32> = vec3<f32>(0.8, 0.5, 0.8);      // Purple
const COLOR_SKY: vec3<f32> = vec3<f32>(0.85, 0.78, 0.72);    // Fog/sky
const COLOR_FOG: vec3<f32> = vec3<f32>(0.85, 0.78, 0.72);


// ═══════════════════════════════════════════════════════════════════════════════
// 2. PRIMITIVES — Canonical Forms
// Objects at origin, axis-aligned, simplest possible expression
// ═══════════════════════════════════════════════════════════════════════════════

// Infinite plane at y = 0
// Returns: positive above, negative below, zero on surface
fn prim_plane(p: vec3<f32>) -> f32 {
    return p.y;
}

// Sphere centered at origin
fn prim_sphere(p: vec3<f32>, r: f32) -> f32 {
    return length(p) - r;
}

// Box centered at origin with half-extents b
fn prim_box(p: vec3<f32>, b: vec3<f32>) -> f32 {
    let q = abs(p) - b;
    return length(max(q, vec3<f32>(0.0))) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// Round cone: base at y=0 (radius r_base), tip at y=h (radius r_tip)
// From Inigo Quilez — works correctly with r_tip=0 for pointed cone
fn prim_round_cone(p: vec3<f32>, r_base: f32, r_tip: f32, h: f32) -> f32 {
    let b = (r_base - r_tip) / h;
    let a = sqrt(1.0 - b * b);
    let q = vec2<f32>(length(p.xz), p.y);
    let k = dot(q, vec2<f32>(-b, a));
    if (k < 0.0) { return length(q) - r_base; }
    if (k > a * h) { return length(q - vec2<f32>(0.0, h)) - r_tip; }
    return dot(q, vec2<f32>(a, b)) - r_base;
}


// ═══════════════════════════════════════════════════════════════════════════════
// 3. DOMAIN OPERATIONS — Transform Space
// These modify the input point before passing to a primitive
// ═══════════════════════════════════════════════════════════════════════════════

// Translation: evaluate as if object were at `offset`
fn op_translate(p: vec3<f32>, offset: vec3<f32>) -> vec3<f32> {
    return p - offset;
}

// Rotation by quaternion: evaluate as if object were rotated
fn op_rotate_quat(p: vec3<f32>, q: vec4<f32>) -> vec3<f32> {
    // Inverse rotation of point = forward rotation of space
    let u = q.xyz;
    let s = q.w;
    // q^(-1) * p * q, but we rotate p by inverse quaternion
    let q_inv = vec4<f32>(-u, s);
    let u_inv = q_inv.xyz;
    let s_inv = q_inv.w;
    return 2.0 * dot(u_inv, p) * u_inv + (s_inv * s_inv - dot(u_inv, u_inv)) * p + 2.0 * s_inv * cross(u_inv, p);
}

// Vertical displacement: shift y by a scalar
fn op_displace_y(p: vec3<f32>, displacement: f32) -> vec3<f32> {
    return vec3<f32>(p.x, p.y - displacement, p.z);
}

// Bounds check: returns true if inside rectangular region
fn op_in_bounds_xz(p: vec3<f32>, half_size: f32) -> bool {
    return abs(p.x) < half_size && abs(p.z) < half_size;
}


// ═══════════════════════════════════════════════════════════════════════════════
// 4. COMBINATION OPERATIONS — Compose Distance Values
// ═══════════════════════════════════════════════════════════════════════════════

fn op_union(a: f32, b: f32) -> f32 {
    return min(a, b);
}

fn op_intersect(a: f32, b: f32) -> f32 {
    return max(a, b);
}

fn op_subtract(a: f32, b: f32) -> f32 {
    return max(a, -b);
}

// Smooth minimum — blends two surfaces
fn op_blend(a: f32, b: f32, k: f32) -> f32 {
    let h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}


// ═══════════════════════════════════════════════════════════════════════════════
// 5. WAVE FUNCTIONS — Musical Displacement Field
// Defines how the ground surface undulates
// ═══════════════════════════════════════════════════════════════════════════════

struct WaveParams {
    amplitude: f32,
    frequency: f32,
    dir_x: f32,
    dir_z: f32,
    cycle_length: f32,
}

// Wave definitions (canonical form: unit amplitude before scaling)
const WAVE_0 = WaveParams(1.0, 0.15, 0.7, 0.7, 8.0);
const WAVE_1 = WaveParams(0.5, 0.30, -0.5, 0.8, 6.0);
const WAVE_2 = WaveParams(0.3, 0.50, 0.9, -0.3, 4.0);

fn eval_wave(pos: vec2<f32>, time: f32, w: WaveParams) -> f32 {
    let spatial = w.frequency * (w.dir_x * pos.x + w.dir_z * pos.y);
    let temporal = (6.28318 / w.cycle_length) * time;
    return w.amplitude * sin(spatial + temporal);
}

// Combined wave displacement at a point
fn wave_displacement(pos: vec2<f32>, time: f32, amplitude_scale: f32) -> f32 {
    var h: f32 = 0.0;
    h += eval_wave(pos, time, WAVE_0);
    h += eval_wave(pos, time, WAVE_1);
    h += eval_wave(pos, time, WAVE_2);
    return h * amplitude_scale;
}

// Maximum possible gradient (Lipschitz constant for safe raymarching)
fn wave_max_gradient(amplitude_scale: f32) -> f32 {
    let base_max = WAVE_0.amplitude * WAVE_0.frequency +
                   WAVE_1.amplitude * WAVE_1.frequency +
                   WAVE_2.amplitude * WAVE_2.frequency;
    return base_max * amplitude_scale * 1.5;  // Safety margin
}


// ═══════════════════════════════════════════════════════════════════════════════
// 6. SCENE OBJECTS — Primitives + Transformations
// Each object is a composition: primitive(transform(p))
// ═══════════════════════════════════════════════════════════════════════════════

// ─── GROUND ─────────────────────────────────────────────────────────────────────
// Ground = Plane + Wave Displacement (vertical)
//
// Construction:
//   1. Start with canonical plane (y = 0)
//   2. Displace vertically by wave function
//   3. Correct distance for domain distortion (Lipschitz)

fn sdf_ground(p: vec3<f32>) -> f32 {
    // Wave displacement at this (x, z) position
    let displacement = wave_displacement(
        vec2<f32>(p.x, p.z),
        render_signal.t_beats,
        render_terrain.amplitude_scale * render_terrain.max_amplitude
    );
    
    // Transform domain: shift y down by displacement
    let p_displaced = op_displace_y(p, displacement);
    
    // Evaluate canonical plane
    let raw_dist = prim_plane(p_displaced);
    
    // Lipschitz correction: domain distortion affects distance bound
    // Without this, rays overshoot on steep slopes
    let max_grad = wave_max_gradient(render_terrain.amplitude_scale * render_terrain.max_amplitude);
    let lipschitz_divisor = sqrt(1.0 + max_grad * max_grad);
    
    return raw_dist / lipschitz_divisor;
}

fn ground_normal(p: vec3<f32>) -> vec3<f32> {
    let eps = 0.1;
    let amp = render_terrain.amplitude_scale * render_terrain.max_amplitude;
    let t = render_signal.t_beats;
    
    let h_L = wave_displacement(vec2<f32>(p.x - eps, p.z), t, amp);
    let h_R = wave_displacement(vec2<f32>(p.x + eps, p.z), t, amp);
    let h_D = wave_displacement(vec2<f32>(p.x, p.z - eps), t, amp);
    let h_U = wave_displacement(vec2<f32>(p.x, p.z + eps), t, amp);
    
    return normalize(vec3<f32>(h_L - h_R, 2.0 * eps, h_D - h_U));
}


// ─── PAWN ───────────────────────────────────────────────────────────────────────
// Pawn = Round Cone + Translation + Rotation
//
// Construction:
//   1. Start with canonical round cone (base at origin, tip at +Y)
//   2. Rotate by orientation quaternion (terrain-following)
//   3. Translate to world position

fn pawn_position() -> vec3<f32> {
    return vec3<f32>(render_pawn.pos_x, render_pawn.pos_y, render_pawn.pos_z);
}

fn pawn_orientation() -> vec4<f32> {
    return render_pawn.orientation;
}

fn sdf_pawn(p: vec3<f32>) -> f32 {
    // Transform chain: world → translated → rotated → local
    let p_translated = op_translate(p, pawn_position());
    let p_local = op_rotate_quat(p_translated, pawn_orientation());
    
    // Evaluate canonical cone
    return prim_round_cone(p_local, PAWN_RADIUS, 0.0, PAWN_HEIGHT);
}


// ═══════════════════════════════════════════════════════════════════════════════
// 7. SCENE COMPOSITION — Final Assembly
// ═══════════════════════════════════════════════════════════════════════════════

const MAT_SKY: i32 = 0;
const MAT_GROUND: i32 = 1;
const MAT_PAWN: i32 = 2;

struct SceneHit {
    dist: f32,
    material: i32,
}

fn sdf_scene(p: vec3<f32>) -> SceneHit {
    var hit: SceneHit;
    
    let d_pawn = sdf_pawn(p);
    
    // Ground only within bounds
    let half_size = render_terrain.size * 0.5;
    let in_bounds = op_in_bounds_xz(p, half_size);
    
    if (in_bounds) {
        let d_ground = sdf_ground(p);
        
        // Union: take nearest surface
        if (d_ground < d_pawn) {
            hit.dist = d_ground;
            hit.material = MAT_GROUND;
        } else {
            hit.dist = d_pawn;
            hit.material = MAT_PAWN;
        }
    } else {
        hit.dist = d_pawn;
        hit.material = MAT_PAWN;
    }
    
    return hit;
}


// ═══════════════════════════════════════════════════════════════════════════════
// 8. RAYMARCHER — Core Algorithm
// ═══════════════════════════════════════════════════════════════════════════════

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
    steps: i32,
    material: i32,
}

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    result.dist = 0.0;
    result.steps = 0;
    result.material = MAT_SKY;
    
    var t: f32 = 0.0;
    
    for (var i: i32 = 0; i < MAX_STEPS; i++) {
        result.steps = i;
        let p = ro + rd * t;
        
        let scene = sdf_scene(p);
        
        if (scene.dist < SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = t;
            result.material = scene.material;
            break;
        }
        
        if (t > MAX_DIST) {
            break;
        }
        
        t += scene.dist * 0.8;  // Conservative stepping
    }
    
    result.dist = t;
    return result;
}

fn calc_normal(p: vec3<f32>) -> vec3<f32> {
    let eps = 0.001;
    let d = sdf_scene(p).dist;
    return normalize(vec3<f32>(
        sdf_scene(p + vec3<f32>(eps, 0.0, 0.0)).dist - d,
        sdf_scene(p + vec3<f32>(0.0, eps, 0.0)).dist - d,
        sdf_scene(p + vec3<f32>(0.0, 0.0, eps)).dist - d
    ));
}


// ═══════════════════════════════════════════════════════════════════════════════
// 9. SHADING — Materials and Lighting
// ═══════════════════════════════════════════════════════════════════════════════

fn get_ray(uv: vec2<f32>) -> vec3<f32> {
    let cos_el = cos(render_camera.elevation);
    let sin_el = sin(render_camera.elevation);
    let cos_az = cos(render_camera.azimuth);
    let sin_az = sin(render_camera.azimuth);
    
    // Orbital direction (pawn → camera)
    let orbital = vec3<f32>(cos_el * sin_az, sin_el, cos_el * cos_az);
    
    // Camera basis
    let look_dir = -orbital;
    let right = vec3<f32>(cos_az, 0.0, -sin_az);
    let up = cross(orbital, right);
    
    // Ray from UV
    let fov_factor = tan(CAMERA_FOV * 0.5);
    return normalize(
        look_dir + 
        right * uv.x * fov_factor * render_signal.aspect_ratio + 
        up * uv.y * fov_factor
    );
}

// ─── GROUND SHADING ─────────────────────────────────────────────────────────────
// Checkerboard pattern from world coordinates

fn shade_ground(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    // Checkerboard from world xz position
    let checker = floor(pos.x / TILE_SIZE) + floor(pos.z / TILE_SIZE);
    let tile = i32(checker) % 2;
    
    // Base color from tile
    var color = select(COLOR_TILE_B, COLOR_TILE_A, tile == 0);
    
    // Modulate by height (subtle)
    let height_factor = clamp(pos.y * 0.1 + 0.5, 0.3, 1.0);
    color = color * height_factor;
    
    // Diffuse lighting
    let ndotl = max(dot(normal, light_dir), 0.0);
    return color * (0.3 + 0.7 * ndotl);
}

// ─── PAWN SHADING ───────────────────────────────────────────────────────────────

fn shade_pawn(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    return COLOR_PAWN * (0.3 + 0.7 * ndotl);
}

// ─── MAIN SHADE FUNCTION ────────────────────────────────────────────────────────

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return COLOR_SKY;
    }
    
    let light_dir = normalize(vec3<f32>(0.3, 1.0, 0.2));
    var color: vec3<f32>;
    
    if (hit.material == MAT_GROUND) {
        let normal = ground_normal(hit.pos);
        color = shade_ground(hit.pos, normal, light_dir);
    } else {
        let normal = calc_normal(hit.pos);
        color = shade_pawn(hit.pos, normal, light_dir);
    }
    
    // Distance fog
    let fog = 1.0 - exp(-hit.dist * 0.015);
    return mix(color, COLOR_FOG, fog);
}


// ═══════════════════════════════════════════════════════════════════════════════
// VERTEX & FRAGMENT SHADERS
// ═══════════════════════════════════════════════════════════════════════════════

@vertex
fn fullscreen_vs(@builtin(vertex_index) vid: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(vid & 1u) * 4 - 1);
    let y = f32(i32((vid >> 1u) & 1u) * 4 - 1);
    return vec4<f32>(x, y, 0.0, 1.0);
}

@fragment
fn world_fs(@builtin(position) frag_coord: vec4<f32>) -> @location(0) vec4<f32> {
    let ar = render_signal.aspect_ratio;
    let resolution = vec2<f32>(ar * 720.0, 720.0);
    let uv = (frag_coord.xy / resolution) * 2.0 - 1.0;
    let corrected_uv = vec2<f32>(uv.x, -uv.y);
    
    let cam_pos = vec3<f32>(render_camera.pos_x, render_camera.pos_y, render_camera.pos_z);
    let rd = get_ray(corrected_uv);
    
    let hit = raymarch(cam_pos, rd);
    let color = shade(hit, rd);
    
    return vec4<f32>(color, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// 10. DYNAMICS — Compute Shader (Simulation)
// ═══════════════════════════════════════════════════════════════════════════════

fn step_spring(value: f32, velocity: f32, goal: f32, dt: f32, stiffness: f32, damping: f32) -> vec2<f32> {
    let displacement = value - goal;
    let spring_force = -stiffness * displacement;
    let damping_force = -damping * sqrt(stiffness) * velocity;
    let acceleration = spring_force + damping_force;
    let new_velocity = velocity + acceleration * dt;
    let new_value = value + new_velocity * dt;
    return vec2<f32>(new_value, new_velocity);
}

fn step_asymptotic(value: f32, goal: f32, dt: f32, rate: f32) -> f32 {
    return value + (goal - value) * (1.0 - exp(-rate * dt));
}

fn quat_from_axis_angle(axis: vec3<f32>, angle: f32) -> vec4<f32> {
    let half_angle = angle * 0.5;
    let s = sin(half_angle);
    return vec4<f32>(axis * s, cos(half_angle));
}

fn quat_multiply(a: vec4<f32>, b: vec4<f32>) -> vec4<f32> {
    return vec4<f32>(
        a.w * b.xyz + b.w * a.xyz + cross(a.xyz, b.xyz),
        a.w * b.w - dot(a.xyz, b.xyz)
    );
}

// Compute-side wave evaluation (uses signal, not render_signal)
fn wave_displacement_compute(pos: vec2<f32>, time: f32, amplitude_scale: f32) -> f32 {
    var h: f32 = 0.0;
    h += eval_wave(pos, time, WAVE_0);
    h += eval_wave(pos, time, WAVE_1);
    h += eval_wave(pos, time, WAVE_2);
    return h * amplitude_scale;
}

fn ground_normal_compute(x: f32, z: f32) -> vec3<f32> {
    let eps = 0.1;
    let amp = terrain_state.amplitude_scale * terrain_state.max_amplitude;
    let t = signal.t_beats;
    
    let h_L = wave_displacement_compute(vec2<f32>(x - eps, z), t, amp);
    let h_R = wave_displacement_compute(vec2<f32>(x + eps, z), t, amp);
    let h_D = wave_displacement_compute(vec2<f32>(x, z - eps), t, amp);
    let h_U = wave_displacement_compute(vec2<f32>(x, z + eps), t, amp);
    
    return normalize(vec3<f32>(h_L - h_R, 2.0 * eps, h_D - h_U));
}

// Trajectory indices
const TRAJ_TERRAIN_AMP: i32 = 0;

// Coupling constants
const TERRAIN_AMP_IDLE: f32 = 1.0;
const TERRAIN_AMP_ATTACK_STIFFNESS: f32 = 15.0;
const TERRAIN_AMP_ATTACK_DAMPING: f32 = 0.6;
const TERRAIN_AMP_RELEASE_RATE: f32 = 0.5;

@compute @workgroup_size(1)
fn update() {
    let dt = signal.dt;
    
    // ─── MUSICAL COUPLING: amplitude ∝ polyphony ─────────────────────────────
    
    let poly = polyphony();
    let goal = TERRAIN_AMP_IDLE * (1.0 + poly);
    let attacking = poly > 0.0;
    
    if (attacking) {
        let result = step_spring(
            trajectories[TRAJ_TERRAIN_AMP].value,
            trajectories[TRAJ_TERRAIN_AMP].velocity,
            goal, dt,
            TERRAIN_AMP_ATTACK_STIFFNESS,
            TERRAIN_AMP_ATTACK_DAMPING
        );
        trajectories[TRAJ_TERRAIN_AMP].value = result.x;
        trajectories[TRAJ_TERRAIN_AMP].velocity = result.y;
    } else {
        trajectories[TRAJ_TERRAIN_AMP].value = step_asymptotic(
            trajectories[TRAJ_TERRAIN_AMP].value,
            TERRAIN_AMP_IDLE, dt,
            TERRAIN_AMP_RELEASE_RATE
        );
        trajectories[TRAJ_TERRAIN_AMP].velocity = 0.0;
    }
    
    terrain_state.amplitude_scale = trajectories[TRAJ_TERRAIN_AMP].value;
    
    // ─── PAWN ────────────────────────────────────────────────────────────────
    
    let cos_az = cos(camera_state.azimuth);
    let sin_az = sin(camera_state.azimuth);
    
    let world_move_x = signal.move_x * cos_az + signal.move_z * sin_az;
    let world_move_z = -signal.move_x * sin_az + signal.move_z * cos_az;
    
    pawn_state.pos_x += world_move_x * PAWN_SPEED * dt;
    pawn_state.pos_z += world_move_z * PAWN_SPEED * dt;
    
    let half = terrain_state.size * 0.5 - 1.0;
    pawn_state.pos_x = clamp(pawn_state.pos_x, -half, half);
    pawn_state.pos_z = clamp(pawn_state.pos_z, -half, half);
    
    // Pawn sits on ground surface
    let amp = terrain_state.amplitude_scale * terrain_state.max_amplitude;
    pawn_state.pos_y = wave_displacement_compute(
        vec2<f32>(pawn_state.pos_x, pawn_state.pos_z),
        signal.t_beats,
        amp
    );
    
    let move_len_sq = world_move_x * world_move_x + world_move_z * world_move_z;
    if (move_len_sq > 0.001) {
        let target_heading = atan2(world_move_x, world_move_z);
        var heading_diff = target_heading - pawn_state.heading;
        
        if (heading_diff > 3.14159) { heading_diff -= 6.28318; }
        else if (heading_diff < -3.14159) { heading_diff += 6.28318; }
        
        let turn_speed = 8.0;
        let max_turn = turn_speed * dt;
        pawn_state.heading += clamp(heading_diff, -max_turn, max_turn);
        
        if (pawn_state.heading > 3.14159) { pawn_state.heading -= 6.28318; }
        else if (pawn_state.heading < -3.14159) { pawn_state.heading += 6.28318; }
    }
    
    // Pawn orientation follows ground normal
    let world_up = vec3<f32>(0.0, 1.0, 0.0);
    let ground_up = ground_normal_compute(pawn_state.pos_x, pawn_state.pos_z);
    
    let dot_val = dot(world_up, ground_up);
    if (dot_val < 0.9999) {
        let axis = normalize(cross(world_up, ground_up));
        let angle = acos(clamp(dot_val, -1.0, 1.0));
        let tilt_quat = quat_from_axis_angle(axis, angle);
        let heading_quat = quat_from_axis_angle(vec3<f32>(0.0, 1.0, 0.0), pawn_state.heading);
        pawn_state.orientation = quat_multiply(tilt_quat, heading_quat);
    } else {
        pawn_state.orientation = quat_from_axis_angle(vec3<f32>(0.0, 1.0, 0.0), pawn_state.heading);
    }
    
    // ─── CAMERA ──────────────────────────────────────────────────────────────
    
    camera_state.azimuth += signal.look_az_delta;
    camera_state.elevation = clamp(
        camera_state.elevation + signal.look_el_delta,
        CAMERA_MIN_ELEVATION,
        CAMERA_MAX_ELEVATION
    );
    camera_state.distance = clamp(
        camera_state.distance + signal.zoom_delta,
        CAMERA_MIN_DISTANCE,
        CAMERA_MAX_DISTANCE
    );
    
    let pan_scale = camera_state.distance * 0.5;
    camera_state.pan_x += signal.pan_x_delta * pan_scale;
    camera_state.pan_y += signal.pan_y_delta * pan_scale;
    
    let cam_cos_el = cos(camera_state.elevation);
    let cam_sin_el = sin(camera_state.elevation);
    let cam_cos_az = cos(camera_state.azimuth);
    let cam_sin_az = sin(camera_state.azimuth);
    
    let cam_forward = vec3<f32>(-cam_cos_el * cam_sin_az, -cam_sin_el, -cam_cos_el * cam_cos_az);
    let cam_right = vec3<f32>(cam_cos_az, 0.0, -cam_sin_az);
    let cam_up = cross(cam_right, cam_forward);
    
    let pawn_pos = vec3<f32>(pawn_state.pos_x, pawn_state.pos_y, pawn_state.pos_z);
    let look_target = pawn_pos + cam_right * camera_state.pan_x + cam_up * camera_state.pan_y;
    
    let cam_offset = camera_state.distance * vec3<f32>(
        cam_cos_el * cam_sin_az,
        cam_sin_el,
        cam_cos_el * cam_cos_az
    );
    let cam_pos = look_target + cam_offset;
    
    camera_state.pos_x = cam_pos.x;
    camera_state.pos_y = cam_pos.y;
    camera_state.pos_z = cam_pos.z;
}
