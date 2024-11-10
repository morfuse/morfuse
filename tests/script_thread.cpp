#include <morfuse/Script/Context.h>
#include <morfuse/Script/StateScript.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/SourceException.h>
#include <morfuse/Common/membuf.h>

#include "helpers/test.h"
#include "helpers/assert.h"

#include <fstream>
#include <iostream>

using namespace mfuse;

static const char scriptContent_level1[] =
"\n\n\n"
"{ println \"Hello, world\\n\"\n }\n"
"\t\tmain:\n\n\n\n"
"local.var = 1000\n"
"local.var += 200\n"
"local.var >>= 1\n"
"local.var = (local.var + 1) * 2 / 3 * 4 + 5\n"
"end local.var";

static const char scriptContent_level2[] =
"main:\n"
"\n\n\n\n\n\n"
"{\n"
"println \"Hello, world 2\\n\"\n"
"local.i = 1\n\n"
"local.j = \tùùù¨slash/dot.ccc\t + test\t\n"
"println local.j\n"
"if(1)\n { }\n else\n {\n \n}\n"
"}\n"
"end local.j\n\n\n";

static const char scriptContent_level3[] =
"main:\n"
"local.i = 1\n"
"switch(local.i)\n"
"{\n"
"case 0: break\n"
"case 1:\n"
"end 571\n"
"break\n"
"default: break\n"
"}\n";

static const char scriptContent_level4[] =
"main:\n"
"local.arr = makeArray\n"
"10 11 12\n"
"20 21 22\n"
"endArray\n"
"end local.arr";

static const char scriptContent_level5[] =
"main:\n"
"do {\n"
"local.val = 10\n"
"break\n"
"local.val = 2000\n"
"} while(0)\n"
"local.i = 1 | 2 | 4 | 8 & 16\n"
"end ((local.i + local.val) ^ 3)";

static const char scriptContent_level6[] =
"main1 local.i local.j local.k local.l:\n"
"local.val = local.i / local.j\n"
"end (local.val * local.k)";

static const char scriptContent_level7[] =
"main:\n"
"local.larr = local::local::local::local\n"
"local.larr println \"4 listeners\"\n"
"local.larr = local::local::local\n"
"local.larr println \"3 listeners\"\n"
"local.larr = local::local\n"
"local.larr println \"2 listeners\"\n"
"local.test1 = 1::2::3::4::5\n"
"local.test2 = 5::4::3::2::1\n"
"local.larr = local.test1::local.test2\n"
"for(local.i = 1; local.i <= local.larr.size; local.i++) {\n"
" local.b = local.larr[local.i]\n"
" println (local.b)\n"
" for(local.j = 1; local.j <= local.larr[local.i].size; local.j++) {\n"
"  println local.b[local.j]\n"
" }\n"
"}\n"
"end";

static const char scriptContent_level8[] =
"main:\n"
"local.arr[0] = 1\n"
"local.arr[1] = 2\n";

static const char scriptContent_level9[] =
"main:\n"
"exec test.scr\n"
"thread test.scr::main";

static const char scriptContent_level10[] =
"main:\n"
"local.i = waitthread other 1\n"
"local.j = -1\n"
"if (local.i == local.j) end -1\n"
"end 0\n"
"other local.var:\n"
"end -local.var\n";

static const char scriptContent_level11[] =
"targetname";

static const char scriptContent_level12[] =
"\n\n";

static const char scriptContent_level13[] =
"";

static const char scriptContent_level14[] =
"main:\n"
"local.var = 1\n"
"local.type = typeof local.var\n"
"println local.type\n"
"end\n";

static const char scriptContent_level15[] =
"main:\n"
"local.listener = local CreateListener\n"
"local .listener.1value = .85\n"
"local.test = test\n"
"println (local.listener.1value)\n"
"end\n";

static const char scriptContent_level16[] =
"main:\n"
"local.value = local CreateListener\n"
"for ( local.i=1;local.i<= 5;local.i++ ) {}\n"
"thread test (value1 + (local.value)) .75 .65\n"
"end\n"
"test local.str local.num1 local.num2:\n"
"println (\"str: \" + local.str + \" num: \" + local.num1 + \" num2: \" + local.num2)\n"
"end\n";

static const char scriptContent_level17[] =
"main:\n"
"thread test2 (0 0 -170) .3\n"
"end\n"
"test2 local.vec local.num:\n"
"println (\"vec: \" + local.vec + \" num: \" + local.num)\n"
"end\n";

static const char scriptContent_level18[] =
"main:\n"
"local.88varwith_num = \"test\"\n"
"println local.88varwith_num\n"
"end\n";

void level1(ScriptMaster& director)
{
	// Compile the script and return the game script object
	const ProgramScript* const script = compile(director, "level1", scriptContent_level1);

	// Execute the thread from the beginning of the script
	Event parms;
	director.ExecuteThread(script, parms);

	uint32_t intValue = parms.GetInteger(1);
	assertTest(intValue == (((1000 + 200) >> 1) + 1) * 2 / 3 * 4 + 5);
}

void level2(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level2", scriptContent_level2);

	Event parms;
	director.ExecuteThread(script, parms);

	const str retVal = parms.GetString(1);
	assertTest(!str::cmp(retVal.c_str(), "ùùù¨slash/dot.ccctest"));
}

void level3(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level3", scriptContent_level3);

	Event parms;
	director.ExecuteThread(script, parms);

	const uint32_t intValue = parms.GetInteger(1);
	assertTest(intValue == 571);
}

void level4(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level4", scriptContent_level4);

	Event parms;
	director.ExecuteThread(script, parms);

	ScriptVariable& var = parms.GetValue(1);
	ScriptVariable* row1 = var.constArrayValue()[1].constArrayValue();
	ScriptVariable* row2 = var.constArrayValue()[2].constArrayValue();
	assertTest(row1[1].GetType() == variableType_e::Integer && row1[2].GetType() == variableType_e::Integer && row1[3].GetType() == variableType_e::Integer);
	assertTest(row2[1].GetType() == variableType_e::Integer && row2[2].GetType() == variableType_e::Integer && row2[3].GetType() == variableType_e::Integer);
	assertTest(row1[1].intValue() == 10 && row1[2].intValue() == 11 && row1[3].intValue() == 12);
	assertTest(row2[1].intValue() == 20 && row2[2].intValue() == 21 && row2[3].intValue() == 22);
}

void level5(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level5", scriptContent_level5);

	Event parms;
	director.ExecuteThread(script, parms);

	const uint32_t intValue = parms.GetInteger(1);
	assertTest(intValue == (((1 | 2 | 4 | 8 & 16) + 10) ^ 3));
}

void level6(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level6", scriptContent_level6);

	Event parms;
	parms.AddInteger(100);
	parms.AddInteger(30);
	parms.AddInteger(300);
	director.ExecuteThread(script, parms);

	const uint32_t intValue = parms.GetInteger(parms.NumArgs());
	assertTest(intValue == ((100 / 30) * 300));
}

void level7(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level7", scriptContent_level7);

	director.ExecuteThread(script);
}

void level8(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level8", scriptContent_level8);

	director.ExecuteThread(script);
	director.ExecuteThread(script);
}

void level9(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level9", scriptContent_level9);

	director.ExecuteThread(script);
}

void level10(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level10", scriptContent_level10);

	Event parms;
	director.ExecuteThread(script, parms);

	assertTest(parms.GetInteger(1) == -1);
}

void level11(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level11", scriptContent_level11);
	director.ExecuteThread(script);
}

void level12(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level12", scriptContent_level12);
	director.ExecuteThread(script);
}

void level13(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level13", scriptContent_level13);
	director.ExecuteThread(script);
}

void level14(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level14", scriptContent_level14);
	director.ExecuteThread(script);
}

void level15(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level15", scriptContent_level15);
	director.ExecuteThread(script);
}

void level16(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level16", scriptContent_level16);
	director.ExecuteThread(script);
}

void level17(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level17", scriptContent_level17);
	director.ExecuteThread(script);
}

void level18(ScriptMaster& director)
{
    const ProgramScript* const script = compile(director, "level18", scriptContent_level18);
    director.ExecuteThread(script);
}

void m3l1a(ScriptMaster& director)
{
	std::fstream stream("m3l1a.scr", std::ios_base::in | std::ios_base::binary);
	if (!stream.is_open())
	{
		// FIXME
		return;
	}

	stream.seekg(0, stream.end);
	const std::streamoff fsize = stream.tellg();
	stream.seekg(0, stream.beg);

	if (!fsize) {
		return;
	}

	const ProgramScript* const script = director.GetProgramScript("m3l1a", stream);

	//assertTest(script);
	director.ExecuteThread(script);
}

const handler_t list[] =
{
	{ "level1", &level1 },
	{ "level2", &level2 },
	{ "level3", &level3 },
	{ "level4", &level4 },
	{ "level5", &level5 },
	{ "level6", &level6 },
	{ "level7", &level7 },
	{ "level8", &level8 },
	{ "level9", &level9 },
	{ "level10", &level10 },
	{ "level11", &level11 },
	{ "level12", &level12 },
	{ "level13", &level13 },
	{ "level14", &level14 },
	{ "level15", &level15 },
	{ "level16", &level16 },
	{ "level17", &level17 },
	{ "level18", &level18 },
	{ "m3l1a", &m3l1a },
};

int main(int argc, char* argv[])
{
	ScriptContext context;
	testAll(context, list);

	// Execute all pending events
	context.Execute();

	assertTest(context.IsIdle());

	return 0;
}
