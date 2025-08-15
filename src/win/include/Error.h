#pragma once

#include "Types.h"

namespace Luna
{
    class Error
    {
    private:
        int32 hrCode;
        int32 lineNum;
        string message;
        string funcName;
        string fileName;
        
    public:
        explicit Error() noexcept;
        explicit Error(const int32 hr, 
            const string_view func, 
            const string_view file, 
            const int32 line,
            const string_view message = "") noexcept;
        virtual string ToString() const;
    };

    #ifndef ThrowIfFailed
    #define ThrowIfFailed(x)                                               \
    {                                                                      \
        int32 hr = (x);                                                    \
        if(FAILED(hr)) { throw Error(hr, __func__, __FILE__, __LINE__); }  \
    }
    #endif

    #ifndef ThrowIfFailure
    #define ThrowIfFailure(x, msg)                                              \
    {                                                                           \
        int32 hr = (x);                                                         \
        if(FAILED(hr)) { throw Error(hr, __func__, __FILE__, __LINE__, msg); }  \
    }
    #endif

    #ifndef ThrowIfError
    #define ThrowIfError(x, condicional)                                    \
    {                                                                       \
        if(condicional) { throw Error(hr, __func__, __FILE__, __LINE__); }  \
    }
    #endif

    #ifndef ThrowIfErrorMessage
    #define ThrowIfErrorMessage(x, condicional, msg)                             \
    {                                                                            \
        if(condicional) { throw Error(hr, __func__, __FILE__, __LINE__, msg); }  \
    }
    #endif
}