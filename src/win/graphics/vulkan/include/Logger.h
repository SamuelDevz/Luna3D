#pragma once

#include "Types.h"
#include "WinInclude.h"
#include <windows.h>

namespace Luna
{
    enum LogLevel
    {
        LOG_LEVEL_FATAL = 0,
        LOG_LEVEL_ERROR = 1,
        LOG_LEVEL_WARN = 2,
        LOG_LEVEL_INFO = 3,
        LOG_LEVEL_DEBUG = 4,
        LOG_LEVEL_TRACE = 5
    };

    class Logger
    {
    private:
        HANDLE outputHandle;
        HANDLE errorHandle;

        void WriteToConsole(const LogLevel level, const wstring_view message) noexcept;
        void WriteToConsole(const LogLevel level, const string_view message) noexcept;

        void WriteToConsoleError(const LogLevel level, const wstring_view message) noexcept;
        void WriteToConsoleError(const LogLevel level, const string_view message) noexcept;

    public:
        Logger() noexcept;
        ~Logger() noexcept;

        void OutputDebug(const LogLevel level, const string_view message) noexcept;
        void OutputDebug(const LogLevel level, const wstring_view message) noexcept;
    };
}