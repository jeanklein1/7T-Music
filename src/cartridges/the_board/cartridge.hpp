#pragma once

// THE_BOARD — Generative world engine. CPU orchestration.
// See world.wgsl for GPU-side (single source of truth).
//
// SEAM[spine] This is the cartridge spine. The seam map (Ch. 15)
//   describes its structural surface. Tags in this file follow the
//   Ch. 1 conventions:
//     SEAM[module:Kn|Ln|Pn]  — observation tag, points back to seam map
//     TODO[phase-N:tag]      — action tag for Claude Code (phases per Ch. 15)
//     NOTE[seam-map]         — explanatory tag for things deliberately kept
//   Three banner-only modules and three specialized-family blocks live
//   inline inside this file (Ch. 12.A-E and Ch. 13). Their tags are
//   added in chunks 2 and 3.

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/the_board/state.hpp"
#include "cartridges/the_board/renderer.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include "external/stb_image.h"

namespace t7 {
    namespace the_board {

        class Cartridge : public RenderCartridge {

        private:

            wgpu::Device device_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            GPUState gpuState_;
            Renderer renderer_;

            struct InputState {
                float move_x = 0.0f;
                float move_z = 0.0f;
                float look_az_delta = 0.0f;
                float look_el_delta = 0.0f;
                float zoom_delta = 0.0f;
                float pan_x_delta = 0.0f;
                float pan_y_delta = 0.0f;
            } inputState_;

            struct KeyState {
                bool forward = false;
                bool backward = false;
                bool left = false;
                bool right = false;
            } keys_;

            struct MouseState {
                bool left_dragging = false;
                bool right_dragging = false;
            } mouse_;

            bool fpvMode_ = false;
            float currentBeats_ = 0.0f;
            float currentSeconds_ = 0.0f;
            float currentDt_ = 0.016f;

            // Sun + atmosphere (driven by active mood — see apply_mood)
            float sunDirection_[3] = { 0.69f, -0.71f, -0.14f };
            float sunColor_[3] = { 1.0f, 0.95f, 0.9f };
            float sunIntensity_ = 0.8f;
            float sunAmbient_ = 0.25f;
            float clearColor_[3] = { 0.85f, 0.78f, 0.72f };
            uint32_t activeMood_ = 0;
            float terrainAmpCeiling_ = 0.0f;    // mirrors GPU config.terrain_amp_ceiling

            // ── Musical Coupling State (modules/musical.inl) ──
            // SEAM[musical:K2] musical.inl currently exposes state declarations
            //   here; the per-frame couplings live in update() below (~lines
            //   8050-8240). End-of-tour resolution: extract tick_musical_couplings()
            //   into the module and have update() call it.
            // SEAM[musical:K3] prevPolyphony_ (declared inside the include) is
            //   consumer state for pulse onset detection at update() ~8200-8236;
            //   stays here today, moves with K2.
#include "modules/musical.inl"

            // ── Player State (unified entity layer) ──
            //
            // The player's relationship to the world, not a physical body.
            // The body lives in agentStateBuffer_[possessed_slot]; this
            // struct is what travels with the player on possession
            // transfer (Caps Lock). Pass 1 only fills possessed_slot;
            // aura/mmodes still live in their respective modules and
            // are folded in by later passes.
            //
            // See agent_system_design.md §2.1 for the full design.
            //
            // SEAM[spine:P8] PlayerState commented "Future (deferred)" fields
            //   are explicit latent infrastructure: aura_presence and
            //   mmode_intensities are scheduled to migrate here once pawn.inl
            //   exists. Pattern P8 visible in source.
            struct PlayerState {
                uint32_t possessed_slot = 0;   // slot in agent_state[] that the player inhabits
                // Future (deferred):
                //   uint32_t active_couplings;         // COUPLING_* bitmask owned by player
                //   float    aura_presence;            // migrated from auraPresence_
                //   float    mmode_intensities[MMODE_COUNT];  // migrated from mmodeIntensity_
            };
            PlayerState player_{};

            GPUSpotLightArray cpuSpotLights_{};  // count=0 disables (outdoor)
            bool spotLightActive_ = false;

            // --- World Transition State Machine ---
            enum class TransitionPhase { IDLE, FADE_OUT, TEARDOWN, FADE_IN };
            TransitionPhase transitionPhase_ = TransitionPhase::IDLE;
            float transitionTimer_ = 0.0f;
            float transitionFadeDuration_ = 0.5f;   // seconds per fade direction
            float transitionFadeAlpha_ = 0.0f;

            // Portal destination — describes the world a door leads to.
            // Also used as the pending transition target (keys + portal crossings).
            struct PortalDestination {
                uint32_t seed = 0;
                bool finite = false;
                uint32_t finite_radius = 2;
                uint32_t mood = 0;               // 0=open, 1=finite (expandable)
            };
            PortalDestination pendingDestination_{};

            // --- Finite patch mode ---
            bool finiteMode_ = false;
            uint32_t finiteRadius_ = 2;              // 2 → 5×5 = 25 patches

            // --- Portal detection ---
            static constexpr float PORTAL_DENSITY = 1.00f;  // fraction of Doorway arches that become portals (was 0.25)

            // Portal color by mood (indexed by destination.mood)
            static constexpr float PORTAL_COLORS[6][3] = {
                { 0.90f, 0.45f, 0.70f },  // mood 0  open_default    — pink
                { 0.72f, 0.45f, 0.85f },  // mood 1  open_sunset     — lilac
                { 0.95f, 0.55f, 0.15f },  // mood 2  indoor_flat     — orange
                { 0.95f, 0.80f, 0.20f },  // mood 3  indoor_vault    — yellow
                { 0.85f, 0.20f, 0.15f },  // mood 4  finite_outdoor  — red
                { 0.70f, 0.15f, 0.12f },  // mood 5  finite_outdoor_ref — dark red
            };
            static constexpr float PORTAL_COLOR_BACK[3] = { 0.35f, 0.55f, 0.90f };  // back-portal — blue

            // ─── Mood System ─────────────────────────────────────────────
            //
            // Each mood defines an atmosphere: sun direction/color, fog,
            // finite vs. open, patch radius. Portals pick a mood for
            // their destination; the mood is applied during teardown.
            //
            // Moods 0-1: infinite outdoor.  Moods 2-3: finite indoor.  Mood 4: finite outdoor.  Mood 5: finite outdoor (reference clone).

            enum class CeilingType : uint32_t {
                NONE = 0,   // outdoor — no shell geometry
                FLAT = 1,   // flat slab ceiling
                VAULT = 2,   // catenary vault ceiling
            };

            struct MoodProfile {
                // ─── World bounds ───────────────────────────────────────
                bool   finite;                 // true = walled world with finite radius
                uint32_t finite_radius_min;    // min patch radius (when finite)
                uint32_t finite_radius_max;    // max patch radius (when finite)

                // ─── Lighting ───────────────────────────────────────────
                float  sun_direction[3];       // directional light vector (normalized)
                float  sun_color[3];           // sun RGB
                float  sun_intensity;          // diffuse strength
                float  sun_ambient;            // ambient fill strength

                // ─── Atmosphere ─────────────────────────────────────────
                float  fog_density;            // exponential fog coefficient
                float  fog_color[3];           // fog/horizon RGB

                // ─── Indoor shell ───────────────────────────────────────
                bool   indoor;                 // true = enclosed space with ceiling
                CeilingType ceiling_type;      // NONE / FLAT / VAULT
                float  ceiling_height;         // ceiling Y (world units)

                // ─── Background ─────────────────────────────────────────
                float  clear_color[3];         // sky or dark ceiling RGB
                float  wall_color[3];          // indoor wall surface RGB
                float  ceiling_color[3];       // indoor ceiling surface RGB

                // ─── Feature selection (per-mood) ───────────────────────
                // Each mood independently declares which systems are active.
                // Not tied to indoor/outdoor — a walled mood can still have
                // musical modes, an open mood can still skip pawn aura.
                bool   allow_musical_modes;    // mode_* config values + terrain wave overlay
                bool   allow_gol_zones;        // GoL zone spawning + visualization
                bool   allow_pawn_aura;        // toroidal spring grid tinting + height boost
                bool   allow_frustum_cull;     // GPU frustum cull for LOD0 terrain (Tier 4)

                // Sky orb config is a parallel table (ORB_MOOD_TABLE, below),
                // indexed by the same mood index as MOOD_TABLE. See orbs.inl
                // for the OrbMoodConfig field definitions.
            };

            static constexpr uint32_t MOOD_COUNT = 6;

            // ─── Mood IDs ───────────────────────────────────────────────────
            //
            // Named indices into MOOD_TABLE / ORB_MOOD_TABLE / AGENT_POPULATIONS
            // and any per-mood multiplier array elsewhere in the codebase.
            // The order is canonical — every per-mood table is written in
            // this order, and AGENT_POPULATIONS has per-row static_asserts
            // that catch reordering.

            static constexpr uint32_t MOOD_OPEN_DEFAULT = 0;
            static constexpr uint32_t MOOD_OPEN_SUNSET = 1;
            static constexpr uint32_t MOOD_INDOOR_FLAT = 2;
            static constexpr uint32_t MOOD_INDOOR_VAULT = 3;
            static constexpr uint32_t MOOD_FINITE_OUTDOOR = 4;
            static constexpr uint32_t MOOD_FINITE_OUTDOOR_REF = 5;

            // ─── Mood Definitions ───────────────────────────────────────────
            //
            // Sky orb config lives in ORB_MOOD_TABLE below, indexed by the
            // same mood index.
            //
            // SEAM[mood:K1] indoor/outdoor binary lives here as bool `finite` +
            //   bool `indoor` flags. With finite_outdoor and finite_outdoor_ref,
            //   the binary doesn't survive contact — the encoding is correct
            //   for today but worth re-examining when finite_outdoor design lands.
            // SEAM[mood:K4] mood-5 (FINITE_OUTDOOR_REF) is a near-bit-identical
            //   clone of mood-0 (OPEN_DEFAULT) with finite=true. The "reference"
            //   role is encoded by ID, not by a profile flag. End-of-tour: add
            //   `has_anchor_ribbon` flag, drop the magic-mood-number checks
            //   in apply_mood / spawn paths.
            //                                  fin  r_min r_max  sun_dir                sun_color              int   amb   fog_d   fog_color               indoor  ceil       ceil_h  clear_color            wall_color             ceil_color               modes   zones  aura   cull
            static constexpr MoodProfile MOOD_TABLE[MOOD_COUNT] = {
                /* MOOD_OPEN_DEFAULT       */  { false, 2, 2, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  {0.85f, 0.78f, 0.72f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  true  },
                /* MOOD_OPEN_SUNSET        */  { false, 2, 2, { 0.96f,-0.26f,-0.13f}, {1.0f, 0.75f, 0.45f}, 0.90f, 0.20f, 0.0050f, {0.95f, 0.70f, 0.45f},  false, CeilingType::NONE,  0.0f,  {0.95f, 0.70f, 0.45f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  true  },
                /* MOOD_INDOOR_FLAT        */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f, 0.0003f, {0.15f, 0.12f, 0.10f},  true,  CeilingType::FLAT,  20.0f, {0.15f, 0.12f, 0.10f}, {0.65f,0.58f,0.50f}, {0.60f,0.55f,0.48f},   true,  true,  true,  false },
                /* MOOD_INDOOR_VAULT       */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f, 0.0003f, {0.15f, 0.12f, 0.10f},  true,  CeilingType::VAULT, 25.0f, {0.15f, 0.12f, 0.10f}, {0.70f,0.62f,0.52f}, {0.65f,0.58f,0.50f},   true,  true,  true,  false },
                /* MOOD_FINITE_OUTDOOR     */  { true,  1, 4, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  {0.85f, 0.78f, 0.72f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  true  },
                /* MOOD_FINITE_OUTDOOR_REF */  { true,  1, 4, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  {0.85f, 0.78f, 0.72f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  true  },
            };

            // Orb mood config lives in ORB_MOOD_TABLE, declared right after
            // the #include "modules/orbs.inl" below (where OrbMoodConfig is
            // defined). Same MOOD_COUNT size, indexed by the same mood index.

            static const char* mood_name(uint32_t mood) {
                // Sized array so the compiler catches a missing entry if
                // MOOD_COUNT changes without updating this list.
                static const char* NAMES[MOOD_COUNT] = {
                    "open_default", "open_sunset", "indoor_flat",
                    "indoor_vault", "finite_outdoor", "finite_outdoor_ref"
                };
                return (mood < MOOD_COUNT) ? NAMES[mood] : "unknown";
            }

            // ─── Indoor Wall Palette ─────────────────────────────────────
            //
            // SEAM[spine:per-mood-data] new finding (Ch. 15 chunk 1): the spine
            //   declares per-mood authoring data alongside MOOD_TABLE —
            //   INDOOR_PALETTES, WALL_ART, LIGHT_SCHEMES, IndoorLightProp.
            //   Same family as ORB_MOOD_TABLE that the seam map flagged for
            //   migration to orbs.inl (orbs:D2). These tables read by mood.inl
            //   apply_mood and spine code; consider migration to mood.inl
            //   when mood.inl gains the structural-decomposition pass (mood:K2).
            //   Distinct from MOOD_TABLE which is global atmosphere data.
            //
            // Per-seed wall+ceiling color override for indoor moods. The
            // MOOD_TABLE wall_color/ceiling_color act as fallback for the
            // open-world finite cases; in flat/vault moods, apply_mood
            // selects one of these palettes from activeSeed_ and substitutes
            // it before generate_indoor_shell uploads the shell mesh.
            //
            // Each palette is a designed (wall, ceiling) pair where the
            // ceiling is a slightly darker shade of the wall — gives the
            // room visual depth instead of flat single-color surfaces.
            // Authored to feel like museum / gallery wall finishes rather
            // than random RGB rolls.
            struct IndoorPalette {
                const char* name;
                float wall_color[3];
                float ceiling_color[3];
            };
            static constexpr IndoorPalette INDOOR_PALETTES[] = {
                { "warm taupe",     { 0.72f, 0.65f, 0.55f }, { 0.65f, 0.58f, 0.50f } },
                { "slate blue",     { 0.58f, 0.62f, 0.68f }, { 0.50f, 0.55f, 0.62f } },
                { "terracotta",     { 0.65f, 0.50f, 0.42f }, { 0.55f, 0.42f, 0.36f } },
                { "sage",           { 0.62f, 0.68f, 0.58f }, { 0.55f, 0.60f, 0.52f } },
                { "pale linen",     { 0.85f, 0.80f, 0.72f }, { 0.78f, 0.73f, 0.65f } },
                { "deep mocha",     { 0.45f, 0.40f, 0.35f }, { 0.38f, 0.34f, 0.30f } },
                { "dusty rose",     { 0.70f, 0.58f, 0.58f }, { 0.62f, 0.50f, 0.50f } },
                { "warm charcoal",  { 0.40f, 0.38f, 0.36f }, { 0.32f, 0.30f, 0.28f } },
            };
            static constexpr uint32_t INDOOR_PALETTE_COUNT =
                sizeof(INDOOR_PALETTES) / sizeof(INDOOR_PALETTES[0]);

            // ─── Wall Art Configuration ──────────────────────────────────
            //
            // Centralized control for all artwork hung on indoor walls —
            // both authored frames and snapshot frames. Replaces previously
            // scattered constants (PAINT_Y_FRAC, INDOOR_SCALES, CORNER_MARGIN,
            // PAINTING_GAP, the wall-count cumulative thresholds, the per-
            // wall painting count, and the per-bucket y-offset ranges).
            //
            // Tuning workflow: edit the WALL_ART struct below, rebuild,
            // regenerate any indoor world to see the changes. No .inl edits
            // needed — place_wall_paintings reads everything from here.
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

            static constexpr WallArtConfig WALL_ART = {
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

                // content mix (was 15% snapshot-only / 5% mixed / 80% authored)
                /* snapshot_only_share */ 0.15f,
                /* mixed_share         */ 0.05f,
                /* mix_snapshot_chance */ 0.40f,
            };

            // ─── Indoor Entity Placement ─────────────────────────────────
            //
            // Minimum distance from any spawning entity's FOOTPRINT EDGE to
            // the room walls (not the entity's center — large entities still
            // keep this gap). Enforced in negotiate_position by clamping
            // the seed-determined candidate position into the legal box
            //   [bmin + margin + r, bmax - margin - r]  in both axes
            // when finiteMode_ is active and the current mood is indoor.
            // Outdoor moods and open worlds are unaffected (no walls to
            // keep clear of).
            //
            // Clamp (not reject) is intentional: rejection-based logic
            // would silently drop entities anchored to corner patches,
            // because their seed-determined position keeps landing in the
            // wall margin and never recovers. Clamping shifts the candidate
            // to the legal-box edge and lets the existing footprint-overlap
            // check handle any pile-ups that result.
            static constexpr float INDOOR_ENTITY_WALL_MARGIN = 10.0f;

            // ─── Indoor Lighting Schemes ─────────────────────────────────
            //
            // Seed-driven procedural lighting for indoor moods. Each scheme
            // defines a lighting character (which surfaces carry lights,
            // how many, primary vs accent roles). Per-light parameters
            // (position along surface, intensity, cone width, color warmth)
            // are derived from activeSeed_ at mood transition time.
            //
            // Three schemes:
            //   Cathedral — ceiling primary + two opposing wall sconces
            //   Gallery   — two opposing wall lights, no ceiling (dramatic)
            //   Sanctum   — single source, maximum contrast
            //
            // The seed also picks which wall pair (N/S or E/W) carries the
            // sconces, so rooms with the same scheme still feel different.

            enum class LightAnchor : uint32_t {
                CEILING, WALL_NORTH, WALL_SOUTH, WALL_EAST, WALL_WEST
            };

            struct IndoorLightProp {
                static constexpr uint32_t SCHEME = 1100u;
                static constexpr uint32_t WALL_PAIR = 1101u;
                static constexpr uint32_t ANCHOR_PICK = 1102u;
                static constexpr uint32_t SLOT_BASE = 1110u;  // + slot*10 + field
                // Per-slot field offsets
                static constexpr uint32_t LATERAL = 0u;
                static constexpr uint32_t HEIGHT = 1u;
                static constexpr uint32_t INTENSITY = 2u;
                static constexpr uint32_t INNER_CONE = 3u;
                static constexpr uint32_t OUTER_CONE = 4u;
                static constexpr uint32_t WARMTH = 5u;
                static constexpr uint32_t AIM_PITCH = 6u;
                static constexpr uint32_t AIM_YAW = 7u;
            };

            // Slot definition: anchor surface + gaussian ranges for the
            // light's character. Position slide uses fixed sigmas.
            // Direction is fully parameterised per slot:
            //   aim_pitch — angle below horizontal (wall) or off-vertical (ceiling), radians
            //   aim_yaw   — lateral rotation along the anchor surface, radians
            struct LightSlotDef {
                LightAnchor anchor;
                float intensity_mean, intensity_sigma;
                float inner_mean, inner_sigma;    // inner half-angle (radians)
                float outer_mean, outer_sigma;    // outer half-angle (radians)
                float warmth_mean, warmth_sigma;  // 0 = warm amber, 1 = cool blue
                float aim_pitch_mean, aim_pitch_sigma;  // radians
                float aim_yaw_mean, aim_yaw_sigma;      // radians
                // Anchor-surface position (carried through from LightSchemeSlot).
                float lat_mean, lat_sigma;        // along anchor surface
                float hfrac_mean, hfrac_sigma;    // ceiling: Z; walls: height
            };

            static constexpr float SCHEME_WEIGHTS[] = { 0.35f, 0.35f, 0.15f, 0.15f };
            static constexpr uint32_t SCHEME_COUNT = 4;
            static constexpr const char* SCHEME_NAMES[] = { "Cathedral", "Quartet", "Gallery", "Sanctum" };
            static constexpr const char* ANCHOR_NAMES[] = { "ceiling", "wall_N", "wall_S", "wall_E", "wall_W" };

            // ─── Lighting Scheme Table ───────────────────────────────────────
            //
            // Constexpr tier matrix for indoor lighting.
            // AnchorRole is resolved to a concrete LightAnchor at runtime
            // (WALL_A/B → N/S or E/W depending on seed-driven wall pair).
            //
            // To tune a scheme: adjust its rows. To add a scheme: add a block
            // + SCHEME_COUNT + SCHEME_WEIGHTS entry.

            enum class AnchorRole : uint32_t {
                CEILING,    // always ceiling
                WALL_A,     // seed-selected wall pair, side A
                WALL_B,     // seed-selected wall pair, side B
                SEED_PICK   // anchor chosen from seed (ceiling or wall)
            };

            struct LightSchemeSlot {
                AnchorRole role;
                float intensity_mean, intensity_sigma;
                float inner_mean, inner_sigma;
                float outer_mean, outer_sigma;
                float warmth_mean, warmth_sigma;
                float aim_pitch_mean, aim_pitch_sigma;
                float aim_yaw_mean, aim_yaw_sigma;
                // Position on the anchor surface (0..1 along the surface span).
                // For ceiling: lat=X-axis, hfrac=Z-axis (both in [0..1]).
                // For walls:   lat=along-wall, hfrac=height (both in [0..1]).
                // Existing schemes (Cathedral/Gallery/Sanctum) used hardcoded
                // 0.5/0.65 means with 0.15/0.10 sigmas; they keep that here.
                // New schemes (e.g. Quartet) override per slot to place lights
                // deliberately rather than relying on each one rolling near
                // center independently.
                float lat_mean, lat_sigma;
                float hfrac_mean, hfrac_sigma;
            };

            struct LightScheme {
                uint32_t slot_count;
                LightSchemeSlot slots[4];  // MAX_SPOT_LIGHTS
            };

            // ─── Scheme Definitions ─────────────────────────────────────────
            //
            //                                role            int_μ  int_σ  inn_μ inn_σ out_μ out_σ wrm_μ wrm_σ  pit_μ  pit_σ  yaw_μ  yaw_σ   lat_μ lat_σ  hfrac_μ hfrac_σ
            static constexpr LightScheme LIGHT_SCHEMES[SCHEME_COUNT] = {
                /* 0: Cathedral — ceiling primary + 2 wall sconces */
                { 3, {
                    { AnchorRole::CEILING,   8.0f, 2.5f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.12f, 0.0f, 0.12f,   0.50f, 0.15f, 0.65f, 0.10f },
                    { AnchorRole::WALL_A,    5.0f, 1.5f, 0.4f, 0.15f, 1.0f, 0.15f, 0.20f, 0.15f,  0.60f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
                    { AnchorRole::WALL_B,    5.0f, 1.5f, 0.4f, 0.15f, 1.0f, 0.15f, 0.75f, 0.15f,  0.60f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
                }},
                /* 1: Quartet — 4 ceiling lights at quadrant corners.
                 *    Tight sigma (0.07) keeps each light pinned in its quadrant;
                 *    means at 0.30/0.70 produce a 2x2 layout that fills the
                 *    room evenly. Per-light intensity dropped to 4.0±1.0 to
                 *    keep total scene brightness comparable to Cathedral
                 *    (which has ~8.0 from a single ceiling light + sconces). */
                { 4, {
                    { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.30f, 0.07f, 0.30f, 0.07f },
                    { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.70f, 0.07f, 0.30f, 0.07f },
                    { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.30f, 0.07f, 0.70f, 0.07f },
                    { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.70f, 0.07f, 0.70f, 0.07f },
                }},
                /* 2: Gallery — 2 opposing wall lights, no ceiling */
                { 2, {
                    { AnchorRole::WALL_A,    7.0f, 2.0f, 0.4f, 0.15f, 1.1f, 0.15f, 0.25f, 0.20f,  0.55f, 0.45f, 0.0f, 0.35f,   0.50f, 0.15f, 0.65f, 0.10f },
                    { AnchorRole::WALL_B,    7.0f, 2.0f, 0.4f, 0.15f, 1.1f, 0.15f, 0.65f, 0.20f,  0.55f, 0.45f, 0.0f, 0.35f,   0.50f, 0.15f, 0.65f, 0.10f },
                }},
                /* 3: Sanctum — single dramatic source */
                { 1, {
                    { AnchorRole::SEED_PICK, 10.0f, 2.5f, 0.5f, 0.2f, 1.2f, 0.15f, 0.45f, 0.25f,  0.50f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
                }},
            };

            GPUPortalArray cpuPortalArray_{};
            bool portalsDirty_ = true;   // true at boot → first upload guaranteed

            // --- Back-portal (guaranteed exit from finite worlds) ---
            // Position is configurable so special-case layouts can relocate it.
            float backPortalPosition_[2] = { 10.0f, 0.0f };   // world XZ
            bool  backPortalPending_ = false;
            uint32_t backPortalReturnSeed_ = 0;
            uint32_t backPortalReturnMood_ = 0;
            uint32_t backPortalReturnRadius_ = 2;

            // GPU agent-state readback machine: IDLE → COPIED → MAPPING → IDLE
            // Reads the full agent_state buffer (MAX_AGENTS × GPUAgentState).
            // Consumers: patch streaming / ribbon / photographer (possessed
            // slot's XZ), portal triggers (possessed slot's portal_trigger),
            // Caps Lock nearest-agent query (all slots, Step 7).
            //
            // SEAM[spine:P5] readback state machines + worldGen_ counter are
            //   pattern P5 (release-pending sentinel) at the spine level.
            //   Pawn + floater readbacks each protect against stale callbacks
            //   from previous worlds via worldGen_ capture in the closure.
            //   NOTE[seam-map] genuinely spine-owned, not a leak.
            enum class PawnReadbackState { IDLE, COPIED, MAPPING };
            PawnReadbackState pawnReadbackState_ = PawnReadbackState::IDLE;
            // Floater readback — separate state machine, same pattern.
            // Synchronizes CPU activeFloaters_/activeCubes_ "active" mirrors
            // with the GPU is_active field (which the kernel writes when
            // a floater drifts beyond FLOATER_EVICTION_RADIUS). Without
            // this sync, kernel-side evictions are invisible to the CPU
            // allocator and slots leak — see run_spawn_preamble's slot
            // search at line ~1641. Stale leakage caps effective floater
            // count well below MAX_*_INSTANCES.
            enum class FloaterReadbackState { IDLE, COPIED, MAPPING };
            FloaterReadbackState floaterReadbackState_ = FloaterReadbackState::IDLE;
            int32_t readbackPortalTrigger_ = -1;
            float pawnReadback_x_ = 0.0f;
            float pawnReadback_z_ = 0.0f;

            // World generation counter — bumped on every teardown.
            // Captured by readback callbacks so that callbacks issued
            // for the previous world drop their data on the floor instead
            // of overwriting pawnReadback_x_/z_ with a stale position
            // (which would corrupt tile-cache archetype rolls in the
            // first frames of the new world). See update() TEARDOWN
            // case and the agent_state readback lambda in render().
            uint32_t worldGen_ = 0;

            // --- Unified Pier System ─────────────────────────────────────────
            //
            // Deterministic slot addressing: test rig at 0-2, arch piers at 4-35,
            // column piers at 36-67. CPU mirrors the GPU buffer for dead-reckoning
            // step-height checks. No allocator — slot = f(entity_slot).
            GPUPierInstance cpuPiers_[Dim::PIER_TOTAL]{};

            // ── Seed Utilities (modules/seed_utils.inl) ──
#include "modules/seed_utils.inl"

            // ── Terrain CPU mirror deleted ────────────────────────────────
            // GPU is single source of truth for entity ground_y.
            // compute_entity_placement samples the heightfield directly.
            // Only estimate_terrain_height (tileCache_ lookup) survives for ribbon.

            // ── Entity Type Definitions (modules/entities.inl) ──
#include "modules/entities.inl"

                        // ── Pawn Aura (modules/pawn_aura.inl) ──
#include "modules/pawn_aura.inl"

                        // ── Ground Architecture (modules/ground_architecture.inl) ──
#include "modules/ground_architecture.inl"

                        // ── Sky Orbs (modules/orbs.inl) ──
#include "modules/orbs.inl"

            // ─── Orb Mood Table ─────────────────────────────────────────────
            //
            // SEAM[orbs:D2] this table currently lives in cartridge.hpp because
            //   OrbMoodConfig is defined in orbs.inl (above), but the table
            //   itself is per-mood orb authoring data. Phase 2 resolution per
            //   12.F: move ORB_MOOD_TABLE into orbs.inl alongside
            //   OrbMoodConfig at the time of orbs.inl's other migrations.
            //   Same family as the per-mood data tables flagged in
            //   spine:per-mood-data (INDOOR_PALETTES, WALL_ART, LIGHT_SCHEMES).
            // SEAM[mood:K4] mood-5 row is bit-identical to mood-0 (open_default)
            //   except for the implicit context that mood-5 is
            //   finite_outdoor_ref. Mirrors the same MOOD_TABLE pattern.
            //   Resolves with the has_anchor_ribbon flag (mood:L1).
            //
            // Per-mood orb config. Indexed by the same mood index as MOOD_TABLE.
            // See OrbMoodConfig in orbs.inl (above) for field semantics. Zero-
            // valued rule-critical fields (drg, nz, orbS, flock radii/weights)
            // are sanitized to system defaults in configure_orbs — "0 = no
            // opinion, system picks a working value." Explicit small non-zero
            // reads as deliberate authorship.
            //
            //  Column legend (short → field name):
            //    en      enabled              rotAxis rotation_axis[3]
            //    n       count                orbS    orbital_base_speed (rule 1)
            //    hueB    base_hue (legacy)    pal     palette_id (ORB_PAL_*)
            //    hueV    hue_variance         pul     color_pulse_enabled
            //    bri     brightness           cnv     color_converge_enabled
            //    drg     drag (1/s)           srg     color_surge_enabled
            //    nz      noise_amp ceiling    hct     hue_converge_target
            //    rul     motion_rule          anc     anchor_to_pawn_default
            //    rotS    rotation_speed       trs     tierset_id (0xFFFFFFFFu = legacy)
            //    sepR/alnR/cohR/sepW/alnW/cohW/maxS   flocking parameters
            //    gst     flock_gesture_default (0..7, ORB_FLOCK_GESTURES index)
            //    drgB/drgO/drgF/drgK          per-rule drag multipliers (0 = 1.0× pass-through)
            //
            //                                              en     n    hueB   hueV   bri    drg   nz      rul  rotS    rotAxis                  orbS  pal  pul    cnv    srg    hct    anc    trs           sepR   alnR    cohR    sepW   alnW   cohW   maxS   gst  drgB  drgO  drgF  drgK
            static constexpr OrbMoodConfig ORB_MOOD_TABLE[MOOD_COUNT] = {
                /* 0 open_default        */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f, 20.0f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  true,  true,  true,  0.12f, false, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
                /* 1 open_sunset         */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f, 20.0f,  3u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  true,  false, false, 0.08f, false, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
                /* 2 indoor_flat         */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f, 0.0f,   0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  false, false, false, 0.12f, false, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
                /* 3 indoor_vault        */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f, 0.0f,   0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  false, false, false, 0.12f, false, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
                /* 4 finite_outdoor      */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f, 20.0f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  true,  true,  true,  0.12f, false, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
                /* 5 finite_outdoor_ref  */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f, 20.0f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  true,  true,  true,  0.12f, false, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
            };

            // ── Agents (modules/agents.inl) ──
            // Unified entity registry: behaviors, tier gains, populations.
            // Pass 1 scaffold — registries declared; kernel/render wiring
            // lands in later steps.
#include "modules/agents.inl"

// ── Spawn Engine & Entity Lifecycle (modules/spawn_engine.inl) ──
#include "modules/spawn_engine.inl"

// ─── Sphere Entity System ────────────────────────────────────────
//
// Orbital spheres. Rare, PGA motor-driven orbits around anchors.
// Slots 0 .. MAX_SPHERE_INSTANCES-1 in the shared floating entity buffer.
// GPU compute: update_sphere. Vertex shader: sphere_vs.
//
// SEAM[sphere:taxonomy] sphere VOCABULARY lives here, not in entities.inl
//   (Ch. 12.C). Generic-pipeline floater family — vocabulary class
//   distinct from grounded families. Phase 2 extraction target:
//   floater_vocabulary.inl (D-floater inclining β with naming care).
// SEAM[sphere:L1] FloatingEntityTierProfile naming claims more than code
//   delivers — used only for spheres; cubes have their own CubeTierProfile.
//   The "(Reuses FloatingEntityTierProfile...)" comment below is also
//   misleading: cubes do NOT reuse this struct in practice.
//   Phase 3 cleanup: rename to SphereTierProfile, update the comment.
// ─────────────────────────────────────────────────────────────────

            // ─── Sphere Tier Profile ─────────────────────────────────────
            // (Reuses FloatingEntityTierProfile — orbit fields are meaningful,
            //  hover-bob fields are zero.)
            // SEAM[sphere:L1] above comment is incorrect: cubes use a
            //   distinct CubeTierProfile struct (line ~3155). The "reuse"
            //   was likely planned but didn't land.
            struct FloatingEntityTierProfile {
                float body_radius_mean, body_radius_sigma;
                float orbit_radius_mean, orbit_radius_sigma;
                float orbit_height_mean, orbit_height_sigma;
                float orbit_speed_mean, orbit_speed_sigma;
                float influence_radius_mean, influence_radius_sigma;
                float spin_speed_mean, spin_speed_sigma;
                float bob_amplitude_mean, bob_amplitude_sigma;
                float bob_period_mean, bob_period_sigma;
                float spin_tilt_sigma;
                float aspect_y_mean, aspect_y_sigma;
                float aspect_z_mean, aspect_z_sigma;
                float face_variance_mean, face_variance_sigma;
                uint32_t geometry_type;
                uint32_t motion_type;
                float weight;
            };

            static constexpr uint32_t SPHERE_TIER_COUNT = 2;
            static constexpr FloatingEntityTierProfile SPHERE_TIERS[SPHERE_TIER_COUNT] = {
                //                    rad_μ  σ     orb_μ  σ     ht_μ   σ      spd_μ  σ     inf_μ  σ     spin_μ σ     bob_μ  σ    per_μ  σ    tilt   asp_y  σ    asp_z  σ    fvar_μ σ     geo  mot  wt
                /* 0: Sentinel  */ {  1.5f, 0.3f,  12.0f, 3.0f,   6.0f,  2.0f, 1.4f, 0.3f,  8.0f, 2.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f,0.0f,  1.0f,0.0f,  0.0f,0.0f,   0, 0, 0.65f },
                /* 1: Anomaly   */ {  1.2f, 0.2f,   8.0f, 2.0f,   4.0f,  1.5f, 2.0f, 0.5f,  6.0f, 1.5f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,  1.0f,0.0f,  1.0f,0.0f,  0.0f,0.0f,   0, 0, 0.35f },
            };

            static constexpr float SPHERE_BASE_TIER_WEIGHTS[SPHERE_TIER_COUNT] = { 0.65f, 0.35f };
            static constexpr const char* SPHERE_TIER_NAMES[] = { "Sentinel", "Anomaly" };

            // ─── Sphere Spawn Configuration ──────────────────────────────
            struct SphereConfig {
                static constexpr float SPAWN_CHANCE = 0.015f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.4f;
            };

            // ─── Property Index Registry (Sphere) ────────────────────────
            // Seed source: tile_seed (shared with all families)
            // Range: 100–126 (original floating range, preserved for seed stability)
            struct FloatingEntityProp {
                static constexpr uint32_t SPAWN_ROLL = 100u;
                static constexpr uint32_t ANCHOR_X = 101u;
                static constexpr uint32_t ANCHOR_Z = 102u;
                static constexpr uint32_t TIER = 103u;
                static constexpr uint32_t BODY_RADIUS = 110u;
                static constexpr uint32_t ORBIT_RADIUS = 111u;
                static constexpr uint32_t ORBIT_HEIGHT = 112u;
                static constexpr uint32_t ORBIT_SPEED = 113u;
                static constexpr uint32_t INFLUENCE_RADIUS = 114u;
                static constexpr uint32_t SPIN_SPEED = 115u;
                static constexpr uint32_t BOB_AMPLITUDE = 116u;
                static constexpr uint32_t BOB_PERIOD = 117u;
                static constexpr uint32_t SPIN_TILT_X = 118u;
                static constexpr uint32_t SPIN_TILT_Z = 119u;
                static constexpr uint32_t COLOR_R = 120u;
                static constexpr uint32_t COLOR_G = 121u;
                static constexpr uint32_t COLOR_B = 122u;
                static constexpr uint32_t ASPECT_Y = 123u;
                static constexpr uint32_t ASPECT_Z = 124u;
                static constexpr uint32_t FACE_VARIANCE = 125u;
                static constexpr uint32_t ROTATION = 126u;
            };

            // ─── Sphere CPU Tracking ─────────────────────────────────────
            //
            // SEAM[sphere:P5] last_alloc_time is pattern P5 (release-pending
            //   sentinel / race protection) — CPU-timestamp variant. When
            //   GPU readback arrives stale ("kernel evicted this slot"), the
            //   timestamp protects freshly-allocated slots from being
            //   incorrectly marked inactive. Same intent as
            //   floaters.inl::toggle_cube_kite_mode's GPU sentinel; different
            //   mechanism. Genuinely correct placement.
            struct ActiveFloater {
                int32_t patch_gx = 0, patch_gz = 0;
                int32_t host_gx = 0, host_gz = 0;
                // See ActiveCube::last_alloc_time — same race protection
                // for sphere slots. Spheres rarely evict in practice
                // (orbital, anchored at origin), but the readback path
                // covers them uniformly so the protection covers them
                // uniformly too.
                float   last_alloc_time = -1000.0f;
                bool active = false;
            };
            ActiveFloater activeFloaters_[Dim::MAX_SPHERE_INSTANCES]{};
            uint32_t activeFloaterCount_ = 0;

            // ═══ Sphere Entity System End ═════════════════════════════════


// ─── Cube Entity System ─────────────────────────────────────────
//
// Hover-bob monoliths. Colorful cubes/slabs floating above terrain.
// Slots 0 .. MAX_CUBE_INSTANCES-1 (buffer offset by CUBE_SLOT_OFFSET).
// GPU compute: update_cube. Vertex shader: monolith_vs.
//
// SEAM[cube:taxonomy] cube VOCABULARY lives here, not in entities.inl
//   (Ch. 12.C). Verified by Ch. 13 chunk-3 read — the seam map's
//   Ch. 9 cube-three-tier-home claim was reframed: vocabulary in
//   13.B (here), sampling profile in entity_pipeline.inl, behavior
//   gains in floaters.inl. Three concerns, three files, each correct.
// SEAM[cube:cx-cz-mirror] ActiveCube has cx, cz fields — CPU mirror of
//   GPU anchor for floaters.inl::corral_cubes / toggle_cube_kite_mode
//   to read without GPU readback. Same family as agents:D2 (slot-0
//   reads); when pawn.inl extracts and provides accessors,
//   corral/kite could analogously have cube_anchor(slot) accessors.
// ─────────────────────────────────────────────────────────────────

            // ─── Cube Tier Profile ───────────────────────────────────────
            struct CubeTierProfile {
                float body_radius_mean, body_radius_sigma;
                float orbit_height_mean, orbit_height_sigma;
                float influence_radius_mean, influence_radius_sigma;
                float spin_speed_mean, spin_speed_sigma;
                float bob_amplitude_mean, bob_amplitude_sigma;
                float bob_period_mean, bob_period_sigma;
                float spin_tilt_sigma;
                float aspect_y_mean, aspect_y_sigma;
                float aspect_z_mean, aspect_z_sigma;
                float face_variance_mean, face_variance_sigma;
                float weight;
            };

            static constexpr uint32_t CUBE_TIER_COUNT = 4;
            static constexpr CubeTierProfile CUBE_TIERS[CUBE_TIER_COUNT] = {
                //                    rad_μ  σ     ht_μ   σ      inf_μ  σ     spin_μ σ     bob_μ  σ    per_μ  σ    tilt   asp_y  σ    asp_z  σ    fvar_μ σ     wt
                /* 0: SmallCube */ {  1.8f, 0.5f,  15.0f, 12.0f,  6.0f, 1.5f,  0.04f,0.015f,1.0f, 0.3f,  5.0f, 1.5f,  0.12f, 1.0f,0.15f, 1.0f,0.15f, 0.40f,0.12f,  0.40f },
                /* 1: MedCube   */ {  4.0f, 1.2f,  25.0f, 18.0f, 10.0f, 2.0f,  0.03f,0.01f, 1.5f, 0.4f,  6.0f, 2.0f,  0.10f, 1.0f,0.20f, 1.0f,0.20f, 0.45f,0.15f,  0.32f },
                /* 2: LargeCube */ {  8.0f, 2.5f,  35.0f, 20.0f, 14.0f, 3.0f,  0.02f,0.008f,2.0f, 0.5f,  8.0f, 2.5f,  0.08f, 1.0f,0.25f, 1.0f,0.25f, 0.35f,0.10f,  0.20f },
                /* 3: Monolith  */ {  3.0f, 0.8f,   5.0f,  2.0f, 12.0f, 3.0f,  0.015f,0.005f,1.2f,0.3f,  6.0f, 2.0f,  0.10f, 5.0f,1.2f,  0.15f,0.03f, 0.45f,0.12f,  0.08f },
            };

            static constexpr float CUBE_BASE_TIER_WEIGHTS[CUBE_TIER_COUNT] = { 0.40f, 0.32f, 0.20f, 0.08f };
            static constexpr const char* CUBE_TIER_NAMES[] = { "SmallCube", "MedCube", "LargeCube", "Monolith" };

            // ─── Cube Spawn Configuration ────────────────────────────────
            struct CubeConfig {
                static constexpr float SPAWN_CHANCE = 0.060f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.4f;
            };

            // ─── Property Index Registry (Cube) ──────────────────────────
            // Range: 130–156 (avoids sphere's 100–126)
            struct CubeEntityProp {
                static constexpr uint32_t SPAWN_ROLL = 130u;
                static constexpr uint32_t ANCHOR_X = 131u;
                static constexpr uint32_t ANCHOR_Z = 132u;
                static constexpr uint32_t TIER = 133u;
                static constexpr uint32_t BODY_RADIUS = 140u;
                static constexpr uint32_t ORBIT_HEIGHT = 142u;
                static constexpr uint32_t INFLUENCE_RADIUS = 144u;
                static constexpr uint32_t SPIN_SPEED = 145u;
                static constexpr uint32_t BOB_AMPLITUDE = 146u;
                static constexpr uint32_t BOB_PERIOD = 147u;
                static constexpr uint32_t SPIN_TILT_X = 148u;
                static constexpr uint32_t SPIN_TILT_Z = 149u;
                static constexpr uint32_t COLOR_R = 150u;
                static constexpr uint32_t COLOR_G = 151u;
                static constexpr uint32_t COLOR_B = 152u;
                static constexpr uint32_t ASPECT_Y = 153u;
                static constexpr uint32_t ASPECT_Z = 154u;
                static constexpr uint32_t FACE_VARIANCE = 155u;
                static constexpr uint32_t ROTATION = 156u;
            };

            // ─── Cube CPU Tracking ───────────────────────────────────────
            struct ActiveCube {
                int32_t patch_gx = 0, patch_gz = 0;
                int32_t host_gx = 0, host_gz = 0;
                // World XZ of the cube's anchor — mirror of fe.anchor[0,2]
                // on GPU. Captured at spawn so corral_cubes can read the
                // current anchor without a GPU readback. Updated when
                // corral writes a new anchor.
                float   cx = 0.0f, cz = 0.0f;
                // Time (currentSeconds_) when this slot was last marked
                // active. Used to suppress race between freshly allocated
                // slots and the floater readback path: readback callbacks
                // process previous-frame data, so a slot allocated this
                // frame would be incorrectly marked inactive by the readback
                // (which sees the *prior tenant* as evicted). Suppression
                // window covers two readback cycles. See render() floater
                // sync block for the consumer.
                float   last_alloc_time = -1000.0f;
                bool active = false;
            };
            ActiveCube activeCubes_[Dim::MAX_CUBE_INSTANCES]{};
            uint32_t activeCubeCount_ = 0;

            // ═══ Cube Entity System End ═══════════════════════════════════

            // ═══ Ribbon Dispatch Pipeline ═══════════════════════════════════
            //
            // Single-instance ribbon through the 3-phase dispatch pipeline.
            // GPU buffer is singleton (upload_ribbon, not slot-indexed).
            //
            // SEAM[ribbon:taxonomy] ribbon MACHINERY lives here while ribbon
            //   VOCABULARY lives in entities.inl (Ch. 12.C). UNIQUE among
            //   bespoke families — gol_zones (Ch. 12.B) and gallery (Ch. 12.E)
            //   keep both vocabulary and machinery together. Phase 2 candidate
            //   for ribbon.inl extraction (D-ribbon inclining yes) which would
            //   normalize ribbon to the gol/gallery shape.
            // SEAM[ribbon:dual-entry] commit_ribbon below has TWO callers:
            //   FAMILY_DISPATCH[RIBBON].try_commit during patch streaming,
            //   AND mood.inl::apply_mood for mood-5 forced spawn. The dual
            //   entry point is owned by mood:K4 (mood-5 reference clone),
            //   not by ribbon machinery. Tag-only awareness.

            // ─── fill_ribbon_selection_geometry ───────────────────────────
            // Shared geometry + color sampler used by both the dispatch
            // pipeline and the mood forced-spawn path.
            void fill_ribbon_selection_geometry(
                uint32_t seed, uint32_t tier_idx, float terrain_est,
                RibbonSelection& sel) const
            {
                const auto& tp = RIBBON_TIERS[tier_idx];

                float count_f = std::max(20.0f,
                    cpu_sample_gaussian(seed, RibbonProp::CUBE_COUNT, tp.cube_count_mean, tp.cube_count_sigma));
                sel.cube_count = std::min((uint32_t)count_f, Dim::RIBBON_MAX_RINGS);
                sel.cube_size = std::max(1.0f,
                    cpu_sample_gaussian(seed, RibbonProp::CUBE_SIZE, tp.cube_size_mean, tp.cube_size_sigma));

                // Length cap — keeps anchor coverage viable (~30 patches max)
                if ((float)sel.cube_count * sel.cube_size > RIBBON_MAX_LENGTH)
                    sel.cube_count = (uint32_t)(RIBBON_MAX_LENGTH / sel.cube_size);

                sel.height = terrain_est + std::max(20.0f,
                    cpu_sample_gaussian(seed, RibbonProp::HEIGHT, tp.height_mean, tp.height_sigma));

                sel.orientation = cpu_hash_f(seed, RibbonProp::ORIENTATION) * 6.2831853f;

                sel.lateral_amp = std::max(0.1f, cpu_sample_gaussian(seed, RibbonProp::LATERAL_AMP, tp.lateral_amp_mean, tp.lateral_amp_sigma));
                sel.lateral_cycles = std::max(0.1f, cpu_sample_gaussian(seed, RibbonProp::LATERAL_CYCLES, tp.lateral_cycles_mean, tp.lateral_cycles_sigma));
                sel.lateral_speed = std::max(0.005f, cpu_sample_gaussian(seed, RibbonProp::LATERAL_SPEED, tp.lateral_speed_mean, tp.lateral_speed_sigma));

                sel.vertical_amp = std::max(0.1f, cpu_sample_gaussian(seed, RibbonProp::VERTICAL_AMP, tp.vertical_amp_mean, tp.vertical_amp_sigma));
                sel.vertical_cycles = sel.lateral_cycles;
                sel.vertical_speed = sel.lateral_speed;

                sel.twist_amp = std::max(0.0f, cpu_sample_gaussian(seed, RibbonProp::TWIST_AMP, tp.twist_amp_mean, tp.twist_amp_sigma));
                sel.twist_cycles = sel.lateral_cycles;
                sel.twist_speed = sel.lateral_speed;

                // Color
                float color_roll = cpu_hash_f(seed, RibbonProp::COLOR_ROLL);
                sel.color_mode = RibbonColorMode::COUNT - 1;
                float ccum = 0.0f;
                for (uint32_t c = 0; c < RibbonColorMode::COUNT; c++) {
                    ccum += RibbonColorMode::WEIGHTS[c];
                    if (color_roll < ccum) { sel.color_mode = c; break; }
                }

                if (sel.color_mode == RibbonColorMode::SMOOTH) {
                    uint32_t pal_idx = (uint32_t)(cpu_hash_f(seed, RibbonProp::PALETTE_IDX) * RIBBON_SMOOTH_PALETTE_COUNT);
                    if (pal_idx >= RIBBON_SMOOTH_PALETTE_COUNT) pal_idx = RIBBON_SMOOTH_PALETTE_COUNT - 1;
                    float var = cpu_hash_f(seed, RibbonProp::COLOR_R) * 0.10f - 0.05f;
                    sel.color[0] = RIBBON_SMOOTH_PALETTE[pal_idx][0] + var;
                    sel.color[1] = RIBBON_SMOOTH_PALETTE[pal_idx][1] + var * 0.8f;
                    sel.color[2] = RIBBON_SMOOTH_PALETTE[pal_idx][2] + var * 0.6f;
                }
                else if (sel.color_mode == RibbonColorMode::TINTED) {
                    sel.color[0] = cpu_hash_f(seed, RibbonProp::COLOR_R) * 0.45f + 0.40f;
                    sel.color[1] = cpu_hash_f(seed, RibbonProp::COLOR_G) * 0.40f + 0.35f;
                    sel.color[2] = cpu_hash_f(seed, RibbonProp::COLOR_B) * 0.45f + 0.35f;
                }
                else {
                    float hue = cpu_hash_f(seed, RibbonProp::COLOR_R);
                    sel.color[0] = 0.20f + hue * 0.35f;
                    sel.color[1] = 0.18f + (1.0f - hue) * 0.30f;
                    sel.color[2] = 0.22f + cpu_hash_f(seed, RibbonProp::COLOR_B) * 0.25f;
                }

                sel.footprint_r = 5.0f;
            }

            // ─── select_ribbon_for_patch ──────────────────────────────────
            bool select_ribbon_for_patch(int32_t gx, int32_t gz, RibbonSelection& sel) {
                // Tip-overlap idempotency: reject if ANY active ribbon's
                // near or far tip falls within this trigger patch.
                for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                    if (!activeRibbons_[i].active) continue;
                    if ((activeRibbons_[i].near_tip_gx == gx && activeRibbons_[i].near_tip_gz == gz) ||
                        (activeRibbons_[i].far_tip_gx == gx && activeRibbons_[i].far_tip_gz == gz))
                        return false;
                }
                auto gate = run_spawn_preamble(gx, gz,
                    activeRibbons_, MAX_RIBBON_INSTANCES,
                    RibbonProp::SPAWN_ROLL, RibbonConfig::SPAWN_CHANCE,
                    RibbonConfig::MOOD_MULTIPLIER,
                    PopFamily::RIBBON, "ribn");
                if (!gate.ok) return false;

                // Tier selection with theme bias
                float tier_weights[RIBBON_TIER_COUNT];
                for (uint32_t t = 0; t < RIBBON_TIER_COUNT; t++)
                    tier_weights[t] = RIBBON_BASE_TIER_WEIGHTS[t];
                for (uint32_t t = 0; t < RIBBON_TIER_COUNT; t++)
                    tier_weights[t] *= THEMES[gate.theme_idx].tier_wt_ribbon[t];
                uint32_t tier_idx = select_tier_biased(gate.seed, RibbonProp::TIER,
                    tier_weights, RIBBON_TIER_COUNT, PopFamily::RIBBON);

                sel.seed = gate.seed;
                sel.trigger_gx = gx;
                sel.trigger_gz = gz;
                sel.slot = gate.slot;
                sel.tier_idx = tier_idx;

                float terrain_est = estimate_terrain_height(
                    (gx + 0.5f) * PATCH_EXTENT, (gz + 0.5f) * PATCH_EXTENT);

                fill_ribbon_selection_geometry(gate.seed, tier_idx, terrain_est, sel);

                // Constrain orientation: ribbon body extends primarily away
                // from the pawn. The hash provides ±60° of spread around the
                // away direction so ribbons aren't all perfectly radial.
                {
                    float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
                    float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
                    float away_angle = std::atan2(patch_cz - pawnReadback_z_,
                        patch_cx - pawnReadback_x_);
                    constexpr float SPREAD = 1.0472f; // ±60° = π/3
                    float hash_spread = cpu_hash_f(gate.seed, RibbonProp::ORIENTATION);
                    sel.orientation = away_angle + (hash_spread * 2.0f - 1.0f) * SPREAD;
                }

                return true;
            }

            // ─── place_ribbon_from_selection ──────────────────────────────
            bool place_ribbon_from_selection(const RibbonSelection& sel, RibbonPlacement& plan) {
                auto pos = negotiate_position(sel.seed,
                    sel.trigger_gx, sel.trigger_gz,
                    RibbonProp::ANCHOR_X, RibbonProp::ANCHOR_Z,
                    RibbonConfig::POSITION_JITTER,
                    RibbonProp::ORIENTATION,
                    sel.footprint_r, PopFamily::RIBBON, sel.tier_idx);
                if (!pos.ok) return false;

                plan = RibbonPlacement{};
                plan.slot = sel.slot;
                plan.trigger_gx = sel.trigger_gx;
                plan.trigger_gz = sel.trigger_gz;
                plan.host_gx = pos.host_gx;
                plan.host_gz = pos.host_gz;
                plan.tier_idx = sel.tier_idx;
                plan.cx = pos.cx;
                plan.cz = pos.cz;
                plan.rotation = pos.rotation;

                plan.cube_count = sel.cube_count;
                plan.cube_size = sel.cube_size;
                plan.height = sel.height;
                plan.orientation = sel.orientation;
                plan.lateral_amp = sel.lateral_amp;
                plan.lateral_cycles = sel.lateral_cycles;
                plan.lateral_speed = sel.lateral_speed;
                plan.vertical_amp = sel.vertical_amp;
                plan.vertical_cycles = sel.vertical_cycles;
                plan.vertical_speed = sel.vertical_speed;
                plan.twist_amp = sel.twist_amp;
                plan.twist_cycles = sel.twist_cycles;
                plan.twist_speed = sel.twist_speed;
                plan.color_mode = sel.color_mode;
                std::memcpy(plan.color, sel.color, sizeof(plan.color));

                record_placement_bookkeeping(PopFamily::RIBBON, plan.tier_idx);
                return true;
            }

            // ─── commit_ribbon ───────────────────────────────────────────
            void commit_ribbon(const RibbonPlacement& plan,
                int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue)
            {
                GPURibbonState r{};
                r.anchor[0] = plan.cx;
                r.anchor[1] = 0.0f;
                r.anchor[2] = plan.cz;
                r.time = currentSeconds_;
                r.cube_count = plan.cube_count;
                r.cube_size = plan.cube_size;
                r.height = plan.height;
                r.orientation = plan.orientation;
                r.lateral_amp = plan.lateral_amp;
                r.lateral_cycles = plan.lateral_cycles;
                r.lateral_speed = plan.lateral_speed;
                r.vertical_amp = plan.vertical_amp;
                r.vertical_cycles = plan.vertical_cycles;
                r.vertical_speed = plan.vertical_speed;
                r.twist_amp = plan.twist_amp;
                r.twist_cycles = plan.twist_cycles;
                r.twist_speed = plan.twist_speed;
                r.color_mode = plan.color_mode;
                r.color[0] = plan.color[0];
                r.color[1] = plan.color[1];
                r.color[2] = plan.color[2];
                r.is_visible = 1u;

                // Store in CPU mirror (per-frame nearest-selection uploads to GPU)
                uint32_t s = plan.slot;
                ribbonStates_[s] = r;

                auto& ar = activeRibbons_[s];
                ar.patch_gx = trigger_gx;
                ar.patch_gz = trigger_gz;
                ar.host_gx = plan.host_gx;
                ar.host_gz = plan.host_gz;
                ar.anchor_x = plan.cx;
                ar.anchor_z = plan.cz;

                // Two-tip anchoring: anchor IS the near tip (t=0).
                // Body extends entirely in the orientation direction (away from pawn).
                float total_length = (float)plan.cube_count * plan.cube_size;
                float dir_x = std::cos(plan.orientation);
                float dir_z = std::sin(plan.orientation);

                ar.near_tip_x = plan.cx;
                ar.near_tip_z = plan.cz;
                ar.far_tip_x = plan.cx + dir_x * total_length;
                ar.far_tip_z = plan.cz + dir_z * total_length;

                ar.near_tip_gx = (int32_t)std::floor(ar.near_tip_x / PATCH_EXTENT);
                ar.near_tip_gz = (int32_t)std::floor(ar.near_tip_z / PATCH_EXTENT);
                ar.far_tip_gx = (int32_t)std::floor(ar.far_tip_x / PATCH_EXTENT);
                ar.far_tip_gz = (int32_t)std::floor(ar.far_tip_z / PATCH_EXTENT);

                ar.near_tip_registered = false;
                ar.far_tip_registered = false;
                ar.ref_count = 0;

                ar.active = true;
                activeRibbonCount_++;
                // SEAM[ribbon:L1] unconditional stdout — exhibition guard
                //   candidate. Same family as the [DIAG:*] stdout pattern
                //   noted across the codebase. Phase 1+ batch: wrap in
                //   #ifdef DIAG_RIBBON or similar before exhibition.
                std::cout << "[Ribbon] SPAWN slot=" << s << " at (" << plan.cx << ", " << plan.cz
                    << ") tier=" << plan.tier_idx
                    << " len=" << total_length
                    << " near=(" << ar.near_tip_gx << "," << ar.near_tip_gz
                    << ") far=(" << ar.far_tip_gx << "," << ar.far_tip_gz << ")\n";
            }

            // ═══ Ribbon Dispatch Pipeline End ═══════════════════════════════

            // ── GoL Zones (modules/gol_zones.inl) ──
#include "modules/gol_zones.inl"

            // ── Gallery System (modules/gallery.inl) ──
#include "modules/gallery.inl"

            uint32_t activeSeed_ = 42;     // world master seed (mutable for world transitions)
            // Patch dimensions aliased from Dim:: for local readability
            static constexpr float    PATCH_EXTENT = Dim::PATCH_EXTENT;
            static constexpr uint32_t GRID_RADIUS = Dim::PATCH_GRID_RADIUS;   // inner priority (3 → 7×7)
            static constexpr uint32_t GRID_SIDE = Dim::PATCH_GRID_SIDE;
            static constexpr uint32_t RENDER_RADIUS = Dim::PATCH_RENDER_RADIUS;  // visible radius (5)
            static constexpr uint32_t RENDER_SIDE = Dim::PATCH_RENDER_SIDE;
            static constexpr uint32_t PREGEN_RADIUS = Dim::PATCH_PREGEN_RADIUS; // deep pre-gen buffer (7)
            static constexpr uint32_t MAX_PATCHES = Dim::MAX_ACTIVE_PATCHES;    // 225

            // Runtime render radius — toggleable within [GRID_RADIUS, RENDER_RADIUS].
            // Buffers/textures always allocated for PREGEN_RADIUS.
            // Visibility uses circular VISIBLE_RADIUS; RENDER_RADIUS retained for
            // allocation bounds and GoL zone eviction.
            uint32_t activeRadius_ = PREGEN_RADIUS;

            enum class PatchPhase : uint8_t {
                ALLOCATED,      // layer assigned, tile cached, no entities yet
                SPAWNED,        // entities selected + placed + committed
                GENERATED,      // heightfield computed, gallery + GoL spawned
                NEEDS_REGEN,    // heightfield stale (new pier in range)
            };

            struct ActivePatch {
                int32_t grid_x = 0;
                int32_t grid_z = 0;
                uint32_t layer = 0;
                bool valid = false;
                PatchPhase phase = PatchPhase::ALLOCATED;
                bool animated = false;   // true if patch overlaps an active pool

                // Entity ownership (recorded at commit, read at eviction)
                struct EntityRef {
                    uint32_t family;   // PopFamily index
                    uint32_t slot;     // index into Active* array
                };
                static constexpr uint32_t MAX_ENTITY_REFS = 10;
                EntityRef entity_refs[MAX_ENTITY_REFS]{};
                uint32_t entity_ref_count = 0;

                void record_entity(uint32_t family, uint32_t slot) {
                    if (entity_ref_count < MAX_ENTITY_REFS) {
                        entity_refs[entity_ref_count++] = { family, slot };
                    }
                }

                void unregister_entity(uint32_t family, uint32_t slot) {
                    for (uint32_t i = 0; i < entity_ref_count; i++) {
                        if (entity_refs[i].family == family && entity_refs[i].slot == slot) {
                            entity_refs[i] = entity_refs[--entity_ref_count];
                            return;
                        }
                    }
                }


            };

            ActivePatch patches_[MAX_PATCHES]{};
            int32_t lastCenterX_ = INT32_MAX;  // force full regeneration on first frame
            int32_t lastCenterZ_ = INT32_MAX;
            uint32_t activePatchCount_ = 0;
            uint32_t renderPatchCount_ = 0;  // visible patches (within circular VISIBLE_RADIUS)
            uint32_t lod0PatchCount_ = 0;    // subset of rendered: within LOD_FULL_RADIUS (full mesh)
            uint32_t allPatchCount_ = 0;     // all generated patches (including pre-gen ring)
            uint32_t entitiesCulled_ = 0;    // entities hidden by distance culling this frame

            // Find the ActivePatch entry for a given grid coordinate.
            // Returns nullptr if not found (should not happen for host patches
            // within the allocation window).
            ActivePatch* find_patch(int32_t gx, int32_t gz) {
                for (uint32_t i = 0; i < activePatchCount_; i++) {
                    if (patches_[i].valid && patches_[i].grid_x == gx && patches_[i].grid_z == gz)
                        return &patches_[i];
                }
                return nullptr;
            }

            // Hook: full eviction of a single patch.
            void evict_patch(uint32_t pi, wgpu::Queue& queue) {
                free_layer(patches_[pi].layer);
                // Painting eviction now handled by dispatch_evict_gallery via entity_refs
                evict_patch_entities(patches_[pi], queue);
                unregister_footprints_for_patch(patches_[pi].grid_x, patches_[pi].grid_z);
                patches_[pi].valid = false;
            }

            void evict_patch_entities(ActivePatch& patch, wgpu::Queue& queue) {
#ifdef DIAG_ENTITY_LIFECYCLE
                if (patch.entity_ref_count > 0) {
                    float wx = (patch.grid_x + 0.5f) * PATCH_EXTENT;
                    float wz = (patch.grid_z + 0.5f) * PATCH_EXTENT;
                    float dx = wx - pawnReadback_x_, dz = wz - pawnReadback_z_;
                    std::cout << "[DIAG:EVICT] patch(" << patch.grid_x << "," << patch.grid_z
                        << ") dist=" << std::sqrt(dx * dx + dz * dz)
                        << " refs=" << patch.entity_ref_count << "\n";
                }
#endif
                for (uint32_t i = 0; i < patch.entity_ref_count; i++) {
                    auto& ref = patch.entity_refs[i];
                    FAMILY_DISPATCH[ref.family].evict_slot(this, ref.slot, queue);
                }

                // DONE[phase-1:spine:L1] Vestigial for-loop removed. Git history
                //   (commit 6f0007f) shows the loop body was originally
                //   `clear_entity_presence(gx, gz, FAMILY_DISPATCH[f].presence_clear_flag)`,
                //   removed when the entity-presence flag system was retired in
                //   favor of the entity_refs[] registry (the loop above this).
                //   The wrapper + gx/gz decls were left behind, accidentally
                //   absorbing `patch.entity_ref_count = 0` as a phantom body
                //   that ran PopFamily::COUNT times to set the same field to 0.
                //   Fix: keep the single assignment, drop the wrapper.
                patch.entity_ref_count = 0;
            }

            void audit_entity_integrity() {
#ifdef DIAG_ENTITY_LIFECYCLE
                // DONE[phase-1:spine:L2] Coverage rationale documented.
                //   Audit covers 4 of 12 families: Arch, Column, Antenna,
                //   Pyramid — the generic-pipeline grounded families whose
                //   lifecycle is the simple { Active*[slot].active ↔ patch
                //   entity_refs } pair this audit checks. Coverage gaps
                //   split into two categories:
                //
                //   - Palm, Cactus, Blade — same generic-pipeline grounded
                //     pattern. Audit shape applies cleanly; not yet extended.
                //     Extension is mechanical (mirror the Arch/Column/etc.
                //     blocks) but DEFERRED: the audit's design needs review
                //     before adding families (see follow-up note below).
                //
                //   - Sphere, Cube, Ribbon, GoL, Gallery — different lifecycle
                //     shapes where the simple "active vs ref" model doesn't
                //     cleanly apply: spheres/cubes are managed via the GPU
                //     floater readback path (last_alloc_time race protection,
                //     not entity_refs); ribbon uses two-tip anchoring with
                //     ref_count for partial eviction; GoL/gallery have their
                //     own bespoke state machines. Auditing these would
                //     require per-family audit shapes, not a uniform
                //     extension of the existing pattern.
                //
                //   FOLLOW-UP[seam-map] Before extending coverage to
                //   Palm/Cactus/Blade, the audit's structure should be
                //   reviewed — currently each family is a hand-written block,
                //   so 3 more families means 3× duplication. A registry-of-
                //   audit-shapes (one entry per family lifecycle pattern)
                //   would scale better. Not in scope for Phase 1.
                // Count actual active slots
                uint32_t act_a = 0, act_c = 0, act_n = 0, act_p = 0;
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) if (activeArches_[i].active) act_a++;
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) if (activeColumns_[i].active) act_c++;
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) if (activeAntennas_[i].active) act_n++;
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) if (activePyramids_[i].active) act_p++;

                // Count consistency
                if (act_a != activeArchCount_)
                    std::cout << "[DIAG:AUDIT] ARCH COUNT active=" << act_a << " tracked=" << activeArchCount_ << "\n";
                if (act_c != activeColumnCount_)
                    std::cout << "[DIAG:AUDIT] COL COUNT active=" << act_c << " tracked=" << activeColumnCount_ << "\n";
                if (act_n != activeAntennaCount_)
                    std::cout << "[DIAG:AUDIT] ANT COUNT active=" << act_n << " tracked=" << activeAntennaCount_ << "\n";
                if (act_p != activePyramidCount_)
                    std::cout << "[DIAG:AUDIT] PYR COUNT active=" << act_p << " tracked=" << activePyramidCount_ << "\n";

                // Collect refs from all patches
                bool ra[Dim::MAX_ARCH_INSTANCES]{};
                bool rc[Dim::MAX_COLUMN_ONLY]{};
                bool rn[Dim::MAX_ANTENNA_ONLY]{};
                bool rp[Dim::MAX_PYRAMID_INSTANCES]{};
                for (uint32_t p = 0; p < activePatchCount_; p++) {
                    if (!patches_[p].valid) continue;
                    for (uint32_t r = 0; r < patches_[p].entity_ref_count; r++) {
                        auto& ref = patches_[p].entity_refs[r];
                        switch (ref.family) {
                        case PopFamily::PYRAMID:
                            if (ref.slot < Dim::MAX_PYRAMID_INSTANCES) {
                                if (rp[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF pyr slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                rp[ref.slot] = true;
                            } break;
                        case PopFamily::ARCH:
                            if (ref.slot < Dim::MAX_ARCH_INSTANCES) {
                                if (ra[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF arch slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                ra[ref.slot] = true;
                            } break;
                        case PopFamily::COLUMN:
                            if (ref.slot < Dim::MAX_COLUMN_ONLY) {
                                if (rc[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF col slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                rc[ref.slot] = true;
                            } break;
                        case PopFamily::ANTENNA:
                            if (ref.slot < Dim::MAX_ANTENNA_ONLY) {
                                if (rn[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF ant slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                rn[ref.slot] = true;
                            } break;
                        }
                    }
                }

                // Ghost: active but no ref (will never be evicted)
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++)
                    if (activeArches_[i].active && !ra[i])
                        std::cout << "[DIAG:AUDIT] GHOST arch slot=" << i << " host=(" << activeArches_[i].host_gx << "," << activeArches_[i].host_gz << ")\n";
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++)
                    if (activeColumns_[i].active && !rc[i])
                        std::cout << "[DIAG:AUDIT] GHOST col slot=" << i << " host=(" << activeColumns_[i].host_gx << "," << activeColumns_[i].host_gz << ")\n";
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++)
                    if (activeAntennas_[i].active && !rn[i])
                        std::cout << "[DIAG:AUDIT] GHOST ant slot=" << i << " host=(" << activeAntennas_[i].host_gx << "," << activeAntennas_[i].host_gz << ")\n";
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++)
                    if (activePyramids_[i].active && !rp[i])
                        std::cout << "[DIAG:AUDIT] GHOST pyr slot=" << i << " host=(" << activePyramids_[i].host_gx << "," << activePyramids_[i].host_gz << ")\n";

                // Orphan: ref but not active (ref points to freed slot)
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++)
                    if (!activeArches_[i].active && ra[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN arch slot=" << i << "\n";
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++)
                    if (!activeColumns_[i].active && rc[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN col slot=" << i << "\n";
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++)
                    if (!activeAntennas_[i].active && rn[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN ant slot=" << i << "\n";
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++)
                    if (!activePyramids_[i].active && rp[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN pyr slot=" << i << "\n";

                // Ref overflow: any patch at capacity
                for (uint32_t p = 0; p < activePatchCount_; p++) {
                    if (patches_[p].valid && patches_[p].entity_ref_count >= ActivePatch::MAX_ENTITY_REFS)
                        std::cout << "[DIAG:AUDIT] REF FULL patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ") count=" << patches_[p].entity_ref_count << "\n";
                }
#endif
            }

            // ─── Deferred Upload Flags ───────────────────────────────────
            bool pierCountDirty_ = false;        // defer recompute_and_upload_pier_count
            bool groundEntriesDirty_ = true;     // defer upload_ground_entries (true at boot)
            bool patchInstancesDirty_ = true;    // defer LOD sort + upload_patch_instances
            bool placementDirty_ = true;         // defer dispatch_placement_correction

            // Free-list of available texture layers
            uint32_t freeLayerStack_[MAX_PATCHES]{};
            uint32_t freeLayerCount_ = MAX_PATCHES;

            // ─── Dynamic Budgets ─────────────────────────────────────────
            //
            // Entity spawning and heightfield generation are both distance-
            // driven and budgeted per frame. Spawning must complete before
            // generation (piers affect heightfields), enforced by requiring
            // phase must be SPAWNED before a patch enters the generation scan.
            static constexpr uint32_t SPAWN_BUDGET_PER_FRAME = 4;    // max patches to spawn entities for
            static constexpr uint32_t ALLOC_BUDGET_PER_FRAME = 4;    // max patches to allocate per frame
            static constexpr uint32_t EVICT_BUDGET_PER_FRAME = 4;    // max patches to evict per frame
            static constexpr uint32_t PATCH_BUDGET_MIN = 1;
            static constexpr uint32_t PATCH_BUDGET_MAX = 6;
            static constexpr uint32_t PATCH_PENDING_TIER_1 = 3;
            static constexpr uint32_t PATCH_PENDING_TIER_2 = 8;
            static constexpr uint32_t PATCH_PENDING_TIER_3 = 20;
            static constexpr uint32_t PATCH_PENDING_TIER_4 = 40;
            static constexpr uint32_t PATCH_BUDGET_MOVE_THRESHOLD = 4;

            uint32_t count_pending_patches() const {
                uint32_t n = 0;
                for (uint32_t i = 0; i < activePatchCount_; i++) {
                    if (!patches_[i].valid) continue;
                    if (patches_[i].phase == PatchPhase::SPAWNED ||
                        patches_[i].phase == PatchPhase::NEEDS_REGEN) n++;
                }
                return n;
            }

            uint32_t patches_budget_this_frame() const {
                uint32_t pending = count_pending_patches();
                uint32_t budget = PATCH_BUDGET_MIN;
                if (pending >= PATCH_PENDING_TIER_4) budget = 6;
                else if (pending >= PATCH_PENDING_TIER_3) budget = 4;
                else if (pending >= PATCH_PENDING_TIER_2) budget = 3;
                else if (pending >= PATCH_PENDING_TIER_1) budget = 2;

                bool moving = (std::abs(inputState_.move_x) > 0.01f ||
                    std::abs(inputState_.move_z) > 0.01f);
                if (moving && pending > PATCH_BUDGET_MOVE_THRESHOLD)
                    budget += 1;

                return std::min(budget, PATCH_BUDGET_MAX);
            }

            // --- Tile World System ---------------------------------------------------
            //
            // Each patch is a tile with an archetype that configures its terrain
            // character. Archetypes are rolled based on the spatial cache of
            // neighboring tiles, creating coherent regions with variety.

            // Derive finite world radius from seed within mood-defined bounds.
            static uint32_t derive_finite_radius(uint32_t seed, const MoodProfile& mood) {
                if (mood.finite_radius_min >= mood.finite_radius_max) return mood.finite_radius_min;
                uint32_t range = mood.finite_radius_max - mood.finite_radius_min + 1;
                return mood.finite_radius_min + cpu_hash(seed, 77u) % range;
            }

            // Biased mood selection for portal destinations.
            // In finite mode: 55% indoor (moods 2-3), 25% infinite outdoor (moods 0-1), 20% finite outdoor (moods 4-5).
            // In open mode: uniform across all moods.
            uint32_t pick_portal_mood(uint32_t seed, uint32_t prop) const {
                float roll = cpu_hash_f(seed, prop);
                if (finiteMode_) {
                    // 0.00–0.125: mood 0 (open_default)
                    // 0.125–0.25: mood 1 (open_sunset)
                    // 0.25–0.525: mood 2 (indoor_flat)
                    // 0.525–0.80: mood 3 (indoor_vault)
                    // 0.80–0.90:  mood 4 (finite_outdoor)
                    // 0.90–1.00:  mood 5 (finite_outdoor_ref)
                    if (roll < 0.125f) return 0;
                    if (roll < 0.25f)  return 1;
                    if (roll < 0.525f) return 2;
                    if (roll < 0.80f)  return 3;
                    if (roll < 0.90f)  return 4;
                    return 5;
                }
                return cpu_hash(seed, prop) % MOOD_COUNT;
            }

            // --- Three Archetypes ---------------------------------------------------
            //
            //  0: Mountainous — high amplitude, elevated, sparse fine detail
            //  1: Varied      — moderate amplitude, wide range, balanced
            //  2: Basin       — low amplitude, depressed, rich fine detail
            //  3: Pool        — near-flat terrain, degenerate wave shape

            static constexpr uint32_t ARCHETYPE_COUNT = 4;

            struct ArchetypeProfile {
                // ─── Terrain modifiers ───────────────────────────────
                float amp_scale;           // height field amplitude multiplier
                float height_bias;         // vertical offset (positive = elevated)
                float activation_scale;    // activity field sensitivity

                // ─── Selection ───────────────────────────────────────
                float base_weight;         // prior probability (before neighbor influence)

                // ─── Per-tile jitter ─────────────────────────────────
                float amp_jitter_range;    // amp_scale *= 1 ± jitter/2
                float bias_jitter_range;   // height_bias += uniform(-jitter/2, +jitter/2)
            };

            //                                     amp   bias   act   weight  amp_jit  bias_jit
            static constexpr ArchetypeProfile ARCHETYPES[ARCHETYPE_COUNT] = {
                /* 0: mountainous */  {  2.0f,   4.0f,  0.7f,  1.8f,   0.3f,    1.0f  },
                /* 1: varied      */  {  1.0f,   0.0f,  1.0f,  1.3f,   0.3f,    1.0f  },
                /* 2: basin       */  {  0.5f,  -2.0f,  1.3f,  1.0f,   0.3f,    1.0f  },
                /* 3: pool        */  {  0.04f, -0.5f,  0.2f,  0.0f,   0.02f,   0.2f  },
            };

            // Neighbor coherence rules for archetype selection.
            // These control how the presence of neighboring archetypes
            // biases the selection for a new tile.
            struct ArchetypeSelectionRules {
                // Neighbor count thresholds and corresponding weight multipliers.
                // Applied in order: first matching threshold wins.
                static constexpr uint32_t DOMINANT_THRESHOLD = 4;    // >= this many → suppress
                static constexpr float    DOMINANT_MULTIPLIER = 0.2f; // strongly reduced
                static constexpr uint32_t COMMON_THRESHOLD = 2;    // >= this many → mild boost
                static constexpr float    COMMON_MULTIPLIER = 1.5f;
                static constexpr uint32_t PRESENT_THRESHOLD = 1;    // == this many → strong coherence
                static constexpr float    PRESENT_MULTIPLIER = 2.0f;
                // 0 neighbors: weight stays at base_weight (no modification)
            };

            // ── Entity Density Field ─────────────────────────────────────────
            //
            // Coarse spatial noise that creates dense and sparse regions.
            // Evaluated per-tile in generate_tile_state, stored on TileState.
            // All entity spawn gates multiply by this value.
            //
            //  ┌──────────────────────────────────┬───────────┬──────────────────────────────────────┐
            //  │ Constant                         │ Value     │ Effect                                │
            //  ├──────────────────────────────────┼───────────┼──────────────────────────────────────┤
            //  │ DENSITY_LATTICE_SPACING          │ 250 wu    │ Region size (~5 patches)              │
            //  │ DENSITY_SEED_BAND                │ 160       │ Decorrelated from terrain/color       │
            //  │ DENSITY_EXPONENT                 │ 0.6       │ <1 = skew toward dense, >1 = sparse  │
            //  │ DENSITY_MIN                      │ 0.1       │ Floor (never fully empty)             │
            //  │ DENSITY_MAX                      │ 3.0       │ Ceiling (3× base spawn rates)         │
            //  └──────────────────────────────────┴───────────┴──────────────────────────────────────┘

            static constexpr float DENSITY_LATTICE_SPACING = 250.0f;
            static constexpr uint32_t DENSITY_SEED_BAND = 160u;
            static constexpr float DENSITY_EXPONENT = 0.6f;
            static constexpr float DENSITY_MIN = 1.0f;
            static constexpr float DENSITY_MAX = 1.0f;

            // ─── Family Dispatch Table ──────────────────────────────────────
            //
            // Table-driven dispatch for the full entity lifecycle:
            // select, place, commit, evict, and mesh generation.
            // Adding a new entity family: write select/place/commit/
            // prepare_mesh functions, add wrappers, add 1 row here.
            //
            // SEAM[spine:owns] FAMILY_DISPATCH is genuinely spine work — the
            //   integration hub that ties the 12 families together. Per Ch. 15.
            //   Each row's body lives in the family's owning module.
            // SEAM[spine:K2-related] the ~400 lines of dispatch_evict_*,
            //   dispatch_prepare_mesh_*, dispatch_mesh_gen_* wrappers below
            //   are integration glue, not module work — they live here
            //   correctly. New finding (Ch. 15 chunk 1): they were
            //   under-credited in the seam map but represent real spine work.
            //   NOTE[seam-map] keep wrappers here; they're the integration
            //   layer between FAMILY_DISPATCH and per-family modules.

            struct FamilyDispatch {
                bool (*try_select)(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
                bool (*try_place)(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
                void (*try_commit)(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);
                void (*evict_slot)(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
                bool (*prepare_mesh)(Cartridge* self, wgpu::Queue& queue);
                void (*dispatch_mesh)(Cartridge* self, wgpu::ComputePassEncoder& pass);
                const char* name;
            };

            // ── Mesh gen dispatch wrappers ──

            static bool dispatch_prepare_mesh_pyramid(Cartridge* self, wgpu::Queue& queue) {
                return self->prepare_pyramid_mesh_gen(queue);
            }
            static void dispatch_mesh_gen_pyramid(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_pyramid_mesh_gen(pass, self->gpuState_.pyramid_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_arch(Cartridge* self, wgpu::Queue& queue) {
                return self->prepare_arch_mesh_gen(queue);
            }
            static void dispatch_mesh_gen_arch(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_arch_mesh_gen(pass, self->gpuState_.arch_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_column(Cartridge* self, wgpu::Queue& queue) {
                return self->prepare_column_mesh_gen(queue);
            }
            static void dispatch_mesh_gen_column(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_column_mesh_gen(pass, self->gpuState_.column_mesh_gen_group());
            }

            // ── Eviction dispatch wrappers ──

            static void dispatch_evict_pyramid(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                self->cpuPyramids_.instances[slot] = GPUPyramidInstance{};
                self->activePyramids_[slot].active = false;
                self->activePyramidCount_--;
                self->groundEntriesDirty_ = true;
                { GPUPyramidMeshParams ep{}; self->gpuState_.upload_pyramid_mesh_params_slot(queue, slot, ep); }
                self->pyramidMeshGenPending_ = true;

                uint32_t max_idx = 0;
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
                    if (self->activePyramids_[i].active) max_idx = i + 1;
                }
                self->cpuPyramids_.count = max_idx;
                self->gpuState_.upload_pyramids(queue, self->cpuPyramids_);
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   pyr slot=" << slot << "\n";
#endif
            }

            static void dispatch_evict_arch(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                self->clear_pier(queue, Dim::PIER_ARCH_BASE + slot * 2);
                self->clear_pier(queue, Dim::PIER_ARCH_BASE + slot * 2 + 1);
                self->activeArches_[slot].active = false;
                self->activeArchCount_--;
                self->portalsDirty_ = true;
                { GPUArchMeshParams ep{}; self->gpuState_.upload_arch_mesh_params_slot(queue, slot, ep); }
                self->archMeshGenPending_ = true;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   arch slot=" << slot << "\n";
#endif
            }

            static void dispatch_evict_column(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                self->clear_pier(queue, Dim::PIER_COLUMN_BASE + slot);
                self->activeColumns_[slot].active = false;
                self->activeColumnCount_--;
                { GPUColumnMeshParams ep{}; self->gpuState_.upload_column_mesh_params_slot(queue, slot, ep); }
                self->columnMeshGenPending_ = true;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   col slot=" << slot << "\n";
#endif
            }

            static void dispatch_evict_antenna(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                uint32_t gpu_slot = slot + Dim::ANTENNA_SLOT_OFFSET;
                self->clear_pier(queue, Dim::PIER_COLUMN_BASE + gpu_slot);
                self->activeAntennas_[slot].active = false;
                self->activeAntennaCount_--;
                { GPUColumnMeshParams ep{}; self->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, ep); }
                self->columnMeshGenPending_ = true;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   ant slot=" << slot << "\n";
#endif
            }

            static void dispatch_evict_palm(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                self->activePalms_[slot].active = false;
                self->activePalmCount_--;
                { GPUPalmMeshParams ep{}; self->gpuState_.upload_palm_mesh_params_slot(queue, slot, ep); }
                self->palmMeshGenPending_ = true;
                self->groundEntriesDirty_ = true;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   palm slot=" << slot << "\n";
#endif
            }

            // ── Mesh gen dispatch wrappers (palm) ──

            static bool dispatch_prepare_mesh_palm(Cartridge* self, wgpu::Queue& queue) {
                return self->prepare_palm_mesh_gen(queue);
            }
            static void dispatch_mesh_gen_palm(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_palm_mesh_gen(pass, self->gpuState_.palm_mesh_gen_group());
            }

            static void dispatch_evict_cactus(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                self->activeCacti_[slot].active = false;
                self->activeCactusCount_--;
                { GPUCactusMeshParams ep{}; self->gpuState_.upload_cactus_mesh_params_slot(queue, slot, ep); }
                self->cactusMeshGenPending_ = true;
                self->groundEntriesDirty_ = true;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   cact slot=" << slot << "\n";
#endif
            }

            // ── Mesh gen dispatch wrappers (cactus) ──

            static bool dispatch_prepare_mesh_cactus(Cartridge* self, wgpu::Queue& queue) {
                return self->prepare_cactus_mesh_gen(queue);
            }
            static void dispatch_mesh_gen_cactus(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_cactus_mesh_gen(pass, self->gpuState_.cactus_mesh_gen_group());
            }

            static void dispatch_evict_blade(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue)
            {
                self->activeBlades_[slot].active = false;
                self->activeBladeCount_--;
                { GPUBladeClusterMeshParams ep{}; self->gpuState_.upload_blade_mesh_params_slot(queue, slot, ep); }
                self->bladeMeshGenPending_ = true;
                self->groundEntriesDirty_ = true;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   blad slot=" << slot << "\n";
#endif
            }

            static bool dispatch_prepare_mesh_blade(Cartridge* self, wgpu::Queue& queue) {
                return self->prepare_blade_mesh_gen(queue);
            }
            static void dispatch_mesh_gen_blade(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_blade_mesh_gen(pass, self->gpuState_.blade_mesh_gen_group());
            }

            static void dispatch_evict_sphere(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue) {
                self->activeFloaters_[slot].active = false;
                self->activeFloaterCount_--;
                GPUFloatingEntityState empty{};
                self->gpuState_.upload_sphere_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   sph slot=" << slot << "\n";
#endif
            }

            static bool dispatch_prepare_mesh_sphere(Cartridge* self, wgpu::Queue& queue) {
                (void)self; (void)queue;
                return false;  // no CPU mesh gen — GPU compute handles update_sphere
            }
            static void dispatch_mesh_gen_sphere(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                (void)self; (void)pass;
            }

            static void dispatch_evict_cube(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue) {
                self->activeCubes_[slot].active = false;
                self->activeCubeCount_--;
                GPUFloatingEntityState empty{};
                self->gpuState_.upload_cube_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   cube slot=" << slot << "\n";
#endif
            }

            static bool dispatch_prepare_mesh_cube(Cartridge* self, wgpu::Queue& queue) {
                (void)self; (void)queue;
                return false;  // no CPU mesh gen — GPU compute handles update_cube
            }
            static void dispatch_mesh_gen_cube(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                (void)self; (void)pass;
            }

            // ── GoL dispatch wrappers ──

            static bool dispatch_select_gol(Cartridge* self,
                int32_t gx, int32_t gz, EntityQueueEntry& e) {
                if (!self->moodAllowsGoLZones_) { return false; }   // mood gate — no new zones
                return self->select_gol_for_patch(gx, gz, e.gol);
            }

            static bool dispatch_place_gol(Cartridge* self,
                EntityQueueEntry& e, PlacementEntry& pe) {
                pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
                if (self->place_gol_from_selection(e.gol, pe.gol)) {
                    return true;
                }
                else {
                    self->golZones_[e.gol.slot].active = false;
                    return false;
                }
            }

            static void dispatch_commit_gol(Cartridge* self,
                PlacementEntry& pe, wgpu::Queue& queue) {
                auto* host = self->find_patch(pe.gol.host_gx, pe.gol.host_gz);
                if (host) {
                    self->commit_gol(pe.gol, pe.gx, pe.gz, queue);
                    host->record_entity(PopFamily::GOL, pe.gol.slot);
                }
                else {
                    self->golZones_[pe.gol.slot].active = false;
#ifdef DIAG_ENTITY_LIFECYCLE
                    std::cout << "[DIAG:REJECT] gol slot=" << pe.gol.slot
                        << " host=(" << pe.gol.host_gx << "," << pe.gol.host_gz
                        << ") -- no host patch\n";
#endif
                }
            }

            static void dispatch_evict_gol(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue) {
                self->gpuState_.deactivate_zone_slot(queue, slot);
                self->golZones_[slot].active = false;
                self->golZoneCount_--;
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   gol slot=" << slot << "\n";
#endif
            }

            static bool dispatch_prepare_mesh_gol(Cartridge* self, wgpu::Queue& queue) {
                (void)self; (void)queue;
                return false;  // zone mesh gen is a separate compute pass
            }
            static void dispatch_mesh_gen_gol(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                (void)self; (void)pass;
            }

            // ── Gallery dispatch wrappers ──

            static bool dispatch_select_gallery(Cartridge* self,
                int32_t gx, int32_t gz, EntityQueueEntry& e) {
                return self->select_gallery_for_patch(gx, gz, e.gallery);
            }

            static bool dispatch_place_gallery(Cartridge* self,
                EntityQueueEntry& e, PlacementEntry& pe) {
                pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
                if (self->place_gallery_from_selection(e.gallery, pe.gallery)) {
                    return true;
                }
                else {
                    self->galleryCenters_[e.gallery.slot].active = false;
                    return false;
                }
            }

            static void dispatch_commit_gallery(Cartridge* self,
                PlacementEntry& pe, wgpu::Queue& queue) {
                auto* host = self->find_patch(pe.gallery.host_gx, pe.gallery.host_gz);
                if (host) {
                    self->commit_gallery(pe.gallery, pe.gx, pe.gz, queue);
                    // Only record entity_ref if gallery is still active (commit may deactivate on 0 paintings)
                    if (self->galleryCenters_[pe.gallery.slot].active) {
                        host->record_entity(PopFamily::GALLERY, pe.gallery.slot);
                    }
                }
                else {
                    self->galleryCenters_[pe.gallery.slot].active = false;
#ifdef DIAG_ENTITY_LIFECYCLE
                    std::cout << "[DIAG:REJECT] gall slot=" << pe.gallery.slot
                        << " host=(" << pe.gallery.host_gx << "," << pe.gallery.host_gz
                        << ") -- no host patch\n";
#endif
                }
            }

            static void dispatch_evict_gallery(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue) {
                auto& gc = self->galleryCenters_[slot];
                if (gc.active) {
                    self->evict_paintings_for_patch(gc.patch_gx, gc.patch_gz, queue);
                    gc.active = false;
                }
#ifdef DIAG_ENTITY_LIFECYCLE
                std::cout << "[DIAG:EVICT]   gall slot=" << slot << "\n";
#endif
            }

            static bool dispatch_prepare_mesh_gallery(Cartridge* self, wgpu::Queue& queue) {
                (void)self; (void)queue;
                return false;
            }
            static void dispatch_mesh_gen_gallery(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                (void)self; (void)pass;
            }

            // ── Ribbon dispatch wrappers ──

            static bool dispatch_select_ribbon(Cartridge* self,
                int32_t gx, int32_t gz, EntityQueueEntry& e) {
                return self->select_ribbon_for_patch(gx, gz, e.ribbon);
            }

            static bool dispatch_place_ribbon(Cartridge* self,
                EntityQueueEntry& e, PlacementEntry& pe) {
                pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
                if (self->place_ribbon_from_selection(e.ribbon, pe.ribbon)) {
                    return true;
                }
                else {
                    self->activeRibbons_[e.ribbon.slot].active = false;
                    return false;
                }
            }

            static void dispatch_commit_ribbon(Cartridge* self,
                PlacementEntry& pe, wgpu::Queue& queue) {
                // Commit the ribbon state (GPU mirror, active record, tip positions)
                self->commit_ribbon(pe.ribbon, pe.gx, pe.gz, queue);

                uint32_t slot = pe.ribbon.slot;
                auto& ar = self->activeRibbons_[slot];

                // Register with tip patches that currently exist.
                // Late registration handles the other tip when its patch is allocated.
                uint32_t refs = 0;
                auto* near_host = self->find_patch(ar.near_tip_gx, ar.near_tip_gz);
                if (near_host) {
                    near_host->record_entity(PopFamily::RIBBON, slot);
                    ar.near_tip_registered = true;
                    refs++;
                }
                auto* far_host = self->find_patch(ar.far_tip_gx, ar.far_tip_gz);
                if (far_host && (ar.far_tip_gx != ar.near_tip_gx || ar.far_tip_gz != ar.near_tip_gz)) {
                    far_host->record_entity(PopFamily::RIBBON, slot);
                    ar.far_tip_registered = true;
                    refs++;
                }

                if (refs == 0) {
                    std::cout << "[Ribbon] REJECT slot=" << slot
                        << " — no tip patches alive\n";
                    ar = ActiveRibbon{};
                    self->ribbonStates_[slot] = GPURibbonState{};
                    self->activeRibbonCount_--;
                    return;
                }
                ar.ref_count = refs;
            }

            static void dispatch_evict_ribbon(Cartridge* self,
                uint32_t slot, wgpu::Queue& queue) {
                auto& ar = self->activeRibbons_[slot];
                if (!ar.active) return;

                // Decrement ref count — one anchor patch has been evicted.
                // Only fully evict when all referencing patches are gone.
                if (ar.ref_count > 1) {
                    ar.ref_count--;
                    return;
                }

                // Final reference gone — full eviction
                ar = ActiveRibbon{};
                self->ribbonStates_[slot] = GPURibbonState{};
                self->activeRibbonCount_--;
                if (self->renderedRibbonSlot_ == slot) {
                    GPURibbonState empty{};
                    self->gpuState_.upload_ribbon(queue, empty);
                    self->renderedRibbonSlot_ = UINT32_MAX;
                }
                std::cout << "[Ribbon] EVICT slot=" << slot << "\n";
            }

            static bool dispatch_prepare_mesh_ribbon(Cartridge* self, wgpu::Queue& queue) {
                (void)self; (void)queue;
                return false;
            }
            static void dispatch_mesh_gen_ribbon(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                (void)self; (void)pass;
                // no-op — GPU compute handles ribbon rendering
            }

            // ── Dispatch table (order matches PopFamily enum) ──

            // ── Generic Entity Pipeline (modules/entity_pipeline.inl) ──
#include "modules/entity_pipeline.inl"

            // ── Cube Behavior Registry (modules/floaters.inl) ──
            // Phase 3: cube behavior IDs, coordination knob, cycling helpers.
            // Depends on entity_pipeline.inl (cube_write_gpu seeds behavior_phase).
#include "modules/floaters.inl"

            static constexpr FamilyDispatch FAMILY_DISPATCH[PopFamily::COUNT] = {
                { dispatch_select_pyramid_generic, dispatch_place_pyramid_generic, dispatch_commit_pyramid_generic,
                  dispatch_evict_pyramid, dispatch_prepare_mesh_pyramid, dispatch_mesh_gen_pyramid,
                  "pyr" },
                { dispatch_select_arch_generic, dispatch_place_arch_generic, dispatch_commit_arch_generic,
                  dispatch_evict_arch,    dispatch_prepare_mesh_arch,    dispatch_mesh_gen_arch,
                  "arch" },
                { dispatch_select_column_generic, dispatch_place_column_generic, dispatch_commit_column_generic,
                  dispatch_evict_column,  dispatch_prepare_mesh_column,  dispatch_mesh_gen_column,
                  "col" },
                { dispatch_select_antenna_generic, dispatch_place_antenna_generic, dispatch_commit_antenna_generic,
                  dispatch_evict_antenna, dispatch_prepare_mesh_column,  dispatch_mesh_gen_column,
                  "ant" },
                { dispatch_select_palm_generic, dispatch_place_palm_generic, dispatch_commit_palm_generic,
                  dispatch_evict_palm,   dispatch_prepare_mesh_palm,   dispatch_mesh_gen_palm,
                  "palm" },
                { dispatch_select_cactus_generic, dispatch_place_cactus_generic, dispatch_commit_cactus_generic,
                  dispatch_evict_cactus, dispatch_prepare_mesh_cactus, dispatch_mesh_gen_cactus,
                  "cact" },
                { dispatch_select_blade_generic, dispatch_place_blade_generic, dispatch_commit_blade_generic,
                  dispatch_evict_blade, dispatch_prepare_mesh_blade, dispatch_mesh_gen_blade,
                  "blad" },
                { dispatch_select_sphere_generic, dispatch_place_sphere_generic, dispatch_commit_sphere_generic,
                  dispatch_evict_sphere, dispatch_prepare_mesh_sphere, dispatch_mesh_gen_sphere,
                  "sph" },
                { dispatch_select_ribbon, dispatch_place_ribbon, dispatch_commit_ribbon,
                  dispatch_evict_ribbon, dispatch_prepare_mesh_ribbon, dispatch_mesh_gen_ribbon,
                  "ribn" },
                { dispatch_select_cube_generic, dispatch_place_cube_generic, dispatch_commit_cube_generic,
                  dispatch_evict_cube, dispatch_prepare_mesh_cube, dispatch_mesh_gen_cube,
                  "cube" },
                { dispatch_select_gol, dispatch_place_gol, dispatch_commit_gol,
                  dispatch_evict_gol, dispatch_prepare_mesh_gol, dispatch_mesh_gen_gol,
                  "gol" },
                { dispatch_select_gallery, dispatch_place_gallery, dispatch_commit_gallery,
                  dispatch_evict_gallery, dispatch_prepare_mesh_gallery, dispatch_mesh_gen_gallery,
                  "gall" },
            };

            // ─── Population Themes ───────────────────────────────────────────
            //
            // A theme is the compositional intent for a region. Like a palette
            // slot sets color character, a theme sets entity character: what
            // spawns, at what scale, how densely, and how it arranges itself.
            //
            // A stochastic lattice at THEME_LATTICE_SPACING picks which theme
            // dominates at each point. Spawn weights blend smoothly across
            // boundaries. Tier bias comes from the dominant theme (no blending).
            //
            // The transition theme is the default — most of the world. Sparse
            // pyramids, small antennas, column clusters, occasional arch.
            // Interesting themes are the exceptions that emerge from the field.
            //
            //  ┌──────────────────────────────────────────────────────────────────────────────────────────┐
            //  │ THEME CONTROL SURFACE                                                                   │
            //  ├──────────────────────┬──────────────────────┬────────────────────────────────────────────┤
            //  │ Theme                │ Weight  Density      │ Character                                  │
            //  ├──────────────────────┼──────────────────────┼────────────────────────────────────────────┤
            //  │ 0: Transition        │  0.40   ×1.0         │ Sparse, quiet connective tissue            │
            //  │ 1: Monumental        │  0.12   ×0.7         │ Big pyramids, monumental arches, imposing  │
            //  │ 2: Colonnade         │  0.18   ×1.5         │ Dense columns, doorway arcades, no pyramid │
            //  │ 3: Antenna path      │  0.15   ×1.2         │ Antenna corridor, colossal sentinels       │
            //  │ 4: Barren            │  0.15   ×0.3         │ Near-empty, occasional obelisk             │
            //  └──────────────────────┴──────────────────────┴────────────────────────────────────────────┘

            static constexpr float THEME_LATTICE_SPACING = 500.0f;
            static constexpr uint32_t THEME_SEED_BAND = 170u;
            static constexpr uint32_t THEME_COUNT = 5;
            static constexpr float THEME_BASE_WEIGHT = 10.0f;

            // ── Theme Envelope ──────────────────────────────────────────────
            //
            // Single active theme at a time. When selected, its weight spikes
            // and decays over a patch count. Cooldown prevents immediate
            // repetition after expiry.

            struct ThemeEnvelope {
                int32_t  active = -1;              // theme index, or -1 (no bias)
                uint32_t elapsed = 0;               // patches since this theme fired
                uint32_t cooldowns[THEME_COUNT]{};  // per-theme remaining cooldown
            };

            struct PopulationTheme {
                float spawn_weight[PopFamily::COUNT];          // multiplier on base spawn chance per family
                float tier_wt_pyramid[3];                      // multiplier on pyramid tier base weights
                float tier_wt_arch[3];                         // multiplier on arch tier base weights
                float tier_wt_column[3];                       // multiplier on column tier base weights (Pillar, Doric, Ornate)
                float tier_wt_antenna[3];                      // multiplier on antenna tier base weights (Antenna, Squat, Colossal)
                float tier_wt_palm[3];                         // multiplier on palm tier base weights (Sapling, Coastal, Royal)
                float tier_wt_cactus[3];                       // multiplier on cactus tier base weights (Finger, Saguaro, Candelabra)
                float tier_wt_blade[3];                        // multiplier on blade tier base weights (Sprout, Clump, Thicket)
                float tier_wt_sphere[2];                       // multiplier on sphere tier base weights (Sentinel, Anomaly)
                float tier_wt_ribbon[3];                       // multiplier on ribbon tier base weights (Serpentine, Helix, Streamer)
                float tier_wt_cube[4];                         // multiplier on cube tier base weights (SmCube..Monolith)
                float density_mult;                            // multiplier on entity_density

                // Envelope parameters (replace lattice weight for theme selection)
                float    spike;
                uint32_t sustain;       // patches at full spike
                uint32_t decay;         // patches for linear decay to base
                uint32_t cooldown;      // patches before re-eligible after expiry

                // Lattice node weight (spatial distribution of themes)
                float weight;
            };

            //  ┌──────────────────────────────────────────────────────────────────────────────┐
            //  │ THEME PROFILES — Envelope-selected                                            │
            //  ├──────────────────┬────────┬────────┬────────┬─────────┬─────────────────────────┤
            //  │ Theme            │ Pyr sp │ Arch sp│ Col sp │ Density │ Envelope                │
            //  ├──────────────────┼────────┼────────┼────────┼─────────┼─────────────────────────┤
            //  │ 0 Transition     │  0.4   │  0.3   │  0.7   │  ×1.0   │ 150/20/3/0             │
            //  │ 1 Monumental     │  1.5   │  1.0   │  1.0   │  ×1.0   │ 150/10/10/8            │
            //  │ 2 Colonnade      │  0.3   │  1.0   │  4.0   │  ×1.0   │ 150/15/6/6             │
            //  │ 3 Antenna        │  0.5   │  0.5   │  4.0   │  ×1.0   │ 180/10/5/5             │
            //  │ 4 Barren         │  0.4   │  0.3   │  0.5   │  ×1.0   │ 100/12/3/4             │
            //  └──────────────────┴────────┴────────┴────────┴─────────┴─────────────────────────┘

            static constexpr PopulationTheme THEMES[THEME_COUNT] = {
                // ── 0: TRANSITION — sparse connective tissue ─────────────────
                {   { 0.4f, 0.3f, 0.7f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 1.0f, 0.5f, 0.5f, 0.5f },   // spawn_weight [pyr..sph, ribn, cube, gol, gall]
                    { 1.0f, 1.0f, 1.0f },                                       // tier_pyr
                    { 1.0f, 0.3f, 1.0f },                                       // tier_arch
                    { 0.1f, 0.2f, 0.3f },                                       // tier_col
                    { 0.1f, 2.0f, 0.7f },                                       // tier_ant
                    { 1.0f, 1.0f, 1.0f },                                       // tier_palm
                    { 1.0f, 1.0f, 1.0f },                                       // tier_cactus
                    { 1.0f, 1.0f, 1.0f },                                       // tier_blade
                    { 1.0f, 1.0f },                                              // tier_sphere (neutral)
                    { 1.0f, 1.0f, 1.0f },                                       // tier_ribbon (neutral)
                    { 1.0f, 1.0f, 1.0f, 1.0f },                                 // tier_cube (neutral)
                    1.0f,                                                         // density
                    150.0f, 20u, 3u, 0u,                                          // spike, sustain, decay, cooldown
                    0.21f                                                         // weight
                },
                // ── 1: MONUMENTAL — big pyramids, varied arches, heavy columns
                {   { 1.5f, 1.0f, 1.0f, 0.5f, 0.2f, 0.2f, 0.5f, 0.3f, 1.0f, 0.3f, 0.3f, 0.3f },
                    { 0.2f, 0.5f, 3.0f },
                    { 2.0f, 0.1f, 3.0f },
                    { 0.01f, 0.01f, 1.0f },
                    { 0.5f, 1.5f, 0.5f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    1.0f,
                    150.0f, 10u, 10u, 8u,
                    0.30f
                },
                // ── 2: COLONNADE — dense columns, moderate arches ────────────
                {   { 0.3f, 1.0f, 4.0f, 0.5f, 0.3f, 0.3f, 0.5f, 0.3f, 1.0f, 0.3f, 0.5f, 0.4f },
                    { 1.0f, 1.0f, 1.0f },
                    { 3.0f, 0.5f, 1.0f },
                    { 0.3f, 3.0f, 5.0f },
                    { 0.2f, 0.1f, 0.1f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    1.0f,
                    150.0f, 15u, 6u, 6u,
                    0.31f
                },
                // ── 3: ANTENNA — antenna-dominant corridor ───────────────────
                {   { 0.5f, 0.5f, 1.0f, 4.0f, 0.5f, 0.5f, 0.5f, 0.3f, 1.0f, 0.3f, 0.3f, 0.3f },
                    { 1.0f, 0.05f, 2.0f },
                    { 1.0f, 0.2f, 0.8f },
                    { 0.1f, 0.3f, 0.3f },
                    { 0.5f, 3.5f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    1.0f,
                    180.0f, 10u, 5u, 5u,
                    0.18f
                },
                // ── 4: BARREN — near-empty ───────────────────────────────────
                {   { 0.4f, 0.3f, 0.5f, 0.3f, 0.2f, 0.2f, 0.1f, 0.3f, 1.0f, 0.1f, 0.2f, 0.6f },
                    { 2.0f, 0.5f, 0.2f },
                    { 1.0f, 1.0f, 1.0f },
                    { 0.2f, 0.5f, 0.5f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f },
                    { 1.0f, 1.0f, 1.0f, 1.0f },
                    1.0f,
                    100.0f, 12u, 3u, 4u,
                    0.04f
                },
            };

            // Select a theme at a lattice node from cumulative weights
            static uint32_t select_theme_at_node(uint32_t node_seed) {
                float roll = cpu_hash_f(node_seed, 370u);
                float cumul = 0.0f;
                float total = 0.0f;
                for (uint32_t t = 0; t < THEME_COUNT; t++) total += THEMES[t].weight;
                for (uint32_t t = 0; t < THEME_COUNT; t++) {
                    cumul += THEMES[t].weight / total;
                    if (roll < cumul) return t;
                }
                return THEME_COUNT - 1;
            }

            // ── Theme Envelope — sequential theme selection ──────────────────
            //
            // Replaces the lattice-based theme blend for spawn decisions.
            // One theme is active at a time. Its weight spikes and decays
            // over a patch count. Cooldown prevents immediate repetition.

            static float theme_envelope_weight(const PopulationTheme& theme, uint32_t elapsed) {
                if (elapsed < theme.sustain) return theme.spike;
                if (elapsed < theme.sustain + theme.decay) {
                    float t = (float)(elapsed - theme.sustain) / (float)theme.decay;
                    return theme.spike + (THEME_BASE_WEIGHT - theme.spike) * t;
                }
                return THEME_BASE_WEIGHT;
            }

            // Called ONCE per patch, inside the spawn loop, BEFORE per-family gates.
            // Returns the theme index to use for this patch.
            uint32_t evaluate_theme_envelope(uint32_t tile_seed_value) {
                auto& env = themeEnvelope_;

                // Build effective weights
                float weights[THEME_COUNT];
                float total = 0.0f;
                for (uint32_t i = 0; i < THEME_COUNT; i++) {
                    if (env.cooldowns[i] > 0) {
                        weights[i] = 0.0f;
                    }
                    else if ((int32_t)i == env.active) {
                        weights[i] = theme_envelope_weight(THEMES[i], env.elapsed);
                    }
                    else {
                        weights[i] = THEME_BASE_WEIGHT;
                    }
                    total += weights[i];
                }
                if (total < 0.001f) total = 1.0f;

                // Roll from weights
                float roll = cpu_hash_f(tile_seed_value, 370u);
                uint32_t selected = THEME_COUNT - 1;
                float cumul = 0.0f;
                for (uint32_t i = 0; i < THEME_COUNT; i++) {
                    cumul += weights[i] / total;
                    if (roll < cumul) { selected = i; break; }
                }

                // State transitions
                if ((int32_t)selected != env.active) {
                    if (env.active >= 0) {
                        env.cooldowns[env.active] = THEMES[env.active].cooldown;
                    }
                    env.active = (int32_t)selected;
                    env.elapsed = 0;

                    // Census dump on theme transition
#ifdef DIAG_ENTITY_CENSUS
                    dump_entity_census(theme_short_name(selected));
#endif
                }
                else {
                    env.elapsed++;
                }

                // Check expiry
                if (env.active >= 0) {
                    const auto& th = THEMES[env.active];
                    if (env.elapsed >= th.sustain + th.decay) {
                        env.cooldowns[env.active] = th.cooldown;
                        env.active = -1;
                        env.elapsed = 0;
                    }
                }

                // Tick cooldowns
                for (uint32_t i = 0; i < THEME_COUNT; i++) {
                    if (env.cooldowns[i] > 0) env.cooldowns[i]--;
                }

                return selected;
            }

            // ─── Terrain Tokens ──────────────────────────────────────────────
            //
            // Carried compositional priors that bias sequential tile generation.
            // Each token holds per-archetype weight multipliers and a generation
            // budget that decrements with each primary tile generation.
            // When budget reaches zero, the token is cleared.
            //
            // Tokens are READ inside generate_tile_state() (member access),
            // TICKED and EMITTED by tick_terrain_tokens() after each primary
            // tile generation. Neighbor padding calls do NOT tick.
            //
            // The mechanism:
            //   1. Patch generates → reads active tokens as priors on archetype weights
            //   2. Archetype outcome + its jitter properties → emission roll
            //   3. Emission may push a new token (bias type + budget drawn stochastically)
            //   4. All tokens decrement budget; dead tokens cleared
            //
            // The stack is small and fixed. If full, the oldest token (lowest budget)
            // is evicted to make room. In practice, ≤4 are alive at any time.

            static constexpr uint32_t MAX_TERRAIN_TOKENS = 8;

            struct TerrainToken {
                float archetype_bias[ARCHETYPE_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f };
                uint32_t budget = 0;
                bool active = false;
            };

            TerrainToken terrainTokens_[MAX_TERRAIN_TOKENS]{};

            // ── Emission Profiles ────────────────────────────────────────────
            //
            // Each archetype defines how it biases subsequent tile generation
            // when it emits a terrain token. The emission mechanism:
            //
            //   1. After a tile generates, it rolls emit_chance to decide
            //      whether it emits a token at all (0.0 = never, 1.0 = always).
            //   2. If emitting, it rolls pivot_chance: continuation vs pivot.
            //      Continuation carries the current terrain character forward.
            //      Pivot transitions to a different landform.
            //   3. Budget is drawn uniformly in [budget_min, budget_max]:
            //      how many primary tile generations the token survives.
            //   4. The bias vector multiplies into archetype selection weights
            //      for all tiles generated while the token is alive.
            //      Values >1.0 boost that archetype, <1.0 suppress it.
            //
            // Bias vector order: { mountainous, varied, basin, pool }
            //
            // Tuning these profiles IS the art direction for terrain composition.

            struct TerrainEmissionProfile {
                float emit_chance;                            // [0,1] probability of emitting any token
                uint32_t budget_min, budget_max;              // generation lifespan range
                float continuation_bias[ARCHETYPE_COUNT];     // archetype weight multipliers when continuing
                float pivot_chance;                           // [0,1] probability of pivoting vs continuing
                float pivot_bias[ARCHETYPE_COUNT];            // archetype weight multipliers when pivoting
            };

            //  ┌────────────────────┬────────┬─────────┬──────────────────────────────────────────┬────────┬──────────────────────────────────────────┐
            //  │                    │ emit%  │ budget  │ continuation bias                         │ pivot% │ pivot bias                               │
            //  │                    │        │ min max │ mount  varied basin  pool                 │        │ mount  varied basin  pool                 │
            //  ├────────────────────┼────────┼─────────┼──────────────────────────────────────────┼────────┼──────────────────────────────────────────┤
            //  │ 0: mountainous     │  0.45  │  2   5  │  2.0    1.5    0.3    0.0  (ridge runs)  │  0.25  │  0.3    2.0    1.5    0.0  (descend)     │
            //  │ 1: varied          │  0.25  │  1   3  │  0.8    1.5    0.8    0.2  (neutral)     │  0.30  │  1.5    0.5    1.5    0.1  (diversify)   │
            //  │ 2: basin           │  0.40  │  2   4  │  0.2    0.8    2.0    1.0  (flat runs)   │  0.20  │  0.5    1.5    0.5    0.3  (ascend)      │
            //  │ 3: pool            │  0.20  │  1   2  │  0.0    0.5    1.5    1.5  (hold flat)   │  0.35  │  0.3    1.0    2.0    0.2  (drain out)   │
            //  └────────────────────┴────────┴─────────┴──────────────────────────────────────────┴────────┴──────────────────────────────────────────┘
            static constexpr TerrainEmissionProfile TERRAIN_EMISSION[ARCHETYPE_COUNT] = {
                /* 0: mountainous */ { 0.45f,  2, 5,  { 2.0f, 1.5f, 0.3f, 0.0f },  0.25f, { 0.3f, 2.0f, 1.5f, 0.0f } },
                /* 1: varied      */ { 0.25f,  1, 3,  { 0.8f, 1.5f, 0.8f, 0.2f },  0.30f, { 1.5f, 0.5f, 1.5f, 0.1f } },
                /* 2: basin       */ { 0.40f,  2, 4,  { 0.2f, 0.8f, 2.0f, 1.0f },  0.20f, { 0.5f, 1.5f, 0.5f, 0.3f } },
                /* 3: pool        */ { 0.20f,  1, 2,  { 0.0f, 0.5f, 1.5f, 1.5f },  0.35f, { 0.3f, 1.0f, 2.0f, 0.2f } },
            };

            // ── Amplitude Momentum ───────────────────────────────────────────
            //
            // When amp_jitter rolls extreme, the token also carries amplitude
            // bias that nudges the next patch further in that direction.
            // Creates natural ridgelines and depth sequences.

            static constexpr float AMP_MOMENTUM_THRESHOLD = 0.15f;  // |jitter - 1.0| above this → emit amp momentum
            static constexpr float AMP_MOMENTUM_CARRY = 0.6f;       // fraction of excess carried forward

            // ─── Population Batch System ─────────────────────────────────────
            //
            // Entities spawn in observed batches. The first few entities of a
            // batch define the neighborhood's character; the rest follow it.
            //
            // Two derived biases update LIVE as observations accumulate:
            //   Type affinity — types that appeared more get boosted proportionally.
            //   Scale tendency — average tier scale biases select_tier toward similar.
            //
            // After POP_BATCH_SIZE patches, the batch resets: a few "exploratory"
            // patches with neutral priors, then the new batch character emerges.
            //
            // Each batch rolls a MODE at birth:
            //   Affinity  — more of the same (columns attract columns)
            //   Repulsion — opposites attract (columns push toward pyramids/arches)
            //   Neutral   — no bias (pure independent rolls, breathing room)
            //
            // No hand-crafted affinity matrices. The correlation IS the aesthetic.
            //
            //  ┌──────────────────────────────────────────────────────────────────────────────────────────┐
            //  │ POPULATION BATCH CONTROL SURFACE                                                        │
            //  ├─────────────────────────────────┬───────────┬────────────────────────────────────────────┤
            //  │ Constant                        │ Value     │ Effect                                     │
            //  ├─────────────────────────────────┼───────────┼────────────────────────────────────────────┤
            //  │ POP_BATCH_SIZE                  │  4        │ Patches per observation window              │
            //  │ POP_TYPE_AFFINITY_STRENGTH      │  3.0      │ Max spawn boost for dominant type           │
            //  │ POP_SCALE_TENDENCY_STRENGTH     │  2.0      │ Max tier proximity boost                    │
            //  │ POP_GOL_SUPPRESSION             │  0.05     │ GoL chance reduction per unit structure     │
            //  │ POP_MIN_OBSERVATIONS            │  2        │ Min entities before bias activates          │
            //  │ POP_MODE_AFFINITY_CHANCE        │  0.50     │ Probability of affinity batch               │
            //  │ POP_MODE_REPULSION_CHANCE       │  0.25     │ Probability of repulsion batch              │
            //  │ (remainder)                     │  0.25     │ Probability of neutral batch                │
            //  └─────────────────────────────────┴───────────┴────────────────────────────────────────────┘

            struct PopBatchMode {
                static constexpr uint32_t AFFINITY = 0;  // more of the same
                static constexpr uint32_t REPULSION = 1;  // opposites attract
                static constexpr uint32_t NEUTRAL = 2;  // pure independent rolls
            };

            static constexpr uint32_t POP_BATCH_SIZE = 16;
            static constexpr float POP_TYPE_AFFINITY_STRENGTH = 0.0f;
            static constexpr float POP_SCALE_TENDENCY_STRENGTH = 0.0f;
            static constexpr float POP_GOL_SUPPRESSION = 0.05f;
            static constexpr uint32_t POP_MIN_OBSERVATIONS = 1;
            static constexpr float POP_MODE_AFFINITY_CHANCE = 0.0f;
            static constexpr float POP_MODE_REPULSION_CHANCE = 0.0f;
            // remainder (1.0) = neutral

            // ── Cross-Family Affinity Matrix ──────────────────────────────────
            //
            // When family X is observed, how much does it influence family Y's
            // spawn chance? Read as: row = observed, column = target.
            // 1.0 = neutral. >1.0 = attracts. <1.0 = suppresses.
            //
            // In AFFINITY mode, values >1 boost the target.
            // In REPULSION mode, the matrix is read inverted (1/value).
            //
            //  ┌──────────────────────────────────────────────────────────────────────────┐
            //  │ CROSS-FAMILY AFFINITY              target →                              │
            //  │ observed ↓          │  Pyramid     │  Arch        │  Column              │
            //  ├──────────────────────┼──────────────┼──────────────┼──────────────────────┤
            //  │ Pyramid              │  0.5 (rare)  │  2.0 (gates) │  1.5 (colonnades)   │
            //  │ Arch                 │  0.8 (mild)  │  1.5 (chain) │  2.0 (flanking)     │
            //  │ Column               │  0.3 (supp)  │  1.2 (mild)  │  1.8 (cluster)      │
            //  └──────────────────────┴──────────────┴──────────────┴──────────────────────┘

            static constexpr float POP_CROSS_AFFINITY[PopFamily::COUNT][PopFamily::COUNT] = {
                //          target:  Pyramid  Arch    Column  Antenna  Palm    Cactus  Blade   Sphere  Ribn    Cube    GoL     Gall
                /* Pyramid */     {  0.5f,    2.0f,   1.5f,   1.5f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Arch    */     {  0.8f,    1.5f,   2.0f,   2.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Column  */     {  0.3f,    1.2f,   1.8f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Antenna */     {  0.3f,    1.2f,   1.0f,   1.8f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Palm    */     {  0.3f,    1.0f,   1.0f,   1.0f,   1.5f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Cactus  */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.5f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Blade   */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Sphere  */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Ribn    */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Cube    */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* GoL     */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
                /* Gall    */     {  1.0f,    1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f,   1.0f },
            };

            // ── Per-Tier Scale Character ──────────────────────────────────────
            //
            // Each tier declares its compositional "size character" on [0, 1].
            // 0.0 = human-scale intimate. 1.0 = monumental/colossal.
            // This replaces the linear tier_idx/(count-1) mapping.
            //
            // Used by record_population_observation to accumulate scale_sum,
            // and by select_tier_biased to compute proximity to the tendency.
            //
            //  ┌──────────────────────────────────────────────────────────┐
            //  │ TIER SCALE CHARACTER                                    │
            //  ├──────────────────────────┬─────────┬────────────────────┤
            //  │ Entity                   │ Scale   │ Character          │
            //  ├──────────────────────────┼─────────┼────────────────────┤
            //  │ Pyramid: Obelisk         │  0.35   │ tall but narrow    │
            //  │ Pyramid: Temple          │  0.60   │ moderate platform  │
            //  │ Pyramid: Colossus        │  1.00   │ massive landmark   │
            //  ├──────────────────────────┼─────────┼────────────────────┤
            //  │ Arch: Doorway            │  0.10   │ human passage      │
            //  │ Arch: Standard           │  0.55   │ medium gateway     │
            //  │ Arch: Monumental         │  0.95   │ cathedral-scale    │
            //  ├──────────────────────────┼─────────┼────────────────────┤
            //  │ Column: Pillar           │  0.15   │ squat post         │
            //  │ Column: Doric            │  0.30   │ classical human    │
            //  │ Column: Ornate           │  0.50   │ decorated medium   │
            //  │ Column: Antenna          │  0.60   │ tall with drums    │
            //  │ Column: Antenna Squat    │  0.45   │ wide + short drums │
            //  │ Column: Antenna Colossal │  0.85   │ tower-scale        │
            //  └──────────────────────────┴─────────┴────────────────────┘

            static constexpr float TIER_SCALE_PYRAMID[] = { 0.35f, 0.60f, 1.00f };
            static constexpr float TIER_SCALE_ARCH[] = { 0.10f, 0.55f, 0.95f };
            static constexpr float TIER_SCALE_COLUMN[] = { 0.15f, 0.30f, 0.50f };
            static constexpr float TIER_SCALE_ANTENNA[] = { 0.60f, 0.45f, 0.85f };
            static constexpr float TIER_SCALE_PALM[] = { 0.20f, 0.45f, 0.75f };
            static constexpr float TIER_SCALE_CACTUS[] = { 0.15f, 0.40f, 0.70f };
            static constexpr float TIER_SCALE_SPHERE[] = { 0.25f, 0.20f };
            static constexpr float TIER_SCALE_CUBE[] = { 0.15f, 0.35f, 0.60f, 0.40f };
            static constexpr float TIER_SCALE_RIBBON[] = { 0.80f, 0.50f, 0.65f };
            // GoL compound tiers: Conway 0–6, then Pulse 7–9
            static constexpr float TIER_SCALE_GOL[] = {
                0.70f, 0.30f, 0.45f, 0.55f, 0.35f, 0.80f, 0.65f,   // Conway: Pillars..Glacier
                0.40f, 0.25f, 0.50f                                   // Pulse: Breathe, Sparkle, Drift
            };
            // Gallery tiers = archetypes: 0=mountainous, 1=varied, 2=basin, 3=pool
            static constexpr float TIER_SCALE_GALLERY[] = { 0.20f, 0.40f, 0.65f, 0.75f };

            // Accessor: look up scale character by family + tier index.
            static float tier_scale_character(uint32_t family, uint32_t tier_idx) {
                switch (family) {
                case PopFamily::PYRAMID: return (tier_idx < 3) ? TIER_SCALE_PYRAMID[tier_idx] : 0.5f;
                case PopFamily::ARCH:    return (tier_idx < 3) ? TIER_SCALE_ARCH[tier_idx] : 0.5f;
                case PopFamily::COLUMN:  return (tier_idx < 3) ? TIER_SCALE_COLUMN[tier_idx] : 0.5f;
                case PopFamily::ANTENNA: return (tier_idx < 3) ? TIER_SCALE_ANTENNA[tier_idx] : 0.5f;
                case PopFamily::PALM:    return (tier_idx < 3) ? TIER_SCALE_PALM[tier_idx] : 0.5f;
                case PopFamily::CACTUS:   return (tier_idx < 3) ? TIER_SCALE_CACTUS[tier_idx] : 0.5f;
                case PopFamily::SPHERE:   return (tier_idx < 2) ? TIER_SCALE_SPHERE[tier_idx] : 0.5f;
                case PopFamily::RIBBON:   return (tier_idx < 3) ? TIER_SCALE_RIBBON[tier_idx] : 0.5f;
                case PopFamily::CUBE:     return (tier_idx < 4) ? TIER_SCALE_CUBE[tier_idx] : 0.5f;
                case PopFamily::GOL:      return (tier_idx < 10) ? TIER_SCALE_GOL[tier_idx] : 0.5f;
                case PopFamily::GALLERY:  return (tier_idx < 4) ? TIER_SCALE_GALLERY[tier_idx] : 0.5f;
                default: return 0.5f;
                }
            }

            struct PopulationBatch {
                uint32_t type_count[PopFamily::COUNT] = {};  // entities per family
                float scale_sum = 0.0f;        // sum of normalized tier positions [0,1]
                uint32_t scale_n = 0;          // number of scale observations
                uint32_t patches_elapsed = 0;  // patches since batch start
                uint32_t mode = PopBatchMode::AFFINITY;  // rolled at batch birth
            };

            PopulationBatch popBatch_{};
            uint32_t popBatchCounter_ = 0;  // global counter for deterministic mode rolls

            // ── Recording ─────────────────────────────────────────────────────
            //
            // Called inside each spawn function after successful spawn.
            // tier_idx: which tier was selected (0-based).
            // tier_count: total tiers in that family (for normalization).

            void record_population_observation(uint32_t family, uint32_t tier_idx) {
                popBatch_.type_count[family]++;
                float scale = tier_scale_character(family, tier_idx);
                popBatch_.scale_sum += scale;
                popBatch_.scale_n++;
            }

            // ── Batch Advance ─────────────────────────────────────────────────
            //
            // Called once per patch after all entity spawns complete.
            // Increments patch counter; resets batch when budget expires.
            // New batch rolls its mode from (seed, batchCounter).

            void advance_population_batch() {
                popBatch_.patches_elapsed++;
                if (popBatch_.patches_elapsed >= POP_BATCH_SIZE) {
                    popBatch_ = PopulationBatch{};
                    // Roll batch mode deterministically
                    popBatchCounter_++;
                    uint32_t mode_seed = cpu_hash(activeSeed_ ^ popBatchCounter_, 330u);
                    float mode_roll = cpu_hash_f(mode_seed, 331u);
                    if (mode_roll < POP_MODE_AFFINITY_CHANCE) {
                        popBatch_.mode = PopBatchMode::AFFINITY;
                    }
                    else if (mode_roll < POP_MODE_AFFINITY_CHANCE + POP_MODE_REPULSION_CHANCE) {
                        popBatch_.mode = PopBatchMode::REPULSION;
                    }
                    else {
                        popBatch_.mode = PopBatchMode::NEUTRAL;
                    }
                }
            }

            // ── Live Accessors (read current batch state) ─────────────────────
            //
            // Called inside spawn functions and GoL detection.
            // Bias builds as observations accumulate within the batch.
            // Before POP_MIN_OBSERVATIONS, returns neutral (1.0 / 0.0 / 0.5).
            //
            // In AFFINITY mode: types that appeared more get boosted.
            // In REPULSION mode: types that appeared LESS get boosted.
            // In NEUTRAL mode: always returns 1.0 (no bias).

            float population_type_affinity(uint32_t family) const {
                if (popBatch_.mode == PopBatchMode::NEUTRAL) return 1.0f;
                uint32_t total = popBatch_.type_count[0] + popBatch_.type_count[1] + popBatch_.type_count[2];
                if (total < POP_MIN_OBSERVATIONS) return 1.0f;
                // Weighted sum: each observed family contributes its cross-affinity to the target
                float influence = 0.0f;
                for (uint32_t obs = 0; obs < PopFamily::COUNT; obs++) {
                    float fraction = (float)popBatch_.type_count[obs] / (float)total;
                    float affinity = POP_CROSS_AFFINITY[obs][family];
                    if (popBatch_.mode == PopBatchMode::REPULSION) {
                        affinity = (affinity > 0.01f) ? (1.0f / affinity) : 10.0f;  // invert
                    }
                    influence += fraction * affinity;
                }
                return 1.0f + (influence - 1.0f) * POP_TYPE_AFFINITY_STRENGTH;
            }

            float population_scale_tendency() const {
                if (popBatch_.mode == PopBatchMode::NEUTRAL) return 0.5f;
                if (popBatch_.scale_n < POP_MIN_OBSERVATIONS) return 0.5f;
                float raw = popBatch_.scale_sum / (float)popBatch_.scale_n;
                if (popBatch_.mode == PopBatchMode::REPULSION) {
                    raw = 1.0f - raw;  // invert: small observations push toward large
                }
                return raw;
            }

            float population_automata_bias() const {
                if (popBatch_.mode == PopBatchMode::NEUTRAL) return 0.0f;
                uint32_t total = popBatch_.type_count[0] + popBatch_.type_count[1] + popBatch_.type_count[2];
                if (total < POP_MIN_OBSERVATIONS) return 0.0f;
                float avg_affinity = (population_type_affinity(0) +
                    population_type_affinity(1) +
                    population_type_affinity(2)) / 3.0f;
                return -(avg_affinity - 1.0f) * POP_GOL_SUPPRESSION;
            }

            // ── Biased Tier Selection ─────────────────────────────────────────
            //
            // Applies scale tendency to tier weights before rolling.
            // Tiers near the batch's average scale get boosted (affinity)
            // or tiers FAR from it get boosted (repulsion).
            // Falls back to unbiased select_tier in neutral mode or pre-observations.
            //
            // family: PopFamily index — needed to look up tier scale character.

            uint32_t select_tier_biased(uint32_t seed, uint32_t tier_prop,
                const float* base_weights, uint32_t count, uint32_t family) const {
                if (popBatch_.mode == PopBatchMode::NEUTRAL ||
                    popBatch_.scale_n < POP_MIN_OBSERVATIONS) {
                    return select_tier(seed, tier_prop, base_weights, count);
                }
                float tendency = population_scale_tendency();
                float weights[8];  // max tiers across all families
                float total = 0.0f;
                for (uint32_t t = 0; t < count && t < 8; t++) {
                    float scale = tier_scale_character(family, t);
                    float proximity = 1.0f - std::abs(scale - tendency);
                    weights[t] = base_weights[t] * (1.0f + proximity * POP_SCALE_TENDENCY_STRENGTH);
                    total += weights[t];
                }
                for (uint32_t t = 0; t < count; t++) weights[t] /= total;
                float roll = cpu_hash_f(seed, tier_prop);
                float cumul = 0.0f;
                for (uint32_t t = 0; t < count; t++) {
                    cumul += weights[t];
                    if (roll < cumul) return t;
                }
                return count - 1;
            }

            // ── Theme Envelope State (replaces lattice-based selection) ─────
            ThemeEnvelope themeEnvelope_{};
            uint32_t active_theme_idx_ = 0;   // set per-patch by evaluate_theme_envelope

            // ── Minimum Separation Matrix ─────────────────────────────────────
            //
            // Compositional spacing: how far apart entities of each family pair
            // must be. Checked against the footprint registry before accepting
            // any position. Footprints persist until patch eviction, so this
            // gives full spatial coverage across the entire active world.
            //
            // 0.0 = no minimum (exception — allow intimate proximity).
            // Positive = minimum world-space center-to-center distance.
            //
            // Read as: row = entity being placed, column = existing entity.
            // The check is asymmetric: placing an arch near a pyramid may have a
            // different minimum than placing a pyramid near an arch.
            //
            //  ┌──────────────────────────────────────────────────────────────────────────────┐
            //  │ MINIMUM SEPARATION — edge-to-edge gap (wu)                                    │
            //  │ placing ↓           │  Pyramid      │  Arch         │  Column                │
            //  ├──────────────────────┼───────────────┼───────────────┼────────────────────────┤
            //  │ Pyramid              │  15 (sparse)  │  10 (wide)    │   5 (spacing)          │
            //  │ Arch                 │  10 (wide)    │  20 (corridor)│  10 (spacing)          │
            //  │ Column               │   5 (spacing) │  10 (spacing) │   8 (colonnade)        │
            //  └──────────────────────┴───────────────┴───────────────┴────────────────────────┘
            //
            // Key exception: Arch→Pyramid = 0. Doorway arches (which become portals)
            // are explicitly allowed on top of pyramids. The footprint system still
            // prevents physical overlap of collision geometry — this matrix only
            // governs aesthetic spacing.

            static constexpr float MIN_SEPARATION[PopFamily::COUNT][PopFamily::COUNT] = {
                //                near:  Pyr    Arch   Col    Ant    Palm   Cact   Blad   Sph    Ribn   Cube   GoL    Gall
                /* placing Pyramid  */ { 15.0f, 10.0f,  5.0f,  5.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Arch     */ { 10.0f, 20.0f, 10.0f, 10.0f,  8.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Column   */ {  5.0f, 10.0f,  8.0f,  6.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Antenna  */ {  5.0f, 10.0f,  6.0f, 12.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Palm     */ {  5.0f,  8.0f,  5.0f,  5.0f,  8.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Cactus   */ {  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  8.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Blade    */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Sphere   */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 20.0f,  0.0f,  0.0f,  0.0f,  0.0f },
                /* placing Ribbon   */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 40.0f,  0.0f,  0.0f,  0.0f },
                /* placing Cube     */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 15.0f,  0.0f,  0.0f },
                /* placing GoL      */ { 10.0f, 10.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 60.0f,  0.0f },
                /* placing Gallery  */ { 10.0f, 10.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 10.0f, 30.0f },
            };

            // --- Tile State (what we remember about each generated tile) ----------

            struct TileState {
                uint32_t archetype = 1;      // default: varied
                float height_bias = 0.0f;
                float amp_scale = 1.0f;
                float activation_scale = 1.0f;
                float amp_momentum = 0.0f;   // signed amplitude excess, carried by terrain tokens
                float entity_density = 1.0f; // spatial density multiplier for entity spawning
                // Theme: evaluated from theme lattice at tile generation time
                float theme_spawn[PopFamily::COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }; // blended per-family spawn multiplier
                uint32_t theme_idx = 0;      // dominant theme index (for tier bias)
            };

            // Spatial cache: keyed by (grid_x, grid_z)
            struct GridKey {
                int32_t x, z;
                bool operator==(const GridKey& o) const { return x == o.x && z == o.z; }
            };
            struct GridKeyHash {
                size_t operator()(const GridKey& k) const {
                    return (size_t)k.x * 73856093u ^ (size_t)k.z * 19349663u;
                }
            };

            std::unordered_map<GridKey, TileState, GridKeyHash> tileCache_;

            // Forgetting radius: tiles beyond this many grid cells get evicted
            static constexpr int32_t FORGET_RADIUS = (int32_t)PREGEN_RADIUS + 2;  // eviction radius (beyond pre-gen)

            void evict_distant_tiles(int32_t centerX, int32_t centerZ) {
                auto it = tileCache_.begin();
                while (it != tileCache_.end()) {
                    int32_t dx = it->first.x - centerX;
                    int32_t dz = it->first.z - centerZ;
                    if (dx < -FORGET_RADIUS || dx > FORGET_RADIUS ||
                        dz < -FORGET_RADIUS || dz > FORGET_RADIUS) {
                        it = tileCache_.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }

            // Build and upload GPUTileGrid from tile cache, centered on (cx, cz).
            void upload_tile_grid_now(wgpu::Queue& queue, int32_t cx, int32_t cz) {
                static constexpr int32_t TILE_PAD = 1;
                int32_t rp = (int32_t)activeRadius_ + TILE_PAD;
                uint32_t tileGridSide = 2 * (activeRadius_ + TILE_PAD) + 1;
                GPUTileGrid grid{};
                grid.origin_x = cx - rp;
                grid.origin_z = cz - rp;
                grid.side = tileGridSide;
                grid.cell_extent = PATCH_EXTENT;

                for (int32_t gz = cz - rp; gz <= cz + rp; gz++) {
                    for (int32_t gx = cx - rp; gx <= cx + rp; gx++) {
                        int32_t lx = gx - grid.origin_x;
                        int32_t lz = gz - grid.origin_z;
                        uint32_t idx = lz * tileGridSide + lx;
                        auto it = tileCache_.find({ gx, gz });
                        if (it != tileCache_.end()) {
                            grid.entries[idx].amp_scale = it->second.amp_scale;
                            grid.entries[idx].height_bias = it->second.height_bias;
                            grid.entries[idx].activation_scale = it->second.activation_scale;
                            grid.entries[idx].archetype = it->second.archetype;
                        }
                        else {
                            grid.entries[idx].amp_scale = 1.0f;
                            grid.entries[idx].height_bias = 0.0f;
                            grid.entries[idx].activation_scale = 1.0f;
                            grid.entries[idx].archetype = 1;
                        }
                    }
                }
                gpuState_.upload_tile_grid(queue, grid);
            }

            // --- Archetype Generation Rule ------------------------------------------
            //
            // Consult cached neighbors → weight archetypes → deterministic roll.
            // All thresholds and multipliers live in ArchetypeSelectionRules.
            // All per-archetype parameters live in the ARCHETYPES matrix.

            TileState generate_tile_state(int32_t gx, int32_t gz) {
                // Count neighbor archetypes
                uint32_t neighbor_counts[ARCHETYPE_COUNT] = {};
                uint32_t total_neighbors = 0;

                for (int32_t dz = -1; dz <= 1; dz++) {
                    for (int32_t dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dz == 0) continue;
                        auto it = tileCache_.find({ gx + dx, gz + dz });
                        if (it != tileCache_.end()) {
                            neighbor_counts[it->second.archetype]++;
                            total_neighbors++;
                        }
                    }
                }

                // Build selection weights from archetype base weights + neighbor influence
                float weights[ARCHETYPE_COUNT];
                for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
                    weights[a] = ARCHETYPES[a].base_weight;
                }

                // Pool archetype: mood-aware injection.
                // Indoor: common (flat floors are natural).
                // Outdoor: very rare (special feature).
                static constexpr uint32_t POOL_IDX = 3;
                if (MOOD_TABLE[activeMood_].indoor) {
                    weights[POOL_IDX] = 1.5f;   // ~30% of indoor tiles become pools
                }
                else {
                    weights[POOL_IDX] = 0.05f;  // ~1.5% of outdoor tiles
                }

                // ── Terrain token priors: multiply active tokens into weights ──
                for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
                    if (!terrainTokens_[t].active) continue;
                    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
                        weights[a] *= terrainTokens_[t].archetype_bias[a];
                    }
                }

                if (total_neighbors > 0) {
                    using R = ArchetypeSelectionRules;
                    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
                        if (neighbor_counts[a] >= R::DOMINANT_THRESHOLD) {
                            weights[a] *= R::DOMINANT_MULTIPLIER;
                        }
                        else if (neighbor_counts[a] >= R::COMMON_THRESHOLD) {
                            weights[a] *= R::COMMON_MULTIPLIER;
                        }
                        else if (neighbor_counts[a] >= R::PRESENT_THRESHOLD) {
                            weights[a] *= R::PRESENT_MULTIPLIER;
                        }
                    }
                }

                // Normalize and roll
                float total_weight = 0.0f;
                for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) total_weight += weights[a];
                for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) weights[a] /= total_weight;

                uint32_t seed = tile_seed(activeSeed_, gx, gz);
                float roll = cpu_hash_f(seed, 300u);

                uint32_t archetype = ARCHETYPE_COUNT - 1;
                float cumulative = 0.0f;
                for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
                    cumulative += weights[a];
                    if (roll < cumulative) { archetype = a; break; }
                }

                // Per-tile jitter from archetype profile
                const auto& profile = ARCHETYPES[archetype];
                float amp_jitter = 1.0f + (cpu_hash_f(seed, 301u) - 0.5f) * profile.amp_jitter_range;
                float bias_jitter = (cpu_hash_f(seed, 302u) - 0.5f) * profile.bias_jitter_range;

                TileState ts;
                ts.archetype = archetype;
                ts.amp_scale = profile.amp_scale * amp_jitter;
                ts.height_bias = profile.height_bias + bias_jitter;
                ts.activation_scale = profile.activation_scale;
                ts.amp_momentum = amp_jitter - 1.0f;  // signed: positive = amplified, negative = dampened

                // ── Entity density field (coarse spatial noise) ──────────
                {
                    float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
                    float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
                    float dlx = patch_cx / DENSITY_LATTICE_SPACING;
                    float dlz = patch_cz / DENSITY_LATTICE_SPACING;
                    int32_t dbx = (int32_t)std::floor(dlx);
                    int32_t dbz = (int32_t)std::floor(dlz);
                    float dfx = dlx - dbx, dfz = dlz - dbz;
                    float dwx = dfx * dfx * (3.0f - 2.0f * dfx);
                    float dwz = dfz * dfz * (3.0f - 2.0f * dfz);
                    float density = 0.0f;
                    for (int dz = 0; dz <= 1; dz++) for (int dx = 0; dx <= 1; dx++) {
                        uint32_t ns = cpu_lattice_node_seed(activeSeed_, dbx + dx, dbz + dz, DENSITY_SEED_BAND);
                        float raw = cpu_hash_f(ns, 350u);
                        float shaped = std::pow(raw, DENSITY_EXPONENT);
                        float w = ((dx == 1) ? dwx : (1.0f - dwx)) * ((dz == 1) ? dwz : (1.0f - dwz));
                        density += shaped * w;
                    }
                    ts.entity_density = DENSITY_MIN + density * (DENSITY_MAX - DENSITY_MIN);
                }

                // ── Theme field (coarse compositional character) ─────────
                {
                    float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
                    float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
                    float tlx = patch_cx / THEME_LATTICE_SPACING;
                    float tlz = patch_cz / THEME_LATTICE_SPACING;
                    int32_t tbx = (int32_t)std::floor(tlx);
                    int32_t tbz = (int32_t)std::floor(tlz);
                    float tfx = tlx - tbx, tfz = tlz - tbz;
                    float twx = tfx * tfx * (3.0f - 2.0f * tfx);
                    float twz = tfz * tfz * (3.0f - 2.0f * tfz);

                    // Blend spawn weights across 4 lattice nodes.
                    // Track dominant node for discrete tier bias lookup.
                    float blended_spawn[PopFamily::COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
                    float blended_density = 0.0f;
                    float best_w = -1.0f;
                    uint32_t dominant_theme = 0;

                    for (int dz = 0; dz <= 1; dz++) for (int dx = 0; dx <= 1; dx++) {
                        uint32_t ns = cpu_lattice_node_seed(activeSeed_, tbx + dx, tbz + dz, THEME_SEED_BAND);
                        uint32_t tidx = select_theme_at_node(ns);
                        const auto& theme = THEMES[tidx];
                        float w = ((dx == 1) ? twx : (1.0f - twx)) * ((dz == 1) ? twz : (1.0f - twz));
                        for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                            blended_spawn[f] += theme.spawn_weight[f] * w;
                        }
                        blended_density += theme.density_mult * w;
                        if (w > best_w) { best_w = w; dominant_theme = tidx; }
                    }

                    for (uint32_t f = 0; f < PopFamily::COUNT; f++)
                        ts.theme_spawn[f] = blended_spawn[f];
                    ts.theme_idx = dominant_theme;
                    ts.entity_density *= blended_density;  // theme density stacks with spatial density
                }

                return ts;
            }

            // ─── Terrain Token Tick + Emission ───────────────────────────────
            //
            // Called ONCE per primary tile generation, NEVER for neighbor padding.
            // Decrements all active token budgets, clears expired tokens,
            // then evaluates the tile outcome for emission of a new token.

            void tick_terrain_tokens(const TileState& outcome, uint32_t seed) {
                // ── Tick existing tokens ─────────────────────────────────
                for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
                    if (!terrainTokens_[t].active) continue;
                    if (terrainTokens_[t].budget <= 1) {
                        terrainTokens_[t].active = false;
                    }
                    else {
                        terrainTokens_[t].budget--;
                    }
                }

                // ── Emission from outcome ────────────────────────────────
                const auto& ep = TERRAIN_EMISSION[outcome.archetype];

                // Roll: does this outcome emit a token?
                // Property index 310: decorrelated from archetype roll (300-302)
                float emit_roll = cpu_hash_f(seed, 310u);
                if (emit_roll >= ep.emit_chance) return;

                // Roll: continuation or pivot?
                float pivot_roll = cpu_hash_f(seed, 311u);
                bool pivot = (pivot_roll < ep.pivot_chance);

                // Budget draw (uniform in [budget_min, budget_max])
                float budget_t = cpu_hash_f(seed, 312u);
                uint32_t budget = ep.budget_min +
                    (uint32_t)(budget_t * (float)(ep.budget_max - ep.budget_min + 1));
                budget = std::min(budget, ep.budget_max);  // clamp rounding

                // Build the token
                TerrainToken token{};
                const float* bias = pivot ? ep.pivot_bias : ep.continuation_bias;
                for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
                    token.archetype_bias[a] = bias[a];
                }

                // Amplitude momentum: if this patch rolled extreme, carry it
                if (std::abs(outcome.amp_momentum) > AMP_MOMENTUM_THRESHOLD) {
                    float carry = outcome.amp_momentum * AMP_MOMENTUM_CARRY;
                    if (carry > 0.0f) {
                        token.archetype_bias[0] *= (1.0f + carry);  // mountainous
                    }
                    else {
                        token.archetype_bias[2] *= (1.0f - carry);  // basin (carry is negative)
                    }
                }

                token.budget = budget;
                token.active = true;

                // ── Insert into stack ────────────────────────────────────
                // Find a free slot. If none, evict the token with lowest budget.
                uint32_t slot = MAX_TERRAIN_TOKENS;
                uint32_t min_budget = UINT32_MAX;
                uint32_t min_slot = 0;

                for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
                    if (!terrainTokens_[t].active) { slot = t; break; }
                    if (terrainTokens_[t].budget < min_budget) {
                        min_budget = terrainTokens_[t].budget;
                        min_slot = t;
                    }
                }
                if (slot == MAX_TERRAIN_TOKENS) slot = min_slot;  // evict oldest

                terrainTokens_[slot] = token;
            }

            // --- World teardown: reset all runtime state for world transition ---

            void teardown_world(wgpu::Queue& queue) {
                // Patches + tile cache
                init_patch_system();
                lastCenterX_ = INT32_MAX;  // force full regen on next frame
                lastCenterZ_ = INT32_MAX;

                // Terrain tokens
                for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
                    terrainTokens_[t] = TerrainToken{};
                }

                // Population batch
                popBatch_ = PopulationBatch{};
                popBatchCounter_ = 0;
                entityQueue_.clear();
                placementResults_.clear();

                // Theme envelope
                themeEnvelope_ = ThemeEnvelope{};
                active_theme_idx_ = 0;

                // Clear all entity piers (keep test rig at slots 0-2)
                for (uint32_t i = Dim::PIER_ARCH_BASE; i < Dim::PIER_TOTAL; i++) {
                    clear_pier(queue, i);
                }

                // Arches
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
                    activeArches_[i] = ActiveArch{};
                }
                activeArchCount_ = 0;
                portalsDirty_ = true;
                gpuState_.set_arch_index_count(0);
                // Clear all arch mesh gen param slots
                {
                    GPUArchMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
                        gpuState_.upload_arch_mesh_params_slot(queue, i, emptyParams);
                    }
                    archMeshGenPending_ = true;
                }

                // Columns + Antennas
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
                    activeColumns_[i] = ActiveColumn{};
                }
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
                    activeAntennas_[i] = ActiveColumn{};
                }
                activeColumnCount_ = 0;
                activeAntennaCount_ = 0;
                gpuState_.set_column_index_count(0);
                // Clear all column mesh gen param slots
                {
                    GPUColumnMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_COLUMN_INSTANCES; i++) {
                        gpuState_.upload_column_mesh_params_slot(queue, i, emptyParams);
                    }
                    columnMeshGenPending_ = true;
                }

                // Palms
                for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
                    activePalms_[i] = ActivePalm{};
                }
                activePalmCount_ = 0;
                gpuState_.set_palm_index_count(0);
                {
                    GPUPalmMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
                        gpuState_.upload_palm_mesh_params_slot(queue, i, emptyParams);
                    }
                    palmMeshGenPending_ = true;
                }

                // Cacti
                for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
                    activeCacti_[i] = ActiveCactus{};
                }
                activeCactusCount_ = 0;
                gpuState_.set_cactus_index_count(0);
                {
                    GPUCactusMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
                        gpuState_.upload_cactus_mesh_params_slot(queue, i, emptyParams);
                    }
                    cactusMeshGenPending_ = true;
                }

                // Blade clusters
                for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
                    activeBlades_[i] = ActiveBlade{};
                }
                activeBladeCount_ = 0;
                gpuState_.set_blade_index_count(0);
                {
                    GPUBladeClusterMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
                        gpuState_.upload_blade_mesh_params_slot(queue, i, emptyParams);
                    }
                    bladeMeshGenPending_ = true;
                }

                // Pyramids
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
                    activePyramids_[i] = ActivePyramid{};
                }
                activePyramidCount_ = 0;
                cpuPyramids_ = GPUPyramidArray{};
                gpuState_.upload_pyramids(queue, cpuPyramids_);
                gpuState_.set_pyramid_index_count(0);
                // Clear all mesh gen param slots (inactive → degenerates on next dispatch)
                {
                    GPUPyramidMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
                        gpuState_.upload_pyramid_mesh_params_slot(queue, i, emptyParams);
                    }
                    pyramidMeshGenPending_ = true;
                }

                // GoL zones
                for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
                    golZones_[i] = GoLZoneState{};
                }
                golZoneCount_ = 0;
                activeZoneSlotCount_ = 0;
                pendingDeriveRequests_.count = 0;
                GPUGoLZoneArray emptyZones{};
                gpuState_.upload_zone_config(queue, emptyZones);

                // Ribbon — clear all slots
                {
                    for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                        activeRibbons_[i] = ActiveRibbon{};
                        ribbonStates_[i] = GPURibbonState{};
                    }
                    activeRibbonCount_ = 0;
                    renderedRibbonSlot_ = UINT32_MAX;
                    GPURibbonState empty{};
                    gpuState_.upload_ribbon(queue, empty);
                }

                // Sphere entities — clear all slots (0..MAX_SPHERE_INSTANCES-1)
                for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
                    activeFloaters_[i] = ActiveFloater{};
                    GPUFloatingEntityState empty{};
                    gpuState_.upload_sphere_entity_slot(queue, i, empty);
                }
                activeFloaterCount_ = 0;

                // Cube entities — clear all slots (0..MAX_CUBE_INSTANCES-1)
                for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
                    activeCubes_[i] = ActiveCube{};
                    GPUFloatingEntityState empty{};
                    gpuState_.upload_cube_entity_slot(queue, i, empty);
                }
                activeCubeCount_ = 0;

                // Gallery / paintings — clear all exhibition + slots, keep staging intact
                for (uint32_t i = 0; i < MAX_GALLERIES; i++) {
                    galleryCenters_[i] = GalleryCenter{};
                }
                pendingSnapshot_.active = false;
                pendingPromotionCount_ = 0;
                wallFrameCount_ = 0;
                activePaintingCount_ = 0;
                // Clear all painting slots (CPU + GPU)
                for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
                    paintingSlots_[i] = GPUPaintingSlot{};
                }
                {
                    GPUPaintingSlot empty[Dim::PAINTING_MAX_SLOTS]{};
                    gpuState_.upload_painting_slots(queue, empty, Dim::PAINTING_MAX_SLOTS);
                }
                // Free all exhibition layers (staging persists across worlds)
                for (uint32_t i = 0; i < Dim::EXHIBITION_LAYERS; i++) exhibitionOccupied_[i] = false;
                exhibitionCount_ = 0;
                // Snapshot staging: consumed flags persist — exhibited snapshots stay consumed.
                // Only new captures (photographer overwrites) make a slot fresh again.
                // Authored staging: rotate consumed slots with fresh images from disk.
                rotate_authored_staging(queue);
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) authoredStaging_[i].consumed = false;

                // Footprints
                for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
                    footprints_[i] = GroundFootprint{};
                }

                // Aura
                auraNeedsClear_ = true;
                auraCfgDirty_ = true;

                // Sky orbs: apply_mood re-enables + re-seeds as needed
                teardown_orbs();

                // Indoor shell
                gpuState_.set_shell_index_count(0);

                // Lights need re-upload with potentially new config
                lightsDirty_ = true;

                // Band motion: reset (apply_mood will re-initialize if needed)
                bandMotionActive_ = false;
                for (int i = 0; i < 6; i++) {
                    bandBlend_[i] = -1.0f;
                    bandBlendTarget_[i] = 0.0f;
                    bandPhaseOrigin_[i] = 0.0f;
                }

                // Musical modes: reset intensities (mask stays — circuits remain wired)
                for (uint32_t m = 0; m < MMODE_COUNT; m++) mmodeIntensity_[m] = 0.0f;
                paletteDriftTarget_ = 0.0f;
                paletteDriftDesired_ = 0.0f;
                gpuState_.set_mode_color_shift(0.0f);
                gpuState_.set_mode_checker_scatter(0.0f);
                gpuState_.set_mode_palette_drift(0.0f, 0.0f, 0.0f);
                gpuState_.set_mode_gol_scales(1.0f, 1.0f);
                for (int i = 0; i < 32; i++) pulseRing_[i] = 0.0f;
                pulseWriteIdx_ = 0;
                prevPolyphony_ = 0.0f;
                float zero_pulses[32] = {};
                gpuState_.set_pulse_data(0, zero_pulses);

                // Y correction

                // New world decides its own upload frequency policy
                gpuState_.set_config_dynamic(false);
            }

            void init_patch_system() {
                for (uint32_t i = 0; i < MAX_PATCHES; i++) {
                    freeLayerStack_[i] = MAX_PATCHES - 1 - i;
                }
                freeLayerCount_ = MAX_PATCHES;
                activePatchCount_ = 0;
                renderPatchCount_ = 0;
                lod0PatchCount_ = 0;
                allPatchCount_ = 0;
                gpuState_.config().placement_patch_count = 0;
                tileCache_.clear();
                pierCountDirty_ = true;
                groundEntriesDirty_ = true;
                patchInstancesDirty_ = true;
                placementDirty_ = true;
            }

            // Test rig piers: ramp + plateau + block at pier slots 0-2.
            // Same geometry as the old test rig solids, now as GPUPierInstance.
            void setup_test_rig_piers(wgpu::Queue queue) {
                // Ramp: height 0→3 along +X.
                GPUPierInstance ramp{};
                ramp.origin[0] = 12.0f;  ramp.origin[1] = 0.0f;
                ramp.half_size[0] = 6.5f; ramp.half_size[1] = 3.0f;
                ramp.height_near = 0.0f;  ramp.height_far = 3.0f;
                ramp.rotation = 0.0f;
                ramp.edge_blend = 0.5f;
                ramp.tier = PierTier::TEST_RIG;
                ramp.is_active = 1;
                write_pier(queue, 0, ramp);

                // Plateau: flat at height 3, overlaps ramp at x=18.
                GPUPierInstance plat{};
                plat.origin[0] = 21.0f;  plat.origin[1] = 0.0f;
                plat.half_size[0] = 3.5f; plat.half_size[1] = 3.0f;
                plat.height_near = 3.0f;  plat.height_far = 3.0f;
                plat.rotation = 0.0f;
                plat.edge_blend = 0.5f;
                plat.tier = PierTier::TEST_RIG;
                plat.is_active = 1;
                write_pier(queue, 1, plat);

                // Block: sharp edges → step-height walls (impassable).
                GPUPierInstance block{};
                block.origin[0] = 21.0f;  block.origin[1] = 0.0f;
                block.half_size[0] = 1.2f; block.half_size[1] = 1.2f;
                block.height_near = 5.0f;  block.height_far = 5.0f;
                block.rotation = 0.0f;
                block.edge_blend = 0.0f;
                block.tier = PierTier::TEST_RIG;
                block.is_active = 1;
                write_pier(queue, 2, block);
            }

            // Batch-generate patches into the caller's command encoder.
            // Two-pass heightfield: pass 1 evaluates ground_formed_with_complexity
            // (POLICY_BAKED_HEIGHTFIELD contributor set, fused with the complexity
            // byproduct) per texel, pass 2 reads neighbors for gradients +
            // evaluates complexity.
            // Compute pass boundary between them provides the storage texture barrier.
            // stagingOffset: slot index into the staging buffer, so multiple
            // batches per frame don't overwrite each other's params.
            void generate_patch_batch(wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
                const GPUPatchParams* params, uint32_t count,
                uint32_t stagingOffset = 0) {
                if (count == 0) return;

                // One WriteBuffer: all params into staging at the given offset
                gpuState_.upload_patch_staging(queue, params, count, stagingOffset);

                for (uint32_t i = 0; i < count; i++) {
                    // Copy this patch's params from staging slot → active params buffer
                    encoder.CopyBufferToBuffer(
                        gpuState_.patch_staging_buffer(), (stagingOffset + i) * sizeof(GPUPatchParams),
                        gpuState_.patch_params_buffer(), 0,
                        sizeof(GPUPatchParams));

                    // Pass 1: heights only (one ground_formed_with_complexity per texel)
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Patch Heights (pass 1)";
                        wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_generate_patch_heights(cp, gpuState_.patch_gen_group(), GPUState::patch_heightfield_workgroups());
                        cp.End();
                    }

                    // Pass boundary: storage texture write → read barrier

                    // Pass 2: gradients from neighbor reads + complexity + cell colors
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Patch Gradients + Cells (pass 2)";
                        wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_generate_patch_gradients(cp, gpuState_.patch_gen_group(), GPUState::patch_heightfield_workgroups());
                        renderer_.dispatch_generate_patch_cells(cp, gpuState_.patch_gen_group(), GPUState::patch_cell_workgroups());
                        cp.End();
                    }
                }
            }

            GPUPatchParams make_patch_params(int32_t gx, int32_t gz, uint32_t layer) const {
                GPUPatchParams p{};
                p.origin[0] = (gx + 0.5f) * PATCH_EXTENT;
                p.origin[1] = (gz + 0.5f) * PATCH_EXTENT;
                p.extent = PATCH_EXTENT;
                p.resolution = 256;
                p.master_seed = activeSeed_;
                p.time = 0.0f;
                p.layer = layer;
                p._pad1 = 0.0f;
                return p;
            }

            uint32_t alloc_layer() {
                if (freeLayerCount_ == 0) {
                    // Safety: no free layers — recycle layer 0 rather than crash.
                    // This shouldn't happen if eviction works correctly.
                    return 0;
                }
                return freeLayerStack_[--freeLayerCount_];
            }

            void free_layer(uint32_t layer) {
                freeLayerStack_[freeLayerCount_++] = layer;
            }

            // Check if grid coordinate is within the allocation window (activeRadius_ = PREGEN_RADIUS)
            bool in_render_window(int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
                int32_t r = (int32_t)activeRadius_;
                return gx >= cx - r && gx <= cx + r &&
                    gz >= cz - r && gz <= cz + r;
            }

            // Check if grid coordinate is within the VISIBLE circle (Euclidean).
            // Radius 5.5 in grid units: inscribes cleanly within the PREGEN square,
            // drops ~24 corner patches that would be deep in fog anyway.
            // Pre-gen patches outside this circle are allocated and generated but NOT rendered.
            static constexpr float VISIBLE_RADIUS = 5.5f;
            static constexpr float VISIBLE_RADIUS_SQ = VISIBLE_RADIUS * VISIBLE_RADIUS;

            // Multi-LOD distance bands (grid units, Euclidean from center).
            // LOD-0 (full 64×64 mesh): patches within LOD_FULL_RADIUS
            // LOD-1 (half 32×32 mesh): patches between LOD_FULL_RADIUS and VISIBLE_RADIUS
            static constexpr float LOD_FULL_RADIUS = 3.5f;
            static constexpr float LOD_FULL_RADIUS_SQ = LOD_FULL_RADIUS * LOD_FULL_RADIUS;

            // ─── Visibility Cylinder ─────────────────────────────────────
            //
            // World-space cylinder centered on the pawn's actual position.
            // Patches enter the draw list when their nearest edge crosses
            // inside the cylinder — one at a time as the pawn moves,
            // not in batches when a grid boundary is crossed.
            //
            // Grid-based allocation/eviction is unchanged; only the
            // draw-list gate uses world-space distance.
            static constexpr float VISIBILITY_CYLINDER_RADIUS = VISIBLE_RADIUS * PATCH_EXTENT;
            static constexpr float VISIBILITY_CYLINDER_RADIUS_SQ = VISIBILITY_CYLINDER_RADIUS * VISIBILITY_CYLINDER_RADIUS;
            static constexpr float LOD0_CYLINDER_RADIUS = LOD_FULL_RADIUS * PATCH_EXTENT;
            static constexpr float LOD0_CYLINDER_RADIUS_SQ = LOD0_CYLINDER_RADIUS * LOD0_CYLINDER_RADIUS;

            // Distance² from point (px,pz) to nearest edge of a patch AABB.
            // Zero when the point is inside the patch.
            static float patch_distance_sq(float px, float pz,
                float origin_x, float origin_z, float half) {
                float dx = std::max(0.0f, std::abs(px - origin_x) - half);
                float dz = std::max(0.0f, std::abs(pz - origin_z) - half);
                return dx * dx + dz * dz;
            }

            // ── Distance-sorted patch scan helper ──────────────────────────

            struct PatchCandidate {
                uint32_t idx;
                float dist2;
            };

            template<typename Pred>
            uint32_t collect_sorted_patches(
                PatchCandidate* out,
                float pawn_wx, float pawn_wz,
                Pred&& pred,
                bool nearest_first) const
            {
                float half = PATCH_EXTENT * 0.5f;
                uint32_t count = 0;
                for (uint32_t i = 0; i < activePatchCount_; i++) {
                    if (!patches_[i].valid) continue;
                    if (!pred(patches_[i])) continue;
                    float ox = (patches_[i].grid_x + 0.5f) * PATCH_EXTENT;
                    float oz = (patches_[i].grid_z + 0.5f) * PATCH_EXTENT;
                    float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);
                    out[count++] = { i, d2 };
                }
                for (uint32_t i = 1; i < count; i++) {
                    PatchCandidate key = out[i];
                    uint32_t j = i;
                    if (nearest_first) {
                        while (j > 0 && out[j - 1].dist2 > key.dist2) {
                            out[j] = out[j - 1]; j--;
                        }
                    }
                    else {
                        while (j > 0 && out[j - 1].dist2 < key.dist2) {
                            out[j] = out[j - 1]; j--;
                        }
                    }
                    out[j] = key;
                }
                return count;
            }

            // Check if grid coordinate is within the priority window (GRID_RADIUS)
            bool in_priority_window(int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
                int32_t r = (int32_t)GRID_RADIUS;
                return gx >= cx - r && gx <= cx + r &&
                    gz >= cz - r && gz <= cz + r;
            }

            // Process entity spawn for pre-collected patch candidates.
            void spawn_selected_patches(
                const PatchCandidate* candidates, uint32_t count,
                wgpu::Queue& queue)
            {
                for (uint32_t s = 0; s < count; s++) {
                    uint32_t pi = candidates[s].idx;
                    active_theme_idx_ = evaluate_theme_envelope(
                        tile_seed(activeSeed_, patches_[pi].grid_x, patches_[pi].grid_z));
                    select_entities_for_patch(patches_[pi].grid_x, patches_[pi].grid_z);
                    advance_population_batch();
                    patches_[pi].phase = PatchPhase::SPAWNED;
                }
                place_entity_queue();
                commit_entity_queue(queue);

                // Late registration: check if newly-spawned patches contain
                // an active ribbon's unregistered tip. This closes the gap
                // when a ribbon's far tip is beyond the streaming window at
                // spawn time but comes into range as the pawn approaches.
                for (uint32_t s = 0; s < count; s++) {
                    uint32_t pi = candidates[s].idx;
                    int32_t gx = patches_[pi].grid_x;
                    int32_t gz = patches_[pi].grid_z;
                    for (uint32_t r = 0; r < MAX_RIBBON_INSTANCES; r++) {
                        auto& ar = activeRibbons_[r];
                        if (!ar.active) continue;
                        // Check near tip
                        if (!ar.near_tip_registered &&
                            ar.near_tip_gx == gx && ar.near_tip_gz == gz) {
                            patches_[pi].record_entity(PopFamily::RIBBON, r);
                            ar.near_tip_registered = true;
                            ar.ref_count++;
                        }
                        // Check far tip
                        if (!ar.far_tip_registered &&
                            ar.far_tip_gx == gx && ar.far_tip_gz == gz) {
                            patches_[pi].record_entity(PopFamily::RIBBON, r);
                            ar.far_tip_registered = true;
                            ar.ref_count++;
                        }
                    }
                }
            }

            // Hook: fires once when a patch transitions SPAWNED → GENERATED.
            void on_patch_first_generated(uint32_t pi, wgpu::Queue& queue) {
                // Galleries → entity pipeline (select_gallery_for_patch)
                // GoL zones → entity pipeline (select_gol_for_patch)
                (void)pi; (void)queue;
            }

            // Process heightfield generation for pre-collected patch candidates.
            void generate_selected_patches(
                const PatchCandidate* candidates, uint32_t count,
                wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
                uint32_t& patchStagingOffset, bool& tileGridDirty)
            {
                if (count == 0) return;
                if (tileGridDirty) {
                    upload_tile_grid_now(queue, lastCenterX_, lastCenterZ_);
                    tileGridDirty = false;
                }
                GPUPatchParams batchParams[MAX_PATCHES];
                uint32_t batchIdx[MAX_PATCHES];
                for (uint32_t i = 0; i < count; i++) {
                    uint32_t pi = candidates[i].idx;
                    batchParams[i] = make_patch_params(
                        patches_[pi].grid_x, patches_[pi].grid_z, patches_[pi].layer);
                    batchIdx[i] = pi;
                }
                generate_patch_batch(encoder, queue, batchParams, count, patchStagingOffset);
                patchStagingOffset += count;
                for (uint32_t b = 0; b < count; b++) {
                    uint32_t pi = batchIdx[b];
                    bool first_gen = (patches_[pi].phase == PatchPhase::SPAWNED);
                    patches_[pi].phase = PatchPhase::GENERATED;
                    if (first_gen) {
                        on_patch_first_generated(pi, queue);
                    }
                }
                patchInstancesDirty_ = true;
            }

        public:
            Cartridge() = default;

            Cartridge(const Cartridge&) = delete;
            Cartridge& operator=(const Cartridge&) = delete;

            void initialize(wgpu::Device device) override {
                device_ = device;
                auto tGpu0 = std::chrono::high_resolution_clock::now();
                gpuState_.init(device);
                auto tGpu1 = std::chrono::high_resolution_clock::now();
                std::cout << "[Cartridge] GPUState init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tGpu1 - tGpu0).count()
                    << " ms\n";
            }

            bool init_renderer(
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                auto t0 = std::chrono::high_resolution_clock::now();
                if (!renderer_.init(
                    device_,
                    gpuState_,
                    colorFormat,
                    depthFormat
                )) return false;

                // Create offscreen textures with the actual swapchain format
                if (!gpuState_.initOffscreenResources(colorFormat)) {
                    std::cerr << "[Cartridge] Failed to init offscreen resources\n";
                    return false;
                }

                auto t1 = std::chrono::high_resolution_clock::now();

                // --- One-shot: generate terrain index buffer on GPU -----------------
                {
                    wgpu::CommandEncoder encoder = device_.CreateCommandEncoder();
                    wgpu::ComputePassDescriptor desc{};
                    desc.label = "Terrain Index Gen (one-shot)";
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&desc);
                    renderer_.dispatch_generate_terrain_indices(
                        pass,
                        gpuState_.terrain_index_gen_group(),
                        GPUState::terrain_mesh_workgroups()
                    );
                    pass.End();
                    wgpu::CommandBuffer cmd = encoder.Finish();
                    device_.GetQueue().Submit(1, &cmd);
                }
                auto t2 = std::chrono::high_resolution_clock::now();

                init_patch_system();
                setup_test_rig_piers(device_.GetQueue());

                // Sky orbs for the initial mood (apply_mood runs only on transitions).
                {
                    wgpu::Queue q = device_.GetQueue();
                    configure_orbs(ORB_MOOD_TABLE[activeMood_], q);
                }

                // Agent registries — single source of truth in modules/agents.inl
                // (AGENT_BEHAVIORS / AGENT_TIER_GAINS), uploaded once to GPU
                // storage buffers at bindings 110 + 111. Values are
                // constexpr-equivalent and never change during a session,
                // so this is a one-shot write at boot.
                {
                    wgpu::Queue q = device_.GetQueue();
                    upload_agent_registries_to_gpu(q);
                }

                // Initial agent population for boot mood. Slot 0 (player) is
                // already live on the GPU via GPUState's init; this populates
                // slots 1..MAX_AGENTS-1 from AGENT_POPULATIONS[activeMood_].
                // Mirror the player's idle pose into cpuAgents_[0] first so
                // the full-buffer upload is idempotent.
                {
                    cpuAgents_[0].pos_x = Idle::PAWN_POS_X;
                    cpuAgents_[0].pos_y = Idle::PAWN_POS_Y;
                    cpuAgents_[0].pos_z = Idle::PAWN_POS_Z;
                    cpuAgents_[0].heading = Idle::PAWN_HEADING;
                    cpuAgents_[0].orient_w = 1.0f;
                    cpuAgents_[0].is_active = 1u;
                    cpuAgents_[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
                    cpuAgents_[0].tier_idx = AGENT_TIER_WORKER;
                    cpuAgents_[0].portal_trigger = -1;

                    wgpu::Queue q = device_.GetQueue();
                    spawn_population_for_mood(activeMood_, activeSeed_,
                        Idle::PAWN_POS_X, Idle::PAWN_POS_Z, q);
                    dump_agent_census("boot");
                }

                // Eager-load authored paintings at boot (avoids mid-frame stall on first gallery)
                {
                    wgpu::Queue q = device_.GetQueue();
                    load_authored_textures(q);
                }

                auto t3 = std::chrono::high_resolution_clock::now();

                std::cout << "[Cartridge] Renderer init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
                std::cout << "[Cartridge] Terrain gen:      "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
                std::cout << "[Cartridge] Patch system:     "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << " ms\n";
                std::cout << "[Cartridge] Total init:       "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t0).count() << " ms\n";

                return true;
            }

            // SEAM[spine:K1] update() currently mixes orchestration (correct)
            //   with module-specific ramps and couplings (leaked).
            //   Resolution: introduce CPU-side Trajectory primitive (mirroring
            //   WGSL §1.2) + per-module tick_*_couplings() functions. After
            //   resolution, update() becomes a sequence of named tick calls,
            //   shrinking by ~250 lines. Phase 4 of the resolution sequence.
            // SEAM[spine:L4] formal phase-table comment block at top of update()
            //   would make ordering grep-discoverable. Today the ordering is
            //   only in section banners and per-site comments.
            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
                // --- Build GPU signal from analysis + input -------------------------
                GPUFrameSignal gpuSignal;

                gpuSignal.t_seconds = signal.t_seconds;
                gpuSignal.t_beats = signal.t_beats;
                gpuSignal.dt = signal.dt;
                gpuSignal.aspect_ratio = aspect_ratio;

                for (size_t i = 0; i < signal.stats.size(); ++i) {
                    gpuSignal.stats[i] = signal.stats[i];
                }

                gpuSignal.move_x = inputState_.move_x;
                gpuSignal.move_z = inputState_.move_z;
                gpuSignal.look_az_delta = inputState_.look_az_delta;
                gpuSignal.look_el_delta = inputState_.look_el_delta;
                gpuSignal.zoom_delta = inputState_.zoom_delta;
                gpuSignal.pan_x_delta = inputState_.pan_x_delta;
                gpuSignal.pan_y_delta = inputState_.pan_y_delta;
                gpuSignal._pad1 = 0.0f;

                currentBeats_ = signal.t_beats;
                currentSeconds_ = signal.t_seconds;
                currentDt_ = signal.dt;

                // --- Upload to GPU --------------------------------------------------

                // Aura presence trajectory: smooth ramp on enable/disable
                //
                // SEAM[pawn:K1] presence ramp lives in spine; should live in
                //   pawn.inl when extracted. Mirrors the WGSL §1.2 Trajectory
                //   abstraction (world.wgsl line 178). End-of-tour Phase 4:
                //   auraPresence_ becomes a Trajectory field in pawn.inl;
                //   this block becomes a tick_pawn_couplings(signal, dt) call.
                {
                    float target = auraEnabled_ ? 1.0f : 0.0f;
                    float rate = (target > auraPresence_) ? AURA_PRESENCE_ATTACK : AURA_PRESENCE_RELEASE;
                    float prev = auraPresence_;
                    auraPresence_ = prev + (target - prev) * (1.0f - std::exp(-rate * currentDt_));
                    if (auraPresence_ < 0.001f && target == 0.0f) auraPresence_ = 0.0f;
                    if (auraPresence_ > 0.999f && target == 1.0f) auraPresence_ = 1.0f;
                    if (auraPresence_ != prev) auraCfgDirty_ = true;
                }

                // Pawn aura height: presence × base height × expansion
                // This same value is used by terrain VS for extrusion, so pawn and terrain always agree.
                float aura_expand_mult = 1.0f + mmodeIntensity_[MMODE_AURA_EXPAND] * 3.0f;
                float effective_aura_height = auraHeightEnabled_
                    ? activeAuraProfile_.height_scale * auraPresence_ * aura_expand_mult : 0.0f;
                gpuState_.set_pawn_aura_height(effective_aura_height);
                gpuState_.set_aura_enabled(auraPresence_ > 0.001f);  // keep compute running while ramping down
                gpuState_.set_world_seed(activeSeed_);
                if (finiteMode_) {
                    float bmin = -(float)finiteRadius_ * PATCH_EXTENT;
                    float bmax = ((float)finiteRadius_ + 1.0f) * PATCH_EXTENT;
                    gpuState_.set_world_bounds(bmin, bmin, bmax, bmax);
                }
                else {
                    gpuState_.set_world_bounds(0.0f, 0.0f, 0.0f, 0.0f);
                }

                // --- Transition state machine ---
                if (transitionPhase_ != TransitionPhase::IDLE) {
                    transitionTimer_ += signal.dt;
                    switch (transitionPhase_) {
                    case TransitionPhase::FADE_OUT:
                        transitionFadeAlpha_ = std::min(1.0f, transitionTimer_ / transitionFadeDuration_);
                        if (transitionFadeAlpha_ >= 1.0f) {
                            transitionPhase_ = TransitionPhase::TEARDOWN;
                        }
                        break;
                    case TransitionPhase::TEARDOWN:
                    {
                        // SEAM[mood:K3] this 70-line TEARDOWN block does
                        //   per-mood-transition work that overlaps with
                        //   mood.inl::apply_mood. The K3 leak (per-transition
                        //   musical reset) lives in apply_mood; this block is
                        //   apply_mood's caller-side parallel. End-of-tour:
                        //   reset_musical_couplings() lives in musical.inl;
                        //   apply_mood calls it; this block stays focused on
                        //   the integration concerns (worldGen bump, agent
                        //   reset, ribbon cleanup) it correctly owns.
                        // SEAM[spine:P5] worldGen_++ at top of TEARDOWN is the
                        //   stale-callback guard (P5 family). Genuinely
                        //   spine-owned.

                        // Bump the world generation counter so any in-flight
                        // pawn readback callback from the previous world
                        // drops its data instead of overwriting pawnReadback
                        // with a stale position. See worldGen_ declaration.
                        worldGen_++;

                        // Capture return seed + mood + radius before overwrite
                        backPortalReturnSeed_ = activeSeed_;
                        backPortalReturnMood_ = activeMood_;
                        backPortalReturnRadius_ = finiteRadius_;

                        activeSeed_ = pendingDestination_.seed;
                        finiteMode_ = pendingDestination_.finite;
                        finiteRadius_ = pendingDestination_.finite_radius;
                        teardown_world(queue);
                        // NOTE: do NOT force pawnReadbackState_ to IDLE here.
                        // If a MapAsync is in-flight (MAPPING), forcing IDLE would
                        // cause CopyBufferToBuffer to a still-mapped buffer.
                        // The existing state machine guards will skip readback
                        // until the pending callback resolves naturally.
                        readbackPortalTrigger_ = -1;
                        pawnReadback_x_ = 0.0f;
                        pawnReadback_z_ = 0.0f;
                        // Preserve the player's tier across mood transitions.
                        // Body identity (tier) is a property of the player, not
                        // the old mood — possessing a Scout and stepping through
                        // a portal should leave you as a Scout on the other side.
                        // Everything else about the body resets to idle defaults.
                        uint32_t preserved_tier = cpuAgents_[player_.possessed_slot].tier_idx;

                        gpuState_.reset_player_agent(queue, preserved_tier);
                        gpuState_.set_possessed_slot(0);
                        // Keep cpuAgents_ in sync with the GPU reset so
                        // patch streaming + ribbon + Caps Lock see current state.
                        std::memset(cpuAgents_, 0, sizeof(cpuAgents_));
                        cpuAgents_[0].pos_x = 0.0f;  // Idle::PAWN_POS_X
                        cpuAgents_[0].pos_y = 0.0f;
                        cpuAgents_[0].pos_z = 0.0f;
                        cpuAgents_[0].orient_w = 1.0f;
                        cpuAgents_[0].is_active = 1u;
                        cpuAgents_[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
                        cpuAgents_[0].tier_idx = preserved_tier;
                        cpuAgents_[0].portal_trigger = -1;
                        player_.possessed_slot = 0;
                        gpuState_.set_world_seed(activeSeed_);
                        apply_mood(pendingDestination_.mood, queue);
                        spawn_population_for_mood(pendingDestination_.mood, activeSeed_,
                            Idle::PAWN_POS_X, Idle::PAWN_POS_Z, queue);
                        dump_agent_census("mood-transition");
                        // Deactivate ribbons in finite mode (mood 5 spawns its own in apply_mood)
                        if (finiteMode_ && activeRibbonCount_ > 0 && activeMood_ != 5) {
                            for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                                activeRibbons_[i] = ActiveRibbon{};
                                ribbonStates_[i] = GPURibbonState{};
                            }
                            activeRibbonCount_ = 0;
                            renderedRibbonSlot_ = UINT32_MAX;
                            GPURibbonState empty{};
                            gpuState_.upload_ribbon(queue, empty);
                        }
                        // Schedule guaranteed back-portal in finite worlds
                        backPortalPending_ = finiteMode_;

                        transitionPhase_ = TransitionPhase::FADE_IN;
                        transitionTimer_ = 0.0f;
                        uint32_t side = finiteMode_ ? 2 * finiteRadius_ + 1 : 0;
                        std::cout << "[World] Teardown complete, seed=" << activeSeed_
                            << " mode=" << (finiteMode_ ? "finite" : "open")
                            << (finiteMode_ ? " " + std::to_string(side) + "x" + std::to_string(side) : "")
                            << "\n";
                    }
                    break;
                    case TransitionPhase::FADE_IN:
                        transitionFadeAlpha_ = std::max(0.0f, 1.0f - transitionTimer_ / transitionFadeDuration_);
                        if (transitionFadeAlpha_ <= 0.0f) {
                            transitionPhase_ = TransitionPhase::IDLE;
                            transitionFadeAlpha_ = 0.0f;
                        }
                        break;
                    default: break;
                    }
                }
                gpuState_.set_fade(transitionFadeAlpha_, 0.0f, 0.0f, 0.0f);

                gpuState_.upload_signal(queue, gpuSignal);

                // ─── Polyphony-driven band motion ────────────────────────
                //
                // SEAM[musical:K2] band motion ramps (~lines 8050-8087) are
                //   another ramp-in-spine site, not in the seam map's original
                //   inventory. NEW FINDING (Ch. 15 chunk 1): six bands ×
                //   per-frame exponential ramp, polyphony→band-blend coupling.
                //   Folds into the K2 resolution: tick_musical_couplings()
                //   handles bands alongside mode intensities.
                if (bandMotionActive_) {
                    float polyphony = signal.stats[0];
                    uint32_t active_count = (uint32_t)std::max(0.0f, std::min(polyphony, 6.0f));

                    // Set per-band targets: bands activate in order from fine to tectonic
                    for (uint32_t i = 0; i < 6; i++) bandBlendTarget_[i] = 0.0f;
                    for (uint32_t i = 0; i < active_count; i++) {
                        bandBlendTarget_[BAND_ACTIVATION_ORDER[i]] = 1.0f;
                    }

                    float dt = signal.dt;
                    bool changed = false;
                    for (uint32_t i = 0; i < 6; i++) {
                        float prev = bandBlend_[i];
                        float target = bandBlendTarget_[i];

                        // Capture phase origin at the moment a band activates
                        if (target > 0.5f && prev < 0.01f) {
                            bandPhaseOrigin_[i] = currentBeats_;
                        }

                        // Exponential ramp toward target
                        float rate = (target > prev) ? BAND_BLEND_ATTACK : BAND_BLEND_RELEASE;
                        bandBlend_[i] = prev + (target - prev) * (1.0f - std::exp(-rate * dt));

                        // Snap to endpoints to avoid perpetual drift
                        if (bandBlend_[i] < 0.001f && target == 0.0f) bandBlend_[i] = 0.0f;
                        if (bandBlend_[i] > 0.999f && target == 1.0f) bandBlend_[i] = 1.0f;

                        if (bandBlend_[i] != prev) changed = true;
                    }

                    if (changed) {
                        gpuState_.set_band_motion(bandBlend_, bandPhaseOrigin_);
                    }
                    gpuState_.set_terrain_time(currentBeats_);
                }

                // ─── Musical animation modes: per-frame intensity ramp ───
                //
                // SEAM[musical:K2] the polyphony-coupling ramps live here for
                //   ~150 lines (8089-8240): mode intensities loop, palette
                //   drift, color shift, checker scatter, aura expand mult,
                //   GoL tempo. Each is an exponential trajectory toward a
                //   polyphony-derived target. End-of-tour Phase 4 mirrors the
                //   GPU §1.2 Trajectory shape: each becomes a Trajectory
                //   field in musical.inl, this block becomes
                //   tick_musical_couplings(signal, dt, queue).
                {
                    float polyphony = signal.stats[0];
                    float dt = signal.dt;
                    bool any_changed = false;

                    for (uint32_t m = 0; m < MMODE_COUNT; m++) {
                        // Skip mode 0 (terrain waves) — handled by band motion system above
                        // Skip mode 3 (palette drift) — has its own steeper intensity curve below
                        if (m == MMODE_TERRAIN_WAVES || m == MMODE_PALETTE_DRIFT) continue;

                        bool on = is_mmode_on(m);
                        float target = on ? std::min(polyphony / 6.0f, 1.0f) : 0.0f;
                        float prev = mmodeIntensity_[m];
                        float rate = (target > prev) ? MMODE_ATTACK : MMODE_RELEASE;
                        float next = prev + (target - prev) * (1.0f - std::exp(-rate * dt));

                        // Snap to endpoints
                        if (next < 0.001f && target == 0.0f) next = 0.0f;
                        if (next > 0.999f && target >= 1.0f) next = 1.0f;

                        if (next != prev) {
                            mmodeIntensity_[m] = next;
                            any_changed = true;
                        }
                    }

                    if (any_changed) {
                        // Color shift: intensity → mode field bias
                        gpuState_.set_mode_color_shift(mmodeIntensity_[MMODE_COLOR_SHIFT] * 0.6f);

                        // Checker scatter: intensity → sparse threshold reduction
                        gpuState_.set_mode_checker_scatter(mmodeIntensity_[MMODE_CHECKER_SCATTER] * 0.5f);

                        // Aura expand: intensity scales aura parameters
                        if (mmodeIntensity_[MMODE_AURA_EXPAND] > 0.0f || is_mmode_on(MMODE_AURA_EXPAND)) {
                            auraCfgDirty_ = true;
                        }

                        // GoL tempo: intensity → tick slow-down + height boost
                        // Inverse: more polyphony = slower GoL (contemplation).
                        // When BPM detection arrives, this source gets swapped.
                        {
                            float gi = mmodeIntensity_[MMODE_GOL_TEMPO];
                            // tick_scale > 1 = slower. Lerp from 1.0 up to 4.0 (4× slower at full)
                            float tick_scale = 1.0f + gi * 3.0f;
                            // height_scale > 1 = taller. Lerp from 1.0 up to 3.0
                            float height_scale = 1.0f + gi * 2.0f;
                            gpuState_.set_mode_gol_scales(tick_scale, height_scale);
                        }
                    }

                    // Palette drift: smooth target transition + push to GPU
                    // Uses its own intensity curve — steeper than generic mmodeIntensity
                    // because palette colors are close and need strong push to read.
                    {
                        float poly = signal.stats[0];
                        bool drift_on = is_mmode_on(MMODE_PALETTE_DRIFT);

                        // Smooth palette mapping: ordered by contrast from sand baseline.
                        //   1 note → green(2)  — biggest hue shift
                        //   2 notes → grey(3)   — desaturated, clearly different
                        //   3+ notes → salmon(1) — warm shift, completes cycle
                        static constexpr float SMOOTH_PALETTE_MAP[] = { 0.0f, 2.0f, 3.0f, 1.0f };

                        // Discrete tier mapping: cycle through all vocabularies.
                        //   Idle   → whatever the threshold cascade gives (natural)
                        //   1 note → tinted mono(1)    — desaturated, grey-tinted cells
                        //   2 notes → chess colorful(4) — parity + vivid per-node colors
                        //   3 notes → pure B&W(2)       — high contrast random assignment
                        //   4+ notes → chess B&W(3)     — structured classic pattern
                        // Full color(0) is the natural idle state for most cells,
                        // so it's not a useful drift target — already there.
                        static constexpr float DISCRETE_TIER_MAP[] = { 0.0f, 1.0f, 4.0f, 2.0f, 3.0f };

                        if (drift_on && poly >= 1.0f) {
                            uint32_t idx = std::min((uint32_t)poly, 3u);
                            paletteDriftDesired_ = SMOOTH_PALETTE_MAP[idx];
                        }
                        if (!drift_on || poly < 0.5f) {
                            paletteDriftDesired_ = 0.0f;
                        }

                        // Ramp target smoothly to avoid color snaps
                        float prev_t = paletteDriftTarget_;
                        paletteDriftTarget_ += (paletteDriftDesired_ - paletteDriftTarget_)
                            * (1.0f - std::exp(-PALETTE_DRIFT_TARGET_RATE * dt));

                        // Intensity: poly/3 so single note is partial, 3 notes = full
                        float drift_intensity = drift_on
                            ? std::min(poly / 3.0f, 1.0f) : 0.0f;
                        // Use same exponential ramp as other modes for smooth on/off
                        float prev_i = mmodeIntensity_[MMODE_PALETTE_DRIFT];
                        float rate_i = (drift_intensity > prev_i) ? MMODE_ATTACK : MMODE_RELEASE;
                        mmodeIntensity_[MMODE_PALETTE_DRIFT] = prev_i
                            + (drift_intensity - prev_i) * (1.0f - std::exp(-rate_i * dt));
                        float intensity = mmodeIntensity_[MMODE_PALETTE_DRIFT];

                        // Discrete tier from lookup
                        float discrete_tier = 0.0f;
                        if (drift_on && poly >= 1.0f) {
                            uint32_t tidx = std::min((uint32_t)poly, 4u);
                            discrete_tier = DISCRETE_TIER_MAP[tidx];
                        }

                        if (intensity > 0.001f || paletteDriftTarget_ != prev_t) {
                            gpuState_.set_mode_palette_drift(paletteDriftTarget_, intensity, discrete_tier);
                        }
                    }
                }

                // ─── Radial pulse onset detection ────────────────────────
                //
                // SEAM[musical:K3] prevPolyphony_ is consumer state for onset
                //   detection (line ~8223). Correctly placed if musical.inl
                //   owns the per-frame update; stranded if K2 doesn't resolve.
                //   Migrates with K2.
                {
                    float poly = signal.stats[0];
                    bool pulse_on = is_mmode_on(MMODE_RADIAL_PULSE);

                    // Detect note onsets: polyphony increased since last frame
                    if (pulse_on && poly > prevPolyphony_ + 0.5f) {
                        float increase = poly - std::max(prevPolyphony_, 0.0f);
                        // Emit one pulse per onset, amplitude proportional to note count
                        uint32_t slot = pulseWriteIdx_ % PULSE_RING_SIZE;
                        uint32_t base = slot * 4;
                        pulseRing_[base + 0] = pawnReadback_x_;      // origin X
                        pulseRing_[base + 1] = pawnReadback_z_;      // origin Z
                        pulseRing_[base + 2] = currentSeconds_;      // onset time
                        pulseRing_[base + 3] = PULSE_AMPLITUDE * std::min(increase, 3.0f);
                        pulseWriteIdx_++;
                        std::cout << "[Pulse] ONSET slot=" << slot
                            << " pos=(" << pawnReadback_x_ << "," << pawnReadback_z_ << ")"
                            << " t=" << currentSeconds_
                            << " amp=" << pulseRing_[base + 3]
                            << " poly=" << poly << " prev=" << prevPolyphony_
                            << "\n";
                    }
                    prevPolyphony_ = poly;

                    // Count active (non-expired) pulses and upload
                    uint32_t active = 0;
                    for (uint32_t i = 0; i < PULSE_RING_SIZE; i++) {
                        float onset = pulseRing_[i * 4 + 2];
                        float amp = pulseRing_[i * 4 + 3];
                        if (amp > 0.001f && (currentSeconds_ - onset) < PULSE_MAX_AGE) {
                            active = std::max(active, i + 1);
                        }
                    }
                    // Always upload if any pulses exist (even decaying ones for GPU to evaluate)
                    gpuState_.set_pulse_data(active, pulseRing_);
                }

                gpuState_.upload_config(queue);

                // Orb musical coupling: polyphony → radial expansion.
                // Always-on when orbs are active; future coupling grammar
                // will put this behind a gate.
                //
                // SEAM[orbs:P1] counter-example to ramp-in-spine: the orb
                //   per-frame coupling is decomposed into orbs.inl
                //   (update_orb_coupling). This is the target shape for
                //   musical:K2, mood:K3, pawn:K1.
                update_orb_coupling(signal.stats[0], signal.dt, queue);

                // Orb dome anchor: follow pawn when toggled on. Uses
                // last-frame pawn readback — one-frame lag is imperceptible.
                update_orb_anchor(pawnReadback_x_, pawnReadback_z_, queue);

                // Pawn position comes from GPU readback (one-frame latency).
                // See render() for the readback state machine.

                // --- Clear deltas for next frame ------------------------------------
                update_photographer(queue);
                clear_input_deltas();
            }

            // ORDER (STREAMING PATCH MODE):
            //   1. (Optional) Compute: compute_ribbon_rings   [0D] -- ring transforms for flying ribbon
            //   2. Compute: update_world                      [0D] -- entities, trajectories, couplings
            //   3. Compute: compute_vp                        [0D] -- camera VP + sun VP (shadow)
            //   4. Render:  patch terrain instances           -- heightfield array sampled in VS
            //   5. Render:  pawn entity                       -- chess pawn
            //   6. Render:  sphere entity                     -- sphere
            //   7. Render:  ribbon rings                      -- instanced ring geometry

            // SEAM[spine:owns] render() is genuinely spine work: readback state
            //   machines, stale-callback guards, portal trigger handling,
            //   patch streaming, photographer cadence. The K1 observation
            //   doesn't apply to render() the same way it applies to update();
            //   render() mixes orchestration (correct) with smaller per-module
            //   GPU upload calls (each lives in its module already).
            void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {

                wgpu::Queue queue = device_.GetQueue();

                // --- GPU agent buffer readback (one-frame latency) ---
                // Copies the full agent_state array (MAX_AGENTS × 80 bytes)
                // to staging each frame after compute. CPU mirror is used
                // for patch streaming / ribbon / photographer (possessed
                // slot's XZ) and for Caps Lock nearest-agent targeting in
                // Step 7. Portal triggers surface from the possessed slot's
                // portal_trigger field.
                //
                // State machine: IDLE → copy agent buffer to staging → COPIED
                //                COPIED → call MapAsync → MAPPING
                //                MAPPING → callback fires, reads data → IDLE
                if (pawnReadbackState_ == PawnReadbackState::COPIED) {
                    pawnReadbackState_ = PawnReadbackState::MAPPING;
                    gpuState_.agent_state_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::agent_state_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [this, gen = worldGen_](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world: gen
                                // captured at issue time differs from current
                                // worldGen_ if a teardown happened in between.
                                // Buffer is still successfully mapped though, so
                                // we Unmap unconditionally (mapping contract is
                                // independent of whether we read the data).
                                if (gen == worldGen_) {
                                    const auto* data = static_cast<const GPUAgentState*>(
                                        gpuState_.agent_state_readback_staging().GetConstMappedRange(
                                            0, GPUState::agent_state_buffer_size()));
                                    if (data) {
                                        std::memcpy(cpuAgents_, data,
                                            GPUState::agent_state_buffer_size());
                                        const auto& p = cpuAgents_[player_.possessed_slot];
                                        pawnReadback_x_ = p.pos_x;
                                        pawnReadback_z_ = p.pos_z;
                                        readbackPortalTrigger_ = p.portal_trigger;
                                    }
                                }
                                gpuState_.agent_state_readback_staging().Unmap();
                            }
                            pawnReadbackState_ = PawnReadbackState::IDLE;
                        });
                }

                // --- Floater is_active sync (one-frame latency) ---
                // The kernel evicts floaters by writing is_active = 0u when
                // they drift beyond FLOATER_EVICTION_RADIUS from the pawn.
                // CPU mirror needs this signal to free slots for new spawns;
                // without it, slots leak and active counts cap far below
                // MAX_*_INSTANCES. This walks the full floater buffer and
                // detects true→false transitions on is_active, decrementing
                // the corresponding active count and clearing the CPU
                // mirror so run_spawn_preamble can reuse the slot.
                //
                // We do NOT trust GPU→CPU for true→true (the cube's still
                // alive, we already know) or false→false (slot's empty,
                // nothing to do). And we do NOT propagate CPU→GPU here —
                // CPU spawn writes a full slot already, so consistency in
                // that direction is already maintained.
                if (floaterReadbackState_ == FloaterReadbackState::COPIED) {
                    floaterReadbackState_ = FloaterReadbackState::MAPPING;
                    gpuState_.floating_entity_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::floating_entity_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [this, gen = worldGen_](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world.
                                // Buffer is still mapped, so Unmap unconditionally.
                                if (gen == worldGen_) {
                                    const auto* data = static_cast<const GPUFloatingEntityState*>(
                                        gpuState_.floating_entity_readback_staging().GetConstMappedRange(
                                            0, GPUState::floating_entity_buffer_size()));
                                    if (data) {
                                        // Race protection: if a slot was allocated
                                        // very recently (within the readback
                                        // pipeline depth), the readback is from
                                        // before allocation and would falsely
                                        // mark the slot inactive. Suppress the
                                        // decrement for slots whose last_alloc_time
                                        // is more recent than the readback's
                                        // "snapshot age."
                                        static constexpr float SPAWN_PROTECTION_S = 0.10f;
                                        float now = currentSeconds_;
                                        // Spheres: slots [0, MAX_SPHERE_INSTANCES)
                                        for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
                                            bool gpu_active = (data[i].is_active != 0u);
                                            if (activeFloaters_[i].active && !gpu_active &&
                                                (now - activeFloaters_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
                                                activeFloaters_[i].active = false;
                                                if (activeFloaterCount_ > 0) activeFloaterCount_--;
                                            }
                                        }
                                        // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
                                        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
                                            bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
                                            if (activeCubes_[i].active && !gpu_active &&
                                                (now - activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
                                                activeCubes_[i].active = false;
                                                if (activeCubeCount_ > 0) activeCubeCount_--;
                                            }
                                        }
                                    }
                                }
                                gpuState_.floating_entity_readback_staging().Unmap();
                            }
                            floaterReadbackState_ = FloaterReadbackState::IDLE;
                        });
                }

                // Check if GPU reported a portal trigger
                if (readbackPortalTrigger_ >= 0 && transitionPhase_ == TransitionPhase::IDLE) {
                    uint32_t arch_idx = static_cast<uint32_t>(readbackPortalTrigger_);
                    readbackPortalTrigger_ = -1;
                    if (arch_idx < Dim::MAX_ARCH_INSTANCES &&
                        activeArches_[arch_idx].active &&
                        activeArches_[arch_idx].is_portal) {
                        pendingDestination_ = activeArches_[arch_idx].destination;
                        transitionPhase_ = TransitionPhase::FADE_OUT;
                        transitionTimer_ = 0.0f;
                        std::cout << "[Portal] GPU trigger: arch " << arch_idx
                            << " -> seed=" << pendingDestination_.seed
                            << " finite=" << pendingDestination_.finite << "\n";
                    }
                }

                // Refill any agent slots the GPU evicted last frame.
                // No-op when no slots were evicted — just a 32-slot scan.
                respawn_evicted_agents(activeMood_, activeSeed_, queue);

                // Advance any in-flight cube corral animations. No-op
                // when none are armed (the common case — animations
                // only run for ~3s after F6 is pressed).
                tick_cube_corral_animations(queue);

                stream_patches(encoder, queue);

                // Periodic agent census dump — followed by the player's
                // last-known position from the GPU readback. The pos line
                // tells us at-a-glance whether the readback is current
                // (a stuck readback would freeze the position; an idle
                // player would do the same — pair the two by visiting
                // the world manually if you need to disambiguate).
                if (currentSeconds_ - lastAgentCensusDump_ >= AGENT_CENSUS_INTERVAL) {
                    dump_agent_census("periodic");
                    const auto& player = cpuAgents_[0];
                    std::cout << "[Player] pos=(" << std::fixed << std::setprecision(1)
                        << player.pos_x << "," << player.pos_z
                        << ") slot=" << player_.possessed_slot
                        << " behavior=" << player.behavior_id
                        << "\n";
                    lastAgentCensusDump_ = currentSeconds_;
                }

                // Periodic entity census dump
#ifdef DIAG_ENTITY_CENSUS
                if (currentSeconds_ - lastCensusDump_ >= CENSUS_DUMP_INTERVAL) {
                    dump_entity_census("periodic");
                    lastCensusDump_ = currentSeconds_;
                }
#endif

                // ─── Ribbon per-frame: eviction, time, nearest-rendering ─────
                {
                    // Ribbon eviction is now fully event-driven via ref_count
                    // in evict_patch_entities — no per-frame scan needed.

                    // Update time on all CPU mirrors
                    for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                        if (activeRibbons_[i].active)
                            ribbonStates_[i].time = currentSeconds_;
                    }

                    // Render one ribbon: hold the current slot until it's evicted,
                    // then pick the nearest active ribbon as the new rendered slot.
                    bool current_alive = renderedRibbonSlot_ != UINT32_MAX
                        && activeRibbons_[renderedRibbonSlot_].active;

                    if (current_alive) {
                        // Hold — just update time
                        gpuState_.upload_ribbon_time(queue, currentSeconds_);
                    }
                    else {
                        // Current slot is gone — find nearest active ribbon
                        uint32_t nearest = UINT32_MAX;
                        float nearest_d2 = FLT_MAX;
                        for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                            if (!activeRibbons_[i].active) continue;
                            float dx = activeRibbons_[i].anchor_x - pawnReadback_x_;
                            float dz = activeRibbons_[i].anchor_z - pawnReadback_z_;
                            float d2 = dx * dx + dz * dz;
                            if (d2 < nearest_d2) { nearest = i; nearest_d2 = d2; }
                        }

                        if (nearest != UINT32_MAX) {
                            gpuState_.upload_ribbon(queue, ribbonStates_[nearest]);
                            renderedRibbonSlot_ = nearest;
                        }
                        else if (renderedRibbonSlot_ != UINT32_MAX) {
                            GPURibbonState empty{};
                            gpuState_.upload_ribbon(queue, empty);
                            renderedRibbonSlot_ = UINT32_MAX;
                        }
                    }
                }

                // ─── Entity mesh gen: single compute pass for all dirty families ──
                {
                    bool dirty[PopFamily::COUNT];
                    bool anyDirty = false;
                    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                        dirty[f] = FAMILY_DISPATCH[f].prepare_mesh(this, queue);
                        anyDirty = anyDirty || dirty[f];
                    }
                    if (anyDirty) {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Entity Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                            if (dirty[f]) FAMILY_DISPATCH[f].dispatch_mesh(this, pass);
                        }
                        pass.End();
                    }
                }
                upload_portal_array(queue);
                upload_lights(queue);
                dispatch_compute(encoder);

                // Copy full agent buffer from GPU to staging (for readback next frame)
                if (pawnReadbackState_ == PawnReadbackState::IDLE) {
                    encoder.CopyBufferToBuffer(
                        gpuState_.agent_state_buffer(), 0,
                        gpuState_.agent_state_readback_staging(), 0,
                        GPUState::agent_state_buffer_size());
                    pawnReadbackState_ = PawnReadbackState::COPIED;
                }

                // Copy full floater buffer from GPU to staging (for readback next frame).
                // ~55 KB/frame; pays for itself by keeping CPU active mirrors
                // accurate so the spawn allocator can reuse evicted slots.
                if (floaterReadbackState_ == FloaterReadbackState::IDLE) {
                    encoder.CopyBufferToBuffer(
                        gpuState_.floating_entity_buffer(), 0,
                        gpuState_.floating_entity_readback_staging(), 0,
                        GPUState::floating_entity_buffer_size());
                    floaterReadbackState_ = FloaterReadbackState::COPIED;
                }

                // GoL zone compute — derive params + sync + evolve (separate passes for barrier)
                if (golZoneCount_ > 0) {
                    flush_zone_derive_requests(queue);
                    upload_gol_zone_config(queue);

                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Sync";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_gol_sync(pass,
                            gpuState_.zone_gol_compute_group(), activeZoneSlotCount_);
                        pass.End();
                    }
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Evolve";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_gol_evolve(pass,
                            gpuState_.zone_gol_compute_group(), activeZoneSlotCount_);
                        pass.End();
                    }

                    // Mesh gen pass (Group 0 = compute entity, Group 1 = zone mesh gen)
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_mesh_reset(pass,
                            gpuState_.zone_mesh_gen_group());
                        renderer_.dispatch_zone_mesh_gen(pass,
                            gpuState_.zone_mesh_gen_group(),
                            activeZoneSlotCount_);
                        pass.End();
                    }
                }

                // Pawn aura compute — persistent terrain influence
                // Run while presence > 0 (ramping down after toggle-off) or clearing
                if (auraPresence_ > 0.0f || auraNeedsClear_) {
                    if (auraCfgDirty_) {
                        // Full config upload — profile changed or first frame
                        auraCfgDirty_ = false;
                        const auto& ap = activeAuraProfile_;
                        // Aura expansion mode: scale radius, height, tint by intensity
                        float aura_expand = mmodeIntensity_[MMODE_AURA_EXPAND];
                        float radius_scale = 1.0f + aura_expand * 2.0f;    // up to 3× radius
                        float tint_scale = 1.0f + aura_expand * 1.5f;      // up to 2.5× tint

                        // Presence scales all aura params for smooth raise/lower
                        float p = auraPresence_;

                        GPUPawnAuraConfig auraCfg{};
                        auraCfg.cell_size = PATCH_CELL_SIZE;
                        auraCfg.influence_radius = ap.influence_radius * radius_scale * p;
                        auraCfg.attack_stiffness = ap.attack_stiffness;
                        auraCfg.attack_damping = ap.attack_damping;
                        auraCfg.release_rate = (p > 0.01f) ? ap.release_rate : 999.0f;
                        auraCfg.dt = currentDt_;
                        auraCfg.effect_mask = ap.effect_mask;
                        auraCfg.aura_n = 64;
                        auraCfg.tint_strength = std::min(ap.tint_strength * tint_scale * p, 1.0f);
                        auraCfg.tint_r = ap.tint_r;
                        auraCfg.tint_g = ap.tint_g;
                        auraCfg.tint_b = ap.tint_b;
                        auraCfg.delta_mode = ap.delta_mode;
                        auraCfg.delta_magnitude = ap.delta_magnitude;
                        auraCfg.t_beats = currentBeats_;
                        // height_scale gates the compute shader's R channel write (> 0.01 = enabled).
                        // Actual terrain extrusion magnitude comes from config.pawn_aura_height in the VS.
                        auraCfg.height_scale = (auraHeightEnabled_ && p > 0.01f) ? ap.height_scale : 0.0f;
                        gpuState_.upload_pawn_aura_config(queue, auraCfg);
                    }
                    else {
                        // Steady state — only dt and t_beats change per frame
                        gpuState_.upload_pawn_aura_frame(queue, currentDt_, currentBeats_);
                    }

                    wgpu::ComputePassDescriptor cpd{};
                    cpd.label = "Pawn Aura";
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                    renderer_.dispatch_compute_pawn_aura(pass,
                        gpuState_.pawn_aura_compute_group(),
                        GPUState::pawn_aura_workgroups());
                    pass.End();

                    // After one cleanup frame with release_rate=999, all cells are zero
                    if (auraNeedsClear_) { auraNeedsClear_ = false; }
                }

                // Orb sky layer: one-shot init, optional color-only refresh,
                // snapshot previous state for flocking neighbor reads, then
                // advance dynamics.
                dispatch_orb_init(encoder);
                dispatch_orb_recolor(encoder);
                dispatch_orb_copy_prev(encoder);
                dispatch_orb_dynamics(encoder, queue);

                if (groundEntriesDirty_) {
                    groundEntriesDirty_ = false;
                    placementDirty_ = true;
                    upload_ground_entries(queue);
                }
                if (placementDirty_) {
                    placementDirty_ = false;
                    dispatch_placement_correction(encoder);
                }

                // DIAG: frustum cull bypassed — direct draw active
                dispatch_frustum_cull(encoder, queue);

                render_shadow_pass(encoder);
                render_main_pass(encoder, backbuffer, depth);
                render_snapshot_pass(encoder);

                // --- Flush pending texture promotions (staging → exhibition) ---
                // Must run AFTER render_snapshot_pass so fresh captures are in staging
                // before being copied to exhibition layers.
                for (uint32_t i = 0; i < pendingPromotionCount_; i++) {
                    auto& p = pendingPromotions_[i];
                    wgpu::Texture src = p.is_snapshot
                        ? gpuState_.snapshot_staging_texture()
                        : gpuState_.authored_staging_texture();
                    gpuState_.promote_to_exhibition(encoder, src, p.staging_layer, p.exhibition_layer);
                }
                pendingPromotionCount_ = 0;
            }


            // --- Patch streaming: determine active 7×7 grid, generate new patches ---
            // SEAM[spine:owns] stream_patches is the patch-streaming integration
            //   backbone — ~460 lines covering allocation budgets, generation
            //   phases, eviction. Genuinely spine work; modules consume but
            //   don't own pieces. Per Ch. 15.
            void stream_patches(wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
                // ─── Patch Generation Pipeline ─────────────────────────────────
                //
                // This function orchestrates terrain streaming. Every stage is
                // continuous and budgeted per frame — no batched operations at
                // grid boundaries except the lightweight grid shift event.
                //
                // ON GRID SHIFT (pawn crosses a patch boundary):
                //   1. Update grid center
                //   2. Evict distant tiles from spatial cache (map erase, no GPU)
                //   3. Evict out-of-range GoL zones (flag clear + deactivate)
                //   4. Re-upload tile grid with new origin
                //   5. FULLREGEN ONLY — batch-allocate ALL, batch-spawn + generate
                //      inner patches synchronously. Pawn needs ground immediately.
                //
                // CONTINUOUS EVICTION (every frame):
                //   Scans patches outside the render window. Evicts up to
                //   EVICT_BUDGET_PER_FRAME (farthest first): free layer, clear
                //   entities, unregister footprints. Compact array afterward.
                //
                // CONTINUOUS ALLOCATION (every frame, after eviction):
                //   Scans grid cells within activeRadius_ of pawn's world position.
                //   Allocates missing patches up to ALLOC_BUDGET_PER_FRAME, nearest
                //   first. Populates tile cache and re-uploads tile grid.
                //
                // DISTANCE-DRIVEN SPAWN (every frame, after allocation):
                //   Scans unspawned patches, sorts by distance to pawn.
                //   Spawns up to SPAWN_BUDGET_PER_FRAME (pyramids → arches → columns).
                //
                // DISTANCE-DRIVEN GENERATION (every frame, after spawn):
                //   Scans spawned-but-ungenerated patches + pending regens.
                //   Sorts by distance to pawn (nearest first), generates up to budget.
                //
                // VISIBILITY CYLINDER (render list gate):
                //   World-space distance from pawn to patch edge. Patches enter
                //   the draw list one at a time as the pawn moves.
                //
                // EXTERNALLY (in render(), after stream_patches returns):
                //   - Entity mesh gen (single compute pass: arches + columns + pyramids)
                //   - dispatch_compute (pawn, camera, VP)
                //   - dispatch GoL zone compute (sync + evolve, if zones active)
                //   - dispatch_placement_correction (Y-correct arches, columns, pyramids, paintings — decoupled from photographer)
                //   - render passes (shadow, main, snapshot)
                //
                // GOL ZONE LIFECYCLE:
                //   - select_gol_for_patch: entity pipeline (select → place → commit),
                //     lattice-gated, footprint-registered. Evicts via host-patch entity_refs.
                //   - upload_gol_zone_config: per frame, before GoL compute dispatch.

                int32_t centerX, centerZ;
                uint32_t patchStagingOffset = 0;  // running offset into staging buffer (multiple batches per frame)
                bool tileGridDirty = false;        // coalesce tile grid uploads to one per frame
                if (finiteMode_) {
                    centerX = 0;
                    centerZ = 0;
                }
                else {
                    centerX = (int32_t)std::floor(pawnReadback_x_ / PATCH_EXTENT);
                    centerZ = (int32_t)std::floor(pawnReadback_z_ / PATCH_EXTENT);
                }

                // In finite mode, cap the effective radius
                uint32_t savedRadius = activeRadius_;
                if (finiteMode_ && activeRadius_ > finiteRadius_) {
                    activeRadius_ = finiteRadius_;
                }

                bool gridChanged = (centerX != lastCenterX_ || centerZ != lastCenterZ_);

                if (gridChanged) {
                    int32_t oldCX = lastCenterX_;
                    int32_t oldCZ = lastCenterZ_;
                    lastCenterX_ = centerX;
                    lastCenterZ_ = centerZ;

                    bool fullRegen = (oldCX == INT32_MAX);  // first frame

                    // Lightweight cache maintenance (no GPU buffer writes)
                    evict_distant_tiles(centerX, centerZ);
                    // GoL zones now evict through host-patch entity_refs

                    // Tile grid origin depends on grid center — re-upload
                    // unconditionally so GPU heightfield gen reads correct
                    // modifiers from the new origin.
                    if (!fullRegen) {
                        tileGridDirty = true;
                    }

                    // Guaranteed back-portal in finite worlds (fires once after teardown)
                    // DEFERRED: must wait for tile cache below (portals need terrain heights)

                    // ─── FULLREGEN: synchronous bootstrap ────────────────────
                    //
                    // First frame of a new world: batch-allocate ALL patches,
                    // spawn + generate inner patches synchronously so the pawn
                    // has ground immediately. Outer patches use the per-frame
                    // distance-driven scans like everything else.
                    if (fullRegen) {
                        int32_t rr = (int32_t)activeRadius_;
                        static constexpr int32_t TILE_PAD = 1;
                        int32_t rp = rr + TILE_PAD;
                        for (int32_t gz = centerZ - rp; gz <= centerZ + rp; gz++) {
                            for (int32_t gx = centerX - rp; gx <= centerX + rp; gx++) {
                                GridKey key{ gx, gz };
                                if (tileCache_.find(key) == tileCache_.end()) {
                                    TileState ts = generate_tile_state(gx, gz);
                                    tick_terrain_tokens(ts, tile_seed(activeSeed_, gx, gz));
                                    tileCache_[key] = ts;
                                }
                            }
                        }

                        // NOW spawn portals — tile cache is populated, terrain heights are correct
                        if (backPortalPending_) {
                            force_spawn_back_portal(queue);
                        }
                        for (int32_t gz = centerZ - rr; gz <= centerZ + rr; gz++) {
                            for (int32_t gx = centerX - rr; gx <= centerX + rr; gx++) {
                                bool found = false;
                                for (uint32_t i = 0; i < activePatchCount_; i++) {
                                    if (patches_[i].grid_x == gx && patches_[i].grid_z == gz) {
                                        found = true; break;
                                    }
                                }
                                if (!found && freeLayerCount_ > 0) {
                                    uint32_t layer = alloc_layer();
                                    patches_[activePatchCount_] = ActivePatch{};
                                    patches_[activePatchCount_].grid_x = gx;
                                    patches_[activePatchCount_].grid_z = gz;
                                    patches_[activePatchCount_].layer = layer;
                                    patches_[activePatchCount_].valid = true;
                                    activePatchCount_++;
                                }
                            }
                        }
                        tileGridDirty = true;

                        // Spawn inner patches
                        PatchCandidate spawnCands[MAX_PATCHES];
                        uint32_t spawnCount = collect_sorted_patches(spawnCands,
                            pawnReadback_x_, pawnReadback_z_,
                            [&](const ActivePatch& p) {
                                return p.phase == PatchPhase::ALLOCATED &&
                                    in_priority_window(p.grid_x, p.grid_z, centerX, centerZ);
                            }, true);
                        spawn_selected_patches(spawnCands, spawnCount, queue);

                        // Generate inner patches
                        PatchCandidate genCands[MAX_PATCHES];
                        uint32_t genCount = collect_sorted_patches(genCands,
                            pawnReadback_x_, pawnReadback_z_,
                            [&](const ActivePatch& p) {
                                return p.phase == PatchPhase::SPAWNED &&
                                    in_priority_window(p.grid_x, p.grid_z, centerX, centerZ);
                            }, true);
                        generate_selected_patches(genCands, genCount,
                            encoder, queue, patchStagingOffset, tileGridDirty);
                    }
                }

                // ─── CONTINUOUS PATCH EVICTION ────────────────────────────────
                //
                // Every frame, scan for patches outside the render window
                // (relative to grid center). Evict up to EVICT_BUDGET_PER_FRAME,
                // farthest first. Frees layers for reuse by the allocation scan.
                // Compact the array after eviction to remove holes.
                {
                    PatchCandidate candidates[MAX_PATCHES];
                    uint32_t count = collect_sorted_patches(candidates,
                        pawnReadback_x_, pawnReadback_z_,
                        [&](const ActivePatch& p) {
                            return !in_render_window(p.grid_x, p.grid_z,
                                lastCenterX_, lastCenterZ_);
                        }, false);  // farthest first

                    uint32_t evictThisFrame = std::min(count, EVICT_BUDGET_PER_FRAME);
                    for (uint32_t e = 0; e < evictThisFrame; e++) {
                        evict_patch(candidates[e].idx, queue);
                    }

                    if (evictThisFrame > 0) {
                        uint32_t write = 0;
                        for (uint32_t i = 0; i < activePatchCount_; i++) {
                            if (patches_[i].valid) patches_[write++] = patches_[i];
                        }
                        activePatchCount_ = write;
                        patchInstancesDirty_ = true;
                    }
                }

                // ─── CONTINUOUS PATCH ALLOCATION ──────────────────────────────
                //
                // Every frame, scan for grid cells within activeRadius_ of
                // the pawn's actual world position that don't have patches.
                // Allocate up to ALLOC_BUDGET_PER_FRAME, nearest first. This
                // spreads allocation across idle frames so patches are ready
                // before the grid shift that would have created them.
                //
                // The pawn's world position can be up to half a patch ahead
                // of the grid center, so this naturally pre-allocates one
                // ring in the direction of movement.
                {
                    int32_t pawnGX = (int32_t)std::floor(pawnReadback_x_ / PATCH_EXTENT);
                    int32_t pawnGZ = (int32_t)std::floor(pawnReadback_z_ / PATCH_EXTENT);
                    int32_t rr = (int32_t)activeRadius_;
                    float pawn_wx = pawnReadback_x_;
                    float pawn_wz = pawnReadback_z_;
                    float half = PATCH_EXTENT * 0.5f;

                    // O(1) patch existence lookup (replaces O(N) inner scan)
                    std::unordered_set<GridKey, GridKeyHash> activePatchSet;
                    activePatchSet.reserve(activePatchCount_);
                    for (uint32_t i = 0; i < activePatchCount_; i++) {
                        activePatchSet.insert({ patches_[i].grid_x, patches_[i].grid_z });
                    }

                    struct AllocCandidate { int32_t gx, gz; float dist2; };
                    AllocCandidate candidates[MAX_PATCHES];
                    uint32_t candidateCount = 0;

                    for (int32_t gz = pawnGZ - rr; gz <= pawnGZ + rr; gz++) {
                        for (int32_t gx = pawnGX - rr; gx <= pawnGX + rr; gx++) {
                            // Must be within allocation window of grid center
                            if (!in_render_window(gx, gz, lastCenterX_, lastCenterZ_)) continue;
                            bool found = activePatchSet.count({ gx, gz }) > 0;
                            if (!found && freeLayerCount_ > 0 && candidateCount < MAX_PATCHES) {
                                float ox = (gx + 0.5f) * PATCH_EXTENT;
                                float oz = (gz + 0.5f) * PATCH_EXTENT;
                                float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);
                                candidates[candidateCount++] = { gx, gz, d2 };
                            }
                        }
                    }

                    // Sort by distance (nearest first)
                    for (uint32_t i = 1; i < candidateCount; i++) {
                        AllocCandidate key = candidates[i];
                        uint32_t j = i;
                        while (j > 0 && candidates[j - 1].dist2 > key.dist2) {
                            candidates[j] = candidates[j - 1];
                            j--;
                        }
                        candidates[j] = key;
                    }

                    bool allocated_any = false;
                    uint32_t allocThisFrame = std::min(candidateCount, ALLOC_BUDGET_PER_FRAME);
                    for (uint32_t a = 0; a < allocThisFrame; a++) {
                        int32_t gx = candidates[a].gx;
                        int32_t gz = candidates[a].gz;
                        // Ensure tile cache entry (primary — ticks terrain tokens)
                        GridKey key{ gx, gz };
                        if (tileCache_.find(key) == tileCache_.end()) {
                            TileState ts = generate_tile_state(gx, gz);
                            tick_terrain_tokens(ts, tile_seed(activeSeed_, gx, gz));
                            tileCache_[key] = ts;
                        }
                        // Also cache neighbors for tile grid padding
                        for (int dz = -1; dz <= 1; dz++) for (int dx = -1; dx <= 1; dx++) {
                            GridKey nk{ gx + dx, gz + dz };
                            if (tileCache_.find(nk) == tileCache_.end()) {
                                tileCache_[nk] = generate_tile_state(gx + dx, gz + dz);
                            }
                        }
                        uint32_t layer = alloc_layer();
                        patches_[activePatchCount_] = ActivePatch{};
                        patches_[activePatchCount_].grid_x = gx;
                        patches_[activePatchCount_].grid_z = gz;
                        patches_[activePatchCount_].layer = layer;
                        patches_[activePatchCount_].valid = true;
                        activePatchCount_++;
                        allocated_any = true;
                    }

                    // Mark tile grid and patch instances dirty whenever new patches were allocated
                    if (allocated_any) {
                        tileGridDirty = true;
                        patchInstancesDirty_ = true;
                    }
                }

                // ─── DISTANCE-DRIVEN ENTITY SPAWNING ─────────────────────────
                //
                // Every frame, scan for unspawned patches. Sort by distance
                // to pawn (nearest first), spawn up to SPAWN_BUDGET_PER_FRAME.
                // Priority order within each patch: pyramids → arches → columns
                // (largest footprint first, matching the ground hierarchy).
                //
                // Spawning must complete before generation — piers from spawned
                // entities affect heightfield baking. The generation scan below
                // only considers patches with phase == SPAWNED or NEEDS_REGEN.
                {
                    PatchCandidate candidates[MAX_PATCHES];
                    uint32_t count = collect_sorted_patches(candidates,
                        pawnReadback_x_, pawnReadback_z_,
                        [](const ActivePatch& p) {
                            return p.phase == PatchPhase::ALLOCATED;
                        }, true);
                    spawn_selected_patches(candidates,
                        std::min(count, SPAWN_BUDGET_PER_FRAME), queue);
                }

                // ─── DISTANCE-DRIVEN HEIGHTFIELD GENERATION ──────────────────
                //
                // Every frame, scan all spawned patches for pending work
                // (SPAWNED or NEEDS_REGEN). Sort by world-space distance
                // to pawn (nearest first) and generate up to budget.
                //
                // Regens (stale heightfields from new piers) are already
                // inside the visibility cylinder, so they're always closer
                // than frontier patches and naturally get priority.
                {
                    PatchCandidate candidates[MAX_PATCHES];
                    uint32_t count = collect_sorted_patches(candidates,
                        pawnReadback_x_, pawnReadback_z_,
                        [](const ActivePatch& p) {
                            return p.phase == PatchPhase::SPAWNED ||
                                p.phase == PatchPhase::NEEDS_REGEN;
                        }, true);
                    generate_selected_patches(candidates,
                        std::min(count, patches_budget_this_frame()),
                        encoder, queue, patchStagingOffset, tileGridDirty);
                }

                // Upload patch instances sorted by LOD band, then pre-gen ring.
                // Layout: [0..lod0) LOD-0 full mesh, [lod0..render) LOD-1 half mesh,
                //          [render..all) pre-gen ring (not drawn, used for placement).
                // This lets render passes issue two indexed draws with firstInstance offset.
                {
                    GPUPatchInstance instances[MAX_PATCHES]{};
                    uint32_t lod0Count = 0;
                    uint32_t lod1Count = 0;
                    uint32_t pregenCount = 0;

                    // Temporary arrays for each band
                    GPUPatchInstance lod0[MAX_PATCHES]{};
                    GPUPatchInstance lod1[MAX_PATCHES]{};
                    GPUPatchInstance pregen[MAX_PATCHES]{};

                    // Visibility cylinder: world-space distance from pawn to
                    // nearest patch edge. Patches cross the threshold one at a
                    // time as the pawn moves — no batch pop on grid shifts.
                    float pawn_wx = pawnReadback_x_;
                    float pawn_wz = pawnReadback_z_;
                    float half = PATCH_EXTENT * 0.5f;

                    for (uint32_t i = 0; i < activePatchCount_; i++) {
                        if (patches_[i].phase != PatchPhase::GENERATED &&
                            patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;

                        float ox = (patches_[i].grid_x + 0.5f) * PATCH_EXTENT;
                        float oz = (patches_[i].grid_z + 0.5f) * PATCH_EXTENT;

                        GPUPatchInstance inst{};
                        inst.origin[0] = ox;
                        inst.origin[1] = oz;
                        inst.extent = PATCH_EXTENT;
                        inst.layer = patches_[i].layer;

                        float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);

                        // Finite mode: all patches visible (walls define boundary, not fog)
                        if (finiteMode_ || d2 <= VISIBILITY_CYLINDER_RADIUS_SQ) {
                            if (d2 <= LOD0_CYLINDER_RADIUS_SQ) {
                                lod0[lod0Count++] = inst;
                            }
                            else {
                                lod1[lod1Count++] = inst;
                            }
                        }
                        else {
                            pregen[pregenCount++] = inst;
                        }
                    }

                    // Pack: LOD-0, then LOD-1, then pregen
                    uint32_t w = 0;
                    std::memcpy(instances + w, lod0, lod0Count * sizeof(GPUPatchInstance)); w += lod0Count;
                    std::memcpy(instances + w, lod1, lod1Count * sizeof(GPUPatchInstance)); w += lod1Count;
                    std::memcpy(instances + w, pregen, pregenCount * sizeof(GPUPatchInstance)); w += pregenCount;

                    gpuState_.upload_patch_instances(queue, instances, w);
                    lod0PatchCount_ = lod0Count;
                    renderPatchCount_ = lod0Count + lod1Count;
                    allPatchCount_ = w;

                    // Sync placement_patch_count so compute_entity_placement
                    // can sample heightfields from the current frame's patch set.
                    gpuState_.config().placement_patch_count = w;
                    gpuState_.upload_placement_patch_count(queue);

                    // Push the CPU's banding pawn so the GPU frustum-cull
                    // shader uses the same pawn position to apply the LOD0
                    // distance gate. Without this, GPU reads the live pawn
                    // (1-2 frames ahead of pawnReadback) and disagrees with
                    // CPU at the LOD0/LOD1 boundary annulus, causing patch
                    // flicker around ~175 world units from the pawn.
                    gpuState_.config().lod_pawn_x = pawn_wx;
                    gpuState_.config().lod_pawn_z = pawn_wz;
                    gpuState_.upload_lod_pawn(queue);

                    // ─── Patch grid: O(1) spatial index for sample_terrain_y_at ────────
                    // Populate (patch_gx, patch_gz) → layer map. The shader derives a
                    // patch's grid cell from floor(world_xz / cell_extent) and looks up
                    // the layer directly, replacing the previous linear bbox scan.
                    // Entries store (layer + 1) so that 0 encodes an empty slot.
                    // Anchor is the bounding-box minimum of the valid set — always fits
                    // in a PATCH_PREGEN_SIDE × PATCH_PREGEN_SIDE window (15×15 = 225).
                    {
                        GPUPatchGrid grid{};
                        grid.side = Dim::PATCH_PREGEN_SIDE;
                        grid.cell_extent = PATCH_EXTENT;

                        int32_t min_gx = INT32_MAX;
                        int32_t min_gz = INT32_MAX;
                        for (uint32_t i = 0; i < activePatchCount_; i++) {
                            if (!patches_[i].valid) continue;
                            if (patches_[i].phase != PatchPhase::GENERATED &&
                                patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;
                            min_gx = std::min(min_gx, patches_[i].grid_x);
                            min_gz = std::min(min_gz, patches_[i].grid_z);
                        }
                        if (min_gx == INT32_MAX) { min_gx = 0; min_gz = 0; }
                        grid.origin_x = min_gx;
                        grid.origin_z = min_gz;
                        // entries[] are zero-initialized by value-init above.

                        for (uint32_t i = 0; i < activePatchCount_; i++) {
                            if (!patches_[i].valid) continue;
                            if (patches_[i].phase != PatchPhase::GENERATED &&
                                patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;
                            int32_t lx = patches_[i].grid_x - grid.origin_x;
                            int32_t lz = patches_[i].grid_z - grid.origin_z;
                            if (lx < 0 || lz < 0 ||
                                lx >= int32_t(grid.side) || lz >= int32_t(grid.side)) continue;
                            grid.entries[lz * grid.side + lx] = patches_[i].layer + 1u;
                        }

                        gpuState_.upload_patch_grid(queue, grid);
                    }
                }
                // GPU Y-correction is additive (ground_y += terrain), so ground
                // entries must be re-uploaded with offset-only values whenever
                // the heightfield changes. Tie groundEntriesDirty to patch changes.
                groundEntriesDirty_ = groundEntriesDirty_ || patchInstancesDirty_;
                placementDirty_ = placementDirty_ || patchInstancesDirty_;
                patchInstancesDirty_ = false;

                // ─── Entity distance culling ─────────────────────────────
                entitiesCulled_ = update_entity_draw_visibility(queue);

                // ─── Deferred uploads (one per frame max) ────────────────
                if (tileGridDirty) upload_tile_grid_now(queue, lastCenterX_, lastCenterZ_);
                flush_pier_count(queue);

                audit_entity_integrity();

                // Restore radius if we capped it for finite mode
                if (finiteMode_) { activeRadius_ = savedRadius; }
            }

            // ── Mood System (modules/mood.inl) ──
#include "modules/mood.inl"


// ── Render Passes (modules/render_passes.inl) ──
#include "modules/render_passes.inl"

        public:

            void on_input(const InputEvent& event) override {
                switch (event.type) {
                case InputEvent::Type::KeyDown:
                    on_key_down(event.key);
                    break;
                case InputEvent::Type::KeyUp:
                    on_key_up(event.key);
                    break;
                case InputEvent::Type::MouseMove:
                    on_mouse_move(event.x, event.y);
                    break;
                case InputEvent::Type::MouseButton:
                    on_mouse_button(event.button, event.pressed);
                    break;
                case InputEvent::Type::Scroll:
                    on_scroll(event.y);
                    break;
                }
            }

        private:

            // ── Input Handling (modules/input.inl) ──
#include "modules/input.inl"

        public:

            void get_clear_color(float& r, float& g, float& b) const override {
                r = clearColor_[0];
                g = clearColor_[1];
                b = clearColor_[2];
            }

            wgpu::TextureFormat depth_format() const override {
                return wgpu::TextureFormat::Depth24Plus;
            }

            bool supports_backspace() const override {
                return true;
            }

            bool reload_shaders() override { return renderer_.reload(); }
            const std::string& shader_path() const { return renderer_.shader_path(); }

        };

    } // namespace the_board
} // namespace t7/