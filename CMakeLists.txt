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
# Application sources
#----------------------------------------------

set(APP_SOURCES
    src/main.cpp
)

#----------------------------------------------
# Executable target
#----------------------------------------------

add_executable(${PROJECT_NAME} ${APP_SOURCES})
