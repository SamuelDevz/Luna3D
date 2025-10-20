#pragma once

#include "Types.h"
#include "Export.h"
#include <xcb/xcb_errors.h>

namespace Luna
{
    class DLL Error
    {
    private:
        xcb_connection_t* connection;
        xcb_errors_context_t* context;
        xcb_generic_error_t* error;
        xcb_void_cookie_t cookie;
        int32 lineNum;
        string message;
        string funcName;
        string fileName;
        
    public:
        explicit Error() noexcept;
        explicit Error(xcb_connection_t * connection,
            const xcb_void_cookie_t cookie,
            xcb_generic_error_t * error,
            const string_view func, 
            const string_view file, 
            const int32 line,
            const string_view message = "") noexcept;
        ~Error() noexcept;
        virtual string ToString() const;
    };

    #ifndef ThrowIfFailed
    #define ThrowIfFailed(connection, cookie)                                                \
    {                                                                                        \
        xcb_generic_error_t * error = xcb_request_check(connection, cookie);                 \
        if(error) { throw Error(connection, cookie, error, __func__, __FILE__, __LINE__); }  \
    }
    #endif

    #ifndef ThrowIfFailure
    #define ThrowIfFailure(connection, cookie, msg)                                               \
    {                                                                                             \
        xcb_generic_error_t * error = xcb_request_check(connection, cookie);                      \
        if(error) { throw Error(connection, cookie, error, __func__, __FILE__, __LINE__, msg); }  \
    }
    #endif

    #ifndef ThrowIfError
    #define ThrowIfError(connection, cookie, error, condicional)                                   \
    {                                                                                              \
        if(condicional) { throw Error(connection, cookie, error, __func__, __FILE__, __LINE__); }  \
    }
    #endif

    #ifndef ThrowIfErrorMessage
    #define ThrowIfErrorMessage(connection, cookie, error, condicional, msg)                            \
    {                                                                                                   \
        if(condicional) { throw Error(connection, cookie, error, __func__, __FILE__, __LINE__, msg); }  \
    }
    #endif
}