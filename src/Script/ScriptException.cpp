#include <morfuse/Script/ScriptException.h>
#include <stdarg.h>

using namespace mfuse;

ScriptException::ScriptException(const str& text)
	: string(text)
{
}

const char* ScriptException::what() const noexcept
{
	return string.c_str();
}

ScriptAbortException::ScriptAbortException(const str& text)
	: string(text)
{
}

const char* ScriptAbortException::what() const noexcept
{
	return "Unknown exception";
}
