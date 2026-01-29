#include <morfuse/Script/TrackedInstances.h>
#include <morfuse/Common/AbstractClass.h>

using namespace mfuse;


TrackedInstances::TrackedInstances()
{

}

TrackedInstances::~TrackedInstances()
{
    Cleanup();
}

void mfuse::TrackedInstances::Add(AbstractClass* instance)
{
    instances.insert(instance);
}

void mfuse::TrackedInstances::Remove(AbstractClass* instance)
{
    instances.erase(instance);
}

void mfuse::TrackedInstances::Cleanup()
{
    auto it = instances.begin();
    while (it != instances.end())
    {
        if (!it->Valid())
        {
            // existing instance was deleted
            it = instances.erase(it);
        }
        else if (it->IsLastReference())
        {
            // no reference so safely delete
            AbstractClass* c = it->Pointer();
            it = instances.erase(it);
            delete c;
        }
        else
        {
            it++;
        }
    }
}

size_t TrackedInstances::GetNumInstances() const
{
    return instances.size();
}
