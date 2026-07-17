#pragma once

#include "Types.h"
#include "Export.h"
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

    class DLL Logger
    {
    private:
        HANDLE outputHandle;
        HANDLE errorHandle;

        void WriteToConsole(HANDLE handle, const LogLevel level, const string_view message) noexcept;
        void WriteToConsole(HANDLE handle, const LogLevel level, const wstring_view message) noexcept;

        void ApplyLevelColor(HANDLE handle, const LogLevel level) noexcept;
        void ResetColors() noexcept;
        void ResetColor(HANDLE handle) noexcept;

    public:
        Logger() noexcept;
        ~Logger() noexcept;

        void OutputDebug(const LogLevel level, const string_view message) noexcept;
        void OutputDebug(const LogLevel level, const wstring_view message) noexcept;
    };
}