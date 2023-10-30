#include <morfuse/Common/ConstStr.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/EventQueue.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/PredefinedString.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Container/set_archive.h>

namespace mfuse
{
EventDef EV_Listener_CancelFor
(
	"cancelFor",
	EV_DEFAULT,
	"s",
	"name",
	"Cancel for event of type name.",
	evType_e::Normal
);

EventDef EV_Listener_CommandDelay
(
	"commanddelay",
	EV_DEFAULT,
	"fsSSSSSS",
	"delay command arg1 arg2 arg3 arg4 arg5 arg6",
	"executes a command after the given delay.",
	evType_e::Normal
);

EventDef EV_Listener_Classname
(
	"classname",
	EV_DEFAULT,
	NULL,
	NULL,
	"classname variable",
	evType_e::Getter
);

EventDef EV_Listener_SetClassname
(
	"classname",
	EV_DEFAULT,
	"s",
	"classname",
	"classname variable",
	evType_e::Normal
);

EventDef EV_Listener_CreateReturnThread
(
	"thread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label.",
	evType_e::Return
);

EventDef EV_Listener_CreateThread
(
	"thread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label.",
	evType_e::Normal
);

EventDef EV_Listener_ExecuteReturnScript
(
	"exec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script.",
	evType_e::Return
);

EventDef EV_Listener_ExecuteScript
(
	"exec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script.",
	evType_e::Normal
);

EventDef EV_Delete
(
	"delete",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately.",
	evType_e::Normal
);

EventDef EV_Remove
(
	"immediateremove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener immediately.",
	evType_e::Normal
);

EventDef EV_ScriptRemove
(
	"remove",
	EV_DEFAULT,
	NULL,
	NULL,
	"Removes this listener the next time events are processed.",
	evType_e::Normal
);

EventDef EV_Listener_EndOn
(
	"endon",
	EV_DEFAULT,
	"s",
	"name",
	"Ends the thread when the specified event is triggered.",
	evType_e::Normal
);

EventDef EV_Listener_InheritsFrom
(
	"inheritsfrom",
	EV_DEFAULT,
	"s",
	"class",
	"Returns 1 if the class inherits from the specified class. 0 otherwise.",
	evType_e::Return
);

EventDef EV_Listener_IsInheritedBy
(
	"isinheritedby",
	EV_DEFAULT,
	"s",
	"class",
	"Returns 1 if the class is inherited by the specified class. 0 otherwise.",
	evType_e::Return
);

EventDef EV_Listener_Notify
(
	"notify",
	EV_DEFAULT,
	"s",
	"name",
	"Triggers an event. Threads waiting on those events will resume their execution.",
	evType_e::Normal
);

EventDef EV_Listener_GetOwner
(
	"owner",
	EV_DEFAULT,
	NULL,
	NULL,
	"Returns the owner.",
	evType_e::Getter
);

EventDef EV_DelayThrow
(
	"delaythrow",
	EV_DEFAULT,
	"s",
	"label",
	"Internal usage.",
	evType_e::Normal
);

EventDef EV_Throw
(
	"throw",
	EV_DEFAULT,
	"s",
	"label",
	"Throws to the specified label.",
	evType_e::Normal
);

EventDef EV_Listener_Unregister
(
	"unregister",
	EV_DEFAULT,
	"s",
	"label",
	"Unregisters the label from the EventDef of the same name.",
	evType_e::Normal
);

EventDef EV_Listener_WaitCreateReturnThread
(
	"waitthread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label and waits until the called thread is finished.",
	evType_e::Return
);

EventDef EV_Listener_WaitCreateThread
(
	"waitthread",
	EV_DEFAULT,
	"s",
	"label",
	"Creates a thread starting at label and waits until the called thread is finished.",
	evType_e::Normal
);

EventDef EV_Listener_WaitExecuteReturnScript
(
	"waitexec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script and waits until the called thread group is finished.",
	evType_e::Return
);

EventDef EV_Listener_WaitExecuteScript
(
	"waitexec",
	EV_DEFAULT,
	"s",
	"script",
	"Executes the specified script and waits until the called thread group is finished.",
	evType_e::Normal
);

EventDef EV_Listener_WaitTill
(
	"waitTill",
	EV_DEFAULT,
	"s",
	"name",
	"Wait until event of type name",
	evType_e::Normal
);

EventDef EV_Listener_WaitTillTimeout
(
	"waittill_timeout",
	EV_DEFAULT,
	"fs",
	"timeout_time name",
	"Wait until event of type name with a timeout time",
	evType_e::Normal
);

EventDef EV_Listener_WaitTillAny
(
	"waittill_any",
	EV_DEFAULT,
	"sS",
	"name1 ...",
	"Wait until any event of type name",
	evType_e::Normal
);

EventDef EV_Listener_WaitTillAnyTimeout
(
	"waittill_any_timeout",
	EV_DEFAULT,
	"fsS",
	"timeout_time name1 ...",
	"Wait until any event of type name with a timeout time",
	evType_e::Normal
);

static bool DisableListenerNotify = false;
};

namespace mfuse::ConstStrings
{
	const mfuse::PredefinedString Delete("delete");
	const mfuse::PredefinedString Remove("remove");
}

using namespace mfuse;

void ArchiveListenerPtr(Archiver& arc, SafePtr<Listener>& obj)
{
	arc.ArchiveSafePointer(obj);
}

template<>
void ConList::Archive(Archiver& arc)
{
	Archive(arc, ArchiveListenerPtr);
}

Listener::Listener()
{
	m_EndList = NULL;

	m_NotifyList = NULL;
	m_WaitForList = NULL;

	vars = NULL;
}

Listener::~Listener()
{
	CancelPendingEvents();

	Unregister((const_str)ConstStrings::Delete);
	Unregister((const_str)ConstStrings::Remove);

	UnregisterAll();
	CancelWaitingAll();

	if (vars) {
		delete vars;
	}
}

template<>
void con::Archive(Archiver& arc, con::Entry<const_str, ConList>& entry)
{
	StringDictionary& dict = ScriptContext::Get().GetDirector().GetDictionary();
	dict.ArchiveString(arc, entry.Key());

	// archive all listener entries
	entry.Value().Archive(arc);
}

void Listener::Archive(Archiver &arc)
{
	constexpr unsigned int LF_NotifyList = 1;
	constexpr unsigned int LF_WaitForList = 2;
	constexpr unsigned int LF_VarList = 4;
	constexpr unsigned int LF_EndList = 8;

	Class::Archive(arc);

	uint8_t flag = 0;

	if (!arc.Loading())
	{
		if (m_NotifyList)
			flag |= LF_NotifyList;
		if (m_WaitForList)
			flag |= LF_WaitForList;
		if (vars)
			flag |= LF_VarList;
		if (m_EndList)
			flag |= LF_EndList;
	}

	arc.ArchiveUInt8(flag);

	// archive the notify list
	if (flag & LF_NotifyList)
	{
		if (arc.Loading())
		{
			m_NotifyList = new con::set<const_str, ConList>;
		}

		m_NotifyList->Archive(arc);
	}

	// archive the waiting thread list
	if (flag & LF_WaitForList)
	{
		if (arc.Loading())
		{
			m_WaitForList = new con::set<const_str, ConList>;
		}

		m_WaitForList->Archive(arc);
	}

	// archive the variable list
	if (flag & LF_VarList)
	{
		if (arc.Loading())
		{
			vars = new ScriptVariableList;
		}

		vars->Archive(arc);
	}

	// archive the end on event list
	if (flag & LF_EndList)
	{
		if (arc.Loading())
		{
			m_EndList = new con::set<const_str, ConList>;
		}

		m_EndList->Archive(arc);
	}
}

void Listener::CancelEventsOfType(const EventDef& ev)
{
	EventContext::Get().GetEventQueue().CancelEventsOfType(this, ev);
}

void Listener::CancelFlaggedEvents(int flags)
{
	EventContext::Get().GetEventQueue().CancelFlaggedEvents(this, flags);
}

void Listener::CancelPendingEvents()
{
	EventContext::Get().GetEventQueue().CancelPendingEvents(this);
}

bool Listener::EventPending(const EventDef &ev)
{
	return EventContext::Get().GetEventQueue().IsEventPending(this, ev);
}

void Listener::PostEventInternal(Event *ev, inttime_t delay, int flags)
{
	if (!ev->Num() || !classinfo().GetResponse(ev->Num()))
	{
		delete ev;
		return;
	}

	return EventContext::Get().GetEventQueue().PostEvent(this, ev, delay, flags);
}

void Listener::PostEvent(Event *ev, inttime_t delay, int flags)
{
	PostEventInternal(ev, delay, flags);
}

bool Listener::PostponeAllEvents(inttime_t time)
{
	return EventContext::Get().GetEventQueue().PostponeAllEvents(this, time);
}

bool Listener::PostponeEvent(Event &ev, inttime_t time)
{
	return EventContext::Get().GetEventQueue().PostponeEvent(this, ev, time);
}

ScriptVariable Listener::ProcessEventReturn(Event& ev)
{
	const ClassDef& c = classinfo();
	const ResponseDefClass* responses = NULL;
	Response response = NULL;
	ScriptVariable returnValue;

	if (!ev.Num())
	{
		return returnValue;
	}

	const EventContext& context = EventContext::Get();
	const EventDef* eventDef = EventSystem::Get().GetEventDef(ev.Num());
	const NamespaceManager& nspaceMan = context.GetNamespaceManager();
	if (!nspaceMan.IsObjectInNamespaceAllowed(*eventDef))
	{
		// if the object is not present in allowed namespaces then an error can be thrown
		throw ListenerErrors::EventNotFound(*this);
	}

	responses = c.GetResponse(ev.Num());

	if (!responses)
	{
		return returnValue;
	}

	response = responses->response;

	size_t previousArgs = ev.NumArgs();

	if (response)
	{
		(this->*response)(ev);
	}

	if (previousArgs != ev.NumArgs() && ev.NumArgs() != 0)
	{
		returnValue = ev.GetValue(ev.NumArgs());
	}

	return returnValue;
}

ScriptVariable Listener::ProcessEventReturn(Event* ev)
{
	ScriptVariable returnValue = ProcessEventReturn(*ev);
	delete ev;
	return returnValue;
}

bool Listener::ProcessEvent(Event* ev)
{
	const bool result = ProcessEvent(*ev);
	delete ev;
	return result;
}

bool Listener::ProcessEvent(Event& ev)
{
	try
	{
		ProcessScriptEvent(ev);
	}
	catch (std::exception& exc)
	{
		std::ostream* const dbg = GlobalOutput::Get().GetOutput(outputLevel_e::Debug);
		if (dbg) {
			*dbg << "^~^~^ " << exc.what() << std::endl;
		}

		return false;
	}

	return true;
}

void Listener::ProcessScriptEvent(Event* ev)
{
	ProcessScriptEvent(*ev);
	delete ev;
}

void Listener::ProcessScriptEvent(Event& ev)
{
	const ClassDef& c = classinfo();
	const ResponseDefClass *responses = nullptr;
	Response response = nullptr;

	if (!ev.Num())
	{
		throw ListenerErrors::EventNotFound(*this);
	}

	const EventContext& context = EventContext::Get();
	const EventDef* eventDef = EventSystem::Get().GetEventDef(ev.Num());
	const NamespaceManager& nspaceMan = context.GetNamespaceManager();
	if (!nspaceMan.IsObjectInNamespaceAllowed(*eventDef))
	{
		// if the object is not present in allowed namespaces then an error can be thrown
		throw ListenerErrors::EventNotFound(*this);
	}

	responses = c.GetResponse(ev.Num());

	if (!responses)
	{
		throw ListenerErrors::EventListenerFailed(*this, ev.Num());
	}

	response = responses->response;

	if (response)
	{
		(this->*response)(ev);
	}
}

void Listener::ProcessContainerEvent(const con::Container<Event *>& conev)
{
	const size_t num = conev.NumObjects();

	for (uintptr_t i = 1; i <= num; i++)
	{
		ProcessEvent(conev.ObjectAt(i));
	}
}

bool Listener::ProcessPendingEvents()
{
	return EventContext::Get().GetEventQueue().ProcessPendingEvents(this);
}

Listener *Listener::GetScriptOwner()
{
	return NULL;
}

void Listener::CreateVars()
{
	vars = new ScriptVariableList();
}

void Listener::ClearVars()
{
	if (vars)
	{
		delete vars;
		CreateVars();
	}
}

ScriptVariableList *Listener::Vars()
{
	if (!vars) {
		CreateVars();
	}

	return vars;
}

bool Listener::BroadcastEvent(const_str name, Event &event)
{
	if (!m_NotifyList) {
		return false;
	}

	ConList *listeners = m_NotifyList->findKeyValue(name);

	if (!listeners) {
		return false;
	}

	return BroadcastEvent(event, listeners);
}

bool Listener::BroadcastEvent(Event &event, ConList *listeners)
{
	const size_t num = listeners->NumObjects();

	if (!num) {
		return false;
	}

	if (num == 1)
	{
		Listener *listener = listeners->ObjectAt(1);

		if (listener) {
			listener->ProcessEvent(event);
			return true;
		}

		return false;
	}

	ConList *listenersCopy = new ConList();
	*listenersCopy = *listeners;
	bool found = false;

	for (uintptr_t i = listenersCopy->NumObjects(); i > 0; i--)
	{
		Listener *listener = listenersCopy->ObjectAt(i);

		if (listener)
		{
			listener->ProcessEvent(event);
			found = true;
		}
	}

	delete listenersCopy;

	return found;
}

void Listener::CancelWaiting(const_str name)
{
	if (!m_WaitForList)
	{
		return;
	}

	ConList *list = m_WaitForList->findKeyValue(name);
	ConList stoppedListeners;

	if (!list)
	{
		return;
	}

	CancelWaitingSources(name, *list, stoppedListeners);

	m_WaitForList->remove(name);

	if (m_WaitForList->isEmpty())
	{
		delete m_WaitForList;
		m_WaitForList = NULL;

		if (!DisableListenerNotify)
		{
			StoppedWaitFor(name, false);
		}
	}

	if (!DisableListenerNotify)
	{
		for (uintptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
		{
			Listener *listener = stoppedListeners.ObjectAt(i);

			if (listener)
			{
				listener->StoppedNotify();
			}
		}
	}
}

void Listener::CancelWaitingAll()
{
	CancelWaiting(const_str(0));

	if (!m_WaitForList)
	{
		return;
	}

	con::set_enum<const_str, ConList> en = *m_WaitForList;
	const con::Entry<const_str, ConList> *e;
	ConList stoppedListeners;

	for (e = en.NextElement(); e; e = en.NextElement())
	{
		CancelWaitingSources(e->Key(), e->Value(), stoppedListeners);
	}

	delete m_WaitForList;
	m_WaitForList = NULL;

	if (!DisableListenerNotify)
	{
		StoppedWaitFor(0, false);

		for (uintptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
		{
			Listener *listener = stoppedListeners.ObjectAt(i);

			if (listener)
			{
				listener->StoppedNotify();
			}
		}
	}
}

void Listener::CancelWaitingSources(const_str name, const ConList &listeners, ConList &stoppedListeners)
{
	for (uintptr_t i = listeners.NumObjects(); i > 0; i--)
	{
		Listener *listener = listeners.ObjectAt(i);

		if (listener && listener->UnregisterSource(name, this))
		{
			stoppedListeners.AddObject(listener);
		}
	}
}

void Listener::StoppedNotify()
{
}

void Listener::StartedWaitFor()
{
}

void Listener::StoppedWaitFor(const_str, bool)
{
}

void Listener::EndOn(const_str name, Listener *listener)
{
	if (!m_EndList)
	{
		m_EndList = new con::set<const_str, ConList>;
	}

	ConList& list = m_EndList->addKeyValue(name);

	list.AddUniqueObject(listener);
}

void Listener::Register(const_str name, Listener *listener)
{
	RegisterSource(name, listener);
	listener->RegisterTarget(name, this);
}

void Listener::RegisterSource(const_str name, Listener *listener)
{
	if (!m_NotifyList)
	{
		m_NotifyList = new con::set<const_str, ConList>;
	}

	ConList& list = m_NotifyList->addKeyValue(name);

	list.AddObject(listener);
}

void Listener::RegisterTarget(const_str name, Listener *listener)
{
	if (!m_WaitForList)
	{
		StartedWaitFor();
		m_WaitForList = new con::set<const_str, ConList>;
	}

	ConList& list = m_WaitForList->addKeyValue(name);

	list.AddObject(listener);
}

void Listener::Unregister(const_str name)
{
	if (m_EndList)
	{
		ConList *list = m_EndList->findKeyValue(name);
		bool bDeleteSelf = false;

		if (list)
		{
			ConList listeners = *list;

			m_EndList->remove(name);

			if (m_EndList->isEmpty())
			{
				delete m_EndList;
				m_EndList = NULL;
			}

			for (uintptr_t i = listeners.NumObjects(); i > 0; i--)
			{
				Listener *listener = listeners.ObjectAt(i);

				if (listener)
				{
					if (listener == this && (name == ConstStrings::Remove || name == ConstStrings::Delete || bDeleteSelf))
					{
						continue;
					}

					if (listener == this)
					{
						bDeleteSelf = true;
					}

					delete listener;
				}
			}
		}

		if (bDeleteSelf)
		{
			return;
		}
	}

	if (!m_NotifyList)
	{
		return;
	}

	ConList *list = m_NotifyList->findKeyValue(name);
	ConList stoppedListeners;
	con::Container<const_str> stoppedNames;

	if (!list)
	{
		return;
	}

	UnregisterTargets(name, *list, stoppedListeners, stoppedNames);

	m_NotifyList->remove(name);

	if (m_NotifyList->isEmpty())
	{
		delete m_NotifyList;
		m_NotifyList = NULL;

		if (!DisableListenerNotify)
		{
			StoppedNotify();
		}
	}

	if (!DisableListenerNotify)
	{
		for (uintptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
		{
			Listener *listener = stoppedListeners.ObjectAt(i);

			if (listener)
			{
				listener->StoppedWaitFor(name, false);
			}
		}
	}
}

void Listener::Unregister(const_str name, Listener *listener)
{
	if (UnregisterSource(name, listener))
	{
		if (!DisableListenerNotify) {
			StoppedNotify();
		}
	}

	if (listener->UnregisterTarget(name, this))
	{
		if (!DisableListenerNotify) {
			listener->StoppedWaitFor(name, false);
		}
	}
}

void Listener::UnregisterAll()
{
	Unregister(const_str(0));

	if (m_EndList)
	{
		delete m_EndList;
		m_EndList = NULL;
	}

	if (!m_NotifyList)
	{
		return;
	}

	con::set_enum<const_str, ConList> en = *m_NotifyList;
	const con::Entry<const_str, ConList> *e;
	ConList stoppedListeners;
	con::Container<const_str> stoppedNames;

	en = *m_NotifyList;

	for (e = en.NextElement(); e; e = en.NextElement())
	{
		UnregisterTargets(e->Key(), e->Value(), stoppedListeners, stoppedNames);
	}

	delete m_NotifyList;
	m_NotifyList = NULL;

	if (!DisableListenerNotify) {
		StoppedNotify();
	}

	for (uintptr_t i = stoppedListeners.NumObjects(); i > 0; i--)
	{
		Listener *listener = stoppedListeners.ObjectAt(i);

		if (listener)
		{
			if (!DisableListenerNotify) {
				listener->StoppedWaitFor(stoppedNames.ObjectAt(i), true);
			}
		}
	}
}

bool Listener::UnregisterSource(const_str name, Listener *listener)
{
	if (!m_NotifyList)
	{
		return false;
	}

	ConList *list = m_NotifyList->findKeyValue(name);
	bool found = false;

	if (!list)
	{
		return false;
	}

	for (uintptr_t i = list->NumObjects(); i > 0; i--)
	{
		if (list->ObjectAt(i) == listener)
		{
			list->RemoveObjectAt(i);
			found = true;
		}
	}

	if (list->NumObjects() == 0)
	{
		m_NotifyList->remove(name);

		if (m_NotifyList->isEmpty())
		{
			delete m_NotifyList;
			m_NotifyList = NULL;
		}
	}

	return found;
}

bool Listener::UnregisterTarget(const_str name, Listener *listener)
{
	if (!m_WaitForList)
	{
		return false;
	}

	ConList *list = m_WaitForList->findKeyValue(name);
	bool found = false;

	if (!list)
	{
		return false;
	}

	for (uintptr_t i = list->NumObjects(); i > 0; i--)
	{
		if (list->ObjectAt(i) == listener)
		{
			list->RemoveObjectAt(i);
			found = true;
		}
	}

	if (list->NumObjects() == 0)
	{
		m_WaitForList->remove(name);

		if (m_WaitForList->isEmpty())
		{
			delete m_WaitForList;
			m_WaitForList = NULL;
		}
	}

	return found;
}

void Listener::UnregisterTargets(const_str name, const ConList &listeners, ConList &stoppedListeners, con::Container<const_str> &stoppedNames)
{
	for (uintptr_t i = listeners.NumObjects(); i > 0; i--)
	{
		Listener *listener = listeners.ObjectAt(i);

		if (listener && listener->UnregisterTarget(name, this))
		{
			stoppedListeners.AddObject(listener);
			stoppedNames.AddObject(name);
		}
	}
}

void Listener::AbortRegistration(const_str name, Listener *l)
{
	UnregisterSource(name, l);
	l->UnregisterTarget(name, this);
}


size_t Listener::RegisterSize(const_str name) const
{
	ConList *listeners;

	if (!m_NotifyList) {
		return 0;
	}

	listeners = m_NotifyList->findKeyValue(name);

	// return the number of listeners waiting for this listener
	if (listeners)
	{
		return listeners->NumObjects();
	}
	else
	{
		return 0;
	}
}

size_t Listener::WaitingSize(const_str name) const
{
	ConList *listeners;

	if (!m_WaitForList) {
		return 0;
	}

	listeners = m_WaitForList->findKeyValue(name);

	// return the number of listeners this listener is waiting for
	if (listeners)
	{
		return listeners->NumObjects();
	}
	else
	{
		return 0;
	}
}

bool Listener::WaitTillDisabled(const_str s)
{
	for (const ClassDef *c = &classinfo(); c != NULL; c = c->GetSuper())
	{
		if (c->WaitTillDefined(s))
		{
			return true;
		}
	}

	return false;
}

int Listener::GetFlags(Event *event) const
{
	return classinfo().GetFlags(event);
}

//==========================
// Listener's events
//==========================

void Listener::EventDelete(Event& ev)
{
	if (ev.NumArgs()) {
		throw ScriptException("Arguments not allowed");
	}

	delete this;
}

void Listener::EventInheritsFrom(Event& ev)
{
	const str className = ev.GetString(1);
	ev.AddInteger(inheritsFrom(className.c_str()));
}

void Listener::EventIsInheritedBy(Event& ev)
{
	const str className = ev.GetString(1);
	ev.AddInteger(isInheritedBy(className.c_str()));
}

void Listener::GetClassname(Event& ev)
{
	ev.AddString(Class::GetClassname());
}

void Listener::CommandDelay(Event& ev)
{
	if (ev.NumArgs() < 2) {
		throw ScriptException("Not enough arguments.");
	}

	EventSystem& eventSystem = EventSystem::Get();
	const eventName_t eventName = eventSystem.GetEventConstName(ev.GetString(2).c_str());
	const eventInfo_t* const eventInfo = eventSystem.FindEventInfo(eventName);
	if(eventInfo)
	{
		const eventNum_t eventNum = eventInfo->normalNum
			? eventInfo->normalNum
			: eventInfo->returnNum ? eventInfo->returnNum
			: eventInfo->setterNum ? eventInfo->setterNum
			: eventInfo->getterNum ? eventInfo->getterNum
			: 0;

		Event* e = new Event(eventNum);

		for (size_t i = 3; i <= ev.NumArgs(); i++)
		{
			e->AddValue(ev.GetValue(i));
		}

		PostEvent(e, uinttime_t(ev.GetFloat(1) * 1000.f));
	}
	else
	{

	}
}

void Listener::CancelFor(Event& ev)
{
	Event RemoveEvent(EV_Remove);
	BroadcastEvent(ev.GetConstString(1), RemoveEvent);
}

void Listener::EventDelayThrow(Event& ev)
{
	BroadcastEvent(const_str(0), ev);
}

void Listener::EventEndOn(Event& ev)
{
	const_str name = ev.GetConstString(1);

	if (ScriptContext::Get().GetDirector().CurrentThread() == this) {
		throw ScriptException("cannot end the current thread!");
	}

	EndOn(name, ScriptContext::Get().GetDirector().CurrentThread());
}

void Listener::EventGetOwner(Event& ev)
{
	ev.AddListener(GetScriptOwner());
}

void Listener::EventNotify(Event& ev)
{
	Unregister(ev.GetConstString(1));
}

void Listener::EventThrow(Event& ev)
{
	BroadcastEvent(const_str(0), ev);
}

void Listener::EventUnregister(Event& ev)
{
	Unregister(ev.GetConstString(1));
}

void Listener::WaitTill(Event& ev)
{
	const_str name;

	if (ScriptContext::Get().GetDirector().CurrentThread() == this) {
		throw ScriptException("cannot waittill on the current thread!");
	}

	name = ev.GetConstString(1);

	if (WaitTillDisabled(name))
	{
		const ScriptMaster& director = ScriptContext::Get().GetDirector();
		throw ScriptException("invalid waittill " + director.GetDictionary().Get(name) + " for " + Class::GetClassname());
	}

	Register(name, ScriptContext::Get().GetDirector().CurrentThread());
}

void Listener::WaitTillTimeout(Event& ev)
{
	const_str name;
	float timeout_time;

	if (ScriptContext::Get().GetDirector().CurrentThread() == this) {
		throw ScriptException("cannot waittill on the current thread!");
	}

	timeout_time = ev.GetFloat(1);
	name = ev.GetConstString(2);

	if (WaitTillDisabled(name))
	{
		const ScriptMaster& director = ScriptContext::Get().GetDirector();
		throw ScriptException("invalid waittill " + director.GetDictionary().Get(name) + " for " + Class::GetClassname());
	}

	Register(name, ScriptContext::Get().GetDirector().CurrentThread());

	Event* newEvent = new Event(EV_ScriptThread_CancelWaiting);
	ScriptContext::Get().GetDirector().CurrentThread()->PostEvent(newEvent, uinttime_t(timeout_time * 1000.f));
}

void Listener::WaitTillAny(Event& ev)
{
	const_str name;

	if (ScriptContext::Get().GetDirector().CurrentThread() == this) {
		throw ScriptException("cannot waittill any on the current thread!");
	}

	for (size_t i = 1; i <= ev.NumArgs(); i++)
	{
		name = ev.GetConstString(i);

		if (WaitTillDisabled(name))
		{
			const ScriptMaster& director = ScriptContext::Get().GetDirector();
			throw ScriptException("invalid waittill " + director.GetDictionary().Get(name) + " for " + Class::GetClassname());
		}

		Register(name, ScriptContext::Get().GetDirector().CurrentThread());
	}
}

void Listener::WaitTillAnyTimeout(Event& ev)
{
	const_str name;
	float timeout_time;

	if (ScriptContext::Get().GetDirector().CurrentThread() == this) {
		throw ScriptException("cannot waittill any on the current thread!");
	}

	timeout_time = ev.GetFloat(1);

	for (size_t i = 1; i <= ev.NumArgs(); i++)
	{
		name = ev.GetConstString(i);

		if (WaitTillDisabled(name))
		{
			const ScriptMaster& director = ScriptContext::Get().GetDirector();
			throw ScriptException("invalid waittill " + director.GetDictionary().Get(name) + " for " + Class::GetClassname());
		}

		Register(name, ScriptContext::Get().GetDirector().CurrentThread());
	}

	Event* newEvent = new Event(EV_ScriptThread_CancelWaiting);
	ScriptContext::Get().GetDirector().CurrentThread()->PostEvent(newEvent, uinttime_t(timeout_time * 1000));
}

void Listener::ExecuteScriptInternal(Event& ev, ScriptVariable& returnValue)
{
	ScriptThread* const thread = CreateScriptInternal(ev.GetValue(1));
	thread->ScriptExecute(ev.GetListView(2), returnValue);
}

void Listener::ExecuteThreadInternal(Event& ev, ScriptVariable& returnValue)
{
	ScriptThread* const thread = CreateThreadInternal(ev.GetValue(1));
	thread->ScriptExecute(ev.GetListView(2), returnValue);
}

void Listener::WaitExecuteScriptInternal(Event& ev, ScriptVariable& returnValue)
{
	ScriptThread* const thread = CreateScriptInternal(ev.GetValue(1));

	thread->GetScriptClass()->Register(const_str(0), ScriptContext::Get().GetDirector().CurrentThread());

	thread->ScriptExecute(ev.GetListView(2), returnValue);
}

void Listener::WaitExecuteThreadInternal(Event& ev, ScriptVariable& returnValue)
{
	ScriptThread* const thread = CreateThreadInternal(ev.GetValue(1));

	thread->Register(const_str(0), ScriptContext::Get().GetDirector().CurrentThread());

	thread->ScriptExecute(ev.GetListView(2), returnValue);
}

ScriptThread *Listener::CreateScriptInternal(const ScriptVariable& label)
{
	ScriptMaster& director = ScriptContext::Get().GetDirector();

	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		const ProgramScript* const scr = director.GetProgramScript(label.constStringValue());
		return director.CreateScriptThread(scr, this, StringResolvable());
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		const ScriptVariable& script = label[1];
		const ScriptVariable& labelname = label[2];

		const ProgramScript* const scr = director.GetProgramScript(script.constStringValue());
		return director.CreateScriptThread(scr, this, labelname.constStringValue());
	}
	else
	{
		throw ListenerErrors::BadLabelType(label.GetTypeName());
	}
}

ScriptThread *Listener::CreateThreadInternal(const ScriptVariable& label)
{
	ScriptMaster& director = ScriptContext::Get().GetDirector();

	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		const ScriptClass* const scriptClass = director.CurrentScriptClass();
		const ProgramScript* const scr = scriptClass->GetScript();
		return director.CreateScriptThread(scr, this, label.constStringValue());
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		const ScriptVariable& script = label[1];
		const ScriptVariable& labelname = label[2];

		const ProgramScript* const scr = director.GetProgramScript(script.constStringValue());
		return director.CreateScriptThread(scr, this, labelname.constStringValue());
	}
	else
	{
		throw ListenerErrors::BadLabelType(label.GetTypeName());
	}
}

void Listener::CreateReturnThread(Event& ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	ExecuteThreadInternal(ev, returnValue);

	ev.AddValue(returnValue);
}

void Listener::CreateThread(Event& ev)
{
	ScriptVariable returnValue;

	ExecuteThreadInternal(ev, returnValue);
}

void Listener::ExecuteReturnScript(Event& ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	ExecuteScriptInternal(ev, returnValue);

	ev.AddValue(returnValue);
}

void Listener::ExecuteScript(Event& ev)
{
	ScriptVariable returnValue;

	ExecuteScriptInternal(ev, returnValue);
}

void Listener::WaitCreateReturnThread(Event& ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	WaitExecuteThreadInternal(ev, returnValue);

	ev.AddValue(returnValue);
}

void Listener::WaitCreateThread(Event& ev)
{
	ScriptVariable returnValue;

	WaitExecuteThreadInternal(ev, returnValue);
}

void Listener::WaitExecuteReturnScript(Event& ev)
{
	ScriptVariable returnValue;

	returnValue.newPointer();

	WaitExecuteScriptInternal(ev, returnValue);

	ev.AddValue(returnValue);
}

void Listener::WaitExecuteScript(Event& ev)
{
	ScriptVariable returnValue;

	WaitExecuteScriptInternal(ev, returnValue);
}

ListenerErrors::BadLabelType::BadLabelType(const rawchar_t* typeNameValue)
	: typeName(typeNameValue)
{
}

const mfuse::rawchar_t* ListenerErrors::BadLabelType::GetTypeName() const
{
	return typeName;
}

const char* ListenerErrors::BadLabelType::what() const noexcept
{
	if (!filled()) {
		fill("Listener::CreateScriptInternal: bad label type '" + str(GetTypeName()) + "'");
	}

	return Messageable::what();
}

ListenerErrors::EventError::EventError(const Listener& lPtr)
	: l(lPtr)
{
}

const Listener& ListenerErrors::EventError::GetListener() const
{
	return l;
}

ListenerErrors::EventNotFound::EventNotFound(const Listener& lPtr)
	: EventError(lPtr)
{
}

const char* ListenerErrors::EventNotFound::what() const noexcept
{
	if (!filled()) {
		fill("Failed execution of event for class '" + str(GetListener().classinfo().GetClassName()) + "'");
	}

	return Messageable::what();
}

ListenerErrors::EventListenerFailed::EventListenerFailed(const Listener& lPtr, eventNum_t eventNumVal)
	: EventError(lPtr)
	, eventNum(eventNumVal)
{
}

eventNum_t ListenerErrors::EventListenerFailed::GetEventNum() const
{
	return eventNum;
}

const char* ListenerErrors::EventListenerFailed::what() const noexcept
{
	if (!filled())
	{
		const EventSystem& evt = EventSystem::Get();
		const str eventName = evt.GetEventName(eventNum);
		fill("Failed execution of event '" + eventName + "' for class '" + str(GetListener().classinfo().GetClassName()) + "'");
	}

	return Messageable::what();
}

MFUS_CLASS_DECLARATION(Class, Listener, NULL)
{
	{ &EV_Listener_CommandDelay,				&Listener::CommandDelay },
	{ &EV_Delete,								&Listener::EventDelete },
	{ &EV_Remove,								&Listener::EventDelete },
	{ &EV_ScriptRemove,							&Listener::EventDelete },
	{ &EV_Listener_Classname,					&Listener::GetClassname },
	{ &EV_Listener_InheritsFrom,				&Listener::EventInheritsFrom },
	{ &EV_Listener_IsInheritedBy,				&Listener::EventIsInheritedBy },
	{ &EV_Listener_CancelFor,					&Listener::CancelFor },
	{ &EV_Listener_CreateReturnThread,			&Listener::CreateReturnThread },
	{ &EV_Listener_CreateThread,				&Listener::CreateThread },
	{ &EV_Listener_ExecuteReturnScript,			&Listener::ExecuteReturnScript },
	{ &EV_Listener_ExecuteScript,				&Listener::ExecuteScript },
	{ &EV_Listener_EndOn,						&Listener::EventEndOn },
	{ &EV_Listener_GetOwner,					&Listener::EventGetOwner },
	{ &EV_Listener_Notify,						&Listener::EventNotify },
	{ &EV_DelayThrow,							&Listener::EventDelayThrow },
	{ &EV_Throw,								&Listener::EventThrow },
	{ &EV_Listener_Unregister,					&Listener::EventUnregister },
	{ &EV_Listener_WaitCreateReturnThread,		&Listener::WaitCreateReturnThread },
	{ &EV_Listener_WaitCreateThread,			&Listener::WaitCreateThread },
	{ &EV_Listener_WaitExecuteReturnScript,		&Listener::WaitExecuteReturnScript },
	{ &EV_Listener_WaitExecuteScript,			&Listener::WaitExecuteScript },
	{ &EV_Listener_WaitTill,					&Listener::WaitTill },
	{ &EV_Listener_WaitTillTimeout,				&Listener::WaitTillTimeout },
	{ &EV_Listener_WaitTillAny,					&Listener::WaitTillAny },
	{ &EV_Listener_WaitTillAnyTimeout,			&Listener::WaitTillAnyTimeout },
	{ NULL, NULL }
};


