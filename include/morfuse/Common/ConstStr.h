#pragma once

#include "../Global.h"
#include "../Common/explicit_int.h"
#include <cstdint>

namespace mfuse
{
/**
 * This class holds an index to a string in the string table.
 */
class mfuse_EXPORTS const_str : public explicit_int<uint32_t, const_str>
{
public:
    using explicit_int::explicit_int;
    using explicit_int::operator=;
    constexpr const_str() {};
    constexpr operator bool() const { return value > 0; }

public:
    static constexpr const_str None() {
        return 0;
    }
};
}
