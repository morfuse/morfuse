#include <morfuse/Common/StringDictionary.h>

using namespace mfuse;

StringDictionary::StringDictionary()
{
}

StringDictionary::~StringDictionary()
{
}

const_str StringDictionary::Add(xstrview s)
{
	if (s.isDynamic()) {
		return (const_str)stringDict.addKeyIndex(s.getDynamicString());
	}
	else {
		return stringDict.addKeyIndex(xstr(s.c_str(), s.len()));
	}
}

const_str StringDictionary::Get(const rawchar_t* s) const
{
	const_str cs = (const_str)stringDict.findKeyIndex(s);
	return cs ? cs : STRING_EMPTY;
}

const xstr& StringDictionary::Get(const_str s) const
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
