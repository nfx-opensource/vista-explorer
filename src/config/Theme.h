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

    // --- Apply ---
    inline void apply( ImGuiStyle& style )
    {
        // Shape
        style.WindowRounding = 6.0f;
        style.ChildRounding = 4.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.WindowPadding = ImVec2( 10.0f, 10.0f );
        style.FramePadding = ImVec2( 6.0f, 4.0f );
        style.ItemSpacing = ImVec2( 8.0f, 6.0f );
        style.ScrollbarSize = 12.0f;
        style.GrabMinSize = 10.0f;

        // Colors
        ImVec4* c = style.Colors;

        c[ImGuiCol_WindowBg] = Bg2;
        c[ImGuiCol_ChildBg] = Bg1;
        c[ImGuiCol_PopupBg] = ImVec4( Bg2.x, Bg2.y, Bg2.z, 0.98f );
        c[ImGuiCol_Border] = Border;

        c[ImGuiCol_FrameBg] = Bg0;
        c[ImGuiCol_FrameBgHovered] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
        c[ImGuiCol_FrameBgActive] = ImVec4( 0.12f, 0.12f, 0.12f, 1.00f );

        c[ImGuiCol_TitleBg] = Bg1;
        c[ImGuiCol_TitleBgActive] = Bg1;
        c[ImGuiCol_TitleBgCollapsed] = Bg0;

        c[ImGuiCol_MenuBarBg] = Bg1;
        c[ImGuiCol_ScrollbarBg] = Bg0;
        c[ImGuiCol_ScrollbarGrab] = Bg4;
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
        c[ImGuiCol_ScrollbarGrabActive] = Accent;

        c[ImGuiCol_CheckMark] = Accent;
        c[ImGuiCol_SliderGrab] = Accent;
        c[ImGuiCol_SliderGrabActive] = AccentBright;

        c[ImGuiCol_Button] = Bg3;
        c[ImGuiCol_ButtonHovered] = Accent;
        c[ImGuiCol_ButtonActive] = AccentDark;

        c[ImGuiCol_Header] = ImVec4( Accent.x, Accent.y, Accent.z, 0.30f );
        c[ImGuiCol_HeaderHovered] = ImVec4( Accent.x, Accent.y, Accent.z, 0.50f );
        c[ImGuiCol_HeaderActive] = ImVec4( Accent.x, Accent.y, Accent.z, 0.80f );

        c[ImGuiCol_Separator] = Border;
        c[ImGuiCol_SeparatorHovered] = ImVec4( Accent.x, Accent.y, Accent.z, 0.70f );
        c[ImGuiCol_SeparatorActive] = Accent;

        c[ImGuiCol_ResizeGrip] = ImVec4( Accent.x, Accent.y, Accent.z, 0.20f );
        c[ImGuiCol_ResizeGripHovered] = ImVec4( Accent.x, Accent.y, Accent.z, 0.60f );
        c[ImGuiCol_ResizeGripActive] = ImVec4( Accent.x, Accent.y, Accent.z, 0.90f );

        c[ImGuiCol_Tab] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
        c[ImGuiCol_TabHovered] = ImVec4( Accent.x, Accent.y, Accent.z, 0.70f );
        c[ImGuiCol_TabActive] = ImVec4( 0.24f, 0.50f, 0.90f, 1.00f );
        c[ImGuiCol_TabUnfocused] = Bg1;
        c[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.22f, 0.40f, 0.70f, 1.00f );

        c[ImGuiCol_DockingPreview] = ImVec4( Accent.x, Accent.y, Accent.z, 0.50f );
        c[ImGuiCol_DockingEmptyBg] = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );

        c[ImGuiCol_PlotLines] = Accent;
        c[ImGuiCol_PlotHistogram] = Accent;

        c[ImGuiCol_TextSelectedBg] = ImVec4( Accent.x, Accent.y, Accent.z, 0.35f );
        c[ImGuiCol_NavHighlight] = Accent;
    }
} // namespace nfx::vista::Theme
