#pragma once

#include <string>
#include <string_view>

namespace Luna
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using string = std::string;
    using wstring = std::wstring;
    using string_view = std::string_view;
    using wstring_view = std::wstring_view;

    using int8 = signed char;
    using int16 = signed short;
    using int32 = signed int;
    using int64 = signed long long;

    using uint8 = unsigned char;
    using uint16 = unsigned short;
    using uint32 = unsigned int;
    using uint64 = unsigned long long;
}