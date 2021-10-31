//
//   Manage main windows.
//
//  Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
//

#include <cstdio>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//
// Various ways to load OpenGL. We are using GLEW as default.
//
#define IMGUI_IMPL_OPENGL_LOADER_GLEW

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h> // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity or
                          // multiple definition errors.
#include <glbinding/Binding.h> // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity or
                          // multiple definition errors.
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#include <GLFW/glfw3.h>

#include "../Smoldyn/smoldyn.h"
#include "../Smoldyn/smoldynfuncs.h"

#include "window.h"

namespace smoldyn {

//
// ImGUi callback.
//
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

inline ImVec4 _GetColorVec(float* clr)
{
    return ImVec4 { clr[0], clr[1], clr[2], clr[3] };
}

inline ImU32 _GetColor(float* clr)
{
    return ImGui::GetColorU32(_GetColorVec(clr));
}

inline ImVec2 _GetPos(float* pos, size_t dim)
{
    if (dim == 1)
        return ImVec2(pos[0], 0);
    if (dim == 2)
        return ImVec2(pos[0], pos[1]);
    if (dim == 3)
        return ImVec2(pos[0], pos[1]);
    fprintf(stderr, "dim=%d is not supported.\n", dim);
}

Window::Window(const char* name)
    : name_(name)
    , width_(720)
    , height_(720)
    , initialized_(false)
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

    //
    // Create windows
    //
    assert(width_ > 30);
    assert(height_ > 30);

    window_ = glfwCreateWindow(width_, height_, name_, NULL, NULL);
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

    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard
    // Controls

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
    sim_ = sim;

    simLog(sim, 2, "Simulating\n");

    sim->clockstt = time(NULL);
    er = simdocommands(sim);
    if (!er) {

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width_, height_);
        // io.DeltaTime = 1.0f / 60.0f;

        // unsigned char* tex_pixels = nullptr;
        // int tex_w, tex_h;
        // io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);

        while ((er = simulatetimestep(sim)) == 0) {
            render();
            fprintf(stderr, ">> Simulation step done...\n");
        }
    }
    sim->elapsedtime += difftime(time(NULL), sim->clockstt);
    return er;
}

int Window::render_molecules()
{
    moleculeptr mptr;
    enum MolecState ms;

    auto dim = sim_->dim;
    auto mols = sim_->mols;

    if (!mols)
        return 1;

    // auto midp = GetCenter();
    auto drawlist = ImGui::GetWindowDrawList();

    if (1 == sim_->graphss->graphics) {
        for (auto ll = 0; ll < sim_->mols->nlist; ll++) {
            if (sim_->mols->listtype[ll] == MLTsystem) {
                for (auto m = 0; m < mols->nl[ll]; m++) {
                    mptr = mols->live[ll][m];
                    auto i = mptr->ident;
                    ms = mptr->mstate;

                    if (mols->display[i][ms] > 0) {
                        drawlist->AddCircleFilled(
                            _GetPos((float*)mptr->pos, dim),
                            mols->display[i][ms],
                            _GetColor((float*)mols->color[i][ms]));
                    }
                }
            }
        }
    }
    return 0;
}

int Window::render()
{
    auto graphss = sim_->graphss;
    if (!graphss || graphss->graphics == 0) {
        fprintf(stderr, "No graphics element found: %d.\n", graphss->graphics);
        return 1;
    }

    auto dim = sim_->dim;

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {

        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                       // and append into it.

        ImGui::Text("This is some useful text."); // Display some text (you can
                                                  // use a format strings too)

        ImGui::SliderFloat("float", &f, 0.0f,
            1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color",
            (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button(
                "Button")) // Buttons return true when clicked (most widgets
                           // return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    assert(window_);

    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
        clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);

    return 0;
}

} // namespace smoldyn

