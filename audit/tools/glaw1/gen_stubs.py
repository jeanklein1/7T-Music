#!/usr/bin/env python3
# G-LAW 1 harness: generate stub SDK headers (webgpu + GLFW) so the REAL
# cartridge TU compiles standalone under g++ -fsyntax-only in-container.
# Project code stays real; only the absent SDK surface is stubbed.
# Member lattice: StubBase (harvested Magic members) -> Magic (harvested
# Magic2 members) -> Magic2 (leaf). operator[]/operator() return StubBase,
# so chains like desc.entries[i].buffer.type resolve.
import re, os

SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "src")
STUB = os.path.join(os.path.dirname(os.path.abspath(__file__)), "stubs")
EXTRA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "members.txt")

os.makedirs(os.path.join(STUB, "webgpu"), exist_ok=True)
os.makedirs(os.path.join(STUB, "GLFW"), exist_ok=True)

def harvest(pattern, dirs, exts=("hpp", "inl", "cpp", "h")):
    out = set()
    for d in dirs:
        p = os.path.join(SRC, d)
        if not os.path.isdir(p):
            continue
        for root, _, files in os.walk(p):
            for f in files:
                if f.split(".")[-1] not in exts:
                    continue
                try:
                    t = open(os.path.join(root, f), encoding="utf-8", errors="ignore").read()
                except OSError:
                    continue
                out |= set(re.findall(pattern, t))
    return out

DIRS = ["cartridges/the_board", "render", "core", "coupling", "analysis", "musical"]
types = harvest(r"wgpu::(\w+)", DIRS)
pairs = harvest(r"wgpu::(\w+)::(\w+)", DIRS)
statics_by_type = {}
for t, s in pairs:
    statics_by_type.setdefault(t, set()).add(s)
types |= set(statics_by_type)
types.discard("wgpu")

members = set()
if os.path.exists(EXTRA):
    members = {l.strip() for l in open(EXTRA) if l.strip()}

m1 = "\n".join(f"    Magic {m};" for m in sorted(members))
m2 = "\n".join(f"    Magic2 {m};" for m in sorted(members))

hdr = f"""// GLAW1 STUB — webgpu_cpp.h stand-in for standalone syntax compiles.
// Not Dawn. Every wgpu type is a Stub with universal member lattice;
// semantics are vacuous, SYNTAX and SCOPE are what this certifies.
#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <initializer_list>
// Dawn's real webgpu_cpp.h transitively provides heavy std headers;
// the stub mirrors that so the TU sees the same std surface as the rig.
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cstring>

namespace wgpu {{

struct StubBase;

struct AnyArg {{
    template<class T> AnyArg(T&&) {{}}
    AnyArg(std::initializer_list<AnyArg>) {{}}
    AnyArg() = default;
}};

// Leaf of the member lattice.
struct Magic2 {{
    const Magic2& operator=(const StubBase&) const;
    template<class... A> StubBase operator()(A&&...) const;
    template<class T> const Magic2& operator=(T&&) const {{ return *this; }}
    template<class T> StubBase operator[](T&&) const;
    template<class T, class = std::enable_if_t<
        std::is_default_constructible_v<std::decay_t<T>>
        && !std::is_base_of_v<StubBase, std::decay_t<T>>>>
    operator T() const {{ return T{{}}; }}
    Magic2 operator|(const Magic2&) const {{ return {{}}; }}
    template<class T> Magic2 operator+(T&&) const {{ return {{}}; }}
    template<class T> Magic2 operator*(T&&) const {{ return {{}}; }}
    template<class T> bool operator==(T&&) const {{ return false; }}
    template<class T> bool operator!=(T&&) const {{ return true; }}
    explicit operator bool() const {{ return false; }}
    Magic2() = default;
}};

struct Magic {{
    const Magic& operator=(const StubBase&) const;
    template<class... A> StubBase operator()(A&&...) const;
    template<class T> const Magic& operator=(T&&) const {{ return *this; }}
    template<class T> StubBase operator[](T&&) const;
    template<class T, class = std::enable_if_t<
        std::is_default_constructible_v<std::decay_t<T>>
        && !std::is_base_of_v<StubBase, std::decay_t<T>>>>
    operator T() const {{ return T{{}}; }}
    Magic operator|(const Magic&) const {{ return {{}}; }}
    Magic operator&(const Magic&) const {{ return {{}}; }}
    Magic operator+(const Magic&) const {{ return {{}}; }}
    template<class T> Magic operator+(T&&) const {{ return {{}}; }}
    template<class T> Magic operator*(T&&) const {{ return {{}}; }}
    template<class T> bool operator==(T&&) const {{ return false; }}
    template<class T> bool operator!=(T&&) const {{ return true; }}
    explicit operator bool() const {{ return false; }}
    Magic() = default;
{m2}
}};

struct StubBase {{
    StubBase() = default;
    template<class T> StubBase(T&&) {{}}
    StubBase(std::initializer_list<AnyArg>) {{}}
    StubBase(AnyArg) {{}}
    StubBase(AnyArg, AnyArg) {{}}
    StubBase(AnyArg, AnyArg, AnyArg) {{}}
    StubBase(AnyArg, AnyArg, AnyArg, AnyArg) {{}}
    StubBase(AnyArg, AnyArg, AnyArg, AnyArg, AnyArg) {{}}
    StubBase(AnyArg, AnyArg, AnyArg, AnyArg, AnyArg, AnyArg) {{}}
    template<class T> StubBase& operator=(T&&) {{ return *this; }}
    template<class T, class = std::enable_if_t<std::is_default_constructible_v<std::decay_t<T>>>>
    operator T() const {{ return T{{}}; }}
    template<class T> bool operator==(T&&) const {{ return false; }}
    template<class T> bool operator!=(T&&) const {{ return true; }}
    explicit operator bool() const {{ return false; }}
{m1}
}};

template<class... A> inline StubBase Magic::operator()(A&&...) const {{ return StubBase{{}}; }}
template<class... A> inline StubBase Magic2::operator()(A&&...) const {{ return StubBase{{}}; }}
template<class T> inline StubBase Magic::operator[](T&&) const {{ return StubBase{{}}; }}
template<class T> inline StubBase Magic2::operator[](T&&) const {{ return StubBase{{}}; }}
inline const Magic& Magic::operator=(const StubBase&) const {{ return *this; }}
inline const Magic2& Magic2::operator=(const StubBase&) const {{ return *this; }}

"""

blocks = [hdr]
for t in sorted(types):
    statics = "\n".join(f"    static inline const Magic {s}{{}};"
                        for s in sorted(statics_by_type.get(t, ())))
    blocks.append(f"""struct {t} : StubBase {{
    using StubBase::StubBase;
    {t}() = default;
    template<class T> {t}& operator=(T&& v) {{ StubBase::operator=(static_cast<T&&>(v)); return *this; }}
{statics}
}};
""")
blocks.append("} // namespace wgpu\n")
open(os.path.join(STUB, "webgpu", "webgpu_cpp.h"), "w").write("\n".join(blocks))

macros = sorted(harvest(r"\b(GLFW_\w+)\b", ["cartridges/the_board", "render", "core", "coupling"]))
fns = sorted(harvest(r"\b(glfw\w+)\s*\(", ["cartridges/the_board", "render", "core", "coupling"]))
g = ["// GLAW1 STUB — GLFW stand-in for standalone syntax compiles.",
     "#pragma once", "struct GLFWwindow;"]
for i, m in enumerate(macros):
    g.append(f"#define {m} {1000 + i}")
for f in fns:
    g.append(f"inline int {f}(...) {{ return 0; }}")
open(os.path.join(STUB, "GLFW", "glfw3.h"), "w").write("\n".join(g) + "\n")

print(f"stub webgpu: {len(types)} types, {len(pairs)} statics, {len(members)} members")
print(f"stub GLFW: {len(macros)} macros, {len(fns)} fns")
