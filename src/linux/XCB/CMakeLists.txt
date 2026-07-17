set(SOURCE_FILES src/Input.cpp
    src/Game.cpp
    src/Engine.cpp)

find_package(X11 REQUIRED COMPONENTS Xcursor xkbcommon xkbcommon_X11 xcb xcb_keysyms X11_xcb xcb_icccm xcb_errors)
find_package(PNG REQUIRED)

set(LIBRARIES X11::xcb
    X11::X11_xcb
    X11::xcb_icccm
    X11::xcb_errors
    X11::xcb_keysyms
    X11::xkbcommon
    X11::xkbcommon_X11
    X11::Xcursor
    PNG::PNG)

# window library
if(SHARED_LIBRARIES)
    add_library(window SHARED src/Window.cpp src/Error.cpp src/MessageBox.cpp)
else()
    add_library(window STATIC src/Window.cpp src/Error.cpp src/MessageBox.cpp)
endif()

target_include_directories(window PUBLIC include ${LIBRARIES})
target_link_libraries(window PUBLIC ${LIBRARIES})
target_compile_definitions(window PUBLIC $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>)

# engine library
if(SHARED_LIBRARIES)
    add_library(engine SHARED ${SOURCE_FILES})
else()
    add_library(engine STATIC ${SOURCE_FILES})
endif()

target_link_libraries(engine PUBLIC window)