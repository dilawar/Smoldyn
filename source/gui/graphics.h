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
#include "helper.hpp"

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

#if 0
    std::array<float, 4> FrameColor = { 0.f };
    std::array<float, 4> GridColor = { 0.f };
    std::array<float, 4> BgColor = { 0.f };
    std::array<float, 4> TextColor = { 0.f };
#endif

    size_t padding = 20;

    size_t CanvasWidth = 0;
    size_t CanvasHeight = 0;

    size_t CanvasOffsetX = 0;

    size_t CanvasOffsetY = 0;

    size_t canvasWidth()
    {
        return CanvasWidth - CanvasOffsetX - padding;
    }

    size_t canvasHeight()
    {
        return CanvasHeight - CanvasOffsetY - padding;
    }

    void computeSize()
    {
        const auto size = ImGui::GetWindowSize();
        CanvasWidth = size.x;
        CanvasHeight = size.y;
        CanvasOffsetY = 0;
    }

    void computeOffset(ImVec2& size)
    {
        //
    }

    void setViewPort()
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glViewport(10, 10, CanvasWidth, CanvasHeight);
    }

    void print() { fmt::print("Zoom={} Dim={}", Zoom, Dimension); }
};

// Use it globally.
extern GraphicsParam gGraphicsParam_;

// core simulation functions
void RenderSurfaces(simptr sim);
void RenderMolecs(simptr sim);
void RenderSim(simptr sim, void* data);

void DrawBoxD(double* pt1, double* pt2, int dim);
void DrawGridD(double* pt1, double* pt2, int* n, int dim);

void ChangeSize();

int GraphicsUpdate(simptr sim);

void Initialize(simptr sim);

/**
 * Compute various parameters for the GUI.
 */
template <typename T = float>
void ComputeParams(T xlo, T xhi, T ylo, T yhi, T zlo, T zhi)
{

    if (ylo == yhi && zlo == zhi)
        gGraphicsParam_.Dimension = 1;
    else if (zlo == zhi)
        gGraphicsParam_.Dimension = 2;
    else
        gGraphicsParam_.Dimension = 3;

    gGraphicsParam_.ClipSize = 1.05
        * ::sqrt((xhi - xlo) * (xhi - xlo) + (yhi - ylo) * (yhi - ylo)
            + (zhi - zlo) * (zhi - zlo));
    if (approximatelyEqual(gGraphicsParam_.ClipSize, 0.f, 0.1f))
        gGraphicsParam_.ClipSize = 1.0f;
    assert(gGraphicsParam_.ClipSize != 0.0f);

    gGraphicsParam_.ClipMidx = (xhi - xlo) / 2.0 + xlo;
    gGraphicsParam_.ClipMidy = (yhi - ylo) / 2.0 + ylo;
    gGraphicsParam_.ClipMidz = (zhi - zlo) / 2.0 + zlo;

    gGraphicsParam_.ClipLeft
        = gGraphicsParam_.ClipMidx - gGraphicsParam_.ClipSize / 2.0;
    gGraphicsParam_.ClipRight
        = gGraphicsParam_.ClipMidx + gGraphicsParam_.ClipSize / 2.0;
    gGraphicsParam_.ClipBot
        = gGraphicsParam_.ClipMidy - gGraphicsParam_.ClipSize / 2.0;
    gGraphicsParam_.ClipTop
        = gGraphicsParam_.ClipMidy + gGraphicsParam_.ClipSize / 2.0;
    gGraphicsParam_.ClipBack
        = gGraphicsParam_.ClipMidz - gGraphicsParam_.ClipSize / 2.0;
    gGraphicsParam_.ClipFront
        = gGraphicsParam_.ClipMidz + gGraphicsParam_.ClipSize / 2.0;

    if (gGraphicsParam_.Dimension == 2) {
        gGraphicsParam_.ClipLeft = xlo;
        gGraphicsParam_.ClipRight = xhi;
        gGraphicsParam_.ClipBot = ylo;
        gGraphicsParam_.ClipTop = yhi;
    }

    gGraphicsParam_.Xtrans = gGraphicsParam_.Ytrans = 0;
    gGraphicsParam_.Near = -gGraphicsParam_.ClipSize / 2.0;

    if (gGraphicsParam_.Dimension == 3) {
        glEnable(GL_DEPTH_TEST);
    }
    return;
}

/**
 * Credit: https://stackoverflow.com/a/7687413/1805129
 * TODO: Test it.
 */
void DrawSphere(double r, int lats, int longs);

template <typename T = float, typename Q = double>
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
