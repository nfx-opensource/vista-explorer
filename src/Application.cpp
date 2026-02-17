#include "Application.h"
#include "panels/GmodViewer.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

namespace nfx::vista
{
    Application::Application()
        : m_window{ nullptr },
          m_vis{ VIS::instance() }
    {
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

            m_window = glfwCreateWindow( 1920, 1080, "Vista explorer", nullptr, nullptr );
            if( !m_window )
            {
                std::cerr << "Failed to create GLFW window\n";
                glfwTerminate();
                return false;
            }

            glfwMakeContextCurrent( m_window );
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
            ImGui_ImplGlfw_InitForOpenGL( m_window, true );
            ImGui_ImplOpenGL3_Init( glslVersion );
        }

        {
            m_gmodViewer = std::make_unique<GmodViewer>( m_vis );
        }

        return true;
    }

    void Application::run()
    {
        while( !glfwWindowShouldClose( m_window ) )
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
            glfwDestroyWindow( m_window );
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
        m_gmodViewer->render();
    }

    void Application::endFrame()
    {
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize( m_window, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0.1f, 0.1f, 0.12f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( m_window );
    }
} // namespace nfx::vista
