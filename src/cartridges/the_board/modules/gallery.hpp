#pragma once
#include <cstdint>
#include <random>     // std::mt19937 + distributions (PhotographerState sampling)
#include <string>
#include <vector>     // authored disk manifest
#include "cartridges/the_board/state.hpp"                    // Dim::*, GPUPaintingSlot, GPUPhotographerConfig, wgpu
#include "cartridges/the_board/modules/mood_constants.hpp"   // MOOD_COUNT (sizes the mood gate)
#include "cartridges/the_board/modules/seed_utils.hpp"       // select_weighted (PhotographerState::sample_shot_type)
#include "cartridges/the_board/modules/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── gallery.hpp (HEADER: vocabulary + configs + payloads + state + decls) ─
// Converted (LADDER-3 c4): history in audit/LADDER.md.
//
// The art system. Photographer captures snapshots; gallery sites
// curate and display them on terrain (outdoor) or on walls (indoor).
// Authored images load from disk and exhibit alongside snapshots.
//
// ┌─── Two halves ──────────────────────────────────────────────────┐
// │                                                                  │
// │  Outdoor: photographer captures snapshots while pawn walks;      │
// │           gallery sites spawn on patches as they stream in;      │
// │           paintings appear as terrain quads.                     │
// │                                                                  │
// │  Indoor:  mood entry calls place_wall_paintings; paintings       │
// │           appear as wall frames, mixing snapshots and authored.  │
// │                                                                  │
// │  Shared:  staging buffers (snapshot + authored), exhibition      │
// │           layers, painting slots, frame style presets.           │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// ┌─── Public surface (called from outside this module) ────────────┐
// │                                                                  │
// │  Module functions take GalleryState& explicitly                  │
// │  (or const GalleryState& when read-only).                        │
// │                                                                  │
// │  Per-frame:                                                      │
// │    update_photographer(gs, c, queue)         — capture cadence   │
// │    render_snapshot_pass(gs, c, encoder)      — capture render    │
// │                                                                  │
// │  Outdoor lifecycle (three-phase):                                │
// │    select_gallery_for_patch(gs, c, gx, gz, sel)                  │
// │    place_gallery_from_selection(c, sel, plan)                    │
// │      (note: no GalleryState — only mediates between sel and      │
// │       spawn-engine helpers; not part of gallery's data)          │
// │    commit_gallery(gs, c, plan, gx, gz, queue)                    │
// │    evict_paintings_for_patch(gs, c, gx, gz, queue)               │
// │                                                                  │
// │  Indoor entry (called by mood.inl::apply_mood):                  │
// │    place_wall_paintings(gs, c, queue, bmin, bmax, ceiling_h)     │
// │    clear_wall_paintings(gs, c, queue)                            │
// │                                                                  │
// │  Authored image loading:                                         │
// │    load_authored_textures(gs, c, queue) — first-call lazy load   │
// │    rotate_authored_staging(gs, c, queue) — at world teardown     │
// │                                                                  │
// │  Cross-module reads (this module's state read by others):        │
// │    gallery_state_.wall_frame_count       — read by render_passes │
// │    gallery_state_.active_painting_count  — read by render_passes │
// │    gallery_state_.gallery_centers[]      — read/written by spine │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// Depends on: state.hpp (Dim::*, GPUPaintingSlot, GPUPhotographerConfig,
// FormType/ContentSource, wgpu), mood_constants.hpp (MOOD_COUNT),
// seed_utils.hpp (select_weighted here; the impl hashes with it). The
// impl additionally reaches spawn-engine services and in-class statics
// (PopFamily / PATCH_EXTENT / GLOBAL_ENTITY_DENSITY) through the
// complete type (Cartridge:: / keyhole), and stb_image (authored disk
// loading).
//
// SEAM[gallery:complete-subsystem] complete bespoke pipeline in one
//   module — vocabulary + state + lifecycle + dispatch all together.
//   Same family as gol_zones.inl and ribbon.inl.
// SEAM[gallery:dual-role] two named sub-systems sharing infrastructure:
//   painting-on-terrain (outdoor) and painting-on-wall (indoor) with
//   shared image loading + frame rendering, divergent spawn paths.
//   The header's "Two halves" box names the division. Not a leak;
//   intentional dual role.
// SEAM[gallery:P8] keeps ENVIRONMENTAL at weight
//   0.01 deliberately — authored-but-unused, kept available for a
//   future "wide environmental" framing pass. Same family as the
//   ribbon harmonic-ratio palettes (ribbon:P8).
// NOTE[gallery:shadows-missing] paintings (terrain quads) and wall
//   frames are not currently drawn in the shadow pass
//   (render_passes.inl::draw_shadow_all). They render in the main
//   pass via draw_wall_paintings + draw_gallery_frames but cast no
//   shadows. Known gap; not addressed in this pass.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ SHOT TIERS (vocabulary) ═════════════════════════════════════
//
// Each tier defines a complete photographic character: how the
// invisible camera relates to the pawn in distance, angle, lens,
// and how the resulting painting takes shape on the terrain.
//
// ShotTypeParams fields:
//   distance_mean/sigma  — how far the camera orbits from the pawn (gaussian)
//   elevation_mean/sigma — vertical angle above horizon in radians (gaussian)
//   fov_degrees/sigma    — vertical field of view of the lens (gaussian)
//   aspect_lo/hi         — width/height ratio of the painting (uniform)
//   tracks_pawn          — whether the camera looks at the pawn or freely

enum class ShotType : uint32_t {
    PANORAMIC = 0,   // distant landscape, pawn small in frame
    ENVIRONMENTAL = 1,   // wide terrain study, pawn incidental
    MEDIUM = 2,   // balanced framing, pawn clearly visible
    CLOSE_UP = 3,   // near, pawn fills much of the frame
    PORTRAIT = 4,   // intimate vertical, pawn centered
    BIRDS_EYE = 5,   // steep overhead, map-like perspective
    LOW_ANGLE = 6,   // near ground level, looking up at pawn
    CINEMATIC = 7,   // dramatic distance + wide lens distortion
    COUNT = 8
};

struct ShotTypeParams {
    float distance_mean, distance_sigma;    // camera-to-pawn distance (world units)
    float elevation_mean, elevation_sigma;  // angle above horizon (radians)
    float fov_degrees, fov_sigma;           // vertical field of view (degrees)
    float aspect_lo, aspect_hi;             // painting width/height ratio range
    bool tracks_pawn;                       // camera aims at pawn vs free direction
    float offset_x_range;                   // max horizontal frame shift (symmetric)
    float offset_y_range;                   // max vertical frame shift (symmetric)
    float weight;                           // tier selection probability (all must sum to 1.0)
};

// ─── Tier Definitions ───────────────────────────────────────────
//
// All parameters that define a tier's character live here.
// To tune a tier: adjust its row. To add a tier: add a row + enum.

//                          dist  σ     elev   σ     fov    σ     asp_lo asp_hi  track  off_x  off_y   weight
// ENVIRONMENTAL keeps weight 0.01 deliberately — held near-zero rather
// than removed (deleting the enum value would rotate every downstream
// tier index); bump the weight to revive. See SEAM[gallery:P8].
inline constexpr ShotTypeParams SHOT_PARAMS[] = {
    /* PANORAMIC     */ {  6.0f, 4.0f,  0.16f, 0.15f,  45.0f, 15.0f,  1.78f, 2.35f,  true,  0.6f, 0.4f,   0.30f },
    /* ENVIRONMENTAL */ { 10.0f, 4.0f,  0.30f, 0.15f,  45.0f, 10.0f,  1.50f, 2.00f,  true,  0.7f, 0.5f,   0.01f },
    /* MEDIUM        */ {  6.0f, 2.0f,  0.18f, 0.16f,  55.0f, 10.0f,  1.33f, 1.78f,  true,  0.35f, 0.25f, 0.20f },
    /* CLOSE_UP      */ {  4.5f, 1.5f,  0.18f, 0.08f,  55.0f,  5.0f,  1.20f, 1.60f,  true,  0.15f, 0.10f, 0.15f },
    /* PORTRAIT      */ {  5.0f, 1.5f,  0.20f, 0.15f,  45.0f,  5.0f,  0.56f, 0.75f,  true,  0.08f, 0.12f, 0.13f },
    /* BIRDS_EYE     */ {  5.0f, 2.0f,  1.20f, 0.20f,  50.0f,  8.0f,  1.00f, 1.33f,  true,  0.4f, 0.4f,   0.07f },
    /* LOW_ANGLE     */ {  3.5f, 1.0f,  0.03f, 0.02f,  50.0f,  8.0f,  1.50f, 2.00f,  true,  0.3f, 0.2f,   0.07f },
    /* CINEMATIC     */ {  8.0f, 3.0f,  0.12f, 0.10f,  90.0f, 12.0f,  2.00f, 2.39f,  true,  0.5f, 0.3f,   0.07f },
};

// painting canvas base area (world units²) — determines physical size
// on terrain before the right-skewed multiplier [0.85, 3.0]
inline constexpr float PAINTING_AREA[] = {
    30.0f,   // PANORAMIC:     large, cinematic canvas
    24.0f,   // ENVIRONMENTAL: medium canvas
    20.0f,   // MEDIUM:        moderate canvas
    20.0f,   // CLOSE_UP:      moderate canvas
    20.0f,   // PORTRAIT:      moderate (aspect makes it tall)
    18.0f,   // BIRDS_EYE:     moderate, near-square
    22.0f,   // LOW_ANGLE:     wide, dramatic
    28.0f,   // CINEMATIC:     large, ultrawide
};

// ═══ TUNING CONSOLE ══════════════════════════════════════════════
//
// System-level dials for the gallery subsystem: photographer
// capture cadence (PhotographerCaptureConfig), outdoor gallery
// placement and curation (GalleryConfig), site-content type
// (GallerySiteType), indoor wall art configuration (WALL_ART), and
// property index registries (GalleryProp, GalleryPaintingProp,
// WallArtProp, WallPaintingProp). Per-tier values (Gaussian shot
// parameters) live in SHOT_PARAMS above.
//
// SEAM[gallery:L2] this is a clean instance of pattern P3 (player
//   state vs mood state, explicit) — concerns separated into
//   named sub-structures rather than mixed in one big config.
//   Same shape as orbs.inl's player-state vs mood-state split.
//
// SEAM[gallery:wall-art] WallArtConfig + WALL_ART are the indoor
//   half of gallery's :dual-role surface. They live here (not in
//   cartridge.hpp) because place_wall_paintings — the only
//   consumer — lives in this module. Same migration class as
//   ribbon active state (Q-closed-4).
//
// Concerns:
//   PhotographerCaptureConfig — snapshot capture cadence
//   GalleryConfig             — where/how paintings appear on terrain (outdoor)
//   GallerySiteType           — site content type enum
//   WallArtConfig + WALL_ART  — indoor wall painting placement
//   GalleryProp et al.        — named property indices for cpu_hash calls

struct PhotographerCaptureConfig {
    // Trigger: how far the pawn walks between capture events
    static constexpr float TRIGGER_DISTANCE_MEAN = 50.0f;
    static constexpr float TRIGGER_DISTANCE_SIGMA = 8.0f;
    static constexpr float TRIGGER_DISTANCE_FLOOR = 20.0f;

    // Burst: how many snapshots per trigger event
    static constexpr float BURST_WEIGHT_1 = 0.40f;
    static constexpr float BURST_WEIGHT_2 = 0.70f;
    static constexpr float BURST_WEIGHT_3 = 0.90f;
    static constexpr uint32_t BURST_MAX = 4;
    static constexpr uint32_t BURST_COOLDOWN_FRAMES = 12;

    // Clamps: hard floors on sampled camera parameters
    static constexpr float DISTANCE_FLOOR = 0.5f;
    static constexpr float ELEVATION_FLOOR = 0.005f;
    static constexpr float FOV_FLOOR = 15.0f;

    // Artistic override: wide-lens on any tier
    static constexpr float WIDE_LENS_CHANCE = 0.10f;
    static constexpr float WIDE_LENS_FOV_LO = 90.0f;
    static constexpr float WIDE_LENS_FOV_HI = 110.0f;
};

struct GalleryConfig {
    // Per-archetype gallery probability
    //   mountainous (0): very rare
    //   varied (1):      rare — checkerboard patterns, not exhibition space
    //   basin (2):       moderate — smooth sand, natural gallery ground
    static constexpr float GALLERY_CHANCE_BY_ARCHETYPE[4] = { 0.03f, 0.06f, 0.30f, 0.40f };
    static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };

    // Painting count per gallery: gaussian, median 5, σ 2
    // Max varies by archetype — basin gets the largest galleries
    static constexpr float PAINTINGS_MEAN = 5.0f;
    static constexpr float PAINTINGS_SIGMA = 2.0f;
    static constexpr uint32_t PAINTINGS_MIN = 2;
    static constexpr uint32_t PAINTINGS_MAX_BY_ARCHETYPE[4] = { 8, 10, 12, 12 };

    // Layout: paintings share a facing direction, staggered in two rows.
    // Odd paintings step forward, even step back — the pawn walks between.
    static constexpr float ROW_SPACING = 18.0f;       // horizontal distance between paintings
    static constexpr float ROW_DEPTH_MIN = 8.0f;      // minimum depth gap between rows
    static constexpr float ROW_DEPTH_RANGE = 4.0f;    // depth jitter on top of minimum
    static constexpr float ROW_LATERAL_JITTER = 2.0f;

    // Gallery mode: two options, no mixing
    //   MONO: all paintings from one tier (Portrait, Panoramic, or Cinematic)
    //   CHRONOLOGICAL: paintings in capture order, any tier
    static constexpr float MONO_TIER_CHANCE = 0.40f;  // 40% mono, 60% chronological

    // Per-gallery canvas size: the gallery rolls a size mean, then
    // each painting jitters around that mean.
    //   Gallery mean: uniform in [GALLERY_SIZE_LO, GALLERY_SIZE_HI]
    //   Per-painting jitter: gaussian σ = PAINTING_SIZE_SIGMA
    static constexpr float GALLERY_SIZE_LO = 0.85f;  // smallest gallery mean
    static constexpr float GALLERY_SIZE_HI = 3.0f;   // largest gallery mean
    static constexpr float PAINTING_SIZE_SIGMA = 0.3f;   // per-painting jitter around gallery mean

    // Minimum snapshots before galleries start appearing
    static constexpr uint32_t MIN_POOL_SIZE = 3;

    // Minimum distance between gallery centers (world units)
    static constexpr float MIN_GALLERY_DISTANCE = 150.0f;

    // ─── Content×Form Mixing ─────────────────────────────────
    //
    // Each site rolls a three-way type: pure-snapshot, pure-authored, or mixed.
    // In mixed mode, each painting independently rolls its content source.
    //
    // Outdoor (select_gallery_for_patch → commit_gallery):
    //   80% snapshot-only terrain quads
    //   5% mixed (each painting rolls independently)
    //   15% authored-only wall frames (monuments in the desert)
    //
    // Indoor (place_wall_paintings):
    //   15% snapshot-only wall frames
    //   5% mixed (each painting rolls independently)
    //   80% authored-only wall frames
    //
    static constexpr float OUTDOOR_SNAPSHOT_ONLY = 0.80f;  // [0.00, 0.80)
    static constexpr float OUTDOOR_MIXED = 0.05f;  // [0.80, 0.85)
    // remainder 0.15 = authored-only                       // [0.85, 1.00)

    // In mixed mode: per-painting chance of being the minority content
    static constexpr float OUTDOOR_MIX_AUTHORED_CHANCE = 0.35f;  // chance each outdoor painting is authored

    // Photographer pacing by archetype
    static constexpr float PHOTO_PACE_BY_ARCHETYPE[4] = { 0.7f, 0.8f, 1.5f, 1.5f };

    // Gallery center jitter (fraction of PATCH_EXTENT)
    static constexpr float POSITION_JITTER = 0.30f;
};

// Site content type (outdoor gallery)
struct GallerySiteType {
    static constexpr uint32_t SNAPSHOT_ONLY = 0;
    static constexpr uint32_t MIXED = 1;
    static constexpr uint32_t AUTHORED_ONLY = 2;
};

// ── Wall art configuration (indoor) ──────────────────────────────
//
// Centralized control for all artwork hung on indoor walls —
// both authored frames and snapshot frames.
//
// Tuning workflow: edit the WALL_ART struct below, rebuild,
// regenerate any indoor world to see the changes. No other edits
// needed — place_wall_paintings (gallery.inl) reads everything
// from here.
//
// The y-position pipeline:
//   1) base_py = ceiling_h × paint_y_frac
//   2) py = base_py + y_offset (sampled per-painting by bucket)
//   3) clamp: ensure py - height/2 ≤ max_bottom_height
//      (paintings hung too high force the camera to crane up;
//      this guarantees the bottom edge stays viewable from
//      pawn standing height)

struct WallArtScaleBucket {
    float height_lo;     // uniform [lo, hi] sample for painting height
    float height_hi;
    float weight;        // selection weight (the three weights must sum to 1)
    float y_offset_lo;   // uniform [lo, hi] additive offset from base_py
    float y_offset_hi;
};

struct WallArtConfig {
    // ─── Wall participation (cumulative thresholds, 0..1) ───
    // roll < t1 → 1 wall, < t2 → 2, < t3 → 3, residual → 4 walls
    float wall_count_t1;
    float wall_count_t2;
    float wall_count_t3;

    // ─── Per-wall painting count: uniform [lo, hi] inclusive ─
    uint32_t per_wall_count_lo;
    uint32_t per_wall_count_hi;

    // ─── Wall surface geometry ──────────────────────────────
    float corner_margin;        // distance from wall corners
    float painting_gap;         // gap between adjacent painting edges
    float paint_y_frac;         // base center as fraction of ceiling
    float max_bottom_height;    // hard upper clamp on bottom edge (m)

    // ─── Size buckets (intimate / standard / statement) ─────
    WallArtScaleBucket intimate;
    WallArtScaleBucket standard;
    WallArtScaleBucket statement;

    // ─── Indoor content mix (snapshot vs authored) ──────────
    // Per-site roll thresholds (cumulative, 0..1):
    //   roll < snapshot_only_share         → all snapshot
    //   roll < snapshot_only + mixed_share → mixed
    //   residual                           → all authored
    float snapshot_only_share;
    float mixed_share;
    // In mixed mode: per-painting chance of being a snapshot.
    float mix_snapshot_chance;
};

inline constexpr WallArtConfig WALL_ART = {
    // wall_count cumulative thresholds:
    //   0.5% → 1 wall, 0.25% → 2, 27% → 3, residual ~72% → 4
    /* wall_count_t1 */ 0.005f,
    /* wall_count_t2 */ 0.0075f,
    /* wall_count_t3 */ 0.2775f,

    // per-wall painting count
    /* per_wall_count_lo */ 1,
    /* per_wall_count_hi */ 5,

    // wall surface geometry
    /* corner_margin     */ 12.0f,
    /* painting_gap      */ 6.0f,
    /* paint_y_frac      */ 0.45f,
    /* max_bottom_height */ 4.0f,   // bottom no higher than 4 m above floor

    //                 height_lo, height_hi, weight, y_offset_lo, y_offset_hi
    /* intimate  */  {  6.0f,    11.0f,    0.25f,   0.0f,        2.0f },
    /* standard  */  {  8.0f,    12.0f,    0.50f,  -1.5f,        1.5f },
    /* statement */  { 10.0f,    14.0f,    0.25f,  -3.5f,       -1.5f },

    // content mix
    /* snapshot_only_share */ 0.15f,
    /* mixed_share         */ 0.05f,
    /* mix_snapshot_chance */ 0.40f,
};

// ── Property index registries ────────────────────────────────────
//
// Named property indices for cpu_hash / cpu_hash_f calls. Replaces
// the previous practice of literal numeric indices (`cpu_hash_f(seed,
// 500u)`). Same family as RibbonProp, GoLZoneProp, the per-family
// <Family>Idx structs in entity_pipeline.inl.
//
// Three seeds are in play in this module's hash chain:
//   1. patch seed  — passed in to select/commit_gallery
//   2. site_seed   — derived from c->world_state_.active_seed for indoor placement
//   3. p_seed      — per-painting, derived from either patch seed
//                    (outdoor) or w_seed (indoor)
//
// Outdoor and indoor per-painting contexts use *different* offsets
// off p_seed, so they get separate registries (GalleryPaintingProp
// vs WallPaintingProp). Same physical seed type, different role.

// Outdoor — patch-level seed ───────────────────────────────────
struct GalleryProp {
    static constexpr uint32_t SPAWN_ROLL          = 500u;  // gallery presence gate
    static constexpr uint32_t PAINTING_COUNT_R1   = 501u;  // sum-of-3-uniforms (Gaussian approx)
    static constexpr uint32_t PAINTING_COUNT_R2   = 502u;
    static constexpr uint32_t PAINTING_COUNT_R3   = 503u;
    static constexpr uint32_t FACING_ANGLE        = 504u;
    static constexpr uint32_t CENTER_OFFSET       = 505u;
    static constexpr uint32_t CENTER_ANGLE        = 506u;
    static constexpr uint32_t PER_PAINTING_BASE   = 510u;  // p_seed = hash(seed, BASE + p*STRIDE)
    static constexpr uint32_t PER_PAINTING_STRIDE = 7u;
    static constexpr uint32_t MONO_TIER_ROLL      = 520u;
    static constexpr uint32_t FAVORITE_TIER_PICK  = 521u;
    static constexpr uint32_t SIZE_JITTER         = 530u;
    static constexpr uint32_t SITE_TYPE_ROLL      = 540u;
};

// Outdoor — per-painting (p_seed = hash(seed, GalleryProp::PER_PAINTING_BASE + p*STRIDE))
struct GalleryPaintingProp {
    static constexpr uint32_t LATERAL_JITTER  = 0u;
    static constexpr uint32_t DEPTH_JITTER    = 1u;
    static constexpr uint32_t SIZE_JITTER_A   = 3u;  // sum-of-3 component (a)
    static constexpr uint32_t GEOMETRY_SEED   = 4u;
    static constexpr uint32_t SIZE_JITTER_B   = 5u;  // sum-of-3 component (b)
    static constexpr uint32_t SIZE_JITTER_C   = 6u;  // sum-of-3 component (c)
    static constexpr uint32_t MIX_AUTHOR_ROLL = 8u;  // chance this painting is authored in MIXED gallery
    static constexpr uint32_t AUTH_STG_PICK   = 9u;  // unused — see Q30 in rollout report
};

// Indoor — site_seed structure
struct WallArtProp {
    // site_seed = hash(c->world_state_.active_seed, SITE_SEED_OFFSET)
    static constexpr uint32_t SITE_SEED_OFFSET    = 5500u;

    // off site_seed:
    static constexpr uint32_t SITE_TYPE_ROLL      = 0u;
    static constexpr uint32_t WALL_COUNT_ROLL     = 1u;
    static constexpr uint32_t WALL_SHUFFLE_BASE   = 2u;   // shuffle index = WALL_SHUFFLE_BASE + i
    static constexpr uint32_t PER_WALL_BASE       = 10u;  // w_seed = hash(site_seed, BASE + w*STRIDE)
    static constexpr uint32_t PER_WALL_STRIDE     = 20u;

    // off w_seed:
    static constexpr uint32_t WALL_PAINTING_COUNT     = 0u;
    static constexpr uint32_t PER_PAINTING_BASE       = 100u;  // p_seed = hash(w_seed, BASE + p*STRIDE)
    static constexpr uint32_t PER_PAINTING_STRIDE     = 10u;
};

// Indoor — per-painting (p_seed = hash(w_seed, WallArtProp::PER_PAINTING_BASE + p*STRIDE))
struct WallPaintingProp {
    static constexpr uint32_t Y_OFFSET_JITTER   = 1u;
    static constexpr uint32_t MIX_SNAPSHOT_ROLL = 2u;  // chance this painting is snapshot in MIXED site
    static constexpr uint32_t HEIGHT_JITTER     = 3u;
    static constexpr uint32_t AUTH_STG_PICK     = 4u;  // unused — see Q30 in rollout report
    static constexpr uint32_t ASPECT_ESTIMATE   = 5u;
    static constexpr uint32_t SCALE_ROLL        = 7u;
};

// ═══ STATE: PHOTOGRAPHER ═════════════════════════════════════════
//
// The photographer's per-session RNG, capture cadence state, and
// burst/cooldown tracking. PhotographerState is the only sub-struct
// in this module with embedded sampling helpers — they wrap a
// std::mt19937 specifically for the capture pipeline.

struct PhotographerState {
    float cumulative_distance = 0.0f;
    float next_threshold = PhotographerCaptureConfig::TRIGGER_DISTANCE_MEAN;
    uint32_t pending_shots = 0;
    float prev_pawn_x = 0.0f;
    float prev_pawn_z = 0.0f;
    bool initialized = false;
    uint32_t frame_cooldown = 0;
    std::mt19937 rng{ 7742u };

    float uniform(float lo, float hi) {
        std::uniform_real_distribution<float> dist(lo, hi);
        return dist(rng);
    }
    float gaussian(float mean, float sigma) {
        std::normal_distribution<float> dist(mean, sigma);
        return dist(rng);
    }
    // how many snapshots per burst (weighted: 1 most common)
    uint32_t sample_shot_count() {
        float roll = uniform(0.0f, 1.0f);
        if (roll < PhotographerCaptureConfig::BURST_WEIGHT_1) return 1;
        if (roll < PhotographerCaptureConfig::BURST_WEIGHT_2) return 2;
        if (roll < PhotographerCaptureConfig::BURST_WEIGHT_3) return 3;
        return PhotographerCaptureConfig::BURST_MAX;
    }
    // tier selection — reads weights from SHOT_PARAMS matrix
    ShotType sample_shot_type() {
        constexpr uint32_t n = static_cast<uint32_t>(ShotType::COUNT);
        float w[n];
        for (uint32_t t = 0; t < n; t++) w[t] = SHOT_PARAMS[t].weight;
        return static_cast<ShotType>(select_weighted(uniform(0.0f, 1.0f), w, n));
    }
};

// ═══ STATE SUB-STRUCTS ═══════════════════════════════════════════

// ── Snapshot Staging (circular buffer, 16 layers) ──
struct SnapshotStagingRecord {
    float aspect_ratio = 1.0f;
    uint32_t shot_type = 0;
    bool valid = false;
    bool consumed = false;    // promoted to exhibition, no longer a candidate
    float capture_x = 0.0f;
    float capture_z = 0.0f;
    float capture_distance = 0.0f;
    uint32_t capture_frame = 0;
};

// ── Authored Staging (circular buffer, 16 layers) ──
struct AuthoredStagingRecord {
    uint32_t disk_index = UINT32_MAX;
    float aspect_ratio = 1.0f;
    float uv_scale_x = 1.0f;
    float uv_scale_y = 1.0f;
    bool valid = false;
    bool consumed = false;
};

// Pending texture promotions (staging → exhibition, executed in render)
struct PendingPromotion {
    bool is_snapshot;       // true = snapshot staging, false = authored staging
    uint32_t staging_layer;
    uint32_t exhibition_layer;
};
inline constexpr uint32_t MAX_PROMOTIONS_PER_FRAME = 32;

// Active gallery centers (for minimum distance enforcement)
struct GalleryCenter {
    float x = 0.0f, z = 0.0f;
    int32_t patch_gx = INT32_MAX, patch_gz = INT32_MAX;
    int32_t host_gx = 0, host_gz = 0;   // host patch (for entity_refs eviction)
    bool active = false;
};
inline constexpr uint32_t MAX_GALLERIES = 48;

struct PendingSnapshot {
    bool active = false;
    uint32_t target_slot = 0;
    uint32_t target_layer = 0;
};

// ═══ SPAWN PAYLOADS ══════════════════════════════════════════════
//
// The type-tagged payloads spawn_engine's EntityQueueEntry /
// PlacementEntry unions carry for the gallery family. Gallery
// vocabulary; at file scope they precede those unions by construction.
// Plain aggregates.
// (Outdoor art exhibitions — composite: 1 center → N paintings)

struct GallerySelection {
    uint32_t seed;
    int32_t  trigger_gx, trigger_gz;
    uint32_t slot;              // gallery center slot
    float    cx, cz;            // gallery center (jittered)
    float    footprint_r;       // gallery spatial envelope
    uint32_t archetype;         // 0–3 (terrain type, used as tier_idx)
    uint32_t painting_count;
    float    facing_angle;
    float    gallery_size_mean;
    uint32_t site_type;         // 0=snapshot, 1=mixed, 2=authored
};

struct GalleryPlacement {
    uint32_t slot;
    int32_t  trigger_gx, trigger_gz;
    int32_t  host_gx, host_gz;
    uint32_t tier_idx;          // = archetype
    float    cx, cz;
    float    footprint_r;
    uint32_t archetype;
    uint32_t painting_count;
    float    facing_angle;
    float    gallery_size_mean;
    uint32_t site_type;
};

// ═══ GALLERY MODULE STATE ════════════════════════════════════════
//
// All gallery-owned state lives in this struct, accessed via
// gallery_state_ on the Cartridge (declared at the composition root).
// Module functions take `GalleryState& gs` explicitly rather than
// reaching via Cartridge*, making ownership language-visible and
// dependencies explicit in signatures.
//
// Sub-grouped by role:
//   • photographer        — per-session RNG + capture cadence
//   • snapshot_staging    — fresh photographer captures (circular)
//   • authored_staging    — disk-loaded paintings (rotation window)
//   • exhibition          — stable layers for display textures
//   • pending_promotions  — staging→exhibition promotion queue
//   • painting_slots      — per-instance GPU mirror
//   • gallery_centers     — active outdoor gallery sites
//   • pending_snapshot    — single in-flight render target

struct GalleryState {
    PhotographerState photographer;

    // Cumulative walk + frame count are session-level companions to
    // the photographer's per-trigger state — read by both the
    // photographer (cadence) and gallery sites (sort by capture_frame).
    float    total_walk_distance = 0.0f;
    uint32_t frame_counter = 0;

    // Two parallel circular buffers (16 layers each):
    //   snapshot_staging — fresh photographer captures
    //   authored_staging — disk-loaded paintings (rotation window
    //                      across the full disk manifest)
    // Promotion to exhibition happens in commit_gallery / place_wall_paintings.
    SnapshotStagingRecord snapshot_staging[Dim::STAGING_LAYERS]{};
    uint32_t              snapshot_write_cursor = 0;
    uint32_t              snapshot_count = 0;

    AuthoredStagingRecord authored_staging[Dim::STAGING_LAYERS]{};
    uint32_t              authored_write_cursor = 0;
    uint32_t              authored_disk_cursor = 0;     // walks through authored_disk_manifest
    uint32_t              authored_staged_count = 0;
    bool                  authored_textures_loaded = false;
    std::vector<std::string> authored_disk_manifest;    // scanned lazily on first load, sorted numerically

    // Exhibition layers (32) hold textures stable until portal transition;
    // painting slots (per-instance) describe each visible painting on
    // the GPU. Galleries register their centers for spatial separation.
    bool     exhibition_occupied[Dim::EXHIBITION_LAYERS]{};
    uint32_t exhibition_count = 0;

    PendingPromotion pending_promotions[MAX_PROMOTIONS_PER_FRAME]{};
    uint32_t         pending_promotion_count = 0;

    GPUPaintingSlot painting_slots[Dim::PAINTING_MAX_SLOTS]{};
    uint32_t        active_painting_count = 0;
    uint32_t        wall_frame_count = 0;

    GalleryCenter gallery_centers[MAX_GALLERIES]{};

    PendingSnapshot pending_snapshot;
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════
//
// DEFINED in gallery.inl (post-class, self-wrapping) — the bodies
// reach the keyhole (gpuState_/renderer_/tileCache_/player_/
// world_state_/mood_state_/ribbon_state_/clearColor_/sunDirection_
// and the spine services) and in-class statics via the complete type.
// capture_snapshot, load_authored_image_to_staging,
// scan_paintings_folder, count_unused_authored, pick_authored_staging,
// fill_slot_wall_frame (+ the FrameStyle presets) and the
// find_free_*/queue_promotion state-local helpers are module-internal
// (impl-only, not declared here).

// Per-frame
void update_photographer(GalleryState& gs, Cartridge* c, wgpu::Queue& queue);
void render_snapshot_pass(GalleryState& gs, Cartridge* c, wgpu::CommandEncoder& encoder);
// Outdoor lifecycle (three-phase)
bool select_gallery_for_patch(GalleryState& gs, Cartridge* c,
    int32_t gx, int32_t gz, GallerySelection& sel);
bool place_gallery_from_selection(Cartridge* c,
    const GallerySelection& sel, GalleryPlacement& plan);
void commit_gallery(GalleryState& gs, Cartridge* c,
    const GalleryPlacement& plan,
    int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue);
void evict_paintings_for_patch(GalleryState& gs, Cartridge* c,
    int32_t gx, int32_t gz, wgpu::Queue& queue);
// Indoor entry (called by mood.inl::apply_mood)
void place_wall_paintings(GalleryState& gs, Cartridge* c, wgpu::Queue& queue,
    float bmin, float bmax, float ceiling_h);
void clear_wall_paintings(GalleryState& gs, Cartridge* c, wgpu::Queue& queue);
// Authored image loading
void load_authored_textures(GalleryState& gs, Cartridge* c, wgpu::Queue& queue);
void rotate_authored_staging(GalleryState& gs, Cartridge* c, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
