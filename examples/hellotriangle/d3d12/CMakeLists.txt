set(SOURCE_FILES src/Renderer.cpp
    src/Triangle.cpp
    src/main.cpp)

set(RESOURCE_FILES resources/Resources.h 
    resources/Resource.rc)

add_executable(triangle WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
target_include_directories(triangle PUBLIC include resources)
target_link_libraries(triangle PUBLIC Luna::Libraries)
target_link_libraries(triangle PRIVATE d3dcompiler.lib)