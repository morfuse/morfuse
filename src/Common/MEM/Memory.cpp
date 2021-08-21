#define __mfuse_MEMORY__ 1
#include <morfuse/Common/MEM/Memory.h>

#include <cstdlib>
#include <cassert>

/*
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
*/

using namespace mfuse;

namespace mfuse
{
	class DefaultMemoryManager : public IMemoryManager
	{
	public:
		~DefaultMemoryManager()
		{}

		void* allocate(size_t size) override
		{
			return malloc(size);
		}

		void free(void* ptr) noexcept override
		{
			::free(ptr);
		}
	};


	class DebugMemoryManager : public IMemoryManager
	{
	public:
		DebugMemoryManager()
			: allocated(0)
		{
			//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
			//_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		}

		~DebugMemoryManager()
		{
			//_CrtDumpMemoryLeaks();
			assert(!allocated);
		}

		void* allocate(size_t size) override
		{
			size_t* const pmem = (size_t*)malloc(size + sizeof(size_t));
			allocated += size;
			*pmem = size;
			return pmem + 1;
		}

		void free(void* ptr) noexcept override
		{
			if (ptr)
			{
				size_t* const pmem = (size_t*)ptr - 1;
				allocated -= *pmem;
				::free(pmem);
			}
		}

	private:
		size_t allocated;
	};

	class NoDelete
	{
	public:
		void operator()(DefaultMemoryManager* manager)
		{
			// The static memory manager will be destroyed during program uninitialization
		}
	};

#if !NDEBUG
	static DebugMemoryManager defaultMemoryManager;
#else
	static DefaultMemoryManager defaultMemoryManager;
#endif
}

IMemoryManager* IMemoryManager::current = &defaultMemoryManager;

IMemoryManager::~IMemoryManager()
{
	if (current == this) current = &defaultMemoryManager;
}

mfuse::IMemoryManager& IMemoryManager::get()
{
	return *current;
}

void IMemoryManager::set(IMemoryManager* newValue)
{
	if (newValue) current = newValue;
	else current = &defaultMemoryManager;
}

void* mfuse::allocateMemory(size_t size)
{
	return IMemoryManager::get().allocate(size);
}

void mfuse::freeMemory(void* ptr)
{
	return IMemoryManager::get().free(ptr);
}
