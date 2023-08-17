#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData1[] =
"local.value = \"1234\"\n"
"level.string_value = local.value\n"
"level.string_value1 = 1\n"
"level.string_value2 = 1\n"
"level.string_value3 = 1\n"
"level.string_value4 = 1\n"
"level.string_value5 = 1\n"
"level.string_value6 = 1\n"
"level.string_value7 = 1\n"
"level.string_value8 = 1\n"
"level.string_value9 = 1\n"
"level.string_value10 = 1\n"
"level.string_value11 = 1\n"
"level.string_value12 = 1\n"
"level.string_value13 = 1\n"
"level.string_value14 = 1\n"
"level.string_value15 = 1\n"
"level.string_value16 = 1\n"
"level.string_value17 = 1\n"
"level.string_value18 = 1\n"
"level.string_value19 = 1\n"
"level.string_value20 = 1\n"
"level.string_value30 = 1\n"
"level.string_value31 = 1\n"
"level.string_value32 = 1\n"
"level.string_value33 = 1\n"
"level.string_value34 = 1\n"
"level.string_value35 = 1\n"
"level.string_value36 = 1\n"
"level.string_value37 = 1\n"
"level.string_value38 = 1\n"
"level.string_value39 = 1\n"
"level.string_value30 = 1\n"
"println \"test\"\n"
"println string (\"value: \" + level.string_value)\n"
;

void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptData1);
	director.ExecuteThread(script);
}

handler_t handlers[] =
{
	"level1", &level1,
};

int main()
{
	testAll(handlers);
	return 0;
}
