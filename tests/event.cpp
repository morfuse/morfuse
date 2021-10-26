#include "helpers/assert.h"
#include <morfuse/Script/Event.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Common/str.h>
#include <morfuse/Container/Container.h>

using namespace mfuse;

struct TestNode {
	TestNode(const str& nameRef)
		: name(nameRef)
		, def1(name.c_str(), 0, "", "", "", evType_e::Normal)
		, def2(name.c_str(), 0, "", "", "", evType_e::Return)
	{

	}

	str name;
	EventDef def1;
	EventDef def2;
};

int main(int argc, const char* argv[])
{
	EventDef node1("ev1", 0, "", "", "", evType_e::Normal);
	EventDef node2("ev2", 0, "", "", "", evType_e::Normal);
	EventDef node3("ev3", 0, "", "", "", evType_e::Normal);
	EventDef node4("ev4", 0, "", "", "", evType_e::Normal);
	EventDef movedNode = std::move(node2);

	assertTest(node4.GetNext() == &node3);
	assertTest(node3.GetNext() == &movedNode);
	assertTest(movedNode.GetNext() == &node1);
	assertTest(node1.GetPrev() == &movedNode);
	assertTest(movedNode.GetPrev() == &node3);
	assertTest(node3.GetPrev() == &node4);

	assertTest(node2.GetNext() == nullptr);
	assertTest(node2.GetPrev() == nullptr);

	const size_t numNodes = 3005;
	con::Container<TestNode> nodes;
	nodes.Resize(numNodes);
	for (size_t i = 0; i < numNodes; i++)
	{
		TestNode* node = new (nodes) TestNode("event" + str(i));
	}

	new (nodes) TestNode("abs");
	new (nodes) TestNode("vector_length");
	new (nodes) TestNode("thread");
	new (nodes) TestNode("exec");
	new (nodes) TestNode("waitthread");

	EventSystem& evt = EventSystem::Get();

	for (size_t i = 0; i < numNodes; i++)
	{
		assertTest(evt.FindNormalEventNum(nodes[i].name.c_str()));
	}

	ScriptContext context;
	Listener* l = new Listener;
	
	for(size_t i = 0; i < 10000; ++i)
	{
		Event* ev = new Event(nodes[i % numNodes].def1.GetEventNum());

		l->PostEvent(ev, 1000 - (i % 1000));
	}

	delete l;

	context.Execute();

	return 0;
}
