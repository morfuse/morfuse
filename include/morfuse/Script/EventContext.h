#pragma once

#include "../Common/ThreadSingleton.h"
#include "../Common/Time.h"
#include "../Common/MEM/BlockAlloc.h"
#include "Event.h"
#include "EventQueue.h"

namespace mfuse
{
	template<size_t BlockSize>
	class EventAllocator
	{
	public:
		template<typename T>
		MEM::BlockAlloc<T, BlockSize>& GetBlock();

		template<>
		MEM::BlockAlloc<Event, BlockSize>& GetBlock<Event>()
		{
			return Event_allocator;
		}

		template<>
		MEM::BlockAlloc<EventQueueNode, BlockSize>& GetBlock<EventQueueNode>()
		{
			return EventQueueNode_allocator;
		}

	private:
		MEM::BlockAlloc<Event, BlockSize> Event_allocator;
		MEM::BlockAlloc<EventQueueNode, BlockSize> EventQueueNode_allocator;
	};
	using DefaultEventAllocator = EventAllocator<MEM::DefaultBlock>;

	class EventContext : public ThreadSingleton<EventContext>
	{
	public:
		mfuse_EXPORTS EventContext();
		mfuse_EXPORTS ~EventContext();

		mfuse_EXPORTS void ProcessEvents(float timeScale = 1.f);
		mfuse_EXPORTS const TimeManager& GetTimeManager() const;
		mfuse_EXPORTS EventQueue& GetEventQueue();
		mfuse_EXPORTS const EventQueue& GetEventQueue() const;

		DefaultEventAllocator& GetAllocator();

	protected:
		TimeManager& GetTimeManagerInternal();

	private:
		DefaultEventAllocator allocator;
		TimeManager timeManager;
		EventQueue eventQueue;
	};
}