#include "Application.h"
#include "panels/GmodViewer.h"
#include "panels/NodeDetails.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

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

            const char* glslVersion = "#version 330";
            ImGui_ImplGlfw_InitForOpenGL( m_window.handle, true );
            ImGui_ImplOpenGL3_Init( glslVersion );
        }

        {
            m_panels.gmodViewer = std::make_unique<GmodViewer>( *m_vis.instance );
            m_panels.nodeDetails = std::make_unique<NodeDetails>();
        }

        return true;
    }

    void Application::run()
    {
        while( !glfwWindowShouldClose( m_window.handle ) )
        {
            glfwWaitEvents();

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
        // glfwPollEvents();

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
                    bool isSelected = (i == m_vis.versionIndex);
                    if( ImGui::MenuItem( VisVersions::toString( versions[i] ).data(), nullptr, isSelected ) )
                    {
                        m_vis.versionIndex = i;
                        m_vis.currentVersion = versions[i];
                        glfwPostEmptyEvent();
                    }
                }
                ImGui::EndMenu();
            }

            if( ImGui::BeginMenu( "View" ) )
            {
                ImGui::MenuItem( "Gmod Viewer", nullptr, &m_ui.showGmodViewer );
                ImGui::MenuItem( "Node Details", nullptr, &m_ui.showNodeDetails );
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
            const GmodNode* selectedNode = m_panels.gmodViewer->selectedNode( m_vis.currentVersion );
            m_panels.nodeDetails->setSelectedNode( selectedNode );
            m_panels.nodeDetails->render();
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

        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 4.0f ) );
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 20.0f, 0.0f ) );

        if( ImGui::Begin( "##StatusBar", nullptr, windowFlags ) )
        {
            // Rendering mode
            ImGui::Text( "Mode: Event-driven" );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // GPU info
            const GLubyte* renderer = glGetString( GL_RENDERER );
            ImGui::Text( "GPU: %s", renderer );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // VIS version
            ImGui::Text( "VIS: %s", VisVersions::toString( m_vis.currentVersion ).data() );

            ImGui::SameLine();
            ImGui::TextDisabled( "|" );
            ImGui::SameLine();

            // Nodes count
            const auto& gmod = m_vis.instance->gmod( m_vis.currentVersion );
            ImGui::Text( "Nodes: %zu", std::distance( gmod.begin(), gmod.end() ) );
        }
        ImGui::End();

        ImGui::PopStyleVar( 2 );
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
