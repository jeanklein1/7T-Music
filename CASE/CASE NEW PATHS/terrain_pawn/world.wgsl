// world.wgsl
// The Visual World — 7T Musical Visualizer
// ═══════════════════════════════════════════════════════════════════════════════
//
// NAVIGATION: Use Ctrl+F with section headers
//   ═══ SIGNAL ═══     - Input from CPU
//   ═══ STATE ═══      - GPU-owned persistent state
//   ═══ DYNAMICS ═══   - Trajectory update functions
//   ═══ TERRAIN ═══    - Config + field + render
//   ═══ PAWN ═══       - Config + render
//   ═══ CAMERA ═══     - Config + projection
//   ═══ CANVAS ═══     - THE action (all couplings)
//
// ═══════════════════════════════════════════════════════════════════════════════


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ SIGNAL ═══
// Input from CPU, uploaded every frame (304 bytes)
// 
// MEMORY LAYOUT:
//   Offset  Field                Size
//   0       t_seconds            4
//   4       t_beats              4
//   8       dt                   4
//   12      aspect_ratio         4
//   16      stats[64]            256
//   272     move_x               4
//   276     move_z               4
//   280     look_az_delta        4
//   284     look_el_delta        4
//   288     zoom_delta           4
//   292     pan_x_delta          4
//   296     pan_y_delta          4
//   300     _pad1                4
//   304     END
//
// STAT INDEXING: stats[channel * STATS_PER_CHANNEL + stat]
// ═══════════════════════════════════════════════════════════════════════════════

// --- Channel indices ---
// Current: 1 channel (index 0)
// Future:  const CH_BASS: i32 = 0;
//          const CH_HARMONY: i32 = 1;
//          const CH_MELODY: i32 = 2;
//          const CH_DRUMS: i32 = 3;

const STATS_PER_CHANNEL: i32 = 16;

// --- Stat indices ---
// Current: polyphony only
// Future:  const STAT_VELOCITY: i32 = 1;
//          const STAT_CENTROID: i32 = 2;
//          const STAT_GATE: i32 = 3;

const STAT_POLYPHONY: i32 = 0;

// --- Indexing helper ---
fn stat_index(channel: i32, stat: i32) -> i32 {
    return channel * STATS_PER_CHANNEL + stat;
}

// --- Current instantiation: channel 0, polyphony ---
fn polyphony() -> f32 { 
    return signal.stats[stat_index(0, STAT_POLYPHONY)]; 
}

struct FrameSignal {
    // Time + viewport (16 bytes)
    t_seconds: f32,              // 0
    t_beats: f32,                // 4
    dt: f32,                     // 8
    aspect_ratio: f32,           // 12 - width/height for projection
    
    // Musical stats (256 bytes = 64 floats)
    // Indexed by stat_index(channel, stat)
    stats: array<f32, 64>,       // 16
    
    // Input intent (32 bytes)
    move_x: f32,                 // 272
    move_z: f32,                 // 276
    look_az_delta: f32,          // 280
    look_el_delta: f32,          // 284
    zoom_delta: f32,             // 288
    pan_x_delta: f32,            // 292 - camera pan right/left
    pan_y_delta: f32,            // 296 - camera pan up/down
    _pad1: f32,                  // 300
}                                // 304 bytes total


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ STATE ═══
// GPU-owned, persists across frames
//
// CRITICAL: All structs use explicit f32 scalars to guarantee C++ alignment match.
// Never use vec3<f32> as a struct member in storage buffers!
// (vec3 has alignment=16 but size=12, creating hidden gaps)
//
// vec4<f32> and mat4x4<f32> are safe (align=size=16 or 64).
// ═══════════════════════════════════════════════════════════════════════════════

// --- Trajectory (16 bytes) ---
// Used in array, so stride must match C++ sizeof
struct Trajectory {
    value: f32,              // 0
    velocity: f32,           // 4
    _pad0: f32,              // 8
    _pad1: f32,              // 12
}                            // 16 bytes, array stride = 16

// --- TerrainState (16 bytes) ---
// Colors are now shader constants - only dynamic/config values remain
struct TerrainState {
    amplitude_scale: f32,        // 0  - trajectory-driven multiplier
    max_amplitude: f32,          // 4  - config: max height displacement
    size: f32,                   // 8  - world size
    _pad: f32,                   // 12
}                                // 16 bytes

// --- PawnState (32 bytes) ---
struct PawnState {
    // Position as explicit scalars (not vec3!)
    pos_x: f32,              // 0
    pos_y: f32,              // 4
    pos_z: f32,              // 8
    heading: f32,            // 12
    // Orientation quaternion (vec4 is safe: align=16, size=16)
    orientation: vec4<f32>,  // 16
}                            // 32 bytes

// --- CameraState (96 bytes) ---
struct CameraState {
    // Position as explicit scalars
    pos_x: f32,              // 0
    pos_y: f32,              // 4
    pos_z: f32,              // 8
    azimuth: f32,            // 12
    elevation: f32,          // 16
    distance: f32,           // 20
    pan_x: f32,              // 24 - accumulated pan offset (screen right)
    pan_y: f32,              // 28 - accumulated pan offset (screen up)
    // View-projection matrix (mat4x4 is safe: align=16, size=64)
    view_proj: mat4x4<f32>,  // 32
}                            // 96 bytes

// ─── COMPUTE BINDINGS (group 0 in compute pipeline) ─────────────────────────────
// Read-write for compute shader to update state
@group(0) @binding(0) var<storage, read> signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> terrain_state: TerrainState;
@group(0) @binding(2) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(3) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(4) var<storage, read_write> trajectories: array<Trajectory, 16>;

// ─── RENDER BINDINGS (group 0 in render pipeline, different binding indices) ────
// Read-only for vertex/fragment shaders (same buffers, different access mode)
// Uses binding indices 10-13 to avoid collision with compute bindings 0-4.
// The render pipeline layout maps these to the same underlying buffers.
@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_terrain: TerrainState;
@group(0) @binding(12) var<storage, read> render_pawn: PawnState;
@group(0) @binding(13) var<storage, read> render_camera: CameraState;


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ DYNAMICS ═══
// Trajectory update functions
// ═══════════════════════════════════════════════════════════════════════════════

// Reference frame rate for damping normalization (matches C++ REFERENCE_FPS)
const REFERENCE_FPS: f32 = 60.0;

fn step_spring(value: f32, velocity: f32, goal: f32, dt: f32, stiffness: f32, damping: f32) -> vec2<f32> {
    // Spring force toward goal
    let force = (goal - value) * stiffness;
    var new_velocity = velocity + force * dt;
    
    // Frame-rate independent damping: damping is the retention factor at 60fps
    // At 60fps (dt=1/60), velocity *= damping^1
    // At 30fps (dt=1/30), velocity *= damping^2
    // General: velocity *= damping^(dt * 60)
    new_velocity = new_velocity * pow(damping, dt * REFERENCE_FPS);
    
    let new_value = value + new_velocity * dt;
    return vec2<f32>(new_value, new_velocity);
}

fn step_asymptotic(value: f32, goal: f32, dt: f32, rate: f32) -> f32 {
    return value + (goal - value) * (1.0 - exp(-rate * dt));
}


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ TERRAIN ═══
// Configuration, field functions, rendering
// ═══════════════════════════════════════════════════════════════════════════════

// --- Config ---
// Wave parameters for terrain height function (artistic constants)
struct WaveParams {
    amplitude: f32,
    spatial_freq: f32,
    dir_x: f32,
    dir_z: f32,
    cycle_length: f32,
    use_beats: f32,
}

const WAVE_0 = WaveParams(1.0, 0.15, 0.7, 0.7, 8.0, 1.0);
const WAVE_1 = WaveParams(0.5, 0.30, -0.5, 0.8, 6.0, 1.0);
const WAVE_2 = WaveParams(0.3, 0.50, 0.9, -0.3, 4.0, 1.0);

// --- Field Functions (for compute) ---

fn eval_wave(x: f32, z: f32, w: WaveParams, t_seconds: f32, t_beats: f32) -> f32 {
    let t = select(t_seconds, t_beats, w.use_beats > 0.5);
    let spatial = w.spatial_freq * (w.dir_x * x + w.dir_z * z);
    let temporal = (6.28318 / w.cycle_length) * t;
    return w.amplitude * sin(spatial + temporal);
}

fn terrain_height_with_scale(x: f32, z: f32, amp_scale: f32, max_amp: f32, t_seconds: f32, t_beats: f32) -> f32 {
    var h: f32 = 0.0;
    h += eval_wave(x, z, WAVE_0, t_seconds, t_beats);
    h += eval_wave(x, z, WAVE_1, t_seconds, t_beats);
    h += eval_wave(x, z, WAVE_2, t_seconds, t_beats);
    return h * amp_scale * max_amp;
}

fn terrain_normal_with_scale(x: f32, z: f32, amp_scale: f32, max_amp: f32, t_seconds: f32, t_beats: f32) -> vec3<f32> {
    let eps = 0.1;
    let hL = terrain_height_with_scale(x - eps, z, amp_scale, max_amp, t_seconds, t_beats);
    let hR = terrain_height_with_scale(x + eps, z, amp_scale, max_amp, t_seconds, t_beats);
    let hD = terrain_height_with_scale(x, z - eps, amp_scale, max_amp, t_seconds, t_beats);
    let hU = terrain_height_with_scale(x, z + eps, amp_scale, max_amp, t_seconds, t_beats);
    return normalize(vec3<f32>(hL - hR, 2.0 * eps, hD - hU));
}

// Convenience wrappers for compute shader (read from terrain_state buffer)
fn terrain_height(x: f32, z: f32) -> f32 {
    return terrain_height_with_scale(x, z, terrain_state.amplitude_scale, terrain_state.max_amplitude, signal.t_seconds, signal.t_beats);
}

fn terrain_normal(x: f32, z: f32) -> vec3<f32> {
    return terrain_normal_with_scale(x, z, terrain_state.amplitude_scale, terrain_state.max_amplitude, signal.t_seconds, signal.t_beats);
}

// --- Render ---

// --- Terrain Config ---
const TERRAIN_RESOLUTION: u32 = 200u;  // Grid cells per side
const TERRAIN_VERTEX_COUNT: u32 = 200u * 200u * 6u;  // 6 verts per cell (2 triangles)

// Colors (GPU-owned constants)
const TERRAIN_VALLEY_COLOR = vec3<f32>(0.0, 0.5, 0.8);
const TERRAIN_PEAK_COLOR = vec3<f32>(1.0, 0.6, 0.2);
const TERRAIN_FOG_COLOR = vec3<f32>(0.85, 0.78, 0.72);
const TERRAIN_FOG_DENSITY: f32 = 0.015;
const TERRAIN_HEIGHT_RANGE: f32 = 4.0;
const TERRAIN_BRIGHTNESS: f32 = 1.0;

// --- Render (Vertex Pulling) ---

struct TerrainVertexOutput {
    @builtin(position) clip_position: vec4<f32>,
    @location(0) world_pos: vec3<f32>,
    @location(1) height: f32,
}

@vertex
fn terrain_vs(@builtin(vertex_index) vid: u32) -> TerrainVertexOutput {
    // Decode vertex index to grid position
    // 6 vertices per cell: 2 triangles
    // Triangle 0: corners (0,0), (0,1), (1,0)
    // Triangle 1: corners (1,0), (0,1), (1,1)
    
    let cell_id = vid / 6u;
    let corner = vid % 6u;
    
    let cell_x = cell_id % TERRAIN_RESOLUTION;
    let cell_z = cell_id / TERRAIN_RESOLUTION;
    
    // Decode corner to vertex offset within cell
    var dx: u32;
    var dz: u32;
    switch corner {
        case 0u: { dx = 0u; dz = 0u; }  // tri0 v0
        case 1u: { dx = 0u; dz = 1u; }  // tri0 v1
        case 2u: { dx = 1u; dz = 0u; }  // tri0 v2
        case 3u: { dx = 1u; dz = 0u; }  // tri1 v0
        case 4u: { dx = 0u; dz = 1u; }  // tri1 v1
        case 5u: { dx = 1u; dz = 1u; }  // tri1 v2
        default: { dx = 0u; dz = 0u; }
    }
    
    let vx = cell_x + dx;
    let vz = cell_z + dz;
    
    // Convert to world coordinates
    let size = render_terrain.size;
    let res_f = f32(TERRAIN_RESOLUTION);
    let world_x = (f32(vx) / res_f - 0.5) * size;
    let world_z = (f32(vz) / res_f - 0.5) * size;
    
    // Sample height
    let h = terrain_height_with_scale(world_x, world_z, 
                                       render_terrain.amplitude_scale,
                                       render_terrain.max_amplitude,
                                       render_signal.t_seconds, 
                                       render_signal.t_beats);
    
    var out: TerrainVertexOutput;
    out.world_pos = vec3<f32>(world_x, h, world_z);
    out.height = h;
    out.clip_position = render_camera.view_proj * vec4<f32>(world_x, h, world_z, 1.0);
    return out;
}

@fragment
fn terrain_fs(in: TerrainVertexOutput) -> @location(0) vec4<f32> {
    // Height-based coloring
    let scale = max(render_terrain.amplitude_scale * render_terrain.max_amplitude, 0.1);
    let norm_h = in.height / max(TERRAIN_HEIGHT_RANGE, 0.1);
    let t = clamp(norm_h * 0.5 + 0.5, 0.0, 1.0);
    let surface = mix(TERRAIN_VALLEY_COLOR, TERRAIN_PEAK_COLOR, t);
    
    // Apply brightness
    let lit_surface = surface * TERRAIN_BRIGHTNESS;
    
    // Distance fog
    let cam_pos = vec3<f32>(render_camera.pos_x, render_camera.pos_y, render_camera.pos_z);
    let dist = length(in.world_pos - cam_pos);
    let fog = 1.0 - exp(-dist * TERRAIN_FOG_DENSITY);
    let result = mix(lit_surface, TERRAIN_FOG_COLOR, fog);
    
    return vec4<f32>(result, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ PAWN ═══
// Configuration, rendering (vertex pulling)
// ═══════════════════════════════════════════════════════════════════════════════

// --- Config ---
const PAWN_RADIUS: f32 = 0.5;
const PAWN_HEIGHT: f32 = 1.5;
const PAWN_SPEED: f32 = 15.0;
const PAWN_COLOR = vec3<f32>(0.8, 0.5, 0.8);
const PAWN_SEGMENTS: u32 = 16u;
const PAWN_VERTEX_COUNT: u32 = 16u * 3u * 2u;  // body + bottom cap

// --- Quaternion Helpers ---

fn quat_rotate(q: vec4<f32>, v: vec3<f32>) -> vec3<f32> {
    let u = q.xyz;
    let s = q.w;
    return 2.0 * dot(u, v) * u + (s * s - dot(u, u)) * v + 2.0 * s * cross(u, v);
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

// --- Render (Vertex Pulling) ---

struct PawnVertexOutput {
    @builtin(position) clip_position: vec4<f32>,
    @location(0) world_normal: vec3<f32>,
}

@vertex
fn pawn_vs(@builtin(vertex_index) vid: u32) -> PawnVertexOutput {
    let PI = 3.14159265;
    let TWO_PI = 6.28318530;
    
    // Cone body: first PAWN_SEGMENTS * 3 vertices
    // Bottom cap: next PAWN_SEGMENTS * 3 vertices
    let body_verts = PAWN_SEGMENTS * 3u;
    
    var local_pos: vec3<f32>;
    var local_normal: vec3<f32>;
    
    // Compute slope normal components (same for all body vertices)
    let slope_len = sqrt(PAWN_RADIUS * PAWN_RADIUS + PAWN_HEIGHT * PAWN_HEIGHT);
    let ny = PAWN_RADIUS / slope_len;
    let n_radial = PAWN_HEIGHT / slope_len;
    
    if (vid < body_verts) {
        // Cone body triangle
        let tri_id = vid / 3u;
        let tri_vert = vid % 3u;
        
        let angle0 = f32(tri_id) / f32(PAWN_SEGMENTS) * TWO_PI;
        let angle1 = f32(tri_id + 1u) / f32(PAWN_SEGMENTS) * TWO_PI;
        
        if (tri_vert == 0u) {
            // Apex
            local_pos = vec3<f32>(0.0, PAWN_HEIGHT, 0.0);
            // Normal at apex: average of adjacent face normals (pointing up-ish)
            let mid_angle = (angle0 + angle1) * 0.5;
            local_normal = vec3<f32>(cos(mid_angle) * n_radial, ny, sin(mid_angle) * n_radial);
        } else if (tri_vert == 1u) {
            // Base vertex 1 (next segment)
            local_pos = vec3<f32>(cos(angle1) * PAWN_RADIUS, 0.0, sin(angle1) * PAWN_RADIUS);
            local_normal = vec3<f32>(cos(angle1) * n_radial, ny, sin(angle1) * n_radial);
        } else {
            // Base vertex 0 (current segment)
            local_pos = vec3<f32>(cos(angle0) * PAWN_RADIUS, 0.0, sin(angle0) * PAWN_RADIUS);
            local_normal = vec3<f32>(cos(angle0) * n_radial, ny, sin(angle0) * n_radial);
        }
    } else {
        // Bottom cap triangle
        let cap_vid = vid - body_verts;
        let tri_id = cap_vid / 3u;
        let tri_vert = cap_vid % 3u;
        
        let angle0 = f32(tri_id) / f32(PAWN_SEGMENTS) * TWO_PI;
        let angle1 = f32(tri_id + 1u) / f32(PAWN_SEGMENTS) * TWO_PI;
        
        local_normal = vec3<f32>(0.0, -1.0, 0.0);
        
        if (tri_vert == 0u) {
            // Center
            local_pos = vec3<f32>(0.0, 0.0, 0.0);
        } else if (tri_vert == 1u) {
            // Edge vertex 0
            local_pos = vec3<f32>(cos(angle0) * PAWN_RADIUS, 0.0, sin(angle0) * PAWN_RADIUS);
        } else {
            // Edge vertex 1
            local_pos = vec3<f32>(cos(angle1) * PAWN_RADIUS, 0.0, sin(angle1) * PAWN_RADIUS);
        }
    }
    
    // Transform to world space
    let pawn_pos = vec3<f32>(render_pawn.pos_x, render_pawn.pos_y, render_pawn.pos_z);
    let rotated_pos = quat_rotate(render_pawn.orientation, local_pos);
    let world_pos = rotated_pos + pawn_pos;
    
    var out: PawnVertexOutput;
    out.clip_position = render_camera.view_proj * vec4<f32>(world_pos, 1.0);
    out.world_normal = quat_rotate(render_pawn.orientation, local_normal);
    return out;
}

@fragment
fn pawn_fs(in: PawnVertexOutput) -> @location(0) vec4<f32> {
    let light_dir = normalize(vec3<f32>(0.3, 1.0, 0.2));
    let ndotl = max(dot(normalize(in.world_normal), light_dir), 0.0);
    let lit = PAWN_COLOR * (0.3 + 0.7 * ndotl);
    return vec4<f32>(lit, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ CAMERA ═══
// Configuration, projection math
// ═══════════════════════════════════════════════════════════════════════════════

// --- Config ---
// Runtime constraints (GPU enforces these during update)
const CAMERA_MIN_ELEVATION: f32 = 0.1;
const CAMERA_MAX_ELEVATION: f32 = 1.5;
const CAMERA_MIN_DISTANCE: f32 = 5.0;
const CAMERA_MAX_DISTANCE: f32 = 100.0;

// Projection parameters
const CAMERA_FOV: f32 = 1.0;
const CAMERA_NEAR: f32 = 0.1;
const CAMERA_FAR: f32 = 500.0;
const CAMERA_ASPECT: f32 = 1.777;  // fallback if signal.aspect_ratio is 0

// --- Projection ---

fn look_at(eye: vec3<f32>, goal: vec3<f32>, up: vec3<f32>) -> mat4x4<f32> {
    let f = normalize(goal - eye);
    let r = normalize(cross(f, up));
    let u = cross(r, f);
    
    return mat4x4<f32>(
        vec4<f32>(r.x, u.x, -f.x, 0.0),
        vec4<f32>(r.y, u.y, -f.y, 0.0),
        vec4<f32>(r.z, u.z, -f.z, 0.0),
        vec4<f32>(-dot(r, eye), -dot(u, eye), dot(f, eye), 1.0)
    );
}

fn perspective(fov: f32, aspect: f32, near: f32, far: f32) -> mat4x4<f32> {
    let f = 1.0 / tan(fov * 0.5);
    let nf = 1.0 / (near - far);
    
    return mat4x4<f32>(
        vec4<f32>(f / aspect, 0.0, 0.0, 0.0),
        vec4<f32>(0.0, f, 0.0, 0.0),
        vec4<f32>(0.0, 0.0, far * nf, -1.0),
        vec4<f32>(0.0, 0.0, far * near * nf, 0.0)
    );
}


// ═══════════════════════════════════════════════════════════════════════════════
// ═══ CANVAS ═══
// THE ACTION — all dynamics, all couplings
// ═══════════════════════════════════════════════════════════════════════════════

// --- Trajectory Indices ---
const TRAJ_TERRAIN_AMP: i32 = 0;

// --- Coupling Constants ---
const TERRAIN_AMP_IDLE: f32 = 1.0;
const TERRAIN_AMP_ATTACK_STIFFNESS: f32 = 15.0;
const TERRAIN_AMP_ATTACK_DAMPING: f32 = 0.6;
const TERRAIN_AMP_RELEASE_RATE: f32 = 0.5;

@compute @workgroup_size(1)
fn update() {
    let dt = signal.dt;
    
    // ─── MUSICAL COUPLING: amplitude ∝ polyphony ────────────────────────────────
    
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
    
    // ─── PAWN ──────────────────────────────────────────────────────────────────
    
    // Transform movement from camera-relative to world space
    // "Forward" (negative move_z) should move toward where camera is looking
    let cos_az = cos(camera_state.azimuth);
    let sin_az = sin(camera_state.azimuth);
    
    // Rotate input by camera azimuth
    // Camera looks along (sin_az, 0, cos_az), so:
    // - "forward" (move_z < 0) moves toward camera's forward
    // - "right" (move_x > 0) moves toward camera's right
    let world_move_x = signal.move_x * cos_az + signal.move_z * sin_az;
    let world_move_z = -signal.move_x * sin_az + signal.move_z * cos_az;
    
    // Apply movement
    pawn_state.pos_x += world_move_x * PAWN_SPEED * dt;
    pawn_state.pos_z += world_move_z * PAWN_SPEED * dt;
    
    // Clamp to terrain bounds (read size from buffer)
    let half = terrain_state.size * 0.5 - 1.0;
    pawn_state.pos_x = clamp(pawn_state.pos_x, -half, half);
    pawn_state.pos_z = clamp(pawn_state.pos_z, -half, half);
    
    // Grounding
    pawn_state.pos_y = terrain_height(pawn_state.pos_x, pawn_state.pos_z);
    
    // Update heading to face movement direction (if moving)
    let move_len_sq = world_move_x * world_move_x + world_move_z * world_move_z;
    if (move_len_sq > 0.001) {
        // atan2(x, z) gives angle from +Z axis (which is our "forward")
        let target_heading = atan2(world_move_x, world_move_z);
        
        // Smooth rotation toward target heading
        var heading_diff = target_heading - pawn_state.heading;
        
        // Normalize to [-PI, PI]
        if (heading_diff > 3.14159) {
            heading_diff -= 6.28318;
        } else if (heading_diff < -3.14159) {
            heading_diff += 6.28318;
        }
        
        // Apply rotation with smoothing (adjust speed as needed)
        let turn_speed = 8.0;  // radians per second
        let max_turn = turn_speed * dt;
        pawn_state.heading += clamp(heading_diff, -max_turn, max_turn);
        
        // Keep heading in [-PI, PI]
        if (pawn_state.heading > 3.14159) {
            pawn_state.heading -= 6.28318;
        } else if (pawn_state.heading < -3.14159) {
            pawn_state.heading += 6.28318;
        }
    }
    
    // Orientation from terrain normal
    let world_up = vec3<f32>(0.0, 1.0, 0.0);
    let terrain_up = terrain_normal(pawn_state.pos_x, pawn_state.pos_z);
    
    // Compute rotation from world_up to terrain_up
    let dot_val = dot(world_up, terrain_up);
    if (dot_val < 0.9999) {
        let axis = normalize(cross(world_up, terrain_up));
        let angle = acos(clamp(dot_val, -1.0, 1.0));
        let tilt_quat = quat_from_axis_angle(axis, angle);
        
        // Combine with heading rotation
        let heading_quat = quat_from_axis_angle(vec3<f32>(0.0, 1.0, 0.0), pawn_state.heading);
        pawn_state.orientation = quat_multiply(tilt_quat, heading_quat);
    } else {
        // Nearly aligned, just use heading
        pawn_state.orientation = quat_from_axis_angle(vec3<f32>(0.0, 1.0, 0.0), pawn_state.heading);
    }
    
    // ─── CAMERA ────────────────────────────────────────────────────────────────
    
    // Update orbital parameters
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
    
    // Accumulate pan offset (scale by distance for consistent feel)
    let pan_scale = camera_state.distance * 0.5;
    camera_state.pan_x += signal.pan_x_delta * pan_scale;
    camera_state.pan_y += signal.pan_y_delta * pan_scale;
    
    // Compute camera basis vectors
    let cam_cos_el = cos(camera_state.elevation);
    let cam_sin_el = sin(camera_state.elevation);
    let cam_cos_az = cos(camera_state.azimuth);
    let cam_sin_az = sin(camera_state.azimuth);
    
    // Camera forward direction (from camera toward target)
    let cam_forward = vec3<f32>(-cam_cos_el * cam_sin_az, -cam_sin_el, -cam_cos_el * cam_cos_az);
    
    // Camera right vector (perpendicular to forward in XZ plane)
    let cam_right = vec3<f32>(cam_cos_az, 0.0, -cam_sin_az);
    
    // Camera up vector (perpendicular to forward and right)
    let cam_up = cross(cam_right, cam_forward);
    
    // Pawn position as base target
    let pawn_pos = vec3<f32>(pawn_state.pos_x, pawn_state.pos_y, pawn_state.pos_z);
    
    // Apply pan offset to get actual look-at target
    let look_target = pawn_pos + cam_right * camera_state.pan_x + cam_up * camera_state.pan_y;
    
    // Camera position: orbit around look target
    let cam_offset = camera_state.distance * vec3<f32>(
        cam_cos_el * cam_sin_az,
        cam_sin_el,
        cam_cos_el * cam_cos_az
    );
    let cam_pos = look_target + cam_offset;
    
    // Store back to explicit scalar fields
    camera_state.pos_x = cam_pos.x;
    camera_state.pos_y = cam_pos.y;
    camera_state.pos_z = cam_pos.z;
    
    // Compute view-projection matrix (use dynamic aspect ratio!)
    let view = look_at(cam_pos, look_target, vec3<f32>(0.0, 1.0, 0.0));
    let aspect = select(CAMERA_ASPECT, signal.aspect_ratio, signal.aspect_ratio > 0.0);
    let proj = perspective(CAMERA_FOV, aspect, CAMERA_NEAR, CAMERA_FAR);
    camera_state.view_proj = proj * view;
}
