#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

GLFWwindow* createWindow()
{
    if( !glfwInit() )
    {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow* window = glfwCreateWindow( 1920, 1080, "Vista explorer", nullptr, nullptr );
    if( !window )
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    return window;
}

bool initializeImGui( GLFWwindow* window )
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 330";
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( glsl_version );

    return true;
}

void cleanup( GLFWwindow* window )
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow( window );
    glfwTerminate();
}

int main()
{
    GLFWwindow* window = createWindow();
    if( !window )
        return 1;

    if( !initializeImGui( window ) )
    {
        cleanup( window );
        return 1;
    }

    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize( window, &display_w, &display_h );
        glViewport( 0, 0, display_w, display_h );
        glClearColor( 0.1f, 0.1f, 0.12f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( window );
    }

    cleanup( window );
    return 0;
}
