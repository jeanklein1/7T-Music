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
// position organ_mark_dirty uses. There is deliberately no id for
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
};

// ─── The enrollment macro ─────────────────────────────────────────────
// The id is "block.field" so it is stable across relabelling: export/import
// keys on it, and a label is prose that may be improved without breaking a
// saved file.
#define ORGAN_PARAM(BLOCK, STRUCT, FIELD, TYPE, MIN, MAX, STEP, GROUP, LABEL) \
    OrganParam{ #BLOCK "." #FIELD, LABEL, GROUP,                              \
                ORGAN_BLOCK_##BLOCK,                                          \
                (uint16_t)offsetof(the_board::STRUCT, FIELD),       \
                ORGAN_##TYPE, MIN, MAX, STEP, 0.0f, 0 },

inline const OrganParam kOrganParams[] = {
#include "console/organ_params.inc"
};
#undef ORGAN_PARAM

inline constexpr size_t kOrganParamCount =
    sizeof(kOrganParams) / sizeof(kOrganParams[0]);

// ─── The live home ────────────────────────────────────────────────────
// Bound once at boot. Null until then, and every ABI entry point returns
// harmlessly on null — the panel's JS may be present on a page whose
// program has not finished booting.
inline the_board::GPUState* g_home = nullptr;
inline uint32_t g_rejected = 0;   // refused organ_set calls, shown in the panel

inline void bind_home(the_board::GPUState* s) { g_home = s; }

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
            "\"couple\":%u,\"v\":[",
            e.id, e.label, e.group, (unsigned)e.block, (unsigned)e.offset,
            (unsigned)e.type, e.minv, e.maxv, e.step, (unsigned)e.couple);
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
EMSCRIPTEN_KEEPALIVE inline void organ_set(int block, int offset, int type,
                                           float x, float y, float z, float w) {
    using namespace t7::organ;
    const OrganParam* e = find_entry(block, offset, type);
    void* base = e ? block_base((uint8_t)block) : nullptr;
    if (!e || !base) { ++g_rejected; return; }   // not in the manifest: refused

    char* p = static_cast<char*>(base) + e->offset;
    if (type == ORGAN_U32 || type == ORGAN_BOOL) {
        uint32_t v = (uint32_t)(x < 0.0f ? 0.0f : x);
        std::memcpy(p, &v, sizeof(v));
    } else {
        const float in[4] = { x, y, z, w };
        const int n = lanes_of((uint8_t)type);
        for (int l = 0; l < n; ++l) {
            float v = in[l];
            if (v < e->minv) v = e->minv;
            if (v > e->maxv) v = e->maxv;
            std::memcpy(p + l * sizeof(float), &v, sizeof(float));
        }
    }
    g_home->organ_mark_dirty((uint32_t)block);
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
EMSCRIPTEN_KEEPALIVE inline int organ_flush_count(void) {
    return t7::organ::g_home
         ? (int)t7::organ::g_home->organ_last_flush_count() : 0;
}
EMSCRIPTEN_KEEPALIVE inline int organ_param_count(void) {
    return (int)t7::organ::kOrganParamCount;
}

} // extern "C"
