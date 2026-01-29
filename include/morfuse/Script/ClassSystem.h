#pragma once

#include "../Common/MEM/PreAllocator.h"

#include <cstdint>

namespace mfuse
{
    class OutputInfo;

    class ClassSystem
    {
    public:
        ClassSystem();

        static ClassSystem& Get();

        size_t GetRequiredLength(size_t numEvents) const;
        void BuildEventResponses(MEM::PreAllocator& allocator);
        void ClearEventResponses(MEM::PreAllocator& allocator);

    private:
        size_t amount;
        size_t numClassesBuilt;
    };
}
