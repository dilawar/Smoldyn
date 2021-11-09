//
//   Manage main windows.
//
//  Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
//

#include "window.h"
#include "ImGuizmo.h"

#include "../Smoldyn/smoldyn.h"
#include "../Smoldyn/smoldynfuncs.h"

#include <fmt/ranges.h>

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
    , arena_({ 480.f, 480.f, 480.f })
    , scale_({ 4.0f, 4.0f, 4.0f })
    , canvas_to_arena_ratio_(1.5f)
{
    update_canvas_size();
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

void Window::update_canvas_size()
{
    canvas_[0] = canvas_to_arena_ratio_ * arena_[0];
    canvas_[1] = canvas_to_arena_ratio_ * arena_[1];
}

void Window::set_arena(float width, float height)
{
    arena_[0] = width;
    arena_[1] = height;
    update_canvas_size();
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
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    //
    // Create windows
    //
    window_ = glfwCreateWindow(
        canvas_[0], canvas_[1], "Smoldyn Window", NULL, NULL);

    if (window_ == nullptr) {
        fprintf(stderr, "Failed to create main windows.\n");
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
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    initialized_ = true;
    return 0;
}

int Window::simulate(simptr sim)
{
    int er;
    sim_.reset(sim);

    simLog(sim, 2, "Simulating\n");

    sim_->clockstt = time(NULL);
    er = simdocommands(sim);

    if (!er) {

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(canvas_[0], canvas_[1]);
        io.DeltaTime = 1.0f / frame_rate_;

        ImGui::GetStyle().ScaleAllSizes(2.f);

        unsigned char* tex_pixels = nullptr;
        int tex_w, tex_h;
        io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

        size_t nFrame = 0;
        while ((er = simulatetimestep(sim)) == 0) {
            nFrame += 1;
            sim_->elapsedtime += difftime(time(NULL), sim->clockstt);
            render_scene();
        }
    }
    return er;
}

void Window::draw_limits()
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float pos[2] = { 0, 0 };
    draw_list->AddCircleFilled(_GetPos(pos, 2), 10.f, COLOR_BLACK, 16.f);

    pos[0] = -0.5;
    pos[1] = -0.5;
    draw_list->AddCircleFilled(_GetPos(pos, 2), 10.f, COLOR_RED, 16.f);

    pos[0] = 0.5;
    pos[1] = -0.5;
    draw_list->AddCircleFilled(_GetPos(pos, 2), 10.f, COLOR_RED, 16.f);

    pos[0] = 0.5;
    pos[1] = 0.5;
    draw_list->AddCircleFilled(_GetPos(pos, 2), 10.f, COLOR_RED, 16.f);

    pos[0] = -0.5;
    pos[1] = 0.5;
    draw_list->AddCircleFilled(_GetPos(pos, 2), 10.f, COLOR_RED, 16.f);
}

int Window::render_molecules()
{
    auto dim = sim_->dim;

    if (!sim_->mols) {
        return 1;
    }

    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    for (auto ll = 0; ll < sim_->mols->nlist; ll++) {
        if (sim_->mols->listtype[ll] == MLTsystem) {
            for (auto m = 0; m < sim_->mols->nl[ll]; m++) {
                auto mptr = sim_->mols->live[ll][m];
                auto i = mptr->ident;
                auto ms = mptr->mstate;

                if (sim_->mols->display[i][ms] > 0) {

                    const auto pos = PointOnCanvas(mptr->pos);
                    const auto size = scalexy(sim_->mols->display[i][ms]);
                    const auto clr = ArrToColor(sim_->mols->color[i][ms]);

#if USE_FMT
#if 0
                    fmt::print("pos ({},{}), clr={}, size={}.\n", pos.x, pos.y,
                        clr, size);
#endif
#endif
                    drawlist->AddCircleFilled(
                        pos, size /* radius */, clr, NUM_SEGMENT_IN_CIRCLE);
                }
            }
        }
    }

    return 0;
}

int Window::render_scene()
{
    bool render = false;

    static int counter = 0;
    static int nthframe = 0;
    counter += 1;  // number of frames.
    nthframe += 1; // count the frames to be skipped.

    auto graphss = sim_->graphss;
    if (!graphss || graphss->graphics == 0) {
#if USE_FMT
        fmt::print(
            stderr, "No graphics element found: {}.\n", graphss->graphics);
#endif
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

    auto dim = sim_->dim;
    float glf1[4];

    //
    // Set background color and text color.
    //
    ImGui::PushStyleColor(
        ImGuiCol_WindowBg, ArrToColor(sim_->graphss->backcolor));

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::SetOrthoGraphic(true);
    ImGuizmo::BeginFrame();
    // ImGuizmo::DrawCubes();

    ImGui::SetNextWindowSize({ canvas_[0], canvas_[1] });
    ImGui::SetNextWindowPos({ 0, 0 }, 0);
    ImGui::Begin(name_);

    //
    // Drawing starts.
    //

    ImGui::TextColored(ArrToColorVec(sim_->graphss->backcolor, true),
        fmt::format("Frame={}, Time={}s.", counter, sim_->elapsedtime).c_str());

    // Render molecules.
    render_molecules();

    // draw bounding box.
    if (graphss->framepts) {
        std::array<float, DIMMAX> pt1, pt2;
        const auto wlist = sim_->wlist;
        pt1[0] = wlist[0]->pos;
        pt2[0] = wlist[1]->pos;
        pt1[1] = dim > 1 ? wlist[2]->pos : 0;
        pt2[1] = dim > 1 ? wlist[3]->pos : 0;
        pt1[2] = dim > 2 ? wlist[4]->pos : 0;
        pt2[2] = dim > 2 ? wlist[5]->pos : 0;
        draw_box_d(pt1, pt2);
    }

    // draw grid.
    if (graphss->gridpts)
        render_grid();

    ImGui::End();

    // Render now.
    ImGui::Render();

    //
    // Clean the window.
    //
    static ImVec4 resetclr = ImVec4(1.f, 1.f, 1.f, 1.00f);
    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(resetclr.x * resetclr.w, resetclr.y * resetclr.w,
        resetclr.z * resetclr.w, resetclr.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);

    return 0;
}

void Window::draw_box_d(
    const std::array<float, DIMMAX>& pt1, const std::array<float, DIMMAX>& pt2)
{
    const auto dim = sim_->dim;
    const auto clr = ArrToColor(sim_->graphss->framecolor);
    const auto width = float(sim_->graphss->framepts);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (dim == 1) {
        ImVec2 p1 { pt1[0], pt1[1] };
        ImVec2 p2 { pt2[0], pt1[1] };
        drawList->AddLine(p1, p2, clr, width);
        return;
    }

    if (dim == 2) {
        auto v1 = toImVec2(pt1.data(), dim);
        auto v2 = toImVec2(pt2.data(), dim);
        drawList->AddRect(v1, v2, clr, width);
        return;
    }

    fmt::print(stderr, "Not implemented yet.");

#if 0
    glVertex3d(pt1[0], pt1[1], pt1[2]);
    glVertex3d(pt1[0], pt1[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt1[2]);
    glVertex3d(pt1[0], pt1[1], pt1[2]);
    glVertex3d(pt2[0], pt1[1], pt1[2]);
    glVertex3d(pt2[0], pt2[1], pt1[2]);
    glVertex3d(pt2[0], pt2[1], pt2[2]);
    glVertex3d(pt2[0], pt1[1], pt2[2]);
    glVertex3d(pt2[0], pt1[1], pt1[2]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3d(pt1[0], pt1[1], pt2[2]);
    glVertex3d(pt2[0], pt1[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt2[2]);
    glVertex3d(pt2[0], pt2[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt1[2]);
    glVertex3d(pt2[0], pt2[1], pt1[2]);
    glEnd();
#endif
    return;
}

int Window::render_grid()
{
    const auto dim = sim_->dim;
    std::array<float, DIMMAX> pt1 = { 0 }, pt2 = { 0 };

    pt1[0] = sim_->boxs->min[0];
    pt2[0] = pt1[0] + sim_->boxs->size[0] * sim_->boxs->side[0];
    pt1[1] = dim > 1 ? sim_->boxs->min[1] : 0;
    pt2[1] = dim > 1 ? pt1[1] + sim_->boxs->size[1] * sim_->boxs->side[1] : 0;
    pt1[2] = dim > 2 ? sim_->boxs->min[2] : 0;
    pt2[2] = dim > 2 ? pt1[2] + sim_->boxs->size[2] * sim_->boxs->side[2] : 0;

    const auto clr = ArrToColor(sim_->graphss->gridcolor);
    const float size = float(sim_->graphss->gridpts);

    const auto n = sim_->boxs->side;
    float delta1 = 0.0f;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    if (dim == 1) {
        delta1 = (pt2[0] - pt1[0]) / n[0];
        for (int i = 0; i <= n[0]; i++)
            drawList->AddCircleFilled(
                PointOnCanvas(pt1[0] + i * delta1, pt1[1], pt1[2]), size, clr,
                NUM_SEGMENT_IN_CIRCLE);
        return 0;
    }

    if (dim == 2) {
        delta1 = (pt2[1] - pt1[1]) / n[1];
        for (int i = 0; i <= n[1]; i++) {
            const auto p1 = PointOnCanvas(pt1[0], pt1[1] + i * delta1, pt1[2]);
            const auto p2 = PointOnCanvas(pt2[0], pt1[1] + i * delta1, pt1[2]);
            drawList->AddLine(p1, p2, clr, size);
        }
        delta1 = (pt2[0] - pt1[0]) / n[0];
        for (int i = 0; i <= n[0]; i++) {
            drawList->AddLine(
                PointOnCanvas(pt1[0] + i * delta1, pt1[1], pt1[2]),
                PointOnCanvas(pt1[0] + i * delta1, pt2[1], pt1[2]), clr, size);
        }
        return 0;
    }

#if 0
    if (dim == 3) {
#endif
    fmt::print(stderr, "Not supported.");
    return 1;
}

} // namespace smoldyn

