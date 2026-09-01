// ORGAN — THE FRAME BOUNDARY. Included inside class Cartridge (cartridge.hpp):
// these are member functions and reach the cartridge's deps and queue directly.
// pawn.cpp's frame() calls them once per frame, before input and update.
// organ_registry.hpp owns the flags and masks consumed here and knows neither
// the deps nor the queue, which is why the consumption lives at this tier.

            // ORGAN — the frame boundary: doors, definition re-speaks and
            // the flush, taken once per frame for the one world.
            void organ_flush(wgpu::Queue& queue) {
                // DOOR 0, Re-speak definitions: it raises the flags the
                // lines below already consume, so the boundary then does
                // what it does every frame. A definition write is durable
                // but not immediate, and this is that round trip.
                const uint32_t doors = t7::organ::take_doors_pending();
                if (doors & (1u << t7::organ::ORGAN_DOOR_RESPEAK)) {
                    t7::organ::g_tier_def_dirty = true;
                    t7::organ::raise_orb_definition();
                }
                // THE SKY'S TWO PLAYER-OWNED FACTS, reached as doors: each
                // calls the function its key calls — the same KP_8 / KP_7
                // commands, guards and partial upload unchanged.
                if (doors & (1u << t7::organ::ORGAN_DOOR_ORB_RULE))
                    cycle_orb_motion_rule(orbs_state_, &orbs_deps_, queue);
                if (doors & (1u << t7::organ::ORGAN_DOOR_ORB_GESTURE))
                    cycle_orb_gesture(orbs_state_, &orbs_deps_, queue);

                // THE HOST DOOR (RIBBON_1) — the same transaction key R
                // presses, with its own guards. A door with a parameter,
                // taken once; possess() early-returns on a no-op and on a
                // RIBBON request with no ribbon to ride.
                {
                    uint32_t host = 0;
                    if (t7::organ::take_go_host(host))
                        possess(&input_deps_, static_cast<PointHost>(host));
                }

                // THE ATMOSPHERE BANK'S FLAG. The twelve sky rows address
                // ATMOS_LIVE directly, so they raise no selecting flag and
                // nothing has to be guarded: there is one sky and it is
                // always this world's. A second arm stood above this one
                // until ONE_WORLD-II U2 — it took a definition flag and
                // dropped the write unless the target mood was the live
                // one, a guard whose whole subject was the six other
                // moods.
                if (t7::organ::take_atmos_dirty()) {
                    stage_sky(&sky_deps_, queue);
                }
                // The tier bank changed, so its author re-speaks the whole
                // registry in its own voice — idempotent by construction.
                // Once per frame however many edits arrived.
                if (t7::organ::take_tier_definition_dirty()) {
                    upload_agent_registries_to_gpu(&agents_deps_, queue);
                }
                // THE CONSOLE DIALS LAND BY THEIR READERS' CADENCES: dome,
                // noise and speed take targeted partials, base size raises
                // the orb re-speak. This sits IMMEDIATELY BEFORE the
                // take_orb_definition_dirty() block.

                // base_size is an OrbConsole field, so its offset means
                // nothing in OrbConfig's bit space: it raises the FLAG
                // with no touched bit, and this local is what keeps a
                // same-frame flock drag from swallowing it.
                bool console_reseed = false;
                if (const uint32_t cm = t7::organ::take_orb_console_dirty()) {
                    if (cm & 1u) gpuState_.upload_orb_dome_radius(queue,
                                     ORB_CONSOLE_LIVE.dome_radius);
                    if (cm & 4u) gpuState_.upload_orb_noise(queue,
                                     ORB_CONSOLE_LIVE.noise_floor);
                    // The master motion strength: a per-frame GPU read
                    // like dome and noise, so a targeted 4-byte partial and
                    // NO re-seed — it scales the sky's energy under the
                    // finger without replacing one orb.
                    if (cm & 8u) gpuState_.upload_orb_speed_mult(queue,
                                     ORB_CONSOLE_LIVE.speed_mult);
                    // A console edit is about the sky we are looking at,
                    // and there is only the one to be looking at.
                    if (cm & 2u) {
                        t7::organ::raise_orb_definition();
                        console_reseed = true;
                    }
                }
                // The orb bank changed, so its applier re-speaks. ONE SKY
                // (ONE_WORLD-II U1b): the bank was seven rows and the
                // boundary had to prove the edit was THIS world's before
                // applying it — a guard whose whole subject was the six
                // other moods. It is one row now, always this world's, so
                // the guard, the target id it compared, and the modulo that
                // aliased an out-of-range one all go.
                if (t7::organ::take_orb_definition_dirty()) {
                    // AN AUTHOR RE-SPEAKS NO MORE THAN THE EDIT REQUIRES.
                    // Four of the nineteen orb facts are baked into
                    // orb_state at init, so touching any of them re-seeds;
                    // the other fifteen ride the uniform upload.
                    //
                    // A RAISE WITH NO TOUCHED BITS MEANS EVERYTHING — the
                    // RESPEAK door, or a caller that did not say.
                    const uint32_t tm = t7::organ::take_orb_def_touched();
                    const bool reseed = console_reseed
                        || (tm == 0u)
                        || ((tm & t7::organ::ORB_RESEED_BITS) != 0u);
                    // THE MASK DRAINS EITHER WAY — take_orb_def_touched
                    // above is unconditional, so a disabled piece cannot
                    // leave bits behind; only the applier is gated.
                    if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (b) — sky dome never configured
                        configure_orbs(orbs_state_, &orbs_deps_,
                            ORB_LIVE, queue, reseed);
                }
                // THE RULE WINDOW, refreshed here and nowhere else. The
                // sky's rule and gesture are player-owned and live in
                // OrbsState; the panel READS them.

                // ONE WRITER, ONE SITE — the boundary, not the door
                // handlers: cycle_orb_motion_rule answers to the door above
                // and to KP_8, cycle_orb_gesture to KP_7, and a refresh
                // here cannot go stale whoever turned the rule.
                t7::organ::set_orb_rule_view(
                    orbs_state_.current_motion_rule,
                    orbs_state_.gesture_idx[orbs_state_.current_motion_rule & 3u],
                    orbs_state_.active, orbs_state_.count);
                gpuState_.organ_flush(queue);
            }
