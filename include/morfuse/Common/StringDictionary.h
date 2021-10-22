#pragma once

#include "../Global.h"
#include "rawchar.h"
#include "ConstStr.h"
#include "str.h"
#include "../Container/arrayset.h"
#include "MEM/BlockAlloc.h"
#include "MEM/DefaultAlloc.h"

namespace mfuse
{
	class StringDictionary
	{
	public:
		StringDictionary();
		~StringDictionary();

		mfuse_EXPORTS const_str Add(xstrview s);
		mfuse_EXPORTS const_str Get(const rawchar_t* s) const;
		mfuse_EXPORTS const xstr& Get(const_str s) const;
		mfuse_EXPORTS void AllocateMoreString(size_t count);
		mfuse_EXPORTS void Reset();

		mfuse_EXPORTS void ArchiveString(Archiver& arc, const_str& constStringValue);

	private:
		/** The string dictionary, used to cache strings into a number. */
		con::arrayset<xstr, xstr, Hash<xstr>, EqualTo<xstr>, MEM::DefaultAlloc_set> stringDict;
	};
}
