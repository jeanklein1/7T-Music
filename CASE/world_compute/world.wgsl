// world.wgsl
// ═══════════════════════════════════════════════════════════════════════════════
// 7T MUSICAL VISUALIZER — Compute-First Architecture
// ═══════════════════════════════════════════════════════════════════════════════
//
// PRINCIPLE: Every piece of data has a natural dimensionality.
//            Compute at that dimensionality. Render samples it.
//
// DIMENSIONALITY:
//   0D — One value for entire scene    → Buffer (scalar/struct)
//   1D — One value per entity in list  → Buffer (array)
//   2D — One value per surface point   → Texture 2D
//   3D — One value per volume point    → Texture 3D
//
// EXECUTION ORDER:
//   1. Compute: update_world()         [1 thread]      → 0D state
//   2. Compute: update_height_field()  [N×N threads]   → 2D height texture
//   3. Compute: update_tiles()         [M×M threads]   → 2D tile texture
//   4. Render:  sample textures, observe
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 1: TYPES
// ═══════════════════════════════════════════════════════════════════════════════

// ─── 0D State Structures ─────────────────────────────────────────────────────

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

struct GroundState {
    amplitude_scale: f32,
    max_amplitude: f32,
    size: f32,
    lipschitz_factor: f32,  // Precomputed for raymarching
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


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 2: BINDINGS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Group 0: Entity State (0D and 1D) ───────────────────────────────────────

// Compute bindings (read-write)
@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> ground_state: GroundState;
@group(0) @binding(2) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(3) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(4) var<storage, read_write> trajectories: array<Trajectory, 16>;

// Render bindings (read-only, same data)
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_ground: GroundState;
@group(0) @binding(12) var<storage, read> render_pawn: PawnState;
@group(0) @binding(13) var<storage, read> render_camera: CameraState;

// ─── Group 1: 2D Textures ────────────────────────────────────────────────────

// Compute bindings (write)
// Note: Using rgba16float instead of r32float for filtering support
@group(1) @binding(0) var height_field_write: texture_storage_2d<rgba16float, write>;
@group(1) @binding(1) var tile_state_write: texture_storage_2d<rgba8unorm, write>;

// Render bindings (read)
@group(1) @binding(10) var height_field_read: texture_2d<f32>;
@group(1) @binding(11) var tile_state_read: texture_2d<f32>;
@group(1) @binding(12) var bilinear_sampler: sampler;
@group(1) @binding(13) var nearest_sampler: sampler;


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 3: CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

// Musical stats
const STATS_PER_CHANNEL: i32 = 16;
const STAT_POLYPHONY: i32 = 0;
const STAT_VELOCITY: i32 = 1;

// Texture dimensions
const HEIGHT_FIELD_SIZE: u32 = 256u;  // 256×256 texels
const TILE_TEXTURE_SIZE: u32 = 64u;   // 64×64 tiles

// Pawn
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

// Visual
const TILE_SIZE: f32 = 4.0;
const COLOR_PAWN: vec3<f32> = vec3<f32>(0.8, 0.5, 0.8);
const COLOR_SKY: vec3<f32> = vec3<f32>(0.85, 0.78, 0.72);
const COLOR_FOG: vec3<f32> = vec3<f32>(0.85, 0.78, 0.72);

// Tile colors
const COLOR_TILE_A: vec3<f32> = vec3<f32>(0.88, 0.78, 0.73);  // Pinkish sand
const COLOR_TILE_B: vec3<f32> = vec3<f32>(0.05, 0.05, 0.05);  // Near black


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 4: SHARED UTILITIES
// Functions used by both compute and render
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Coordinate Transforms ───────────────────────────────────────────────────

fn height_texel_to_world(texel: vec2<u32>, ground_size: f32) -> vec2<f32> {
    let uv = vec2<f32>(texel) / f32(HEIGHT_FIELD_SIZE);
    return (uv - 0.5) * ground_size;
}

fn world_to_height_uv(world_xz: vec2<f32>, ground_size: f32) -> vec2<f32> {
    return (world_xz / ground_size) + 0.5;
}

fn tile_texel_to_world(texel: vec2<u32>, ground_size: f32) -> vec2<f32> {
    let uv = vec2<f32>(texel) / f32(TILE_TEXTURE_SIZE);
    return (uv - 0.5) * ground_size;
}

fn world_to_tile_uv(world_xz: vec2<f32>, ground_size: f32) -> vec2<f32> {
    return (world_xz / ground_size) + 0.5;
}

// ─── Wave Functions (the mathematical definition) ────────────────────────────

struct WaveParams {
    amplitude: f32,
    frequency: f32,
    dir_x: f32,
    dir_z: f32,
    cycle_length: f32,
}

const WAVE_0 = WaveParams(1.0, 0.15, 0.7, 0.7, 8.0);
const WAVE_1 = WaveParams(0.5, 0.30, -0.5, 0.8, 6.0);
const WAVE_2 = WaveParams(0.3, 0.50, 0.9, -0.3, 4.0);

fn eval_wave(pos: vec2<f32>, time: f32, w: WaveParams) -> f32 {
    let spatial = w.frequency * (w.dir_x * pos.x + w.dir_z * pos.y);
    let temporal = (6.28318 / w.cycle_length) * time;
    return w.amplitude * sin(spatial + temporal);
}

fn compute_wave_height(pos: vec2<f32>, time: f32, amplitude_scale: f32) -> f32 {
    var h: f32 = 0.0;
    h += eval_wave(pos, time, WAVE_0);
    h += eval_wave(pos, time, WAVE_1);
    h += eval_wave(pos, time, WAVE_2);
    return h * amplitude_scale;
}

fn compute_wave_max_gradient(amplitude_scale: f32) -> f32 {
    let base_max = WAVE_0.amplitude * WAVE_0.frequency +
                   WAVE_1.amplitude * WAVE_1.frequency +
                   WAVE_2.amplitude * WAVE_2.frequency;
    return base_max * amplitude_scale * 1.5;
}

// ─── Analytic Gradient (exact, from calculus) ────────────────────────────────
// d/dx sin(f*(dx*x + dz*z) + wt) = f * dx * cos(...)
// d/dz sin(f*(dx*x + dz*z) + wt) = f * dz * cos(...)

fn eval_wave_gradient(pos: vec2<f32>, time: f32, w: WaveParams) -> vec2<f32> {
    let phase = w.frequency * (w.dir_x * pos.x + w.dir_z * pos.y) 
              + (6.28318 / w.cycle_length) * time;
    let c = cos(phase);
    let scale = w.amplitude * w.frequency * c;
    return vec2<f32>(w.dir_x * scale, w.dir_z * scale);
}

fn compute_wave_gradient(pos: vec2<f32>, time: f32, amplitude_scale: f32) -> vec2<f32> {
    var grad = vec2<f32>(0.0, 0.0);
    grad += eval_wave_gradient(pos, time, WAVE_0);
    grad += eval_wave_gradient(pos, time, WAVE_1);
    grad += eval_wave_gradient(pos, time, WAVE_2);
    return grad * amplitude_scale;
}

fn compute_wave_normal(pos: vec2<f32>, time: f32, amplitude_scale: f32) -> vec3<f32> {
    let grad = compute_wave_gradient(pos, time, amplitude_scale);
    return normalize(vec3<f32>(-grad.x, 1.0, -grad.y));
}

// ─── Quaternion Operations ───────────────────────────────────────────────────

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

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let u = q.xyz;
    let s = q.w;
    return 2.0 * dot(u, v) * u + (s * s - dot(u, u)) * v + 2.0 * s * cross(u, v);
}

fn quat_inverse(q: vec4<f32>) -> vec4<f32> {
    return vec4<f32>(-q.xyz, q.w);
}

// ─── Trajectory Helpers ──────────────────────────────────────────────────────

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

// ─── Hash Functions (for tile identity) ──────────────────────────────────────

fn hash21(p: vec2<f32>) -> f32 {
    var p3 = fract(vec3<f32>(p.x, p.y, p.x) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 5: COMPUTE PASS 1 — Update World (0D)
// Single thread, updates global state
// ═══════════════════════════════════════════════════════════════════════════════

const TRAJ_GROUND_AMP: i32 = 0;

const GROUND_AMP_IDLE: f32 = 1.0;
const GROUND_AMP_ATTACK_STIFFNESS: f32 = 15.0;
const GROUND_AMP_ATTACK_DAMPING: f32 = 0.6;
const GROUND_AMP_RELEASE_RATE: f32 = 0.5;

fn stat_index(channel: i32, stat: i32) -> i32 {
    return channel * STATS_PER_CHANNEL + stat;
}

@compute @workgroup_size(1)
fn update_world() {
    let dt = signal.dt;
    let t = signal.t_beats;
    
    // ─── GROUND: Musical coupling (amplitude ∝ polyphony) ────────────────────
    
    let poly = signal.stats[stat_index(0, STAT_POLYPHONY)];
    let goal = GROUND_AMP_IDLE * (1.0 + poly);
    let attacking = poly > 0.0;
    
    if (attacking) {
        let result = step_spring(
            trajectories[TRAJ_GROUND_AMP].value,
            trajectories[TRAJ_GROUND_AMP].velocity,
            goal, dt,
            GROUND_AMP_ATTACK_STIFFNESS,
            GROUND_AMP_ATTACK_DAMPING
        );
        trajectories[TRAJ_GROUND_AMP].value = result.x;
        trajectories[TRAJ_GROUND_AMP].velocity = result.y;
    } else {
        trajectories[TRAJ_GROUND_AMP].value = step_asymptotic(
            trajectories[TRAJ_GROUND_AMP].value,
            GROUND_AMP_IDLE, dt,
            GROUND_AMP_RELEASE_RATE
        );
        trajectories[TRAJ_GROUND_AMP].velocity = 0.0;
    }
    
    ground_state.amplitude_scale = trajectories[TRAJ_GROUND_AMP].value;
    
    // Precompute Lipschitz factor for raymarching
    let max_grad = compute_wave_max_gradient(ground_state.amplitude_scale * ground_state.max_amplitude);
    ground_state.lipschitz_factor = sqrt(1.0 + max_grad * max_grad);
    
    // ─── PAWN: Movement and terrain following ────────────────────────────────
    
    let cos_az = cos(camera_state.azimuth);
    let sin_az = sin(camera_state.azimuth);
    
    let world_move_x = signal.move_x * cos_az + signal.move_z * sin_az;
    let world_move_z = -signal.move_x * sin_az + signal.move_z * cos_az;
    
    pawn_state.pos.x += world_move_x * PAWN_SPEED * dt;
    pawn_state.pos.z += world_move_z * PAWN_SPEED * dt;
    
    let half = ground_state.size * 0.5 - 1.0;
    pawn_state.pos.x = clamp(pawn_state.pos.x, -half, half);
    pawn_state.pos.z = clamp(pawn_state.pos.z, -half, half);
    
    // Pawn Y from ground height
    let amp = ground_state.amplitude_scale * ground_state.max_amplitude;
    pawn_state.pos.y = compute_wave_height(pawn_state.pos.xz, t, amp);
    
    // Pawn heading
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
    
    // Pawn orientation (terrain following) — analytic normal, exact
    let ground_normal = compute_wave_normal(pawn_state.pos.xz, t, amp);
    
    let world_up = vec3<f32>(0.0, 1.0, 0.0);
    let dot_val = dot(world_up, ground_normal);
    if (dot_val < 0.9999) {
        let axis = normalize(cross(world_up, ground_normal));
        let angle = acos(clamp(dot_val, -1.0, 1.0));
        let tilt_quat = quat_from_axis_angle(axis, angle);
        let heading_quat = quat_from_axis_angle(vec3<f32>(0.0, 1.0, 0.0), pawn_state.heading);
        pawn_state.orientation = quat_multiply(tilt_quat, heading_quat);
    } else {
        pawn_state.orientation = quat_from_axis_angle(vec3<f32>(0.0, 1.0, 0.0), pawn_state.heading);
    }
    
    // ─── CAMERA: Orbital follow ──────────────────────────────────────────────
    
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
    
    let look_target = pawn_state.pos + cam_right * camera_state.pan_x + cam_up * camera_state.pan_y;
    
    let cam_offset = camera_state.distance * vec3<f32>(
        cam_cos_el * cam_sin_az,
        cam_sin_el,
        cam_cos_el * cam_cos_az
    );
    
    camera_state.pos = look_target + cam_offset;
}


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 6: COMPUTE PASS 2 — Update Height Field (2D)
// One thread per texel, bakes wave height to texture
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(8, 8)
fn update_height_field(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= HEIGHT_FIELD_SIZE || id.y >= HEIGHT_FIELD_SIZE) {
        return;
    }
    
    let world_xz = height_texel_to_world(id.xy, ground_state.size);
    let amp = ground_state.amplitude_scale * ground_state.max_amplitude;
    let h = compute_wave_height(world_xz, signal.t_beats, amp);
    let grad = compute_wave_gradient(world_xz, signal.t_beats, amp);
    
    // Store height in .x, gradient in .yz (rgba16float for filtering support)
    // This allows single-sample normal lookup at render time
    textureStore(height_field_write, id.xy, vec4<f32>(h, grad.x, grad.y, 1.0));
}


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 7: COMPUTE PASS 3 — Update Tiles (2D)
// One thread per tile, bakes tile state to texture
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(8, 8)
fn update_tiles(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= TILE_TEXTURE_SIZE || id.y >= TILE_TEXTURE_SIZE) {
        return;
    }
    
    // Tile identity (deterministic from position)
    let tile_pos = vec2<f32>(f32(id.x), f32(id.y));
    let hash = hash21(tile_pos);
    
    // Checkerboard pattern
    let is_dark = ((id.x + id.y) % 2u) == 0u;
    
    // Base color
    var color: vec3<f32>;
    if (is_dark) {
        color = COLOR_TILE_B;
    } else {
        color = COLOR_TILE_A;
    }
    
    // Future: Add musical couplings here
    // let activation = compute_tile_activation(id.xy, poly, pawn_pos, ...);
    // color = mix(muted_color, vibrant_color, activation);
    
    let activation = 1.0;  // Placeholder for future coupling
    
    textureStore(tile_state_write, id.xy, vec4<f32>(color, activation));
}


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 8: RENDER — SDF Primitives
// Pure math, no state access
// ═══════════════════════════════════════════════════════════════════════════════

fn prim_plane(p: vec3<f32>) -> f32 {
    return p.y;
}

fn prim_sphere(p: vec3<f32>, r: f32) -> f32 {
    return length(p) - r;
}

fn prim_round_cone(p: vec3<f32>, r_base: f32, r_tip: f32, h: f32) -> f32 {
    let b = (r_base - r_tip) / h;
    let a = sqrt(1.0 - b * b);
    let q = vec2<f32>(length(p.xz), p.y);
    let k = dot(q, vec2<f32>(-b, a));
    if (k < 0.0) { return length(q) - r_base; }
    if (k > a * h) { return length(q - vec2<f32>(0.0, h)) - r_tip; }
    return dot(q, vec2<f32>(a, b)) - r_base;
}

fn op_blend(a: f32, b: f32, k: f32) -> f32 {
    let h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}


// ═══════════════════════════════════════════════════════════════════════════════
// SECTION 9: RENDER — Scene SDFs (sample precomputed data)
// ═══════════════════════════════════════════════════════════════════════════════

// Ground: sample height from precomputed texture
fn sdf_ground(p: vec3<f32>) -> f32 {
    let uv = world_to_height_uv(p.xz, render_ground.size);
    let h = textureSampleLevel(height_field_read, bilinear_sampler, uv, 0.0).x;
    let vertical_dist = p.y - h;
    return vertical_dist / render_ground.lipschitz_factor;
}

// Ground normal: single sample, gradient pre-baked in .yz
fn ground_normal_from_texture(p: vec3<f32>) -> vec3<f32> {
    let uv = world_to_height_uv(p.xz, render_ground.size);
    let data = textureSampleLevel(height_field_read, bilinear_sampler, uv, 0.0);
    // data.x = height, data.y = ∂h/∂x, data.z = ∂h/∂z
    return normalize(vec3<f32>(-data.y, 1.0, -data.z));
}

// Pawn: procedural SDF (complex shape, worth computing per-query)
fn sdf_pawn(p: vec3<f32>) -> f32 {
    let p_local = quat_rotate(quat_inverse(render_pawn.orientation), p - render_pawn.pos);
    return prim_round_cone(p_local, PAWN_RADIUS, 0.0, PAWN_HEIGHT);
}

// Scene composition
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
    
    let half_size = render_ground.size * 0.5;
    let in_bounds = abs(p.x) < half_size && abs(p.z) < half_size;
    
    if (in_bounds) {
        let d_ground = sdf_ground(p);
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
// SECTION 10: RENDER — Raymarcher
// ═══════════════════════════════════════════════════════════════════════════════

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
    material: i32,
}

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result: RayHit;
    result.hit = false;
    result.dist = 0.0;
    result.material = MAT_SKY;
    
    var t: f32 = 0.0;
    
    for (var i: i32 = 0; i < MAX_STEPS; i++) {
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
        
        t += scene.dist * 0.8;
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
// SECTION 11: RENDER — Shading (samples precomputed tile colors)
// ═══════════════════════════════════════════════════════════════════════════════

fn get_ray(uv: vec2<f32>) -> vec3<f32> {
    let cos_el = cos(render_camera.elevation);
    let sin_el = sin(render_camera.elevation);
    let cos_az = cos(render_camera.azimuth);
    let sin_az = sin(render_camera.azimuth);
    
    let orbital = vec3<f32>(cos_el * sin_az, sin_el, cos_el * cos_az);
    let look_dir = -orbital;
    let right = vec3<f32>(cos_az, 0.0, -sin_az);
    let up = cross(orbital, right);
    
    let fov_factor = tan(CAMERA_FOV * 0.5);
    return normalize(
        look_dir + 
        right * uv.x * fov_factor * render_signal.aspect_ratio + 
        up * uv.y * fov_factor
    );
}

fn shade_ground(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    // Sample tile color from precomputed texture
    let tile_uv = world_to_tile_uv(pos.xz, render_ground.size);
    let tile = textureSampleLevel(tile_state_read, nearest_sampler, tile_uv, 0.0);
    
    let color = tile.rgb;
    let activation = tile.a;  // For future use
    
    // Lighting
    let ndotl = max(dot(normal, light_dir), 0.0);
    return color * (0.35 + 0.65 * ndotl);
}

fn shade_pawn(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    return COLOR_PAWN * (0.3 + 0.7 * ndotl);
}

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return COLOR_SKY;
    }
    
    let light_dir = normalize(vec3<f32>(0.3, 1.0, 0.2));
    var color: vec3<f32>;
    
    if (hit.material == MAT_GROUND) {
        let normal = ground_normal_from_texture(hit.pos);
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
// SECTION 12: RENDER — Entry Points
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
    
    let cam_pos = render_camera.pos;
    let rd = get_ray(corrected_uv);
    
    let hit = raymarch(cam_pos, rd);
    let color = shade(hit, rd);
    
    return vec4<f32>(color, 1.0);
}
