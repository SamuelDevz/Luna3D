#pragma once

#include "Types.h"

namespace Luna
{
    using HANDLE = void*;

    enum class LogLevel : uint8
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

        string bufferA;
        wstring bufferW;

        static constexpr uint32 MAX_LOG_BUFFER = 2048;
        void SetTextAttribute(HANDLE handle, const LogLevel level) noexcept;

    public:
        Logger() noexcept;
        ~Logger() noexcept;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void OutputDebug(const LogLevel level, const string_view message) noexcept;
        void OutputDebug(const LogLevel level, const wstring_view message) noexcept;
    };
}