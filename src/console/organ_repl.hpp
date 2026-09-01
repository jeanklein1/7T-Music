#pragma once
// ═══════════════════════════════════════════════════════════════════════
// ORGAN — THE HAND (THE_PANEL II U2)
//
// A stdin lane on the console tier. Nine verbs, one parser, and THE
// MANIFEST AS ITS WHOLE VOCABULARY:
//
//   list [filter]      sections and rows, `shown/total`
//   get <id>           one row, its value, its range, its cadence
//   set <id> <v…>      clamped by organ_set; a refusal is named
//   doors              the door roster, by id
//   door <n>           press one
//   export <file>      every writable row, as a schema-2 scene
//   import <file>      the scene road (console/organ_scene.hpp)
//   probe <N>          arm the device gate for the next N frames
//   help               the nine lines above
//
// THE ACCEPTANCE TEST, AND IT IS THE CAMPAIGN'S: **a new dial is one line
// in `organ_params.inc` and zero lines here.** Nothing below names a
// dial, a block, a type, a range or a section. `list` derives its
// sections by splitting a row's own group string; `set` reads the lane
// count off the row's type; `get` prints the cadence `derived_cadence()`
// computes. Add a row to the enrollment list and every verb above
// carries it on the next build, unedited.
//
// ONE WRITE ROAD, MANY DOORS ONTO IT. `set` calls `organ_set` and
// `import` calls `apply_scene`, which calls `organ_set`. There is no
// second write path and this file does not create one: it does not touch
// a bank, a flag or a queue. THE MANIFEST IS THE WHITELIST and the REPL
// is a consumer of it like any other.
//
// EXPORT IS DESIGN-TIME LAW, KEPT. "A preset is for DESIGN TIME; at ship
// time the values are transcribed into the C++ tables and the JSON is
// spent — one fact, one home." So `export` writes what a hand can read
// back and nothing more: WITNESSES EXPORT NOTHING, because an `_RO` row
// is a meter and a meter's value is its author's, not a scene's.
//
// AND IT WALKS THE MANIFEST, NOT A STRUCT. THE_PANEL I U3c found nine
// keys in the web era's own `baseline.json` that no `organ_set` call
// could ever have accepted — ARRAY members over a live block, written by
// an export that walked the C++ struct instead of the row list. An export
// that does not walk the manifest writes a file the program cannot read
// back. This one walks `kOrganParams` and can only emit what `import`
// can resolve, which is what makes the round trip a law rather than a
// hope.
//
// THE READ IS NON-BLOCKING AND OWNS NOTHING. The frame loop is a busy
// `while (console.running())` with a blocking `present()`, so a blocking
// `std::getline` on the render thread would stall the world between
// keystrokes. `poll()` on fd 0 answers "is there a line" in microseconds
// and returns immediately when there is not. II's U0 recon established
// that NOTHING in `src/` reads stdin — no `std::cin`, no `getline`, no
// `read(0, …)` — so this lane fights no existing reader.
//
// NOT ON WINDOWS'S PATH YET, AND SAID HERE RATHER THAN DISCOVERED. The
// poll is POSIX. The tree's Windows lane is the Visual Studio build
// (`the-board-vs`), where the equivalent is `WaitForSingleObject` on the
// console handle — a second `#ifdef` arm of this one function, and no
// other line in this file moves. It is not written because it cannot be
// tested from here, and a platform arm nobody can run is worse than an
// absent one that says so.
// ═══════════════════════════════════════════════════════════════════════

#include "console/organ_registry.hpp"
#include "console/organ_scene.hpp"
#include "core/boot_params.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
  #include <poll.h>
  #include <unistd.h>
  #define T7_REPL_POSIX 1
#else
  #define T7_REPL_POSIX 0
#endif

namespace t7 {
namespace organ {

// ─── The cadence words the manifest derives ───────────────────────────
// Restated NOWHERE ELSE: `derived_cadence()` is the one place the rule
// lives and this is only its spelling.
inline const char* cadence_word(uint8_t c) {
    switch (c) {
    case ORGAN_CAD_GEN:      return "gen (on respawn)";
    case ORGAN_CAD_BOUNDARY: return "boundary";
    case ORGAN_CAD_DRIVEN:   return "driven (a meter)";
    default:                 return "live";
    }
}

inline void repl_print_row(const OrganParam& e) {
    std::cout << "  " << e.id << "  " << e.group << " — " << e.label << "\n";
    std::cout << "    value ";
    const int n = lanes_of(e.type);
    for (int l = 0; l < n; ++l)
        std::cout << (l ? ", " : "") << read_lane(e, l);
    std::cout << "\n";
    if (e.ro) {
        std::cout << "    (witness — organ_set refuses it)\n";
    } else {
        std::cout << "    range " << e.minv << " … " << e.maxv
                  << "  step " << e.step << "\n";
    }
    std::cout << "    cadence " << cadence_word(derived_cadence(e)) << "\n";
}

// ─── export ───────────────────────────────────────────────────────────
inline bool repl_export(const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f) {
        std::cout << "[REPL] cannot write " << path << "\n";
        return false;
    }
    f << "{\n \"schema\": " << SCENE_SCHEMA << ",\n";
    f << " \"note\": \"exported by the REPL's manifest walk — every writable "
         "row, and only rows import can resolve\",\n";
    size_t wrote = 0, skipped = 0;
    for (size_t i = 0; i < kOrganParamCount; ++i) {
        const OrganParam& e = kOrganParams[i];
        if (e.ro) { ++skipped; continue; }   // a meter is not a scene's
        f << " \"" << e.id << "\": [";
        const int n = lanes_of(e.type);
        for (int l = 0; l < n; ++l) {
            if (l) f << ", ";
            f << read_lane(e, l);
        }
        f << "]" << (i + 1 < kOrganParamCount ? "," : "") << "\n";
        ++wrote;
    }
    f << "}\n";
    std::cout << "[REPL] exported " << wrote << " row(s) to " << path
              << "; " << skipped << " witness(es) skipped — a meter is not "
                 "a scene's to carry\n";
    return true;
}

// ─── the one command parser ───────────────────────────────────────────
inline void repl_help() {
    std::cout <<
        "[REPL] list [filter] · get <id> · set <id> <v...> · doors · door <n>\n"
        "       export <file> · import <file> · probe <N> · help\n"
        "       ids are the manifest's own `BLOCK.field`; `list` with no\n"
        "       filter prints every section and its count.\n";
}

inline void repl_exec(const std::string& line) {
    std::istringstream in(line);
    std::string verb;
    if (!(in >> verb)) return;

    if (verb == "help") { repl_help(); return; }

    if (verb == "list") {
        std::string filter;
        in >> filter;
        // SECTIONS ARE DERIVED FROM THE ROWS, never listed here. A group
        // is "Section · Group" and a consumer splits on the FIRST
        // separator — the same rule the enrollment file states.
        std::string last;
        size_t shown = 0;
        for (size_t i = 0; i < kOrganParamCount; ++i) {
            const OrganParam& e = kOrganParams[i];
            if (!filter.empty()
                && std::string(e.id).find(filter) == std::string::npos
                && std::string(e.group).find(filter) == std::string::npos)
                continue;
            if (last != e.group) {
                last = e.group;
                std::cout << "  ── " << e.group << "\n";
            }
            std::cout << "     " << e.id << (e.ro ? "  (witness)" : "") << "\n";
            ++shown;
        }
        std::cout << "[REPL] " << shown << "/" << kOrganParamCount << " row(s)"
                  << (filter.empty() ? "" : " matching \"" + filter + "\"")
                  << "\n";
        return;
    }

    if (verb == "get") {
        std::string id;
        if (!(in >> id)) { std::cout << "[REPL] get <id>\n"; return; }
        const OrganParam* e = find_by_id(id);
        if (!e) { std::cout << "[REPL] no row `" << id << "` in the manifest\n"; return; }
        repl_print_row(*e);
        return;
    }

    if (verb == "set") {
        std::string id;
        if (!(in >> id)) { std::cout << "[REPL] set <id> <v...>\n"; return; }
        const OrganParam* e = find_by_id(id);
        if (!e) { std::cout << "[REPL] no row `" << id << "` in the manifest\n"; return; }
        float lane[4] = { 0, 0, 0, 0 };
        int got = 0;
        while (got < 4 && (in >> lane[got])) ++got;
        const int want = lanes_of(e->type);
        if (got < want) {
            std::cout << "[REPL] `" << id << "` takes " << want
                      << " value(s); got " << got << "\n";
            return;
        }
        const int before = organ_rejected_count();
        organ_set(e->block, e->offset, e->type,
                  lane[0], lane[1], lane[2], lane[3], 0);
        if (organ_rejected_count() != before) {
            std::cout << "[REPL] refused: " << organ_last_reject() << "\n";
            return;
        }
        // THE READBACK IS THE POINT. organ_set clamps and converts, so
        // what the hand typed and what the program holds are two
        // different questions — and only the second one is true.
        repl_print_row(*e);
        return;
    }

    if (verb == "doors") {
        for (size_t i = 0; i < ORGAN_DOOR_COUNT; ++i)
            std::cout << "  " << kOrganDoors[i].id << "  "
                      << kOrganDoors[i].label << "\n";
        std::cout << "[REPL] " << (unsigned)ORGAN_DOOR_COUNT << " door(s)\n";
        return;
    }

    if (verb == "door") {
        int n = -1;
        if (!(in >> n)) { std::cout << "[REPL] door <n> — `doors` lists them\n"; return; }
        if (n < 0 || n >= (int)ORGAN_DOOR_COUNT) {
            std::cout << "[REPL] no door " << n << "; there are "
                      << (unsigned)ORGAN_DOOR_COUNT << "\n";
            return;
        }
        organ_door((uint32_t)n);
        std::cout << "[REPL] pressed " << n << " — " << kOrganDoors[n].label
                  << " (taken at the next frame boundary)\n";
        return;
    }

    if (verb == "export") {
        std::string path;
        if (!(in >> path)) { std::cout << "[REPL] export <file>\n"; return; }
        repl_export(path);
        return;
    }

    if (verb == "import") {
        std::string path;
        if (!(in >> path)) { std::cout << "[REPL] import <file>\n"; return; }
        apply_scene(path);   // ONE ROAD — the same walk `--scene=` takes
        return;
    }

    if (verb == "probe") {
        // THE DEVICE GATE, ARMED FROM THE HAND. It reads the same
        // boot_params the flag writes, so a probe asked for here and one
        // asked for at argv are the same run.
        unsigned n = 0;
        if (!(in >> n) || n == 0) { std::cout << "[REPL] probe <N>\n"; return; }
        boot_params().has_probe = true;
        boot_params().probe_frames = n;
        std::cout << "[REPL] probe armed for " << n
                  << " frame(s) — the verdict prints and the program exits\n";
        return;
    }

    std::cout << "[REPL] `" << verb << "`? — try `help`\n";
}

// ─── the non-blocking poll, called once per frame ─────────────────────
inline void repl_poll() {
#if T7_REPL_POSIX
    static bool announced = false;
    if (!announced) {
        announced = true;
        std::cout << "[REPL] the hand is open — `help` for the nine verbs\n";
    }
    for (;;) {
        struct pollfd p { 0, POLLIN, 0 };
        if (::poll(&p, 1, 0) <= 0 || !(p.revents & POLLIN)) return;
        std::string line;
        if (!std::getline(std::cin, line)) return;
        // ONE LINE PER LOOP TURN, and the loop runs until the pipe is
        // empty: a scripted session pasted whole must not take one frame
        // per command, and a hand typing one line must not wait for the
        // next frame to see it.
        if (!line.empty()) repl_exec(line);
    }
#endif
}

}  // namespace organ
}  // namespace t7
