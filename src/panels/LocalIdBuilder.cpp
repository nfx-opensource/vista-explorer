#include "panels/LocalIdBuilder.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace nfx::vista
{
    LocalIdBuilder::LocalIdBuilder( const VIS& vis )
        : m_vis{ vis }
    {
        const auto latestVersion = m_vis.latest();
        const auto& codebooks = m_vis.codebooks( latestVersion );

        for( const auto& codebookName : { CodebookName::Quantity,
                                          CodebookName::Content,
                                          CodebookName::Position,
                                          CodebookName::Calculation,
                                          CodebookName::State,
                                          CodebookName::Command,
                                          CodebookName::Type,
                                          CodebookName::Detail } )
        {
            const auto& cb = codebooks[codebookName];
            const auto& standardValues = cb.standardValues();

            std::vector<std::string> sorted;
            sorted.reserve( standardValues.size() );
            for( const auto& value : standardValues )
            {
                sorted.push_back( value );
            }
            std::sort( sorted.begin(), sorted.end() );

            m_codebookCache[codebookName] = std::move( sorted );
        }

        m_cachedVersion = latestVersion;
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

        renderLocationSection( version );
        ImGui::Spacing();

        renderMetadataSection( version );
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
            m_state.primaryPathDirty = true;
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
                std::string shortPath = m_currentGmodPath->toString();

                strncpy( m_state.primaryPath, shortPath.c_str(), sizeof( m_state.primaryPath ) - 1 );
                m_state.primaryPath[sizeof( m_state.primaryPath ) - 1] = '\0';
                m_state.primaryPathDirty = true;

                if( m_onChanged )
                {
                    m_onChanged();
                }
            }
        }

        // Reparse only when dirty
        if( m_state.primaryPathDirty )
        {
            const auto& gmod = m_vis.gmod( version );
            const auto& locations = m_vis.locations( version );
            ParsingErrors tempErrors;
            if( m_state.primaryPath[0] != '\0' )
            {
                m_state.primaryPathOpt = GmodPath::fromShortPath( m_state.primaryPath, gmod, locations, tempErrors );
            }
            else
            {
                m_state.primaryPathOpt = std::nullopt;
            }
            m_state.primaryPathDirty = false;
        }

        // Validation display
        if( m_state.primaryPath[0] != '\0' )
        {
            if( m_state.primaryPathOpt.has_value() )
            {
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ) );
                ImGui::TextWrapped( "[OK] %s", m_state.primaryPathOpt->node().metadata().name().data() );
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ) );
                ImGui::Text( "[X] Invalid" );
                ImGui::PopStyleColor();
            }
        }

        ImGui::TextDisabled( "Examples: 411.1, 411.1-1P, 612.21/C701.13/S93" );
    }

    void LocalIdBuilder::renderSecondaryItemSection( VisVersion version )
    {
        bool isOpen = ImGui::CollapsingHeader( "Secondary Item (Optional)" );

        m_state.hasSecondaryItem = isOpen;

        if( !isOpen )
        {
            return;
        }

        ImGui::InputTextWithHint(
            "##secondaryPath", "Enter secondary Gmod path", m_state.secondaryPath, sizeof( m_state.secondaryPath ) );

        if( ImGui::IsItemDeactivatedAfterEdit() )
        {
            m_state.secondaryPathDirty = true;
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
                m_state.secondaryPathDirty = true;
                if( m_onChanged )
                {
                    m_onChanged();
                }
            }
        }

        // Reparse only when dirty
        if( m_state.secondaryPathDirty )
        {
            const auto& gmod = m_vis.gmod( version );
            const auto& locations = m_vis.locations( version );
            ParsingErrors tempErrors;
            if( m_state.secondaryPath[0] != '\0' )
            {
                m_state.secondaryPathOpt =
                    GmodPath::fromShortPath( m_state.secondaryPath, gmod, locations, tempErrors );
            }
            else
            {
                m_state.secondaryPathOpt = std::nullopt;
            }
            m_state.secondaryPathDirty = false;
        }

        // Validation display
        if( m_state.secondaryPath[0] != '\0' )
        {
            if( m_state.secondaryPathOpt.has_value() )
            {
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ) );
                ImGui::TextWrapped( "[OK] %s", m_state.secondaryPathOpt->node().metadata().name().data() );
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ) );
                ImGui::Text( "[X] Invalid" );
                ImGui::PopStyleColor();
            }
        }
    }

    void LocalIdBuilder::renderLocationSection( VisVersion version )
    {
        if( !ImGui::CollapsingHeader( "Location (Optional)" ) )
        {
            return;
        }

        const auto& locations = m_vis.locations( version );

        // Build current location string using LocationBuilder
        std::string builtLocation;
        bool hasAnyComponent = false;

        try
        {
            auto lb = LocationBuilder::create( locations );

            if( m_state.locationNumber > 0 )
            {
                lb = std::move( lb ).withNumber( m_state.locationNumber );
                hasAnyComponent = true;
            }
            if( m_state.locationSide != 0 )
            {
                lb = std::move( lb ).withSide( m_state.locationSide );
                hasAnyComponent = true;
            }
            if( m_state.locationVertical != 0 )
            {
                lb = std::move( lb ).withVertical( m_state.locationVertical );
                hasAnyComponent = true;
            }
            if( m_state.locationTransverse != 0 )
            {
                lb = std::move( lb ).withTransverse( m_state.locationTransverse );
                hasAnyComponent = true;
            }
            if( m_state.locationLong != 0 )
            {
                lb = std::move( lb ).withLongitudinal( m_state.locationLong );
                hasAnyComponent = true;
            }

            if( hasAnyComponent )
            {
                builtLocation = lb.build().value();
            }
        }
        catch( ... )
        {
            builtLocation.clear();
        }

        // Helper: render a group of toggle buttons on a new indented line
        // Returns true if the value changed.
        auto renderToggleGroup =
            [&]( const char* label, char& stateVal, LocationGroup group, const char* btnSuffix, const char* clearId ) {
                ImGui::Text( "%s:", label );
                ImGui::Indent();
                const auto& groups = locations.groups();
                auto it = groups.find( group );
                if( it != groups.end() )
                {
                    for( const auto& relLoc : it->second )
                    {
                        bool selected = ( stateVal == relLoc.code() );
                        if( selected )
                        {
                            ImGui::PushStyleColor( ImGuiCol_Button, ImGui::GetStyleColorVec4( ImGuiCol_ButtonActive ) );
                        }
                        std::string btnLabel =
                            std::string( 1, relLoc.code() ) + "  " + relLoc.name() + "##" + btnSuffix;
                        if( ImGui::Button( btnLabel.c_str() ) )
                        {
                            stateVal = selected ? 0 : relLoc.code();
                        }
                        if( selected )
                        {
                            ImGui::PopStyleColor();
                        }
                        ImGui::SameLine();
                    }
                }
                if( stateVal != 0 )
                {
                    if( ImGui::SmallButton( clearId ) )
                    {
                        stateVal = 0;
                    }
                }
                ImGui::Unindent();
                ImGui::Spacing();
            };

        // --- Number ---
        ImGui::Text( "Number:" );
        ImGui::Indent();
        ImGui::SetNextItemWidth( 60 );
        if( ImGui::InputInt( "##locNumber", &m_state.locationNumber, 0, 0 ) )
        {
            if( m_state.locationNumber < 0 )
            {
                m_state.locationNumber = 0;
            }
        }
        if( m_state.locationNumber > 0 )
        {
            ImGui::SameLine();
            if( ImGui::SmallButton( "x##locNum" ) )
            {
                m_state.locationNumber = 0;
            }
        }
        ImGui::Unindent();
        ImGui::Spacing();

        // --- Side / Vertical / Transverse / Longitudinal ---
        renderToggleGroup( "Side", m_state.locationSide, LocationGroup::Side, "side", "x##locSide" );
        renderToggleGroup( "Vertical", m_state.locationVertical, LocationGroup::Vertical, "vert", "x##locVert" );
        renderToggleGroup(
            "Transverse", m_state.locationTransverse, LocationGroup::Transverse, "trans", "x##locTrans" );
        renderToggleGroup( "Longitudinal", m_state.locationLong, LocationGroup::Longitudinal, "longi", "x##locLong" );

        ImGui::Separator();
        ImGui::Spacing();

        // Apply location to the correct individualizable node in the path using the SDK
        auto applyLocation = [&]( char* pathBuf, size_t bufSize ) {
            const auto& gmod = m_vis.gmod( version );
            ParsingErrors tempErr;

            auto pathOpt = GmodPath::fromShortPath( pathBuf, gmod, locations, tempErr );
            if( !pathOpt.has_value() )
            {
                // Can't parse — raw text fallback: put location on first segment
                std::string pathStr( pathBuf );
                auto slashPos = pathStr.find( '/' );
                std::string firstSeg = ( slashPos == std::string::npos ) ? pathStr : pathStr.substr( 0, slashPos );
                std::string rest = ( slashPos == std::string::npos ) ? "" : pathStr.substr( slashPos );
                auto dashPos = firstSeg.rfind( '-' );
                if( dashPos != std::string::npos )
                {
                    firstSeg = firstSeg.substr( 0, dashPos );
                }
                std::string newPath = firstSeg + "-" + builtLocation + rest;
                strncpy( pathBuf, newPath.c_str(), bufSize - 1 );
                pathBuf[bufSize - 1] = '\0';
                if( m_onChanged )
                {
                    m_onChanged();
                }
                return;
            }

            // Get the clean path string (no locations) using the SDK
            std::string cleanPath = pathOpt->withoutLocations().toString();

            // Find the first individualizable set to know which segment gets the location
            auto sets = pathOpt->individualizableSets();

            std::string newPath;
            if( sets.empty() )
            {
                // No individualizable set: just use the clean path as-is (no location applicable)
                newPath = cleanPath;
            }
            else
            {
                // targetIdx is a fullPath index. Find what short-path segment it maps to.
                // Build a mapping: fullPath index -> short-path segment index (0-based)
                int shortSegIdx = 0; // which segment in cleanPath gets the location
                int shortSeg = 0;
                const GmodPath& constPath = *pathOpt;
                for( size_t i = 0; i < constPath.length(); ++i )
                {
                    const auto& n = constPath[i];
                    if( !n.isLeafNode() && i != constPath.length() - 1 )
                    {
                        continue;
                    }
                    if( (int)i == sets[0].nodeIndices().front() )
                    {
                        shortSegIdx = shortSeg;
                        break;
                    }
                    shortSeg++;
                }

                // Split cleanPath by '/', insert location on shortSegIdx-th segment
                std::istringstream ss( cleanPath );
                std::string seg;
                int segN = 0;
                while( std::getline( ss, seg, '/' ) )
                {
                    if( !newPath.empty() )
                    {
                        newPath += "/";
                    }
                    newPath += seg;
                    if( segN == shortSegIdx )
                    {
                        newPath += "-" + builtLocation;
                    }
                    segN++;
                }
            }

            strncpy( pathBuf, newPath.c_str(), bufSize - 1 );
            pathBuf[bufSize - 1] = '\0';
            if( m_onChanged )
            {
                m_onChanged();
            }
        };

        // --- Preview ---
        if( hasAnyComponent && !builtLocation.empty() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.9f, 0.8f, 0.2f, 1.0f ) );

            // Show compact code
            ImGui::Text( "Location: " );
            ImGui::SameLine();
            ImGui::Text( "-%s", builtLocation.c_str() );

            // Show human-readable breakdown
            ImGui::SameLine();
            ImGui::TextDisabled( "  (" );
            if( m_state.locationNumber > 0 )
            {
                ImGui::SameLine( 0, 0 );
                ImGui::TextDisabled( "#%d", m_state.locationNumber );
            }
            if( m_state.locationSide != 0 )
            {
                ImGui::SameLine( 0, 4 );
                const char* sideName = m_state.locationSide == 'P'   ? "Port"
                                       : m_state.locationSide == 'S' ? "Starboard"
                                                                     : "Centre";
                ImGui::TextDisabled( "%s", sideName );
            }
            if( m_state.locationVertical != 0 )
            {
                ImGui::SameLine( 0, 4 );
                const char* vertName = m_state.locationVertical == 'U'   ? "Upper"
                                       : m_state.locationVertical == 'L' ? "Lower"
                                                                         : "Middle";
                ImGui::TextDisabled( "%s", vertName );
            }
            if( m_state.locationTransverse != 0 )
            {
                ImGui::SameLine( 0, 4 );
                ImGui::TextDisabled( "%s", m_state.locationTransverse == 'I' ? "Inner" : "Outer" );
            }
            if( m_state.locationLong != 0 )
            {
                ImGui::SameLine( 0, 4 );
                ImGui::TextDisabled( "%s", m_state.locationLong == 'F' ? "Forward" : "Aft" );
            }
            ImGui::SameLine( 0, 0 );
            ImGui::TextDisabled( ")" );

            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::TextDisabled( "(no location)" );
        }

        ImGui::Spacing();

        // --- Apply buttons ---
        if( hasAnyComponent && !builtLocation.empty() )
        {
            if( ImGui::Button( "Apply to Primary" ) )
            {
                if( m_state.primaryPath[0] != '\0' )
                {
                    applyLocation( m_state.primaryPath, sizeof( m_state.primaryPath ) );
                }
            }

            if( m_state.hasSecondaryItem && m_state.secondaryPath[0] != '\0' )
            {
                ImGui::SameLine();
                if( ImGui::Button( "Apply to Secondary" ) )
                {
                    applyLocation( m_state.secondaryPath, sizeof( m_state.secondaryPath ) );
                }
            }

            ImGui::SameLine();
        }

        // --- Reset (always visible) ---
        if( ImGui::Button( "Reset Location" ) )
        {
            m_state.locationNumber = 0;
            m_state.locationSide = 0;
            m_state.locationVertical = 0;
            m_state.locationTransverse = 0;
            m_state.locationLong = 0;
        }
    }

    void LocalIdBuilder::renderMetadataSection( VisVersion version )
    {
        ImGui::SeparatorText( "Metadata Tags" );

        // Rebuild codebook cache if version changed
        if( m_cachedVersion.value() != version )
        {
            m_codebookCache.clear();
            const auto& codebooks = m_vis.codebooks( version );

            for( const auto& codebookName : { CodebookName::Quantity,
                                              CodebookName::Content,
                                              CodebookName::Position,
                                              CodebookName::Calculation,
                                              CodebookName::State,
                                              CodebookName::Command,
                                              CodebookName::Type,
                                              CodebookName::Detail } )
            {
                const auto& cb = codebooks[codebookName];
                const auto& standardValues = cb.standardValues();

                std::vector<std::string> sorted;
                sorted.reserve( standardValues.size() );
                for( const auto& value : standardValues )
                    sorted.push_back( value );
                std::sort( sorted.begin(), sorted.end() );

                m_codebookCache[codebookName] = std::move( sorted );
            }

            m_cachedVersion = version;
        }

        ImGui::Columns( 2, "metadata", false );

        // Column 1
        renderMetadataInput(
            "##quantity", "Quantity", m_state.quantity, sizeof( m_state.quantity ), CodebookName::Quantity );
        ImGui::Spacing();
        renderMetadataInput(
            "##content", "Content", m_state.content, sizeof( m_state.content ), CodebookName::Content );
        ImGui::Spacing();
        renderMetadataInput(
            "##position", "Position", m_state.position, sizeof( m_state.position ), CodebookName::Position );
        ImGui::Spacing();
        renderMetadataInput(
            "##calculation",
            "Calculation",
            m_state.calculation,
            sizeof( m_state.calculation ),
            CodebookName::Calculation );

        ImGui::NextColumn();

        // Column 2
        renderMetadataInput( "##state", "State", m_state.state, sizeof( m_state.state ), CodebookName::State );
        ImGui::Spacing();
        renderMetadataInput(
            "##command", "Command", m_state.command, sizeof( m_state.command ), CodebookName::Command );
        ImGui::Spacing();
        renderMetadataInput( "##type", "Type", m_state.type, sizeof( m_state.type ), CodebookName::Type );
        ImGui::Spacing();

        ImGui::TextDisabled( "Detail" );
        ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x );
        ImGui::InputTextWithHint( "##detail", "Free text...", m_state.detail, sizeof( m_state.detail ) );

        ImGui::Columns( 1 );
    }

    void LocalIdBuilder::renderOutputSection( VisVersion version )
    {
        ImGui::SeparatorText( "Generated LocalId" );

        // Checkbox for verbose mode
        ImGui::Checkbox( "Verbose mode (include common names)", &m_state.verboseMode );
        ImGui::SameLine();
        ImGui::TextDisabled( "(?)" );
        if( ImGui::IsItemHovered() )
        {
            ImGui::SetTooltip( "Include human-readable node names in the LocalId" );
        }

        m_state.generatedLocalId.clear();
        m_state.errors = ParsingErrors();

        const auto& codebooks = m_vis.codebooks( version );

        // Helper lambda to add metadata tag with correct separator (- for standard, ~ for custom)
        // Detail always uses '-' regardless of value
        auto addMetadataTag =
            [&codebooks]( std::string& str, const char* prefix, const char* value, CodebookName codebookName ) {
                bool isCustom = false;

                if( codebookName != CodebookName::Detail )
                {
                    const auto& codebook = codebooks[codebookName];
                    const auto& standardValues = codebook.standardValues();

                    isCustom = true;
                    for( const auto& standardValue : standardValues )
                    {
                        if( standardValue == value )
                        {
                            isCustom = false;
                            break;
                        }
                    }
                }

                str += "/" + std::string( prefix ) + ( isCustom ? "~" : "-" ) + std::string( value );
            };

        // Use cached parsed paths
        const auto& primaryPathOpt = m_state.primaryPathOpt;
        const auto& secondaryPathOpt = m_state.secondaryPathOpt;

        // Build LocalId string
        std::string localIdStr;

        // Use SDK builder if all paths are valid (to support verbose mode)
        if( primaryPathOpt.has_value() &&
            ( !m_state.hasSecondaryItem || m_state.secondaryPath[0] == '\0' || secondaryPathOpt.has_value() ) )
        {
            auto builder = dnv::vista::sdk::LocalIdBuilder::create( version ).withVerboseMode( m_state.verboseMode );
            builder = std::move( builder ).withPrimaryItem( *primaryPathOpt );

            if( secondaryPathOpt.has_value() )
            {
                builder = std::move( builder ).withSecondaryItem( *secondaryPathOpt );
            }

            localIdStr = builder.toString();
        }
        else
        {
            // Build manually if any path is invalid
            localIdStr = "/dnv-v2/vis-" + std::string( VisVersions::toString( version ) );

            if( m_state.primaryPath[0] != '\0' )
            {
                localIdStr += "/" + std::string( m_state.primaryPath );
            }

            if( m_state.hasSecondaryItem && m_state.secondaryPath[0] != '\0' )
            {
                localIdStr += "/sec/" + std::string( m_state.secondaryPath );
            }
        }

        // Add metadata section
        bool hasMetadata = false;
        std::string metadataStr;

        if( m_state.quantity[0] != '\0' )
        {
            addMetadataTag( metadataStr, "qty", m_state.quantity, CodebookName::Quantity );
            hasMetadata = true;
        }

        if( m_state.content[0] != '\0' )
        {
            addMetadataTag( metadataStr, "cnt", m_state.content, CodebookName::Content );
            hasMetadata = true;
        }

        if( m_state.calculation[0] != '\0' )
        {
            addMetadataTag( metadataStr, "calc", m_state.calculation, CodebookName::Calculation );
            hasMetadata = true;
        }

        if( m_state.state[0] != '\0' )
        {
            addMetadataTag( metadataStr, "state", m_state.state, CodebookName::State );
            hasMetadata = true;
        }

        if( m_state.command[0] != '\0' )
        {
            addMetadataTag( metadataStr, "cmd", m_state.command, CodebookName::Command );
            hasMetadata = true;
        }

        if( m_state.type[0] != '\0' )
        {
            addMetadataTag( metadataStr, "type", m_state.type, CodebookName::Type );
            hasMetadata = true;
        }

        if( m_state.position[0] != '\0' )
        {
            addMetadataTag( metadataStr, "pos", m_state.position, CodebookName::Position );
            hasMetadata = true;
        }

        if( m_state.detail[0] != '\0' )
        {
            addMetadataTag( metadataStr, "detail", m_state.detail, CodebookName::Detail );
            hasMetadata = true;
        }

        // Add /meta section if not already present
        if( m_state.primaryPath[0] != '\0' )
        {
            // Check if /meta is already in the string (from SDK builder)
            if( localIdStr.find( "/meta" ) == std::string::npos )
            {
                localIdStr += "/meta";
            }

            if( hasMetadata )
            {
                localIdStr += metadataStr;
            }
        }

        m_state.generatedLocalId = localIdStr;

        // Validate using SDK's fromString() to get detailed errors
        if( !m_state.generatedLocalId.empty() )
        {
            (void)LocalId::fromString( m_state.generatedLocalId, m_state.errors );
        }

        ImGui::Spacing(); // LocalId in a read-only input field (always present)
        char localIdBuffer[1024];
        strncpy( localIdBuffer, m_state.generatedLocalId.c_str(), sizeof( localIdBuffer ) - 1 );
        localIdBuffer[sizeof( localIdBuffer ) - 1] = '\0';

        ImGui::PushItemWidth( -220 );
        ImGui::InputText( "##localIdOutput", localIdBuffer, sizeof( localIdBuffer ), ImGuiInputTextFlags_ReadOnly );
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if( ImGui::Button( "Copy", ImVec2( 100, 0 ) ) )
        {
            if( !m_state.generatedLocalId.empty() )
            {
                ImGui::SetClipboardText( m_state.generatedLocalId.c_str() );
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

        // Show validation status
        if( m_state.errors.hasErrors() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.3f, 0.3f, 1.0f ) );
            ImGui::Text( "Invalid Local ID" );
            ImGui::Spacing();
            ImGui::Indent();
            for( const auto& [type, message] : m_state.errors )
            {
                ImGui::TextWrapped( "%s", message.c_str() );
            }
            ImGui::Unindent();
            ImGui::PopStyleColor();
        }
        else if( !m_state.generatedLocalId.empty() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ) );
            ImGui::Text( "Valid LocalId" );
            ImGui::PopStyleColor();
        }
    }

    void LocalIdBuilder::renderMetadataInput(
        const char* id, const char* label, char* buffer, size_t bufferSize, CodebookName codebook )
    {
        ImGui::TextDisabled( "%s", label );

        // Input field for direct editing
        const float arrowButtonWidth = ImGui::GetFrameHeight();
        ImGui::PushItemWidth( ImGui::GetContentRegionAvail().x - arrowButtonWidth - ImGui::GetStyle().ItemSpacing.x );
        ImGui::InputTextWithHint( id, "Type custom or select...", buffer, bufferSize );
        ImGui::PopItemWidth();

        // Button to open combo with standard values
        ImGui::SameLine();
        std::string buttonId = "##btn_" + std::string( id );
        bool openCombo = ImGui::ArrowButton( buttonId.c_str(), ImGuiDir_Down );

        // Get cached sorted codebook
        const auto& cachedCodebook = m_codebookCache[codebook];

        // Open combo popup
        std::string popupId = "SelectMetadata##" + std::string( id );
        if( openCombo )
        {
            ImGui::OpenPopup( popupId.c_str() );
            if( m_onChanged )
            {
                m_onChanged();
            }
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

            for( const auto& value : cachedCodebook )
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
