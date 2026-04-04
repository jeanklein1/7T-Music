// ═══════════════════════════════════════════════════════════════════════════════
// PLAYGROUND HYBRID — GPU Scroll
// ═══════════════════════════════════════════════════════════════════════════════
//
// Raymarched terrain + rasterized mesh entities in the same scene.
// Foundation for worlds like n_dimensional_2.
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ MANIFEST                                                                    │
// ├─────────────────────────────────────────────────────────────────────────────┤
// │ Chart:     Terrain plane, bounded                                           │
// │ Terrain:   Raymarched height field (procedural waves)                       │
// │ Entity:    Rasterized mesh (pawn)                                           │
// │ Camera:    Full spherical orbit                                             │
// │ Lighting:  Directional + ambient                                            │
// └─────────────────────────────────────────────────────────────────────────────┘
//
// RENDERING ORDER:
//   1. Compute: update camera
//   2. Render pass 1: Raymarch terrain (writes color + depth)
//   3. Render pass 2: Rasterize entity (uses terrain depth)
//
// DEPTH COORDINATION:
//   Terrain fragment shader writes frag_depth so entities correctly
//   occlude and are occluded by terrain.
//
// SCROLL INDEX:
//   §1    SIGNAL .......................... line 45
//   §2    CAMERA .......................... line 70
//   §3    TERRAIN ......................... line 130
//   §4    ENTITY .......................... line 230
//   §5    BINDINGS ........................ line 290
//   §6    ENTRY POINTS .................... line 330
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
// §2 CAMERA — Full spherical orbit + projection
// ═══════════════════════════════════════════════════════════════════════════════

const CAMERA_FOV: f32 = 1.0;
const CAMERA_NEAR: f32 = 0.1;
const CAMERA_FAR: f32 = 200.0;
const CAMERA_MIN_ELEVATION: f32 = 0.1;      // Stay above terrain
const CAMERA_MAX_ELEVATION: f32 = 1.5;
const CAMERA_MIN_DISTANCE: f32 = 5.0;
const CAMERA_MAX_DISTANCE: f32 = 80.0;

struct CameraState {
    pos: vec3<f32>,
    azimuth: f32,
    elevation: f32,
    distance: f32,
    pan_x: f32,
    pan_y: f32,
}

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

fn camera_view_matrix(cam: CameraState, aim_point: vec3<f32>) -> mat4x4<f32> {
    let cos_el = cos(cam.elevation);
    let sin_el = sin(cam.elevation);
    let cos_az = cos(cam.azimuth);
    let sin_az = sin(cam.azimuth);
    
    let forward = vec3(-cos_el * sin_az, -sin_el, -cos_el * cos_az);
    let right = vec3(cos_az, 0.0, -sin_az);
    let up = cross(right, forward);
    
    let look_at = aim_point + right * cam.pan_x + up * cam.pan_y;
    
    let tx = -dot(right, cam.pos);
    let ty = -dot(up, cam.pos);
    let tz = -dot(-forward, cam.pos);
    
    return mat4x4<f32>(
        vec4(right.x, up.x, -forward.x, 0.0),
        vec4(right.y, up.y, -forward.y, 0.0),
        vec4(right.z, up.z, -forward.z, 0.0),
        vec4(tx, ty, tz, 1.0)
    );
}

fn camera_projection_matrix(aspect: f32) -> mat4x4<f32> {
    let f = 1.0 / tan(CAMERA_FOV * 0.5);
    let nf = 1.0 / (CAMERA_NEAR - CAMERA_FAR);
    
    return mat4x4<f32>(
        vec4(f / aspect, 0.0, 0.0, 0.0),
        vec4(0.0, f, 0.0, 0.0),
        vec4(0.0, 0.0, CAMERA_FAR * nf, -1.0),
        vec4(0.0, 0.0, CAMERA_FAR * CAMERA_NEAR * nf, 0.0)
    );
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
    
    cam.pos = camera_compute_position(vec3(0.0, 0.0, 0.0), cam);
    
    camera_state = cam;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 TERRAIN — Raymarched height field
// ═══════════════════════════════════════════════════════════════════════════════
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ TERRAIN CREATIVE CANVAS                                                     │
// │                                                                             │
// │ Modify terrain_height() to experiment with different topographies.          │
// │ Waves, noise, craters, plateaus — anything that returns a height.          │
// └─────────────────────────────────────────────────────────────────────────────┘

const TERRAIN_EXTENT: f32 = 50.0;           // Half-size of terrain
const TERRAIN_COLOR: vec3<f32> = vec3(0.55, 0.45, 0.35);

// ─── [TERRAIN:height] ───────────────────────────────────────────────────────
//
// Current: animated wave sum
// Modify this function for different terrains.

fn terrain_height(xz: vec2<f32>, t: f32) -> f32 {
    // Three overlapping waves
    let w1 = sin(xz.x * 0.15 + t * 0.5) * cos(xz.y * 0.12 + t * 0.3) * 2.0;
    let w2 = sin(xz.x * 0.08 - xz.y * 0.1 + t * 0.2) * 1.5;
    let w3 = cos(xz.x * 0.2 + xz.y * 0.15 - t * 0.4) * 0.8;
    
    return w1 + w2 + w3;
}

fn terrain_normal(xz: vec2<f32>, t: f32) -> vec3<f32> {
    let eps = 0.1;
    let h = terrain_height(xz, t);
    let hx = terrain_height(xz + vec2(eps, 0.0), t);
    let hz = terrain_height(xz + vec2(0.0, eps), t);
    return normalize(vec3(h - hx, eps, h - hz));
}

// ─── [TERRAIN:sdf] ──────────────────────────────────────────────────────────

fn sdf_terrain(p: vec3<f32>, t: f32) -> f32 {
    // Only valid within bounds
    let half = TERRAIN_EXTENT;
    if (abs(p.x) > half || abs(p.z) > half) {
        return 1000.0;  // Far away
    }
    
    let h = terrain_height(p.xz, t);
    return p.y - h;
}

// ─── [TERRAIN:raymarch] ─────────────────────────────────────────────────────

const TERRAIN_MAX_STEPS: i32 = 80;
const TERRAIN_MAX_DIST: f32 = 150.0;
const TERRAIN_SURF_DIST: f32 = 0.02;

struct TerrainHit {
    hit: bool,
    pos: vec3<f32>,
    dist: f32,
}

fn raymarch_terrain(ro: vec3<f32>, rd: vec3<f32>, t: f32) -> TerrainHit {
    var result: TerrainHit;
    result.hit = false;
    result.dist = 0.0;
    
    var ray_t: f32 = 0.0;
    
    for (var i: i32 = 0; i < TERRAIN_MAX_STEPS; i++) {
        let p = ro + rd * ray_t;
        let d = sdf_terrain(p, t);
        
        if (d < TERRAIN_SURF_DIST) {
            result.hit = true;
            result.pos = p;
            result.dist = ray_t;
            return result;
        }
        
        ray_t += d * 0.8;  // Relaxation factor
        
        if (ray_t > TERRAIN_MAX_DIST) {
            break;
        }
    }
    
    result.dist = ray_t;
    return result;
}

// ─── [TERRAIN:shading] ──────────────────────────────────────────────────────

const SKY_COLOR: vec3<f32> = vec3(0.6, 0.7, 0.85);
const FOG_DENSITY: f32 = 0.015;

fn shade_terrain(hit: TerrainHit, rd: vec3<f32>, t: f32) -> vec3<f32> {
    if (!hit.hit) {
        return SKY_COLOR;
    }
    
    let normal = terrain_normal(hit.pos.xz, t);
    let light_dir = normalize(vec3(0.3, 0.8, 0.2));
    let ndotl = max(dot(normal, light_dir), 0.0);
    
    var color = TERRAIN_COLOR * (0.35 + 0.65 * ndotl);
    
    // Fog
    let fog = 1.0 - exp(-hit.dist * FOG_DENSITY);
    color = mix(color, SKY_COLOR, fog);
    
    return color;
}

// ─── [TERRAIN:depth] ────────────────────────────────────────────────────────
//
// Convert ray distance to NDC depth for depth buffer coordination.

fn ray_dist_to_depth(ray_dist: f32) -> f32 {
    // Perspective depth: z_ndc = (far * near / (far - near)) / z + far / (far - near)
    // Simplified for our projection matrix
    let z = ray_dist;
    let ndc_depth = (CAMERA_FAR * CAMERA_NEAR / (CAMERA_NEAR - CAMERA_FAR)) / z 
                  + CAMERA_FAR / (CAMERA_FAR - CAMERA_NEAR);
    return clamp(ndc_depth, 0.0, 1.0);
}

// ─── [TERRAIN:ray_direction] ────────────────────────────────────────────────

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
// §4 ENTITY — Rasterized mesh (pawn)
// ═══════════════════════════════════════════════════════════════════════════════
//
// ┌─────────────────────────────────────────────────────────────────────────────┐
// │ ENTITY MESH                                                                 │
// │                                                                             │
// │ The mesh is defined in state.hpp MeshGen.                                   │
// │ Entity position is set by EntityState uniform.                              │
// └─────────────────────────────────────────────────────────────────────────────┘

const ENTITY_COLOR: vec3<f32> = vec3(0.8, 0.5, 0.7);

struct EntityState {
    pos: vec3<f32>,
    _pad0: f32,
}

struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
}

struct VertexOutput {
    @builtin(position) clip_pos: vec4<f32>,
    @location(0) world_pos: vec3<f32>,
    @location(1) world_normal: vec3<f32>,
}

fn shade_entity(world_pos: vec3<f32>, normal: vec3<f32>) -> vec3<f32> {
    let light_dir = normalize(vec3(0.3, 0.8, 0.2));
    let ndotl = max(dot(normal, light_dir), 0.0);
    
    var color = ENTITY_COLOR * (0.3 + 0.7 * ndotl);
    
    // Match terrain fog
    let dist = length(world_pos - render_camera.pos);
    let fog = 1.0 - exp(-dist * FOG_DENSITY);
    color = mix(color, SKY_COLOR, fog);
    
    return color;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 BINDINGS — GPU memory layout
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [BINDINGS:compute] ─────────────────────────────────────────────────────

@group(0) @binding(0) var<storage, read>       signal: FrameSignal;
@group(0) @binding(1) var<storage, read_write> camera_state: CameraState;

// ─── [BINDINGS:render_terrain] ──────────────────────────────────────────────

@group(0) @binding(10) var<storage, read> render_signal: FrameSignal;
@group(0) @binding(11) var<storage, read> render_camera: CameraState;

// ─── [BINDINGS:render_entity] ───────────────────────────────────────────────
// Same group, entity also needs camera for MVP

@group(0) @binding(12) var<storage, read> entity_state: EntityState;


// ═══════════════════════════════════════════════════════════════════════════════
// §6 ENTRY POINTS
// ═══════════════════════════════════════════════════════════════════════════════

// ─── [ENTRY:compute] ────────────────────────────────────────────────────────

@compute @workgroup_size(1)
fn update_camera() {
    compose_update_camera();
}

// ─── [ENTRY:terrain] ────────────────────────────────────────────────────────

@vertex
fn terrain_vs(@builtin(vertex_index) vid: u32) -> @builtin(position) vec4<f32> {
    let x = f32(i32(vid & 1u) * 4 - 1);
    let y = f32(i32((vid >> 1u) & 1u) * 4 - 1);
    return vec4(x, y, 0.0, 1.0);
}

struct TerrainOutput {
    @location(0) color: vec4<f32>,
    @builtin(frag_depth) depth: f32,
}

@fragment
fn terrain_fs(@builtin(position) frag_coord: vec4<f32>) -> TerrainOutput {
    let resolution = vec2(render_signal.aspect_ratio * 720.0, 720.0);
    let uv = (frag_coord.xy / resolution) * 2.0 - 1.0;
    let corrected_uv = vec2(uv.x, -uv.y);
    
    let rd = get_ray_direction(corrected_uv);
    let hit = raymarch_terrain(render_camera.pos, rd, render_signal.t_seconds);
    let color = shade_terrain(hit, rd, render_signal.t_seconds);
    
    var output: TerrainOutput;
    output.color = vec4(color, 1.0);
    
    // Write depth for entity occlusion
    if (hit.hit) {
        output.depth = ray_dist_to_depth(hit.dist);
    } else {
        output.depth = 1.0;  // Far plane
    }
    
    return output;
}

// ─── [ENTRY:entity] ─────────────────────────────────────────────────────────

@vertex
fn entity_vs(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    
    // Model transform: translate to entity position
    let world_pos = in.position + entity_state.pos;
    let world_normal = in.normal;
    
    // View-projection
    let view = camera_view_matrix(render_camera, vec3(0.0, 0.0, 0.0));
    let proj = camera_projection_matrix(render_signal.aspect_ratio);
    let vp = proj * view;
    
    out.clip_pos = vp * vec4(world_pos, 1.0);
    out.world_pos = world_pos;
    out.world_normal = world_normal;
    
    return out;
}

@fragment
fn entity_fs(in: VertexOutput) -> @location(0) vec4<f32> {
    let normal = normalize(in.world_normal);
    let color = shade_entity(in.world_pos, normal);
    return vec4(color, 1.0);
}


// ═══════════════════════════════════════════════════════════════════════════════
// END OF SCROLL
// ═══════════════════════════════════════════════════════════════════════════════
