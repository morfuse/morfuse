#include <morfuse/Script/ScriptException.h>
#include <stdarg.h>

using namespace mfuse;

ScriptException::ScriptException(const xstr& text)
	: string(text)
{
}

ScriptException::ScriptException(const rawchar_t* format, ...)
{
	va_list va;
	rawchar_t data[4100];

	va_start(va, format);
	vsprintf(data, format, va);
	va_end(va);

	string = data;
}

ScriptAbortException::ScriptAbortException(const xstr& text)
	: ScriptException(text)
{
}

ScriptAbortException::ScriptAbortException(const rawchar_t* text)
	: ScriptException(text)
{

}
