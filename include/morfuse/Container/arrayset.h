#pragma once

#include "../Common/MEM/BlockAlloc.h"
#include "set_generic_hash.h"
#include "set_types.h"

#include <cstdint>
#include <utility>
#include <algorithm>

namespace mfuse
{
namespace con
{
    template<typename KeyT, typename ValueT>
    class EntryArraySet
    {
    public:
        EntryArraySet(const ValueT& inValue);
        EntryArraySet(const ValueT& inValue, uintptr_t inIndex);

        KeyT& Key() noexcept { return value; }
        const KeyT& Key() const noexcept { return value; }
        ValueT& Value() noexcept { return value; }
        const ValueT& Value() const noexcept { return value; }
        EntryArraySet* Next() const noexcept { return next; }
        uintptr_t Index() const noexcept { return index; }
        void SetNext(EntryArraySet* nextValue) noexcept { next = nextValue; }

    private:
        ValueT value;
        EntryArraySet* next;
        uintptr_t index;
    };

    template<typename KeyT, typename ValueT>
    EntryArraySet<KeyT, ValueT>::EntryArraySet(const ValueT& inValue)
        : value(inValue)
        , next(nullptr)
        , index(0)
    {
    }

    template<typename KeyT, typename ValueT>
    EntryArraySet<KeyT, ValueT>::EntryArraySet(const ValueT& inValue, uintptr_t inIndex)
        : value(inValue)
        , next(nullptr)
        , index(inIndex)
    {
    }

    template<typename Key, typename Value>
    using arrayset_default_allocator = MEM::BlockAllocSafe_set<EntryArraySet<Key, Value>>;

    /**
     * A set of entries.
     * It is the same as con::set, except that it stores element index in a reverse table.
     * Elements can be lookup by index.
     */
    template<
        typename KeyT,
        typename ValueT,
        /** Hash function used to hash the key. */
        typename HashT = Hash<KeyT>,
        typename KeyEqT = EqualTo<KeyT>,
        /** The allocator to use for allocating table and entries. */
        typename AllocatorT = arrayset_default_allocator<KeyT, ValueT>
    >
    class arrayset
    {
    public:
        arrayset();
        ~arrayset();

        void clear();
        void resize(size_t count = 0);
        void shrink();
        size_t size() const noexcept;
        size_t allocated() const noexcept;

        ValueT* findKeyValue(const KeyT& key);
        const ValueT* findKeyValue(const KeyT& key) const;
        uintptr_t findKeyIndex(const KeyT& key) const;
        ValueT& addKeyValue(const KeyT& key);
        uintptr_t addKeyIndex(const KeyT& key);
        uintptr_t addKeyIndex(const KeyT& key, bool& wasAdded);
        bool remove(const KeyT& key);

        ValueT& operator[](uintptr_t index);
        const ValueT& operator[](uintptr_t index) const;

        AllocatorT& getAllocator() noexcept;
        const AllocatorT& getAllocator() const noexcept;

        static size_t countEntryBytes(size_t count) noexcept;

    private:
        EntryArraySet<KeyT, ValueT>* findKeyEntry(const KeyT& key);
        const EntryArraySet<KeyT, ValueT>* findKeyEntry(const KeyT& key) const;
        EntryArraySet<KeyT, ValueT>* addKeyEntry(const KeyT& key);
        EntryArraySet<KeyT, ValueT>* addKeyEntry(const KeyT& key, bool& wasAdded);
        EntryArraySet<KeyT, ValueT>* addNewKeyEntry(const KeyT& key, uintptr_t index);

        template<typename...Args>
        EntryArraySet<KeyT, ValueT>* NewEntry(Args&& ...args)
        {
            void* const mem = Entry_allocator.Alloc(sizeof(EntryArraySet<KeyT, ValueT>));
            return new (mem) EntryArraySet<KeyT, ValueT>(std::forward<Args>(args)...);
        }

        void DeleteEntry(EntryArraySet<KeyT, ValueT>* entry)
        {
            entry->~EntryArraySet();
            Entry_allocator.Free(entry);
        }

        void rehash();

    private:
        /** Hash table. */
        AllocatorT Entry_allocator;
        EntryArraySet<KeyT, ValueT>** table;
        /** The index table. */
        EntryArraySet<KeyT, ValueT>** reverseTable;
        size_t tableLength;
        size_t threshold;
        /** The number of entries. */
        size_t count;
        uint16_t tableLengthIndex;
        EntryArraySet<KeyT, ValueT>* defaultEntry;
    };

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::arrayset()
    {
        tableLength = 1;
        table = &defaultEntry;
        reverseTable = (&this->defaultEntry) - 1;

        threshold = 1;
        count = 0;
        tableLengthIndex = 0;

        defaultEntry = nullptr;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::~arrayset()
    {
        clear();
    }

    template<typename Key, typename Value, typename HashT, typename KeyEqT, typename AllocatorT>
    size_t arrayset<Key, Value, HashT, KeyEqT, AllocatorT>::countEntryBytes(size_t count) noexcept
    {
        if (count > 1) {
            return (sizeof(EntryArraySet<Key, Value>) + sizeof(EntryArraySet<Key, Value>*)) * count;
        }
        else if (count == 1) {
            return (sizeof(EntryArraySet<Key, Value>)) * count;
        }

        return 0;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    AllocatorT& arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::getAllocator() noexcept
    {
        return Entry_allocator;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    const AllocatorT& arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::getAllocator() const noexcept
    {
        return Entry_allocator;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    ValueT* arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyValue(const KeyT& key)
    {
        EntryArraySet<KeyT, ValueT>* entry = findKeyEntry(key);

        if (entry) {
            return &entry->value;
        }
        else {
            return nullptr;
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    const ValueT* arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyValue(const KeyT& key) const
    {
        const EntryArraySet<KeyT, ValueT>* entry = findKeyEntry(key);

        if (entry) {
            return &entry->value;
        }
        else {
            return nullptr;
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    uintptr_t arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyIndex(const KeyT& key) const
    {
        const EntryArraySet<KeyT, ValueT>* entry = findKeyEntry(key);

        if (entry != NULL) {
            return entry->Index();
        }
        else {
            return 0;
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    ValueT& arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyValue(const KeyT& key)
    {
        EntryArraySet<KeyT, ValueT>* entry = addKeyEntry(key);
        return entry->Value();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    uintptr_t arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyIndex(const KeyT& key)
    {
        const EntryArraySet<KeyT, ValueT>* entry = addKeyEntry(key);
        return entry->Index();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    uintptr_t arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyIndex(const KeyT& key, bool& wasAdded)
    {
        const EntryArraySet<KeyT, ValueT>* entry = addKeyEntry(key, wasAdded);
        return entry->Index();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    bool arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::remove(const KeyT& key)
    {
        for (uintptr_t i = 1; i <= tableLength; i++)
        {
            if (reverseTable[i] &&
                KeyEqT()(reverseTable[i]->Key(), key))
            {
                reverseTable[i] = nullptr;
            }
        }

        intptr_t index = HashT()(key) % tableLength;
        EntryArraySet<KeyT, ValueT>* prev = nullptr;
        EntryArraySet<KeyT, ValueT>* entry;

        for (entry = table[index]; entry; entry = entry->Next())
        {
            // just to make sure we're using the correct overloaded operator
            if (!KeyEqT()(entry->Key(), key))
            {
                prev = entry;
                continue;
            }

            if (defaultEntry == entry) defaultEntry = prev;

            if (prev) {
                prev->SetNext(entry->Next());
            }
            else {
                table[index] = entry->Next();
            }

            count--;
            DeleteEntry(entry);

            return true;
        }

        return false;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    ValueT& arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::operator[](uintptr_t index)
    {
        return reverseTable[index]->Value();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    const ValueT& arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::operator[](uintptr_t index) const
    {
        return reverseTable[index]->Value();
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    void arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::resize(size_t newCount)
    {
        const size_t oldTableLength = tableLength;

        tableLength = newCount;
        threshold = tableLength;
        //threshold = (uint32_t)((float)tableLength * 0.75);

        // save old values to be able to copy tables
        EntryArraySet<KeyT, ValueT>** const oldTable = table;
        EntryArraySet<KeyT, ValueT >** const oldReverseTable = reverseTable;
        // allocate a new table
        table = new(
            Entry_allocator.AllocTable(
                sizeof(EntryArraySet<KeyT, ValueT>*) * tableLength * 2
            )
        ) EntryArraySet<KeyT, ValueT> *[tableLength]();
        // allocate a bigger reverse table
        reverseTable = new(table + tableLength) EntryArraySet<KeyT, ValueT> *[tableLength]() - 1;

        // rehash the table
        for (uintptr_t i = std::min(oldTableLength, newCount); i > 0; i--)
        {
            // rehash all entries from the old table
            EntryArraySet<KeyT, ValueT>* old;
            for (EntryArraySet<KeyT, ValueT>* e = oldTable[i - 1]; e != nullptr; e = old)
            {
                old = e->Next();

                // insert the old entry to the table hashindex
                const uintptr_t index = HashT()(e->Value()) % tableLength;

                e->SetNext(table[index]);
                table[index] = e;
            }
            reverseTable[i] = oldReverseTable[i];
        }

        if (oldTableLength > 1)
        {
            // delete the previous table
            Entry_allocator.FreeTable(oldTable);
            //Entry_allocator.FreeTable(oldReverseTable + 1);
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    void arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::rehash()
    {
        size_t newLen = 0;
        for (size_t i = 0; i < sizeof(set_primes) / sizeof(set_primes[0]); ++i)
        {
            newLen = set_primes[i];
            if(newLen > tableLength)
            {
                tableLengthIndex = (uint16_t)i;
                break;
            }
        }

        resize(newLen);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    void arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::shrink()
    {
        if (count) {
            resize(count);
        } else {
            clear();
        }
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    size_t arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::size() const noexcept
    {
        return count;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    size_t arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::allocated() const noexcept
    {
        return tableLength;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    void arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::clear()
    {
        if (this->tableLength > 1)
        {
            //Entry_allocator.FreeTable(reverseTable + 1);
            reverseTable = (&this->defaultEntry) - 1;
        }

        for (uintptr_t i = 0; i < tableLength; i++)
        {
            EntryArraySet<KeyT, ValueT>* next;
            for (EntryArraySet<KeyT, ValueT>* entry = table[i]; entry; entry = next)
            {
                next = entry->Next();
                DeleteEntry(entry);
            }
        }

        if (tableLength > 1)
        {
            Entry_allocator.FreeTable(table);
        }

        tableLength = 1;
        table = &defaultEntry;

        threshold = 1;
        count = 0;
        tableLengthIndex = 0;

        defaultEntry = nullptr;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    EntryArraySet<KeyT, ValueT>* arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyEntry(const KeyT& key)
    {
        EntryArraySet<KeyT, ValueT>* entry = table[HashT()(key) % tableLength];

        for (; entry; entry = entry->next)
        {
            if (KeyEqT()(entry->Key(), key)) {
                return entry;
            }
        }

        return nullptr;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    const EntryArraySet<KeyT, ValueT>* arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyEntry(const KeyT& key) const
    {
        const EntryArraySet<KeyT, ValueT>* entry = table[HashT()(key) % tableLength];

        for (; entry; entry = entry->Next())
        {
            if (KeyEqT()(entry->Key(), key)) {
                return entry;
            }
        }

        return nullptr;
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    EntryArraySet<KeyT, ValueT>* arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyEntry(const KeyT& key)
    {
        const uintptr_t index = HashT()(key) % tableLength;
        for (EntryArraySet<KeyT, ValueT>* entry = table[index]; entry; entry = entry->Next())
        {
            if (KeyEqT()(entry->Key(), key)) {
                return entry;
            }
        }

        return addNewKeyEntry(key, index);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    EntryArraySet<KeyT, ValueT>* arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyEntry(const KeyT& key, bool& wasAdded)
    {
        const uintptr_t index = HashT()(key) % tableLength;
        for (EntryArraySet<KeyT, ValueT>* entry = table[index]; entry; entry = entry->next)
        {
            if (KeyEqT()(entry->Key(), key))
            {
                wasAdded = false;
                return entry;
            }
        }

        wasAdded = true;
        return addNewKeyEntry(key, index);
    }

    template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
    EntryArraySet<KeyT, ValueT> *arrayset<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addNewKeyEntry(const KeyT& key, uintptr_t index)
    {
        if (count >= threshold)
        {
            rehash();
            index = HashT()(key) % tableLength;
        }

        count++;

        EntryArraySet<KeyT, ValueT>* const entry = NewEntry(key, count);

        if (defaultEntry == nullptr)
        {
            defaultEntry = entry;
            entry->SetNext(nullptr);
        }
        else {
            entry->SetNext(table[index]);
        }

        table[index] = entry;
        reverseTable[count] = entry;

        return entry;
    }
}
};
