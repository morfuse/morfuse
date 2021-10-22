#pragma once

#include "Listener.h"
#include "ScriptVM.h"
#include "../Common/Time.h"
#include "../Container/ContainerView.h"

namespace mfuse
{
	class Flag;
	class EndOn;
	class ScriptMaster;
	class ScriptVM;
	class ScriptClass;

	extern EventDef EV_ScriptThread_CancelWaiting;

	class ScriptThread : public Listener
	{
		MFUS_CLASS_PROTOTYPE(ScriptThread);

		friend class ScriptClass;

	public:
		ScriptThread();
		ScriptThread(ScriptClass* scriptClass, const opval_t* pCodePos);
		virtual ~ScriptThread();

		void* operator new(size_t size);
		void operator delete(void* ptr);

		void Archive(Archiver &arc) override;
		void ArchiveInternal(Archiver& arc);
		void StartedWaitFor() override;
		void StoppedNotify() override;
		void StoppedWaitFor(const_str name, bool bDeleting) override;

		void Execute();
		void Execute(Event& ev);
		void DelayExecute();
		void DelayExecute(Event& ev);

		ScriptClass *GetScriptClass() const;
		ScriptVM *GetScriptVM() const;
		threadState_e GetThreadState();
		void SetThreadState(threadState_e newThreadState);

		void StartTiming(uinttime_t time);
		void StartTiming();
		void StartWaiting();

		void Pause();
		void ScriptExecute(const VarListView& data, ScriptVariable &returnValue);
		void Stop();
		void Wait(uinttime_t time);
		void Resume();

		void CharToInt(Event *ev);
		/*
		void FileOpen(Event *ev);
		void FileWrite(Event *ev);
		void FileRead(Event *ev);
		void FileClose(Event *ev);
		void FileEof(Event *ev);
		void FileSeek(Event *ev);
		void FileTell(Event *ev);
		void FileRewind(Event *ev);
		void FilePutc(Event *ev);
		void FilePuts(Event *ev);
		void FileGetc(Event *ev);
		void FileGets(Event *ev);
		void FileError(Event *ev);
		void FileFlush(Event *ev);
		void FileExists(Event *ev);
		void FileReadAll(Event *ev);
		void FileSaveAll(Event *ev);
		void FileRemove(Event *ev);
		void FileRename(Event *ev);
		void FileCopy(Event *ev);
		void FileReadPak(Event *ev);
		void FileList(Event *ev);
		void FileNewDirectory(Event *ev);
		void FileRemoveDirectory(Event *ev);
		*/
		void FlagClear(Event *ev);
		void FlagInit(Event *ev);
		void FlagSet(Event *ev);
		void FlagWait(Event *ev);
		void Lock(Event *ev);
		void UnLock(Event *ev);
		void GetArrayKeys(Event *ev);
		void GetArrayValues(Event *ev);
		void GetTime(Event *ev);
		void GetDate(Event *ev);
		void GetTimeZone(Event *ev);
		void PregMatch(Event *ev);
		void EventIsArray(Event *ev);
		void EventIsDefined(Event *ev);
		void MathCos(Event *ev);
		void MathSin(Event *ev);
		void MathTan(Event *ev);
		void MathACos(Event *ev);
		void MathASin(Event *ev);
		void MathATan(Event *ev);
		void MathATan2(Event *ev);
		void MathCosH(Event *ev);
		void MathSinH(Event *ev);
		void MathTanH(Event *ev);
		void MathExp(Event *ev);
		void MathFrexp(Event *ev);
		void MathLdexp(Event *ev);
		void MathLog(Event *ev);
		void MathLog10(Event *ev);
		void MathModf(Event *ev);
		void MathPow(Event *ev);
		void MathSqrt(Event *ev);
		void MathCeil(Event *ev);
		void MathFloor(Event *ev);
		void MathFmod(Event *ev);
		void Md5File(Event *ev);
		void Md5String(Event *ev);
		void SetTimer(Event *ev);
		void TypeOfVariable(Event *ev);
		void CancelWaiting(Event *ev);

		void Abs(Event *ev);

		void Angles_ToForward(Event *ev);
		void Angles_ToLeft(Event *ev);
		void Angles_ToUp(Event *ev);

		void Assert(Event *ev);
		void Cache(Event *ev);

		void CastBoolean(Event *ev);
		void CastFloat(Event *ev);
		void CastInt(Event *ev);
		void CastString(Event *ev);

		void CreateReturnThread(Event *ev);
		void CreateThread(Event *ev);
		void ExecuteReturnScript(Event *ev);
		void ExecuteScript(Event *ev);

		void EventCreateListener(Event *ev);
		void EventDelayThrow(Event *ev);
		void EventEnd(Event *ev);
		void EventTimeout(Event *ev);
		void EventError(Event *ev);
		void EventGoto(Event *ev);
		void EventRegisterCommand(Event *ev);
		void EventSightTrace(Event *ev);
		void EventTrace(Event *ev);
		void EventThrow(Event *ev);
		void EventWait(Event *ev);
		void EventWaitFrame(Event *ev);

		void GetSelf(Event *ev);

		void Println(Event *ev);
		void Print(Event *ev);
		void MPrintln(Event *ev);
		void MPrint(Event *ev);

		void EventBspTransition(Event *ev);
		void EventLevelTransition(Event *ev);
		void EventMissionTransition(Event *ev);

		void EventGetBoundKey1(Event *ev);
		void EventGetBoundKey2(Event *ev);
		void EventLocConvertString(Event *ev);

		void RandomFloat(Event *ev);
		void RandomInt(Event *ev);

		void Spawn(Event *ev);
		Listener* SpawnInternal(Event *ev);
		void SpawnReturn(Event *ev);

		void EventVectorAdd(Event *ev);
		void EventVectorCloser(Event *ev);
		void EventVectorCross(Event *ev);
		void EventVectorDot(Event *ev);
		void EventVectorLength(Event *ev);
		void EventVectorNormalize(Event *ev);
		void EventVectorScale(Event *ev);
		void EventVectorSubtract(Event *ev);
		void EventVectorToAngles(Event *ev);
		void EventVectorWithin(Event* ev);

	private:
		void ScriptExecuteInternal(const VarListView& data = VarListView());

	private:
		ScriptVM* m_ScriptVM;
		/** The current state of the thread itself. */
		threadState_e m_ThreadState;
	};

	typedef struct mutex_thread_list_s {
		SafePtr< ScriptThread >			m_pThread;
		struct mutex_thread_list_s		*next;
		struct mutex_thread_list_s		*prev;
	} mutex_thread_list_t;

	class ScriptMutex : public Listener
	{
		MFUS_CLASS_PROTOTYPE(ScriptMutex);

	public:
		ScriptMutex();
		~ScriptMutex();

		void StoppedNotify(void) override;

		void Lock();
		void Unlock();

	public:
		SafePtr< ScriptThread >		m_pLockThread;
		int							m_iLockCount;
		mutex_thread_list_t			m_list;

	private:
		void setOwner(ScriptThread* pThread);
		void Lock(mutex_thread_list_t* pList);
	};

	class Flag : public Class
	{
	public:
		xstr flagName;
		bool bSignaled;

	private:
		con::Container<ScriptVM *> m_WaitList;

	public:
		Flag();
		~Flag();

		void Reset(void);
		void Set(void);
		void Wait(ScriptThread *Thread);
	};

	class FlagList
	{
		friend class Flag;

	private:
		void AddFlag(Flag *flag);
		void RemoveFlag(Flag *flag);

	public:
		con::Container<Flag *> m_Flags;

		Flag *FindFlag(const rawchar_t* name);
	};

	namespace ScriptErrors
	{
		class Base : public ScriptExceptionBase {};

		class InvalidClassName : public Base, public Messageable
		{
		public:
			InvalidClassName(const xstr& classNameRef);

			const xstr& getClassName() const;
			const char* what() const noexcept override;

		private:
			xstr className;
		};
	}
}
