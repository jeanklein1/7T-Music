// ═══════════════════════════════════════════════════════════════════════════════
// N-DIMENSIONAL_4 CARTRIDGE — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// A pawn and orbiting sphere affecting terrain color through spatial memory.
// Sphere follows a parametric curve with derived orientation (Frenet frame).
// Each entity contributes distinct color shifts that blend based on proximity.
//
// ADDITIONS OVER N_DIMENSIONAL_3:
//   • Polyphony → sphere color coupling (music drives sphere appearance)
//   • Sphere position → terrain tint coupling (sphere orbit paints the world)
//   • Idle-as-function pattern (computed, not stored)
//   • Multi-channel cells (vec4: rgb + height_mod)
//   • Dynamic cell resolution (active_cell_size)
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ Chart:     Plane, 100×100 world units, 256² height texels, 64² tile texels  │
// │ Fields:    height [WaveSum], tile_color [TrajectoryField + ColorBlend]      │
// │ Cells:     64×64 grid, multi-channel trajectories (color + height_mod)      │
// │ Entities:  pawn [RoundCone], camera [Orbital], sphere [Sphere + Frenet]     │
// │ Couplings: 14 total                                                         │
// │            • 2 signal→entity    (polyphony → amplitude, polyphony → sphere) │
// │            • 2 substrate→entity (height/normal → pawn)                      │
// │            • 1 entity→entity    (pawn → camera)                             │
// │            • 3 input→entity     (arrows→pawn, mouse→camera, scroll→zoom)    │
// │            • 2 entity→substrate (pawn/sphere proximity → field color)       │
// │            • 3 signal/entity→cells (poly/pawn/sphere → cell goals)          │
// │            • 1 entity→substrate (sphere position → terrain tint)            │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// COLOR BLENDING (trajectory_field system):
//   Pawn   → purple shift (0.4, 0.2, 0.5)
//   Sphere → gold shift   (0.5, 0.35, 0.0)
//   Overlap → weighted blend based on current stimulus ratio
//
// SPHERE APPEARANCE (NEW):
//   Base color: warm gold (0.95, 0.75, 0.4)
//   Excited color: coral pink (1.0, 0.45, 0.55) when polyphony high
//   Smooth trajectory-based transition between states
//
// TERRAIN TINT (NEW):
//   Sphere's orbital position creates a global color wash:
//   +X quadrant: warm amber     -X quadrant: cool azure
//   +Z quadrant: bright         -Z quadrant: deep/shadowed
//   Creates a slowly rotating "time of day" effect as sphere orbits
//
// CELL TRAJECTORIES (cells system):
//   idle:   computed checkerboard pattern (not stored)
//   goal:   set by polyphony hue rotation + entity proximity
//   value:  springs toward goal or releases to idle
//
// CURVE ARCHITECTURE:
//   sphere.pos = curve_xxx_position(t)
//   sphere.orientation = quat_from_forward(curve_xxx_tangent(t))
//   Curves are factored for future swappability (orbit, lissajous, helix, etc.)
//
// COUPLING GRAPH:
//
//     signal.polyphony ────▶ trajectory ────▶ terrain.amplitude
//            │                                      │
//            │                                      │
//            ├──────────────▶ sphere.color          │
//            │                                      │
//            └──────────────▶ cells[*].goal ◀──────┼─────────────────┐
//                             (hue rotation)       │                 │
//                                   │              │                 │
//     input.arrows ────▶ pawn.velocity ────▶ pawn.position          │
//                                                  │                 │
//                         ┌────────────────────────┼────────────────┐│
//                         ▼                        ▼                ││
//                  terrain.height(xz)      terrain.normal(xz)       ││
//                         │                        │                ││
//                         ▼                        ▼                ││
//                     pawn.y                pawn.orientation        ││
//                         │                                         ││
//                         └────────────────▶ camera.target          ││
//                                                  │                ││
//     input.mouse ─────────────────────────▶ camera.orbit           ││
//     input.scroll ────────────────────────▶ camera.distance        ││
//                                                  │                ││
//                                                  ▼                ││
//                                            camera.pos             ││
//                                                                   ││
//     pawn.position ──[purple]────────────▶ trajectory_field(xz) ───┤│
//           │                                     │                 ││
//           │   (2D proximity stimulus)           │ (spring/release)││
//           │                                     │                 ││
//     sphere.position ─[gold]─────────────▶ trajectory_field(xz) ───┘│
//           │                                     │                  │
//           │   (3D distance → blended color)     │                  │
//           │                                     │                  │
//           ├─────────────────────────────────────┴──▶ tile_color(xz)│
//           │                                                        │
//           ├──[gold]───────────────────────────▶ cells[*].goal ─────┘
//           │
//           └──[position]───────────────────────▶ terrain.tint (global)
//
//     pawn.position ──[purple]──────────────────▶ cells[*].goal
//
// SCROLL INDEX:
//   §1    CONFIG .......................... line 100
//   §1.1  IDLE STATE ...................... line 170
//   §2    CHART ........................... line 210
//   §3    FIELDS .......................... line 260
//   §4    CELLS ........................... line 430
//   §5    ENTITIES ........................ line 560
//   §5.1  ENTITY SHAPES ................... line 710
//   §6    COUPLINGS ....................... line 770
//   §7    COMPOSE ......................... line 930
//   §8    RENDER .......................... line 1210
//   §9    BINDINGS ........................ line 1410
//   §10   ENTRY POINTS .................... line 1490
//
// SYNCHRONIZATION:
//   This scroll is the single source of truth for GPU-side logic.
//   state.hpp mirrors struct layouts and constants for CPU-side initialization.
//   Keep coupling bits, struct layouts, and field parameters synchronized.
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
// │  9  │ POLYPHONY_TO_CELL_COLOR        │ signal.poly → cells[*].goal (hue)     │
// │ 10  │ PAWN_TO_CELL_COLOR             │ pawn.proximity → cells[*].goal        │
// │ 11  │ SPHERE_TO_CELL_COLOR           │ sphere.proximity → cells[*].goal      │
// │ 12  │ POLYPHONY_TO_SPHERE_COLOR      │ signal.poly → sphere.color            │
// │ 13  │ SPHERE_TO_TERRAIN_TINT         │ sphere.pos → terrain.tint (global)    │
// └─────┴────────────────────────────────┴───────────────────────────────────────┘

const COUPLING_POLYPHONY_TO_AMPLITUDE:    u32 = 1u << 0u;
const COUPLING_TERRAIN_TO_PAWN_Y:         u32 = 1u << 1u;
const COUPLING_TERRAIN_TO_PAWN_TILT:      u32 = 1u << 2u;
const COUPLING_PAWN_TO_CAMERA_TARGET:     u32 = 1u << 3u;
const COUPLING_INPUT_MOVES_PAWN:          u32 = 1u << 4u;
const COUPLING_INPUT_ORBITS_CAMERA:       u32 = 1u << 5u;
const COUPLING_INPUT_ZOOMS_CAMERA:        u32 = 1u << 6u;
const COUPLING_PAWN_TO_FIELD_COLOR:       u32 = 1u << 7u;
const COUPLING_SPHERE_TO_FIELD_COLOR:     u32 = 1u << 8u;
const COUPLING_POLYPHONY_TO_CELL_COLOR:   u32 = 1u << 9u;
const COUPLING_PAWN_TO_CELL_COLOR:        u32 = 1u << 10u;
const COUPLING_SPHERE_TO_CELL_COLOR:      u32 = 1u << 11u;
const COUPLING_POLYPHONY_TO_SPHERE_COLOR: u32 = 1u << 12u;  // NEW: music → sphere appearance
const COUPLING_SPHERE_TO_TERRAIN_TINT:    u32 = 1u << 13u;  // NEW: sphere orbit → world color

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
    freeze_sphere: u32,
    active_cell_size: f32,    // Animatable: 8.0 → 64.0 for resolution effects
    _cfg_pad0: f32,
    _cfg_pad1: f32,
    _cfg_pad2: f32,
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

fn sphere_frozen() -> bool {
    return config.freeze_sphere != 0u;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §1.1 IDLE STATE — Trajectory attractors when stimulus is absent
// ═══════════════════════════════════════════════════════════════════════════════
//
// Idle values define where stimulus-coupled parameters decay to.
// Only parameters that participate in trajectory dynamics have idle states.
//
// Non-coupled parameters don't "idle":
//   • Pawn position — derived (Y from terrain) or accumulated (XZ from input)
//   • Camera angles — accumulated from input
//   • Sphere position — derived from parametric curve
//
// These are initialized in state.hpp but have no trajectory attractor here.
//
// ┌─────────────────────────────────┬────────────────┬─────────────────────────────────┐
// │ Parameter                       │ Idle Value     │ Stimulus Source                 │
// ├─────────────────────────────────┼────────────────┼─────────────────────────────────┤
// │ ground.amplitude_scale          │ 1.0            │ signal.polyphony                │
// │ trajectory_field[*].value       │ 0.0            │ entity proximity (pawn/sphere)  │
// │ cells[*].value                  │ checkerboard   │ polyphony + entity proximity    │
// └─────────────────────────────────┴────────────────┴─────────────────────────────────┘
//
// At idle, terrain waves still animate — but at baseline intensity.
// At idle, tiles show base color — no proximity-induced offset.
// At idle, cells show checkerboard pattern — artist's deliberate choice.

const IDLE_AMPLITUDE_SCALE: f32 = 1.0;
const IDLE_FIELD_OFFSET: f32 = 0.0;


// ═══════════════════════════════════════════════════════════════════════════════
// §2 CHART — Coordinate system
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [CHART:plane] ──────────────────────────────────────────────────────────
//
// Topology: Plane with clamp boundary
// Domain:   (x, z) ∈ [-50, 50]²

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
// Modulated by: entity proximity → trajectory field → color offset
//
// COLOR BLENDING:
//   Trajectory value = "how much" (persistent memory)
//   Color direction = weighted blend of entity shifts (instantaneous)

// Base terrain color (uniform)
const TERRAIN_BASE_COLOR: vec3<f32> = vec3(0.55, 0.45, 0.35);

// Color shift directions (distinct hues for visual differentiation)
const COLOR_SHIFT_PAWN: vec3<f32> = vec3(0.4, 0.2, 0.5);    // Purple
const COLOR_SHIFT_SPHERE: vec3<f32> = vec3(0.5, 0.35, 0.0); // Warm gold

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

fn field_apply_color_offset(base: vec3<f32>, offset: f32, shift_direction: vec3<f32>) -> vec3<f32> {
    return clamp(base + shift_direction * offset, vec3(0.0), vec3(1.0));
}

fn field_blend_color_shift(pawn_stimulus: f32, sphere_stimulus: f32) -> vec3<f32> {
    // Blend shift direction based on relative stimulus contributions
    let total = pawn_stimulus + sphere_stimulus;
    if (total < 0.001) {
        return COLOR_SHIFT_PAWN;  // Default (irrelevant when offset → 0)
    }
    let pawn_weight = pawn_stimulus / total;
    let sphere_weight = sphere_stimulus / total;
    return pawn_weight * COLOR_SHIFT_PAWN + sphere_weight * COLOR_SHIFT_SPHERE;
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
// §4 CELLS — Trajectory-driven multi-effect grid
// ═══════════════════════════════════════════════════════════════════════════════
//
// Each cell holds a trajectory with MULTIPLE CHANNELS:
//   value:     what the renderer observes (rgb=color, a=height_mod)
//   velocity:  rate of change (momentum)
//   goal:      where stimulus wants it (when active)
//   is_active: 1.0 = spring toward goal, 0.0 = release to idle
//
// IDLE IS COMPUTED, NOT STORED:
//   idle_color(cell_id)      → checkerboard pattern
//   idle_height_mod(cell_id) → flat (0.0)
//   cell_idle(cell_id)       → combined vec4
//
// This enables:
//   • Different idle patterns per channel
//   • Runtime pattern changes (just modify the function)
//   • Reduced memory (48 bytes/cell instead of 64)
//   • Animated idle states (idle can depend on time)
//
// This system runs in PARALLEL with the trajectory_field system.
// trajectory_field: scalar offset + blended shift direction (legacy)
// cells: multi-channel trajectories with computed idle (new)

const CELL_GRID_SIZE: u32 = 64u;
const MAX_CELL_GRID_SIZE: u32 = 64u;  // Buffer allocation size

// ─── [CELLS:struct] ──────────────────────────────────────────────────────────
//
// 48 bytes per cell. No stored idle — computed from cell_id.
// value/velocity/goal are vec4: rgb=color, a=height_mod (or other effect)

struct CellState {
    value: vec4<f32>,       // .rgb = color, .a = height_mod
    velocity: vec4<f32>,    // rate of change per channel
    goal: vec4<f32>,        // stimulus target per channel
    is_active: f32,         // 1.0 = spring, 0.0 = release
    _pad0: f32,
    _pad1: f32,
    _pad2: f32,
}

// ─── [CELLS:idle_patterns] ───────────────────────────────────────────────────
//
// IDLE AS FUNCTION — Artist's choices per channel, pure functions.
// These define what cells return to when stimulus is removed.
// Changing these functions changes the rest state without touching buffers.

// Base color constants for idle patterns
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

// ─── [CELLS:dynamics_params] ─────────────────────────────────────────────────
//
// These control how cell trajectories feel.
// Stiffness: how quickly it accelerates toward goal
// Damping: how much velocity is absorbed (0.7 = slightly underdamped, springy)
// Release rate: exponential decay constant

const CELL_SPRING_STIFFNESS: f32 = 8.0;
const CELL_SPRING_DAMPING: f32 = 0.7;
const CELL_RELEASE_RATE: f32 = 2.0;

// ─── [CELLS:indexing] ────────────────────────────────────────────────────────

fn cell_index(texel: vec2<u32>) -> u32 {
    return texel.y * CELL_GRID_SIZE + texel.x;
}

fn cell_index_dynamic(texel: vec2<u32>, active_size: u32) -> u32 {
    // For dynamic resolution: index within active region
    return texel.y * active_size + texel.x;
}

// ─── [CELLS:trajectory_spring] ───────────────────────────────────────────────
//
// Spring dynamics: accelerate toward goal, damped by velocity
// This creates smooth, organic motion with slight overshoot
// Now operates on vec4 for multi-channel effects

fn cell_trajectory_spring(cell: CellState, dt: f32) -> CellState {
    var c = cell;
    
    let displacement = c.value - c.goal;
    let spring_force = -CELL_SPRING_STIFFNESS * displacement;
    let damping_force = -CELL_SPRING_DAMPING * sqrt(CELL_SPRING_STIFFNESS) * c.velocity;
    let accel = spring_force + damping_force;
    
    c.velocity = c.velocity + accel * dt;
    c.value = c.value + c.velocity * dt;
    
    return c;
}

// ─── [CELLS:trajectory_release] ──────────────────────────────────────────────
//
// Release dynamics: exponential decay toward idle
// This creates gentle fade-back when stimulus is removed
// IDLE IS COMPUTED from cell_id, not loaded from struct

fn cell_trajectory_release(cell: CellState, cell_id: vec2<u32>, dt: f32) -> CellState {
    var c = cell;
    
    let idle = cell_idle(cell_id);  // Computed, not loaded!
    let decay = 1.0 - exp(-CELL_RELEASE_RATE * dt);
    c.value = c.value + (idle - c.value) * decay;
    c.velocity = vec4(0.0);  // No momentum during release
    
    return c;
}

// ─── [CELLS:color_from_hue] ──────────────────────────────────────────────────
//
// Convert hue (0-1) to RGB. Used for polyphony → color mapping.

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

// ─── [CELLS:polyphony_to_color] ──────────────────────────────────────────────
//
// Maps polyphony count to a goal color via hue rotation.
// More notes = further around the color wheel.

fn polyphony_to_color(poly: f32) -> vec3<f32> {
    // Rotate hue based on polyphony
    // poly=0 → base hue (~0.08, warm orange)
    // poly=8 → rotated by ~0.4 (into greens/cyans)
    let base_hue = 0.08;  // Start at warm orange
    let rotation_rate = 0.05;  // Hue shift per note
    let hue = fract(base_hue + poly * rotation_rate);
    
    // Saturation and value stay moderate for pleasant colors
    let saturation = 0.45;
    let value = 0.75;
    
    return hsv_to_rgb(vec3(hue, saturation, value));
}

// ─── [CELLS:entity_stimulus] ─────────────────────────────────────────────────
//
// Entity proximity stimulus for cells (separate from trajectory_field)

fn entity_pawn_stimulus(tile_world: vec2<f32>, pawn_xz: vec2<f32>) -> f32 {
    let dist = distance(tile_world, pawn_xz);
    return smoothstep(PAWN_INFLUENCE_RADIUS, 0.0, dist);
}

fn entity_sphere_stimulus(tile_world: vec2<f32>, tile_height: f32, sphere: SphereState) -> f32 {
    let tile_3d = vec3(tile_world.x, tile_height, tile_world.y);
    let dist_3d = distance(tile_3d, sphere.pos);
    return smoothstep(sphere.influence_radius, 0.0, dist_3d);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 ENTITIES — Point objects and state
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
// Motion: Parametric curve, orientation derived from tangent (Frenet frame)
// State:  position (vec3), radius (f32), orientation (quat), influence_radius (f32)
//
// CURVE ARCHITECTURE:
//   The orbit function is factored for future swappability.
//   Any curve_xxx(t) → vec3 can replace the motion.
//   Orientation is computed from the curve's tangent vector.

const SPHERE_RADIUS: f32 = 2.5;
const SPHERE_INFLUENCE_RADIUS: f32 = 12.0;

// ─── [ENTITY:sphere:curve_params] ──────────────────────────────────────────
// Orbit curve parameters (compile-time constants)

const CURVE_ORBIT_RADIUS: f32 = 25.0;
const CURVE_ORBIT_SPEED: f32 = 0.3;    // rad/sec
const CURVE_ORBIT_HEIGHT: f32 = 8.0;

// ─── [ENTITY:sphere:curve] ─────────────────────────────────────────────────
// Swappable curve: position and tangent from time

fn curve_orbit_position(t: f32) -> vec3<f32> {
    let angle = t * CURVE_ORBIT_SPEED;
    return vec3(
        CURVE_ORBIT_RADIUS * cos(angle),
        CURVE_ORBIT_HEIGHT,
        CURVE_ORBIT_RADIUS * sin(angle)
    );
}

fn curve_orbit_tangent(t: f32) -> vec3<f32> {
    // Derivative of position with respect to t, normalized
    let angle = t * CURVE_ORBIT_SPEED;
    // d/dt [R*cos(ωt), H, R*sin(ωt)] = [-Rω*sin(ωt), 0, Rω*cos(ωt)]
    return normalize(vec3(
        -sin(angle),
        0.0,
        cos(angle)
    ));
}

// ─── [ENTITY:sphere:orientation] ────────────────────────────────────────────
// Build quaternion from forward direction (tangent)

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

// ─── [ENTITY:sphere:state] ──────────────────────────────────────────────────

// Sphere base color (when idle, no musical stimulus)
const SPHERE_BASE_COLOR: vec3<f32> = vec3(0.65, 0.75, 0.4);    // Warm gold
// Sphere excited color (high polyphony)
const SPHERE_EXCITED_COLOR: vec3<f32> = vec3(1.0, 0.45, 0.55); // Coral pink

struct SphereState {
    pos: vec3<f32>,
    radius: f32,
    orientation: vec4<f32>,
    influence_radius: f32,
    t: f32,                    // curve parameter (advances when not frozen)
    _pad1: f32,
    _pad2: f32,
    color: vec3<f32>,          // NEW: current appearance (driven by polyphony)
    _pad3: f32,
}

fn entity_sphere_update(t: f32) -> SphereState {
    var s: SphereState;
    s.pos = curve_orbit_position(t);
    s.radius = SPHERE_RADIUS;
    s.orientation = quat_from_forward(curve_orbit_tangent(t));
    s.influence_radius = SPHERE_INFLUENCE_RADIUS;
    // Note: color is NOT set here — it's driven by coupling, not curve
    return s;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5.1 ENTITY SHAPES — SDF primitives
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
// §6 COUPLINGS — Flow functions between subsystems
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

// Time to reach ~95% of target (in seconds)
// rate = 3.0 / time (since 1 - exp(-3) ≈ 0.95)
const AMPLITUDE_ATTACK_TIME: f32 = 10.0;    // seconds to respond to notes
const AMPLITUDE_RELEASE_TIME: f32 = 5.0;   // seconds to decay to idle

fn coupling_polyphony_to_amplitude(polyphony: f32, traj: Trajectory, dt: f32) -> Trajectory {
    let goal = IDLE_AMPLITUDE_SCALE * (1.0 + polyphony);
    
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

// ─── [COUPLING:polyphony_to_sphere_color] ───────────────────────────────────
//
// Musical stimulus drives sphere appearance.
// This is a STIMULUS → ENTITY coupling: signal.polyphony → sphere.color
//
// Design choice: warm gold at rest, coral pink when excited
// The sphere becomes a visual indicator of musical intensity.

const SPHERE_COLOR_ATTACK_RATE: f32 = 6.0;   // Quick response to music
const SPHERE_COLOR_RELEASE_RATE: f32 = 2.0;  // Gentle fade when music stops

fn coupling_polyphony_to_sphere_color(polyphony: f32, current: vec3<f32>, dt: f32) -> vec3<f32> {
    // Normalize polyphony: 8 simultaneous notes = full effect
    let intensity = saturate(polyphony / 8.0);
    let goal_color = mix(SPHERE_BASE_COLOR, SPHERE_EXCITED_COLOR, intensity);
    
    // Choose rate based on whether we're attacking or releasing
    let rate = select(SPHERE_COLOR_RELEASE_RATE, SPHERE_COLOR_ATTACK_RATE, intensity > 0.01);
    
    // Exponential smoothing creates trajectory-like behavior
    return current + (goal_color - current) * (1.0 - exp(-rate * dt));
}

// ─── [COUPLING:sphere_to_terrain_tint] ──────────────────────────────────────
//
// Sphere's orbital position creates a global terrain color wash.
// This is an ENTITY → SUBSTRATE coupling: sphere.pos → ground.tint
//
// Design choice: creates a "time of day" effect as sphere orbits
//   +X quadrant: warm amber light (afternoon)
//   -X quadrant: cool azure (morning/evening)
//   +Z quadrant: bright (noon)
//   -Z quadrant: deep/shadowed (dusk)
//
// The world slowly breathes with color as the sphere circles.

const TINT_WARM: vec3<f32> = vec3(1.20, 0.1, 0.75);   // ±20-25% swing
const TINT_COOL: vec3<f32> = vec3(0.25, 0.90, 1.20);
const TINT_BRIGHT: f32 = 1.15;
const TINT_SHADOW: f32 = 0.50;

fn coupling_sphere_to_terrain_tint(sphere_pos: vec3<f32>) -> vec3<f32> {
    // Normalize position to [-1, 1] based on orbit radius
    let nx = sphere_pos.x / CURVE_ORBIT_RADIUS;
    let nz = sphere_pos.z / CURVE_ORBIT_RADIUS;
    
    // X axis controls warm/cool balance
    let warm_factor = nx * 0.5 + 0.5;  // 0 at -X, 1 at +X
    let hue_tint = mix(TINT_COOL, TINT_WARM, warm_factor);
    
    // Z axis controls brightness
    let bright_factor = nz * 0.5 + 0.5;  // 0 at -Z, 1 at +Z
    let brightness = mix(TINT_SHADOW, TINT_BRIGHT, bright_factor);
    
    return hue_tint * brightness;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §7 COMPOSE — Orchestration of dynamics and couplings
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
    // WIRING: Time → Sphere (curve motion + derived orientation)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (!sphere_frozen()) {
        // Advance sphere's curve parameter
        var sphere = sphere_state;
        sphere.t = sphere.t + signal.dt;
        
        // Compute position and orientation from sphere's own time
        let sphere_updated = entity_sphere_update(sphere.t);
        sphere.pos = sphere_updated.pos;
        sphere.orientation = sphere_updated.orientation;
        sphere_state = sphere;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // WIRING: Signal → Sphere (polyphony drives color)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // This runs even when sphere is frozen — color responds to music
    // independently of position. The sphere becomes a visual metronome.
    
    if (signal_active() && coupling_active(COUPLING_POLYPHONY_TO_SPHERE_COLOR)) {
        sphere_state.color = coupling_polyphony_to_sphere_color(
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
        ground_state.tint = coupling_sphere_to_terrain_tint(sphere_state.pos);
    } else {
        ground_state.tint = vec3(1.0);  // Neutral when disabled
    }
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

// ─── [COMPOSE:update_cells] ──────────────────────────────────────────────────
//
// Cell trajectory system with:
//   • Multi-channel vec4 (rgb=color, a=height_mod)
//   • Computed idle patterns (not stored)
//   • Dynamic cell resolution via active_cell_size

fn compose_update_cells(texel: vec2<u32>, dt: f32) {
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMIC RESOLUTION: Skip cells outside active region
    // ═══════════════════════════════════════════════════════════════════════
    
    let active_size = u32(config.active_cell_size);
    if (texel.x >= active_size || texel.y >= active_size) {
        return;
    }
    
    let idx = cell_index(texel);
    var cell = cells[idx];
    
    // Compute cell_id for idle function (always use texel, not remapped)
    let cell_id = texel;
    
    // ═══════════════════════════════════════════════════════════════════════
    // WORLD POSITION: Remap to fill world extent regardless of active size
    // ═══════════════════════════════════════════════════════════════════════
    
    let normalized = vec2<f32>(texel) / f32(active_size);
    let tile_world = (normalized - 0.5) * CHART_EXTENT;
    
    // Approximate terrain height at this tile (for 3D sphere distance)
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = signal.t_beats * time_scale;
    let tile_height = field_height_eval(tile_world, wave_time, ground_state.amplitude_scale);
    
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
        pawn_stimulus = entity_pawn_stimulus(tile_world, pawn_state.pos.xz);
        total_stimulus += pawn_stimulus;
    }
    
    // Sphere proximity stimulus
    var sphere_stimulus: f32 = 0.0;
    if (coupling_active(COUPLING_SPHERE_TO_CELL_COLOR)) {
        sphere_stimulus = entity_sphere_stimulus(tile_world, tile_height, sphere_state);
        total_stimulus += sphere_stimulus;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // GOAL: Blend vec4 based on relative stimulus contributions
    // ═══════════════════════════════════════════════════════════════════════
    
    // Get computed idle for this cell
    let idle = cell_idle(cell_id);
    
    if (total_stimulus > 0.01) {
        cell.is_active = 1.0;
        
        // Start from idle (computed)
        var blended_color = idle.rgb;
        var blended_height = idle.a;
        
        // Add polyphony hue shift + height modulation
        if (poly_stimulus > 0.01) {
            let poly_color = polyphony_to_color(signal.stats[0]);
            let poly_weight = poly_stimulus / total_stimulus;
            blended_color = mix(blended_color, poly_color, poly_weight * 0.8);
            // Polyphony also modulates height
            blended_height = mix(blended_height, signal.stats[0] * 0.1, poly_weight * 0.5);
        }
        
        // Add pawn purple shift + height depression (pawn pushes down)
        if (pawn_stimulus > 0.01) {
            let pawn_color = idle.rgb + COLOR_SHIFT_PAWN;
            let pawn_weight = pawn_stimulus / total_stimulus;
            blended_color = mix(blended_color, pawn_color, pawn_weight * 0.9);
            // Pawn depresses terrain slightly
            blended_height = mix(blended_height, -0.3 * pawn_stimulus, pawn_weight);
        }
        
        // Add sphere gold shift + height bulge (sphere pushes up)
        if (sphere_stimulus > 0.01) {
            let sphere_color = idle.rgb + COLOR_SHIFT_SPHERE;
            let sphere_weight = sphere_stimulus / total_stimulus;
            blended_color = mix(blended_color, sphere_color, sphere_weight * 0.9);
            // Sphere raises terrain slightly
            blended_height = mix(blended_height, 0.5 * sphere_stimulus, sphere_weight);
        }
        
        cell.goal = vec4(clamp(blended_color, vec3(0.0), vec3(1.0)), blended_height);
    } else {
        cell.is_active = 0.0;
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMICS: Spring toward goal or release to idle (computed)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (dynamics_2d_active()) {
        if (cell.is_active > 0.5) {
            cell = cell_trajectory_spring(cell, dt);
        } else {
            cell = cell_trajectory_release(cell, cell_id, dt);  // Pass cell_id for computed idle
        }
    }
    
    cells[idx] = cell;
}

// ─── [COMPOSE:update_tiles] ─────────────────────────────────────────────────
//
// The core of the 2D trajectory field principle:
//   1. Load trajectory state for this cell
//   2. Compute stimulus from pawn proximity
//   3. Spring toward goal (if stimulus > 0) or release to idle (if stimulus = 0)
//   4. Store updated trajectory state
//   5. Compute final color: base + offset from trajectory
//   6. Blend with cell system (multi-channel: color + height_mod)

fn compose_update_tiles(texel: vec2<u32>) {
    // ═══════════════════════════════════════════════════════════════════════
    // LOAD: Get current trajectory state for this cell
    // ═══════════════════════════════════════════════════════════════════════
    
    let tile_idx = field_tile_index(texel);
    var traj = trajectory_field[tile_idx];
    
    // ═══════════════════════════════════════════════════════════════════════
    // STIMULUS: Compute separate influences from pawn and sphere
    // ═══════════════════════════════════════════════════════════════════════
    
    let tile_world = chart_tile_texel_to_world(texel);
    
    // Approximate terrain height at this tile (for 3D sphere distance)
    let time_scale = select(1.0, config.wave_time_scale, config.wave_time_scale > 0.0);
    let wave_time = signal.t_beats * time_scale;
    let tile_height = field_height_eval(tile_world, wave_time, ground_state.amplitude_scale);
    
    // Track stimuli separately for color blending
    var pawn_stimulus: f32 = 0.0;
    var sphere_stimulus: f32 = 0.0;
    
    // Pawn stimulus (2D distance on ground plane)
    if (coupling_active(COUPLING_PAWN_TO_FIELD_COLOR)) {
        pawn_stimulus = field_pawn_stimulus(tile_world, pawn_state.pos.xz);
    }
    
    // Sphere stimulus (3D distance from floating sphere)
    if (coupling_active(COUPLING_SPHERE_TO_FIELD_COLOR)) {
        sphere_stimulus = field_sphere_stimulus(tile_world, tile_height, sphere_state);
    }
    
    // Combined stimulus drives trajectory dynamics
    let total_stimulus = pawn_stimulus + sphere_stimulus;
    
    // ═══════════════════════════════════════════════════════════════════════
    // DYNAMICS: Spring toward goal or release to idle
    // ═══════════════════════════════════════════════════════════════════════
    
    if (dynamics_2d_active()) {
        if (total_stimulus > 0.001) {
            // Stimulus present: spring toward goal
            let goal = total_stimulus * FIELD_MAX_OFFSET;
            traj = trajectory_spring(traj, goal, signal.dt,
                                     FIELD_ATTACK_STIFFNESS, FIELD_ATTACK_DAMPING);
        } else {
            // No stimulus: release back to idle
            traj = trajectory_release(traj, IDLE_FIELD_OFFSET, signal.dt, FIELD_RELEASE_RATE);
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // STORE: Save trajectory state for next frame
    // ═══════════════════════════════════════════════════════════════════════
    
    trajectory_field[tile_idx] = traj;
    
    // ═══════════════════════════════════════════════════════════════════════
    // OUTPUT: Compute final color from base + blended trajectory offset
    // ═══════════════════════════════════════════════════════════════════════
    //
    // Trajectory value = "how much" displacement (persistent memory)
    // Color direction = weighted blend of entity shifts (instantaneous)
    
    let base_color = field_tile_base_color();
    let color_shift = field_blend_color_shift(pawn_stimulus, sphere_stimulus);
    var final_color = field_apply_color_offset(base_color, traj.value, color_shift);
    
    // ═══════════════════════════════════════════════════════════════════════
    // BLEND: Mix in cell trajectory (multi-channel: .rgb=color, .a=height_mod)
    // ═══════════════════════════════════════════════════════════════════════
    //
    // The cell system provides an additional color layer that blends with
    // the trajectory_field system. Cell color is mixed on top.
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
        let cell = cells[cell_idx];
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
    
    final_color = final_color * ground_state.tint;
    
    // Store color in RGB, height_mod in A (available for terrain shader)
    textureStore(tile_state_write, texel, vec4(final_color, cell_height_mod));
}


// ═══════════════════════════════════════════════════════════════════════════════
// §8 RENDER — Scene observation
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
    
    // Start with pawn
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

// ─── [RENDER:normals] ───────────────────────────────────────────────────────

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
    // Use sphere's coupled color (driven by polyphony)
    return render_sphere.color * (0.3 + 0.7 * ndotl);
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
// §9 BINDINGS — GPU memory layout
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
    tint: vec3<f32>,          // NEW: global color modifier from sphere position
    _pad: f32,
}

// ─── [BINDINGS:compute] ─────────────────────────────────────────────────────

@group(0) @binding(0) var<uniform>             signal: FrameSignal;
@group(0) @binding(1) var<uniform>             config: DesignConfig;
@group(0) @binding(2) var<storage, read_write> ground_state: GroundState;
@group(0) @binding(3) var<storage, read_write> pawn_state: PawnState;
@group(0) @binding(4) var<storage, read_write> camera_state: CameraState;
@group(0) @binding(5) var<storage, read_write> sphere_state: SphereState;
@group(0) @binding(6) var<storage, read_write> trajectories: array<Trajectory, 16>;
@group(0) @binding(7) var<storage, read_write> trajectory_field: array<Trajectory, 4096>;
@group(0) @binding(8) var<storage, read_write> cells: array<CellState, 4096>;

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
// §10 ENTRY POINTS — Compute and render dispatches
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
fn update_cells(@builtin(global_invocation_id) id: vec3<u32>) {
    if (id.x >= CELL_GRID_SIZE || id.y >= CELL_GRID_SIZE) {
        return;
    }
    compose_update_cells(id.xy, signal.dt);
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
