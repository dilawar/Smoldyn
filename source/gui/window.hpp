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
    Window(const char* name);
    ~Window();

    inline GLFWwindow* ref() const { return window_; }

    int graphicsUpdate();

    int simulate(simptr sim);

    int renderScene();

    int init();
    int clear();

    void set_arena(float width, float height);
    void update_canvas_size();

    void draw_limits(); // debug

    /**
     * Draw a box.
     */
    void draw_box_d(const std::array<float, DIMMAX>& pt1,
        const std::array<float, DIMMAX>& pt2);

    /**
     * Scaling function.
     *
     * Given arena size and dimension, compute pixel value for a given float.
     */
    template <typename T = double, typename U = float> U scalex(const T x)
    {
        if (!isArenaNormalized())
            return U(x);
        return U(arena_[0] * x / 2.0);
    }

    template <typename T = double, typename U = float> U scaley(const T y)
    {
        if (!isArenaNormalized())
            return U(y);
        return U(arena_[1] * y / 2.0f);
    }

    template <typename T = double, typename U = float> U scalexy(const T x)
    {
        //
        // Average of xscale and yscale.
        // TODO: One can write a more efficient function.
        //
        return (scalex<T, U>(x) + scaley<T, U>(x)) / 2.0;
    }

    /**
     * Maps bottom-left (-1,-1) - top-right (1,1) box to to (0,H), (W,0) (with
     * y-axis positive downwards).
     *
     * It is tricky to get right.
     * TODO: Have unit tests.
     */
    template <typename T = double, typename U = float> U X(const T x)
    {
        const auto xmax = ImGui::GetIO().DisplaySize.x;
        const auto scale = scale_[0];

        if (!isArenaNormalized())
            return U(scale * x + xmax / (1 + scale));

        const auto f = 1.f / canvas_to_arena_ratio_;
        if (!isArenaNormalized())
            return U((x / f + canvas_[0] / 2.0f) * f);

        U _t = U((x + 1) / 2.0 * canvas_[0]);
        return (f * _t) + (1 - f) * canvas_[0] / 2.0;
    }

    template <typename T = double, typename U = float> U Y(const T y)
    {
        const auto scale = scale_[1];
        const auto ymax = ImGui::GetIO().DisplaySize.y;
        if (!isArenaNormalized())
            return U(scale * y + ymax / (1 + scale));

        const auto f = 1.f / canvas_to_arena_ratio_;
        if (!isArenaNormalized())
            return U((y / f + canvas_[1] / 2.0f) * f);

        U _t = U((y + 1) / 2.0 * canvas_[1]);
        return (f * _t) + (1 - f) * canvas_[1] / 2.0;
    }

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
     * Compute 2d projection.
     *
     * TODO: Needs a lot of work if I should support 3d views with dynamic
     * camera position. May be Imguizmo might help a bit here.
     */
    template <typename T = double>
    inline ImVec2 _GetPos(T* const pos, size_t dim)
    {
        if (dim == 1)
            return ImVec2(X(pos[0]), canvas_[1] / 2.f);
        if (dim == 2)
            return ImVec2(X(pos[0]), Y(pos[1]));

        // 3d to 2d projection.
        auto f = 5;
        return ImVec2(
            X(f * pos[0] / (1 + pos[2])), Y(f * pos[1] / (1 + pos[2])));
    }

    template <typename T = float> inline ImVec2 PointOnCanvas(T x, T y, T z)
    {
        T pts[3] = { x, y, z };
        return _GetPos(pts, sim_->dim);
    }

    template <typename T = float> inline ImVec2 PointOnCanvas(T* pts)
    {
        return _GetPos(pts, sim_->dim);
    }

    /**
     * Convert a double* to ImVec2
     */
    template <typename T = double>
    inline ImVec2 toImVec2(T* const pos, size_t dim)
    {
        if (dim == 1)
            return ImVec2(X(pos[0]), canvas_[1] / 2);

        if (dim == 2)
            return ImVec2(X(pos[0]), Y(pos[1]));

        auto f = 5;
        if (dim == 3) {
            // 3d to 2d projection.
            return ImVec2(
                X(f * pos[0] / (1 + pos[2])), Y(f * pos[1] / (1 + pos[2])));
        }
        fmt::print(stderr, "We should not be here: dim={}", dim);
        return ImVec2();
    }

protected:
private:
    /* data */
    const char* name_;
    std::shared_ptr<simstruct> sim_;

    GLFWwindow* window_;
    int error_code_;

    bool initialized_;
    float frame_rate_;

    std::array<float, DIMMAX> arena_;  // size of simulation space.
    std::array<float, DIMMAX> canvas_; // Size of canvas. Usually 2x of arena_
    std::array<float, DIMMAX> scale_;  // scale graphics.

    float canvas_to_arena_ratio_;

    unsigned int fbo_;
};

}

#endif /* end of include guard: WINDOW_H_VQ6821WA */
