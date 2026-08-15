# LANTERN U0 — THE CENSUS

*Read-only. Facts from the tree at `9a00f11`, no design, no
recommendation. Sites cite FILE · SYMBOL, never line numbers — a line
number is a position and this page must survive edits above it. Where
document and code disagree, code is fact.*

**Why this page exists.** U2 (walk affordance) is deferred by ruling to
a fresh session. That session inherits this census, not a memory of it.

---

## §A — THE BOOT-REPORT DIVERGENCE

**Mechanism: `#ifdef`, not a runtime branch.** `console.hpp ·
Console::initWebGPU()` is two disjoint bodies — `#ifdef __EMSCRIPTEN__`
… `#else` … `#endif`. The native device-fact block is *compiled out* on
the web. Nothing is skipped at runtime; on the web those lines do not
exist.

### A1 — what each twin prints

| line | twin | site |
|---|---|---|
| `[Console] Adapter <i>: <type>/<backend> \| <device> (<desc>) vendor=` | native | `console.hpp · Console::initWebGPU()` (`#else`), enumeration loop |
| `[Console] Adapter selected: index= backend=` | native | same, after the scorer |
| `[Console] Adapter limits: storageBuffers/stage · uniformBuffers/stage · bindingsPerGroup` | native | same, from `adapterLimits` |
| `[Console] Compiler plan (request):` | native | same, `kCompilerPlan` / `compiler_plan_name()` |
| `[Console] Adapter features (N): <numeric ids>` | native | same, `adapter.GetFeatures()` |
| `[Console] feature multi-draw-indirect= timestamp-query=` | native | same, `adapter.HasFeature()` |
| `[Console] Toggles used (N):` | native | same, `dawn::native::GetTogglesUsed()` |
| `[Device] adapter: vendor \| architecture \| device \| description` | web | `console.hpp · Console::initWebGPU()` (`#ifdef`), RequestAdapter callback |
| `[Device] requesting CORE DEFAULTS` / `FULL ADAPTER PASSTHROUGH` | web | `console.hpp · Console::request_device_web()` |
| `[Device] granted vs floor:` (6 rows since LANTERN U1) | web | same, RequestDevice callback, `!passthrough` branch |
| `[Device] BELOW FLOOR` · `DISCARDING` · `REISSUING` | web | same, conditional |
| `[Device] modest device accepted — NO DISCARD` | web | same |
| `[Device] features: adapter offers N (…); granted timestamp-query=` | web | same, device-adoption site — **added by LANTERN U1** |
| `[Device] KEEPING the device from:` | web | same |

### A2 — the three-way split

| fact | class | note |
|---|---|---|
| adapter identity (vendor/architecture/device/description) | **web-exposed** | browser redacts freely; the web line prints `?` per empty field, by design |
| adapter feature set (count + ids) | **web-exposed** | available via `adapter.GetFeatures()`; **was unprinted until LANTERN U1** |
| device feature grant | **web-exposed** | `device.HasFeature()`; **was unprinted until LANTERN U1** |
| all limits, adapter and device | **web-exposed** | `GetLimits()` both sides; 3 of them printed before U1, 6 now |
| adapter **enumeration** | **native-only by nature** | the web has no enumeration API — `RequestAdapter` returns one adapter |
| compiler plan (`use_dxc` / FXC / DXC) | **native-only by nature** | a Dawn backend-compiler choice; no web meaning |
| Dawn toggle list | **native-only by nature** | `dawn::native::GetTogglesUsed` is Dawn-internal; no web counterpart |
| **TimestampQuery, requested** | **requested, never printed** (pre-U1) | `console.hpp · Console::request_device_web()` requests it when `adapter_.HasFeature(TimestampQuery)`, and printed nothing either way |

### A3 — the inference, CONFIRMED and worse than stated

`cartridge.hpp · Cartridge::initialize()` sets `meter_gpu_ =
device_.HasFeature(wgpu::FeatureName::TimestampQuery)` and prints **only
the negative** (`[METER] timestamp-query unavailable on this adapter —
CPU rows only`). The web console shows GPU timings ⇒ `meter_gpu_` true
⇒ the device carries the feature ⇒ it was granted.

**So the grant's only evidence was the ABSENCE of a failure line.** A
switch witnessed by silence is indistinguishable from one that never
fired (P6). LANTERN U1 prints the grant; this row is closed.

---

## §B — THE WEB CONTROL SURFACE

*For deferred U2. Existence only. `exists` is YES / NO / KEYBOARD-ONLY.*

### B1 — every channel

| channel | reaches | exists | site |
|---|---|---|---|
| URL query parameters | nothing | **NO** | no reader anywhere in repo — searched `location.search`, `URLSearchParams`, `searchParams`, `location.href`, manual `?` splitting across `.hpp/.cpp/.js/.html/.json`. Only hit is a reload button, `web/index.html · #reload` |
| DPR cap | framebuffer size (backing store) | **YES, compile-time only** | `console.hpp · MAX_DEVICE_PIXEL_RATIO` (= 1.5f) → `Console::apply_pixel_cap()`. No runtime channel of any kind |
| touch — stick (one finger, its half) | walk / move intent | **YES** | `console.hpp · Console::emit_touch_intents()` → `InputEvent::TouchMove` → `input.hpp · on_touch_move()` → `update_movement_intent()` |
| touch — drag look | camera look deltas (mouse's own two deltas) | **YES** | `emit_touch_intents()` → `TouchLook` → `input.hpp · on_touch_look()` |
| touch — pinch | zoom (the scroll wheel's channel) | **YES** | `emit_touch_intents()` → `TouchZoom` → `input.hpp · on_touch_zoom()` → `inputState_.zoom_delta` |
| touch — clean tap, LEFT half | aura toggle (key 3's door) | **YES** | `emit_touch_intents()` → `TouchTapLeft` → `input.hpp · on_touch_tap_left()` → `toggle_aura()` |
| touch — clean tap, RIGHT half | possess nearest (CAPS_LOCK's door) | **YES** | `emit_touch_intents()` → `TouchTapRight` → `input.hpp · on_touch_tap_right()` → `try_possess_nearest()` |
| touch stream ownership | — | **YES** | `console.hpp · Console::claim_touch_stream()` deregisters the GLFW port's handlers on `TOUCH_TARGET` (`Module['canvas']`), installs `Console::touch_cb`; backstop `Console::any_touch_active()` suppresses synthesized mouse events |
| keyboard — moods 5/6/7/8 | `request_mood_transition()` → OPEN_SUNSET / INDOOR_FLAT / INDOOR_VAULT / FINITE_OUTDOOR | **KEYBOARD-ONLY** | `input.hpp · handle_key_down()`, cases `GLFW_KEY_5..8` |
| keyboard — aura height (2), aura (3) | pawn command doors | **KEYBOARD-ONLY** (3 also reachable by tap-left) | `input.hpp · handle_key_down()` |
| keyboard — orb palette (0), motion rule (KP8), gesture (KP.) | orb doors | **KEYBOARD-ONLY** | `input.hpp · handle_key_down()` |
| keyboard — render radius (`[` / `]`) | `set_render_radius()` | **KEYBOARD-ONLY** | `input.hpp · handle_key_down()` |
| keyboard — veil dither (V) | `toggle_veil_dither()` | **KEYBOARD-ONLY** | `input.hpp · handle_key_down()` |
| keyboard — look sensitivity (KP+/KP−) | `nudge_look_sensitivity()` | **KEYBOARD-ONLY** | `input.hpp · handle_key_down()` |
| keyboard — FPV / possession (CTRL, CAPS_LOCK) | `toggle_fpv_mode()`, possession | **KEYBOARD-ONLY** (possession also by tap-right) | `input.hpp · handle_key_down()` |
| portal entry | `point_.portal_trigger` (GPU readback on the possessed slot) → world transition | **YES, proximity** | written in `cartridge.hpp · Cartridge::phase_witness_harvest()` MapAsync callback; consumed by `cartridge.hpp · Cartridge::phase_portal_trigger()` |
| shell — DETAILS panel | shows `Module.print` lines; reads nothing back | **YES, read-only** | `web/index.html · #logToggle` / `#logToggle2` |
| shell — reload button | page reload | **YES** | `web/index.html · #reload` |
| shell — viewport resize / orientation | canvas + framebuffer size | **YES, automatic** | `web/index.html · sizeToViewport()` |
| shell — gesture / dblclick suppression | prevents browser zoom | **YES, automatic** | `web/index.html`, `gesturestart` + `dblclick` listeners |
| shell — wake lock | screen stays awake | **YES, automatic** | `web/index.html`, `navigator.wakeLock` |
| meter / window control | — | **NO** | no input path of any kind reaches the meter; see §C |

### B2 — the three targets the handoff named

| target | phone-reachable? | via | site |
|---|---|---|---|
| mood selection | **NO** | nothing — no touch intent, no URL parameter, no DOM hook reaches `request_mood_transition()` | `input.hpp · handle_key_down()` cases `GLFW_KEY_5..8` are the sole callers |
| portal entry | **YES** | walk into it — the stick reaches the possessed slot, the GPU writes the trigger | `cartridge.hpp · Cartridge::phase_portal_trigger()` |
| meter / window control | **NO** | nothing — the window is time-driven only | `cartridge.hpp · Cartridge::phase_census_dumps()` |

**Stated plainly:** a phone visitor can walk, look, zoom, toggle the
aura, possess a pawn, and enter a portal by walking into it. Four moods
exist; a phone reaches **one** — whichever the world booted into, until
a portal changes it.

---

## §C — THE METER'S WINDOW BOUNDARY

| question | answer | site |
|---|---|---|
| what opens/closes a window | one wall-clock cadence: `time_state_.seconds − lastCensusDump_ ≥ CENSUS_DUMP_INTERVAL` | `cartridge.hpp · Cartridge::phase_census_dumps()`; `machine/spawn_engine.hpp · CENSUS_DUMP_INTERVAL` = **30.0 s** |
| what gates it | `INSTRUMENTS.periodic_census` (cadence bookkeeping runs regardless, or the window would never close) | `core/instruments.hpp` |
| what counts a frame | `meter_.window_frames++` at the head of render | `cartridge.hpp · Cartridge::render()` |
| what ends a window | the print, then `FrameMeter::reset()` — zero rows, zero frames, restamp `window_start` | `cartridge.hpp · FrameMeter::reset()` |
| reset sites (all three) | boot end (`Cartridge::initialize()`, so the first fps excludes init wall-time) · the first-dump SKIP branch · the periodic close after the residue line | `cartridge.hpp · Cartridge::initialize()`, `Cartridge::phase_census_dumps()` ×2 |
| why the first window is skipped | `lastCensusDump_` is seeded negative, so the first dump fires on frame 1 and would report a ONE-frame window whose wall clock spans the whole boot | `machine/spawn_engine.hpp` (the seed); the skip prints its own line rather than being silent |
| does any visitor action mark a window | **NO** | boot and mood-transition censuses call `dump_entity_census()` directly and never touch `meter_` |

---

## Findings carried out of this census

1. **The `?dprcap=` premise is refuted.** No URL parameter parsing
   exists anywhere in the repository. The DPR cap is a compile-time
   constant. Any future design that assumed a URL channel must build
   one first.
2. **Mood is unreachable from a phone.** The largest gap in the control
   surface, and the reason U2 exists.
3. **The feature grant was witnessed only by silence** until LANTERN
   U1 — see §A3.
