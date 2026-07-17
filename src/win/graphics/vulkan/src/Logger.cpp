#include "Logger.h"
#include <format>
using std::format;

#if !defined(__GNUC__)
    #include <ConsoleApi2.h>
    #include <ConsoleApi.h>
#endif

namespace Luna
{
    enum ForeGroundColors
    {
        BLUE = 1,
        GREEN = 2,
        RED = 4,
        YELLOW = 6,
        WHITE = 7,
        GRAY = 8
    };

    enum BackGroundColors
    {
        DARK_RED = 64
    };

    Logger::Logger() noexcept
    {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        errorHandle = GetStdHandle(STD_ERROR_HANDLE);
    }

    Logger::~Logger() noexcept
    {
        FreeConsole();
    }

    void SetTextAttribute(HANDLE handle, const LogLevel level)
    {
        // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
        static uint8 LOG_LEVEL_COLORS[6]
        {
            BackGroundColors::DARK_RED,
            ForeGroundColors::RED,
            ForeGroundColors::YELLOW,
            ForeGroundColors::GREEN,
            ForeGroundColors::BLUE,
            ForeGroundColors::GRAY
        };
        SetConsoleTextAttribute(handle, LOG_LEVEL_COLORS[level]);
    }

    void Logger::WriteToConsole(const LogLevel level, const string_view message) noexcept
    {
        SetTextAttribute(outputHandle, level);
        OutputDebugStringA(message.data());
        WriteConsoleA(outputHandle, message.data(), message.size(), nullptr, nullptr);
    }

    void Logger::WriteToConsole(const LogLevel level, const wstring_view message) noexcept
    {
        SetTextAttribute(outputHandle, level);
        OutputDebugStringW(message.data());
        WriteConsoleW(outputHandle, message.data(), message.size(), nullptr, nullptr);
    }

    void Logger::WriteToConsoleError(const LogLevel level, const string_view message) noexcept
    {
        SetTextAttribute(errorHandle, level);
        OutputDebugStringA(message.data());
        WriteConsoleA(outputHandle, message.data(), message.size(), nullptr, nullptr);
    }

    void Logger::WriteToConsoleError(const LogLevel level, const wstring_view message) noexcept
    {
        SetTextAttribute(errorHandle, level);
        OutputDebugStringW(message.data());
        WriteConsoleW(outputHandle, message.data(), message.size(), nullptr, nullptr);
    }

    void Logger::OutputDebug(const LogLevel level, const string_view message) noexcept
    {
        const bool isError = level < LOG_LEVEL_WARN;

        static constexpr const char* LOG_LEVEL_STRINGS[6]
        {
            "[FATAL]:",
            "[ERROR]:",
            "[WARN]:",
            "[INFO]:",
            "[DEBUG]:",
            "[TRACE]:"
        };

        string outputMessage = format("{} {}", LOG_LEVEL_STRINGS[level], message);

        if (isError) WriteToConsoleError(level, outputMessage);
        else         WriteToConsole(level, outputMessage);

        SetConsoleTextAttribute(outputHandle, ForeGroundColors::WHITE);
        SetConsoleTextAttribute(errorHandle, ForeGroundColors::WHITE);
    }

    void Logger::OutputDebug(const LogLevel level, const wstring_view message) noexcept
    {
        const bool isError = level < LOG_LEVEL_WARN;

        static constexpr const wchar_t* LOG_LEVEL_STRINGS_W[6]
        {
            L"[FATAL]:",
            L"[ERROR]:",
            L"[WARN]:",
            L"[INFO]:",
            L"[DEBUG]:",
            L"[TRACE]:"
        };

        wstring outputMessage = format(L"{} {}", LOG_LEVEL_STRINGS_W[level], message);

        if (isError) WriteToConsoleError(level, outputMessage);
        else         WriteToConsole(level, outputMessage);

        SetConsoleTextAttribute(outputHandle, ForeGroundColors::WHITE);
        SetConsoleTextAttribute(errorHandle, ForeGroundColors::WHITE);
    }
}
