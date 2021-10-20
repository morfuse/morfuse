#include <morfuse/Script/ScriptException.h>
#include <stdarg.h>

using namespace mfuse;

ScriptException::ScriptException(const xstr& text)
	: string(text)
{
}

const char* ScriptException::what() const noexcept
{
	return string.c_str();
}

ScriptAbortException::ScriptAbortException(const xstr& text)
	: string(text)
{
}

const char* ScriptAbortException::what() const noexcept
{
	return "Unknown exception";
}

void Messageable::fill(const xstr& msg) const
{
	// fill the message for the first time
	Messageable* This = const_cast<Messageable*>(this);
	This->msg = msg;
}

bool Messageable::filled() const noexcept
{
	return !msg.isEmpty();
}

const char* Messageable::what() const noexcept
{
	return msg.c_str();
}
