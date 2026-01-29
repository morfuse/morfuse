#include <morfuse/Common/MEM/PreAllocator.h>
#include <morfuse/Common/MEM/Memory.h>

#include <cassert>

using namespace mfuse;

MEM::PreAllocator::PreAllocator()
    : allocatedBlock(nullptr)
{
}

MEM::PreAllocator::PreAllocator(MEM::PreAllocator&& other)
    : allocatedBlock(other.allocatedBlock)
    , endBlock(other.endBlock)
    , current(other.current)
{
    other.allocatedBlock = nullptr;
    other.endBlock = nullptr;
    other.current = nullptr;
}

MEM::PreAllocator& MEM::PreAllocator::operator=(MEM::PreAllocator&& other)
{
    allocatedBlock = other.allocatedBlock;
    endBlock = other.endBlock;
    current = other.current;
    other.allocatedBlock = nullptr;
    other.endBlock = nullptr;
    other.current = nullptr;
    return *this;
}

MEM::PreAllocator::~PreAllocator()
{
    Release();
}

void* MEM::PreAllocator::Alloc(size_t size)
{
    assert(current + size <= endBlock);

    void* const newPtr = current;
    current += size;
    return newPtr;
}

void MEM::PreAllocator::Free(void*)
{
}

void MEM::PreAllocator::PreAllocate(size_t size)
{
    allocatedBlock = (unsigned char*)IMemoryManager::get().allocate(size);
    current = allocatedBlock;
    endBlock = allocatedBlock + size;
}

void MEM::PreAllocator::Release()
{
    if (allocatedBlock)
    {
        IMemoryManager::get().free(allocatedBlock);
        allocatedBlock = nullptr;
    }
}

size_t MEM::PreAllocator::Size() const
{
    return endBlock - allocatedBlock;
}

MEM::ChildPreAllocator::ChildPreAllocator()
    : allocator(nullptr)
{
}

void MEM::ChildPreAllocator::SetAllocator(MEM::PreAllocator& newAllocator)
{
    allocator = &newAllocator;
}

MEM::PreAllocator* MEM::ChildPreAllocator::GetAllocator() const
{
    return allocator;
}

void* MEM::ChildPreAllocator::Alloc(size_t size)
{
    return allocator->Alloc(size);
}

void MEM::ChildPreAllocator::Free(void* ptr)
{
    allocator->Free(ptr);
}

void* MEM::ChildPreAllocator_set::AllocTable(size_t size)
{
    return GetAllocator()->Alloc(size);
}

void MEM::ChildPreAllocator_set::FreeTable(void* ptr)
{
    GetAllocator()->Free(ptr);
}

void* operator new(size_t count, MEM::PreAllocator& allocator)
{
    return allocator.Alloc(count);
}

void* operator new[](size_t count, MEM::PreAllocator& allocator)
{
    return allocator.Alloc(count);
}

void operator delete(void* ptr, MEM::PreAllocator& allocator)
{
    return allocator.Free(ptr);
}

void operator delete[](void* ptr, MEM::PreAllocator& allocator)
{
    return allocator.Free(ptr);
}

#if __cplusplus >= 201703L
void* operator new(size_t count, std::align_val_t, MEM::PreAllocator& allocator)
{
    return allocator.Alloc(count);
}

void* operator new[](size_t count, std::align_val_t, MEM::PreAllocator& allocator)
{
    return allocator.Alloc(count);
}

void operator delete(void* ptr, std::align_val_t, MEM::PreAllocator& allocator)
{
    return allocator.Free(ptr);
}

void operator delete[](void* ptr, std::align_val_t, MEM::PreAllocator& allocator)
{
    return allocator.Free(ptr);
}
#endif
