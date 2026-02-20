#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <imgui.h>

#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace dnv::vista::sdk
{
    class VIS;
    class Gmod;
    class GmodPath;
} // namespace dnv::vista::sdk

namespace nfx::vista
{
    class GmodViewer
    {
    public:
        explicit GmodViewer( const dnv::vista::sdk::VIS& vis );

        void render( dnv::vista::sdk::VisVersion version );

        const dnv::vista::sdk::GmodNode* selectedNode( dnv::vista::sdk::VisVersion version ) const;

        void setChangeNotifier( std::function<void()> notifier )
        {
            m_onChanged = std::move( notifier );
        }

        void setNodeSelectionCallback( std::function<void( std::optional<dnv::vista::sdk::GmodPath> )> callback )
        {
            m_onNodeSelected = std::move( callback );
        }

    private:
        void renderHeader();
        void renderHelp();
        void renderTree( const dnv::vista::sdk::Gmod& gmod, dnv::vista::sdk::VisVersion version );
        void renderSearchResults( const dnv::vista::sdk::Gmod& gmod, dnv::vista::sdk::VisVersion version );
        void renderSearchResultsOverlay( const dnv::vista::sdk::Gmod& gmod, dnv::vista::sdk::VisVersion version );

        std::pair<ImVec4, ImVec4> badgeColors( const dnv::vista::sdk::GmodNode& node ) const;
        bool renderBadge( const dnv::vista::sdk::GmodNode& node );

        void notifyNodeSelection( const dnv::vista::sdk::GmodNode* node, dnv::vista::sdk::VisVersion version );
        void selectNode( const dnv::vista::sdk::GmodNode& node, dnv::vista::sdk::VisVersion version );

        // Helper methods
        std::string buildFullPathString( const dnv::vista::sdk::GmodNode* node ) const;
        std::optional<dnv::vista::sdk::GmodPath> buildGmodPath(
            const dnv::vista::sdk::GmodNode* node, dnv::vista::sdk::VisVersion version ) const;

        const dnv::vista::sdk::VIS& m_vis;
        std::function<void()> m_onChanged;
        std::function<void( std::optional<dnv::vista::sdk::GmodPath> )> m_onNodeSelected;

        struct SearchState
        {
            std::string buffer;
            bool boxHasFocus = false;
            ImVec2 boxPos;
            ImVec2 boxSize;
            bool overlayHovered = false;
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
