#include "Error.h"
#include <X11/Xlib.h>
#include <format>
using std::format;

namespace Luna
{
    Error::Error() noexcept : hrCode{}, lineNum{-1}
    {
    }

    Error::Error(Display * display,
        const int32 hr, 
        const string_view func, 
        const string_view file, 
        const int32 line,
        const string_view message) noexcept
        : display{display}, hrCode{hr}, funcName{func}, lineNum{line}, message{message}
    {
        auto pos = file.find_last_of('/');

        if (pos != string::npos)
            fileName = file.substr(pos + 1);
    }

    string Error::ToString()
    {
        char buffer[1024];
        XGetErrorText(display, hrCode, buffer, sizeof(buffer));

        return format("{} failed in {}, line {}:\n{}\n{}",
            funcName, fileName, lineNum, buffer, message);
    }
}