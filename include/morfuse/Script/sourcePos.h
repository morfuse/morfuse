#pragma once

#include "../Global.h"
#include "../Common/str.h"

#include <cstdint>
#include <ostream>

namespace mfuse
{
    class ostream;

    struct sourceLocation_t {
        uint32_t sourcePos;
        uint32_t line;
        uint16_t column;

        sourceLocation_t();
        sourceLocation_t(uint8_t val);

        mfuse_EXPORTS bool getLine(const rawchar_t* sourceBuffer, uint64_t sourceLength, str& sourceLine) const;
    };

    void printSourcePos(const sourceLocation_t& sourcePos, const rawchar_t* scriptName, const rawchar_t* sourceBuffer, uint64_t sourceLength, std::ostream& out);
}