#pragma once

#include "Types.h"
#include "Export.h"

typedef struct _XDisplay Display;

namespace Luna
{
    class DLL Error
    {
    private:
        Display * display;
        int32 hrCode;
        int32 lineNum;
        string message;
        string funcName;
        string fileName;
        
    public:
        explicit Error() noexcept;
        explicit Error(Display * display,
            const int32 hr, 
            const string_view func, 
            const string_view file, 
            const int32 line,
            const string_view message = "") noexcept;
        virtual string ToString();
    };

    #ifndef ThrowIfFailed
    #define ThrowIfFailed(display, x)                                            \
    {                                                                            \
        int32 hr = (x);                                                          \
        if(hr != 0) { throw Error(display, hr, __func__, __FILE__, __LINE__); }  \
    }
    #endif

    #ifndef ThrowIfFailure
    #define ThrowIfFailure(display, x, msg)                                           \
    {                                                                                 \
        int32 hr = (x);                                                               \
        if(hr != 0) { throw Error(display, hr, __func__, __FILE__, __LINE__, msg); }  \
    }
    #endif

    #ifndef ThrowIfError
    #define ThrowIfError(display, x, condicional)                                   \
    {                                                                               \
        if(condicional) { throw Error(display, x, __func__, __FILE__, __LINE__); }  \
    }
    #endif

    #ifndef ThrowIfErrorMessage
    #define ThrowIfErrorMessage(display, x, condicional, msg)                            \
    {                                                                                    \
        if(condicional) { throw Error(display, x, __func__, __FILE__, __LINE__, msg); }  \
    }
    #endif
}