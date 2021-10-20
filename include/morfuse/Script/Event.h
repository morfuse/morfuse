#pragma once

#include "Class.h"
#include "ScriptVariable.h"
#include "../Container/Container.h"
#include "../Container/ContainerView.h"
#include "../Common/TimeTypes.h"

namespace mfuse
{
	/** Event flags */

	/** Allow entry from console */
	static constexpr unsigned int EV_CONSOLE			= (1<<0);
	/** Only allow entry from console if cheats are enabled */
	static constexpr unsigned int EV_CHEAT				= (1<<1);
	/** Hide from eventlist */
	static constexpr unsigned int EV_CODEONLY			= (1<<2);
	/** This event is used to cache data in */
	static constexpr unsigned int EV_CACHE				= (1<<3);
	/** This command only applies to TIKI files */
	static constexpr unsigned int EV_TIKIONLY			= (1<<4);
	/** This command only applies to SCRIPT files */
	static constexpr unsigned int EV_SCRIPTONLY			= (1<<5);
	/** Client : server command */
	static constexpr unsigned int EV_SERVERCMD			= (1<<6);
	/** default flag */
	static constexpr unsigned int EV_DEFAULT			= 0;

	enum class evType_e
	{
		/** Means 'anything' and is not intended to be used in scripts. */
		None = -1,
		/** Normal command. */
		Normal,
		/** Return as a function (local.var = local ReturnCommand). */
		Return,
		/** Return as a variable (local.var = local.listener.some_getter). */
		Getter,
		/** Set as a variable (local.listener.some_setter = "value"). */
		Setter
	};

	/**
	 * times for posting events
	 * Even though negative times technically don't make sense, the effect is to
	 * sort events that take place at the start of a map so that they are executed
	 * in the proper order.  For example, spawnargs must occur before any script
	 * commands take place, while unused entities must be removed before the spawnargs
	 * are parsed.
	 */

	/** remove any unused entities before spawnargs are parsed */
	static constexpr inttime_t EV_REMOVE			= -12000;
	/** for priority spawn args passed in by the bsp file */
	static constexpr inttime_t EV_PRIORITY_SPAWNARG	= -11000;
	/** for spawn args passed in by the bsp file */
	static constexpr inttime_t EV_SPAWNARG			= -10000;
	static constexpr inttime_t EV_PROCESS_INIT		= -6000;
	/** for any processing that must occur after all objects are spawned */
	static constexpr inttime_t EV_POSTSPAWN			= -5000;
	static constexpr inttime_t EV_SPAWNENTITIES		= -4000;

	using VarListView = con::ContainerView<const ScriptVariable>;

	class Listener;
	class SimpleEntity;
	class ScriptVariable;
	class Vector;
	class EventQueueNode;
	class Event;
	class ModuleDef;

	class EventDefAttributes
	{
	public:
		EventDefAttributes();
		EventDefAttributes(const rawchar_t* inName, evType_e type, eventNum_t num);
		EventDefAttributes(const EventDefAttributes& other);
		EventDefAttributes& operator=(const EventDefAttributes& other);
		EventDefAttributes(EventDefAttributes&& other);
		EventDefAttributes& operator=(EventDefAttributes&& other);

		bool operator==(const rawchar_t* inName) const;
		bool operator!=(const rawchar_t* inName) const;
		bool operator==(const EventDefAttributes& other) const;
		bool operator!=(const EventDefAttributes& other) const;

		const rawchar_t* GetString() const;
		eventNum_t GetNum() const;
		evType_e GetType() const;

	private:
		const rawchar_t* name;
		eventNum_t eventNum;
		evType_e type;
	};

	class EventDef
	{
	public:
		EventDef();

		// Arguments are : 'e' (Entity) 'v' (Vector) 'i' (Integer) 'f' (Float) 's' (String) 'b' (Boolean).
		// Uppercase arguments means optional.
		mfuse_EXPORTS EventDef(const rawchar_t* command, uint32_t flags, const rawchar_t* formatspec, const rawchar_t* argument_names, const rawchar_t* documentation, evType_e type = evType_e::Normal);
		mfuse_EXPORTS EventDef(const ModuleDef* def, const rawchar_t* command, uint32_t flags, const rawchar_t* formatspec, const rawchar_t* argument_names, const rawchar_t* documentation, evType_e type = evType_e::Normal);
		mfuse_EXPORTS ~EventDef();
		mfuse_EXPORTS EventDef(EventDef&& other);
		mfuse_EXPORTS EventDef& operator=(EventDef&& other);

		mfuse_EXPORTS bool operator==(const rawchar_t* name) const;
		mfuse_EXPORTS bool operator!=(const rawchar_t* name) const;
		mfuse_EXPORTS bool operator==(const EventDef& other) const;
		mfuse_EXPORTS bool operator!=(const EventDef& other) const;

		mfuse_EXPORTS const EventDefAttributes& GetAttributes() const;
		mfuse_EXPORTS EventDef* GetNext() const;
		mfuse_EXPORTS EventDef* GetPrev() const;
		mfuse_EXPORTS eventNum_t GetEventNum() const;
		mfuse_EXPORTS uint32_t GetFlags() const;
		mfuse_EXPORTS const rawchar_t* GetFormatSpec() const;
		mfuse_EXPORTS const rawchar_t* GetArgumentNames() const;
		mfuse_EXPORTS const rawchar_t* GetDocumentation() const;

		mfuse_EXPORTS static size_t GetDefCount();

	public:
		EventDef* next;
		EventDef* prev;
		using List = LinkedList<EventDef*, &EventDef::next, &EventDef::prev>;
		static const EventDef* GetHead();
		static List::iterator GetList();

	private:
		EventDefAttributes GetNewAttributes(const rawchar_t* command, evType_e type);

	public:
		const ModuleDef* moduleDef;
		EventDefAttributes attributes;
		const rawchar_t* formatspec;
		const rawchar_t* argument_names;
		const rawchar_t* documentation;
		size_t minArgs;
		uint32_t flags;

	private:
		static List head;
		static size_t defCount;
	};

	class Event : public Class
	{
		friend class ScriptVM;
		friend class VMOperation;

		MFUS_CLASS_PROTOTYPE(Event);

	public:
		mfuse_EXPORTS Event();
		mfuse_EXPORTS Event(const EventDef& def);
		mfuse_EXPORTS Event(const EventDef& def, size_t numArgs);
		mfuse_EXPORTS Event(uintptr_t eventnum);
		mfuse_EXPORTS Event(uintptr_t eventnum, size_t numArgs);
		mfuse_EXPORTS Event(const Event& other);
		mfuse_EXPORTS Event(Event&& other);
		mfuse_EXPORTS ~Event();

		mfuse_EXPORTS Event& operator=(const Event& other);
		mfuse_EXPORTS Event& operator=(Event&& other);

		mfuse_EXPORTS static void* operator new(size_t size);
		mfuse_EXPORTS static void operator delete(void* ptr);

		mfuse_EXPORTS bool operator==(const Event& ev);
		mfuse_EXPORTS bool operator!=(const Event& ev);

		void Archive(Archiver &arc) override;

		mfuse_EXPORTS const EventDef* GetInfo() const;
		mfuse_EXPORTS const rawchar_t* GetName() const;
		mfuse_EXPORTS const rawchar_t* GetFormat() const;
		mfuse_EXPORTS eventNum_t Num() const;
		mfuse_EXPORTS bool IsFromScript() const;

		/** Return the number of arguments in the event. */
		mfuse_EXPORTS size_t NumArgs() const;

		mfuse_EXPORTS void Clear();
		mfuse_EXPORTS void ReserveArguments(size_t count);

		mfuse_EXPORTS void AddContainer(const con::Container<SafePtr<Listener>>* container);
		mfuse_EXPORTS void AddFloat(float number);
		mfuse_EXPORTS void AddInteger(int32_t number);
		mfuse_EXPORTS void AddLong(int64_t number);
		mfuse_EXPORTS void AddListener(Listener* listener);
		mfuse_EXPORTS void AddNil();
		mfuse_EXPORTS void AddConstString(const_str string);
		mfuse_EXPORTS void AddString(const xstr& string);
		mfuse_EXPORTS void AddString(const rawchar_t* string);
		mfuse_EXPORTS void AddTokens(size_t argc, const rawchar_t** argv);
		mfuse_EXPORTS void AddValue(const ScriptVariable& value);
		mfuse_EXPORTS void AddValue(ScriptVariable&& value);
		mfuse_EXPORTS void AddVector(const Vector& vector);

		mfuse_EXPORTS bool GetBoolean(uintptr_t pos);
		mfuse_EXPORTS const_str GetConstString(uintptr_t pos);
		mfuse_EXPORTS float GetFloat(uintptr_t pos);
		mfuse_EXPORTS int GetInteger(uintptr_t pos);
		mfuse_EXPORTS Listener* GetListener(uintptr_t pos);

		mfuse_EXPORTS xstr GetString(uintptr_t pos);
		mfuse_EXPORTS ScriptVariable& GetValue(uintptr_t pos);
		mfuse_EXPORTS ScriptVariable& GetValueChecked(uintptr_t pos);
		mfuse_EXPORTS ScriptVariable& GetValue();
		mfuse_EXPORTS VarListView GetListView() const;
		mfuse_EXPORTS VarListView GetListView(uintptr_t startPos) const;
		mfuse_EXPORTS ScriptVariable* GetData();
		mfuse_EXPORTS const ScriptVariable* GetData() const;
		mfuse_EXPORTS Vector GetVector(uintptr_t pos);

		mfuse_EXPORTS bool IsListenerAt(uintptr_t pos);
		mfuse_EXPORTS bool IsNilAt(uintptr_t pos);
		mfuse_EXPORTS bool IsNumericAt(uintptr_t pos);
		mfuse_EXPORTS bool IsStringAt(uintptr_t pos);
		mfuse_EXPORTS bool IsVectorAt(uintptr_t pos);

	private:
		template<typename...Args>
		void ConstructValue(Args&&... args)
		{
			data.AddObject(std::forward<Args>(args)...);
		}

		void* GetUninitializedValue();
		void CheckPos(uintptr_t pos);

	private:
		con::Container<ScriptVariable> data;
		eventNum_t eventnum;

	protected:
		bool fromScript;
	};
};
