#include "Error.h"
#include <comdef.h>
#include <format>
using std::format;

namespace Luna
{
    Error::Error() noexcept : hrCode{}, lineNum{-1}
    {
    }

    Error::Error(const int32 hr, 
        const string_view func, 
        const string_view file, 
        const int32 line,
        const string_view message) noexcept
        : hrCode{hr}, funcName{func}, lineNum{line}, message{message}
    {
        auto pos = file.find_last_of('\\');

        if (pos != string::npos)
            fileName = file.substr(pos + 1);
    }

    string Error::ToString() const
    {
        _com_error err(static_cast<HRESULT>(hrCode));
        
        return format("{} failed in {}, line {}:\n{}\n{}",
            funcName, fileName, lineNum, err.ErrorMessage(), message);
    }
}