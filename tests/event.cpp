#include <morfuse/Script/Event.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>

int main(int argc, const char* argv[])
{
	using namespace mfuse;

	EventSystem::Get();

	ScriptContext context;
	Listener* l = new Listener;
	
	for(size_t i = 0; i < 10000; ++i)
	{
		Event* ev = new Event(3);

		l->PostEvent(ev, 1000 - (i % 1000));
	}

	delete l;

	context.Execute();

	return 0;
}
