#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData1[] =
"local.a = 1::2\n"
"end local.a";

const char scriptData2[] =
"local.a = 1::2::3\n"
"end local.a;";

const char scriptData3[] =
"local.a = (1::2::3)::(4::5::6)::(7::8::9)\n"
"println local.a[1]\n"
"println local.a[2]\n"
"println local.a[3]\n"
"end local.a";

const char scriptData4[] =
"println a::b::c::d\n"
"println \"a\"::b::2\n"
"end";

void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptData1);

	Event parms;
	director.ExecuteThread(script, parms);

	assertTest(parms.NumArgs() == 1);
	const ScriptVariable& constArray = parms.GetValue(1);

	assertTest(constArray.constArrayElement(1).intValue() == 1);
	assertTest(constArray.constArrayElement(2).intValue() == 2);
}

void level2(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level2", scriptData2);

	Event parms;
	director.ExecuteThread(script, parms);

	assertTest(parms.NumArgs() == 1);
	const ScriptVariable& constArray = parms.GetValue(1);

	assertTest(constArray.constArrayElement(1).intValue() == 1);
	assertTest(constArray.constArrayElement(2).intValue() == 2);
	assertTest(constArray.constArrayElement(3).intValue() == 3);
}

void level3(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level3", scriptData3);

	Event parms;
	director.ExecuteThread(script, parms);

	assertTest(parms.NumArgs() == 1);

	const ScriptVariable& constArray = parms.GetValue(1);
	assertTest(constArray.GetType() == variableType_e::ConstArray);

	const ScriptVariable* subArray[] =
	{
		&constArray.constArrayElement(1),
		&constArray.constArrayElement(2),
		&constArray.constArrayElement(3)
	};

	assertTest(subArray[0]->size() == 3);
	assertTest(subArray[0]->constArrayElement(1).intValue() == 1);
	assertTest(subArray[0]->constArrayElement(2).intValue() == 2);
	assertTest(subArray[0]->constArrayElement(3).intValue() == 3);

	assertTest(subArray[1]->size() == 3);
	assertTest(subArray[1]->constArrayElement(1).intValue() == 4);
	assertTest(subArray[1]->constArrayElement(2).intValue() == 5);
	assertTest(subArray[1]->constArrayElement(3).intValue() == 6);

	assertTest(subArray[2]->size() == 3);
	assertTest(subArray[2]->constArrayElement(1).intValue() == 7);
	assertTest(subArray[2]->constArrayElement(2).intValue() == 8);
	assertTest(subArray[2]->constArrayElement(3).intValue() == 9);
}

void level4(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level4", scriptData1);

	Event parms;
	director.ExecuteThread(script, parms);
}

handler_t handlers[] =
{
	"level1", &level1,
	"level2", &level2,
	"level3", &level3,
	"level4", &level4
};

int main()
{
	testAll(handlers);
	return 0;
}

