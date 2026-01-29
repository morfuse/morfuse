#pragma once

#include "../Common/str.h"
#include "../Common/SafePtr.h"
#include "../Common/Time.h"

namespace mfuse
{
    class Event;
    class EventQueueNode;
    class Listener;

    class EventQueueNode
    {
        friend class EventQueue;

    public:
        EventQueueNode();
        EventQueueNode(Listener* initialListener);
        ~EventQueueNode();

        void* operator new(size_t size);
        void operator delete(void* ptr);

        Listener* GetSourceObject();
        void SetSourceObject(Listener* obj);

    private:
        Event* event;
        SafePtr<Listener> m_sourceobject;
        inttime_t time;
        uint32_t flags;

        EventQueueNode* prev;
        EventQueueNode* next;
    };
}