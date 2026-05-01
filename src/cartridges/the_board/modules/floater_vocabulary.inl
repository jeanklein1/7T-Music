// ─── floater_vocabulary.inl ──────────────────────────────────────
//
// Per-family VOCABULARY for the two generic-pipeline floater families:
// Sphere (orbital, PGA motor-driven) and Cube (hover-bob monoliths).
// Tier profiles, spawn config, property-index registry, runtime
// tracking structs.
//
// Behavior layer for cubes (force functions, coordination knob,
// kite mode, corral animation) lives in cube_behaviors.inl.
// Spheres have no behavior layer — analytical PGA orbit only.
// Sampling profiles (Gaussian draws) for both families live in
// entity_pipeline.inl.
//
// Included inside the Cartridge class body.
// Depends on: state.hpp (Dim::*), entities.inl (MOOD_COUNT)
//
// SEAM[floater_vocabulary:taxonomy] generic-pipeline floater families
//   parallel grounded families (entities.inl) but live in their own
//   file because their tier shapes differ (sphere has orbit_radius/
//   orbit_speed, cube doesn't). Phase 2 outcome of D-floater (β):
//   extract floater_vocabulary.inl, rename old floaters.inl to
//   cube_behaviors.inl. Three concerns, three files: vocabulary
//   here, sampling profile in entity_pipeline.inl, cube behavior
//   gains in cube_behaviors.inl.
// ─────────────────────────────────────────────────────────────────

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
// DONE[sphere:L1] FloatingEntityTierProfile renamed to SphereTierProfile
//   in Phase 3. Used only by spheres; cubes have their own
//   CubeTierProfile struct with different fields. The misleading
//   "(Reuses FloatingEntityTierProfile...)" comment is also gone.
// ─────────────────────────────────────────────────────────────────

            // ─── Sphere Tier Profile ─────────────────────────────────────
            // Cubes have their own CubeTierProfile (different fields —
            // floater physics differs by motion shape: spheres orbit,
            // cubes hover-bob).
            struct SphereTierProfile {
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
            static constexpr SphereTierProfile SPHERE_TIERS[SPHERE_TIER_COUNT] = {
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
            //   cube_behaviors.inl::toggle_cube_kite_mode's GPU sentinel; different
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
//   gains in cube_behaviors.inl. Three concerns, three files, each correct.
// SEAM[cube:cx-cz-mirror] ActiveCube has cx, cz fields — CPU mirror of
//   GPU anchor for cube_behaviors.inl::corral_cubes / toggle_cube_kite_mode
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
