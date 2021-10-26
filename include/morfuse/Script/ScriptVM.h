#pragma once

#include "Class.h"
#include "Event.h"
#include "ScriptVariable.h"
#include "ScriptOpcodes.h"
#include "StringResolvable.h"
#include "../Common/TimeTypes.h"

namespace mfuse
{
	/** The maximum variable stack size (not used as it's dynamic now). */
	constexpr unsigned int SCRIPTTHREAD_VARSTACK_SIZE = 255;
	/** Maximum number of opcodes to process at once. */
	constexpr unsigned int MAX_SCRIPTCYCLES = 9999;

	class EventSystem;
	class ScriptClass;
	class ScriptContext;
	class ScriptExceptionBase;
	class ScriptMaster;
	class StateScript;
	class ScriptThread;
	class StringResolvable;
	class OutputInfo;

	enum class vmState_e
	{
		/** Currently executing code. */
		Running,
		/** Suspended and will be resumed as soon as possible. */
		Suspended,
		/** Pending destroy. */
		Destroy,
		/** VM is not executing, ready to be fired. */
		Idling,
		/** Pending deletion. */
		Destroyed
	};

	enum class threadState_e
	{
		/** Thread is running. */
		Running,
		/** Thread is scheduled to be resumed after a certain time. */
		Timing,
		/** Thread is suspended, waiting for something, unknown when it is going to be resumed. */
		Waiting,
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

	/*
	class ScriptStack
	{
	public:
		ScriptVariable* m_Array;
		uint32_t m_Count;
	};
	*/

	class ScriptStack
	{
	public:
		ScriptStack();
		ScriptStack(size_t stackSize);
		~ScriptStack();

		ScriptStack(const ScriptStack& other) = delete;
		ScriptStack& operator=(const ScriptStack& other) = delete;
		ScriptStack(ScriptStack&& other);
		ScriptStack& operator=(ScriptStack&& other);

		void Archive(Archiver& arc);

		size_t GetStackSize() const;
		ScriptVariable& SetTop(ScriptVariable& newTop);
		ScriptVariable& GetTop() const;
		ScriptVariable& GetTop(size_t offset) const;
		ScriptVariable* GetTopPtr() const;
		ScriptVariable* GetTopPtr(size_t offset) const;
		ScriptVariable* GetTopArray(size_t offset = 0) const;
		uintptr_t GetIndex() const;
		void MoveTop(ScriptVariable&& other);
		ScriptVariable* GetListenerVar(uintptr_t index);
		void SetListenerVar(uintptr_t index, ScriptVariable* newVar);

		/** Pop and return the previous value. */
		ScriptVariable& Pop();
		ScriptVariable& Pop(size_t offset);
		ScriptVariable& PopAndGet();
		ScriptVariable& PopAndGet(size_t offset);
		/** Push and return the previous value. */
		ScriptVariable& Push();
		ScriptVariable& Push(size_t offset);
		ScriptVariable& PushAndGet();
		ScriptVariable& PushAndGet(size_t offset);

	private:
		/** The VM's local stack. */
		ScriptVariable* localStack;
		/** The local stack size. */
		ScriptVariable* stackBottom;
		/** Variable from the top stack of the local stack. */
		ScriptVariable* pTop;
		ScriptVariable** listenerVarPtr;
	};

	class ScriptVM
	{
		friend class ScriptClass;
		friend class ScriptThread;
		friend class VMOperation;

	public:
		ScriptVM();
		ScriptVM(ScriptClass *ScriptClass, const opval_t*pCodePos, ScriptThread *thread);
		~ScriptVM();

		void* operator new(size_t size);
		void operator delete(void* ptr);

		void Archive(Archiver& arc);

		void EnterFunction(ScriptVariableContainer&& data);
		void LeaveFunction();

		void End(const ScriptVariable& returnValue);
		void End();

		void Execute(const VarListView& data = VarListView(), const StringResolvable& label = StringResolvable());
		void NotifyDelete();
		void Resume(bool bForce = false);
		void Suspend();

		const_str Filename();
		const_str Label();
		ScriptClass* GetScriptClass() const;
		void ClearScriptClass();

		ScriptThread* GetScriptThread() const;
		ScriptVM* GetNext() const;
		void SetNext(ScriptVM* newNext);

		bool IsSuspended();
		vmState_e State();

		void EventGoto(Event& ev);
		bool EventThrow(Event& ev);

	private:
		bool Process(ScriptContext& context, uinttime_t interruptTime);

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

		template<bool bMethod = false, bool bReturn = false>
		void executeCommand(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum);
		void executeCommandInternal(Event& ev, Listener* listener, ScriptVariable* fromVar, op_parmNum_t iParamCount);
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

		void ExecCmdCommon(op_parmNum_t param);
		void ExecCmdMethodCommon(op_parmNum_t param);
		void ExecMethodCommon(op_parmNum_t param);
		void ExecFunction(ScriptMaster& Director);

		void SetFastData(const VarListView& data);
		void SetFastData(ScriptVariableContainer&& data);
		void SetFastData(Event&& ev);

		bool Switch(const StateScript* stateScript, ScriptVariable& var);

		const opval_t* ProgBuffer();
		void HandleScriptException(const std::exception& exc, const OutputInfo& info);
		void HandleScriptExceptionAbort(const OutputInfo& info);

	private:
		/** The next VM in script class. */
		ScriptVM* next;
		/** The owning ScriptThread. */
		ScriptThread* m_Thread;
		/** The VM in the group of threads. */
		ScriptClass* m_ScriptClass;

	private:
		/** This is the stack but it is unused, should consider removing it later. */
		ScriptStack m_Stack;
		/** The return value that is set when the VM is exiting. */
		ScriptVariable m_ReturnValue;
		/** Current code position. */
		const opval_t* m_CodePos;
		/** Previous op codes for use with script exceptions. */
		const opval_t* m_PrevCodePos;

	private:
		/** The callstack. */
		con::Container<ScriptCallStack*> callStack;
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
		/** If it has marked stack (with OP_MARK_STACK_POS and OP_RESTORE_STACK_POS). */
		bool m_bMarkStack;
	};

	class ScriptExecutionStack
	{
	public:
		ScriptExecutionStack();
		~ScriptExecutionStack();

		/**
		 * Set the maximum number of allowed interpreters to stack
		 *
		 * @param maxDepth the maximum number of interpreters
		 */
		mfuse_EXPORTS static void SetMaxStackDepth(size_t maxDepth);

		/**
		 * Return the maximum interpreter stack depth.
		 */
		mfuse_EXPORTS static size_t GetMaxStackDepth();

		/**
		 * Get the current interpreter stack depth.
		 */
		mfuse_EXPORTS static size_t GetStackDepth();

	private:
		static thread_local size_t stackDepth;
		static thread_local size_t maxStackDepth;
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

	namespace ScriptVMErrors
	{
		class BaseAbort : public ScriptAbortExceptionBase
		{};

		class BaseWarning : public ScriptExceptionBase
		{};

		class StackError : public BaseAbort, public Messageable
		{
		public:
			StackError(intptr_t stackVal);

			intptr_t GetStack() const noexcept;
			const char* what() const noexcept override;

		private:
			intptr_t stack;
		};

		class CommandOverflow : public BaseAbort
		{
		public:
			const char* what() const noexcept override;
		};

		class MaxStackDepth : public BaseAbort
		{
		public:
			const char* what() const noexcept override;
		};

		class NegativeStackDepth : public BaseAbort
		{
		public:
			const char* what() const noexcept override;
		};

		class NullListenerField : public BaseWarning, public Messageable
		{
		public:
			NullListenerField(const_str fieldNameValue);

			const_str GetFieldName() const;
			const char* what() const noexcept override;

		private:
			const_str fieldName;
		};

		class NilListenerCommand : public BaseWarning, public Messageable
		{
		public:
			NilListenerCommand(eventNum_t eventNumValue);

			eventNum_t GetEventNum() const;
			const char* what() const noexcept override;

		private:
			eventNum_t eventNum;
		};

		class NullListenerCommand : public BaseWarning, public Messageable
		{
		public:
			NullListenerCommand(eventNum_t eventNumValue);

			eventNum_t GetEventNum() const;
			const char* what() const noexcept override;

		private:
			eventNum_t eventNum;
		};
	}
};
