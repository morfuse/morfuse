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
"local.ent3 = spawn SimpleEntity\n"
"local.ent3.targetname = \"test\"\n"
"local.org = local.ent1.origin\n"
"local.origin = local.org\n"
"local.origin[2] += 64\n"
"println local.origin[2]\n"
"println $test[1].targetname\n"
"println $test[2].targetname\n"
"println $test[0].targetname\n"
"end";

static const char scriptContent_level3[] =
"main:\n"
"for (local.i = 0; local.i <= 20; local.i++) {\n"
"local.p = spawn SimpleEntity targetname \"entity\"\n"
"}\n"
"$entity target \"other\"\n"
"end";

static const char scriptContent_level4[] =
"// -----------------------------------------------\n"
"//\n"
"// Random comment\n"
"local.p = spawn SimpleEntity targetname \"targetent\"\n"
"local.\"string\" = targetent\n"
//"$a::b = 1\n"
//"$(\"targetent\").value = 1\n"
"($(local.string)).value = 1\n"
//"$(local.string).value = 1\n"
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

void level4(ScriptMaster& director)
{
	ScriptContext::Get().GetOutputInfo().SetOutputStream(mfuse::outputLevel_e::Verbose, &std::cout);

	const ProgramScript* const script = compile(director, "level4", scriptContent_level4);
	director.ExecuteThread(script);
}

const handler_t handlers[] =
{
	{ "level1", &level1 },
	{ "level2", &level2 },
	{ "level3", &level3 },
	{ "level4", &level4 }
};

int main()
{
	testAll(handlers);
	return 0;
}
