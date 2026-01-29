#include <morfuse/Script/StringResolvable.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/PredefinedString.h>

#include <utility>

using namespace mfuse;

StringResolvable::StringResolvable()
    : constString(0)
{
}

StringResolvable::StringResolvable(const rawchar_t* stringValue)
    : string(stringValue)
    , constString(0)
{
}

StringResolvable::StringResolvable(const str& stringValue)
    : string(stringValue)
    , constString(0)
{
}

StringResolvable::StringResolvable(const_str constStringValue)
    : constString(constStringValue)
{
}

StringResolvable::StringResolvable(const StringResolvable& other)
    : string(other.string)
    , constString(other.constString)
{
}

StringResolvable::StringResolvable(StringResolvable&& other)
    : string(std::move(other.string))
    , constString(other.constString)
{
}

StringResolvable& StringResolvable::operator=(const StringResolvable& other)
{
    string = other.string;
    constString = other.constString;
    return *this;
}

StringResolvable& StringResolvable::operator=(StringResolvable&& other)
{
    string = std::move(other.string);
    constString = other.constString;
    return *this;
}

StringResolvable::~StringResolvable()
{
}

const str& StringResolvable::GetString()
{
    if (!string.isEmpty()) {
        return string;
    }

    return GetStringInternal(ScriptContext::Get().GetDirector().GetDictionary());
}

const str& StringResolvable::GetString() const
{
    if (!string.isEmpty()) {
        return string;
    }

    return GetStringInternal(ScriptContext::Get().GetDirector().GetDictionary());
}

const str& StringResolvable::GetString(const StringDictionary& dict)
{
    if (!string.isEmpty()) {
        return string;
    }

    return GetStringInternal(dict);
}

const str& StringResolvable::GetString(const StringDictionary& dict) const
{
    if (!string.isEmpty()) {
        return string;
    }

    return GetStringInternal(dict);
}

const rawchar_t* StringResolvable::GetRawString()
{
    return GetString().c_str();
}

const rawchar_t* StringResolvable::GetRawString() const
{
    return GetString().c_str();
}

const mfuse::rawchar_t* StringResolvable::GetRawString(const StringDictionary& dict)
{
    return GetString(dict).c_str();
}

const mfuse::rawchar_t* StringResolvable::GetRawString(const StringDictionary& dict) const
{
    return GetString(dict).c_str();
}

const_str StringResolvable::GetConstString()
{
    if (constString) {
        return constString;
    }

    return GetConstString(ScriptContext::Get().GetDirector().GetDictionary());
}

const_str StringResolvable::GetConstString() const
{
    if (constString) {
        return constString;
    }

    return GetConstString(ScriptContext::Get().GetDirector().GetDictionary());
}

const_str StringResolvable::GetConstString(StringDictionary& dict)
{
    if (constString)
    {
        // cached
        return constString;
    }

    return GetConstStringInternal(dict);
}

const_str StringResolvable::GetConstString(StringDictionary& dict) const
{
    if (constString)
    {
        // cached
        return constString;
    }

    return GetConstStringInternal(dict);
}

const str& StringResolvable::GetStringInternal(const StringDictionary& dict)
{
    // cache the string
    if (constString) {
        string = dict.Get(constString);
    }

    return string;
}

const str& StringResolvable::GetStringInternal(const StringDictionary& dict) const
{
    if (constString) {
        return dict.Get(constString);
    }

    return str::getEmpty();
}

const rawchar_t* StringResolvable::GetRawStringInternal(const StringDictionary& dict)
{
    return GetStringInternal(dict).c_str();
}

const rawchar_t* StringResolvable::GetRawStringInternal(const StringDictionary& dict) const
{
    return GetStringInternal(dict).c_str();
}

const_str StringResolvable::GetConstStringInternal(StringDictionary& dict)
{
    if (!string.isEmpty())
    {
        // cache the const string
        constString = dict.Add(string);
    }
    else
    {
        // use the default
        constString = ConstStrings::Empty;
    }

    return constString;
}

const_str StringResolvable::GetConstStringInternal(StringDictionary& dict) const
{
    if (!string.isEmpty())
    {
        // cache the const string
        return dict.Add(string);
    }

    return const_str(0);
}

bool StringResolvable::IsEmpty() const
{
    return !constString && string.isEmpty();
}

StringResolvable::operator const str& () const
{
    return GetString();
}

StringResolvable::operator const rawchar_t* () const
{
    return GetRawString();
}

StringResolvable::operator const_str() const
{
    return GetConstString();
}

bool StringResolvable::operator==(const StringResolvable& other) const
{
    if (constString && other.constString)
    {
        // both have a const string so use it in priority
        return constString == other.constString;
    }
    else if (!constString && !other.constString)
    {
        // string match
        return string == other.string;
    }
    else if (constString && !other.constString)
    {
        if (string.length())
        {
            // fallback to the string to avoid allocation
            return string == other.string;
        }
        else
        {
            return GetString() == other.string;
        }
    }
    else if (!constString && other.constString)
    {
        if (other.string.length())
        {
            // fallback to the string to avoid allocation
            return string == other.string;
        }
        else
        {
            return string == other.GetString();
        }
    }

    return false;
}

bool StringResolvable::operator!=(const StringResolvable& other) const
{
    return !(*this == other);
}

bool mfuse::operator==(const StringResolvable& left, const_str right)
{
    if (left.IsEmpty())
    {
        // true if both empty
        return !right;
    }
    else if (!right)
    {
        return left.IsEmpty();
    }

    return left.GetConstString() == right;
}

bool mfuse::operator!=(const StringResolvable& left, const_str right)
{
    return !(left == right);
}

bool mfuse::operator==(const_str left, const StringResolvable& right)
{
    if (right.IsEmpty())
    {
        return !left;
    }
    else if (!left)
    {
        // true if both empty
        return right.IsEmpty();
    }
    

    return left == right.GetConstString();
}

bool mfuse::operator!=(const_str left, const StringResolvable& right)
{
    return !(left == right);
}

bool mfuse::operator==(const StringResolvable& left, const rawchar_t* right)
{
    if (left.IsEmpty())
    {
        // true if both empty
        return str::isEmpty(right);
    }
    else if (str::isEmpty(right))
    {
        return left.string.isEmpty();
    }

    return left.GetString() == right;
}

bool mfuse::operator!=(const StringResolvable& left, const rawchar_t* right)
{
    return !(left == right);
}

bool mfuse::operator==(const rawchar_t* left, const StringResolvable& right)
{
    if (str::isEmpty(left))
    {
        return right.string.isEmpty();
    }
    else if (right.IsEmpty())
    {
        // true if both empty
        return str::isEmpty(left);
    }

    return left == right.GetString();
}


bool mfuse::operator!=(const rawchar_t* left, const StringResolvable& right)
{
    return !(left == right);
}
