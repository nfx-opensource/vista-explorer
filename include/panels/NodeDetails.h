#pragma once

#include <dnv/vista/sdk/VIS.h>
#include <optional>
#include <string>

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    class NodeDetails
    {
    public:
        NodeDetails() = default;

        void render();

        void setCurrentGmodPath( const std::optional<GmodPath>& path )
        {
            m_currentGmodPath = path;
        }

    private:
        std::optional<GmodPath> m_currentGmodPath;
    };
} // namespace nfx::vista
