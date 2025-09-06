#pragma once

#include <string>

std::string GetCursorFile()
{
    std::string pathCursor = __FILE__;
    pathCursor = pathCursor.substr(0, 58);
    pathCursor += "/resources/Cursor";
    return pathCursor;
}

std::string GetIconFile()
{
    std::string pathCursor = __FILE__;
    pathCursor = pathCursor.substr(0, 58);
    pathCursor += "/resources/linux.png";
    return pathCursor;
}