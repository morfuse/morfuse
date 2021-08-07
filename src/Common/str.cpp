#include <morfuse/Common/str.h>
#include <morfuse/Container/set_generic_hash.h>

#include <cmath>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <codecvt>
#include <utf8.h>

// VC compiler...
#include <utf8/cpp11.h>

using namespace mfuse;

#ifdef _WIN32
#pragma warning(disable : 4244)     // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4710)     // function 'blah' not inlined
#endif

static constexpr unsigned int STR_ALLOC_GRAN = 11;

template<typename CharT>
static constexpr CharT trueStr[] = { 't', 'r', 'u', 'e', 0 };

template<typename CharT>
static constexpr CharT falseStr[] = { 'f', 'a', 'l', 's', 'e', 0 };

template<typename CharT>
static constexpr CharT emptyStr[] = { 0 };

template<>
mfuse_EXPORTS intptr_t Hash<str>::operator()(const str& key) const
{
	return Hash<const char*>()(key.c_str());
}

template<>
mfuse_EXPORTS intptr_t Hash<const_str_static>::operator()(const const_str_static& key) const
{
	return Hash<const char*>()(key.c_str());
}

template<typename CharT, typename intType>
static size_t numtoStr(intType num, CharT* output, size_t len, uintptr_t base)
{
	if (len == 0) {
		return 0;
	}

	intType sum = num;
	uintptr_t i = 0;
	do
	{
		uint32_t digit = sum % base;

		if (digit < 0xA)
			output[i++] = '0' + digit;
		else
			output[i++] = 'A' + digit - 0xA;

		sum /= base;

	} while (sum && (i < (len - 1)));

	if (i == (len - 1) && sum)
		return 0;

	output[i] = '\0';
	base_str<CharT>::reverse(output);

	return i;
}

template<typename CharT>
size_t base_str<CharT>::length(void) const
{
	return (m_data != nullptr) ? m_data->len : 0;
}

template<typename CharT>
base_str<CharT>::~base_str()
{
	clear();
}

template<typename CharT>
base_str<CharT>::base_str()
	: m_data(nullptr)
{

}

template<typename CharT>
base_str<CharT>::base_str(base_str&& string) noexcept
	: m_data(string.m_data)
{
	string.m_data = nullptr;
}

template<typename CharT>
base_str<CharT>::base_str(const CharT* text)
	: m_data(nullptr)
{
	size_t len;

	assert(text);
	if (*text)
	{
		len = base_str::len(text);

		if (len)
		{
			EnsureAlloced(len + 1);
			copyn(m_data->data(), text, len);
			m_data->data()[len] = 0;
			m_data->len = len;
		}
	}
}

template<typename CharT>
base_str<CharT>::base_str(const CharT* text, size_t len)
	: m_data(nullptr)
{
	assert(text);
	if (*text && len)
	{
		EnsureAlloced(len + 1);
		copyn(m_data->data(), text, len);
		m_data->data()[len] = 0;
		m_data->len = len;
	}
}

template<typename CharT>
base_str<CharT>::base_str(const base_str& text)
{
	m_data = text.m_data;
	if (m_data) m_data->AddRef();
}

template<typename CharT>
base_str<CharT>::base_str(const base_str& text, size_t start, size_t end)
	: m_data(nullptr)
{
	if (end > text.length()) {
		end = text.length();
	}

	if (start > text.length()) {
		start = text.length();
	}

	const size_t len = end > start
		? (end - start)
		: 0;

	EnsureAlloced(len + 1);

	for (size_t i = 0; i < len; i++)
	{
		m_data->data()[i] = text[start + i];
	}

	m_data->data()[len] = 0;
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const CharT ch)
	: m_data(nullptr)
{
	EnsureAlloced(2);

	m_data->data()[0] = ch;
	m_data->data()[1] = 0;
	m_data->len = 1;
}

template<typename CharT>
base_str<CharT>::base_str(const float num)
	: m_data(nullptr)

{
	static constexpr CharT formatSpecifier[] = { '%', '.', '3', 'f', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	floattoStr(num, text, 32, 3);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const int num)
	: m_data(nullptr)
{
	static constexpr CharT formatSpecifier[] = { '%', 'd', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	i32toStr(num, text, 32);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const unsigned int num)
	: m_data(nullptr)

{
	static constexpr CharT formatSpecifier[] = { '%', 'u', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	ui32toStr(num, text, 32);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const long num)
	: m_data(nullptr)
{
	static constexpr CharT formatSpecifier[] = { '%', 'l', 'd', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	i64toStr(num, text, 32);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const unsigned long num)
	: m_data(nullptr)
{
	static constexpr CharT formatSpecifier[] = { '%', 'l', 'u', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	ui64toStr(num, text, 32);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const long long num)
	: m_data(nullptr)
{
	static constexpr CharT formatSpecifier[] = { '%', 'l', 'l', 'd', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	i64toStr(num, text, 32);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
base_str<CharT>::base_str(const unsigned long long num)
	: m_data(nullptr)
{
	static constexpr CharT formatSpecifier[] = { '%', 'l', 'l', 'u', 0 };

	CharT text[32];
	size_t len;

	//bprintf(text, formatSpecifier, num);
	ui64toStr(num, text, 32);
	len = base_str::len(text);
	EnsureAlloced(len + 1);
	copy(m_data->data(), text);
	m_data->len = len;
}

template<typename CharT>
const CharT* base_str<CharT>::c_str() const
{
	return m_data ? m_data->data() : emptyStr<CharT>;
}

template<typename CharT>
void base_str<CharT>::append(const CharT* text)
{
	size_t len;

	assert(text);

	if (text)
	{
		len = length();
		len += base_str::len(text);
		EnsureAlloced(len + 1);

		base_str::cat(m_data->data(), text);
		m_data->len = len;
	}
}

template<typename CharT>
void base_str<CharT>::append(const CharT c)
{
	if (c)
	{
		const size_t len = length();
		const size_t newLen = len + 1;
		EnsureAlloced(newLen + 1);

		CharT* data = m_data->data();
		data[len] = c;
		data[newLen] = 0;
		m_data->len = newLen;
	}
}

template<typename CharT>
void base_str<CharT>::append(const base_str& text)
{
	size_t len;

	len = length();
	len += text.length();
	EnsureAlloced(len + 1);

	base_str::cat(m_data->data(), text.c_str());
	m_data->len = len;
}

template<typename CharT>
CharT base_str<CharT>::operator[](intptr_t index) const
{
	assert(m_data);

	if (!m_data) {
		return 0;
	}

	// don't include the '/0' in the test, because technically, it's out of bounds
	//assert((index >= 0) && (index < m_data->len));

	// In release mode, give them a nullptr character
	// don't include the '/0' in the test, because technically, it's out of bounds
	if ((index < 0) || (index >= m_data->len)) {
		return 0;
	}

	return m_data->data()[index];
}

template<typename CharT>
CharT& base_str<CharT>::operator[](intptr_t index)
{
	// Used for result for invalid indices
	assert(m_data);

	// We don't know if they'll write to it or not
	// if it's not a const object
	EnsureDataWritable();

	/*
	if (!m_data) {
		return dummy;
	}
	*/

	// don't include the '/0' in the test, because technically, it's out of bounds
	//assert((index >= 0) && (index < (int)m_data->len));

	// In release mode, let them change a safe variable
	// don't include the '/0' in the test, because technically, it's out of bounds
	if ((index < 0) || (index >= m_data->len))
	{
		static CharT dummy;
		return dummy;
	}

	return m_data->data()[index];
}

template<typename CharT>
void base_str<CharT>::operator=(const base_str& text)
{
	// adding the reference before deleting our current reference prevents
	// us from deleting our string if we are copying from ourself
	if (text.m_data)
		text.m_data->AddRef();

	if (m_data)
		m_data->DelRef();

	m_data = text.m_data;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator=(base_str&& text) noexcept
{
	if (m_data) m_data->DelRef();

	m_data = text.m_data;
	text.m_data = nullptr;
	return *this;
}

template<typename CharT>
void base_str<CharT>::operator=(const CharT* text)
{
	size_t len;

	assert(text);

	if (m_data)
	{
		if (text == m_data->data())
		{
			// Copying same thing.  Punt.
			return;
		}

		m_data->DelRef();
		m_data = nullptr;
	}

	if (*text)
	{
		len = base_str::len(text);

		unsigned char* buf = new unsigned char[sizeof(strdata<CharT>) + sizeof(CharT) * (len + 1)];
		//m_data = new strdata<CharT>;
		m_data = new (buf) strdata<CharT>;
		m_data->len = len;
		m_data->alloced = len + 1;
		//m_data->data() = new CharT[len + 1];
		copy(m_data->data(), text);
	}
}

template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const base_str& b)
{
	base_str result(*this);

	result.append(b);

	return result;
}

template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const CharT* b)
{
	base_str result(*this);

	result.append(b);

	return result;
}

template<typename CharT>
base_str<CharT> mfuse::operator+(const CharT* a, const base_str<CharT>& b)
{
	base_str result(a);

	result.append(b);

	return result;
}


template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const bool b)
{
	base_str result(*this);

	result.append(b ? trueStr<CharT> : falseStr<CharT>);

	return result;
}

template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const CharT b)
{
	CharT text[2];

	text[0] = b;
	text[1] = 0;

	return *this + text;
}

template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const float b)
{
	static constexpr CharT formatSpecifier[] = { '%', 'f', 0 };

	CharT text[20];

	base_str result(*this);

	//bprintf(text, formatSpecifier, b);
	floattoStr(b, text, 20);
	result.append(text);

	return result;
}

template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const int b)
{
	static constexpr CharT formatSpecifier[] = { '%', 'd', 0 };

	CharT text[20];

	base_str result(*this);

	//bprintf(text, formatSpecifier, b);
	i32toStr(b, text, 20);
	result.append(text);

	return result;
}

template<typename CharT>
base_str<CharT> base_str<CharT>::operator+(const unsigned b)
{
	static constexpr CharT formatSpecifier[] = { '%', 'u', 0 };

	CharT text[20];

	base_str result(*this);

	//bprintf(text, formatSpecifier, b);
	ui32toStr(b, text, 20);
	result.append(text);

	return result;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const base_str<CharT>& a)
{
	append(a);
	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const CharT* a)
{
	append(a);
	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const CharT a)
{
	CharT text[2];

	text[0] = a;
	text[1] = 0;
	append(text);

	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const bool a)
{
	append(a ? trueStr<CharT> : falseStr<CharT>);
	return *this;
}

template<typename CharT>
void mfuse::base_str<CharT>::copy(CharT* dest, const CharT* s)
{
	while((*dest = *s))
	{
		++s;
		++dest;
	}
}

template<typename CharT>
void mfuse::base_str<CharT>::copyn(CharT* dest, const CharT* s, size_t max)
{
	size_t len = 0;
	while ((*dest = *s) && len < max)
	{
		++s;
		++dest;
		++len;
	}
}

template<typename CharT>
CharT* mfuse::base_str<CharT>::cat(CharT* dest, const CharT* s)
{
	copy(dest + base_str::len(dest), s);
	return dest;
}

template<typename CharT>
CharT* mfuse::base_str<CharT>::catn(CharT* dest, const CharT* s, size_t max)
{
	CharT* old = dest;
	dest += base_str::len(dest);

	const size_t srcLen = base_str::lenn(s, max);

	dest[srcLen] = 0;
	memcpy(dest, s, srcLen);

	return old;
}

template<typename CharT>
int base_str<CharT>::icmpn(const CharT* text, size_t n) const
{
	assert(m_data);
	assert(text);

	return base_str<CharT>::icmpn(m_data->data(), text, n);
}

template<typename CharT>
int base_str<CharT>::icmpn(const base_str<CharT>& text, size_t n) const
{
	assert(m_data);
	assert(text.m_data);

	return base_str<CharT>::icmpn(m_data->data(), text.m_data->data(), n);
}

template<typename CharT>
int base_str<CharT>::icmp(const CharT* text) const
{
	assert(m_data);
	assert(text);

	return base_str<CharT>::icmp(m_data->data(), text);
}

template<typename CharT>
int base_str<CharT>::icmp(const base_str<CharT>& text) const
{
	assert(c_str());
	assert(text.c_str());

	return base_str<CharT>::icmp(c_str(), text.c_str());
}

template<typename CharT>
int base_str<CharT>::cmpn(const CharT* text, size_t n) const
{
	assert(c_str());
	assert(text);

	return base_str<CharT>::cmpn(c_str(), text, n);
}

template<typename CharT>
int base_str<CharT>::cmpn(const base_str<CharT>& text, size_t n) const
{
	assert(c_str());
	assert(text.c_str());

	return base_str<CharT>::cmpn(c_str(), text.c_str(), n);
}

template<typename CharT>
void base_str<CharT>::tolower()
{
	assert(m_data);

	EnsureDataWritable();

	base_str<CharT>::tolower(m_data->data());
}

template<typename CharT>
void base_str<CharT>::toupper()
{
	assert(m_data);

	EnsureDataWritable();

	base_str<CharT>::toupper(m_data->data());
}

template<typename CharT>
bool base_str<CharT>::isNumeric() const

{
	assert(m_data);

	return base_str<CharT>::isNumeric(m_data->data());
}

template<typename CharT>
base_str<CharT>::operator const CharT* () const
{
	return c_str();
}

template<typename CharT>
CharT* base_str<CharT>::tolower(CharT* s1)
{
	CharT* s;

	s = s1;
	while (*s)
	{
		*s = tolower(*s);
		s++;
	}

	return s1;
}

template<typename CharT>
CharT* base_str<CharT>::toupper(CharT* s1)
{
	CharT* s;

	s = s1;
	while (*s)
	{
		*s = toupper(*s);
		s++;
	}

	return s1;
}

template<typename CharT>
CharT base_str<CharT>::tolower(CharT c)
{
	return ::tolower(c);
}

template<typename CharT>
CharT base_str<CharT>::toupper(CharT c)
{
	return ::toupper(c);
}

template<typename CharT>
int base_str<CharT>::icmpn(const CharT* s1, const CharT* s2, size_t n)
{
	CharT c1;
	CharT c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
		{
			// strings are equal until end point
			return 0;
		}

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
			{
				c1 -= ('a' - 'A');
			}

			if (c2 >= 'a' && c2 <= 'z')
			{
				c2 -= ('a' - 'A');
			}

			if (c1 < c2)
			{
				// strings less than
				return -1;
			}
			else if (c1 > c2)
			{
				// strings greater than
				return 1;
			}
		}
	} while (c1);

	// strings are equal
	return 0;
}

template<typename CharT>
int base_str<CharT>::icmp(const CharT* s1, const CharT* s2)
{
	int c1;
	int c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (c1 != c2)
		{
			if (c1 >= 'a' && c1 <= 'z')
			{
				c1 -= ('a' - 'A');
			}

			if (c2 >= 'a' && c2 <= 'z')
			{
				c2 -= ('a' - 'A');
			}

			if (c1 < c2)
			{
				// strings less than
				return -1;
			}
			else if (c1 > c2)
			{
				// strings greater than
				return 1;
			}
		}
	} while (c1);

	// strings are equal
	return 0;
}

template<typename CharT>
int base_str<CharT>::cmpn(const CharT* s1, const CharT* s2, size_t n)
{
	int c1;
	int c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
		{
			// strings are equal until end point
			return 0;
		}

		if (c1 < c2)
		{
			// strings less than
			return -1;
		}
		else if (c1 > c2)
		{
			// strings greater than
			return 1;
		}
	} while (c1);

	// strings are equal
	return 0;
}

template<typename CharT>
int base_str<CharT>::cmp(const CharT* s1, const CharT* s2)
{
	int c1;
	int c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (c1 < c2)
		{
			// strings less than
			return -1;
		}
		else if (c1 > c2)
		{
			// strings greater than
			return 1;
		}
	} while (c1);

	// strings are equal
	return 0;
}

template<typename CharT>
size_t base_str<CharT>::len(const CharT* s)
{
	size_t l = 0;
	while (*s++) ++l;
	return l;
}

template<typename CharT>
size_t mfuse::base_str<CharT>::lenn(const CharT* s, size_t max)
{
	size_t l = 0;
	while (*s++ && l < max) ++l;
	return l;
}

template<typename CharT>
const CharT* base_str<CharT>::ifindchar(const CharT* s, CharT c)
{
	do {
		if (tolower(*s) == tolower(c)) {
			return s;
		}
	} while (*s++);

	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::ifindcharn(const CharT* s, CharT c, size_t n)
{
	for (size_t i = 0; i < n && *s; ++i, ++s)
	{
		if (tolower(*s) == tolower(c)) {
			return s;
		}
	}
	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::ifind(const CharT* s1, const CharT* s2)
{
	const size_t len = base_str<CharT>::len(s2);
	for (const CharT* p = s1; (p = ifindchar(p, *s2)); ++p)
	{
		if (icmpn(p, s2, len) == 0) {
			return p;
		}
	}

	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::ifindn(const CharT* s1, const CharT* s2, size_t n)
{
	const size_t len = base_str<CharT>::len(s2);
	for (const CharT* p = s1; (p = ifindchar(p, *s2)) && size_t(p - s1) < n; ++p)
	{
		if (icmpn(p, s2, len) == 0) {
			return p;
		}
	}

	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::findchar(const CharT* s, CharT c)
{
	do {
		if (*s == c) {
			return s;
		}
	} while (*s++);

	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::findcharn(const CharT* s, CharT c, size_t n)
{
	for(size_t i = 0; i < n && *s; ++i, ++s)
	{
		if(*s == c) {
			return s;
		}
	}
	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::find(const CharT* s1, const CharT* s2)
{
	const size_t len = base_str<CharT>::len(s2);
	for (const CharT* p = s1; (p = findchar(p, *s2)); ++p)
	{
		if (cmpn(p, s2, len) == 0) {
			return p;
		}
	}

	return 0;
}

template<typename CharT>
const CharT* base_str<CharT>::findn(const CharT* s1, const CharT* s2, size_t n)
{
	const size_t len = base_str<CharT>::len(s2);
	for (const CharT* p = s1; (p = findchar(p, *s2)) && size_t(p - s1) < n; ++p)
	{
		if (cmpn(p, s2, len) == 0) {
			return p;
		}
	}

	return 0;
}

template<typename CharT>
void base_str<CharT>::reverse(CharT* s)
{
	size_t l = len(s);

	uintptr_t i, j;
	for (i = 0, j = l - 1; i < j; i++, j--)
	{
		CharT a = s[i];
		s[i] = s[j];
		s[j] = a;
	}
}

template<typename CharT>
size_t base_str<CharT>::i32toStr(int32_t num, CharT* output, size_t len, uintptr_t base)
{
	return numtoStr(num, output, len, base);
}

template<typename CharT>
size_t mfuse::base_str<CharT>::ui32toStr(uint32_t num, CharT* output, size_t len, uintptr_t base)
{
	return numtoStr(num, output, len, base);
}

template<typename CharT>
size_t base_str<CharT>::i64toStr(int64_t num, CharT* output, size_t len, uintptr_t base)
{
	return numtoStr(num, output, len, base);
}

template<typename CharT>
size_t mfuse::base_str<CharT>::ui64toStr(uint64_t num, CharT* output, size_t len, uintptr_t base)
{
	return numtoStr(num, output, len, base);
}

template<typename CharT>
size_t mfuse::base_str<CharT>::floattoStr(float num, CharT* output, size_t len, uintptr_t precision)
{
	int32_t ipart = (int32_t)num;
	float fpart = num - (float)ipart;
	size_t totalDigits = numtoStr(ipart, output, len, 10);

	if (precision != 0)
	{
		// decimals
		output[totalDigits] = '.';

		fpart = fpart * pow(10, precision);

		const size_t digits = numtoStr((int32_t)fpart, output + totalDigits + 1, len, 10);

		totalDigits += digits;

		uintptr_t k;
		for(k = digits; k < precision; ++k) {
			output[k] = '0';
		}
		output[totalDigits + 1] = 0;
	}

	return totalDigits;
}

/*
============
IsNumeric

Checks a string to see if it contains only numerical values.
============
*/
template<typename CharT>
bool base_str<CharT>::isNumeric(const CharT* strValue)
{
	size_t len;
	size_t i;
	bool dot;

	if (*strValue == '-')
	{
		strValue++;
	}

	dot = false;
	len = base_str::len(strValue);
	for (i = 0; i < len; i++)
	{
		if (!isdigit(strValue[i]))
		{
			if ((strValue[i] == '.') && !dot)
			{
				dot = true;
				continue;
			}
			return false;
		}
	}

	return true;
}

template<typename CharT>
bool base_str<CharT>::isEmpty() const
{
	return !m_data || !m_data->len || !*m_data->data();
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const float a)
{
	static constexpr CharT formatSpecifier[] = { '%', 'f', 0 };

	CharT text[20];

	//bprintf(text, formatSpecifier, a);
	floattoStr(a, text, 20);
	append(text);

	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const int a)
{
	static constexpr CharT formatSpecifier[] = { '%', 'd', 0 };

	CharT text[20];

	//bprintf(text, formatSpecifier, a);
	i32toStr(a, text, 20);
	append(text);

	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator+=(const unsigned a)
{
	static constexpr CharT formatSpecifier[] = { '%', 'u', 0 };

	CharT text[20];

	//bprintf(text, formatSpecifier, a);
	ui32toStr(a, text, 20);
	append(text);

	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator-=(int c)
{
	if (!m_data)
	{
		return *this;
	}

	if (!m_data->len)
	{
		return *this;
	}

	m_data->len -= c;
	if (m_data->len < 0)
	{
		m_data->len = 0;
	}

	EnsureDataWritable();

	m_data->data()[m_data->len] = 0;

	return *this;
}

template<typename CharT>
base_str<CharT>& base_str<CharT>::operator--(int)
{
	*this -= 1;
	return *this;
}

template<typename CharT>
void base_str<CharT>::CapLength(size_t newlen)
{
	assert(m_data);

	if (length() <= newlen)
		return;

	EnsureDataWritable();

	m_data->data()[newlen] = 0;
	m_data->len = newlen;
}

template<typename CharT>
void base_str<CharT>::EnsureDataWritable()
{
	strdata<CharT>* olddata;
	size_t len;

	if (!m_data) {
		return;
	}

	if (!m_data->refcount) {
		return;
	}

	olddata = m_data;
	len = length();

	//m_data = new strdata<CharT>;
	m_data = nullptr;

	EnsureAlloced(len + 1, false);
	copyn(m_data->data(), olddata->data(), len + 1);
	m_data->len = len;

	olddata->DelRef();
}

template<typename CharT>
void base_str<CharT>::EnsureAlloced(size_t amount, bool keepold)
{
	if (!m_data)
	{
		if (amount > 1)
		{
			unsigned char* buf = new unsigned char[sizeof(strdata<CharT>) + sizeof(CharT) * amount];
			m_data = new (buf) strdata<CharT>;

			//m_data->data() = new CharT[amount];
			m_data->alloced = amount;
			m_data->data()[0] = '\0';

			return;
		}

		if (!m_data) {
			return;
		}
	}

	CharT* newbuffer;
	bool wasalloced = (m_data->alloced != 0);

	if (amount < m_data->alloced) {
		return;
	}

	if (amount == m_data->alloced && !m_data->refcount)
	{
		// don't bother reallocating if it's the same amount
		return;
	}

	// Now, let's make sure it's writable
	//EnsureDataWritable();

	assert(amount);

	/*
	if (amount == 1)
	{
		m_data->alloced = 1;
	}
	else
	{
		size_t newsize;
		const size_t mod = amount % STR_ALLOC_GRAN;

		if (!mod)
			newsize = amount;
		else
			newsize = amount + STR_ALLOC_GRAN - mod;

		m_data->alloced = newsize;
	}
	*/

	unsigned char* buf = new unsigned char[sizeof(strdata<CharT>) + sizeof(CharT) * amount];
	strdata<CharT>* newdata = new (buf) strdata<CharT>;
	newbuffer = reinterpret_cast<CharT*>(buf + sizeof(strdata<CharT>));

	//if (wasalloced && keepold)
	if (keepold)
	{
		copy(newbuffer, m_data->data());
	}

	/*
	if (m_data->data())
	{
		delete[] m_data->data();
	}
	*/
	m_data->DelRef();

	m_data = newdata;
	//m_data->data() = newbuffer;
}

template<typename CharT>
void base_str<CharT>::BackSlashesToSlashes()
{
	size_t i;

	EnsureDataWritable();

	for (i = 0; i < m_data->len; i++)
	{
		if (m_data->data()[i] == '\\')
			m_data->data()[i] = '/';
	}
}

template<typename CharT>
void base_str<CharT>::SlashesToBackSlashes()
{
	size_t i;

	EnsureDataWritable();

	for (i = 0; i < m_data->len; i++)
	{
		if (m_data->data()[i] == '/')
			m_data->data()[i] = '\\';
	}
}

template<typename CharT>
void base_str<CharT>::DefaultExtension(const CharT* extension)
{
	static constexpr CharT dot[] = { '.', 0 };

	EnsureDataWritable();

	const CharT* src = m_data->data() + m_data->len - 1;

	while (*src != '/' && src != m_data->data())
	{
		if (*src == '.')
		{
			// it has an extension
			return;
		}
		src--;
	}

	append(dot);
	append(extension);
}

template<typename CharT>
const CharT* base_str<CharT>::GetExtension() const
{
	if(!m_data || !m_data->len)
	{
		// empty string
		return emptyStr<CharT>;
	}

	const size_t length = m_data->len - 1;
	size_t i = length;

	while (m_data->data()[i] != '.')
	{
		i--;
		if (m_data->data()[i] == '/' || i == 0)
		{
			// no extension
			return emptyStr<CharT>;
		}
	}

	return &m_data->data()[i + 1];
}

template<typename CharT>
void base_str<CharT>::StripExtension()
{
	EnsureDataWritable();

	size_t i = m_data->len;
	while (i > 0 && m_data->data()[i] != '.')
	{
		i--;
		if (m_data->data()[i] == '/')
			return; // no extension
	}
	if (i)
	{
		m_data->len = i;
		m_data->data()[m_data->len] = 0;

		EnsureDataWritable();
	}
}

template<typename CharT>
void base_str<CharT>::SkipFile()
{
	EnsureDataWritable();

	size_t i = m_data->len;
	while (i > 0 && m_data->data()[i] != '/' && m_data->data()[i] != '\\')
	{
		i--;
	}
	m_data->len = i;
	m_data->data()[m_data->len] = 0;

	EnsureDataWritable();
}

template<typename CharT>
void base_str<CharT>::SkipPath()
{
	EnsureDataWritable();

	const CharT* pathname = m_data->data();
	const CharT* last;

	last = m_data->data();
	while (*pathname)
	{
		if (*pathname == '/' || *pathname == '\\')
			last = pathname + 1;
		pathname++;
	}

	size_t lastpos = last - m_data->data();
	if (lastpos > 0)
	{
		size_t length = m_data->len - lastpos;
		for (size_t i = 0; i < length; i++)
		{
			m_data->data()[i] = last[i];
		}

		m_data->len = length;
		m_data->data()[length] = 0;

		EnsureDataWritable();
	}
}

template<typename CharT>
void base_str<CharT>::strip(void)
{
	CharT* last;
	CharT* s;
	size_t i;

	if (m_data == nullptr || m_data->data() == nullptr) return;
	s = m_data->data();
	while (isspace((int)*s) && *s) s++;

	last = s + m_data->len - (s - m_data->data());
	while (last > s) {
		if (!isspace((int)*(last - 1)))
			break;
		last--;
	}

	*last = '\0';

	m_data->len = last - s;
	for (i = 0; i < m_data->len + 1; i++)
	{
		m_data->data()[i] = s[i];
	}

	EnsureDataWritable();
}

template<typename CharT>
void base_str<CharT>::assign(const CharT* text, size_t sz)
{
	EnsureAlloced(sz + 1, true);

	m_data->len = sz;
	for (size_t i = 0; i < sz; ++i) {
		m_data->data()[i] = text[i];
	}

	// make sure to terminate the string
	m_data->data()[sz] = 0;
}

template<typename CharT>
void base_str<CharT>::resize(size_t len)
{
	EnsureAlloced(len + 1, true);

	const size_t start = m_data->len;
	m_data->len = len;
	for (size_t i = start; i < len + 1; ++i) {
		m_data->data()[i] = 0;
	}
}

template<typename CharT>
void base_str<CharT>::reserve(size_t len)
{
	// just reserve an amount
	EnsureAlloced(len + 1, true);
}

template<typename CharT>
void base_str<CharT>::clear()
{
	if (m_data)
	{
		m_data->DelRef();
		m_data = nullptr;
	}
}

template<>
base_str<char> base_str<char>::printfImpl(const char* fmt, ...)
{
	size_t len;
	va_list va;

	va_start(va, fmt);
	// Calculate the length
	len = vsnprintf(nullptr, 0, fmt, va);
	va_end(va);

	base_str<char> string;
	string.resize(len);

	va_start(va, fmt);
	vsnprintf(string.m_data->data(), string.m_data->alloced, fmt, va);
	va_end(va);

	return string;
}

template<>
base_str<wchar_t> base_str<wchar_t>::printfImpl(const wchar_t* fmt, ...)
{
	size_t len;
	va_list va;

	va_start(va, fmt);
	// Calculate the length
	len = vswprintf(nullptr, -1, fmt, va);
	va_end(va);

	base_str<wchar_t> string;
	string.resize(len);

	va_start(va, fmt);
	vswprintf(string.m_data->data(), string.m_data->alloced, fmt, va);
	va_end(va);

	return string;
}

template<typename CharT>
base_strview<CharT>::base_strview()
	: charArray(emptyStr<CharT>)
	, length(0)
	, isCharArray(true)
{
}

template<typename CharT>
base_strview<CharT>::base_strview(const CharT* inCharArray, size_t inLength)
	: charArray(inCharArray)
	, length(inLength != -1 ? inLength : base_str<CharT>::len(inCharArray))
	, isCharArray(true)
{
	if (!charArray) charArray = emptyStr<CharT>;
}

template<typename CharT>
base_strview<CharT>::base_strview(const base_str<CharT>& inString)
	: string(&inString)
	, length(inString.length())
	, isCharArray(false)
{
}

template<typename CharT>
const CharT* base_strview<CharT>::c_str() const
{
	return isCharArray ? charArray : string->c_str();
}

template<typename CharT>
size_t base_strview<CharT>::len() const
{
	return length;
}

template<typename CharT>
bool base_strview<CharT>::isDynamic() const
{
	return !isCharArray;
}

template<typename CharT>
const base_str<CharT>& base_strview<CharT>::getDynamicString() const
{
	assert(!isCharArray && string);
	return *string;
}

template<typename CharT>
base_const_str_static<CharT>::base_const_str_static(const CharT* stringValue)
	: string(stringValue)
{
}

template<typename CharT>
bool base_const_str_static<CharT>::operator==(const CharT* other) const
{
	return other && base_str<CharT>::cmp(string, other) == 0;
}

template<typename CharT>
bool base_str<CharT>::operator==(const base_str<CharT>& b) const
{
	return c_str() && !base_str::cmp(c_str(), b.c_str());
}

template<typename CharT>
bool base_str<CharT>::operator==(const CharT* b) const
{
/*
	assert(b);
	if (!b)
	{
		return false;
	}
	return (!base_str::cmp(c_str(), b));
*/
	return b && !base_str::cmp(c_str(), b);
}

template<typename CharT>
bool mfuse::operator==(const CharT* a, const base_str<CharT>& b)
{
	return a && !base_str<CharT>::cmp(a, b.c_str());
}

template<typename CharT>
bool base_const_str_static<CharT>::operator!=(const CharT* other) const
{
	return base_str<CharT>::cmp(string, other) != 0;
}

template<typename CharT>
bool base_str<CharT>::operator!=(const base_str<CharT>& b) const
{
	return !(*this == b);
}

template<typename CharT>
bool base_str<CharT>::operator!=(const CharT* b) const
{
	return !(*this == b);
}

template<typename CharT>
bool mfuse::operator!=(const CharT* a, const base_str<CharT>& b)
{
	return !(a == b);
}

template<typename CharT>
const CharT* base_const_str_static<CharT>::c_str() const
{
	return string;
}

template<typename CharT>
base_const_str_static<CharT>::operator const CharT* () const
{
	return string;
}

template<>
void mfuse::StringConvert(base_str<char>& to, const char* from)
{
	to = from;
}

template<>
void mfuse::StringConvert(base_str<char16_t>& to, const char* from)
{
	//std::wstring wcstr = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(from);
	//to = wcstr.c_str();
	std::u16string u16str = utf8::utf8to16(from);
	to = u16str.c_str();
}

template<>
void mfuse::StringConvert(base_str<char>& to, const char16_t* from)
{
	//std::string cstr = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(from);
	//to = cstr.c_str();
	std::string str8 = utf8::utf16to8(from);
	to = str8.c_str();
}

template<>
void mfuse::StringConvert(base_str<char32_t>& to, const char* from)
{
	//std::u32string u32cstr = std::wstring_convert<std::codecvt_utf8_utf16<char32_t>, char32_t>{}.from_bytes(from);
	//to = u32cstr.c_str();
	std::u32string u32str = utf8::utf8to32(from);
	to = u32str.c_str();
}

template<>
void mfuse::StringConvert(base_str<char>& to, const char32_t* from)
{
	//std::string cstr = std::wstring_convert<std::codecvt_utf8_utf16<char32_t>, char32_t>{}.to_bytes(from);
	//to = cstr.c_str();
	std::string str8 = utf8::utf32to8(from);
	to = str8.c_str();
}

template<>
void mfuse::StringConvert(base_str<wchar_t>& to, const char* from)
{
	switch (sizeof(wchar_t))
	{
	default:
	case 8:
		StringConvert(reinterpret_cast<base_str<char>&>(to), from);
		break;
	case 16:
		StringConvert(reinterpret_cast<base_str<char16_t>&>(to), from);
		break;
	case 32:
		StringConvert(reinterpret_cast<base_str<char32_t>&>(to), from);
		break;
	}
}

template<>
void mfuse::StringConvert(base_str<char>& to, const wchar_t* from)
{
	switch (sizeof(wchar_t))
	{
	default:
	case 8:
		StringConvert(to, reinterpret_cast<const char*>(from));
		break;
	case 16:
		StringConvert(to, reinterpret_cast<const char16_t*>(from));
		break;
	case 32:
		StringConvert(to, reinterpret_cast<const char32_t*>(from));
		break;
	}
}

namespace mfuse
{
template mfuse_EXPORTS class base_str<char>;
//template mfuse_EXPORTS class base_str<char16_t>;
template mfuse_EXPORTS class base_str<wchar_t>;
template mfuse_EXPORTS base_str<char> mfuse::operator+(const char* a, const base_str<char>& b);
template mfuse_EXPORTS base_str<wchar_t> mfuse::operator+(const wchar_t* a, const base_str<wchar_t>& b);
template mfuse_EXPORTS bool mfuse::operator==(const char* a, const base_str<char>& b);
template mfuse_EXPORTS bool mfuse::operator==(const wchar_t* a, const base_str<wchar_t>& b);
template mfuse_EXPORTS bool mfuse::operator!=(const char* a, const base_str<char>& b);
template mfuse_EXPORTS bool mfuse::operator!=(const wchar_t* a, const base_str<wchar_t>& b);
template mfuse_EXPORTS class base_strview<char>;
template mfuse_EXPORTS class base_strview<wchar_t>;
template mfuse_EXPORTS class base_const_str_static<char>;
template mfuse_EXPORTS class base_const_str_static<wchar_t>;
}
