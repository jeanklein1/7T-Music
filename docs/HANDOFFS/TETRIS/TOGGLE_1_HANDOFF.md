# TOGGLE_1 — THE CHAIN, IF IT IS BROKEN (conditional)

The fresh Vulkan console reads `[Console] Toggles used (9)` with
`disable_symbol_renaming` **absent**. If that boot ran a binary built at
`30c9a7c` or later, the control test has answered: **branch (b) — the
PIVOT_0d-ii chain never propagates**, every Dawn toggle rides it into
the void, and Boot 2's verdict softens from "Dawn refused DXC" to
"use_dxc may never have reached Dawn at all." (Nothing in the queue
depends on re-litigating DXC; the finding matters because future
toggles do.)

## GATE CONDITION — Jean, one line

Confirm: was the (9)-boot's binary built from a tree containing
`30c9a7c` (`console.hpp` with the control)? If **uncertain or no**:
rebuild at HEAD, boot once, read the line. `(10)` with the control
present → branch (a) after all — Dawn refused DXC, the chain works;
run TOGGLE_0 U2 (retire the control), record the verdict, **discard
this file**. `(9)` again on a confirmed-fresh binary → proceed below.

## U0 — TRACE THE BREAK (read-only)

From the chain site CC verified verbatim (static toggle array →
`DawnTogglesDescriptor` → `InstanceDescriptor.nextInChain`), trace
forward and answer, with symbols:

1. Is the **same** `InstanceDescriptor` object passed to the actual
   `CreateInstance` call the program uses, or does a second creation
   path win?
2. **Lifetime:** do the descriptor, the toggles descriptor, and the
   toggle-name array all outlive the `CreateInstance` call (no stack
   scope ends between construction and use)?
3. Is there exactly one instance creation in the program, or several
   (hot-reload path included — the console header says
   `Hot Reload Enabled`)?
4. Where does the `[Console] Toggles used` line read from — instance,
   adapter, or device? (If it reports device toggles only, an
   instance-stage success could be invisible to it; say so if found.)

Report the break verbatim. If no break is identifiable from reading the
tree, **STOP** and request the paste: the toggle-inheritance section of
Dawn's native headers (`dawn/include/dawn/native/DawnNative.h` and the
toggles documentation) into `DAWN_REFERENCE.md` — the file exists,
empty, waiting for exactly this job.

## U1 — THE MINIMAL FIX (one commit, master)

Fix the identified break and nothing else — a lifetime hoist, a
descriptor actually passed, or the chain moved to the creation path
that wins. Nothing bespoke; the control stays armed. Jean boots once:
expected `(10)` with `disable_symbol_renaming` present.

## CLOSE

On `(10)`: TOGGLE_0 U2 retires the control (its own commit, as already
specified); debt 12 is recorded closed — branch (b), found and fixed —
wherever the debt ledger lives. The book on native D3D12 may then be
reopened or left shut at Jean's pleasure; nothing in the queue waits on
it.

## STOP CONDITIONS

- Acting without the gate condition confirmed.
- Any edit before U0's break is identified and reported.
- Any edit outside `console.hpp` unless U0 proves the break lives
  elsewhere — in which case report first, edit after.
- Fixing "while in there" anything that is not the identified break.
