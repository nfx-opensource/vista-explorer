#include "panels/ProjectManager.h"
#include "Theme.h"

#include <dnv/vista/sdk/ImoNumber.h>
#include <dnv/vista/sdk/transport/ShipId.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace nfx::vista
{
    ProjectManager::ProjectManager()
    {
        const auto dir = ProjectSerializer::defaultDir();
        m_saveAsPath = ( dir / "project.json" ).string();
        m_openPath = m_saveAsPath;
        m_browserCurrentDir = dir;
        m_browserFileName = "project.json";
    }

    void ProjectManager::refreshBrowserEntries()
    {
        m_browserEntries.clear();

        std::error_code ec;

        // Add parent directory entry if not at root
        if( m_browserCurrentDir.has_parent_path() && m_browserCurrentDir != m_browserCurrentDir.root_path() )
        {
            m_browserEntries.push_back( { "..", m_browserCurrentDir.parent_path(), true } );
        }

        std::vector<BrowserEntry> dirs;
        std::vector<BrowserEntry> files;

        for( const auto& entry : std::filesystem::directory_iterator( m_browserCurrentDir, ec ) )
        {
            if( ec )
            {
                break;
            }

            const bool isDir = entry.is_directory( ec );
            if( ec )
            {
                ec.clear();
                continue;
            }

            if( isDir )
            {
                dirs.push_back( { entry.path().filename().string(), entry.path(), true } );
            }
            else if( entry.path().extension() == ".json" )
            {
                files.push_back( { entry.path().filename().string(), entry.path(), false } );
            }
        }

        std::sort(
            dirs.begin(), dirs.end(), []( const BrowserEntry& a, const BrowserEntry& b ) { return a.name < b.name; } );
        std::sort( files.begin(), files.end(), []( const BrowserEntry& a, const BrowserEntry& b ) {
            return a.name < b.name;
        } );

        for( auto& d : dirs )
        {
            m_browserEntries.push_back( std::move( d ) );
        }
        for( auto& f : files )
        {
            m_browserEntries.push_back( std::move( f ) );
        }

        m_browserDirty = false;
    }

    const Project* ProjectManager::activeProject() const
    {
        if( !m_activeProject.has_value() )
        {
            return nullptr;
        }
        return &m_activeProject.value();
    }

    Project* ProjectManager::activeProject()
    {
        if( !m_activeProject.has_value() )
        {
            return nullptr;
        }
        return &m_activeProject.value();
    }

    void ProjectManager::render()
    {
        if( !ImGui::Begin( "Project Manager" ) )
        {
            ImGui::End();
            return;
        }

        renderToolbar();
        ImGui::Separator();

        if( m_activeProject.has_value() )
        {
            renderProjectInfo();
            ImGui::Spacing();
            renderParticulars();
        }
        else
        {
            ImGui::TextDisabled( "No project open. Use New or Open." );
        }

        ImGui::Separator();
        renderStatusBar();

        renderNewProjectDialog();
        renderOpenDialog();
        renderSaveAsDialog();

        ImGui::End();
    }

    void ProjectManager::renderToolbar()
    {
        if( ImGui::Button( "New" ) )
        {
            m_showNewDialog = true;
            m_newName.clear();
            m_newShipId.clear();
            m_newShipIdValid = false;
            m_newShipIdError.clear();
            ImGui::OpenPopup( "New Project" );
        }

        ImGui::SameLine();

        if( ImGui::Button( "Open" ) )
        {
            m_showOpenDialog = true;
            m_browserCurrentDir = ProjectSerializer::defaultDir();
            m_browserFileName.clear();
            m_browserDirty = true;
            ImGui::OpenPopup( "Open Project" );
        }

        ImGui::SameLine();

        const bool hasProject = m_activeProject.has_value();

        if( !hasProject )
        {
            ImGui::BeginDisabled();
        }

        if( ImGui::Button( "Save" ) )
        {
            if( hasProject )
            {
                if( m_activeProject->filePath.empty() )
                {
                    m_showSaveAsDialog = true;
                    m_browserCurrentDir = ProjectSerializer::defaultDir();
                    m_browserFileName = m_activeProject->name + "-" + m_activeProject->shipId.toString() + ".json";
                    m_browserDirty = true;
                    ImGui::OpenPopup( "Save As" );
                }
                else
                {
                    doSave( m_activeProject->filePath );
                }
            }
        }

        ImGui::SameLine();

        if( ImGui::Button( "Save As" ) )
        {
            m_showSaveAsDialog = true;
            m_browserCurrentDir = ProjectSerializer::defaultDir();
            m_browserFileName = m_activeProject.has_value()
                                    ? ( m_activeProject->name + "-" + m_activeProject->shipId.toString() + ".json" )
                                    : "project.json";
            m_browserDirty = true;
            ImGui::OpenPopup( "Save As" );
        }

        if( !hasProject )
        {
            ImGui::EndDisabled();
        }
    }

    void ProjectManager::renderNewProjectDialog()
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos( center, ImGuiCond_Appearing, ImVec2( 0.5f, 0.5f ) );
        ImGui::SetNextWindowSize( ImVec2( 400, 200 ), ImGuiCond_Appearing );

        if( !ImGui::BeginPopupModal( "New Project", nullptr, ImGuiWindowFlags_NoResize ) )
        {
            return;
        }

        ImGui::Text( "Project name" );
        ImGui::SetNextItemWidth( -1 );
        ImGui::InputText( "##newName", &m_newName );

        ImGui::Spacing();

        ImGui::Text( "Ship ID (IMO number or free text)" );
        ImGui::SetNextItemWidth( -1 );

        if( ImGui::InputText( "##newShipId", &m_newShipId ) )
        {
            const std::string_view sv( m_newShipId );
            if( sv.empty() )
            {
                m_newShipIdValid = false;
                m_newShipIdError = "Ship ID is required.";
            }
            else
            {
                // If it looks like an IMO number (digits only or "IMO" prefix), validate checksum
                bool looksLikeImo = sv.starts_with( "IMO" ) || sv.starts_with( "imo" );
                if( !looksLikeImo )
                {
                    looksLikeImo = !sv.empty() && std::all_of( sv.begin(), sv.end(), ::isdigit );
                }

                if( looksLikeImo )
                {
                    auto imo = dnv::vista::sdk::ImoNumber::fromString( sv );
                    if( imo.has_value() )
                    {
                        m_newShipIdValid = true;
                        m_newShipIdError.clear();
                    }
                    else
                    {
                        m_newShipIdValid = false;
                        m_newShipIdError = "Invalid IMO number (bad checksum or format).";
                    }
                }
                else
                {
                    // Free-text other ID — always valid as long as non-empty
                    m_newShipIdValid = true;
                    m_newShipIdError.clear();
                }
            }
        }

        if( !m_newShipIdError.empty() )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, Theme::TextError );
            ImGui::TextWrapped( "%s", m_newShipIdError.c_str() );
            ImGui::PopStyleColor();
        }
        else if( m_newShipIdValid )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, Theme::TextSuccess );
            ImGui::Text( "[OK]" );
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const bool canCreate = m_newShipIdValid && !m_newName.empty();

        if( !canCreate )
        {
            ImGui::BeginDisabled();
        }

        if( ImGui::Button( "Create", ImVec2( 120, 0 ) ) )
        {
            const std::string_view sv( m_newShipId );

            // Build ShipId — if it parses as a valid IMO, use ImoNumber constructor
            // so that toString() returns "IMO1234567" and not a bare number
            std::optional<dnv::vista::sdk::ShipId> shipId;
            auto imo = dnv::vista::sdk::ImoNumber::fromString( sv );
            if( imo.has_value() )
            {
                shipId = dnv::vista::sdk::ShipId( *imo );
            }
            else
            {
                shipId = dnv::vista::sdk::ShipId::fromString( sv );
            }

            if( shipId.has_value() )
            {
                Project p{ m_newName, std::move( *shipId ) };
                p.isDirty = true;

                // Build default save path from name and shipId
                const auto dir = ProjectSerializer::defaultDir();
                const auto filename = m_newName + "-" + p.shipId.toString() + ".json";
                m_saveAsPath = ( dir / filename ).string();
                m_openPath = m_saveAsPath;

                m_activeProject = std::move( p );
                m_statusMessage = "New project created.";
                m_statusIsError = false;

                notifyChanged();
                ImGui::CloseCurrentPopup();
            }
        }

        if( !canCreate )
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if( ImGui::Button( "Cancel", ImVec2( 120, 0 ) ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void ProjectManager::renderOpenDialog()
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos( center, ImGuiCond_Appearing, ImVec2( 0.5f, 0.5f ) );
        ImGui::SetNextWindowSize( ImVec2( 600, 400 ), ImGuiCond_Appearing );

        if( !ImGui::BeginPopupModal( "Open Project", nullptr, ImGuiWindowFlags_NoResize ) )
        {
            return;
        }

        if( m_browserDirty )
        {
            refreshBrowserEntries();
        }

        // Current directory display
        ImGui::TextDisabled( "Directory:" );
        ImGui::SameLine();
        ImGui::TextUnformatted( m_browserCurrentDir.string().c_str() );
        ImGui::Separator();

        // File list
        const float listHeight = 270.0f;
        ImGui::BeginChild( "##browserList", ImVec2( 0, listHeight ), true );

        for( const auto& entry : m_browserEntries )
        {
            const std::string label = entry.isDirectory ? ( "[DIR]  " + entry.name ) : ( "       " + entry.name );

            bool selected = !entry.isDirectory && ( m_browserFileName == entry.name );

            if( ImGui::Selectable( label.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick ) )
            {
                if( entry.isDirectory )
                {
                    if( ImGui::IsMouseDoubleClicked( 0 ) )
                    {
                        m_browserCurrentDir = entry.fullPath;
                        m_browserDirty = true;
                        m_browserFileName.clear();
                    }
                }
                else
                {
                    m_browserFileName = entry.name;
                }
            }
        }

        ImGui::EndChild();

        // Selected file field
        ImGui::Spacing();
        ImGui::Text( "File:" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( -1 );
        ImGui::InputText( "##openFileName", &m_browserFileName );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const bool canOpen = !m_browserFileName.empty();

        if( !canOpen )
        {
            ImGui::BeginDisabled();
        }

        if( ImGui::Button( "Open", ImVec2( 120, 0 ) ) )
        {
            const auto fullPath = m_browserCurrentDir / m_browserFileName;
            doLoad( fullPath.string() );
            ImGui::CloseCurrentPopup();
        }

        if( !canOpen )
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if( ImGui::Button( "Cancel", ImVec2( 120, 0 ) ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void ProjectManager::renderSaveAsDialog()
    {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos( center, ImGuiCond_Appearing, ImVec2( 0.5f, 0.5f ) );
        ImGui::SetNextWindowSize( ImVec2( 600, 420 ), ImGuiCond_Appearing );

        if( !ImGui::BeginPopupModal( "Save As", nullptr, ImGuiWindowFlags_NoResize ) )
        {
            return;
        }

        if( m_browserDirty )
        {
            refreshBrowserEntries();
        }

        // Current directory display
        ImGui::TextDisabled( "Directory:" );
        ImGui::SameLine();
        ImGui::TextUnformatted( m_browserCurrentDir.string().c_str() );
        ImGui::Separator();

        // File list
        const float listHeight = 250.0f;
        ImGui::BeginChild( "##browserListSave", ImVec2( 0, listHeight ), true );

        for( const auto& entry : m_browserEntries )
        {
            const std::string label = entry.isDirectory ? ( "[DIR]  " + entry.name ) : ( "       " + entry.name );

            bool selected = !entry.isDirectory && ( m_browserFileName == entry.name );

            if( ImGui::Selectable( label.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick ) )
            {
                if( entry.isDirectory )
                {
                    if( ImGui::IsMouseDoubleClicked( 0 ) )
                    {
                        m_browserCurrentDir = entry.fullPath;
                        m_browserDirty = true;
                    }
                }
                else
                {
                    m_browserFileName = entry.name;
                }
            }
        }

        ImGui::EndChild();

        // Editable filename
        ImGui::Spacing();
        ImGui::Text( "File name:" );
        ImGui::SameLine();
        ImGui::SetNextItemWidth( -1 );
        ImGui::InputText( "##saveFileName", &m_browserFileName );

        // Full path preview
        ImGui::Spacing();
        const auto fullPath = m_browserCurrentDir / m_browserFileName;
        ImGui::TextDisabled( "%s", fullPath.string().c_str() );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const bool canSave = !m_browserFileName.empty();

        if( !canSave )
        {
            ImGui::BeginDisabled();
        }

        if( ImGui::Button( "Save", ImVec2( 120, 0 ) ) )
        {
            doSave( fullPath.string() );
            ImGui::CloseCurrentPopup();
        }

        if( !canSave )
        {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if( ImGui::Button( "Cancel", ImVec2( 120, 0 ) ) )
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    void ProjectManager::renderProjectInfo()
    {
        auto& p = m_activeProject.value();

        ImGui::SeparatorText( "Project" );

        ImGui::Text( "Name" );
        ImGui::SameLine( 160 );
        ImGui::SetNextItemWidth( -1 );
        if( ImGui::InputText( "##projName", &p.name ) )
        {
            p.isDirty = true;
            notifyChanged();
        }

        ImGui::Text( "Ship ID" );
        ImGui::SameLine( 160 );

        // ShipId is immutable after creation — display only
        ImGui::TextDisabled( "%s", p.shipId.toString().c_str() );

        ImGui::Text( "File" );
        ImGui::SameLine( 160 );
        if( p.filePath.empty() )
        {
            ImGui::TextDisabled( "(not saved)" );
        }
        else
        {
            ImGui::TextDisabled( "%s", p.filePath.c_str() );
        }
    }

    void ProjectManager::renderParticulars()
    {
        if( !ImGui::CollapsingHeader( "Ship Particulars" ) )
        {
            return;
        }

        auto& p = m_activeProject.value();
        auto& s = p.particulars;

        bool changed = false;

        auto strField = [&]( const char* label, std::string& value, const char* hint = "" ) {
            ImGui::Text( "%s", label );
            ImGui::SameLine( 200 );
            ImGui::SetNextItemWidth( -1 );
            const std::string id = std::string( "##" ) + label;
            if( ImGui::InputTextWithHint( id.c_str(), hint, &value ) )
            {
                changed = true;
            }
        };

        auto optDoubleField = [&]( const char* label, std::optional<double>& value, const char* unit = "" ) {
            ImGui::Text( "%s", label );
            ImGui::SameLine( 200 );
            ImGui::SetNextItemWidth( -1 );
            double v = value.value_or( 0.0 );
            const std::string id = std::string( "##" ) + label;
            if( ImGui::InputDouble( id.c_str(), &v, 0.0, 0.0, "%.2f" ) )
            {
                value = v;
                changed = true;
            }
            if( unit[0] != '\0' )
            {
                ImGui::SameLine();
                ImGui::TextDisabled( "%s", unit );
            }
        };

        auto optIntField = [&]( const char* label, std::optional<int>& value ) {
            ImGui::Text( "%s", label );
            ImGui::SameLine( 200 );
            ImGui::SetNextItemWidth( -1 );
            int v = value.value_or( 0 );
            const std::string id = std::string( "##" ) + label;
            if( ImGui::InputInt( id.c_str(), &v ) )
            {
                value = v;
                changed = true;
            }
        };

        ImGui::SeparatorText( "Identification" );

        // Ship ID — read-only, set at project creation
        ImGui::Text( "Ship ID" );
        ImGui::SameLine( 200 );
        ImGui::SetNextItemWidth( -1 );
        ImGui::BeginDisabled();
        std::string shipIdStr = p.shipId.toString();
        ImGui::InputText( "##shipIdRO", &shipIdStr );
        ImGui::EndDisabled();

        strField( "Vessel name", s.vesselName );
        strField( "Call sign", s.callSign );
        strField( "MMSI", s.mmsi, "9 digits" );
        strField( "Flag state", s.flagState, "ISO 3166-1 alpha-2, e.g. NO" );
        strField( "Port of registry", s.portOfRegistry );
        strField( "Owner", s.owner );
        strField( "Vessel operator", s.vesselOperator );

        ImGui::SeparatorText( "Classification & Type" );
        strField( "Classification soc.", s.classificationSociety, "e.g. DNV, Lloyd's Register" );
        strField( "Ship type", s.shipType, "e.g. Bulk carrier, Tanker, OSV" );

        ImGui::SeparatorText( "Propulsion & Performance" );
        strField( "Propulsion type", s.propulsionType, "e.g. Diesel, LNG, Diesel-Electric" );
        strField( "Propeller type", s.propellerType, "e.g. FPP, CPP, Azipod" );
        optIntField( "Propeller count", s.propellerCount );
        optDoubleField( "MCR", s.mcr, "kW" );
        optDoubleField( "Service speed", s.serviceSpeed, "knots" );

        ImGui::SeparatorText( "Dimensions" );
        optDoubleField( "Depth (moulded)", s.depth, "m" );
        optDoubleField( "Draft", s.draft, "m" );

        ImGui::SeparatorText( "Tonnage" );
        optDoubleField( "Gross tonnage", s.grossTonnage );
        optDoubleField( "Net tonnage", s.netTonnage );
        optDoubleField( "Deadweight", s.deadweight, "DWT (metric tons)" );

        if( changed )
        {
            p.isDirty = true;
            notifyChanged();
        }
    }

    void ProjectManager::renderStatusBar()
    {
        if( m_statusMessage.empty() )
        {
            return;
        }

        if( m_statusIsError )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, Theme::TextError );
        }
        else
        {
            ImGui::PushStyleColor( ImGuiCol_Text, Theme::TextSuccess );
        }

        ImGui::TextUnformatted( m_statusMessage.c_str() );
        ImGui::PopStyleColor();
    }

    void ProjectManager::doSave( const std::string& path )
    {
        if( !m_activeProject.has_value() )
        {
            return;
        }

        auto& p = m_activeProject.value();

        if( ProjectSerializer::save( p, path ) )
        {
            p.filePath = path;
            p.isDirty = false;
            m_statusMessage = "Saved to " + path;
            m_statusIsError = false;
            notifyChanged();
        }
        else
        {
            m_statusMessage = "Failed to save: " + path;
            m_statusIsError = true;
        }
    }

    void ProjectManager::doLoad( const std::string& path )
    {
        auto result = ProjectSerializer::load( path );
        if( result.has_value() )
        {
            m_activeProject = std::move( *result );
            m_statusMessage = "Opened: " + path;
            m_statusIsError = false;
            notifyChanged();
        }
        else
        {
            m_statusMessage = "Failed to open: " + path;
            m_statusIsError = true;
        }
    }

    void ProjectManager::notifyChanged()
    {
        if( m_onChanged )
        {
            m_onChanged();
        }
    }
} // namespace nfx::vista
