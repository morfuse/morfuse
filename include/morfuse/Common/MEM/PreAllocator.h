#pragma once

#include <new>
#include <cstdint>

namespace mfuse
{
namespace MEM
{
	class PreAllocator
	{
	public:
		PreAllocator();
		PreAllocator(PreAllocator&& other);
		PreAllocator(const PreAllocator& other) = delete;
		PreAllocator& operator=(PreAllocator&& other);
		PreAllocator& operator=(const PreAllocator& other) = delete;
		~PreAllocator();

		void* Alloc(size_t size);
		void Free(void* ptr);

		void PreAllocate(size_t size);
		void Release();

		size_t Size() const;

	private:
		unsigned char* allocatedBlock;
		unsigned char* endBlock;
		unsigned char* current;
	};

	class ChildPreAllocator
	{
	public:
		ChildPreAllocator();

		void SetAllocator(PreAllocator& newAllocator);
		PreAllocator* GetAllocator() const;

		void* Alloc(size_t size);
		void Free(void* ptr);

	private:
		PreAllocator* allocator;
	};

	class ChildPreAllocator_set : public ChildPreAllocator
	{
	public:
		void* AllocTable(size_t size);
		void FreeTable(void* ptr);
	};
}
}

void* operator new(size_t count, mfuse::MEM::PreAllocator& allocator);
void* operator new(size_t count, std::align_val_t, mfuse::MEM::PreAllocator& allocator);
void* operator new[](size_t count, mfuse::MEM::PreAllocator& allocator);
void* operator new[](size_t count, std::align_val_t, mfuse::MEM::PreAllocator& allocator);
void operator delete(void* ptr, mfuse::MEM::PreAllocator& allocator);
void operator delete(void* ptr, std::align_val_t, mfuse::MEM::PreAllocator& allocator);
void operator delete[](void* ptr, mfuse::MEM::PreAllocator& allocator);
void operator delete[](void* ptr, std::align_val_t, mfuse::MEM::PreAllocator& allocator);
