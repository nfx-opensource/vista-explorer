#==============================================================================
# nfx-vista-explorer - Top-level CMake configuration
#==============================================================================

#----------------------------------------------
# Project definition
#----------------------------------------------

cmake_minimum_required(VERSION 3.20)

project(nfx-vista-explorer
    VERSION 0.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#----------------------------------------------
# Dependencies - ImGui
#----------------------------------------------

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(imgui)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# Platform-specific ImGui backends
list(APPEND IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImGui stdlib helpers (InputText for std::string)
list(APPEND IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

#----------------------------------------------
# Dependencies - GLFW
#----------------------------------------------

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw)

#----------------------------------------------
# Dependencies - Vista SDK
#----------------------------------------------

# Vista SDK build options - explicit configuration
set(DNV_VISTA_SDK_CPP_BUILD_STATIC        OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_SHARED        ON  CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_TOOLS         OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_TESTS         OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_SAMPLES       OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_BENCHMARKS    OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_ENABLE_SIMD         ON  CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_LINK_TESTS_SHARED   OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_BUILD_SMOKE_TESTS   OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_INSTALL_PROJECT     OFF CACHE BOOL "" FORCE)
set(DNV_VISTA_SDK_CPP_ENABLE_PACKAGING    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    vista
    GIT_REPOSITORY https://github.com/nfx-opensource/vista-sdk.git
    GIT_TAG        feature/cpp
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  cpp
)

FetchContent_MakeAvailable(vista)

#----------------------------------------------
# Application sources
#----------------------------------------------

set(APP_SOURCES
    src/main.cpp

    src/panels/GmodViewer.cpp
    src/panels/NodeDetails.cpp
    src/panels/LocalIdBuilder.cpp
    src/panels/ProjectManager.cpp
    src/ProjectSerializer.cpp
    src/Application.cpp
    ${IMGUI_SOURCES}
)

#----------------------------------------------
# Executable target
#----------------------------------------------

add_executable(${PROJECT_NAME} WIN32 ${APP_SOURCES})

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE /ENTRY:mainCRTStartup)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_NAME="${PROJECT_NAME}"
)

#----------------------------------------------
# Include directories
#----------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
)

#----------------------------------------------
# Link libraries
#----------------------------------------------

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME}  PRIVATE OpenGL::GL)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    dnv-vista-sdk-cpp
)
