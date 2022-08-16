#include "helpers/archive.h"
#include "helpers/context.h"
#include "helpers/assert.h"

#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptVariable.h>

using namespace mfuse;

const char scriptData1[] =
"main:\n"
"println \"Hello\"\n\n"
"wait 1\n"
"println \"Hello 2\"\n";

class FileImpl : public IFile
{
public:
	FileImpl(const rawchar_t* scriptData, size_t size)
		: stream(scriptData, size)
	{

	}

	std::istream& getStream() noexcept override
	{
		return stream;
	}

private:
	imemstream stream;
};

class FileManagement : public IFileManagement
{
public:
	IFile* OpenFile(const char* fname) override
	{
		return new FileImpl(scriptData1, sizeof(scriptData1));
	}

	void CloseFile(IFile* file) noexcept override
	{
		delete file;
	}
};

void WriteTest(std::ostream& stream, const version_info_t& info)
{
	Archiver arc = Archiver::CreateWrite(stream, info);

	ScriptVariable stringVar;
	stringVar.setStringValue("test");

	ScriptVariable intVar;
	intVar.setIntValue(3);

	ScriptVariable floatVar;
	floatVar.setFloatValue(3.1f);

	ScriptVariable arrayVar;
	ScriptVariable arrayIndex;
	ScriptVariable arrayValue;
	arrayIndex.setIntValue(1);
	arrayValue.setFloatValue(10.f);
	arrayVar.setArrayAtRef(arrayIndex, arrayValue);

	stringVar.Archive(arc);
	intVar.Archive(arc);
	floatVar.Archive(arc);
	arrayVar.Archive(arc);

	ScriptContext& context = ScriptContext::Get();
	ScriptMaster& director = context.GetDirector();

	director.ExecuteThread("test");

	director.Archive(arc);
}

void ReadTest(std::istream& stream, const version_info_t& info)
{
	ScriptContext& context = ScriptContext::Get();
	ScriptMaster& director = context.GetDirector();
	director.Reset();
	{
		Archiver arc = Archiver::CreateRead(stream, info);

		ScriptVariable stringVar, intVar, floatVar, arrayVar;

		stringVar.Archive(arc);
		intVar.Archive(arc);
		floatVar.Archive(arc);
		arrayVar.Archive(arc);

		assertTest(stringVar.stringValue() == "test");
		assertTest(intVar.intValue() == 3);
		assertTest(floatVar.floatValue() == 3.1f);

		assertTest(arrayVar.arraysize() == 1);
		ScriptVariableIterator it(arrayVar);
		assertTest(it.GetKey().intValue() == 1);
		assertTest(it.GetValue().floatValue() == 10.f);

		director.Archive(arc);
	}

	context.Execute();
}

int main()
{
	ScriptContext context;
	setupContext(context);

	FileManagement fileManagement;
	context.GetScriptInterfaces().fileManagement = &fileManagement;

	TestArchive<16384>(&WriteTest, &ReadTest);

	return 0;
}
