#pragma once

#include "../Global.h"
#include "Class.h"
#include "ScriptThread.h"
#include "Parm.h"
#include "StringResolvable.h"
#include "timer.h"
#include "../Container/set.h"
#include "../Container/arrayset.h"
#include "../Common/str.h"
#include "../Common/StringDictionary.h"
#include "../Common/Linklist.h"

#include <istream>

namespace mfuse
{
	class Listener;
	class ScriptClass;
	class ProgramScript;
	class FlagList;
	class StringDictionary;

	class ThreadExecutionProtection
	{
	public:
		mfuse_EXPORTS ThreadExecutionProtection();

		mfuse_EXPORTS uinttime_t GetMaxExecutionTime() const;
		mfuse_EXPORTS void SetMaxExecutionTime(uinttime_t time);

		mfuse_EXPORTS bool ShouldDrop() const;
		mfuse_EXPORTS void SetLoopProtection(bool protect);

	private:
		uinttime_t maxExecutionTime;
		bool loopProtection;
	};

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
		mfuse_EXPORTS const con::timer& GetTimerList() const;

		/** Return the dictionary used to map string to integers */
		mfuse_EXPORTS StringDictionary& GetDictionary();
		mfuse_EXPORTS const StringDictionary& GetDictionary() const;

		mfuse_EXPORTS void AddTiming(ScriptThread* Thread, uint64_t Time);
		mfuse_EXPORTS void RemoveTiming(ScriptThread* Thread);

		mfuse_EXPORTS ScriptClass* GetHeadContainer() const;

		mfuse_EXPORTS ThreadExecutionProtection& GetThreadExecutionProtection();
		mfuse_EXPORTS const ThreadExecutionProtection& GetThreadExecutionProtection() const;

		/**
		 * Create a new script thread.
		 *
		 * @param scr The program script to point the thread to.
		 * @param self The listener value of the self variable.
		 * @param label Label to start the thread to.
		 * @return Newly created thread.
		 */
		mfuse_EXPORTS ScriptThread* CreateScriptThread(const ProgramScript *scr, Listener *self, const StringResolvable& label);

		/**
		 * Create a new script thread.
		 *
		 * @param scriptClass The class to create the script thread in.
		 * @param label Label to start the thread to.
		 * @return Newly created thread.
		 */
		mfuse_EXPORTS ScriptThread* CreateScriptThread(ScriptClass *scriptClass, const StringResolvable& label);

		/**
		 * Create a new script thread.
		 *
		 * @param scriptClass The class to create the script thread in.
		 * @param codePos Initial code position for the thread.
		 * @return Newly created thread.
		 */
		mfuse_EXPORTS ScriptThread* CreateScriptThread(ScriptClass *scriptClass, const opval_t* codePos);
		mfuse_EXPORTS ScriptClass* CurrentScriptClass();

		/** Return the currently running thread. */
		mfuse_EXPORTS ScriptThread* CurrentThread() noexcept;
		/** Return the thread that was running before the current thread. */
		mfuse_EXPORTS ScriptThread* PreviousThread() noexcept;

		mfuse_EXPORTS ScriptThread* ExecuteThread(const ProgramScript* scr, const StringResolvable& label = {});
		mfuse_EXPORTS ScriptThread* ExecuteThread(const StringResolvable& scriptName, const StringResolvable& label = {});
		mfuse_EXPORTS ScriptThread* ExecuteThread(const ProgramScript* scr, Event& parms, const StringResolvable& label = {});
		mfuse_EXPORTS ScriptThread* ExecuteThread(const StringResolvable& scriptName, Event& parms, const StringResolvable& label = {});

		/**
		 * Compile and return an anonymous program script.
		 *
		 * @param stream The stream to compile.
		 * @return Anonymous program script.
		 */
		mfuse_EXPORTS const ProgramScript* GetTempScript(std::istream& stream);
		mfuse_EXPORTS const ProgramScript* GetProgramScript(const StringResolvable& scriptName, std::istream& stream, bool recompile = false);
		mfuse_EXPORTS const ProgramScript* GetProgramScript(const StringResolvable& scriptName, bool recompile = false);

		mfuse_EXPORTS void ExecuteRunning();
		mfuse_EXPORTS size_t GetNumScripts() const;
		mfuse_EXPORTS size_t GetNumRunningScripts() const;
		/** Reset the script master data, will kill all threads and remove game scripts. */
		mfuse_EXPORTS void Reset();

		void SetTime(uinttime_t time);

		mfuse_EXPORTS void Archive(Archiver& arc);

	public:
		FlagList flags;

	private:
		void InitConstStrings();
		const ProgramScript* GetProgramScriptInternal(const_str scriptName, std::istream& stream);
		void DeleteProgramScript(ProgramScript* script);
		ProgramScript* FindScript(const_str scriptName) const;

		void KillScripts();
		void CloseProgramScript();
		void ClearAll();

	private:
		SafePtr<ScriptThread> m_PreviousThread;
		SafePtr<ScriptThread> m_CurrentThread;
		con::map<const_str, ProgramScript*> m_ProgramScripts;
		con::timer timerList;
		ScriptClass* headScript;
		StringDictionary dict;
		Parm parm;
		ThreadExecutionProtection execProtection;
	};
};
