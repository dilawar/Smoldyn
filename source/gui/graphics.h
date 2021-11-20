//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//

#ifndef GRAPHICS_H_48HTFCAR
#define GRAPHICS_H_48HTFCAR

#include "imgui.h"
#include "imgui_impl_opengl3.h"

#include <cmath>
#include <GL/glew.h>

#include "smoldyn.h"

namespace gui {

// core simulation functions
void RenderSurfaces(simptr sim);
void RenderMolecs(simptr sim);
void RenderText(simptr sim);
void RenderSim(simptr sim, void* data);

/**
 * Credit: https://stackoverflow.com/a/7687413/1805129
 * TODO: Test it.
 */
void draw_sphere(double r, int lats, int longs);

template <typename T = float>
inline T* ConvertDoubleTo(double* const vals, T* output, const size_t N)
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
