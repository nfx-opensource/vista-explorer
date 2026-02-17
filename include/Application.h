#pragma once

#include <dnv/vista/sdk/VIS.h>

#include <memory>
#include <string>

struct GLFWwindow;

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    class GmodViewer;

    class Application
    {
    public:
        Application();
        ~Application();
        Application( const Application& ) = delete;
        Application& operator=( const Application& ) = delete;

        bool initialize();
        void run();
        void shutdown();

    private:
        void beginFrame();
        void renderFrame();
        void endFrame();

        GLFWwindow* m_window;
        bool m_running = true;

        const VIS& m_vis;

        std::unique_ptr<GmodViewer> m_gmodViewer;
        bool m_showGmodViewer = true;
    };
} // namespace nfx::vista
