#pragma once

#include "../Global.h"
#include <type_traits>

namespace mfuse
{
    namespace LinkList
    {
        using namespace std;

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        bool IsEmpty(T rootnode)
        {
            return ((rootnode->*next) == (rootnode)) &&
                ((rootnode->*prev) == (rootnode));
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void Reset(T rootnode)
        {
            (rootnode->*next) = (rootnode->*prev) = rootnode;
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void Add(T rootnode, const T& newnode)
        {
            (newnode->*next) = rootnode;
            (newnode->*prev) = (rootnode->*prev);
            (rootnode->*prev->*next) = newnode;
            (rootnode->*prev) = newnode;
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void AddFirst(T rootnode, const T& newnode)
        {
            (newnode->*next) = (rootnode->*next);
            (newnode->*prev) = rootnode;
            (rootnode->*next->*prev) = newnode;
            (rootnode->*next) = newnode;
        }

        template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
        void Remove(const T& node)
        {
            (node->*prev->*next) = (node->*next);
            (node->*next->*prev) = (node->*prev);
            (node->*next) = node;
            (node->*prev) = node;
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void Transfer(T oldroot, T newroot)
        {
            if (oldroot->*prev != oldroot)
            {
                (oldroot->*prev->*next) = newroot;
                (oldroot->*next->*prev) = (newroot->*prev);
                (newroot->*prev->*next) = (oldroot->*next);
                (newroot->*prev) = (oldroot->*prev);
                (oldroot->*next) = oldroot;
                (oldroot->*prev) = oldroot;
            }
        }

        template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::* prev>
        void Move(const T& node, T newroot)
        {
            Remove<T, next, prev>(node);
            Add<T, next, prev>(newroot, node);
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void Reverse(T root)
        {
            T *newend, *trav, *tprev;

            newend = root->next;
            for (trav = root->prev; trav != newend; trav = tprev)
            {
                tprev = trav->prev;
                Move<T, next, prev>(trav, newend);
            }
        }

        template<typename T, T remove_pointer_t<T>::* next, T remove_pointer_t<T>::*prev, typename T2, T2 remove_pointer_t<T>::*sortparm>
        void SortedInsertion(T& rootnode, T insertnode)
        {
            T* hoya;

            hoya = rootnode->next;
            while ((hoya != rootnode) && (insertnode->*sortparm > hoya->*sortparm))
            {
                hoya = hoya->next;
            }
            Add(hoya, insertnode, next, prev);
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void SafeAdd(T& rootnode, const T& newnode)
        {
            (newnode->*next) = T();
            (newnode->*prev) = rootnode;
            if (rootnode) (rootnode->*next) = newnode;
            rootnode = newnode;
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void SafeAddFront(T& rootnode, const T& newnode)
        {
            (newnode->*next) = rootnode;
            (newnode->*prev) = T();
            if (rootnode) (rootnode->*prev) = newnode;
            rootnode = newnode;
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void SafeRemove(const T& node)
        {
            if (node->*prev) (node->*prev->*next) = (node->*next);
            if (node->*next) (node->*next->*prev) = (node->*prev);
        }

        template<typename T, T remove_pointer_t<T>::*next, T remove_pointer_t<T>::*prev>
        void SafeRemoveRoot(T& rootnode, const T& node)
        {
            if (rootnode == node) rootnode = (node->*next);
            if (node->*prev) (node->*prev->*next) = (node->*next);
            if (node->*next) (node->*next->*prev) = (node->*prev);
        }
    }
    namespace LL = LinkList;


    template<typename Type, Type* Type::*next, Type* Type::*prev>
    class LinkListIterator
    {
    public:
        LinkListIterator() = default;
        LinkListIterator(Type* initial)
            : currentNode(initial)
        {}

        Type* Node() const
        {
            return currentNode;
        }

        LinkListIterator Next() const
        {
            return LinkListIterator(currentNode->*next);
        }

        LinkListIterator Prev() const
        {
            return LinkListIterator(currentNode->*prev);
        }

        operator bool() const
        {
            return currentNode != nullptr;
        }

        operator Type* () const
        {
            return Node();
        }

        operator const Type* () const
        {
            return Node();
        }

        Type* operator->() const
        {
            return currentNode;
        }

    private:
        Type* currentNode;
    };
    template<typename T, auto next, auto prev>
    class LinkedList;

    template<typename T, T* T::*next, T* T::*prev>
    class LinkedList<T, next, prev>
    {
    public:
        using iterator = LinkListIterator<T, next, prev>;
        using Type = T;

        constexpr bool IsEmpty() const
        {
            return rootnode.*next == rootnode && rootnode.*prev == rootnode;
        }

        constexpr void Reset()
        {
            (rootnode.*next) = (rootnode.*prev) = rootnode;
        }

        constexpr void Add(Type* newnode)
        {
            (newnode->*next) = rootnode;
            (newnode->*prev) = (rootnode.*prev);
            (rootnode.*prev->*next) = newnode;
            (rootnode.*prev) = newnode;
        }

        constexpr void AddFirst(Type* newnode)
        {
            (newnode->*next) = (rootnode.*next);
            (newnode->*prev) = rootnode;
            (rootnode.*next->*prev) = newnode;
            (rootnode.*next) = newnode;
        }

        constexpr void Remove(Type* node)
        {
            (node->*prev->*next) = (node->*next);
            (node->*next->*prev) = (node->*prev);
            (node->*next) = node;
            (node->*prev) = node;
        }

        constexpr void Transfer(Type* oldroot)
        {
            if (oldroot->*prev != oldroot)
            {
                Type* const newroot = rootnode;
                (oldroot->*prev->*next) = newroot;
                (oldroot->*next->*prev) = (newroot->*prev);
                (newroot->*prev->*next) = (oldroot->*next);
                (newroot->*prev) = (oldroot->*prev);
                (oldroot->*next) = oldroot;
                (oldroot->*prev) = oldroot;
            }
        }

        constexpr void Reverse(Type* root)
        {
            Type* newend, * trav, * tprev;

            newend = root->*next;
            for (trav = root->*prev; trav != newend; trav = tprev)
            {
                tprev = trav->*prev;
                Move(trav, newend);
            }
        }

        template<typename TSortParm, TSortParm Type::*sortparm>
        constexpr void SortedInsertion(Type* insertnode)
        {
            Type* hoya;

            hoya = rootnode.*next;
            while ((hoya != rootnode) && (insertnode->*sortparm > hoya->*sortparm))
            {
                hoya = hoya->*next;
            }
            Add(hoya, insertnode, insertnode->*next, insertnode->*prev);
        }

        constexpr void Move(Type* newnode, Type* older)
        {
            newnode->*prev->*next = newnode->*next;
            newnode->*next->*prev = newnode->*prev;
            newnode->*next = older->*next;
            newnode->*prev = older->*prev;
            newnode->*next->*prev = newnode;
            newnode->*prev->*next = newnode;
            older->*prev = rootnode;
            older->*next = rootnode;
        }

        constexpr T& Root()
        {
            return rootnode;
        }

        constexpr iterator CreateIterator()
        {
            return iterator(&rootnode);
        }

    private:
        T rootnode;
    };

    template<typename T, T* T::*next, T* T::*prev>
    class LinkedList<T*, next, prev>
    {
    public:
        using iterator = LinkListIterator<T, next, prev>;
        using Type = T;

        constexpr bool IsEmpty() const
        {
            return !rootnode;
        }

        constexpr void Reset()
        {
            rootnode = nullptr;
        }

        constexpr void Add(Type* newnode)
        {
            if (!rootnode)
            {
                // set both values
                tail = rootnode = newnode;
                newnode->*next = newnode->*prev = nullptr;
            }
            else
            {
                // there is obviously a tail
                tail->*next = newnode;
                newnode->*prev = tail;
                newnode->*next = nullptr;
                tail = newnode;
            }
        }

        constexpr void AddFirst(Type* newnode)
        {
            if (!rootnode)
            {
                // set the tail
                tail = newnode;
                newnode->*next = newnode->*prev = nullptr;
            }
            else
            {
                newnode->*next = rootnode;
                newnode->*prev = nullptr;
                rootnode->*prev = newnode;
            }
            rootnode = newnode;
        }

        constexpr void Insert(Type* currentnode, Type* newnode)
        {
            newnode->*prev = currentnode->*prev;
            newnode->*next = currentnode;
            //if (currentnode->*prev) currentnode->*prev->*next = newnode;
            if (currentnode->*prev) currentnode->*prev->*next = newnode;
            currentnode->*prev = newnode;
            //rootnode = newnode;
        }

        constexpr void Remove(Type* node)
        {
            if (node == rootnode) rootnode = rootnode->*next;
            if (node == tail) tail = tail->*prev;
            if (node->*prev) node->*prev->*next = node->*next;
            if (node->*next) node->*next->*prev = node->*prev;
        }

        constexpr void Transfer(Type* oldroot)
        {
            if (oldroot->*prev != oldroot)
            {
                Type* const newroot = rootnode;
                (oldroot->*prev->*next) = newroot;
                (oldroot->*next->*prev) = (newroot->*prev);
                (newroot->*prev->*next) = (oldroot->*next);
                (newroot->*prev) = (oldroot->*prev);
                (oldroot->*next) = oldroot;
                (oldroot->*prev) = oldroot;
            }
        }

        constexpr void Reverse(Type* root)
        {
            Type *newend, *trav, *tprev;

            newend = root->*next;
            for (trav = root->*prev; trav != newend; trav = tprev)
            {
                tprev = trav->*prev;
                LL::Move<T*, next, prev>(trav, newend);
            }
        }

        template<typename TSortParm, TSortParm Type::*sortparm>
        constexpr void SortedInsertion(Type* insertnode)
        {
            Type* hoya;

            hoya = rootnode->*next;
            while ((hoya != rootnode) && (insertnode->*sortparm > hoya->*sortparm))
            {
                hoya = hoya->*next;
            }
            Add(hoya, insertnode, insertnode->*next, insertnode->*prev);
        }

        constexpr void Move(Type* newnode, Type* older)
        {
            if (rootnode == older) rootnode = newnode;
            if (tail == older) tail = newnode;

            if (newnode->*prev) newnode->*prev->*next = newnode->*next;
            if (newnode->*next) newnode->*next->*prev = newnode->*prev;
            newnode->*next = older->*next;
            newnode->*prev = older->*prev;
            if (newnode->*prev) older->*prev->*next = newnode;
            if (newnode->*next) older->*next->*prev = newnode;
            older->*prev = nullptr;
            older->*next = nullptr;
        }

        constexpr void SetRoot(T* newroot)
        {
            rootnode = newroot;
        }

        constexpr T* TakeRoot()
        {
            T* const node = rootnode;
            rootnode = nullptr;
            return node;
        }

        constexpr T* Root() const
        {
            return rootnode;
        }

        constexpr T* Tail() const
        {
            return tail;
        }

        constexpr iterator CreateIterator()
        {
            return iterator(rootnode);
        }

        constexpr iterator CreateConstIterator() const
        {
            return iterator(rootnode);
        }

        constexpr bool operator==(T* other) const { return rootnode == other; }
        constexpr bool operator!=(T* other) const { return rootnode != other; }

    private:
        T* rootnode;
        T* tail;
    };
}
