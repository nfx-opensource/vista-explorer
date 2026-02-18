#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <imgui.h>

#include <functional>
#include <optional>
#include <string>

namespace dnv::vista::sdk
{
    class VIS;
    class LocalId;
} // namespace dnv::vista::sdk

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    class LocalIdBuilder
    {
    public:
        explicit LocalIdBuilder( const VIS& vis );

        void render( VisVersion version );

        void setChangeNotifier( std::function<void()> notifier )
        {
            m_onChanged = std::move( notifier );
        }

        void setCurrentGmodPath( const std::optional<GmodPath>& path )
        {
            m_currentGmodPath = path;
        }

    private:
        void renderPrimaryItemSection( VisVersion version );
        void renderSecondaryItemSection( VisVersion version );
        void renderMetadataSection();
        void renderOutputSection( VisVersion version );

        const VIS& m_vis;
        std::function<void()> m_onChanged;
        std::optional<GmodPath> m_currentGmodPath;

        // Builder state
        struct
        {
            char primaryPath[512] = {};
            char secondaryPath[512] = {};
            bool hasSecondaryItem = false;

            // Metadata tags
            char quantity[128] = {};
            char content[128] = {};
            char position[128] = {};
            char calculation[128] = {};
            char state[128] = {};
            char command[128] = {};
            char type[128] = {};
            char detail[128] = {};

            // Output
            std::string generatedLocalId;
            ParsingErrors errors;
        } m_state;
    };
} // namespace nfx::vista
