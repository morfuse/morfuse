#include <morfuse/Script/Context.h>
#include "helpers/assert.h"

using namespace mfuse;

void testSafeListeners(TrackedInstances& trInst, size_t numSafe, size_t numUnsafe)
{
	SafePtr<Listener>* listeners = numSafe ? new SafePtr<Listener>[numSafe] : nullptr;
	for (size_t i = 0; i < numSafe; i++)
	{
		Listener* l = new Listener();
		listeners[i] = l;
		trInst.Add(l);
	}

	for (size_t i = 0; i < numUnsafe; i++)
	{
		Listener* l = new Listener();
		trInst.Add(l);
	}

	assertTest(trInst.GetNumInstances() == numSafe + numUnsafe);
	// this should clear all unsafe listeners
	trInst.Cleanup();
	assertTest(trInst.GetNumInstances() == numSafe);

	if (listeners)
	{
		delete[] listeners;
		trInst.Cleanup();
		assertTest(trInst.GetNumInstances() == 0);
	}

	if (numSafe && numUnsafe)
	{
		listeners = new SafePtr<Listener>[numSafe];
		for (size_t i = 0; i < numSafe + numUnsafe; i++)
		{
			Listener* l = new Listener();
			if (i % 2)
			{
				// safe
				listeners[i / 2] = l;
			}

			trInst.Add(l);
		}

		assertTest(trInst.GetNumInstances() == numSafe + numUnsafe);
		// this should clear all unsafe listeners
		trInst.Cleanup();
		assertTest(trInst.GetNumInstances() == numSafe);

		if (listeners)
		{
			delete[] listeners;
			trInst.Cleanup();
			assertTest(trInst.GetNumInstances() == 0);
		}
	}
}

int main()
{
	ScriptContext context;
	TrackedInstances& trInst = context.GetTrackedInstances();

	for (size_t i = 0; i < 1000; i++)
	{
		Listener* l = new Listener();
		trInst.Add(l);
	}

	trInst.Cleanup();
	assertTest(!trInst.GetNumInstances());

	testSafeListeners(trInst, 10000, 0);
	testSafeListeners(trInst, 0, 10000);
	testSafeListeners(trInst, 10000, 10000);

	return 0;
}
