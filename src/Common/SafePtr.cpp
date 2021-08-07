#include <morfuse/Common/SafePtr.h>
#include <morfuse/Common/AbstractClass.h>

using namespace mfuse;

SafePtrBase::SafePtrBase()
	: prev(nullptr)
	, next(nullptr)
	, ptr(nullptr)
{
}

SafePtrBase::SafePtrBase(AbstractClass* initial)
{
	ptr = initial;
	if (ptr) AddReference(ptr);
}

SafePtrBase::~SafePtrBase()
{
	Clear();
}

void SafePtrBase::AddReference(AbstractClass* classPtr)
{
	if (!classPtr->SafePtrList)
	{
		classPtr->SafePtrList = this;
		next = prev = this;
	}
	else
	{
		next = classPtr->SafePtrList;
		prev = classPtr->SafePtrList->prev;
		classPtr->SafePtrList->prev->next = this;
		classPtr->SafePtrList->prev = this;
	}
}

void SafePtrBase::RemoveReference(AbstractClass* classPtr)
{
	if (classPtr->SafePtrList == this)
	{
		if (classPtr->SafePtrList->next == this)
		{
			classPtr->SafePtrList = nullptr;
		}
		else
		{
			classPtr->SafePtrList = next;
			prev->next = next;
			next->prev = prev;
			next = this;
			prev = this;
		}
	}
	else
	{
		prev->next = next;
		next->prev = prev;
		next = this;
		prev = this;
	}
}

void SafePtrBase::Clear()
{
	if (ptr)
	{
		RemoveReference(ptr);
		ptr = nullptr;
	}
}

AbstractClass* SafePtrBase::Pointer() const
{
	return ptr;
}

void SafePtrBase::InitSafePtr(AbstractClass* newptr)
{
	if (ptr != newptr)
	{
		if (ptr)
		{
			RemoveReference(ptr);
		}

		ptr = newptr;
		if (ptr == nullptr)
		{
			return;
		}

		AddReference(ptr);
	}
}
