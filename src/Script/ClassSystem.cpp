#include <morfuse/Script/ClassSystem.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Class.h>
#include <morfuse/Common/OutputInfo.h>

#include <vector>

using namespace mfuse;

ClassSystem::ClassSystem()
	: amount(0)
	, numClassesBuilt(0)
{

}

ClassSystem& mfuse::ClassSystem::Get()
{
	static ClassSystem singleton;
	return singleton;
}

size_t ClassSystem::GetRequiredLength(size_t numEvents) const
{
	const size_t numClasses = ClassDef::GetNumClasses();
	return sizeof(const ResponseDefClass*) * numEvents * numClasses;
}

void ClassSystem::BuildEventResponses(MEM::PreAllocator& allocator)
{
	if(numClassesBuilt)
	{
		// Prepare for a rebuild
		ClearEventResponses(allocator);
	}

	amount = 0;
	numClassesBuilt = 0;

	for (ClassDef::List::iterator c = ClassDef::GetList(); c; c = c.Next())
	{
		c->BuildResponseList(allocator);

		amount += c->GetNumEvents() * sizeof(Response*);
		numClassesBuilt++;
	}

	ClassDefExt::InitClassDef();

	std::ostream* dbg = GlobalOutput::Get().GetOutput(outputLevel_e::Debug);
	if (dbg)
	{
		*dbg << "\n------------------\nEvent system initialized: "
			<< numClassesBuilt << " classes " << EventSystem::NumEventCommands() << " events " << allocator.Size() << " total bytes allocated" << std::endl;
	}
}

void ClassSystem::ClearEventResponses(MEM::PreAllocator& allocator)
{
	for (auto c = ClassDef::GetList(); c; c = c.Next())
	{
		c->ClearResponseList(allocator);
	}
}
