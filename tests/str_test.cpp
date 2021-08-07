#include <morfuse/Common/str.h>

#include <cassert>

using namespace mfuse;

#define string_assert(s, expected_len, expected_data) assert((s).length() == (expected_len) && !str::cmp((s), (expected_data)))

/*
=================
TestStringClass

This is a fairly rigorous test of the base_str class's functionality.
Because of the fairly global and subtle ramifications of a bug occuring
in this class, it should be run after any changes to the class.
Add more tests as functionality is changed.  Tests should include
any possible bounds violation and nullptr data tests.
=================
*/
void TestStringClass(void)
{
	// ch == ?
	char ch;
	// t == ?
	str* t;
	// a.len == 0, a.data == "\0"
	str a;
	// b.len == 0, b.data == "\0"
	str b;
	// c.len == 4, c.data == "test\0"
	str c("test");
	// d.len == 4, d.data == "test\0"
	str d(c);
	//str	e(reinterpret_cast<const char*>(nullptr));
	size_t i;

	// e.len == 0, e.data == "\0"					ASSERT!

	// i == 0
	i = a.length();
	assert(i == 0);
	// i == 4
	i = c.length();
	assert(i == 4);

	// s1 == "\0"
	const char* s1 = a.c_str();
	assert(*s1 == 0);
	// s2 == "test\0"
	const char* s2 = c.c_str();
	assert(!str::cmp(s2, "test"));

	// t->len == 0, t->data == "\0"
	t = new str();
	string_assert(*t, 0, "");
	// t == ?
	delete t;

	// b.len == 4, b.data == "test\0"
	b = "test";
	string_assert(b, 4, "test");
	// t->len == 4, t->data == "test\0"
	t = new str("test");
	string_assert(*t, 4, "test");
	// t == ?
	delete t;

	// a.len == 4, a.data == "test\0"
	a = c;
	string_assert(a, 4, "test");
	a = "";
	string_assert(a, 0, "");
	// a.len == 0, a.data == "\0"					ASSERT!
//	a = nullptr;
	// a.len == 8, a.data == "testtest\0"
	a = c + d;
	string_assert(a, 8, "testtest");
	// a.len == 7, a.data == "testwow\0"
	a = c + "wow";
	string_assert(a, 7, "testwow");
	// a.len == 4, a.data == "test\0"			ASSERT!
//	a = c + reinterpret_cast<const char*>(nullptr);
	a = c + "";
	// a.len == 8, a.data == "thistest\0"
	a = "this" + d;
	string_assert(a, 8, "thistest");
	// a.len == 4, a.data == "test\0"			ASSERT!
//	a = reinterpret_cast<const char*>(nullptr) + d;
	a = "" + d;
	// a.len == 8, a.data == "testtest\0"
	a += c;
	string_assert(a, 8, "testtest");
	// a.len == 11, a.data == "testtestwow\0"
	a += "wow";
	string_assert(a, 11, "testtestwow");

	// a.len == 11, a.data == "testtestwow\0"	ASSERT!
//	a += reinterpret_cast<const char*>(nullptr);
	a += "";

	// a.len == 4, a.data == "test\0"
	a = "test";
	string_assert(a, 4, "test");
	const str& ca = a;
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
	string_assert(a, 4, "tbst");
	// a.len == 4, a.data == "tbst\0"			ASSERT!
	a[-1] = 'b';
	string_assert(a, 4, "tbst");
	// a.len == 4, a.data == "0bst\0"
	a[0] = '0';
	string_assert(a, 4, "0bst");
	// a.len == 4, a.data == "01st\0"
	a[1] = '1';
	string_assert(a, 4, "01st");
	// a.len == 4, a.data == "012t\0"
	a[2] = '2';
	string_assert(a, 4, "012t");
	// a.len == 4, a.data == "0123\0"
	a[3] = '3';
	string_assert(a, 4, "0123");
	// a.len == 4, a.data == "0123\0"			ASSERT!
	a[4] = '4';
	string_assert(a, 4, "0123");
	// a.len == 4, a.data == "0123\0"			ASSERT!
	a[5] = '5';
	string_assert(a, 4, "0123");
	// a.len == 4, a.data == "0123\0"			ASSERT!
	a[7] = '7';
	string_assert(a, 4, "0123");

	// a.len == 4, a.data == "test\0"
	a = "test";
	string_assert(a, 4, "test");
	// b.len == 2, b.data == "no\0"
	b = "no";
	string_assert(b, 2, "no");

	// i == 0
	i = (a == b);
	assert(i == 0);
	// i == 1
	i = (a == c);
	assert(i == 1);

	// i == 0
	i = (a == "blow");
	assert(i == 0);
	// i == 1
	i = (a == "test");
	assert(i == 1);
	// i == 0											ASSERT!
	i = (a == nullptr);
	assert(i == 0);

	// i == 0
	i = ("test" == b);
	assert(i == 0);
	// i == 1
	i = ("test" == a);
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
	i = (a != "blow");
	assert(i == 1);
	// i == 0
	i = (a != "test");
	assert(i == 0);
	// i == 1											ASSERT!
	i = (a != nullptr);
	assert(i == 1);

	// i == 1
	i = ("test" != b);
	assert(i == 1);
	// i == 0
	i = ("test" != a);
	assert(i == 0);
	// i == 1											ASSERT!
	i = (nullptr != a);
	assert(i == 1);

	// a.data == "test"
	a = "test";
	string_assert(a, 4, "test");
	// b.data == "test"
	b = a;
	string_assert(b, 4, "test");

	// a.data == "not", b.data == "test"
	a = "not";
	string_assert(a, 3, "not");

	// a.data == b.data == "test"
	a = b;
	string_assert(a, 4, "test");

	// a.data == "testtest", b.data = "test"
	a += b;
	string_assert(a, 8, "testtest");

	a = b;
	string_assert(a, 4, "test");

	// a.data = "t1st", b.data = "test"
	a[1] = '1';
	string_assert(a, 4, "t1st");
	string_assert(b, 4, "test");

	a = "TeST";
	string_assert(a, 4, "TeST");
	a.tolower();
	string_assert(a, 4, "test");

	a = "";
	a.append('a');
	a.append('b');
	a.append('c');
	string_assert(a, 3, "abc");
}

int main(int argc, const char* argv[])
{
	TestStringClass();
	return 0;
}