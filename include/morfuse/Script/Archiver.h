#pragma once

#include "../Container/set.h"
#include "../Common/str.h"
#include <cstdint>

namespace mfuse
{
	class SafePtrBase;

	class Archiver
	{
	public:
		virtual ~Archiver();

		void ArchiveInt8(int8_t& num);
		void ArchiveInt16(int16_t& num);
		void ArchiveInt32(int32_t& num);
		void ArchiveInt64(int64_t& unum);
		void ArchiveUInt8(uint8_t& num);
		void ArchiveUInt16(uint16_t& num);
		void ArchiveUInt32(uint32_t& num);
		void ArchiveUInt64(uint64_t& num);
		void ArchiveByte(uint8_t& num);
		void ArchiveFloat(float& num);
		void ArchiveDouble(double& num);
		void ArchiveBoolean(bool& boolean);
		void ArchiveString(str& string);
		void ArchiveObjectPointer(const void*& ptr);
		void ArchiveObjectPosition(void* obj);
		void ArchiveSafePointer(SafePtrBase& ptr);
		void ArchiveBool(bool& boolean);
		void ArchivePosition(uintptr_t& pos);
		void ArchiveObject(Class* obj);

		void ArchiveRaw(void* data, size_t size);
	};

#if 0
	template<class Type, class Allocator>
	inline void con::Container<Type, Allocator>::Archive(Archiver& arc, void(*ArchiveFunc)(Archiver& arc, Type *obj))
	{
		/*
		int num;
		int i;

		if (arc.Loading())
		{
			arc.ArchiveInteger(&num);
			Resize(num);
		}
		else
		{
			num = numobjects;
			arc.ArchiveInteger(&num);
		}

		for (i = 1; i <= num; i++)
		{
			if (num > numobjects) {
				numobjects = num;
			}

			ArchiveFunc(arc, &objlist[i]);
		}
		*/
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void con::set<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::Archive(Archiver& arc)
	{
		/*
		Entry< key, value > *e;
		int hash;
		int i;

		arc.ArchiveUnsigned(&tableLength);
		arc.ArchiveUnsigned(&threshold);
		arc.ArchiveUnsigned(&count);
		arc.ArchiveUnsignedShort(&tableLengthIndex);

		if (arc.Loading())
		{
			if (tableLength != 1)
			{
				table = new Entry< key, value > *[tableLength]();
				memset(table, 0, tableLength * sizeof(Entry< key, value > *));
			}

			for (i = 0; i < count; i++)
			{
				e = new Entry< key, value >;
				e->Archive(arc);

				hash = HashCode< key >(e->key) % tableLength;

				e->index = i;
				e->next = table[hash];
				table[hash] = e;
			}
		}
		else
		{
			for (i = tableLength - 1; i >= 0; i--)
			{
				for (e = table[i]; e != NULL; e = e->next)
				{
					e->Archive(arc);
				}
			}
		}
		*/
	}

	template<typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT>
	void con::map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>::Archive(Archiver& arc)
	{
		m_con_set.Archive(arc);
	}
#endif
}
