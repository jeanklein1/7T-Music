// ─── gallery.inl ─────────────────────────────────────────────────
//
// The art system. Photographer, paintings, exhibitions,
// wall paintings, authored image loading + staging.
//
// Outdoor: photographer captures snapshots, paintings spawn on terrain.
// Indoor: wall paintings placed by mood system (authored + snapshot mix).
//
// Included inside the Cartridge class body.
// Depends on: entities.inl, terrain_cpu.inl, seed_utils.inl
//
// SEAM[gallery:complete-subsystem] complete bespoke pipeline in one
//   block — vocabulary + state + lifecycle + dispatch all together.
//   Same pattern as gol_zones (Ch. 12.B). Phase 2 extraction target.
// SEAM[gallery:dual-role] two named sub-systems sharing infrastructure:
//   painting-on-terrain (outdoor) and painting-on-wall (indoor) with
//   shared image loading + frame rendering, divergent spawn paths.
//   Comment-as-policy at the file header explicitly names the division.
//   Not a leak; intentional dual role.
// ─────────────────────────────────────────────────────────────────


// ─── Self-Portrait Gallery (photographer system) ─────────────────
//
// ─── Shot Tiers ─────────────────────────────────────────────────
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
            // SEAM[gallery:L1] ENVIRONMENTAL row has weight 0.01 — effectively
            //   disabled at the authoring level. Either intentionally rare for
            //   a reason that should be named, OR latent infrastructure (P8)
            //   for a future authoring change. Phase 3 cleanup batch: add a
            //   header comment naming the choice.
            static constexpr ShotTypeParams SHOT_PARAMS[] = {
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
            static constexpr float PAINTING_AREA[] = {
                30.0f,   // PANORAMIC:     large, cinematic canvas
                24.0f,   // ENVIRONMENTAL: medium canvas
                20.0f,   // MEDIUM:        moderate canvas
                20.0f,   // CLOSE_UP:      moderate canvas
                20.0f,   // PORTRAIT:      moderate (aspect makes it tall)
                18.0f,   // BIRDS_EYE:     moderate, near-square
                22.0f,   // LOW_ANGLE:     wide, dramatic
                28.0f,   // CINEMATIC:     large, ultrawide
            };

            // ─── Painting Spawn Configuration ───────────────────────────────
            //
            // SEAM[gallery:L2] this is a clean instance of pattern P3 (player
            //   state vs mood state, explicit) — concerns separated into
            //   named sub-structures rather than mixed in one big config.
            //   Same shape as orbs.inl's player-state vs mood-state split.
            //   Tag-only recognition.
            //
            // Split into two concerns:
            //   PhotographerConfig — controls snapshot capture cadence
            //   GalleryConfig      — controls where/how paintings appear on terrain

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
                static constexpr float ROW_SPACING = 18.0f;       // horizontal distance between paintings (was 10)
                static constexpr float ROW_DEPTH_MIN = 8.0f;      // minimum depth gap between rows (was 12)
                static constexpr float ROW_DEPTH_RANGE = 4.0f;    // depth jitter on top of minimum (was 6)
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
                //   70% snapshot-only terrain quads
                //   20% mixed (each painting rolls independently)
                //   10% authored-only wall frames (monuments in the desert)
                //
                // Indoor (place_wall_paintings):
                //   40% snapshot-only wall frames
                //   20% mixed (each painting rolls independently)
                //   40% authored-only wall frames
                //
                static constexpr float OUTDOOR_SNAPSHOT_ONLY = 0.80f;  // [0.00, 0.80)
                static constexpr float OUTDOOR_MIXED = 0.05f;  // [0.80, 0.85)
                // remainder 0.15 = authored-only                       // [0.85, 1.00)

                // (Indoor mix thresholds and snapshot chance moved to WALL_ART
                //  in the indoor section near the top of the cartridge — they
                //  control wall-art placement, not outdoor gallery placement.)

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
                    float roll = uniform(0.0f, 1.0f);
                    float cumul = 0.0f;
                    for (uint32_t t = 0; t < static_cast<uint32_t>(ShotType::COUNT); t++) {
                        cumul += SHOT_PARAMS[t].weight;
                        if (roll < cumul) return static_cast<ShotType>(t);
                    }
                    return static_cast<ShotType>(static_cast<uint32_t>(ShotType::COUNT) - 1);
                }
            } photographer_;

            // ─── Snapshot Staging (circular buffer, 16 layers) ───────────
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
            SnapshotStagingRecord snapshotStaging_[Dim::STAGING_LAYERS]{};
            uint32_t snapshotWriteCursor_ = 0;
            uint32_t snapshotCount_ = 0;
            float totalWalkDistance_ = 0.0f;
            uint32_t frameCounter_ = 0;

            // ─── Authored Staging (circular buffer, 16 layers) ───────────
            struct AuthoredStagingRecord {
                uint32_t disk_index = UINT32_MAX;
                float aspect_ratio = 1.0f;
                float uv_scale_x = 1.0f;
                float uv_scale_y = 1.0f;
                bool valid = false;
                bool consumed = false;
            };
            AuthoredStagingRecord authoredStaging_[Dim::STAGING_LAYERS]{};
            uint32_t authoredWriteCursor_ = 0;
            uint32_t authoredDiskCursor_ = 0;     // walks through authoredDiskManifest_
            uint32_t authoredStagedCount_ = 0;
            bool authoredTexturesLoaded_ = false;
            std::vector<std::string> authoredDiskManifest_;  // scanned at startup, sorted alphabetically

            // ─── Exhibition (32 layers, stable until portal) ─────────────
            bool exhibitionOccupied_[Dim::EXHIBITION_LAYERS]{};
            uint32_t exhibitionCount_ = 0;

            uint32_t find_free_exhibition_layer() const {
                for (uint32_t i = 0; i < Dim::EXHIBITION_LAYERS; i++)
                    if (!exhibitionOccupied_[i]) return i;
                return UINT32_MAX;
            }

            // Pending texture promotions (staging → exhibition, executed in render)
            struct PendingPromotion {
                bool is_snapshot;       // true = snapshot staging, false = authored staging
                uint32_t staging_layer;
                uint32_t exhibition_layer;
            };
            static constexpr uint32_t MAX_PROMOTIONS_PER_FRAME = 32;
            PendingPromotion pendingPromotions_[MAX_PROMOTIONS_PER_FRAME]{};
            uint32_t pendingPromotionCount_ = 0;

            void queue_promotion(bool is_snapshot, uint32_t staging_layer, uint32_t exhibition_layer) {
                if (pendingPromotionCount_ < MAX_PROMOTIONS_PER_FRAME) {
                    pendingPromotions_[pendingPromotionCount_++] = {
                        is_snapshot, staging_layer, exhibition_layer
                    };
                }
            }

            // ─── Painting Slots (exhibited paintings) ────────────────────
            GPUPaintingSlot paintingSlots_[Dim::PAINTING_MAX_SLOTS]{};
            uint32_t activePaintingCount_ = 0;
            uint32_t wallFrameCount_ = 0;

            // Active gallery centers (for minimum distance enforcement)
            struct GalleryCenter {
                float x = 0.0f, z = 0.0f;
                int32_t patch_gx = INT32_MAX, patch_gz = INT32_MAX;
                int32_t host_gx = 0, host_gz = 0;   // host patch (for entity_refs eviction)
                bool active = false;
            };
            static constexpr uint32_t MAX_GALLERIES = 48;
            GalleryCenter galleryCenters_[MAX_GALLERIES]{};

            struct PendingSnapshot {
                bool active = false;
                uint32_t target_slot = 0;
                uint32_t target_layer = 0;
            } pendingSnapshot_;

            // ─── Slot Management ─────────────────────────────────────────────

            uint32_t find_free_painting_slot() const {
                for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++)
                    if (paintingSlots_[i].is_active == 0) return i;
                return UINT32_MAX;
            }

            // ─── Photographer: Capture Only (no placement) ───────────────────
            //
            // Captures snapshots into a circular buffer of texture layers.
            // Never places paintings. Gallery sites consume the pool.

            void update_photographer(wgpu::Queue& queue) {
                float px = pawnReadback_x_;
                float pz = pawnReadback_z_;

                if (!photographer_.initialized) {
                    photographer_.prev_pawn_x = px;
                    photographer_.prev_pawn_z = pz;
                    photographer_.initialized = true;
                    return;
                }

                float dx = px - photographer_.prev_pawn_x;
                float dz = pz - photographer_.prev_pawn_z;
                float step = std::sqrt(dx * dx + dz * dz);
                photographer_.prev_pawn_x = px;
                photographer_.prev_pawn_z = pz;
                if (step > 5.0f) return;

                photographer_.cumulative_distance += step;
                totalWalkDistance_ += step;
                frameCounter_++;
                if (photographer_.frame_cooldown > 0) photographer_.frame_cooldown--;

                if (photographer_.pending_shots > 0 && photographer_.frame_cooldown == 0) {
                    capture_snapshot(px, pz, queue);
                    photographer_.pending_shots--;
                    photographer_.frame_cooldown = PhotographerCaptureConfig::BURST_COOLDOWN_FRAMES;
                    return;
                }

                if (photographer_.cumulative_distance >= photographer_.next_threshold) {
                    photographer_.cumulative_distance = 0.0f;

                    // Pace modulation: less active in sand/basin, more in colored terrain
                    float pace = 1.0f;
                    int32_t tx = (int32_t)std::floor(px / PATCH_EXTENT);
                    int32_t tz = (int32_t)std::floor(pz / PATCH_EXTENT);
                    auto it = tileCache_.find({ tx, tz });
                    if (it != tileCache_.end()) {
                        pace = GalleryConfig::PHOTO_PACE_BY_ARCHETYPE[it->second.archetype];
                    }

                    photographer_.next_threshold = std::max(
                        PhotographerCaptureConfig::TRIGGER_DISTANCE_FLOOR,
                        photographer_.gaussian(
                            PhotographerCaptureConfig::TRIGGER_DISTANCE_MEAN * pace,
                            PhotographerCaptureConfig::TRIGGER_DISTANCE_SIGMA));
                    photographer_.pending_shots = photographer_.sample_shot_count();
                    photographer_.frame_cooldown = 0;
                }
            }

            void capture_snapshot(float pawn_x, float pawn_z, wgpu::Queue& queue) {
                ShotType shot = photographer_.sample_shot_type();
                const auto& params = SHOT_PARAMS[static_cast<uint32_t>(shot)];

                float aspect_ratio = photographer_.uniform(params.aspect_lo, params.aspect_hi);
                float azimuth = photographer_.uniform(0.0f, 6.283185f);
                float dist = std::max(PhotographerCaptureConfig::DISTANCE_FLOOR,
                    photographer_.gaussian(params.distance_mean, params.distance_sigma));
                float elev = std::max(PhotographerCaptureConfig::ELEVATION_FLOOR,
                    photographer_.gaussian(params.elevation_mean, params.elevation_sigma));
                float fov_deg = std::max(PhotographerCaptureConfig::FOV_FLOOR,
                    photographer_.gaussian(params.fov_degrees, params.fov_sigma));

                if (photographer_.uniform(0.0f, 1.0f) < PhotographerCaptureConfig::WIDE_LENS_CHANCE) {
                    fov_deg = photographer_.uniform(PhotographerCaptureConfig::WIDE_LENS_FOV_LO,
                        PhotographerCaptureConfig::WIDE_LENS_FOV_HI);
                }

                float fov_rad = fov_deg * 3.14159f / 180.0f;
                float offset_x = photographer_.uniform(-params.offset_x_range, params.offset_x_range);
                float offset_y = photographer_.uniform(-params.offset_y_range, params.offset_y_range);

                // Record in snapshot staging — cursor wraps freely, unconditional overwrite
                uint32_t layer = snapshotWriteCursor_;
                auto& rec = snapshotStaging_[layer];
                rec.aspect_ratio = aspect_ratio;
                rec.shot_type = static_cast<uint32_t>(shot);
                rec.valid = true;
                rec.consumed = false;  // fresh capture, available for exhibition
                rec.capture_x = pawn_x;
                rec.capture_z = pawn_z;
                rec.capture_distance = totalWalkDistance_;
                rec.capture_frame = frameCounter_;
                snapshotWriteCursor_ = (layer + 1) % Dim::STAGING_LAYERS;
                if (snapshotCount_ < Dim::STAGING_LAYERS) snapshotCount_++;

                // Upload photographer config for GPU compute
                GPUPhotographerConfig cfg{};
                float slen = std::sqrt(sunDirection_[0] * sunDirection_[0] + sunDirection_[1] * sunDirection_[1] + sunDirection_[2] * sunDirection_[2]);
                cfg.sun_direction[0] = sunDirection_[0] / slen;
                cfg.sun_direction[1] = sunDirection_[1] / slen;
                cfg.sun_direction[2] = sunDirection_[2] / slen;
                cfg.azimuth = azimuth;
                cfg.elevation = elev;
                cfg.distance = dist;
                cfg.fov_rad = fov_rad;
                cfg.aspect_ratio = aspect_ratio;
                cfg.patch_count = allPatchCount_;
                cfg.frame_offset_x = offset_x;
                cfg.frame_offset_y = offset_y;
                cfg._pad0 = 0.0f;
                gpuState_.upload_photographer_config(queue, cfg);

                pendingSnapshot_.active = true;
                pendingSnapshot_.target_slot = UINT32_MAX;
                pendingSnapshot_.target_layer = layer;

                const char* shot_names[] = {
                    "Panoramic", "Environmental", "Medium", "Close-up",
                    "Portrait", "Bird's Eye", "Low Angle", "Cinematic"
                };
                std::cout << "[Photographer] Capture -> layer " << layer
                    << " (" << shot_names[static_cast<uint32_t>(shot)] << ")"
                    << " aspect=" << aspect_ratio
                    << " pool=" << snapshotCount_ << "/" << Dim::STAGING_LAYERS << "\n";
            }

            // ─── Gallery Sites: Paintings Spawned When Patches Stream In ─────
            //
            // Each patch's seed determines if it hosts a gallery, how many
            // paintings, and where. Paintings draw from the snapshot pool.
            // The user discovers photos from other places in distant galleries.

            // ─── select_gallery_for_patch ────────────────────────────────
            //
            // Phase 1: content gate, spawn roll, center jitter, parameter
            // sampling. No GPU writes. No content availability validation
            // (deferred to commit where queue is available).

            bool select_gallery_for_patch(int32_t gx, int32_t gz, GallerySelection& sel) {
                // Content gate: minimum snapshot pool
                if (snapshotCount_ < GalleryConfig::MIN_POOL_SIZE) return false;

                // Mood gate
                float adj_mod = GalleryConfig::MOOD_MULTIPLIER[activeMood_];
                if (adj_mod <= 0.0f) return false;

                // Density + theme modifiers
                adj_mod *= GLOBAL_ENTITY_DENSITY;
                adj_mod *= population_type_affinity(PopFamily::GALLERY);
                uint32_t archetype = 1;
                {
                    auto dit = tileCache_.find({ gx, gz });
                    if (dit != tileCache_.end()) {
                        adj_mod *= dit->second.entity_density;
                        adj_mod *= dit->second.theme_spawn[PopFamily::GALLERY];
                        archetype = dit->second.archetype;
                    }
                }

                // Idempotency: skip if paintings already exist at this patch
                for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
                    if (paintingSlots_[i].is_active != 0 &&
                        paintingSlots_[i].patch_gx == gx && paintingSlots_[i].patch_gz == gz) {
                        return false;
                    }
                }

                // Also check if a gallery center is already active for this patch
                for (uint32_t g = 0; g < MAX_GALLERIES; g++) {
                    if (galleryCenters_[g].active &&
                        galleryCenters_[g].patch_gx == gx && galleryCenters_[g].patch_gz == gz)
                        return false;
                }

                // Spawn roll
                uint32_t seed = tile_seed(activeSeed_, gx, gz);
                float gallery_roll = cpu_hash_f(seed, 500u);
                float gallery_chance = GalleryConfig::GALLERY_CHANCE_BY_ARCHETYPE[archetype] * adj_mod;
                if (gallery_roll >= gallery_chance) return false;

                // Gallery center (jittered within patch)
                float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
                float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
                float center_offset = cpu_hash_f(seed, 505u) * PATCH_EXTENT * GalleryConfig::POSITION_JITTER;
                float center_angle = cpu_hash_f(seed, 506u) * 6.283185f;
                float gallery_cx = patch_cx + std::cos(center_angle) * center_offset;
                float gallery_cz = patch_cz + std::sin(center_angle) * center_offset;

                // Gallery-to-gallery distance check (belt + suspenders; MIN_SEPARATION handles most)
                float min_dist_sq = GalleryConfig::MIN_GALLERY_DISTANCE * GalleryConfig::MIN_GALLERY_DISTANCE;
                for (uint32_t g = 0; g < MAX_GALLERIES; g++) {
                    if (!galleryCenters_[g].active) continue;
                    float dx = gallery_cx - galleryCenters_[g].x;
                    float dz = gallery_cz - galleryCenters_[g].z;
                    if (dx * dx + dz * dz < min_dist_sq) return false;
                }

                // Find free center slot
                uint32_t gallery_slot = UINT32_MAX;
                for (uint32_t g = 0; g < MAX_GALLERIES; g++) {
                    if (!galleryCenters_[g].active) { gallery_slot = g; break; }
                }
                if (gallery_slot == UINT32_MAX) return false;

                // Reserve slot
                galleryCenters_[gallery_slot].active = true;
                galleryCenters_[gallery_slot].patch_gx = gx;
                galleryCenters_[gallery_slot].patch_gz = gz;

                // Footprint radius: gallery spread
                float footprint_r = (float)GalleryConfig::PAINTINGS_MAX_BY_ARCHETYPE[archetype]
                    * 0.5f * GalleryConfig::ROW_SPACING + 15.0f;

                // Painting count (seed-derived; capped to content availability in commit)
                float count_raw = GalleryConfig::PAINTINGS_MEAN
                    + (cpu_hash_f(seed, 501u) + cpu_hash_f(seed, 502u)
                        + cpu_hash_f(seed, 503u) - 1.5f) * GalleryConfig::PAINTINGS_SIGMA;
                uint32_t painting_count = (uint32_t)std::max(
                    (float)GalleryConfig::PAINTINGS_MIN,
                    std::min((float)GalleryConfig::PAINTINGS_MAX_BY_ARCHETYPE[archetype],
                        std::round(count_raw)));

                // Facing + size
                float facing_angle = cpu_hash_f(seed, 504u) * 6.283185f;
                float gallery_size_mean = GalleryConfig::GALLERY_SIZE_LO
                    + cpu_hash_f(seed, 530u) * (GalleryConfig::GALLERY_SIZE_HI - GalleryConfig::GALLERY_SIZE_LO);

                // Site type (seed-derived; content availability validated in commit)
                float site_roll = cpu_hash_f(seed, 540u);
                uint32_t site_type;
                if (site_roll < GalleryConfig::OUTDOOR_SNAPSHOT_ONLY) {
                    site_type = GallerySiteType::SNAPSHOT_ONLY;
                }
                else if (site_roll < GalleryConfig::OUTDOOR_SNAPSHOT_ONLY + GalleryConfig::OUTDOOR_MIXED) {
                    site_type = GallerySiteType::MIXED;
                }
                else {
                    site_type = GallerySiteType::AUTHORED_ONLY;
                }

                sel.seed = seed;
                sel.trigger_gx = gx;
                sel.trigger_gz = gz;
                sel.slot = gallery_slot;
                sel.cx = gallery_cx;
                sel.cz = gallery_cz;
                sel.footprint_r = footprint_r;
                sel.archetype = archetype;
                sel.painting_count = painting_count;
                sel.facing_angle = facing_angle;
                sel.gallery_size_mean = gallery_size_mean;
                sel.site_type = site_type;

                return true;
            }

            // ─── place_gallery_from_selection ────────────────────────────
            //
            // Phase 2: footprint check + registration. Gallery center is
            // seed-determined (no negotiation), but standard check_position
            // enforces MIN_SEPARATION against all families.

            bool place_gallery_from_selection(const GallerySelection& sel, GalleryPlacement& plan) {
                if (!check_position(sel.cx, sel.cz, sel.footprint_r, PopFamily::GALLERY))
                    return false;

                int32_t host_gx = (int32_t)std::floor(sel.cx / PATCH_EXTENT);
                int32_t host_gz = (int32_t)std::floor(sel.cz / PATCH_EXTENT);

                if (register_footprint(sel.cx, sel.cz, sel.footprint_r,
                    host_gx, host_gz, PopFamily::GALLERY, sel.archetype) == UINT32_MAX)
                    return false;

                plan = GalleryPlacement{};
                plan.slot = sel.slot;
                plan.trigger_gx = sel.trigger_gx;
                plan.trigger_gz = sel.trigger_gz;
                plan.host_gx = host_gx;
                plan.host_gz = host_gz;
                plan.tier_idx = sel.archetype;
                plan.cx = sel.cx;
                plan.cz = sel.cz;
                plan.footprint_r = sel.footprint_r;
                plan.archetype = sel.archetype;
                plan.painting_count = sel.painting_count;
                plan.facing_angle = sel.facing_angle;
                plan.gallery_size_mean = sel.gallery_size_mean;
                plan.site_type = sel.site_type;

                record_placement_bookkeeping(PopFamily::GALLERY, plan.tier_idx);
                return true;
            }

            // ─── commit_gallery ──────────────────────────────────────────
            //
            // Phase 3: content curation, painting layout, slot allocation,
            // GPU upload. All content-dependent decisions happen here where
            // queue is available for authored texture loading.

            void commit_gallery(const GalleryPlacement& plan,
                int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue)
            {
                uint32_t seed = plan.trigger_gx != INT32_MAX
                    ? tile_seed(activeSeed_, plan.trigger_gx, plan.trigger_gz) : 0u;
                int32_t gx = plan.trigger_gx, gz = plan.trigger_gz;
                float gallery_cx = plan.cx, gallery_cz = plan.cz;
                uint32_t archetype = plan.archetype;
                float gallery_size_mean = plan.gallery_size_mean;
                float facing_angle = plan.facing_angle;

                // Resolve site type with content availability
                uint32_t site_type = plan.site_type;
                if (site_type != GallerySiteType::SNAPSHOT_ONLY && !authoredTexturesLoaded_) {
                    load_authored_textures(queue);
                }
                if (site_type != GallerySiteType::SNAPSHOT_ONLY && !authoredTexturesLoaded_) {
                    site_type = GallerySiteType::SNAPSHOT_ONLY;
                }

                // Snapshot candidates
                struct Candidate { uint32_t layer; };
                Candidate candidates[Dim::STAGING_LAYERS];
                uint32_t candidate_count = 0;
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                    if (snapshotStaging_[i].valid && !snapshotStaging_[i].consumed)
                        candidates[candidate_count++] = { i };
                }

                bool have_snapshots = candidate_count > 0;
                bool have_authored = authoredStagedCount_ > 0;
                if (site_type == GallerySiteType::SNAPSHOT_ONLY && !have_snapshots) {
                    galleryCenters_[plan.slot].active = false;
                    return;
                }
                if (site_type == GallerySiteType::AUTHORED_ONLY && !have_authored) {
                    galleryCenters_[plan.slot].active = false;
                    return;
                }
                if (site_type == GallerySiteType::MIXED && !have_snapshots && !have_authored) {
                    galleryCenters_[plan.slot].active = false;
                    return;
                }

                // Snapshot curation (mono tier + chronological sort)
                if (have_snapshots) {
                    bool mono_tier = cpu_hash_f(seed, 520u) < GalleryConfig::MONO_TIER_CHANCE;
                    if (mono_tier) {
                        static constexpr uint32_t FAVORITE_TIERS[] = {
                            (uint32_t)ShotType::PANORAMIC,
                            (uint32_t)ShotType::PORTRAIT,
                            (uint32_t)ShotType::CINEMATIC
                        };
                        uint32_t chosen = FAVORITE_TIERS[cpu_hash(seed, 521u) % 3];
                        uint32_t write = 0;
                        for (uint32_t c = 0; c < candidate_count; c++) {
                            if (snapshotStaging_[candidates[c].layer].shot_type == chosen)
                                candidates[write++] = candidates[c];
                        }
                        candidate_count = write;
                        have_snapshots = candidate_count > 0;
                    }
                    for (uint32_t i = 0; i < candidate_count; i++) {
                        for (uint32_t j = i + 1; j < candidate_count; j++) {
                            if (snapshotStaging_[candidates[j].layer].capture_frame
                                < snapshotStaging_[candidates[i].layer].capture_frame) {
                                Candidate tmp = candidates[i];
                                candidates[i] = candidates[j];
                                candidates[j] = tmp;
                            }
                        }
                    }
                }
                uint32_t snap_cursor = 0;

                // Cap painting count to available content
                uint32_t painting_count = plan.painting_count;
                uint32_t max_available = candidate_count + authoredStagedCount_;
                if (site_type == GallerySiteType::SNAPSHOT_ONLY) max_available = candidate_count;
                if (site_type == GallerySiteType::AUTHORED_ONLY) max_available = authoredStagedCount_;
                if (painting_count > max_available) painting_count = max_available;

                // Layout
                float face_x = std::cos(facing_angle);
                float face_z = std::sin(facing_angle);
                float row_x = -face_z;
                float row_z = face_x;
                float row_start = -(float)(painting_count - 1) * 0.5f * GalleryConfig::ROW_SPACING;

                uint32_t placed = 0;
                bool usedAuthored[Dim::STAGING_LAYERS]{};

                for (uint32_t p = 0; p < painting_count; p++) {
                    uint32_t slot = find_free_painting_slot();
                    if (slot == UINT32_MAX) break;

                    uint32_t p_seed = cpu_hash(seed, 510u + p * 7u);

                    float t = row_start + (float)p * GalleryConfig::ROW_SPACING;
                    float lateral_jitter = (cpu_hash_f(p_seed, 0u) - 0.5f) * 2.0f
                        * GalleryConfig::ROW_LATERAL_JITTER;
                    float depth_offset = GalleryConfig::ROW_DEPTH_MIN
                        + cpu_hash_f(p_seed, 1u) * GalleryConfig::ROW_DEPTH_RANGE;
                    if (p % 2 == 1) depth_offset = -depth_offset;

                    float paint_x = gallery_cx + row_x * (t + lateral_jitter) + face_x * depth_offset;
                    float paint_z = gallery_cz + row_z * (t + lateral_jitter) + face_z * depth_offset;

                    bool use_authored = (site_type == GallerySiteType::AUTHORED_ONLY)
                        || (site_type == GallerySiteType::MIXED
                            && cpu_hash_f(p_seed, 8u) < GalleryConfig::OUTDOOR_MIX_AUTHORED_CHANCE);

                    if (use_authored && count_unused_authored(usedAuthored) == 0) {
                        use_authored = false;
                    }
                    if (!use_authored && snap_cursor >= candidate_count) {
                        if (count_unused_authored(usedAuthored) > 0) {
                            use_authored = true;
                        }
                        else {
                            break;
                        }
                    }

                    auto& s = paintingSlots_[slot];
                    bool placed_this = false;

                    if (use_authored) {
                        uint32_t auth_stg = pick_authored_staging(p_seed, 9u);
                        if (auth_stg == UINT32_MAX || usedAuthored[auth_stg]) {
                            uint32_t best = UINT32_MAX, best_disk = UINT32_MAX;
                            for (uint32_t a = 0; a < Dim::STAGING_LAYERS; a++) {
                                if (!usedAuthored[a] && authoredStaging_[a].valid && !authoredStaging_[a].consumed
                                    && authoredStaging_[a].disk_index < best_disk) {
                                    best_disk = authoredStaging_[a].disk_index;
                                    best = a;
                                }
                            }
                            if (best == UINT32_MAX) { use_authored = false; }
                            else { auth_stg = best; }
                        }

                        if (use_authored) {
                            uint32_t exh = find_free_exhibition_layer();
                            if (exh == UINT32_MAX) break;

                            usedAuthored[auth_stg] = true;
                            const auto& img = authoredStaging_[auth_stg];
                            float jitter = (cpu_hash_f(p_seed, 3u) + cpu_hash_f(p_seed, 5u)
                                + cpu_hash_f(p_seed, 6u) - 1.5f) * GalleryConfig::PAINTING_SIZE_SIGMA;
                            float height = std::max(2.0f, (5.0f + jitter) * gallery_size_mean);

                            fill_slot_wall_frame(s,
                                paint_x, 0.0f, paint_z,
                                face_x, 0.0f, face_z,
                                img.aspect_ratio, height,
                                exh, ContentSource::AUTHORED,
                                img.uv_scale_x, img.uv_scale_y,
                                FRAME_AUTHORED, gx, gz);
                            s.geometry_seed = cpu_hash_f(p_seed, 4u);

                            exhibitionOccupied_[exh] = true;
                            exhibitionCount_++;
                            authoredStaging_[auth_stg].consumed = true;
                            queue_promotion(false, auth_stg, exh);
                            wallFrameCount_++;
                            placed_this = true;
                        }
                    }

                    if (!placed_this && snap_cursor < candidate_count) {
                        uint32_t staging_layer = candidates[snap_cursor].layer;
                        const auto& snap = snapshotStaging_[staging_layer];
                        snap_cursor++;

                        uint32_t exh = find_free_exhibition_layer();
                        if (exh == UINT32_MAX) break;

                        uint32_t shot_idx = snap.shot_type;
                        float jitter = (cpu_hash_f(p_seed, 3u) + cpu_hash_f(p_seed, 5u)
                            + cpu_hash_f(p_seed, 6u) - 1.5f) * GalleryConfig::PAINTING_SIZE_SIGMA;
                        float size_mult = std::max(0.5f, gallery_size_mean + jitter);
                        float area = PAINTING_AREA[shot_idx] * size_mult;
                        float scale_x = std::sqrt(area * snap.aspect_ratio);
                        float scale_y = scale_x / snap.aspect_ratio;

                        s = {};
                        s.position[0] = paint_x;
                        s.position[1] = 0.0f;
                        s.position[2] = paint_z;
                        s.geometry_seed = cpu_hash_f(p_seed, 4u);
                        s.forward[0] = face_x; s.forward[1] = 0.0f; s.forward[2] = face_z;
                        s.scale_x = scale_x;
                        s.up[0] = 0.0f; s.up[1] = 1.0f; s.up[2] = 0.0f;
                        s.scale_y = scale_y;
                        s.texture_layer = exh;
                        s.form_type = FormType::TERRAIN_QUAD;
                        s.is_active = 1;
                        s.content_source = ContentSource::SNAPSHOT;
                        s.uv_scale_x = 1.0f;
                        s.uv_scale_y = 1.0f;
                        s.patch_gx = gx; s.patch_gz = gz;

                        exhibitionOccupied_[exh] = true;
                        exhibitionCount_++;
                        snapshotStaging_[staging_layer].consumed = true;
                        queue_promotion(true, staging_layer, exh);
                        placed_this = true;
                    }

                    if (!placed_this) break;

                    gpuState_.upload_painting_slot(queue, slot, s);
                    activePaintingCount_++;
                    placed++;
                }

                // Record gallery center
                auto& gc = galleryCenters_[plan.slot];
                gc.x = gallery_cx;
                gc.z = gallery_cz;
                gc.host_gx = plan.host_gx;
                gc.host_gz = plan.host_gz;

                if (placed == 0) {
                    gc.active = false;  // no paintings placed — release center
                }
                else {
                    std::cout << "[Gallery] slot=" << plan.slot
                        << " at (" << gallery_cx << "," << gallery_cz << ")"
                        << " host=(" << plan.host_gx << "," << plan.host_gz << ")"
                        << " arch=" << archetype
                        << " paintings=" << placed << "/" << painting_count
                        << " type=" << (site_type == GallerySiteType::SNAPSHOT_ONLY ? "snap" :
                            site_type == GallerySiteType::MIXED ? "mix" : "auth")
                        << "\n";
                }
            }
            void evict_paintings_for_patch(int32_t gx, int32_t gz, wgpu::Queue& queue) {
                for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
                    if (paintingSlots_[i].is_active != 0 &&
                        paintingSlots_[i].patch_gx == gx && paintingSlots_[i].patch_gz == gz) {

                        // Free the exhibition layer
                        uint32_t exh = paintingSlots_[i].texture_layer;
                        if (exh < Dim::EXHIBITION_LAYERS) {
                            exhibitionOccupied_[exh] = false;
                            exhibitionCount_--;
                        }

                        if (paintingSlots_[i].form_type == FormType::WALL_FRAME) {
                            wallFrameCount_--;
                        }

                        paintingSlots_[i].is_active = 0;
                        gpuState_.deactivate_painting_slot(queue, i);
                        activePaintingCount_--;
                    }
                }
                // Evict gallery center for this patch — NO: gallery centers persist
                // beyond patch lifetime to prevent overlap when patches re-stream.
                // Centers are evicted by distance sweep in stream_patches instead.
            }

            // ─── Snapshot Render + Y Correction ──────────────────────────────

            // (pendingYCorrection_ removed — placement correction now runs every frame)

            void render_snapshot_pass(wgpu::CommandEncoder& encoder) {
                if (!pendingSnapshot_.active) return;

                // Snapshot needs its own VP compute (camera position + VP matrix).
                // Entity Y-correction already ran in dispatch_placement_correction
                // (separate pipeline, unconditional every frame).
                // This dispatch only builds the photographer camera VP + light VP.
                {
                    wgpu::ComputePassDescriptor cpd{};
                    cpd.label = "Photographer VP Compute";
                    wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&cpd);
                    renderer_.dispatch_compute_photographer_vp(
                        compute, gpuState_.photographer_compute_group()
                    );
                    compute.End();
                }

                // Only render the snapshot if a capture is pending
                if (!pendingSnapshot_.active) return;
                pendingSnapshot_.active = false;
                uint32_t layer = pendingSnapshot_.target_layer;
                std::cout << "[Photographer] Rendering snapshot -> layer " << layer << "\n";

                wgpu::RenderPassColorAttachment colorAtt{};
                colorAtt.view = gpuState_.offscreen_color_view();
                colorAtt.loadOp = wgpu::LoadOp::Clear;
                colorAtt.storeOp = wgpu::StoreOp::Store;
                colorAtt.clearValue = { (double)clearColor_[0], (double)clearColor_[1], (double)clearColor_[2], 1.0 };

                wgpu::RenderPassDepthStencilAttachment depthAtt{};
                depthAtt.view = gpuState_.offscreen_depth_view();
                depthAtt.depthLoadOp = wgpu::LoadOp::Clear;
                depthAtt.depthStoreOp = wgpu::StoreOp::Store;
                depthAtt.depthClearValue = 1.0f;

                wgpu::RenderPassDescriptor desc{};
                desc.label = "Photographer Snapshot";
                desc.colorAttachmentCount = 1;
                desc.colorAttachments = &colorAtt;
                desc.depthStencilAttachment = &depthAtt;

                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);

                renderer_.draw_patch_terrain_direct(pass,
                    gpuState_.photographer_render_entity_group(),
                    gpuState_.render_texture_group(),
                    gpuState_.patch_index_buffer(),
                    gpuState_.patch_index_count(),
                    renderPatchCount_);

                renderer_.draw_pawn(pass,
                    gpuState_.photographer_render_entity_group(),
                    gpuState_.render_texture_group(),
                    GPUState::pawn_vertex_count());

                renderer_.draw_sphere(pass,
                    gpuState_.photographer_render_entity_group(),
                    gpuState_.render_texture_group(),
                    gpuState_.sphere_vertex_buffer(),
                    gpuState_.sphere_index_buffer(),
                    gpuState_.sphere_index_count());

                if (renderedRibbonSlot_ != UINT32_MAX) {
                    renderer_.draw_ribbon(pass,
                        gpuState_.photographer_render_entity_group(),
                        gpuState_.render_texture_group(),
                        GPUState::ribbon_vertex_count());
                }

                renderer_.draw_arch(pass,
                    gpuState_.photographer_render_entity_group(),
                    gpuState_.render_texture_group(),
                    gpuState_.arch_vertex_buffer(),
                    gpuState_.arch_index_buffer(),
                    gpuState_.arch_index_count());

                renderer_.draw_column(pass,
                    gpuState_.photographer_render_entity_group(),
                    gpuState_.render_texture_group(),
                    gpuState_.column_vertex_buffer(),
                    gpuState_.column_index_buffer(),
                    gpuState_.column_index_count());

                renderer_.draw_shell(pass,
                    gpuState_.photographer_render_entity_group(),
                    gpuState_.render_texture_group(),
                    gpuState_.shell_vertex_buffer(),
                    gpuState_.shell_index_buffer(),
                    gpuState_.shell_index_count());

                // Pyramids: terrain surface IS the pyramid shape (via the baked
                // heightfield, which caches POLICY_BAKED_HEIGHTFIELD = static
                // base + pyramids). No separate mesh draw needed.

                pass.End();

                wgpu::TexelCopyTextureInfo src{};
                src.texture = gpuState_.offscreen_color_texture();
                src.mipLevel = 0;
                src.origin = { 0, 0, 0 };

                wgpu::TexelCopyTextureInfo dst{};
                dst.texture = gpuState_.snapshot_staging_texture();
                dst.mipLevel = 0;
                dst.origin = { 0, 0, layer };

                wgpu::Extent3D extent = { Dim::PAINTING_RESOLUTION, Dim::PAINTING_RESOLUTION, 1 };
                encoder.CopyTextureToTexture(&src, &dst, &extent);
            }


            // ─── Wall Paintings + Authored Image Loading ─────────────────────
            // Called by apply_mood / generate_indoor_shell for indoor worlds.

                        // ─── Authored Image Loading (staging model) ─────────────────

            void load_authored_image_to_staging(wgpu::Queue& queue, uint32_t staging_layer, uint32_t disk_index, const char* path) {
                int width = 0, height = 0, channels = 0;
                unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
                if (!data) {
                    // Try fallback paths
                    std::string alt = std::string("7t/") + path;
                    data = stbi_load(alt.c_str(), &width, &height, &channels, 4);
                }
                if (!data) {
                    std::cerr << "[Authored] Failed to load: " << path << "\n";
                    return;
                }

                std::cout << "[Authored] Loaded: " << path
                    << " (" << width << "x" << height << ") → staging " << staging_layer << "\n";

                constexpr uint32_t RES = Dim::PAINTING_RESOLUTION;
                float scale = std::min((float)RES / width, (float)RES / height);
                if (scale > 1.0f) scale = 1.0f;
                uint32_t dst_w = std::min((uint32_t)(width * scale + 0.5f), RES);
                uint32_t dst_h = std::min((uint32_t)(height * scale + 0.5f), RES);

                std::vector<uint8_t> padded(RES * RES * 4, 0);
                for (uint32_t dy = 0; dy < dst_h; ++dy) {
                    float src_yf = (float)dy / scale;
                    uint32_t sy0 = (uint32_t)src_yf;
                    uint32_t sy1 = std::min(sy0 + 1, (uint32_t)(height - 1));
                    float fy = src_yf - sy0;
                    for (uint32_t dx = 0; dx < dst_w; ++dx) {
                        float src_xf = (float)dx / scale;
                        uint32_t sx0 = (uint32_t)src_xf;
                        uint32_t sx1 = std::min(sx0 + 1, (uint32_t)(width - 1));
                        float fx = src_xf - sx0;
                        uint32_t i00 = (sy0 * width + sx0) * 4;
                        uint32_t i10 = (sy0 * width + sx1) * 4;
                        uint32_t i01 = (sy1 * width + sx0) * 4;
                        uint32_t i11 = (sy1 * width + sx1) * 4;
                        uint32_t di = (dy * RES + dx) * 4;
                        for (int c = 0; c < 4; ++c) {
                            float v = (1 - fx) * (1 - fy) * data[i00 + c] + fx * (1 - fy) * data[i10 + c]
                                + (1 - fx) * fy * data[i01 + c] + fx * fy * data[i11 + c];
                            padded[di + c] = (uint8_t)(v + 0.5f);
                        }
                    }
                }

                gpuState_.upload_authored_painting(queue, staging_layer, padded.data(), RES, RES);
                stbi_image_free(data);

                auto& rec = authoredStaging_[staging_layer];
                rec.disk_index = disk_index;
                rec.aspect_ratio = (height > 0) ? (float)width / (float)height : 1.0f;
                rec.uv_scale_x = (float)dst_w / RES;
                rec.uv_scale_y = (float)dst_h / RES;
                rec.valid = true;
                rec.consumed = false;

                std::cout << "[Authored] Scaled → " << dst_w << "x" << dst_h
                    << " (aspect " << rec.aspect_ratio << ")\n";
            }

            // ─── Paintings Folder Scan ─────────────────────────────────
            // Scans assets/paintings/ for PAINTING_*.jpg/jpeg, sorted alphabetically.
            // Called once at first load. The full collection lives on disk;
            // a rotating 16-layer staging window loads into GPU memory.

            void scan_paintings_folder() {
                namespace fs = std::filesystem;
                authoredDiskManifest_.clear();

                // Try multiple base paths (build dir vs working dir)
                static constexpr const char* SEARCH_DIRS[] = {
                    "assets/paintings",
                    "7t/assets/paintings",
                };

                fs::path found_dir;
                for (const char* dir : SEARCH_DIRS) {
                    if (fs::exists(dir) && fs::is_directory(dir)) {
                        found_dir = dir;
                        break;
                    }
                }
                if (found_dir.empty()) {
                    std::cout << "[Authored] No paintings folder found\n";
                    return;
                }

                for (const auto& entry : fs::directory_iterator(found_dir)) {
                    if (!entry.is_regular_file()) continue;
                    std::string name = entry.path().filename().string();
                    // Match PAINTING_*.jpg or PAINTING_*.jpeg (case-insensitive extension)
                    if (name.rfind("PAINTING_", 0) != 0) continue;
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext != ".jpg" && ext != ".jpeg") continue;
                    authoredDiskManifest_.push_back(entry.path().string());
                }

                // Sort by numeric value after PAINTING_ (not lexicographic)
                // PAINTING_1 < PAINTING_2 < PAINTING_10 < PAINTING_100
                auto extract_number = [](const std::string& path) -> int {
                    namespace fs = std::filesystem;
                    std::string name = fs::path(path).stem().string();  // "PAINTING_12"
                    size_t pos = name.find('_');
                    if (pos == std::string::npos || pos + 1 >= name.size()) return 0;
                    try { return std::stoi(name.substr(pos + 1)); }
                    catch (...) { return 0; }
                    };
                std::sort(authoredDiskManifest_.begin(), authoredDiskManifest_.end(),
                    [&](const std::string& a, const std::string& b) {
                        return extract_number(a) < extract_number(b);
                    });

                std::cout << "[Authored] Scanned " << found_dir.string()
                    << " — found " << authoredDiskManifest_.size() << " paintings\n";
            }

            void load_authored_textures(wgpu::Queue& queue) {
                if (authoredTexturesLoaded_) return;

                // Scan folder on first load
                if (authoredDiskManifest_.empty()) {
                    scan_paintings_folder();
                }
                if (authoredDiskManifest_.empty()) {
                    authoredTexturesLoaded_ = true;
                    return;
                }

                // Fill staging with the first STAGING_LAYERS images from manifest
                uint32_t manifest_size = (uint32_t)authoredDiskManifest_.size();
                uint32_t to_load = std::min(manifest_size, Dim::STAGING_LAYERS);
                for (uint32_t i = 0; i < to_load; i++) {
                    load_authored_image_to_staging(queue, i, i, authoredDiskManifest_[i].c_str());
                    if (authoredStaging_[i].valid) authoredStagedCount_++;
                }
                authoredWriteCursor_ = to_load % Dim::STAGING_LAYERS;
                authoredDiskCursor_ = to_load % manifest_size;
                authoredTexturesLoaded_ = true;
                std::cout << "[Authored] Staged " << authoredStagedCount_
                    << "/" << manifest_size << " images\n";
            }

            // Replace consumed authored staging slots with the next images from disk.
            // Called at teardown — consumed slots get fresh paintings, unconsumed survive.
            // The disk cursor walks through the entire manifest across world transitions,
            // so the pawn sees different paintings in each world.
            void rotate_authored_staging(wgpu::Queue& queue) {
                if (authoredDiskManifest_.empty()) return;
                uint32_t manifest_size = (uint32_t)authoredDiskManifest_.size();

                // Collect disk indices currently in unconsumed (surviving) slots
                // to avoid loading duplicates
                bool disk_in_use[256]{};  // generous upper bound
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                    if (authoredStaging_[i].valid && !authoredStaging_[i].consumed) {
                        if (authoredStaging_[i].disk_index < 256)
                            disk_in_use[authoredStaging_[i].disk_index] = true;
                    }
                }

                uint32_t rotated = 0;
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                    if (!authoredStaging_[i].consumed) continue;  // keep unconsumed

                    // Find next disk image not already in a surviving slot
                    uint32_t attempts = 0;
                    while (attempts < manifest_size) {
                        uint32_t disk_idx = authoredDiskCursor_;
                        authoredDiskCursor_ = (authoredDiskCursor_ + 1) % manifest_size;
                        if (disk_idx < 256 && disk_in_use[disk_idx]) {
                            attempts++;
                            continue;
                        }
                        // Load this image into the vacated staging slot
                        load_authored_image_to_staging(queue, i, disk_idx,
                            authoredDiskManifest_[disk_idx].c_str());
                        if (disk_idx < 256) disk_in_use[disk_idx] = true;
                        rotated++;
                        break;
                    }
                    // If all manifest images are in surviving slots (unlikely with 50+),
                    // the consumed slot just stays invalid
                }

                if (rotated > 0) {
                    // Recount valid slots after rotation
                    authoredStagedCount_ = 0;
                    for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                        if (authoredStaging_[i].valid) authoredStagedCount_++;
                    }
                    std::cout << "[Authored] Rotated " << rotated
                        << " slot(s), " << authoredStagedCount_ << " valid"
                        << ", disk cursor at " << authoredDiskCursor_
                        << "/" << manifest_size << "\n";
                }
            }

            // Count how many valid authored staging entries aren't in usedAuthored[]
            uint32_t count_unused_authored(const bool usedAuthored[]) const {
                uint32_t count = 0;
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                    if (authoredStaging_[i].valid && !authoredStaging_[i].consumed && !usedAuthored[i]) count++;
                }
                return count;
            }

            // Pick the next authored painting in alphabetical order (lowest disk_index first)
            uint32_t pick_authored_staging(uint32_t /*seed*/, uint32_t /*prop*/) {
                uint32_t best_slot = UINT32_MAX;
                uint32_t best_disk = UINT32_MAX;
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                    if (authoredStaging_[i].valid && !authoredStaging_[i].consumed
                        && authoredStaging_[i].disk_index < best_disk) {
                        best_disk = authoredStaging_[i].disk_index;
                        best_slot = i;
                    }
                }
                return best_slot;
            }

            // ─── Frame Style Presets ─────────────────────────────────────
            struct FrameStyle {
                float depth, width, recess;
                float color[3];
            };
            // Authored: thick dark wood (museum frame)
            static constexpr FrameStyle FRAME_AUTHORED = { 0.30f, 0.45f, 0.09f, { 0.25f, 0.15f, 0.08f } };
            // Snapshot on wall: same museum frame (content is different, ceremony is the same)
            static constexpr FrameStyle FRAME_SNAPSHOT = { 0.30f, 0.45f, 0.09f, { 0.25f, 0.15f, 0.08f } };

            // ─── Slot Fill Helpers ────────────────────────────────────────

            void fill_slot_wall_frame(
                GPUPaintingSlot& s,
                float x, float y, float z,
                float nx, float ny, float nz,
                float aspect_ratio, float base_height,
                uint32_t layer, uint32_t content,
                float uv_sx, float uv_sy,
                const FrameStyle& frame,
                int32_t gx, int32_t gz
            ) {
                s = {};
                s.position[0] = x; s.position[1] = y; s.position[2] = z;
                s.forward[0] = nx; s.forward[1] = ny; s.forward[2] = nz;
                s.up[0] = 0.0f; s.up[1] = 1.0f; s.up[2] = 0.0f;
                s.form_type = FormType::WALL_FRAME;
                s.is_active = 1;
                s.scale_x = base_height * aspect_ratio;
                s.scale_y = base_height;
                s.texture_layer = layer;
                s.content_source = content;
                s.uv_scale_x = uv_sx;
                s.uv_scale_y = uv_sy;
                s.frame_depth = frame.depth;
                s.frame_width = frame.width;
                s.canvas_recess = frame.recess;
                s.frame_color[0] = frame.color[0];
                s.frame_color[1] = frame.color[1];
                s.frame_color[2] = frame.color[2];
                s.patch_gx = gx; s.patch_gz = gz;
            }

            // ─── Wall Painting Placement (unified slots, multi-wall, mixing) ─

            void place_wall_paintings(wgpu::Queue& queue, float bmin, float bmax, float ceiling_h) {
                // Clear any existing wall paintings first (indoor→indoor transitions)
                clear_wall_paintings(queue);

                load_authored_textures(queue);

                // Painting center base height (fraction of ceiling) — WALL_ART knob.
                constexpr float WALL_OFFSET = 0.05f;    // distance from wall surface

                float paint_y_base = ceiling_h * WALL_ART.paint_y_frac;
                float wall_span = bmax - bmin;
                float wall_center = (bmin + bmax) * 0.5f;

                // Three-way site type: snapshot-only / mixed / authored-only
                uint32_t site_seed = cpu_hash(activeSeed_, 5500u);
                float site_roll = cpu_hash_f(site_seed, 0u);
                enum class IndoorSiteType { SNAPSHOT_ONLY, MIXED, AUTHORED_ONLY };
                IndoorSiteType site_type;
                if (site_roll < WALL_ART.snapshot_only_share && snapshotCount_ > 0) {
                    site_type = IndoorSiteType::SNAPSHOT_ONLY;
                }
                else if (site_roll < WALL_ART.snapshot_only_share + WALL_ART.mixed_share
                    && snapshotCount_ > 0) {
                    site_type = IndoorSiteType::MIXED;
                }
                else {
                    site_type = IndoorSiteType::AUTHORED_ONLY;
                }

                // Wall definitions: position, normal, tangent (for spacing)
                struct WallDef {
                    float px, py, pz;    // wall center position
                    float nx, ny, nz;    // inward normal
                    float tx, tz;        // tangent direction (for spacing paintings along wall)
                    float span;          // wall length
                };
                WallDef walls[] = {
                    { wall_center, paint_y_base, bmin + WALL_OFFSET,   0,0,1,   1,0,  wall_span },
                    { wall_center, paint_y_base, bmax - WALL_OFFSET,   0,0,-1,  -1,0, wall_span },
                    { bmin + WALL_OFFSET, paint_y_base, wall_center,   1,0,0,   0,1,  wall_span },
                    { bmax - WALL_OFFSET, paint_y_base, wall_center,  -1,0,0,   0,-1, wall_span },
                };
                constexpr uint32_t WALL_COUNT = 4;

                // Roll how many walls get paintings (1–4). Cumulative thresholds
                // come from WALL_ART — t1/t2/t3, residual → 4 walls.
                float wall_count_roll = cpu_hash_f(site_seed, 1u);
                uint32_t active_wall_count;
                if (wall_count_roll < WALL_ART.wall_count_t1)      active_wall_count = 1;
                else if (wall_count_roll < WALL_ART.wall_count_t2) active_wall_count = 2;
                else if (wall_count_roll < WALL_ART.wall_count_t3) active_wall_count = 3;
                else                                               active_wall_count = 4;
                uint32_t active_walls[4] = { 0, 1, 2, 3 };
                // Fisher-Yates shuffle
                for (uint32_t i = 3; i > 0; i--) {
                    uint32_t j = cpu_hash(site_seed, 2u + i) % (i + 1);
                    uint32_t tmp = active_walls[i];
                    active_walls[i] = active_walls[j];
                    active_walls[j] = tmp;
                }

                // Track which authored layers have been used across all walls (no duplicates)
                bool usedAuthored[Dim::STAGING_LAYERS]{};

                // ─── Painting scale buckets ────────────────────────────────────
                // Tabulated form lets the bucket-selection loop iterate the WALL_ART
                // sub-structs uniformly without repeating field names.
                const WallArtScaleBucket* INDOOR_SCALES[] = {
                    &WALL_ART.intimate,
                    &WALL_ART.standard,
                    &WALL_ART.statement,
                };
                static constexpr uint32_t INDOOR_SCALE_COUNT = 3;

                for (uint32_t aw = 0; aw < active_wall_count; aw++) {
                    uint32_t w = active_walls[aw];
                    const auto& wall = walls[w];
                    uint32_t w_seed = cpu_hash(site_seed, 10u + w * 20u);

                    // Per-wall count: uniform [lo, hi] inclusive, from WALL_ART.
                    uint32_t count_range = WALL_ART.per_wall_count_hi - WALL_ART.per_wall_count_lo + 1;
                    uint32_t count = WALL_ART.per_wall_count_lo + cpu_hash(w_seed, 0u) % count_range;

                    // Keep paintings away from corners — WALL_ART knob.
                    float usable_span = std::max(wall.span - 2.0f * WALL_ART.corner_margin,
                        wall.span * 0.3f);

                    // ─── Pre-compute widths to center the group on the wall ──
                    float total_width = 0.0f;
                    float painting_widths[8]{};
                    float painting_heights[8]{};
                    uint32_t effective_count = std::min(count, 8u);

                    for (uint32_t p = 0; p < effective_count; p++) {
                        uint32_t p_seed = cpu_hash(w_seed, 100u + p * 10u);

                        // Scale selection (weighted)
                        float scale_roll = cpu_hash_f(p_seed, 7u);
                        float cumul = 0.0f;
                        uint32_t scale_idx = INDOOR_SCALE_COUNT - 1;
                        for (uint32_t si = 0; si < INDOOR_SCALE_COUNT; si++) {
                            cumul += INDOOR_SCALES[si]->weight;
                            if (scale_roll < cumul) { scale_idx = si; break; }
                        }
                        float h = INDOOR_SCALES[scale_idx]->height_lo
                            + cpu_hash_f(p_seed, 3u) * (INDOOR_SCALES[scale_idx]->height_hi - INDOOR_SCALES[scale_idx]->height_lo);
                        painting_heights[p] = h;

                        // Estimate width from typical aspect ratio (~1.3)
                        float est_aspect = 0.8f + cpu_hash_f(p_seed, 5u) * 0.8f;  // [0.8, 1.6]
                        painting_widths[p] = h * est_aspect;
                        total_width += painting_widths[p];
                        if (p > 0) total_width += WALL_ART.painting_gap;
                    }

                    // Trim paintings that don't fit
                    while (effective_count > 1 && total_width > usable_span) {
                        total_width -= painting_widths[effective_count - 1];
                        total_width -= WALL_ART.painting_gap;
                        effective_count--;
                    }

                    // Center the group on the wall
                    float group_start = wall_center - total_width * 0.5f;
                    float cursor = group_start;

                    for (uint32_t p = 0; p < effective_count; p++) {

                        uint32_t slot = find_free_painting_slot();
                        if (slot == UINT32_MAX) return;

                        uint32_t p_seed = cpu_hash(w_seed, 100u + p * 10u);

                        // Vertical position: scale-dependent offset from base
                        // Intimate pieces: hung higher. Statement pieces: anchored lower.
                        float scale_roll = cpu_hash_f(p_seed, 7u);
                        float cumul = 0.0f;
                        uint32_t scale_idx = INDOOR_SCALE_COUNT - 1;
                        for (uint32_t si = 0; si < INDOOR_SCALE_COUNT; si++) {
                            cumul += INDOOR_SCALES[si]->weight;
                            if (scale_roll < cumul) { scale_idx = si; break; }
                        }

                        // Y-offset: uniform [lo, hi] per bucket from WALL_ART.
                        const auto& bucket = *INDOOR_SCALES[scale_idx];
                        float y_offset = bucket.y_offset_lo
                            + cpu_hash_f(p_seed, 1u) * (bucket.y_offset_hi - bucket.y_offset_lo);

                        float py = wall.py + y_offset;

                        // Upper clamp on painting bottom edge — keeps the bottom
                        // viewable from pawn standing height. The painting height
                        // we sampled into painting_heights[p] earlier defines half.
                        float h_for_clamp = painting_heights[p];
                        float bottom = py - h_for_clamp * 0.5f;
                        if (bottom > WALL_ART.max_bottom_height) {
                            py = WALL_ART.max_bottom_height + h_for_clamp * 0.5f;
                        }

                        // ─── Content decision (three-way) ────────────────
                        bool use_snapshot = (site_type == IndoorSiteType::SNAPSHOT_ONLY)
                            || (site_type == IndoorSiteType::MIXED
                                && cpu_hash_f(p_seed, 2u) < WALL_ART.mix_snapshot_chance);

                        if (!use_snapshot && count_unused_authored(usedAuthored) == 0) {
                            use_snapshot = true;
                        }

                        auto& s = paintingSlots_[slot];
                        float paint_width = 0.0f;  // will be set by whichever path fills the slot

                        if (use_snapshot) {
                            uint32_t snap_stg = UINT32_MAX;
                            for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) {
                                if (snapshotStaging_[i].valid && !snapshotStaging_[i].consumed) {
                                    snap_stg = i;
                                    break;
                                }
                            }
                            if (snap_stg == UINT32_MAX) {
                                if (count_unused_authored(usedAuthored) == 0) continue;
                                use_snapshot = false;
                            }
                            else {
                                uint32_t exh = find_free_exhibition_layer();
                                if (exh == UINT32_MAX) return;

                                const auto& snap = snapshotStaging_[snap_stg];
                                float height = painting_heights[p];
                                paint_width = height * snap.aspect_ratio;

                                // Safety check: actual width may differ from estimate
                                float wall_right = wall_center + usable_span * 0.5f;
                                if (cursor + paint_width > wall_right) break;

                                float paint_center = cursor + paint_width * 0.5f;
                                float px = wall.px + wall.tx * (paint_center - wall_center);
                                float pz = wall.pz + wall.tz * (paint_center - wall_center);

                                fill_slot_wall_frame(s,
                                    px, py, pz,
                                    wall.nx, wall.ny, wall.nz,
                                    snap.aspect_ratio, height,
                                    exh, ContentSource::SNAPSHOT,
                                    1.0f, 1.0f,
                                    FRAME_SNAPSHOT,
                                    INT32_MAX, INT32_MAX);

                                exhibitionOccupied_[exh] = true;
                                exhibitionCount_++;
                                snapshotStaging_[snap_stg].consumed = true;
                                queue_promotion(true, snap_stg, exh);

                                cursor += paint_width + WALL_ART.painting_gap;
                                gpuState_.upload_painting_slot(queue, slot, s);
                                wallFrameCount_++;
                                continue;
                            }
                        }

                        if (!use_snapshot) {
                            uint32_t auth_stg = pick_authored_staging(p_seed, 4u);
                            if (auth_stg == UINT32_MAX) continue;
                            if (usedAuthored[auth_stg]) {
                                uint32_t best = UINT32_MAX, best_disk = UINT32_MAX;
                                for (uint32_t a = 0; a < Dim::STAGING_LAYERS; a++) {
                                    if (!usedAuthored[a] && authoredStaging_[a].valid && !authoredStaging_[a].consumed
                                        && authoredStaging_[a].disk_index < best_disk) {
                                        best_disk = authoredStaging_[a].disk_index;
                                        best = a;
                                    }
                                }
                                if (best == UINT32_MAX) continue;
                                auth_stg = best;
                            }

                            uint32_t exh = find_free_exhibition_layer();
                            if (exh == UINT32_MAX) return;

                            usedAuthored[auth_stg] = true;

                            const auto& img = authoredStaging_[auth_stg];
                            float height = painting_heights[p];
                            paint_width = height * img.aspect_ratio;

                            // Safety check: actual width may differ from estimate
                            float wall_right = wall_center + usable_span * 0.5f;
                            if (cursor + paint_width > wall_right) break;

                            float paint_center = cursor + paint_width * 0.5f;
                            float px = wall.px + wall.tx * (paint_center - wall_center);
                            float pz = wall.pz + wall.tz * (paint_center - wall_center);

                            fill_slot_wall_frame(s,
                                px, py, pz,
                                wall.nx, wall.ny, wall.nz,
                                img.aspect_ratio, height,
                                exh, ContentSource::AUTHORED,
                                img.uv_scale_x, img.uv_scale_y,
                                FRAME_AUTHORED,
                                INT32_MAX, INT32_MAX);

                            exhibitionOccupied_[exh] = true;
                            exhibitionCount_++;
                            authoredStaging_[auth_stg].consumed = true;
                            queue_promotion(false, auth_stg, exh);

                            cursor += paint_width + WALL_ART.painting_gap;
                            gpuState_.upload_painting_slot(queue, slot, s);
                            wallFrameCount_++;
                        }
                    }
                }

                const char* site_type_name = (site_type == IndoorSiteType::SNAPSHOT_ONLY) ? "SNAPSHOT"
                    : (site_type == IndoorSiteType::MIXED) ? "MIXED" : "AUTHORED";
                std::cout << "[WallPainting] Placed " << wallFrameCount_
                    << " frame(s) across " << active_wall_count << " walls"
                    << " (" << site_type_name << ")\n";
            }

            void clear_wall_paintings(wgpu::Queue& queue) {
                for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
                    if (paintingSlots_[i].is_active != 0 &&
                        paintingSlots_[i].form_type == FormType::WALL_FRAME) {
                        uint32_t exh = paintingSlots_[i].texture_layer;
                        if (exh < Dim::EXHIBITION_LAYERS) {
                            exhibitionOccupied_[exh] = false;
                            exhibitionCount_--;
                        }
                        paintingSlots_[i].is_active = 0;
                        gpuState_.deactivate_painting_slot(queue, i);
                    }
                }
                wallFrameCount_ = 0;
            }

