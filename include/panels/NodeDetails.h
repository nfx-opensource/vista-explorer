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
        void setSelectedNode( const GmodNode* node );

    private:
        const GmodNode* m_selectedNode = nullptr;
    };
} // namespace nfx::vista
