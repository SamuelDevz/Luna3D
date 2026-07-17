cmake_minimum_required(VERSION 3.5...4.1)
project(Luna3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build examples programs" OFF)
option(SHARED_LIBRARIES "Use the shared libs" OFF)
option(BUILD_DIRECT3D11 "Build the engine using Direct3D 11." OFF)
option(BUILD_DIRECT3D12 "Build the engine using Direct3D 12." OFF)

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    option(BUILD_X11 "Build the engine using Xlib" OFF)
    option(BUILD_XCB "Build the engine using XCB" OFF)
endif ()

add_subdirectory(src)

add_library(Luna-Libraries INTERFACE)
add_library(Luna::Libraries ALIAS Luna-Libraries)
target_link_libraries(Luna-Libraries INTERFACE engine graphics window)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()