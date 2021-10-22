#include <morfuse/Script/ClassSystem.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/Event.h>
#include <morfuse/Script/ScriptVariable.h>

#include <algorithm>
#include <string>

using namespace mfuse;

size_t EventSystem::numEvents;

eventInfo_t::eventInfo_t()
	: normalNum(0)
	, returnNum(0)
	, setterNum(0)
	, getterNum(0)
{
}

intptr_t EventNameHash::operator()(const const_str_static& key) const
{
	intptr_t hash = 0;

	for (decltype(key.c_str()) p = key.c_str(); *p; p++)
	{
		hash = hash * 31 + str::tolower(*p);
	}

	return hash;
}

bool EventNameCompare::operator()(const const_str_static& lhs, const const_str_static& rhs) const
{
	return !str::icmp(lhs.c_str(), rhs.c_str());
}

EventSystem::EventSystem()
{
	InitEvents();
}

EventSystem::~EventSystem()
{
	if (EventSystemStarted())
	{
		ClassSystem::Get().ClearEventResponses(preAllocator);
		//ClearEventList();
		UnloadEvents();
	}
}

bool EventSystem::EventSystemStarted()
{
	return bEventSystemStarted;
}

void EventSystem::InitEvents()
{
	if(EventSystemStarted())
	{
		// when rebuilding erase the previous list
		// and perform deallocations
		UnloadEvents();
	}

	size_t numUniqueEvents;
	size_t numUniqueNameEvents;
	size_t numNormals, numReturns, numGetters , numSetters;
	size_t requiredLength = GetRequiredLength(numUniqueEvents, numUniqueNameEvents, numNormals, numReturns, numGetters, numSetters);

	ClassSystem& classSystem = ClassSystem::Get();
	requiredLength += classSystem.GetRequiredLength(numUniqueEvents);

	PreallocateMemory(requiredLength, numUniqueEvents, numUniqueNameEvents);

	LoadEvents();
	assert(eventDefName.size() == numUniqueNameEvents);
	// build event methods for classes
	classSystem.BuildEventResponses(preAllocator);
	bEventSystemStarted = true;
}

size_t EventSystem::GetRequiredLength(size_t& numUniqueEvents, size_t& numUniqueNameEvents, size_t& numNormals, size_t& numReturns, size_t& numGetters, size_t& numSetters) const
{
	// events with unique name and type
	numUniqueEvents = 0;
	// number of unique strings
	numUniqueNameEvents = 0;
	numNormals = 0, numReturns = 0, numGetters = 0, numSetters = 0;

	std::ostream* warn = GlobalOutput::Get().GetOutput(outputLevel_e::Warn);

	// count the number of events with same name
	// and also count the number of events that are exactly the same (name and type)
	for (const EventDef* e1 = EventDef::GetHead(); e1; e1 = e1->GetNext())
	{
		bool bSame = false;
		bool bSameType = false;

		for (const EventDef* e2 = e1->GetNext(); e2; e2 = e2->GetNext())
		{
			const EventDefAttributes& attributes1 = e1->GetAttributes();
			const EventDefAttributes& attributes2 = e2->GetAttributes();

			if (!str::icmp(attributes1.GetString(), attributes2.GetString()))
			{
				bSame = true;
				if (attributes1.GetNum() == attributes2.GetNum())
				{
					bSameType = true;
					if (warn)
					{
						*warn << "Event '" << attributes1.GetString() << "' has duplicates. This is not an error but it's better to remove it to avoid eating memory.";
					}
				}
				break;
			}
		}

		if (!bSame) {
			++numUniqueNameEvents;
		}

		if (!bSameType)
		{
			switch (e1->GetAttributes().GetType())
			{
			case evType_e::Normal:
				++numNormals;
				break;
			case evType_e::Return:
				++numReturns;
				break;
			case evType_e::Getter:
				++numGetters;
				break;
			case evType_e::Setter:
				++numSetters;
				break;
			default:
				break;
			}

			++numUniqueEvents;
		}
	}

	constexpr size_t defListEntrySize = sizeof(const EventDef*);
	constexpr size_t defNameEntrySize = sizeof(con::EntryArraySet<const_str_static, const_str_static>);

	const size_t defListSize = defListEntrySize * numUniqueEvents;
	const size_t defNameSize = (defNameEntrySize + sizeof(con::EntryArraySet<const_str_static, const_str_static>*) * 2) * numUniqueNameEvents;
	const size_t commandSize = sizeof(eventInfo_t) * (numUniqueEvents + 1);

	return defListSize + defNameSize + commandSize;
}

void EventSystem::PreallocateMemory(size_t totalLength, size_t numUniqueEvents, size_t numUniqueNameEvents)
{
	preAllocator.PreAllocate(totalLength);

	eventDefName.getAllocator().SetAllocator(preAllocator);

	// resize sets to their corresponding size
	//eventDefList.resize(numUniqueEvents);
	eventDefList = new (preAllocator) const EventDef * [numUniqueEvents];
	commandList = new (preAllocator) eventInfo_t[numUniqueEvents + 1];
	eventDefName.resize(numUniqueNameEvents);

	numEvents = numUniqueEvents;
}

void EventSystem::LoadEvents()
{
	for(const EventDef* e = EventDef::GetHead(); e; e = e->GetNext())
	{
		const EventDefAttributes& attributes = e->GetAttributes();
		const eventNum_t eventNum = attributes.GetNum();

		//eventDefList.addKeyValue(e->GetEventNum(), e);
		eventDefList[eventNum - 1] = e;

		const eventName_t index = eventDefName.addKeyIndex(attributes.GetString());

		if (attributes.GetType() != evType_e::None)
		{
			eventInfo_t& info = commandList[index];

			switch (attributes.GetType())
			{
			case evType_e::Normal:
				//normalCommandList[index] = eventNum;
				info.normalNum = eventNum;
				break;
			case evType_e::Return:
				//returnCommandList[index] = eventNum;
				info.returnNum = eventNum;
				break;
			case evType_e::Getter:
				//getterCommandList[index] = eventNum;
				info.getterNum = eventNum;
				break;
			case evType_e::Setter:
				//setterCommandList[index] = eventNum;
				info.setterNum = eventNum;
				break;
			default:
				break;
			}
		}
	}
}

void EventSystem::UnloadEvents()
{

	//commandList.clear();
	for(size_t i = 0; i < numEvents; ++i) {
		commandList[i].~eventInfo_t();
	}

	eventDefName.clear();
	//eventDefList.clear();

	/*
	normalCommandList.clear();
	returnCommandList.clear();
	getterCommandList.clear();
	setterCommandList.clear();
	*/
	numEvents = 0;

	// release memory
	preAllocator.Release();
}

size_t EventSystem::NumEventCommands()
{
	return numEvents;
}

void EventSystem::ShutdownEvents()
{
	if (!bEventSystemStarted) {
		return;
	}

	//ClearEventList();
	UnloadEvents();

	bEventSystemStarted = false;
}

/*
void EventManager::ArchiveEvents(Archiver &arc)
{
	EventQueueNode *event;
	int num;

	num = 0;
	for (event = EventQueue.next; event != &EventQueue; event = event->next)
	{
		Listener * obj;

		assert(event);

		obj = event->GetSourceObject();

		assert(obj);

#if defined ( GAME_DLL )
		if (obj->isSubclassOf(Entity) &&
			(((Entity *)obj)->flags & FL_DONTSAVE))
		{
			continue;
		}
#endif

		num++;
	}

	arc.ArchiveInteger(&num);
	for (event = EventQueue.next; event != &EventQueue; event = event->next)
	{
		Listener * obj;

		assert(event);

		obj = event->GetSourceObject();

		assert(obj);

#if defined ( GAME_DLL )
		if (obj->isSubclassOf(Entity) &&
			(((Entity *)obj)->flags & FL_DONTSAVE))
		{
			continue;
		}
#endif

		event->event->Archive(arc);
		arc.ArchiveInteger(&event->inttime);
		arc.ArchiveInteger(&event->flags);
		arc.ArchiveSafePointer(&event->m_sourceobject);
	}
}

void EventManager::UnarchiveEvents(Archiver &arc)
{
	EventQueueNode *node;
	Event *e;
	int i, numEvents;

	// the FreeEvents list would already be allocated at this point
	// clear out any events that may exist
	L_ClearEventList();

	arc.ArchiveInteger(&numEvents);
	for (i = 0; i < numEvents; i++)
	{
		node = new EventQueueNode();
		e = new Event();
		e->Archive(arc);

		arc.ArchiveInteger(&node->inttime);
		arc.ArchiveInteger(&node->flags);
		arc.ArchiveSafePointer(&node->m_sourceobject);

		LL_Add(&EventQueue, node, next, prev);
	}
}
*/

const EventDef *EventSystem::GetEventDef(eventNum_t eventNum) const
{
	//const EventDef** pDef = eventDefList.findKeyValue(eventNum);
	//return pDef ? *pDef : nullptr;
	return eventNum <= EventSystem::NumEventCommands() ? eventDefList[eventNum - 1] : nullptr;
}

unsigned int EventSystem::GetEventFlags(eventNum_t eventnum) const
{
	const EventDef *cmd = GetEventDef(eventnum);

	if (cmd)
	{
		return cmd->flags;
	}
	else
	{
		return 0;
	}

	return 0;
}

const rawchar_t*EventSystem::GetEventName(eventNum_t eventnum) const
{
	const EventDef* cmd = GetEventDef(eventnum);

	if (cmd)
	{
		return cmd->GetAttributes().GetString();
	}
	else
	{
		return "";
	}

	return "";
}

const eventInfo_t& EventSystem::FindEventInfoChecked(eventName_t s) const
{
	return commandList[s];
}

const eventInfo_t* EventSystem::FindEventInfo(eventName_t s) const
{
	if (s > 0 && s < eventDefName.size())
	{
		return &commandList[s];
	}

	return nullptr;
}

const eventInfo_t* EventSystem::FindEventInfo(const rawchar_t* s) const
{
	return FindEventInfo(GetEventConstName(s));
}

eventNum_t EventSystem::FindNormalEventNum(eventName_t s) const
{
	const eventInfo_t* eventInfo = FindEventInfo(s);
	if (eventInfo)
	{
		return eventInfo->normalNum;
	}
	else
	{
		return 0;
	}
}

eventNum_t EventSystem::FindNormalEventNum(const rawchar_t* s) const
{
	return FindEventInfoChecked(GetEventConstName(s)).normalNum;
}

eventNum_t EventSystem::FindReturnEventNum(eventName_t s) const
{
	const eventInfo_t* eventInfo = FindEventInfo(s);
	if (eventInfo)
	{
		return eventInfo->returnNum;
	}
	else
	{
		return 0;
	}
}

eventNum_t EventSystem::FindReturnEventNum(const rawchar_t* s) const
{
	return FindEventInfoChecked(GetEventConstName(s)).returnNum;
}

eventNum_t EventSystem::FindSetterEventNum(eventName_t s) const
{
	const eventInfo_t* eventInfo = FindEventInfo(s);
	if (eventInfo)
	{
		return eventInfo->setterNum;
	}
	else
	{
		return 0;
	}
}

eventNum_t EventSystem::FindSetterEventNum(const rawchar_t* s) const
{
	return FindEventInfoChecked(GetEventConstName(s)).setterNum;
}

eventNum_t EventSystem::FindGetterEventNum(eventName_t s) const
{
	const eventInfo_t* eventInfo = FindEventInfo(s);
	if (eventInfo)
	{
		return eventInfo->getterNum;
	}
	else
	{
		return 0;
	}
}

eventNum_t EventSystem::FindGetterEventNum(const rawchar_t* s) const
{
	return FindEventInfoChecked(GetEventConstName(s)).getterNum;
}

eventName_t EventSystem::GetEventConstName(const rawchar_t* name) const
{
	return eventDefName.findKeyIndex(name);
}

EventSystem& EventSystem::Get()
{
	static EventSystem singleton;
	return singleton;
}
