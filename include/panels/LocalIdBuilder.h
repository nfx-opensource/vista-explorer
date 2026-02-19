#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <imgui.h>

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace dnv::vista::sdk
{
    class VIS;
    class LocalId;
} // namespace dnv::vista::sdk

namespace nfx::vista
{
    class LocalIdBuilder
    {
    public:
        explicit LocalIdBuilder( const dnv::vista::sdk::VIS& vis );

        void render( dnv::vista::sdk::VisVersion version );

        void setChangeNotifier( std::function<void()> notifier )
        {
            m_onChanged = std::move( notifier );
        }

        void setCurrentGmodPath( const std::optional<dnv::vista::sdk::GmodPath>& path )
        {
            m_currentGmodPath = path;
        }

    private:
        void renderPrimaryItemSection( dnv::vista::sdk::VisVersion version );
        void renderSecondaryItemSection( dnv::vista::sdk::VisVersion version );
        void renderLocationSection( dnv::vista::sdk::VisVersion version );
        void renderMetadataSection( dnv::vista::sdk::VisVersion version );
        void renderOutputSection( dnv::vista::sdk::VisVersion version );

        void renderMetadataInput(
            const char* id,
            const char* label,
            char* buffer,
            size_t bufferSize,
            dnv::vista::sdk::CodebookName codebook );

        const dnv::vista::sdk::VIS& m_vis;
        std::function<void()> m_onChanged;
        std::optional<dnv::vista::sdk::GmodPath> m_currentGmodPath;

        std::unordered_map<dnv::vista::sdk::CodebookName, std::vector<std::string>> m_codebookCache;
        std::optional<dnv::vista::sdk::VisVersion> m_cachedVersion;

        // Builder state
        struct
        {
            char primaryPath[512] = {};
            char secondaryPath[512] = {};
            bool hasSecondaryItem = false;
            bool verboseMode = false;

            // Cached parsed paths — invalidated when path text changes
            std::optional<dnv::vista::sdk::GmodPath> primaryPathOpt;
            std::optional<dnv::vista::sdk::GmodPath> secondaryPathOpt;
            bool primaryPathDirty = true;
            bool secondaryPathDirty = true;

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
            dnv::vista::sdk::ParsingErrors errors;

            // Location builder state
            int locationNumber = 0;      // 0 = none
            char locationSide = 0;       // 0 = none, 'P'/'C'/'S'
            char locationVertical = 0;   // 0 = none, 'U'/'M'/'L'
            char locationTransverse = 0; // 0 = none, 'I'/'O'
            char locationLong = 0;       // 0 = none, 'F'/'A'
        } m_state;

        std::unordered_map<std::string, std::string> m_comboFilters;
    };
} // namespace nfx::vista
