#pragma once

#include "../Global.h"
#include "../Script/Class.h"
#include "../Script/ScriptThread.h"
#include "../Container/set.h"
#include "../Container/arrayset.h"
#include "../Common/str.h"
#include "../Container/timer.h"
#include "../Script/Parm.h"

namespace mfuse
{
	class Listener;
	class ScriptClass;
	class ProgramScript;
	class FlagList;

	/**
	 * This is a class that handles string management, thread scheduling and creation, and compiling game script.
	 */
	class ScriptMaster
	{
		friend class ScriptClass;
		friend class ScriptThread;
		friend class Flag;

	public:
		ScriptMaster();
		~ScriptMaster();

		mfuse_EXPORTS Parm& GetParm();

		mfuse_EXPORTS const_str AddString(xstrview s);
		mfuse_EXPORTS const_str GetString(const rawchar_t* s);
		mfuse_EXPORTS void AllocateMoreString(size_t count);

		mfuse_EXPORTS const xstr& GetString(const_str s);

		mfuse_EXPORTS void AddTiming(ScriptThread* Thread, uint64_t Time);
		mfuse_EXPORTS void RemoveTiming(ScriptThread* Thread);

		mfuse_EXPORTS uintptr_t GetStackCount() const;
		mfuse_EXPORTS void AddStack();
		mfuse_EXPORTS void RemoveStack();

		mfuse_EXPORTS bool HasLoopDrop() const;
		mfuse_EXPORTS bool HasLoopProtection() const;
		mfuse_EXPORTS ScriptClass* GetHeadContainer() const;

		mfuse_EXPORTS ScriptThread* CreateScriptThread(const ProgramScript *scr, Listener *self, const_str label);
		mfuse_EXPORTS ScriptThread* CreateScriptThread(const ProgramScript *scr, Listener *self, const rawchar_t* label);
		mfuse_EXPORTS ScriptThread* CreateScriptThread(ScriptClass *ScriptClass, const_str label);
		mfuse_EXPORTS ScriptThread* CreateScriptThread(ScriptClass *ScriptClass, const rawchar_t* label);
		mfuse_EXPORTS ScriptThread* CreateScriptThread(ScriptClass *ScriptClass, const opval_t* m_pCodePos);
		mfuse_EXPORTS ScriptThread* CreateThread(const ProgramScript* scr, const rawchar_t* label, Listener* self = NULL);
		mfuse_EXPORTS ScriptThread* CreateThread(const ProgramScript* scr, const_str label, Listener* self = NULL);
		mfuse_EXPORTS ScriptThread* CreateThread(const rawchar_t* scriptName, const rawchar_t* label, Listener* self = NULL);
		mfuse_EXPORTS ScriptThread* CreateThread(const rawchar_t* scriptName, const_str label, Listener* self = NULL);
		mfuse_EXPORTS ScriptClass* CurrentScriptClass();

		mfuse_EXPORTS ScriptThread* CurrentThread();
		mfuse_EXPORTS ScriptThread* PreviousThread();

		mfuse_EXPORTS void ExecuteThread(const ProgramScript* scr, const rawchar_t* label = nullptr);
		mfuse_EXPORTS void ExecuteThread(const ProgramScript* scr, const rawchar_t* label, Event& parms);
		mfuse_EXPORTS void ExecuteThread(const rawchar_t* scriptName, const rawchar_t* label = nullptr);
		mfuse_EXPORTS void ExecuteThread(const rawchar_t* scriptName, const rawchar_t* label, Event& parms);

		mfuse_EXPORTS const ProgramScript* GetTempScript(const char* data, uint64_t dataLength);
		mfuse_EXPORTS const ProgramScript* GetGameScript(const rawchar_t* scriptName, const char* data, uint64_t dataLength, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetGameScript(const_str scriptName, const char* data, uint64_t dataLength, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetGameScript(const rawchar_t* scriptName, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetGameScript(const_str scriptName, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetScript(const rawchar_t* scriptName, const char* data, uint64_t dataLength, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetScript(const_str scriptName, const char* data, uint64_t dataLength, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetScript(const rawchar_t* scriptName, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetScript(const_str scriptName, bool recompile = false);

		mfuse_EXPORTS void ExecuteRunning();
		void SetTime(uinttime_t time);

	public:
		FlagList flags;

	private:
		void InitConstStrings();
		const ProgramScript* GetGameScriptInternal(const_str scriptName, const rawchar_t* data, size_t dataLength);

		void CloseGameScript();
		void ClearAll();
		void Reset();

	private:
		/** VM recursions */
		int32_t stackCount;

		/** The previous thread. */
		SafePtr<ScriptThread> m_PreviousThread;

		/** The current playing thread. */
		SafePtr<ScriptThread> m_CurrentThread;

		/** The list of compiled game scripts. */
		con::map<const_str, ProgramScript*> m_GameScripts;

		/** Threads that are currently waiting */
		con::timer timerList;

		/** The string dictionary, used to cache strings into a number. */
		con::arrayset<xstr, xstr, Hash<xstr>, EqualTo<xstr>, MEM::DefaultAlloc_set> StringDict;

		/** The head of the script container. */
		ScriptClass* ContainerHead;

		/** Parm instance for scripts. */
		Parm parm;

	private:
		static const rawchar_t* ConstStrings[];
	};
};
