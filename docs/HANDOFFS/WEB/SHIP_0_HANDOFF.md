# SHIP_0 — HANDOFF (CC)

Contract: reformed register. Outcome invariants and blast radii below; CC binds
mechanisms by reading the tree and reports the binding. Tiers: EXECUTE /
RESOLVE (report options, do not improvise) / STOP.

Campaign goal: put the web twin on a real HTTPS URL, and make every future
console capture name its silicon and carry timestamps. No behavior of the
artwork changes in this campaign.

FXC note: no WGSL edits, no binding changes anywhere in SHIP_0. The FXC banner
law is untouched by construction.

---

## U1 — LAWS.md correction  [EXECUTE]

Recon: read the law entry sourced to commit `017367e` (the limits/boot-timing
entry).

Edit outcome:
- REMOVE the performance claim: "62,517 ms with adapter-maximum limits vs
  5,609 ms with core defaults" (or however the tree words it), and any sentence
  resting on that comparison.
- KEEP the compatibility rationale: modest limits are required for
  compatibility (L14, C6). That part is true and earned.
- ADD, in the same entry: "Patch-system timing on the development machine
  varies by an order of magnitude between runs; no single-run comparison from
  it is evidence. Bracket: identical code measured 49–62 s and, on 2026-08-07
  with Chrome open beside it, 1,223 ms."
- ADD as a law candidate (Jean stamps the ordinal per the tree's own preamble
  permanence rule): "A timer names where the wait surfaced, not where the
  cost lives. On the web, per-pipeline creation times are wire-enqueue
  latency; the backend compile cost of all pipelines executes in order in the
  browser's GPU process and lands on the first phase that waits on the queue.
  Witness: web 'Patch system 51,801 / 55,913 ms' ≈ native 'Total pipelines
  70,459 ms' — one cost, two attributions."

Invariant: after U1, no law in the tree rests on a single-run timing from this
machine, and no future ruling treats a web-side per-pipeline time as a
compile cost.

Blast radius: `LAWS.md` only.

---

## U2 — Witness identity (web half only)  [EXECUTE]

The native half is already satisfied: the 2026-08-07 boot log carries full
adapter enumeration, `Adapter selected: index=2` (NVIDIA GeForce 920M, D3D12,
driver 25.21.14.2531 / 425.31), and the Dawn revision. Verify that logging is
committed on master, not local-only; if committed, native needs nothing.

Web twin, two edits at the adapter-acquisition site:

1. Request `powerPreference = high-performance` in the
   `WGPURequestAdapterOptions`. Rationale: harmless on single-GPU phones,
   correct default for a real-time artwork, and on dual-GPU Windows machines
   modern Chromium honors it — the browser may legitimately reach the 920M,
   the same silicon native is green on.
2. Log what came back, one line via `wgpuAdapterGetInfo` (WGPUAdapterInfo:
   vendor, architecture, device, description), placed by the existing
   `[Device]` block:

       [Device] adapter: <vendor> | <architecture> | <device> | <description>

Recon: read the web-branch device-acquisition site before binding.

Invariant: every web capture from now on names its silicon on one line. The
standing presumption — browser runs the Intel HD 5500 (driver 20.19.15.4703,
2016) — becomes a logged fact or gets overturned.

RESOLVE trigger: if `WGPUAdapterInfo` fields come back empty in the browser
(some builds redact), report what came back and stop; do not add fallback
plumbing.

Blast radius: web-branch adapter request + boot logging. No bindings, no WGSL.

---

## U3 — Deploy  [EXECUTE prep; Jean executes account-touching steps]

Goal: the `web/` output on a static HTTPS host. WebGPU requires a secure
context; `localhost` is privileged, LAN IPs are not — HTTPS is mandatory.

1. Inventory the web build output. Report each file and its size:
   expected shape: `index.html`, `the_board.js`, `the_board.wasm`, and the
   MEMFS package (`.data`) carrying the 57 paintings + `world.wgsl`.
2. Host selection by the numbers:
   - Cloudflare Pages: free, fastest to stand up, **25 MiB per-file cap**.
   - GitHub Pages: free, ~100 MiB per-file, 1 GB repo soft cap.
   - If every file ≤ 25 MiB → Cloudflare Pages. Else → GitHub Pages.
   RESOLVE trigger: if any single file exceeds the chosen host's cap, report
   packaging options (split preload, lazy-fetch assets) — do not repackage on
   your own authority.
3. Prepare a `web/dist/` (or equivalent) folder containing exactly the files
   the page needs, plus the exact deploy commands for Jean:
   - Cloudflare: `npx wrangler pages deploy <dist> --project-name 7t` (or the
     dashboard drag-drop path).
   - GitHub Pages: branch/folder convention, one paragraph.
4. Headers: none special. No COOP/COEP — the build is single-threaded, no
   SharedArrayBuffer. All listed hosts serve `application/wasm` correctly.
5. Phone protocol (for Jean, include verbatim in your report):
   - Android: open the URL in Chrome; for the console, `chrome://inspect`
     over USB from any desktop gives the same DevTools.
   - iPhone: iOS 26 / Safari 26 has WebGPU on by default.
   - Expectation-setting: first visit pays the pipeline compile storm plus
     the synchronous patch phase. Phone compilers (Metal/Vulkan via Tint) are
     fast; expect seconds, not this laptop's ~52 s — but wait through black
     for up to two minutes before declaring failure, and capture the console
     if it dies. Chromium disk-caches compiled pipelines per origin, so a
     revisit that boots much faster than the first visit is expected, not
     suspicious.

Invariant: at the end of U3 there exists a URL that any phone can open. That
URL is also the QR destination — the deliverable itself.

---

## U4 — Capture protocol  [EXECUTE: record in PROCESS_LAWS.md or the audit dir]

All future browser captures: DevTools console **timestamps ON**
(Console settings → Show timestamps).

Why (record this rationale with the protocol): the delta between the last
submission log and the loss line discriminates mechanisms —
~2 s ⇒ Windows TDR on a single submission; ~10–15 s ⇒ Chromium GPU-process
watchdog kill; ~0 s ⇒ immediate fault. The 2026-08 captures cannot answer
this because they carry no timestamps.

---

## U5 — Housekeeping  [EXECUTE with recon; tag push is Jean's]

Recon first: verify each ref exists before acting.

    git push origin --delete claude/cut-1-limits-fit
    git push origin --delete claude/port-0-seam-census-5z0at8

Jean pushes the tag (proxy blocks CC tag pushes):

    git tag -a web-first-boot-2026-08 -m "Lean twin boots in the browser: 60 pipelines, MEMFS, frame loop reached. Device loss open."
    git push origin web-first-boot-2026-08

`rescue-175fd17`: merge or cherry-pick its housekeeping commit to master when
convenient, then delete the branch. Verify contents before merging.

---

## Standing context for this campaign

- The device loss is browser-manufactured: `dawn.json` marks
  `device lost reason` as string-converted on Emscripten with values 3/4
  excluded — `reason=1` can only arrive from `device.lost.reason ==
  "unknown"`. The message is Chromium's internal teardown echo. Nothing in
  our tree drops anything. Do not spend edits on the wasm-side instance.
- Chrome + Edge = one Chromium witness, not two.
- The `CheckAndUpdateCompletedSerials … S_OK` text belongs to an earlier run;
  it is absent from the 2026-08 captures.
- Adapter facts (2026-08-07 native log): native selects index=2, NVIDIA
  GeForce 920M / D3D12 / driver 425.31 (2019). The Intel HD 5500 sits on
  driver 20.19.15.4703 (2016) and is the browser's presumed adapter until
  U2's log line confirms. Dawn revision f0bf8ab547. "Native green, web dead"
  is an adapter comparison, not a platform comparison, until proven otherwise.
- Attribution finding: web per-pipeline "0 ms" is wire-enqueue; the compile
  storm executes in-order in the GPU process and surfaces in the first
  waiting phase (the 52 s web "patch" ≈ the 70 s native "pipelines"). Since
  the wire preserves order, all compiles complete before the first frame —
  the device loss at first heavy frame is therefore NOT a compile storm; it
  remains the heaviest frame on the weakest adapter under the oldest driver.
- Patch system acquitted: 1,223 ms on 2026-08-07, identical code, Chrome
  open beside it. PORT_5 is innocent of the 49–62 s cluster.
