#include "helpers/test.h"
#include "helpers/assert.h"

#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/PredefinedString.h>

using namespace mfuse;

NamespaceDef namespace1("namespace1", "test");
NamespaceDef namespace2("namespace2", "test");

PredefinedString entString("ent");

const char scriptData1[] =
"local.test[0] = spawn InNamespace targetname ent\n"
"local.test[1] = spawn OutNamespace targetname ent\n"
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
}

int main()
{
	includeTest();
	excludeTest();
	return 0;
}

class InNamespace : public SimpleEntity {
public:
	MFUS_CLASS_PROTOTYPE(InNamespace);
};

class OutNamespace : public SimpleEntity {
public:
	MFUS_CLASS_PROTOTYPE(OutNamespace);
};

MFUS_CLASS_DECLARATION_NAMESPACE(namespace1, SimpleEntity, InNamespace, nullptr)
{
	{ nullptr, nullptr }
};

MFUS_CLASS_DECLARATION_NAMESPACE(namespace2, SimpleEntity, OutNamespace, nullptr)
{
	{ nullptr, nullptr }
};
