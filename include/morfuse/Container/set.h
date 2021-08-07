#pragma once

#include "../Global.h"
#include "set_generic_hash.h"
#include "set_types.h"
#include "../Common/MEM/BlockAlloc.h"

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <algorithm>

namespace mfuse
{
	class Class;
	class Archiver;

	namespace con
	{
	template<typename key, typename value, typename HashT, typename KeyEqT, typename AllocatorT>
	class map_enum;

	template<typename key, typename value, typename HashT, typename KeyEqT, typename AllocatorT>
	class set_enum;

	template<typename T>
	struct set_is_pointer { static const bool value = false; };

	template<typename T>
	struct set_is_pointer<T*> { static const bool value = true; };

	template<typename KeyT, typename ValueT>
	class Entry
	{
	public:
		Entry(const KeyT& inKey);
		Entry(const KeyT& inKey, const ValueT& inValue);

		KeyT& Key() { return key; }
		const KeyT& Key() const { return key; }
		ValueT& Value() { return value; }
		const ValueT& Value() const { return value; }
		Entry* Next() const { return next; }
		void SetNext(Entry* nextValue) { next = nextValue; }

	private:
		KeyT key;
		ValueT value;
		Entry* next;
	};

	template<typename KeyT, typename ValueT>
	using set_default_allocator = MEM::BlockAllocSafe_set<Entry<KeyT, ValueT>>;

	/**
	 * Map a set of key:values in an hash-table.
	 * 
	 * @tparam KeyT The type of the key.
	 * @tparam ValueT The type of the value.
	 * @tparam HashT The hash function to use for hashing elements.
	 * @tparam AllocatorT The allocator type to use for allocating table and elements.
	 *   The allocator must contain: Alloc(size_t elemsize): used to allocate an entry and return a memory-allocated region.
	 *   Free(void* ptr): frees a memory region returned by Alloc().
	 *   AllocTable(size_t tablesize): allocate a table.
	 *   FreeTable(void* ptr): free a previously allocated table from AllocTable().
	 */
	template<
		typename KeyT,
		typename ValueT,
		/** Hash function used to hash the key. */
		typename HashT = Hash<KeyT>,
		typename KeyEqT = EqualTo<KeyT>,
		/** The allocator to use for allocating table and entries. */
		typename AllocatorT = set_default_allocator<KeyT, ValueT>
	>
	class set
	{
		friend class set_enum <KeyT, ValueT, HashT, KeyEqT, AllocatorT>;

	public:
		set();
		~set();

		void clear();
		void resize(size_t count = 0);
		void shrink();
		bool isEmpty();
		size_t size();
		size_t allocated();

		ValueT* findKeyValue(const KeyT& key);
		const ValueT* findKeyValue(const KeyT& key) const;
		ValueT& addKeyValue(const KeyT& key);
		ValueT& addKeyValue(const KeyT& key, const ValueT& initVal);

		bool remove(const KeyT& key);

		AllocatorT& getAllocator();
		const AllocatorT& getAllocator() const;

		static size_t countEntryBytes(size_t count);

	private:
		Entry<KeyT, ValueT>* findKeyEntry(const KeyT& key);
		const Entry<KeyT, ValueT>* findKeyEntry(const KeyT& key) const;
		Entry<KeyT, ValueT>* addKeyEntry(const KeyT& key);
		Entry<KeyT, ValueT>* addKeyEntry(const KeyT& key, const ValueT& initVal);
		Entry<KeyT, ValueT>* addNewKeyEntry(const KeyT& key, uintptr_t index);
		Entry<KeyT, ValueT>* addNewKeyEntry(const KeyT& key, const ValueT& initVal, uintptr_t index);
		uintptr_t increment(const KeyT& key, uintptr_t index);
		void insertEntry(Entry<KeyT, ValueT>& entry, uintptr_t index);

		template<typename...Args>
		Entry<KeyT, ValueT>* NewEntry(Args&& ...args)
		{
			void* const mem = Entry_allocator.Alloc(sizeof(Entry<KeyT, ValueT>));
			return new (mem) Entry<KeyT, ValueT>(std::forward<Args>(args)...);
		}

		void DeleteEntry(Entry<KeyT, ValueT>* entry)
		{
			entry->~Entry();
			Entry_allocator.Free(entry);
		}

		void rehash();

	protected:
		/** Hash table. */
		AllocatorT Entry_allocator;
		Entry<KeyT, ValueT>** table;
		uintptr_t tableLength;
		uintptr_t threshold;
		/** The number of entries. */
		uintptr_t count;
		Entry<KeyT, ValueT>* defaultEntry;
		uint16_t tableLengthIndex;
	};

	template<
		typename KeyT,
		typename ValueT,
		typename HashT = Hash<KeyT>,
		typename KeyEqT = EqualTo<KeyT>,
		typename AllocatorT = set_default_allocator<KeyT, ValueT>
	>
	class set_enum
	{
		friend class map_enum <KeyT, ValueT, HashT, KeyEqT, AllocatorT>;

	protected:
		set<KeyT, ValueT, HashT, KeyEqT, AllocatorT> *m_Set;
		uintptr_t m_Index;
		Entry<KeyT, ValueT> *m_CurrentEntry;
		Entry<KeyT, ValueT> *m_NextEntry;

	public:

		set_enum();
		set_enum(set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>& set);

		bool					operator=(set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>& set);

		Entry<KeyT, ValueT>		*NextElement(void);
		Entry<KeyT, ValueT>		*CurrentElement(void);
	};

	/**
	 * Same as con_set. Except it is more beginner-friendly.
	 * operator[key] can be used to find or create a key entry with the key.
	 */
	template<
		typename KeyT,
		typename ValueT,
		typename HashT = Hash<KeyT>,
		typename KeyEqT = EqualTo<KeyT>,
		typename AllocatorT = set_default_allocator<KeyT, ValueT>
	>
	class map {
		friend class map_enum <KeyT, ValueT, HashT, KeyEqT, AllocatorT>;

	private:
		set<KeyT, ValueT, HashT, KeyEqT, AllocatorT> m_set;

	public:
		void clear();
		void resize(size_t count = 0);

		ValueT& operator[](const KeyT& index);

		ValueT* find(const KeyT& index);
		bool remove(const KeyT& index);

		uintptr_t size();

		AllocatorT& getAllocator();
		const AllocatorT& getAllocator() const;
	};

	template<
		typename KeyT,
		typename ValueT,
		typename HashT = Hash<KeyT>,
		typename KeyEqT = EqualTo<KeyT>,
		typename AllocatorT = set_default_allocator<KeyT, ValueT>
	>
	class map_enum
	{
	private:
		set_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT> m_Set_Enum;

	public:

		map_enum();
		map_enum(map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>& map);

		bool	operator=(map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>& map);

		KeyT		*NextKey();
		ValueT	*NextValue();
		KeyT		*CurrentKey();
		ValueT	*CurrentValue();
	};

	template<typename KeyT, typename ValueT>
	Entry<KeyT, ValueT>::Entry(const KeyT& inKey)
		: key(inKey)
	{
		next = nullptr;
	}

	template<typename KeyT, typename ValueT>
	Entry<KeyT, ValueT>::Entry(const KeyT& inKey, const ValueT& inValue)
		: key(inKey)
		, value(inValue)
	{
		next = nullptr;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::set()
	{
		tableLength = 1;
		table = &defaultEntry;

		threshold = 1;
		count = 0;
		tableLengthIndex = 0;

		defaultEntry = nullptr;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::~set()
	{
		clear();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	size_t set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::countEntryBytes(size_t count)
	{
		if (count > 1) {
			return (sizeof(Entry<KeyT, ValueT>) + sizeof(Entry<KeyT, ValueT>*)) * count;
		}
		else if (count == 1) {
			return (sizeof(Entry<KeyT, ValueT>)) * count;
		}

		return 0;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	AllocatorT& set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::getAllocator()
	{
		return Entry_allocator;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	const AllocatorT& set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::getAllocator() const
	{
		return Entry_allocator;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::clear()
	{
		for (uintptr_t i = 0; i < tableLength; i++)
		{
			Entry<KeyT, ValueT>* next;
			for (Entry<KeyT, ValueT>* entry = table[i]; entry; entry = next)
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
	void set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::resize(size_t newCount)
	{
		if (newCount <= 1) {
			return;
		}

		Entry<KeyT, ValueT>** const oldTable = table;
		const size_t oldTableLength = tableLength;

		tableLength = newCount;
		threshold = tableLength;
		//threshold = (uint32_t)((float)tableLength * 0.75);

		// allocate a new table
		//table = new Entry<KeyT, ValueT> *[tableLength]();
		table = new(Entry_allocator.AllocTable(sizeof(Entry<KeyT, ValueT>*) * tableLength)) Entry<KeyT, ValueT> *[tableLength]();

		// rehash the table
		for (uintptr_t i = std::min(oldTableLength, newCount); i > 0; i--)
		{
			// rehash all entries from the old table
			Entry<KeyT, ValueT>* old;
			for (Entry<KeyT, ValueT>* e = oldTable[i - 1]; e != nullptr; e = old)
			{
				old = e->Next();

				// insert the old entry to the table hashindex
				const uintptr_t index = HashT()(e->Key()) % tableLength;

				e->SetNext(table[index]);
				table[index] = e;
			}
		}

		if (oldTableLength > 1)
		{
			// delete the previous table
			Entry_allocator.FreeTable(oldTable);
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::rehash()
	{
		size_t newLen = 0;
		for (size_t i = 0; i < sizeof(set_primes) / sizeof(set_primes[0]); ++i)
		{
			newLen = set_primes[i];
			if (newLen > tableLength)
			{
				tableLengthIndex = (uint16_t)i;
				break;
			}
		}

		resize(newLen);
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::shrink()
	{
		if (count) {
			resize(count);
		} else {
			clear();
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	const ValueT* set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyValue(const KeyT& key) const
	{
		const Entry<KeyT, ValueT>* entry = findKeyEntry(key);

		if (entry) {
			return &entry->Value();
		}
		else {
			return nullptr;
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT> *set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyEntry(const KeyT& key)
	{
		Entry<KeyT, ValueT> *entry = table[HashT()(key) % tableLength];

		for (; entry; entry = entry->Next())
		{
			if (KeyEqT()(entry->Key(), key)) {
				return entry;
			}
		}

		return nullptr;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	const Entry<KeyT, ValueT>* set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyEntry(const KeyT& key) const
	{
		const Entry<KeyT, ValueT>* entry = table[HashT()(key) % tableLength];

		for (; entry; entry = entry->Next())
		{
			if (KeyEqT()(entry->Key(), key)) {
				return entry;
			}
		}

		return nullptr;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT> *set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyEntry(const KeyT& key)
	{
		const uintptr_t index = HashT()(key) % tableLength;
		for (Entry<KeyT, ValueT>* entry = table[index]; entry; entry = entry->Next())
		{
			if (KeyEqT()(entry->Key(), key)) {
				return entry;
			}
		}

		return addNewKeyEntry(key, index);
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT>* set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyEntry(const KeyT& key, const ValueT& initVal)
	{
		const uintptr_t index = HashT()(key) % tableLength;
		Entry<KeyT, ValueT>* entry;
		for (entry = table[index]; entry; entry = entry->Next())
		{
			if (KeyEqT()(entry->Key(), key)) {
				return entry;
			}
		}

		return (entry = addNewKeyEntry(key, initVal, index));
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT> *set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addNewKeyEntry(const KeyT& key, uintptr_t index)
	{
		index = increment(key, index);

		Entry<KeyT, ValueT>* const entry = NewEntry(key);
		insertEntry(*entry, index);

		return entry;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT>* set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addNewKeyEntry(const KeyT& key, const ValueT& initVal, uintptr_t index)
	{
		index = increment(key, index);

		Entry<KeyT, ValueT>* const entry = NewEntry(key, initVal);
		insertEntry(*entry, index);

		return entry;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::insertEntry(Entry<KeyT, ValueT>& entry, uintptr_t index)
	{
		if (defaultEntry == nullptr)
		{
			defaultEntry = &entry;
			entry.SetNext(nullptr);
		}
		else {
			entry.SetNext(table[index]);
		}

		table[index] = &entry;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	uintptr_t set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::increment(const KeyT& key, uintptr_t index)
	{
		if (count >= threshold)
		{
			rehash();
			index = HashT()(key) % tableLength;
		}

		count++;

		return index;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	bool set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::isEmpty()
	{
		return count == 0;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	bool set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::remove(const KeyT& key)
	{
		intptr_t index = HashT()(key) % tableLength;
		Entry<KeyT, ValueT> *prev = nullptr;
		Entry<KeyT, ValueT> *entry;

		for (entry = table[index]; entry; entry = entry->Next())
		{
			// just to make sure we're using the correct overloaded operator
			if (!KeyEqT()(entry->Key(), key))
			{
				prev = entry;
				continue;
			}

			if (defaultEntry == entry)
			{
				defaultEntry = prev;
			}

			if (prev)
			{
				prev->SetNext(entry->Next());
			}
			else
			{
				table[index] = entry->Next();
			}

			count--;
			DeleteEntry(entry);

			return true;
		}

		return false;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT *set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::findKeyValue(const KeyT& key)
	{
		Entry<KeyT, ValueT> *entry = findKeyEntry(key);

		if (entry) {
			return &entry->Value();
		}
		else {
			return nullptr;
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT& set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyValue(const KeyT& key)
	{
		Entry<KeyT, ValueT> *entry = addKeyEntry(key);

		return entry->Value();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT& set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::addKeyValue(const KeyT& key, const ValueT& initVal)
	{
		Entry<KeyT, ValueT>* entry = addKeyEntry(key, initVal);

		return entry->Value();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	size_t set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::size()
	{
		return count;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	size_t set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::allocated()
	{
		return tableLength;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	set_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::set_enum()
	{
		m_Set = nullptr;
		m_Index = 0;
		m_CurrentEntry = nullptr;
		m_NextEntry = nullptr;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	set_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::set_enum(set<KeyT, ValueT, HashT, KeyEqT, AllocatorT> &set)
	{
		*this = set;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	bool set_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::operator=(set<KeyT, ValueT, HashT, KeyEqT, AllocatorT> &set)
	{
		m_Set = &set;
		m_Index = m_Set->tableLength;
		m_CurrentEntry = nullptr;
		m_NextEntry = nullptr;

		return true;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT>	*set_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::CurrentElement()
	{
		return m_CurrentEntry;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	Entry<KeyT, ValueT>	*set_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::NextElement()
	{
		if (!m_NextEntry)
		{
			while (1)
			{
				if (!m_Index) {
					break;
				}

				m_Index--;
				m_NextEntry = m_Set->table[m_Index];

				if (m_NextEntry) {
					break;
				}
			}

			if (!m_NextEntry)
			{
				m_CurrentEntry = nullptr;
				return nullptr;
			}
		}

		m_CurrentEntry = m_NextEntry;
		m_NextEntry = m_NextEntry->Next();

		return m_CurrentEntry;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	AllocatorT& map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::getAllocator()
	{
		return m_set.getAllocator();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	const AllocatorT& map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::getAllocator() const
	{
		return m_set.getAllocator();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::clear()
	{
		m_set.clear();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::resize(size_t count)
	{
		m_set.resize(count);
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT& map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::operator[](const KeyT& index)
	{
		return m_set.addKeyValue(index);
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT* map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::find(const KeyT& index)
	{
		return m_set.findKeyValue(index);
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	bool map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::remove(const KeyT& index)
	{
		return m_set.remove(index);
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	uintptr_t map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::size(void)
	{
		return m_set.size();
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::map_enum()
	{
		m_Set_Enum.m_Set = nullptr;
		m_Set_Enum.m_Index = 0;
		m_Set_Enum.m_CurrentEntry = nullptr;
		m_Set_Enum.m_NextEntry = nullptr;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::map_enum(map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>& map)
	{
		*this = map;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	bool map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::operator=(map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>& map)
	{
		m_Set_Enum = map.m_set;

		return true;
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	KeyT *map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::CurrentKey(void)
	{
		Entry<KeyT, ValueT> *entry = m_Set_Enum.CurrentElement();

		if (entry)
		{
			return &entry->KeyT;
		}
		else
		{
			return nullptr;
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT *map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::CurrentValue(void)
	{
		Entry<KeyT, ValueT> *entry = m_Set_Enum.CurrentElement();

		if (entry)
		{
			return &entry->ValueT;
		}
		else
		{
			return nullptr;
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	KeyT *map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::NextKey()
	{
		Entry<KeyT, ValueT> *entry = m_Set_Enum.NextElement();

		if (entry)
		{
			return &entry->KeyT;
		}
		else
		{
			return nullptr;
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	ValueT *map_enum<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::NextValue()
	{
		Entry<KeyT, ValueT> *entry = m_Set_Enum.NextElement();

		if (entry)
		{
			return &entry->Value();
		}
		else
		{
			return nullptr;
		}
	}
	}
};
