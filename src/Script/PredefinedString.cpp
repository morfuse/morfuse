#include <morfuse/Script/PredefinedString.h>

using namespace mfuse;

namespace mfuse::ConstStrings
{
	const PredefinedString Empty("");
}

PredefinedString::List PredefinedString::list = PredefinedString::List();
size_t PredefinedString::numStrings = 0;
uintptr_t PredefinedString::lastIndex = 0;

PredefinedString::PredefinedString(const rawchar_t* value)
	: stringValue(value)
	, index(++lastIndex)
{
	list.Add(this);
	++numStrings;
}

PredefinedString::~PredefinedString()
{
	list.Remove(this);
	--numStrings;
}

const mfuse::rawchar_t* PredefinedString::GetString() const
{
	return stringValue;
}

const_str PredefinedString::GetIndex() const
{
	return index;
}

size_t PredefinedString::GetNumStrings()
{
	return numStrings;
}

bool PredefinedString::operator==(const PredefinedString& other) const
{
	return GetIndex() == other.GetIndex();
}

bool mfuse::operator==(const PredefinedString& left, const_str right)
{
	return left.GetIndex() == (const_str)right;
}

bool mfuse::operator==(const_str left, const PredefinedString& right)
{
	return (const_str)left == right.GetIndex();
}

PredefinedString::List::iterator PredefinedString::GetList()
{
	return list.CreateConstIterator();
}

PredefinedString::operator const_str() const
{
	return GetIndex();
}
