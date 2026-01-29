#pragma once

#include <cstdint>

namespace mfuse
{
template<typename T>
struct EqualTo
{
    constexpr bool operator()(const T& lhs, const T& rhs) const
    {
        return lhs == rhs;
    }
};

namespace con
{
    mfuse_EXPORTS extern size_t set_primes[24];
}
}