#pragma once

#include "Types.h"
#include "Export.h"
#include <time.h>

namespace Luna
{
    class DLL Timer
    {
    private:
        timespec start, end;
        clockid_t clock_id;
        bool stopped;

        static inline uint64 freq;

        inline int64 GetNanoseconds(const timespec& ts) const noexcept 
        { return (static_cast<int64>(ts.tv_sec) * freq) + ts.tv_nsec; }

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
    { return (Elapsed() >= time); }

    inline bool Timer::Elapsed(const int64 stamp, const float time) noexcept
    { return (Elapsed(stamp) >= time); }

    inline int64 Timer::Stamp() noexcept
    {
        clock_gettime(clock_id, &end);
        return GetNanoseconds(end);
    }
}