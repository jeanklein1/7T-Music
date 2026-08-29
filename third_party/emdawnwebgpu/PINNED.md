# PINNED — emdawnwebgpu

## ROLE (amended at WEB_SUNSET)

The web build this payload served is gone (tag web-sunset). The payload
STAYS as a pinned header surface: glaw1 and console_gate's CARTRIDGE
tier compile against `webgpu_cpp/include` here. It is the WEB-TARGET
generation of revision 56f332d7 — same generation as the native pin,
NOT the same surface: it lacks the Dawn-native extensions, which is why
the CONSOLE tier compiles against `third_party/dawn_native_headers`
instead (its own PINNED.md, same revision, L37). The sha receipts below
are unchanged and still bind.

generation: v20260814.182433
zip sha256: c2983584f5841de83fc7c460abe9385392df1de05151d4e487394f8319c15e0e
fetched: 2026-08-16 (F5F-a)
law: the pin is the vendored payload itself; this file is its
receipt. The build consumes ONLY the in-tree port (F5F-b); a
missing payload is a configure-time FATAL, never a fallback.
witness: the payload must declare SetImmediates,
ImmediateAddressSpace and the immediate-size limit — the symbols the
tree calls (render_passes.hpp shadow_slot). world.wgsl is PLAIN WGSL —
no `requires` directive, no immediate address space — and
tools/wgsl_gate.py polices that absence with its lane regex, per commit.

## The artifact

| field | value |
|---|---|
| upstream | `google/dawn` |
| release tag | `v20260814.182433` |
| tagged commit | `56f332d7d8d03f36149f201ab8cce8aee187e8c6` |
| asset | `emdawnwebgpu_pkg-v20260814.182433.zip` |
| zip sha256 | `c2983584f5841de83fc7c460abe9385392df1de05151d4e487394f8319c15e0e` |
| zip bytes | 133561 |
| vendored at | `third_party/emdawnwebgpu/emdawnwebgpu_pkg/` (18 files) |
| port file | `third_party/emdawnwebgpu/emdawnwebgpu_pkg/emdawnwebgpu.port.py` |

`VERSION.txt` inside the payload names revision
`56f332d7d8d03f36149f201ab8cce8aee187e8c6`, byte-equal to the tagged commit
the F5-a census read from upstream — so the artifact and the record agree
without either being asked to trust the other.

The payload's bytes are upstream's. `.gitattributes` marks the whole
directory `-text` so `* text=auto` cannot translate them on a Windows
checkout; that is what keeps the sha256 above re-checkable anywhere.

## The witness, re-run against the bytes

F5-a's census read `src/dawn/dawn.json` — the authority the header is
*generated from*. This one reads the generated header itself, which is what
the compiler will actually open:

| the tree calls | the payload declares |
|---|---|
| `pass.SetImmediates(0, &li, sizeof(uint32_t))` (render_passes.hpp) | `webgpu_cpp.h:1607/1633/1666` `SetImmediates(uint32_t, void const*, size_t)` — render pass, compute pass, render bundle |
| `d.immediateSize = …` (renderer.hpp) | `webgpu_cpp.h:1892` `PipelineLayoutDescriptor::immediateSize` |
| `maxImmediateSize` (NEEDS r7) | `webgpu_cpp.h:2397` `Limits::maxImmediateSize` |

A WITNESS ATTESTS THE ARTIFACT, NOT THE INTENTION.

## history

Below is F5-a's record, kept verbatim. It is the account of a pin that was
correct on paper and absent on disk — every fact it states about the
upstream tree still holds, and its "The bytes — FLAGGED, not vendored"
section is the debt F5F-a paid. Two of its closing claims are superseded and
are left standing as history rather than corrected: the fetch it calls
impossible succeeded from this session (the proxy serves the release-asset
host even though it still refuses `api.github.com`), and "No build-system
edit follows" is exactly what F5F-b overturned — the preference it describes
was the defect, because a preference can silently choose the other thing.

### emdawnwebgpu — THE PIN (F5-a, 2026-08-14)

*one-generation law — the program's WebGPU generation is this pin; a
reference document without a stated revision is RECALLED, not CITED.
(Until SUNSET_0 the law had a second clause, that the native checkout
track the pin. That clause is history: the twin was archived at tag
`native-sunset` instead of caught up.)*

DOMESDAY_2 F5-a. This is the law's first enforcement.

## The pin

| field | value |
|---|---|
| upstream | `google/dawn` |
| release tag | **`v20260814.182433`** |
| tagged commit | `56f332d7d8d03f36149f201ab8cce8aee187e8c6` |
| asset | `emdawnwebgpu_pkg-v20260814.182433.zip` |
| asset URL | `https://github.com/google/dawn/releases/download/v20260814.182433/emdawnwebgpu_pkg-v20260814.182433.zip` |
| port file, inside the zip | `emdawnwebgpu_pkg/emdawnwebgpu.port.py` |
| extracted path this repo expects | `third_party/emdawnwebgpu/emdawnwebgpu_pkg/emdawnwebgpu.port.py` |
| asset sha256 | **OWED** — see "The bytes" below |

Every row above is read from the upstream tree at the tagged commit, not
inferred: the tag/version identity from
`.github/workflows/package-emdawnwebgpu.sh` (`PKG_VERSION=v${VERSION_DATETIME}`,
where `VERSION_DATETIME` is the tagged commit's own date — recomputed here
and byte-equal to the tag), the asset name from `PKG_FILE=emdawnwebgpu_pkg-${PKG_VERSION}.zip`,
the URL from that script's `EXTERNAL_PORT` template, the in-zip port path
from its `PORT_FILE`, and the extraction shape from the packaging line
`(cd out/wasm && zip -9roX - emdawnwebgpu_pkg > ...)`, which roots the
archive at a single `emdawnwebgpu_pkg/` directory.

## The census gate — PASSED

The gate asks whether this generation speaks the statute. Verified against
`src/dawn/dawn.json` at the tagged commit, which is the authority the
package's C++ header is *generated from*:

| required | found |
|---|---|
| `SetImmediates` | `"set immediates"` × 3 (render pass / compute pass / render bundle encoders) |
| `immediateSize` | `"immediate size"` × 1 (pipeline layout descriptor) |
| `maxImmediateSize` | `"max immediate size"` × 1 (limits) |
| `WGSLLanguageFeatureName::ImmediateAddressSpace` | `{"value": 11, "name": "immediate address space", "jsrepr": "'immediate_address_space'"}` |

Two findings beyond the gate, both load-bearing:

1. **The feature is STANDARD at this generation, not Dawn-flavoured.** Entry 11
   sits in the untagged block; every Dawn extension in the same enum carries
   an explicit `"tags": ["dawn"]` (they begin immediately below it). At the
   native checkout's older revision it was Dawn-tagged — that difference is
   the generational gap this pin closes.
2. **`FeatureStatus::kShipped`** — `src/tint/lang/wgsl/feature_status.cc` puts
   `kImmediateAddressSpace` in the *Shipped* group, the top tier. Per
   `feature_status.h`, shipped features are exposed by default: at this
   generation the dialect needs no instance control, no experimental flag and
   no unsafe flag. The whole F3 enablement apparatus is a courtesy to older
   generations, not a requirement of this one.

## The bytes — FLAGGED, not vendored

**Expected:** the release zip vendored beside this file.
**Found:** CC's execution environment cannot fetch it. The session's proxy
serves anonymous *git* reads of public GitHub repositories (which is how
every fact above was verified, from a shallow clone at the tag) but refuses
`github.com` HTTP — the releases API, the releases page and the asset
download URL all return 403 — and attaching the repository with credentials
was declined. Dawn's own sanctioned way to build the package locally
(`src/emdawnwebgpu/README.md`) requires **emsdk** (`emcmake cmake` →
`emdawnwebgpu_pkg`, whose target compiles `emdawnwebgpu_c`), and no emsdk
exists in that environment.

Hand-assembling a look-alike from the source tree was considered and
**refused on principle**: an unofficial package with no upstream hash is
exactly "a reference document without a stated revision," and vendoring one
as this law's first enforcement would defeat the law it enforces.

### To complete the pin (about a minute)

```
curl -L -o emdawnwebgpu_pkg-v20260814.182433.zip \
  https://github.com/google/dawn/releases/download/v20260814.182433/emdawnwebgpu_pkg-v20260814.182433.zip
sha256sum emdawnwebgpu_pkg-v20260814.182433.zip     # record it in the table above
unzip emdawnwebgpu_pkg-v20260814.182433.zip -d third_party/emdawnwebgpu/
test -f third_party/emdawnwebgpu/emdawnwebgpu_pkg/emdawnwebgpu.port.py && echo PINNED
```

No build-system edit follows. CMakeLists already prefers this path the
moment the file exists and says so at configure time; until then it falls
back to the emsdk built-in remote port and says *that*, loudly. Nothing is
silent either way.

### The alternative pin, recorded not taken

The same release publishes `emdawnwebgpu-v20260814.182433.remoteport.py` — a
single text file that pins the package by URL and SHA512 and downloads it at
build time (Emscripten 4.0.10+). It is a smaller thing to vendor, but it
pins by *download* rather than by *sources local*, which is the property
F5-a chose. Recorded so the choice is visible, not so it is reopened.

## What this pin does and does not buy

emdawnwebgpu is a binding layer: it maps `webgpu.h`/`webgpu_cpp.h` onto the
browser's own WebGPU. Pinning it removes the **compile-time** wall (the
emsdk-bundled port's `RenderPassEncoder` has no `SetImmediates`, its
`WGSLLanguageFeatureName` has no `ImmediateAddressSpace`; its `Limits`
already carried `maxImmediateSize`, which is why A3 compiled and misled).

It does **not** decide the runtime lane. `var<immediate>` is compiled by the
*browser's* Tint, so whether the shadow family's immediate actually runs is
Chrome's generation to answer, not this pin's — which is what the court
probe in DOMESDAY_2_REPORT §10 asks, and why the F3 testimony line prints on
the web twin at all.
