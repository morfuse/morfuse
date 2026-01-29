#pragma once

#include "../Common/rawchar.h"
#include "../Common/Linklist.h"
#include "MessageableException.h"

#include <exception>

namespace mfuse
{
    using namespaceNum_t = uint32_t;

    class NamespaceDef
    {
    public:
        mfuse_EXPORTS NamespaceDef(const rawchar_t* namespaceValue, const rawchar_t* descriptionValue);
        mfuse_EXPORTS ~NamespaceDef();

        // non-copyable
        NamespaceDef(const NamespaceDef&) = delete;
        NamespaceDef& operator=(const NamespaceDef&) = delete;
        
        // non-movable
        NamespaceDef(NamespaceDef&&) = delete;
        NamespaceDef& operator=(NamespaceDef&&) = delete;

        mfuse_EXPORTS const rawchar_t* GetName() const;
        mfuse_EXPORTS const rawchar_t* GetDescription() const;
        mfuse_EXPORTS namespaceNum_t GetId() const;

    private:
        const rawchar_t* namespaceName;
        const rawchar_t* description;
        NamespaceDef* prev;
        NamespaceDef* next;
        namespaceNum_t num;

        using List = LinkedList<NamespaceDef*, &NamespaceDef::prev, &NamespaceDef::next>;
        static List head;
        static namespaceNum_t lastNum;
    };

    class mfuse_EXPORTS ObjectInNamespace
    {
    public:
        ObjectInNamespace();
        ObjectInNamespace(const NamespaceDef& def);
        ~ObjectInNamespace();

        const NamespaceDef* GetNamespace() const;

    private:
        const NamespaceDef* namespaceDef;
    };

    namespace NamespaceDefErrors
    {
        class Base : public std::exception {};

        class AlreadyExists : public Base, public Messageable
        {
        public:
            AlreadyExists(const NamespaceDef& otherRef);

            const rawchar_t* GetName() const;
            const rawchar_t* GetDescription() const;

            const char* what() const noexcept override;

        private:
            const NamespaceDef& other;
        };
    }
}
