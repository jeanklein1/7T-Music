// ─── entities.inl ────────────────────────────────────────────────
//
// The vocabulary of forms: Ribbon, Arch, Column, Pyramid.
// Tier profiles, property registries, tracking structs.
//
// Included inside the Cartridge class body.
// Depends on: seed_utils.inl
//
// SEAM[entities:P10] this block is the canonical home of pattern P10
//   (per-family vocabulary block). Eight family applications follow:
//   Ribbon (bespoke), Arch, Column, Antenna, Palm, Cactus, Blade,
//   Pyramid (generic-pipeline grounded). Each block has the same
//   structural template: TierEnum / Color palette / Config / Prop
//   registry / Active tracking. Don't fight the cookie-cutter — it's
//   intentional specificity per family.
// DONE[entities:K1] resolved via Option B: tier sampling profile +
//   extras live as a single per-family TierRow struct in
//   entity_pipeline.inl. The TierParams structs and TIERS arrays that
//   used to sit here are gone (single source of truth, no converters,
//   no derived tables). The legacy enum classes (ArchTier, ColumnTier,
//   etc.) stay here — they're indexing semantics, not data.
// SEAM[entities:taxonomy] this block holds vocabulary for the 7 grounded
//   families plus Ribbon (bespoke). Sphere/Cube vocabulary is NOT here
//   — those live in cartridge.hpp itself in the specialized-families
//   block (Ch. 13.A, 13.B). Verified by Ch. 13 chunk-3 read.
// ─────────────────────────────────────────────────────────────────


//
// Ribbons exist at deterministic world locations on a coarse grid (~600 units).
// Each cell has a probability of containing a ribbon. When the pawn is within
// render distance, the ribbon activates with seed-derived parameters.
//
// Flying ribbons are alive, high in the sky, with animated waves.
//
// Cube count: 100–400. Cube size: pawn height (1.5) to 4× (6.0).
// Flying height: 50–80 units. All wave params independently seeded.

// ─── Sky Ribbon System ─────────────────────────────────────────
//
// Flying ribbons: compound wave functions (lateral + vertical + twist)
// forming square-tube geometry in the sky. Each ribbon is a tier
// instance with Gaussian-sampled parameters.
//
// Architecture follows the entity pattern:
//   RibbonProp        — property index registry
//   RibbonColorMode   — color tier weights
//   RibbonTierProfile — mean+sigma for all wave/geometry parameters

            // ─── Ribbon dispatch-pipeline config ────────────────────────────
            struct RibbonConfig {
                static constexpr float SPAWN_CHANCE = 0.400f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.3f;
            };

            // ── Color Modes ──────────────────────────────────────────────────
            struct RibbonColorMode {
                static constexpr uint32_t SMOOTH = 0;  // terrain-derived monochrome
                static constexpr uint32_t TINTED = 1;  // warm/cool hue shift
                static constexpr uint32_t CONTRAST = 2;  // high-contrast alternating segments
                static constexpr uint32_t COUNT = 3;
                static constexpr float WEIGHTS[COUNT] = { 0.40f, 0.35f, 0.25f };
            };

            // Smooth color palettes: base colors for SMOOTH mode ribbons
            static constexpr float RIBBON_SMOOTH_PALETTE[][3] = {
                { 0.82f, 0.75f, 0.62f },   // warm sandstone
                { 0.55f, 0.65f, 0.78f },   // sky blue
                { 0.85f, 0.78f, 0.58f },   // golden
                { 0.50f, 0.68f, 0.55f },   // sage green
            };
            static constexpr uint32_t RIBBON_SMOOTH_PALETTE_COUNT = 4;

            // ── Property Index Registry ──────────────────────────────────────
            //
            // DONE[entities:L1] Stride convention (intentional, do not compact):
            //   400      SPAWN_ROLL
            //   401-409  per-instance scalar rolls (ANCHOR_X..PALETTE_IDX)
            //   410-419  cube-count / size / height       (10-row reserve)
            //   420-429  lateral wave  (amp, cycles, speed; rest reserved)
            //   430-439  vertical wave (amp, ratio; rest reserved)
            //   440-449  twist wave    (amp, ratio; rest reserved)
            //   The per-axis stride of 10 leaves room for future per-axis
            //   params without renumbering downstream. Same self-documentation
            //   discipline used by the WGSL side.
            struct RibbonProp {
                static constexpr uint32_t SPAWN_ROLL = 400u;
                static constexpr uint32_t ANCHOR_X = 401u;
                static constexpr uint32_t ANCHOR_Z = 402u;
                static constexpr uint32_t TIER = 403u;
                static constexpr uint32_t COLOR_ROLL = 404u;
                static constexpr uint32_t ORIENTATION = 405u;
                static constexpr uint32_t COLOR_R = 406u;
                static constexpr uint32_t COLOR_G = 407u;
                static constexpr uint32_t COLOR_B = 408u;
                static constexpr uint32_t PALETTE_IDX = 409u;
                // Gaussian draw indices
                static constexpr uint32_t CUBE_COUNT = 410u;
                static constexpr uint32_t CUBE_SIZE = 411u;
                static constexpr uint32_t HEIGHT = 412u;
                static constexpr uint32_t LATERAL_AMP = 420u;
                static constexpr uint32_t LATERAL_CYCLES = 421u;
                static constexpr uint32_t LATERAL_SPEED = 422u;
                static constexpr uint32_t VERTICAL_AMP = 430u;
                static constexpr uint32_t VERTICAL_SPEED = 432u;
                static constexpr uint32_t VERTICAL_RATIO = 433u;    // seed roll for vertical harmonic ratio selection
                static constexpr uint32_t TWIST_AMP = 440u;
                static constexpr uint32_t TWIST_SPEED = 442u;
                static constexpr uint32_t TWIST_RATIO = 443u;       // seed roll for twist harmonic ratio selection
            };

            // ─── Harmonic Ratio Palettes ─────────────────────────────────────
            //
            // Secondary wave cycles (vertical, twist) are derived as simple
            // ratios of the lateral fundamental. This eliminates irrational
            // beating and gives each ribbon a harmonically coherent form.
            //
            // Ratios ≤ 1 keep secondary motion slower than lateral sway
            // (contemplative, not snaky). Weights favor the middle intervals.

            struct HarmonicRatio {
                float ratio;
                float weight;
                const char* name;      // musical interval name for diagnostics
            };

            static constexpr uint32_t VERTICAL_RATIO_COUNT = 4;
            static constexpr HarmonicRatio VERTICAL_RATIOS[VERTICAL_RATIO_COUNT] = {
                { 1.0f / 3.0f,  0.15f, "1:3" },   // breathe at 1/3 of sway
                { 1.0f / 2.0f,  0.35f, "1:2" },   // octave below — one breath per two sways
                { 2.0f / 3.0f,  0.30f, "2:3" },   // fifth below — gentle polyrhythm
                { 1.0f / 1.0f,  0.20f, "1:1" },   // unison — breathe with sway
            };

            static constexpr uint32_t TWIST_RATIO_COUNT = 4;
            static constexpr HarmonicRatio TWIST_RATIOS[TWIST_RATIO_COUNT] = {
                { 1.0f / 4.0f,  0.15f, "1:4" },   // very slow corkscrew
                { 1.0f / 3.0f,  0.30f, "1:3" },   // one turn per three sways
                { 1.0f / 2.0f,  0.35f, "1:2" },   // one turn per two sways
                { 2.0f / 3.0f,  0.20f, "2:3" },   // fifth below
            };

            static uint32_t select_harmonic_ratio(uint32_t seed, uint32_t prop,
                const HarmonicRatio* palette, uint32_t count) {
                float roll = cpu_hash_f(seed, prop);
                float cumul = 0.0f;
                for (uint32_t i = 0; i < count; i++) {
                    cumul += palette[i].weight;
                    if (roll < cumul) return i;
                }
                return count - 1;
            }

            // ── Tier Profile (mean+sigma, matches GoLTierProfile pattern) ────
            static constexpr uint32_t RIBBON_TIER_COUNT = 3;
            static constexpr float RIBBON_BASE_TIER_WEIGHTS[RIBBON_TIER_COUNT] = {
                0.45f, 0.30f, 0.25f
            };

            struct RibbonTierProfile {
                // ─── Geometry ────────────────────────────────────────────
                float cube_count_mean, cube_count_sigma;
                float cube_size_mean, cube_size_sigma;

                // ─── Altitude ────────────────────────────────────────────
                float height_mean, height_sigma;

                // ─── Lateral wave ─────────────────────────────────────
                float lateral_amp_mean, lateral_amp_sigma;
                float lateral_cycles_mean, lateral_cycles_sigma;
                float lateral_speed_mean, lateral_speed_sigma;

                // ─── Vertical wave ────────────────────────────────────
                float vertical_amp_mean, vertical_amp_sigma;
                float vertical_cycles_mean, vertical_cycles_sigma;
                float vertical_speed_mean, vertical_speed_sigma;

                // ─── Twist (corkscrew) ───────────────────────────────────
                float twist_amp_mean, twist_amp_sigma;
                float twist_cycles_mean, twist_cycles_sigma;
                float twist_speed_mean, twist_speed_sigma;

                // ─── Selection ───────────────────────────────────────────
                float weight;
            };

            //                          ┌── Serpentine ──┬──── Helix ─────┬─── Streamer ───┐
            //                          │   μ       σ    │   μ       σ    │   μ       σ    │
            // ─── Geometry ────────────┤                │                │                │
            //   cube_count             │ 350      60    │ 150      40    │ 250      50    │
            //   cube_size              │   8.0     2.0  │   5.0     1.2  │   6.0     1.6  │
            // ─── Altitude ────────────┤                │                │                │
            //   height                 │  60      15    │  55      12    │  70      20    │
            // ─── Lateral wave ────────┤                │                │                │
            //   lateral_amp             │  10.0     0.6  │   3.5     0.6  │   5.5     0.8  │
            //   lateral_cycles          │   1.5     0.4  │   3.0     0.8  │   2.0     0.5  │
            //   lateral_speed           │   0.25   0.075 │   0.60   0.175 │   0.40    0.10 │
            // ─── Vertical wave ───────┤  cycles + speed = lateral                        │
            //   vertical_amp            │   5.0     0.8  │   2.5     0.5  │   8.0     1.2  │
            // ─── Twist (corkscrew) ───┤  cycles + speed = lateral                        │
            //   twist_amp              │   0.6     0.2  │   6.0     0.8  │   1.6     0.6  │
            // ─── Selection ───────────┤                │                │                │
            //   weight                 │   0.45         │   0.30         │   0.25         │
            //                          └────────────────┴────────────────┴────────────────┘
            static constexpr RibbonTierProfile RIBBON_TIERS[RIBBON_TIER_COUNT] = {
                // Tier 0: Serpentine — long, massive, slow motion
                {   350.0f, 60.0f,      // cube_count
                      8.0f,  2.0f,      // cube_size
                     60.0f, 15.0f,      // height
                     10.0f,  0.6f,      // lateral_amp
                      1.5f,  0.4f,      // lateral_cycles
                      0.25f, 0.075f,    // lateral_speed
                      5.0f,  0.8f,      // vertical_amp
                      0.8f,  0.2f,      // vertical_cycles (overridden = lateral)
                      0.20f, 0.06f,     // vertical_speed (overridden = lateral)
                      0.6f,  0.2f,      // twist_amp
                      0.5f,  0.2f,      // twist_cycles (overridden = lateral)
                      0.15f, 0.05f,     // twist_speed (overridden = lateral)
                      0.45f },          // weight
                      // Tier 1: Helix — tighter cycles, visible corkscrew
                      {   150.0f, 40.0f,      // cube_count
                            5.0f,  1.2f,      // cube_size
                           55.0f, 12.0f,      // height
                            3.5f,  0.6f,      // lateral_amp
                            3.0f,  0.8f,      // lateral_cycles
                            0.60f, 0.175f,    // lateral_speed
                            2.5f,  0.5f,      // vertical_amp
                            2.5f,  0.6f,      // vertical_cycles (overridden = lateral)
                            0.50f, 0.15f,     // vertical_speed (overridden = lateral)
                            6.0f,  0.8f,      // twist_amp
                            2.0f,  0.5f,      // twist_cycles (overridden = lateral)
                            0.20f, 0.05f,     // twist_speed (overridden = lateral)
                            0.30f },          // weight
                            // Tier 2: Streamer — tall vertical form, deep breathing
                            {   250.0f, 50.0f,      // cube_count
                                  6.0f,  1.6f,      // cube_size
                                 70.0f, 20.0f,      // height
                                  5.5f,  0.8f,      // lateral_amp
                                  2.0f,  0.5f,      // lateral_cycles
                                  0.40f, 0.10f,     // lateral_speed
                                  8.0f,  1.2f,      // vertical_amp
                                  1.2f,  0.3f,      // vertical_cycles (overridden = lateral)
                                  0.325f, 0.075f,   // vertical_speed (overridden = lateral)
                                  1.6f,  0.6f,      // twist_amp
                                  1.5f,  0.4f,      // twist_cycles (overridden = lateral)
                                  0.25f, 0.08f,     // twist_speed (overridden = lateral)
                                  0.25f },          // weight
            };

            static constexpr const char* RIBBON_TIER_NAMES[] = {
                "Serpentine", "Helix", "Streamer"
            };
            static constexpr const char* RIBBON_COLOR_NAMES[] = {
                "smooth", "tinted", "contrast"
            };

            static constexpr float PAWN_HEIGHT_UNITS = 1.5f;     // matches WGSL PAWN_HEIGHT

            // ─── Generative Catenary Arches ──────────────────────────────────
            //
            // Three tiers: doorway, standard, monumental. Each defined by a
            // parameter row in the ARCH_TIERS matrix (one row = one character).
            //
            // Collision: two pier solids per arch, walkable via step-height.
            // Visual:    CPU-generated barrel vault mesh with per-vertex color.
            // Color:     default = warm sandstone; override = arch palette.
            //
            // Distribution: placeholder — piggybacks on patch streaming,
            // per-archetype probability. Will be replaced once the full
            // object vocabulary exists and placement rules emerge.
            //
            // ─── Pier Sizing Rule ────────────────────────────────────────────
            //
            //   Pier footprint is DERIVED from shell geometry, not independent:
            //     pier_half_x = thickness/2 + pier_padding + edge_blend
            //     pier_half_z = depth/2     + pier_padding + edge_blend
            //
            //   The artist controls pier_padding (extra margin beyond the shell
            //   base). Piers always cover the arch feet with room to spare.

            enum class ArchTier : uint32_t {
                DOORWAY = 0,   // human-scale passage
                STANDARD = 1,   // the arch we started with
                MONUMENTAL = 2,   // cathedral-scale gateway
                COUNT = 3
            };

            // ─── Tier Parameter Struct ───────────────────────────────────────
            //
            // Fields grouped by artistic concern:
            //   Shell:      span, rise, depth, thickness
            //   Piers:      pier_height, pier_padding, edge_blend
            //   Appearance: color_override, burial
            //   Quality:    segs_u, segs_v
            //   Selection:  weight (probability of this tier being chosen)

            // entities:K1 (Option B): ArchTierParams and the named-struct
            // ARCH_TIERS array migrated to entity_pipeline.inl as ArchTierRow
            // (struct, not the same as the ArchTier enum). To tune a tier
            // (or add one) edit ARCH_TIERS in entity_pipeline.inl.

            // ─── Color Palette ───────────────────────────────────────────────
            // Paradigm: palette + sandstone fallback.
            // When color_override fires, the arch draws from this palette
            // instead of terrain sandstone. Extend by adding rows.

            static constexpr float ARCH_PALETTE[][3] = {
                { 0.82f, 0.80f, 0.78f },   // 0: light grey stone
            };
            static constexpr uint32_t ARCH_PALETTE_COUNT = 1;

            // Default terrain-derived sandstone (used when no color override)
            static constexpr float ARCH_SANDSTONE_BASE[3] = { 0.75f, 0.68f, 0.60f };
            static constexpr float ARCH_SANDSTONE_VARIANCE = 0.04f;

            // ─── Spawn Configuration ─────────────────────────────────────────
            //
            // These control WHERE arches appear and are deliberately separate
            // from the tier matrix (which controls WHAT each arch looks like).
            // The spawn rules are a placeholder awaiting the full object vocabulary.

            struct ArchConfig {
                // Flat spawn probability — terrain-independent (themes control variation)
                static constexpr float SPAWN_CHANCE = 0.030f;

                // Per-mood spawn multiplier (Bayesian: prior × mood_factor × adjacency_factor)
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

                // Position jitter within patch (fraction of PATCH_EXTENT)
                static constexpr float POSITION_JITTER = 0.35f;
            };

            // ─── Property Indices ────────────────────────────────────────────
            //
            // Named constants for cpu_hash_f(seed, prop) → deterministic draw.
            // 600-series: decorrelated from ribbons (400) and galleries (500).

            struct ArchProp {
                static constexpr uint32_t SPAWN_ROLL = 600u;
                static constexpr uint32_t POSITION_X = 601u;
                static constexpr uint32_t POSITION_Z = 602u;
                static constexpr uint32_t ROTATION = 603u;
                static constexpr uint32_t TIER = 604u;
                static constexpr uint32_t SPAN = 610u;
                static constexpr uint32_t RISE = 611u;
                static constexpr uint32_t DEPTH = 612u;
                static constexpr uint32_t THICKNESS = 613u;
                static constexpr uint32_t PIER_HEIGHT = 614u;
                static constexpr uint32_t PIER_PADDING = 615u;
                static constexpr uint32_t EDGE_BLEND = 616u;
                static constexpr uint32_t COLOR_OVER = 620u;
                static constexpr uint32_t COLOR_VAR_R = 621u;
                static constexpr uint32_t COLOR_VAR_G = 622u;
                static constexpr uint32_t COLOR_VAR_B = 623u;
            };

            // ─── Active Arch Tracking ────────────────────────────────────────

            struct ActiveArch {
                int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
                int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
                // Pier slots derived from arch slot: PIER_ARCH_BASE + slot*2, +1
                bool active = false;
                bool draw_visible = true;    // false = mesh zeroed for distance culling

                // Geometry (for portal detection + mesh rebuild)
                float world_x = 0.0f, world_z = 0.0f;
                float rotation = 0.0f;               // facing angle (radians)
                float half_span = 0.0f;
                float total_height = 0.0f;            // pier_height + rise
                ArchTier tier = ArchTier::DOORWAY;

                // Cached mesh parameters (set at spawn, read by rebuild)
                float depth = 0.0f;
                float thickness = 0.0f;
                float rise = 0.0f;                    // catenary height above piers
                float pier_height = 0.0f;
                float burial = 0.0f;
                uint32_t segs_u = 16, segs_v = 4;
                float col_r = 0.75f, col_g = 0.68f, col_b = 0.60f;

                // Placement (computed once at spawn, immutable)
                float cached_ground_y = 0.0f;         // absolute pier-top Y for VS offset

                // Portal state
                bool is_portal = false;
                bool is_back_portal = false;
                uint32_t position_hash = 0;           // for crossing_seed
                PortalDestination destination{};      // world this portal leads to
            };

            ActiveArch activeArches_[Dim::MAX_ARCH_INSTANCES]{};
            uint32_t activeArchCount_ = 0;
            bool archMeshGenPending_ = false;  // true → dispatch GPU mesh gen
            bool lightsDirty_ = true;      // set true at init, cleared after first upload

            // ─── Generative Columns ──────────────────────────────────────────
            //
            // Three classical tiers: pillar, doric, ornate. Each defined by a
            // parameter row in the COLUMN_TIERS matrix. Mesh is a surface of
            // revolution from a profile curve: base layers → shaft (with taper
            // + entasis) → capital layers.
            //
            // Collision: one solid per column (square footprint with edge_blend).
            // Visual:    CPU-generated revolution mesh, per-vertex color.
            // Color:     default = warm sandstone; override = column palette.

            enum class ColumnTier : uint32_t {
                PILLAR = 0,        // thick sturdy post, minimal ornamentation
                DORIC = 1,         // classical proportions, no base, subtle taper
                ORNATE = 2,        // monumental, entasis, layered base + capital
                COUNT = 3
            };

            // ─── Generative Antennas ─────────────────────────────────────────
            //
            // Three tiers: antenna, squat, colossal. Tall posts with stacked
            // drum elements. Shares ColumnTierParams struct (field reuse:
            // base_layers=drum_count, base_height=drum_height,
            // base_overhang=drum_radius_overhang, capital_height=spacer_height).
            //
            // Shares ActiveColumn tracking, mesh gen pipeline, and GPU tier
            // indices (3/4/5) with classical columns.

            enum class AntennaTier : uint32_t {
                ANTENNA = 0,       // tall post with stacked drum elements
                SQUAT = 1,         // wider post + wider/shorter drums
                COLOSSAL = 2,      // massive tower-scale antenna
                COUNT = 3
            };

            // Tier counts (GPU tier indices: 0–2 column, 3–5 antenna)
            static constexpr uint32_t COLUMN_TIER_COUNT = static_cast<uint32_t>(ColumnTier::COUNT);
            static constexpr uint32_t ANTENNA_TIER_COUNT = static_cast<uint32_t>(AntennaTier::COUNT);

            // ─── Tier Parameter Struct ───────────────────────────────────────
            //
            // Fields grouped by artistic concern:
            //   Profile:    height, shaft_radius, taper, entasis
            //   Base:       base_layers, base_height, base_overhang
            //   Capital:    capital_layers, capital_height, capital_overhang
            //   Collision:  solid_radius_padding, solid_height, edge_blend
            //   Appearance: color_override, burial
            //   Quality:    segs_around, shaft_rings
            //   Selection:  weight

            // entities:K1 (Option B): ColumnTierParams plus the COLUMN_TIERS
            // and ANTENNA_TIERS arrays migrated to entity_pipeline.inl as
            // ColumnTierRow (struct, distinct from the ColumnTier enum). Both
            // tables use the same row struct since Antenna shares Column's
            // 13-param shape (with field reuse — base_layers = drum_count,
            // base_height = drum_height, etc.).

            // ─── Color Palette ───────────────────────────────────────────────
            // Paradigm: palette + sandstone fallback (same as arch).
            // When color_override fires, column draws from this palette.
            // Default: terrain-derived sandstone with per-instance variance.
            static constexpr float COLUMN_PALETTE[][3] = {
                { 0.82f, 0.80f, 0.78f },   // 0: light grey stone
                { 0.88f, 0.83f, 0.72f },   // 1: warm limestone (cream/ivory)
                { 0.55f, 0.58f, 0.63f },   // 2: cool blue-grey slate
                { 0.72f, 0.45f, 0.32f },   // 3: terracotta / burnt sienna
                { 0.90f, 0.87f, 0.82f },   // 4: weathered marble (off-white, subtle warmth)
                { 0.35f, 0.33f, 0.32f },   // 5: dark basalt
                { 0.52f, 0.58f, 0.48f },   // 6: mossy green-grey
            };
            static constexpr uint32_t COLUMN_PALETTE_COUNT = 7;
            static constexpr float COLUMN_SANDSTONE_BASE[3] = { 0.75f, 0.68f, 0.60f };
            static constexpr float COLUMN_SANDSTONE_VARIANCE = 0.04f;

            struct ColumnConfig {
                static constexpr float SPAWN_CHANCE = 0.030f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.35f;
            };

            struct ColumnProp {
                static constexpr uint32_t SPAWN_ROLL = 700u;
                static constexpr uint32_t POSITION_X = 701u;
                static constexpr uint32_t POSITION_Z = 702u;
                static constexpr uint32_t TIER = 703u;
                static constexpr uint32_t HEIGHT = 710u;
                static constexpr uint32_t SHAFT_RADIUS = 711u;
                static constexpr uint32_t TAPER = 712u;
                static constexpr uint32_t ENTASIS = 713u;
                static constexpr uint32_t BASE_LAYERS = 714u;
                static constexpr uint32_t BASE_HEIGHT = 715u;
                static constexpr uint32_t BASE_OVERHANG = 716u;
                static constexpr uint32_t CAPITAL_LAYERS = 720u;
                static constexpr uint32_t CAPITAL_HEIGHT = 721u;
                static constexpr uint32_t CAPITAL_OVERHANG = 722u;
                static constexpr uint32_t SOLID_PADDING = 730u;
                static constexpr uint32_t SOLID_HEIGHT = 731u;
                static constexpr uint32_t EDGE_BLEND = 732u;
                static constexpr uint32_t COLOR_OVER = 740u;
                static constexpr uint32_t COLOR_VAR_R = 741u;
                static constexpr uint32_t COLOR_VAR_G = 742u;
                static constexpr uint32_t COLOR_VAR_B = 743u;
            };

            struct AntennaConfig {
                static constexpr float SPAWN_CHANCE = 0.025f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.35f;
            };

            struct AntennaProp {
                static constexpr uint32_t SPAWN_ROLL = 900u;
                static constexpr uint32_t POSITION_X = 901u;
                static constexpr uint32_t POSITION_Z = 902u;
                static constexpr uint32_t TIER = 903u;
                static constexpr uint32_t HEIGHT = 910u;
                static constexpr uint32_t SHAFT_RADIUS = 911u;
                static constexpr uint32_t TAPER = 912u;
                static constexpr uint32_t ENTASIS = 913u;
                static constexpr uint32_t BASE_LAYERS = 914u;
                static constexpr uint32_t BASE_HEIGHT = 915u;
                static constexpr uint32_t BASE_OVERHANG = 916u;
                static constexpr uint32_t CAPITAL_LAYERS = 920u;
                static constexpr uint32_t CAPITAL_HEIGHT = 921u;
                static constexpr uint32_t CAPITAL_OVERHANG = 922u;
                static constexpr uint32_t SOLID_PADDING = 930u;
                static constexpr uint32_t SOLID_HEIGHT = 931u;
                static constexpr uint32_t EDGE_BLEND = 932u;
                static constexpr uint32_t COLOR_OVER = 940u;
                static constexpr uint32_t COLOR_VAR_R = 941u;
                static constexpr uint32_t COLOR_VAR_G = 942u;
                static constexpr uint32_t COLOR_VAR_B = 943u;
            };

            // ─── Active Column Tracking ──────────────────────────────────────

            struct ActiveColumn {
                int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
                int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
                // Pier slot derived from column slot: PIER_COLUMN_BASE + slot
                bool active = false;
                bool draw_visible = true;    // false = mesh zeroed for distance culling

                // Cached mesh parameters (set at spawn, read by rebuild)
                float world_x = 0.0f, world_z = 0.0f;
                float height = 0.0f;
                float shaft_radius = 0.0f;
                float taper = 1.0f;
                float entasis = 0.0f;
                uint32_t base_layers = 0;
                float base_height = 0.0f;
                float base_overhang = 0.0f;
                uint32_t cap_layers = 0;
                float cap_height = 0.0f;
                float cap_overhang = 0.0f;
                float solid_height = 0.0f;
                float burial = 0.0f;
                uint32_t segs_around = 12;
                uint32_t shaft_rings = 4;
                float col_r = 0.75f, col_g = 0.68f, col_b = 0.60f;
                uint32_t tier_idx = 0;
                // Antenna drum colors (cached for rebuild)
                float drum_colors[9] = {};  // 3 drums × RGB

                // Placement (computed once at spawn, immutable)
                float cached_ground_y = 0.0f;         // absolute pier-top Y for VS offset
            };

            ActiveColumn activeColumns_[Dim::MAX_COLUMN_ONLY]{};
            ActiveColumn activeAntennas_[Dim::MAX_ANTENNA_ONLY]{};
            uint32_t activeColumnCount_ = 0;
            uint32_t activeAntennaCount_ = 0;
            bool columnMeshGenPending_ = false;  // true → dispatch GPU mesh gen (shared by column + antenna)

            // (generate_column_mesh removed — replaced by GPU compute: column_mesh_gen)

            // ─── Generative Palms ────────────────────────────────────────────
            //
            // Three tiers: Sapling, Coastal, Royal. Tapered trunk with lean +
            // bark rings, crowned with radial fronds. No piers, no collision
            // solids, no heightfield contribution.

            enum class PalmTier : uint32_t { SAPLING = 0, COASTAL = 1, ROYAL = 2, COUNT = 3 };
            static constexpr uint32_t PALM_TIER_COUNT = static_cast<uint32_t>(PalmTier::COUNT);

            // entities:K1 (Option B): PalmTierParams and PALM_TIERS migrated
            // to entity_pipeline.inl as PalmTierRow.

            // ─── Color Palette ───────────────────────────────────────────────
            // Paradigm: body-part bases with per-instance variance.
            // Each part (trunk, frond, aged frond) has an independent RGB base.
            // Variance from tier's trunk_var / frond_var applied per-spawn.
            static constexpr float PALM_TRUNK_BASE[3] = { 0.45f, 0.35f, 0.25f };
            static constexpr float PALM_FROND_BASE[3] = { 0.25f, 0.45f, 0.20f };
            static constexpr float PALM_AGED_BASE[3] = { 0.35f, 0.38f, 0.18f };

            struct PalmConfig {
                static constexpr float SPAWN_CHANCE = 0.200f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.45f;
            };

            struct PalmProp {
                static constexpr uint32_t SPAWN_ROLL = 950u;
                static constexpr uint32_t POSITION_X = 951u;
                static constexpr uint32_t POSITION_Z = 952u;
                static constexpr uint32_t ROTATION = 953u;
                static constexpr uint32_t TIER = 954u;
                static constexpr uint32_t HEIGHT = 960u;
                static constexpr uint32_t BASE_R = 961u;
                static constexpr uint32_t TOP_R = 962u;
                static constexpr uint32_t LEAN = 963u;
                static constexpr uint32_t LEAN_DIR = 964u;
                static constexpr uint32_t BARK_RINGS = 965u;
                static constexpr uint32_t BARK_DEPTH = 966u;
                static constexpr uint32_t FROND_COUNT = 970u;
                static constexpr uint32_t FROND_LEN = 971u;
                static constexpr uint32_t FROND_WIDTH = 972u;
                static constexpr uint32_t FROND_DROOP = 973u;
                static constexpr uint32_t FROND_ARCH = 974u;
                static constexpr uint32_t CROWN_SPREAD = 975u;
                static constexpr uint32_t CROWN_SKIRT = 976u;
                static constexpr uint32_t SOLID_PADDING = 980u;
                static constexpr uint32_t EDGE_BLEND = 981u;
                static constexpr uint32_t COLOR_OVER = 990u;
                static constexpr uint32_t COLOR_VAR_R = 991u;
                static constexpr uint32_t COLOR_VAR_G = 992u;
                static constexpr uint32_t COLOR_VAR_B = 993u;
            };

            struct ActivePalm {
                int32_t patch_gx = 0, patch_gz = 0;
                int32_t host_gx = 0, host_gz = 0;
                bool active = false;
                bool draw_visible = true;
                float world_x = 0.0f, world_z = 0.0f;
                float height = 0.0f;
                float base_r = 0.0f;
                uint32_t tier_idx = 0;
                float cached_ground_y = 0.0f;
            };

            ActivePalm activePalms_[Dim::MAX_PALM_INSTANCES]{};
            uint32_t activePalmCount_ = 0;
            bool palmMeshGenPending_ = false;

            bool prepare_palm_mesh_gen(wgpu::Queue& queue) {
                if (!palmMeshGenPending_) return false;
                palmMeshGenPending_ = false;
                uint32_t maxSlot = 0;
                bool anyActive = false;
                for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
                    if (activePalms_[i].active) { maxSlot = i; anyActive = true; }
                }
                gpuState_.set_palm_index_count(anyActive
                    ? (maxSlot + 1) * Dim::PALMG_MAX_INDICES_PER_SLOT : 0);
                return true;
            }

            // ─── Generative Cacti ─────────────────────────────────────────────
            //
            // Three tiers: Finger, Saguaro, Candelabra. Ribbed columnar trunk
            // with optional forking arms. No piers, no collision solids, no
            // heightfield contribution.

            enum class CactusTier : uint32_t { FINGER = 0, SAGUARO = 1, CANDELABRA = 2, COUNT = 3 };
            static constexpr uint32_t CACTUS_TIER_COUNT = static_cast<uint32_t>(CactusTier::COUNT);

            // entities:K1 (Option B): CactusTierParams and CACTUS_TIERS
            // migrated to entity_pipeline.inl as CactusTierRow.

            // ─── Color Palette ───────────────────────────────────────────────
            // Paradigm: body-part bases with per-instance variance.
            // Trunk body and rib highlights have independent RGB bases.
            // Variance from tier's color_var applied per-spawn.
            static constexpr float CACTUS_BODY_BASE[3] = { 0.30f, 0.45f, 0.25f };
            static constexpr float CACTUS_RIB_BASE[3] = { 0.35f, 0.55f, 0.30f };

            struct CactusConfig {
                static constexpr float SPAWN_CHANCE = 0.100f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.35f;
            };

            struct CactusProp {
                static constexpr uint32_t SPAWN_ROLL = 1000u;
                static constexpr uint32_t POSITION_X = 1001u;
                static constexpr uint32_t POSITION_Z = 1002u;
                static constexpr uint32_t ROTATION = 1003u;
                static constexpr uint32_t TIER = 1004u;
                static constexpr uint32_t HEIGHT = 1010u;
                static constexpr uint32_t RADIUS = 1011u;
                static constexpr uint32_t TAPER = 1012u;
                static constexpr uint32_t RIBS = 1013u;
                static constexpr uint32_t RIB_DEPTH = 1014u;
                static constexpr uint32_t LEAN = 1015u;
                static constexpr uint32_t LEAN_DIR = 1016u;
                static constexpr uint32_t CAP_ROUND = 1017u;
                static constexpr uint32_t ARM_COUNT = 1020u;
                static constexpr uint32_t ARM_HEIGHT = 1021u;
                static constexpr uint32_t ARM_LENGTH = 1022u;
                static constexpr uint32_t ARM_RADIUS = 1023u;
                static constexpr uint32_t ARM_CURVE = 1024u;
                static constexpr uint32_t COLOR_OVER = 1030u;
                static constexpr uint32_t COLOR_VAR_R = 1031u;
                static constexpr uint32_t COLOR_VAR_G = 1032u;
                static constexpr uint32_t COLOR_VAR_B = 1033u;
            };

            struct ActiveCactus {
                int32_t patch_gx = 0, patch_gz = 0;
                int32_t host_gx = 0, host_gz = 0;
                bool active = false;
                bool draw_visible = true;
                float world_x = 0.0f, world_z = 0.0f;
                float height = 0.0f;
                float radius = 0.0f;
                uint32_t tier_idx = 0;
                float cached_ground_y = 0.0f;
            };

            ActiveCactus activeCacti_[Dim::MAX_CACTUS_INSTANCES]{};
            uint32_t activeCactusCount_ = 0;
            bool cactusMeshGenPending_ = false;

            // ─── Generative Blade Clusters ──────────────────────────────────
            //
            // Ground-level leaf clusters: 3-7 thick pointed blades from a
            // single ground point. Three tiers: Sprout / Clump / Thicket.
            // Geometry: flat quad strips along curved midribs, golden-angle packed.

            enum class BladeClusterTier : uint32_t { SPROUT = 0, CLUMP = 1, THICKET = 2, COUNT = 3 };
            static constexpr uint32_t BLADE_TIER_COUNT = static_cast<uint32_t>(BladeClusterTier::COUNT);

            // entities:K1 (Option B): BladeClusterTierParams and BLADE_TIERS
            // migrated to entity_pipeline.inl as BladeTierRow.

            // ─── Color Palette ───────────────────────────────────────────────
            // Paradigm: body-part bases with per-instance variance.
            // Fresh blade body and aged (dried) blade have independent RGB bases.
            // Variance from tier's color_var applied per-spawn.
            static constexpr float BLADE_BODY_BASE[3] = { 0.28f, 0.52f, 0.22f };
            static constexpr float BLADE_AGED_BASE[3] = { 0.48f, 0.45f, 0.28f };

            struct BladeClusterConfig {
                static constexpr float SPAWN_CHANCE = 0.025f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.30f;
            };

            struct BladeProp {
                static constexpr uint32_t SPAWN_ROLL = 1100u;
                static constexpr uint32_t POSITION_X = 1101u;
                static constexpr uint32_t POSITION_Z = 1102u;
                static constexpr uint32_t ROTATION = 1103u;
                static constexpr uint32_t TIER = 1104u;
                static constexpr uint32_t BLADE_COUNT = 1110u;
                static constexpr uint32_t HEIGHT = 1111u;
                static constexpr uint32_t HEIGHT_VAR = 1112u;
                static constexpr uint32_t WIDTH = 1113u;
                static constexpr uint32_t SPLAY = 1114u;
                static constexpr uint32_t CURVE = 1115u;
                static constexpr uint32_t TWIST = 1116u;
                static constexpr uint32_t TAPER = 1117u;
                static constexpr uint32_t COLOR_VAR_R = 1120u;
                static constexpr uint32_t COLOR_VAR_G = 1121u;
                static constexpr uint32_t COLOR_VAR_B = 1122u;
            };

            struct ActiveBlade {
                int32_t patch_gx = 0, patch_gz = 0;
                int32_t host_gx = 0, host_gz = 0;
                bool active = false;
                bool draw_visible = true;
                float world_x = 0.0f, world_z = 0.0f;
                float height = 0.0f;
                float radius = 0.0f;
                uint32_t tier_idx = 0;
                float cached_ground_y = 0.0f;
            };

            ActiveBlade activeBlades_[Dim::MAX_BLADE_INSTANCES]{};
            uint32_t activeBladeCount_ = 0;
            bool bladeMeshGenPending_ = false;

            bool prepare_blade_mesh_gen(wgpu::Queue& queue) {
                if (!bladeMeshGenPending_) return false;
                bladeMeshGenPending_ = false;
                uint32_t maxSlot = 0;
                bool anyActive = false;
                for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
                    if (activeBlades_[i].active) { maxSlot = i; anyActive = true; }
                }
                gpuState_.set_blade_index_count(anyActive
                    ? (maxSlot + 1) * Dim::BLADEG_MAX_INDICES_PER_SLOT : 0);
                return true;
            }

            bool prepare_cactus_mesh_gen(wgpu::Queue& queue) {
                if (!cactusMeshGenPending_) return false;
                cactusMeshGenPending_ = false;
                uint32_t maxSlot = 0;
                bool anyActive = false;
                for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
                    if (activeCacti_[i].active) { maxSlot = i; anyActive = true; }
                }
                gpuState_.set_cactus_index_count(anyActive
                    ? (maxSlot + 1) * Dim::CACTUSG_MAX_INDICES_PER_SLOT : 0);
                return true;
            }

            // ─── Generative Pyramids ─────────────────────────────────────────
            //
            // Three tiers: obelisk, temple, colossus. Each defined by a
            // parameter row in the PYRAMID_TIERS matrix.
            //
            // Collision: pyramid height function baked into heightfield.
            //   Pawn blocked by step-height on steep faces (no solid needed).
            // Visual:    CPU-generated 4-face (pointed) or 5-face (truncated) mesh.
            // Color:     sandstone (shared palette, distinct base tone).

            enum class PyramidTier : uint32_t {
                OBELISK = 0,     // tall narrow marker, pointed apex
                TEMPLE = 1,      // medium, truncated platform top
                COLOSSUS = 2,    // massive landmark, slight or no truncation
                COUNT = 3
            };

            // entities:K1 (Option B): PyramidTierParams and the named-struct
            // PYRAMID_TIERS array migrated to entity_pipeline.inl as a single
            // PyramidTier struct (TierProfile + extras). The PyramidTier enum
            // and all other pyramid vocabulary (color palette, spawn config,
            // prop registry, active-instance tracking) stay here.

            // ─── Color Palette ───────────────────────────────────────────────
            // Paradigm: sandstone base with per-instance variance (no palette).
            // All pyramids derive color from a single sandstone RGB base.
            static constexpr float PYRAMID_SANDSTONE_BASE[3] = { 0.80f, 0.72f, 0.58f };
            static constexpr float PYRAMID_SANDSTONE_VARIANCE = 0.05f;

            struct PyramidConfig {
                // Flat spawn probability — terrain-independent (themes control variation)
                static constexpr float SPAWN_CHANCE = 0.030f;
                static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f };
                static constexpr float POSITION_JITTER = 0.25f;
            };

            struct PyramidProp {
                static constexpr uint32_t SPAWN_ROLL = 800u;
                static constexpr uint32_t POSITION_X = 801u;
                static constexpr uint32_t POSITION_Z = 802u;
                static constexpr uint32_t ROTATION = 803u;
                static constexpr uint32_t TIER = 804u;
                static constexpr uint32_t HEIGHT = 810u;
                static constexpr uint32_t BASE_HALF = 811u;
                static constexpr uint32_t ASPECT = 812u;
                static constexpr uint32_t TRUNCATION = 813u;
                static constexpr uint32_t EDGE_BLEND = 814u;
                static constexpr uint32_t COLOR_OVER = 820u;
                static constexpr uint32_t COLOR_VAR_R = 821u;
                static constexpr uint32_t COLOR_VAR_G = 822u;
                static constexpr uint32_t COLOR_VAR_B = 823u;
            };

            // ─── Active Pyramid Tracking ─────────────────────────────────────

            struct ActivePyramid {
                int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
                int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
                bool active = false;
                // Cached color (set at spawn, read by rebuild)
                float col_r = 0.80f, col_g = 0.72f, col_b = 0.58f;

                // Placement (computed once at spawn, immutable)
                float cached_ground_y = 0.0f;         // absolute base Y for VS offset
            };

            ActivePyramid activePyramids_[Dim::MAX_PYRAMID_INSTANCES]{};
            uint32_t activePyramidCount_ = 0;
            bool pyramidMeshGenPending_ = false;  // true → dispatch GPU mesh gen

            // CPU mirror of GPU pyramid instances (for heightfield baking)
            GPUPyramidArray cpuPyramids_{};

            // (generate_pyramid_mesh removed — replaced by GPU compute: pyramid_mesh_gen)

