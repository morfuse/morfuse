#include <morfuse/Script/EventQueueNode.h>
#include <morfuse/Script/EventContext.h>
#include <morfuse/Script/Listener.h>

using namespace mfuse;

EventQueueNode::EventQueueNode()
	: prev(nullptr)
	, next(nullptr)
{
	//prev = this; next = this;
}

EventQueueNode::EventQueueNode(Listener* initialListener)
	: prev(nullptr)
	, next(nullptr)
	, m_sourceobject(initialListener)
{

}

EventQueueNode::~EventQueueNode()
{

}

void* EventQueueNode::operator new(size_t size)
{
	return EventContext::Get().GetAllocator().GetBlock<EventQueueNode>().Alloc();
}

void EventQueueNode::operator delete(void* ptr)
{
	return EventContext::Get().GetAllocator().GetBlock<EventQueueNode>().Free(ptr);
}

Listener* EventQueueNode::GetSourceObject()
{
	return m_sourceobject;
}

void EventQueueNode::SetSourceObject(Listener* obj)
{
	m_sourceobject = obj;
}
