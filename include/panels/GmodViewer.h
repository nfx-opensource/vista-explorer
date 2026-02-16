#pragma once

#include <dnv/vista/sdk/VisVersions.h>

namespace dnv::vista::sdk
{
    class VIS;
    class Gmod;
} // namespace dnv::vista::sdk

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    class GmodViewer
    {
    public:
        explicit GmodViewer( const VIS& vis );

        void render();

    private:
        void renderHeader();
        void renderLegend();
        void renderTree( const Gmod& gmod );

        const VIS& m_vis;
        VisVersion m_currentVersion;
        int m_versionIndex;
    };
} // namespace nfx::vista
