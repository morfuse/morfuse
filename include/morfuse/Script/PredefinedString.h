#pragma once

#include "../Common/rawchar.h"
#include "../Common/Linklist.h"
#include "../Common/ConstStr.h"

#include <cstdint>

namespace mfuse
{
    class PredefinedString
    {
    public:
        mfuse_EXPORTS PredefinedString(const rawchar_t* value);
        mfuse_EXPORTS ~PredefinedString();

        /** Return the string value for this predefined string. */
        mfuse_EXPORTS const rawchar_t* GetString() const;

        /** Return the index that should be consistent with the dictionary table. */
        mfuse_EXPORTS const_str GetIndex() const;
        mfuse_EXPORTS operator const_str () const;

        mfuse_EXPORTS bool operator==(const PredefinedString& other) const;
        mfuse_EXPORTS friend bool operator==(const PredefinedString& left, const_str right);
        mfuse_EXPORTS friend bool operator==(const_str left, const PredefinedString& right);

    private:
        PredefinedString* next;
        PredefinedString* prev;
        const rawchar_t* stringValue;
        const_str index;

    public:
        using List = LinkedList<PredefinedString*, &PredefinedString::next, &PredefinedString::prev>;

        /** Return the list's iterator to iterate through strings. */
        static List::iterator GetList();

        /** Return the number of strings in list. */
        static size_t GetNumStrings();

    private:
        static List list;
        static size_t numStrings;
        static uintptr_t lastIndex;
    };

    namespace ConstStrings
    {
        mfuse_EXPORTS extern const PredefinedString Empty;
    }
}
