/**
 * @file NodeDetails.cpp
 * @brief Node details panel implementation
 *
 * Displays detailed information about the selected Gmod node.
 */

#include "panels/NodeDetails.h"
#include <imgui.h>

namespace nfx::vista
{
    NodeDetails::NodeDetails( const VIS& vis )
        : m_vis{ vis }
    {
    }

    void NodeDetails::setSelectedNode( const GmodNode* node )
    {
        m_selectedNode = node;
    }

    void NodeDetails::render()
    {
        ImGui::Begin( "Node Details" );

        if( !m_selectedNode )
        {
            ImGui::TextDisabled( "No node selected" );
            ImGui::Separator();
            ImGui::TextWrapped( "Click on a node in the Gmod Viewer to see its details here." );
            ImGui::End();
            return;
        }

        const GmodNode& node = *m_selectedNode;

        // Node header
        ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.4f, 0.8f, 1.0f, 1.0f ) );
        ImGui::TextUnformatted( node.code().data() );
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::TextDisabled( "—" );
        ImGui::SameLine();

        if( node.metadata().commonName().has_value() )
        {
            ImGui::TextUnformatted( node.metadata().commonName().value().data() );
        }
        else
        {
            ImGui::TextUnformatted( node.metadata().name().data() );
        }

        ImGui::Separator();

        // Basic information
        ImGui::SeparatorText( "Basic Information" );

        ImGui::Text( "Code: %s", node.code().data() );
        ImGui::Text( "Name: %s", node.metadata().name().data() );

        if( node.metadata().commonName().has_value() )
        {
            ImGui::Text( "Common Name: %s", node.metadata().commonName().value().data() );
        }

        ImGui::Text( "Category: %s", node.metadata().category().data() );
        ImGui::Text( "Type: %s", node.metadata().type().data() );

        // Definition
        if( node.metadata().definition().has_value() )
        {
            ImGui::Spacing();
            ImGui::SeparatorText( "Definition" );
            ImGui::TextWrapped( "%s", node.metadata().definition().value().data() );
        }

        // Hierarchy information
        ImGui::Spacing();
        ImGui::SeparatorText( "Hierarchy" );

        // Parents
        if( !node.parents().isEmpty() )
        {
            ImGui::Text( "Parents: %zu", node.parents().size() );
            
            if( node.parents().size() > 4 )
            {
                if( ImGui::TreeNode( "View Parents" ) )
                {
                    ImGui::Indent();
                    for( const auto* parent : node.parents() )
                    {
                        ImGui::BulletText( "%s - %s", parent->code().data(), parent->metadata().name().data() );
                    }
                    ImGui::Unindent();
                    ImGui::TreePop();
                }
            }
            else
            {
                ImGui::Indent();
                for( const auto* parent : node.parents() )
                {
                    ImGui::BulletText( "%s - %s", parent->code().data(), parent->metadata().name().data() );
                }
                ImGui::Unindent();
            }
        }
        else
        {
            ImGui::TextDisabled( "No parents (root node)" );
        }

        ImGui::Spacing();

        // Children
        if( !node.children().isEmpty() )
        {
            ImGui::Text( "Children: %zu", node.children().size() );
            
            if( node.children().size() > 4 )
            {
                if( ImGui::TreeNode( "View Children" ) )
                {
                    for( const auto* child : node.children() )
                    {
                        ImGui::BulletText( "%s - %s", child->code().data(), child->metadata().name().data() );
                    }
                    ImGui::TreePop();
                }
            }
            else
            {
                for( const auto* child : node.children() )
                {
                    ImGui::BulletText( "%s - %s", child->code().data(), child->metadata().name().data() );
                }
            }
        }
        else
        {
            ImGui::TextDisabled( "No children (leaf node)" );
        }

        // Product Type
        if( node.productType().has_value() )
        {
            ImGui::Spacing();
            ImGui::SeparatorText( "Product Type" );
            const auto* productType = node.productType().value();
            ImGui::Text( "Code: %s", productType->code().data() );
            ImGui::Text( "Name: %s", productType->metadata().name().data() );
        }

        // Special flags
        ImGui::Spacing();
        ImGui::SeparatorText( "Properties" );

        bool isProductSelection = node.isProductSelection();
        ImGui::Text( "Product Selection: %s", isProductSelection ? "Yes" : "No" );
        if( isProductSelection )
        {
            ImGui::SameLine();
            ImGui::TextDisabled( "(Component selection)" );
        }

        ImGui::End();
    }
} // namespace nfx::vista
