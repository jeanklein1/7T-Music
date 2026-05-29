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
#include "imgui_internal.h"   // DockBuilder API (docking branch)
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include "implot.h"
#include <GLFW/glfw3.h>       // glfwMaximizeWindow (Console owns the GLFW window)

#include <algorithm>
#include <array>
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
// VECTOR HISTORY -- 2D rolling buffer for vector-stat heatmap strips
// =========================================================================
//
// A rolling window of the last `max_cols` frames, each a column of `bins`
// values. Stored column-major in a ring; unrolled oldest->newest into a
// contiguous row-major block at draw time for ImPlot::PlotHeatmap.

struct VectorHistory {
    int bins;
    int max_cols;
    int head = 0;
    int filled = 0;
    std::vector<float> ring;  // max_cols columns, each `bins` long (column-major)

    VectorHistory(int n_bins, int cols = 240)
        : bins(n_bins), max_cols(cols), ring(n_bins * cols, 0.0f) {}

    void push(const float* col) {
        for (int b = 0; b < bins; ++b) ring[head * bins + b] = col[b];
        head = (head + 1) % max_cols;
        if (filled < max_cols) ++filled;
    }

    // Unroll oldest->newest into row-major [bins][max_cols] for PlotHeatmap.
    void unroll(std::vector<float>& out) const {
        out.resize(bins * max_cols);
        for (int t = 0; t < max_cols; ++t) {
            int src = (head + t) % max_cols;  // oldest first
            for (int b = 0; b < bins; ++b)
                out[b * max_cols + t] = ring[src * bins + b];
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
    ImGui::SeparatorText("AnalysisSignal  (slots 0-15 of 128 per channel)");

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

// ─── Stat panels (descriptor-driven) ──────────────────────────────────────
// Renders each StatGroup in its declared shape: scalars as scrolling lines,
// vectors as bar charts. Iterates AnalysisCartridge::STAT_LAYOUT, so no slot
// numbers are hardcoded here — new stats appear by adding a STAT_LAYOUT row.

static void draw_scalar(const t7::StatGroup& g, const ScrollingBuffer& buf,
                        const t7::AnalysisSignal& signal) {
    const float now = signal.t_seconds;
    ImGui::Text("%.3f", signal.stat(g.channel, g.slot_base));

    char id[64];
    std::snprintf(id, sizeof(id), "##%s", g.name);
    if (ImPlot::BeginPlot(id, ImVec2(-1, 110))) {
        ImPlot::SetupAxes("t (s)", "value",
                          ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, now - 10.0, now, ImPlotCond_Always);
        if (!buf.xs.empty()) {
            ImPlotSpec spec;
            spec.Offset = buf.offset;
            ImPlot::PlotLine(g.name, buf.xs.data(), buf.ys.data(),
                             (int)buf.xs.size(), spec);
        }
        ImPlot::EndPlot();
    }
}

static void draw_vector(const t7::StatGroup& g, const t7::AnalysisSignal& signal) {
    float vals[128];
    for (int i = 0; i < g.count; ++i)
        vals[i] = signal.stat(g.channel, g.slot_base + i);

    char id[64];
    std::snprintf(id, sizeof(id), "##%s", g.name);
    if (ImPlot::BeginPlot(id, ImVec2(-1, 150))) {
        ImPlot::SetupAxes("slot", "value",
                          ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::PlotBars(g.name, vals, g.count);
        ImPlot::EndPlot();
    }
}

// Heatmap-strip: a vector stat's history as bin(Y) x time(X), value as color.
// Turns the instantaneous bar chart into a moving picture (the progression).
static void draw_vector_history(const t7::StatGroup& g, const VectorHistory& hist) {
    static std::vector<float> temp;  // reused scratch; dev tool, single-threaded UI
    hist.unroll(temp);

    float vmax = 0.0f;
    for (float v : temp) vmax = std::max(vmax, v);
    if (vmax < 1e-6f) vmax = 1.0f;  // empty strip: avoid a zero-width color range

    char id[64];
    std::snprintf(id, sizeof(id), "##%s_hist", g.name);

    ImPlot::PushColormap(ImPlotColormap_Viridis);
    if (ImPlot::BeginPlot(id, ImVec2(-1, 120))) {
        ImPlot::SetupAxes("t (older -> newer)", "bin",
                          ImPlotAxisFlags_NoDecorations,
                          ImPlotAxisFlags_NoDecorations);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, hist.max_cols, ImPlotCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, g.count,        ImPlotCond_Always);
        ImPlot::PlotHeatmap(g.name, temp.data(), g.count, hist.max_cols,
                            0.0, (double)vmax, nullptr,
                            ImPlotPoint(0, 0),
                            ImPlotPoint((double)hist.max_cols, (double)g.count));
        ImPlot::EndPlot();
    }
    ImPlot::PopColormap();
}

struct StatScopes {
    std::vector<ScrollingBuffer> scalar_history;
    std::vector<VectorHistory>   vector_history;

    // Display grouping: stats clustered per channel, reorderable. Reordering
    // affects DISPLAY only — histories stay keyed to declared order via
    // hist_index, so moving a stat never disturbs its scrolling/heatmap buffer.
    struct ChannelView {
        int  channel;
        char label[32];          // e.g. "abbott", parsed from the name prefix
        std::vector<int> order;  // STAT_LAYOUT indices, in display order
    };
    std::vector<ChannelView> channel_views;
    std::array<int, AnalysisCartridge::STAT_LAYOUT_COUNT> hist_index;  // group -> buffer slot

    StatScopes() {
        for (int g = 0; g < AnalysisCartridge::STAT_LAYOUT_COUNT; ++g) {
            const auto& grp = AnalysisCartridge::STAT_LAYOUT[g];

            // history buffers, in declared order; remember each group's slot
            if (grp.shape == t7::StatShape::Scalar) {
                hist_index[g] = (int)scalar_history.size();
                scalar_history.emplace_back();
            } else {
                hist_index[g] = (int)vector_history.size();
                vector_history.emplace_back(grp.count);
            }

            // find/create the channel view, append this group
            ChannelView* cv = nullptr;
            for (auto& c : channel_views)
                if (c.channel == grp.channel) { cv = &c; break; }
            if (!cv) {
                channel_views.push_back(ChannelView{});
                cv = &channel_views.back();
                cv->channel = grp.channel;
                // label = name up to '.' (manual parse, no <cstring>)
                int k = 0;
                while (grp.name[k] && grp.name[k] != '.' && k < 31) {
                    cv->label[k] = grp.name[k]; ++k;
                }
                cv->label[k] = '\0';
            }
            cv->order.push_back(g);
        }
    }

    void tick(const t7::AnalysisSignal& signal) {
        for (int g = 0; g < AnalysisCartridge::STAT_LAYOUT_COUNT; ++g) {
            const auto& grp = AnalysisCartridge::STAT_LAYOUT[g];
            if (grp.shape == t7::StatShape::Scalar) {
                scalar_history[hist_index[g]].push(
                    signal.t_seconds, signal.stat(grp.channel, grp.slot_base));
            } else {
                float col[128];
                for (int i = 0; i < grp.count; ++i)
                    col[i] = signal.stat(grp.channel, grp.slot_base + i);
                vector_history[hist_index[g]].push(col);
            }
        }
    }

    void draw(const t7::AnalysisSignal& signal) {
        for (auto& cv : channel_views) {
            if (!ImGui::CollapsingHeader(cv.label, ImGuiTreeNodeFlags_DefaultOpen))
                continue;
            ImGui::Indent();

            int swap_pos = -1, swap_with = -1;  // deferred (mutate after the loop)
            for (int pos = 0; pos < (int)cv.order.size(); ++pos) {
                const int g = cv.order[pos];
                const auto& grp = AnalysisCartridge::STAT_LAYOUT[g];

                // stat name without the channel prefix
                const char* dot = grp.name;
                while (*dot && *dot != '.') ++dot;
                const char* short_name = (*dot == '.') ? dot + 1 : grp.name;

                ImGui::PushID(g);
                if (ImGui::SmallButton("^") && pos > 0) {
                    swap_pos = pos; swap_with = pos - 1;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("v") && pos < (int)cv.order.size() - 1) {
                    swap_pos = pos; swap_with = pos + 1;
                }
                ImGui::SameLine();

                if (ImGui::CollapsingHeader(short_name)) {  // collapsed by default
                    if (grp.shape == t7::StatShape::Scalar) {
                        draw_scalar(grp, scalar_history[hist_index[g]], signal);
                    } else {
                        draw_vector(grp, signal);
                        draw_vector_history(grp, vector_history[hist_index[g]]);
                    }
                }
                ImGui::PopID();
            }

            if (swap_pos >= 0) {  // one reorder per frame, applied after iterating
                int tmp = cv.order[swap_pos];
                cv.order[swap_pos] = cv.order[swap_with];
                cv.order[swap_with] = tmp;
            }
            ImGui::Unindent();
        }
    }
};

static void draw_analysis_window(const t7::AnalysisSignal& signal,
                                 StatScopes& scopes,
                                 AnalysisCartridge& analysis) {
    // Layout is owned by the dockspace; no manual pos/size here.
    ImGui::Begin("Analysis", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("t = %.2f s   beat = %.2f   dt = %.4f s   |   analysis: %s",
                signal.t_seconds, signal.t_beats, signal.dt, ANALYSIS_NAME);
    ImGui::Separator();

    bool kbd = analysis.keyboard_enabled();
    if (ImGui::Checkbox("keyboard debug input (-> abbott)", &kbd))
        analysis.set_keyboard_enabled(kbd);

    scopes.draw(signal);

    if (ImGui::CollapsingHeader("raw slot grid (channels x slots 0-15)"))
        draw_stats_grid(signal);
    ImGui::End();
}

static void draw_coupling_window(const t7::AnalysisSignal& signal,
                                 TestCoupling& tc) {
    // Layout is owned by the dockspace; no manual pos/size here.
    ImGui::Begin("Coupling Lab", nullptr, ImGuiWindowFlags_NoCollapse);
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

    // Fit the screen on launch: the hardcoded 1400x900 can overflow a smaller
    // monitor (the bottom becoming unreachable). Maximize to the work area;
    // the dockspace below then lays the panels out to fill it.
    glfwMaximizeWindow(console.window());

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // dock within main window
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
    StatScopes stat_scopes;
    wgpu::Queue queue = console.queue();

    // --- Main loop ----------------------------------------------------------
    while (console.running()) {
        const float dt = console.begin_frame();

        // Music keys must reach the analyzer even when a plot/window has focus.
        // WantCaptureKeyboard is true whenever the mouse is over any widget, so
        // gating on it swallowed keys unless you clicked empty space. Only a
        // live text field should suppress music keys, so gate on WantTextInput
        // (the lab has no text fields yet, so keys always flow for now).
        for (const auto& event : console.input_events()) {
            if (event.type == t7::InputEvent::Type::KeyDown ||
                event.type == t7::InputEvent::Type::KeyUp) {
                if (!io.WantTextInput) {
                    analysis.on_input(event);
                }
            }
        }
        console.clear_input_events();

        // Analysis tick + coupling tick
        analysis.update(dt);
        test_coupling.tick(analysis.output());
        stat_scopes.tick(analysis.output());

        // ImGui frame
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Dock space over the main viewport (native drag-out / dock / tab /
        // split-resize). On first ever launch — before imgui.ini exists — build
        // a default split; afterwards imgui.ini wins.
        ImGuiID dock_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        static bool dock_inited = false;
        if (!dock_inited) {
            dock_inited = true;
            // DockSpaceOverViewport() creates the node immediately, so a fresh
            // launch yields an EMPTY node (not null). Only skip the default when
            // imgui.ini already restored a populated layout.
            ImGuiDockNode* node = ImGui::DockBuilderGetNode(dock_id);
            if (node == nullptr || node->IsEmpty()) {
                ImGui::DockBuilderRemoveNode(dock_id);
                ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dock_id,
                                              ImGui::GetMainViewport()->WorkSize);
                ImGuiID right;
                ImGuiID left = ImGui::DockBuilderSplitNode(
                    dock_id, ImGuiDir_Left, 0.60f, nullptr, &right);
                ImGui::DockBuilderDockWindow("Analysis", left);
                ImGui::DockBuilderDockWindow("Coupling Lab", right);
                ImGui::DockBuilderFinish(dock_id);
            }
        }

        draw_analysis_window(analysis.output(), stat_scopes, analysis);
        draw_coupling_window(analysis.output(), test_coupling);
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
