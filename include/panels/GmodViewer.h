#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <imgui.h>

#include <functional>
#include <optional>
#include <utility>

namespace dnv::vista::sdk
{
    class VIS;
    class Gmod;
    class GmodPath;
} // namespace dnv::vista::sdk

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    class GmodViewer
    {
    public:
        explicit GmodViewer( const VIS& vis );

        void render( VisVersion version );

        const GmodNode* selectedNode( VisVersion version ) const;

        void setChangeNotifier( std::function<void()> notifier )
        {
            m_onChanged = std::move( notifier );
        }

        void setNodeSelectionCallback( std::function<void( std::optional<GmodPath> )> callback )
        {
            m_onNodeSelected = std::move( callback );
        }

    private:
        void renderHeader();
        void renderHelp();
        void renderTree( const Gmod& gmod, VisVersion version );
        void renderSearchResults( const Gmod& gmod, VisVersion version );
        void renderSearchResultsOverlay( const Gmod& gmod, VisVersion version );

        std::pair<ImVec4, ImVec4> badgeColors( const GmodNode& node ) const;
        bool renderBadge( const GmodNode& node );

        void notifyNodeSelection( const GmodNode* node, VisVersion version );
        void selectNode( const GmodNode& node, VisVersion version );

        // Helper methods
        std::string buildFullPathString( const GmodNode* node ) const;
        std::optional<GmodPath> buildGmodPath( const GmodNode* node, VisVersion version ) const;

        const VIS& m_vis;
        std::function<void()> m_onChanged;
        std::function<void( std::optional<GmodPath> )> m_onNodeSelected;

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
