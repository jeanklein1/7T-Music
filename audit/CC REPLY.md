PORT_1 accepted. Starting with P0 recon — no edits: locate the Dawn checkout / emdawnwebgpu / emsdk in this environment, then the HEAD censuses. One flag upfront: the native build's `DAWN_DIR` is `C:/dev/dawn` — a Windows path on your design machine — so the container may not hold the checkout; I'll search broadly and, if absent, fetch a current Dawn release archive purely to *read* the emdawnwebgpu headers for the VERIFY bindings (the build-time package placement stays yours).

The proxy gates GitHub per-repo — requesting read access to `google/dawn`:

Clone landed. Searching the checkout for the emdawnwebgpu package and port:

C++ shapes bound. Now the contrib.glfw3 canvas convention and P0.4's HEAD censuses:

`a6e7d96` pushed — but bash backtick substitution may have mangled the commit message body. Checking:

All edits in. Verifying guard balance, the clock's zero-includer status, and committing via message file (no more backtick casualties):

Now the main() split — native tail calls `init_world()` in place; web tail hands the loop to rAF:

# PORT_1 — CAMPAIGN REPORT

**Landed on master (all pushed):**

| Commit | Unit |
|---|---|
| `a6e7d96` | `PORT_1a: frame() extracted; loop-carried state homed in App` — native-green, gate with `glaw1` + boot |
| `1e7aede` | `PORT_1b: console boot grammar (async-capable, native-synchronous); dt clamp homed; core/clock.hpp retired` — native-green, gate with `glaw1` + boot |
| `15b975a` | `PORT_1c: web build preset + host page; watcher native-only` — gate with the wasm build + Chrome witness |
| — | **`PORT_1d` does not exist**: AllowSpontaneous is verified supported in emdawnwebgpu (`webgpu.cpp:403/666/691`; Dawn's own `InitOnlySample` uses it) — the three MapAsync sites carry unchanged |

**P0 recon — no STOP fired, one environmental deviation reported:** the native Dawn checkout (`DAWN_DIR=C:/dev/dawn`) lives on your design machine and could not be searched from this container. I verified against a fresh shallow clone of `google/dawn` @ `da44df7da` instead: **emdawnwebgpu exists** (`src/emdawnwebgpu/pkg/emdawnwebgpu.port.py` + `third_party/emdawnwebgpu/pkg`), and per its own `pkg/README` the easiest consumption is **Emscripten's built-in remote port** — so the preset defaults to `--use-port=emdawnwebgpu`, with `-DEMDAWNWEBGPU_PORT=path/to/emdawnwebgpu.port.py` to pin a Dawn-release package instead (note: the *source-tree* port file is unusable un-built — it self-checks for generated headers). emsdk is absent in this environment; your one-time install stands as written. contrib.glfw3 confirmed in the ports registry since Emscripten 3.1.55 (upstream currently 3.4.0.20260301 / GLFW 3.4).

**Every VERIFY binding:**
- **CallbackMode**: `WaitAnyOnly / AllowProcessEvents / AllowSpontaneous` all defined; AllowSpontaneous supported → boot chain and readbacks use it.
- **`Surface::GetCapabilities`**: exists (`webgpu.cpp:2157`) → the shared `caps.formats[0]` tail runs on web; **no BGRA8Unorm fallback was needed**.
- **`Surface::Present`**: exists but **aborts at runtime** ("use requestAnimationFrame via html5.h instead") → web `present()` is a hard no-op.
- **Canvas source**: `wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector{ .selector = "#canvas" }` — Emscripten-prefixed, *not* the unprefixed name the handoff expected.
- **`Adapter::GetLimits`**: exists (`library_webgpu.js:775`) → **full-adapter limits passthrough verbatim as native**, covering both known exceedances (9 storage/compute stage, 289 layers).
- **Request shapes**: `CreateInstance()` → `RequestAdapter(nullptr, mode, λ(status, Adapter, StringView))` → `RequestDevice(&desc, mode, λ(status, Device, StringView))`.

**dt clamp**: `std::clamp(dt, 0.0f, 0.1f)` — the dormant clock's exact 100 ms "spiral of death" cap, now in `begin_frame()` both platforms; `core/clock.hpp` removed (re-verified zero includers).

**Preload mapping as landed** (P0 re-verified the live search list, `renderer.hpp:1250-1251` — the handoff's mapping stood unchanged): `assets@/assets` + `world.wgsl@/src/cartridges/the_board/realization/world.wgsl`; the loader's second entry resolves verbatim from MEMFS CWD `/`. Output: `web/the_board.{js,wasm,data}` beside the committed `web/index.html` (dark page, `#canvas`, `Module.canvas` handoff — contrib.glfw3's default selector is literally `Module['canvas']`, its Config.h:37 — plus the pointer-lock hint).

**Forced bindings beyond the letter, each reported in its commit body:** (1) `GLFW_EXPOSE_NATIVE_*` + `glfw3native.h` joined Region 1's native guard — contrib.glfw3 ships no `glfw3native.h`; (2) the post-console init sequence became `static bool init_world()` (one home) because web `main()` has no device yet — native calls it exactly where those lines were, web `frame()` calls it once at Ready; (3) P1a's one token change: the acquire-failure `continue` is `return` in `frame()` (the P1a commit body has two words eaten by a shell-quoting slip — this sentence is the correction; later commits used message files).

**Re-anchors (post-CUT_1)**: loop locals were `incubator_dual.cpp` main() :131/:141/:147/:164/:169/:170 → `struct App`; MapAsync census 3 = agent `cartridge.hpp:1169` + floater `:1212` live, meter `:1921` folded; ESC handler `console.hpp:117-120`; surface seam `initSurface`; frame clock `begin_frame` 393-395 region.

**For your gates**: `glaw1` + boot after P1a (`a6e7d96`) and P1b (`1e7aede`) — identical behavior both times; then emsdk env → `cmake --preset the-board-web && cmake --build --preset the-board-web` → serve `web/` → Chrome. First light = world renders, click captures, WASD moves, ~20 one-time `[SignalLayout]` warns in devtools. A black canvas reads its cause in the console — paste it back verbatim. Still open in parallel: the C8 shell stamp, and the `f62c519` limits-fit note for the next design session.