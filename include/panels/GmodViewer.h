#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <imgui.h>

#include <utility>

namespace dnv::vista::sdk
{
    class VIS;
    class Gmod;
} // namespace dnv::vista::sdk

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    class GmodViewer
    {
    public:
        explicit GmodViewer( const VIS& vis );

        void render();
        void setVersion( VisVersion version );

        const GmodNode* selectedNode() const;

    private:
        void renderHeader();
        void renderHelp();
        void renderTree( const Gmod& gmod );
        void renderSearchResults( const Gmod& gmod );
        void renderSearchResultsOverlay( const Gmod& gmod );

        std::pair<ImVec4, ImVec4> badgeColors( const GmodNode& node ) const;
        bool renderBadge( const GmodNode& node );

        const VIS& m_vis;
        VisVersion m_currentVersion;

        struct SearchState
        {
            char buffer[256] = {};
            bool boxHasFocus = false;
            ImVec2 boxPos;
            ImVec2 boxSize;
            bool overlayHovered = false;
            int id = 0; // Incremented when search changes to force window reordering
        };
        SearchState m_search;

        struct NavigationState
        {
            std::string selectedNodeCode;
            bool scrollToNode = false;
            bool expandSelectedNode = false;
        };
        NavigationState m_navigation;
    };
} // namespace nfx::vista
