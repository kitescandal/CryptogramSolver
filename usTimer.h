// Simple microsecond timer class

#ifndef US_TIMER_H
#define US_TIMER_H

#include<windows.h>
#include<cmath>

class usTimer
{
    LARGE_INTEGER startCount;
    LARGE_INTEGER endCount;
    LARGE_INTEGER pauseCount;
    LARGE_INTEGER freq;

public:
    usTimer() {
        QueryPerformanceFrequency(&freq);
    }

    void start()
    {
        endCount.QuadPart = 0;
        pauseCount.QuadPart = 0;
        QueryPerformanceCounter(&startCount);
    }

    void stop() {
        QueryPerformanceCounter(&endCount);
    }

    void restart()
    {
        pauseCount.QuadPart += endCount.QuadPart - startCount.QuadPart;
        endCount.QuadPart = 0;
        QueryPerformanceCounter(&startCount);
    }

    int get()
    {
        LARGE_INTEGER usedCount;

        if(endCount.QuadPart)
            usedCount = endCount;
        else
            QueryPerformanceCounter(&usedCount);

        return (int)((usedCount.QuadPart - startCount.QuadPart + pauseCount.QuadPart) * 1000000 / freq.QuadPart);
    }

    float getSeconds() {
        return (float)get() / 1000000.0f;
    }

    float getSecondsRounded(float rounding) {
        if(rounding < 0.000001f)
            rounding = 0.000001f;
        return std::roundf(getSeconds() / rounding) * rounding;
    }
};

#endif // US_TIMER_H
