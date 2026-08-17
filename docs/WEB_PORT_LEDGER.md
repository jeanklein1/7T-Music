> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# THE WEB PORT LEDGER

> Written **before** `web/` was deleted (PRUNING_1 P1 Step 2, ruling R1).
> Delete the code, keep the finding. Anchor: `1560038`; the port itself was
> last synchronized at `f1b16f5d2ee2860d34fce08beb7b80b8b4d8af81`.

## What was deleted

`web/` in full — 11 files, 824 KB:

| path | what it was |
|---|---|
| `web/shaders/world.wgsl` | a hand-copied fork of the desktop shader, 12,065 lines |
| `web/shaders/world.wgsl.gz` | the served-compressed copy |
| `web/shaders/world.wgsl.source` | the sidecar recording source commit + sha256 |
| `web/js/boot.js` | the host: device, layouts, pipelines, frame graph |
| `web/js/uniforms.js` | the hand-indexed 592-byte config packer |
| `web/js/state.js`, `web/js/passes/terrain.js` | host state + the terrain pass |
| `web/index.html`, `web/harness.html` | the page and the self-contained reference harness |
| `PORT_MAP.md` | the Phase-0 recon map |
| `web/dev/reflect.mjs` | a dev reflection helper |

Also removed: the **Mirror doctrine** and **Resync ritual** sections of
`CLAUDE.md`, and the mirror clause
in `.gitattributes` (the LF pin itself stays — it is correct for its own
reasons).

## The last-good state, for a resume

| | |
|---|---|
| source commit | `f1b16f5d2ee2860d34fce08beb7b80b8b4d8af81` |
| mirror sha256 | `1c8e5e225f00c0c1325b902a24087abd4f2c2b1416209295f5267ed59fb0e4a0` |
| mirrored | 2026-07-19 (post TERRAIN P2 + CHECKER-REBUILD) |
| recover with | `git show f1b16f5:web/` — git keeps every word |

**A resume starts from the CURRENT native shader, never from that fork.**
By the time of deletion the fork was three entry points ahead and five
behind, and it still carried the whole pre-U4 zone-mesh family the desktop
had retired. Reviving it means re-porting those divergences before doing
any new work — strictly more expensive than lifting the current shader.

## THE LESSON — this is the part worth keeping

The directory was 12,000 lines of derivable material wrapped around one
finding. The material regenerates; the finding cost a reversal to get right.

**A web host needs only 6 of the shader's 64 entry points.** At deletion it
dispatched `patch_terrain_vs`, `patch_terrain_fs`, `generate_patch_heights`,
`generate_patch_gradients`, `generate_patch_cells`, `frustum_cull_patches`.
Everything else in the 12,065-line mirror was carried, never used.

**The test that governs a resync is BINDING CLOSURE, not entry-point
existence.** This is the reversal. The obvious check — "do the entry points
the host names still exist upstream?" — passes and means nothing. What
matters is the set of `@group/@binding` slots reachable from each dispatched
entry point, because the host builds each pipeline against a bind-group
layout it writes **by hand**.

At the moment of deletion the desktop `patch_terrain_vs` / `patch_terrain_fs`
reached `g1:34 live_card_read` through `sample_live_card()`. The mirror never
declared binding 34, so the host's layout never provided it. A `cp` resync —
the ritual exactly as it was written — would have handed the host a module
its layout could not satisfy, and **pipeline creation would have failed at
boot**. Both entry points existed the whole time.

Corollary for whoever rebuilds: **the resync ritual as written was unsound.**
`cp` + `gzip` + sidecar sha is a file operation; keeping a hand-written host
in step with an evolving shader is a binding-closure obligation. Any future
port wants generated layouts, not transcribed ones — which is the same thing
the preamble and voice-bus campaigns are for.

## The third room, and why it decided this

`web/js/uniforms.js` packed the same 592-byte `DesignConfig` by raw word
index, with byte offsets typed into the source as comments. Nothing checked
it — no `static_assert`, no generator, no test. The C++ room is held by
`offsetof` witnesses and the WGSL room by the shared struct; the JS room was
held by nothing.

It was also, in its way, a witness: all 21 hand-annotated offsets agreed
exactly with the offsets computed from `state.hpp`. Authored independently,
landing on the same numbers. That is what made it credible **and** what made
it dangerous — deleting any config field would have re-flowed every offset
above it and the packer would have kept writing the old ones, silently.

Three unsynchronized rooms — hand-copied shader, hand-written layouts,
hand-indexed packer — is the shape the current campaigns exist to eliminate.
A host built later against generated tables is a cheaper object than the one
that was here.

## Open ruling (Jean's, recorded not decided)

Is web a **destination** the program is designed toward, or a
**maybe-someday**? If a destination, a web-compatibility constraint belongs
in the charter even with no web code in the tree. If maybe-someday, the
desktop optimizes for itself and a future port adapts. One charter line
either way; nothing else in this campaign depends on the answer.

---

## Addendum — the boot clock (PRUNING_1 P2/P3/P4, ruling F4)

Recorded here because it is the number the campaign's successors should be
judged against, and because it is not the number anyone was watching.

| | |
|---|---|
| boot, total | **227 s** |
| of which pipeline creation | **221 s** |
| slowest two | `patch_terrain` 10.9 s · `patch_terrain_indirect` 10.5 s |
| glaw1 (syntax-only, one TU) | ~18 s |

**glaw1 is not the iteration cost.** It checks one translation unit for
syntax and lookup; it is a gate, not a wait. What a person waits for is
221 s of pipeline creation, every boot. PRUNING_1 did not move it and could
not have: §1.1 found zero dead entry points, so there was never a pipeline
to remove. The campaign's honest payoff was tree mass and legibility, which
is what its §8 says.

The two leaders are the same shader stage compiled twice — `patch_terrain`
and `patch_terrain_indirect` differ by the `USE_PATCH_INDIRECTION` override.
Anyone hunting this number should start there, and should measure before
believing: a pipeline cache, a smaller shared module, or fewer permutations
are all plausible and none is proven.
