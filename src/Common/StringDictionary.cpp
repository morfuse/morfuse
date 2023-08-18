#include <morfuse/Common/StringDictionary.h>
#include <morfuse/Script/Archiver.h>

using namespace mfuse;

StringDictionary::StringDictionary()
{
}

StringDictionary::~StringDictionary()
{
}

const_str StringDictionary::Add(strview s)
{
	if (s.isDynamic()) {
		return (const_str)stringDict.addKeyIndex(s.getDynamicString());
	}
	else {
		return stringDict.addKeyIndex(str(s.c_str(), s.len()));
	}
}

const_str StringDictionary::Get(const rawchar_t* s) const
{
	const_str cs = (const_str)stringDict.findKeyIndex(s);
	return cs ? cs : const_str_e::NONE;
}

const str& StringDictionary::Get(const_str s) const
{
	return stringDict[s];
}

void StringDictionary::AllocateMoreString(size_t count)
{
	if (stringDict.size() + count > stringDict.allocated()) {
		stringDict.resize(stringDict.size() + count);
	}
}

void StringDictionary::Reset()
{
	stringDict.clear();
}

void StringDictionary::ArchiveString(Archiver& arc, const_str& constStringValue)
{
	if (arc.Loading())
	{
		uint8_t hasString;
		arc.ArchiveByte(hasString);

		if (hasString)
		{
			str value;
			::Archive(arc, value);

			constStringValue = Add(value);
		}
		else
		{
			constStringValue = const_str(0);
		}
	}
	else
	{
		uint8_t hasString = constStringValue != 0u;
		arc.ArchiveByte(hasString);

		if (hasString)
		{
			::Archive(arc, stringDict[constStringValue]);
		}
	}
}
