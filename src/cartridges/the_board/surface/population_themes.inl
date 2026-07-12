// ─── population_themes.inl (S2 · IMPL: post-class definitions) ────
// Born at LADDER-6 (S2 extraction): history in audit/LADDER.md.
//
// The envelope machine's per-patch step. Reaches the keyhole only for
// the flag-gated census dump (dump_entity_census, spawn_engine.hpp);
// theme_short_name lives here, beside the names it abbreviates.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.

namespace t7 {
namespace the_board {

// Census home (LADDER-6 3b): sole consumer is the census dump below;
// the names are this module's vocabulary.
inline const char* theme_short_name(uint32_t theme) {
    static const char* NAMES[] = { "transition", "monumental", "colonnade", "antenna", "barren" };
    return (theme < THEME_COUNT) ? NAMES[theme] : "???";
}

inline uint32_t evaluate_theme_envelope(ThemesState& ts, Cartridge* c, uint32_t tile_seed_value) {
    (void)c;
    auto& env = ts.envelope_;

    // Build effective weights
    float weights[THEME_COUNT];
    float total = 0.0f;
    for (uint32_t i = 0; i < THEME_COUNT; i++) {
        if (env.cooldowns[i] > 0) {
            weights[i] = 0.0f;
        }
        else if ((int32_t)i == env.active) {
            weights[i] = theme_envelope_weight(THEMES[i], env.elapsed);
        }
        else {
            weights[i] = THEME_BASE_WEIGHT;
        }
        total += weights[i];
    }
    if (total < 0.001f) total = 1.0f;

    // Roll from weights
    float roll = cpu_hash_f(tile_seed_value, 370u);
    uint32_t selected = THEME_COUNT - 1;
    float cumul = 0.0f;
    for (uint32_t i = 0; i < THEME_COUNT; i++) {
        cumul += weights[i] / total;
        if (roll < cumul) { selected = i; break; }
    }

    // State transitions
    if ((int32_t)selected != env.active) {
        if (env.active >= 0) {
            env.cooldowns[env.active] = THEMES[env.active].cooldown;
        }
        env.active = (int32_t)selected;
        env.elapsed = 0;

        // Census dump on theme transition
#ifdef DIAG_ENTITY_CENSUS
        dump_entity_census(c, theme_short_name(selected));
#endif
    }
    else {
        env.elapsed++;
    }

    // Check expiry
    if (env.active >= 0) {
        const auto& th = THEMES[env.active];
        if (env.elapsed >= th.sustain + th.decay) {
            env.cooldowns[env.active] = th.cooldown;
            env.active = -1;
            env.elapsed = 0;
        }
    }

    // Tick cooldowns
    for (uint32_t i = 0; i < THEME_COUNT; i++) {
        if (env.cooldowns[i] > 0) env.cooldowns[i]--;
    }

    return selected;
}

} // namespace the_board
} // namespace t7
