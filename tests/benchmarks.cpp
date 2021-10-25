#include "helpers/assert.h"
#include <morfuse/Script/Context.h>
#include <morfuse/Common/membuf.h>

#include <fstream>
#include <iostream>
#include <chrono>

using namespace mfuse;

static const char script1[] =
"main:\n"
"local.j = 1\n"
"local.k = 3\n"
"for(local.i = 0; local.i < 100000; local.i++) {\n"
"local.j = local.j * local.k\n"
"local.k += 2\n"
"}\n"
"end\n";

static const char script2[] =
"main local.j local.k:\n"
"local.j = local.j * local.k\n"
"local.k += 2\n"
"end\n";

int main(int argc, char* argv[])
{
	GlobalOutput::Get().SetOutputStream(outputLevel_e::Debug, &std::cout);

	EventSystem::Get();

	ScriptContext context;

	context.GetOutputInfo().SetOutputStream(outputLevel_e::Debug, &std::cout);

	ScriptSettings& settings = context.GetSettings();
	settings.SetDeveloperEnabled(false);

	// Get the script master
	ScriptMaster& director = context.GetDirector();
	imemstream stream[] =
	{
		{ script1, sizeof(script1) },
		{ script2, sizeof(script2) }
	};

	director.GetThreadExecutionProtection().SetLoopProtection(false);

	const ProgramScript* const compiledScript1 = director.GetProgramScript("bench1", stream[0]);
	const ProgramScript* const compiledScript2 = director.GetProgramScript("bench2", stream[1]);

	using namespace std::chrono;
	time_point<high_resolution_clock> start, end;
	duration<double> diff1, diff2;

	start = high_resolution_clock::now();
	// avoid optimizing away those variables
	// the loop must be done at run-time
	volatile uint32_t j = 1;
	volatile uint32_t k = 3;
	for(uint32_t i = 0; i < 100000; ++i) {
		j = j * k;
		k += 2;
	}
	end = high_resolution_clock::now();
	diff1 = end - start;

	std::cout << "C code: " << duration<double>(diff1).count() * 1000.0 << " ms" << std::endl;

	start = high_resolution_clock::now();
	director.ExecuteThread(compiledScript1);
	end = high_resolution_clock::now();

	diff2 = end - start;
	std::cout << "script code: " << duration<double>(diff2).count() * 1000.0 << " ms" << std::endl;

	const double slowness = (diff2 / diff1);

	std::cout << "script is " << slowness << "x slower" << std::endl;

	ScriptVariable val0, val1;
	val0.setIntValue(0);
	val1.setIntValue(1);

	start = high_resolution_clock::now();
	j = 1;
	k = 3;
	Event parms;
	parms.ReserveArguments(3);
	parms.AddInteger(j);
	parms.AddInteger(k);

	for (uint32_t i = 0; i < 100000; ++i)
	{
		director.ExecuteThread(compiledScript2, parms);

		/*
		ScriptVariable& arr = parms.GetValue(parms.NumArgs());
		const ScriptVariable& jVar = arr[val0];
		const ScriptVariable& kVar = arr[val1];

		j = jVar.intValue();
		k = kVar.intValue();

		parms.Clear();
		*/
	}
	end = high_resolution_clock::now();
	diff1 = end - start;
	std::cout << "C mixed with script code: " << duration<double>(diff1).count() * 1000.0 << " ms" << std::endl;
}
