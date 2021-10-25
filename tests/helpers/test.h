#include "context.h"
#include "handler.h"

#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Common/membuf.h>

#include <cassert>

template<size_t N>
inline void testAll(mfuse::ScriptContext& context, const handler_t (&handlers)[N])
{
	setupContext(context);
	executeHandlers(context, handlers);
}

template<size_t N>
inline void testAll(const handler_t (&handlers)[N])
{
	mfuse::ScriptContext context;

	testAll(context, handlers);
}

template<size_t N>
inline const mfuse::ProgramScript* compile(mfuse::ScriptMaster& director, const char* name, const char(&buf)[N])
{
	mfuse::imemstream stream(buf, sizeof(buf) - 1);
	const mfuse::ProgramScript* const script = director.GetProgramScript(name, stream);
	assert(script);

	return script;
}
