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

#include <dnv/vista/sdk/VIS.h>
#include <imgui.h>

namespace nfx::vista
{
    GmodViewer::GmodViewer( const VIS& vis )
        : m_vis{ vis },
          m_currentVersion{ vis.latest() },
          m_versionIndex{ static_cast<int>( vis.versions().size() ) - 1 }
    {
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
        renderTree( gmod );

        ImGui::End();
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

        // Helper to determine badge colors based on node type
        auto getBadgeColors = []( const GmodNode& node ) -> std::pair<ImVec4, ImVec4> {
            std::string_view category = node.metadata().category();
            std::string_view type = node.metadata().type();

            ImVec4 bg, text;

            // Red for Product Selections
            if( node.isProductSelection() )
            {
                bg = ImVec4( 0.9f, 0.2f, 0.2f, 1.0f );
                text = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
            }
            // Dark green for GROUP
            else if( type == "GROUP" )
            {
                bg = ImVec4( 0.0f, 0.5f, 0.0f, 1.0f );
                text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
            }
            // Lime green for ASSET FUNCTION LEAF
            else if( category == "ASSET FUNCTION" && type == "LEAF" )
            {
                bg = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
                text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
            }
            // Yellow-green for PRODUCT FUNCTION COMPOSITION
            else if( category == "PRODUCT FUNCTION" && type == "COMPOSITION" )
            {
                bg = ImVec4( 0.6f, 0.8f, 0.0f, 1.0f );
                text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
            }
            // Light green for PRODUCT FUNCTION LEAF
            else if( category == "PRODUCT FUNCTION" && type == "LEAF" )
            {
                bg = ImVec4( 0.8f, 1.0f, 0.8f, 1.0f );
                text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
            }
            // Default green
            else
            {
                bg = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
                text = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
            }

            return { bg, text };
        };

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
                    child->metadata().type() == "SELECTION" &&
                    ( child->metadata().category() == "PRODUCT FUNCTION" ||
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
            bool isProductType = node.metadata().category() == "PRODUCT" && node.metadata().type() == "TYPE";
            auto [badgeBg, badgeText] = getBadgeColors( node );

            // Render tree node
            ImGui::AlignTextToFramePadding();

            char treeId[256];
            snprintf( treeId, sizeof( treeId ), "##tree_%s_%p", node.code().data(), static_cast<const void*>( &node ) );

            bool nodeOpen = false;
            bool willHaveChildren = hasVisibleChildren( node );
            
            if( willHaveChildren )
            {
                nodeOpen = ImGui::TreeNodeEx( treeId, ImGuiTreeNodeFlags_None );
                ImGui::SameLine();
            }
            else
            {
                // Leaf nodes: display bullet instead of arrow
                ImGui::TreeNodeEx(
                    treeId, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet );
                ImGui::SameLine();
            }

            // Render parent badge if provided (for nodes from skipped selections)
            if( parentNode != nullptr )
            {
                auto [parentBadgeBg, parentBadgeText] = getBadgeColors( *parentNode );

                ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
                ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
                ImGui::PushStyleColor( ImGuiCol_Button, parentBadgeBg );
                ImGui::PushStyleColor( ImGuiCol_ButtonHovered, parentBadgeBg );
                ImGui::PushStyleColor( ImGuiCol_ButtonActive, parentBadgeBg );
                ImGui::PushStyleColor( ImGuiCol_Text, parentBadgeText );

                ImGui::Button( parentNode->code().data(), ImVec2( 60.0f, 0.0f ) );

                ImGui::PopStyleColor( 4 );
                ImGui::PopStyleVar( 2 );
                ImGui::SameLine();
            }

            // Render main badge
            ImVec4 mainBadgeBg = isProductType ? ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) : badgeBg;
            ImVec4 mainBadgeText = isProductType ? ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) : badgeText;

            ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
            ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
            ImGui::PushStyleColor( ImGuiCol_Button, mainBadgeBg );
            ImGui::PushStyleColor( ImGuiCol_ButtonHovered, mainBadgeBg );
            ImGui::PushStyleColor( ImGuiCol_ButtonActive, mainBadgeBg );
            ImGui::PushStyleColor( ImGuiCol_Text, mainBadgeText );

            ImGui::Button( node.code().data(), ImVec2( 60.0f, 0.0f ) );

            ImGui::PopStyleColor( 4 );
            ImGui::PopStyleVar( 2 );

            // Render Product Type badge if node has one
            auto productTypeOpt = node.productType();
            std::string_view category = node.metadata().category();
            if( productTypeOpt.has_value() && ( category == "PRODUCT FUNCTION" || category == "ASSET FUNCTION" ) )
            {
                const auto* productTypeNode = productTypeOpt.value();
                ImGui::SameLine();
                ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 12.0f );
                ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 8.0f, 2.0f ) );
                ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
                ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
                ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.9f, 0.2f, 0.2f, 1.0f ) );
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

                ImGui::Button( productTypeNode->code().data(), ImVec2( 60.0f, 0.0f ) );

                ImGui::PopStyleColor( 4 );
                ImGui::PopStyleVar( 2 );
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
} // namespace nfx::vista
