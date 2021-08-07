#include <morfuse/Script/ScriptException.h>
#include <stdarg.h>

using namespace mfuse;

int ScriptException::next_abort = 0;
int ScriptException::next_bIsForAnim = 0;

void ScriptException::CreateException(const rawchar_t* data)
{
	string = data;

	bAbort = next_abort;
	next_abort = 0;
	bIsForAnim = next_bIsForAnim;
	next_bIsForAnim = 0;
}

ScriptException::ScriptException(const xstr& text)
{
	CreateException(text.c_str());
}

ScriptException::ScriptException(const rawchar_t* format, ...)
{
	va_list va;
	rawchar_t data[4100];

	va_start(va, format);
	vsprintf(data, format, va);
	va_end(va);

	CreateException(data);
}

void Error(const rawchar_t* format, ...)
{
	va_list va;
	rawchar_t data[4100];

	va_start(va, format);
	vsprintf(data, format, va);
	va_end(va);

	throw new ScriptException((const rawchar_t*)data);
}
