#pragma once

#include "RenderingMode.h"

#include <dnv/vista/sdk/VIS.h>

#include <memory>
#include <optional>
#include <string>

struct GLFWwindow;

namespace nfx::vista
{
    class GmodViewer;
    class NodeDetails;
    class LocalIdBuilder;
    class ProjectManager;

    class Application
    {
    public:
        Application();
        ~Application();
        Application( const Application& ) = delete;
        Application& operator=( const Application& ) = delete;

        bool initialize();
        void run();

    private:
        void beginFrame();
        void renderFrame();
        void endFrame();

        void renderStatusBar();

        void shutdown();

        void setupDefaultLayout( unsigned int dockspaceId );

        struct
        {
            GLFWwindow* handle = nullptr;
            bool running = true;
        } m_window;

        struct
        {
            const dnv::vista::sdk::VIS* instance;
            dnv::vista::sdk::VisVersion currentVersion;
            int versionIndex;
        } m_vis;

        struct
        {
            std::unique_ptr<GmodViewer> gmodViewer;
            std::unique_ptr<NodeDetails> nodeDetails;
            std::unique_ptr<LocalIdBuilder> localIdBuilder;
            std::unique_ptr<ProjectManager> projectManager;
        } m_panels;

        struct
        {
            bool showGmodViewer = true;
            bool showNodeDetails = true;
            bool showLocalIdBuilder = true;
            bool showProjectManager = true;
        } m_ui;

        struct
        {
            RenderingMode mode;
            double lastFrameTime = 0.0;
            double fps = 0.0;
        } m_rendering;

        std::string m_rendererName;
        std::string m_glVersion;
        size_t m_nodeCount = 0;
        bool m_layoutNeedsSetup = true;
        bool m_layoutResetRequested = false;

        std::optional<dnv::vista::sdk::GmodPath> m_currentGmodPath;
    };
} // namespace nfx::vista
