# 7T — The Ever Expanding Board · session constitution

7T is a web-first WebGPU generative artwork (C++20 + WGSL, Dawn via
Emscripten/emdawnwebgpu, vendored). Audience: everexpandingboard.com.
The program is essentially artwork; engineering serves that.

## Boot preflight (every session, before any claim about history)
- `git rev-parse --is-shallow-repository` → if true, `git fetch --unshallow origin`.
  Shallow clones silently report graft-boundary dates and fake divergence.
- Work on master unless the handoff says otherwise. Handoff outranks harness defaults.

## The triangle
Jean holds all gates: build (glaw1), visual sign-off, merge, deploy, naming.
Claude holds architecture, rulings, and handoff authoring.
CC executes handoffs on the tree: recon before edit, STOP-on-mismatch scoped to
the unit, flag-and-continue, one commit per logical unit, report findings without
improvising on authority-bearing decisions. Cite symbols, not line numbers.

## Build & deploy (Jean runs these; listed for orientation)
cmake --preset the-board-web → cmake --build --preset the-board-web
→ python tools\web_dist.py → npx wrangler pages deploy dist --project-name=7t
(the persistent EMSDK user variable carries the presets — L40)
dist/ is the deploy target. web/ holds the shell sources and receives the
build artifacts; only dist/ ships.

## Where truth lives
- docs/LAWS.md — the rule book. Read before proposing.
- docs/OPEN.md — the only home of open/parked state.
- docs/HANDOFFS/ — open work orders only; if it's in here, it isn't
  done. (The directory exists only while work is open — absence is health.)
- audit/ — the machine's room: generated ledgers only (their tools live in
  tools/). MANIFEST.md and BINDING_LEDGER.md are law and stay searchable.
- The tree holds living matter only; git history is the attic.
  Resurrect: `git checkout <sha>^ -- <path>`.
