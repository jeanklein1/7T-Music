# The Board, Music (7T-Music)

A WebGPU generative artwork being taken native so it can be played against a
DAW. C++20 + WGSL on Dawn. Forked from 7T (`jeanklein1/7T-Pawns`) at
`de4b8b6f` and independent from there.

**One program, native.** The web twin was attic'd at tag `web-sunset`
(docs/LAWS.md: WEB_SUNSET). This repo has never deployed and now carries
no route to a deploy: the dist tooling went with the twin. The live site
everexpandingboard.com and the Cloudflare Pages project `7t` belong to
the sibling repo, and always did. See CLAUDE.md.

## Build (Windows)
cmake --preset the-board-full-release
cmake --build --preset the-board-full-release

`the-board-full` is the diagnostic twin (Debug); `-meter` arms the frame
meter. Dawn is built separately and pinned — docs/OPEN.md (N-a).

## Orientation
- docs/LAWS.md — project law. docs/OPEN.md — open items.
- docs/reference/ — Dawn/WebGPU reference material.
- audit/ — generated ledgers (do not hand-edit).
- CLAUDE.md — session constitution for agent collaborators.
