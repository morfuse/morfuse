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
	currentTime += deltaTime;

	lastClockTime = clockTime;
	
	return deltaTime;
}

uinttime_t TimeManager::Frame(float timeScale)
{
	using namespace std::chrono;

	time_point<steady_clock> clockTime = steady_clock::now();

	steady_clock::duration deltaClock = clockTime - lastClockTime;
	deltaTime = duration_cast<milliseconds>(deltaClock).count();
	currentTime += deltaTime;

	lastClockTime = clockTime;

	return deltaTime;
}

uinttime_t TimeManager::GetDelta() const
{
	return deltaTime;
}

uinttime_t TimeManager::GetTime() const
{
	return currentTime;
}

void TimeManager::Reset()
{
	startTime = std::chrono::steady_clock::now();
	lastClockTime = startTime;
	currentTime = 0;
}
