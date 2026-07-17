set(SOURCE_FILES src/Logger.cpp
    src/DebugLayer.cpp
    src/Graphics.cpp)

add_library(graphics STATIC ${SOURCE_FILES})

target_include_directories(graphics PUBLIC include)
target_link_libraries(graphics PRIVATE dxgi.lib d3d11.lib dxguid.lib)
target_link_libraries(graphics PUBLIC window)