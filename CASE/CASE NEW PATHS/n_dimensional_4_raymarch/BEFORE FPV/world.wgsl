// ═══════════════════════════════════════════════════════════════════════════════
// N-DIMENSIONAL_4 CARTRIDGE — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// This version merges:
//   - RANDOMNESS organization and documentation style
//   - PGA sphere orbit and color motor logic
//
// ═══════════════════════════════════════════════════════════════════════════════

// SCROLL INDEX:
//   §1    FOUNDATIONS
//   §1.1    PGA Grimorium
//   §1.2    Trajectory Primitives
//   §1.3    Idle State Functions
//   §1.4    Coordinate Systems
//   §1.5    Utilities
//   §1.6    Deterministic Randomness
//   §2    STATE
//   §2.1    Structs
//   §2.2    Constants
//   §2.3    Muting Control
//   §3    COUPLINGS
//   §4    DYNAMICS
//   §5    COMPOSITION
//   §6    OBSERVATION
//   §7    INFRASTRUCTURE

// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 FOUNDATIONS
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// §1.1 PGA GRIMORIUM — Projective Geometric Algebra for WGSL
// ═══════════════════════════════════════════════════════════════════════════════
//
// Algebra: P(ℝ*₃,₀,₁) — 3D Euclidean PGA
// Version: 3.1 — Outer Product Signs Verified via Wedge Derivation
//
// The Motor is the fundamental transformation element. It encodes both
// rotation and translation in a single algebraic object, enabling elegant
// composition via the geometric product.

// ─── PGA TYPES ──────────────────────────────────────────────────────────────────

struct Motor {
    p0: vec4<f32>,  // [s, e23, e31, e12]     — scalar + rotation bivectors
    p1: vec4<f32>,  // [e01, e02, e03, e0123] — translation + pseudoscalar
}

struct Line {
    d: vec3<f32>,   // [e23, e31, e12] — direction/axis
    m: vec3<f32>,   // [e01, e02, e03] — moment
}

struct Plane {
    v: vec4<f32>,   // [e0, e1, e2, e3] = [d, nx, ny, nz]
}

struct Point {
    v: vec4<f32>,   // [e123, e032, e013, e021] = [w, x, y, z]
}

// ─── PGA CONSTANTS ──────────────────────────────────────────────────────────────

const PI: f32 = 3.14159265359;
const EPSILON: f32 = 1e-7;
const MOTOR_IDENTITY: Motor = Motor(vec4<f32>(1.0, 0.0, 0.0, 0.0), vec4<f32>(0.0));
const LINE_Y: Line = Line(vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0)); // Y-axis

// ─── PGA CONSTRUCTORS ───────────────────────────────────────────────────────────

fn point_from_vec3(p: vec3<f32>) -> Point {
    return Point(vec4<f32>(1.0, p.x, p.y, p.z));
}

fn point_to_vec3(p: Point) -> vec3<f32> {
    let w = p.v.x;
    if (abs(w) < EPSILON) { return vec3<f32>(0.0); }
    return p.v.yzw / w;
}

fn rotor(axis: vec3<f32>, angle: f32) -> Motor {
    // Creates a Motor that rotates around the given axis by the given angle.
    // The axis passes through the origin.
    let len_sq = dot(axis, axis);
    if (len_sq < EPSILON) { return MOTOR_IDENTITY; }
    let half_angle = angle * 0.5;
    let n = axis * inverseSqrt(len_sq);
    let c = cos(half_angle);
    let s = sin(half_angle);
    // Negative s for counter-clockwise convention
    return Motor(vec4<f32>(c, -s * n.x, -s * n.y, -s * n.z), vec4<f32>(0.0));
}

fn translator(dir: vec3<f32>, dist: f32) -> Motor {
    // Creates a Motor that translates along the given direction by the given distance.
    let len_sq = dot(dir, dir);
    if (len_sq < EPSILON) { return MOTOR_IDENTITY; }
    let n = dir * inverseSqrt(len_sq);
    let half_dist = dist * (-0.5); // Negative for correct sandwich product
    return Motor(vec4<f32>(1.0, 0.0, 0.0, 0.0), vec4<f32>(half_dist * n, 0.0));
}

// ─── PGA OPERATIONS ─────────────────────────────────────────────────────────────

fn reverse_m(m: Motor) -> Motor {
    // Reversal: negates all bivector components
    return Motor(
        vec4<f32>(m.p0.x, -m.p0.y, -m.p0.z, -m.p0.w),
        vec4<f32>(-m.p1.x, -m.p1.y, -m.p1.z, m.p1.w)
    );
}

fn gp_mm(a: Motor, b: Motor) -> Motor {
    // Geometric Product (Motor * Motor)
    // This is how transformations compose in PGA.
    let p0 = vec4<f32>(
        a.p0.x * b.p0.x - dot(a.p0.yzw, b.p0.yzw),
        a.p0.x * b.p0.yzw + b.p0.x * a.p0.yzw + cross(a.p0.yzw, b.p0.yzw)
    );
    let p1 = vec4<f32>(
        a.p0.x * b.p1.xyz + b.p0.x * a.p1.xyz + cross(a.p0.yzw, b.p1.xyz) + cross(a.p1.xyz, b.p0.yzw) - b.p1.w * a.p0.yzw - a.p1.w * b.p0.yzw,
        a.p0.x * b.p1.w + b.p0.x * a.p1.w + dot(a.p0.yzw, b.p1.xyz) + dot(a.p1.xyz, b.p0.yzw)
    );
    return Motor(p0, p1);
}

fn sw_mp(m: Motor, p: vec3<f32>) -> vec3<f32> {
    // Sandwich Product (Motor * Point * ~Motor) — optimized for vec3 input
    let t = cross(p, m.p0.yzw) - m.p1.xyz;
    return (m.p0.x * t + cross(t, m.p0.yzw) - m.p0.yzw * m.p1.w) * 2.0 + p;
}

fn sw_motor_point(m: Motor, p: Point) -> Point {
    // Sandwich Product for Point type
    let pos = point_to_vec3(p);
    let transformed = sw_mp(m, pos);
    return point_from_vec3(transformed);
}

fn verify_motor_norm(m: Motor) -> vec2<f32> {
    // Verification: A valid motor satisfies m * ~m = 1
    // Returns (scalar_error, pseudoscalar_error)
    let product = gp_mm(m, reverse_m(m));
    return vec2<f32>(abs(product.p0.x - 1.0), abs(product.p1.w));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §1.2 TRAJECTORY PRIMITIVES
// ═══════════════════════════════════════════════════════════════════════════════

struct Trajectory {
    value: f32,
    velocity: f32,
    _pad0: f32,
    _pad1: f32,
}

fn trajectory_spring(t: Trajectory, goal: f32, dt: f32, stiffness: f32, damping: f32) -> Trajectory {
    let displacement = t.value - goal;
    let spring_force = -stiffness * displacement;
    let damping_force = -damping * sqrt(stiffness) * t.velocity;
    let accel = spring_force + damping_force;
    let new_vel = t.velocity + accel * dt;
    let new_val = t.value + new_vel * dt;
    return Trajectory(new_val, new_vel, 0.0, 0.0);
}

fn trajectory_release(t: Trajectory, goal: f32, dt: f32, rate: f32) -> Trajectory {
    let new_val = t.value + (goal - t.value) * (1.0 - exp(-rate * dt));
    return Trajectory(new_val, 0.0, 0.0, 0.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §1.3 IDLE STATE FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════

const IDLE_BASE_COLOR: vec3<f32> = vec3(0.55, 0.45, 0.35);
const IDLE_CHECKER_LIGHT: f32 = 1.15;
const IDLE_CHECKER_DARK: f32 = 0.85;

fn idle_color(cell_id: vec2<u32>) -> vec3<f32> {
    // Checkerboard pattern — artist's deliberate choice
    let checker = (cell_id.x + cell_id.y) & 1u;
    let factor = select(IDLE_CHECKER_DARK, IDLE_CHECKER_LIGHT, checker == 0u);
    return IDLE_BASE_COLOR * factor;
}

fn idle_height_mod(cell_id: vec2<u32>) -> f32 {
    // Flat — no pattern (height modulation rests at zero)
    return 0.0;
}

fn idle_emission(cell_id: vec2<u32>) -> f32 {
    // Radial gradient from center — subtle glow at center
    let center = vec2<f32>(f32(CELL_GRID_SIZE) * 0.5);
    let dist = length(vec2<f32>(cell_id) - center) / f32(CELL_GRID_SIZE);
    return 0.05 * (1.0 - dist);
}

fn cell_idle(cell_id: vec2<u32>) -> vec4<f32> {
    // Combined idle lookup — packs all channels
    let color = idle_color(cell_id);
    let height = idle_height_mod(cell_id);
    // Currently: rgb=color, a=height_mod
    // Future: could pack emission into unused bits or add more channels
    return vec4(color, height);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §1.4 COORDINATE SYSTEMS
// ═══════════════════════════════════════════════════════════════════════════════

const CHART_EXTENT: f32 = 100.0;
const CHART_HEIGHT_RESOLUTION: u32 = 256u;
const CHART_SURFACE_COLOR_RESOLUTION: u32 = 64u;

fn chart_world_to_uv(world_xz: vec2<f32>) -> vec2<f32> {
    return (world_xz / CHART_EXTENT) + 0.5;
}

fn chart_uv_to_world(uv: vec2<f32>) -> vec2<f32> {
    return (uv - 0.5) * CHART_EXTENT;
}

fn chart_clamp(pos: vec2<f32>) -> vec2<f32> {
    let half = CHART_EXTENT * 0.5 - 1.0;
    return clamp(pos, vec2(-half), vec2(half));
}

fn chart_height_texel_to_world(texel: vec2<u32>) -> vec2<f32> {
    let uv = vec2<f32>(texel) / f32(CHART_HEIGHT_RESOLUTION);
    return chart_uv_to_world(uv);
}

fn chart_surface_color_texel_to_world(texel: vec2<u32>) -> vec2<f32> {
    let uv = vec2<f32>(texel) / f32(CHART_SURFACE_COLOR_RESOLUTION);
    return chart_uv_to_world(uv);
}

const CELL_GRID_SIZE: u32 = 64u;
const MAX_CELL_GRID_SIZE: u32 = 64u;

fn cell_index(texel: vec2<u32>) -> u32 {
    return texel.y * CELL_GRID_SIZE + texel.x;
}

fn cell_index_dynamic(texel: vec2<u32>, active_size: u32) -> u32 {
    // For dynamic resolution: index within active region
    return texel.y * active_size + texel.x;
}

fn proximity_field_index(texel: vec2<u32>) -> u32 {
    return texel.y * PROXIMITY_FIELD_SIZE + texel.x;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §1.5 UTILITIES
// ═══════════════════════════════════════════════════════════════════════════════

fn hsv_to_rgb(hsv: vec3<f32>) -> vec3<f32> {
    let h = hsv.x * 6.0;
    let s = hsv.y;
    let v = hsv.z;
    
    let c = v * s;
    let x = c * (1.0 - abs(fract(h / 2.0) * 2.0 - 1.0));
    let m = v - c;
    
    var rgb: vec3<f32>;
    let hi = i32(floor(h)) % 6;
    
    if (hi == 0) { rgb = vec3(c, x, 0.0); }
    else if (hi == 1) { rgb = vec3(x, c, 0.0); }
    else if (hi == 2) { rgb = vec3(0.0, c, x); }
    else if (hi == 3) { rgb = vec3(0.0, x, c); }
    else if (hi == 4) { rgb = vec3(x, 0.0, c); }
    else { rgb = vec3(c, 0.0, x); }
    
    return rgb + m;
}

fn rgb_to_hsv(rgb: vec3<f32>) -> vec3<f32> {
    let max_c = max(max(rgb.r, rgb.g), rgb.b);
    let min_c = min(min(rgb.r, rgb.g), rgb.b);
    let delta = max_c - min_c;
    
    var h: f32 = 0.0;
    var s: f32 = 0.0;
    let v: f32 = max_c;
    
    if (delta > 0.00001) {
        s = delta / max_c;
        
        if (rgb.r >= max_c) {
            h = (rgb.g - rgb.b) / delta;
        } else if (rgb.g >= max_c) {
            h = 2.0 + (rgb.b - rgb.r) / delta;
        } else {
            h = 4.0 + (rgb.r - rgb.g) / delta;
        }
        
        h = h / 6.0;
        if (h < 0.0) { h = h + 1.0; }
    }
    
    return vec3(h, s, v);
}


fn quat_from_axis_angle(axis: vec3<f32>, angle: f32) -> vec4<f32> {
    let half_angle = angle * 0.5;
    return vec4(axis * sin(half_angle), cos(half_angle));
}

fn quat_multiply(a: vec4<f32>, b: vec4<f32>) -> vec4<f32> {
    return vec4(
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
    return vec4(-q.xyz, q.w);
}

fn quat_from_forward(forward: vec3<f32>) -> vec4<f32> {
    // Builds quaternion that rotates +Z axis to align with forward
    let world_forward = vec3(0.0, 0.0, 1.0);
    
    let d = dot(world_forward, forward);
    
    // Handle parallel cases
    if (d > 0.9999) {
        return vec4(0.0, 0.0, 0.0, 1.0);  // Identity
    }
    if (d < -0.9999) {
        return vec4(0.0, 1.0, 0.0, 0.0);  // 180° around Y
    }
    
    // General case: axis-angle from cross product
    let axis = normalize(cross(world_forward, forward));
    let angle = acos(clamp(d, -1.0, 1.0));
    return quat_from_axis_angle(axis, angle);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §1.6 DETERMINISTIC RANDOMNESS
// ═══════════════════════════════════════════════════════════════════════════════

fn hash_cell_id(cell_id: vec2<u32>) -> u32 {
    // Wang hash - fast, deterministic pseudo-random distribution
    var h = cell_id.x;
    h = (h ^ 61u) ^ (h >> 16u);
    h = h + (h << 3u);
    h = h ^ (h >> 4u);
    h = h * 0x27d4eb2du;
    h = h ^ (h >> 15u);
    
    var v = cell_id.y;
    v = (v ^ 61u) ^ (v >> 16u);
    v = v * 0x27d4eb2du;
    
    return h ^ v;
}

fn random_float(cell_id: vec2<u32>, seed: u32) -> f32 {
    // Returns [0, 1] deterministically for this cell
    let hash = hash_cell_id(cell_id) ^ seed;
    return f32(hash) / 4294967295.0;
}

fn random_float_range(cell_id: vec2<u32>, seed: u32, min_val: f32, max_val: f32) -> f32 {
    // Returns [min_val, max_val]
    let t = random_float(cell_id, seed);
    return min_val + t * (max_val - min_val);
}

fn random_vec3(cell_id: vec2<u32>, seed: u32) -> vec3<f32> {
    // Independent random for each channel
    return vec3(
        random_float(cell_id, seed),
        random_float(cell_id, seed + 1u),
        random_float(cell_id, seed + 2u)
    );
}

fn random_vec3_range(cell_id: vec2<u32>, seed: u32, min_val: vec3<f32>, max_val: vec3<f32>) -> vec3<f32> {
    let t = random_vec3(cell_id, seed);
    return min_val + t * (max_val - min_val);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 STATE
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// §2.1 STRUCTS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [STATE:signal] FrameSignal ───────────────────────────────────────────────

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

// ─── [STATE:terrain] TerrainState ─────────────────────────────────────────────

struct TerrainState {
    amplitude_scale: f32,
    max_amplitude: f32,
    size: f32,
    lipschitz_factor: f32,
    tint: vec3<f32>,
    _pad: f32,
}

// ─── [STATE:pawn] PawnState ───────────────────────────────────────────────────

struct PawnState {
    pos: vec3<f32>,
    heading: f32,
    orientation: vec4<f32>,
    velocity: vec2<f32>,      // Current XZ velocity (for force field radius)
    _pad: vec2<f32>,          // Padding for alignment
}

// ─── [STATE:camera] CameraState ───────────────────────────────────────────────

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

// ─── [STATE:sphere] SphereState ───────────────────────────────────────────────

struct SphereState {
    pos: vec3<f32>,
    radius: f32,
    orientation: vec4<f32>,
    influence_radius: f32,
    t: f32,                    // curve parameter (advances when not frozen)
    _pad1: f32,
    _pad2: f32,
    color: vec3<f32>,          // current appearance (driven by polyphony via PGA motor)
    _pad3: f32,
}

// ─── [STATE:cells] CellState ──────────────────────────────────────────────────

struct CellState {
    value: vec4<f32>,       // .rgb = color, .a = height_mod
    velocity: vec4<f32>,    // rate of change per channel
    goal: vec4<f32>,        // stimulus target per channel
    is_active: f32,         // 1.0 = spring, 0.0 = release
    life: f32,              // Game of Life: 0.0 = dead, 1.0 = alive (display state)
    life_next: f32,         // Game of Life: next generation buffer (double-buffering)
    _pad2: f32,
}

// ─── [STATE:rendering] Hit detection ──────────────────────────────────────────

struct SceneHit {
    dist: f32,       // TRUE Euclidean distance (for hit detection and comparison)
    step_dist: f32,  // Conservative bound for safe ray stepping
    material: i32,
}

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
    material: i32,
}

// ─── [STATE:config] DesignConfig ──────────────────────────────────────────────

struct DesignConfig {
    mute_dynamics_0d: u32,
    mute_dynamics_2d: u32,
    mute_signal: u32,
    mute_couplings: u32,
    wave_time_scale: f32,
    pawn_speed: f32,
    camera_sensitivity: f32,
    freeze_sphere: u32,
    active_cell_size: f32,    // Animatable: 8.0 → 64.0 for resolution effects
    _cfg_pad0: f32,
    _cfg_pad1: f32,
    _cfg_pad2: f32,
}

// ═══════════════════════════════════════════════════════════════════════════════
// §2.2 CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Terrain constants ────────────────────────────────────────────────────────

struct WaveComponent {
    amplitude: f32,
    frequency: f32,
    dir: vec2<f32>,
    period: f32,
}

const WAVES = array<WaveComponent, 3>(
    WaveComponent(1.0, 0.15, vec2(0.7, 0.7),   8.0),
    WaveComponent(0.5, 0.30, vec2(-0.5, 0.8),  6.0),
    WaveComponent(0.3, 0.50, vec2(0.9, -0.3),  4.0),
);
const WAVE_COUNT: i32 = 3;
const HEIGHT_MAX_AMPLITUDE: f32 = 2.0;
const IDLE_AMPLITUDE_SCALE: f32 = 1.0;
const AMPLITUDE_ATTACK_TIME: f32 = 10.0;
const AMPLITUDE_RELEASE_TIME: f32 = 5.0;
const SAND_DUNE_CENTER: vec3<f32> = vec3(0.85, 0.70, 0.50);
const SAND_DUNE_VARIANCE: f32 = 0.35;

// ─── Pawn constants ───────────────────────────────────────────────────────────

const PAWN_HEIGHT: f32 = 1.5;
const PAWN_RADIUS: f32 = 0.5;
const PAWN_TIP: f32 = 0.0;
const PAWN_SPEED: f32 = 15.0;
const PAWN_TURN_SPEED: f32 = 8.0;
const PAWN_INFLUENCE_RADIUS: f32 = 8.0;

// ─── Camera constants ─────────────────────────────────────────────────────────

const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_DISTANCE: f32 = 5.0;
const CAMERA_MAX_DISTANCE: f32 = 100.0;
const CAMERA_MIN_ELEVATION: f32 = -0.5;
const CAMERA_MAX_ELEVATION: f32 = 1.5;

// ─── Sphere constants ─────────────────────────────────────────────────────────

const SPHERE_RADIUS: f32 = 2.5;
const SPHERE_BASE_COLOR: vec3<f32> = vec3(0.95, 0.75, 0.4);
const SPHERE_EXCITED_COLOR: vec3<f32> = vec3(1.0, 0.45, 0.55);
const SPHERE_COLOR_ATTACK_RATE: f32 = 6.0;
const SPHERE_COLOR_RELEASE_RATE: f32 = 2.0;
const SPHERE_INFLUENCE_RADIUS: f32 = 12.0;
const CURVE_ORBIT_RADIUS: f32 = 25.0;
const CURVE_ORBIT_HEIGHT: f32 = 8.0;
const CURVE_ORBIT_SPEED: f32 = 0.8;
const SPHERE_MIN_TERRAIN_CLEARANCE: f32 = 5.0;

// ─── Proximity field constants ────────────────────────────────────────────────

const PROXIMITY_FIELD_SIZE: u32 = 64u;
const IDLE_PROXIMITY_OFFSET: f32 = 0.0;
const PROXIMITY_FIELD_ATTACK_STIFFNESS: f32 = 12.0;
const PROXIMITY_FIELD_ATTACK_DAMPING: f32 = 0.7;
const PROXIMITY_FIELD_RELEASE_RATE: f32 = 1.5;
const PROXIMITY_FIELD_MAX_OFFSET: f32 = 0.5;
const PROXIMITY_SHIFT_PAWN: vec3<f32> = vec3(0.4, 0.2, 0.5);
const PROXIMITY_SHIFT_SPHERE: vec3<f32> = vec3(0.5, 0.35, 0.0);

// ─── Cell constants ───────────────────────────────────────────────────────────

const CELL_SPRING_STIFFNESS: f32 = 8.0;
const CELL_SPRING_DAMPING: f32 = 0.7;
const CELL_RELEASE_RATE: f32 = 2.0;
const GOL_COLOR_ALIVE: vec3<f32> = vec3(0.0, 0.0, 0.0);

// Cell randomness (organic per-cell variation when stimulated)
const CELL_RANDOM_COLOR_AMOUNT: vec3<f32> = vec3(0.35, 0.30, 0.25);
const CELL_RANDOM_HEIGHT_AMOUNT: f32 = 0.5;
const CELL_RANDOM_SEED_COLOR: u32 = 42u;
const CELL_RANDOM_SEED_HEIGHT: u32 = 7u;
const CELL_RANDOM_MODE_HUE_SHIFT: bool = true;
const CELL_RANDOM_HUE_RANGE: f32 = 1.0;

// Temporal behavior (breathing with music)
const CELL_RANDOM_TEMPORAL_PATTERN_CHANGE: bool = false;
const CELL_RANDOM_TEMPORAL_FREQUENCY: f32 = 0.5;
const CELL_RANDOM_TEMPORAL_HUE_CYCLE: bool = true;
const CELL_RANDOM_HUE_CYCLE_SPEED: f32 = 0.1;
const CELL_RANDOM_HUE_CYCLE_FORCE_ACTIVE: bool = false;

// DEBUG: Ultra-visible oscillation (overrides all other color logic)
const CELL_RANDOM_DEBUG_OSCILLATION: bool = false;
const CELL_RANDOM_DEBUG_OSC_SPEED: f32 = 0.1;
const CELL_RANDOM_DEBUG_COLOR_A: vec3<f32> = vec3(0.0, 0.0, 0.0);
const CELL_RANDOM_DEBUG_COLOR_B: vec3<f32> = vec3(1.0, 1.0, 1.0);

// ─── Cell Height Geometry (separate from wave terrain) ───────────────────────
//
// Cells can raise/lower terrain independently of waves.
// Style determines sampling: blocky (nearest), smooth (bilinear), or tower (peaks).

const CELL_HEIGHT_STYLE_BLOCKY: u32 = 0u;    // Rectangular blocks (Minecraft-like)
const CELL_HEIGHT_STYLE_SMOOTH: u32 = 1u;    // Bilinear interpolation (gentle hills)
const CELL_HEIGHT_STYLE_TOWER: u32 = 2u;     // Sharp peaks at cell centers
const CELL_HEIGHT_STYLE_VOXEL: u32 = 3u;     // True 3D boxes on wave surface (exact SDF)

const CELL_HEIGHT_ENABLED: bool = false;      // Master switch for cell height geometry
const CELL_HEIGHT_STYLE: u32 = CELL_HEIGHT_STYLE_BLOCKY;  // Active style

const CELL_HEIGHT_BASE: f32 = 0.0;           // Base height contribution when dead
const CELL_HEIGHT_ALIVE: f32 = 2.0;          // Added height when alive (GoL)
const CELL_HEIGHT_STIMULUS: f32 = 1.0;       // Multiplier for stimulus-driven height (cell.value.a)
const CELL_HEIGHT_RANDOM_RANGE: f32 = 0.5;   // Per-cell random variation
const CELL_HEIGHT_SCALE: f32 = 1.0;          // Global multiplier in SDF blend

// Precomputed Lipschitz adjustment (avoids per-ray-step multiplication)
const CELL_HEIGHT_LIPSCHITZ_ADJUST: f32 = 1.0 + CELL_HEIGHT_SCALE * 2.0;

// ─── Pawn Safety Force Field ─────────────────────────────────────────────────
//
// The pawn creates a "calm zone" where cell heights are suppressed.
// This lets the pawn walk smoothly over terrain without being jolted by GoL blocks.
// The force field shrinks when the pawn moves fast, leaving a trail of disturbance.

const PAWN_FORCEFIELD_ENABLED: bool = true;
const PAWN_FORCEFIELD_RADIUS_STATIONARY: f32 = 6.0;  // Radius when not moving
const PAWN_FORCEFIELD_RADIUS_MOVING: f32 = 2.0;      // Radius at max speed
const PAWN_FORCEFIELD_FALLOFF: f32 = 2.0;            // Edge softness (smoothstep width)
const PAWN_FORCEFIELD_SPEED_SCALE: f32 = 1.0;        // How quickly radius shrinks with speed

// ─── Game of Life Timing ─────────────────────────────────────────────────────

const GOL_TICKS_PER_BEAT: f32 = 2.0;         // How many GoL generations per musical beat
                                              // 1.0 = once per beat, 2.0 = twice per beat, etc.

// ─── Rendering constants ──────────────────────────────────────────────────────

const COLOR_SKY: vec3<f32> = vec3(0.85, 0.78, 0.72);
const COLOR_FOG: vec3<f32> = vec3(0.85, 0.78, 0.72);
const COLOR_PAWN: vec3<f32> = vec3(0.8, 0.5, 0.8);
const TERRAIN_BASE_COLOR: vec3<f32> = vec3(0.55, 0.45, 0.35);
const MAT_TERRAIN: i32 = 1;
const MAT_PAWN: i32 = 2;
const MAT_SPHERE: i32 = 3;
const MAT_SKY: i32 = 0;
const MAX_STEPS: i32 = 512;
const MAX_DIST: f32 = 200.0;
const SURF_DIST: f32 = 0.05;
const RAYMARCH_STEP_FACTOR: f32 = 1.0;   // Step multiplier (step_dist is already conservative)

// ═══════════════════════════════════════════════════════════════════════════════
// §2.3 MUTING CONTROL
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Coupling bit flags ───────────────────────────────────────────────────────

const COUPLING_POLYPHONY_TO_AMPLITUDE:       u32 = 1u << 0u;
const COUPLING_TERRAIN_TO_PAWN_Y:            u32 = 1u << 1u;
const COUPLING_TERRAIN_TO_PAWN_TILT:         u32 = 1u << 2u;
const COUPLING_PAWN_TO_CAMERA_TARGET:        u32 = 1u << 3u;
const COUPLING_INPUT_MOVES_PAWN:             u32 = 1u << 4u;
const COUPLING_INPUT_ORBITS_CAMERA:          u32 = 1u << 5u;
const COUPLING_INPUT_ZOOMS_CAMERA:           u32 = 1u << 6u;
const COUPLING_PAWN_TO_PROXIMITY_FIELD:      u32 = 1u << 7u;
const COUPLING_SPHERE_TO_PROXIMITY_FIELD:    u32 = 1u << 8u;
const COUPLING_POLYPHONY_TO_CELL_COLOR:      u32 = 1u << 9u;
const COUPLING_PAWN_TO_CELL_COLOR:           u32 = 1u << 10u;
const COUPLING_SPHERE_TO_CELL_COLOR:         u32 = 1u << 11u;
const COUPLING_POLYPHONY_TO_SPHERE_COLOR:    u32 = 1u << 12u;
const COUPLING_SPHERE_TO_TERRAIN_TINT:       u32 = 1u << 13u;
const COUPLING_TERRAIN_TO_SPHERE_HEIGHT:     u32 = 1u << 14u;
const COUPLING_RANDOM_TO_CELL_GOALS:         u32 = 1u << 15u;

// ─── Muting query functions ───────────────────────────────────────────────────

fn coupling_active(bit: u32) -> bool {
    return (config.mute_couplings & bit) == 0u;
}

fn dynamics_0d_active() -> bool {
    return config.mute_dynamics_0d == 0u;
}

fn dynamics_2d_active() -> bool {
    return config.mute_dynamics_2d == 0u;
}

fn signal_active() -> bool {
    return config.mute_signal == 0u;
}

fn sphere_frozen() -> bool {
    return config.freeze_sphere != 0u;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 COUPLINGS
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// §3.1 signal → terrain
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:signal.polyphony→terrain:amplitude] ──────────────────────────────────────

fn coupling_signal_polyphony_to_terrain_amplitude(polyphony: f32, traj: Trajectory, dt: f32) -> Trajectory {
    let goal = IDLE_AMPLITUDE_SCALE * (1.0 + polyphony/4);
    
    if (polyphony > 0.0) {
        // Attack: fast exponential approach
        let rate = 3.0 / AMPLITUDE_ATTACK_TIME;
        let new_val = traj.value + (goal - traj.value) * (1.0 - exp(-rate * dt));
        return Trajectory(new_val, 0.0, 0.0, 0.0);
    } else {
        // Release: slow exponential decay
        let rate = 3.0 / AMPLITUDE_RELEASE_TIME;
        return trajectory_release(traj, IDLE_AMPLITUDE_SCALE, dt, rate);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// §3.2 signal → entities
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:signal.polyphony→sphere:color] ───────────────────────────────────────────
//
// PGA COLOR MOTOR: Uses Motor algebra to create organic spiraling through color space.
// The sphere's color doesn't just interpolate — it TWISTS around the luminance axis,
// creating rich, evolving hues that feel musical rather than mechanical.

fn coupling_signal_polyphony_to_sphere_color(polyphony: f32, current: vec3<f32>, dt: f32) -> vec3<f32> {
    let intensity = saturate(polyphony / 8.0);
    
    // ─── HYBRID APPROACH ─────────────────────────────────────────────────────
    //   Silent: Exponential decay back to base color (resting state)
    //   Active: PGA motor spiral through color space (musical response)
    
    if (intensity < 0.01) {
        // Silent: smooth return to base color using existing release rate
        return current + (SPHERE_BASE_COLOR - current) * (1.0 - exp(-SPHERE_COLOR_RELEASE_RATE * dt));
    }
    
    // Active: PGA spiral (rotation scales with intensity, no idle drift)
    let hue_speed = intensity * 2.5;
    let sat_push = intensity * 0.5;
    let val_climb = intensity * 0.2;
    
    return pga_color_motor(current, hue_speed, sat_push, val_climb, dt);
}

// ═══════════════════════════════════════════════════════════════════════════════
// §3.3 input → entities
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:input→pawn:velocity] ───────────────────────────────────────────

fn coupling_input_to_pawn_velocity(input_dir: vec2<f32>, camera_azimuth: f32) -> vec2<f32> {
    let cos_az = cos(camera_azimuth);
    let sin_az = sin(camera_azimuth);
    return vec2(
        input_dir.x * cos_az + input_dir.y * sin_az,
        -input_dir.x * sin_az + input_dir.y * cos_az
    );
}

// ─── [COUPLING:input→camera:orbit] ────────────────────────────────────────────

fn coupling_input_to_camera_orbit(az_delta: f32, el_delta: f32, cam: CameraState) -> CameraState {
    var c = cam;
    c.azimuth += az_delta;
    c.elevation = clamp(c.elevation + el_delta, CAMERA_MIN_ELEVATION, CAMERA_MAX_ELEVATION);
    return c;
}

// ─── [COUPLING:input→camera:distance] ─────────────────────────────────────────

fn coupling_input_to_camera_distance(zoom_delta: f32, cam: CameraState) -> CameraState {
    var c = cam;
    c.distance = clamp(c.distance + zoom_delta, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
    return c;
}

// ─── [COUPLING:input→camera:pan] ──────────────────────────────────────────────

fn coupling_input_to_camera_pan(pan_delta: vec2<f32>, cam: CameraState) -> CameraState {
    var c = cam;
    c.pan_x += pan_delta.x * cam.distance * 0.5;
    c.pan_y += pan_delta.y * cam.distance * 0.5;
    return c;
}

// ═══════════════════════════════════════════════════════════════════════════════
// §3.4 terrain → entities
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:pawn_forcefield] ──────────────────────────────────────────────
//
// Computes the force field suppression factor at a world position.
// Returns 0.0 at pawn center (full suppression), 1.0 outside field (no suppression).

fn coupling_pawn_forcefield_factor(world_xz: vec2<f32>) -> f32 {
    if (!PAWN_FORCEFIELD_ENABLED) {
        return 1.0;  // No suppression
    }
    
    // Distance from pawn
    let pawn_dist = length(world_xz - pawn_state.pos.xz);
    
    // Force field radius based on pawn speed
    // Fast movement = smaller radius, slow/stationary = larger radius
    let pawn_speed = length(pawn_state.velocity);
    let speed_factor = saturate(pawn_speed * PAWN_FORCEFIELD_SPEED_SCALE / PAWN_SPEED);
    let radius = mix(PAWN_FORCEFIELD_RADIUS_STATIONARY, PAWN_FORCEFIELD_RADIUS_MOVING, speed_factor);
    
    // Smooth falloff from center to edge
    // 0.0 at center → 1.0 at radius + falloff
    return smoothstep(radius - PAWN_FORCEFIELD_FALLOFF, radius + PAWN_FORCEFIELD_FALLOFF, pawn_dist);
}

// ─── [COUPLING:cells→terrain:height] ─────────────────────────────────────────
//
// Returns cell-driven height contribution at a world position.
// Samples from terrain_cells buffer (not texture) so it works in update_world.
// Suppressed within the pawn's safety force field.

fn coupling_cells_to_terrain_height(world_xz: vec2<f32>) -> f32 {
    if (!CELL_HEIGHT_ENABLED) {
        return 0.0;
    }
    
    let active_size = u32(config.active_cell_size);
    if (active_size == 0u) {
        return 0.0;
    }
    
    // Map world position to cell coordinates
    let normalized = (world_xz / CHART_EXTENT) + 0.5;  // [0, 1]
    let cell_f = normalized * f32(active_size);
    let cell_i = vec2<u32>(clamp(vec2<i32>(floor(cell_f)), vec2(0), vec2(i32(active_size) - 1)));
    
    // Sample cell from buffer
    let idx = cell_i.y * CELL_GRID_SIZE + cell_i.x;
    let cell = terrain_cells[idx];
    
    // Base cell height (trajectory-animated)
    let cell_height = (CELL_HEIGHT_BASE + cell.value.a * CELL_HEIGHT_STIMULUS) * CELL_HEIGHT_SCALE;
    
    // Apply pawn force field suppression
    let forcefield = coupling_pawn_forcefield_factor(world_xz);
    
    return cell_height * forcefield;
}

// ─── [COUPLING:terrain→pawn:height] ───────────────────────────────────────

fn coupling_terrain_to_pawn_height(pawn_xz: vec2<f32>, t: f32, amplitude_scale: f32) -> f32 {
    let wave_height = dynamics_terrain_wave_eval(pawn_xz, t, amplitude_scale);
    let cell_height = coupling_cells_to_terrain_height(pawn_xz);
    return wave_height + cell_height;
}

// ─── [COUPLING:terrain→pawn:orientation] ──────────────────────────────────────

fn coupling_terrain_to_pawn_orientation(pawn_xz: vec2<f32>, heading: f32, t: f32, amplitude_scale: f32) -> vec4<f32> {
    let normal = dynamics_terrain_normal(pawn_xz, t, amplitude_scale);
    let world_up = vec3(0.0, 1.0, 0.0);
    let dot_val = dot(world_up, normal);
    
    var tilt_quat: vec4<f32>;
    if (dot_val < 0.9999) {
        let axis = normalize(cross(world_up, normal));
        let angle = acos(clamp(dot_val, -1.0, 1.0));
        tilt_quat = quat_from_axis_angle(axis, angle);
    } else {
        tilt_quat = vec4(0.0, 0.0, 0.0, 1.0);
    }
    
    let heading_quat = quat_from_axis_angle(vec3(0.0, 1.0, 0.0), heading);
    return quat_multiply(tilt_quat, heading_quat);
}

// ─── [COUPLING:terrain→sphere:orbit_height] ──────────────────────────────

fn coupling_terrain_to_sphere_orbit_height(sphere_xz: vec2<f32>, base_height: f32) -> f32 {
    if (!coupling_active(COUPLING_TERRAIN_TO_SPHERE_HEIGHT)) {
        return base_height;
    }
    
    // Wave height (analytical)
    let wave_height = dynamics_terrain_wave_eval(
        sphere_xz,
        signal.t_beats,
        terrain_state.amplitude_scale
    );
    
    // Cell height contribution (from cells buffer)
    let cell_height = coupling_cells_to_terrain_height(sphere_xz);
    
    // Combined terrain height
    let terrain_height = wave_height + cell_height;
    
    // Ensure minimum clearance above terrain
    let min_height = terrain_height + SPHERE_MIN_TERRAIN_CLEARANCE;
    
    return max(base_height, min_height);
}

// ═══════════════════════════════════════════════════════════════════════════════
// §3.5 entities → entities
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:pawn→camera:target] ────────────────────────────────────────────

fn coupling_pawn_to_camera_target(pawn_pos: vec3<f32>, cam: CameraState) -> vec3<f32> {
    return compose_camera_position_from_orbit(pawn_pos, cam);
}

// ─── [COUPLING:velocity→pawn:heading] ─────────────────────────────────────────

fn coupling_velocity_to_pawn_heading(velocity: vec2<f32>, current_heading: f32, dt: f32) -> f32 {
    let speed_sq = dot(velocity, velocity);
    if (speed_sq < 0.001) {
        return current_heading;
    }
    
    let goal_heading = atan2(velocity.x, velocity.y);
    var diff = goal_heading - current_heading;
    
    if (diff > 3.14159) { diff -= 6.28318; }
    else if (diff < -3.14159) { diff += 6.28318; }
    
    let max_turn = PAWN_TURN_SPEED * dt;
    var new_heading = current_heading + clamp(diff, -max_turn, max_turn);
    
    if (new_heading > 3.14159) { new_heading -= 6.28318; }
    else if (new_heading < -3.14159) { new_heading += 6.28318; }
    
    return new_heading;
}

// ═══════════════════════════════════════════════════════════════════════════════
// §3.6 entities → fields
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:pawn→field:offset] ───────────────────────────────────────

fn coupling_pawn_to_proximity_field(world_pos: vec2<f32>, pawn_xz: vec2<f32>) -> f32 {
    let dist = distance(world_pos, pawn_xz);
    return smoothstep(PAWN_INFLUENCE_RADIUS, 0.0, dist);
}

// ─── [COUPLING:sphere→field:offset] ─────────────────────────────────────

fn coupling_sphere_to_proximity_field(world_pos: vec2<f32>, terrain_height: f32, sphere: SphereState) -> f32 {
    // Tile position in 3D (on terrain surface)
    let pos_3d = vec3(world_pos.x, terrain_height, world_pos.y);
    
    // 3D distance to sphere center
    let dist_3d = distance(pos_3d, sphere.pos);
    
    // Influence falls off with 3D distance
    return smoothstep(sphere.influence_radius, 0.0, dist_3d);
}

// ─── [COUPLING:sphere→terrain:tint] ───────────────────────────────────────────

fn coupling_sphere_to_terrain_tint(sphere_pos: vec3<f32>) -> vec3<f32> {
    // Normalize position to [-1, 1] based on orbit radius
    let nx = sphere_pos.x / CURVE_ORBIT_RADIUS;
    let nz = sphere_pos.z / CURVE_ORBIT_RADIUS;
    
    // Map sphere position to RGB offsets within variance bounds
    let offset = vec3(
        nx * SAND_DUNE_VARIANCE,
        nz * SAND_DUNE_VARIANCE * 0.5,
        -nx * SAND_DUNE_VARIANCE * 0.6
    );
    
    return clamp(SAND_DUNE_CENTER + offset, vec3(0.0), vec3(1.0));
}

// ═══════════════════════════════════════════════════════════════════════════════
// §3.7 combinatorial → cells
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [COUPLING:signal.polyphony→cells:color] ─────────────────────────────────────────

fn coupling_signal_polyphony_to_cells_color(poly: f32) -> vec3<f32> {
    let base_hue = 0.08;
    let rotation_rate = 0.05;
    let hue = fract(base_hue + poly * rotation_rate);
    let saturation = 0.45;
    let value = 0.75;
    return hsv_to_rgb(vec3(hue, saturation, value));
}

// ─── [COUPLING:pawn→cells:influence] ──────────────────────────────────────────

fn coupling_pawn_to_cells_influence(world_pos: vec2<f32>, pawn_xz: vec2<f32>) -> f32 {
    let dist = distance(world_pos, pawn_xz);
    return smoothstep(PAWN_INFLUENCE_RADIUS, 0.0, dist);
}

// ─── [COUPLING:sphere→cells:influence] ────────────────────────────────────────

fn coupling_sphere_to_cells_influence(world_pos: vec2<f32>, terrain_height: f32, sphere: SphereState) -> f32 {
    let pos_3d = vec3(world_pos.x, terrain_height, world_pos.y);
    let dist_3d = distance(pos_3d, sphere.pos);
    return smoothstep(sphere.influence_radius, 0.0, dist_3d);
}

// ─── [COUPLING:gol→cells:life] ─────────────────────────────────────────

fn coupling_gol_count_neighbors(texel: vec2<u32>, active_size: u32) -> i32 {
    var count: i32 = 0;
    
    for (var dy: i32 = -1; dy <= 1; dy++) {
        for (var dx: i32 = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) { continue; }
            
            // Wrap around (toroidal topology)
            let nx = (i32(texel.x) + dx + i32(active_size)) % i32(active_size);
            let ny = (i32(texel.y) + dy + i32(active_size)) % i32(active_size);
            
            let neighbor_idx = u32(ny) * CELL_GRID_SIZE + u32(nx);
            if (terrain_cells[neighbor_idx].life > 0.5) {
                count++;
            }
        }
    }
    return count;
}

fn coupling_gol_next_state(alive: bool, neighbors: i32) -> f32 {
    // Conway's rules:
    // - Alive cell with 2-3 neighbors survives
    // - Dead cell with exactly 3 neighbors becomes alive
    // - All other cells die/stay dead
    if (alive) {
        return select(0.0, 1.0, neighbors == 2 || neighbors == 3);
    } else {
        return select(0.0, 1.0, neighbors == 3);
    }
}

// ─── [COUPLING:random→cells] Per-cell variance ───────────────────────────

fn cell_random_factor(cell_id: vec2<u32>) -> vec4<f32> {
    // Returns random offset for (color.rgb, height_mod.a)
    // Only applied when coupling is active (mutable effect)
    
    if (!coupling_active(COUPLING_RANDOM_TO_CELL_GOALS)) {
        return vec4(0.0);  // No randomness when muted
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // TEMPORAL BEHAVIOR: Multiple modes available
    // ═══════════════════════════════════════════════════════════════════════
    
    var seed_color = CELL_RANDOM_SEED_COLOR;
    var seed_height = CELL_RANDOM_SEED_HEIGHT;
    
    // Amount modulation (smooth breathing) - always active
    let phase = signal.t_beats * 3.14159 * CELL_RANDOM_TEMPORAL_FREQUENCY;
    let modulation = sin(phase) * 0.5 + 0.5;  // Oscillates [0, 1]
    
    // Generate random height offset
    let random_height = random_float_range(
        cell_id,
        seed_height,
        -CELL_RANDOM_HEIGHT_AMOUNT * modulation,
        CELL_RANDOM_HEIGHT_AMOUNT * modulation
    );
    
    // ═══════════════════════════════════════════════════════════════════════
    // COLOR BEHAVIOR: Three temporal modes
    // ═══════════════════════════════════════════════════════════════════════
    
    var random_color: vec3<f32>;
    
    if (CELL_RANDOM_MODE_HUE_SHIFT && CELL_RANDOM_TEMPORAL_HUE_CYCLE) {
        // ─── MODE 1: CONTINUOUS HUE CYCLING ───────────────────────────────
        let cell_phase_offset = random_float(cell_id, seed_color);
        let hue_cycle = signal.t_beats * CELL_RANDOM_HUE_CYCLE_SPEED + cell_phase_offset;
        let hue_shift = fract(hue_cycle) * CELL_RANDOM_HUE_RANGE * 2.0 - CELL_RANDOM_HUE_RANGE;
        let sat_shift = random_float_range(cell_id, seed_color + 1000u, -0.1, 0.1) * modulation;
        let val_shift = random_float_range(cell_id, seed_color + 2000u, -0.15, 0.15) * modulation;
        random_color = vec3(hue_shift, sat_shift, val_shift);
        
    } else if (CELL_RANDOM_MODE_HUE_SHIFT) {
        // ─── MODE 2: DISCRETE PATTERN CHANGES (original) ──────────────────
        if (CELL_RANDOM_TEMPORAL_PATTERN_CHANGE) {
            let half_beat_count = u32(floor(signal.t_beats * 2.0));
            seed_color = seed_color ^ half_beat_count;
        }
        let hue_shift = random_float_range(
            cell_id,
            seed_color,
            -CELL_RANDOM_HUE_RANGE * modulation,
            CELL_RANDOM_HUE_RANGE * modulation
        );
        let sat_shift = random_float_range(cell_id, seed_color + 1000u, -0.1, 0.1) * modulation;
        let val_shift = random_float_range(cell_id, seed_color + 2000u, -0.15, 0.15) * modulation;
        random_color = vec3(hue_shift, sat_shift, val_shift);
        
    } else {
        // ─── MODE 3: RGB OFFSET MODE (subtle) ─────────────────────────────
        if (CELL_RANDOM_TEMPORAL_PATTERN_CHANGE) {
            let half_beat_count = u32(floor(signal.t_beats * 2.0));
            seed_color = seed_color ^ half_beat_count;
        }
        random_color = random_vec3_range(
            cell_id, 
            seed_color,
            -CELL_RANDOM_COLOR_AMOUNT * modulation,
            CELL_RANDOM_COLOR_AMOUNT * modulation
        );
    }
    
    return vec4(random_color, random_height);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 DYNAMICS
// ═══════════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════════
// §4.1 TERRAIN
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [DYNAMICS:terrain] Wave evaluation ───────────────────────────────────────

fn dynamics_terrain_wave_single(pos: vec2<f32>, t: f32, w: WaveComponent) -> f32 {
    let spatial = w.frequency * dot(w.dir, pos);
    let temporal = (6.28318 / w.period) * t;
    return w.amplitude * sin(spatial + temporal);
}

fn dynamics_terrain_wave_eval(pos: vec2<f32>, t: f32, amplitude_scale: f32) -> f32 {
    var h: f32 = 0.0;
    for (var i: i32 = 0; i < WAVE_COUNT; i++) {
        h += dynamics_terrain_wave_single(pos, t, WAVES[i]);
    }
    return h * amplitude_scale * HEIGHT_MAX_AMPLITUDE;
}

fn dynamics_terrain_gradient_single(pos: vec2<f32>, t: f32, w: WaveComponent) -> vec2<f32> {
    let spatial = w.frequency * dot(w.dir, pos);
    let temporal = (6.28318 / w.period) * t;
    let c = cos(spatial + temporal);
    return w.amplitude * w.frequency * c * w.dir;
}

fn dynamics_terrain_gradient_eval(pos: vec2<f32>, t: f32, amplitude_scale: f32) -> vec2<f32> {
    var grad = vec2<f32>(0.0);
    for (var i: i32 = 0; i < WAVE_COUNT; i++) {
        grad += dynamics_terrain_gradient_single(pos, t, WAVES[i]);
    }
    return grad * amplitude_scale * HEIGHT_MAX_AMPLITUDE;
}

fn dynamics_terrain_gradient_max(amplitude_scale: f32) -> f32 {
    var max_grad: f32 = 0.0;
    for (var i: i32 = 0; i < WAVE_COUNT; i++) {
        max_grad += WAVES[i].amplitude * WAVES[i].frequency;
    }
    return max_grad * amplitude_scale * HEIGHT_MAX_AMPLITUDE * 1.5;
}

fn dynamics_terrain_normal(pos: vec2<f32>, t: f32, amplitude_scale: f32) -> vec3<f32> {
    let grad = dynamics_terrain_gradient_eval(pos, t, amplitude_scale);
    return normalize(vec3(-grad.x, 1.0, -grad.y));
}

// ═══════════════════════════════════════════════════════════════════════════════
// §4.2 PGA DYNAMICS
// ═══════════════════════════════════════════════════════════════════════════════
//
// These functions use Projective Geometric Algebra for elegant transformations.
// The Motor encodes rotation and translation together, enabling natural composition.

// ─── [DYNAMICS:PGA] Color Motor ────────────────────────────────────────────────
//
// Treats RGB space as a 3D projective space. The "luminance axis" (grey line from
// black to white) becomes the axis of rotation. Hue becomes angular position around
// this axis. Saturation is distance from the axis. Value is position along the axis.
//
// This creates organic color spiraling that feels musical — notes cause the sphere
// to twist through color space rather than linearly interpolate between two colors.

fn pga_color_motor(current_rgb: vec3<f32>, hue_speed: f32, sat_push: f32, val_climb: f32, dt: f32) -> vec3<f32> {
    // 1. CONVERT COLOR TO POINT
    let p_color = point_from_vec3(current_rgb);
    
    // 2. DEFINE AXIS OF LUMINANCE (The Grey Line)
    let axis_dir = normalize(vec3(0.60, 0.20, 0.10));
    
    // 3. HUE ROTOR (Twist)
    //    Rotation around the luminance axis shifts hue
    let r_hue = rotor(axis_dir, hue_speed * dt);
    
    // 4. VALUE TRANSLATOR (Climb)
    //    Translation along the luminance axis changes value
    let t_val = translator(axis_dir, val_climb * dt);
    
    // 5. SATURATION TRANSLATOR (Push)
    //    Translation perpendicular to luminance axis changes saturation
    //    Push AWAY from the grey line.
    let parallel = dot(current_rgb, axis_dir) * axis_dir;
    let perpendicular = current_rgb - parallel;
    var sat_dir = vec3(0.0);
    if (length(perpendicular) > 0.0001) {
        sat_dir = normalize(perpendicular);
    }
    let t_sat = translator(sat_dir, sat_push * dt);
    
    // 6. COMPOSE MOTOR: Hue -> Sat -> Val
    //    The geometric product composes transformations elegantly
    var m = gp_mm(t_sat, r_hue);
    m = gp_mm(t_val, m);
    
    // 7. APPLY AND RETURN
    let p_new = sw_motor_point(m, p_color);
    return clamp(point_to_vec3(p_new), vec3(0.0), vec3(1.0));
}

// ─── [DYNAMICS:PGA] Sphere Orbit ────────────────────────────────────────────────
//
// Uses a PGA Motor (rotor) to orbit the sphere around the Y axis.
// The Motor provides both position AND orientation naturally — the sphere
// automatically faces its direction of travel without manual tangent computation.

fn dynamics_sphere_motor_orbit(t: f32) -> SphereState {
    var s: SphereState;

    // 1. DEFINITION
    //    Axis: The vertical line through the origin (Line Y)
    //    Speed: The orbital speed constant
    let orbit_axis = LINE_Y.d; 
    let angle = t * CURVE_ORBIT_SPEED;
    
    // 2. THE MOTOR (The Spell)
    //    Create a rotor that spins around the Y axis.
    let m_orbit = rotor(orbit_axis, angle); 
    
    // 3. LIFT (The Input)
    //    Define the starting position as a standard point.
    //    (Start at Radius on X, Height on Y)
    let start_pos_vec = vec3(CURVE_ORBIT_RADIUS, CURVE_ORBIT_HEIGHT, 0.0);
    let p_start = point_from_vec3(start_pos_vec);
    
    // 4. MOTIVATE (The Transformation)
    //    Apply the motor to the point via sandwich product.
    let p_moved = sw_motor_point(m_orbit, p_start);
    
    // 5. DROP (The Output)
    //    Convert back to vec3 for the humble renderer.
    s.pos = point_to_vec3(p_moved);
    
    // Bonus: PGA gives us the orientation for free!
    // The sphere rotates to face its path.
    s.orientation = m_orbit.p0; 
    
    s.radius = SPHERE_RADIUS;
    s.influence_radius = SPHERE_INFLUENCE_RADIUS;
    s.t = t;
    
    return s;
}

// ─── [DYNAMICS:sphere] Tangent (for legacy compatibility) ─────────────────────

fn dynamics_sphere_orbit_tangent(t: f32) -> vec3<f32> {
    // Derivative of position with respect to t, normalized
    // Kept for compatibility but PGA motor provides orientation directly
    let angle = t * CURVE_ORBIT_SPEED;
    return normalize(vec3(-sin(angle), 0.0, cos(angle)));
}

// ═══════════════════════════════════════════════════════════════════════════════
// §4.3 FIELDS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [DYNAMICS:field] Color field operations ──────────────────────────────────

fn dynamics_proximity_field_blend_shift_direction(pawn_stimulus: f32, sphere_stimulus: f32) -> vec3<f32> {
    // Blend shift direction based on relative stimulus contributions
    let total = pawn_stimulus + sphere_stimulus;
    if (total < 0.001) {
        return PROXIMITY_SHIFT_PAWN;  // Default (irrelevant when offset → 0)
    }
    let pawn_weight = pawn_stimulus / total;
    let sphere_weight = sphere_stimulus / total;
    return pawn_weight * PROXIMITY_SHIFT_PAWN + sphere_weight * PROXIMITY_SHIFT_SPHERE;
}

fn dynamics_proximity_field_apply_shift(base: vec3<f32>, offset: f32, shift_direction: vec3<f32>) -> vec3<f32> {
    return clamp(base + shift_direction * offset, vec3(0.0), vec3(1.0));
}

fn dynamics_terrain_base_color() -> vec3<f32> {
    return TERRAIN_BASE_COLOR;
}

// ═══════════════════════════════════════════════════════════════════════════════
// §4.4 CELLS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [DYNAMICS:cells] Cell trajectory dynamics ────────────────────────────────

fn dynamics_cell_trajectory_spring(cell: CellState, dt: f32) -> CellState {
    var c = cell;
    
    let displacement = c.value - c.goal;
    let spring_force = -CELL_SPRING_STIFFNESS * displacement;
    let damping_force = -CELL_SPRING_DAMPING * sqrt(CELL_SPRING_STIFFNESS) * c.velocity;
    let accel = spring_force + damping_force;
    
    c.velocity = c.velocity + accel * dt;
    c.value = c.value + c.velocity * dt;
    
    return c;
}

fn dynamics_cell_trajectory_release(cell: CellState, cell_id: vec2<u32>, dt: f32) -> CellState {
    var c = cell;
    
    let idle = cell_idle(cell_id);  // Computed, not loaded!
    let decay = 1.0 - exp(-CELL_RELEASE_RATE * dt);
    c.value = c.value + (idle - c.value) * decay;
    c.velocity = vec4(0.0);  // No momentum during release
    
    return c;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 COMPOSITION
// ═══════════════════════════════════════════════════════════════════════════════

// Execution orchestration - where couplings and dynamics are applied.

// ─── Helper: Sphere construction from PGA orbit ──────────────────────────────
//
// Uses the PGA Motor orbit to compute sphere position and orientation.
// The Motor naturally encodes both rotation and the resulting orientation.

fn compose_sphere_from_orbit_pga(t: f32) -> SphereState {
    // 1. Get the pure PGA orbit (circular motion via Motor)
    var s = dynamics_sphere_motor_orbit(t);
    
    // 2. Apply terrain coupling (Height adjustment)
    //    This is an "effect" applied after the ideal motion
    let base_height = s.pos.y;  // Should be CURVE_ORBIT_HEIGHT from motor
    let adjusted_height = coupling_terrain_to_sphere_orbit_height(
        vec2(s.pos.x, s.pos.z),
        base_height
    );
    
    // Update position with terrain-adjusted height
    s.pos.y = adjusted_height;
    
    // Note: Orientation from motor is already set by dynamics_sphere_motor_orbit
    // Color is set by coupling in main loop (not here)
    
    return s;
}

// ─── Helper: Camera position from orbital parameters ─────────────────────────

fn compose_camera_position_from_orbit(aim_point: vec3<f32>, cam: CameraState) -> vec3<f32> {
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

// ═══════════════════════════════════════════════════════════════════════════════
// §5.1 0D COMPOSITION
// ═══════════════════════════════════════════════════════════════════════════════

fn compose_entities_0d_update(dt: f32, t: f32) {
    if (!dynamics_0d_active()) {
        return;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Signal → Substrate (amplitude modulation)
    // ═══════════════════════════════════════════════════════════════════════
    
    var amplitude_scale = terrain_state.amplitude_scale;
    
    if (signal_active() && coupling_active(COUPLING_POLYPHONY_TO_AMPLITUDE)) {
        let poly = signal.stats[0];
        let traj = trajectories[0];
        let new_traj = coupling_signal_polyphony_to_terrain_amplitude(poly, traj, dt);
        trajectories[0] = new_traj;
        amplitude_scale = new_traj.value;
    }
    
    terrain_state.amplitude_scale = amplitude_scale;
    
    let max_grad = dynamics_terrain_gradient_max(amplitude_scale);
    terrain_state.lipschitz_factor = sqrt(1.0 + max_grad * max_grad);
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Input → Pawn (movement)
    // ═══════════════════════════════════════════════════════════════════════
    
    var pawn = pawn_state;
    
    if (coupling_active(COUPLING_INPUT_MOVES_PAWN)) {
        let input_dir = vec2(signal.move_x, signal.move_z);
        let world_vel = coupling_input_to_pawn_velocity(input_dir, camera_state.azimuth);
        
        let speed = select(PAWN_SPEED, config.pawn_speed, config.pawn_speed > 0.0);
        pawn.pos.x += world_vel.x * speed * dt;
        pawn.pos.z += world_vel.y * speed * dt;
        
        let clamped = chart_clamp(pawn.pos.xz);
        pawn.pos.x = clamped.x;
        pawn.pos.z = clamped.y;
        
        pawn.heading = coupling_velocity_to_pawn_heading(world_vel, pawn.heading, dt);
        
        // Store velocity for force field radius calculation
        pawn.velocity = world_vel * speed;
    } else {
        // No input = velocity decays (could add smoothing here)
        pawn.velocity = vec2(0.0);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Substrate → Pawn (terrain following)
    // ═══════════════════════════════════════════════════════════════════════
    
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = t * time_scale;
    
    if (coupling_active(COUPLING_TERRAIN_TO_PAWN_Y)) {
        pawn.pos.y = coupling_terrain_to_pawn_height(pawn.pos.xz, wave_time, amplitude_scale);
    }
    
    if (coupling_active(COUPLING_TERRAIN_TO_PAWN_TILT)) {
        pawn.orientation = coupling_terrain_to_pawn_orientation(pawn.pos.xz, pawn.heading, wave_time, amplitude_scale);
    }
    
    pawn_state = pawn;
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Input → Camera (orbit, zoom, pan)
    // ═══════════════════════════════════════════════════════════════════════
    
    var camera = camera_state;
    
    if (coupling_active(COUPLING_INPUT_ORBITS_CAMERA)) {
        camera = coupling_input_to_camera_orbit(signal.look_az_delta, signal.look_el_delta, camera);
        camera = coupling_input_to_camera_pan(vec2(signal.pan_x_delta, signal.pan_y_delta), camera);
    }
    
    if (coupling_active(COUPLING_INPUT_ZOOMS_CAMERA)) {
        camera = coupling_input_to_camera_distance(signal.zoom_delta, camera);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Pawn → Camera (follow target)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (coupling_active(COUPLING_PAWN_TO_CAMERA_TARGET)) {
        camera.pos = coupling_pawn_to_camera_target(pawn_state.pos, camera);
    }
    
    camera_state = camera;
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Time → Sphere (PGA Motor orbit + derived orientation)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // The sphere's motion uses PGA: a Motor (rotor) around the Y axis.
    // This gives us position AND orientation from a single algebraic object.
    
    if (!sphere_frozen()) {
        // Advance sphere's curve parameter
        var sphere = sphere_state;
        sphere.t = sphere.t + signal.dt;
        
        // Compute position and orientation from PGA Motor orbit
        let sphere_updated = compose_sphere_from_orbit_pga(sphere.t);
        sphere.pos = sphere_updated.pos;
        sphere.orientation = sphere_updated.orientation;
        sphere_state = sphere;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Signal → Sphere (polyphony drives color via PGA Color Motor)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // This runs even when sphere is frozen — color responds to music
    // independently of position. The PGA Color Motor creates organic spiraling
    // through color space rather than linear interpolation.
    
    if (signal_active() && coupling_active(COUPLING_POLYPHONY_TO_SPHERE_COLOR)) {
        sphere_state.color = coupling_signal_polyphony_to_sphere_color(
            signal.stats[0],        // polyphony count
            sphere_state.color,     // current color (for smoothing)
            dt
        );
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Sphere → Terrain (position paints the world)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // Sphere's orbital position creates a global tint.
    // As it circles, the world breathes through warm and cool tones.
    
    if (coupling_active(COUPLING_SPHERE_TO_TERRAIN_TINT)) {
        terrain_state.tint = coupling_sphere_to_terrain_tint(sphere_state.pos);
    } else {
        terrain_state.tint = vec3(1.0);  // Neutral when disabled
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// §5.2 2D COMPOSITION
// ═══════════════════════════════════════════════════════════════════════════════

fn compose_terrain_height_2d_update(texel: vec2<u32>, t: f32) {
    let world_xz = chart_height_texel_to_world(texel);
    
    var h: f32;
    var grad: vec2<f32>;
    
    if (dynamics_2d_active()) {
        let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
        let wave_time = t * time_scale;
        let amp = terrain_state.amplitude_scale;
        
        h = dynamics_terrain_wave_eval(world_xz, wave_time, amp);
        grad = dynamics_terrain_gradient_eval(world_xz, wave_time, amp);
    } else {
        h = 0.0;
        grad = vec2(0.0);
    }
    
    textureStore(terrain_height_field_write, texel, vec4(h, grad.x, grad.y, 1.0));
}

fn compose_cell_height_2d_update(texel: vec2<u32>) {
    // ═══════════════════════════════════════════════════════════════════════
    // CELL HEIGHT FIELD — Geometry layer driven by cell trajectory
    // ═══════════════════════════════════════════════════════════════════════
    //
    // Height emerges from cell.value.a which is animated via spring dynamics.
    // Suppressed within pawn's safety force field.
    // Resolution: 64×64 (matches cell grid exactly)
    
    let active_size = u32(config.active_cell_size);
    if (texel.x >= active_size || texel.y >= active_size) {
        textureStore(cell_height_field_write, texel, vec4(0.0));
        return;
    }
    
    let cell = terrain_cells[cell_index(texel)];
    
    // Base height from trajectory (includes GoL, pawn, sphere, polyphony effects)
    var h: f32 = CELL_HEIGHT_BASE + cell.value.a * CELL_HEIGHT_STIMULUS;
    
    // Per-cell random variation, modulated by activity
    let activity = saturate(abs(cell.value.a));
    if (activity > 0.01 && CELL_HEIGHT_RANDOM_RANGE > 0.0) {
        let random_factor = random_float_range(texel, CELL_RANDOM_SEED_HEIGHT, -1.0, 1.0);
        h += random_factor * CELL_HEIGHT_RANDOM_RANGE * activity;
    }
    
    // Apply pawn force field suppression (smooth terrain near pawn)
    let normalized = vec2<f32>(texel) / f32(active_size);
    let world_xz = (normalized - 0.5) * CHART_EXTENT;
    let forcefield = coupling_pawn_forcefield_factor(world_xz);
    h *= forcefield;
    
    textureStore(cell_height_field_write, texel, vec4(h, 0.0, 0.0, 0.0));
}

fn compose_cells_2d_update(texel: vec2<u32>, dt: f32) {
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMIC RESOLUTION: Skip cells outside active region
    // ═══════════════════════════════════════════════════════════════════════
    
    let active_size = u32(config.active_cell_size);
    if (texel.x >= active_size || texel.y >= active_size) {
        return;
    }
    
    let idx = cell_index(texel);
    var cell = terrain_cells[idx];
    
    // Compute cell_id for idle function (always use texel, not remapped)
    let cell_id = texel;
    
    // ═══════════════════════════════════════════════════════════════════════
    // WORLD POSITION: Remap to fill world extent regardless of active size
    // ═══════════════════════════════════════════════════════════════════════
    
    let normalized = vec2<f32>(texel) / f32(active_size);
    let world_pos = (normalized - 0.5) * CHART_EXTENT;
    
    // Approximate terrain height at this tile (for 3D sphere distance)
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = signal.t_beats * time_scale;
    let terrain_height = dynamics_terrain_wave_eval(world_pos, wave_time, terrain_state.amplitude_scale);
    
    // ═══════════════════════════════════════════════════════════════════════
    // GAME OF LIFE: Update cellular automaton every half beat
    // ═══════════════════════════════════════════════════════════════════════
    //
    // Ticks every 0.5 beats. Creates organic evolving patterns.
    // Visibility is controlled by polyphony — invisible when silent.
    
    // Check if we just crossed a tick boundary
    // Using fixed 5% window (framerate-independent)
    let tick_phase = signal.t_beats * GOL_TICKS_PER_BEAT;
    let should_tick = fract(tick_phase) < 0.05;
    
    if (should_tick) {
        // ─── Mathematically Correct Game of Life ─────────────────────────────
        //
        // Phase 1: Synchronization (life_next → life) is handled by the
        //          dedicated sync_life_state compute pass that runs BEFORE
        //          this function. All cells now have synchronized state.
        //
        // Phase 2: Count neighbors from synchronized display state.
        //          Since sync_life_state completed for ALL cells before this
        //          pass began, there is ZERO race condition. Perfect determinism.
        let neighbors = coupling_gol_count_neighbors(texel, active_size);
        let currently_alive = cell.life > 0.5;
        
        // Phase 3: Compute next generation into buffer.
        //          Write to life_next (not life). The next tick's sync_life_state
        //          pass will apply this result to all cells simultaneously.
        cell.life_next = coupling_gol_next_state(currently_alive, neighbors);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // [SAFETY CHECK] Verification of the Ring (PGA Library Check)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // Only runs on the first cell to save perf. If the PGA library is broken,
    // this cell turns BRIGHT RED as a visual alarm.
    
    if (all(texel == vec2(0u))) {
        let test_motor = rotor(vec3(0.0, 1.0, 0.0), 0.1);
        let error = verify_motor_norm(test_motor);
        if (error.x > 0.001 || error.y > 0.001) {
            cell.value = vec4(1.0, 0.0, 0.0, 1.0); 
            cell.is_active = 1.0;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // STIMULUS: Gather from all sources
    // ═══════════════════════════════════════════════════════════════════════
    
    var total_stimulus: f32 = 0.0;
    
    // Musical stimulus (polyphony → hue rotation + height modulation)
    var poly_stimulus: f32 = 0.0;
    if (signal_active() && coupling_active(COUPLING_POLYPHONY_TO_CELL_COLOR)) {
        poly_stimulus = signal.stats[0] / 4.0;  // Normalize roughly
        total_stimulus += poly_stimulus;
    }
    
    // Pawn proximity stimulus
    var pawn_stimulus: f32 = 0.0;
    if (coupling_active(COUPLING_PAWN_TO_CELL_COLOR)) {
        pawn_stimulus = coupling_pawn_to_cells_influence(world_pos, pawn_state.pos.xz);
        total_stimulus += pawn_stimulus;
    }
    
    // Sphere proximity stimulus
    var sphere_stimulus: f32 = 0.0;
    if (coupling_active(COUPLING_SPHERE_TO_CELL_COLOR)) {
        sphere_stimulus = coupling_sphere_to_cells_influence(world_pos, terrain_height, sphere_state);
        total_stimulus += sphere_stimulus;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // GOAL: Blend vec4 based on relative stimulus contributions
    // ═══════════════════════════════════════════════════════════════════════
    
    // Get computed idle for this cell
    let idle = cell_idle(cell_id);
    
    // ═══════════════════════════════════════════════════════════════════════
    // DEBUG MODE: Pure oscillation visualization (OVERRIDES EVERYTHING)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (CELL_RANDOM_DEBUG_OSCILLATION && coupling_active(COUPLING_RANDOM_TO_CELL_GOALS)) {
        // Force ALL cells active in debug mode (ignore stimulus)
        cell.is_active = 1.0;
        
        // Each cell oscillates between two pure colors
        // Phase offset so cells aren't synchronized
        let cell_phase = random_float(cell_id, CELL_RANDOM_SEED_COLOR);
        
        // Oscillate from 0.0 to 1.0
        let osc_phase = signal.t_beats * 3.14159 * 2.0 * CELL_RANDOM_DEBUG_OSC_SPEED + cell_phase * 6.28318;
        let osc_value = sin(osc_phase) * 0.5 + 0.5;  // [0, 1]
        
        // Lerp between two pure colors
        let debug_color = mix(CELL_RANDOM_DEBUG_COLOR_A, CELL_RANDOM_DEBUG_COLOR_B, osc_value);
        
        // Set goal directly
        cell.goal = vec4(debug_color, 0.0);  // Flat terrain in debug mode
        
    } else if (CELL_RANDOM_TEMPORAL_HUE_CYCLE && CELL_RANDOM_HUE_CYCLE_FORCE_ACTIVE && 
               CELL_RANDOM_MODE_HUE_SHIFT && coupling_active(COUPLING_RANDOM_TO_CELL_GOALS)) {
        // Force ALL cells to cycle through hues (ignore stimulus)
        cell.is_active = 1.0;
        
        // Start from idle base color
        let base_color = idle.rgb;
        
        // Apply continuous hue cycling
        let cell_phase_offset = random_float(cell_id, CELL_RANDOM_SEED_COLOR);
        let hue_cycle = signal.t_beats * CELL_RANDOM_HUE_CYCLE_SPEED + cell_phase_offset;
        let hue_shift = fract(hue_cycle) * CELL_RANDOM_HUE_RANGE * 2.0 - CELL_RANDOM_HUE_RANGE;
        
        // Convert to HSV, shift hue, convert back
        let hsv = rgb_to_hsv(base_color);
        let new_hue = fract(hsv.x + hue_shift);
        let cycled_color = hsv_to_rgb(vec3(new_hue, hsv.y, hsv.z));
        
        // Set goal
        cell.goal = vec4(cycled_color, 0.0);
        
    } else if (total_stimulus > 0.01) {
        // NORMAL MODE: Only active cells get colors
        cell.is_active = 1.0;
        
        // Start from idle (computed)
        var blended_color = idle.rgb;
        var blended_height = idle.a;
        
        // Add polyphony hue shift + height modulation
        if (poly_stimulus > 0.01) {
            let poly_color = coupling_signal_polyphony_to_cells_color(signal.stats[0]);
            let poly_weight = poly_stimulus / total_stimulus;
            blended_color = mix(blended_color, poly_color, poly_weight * 0.8);
            // Polyphony also modulates height
            blended_height = mix(blended_height, signal.stats[0] * 0.1, poly_weight * 0.5);
            
            // Game of Life: alive cells glow with polyphony
            // Invisible when silent, increasingly visible with more notes
            let life_visibility = cell.life * poly_stimulus * 0.7;
            blended_color = mix(blended_color, GOL_COLOR_ALIVE, life_visibility);
        }
        
        // Game of Life: alive cells rise (goes through trajectory for smooth animation)
        // This is OUTSIDE the poly_stimulus check so height goal is always set for alive cells
        // Suppressed within pawn's safety force field
        let forcefield = coupling_pawn_forcefield_factor(world_pos);
        blended_height = blended_height + cell.life * CELL_HEIGHT_ALIVE * forcefield;
        
        // Add pawn purple shift + height depression (pawn pushes down)
        if (pawn_stimulus > 0.01) {
            let pawn_color = idle.rgb + PROXIMITY_SHIFT_PAWN;
            let pawn_weight = pawn_stimulus / total_stimulus;
            blended_color = mix(blended_color, pawn_color, pawn_weight * 0.9);
            // Pawn depresses terrain slightly
            blended_height = mix(blended_height, -0.3 * pawn_stimulus, pawn_weight);
        }
        
        // Add sphere gold shift + height bulge (sphere pushes up)
        if (sphere_stimulus > 0.01) {
            let sphere_color = idle.rgb + PROXIMITY_SHIFT_SPHERE;
            let sphere_weight = sphere_stimulus / total_stimulus;
            blended_color = mix(blended_color, sphere_color, sphere_weight * 0.9);
            // Sphere raises terrain slightly
            blended_height = mix(blended_height, 0.5 * sphere_stimulus, sphere_weight);
        }
        
        // Base goal from blended stimulus
        let base_goal_color = clamp(blended_color, vec3(0.0), vec3(1.0));
        let base_goal = vec4(base_goal_color, blended_height);
        
        var final_color: vec3<f32>;
        
        if (CELL_RANDOM_MODE_HUE_SHIFT && coupling_active(COUPLING_RANDOM_TO_CELL_GOALS)) {
            // Hue shift mode
            let random_factor = cell_random_factor(cell_id);
            
            // HUE SHIFT MODE: Convert to HSV, shift hue, convert back
            let hsv = rgb_to_hsv(base_goal_color);
            let hue_shift = random_factor.r;
            let sat_shift = random_factor.g;
            let val_shift = random_factor.b;
            
            // Apply shifts with wrapping for hue
            let new_hue = fract(hsv.x + hue_shift);  // Wrap around color wheel
            let new_sat = clamp(hsv.y + sat_shift, 0.0, 1.0);
            let new_val = clamp(hsv.z + val_shift, 0.0, 1.0);
            
            final_color = hsv_to_rgb(vec3(new_hue, new_sat, new_val));
        } else {
            // RGB OFFSET MODE: Direct color addition
            let random_factor = cell_random_factor(cell_id);
            let randomized_goal = base_goal + random_factor;
            final_color = clamp(randomized_goal.rgb, vec3(0.0), vec3(1.0));
        }
        
        // Apply to goal
        let random_factor_height = cell_random_factor(cell_id);
        cell.goal = vec4(final_color, blended_height + random_factor_height.a);
    } else {
        cell.is_active = 0.0;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMICS: Spring toward goal or release to idle (computed)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (dynamics_2d_active()) {
        if (CELL_RANDOM_DEBUG_OSCILLATION && coupling_active(COUPLING_RANDOM_TO_CELL_GOALS)) {
            // DEBUG MODE: Bypass spring dynamics, set value directly
            cell.value = cell.goal;  // Direct assignment (color.rgb, height.a)
            cell.velocity = vec4(0.0);  // Kill all velocity
        } else if (CELL_RANDOM_TEMPORAL_HUE_CYCLE && CELL_RANDOM_HUE_CYCLE_FORCE_ACTIVE && 
                   CELL_RANDOM_MODE_HUE_SHIFT && coupling_active(COUPLING_RANDOM_TO_CELL_GOALS)) {
            // FORCED HUE CYCLE MODE: Bypass spring dynamics for immediate color response
            cell.value = cell.goal;  // Direct assignment
            cell.velocity = vec4(0.0);  // Kill all velocity
        } else if (cell.is_active > 0.5) {
            cell = dynamics_cell_trajectory_spring(cell, dt);
        } else {
            cell = dynamics_cell_trajectory_release(cell, cell_id, dt);  // Pass cell_id for computed idle
        }
    }
    
    terrain_cells[idx] = cell;
}

fn compose_surface_color_2d_update(texel: vec2<u32>) {
    // ═══════════════════════════════════════════════════════════════════════
    // LOAD: Get current trajectory state for this cell
    // ═══════════════════════════════════════════════════════════════════════
    
    let field_idx = proximity_field_index(texel);
    var proximity_traj = terrain_proximity_field[field_idx];
    
    // ═══════════════════════════════════════════════════════════════════════
    // STIMULUS: Compute separate influences from pawn and sphere
    // ═══════════════════════════════════════════════════════════════════════
    
    let world_pos = chart_surface_color_texel_to_world(texel);
    
    // Approximate terrain height at this tile (for 3D sphere distance)
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = signal.t_beats * time_scale;
    let terrain_height = dynamics_terrain_wave_eval(world_pos, wave_time, terrain_state.amplitude_scale);
    
    // Track stimuli separately for color blending
    var pawn_stimulus: f32 = 0.0;
    var sphere_stimulus: f32 = 0.0;
    
    // Pawn stimulus (2D distance on ground plane)
    if (coupling_active(COUPLING_PAWN_TO_PROXIMITY_FIELD)) {
        pawn_stimulus = coupling_pawn_to_proximity_field(world_pos, pawn_state.pos.xz);
    }
    
    // Sphere stimulus (3D distance from floating sphere)
    if (coupling_active(COUPLING_SPHERE_TO_PROXIMITY_FIELD)) {
        sphere_stimulus = coupling_sphere_to_proximity_field(world_pos, terrain_height, sphere_state);
    }
    
    // Combined stimulus drives trajectory dynamics
    let total_stimulus = pawn_stimulus + sphere_stimulus;
    
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMICS: Spring toward goal or release to idle
    // ═══════════════════════════════════════════════════════════════════════
    
    if (dynamics_2d_active()) {
        if (total_stimulus > 0.001) {
            // Stimulus present: spring toward goal
            let goal = total_stimulus * PROXIMITY_FIELD_MAX_OFFSET;
            proximity_traj = trajectory_spring(proximity_traj, goal, signal.dt,
                                     PROXIMITY_FIELD_ATTACK_STIFFNESS, PROXIMITY_FIELD_ATTACK_DAMPING);
        } else {
            // No stimulus: release back to idle
            proximity_traj = trajectory_release(proximity_traj, IDLE_PROXIMITY_OFFSET, signal.dt, PROXIMITY_FIELD_RELEASE_RATE);
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // STORE: Save trajectory state for next frame
    // ═══════════════════════════════════════════════════════════════════════
    
    terrain_proximity_field[field_idx] = proximity_traj;
    
    // ═══════════════════════════════════════════════════════════════════════
    // OUTPUT: Compute final color from base + blended trajectory offset
    // ═══════════════════════════════════════════════════════════════════════
    //
    // Trajectory value = "how much" displacement (persistent memory)
    // Color direction = weighted blend of entity shifts (instantaneous)
    
    let base_color = TERRAIN_BASE_COLOR;
    let color_shift = dynamics_proximity_field_blend_shift_direction(pawn_stimulus, sphere_stimulus);
    var final_color = dynamics_proximity_field_apply_shift(base_color, proximity_traj.value, color_shift);
    
    // ═══════════════════════════════════════════════════════════════════════
    // BLEND: Mix in cell trajectory (multi-channel: .rgb=color, .a=height_mod)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // The cell system provides an additional color layer that blends with
    // the proximity field system. Cell color is mixed on top.
    // Height modulation is stored in alpha for future use by terrain.
    
    // Map texel to cell buffer with dynamic resolution
    let active_size = u32(config.active_cell_size);
    let cell_texel = vec2<u32>(
        (texel.x * active_size) / CELL_GRID_SIZE,
        (texel.y * active_size) / CELL_GRID_SIZE
    );
    let cell_idx = cell_texel.y * active_size + cell_texel.x;
    
    // Bounds check (active_size might be smaller than CELL_GRID_SIZE)
    var cell_color = base_color;
    var cell_active: f32 = 0.0;
    var cell_height_mod: f32 = 0.0;
    
    if (cell_idx < active_size * active_size) {
        let cell = terrain_cells[cell_idx];
        cell_color = cell.value.rgb;
        cell_active = cell.is_active;
        cell_height_mod = cell.value.a;
    }
    
    // Blend cell color with trajectory_field color
    // When cell is active (stimulated), its color dominates
    // When cell is at idle (checkerboard), it provides subtle texture
    let cell_influence = max(cell_active, 0.3);  // Always some cell presence
    final_color = mix(final_color, cell_color, cell_influence * 0.5);
    
    // ═══════════════════════════════════════════════════════════════════════
    // Apply global terrain tint from sphere position coupling
    // ═══════════════════════════════════════════════════════════════════════
    //
    // The sphere's orbit creates a slowly rotating color wash.
    // This is the observable effect of COUPLING_SPHERE_TO_TERRAIN_TINT.
    
    final_color = final_color * terrain_state.tint;
    
    // Store color in RGB, height_mod in A (available for terrain shader)
    textureStore(terrain_surface_color_write, texel, vec4(final_color, cell_height_mod));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 OBSERVATION
// ═══════════════════════════════════════════════════════════════════════════════

// Rendering (read-only) - observes state without modifying it.

// ═══════════════════════════════════════════════════════════════════════════════
// §6.1 SDF PRIMITIVES
// ═══════════════════════════════════════════════════════════════════════════════

fn sdf_round_cone(p: vec3<f32>, r_base: f32, r_tip: f32, h: f32) -> f32 {
    let b = (r_base - r_tip) / h;
    let a = sqrt(1.0 - b * b);
    let q = vec2(length(p.xz), p.y);
    let k = dot(q, vec2(-b, a));
    if (k < 0.0) { return length(q) - r_base; }
    if (k > a * h) { return length(q - vec2(0.0, h)) - r_tip; }
    return dot(q, vec2(a, b)) - r_base;
}

fn sdf_sphere(p: vec3<f32>) -> f32 {
    return length(p - render_sphere.pos) - render_sphere.radius;
}

fn sdf_box(p: vec3<f32>, b: vec3<f32>) -> f32 {
    // Exact signed distance to axis-aligned box centered at origin
    // b = half-extents (box goes from -b to +b)
    let q = abs(p) - b;
    return length(max(q, vec3(0.0))) + min(max(q.x, max(q.y, q.z)), 0.0);
}

// ─── VOXEL MODE: True 3D box geometry ────────────────────────────────────────
//
// In VOXEL mode, cells become solid 3D boxes sitting on the wave surface.
// This provides exact SDF (no Lipschitz approximation needed) and eliminates
// the "stacked layers" artifact of heightfield-based BLOCKY mode.

fn sdf_cell_voxel_at(p: vec3<f32>, cell_idx: vec2<i32>, cell_size: f32, half_extent: f32) -> f32 {
    // Returns SDF for a single cell's voxel box
    // Returns large positive value if cell has no height
    
    // Bounds check first (cheapest)
    let active_size = i32(CELL_GRID_SIZE);
    if (cell_idx.x < 0 || cell_idx.x >= active_size || 
        cell_idx.y < 0 || cell_idx.y >= active_size) {
        return 1000.0;
    }
    
    // Sample cell height (skip if negligible - avoids wave sample)
    let h_cell = textureLoad(cell_height_field_read, cell_idx, 0).x * CELL_HEIGHT_SCALE;
    if (h_cell < 0.01) {
        return 1000.0;
    }
    
    // Cell center in world space
    let cell_center_xz = (vec2<f32>(cell_idx) + 0.5) * cell_size - half_extent;
    
    // Sample wave height at cell center (box sits on wave surface)
    let wave_uv = (cell_center_xz / CHART_EXTENT) + 0.5;
    let wave_h = textureSampleLevel(terrain_height_field_read, bilinear_sampler, wave_uv, 0.0).x;
    
    // Box geometry: sits on wave, extends upward by cell height
    let box_half_h = h_cell * 0.5;
    let box_center = vec3(cell_center_xz.x, wave_h + box_half_h, cell_center_xz.y);
    let box_half_size = vec3(cell_size * 0.5, box_half_h, cell_size * 0.5);
    
    return sdf_box(p - box_center, box_half_size);
}

fn sdf_cell_voxels(p: vec3<f32>) -> f32 {
    // Returns union (min) of nearby cell voxel boxes
    // Optimized: early exit when inside, distance-based culling
    
    let cell_size = CHART_EXTENT / f32(CELL_GRID_SIZE);
    let half_extent = CHART_EXTENT * 0.5;
    
    // Find which cell we're in
    let cell_f = (p.xz + half_extent) / cell_size;
    let cell_center = vec2<i32>(floor(cell_f));
    
    // First check the cell we're directly in (most likely hit)
    var d_min = sdf_cell_voxel_at(p, cell_center, cell_size, half_extent);
    
    // Early exit: if inside this cell's box, we're done
    if (d_min < 0.0) {
        return d_min;
    }
    
    // Only check neighbors if we're close enough that they could matter
    // Max cell height determines search radius
    let max_possible_height = CELL_HEIGHT_ALIVE * CELL_HEIGHT_SCALE * 2.0;
    
    if (d_min > cell_size + max_possible_height) {
        return d_min;  // Too far for any neighbor to be closer
    }
    
    // Check 8 neighbors (skip center, already checked)
    for (var dy: i32 = -1; dy <= 1; dy++) {
        for (var dx: i32 = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) { continue; }
            
            let idx = cell_center + vec2<i32>(dx, dy);
            let d = sdf_cell_voxel_at(p, idx, cell_size, half_extent);
            
            // Early exit if inside
            if (d < 0.0) {
                return d;
            }
            
            d_min = min(d_min, d);
        }
    }
    
    return d_min;
}

fn sdf_terrain_height(p: vec3<f32>) -> f32 {
    // Returns the combined terrain height at position p.xz
    // Used by sdf_terrain and can be reused elsewhere
    
    let uv = chart_world_to_uv(p.xz);
    
    // ─── Wave Height (smooth, 256×256) ──────────────────────────────────────
    let h_wave = textureSampleLevel(terrain_height_field_read, bilinear_sampler, uv, 0.0).x;
    
    // ─── Cell Height (64×64) ─────────────────────────────────────────────────
    
    if (!CELL_HEIGHT_ENABLED) {
        return h_wave;
    }
    
    // VOXEL mode: cell heights are handled by separate box SDFs, not heightfield
    if (CELL_HEIGHT_STYLE == CELL_HEIGHT_STYLE_VOXEL) {
        return h_wave;
    }
    
    // Style-specific sampling (avoid redundant texture fetches)
    var h_cell: f32;
    
    if (CELL_HEIGHT_STYLE == CELL_HEIGHT_STYLE_BLOCKY) {
        // BLOCKY: Single nearest-neighbor sample
        h_cell = textureSampleLevel(cell_height_field_read, nearest_sampler, uv, 0.0).x;
        
    } else if (CELL_HEIGHT_STYLE == CELL_HEIGHT_STYLE_SMOOTH) {
        // SMOOTH: Manual bilinear (skip the redundant nearest sample)
        let tex_size = f32(CELL_GRID_SIZE);
        let texel_f = uv * tex_size - 0.5;
        let texel_i = vec2<i32>(floor(texel_f));
        let frac = fract(texel_f);
        
        let wrap = vec2<i32>(i32(CELL_GRID_SIZE));
        let c00 = textureLoad(cell_height_field_read, (texel_i + vec2<i32>(0, 0) + wrap) % wrap, 0).x;
        let c10 = textureLoad(cell_height_field_read, (texel_i + vec2<i32>(1, 0) + wrap) % wrap, 0).x;
        let c01 = textureLoad(cell_height_field_read, (texel_i + vec2<i32>(0, 1) + wrap) % wrap, 0).x;
        let c11 = textureLoad(cell_height_field_read, (texel_i + vec2<i32>(1, 1) + wrap) % wrap, 0).x;
        
        h_cell = mix(mix(c00, c10, frac.x), mix(c01, c11, frac.x), frac.y);
        
    } else {
        // TOWER: Nearest sample + distance-based falloff
        h_cell = textureSampleLevel(cell_height_field_read, nearest_sampler, uv, 0.0).x;
        
        // Cell geometry (these could be uniforms but compiler should optimize)
        let cell_size = CHART_EXTENT / f32(CELL_GRID_SIZE);
        let cell_center = (floor(p.xz / cell_size) + 0.5) * cell_size;
        let dist_to_center = length(p.xz - cell_center);
        h_cell *= 1.0 - smoothstep(0.0, cell_size * 0.4, dist_to_center);
    }
    
    // ─── Combined Height ────────────────────────────────────────────────────
    return h_wave + h_cell * CELL_HEIGHT_SCALE;
}

fn sdf_terrain(p: vec3<f32>) -> f32 {
    // Returns TRUE signed distance from point to terrain surface
    // Positive = above terrain, Negative = inside terrain
    
    // Wave heightfield (base terrain)
    let h = sdf_terrain_height(p);
    let d_wave = p.y - h;
    
    // VOXEL mode: combine wave surface with solid box geometry
    if (CELL_HEIGHT_ENABLED && CELL_HEIGHT_STYLE == CELL_HEIGHT_STYLE_VOXEL) {
        // Skip voxel check if clearly above all possible boxes
        let max_box_height = CELL_HEIGHT_ALIVE * CELL_HEIGHT_SCALE * 2.0;
        if (d_wave > max_box_height) {
            return d_wave;  // Above wave + max possible box height
        }
        
        let d_voxels = sdf_cell_voxels(p);
        return min(d_wave, d_voxels);
    }
    
    return d_wave;
}

fn sdf_terrain_lipschitz() -> f32 {
    // Returns the Lipschitz factor for safe ray stepping over terrain
    // VOXEL mode uses exact box SDFs, so only wave Lipschitz applies
    // Other modes need additional scaling for cell height discontinuities
    if (CELL_HEIGHT_ENABLED && CELL_HEIGHT_STYLE != CELL_HEIGHT_STYLE_VOXEL) {
        return render_terrain.lipschitz_factor * CELL_HEIGHT_LIPSCHITZ_ADJUST;
    }
    return render_terrain.lipschitz_factor;
}

fn sdf_pawn(p: vec3<f32>) -> f32 {
    let p_local = quat_rotate(quat_inverse(render_pawn.orientation), p - render_pawn.pos);
    return sdf_round_cone(p_local, PAWN_RADIUS, PAWN_TIP, PAWN_HEIGHT);
}

fn sdf_scene(p: vec3<f32>) -> SceneHit {
    // ═══════════════════════════════════════════════════════════════════════════
    // SCENE SDF — Returns TRUE distance for hit detection + safe step bound
    // ═══════════════════════════════════════════════════════════════════════════
    //
    // All objects return TRUE Euclidean distance for correct comparison.
    // Step bound is the minimum conservative bound across all objects.
    // For exact SDFs (pawn, sphere): step_bound = true_dist
    // For heightfields (terrain): step_bound = true_dist / lipschitz
    
    let d_pawn = sdf_pawn(p);
    let d_sphere = sdf_sphere(p);
    
    // Track closest object (using TRUE distances)
    var closest_dist = d_pawn;
    var closest_material = MAT_PAWN;
    
    // Track minimum step bound (conservative for safe stepping)
    var min_step = d_pawn;  // Pawn is exact SDF
    
    // Check sphere (exact SDF)
    if (d_sphere < closest_dist) {
        closest_dist = d_sphere;
        closest_material = MAT_SPHERE;
    }
    min_step = min(min_step, d_sphere);
    
    // Check ground (heightfield - needs Lipschitz scaling for step bound)
    let half = CHART_EXTENT * 0.5;
    let in_bounds = abs(p.x) < half && abs(p.z) < half;
    
    if (in_bounds) {
        let d_terrain = sdf_terrain(p);  // TRUE distance
        
        if (d_terrain < closest_dist) {
            closest_dist = d_terrain;
            closest_material = MAT_TERRAIN;
        }
        
        // Terrain step bound must be conservative (scaled by Lipschitz)
        let terrain_step = d_terrain / sdf_terrain_lipschitz();
        min_step = min(min_step, terrain_step);
    }
    
    return SceneHit(closest_dist, min_step, closest_material);
}

fn sdf_calc_normal(p: vec3<f32>) -> vec3<f32> {
    let eps = 0.001;
    let d = sdf_scene(p).dist;
    return normalize(vec3(
        sdf_scene(p + vec3(eps, 0.0, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, eps, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, 0.0, eps)).dist - d
    ));
}

fn sdf_terrain_normal_from_texture(p: vec3<f32>) -> vec3<f32> {
    let uv = chart_world_to_uv(p.xz);
    let data = textureSampleLevel(terrain_height_field_read, bilinear_sampler, uv, 0.0);
    return normalize(vec3(-data.y, 1.0, -data.z));
}

fn sdf_terrain_normal_box_aware(p: vec3<f32>) -> vec3<f32> {
    // ═══════════════════════════════════════════════════════════════════════════
    // BOX-AWARE NORMAL — Respects vertical walls of BLOCKY cell geometry
    // ═══════════════════════════════════════════════════════════════════════════
    //
    // Detects if hit point is on a wall or top by comparing 3D distances.
    // A wall exists between this cell and a shorter neighbor.
    
    let cell_size = CHART_EXTENT / f32(CELL_GRID_SIZE);
    let half_extent = CHART_EXTENT * 0.5;
    
    // Find which cell we're in
    let cell_f = (p.xz + half_extent) / cell_size;
    let cell_idx = vec2<i32>(floor(cell_f));
    let cell_frac = fract(cell_f);  // Position within cell [0,1]
    
    // Cell center in world space
    let cell_center_xz = (vec2<f32>(cell_idx) + 0.5) * cell_size - half_extent;
    
    // Sample wave height at cell center
    let wave_uv = (cell_center_xz / CHART_EXTENT) + 0.5;
    let wave_h = textureSampleLevel(terrain_height_field_read, bilinear_sampler, wave_uv, 0.0).x;
    
    // Sample this cell's height
    let wrap = vec2<i32>(i32(CELL_GRID_SIZE));
    let h_center = textureLoad(cell_height_field_read, (cell_idx + wrap) % wrap, 0).x * CELL_HEIGHT_SCALE;
    
    // If this cell has no height, use wave normal
    if (h_center < 0.01) {
        return sdf_terrain_normal_from_texture(p);
    }
    
    // Sample 4 cardinal neighbors
    let h_east  = textureLoad(cell_height_field_read, (cell_idx + vec2<i32>( 1, 0) + wrap) % wrap, 0).x * CELL_HEIGHT_SCALE;
    let h_west  = textureLoad(cell_height_field_read, (cell_idx + vec2<i32>(-1, 0) + wrap) % wrap, 0).x * CELL_HEIGHT_SCALE;
    let h_north = textureLoad(cell_height_field_read, (cell_idx + vec2<i32>( 0, 1) + wrap) % wrap, 0).x * CELL_HEIGHT_SCALE;
    let h_south = textureLoad(cell_height_field_read, (cell_idx + vec2<i32>( 0,-1) + wrap) % wrap, 0).x * CELL_HEIGHT_SCALE;
    
    // Height relative to wave surface
    let p_height = p.y - wave_h;
    let box_top = h_center;
    
    // If clearly above box, use wave normal
    if (p_height > box_top + 0.05) {
        return sdf_terrain_normal_from_texture(p);
    }
    
    // Distance to box top (vertical)
    let dist_to_top = abs(p_height - box_top);
    
    // Distance to each wall (horizontal, only if wall exists)
    let dist_to_east_wall  = (1.0 - cell_frac.x) * cell_size;
    let dist_to_west_wall  = cell_frac.x * cell_size;
    let dist_to_north_wall = (1.0 - cell_frac.y) * cell_size;
    let dist_to_south_wall = cell_frac.y * cell_size;
    
    // Check each potential wall - wall exists if we're higher than neighbor
    // and our height is in the "wall zone" (between neighbor top and our top)
    var best_dist = dist_to_top;
    var best_normal = vec3(0.0, 1.0, 0.0);  // Default: top face
    
    // East wall
    if (h_center > h_east + 0.01 && p_height > h_east && p_height <= box_top) {
        if (dist_to_east_wall < best_dist) {
            best_dist = dist_to_east_wall;
            best_normal = vec3(1.0, 0.0, 0.0);
        }
    }
    
    // West wall
    if (h_center > h_west + 0.01 && p_height > h_west && p_height <= box_top) {
        if (dist_to_west_wall < best_dist) {
            best_dist = dist_to_west_wall;
            best_normal = vec3(-1.0, 0.0, 0.0);
        }
    }
    
    // North wall
    if (h_center > h_north + 0.01 && p_height > h_north && p_height <= box_top) {
        if (dist_to_north_wall < best_dist) {
            best_dist = dist_to_north_wall;
            best_normal = vec3(0.0, 0.0, 1.0);
        }
    }
    
    // South wall
    if (h_center > h_south + 0.01 && p_height > h_south && p_height <= box_top) {
        if (dist_to_south_wall < best_dist) {
            best_dist = dist_to_south_wall;
            best_normal = vec3(0.0, 0.0, -1.0);
        }
    }
    
    // If best is top face, use wave normal for organic feel
    if (best_normal.y > 0.5) {
        return sdf_terrain_normal_from_texture(p);
    }
    
    return best_normal;
}

// ═══════════════════════════════════════════════════════════════════════════════
// §6.2 RAYMARCH
// ═══════════════════════════════════════════════════════════════════════════════

fn raymarch_get_direction(uv: vec2<f32>) -> vec3<f32> {
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

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result = RayHit(false, vec3(0.0), 0.0, MAT_SKY);
    var t: f32 = 0.0;
    
    for (var i: i32 = 0; i < MAX_STEPS; i++) {
        let p = ro + rd * t;
        let scene = sdf_scene(p);
        
        // Hit detection uses conservative step_dist (accounts for Lipschitz scaling)
        // This matches the original behavior where scaled terrain distance was used
        if (scene.step_dist < SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = t;
            result.material = scene.material;
            break;
        }
        
        if (t > MAX_DIST) {
            break;
        }
        
        // Stepping also uses conservative bound
        t += scene.step_dist * RAYMARCH_STEP_FACTOR;
    }
    
    result.dist = t;
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// §6.3 SHADING
// ═══════════════════════════════════════════════════════════════════════════════

fn shade_terrain(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    var sample_pos = pos.xz;
    
    // For walls (horizontal normal), sample color from cell center
    // This ensures walls show the cell's color, not boundary artifacts
    let is_wall = abs(normal.y) < 0.1;
    if (is_wall) {
        let cell_size = CHART_EXTENT / f32(CELL_GRID_SIZE);
        let half_extent = CHART_EXTENT * 0.5;
        let cell_idx = floor((pos.xz + half_extent) / cell_size);
        sample_pos = (cell_idx + 0.5) * cell_size - half_extent;
    }
    
    let tile_uv = chart_world_to_uv(sample_pos);
    let tile = textureSampleLevel(terrain_surface_color_read, nearest_sampler, tile_uv, 0.0);
    let color = tile.rgb;
    let ndotl = max(dot(normal, light_dir), 0.0);
    
    // Walls get boosted ambient since light is mostly vertical
    let wall_boost = 0.25;  // Tweak this: 0.0 = no boost, 0.5 = much brighter
    let ambient = select(0.35, 0.35 + wall_boost, is_wall);
    
    return color * (ambient + (1.0 - ambient) * ndotl);
}

fn shade_pawn(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    return COLOR_PAWN * (0.3 + 0.7 * ndotl);
}

fn shade_sphere(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    // Use sphere's coupled color (driven by polyphony via PGA Color Motor)
    return render_sphere.color * (0.3 + 0.7 * ndotl);
}

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return COLOR_SKY;
    }
    
    let light_dir = normalize(vec3(0.3, 1.0, 0.2));
    var color: vec3<f32>;
    var normal: vec3<f32>;
    
    if (hit.material == MAT_TERRAIN) {
        // Use box-aware normals for BLOCKY mode (vertical walls)
        // Other modes use wave-only normals from texture
        if (CELL_HEIGHT_ENABLED && CELL_HEIGHT_STYLE == CELL_HEIGHT_STYLE_BLOCKY) {
            normal = sdf_terrain_normal_box_aware(hit.pos);
        } else {
            normal = sdf_terrain_normal_from_texture(hit.pos);
        }
        color = shade_terrain(hit.pos, normal, light_dir);
    } else if (hit.material == MAT_PAWN) {
        normal = sdf_calc_normal(hit.pos);
        color = shade_pawn(hit.pos, normal, light_dir);
    } else if (hit.material == MAT_SPHERE) {
        normal = sdf_calc_normal(hit.pos);
        color = shade_sphere(hit.pos, normal, light_dir);
    } else {
        return COLOR_SKY;
    }
    
    // View angle: 0 at grazing, 1 at direct view
    let ndotv = abs(dot(normal, rd));
    let grazing = pow(1.0 - ndotv, 2.0);  // High at grazing angles
    
    // At grazing angles, fog picks up surface color instead of pure sky
    let tinted_fog = mix(COLOR_FOG, color, grazing * 0.5);
    
    let fog = 1.0 - exp(-hit.dist * 0.003);
    return mix(color, tinted_fog, fog);
}

// ═══════════════════════════════════════════════════════════════════════════════
// §7 INFRASTRUCTURE
// ═══════════════════════════════════════════════════════════════════════════════

// GPU memory layout and entry points.

// ═══════════════════════════════════════════════════════════════════════════════
// §7.1 BINDINGS
// ═══════════════════════════════════════════════════════════════════════════════

@group(0) @binding(0) var<uniform>             signal: FrameSignal;
@group(0) @binding(1) var<uniform>             config: DesignConfig;
@group(0) @binding(2) var<storage, read_write> terrain_state: TerrainState;
@group(0) @binding(3) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(4) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(5) var<storage, read_write> sphere_state: SphereState;
@group(0) @binding(6) var<storage, read_write> trajectories: array<Trajectory, 16>;
@group(0) @binding(7) var<storage, read_write> terrain_proximity_field: array<Trajectory, 4096>;
@group(0) @binding(8) var<storage, read_write> terrain_cells: array<CellState, 4096>;

@group(1) @binding(0) var terrain_height_field_write: texture_storage_2d<rgba16float, write>;
@group(1) @binding(1) var terrain_surface_color_write: texture_storage_2d<rgba8unorm, write>;
@group(1) @binding(2) var cell_height_field_write: texture_storage_2d<r32float, write>;

// ─── [BINDINGS:render] ──────────────────────────────────────────────────────

@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_terrain: TerrainState;
@group(0) @binding(12) var<storage, read> render_pawn: PawnState;
@group(0) @binding(13) var<storage, read> render_camera: CameraState;
@group(0) @binding(14) var<storage, read> render_sphere: SphereState;

@group(1) @binding(10) var terrain_height_field_read: texture_2d<f32>;
@group(1) @binding(11) var terrain_surface_color_read: texture_2d<f32>;
@group(1) @binding(12) var bilinear_sampler: sampler;
@group(1) @binding(13) var nearest_sampler: sampler;
@group(1) @binding(14) var cell_height_field_read: texture_2d<f32>;


// ═══════════════════════════════════════════════════════════════════════════════
// §7.2 ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════
//
// Execution order (critical for correctness):
//   1. update_world         — 0D entity state (terrain amplitude, pawn, camera, sphere)
//   2. update_height_field  — 2D terrain height computation (waves)
//   3. sync_life_state      — Game of Life synchronization (life_next → life)
//   4. update_cells         — 2D cell stimulus and Game of Life rules
//   5. update_cell_heights  — 2D cell height field (separate from waves)
//   6. update_tiles         — 2D surface color computation
//   7. render               — Fragment shader observation

@compute @workgroup_size(1)
fn update_world() {
    compose_entities_0d_update(signal.dt, signal.t_beats);
}


@compute @workgroup_size(8, 8)
fn update_height_field(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CHART_HEIGHT_RESOLUTION || id.y >= CHART_HEIGHT_RESOLUTION) {
        return;
    }
    compose_terrain_height_2d_update(id.xy, signal.t_beats);
}


@compute @workgroup_size(8, 8)
fn sync_life_state(@builtin(global_invocation_id) id: vec3<u32>) {
    // ═══════════════════════════════════════════════════════════════════════
    // SYNC LIFE STATE — Mathematically correct Game of Life synchronization
    // ═══════════════════════════════════════════════════════════════════════
    //
    // This compute pass applies the previous generation's computed result
    // (life_next) to the display state (life) for ALL cells before any
    // neighbor counting begins in update_cells.
    //
    // Execution order:
    //   1. update_world
    //   2. update_height_field
    //   3. sync_life_state        ← This pass (copies life_next → life)
    //   4. update_cells           ← Reads synchronized life state
    //   5. update_tiles
    //   6. render
    //
    // This guarantees perfect determinism: all cells read from the same
    // generation when counting neighbors, eliminating any race conditions.
    
    if (id.x >= CELL_GRID_SIZE || id.y >= CELL_GRID_SIZE) {
        return;
    }
    
    let idx = cell_index(id.xy);
    // Simple copy: apply previous generation's result
    terrain_cells[idx].life = terrain_cells[idx].life_next;
}


@compute @workgroup_size(8, 8)
fn update_cells(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CELL_GRID_SIZE || id.y >= CELL_GRID_SIZE) {
        return;
    }
    compose_cells_2d_update(id.xy, signal.dt);
}


@compute @workgroup_size(8, 8)
fn update_cell_heights(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CELL_GRID_SIZE || id.y >= CELL_GRID_SIZE) {
        return;
    }
    compose_cell_height_2d_update(id.xy);
}


@compute @workgroup_size(8, 8)
fn update_tiles(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CHART_SURFACE_COLOR_RESOLUTION || id.y >= CHART_SURFACE_COLOR_RESOLUTION) {
        return;
    }
    compose_surface_color_2d_update(id.xy);
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
    
    let rd = raymarch_get_direction(corrected_uv);
    let hit = raymarch(render_camera.pos, rd);
    let color = shade(hit, rd);
    
    return vec4(color, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// END OF SCROLL
// ═══════════════════════════════════════════════════════════════════════════════
