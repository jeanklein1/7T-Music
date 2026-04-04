// ═══════════════════════════════════════════════════════════════════════════════
// N-DIMENSIONAL_2 CARTRIDGE — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// A pawn and orbiting sphere affecting terrain color through spatial memory.
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ Chart:     Plane, 100×100 world units, 256² height texels, 64² tile texels  │
// │ Fields:    height [WaveSum], tile_color [TrajectoryField]                   │
// │ Entities:  pawn [RoundCone], camera [Orbital], sphere [Sphere]              │
// │ Couplings: 9 total                                                          │
// │            • 1 signal→substrate  (polyphony → amplitude)                    │
// │            • 2 substrate→entity  (height/normal → pawn)                     │
// │            • 1 entity→entity     (pawn → camera)                            │
// │            • 3 input→entity      (arrows→pawn, mouse→camera, scroll→zoom)   │
// │            • 2 entity→substrate  (pawn/sphere proximity → field color)      │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// COUPLING GRAPH:
//
//     signal.polyphony ────▶ trajectory ────▶ terrain.amplitude
//                                                    │
//     input.arrows ────▶ pawn.velocity ────▶ pawn.position
//                                                    │
//                           ┌────────────────────────┼────────────────────┐
//                           ▼                        ▼                    │
//                    terrain.height(xz)      terrain.normal(xz)           │
//                           │                        │                    │
//                           ▼                        ▼                    │
//                       pawn.y                pawn.orientation            │
//                           │                                             │
//                           └────────────────▶ camera.target              │
//                                                    │                    │
//     input.mouse ─────────────────────────▶ camera.orbit                 │
//     input.scroll ────────────────────────▶ camera.distance              │
//                                                    │                    │
//                                                    ▼                    │
//                                              camera.pos                 │
//                                                                         │
//     pawn.position ──────────────────────▶ trajectory_field(xz) ─────────┤
//           │                                       │                     │
//           │   (proximity stimulus)                │ (spring/release)    │
//           │                                       │                     │
//     sphere.position ────────────────────▶ trajectory_field(xz) ─────────┘
//           │                                       │
//           │   (3D distance → 2D footprint)        │
//           └───────────────────────────────────────┴──▶ tile_color(xz)
//
// SCROLL INDEX:
//   §1  CONFIG ............................ line 66
//   §2  CHART ............................. line 157
//   §3  FIELDS ............................ line 198
//   §4  ENTITIES .......................... line 325
//   §5  COUPLINGS ......................... line 452
//   §6  COMPOSE ........................... line 590
//   §7  RENDER ............................ line 791
//   §8  BINDINGS .......................... line 975
//   §9  ENTRY POINTS ...................... line 1032
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// §1 CONFIG — Muting control and design parameters
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [CONFIG:muting] ────────────────────────────────────────────────────────
//
// Muting allows isolating parts of the system for design and debugging.
// Must match state.hpp Coupling:: namespace.
//
// ┌─────┬────────────────────────────────┬───────────────────────────────────────┐
// │ Bit │ Coupling                       │ Flow                                  │
// ├─────┼────────────────────────────────┼───────────────────────────────────────┤
// │  0  │ POLYPHONY_TO_AMPLITUDE         │ signal.poly → terrain.amplitude       │
// │  1  │ TERRAIN_TO_PAWN_Y              │ terrain.h(xz) → pawn.y                │
// │  2  │ TERRAIN_TO_PAWN_TILT           │ terrain.n(xz) → pawn.orientation      │
// │  3  │ PAWN_TO_CAMERA_TARGET          │ pawn.pos → camera.target              │
// │  4  │ INPUT_MOVES_PAWN               │ arrows → pawn.velocity                │
// │  5  │ INPUT_ORBITS_CAMERA            │ mouse.drag → camera.az/el             │
// │  6  │ INPUT_ZOOMS_CAMERA             │ mouse.scroll → camera.distance        │
// │  7  │ PAWN_TO_FIELD_COLOR            │ pawn.proximity → field.color_offset   │
// │  8  │ SPHERE_TO_FIELD_COLOR          │ sphere.proximity → field.color_offset │
// └─────┴────────────────────────────────┴───────────────────────────────────────┘

const COUPLING_POLYPHONY_TO_AMPLITUDE:  u32 = 1u << 0u;
const COUPLING_TERRAIN_TO_PAWN_Y:       u32 = 1u << 1u;
const COUPLING_TERRAIN_TO_PAWN_TILT:    u32 = 1u << 2u;
const COUPLING_PAWN_TO_CAMERA_TARGET:   u32 = 1u << 3u;
const COUPLING_INPUT_MOVES_PAWN:        u32 = 1u << 4u;
const COUPLING_INPUT_ORBITS_CAMERA:     u32 = 1u << 5u;
const COUPLING_INPUT_ZOOMS_CAMERA:      u32 = 1u << 6u;
const COUPLING_PAWN_TO_FIELD_COLOR:     u32 = 1u << 7u;
const COUPLING_SPHERE_TO_FIELD_COLOR:   u32 = 1u << 8u;

// ─── [CONFIG:struct] ────────────────────────────────────────────────────────
// Must match state.hpp GPUDesignConfig exactly.

struct DesignConfig {
    mute_dynamics_0d: u32,
    mute_dynamics_2d: u32,
    mute_signal: u32,
    mute_couplings: u32,
    wave_time_scale: f32,
    pawn_speed: f32,
    camera_sensitivity: f32,
    _pad0: f32,
}

// ─── [CONFIG:queries] ───────────────────────────────────────────────────────

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


// ═══════════════════════════════════════════════════════════════════════════════
// §1.2 IDLE STATE — The design at rest
// ═══════════════════════════════════════════════════════════════════════════════
//
// When all couplings are muted and signal is zeroed, this is what you see.
// This is the "instrument" before it is played.
// Must match state.hpp Idle:: namespace.
//
// Substrate:
//   • height field: waves animate at IDLE amplitude (no musical boost)
//   • tile color: static checkerboard pattern
//
// Entities:
//   • pawn: origin, heading north, upright
//   • camera: behind and above pawn, default orbit

const IDLE_AMPLITUDE_SCALE: f32 = 1.0;
const IDLE_PAWN_POS: vec3<f32> = vec3(0.0, 0.0, 0.0);
const IDLE_PAWN_HEADING: f32 = 0.0;
const IDLE_PAWN_ORIENTATION: vec4<f32> = vec4(0.0, 0.0, 0.0, 1.0);
const IDLE_CAMERA_AZIMUTH: f32 = 0.0;
const IDLE_CAMERA_ELEVATION: f32 = 0.4;
const IDLE_CAMERA_DISTANCE: f32 = 30.0;


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CHART — Coordinate system
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [CHART:plane] ──────────────────────────────────────────────────────────
//
// Topology: Plane with clamp boundary
// Domain:   (x, z) ∈ [-50, 50]²
// Texels:   256 × 256 for height field
//           64 × 64 for tile state

const CHART_EXTENT: f32 = 100.0;
const CHART_HEIGHT_RESOLUTION: u32 = 256u;
const CHART_TILE_RESOLUTION: u32 = 64u;

// ─── [CHART:transforms] ─────────────────────────────────────────────────────

fn chart_world_to_uv(world_xz: vec2<f32>) -> vec2<f32> {
    return (world_xz / CHART_EXTENT) + 0.5;
}

fn chart_uv_to_world(uv: vec2<f32>) -> vec2<f32> {
    return (uv - 0.5) * CHART_EXTENT;
}

fn chart_height_texel_to_world(texel: vec2<u32>) -> vec2<f32> {
    let uv = vec2<f32>(texel) / f32(CHART_HEIGHT_RESOLUTION);
    return chart_uv_to_world(uv);
}

fn chart_tile_texel_to_world(texel: vec2<u32>) -> vec2<f32> {
    let uv = vec2<f32>(texel) / f32(CHART_TILE_RESOLUTION);
    return chart_uv_to_world(uv);
}

fn chart_clamp(pos: vec2<f32>) -> vec2<f32> {
    let half = CHART_EXTENT * 0.5 - 1.0;
    return clamp(pos, vec2(-half), vec2(half));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 FIELDS — Substrate data and dynamics
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [FIELD:height] ─────────────────────────────────────────────────────────
//
// Type:     Scalar (stored in texture .x, gradient in .yz)
// Dynamics: WaveSum — superposition of traveling sine waves
// Idle:     0.0 (flat plane at amplitude_scale = 1.0)
// Modulated by: amplitude_scale (from polyphony coupling)

struct WaveComponent {
    amplitude: f32,
    frequency: f32,
    dir: vec2<f32>,
    period: f32,
}

const WAVE_COUNT: i32 = 3;
const WAVES = array<WaveComponent, 3>(
    WaveComponent(1.0, 0.15, vec2(0.7, 0.7),   8.0),
    WaveComponent(0.5, 0.30, vec2(-0.5, 0.8),  6.0),
    WaveComponent(0.3, 0.50, vec2(0.9, -0.3),  4.0),
);

const HEIGHT_MAX_AMPLITUDE: f32 = 2.0;

fn field_height_single_wave(pos: vec2<f32>, t: f32, w: WaveComponent) -> f32 {
    let spatial = w.frequency * dot(w.dir, pos);
    let temporal = (6.28318 / w.period) * t;
    return w.amplitude * sin(spatial + temporal);
}

fn field_height_eval(pos: vec2<f32>, t: f32, amplitude_scale: f32) -> f32 {
    var h: f32 = 0.0;
    for (var i: i32 = 0; i < WAVE_COUNT; i++) {
        h += field_height_single_wave(pos, t, WAVES[i]);
    }
    return h * amplitude_scale * HEIGHT_MAX_AMPLITUDE;
}

fn field_height_single_gradient(pos: vec2<f32>, t: f32, w: WaveComponent) -> vec2<f32> {
    let spatial = w.frequency * dot(w.dir, pos);
    let temporal = (6.28318 / w.period) * t;
    let c = cos(spatial + temporal);
    return w.amplitude * w.frequency * c * w.dir;
}

fn field_height_gradient(pos: vec2<f32>, t: f32, amplitude_scale: f32) -> vec2<f32> {
    var grad = vec2<f32>(0.0);
    for (var i: i32 = 0; i < WAVE_COUNT; i++) {
        grad += field_height_single_gradient(pos, t, WAVES[i]);
    }
    return grad * amplitude_scale * HEIGHT_MAX_AMPLITUDE;
}

fn field_height_normal(pos: vec2<f32>, t: f32, amplitude_scale: f32) -> vec3<f32> {
    let grad = field_height_gradient(pos, t, amplitude_scale);
    return normalize(vec3(-grad.x, 1.0, -grad.y));
}

fn field_height_max_gradient(amplitude_scale: f32) -> f32 {
    var max_grad: f32 = 0.0;
    for (var i: i32 = 0; i < WAVE_COUNT; i++) {
        max_grad += WAVES[i].amplitude * WAVES[i].frequency;
    }
    return max_grad * amplitude_scale * HEIGHT_MAX_AMPLITUDE * 1.5;
}

// ─── [FIELD:tile_color] ─────────────────────────────────────────────────────
//
// Type:     vec4 (RGB + activation)
// Dynamics: TrajectoryField — each cell springs toward stimulus, releases to idle
// Idle:     Uniform base color (displacement = 0)
// Modulated by: pawn proximity → trajectory field → color offset

// Base terrain color (uniform)
const TERRAIN_BASE_COLOR: vec3<f32> = vec3(0.55, 0.45, 0.35);

// Color shift direction when displaced from idle
const COLOR_SHIFT: vec3<f32> = vec3(0.4, 0.2, 0.5);

// Trajectory field dimensions (must match TRAJECTORY_FIELD_SIZE in state.hpp)
const TRAJECTORY_FIELD_SIZE: u32 = 64u;

// Pawn influence parameters
const PAWN_INFLUENCE_RADIUS: f32 = 8.0;

// Field dynamics (spring/release characteristics)
const FIELD_ATTACK_STIFFNESS: f32 = 12.0;
const FIELD_ATTACK_DAMPING: f32 = 0.7;
const FIELD_RELEASE_RATE: f32 = 1.5;
const FIELD_MAX_OFFSET: f32 = 0.5;

// ─── [FIELD:tile_color:helpers] ─────────────────────────────────────────────

fn field_tile_index(tile_id: vec2<u32>) -> u32 {
    return tile_id.y * TRAJECTORY_FIELD_SIZE + tile_id.x;
}

fn field_tile_base_color() -> vec3<f32> {
    return TERRAIN_BASE_COLOR;
}

fn field_apply_color_offset(base: vec3<f32>, offset: f32) -> vec3<f32> {
    return clamp(base + COLOR_SHIFT * offset, vec3(0.0), vec3(1.0));
}

// ─── [FIELD:tile_color:stimulus] ────────────────────────────────────────────

fn field_pawn_stimulus(tile_world: vec2<f32>, pawn_xz: vec2<f32>) -> f32 {
    let dist = distance(tile_world, pawn_xz);
    return smoothstep(PAWN_INFLUENCE_RADIUS, 0.0, dist);
}

fn field_sphere_stimulus(tile_world: vec2<f32>, tile_height: f32, sphere: SphereState) -> f32 {
    // Tile position in 3D (on terrain surface)
    let tile_3d = vec3(tile_world.x, tile_height, tile_world.y);
    
    // 3D distance to sphere center
    let dist_3d = distance(tile_3d, sphere.pos);
    
    // Influence falls off with 3D distance
    return smoothstep(sphere.influence_radius, 0.0, dist_3d);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 ENTITIES — Point objects and state
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [ENTITY:pawn] ──────────────────────────────────────────────────────────
//
// Shape:  RoundCone (base_radius=0.5, tip_radius=0.0, height=1.5)
// State:  position (vec3), heading (f32), orientation (quaternion)

const PAWN_RADIUS: f32 = 0.5;
const PAWN_TIP: f32 = 0.0;
const PAWN_HEIGHT: f32 = 1.5;
const PAWN_SPEED: f32 = 15.0;
const PAWN_TURN_SPEED: f32 = 8.0;

struct PawnState {
    pos: vec3<f32>,
    heading: f32,
    orientation: vec4<f32>,
}

// ─── [ENTITY:camera] ────────────────────────────────────────────────────────
//
// Type:   Orbital camera following a target
// State:  azimuth, elevation, distance, pan offset

const CAMERA_FOV: f32 = 1.0;
const CAMERA_MIN_ELEVATION: f32 = -0.5;
const CAMERA_MAX_ELEVATION: f32 = 1.5;
const CAMERA_MIN_DISTANCE: f32 = 5.0;
const CAMERA_MAX_DISTANCE: f32 = 100.0;

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

fn entity_camera_compute_position(aim_point: vec3<f32>, cam: CameraState) -> vec3<f32> {
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

// ─── [ENTITY:sphere] ───────────────────────────────────────────────────────
//
// Shape:  Sphere (radius=2.5, 5× pawn base diameter)
// Motion: Circular orbit around origin, computed from time
// State:  position (vec3), radius (f32), influence_radius (f32)

const SPHERE_RADIUS: f32 = 2.5;
const SPHERE_ORBIT_RADIUS: f32 = 25.0;
const SPHERE_ORBIT_SPEED: f32 = 0.3;
const SPHERE_HOVER_HEIGHT: f32 = 8.0;
const SPHERE_INFLUENCE_RADIUS: f32 = 12.0;

const COLOR_SPHERE: vec3<f32> = vec3(0.9, 0.7, 0.5);

struct SphereState {
    pos: vec3<f32>,
    radius: f32,
    influence_radius: f32,
    _pad0: f32,
    _pad1: f32,
    _pad2: f32,
}

fn entity_sphere_orbit_position(t: f32) -> vec3<f32> {
    let angle = t * SPHERE_ORBIT_SPEED;
    return vec3(
        SPHERE_ORBIT_RADIUS * cos(angle),
        SPHERE_HOVER_HEIGHT,
        SPHERE_ORBIT_RADIUS * sin(angle)
    );
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4.2 ENTITY SHAPES — SDF primitives
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


// ═══════════════════════════════════════════════════════════════════════════════
// §5 COUPLINGS — Flow functions between subsystems
// ═══════════════════════════════════════════════════════════════════════════════
//
// Each coupling is a PURE FUNCTION: (source, current, dt) → new_state
// The COMPOSE section decides when to call them based on mute flags.

// ─── [COUPLING:trajectory] ──────────────────────────────────────────────────

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

// ─── [COUPLING:polyphony_to_amplitude] ──────────────────────────────────────

const AMPLITUDE_ATTACK_STIFFNESS: f32 = 15.0;
const AMPLITUDE_ATTACK_DAMPING: f32 = 0.6;
const AMPLITUDE_RELEASE_RATE: f32 = 0.5;

fn coupling_polyphony_to_amplitude(polyphony: f32, traj: Trajectory, dt: f32) -> Trajectory {
    let goal = IDLE_AMPLITUDE_SCALE * (1.0 + polyphony);
    
    if (polyphony > 0.0) {
        return trajectory_spring(traj, goal, dt,
            AMPLITUDE_ATTACK_STIFFNESS, AMPLITUDE_ATTACK_DAMPING);
    } else {
        return trajectory_release(traj, IDLE_AMPLITUDE_SCALE, dt,
            AMPLITUDE_RELEASE_RATE);
    }
}

// ─── [COUPLING:terrain_to_pawn_y] ───────────────────────────────────────────

fn coupling_terrain_to_pawn_y(pawn_xz: vec2<f32>, t: f32, amplitude_scale: f32) -> f32 {
    return field_height_eval(pawn_xz, t, amplitude_scale);
}

// ─── [COUPLING:terrain_to_pawn_tilt] ────────────────────────────────────────

fn coupling_terrain_to_pawn_tilt(pawn_xz: vec2<f32>, heading: f32, t: f32, amplitude_scale: f32) -> vec4<f32> {
    let normal = field_height_normal(pawn_xz, t, amplitude_scale);
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

// ─── [COUPLING:pawn_to_camera_target] ───────────────────────────────────────

fn coupling_pawn_to_camera_target(pawn_pos: vec3<f32>, cam: CameraState) -> vec3<f32> {
    return entity_camera_compute_position(pawn_pos, cam);
}

// ─── [COUPLING:input_moves_pawn] ────────────────────────────────────────────

fn coupling_input_to_world_velocity(input_dir: vec2<f32>, camera_azimuth: f32) -> vec2<f32> {
    let cos_az = cos(camera_azimuth);
    let sin_az = sin(camera_azimuth);
    return vec2(
        input_dir.x * cos_az + input_dir.y * sin_az,
        -input_dir.x * sin_az + input_dir.y * cos_az
    );
}

fn coupling_velocity_to_heading(velocity: vec2<f32>, current_heading: f32, dt: f32) -> f32 {
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

// ─── [COUPLING:input_orbits_camera] ─────────────────────────────────────────

fn coupling_input_to_camera_orbit(az_delta: f32, el_delta: f32, cam: CameraState) -> CameraState {
    var c = cam;
    c.azimuth += az_delta;
    c.elevation = clamp(c.elevation + el_delta, CAMERA_MIN_ELEVATION, CAMERA_MAX_ELEVATION);
    return c;
}

// ─── [COUPLING:input_zooms_camera] ──────────────────────────────────────────

fn coupling_input_to_camera_zoom(zoom_delta: f32, cam: CameraState) -> CameraState {
    var c = cam;
    c.distance = clamp(c.distance + zoom_delta, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
    return c;
}

fn coupling_input_to_camera_pan(pan_delta: vec2<f32>, cam: CameraState) -> CameraState {
    var c = cam;
    c.pan_x += pan_delta.x * cam.distance * 0.5;
    c.pan_y += pan_delta.y * cam.distance * 0.5;
    return c;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §6 COMPOSE — Orchestration of dynamics and couplings
// ═══════════════════════════════════════════════════════════════════════════════
//
// This is where muting happens. Each section checks flags before calling
// dynamics or coupling functions.

// ─── [COMPOSE:update_world] ─────────────────────────────────────────────────

fn compose_update_world(dt: f32, t: f32) {
    if (!dynamics_0d_active()) {
        return;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Signal → Substrate (amplitude modulation)
    // ═══════════════════════════════════════════════════════════════════════
    
    var amplitude_scale = ground_state.amplitude_scale;
    
    if (signal_active() && coupling_active(COUPLING_POLYPHONY_TO_AMPLITUDE)) {
        let poly = signal.stats[0];
        let traj = trajectories[0];
        let new_traj = coupling_polyphony_to_amplitude(poly, traj, dt);
        trajectories[0] = new_traj;
        amplitude_scale = new_traj.value;
    }
    
    ground_state.amplitude_scale = amplitude_scale;
    
    let max_grad = field_height_max_gradient(amplitude_scale);
    ground_state.lipschitz_factor = sqrt(1.0 + max_grad * max_grad);
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Input → Pawn (movement)
    // ═══════════════════════════════════════════════════════════════════════
    
    var pawn = pawn_state;
    
    if (coupling_active(COUPLING_INPUT_MOVES_PAWN)) {
        let input_dir = vec2(signal.move_x, signal.move_z);
        let world_vel = coupling_input_to_world_velocity(input_dir, camera_state.azimuth);
        
        let speed = select(PAWN_SPEED, config.pawn_speed, config.pawn_speed > 0.0);
        pawn.pos.x += world_vel.x * speed * dt;
        pawn.pos.z += world_vel.y * speed * dt;
        
        let clamped = chart_clamp(pawn.pos.xz);
        pawn.pos.x = clamped.x;
        pawn.pos.z = clamped.y;
        
        pawn.heading = coupling_velocity_to_heading(world_vel, pawn.heading, dt);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Substrate → Pawn (terrain following)
    // ═══════════════════════════════════════════════════════════════════════
    
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = t * time_scale;
    
    if (coupling_active(COUPLING_TERRAIN_TO_PAWN_Y)) {
        pawn.pos.y = coupling_terrain_to_pawn_y(pawn.pos.xz, wave_time, amplitude_scale);
    }
    
    if (coupling_active(COUPLING_TERRAIN_TO_PAWN_TILT)) {
        pawn.orientation = coupling_terrain_to_pawn_tilt(pawn.pos.xz, pawn.heading, wave_time, amplitude_scale);
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
        camera = coupling_input_to_camera_zoom(signal.zoom_delta, camera);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Pawn → Camera (follow target)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (coupling_active(COUPLING_PAWN_TO_CAMERA_TARGET)) {
        camera.pos = coupling_pawn_to_camera_target(pawn_state.pos, camera);
    }
    
    camera_state = camera;
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Time → Sphere (orbital motion)
    // ═══════════════════════════════════════════════════════════════════════
    
    var sphere = sphere_state;
    sphere.pos = entity_sphere_orbit_position(signal.t_seconds);
    sphere_state = sphere;
}

// ─── [COMPOSE:update_height_field] ──────────────────────────────────────────

fn compose_update_height_field(texel: vec2<u32>, t: f32) {
    let world_xz = chart_height_texel_to_world(texel);
    
    var h: f32;
    var grad: vec2<f32>;
    
    if (dynamics_2d_active()) {
        let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
        let wave_time = t * time_scale;
        let amp = ground_state.amplitude_scale;
        
        h = field_height_eval(world_xz, wave_time, amp);
        grad = field_height_gradient(world_xz, wave_time, amp);
    } else {
        h = 0.0;
        grad = vec2(0.0);
    }
    
    textureStore(height_field_write, texel, vec4(h, grad.x, grad.y, 1.0));
}

// ─── [COMPOSE:update_tiles] ─────────────────────────────────────────────────
//
// The core of the 2D trajectory field principle:
//   1. Load trajectory state for this cell
//   2. Compute stimulus from pawn proximity
//   3. Spring toward goal (if stimulus > 0) or release to idle (if stimulus = 0)
//   4. Store updated trajectory state
//   5. Compute final color: base + offset from trajectory

fn compose_update_tiles(texel: vec2<u32>) {
    // ═══════════════════════════════════════════════════════════════════════
    // LOAD: Get current trajectory state for this cell
    // ═══════════════════════════════════════════════════════════════════════
    
    let tile_idx = field_tile_index(texel);
    var traj = trajectory_field[tile_idx];
    
    // ═══════════════════════════════════════════════════════════════════════
    // STIMULUS: Compute combined influence from pawn and sphere
    // ═══════════════════════════════════════════════════════════════════════
    
    let tile_world = chart_tile_texel_to_world(texel);
    
    // Approximate terrain height at this tile (for 3D sphere distance)
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = signal.t_beats * time_scale;
    let tile_height = field_height_eval(tile_world, wave_time, ground_state.amplitude_scale);
    
    var stimulus: f32 = 0.0;
    
    // Pawn stimulus (2D distance on ground plane)
    if (coupling_active(COUPLING_PAWN_TO_FIELD_COLOR)) {
        stimulus += field_pawn_stimulus(tile_world, pawn_state.pos.xz);
    }
    
    // Sphere stimulus (3D distance from floating sphere)
    if (coupling_active(COUPLING_SPHERE_TO_FIELD_COLOR)) {
        stimulus += field_sphere_stimulus(tile_world, tile_height, sphere_state);
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMICS: Spring toward goal or release to idle
    // ═══════════════════════════════════════════════════════════════════════
    
    if (dynamics_2d_active()) {
        if (stimulus > 0.001) {
            // Stimulus present: spring toward goal
            let goal = stimulus * FIELD_MAX_OFFSET;
            traj = trajectory_spring(traj, goal, signal.dt,
                                     FIELD_ATTACK_STIFFNESS, FIELD_ATTACK_DAMPING);
        } else {
            // No stimulus: release back to idle (0)
            traj = trajectory_release(traj, 0.0, signal.dt, FIELD_RELEASE_RATE);
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // STORE: Save trajectory state for next frame
    // ═══════════════════════════════════════════════════════════════════════
    
    trajectory_field[tile_idx] = traj;
    
    // ═══════════════════════════════════════════════════════════════════════
    // OUTPUT: Compute final color from base + trajectory offset
    // ═══════════════════════════════════════════════════════════════════════
    
    let base_color = field_tile_base_color();
    let final_color = field_apply_color_offset(base_color, traj.value);
    
    textureStore(tile_state_write, texel, vec4(final_color, 1.0));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 RENDER — Scene observation
// ═══════════════════════════════════════════════════════════════════════════════

const MAX_STEPS: i32 = 128;
const MAX_DIST: f32 = 200.0;
const SURF_DIST: f32 = 0.01;

const COLOR_PAWN: vec3<f32> = vec3(0.8, 0.5, 0.8);
const COLOR_SKY: vec3<f32> = vec3(0.85, 0.78, 0.72);
const COLOR_FOG: vec3<f32> = vec3(0.85, 0.78, 0.72);

const MAT_SKY: i32 = 0;
const MAT_GROUND: i32 = 1;
const MAT_PAWN: i32 = 2;
const MAT_SPHERE: i32 = 3;

// ─── [RENDER:sdf_scene] ─────────────────────────────────────────────────────

fn sdf_ground(p: vec3<f32>) -> f32 {
    let uv = chart_world_to_uv(p.xz);
    let data = textureSampleLevel(height_field_read, bilinear_sampler, uv, 0.0);
    let h = data.x;
    return (p.y - h) / render_ground.lipschitz_factor;
}

fn sdf_pawn(p: vec3<f32>) -> f32 {
    let p_local = quat_rotate(quat_inverse(render_pawn.orientation), p - render_pawn.pos);
    return sdf_round_cone(p_local, PAWN_RADIUS, PAWN_TIP, PAWN_HEIGHT);
}

fn sdf_sphere(p: vec3<f32>) -> f32 {
    return length(p - render_sphere.pos) - render_sphere.radius;
}

struct SceneHit {
    dist: f32,
    material: i32,
}

fn sdf_scene(p: vec3<f32>) -> SceneHit {
    let d_pawn = sdf_pawn(p);
    let d_sphere = sdf_sphere(p);
    
    // Start with pawn as closest
    var closest = SceneHit(d_pawn, MAT_PAWN);
    
    // Check sphere
    if (d_sphere < closest.dist) {
        closest = SceneHit(d_sphere, MAT_SPHERE);
    }
    
    // Check ground (only within bounds)
    let half = CHART_EXTENT * 0.5;
    let in_bounds = abs(p.x) < half && abs(p.z) < half;
    
    if (in_bounds) {
        let d_ground = sdf_ground(p);
        if (d_ground < closest.dist) {
            closest = SceneHit(d_ground, MAT_GROUND);
        }
    }
    
    return closest;
}

// ─── [RENDER:raymarch] ──────────────────────────────────────────────────────

struct RayHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
    material: i32,
}

fn raymarch(ro: vec3<f32>, rd: vec3<f32>) -> RayHit {
    var result = RayHit(false, vec3(0.0), 0.0, MAT_SKY);
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
    return normalize(vec3(
        sdf_scene(p + vec3(eps, 0.0, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, eps, 0.0)).dist - d,
        sdf_scene(p + vec3(0.0, 0.0, eps)).dist - d
    ));
}

fn ground_normal_from_texture(p: vec3<f32>) -> vec3<f32> {
    let uv = chart_world_to_uv(p.xz);
    let data = textureSampleLevel(height_field_read, bilinear_sampler, uv, 0.0);
    return normalize(vec3(-data.y, 1.0, -data.z));
}

// ─── [RENDER:shading] ───────────────────────────────────────────────────────

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

fn shade_ground(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let tile_uv = chart_world_to_uv(pos.xz);
    let tile = textureSampleLevel(tile_state_read, nearest_sampler, tile_uv, 0.0);
    let color = tile.rgb;
    let ndotl = max(dot(normal, light_dir), 0.0);
    return color * (0.35 + 0.65 * ndotl);
}

fn shade_pawn(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    return COLOR_PAWN * (0.3 + 0.7 * ndotl);
}

fn shade_sphere(pos: vec3<f32>, normal: vec3<f32>, light_dir: vec3<f32>) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    return COLOR_SPHERE * (0.3 + 0.7 * ndotl);
}

fn shade(hit: RayHit, rd: vec3<f32>) -> vec3<f32> {
    if (!hit.hit) {
        return COLOR_SKY;
    }
    
    let light_dir = normalize(vec3(0.3, 1.0, 0.2));
    var color: vec3<f32>;
    
    if (hit.material == MAT_GROUND) {
        let normal = ground_normal_from_texture(hit.pos);
        color = shade_ground(hit.pos, normal, light_dir);
    } else if (hit.material == MAT_PAWN) {
        let normal = calc_normal(hit.pos);
        color = shade_pawn(hit.pos, normal, light_dir);
    } else if (hit.material == MAT_SPHERE) {
        let normal = calc_normal(hit.pos);
        color = shade_sphere(hit.pos, normal, light_dir);
    } else {
        color = COLOR_SKY;
    }
    
    let fog = 1.0 - exp(-hit.dist * 0.015);
    return mix(color, COLOR_FOG, fog);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §8 BINDINGS — GPU memory layout
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [BINDINGS:types] ───────────────────────────────────────────────────────

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

struct GroundState {
    amplitude_scale: f32,
    max_amplitude: f32,
    size: f32,
    lipschitz_factor: f32,
}

// ─── [BINDINGS:compute] ─────────────────────────────────────────────────────

@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read>       config: DesignConfig;
@group(0) @binding(2) var<storage, read_write> ground_state: GroundState;
@group(0) @binding(3) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(4) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(5) var<storage, read_write> sphere_state: SphereState;
@group(0) @binding(6) var<storage, read_write> trajectories: array<Trajectory, 16>;
@group(0) @binding(7) var<storage, read_write> trajectory_field: array<Trajectory, 4096>;

@group(1) @binding(0) var height_field_write: texture_storage_2d<rgba16float, write>;
@group(1) @binding(1) var tile_state_write: texture_storage_2d<rgba8unorm, write>;

// ─── [BINDINGS:render] ──────────────────────────────────────────────────────

@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_ground: GroundState;
@group(0) @binding(12) var<storage, read> render_pawn: PawnState;
@group(0) @binding(13) var<storage, read> render_camera: CameraState;
@group(0) @binding(14) var<storage, read> render_sphere: SphereState;

@group(1) @binding(10) var height_field_read: texture_2d<f32>;
@group(1) @binding(11) var tile_state_read: texture_2d<f32>;
@group(1) @binding(12) var bilinear_sampler: sampler;
@group(1) @binding(13) var nearest_sampler: sampler;


// ═══════════════════════════════════════════════════════════════════════════════
// §9 ENTRY POINTS — Compute and render dispatches
// ═══════════════════════════════════════════════════════════════════════════════

@compute @workgroup_size(1)
fn update_world() {
    compose_update_world(signal.dt, signal.t_beats);
}

@compute @workgroup_size(8, 8)
fn update_height_field(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CHART_HEIGHT_RESOLUTION || id.y >= CHART_HEIGHT_RESOLUTION) {
        return;
    }
    compose_update_height_field(id.xy, signal.t_beats);
}

@compute @workgroup_size(8, 8)
fn update_tiles(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CHART_TILE_RESOLUTION || id.y >= CHART_TILE_RESOLUTION) {
        return;
    }
    compose_update_tiles(id.xy);
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
