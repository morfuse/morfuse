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

intptr_t EventNameHash::operator()(const const_xstr_static& key) const
{
	intptr_t hash = 0;

	for (decltype(key.c_str()) p = key.c_str(); *p; p++)
	{
		hash = hash * 31 + str::tolower(*p);
	}

	return hash;
}

bool EventNameCompare::operator()(const const_xstr_static& lhs, const const_xstr_static& rhs) const
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

	PreallocateMemory(requiredLength, numUniqueEvents, numUniqueNameEvents, numNormals, numReturns, numGetters, numSetters);

	LoadEvents();
	classSystem.BuildEventResponses(preAllocator);
	//ClearEventList();
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

	//constexpr size_t defListEntrySize = sizeof(Entry<eventNum_t, const EventDef*>);
	constexpr size_t defListEntrySize = sizeof(const EventDef*);
	constexpr size_t defNameEntrySize = sizeof(con::EntryArraySet<const_str_static, const_str_static>);
	constexpr size_t commandEntrySize = sizeof(con::Entry<eventName_t, eventInfo_t>);
	constexpr size_t normalEntrySize = sizeof(con::Entry<eventName_t, eventNum_t>);
	constexpr size_t returnEntrySize = sizeof(con::Entry<eventName_t, eventNum_t>);
	constexpr size_t getterEntrySize = sizeof(con::Entry<eventName_t, eventNum_t>);
	constexpr size_t setterEntrySize = sizeof(con::Entry<eventName_t, eventNum_t>);

	//const size_t defListSize = (defListEntrySize + sizeof(Entry<eventNum_t, const EventDef*>*)) * numUniqueEvents;
	const size_t defListSize = defListEntrySize * numUniqueEvents;
	const size_t defNameSize = (defNameEntrySize + sizeof(con::EntryArraySet<const_str_static, const_str_static>*) * 2) * numUniqueNameEvents;
	//const size_t commandSize = (normalEntrySize + sizeof(Entry<eventName_t, eventInfo_t>*)) * numUniqueEvents;
	const size_t commandSize = sizeof(eventInfo_t) * (numUniqueEvents + 1);
	const size_t normalSize = (normalEntrySize + sizeof(con::Entry<eventName_t, eventNum_t>*)) * numNormals;
	const size_t returnSize = (returnEntrySize + sizeof(con::Entry<eventName_t, eventNum_t>*)) * numReturns;
	const size_t getterSize = (getterEntrySize + sizeof(con::Entry<eventName_t, eventNum_t>*)) * numGetters;
	const size_t setterSize = (setterEntrySize + sizeof(con::Entry<eventName_t, eventNum_t>*)) * numSetters;

	return defListSize + defNameSize + commandSize; // + normalSize + returnSize + getterSize + setterSize;
}

void EventSystem::PreallocateMemory(size_t totalLength, size_t numUniqueEvents, size_t numUniqueNameEvents, size_t numNormals, size_t numReturns, size_t numGetters, size_t numSetters)
{
	preAllocator.PreAllocate(totalLength);

	//eventDefList.getAllocator().SetAllocator(preAllocator);
	eventDefName.getAllocator().SetAllocator(preAllocator);
	/*
	normalCommandList.getAllocator().SetAllocator(preAllocator);
	returnCommandList.getAllocator().SetAllocator(preAllocator);
	getterCommandList.getAllocator().SetAllocator(preAllocator);
	setterCommandList.getAllocator().SetAllocator(preAllocator);
	*/

	// resize sets to their corresponding size
	//eventDefList.resize(numUniqueEvents);
	eventDefList = new (preAllocator) const EventDef * [numUniqueEvents];
	commandList = new (preAllocator) eventInfo_t[numUniqueEvents + 1];
	eventDefName.resize(numUniqueNameEvents);
	/*
	normalCommandList.resize(numNormals);
	returnCommandList.resize(numReturns);
	getterCommandList.resize(numGetters);
	setterCommandList.resize(numSetters);
	*/

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

const EventDef *EventSystem::GetEventDef(eventNum_t eventNum)
{
	//const EventDef** pDef = eventDefList.findKeyValue(eventNum);
	//return pDef ? *pDef : nullptr;
	return eventNum <= EventSystem::NumEventCommands() ? eventDefList[eventNum - 1] : nullptr;
}

unsigned int EventSystem::GetEventFlags(eventNum_t eventnum)
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

const rawchar_t*EventSystem::GetEventName(eventNum_t eventnum)
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

const eventInfo_t& EventSystem::FindEventInfoChecked(eventName_t s)
{
	return commandList[s];
}

const eventInfo_t* EventSystem::FindEventInfo(eventName_t s)
{
	if (s > 0 && s < eventDefName.size())
	{
		return &commandList[s];
	}

	return nullptr;
}

const eventInfo_t* EventSystem::FindEventInfo(const rawchar_t* s)
{
	return FindEventInfo(GetEventConstName(s));
}

eventNum_t EventSystem::FindNormalEventNum(eventName_t s)
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

eventNum_t EventSystem::FindNormalEventNum(const rawchar_t* s)
{
	return FindEventInfoChecked(GetEventConstName(s)).normalNum;
}

eventNum_t EventSystem::FindReturnEventNum(eventName_t s)
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

eventNum_t EventSystem::FindReturnEventNum(const rawchar_t* s)
{
	return FindEventInfoChecked(GetEventConstName(s)).returnNum;
}

eventNum_t EventSystem::FindSetterEventNum(eventName_t s)
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

eventNum_t EventSystem::FindSetterEventNum(const rawchar_t* s)
{
	return FindEventInfoChecked(GetEventConstName(s)).setterNum;
}

eventNum_t EventSystem::FindGetterEventNum(eventName_t s)
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

eventNum_t EventSystem::FindGetterEventNum(const rawchar_t* s)
{
	return FindEventInfoChecked(GetEventConstName(s)).getterNum;
}

#define MAX_INHERITANCE 64
void EventSystem::ClassEvents(const rawchar_t*classname, bool print_to_disk)
{
// FIXME
#if 0
	ClassDef *c;
	ResponseDef<BaseScriptClass> *r;
	uintptr_t ev;
	uintptr_t i;
	intptr_t j;
	bool *set;
	size_t num;
	uint8_t orderNum;
	EventDef **events;
	uint8_t *order;
	FILE *class_file;
	str classNames[MAX_INHERITANCE];
	str class_filename;

	c = GetClass(classname);
	if (!c)
	{
		//CLASS_DPrintf("Unknown class: %s\n", classname);
		return;
	}

	class_file = NULL;

	if (print_to_disk)
	{
		class_filename = str(classname) + ".txt";
		class_file = fopen(class_filename.c_str(), "w");
		if (class_file == NULL)
			return;
	}

	num = NumEventCommands();

	set = new bool[num];
	memset(set, 0, sizeof(bool) * num);

	events = new EventDef *[num];
	memset(events, 0, sizeof(Event *) * num);

	order = new uint8_t[num];
	memset(order, 0, sizeof(uint8_t) * num);

	orderNum = 0;
	for (; c != NULL; c = c->GetSuper())
	{
		if (orderNum < MAX_INHERITANCE)
		{
			classNames[orderNum] = c->GetClassName();
		}
		r = c->GetResponseList();
		if (r)
		{
			for (i = 0; r[i].event != NULL; i++)
			{
				ev = (int)r[i].event->GetEventNum();
				if (!set[ev])
				{
					set[ev] = true;

					if (r[i].response)
					{
						events[ev] = r[i].event;
						order[ev] = orderNum;
					}
				}
			}
		}
		orderNum++;
	}

	CLASS_Print(class_file, "********************************************************\n");
	CLASS_Print(class_file, "********************************************************\n");
	CLASS_Print(class_file, "* All Events For Class: %s\n", classname);
	CLASS_Print(class_file, "********************************************************\n");
	CLASS_Print(class_file, "********************************************************\n\n");

	for (j = orderNum - 1; j >= 0; j--)
	{
		CLASS_Print(class_file, "\n********************************************************\n");
		CLASS_Print(class_file, "* Class: %s\n", classNames[j].c_str());
		CLASS_Print(class_file, "********************************************************\n\n");
		for (i = 0; i < num; i++)
		{
			intptr_t index;

			index = ClassDef::sortedList.at(i);
			if (events[index] && (order[index] == j))
			{
				const EventDef** pDef = eventDefList.findKeyValue(events[index]->GetEventNum());
				if(pDef && *pDef)
				{
					const EventDef* def = *pDef;
					// FIXME
					//def->PrintEventDocumentation(class_file, false);
				}
			}
		}
	}

	if (class_file != NULL)
	{
		//CLASS_DPrintf("Printed class info to file %s\n", class_filename.c_str());
		fclose(class_file);
	}

	delete[] events;
	delete[] order;
	delete[] set;
#endif
}

void EventSystem::DumpClass(FILE * class_file, const rawchar_t* className)
{
// FIXME
#if 0
	ClassDef		*c;
	ResponseDef<BaseScriptClass> *r;
	int			ev;
	size_t		num, num2;
	EventDef    **events;

	c = GetClass(className);
	if (!c)
	{
		return;
	}

	num = eventDefName.size();
	num2 = NumEventCommands();

	events = new EventDef *[num2];
	memset(events, 0, sizeof(Event *) * num2);

	// gather event responses for this class
	r = c->responses;
	if (r)
	{
		for (size_t i = 0; r[i].event != NULL; i++)
		{
			ev = (int)r[i].event->GetEventNum();
			if (r[i].response)
			{
				events[ev] = r[i].event;
			}
		}
	}

	CLASS_Print(class_file, "\n");
	if (c->classID[0])
	{
		CLASS_Print(class_file, "<h2> <a name=\"%s\">%s (<i>%s</i>)</a>", c->classname, c->classname, c->classID);
	}
	else
	{
		CLASS_Print(class_file, "<h2> <a name=\"%s\">%s</a>", c->classname, c->classname);
	}

	// print out lineage
	for (c = c->super; c != NULL; c = c->super)
	{
		CLASS_Print(class_file, " -> <a href=\"#%s\">%s</a>", c->classname, c->classname);
	}
	CLASS_Print(class_file, "</h2>\n");

	ClassDef::dump_numclasses++;

	CLASS_Print(class_file, "<BLOCKQUOTE>\n");
	for (size_t i = 0; i < num; i++)
	{
		intptr_t index;

		index = ClassDef::sortedList.at(i);
		if (events[index])
		{
			// FIXME
			//eventDefList.findKeyValue(events[index]->GetEventNum())->PrintEventDocumentation(class_file, true);
			ClassDef::dump_numevents++;
		}
	}
	CLASS_Print(class_file, "</BLOCKQUOTE>\n");
	delete[] events;
#endif
}


#define MAX_CLASSES 1024
void EventSystem::DumpAllClasses()
{
// FIXME
#if 0
	size_t i, num;
	ClassDef *c;
	FILE * class_file;
	str class_filename;
	str class_title;
	str classes[MAX_CLASSES];

#if defined( GAME_DLL )
	class_filename = "g_allclasses.html";
	class_title = "Game Module";
#elif defined( CGAME_DLL )
	class_filename = "cg_allclasses.html";
	class_title = "Client Game Module";
#else
	class_filename = "cl_allclasses.html";
	class_title = "Client Module";
#endif

	class_file = fopen(class_filename.c_str(), "w");
	if (class_file == NULL)
		return;

	// construct the HTML header for the document
	CLASS_Print(class_file, "<HTML>\n");
	CLASS_Print(class_file, "<HEAD>\n");
	CLASS_Print(class_file, "<Title>%s Classes</Title>\n", class_title.c_str());
	CLASS_Print(class_file, "</HEAD>\n");
	CLASS_Print(class_file, "<BODY>\n");
	CLASS_Print(class_file, "<H1>\n");
	CLASS_Print(class_file, "<center>%s Classes</center>\n", class_title.c_str());
	CLASS_Print(class_file, "</H1>\n");
#if defined( GAME_DLL )
	//
	// print out some commonly used classnames
	//
	CLASS_Print(class_file, "<h2>");
	CLASS_Print(class_file, "<a href=\"#Actor\">Actor</a>, ");
	CLASS_Print(class_file, "<a href=\"#Animate\">Animate</a>, ");
	CLASS_Print(class_file, "<a href=\"#Entity\">Entity</a>, ");
	CLASS_Print(class_file, "<a href=\"#ScriptSlave\">ScriptSlave</a>, ");
	CLASS_Print(class_file, "<a href=\"#ScriptThread\">ScriptThread</a>, ");
	CLASS_Print(class_file, "<a href=\"#Sentient\">Sentient</a>, ");
	CLASS_Print(class_file, "<a href=\"#StateMap\">StateMap</a>, ");
	CLASS_Print(class_file, "<a href=\"#Trigger\">Trigger</a>, ");
	CLASS_Print(class_file, "<a href=\"#World\">World</a>");
	CLASS_Print(class_file, "</h2>");
#endif

	ClassDef::dump_numclasses = 0;
	ClassDef::dump_numevents = 0;

	Container<uintptr_t> sortedList;
	Container<ClassDef*> sortedClassList;

	ClassDef::sortedList.clear();
	ClassDef::sortedClassList.clear();

	SortEventList(&sortedList);
	ClassDef::SortClassList(&sortedClassList);

	num = ClassDef::sortedClassList.size();

	// go through and process each class from smallest to greatest
	for (i = 0; i < num; i++)
	{
		c = ClassDef::sortedClassList.at(i);
		DumpClass(class_file, c->classname);
	}

	if (class_file != NULL)
	{
		CLASS_Print(class_file, "<H2>\n");
		CLASS_Print(class_file, "%d %s Classes.<BR>%d %s Events.\n", ClassDef::dump_numclasses, class_title.c_str(), ClassDef::dump_numevents, class_title.c_str());
		CLASS_Print(class_file, "</H2>\n");
		CLASS_Print(class_file, "</BODY>\n");
		CLASS_Print(class_file, "</HTML>\n");
		//CLASS_DPrintf("Dumped all classes to file %s\n", class_filename.c_str());
		fclose(class_file);
	}
#endif
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
