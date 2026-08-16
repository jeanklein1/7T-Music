# PROBATE_SEAL — the serve incident, and the door that closes behind it

Five units executed, one unpriced, six commits. PROBATE closes.

---

## Header

| field | value |
|---|---|
| HEAD before | `f86b5ad` — PROBATE_E6 |
| HEAD after | this commit (`PROBATE_SEAL6`) |
| branch | `master`, per O-1 |
| baseline (§S0.1) | `--check` **GREEN**, `wgsl_gate.py` **GREEN**, tree clean at `f86b5ad` — no halt |

| # | commit | subject |
|---|---|---|
| 1 | `1e9a95d` | PROBATE_SEAL2: the serve is witnessed — the shader's hash is checked where the audience receives it |
| 2 | `004c6df` | PROBATE_SEAL3: the floor stops — no frame loop on a device the program already knows cannot run it; the audience meets a card, not a console |
| 3 | `3ad06be` | PROBATE_SEAL5: EF5 and EF6 ruled — precedent may be cited, authority may not be borrowed from the dead; minutes are stamped, never rewritten |
| 4 | this one | PROBATE_SEAL6: instrument close — the serve is witnessed, the floor stops, the campaign closes |

`PROBATE_SEAL1` was not written — §S0 disproved the branch that would
have needed it. `PROBATE_SEAL4` was not written — the archive it depends
on is not reachable from here. Both are below.

---

## §S0 — THE DISCRIMINATOR'S VERDICT

**The tree is clean. The incident is a SERVE defect.**

### Tree side (§S0.2) — clean, and precisely so

`git show HEAD:…/world.wgsl` at the named region shows `orb_vs` whole:
`+ cam_right * (quad_pos.x * orb.size)` intact, the function closing, the
file ending at **13 663 lines / 636 955 bytes** with `// END OF SCROLL`
and a single trailing LF. No `TREE-CORRUPT` flag; §S0a not entered.

### Where the cut fell — the forensic reading

| fact | value |
|---|---|
| reported error | `:13639:28 expected ')'` inside `orb_vs` |
| line 13639 in the clean tree | `        + cam_right * (quad_pos.x * orb.size)` |
| byte offset of 13639:28 | **636 290** of 636 955 |
| bytes missing | **665** — the served file ended mid-identifier, inside `quad_p|os` |
| chunk alignment | **none.** Not a multiple of 16 KiB, 64 KiB, 512 KiB or 1 MiB |

A parser that reaches EOF inside an unclosed parenthesis reports exactly
`expected ')'` at the last position it read — so the error names the
truncation point, not a defect. The file is 99.9 % intact; the last 665
bytes never arrived.

**The absence of chunk alignment is the informative part.** A packaging
bug writing in blocks, or a CDN storing in fixed objects, would cut on a
boundary. An unaligned tail loss 665 bytes from the end is the signature
of a transfer that stopped, not of a file that was written wrong.

### Packaging side (§S0.3) — disproved by inspection

`tools/web_dist.py` moves `the_board.js`, `the_board.wasm` and
`the_board.data` with **`shutil.copy2` — byte-exact**. The only
transformation in the whole script is `str.replace` of `__BUILD_ID__` in
`index.html`, a different file that contains no shader. **No regex has
ever run over shader bytes, and no size bookkeeping touches them.**
`world.wgsl` reaches the browser inside `the_board.data`, packed by
emscripten `--preload-file` (CMakeLists, `T7_WEB_SHADER`), and the
staleness hazard on that path was already closed by `LINK_DEPENDS`.

So the §S1 packaging branch **does not fire**: there is no named site to
fix, because there is no site that could cut a shader. Nothing was
committed for it.

*Empirical half not run:* a fresh local dist needs a build, and the three
artifacts are `.gitignore`d because CC never builds (O-4). I exercised
the packaging path instead with three synthetic artifacts, which proved
the real code path end to end — see the §S2 verification below — and then
removed them.

### Serve side (§S0.3) — UNFETCHABLE from here

`everexpandingboard.com` is **blocked by this environment's network
egress policy** — `CONNECT` refused with 403 at the gateway, confirmed
by both `curl` and the fetch tool, and visible in the proxy's own
`recentRelayFailures`. It is a policy denial, not a transient failure.
The live byte-length/sha256 comparison against the served artifacts
**could not be performed**. Flag `SERVE-UNFETCHABLE`.

### The verdict, stated plainly

The defect is **downstream of `dist/`** — upload, CDN object, or the
transfer to the device — and cannot be localized further from this
container. That is the §S1 upload/CDN branch: *nothing to commit in the
tree; rebuild and hand Jean the deploy.*

**And it is exactly why §S2 is the round's centre.** Every instrument
this program owns was pointed at the right thing and green, because
every one of them looks at the tree. The one question nobody could ask
was *did the audience receive what we shipped* — and after this round the
program asks it itself, in one line, before it does anything else.

---

## Flags

| # | unit | site | expected | found | action |
|---|---|---|---|---|---|
| SF1 | §S0.3 | the live artifacts | fetch `the_board.js?v=…` and the data package, compare sha256 against a fresh local dist | **egress blocked** — 403 CONNECT at the gateway, by policy, for `everexpandingboard.com` | `skipped-step` — the serve-side comparison is unrunnable from this container. The tree side and the packaging side were both settled by other means, and the verdict above stands on them. **Jean can close this in ten seconds with a hard refresh** (checklist item 1), which is why that item is first. |
| SF2 | §S1 | `web_dist.py` | a named site where packaging could cut a shader | **no such site exists** — `.data`/`.js`/`.wasm` are `shutil.copy2`; the only transform is a `str.replace` on `index.html` | `skipped-unit` — `PROBATE_SEAL1` unwritten. The unit's own condition ("Packaging defect") is false. Recorded rather than invented: a fix to a mechanism that cannot fail would be a comment claiming a defect that was never there. |
| SF3 | §S3.3 | `GetCompilationInfo` | "await … zero errors gates pipeline creation" | **it cannot be awaited on this twin.** The callback resolves on the browser's event loop; the boot path is synchronous and the build carries **no `-sASYNCIFY`**. A spin-wait would convert a wrong shader into a hung tab. `WaitAny` needs the same event loop; `TimedWaitAny` is not offered on this surface | `graduated` (P7, minimal form) — the request is **fired**, not awaited, and its verdict prints when it lands: real errors with real line numbers, plus `[Floor] STOP` so the visitor gets the card. **The corruption case is stopped synchronously and earlier**, by §S2's digest check, before a module is created at all. This arm is a net under everything else, and its comment says so rather than claiming to be a gate. My first draft *did* spin on `device_.Tick()`; the type-check caught that `Tick` does not exist on this generation, and the second look caught that even the right call would have been wrong. |
| SF4 | §S4 | `C:\dev\dawn` | recon the archived checkout; build standalone `tint` | **not reachable.** This session is a Linux container; the checkout is on Jean's Windows machine and `DAWN_REFERENCE.md` pins it to `Visual Studio 18 2026` + MSVC. No Dawn source is vendored (EF4) | `TINT-UNPRICED` — `PROBATE_SEAL4` unwritten, `wgsl_gate.py` grows no second arm. **Priced for Jean below** as a hypothesis, guarded per P1: I cannot open that machine, and a report that says "verified" about a machine the executor cannot open is the exact tell P1 names. |
| SF5 | §S2 | `EM_ASM_PTR` + `stringToNewUTF8` | the obvious way to read a JS string into C++ | the stub surface lacks `EM_ASM_PTR`, and `stringToNewUTF8` is a **runtime helper whose presence in the shipped glue I cannot verify** — F5F's exact failure class, one API over | `graduated` — rewritten to write bytes through `HEAPU8` into a stack buffer, which is `boot_params.hpp`'s existing proven pattern (it writes doubles through `HEAPF64`). Needs nothing but the heap view. Caught by type-checking the cartridge TU, not by review. |
| SF6 | §S3.1 | `FLOOR_MAX_IMMEDIATE_SIZE` | use the emitted floor constant | it lives in **`console.hpp`'s namespace**, and `cartridge.hpp` — glaw1's own translation unit — does not include console.hpp. The generated `.inc` has no include guard, and including it in a second namespace would put the constants somewhere else again | `graduated` — the floor is spelled `sizeof(GPUPatchParams)`, which is **the same expression NEEDS r7 sources from** (`binding_schema.py`). One home, no second copy, and the check cannot drift from the request. |
| SF7 | §S2/§S3 (method) | `renderer.hpp` | — | **nothing in this container had ever compiled `renderer.hpp`** — the console gate's TU is `console.hpp`, and glaw1 (which does compile `cartridge.hpp`) is Jean's. My PROBATE_I C++ edits had never been type-checked either | `graduated` — I ran the cartridge TU through the console gate's own stub surface and vendored headers (`clang++ -fsyntax-only`, `#include "cartridges/the_board/cartridge.hpp"`). It **caught three real errors** before they reached Jean (SF3, SF5, and an undeclared-identifier from the JS body). Deliberately **not pinned as a new gate**: glaw1 already compiles this TU, and the console gate exists precisely because glaw1 did *not* cover console.hpp. Adding a ceremony that duplicates glaw1 would be the P12 mistake. |
| SF8 | §S0 (observation) | the incident's most likely cause | — | 665 bytes lost at an **unaligned** offset, 99.9 % through the file | recorded, not acted on. A truncated transfer is the cheapest explanation and the one a hard refresh disproves or confirms immediately. §S2 makes the question answerable forever after, whatever the answer is this time. |

---

## Numbers

### The serve witness, verified end to end

| end | value |
|---|---|
| `hashlib.sha256(world.wgsl)[:8]` (build side, `web_dist.py`) | **`14079f5c`** |
| `t7::sha256_short` over the same bytes (boot side, `src/core/sha256.hpp`) | **`14079f5c`** |
| substituted into `dist/index.html` by a real `web_dist.py` run | **`14079f5c`** |
| bytes hashed | 636 955 |

The two implementations are held to each other by
`tools/gates/sha256_gate/`, which compiles the header and compares
against `hashlib` over seven vectors — including the three lengths where
SHA-256's padding rule changes behaviour (55, 56, 64 bytes) — **and over
the real `world.wgsl`**. Negative-controlled: one flipped init constant
turns it red.

That gate is not decoration. A witness whose two ends disagree by a
padding rule reports MISMATCH on a good serve, and a witness that cries
wolf gets switched off — which is how the corridor came to be unwatched.

### The floor's four arms

| arm | source of truth | checked where |
|---|---|---|
| dialect `immediate_address_space` | `navigator.gpu.wgslLanguageFeatures` (instance-scoped, F3-a) | `floorHolds`, before any GPU object |
| granted `maxImmediateSize >= 32` | `sizeof(GPUPatchParams)` — NEEDS r7's own source expression | `floorHolds` |
| `setImmediates` on the compute encoder | `GPUComputePassEncoder.prototype` — asked of the prototype, since creating a pass to ask would be a GPU object made before the floor that decides whether to make GPU objects | `floorHolds` |
| shader received == shader shipped | `SHADER_SHA` vs `sha256(shaderSource_)` | `loadShader`, before `CreateShaderModule` |

Any arm false → one `[Floor] STOP` line naming the arm → no module, no
pipelines, no frame loop → the shell matches the line and shows the card.

### What each console line becomes

| before | after |
|---|---|
| `:13639:28 expected ')'` — a syntax error in a file with no syntax error | `[Dist] world.wgsl sha=… expected=… MISMATCH` + `[Floor] STOP` |
| R3 narrates, then compiles the shader it just said cannot compile | `[Floor] STOP` before any GPU object exists |
| `[Pipeline] gen_patch_heights: 0 ms` × 59, success-shaped | `[Renderer] Shader compile (create call)` + `[Shader] world.wgsl compiled: N error(s)` |
| thousands of validation errors against a dead device | nothing runs; the boot ended at the floor |
| Firefox: uncaught throw, every frame | the floor stops it; the visitor meets the card |
| the visitor: a black canvas | *"This work needs a current Chromium-based browser with WebGPU immediates."* |

### The instruments

| quantity | §S0 baseline | §S6 close |
|---|---|---|
| Table H rows | 68 | 68 |
| computed preamble | `68 sites at this run` | `68 sites at this run` |
| Table D rows / filled | 14 / 14 | **14 / 14**, zero warnings |
| `E1-identity` | PASS | **PASS** — 4 rows, 4 layouts, all 4 entry points by name |
| room row | `5 of 8`, `0` demotable | unchanged |
| laws | L27 | **L28** |
| gates | `--check`, wgsl, console | **+ sha256** |

### Probe verdicts carried forward

| probe | verdict |
|---|---|
| EF3 — a newer naga | `naga-cli 30.0.0` is still the newest published; nothing to upgrade to |
| EF4 — tint in the pinned tree | no build system vendored; confirmed again |
| **SF4 — tint from the archive** | **unreachable from here.** Priced below |

**The price, as a guarded hypothesis (P1).** On the machine that holds
`C:\dev\dawn`, the standalone CLI is a target the reference's existing
configure line does not request. It would be, in the reference's own
pins: configure as documented, adding `-DTINT_BUILD_CMD_TOOLS=ON`, then
`cmake --build out --config Release --target tint_cmd_tint_cmd`. I have
**not** run this, cannot, and name it as a hypothesis rather than a
recipe: the target name and the flag are Dawn conventions, not facts I
verified against that checkout. Configure is ~2 minutes per the
reference; the build is tens of minutes. If it produces a `tint`
executable, `wgsl_gate.py` gains its second arm and validates the REAL
module — `requires`, `var<immediate>` and all — beside naga's shimmed
pass. **The residual stays either way:** the archive's Tint and the
Pixel's Chrome are different generations, so the gate would narrow the
blind spot, not close it. The boot remains the witness of record.

---

## Per-unit status

| unit | status | one line |
|---|---|---|
| §S0 | **done** | Tree clean; cut located at byte 636 290 of 636 955, unaligned, 665 short. Packaging disproved by inspection; serve side unfetchable (SF1). |
| §S1 | **skipped-unit** | Its condition is false — no packaging site can cut a shader (SF2). The upload/CDN branch has nothing to commit; the deploy is Jean's. |
| §S2 | **done** | One fact, two ends, verified to agree on `14079f5c` by a negative-controlled gate. Third refusal added to `web_dist.py`, and it fires before `rmtree` so a bad shell never costs the previous dist. |
| §S3 | **done, graduated** | Floor stops on four arms before any GPU object; the card is a third state beside `fallback` and `lost`, reusing the existing `showCard`. §S3.3 is a net, not a gate, and says so (SF3). |
| §S4 | **skipped-unit** | `TINT-UNPRICED` (SF4). Priced as a guarded hypothesis for the machine that can run it. |
| §S5 | **done** | EF5 restated in the banner's own words with L2 as past-tense provenance; EF6 stamped, both files, text untouched; L28 on the books. |
| §S6 | **done** | Ledger regenerated; all witnesses green; this report. |

---

## Jean's checklist

1. **Hard-refresh the Pixel first.** Before anything else, before any
   rebuild. It discriminates a transient fetch truncation in ten seconds
   and may already be the whole fix — SF8 says an unaligned 665-byte
   tail loss is what an interrupted transfer looks like. If it boots
   clean, the incident was the corridor and §S2 is now standing in it.
2. **Then rebuild and redeploy.** `web_dist.py` now prints a `shader
   sha` line and tells you exactly what the Pixel console must read.
3. **After redeploy, on the Pixel:**
   - `[Dist] world.wgsl sha=14079f5c expected=14079f5c MATCH`
   - all pipelines compile; `[Shader] world.wgsl compiled: 0 error(s)`
   - zero validation errors
   - **the terrain intact on the pinned seed** — heights, cell colours,
     GoL placement. This is also the still-outstanding step-4 word for
     PROBATE_I, and it has been waiting two rounds. If it is wrong, the
     words are *"the whole terrain is the same hill repeated"* or
     *"everything outside the first patch went flat"*.
4. **Firefox once more.** The audience card, not the spam. That is the
   PENDING line doing its job in public — and if Firefox has since
   shipped the dialect, it boots instead, which settles the banner the
   other way.
5. **Two things want your eye, not your keyboard:** the card's exact
   wording and look (§S3.2 — a sober default is shipped, the register is
   yours), and SF4's price, which only your machine can pay.
6. **Read §Flags.** SF1 and SF4 are environment walls, not defects. SF3
   and SF5–SF7 are places the tree outranked the handoff and the round
   followed the tree.

---

*Standing after this round, on the record and untouched: TEX_C0, the A2
price list (id-keyed), the agent merge (L24's named payment), the SOAK
watchlist, and the Tint gate — unpriced, with its price written down.*

---

**The campaign's one law, final form, earned five times over** — the naga
gate, the room filter, the hand-carried 74, the serve, and now the
compilation verdict that was never read: *a reference outlives its
referent, and a witness that never meets its subject will eventually
testify for the wrong side.*

Every corridor between the schema and the audience's screen now has a
witness standing in it. The last one to be built is the one that was
missing when the Pixel spoke — and the reason it was missing is that
nobody had ever stood where the audience stands and asked whether what
arrived was what was sent.
