cmake_minimum_required(VERSION 4.1.0)
project(NativeActivity LANGUAGES CXX C)

set(NATIVE_APP_GLUE_INCLUDE ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(native_app_glue STATIC ${NATIVE_APP_GLUE_INCLUDE}/android_native_app_glue.c)

add_library(native-activity SHARED main.cpp)

target_include_directories(native-activity PRIVATE ${NATIVE_APP_GLUE_INCLUDE})

target_link_libraries(native-activity
    android
    -Wl,--whole-archive native_app_glue -Wl,--no-whole-archive
    log
)

if(ANDROID_ABI STREQUAL riscv64)
    target_compile_options(native-activity PRIVATE -Wno-deprecated-declarations)
endif()