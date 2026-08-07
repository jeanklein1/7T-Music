# STREAM_0 — DECISION BRIEF (PARKED per Jean's 2026-08-07 directive:
# no optimization before the piece runs on phones. Re-open after the
# phone verdict.)

One question: does `fullRegen` die?

## The finding it rests on (PORT_6 U4, corroborated 2026-08)

Boot generates the 7×7 priority window (49 patches) synchronously; the other
176 already stream at 4/frame. `reset_surface` is called from exactly two
places — boot and TEARDOWN — and both hit the same `fullRegen` block. So
every mood change pays the same synchronous freeze that boot does; the fade
does not cover streaming, it covers a stall.

Honest sizing (corrected 2026-08-07): the block's true cost is the
seconds class — 1,223 ms measured natively on the 920M with Chrome open.
The 52 s web readings and the 49–62 s native cluster previously attributed
here were, respectively, the pipeline compile storm surfacing in the first
waiting phase, and this machine's run-to-run instability. The case for
STREAM_0 no longer rests on a 52-second headline; it rests on the seconds
that remain — a freeze is a freeze, it scales with device weakness, and the
phones we ship to sit below the 920M.

## Why it should die (three reasons, one mechanism)

1. **Law.** It is a no-teleportation violation nobody had seen: the world
   does not move through the transition, it halts and reappears.
2. **First contact.** A QR destination cannot open onto a frozen page. The
   sub-second first-pixel target is unreachable while the block exists.
3. **Mechanical sympathy.** One giant uninterrupted submission burst is
   exactly the shape that trips driver watchdogs on weak GPUs — the phones
   we are shipping to. Many small submissions per frame is the shape that
   survives. STREAM_0 does not fix this laptop, but it removes the program's
   contribution to the failure mode everywhere.

## The shape

`fullRegen` as a special case is deleted. Boot and TEARDOWN both become:
reset, then the *existing* 4/frame streamer at an elevated boot budget B,
with `FADE_IN` gating on residency of the priority window. One door already
exists — the tree says so itself ("the same door the transition machine
uses"); this makes the door the only mechanism.

- The budget constant B belongs to the master-control-panel surface.
- No WGSL changes, no binding changes — outside the FXC law entirely.
- Blast radius: cartridge boot/TEARDOWN sequencing and the fade-complete
  condition. Nothing else.

## Invariants (what must remain true)

- No frame submits more than B patch generations.
- FADE_IN completes only when the gate criterion is resident; the world is
  never shown mid-fill unless Jean later chooses to lift the fade.
- The door guarantee and no-teleportation hold on the portal path — touch
  users have no keyboard.
- Boot and transitions share one mechanism; deleting `fullRegen` removes a
  structure, not a fact.

## Knobs for Jean's ruling

1. **B at boot** vs **B during transitions** — same number, or boot elevated?
   (Default proposal: same B; simplicity first, split only if a measurement
   asks.)
2. **Fade gate criterion** — full 7×7 resident, or center 3×3 resident with
   the rest streaming in behind the fade-up? (Default proposal: full 7×7;
   identical look to today, only honest and shorter.)

## What changes in feel

Mood changes: the black is the same black, but it now covers a fill that
progresses, and its duration becomes proportional and interruptible instead
of a fixed stall. Boot: first pixel arrives when the fade lifts, on healthy
hardware inside the sub-second class once B is tuned.

## Sequencing

STREAM_0 is independent of SHIP_0 and should not block it — the phone test
is meaningful either way. But land STREAM_0 before SHIP_1 (the public URL on
the business card), because first contact is the product.

Companion: with the patch phase acquitted, the measured boot dominator on
both twins is pipeline compilation. That is PIPE_0's territory — a separate
one-page decision. STREAM_0 fixes the transition law; PIPE_0 fixes the
first-visit clock. Together they are the sub-second first pixel.
