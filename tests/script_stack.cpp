#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptContent_level1[] =
"array_to_int    local.array: end(waitthread _array_to local.array 0)\n"
"array_to_float  local.array: end(waitthread _array_to local.array 1)\n"
"array_to_str    local.array: end(waitthread _array_to local.array 2)\n"
"_array_to local.array local.cast:\n"
"local.i = (local.array[0] == NIL)\n"
"while (local.i <= local.array.size) {\n"
"switch (local.cast) {\n"
"case 0: local.array[local.i] = int      local.array[local.i] ; break\n"
"case 1: local.array[local.i] = float    local.array[local.i] ; break\n"
"case 2: local.array[local.i] = string   local.array[local.i] ; break\n"
"}\n"
"local.i++\n"
"}\n"
"end(local.array)\n";

void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptContent_level1);

	Event parms;
	director.ExecuteThread(script, parms);
}

handler_t handlers[] =
{
	{ "level1", &level1 }
};

int main()
{
	testAll(handlers);
	return 0;
}

