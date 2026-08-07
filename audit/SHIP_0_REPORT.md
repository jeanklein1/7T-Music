# SHIP_0 — CAMPAIGN REPORT

Executes `src/docs/HANDOFFS/WEB/SHIP_0_HANDOFF.md` (v2). Reformed register:
outcome invariants named, mechanisms bound by reading the tree, bindings
reported. Every claim carries file + symbol; absences were verified over whole
files (P11).

**STREAM_0 and PIPE_0 were read and NOT executed.** Both open with their own
parking notice — *"PARKED per Jean's 2026-08-07 directive: no optimization
before the piece runs on phones"* — and both are decision briefs, not
execution handoffs. Nothing in them was started. Their contents are
summarised at the foot of this report so the phone verdict can re-open them
without a re-read.

Base: `3fb8518` (Jean's "New handoffs", pushed onto this branch). Confirmed by
`git fetch --all` in the same command sequence (P9).

---

## UNIT TABLE

| unit | tier | commit | state |
|---|---|---|---|
| U1 LAWS.md correction | EXECUTE | `4bdfd3c` | **DONE** — both rooms |
| U2 web adapter identity | EXECUTE | `8fe2fd7` | **DONE** |
| U3 first-contact shell | EXECUTE | `39650e7` | **DONE** — five states verified headless |
| U4 deploy prep | EXECUTE prep | `0585808` | **DONE** — tool + verdict; Jean runs the account steps |
| U5 housekeeping | EXECUTE w/ recon | this report | **STOP on the first item** (below) |

No WGSL edits, no binding changes — SHIP_0's standing constraint, held.

---

## U5 — THE STOP, AND WHY THE PREMISE INVERTED

U5 reasons: *"its witness lines (`[Ground] zones active anywhere`, `[Card]
live-card field`) print in the 2026-08 web captures, so it appears merged.
Verify merged-to-master, then delete the branch. If NOT merged, STOP and
report — the running build would then be ahead of master, which is its own
finding."*

**It is not merged. It is 8 commits ahead of master.**

```
origin/master                          4c1a804
origin/claude/web-handoffs-review-...  3fb8518   (8 ahead, 0 behind)
git merge-base --is-ancestor 918ed2e origin/master  ->  false
```

And the premise that produced the inference is wrong in an instructive way.
**Those witness lines are in the NATIVE capture, not the web one.** The file
Jean supplied holds two captures back to back:

| | capture 1 — web twin (lines 1–279) | capture 2 — native (lines 280–560) |
|---|---|---|
| banner | `INCUBATOR DUAL (web twin — no hot reload)` | `INCUBATOR DUAL (Hot Reload Enabled)` |
| `[Ground] zone rects in core` | ✓ line 261 | ✓ line 550 |
| `[Ground] zones active anywhere` | **absent** | ✓ line 551 |
| `[Card] live-card field` | **absent** | ✓ line 552 |

So the two captures are from **two different builds**: the native one carries
this branch, the web one predates it. The correct reading of the evidence is
not "the branch appears merged" but:

- the **native** build is ahead of master (it is this branch), and
- the **web** build is behind both.

Which matters directly for SHIP_0: **the web capture the standing verdict
rests on was produced by a tree without OPT_1a's rest skip, without OPT_1e's
boot witness, and without U2's adapter line.** Any web number in it predates
this branch. The next web capture will not be comparable to it, and that is a
feature — it will be the first one that names its own GPU.

**Nothing was deleted and nothing was merged.** Merging to master is outside
this session's git law (all work pinned to the designated branch), and U5's
own instruction is to stop here.

### The other three, verified

| ref | state | disposition |
|---|---|---|
| `claude/cut-1-limits-fit` @ `af839dd` | **MERGED** — ancestor of `origin/master` | safe to delete; **my delete was blocked** (below) |
| `claude/port-0-seam-census-5z0at8` | **already absent** from the remote | nothing to do — the PORT_6 reply's "still standing" is stale |
| `rescue-175fd17` / commit `175fd17` | **unreachable**: no such branch, and `git fetch origin 175fd17` → `couldn't find remote ref` | see below |

**The orphan is gone.** `175fd17` was never rescued — the PORT_6 reply left
`git branch rescue-175fd17 175fd17` as Jean's to run, and it was not run. An
unreferenced commit is reachable only from the reflog of the clone that made
it. If that commit still matters, it exists **only in Jean's local clone**, and
only until its reflog expires (default 90 days for unreachable objects, 30 for
some). Nothing can be done for it from here. If it does not matter, U5's line
about it is closed by this paragraph.

**Branch deletion is proxy-blocked.** Four attempts, backoff, same result:

```
send-pack: unexpected disconnect while reading sideband packet
fatal: the remote end hung up unexpectedly
Everything up-to-date
```

The delete refspec is dropped silently — note the `Everything up-to-date`.
This is the same class as the block the handoff already anticipates for tags
(*"Jean pushes the tag … proxy blocks CC tags"*), so branch deletion joins
tag-pushing on Jean's list:

```
git push origin --delete claude/cut-1-limits-fit
```

**OPT_1d remains shut.** No word was given; silence keeps it closed. Not
opened, not designed, nothing left in the tree.

---

## U1 — THE WITHDRAWN CLAIM

L14 rested "modest limits are a PERFORMANCE requirement" on one bisect:
62,517 ms vs 5,609 ms, an 11× slowdown. Withdrawn. The same machine has since
produced native pipeline creation at **70,459 and 205,527 ms on identical
code**, and web total boot from 5.6 s to 73.6 s. One run from it is not
evidence.

The compatibility ground — which never needed the number — stands and is now
stated as sufficient.

**Blast radius extended past LAWS.md, deliberately, and this is the report of
it.** PORT_6c recorded the measurement "at its site and in L14", so
`console.hpp` carried the same numbers plus the sentence *"L14 carries this
measurement as law."* Editing only LAWS.md would have produced a tree strictly
worse than either doing both or doing neither: a comment asserting a retracted
number, citing a law that no longer makes the claim, greppable by the next
reader asking why the limits are modest. That is P4's failure mode exactly.
P7 governs — take the minimal form, report it — and the minimal form is both
rooms. Nothing else moved.

### The law candidate, unnumbered, for Jean's ordinal

**WHERE A TIMER POINTS — a timer names where the wait surfaced, not where the
cost lives.** Filed in LAWS.md in the unnumbered-candidate form PROCESS_LAWS.md
already uses. The capture Jean supplied is an unusually clean witness because
both twins sit in one file:

| phase | web twin | native twin |
|---|---|---|
| `Total pipelines` | **14 ms** | 205,527 ms (as `Renderer init`) |
| `Patch system` | **56,887 ms** | 1,413 ms |
| `Total init` | 56,945 ms | 206,941 ms |

Neither twin is lying and neither measures what its label says. Web
per-pipeline times are wire-enqueue latency — the enqueues returned
immediately; the backend compile runs in-order in the browser's GPU process
and comes due on the first phase that **waits**, which is the patch phase. The
native twin shows the reverse, and its 1.4 s patch phase is the honest cost of
patch generation — the number STREAM_0 now correctly rests on.

---

## U2 — THE WEB TWIN NAMES ITS SILICON

Two edits at the web adapter-acquisition site (`console.hpp`, the
`__EMSCRIPTEN__` branch of `init`):

1. `powerPreference = HighPerformance` — the call passed `nullptr` before,
   i.e. "browser's choice".
2. `[Device] adapter: <vendor> | <architecture> | <device> | <description>`
   from `GetInfo`, in the success arm.

Native has enumerated and logged since PROBE_1 C1, with that unit's own reason
attached: *"every METER number is uninterpretable without this line."* The web
twin had no such line — and the capture proves the cost: it reports
`Total pipelines 14 ms` and `Patch system 56,887 ms` with **nothing in the log
saying which GPU produced them**. The standing assumption that the browser
runs the HD 5500 rather than the 920M is, today, a presumption; the whole
D3D12-fence account in the standing verdict rests on which one it is.

Empty fields print `?` rather than nothing — some builds redact, and a blank
is indistinguishable from a line that never ran. **A capture reading all `?`
is the RESOLVE case: report it and stop, no fallback plumbing.** The printing
shape is what makes that case recognisable in one read.

---

## U3 — THE SHELL, AND THE BINDINGS

Three states in `index.html`. **Zero wasm edits** — no `EM_ASM` hook was
needed; the page already owns `print`/`printErr` and every trigger is a line
the program prints for its own reasons.

### Bindings, reported

| what | bound to |
|---|---|
| **dismissal** | `Controls:` at line start — printed once the frame loop is live, the cleanest "world visible" moment the log offers. The veil cross-dissolves over 900 ms so it lands inside boot's own FADE_IN rather than cutting to a finished frame. |
| **FALLBACK** | `navigator.gpu` falsy, tested **before** the wasm is injected; `RequestAdapter failed:`; `Failed to create WebGPU instance`; `RequestDevice failed (full adapter passthrough)` |
| **LOST** | `[Device] LOST` |

Only the **passthrough** device failure is terminal. The modest request failing
is followed by a reissue (`request_device_web`), and showing the card there
would be a lie about a boot still in progress.

The gate ordering is the one real constraint in the file: an unsupported phone
pays nothing for a download it cannot use. Tested by **truthiness**, not by
`'gpu' in navigator` — a browser can carry the property and hand back
`undefined`; `in` returns true for both and the visitor eats the download. My
first draft used `in`; the harness caught it.

### Status wording

Phases print on *completion*, so each line announces what is **about** to run.
The long wait on the web twin sits between `Renderer init` and `Patch system`
— 56.9 s in the capture — which is where the deferred compile storm comes due,
so that is where the shell says *"Growing the terrain — the long part."*

Counts are **parsed, never typed**: the painting total is read off
`found N paintings`; pipelines are counted bare because the program prints no
total. A literal `57` or `60` here would be a P5 violation waiting for the
next painting to be added.

### DPR — bound, not tuned

PORT_3c derives the effective ratio from `glfwGetWindowSize` (CSS px) against
the framebuffer size, and its banner names this file as the other half of the
contract: *"The canvas ELEMENT still fills the page — web/index.html sizes it
with CSS (width/height 100%)."* So the canvas is sized by CSS only;
`.width`/`.height` are never touched from JS, with the reason written above the
rule. `MAX_DEVICE_PIXEL_RATIO = 1.5` is the future knob and was not moved.

### Verified, not asserted

Driven headless in Chromium against a stub replaying the real boot lines.
Five scenarios, all passing:

| scenario | result |
|---|---|
| no WebGPU | FALLBACK card, **wasm not fetched** |
| mid-boot | veil up, `Hanging the paintings (1/57)` — total parsed from the log |
| full boot | veil lifts on `Controls:`, hint appears |
| boot then LOST | LOST card replaces the world, reload offered, hint hidden |
| landscape 844×390 | canvas 844×390, nothing scrolls |

In all five the canvas matches the viewport exactly and document scroll is
dead. The harness is in the session scratchpad, not the tree — this repo has
no web test infrastructure and SHIP_0 asked for none. Say the word and it
lands.

**Two bugs my own review caught before commit**, recorded because the second
is the interesting one: the `'gpu' in navigator` test above, and a single
`settled` flag that let the 4-minute watchdog card outlive a boot that
eventually succeeded — a slow phone would have got a permanent apology with
the piece running behind it. Split into `terminal` (a designed end, nothing
overrides) and `shown` (something other than the veil).

**CONTACT is a placeholder** pointing at the repo's issues. This page is built
to be deployed publicly; putting a personal email on it is Jean's call, not
the file's default. The poster slots are marked in both cards.

---

## U4 — DEPLOY

The three artifacts do not exist in the tree — `.gitignore`'d since PORT_1d U3,
correctly, as build output. So the inventory could not be a static table;
`tools/web_dist.py` is the unit's executable form.

```
python tools/web_dist.py            inventory + verdict + write dist/
python tools/web_dist.py --check    inventory + verdict only
```

It reads sizes off disk, applies the host rule, and refuses to act on the
RESOLVE branch. All three exit paths exercised against synthetic artifacts:
`0` deploy-ready, `2` build first, `3` RESOLVE.

### The .data size, predicted from what IS in the tree

The preload is `--preload-file assets@/assets` plus `world.wgsl`
(`CMakeLists.txt`, the `EMSCRIPTEN` branch):

| | bytes | MiB |
|---|---|---|
| `assets/` — 57 files | 9,072,698 | 8.65 |
| `world.wgsl` | 618,986 | 0.59 |
| **`the_board.data` predicted** | **~9,691,684** | **~9.24** |

**~9.2 MiB is the mobile download cost**, plus wasm+js. The whole first visit
should land in the low teens of MiB uncompressed, so the verdict will be
**Cloudflare Pages** unless the `.wasm` is far larger than this program's shape
suggests. Recorded, **not optimized** — Jean's directive parks that until the
phone verdict, and the script says so in its own output.

Host rule, encoded exactly: every file ≤ 25 MiB → Cloudflare Pages; else →
GitHub Pages (~100 MiB/file); neither → RESOLVE with three repackaging options
named for Jean's ruling rather than taken.

**Headers: none.** Single-threaded, no COOP/COEP; adding them would be risk
without benefit. **HTTPS is mandatory** — WebGPU needs a secure context and a
LAN IP is not one, which is why "serve it locally and open it on the phone" is
not a phone test.

### PHONE PROTOCOL (verbatim, as U4 requires)

- Android Chrome; console via `chrome://inspect` over USB.
- iPhone: iOS 26 / Safari 26 has WebGPU by default.
- First visit pays the compile storm + patch phase — seconds-class on phone
  compilers; a much faster revisit is the pipeline cache, not a bug. The
  loading state (U3) makes the wait honest.
- If it dies, the LOST card appears — capture the console anyway, timestamps
  on.

### CAPTURE PROTOCOL, standing

DevTools timestamps **ON** — the delta from last submission to loss
discriminates a ~2 s TDR from a ~10–15 s watchdog kill.

---

## WHAT THE SUPPLIED CAPTURE ALREADY SETTLES

Reading it for SHIP_0 closed three open questions elsewhere, at no cost:

1. **PORT_6 U2 is answered.** That unit was left "awaiting U1's log" — does the
   modest device get discarded? The capture says
   `[Device] modest device accepted — NO DISCARD`, with
   `granted vs floor: maxTextureDimension2D=8192/2048
   maxStorageBuffersPerShaderStage=8/8 maxUniformBufferBindingSize=65536/65536`.
   No discard occurs; the device loss has another cause. The hypothesis that
   sent PORT_6 U2 to a deferral is falsified.
2. **C6 and OPT_1b are confirmed live in a browser**, not just by census:
   `maxStorageBuffersPerShaderStage=8/8` is C6's fit, and the GPU budget's
   largest allocation reads `Patch Heightfield Array (225x256x256, RGBA16Float;
   225 = Dim::MAX_ACTIVE_PATCHES)` at 112.5 MiB — OPT_1b's 225 layers.
3. **My own O0-g correction is corroborated independently.** The capture prints
   `[SignalLayout] 12 sources unbound (no audio source)` — the empty socket,
   confirming that `has_mode_bias` cannot leave rest and E3 is unreachable. The
   first version of the OPT_1 report claimed the opposite; the log agrees with
   the correction, not the claim.

### One observation, deliberately not acted on

The web capture's death line is:

```
A valid external Instance reference no longer exists.
[Device] LOST reason=1 : A valid external Instance reference no longer exists.
```

That is not the D3D12 fence signature the standing verdict describes
(`CheckAndUpdateCompletedSerials`, `Device removed reason: S_OK`), and it is
the exact message PORT_4a's instance anchor exists to prevent (`g_instanceAnchor`,
"the WebGPU instance outlives the frame loop"). **No fix is proposed and none
was attempted** — the verdict closes this loss as a debugging target and names
the phone as the next witness, and that instruction is respected here. It is
recorded only so that if the phone dies with the *same* string, the anchor is
the first place to look rather than the last.

---

## STREAM_0 AND PIPE_0 — READ, PARKED, NOT STARTED

Both carry their own parking notice. Summarised so the phone verdict can
re-open them without a re-read:

**STREAM_0 — does `fullRegen` die?** Boot generates the 7×7 priority window
(49 patches) synchronously; the other 176 already stream at 4/frame.
`reset_surface` is called from exactly two places — boot and TEARDOWN — and
both hit the same `fullRegen` block, so every mood change pays the same
synchronous freeze boot does. Honest sizing, corrected: the seconds class
(1,413 ms in the native capture), not the 52 s once attributed here — that was
the compile storm surfacing in the first waiting phase. The case now rests on
three grounds: a no-teleportation law violation; a QR destination cannot open
onto a frozen page; and one giant uninterrupted submission burst is the shape
that trips driver watchdogs on weak GPUs. Two knobs await Jean's ruling (boot
budget B same as transitions or elevated; fade gate on full 7×7 or centre 3×3).

**PIPE_0 — do the 60 pipelines compile in parallel?** Switch boot-time creation
to `CreateComputePipelineAsync` / `CreateRenderPipelineAsync`, collect the
futures, gate the end of renderer init on one barrier. Web: Chromium compiles
async pipelines on a GPU-process worker pool and disk-caches per origin.
Native: Dawn's platform worker pool, if its default pool parallelizes. No WGSL,
no bindings. Its own RESOLVE triggers are already written.

Sequencing per the handoff: phone renders → SHIP_1 touch sticks → Jean's
aesthetic pass → publish + QR. Phone fails at the same point → the bug is ours,
first suspect a loop bounded by data that differs on the web.

---

## FOR JEAN

**Three things need your hands**, all of them account- or proxy-blocked:

1. `git push origin --delete claude/cut-1-limits-fit` — verified merged, my
   delete is silently dropped by the proxy.
2. `git tag web-first-boot-2026-08 && git push origin web-first-boot-2026-08`.
3. The deploy itself: build, `python tools/web_dist.py`, then the printed
   commands.

**One decision before the URL is public:** the contact link in `index.html` is
a placeholder pointing at the repo's issues, and both poster slots are empty
gradients. Those are your aesthetic gate; the shell works without them and
looks deliberately unfinished with them.

**And the STOP:** this branch is 8 commits ahead of master and the web build
you captured is behind it. Merge before the next web capture, or the numbers
will not be comparable to anything — including each other.

**What the next capture will tell you that this one could not:** which GPU the
browser actually chose (`[Device] adapter:`), whether the rest skip ever fires
(`[Card] live-card field`), and whether `powerPreference` moved the answer.
That is three open questions closed by one line each, on a build you have not
run yet.
