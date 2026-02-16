/**
 * @file GmodViewer.cpp
 * @brief Gmod tree viewer panel implementation
 *
 * Displays the Generic Product Model (Gmod) hierarchy following DNV's classification model.
 *
 * References:
 * - Vindøy, V. (2008). "A Functionally Oriented Vessel Data Model Used as Basis for Classification"
 *   Det Norske Veritas, Oslo/Norway
 *   The Gmod model is compliant with the modelling principles defined in ISO 15926.
 * - ISO 19848: Ships and marine technology - Standard data for shipboard machinery and equipment
 *   (Annex C: Gmod structure and coding system)
 */

#include "panels/GmodViewer.h"

namespace nfx::vista
{
    GmodViewer::GmodViewer( const VIS& vis )
        : m_vis{ vis },
          m_currentVersion{ vis.latest() },
          m_versionIndex{ static_cast<int>( vis.versions().size() ) - 1 }
    {
    }

    std::pair<ImVec4, ImVec4> GmodViewer::badgeColors( const GmodNode& node ) const
    {
        std::string_view category = node.metadata().category();
        std::string_view type = node.metadata().type();

        ImVec4 bg, text;

        if( node.isProductSelection() )
        {
            bg = ImVec4( 0.9f, 0.2f, 0.2f, 1.0f );
            text = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
        }
        else if( type == "GROUP" )
        {
            bg = ImVec4( 0.0f, 0.5f, 0.0f, 1.0f );
            text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }
        else if( category == "ASSET FUNCTION" && type == "LEAF" )
        {
            bg = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
            text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }
        else if( category == "PRODUCT FUNCTION" && type == "COMPOSITION" )
        {
            bg = ImVec4( 0.6f, 0.8f, 0.0f, 1.0f );
            text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }
        else if( category == "PRODUCT FUNCTION" && type == "LEAF" )
        {
            bg = ImVec4( 0.8f, 1.0f, 0.8f, 1.0f );
            text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }
        else
        {
            bg = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
            text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
        }

        return { bg, text };
    }

    bool GmodViewer::renderBadge( const GmodNode& node )
    {
        bool isProductType = node.metadata().category() == "PRODUCT" && node.metadata().type() == "TYPE";
        auto [badgeBg, badgeText] = badgeColors( node );

        ImVec4 mainBadgeBg = isProductType ? ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) : badgeBg;
        ImVec4 mainBadgeText = isProductType ? ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) : badgeText;

        ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
        ImGui::PushStyleColor( ImGuiCol_Button, mainBadgeBg );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, mainBadgeBg );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, mainBadgeBg );
        ImGui::PushStyleColor( ImGuiCol_Text, mainBadgeText );

        bool clicked = ImGui::Button( node.code().data(), ImVec2( 0.0f, 0.0f ) );

        ImGui::PopStyleColor( 4 );
        ImGui::PopStyleVar( 2 );

        return clicked;
    }

    void GmodViewer::render()
    {
        ImGui::SetNextWindowSize( ImVec2( 800, 600 ), ImGuiCond_FirstUseEver );
        ImGui::Begin( "Gmod Viewer" );

        renderLegend();
        ImGui::Separator();

        renderHeader();
        ImGui::Separator();

        const auto& gmod = m_vis.gmod( m_currentVersion );

        // Always show tree
        renderTree( gmod );

        ImGui::End();

        const auto& gmodForSearch = m_vis.gmod( m_currentVersion );
        bool showOverlay = m_search.buffer[0] != '\0' && ( m_search.boxHasFocus || m_search.overlayHovered );

        if( showOverlay )
        {
            renderSearchResultsOverlay( gmodForSearch );
        }
        else if( m_search.buffer[0] != '\0' )
        {
            // Search buffer not empty but overlay not shown = clicked outside, clear search
            m_search.buffer[0] = '\0';
        }
    }

    void GmodViewer::renderHeader()
    {
        ImGui::SeparatorText( "VIS version" );

        const auto& versions = m_vis.versions();

        if( ImGui::BeginCombo( "VIS Version", VisVersions::toString( versions[m_versionIndex] ).data() ) )
        {
            for( size_t i = 0; i < versions.size(); ++i )
            {
                bool isSelected = ( m_versionIndex == static_cast<int>( i ) );
                if( ImGui::Selectable( VisVersions::toString( versions[i] ).data(), isSelected ) )
                {
                    m_versionIndex = static_cast<int>( i );
                    m_currentVersion = versions[m_versionIndex];
                }
                if( isSelected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        const auto& gmod = m_vis.gmod( m_currentVersion );
        ImGui::SameLine();
        ImGui::Text( "| Nodes: %zu", std::distance( gmod.begin(), gmod.end() ) );

        // Search box
        ImGui::Spacing();
        ImGui::SetNextItemWidth( -1.0f ); // Full width

        char previousBuffer[256];
        strncpy( previousBuffer, m_search.buffer, sizeof( previousBuffer ) );

        ImGui::InputTextWithHint(
            "##search", "Search nodes (code or name)...", m_search.buffer, sizeof( m_search.buffer ) );

        // Increment search ID when buffer changes (new search) to force window reordering
        if( strcmp( previousBuffer, m_search.buffer ) != 0 )
        {
            m_search.id++;
        }

        m_search.boxHasFocus = ImGui::IsItemActive();
        m_search.boxPos = ImGui::GetItemRectMin();
        m_search.boxSize = ImGui::GetItemRectSize();
    }

    void GmodViewer::renderLegend()
    {
        if( ImGui::CollapsingHeader( "Legend", ImGuiTreeNodeFlags_DefaultOpen ) )
        {
            ImGui::Indent();

            // Badge types
            ImGui::TextUnformatted( "Badge Types:" );
            ImGui::BulletText( "Green badge: Asset function or Product function code" );
            ImGui::BulletText( "Red badge: Product Type code (products that can be assigned to functions)" );

            ImGui::Spacing();

            // Node categories (from Annex C, Table C.1)
            ImGui::TextUnformatted( "Node Categories:" );
            ImGui::BulletText( "Asset function leaf: Lowest level asset functions (e.g., 411.1 Propulsion driver)" );
            ImGui::BulletText( "Asset function group: Grouping of asset functions (e.g., 411 Propulsion)" );
            ImGui::BulletText( "Product function leaf: Lowest level product functions within products" );
            ImGui::BulletText( "Product function group: Grouping of product functions" );
            ImGui::BulletText( "Product Type: Specific product that can be assigned (e.g., C101 Engine)" );
            ImGui::BulletText( "Product Selection: Group of selectable Product Types (hidden, children shown)" );

            ImGui::Spacing();

            // Colors
            ImGui::TextUnformatted( "Badge Colors:" );

            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.5f, 0.0f, 1.0f ) );
            ImGui::SmallButton( "#008000" );
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextUnformatted( "Dark green - GROUP nodes" );

            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
            ImGui::SmallButton( "#00ff00" );
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextUnformatted( "Lime green - ASSET FUNCTION LEAF" );

            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.6f, 0.8f, 0.0f, 1.0f ) );
            ImGui::SmallButton( "#99cc00" );
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextUnformatted( "Yellow-green - PRODUCT FUNCTION COMPOSITION" );

            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.8f, 1.0f, 0.8f, 1.0f ) );
            ImGui::SmallButton( "#ccffcc" );
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextUnformatted( "Light green - PRODUCT FUNCTION LEAF" );

            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
            ImGui::SmallButton( "#ff0000" );
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextUnformatted( "Red - PRODUCT TYPE" );

            ImGui::Unindent();
        }
    }

    void GmodViewer::renderTree( const Gmod& gmod )
    {
        ImGui::BeginChild( "GmodTree", ImVec2( 0, 0 ), true );

        // Based on Vindøy (2008) "A Functionally Oriented Vessel Data Model Used as Basis for Classification"
        // The Gmod tree structure is compliant with ISO 15926 modelling principles and defines:
        // - Function leaves: end nodes connected to physical components
        // - Function compositions: parent composed of children (not substitutable)
        // - Function selections: children are specializations of parent (substitutable, removed in vessel models)
        // - Function groups: organizational grouping

        // Helper to check if a node will have visible children after filtering
        auto hasVisibleChildren = [&]( const GmodNode& node ) -> bool {
            if( node.children().isEmpty() )
            {
                return false;
            }

            auto nodeProductType = node.productType();
            for( const auto* child : node.children() )
            {
                // Skip if child is the Product Type (already shown as badge)
                if( nodeProductType.has_value() && child == nodeProductType.value() )
                {
                    // Check if Product Type has grandchildren
                    if( !child->children().isEmpty() )
                    {
                        return true;
                    }
                    continue;
                }

                // Product Selections and Function Selections are skipped but their children are shown
                if( child->isProductSelection() )
                {
                    if( !child->children().isEmpty() )
                    {
                        return true;
                    }
                }
                else if(
                    child->metadata().type() == "SELECTION" && ( child->metadata().category() == "PRODUCT FUNCTION" ||
                                                                 child->metadata().category() == "ASSET FUNCTION" ) )
                {
                    if( !child->children().isEmpty() )
                    {
                        return true;
                    }
                }
                else
                {
                    // This child will be rendered
                    return true;
                }
            }

            return false;
        };

        // Recursive function to render tree nodes
        // parentNode: optional parent node to display as badge (for nodes promoted from skipped selections)
        std::function<void( const GmodNode&, const GmodNode* )> renderNode;
        renderNode = [&]( const GmodNode& node, const GmodNode* parentNode = nullptr ) {
            // Push unique ID scope for this entire node (to avoid conflicts with nodes having same name)
            ImGui::PushID( static_cast<const void*>( &node ) );

            bool isProductType = node.metadata().category() == "PRODUCT" && node.metadata().type() == "TYPE";
            auto [badgeBg, badgeText] = badgeColors( node );

            // Check if this is the target node for navigation
            bool isTargetNode = m_navigation.scrollToNode && ( node.code() == m_navigation.selectedNodeCode );
            bool isSelectedNode =
                !m_navigation.selectedNodeCode.empty() && ( node.code() == m_navigation.selectedNodeCode );

            // Render tree node
            ImGui::AlignTextToFramePadding();

            char treeId[256];
            snprintf( treeId, sizeof( treeId ), "##tree_%s_%p", node.code().data(), static_cast<const void*>( &node ) );

            bool nodeOpen = false;
            bool willHaveChildren = hasVisibleChildren( node );

            if( willHaveChildren )
            {
                // Check if we should auto-expand this node (if target is a descendant OR if it's the selected node)
                bool shouldExpand = false;
                if( m_navigation.scrollToNode && !m_navigation.selectedNodeCode.empty() )
                {
                    auto& gmod = m_vis.gmod( m_currentVersion );
                    auto targetOpt = gmod.node( m_navigation.selectedNodeCode );
                    if( targetOpt.has_value() )
                    {
                        const GmodNode* target = targetOpt.value();
                        // Walk up from target to see if current node is an ancestor
                        const GmodNode* current = target;
                        while( !current->parents().isEmpty() )
                        {
                            current = current->parents()[0];
                            if( current == &node )
                            {
                                shouldExpand = true;
                                break;
                            }
                        }
                    }
                }

                // Also expand if this is the selected node and we want to expand it
                if( isSelectedNode && m_navigation.expandSelectedNode )
                {
                    shouldExpand = true;
                }

                if( shouldExpand )
                {
                    ImGui::SetNextItemOpen( true );
                }

                nodeOpen =
                    ImGui::TreeNodeEx( treeId, ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow );
                ImGui::SameLine();
            }
            else
            {
                // Leaf nodes: display bullet instead of arrow
                ImGui::TreeNodeEx(
                    treeId,
                    ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet |
                        ImGuiTreeNodeFlags_SpanFullWidth );
                ImGui::SameLine();
            }

            // Scroll to target node
            if( isTargetNode )
            {
                ImGui::SetScrollHereY( 0.5f );
                m_navigation.scrollToNode = false;       // Reset flag after scrolling
                m_navigation.expandSelectedNode = false; // Reset expand flag
            }

            // Render parent badge if provided (for nodes from skipped selections)
            if( parentNode != nullptr )
            {
                renderBadge( *parentNode );
                ImGui::SameLine();
            }

            // Render main badge
            renderBadge( node );

            // Render Product Type badge if node has one
            auto productTypeOpt = node.productType();
            std::string_view category = node.metadata().category();
            if( productTypeOpt.has_value() && ( category == "PRODUCT FUNCTION" || category == "ASSET FUNCTION" ) )
            {
                const auto* productTypeNode = productTypeOpt.value();
                ImGui::SameLine();
                renderBadge( *productTypeNode );
            }

            ImGui::SameLine();

            // Display name
            if( node.metadata().commonName().has_value() )
            {
                ImGui::TextUnformatted( node.metadata().commonName().value().data() );
            }
            else
            {
                ImGui::TextUnformatted( node.metadata().name().data() );
            }

            // Render children recursively
            if( nodeOpen && !node.children().isEmpty() )
            {
                // Check if node has a Product Type to avoid rendering it twice
                auto nodeProductType = node.productType();

                for( const auto* child : node.children() )
                {
                    // Skip child if it's the same as the node's Product Type (already shown as badge)
                    // But render its children (grandchildren)
                    if( nodeProductType.has_value() && child == nodeProductType.value() )
                    {
                        // Render grandchildren as if they were direct children
                        for( const auto* grandchild : child->children() )
                        {
                            if( grandchild->isProductSelection() )
                            {
                                for( const auto* greatGrandchild : grandchild->children() )
                                {
                                    renderNode( *greatGrandchild, &node );
                                }
                            }
                            else if(
                                grandchild->metadata().category() == "PRODUCT" &&
                                grandchild->metadata().type() == "TYPE" )
                            {
                                renderNode( *grandchild, &node );
                            }
                            else
                            {
                                renderNode( *grandchild, nullptr );
                            }
                        }
                        continue;
                    }

                    // Skip Product Selections (Component selections like CS1, CS2) but render their children
                    // Reference: Vindøy (2008) Section 2.3 - "Component selections are groups of Components
                    // with a parent and children. When the selection has been performed, the Component selection
                    // is substituted by the selected child."
                    if( child->isProductSelection() )
                    {
                        for( const auto* grandchild : child->children() )
                        {
                            renderNode( *grandchild, &node );
                        }
                    }
                    // Skip Product Function Selections (Function selections like C101.2s) but render their children
                    // Reference: Vindøy (2008) Section 2.2 - "Function selections are groups of Functions with
                    // a parent and children. When applied to a vessel, it is generally allowed to select more
                    // than one child. When the selection has been performed, the Function selection is removed."
                    else if(
                        child->metadata().type() == "SELECTION" &&
                        ( child->metadata().category() == "PRODUCT FUNCTION" ||
                          child->metadata().category() == "ASSET FUNCTION" ) )
                    {
                        for( const auto* grandchild : child->children() )
                        {
                            renderNode( *grandchild, &node );
                        }
                        continue; // Don't render the selection node itself
                    }
                    // Product Types get parent badge
                    else if( child->metadata().category() == "PRODUCT" && child->metadata().type() == "TYPE" )
                    {
                        // If current node is also a Product Type, propagate the parent badge
                        const GmodNode* badgeToShow = isProductType ? parentNode : &node;
                        renderNode( *child, badgeToShow );
                    }
                    else
                    {
                        renderNode( *child, nullptr );
                    }
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
        };

        // Start from root node
        const auto& rootNode = gmod.rootNode();
        if( !rootNode.children().isEmpty() )
        {
            std::vector<const GmodNode*> sortedChildren;
            for( const auto* child : rootNode.children() )
            {
                sortedChildren.push_back( child );
            }

            // Natural sort: extract numeric prefix for proper ordering
            std::sort( sortedChildren.begin(), sortedChildren.end(), []( const GmodNode* a, const GmodNode* b ) {
                auto extractNumber = []( std::string_view code ) -> int {
                    int num = 0;
                    for( char c : code )
                    {
                        if( c >= '0' && c <= '9' )
                        {
                            num = num * 10 + ( c - '0' );
                        }
                        else
                        {
                            break;
                        }
                    }
                    return num;
                };
                int numA = extractNumber( a->code() );
                int numB = extractNumber( b->code() );
                if( numA != numB )
                {
                    return numA < numB;
                }
                return a->code() < b->code();
            } );

            for( const auto* child : sortedChildren )
            {
                renderNode( *child, nullptr );
            }
        }

        ImGui::EndChild();
    }

    void GmodViewer::renderSearchResults( const Gmod& gmod )
    {
        // Convert search string to lowercase for case-insensitive search
        std::string searchLower = m_search.buffer;
        std::transform( searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower );

        if( searchLower.empty() )
        {
            ImGui::TextDisabled( "Type to search..." );
            return;
        }

        // Search through all nodes and render results
        int resultCount = 0;
        for( const auto& [code, node] : gmod )
        {
            // Convert node code and name to lowercase for comparison
            std::string codeLower = std::string( node.code() );
            std::transform( codeLower.begin(), codeLower.end(), codeLower.begin(), ::tolower );

            std::string nameLower = std::string( node.metadata().name() );
            std::transform( nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower );

            std::string commonNameLower;
            if( node.metadata().commonName().has_value() )
            {
                commonNameLower = std::string( node.metadata().commonName().value() );
                std::transform( commonNameLower.begin(), commonNameLower.end(), commonNameLower.begin(), ::tolower );
            }

            // Check if search term matches code or name
            // Code: contains search term (incremental search: "c10" matches "C101", "C1082", etc.)
            bool codeMatch = codeLower.find( searchLower ) != std::string::npos;
            bool nameMatch = nameLower.find( searchLower ) != std::string::npos;
            bool commonNameMatch = !commonNameLower.empty() && commonNameLower.find( searchLower ) != std::string::npos;

            if( codeMatch || nameMatch || commonNameMatch )
            {
                resultCount++;

                // Build short path following ISO 19848 rules:
                // Only include "individualizable" nodes (ending with 'i', 's') and Product Types
                std::vector<const GmodNode*> shortPath;
                const GmodNode* current = &node;

                // Walk up to root, collecting only individualizable nodes and Product Types
                while( !current->parents().isEmpty() )
                {
                    const GmodNode* parent = current->parents()[0];
                    if( parent->code() == "VE" )
                        break; // Stop at root

                    // Include if individualizable (ends with 'i' or 's') or is a Product Type
                    std::string_view code = parent->code();
                    bool isIndividualizable = !code.empty() && ( code.back() == 'i' || code.back() == 's' );
                    bool isProductType =
                        parent->metadata().category() == "PRODUCT" && parent->metadata().type() == "TYPE";

                    if( isIndividualizable || isProductType )
                    {
                        shortPath.push_back( parent );
                    }

                    current = parent;
                }
                std::reverse( shortPath.begin(), shortPath.end() );

                ImGui::PushID( resultCount );

                bool clicked = false;
                std::string clickedNodeCode;

                // Render short path badges
                int badgeIndex = 0;
                for( const GmodNode* parent : shortPath )
                {
                    ImGui::PushID( badgeIndex++ );
                    if( renderBadge( *parent ) )
                    {
                        clicked = true;
                        clickedNodeCode = std::string( parent->code() );
                    }
                    ImGui::PopID();
                    ImGui::SameLine();
                }

                // Render current node badge
                ImGui::PushID( badgeIndex );
                if( renderBadge( node ) )
                {
                    clicked = true;
                    clickedNodeCode = std::string( node.code() );
                }
                ImGui::PopID();

                ImGui::SameLine();

                // Display name as selectable
                const char* displayName = node.metadata().commonName().has_value()
                                              ? node.metadata().commonName().value().data()
                                              : node.metadata().name().data();

                if( ImGui::Selectable( displayName, false ) )
                {
                    clicked = true;
                    clickedNodeCode = std::string( node.code() );
                }

                // Handle click: navigate to node in tree
                if( clicked )
                {
                    m_navigation.selectedNodeCode = clickedNodeCode;
                    m_navigation.scrollToNode = true;
                    m_navigation.expandSelectedNode = true; // Also expand the selected node if it has children
                    // Don't close search - user must click outside
                }

                ImGui::PopID();
            }
        }

        if( resultCount == 0 )
        {
            ImGui::TextDisabled( "No results found" );
        }
    }

    void GmodViewer::renderSearchResultsOverlay( const Gmod& gmod )
    {
        // Position the overlay window below the search box
        ImVec2 overlayPos = ImVec2( m_search.boxPos.x, m_search.boxPos.y + m_search.boxSize.y );

        ImGui::SetNextWindowPos( overlayPos, ImGuiCond_Always );
        ImGui::SetNextWindowSize( ImVec2( m_search.boxSize.x, 300 ), ImGuiCond_Always );
        ImGui::SetNextWindowBgAlpha( 0.95f );

        // Use search ID in window name to force re-ordering when search changes
        char windowName[64];
        snprintf( windowName, sizeof( windowName ), "SearchOverlay##%d", m_search.id );

        ImGui::Begin(
            windowName,
            nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoFocusOnAppearing );

        renderSearchResults( gmod );

        // Track if overlay is hovered to keep it open
        m_search.overlayHovered = ImGui::IsWindowHovered( ImGuiHoveredFlags_AllowWhenBlockedByActiveItem );

        ImGui::End();
    }
} // namespace nfx::vista
