set(SOURCE_FILES src/WinGame.cpp 
    src/main.cpp)

set(RESOURCE_FILES resources/Resources.h 
    resources/Resources.rc)

add_executable(simplewindow WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
target_include_directories(simplewindow PUBLIC include resources)
target_link_libraries(simplewindow PUBLIC Luna::Libraries)

if(SHARED_LIBRARIES)
    add_custom_command(TARGET simplewindow POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
            $<TARGET_RUNTIME_DLLS:simplewindow> 
            $<TARGET_FILE_DIR:simplewindow>
            COMMAND_EXPAND_LISTS)
endif()