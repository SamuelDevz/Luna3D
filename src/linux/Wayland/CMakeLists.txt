set(SOURCE_FILES src/Game.cpp
    src/Engine.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
    wayland-client
    wayland-cursor
)

# window library
add_library(protocols STATIC src/xdg-shell-client-protocol.c
    src/xdg-decoration-unstable-v1.c)

if(SHARED_LIBRARIES)
    add_library(window SHARED src/Window.cpp)
else()
    add_library(window STATIC src/Window.cpp)
endif()

target_link_libraries(window PUBLIC protocols)

target_include_directories(window PUBLIC include ${WAYLAND_INCLUDE_DIRS})
target_link_libraries(window PUBLIC PkgConfig::WAYLAND)
target_compile_definitions(window PUBLIC $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>)

target_compile_options(window PUBLIC -g)

# engine library
if(SHARED_LIBRARIES)
    add_library(engine SHARED ${SOURCE_FILES})
else()
    add_library(engine STATIC ${SOURCE_FILES})
endif()

target_link_libraries(engine PUBLIC window)