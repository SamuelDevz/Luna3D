#include "Timer.h"

namespace Luna
{
    Timer::Timer() noexcept : start{}, end{}, stoped{false} 
    {
        if (!freq.QuadPart)
            QueryPerformanceFrequency(&freq);
    }

    void Timer::Start() noexcept
    {
        if (stoped)
        {
            // resumes time counting
            //
            //      <--- elapsed ---->
            // ----|------------------|------------> time
            //    start               end     
            //

            int64 elapsed = end.QuadPart - start.QuadPart;
            QueryPerformanceCounter(&start); 
            start.QuadPart -= elapsed;
            stoped = false;
        }
        else
        {
            QueryPerformanceCounter(&start);
        }
    }

    void Timer::Stop() noexcept
    {
        if (!stoped)
        {
            QueryPerformanceCounter(&end);
            stoped = true;
        }
    }

    float Timer::Reset() noexcept
    {
        int64 elapsed{};

        if (stoped)
        {
            elapsed = end.QuadPart - start.QuadPart;
            QueryPerformanceCounter(&start); 
            stoped = false;
        }
        else
        {
            QueryPerformanceCounter(&end);
            elapsed = end.QuadPart - start.QuadPart;
            start = end;
        }

        return static_cast<float>(elapsed / double(freq.QuadPart));    
    }

    float Timer::Elapsed() noexcept
    {
        int64 elapsed{};

        if (stoped)
        {
            elapsed = end.QuadPart - start.QuadPart;
        }
        else
        {
            QueryPerformanceCounter(&end);
            elapsed = end.QuadPart - start.QuadPart;
        }

        return static_cast<float>(elapsed / double(freq.QuadPart));
    }

    float Timer::Elapsed(const int64 stamp) noexcept
    {
        int64 elapsed{};

        if (stoped)
        {
            elapsed = end.QuadPart - stamp;
        }
        else
        {
            QueryPerformanceCounter(&end);
            elapsed = end.QuadPart - stamp;
        }

        return static_cast<float>(elapsed / double(freq.QuadPart));
    }
}