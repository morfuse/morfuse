#include <morfuse/Common/MEM/BlockAlloc.h>
#include <morfuse/Common/MEM/Memory.h>

using namespace mfuse;

void* MEM::Alloc(size_t sz)
{
	return allocateMemory(sz);
}

void MEM::Free(void* ptr)
{
	return freeMemory(ptr);
}
