#pragma once

#include "../Global.h"
#include "rawchar.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace mfuse
{
	class Archiver;

	template<typename CharT>
	class strdata
	{
	public:
		strdata();
		~strdata();

		void AddRef();
		bool DelRef();
		CharT* data();
		const CharT* data() const;

	public:
		uintptr_t refcount;
		size_t alloced;
		size_t len;
	};

	template<typename CharT>
	class mfuse_EXPORTS base_str
	{
		friend class Archiver;

	public:
		~base_str();
		base_str();
		base_str(const CharT* text);
		base_str(const CharT* text, size_t len);
		base_str(const base_str& string);
		base_str(base_str&& string) noexcept;
		base_str(const base_str& string, size_t start, size_t end);
		explicit base_str(const CharT ch);
		explicit base_str(const float num);
		explicit base_str(const int num);
		explicit base_str(const unsigned int num);
		explicit base_str(const long num);
		explicit base_str(const unsigned long num);
		explicit base_str(const long long num);
		explicit base_str(const unsigned long long num);

		size_t length(void) const;
		const CharT* c_str(void) const;

		void append(const CharT* text);
		void append(const CharT c);
		void append(const base_str& text);
		void assign(const CharT* text, size_t sz);
		void resize(size_t len);
		void reserve(size_t len);
		void clear();

		CharT operator[](uintptr_t index) const;
		CharT& operator[](uintptr_t index);

		void operator=(const base_str& text);
		base_str& operator=(base_str&& text) noexcept;
		void operator=(const CharT* text);

		base_str operator+(const base_str& b);
		base_str operator+(const CharT* b);
		template<typename U> friend base_str<U> operator+(const U* a, const base_str<U>& b);

		base_str operator+(const float b);
		base_str operator+(const int b);
		base_str operator+(const unsigned b);
		base_str operator+(const bool b);
		base_str operator+(const CharT b);

		base_str& operator+=(const base_str& a);
		base_str& operator+=(const CharT* a);
		base_str& operator+=(const float a);
		base_str& operator+=(const CharT a);
		base_str& operator+=(const int a);
		base_str& operator+=(const unsigned a);
		base_str& operator+=(const bool a);

		base_str& operator-=(int c);
		base_str& operator--(int);

		bool operator==(const base_str& b) const;
		bool operator==(const CharT* b) const;
		template<typename U> friend bool operator==(const U* a, const base_str<U>& b);

		bool operator!=(const base_str& b) const;
		bool operator!=(const CharT* b) const;
		template<typename U> friend bool operator!=(const U* a, const base_str<U>& b);

		operator const CharT* () const;

		int icmpn(const CharT* text, size_t n) const;
		int icmpn(const base_str& text, size_t n) const;
		int icmp(const CharT* text) const;
		int icmp(const base_str& text) const;
		int cmpn(const CharT* text, size_t n) const;
		int cmpn(const base_str& text, size_t n) const;

		void tolower();
		void toupper();

		static CharT* tolower(CharT* s1);
		static CharT* toupper(CharT* s1);
		static CharT tolower(CharT c);
		static CharT toupper(CharT c);

		static bool isEmpty(const CharT* s);
		static void copy(CharT* dest, const CharT* s);
		static void copyn(CharT* dest, const CharT* s, size_t max);
		static CharT* cat(CharT* dest, const CharT* s);
		static CharT* catn(CharT* dest, const CharT* s, size_t max);
		static int icmpn(const CharT* s1, const CharT* s2, size_t n);
		static int icmp(const CharT* s1, const CharT* s2);
		static int cmpn(const CharT* s1, const CharT* s2, size_t n);
		static int cmp(const CharT* s1, const CharT* s2);
		static size_t len(const CharT* s);
		static size_t lenn(const CharT* s, size_t max);
		static const CharT* ifindchar(const CharT* s, CharT c);
		static const CharT* ifindcharn(const CharT* s, CharT c, size_t n);
		static const CharT* ifind(const CharT* s1, const CharT* s2);
		static const CharT* ifindn(const CharT* s1, const CharT* s2, size_t n);
		static const CharT* findchar(const CharT* s, CharT c);
		static const CharT* findcharn(const CharT* s, CharT c, size_t n);
		static const CharT* find(const CharT* s1, const CharT* s2);
		static const CharT* findn(const CharT* s1, const CharT* s2, size_t n);
		static void reverse(CharT* s);
		static size_t i32toStr(int32_t num, CharT* output, size_t len, uintptr_t base = 10);
		static size_t ui32toStr(uint32_t num, CharT* output, size_t len, uintptr_t base = 10);
		static size_t i64toStr(int64_t num, CharT* output, size_t len, uintptr_t base = 10);
		static size_t ui64toStr(uint64_t num, CharT* output, size_t len, uintptr_t base = 10);
		static size_t floattoStr(float num, CharT* output, size_t len, uintptr_t precision = 6);

		void strip();

		static bool isNumeric(const CharT* base_str);
		bool isNumeric() const;
		bool isEmpty() const;

		void CapLength(size_t newlen);

		void BackSlashesToSlashes();
		void SlashesToBackSlashes();
		void DefaultExtension(const CharT* extension);
		const CharT* GetExtension() const;
		void StripExtension();
		void SkipFile();
		void SkipPath();

	public:
		static const base_str& getEmpty();

	private:
		strdata<CharT> *m_data;
		void EnsureAlloced(size_t, bool keepold = true);
		void EnsureDataWritable();

		static const base_str empty;
	};

	using str = base_str<char>;
	using wstr = base_str<wchar_t>;
	using u16str = base_str<char16_t>;
	using u32str = base_str<char32_t>;

	template<typename CharT>
	class mfuse_EXPORTS base_strview
	{
	public:
		base_strview();
		base_strview(const CharT* charArray, size_t length = -1);
		base_strview(const base_str<CharT>& string);

		const CharT* c_str() const;
		size_t len() const;
		bool isDynamic() const;
		const base_str<CharT>& getDynamicString() const;

	private:
		union {
			const CharT* charArray;
			const base_str<CharT>* string;
		};
		size_t length;
		bool isCharArray;
	};
	using strview = base_strview<char>;
	using wstrview = base_strview<wchar_t>;
	using u16strview = base_strview<char16_t>;
	using u32strview = base_strview<char32_t>;

	template<typename CharT>
	class mfuse_EXPORTS base_const_str_static
	{
	public:
		base_const_str_static(const CharT* stringValue);

		bool operator==(const CharT* other) const;
		bool operator!=(const CharT* other) const;

		const CharT* c_str() const;

		operator const CharT*() const;

	private:
		const CharT* string;
	};
	using const_str_static = base_const_str_static<char>;
	using const_wstr_static = base_const_str_static<wchar_t>;
	using const_u16strview = base_const_str_static<char16_t>;
	using const_u32strview = base_const_str_static<char32_t>;

	template<typename T>
	void Archive(Archiver& arc, base_str<T>& s);
};
