#include "Logger.h"
#include <iostream>
#include <format>
using std::cout;
using std::cerr;
using std::format;

namespace Luna
{
    static constexpr const char* ANSI_RESET = "\033[0m";
    
    static const char* GetLevelColor(const LogLevel level)
    {
        switch (level)
        {
            case LOG_LEVEL_FATAL: return "\033[41m";
            case LOG_LEVEL_ERROR: return "\033[31m";
            case LOG_LEVEL_WARN:  return "\033[33m";
            case LOG_LEVEL_INFO:  return "\033[32m";
            case LOG_LEVEL_DEBUG: return "\033[34m";
            case LOG_LEVEL_TRACE: return "\033[90m";
            default:              return ANSI_RESET;
        }
    }
    
    void Logger::WriteToConsole(const LogLevel level, const string_view message) const noexcept
    {
        cout << format("{}{}{}", GetLevelColor(level), message, ANSI_RESET);
    }
    
    void Logger::WriteToConsoleError(const LogLevel level, const string_view message) const noexcept
    {
        cerr << format("{}{}{}", GetLevelColor(level), message, ANSI_RESET);
    }

    void Logger::OutputDebug(const LogLevel level, const string_view message) const noexcept
    {
        static constexpr const char* LOG_LEVEL_STRINGS[6]
        {
            "[FATAL]:", 
            "[ERROR]:", 
            "[WARN]:", 
            "[INFO]:", 
            "[DEBUG]:", 
            "[TRACE]:"
        };

        const string outputMessage = format("{} {}", LOG_LEVEL_STRINGS[level], message);

        if (level < LOG_LEVEL_WARN)
            WriteToConsoleError(level, outputMessage);
        else
            WriteToConsole(level, outputMessage);
    }
}