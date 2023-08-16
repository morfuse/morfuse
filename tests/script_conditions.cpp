#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData1[] =
"local.value = 10\n"
"if (local.value == 0) println \"test\"\n"
"else\n"
"if (local.value == 1) println \"test2\"\n"
"else\n"
"if (local.value == 2) println \"test3\"\n"
"else\n"
"if (local.value == 3) println \"test3\"\n"
"else\n"
"if (local.value == 4) println \"test3\"\n"
"else\n"
"if (local.value == 5) println \"test3\"\n"
"else\n"
"if (local.value == 6) println \"test3\"\n"
"else\n"
"if (local.value == 7) println \"test3\"\n"
"else\n"
"if (local.value == 8) println \"test3\"\n"
"else\n"
"if (local.value == 9) println \"test3\"\n"
"else\n"
"if (local.value == 10) println \"test3\"\n"
;

const char scriptData2[] =
"local.value = 3\n"
"if ( local . value != 3 )\n"
"  local.value = 4;\n"
"else\n"
"{\n"
"  local.value = 5\n"
"}\n"
;

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

handler_t handlers[] =
{
	"level1", &level1,
	"level2", &level2,
};

int main()
{
	testAll(handlers);
	return 0;
}
