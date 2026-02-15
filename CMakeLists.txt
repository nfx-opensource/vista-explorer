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

#----------------------------------------------
# Dependencies - ImGui
#----------------------------------------------

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
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

#----------------------------------------------
# Dependencies - GLFW
#----------------------------------------------

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

#----------------------------------------------
# Application sources
#----------------------------------------------

set(APP_SOURCES
    src/main.cpp
    ${IMGUI_SOURCES}
)

#----------------------------------------------
# Executable target
#----------------------------------------------

add_executable(${PROJECT_NAME} ${APP_SOURCES})

#----------------------------------------------
# Include directories
#----------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

#----------------------------------------------
# Link libraries
#----------------------------------------------

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME}  PRIVATE OpenGL::GL)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
)
