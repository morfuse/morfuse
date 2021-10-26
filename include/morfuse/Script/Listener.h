#pragma once

#include "Class.h"
#include "ContainerClass.h"
#include "../Container/set.h"
#include "../Common/TimeTypes.h"
#include "Event.h"

namespace mfuse
{
	class Listener;
	class Event;
	class EventDef;
	class ScriptThread;
	class ScriptVariableList;
	class StringResolvable;
	class PredefinedString;

	mfuse_EXPORTS extern EventDef EV_DelayThrow;
	mfuse_EXPORTS extern EventDef EV_Throw;
	mfuse_EXPORTS extern EventDef EV_Listener_GetOwner;
	mfuse_EXPORTS extern EventDef EV_Listener_CreateReturnThread;
	mfuse_EXPORTS extern EventDef EV_Listener_CreateThread;
	mfuse_EXPORTS extern EventDef EV_Listener_ExecuteReturnScript;
	mfuse_EXPORTS extern EventDef EV_Listener_ExecuteScript;
	mfuse_EXPORTS extern EventDef EV_Listener_WaitCreateReturnThread;
	mfuse_EXPORTS extern EventDef EV_Remove;

	using ListenerPtr = SafePtr<class Listener>;

	using eventMap = con::map<Event*, EventDef*>;

	class Listener : public Class
	{
		MFUS_CLASS_PROTOTYPE_EXPORTS(Listener);

	public:
		mfuse_EXPORTS Listener();
		mfuse_EXPORTS ~Listener();

		mfuse_EXPORTS virtual Listener* GetScriptOwner();

		mfuse_EXPORTS void Archive(Archiver &arc) override;

		mfuse_EXPORTS void CancelEventsOfType(const EventDef& ev);
		mfuse_EXPORTS void CancelFlaggedEvents(int flags);
		mfuse_EXPORTS void CancelPendingEvents(void);
		mfuse_EXPORTS bool EventPending(const EventDef &ev);
		mfuse_EXPORTS void PostEvent(Event *ev, inttime_t delay, int flags = 0);
		mfuse_EXPORTS bool PostponeAllEvents(inttime_t time);
		mfuse_EXPORTS bool PostponeEvent(Event& ev, inttime_t time);
		mfuse_EXPORTS bool ProcessEvent(Event *ev);
		mfuse_EXPORTS bool ProcessEvent(Event& ev);
		mfuse_EXPORTS ScriptVariable ProcessEventReturn(Event& ev);
		mfuse_EXPORTS ScriptVariable ProcessEventReturn(Event* ev);
		mfuse_EXPORTS void ProcessContainerEvent(const con::Container<Event *>& conev);
		mfuse_EXPORTS bool ProcessPendingEvents(void);
		mfuse_EXPORTS void ProcessScriptEvent(Event &ev);
		mfuse_EXPORTS void ProcessScriptEvent(Event *ev);

		void CreateVars();
		void ClearVars();
		ScriptVariableList* Vars();

		mfuse_EXPORTS bool BroadcastEvent(Event& event, ConList* listeners);
		mfuse_EXPORTS bool BroadcastEvent(const_str name, Event& event);
		mfuse_EXPORTS void CancelWaiting(const_str name);
		mfuse_EXPORTS void CancelWaitingAll(void);
		mfuse_EXPORTS void CancelWaitingSources(const_str name, const ConList& listeners, ConList& stoppedListeners);

		mfuse_EXPORTS void EndOn(const_str name, Listener *listener);
		mfuse_EXPORTS void Register(const_str name, Listener *listener);
		mfuse_EXPORTS void RegisterSource(const_str name, Listener *listener);
		mfuse_EXPORTS void RegisterTarget(const_str name, Listener *listener);
		mfuse_EXPORTS void Unregister(const_str name);
		mfuse_EXPORTS void Unregister(const_str name, Listener *listener);
		mfuse_EXPORTS void UnregisterAll(void);
		mfuse_EXPORTS void AbortRegistration(const_str name, Listener *l);

		mfuse_EXPORTS size_t RegisterSize(const_str name) const;
		mfuse_EXPORTS size_t WaitingSize(const_str name) const;

		mfuse_EXPORTS bool WaitTillDisabled(const_str s);

		mfuse_EXPORTS int GetFlags(Event *event) const;

		//
		// Scripting functions
		//
		void CommandDelay(Event& ev);
		void EventDelete(Event& ev);
		void EventInheritsFrom(Event& ev);
		void EventIsInheritedBy(Event& ev);
		void GetClassname(Event& ev);

		void CancelFor(Event& ev);
		void CreateReturnThread(Event& ev);
		void CreateThread(Event& ev);
		void ExecuteReturnScript(Event& ev);
		void ExecuteScript(Event& ev);
		void EventDelayThrow(Event& ev);
		void EventEndOn(Event& ev);
		void EventGetOwner(Event& ev);
		void EventNotify(Event& ev);
		void EventThrow(Event& ev);
		void EventUnregister(Event& ev);
		void WaitCreateReturnThread(Event& ev);
		void WaitCreateThread(Event& ev);
		void WaitExecuteReturnScript(Event& ev);
		void WaitExecuteScript(Event& ev);
		void WaitTill(Event& ev);
		void WaitTillTimeout(Event& ev);
		void WaitTillAny(Event& ev);
		void WaitTillAnyTimeout(Event& ev);

	protected:
		/* Game functions */
		mfuse_EXPORTS virtual ScriptThread* CreateThreadInternal(const ScriptVariable& label);
		mfuse_EXPORTS virtual ScriptThread* CreateScriptInternal(const ScriptVariable& label);
		mfuse_EXPORTS virtual void StoppedNotify();
		mfuse_EXPORTS virtual void StartedWaitFor();
		mfuse_EXPORTS virtual void StoppedWaitFor(const_str name, bool bDeleting);

	private:
		bool UnregisterSource(const_str name, Listener* listener);
		bool UnregisterTarget(const_str name, Listener* listener);
		void UnregisterTargets(const_str name, const ConList& listeners, ConList& stoppedListeners, con::Container<const_str>& stoppedNames);
		void ExecuteScriptInternal(Event& ev, ScriptVariable& scriptVariable);
		void ExecuteThreadInternal(Event& ev, ScriptVariable& returnValue);
		void WaitExecuteScriptInternal(Event& ev, ScriptVariable& returnValue);
		void WaitExecuteThreadInternal(Event& ev, ScriptVariable& returnValue);

		void PostEventInternal(Event* ev, inttime_t delay, int flags);

	private:
		con::set<const_str, ConList>* m_NotifyList;
		con::set<const_str, ConList>* m_WaitForList;
		con::set<const_str, ConList>* m_EndList;
		ScriptVariableList* vars;
	};

	namespace ConstStrings
	{
		mfuse_EXPORTS extern const PredefinedString Delete;
		mfuse_EXPORTS extern const PredefinedString Remove;
	}

	namespace ListenerErrors
	{
		class Base : public ScriptExceptionBase {};

		class BadLabelType : public Base, public Messageable
		{
		public:
			BadLabelType(const rawchar_t* typeNameValue);

			const rawchar_t* GetTypeName() const;
			const char* what() const noexcept override;

		private:
			const rawchar_t* typeName;
		};

		class EventError : public Base
		{
		public:
			EventError(const Listener& lRef);

			const Listener& GetListener() const;

		private:
			const Listener& l;
		};

		class EventNotFound : public EventError, public Messageable
		{
		public:
			EventNotFound(const Listener& lRef);

			const char* what() const noexcept override;
		};

		class EventListenerFailed : public EventError, public Messageable
		{
		public:
			EventListenerFailed(const Listener& lRef, eventNum_t eventNum);

			eventNum_t GetEventNum() const;
			const char* what() const noexcept override;

		private:
			eventNum_t eventNum;
		};
	}
};
