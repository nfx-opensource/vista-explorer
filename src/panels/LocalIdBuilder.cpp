#include "panels/LocalIdBuilder.h"
#include <algorithm>
#include <cctype>

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
        renderMetadataInput( "##quantity", "Quantity:", m_state.quantity, sizeof( m_state.quantity ),
                             CodebookName::Quantity, m_vis.latest() );
        renderMetadataInput( "##content", "Content:", m_state.content, sizeof( m_state.content ),
                             CodebookName::Content, m_vis.latest() );
        renderMetadataInput( "##position", "Position:", m_state.position, sizeof( m_state.position ),
                             CodebookName::Position, m_vis.latest() );
        renderMetadataInput( "##calculation", "Calculation:", m_state.calculation, sizeof( m_state.calculation ),
                             CodebookName::Calculation, m_vis.latest() );

        ImGui::NextColumn();

        // Column 2
        renderMetadataInput( "##state", "State:", m_state.state, sizeof( m_state.state ), CodebookName::State,
                             m_vis.latest() );
        renderMetadataInput( "##command", "Command:", m_state.command, sizeof( m_state.command ),
                             CodebookName::Command, m_vis.latest() );
        renderMetadataInput( "##type", "Type:", m_state.type, sizeof( m_state.type ), CodebookName::Type,
                             m_vis.latest() );

        // Detail is free text, no codebook selection
        ImGui::Text( "Detail:" );
        ImGui::InputTextWithHint( "##detail", "Free text...", m_state.detail, sizeof( m_state.detail ) );

        ImGui::Columns( 1 );

        ImGui::Spacing();
        ImGui::TextDisabled( "Tip: Click in a field to see autocomplete. Use arrows to navigate, Enter to select." );
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

    void LocalIdBuilder::renderMetadataInput( const char* id, const char* label, char* buffer, size_t bufferSize,
                                              CodebookName codebook, VisVersion version )
    {
        ImGui::Text( "%s", label );

        // Input field for direct editing (custom tags)
        ImGui::PushItemWidth( ImGui::GetContentRegionAvail().x - 30 );
        ImGui::InputTextWithHint( id, "Type custom or select...", buffer, bufferSize );
        ImGui::PopItemWidth();

        // Button to open combo with standard values
        ImGui::SameLine();
        std::string buttonId = "...##" + std::string( id );
        bool openCombo = ImGui::Button( buttonId.c_str(), ImVec2( 25, 0 ) );

        // Get codebook values
        const auto& codebooks = m_vis.codebooks( version );
        const auto& cb = codebooks[codebook];
        const auto& standardValues = cb.standardValues();

        // Build sorted list
        std::vector<std::string> sorted;
        for( const auto& value : standardValues )
        {
            sorted.push_back( value );
        }
        std::sort( sorted.begin(), sorted.end() );

        // Open combo popup
        std::string popupId = "SelectMetadata##" + std::string( id );
        if( openCombo )
        {
            ImGui::OpenPopup( popupId.c_str() );
        }

        // Combo popup with filter
        if( ImGui::BeginPopup( popupId.c_str() ) )
        {
            // Get or create filter buffer for this combo
            auto& filterStr = m_comboFilters[id];
            char filterBuf[256];
            strncpy( filterBuf, filterStr.c_str(), sizeof( filterBuf ) - 1 );
            filterBuf[sizeof( filterBuf ) - 1] = '\0';

            // Filter input at top
            ImGui::SetNextItemWidth( 300 );
            if( ImGui::InputTextWithHint( "##filter", "Filter...", filterBuf, sizeof( filterBuf ) ) )
            {
                filterStr = filterBuf;
            }

            ImGui::Separator();

            // Filter and display items in a child window for scrolling
            ImGui::BeginChild( "##items", ImVec2( 300, 300 ), true );

            // Filter and display items
            std::string lowerFilter( filterBuf );
            std::transform( lowerFilter.begin(), lowerFilter.end(), lowerFilter.begin(), ::tolower );

            for( const auto& value : sorted )
            {
                // Filter
                if( lowerFilter[0] != '\0' )
                {
                    std::string lowerValue = value;
                    std::transform( lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower );
                    if( lowerValue.find( lowerFilter ) == std::string::npos )
                    {
                        continue;
                    }
                }

                bool isSelected = ( strcmp( buffer, value.c_str() ) == 0 );
                if( ImGui::Selectable( value.c_str(), isSelected ) )
                {
                    strncpy( buffer, value.c_str(), bufferSize - 1 );
                    buffer[bufferSize - 1] = '\0';
                    m_comboFilters[id].clear(); // Clear filter on selection
                    ImGui::CloseCurrentPopup();
                }
                if( isSelected )
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndChild();
            ImGui::EndPopup();
        }
        else
        {
            // Clear filter when popup is closed
            if( !ImGui::IsPopupOpen( popupId.c_str() ) )
            {
                m_comboFilters[id].clear();
            }
        }
    }
} // namespace nfx::vista
