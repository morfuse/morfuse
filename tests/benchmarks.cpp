#include <morfuse/Script/Context.h>

#include <fstream>
#include <iostream>
#include <chrono>

using namespace mfuse;

static const char script1[] =
"main:\n"
"local.j = 1\n"
"local.k = 3\n"
"for(local.i = 0; local.i < 10000000; local.i++) {\n"
"local.j = local.j * local.k\n"
//"local.l = abs(local.j)\n"
//"local.st = \"test\"\n"
//"local.st2 = \"test\"\n"
//"local.st3 = \"test\"\n"
"local.k += 2\n"
"}\n"
"end\n";

static const char script2[] =
"main local.j local.k:\n"
"local.j = local.j * local.k\n"
"local.k += 2\n"
"local.arr[0] = local.j\n"
"local.arr[1] = local.k\n"
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
	const ProgramScript* const compiledScript1 = director.GetScript("bench1", script1, sizeof(script1));
	const ProgramScript* const compiledScript2 = director.GetScript("bench2", script2, sizeof(script2));

	using namespace std::chrono;
	time_point<high_resolution_clock> start, end;
	duration<double> diff1, diff2;

	start = high_resolution_clock::now();
	volatile size_t j = 1;
	volatile size_t k = 3;
	for(size_t i = 0; i < 10000000; ++i) {
		j = j * k;
		k += 2;
	}
	end = high_resolution_clock::now();
	diff1 = end - start;

	std::cout << "C code: " << duration<double>(diff1).count() * 1000.0 << " ms" << std::endl;

	start = high_resolution_clock::now();
	director.ExecuteThread(compiledScript1, nullptr);
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

	for (size_t i = 0; i < 10000000; ++i)
	{
		director.ExecuteThread(compiledScript2, nullptr, parms);

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
