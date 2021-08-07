#pragma once

#include "../Global.h"
#include "../Script/Event.h"
#include "../Container/set.h"
#include "../Container/arrayset.h"
#include "../Common/MEM/PreAllocator.h"

namespace mfuse
{
	class Archiver;
	class ScriptContext;

	struct eventInfo_t {
		eventNum_t normalNum;
		eventNum_t returnNum;
		eventNum_t setterNum;
		eventNum_t getterNum;

		eventInfo_t();
	};

	/**
	 * Insensitive event name hash.
	 */
	struct EventNameHash
	{
		intptr_t operator()(const const_xstr_static& key) const;
	};

	/**
	 * Insensitive event name comparison.
	 */
	struct EventNameCompare
	{
		bool operator()(const const_xstr_static& lhs, const const_xstr_static& rhs) const;
	};

	/** Index to the event name table. */
	using eventName_t = uintptr_t;
	/** Map of event number to event definition. */
	using eventDefMap = const EventDef**;
	/** The event name table. */
	using eventNameSet = con::arrayset<
		const_xstr_static,
		const_xstr_static,
		EventNameHash,
		EventNameCompare,
		MEM::ChildPreAllocator_set
	>;

	class EventSystem
	{
		friend class Event;
		friend class Listener;
		friend class ScriptMaster;

	public:
		mfuse_EXPORTS static EventSystem& Get();
		mfuse_EXPORTS bool EventSystemStarted();
		mfuse_EXPORTS static size_t NumEventCommands();
		mfuse_EXPORTS void InitEvents();

		mfuse_EXPORTS eventName_t GetEventConstName(const rawchar_t* name) const;

		mfuse_EXPORTS const EventDef* GetEventDef(eventNum_t eventnum);
		mfuse_EXPORTS unsigned int GetEventFlags(eventNum_t eventnum);
		mfuse_EXPORTS const rawchar_t* GetEventName(eventNum_t eventnum);

		mfuse_EXPORTS const eventInfo_t& FindEventInfoChecked(eventName_t s);
		mfuse_EXPORTS const eventInfo_t* FindEventInfo(eventName_t s);
		mfuse_EXPORTS const eventInfo_t* FindEventInfo(const rawchar_t* s);
		mfuse_EXPORTS eventNum_t FindNormalEventNum(eventName_t s);
		mfuse_EXPORTS eventNum_t FindNormalEventNum(const rawchar_t* s);
		mfuse_EXPORTS eventNum_t FindReturnEventNum(eventName_t s);
		mfuse_EXPORTS eventNum_t FindReturnEventNum(const rawchar_t* s);
		mfuse_EXPORTS eventNum_t FindSetterEventNum(eventName_t s);
		mfuse_EXPORTS eventNum_t FindSetterEventNum(const rawchar_t* s);
		mfuse_EXPORTS eventNum_t FindGetterEventNum(eventName_t s);
		mfuse_EXPORTS eventNum_t FindGetterEventNum(const rawchar_t* s);

	private:
		EventSystem();
		~EventSystem();

		void LoadEvents();
		void UnloadEvents();
		void ShutdownEvents();
		void ArchiveEvents(Archiver &arc);
		void UnarchiveEvents(Archiver &arc);

		void SortEventList(con::Container<uintptr_t> *sortedList);
		void ClassEvents(const rawchar_t*classname, bool print_to_disk);
		void DumpClass(FILE * class_file, const rawchar_t* className);
		void DumpAllClasses();

		size_t GetRequiredLength(size_t& numUniqueEvents, size_t& numUniqueNameEvents, size_t& numNormals, size_t& numReturns, size_t& numGetters, size_t& numSetters) const;
		void PreallocateMemory(size_t totalLength, size_t numUniqueEvents, size_t numUniqueNameEvents, size_t numNormals, size_t numReturns, size_t numGetters, size_t numSetters);
	private:
#ifdef _WIN32
		static int compareEvents(void *context, const void *arg1, const void *arg2);
#else
		static int compareEvents(const void *arg1, const void *arg2, void *context);
#endif

	private:
		MEM::PreAllocator preAllocator;
		eventDefMap eventDefList;
		eventNameSet eventDefName;
		eventInfo_t* commandList;

	private:
		bool bEventSystemStarted;
		static size_t numEvents;
	};
};
