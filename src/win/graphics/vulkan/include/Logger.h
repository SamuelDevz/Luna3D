#pragma once

#include "Types.h"
#include "WinInclude.h"
#include <windows.h>

namespace Luna
{
    enum LogLevel
    {
        LOG_LEVEL_FATAL,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_WARN,
        LOG_LEVEL_INFO,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_TRACE
    };

    class Logger
    {
    private:
        HANDLE outputHandle;
        HANDLE errorHandle;

        void WriteToConsole(HANDLE handle, LogLevel level, string_view message) noexcept;
        void WriteToConsole(HANDLE handle, LogLevel level, wstring_view message) noexcept;

        void ApplyLevelColor(HANDLE handle, LogLevel level) noexcept;
        void ResetColors() noexcept;
        void ResetColor(HANDLE handle) noexcept;

    public:
        Logger() noexcept;
        ~Logger() noexcept;

        void OutputDebug(LogLevel level, string_view  message) noexcept;
        void OutputDebug(LogLevel level, wstring_view message) noexcept;
    };

    inline void Logger::ResetColor(HANDLE handle) noexcept
    {
        ApplyLevelColor(handle, LOG_LEVEL_INFO);
    }

    inline void Logger::ResetColors() noexcept
    {
        ResetColor(outputHandle);
        ResetColor(errorHandle);
    }
}