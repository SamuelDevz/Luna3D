#pragma once

#include "Types.h"
#include "Export.h"
#include "WinInclude.h"
#include <windows.h>

namespace Luna
{
    class DLL Timer
    {
    private:
        LARGE_INTEGER start, end;
        static inline LARGE_INTEGER freq;
        bool stoped;
        
    public:
        explicit Timer() noexcept;

        void Start() noexcept;
        void Stop() noexcept;
        float Reset() noexcept;
        float Elapsed() noexcept;
        bool Elapsed(const float time) noexcept;

        int64 Stamp() noexcept;
        float Elapsed(const int64 stamp) noexcept;
        bool Elapsed(const int64 stamp, const float time) noexcept;
    }; 

    inline bool Timer::Elapsed(const float time) noexcept
    { return (Elapsed() >= time ? true : false); }

    inline bool Timer::Elapsed(const int64 stamp, const float time) noexcept
    { return (Elapsed(stamp) >= time ? true : false); }

    inline int64 Timer::Stamp() noexcept
    {
        QueryPerformanceCounter(&end);
        return end.QuadPart;
    }
}