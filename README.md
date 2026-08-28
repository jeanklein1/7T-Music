# The Board, Music (7T-Music)

A WebGPU generative artwork being taken native so it can be played against a
DAW. C++20 + WGSL on Dawn. Forked from 7T (`jeanklein1/7T-Pawns`) at
`de4b8b6f` and independent from there.

**This repo never deploys.** The live site everexpandingboard.com and the
Cloudflare Pages project `7t` belong to the sibling repo. See CLAUDE.md.

## Build (Windows)
Web — the control witness. Requires the persistent EMSDK environment
variable (set once; docs/LAWS.md L40).
cmake --preset the-board-web
cmake --build --preset the-board-web
python tools\web_dist.py
npx wrangler pages dev dist        # local preview only; publishes nothing

Native — this fork's target.
cmake --preset the-board-full-release
cmake --build --preset the-board-full-release

## Orientation
- docs/LAWS.md — project law. docs/OPEN.md — open items.
- docs/reference/ — Dawn/WebGPU reference material.
- audit/ — generated ledgers (do not hand-edit).
- CLAUDE.md — session constitution for agent collaborators.
