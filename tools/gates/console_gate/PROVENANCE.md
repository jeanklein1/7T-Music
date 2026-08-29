# console_gate — PROVENANCE

## `stubs/GLFW/glfw3.h`

| field | value |
|---|---|
| upstream | `pongasoft/emscripten-glfw`, path `external/GLFW/glfw3.h` |
| commit | `81c2f8d4fbe1b5e3c99e09a081569025b8a3dd14` |
| version | GLFW **3.5.1** (`GLFW_VERSION_MAJOR 3` / `MINOR 5` / `REVISION 1`) |
| sha256 | `95ecc16e4875bca18cff863232d5dbb623f3457b05f07efd26fe8bc8a06345b6` |
| fetched | 2026-08-16 (GATE_1) |
| license | zlib/libpng — © 2002-2006 Marcus Geelnard, © 2006-2019 Camilla Löwy. Full text in the header. |

**Why this copy and not another.** This is the header the `contrib.glfw3`
port actually wraps — the port vendors it at `external/GLFW/`, and the API
the web build compiles against is this one, not whatever a system package
or an emsdk checkout happens to hold. The gate exists to catch a symbol
console.hpp names that the real surface lacks; pointed at a different copy
it would answer a different question.

**FLAGGED — the version.** GATE_1's handoff said GLFW 3.4. The port carries
3.5.1. The artifact won: 3.5.1 is what the port wraps today, and vendoring
3.4 would make the gate agree with a document instead of with the build.
`GLFW_SCALE_FRAMEBUFFER` (CAP_1's switch) is present in both.

**Byte-identical to upstream, deliberately.** The handoff asked for the
provenance banner inside the header. It lives here instead, because a
vendored artifact whose bytes you have edited can no longer be checked
against the sha256 above — the same law F5F installed for the emdawnwebgpu
payload (`third_party/emdawnwebgpu/PINNED.md`), and the same reason
`.gitattributes` marks that payload `-text`. Verify with:

```
sha256sum tools/gates/console_gate/stubs/GLFW/glfw3.h
```

## `stubs/GLFW/emscripten_glfw3.h`

| field | value |
|---|---|
| upstream | `pongasoft/emscripten-glfw`, path `include/GLFW/emscripten_glfw3.h` |
| commit | `81c2f8d4fbe1b5e3c99e09a081569025b8a3dd14` |
| sha256 | `b6f01b0844523fc23b13348f2fa18d9fec0ee010c4de79ac3b5d401286827059` |
| fetched | 2026-08-16 (GATE_1) |
| license | Apache-2.0 — © 2024 pongasoft. Full text linked in the header. |

The port's own public header, vendored rather than stubbed for the same
reason as `glfw3.h`: `emscripten_glfw_make_canvas_resizable`'s real
signature is what FRAME_0 calls, and a stub would only prove that our call
matches our own guess. Byte-identical to upstream.

The gate found this one itself — its first run failed on the missing
include rather than passing over it, which is the behaviour the unit was
built for.

## `stubs/GLFW/glfw3native.h`

**Ours, not vendored** — the one hand-written file in this GLFW directory,
and the only stub WEB_SUNSET added. It carries the standing banner
`STUB — declarations only, the native boot remains the runtime witness`.

Upstream's `glfw3native.h` cannot be vendored here for the reason the
stub's own banner gives: under `GLFW_EXPOSE_NATIVE_X11` it includes
`<X11/Xlib.h>` and Xrandr, under `_WIN32` it includes `<windows.h>`, and a
gate that needed a platform SDK would run on one machine instead of on
any. Declaring the three getters `console.hpp` names, behind the same
expose macros upstream gates them with, asks the only question this gate
can honestly ask: does OUR call match the shape the surface sources hold?

The return types are opaque and chosen against the real structs rather
than guessed — `SurfaceSourceXlibWindow::display` is `void *` and
`::window` is `uint64_t`; `SurfaceSourceWindowsHWND::hwnd` and
`::hinstance` are both `void *` (`third_party/dawn_native_headers`,
`dawn/webgpu_cpp.h`). `glfwGetCocoaWindow` is declared and never called:
`initSurface`'s platform chain is `_WIN32` / `__linux__` only, so on
macOS the surface descriptor is left unchained. Declared anyway, so the
stub matches upstream's shape rather than this host's.

## `stubs/emscripten*.h`, `stubs/GL/gl.h`

Ours, not vendored. Each declares only what `console.hpp` names, and each
carries the banner `STUB — declarations only, the web boot remains the
runtime witness`. They are the gate's boundary, stated the way glaw1 states
its own: this harness certifies OUR names, scope and types; the SDK surface
behind these declarations is the boot's jurisdiction.

Field names and types inside `EmscriptenTouchEvent` / `EmscriptenTouchPoint`
match upstream on purpose — if Emscripten renames one, this gate should fail
before the phone does.

## `stubs/webgpu` — absent, on purpose

There is no WebGPU stub. The gate compiles against the **vendored
emdawnwebgpu payload** at
`third_party/emdawnwebgpu/emdawnwebgpu_pkg/` — the real generated
`webgpu_cpp.h`, pinned by `PINNED.md`. That is the whole point: a stub
would have happily accepted `SetImmediates` against a generation that did
not have it, which is precisely the hole F5F closed.
