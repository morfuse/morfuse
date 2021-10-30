#pragma once

#include <cstdint>
#include <cstdlib>

namespace mfuse
{
namespace MEM
{
	class tempBlock_t
	{
	public:
		void* GetData();
		void* GetData(size_t pos);

	public:
		tempBlock_t* prev;
	};

	class TempAlloc
	{
	public:
		TempAlloc(size_t blockSize = 65536);
		TempAlloc(const TempAlloc& other) = delete;
		TempAlloc& operator=(const TempAlloc& other) = delete;
		TempAlloc(TempAlloc&& other);
		TempAlloc& operator=(TempAlloc&& other);
		~TempAlloc();

		void* Alloc(size_t len);
		void* Alloc(size_t len, size_t alignment);
		void Free(void* ptr);
		void FreeAll();

	private:
		void* CreateBlock(size_t len);

	private:
		tempBlock_t* m_CurrentMemoryBlock;
		size_t m_CurrentMemoryPos;
		size_t m_BlockSize;
		size_t m_LastPos;
	};
}
};
