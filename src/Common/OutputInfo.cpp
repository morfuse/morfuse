#include <morfuse/Common/OutputInfo.h>

#include <cstdarg>
#include <cstdio>

using namespace mfuse;

OutputInfo::OutputInfo()
	: outputs{}
{
}

void OutputInfo::SetOutputStream(outputLevel_e level, std::ostream* stream)
{
	outputs[(uintptr_t)level] = stream;
}

std::ostream* OutputInfo::GetOutput(outputLevel_e level) const
{
	return outputs[(uintptr_t)level];
}

void OutputInfo::Printf(outputLevel_e level, const char* fmt, va_list args) const
{
	std::ostream* stream = outputs[(uintptr_t)level];
	if (!stream) return;

	const size_t len = vsnprintf(nullptr, 0, fmt, args);

	char* data = new char[len + 1];
	vsnprintf(data, len + 1, fmt, args);

	*stream << data;

	delete[] data;
}

void OutputInfo::Printf(outputLevel_e level, const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Printf(level, fmt, args);
	va_end(args);
}

void OutputInfo::VPrintf(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Printf(outputLevel_e::Verbose, fmt, args);
	va_end(args);
}

void OutputInfo::DPrintf(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Printf(outputLevel_e::Debug, fmt, args);
	va_end(args);
}

void OutputInfo::Printf(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Printf(outputLevel_e::Output, fmt, args);
	va_end(args);
}

void OutputInfo::Warn(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Printf(outputLevel_e::Warn, fmt, args);
	va_end(args);
}

void OutputInfo::Error(const char* fmt, ...) const
{
	va_list args;
	va_start(args, fmt);
	Printf(outputLevel_e::Error, fmt, args);
	va_end(args);
}

OutputInfo& GlobalOutput::Get()
{
	static OutputInfo info;
	return info;
}
