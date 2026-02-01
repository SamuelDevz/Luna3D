#include "Timer.h"

namespace Luna
{
    Timer::Timer() noexcept : start{}, end{}, stopped{false} 
    {
        freq = 1000000000;
        clock_id = CLOCK_MONOTONIC;
        timespec value;
        
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &value) == 0)
            clock_id = CLOCK_MONOTONIC_RAW;

        Start();
    }

    void Timer::Start() noexcept
    {
        if (stopped)
        {
            int64 elapsedNs = GetNanoseconds(end) - GetNanoseconds(start);
            clock_gettime(clock_id, &start);

            int64 newStartNs = GetNanoseconds(start) - elapsedNs;
            
            start.tv_sec = newStartNs / freq;
            start.tv_nsec = newStartNs % freq;
            stopped = false;
        }
        else
        {
            clock_gettime(clock_id, &start);
        }
    }

    void Timer::Stop() noexcept
    {
        if (!stopped)
        {
            clock_gettime(clock_id, &end);
            stopped = true;
        }
    }

    float Timer::Reset() noexcept
    {
        float elapsed = Elapsed();
        start = end; 
        stopped = false;
        return elapsed;
    }

    float Timer::Elapsed() noexcept
    {
        if (!stopped) 
            clock_gettime(clock_id, &end);
        
        int64 elapsedNs = GetNanoseconds(end) - GetNanoseconds(start);
        return static_cast<float>(elapsedNs / static_cast<double>(freq));
    }

    float Timer::Elapsed(const int64 stamp) noexcept
    {
        if (!stopped) 
            clock_gettime(clock_id, &end);
        
        int64 elapsedNs = GetNanoseconds(end) - stamp;
        return static_cast<float>(elapsedNs / static_cast<double>(freq));
    }
}