#pragma once
// ─── entity_types.hpp ────────────────────────────────────────────
// Converted (LADDER-1 c3): history in audit/LADDER.md.
//
// Type definitions for the generic entity pipeline: pure
// declarations, no functions, no coupling beyond the Cartridge
// forward reference in adapter signatures.
//
// ┌─── Public surface (consumed by other files) ────────────────────┐
// │                                                                  │
// │  Pipeline contracts:                                             │
// │    MAX_ENTITY_PARAMS       — params[] array length               │
// │    MAX_COLOR_CHANNELS      — colors[] array length               │
// │    ParamDist (enum)        — sampling distribution               │
// │    TierParamDef            — one param's contract                │
// │    TierMuSigma, TierProfile — Gaussian sampling input            │
// │    ColorPartDef            — one color part's spec               │
// │                                                                  │
// │  Family description:                                             │
// │    EntityFamilyTraits      — declarative family metadata         │
// │                                                                  │
// │  Per-instance + adapter:                                         │
// │    EntityInstance          — one rolled instance, pipeline state │
// │    SpawnGateOutput         — gate result                         │
// │    EntityFamilyAdapter     — function-pointer table per family   │
// │                                                                  │
// │  Boundary DTOs (the bespoke families' payload pairs):             │
// │    GoLSelection / GoLPlacement                                   │
// │    GallerySelection / GalleryPlacement                           │
// │    RibbonSelection / RibbonPlacement                             │
// │                                                                  │
// │  Dispatch contract (end of header):                               │
// │    EntityQueueEntry        — tagged selection union              │
// │    PlacementEntry          — tagged placement union              │
// │    FamilyDispatch          — dispatch row type (six verbs + name)│
// │    FAMILY_DISPATCH (extern) — the table (family_dispatch.inl)    │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// A file-scope header, included above the class with roster.hpp's
// cohort. THE CONTRACT HOME: it carries the pipeline contracts, the
// boundary DTOs (the three bespoke families' Selection/Placement
// pairs — a DTO that exists to cross a boundary belongs to the
// boundary's contract, not to either side), AND the dispatch
// contract (the queue-entry unions + the row type + the table
// declaration). Namespace t7::the_board (the cartridge's own).
// Depends on: <cstdint>, <cstring>, roster.hpp (PopFamily),
// keyhole.hpp (the Cartridge and wgpu::Queue fwds), and a local
// wgpu::ComputePassEncoder fwd — no module header: the DTOs are
// plain aggregates of built-ins by design, so the contract home
// carries no owner vocabulary and every owner header may include it.
//
// SEAM[entity_types:P9] this file is the canonical home of pattern
//   P9 (type definitions extracted to header-style file) — a real
//   file-scope header. Pure declarations; the implementations they
//   describe live in entity_pipeline.inl (generic functions, family
//   data, adapters, dispatch wrappers). Same family as seed_utils (P9
//   instance for hashing primitives).
// Tier sampling profiles + extras live in per-family TierRow structs
//   in entity_pipeline.inl, reached via adapter.get_tier_profile —
//   not on EntityFamilyTraits.
// ─────────────────────────────────────────────────────────────────

#include <cstdint>
#include <cstring>                                        // std::memset (queue-entry ctors)
#include "cartridges/the_board/roster.hpp"                // PopFamily (sizes the dispatch table)
#include "cartridges/the_board/modules/keyhole.hpp"       // Cartridge + wgpu::Queue fwds (the keyhole)

// fwd — FamilyDispatch::dispatch_mesh takes wgpu::ComputePassEncoder&
// (reference; forward decl suffices). LOCKSTEP INSURANCE (same
// construct as keyhole.hpp): mirrors webgpu_cpp.h's declaration form.
// If Dawn ever changes that form, replace this with the include.
namespace wgpu { class ComputePassEncoder; }

namespace t7 {
namespace the_board {

// ═══ PIPELINE CONTRACTS ══════════════════════════════════════════
//
// Numerical contracts and sampling primitives shared across every
// generic-pipeline family. These define the shape of the pipeline
// interface — touch only with intent.

// ── Array bounds ─────────────────────────────────────────────────
inline constexpr uint32_t MAX_ENTITY_PARAMS = 32;
inline constexpr uint32_t MAX_COLOR_CHANNELS = 12;

// ── Sampling distributions ───────────────────────────────────────
// Determines how a TierParamDef's `prop` is rolled. GAUSSIAN draws
// from the per-tier (mean, sigma); UNIFORM_01 returns hash(seed,
// prop) directly; UNIFORM_TAU returns the same scaled to [0, 2π).
enum class ParamDist : uint32_t {
    GAUSSIAN,
    UNIFORM_01,
    UNIFORM_TAU,
};

// ── Param contract ───────────────────────────────────────────────
// One row per parameter the family rolls. Order in the family's
// PARAM_DEFS[] array must match the family's *Idx struct one-to-one.
struct TierParamDef {
    uint32_t   prop;
    float      floor;
    float      ceiling;    // upper clamp (1e30 = no ceiling)
    bool       do_round;
    ParamDist  dist;
};

// ── Gaussian sampling input ──────────────────────────────────────
// Per-parameter (mean, sigma) pair. TierProfile holds one of these
// for every parameter the family rolls, indexed in lockstep with
// the family's PARAM_DEFS[] array.
struct TierMuSigma {
    float mean, sigma;
};

struct TierProfile {
    float          weight;
    float          color_var;     // per-tier scalar color variance; 0 = use ColorPartDef.variance fallback
    TierMuSigma    params[MAX_ENTITY_PARAMS];
};

// ── Color part spec ──────────────────────────────────────────────
// One row per coloured "part" of the family (body / aged / trunk /
// frond / etc.). Variance is rolled from prop_base + prop_offset
// triplet.
//
// The `variance` field is the per-part fallback. If the active
// TierProfile carries a nonzero `color_var`, that overrides this
// per-part value uniformly across all parts. Families that need
// per-part-per-tier variance (Palm) keep their own override.
struct ColorPartDef {
    float    base[3];
    float    variance;
    uint32_t prop_base;
    uint32_t prop_offset;
};

// ═══ FAMILY DESCRIPTION ══════════════════════════════════════════
//
// Declarative metadata describing a generic-pipeline family. One
// of these is constructed per family in entity_pipeline.inl as a
// `static constexpr <FAMILY>_TRAITS` and passed to generic_select
// / generic_place / generic_commit.

struct EntityFamilyTraits {
    uint32_t    family_id;
    const char* short_name;
    uint32_t    max_instances;
    bool        grounded;
    bool        creates_ground;
    uint32_t    piers_per_entity;
    bool        has_footprint;
    float       cull_base;
    float       cull_height_scale;
    uint32_t    spawn_roll_prop;
    float       spawn_chance;
    const float* mood_multiplier;
    float       position_jitter;
    uint32_t    tier_count;
    uint32_t    tier_prop;
    const TierParamDef* param_defs;
    uint32_t    param_count;
    uint32_t    pos_x_prop;
    uint32_t    pos_z_prop;
    uint32_t    rotation_prop;
    bool        gpu_ground_y;       // true = GPU compute corrects ground_y (CPU uploads offset only)
    uint32_t    color_part_count;
    const ColorPartDef* color_parts;
};

// ═══ PER-INSTANCE + ADAPTER ══════════════════════════════════════
//
// What flows through the three-phase pipeline (EntityInstance) and
// how each family customizes the generic steps (EntityFamilyAdapter).

// ── Spawn gate result ────────────────────────────────────────────
// Returned by the family's run_gate adapter. ok=false → early exit
// from generic_select.
struct SpawnGateOutput {
    bool     ok;
    uint32_t seed;
    uint32_t slot;
    uint32_t theme_idx;
};

// ── Per-instance pipeline state ──────────────────────────────────
// One of these per spawning entity. Populated incrementally:
//   generic_select  → family_id, seed, trigger_*, slot, tier_idx,
//                     theme_idx, params[], colors[], solid_half
//   generic_place   → cx, cz, rotation, host_*
//   generic_commit  → consumed by adapter.write_active / write_gpu
struct EntityInstance {
    uint32_t family_id = 0;
    uint32_t seed = 0;
    int32_t  trigger_gx = 0, trigger_gz = 0;
    int32_t  host_gx = 0, host_gz = 0;
    uint32_t slot = 0;
    uint32_t tier_idx = 0;
    uint32_t theme_idx = 0;
    float    cx = 0.0f, cz = 0.0f;
    float    rotation = 0.0f;
    float    params[MAX_ENTITY_PARAMS]{};
    float    solid_half = 0.0f;
    float    cached_ground_y = 0.0f;
    float    ground_y_offset = 0.0f;  // added to terrain Y (e.g. solid_height for pier entities)
    float    burial = 0.0f;
    float    colors[MAX_COLOR_CHANNELS]{};
};

// ── Per-family adapter ───────────────────────────────────────────
// Function-pointer table. Each family in entity_pipeline.inl
// constructs one as a `static constexpr <FAMILY>_ADAPTER`.
//
// get_tier_profile is per-family because the TierProfile lives
// embedded in each family's per-family TierRow struct (one source
// of truth — there's no generic table on traits to index).
struct EntityFamilyAdapter {
    SpawnGateOutput(*run_gate)(Cartridge* c, int32_t gx, int32_t gz);
    const float* (*get_theme_tier_weights)(uint32_t theme_idx);
    void (*apply_indoor_rescale)(EntityInstance& inst, float ceiling_h);
    void (*compute_solid_half)(EntityInstance& inst, const TierProfile& tier);
    void (*compute_colors)(EntityInstance& inst, const EntityFamilyTraits& traits, const TierProfile& tier);
    void (*write_active)(Cartridge* c, const EntityInstance& inst);
    void (*write_gpu)(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue);
    void (*post_commit)(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue);
    const TierProfile& (*get_tier_profile)(uint32_t tier_idx);
};

// ═══ DISPATCH CONTRACT (queue entries + row type) ═════════════════
//
// The tagged unions that carry every family — generic and bespoke —
// through select → place → commit, and the dispatch-row type that
// walks them. The QUEUES themselves (entityQueue_, placementResults_)
// are machine state and live in spawn_engine.inl; only the vocabulary
// lives here.

// ═══ SPAWN PAYLOADS (the boundary DTOs) ═══════════════════════════
//
// The three bespoke families' Selection/Placement DTOs. They exist
// to cross the machine/owner boundary inside EntityQueueEntry /
// PlacementEntry (below) — and a DTO that exists to cross a boundary
// belongs to the boundary's contract, not to either side. Plain
// aggregates of built-ins, by design: the contract home carries no
// owner vocabulary.

// ── GoL (zone lattice; Conway/Pulse) ──────────────────────────────
struct GoLSelection {
    uint32_t seed;
    int32_t  trigger_gx, trigger_gz;
    uint32_t slot;
    int32_t  zone_nx, zone_nz;     // lattice node
    float    corner_x, corner_z;   // zone corner (cell-grid-snapped)
    uint32_t algorithm;            // AlgorithmType::CONWAY or PULSE
    uint32_t tier_idx;             // compound: Conway 0–6, Pulse 7–9
    float    tick_period;
    float    initial_density;
    bool     height_enabled;
    float    footprint_r;
};

struct GoLPlacement {
    uint32_t slot;
    int32_t  trigger_gx, trigger_gz;
    int32_t  host_gx, host_gz;
    uint32_t tier_idx;
    float    cx, cz;               // zone center
    int32_t  zone_nx, zone_nz;
    float    corner_x, corner_z;
    uint32_t algorithm;
    float    tick_period;
    float    initial_density;
    bool     height_enabled;
};

// ── Gallery (outdoor art exhibitions — composite: 1 center → N paintings) ──
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

// ── Ribbon ─────────────────────────────────────────────────────────
struct RibbonSelection {
    uint32_t seed;
    int32_t  trigger_gx, trigger_gz;
    uint32_t slot;
    uint32_t tier_idx;
    // Geometry (from select_ribbon_for_patch)
    uint32_t cube_count;
    float cube_size;
    float height;
    float orientation;
    float lateral_amp, lateral_cycles;
    float vertical_amp;
    // Color
    uint32_t color_mode;
    float color[3];
    float color_b[3];        // CONTRAST second median
    float checker_scatter = 0.0f;
    float checker_hue_spread = 0.0f;
    // Footprint
    float footprint_r;
};

struct RibbonPlacement {
    uint32_t slot;
    int32_t  trigger_gx, trigger_gz;
    int32_t  host_gx, host_gz;
    uint32_t tier_idx;
    float cx, cz;
    // Geometry (copied from selection)
    uint32_t cube_count;
    float cube_size, height, orientation;
    float lateral_amp, lateral_cycles;
    float vertical_amp;
    uint32_t color_mode;
    float color[3];
    float color_b[3];        // CONTRAST second median
    float checker_scatter = 0.0f;
    float checker_hue_spread = 0.0f;
    uint32_t seed = 0u;   // spawn seed, carried so commit samples its channels
};

// ─── Entity Selection Queue entry ─────────────────────────────────
//
// Lightweight tagged entry holding one family's selection.
// Produced by select_entities_for_patch, consumed by
// drain_entity_queue. The queue decouples WHAT exists from
// WHERE it goes — selections are position-independent.

struct EntityQueueEntry {
    uint32_t family;    // PopFamily index
    int32_t  gx, gz;    // trigger patch (for commit bookkeeping)
    union {
        RibbonSelection ribbon;
        GoLSelection    gol;
        GallerySelection gallery;
        EntityInstance   generic;    // used by all 9 generic-pipeline families
    };
    EntityQueueEntry() : family(0), gx(0), gz(0) { std::memset(&generic, 0, sizeof(generic)); }
};

// ─── Placement Results entry ───────────────────────────────────────
//
// Output of place_entity_queue: entities that passed spatial
// negotiation and are ready for GPU commit. Tagged union mirrors
// EntityQueueEntry but holds Placement structs instead of Selections.

struct PlacementEntry {
    uint32_t family;
    int32_t  gx, gz;
    union {
        RibbonPlacement ribbon;
        GoLPlacement    gol;
        GalleryPlacement gallery;
        EntityInstance   generic;    // used by all 9 generic-pipeline families
    };
    PlacementEntry() : family(0), gx(0), gz(0) { std::memset(&generic, 0, sizeof(generic)); }
};

// ─── Dispatch row type ─────────────────────────────────────────────
//
// One row per family: the six verbs the spine's dispatch loops call
// through. The table itself (FAMILY_DISPATCH) is the spine's
// integration hub (SEAM[spine:owns] at its banner in cartridge.hpp).

struct FamilyDispatch {
    bool (*try_select)(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
    bool (*try_place)(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
    void (*try_commit)(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);
    void (*evict_slot)(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
    bool (*prepare_mesh)(Cartridge* self, wgpu::Queue& queue);
    void (*dispatch_mesh)(Cartridge* self, wgpu::ComputePassEncoder& pass);
    const char* name;
};

// THE TABLE — one row per family, PopFamily order. DEFINED at file
// scope in modules/family_dispatch.inl (post-class: the rows take the
// addresses of wrappers that live with their owners and, for the
// generic families, on the class); DECLARED here so the spine's
// dispatch loops (in-class) read it by namespace lookup.
extern const FamilyDispatch FAMILY_DISPATCH[PopFamily::COUNT];

} // namespace the_board
} // namespace t7

// ═══ END entity_types.hpp ════════════════════════════════════════
