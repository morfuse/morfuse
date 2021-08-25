#include <morfuse/Script/Context.h>

#include <fstream>
#include <iostream>

using namespace mfuse;

static const char scriptContent_level1[] =
"\n\n\n"
"{ println \"Hello, world\\n\"\n }\n"
"\t\tmain:\n\n\n\n"
"local.var = 1000\n"
"local.var += 200\n"
"local.var >>= 1\n"
"local.var = (local.var + 1) * 2 / 3 * 4 + 5\n"
"end local.var\n";

static const char scriptContent_level2[] =
"main:\n"
"\n\n\n\n\n\n"
"{\n"
"println \"Hello, world 2\\n\"\n"
//"local createListener\n"
"local.i = 1\n\n"
"local.j = \tslash/dot.ccc\t + test\t\n"
"if(1)\n { }\n else\n {\n \n}\n"
"}\n"
"end local.j\n\n\n";

static const char scriptContent_level3[] =
"main:\n"
"local.i = 1\n"
"switch(local.i)\n"
"{\n"
"case 0: break\n"
"case 1:\n"
"end 571\n"
"break\n"
"default: break\n"
"}\n";

static const char scriptContent_level4[] =
"main:\n"
"local.arr = makeArray\n"
"10 11 12\n"
"20 21 22\n"
"endArray\n"
"end local.arr\n";

static const char scriptContent_level5[] =
"main:\n"
"do {\n"
"local.val = 10\n"
"break\n"
"local.val = 2000\n"
"} while(0)\n"
"local.i = 1 | 2 | 4 | 8 & 16\n"
"end ((local.i + local.val) ^ 3)\n";

static const char scriptContent_level6[] =
"main1 local.i local.j local.k local.l:\n"
"local.val = local.i / local.j\n"
"end (local.val * local.k)\n";

static const char scriptContent_level7[] =
"main:\n"
"local.larr = local::local::local::local\n"
"local.larr println \"4 listeners\"\n"
"local.larr = local::local::local\n"
"local.larr println \"3 listeners\"\n"
"local.larr = local::local\n"
"local.larr println \"2 listeners\"\n"
"local.test1 = 1::2::3::4::5\n"
"local.test2 = 5::4::3::2::1\n"
"local.larr = local.test1::local.test2\n"
"for(local.i = 1; local.i <= local.larr.size; local.i++) {\n"
" local.b = local.larr[local.i]\n"
" println (local.b)\n"
" for(local.j = 1; local.j <= local.larr[local.i].size; local.j++) {\n"
"  println local.b[local.j]\n"
" }\n"
"}\n"
"end\n";

void level1(ScriptMaster& director)
{
	// Compile the script and return the game script object
	const ProgramScript* const script = director.GetScript("level1", scriptContent_level1, sizeof(scriptContent_level1));
	assert(script);

	// Execute the thread from the beginning of the script
	Event parms;
	director.ExecuteThread(script, nullptr, parms);

	uint32_t intValue = parms.GetInteger(1);
	assert(intValue == (((1000 + 200) >> 1) + 1) * 2 / 3 * 4 + 5);
}

void level2(ScriptMaster& director)
{
	const ProgramScript* const script = director.GetScript("level2", scriptContent_level2, sizeof(scriptContent_level2));
	assert(script);

	Event parms;
	director.ExecuteThread(script, nullptr, parms);

	const str retVal = parms.GetString(1);
	assert(!retVal.icmp("slash/dot.ccctest"));
}

void level3(ScriptMaster& director)
{
	const ProgramScript* const script = director.GetScript("level3", scriptContent_level3, sizeof(scriptContent_level3));
	assert(script);

	Event parms;
	director.ExecuteThread(script, nullptr, parms);

	const uint32_t intValue = parms.GetInteger(1);
	assert(intValue == 571);
}

void level4(ScriptMaster& director)
{
	const ProgramScript* const script = director.GetScript("level4", scriptContent_level4, sizeof(scriptContent_level4));
	assert(script);

	Event parms;
	director.ExecuteThread(script, nullptr, parms);

	ScriptVariable& var = parms.GetValue(1);
	ScriptVariable* row1 = var.constArrayValue()[1].constArrayValue();
	ScriptVariable* row2 = var.constArrayValue()[2].constArrayValue();
	assert(row1[1].intValue() == 10 && row1[2].intValue() == 11 && row1[3].intValue() == 12);
	assert(row2[1].intValue() == 20 && row2[2].intValue() == 21 && row2[3].intValue() == 22);
}

void level5(ScriptMaster& director)
{
	const ProgramScript* const script = director.GetScript("level5", scriptContent_level5, sizeof(scriptContent_level5));
	assert(script);

	Event parms;
	director.ExecuteThread(script, nullptr, parms);

	const uint32_t intValue = parms.GetInteger(1);
	assert(intValue == (((1 | 2 | 4 | 8 & 16) + 10) ^ 3));
}

void level6(ScriptMaster& director)
{
	const ProgramScript* const script = director.GetScript("level6", scriptContent_level6, sizeof(scriptContent_level6));
	assert(script);

	Event parms;
	parms.AddInteger(100);
	parms.AddInteger(30);
	parms.AddInteger(300);
	director.ExecuteThread(script, nullptr, parms);

	const uint32_t intValue = parms.GetInteger(parms.NumArgs());
	assert(intValue == ((100 / 30) * 300));
}

void level7(ScriptMaster& director)
{
	const ProgramScript* const script = director.GetScript("level7", scriptContent_level7, sizeof(scriptContent_level7));
	assert(script);

	director.ExecuteThread(script);
}

void m3l1a(ScriptMaster& director)
{
	std::fstream stream("m3l1a.scr", std::ios_base::in | std::ios_base::binary);
	if (!stream.is_open())
	{
		// FIXME
		return;
	}

	stream.seekg(0, stream.end);
	const size_t fsize = stream.tellg();
	stream.seekg(0, stream.beg);

	if (!fsize) {
		return;
	}

	char* scriptFileData = new char[fsize + 1];
	stream.read(scriptFileData, fsize);
	scriptFileData[fsize] = 0;
	stream.close();

	const ProgramScript* const script = director.GetScript("m3l1a", scriptFileData, fsize);

	delete[] scriptFileData;

	//assert(script);
	director.ExecuteThread(script);
}

int main(int argc, char* argv[])
{
	GlobalOutput::Get().SetOutputStream(outputLevel_e::Debug, &std::cout);
	GlobalOutput::Get().SetOutputStream(outputLevel_e::Warn, &std::cout);
	GlobalOutput::Get().SetOutputStream(outputLevel_e::Error, &std::cout);
	GlobalOutput::Get().SetOutputStream(outputLevel_e::Output, &std::cout);

	// Initialize the event system
	EventSystem::Get();

	ScriptContext context;

	context.GetOutputInfo().SetOutputStream(outputLevel_e::Debug, &std::cout);
	context.GetOutputInfo().SetOutputStream(outputLevel_e::Warn, &std::cout);
	context.GetOutputInfo().SetOutputStream(outputLevel_e::Error, &std::cout);
	context.GetOutputInfo().SetOutputStream(outputLevel_e::Output, &std::cout);

	ScriptSettings& settings = context.GetSettings();
	settings.SetDeveloperEnabled(false);

	// Get the script master
	ScriptMaster& director = context.GetDirector();

	level1(director);
	level2(director);
	level3(director);
	level4(director);
	level5(director);
	level6(director);
	level7(director);
	m3l1a(director);

	// Execute all pending events
	context.Execute();

	return 0;
}
