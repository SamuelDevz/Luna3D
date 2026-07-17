#pragma once

#if !defined(ENGINE_EXPORT)
    #define DLL
#else
    #if defined(__GNUC__) && defined(ENGINE_EXPORT)
        #define DLL __attribute__((visibility("default")))
    #endif
#endif