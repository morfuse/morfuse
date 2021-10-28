#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

static const char scriptContent_level1[] =
"main:\n"
"local.obj1 = spawn SimpleEntity\n"
"local.obj1.targetname = \"test\"\n"
"local.obj2 = spawn SimpleEntity\n"
"local.obj2.targetname = \"test\"\n"
"local.l1 = local CreateListener\n"
"local.l2 = local CreateListener\n"
"println $test[1].targetname\n"
"end";

static const char scriptContent_level2[] =
"main:\n"
"local.ent1 = spawn SimpleEntity\n"
"local.ent1.targetname = \"test\"\n"
"local.ent2 = spawn SimpleEntity\n"
"local.ent2.targetname = \"test\"\n"
"println $test[0].targetname\n"
"end";

static const char scriptContent_level3[] =
"main:\n"
"for (local.i = 0; local.i <= 20; local.i++) {\n"
"local.p = spawn SimpleEntity targetname \"entity\"\n"
"}\n"
"$entity target \"other\"\n"
"end";


void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptContent_level1);
	director.ExecuteThread(script);

	ScriptContext& context = ScriptContext::Get();
	const TargetList& tl = context.GetTargetList();
	const_str targetName = director.GetDictionary().Get("test");
	const Listener* comp = tl.GetNextTarget(nullptr, targetName);
	assertTest(comp);
	assertTest(tl.GetNextTarget(comp, targetName));

	context.GetTrackedInstances().Cleanup();
	comp = tl.GetNextTarget(nullptr, targetName);
	assertTest(comp);
	assertTest(tl.GetNextTarget(comp, targetName));
}

void level2(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level2", scriptContent_level2);
	director.ExecuteThread(script);
}

void level3(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level3", scriptContent_level3);
	director.ExecuteThread(script);
}

const handler_t handlers[] =
{
	{ "level1", &level1 },
	{ "level2", &level2 },
	{ "level3", &level3 }
};

int main()
{
	testAll(handlers);
	return 0;
}
