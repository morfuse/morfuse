#pragma once

#include "../Linklist.h"
#include "../../Global.h"
#include <shared_mutex>
#include <cstdint>
#include <type_traits>

namespace mfuse
{
namespace MEM
{
	mfuse_EXPORTS void* Alloc(size_t sz);
	mfuse_EXPORTS void Free(void* ptr);

	static constexpr size_t DefaultBlock = 256;

	enum class alloc_source_e
	{
		SourceBlock = 174,
		SourceMalloc
	};

	template<typename aclass, size_t blocksize>
	class BlockAlloc_enum;

	template<typename aclass, size_t blocksize>
	class block_s {
	private:
		static constexpr size_t bitsNeeded =
			blocksize <= 0x80 ? 8 :
			blocksize <= 0x8000 ? 16 :
			blocksize <= 0x80000000 ? 32 :
			64;

	public:
		block_s();

#if !_DEBUG_MEMBLOCK
		bool usedDataAvailable() const;
		bool freeDataAvailable() const;
#endif

		static constexpr size_t getHeaderSize();

	public:
		template<size_t bits>
		struct selectType_t;

		template<> struct selectType_t<8> { using type = uint8_t; };
		template<> struct selectType_t<16> { using type = uint16_t; };
		template<> struct selectType_t<32> { using type = uint32_t; };
		template<> struct selectType_t<64> { using type = uint64_t; };

		using offset_t = typename selectType_t<bitsNeeded>::type;

		struct info_t {
			offset_t index;
			alloc_source_e source;
			static constexpr uint16_t typeSize = sizeof(aclass);
			alignas(alignof(aclass)) char data[sizeof(aclass)];
		};

	public:
#if !_DEBUG_MEMBLOCK
		info_t data[blocksize];
		offset_t prev_data[blocksize];
		offset_t next_data[blocksize];

		offset_t free_data;
		offset_t used_data;
		bool has_free_data : 1;
		bool has_used_data : 1;
#else
		offset_t data[sizeof(aclass)];
#endif

		block_s<aclass, blocksize>* prev_block;
		block_s<aclass, blocksize>* next_block;

	public:
		//static constexpr size_t headersize = offsetof(info_t, data); // so annoying, why it suddently stopped working in clang wtf?
		static constexpr size_t dataoffset = 0;
		static constexpr size_t datasize = sizeof(info_t);
	};

	template<typename aclass, size_t blocksize>
	block_s<aclass, blocksize>::block_s()
#if !_DEBUG_MEMBLOCK
	{
		info_t* header;
		offset_t curr;
		for (curr = 0; curr < blocksize - 1; ++curr)
		{
			offset_t next = curr + 1;
			header = &data[curr];
			header->source = alloc_source_e::SourceBlock;
			header->index = curr;
			//header->typeSize = sizeof(aclass);
			prev_data[next] = curr;
			next_data[curr] = next;
		}

		header = &data[curr];
		header->source = alloc_source_e::SourceBlock;
		header->index = blocksize - 1;
		//header->typeSize = sizeof(aclass);
		prev_data[0] = blocksize - 1;
		next_data[blocksize - 1] = 0;
		free_data = 0;
		prev_block = next_block = nullptr;

		has_free_data = true;
		has_used_data = false;
	}
#else
		: prev_block(nullptr)
		, next_block(nullptr)
	{

	}
#endif

#if !_DEBUG_MEMBLOCK
	template<typename aclass, size_t blocksize>
	bool block_s<aclass, blocksize>::usedDataAvailable() const {
		return has_used_data;
	}

	template<typename aclass, size_t blocksize>
	bool block_s<aclass, blocksize>::freeDataAvailable() const {
		return has_free_data;
	}
#endif

    template<typename aclass, size_t blocksize>
	constexpr size_t block_s<aclass, blocksize>::getHeaderSize()
    {
        return (size_t)(&((info_t*)NULL)->data);
    }

	template<typename aclass, size_t blocksize = DefaultBlock>
	class BlockAlloc
	{
		static_assert(blocksize >= 2, "Minimum 2x class preallocation required!!");
	public:
		BlockAlloc();
		~BlockAlloc();

		void* Alloc();
		void Free(void* ptr) noexcept;
		void FreeAll() noexcept;
		size_t Count();
		size_t BlockCount();
		size_t BlockMemory();

	private:
		friend class BlockAlloc_enum<aclass, blocksize>;
		using block_t = block_s<aclass, blocksize>;
		using block_offset_t = typename block_t::offset_t;
		using List = LinkedList<block_t*, &block_t::next_block, &block_t::prev_block>;

#if !_DEBUG_MEMBLOCK
		// Free block list [not usable], avoid calling free()
		block_t* m_FreeBlock;

		// Starting block list that will be used for further memory allocation
		//block_t* m_StartUsedBlock;

		// Full block list [not usable], no space available for further memory allocation
		//block_t* m_StartFullBlock;
		//List m_FreeBlock;
		List m_StartUsedBlock;
		List m_StartFullBlock;
#else
		List m_Block;
#endif
		size_t m_BlockCount;

	private:
		void* TakeFree(block_t* block, uintptr_t free_data);
		size_t Count(const List& block);
	};

	template<typename aclass, size_t blocksize = DefaultBlock>
	class BlockAlloc_enum
	{
	public:
		BlockAlloc_enum(BlockAlloc<aclass, blocksize>& owner);

		aclass* NextElement();
		aclass* CurrentElement();

		enum blockType_e {
			used,
			full
		};

	private:
		using block_t = block_s<aclass, blocksize>;
		using offset_t = typename block_t::offset_t;

		BlockAlloc<aclass, blocksize>* m_Owner;
		block_t* m_CurrentBlock;

#if !_DEBUG_MEMBLOCK
		offset_t m_CurrentData;
		blockType_e m_CurrentBlockType;
#endif
	};

	template<typename a, size_t b>
	BlockAlloc<a, b>::BlockAlloc()
#if !_DEBUG_MEMBLOCK
		: m_StartUsedBlock()
		, m_StartFullBlock()
	{
		m_FreeBlock = nullptr;
		m_BlockCount = 0;
	}
#else
		: m_Block()
	{
		m_BlockCount = 0;
	}
#endif

	template<typename a, size_t b>
	BlockAlloc<a, b>::~BlockAlloc()
	{
		FreeAll();
	}

	template<typename a, size_t b>
	void* BlockAlloc<a, b>::Alloc()
	{
#if _DEBUG_MEMBLOCK
		block_t* block = new(MEM::Alloc(sizeof(block_t))) block_t();

		m_Block.AddFirst(block);

		m_BlockCount++;
		return (void*)block->data;
#else
		block_t* used_block;
		block_offset_t free_data;
		block_offset_t next_data;

		//if (m_StartUsedBlock)
		if (!m_StartUsedBlock.IsEmpty())
		{
			used_block = m_StartUsedBlock.Root();

			free_data = used_block->free_data;
			next_data = used_block->next_data[free_data];

			if (next_data == free_data)
			{
				// Move the block to the next block list as there is no space available
				//m_StartUsedBlock = used_block->next_block;
				m_StartUsedBlock.SetRoot(used_block->next_block);

				//LL::SafeRemove<block_t*, &block_t::next_block, &block_t::prev_block>(used_block);
				//LL::SafeAddFront<block_t*, &block_t::next_block, &block_t::prev_block>(m_StartFullBlock, used_block);
				m_StartUsedBlock.Remove(used_block);
				m_StartFullBlock.AddFirst(used_block);

				used_block->has_free_data = false;
				return TakeFree(used_block, free_data);
			}
		}
		else
		{
			if (m_FreeBlock)
			{
				// start from the free block
				used_block = m_FreeBlock;
				m_FreeBlock = nullptr;
				free_data = used_block->free_data;
				next_data = used_block->next_data[free_data];
			}
			else
			{
				m_BlockCount++;
				// allocate and construct a new block
				used_block = new(MEM::Alloc(sizeof(block_t))) block_t();

				free_data = 0;
				next_data = 1;
			}

			//LL::SafeAddFront<block_t*, &block_t::next_block, &block_t::prev_block>(m_StartUsedBlock, used_block);
			m_StartUsedBlock.AddFirst(used_block);
		}

		const block_offset_t prev_data = used_block->prev_data[free_data];

		used_block->next_data[prev_data] = next_data;
		used_block->prev_data[next_data] = prev_data;
		used_block->free_data = next_data;
		used_block->has_free_data = true;

		if (!used_block->usedDataAvailable())
		{
			used_block->used_data = free_data;
			used_block->has_used_data = true;
			used_block->next_data[free_data] = free_data;
			used_block->prev_data[free_data] = free_data;
			return used_block->data[free_data].data;
		}

		return TakeFree(used_block, free_data);
#endif
	}

	template< typename aclass, size_t blocksize>
	void* BlockAlloc<aclass, blocksize>::TakeFree(block_t* block, uintptr_t free_data)
	{
		const block_offset_t used_data = block->used_data;
		const block_offset_t prev_data = block->prev_data[used_data];

		block->next_data[prev_data] = (block_offset_t)free_data;
		block->prev_data[used_data] = (block_offset_t)free_data;
		block->next_data[free_data] = used_data;
		block->prev_data[free_data] = prev_data;
		return block->data[free_data].data;
	}

	template<typename a, size_t b>
	void BlockAlloc<a, b>::Free(void* ptr) noexcept
	{
#if _DEBUG_MEMBLOCK
		block_s<a, b>* block = (block_s<a, b> *)ptr;

		m_Block.Remove(block);

		m_BlockCount--;
		MEM::Free(block);
#else
		// get the header of the pointer
		typename block_t::info_t* header = reinterpret_cast<typename block_t::info_t*>(static_cast<unsigned char*>(ptr) - block_t::getHeaderSize());
		const block_offset_t used_data = header->index;
		// get the block from the header
		block_t* const block = (block_t*)((uint8_t*)header - used_data * block_t::datasize - block_t::dataoffset);
		const block_offset_t next_data = block->next_data[used_data];
		if (next_data == used_data)
		{
			//LL::SafeRemoveRoot<block_t*, &block_t::next_block, &block_t::prev_block>(m_StartUsedBlock, block);
			m_StartUsedBlock.Remove(block);

			if (m_FreeBlock)
			{
				// deallocate the free block because of another deallocation
				--m_BlockCount;
				MEM::Free(m_FreeBlock);
				m_FreeBlock = nullptr;
			}

			m_FreeBlock = block;
			block->has_used_data = false;

			const block_offset_t free_data = block->free_data;
			const block_offset_t prev_data = block->prev_data[free_data];

			block->next_data[prev_data] = used_data;
			block->prev_data[free_data] = used_data;
			block->next_data[used_data] = free_data;
			block->prev_data[used_data] = prev_data;
		}
		else
		{
			const block_offset_t prev_data = block->prev_data[used_data];

			block->next_data[prev_data] = next_data;
			block->prev_data[next_data] = prev_data;
			block->used_data = next_data;
			block->has_used_data = true;

			if (block->freeDataAvailable())
			{
				const block_offset_t free_data = block->free_data;
				const block_offset_t prev_data = block->prev_data[free_data];

				block->next_data[prev_data] = used_data;
				block->prev_data[free_data] = used_data;
				block->next_data[used_data] = free_data;
				block->prev_data[used_data] = prev_data;
				return;
			}

			if (m_StartFullBlock == block)
			{
				//m_StartFullBlock = block->next_block;
				m_StartFullBlock.SetRoot(block->next_block);
			}

			m_StartFullBlock.Remove(block);
			m_StartUsedBlock.AddFirst(block);
			//LL::SafeRemove<block_t*, &block_t::next_block, &block_t::prev_block>(block);
			//LL::SafeAddFront<block_t*, &block_t::next_block, &block_t::prev_block>(m_StartUsedBlock, block);

			block->free_data = used_data;
			block->has_free_data = true;
			block->prev_data[used_data] = used_data;
			block->next_data[used_data] = used_data;
		}
#endif
	}

	template<typename a, size_t b>
	void BlockAlloc<a, b>::FreeAll() noexcept
	{
#if _DEBUG_MEMBLOCK
		block_t* block;
		block_t* next = m_Block.Root();
		for (block = next; block; block = next)
		{
			next = block->next_block;
			m_BlockCount--;
			a* ptr = (a*)block->data;
			ptr->~a();
			MEM::Free(block);
		}
		m_Block.Reset();
#else
		//block_s<a, b>* block;

		//block = m_StartFullBlock;
		typename List::iterator block = m_StartFullBlock.CreateIterator();
		while (block)
		{
			if (block->usedDataAvailable())
			{
				a* ptr = (a*)block->data[block->used_data].data;
				ptr->~a();
				Free(ptr);
				block = m_StartFullBlock.CreateIterator();
			}
		}

		block = m_StartUsedBlock.CreateIterator();
		while (block)
		{
			if (block->usedDataAvailable())
			{
				a* ptr = (a*)block->data[block->used_data].data;
				ptr->~a();
				Free(ptr);
				block = m_StartUsedBlock.CreateIterator();
			}
		}

		if (m_FreeBlock)
		{
			m_BlockCount--;
			MEM::Free(m_FreeBlock);
			m_FreeBlock = nullptr;
		}
#endif
	}

	template<typename a, size_t b>
	//size_t BlockAlloc<a, b>::Count(block_s< a, b>* block)
	size_t BlockAlloc<a, b>::Count(const List& list)
	{
		int count = 0;
#if _DEBUG_MEMBLOCK
		for (typename List::iterator block = list.CreateConstIterator(); block; block = block.Next())
		{
			count++;
		}
		return count;
#else

		for (typename List::iterator block = list.CreateConstIterator(); block; block = block.Next())
		{
			if (!block->usedDataAvailable())
			{
				continue;
			}

			const block_offset_t used_data = block->used_data;
			block_offset_t current_used_data = used_data;

			do
			{
				count++;
				current_used_data = block->next_data[current_used_data];
			} while (current_used_data != used_data);
		}

		return count;
#endif
	}

	template<typename a, size_t b>
	size_t BlockAlloc<a, b>::Count()
	{
#if _DEBUG_MEMBLOCK
		return Count(m_Block);
#else
		return Count(m_StartFullBlock) + Count(m_StartUsedBlock);
#endif
	}

	template<typename a, size_t b>
	size_t BlockAlloc<a, b>::BlockCount()
	{
		return m_BlockCount;
	}

	template<typename a, size_t b>
	size_t BlockAlloc<a, b>::BlockMemory()
	{
		return sizeof(block_s<a, b>);
	}


	template<typename a, size_t b>
	BlockAlloc_enum<a, b >::BlockAlloc_enum(BlockAlloc< a, b>& owner)
	{
		m_Owner = &owner;
		m_CurrentBlock = nullptr;
#if !_DEBUG_MEMBLOCK
		m_CurrentBlockType = BlockAlloc_enum::used;
#endif
	}

	template<typename a, size_t b>
	a* BlockAlloc_enum<a, b>::NextElement()
	{
#if _DEBUG_MEMBLOCK
		if (!m_CurrentBlock)
		{
			m_CurrentBlock = m_Owner->m_Block.Root();
		}
		else
		{
			m_CurrentBlock = m_CurrentBlock->next_block;
		}
		return (a*)m_CurrentBlock;
#else
		// search for a valid block type
		while (!m_CurrentBlock)
		{
			switch (m_CurrentBlockType)
			{
			case blockType_e::used:
				m_CurrentBlock = m_Owner->m_StartUsedBlock.Root();
				break;
			case blockType_e::full:
				m_CurrentBlock = m_Owner->m_StartFullBlock.Root();
				break;
			default:
				return nullptr;
			}

			reinterpret_cast<uint8_t&>(m_CurrentBlockType)++;

		_label:
			for (; m_CurrentBlock; m_CurrentBlock = m_CurrentBlock->next_block)
			{
				if (m_CurrentBlock->usedDataAvailable())
				{
					m_CurrentData = m_CurrentBlock->used_data;
					return reinterpret_cast<a*>(m_CurrentBlock->data[m_CurrentData].data);
				}
			}
		}

		m_CurrentData = m_CurrentBlock->next_data[m_CurrentData];

		if (m_CurrentData == m_CurrentBlock->used_data)
		{
			// found an object
			m_CurrentBlock = m_CurrentBlock->next_block;
			goto _label;
		}

		return reinterpret_cast<a*>(m_CurrentBlock->data[m_CurrentData].data);
#endif
	}

	template<typename a, size_t b>
	a* BlockAlloc_enum<a, b>::CurrentElement()
	{
		return m_CurrentBlock;
	}

	template<typename aclass, size_t blocksize = DefaultBlock>
	class BlockAllocSafe : public BlockAlloc<aclass, blocksize>
	{
	public:
		void* Alloc();
		void Free(void* ptr);
		void FreeAll();
		size_t Count();

	private:
		using super = BlockAlloc<aclass, blocksize>;

		std::shared_mutex mutex;
	};

	template<typename a, size_t b>
	void* BlockAllocSafe<a, b>::Alloc()
	{
		std::shared_lock<std::shared_mutex> lock(mutex);
		return super::Alloc();
	}

	template<typename a, size_t b>
	void BlockAllocSafe<a, b>::Free(void* ptr)
	{
		std::shared_lock<std::shared_mutex> lock(mutex);
		return super::Free(ptr);
	}

	template<typename a, size_t b>
	void BlockAllocSafe<a, b>::FreeAll()
	{
		std::shared_lock<std::shared_mutex> lock(mutex);
		// the parent will call its own version of Free()
		// so there won't be any recursive lock
		return super::FreeAll();
	}

	template<typename a, size_t b>
	size_t BlockAllocSafe<a, b>::Count()
	{
		// Lock the mutex for reading (exclusive)
		mutex.lock();
		const size_t count = super::Count();
		mutex.unlock();

		return count;
	}

	template<typename T, size_t BlockSize = DefaultBlock>
	class BlockAlloc_set
	{
	public:
		void* Alloc(size_t count);
		void* AllocTable(size_t count);
		void Free(void* ptr);
		void FreeTable(void* ptr);

	private:
		static BlockAlloc<T, BlockSize> allocator;
	};

	template<typename T, size_t BlockSize>
	BlockAlloc<T, BlockSize> BlockAlloc_set<T, BlockSize>::allocator;

	template<typename T, size_t BlockSize>
	void* BlockAlloc_set<T, BlockSize>::Alloc(size_t)
	{
		return allocator.Alloc();
	}

	template<typename T, size_t BlockSize>
	void* BlockAlloc_set<T, BlockSize>::AllocTable(size_t count)
	{
		return MEM::Alloc(count);
	}

	template<typename T, size_t BlockSize>
	void BlockAlloc_set<T, BlockSize>::Free(void* ptr)
	{
		return allocator.Free(ptr);
	}

	template<typename T, size_t BlockSize>
	void BlockAlloc_set<T, BlockSize>::FreeTable(void* ptr)
	{
		return MEM::Free(ptr);
	}

	template<typename T, size_t BlockSize = DefaultBlock>
	class BlockAllocSafe_set
	{
	public:
		void* Alloc(size_t count);
		void* AllocTable(size_t count);
		void Free(void* ptr);
		void FreeTable(void* ptr);

	private:
		static BlockAllocSafe<T, BlockSize> allocator;
	};

	template<typename T, size_t BlockSize>
	BlockAllocSafe<T, BlockSize> BlockAllocSafe_set<T, BlockSize>::allocator;

	template<typename T, size_t BlockSize>
	void* BlockAllocSafe_set<T, BlockSize>::Alloc(size_t)
	{
		return allocator.Alloc();
	}

	template<typename T, size_t BlockSize>
	void* BlockAllocSafe_set<T, BlockSize>::AllocTable(size_t count)
	{
		return MEM::Alloc(count);
	}

	template<typename T, size_t BlockSize>
	void BlockAllocSafe_set<T, BlockSize>::Free(void* ptr)
	{
		return allocator.Free(ptr);
	}

	template<typename T, size_t BlockSize>
	void BlockAllocSafe_set<T, BlockSize>::FreeTable(void* ptr)
	{
		return MEM::Free(ptr);
	}
}
}

template<typename a, size_t b>
void* operator new(size_t, mfuse::MEM::BlockAlloc<a, b>& allocator)
{
	return allocator.Alloc();
}

template<typename a, size_t b>
void operator delete(void* ptr, mfuse::MEM::BlockAlloc<a, b>& allocator) noexcept
{
	return allocator.Free(ptr);
}

template<typename a, size_t b>
void* operator new(size_t count, mfuse::MEM::BlockAlloc_set<a, b>& allocator)
{
	return allocator.Alloc(count);
}

template<typename a, size_t b>
void operator delete(void* ptr, mfuse::MEM::BlockAlloc_set<a, b>& allocator) noexcept
{
	return allocator.Free(ptr);
}
