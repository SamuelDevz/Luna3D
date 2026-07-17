#include "Timer.h"

namespace Luna
{
    Timer::Timer() noexcept : start{}, end{}, stopped{false} 
    {
        clock = CLOCK_MONOTONIC;
        freq = 1000000000;

        timespec value;
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &value) == 0)
            clock = CLOCK_MONOTONIC_RAW;
    }

    void Timer::Start() noexcept
    {
        if (stopped)
        {
            // resumes time counting
            //
            //      <--- elapsed ---->
            // ----|------------------|------------> time
            //    start               end     
            //

            int64 elapsed = (end.tv_sec * freq + end.tv_nsec) - (start.tv_sec * freq + start.tv_nsec);
            clock_gettime(clock, &start);

            int64 startTimeNs = start.tv_sec * freq + start.tv_nsec;
            startTimeNs -= elapsed;
            
            start.tv_sec = startTimeNs / freq;
            start.tv_nsec = startTimeNs % freq;
            stopped = false;
        }
        else
        {
            clock_gettime(clock, &start);
        }
    }

    void Timer::Stop() noexcept
    {
        if (!stopped)
        {
            clock_gettime(clock, &end);
            stopped = true;
        }
    }

    float Timer::Reset() noexcept
    {
        int64 elapsed{};

        if (stopped)
        {
            elapsed = (end.tv_sec * freq + end.tv_nsec) - (start.tv_sec * freq + start.tv_nsec);
            clock_gettime(clock, &start); 
            stopped = false;
        }
        else
        {
            clock_gettime(clock, &end);
            elapsed = (end.tv_sec * freq + end.tv_nsec) - (start.tv_sec * freq + start.tv_nsec);
            start = end;
        }

        return static_cast<float>(elapsed / double(freq));
    }

    float Timer::Elapsed() noexcept
    {
        int64 elapsed{};

        if (stopped)
        {
            elapsed = (end.tv_sec * freq + end.tv_nsec) - (start.tv_sec * freq + start.tv_nsec);
        }
        else
        {
            clock_gettime(clock, &end);
            elapsed = (end.tv_sec * freq + end.tv_nsec) - (start.tv_sec * freq + start.tv_nsec);
        }

        return static_cast<float>(elapsed / double(freq));
    }

    float Timer::Elapsed(const int64 stamp) noexcept
    {
        int64 elapsed{};

        if (stopped)
        {
            elapsed = (end.tv_sec * freq + end.tv_nsec) - stamp;
        }
        else
        {
            clock_gettime(clock, &end);
            elapsed = (end.tv_sec * freq + end.tv_nsec) - stamp;
        }

        return static_cast<float>(elapsed / double(freq));
    }
}