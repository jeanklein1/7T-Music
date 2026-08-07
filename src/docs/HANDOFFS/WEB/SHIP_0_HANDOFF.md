# SHIP_0 — HANDOFF v2 (CC)

Supersedes v1, which never reached the handoff folder. **Jean: place this
file at `7t/docs/handoffs/WEB/` — that is where CC looks.**

Contract: reformed register. Outcome invariants and blast radii; CC binds
mechanisms by reading the tree and reports the binding. Tiers: EXECUTE /
RESOLVE (report options, do not improvise) / STOP.

Campaign goal, in Jean's words: minimum criteria for the piece to actually
run — on phones above all — with a loading state, a fallback, and a public
HTTPS URL. Aesthetics and all optimization follow the phone verdict.
STREAM_0 and PIPE_0 are parked by that directive.

FXC note: no WGSL edits, no binding changes anywhere in SHIP_0.

---

## STANDING VERDICT (read once, then stop spending on it)

The device loss on the development laptop is closed as a debugging target:

- Edge 2026-08 reproduced the cause inside the browser's own bundled Dawn:
  `CheckAndUpdateCompletedSerials (third_party\dawn\...\QueueD3D12.cpp:185)`,
  `Device removed reason: S_OK` — a D3D12 fence anomaly on the Intel HD 5500
  (driver 20.19.15.4703, 2016).
- Chrome died at the identical point after a 5.6 s boot — the death is
  independent of boot duration and of the compile storm.
- Firefox returns a null adapter on this machine — a second vendor's
  blocklist judging the GPU unfit. Its capture proves the wasm's no-adapter
  path prints a clean `RequestAdapter failed:` line (a fallback hook).
- `dawn.json` marks `device lost reason` string-converted on Emscripten with
  values 3/4 excluded — `reason=1` can only arrive from the browser. Nothing
  in our tree drops anything.
- Machine timing is weather: native pipeline compile 70,459 ms and
  205,527 ms on identical code; web boot-to-death 5.6 s to 73.6 s.

Do not propose fixes for this loss. The phone is the next witness.

---

## U1 — LAWS.md correction  [EXECUTE]

Recon: read the law entry sourced to commit `017367e`.

- REMOVE the "62,517 ms vs 5,609 ms" performance claim and anything resting
  on it.
- KEEP the compatibility rationale for modest limits (L14, C6).
- ADD: "Timing on the development machine varies by an order of magnitude
  between runs; no single-run comparison from it is evidence. Brackets on
  identical code: native pipeline creation 70,459 → 205,527 ms; patch
  system 1,223 → 62,000 ms era-dependent; web total boot 5.6 → 73.6 s."
- ADD as a law candidate (Jean stamps the ordinal): "A timer names where
  the wait surfaced, not where the cost lives. Web per-pipeline times are
  wire-enqueue latency; the backend compile executes in-order in the
  browser's GPU process and lands on the first phase that waits. Witness:
  web 'Patch system' 51.8–73.6 s ≈ native 'Total pipelines' 70–205 s — one
  cost, two attributions. Corollary: Chromium disk-caches compiled
  pipelines per origin, so a fast revisit (5.5 s observed) is expected."

Invariant: no law rests on a single-run timing from this machine, and no
future ruling reads a web per-pipeline time as a compile cost.

Blast radius: `LAWS.md` only.

---

## U2 — Witness identity, web half  [EXECUTE]

Native already enumerates and logs (index=2, 920M). Web twin, two edits at
the adapter-acquisition site:

1. Request `powerPreference = high-performance` in
   `WGPURequestAdapterOptions`. Harmless on single-GPU phones; correct for a
   real-time artwork; on dual-GPU Windows, modern Chromium honors it.
2. Log what came back via `wgpuAdapterGetInfo`:

       [Device] adapter: <vendor> | <architecture> | <device> | <description>

Invariant: every web capture names its silicon. The presumption that the
browser runs the HD 5500 becomes a logged fact or is overturned.

RESOLVE: if info fields come back empty (some builds redact), report and
stop; no fallback plumbing.

Blast radius: web-branch adapter request + one log line.

---

## U3 — First-contact shell  [EXECUTE] — the new heart of SHIP_0

Three states in `index.html`. Prefer zero wasm edits: the page already owns
`print`/`printErr`, and every trigger below is a line already observed in
captures. If line-coupling proves too fragile at any point, one `EM_ASM`
hook at that point is the permitted alternative — CC's binding, reported.

**State 1 — LOADING.** Visible from page-open until the world is being
presented. A poster slot (image or short muted looping video — placeholder
now; the asset is Jean's aesthetic gate) plus one status line in plain
human words, mapped from known boot phases (`[Cartridge] GPUState`,
`[Renderer]`, `[Cartridge] Patch system`, `[Incubator] the_board renderer
ready`). Raw log passthrough may exist behind a tap, never by default.
Dismissal signal: CC binds the cleanest "world visible" moment (the
`Controls:` line marks the frame loop live; FADE_IN covers the rest) and
reports the binding.

**State 2 — FALLBACK (cannot run).** Triggers: `!navigator.gpu` checked
BEFORE fetching the wasm/data (spares the download on unsupported phones);
the `RequestAdapter failed:` line; device-request failure. Card: poster,
one line in the work's own voice, contact link, and a slot for a short
video of the piece. This card is a designed destination, not an apology —
Firefox on the dev laptop lands here today, correctly.

**State 3 — LOST (was running, then died).** Trigger: the `[Device] LOST`
line. Same card, softer sentence (the device's graphics driver could not
sustain the piece), plus a reload affordance. NO auto-retry — L10: the
world has one way to come into being; a reload button is the honest retry.

**Mobile correctness (not optimization):** viewport meta
(`width=device-width, initial-scale=1`), canvas fills the visual viewport
in both orientations, touch/gesture defaults suppressed on the canvas
(no scroll/zoom hijacking the piece). DPR policy: bind to whatever the
console does today; note the cap as a future knob, do not tune it now.

Invariant: no visitor ever sees a dead white page or a raw console. Every
device — including the dev laptop — reaches one of the three designed
states.

Blast radius: `index.html` (and one optional EM_ASM per fragile trigger).

---

## U4 — Deploy  [EXECUTE prep; Jean executes account-touching steps]

WebGPU requires a secure context; HTTPS is mandatory, LAN IPs are not it.

1. Inventory the web output (`index.html`, `the_board.js`, `.wasm`, `.data`)
   with sizes. Report the `.data` size explicitly — it is the mobile
   download cost. Record it; do not optimize it (Jean's directive).
2. Host by the numbers: every file ≤ 25 MiB → Cloudflare Pages; else →
   GitHub Pages (~100 MiB/file). RESOLVE if neither fits: report packaging
   options, do not repackage on your own authority.
3. Produce the dist folder + exact deploy commands for Jean (wrangler or
   dashboard drag-drop; GH Pages branch convention in one paragraph).
4. Headers: none special — single-threaded, no COOP/COEP.
5. Phone protocol (verbatim in your report):
   - Android Chrome; console via `chrome://inspect` over USB.
   - iPhone: iOS 26 / Safari 26 has WebGPU by default.
   - First visit pays the compile storm + patch phase — seconds-class on
     phone compilers; a much faster revisit is the pipeline cache, not a
     bug. The loading state (U3) makes the wait honest.
   - If it dies, the LOST card appears — capture the console anyway,
     timestamps on.
6. Capture protocol, standing: DevTools timestamps ON (delta from last
   submission to loss discriminates ~2 s TDR vs ~10–15 s watchdog kill).

Invariant: a URL exists that any phone can open, and it is the QR
destination — the deliverable itself.

---

## U5 — Housekeeping  [EXECUTE with recon]

- `claude/web-handoffs-review-u1aalo`: its witness lines (`[Ground] zones
  active anywhere`, `[Card] live-card field`) print in the 2026-08 web
  captures, so it appears merged. Verify merged-to-master, then delete the
  branch. If NOT merged, STOP and report — the running build would then be
  ahead of master, which is its own finding.
- Verify-then-delete: `claude/cut-1-limits-fit`,
  `claude/port-0-seam-census-5z0at8`. `rescue-175fd17`: merge its
  housekeeping commit if not yet, then delete.
- Jean pushes the tag: `web-first-boot-2026-08` (proxy blocks CC tags).
- OPT_1d remains stamp-gated on Jean's explicit word; silence keeps it
  shut. Do not open it.

---

## SEQUENCE AFTER SHIP_0

Phone renders → **SHIP_1: touch sticks** (spec exists: left thumb writes
`move_x/move_z`; right thumb camera deltas sampled per-frame tick, not per
callback; two-finger pinch zoom; verify camera-relative vs world-relative
interpretation in the kernel before binding) → Jean's aesthetic pass
(posters, cards, tweaks) → publish + QR. Phone fails at the same point →
the bug is ours; first suspect: a loop bounded by data that differs on the
web; the timestamped capture opens that session.
