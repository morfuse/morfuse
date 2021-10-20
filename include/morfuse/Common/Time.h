#pragma once

#include "../Global.h"
#include "TimeTypes.h"
#include <cstdint>
#include <chrono>

namespace mfuse
{
	class TimeManager
	{
	public:
		TimeManager();

		uinttime_t Frame();
		uinttime_t Frame(float timeScale);

		mfuse_EXPORTS uinttime_t GetDelta() const;
		mfuse_EXPORTS uinttime_t GetScaledTime() const;
		mfuse_EXPORTS uinttime_t GetTime() const;

		void Reset();

	private:
		std::chrono::time_point<std::chrono::steady_clock> startTime;
		std::chrono::time_point<std::chrono::steady_clock> lastClockTime;
		uinttime_t deltaTime;
		uinttime_t scaledTime;
	};
}