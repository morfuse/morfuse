#include <morfuse/Script/Component.h>

using namespace mfuse;

Component::Component(Listener& parentRef)
	: parent(parentRef)
{
}

Component::~Component()
{
}

Listener& Component::Parent() const
{
	return parent;
}

void Component::Archive(Archiver& arc)
{
}
