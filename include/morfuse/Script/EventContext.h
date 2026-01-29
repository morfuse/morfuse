#pragma once

#include "../Common/ThreadSingleton.h"
#include "../Common/Time.h"
#include "../Common/MEM/BlockAlloc.h"
#include "Event.h"
#include "EventQueue.h"
#include "NamespaceManager.h"

namespace mfuse
{
    template<size_t BlockSize>
    class EventAllocator
    {
    public:
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
        mfuse_EXPORTS NamespaceManager& GetNamespaceManager();
        mfuse_EXPORTS const NamespaceManager& GetNamespaceManager() const;

        DefaultEventAllocator& GetAllocator();

    protected:
        TimeManager& GetTimeManagerInternal();

    private:
        DefaultEventAllocator allocator;
        TimeManager timeManager;
        EventQueue eventQueue;
        NamespaceManager namespaceManager;
    };

    //mfuse_TEMPLATE template class mfuse_EXPORTS ThreadSingleton<EventContext>;
}