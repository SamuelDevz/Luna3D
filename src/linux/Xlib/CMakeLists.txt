set(SOURCE_FILES src/Timer.cpp
    src/Input.cpp
    src/Game.cpp
    src/Engine.cpp)

find_package(X11 REQUIRED COMPONENTS Xcursor Xrandr Xinerama)
find_package(PNG REQUIRED)

set(LIBRARIES X11::X11 
    X11::Xcursor
    X11::Xrandr
    X11::Xinerama)

# window library
if(SHARED_LIBRARIES)
    add_library(window SHARED src/Window.cpp src/Error.cpp src/MessageBox.cpp)
else()
    add_library(window STATIC src/Window.cpp src/Error.cpp src/MessageBox.cpp)
endif()

target_include_directories(window PUBLIC include ${X11_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
target_link_libraries(window PUBLIC ${LIBRARIES} ${PNG_LIBRARIES})
target_compile_definitions(window PUBLIC $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>)

# graphics library
include(FetchContent)
FetchContent_Declare(
    graphics
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../graphics
)
FetchContent_MakeAvailable(graphics)
target_compile_definitions(graphics PUBLIC PLATFORM_XLIB)

# engine library
if(SHARED_LIBRARIES)
    add_library(engine SHARED ${SOURCE_FILES})
else()
    add_library(engine STATIC ${SOURCE_FILES})
endif()

target_link_libraries(engine PUBLIC window graphics)