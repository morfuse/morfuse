#pragma once

#include "../Global.h"
#include "EventQueueNode.h"

namespace mfuse
{
	class EventDef;
	class TimeManager;
	class EventQueueNode;

	template<typename, size_t>
	class MEM_BlockAlloc;

	class EventQueue
	{
	public:
		EventQueue();

		mfuse_EXPORTS void ProcessPendingEvents();
		mfuse_EXPORTS void ClearEventList();
		mfuse_EXPORTS bool HasPendingEvents() const;

		mfuse_EXPORTS bool IsEventPending(Listener* l, const EventDef& ev);
		mfuse_EXPORTS void CancelEventsOfType(Listener* l, const EventDef& ev);
		mfuse_EXPORTS void CancelFlaggedEvents(Listener* l, uint32_t flags);
		mfuse_EXPORTS void CancelPendingEvents(Listener* l);

		mfuse_EXPORTS bool ProcessPendingEvents(Listener* l);
		mfuse_EXPORTS void PostEvent(Listener* l, Event* ev, inttime_t delay, uint32_t flags);
		mfuse_EXPORTS bool PostponeAllEvents(Listener* l, inttime_t time);
		mfuse_EXPORTS bool PostponeEvent(Listener* l, Event& ev, inttime_t time);

	private:
		//EventQueueNode Node;
		using List = LinkedList<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>;
		List Node;
	};
}