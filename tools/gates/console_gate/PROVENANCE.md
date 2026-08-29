# console_gate — PROVENANCE

> DELETED AT WEB_SUNSET (W5), with the arms that named them:
> `stubs/emscripten.h`, `stubs/emscripten/em_types.h`,
> `stubs/emscripten/fetch.h`, `stubs/emscripten/html5.h` (ours), and the
> vendored `stubs/GLFW/emscripten_glfw3.h`. Their entries are gone with
> them — a receipt for a file that is not here is the false-authority
> class L30 exists for. Resurrection is archaeology from tag
> `web-sunset`.

## `stubs/GLFW/glfw3.h`

| field | value |
|---|---|
| upstream | `pongasoft/emscripten-glfw`, path `external/GLFW/glfw3.h` |
| commit | `81c2f8d4fbe1b5e3c99e09a081569025b8a3dd14` |
| version | GLFW **3.5.1** (`GLFW_VERSION_MAJOR 3` / `MINOR 5` / `REVISION 1`) |
| sha256 | `95ecc16e4875bca18cff863232d5dbb623f3457b05f07efd26fe8bc8a06345b6` |
| fetched | 2026-08-16 (GATE_1) |
| license | zlib/libpng — © 2002-2006 Marcus Geelnard, © 2006-2019 Camilla Löwy. Full text in the header. |

**Why this copy and not another.** It is REAL GLFW 3.5.1, receipted, and
`input.hpp` needs its key codes on the one program — which is why the
stubs directory stays on both gates' include lines after WEB_SUNSET
(glaw1's banner says so). It came here as the header the `contrib.glfw3`
port vendors at `external/GLFW/`; that port went with the web twin, the
bytes did not, and a pinned copy still beats whatever a system package
happens to hold. The gate exists to catch a symbol the tree names that
the real surface lacks; pointed at a different copy it would answer a
different question.

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

## `stubs/GL/gl.h`

Ours, not vendored, and NOT web residue — which is why it survived W5's
sweep of the emscripten stubs beside it. It is insurance for the vendored
`glfw3.h`'s own OpenGL cascade: upstream GLFW includes `<GL/gl.h>` unless
`GLFW_INCLUDE_NONE` is defined, and both gates define it on every run.
Measured at W5 with the flag dropped: the cartridge TU reaches this file,
and with the file also removed the compile is a FATAL ERROR at
`glfw3.h:241`. So nothing reaches it today, one command-line flag is the
whole reason, and it costs one small file to keep that true by
construction rather than by vigilance.

## `stubs/webgpu` — absent, on purpose

There is no WebGPU stub, and after WEB_SUNSET there are TWO real
surfaces rather than one. Tier CARTRIDGE compiles against the vendored
emdawnwebgpu payload at `third_party/emdawnwebgpu/emdawnwebgpu_pkg/`;
tier CONSOLE compiles against `third_party/dawn_native_headers/`, Dawn's
NATIVE generation at the same revision, because the web-target generation
does not carry the extensions `console.hpp`'s arms use. Each has its own
PINNED.md receipt.

That is the whole point, and it has now been paid for twice: a stub would
have happily accepted `SetImmediates` against a generation that did not
have it (the hole F5F closed), and a stub could not have supplied
`Adapter::CreateDevice` at all, because it is a member.
