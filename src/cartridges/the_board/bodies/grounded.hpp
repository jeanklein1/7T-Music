#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // Dim::*, GPUPyramidArray, wgpu
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT, PortalDestination, WORLD_DRAW_LIVE (the portal palette), portal_color_for
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"     // queue types (the clean three's funnel signatures)

// ─── grounded.hpp (HEADER: the grounded-seven registry) ──────────
//
// Vocabulary for the grounded entity families that share the generic dispatch pipeline.
//
// SEAM[entities:P10] this block is the canonical home of pattern P10
//   (per-family vocabulary block). Seven family applications follow.
//   Each block has the same structural template: TierEnum / Color
//   palette / Config / Prop registry / Active tracking. Don't fight
//   the cookie-cutter — it's intentional specificity per family.
// SEAM[entities:taxonomy] this block holds vocabulary for the seven
//   grounded families that share machine/entity_pipeline.hpp. Sphere/Cube
//   vocabulary is in floaters.hpp; Ribbon and GoL are complete
//   subsystems in their own files.
// Tier sampling profiles + extras live as a single per-family TierRow
//   struct in machine/entity_pipeline.hpp (single source of truth, no
//   converters, no derived tables). The tier enum classes (ArchTier,
//   ColumnTier, etc.) stay here — they're indexing semantics, not data.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>   // std::max (portal-arch burial floor)   // (impl, merged)
#include <cmath>       // std::floor, std::cos, std::sin (portal-arch placement)   // (impl, merged)
#include <cstdint>   // (impl, merged)

namespace t7 {
namespace the_board {

// ═══ SHARED CONSTANTS ════════════════════════════════════════════

// STATUS: LATENT[unused] — zero callers in the tree;
// declared as the CPU mirror of WGSL PAWN_HEIGHT. Kept per flag-don't-
// delete; revive-or-delete when the pawn-height coupling is next worked.
inline constexpr float PAWN_HEIGHT_UNITS = 1.5f;     // matches WGSL PAWN_HEIGHT

// ═══ THE SPREAD LAW (MOSAIC_2) ═══════════════════════════════════
//
// The terrain's sorted-variance shape at the body's rate: raffle a
// median, then raffle how far THIS body may sit from it. A fixed ±3%
// puts every body the same distance from its color, which is why a
// hundred plain arches read as one stone. A raffled spread makes some
// bodies nearly flat and others strongly mottled — that is what
// "varied" means, and it is the terrain's own law (var = base +
// raw·span) at a smaller number.
//
// The span sits well under the terrain's 0.23 deliberately: a region is
// a field the eye averages across, while a body wears its whole spread
// at once. 0.16 is a body reading as its palette color in a different
// light; 0.23 is a body reading as a different color.
inline constexpr float ENTITY_SPREAD_BASE = 0.02f;
inline constexpr float ENTITY_SPREAD_SPAN = 0.16f;
// ═══ VOCABULARY: ARCH ════════════════════════════════════════════

enum class ArchTier : uint32_t {
    DOORWAY = 0,   // human-scale passage
    STANDARD = 1,   // the arch we started with
    MONUMENTAL = 2,   // cathedral-scale gateway
    COUNT = 3
};

// ── Color Palette ────────────────────────────────────────────────
// A QUARRIED vocabulary (MOSAIC_2d), deliberately distinct from the two
// sets already in the tree: COLUMN_PALETTE is painted (sky, coral,
// lavender), MOSAIC_MEDIANS is ceramic. Index 0 is the incumbent grey
// unchanged, so every existing arch that rolled palette keeps a color
// it could already have had. ARCH_SANDSTONE_BASE is deliberately NOT
// duplicated here — it has one home.
inline constexpr float ARCH_PALETTE[][3] = {
    { 0.82f, 0.80f, 0.78f },   // 0: grey stone   (the incumbent)
    { 0.88f, 0.85f, 0.76f },   // 1: limestone
    { 0.28f, 0.28f, 0.30f },   // 2: basalt
    { 0.72f, 0.42f, 0.30f },   // 3: terracotta
    { 0.80f, 0.62f, 0.32f },   // 4: ochre
    { 0.44f, 0.62f, 0.56f },   // 5: verdigris
    { 0.48f, 0.30f, 0.34f },   // 6: porphyry
    { 0.92f, 0.90f, 0.86f },   // 7: chalk
};
inline constexpr uint32_t ARCH_PALETTE_COUNT = 8;
inline constexpr float ARCH_SANDSTONE_BASE[3] = { 0.75f, 0.68f, 0.60f };

// ── Spawn Configuration ──────────────────────────────────────────
struct ArchConfig {
    static constexpr float SPAWN_CHANCE = 0.050f;
    // Per-mood spawn multiplier (Bayesian: prior × mood_factor × adjacency_factor)
    // Position jitter within patch (fraction of Dim::PATCH_EXTENT)
    static constexpr float POSITION_JITTER = 0.35f;
};

// ── Property Index Registry ──────────────────────────────────────
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
    static constexpr uint32_t MOSAIC_ROLL = 650u;   // MOSAIC_2: the tier's ceramic roll
    static constexpr uint32_t MOSAIC_SEED = 651u;   // MOSAIC_1: 16-bit paint identity
    static constexpr uint32_t COLOR_SPREAD = 652u;  // MOSAIC_2: how far THIS body sits from its median
};

// ── Active Arch Tracking ─────────────────────────────────────────
struct ActiveArch {
    int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
    int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
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
    float cached_ground_y = 0.0f;         // absolute ground Y for VS offset

    // Portal state
    bool is_portal = false;
    bool is_back_portal = false;
    uint32_t position_hash = 0;           // for crossing_seed
    PortalDestination destination{};      // world this portal leads to

    uint32_t mosaic_seed = 0;   // MOSAIC_1 — frozen at spawn; 0 = plain
};

// ═══ VOCABULARY: COLUMN ══════════════════════════════════════════

enum class ColumnTier : uint32_t {
    PILLAR = 0,        // thick sturdy post, minimal ornamentation
    DORIC = 1,         // classical proportions, no base, subtle taper
    ORNATE = 2,        // monumental, entasis, layered base + capital
    COUNT = 3
};
inline constexpr uint32_t COLUMN_TIER_COUNT = static_cast<uint32_t>(ColumnTier::COUNT);

// ── Color Palette ────────────────────────────────────────────────
inline constexpr float COLUMN_PALETTE[][3] = {
    { 0.937f, 0.902f, 0.831f },   // 0: sand      #EFE6D4
    { 0.882f, 0.827f, 0.714f },   // 1: bone      #E1D3B6
    { 0.129f, 0.118f, 0.110f },   // 2: ink       #211E1C
    { 0.353f, 0.333f, 0.298f },   // 3: ink-soft  #5A554C
    { 0.431f, 0.608f, 0.753f },   // 4: sky       #6E9BC0
    { 0.863f, 0.596f, 0.482f },   // 5: coral     #DC987B
    { 0.878f, 0.635f, 0.306f },   // 6: gold      #E0A24E
    { 0.616f, 0.631f, 0.467f },   // 7: olive     #9DA177
    { 0.635f, 0.620f, 0.686f },   // 8: lavender  #A29EAF
    { 0.753f, 0.325f, 0.184f },   // 9: orb       #C0532F
};
inline constexpr uint32_t COLUMN_PALETTE_COUNT = 10;
inline constexpr float COLUMN_SANDSTONE_BASE[3] = { 0.75f, 0.68f, 0.60f };
// MOSAIC_2d: the column's own paths took the spread law, but this
// survives with ONE reader — antenna_compute_colors, which the campaign
// deliberately leaves alone (three decorrelated drums is a different
// concept from one body color; the ruling is Horizon's).
inline constexpr float COLUMN_SANDSTONE_VARIANCE = 0.04f;

// ── Spawn Configuration ──────────────────────────────────────────
struct ColumnConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    static constexpr float POSITION_JITTER = 0.35f;
};

// ── Property Index Registry ──────────────────────────────────────
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
    static constexpr uint32_t COLOR_SPREAD = 744u;  // MOSAIC_2: how far THIS body sits from its median
    static constexpr uint32_t MOSAIC_ROLL = 750u;   // MOSAIC_2: the tier's ceramic roll
    static constexpr uint32_t MOSAIC_SEED = 751u;   // MOSAIC_1: 16-bit paint identity
};

// ═══ VOCABULARY: ANTENNA ═════════════════════════════════════════

enum class AntennaTier : uint32_t {
    ANTENNA = 0,       // tall post with stacked drum elements
    SQUAT = 1,         // wider post + wider/shorter drums
    COLOSSAL = 2,      // massive tower-scale antenna
    COUNT = 3
};
inline constexpr uint32_t ANTENNA_TIER_COUNT = static_cast<uint32_t>(AntennaTier::COUNT);

// ── Spawn Configuration ──────────────────────────────────────────
struct AntennaConfig {
    static constexpr float SPAWN_CHANCE = 0.025f;
    static constexpr float POSITION_JITTER = 0.35f;
};

// ── Property Index Registry ──────────────────────────────────────
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

// ── Active Column Tracking (shared by Column and Antenna) ────────
struct ActiveColumn {
    int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
    int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
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
    float cached_ground_y = 0.0f;         // absolute ground Y for VS offset

    uint32_t mosaic_seed = 0;   // MOSAIC_1 — frozen at spawn; 0 = plain
};

// ═══ VOCABULARY: PALM ════════════════════════════════════════════

enum class PalmTier : uint32_t { SAPLING = 0, COASTAL = 1, ROYAL = 2, COUNT = 3 };
inline constexpr uint32_t PALM_TIER_COUNT = static_cast<uint32_t>(PalmTier::COUNT);

// ── Color Palette ────────────────────────────────────────────────
inline constexpr float PALM_TRUNK_BASE[3] = { 0.45f, 0.35f, 0.25f };
inline constexpr float PALM_FROND_BASE[3] = { 0.25f, 0.45f, 0.20f };
inline constexpr float PALM_AGED_BASE[3] = { 0.35f, 0.38f, 0.18f };

// ── Spawn Configuration ──────────────────────────────────────────
struct PalmConfig {
    static constexpr float SPAWN_CHANCE = 0.200f;
    static constexpr float POSITION_JITTER = 0.45f;
};

// ── Property Index Registry ──────────────────────────────────────
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

// ── Active Palm Tracking ─────────────────────────────────────────
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

// ═══ VOCABULARY: PYRAMID ═════════════════════════════════════════

enum class PyramidTier : uint32_t {
    OBELISK = 0,     // tall narrow marker, pointed apex
    TEMPLE = 1,      // medium, truncated platform top
    COLOSSUS = 2,    // massive landmark, slight or no truncation
    COUNT = 3
};

// ── Color Palette ────────────────────────────────────────────────
// Sandstone base only. All pyramids derive color from a single base.
inline constexpr float PYRAMID_SANDSTONE_BASE[3] = { 0.80f, 0.72f, 0.58f };
inline constexpr float PYRAMID_SANDSTONE_VARIANCE = 0.05f;

// ── Spawn Configuration ──────────────────────────────────────────
struct PyramidConfig {
    static constexpr float SPAWN_CHANCE = 0.030f;
    static constexpr float POSITION_JITTER = 0.25f;
};

// ── Property Index Registry ──────────────────────────────────────
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

// ── Active Pyramid Tracking ──────────────────────────────────────
struct ActivePyramid {
    int32_t patch_gx = 0, patch_gz = 0;   // trigger patch (idempotency)
    int32_t host_gx = 0, host_gz = 0;     // actual patch covering entity position (eviction)
    bool active = false;
    // Cached color (set at spawn, read by rebuild)
    float col_r = 0.80f, col_g = 0.72f, col_b = 0.58f;

    // Placement (computed once at spawn, immutable)
    float cached_ground_y = 0.0f;         // absolute base Y for VS offset
};

// ═══ ENTITIES MODULE STATE ════════════════════════════════════════

struct EntitiesState {
    // ── Arch ─────────────────────────────────────────────────────
    ActiveArch arches[Dim::MAX_ARCH_INSTANCES]{};
    bool       arch_mesh_gen_pending = false;
    float      arch_mesh_gen_since = -1.0f;   // the settle's stamp (PANORAMA_1)

    // ── Column + Antenna (sibling families, shared mesh-gen flag) ─
    ActiveColumn columns[Dim::MAX_COLUMN_ONLY]{};
    ActiveColumn antennas[Dim::MAX_ANTENNA_ONLY]{};
    bool         column_mesh_gen_pending = false;  // shared by column + antenna
    float        column_mesh_gen_since = -1.0f;

    // ── Palm ─────────────────────────────────────────────────────
    ActivePalm palms[Dim::MAX_PALM_INSTANCES]{};
    bool       palm_mesh_gen_pending = false;
    float      palm_mesh_gen_since = -1.0f;

    // ── Pyramid ──────────────────────────────────────────────────
    ActivePyramid   pyramids[Dim::MAX_PYRAMID_INSTANCES]{};
    GPUPyramidArray cpu_pyramids{};                 // CPU mirror for heightfield baking
};

// ═══ MESH-GEN PREPARERS — DECLARATIONS ════════════════════════════

bool prepare_palm_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue);
bool prepare_column_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue);
bool prepare_arch_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue);

// ═══ THE EVICTORS — DECLARATIONS ═══════════════════════════════════

void evict_pyramid(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void evict_arch(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void evict_column(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void evict_antenna(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void evict_palm(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
// Dispatch funnels for the clean one (table-shaped; defined below
// beside its recipes)
bool dispatch_select_palm_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_palm_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_palm_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue);
void teardown_entities(MachineCtx* c, wgpu::Queue& queue);

// ═══ THE ARCH FORCE-SPAWN AUTHOR (the portal channel) ═══════════
//
uint32_t force_spawn_portal_arch(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue,
    float cx, float cz, float rotation,
    const PortalDestination& dest, bool is_back_portal);

// ═══ IMPL:
// bodies deref EntitiesState(own) + World/Mood/GPU via MachineCtx; no
// Cartridge. COHORT: after contracts/spawn_services.hpp (generic_* +
// preamble DECLS — the machine bodies ride the cohort tail) +
// patch_system.hpp (WorldState, find_patch) + mood.hpp.

// ═══ MESH-GEN PREPARERS ═══════════════════════════════════════════

// THE SETTLE (PANORAMA_1). A family regenerates at most once per
// MESH_GEN_SETTLE_FRAMES outside a world's birth. A crossing raises
// `pending` on several consecutive frames; without this each raise was a
// whole-family rebake (4–8 ms on the floor device), so a burst of spawns
// bought a burst of firings. What arrives late arrives at the ring's edge,
// materializing through the icing, where a ~130 ms delay is invisible; what
// leaves late leaves beyond the ring, where it is already veiled — eviction
// is radius-driven and fires outside the render window by construction.
//
// THE BIRTH BURST BYPASSES IT, and `world_young` is the whole signal: it
// boots true, `reset_surface` re-raises it on every rebirth BEFORE
// teardown_entities runs in the same block, and `request_recenter` raises it
// on a radius change. So a boot, a portal and a re-centre all regenerate at
// once — behind the veil or behind the fade — and the settle governs only
// the steady world, which is the only place it was ever paying for anything.
//
// STAMPED AT FIRST SIGHT, NOT AT THE RAISE. The preparers run every frame, so
// the first frame that observes a raise is the raise's own frame or the next;
// against a settle of eight that is exact enough, and it puts the stamp in ONE
// place instead of at twenty raise sites, where a new raiser could forget it.
//
// PACED IN SECONDS, BECAUSE THE CLAIM WAS ALWAYS A TIME CLAIM (WRAP_0 U6).
// PANORAMA_1 wrote this as eight FRAMES and argued it in milliseconds — "a
// ~130 ms delay is invisible" — which are the same number only at 60 Hz. At
// `?pace=2` a frame is 33 ms and the window silently became ~266 ms. The
// constant now reads as what the argument always was, and no pace can double
// it.
inline constexpr float MESH_GEN_SETTLE_S = 0.133f;

inline bool mesh_gen_settled(bool& pending, float& since, const TimeState& ts,
                             const WorldState& ws) {
    if (!pending) return false;
    if (since < 0.0f) since = ts.seconds;
    if (!ws.world_young && ts.seconds - since < MESH_GEN_SETTLE_S) return false;
    pending = false;
    since = -1.0f;
    return true;
}


inline bool prepare_palm_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!mesh_gen_settled(es.palm_mesh_gen_pending, es.palm_mesh_gen_since,
                          c->time_state_, c->world_state_)) return false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
        if (es.palms[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_palm_index_count(anyActive
        ? (maxSlot + 1) * Dim::PALMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_column_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!mesh_gen_settled(es.column_mesh_gen_pending, es.column_mesh_gen_since,
                          c->time_state_, c->world_state_)) return false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        if (es.columns[i].active) { maxSlot = i; anyActive = true; }
    }
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        if (es.antennas[i].active) {
            maxSlot = i + Dim::ANTENNA_SLOT_OFFSET;
            anyActive = true;
        }
    }
    c->gpuState_.set_column_index_count(anyActive
        ? (maxSlot + 1) * Dim::CMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_arch_mesh_gen(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue) {
    (void)queue;
    if (!mesh_gen_settled(es.arch_mesh_gen_pending, es.arch_mesh_gen_since,
                          c->time_state_, c->world_state_)) return false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (es.arches[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_arch_index_count(anyActive
        ? (maxSlot + 1) * Dim::AMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

// ═══ THE ARCH FORCE-SPAWN AUTHOR (the portal channel) ═══════════

inline uint32_t force_spawn_portal_arch(EntitiesState& es, MachineCtx* c, wgpu::Queue& queue,
    float cx, float cz, float rotation,
    const PortalDestination& dest, bool is_back_portal) {
    // ROSTER-GATE portal (b) — THE SECOND DOOR. Portals force-spawn arches
    // directly (bypassing FAMILY_DISPATCH — R3), so this is the
    // single choke point every portal spawner routes through (back, finite,
    // future). Disabled: spawn nothing (no arch, no mesh-pending),
    // return the no-free-slot sentinel so callers treat it as "none placed".
    // HOME (K4): MIGRATED here from mood's force_spawn_portal_at. The
    // door's written retirement condition ("when mood converts and
    // force-spawn becomes a request channel, this door MIGRATES INTO
    // that channel") RESOLVED at REQUEST_1: the request channel exists —
    // for the RIBBON, the author the prophecy was really about. This
    // door deliberately does NOT migrate: a portal is the transition's
    // own machinery (the way in and the way out), not a patient request,
    // and it already claims lawfully (register_footprint below). The
    // arch's owner holds the door to the arch's storage; mood computes
    // values upstream.
    if constexpr (!ROSTER.portal) { (void)queue; (void)cx; (void)cz; (void)rotation; (void)dest; (void)is_back_portal; return UINT32_MAX; }

    uint32_t slot = UINT32_MAX;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (!es.arches[i].active) { slot = i; break; }
    }
    if (slot == UINT32_MAX) return UINT32_MAX;

    const auto& tp = ARCH_TIERS[static_cast<uint32_t>(ArchTier::DOORWAY)];
    float half_span = tp.profile.params[ArchIdx::SPAN].mean * 0.5f;
    float rise = tp.profile.params[ArchIdx::RISE].mean;
    float depth = tp.profile.params[ArchIdx::DEPTH].mean;
    float thickness = tp.profile.params[ArchIdx::THICKNESS].mean;
    float pier_height = tp.profile.params[ArchIdx::PIER_HEIGHT].mean;

    int32_t gx = static_cast<int32_t>(std::floor(cx / Dim::PATCH_EXTENT));
    int32_t gz = static_cast<int32_t>(std::floor(cz / Dim::PATCH_EXTENT));

    auto& aa = es.arches[slot];
    aa.patch_gx = gx;
    aa.patch_gz = gz;
    // RULING 20, and it must land BEFORE the registration below. This channel
    // never wrote the host patch — the slot kept whatever the previous arch
    // left there, so registering first would have filed the portal's ground
    // under an arbitrary patch, and released it when THAT patch evicted. The
    // generic path has always written both (entity_pipeline.hpp): patch_* is
    // the trigger, host_* is the patch actually covering the body. Here they
    // coincide, because a forced portal has no trigger patch — gx/gz above are
    // derived from its own world position.
    aa.host_gx = gx;
    aa.host_gz = gz;
    aa.active = true;
    aa.draw_visible = true;
    aa.world_x = cx;
    aa.world_z = cz;
    aa.rotation = rotation;
    aa.half_span = half_span;
    aa.total_height = pier_height + rise;
    aa.tier = ArchTier::DOORWAY;
    aa.depth = depth;
    aa.thickness = thickness;
    aa.rise = rise;
    aa.pier_height = pier_height;
    aa.burial = std::max(0.2f, pier_height * tp.burial);
    aa.segs_u = tp.segs_u;
    aa.segs_v = tp.segs_v;
    // PORTAL_1 C4/C5 — the one home, in this channel too. This function only
    // ever makes portals, so it is its own decision point: col_* takes the
    // destination's colour here, and meshParams below reads col_* rather
    // than a value handed in. The sandstone literal that stood here was the
    // mirrored disease — after C3c deleted the recomputing branch in
    // build_arch_mesh_params, it would have cemented every forced portal at
    // its first ring toggle.
    // Derived from the PARAMETERS, not from aa.destination / aa.is_back_portal
    // — those are written below, at the portal-state block.
    const float* pc = portal_color_for(dest, is_back_portal);
    aa.col_r = pc[0];  aa.col_g = pc[1];  aa.col_b = pc[2];

    {
        // GPU compute_entity_placement handles ground_y from heightfield
        aa.cached_ground_y = 0.0f;
    }

    aa.mosaic_seed = 0;   // MOSAIC_1: portals are functional markers — always plain (recycled slot must not inherit)
    aa.is_portal = true;
    aa.is_back_portal = is_back_portal;
    aa.position_hash = cpu_hash(static_cast<uint32_t>(cx * 73856093.0f), static_cast<uint32_t>(cz * 19349663.0f));
    aa.destination = dest;

    // RULING 14 — CHANNEL B ONLY. A portal you can walk a pyramid into is a
    // real artifact and claims real ground. Channel A (generic, dispatch-
    // spawned) already registers through negotiate_position, and with
    // WORLD_DRAW_LIVE.portal_density at 1.0 every DOORWAY-tier arch through dispatch IS a
    // portal — so this is the one channel that claimed nothing. The census
    // measured the gap exactly twice: arch delta -2 in a world with two forced
    // portals, -4 in a world with four.
    //
    // NO check_position, deliberately. This is a FORCE-spawn: the transition
    // machinery has already chosen where the portal goes and the arch is
    // committed by this point. It claims its ground; it does not ask for it.
    // A full registry therefore cannot deny the portal — the loud line inside
    // register_footprint reports it and the portal stands anyway, which is the
    // correct trade for the one family the world cannot do without.
    (void)register_footprint(c, cx, cz, half_span,
        gx, gz, PopFamily::ARCH, slot, static_cast<uint32_t>(ArchTier::DOORWAY));

    GPUArchMeshParams meshParams{};
    meshParams.center_x = cx;
    meshParams.center_z = cz;
    meshParams.rotation = rotation;
    meshParams.half_span = half_span;
    meshParams.rise = rise;
    meshParams.depth = depth;
    meshParams.thickness = thickness;
    meshParams.pier_height = pier_height;
    meshParams.burial = aa.burial;
    meshParams.catenary_a = solve_catenary_a(half_span, rise);
    meshParams.segs_u = tp.segs_u;
    meshParams.segs_v = tp.segs_v;
    meshParams.color_r = aa.col_r;
    meshParams.color_g = aa.col_g;
    meshParams.color_b = aa.col_b;
    meshParams.is_active = 1;
    c->gpuState_.upload_arch_mesh_params_slot(queue, slot, meshParams);
    es.arch_mesh_gen_pending = true;

    return slot;
}

// ═══ THE EVICTORS ═════════════════════════════════════════════════

inline void evict_pyramid(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::PYRAMID, slot);   // the hand that claims is the hand that frees
    self->entities_state_.cpu_pyramids.instances[slot] = GPUPyramidInstance{};
    self->entities_state_.pyramids[slot].active = false;
    self->world_state_.ground_entries_dirty = true;

    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        if (self->entities_state_.pyramids[i].active) max_idx = i + 1;
    }
    self->entities_state_.cpu_pyramids.count = max_idx;
    self->gpuState_.upload_pyramids(queue, self->entities_state_.cpu_pyramids);
}

inline void evict_arch(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::ARCH, slot);   // the hand that claims is the hand that frees
    self->entities_state_.arches[slot].active = false;
    // THE STALE-DESTINATION LATCH. A freed slot keeps its bytes, and the
    // spawn preamble re-reserves by setting active=true ALONE
    // (run_spawn_preamble) — arch_write_active rewrites the portal
    // fields only at commit. In that window an evicted portal's
    // is_portal/destination would read as a LIVE portal on the reserved
    // slot to every active && is_portal scan (upload_portal_array, the
    // trigger validation, the census, the door-guarantee gate). Portal
    // identity dies with the arch.
    self->entities_state_.arches[slot].is_portal = false;
    self->entities_state_.arches[slot].is_back_portal = false;
    self->entities_state_.arches[slot].destination = PortalDestination{};
    self->mood_state_.portals_dirty = true;
    { GPUArchMeshParams ep{}; self->gpuState_.upload_arch_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.arch_mesh_gen_pending = true;
}

inline void evict_column(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::COLUMN, slot);   // the hand that claims is the hand that frees
    self->entities_state_.columns[slot].active = false;
    { GPUColumnMeshParams ep{}; self->gpuState_.upload_column_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.column_mesh_gen_pending = true;
}

inline void evict_antenna(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::ANTENNA, slot);   // the hand that claims is the hand that frees
    uint32_t gpu_slot = slot + Dim::ANTENNA_SLOT_OFFSET;
    self->entities_state_.antennas[slot].active = false;
    { GPUColumnMeshParams ep{}; self->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, ep); }
    self->entities_state_.column_mesh_gen_pending = true;
}

inline void evict_palm(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue)
{
    unregister_footprint_for(self, PopFamily::PALM, slot);   // the hand that claims is the hand that frees
    self->entities_state_.palms[slot].active = false;
    { GPUPalmMeshParams ep{}; self->gpuState_.upload_palm_mesh_params_slot(queue, slot, ep); }
    self->entities_state_.palm_mesh_gen_pending = true;
    self->world_state_.ground_entries_dirty = true;
}

// ═══ THE CLEAN ONE — PALM RECIPES ════════════════════════════════
//
// Per-family tier tables, traits, adapters, and dispatch funnels.
// Each funnels into the machine's generic three-phase verbs via
// MachineCtx; the table rows point here (FAMILY_DISPATCH,
// cartridge.hpp post-class).

// ═══ FAMILY: PALM ═════════════════════════════════════════════════

struct PalmIdx {
    static constexpr uint32_t HEIGHT       = 0;
    static constexpr uint32_t BASE_R       = 1;
    static constexpr uint32_t TOP_R        = 2;
    static constexpr uint32_t LEAN         = 3;
    static constexpr uint32_t LEAN_DIR     = 4;
    static constexpr uint32_t BARK_RINGS   = 5;
    static constexpr uint32_t BARK_DEPTH   = 6;
    static constexpr uint32_t FROND_COUNT  = 7;
    static constexpr uint32_t FROND_LEN    = 8;
    static constexpr uint32_t FROND_WIDTH  = 9;
    static constexpr uint32_t FROND_DROOP  = 10;
    static constexpr uint32_t FROND_ARCH   = 11;
    static constexpr uint32_t CROWN_SPREAD = 12;
    static constexpr uint32_t CROWN_SKIRT  = 13;
    static constexpr uint32_t SOLID_PAD    = 14;
    static constexpr uint32_t EDGE_BLEND   = 15;
    static constexpr uint32_t COUNT        = 16;
};

inline constexpr TierParamDef PALM_PARAM_DEFS[] = {
    { PalmProp::HEIGHT,       2.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::BASE_R,       0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::TOP_R,        0.05f, 1e30f, false, ParamDist::GAUSSIAN },
    { PalmProp::LEAN,         0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::LEAN_DIR,     0.0f, 1e30f,  false, ParamDist::UNIFORM_TAU },
    { PalmProp::BARK_RINGS,   3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { PalmProp::BARK_DEPTH,   0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_COUNT,  3.0f, 1e30f,  true,  ParamDist::GAUSSIAN },
    { PalmProp::FROND_LEN,    1.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_WIDTH,  0.3f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_DROOP,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::FROND_ARCH,   0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::CROWN_SPREAD, 0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::CROWN_SKIRT,  0.0f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::SOLID_PADDING,0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
    { PalmProp::EDGE_BLEND,   0.1f, 1e30f,  false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t PALM_PARAM_COUNT = sizeof(PALM_PARAM_DEFS) / sizeof(TierParamDef);
static_assert(PALM_PARAM_COUNT == PalmIdx::COUNT,
    "F-4: PALM_PARAM_DEFS must cover PalmIdx exactly (row order IS the index)");

// params[] order MUST match PALM_PARAM_DEFS:
//   [0]HEIGHT [1]BASE_R [2]TOP_R [3]LEAN [4]LEAN_DIR(uniform — {0,0})
//   [5]BARK_RINGS [6]BARK_DEPTH [7]FROND_COUNT [8]FROND_LEN
//   [9]FROND_WIDTH [10]FROND_DROOP [11]FROND_ARCH [12]CROWN_SPREAD
//   [13]CROWN_SKIRT [14]SOLID_PAD [15]EDGE_BLEND
//
// LEAN_DIR is UNIFORM_TAU — uniform [0, 2π] has no meaningful mean/sigma,
// the slot is a literal {0, 0} placeholder.
struct PalmTierRow {
    TierProfile profile;
    float       color_over;
    float       burial;
    float       trunk_var;
    float       frond_var;
    uint32_t    trunk_segs;
    uint32_t    frond_segs;
};

// ── Palm tier table ────────────────────────────────────────────────
// Row = PalmTier order (0 SAPLING / 1 COASTAL / 2 ROYAL); each row =
// { weight, color_var, { 16 {μ,σ} pairs in PalmIdx order, wrapped:
//   line 1: HEIGHT  BASE_R  TOP_R  LEAN  LEAN_DIR
//   line 2: BARK_RINGS  BARK_DEPTH  FROND_COUNT  FROND_LEN  FROND_WIDTH
//   line 3: FROND_DROOP  FROND_ARCH  CROWN_SPREAD  CROWN_SKIRT  SOLID_PAD
//   line 4: EDGE_BLEND } },
//   then: color_over, burial, trunk_var, frond_var, trunk_segs, frond_segs.
// UNITS: lengths = wu; LEAN/FROND_DROOP/FROND_ARCH = radians;
//   BARK_RINGS/FROND_COUNT = counts (do_round); LEAN_DIR row is a {0,0}
//   PLACEHOLDER — it samples ParamDist::UNIFORM_TAU, ignoring μ/σ;
//   weight = tier-selection weight; burial = fraction sunk;
//   trunk/frond_var = color variance; segs = mesh tessellation.
// CONSUMERS: palm_get_tier_profile (generic sampling); burial at
//   palm solid-half; trunk/frond_var at palm_compute_colors; segs at
//   palm write_active. Biography determinant — frozen biography (§12).
inline constexpr PalmTierRow PALM_TIERS[] = {
    /* SAPLING */ {
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
                   {1.20f, 0.10f}, {0.45f, 0.08f}, {0.65f, 0.10f}, {0.20f, 0.08f}, {0.20f, 0.05f},
                   {0.30f, 0.05f} }},
        0.15f, 0.15f, 0.06f, 0.06f, 12, 8
    },
    /* COASTAL */ {
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
                   {0.65f, 0.12f}, {0.45f, 0.20f}, {0.95f, 0.12f}, {0.60f, 0.10f}, {0.30f, 0.08f},
                   {0.40f, 0.08f} }},
        0.20f, 0.15f, 0.06f, 0.06f, 16, 10
    },
    /* ROYAL   */ {
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
                   {0.75f, 0.22f}, {0.40f, 0.12f}, {0.85f, 0.10f}, {0.50f, 0.12f}, {0.50f, 0.10f},
                   {0.50f, 0.10f} }},
        0.25f, 0.20f, 0.08f, 0.08f, 20, 12
    },
};
static_assert(sizeof(PALM_TIERS) / sizeof(PalmTierRow) == static_cast<uint32_t>(PalmTier::COUNT),
    "F-5: PALM_TIERS must have exactly one row per PalmTier");

inline const TierProfile& palm_get_tier_profile(uint32_t tier_idx) {
    return PALM_TIERS[tier_idx].profile;
}

inline constexpr ColorPartDef PALM_COLOR_PARTS[] = {
    { {0.45f,0.35f,0.25f}, 0.0f, PalmProp::COLOR_VAR_R, 0 },   // trunk
    { {0.25f,0.45f,0.20f}, 0.0f, PalmProp::COLOR_VAR_R, 10 },  // frond
    { {0.35f,0.38f,0.18f}, 0.0f, PalmProp::COLOR_VAR_R, 20 },  // aged
};

inline constexpr EntityFamilyTraits PALM_TRAITS = {
    PopFamily::PALM, Dim::MAX_PALM_INSTANCES,
    true,                 // grounded
    PalmProp::SPAWN_ROLL, PalmConfig::SPAWN_CHANCE,
    mood_mult_for(PopFamily::PALM), PalmConfig::POSITION_JITTER,
    PALM_TIER_COUNT, PalmProp::TIER,
    PALM_PARAM_DEFS, PALM_PARAM_COUNT,
    PalmProp::POSITION_X, PalmProp::POSITION_Z, PalmProp::ROTATION,
    3, PALM_COLOR_PARTS,
};

// ── Palm adapter functions ──

inline SpawnGateOutput palm_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    return gate_from_traits(c, gx, gz, PALM_TRAITS, c->entities_state_.palms);
}


inline constexpr uint32_t PALM_INDOOR_RESCALE_PARAMS[] = {
    PalmIdx::HEIGHT, PalmIdx::BASE_R, PalmIdx::TOP_R, PalmIdx::BARK_DEPTH,
    PalmIdx::FROND_LEN, PalmIdx::FROND_WIDTH, PalmIdx::CROWN_SPREAD,
    PalmIdx::CROWN_SKIRT, PalmIdx::SOLID_PAD, PalmIdx::EDGE_BLEND,
    // LEAN/LEAN_DIR (angles), BARK_RINGS/FROND_COUNT (counts),
    // FROND_DROOP/FROND_ARCH (angles) intentionally not scaled.
};

// Palm policy: CAP (INDOOR_TREATMENT) — outdoor size stands unless
// taller than the cap. The old tighter rolled band ([0.80, 0.95],
// canopy-anchor rationale) is held by git.
inline void palm_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    cap_to_ceiling(inst, ceiling_h, INDOOR_LIVE.height_cap_fraction,
        /*current_h*/ inst.params[PalmIdx::HEIGHT],
        PALM_INDOOR_RESCALE_PARAMS);
}

inline void palm_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    float base_r = inst.params[PalmIdx::BASE_R];
    float pad    = inst.params[PalmIdx::SOLID_PAD];
    float blend  = inst.params[PalmIdx::EDGE_BLEND];
    inst.solid_half = base_r + pad + blend;
    inst.burial = PALM_TIERS[inst.tier_idx].burial;
}

inline void palm_compute_colors(EntityInstance& inst, const EntityFamilyTraits& traits, const TierProfile& /*tier*/) {
    const auto& tp = PALM_TIERS[inst.tier_idx];
    // trunk: trunk_var
    inst.colors[0] = traits.color_parts[0].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R) - 0.5f) * tp.trunk_var;
    inst.colors[1] = traits.color_parts[0].base[1] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_G) - 0.5f) * tp.trunk_var;
    inst.colors[2] = traits.color_parts[0].base[2] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_B) - 0.5f) * tp.trunk_var;
    // frond: frond_var
    inst.colors[3] = traits.color_parts[1].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R + 10u) - 0.5f) * tp.frond_var;
    inst.colors[4] = traits.color_parts[1].base[1] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_G + 10u) - 0.5f) * tp.frond_var;
    inst.colors[5] = traits.color_parts[1].base[2] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_B + 10u) - 0.5f) * tp.frond_var;
    // aged: frond_var (same variance as frond)
    inst.colors[6] = traits.color_parts[2].base[0] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_R + 20u) - 0.5f) * tp.frond_var;
    inst.colors[7] = traits.color_parts[2].base[1] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_G + 20u) - 0.5f) * tp.frond_var;
    inst.colors[8] = traits.color_parts[2].base[2] + (cpu_hash_f(inst.seed, PalmProp::COLOR_VAR_B + 20u) - 0.5f) * tp.frond_var;
}

inline void palm_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ap = c->entities_state_.palms[inst.slot];
    ap.patch_gx = inst.trigger_gx; ap.patch_gz = inst.trigger_gz;
    ap.host_gx = inst.host_gx; ap.host_gz = inst.host_gz;
    ap.active = true; ap.draw_visible = true;
    ap.world_x = inst.cx; ap.world_z = inst.cz;
    ap.height = inst.params[PalmIdx::HEIGHT];
    ap.base_r = inst.params[PalmIdx::BASE_R];
    ap.tier_idx = inst.tier_idx;
    ap.cached_ground_y = inst.cached_ground_y;
}

inline void palm_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    const auto& tp = PALM_TIERS[inst.tier_idx];
    GPUPalmMeshParams mp{};
    mp.center_x     = inst.cx;
    mp.center_z     = inst.cz;
    mp.height       = inst.params[PalmIdx::HEIGHT];
    mp.base_r       = inst.params[PalmIdx::BASE_R];
    mp.top_r        = inst.params[PalmIdx::TOP_R];
    mp.lean         = inst.params[PalmIdx::LEAN];
    mp.lean_dir     = inst.params[PalmIdx::LEAN_DIR];
    mp.bark_rings   = inst.params[PalmIdx::BARK_RINGS];
    mp.bark_depth   = inst.params[PalmIdx::BARK_DEPTH];
    mp.frond_count  = inst.params[PalmIdx::FROND_COUNT];
    mp.frond_len    = inst.params[PalmIdx::FROND_LEN];
    mp.frond_width  = inst.params[PalmIdx::FROND_WIDTH];
    mp.frond_droop  = inst.params[PalmIdx::FROND_DROOP];
    mp.frond_arch   = inst.params[PalmIdx::FROND_ARCH];
    mp.crown_spread = inst.params[PalmIdx::CROWN_SPREAD];
    mp.crown_skirt  = inst.params[PalmIdx::CROWN_SKIRT];
    mp.burial       = inst.burial;
    mp.trunk_r = inst.colors[0]; mp.trunk_g = inst.colors[1]; mp.trunk_b = inst.colors[2];
    mp.frond_r = inst.colors[3]; mp.frond_g = inst.colors[4]; mp.frond_b = inst.colors[5];
    mp.aged_r  = inst.colors[6]; mp.aged_g  = inst.colors[7]; mp.aged_b  = inst.colors[8];
    mp.trunk_segs = tp.trunk_segs;
    mp.frond_segs = tp.frond_segs;
    mp.is_active = 1;
    c->gpuState_.upload_palm_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.palm_mesh_gen_pending = true;
}

inline constexpr EntityFamilyAdapter PALM_ADAPTER = {
    palm_run_gate,
    palm_apply_indoor_rescale,
    palm_compute_solid_half, palm_compute_colors,
    palm_write_active, palm_write_gpu, nullptr,
    palm_get_tier_profile,
};

// ── Palm dispatch wrappers ──

inline bool dispatch_select_palm_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, PALM_TRAITS, PALM_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::PALM; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_palm_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, PALM_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.palms[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_palm_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { generic_commit(self, PALM_TRAITS, PALM_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::PALM, pe.generic.slot); }
    // HOST PATCH GONE. The footprint was registered at place; its host
    // vanished before commit. Release by OWNER — the one release path.
    else { unregister_footprint_for(self, PopFamily::PALM, pe.generic.slot); self->entities_state_.palms[pe.generic.slot].active = false; }
}

// ─── Teardown (owner verb) ────────────────────────────────────────
// The grounded families' half of the world-teardown sweep — CPU slot
// clears + GPU param-slot clears + mesh-gen re-arm, five families in
// their one organ. UNGATED by design: the five families share this
// organ, and per-family gating buys nothing (empty arrays clear to
// empty). The arch clear announces the portal-set change on the
// standing flag channel (mood_state_.portals_dirty).
inline void teardown_entities(MachineCtx* c, wgpu::Queue& queue) {
    // Arches
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        c->entities_state_.arches[i] = ActiveArch{};
    }
    c->mood_state_.portals_dirty = true;
    c->gpuState_.set_arch_index_count(0);
    // Clear all arch mesh gen param slots
    {
        GPUArchMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
            c->gpuState_.upload_arch_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.arch_mesh_gen_pending = true;
    }

    // Columns + Antennas
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        c->entities_state_.columns[i] = ActiveColumn{};
    }
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        c->entities_state_.antennas[i] = ActiveColumn{};
    }
    c->gpuState_.set_column_index_count(0);
    // Clear all column mesh gen param slots
    {
        GPUColumnMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_COLUMN_INSTANCES; i++) {
            c->gpuState_.upload_column_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.column_mesh_gen_pending = true;
    }

    // Palms
    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
        c->entities_state_.palms[i] = ActivePalm{};
    }
    c->gpuState_.set_palm_index_count(0);
    {
        GPUPalmMeshParams emptyParams{};
        for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
            c->gpuState_.upload_palm_mesh_params_slot(queue, i, emptyParams);
        }
        c->entities_state_.palm_mesh_gen_pending = true;
    }

    // Pyramids
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        c->entities_state_.pyramids[i] = ActivePyramid{};
    }
    c->entities_state_.cpu_pyramids = GPUPyramidArray{};
    c->gpuState_.upload_pyramids(queue, c->entities_state_.cpu_pyramids);
}

} // namespace the_board
} // namespace t7
