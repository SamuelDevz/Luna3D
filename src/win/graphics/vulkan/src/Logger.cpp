#include "Logger.h"
#include <format>
using std::format;

#if !defined(__GNUC__)
    #include <ConsoleApi2.h>
    #include <ConsoleApi.h>
#endif

namespace Luna
{
    Logger::Logger() noexcept
    {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        errorHandle  = GetStdHandle(STD_ERROR_HANDLE);
    }

    Logger::~Logger() noexcept
    {
        FreeConsole();
    }

    void Logger::ApplyLevelColor(HANDLE handle, LogLevel level) noexcept
    {
        enum ForegroundColor : WORD
        {
            COLOR_BLUE   = FOREGROUND_BLUE,
            COLOR_GREEN  = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
            COLOR_RED    = FOREGROUND_RED | FOREGROUND_INTENSITY,
            COLOR_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
            COLOR_WHITE  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
            COLOR_GRAY   = FOREGROUND_INTENSITY
        };

        enum BackgroundColor : WORD
        {
            COLOR_BG_DARK_RED = BACKGROUND_RED
        };

        const constexpr WORD LOG_LEVEL_COLORS[]
        {
            COLOR_BG_DARK_RED,
            COLOR_RED,
            COLOR_YELLOW,
            COLOR_GREEN,
            COLOR_BLUE,
            COLOR_GRAY
        };

        SetConsoleTextAttribute(handle, LOG_LEVEL_COLORS[level]);
    }

    void Logger::WriteToConsole(HANDLE handle, LogLevel level, string_view message) noexcept
    {
        ApplyLevelColor(handle, level);

        const string owned(message);
        OutputDebugStringA(owned.c_str());
        WriteConsoleA(outputHandle, message.data(), static_cast<DWORD>(message.size()), nullptr, nullptr);
    }

    void Logger::WriteToConsole(HANDLE handle, LogLevel level, wstring_view message) noexcept
    {
        ApplyLevelColor(handle, level);

        const wstring owned(message);
        OutputDebugStringW(owned.c_str());
        WriteConsoleW(outputHandle, message.data(), static_cast<DWORD>(message.size()), nullptr, nullptr);
    }

    void Logger::OutputDebug(const LogLevel level, const string_view message) noexcept
    {
        constexpr const char* LOG_LEVEL_PREFIX[]
        {
            "[FATAL]: ",
            "[ERROR]: ",
            "[WARN]:  ",
            "[INFO]:  ",
            "[DEBUG]: ",
            "[TRACE]: "
        };

        const bool isError = (level < LOG_LEVEL_WARN);
        const string output = format("{}{}", LOG_LEVEL_PREFIX[level], message);

        WriteToConsole((isError ? errorHandle : outputHandle), level, output);

        ResetColors();
    }

    void Logger::OutputDebug(const LogLevel level, const wstring_view message) noexcept
    {
        constexpr const wchar_t* LOG_LEVEL_PREFIX[]
        {
            L"[FATAL]: ",
            L"[ERROR]: ",
            L"[WARN]:  ",
            L"[INFO]:  ",
            L"[DEBUG]: ",
            L"[TRACE]: "
        };

        const bool isError = (level < LOG_LEVEL_WARN);
        const wstring output = format(L"{}{}", LOG_LEVEL_PREFIX[level], message);

        WriteToConsole((isError ? errorHandle : outputHandle), level, output);

        ResetColors();
    }
}