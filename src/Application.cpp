#include "Application.h"
#include "panels/GmodViewer.h"
#include "panels/NodeDetails.h"
#include "panels/LocalIdBuilder.h"
#include "panels/ProjectManager.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <iterator>

using namespace dnv::vista::sdk;

namespace nfx::vista
{
    Application::Application()
        : m_window{ nullptr },
          m_vis{ &VIS::instance() }
    {
        auto versions = m_vis.instance->versions();
        m_vis.versionIndex = versions.size() - 1;
        m_vis.currentVersion = versions[m_vis.versionIndex];
    }

    Application::~Application()
    {
    }

    bool Application::initialize()
    {
        {
            if( !glfwInit() )
            {
                std::cerr << "Failed to initialize GLFW\n";
                return false;
            }

            glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
            glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
            glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

            m_window.handle = glfwCreateWindow( 1920, 1080, "Vista explorer", nullptr, nullptr );
            if( !m_window.handle )
            {
                std::cerr << "Failed to create GLFW window\n";
                glfwTerminate();
                return false;
            }

            glfwMakeContextCurrent( m_window.handle );
            glfwSwapInterval( 1 );
        }

        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            ImGui::StyleColorsDark();

            ImGuiStyle& style = ImGui::GetStyle();

            // --- Shape ---
            style.WindowRounding = 6.0f;
            style.ChildRounding = 4.0f;
            style.FrameRounding = 4.0f;
            style.PopupRounding = 4.0f;
            style.ScrollbarRounding = 6.0f;
            style.GrabRounding = 4.0f;
            style.TabRounding = 4.0f;
            style.WindowBorderSize = 1.0f;
            style.FrameBorderSize = 0.0f;
            style.WindowPadding = ImVec2( 10.0f, 10.0f );
            style.FramePadding = ImVec2( 6.0f, 4.0f );
            style.ItemSpacing = ImVec2( 8.0f, 6.0f );
            style.ScrollbarSize = 12.0f;
            style.GrabMinSize = 10.0f;

            // --- Colors ---
            ImVec4* c = style.Colors;

            // Palette
            const ImVec4 bg0 = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f ); // darkest bg (frames, scrollbar)
            const ImVec4 bg1 = ImVec4( 0.18f, 0.18f, 0.18f, 1.00f ); // panel bg, child bg
            const ImVec4 bg2 = ImVec4( 0.22f, 0.22f, 0.22f, 1.00f ); // window bg
            const ImVec4 bg3 = ImVec4( 0.30f, 0.30f, 0.30f, 1.00f ); // buttons, interactive
            const ImVec4 bg4 = ImVec4( 0.40f, 0.40f, 0.40f, 1.00f ); // scrollbar grab, hover
            const ImVec4 border = ImVec4( 0.35f, 0.35f, 0.35f, 1.00f );
            const ImVec4 accent = ImVec4( 0.29f, 0.60f, 1.00f, 1.00f ); // blue
            const ImVec4 accentBright = ImVec4( 0.40f, 0.70f, 1.00f, 1.00f );
            const ImVec4 accentDark = ImVec4( 0.20f, 0.48f, 0.90f, 1.00f );

            c[ImGuiCol_WindowBg] = bg2;
            c[ImGuiCol_ChildBg] = bg1;
            c[ImGuiCol_PopupBg] = ImVec4( bg2.x, bg2.y, bg2.z, 0.98f );
            c[ImGuiCol_Border] = border;

            c[ImGuiCol_FrameBg] = bg0;
            c[ImGuiCol_FrameBgHovered] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
            c[ImGuiCol_FrameBgActive] = ImVec4( 0.12f, 0.12f, 0.12f, 1.00f );

            c[ImGuiCol_TitleBg] = bg1;
            c[ImGuiCol_TitleBgActive] = ImVec4( 0.18f, 0.18f, 0.18f, 1.00f );
            c[ImGuiCol_TitleBgCollapsed] = bg0;

            c[ImGuiCol_MenuBarBg] = bg1;
            c[ImGuiCol_ScrollbarBg] = bg0;
            c[ImGuiCol_ScrollbarGrab] = bg4;
            c[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
            c[ImGuiCol_ScrollbarGrabActive] = accent;

            c[ImGuiCol_CheckMark] = accent;
            c[ImGuiCol_SliderGrab] = accent;
            c[ImGuiCol_SliderGrabActive] = accentBright;

            c[ImGuiCol_Button] = bg3;
            c[ImGuiCol_ButtonHovered] = accent;
            c[ImGuiCol_ButtonActive] = accentDark;

            c[ImGuiCol_Header] = ImVec4( accent.x, accent.y, accent.z, 0.30f );
            c[ImGuiCol_HeaderHovered] = ImVec4( accent.x, accent.y, accent.z, 0.50f );
            c[ImGuiCol_HeaderActive] = ImVec4( accent.x, accent.y, accent.z, 0.80f );

            c[ImGuiCol_Separator] = border;
            c[ImGuiCol_SeparatorHovered] = ImVec4( accent.x, accent.y, accent.z, 0.70f );
            c[ImGuiCol_SeparatorActive] = accent;

            c[ImGuiCol_ResizeGrip] = ImVec4( accent.x, accent.y, accent.z, 0.20f );
            c[ImGuiCol_ResizeGripHovered] = ImVec4( accent.x, accent.y, accent.z, 0.60f );
            c[ImGuiCol_ResizeGripActive] = ImVec4( accent.x, accent.y, accent.z, 0.90f );

            c[ImGuiCol_Tab] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
            c[ImGuiCol_TabHovered] = ImVec4( accent.x, accent.y, accent.z, 0.70f );
            c[ImGuiCol_TabActive] = ImVec4( 0.24f, 0.50f, 0.90f, 1.00f );
            c[ImGuiCol_TabUnfocused] = bg1;
            c[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.22f, 0.40f, 0.70f, 1.00f );

            c[ImGuiCol_DockingPreview] = ImVec4( accent.x, accent.y, accent.z, 0.50f );
            c[ImGuiCol_DockingEmptyBg] = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );

            c[ImGuiCol_PlotLines] = accent;
            c[ImGuiCol_PlotHistogram] = accent;

            c[ImGuiCol_TextSelectedBg] = ImVec4( accent.x, accent.y, accent.z, 0.35f );
            c[ImGuiCol_NavHighlight] = accent;

            const char* glslVersion = "#version 330";
            ImGui_ImplGlfw_InitForOpenGL( m_window.handle, true );
            ImGui_ImplOpenGL3_Init( glslVersion );
        }

        {
            m_panels.gmodViewer = std::make_unique<GmodViewer>( *m_vis.instance );
            m_panels.gmodViewer->setChangeNotifier( [this]() { m_rendering.mode.notifyChange(); } );
            m_panels.gmodViewer->setNodeSelectionCallback( [this]( std::optional<GmodPath> path ) {
                m_currentGmodPath = path;
                m_panels.nodeDetails->setCurrentGmodPath( path );
                m_panels.localIdBuilder->setCurrentGmodPath( path );
                m_rendering.mode.notifyChange();
            } );

            m_panels.nodeDetails = std::make_unique<NodeDetails>();

            m_panels.localIdBuilder = std::make_unique<LocalIdBuilder>( *m_vis.instance );
            m_panels.localIdBuilder->setChangeNotifier( [this]() { m_rendering.mode.notifyChange(); } );

            m_panels.projectManager = std::make_unique<ProjectManager>();
            m_panels.projectManager->setChangeNotifier( [this]() { m_rendering.mode.notifyChange(); } );
        }

        {
            if( const GLubyte* renderer = glGetString( GL_RENDERER ) )
            {
                m_rendererName = reinterpret_cast<const char*>( renderer );
            }

            const auto& gmod = m_vis.instance->gmod( m_vis.currentVersion );
            m_nodeCount = static_cast<size_t>( std::distance( gmod.begin(), gmod.end() ) );
        }

        return true;
    }

    void Application::run()
    {
        while( !glfwWindowShouldClose( m_window.handle ) )
        {
            m_rendering.mode.waitOrPollEvents();

            beginFrame();
            renderFrame();
            endFrame();
        }

        shutdown();
    }

    void Application::shutdown()
    {
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        {
            glfwDestroyWindow( m_window.handle );
            glfwTerminate();
        }
    }

    void Application::beginFrame()
    {
        // Calculate FPS for polling and adaptive modes
        if( m_rendering.mode.mode() != RenderingMode::Mode::EventDriven )
        {
            double currentTime = glfwGetTime();
            if( m_rendering.lastFrameTime > 0.0 )
            {
                double deltaTime = currentTime - m_rendering.lastFrameTime;
                if( deltaTime > 0.0 )
                {
                    m_rendering.fps = 1.0 / deltaTime;
                }
            }
            m_rendering.lastFrameTime = currentTime;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Application::renderFrame()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        const float statusBarHeight = 25.0f;

        ImGui::SetNextWindowPos( viewport->WorkPos );
        ImGui::SetNextWindowSize( ImVec2( viewport->WorkSize.x, viewport->WorkSize.y - statusBarHeight ) );
        ImGui::SetNextWindowViewport( viewport->ID );

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        windowFlags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

        ImGui::Begin( "DockSpace", nullptr, windowFlags );
        ImGui::PopStyleVar( 3 );

        // Create dockspace
        ImGuiID dockspaceId = ImGui::GetID( "MainDockSpace" );
        ImGui::DockSpace( dockspaceId, ImVec2( 0.0f, 0.0f ), ImGuiDockNodeFlags_PassthruCentralNode );

        // Menu bar
        if( ImGui::BeginMenuBar() )
        {
            if( ImGui::BeginMenu( "File" ) )
            {
                if( ImGui::MenuItem( "Exit" ) )
                {
                    glfwSetWindowShouldClose( m_window.handle, true );
                }
                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "VIS" ) )
            {
                auto versions = m_vis.instance->versions();
                for( size_t i = 0; i < versions.size(); ++i )
                {
                    bool isSelected = ( i == m_vis.versionIndex );
                    if( ImGui::MenuItem( VisVersions::toString( versions[i] ).data(), nullptr, isSelected ) )
                    {
                        m_vis.versionIndex = i;
                        m_vis.currentVersion = versions[i];
                        const auto& gmod = m_vis.instance->gmod( m_vis.currentVersion );
                        m_nodeCount = static_cast<size_t>( std::distance( gmod.begin(), gmod.end() ) );
                        m_rendering.mode.notifyChange();
                    }
                }
                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "View" ) )
            {
                if( ImGui::MenuItem( "Gmod Viewer", nullptr, &m_ui.showGmodViewer ) )
                {
                    m_rendering.mode.notifyChange();
                }
                if( ImGui::MenuItem( "Node Details", nullptr, &m_ui.showNodeDetails ) )
                {
                    m_rendering.mode.notifyChange();
                }
                if( ImGui::MenuItem( "LocalId Builder", nullptr, &m_ui.showLocalIdBuilder ) )
                {
                    m_rendering.mode.notifyChange();
                }
                if( ImGui::MenuItem( "Project Manager", nullptr, &m_ui.showProjectManager ) )
                {
                    m_rendering.mode.notifyChange();
                }

                ImGui::Separator();
                ImGui::Text( "Rendering Mode" );
                bool isEventDriven = m_rendering.mode.mode() == RenderingMode::Mode::EventDriven;
                bool isAdaptive = m_rendering.mode.mode() == RenderingMode::Mode::Adaptive;
                bool isPolling = m_rendering.mode.mode() == RenderingMode::Mode::Polling;

                if( ImGui::MenuItem( "Adaptive", nullptr, isAdaptive ) )
                {
                    m_rendering.mode.setMode( RenderingMode::Mode::Adaptive );
                    m_rendering.mode.notifyChange();
                }
                if( ImGui::MenuItem( "Event-driven (Low GPU)", nullptr, isEventDriven ) )
                {
                    m_rendering.mode.setMode( RenderingMode::Mode::EventDriven );
                    m_rendering.mode.notifyChange();
                }
                if( ImGui::MenuItem( "Polling (High CPU)", nullptr, isPolling ) )
                {
                    m_rendering.mode.setMode( RenderingMode::Mode::Polling );
                    m_rendering.mode.notifyChange();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        // Render panels
        if( m_ui.showGmodViewer )
        {
            m_panels.gmodViewer->render( m_vis.currentVersion );
        }

        if( m_ui.showNodeDetails )
        {
            m_panels.nodeDetails->render();
        }

        if( m_ui.showLocalIdBuilder )
        {
            m_panels.localIdBuilder->render( m_vis.currentVersion );
        }

        if( m_ui.showProjectManager )
        {
            m_panels.projectManager->render();
        }

        // Render status bar
        renderStatusBar();
    }

    void Application::renderStatusBar()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Position at bottom of viewport
        const float statusBarHeight = 25.0f;
        ImGui::SetNextWindowPos(
            ImVec2( viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - statusBarHeight ) );
        ImGui::SetNextWindowSize( ImVec2( viewport->WorkSize.x, statusBarHeight ) );
        ImGui::SetNextWindowViewport( viewport->ID );

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 4.0f ) );
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 20.0f, 0.0f ) );

        if( ImGui::Begin( "##StatusBar", nullptr, windowFlags ) )
        {
            // Rendering mode
            ImGui::Text( "Mode: %s", m_rendering.mode.modeName() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // GPU info
            ImGui::Text( "GPU: %s", m_rendererName.c_str() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // VIS version
            ImGui::Text( "VIS: %s", VisVersions::toString( m_vis.currentVersion ).data() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // Nodes count
            ImGui::Text( "Nodes: %zu", m_nodeCount );

            // FPS (in polling and adaptive modes)
            if( m_rendering.mode.mode() != RenderingMode::Mode::EventDriven )
            {
                ImGui::SameLine();
                ImGui::TextDisabled( "|" );
                ImGui::SameLine();
                ImGui::Text( "FPS: %.1f", m_rendering.fps );
            }
        }
        ImGui::End();

        ImGui::PopStyleVar( 3 );
    }

    void Application::endFrame()
    {
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize( m_window.handle, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0.1f, 0.1f, 0.12f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( m_window.handle );
    }
} // namespace nfx::vista
