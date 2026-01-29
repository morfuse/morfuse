#include <morfuse/Common/MEM/TempAlloc.h>

#include <cstdlib>
#include <algorithm>

using namespace mfuse;

MEM::TempAlloc::TempAlloc(size_t blockSize)
    : m_BlockSize(blockSize)
{
    m_CurrentMemoryBlock = nullptr;
    m_CurrentMemoryPos = 0;
    m_LastPos = 0;
}

MEM::TempAlloc::TempAlloc(MEM::TempAlloc&& other)
    : m_CurrentMemoryBlock(other.m_CurrentMemoryBlock)
    , m_CurrentMemoryPos(other.m_CurrentMemoryPos)
    , m_BlockSize(other.m_BlockSize)
    , m_LastPos(other.m_LastPos)
{
    other.m_CurrentMemoryBlock = nullptr;
    other.m_CurrentMemoryPos = 0;
    other.m_BlockSize = 0;
    other.m_LastPos = 0;
}

MEM::TempAlloc& MEM::TempAlloc::operator=(MEM::TempAlloc&& other)
{
    m_CurrentMemoryBlock = other.m_CurrentMemoryBlock;
    m_CurrentMemoryPos = other.m_CurrentMemoryPos;
    m_BlockSize = other.m_BlockSize;
    m_LastPos = other.m_LastPos;
    other.m_CurrentMemoryBlock = nullptr;
    other.m_CurrentMemoryPos = 0;
    other.m_BlockSize = 0;
    other.m_LastPos = 0;

    return *this;
}

MEM::TempAlloc::~TempAlloc()
{
    FreeAll();
}

void* MEM::TempAlloc::Alloc(size_t len)
{
    if (m_CurrentMemoryBlock && m_CurrentMemoryPos + len <= m_BlockSize)
    {
        void* data = m_CurrentMemoryBlock->GetData(m_CurrentMemoryPos);
        m_LastPos = m_CurrentMemoryPos;
        m_CurrentMemoryPos += len;
        return data;
    }
    else
    {
        return CreateBlock(len);
    }
}

void* MEM::TempAlloc::Alloc(size_t len, size_t alignment)
{
    if (m_CurrentMemoryBlock)
    {
        if (m_CurrentMemoryPos % alignment != 0) {
            m_CurrentMemoryPos += alignment - m_CurrentMemoryPos % alignment;
        }

        if (m_CurrentMemoryPos + len <= m_BlockSize)
        {
            void* data = m_CurrentMemoryBlock->GetData(m_CurrentMemoryPos);
            m_LastPos = m_CurrentMemoryPos;
            m_CurrentMemoryPos += len;
            return data;
        }
    }

    return CreateBlock(len);
}

void MEM::TempAlloc::Free(void* ptr)
{
    if (ptr == m_CurrentMemoryBlock->GetData(m_LastPos))
    {
        m_CurrentMemoryPos = m_LastPos;
    }
}

void MEM::TempAlloc::FreeAll(void)
{
    while (m_CurrentMemoryBlock)
    {
        tempBlock_t* prev_block = m_CurrentMemoryBlock->prev;
        free(m_CurrentMemoryBlock);
        m_CurrentMemoryBlock = prev_block;
    }
}

void* MEM::TempAlloc::CreateBlock(size_t len)
{
    m_CurrentMemoryPos = len;

    // allocate a new block
    tempBlock_t* prev_block = m_CurrentMemoryBlock;
    m_CurrentMemoryBlock = (tempBlock_t*)malloc(sizeof(tempBlock_t) + std::max(m_BlockSize, len));
    m_CurrentMemoryBlock->prev = prev_block;
    return m_CurrentMemoryBlock->GetData();
}

void* MEM::tempBlock_t::GetData()
{
    return (void*)(this + 1);
}

void* MEM::tempBlock_t::GetData(size_t pos)
{
    return (uint8_t*)(this + 1) + pos;
}
