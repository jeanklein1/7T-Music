# PINNED — dawn_native_headers

generation: `v20260814.182433`
revision: `56f332d7d8d03f36149f201ab8cce8aee187e8c6`
vendored: 2026-08-29 (WEB_SUNSET W1b)
law: the pin is the vendored closure itself; this file is its receipt.
console_gate's CONSOLE tier compiles ONLY against these headers, never
against a system copy and never against a checkout's `out/` dir.

## Why this payload exists

`third_party/emdawnwebgpu` is the **web-target** generation of this same
revision. Same generation, NOT the same surface: it carries no
Dawn-native extensions, so five names `console.hpp`'s native arms use are
simply absent from it —

| the arm names | emdawnwebgpu | here |
|---|---|---|
| `wgpu::DawnTogglesDescriptor` | absent | `dawn/webgpu_cpp.h` |
| `wgpu::DawnWireWGSLControl` | absent | `dawn/webgpu_cpp.h` |
| `wgpu::Adapter::CreateDevice` | absent (async `RequestDevice` only) | `dawn/webgpu_cpp.h` |
| `wgpu::SurfaceSourceXlibWindow` | SType enumerator only, no struct | `dawn/webgpu_cpp.h` |
| `wgpu::SurfaceSourceWindowsHWND` | SType enumerator only, no struct | `dawn/webgpu_cpp.h` |

`Adapter::CreateDevice` is the one that closed the door on stubs: it is a
member, and nobody adds members from outside. WEB_SUNSET's first attempt
at a native gate stopped on exactly these five, which is the gate working
before it existed.

The one-generation law (L37) now governs two pinned artifacts and the
`C:/dev/dawn` checkout, all at `56f332d7` — the law working, not
straining.

## The closure, and how it was chosen

Not a copy of Dawn's `include/`: the **compiler** chose these ten files.
`console.hpp` and `the_board.cpp` were compiled against the full checkout
with `clang++ -H`, and exactly what the trace named was vendored,
relative paths preserved. Nothing here is unreached; nothing reached is
missing.

| file | kind | sha256 |
|---|---|---|
| `include/dawn/dawn_proc.h` | source | `e071805b66e5702e298f6a3589a7fbf7b63805c11d52ada46caea21759acbbf9` |
| `include/dawn/dawn_proc_table.h` | generated | `dab0539ecd667da2efb9641b0bd6b1e9cd32e2a9b9cd07132792112c5c5b2104` |
| `include/dawn/native/DawnNative.h` | source | `75902a6e1e6d6d4f52f410997bad2f58052e1279150cba9f438ca13273e15070` |
| `include/dawn/native/dawn_native_export.h` | source | `c722ebaa9c0c9fda4d477bb8861cf60fdcbe225aa10ade9c523ee9456c5eb639` |
| `include/dawn/webgpu.h` | generated | `791eca8e9b9dffcd6cde63bb11b000e84541d827ecd1dd22e0c2a48fa81bdac2` |
| `include/dawn/webgpu_cpp.h` | generated | `110fa9a71116114c0309b52a3312b4f3d5ace85da1f04c1d0a38595434509c22` |
| `include/webgpu/webgpu.h` | source | `5316d9fd241e604b3260fa8d4398a458a3dc88ee69c540ac959260b444bdfae3` |
| `include/webgpu/webgpu_cpp.h` | source | `1166cea03743213cbd0a7fe04b01c8bcae8aa117d1370c124572c65b24f74aac` |
| `include/webgpu/webgpu_cpp_chained_struct.h` | generated | `7329fc197c9047ce1c08c3294401836f7f7b24bdf07fbaf0a266253ffa1ac3b8` |
| `include/webgpu/webgpu_enum_class_bitmasks.h` | source | `fd436dc17d050e156ac073b6148d90d388e2585de924ae0274b6aed3067c2a96` |

**Two kinds of receipt, because there are two kinds of file.** The six
SOURCE files are byte-identical to the revision — verified against the
checkout at vendoring time, and re-checkable by anyone who clones it. The
four GENERATED files have no upstream bytes to be identical to; they are
receipted by RECIPE PLUS HASH, and the recipe below reproduces them.

`.gitattributes` marks this directory `-text`, the same law F5F installed
for the emdawnwebgpu payload: `* text=auto` must not translate these
bytes on a Windows checkout, or the hashes above stop re-checking
anywhere.

## The generation recipe, verbatim as run

```sh
git clone --depth 1 https://github.com/google/dawn <dawn>
git -C <dawn> fetch --depth 1 origin 56f332d7d8d03f36149f201ab8cce8aee187e8c6
git -C <dawn> checkout --detach 56f332d7d8d03f36149f201ab8cce8aee187e8c6

python3 generator/dawn_json_generator.py \
  --dawn-json      <dawn>/src/dawn/dawn.json \
  --wire-json      <dawn>/src/dawn/dawn_wire.json \
  --native-json    <dawn>/src/dawn/dawn_native.json \
  --kotlin-json    <dawn>/src/dawn/dawn_kotlin.json \
  --webgpu-kt-docs <dawn>/src/dawn/webgpu_kt_docs.json \
  --targets        headers,cpp_headers \
  --template-dir   <dawn>/generator/templates \
  --root-dir       <dawn> \
  --output-dir     <gen>
```

The argument list is Dawn's own: it is `DawnJSONGenerator` in
`generator/CMakeLists.txt`, read at the pin rather than invented. **No
C++ was built.** The header generation is Python and jinja2 only, which
is why this payload can be regenerated in a container that could never
link Dawn — and why OPEN.md's N-a (the Dawn *library*) is a separate,
still-open acquisition.

No dependency fetch was needed either: `tools/fetch_dawn_dependencies.py`
and `DAWN_FETCH_DEPENDENCIES=ON` (OPEN.md N-a, delta 1) serve a *build*,
and the generator reads only in-repo JSON.

## What this pin does and does not buy

It buys a TYPE surface: `console.hpp` and `the_board.cpp` parse, scope
and type-check as native TUs, on any machine, per commit. It buys the
first proof of the SUNRISE_0 N2 arms outside Windows.

It buys no link and no run. Nothing here is a library; `dawn_native.lib`
remains N-a's business, and THE NATIVE BOOT IS THE WITNESS OF RECORD past
the type surface.
