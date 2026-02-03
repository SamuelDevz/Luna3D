set(SOURCE_FILES src/VkError.cpp
    src/Mesh.cpp
    src/Logger.cpp
    src/ValidationLayer.cpp
    src/Graphics.cpp)

if(SHARED_LIBRARIES)
    add_library(graphics SHARED ${SOURCE_FILES})
else()
    add_library(graphics STATIC ${SOURCE_FILES})
endif()

find_package(Vulkan REQUIRED)

target_include_directories(graphics PUBLIC include ${Vulkan_INCLUDE_DIR})
target_link_libraries(graphics PRIVATE Vulkan::Vulkan)
target_link_libraries(graphics PUBLIC window)