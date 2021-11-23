//
//  Smoldyn GUI.
//   Author:  Dilawar Singh (), dilawar@subcom.tech
//

#ifndef WINDOW_H_VQ6821WA
#define WINDOW_H_VQ6821WA

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
#include <array>
#include <memory>

#ifdef USE_FMT
#include <fmt/core.h>
#endif

#include "smoldyn.h"

namespace smoldyn {

class Window {

public:
    Window(simptr sim);
    ~Window();

    inline GLFWwindow* ref() const { return window_; }

    int graphicsUpdate();
    int renderScene();

    bool isOpenGLEnabled() const;

    int initGLFW();

    void changeSize();
    void renderSim();

    static size_t getWidth();
    static size_t getHeight();

    void updateCanvasSize();

    int writeTIFF(const char* filename, const char* desc, int compression);

    /**
     * Simulate
     */
    int simulate();

    /**
     * Get the graphics type. Scaling functions uses this information.
     */
    int getGraphicsType() const { return sim_->graphss->graphics; }

    /**
     * If the boundary of simuilation are between -1 and 1, we call the arena
     * normalized. In this case, we need to map (-1, 1) to (H, W).
     */
    inline bool isArenaNormalized() const { return false; }

    /**
     * Rotate
     */
    void rotate();

protected:
    simstruct* sim_;
    bool paused_;

    char snapshotName_[120] = "OpenGL00000.tif";
    size_t numSnapshots_ = 0;

    std::array<float, 3> angles_ = { 0.f };
    std::array<float, 3> old_angles_ = { 0.f };

    // camera view
    std::array<float, 16> matrix_;

private:
    GLFWwindow* window_;
    int error_code_;

    bool initialized_;
    float frame_rate_;

    // Size of canvas. Usually 2x of arena_
    std::array<float, DIMMAX> canvas_;
    size_t nframe_ = 0;
};

}

#endif /* end of include guard: WINDOW_H_VQ6821WA */
