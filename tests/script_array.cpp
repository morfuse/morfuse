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
"end";

void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptData1);
	director.ExecuteThread(script);
}

handler_t handlers[] =
{
	"level1", &level1
};

int main()
{
	testAll(handlers);
	return 0;
}

