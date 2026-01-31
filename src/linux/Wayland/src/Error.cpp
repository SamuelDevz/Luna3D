#include "Error.h"
#include <format>
using std::format;

namespace Luna
{
    Error::Error(wl_display* display,
        const string_view func, 
        const string_view file, 
        const int32 line,
        const string_view msg) noexcept
        : funcName{func}, lineNum{line}, message{msg}, objectId{}, errorCode{}
    {
        const struct wl_interface* wayland_interface = nullptr;
        uint32_t protocol_id = 0;
        
        errorCode = wl_display_get_protocol_error(display, &wayland_interface, &protocol_id);
        
        if (wayland_interface) 
        {
            interfaceName = wayland_interface->name;
            objectId = protocol_id;
        }

        if (errorCode == 0) 
            errorCode = wl_display_get_error(display);

        auto pos = file.find_last_of('/');
        fileName = (pos != string::npos) ? file.substr(pos + 1) : file;
    }

    string Error::ToString() const
    {
        string buffer = format(
            "Wayland Error: {}\nInterface: {}\nObject ID: {}",
            errorCode,
            interfaceName.empty() ? "Unknown" : interfaceName,
            objectId
        );

        return format("{} failed in {}, line {}:\n{}\n{}",
            funcName, fileName, lineNum, buffer, message);
    }
}