#include <morfuse/Common/SafePtr.h>
#include <morfuse/Common/AbstractClass.h>

using namespace mfuse;

SafePtrBase::SafePtrBase() noexcept
    : prev(nullptr)
    , next(nullptr)
    , ptr(nullptr)
{
}

SafePtrBase::SafePtrBase(AbstractClass* initial) noexcept
{
    ptr = initial;
    if (ptr) AddReference(ptr);
}

SafePtrBase::~SafePtrBase()
{
    Clear();
}

void SafePtrBase::AddReference(AbstractClass* classPtr) noexcept
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

void SafePtrBase::RemoveReference(AbstractClass* classPtr) noexcept
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

void SafePtrBase::Clear() noexcept
{
    if (ptr)
    {
        RemoveReference(ptr);
        ptr = nullptr;
    }
}

AbstractClass* SafePtrBase::Pointer() const noexcept
{
    return ptr;
}

void SafePtrBase::InitSafePtr(AbstractClass* newptr) noexcept
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

bool SafePtrBase::IsLastReference() const noexcept
{
    return next == this && prev == this;
}

bool SafePtrBase::Valid() const noexcept
{
    return Pointer() != nullptr;
}
