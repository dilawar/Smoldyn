//
//   Manage main windows.
//
//  Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
//

#include "../Smoldyn/smoldyn.h"
#include "../Smoldyn/smoldynfuncs.h"

#include "helper.hpp"
#include "graphics.h"
#include "window.hpp"

#include <vector>

namespace smoldyn {

/**
 * Approximate a circle with this many linear segments.
 */
constexpr float NUM_SEGMENT_IN_CIRCLE = 16.0f;

//
// ImGUi callback.
//
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

Window::Window(const char* name)
    : name_(name)
    , initialized_(false)
    , frame_rate_(20.0f)
    , canvas_({ 720.f, 480.f, 480.f })
{
}

Window::~Window()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window_) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

int Window::init()
{
    if (initialized_) {
        fprintf(stderr, "Already initialized.");
        return 1;
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW windows.");
        return 1;
    }

    //
    // Initialize ImGui
    //
#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    //
    // Create windows
    //
    window_ = glfwCreateWindow(canvas_[0], canvas_[1],
        fmt::format("Smoldyn {}", VERSION).c_str(), NULL, NULL);

    if (window_ == nullptr) {
        fprintf(stderr, "Failed to create main window.\n");
        initialized_ = false;
        return 1;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

#ifdef IMGUI_IMPL_OPENGL_LOADER_GL3W
    bool err = glewInit() != GLEW_OK;
#else
    bool err = false;
#endif

    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loaded!\n");
        initialized_ = false;
        return 1;
    }

    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    glfwSwapInterval(1);

    ImGuiIO& io = ImGui::GetIO();

    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.DisplaySize = ImVec2(canvas_[0], canvas_[1]);
    io.DeltaTime = 1.0f / frame_rate_;

    initialized_ = true;
    return 0;
}

int Window::graphicsUpdate()
{
    //
    // Initialize GraphicsParam from sim
    //
    assert(sim_);
    assert(sim_->graphss);
    auto graphss = sim_->graphss;

    graphss->condition = SCparams;

    gui::GraphicsUpdate(sim_.get());
}

int Window::simulate(simptr sim)
{
    int er;
    sim_.reset(sim);

    simLog(sim, 2, "Simulating\n");

    sim_->clockstt = time(NULL);
    er = simdocommands(sim);

    if (!er) {
        size_t nFrame = 0;
        while ((er = simulatetimestep(sim)) == 0) {
            graphicsUpdate();
            nFrame += 1;
            sim_->elapsedtime += difftime(time(NULL), sim->clockstt);
            renderScene();
        }
    }
    return er;
}

void Window::updateCanvasSize()
{
    ImGuiIO& io = ImGui::GetIO();
    canvas_[0] = io.DisplaySize.x;
    canvas_[1] = io.DisplaySize.y;
}

size_t Window::getWidth() { return ImGui::GetIO().DisplaySize.x; }

size_t Window::getHeight() { return ImGui::GetIO().DisplaySize.y; }

int Window::renderScene()
{
    bool render = false;

    static int counter = 0;
    static int nthframe = 0;
    counter += 1;  // number of frames.
    nthframe += 1; // count the frames to be skipped.

    auto graphss = sim_->graphss;
    if (!graphss || graphss->graphics == 0) {
        return 1;
    }

    //
    // render only every graphss->graphssit frame.
    //
    if (nthframe == graphss->graphicit) {
        nthframe = 0;
        render = true;
    }

    if (!render)
        return -1;

    // Update canvas size.
    updateCanvasSize();

    auto dim = sim_->dim;

    // Poll events.
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //
    // Drawing starts.
    //
    ImGui::SetNextWindowPos({ canvas_[0], 0 }, 0, { 1.f, 0 });
    ImGui::SetNextWindowSize({ 0.f, canvas_[1] });

    ImGui::Begin("Menu");
    ImGui::TextColored(ArrToColorVec(sim_->graphss->backcolor, true),
        _format("Frame={}, Time={}s.", counter, sim_->elapsedtime).c_str());

    ImGui::SliderInt("Zoom", &gui::gGraphicsParam_.Zoom, 1, 10);
    ImGui::SliderFloat("FoV", &gui::gGraphicsParam_.FieldOfView, -180.f, 180.f);

    ImGui::Separator();
    ImGui::ColorEdit4("Bg Color", sim_->graphss->backcolor);
    ImGui::ColorEdit4("Frame Color", sim_->graphss->framecolor);
    ImGui::ColorEdit4("Text Color", sim_->graphss->textcolor);
    ImGui::ColorEdit4("Grid Color", sim_->graphss->gridcolor);

    gui::gGraphicsParam_.CanvasOffsetX = 10 + ImGui::GetWindowWidth();

    ImGui::Separator();

#if 0
    ImGui::Separator();
    ImGui::LabelText("ClipSize", "%f", gui::gGraphicsParam_.ClipSize);
    ImGui::LabelText("ClipMid", "%5.2f,%5.2f,%5.2f",
        gui::gGraphicsParam_.ClipMidx, gui::gGraphicsParam_.ClipMidy,
        gui::gGraphicsParam_.ClipMidz);
    ImGui::LabelText("Clip", "%5.2f,%5.2f\n%5.2f,%5.2f\n%5.2f,%5.2f",
        gui::gGraphicsParam_.ClipLeft, gui::gGraphicsParam_.ClipRight,
        gui::gGraphicsParam_.ClipTop, gui::gGraphicsParam_.ClipBot,
        gui::gGraphicsParam_.ClipFront, gui::gGraphicsParam_.ClipBack);
#endif

    ImGui::End();

    // Render the simulation.
    gui::RenderSim(sim_.get(), nullptr);

    //
    // Render the window
    //
    ImGui::Render();

    //
    // Clean the window.
    //
    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);

    return 0;
}

} // namespace smoldyn
