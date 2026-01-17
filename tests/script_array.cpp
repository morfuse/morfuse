#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData1[] =
"group.test = makeArray\n"
"/*\n"
"  test comment\n"
"*/\n"
"\"hai\" 1\n"
"\"2\" 1\n"
"\"u\" 1\n"
"\"all\" 1\n"
"endArray\n"
"thread test\n"
"test:\n"
"println group.test[1][1]\n"
"end;";

const char scriptData2[] =
"local.test[0] = 0\n"
"local.test[1] = spawn NULL\n"
"if (local.test[1]) println \"test\"\n"
"end";

const char scriptData3[] =
"local.value = 0\n"
"local.test[local.value + 1][1] = teststr\n"
"local.test[local.value + 1][2] = 1\n"
"local.test[local.value + 1][3] = local\n"
"local.test[local.value + 1][4] = 3.4\n"
"local.test[local.value + 1][5] = \"value\"\n"
"for(local.i = 1; local.i <= local.test.size; local.i++) {\n"
"println local.test[local.i][5]\n"
"}\n"
"end";

const char scriptData4[] =
"level.map_bsp[0] = spawn Listener\n"
"level.map_bsp[0].pa = -0.688:: -0.749\n"
"println level.map_bsp[0].pa[1]\n"
"end";

const char scriptData5[] =
"local.value1 = thread test\n"
"end local.value1[2]\n"
"test:\n"
"wait 0.1\n"
"end 2";

void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptData1);
	director.ExecuteThread(script);
}

void level2(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level2", scriptData2);
	director.ExecuteThread(script);
}

void level3(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level3", scriptData3);
	director.ExecuteThread(script);
}

void level4(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level4", scriptData4);
	director.ExecuteThread(script);
}

void level5(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level5", scriptData5);
	director.ExecuteThread(script);
}

handler_t handlers[] =
{
	"level1", &level1,
	"level2", &level2,
	"level3", &level3,
	"level4", &level4,
	"level5", &level5
};

int main()
{
	testAll(handlers);
	return 0;
}

