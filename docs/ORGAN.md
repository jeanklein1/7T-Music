# ORGAN — the master control panel

The panel is a VIEW of the program, never a second copy. Its subjects
are the persistent CPU homes CHORD built — one struct per cadence
block, one write path per change. The panel puts dials on those homes.

## The three tiers of a parameter
Every enrolled parameter has, in principle, three tiers:
1. CONSTANT — its design value (a stop, drawn or not).
2. IDLE — its oscillation around the design value when uncoupled or
   unstimulated: amplitude, period, waveform (the tremulant).
3. COUPLED — the ear that drives it (one of SignalLayout's seven),
   with gain and shape (the keys).
ORGAN_0 ships tier 1 live and reserves columns for tiers 2 and 3 in
the same registry — one registry, three tiers, never three systems.
Idleness law restated: a parameter without a coupling in this run is
a control-panel design constant; any procedural parameter is a
coupling candidate by design.

## The compiled-registry law
The registry is COMPILED, not parsed. Enrollment is one macro line
beside the field it enrolls; the compiler produces the manifest from
the same declaration the program reads. A registry the compiler did
not build can drift; this one cannot. (The lineage: 0b-4's markers,
the schema as one home, witnesses that read their subjects.)

## The sovereignty boundary
The panel writes CPU-authored homes only: DesignConfig, the CHORD
blocks (agent_room, scene_constants, frame_r's lighting), field_bus's
authored rows. GPU truth (positions, vp, camera, simulation state) is
never a panel subject — the panel may one day DISPLAY it, but the
write path does not exist by law.

## The write path
A panel edit writes the home struct member and marks the block dirty.
Dirty blocks flush once, at the frame boundary, through the block's
existing upload — the same reconciliation philosophy as ACQ_0 and
CAP_2: intent asserted per frame, not per event. A slider drag is
many events and one WriteBuffer.

## The contested-dial instrument (O1a)
A dial the panel CAN write is not yet a dial the panel OWNS. Some
homes have a second author — a mood apply, a per-frame updater — and
there the panel's word is not wrong, only temporary. Which dials
those are is DISCOVERED, never hand-censused: organ_set shadows the
bytes that land in the home, and once a frame, at the flush boundary,
the program re-reads the home and asks whether it still says them.
Never disagreed is FREE; stood a while and then lost it is EVENT;
lost it at once is PER-FRAME. The evidence behind a reading is the
survival count — the frames the panel's word stood. The instrument
reports and does not act: what a reading MEANS for the panel is a
separate ruling, and that ruling wants a census as its evidence.

## Access
The panel exists only under `?organ=1`; backtick toggles visibility.
Without the flag, no DOM is built, no export is called, the audience
path is byte-identical. The panel is an instrument, not the art.
