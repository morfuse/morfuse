#include <morfuse/Script/Event.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Container/ContainerView.h>
#include <cstdarg>
#include <string>

using namespace mfuse;

EventDef::List EventDef::head = EventDef::List();
size_t EventDef::defCount = 0;

//static MEM_BlockAllocSafe<Event, MEM_DefaultBlock> Event_allocator;

template<>
intptr_t Hash<EventDefAttributes>::operator()(const EventDefAttributes& key) const
{
	intptr_t hash = 0;

	for (const rawchar_t* p = key.GetString(); *p != 0; p++)
	{
		hash = str::tolower(*p) + 31 * hash;
	}

	return hash;
}

EventDefAttributes::EventDefAttributes()
	: name(nullptr)
	, type(evType_e::None)
{
}

EventDefAttributes::EventDefAttributes(const rawchar_t* inName, evType_e inType, eventNum_t inNum)
	: name(inName)
	, eventNum(inNum)
	, type(inType)
{
}

EventDefAttributes::EventDefAttributes(const EventDefAttributes& other)
	: name(other.name)
	, eventNum(other.eventNum)
	, type(other.type)
{
}

EventDefAttributes& EventDefAttributes::operator=(const EventDefAttributes& other)
{
	name = other.name;
	eventNum = other.eventNum;
	type = other.type;
	return *this;
}

EventDefAttributes::EventDefAttributes(EventDefAttributes&& other)
	: name(other.name)
	, eventNum(other.eventNum)
	, type(other.type)
{
	other.name = nullptr;
	other.eventNum = 0;
	other.type = evType_e::None;
}

EventDefAttributes& EventDefAttributes::operator=(EventDefAttributes&& other)
{
	name = other.name;
	eventNum = other.eventNum;
	type = other.type;
	other.name = nullptr;
	other.eventNum = 0;
	other.type = evType_e::None;
	return *this;
}

const rawchar_t* EventDefAttributes::GetString() const
{
	return name;
}

eventNum_t EventDefAttributes::GetNum() const
{
	return eventNum;
}

evType_e EventDefAttributes::GetType() const
{
	return type;
}

bool EventDefAttributes::operator==(const rawchar_t* inName) const
{
	return !str::icmp(inName, name);
}

bool EventDefAttributes::operator!=(const rawchar_t* inName) const
{
	return str::icmp(inName, name) != 0;
}

bool EventDefAttributes::operator==(const EventDefAttributes& other) const
{
	return *this == other.name && (other.type == evType_e::None || other.type == type);
}

bool EventDefAttributes::operator!=(const EventDefAttributes& other) const
{
	return !(*this == other);
}

EventDef::EventDef()
{
}

EventDef::EventDef(const rawchar_t* command, uint32_t flags, const rawchar_t* formatspec, const rawchar_t* argument_names, const rawchar_t* documentation, evType_e type)
	: EventDef(nullptr, command, flags, formatspec, argument_names, documentation, type)
{
}

EventDef::EventDef(const ModuleDef* def, const rawchar_t* command, uint32_t flags, const rawchar_t* formatspec, const rawchar_t* argument_names, const rawchar_t* documentation, evType_e type)
	: moduleDef(def)
	, attributes(GetNewAttributes(command, type))
{
	this->flags = flags;
	this->formatspec = formatspec;
	this->argument_names = argument_names;
	this->documentation = documentation;

	if (argument_names)
	{
		// get the number of args
		minArgs = 1;
		const rawchar_t* p = argument_names;
		while (*p)
		{
			if (*p == ' ')
			{
				++minArgs;
				while (*p++ == ' ');
			}
			else
			{
				p++;
			}
		}
	}
	else {
		minArgs = 0;
	}
}

EventDef::EventDef(EventDef&& other)
	: moduleDef(other.moduleDef)
	, attributes(std::move(other.attributes))
	, formatspec(other.formatspec)
	, argument_names(other.argument_names)
	, documentation(other.documentation)
	, minArgs(other.minArgs)
	, flags(other.flags)
{
	prev = next = nullptr;
	head.Move(this, &other);
}

EventDef& EventDef::operator=(EventDef&& other)
{
	moduleDef = other.moduleDef;
	attributes = std::move(other.attributes);
	formatspec = other.formatspec;
	argument_names = other.argument_names;
	documentation = other.documentation;
	minArgs = other.minArgs;
	flags = other.flags;

	head.Move(this, &other);

	return *this;
}

EventDef::~EventDef()
{
	head.Remove(this);

	if (GetAttributes().GetType() != evType_e::None) {
		--defCount;
	}
}

EventDefAttributes EventDef::GetNewAttributes(const rawchar_t* command, evType_e type)
{
	// try to find another event's attributes with similar name and type
	// so that there is no conflict
	for (const EventDef* e = GetHead(); e; e = e->GetNext())
	{
		const EventDefAttributes& eAttr = e->GetAttributes();
		if (!str::icmp(eAttr.GetString(), command) && eAttr.GetType() == type)
		{
			next = prev = nullptr;
			//head.AddFirst(this);
			// use the matching event's attributes
			return eAttr;
		}
	}
	// add the event
	head.AddFirst(this);
	// create a new event with unique index
	return EventDefAttributes(command, type, ++defCount);
}

bool EventDef::operator==(const rawchar_t* name) const
{
	return attributes == name;
}

bool EventDef::operator!=(const rawchar_t* name) const
{
	return attributes != name;
}

bool EventDef::operator==(const EventDef& other) const
{
	return other.attributes == attributes;
}

bool EventDef::operator!=(const EventDef& other) const
{
	return other.attributes != attributes;
}

const EventDefAttributes& EventDef::GetAttributes() const
{
	return attributes;
}

eventNum_t EventDef::GetEventNum() const
{
	return attributes.GetNum();
}

size_t EventDef::GetDefCount()
{
	return defCount;
}

EventDef* EventDef::GetNext() const
{
	return next;
}

EventDef* EventDef::GetPrev() const
{
	return prev;
}

const EventDef* EventDef::GetHead()
{
	return head.Root();
}

EventDef::List::iterator EventDef::GetList()
{
	return head.CreateConstIterator();
}

uint32_t EventDef::GetFlags() const
{
	return flags;
}

const rawchar_t* EventDef::GetFormatSpec() const
{
	return formatspec;
}

const rawchar_t* EventDef::GetArgumentNames() const
{
	return argument_names;
}

const rawchar_t* EventDef::GetDocumentation() const
{
	return documentation;
}

MFUS_CLASS_DECLARATION(Class, Event, NULL)
{
	{ NULL, NULL }
};

void Event::Archive(Archiver&)
{
	/*
	if (arc.Loading())
	{
		fromScript = false;
	}

	Class::Archive(arc);

	arc.ArchiveUnsignedShort(&eventnum);
	arc.ArchiveUnsignedShort(&dataSize);

	if (arc.Loading())
	{
		data = new ScriptVariable[dataSize + 1];
	}

	for (int i = dataSize; i > 0; i--)
	{
		data[i].ArchiveInternal(arc);
	}
	*/
}

Event::Event()
{
	fromScript = false;
	eventnum = 0;
	//dataSize = 0;
	//data = nullptr;
}

Event::Event(const EventDef& def)
	: Event(def.GetEventNum())
{
}

Event::Event(const EventDef& def, size_t numArgs)
	: Event(def.GetEventNum(), numArgs)
{
}

Event::Event(uintptr_t eventnumValue)
	: eventnum(eventnumValue)
	, fromScript(false)
{
	//dataSize = 0;
	//data = nullptr;
}

Event::Event(uintptr_t eventnumValue, size_t numArgs)
	: data(numArgs)
	, eventnum(eventnumValue)
	, fromScript(false)
{
}

Event::Event(const Event& other)
	: data(other.data)
	, eventnum(other.eventnum)
	, fromScript(other.fromScript)
{
	/*
	dataSize = other.dataSize;

	if (dataSize)
	{
		data = new ScriptVariable[other.dataSize];

		for (uintptr_t i = 0; i < other.dataSize; i++)
		{
			data[i] = other.data[i];
		}
	}
	else
	{
		data = NULL;
	}
	*/
}

Event::Event(Event&& other)
	: data(std::move(other.data))
	, eventnum(other.eventnum)
	, fromScript(other.fromScript)
{
	other.eventnum = 0;
}

Event::~Event()
{
}

Event& Event::operator=(const Event& other)
{
	data = other.data;
	eventnum = other.eventnum;
	fromScript = other.fromScript;
	return *this;
}

Event& Event::operator=(Event&& other)
{
	data = std::move(other.data);
	eventnum = other.eventnum;
	fromScript = other.fromScript;
	other.eventnum = 0;
	return *this;
}

void* Event::operator new(size_t)
{
	return EventContext::Get().GetAllocator().GetBlock<Event>().Alloc();
}

void Event::operator delete(void* ptr)
{
	return EventContext::Get().GetAllocator().GetBlock<Event>().Free(ptr);
}

bool Event::operator==(const Event& ev)
{
	return eventnum == ev.eventnum;
}

bool Event::operator!=(const Event& ev)
{
	return eventnum != ev.eventnum;
}

void Event::Clear()
{
	data.ClearObjectList();
}

void Event::ReserveArguments(size_t count)
{
	if (count > data.MaxObjects()) data.Resize(count);
}

void Event::CheckPos(uintptr_t pos)
{
	if (pos > NumArgs()) {
		throw ScriptException("Index " + str(pos) + " out of range");
	}
}
const EventDef* Event::GetInfo() const
{
	return EventSystem::Get().GetEventDef(eventnum);
}

const rawchar_t* Event::GetName() const
{
	return EventSystem::Get().GetEventName(eventnum);
}

const rawchar_t* Event::GetFormat() const
{
	const EventDef* const eventDef = GetInfo();
	return eventDef ? eventDef->formatspec : nullptr;
}

uintptr_t Event::Num() const
{
	return eventnum;
}

bool Event::IsFromScript() const
{
	return fromScript;
}

void Event::AddContainer(const con::Container<ListenerPtr> *container)
{
	ConstructValue(container);
}

void Event::AddFloat(float number)
{
	ConstructValue(number);
}

void Event::AddInteger(int32_t number)
{
	ConstructValue(number);
}

void Event::AddLong(int64_t number)
{
	ConstructValue(number);
}

void Event::AddListener(Listener* listener)
{
	ConstructValue(listener);
}

void Event::AddNil()
{
	ConstructValue();
}

void Event::AddConstString(const_str string)
{
	ConstructValue(string);
}

void Event::AddString(const str& string)
{
	ConstructValue(string);
}

void Event::AddString(const rawchar_t* string)
{
	ConstructValue(string);
}

void Event::AddTokens(size_t argc, const rawchar_t **argv)
{
	ReserveArguments(data.NumObjects() + argc);

	for (uintptr_t i = 0; i < argc; i++)
	{
		AddString(argv[i]);
	}
}

void Event::AddValue(const ScriptVariable& value)
{
	//ScriptVariable& variable = GetValue();
	//variable = value;
	ConstructValue(value);
}

void Event::AddValue(ScriptVariable&& value)
{
	ConstructValue(std::move(value));
}

void Event::AddVector(const Vector& vector)
{
	ConstructValue(vector);
}

bool Event::GetBoolean(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.booleanNumericValue();
}

const_str Event::GetConstString(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.constStringValue();
}

float Event::GetFloat(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.floatValue();
}

int Event::GetInteger(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.intValue();
}

Listener* Event::GetListener(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.listenerValue();
}

str Event::GetString(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);
	return variable.stringValue();
}

ScriptVariable&	Event::GetValue(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1];
}

ScriptVariable& Event::GetLastValue()
{
	return GetValue(NumArgs());
}

ScriptVariable& Event::GetValueChecked(uintptr_t pos)
{
	assert(pos <= data.NumObjects());
	return data[pos - 1];
}

ScriptVariable&	Event::GetValue()
{
/*
	ScriptVariable* tmp = data;

	const size_t dataSize = data.NumObjects();
	data = new ScriptVariable[dataSize + 1];

	if (tmp != NULL)
	{
		for (int i = 0; i < dataSize; i++)
		{
			data[i] = tmp[i];
		}

		delete[] tmp;
	}

	dataSize++;

	return data[dataSize - 1];
*/
	const uintptr_t index = data.AddObject();
	return data[index];
}

VarListView Event::GetListView() const
{
	return VarListView(data.Data(), NumArgs());
}

VarListView Event::GetListView(uintptr_t startPos) const
{
	if (startPos < 1 || startPos > NumArgs()) {
		return VarListView();
	}

	const size_t offset = startPos - 1;
	return VarListView(data.Data() + offset, NumArgs() - offset);
}

void* Event::GetUninitializedValue()
{
	const uintptr_t index = data.AddObjectUninitialized();
	return &data[index];
}

ScriptVariable* Event::GetData()
{
	return data.NumObjects() ? data.Data() : nullptr;
}

const ScriptVariable* Event::GetData() const
{
	return data.NumObjects() ? data.Data() : nullptr;
}

Vector Event::GetVector(uintptr_t pos)
{
	ScriptVariable& variable = GetValue(pos);

	return variable.vectorValue();
}

bool Event::IsListenerAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsListener();
}

bool Event::IsNilAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].GetType() == variableType_e::None;
}

bool Event::IsNumericAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsNumeric();
}

bool Event::IsStringAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsString();
}

bool Event::IsVectorAt(uintptr_t pos)
{
	CheckPos(pos);

	return data[pos - 1].IsVector();
}

size_t Event::NumArgs() const
{
	return data.NumObjects();
}

