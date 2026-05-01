// ─── ribbon.inl ──────────────────────────────────────────────────
//
// Ribbon dispatch pipeline (bespoke, not table-driven).
// Single-instance ribbon through the 3-phase dispatch pipeline.
// GPU buffer is singleton (upload_ribbon, not slot-indexed).
//
// Vocabulary (RibbonTierProfile, RibbonProp, RibbonConfig,
// HarmonicRatio palettes, RIBBON_TIERS table) lives in entities.inl.
// This module owns the machinery: fill_ribbon_selection_geometry,
// select_ribbon_for_patch, place_ribbon_from_selection, commit_ribbon.
//
// Included inside the Cartridge class body.
// Depends on: entities.inl (vocabulary), spawn_engine.inl (helpers),
//             seed_utils.inl, cartridge.hpp core (currentSeconds_,
//             pawnReadback_*, THEMES, PATCH_EXTENT, Dim::*).
//
// SEAM[ribbon:taxonomy] ribbon MACHINERY lives here while ribbon
//   VOCABULARY lives in entities.inl. UNIQUE among bespoke families
//   in keeping vocabulary and machinery in separate files. Phase 2.7
//   normalized ribbon to the gol_zones / gallery shape (own block) by
//   extracting the machinery here.
// SEAM[ribbon:dual-entry] commit_ribbon below has TWO callers:
//   FAMILY_DISPATCH[RIBBON].try_commit during patch streaming, AND
//   mood.inl::apply_mood for mood-5 forced spawn. The dual entry
//   point is owned by mood:K4 (mood-5 reference clone), not by
//   ribbon machinery. Tag-only awareness.
// ─────────────────────────────────────────────────────────────────


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
