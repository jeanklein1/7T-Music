# DEFERRED REGISTER

Durable list of intentionally-deferred work: items that are correct to postpone,
with the reason and the trigger for picking them up. "Queued" in a commit or
charter means "recorded here."

## Open

### D1 — Web mirror resync (`web/shaders/world.wgsl`)
- **State:** the mirror is a full-file byte-identical snapshot of
  `src/cartridges/the_board/realization/world.wgsl`, last synced **2026-07-19**
  from commit `f1b16f5` (per `web/shaders/world.wgsl.source`). It has since
  diverged by **~2596 lines** from desktop `master` — the web port runs on its
  own resync cadence.
- **Immediate consequence:** the mirror still holds `FLOATER_EVICTION_RADIUS = 400`
  while desktop `master` is `800` (FIX batch S4). The value now lives in three
  rooms and only the two desktop ones (the const + its doc-table line) agree.
- **Why deferred, not done now:** the resync ritual (CLAUDE.md) is a full-file
  `cp` + `gzip -9` regen + `.source` sha256 update + a **boot smoke-test of the
  web page** — an out-of-band, human-gated operation. Folding a 2596-line mirror
  refresh into a floater-radius fix would import unrelated desktop divergence and
  cannot be smoke-tested from here.
- **Trigger:** the next scheduled resync ritual. At that point `FLOATER_EVICTION_
  RADIUS 800` (and everything else since 2026-07-19) crosses over in one vetted step.

### D2 — Derive `FLOATER_EVICTION_RADIUS` from the allocation radius
- **State:** the eviction radius (GPU const, `world.wgsl`) must exceed the patch
  allocation radius (CPU quantity, `active_radius × PATCH_EXTENT`). The relation
  is real but **unassertable** — the two operands live in different rooms.
- **Why deferred:** deriving it CPU-side and uploading through `config` adds a
  `DesignConfig` field (~592 B C++/WGSL mirror) — structural work that does not
  belong in a diagnostic/fix batch.
- **Trigger:** a dedicated change that puts both values in one room, making the
  relation `const_assert`-able (COLLISION_CHARTER, the feasibility corollary + the
  containment rule).
