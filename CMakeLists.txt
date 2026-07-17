cmake_minimum_required(VERSION 3.5...4.1)
project(Luna3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build examples programs" OFF)
option(SHARED_LIBRARIES "Use the shared libs" OFF)

add_subdirectory(src)

add_library(Luna-Libraries INTERFACE)
add_library(Luna::Libraries ALIAS Luna-Libraries)
target_link_libraries(Luna-Libraries INTERFACE engine window)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()