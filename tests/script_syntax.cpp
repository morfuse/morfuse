#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptContent_level1[] =
"local.value = 1\n"
"println (value)local.value\n"
"local.arr[1] = 400\n"
"println local.arr[1]2\n"
"end\n";

const char scriptContent_level2[] =
"local.value = 1\n"
"assert(local   .    value == 1)\n"
"assert(local   .    \"value\" == 1)\n"
"println (\"value: \" + local.value)\n"
"local.3value = 3\n"
"assert(local.3value == 3)\n"
"assert(local.    3value == 3)\n"
"assert(local    .3value == 3)\n"
"end\n";

const char scriptContent_level3[] =
"main:\n"
"  local.end = 2\n"
"  local.value = local.end//test - value, with comment\n"
"  println (\"level3_value: \" + local.value)\n"
"  local end//test - value, with comment\n"
"  local.value = int end//test - value, with comment\n"
"  end//test - value, with comment\n";

void level1(ScriptMaster& director)
{
    const ProgramScript* const script = compile(director, "level1", scriptContent_level1);

    Event parms;
    director.ExecuteThread(script, parms);
}

void level2(ScriptMaster& director)
{
    const ProgramScript* const script = compile(director, "level2", scriptContent_level2);

    Event parms;
    director.ExecuteThread(script, parms);
}

void level3(ScriptMaster& director)
{
    const ProgramScript* const script = compile(director, "level3", scriptContent_level3);

    Event parms;
    director.ExecuteThread(script, parms);
}

handler_t handlers[] =
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
