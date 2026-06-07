# 7T — Analyzer Objects: Target Spec

*A living record of what the musical reading objects are becoming. The existing
modules are a half-baked snapshot; the 7T md files are the intent; this document
is the bridge — the agreed shape we revise the code toward, and the hedge against
losing detail when context is compressed.*

**This document holds only what we have settled together.** Threads still open are
listed once, as bare stubs, at the end — not fleshed out, so the body reads as
decisions rather than intentions.

**How to read each section.** A short *motivation* (why the object exists, the
reasoning that fixed its shape), then a code-shaped block:

```
HOLDS   what state it owns
READS   what substrate it consumes
EMITS   what it hands out
NOT     the boundary — what it deliberately does not do
```

plus pseudo-code where the logic is load-bearing.

---

## 0. Operating principles

The constraints every object below respects.

- **Measure, don't interpret.** The analyzer emits honest facts. Meaning is bound
  downstream, in the coupling. Over-measure, under-interpret: emit the richest raw
  form; folds (membership thresholds, re-origining, naming, cross-channel union)
  wait on the far side.
- **External MIDI only.** `MidiPort` ← Ableton is the sole source. `MidiFile` and
  `KeyboardMidi` are out of scope; the loop/seek beat-space and the debug keyboard
  channel go with them.
- **Code is truth, comments are suspect.** Behaviour is read from the code, never
  from its narration.
- **Allocation-free after init.** Fixed storage, no heap during runtime.
- **The boundary holds.** Only bridge data crosses analysis → render. The analyzer
  never knows about vertices, cameras, or palettes.

---

## 1. The substrate: active vs completed

**Motivation.** A single MIDI message is thin: it says a note started or stopped,
on which channel, at what pitch (hence pitch class and octave), with what velocity.
Length — the one quantity named beside pitch — is not in a single event; it exists
only once an onset is paired with its later offset. That pairing is the spine. A
note-on is an onset with no offset yet; a note-off closes the pair. The present is
the set of onsets still waiting; the past is the set of closed pairs. Playhead and
Wagon are these two halves. Length sits on the seam: provisional and growing on the
active side (`anchor − onset`), fixed on the completed side (`offset − onset`).

```
MidiStream (one per channel)
HOLDS   ActiveSet     — notes sounding now (onset known, offset undetermined)
        CompletedRing — finished pairs, ordered by completion; the ONLY history
                        store in the pipeline; pruned to retention_beats
READS   MidiEvents routed to its channel
EMITS   snapshot (active set @ beat) ; history (completed ring, read-only)
NOT     no derived readings ; no held reign ; no memory of derived state.
        It stores note FACTS, for ~retention_beats, and nothing else.
```

There is no time series of derived readings and no store for a held reference.
Anything that must survive longer than retention, or that is a derived held value,
needs its own home (see §4).

---

## 2. Playhead — the present, and only the present

**Motivation.** The Playhead tracks action at the anchor: signal or silence. That
binary — active set non-empty vs empty — is the synchronicity primitive itself,
together-in-time stripped of pitch and meaning; the note content riding on it is
the pitch-aware layer. It carries no held memory of the previous note: that memory
lives on a different clock and is its own object (§3). What stays is present-tense,
including frame-edge detection — "a note just onset" is the synchronicity event,
the boundary of now, not a relation to a distant past.

```
Playhead
HOLDS   active set at the anchor ; one frame of mask (for edge detection)
READS   the stream snapshot (active notes) at the anchor beat
EMITS   current set + mask ; gate / silence ;
        onsets & releases THIS frame ;
        state_duration (how long the present state has held)
NOT     no PREVIOUS, no gap_duration, no held memory.
        Frame-rate, ephemeral — recomputed each frame, gone the next.
```

---

## 3. PreviousEvent — held memory of the prior onset-group

**Motivation.** "Previous" must survive a gap of any length; a window cannot
promise that, since once the gap exceeds the span the previous falls out. So it is
a *held* value, latched when it happens and kept. The criterion is onset-grouping,
not release: melodic motion is attack-to-attack, so the previous is the prior group
of near-simultaneous onsets. Release-keying only approximates this and is what
breaks on legato. For the melodic interval we take the shortest distance from the
current note to the previous group — minimal-motion, no voice tracking. We keep all
the group's data. The same onset-simultaneity rule defines three things at once: the
current chord, the previous chord, and the step between them. The drone needs no
special case — a held note onsets once and never re-latches, and the drone's harmony
is the reign's job, not previous's.

```
PreviousEvent  — a latch, not a window
HOLDS   the prior onset-group, WHOLE — each note's pitch, velocity, onset,
        offset — latched when a new group opens; survives any silence
READS   onsets off the stream; nothing windowed, nothing pruned
EMITS   the group's full data ;
        temporal distance    = anchor − group.onset ;
        pitch distance(cur)  = min |cur.pitch − member.pitch|   (min-motion)
NOT     no span, no pruning, no voice tracking, no release grouping

GROUP   onset-simultaneity within `tolerance`:
          on onset e:
            open empty            -> open = {e}; t0 = e.onset
            e.onset − t0 ≤ tol    -> open += e
            else                  -> previous = open      // latch
                                     open = {e}; t0 = e.onset
```

The tolerance is empirical — settle it in the lab. A very spread arpeggio then reads
as a melodic sequence rather than one chord; that is correct, not a defect.

---

## 4. The held-value-surviving-silence primitive

**Motivation.** PreviousEvent and the reign are the same shape: a value last-seen,
updated on an event, persisting through any gap. They differ only in speed —
previous is fast (per onset-group), the reign is slow (trans-window). Recognizing
them as one primitive is the storage-side answer to the docs' "Homes" question: the
system has no home for a held, event-driven value, which is why both are missing or
faked.

```
HeldValue<T>  (the shape)
HOLDS   the latched T ; the beat it was latched
READS   the event that updates it
EMITS   the held T ; distance-to-anchor (anchor − latched_beat)
NOT     not windowed, not pruned, not recomputed per frame
```

PreviousEvent is the fast instance. The reign is the slow instance — the held bass
origin that carries the drone's harmony. Its full design is not yet settled (see
the open registry).

---

## 5. Chord / simultaneous-group operations

**Motivation.** A family of operations dedicated to the simultaneous group — the
chord sounding now. They use the same onset-simultaneity grouping that defines the
current chord, and they live on the present substrate. Most need only the Playhead;
the one that measures a chosen voice against the reign reaches for the reign.

```
ChordOps  (operate on the present onset-group)
READS   the Playhead's current onset-group
ZERO    the group's lowest (local zero)
EMITS   intervals among the simultaneous notes (DISTANCE; signed; raw)
NOT     no window, no previous, no release logic
```

---

## 6. Wagon — completed pairs within a span

**Motivation.** The Wagon registers a temporary history of finished notes over a
span. Completed-only is deliberate: it makes Playhead and Wagon *additive*, because
a note is either active or completed, never both, so their contents are disjoint and
the windowed picture is their clean sum. Two consequences. The active contribution to
any windowed read comes from the Playhead side and is summed in — never via the
Wagon's `include_active`, which would reach into the same notes and double-count.
Straddling *completed* notes (started before the window, finished inside) are the
Wagon's job: clip to the window and count the in-window span — this is what lets
held/pedal notes register. Length is the coordinate that makes a sync point
load-bearing: it is a continuous in-window span, and misalignment corrupts it.
Membership does not bend that way, so the sync discipline binds the length-weighted
reads and nothing else.

```
Wagon
HOLDS   readout of completed notes overlapping [anchor−span, anchor],
        each clipped to the window (window_onset, window_offset)
READS   the stream history (completed ring)
EMITS   per-note in-window span, pitch, velocity ; counts
NOT     no active notes (that contribution comes from the Playhead) ;
        no held memory
```

---

## 7. Operations declare their context — the menu, not the pipeline

**Motivation.** The apparatus — present, window, previous, reign — is a menu, not a
mandatory pipeline. Each operation reads only the substrate it needs. The pitch set
is the proof: membership is "which classes appear," a pure aggregate, so no time
relation touches it. Channels follow the same logic and instantiate only the
operations they run: a territory-only channel needs neither previous nor reign.

```
Operation  (model)
DECLARES  substrate ∈ { present, window, previous, reign, another operation }
EMITS     a measurement — a vector or a scalar, never a fold
Channel   instantiates only the operations it runs, and therefore only the
          substrate machinery those operations require
```

---

## 8. Designing a statistic: the pipeline

**Motivation.** This is the mental model for how every statistic is built. The
stages read linearly — Channel → Context → Operations → Contract — but the
Operations stage is a dependency graph: operations feed operations. Every statistic
grows from one universal root — lift the notes the context holds into a
representation (the most basic read there is). From that root, branches transform;
some stop at the first level, some build further; some take inputs from other
branches. Per-channel subgraphs feed a few cross-channel *compound* nodes, which are
not special machinery — just nodes whose inputs span channels (the pc-set union
consumes each channel's twelve-vector). The contract is the sink.

```
Statistic = a DAG rooted in a context
ROOT      lift the context's notes into a representation (the basic read)
NODE      a pure function of its inputs (context readouts and/or other nodes)
EDGE      carries a representation (vector / tuple), never a hidden scalar
COMPOUND  a node whose inputs span channels (e.g. pc-set union) — not special
SINK      the contract: serialize the published nodes; no logic
GRADIENT  projections & aggregations = measurement (stay on the edges) ;
          thresholds & namings        = folds (downstream of the sink)
```

The measure/fold gradient is where "measure, don't interpret" lives inside the DAG:
collapsing to the twelve-vector is the basis, not an opinion, so it stays; "which
classes are nonzero" is a fold and waits at or past the sink.

**The currency is the vector.** Every edge carries a vector — a note is its tuple of
one-hots (pitch class, octave, length, velocity); an aggregate is a twelve-vector.
Linear algebra is the point, so operations read and write vectors. Vectorization
happens at the *read boundary* — the root — never at MIDI ingestion: length is not
known at the event, so the stream stays note-facts and the root lift turns the
context's notes into vectors. Nodes then transform vectors with a small vocabulary,
each leaving its source intact (a projection reads a slice, it does not destroy the
rest — consistent with "keep all data").

```
CURRENCY  every edge carries a vector — a note is a tuple of one-hots
          (pc, octave, length, velocity); an aggregate is a twelve-vector
LIFT      vectorize at the read boundary (the root), never at ingestion;
          the stream stays note-facts, the lift makes the vectors
VOCAB     nodes transform vectors:
            project  (collapse octave; take the pc sub-vector)
            select   (argmin for the lowest — yields a one-hot)
            reduce   (sum to a histogram)
            rotate   (re-origin onto the reign)
SCALE     vectors are small — pc-space is 12, the present is a handful of notes;
          never heavy dense arrays
```

The representation is lifted to its *rich* form (the full tuple); the current
operation set is not baked into it. Materializing only the components some node
actually consumes is an implementation choice, deferred to the production line (see
the open registry).

---

## 9. The transport — one wire, no Train

**Motivation.** The Train conflated three jobs: owning the analyzers, composing
them, and shipping the result. The pipeline gives each a separate home, so the Train
is removed — keeping it would only re-bundle the three and let the AnalysisSignal
override it again, as it already did. There is one transport: the signal plus its
descriptor, fed by a pure serializer that flattens the published vector nodes. No
competing contract. The metaphor survives as vocabulary — track = channel, wagon =
context window — without a Train class. A cross-channel compound is a multi-input
node, not an owning container.

```
Object inventory (after removing the Train)
CONTEXT     per-channel, stateful — owns the windows (Playhead / Wagon /
            PreviousEvent / reign); updated each frame
OPERATIONS  pure functions — the DAG; no owner, no state
SERIALIZER  the sink — flattens published vector nodes into the signal + a layout
            descriptor; no logic
SIGNAL      the one wire that crosses to render; resolved by name via the descriptor
NO TRAIN    its three jobs (own / compose / ship) now live in
            context / operations / serializer
```

---

## 10. The canvas as coding interface — the goal

**Motivation.** The canvas is where the DAG is wired. The whole refinement effort
aims at one outcome: composing a statistic in the canvas should read as *declaring*
the DAG, with no musical logic hand-written. That is the inverse of today's canvas,
where the union is hand-coded and the representation is shattered.

```
Canvas invariants (what makes it a clean interface)
- operations are PURE functions of their inputs (no hidden state)
- the only state lives in the context objects
- edges carry REPRESENTATIONS (vectors), not scalars
- compounds are MULTI-INPUT nodes, not bespoke combine code
- the contract is a PURE SINK — a projection of published nodes
GOAL  composing a statistic = pick channel(s), name the context, list the
      operations and their dependencies, mark which to publish.
      A declared DAG. No musical logic hand-written.
```

How much of this becomes first-class structs is deferred on purpose: design the
product before the production line. Nail these invariants and the structs fall out
of them; guess the structs first and the canvas's current sins get encoded into the
type system.

---

## 11. Pitch bend — parked

**Motivation.** A continuous controller over time, not an atom with an onset and
offset — a trajectory; you read a curve, never one message. It fights the discrete
one-hot basis (a bend smears a note between pitch classes) and modulates active
notes, so it lives on the active side and never completes. Revisit deliberately,
later.

---

## 12. Process

Decisions live in this document. Objects are revised toward it; the document is
revised as each open thread closes. It is the reload point if the conversation is
lost.

---

## Open threads — not yet settled

Listed once, deliberately not designed.

- **The combine mechanism.** The active⊕completed sum over one window at a sync
  point. The constraint is settled (active from the Playhead, summed, at a sync
  point); the named object is not.
- **The reign.** Its home, its clock, the duration-weighted estimator and its
  empirical constants, the categorical turnover, and the pre-modulation hint.
- **The descriptor format.** How the layout the serializer writes expresses nested /
  tuple-of-tuple shapes (the lean is flat floats with a nestable, path-resolved
  descriptor, unratified).
- **Materialization.** Computing only the vector components some node actually
  consumes — a production-line optimization, deferred.
- **Structural homes for compounds.** Where cross-channel compound nodes get wired
  in the canvas (by hand vs a convenience struct) — the deferred struct question.
- **The interval profile.** The window re-origined on the reign, one-hot per note at
  its interval above the bass. Depends on the combine, the reign, and re-origining.
- **The simultaneity tolerance value** (§3) — empirical.
- **The octave and velocity one-hot axes** — the next axes after pitch class; the
  note as a tuple of one-hots.
