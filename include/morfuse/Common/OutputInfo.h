#pragma once

#include "../Global.h"

#include <cstddef>
#include <cstdarg>
#include <ostream>

namespace mfuse
{
    enum class outputLevel_e
    {
        Verbose,
        Debug,
        Output,
        Warn,
        Error,
        Max
    };

    class OutputInfo
    {
    public:
        OutputInfo();

        mfuse_EXPORTS void SetOutputStream(outputLevel_e level, std::ostream* stream);
        mfuse_EXPORTS std::ostream* GetOutput(outputLevel_e level) const;

        mfuse_EXPORTS void Printf(outputLevel_e level, const char* fmt, va_list args) const;
        mfuse_EXPORTS void Printf(outputLevel_e level, const char* fmt, ...) const;
        mfuse_EXPORTS void VPrintf(const char* fmt, ...) const;
        mfuse_EXPORTS void DPrintf(const char* fmt, ...) const;
        mfuse_EXPORTS void Printf(const char* fmt, ...) const;
        mfuse_EXPORTS void Warn(const char* fmt, ...) const;
        mfuse_EXPORTS void Error(const char* fmt, ...) const;

    private:
        std::ostream* outputs[(size_t)outputLevel_e::Max];
    };

    class GlobalOutput
    {
    public:
        mfuse_EXPORTS static OutputInfo& Get();
    };
}
