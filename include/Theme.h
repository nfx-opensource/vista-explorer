#pragma once

#include <imgui.h>

namespace nfx::vista::Theme
{
    // --- Base palette (mirrors Application.cpp) ---
    inline constexpr ImVec4 Bg0 = { 0.14f, 0.14f, 0.14f, 1.00f };          // darkest bg (frames, scrollbar)
    inline constexpr ImVec4 Bg1 = { 0.18f, 0.18f, 0.18f, 1.00f };          // panel bg, child bg
    inline constexpr ImVec4 Bg2 = { 0.22f, 0.22f, 0.22f, 1.00f };          // window bg
    inline constexpr ImVec4 Bg3 = { 0.30f, 0.30f, 0.30f, 1.00f };          // buttons, interactive
    inline constexpr ImVec4 Bg4 = { 0.40f, 0.40f, 0.40f, 1.00f };          // scrollbar grab, hover
    inline constexpr ImVec4 Border = { 0.35f, 0.35f, 0.35f, 1.00f };       // window/separator borders
    inline constexpr ImVec4 Accent = { 0.29f, 0.60f, 1.00f, 1.00f };       // blue
    inline constexpr ImVec4 AccentBright = { 0.40f, 0.70f, 1.00f, 1.00f }; // slider grab active, labels
    inline constexpr ImVec4 AccentDark = { 0.20f, 0.48f, 0.90f, 1.00f };   // button active

    // --- Semantic colors ---
    inline constexpr ImVec4 TextError = { 1.00f, 0.35f, 0.35f, 1.00f };   // validation errors, status errors
    inline constexpr ImVec4 TextSuccess = { 0.30f, 1.00f, 0.30f, 1.00f }; // validation OK, status success
    inline constexpr ImVec4 TextWarning = { 0.90f, 0.80f, 0.20f, 1.00f }; // previews, non-critical notices
    inline constexpr ImVec4 TextPath = { 0.60f, 1.00f, 0.60f, 1.00f };    // short paths (green)
    inline constexpr ImVec4 TextLabel = AccentBright;                     // section labels, field headers in tooltips
    inline constexpr ImVec4 TextCode = AccentBright;                      // node codes, identifiers

    // --- Overlay ---
    inline constexpr ImVec4 OverlayBg = { 0.15f, 0.15f, 0.15f, 0.90f }; // floating search overlay background
} // namespace nfx::vista::Theme
