#include "Logger.h"
#include "WinInclude.h"
#include <windows.h>
#include <format>
using std::format_to;

#if !defined(__GNUC__)
    #include <ConsoleApi2.h>
    #include <ConsoleApi.h>
#endif

#ifndef CP_UTF8
#define CP_UTF8 65001
#endif

namespace Luna
{
    enum ForeGroundColors : uint16
    {
        BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
        GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        RED = FOREGROUND_RED | FOREGROUND_INTENSITY,
        YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        GRAY = FOREGROUND_INTENSITY
    };

    enum BackGroundColors : uint16
    {
        DARK_RED = BACKGROUND_RED
    };

    Logger::Logger() noexcept
    {
        AllocConsole();
        outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        errorHandle = GetStdHandle(STD_ERROR_HANDLE);
        SetConsoleOutputCP(CP_UTF8);
        
        bufferA.reserve(2048);
        bufferW.reserve(2048);
    }

    Logger::~Logger() noexcept
    {
        FreeConsole();
    }

    void Logger::SetTextAttribute(HANDLE handle, const LogLevel level) noexcept
    {
        static const uint16 LOG_LEVEL_COLORS[6] = 
        {
            (BackGroundColors::DARK_RED | ForeGroundColors::WHITE),
            ForeGroundColors::RED,
            ForeGroundColors::YELLOW,
            ForeGroundColors::GREEN,
            ForeGroundColors::BLUE,
            ForeGroundColors::GRAY
        };
        SetConsoleTextAttribute(handle, LOG_LEVEL_COLORS[static_cast<size_t>(level)]);
    }

    void Logger::OutputDebug(const LogLevel level, const std::string_view message) noexcept
    {
        const bool isError = level <= LogLevel::LOG_LEVEL_ERROR;
        HANDLE target = isError ? errorHandle : outputHandle;

        static constexpr const char* LABELS[6] = 
        {
            "[FATAL]: ", "[ERROR]: ", "[WARN]:  ",
            "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "
        };

        bufferA.clear();

        format_to(std::back_inserter(bufferA), "{} {}\n", 
            LABELS[static_cast<size_t>(level)], message);

        SetTextAttribute(target, level);
        
        OutputDebugStringA(bufferA.c_str());
        WriteConsoleA(target, bufferA.data(), static_cast<DWORD>(bufferA.size()), nullptr, nullptr);

        SetConsoleTextAttribute(target, ForeGroundColors::WHITE);
    }

    void Logger::OutputDebug(const LogLevel level, const std::wstring_view message) noexcept
    {
        const bool isError = level <= LogLevel::LOG_LEVEL_ERROR;
        HANDLE target = isError ? errorHandle : outputHandle;

        static constexpr const wchar_t* LABELS_W[6] = 
        {
            L"[FATAL]: ", L"[ERROR]: ", L"[WARN]:  ",
            L"[INFO]:  ", L"[DEBUG]: ", L"[TRACE]: "
        };

        bufferW.clear();

        format_to(std::back_inserter(bufferW), L"{} {}\n", 
            LABELS_W[static_cast<size_t>(level)], message);

        SetTextAttribute(target, level);
        
        OutputDebugStringW(bufferW.c_str());
        WriteConsoleW(target, bufferW.data(), static_cast<DWORD>(bufferW.size()), nullptr, nullptr);

        SetConsoleTextAttribute(target, ForeGroundColors::WHITE);
    }
}