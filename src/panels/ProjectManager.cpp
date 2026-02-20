#include "panels/ProjectManager.h"

#include <dnv/vista/sdk/ImoNumber.h>
#include <dnv/vista/sdk/transport/ShipId.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cctype>

namespace nfx::vista
{
    ProjectManager::ProjectManager()
    {
        const auto dir = ProjectSerializer::defaultDir();
        m_saveAsPath = ( dir / "project.json" ).string();
        m_openPath = m_saveAsPath;
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
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.35f, 0.35f, 1.0f ) );
            ImGui::TextWrapped( "%s", m_newShipIdError.c_str() );
            ImGui::PopStyleColor();
        }
        else if( m_newShipIdValid )
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.3f, 1.0f, 0.3f, 1.0f ) );
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
        ImGui::SetNextWindowSize( ImVec2( 500, 120 ), ImGuiCond_Appearing );

        if( !ImGui::BeginPopupModal( "Open Project", nullptr, ImGuiWindowFlags_NoResize ) )
        {
            return;
        }

        ImGui::Text( "Project file path (.json)" );
        ImGui::SetNextItemWidth( -1 );
        ImGui::InputText( "##openPath", &m_openPath );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if( ImGui::Button( "Open", ImVec2( 120, 0 ) ) )
        {
            doLoad( m_openPath );
            ImGui::CloseCurrentPopup();
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
        ImGui::SetNextWindowSize( ImVec2( 500, 120 ), ImGuiCond_Appearing );

        if( !ImGui::BeginPopupModal( "Save As", nullptr, ImGuiWindowFlags_NoResize ) )
        {
            return;
        }

        ImGui::Text( "Save path (.json)" );
        ImGui::SetNextItemWidth( -1 );
        ImGui::InputText( "##saveAsPath", &m_saveAsPath );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if( ImGui::Button( "Save", ImVec2( 120, 0 ) ) )
        {
            doSave( m_saveAsPath );
            ImGui::CloseCurrentPopup();
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
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.35f, 0.35f, 1.0f ) );
        }
        else
        {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.5f, 0.9f, 0.5f, 1.0f ) );
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
