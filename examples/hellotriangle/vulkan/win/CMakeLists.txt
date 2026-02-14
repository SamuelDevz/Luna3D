set(SOURCE_FILES src/Renderer.cpp
    src/Triangle.cpp
    src/main.cpp)

set(RESOURCE_FILES resources/Resources.h 
    resources/Resource.rc)

set(SHADER_FILES resources/Shaders/Vertex.vert
    resources/Shaders/Fragment.frag)

# Create output directory for compiled SPIR-V shaders
add_custom_target(directory)
add_custom_command(TARGET directory POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:triangle>/Shaders
    COMMAND_EXPAND_LISTS)

# Compile GLSL shader files to SPIR-V
add_custom_target(shaders)
foreach(FILE ${SHADER_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)
    set(SHADER_DIRECTORY $<TARGET_FILE_DIR:triangle>/Shaders)
    add_custom_command(TARGET shaders POST_BUILD
        COMMAND glslangValidator -V ${FILE} -o ${SHADER_DIRECTORY}/${FILE_WE}.spv
        COMMENT "Compiling GLSL to SPIR-V: ${FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM)
endforeach(FILE)

add_executable(triangle WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
target_include_directories(triangle PUBLIC include resources)
target_link_libraries(triangle PUBLIC Luna::Libraries)

add_dependencies(triangle directory shaders)

if(SHARED_LIBRARIES)
    add_custom_command(TARGET triangle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_RUNTIME_DLLS:triangle> 
            $<TARGET_FILE_DIR:triangle>
            COMMAND_EXPAND_LISTS)
endif()