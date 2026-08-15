# LANTERN — THE CENSUS

*Read-only findings, transcribed to fixed rows. No design, no
recommendation, no proposed mechanism. Sites cite FILE · SYMBOL and
never line numbers — a line number is a position, and this page must
survive edits above it (P2). Where document and code disagree, code is
fact.*

**Why this page exists.** U2 — the parameter/control affordance —
is deferred by ruling and will be authored fresh from **§L2** and
**§L4**. A session boundary does not carry prose; it carries files.

## Provenance

| field | value |
|---|---|
| date | 2026-08-15 |
| HEAD censused | `fbc5c6011cd1594b4c4953e3100bb6d4094c20d4` (LANTERN U3 C1) |
| tool | none — this is a read of the tree, not a generated artifact |

| input file | content hash |
|---|---|
| `src/console/console.hpp` | `sha256:f1b99e181d120297309ab408acbf5ba8d6f3eda3823027b0969ccb35c24325ed` |
| `src/cartridges/the_board/cartridge.hpp` | `sha256:6a9e9ec0ccd80ab9b7e01a7d368d9573237b40bab446b890bf46207f22002e8a` |
| `src/cartridges/the_board/direction/input.hpp` | `sha256:acbaffab70883a77532a8e0bd8361598ee26c9cae2a4fcde9d8a775f80262d86` |
| `src/cartridges/the_board/machine/spawn_engine.hpp` | `sha256:3c83342249537d0a7ee735d0e6bc4873ca7f1f898a13fc0a10dd75b6257586dc` |
| `src/core/input_event.hpp` | `sha256:f73d45500345855d3a7fb7ba5ddad3157c5214dfa3b597d4b846a42496d5b0eb` |
| `web/index.html` | `sha256:f6df5dcc0861d31610f7886a2a8ba32a4d59452a9adc6f117d12ed43dc25b9b1` |

---

## §L1 — THE BOOT-REPORT DIVERGENCE

**Mechanism, all rows:** `console.hpp · Console::initWebGPU()` is two
disjoint bodies — `#ifdef __EMSCRIPTEN__` … `#else` … `#endif`. The
divergence is **compile-time, not a runtime branch**: on the web the
native lines do not exist. The web half continues in
`console.hpp · Console::request_device_web()`.

Rows marked **CLOSED** were open when this census was taken and have
since been answered; the row is kept so the page reads as a
before/after rather than a clean slate.

| fact | native line | web line | class | state |
|---|---|---|---|---|
| adapter identity | `[Console] Adapter <i>: <type>/<backend> \| <device> (<desc>) vendor=` | `[Device] adapter: vendor \| architecture \| device \| description` (`?` per redacted field) | web-available | both print |
| adapter **enumeration** (every adapter) | yes, unfiltered loop | — | native-only-by-nature | the web has no enumeration API; `RequestAdapter` returns one |
| adapter pick / scorer | `[Console] Adapter selected: index= backend=` | — | native-only-by-nature | nothing to pick from one |
| adapter/device limits | `[Console] Adapter limits:` (3 rows) | `[Device] granted vs floor:` (**6 rows** since U1) | web-available | both print |
| compiler plan | `[Console] Compiler plan (request):` | — | native-only-by-nature | `use_dxc`/FXC/DXC is a Dawn backend choice |
| Dawn toggles actually enabled | `[Console] Toggles used (N):` (`dawn::native::GetTogglesUsed`) | — | native-only-by-nature | Dawn-internal, no web counterpart |
| adapter feature SET (count + ids) | `[Console] Adapter features (N): <ids>` | `[Device] features: adapter offers N (<ids>)` | web-available | **CLOSED-BY-U1** — was unprinted on web |
| named feature posture | `[Console] feature multi-draw-indirect= timestamp-query=` | `[Device] features: … granted timestamp-query=` | web-available | **CLOSED-BY-U1** |
| device feature GRANT (vs adapter support) | — (native prints adapter support only) | `[Device] features: … granted timestamp-query=` | web-available | **CLOSED-BY-U1** — the web now prints a fact native does not |
| TimestampQuery **requested** | requested in the `#else` body | requested in `Console::request_device_web()` | requested-but-unprinted | **CLOSED-BY-U1** — see the note below |
| pixel cap + device dpr | — (`apply_pixel_cap` is web-only) | `[Device] pixel cap: <cap> (compile-time constant, not a setting); device dpr <n>` | web-available | **CLOSED-BY-U3 C1** |

**The note (why the last row mattered).** `cartridge.hpp ·
Cartridge::initialize()` sets `meter_gpu_ =
device_.HasFeature(TimestampQuery)` and prints **only the negative**
(`[METER] timestamp-query unavailable on this adapter — CPU rows
only`). The web console produced GPU timings, so the feature was
granted — and the grant's only evidence was **the absence of a failure
line**. A switch witnessed by silence is indistinguishable from one
that never fired (P6).

---

## §L2 — THE WEB CONTROL SURFACE

*The row shape the deferred unit needs. `exists` is strictly YES/NO:
it answers "does this channel exist in the tree", not "can a phone
reach it" — the channel name says whose hand it is.*

| channel | vocabulary (what it emits) | reaches | exists | site |
|---|---|---|---|---|
| URL query parameters | — | nothing | **NO** | no reader anywhere in the repository — `location.search`, `URLSearchParams`, `searchParams`, `document.location`, manual `?` splitting, searched untruncated across `.hpp/.cpp/.js/.html/.json`: **0 hits** (P11) |
| touch — stick (one finger, its own half) | `InputEvent::TouchMove` (x,z analog, dead-zoned, unit-clamped) | `TouchMoveState` → `update_movement_intent()` → walk | **YES** | `console.hpp · Console::emit_touch_intents()` → `input.hpp · on_touch_move()` |
| touch — drag look | `InputEvent::TouchLook` (dx,dy × `LOOK_SENS_TOUCH`) | the same two camera deltas the mouse drag lands on | **YES** | same → `input.hpp · on_touch_look()` |
| touch — pinch | `InputEvent::TouchZoom` (× `PINCH_SENS`) | `inputState_.zoom_delta` — the scroll wheel's channel | **YES** | same → `input.hpp · on_touch_zoom()` |
| touch — clean tap, LEFT half | `InputEvent::TouchTapLeft` | `toggle_aura()` — key 3's door | **YES** | same → `input.hpp · on_touch_tap_left()` |
| touch — clean tap, RIGHT half | `InputEvent::TouchTapRight` | `try_possess_nearest()` — CAPS_LOCK's door | **YES** | same → `input.hpp · on_touch_tap_right()` |
| touch stream ownership | — (plumbing) | claims the canvas handlers from the GLFW port | **YES** | `console.hpp · Console::claim_touch_stream()` on `TOUCH_TARGET`; backstop `Console::any_touch_active()` |
| keyboard — mood keys 5/6/7/8 | GLFW key events | `request_mood_transition(… MOOD_OPEN_SUNSET / INDOOR_FLAT / INDOOR_VAULT / FINITE_OUTDOOR)` | **YES** (keyboard only) | `input.hpp · handle_key_down()` — these four are the **only** call sites of `request_mood_transition` in the tree |
| keyboard — other verbs (2, 3, 0, `[`, `]`, V, KP±, KP8, KP., CTRL, CAPS_LOCK) | GLFW key events | aura height/aura, orb palette/rule/gesture, render radius, veil dither, look sensitivity, FPV, possession | **YES** (keyboard only; aura and possession also reachable by tap) | `input.hpp · handle_key_down()` |
| portal entry | — (no input vocabulary; GPU-side proximity on the possessed slot) | `point_.portal_trigger` → `pendingDestination_` (the arch's authored `destination`) → world rebuild + `apply_mood(pendingDestination_.mood, …)` | **YES** (walk-reachable) | `cartridge.hpp · Cartridge::phase_witness_harvest()` writes it; `Cartridge::phase_portal_trigger()` consumes it |
| meter / window control | — | nothing | **NO** | no input path of any kind reaches `meter_`; see §L3 |
| DOM — DETAILS panel | — | displays `Module.print` lines; reads nothing back into the cartridge | **YES** (read-only) | `web/index.html · #logToggle` / `#logToggle2` |
| DOM — reload button | — | page reload | **YES** | `web/index.html · #reload` |
| DOM — viewport resize / orientation | — | canvas + framebuffer size | **YES** (automatic) | `web/index.html · sizeToViewport()` |
| DOM — gesture / dblclick suppression | — | prevents browser zoom | **YES** (automatic) | `web/index.html`, `gesturestart` + `dblclick` listeners |
| DOM — wake lock | — | keeps the screen awake | **YES** (automatic) | `web/index.html`, `navigator.wakeLock` |
| pixel cap | — | framebuffer backing-store size | **YES** (compile-time constant; no runtime channel of any kind) | `console.hpp · MAX_DEVICE_PIXEL_RATIO` → `Console::apply_pixel_cap()`; printed since U3 C1 |

### What a phone can and cannot do

A phone visitor can walk, look, zoom, toggle the aura, possess a pawn,
and enter a portal by walking into it. **Mood SELECTION** — naming
which of the four — has no touch intent, no URL parameter and no DOM
hook; its only entry is the four keyboard cases above. **Mood CHANGE**
does happen on a phone: walking through a portal rebuilds the world and
applies `pendingDestination_.mood`, but that mood is the arch's
authored destination, chosen by the world's seed rather than by the
visitor.

### The measurement problem this hands forward

The four moods are **diegetically reachable** — a walker will arrive at
them — but there is **no way to drive the twin deterministically to a
named arm**. A soak walk that wants, say, the four-light indoor vault
cannot ask for it: it must boot, walk, and accept whatever the seed's
portals offer. So a measurement taken on one arm cannot be repeated on
demand, and two captures cannot be known to be of the same arm.

*Stated as a problem only. This census proposes no solution and names
no mechanism; that is the deferred unit's authorship, not this page's.*

---

## §L3 — THE METER'S WINDOW BOUNDARY

**The cadence.** One wall-clock tick and nothing else:
`time_state_.seconds − lastCensusDump_ ≥ CENSUS_DUMP_INTERVAL`, where
the constant is **30.0 s** (`machine/spawn_engine.hpp ·
CENSUS_DUMP_INTERVAL`), evaluated in `cartridge.hpp ·
Cartridge::phase_census_dumps()` under the `INSTRUMENTS.periodic_census`
gate. Frames are counted by `meter_.window_frames++` at the head of
`cartridge.hpp · Cartridge::render()`. The cadence bookkeeping advances
whether or not the text prints — otherwise the window it drives would
never close.

| reset site | trigger | what it restamps |
|---|---|---|
| `cartridge.hpp · Cartridge::initialize()` (boot end) | boot completes | zeroes all rows + `window_frames`, restamps `window_start` — so the first window's fps excludes init wall-time |
| `cartridge.hpp · Cartridge::phase_census_dumps()`, first-dump branch | the first cadence tick — `lastCensusDump_` is seeded negative, so it fires on frame 1 | same; the skipped window would otherwise report ONE frame whose wall clock spans the entire boot, pipeline compilation included. The skip prints its own line rather than being silent (P6) |
| `cartridge.hpp · Cartridge::phase_census_dumps()`, periodic close | each 30 s tick thereafter, after the table and residue lines print | same — zero rows, zero frames, restamp `window_start` |

All three call `cartridge.hpp · FrameMeter::reset()`. `snap_pairs` is
deliberately **not** zeroed: a readback may be in flight across a
window boundary.

**Nothing a visitor can do marks a window.** The boot and
mood-transition censuses call `dump_entity_census()` directly and never
touch `meter_`.

---

## §L4 — THE OPEN DOCKET

*Handed forward by this campaign. One line each; the blocking question
is the reason it is not already done.*

1. **Feature ids → names.** The web and native lines both print numeric
   ids. *Blocking question:* which `wgpu::FeatureName` enumerator
   spellings does the emdawnwebgpu build actually define? Unanswerable
   from a census container without the Dawn headers or a build. Needed
   the day an optional wing is proposed, not before.
2. **"granted vs floor" → "granted vs used".** The line prints six rows
   and the discard net enforces three, so the label promises more than
   the mechanism delivers. *Blocking question:* what is the authority
   mapping a schema budget row to a WebGPU limit name, and does it live
   in `binding_schema.py` or in the console? Sourcing the row set from
   the schema would make the boot log read the wallet on the actual
   device and would retire the print-vs-enforce hazard.
3. **The pixel cap as a design surface rather than a literal.**
   `MAX_DEVICE_PIXEL_RATIO` is now printed but is still a rebuild, not a
   setting. *Blocking question:* what is the cap FOR — a fixed artistic
   ceiling, or a device-adaptive budget? Until that is answered,
   "steerable" has no target to steer toward.
4. **The parameter surface.** *Blocking question:* what must be drivable
   deterministically for a soak walk, and by what identity are worlds
   and arms named? Measurement is the first consumer; the roadmap's
   world-naming mechanic is a second one, and a surface built for only
   one of them will be rebuilt for the other.
