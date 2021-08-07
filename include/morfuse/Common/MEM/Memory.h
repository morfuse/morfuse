#pragma once

#include "../../Global.h"

#include <cstdint>
#include <cstddef>

namespace mfuse
{
	/**
	 * Memory manager interface, for allocating and freeing memory.
	 */
	class IMemoryManager
	{
	public:
		virtual ~IMemoryManager();

		/**
		 * Allocate memory of the specified size.
		 *
		 * @param size The size to allocate.
		 * @return ptr if allocated.
		 */
		virtual void* allocate(size_t size) = 0;
		/**
		 * Free the specified memory that was previously allocated with allocate.
		 *
		 * @param ptr The memory region to free.
		 */
		virtual void free(void* ptr) = 0;

		/**
		 * Return the memory manager interface.
		 */
		static IMemoryManager& get();

		/**
		 * Set a new memory manager interface.
		 *
		 * @param newValue The new memory manager interface.
		 * @note: This parameter can be NULL. In this case, the default memory manager (std allocation) will be used.
		 */
		static void set(IMemoryManager* newValue);

	private:
		static IMemoryManager* current;
	};

	extern "C"
	{
		mfuse_EXPORTS void* allocateMemory(size_t size);
		mfuse_EXPORTS void freeMemory(void* ptr);
	}
}

/*
#if !defined(__mfuse_MEMORY__) && defined(mfuse_DLL) && mfuse_DLL
#include <new.h>

void* operator new(size_t size);
void* operator new[](size_t size);
void* operator new(size_t size, std::align_val_t);
void* operator new[](size_t size, std::align_val_t);
void operator delete(void* ptr);
void operator delete[](void* ptr);
void operator delete(void* ptr, std::align_val_t);
void operator delete[](void* ptr, std::align_val_t);
#endif
*/
