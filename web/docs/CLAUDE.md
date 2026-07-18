# 7T — Web Port Mission

This repo is the desktop 7T Musical Visualizer: C++20 host, WGSL shaders, WebGPU via
Google Dawn. The mission is a browser build in `web/` that becomes the public demo
("art gallery" experience) for The Ever Expanding Board website. The desktop app
stays untouched and its build stays green.

## Decisions already made — do not relitigate

- **WGSL is the portable asset.** Shaders cross over with minimal edits. The C++ host
  does not cross over; it gets a thin JavaScript replacement.
- **Host = plain JS on native WebGPU.** No Emscripten, no bundler, no framework, no
  npm runtime deps. ES modules served statically. Zero-build stays zero-build.
- **No MIDI.** Beat timing = fixed default BPM (start at **120**; Jean will tune).
  Derive `beatPhase` from `AudioContext.currentTime` so audio and beats share one
  clock. The uniform block carries `bpm` + `beatPhase`; shaders never know the
  source. If MIDI arrives later it just takes over writing those fields.
- **Audio source:** synthesized drone (already implemented in the harness). Fallback
  is a soundtrack file via `createMediaElementSource` into the *same* AnalyserNode.
  CORS note: audio must be same-origin or the analyser silently reads zeros.
- **Audio → shader bridge:** AnalyserNode → RMS + bass/mid/treble → one uniform
  buffer rewritten per frame. This uniform block is the stable contract between
  host and shaders. Extend it; don't fork it.
- **Palette:** two-tier CSS custom properties. Palette tier = literal hues (Sand
  #EFE6D4, Bone #E1D3B6, Ink #211E1C, Ink-soft #5A554C, Sky #6E9BC0, Coral #DC987B,
  Gold #E0A24E, Olive #9DA177, Lavender #A29EAF, Orb-red #C0532F as the single
  rationed punctuation color). Semantic tier = roles. JS reads tokens at runtime and
  feeds colors to shaders via uniforms. Never hardcode hex downstream of the tokens.
- **Storage buffer limits:** desktop Dawn exposes 10 per stage; the WebGPU spec
  floor is 8. Preferred: coalesce entity families to ≤8 bindings per stage for the
  public build (merge families into one buffer with offsets; strip families the
  demo doesn't need). Acceptable interim: `requiredLimits` raise on Chrome/Dawn
  with a `TODO(portability)` comment.

## Reference implementation

`web/harness.html` is self-contained and proves the full chain end to end:
drone → AnalyserNode → bands/RMS uniforms → compute pass mutating an agent storage
buffer → instanced render reading it back, themed through the palette tokens.
It has two marked seams — `SEAM 1 — COMPUTE` and `SEAM 2 — RENDER` — where real 7T
WGSL replaces the placeholder dynamics. Grow it into modules; keep it runnable.

## Phase 0 — Recon (do this first; report before editing anything)

Produce `web/PORT_MAP.md` containing:

1. **Shader inventory.** Every WGSL entry point; which pipeline (compute/render) it
   belongs to; the dispatch/draw order across one frame (the frame graph).
2. **Storage buffer census.** Per shader stage, count storage buffer bindings. Flag
   any stage over 8 and propose the specific coalescing/stripping for it.
3. **Uniform feed.** Every value the C++ host writes per frame (uniforms, constants,
   MIDI-derived params). Map each one to: keep as-is / replace with audio band /
   replace with BPM clock / drop for the demo.
4. **CPU-side visual logic.** Any per-frame host code that computes visual state
   rather than just plumbing data. If substantial, flag it — that is the only thing
   that could ever justify WASM later.
5. **Portability flags.** Dawn-specific or non-core WGSL (extensions, f16, etc.).
6. **Proposed module layout** for `web/` (shader files, host modules, entry page).

## Phase 1 — Lift shaders

Copy WGSL into `web/shaders/`. If the desktop build can load from that same path,
dedupe to a single source of truth; otherwise copy and note the divergence risk in
PORT_MAP.md. Replace the harness placeholder at the seams one pipeline at a time.
The page must stay runnable after every step.

## Phase 2 — Host loop

Recreate the frame graph in JS: buffer creation from the PORT_MAP layouts, bind
groups, dispatch order, resize with devicePixelRatio clamped to 2.

## Phase 3 — Beat clock + audio

Wire `bpm`/`beatPhase` into the uniform block. Keep the drone as default; stub the
soundtrack path behind the same analyser so swapping is a one-line change.

## Phase 4 — Fit and finish

- Live-read palette tokens; verify a full re-theme works by editing the palette
  tier only.
- The browser autoplay gate (user gesture before audio) is framed as the gallery
  "enter" moment, not a chore.

## Acceptance

- Runs in released Chrome with no flags and no console errors.
- Storage buffers ≤8 per stage, or the exception documented in PORT_MAP.md.
- Visual behavior matches desktop at default BPM within reason; when a parity call
  is uncertain, match desktop and note the question in PORT_MAP.md — don't invent.
- Page weight sane for a landing-page feature; no runtime dependencies.

## Working style

- Claude cannot see the rendered canvas. Jean runs the page in Chrome and reports
  console output and screenshots — ask for them at each phase checkpoint. Log
  `adapter.limits` and any device errors at boot to make those reports useful.
- Small commits per phase. Desktop build stays green throughout.
- This file is the contract. If something here conflicts with what's found in the
  repo, stop and surface it rather than silently choosing.
