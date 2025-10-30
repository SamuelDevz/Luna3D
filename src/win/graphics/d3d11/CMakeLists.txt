set(SOURCE_FILES src/Graphics.cpp)

if(SHARED_LIBRARIES)
    add_library(graphics SHARED ${SOURCE_FILES})
else()
    add_library(graphics STATIC ${SOURCE_FILES})
endif()

target_include_directories(graphics PUBLIC include)
target_link_libraries(graphics PRIVATE dxgi.lib d3d11.lib dxguid.lib)
target_link_libraries(graphics PUBLIC window)