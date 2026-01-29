#include <morfuse/Common/MEM/DefaultAlloc.h>
#include <morfuse/Common/MEM/Memory.h>

using namespace mfuse;

void* MEM::DefaultAlloc::Alloc(size_t size)
{
    return IMemoryManager::get().allocate(size);
}

void MEM::DefaultAlloc::Free(void* ptr)
{
    return IMemoryManager::get().free(ptr);
}

void* MEM::DefaultAlloc_set::AllocTable(size_t size)
{
    return IMemoryManager::get().allocate(size);
}

void MEM::DefaultAlloc_set::FreeTable(void* ptr)
{
    return IMemoryManager::get().free(ptr);
}
