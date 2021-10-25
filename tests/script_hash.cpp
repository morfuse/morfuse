#include "helpers/test.h"
#include "helpers/assert.h"

using namespace mfuse;

const char scriptData_level1[] =
"main local.l:"
"local.hash[\"test\"] = 1\n"
"local.hash[1] = 2\n"
"local.hash[local.l] = local.l\n"
"local.keys = getarraykeys local.hash\n"
"local.values = getarrayvalues local.hash\n"
"local.arr[0] = local.hash\n"
"local.arr[1] = local.keys\n"
"local.arr[2] = local.values\n"
"end local.arr";

template<typename T>
bool checkType(const ScriptVariable& var, T value);

template<>
bool checkType<Listener*>(const ScriptVariable& var, Listener* val)
{
	return var.listenerValue() == val;
}

template<>
bool checkType<const char*>(const ScriptVariable& var, const char* val)
{
	return var.stringValue() == val;
}

template<>
bool checkType<int>(const ScriptVariable& var, int val)
{
	return var.intValue() == val;
}

template<typename T>
bool findInArray(const ScriptVariable& arr, size_t num, variableType_e type, T value)
{
	for (size_t i = 1; i <= num; i++)
	{
		const ScriptVariable& var = arr[i];
		if (var.GetType() == type && checkType(var, value))
		{
			// found the matching type with value
			return true;
		}
	}

	return false;
}

void level1_innerloop(ScriptMaster& director, const ProgramScript* script)
{
	Listener* l = new Listener();
	Event parms;
	parms.AddListener(l);
	director.ExecuteThread(script, parms);

	const ScriptVariable& val = parms.GetValue(2);
	assertTest(val.GetType() == variableType_e::Array);

	const ScriptVariable* vars[] =
	{
		&val[ScriptVariable(0)],
		&val[ScriptVariable(1)],
		&val[ScriptVariable(2)]
	};

	assertTest(vars[0]->GetType() == variableType_e::Array);
	assertTest(vars[1]->GetType() == variableType_e::ConstArray);
	assertTest(vars[2]->GetType() == variableType_e::ConstArray);

	const ScriptVariable& arr = *vars[0];
	const ScriptVariable& keys = *vars[1];
	const ScriptVariable& values = *vars[2];

	const ScriptVariable* arrayValues[] =
	{
		&arr[ScriptVariable("test")],
		&arr[ScriptVariable(1)],
		&arr[ScriptVariable(l)]
	};

	assertTest(arrayValues[0]->GetType() == variableType_e::Integer);
	assertTest(arrayValues[0]->intValue() == 1);
	assertTest(arrayValues[1]->GetType() == variableType_e::Integer);
	assertTest(arrayValues[1]->intValue() == 2);
	assertTest(arrayValues[2]->GetType() == variableType_e::Listener);
	assertTest(arrayValues[2]->listenerValue() == l);

	assertTest(findInArray(keys, 3, variableType_e::Integer, 1));
	assertTest(findInArray(keys, 3, variableType_e::ConstString, "test"));
	assertTest(findInArray(keys, 3, variableType_e::Listener, l));
	assertTest(findInArray(values, 3, variableType_e::Integer, 1));
	assertTest(findInArray(values, 3, variableType_e::Integer, 2));
	assertTest(findInArray(values, 3, variableType_e::Listener, l));

	delete l;
}

void level1(ScriptMaster& director)
{
	const ProgramScript* const script = compile(director, "level1", scriptData_level1);

	for (size_t i = 0; i < 1000; i++)
	{
		level1_innerloop(director, script);
	}
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
