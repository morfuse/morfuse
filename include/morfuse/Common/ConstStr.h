#pragma once

#include "../Global.h"
#include "../Common/explicit_int.h"
#include <cstdint>

namespace mfuse
{

enum const_str_e
{
	STRING_EMPTY = 1,
};

/**
 * This class holds an index to a string in the string table.
 */
class mfuse_EXPORTS const_str : public explicit_int<uint32_t, const_str>
{
public:
	using explicit_int::explicit_int;
	using explicit_int::operator=;
	constexpr const_str() {};
	constexpr const_str(const_str_e valueValue) : explicit_int((uint32_t)valueValue) {};
	constexpr const_str& operator=(const_str_e valueValue) { value = valueValue; return *this; };
	constexpr operator bool() const { return value > 0; }

	friend constexpr bool operator==(const_str lhs, const_str_e rhs) { return lhs.value == rhs; }
	friend constexpr bool operator!=(const_str lhs, const_str_e rhs) { return lhs.value != rhs; }
	friend constexpr bool operator==(const_str_e lhs, const_str rhs) { return lhs == rhs.value; }
	friend constexpr bool operator!=(const_str_e lhs, const_str rhs) { return lhs != rhs.value; }
};
}
