//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//

#ifndef GRAPHICS_H_48HTFCAR
#define GRAPHICS_H_48HTFCAR

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <cmath>
#include <array>

#include <GL/glew.h>

#include "smoldyn.h"

namespace gui {

/**
 * Structure to keep graphics related parameters.
 */
struct GraphicsParam {
    size_t Dimension = 3;

    float ClipSize = 1.0f;

    float ClipMidx = 0.f;
    float ClipMidy = 0.f;
    float ClipMidz = 0.f;

    float ClipLeft = 0.f;
    float ClipRight = 0.f;
    float ClipBot = 0.f;
    float ClipTop = 0.f;
    float ClipFront = 0.f;
    float ClipBack = 0.f;

    int Zoom = 1;
    float FieldOfView = 45.f;
    float Aspect = 1.0;

    size_t PixWide = 0, PixHigh = 0;

    int Fix2DAspect = 0;

    int Xtrans = 0, Ytrans = 0;
    int Near = 0;
    int Gl2PauseState = 0;

    std::array<float, 4> FrameColor = { 0.f };
    std::array<float, 4> GridColor = { 0.f };
    std::array<float, 4> BgColor = { 0.f };
    std::array<float, 4> TextColor = { 0.f };

    void print() { fmt::print("Zoom={} Dim={}", Zoom, Dimension); }
};

// Use it globally.
extern GraphicsParam gGraphicsParam_;

// core simulation functions
void RenderSurfaces(simptr sim);
void RenderMolecs(simptr sim);
void RenderSim(simptr sim, void* data);

void ChangeSize(int w, int h);

int GraphicsUpdate(simptr sim);

void Initialize(simptr sim);
void GL2_Initialize(float* low, float* high, size_t dim);

/**
 * Credit: https://stackoverflow.com/a/7687413/1805129
 * TODO: Test it.
 */
void DrawSphere(double r, int lats, int longs);

template <typename T = float, typename Q=double>
inline T* ConvertTo(Q* const vals, T* output, const size_t N)
{
    for (size_t i = 0; i < N; ++i)
        output[i] = (float)vals[i];
    return output;
}

template <typename T = double> T normalize(T* x, const size_t N)
{
    T sum = T(0.0);
    for (size_t i = 0; i < N; ++i)
        sum += x[i] * x[i];
    if (!sum)
        return 0;
    sum = std::sqrt(sum);
    for (size_t i = 0; i < N; i++)
        x[i] /= sum;
    return sum;
}

} // namespace gui

#endif /* end of include guard: GRAPHICS_H_48HTFCAR */
