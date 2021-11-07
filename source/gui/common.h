//
//    Description: Common function and data.
//
//         Author:  Dilawar Singh (), dilawar@subcom.tech
//   Organization:  Subconscious Compute
//

#ifndef COMMON_H_ACUI6QRG
#define COMMON_H_ACUI6QRG

#include "imgui.h"
#include <array>
#include <string>

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
        fmt::print(" <{},{},{},{}> -> <{},{},{},{}> \n", clr[0], clr[1], clr[2],
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
    return fmt::format("<{} {}>", vec.x, vec.y);
}

#endif /* end of include guard: COMMON_H_ACUI6QRG */
