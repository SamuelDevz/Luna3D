#include "Error.h"
#include <format>
using std::format;

namespace Luna
{
    Error::Error() noexcept : connection{nullptr}, context{nullptr}, cookie{}, lineNum{-1}
    {
    }

    Error::Error(xcb_connection_t * connection,
        const xcb_void_cookie_t cookie, 
        xcb_generic_error_t * error,
        const string_view func, 
        const string_view file, 
        const int32 line,
        const string_view message) noexcept
        : connection{connection}, 
        cookie{cookie}, 
        error{error},
        funcName{func}, 
        lineNum{line}, 
        message{message}, 
        context{nullptr}
    {
        xcb_errors_context_new(connection, &context);
        auto pos = file.find_last_of('/');

        if (pos != string::npos)
            fileName = file.substr(pos + 1);
    }

    Error::~Error() noexcept
    {
        xcb_errors_context_free(context);
        if(error) delete error;
    }

    string Error::ToString() const
    {
        const char * errorName = xcb_errors_get_name_for_error(context, error->error_code, nullptr);
        string buffer = format("Captured XCB error: {}\nMajor code: {}\nMinor code: {}",
            errorName ? errorName : "Unknow", error->error_code,
            xcb_errors_get_name_for_major_code(context, error->major_code),
            xcb_errors_get_name_for_minor_code(context, error->major_code, error->minor_code)
        );

        return format("{} failed in {}, line {}:\n{}\n{}",
            funcName, fileName, lineNum, buffer, message);
    }
}