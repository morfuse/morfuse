#include <morfuse/Script/EventContext.h>

using namespace mfuse;

EventContext::EventContext()
	: ThreadSingleton(this)
{
}

EventContext::~EventContext()
{
}

void EventContext::ProcessEvents(float timeScale)
{
	if (timeScale) {
		timeManager.Frame();
	} else {
		timeManager.Frame(timeScale);
	}
	eventQueue.ProcessPendingEvents();
}

const TimeManager& EventContext::GetTimeManager() const
{
	return timeManager;
}

TimeManager& EventContext::GetTimeManagerInternal()
{
	return timeManager;
}

EventQueue& EventContext::GetEventQueue()
{
	return eventQueue;
}

const mfuse::EventQueue& EventContext::GetEventQueue() const
{
	return eventQueue;
}

DefaultEventAllocator& EventContext::GetAllocator()
{
	return allocator;
}
