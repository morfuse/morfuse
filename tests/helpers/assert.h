#include <iostream>

#define assertTest(condition) \
	if (!(condition)) assertFail(#condition, __FILE__, __LINE__)

inline void assertFail(const char* condition, const char* fileName, unsigned int lineNum)
{
	std::cerr << "Assertion failed at '" << fileName << "' line " << lineNum << ":" << std::endl;
	std::cerr << condition << std::endl;

	throw;
}