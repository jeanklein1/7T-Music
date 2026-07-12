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
// │  Dispatch contract (end of header):                               │
// │    EntityQueueEntry        — tagged selection union              │
// │    PlacementEntry          — tagged placement union              │
// │    FamilyDispatch          — dispatch row type (six verbs + name)│
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// A file-scope header, included above the class with roster.hpp's
// cohort. THE CONTRACT HOME: it carries the pipeline contracts AND
// the dispatch contract (the queue-entry unions + the row type).
// Namespace t7::the_board (the cartridge's own). Depends on:
// <cstdint>, <cstring>, keyhole.hpp (the Cartridge and wgpu::Queue
// forward declarations), and the three bespoke subsystem headers —
// ribbon.hpp / gol_zones.hpp / gallery.hpp — whose Selection/
// Placement types the unions embed BY VALUE (complete types
// required; they carry state.hpp, so wgpu is complete here too).
// That dependency is honest: the union IS the coupling between the
// machine and the bespoke subsystems. Consequence: those three
// headers can never include this one (circularity) — functions with
// queue-shaped signatures are declared HERE, not in owner headers.
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
#include "cartridges/the_board/modules/ribbon.hpp"        // RibbonSelection/RibbonPlacement (union members)
#include "cartridges/the_board/modules/gol_zones.hpp"     // GoLSelection/GoLPlacement (union members)
#include "cartridges/the_board/modules/gallery.hpp"       // GallerySelection/GalleryPlacement (union members)

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

// ─── Bespoke dispatch funnels (defined in their owners' impls) ────
//
// gol_zones.inl / gallery.inl / ribbon.inl. Declared HERE, not in the
// owner headers, because the signatures carry the queue types and the
// owner headers cannot include this header (circularity — see the
// banner). The table names them by these declarations.

bool dispatch_select_gol(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_gol(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_gol(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);

bool dispatch_select_gallery(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_gallery(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_gallery(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);

bool dispatch_select_ribbon(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_ribbon(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_ribbon(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7

// ═══ END entity_types.hpp ════════════════════════════════════════
