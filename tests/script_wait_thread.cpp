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

void script1(ScriptMaster& director)
{
	// Compile the script and return the game script object
	const ProgramScript* const script = compile(director, "script1", scriptContent_1);

	// Execute the thread from the beginning of the script
	Event parms;
	director.ExecuteThread(script, parms);
}

const handler_t list[] =
{
	"script1", &script1
};

int main()
{
	ScriptContext context;
	testAll(context, list);

	// Execute all pending events
	const TimeManager& tm = context.GetTimeManager();
	while (!context.IsIdle() && tm.GetTime() < 1000) {
		context.Execute();
	}

	assertTest(context.IsIdle());

	return 0;
}
