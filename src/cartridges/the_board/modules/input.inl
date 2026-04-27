// ─── input.inl ──────────────────────────────────────────────────
//
// Keyboard, mouse, scroll. Key bindings, movement intent,
// camera mode toggle, render radius control.
//
// Included inside the Cartridge class body (private section).
// Depends on: musical.inl (toggle_mmode), pawn_aura.inl (aura flags)
// ─────────────────────────────────────────────────────────────────

// Platform workaround: GLFW key codes not available in all header configurations.
#ifndef GLFW_KEY_KP_0
#define GLFW_KEY_KP_0  320
#endif
#ifndef GLFW_KEY_LEFT_CONTROL
#define GLFW_KEY_LEFT_CONTROL   341
#endif
#ifndef GLFW_KEY_RIGHT_CONTROL
#define GLFW_KEY_RIGHT_CONTROL  345
#endif
#ifndef GLFW_KEY_KP_1
#define GLFW_KEY_KP_1  321
#endif
#ifndef GLFW_KEY_KP_2
#define GLFW_KEY_KP_2  322
#endif
#ifndef GLFW_KEY_KP_3
#define GLFW_KEY_KP_3  323
#endif
#ifndef GLFW_KEY_KP_4
#define GLFW_KEY_KP_4  324
#endif
#ifndef GLFW_KEY_KP_5
#define GLFW_KEY_KP_5  325
#endif
#ifndef GLFW_KEY_KP_6
#define GLFW_KEY_KP_6  326
#endif
#ifndef GLFW_KEY_KP_7
#define GLFW_KEY_KP_7  327
#endif
#ifndef GLFW_KEY_KP_8
#define GLFW_KEY_KP_8  328
#endif
#ifndef GLFW_KEY_KP_9
#define GLFW_KEY_KP_9  329
#endif
#ifndef GLFW_KEY_KP_DECIMAL
#define GLFW_KEY_KP_DECIMAL  330
#endif
#ifndef GLFW_KEY_CAPS_LOCK
#define GLFW_KEY_CAPS_LOCK   280
#endif
#ifndef GLFW_KEY_F1
#define GLFW_KEY_F1  290
#endif
#ifndef GLFW_KEY_F2
#define GLFW_KEY_F2  291
#endif
#ifndef GLFW_KEY_F3
#define GLFW_KEY_F3  292
#endif
#ifndef GLFW_KEY_F4
#define GLFW_KEY_F4  293
#endif
#ifndef GLFW_KEY_F5
#define GLFW_KEY_F5  294
#endif
#ifndef GLFW_KEY_F6
#define GLFW_KEY_F6  295
#endif
#ifndef GLFW_KEY_F7
#define GLFW_KEY_F7  296
#endif

void on_key_down(int key) {
    switch (key) {
    case GLFW_KEY_UP:    keys_.forward = true; break;
    case GLFW_KEY_DOWN:  keys_.backward = true; break;
    case GLFW_KEY_LEFT:  keys_.left = true; break;
    case GLFW_KEY_RIGHT: keys_.right = true; break;
    case GLFW_KEY_1:
        gpuState_.toggle_freeze_sphere();
        break;
    case GLFW_KEY_2:
        auraHeightEnabled_ = !auraHeightEnabled_;
        auraCfgDirty_ = true;
        std::cout << "[Aura] Height extrusion: " << (auraHeightEnabled_ ? "ON" : "OFF") << "\n";
        break;
    case GLFW_KEY_3:
        auraEnabled_ = !auraEnabled_;
        auraCfgDirty_ = true;
        std::cout << "[Aura] Field: " << (auraEnabled_ ? "ON" : "OFF") << "\n";
        break;
    case GLFW_KEY_5:
    {
        if (transitionPhase_ != TransitionPhase::IDLE) break;
        uint32_t mood = MOOD_OPEN_SUNSET;
        const auto& mp = MOOD_TABLE[mood];
        uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
        pendingDestination_ = { dest_seed, mp.finite, derive_finite_radius(dest_seed, mp), mood };
        transitionPhase_ = TransitionPhase::FADE_OUT;
        transitionTimer_ = 0.0f;
        std::cout << "[World] Transition (" << mood_name(mood) << "): seed " << activeSeed_
            << " -> " << pendingDestination_.seed << "\n";
    }
    break;
    case GLFW_KEY_6:
    {
        if (transitionPhase_ != TransitionPhase::IDLE) break;
        uint32_t mood = MOOD_INDOOR_FLAT;
        const auto& mp = MOOD_TABLE[mood];
        uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
        uint32_t radius = derive_finite_radius(dest_seed, mp);
        pendingDestination_ = { dest_seed, mp.finite, radius, mood };
        transitionPhase_ = TransitionPhase::FADE_OUT;
        transitionTimer_ = 0.0f;
        uint32_t side = 2 * radius + 1;
        std::cout << "[World] Transition (" << mood_name(mood) << " " << side << "x" << side
            << "): seed " << activeSeed_
            << " -> " << pendingDestination_.seed << "\n";
    }
    break;
    case GLFW_KEY_7:
    {
        if (transitionPhase_ != TransitionPhase::IDLE) break;
        uint32_t mood = MOOD_INDOOR_VAULT;
        const auto& mp = MOOD_TABLE[mood];
        uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
        uint32_t radius = derive_finite_radius(dest_seed, mp);
        pendingDestination_ = { dest_seed, mp.finite, radius, mood };
        transitionPhase_ = TransitionPhase::FADE_OUT;
        transitionTimer_ = 0.0f;
        uint32_t side = 2 * radius + 1;
        std::cout << "[World] Transition (" << mood_name(mood) << " " << side << "x" << side
            << "): seed " << activeSeed_
            << " -> " << pendingDestination_.seed << "\n";
    }
    break;
    case GLFW_KEY_8:
    {
        if (transitionPhase_ != TransitionPhase::IDLE) break;
        uint32_t mood = MOOD_FINITE_OUTDOOR;
        const auto& mp = MOOD_TABLE[mood];
        uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
        uint32_t radius = derive_finite_radius(dest_seed, mp);
        pendingDestination_ = { dest_seed, mp.finite, radius, mood };
        transitionPhase_ = TransitionPhase::FADE_OUT;
        transitionTimer_ = 0.0f;
        uint32_t side = 2 * radius + 1;
        std::cout << "[World] Transition (" << mood_name(mood) << " " << side << "x" << side
            << "): seed " << activeSeed_
            << " -> " << pendingDestination_.seed << "\n";
    }
    break;
    case GLFW_KEY_9:
    {
        if (transitionPhase_ != TransitionPhase::IDLE) break;
        uint32_t mood = MOOD_FINITE_OUTDOOR_REF;
        const auto& mp = MOOD_TABLE[mood];
        uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
        uint32_t radius = derive_finite_radius(dest_seed, mp);
        pendingDestination_ = { dest_seed, mp.finite, radius, mood };
        transitionPhase_ = TransitionPhase::FADE_OUT;
        transitionTimer_ = 0.0f;
        uint32_t side = 2 * radius + 1;
        std::cout << "[World] Transition (" << mood_name(mood) << " " << side << "x" << side
            << "): seed " << activeSeed_
            << " -> " << pendingDestination_.seed << "\n";
    }
    break;
    // ─── Musical animation mode toggles (numpad) ─────────────
    case GLFW_KEY_KP_1: toggle_mmode(MMODE_TERRAIN_WAVES);   break;
    case GLFW_KEY_KP_2: toggle_mmode(MMODE_COLOR_SHIFT);     break;
    case GLFW_KEY_KP_3: toggle_mmode(MMODE_CHECKER_SCATTER); break;
    case GLFW_KEY_KP_4: toggle_mmode(MMODE_PALETTE_DRIFT);   break;
    case GLFW_KEY_KP_5: toggle_mmode(MMODE_GOL_TEMPO);       break;
    case GLFW_KEY_KP_6: toggle_mmode(MMODE_AURA_EXPAND);     break;
    case GLFW_KEY_KP_7: toggle_mmode(MMODE_RADIAL_PULSE);    break;
    case GLFW_KEY_KP_8: {
        wgpu::Queue q = device_.GetQueue();
        cycle_orb_motion_rule(q);
        break;
    }
    case GLFW_KEY_KP_9: toggle_orb_anchor();                 break;
    case GLFW_KEY_KP_DECIMAL: {
        wgpu::Queue q = device_.GetQueue();
        cycle_orb_gesture(q);
        break;
    }
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL:
        toggle_fpv_mode();
        break;
    case GLFW_KEY_CAPS_LOCK: {
        wgpu::Queue q = device_.GetQueue();
        try_possess_nearest(q);
        break;
    }
    // ─── Diagnostic keys (F1-F6) ───────────────────────────────────
    // Why function keys: the analysis layer above the cartridge
    // consumes A-Z as MIDI piano notes ("A-Z=piano keys" in the boot
    // banner). Letter keys can't double as cartridge diagnostics
    // without playing notes every time you cycle a behavior. Function
    // keys are unclaimed and naturally read as "tools, not input."
    //
    //   F1 — cycle agent behavior override   (none → random_walk → ... → none)
    //   F2 — cycle agent tier override       (none → worker → ... → none)
    //   F3 — force-respawn agent population
    //   F4 — cycle cube behavior              (stationary → curlfield → phasewave)
    //   F5 — cycle floater coordination       (0.0 → 0.5 → 1.0)
    //   F6 — corral cubes around pawn         (diagnostic teleport)
    //   F7 — toggle cube kite mode            (cubes follow pawn on/off)
    case GLFW_KEY_F1: {
        wgpu::Queue q = device_.GetQueue();
        cycle_agent_behavior_override(q);
        break;
    }
    case GLFW_KEY_F2: {
        wgpu::Queue q = device_.GetQueue();
        cycle_agent_tier_override(q);
        break;
    }
    case GLFW_KEY_F3: {
        wgpu::Queue q = device_.GetQueue();
        force_respawn_population(q);
        break;
    }
    case GLFW_KEY_F4: {
        wgpu::Queue q = device_.GetQueue();
        cycle_cube_behavior_override(q);
        break;
    }
    case GLFW_KEY_F5: {
        cycle_floater_coordination();
        break;
    }
    case GLFW_KEY_F6: {
        wgpu::Queue q = device_.GetQueue();
        corral_cubes(q);
        break;
    }
    case GLFW_KEY_F7: {
        wgpu::Queue q = device_.GetQueue();
        toggle_cube_kite_mode(q);
        break;
    }
    case GLFW_KEY_LEFT_BRACKET:
        set_render_radius(activeRadius_ - 1);
        break;
    case GLFW_KEY_RIGHT_BRACKET:
        set_render_radius(activeRadius_ + 1);
        break;
    case GLFW_KEY_0: {
        wgpu::Queue q = device_.GetQueue();
        cycle_orb_palette(q);
        break;
    }
    }
    update_movement_intent();
}

void on_key_up(int key) {
    switch (key) {
    case GLFW_KEY_UP:    keys_.forward = false; break;
    case GLFW_KEY_DOWN:  keys_.backward = false; break;
    case GLFW_KEY_LEFT:  keys_.left = false; break;
    case GLFW_KEY_RIGHT: keys_.right = false; break;
    }
    update_movement_intent();
}

void on_mouse_move(float dx, float dy) {
    constexpr float sensitivity = 0.005f;
    if (mouse_.left_dragging) {
        inputState_.look_az_delta += dx * sensitivity;
        inputState_.look_el_delta += dy * sensitivity;
    }
    if (mouse_.right_dragging) {
        inputState_.pan_x_delta += dx * sensitivity;
        inputState_.pan_y_delta -= dy * sensitivity;
    }
}

void on_mouse_button(int button, bool pressed) {
    if (button == 0) mouse_.left_dragging = pressed;
    if (button == 1) mouse_.right_dragging = pressed;
}

void on_scroll(float delta) {
    inputState_.zoom_delta -= delta * 2.0f;
}

void update_movement_intent() {
    inputState_.move_x = 0.0f;
    inputState_.move_z = 0.0f;

    if (keys_.forward)  inputState_.move_z -= 1.0f;
    if (keys_.backward) inputState_.move_z += 1.0f;
    if (keys_.left)     inputState_.move_x -= 1.0f;
    if (keys_.right)    inputState_.move_x += 1.0f;

    float len = std::sqrt(inputState_.move_x * inputState_.move_x +
        inputState_.move_z * inputState_.move_z);
    if (len > 1.0f) {
        inputState_.move_x /= len;
        inputState_.move_z /= len;
    }
}

void clear_input_deltas() {
    inputState_.look_az_delta = 0.0f;
    inputState_.look_el_delta = 0.0f;
    inputState_.zoom_delta = 0.0f;
    inputState_.pan_x_delta = 0.0f;
    inputState_.pan_y_delta = 0.0f;
}

void toggle_fpv_mode() {
    fpvMode_ = !fpvMode_;
    gpuState_.set_fpv_mode(fpvMode_ ? 1 : 0);
    std::cout << "[the_board] Camera mode: "
        << (fpvMode_ ? "First-Person View" : "Orbit") << std::endl;
}

void set_render_radius(uint32_t r) {
    r = std::max(r, GRID_RADIUS);
    r = std::min(r, PREGEN_RADIUS);
    if (r == activeRadius_) return;
    activeRadius_ = r;
    uint32_t side = 2 * r + 1;
    std::cout << "[the_board] Render radius: " << r
        << " (" << side << "x" << side << " = " << side * side << " patches)" << std::endl;
    // Force full re-evaluation on next frame
    lastCenterX_ = INT32_MAX;
    lastCenterZ_ = INT32_MAX;
}
