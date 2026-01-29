#include <morfuse/Common/MEM/Memory.h>
#include <new>

using namespace mfuse;

void* operator new(std::size_t size)
{
    return IMemoryManager::get().allocate(size);
}

void* operator new[](std::size_t size)
{
    return IMemoryManager::get().allocate(size);
}

void operator delete(void* ptr) noexcept
{
    return IMemoryManager::get().free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    return IMemoryManager::get().free(ptr);
}

#if __cplusplus >= 201402L
void operator delete(void* ptr, std::size_t) noexcept
{
    return IMemoryManager::get().free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept
{
    return IMemoryManager::get().free(ptr);
}
#endif

#if __cplusplus >= 201703L
void* operator new(size_t size, std::align_val_t)
{
    return IMemoryManager::get().allocate(size);
}

void* operator new[](size_t size, std::align_val_t)
{
    return IMemoryManager::get().allocate(size);
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
    return IMemoryManager::get().free(ptr);
}

void operator delete[](void* ptr, std::align_val_t) noexcept
{
    return IMemoryManager::get().free(ptr);
}
#endif
