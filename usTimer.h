// Simple microsecond timer class

#ifndef US_TIMER_H
#define US_TIMER_H

#include<chrono>
#include<cmath>

class usTimer
{
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
    typedef std::chrono::duration<double, std::micro> TimeDuration;

    TimePoint startTime;
    TimeDuration totalTime;
    bool paused;

public:
    usTimer() : paused(true), totalTime(std::chrono::microseconds(0)) {}

    void start()
    {
        startTime = std::chrono::steady_clock::now();
        totalTime = std::chrono::microseconds(0);
        paused = false;
    }

    void stop()
    {
        TimePoint endTime = std::chrono::steady_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        paused = true;
    }

    void restart()
    {
        startTime = std::chrono::steady_clock::now();
        paused = false;
    }

    int get()
    {
        TimeDuration duration;

        if(paused)
            duration = totalTime;
        else {
            TimePoint thisTime = std::chrono::steady_clock::now();
            duration = totalTime + std::chrono::duration_cast<std::chrono::microseconds>(thisTime - startTime);
        }
        return (int)duration.count();
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
