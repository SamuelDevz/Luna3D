#pragma once

#include <string>
#include <filesystem>
#include <source_location>
#include <iostream>

std::string GetCursorFile()
{
    const std::source_location& location = std::source_location::current();
    const std::filesystem::path baseDir = std::filesystem::path(location.file_name()).parent_path();
    std::string cursorPath = baseDir.string().substr(0, baseDir.string().size() - 8);
    cursorPath += "/resources/Cursor";
    return cursorPath;
}

std::string GetIconFile()
{
    const std::source_location& location = std::source_location::current();
    const std::filesystem::path baseDir = std::filesystem::path(location.file_name()).parent_path();
    std::string iconPath = baseDir.string().substr(0, baseDir.string().size() - 8);
    iconPath += "/resources/Linux.png";
    return iconPath;
}