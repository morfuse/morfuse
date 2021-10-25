#include "helpers/archive.h"
#include "helpers/assert.h"

#include <morfuse/Script/TargetList.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/PredefinedString.h>
#include <morfuse/Script/Components/TargetComponent.h>

using namespace mfuse;

PredefinedString test0("test0");
PredefinedString test1("test1");
PredefinedString test2("test2");

void WriteTest(std::ostream& stream, const version_info_t& info)
{
	TargetList& list = ScriptContext::Get().GetTargetList();

	Archiver arc = Archiver::CreateWrite(stream, info);
	Listener l1, l2;
	TargetComponent comp1(l1);
	TargetComponent comp2(l2);

	{
		comp1.SetTargetName(test0.GetIndex());
		comp1.SetTargetName(test1.GetIndex());
		comp2.SetTargetName(test2.GetIndex());
		arc.ArchiveObject(comp1);
		arc.ArchiveObject(comp2);
		arc.ArchiveObject(l1);
		arc.ArchiveObject(l2);

		list.Archive(arc);
	}

	assertTest(!list.GetTarget(test0));
	assertTest(list.GetTarget(test1));
	assertTest(list.GetTarget(test2));
}

void ReadTest(std::istream& stream, const version_info_t& info)
{
	TargetList& list = ScriptContext::Get().GetTargetList();

	Listener l1, l2;
	TargetComponent comp1(l1);
	TargetComponent comp2(l2);

	{
		Archiver arc = Archiver::CreateRead(stream, info);
		arc.ArchiveObject(comp1);
		arc.ArchiveObject(comp2);

		assertTest(comp1.GetTargetName().GetConstString() == test1);
		assertTest(comp2.GetTargetName().GetConstString() == test2);

		arc.ArchiveObject(l1);
		arc.ArchiveObject(l2);

		list.Archive(arc);
	}

	assertTest(!list.GetTarget(test0));
	assertTest(list.GetTarget(test1));
	assertTest(list.GetTarget(test2));
}

int main()
{
	ScriptContext context;
	EventContext::Set(&context);

	TestArchive<2048>(&WriteTest, &ReadTest);

	return 0;
}
