#pragma once

#include "../../Global.h"
#include <cstdint>
#include <cstddef>

namespace mfuse
{
namespace MEM
{
    class mfuse_EXPORTS DefaultAlloc
    {
    public:
        void* Alloc(size_t size);
        void Free(void* ptr);
    };

    class mfuse_EXPORTS DefaultAlloc_set : public DefaultAlloc
    {
    public:
        void* AllocTable(size_t size);
        void FreeTable(void* ptr);
    };
}
}
