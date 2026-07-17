set(SOURCE_FILES src/WinGame.cpp 
    src/main.cpp)

add_executable(simplewindow ${SOURCE_FILES})
target_include_directories(simplewindow PUBLIC include)
target_link_libraries(simplewindow PUBLIC Luna::Libraries)

if(SHARED_LIBRARIES)
    add_custom_command(TARGET simplewindow POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_RUNTIME_DLLS:simplewindow>
            $<TARGET_FILE_DIR:simplewindow>
            COMMAND_EXPAND_LISTS)
endif()