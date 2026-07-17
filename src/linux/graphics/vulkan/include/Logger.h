#pragma once

#include "Types.h"
#include "Export.h"

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
        void* outputHandle;
        void* errorHandle;
        void* pipeStream;

        void WriteToConsole(void* handle, const LogLevel level, string_view message) noexcept;

        void ApplyLevelColor(void* handle, const LogLevel level) noexcept;
        void ResetColor(void* handle) noexcept;
        void ResetColors() noexcept;

        void AllocConsole() noexcept;
        
    public:
        Logger() noexcept;
        ~Logger() noexcept;

        void OutputDebug(const LogLevel level, const string_view message) noexcept;
        void OutputDebug(const LogLevel level, const wstring_view message) noexcept;
    };
}