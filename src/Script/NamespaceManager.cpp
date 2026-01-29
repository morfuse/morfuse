#include <morfuse/Script/NamespaceManager.h>

using namespace mfuse;


NamespaceManager::NamespaceManager()
    : filterMode(namespaceFilterMode_e::None)
{
}

NamespaceManager::~NamespaceManager()
{
}

namespaceFilterMode_e NamespaceManager::GetFilterMode() const
{
    return filterMode;
}

void NamespaceManager::SetFilterMode(namespaceFilterMode_e newMode)
{
    filterMode = newMode;
}

void NamespaceManager::SetFilteredNamespace(const con::ContainerView<const NamespaceDef*>& newDefList)
{
    defList.FreeObjectList();
    defList.Resize(newDefList.NumObjects());

    for (size_t i = 1; i <= newDefList.NumObjects(); ++i)
    {
        const NamespaceDef* def = newDefList.ObjectAt(i);
        defList.AddObject(def->GetId());
    }
}

bool NamespaceManager::IsNamespaceAllowed(const NamespaceDef* def) const
{
    switch (filterMode)
    {
    case namespaceFilterMode_e::Inclusive:
        // global namespace, allow
        if (!def) return true;
        // inclusive, check if the namespace is in list
        return defList.ObjectInList(def->GetId());
    case namespaceFilterMode_e::Exclusive:
        // global namespace, allow
        if (!def) return true;
        // exclusive, check if the namespace is not present list
        return !defList.ObjectInList(def->GetId());
    default:
        // everything is allowed by default
        return true;
    }
}

bool NamespaceManager::IsObjectInNamespaceAllowed(const ObjectInNamespace& obj) const
{
    return IsNamespaceAllowed(obj.GetNamespace());
}
