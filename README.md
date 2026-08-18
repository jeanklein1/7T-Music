# The Ever Expanding Board (7T)

A web-first WebGPU generative artwork. Live at https://everexpandingboard.com.
C++20 + WGSL on Dawn, compiled with Emscripten (emdawnwebgpu, vendored),
deployed as static files via Cloudflare Pages.

## Build (Windows)
Requires the persistent EMSDK environment variable (set once; docs/LAWS.md L40).
cmake --preset the-board-web
cmake --build --preset the-board-web
python tools\web_dist.py
npx wrangler pages deploy dist --project-name=7t

## Orientation
- docs/LAWS.md — project law. docs/OPEN.md — open items.
- docs/reference/ — Dawn/WebGPU reference material.
- audit/ — generated ledgers (do not hand-edit).
- CLAUDE.md — session constitution for agent collaborators.
