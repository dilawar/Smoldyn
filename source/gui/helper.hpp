//
//    Description: Helper functions.
//
//         Author:  Dilawar Singh (), dilawar@subcom.tech
//   Organization:  Subconscious Compute
//
// CREDIT: Most of these functions are modified from
// https://github.com/CedricGuillemet/ImGuizmo/blob/master/example/main.cpp
//
// TODO: Should I use eigen here?
//

#ifndef HELPER_HPP_R2QBDZCR
#define HELPER_HPP_R2QBDZCR

#include <array>
#include <cmath>
#include <limits>

#include "imgui.h"

#if USE_FMT

#include "fmt/core.h"
#include "fmt/ranges.h"

//
// Helper function using fmt.
//
template <typename... Args>
void _print(fmt::format_string<Args...> s, Args&&... args)
{
    fmt::print(std::move(s), std::forward<Args>(args)...);
}

template <typename... Args>
std::string _format(fmt::format_string<Args...> s, Args&&... args)
{
    return fmt::format(std::move(s), std::forward<Args>(args)...);
}

#endif

using namespace std;

static array<array<float, 16>, 4> gObjectMatrix
    = { { { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
              0.f, 0.f, 1.f },

        { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 2.f, 0.f,
            0.f, 1.f },

        { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 2.f, 0.f,
            2.f, 1.f },

        { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
            2.f, 1.f } } };

static const array<float, 16> gIdentityMatrix = { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f,
    0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f };

template <typename T = float>
void Frustum(T left, T right, T bottom, T top, T znear, T zfar, T* m16)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0f * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    m16[0] = temp / temp2;
    m16[1] = 0.0;
    m16[2] = 0.0;
    m16[3] = 0.0;
    m16[4] = 0.0;
    m16[5] = temp / temp3;
    m16[6] = 0.0;
    m16[7] = 0.0;
    m16[8] = (right + left) / temp2;
    m16[9] = (top + bottom) / temp3;
    m16[10] = (-zfar - znear) / temp4;
    m16[11] = -1.0f;
    m16[12] = 0.0;
    m16[13] = 0.0;
    m16[14] = (-temp * zfar) / temp4;
    m16[15] = 0.0;
}

template <typename T = float>
void Perspective(T fovyInDegrees, T aspectRatio, T znear, T zfar, T* m16)
{
    float ymax, xmax;
    ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
    xmax = ymax * aspectRatio;
    Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
}

template <typename T = float> void Cross(const T* a, const T* b, T* r)
{
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
}

template <typename T = float> float Dot(const T* a, const T* b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

template <typename T = float> void Normalize(const T* a, T* r)
{
    T il = 1.f / (sqrtf(Dot(a, a)) + std::numeric_limits<T>::epsilon());
    r[0] = a[0] * il;
    r[1] = a[1] * il;
    r[2] = a[2] * il;
}

template <typename T = float>
void LookAt(const T* eye, const T* at, const T* up, T* m16)
{
    T X[3], Y[3], Z[3], tmp[3];

    tmp[0] = eye[0] - at[0];
    tmp[1] = eye[1] - at[1];
    tmp[2] = eye[2] - at[2];
    Normalize(tmp, Z);
    Normalize(up, Y);

    Cross(Y, Z, tmp);
    Normalize(tmp, X);

    Cross(Z, X, tmp);
    Normalize(tmp, Y);

    m16[0] = X[0];
    m16[1] = Y[0];
    m16[2] = Z[0];
    m16[3] = 0.0f;
    m16[4] = X[1];
    m16[5] = Y[1];
    m16[6] = Z[1];
    m16[7] = 0.0f;
    m16[8] = X[2];
    m16[9] = Y[2];
    m16[10] = Z[2];
    m16[11] = 0.0f;
    m16[12] = -Dot(X, eye);
    m16[13] = -Dot(Y, eye);
    m16[14] = -Dot(Z, eye);
    m16[15] = 1.0f;
}

template <typename T = float>
void OrthoGraphic(const T l, T r, T b, const T t, T zn, const T zf, T* m16)
{
    m16[0] = 2 / (r - l);
    m16[1] = 0.0f;
    m16[2] = 0.0f;
    m16[3] = 0.0f;
    m16[4] = 0.0f;
    m16[5] = 2 / (t - b);
    m16[6] = 0.0f;
    m16[7] = 0.0f;
    m16[8] = 0.0f;
    m16[9] = 0.0f;
    m16[10] = 1.0f / (zf - zn);
    m16[11] = 0.0f;
    m16[12] = (l + r) / (l - r);
    m16[13] = (t + b) / (b - t);
    m16[14] = zn / (zn - zf);
    m16[15] = 1.0f;
}

template <typename T> inline void rotationY(const T angle, T* m16)
{
    T c = cosf(angle);
    T s = sinf(angle);

    m16[0] = c;
    m16[1] = 0.0f;
    m16[2] = -s;
    m16[3] = 0.0f;
    m16[4] = 0.0f;
    m16[5] = 1.f;
    m16[6] = 0.0f;
    m16[7] = 0.0f;
    m16[8] = s;
    m16[9] = 0.0f;
    m16[10] = c;
    m16[11] = 0.0f;
    m16[12] = 0.f;
    m16[13] = 0.f;
    m16[14] = 0.f;
    m16[15] = 1.0f;
}

//
// Colors.
//
#define COLOR_BLACK IM_COL32(0, 0, 0, 255)
#define COLOR_RED IM_COL32(255, 0, 0, 255)
#define COLOR_WHITE IM_COL32(255, 255, 255, 255)

//
// Coversions.
//
inline std::array<float, 2> ToArr2(const ImVec2& vec)
{
    return { vec.x, vec.y };
}

inline std::array<float, 4> ToArr4(const ImVec4& vec)
{
    return { vec.x, vec.y, vec.z, vec.w };
}

/**
 * Get r,g,b,a vector where r, g, b, a ∈ (0, 255) while the input vector
 * has range of (-1, 1).
 */
template <typename T = float> ImColor ArrToImColor(const T* const clr)
{
    float a = clr[3];
    if (a <= 0.001f) // numerical issue. Close to 0 is fine.
        a = 1.0f;
    return ImColor(float(clr[0]), float(clr[1]), float(clr[2]), a);
}

template <typename T = float>
ImVec4 ArrToColorVec(
    const T* const clr, bool inverted = false, bool debug = false)
{
    float a = clr[3];
    if (a <= 0.01f) // numerical issue. Close to 0 is fine.
        a = 1.0f;

    ImU32 R, G, B, A;
    R = ImU32(clr[0] * 255.0f);
    G = ImU32(clr[1] * 255.0f);
    B = ImU32(clr[2] * 255.0f);
    A = ImU32(a * 255.0f);

    if (inverted) {
        R = 255 - R;
        G = 255 - G;
        B = 255 - B;
    }

    if (debug) {
        _print(" <{},{},{},{}> -> <{},{},{},{}> \n", clr[0], clr[1], clr[2],
            clr[3], R, G, B, A);
    }
    return ImVec4(R, G, B, A);
}

/**
 * Helper function to convert a 4-array to ImGui color.
 */
template <typename T = float>
ImU32 ArrToColor(const T* const clr, bool inverted = false, bool debug = false)
{
    ImVec4 v = ArrToColorVec(clr, inverted, debug);
    return IM_COL32(v.x, v.y, v.z, v.w);
}

inline std::string STR(const ImVec2& vec)
{
    return _format("<{} {}>", vec.x, vec.y);
}

#endif /* end of include guard: HELPER_HPP_R2QBDZCR */
