set(SOURCE_FILES src/WinGame.cpp 
    src/main.cpp)

set(RESOURCE_FILES resources/Resources.h 
    resources/Resources.rc)
    
add_executable(simplewindow WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
target_include_directories(simplewindow PUBLIC include resources)
target_link_libraries(simplewindow PRIVATE Luna::Libraries)