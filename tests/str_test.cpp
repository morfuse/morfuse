#include <morfuse/Common/str.h>

#include <cassert>

using namespace mfuse;

template<typename CharT>
void string_assert(const base_str<CharT>& s, size_t expected_len, const CharT* expected_data)
{
	assert(s.length() == expected_len);
	assert(!base_str<CharT>::icmp(s, expected_data));
}

/*
=================
TestStringClass

This is a fairly rigorous test of the base_str class's functionality.
Because of the fairly global and subtle ramifications of a bug occurring
in this class, it should be run after any changes to the class.
Add more tests as functionality is changed.  Tests should include
any possible bounds violation and nullptr data tests.
=================
*/
template<typename CharT>
void TestStringClass()
{
	using StrT = base_str<CharT>;

	constexpr CharT testStr[] = { 't', 'e', 's', 't', 0 };
	constexpr CharT testTestStr[] = { 't', 'e', 's', 't', 't', 'e', 's', 't', 0 };
	constexpr CharT emptyStr[] = { 0 };

	// ch == ?
	CharT ch;
	// t == ?
	base_str<CharT>* t;
	// a.len == 0, a.data == "\0"
	base_str<CharT> a;
	// b.len == 0, b.data == "\0"
	base_str<CharT> b;
	// c.len == 4, c.data == "test\0"
	base_str<CharT> c(testStr);
	// d.len == 4, d.data == "test\0"
	base_str<CharT> d(c);
	base_str<CharT> e;
	size_t i;

	// e.len == 0, e.data == "\0"
	string_assert(e, 0, emptyStr);

	// i == 0
	i = a.length();
	assert(i == 0);
	// i == 4
	i = c.length();
	assert(i == 4);

	// s1 == "\0"
	const CharT* s1 = a.c_str();
	assert(*s1 == 0);
	// s2 == "test\0"
	const CharT* s2 = c.c_str();
	assert(!base_str<CharT>::cmp(s2, testStr));

	// t->len == 0, t->data == "\0"
	t = new base_str<CharT>();
	string_assert(*t, 0, emptyStr);
	// t == ?
	delete t;

	// b.len == 4, b.data == "test\0"
	b = testStr;
	string_assert(b, 4, testStr);
	// t->len == 4, t->data == "test\0"
	t = new base_str<CharT>(testStr);
	string_assert(*t, 4, testStr);
	// t == ?
	delete t;

	// a.len == 4, a.data == "test\0"
	a = c;
	string_assert(a, 4, testStr);
	a = emptyStr;
	string_assert(a, 0, emptyStr);
	// a.len == 0, a.data == "\0"					ASSERT!
//	a = nullptr;
	// a.len == 8, a.data == "testtest\0"
	a = c + d;

	string_assert(a, 8, testTestStr);
	// a.len == 7, a.data == "testwow\0"
	constexpr CharT wowStr[] = { 'w', 'o', 'w', 0 };
	{
		a = c + wowStr;
		constexpr CharT expectedStr[] = { 't', 'e', 's', 't', 'w', 'o', 'w', 0};
		string_assert(a, 7, expectedStr);
	}
	// a.len == 4, a.data == "test\0"			ASSERT!
//	a = c + reinterpret_cast<const char*>(nullptr);
	a = c + emptyStr;
	// a.len == 8, a.data == "thistest\0"
	{
		constexpr CharT thisStr[] = { 't', 'h', 'i', 's', 0};
		a = thisStr + d;
		constexpr CharT expectedStr[] = { 't', 'h', 'i', 's', 't', 'e', 's', 't', 0};
		string_assert(a, 8, expectedStr);
	}
	// a.len == 4, a.data == "test\0"			ASSERT!
//	a = reinterpret_cast<const char*>(nullptr) + d;
	a = emptyStr + d;
	// a.len == 8, a.data == "testtest\0"
	a += c;
	string_assert(a, 8, testTestStr);
	// a.len == 11, a.data == "testtestwow\0"
	a += wowStr;

	constexpr CharT testTestWowStr[] = { 't', 'e', 's', 't', 't', 'e', 's', 't', 'w', 'o', 'w', 0};
	string_assert(a, 11, testTestWowStr);

	// a.len == 11, a.data == "testtestwow\0"	ASSERT!
//	a += reinterpret_cast<const char*>(nullptr);
	a += emptyStr;

	// a.len == 4, a.data == "test\0"
	a = testStr;
	string_assert(a, 4, testStr);
	const base_str<CharT>& ca = a;
	// ch == 't'
	ch = ca[0];
	assert(ch == 't');
	// ch == 0											ASSERT!
	ch = ca[-1];
	assert(ch == 0);
	// ch == 0											ASSERT!
	ch = ca[1000];
	assert(ch == 0);
	// ch == 't'
	ch = ca[0];
	assert(ch == 't');
	// ch == 'e'
	ch = ca[1];
	assert(ch == 'e');
	// ch == 's'
	ch = ca[2];
	assert(ch == 's');
	// ch == 't'
	ch = ca[3];
	assert(ch == 't');
	// ch == '\0'										ASSERT!
	ch = ca[4];
	assert(ch == 0);
	// ch == '\0'										ASSERT!
	ch = ca[5];
	assert(ch == 0);

	// a.len == 4, a.data == "tbst\0"
	a[1] = 'b';
	constexpr CharT tbstStr[] = { 't', 'b', 's', 't', 0};
	string_assert(a, 4, tbstStr);
	// a.len == 4, a.data == "tbst\0"			ASSERT!
	a[-1] = 'b';
	string_assert(a, 4, tbstStr);
	// a.len == 4, a.data == "0bst\0"
	a[0] = '0';
	constexpr CharT _0bstStr[] = { '0', 'b', 's', 't', 0 };
	string_assert(a, 4, _0bstStr);
	// a.len == 4, a.data == "01st\0"
	a[1] = '1';
	constexpr CharT _01stStr[] = { '0', '1', 's', 't', 0 };
	string_assert(a, 4, _01stStr);
	// a.len == 4, a.data == "012t\0"
	a[2] = '2';
	constexpr CharT _012tStr[] = { '0', '1', '2', 't', 0 };
	string_assert(a, 4, _012tStr);
	// a.len == 4, a.data == "0123\0"
	a[3] = '3';
	constexpr CharT _0123Str[] = { '0', '1', '2', '3', 0 };
	string_assert(a, 4, _0123Str);
	// a.len == 4, a.data == "0123\0"			ASSERT!
	a[4] = '4';
	string_assert(a, 4, _0123Str);
	// a.len == 4, a.data == "0123\0"			ASSERT!
	a[5] = '5';
	string_assert(a, 4, _0123Str);
	// a.len == 4, a.data == "0123\0"			ASSERT!
	a[7] = '7';
	string_assert(a, 4, _0123Str);

	// a.len == 4, a.data == "test\0"
	a = testStr;
	string_assert(a, 4, testStr);
	// b.len == 2, b.data == "no\0"
	constexpr CharT noStr[] = { 'n', 'o', 0 };
	b = noStr;
	string_assert(b, 2, noStr);

	// i == 0
	i = (a == b);
	assert(i == 0);
	// i == 1
	i = (a == c);
	assert(i == 1);

	// i == 0
	constexpr CharT blowStr[] = { 'b', 'l', 'o', 'w', 0};
	i = (a == blowStr);
	assert(i == 0);
	// i == 1
	i = (a == testStr);
	assert(i == 1);
	// i == 0											ASSERT!
	i = (a == nullptr);
	assert(i == 0);

	// i == 0
	i = (testStr == b);
	assert(i == 0);
	// i == 1
	i = (testStr == a);
	assert(i == 1);
	// i == 0											ASSERT!
	i = (nullptr == a);
	assert(i == 0);

	// i == 1
	i = (a != b);
	assert(i == 1);
	// i == 0
	i = (a != c);
	assert(i == 0);

	// i == 1
	i = (a != blowStr);
	assert(i == 1);
	// i == 0
	i = (a != testStr);
	assert(i == 0);
	// i == 1											ASSERT!
	i = (a != nullptr);
	assert(i == 1);

	// i == 1
	i = (testStr != b);
	assert(i == 1);
	// i == 0
	i = (testStr != a);
	assert(i == 0);
	// i == 1											ASSERT!
	i = (nullptr != a);
	assert(i == 1);

	// a.data == testStr
	a = testStr;
	string_assert(a, 4, testStr);
	// b.data == testStr
	b = a;
	string_assert(b, 4, testStr);

	// a.data == "not", b.data == testStr
	constexpr CharT notStr[] = { 'n', 'o', 't', 0 };
	a = notStr;
	string_assert(a, 3, notStr);

	// a.data == b.data == testStr
	a = b;
	string_assert(a, 4, testStr);

	// a.data == "testtest", b.data = testStr
	a += b;
	string_assert(a, 8, testTestStr);

	a = b;
	string_assert(a, 4, testStr);

	// a.data = "t1st", b.data = testStr
	a[1] = '1';
	constexpr CharT t1stStr[] = { 't', '1', 's', 't', 0};
	string_assert(a, 4, t1stStr);
	string_assert(b, 4, testStr);

	a = testStr;
	string_assert(a, 4, testStr);
	a.tolower();
	string_assert(a, 4, testStr);

	a = emptyStr;
	a.append('a');
	a.append('b');
	a.append('c');
	constexpr CharT abcStr[] = { 'a', 'b', 'c', 0};
	string_assert(a, 3, abcStr);

	e = base_str<CharT>(1);
	constexpr CharT _1str[] = { '1', 0 };
	string_assert(e, 1, _1str);
	e = base_str<CharT>(10);
	constexpr CharT _10str[] = { '1', '0', 0};
	string_assert(e, 2, _10str);
	e = base_str<CharT>(1.5f);
	constexpr CharT _1500str[] = { '1', '.', '5', '0', '0', 0};
	string_assert(e, 5, _1500str);
}

int main(int argc, const char* argv[])
{
	TestStringClass<char>();
	TestStringClass<wchar_t>();
	TestStringClass<char16_t>();
	TestStringClass<char32_t>();

	return 0;
}