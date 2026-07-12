# THE 7T ENTITY CONTRACT — v0
Repo home: src/docs/ · Ratification: the ribbon must PASS §7 against this
text; any failure means the contract or the ribbon is lying — fix the liar.
Amendment: same-commit with any change to the practiced law (PENDING[stage]
when a doc must lead). Ruling authority: Jean.

## PREAMBLE — THE DEAL
You deliver an entity; we never edit it. Your entity runs in our world
exactly as you designed it, forever, in silence — and becomes musical only
through the surface you declare. In exchange for structural compliance and
an honest datasheet, the system binds itself: your code is inviolate, your
idle is sacred, and every coupling that ever touches your entity flows
through pipes you named. The canvas is the only hands.

## §1 — DEFINITIONS
ENTITY: a self-contained inhabitant (look + motion + lifecycle) delivered
as one module. MODULE: a self-contained header at file scope owning its
state and laws, or — under the transitional regime of the cartridge
constitution's amended §1 (the header ladder) — a class-body include in
the cartridge organism; both lawful until the last module converts.
PIPE: one runtime-writable parameter row in the bank. READING: one
published musical quantity in the analysis contract. REST: a pipe's idle
value. COUPLING: a canvas decode writing pipes from readings. CONDUCTOR:
your per-frame entry point, called once. DATASHEET: your surface,
documented to the wiggle-test standard (§6). PANEL: the future generic
renderer over declared rows — also the certification bench.

## §2 — WHAT YOU DELIVER
D1. The module file(s), per §3 anatomy, its kind named in the header's
    identity sentence.
D2. Shader section(s) if you realize geometry — numbered into world.wgsl's
    § scheme, mirrors comment-pinned, hazards of §9 respected.
D3. The datasheet, per §6, true at delivery.
D4. Console defaults that look intentional with zero music — your entity
    ships as a finished procedural work; music is a bonus layer.

## §3 — STRUCTURAL REQUIREMENTS
R1. ANATOMY: header (identity sentence, public-surface box, SEAM tags) →
    tuning console → vocabulary → registry (stride convention stated) →
    tiers beside their governing law → runtime state → author seats →
    laws → conductor → lifecycle. Every section present; an empty
    section is a statement, a missing one is a question. (The in-module
    target-surface section is RETIRED: the bank rows ARE the surface —
    R10's declaration replaced it; the ribbon's T2 deletion is the
    precedent.)
R2. CONSTANTS: no number hides inline in a law; design-time dials live in
    the console with the governing law in the adjacent comment.
R3. STATE: one XxxState struct; module functions take it explicitly;
    cross-cutting needs reach only through the conductor keyhole.
R4. CONDUCTOR: one function, called once per frame from its position in
    the cartridge's frame score (constitution §3 — one movement, one
    call); consumers consume, only the conductor orchestrates; no
    per-entity logic leaks into the cartridge.
R5. LIFECYCLE: select → place → commit → evict, all four owned; COMMIT is
    the succession choke point — identity and rebirth are structural
    there, never policed at call sites; eviction restores rest truthfully.
R6. DETERMINISM: the seed and the spawn moment are the entity's whole
    biography — every seed-drawn property is a stateless,
    order-independent channel: same seed, same draws, always. (Live
    steering by the player is the instrument's hands, by design outside
    the biography.)
R7. COMMENTS state present behavior; history lives in git; exceptions are
    tagged in place with their retirement condition (constitution §5
    format).
R8. ENCODINGS AND BANNERS follow the repo conventions (stated in the
    constitution §0's mirror-conventions clause); deltas to both
    cartridges byte-identical unless a divergence is authored and
    declared.

## §4 — THE SURFACE (your couplable self)
R9. COMPLETENESS OF DECLARED INTENT: every parameter you intend ever to
    be played is DECLARED at delivery — in the datasheet, as a row or a
    ledger entry with class and rest — because at a boundary we cannot
    cross later, the surface is the product. Bank rows and wires may
    land wires-on-demand as couplings arrive (G4: growth appends); the
    DECLARATION is what must be complete, not the plumbing.
R10. ROWS: each pipe is a PARAM_LAYOUT row — name (entity.param), slot,
    width, shape, REST. Names are namespaced by your entity.
R11. REST = IDENTITY (the safety contract): with every pipe at rest, your
    entity is byte-identical to its pure procedural self. Silence
    reproduces your design exactly. Prefer rests that are the formula's
    own zero — it buys call-site classification for free.
R12. DEVIATIONS COMPOSE ADDITIVELY over inviolate idleness: couplings add
    to your baseline; they never suppress it. (Recorded lesson: presence-
    ducking punishes short gestures.)
R13. FLUSH SHIPS INSIDE: your conductor contains the bank-reading,
    deviation-composing seams for every declared pipe — reading valid
    rows, composing over your idle bases (spawn snapshots where lifetime
    values need an anchor), clamped so invariants survive any music
    (entity-side, or write-side where the canvas's goal law is the
    declared guardrail — the datasheet states which, per row).
R14. CLASSES DECLARED per pipe: L-global (body-wide), LH (through
    history/propagation), D (discrete, state-inheriting on swap), C
    (identity — never a live target). Frequencies are never pipes; expose
    a rate into an accumulator instead.
R15. ENVELOPES: where a pipe distinguishes attack from release, the spans
    are your constants and the direction is chosen at the call site
    (goal == rest ⇒ release).
R16. WIRES: partial-write GPU uploads for per-frame pipes arrive with the
    pipe (wires-on-demand), offsetof-addressed, beside your other wires.

## §5 — BOUNDARIES YOU HONOR
R17. THE DOORS LAW: the world enters through layouts (read the substrate
    freely — that is every entity's charter); PEERS enter through the
    frame signal, CPU-composed (you never read another entity's state,
    CPU or GPU); MUSIC enters through the bank (you never read organs or
    readings; your entity must not know what drove it).
R18. GPU SOVEREIGNTY: CPU authors intent, GPU realizes geometry; no law
    is duplicated across the bus; mirrors are comment-pinned and, where
    constants twin (hot-reload tuning), the GPU set is the tuning
    authority with lockstep comments both sides and a stated drift test.
R19. THE KEYHOLE: through the cartridge pointer you may reach the
    declared services — the clocks, the canvas params, the GPU wires,
    your own bindings, the spine-owned player/flight state and raw
    input (writes SEAM-tagged in place), the spine's terrain and
    spawn-engine services — and nothing entity-private of a peer.
R20. TIME: periodic idles run on the beat-rate follower scaled by your
    reference-BPM anchor, so your authored feel is defined at the anchor
    and converges to live tempo.

## §6 — THE DATASHEET (the wiggle-test standard)
One table, repo-resident, updated in the same commit as any surface
change. Per row: name · slot · width · shape · class · REST · what the
eye sees when it moves · guardrail. Plus: the casting note (which voice,
if pre-cast), envelope constants, and a LEDGER of parked couplings and
known interactions. THE TEST: a stranger reading only this table can
wiggle each row on the panel and predict the screen. If a claim needs
the source to verify, the datasheet is incomplete.
PROTOTYPE-REGIME NOTE (2026-07-12): this standard is DORMANT during
the prototype (constitution §1, prototype-regime clause) — per-file
prose is retired until ship, when the datasheet reactivates as a §7
certification requirement, written once against the settled structure.

## §7 — CERTIFICATION (the bench; performable today)
C1. Build both cartridges; FXC clean; zero warnings from your sections.
C2. GOLDEN AT REST: boot with no music — your entity runs its procedural
    self; two runs, same seed, pixel-stable frames.
C3. THE WIGGLE: each declared row driven by hand across its range —
    the screen matches the datasheet's per-row prediction; rests restore
    identity exactly.
C4. FAIL-SOFT: with your pipes absent from the layout, your entity runs
    unchanged (binds warn and unbind; nothing corrupts).
C5. INVARIANTS SURVIVE MUSIC: rows driven to extremes simultaneously —
    clamps hold; no teleporting, no NaNs, no escape from guardrails.
C6. LIFECYCLE: spawn/evict cycles under mood transitions leak nothing and
    leave rest-state true.
C7. CENSUS: exceptions all tagged per R7; the adversarial census finds no
    untagged deviation, no unstated consumer, no law duplicated across
    the bus.
Pass all seven and the entity is playable by anyone who can read §6.

## §8 — WHAT THE SYSTEM GUARANTEES (our side of the deal)
G1. WE NEVER EDIT YOUR CODE. Integration, coupling, and retuning happen
    in the canvas and the consoles you shipped.
G2. YOUR REST IS SACRED: no coupling exists without your declared pipe;
    at rest you render exactly as delivered, forever.
G3. BINDS FAIL SOFT: absent readings or rows degrade to your procedural
    self with a logged warning, never a crash, never corruption.
G4. ROW STABILITY: once assigned, your slots are not reshuffled beneath
    you; growth appends.
G5. CASTING IS DECLARED: if your entity is voiced, the assignment lives
    in the canvas casting sheet, one constant, visible.
G6. YOUR EXCEPTIONS ARE HONORED: tagged deviations with retirement
    conditions are ledger entries, not fix-me invitations.
G7. THE CANVAS CARRIES THE MUSIC: decodes, organs, envelopes evolve
    freely on our side; your surface is the stable interface between us.

## §9 — KNOWN SUBSTRATE HAZARDS (read before writing WGSL)
H1. FXC fragility: instance structs in hot loops stay lean and
    byte-pinned (GPUPierInstance, 48 B with its static_assert, is the
    living instance — successor of the retired 32-byte SolidInstance
    rule); the collision/ground chain admits no new runtime branching —
    bounded loops over uniform counts and uniform function choice, not
    branches (evaluate_pier's caller is the pattern); texture/array
    stamps hang compilation; new collision shapes proxy existing rect
    solids; CPU dead-reckoning handles footprint precision.
H2. Attribute adjacency: never place a function between @vertex/@fragment/
    @compute and its fn — insertions name an exact host.
H3. Uniform layout: WGSL's uniform address space wants 16-byte array
    strides — prefer vec3/vec4 packing (pulse_data is the worked
    example); mirrors are byte-pinned with static_asserts CPU-side and
    stated sizes shader-side. STANDING EXCEPTION, eyes open: the frame
    signal's stats array<f32,64> rides packed in a uniform binding and
    works on the current Dawn (the sky words after it read true) — do
    not "fix" it to vec4s without moving the CPU mirror in the same
    commit, and re-verify on any Dawn upgrade.
H4. Sized aggregates initialized short are compiler-silent nullptr
    factories — size arrays to contents.
H5. Hot-reload: world.wgsl is live; prefer WGSL residence for purely
    visual constants; declare rebuild-tier dials in your console.

## APPENDIX — THE WORKED EXAMPLE
ribbon.inl and src/docs/ribbon_color_coupling_datasheet.md are the
reference instance of every requirement above: the anatomy (R1), the
commit choke (R5), the seed biography (R6), the flush seams and spawn
snapshots (R13), the frame-law mirrors with the rider as drift test
(R18), the casting (G5). Copy the idioms; do not reinvent them. The
certification transcript of the ribbon against §7 accompanies this
contract as its ratification.
