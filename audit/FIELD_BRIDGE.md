# FIELD BRIDGE — session handover (written at Phase B, branch held)

READ FIRST in any fresh window continuing the field work. The tree
is the memory; this file is the index.

## STATE
- Phase A COMPLETE: FIELD_2/2t/3/3b/4/4b on master. Stamped consts
  (world.wgsl ~2214): SLACK 3.0, K 300, FMAX 600, gains 4/1/4.
  gain applies AFTER the FMAX clamp. F5 drives the beacon
  (authored table g2:5, writer in phase_motion_drivers).
- Phase B: B0 (ribbon floor guarantee) on master. Branch
  `claude/field-phase-b` holds B1 (sphere presence migrated),
  B2 (agent presence migrated; possessed emits ×4), B2b (possessed
  emits to AGENT lanes only), B3 (orb 1/d² sep kernel retired —
  field_pair called in place). NOT MERGED. Jean owes: one
  checkout/build session gating B1/B2b/B3, the METER
  DispatchCompute paste (owed since FIELD_2), merge, branch death.

## RULINGS (binding)
1. Presence migrates; APPROACH stays behavioral (flee/dodge live).
2. The POINT's rows are a separate arc, undesigned: row_cube_push,
   row_sphere_push (+camera twin), row_point_flee/agents-part,
   occupier_contact-for-agents. Migrate-vs-permanent needs Jean.
3. Possessed: emits ×4 (PAWN_CONTACT_MASS_MULT) to agents only;
   never subscribes. Floater←possessed stays row_cube_push's until
   the point arc rules.
4. ONE LAW, TWO TRANSPORTS: the field_forces buffer where classes
   must hear each other; the direct field_pair call where a
   subsystem is closed (orbs are the precedent).
5. The lure (authored→ribbon) is LATERAL ONLY; the pen owns
   altitude; B0's clamp makes the floor guarantee true.
6. No anti-windup mechanism exists; no standing fy source remains.

## KNOWN DEFECTS / DIALS
- Beacon clot: ring capacity ~9 cubes at r0 25 (circumference /
  shell width); dials r0/R/S in cartridge.hpp (rebuild to tune —
  the three-homes problem). Authored SHAPES (rings/arcs) unbuilt.
- Gates B1/B2 are RE-PRICINGS: field reach = SLACK×3 vs old
  one-body-width CONTACT shells. Contingencies named, unbuilt:
  FIELD_SLACK_AGENT (distance), subscriber-mass division (yield).
- SPAWN_3 parked: F6 newborns seat inside render radius on move.

## QUEUE (fresh session picks from here)
1. Gates + merge + METER paste (Jean's machine).
2. The point-grammar arc — design round first.
3. Geometry coverage: palms/cacti/blades/galleries/GoL/indoor —
   parity-with-agents was the FIELD_3 ruling; revisit scope.
4. Rings as individual subscribers (they emit; only the head yields).
5. FIELD_5: ch1 → beacon strength/radius (rows coupling-eligible).
6. The three-homes constants (WGSL consts + ribbon.hpp mirrors +
   beacon statics) → the master control panel's opening exhibit.
7. Layer E still waits: the frame is geometry-bound (~22/29 ms);
   the field's cost is unmeasured until the METER paste lands.

## ANCHORS
field consts world.wgsl ~2214 · field_pair/field_sum ~7718/7736 ·
the room: binding_registry g2:0–5 · authored writer:
cartridge.hpp phase_motion_drivers · ribbon field block:
ribbon.hpp (trio + lookahead + lure) · recon corpus:
audit/FIELD_1_REPORT.md · laws: src/docs/LAWS.md.
