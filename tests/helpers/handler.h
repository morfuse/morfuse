#pragma once

#include <morfuse/Script/ScriptMaster.h>

#include <cstdint>
#include <cstddef>
#include <iostream>

struct handler_t
{
	const char* name;
	void (*handler)(mfuse::ScriptMaster& director);
};

template<size_t N>
inline void executeHandlers(mfuse::ScriptContext& context, const handler_t (&handlers)[N])
{
	// Get the script master
	mfuse::ScriptMaster& director = context.GetDirector();

	for (size_t i = 0; i < N; ++i)
	{
		const handler_t& h = handlers[i];
		h.handler(director);
	}
}
