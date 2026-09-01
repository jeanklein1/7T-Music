#pragma once
// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE SCENE ROAD (THE_PANEL II U1)
//
// THE NATIVE IMPORT WALK. It parses a scene file's `block.field` keys,
// resolves each against THE MANIFEST, and applies it through `organ_set`.
// It is the import path the web panel had, reborn native — same
// semantics, one road.
//
// NAME-BLIND BY CONSTRUCTION, which is the whole acceptance test of this
// campaign: **a new dial is one line in organ_params.inc and zero lines
// here.** Nothing below names a dial, a block, a type or a range. It
// walks `kOrganParams` by index, matches an id string, and hands the row's
// own (block, offset, type) triple to `organ_set` — which clamps, which
// refuses what the manifest does not carry, and which counts and NAMES
// every refusal. THE MANIFEST IS THE WHITELIST and this file does not
// second-guess it.
//
// A SCENE MUST NEVER HALF-APPLY, and that is why the walk is TWO PASSES.
// A file that half-lands leaves behind a world nobody authored: half one
// scene, half another, and no way to say which. So pass one PARSES and
// RESOLVES every key without writing anything, and only a file that
// resolves whole reaches pass two. Every rejection is loud and names the
// key.
//
// THE PARSER IS DELIBERATELY SMALL AND DELIBERATELY STRICT. A scene is a
// flat JSON object of `"id": [numbers]`, which is exactly what the export
// walk writes and exactly what `presets/baseline.json` is. It is not a
// JSON library and does not pretend to be one: nested objects, strings as
// values and trailing garbage are PARSE FAILURES, named by line. The
// tree carries no JSON dependency and this road does not introduce one —
// the same standard `FileWatcher` was held to.
//
// THE HEADER KEYS ARE SKIPPED BY SHAPE, NOT BY NAME. A key whose value is
// not an array of numbers is not a dial row: `schema` is a number,
// `note` is a string. The parser records the schema when it sees one and
// ignores any other non-array key rather than refusing the file, because
// a scene format that cannot carry a comment is a format people work
// around.
// ═══════════════════════════════════════════════════════════════════════

#include "console/organ_registry.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace t7 {
namespace organ {

// THE SCHEMA THIS ROAD KNOWS. `presets/index.json` carries the ledger of
// what each version means and which ids retired at which campaign; this
// number is the one thing the PROGRAM has to agree with it about. A file
// with no `schema` key is read as version 1 — every scene the web panel
// ever wrote is one, and none of them said so.
inline constexpr int SCENE_SCHEMA = 2;

struct SceneRow {
    std::string id;
    float lane[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    int   lanes = 0;
    int   line = 0;
};

struct SceneResult {
    bool ok = false;          // did the file APPLY — false means nothing was written
    int  applied = 0;
    int  refused = 0;         // organ_set said no (a witness, a stale target)
    int  unknown = 0;         // the manifest does not carry this id
    int  schema = 1;
    std::string why;          // set when ok == false
};

// ─── THE PARSER ───────────────────────────────────────────────────────
// One pass over the text, no allocation per token beyond the strings it
// keeps. Whitespace-insensitive; `//` line comments are tolerated because
// a hand-edited scene is the point of the watched file.
inline bool scene_parse(const std::string& text, std::vector<SceneRow>& out,
                        int& schema, std::string& err) {
    size_t i = 0;
    int line = 1;
    auto skip = [&]() {
        while (i < text.size()) {
            const char c = text[i];
            if (c == '\n') { ++line; ++i; }
            else if (c == ' ' || c == '\t' || c == '\r' || c == ',') { ++i; }
            else if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
                while (i < text.size() && text[i] != '\n') ++i;
            } else break;
        }
    };
    auto fail = [&](const char* what) {
        std::ostringstream o;
        o << what << " at line " << line;
        err = o.str();
        return false;
    };
    auto string_at = [&](std::string& s) {
        if (i >= text.size() || text[i] != '"') return false;
        ++i;
        s.clear();
        while (i < text.size() && text[i] != '"') {
            if (text[i] == '\n') ++line;
            s += text[i++];
        }
        if (i >= text.size()) return false;
        ++i;
        return true;
    };

    skip();
    if (i >= text.size() || text[i] != '{') return fail("expected '{'");
    ++i;
    for (;;) {
        skip();
        if (i < text.size() && text[i] == '}') { ++i; break; }
        std::string key;
        if (!string_at(key)) return fail("expected a quoted key");
        skip();
        if (i >= text.size() || text[i] != ':') return fail("expected ':'");
        ++i;
        skip();
        if (i >= text.size()) return fail("unexpected end of file");

        if (text[i] == '[') {
            ++i;
            SceneRow r;
            r.id = key;
            r.line = line;
            for (;;) {
                skip();
                if (i < text.size() && text[i] == ']') { ++i; break; }
                char* end = nullptr;
                const double v = std::strtod(text.c_str() + i, &end);
                if (end == text.c_str() + i) return fail("expected a number");
                if (r.lanes < 4) r.lane[r.lanes] = (float)v;
                ++r.lanes;
                i = (size_t)(end - text.c_str());
            }
            if (r.lanes < 1) return fail("a row needs at least one value");
            if (r.lanes > 4) return fail("a row carries at most four lanes");
            out.push_back(r);
        } else if (text[i] == '"') {
            std::string ignored;
            if (!string_at(ignored)) return fail("unterminated string");
        } else if (text[i] == '{') {
            return fail("nested objects are not a scene");
        } else {
            char* end = nullptr;
            const double v = std::strtod(text.c_str() + i, &end);
            if (end == text.c_str() + i) return fail("expected a value");
            if (key == "schema") schema = (int)v;
            i = (size_t)(end - text.c_str());
        }
    }
    return true;
}

// ─── THE ROW LOOKUP, BY ID ────────────────────────────────────────────
// The manifest's own id — `#BLOCK "." #FIELD`, stable across relabelling,
// which is exactly why an export keys on it. WHOLE-STRING, never a
// prefix: block NAMES are reusable (presets/index.json records the one
// reuse — `WORLD.*` meant the retired world-draw block before it meant
// block 15), and a prefix match would write a live dial from a dead key.
inline const OrganParam* find_by_id(const std::string& id) {
    for (size_t i = 0; i < kOrganParamCount; ++i)
        if (id == kOrganParams[i].id) return &kOrganParams[i];
    return nullptr;
}

// ─── THE WALK ─────────────────────────────────────────────────────────
inline SceneResult apply_scene(const std::string& path) {
    SceneResult res;
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        res.why = "cannot open";
        std::cout << "[Scene] REFUSED " << path << " — cannot open\n";
        return res;
    }
    std::ostringstream buf;
    buf << f.rdbuf();

    std::vector<SceneRow> rows;
    std::string err;
    if (!scene_parse(buf.str(), rows, res.schema, err)) {
        res.why = err;
        std::cout << "[Scene] REFUSED " << path << " — parse: " << err
                  << " (nothing applied)\n";
        return res;
    }

    // THE VERSION IS CHECKED BEFORE ONE KEY IS APPLIED. A schema this
    // road does not know may spell ids it would silently mis-resolve.
    if (res.schema != SCENE_SCHEMA) {
        std::ostringstream o;
        o << "schema " << res.schema << ", this build reads " << SCENE_SCHEMA;
        res.why = o.str();
        std::cout << "[Scene] REFUSED " << path << " — " << o.str()
                  << ". presets/index.json carries the retired-id ledger and "
                     "what each version means (nothing applied)\n";
        return res;
    }

    // PASS ONE — resolve every key, write nothing.
    std::vector<const OrganParam*> hit(rows.size(), nullptr);
    for (size_t k = 0; k < rows.size(); ++k) {
        hit[k] = find_by_id(rows[k].id);
        if (!hit[k]) ++res.unknown;
    }
    if (res.unknown) {
        std::cout << "[Scene] REFUSED " << path << " — " << res.unknown
                  << " key(s) the manifest does not carry, NOTHING APPLIED:\n";
        for (size_t k = 0; k < rows.size(); ++k)
            if (!hit[k])
                std::cout << "[Scene]   line " << rows[k].line << ": "
                          << rows[k].id << "\n";
        std::cout << "[Scene] presets/index.json names the retired ids and the "
                     "campaign each left at.\n";
        res.why = "unknown keys";
        return res;
    }

    // PASS TWO — apply. organ_set clamps, converts and refuses; its
    // refusal counter and its last-reject line are the witnesses.
    const int before = organ_rejected_count();
    bool definition_landed = false;
    for (size_t k = 0; k < rows.size(); ++k) {
        const OrganParam& e = *hit[k];
        const int was = organ_rejected_count();
        organ_set(e.block, e.offset, e.type,
                  rows[k].lane[0], rows[k].lane[1],
                  rows[k].lane[2], rows[k].lane[3], 0);
        if (organ_rejected_count() != was) {
            std::cout << "[Scene]   refused: " << organ_last_reject() << "\n";
        } else {
            ++res.applied;
            if (e.def_kind != ORGAN_DEF_NONE) definition_landed = true;
        }
    }
    res.refused = organ_rejected_count() - before;

    // ONE RESPEAK PER FILE, and only when a definition landed. A
    // definition write does not produce its instance — the fact's own
    // author does, at the frame boundary — so the door is what turns a
    // file of definitions into a world. An instance-only scene needs
    // nothing: those writes already landed where they live.
    if (definition_landed) organ_door(ORGAN_DOOR_RESPEAK);

    res.ok = true;
    std::cout << "[Scene] " << path << " — applied " << res.applied
              << ", refused " << res.refused
              << ", of " << rows.size() << " row(s)"
              << (definition_landed ? "; RESPEAK pressed" : "") << "\n";
    return res;
}

}  // namespace organ
}  // namespace t7
