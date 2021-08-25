#pragma once

#include "Class.h"
#include "Event.h"
#include "ScriptVariable.h"
#include "ScriptOpcodes.h"

namespace mfuse
{
	/** The maximum number of thread execution depth. */
	constexpr unsigned int MAX_STACK_DEPTH = 20;
	/** The maximum variable stack size (not used as it's dynamic now). */
	constexpr unsigned int SCRIPTTHREAD_VARSTACK_SIZE = 255;
	/** Maximum number of opcodes to process at once. */
	constexpr unsigned int MAX_SCRIPTCYCLES = 9999;

	class EventSystem;
	class ScriptClass;
	class ScriptContext;
	class ScriptException;
	class ScriptMaster;
	class StateScript;
	class ScriptThread;

	enum class vmState_e
	{
		/** Currently executing code. */
		Running,
		/** Suspended and will be resumed as soon as possible. */
		Suspended,
		/** Waiting for a specific event to end. */
		Waiting,
		/** Resume to execution. */
		Execution,
		/** Pending deletion. */
		Destroyed
	};

	enum class threadState_e
	{
		Running,
		Waiting,
		Suspended,
	};

	class ScriptCallStack
	{
	public:
		/** The code position after the function, will be restored. */
		const opval_t* codePos;

		/** Previous local stack. */
		ScriptVariable* localStack;
		/** Previous top value. */
		ScriptVariable* pTop;

		/** The return value that is saved before entering function. */
		ScriptVariable returnValue;

		/** The previous self value. */
		SafePtr<Listener> m_Self;
	};

	class ScriptStack
	{
	public:
		ScriptVariable* m_Array;
		uint32_t m_Count;
	};

	class ScriptVM
	{
		friend class ScriptThread;
		friend class VMOperation;

	public:
		ScriptVM(ScriptClass *ScriptClass, const opval_t*pCodePos, ScriptThread *thread);
		~ScriptVM();

		void* operator new(size_t size);
		void operator delete(void* ptr);

		void Archive(Archiver& arc);

		void EnterFunction(ScriptMaster& Director, ScriptVariableContainer&& data);
		void LeaveFunction();

		void End(const ScriptVariable& returnValue);
		void End();

		void Execute(const ScriptVariable *data = nullptr, size_t dataSize = 0, const rawchar_t* label = nullptr);
		void NotifyDelete();
		void Resume(bool bForce = false);
		void Suspend();

		const xstr& Filename();
		const xstr& Label();
		ScriptClass* GetScriptClass() const;
		void ClearScriptClass();

		ScriptThread* GetScriptThread() const;
		ScriptVM* GetNext() const;
		void SetNext(ScriptVM* newNext);

		bool IsSuspended();
		vmState_e State();
		threadState_e ThreadState();
		void SetThreadState(threadState_e newThreadState);

		void EventGoto(Event *ev);
		bool EventThrow(Event* ev);

	private:
		void ReadOpcodeValue(void* outValue, size_t size);
		void ReadGetOpcodeValue(void* outValue, size_t size);

		template<typename T> T ReadOpcodeValue()
		{
			T value;
			ReadOpcodeValue(&value, sizeof(T));
			return value;
			/*
			T value = *reinterpret_cast<const T*>(m_CodePos);
			m_CodePos += sizeof(T);
			return value;
			*/
		}

		template<typename T> T ReadGetOpcodeValue()
		{
			T value;
			ReadGetOpcodeValue(&value, sizeof(T));
			return value;
			//return *reinterpret_cast<const T*>(m_CodePos);
		}

		void error(const rawchar_t* format, ...);
		template<bool bMethod = false, bool bReturn = false>
		void executeCommand(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum);
		void executeCommandInternal(Event& ev, Listener* listener, ScriptVariable* fromVar, op_parmNum_t iParamCount, op_evName_t eventnum);
		bool executeGetter(EventSystem& eventSystem, Listener* listener, op_evName_t eventName);
		bool executeSetter(EventSystem& eventSystem, Listener* listener, op_evName_t eventName);

		void jump(op_offset_t offset);
		void jumpBack(op_offset_t offset);
		void jumpBool(op_offset_t offset, bool booleanValue);
		bool jumpVar(op_offset_t offset, bool booleanValue);
		void doJumpIf(bool booleanValue);
		bool doJumpVarIf(bool booleanValue);

		template<bool noTop = false> void loadTop(EventSystem& eventSystem, Listener* listener);
		template<bool noTop = false> ScriptVariable* storeTop(EventSystem& eventSystem, Listener* listener);
		void storeField(op_name_t fieldName, Listener* listener);
		void skipField();

		void ExecCmdCommon(EventSystem& eventSystem, op_parmNum_t param);
		void ExecCmdMethodCommon(EventSystem& eventSystem, op_parmNum_t param);
		void ExecMethodCommon(EventSystem& eventSystem, op_parmNum_t param);
		void ExecFunction(ScriptMaster& Director);

		void SetFastData(const ScriptVariable* data, size_t dataSize);
		void SetFastData(ScriptVariableContainer&& data);
		void SetFastData(Event&& ev);

		bool Switch(const StateScript* stateScript, ScriptVariable& var);

		const opval_t* ProgBuffer();
		void HandleScriptException(ScriptException& exc, std::ostream* out);
		void HandleScriptExceptionAbort(ScriptException& exc, std::ostream* out);

	private:
		/** The next VM in script class. */
		ScriptVM* next;
		/** The owning ScriptThread. */
		ScriptThread* m_Thread;
		/** The VM in the group of threads. */
		ScriptClass* m_ScriptClass;

	private:
		/** This is the stack but it is unused, should consider removing it later. */
		ScriptStack* m_Stack;
		/** The return value that is set when the VM is exiting. */
		ScriptVariable m_ReturnValue;

		/** Previous op codes for use with script exceptions. */
		const opval_t* m_PrevCodePos;
		/** Current code position. */
		const opval_t* m_CodePos;

	private:
		/** The callstack. */
		con::Container<ScriptCallStack*> callStack;
		/** The VM's local stack. */
		ScriptVariable* localStack;
		/** The local stack size. */
		ScriptVariable* stackBottom;
		/** Variable from the top stack of the local stack. */
		ScriptVariable* pTop;
		ScriptVariable** listenerVarPtr;
		/** The marked stack position. */
		ScriptVariable* m_StackPos;

		/** List of parameters when calling script commands. */
		Event fastEvent;
		uintptr_t fastIndex;
		/** Data from fastEvent. */
		//ScriptVariable* m_pOldData;
		/** Size of the old data. */
		//size_t m_OldDataSize;
		//con::Container<ScriptVariable> m_OldData;
		/** The current state of the VM. */
		vmState_e state;
		/** The current state of the thread itself. */
		threadState_e m_ThreadState;
		/** If it has marked stack (with OP_MARK_STACK_POS and OP_RESTORE_STACK_POS). */
		bool m_bMarkStack;
	};

	template<>
	inline uint8_t ScriptVM::ReadOpcodeValue<uint8_t>()
	{
		return *m_CodePos++;
	}

	template<>
	inline uint8_t ScriptVM::ReadGetOpcodeValue<uint8_t>()
	{
		return *m_CodePos;
	}

};
