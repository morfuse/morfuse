#pragma once

#include "set.h"

#include "../Script/Archiver.h"

namespace mfuse
{
	namespace con
	{
	template<typename KeyT, typename ValueT>
	void Archive(Archiver&, Entry<KeyT, ValueT>&)
	{
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::Archive(Archiver& arc)
	{
		if (arc.Loading())
		{
			clear();

			uint32_t tableLength32;
			uint32_t threshold32;
			uint32_t count32;
			arc.ArchiveUInt32(tableLength32);
			arc.ArchiveUInt32(threshold32);
			arc.ArchiveUInt32(count32);

			tableLength = tableLength32;
			threshold = threshold32;
			count = count32;
		}
		else
		{
			uint32_t tableLength32 = (uint32_t)tableLength;
			uint32_t threshold32 = (uint32_t)threshold;
			uint32_t count32 = (uint32_t)count;
			arc.ArchiveUInt32(tableLength32);
			arc.ArchiveUInt32(threshold32);
			arc.ArchiveUInt32(count32);
		}

		arc.ArchiveUInt16(tableLengthIndex);

		if (arc.IsReading())
		{
			if (tableLength != 1)
			{
				table = new(Entry_allocator.AllocTable(sizeof(Entry<KeyT, ValueT>*) * tableLength)) Entry<KeyT, ValueT> *[tableLength]();
				memset(table, 0, tableLength * sizeof(Entry<KeyT, ValueT> *));
			}

			for (uintptr_t i = 0; i < count; i++)
			{
				Entry<KeyT, ValueT>* const e = NewEntry();
				con::Archive(arc, *e);

				const uintptr_t hash = HashT()(e->Key()) % tableLength;

				e->SetNext(table[hash]);
				table[hash] = e;
			}
		}
		else
		{
			for (uintptr_t i = tableLength; i > 0; i--)
			{
				for (Entry<KeyT, ValueT>* e = table[i - 1]; e; e = e->Next())
				{
					con::Archive(arc, *e);
				}
			}
		}
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::Archive(Archiver& arc)
	{
		m_set.Archive(arc);
	}
	}
}
