#pragma once

#include "Types.h"
#include "Export.h"
#include <wayland-client.h>

namespace Luna
{
    class DLL Error
    {
    private:
        int32 errorCode;
        uint32 objectId;
        string interfaceName;
        string message;
        string funcName;
        string fileName;
        int32 lineNum;

    public:
        explicit Error(wl_display* display,
            const string_view func, 
            const string_view file, 
            const int32 line,
            const string_view msg = "") noexcept;

        virtual string ToString() const;
    };

    #ifndef ThrowIfWaylandFailed
    #define ThrowIfWaylandFailed(display)                                                \
    {                                                                                    \
        if(wl_display_get_error(display)) {                                              \
            throw Error(display, __func__, __FILE__, __LINE__, "Wayland Fatal");  \
        }                                                                                \
    }
    #endif
}