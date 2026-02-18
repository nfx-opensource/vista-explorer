#include "panels/LocalIdBuilder.h"

namespace nfx::vista
{
    LocalIdBuilder::LocalIdBuilder( const VIS& vis )
        : m_vis( vis )
    {
    }

    void LocalIdBuilder::render( VisVersion version )
    {
        if( !ImGui::Begin( "LocalId Builder" ) )
        {
            ImGui::End();
            return;
        }

        ImGui::TextWrapped( "Build VIS Local IDs by selecting Gmod paths and metadata tags." );
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        renderPrimaryItemSection( version );
        ImGui::Spacing();

        renderSecondaryItemSection( version );
        ImGui::Spacing();

        renderMetadataSection();
        ImGui::Spacing();

        renderOutputSection( version );

        ImGui::End();
    }

    void LocalIdBuilder::renderPrimaryItemSection( VisVersion version )
    {
        ImGui::SeparatorText( "Primary Item (Required)" );

        ImGui::InputTextWithHint(
            "##primaryPath",
            "Enter Gmod path (e.g., 411.1-1P or 612.21/C701.13)",
            m_state.primaryPath,
            sizeof( m_state.primaryPath ) );

        if( ImGui::IsItemDeactivatedAfterEdit() )
        {
            if( m_onChanged )
            {
                m_onChanged();
            }
        }

        ImGui::SameLine();
        if( ImGui::Button( "Pick from tree##primary" ) )
        {
            if( m_currentGmodPath.has_value() )
            {
                // Use toString() to get the correct short path (only leaf nodes)
                std::string shortPath = m_currentGmodPath->toString();

                strncpy( m_state.primaryPath, shortPath.c_str(), sizeof( m_state.primaryPath ) - 1 );
                m_state.primaryPath[sizeof( m_state.primaryPath ) - 1] = '\0';

                if( m_onChanged )
                {
                    m_onChanged();
                }
            }
        }

        ImGui::SameLine();
        if( ImGui::Button( "Validate##primary" ) )
        {
            if( m_state.primaryPath[0] != '\0' )
            {
                const auto& gmod = m_vis.gmod( version );
                const auto& locations = m_vis.locations( version );
                auto gmodPathOpt = GmodPath::fromShortPath( m_state.primaryPath, gmod, locations, m_state.errors );

                if( gmodPathOpt.has_value() )
                {
                    m_state.generatedLocalId = "✓ Valid path: " + gmodPathOpt->toFullPathString();
                }

                if( m_onChanged )
                {
                    m_onChanged();
                }
            }
        }

        ImGui::TextDisabled( "Examples: 411.1, 411.1-1P, 612.21/C701.13/S93" );
    }

    void LocalIdBuilder::renderSecondaryItemSection( VisVersion version )
    {
        ImGui::SeparatorText( "Secondary Item (Optional)" );

        ImGui::Checkbox( "Add secondary item", &m_state.hasSecondaryItem );

        if( m_state.hasSecondaryItem )
        {
            ImGui::InputTextWithHint(
                "##secondaryPath",
                "Enter secondary Gmod path",
                m_state.secondaryPath,
                sizeof( m_state.secondaryPath ) );

            if( ImGui::IsItemDeactivatedAfterEdit() )
            {
                if( m_onChanged )
                {
                    m_onChanged();
                }
            }

            ImGui::SameLine();
            if( ImGui::Button( "Pick from tree##secondary" ) )
            {
                if( m_currentGmodPath.has_value() )
                {
                    std::string shortPath = m_currentGmodPath->toString();

                    strncpy( m_state.secondaryPath, shortPath.c_str(), sizeof( m_state.secondaryPath ) - 1 );
                    m_state.secondaryPath[sizeof( m_state.secondaryPath ) - 1] = '\0';

                    if( m_onChanged )
                    {
                        m_onChanged();
                    }
                }
            }

            ImGui::SameLine();
            if( ImGui::Button( "Validate##secondary" ) )
            {
                if( m_state.secondaryPath[0] != '\0' )
                {
                    const auto& gmod = m_vis.gmod( version );
                    const auto& locations = m_vis.locations( version );
                    auto gmodPathOpt =
                        GmodPath::fromShortPath( m_state.secondaryPath, gmod, locations, m_state.errors );

                    if( gmodPathOpt.has_value() )
                    {
                        m_state.generatedLocalId = "✓ Valid path: " + gmodPathOpt->toFullPathString();
                    }

                    if( m_onChanged )
                    {
                        m_onChanged();
                    }
                }
            }
        }
        else
        {
            m_state.secondaryPath[0] = '\0';
        }
    }

    void LocalIdBuilder::renderMetadataSection()
    {
        ImGui::SeparatorText( "Metadata Tags (Optional)" );

        ImGui::Columns( 2, "metadata", false );

        // Column 1
        ImGui::InputTextWithHint( "##quantity", "Quantity", m_state.quantity, sizeof( m_state.quantity ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Quantity:" );

        ImGui::InputTextWithHint( "##content", "Content", m_state.content, sizeof( m_state.content ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Content:" );

        ImGui::InputTextWithHint( "##position", "Position", m_state.position, sizeof( m_state.position ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Position:" );

        ImGui::InputTextWithHint( "##calculation", "Calculation", m_state.calculation, sizeof( m_state.calculation ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Calculation:" );

        ImGui::NextColumn();

        // Column 2
        ImGui::InputTextWithHint( "##state", "State", m_state.state, sizeof( m_state.state ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "State:" );

        ImGui::InputTextWithHint( "##command", "Command", m_state.command, sizeof( m_state.command ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Command:" );

        ImGui::InputTextWithHint( "##type", "Type", m_state.type, sizeof( m_state.type ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Type:" );

        ImGui::InputTextWithHint( "##detail", "Detail", m_state.detail, sizeof( m_state.detail ) );
        ImGui::SameLine();
        ImGui::TextDisabled( "Detail:" );

        ImGui::Columns( 1 );

        ImGui::Spacing();
        ImGui::TextDisabled( "Tip: Use Codebooks to find valid tag values" );
    }

    void LocalIdBuilder::renderOutputSection( VisVersion version )
    {
        ImGui::SeparatorText( "Generated LocalId" );

        if( ImGui::Button( "Build LocalId", ImVec2( 150, 0 ) ) )
        {
            m_state.generatedLocalId.clear();
            m_state.errors = ParsingErrors();

            try
            {
                // Parse primary path
                const auto& gmod = m_vis.gmod( version );
                const auto& locations = m_vis.locations( version );
                const auto& codebooks = m_vis.codebooks( version );

                auto primaryPathOpt = GmodPath::fromShortPath( m_state.primaryPath, gmod, locations, m_state.errors );

                // Start building LocalId
                auto builder = dnv::vista::sdk::LocalIdBuilder::create( version ).withPrimaryItem( *primaryPathOpt );

                // Add secondary item if present
                if( m_state.hasSecondaryItem && m_state.secondaryPath[0] != '\0' )
                {
                    auto secondaryPathOpt =
                        GmodPath::fromShortPath( m_state.secondaryPath, gmod, locations, m_state.errors );

                    if( secondaryPathOpt.has_value() )
                    {
                        builder = builder.withSecondaryItem( *secondaryPathOpt );
                    }
                }

                if( m_state.quantity[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Quantity].createTag( m_state.quantity );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.content[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Content].createTag( m_state.content );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.position[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Position].createTag( m_state.position );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.calculation[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Calculation].createTag( m_state.calculation );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.state[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::State].createTag( m_state.state );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.command[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Command].createTag( m_state.command );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.type[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Type].createTag( m_state.type );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                if( m_state.detail[0] != '\0' )
                {
                    auto tagOpt = codebooks[CodebookName::Detail].createTag( m_state.detail );
                    if( tagOpt.has_value() )
                    {
                        builder = builder.withMetadataTag( *tagOpt );
                    }
                }

                // Build the LocalId if no errors
                if( !m_state.errors.hasErrors() )
                {
                    auto localId = builder.build();
                    m_state.generatedLocalId = localId.toString();
                }
            }
            catch( const std::exception& e )
            {
                m_state.generatedLocalId = std::string( "Exception: " ) + e.what();
            }

            if( m_onChanged )
            {
                m_onChanged();
            }
        }

        ImGui::SameLine();
        if( ImGui::Button( "Clear", ImVec2( 100, 0 ) ) )
        {
            m_state = {};
            if( m_onChanged )
            {
                m_onChanged();
            }
        }

        ImGui::Spacing();
        if( m_state.errors.hasErrors() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ) );
            ImGui::TextWrapped( "%s", m_state.errors.toString().c_str() );
            ImGui::PopStyleColor();
        }
        else if( !m_state.generatedLocalId.empty() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ) );
            ImGui::TextWrapped( "LocalId: %s", m_state.generatedLocalId.c_str() );
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if( ImGui::Button( "Copy" ) )
            {
                ImGui::SetClipboardText( m_state.generatedLocalId.c_str() );
                if( m_onChanged )
                {
                    m_onChanged();
                }
            }
        }
    }
} // namespace nfx::vista
