#pragma once
// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE COMPILED REGISTRY AND THE PANEL'S C ABI (ORGAN_0b)
//
// THE COMPILED-REGISTRY LAW (docs/ORGAN.md). The registry is COMPILED, not
// parsed. Enrollment is one macro line in organ_params.inc beside nothing
// but its own siblings, and the offset in every entry is `offsetof` — the
// compiler's own answer about the same declaration the program reads. A
// registry that is parsed, or typed, or generated from a second copy of the
// struct can drift; this one cannot, because there is no second copy to
// drift from. Rename a field and the BUILD fails, at the enrollment line,
// naming the field. That is the entire mechanism.
//
// Lineage, stated so the pattern is recognised rather than reinvented:
// 0b-4's BYTE-FOR-BYTE markers (the subject registers itself), the schema
// as one home (L22), LOOM_3's rule that a rename in the subject must never
// require an edit in a witness. This is the same idea pointed at dials.
//
// THE SOVEREIGNTY BOUNDARY is not enforced here — it is enforced in
// state.hpp, which exposes exactly three homes to the panel and no others
// (organ_config_home / organ_lighting_home / organ_agent_room_home). There
// is no block id for GPU truth because there is no accessor to build one
// from. A panel that cannot name a thing cannot write it. Blocks 3 and
// above are the exception that proves it: they are contracts-tier CPU
// surfaces built FOR the panel, so each base is the instance itself and
// no GPUState accessor exists or is needed. The boundary still holds
// where it was drawn — GPU truth has no block id, because there is no
// accessor to build one from.
//
// THE MANIFEST IS THE WHITELIST. organ_set refuses any (block, offset,
// type) triple that is not an entry in kOrganParams — not merely one that
// fits inside the home. Bounds-checking against sizeof would make this a
// memory editor with a range check; checking against the registry makes it
// a panel. Rejections are counted and the count is shown in the panel's own
// status line, so a refusal is visible rather than silent.
// ═══════════════════════════════════════════════════════════════════════

#include "cartridges/the_board/realization/state.hpp"
#include "cartridges/the_board/contracts/spine_state.hpp"   // O1b — MoodProfile + mood_def: the definition side
#include "cartridges/the_board/contracts/agent_tiers.hpp"    // ORGAN_2b — TIER_LIVE, the world's definition bank
#include "cartridges/the_board/contracts/pawn_surface.hpp"    // ORGAN_3 w2 — PAWN_AURA_LIVE (block 4)
#include "cartridges/the_board/contracts/orb_surface.hpp"     // ORGAN_3 w2 — ORB_CONSOLE_LIVE (block 5)
#include "cartridges/the_board/contracts/control_panel.hpp"   // ORGAN_3 w2 — PANEL_LIVE (block 6)
#include "cartridges/the_board/contracts/ribbon_surface.hpp"  // ORGAN_3 w2 — RIBBON_LIVE (block 7)
#include "cartridges/the_board/contracts/indoor_module.hpp"   // ORGAN_3 w3 — INDOOR_LIVE (block 8, destructive)
#include "coupling/canvas_surface.hpp"                        // ORGAN_3b P2 — CANVAS_LIVE (block 9, t7::canvas)
#include "cartridges/the_board/contracts/driver_surface.hpp"  // ORGAN_2a — the drivers' room (block 3)

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace t7 {
namespace organ {

// ─── Type tags ────────────────────────────────────────────────────────
// The lane count is the whole difference between them at the ABI: a VEC3
// is three contiguous floats, and a colour is a VEC3 over 0..1 that the
// panel happens to render with a colour input.
enum : uint8_t {
    ORGAN_F32 = 0,
    ORGAN_U32 = 1,
    ORGAN_BOOL = 2,
    ORGAN_VEC3 = 3,
    ORGAN_VEC4 = 4,
};

inline int lanes_of(uint8_t type) {
    switch (type) {
    case ORGAN_VEC3: return 3;
    case ORGAN_VEC4: return 4;
    default:         return 1;
    }
}

// ─── Block ids ────────────────────────────────────────────────────────
// One per CPU home GPUState hands the panel, and the numbering is the bit
// position organ_mark_dirty uses.
//
// A BLOCK ID IS NOT A PROMISE ABOUT HOW THE HOME REACHES THE GPU (O1d).
// config_ is STAGED and its upload is the spine's, off its own configDirty_;
// lightingStage_ and agentRoomStage_ are flushed by organ_flush. The panel
// says "this home changed" and the home decides what that costs — which is
// why organ_mark_dirty routes and this enum does not.
//
// There is deliberately no id for
// GPUSceneConstants: its tier_gains are a WINDOW onto the agents' room
// (CHORD's WINDOWS-NOT-HOMES ruling), its figure profiles are packed from
// a constexpr table, and its ribbon's home lives in bodies/ribbon.hpp — so
// a SceneConstants block id could only address a second copy of somebody
// else's fact, which is the one thing the charter forbids. Nor is there an
// id for frame_r as a whole: only its lighting region is CPU-authored, and
// that region has its own home and its own id.
enum : uint8_t {
    ORGAN_BLOCK_CONFIG     = 0,   // GPUDesignConfig      — config_
    ORGAN_BLOCK_LIGHTING   = 1,   // GPULighting          — lightingStage_
    ORGAN_BLOCK_AGENT_ROOM = 2,   // GPUAgentRoomConstants — agentRoomStage_
    ORGAN_BLOCK_DRIVERS    = 3,   // DriverSurface         — DRIVER_LIVE
                                  //   (contracts/driver_surface.hpp; CPU-read
                                  //   home — the seams are its flush)
    // ORGAN_3 w2 — THE GRADUATED MODULE BANKS. Each is a contracts-tier
    // CPU surface: a module's authored console, given a live shadow so the
    // panel can name it. Same shape as block 3 — no GPUState accessor, no
    // upload, and the readers that consume the bank each tick ARE its
    // flush. The bit space is organTouched_ (32 bits); the running count
    // is recorded in the disposition ledger so a future wave can see it
    // approaching rather than discover it.
    ORGAN_BLOCK_PAWN       = 4,   // PawnAuraProfile       — PAWN_AURA_LIVE
    ORGAN_BLOCK_ORBS       = 5,   // OrbConsole            — ORB_CONSOLE_LIVE
    ORGAN_BLOCK_PANEL      = 6,   // PanelSurface          — PANEL_LIVE
    ORGAN_BLOCK_RIBBON     = 7,   // RibbonSurface         — RIBBON_LIVE
    ORGAN_BLOCK_INDOOR     = 8,   // IndoorSurface         — INDOOR_LIVE
                                  //   DESTRUCTIVE temperament: no boundary
                                  //   re-speak, the edit lands at the next
                                  //   spawn (ORGAN_3 w3)
    ORGAN_BLOCK_CANVAS     = 9,   // canvas::CanvasSurface — CANVAS_LIVE
                                  //   the first block BELOW the cartridge
                                  //   tier (coupling/canvas_surface.hpp);
                                  //   the _NS macros exist for it (P2)
    ORGAN_BLOCK_COUNT      = 10,
};

// A definition-only entry has no instance anywhere: its block is the
// sentinel, block_base answers null, and organ_set routes it straight to
// the definition path — preview on it is refused, because there is
// nothing a preview could show. Its `offset` carries def_offset so the
// (block, offset, type) triple stays unique and the manifest round-trips.
// ORGAN_3b P3 — A SECOND DEFINITION-ONLY FAMILY, AND WHY IT NEEDS A
// SECOND SENTINEL. An entry's identity is its (block, offset, type)
// triple, and a definition-only entry's `offset` is its def_offset — an
// offset into ITS OWN struct. Two families sharing block 255 would let
// `offsetof(MoodProfile, clear_color)` and
// `offsetof(OrbMoodConfig, something)` collide at the same number and
// resolve to each other. One sentinel per family makes that impossible
// by construction rather than by luck.
//
// The convention DESCENDS from 255: a third family takes 253. Descending
// keeps the real block ids growing upward from 0 with the whole space
// between them, and keeps `is_defonly` a small explicit list rather than
// a range test that would silently swallow a future block 253.
enum : uint8_t {
    ORGAN_BLOCK_NONE     = 255,   // the_board::MoodProfile family (ORGAN_2b)
    ORGAN_BLOCK_NONE_ORB = 254,   // the_board::OrbMoodConfig family (ORGAN_3b)
};

// ─── Definition targets (O1b) ─────────────────────────────────────────
// Where a dial's DEFINITION lives, if it has one. An entry's home is its
// INSTANCE — what the program is showing right now — and for a dial with
// a second author the instance is on loan. The definition is the fact the
// author reads when it next speaks, so writing the definition is how a
// panel edit outlives the author (docs/ORGAN.md, "Instance and
// definition").
//
// NEVER REINTERPRET (ORGAN_1, amended by ORGAN_3b). A float definition
// target is a run of floats with the same lane count as the entry's type.
// An integer target — U32 or BOOL — is CONVERTED, by the same rule
// organ_set's instance path has used since ORGAN_0, and never has a
// float's bit pattern written into it. ORGAN_1 refused integers here
// because no definition target was one; ORGAN_3b's orb bank made that
// false, and the rule that survived the change is the one about
// reinterpreting, not the one about refusing.
//
// A KIND NAMES THE FAMILY, AND THE FAMILY ANSWERS ONE QUESTION (ORGAN_2b).
// MOOD answers "what does this mood mean" — there is one profile per mood
// and the write's target selects which. TIER answers "what does this
// WORLD mean by its tiers" — there is one bank, so the target is ignored
// by design rather than by oversight. BEHAVIOR answers the world question
// too, for the other half of the same author's material. definition_base
// is the one place that mapping lives.
//
// A KIND IS NOT A FLAG. Two kinds share a flag when they share an AUTHOR,
// because the flag names the occasion and the occasion is the author
// speaking — which is why BEHAVIOR raises TIER's (ORGAN_3 w3).
enum : uint8_t {
    ORGAN_DEF_NONE = 0,   // no definition: the home IS the only truth there is
    ORGAN_DEF_MOOD = 1,   // the_board::MoodProfile, at def_offset, same lanes
                          //   — per-mood: target selects WHICH mood it means
    ORGAN_DEF_TIER = 2,   // the_board::AgentTierBank (TIER_LIVE), at def_offset
                          //   — the WORLD'S definition: one bank, target ignored
    ORGAN_DEF_BEHAVIOR = 3,  // the_board::AgentBehaviorBank (BEHAVIOR_LIVE)
                          //   — the world's definition too, and the SAME
                          //   author's: it raises the TIER flag rather than
                          //   keeping a second name for one occasion
                          //   (ORGAN_3 w3)
    ORGAN_DEF_ORB_MOOD = 4,  // the_board::ORB_MOOD_LIVE[mood] — MOOD-SELECTED,
                          //   like MOOD and unlike TIER/BEHAVIOR: there is a
                          //   row per mood and the write's target picks it.
                          //   Its applier is configure_orbs (bodies/orbs.hpp),
                          //   reached from the mood fan; its own flag is
                          //   g_orb_def_dirty (ORGAN_3b P3)
};

// ─── Cadence (ORGAN_3b) ───────────────────────────────────────────────
// WHEN a stop sounds. Jean's first sweep on the disposition named the one
// defect it inherited rather than created: a generational dial reads as a
// DEAD dial, because the panel said what every stop was and never said
// when it spoke.
//
// CADENCE IS A PROPERTY OF A FACT'S AUTHORSHIP, so it is DERIVED and not
// hand-painted. Only the one bit the registry cannot infer is stored:
//
//   DRIVEN   — an `ro` entry: its author speaks every frame, and the row
//              is a meter. Inferred from `ro`.
//   BOUNDARY — a definition-kind entry, a definition-only entry, or an
//              entry in a block whose writes raise a re-speak flag: the
//              edit lands within a frame. Inferred from def_kind, from
//              the sentinel block, and from block_has_boundary below.
//   GEN      — the author's next natural event (a spawn, a world init).
//              STORED, because nothing in the entry can tell you that the
//              nine readers of a cap fraction run at spawn time. This is
//              the one fact the enrollment line must volunteer, and
//              ORGAN_PARAM_GEN is how it does.
//   LIVE     — everything else: the home is read where it is needed and
//              the edit is simply true.
//
// derived_cadence() is the ONE place the rule lives, so the manifest
// emitter and the harness cannot disagree about what a row means.
enum : uint8_t {
    ORGAN_CAD_LIVE     = 0,
    ORGAN_CAD_GEN      = 1,
    ORGAN_CAD_BOUNDARY = 2,   // derived, never stored
    ORGAN_CAD_DRIVEN   = 3,   // derived, never stored
};

// ─── The entry ────────────────────────────────────────────────────────
// POD, so the table is a constant the linker can place in rodata.
// `couple` is tier 3's reserved column (docs/ORGAN.md, "The three tiers"):
// zero means "no ear drives this", and ORGAN_0 ships every entry that way.
// The column exists now so tiers 2 and 3 arrive in this registry rather
// than in a second system beside it.
struct OrganParam {
    const char* id;
    const char* label;
    const char* group;
    uint8_t     block;
    uint16_t    offset;
    uint8_t     type;
    float       minv, maxv, step, def;
    uint8_t     couple;
    uint8_t     def_kind;     // O1b — ORGAN_DEF_NONE | ORGAN_DEF_MOOD | _TIER
    uint16_t    def_offset;   // byte offset into the kind's own struct
                              // (MoodProfile or AgentTierBank), when there is one
    uint8_t     ro;           // ORGAN_2a — a WITNESS, not a dial: the panel
                              // meters it and organ_set refuses to write it
    uint8_t     cad;          // ORGAN_3b — ORGAN_CAD_LIVE | _GEN, the only
                              // cadence the entry cannot infer about itself
};

// ─── The enrollment macro ─────────────────────────────────────────────
// The id is "block.field" so it is stable across relabelling: export/import
// keys on it, and a label is prose that may be improved without breaking a
// saved file.
#define ORGAN_PARAM_NS(NS, BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL) \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(NS::STRUCT, FIELD),       \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0,                        \
                ORGAN_DEF_NONE, 0, 0, ORGAN_CAD_LIVE },

// ORGAN_3b — THE SAME LINE, DECLARED GENERATIONAL. Identical in every
// column but the last: the edit lands at the author's next natural event,
// not now and not at the boundary. A dial that edits the future must say
// so WHERE THE HAND IS — at the row, not only in the group's name — which
// is what the chip this feeds is for.
#define ORGAN_PARAM_GEN_NS(NS, BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL) \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(NS::STRUCT, FIELD),       \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0,                        \
                ORGAN_DEF_NONE, 0, 0, ORGAN_CAD_GEN },

// The same line plus the field the dial DEFINES, and the family that field
// belongs to. The compiler takes this offset too, so a rename on the
// definition side fails at the enrollment exactly as a rename on the
// instance side does. DEFKIND is MOOD or TIER (ORGAN_2b) and DEFSTRUCT is
// that family's struct — named by the caller so the macro never has to
// guess, and so a third family costs one more enum value and nothing here.
#define ORGAN_PARAM_DEF_NS(NS, BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP,    \
                        LABEL, DEFKIND, DEFSTRUCT, DEFFIELD)                  \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(NS::STRUCT, FIELD),       \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0,                        \
                ORGAN_DEF_##DEFKIND,                                          \
                (uint16_t)offsetof(NS::DEFSTRUCT, DEFFIELD), 0,        \
                ORGAN_CAD_LIVE },

// ORGAN_2b — A DEFINITION WITH NO INSTANCE. Some facts have a definition
// the panel may write and no home the panel may address: MoodProfile's
// clear_color is read by apply_mood_lighting into clearColor_, a cartridge
// member and not one of the three exposed homes. Enrolling it as a dial on
// nothing would be a lie; enrolling it here says the truth — the write is
// always a definition, preview is refused, and the value shown is the live
// mood's meaning. The offset column carries def_offset, which keeps the
// (block, offset, type) triple unique inside the sentinel block.
// THE SENTINEL IS DERIVED FROM THE KIND, not written at the call site.
// A definition-only entry's `offset` is an offset into ITS OWN struct, so
// two families in one sentinel block can collide at the same number — and
// did, the moment ORGAN_3b P3 added the second: MoodProfile.clear_color
// and OrbMoodConfig.rotation_axis both sat at (255, 32, VEC3) and
// resolved to each other. One mapping line per family, here, makes that
// impossible; a third family adds one #define and no call site changes.
#define ORGAN_DEFONLY_BLOCK_MOOD     ORGAN_BLOCK_NONE
#define ORGAN_DEFONLY_BLOCK_ORB_MOOD ORGAN_BLOCK_NONE_ORB

#define ORGAN_PARAM_DEFONLY_NS(NS, TYPE, MIN, MAX, STEP, GROUP, LABEL,               \
                            DEFKIND, DEFSTRUCT, DEFFIELD)                     \
    OrganParam{ #DEFSTRUCT "." #DEFFIELD, LABEL, GROUP,                       \
                ORGAN_DEFONLY_BLOCK_##DEFKIND,                                \
                (uint16_t)offsetof(NS::DEFSTRUCT, DEFFIELD), \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0,                        \
                ORGAN_DEF_##DEFKIND,                                          \
                (uint16_t)offsetof(NS::DEFSTRUCT, DEFFIELD), 0,        \
                ORGAN_CAD_LIVE },

// ORGAN_2a — A WITNESS, NOT A DIAL. The same offsetof plumbing pointed at a
// DRIVEN value: the panel reads it every 250 ms and shows it moving, and
// organ_set refuses to write it. No min/max/step, because a meter has no
// range to clamp against — the driver's own dials carry the ranges, and
// they are enrolled with ORGAN_PARAM above.
#define ORGAN_PARAM_RO_NS(NS, BLOCK, STRUCT, FIELD, TYPE, GROUP, LABEL)              \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(NS::STRUCT, FIELD),       \
                ORGAN_##TYPE, 0.0f, 0.0f, 0.0f, 0.0f, 0,                      \
                ORGAN_DEF_NONE, 0, 1, ORGAN_CAD_LIVE },

// ─── ORGAN_3b P2 — THE NAMESPACE PARAMETER, MADE INVISIBLE ────────────
// Every form above now takes the enrolled struct's NAMESPACE as its first
// argument, because the canvas's banks live in `t7::canvas` — one tier
// BELOW the cartridge — and a registry that could only spell `the_board::`
// could not name them. The ledger priced exactly this: "one extra macro
// parameter, four call sites."
//
// THE FIVE FORWARDS BELOW ARE THAT PRICE, PAID ONCE. An enrollment line
// that does not care about the namespace does not mention it, so all 223
// lines written before this commit are untouched — and a line that DOES
// care writes _NS and says which. A parameter nobody has to see is the
// only kind worth adding to a file whose whole argument is that adding a
// dial is one line.
#define ORGAN_PARAM(...)         ORGAN_PARAM_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_GEN(...)     ORGAN_PARAM_GEN_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_DEF(...)     ORGAN_PARAM_DEF_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_DEFONLY(...) ORGAN_PARAM_DEFONLY_NS(the_board, __VA_ARGS__)
#define ORGAN_PARAM_RO(...)      ORGAN_PARAM_RO_NS(the_board, __VA_ARGS__)

inline const OrganParam kOrganParams[] = {
#include "console/organ_params.inc"
};
#undef ORGAN_PARAM
#undef ORGAN_PARAM_GEN
#undef ORGAN_PARAM_DEF
#undef ORGAN_PARAM_DEFONLY
#undef ORGAN_PARAM_RO
#undef ORGAN_PARAM_NS
#undef ORGAN_PARAM_GEN_NS
#undef ORGAN_PARAM_DEF_NS
#undef ORGAN_PARAM_DEFONLY_NS
#undef ORGAN_DEFONLY_BLOCK_MOOD
#undef ORGAN_DEFONLY_BLOCK_ORB_MOOD
#undef ORGAN_PARAM_RO_NS

inline constexpr size_t kOrganParamCount =
    sizeof(kOrganParams) / sizeof(kOrganParams[0]);

// ─── The live home ────────────────────────────────────────────────────
// Bound once at boot. Null until then, and every ABI entry point returns
// harmlessly on null — the panel's JS may be present on a page whose
// program has not finished booting.
inline the_board::GPUState* g_home = nullptr;
inline uint32_t g_rejected = 0;   // refused organ_set calls, shown in the panel

// O1b — the live mood id. The registry does not own it and does not want
// to: it borrows the spine's own field, so the panel can never be looking
// at a mood the program has left.
inline const uint32_t* g_mood = nullptr;

inline void bind_home(the_board::GPUState* s) { g_home = s; }
inline void bind_mood(const uint32_t* active) { g_mood = active; }
inline uint32_t current_mood() { return g_mood ? *g_mood : 0u; }

inline void* block_base(uint8_t block) {
    if (!g_home) return nullptr;
    switch (block) {
    case ORGAN_BLOCK_CONFIG:     return g_home->organ_config_home();
    case ORGAN_BLOCK_LIGHTING:   return g_home->organ_lighting_home();
    case ORGAN_BLOCK_AGENT_ROOM: return g_home->organ_agent_room_home();
    case ORGAN_BLOCK_DRIVERS:    return &the_board::DRIVER_LIVE;
    case ORGAN_BLOCK_PAWN:       return &the_board::PAWN_AURA_LIVE;
    case ORGAN_BLOCK_ORBS:       return &the_board::ORB_CONSOLE_LIVE;
    case ORGAN_BLOCK_PANEL:      return &the_board::PANEL_LIVE;
    case ORGAN_BLOCK_RIBBON:     return &the_board::RIBBON_LIVE;
    case ORGAN_BLOCK_INDOOR:     return &the_board::INDOOR_LIVE;
    case ORGAN_BLOCK_CANVAS:     return &canvas::CANVAS_LIVE;
    default:                     return nullptr;
    }
}

// ORGAN_3b — A DEFINITION-ONLY ENTRY'S BLOCK. One helper rather than a
// literal at three call sites, because ORGAN_3b's orb family takes a
// second sentinel and a third family will take a third (the convention
// descends from 255; see the block enum).
inline bool is_defonly(uint8_t block) {
    return block == ORGAN_BLOCK_NONE || block == ORGAN_BLOCK_NONE_ORB;
}

// ORGAN_3b — DOES A WRITE TO THIS BLOCK RAISE A RE-SPEAK FLAG? A bank
// whose author is re-spoken at the frame boundary gives its dials
// BOUNDARY cadence, and that is a property of the BLOCK, not of the
// entry — so it is stated once, here, rather than per line. Empty today:
// blocks 0..8 either reach the GPU directly (LIVE) or are read where they
// are needed. ORGAN_3b P3 adds the orb console, whose author is
// configure_orbs and whose re-speak the orb flag carries.
inline bool block_has_boundary(uint8_t block) {
    // The orb console's only reader is configure_orbs — the same author
    // the orb MOOD bank has — so a write to block 5 raises that author's
    // flag (see organ_set) and the boundary re-speaks it. Which makes the
    // three console dials BOUNDARY cadence, not LIVE: before ORGAN_3b P3
    // they read LIVE and the wait was real but unstated.
    return block == ORGAN_BLOCK_ORBS;
}

// ORGAN_3b — THE ONE PLACE THE CADENCE RULE LIVES. The manifest emitter
// and the harness both call this, so they cannot disagree about what a
// row means. Order matters: a witness is DRIVEN even if its block had a
// boundary, because the row is a meter and the meter's cadence is its
// author's.
inline uint8_t derived_cadence(const OrganParam& e) {
    if (e.ro) return ORGAN_CAD_DRIVEN;
    if (e.def_kind != ORGAN_DEF_NONE || is_defonly(e.block)
        || block_has_boundary(e.block)) return ORGAN_CAD_BOUNDARY;
    return e.cad;                       // LIVE or the stored GEN
}

// THE WHITELIST LOOKUP. A triple that is not an entry is not addressable,
// full stop — this is what keeps organ_set from being a memory editor.
inline const OrganParam* find_entry(int block, int offset, int type) {
    for (size_t i = 0; i < kOrganParamCount; ++i) {
        const OrganParam& e = kOrganParams[i];
        if (e.block == block && e.offset == offset && e.type == type)
            return &e;
    }
    return nullptr;
}

// Declared here because read_lane reaches for it: a definition-only entry
// has no instance, so reading its value IS reading its definition. The
// body stays beside the rest of the definition path, below.
inline float read_definition(const OrganParam& e, uint32_t mood, int lane);

inline float read_lane(const OrganParam& e, int lane) {
    // ORGAN_2b — a definition-only entry has no instance to read, so its
    // "value" is the LIVE mood's definition. The manifest and any meter
    // therefore show what the current mood means; a mood change is
    // reflected on the next panel open, the same freshness the mood-def
    // dials already have.
    if (is_defonly(e.block))
        return read_definition(e, current_mood(), lane);
    void* base = block_base(e.block);
    if (!base || lane < 0 || lane >= lanes_of(e.type)) return 0.0f;
    const char* p = static_cast<const char*>(base) + e.offset;
    if (e.type == ORGAN_U32 || e.type == ORGAN_BOOL) {
        uint32_t v = 0;
        std::memcpy(&v, p, sizeof(v));
        return static_cast<float>(v);
    }
    float v = 0.0f;
    std::memcpy(&v, p + lane * sizeof(float), sizeof(float));
    return v;
}

// ─── THE CONTESTED-DIAL INSTRUMENT (O1a) ──────────────────────────────
// A dial the panel CAN write is not yet a dial the panel OWNS. Some homes
// have a second author — a mood apply, a per-frame updater — and where one
// exists the panel's word is not wrong, it is merely temporary. Before
// ORGAN can rule on that, it has to know WHICH dials are contested, and by
// the authors' own behaviour rather than by a hand census: do not
// hand-census what an instrument can discover.
//
// THE MEASUREMENT is one question, asked once a frame. organ_set records
// exactly the bytes that landed in the home — a shadow of the panel's last
// word — and the observer re-reads the home at the frame boundary and asks
// whether it still says that. Nothing is inferred beyond the answer: a
// disagreement means SOMEBODY ELSE WROTE THIS, and the only open question
// is when.
//
// THE THREE READINGS follow from when the disagreement first appears:
//   FREE      — never disagreed. No other author has spoken since the write.
//   EVENT     — stood for a while, then lost it. An author that runs on an
//               occasion (a mood change, a transition), not on the clock.
//   PER-FRAME — lost it at once. An author on the clock.
// The evidence behind the reading is the SURVIVAL COUNT: how many frames
// the panel's last word stood before the home first disagreed.
//
// WHY THE PER-FRAME THRESHOLD IS ONE AND NOT ZERO. The observer sits at
// the head of the frame, beside the flush, so a write that arrives between
// frames is seen intact once before the frame's own authors have run
// again. A per-frame author therefore leaves survival == 1, never 0, and
// reading 0 as the only per-frame signature would classify every one of
// them as an EVENT. The threshold is a consequence of where the observer
// stands, so it is stated here beside it.
//
// THIS UNIT REPORTS AND DOES NOT ACT. No write path changes, no dial is
// withdrawn, no author is edited. What the readings mean for the panel is
// O1b's ruling, and it wants this census as evidence, not as a fait
// accompli.
enum : uint8_t {
    ORGAN_CONTEST_FREE      = 0,
    ORGAN_CONTEST_EVENT     = 1,
    ORGAN_CONTEST_PER_FRAME = 2,
};

struct OrganContest {
    float    written[4];   // re-read from the home, so it is what LANDED
    uint32_t survived;     // frames the write stood before the first disagreement
    uint32_t disagreed;    // frames observed in disagreement since that write
    uint8_t  seen;         // the panel has written this dial at least once
};

inline OrganContest g_contest[kOrganParamCount] = {};

// THE SHADOW IS READ BACK, NOT COPIED FORWARD. organ_set clamps, and a u32
// dial narrows; taking the shadow from the home rather than from the
// argument means the instrument compares against the bytes that are
// actually there, so a clamp can never masquerade as a rival author.
inline void note_write(const OrganParam& e) {
    const size_t i = static_cast<size_t>(&e - kOrganParams);
    OrganContest& c = g_contest[i];
    const int n = lanes_of(e.type);
    for (int l = 0; l < 4; ++l) c.written[l] = (l < n) ? read_lane(e, l) : 0.0f;
    c.survived  = 0;
    c.disagreed = 0;
    c.seen      = 1;
}

// Exact comparison, deliberately. The shadow holds the very bytes the write
// left behind, so any difference at all is another hand; a tolerance here
// would hide precisely the small corrections worth seeing.
inline bool home_agrees(const OrganParam& e, const OrganContest& c) {
    const int n = lanes_of(e.type);
    for (int l = 0; l < n; ++l)
        if (read_lane(e, l) != c.written[l]) return false;
    return true;
}

// Once a frame, at the boundary, beside the flush. Untouched dials cost a
// branch: a dial the panel has never written has nothing to be contested.
inline void observe_frame() {
    for (size_t i = 0; i < kOrganParamCount; ++i) {
        OrganContest& c = g_contest[i];
        if (!c.seen) continue;
        if (home_agrees(kOrganParams[i], c)) {
            if (c.disagreed == 0) ++c.survived;   // still standing
        } else {
            ++c.disagreed;
        }
    }
}

inline int contest_class(size_t i) {
    const OrganContest& c = g_contest[i];
    if (!c.seen || c.disagreed == 0) return ORGAN_CONTEST_FREE;
    return c.survived <= 1 ? ORGAN_CONTEST_PER_FRAME : ORGAN_CONTEST_EVENT;
}

// ─── THE DEFINITION WRITE PATH (O1b) ──────────────────────────────────
// Writing a definition does NOT write the instance. That is the whole
// point: the panel says what the mood MEANS, and the mood's own apply is
// what turns that into an instance — the same apply that runs on every
// mood change, unchanged, with nothing duplicated beside it. The panel
// stays a VIEW of the program (docs/ORGAN.md) even while it is editing
// the program's definitions.
//
// The re-apply is deferred to the frame boundary for the same reason the
// flush is: a slider drag is many events, and the mood should be applied
// once. The cartridge takes the flag there, because the cartridge is the
// layer that owns both the mood deps and the queue; this file knows
// neither and should not learn them.
inline bool     g_def_dirty = false;
inline uint32_t g_def_dirty_mood = 0;
inline bool     g_tier_def_dirty = false;   // ORGAN_2b — the world bank changed
inline bool     g_orb_def_dirty  = false;   // ORGAN_3b — the orb mood bank changed

// One base per definition family. MOOD selects by target; TIER is the
// world's single bank and ignores it.
inline char* definition_base(const OrganParam& e, uint32_t mood) {
    switch (e.def_kind) {
    case ORGAN_DEF_MOOD: return reinterpret_cast<char*>(&the_board::mood_def(mood));
    case ORGAN_DEF_TIER: return reinterpret_cast<char*>(&the_board::TIER_LIVE);
    case ORGAN_DEF_BEHAVIOR: return reinterpret_cast<char*>(&the_board::BEHAVIOR_LIVE);
    case ORGAN_DEF_ORB_MOOD:
        return reinterpret_cast<char*>(
            &the_board::ORB_MOOD_LIVE[mood % the_board::MOOD_COUNT]);
    default:             return nullptr;
    }
}

inline bool write_definition(const OrganParam& e, uint32_t mood, const float* in) {
    if (e.def_kind == ORGAN_DEF_NONE) return false;

    char* p = definition_base(e, mood);
    if (!p) return false;
    p += e.def_offset;

    // ORGAN_3b P3 — A U32 OR BOOL DEFINITION CONVERTS; IT DOES NOT
    // REINTERPRET. ORGAN_1 refused these outright, and the reason it gave
    // was reinterpretation — writing a float's bit pattern into an integer
    // field. That reason is sound and this is not that: it is the SAME
    // conversion organ_set's instance path has done since ORGAN_0, applied
    // on the definition side. The rule that survives is "never reinterpret";
    // the rule that had to go was "therefore refuse", which was only ever
    // true because no definition target had been an integer.
    //
    // The MOOD family is unaffected — every MoodProfile target is a float
    // run. ORGAN_3b's orb bank is what needed it: `count`, `enabled` and
    // the three id choices are the sky's most useful dials, and a panel
    // that could not turn them would have been a panel with a hole in it.
    if (e.type == ORGAN_U32 || e.type == ORGAN_BOOL) {
        uint32_t v = (uint32_t)(in[0] < 0.0f ? 0.0f : in[0]);
        std::memcpy(p, &v, sizeof(v));
    } else {
        const int n = lanes_of(e.type);
        for (int l = 0; l < n; ++l) {
            float f = in[l];
            if (f < e.minv) f = e.minv;
            if (f > e.maxv) f = e.maxv;
            std::memcpy(p + l * sizeof(float), &f, sizeof(float));
        }
    }
    // TIER and BEHAVIOR share one author — upload_agent_registries_to_gpu
    // reads both banks — so they share one flag and one boundary re-speak.
    // A second flag would be a second name for one occasion.
    if (e.def_kind == ORGAN_DEF_TIER || e.def_kind == ORGAN_DEF_BEHAVIOR) {
        g_tier_def_dirty = true;
    } else if (e.def_kind == ORGAN_DEF_ORB_MOOD) {
        // Its own author, so its own flag — the converse of BEHAVIOR's
        // case, and the same rule: the flag names the occasion.
        g_orb_def_dirty = true;
    } else {
        g_def_dirty = true; g_def_dirty_mood = mood;
    }
    return true;
}

inline float read_definition(const OrganParam& e, uint32_t mood, int lane) {
    if (e.def_kind == ORGAN_DEF_NONE || lane < 0 || lane >= lanes_of(e.type))
        return 0.0f;
    const char* p = definition_base(e, mood);
    if (!p) return 0.0f;
    p += e.def_offset;
    // The read mirrors the write, and read_lane's instance branch, exactly:
    // an integer definition is CONVERTED back, never reinterpreted.
    if (e.type == ORGAN_U32 || e.type == ORGAN_BOOL) {
        uint32_t u = 0;
        std::memcpy(&u, p, sizeof(u));
        return static_cast<float>(u);
    }
    float v = 0.0f;
    std::memcpy(&v, p + lane * sizeof(float), sizeof(float));
    return v;
}

// Taken once, by the frame boundary. Returns false when there is nothing
// to re-apply, so the caller pays a branch on a quiet frame.
inline bool take_definition_dirty(uint32_t& mood) {
    if (!g_def_dirty) return false;
    g_def_dirty = false;
    mood = g_def_dirty_mood;
    return true;
}

// ─── DOORS (ORGAN_3b) ─────────────────────────────────────────────────
// A DOOR IS THE PANEL PRESSING THE PROGRAM'S OWN MACHINERY. It adds no
// author, invents no behavior, and opens no write path the panel did not
// already have: it raises flags the frame boundary already consumes, and
// the boundary then does exactly what it does every frame. That is the
// whole mechanism, and it is why doors do not violate the sovereignty
// boundary — a door that DID something of its own would be a second
// author wearing a button.
//
// The table below carries ids and labels ONLY. Behavior lives in the
// cartridge, where the deps are; this file knows neither the mood state
// nor the queue and must not learn them (the take_definition_dirty
// precedent, ORGAN_1).
enum : uint32_t {
    ORGAN_DOOR_RESPEAK = 0,   // raise every definition flag at once
    ORGAN_DOOR_COUNT   = 1,
};

struct OrganDoor { uint32_t id; const char* label; };

inline constexpr OrganDoor kOrganDoors[] = {
    { ORGAN_DOOR_RESPEAK, "Re-speak definitions" },
};
static_assert(sizeof(kOrganDoors) / sizeof(kOrganDoors[0]) == ORGAN_DOOR_COUNT,
    "one row per door id — the manifest emits this table and the shell "
    "renders one button per row, so a missing row is a missing button");

// A BITMASK, so presses coalesce by construction: three clicks between
// two frame boundaries are one raise, exactly as a slider drag is one
// WriteBuffer. Same reconciliation philosophy as the flush itself.
inline uint32_t g_doors_pending = 0;

inline uint32_t take_doors_pending() {
    const uint32_t m = g_doors_pending;
    g_doors_pending = 0;
    return m;
}

// The tier bank's re-apply, taken once by the frame boundary (the
// cartridge, which owns the agents' deps and the queue — this file
// knows neither).
inline bool take_tier_definition_dirty() {
    if (!g_tier_def_dirty) return false;
    g_tier_def_dirty = false;
    return true;
}

// ORGAN_3b — the orb mood bank's re-apply, taken once by the frame
// boundary. Its applier is configure_orbs, which the cartridge can reach
// and this file cannot.
inline bool take_orb_definition_dirty() {
    if (!g_orb_def_dirty) return false;
    g_orb_def_dirty = false;
    return true;
}

} // namespace organ
} // namespace t7

// ═══ THE C ABI ═══════════════════════════════════════════════════════
// extern "C" so ccall/cwrap can reach it by name; KEEPALIVE so the linker
// does not garbage-collect a function no C++ caller has.
#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {

// Built lazily, once, into a static string whose c_str outlives the call.
// Carries the CURRENT value of every entry, so the panel opens showing the
// program rather than showing its own defaults — a VIEW, per the charter.
EMSCRIPTEN_KEEPALIVE inline const char* organ_manifest(void) {
    using namespace t7::organ;
    static std::string json;
    json.clear();
    json.reserve(kOrganParamCount * 220 + 32);
    json.push_back('[');
    char buf[512];
    for (size_t i = 0; i < kOrganParamCount; ++i) {
        const OrganParam& e = kOrganParams[i];
        if (i) json.push_back(',');
        std::snprintf(buf, sizeof buf,
            "{\"id\":\"%s\",\"label\":\"%s\",\"group\":\"%s\",\"block\":%u,"
            "\"offset\":%u,\"type\":%u,\"min\":%g,\"max\":%g,\"step\":%g,"
            "\"couple\":%u,\"def\":%u,\"ro\":%u,\"cad\":%u,\"v\":[",
            e.id, e.label, e.group, (unsigned)e.block, (unsigned)e.offset,
            (unsigned)e.type, e.minv, e.maxv, e.step, (unsigned)e.couple,
            (unsigned)e.def_kind,
            (unsigned)(e.ro ? 1u : 0u),
            (unsigned)derived_cadence(e));   // ORGAN_3b — derived, not stored
        json += buf;
        const int n = lanes_of(e.type);
        for (int l = 0; l < n; ++l) {
            std::snprintf(buf, sizeof buf, "%s%g", l ? "," : "", read_lane(e, l));
            json += buf;
        }
        json += "]}";
    }
    json.push_back(']');
    return json.c_str();
}

// ORGAN_3b — the door roster, emitted separately so the dial manifest's
// shape is untouched and a shell that predates doors keeps working.
EMSCRIPTEN_KEEPALIVE inline const char* organ_doors(void) {
    using namespace t7::organ;
    static std::string json;
    json.clear();
    json.push_back('[');
    char buf[256];
    for (size_t i = 0; i < ORGAN_DOOR_COUNT; ++i) {
        if (i) json.push_back(',');
        std::snprintf(buf, sizeof buf, "{\"i\":%u,\"l\":\"%s\"}",
                      kOrganDoors[i].id, kOrganDoors[i].label);
        json += buf;
    }
    json.push_back(']');
    return json.c_str();
}

// Writes the member and sets the block's dirty bit. It does NOT upload:
// the flush is once a frame at the frame boundary, so a slider drag is many
// of these calls and one WriteBuffer (docs/ORGAN.md, "The write path").
//
// TARGET (O1b). -1 is PREVIEW: write the instance, which is what the
// program is showing and what its other authors may take back. A mood id
// (0..MOOD_COUNT-1) is DEFINITION: write what that mood MEANS and let the
// mood's own apply produce the instance. Definition is the panel's
// default; preview is for seeing a value before committing it. A dial with
// no definition target falls back to the instance under either mode —
// there is no definition for it to write, which is exactly the finding
// O1a's census hands to ORGAN_2.
EMSCRIPTEN_KEEPALIVE inline void organ_set(int block, int offset, int type,
                                           float x, float y, float z, float w,
                                           int target) {
    using namespace t7::organ;
    const OrganParam* e = find_entry(block, offset, type);
    if (!e)    { ++g_rejected; return; }   // not in the manifest: refused
    if (e->ro) { ++g_rejected; return; }   // a witness, not a dial (ORGAN_2a)
    if (is_defonly(e->block)) {            // definition-only (ORGAN_2b):
        const float lanes_only[4] = { x, y, z, w };
        if (target < 0 || !write_definition(*e, (uint32_t)target, lanes_only))
            ++g_rejected;                  // no instance exists to fall back to
        return;
    }
    void* base = block_base((uint8_t)block);
    if (!base) { ++g_rejected; return; }

    const float lanes_in[4] = { x, y, z, w };
    if (target >= 0 && write_definition(*e, (uint32_t)target, lanes_in)) {
        // Deliberately no instance write, no dirty bit and no note_write:
        // the instance is the mood apply's to produce, and the contest
        // instrument must keep measuring the instance rather than a value
        // the panel put there on the definition's behalf.
        return;
    }

    char* p = static_cast<char*>(base) + e->offset;
    if (type == ORGAN_U32 || type == ORGAN_BOOL) {
        uint32_t v = (uint32_t)(x < 0.0f ? 0.0f : x);
        std::memcpy(p, &v, sizeof(v));
    } else {
        const int n = lanes_of((uint8_t)type);
        for (int l = 0; l < n; ++l) {
            float v = lanes_in[l];
            if (v < e->minv) v = e->minv;
            if (v > e->maxv) v = e->maxv;
            std::memcpy(p + l * sizeof(float), &v, sizeof(float));
        }
    }
    g_home->organ_mark_dirty((uint32_t)block);
    // ORGAN_3b P3 — A BLOCK WITH A BOUNDARY RAISES ITS AUTHOR'S FLAG.
    // The orb console's only reader is configure_orbs, which is also the
    // orb mood bank's applier — one author, so one flag, the same rule
    // BEHAVIOR follows against TIER. Without this the three console dials
    // would wait for a mood change with nothing on the panel saying so.
    if (block_has_boundary((uint8_t)block)) g_orb_def_dirty = true;
    note_write(*e);   // O1a — the shadow, read back from the home
}

EMSCRIPTEN_KEEPALIVE inline float organ_get(int block, int offset, int lane) {
    using namespace t7::organ;
    for (size_t i = 0; i < kOrganParamCount; ++i) {
        const OrganParam& e = kOrganParams[i];
        if (e.block == block && e.offset == offset) return read_lane(e, lane);
    }
    return 0.0f;
}

// The panel's own witnesses, read once per frame by its status line.
EMSCRIPTEN_KEEPALIVE inline int organ_rejected_count(void) {
    return (int)t7::organ::g_rejected;
}
// Blocks the panel's edits RECONCILED on the last frame boundary — not
// blocks written at that boundary. config_ counts here and uploads in the
// spine (O1d); the panel labels this "reconciled" for exactly that reason.
EMSCRIPTEN_KEEPALIVE inline int organ_flush_count(void) {
    return t7::organ::g_home
         ? (int)t7::organ::g_home->organ_last_flush_count() : 0;
}
EMSCRIPTEN_KEEPALIVE inline int organ_param_count(void) {
    return (int)t7::organ::kOrganParamCount;
}

// O1a — the contest reading for one manifest entry, by its index in the
// manifest (which IS its index in kOrganParams: the manifest is emitted in
// table order, so the panel never has to carry a second key).
// 0 free, 1 event, 2 per-frame.
EMSCRIPTEN_KEEPALIVE inline int organ_contest(int index) {
    using namespace t7::organ;
    if (index < 0 || (size_t)index >= kOrganParamCount)
        return ORGAN_CONTEST_FREE;
    return contest_class((size_t)index);
}

// The evidence behind that reading: frames the panel's last write STOOD
// before the home first disagreed. For a dial still standing this keeps
// climbing, and a large number is how a FREE reading earns confidence.
EMSCRIPTEN_KEEPALIVE inline int organ_contest_frames(int index) {
    using namespace t7::organ;
    if (index < 0 || (size_t)index >= kOrganParamCount) return 0;
    return (int)g_contest[index].survived;
}

// O1b — the mood the program is in. The panel needs it to address a
// definition and to key an export, and it must not keep its own copy.
EMSCRIPTEN_KEEPALIVE inline int organ_mood(void) {
    return (int)t7::organ::current_mood();
}

// ORGAN_3b — press a door. Out-of-range ids are ignored rather than
// counted as rejections: a rejection means the panel asked for something
// the manifest forbids, and a door id the build does not carry is a stale
// shell, not a refused write.
EMSCRIPTEN_KEEPALIVE inline void organ_door(uint32_t id) {
    using namespace t7::organ;
    if (id < ORGAN_DOOR_COUNT) g_doors_pending |= (1u << id);
}

// O1b — one lane of one dial's DEFINITION for one mood. Zero for a dial
// that has no definition target; the panel asks the manifest's "def"
// before it asks this.
EMSCRIPTEN_KEEPALIVE inline float organ_def_get(int index, int mood, int lane) {
    using namespace t7::organ;
    if (index < 0 || (size_t)index >= kOrganParamCount || mood < 0) return 0.0f;
    return read_definition(kOrganParams[index], (uint32_t)mood, lane);
}

} // extern "C"
