#pragma once

#include "Types.h"

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
        string asciiMessage;
        wstring unicodeMessage;
        LogLevel level;

    public:
        Logger() noexcept;
        ~Logger() noexcept;

        void OutputDebug(const LogLevel level, const string_view message) noexcept;
        void OutputDebugW(const LogLevel level, const wstring_view message) noexcept;
    };
}