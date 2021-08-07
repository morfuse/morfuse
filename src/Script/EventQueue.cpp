#include <morfuse/Script/EventQueue.h>
#include <morfuse/Script/EventQueueNode.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/Context.h>

using namespace mfuse;

//BlockAlloc<EventQueueNode> EventQueueNode::allocator;

EventQueue::EventQueue()
{
}

void EventQueue::ProcessPendingEvents()
{
	const uinttime_t t = EventContext::Get().GetTimeManager().GetTime();

	//while (!LL::Empty<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(&Node))
	size_t numProcessed = 0;
	while(!Node.IsEmpty())
	{
		EventQueueNode* const node = Node.Root();

		assert(node);

		Listener* const obj = node->GetSourceObject();

		assert(obj);

		if (node->time > t)
		{
			break;
		}

		// the event is removed from its list
		//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(node);
		Node.Remove(node);

		// ProcessEvent will dispose of this event when it is done
		obj->ProcessEvent(node->event);

		delete node;
		++numProcessed;
	}
}

bool EventQueue::ProcessPendingEvents(Listener* l)
{
	bool processedEvents;

	processedEvents = false;

	const uinttime_t t = EventContext::Get().GetTimeManager().GetTime();

	List::iterator event = Node.CreateIterator();
	while (event)
	{
		Listener* obj = event->GetSourceObject();

		if (event->time > t)
		{
			break;
		}

		if (obj != l)
		{
			// traverse normally
			event = event.Next();
		}
		else
		{
			// the event is removed from its list and temporarily added to the active list
			//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(event);
			Node.Remove(event.Node());

			// ProcessEvent will dispose of this event when it is done
			obj->ProcessEvent(event->event);

			// free up the node
			delete event.Node();

			// start over, since can't guarantee that we didn't process any previous or following events
			event = Node.CreateIterator();

			processedEvents = true;
		}
	}

	return processedEvents;
}

void EventQueue::ClearEventList()
{
	List::iterator next;
	for (List::iterator node = Node.CreateIterator(); node; node = next)
	{
		next = node.Next();

		delete node->event;
		delete node.Node();
	}

	//LL::Reset<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(&Node);
	Node.Reset();
}

void EventQueue::CancelEventsOfType(Listener* l, const EventDef& ev)
{
	eventNum_t eventnum = ev.GetEventNum();

	List::iterator next;
	for (List::iterator node = Node.CreateIterator(); node; node = next)
	{
		next = node.Next();
		if ((node->GetSourceObject() == l) && (node->event->Num() == eventnum))
		{
			//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(node);
			Node.Remove(node.Node());
			delete node.Node();
		}
	}
}

void EventQueue::CancelFlaggedEvents(Listener* l, uint32_t flags)
{
	List::iterator next;
	for (List::iterator node = Node.CreateIterator(); node; node = next)
	{
		next = node.Next();
		if ((node->GetSourceObject() == l) && (node->flags & flags))
		{
			//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(node);
			Node.Remove(node.Node());
			delete node.Node();
		}
	}
}

void EventQueue::CancelPendingEvents(Listener* l)
{
	List::iterator next;
	for(List::iterator node = Node.CreateIterator(); node; node = next)
	{
		next = node.Next();
		if (node->GetSourceObject() == l)
		{
			//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(node);
			Node.Remove(node.Node());
			delete node.Node();
		}
	}
}

bool EventQueue::IsEventPending(Listener* l, const EventDef& ev)
{
	List::iterator event = Node.CreateIterator();

	const uintptr_t eventnum = ev.GetEventNum();

	for (List::iterator event = Node.CreateIterator(); event; event = event.Next())
	{
		if ((event->GetSourceObject() == l) && (event->event->Num()))
		{
			return true;
		}
	}

	return false;
}

void EventQueue::PostEvent(Listener* l, Event* ev, uint64_t delay, uint32_t flags)
{
	const uinttime_t time = EventContext::Get().GetTimeManager().GetTime() + delay; // + 0.0005f;
	EventQueueNode* node = new EventQueueNode(l);
	node->time = time;
	node->event = ev;
	node->flags = flags;
	
	if(Node.Root() && time < Node.Tail()->time)
	{
		if (time >= Node.Root()->time)
		{
			List::iterator i;
			// find a node with the highest time before the new one
			for (i = Node.CreateIterator(); i; i = i.Next())
			{
				if (i->time > time) {
					break;
				}
			}
			
			assert(i);
			// insert between nodes
			Node.Insert(i, node);
		}
		else
		{
			// the node has the lowest time, as a consequence add it first
			Node.AddFirst(node);
		}
	}
	else
	{
		// add first root node
		// or append at tail if it has the highest time
		Node.Add(node);
	}

	//LL::Add<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(i, node);
}

bool EventQueue::PostponeAllEvents(Listener* l, uint64_t time)
{
	for (List::iterator event = Node.CreateIterator(); event; event = event.Next())
	{
		if (event->GetSourceObject() == l)
		{
			event->time += time; // + 0.0005f;

			List::iterator node;
			for(node = event.Next(); node; node = node.Next())
			{
				if (event->time < node->time) {
					break;
				}
			}

			//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(event);
			//LL::Add<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(node, event);
			Node.Remove(event);
			Node.Insert(node, event);

			return true;
		}
	}

	return false;
}

bool EventQueue::PostponeEvent(Listener* l, Event& ev, uint64_t time)
{
	const eventNum_t eventnum = ev.Num();

	for (List::iterator event = Node.CreateIterator(); event; event = event.Next())
	{
		if ((event->GetSourceObject() == l) && (event->event->Num() == eventnum))
		{
			event->time += time; // + 0.0005f;

			List::iterator node;
			for (node = event.Next(); node; node = node.Next())
			{
				if (event->time < node->time) {
					break;
				}
			}

			//LL::Remove<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(event);
			//LL::Add<EventQueueNode*, &EventQueueNode::next, &EventQueueNode::prev>(node, event);
			Node.Remove(event);
			Node.Insert(node, event);

			return true;
		}
	}

	return false;
}
