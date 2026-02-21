#include "Application.h"
#include "config/Theme.h"
#include "panels/GmodViewer.h"
#include "panels/NodeDetails.h"
#include "panels/LocalIdBuilder.h"
#include "panels/ProjectManager.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

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

            m_window.handle = glfwCreateWindow( 1280, 720, "Vista explorer", nullptr, nullptr );
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

            c[ImGuiCol_WindowBg] = Theme::Bg2;
            c[ImGuiCol_ChildBg] = Theme::Bg1;
            c[ImGuiCol_PopupBg] = ImVec4( Theme::Bg2.x, Theme::Bg2.y, Theme::Bg2.z, 0.98f );
            c[ImGuiCol_Border] = Theme::Border;

            c[ImGuiCol_FrameBg] = Theme::Bg0;
            c[ImGuiCol_FrameBgHovered] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
            c[ImGuiCol_FrameBgActive] = ImVec4( 0.12f, 0.12f, 0.12f, 1.00f );

            c[ImGuiCol_TitleBg] = Theme::Bg1;
            c[ImGuiCol_TitleBgActive] = Theme::Bg1;
            c[ImGuiCol_TitleBgCollapsed] = Theme::Bg0;

            c[ImGuiCol_MenuBarBg] = Theme::Bg1;
            c[ImGuiCol_ScrollbarBg] = Theme::Bg0;
            c[ImGuiCol_ScrollbarGrab] = Theme::Bg4;
            c[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.52f, 0.52f, 0.52f, 1.00f );
            c[ImGuiCol_ScrollbarGrabActive] = Theme::Accent;

            c[ImGuiCol_CheckMark] = Theme::Accent;
            c[ImGuiCol_SliderGrab] = Theme::Accent;
            c[ImGuiCol_SliderGrabActive] = Theme::AccentBright;

            c[ImGuiCol_Button] = Theme::Bg3;
            c[ImGuiCol_ButtonHovered] = Theme::Accent;
            c[ImGuiCol_ButtonActive] = Theme::AccentDark;

            c[ImGuiCol_Header] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.30f );
            c[ImGuiCol_HeaderHovered] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.50f );
            c[ImGuiCol_HeaderActive] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.80f );

            c[ImGuiCol_Separator] = Theme::Border;
            c[ImGuiCol_SeparatorHovered] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.70f );
            c[ImGuiCol_SeparatorActive] = Theme::Accent;

            c[ImGuiCol_ResizeGrip] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.20f );
            c[ImGuiCol_ResizeGripHovered] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.60f );
            c[ImGuiCol_ResizeGripActive] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.90f );

            c[ImGuiCol_Tab] = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );
            c[ImGuiCol_TabHovered] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.70f );
            c[ImGuiCol_TabActive] = ImVec4( 0.24f, 0.50f, 0.90f, 1.00f );
            c[ImGuiCol_TabUnfocused] = Theme::Bg1;
            c[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.22f, 0.40f, 0.70f, 1.00f );

            c[ImGuiCol_DockingPreview] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.50f );
            c[ImGuiCol_DockingEmptyBg] = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );

            c[ImGuiCol_PlotLines] = Theme::Accent;
            c[ImGuiCol_PlotHistogram] = Theme::Accent;

            c[ImGuiCol_TextSelectedBg] = ImVec4( Theme::Accent.x, Theme::Accent.y, Theme::Accent.z, 0.35f );
            c[ImGuiCol_NavHighlight] = Theme::Accent;

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

            m_panels.nodeDetails->setUsePrimaryCallback( [this]( const dnv::vista::sdk::GmodPath& path ) {
                m_panels.localIdBuilder->setPrimaryPath( path );
                m_rendering.mode.notifyChange();
            } );

            m_panels.nodeDetails->setUseSecondaryCallback( [this]( const dnv::vista::sdk::GmodPath& path ) {
                m_panels.localIdBuilder->setSecondaryPath( path );
                m_rendering.mode.notifyChange();
            } );

            m_panels.projectManager = std::make_unique<ProjectManager>();
            m_panels.projectManager->setChangeNotifier( [this]() { m_rendering.mode.notifyChange(); } );
        }

        {
            if( const GLubyte* renderer = glGetString( GL_RENDERER ) )
            {
                m_status.rendererName = reinterpret_cast<const char*>( renderer );
            }

            if( const GLubyte* version = glGetString( GL_VERSION ) )
            {
                m_status.glVersion = reinterpret_cast<const char*>( version );
            }

            const auto& gmod = m_vis.instance->gmod( m_vis.currentVersion );
            m_status.nodeCount = static_cast<size_t>( std::distance( gmod.begin(), gmod.end() ) );
        }

        return true;
    }

    void Application::run()
    {
        while( !glfwWindowShouldClose( m_window.handle ) )
        {
            beginFrame();
            renderFrame();
            endFrame();

            m_rendering.mode.waitOrPollEvents();
        }

        shutdown();
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

        if( m_layout.resetRequested )
        {
            setupDefaultLayout( dockspaceId );
            m_layout.resetRequested = false;
        }
        else if( m_layout.needsSetup )
        {
            setupDefaultLayout( dockspaceId );
            m_layout.needsSetup = false;
        }

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
                        m_status.nodeCount = static_cast<size_t>( std::distance( gmod.begin(), gmod.end() ) );
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
                if( ImGui::MenuItem( "Reset Layout" ) )
                {
                    m_layout.resetRequested = true;
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
            // Left: VIS data
            ImGui::Text( "VIS: %s", VisVersions::toString( m_vis.currentVersion ).data() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            ImGui::Text( "Nodes: %zu", m_status.nodeCount );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // Rendering info
            ImGui::Text( "Mode: %s", m_rendering.mode.modeName() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            ImGui::Text( "GPU: %s", m_status.rendererName.c_str() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            ImGui::Text( "OpenGL %s", m_status.glVersion.c_str() );

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

    void Application::setupDefaultLayout( unsigned int dockspaceId )
    {
        ImGui::DockBuilderRemoveNode( dockspaceId );
        ImGui::DockBuilderAddNode( dockspaceId, ImGuiDockNodeFlags_DockSpace );
        ImGui::DockBuilderSetNodeSize( dockspaceId, ImGui::GetMainViewport()->WorkSize );

        // Split vertically 50/50: left | right
        ImGuiID rightId;
        ImGuiID leftId = ImGui::DockBuilderSplitNode( dockspaceId, ImGuiDir_Left, 0.5f, nullptr, &rightId );

        // Split right half horizontally 50/50: top (GmodViewer) | bottom (NodeDetails)
        ImGuiID rightBottomId;
        ImGuiID rightTopId = ImGui::DockBuilderSplitNode( rightId, ImGuiDir_Up, 0.5f, nullptr, &rightBottomId );

        // Dock panels
        ImGui::DockBuilderDockWindow( "LocalId Builder", leftId );
        ImGui::DockBuilderDockWindow( "Project Manager", leftId );
        ImGui::DockBuilderDockWindow( "Gmod Viewer", rightTopId );
        ImGui::DockBuilderDockWindow( "Node Details", rightBottomId );

        ImGui::DockBuilderFinish( dockspaceId );
    }
} // namespace nfx::vista
