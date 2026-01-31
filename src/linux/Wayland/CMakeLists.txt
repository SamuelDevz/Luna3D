set(SOURCE_FILES src/Input.cpp
    src/Timer.cpp
    src/Game.cpp
    src/Engine.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET
    wayland-client
    wayland-cursor
)

pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET
    xkbcommon
)

# window library
add_library(protocols STATIC src/xdg-shell-client-protocol.cpp
    src/xdg-decoration-unstable-v1.cpp)
target_include_directories(protocols PUBLIC include)

if(SHARED_LIBRARIES)
    add_library(window SHARED src/Error.cpp src/MessageBox.cpp src/Window.cpp)
else()
    add_library(window STATIC src/Error.cpp src/MessageBox.cpp src/Window.cpp)
endif()

target_link_libraries(window PUBLIC protocols)

target_include_directories(window PUBLIC include ${WAYLAND_INCLUDE_DIRS} ${XKBCOMMON_INCLUDE_DIRS})
target_link_libraries(window PUBLIC PkgConfig::WAYLAND PkgConfig::XKBCOMMON)
target_compile_definitions(window PUBLIC $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>)

target_compile_options(window PUBLIC -g)

# engine library
if(SHARED_LIBRARIES)
    add_library(engine SHARED ${SOURCE_FILES})
else()
    add_library(engine STATIC ${SOURCE_FILES})
endif()

target_link_libraries(engine PUBLIC window)