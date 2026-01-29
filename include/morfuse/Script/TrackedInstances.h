#pragma once

#include "../Global.h"
#include "../Common/SafePtr.h"

#include <set>

namespace mfuse
{
    class AbstractClass;

    /**
     * Keep track of instances created through script
     */
    class TrackedInstances
    {
    public:
        TrackedInstances();
        ~TrackedInstances();

        mfuse_EXPORTS void Cleanup();
        mfuse_EXPORTS size_t GetNumInstances() const;

        mfuse_EXPORTS void Add(AbstractClass* instance);
        mfuse_EXPORTS void Remove(AbstractClass* instance);

    private:
        std::set<SafePtr<AbstractClass>> instances;
    };
}