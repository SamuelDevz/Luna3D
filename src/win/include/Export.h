#pragma once

#if !defined(ENGINE_EXPORT)
    #define DLL
#else
    #if defined(_WIN32)
        #if defined(ENGINE_EXPORT)
            #define DLL __declspec(dllexport)
        #else
            #define DLL __declspec(dllimport)
        #endif
        
        #pragma warning(disable: 4251)
    #elif defined(__GNUC__) && defined(ENGINE_EXPORT)
        #define DLL __attribute__((visibility("default")))
    #endif
#endif