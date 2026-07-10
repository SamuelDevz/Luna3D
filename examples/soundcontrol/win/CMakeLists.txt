set(SOURCE_FILES src/SoundCtrl.cpp
    src/main.cpp)

set(RESOURCE_FILES resources/Resources.h
    resources/Resources.rc)

set(AUDIO_FILES resources/Intro.wav
    resources/Pulse.wav
    resources/Explosion.wav)

add_executable(soundcontrol WIN32 ${SOURCE_FILES} ${RESOURCE_FILES})
target_include_directories(soundcontrol PUBLIC include resources)
target_link_libraries(soundcontrol PUBLIC Luna::Libraries)

# create the directory in the path the executable
add_custom_command(TARGET soundcontrol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:soundcontrol>/resources
    COMMAND_EXPAND_LISTS)

# copy audio files for the resources
foreach(AUDIO_FILE ${AUDIO_FILES})
    add_custom_command(TARGET soundcontrol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${AUDIO_FILE}
            $<TARGET_FILE_DIR:soundcontrol>/resources)
endforeach()

if(SHARED_LIBRARIES)
    add_custom_command(TARGET soundcontrol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_RUNTIME_DLLS:soundcontrol>
            $<TARGET_FILE_DIR:soundcontrol>
            COMMAND_EXPAND_LISTS)
endif()