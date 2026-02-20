#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <functional>
#include <optional>
#include <string>

namespace nfx::vista
{
    class NodeDetails
    {
    public:
        NodeDetails() = default;

        void render();

        void setCurrentGmodPath( const std::optional<dnv::vista::sdk::GmodPath>& path )
        {
            m_currentGmodPath = path;
        }

        void setUsePrimaryCallback( std::function<void( const dnv::vista::sdk::GmodPath& )> cb )
        {
            m_onUsePrimary = std::move( cb );
        }

        void setUseSecondaryCallback( std::function<void( const dnv::vista::sdk::GmodPath& )> cb )
        {
            m_onUseSecondary = std::move( cb );
        }

    private:
        std::optional<dnv::vista::sdk::GmodPath> m_currentGmodPath;
        std::function<void( const dnv::vista::sdk::GmodPath& )> m_onUsePrimary;
        std::function<void( const dnv::vista::sdk::GmodPath& )> m_onUseSecondary;
    };
} // namespace nfx::vista
