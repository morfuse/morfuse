#pragma once

#include <cstddef>

namespace mfuse
{
    /** Rawchar is a type of character that is platform-dependent. */
    using rawchar_t = char;

    /*
    template <size_t S>
    constexpr const wchar_t* _s(const char (&literal)[S])
    {
        static wchar_t r[S] = {};

        for (size_t i = 0; i < S; i++)
            r[i] = literal[i];

        return r;
    }

    constexpr std::nullptr_t _s(std::nullptr_t)
    {
        return nullptr;
    }
    */
}