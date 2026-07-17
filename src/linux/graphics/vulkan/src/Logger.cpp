#include "Logger.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <unistd.h>
#include <format>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using std::format;
using std::system;
using std::ostream;

namespace Luna
{
    static constexpr const char* LOG_LEVEL_PREFIX[] = 
    {
        "[FATAL]: ",
        "[ERROR]: ",
        "[WARN]: ",
        "[INFO]: ",
        "[DEBUG]: ",
        "[TRACE]: "
    };
    
    Logger::Logger() noexcept
    {
        outputHandle = static_cast<void*>(&std::cout);
        errorHandle  = static_cast<void*>(&std::cerr);
        AllocConsole();
    }

    Logger::~Logger() noexcept
    {
        if (pipeStream != nullptr)
            delete static_cast<std::ofstream*>(pipeStream);
    }
    
    void Logger::AllocConsole() noexcept
    {
        string fifoPath = format("/tmp/console{}", std::to_string(getpid()));

        if (mkfifo(fifoPath.c_str(), 0666) == -1)
        {
            if (errno != EEXIST) 
            {
                std::perror("Falha ao criar o Named Pipe para o console");
                return;
            }
        }
        
        struct TerminalInfo 
        {
            const char* name;
            const char* arg;
        };

        constexpr TerminalInfo terminals[] = 
        {
            {"alacritty", "-e"},           // Alacritty
            {"kitty", "--"},               // Kitty
            {"westerm", "start --"},       // WesTerm
            {"gnome-terminal", "--"},      // GNOME
            {"konsole", "-e"},             // KDE
            {"xfce4-terminal", "-x"},      // XFCE
            {"lxqt-terminal", "-e"},       // LXQt
            {"qterminal", "-e"},           // LXQt / Outros
            {"lxterminal", "-e"},          // LXDE
            {"x-terminal-emulator", "-e"}, // Padrão no Debian/Ubuntu
            {"terminator", "-x"},          // Terminator
            {"xterm", "-e"}                // Fallback universal
        };

        bool terminalLaunched = false;
        for (const auto& term : terminals)
        {
            string checkCmd = format("command -v {} >/dev/null 2>&1", term.name);
            if (system(checkCmd.c_str()) == 0)
            {
                string launchCmd = format("{} {} bash -c \"cat '{}'; rm -f '{}'\" &", 
                    term.name, term.arg, fifoPath, fifoPath);
                system(launchCmd.c_str());
                terminalLaunched = true;
                break;
            }
        }

        if (!terminalLaunched)
            std::cerr << "[Logger] Aviso: Nenhum emulador de terminal suportado foi encontrado no sistema." << std::endl;
        
        auto outPipe = new std::ofstream(fifoPath);
        if (outPipe->is_open())
        {
            pipeStream = static_cast<void*>(outPipe);
            outputHandle = pipeStream;
            errorHandle  = pipeStream;
        }
        else
        {
            delete outPipe;
            std::perror("Falha ao conectar ao terminal de logs");
        }
    }

    void Logger::ApplyLevelColor(void* handle, const LogLevel level) noexcept
    {
        if (!handle) 
            return;
        auto& stream = *static_cast<std::ostream*>(handle);

        constexpr std::string_view LOG_LEVEL_COLORS[] = 
        {
            "\x1b[41m",   // FATAL
            "\x1b[1;31m", // ERROR
            "\x1b[1;33m", // WARN
            "\x1b[32m",   // INFO
            "\x1b[34m",   // DEBUG
            "\x1b[90m"    // TRACE
        };

        stream << LOG_LEVEL_COLORS[level];
    }

    void Logger::ResetColor(void* handle) noexcept
    {
        if (!handle) 
            return;
        auto& stream = *static_cast<std::ostream*>(handle);
        stream << "\x1b[0m";
    }

    void Logger::ResetColors() noexcept
    {
        ResetColor(outputHandle);
        ResetColor(errorHandle);
    }

    void Logger::WriteToConsole(void* handle, const LogLevel level, const std::string_view message) noexcept
    {
        if (!handle) 
            return;
        auto& stream = *static_cast<std::ostream*>(handle);

        ApplyLevelColor(handle, level);
        stream << message;
        stream.flush(); 
    }

    void Logger::OutputDebug(const LogLevel level, const std::string_view message) noexcept
    {
        const bool isError = (level < LOG_LEVEL_WARN);
        const string output = format("{}{}", LOG_LEVEL_PREFIX[level], message);

        WriteToConsole(isError ? errorHandle : outputHandle, level, output);
        ResetColors();
    }

    void Logger::OutputDebug(const LogLevel level, const std::wstring_view message) noexcept
    {
        const bool isError = (level < LOG_LEVEL_WARN);
        
        string utf8Message(message.begin(), message.end());
        const string output = format("{}{}", LOG_LEVEL_PREFIX[level], utf8Message);

        WriteToConsole(isError ? errorHandle : outputHandle, level, output);
        ResetColors();
    }
}