# DISCARD_0 — TWO STORES WITH NO READER (F1, F2)

The smallest subject campaign the pass ledger licenses. Two depth
attachments are `storeOp: Store` on textures that are **provably
unreadable** — usage `RenderAttachment` alone, no `TextureBinding`, no
`CopySrc` — so every stored byte is written to main memory and then
unreachable forever. `StoreOp::Discard` is the exact op for this case
and appears nowhere in the tree (PASS_LEDGER Q2). Under the tiler model
this is `4·W·H` per frame on every device, every arm: ~4.0 MiB/frame on
the Pixel's 672×1495 canvas, 7.9 MiB/frame at 1920×1080.

Two units, two commits, no WGSL, no ledger movement.

## U0 — RECON GATE (the safety proof, re-verified at HEAD)

1. Git hygiene; record HEAD.
2. Verify verbatim, STOP on mismatch:
   - `console.hpp` `createDepthBuffer()`: `depthDesc.usage =
     wgpu::TextureUsage::RenderAttachment;` — nothing else in the mask.
   - `state.hpp` `offscreenDepthTexture_` creation: usage
     `RenderAttachment` alone.
   If either mask has grown a `TextureBinding` or `CopySrc` since
   PASS_0, the safety proof is void — **STOP, report, edit nothing.**
3. Count the tree's `StoreOp::` assignments. PASS_0 counted 12
   `LoadOp::`/`StoreOp::` assignments repo-wide; verify the two subject
   sites read `depthStoreOp = wgpu::StoreOp::Store` today:
   - `render_passes.hpp`, `render_main_pass` (P3 depth).
   - `bodies/gallery.hpp`, `render_snapshot_pass` (P4 depth).

## U1 — MAIN PASS DEPTH (one commit)

In `render_main_pass`, the depth attachment only:
`StoreOp::Store` → `StoreOp::Discard`. Color is the presented
backbuffer and is untouched. Commit:
`DISCARD_0: main depth storeOp Discard (PASS_0 F1 — no reader exists)`.

## U2 — SNAPSHOT DEPTH (one commit)

In `render_snapshot_pass`, the offscreen depth attachment only:
`StoreOp::Store` → `StoreOp::Discard`. The offscreen **color** stays
`Store` — it is `CopySrc` and the promotion copy reads it. Commit:
`DISCARD_0: snapshot depth storeOp Discard (PASS_0 F2)`.

## WITNESSES

No shader change → no naga run required. glaw1 + boot + visual (the
gate is *nothing changes* — a visible difference is a defect, since
nothing can read what was stored). Then one preview deploy + one Pixel
window per the spend governor; the meter comparison is row-vs-itself
and may sit under quantization — the win is the model's, banked either
way.

## STOP CONDITIONS

- Either usage mask differs from U0.2 verbatim.
- Either subject site's current op differs from `Store`.
- Any temptation to touch a third `StoreOp` site — there are exactly
  two subjects in this campaign.
