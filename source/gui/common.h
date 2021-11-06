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
template <typename T = double> ImColor ArrToImColor(const T* const clr)
{
    float a = clr[3];
    if (a <= 0.0001f) // numerical issue. Close to 0 is fine.
        a = 1.0f;
    return ImColor(float(clr[0]), float(clr[1]), float(clr[2]), a);
}

/**
 * Helper function to convert a 4-array to ImGui color.
 */
template <typename T = double> ImU32 ArrToColor(const T* const clr)
{
    auto c = ImU32(ArrToImColor(clr));
    // fmt::print(">> clr {} {} {} {} => {}\n", clr[0], clr[1], clr[2], clr[3],
    // c);
    return c;
}

#endif /* end of include guard: COMMON_H_ACUI6QRG */
