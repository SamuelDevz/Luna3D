set(SOURCE_FILES src/Game.cpp
    src/Engine.cpp)

# window library
if(SHARED_LIBRARIES)
    add_library(window SHARED src/Window.cpp)
    target_compile_definitions(window PUBLIC ENGINE_EXPORTS)
else()
    add_library(window STATIC src/Window.cpp)
endif()

target_include_directories(window PUBLIC include)
target_compile_definitions(window PUBLIC $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>)

# engine library
if(SHARED_LIBRARIES)
    add_library(engine SHARED ${SOURCE_FILES})
else()
    add_library(engine STATIC ${SOURCE_FILES})
endif()

target_link_libraries(engine PRIVATE winmm.lib)
target_link_libraries(engine PUBLIC window)