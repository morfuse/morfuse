#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData1[] =
"local.org = (21.1 21 -4)\n"
"println local.org\n"
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

