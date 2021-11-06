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

#include "smoldyn.h"

namespace smoldyn {

class Window {

public:
    Window(const char* name);
    ~Window();

    inline GLFWwindow* ref() const { return window_; }

    int simulate(simptr sim);

    int render_molecules(ImDrawList* drawlist);

    int render_scene();

    int init();
    int clear();

    void set_arena(float width, float height);
    void update_canvas_size();

    void draw_limits(); // debug

    /**
     * Scaling function.
     *
     * Given arena size and dimension, compute pixel value for a given float.
     */
    template <typename T = double, typename U = float> U scalex(const T x)
    {
        return U(arena_[0] * x / 2.0);
    }

    template <typename T = double, typename U = float> U scaley(const T y)
    {
        return U(arena_[1] * y / 2.0);
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
        const auto f = 1 / canvas_to_arena_ratio_;
        U _t = U((x + 1) / 2.0 * canvas_[0]);
        // return _t;
        return (f * _t) + (1 - f) * canvas_[0] / 2.0;
    }

    template <typename T = double, typename U = float> U Y(const T y)
    {
        const auto f = 1 / canvas_to_arena_ratio_;
        U _t = U((y + 1) / 2.0 * canvas_[1]);
        return (f * _t) + (1 - f) * canvas_[1] / 2.0;
    }

    /**
     * Get r,g,b,a vector where r, g, b, a ∈ (0, 255) while the input vector
     * has range of (-1, 1).
     */
    template <typename T = double> inline ImVec4 _GetColorVec(T* const clr)
    {
        // printf(">> clr %f %f %f %f\n", clr[0], clr[1], clr[2], clr[3]);
        // fixme: The A value has to be substracted from 1.0

        return ImVec4 { 255 * float(clr[0]), 255 * float(clr[1]),
            255 * float(clr[2]), 255 * (1 - float(clr[3])) };
    }

    /**
     * Helper function to convert a 4-array to ImGui color.
     */
    template <typename T = double> ImU32 _GetColor(T* const clr)
    {
        return ImGui::GetColorU32(_GetColorVec<T>(clr));
    }

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
            return ImVec2(X(pos[0]), canvas_[1] / 2);
        if (dim == 2)
            return ImVec2(X(pos[0]), Y(pos[1]));
        if (dim == 3) {
            // 3d to 2d projection.
            auto f = 5;
            return ImVec2(
                X(f * pos[0] / (1 + pos[2])), Y(f * pos[1] / (1 + pos[2])));
        }
        fprintf(stderr, "dim=%d is not supported.\n", dim);
    }

private:
    /* data */
    const char* name_;
    simptr sim_;

    GLFWwindow* window_;
    int error_code_;

    bool initialized_;
    float frame_rate_;

    float arena_[2];  // size of simulation space.
    float canvas_[2]; // Size of canvas. Usually 2x of arena_

    float canvas_to_arena_ratio_;
};

}

#endif /* end of include guard: WINDOW_H_VQ6821WA */
