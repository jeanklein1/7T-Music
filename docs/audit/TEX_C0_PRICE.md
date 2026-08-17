# TEX_C0 — the price sheet

*Written at BEQUEST (2026-08-17), against the tree at `a5e84bf`. TEX_C0
was parked behind the feature wallet and Jean's eye on ASTC banding
since PROBATE_F. It is priced here and shelved with its bill attached —
not refused, and not scheduled. The next gallery campaign opens this
document first.*

**The headline, before the detail: TEX_C0 as originally sketched is
impossible.** Not expensive — impossible. The recon below is what no
earlier document knew, and it changes the shape of the work rather than
its cost.

---

## 1. The architecture as found

Paintings are a **three-tier estate**. `authoredStagingTexture_` (32
layers, disk images decoded in through stb) and
`snapshotStagingTexture_` (32 layers, where the photographer *renders*)
both feed `promote_to_exhibition`, a `CopyTextureToTexture` into ONE
shared `exhibitionTexture_` (40 layers) that the gallery and wall
shaders sample. All three arrays are created at `Dim::PAINTING_RESOLUTION`
= 512² and — this is the load-bearing part — at the same format:
`colorFormat_`, which is not a constant at all but **the surface's own
swapchain format**, taken from `caps.formats[0]` at device init
(typically `BGRA8Unorm`).

## 2. The blocking fact

Texture-to-texture copies require **identical formats** on both sides;
snapshots are GPU-rendered and a render target cannot be a compressed
format. So the shared exhibition array can never carry a compressed
format while snapshots promote into it — and it is worse than "shared",
because the format is inherited from the swapchain and is not the
program's to choose.

That single sentence retires the original sketch. There is no version of
"compress the exhibition array" that survives the promotion copy.

## 3. The viable design

Split the exhibition in two, along the seam the blocking fact already
draws:

- **`authored-exhibition`** — compressed. **ASTC on mobile / BC7 on
  desktop.** The adapter guarantee makes that pair sufficient: a WebGPU
  adapter offers `texture-compression-bc` or
  (`texture-compression-etc2` **and** `texture-compression-astc`), so
  ASTC+BC covers every device that can compress at all. **ETC2 is
  unneeded** — it only ever arrives alongside ASTC, and ASTC is the
  better of the two.
- **`snapshot-exhibition`** — `colorFormat_`, exactly as today. The
  photographer keeps rendering into a format it can render into, and
  keeps promoting into a format that matches.

The authored path then skips staging entirely where it can: encoded
blocks upload straight to the compressed array, and the
decode-then-promote round trip disappears with the intermediate.

## 4. The bill

Every item below is work this program does not have today.

**Binding surface.** One new gallery-band g3 texture seat, its array
view, and the gallery/wall layout growth to carry it — plus the schema
rows, the regenerated mirrors, and the `--check` relations that follow
any seat. The gallery band has room; the seat is not free.

**Shaders.** A routing bit in `UnifiedPaintingSlot` (authored vs
snapshot) and a sample branch in the wall and gallery fragment shaders.
Two arrays now answer one question, so every sample site must know which
one it is asking.

**The wallet.** `texture-compression-astc` and `texture-compression-bc`
flip from `vaulted` to `granted` — but *conditionally*, which the wallet
cannot currently express. `FEATURES` needs a `request` policy field
(request-if-offered, as against require), the request site needs to ask
for what the adapter actually carries, and **witness F-1 must be
redefined**: it currently holds the request to be exactly the granted
set, and a conditional grant is not that. That redefinition is the
delicate part of this bill — F-1 is what makes the wallet a law rather
than a list.

**Tooling and assets.** An offline encoder toolchain in `tools/`
(ASTC and BC7 both), a source-hash manifest so an unencoded or stale
painting fails the build loudly rather than shipping a hole, and
per-format shipping in `web_dist.py` — which also means the host verdict
is recomputed, since the exhibition roughly triples in deployed bytes.

**The committed encoded assets**, with the arithmetic rather than a
remembered number. 57 paintings at 512²:

| format | B/texel | ratio | 57 paintings |
|---|---|---|---|
| `bgra8` (today) | 4.000 | 1× | 57.0 MiB |
| BC7 / ASTC 4×4 | 1.000 | 4× | 14.2 MiB |
| ASTC 5×5 | 0.640 | 6.2× | 9.1 MiB |
| ASTC 6×6 | 0.444 | 9× | 6.3 MiB |
| ASTC 8×8 | 0.250 | 16× | 3.6 MiB |

Two formats ship, so the committed cost is the ASTC row **plus** the BC7
row: **~21 MiB at ASTC 6×6, ~28 MiB at 4×4**. Against 8.65 MiB of
source JPEGs today, that is the real number to weigh — and it is the
line item most likely to change the answer, because it is paid in the
repository and in every deploy, forever, whether or not a given visitor
benefits.

## 5. The prize

Resident painting memory today is **104 MiB** — exhibition 40 layers +
authored staging 32 + snapshot staging 32, all at 1 MiB per layer.

The authored half of that is what compresses. At the block sizes worth
considering the authored exhibition shrinks **4× (BC7 / ASTC 4×4) to 9×
(ASTC 6×6)** — desktop is capped at 4× because BC7 has no smaller block,
so the mobile and desktop dividends are genuinely different and only the
mobile one is large. Boot decode for authored paintings is eliminated
outright, and gallery-scene sampling bandwidth falls with the resident
size.

*(The original sketch's "~5–8×" is not wrong so much as unsourced: it
lands between ASTC 5×5 and 6×6 and quietly assumes the mobile path. The
table above is the honest form.)*

## 6. The gate

**A gallery-campaign slot, and Jean's eye on ASTC banding.** Lossy block
compression on painted surfaces is exactly where banding shows, and no
instrument in this program can rule on it — it is a judgment about how
the work looks.

So the campaign that takes this ships a **`?paintings=legacy` A/B
toggle** for precisely that gate: the same wall, the same seed, one
query parameter apart. Without it the ruling would be a memory of
yesterday's screen against today's, which is not a comparison.

---

## Standing

**HELD.** The blocking fact is permanent and the design above is the
only one that survives it. The bill is large and falls mostly on the
wallet's F-1 redefinition and on ~21–28 MiB of committed binaries; the
prize is large on mobile and modest on desktop. Nothing here expires,
and nothing here should be started outside a gallery campaign that has
already booked Jean's eye for the banding gate.
