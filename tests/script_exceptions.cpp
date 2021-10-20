#include "helpers/test.h"

#include <morfuse/Script/StateScript.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/SourceException.h>

using namespace mfuse;

static const char scriptContent_exception1[] =
"main:\n"
"end\n"
"valid_label:\n"
"println \"valid_label\"\n"
"local.i = 1";

static const char scriptContent_exception2[] =
"main:\n"
"local.i = local.i + 1\n"
"local targetname \"test\"";

static const char scriptContent_exception3[] =
"main:\n"
"test";

void exception1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "exception1", scriptContent_exception1);

	bool gotException = false;
	try
	{
		director.ExecuteThread(script, "invalid_label");
	}
	catch (StateScriptErrors::LabelNotFound& e)
	{
		gotException = true;
		assert(e.GetFileName() == script->Filename());
		assert(e.GetLabel() == "invalid_label");
	}

	assert(gotException);

	director.ExecuteThread(script, "valid_label");
}

void exception2(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "exception2", scriptContent_exception2);
	director.ExecuteThread(script);
}

void exception3(ScriptMaster& director)
{
	memstream stream(scriptContent_exception3, sizeof(scriptContent_exception3));

	bool gotException = false;
	try
	{
		const ProgramScript* const script = director.GetProgramScript("exception3", stream);
		assert(!script);
	}
	catch (CompileException::UnknownCommand& e)
	{
		gotException = true;
		assert(!str::cmp(e.getCommandName(), "test"));
	}
}

const handler_t handlers[] =
{
	{ "exception1", &exception1 },
	{ "exception2", &exception2 },
	{ "exception3", &exception3 }
};

int main()
{
	testAll(handlers);
	return 0;
}
