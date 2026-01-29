#include <morfuse/Common/Time.h>

using namespace mfuse;

TimeManager::TimeManager()
{
    Reset();
}

uinttime_t TimeManager::Frame()
{
    using namespace std::chrono;

    time_point<steady_clock> clockTime = steady_clock::now();

    steady_clock::duration deltaClock = clockTime - lastClockTime;
    deltaTime = duration_cast<milliseconds>(deltaClock).count();
    scaledTime += deltaTime;

    lastClockTime = clockTime;
    
    return deltaTime;
}

uinttime_t TimeManager::Frame(float timeScale)
{
    using namespace std::chrono;

    time_point<steady_clock> clockTime = steady_clock::now();

    steady_clock::duration deltaClock = clockTime - lastClockTime;
    deltaTime = duration_cast<milliseconds>(deltaClock).count();
    scaledTime += uinttime_t(deltaTime * timeScale);

    lastClockTime = clockTime;

    return deltaTime;
}

uinttime_t TimeManager::GetDelta() const
{
    return deltaTime;
}

uinttime_t TimeManager::GetScaledTime() const
{
    return scaledTime;
}

uinttime_t TimeManager::GetTime() const
{
    using namespace std::chrono;

    time_point<steady_clock> clockTime = steady_clock::now();
    return duration_cast<milliseconds>(clockTime - startTime).count();
}

void TimeManager::Reset()
{
    startTime = std::chrono::steady_clock::now();
    lastClockTime = startTime;
    scaledTime = 0;
}
