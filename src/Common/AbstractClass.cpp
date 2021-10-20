#include <morfuse/Common/AbstractClass.h>
#include <morfuse/Common/SafePtr.h>
#include <morfuse/Common/MEM/Memory.h>

using namespace mfuse;

AbstractClass::AbstractClass()
{
	SafePtrList = nullptr;
}

AbstractClass::~AbstractClass()
{
	while (SafePtrList)
	{
		// nullify safe pointers pointing to this call
		SafePtrList->Clear();
	}
}

void* AbstractClass::operator new(size_t size)
{
	return IMemoryManager::get().allocate(size);
}

void* AbstractClass::operator new[](size_t size)
{
	return IMemoryManager::get().allocate(size);
}

void AbstractClass::operator delete(void* ptr)
{
	return IMemoryManager::get().free(ptr);
}

void AbstractClass::operator delete[](void* ptr)
{
	return IMemoryManager::get().free(ptr);
}

void* AbstractClass::operator new(size_t, void* placement)
{
	return placement;
}

void AbstractClass::operator delete(void*, void*)
{
}
