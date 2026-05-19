/**
 * THE LAB -- Coupling Laboratory
 * ==============================
 *
 * Fast-iteration sandbox for designing musical couplings. Pairs the analysis
 * cartridge of choice with a tiny ImGui+ImPlot dashboard so we can see a
 * parameter move (raw stat, normalized target, and trajectory output) in
 * sub-second iteration loops.
 *
 * No render cartridge. No project WGSL pipelines. FXC compile time is
 * trivial -- only ImGui's internal render pipeline gets compiled.
 *
 * Selection of the analysis cartridge is controlled from CMakeLists.txt
 * via ACTIVE_ANALYSIS_CARTRIDGE (passed as LAB_ANALYSIS).
 *
 * Build:  cmake --build . --target the_lab
 * Run:    ./the_lab.exe [optional_midi_file]
 *
 * CONVENTION
 *   | Folder name        | Namespace            | Class  |
 *   |--------------------|----------------------|--------|
 *   | polyphony_basic/   | t7::polyphony_basic  | Canvas |
 */

// =========================================================================
// TARGET SELECTION -- Provided by CMake, with fallback for manual override
// =========================================================================

#ifndef LAB_ANALYSIS
#define LAB_ANALYSIS polyphony_basic
#endif

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define ANALYSIS_HEADER(name) STRINGIFY(analysis/name/canvas.hpp)

// =========================================================================
// INCLUDES
// =========================================================================

#include "console/console.hpp"
#include "musical/trajectory.hpp"

// IntelliSense cannot resolve macro-expanded #include paths.
// Literal include gives navigation; the macro include below pulls the same file.
#if defined(__INTELLISENSE__)
#include "analysis/polyphony_basic/canvas.hpp"
#else
#include ANALYSIS_HEADER(LAB_ANALYSIS)
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include "implot.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

// =========================================================================
// ACTIVE CARTRIDGE TYPE -- Derived from define
// =========================================================================

namespace analysis_ns = t7::LAB_ANALYSIS;
using AnalysisCartridge = analysis_ns::Canvas;

constexpr const char* ANALYSIS_NAME = STRINGIFY(LAB_ANALYSIS);

// =========================================================================
// SCROLLING BUFFER -- Ring buffer for ImPlot time series
// =========================================================================

struct ScrollingBuffer {
    int max_size;
    int offset;
    std::vector<float> xs;
    std::vector<float> ys;

    explicit ScrollingBuffer(int max = 4000) : max_size(max), offset(0) {
        xs.reserve(max);
        ys.reserve(max);
    }

    void push(float x, float y) {
        if ((int)xs.size() < max_size) {
            xs.push_back(x);
            ys.push_back(y);
        } else {
            xs[offset] = x;
            ys[offset] = y;
            offset = (offset + 1) % max_size;
        }
    }
};

// =========================================================================
// TEST COUPLING -- One coupling under design; intensity idiom
// =========================================================================
//
// Reads one stat from the AnalysisSignal, normalizes by FULL, ramps a
// Trajectory toward the normalized target with asymmetric attack/release.
// This is the simplest coupling shape (the most common one in musical.inl);
// future iterations of the lab can host rate and event idioms alongside.

struct TestCoupling {
    bool  enabled        = true;
    int   source_channel = 0;     // which AnalysisSignal channel to read
    int   source_slot    = 0;     // which stat within that channel
    float attack         = 4.0f;  // 1/s
    float release        = 2.5f;  // 1/s
    float full           = 6.0f;  // input value that saturates target at 1.0

    t7::Trajectory trajectory{};

    ScrollingBuffer input_history;   // raw stat over time
    ScrollingBuffer target_history;  // normalized target over time
    ScrollingBuffer output_history;  // trajectory.value over time

    void tick(const t7::AnalysisSignal& signal) {
        const float input = signal.stat(source_channel, source_slot);
        const float target = enabled ? std::min(input / full, 1.0f) : 0.0f;
        const float rate = (target > trajectory.value) ? attack : release;
        trajectory = t7::trajectory_release(trajectory, target, signal.dt, rate);

        input_history.push(signal.t_seconds, input);
        target_history.push(signal.t_seconds, target);
        output_history.push(signal.t_seconds, trajectory.value);
    }
};

// =========================================================================
// DASHBOARD -- The whole window per frame
// =========================================================================

static void draw_stats_grid(const t7::AnalysisSignal& signal) {
    ImGui::SeparatorText("AnalysisSignal  (4 channels x 16 slots)");

    if (ImGui::BeginTable("stats", 17,
            ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("ch");
        for (int s = 0; s < 16; ++s) {
            char header[8];
            std::snprintf(header, sizeof(header), "%d", s);
            ImGui::TableSetupColumn(header);
        }
        ImGui::TableHeadersRow();

        for (int c = 0; c < 4; ++c) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("ch%d", c);
            for (int s = 0; s < 16; ++s) {
                ImGui::TableSetColumnIndex(s + 1);
                const float v = signal.stat(c, s);
                if (std::abs(v) > 0.001f) {
                    ImGui::Text("%.2f", v);
                } else {
                    ImGui::TextDisabled(" - ");
                }
            }
        }
        ImGui::EndTable();
    }
}

static void draw_coupling_controls(TestCoupling& tc) {
    ImGui::SeparatorText("Test Coupling  (intensity idiom)");

    ImGui::Checkbox("Enabled", &tc.enabled);
    ImGui::SameLine();
    ImGui::Text("  source: stat(ch=%d, slot=%d)", tc.source_channel, tc.source_slot);

    ImGui::SliderInt("Source channel", &tc.source_channel, 0, 3);
    ImGui::SliderInt("Source slot",    &tc.source_slot,    0, 15);

    ImGui::SliderFloat("Attack rate (1/s)",   &tc.attack,  0.1f, 20.0f, "%.2f");
    ImGui::SliderFloat("Release rate (1/s)",  &tc.release, 0.1f, 20.0f, "%.2f");
    ImGui::SliderFloat("Normalization (FULL)", &tc.full,   1.0f, 16.0f, "%.2f");
}

static void draw_input_scope(const TestCoupling& tc, float now) {
    ImGui::SeparatorText("Input scope  (raw stat over time)");

    if (ImPlot::BeginPlot("##input_scope", ImVec2(-1, 200))) {
        ImPlot::SetupAxes("t (s)", "value", ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, now - 10.0, now, ImPlotCond_Always);

        if (!tc.input_history.xs.empty()) {
            ImPlotSpec spec;
            spec.Offset = tc.input_history.offset;
            ImPlot::PlotLine("input",
                tc.input_history.xs.data(), tc.input_history.ys.data(),
                (int)tc.input_history.xs.size(), spec);
        }
        ImPlot::EndPlot();
    }
}

static void draw_trajectory_scope(const TestCoupling& tc, float now) {
    ImGui::SeparatorText("Trajectory scope  (target vs output, both in [0,1])");

    if (ImPlot::BeginPlot("##trajectory_scope", ImVec2(-1, 240))) {
        ImPlot::SetupAxes("t (s)", "value");
        ImPlot::SetupAxisLimits(ImAxis_X1, now - 10.0, now, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.1, 1.2);

        if (!tc.target_history.xs.empty()) {
            ImPlotSpec spec;
            spec.Offset = tc.target_history.offset;
            ImPlot::PlotLine("target (normalized)",
                tc.target_history.xs.data(), tc.target_history.ys.data(),
                (int)tc.target_history.xs.size(), spec);
        }
        if (!tc.output_history.xs.empty()) {
            ImPlotSpec spec;
            spec.Offset = tc.output_history.offset;
            ImPlot::PlotLine("trajectory.value",
                tc.output_history.xs.data(), tc.output_history.ys.data(),
                (int)tc.output_history.xs.size(), spec);
        }
        ImPlot::EndPlot();
    }
}

static void draw_dashboard(const t7::AnalysisSignal& signal, TestCoupling& tc) {
    ImGui::Begin("The Lab -- Coupling Laboratory", nullptr,
                 ImGuiWindowFlags_NoCollapse);

    ImGui::Text("t = %.2f s   beat = %.2f   dt = %.4f s   |   analysis: %s",
                signal.t_seconds, signal.t_beats, signal.dt, ANALYSIS_NAME);
    ImGui::Separator();

    draw_stats_grid(signal);
    draw_coupling_controls(tc);
    draw_input_scope(tc, signal.t_seconds);
    draw_trajectory_scope(tc, signal.t_seconds);

    ImGui::End();
}

// =========================================================================
// MAIN
// =========================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  THE LAB -- Coupling Laboratory\n";
    std::cout << "  Analysis: " << ANALYSIS_NAME << "\n";
    std::cout << "========================================\n\n";

    // --- Console (window, GPU device, surface, input) -----------------------
    t7::Console console;
    if (!console.init("The Lab", 1400, 900)) {
        std::cerr << "Failed to initialize console\n";
        return 1;
    }

    // --- Analysis cartridge -------------------------------------------------
    AnalysisCartridge analysis;
    analysis.initialize("assets");

    if (argc > 1) {
        if (analysis.load_midi(argv[1])) {
            std::cout << "[Lab] Loaded MIDI: " << argv[1] << "\n";
        }
    }

    std::cout << "[Lab] " << ANALYSIS_NAME << " analysis ready\n";

    // --- ImGui + ImPlot -----------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Bind ImGui to Console's GLFW window
    ImGui_ImplGlfw_InitForOther(console.window(), /*install_callbacks=*/true);

    // Bind ImGui to Console's WebGPU device
    ImGui_ImplWGPU_InitInfo wgpu_init{};
    wgpu_init.Device              = console.device().Get();
    wgpu_init.NumFramesInFlight   = 3;
    wgpu_init.RenderTargetFormat  = static_cast<WGPUTextureFormat>(console.color_format());
    wgpu_init.DepthStencilFormat  = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&wgpu_init);

    std::cout << "[Lab] ImGui + WebGPU backend ready\n\n";

    // --- Lab state ----------------------------------------------------------
    TestCoupling test_coupling;
    wgpu::Queue queue = console.queue();

    // --- Main loop ----------------------------------------------------------
    while (console.running()) {
        const float dt = console.begin_frame();

        // Route music keys to the analysis cartridge only when ImGui doesn't
        // want the keyboard (e.g., user typing into an InputText widget).
        for (const auto& event : console.input_events()) {
            if (event.type == t7::InputEvent::Type::KeyDown ||
                event.type == t7::InputEvent::Type::KeyUp) {
                if (!io.WantCaptureKeyboard) {
                    analysis.on_input(event);
                }
            }
        }
        console.clear_input_events();

        // Analysis tick + coupling tick
        analysis.update(dt);
        test_coupling.tick(analysis.output());

        // ImGui frame
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_dashboard(analysis.output(), test_coupling);
        ImGui::Render();

        // Render the ImGui draw data via a single render pass
        if (!console.acquire_surface_texture()) {
            continue;
        }

        wgpu::CommandEncoderDescriptor enc_desc{};
        wgpu::CommandEncoder encoder = console.device().CreateCommandEncoder(&enc_desc);

        wgpu::RenderPassColorAttachment color_att{};
        color_att.view       = console.backbuffer();
        color_att.loadOp     = wgpu::LoadOp::Clear;
        color_att.storeOp    = wgpu::StoreOp::Store;
        color_att.clearValue = { 0.07, 0.07, 0.09, 1.0 };

        wgpu::RenderPassDescriptor pass_desc{};
        pass_desc.colorAttachmentCount = 1;
        pass_desc.colorAttachments     = &color_att;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&pass_desc);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass.Get());
        pass.End();

        wgpu::CommandBufferDescriptor cmd_desc{};
        wgpu::CommandBuffer cmds = encoder.Finish(&cmd_desc);
        queue.Submit(1, &cmds);

        console.present();
    }

    // --- Shutdown -----------------------------------------------------------
    ImGui_ImplWGPU_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    std::cout << "[Lab] Shutdown\n";
    return 0;
}
