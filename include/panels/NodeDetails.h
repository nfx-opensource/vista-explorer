#pragma once

#include <dnv/vista/sdk/VIS.h>
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

    private:
        std::optional<dnv::vista::sdk::GmodPath> m_currentGmodPath;
    };
} // namespace nfx::vista
