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
// from. A panel that cannot name a thing cannot write it.
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
    ORGAN_BLOCK_COUNT      = 3,
};

// ─── Definition targets (O1b) ─────────────────────────────────────────
// Where a dial's DEFINITION lives, if it has one. An entry's home is its
// INSTANCE — what the program is showing right now — and for a dial with
// a second author the instance is on loan. The definition is the fact the
// author reads when it next speaks, so writing the definition is how a
// panel edit outlives the author (docs/ORGAN.md, "Instance and
// definition").
//
// FLOAT LANES ONLY. Every definition target is a run of floats in
// MoodProfile with the same lane count as the entry's type, because that
// is the only shape the atmospheric group takes. A U32 or BOOL entry with
// a target would need a second conversion and gets refused at the write
// rather than silently reinterpreted.
enum : uint8_t {
    ORGAN_DEF_NONE = 0,   // no definition: the home IS the only truth there is
    ORGAN_DEF_MOOD = 1,   // the_board::MoodProfile, at def_offset, same lanes
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
    uint8_t     def_kind;     // O1b — ORGAN_DEF_NONE | ORGAN_DEF_MOOD
    uint16_t    def_offset;   // byte offset into MoodProfile, when def_kind is MOOD
};

// ─── The enrollment macro ─────────────────────────────────────────────
// The id is "block.field" so it is stable across relabelling: export/import
// keys on it, and a label is prose that may be improved without breaking a
// saved file.
#define ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL) \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(the_board::STRUCT, FIELD),       \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0,                        \
                ORGAN_DEF_NONE, 0 },

// The same line plus the field in MoodProfile the dial DEFINES. The
// compiler takes this offset too, so a rename on the definition side fails
// at the enrollment exactly as a rename on the instance side does.
#define ORGAN_PARAM_DEF(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP,    \
                        LABEL, DEFFIELD)                                      \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(the_board::STRUCT, FIELD),       \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0,                        \
                ORGAN_DEF_MOOD,                                               \
                (uint16_t)offsetof(the_board::MoodProfile, DEFFIELD) },

inline const OrganParam kOrganParams[] = {
#include "console/organ_params.inc"
};
#undef ORGAN_PARAM
#undef ORGAN_PARAM_DEF

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
    default:                     return nullptr;
    }
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

inline float read_lane(const OrganParam& e, int lane) {
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

inline bool write_definition(const OrganParam& e, uint32_t mood, const float* in) {
    if (e.def_kind != ORGAN_DEF_MOOD) return false;
    // Float lanes only, by the rule at the enum: refuse rather than
    // reinterpret. A refusal here falls back to the instance write, which
    // is the honest behaviour — the edit still shows, it just will not last.
    if (e.type == ORGAN_U32 || e.type == ORGAN_BOOL) return false;

    char* p = reinterpret_cast<char*>(&the_board::mood_def(mood)) + e.def_offset;
    const int n = lanes_of(e.type);
    for (int l = 0; l < n; ++l) {
        float v = in[l];
        if (v < e.minv) v = e.minv;
        if (v > e.maxv) v = e.maxv;
        std::memcpy(p + l * sizeof(float), &v, sizeof(float));
    }
    g_def_dirty      = true;
    g_def_dirty_mood = mood;
    return true;
}

inline float read_definition(const OrganParam& e, uint32_t mood, int lane) {
    if (e.def_kind != ORGAN_DEF_MOOD || lane < 0 || lane >= lanes_of(e.type))
        return 0.0f;
    const char* p = reinterpret_cast<const char*>(&the_board::mood_def(mood))
                  + e.def_offset;
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
            "\"couple\":%u,\"def\":%u,\"v\":[",
            e.id, e.label, e.group, (unsigned)e.block, (unsigned)e.offset,
            (unsigned)e.type, e.minv, e.maxv, e.step, (unsigned)e.couple,
            (unsigned)(e.def_kind != ORGAN_DEF_MOOD ? 0u : 1u));
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
    void* base = e ? block_base((uint8_t)block) : nullptr;
    if (!e || !base) { ++g_rejected; return; }   // not in the manifest: refused

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

// O1b — one lane of one dial's DEFINITION for one mood. Zero for a dial
// that has no definition target; the panel asks the manifest's "def"
// before it asks this.
EMSCRIPTEN_KEEPALIVE inline float organ_def_get(int index, int mood, int lane) {
    using namespace t7::organ;
    if (index < 0 || (size_t)index >= kOrganParamCount || mood < 0) return 0.0f;
    return read_definition(kOrganParams[index], (uint32_t)mood, lane);
}

} // extern "C"
