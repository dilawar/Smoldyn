//
//   Manage main windows.
//
//  Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
//

#include "window.h"

#include "../Smoldyn/smoldyn.h"
#include "../Smoldyn/smoldynfuncs.h"

#define COLOR_BLACK IM_COL32(0, 0, 0, 255)
#define COLOR_RED IM_COL32(255, 0, 0, 255)
#define COLOR_WHITE IM_COL32(255, 255, 255, 255)

namespace smoldyn {

//
// ImGUi callback.
//
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

Window::Window(const char* name)
    : name_(name)
    , arena_({ 480.0f, 480.f })
    , initialized_(false)
    , frame_rate_(20.0f)
    , canvas_to_arena_ratio_(1.5)
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
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    initialized_ = true;
    return 0;
}

int Window::simulate(simptr sim)
{
    int er;
    sim_ = sim;

    simLog(sim, 2, "Simulating\n");

    sim->clockstt = time(NULL);
    er = simdocommands(sim);

    if (!er) {

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(canvas_[0], canvas_[1]);
        io.DeltaTime = 1.0f / frame_rate_;

        unsigned char* tex_pixels = nullptr;
        int tex_w, tex_h;
        io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

        // background color = white
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));

        while ((er = simulatetimestep(sim)) == 0) {
            sim->elapsedtime += difftime(time(NULL), sim->clockstt);
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

int Window::render_molecules(ImDrawList* drawlist)
{
    auto dim = sim_->dim;
    auto mols = sim_->mols;

    if (!mols) {
        return 1;
    }

    // draw_limits();

    // if (1 == sim_->graphss->graphics)
    {
        for (auto ll = 0; ll < sim_->mols->nlist; ll++) {
            if (sim_->mols->listtype[ll] == MLTsystem) {
                for (auto m = 0; m < mols->nl[ll]; m++) {
                    auto mptr = mols->live[ll][m];
                    auto i = mptr->ident;
                    auto ms = mptr->mstate;

                    if (mols->display[i][ms] > 0) {

                        const auto pos = _GetPos(mptr->pos, dim);
                        const auto size = scalexy(mols->display[i][ms]);
                        const auto clr = _GetColor(mols->color[i][ms]);

                        // printf("pos (%f,%f), clr=%d, size=%f\n", pos.x,
                        // pos.y, clr, size);
                        drawlist->AddCircleFilled(pos, size, clr, 32.f);
                    }
                }
            }
        }
    }

    return 0;
}

int Window::render_scene()
{
    static int counter = 0;
    counter += 1; // number of frames.

    auto graphss = sim_->graphss;
    if (!graphss || graphss->graphics == 0) {
        fprintf(stderr, "No graphics element found: %d.\n", graphss->graphics);
        return 1;
    }

    auto dim = sim_->dim;

    double pt1[DIMMAX], pt2[DIMMAX];
    float glf1[4];

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize({ canvas_[0], canvas_[1] });
    ImGui::SetNextWindowPos({ 0, 0 }, 0);
    ImGui::Begin(name_);

    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    {
        ImGui::TextColored({ 0, 0, 0, 1 }, "Frame=%d, Time=%06.2fs.", counter,
            sim_->elapsedtime);

        // Render the scene now.
        if (3 == dim)
            render_molecules(drawlist);

        // draw bounding box.
        if (graphss->framepts) {
            // draw bounding box
            const auto wlist = sim_->wlist;
            pt1[0] = wlist[0]->pos;
            pt2[0] = wlist[1]->pos;
            pt1[1] = dim > 1 ? wlist[2]->pos : 0;
            pt2[1] = dim > 1 ? wlist[3]->pos : 0;
            pt1[2] = dim > 2 ? wlist[4]->pos : 0;
            pt2[2] = dim > 2 ? wlist[5]->pos : 0;

            // glColor4fv(gl2Double2GLfloat(graphss->framecolor, glf1, 4));
            // glLineWidth((GLfloat)graphss->framepts);
            // gl2DrawBoxD(pt1, pt2, dim);
            for (size_t i = 0; i < dim - 1; ++i) {

                printf(" %f %f -> %f %f\n", pt1[i], pt1[i+1], pt2[i],
                    pt2[i + 1]);

                drawlist->AddLine(
                    { pt1[i], pt2[i] }, { pt1[i + 1], pt2[i + 1] }, COLOR_RED);
            }
        }
    }

    ImGui::End(); // End of window.

    ImGui::Render();
#if 0
    // Not sure if these are neccessary.
    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
        clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);

    return 0;
}

} // namespace smoldyn

