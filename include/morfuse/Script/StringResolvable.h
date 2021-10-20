#pragma once

#include "../Global.h"
#include "../Common/rawchar.h"
#include "../Common/str.h"
#include "../Common/ConstStr.h"

namespace mfuse
{
	class StringDictionary;

	class mfuse_EXPORTS StringResolvable
	{
	public:
		StringResolvable();
		StringResolvable(const rawchar_t* labelName);
		StringResolvable(const xstr& labelName);
		StringResolvable(const_str labelName);
		StringResolvable(const StringResolvable& other);
		StringResolvable& operator=(const StringResolvable& other);
		StringResolvable(StringResolvable&& other);
		StringResolvable& operator=(StringResolvable&& other);
		~StringResolvable();

		const xstr& GetString();
		const xstr& GetString() const;
		const xstr& GetString(const StringDictionary& dict);
		const xstr& GetString(const StringDictionary& dict) const;
		const rawchar_t* GetRawString();
		const rawchar_t* GetRawString() const;
		const rawchar_t* GetRawString(const StringDictionary& dict);
		const rawchar_t* GetRawString(const StringDictionary& dict) const;
		const_str GetConstString();
		const_str GetConstString() const;
		const_str GetConstString(StringDictionary& dict);
		const_str GetConstString(StringDictionary& dict) const;
		bool IsEmpty() const;

		operator const xstr& () const;
		operator const rawchar_t* () const;
		operator const_str () const;

		bool operator==(const StringResolvable& other) const;
		bool operator!=(const StringResolvable& other) const;
		mfuse_EXPORTS friend bool operator==(const StringResolvable& left, const_str right);
		mfuse_EXPORTS friend bool operator!=(const StringResolvable& left, const_str right);
		mfuse_EXPORTS friend bool operator==(const_str left, const StringResolvable& right);
		mfuse_EXPORTS friend bool operator!=(const_str left, const StringResolvable& right);
		mfuse_EXPORTS friend bool operator==(const StringResolvable& left, const rawchar_t* right);
		mfuse_EXPORTS friend bool operator!=(const StringResolvable& left, const rawchar_t* right);
		mfuse_EXPORTS friend bool operator==(const rawchar_t* left, const StringResolvable& right);
		mfuse_EXPORTS friend bool operator!=(const rawchar_t* left, const StringResolvable& right);

	private:
		const xstr& GetStringInternal(const StringDictionary& dict);
		const xstr& GetStringInternal(const StringDictionary& dict) const;
		const rawchar_t* GetRawStringInternal(const StringDictionary& dict);
		const rawchar_t* GetRawStringInternal(const StringDictionary& dict) const;
		const_str GetConstStringInternal(StringDictionary& dict);
		const_str GetConstStringInternal(StringDictionary& dict) const;

	private:
		xstr string;
		const_str constString;
	};
}
