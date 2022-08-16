#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptContent_1[] =
"main:\n"
"thread t1\n"
"thread t2\n"
"println \"done\"\n"
"end\n"
"t1:\n"
"println \"begin t1\"\n"
"wait 0.5\n"
"println \"end t1\"\n"
"end\n"
"t2:\n"
"println \"begin t2\"\n"
"wait 0.5\n"
"println \"end t2\"\n"
"end\n";

const char scriptContent_2[] =
"main:\n"
"thread continuelater local\n"
"pause\n"
"println ok\n"
"end 1\n"
"continuelater local.thread:\n"
"wait 0.05\n"
"local.thread wait 0\n"
"end";

void script1(ScriptMaster& director)
{
	// Compile the script and return the game script object
	const ProgramScript* const script = compile(director, "script1", scriptContent_1);

	// Execute the thread from the beginning of the script
	Event parms;
	director.ExecuteThread(script, parms);

	// Execute all pending events
	ScriptContext& ctx = ScriptContext::Get();
	const TimeManager& tm = ctx.GetTimeManager();
	while (!ctx.IsIdle() && tm.GetTime() < 1000) {
		ctx.Execute();
	}
}

void script2(ScriptMaster& director)
{
	// Compile the script and return the game script object
	const ProgramScript* const script = compile(director, "script2", scriptContent_2);

	// Execute the thread from the beginning of the script
	Event parms;
	director.ExecuteThread(script, parms);

	// Execute all pending events
	ScriptContext& ctx = ScriptContext::Get();
	const TimeManager& tm = ctx.GetTimeManager();
	while (!ctx.IsIdle() && tm.GetTime() < 1000) {
		ctx.Execute();
	}

	assertTest(parms.GetInteger(1) == 1);
}

const handler_t list[] =
{
	{ "script1", &script1 },
	{ "script2", &script2 }
};

int main()
{
	ScriptContext context;
	testAll(context, list);

	assertTest(context.IsIdle());

	return 0;
}
