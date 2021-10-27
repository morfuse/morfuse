#include "helpers/test.h"
#include "helpers/assert.h"

#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/PredefinedString.h>

using namespace mfuse;

NamespaceDef namespace1("namespace1", "test");
NamespaceDef namespace2("namespace2", "test");

class InNamespace : public SimpleEntity {
public:
	MFUS_CLASS_PROTOTYPE(InNamespace);

	InNamespace()
		: wasCalled(false)
	{}

	void TestEvent1(Event& ev)
	{
		std::cout << "Successfully called" << std::endl;
		wasCalled = true;
	}

public:
	bool wasCalled;
};

class OutNamespace : public SimpleEntity {
public:
	MFUS_CLASS_PROTOTYPE(OutNamespace);

	void TestEvent2(Event& ev)
	{
		// shouldn't be called
		throw;
	}
};

PredefinedString entString("ent");

const char scriptData1[] =
"local.test[0] = spawn InNamespace targetname ent\n"
"local.test[1] = spawn OutNamespace targetname ent\n"
"local.test[0] test_event1\n"
"local.test[0] test_event2\n"
"if (local.test[1]) local.test[1] test_event2\n"
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

void includeTest()
{
	ScriptContext context;
	setupContext(context);

	NamespaceManager& nspaceMan = context.GetNamespaceManager();

	const NamespaceDef* defList[] =
	{
		&namespace1
	};

	nspaceMan.SetFilterMode(namespaceFilterMode_e::Inclusive);
	nspaceMan.SetFilteredNamespace(defList);

	executeHandlers(context, handlers);

	const TargetList& list = context.GetTargetList();
	const ConTarget* targets = list.GetExistingConstTargetList(entString);
	assertTest(targets);
	assertTest(targets->NumObjects() == 1);

	InNamespace* ent = static_cast<InNamespace*>(targets->ObjectAt(1).Pointer());
	assert(ent->wasCalled);
}

void excludeTest()
{
	ScriptContext context;
	setupContext(context);

	NamespaceManager& nspaceMan = context.GetNamespaceManager();

	const NamespaceDef* defList[] =
	{
		&namespace2
	};

	nspaceMan.SetFilterMode(namespaceFilterMode_e::Exclusive);
	nspaceMan.SetFilteredNamespace(defList);

	executeHandlers(context, handlers);

	const TargetList& list = context.GetTargetList();
	const ConTarget* targets = list.GetExistingConstTargetList(entString);
	assertTest(targets);
	assertTest(targets->NumObjects() == 1);

	InNamespace* ent = static_cast<InNamespace*>(targets->ObjectAt(1).Pointer());
	assert(ent->wasCalled);
}

int main()
{
	includeTest();
	excludeTest();
	return 0;
}

EventDef evInNamespace(
	namespace1,
	"test_event1",
	0,
	"",
	"",
	""
);

EventDef evOutNamespace(
	namespace2,
	"test_event2",
	0,
	"",
	"",
	""
);

MFUS_CLASS_DECLARATION_NAMESPACE(namespace1, SimpleEntity, InNamespace, nullptr)
{
	{ &evInNamespace, & InNamespace::TestEvent1 },
	{ nullptr, nullptr }
};

MFUS_CLASS_DECLARATION_NAMESPACE(namespace2, SimpleEntity, OutNamespace, nullptr)
{
	{ &evOutNamespace, & OutNamespace::TestEvent2 },
	{ nullptr, nullptr }
};
