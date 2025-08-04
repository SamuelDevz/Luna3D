#include "Logger.h"
#include "WinInclude.h"
#include <windows.h>
#include <format>
using std::format;

#if defined(_DEBUG) && !defined(__GNUC__)
    #include <ConsoleApi2.h>
    #include <ConsoleApi.h>
#endif

namespace Luna
{
    // Cores dos níveis de log: FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static constexpr const uint8 LOG_LEVEL_COLORS[6] = {64, 4, 6, 2, 1, 8};
    
    Logger::Logger() noexcept
    {
        AllocConsole();
    }

    Logger::~Logger() noexcept
    {
        FreeConsole();
    }

    static void PlatformConsoleWrite(const LogLevel level, const string_view message) 
    {
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(consoleHandle, LOG_LEVEL_COLORS[level]);
        OutputDebugString(message.data());
        WriteConsoleA(consoleHandle, message.data(), message.size(), nullptr, nullptr);
    }

    static void PlatformConsoleWriteUnicode(const LogLevel level, const wstring_view message) 
    {
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(consoleHandle, LOG_LEVEL_COLORS[level]);
        OutputDebugStringW(message.data());
        WriteConsoleW(consoleHandle, message.data(), message.size(), nullptr, nullptr);
    }

    static void PlatformConsoleWriteError(const LogLevel level, const string_view message)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), LOG_LEVEL_COLORS[level]);
        OutputDebugString(message.data());
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message.data(), message.size(), nullptr, nullptr);
    }

    static void PlatformConsoleWriteErrorUnicode(const LogLevel level, const wstring_view message)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), LOG_LEVEL_COLORS[level]);
        OutputDebugStringW(message.data());
        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), message.data(), message.size(), nullptr, nullptr);
    }

    void Logger::OutputDebug(const LogLevel level, const string_view message) noexcept
    {
        static const char* LOG_LEVEL_STRINGS[6]
        {
            "[FATAL]:", 
            "[ERROR]:", 
            "[WARN]:", 
            "[INFO]:", 
            "[DEBUG]:", 
            "[TRACE]:"
        };

        const bool isError = level < LOG_LEVEL_WARN;

        asciiMessage = format("{} {}", LOG_LEVEL_STRINGS[level], message);

        if (isError) PlatformConsoleWriteError(level, asciiMessage);
        else         PlatformConsoleWrite(level, asciiMessage);
    }

    void Logger::OutputDebugW(const LogLevel level, const wstring_view message) noexcept
    {
        static const wchar_t* LOG_LEVEL_STRINGS_W[6]
        {
            L"[FATAL]:", 
            L"[ERROR]:", 
            L"[WARN]:", 
            L"[INFO]:", 
            L"[DEBUG]:", 
            L"[TRACE]:"
        };

        const bool isError = level < LOG_LEVEL_WARN;

        unicodeMessage = format(L"{} {}", LOG_LEVEL_STRINGS_W[level], message);

        if (isError) PlatformConsoleWriteErrorUnicode(level, unicodeMessage);
        else         PlatformConsoleWriteUnicode(level, unicodeMessage);
    }
}