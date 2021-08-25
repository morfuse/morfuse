#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/Game.h>
#include <morfuse/Script/Parm.h>
#include <morfuse/Script/Listener.h>
#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/ScriptOpcodes.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Common/short3.h>
#include "ScriptVMOperation.h"
#include <cstdarg>

using namespace mfuse;

ScriptVM::ScriptVM(ScriptClass *scriptClass, const opval_t *pCodePos, ScriptThread *thread)
{
	next = NULL;

	m_Thread = thread;
	m_ScriptClass = scriptClass;

	m_Stack = NULL;

	m_PrevCodePos = NULL;
	m_CodePos = pCodePos;

	state = vmState_e::Running;
	m_ThreadState = threadState_e::Running;

	//m_pOldData = NULL;
	//m_OldDataSize = 0;

	m_bMarkStack = false;
	m_StackPos = NULL;

	size_t localStackSize = m_ScriptClass->GetScript()->GetRequiredStackSize();
	if (!localStackSize) localStackSize = 1;

	// allocate at once
	uint8_t* data = new uint8_t[(sizeof(ScriptVariable) + sizeof(ScriptVariable*)) * localStackSize];
	localStack = new (data) ScriptVariable[localStackSize];
	data += sizeof(ScriptVariable) * localStackSize;

	pTop = localStack;
	listenerVarPtr = new (data) ScriptVariable*[localStackSize]();
	//fastEvent = new Event;
	fastIndex = 0;

	stackBottom = localStack + localStackSize;

	m_ScriptClass->AddThread(this);
}

/*
====================
~ScriptVM
====================
*/
ScriptVM::~ScriptVM()
{
	//fastEvent.data = m_pOldData;
	//fastEvent.dataSize = m_OldDataSize;
	//fastEvent.data = std::move(m_OldData);

	// clean-up the call stack
	while (callStack.NumObjects()) {
		LeaveFunction();
	}
	
	const size_t localStackSize = stackBottom - localStack;
	for(uintptr_t i = 0; i < localStackSize; ++i) {
		localStack[i].~ScriptVariable();
	}
	
	uint8_t* data = (uint8_t*)localStack;
	delete[] data;
	// listenerVarPtr will also be deleted as it was allocated in the same chunk
	//delete fastEvent;
}

void* ScriptVM::operator new(size_t size)
{
	return ScriptContext::Get().GetAllocator().GetBlock<ScriptVM>().Alloc();
	//return allocateMemory(size);
}

void ScriptVM::operator delete(void* ptr)
{
	ScriptContext::Get().GetAllocator().GetBlock<ScriptVM>().Free(ptr);
	//freeMemory(ptr);
}

/*
====================
Archive
====================
*/
void ScriptVM::Archive(Archiver& arc)
{
	/*
	int stack = 0;

	if (arc.Saving())
	{
		if (m_Stack)
			stack = m_Stack->m_Count;

		arc.ArchiveInteger(&stack);
	}
	else
	{
		arc.ArchiveInteger(&stack);

		if (stack)
		{
			m_Stack = new ScriptStack;
			m_Stack->m_Array = new ScriptVariable[stack];
			m_Stack->m_Count = stack;
		}
	}

	for (int i = 1; i <= stack; i++)
	{
		m_Stack->m_Array[i].ArchiveInternal(arc);
	}

	m_ReturnValue.ArchiveInternal(arc);
	m_ScriptClass->ArchiveCodePos(arc, &m_PrevCodePos);
	m_ScriptClass->ArchiveCodePos(arc, &m_CodePos);
	arc.ArchiveByte(&state);
	arc.ArchiveByte(&m_ThreadState);
	*/
}

/*
====================
error

Triggers an error
====================
*/
void ScriptVM::error(const rawchar_t *format, ...)
{
	rawchar_t buffer[4000];
	va_list va;

	va_start(va, format);
	vsprintf(buffer, format, va);
	va_end(va);

	//glbs.Printf("----------------------------------------------------------\n%s\n", buffer);
	m_ReturnValue.setStringValue("$.INTERRUPTED");
}

const opval_t *ScriptVM::ProgBuffer()
{
	return m_CodePos;
}

void ScriptVM::EnterFunction(ScriptMaster& Director, ScriptVariableContainer&& data)
{
	ScriptCallStack *stack;
	const const_str label = data[0].constStringValue();

	SetFastData(std::move(data));

	const opval_t *codePos = m_ScriptClass->FindLabel(label);

	if (!codePos)
	{
		const xstr& labelName = Director.GetString(label);
		ScriptError("ScriptVM::EnterFunction: label '%s' does not exist in '%s'.", labelName.c_str(), Filename().c_str());
	}

	stack = new ScriptCallStack;

	stack->codePos = m_CodePos;

	stack->pTop = pTop;
	stack->returnValue = m_ReturnValue;
	stack->localStack = localStack;
	stack->m_Self = m_ScriptClass->GetSelf();

	callStack.AddObject(stack);

	m_CodePos = codePos;

	const size_t localStackSize = stackBottom - localStack;
	localStack = new ScriptVariable[localStackSize];

	pTop = localStack;
	m_ReturnValue.Clear();
}

void ScriptVM::LeaveFunction()
{
	size_t num = callStack.NumObjects();

	if (num)
	{
		ScriptCallStack *stack = callStack.ObjectAt(num);

		pTop = stack->pTop;
		*pTop = m_ReturnValue;

		m_CodePos = stack->codePos;
		m_ReturnValue = stack->returnValue;
		m_ScriptClass->SetSelf(stack->m_Self);

		delete localStack;

		localStack = stack->localStack;

		delete stack;

		callStack.RemoveObjectAt(num);
	}
	else
	{
		delete m_Thread;
	}
}

void ScriptVM::End(const ScriptVariable& returnValue)
{
	m_ReturnValue.setPointer(returnValue);

	LeaveFunction();
}

void ScriptVM::End()
{
	m_ReturnValue.ClearPointer();

	LeaveFunction();
}

void ScriptVM::HandleScriptException(ScriptException& exc, std::ostream* out)
{
	if (m_ScriptClass)
	{
		const ProgramScript* const scr = m_ScriptClass->GetScript();
		scr->PrintSourcePos(m_PrevCodePos - scr->GetProgBuffer());
	}

	if (out)
	{
		// write debug message
		*out << "^~^~^ Script Error : " << exc.string.c_str() << "\n\n";
	}
}

void ScriptVM::HandleScriptExceptionAbort(ScriptException& exc, std::ostream* out)
{
	if (m_ScriptClass)
	{
		const ProgramScript* const scr = m_ScriptClass->GetScript();
		scr->PrintSourcePos(m_PrevCodePos - scr->GetProgBuffer());
	}

	state = vmState_e::Execution;
	throw exc;
}

void ScriptVM::SetFastData(const ScriptVariable* data, size_t dataSize)
{
	fastEvent.Clear();
	fastIndex = 0;

	if (dataSize)
	{
		fastEvent.data.SetNumObjects(dataSize);
		fastIndex = 0;

		for (uintptr_t i = 0; i < dataSize; i++)
		{
			fastEvent.data[i] = std::move(data[i]);
		}
	}
}

void ScriptVM::SetFastData(ScriptVariableContainer&& data)
{
	fastEvent.data = std::move(data);
	fastIndex = 0;
}

void ScriptVM::SetFastData(Event&& ev)
{
	fastEvent = std::move(ev);
	fastIndex = 0;
}

void ScriptVM::NotifyDelete()
{
	switch (state)
	{
	case vmState_e::Destroyed:
		ScriptError("Attempting to delete a dead thread.");
		break;

	case vmState_e::Running:
	case vmState_e::Suspended:
	case vmState_e::Waiting:
		state = vmState_e::Destroyed;

		if (m_ScriptClass) {
			m_ScriptClass->RemoveThread(this);
		}

		break;

	case vmState_e::Execution:
		state = vmState_e::Destroyed;

		if (m_ScriptClass) {
			m_ScriptClass->RemoveThread(this);
		}

		delete this;
		break;
	}
}

void ScriptVM::Resume(bool bForce)
{
	if (state == vmState_e::Suspended || (bForce && state != vmState_e::Destroyed))
	{
		state = vmState_e::Running;
	}
}

void ScriptVM::Suspend()
{
	if (state == vmState_e::Destroyed) {
		ScriptError("Cannot suspend a dead thread.");
	}
	else if (state == vmState_e::Running) {
		state = vmState_e::Suspended;
	}
}

bool ScriptVM::Switch(const StateScript *stateScript, ScriptVariable& var)
{
	//fastEvent.dataSize = 0;

	const opval_t* pos = stateScript->FindLabel(var.constStringValue());

	if (!pos)
	{
		pos = stateScript->FindLabel(STRING_EMPTY);

		if (!pos) {
			return false;
		}
	}

	m_CodePos = pos;

	return true;
}

const xstr& ScriptVM::Filename()
{
	return m_ScriptClass->Filename();
}

const xstr& ScriptVM::Label()
{
	// FIXME: Find VM label
	//const_str label = m_ScriptClass->NearestLabel(m_CodePos);
	ScriptMaster& Director = ScriptContext::Get().GetDirector();
	return Director.GetString(STRING_EMPTY);
}

ScriptClass *ScriptVM::GetScriptClass() const
{
	return m_ScriptClass;
}

void ScriptVM::ClearScriptClass()
{
	m_ScriptClass = nullptr;
}

ScriptThread* ScriptVM::GetScriptThread() const
{
	return m_Thread;
}

ScriptVM* ScriptVM::GetNext() const
{
	return next;
}

void ScriptVM::SetNext(ScriptVM* newNext)
{
	next = newNext;
}

bool ScriptVM::IsSuspended()
{
	return state == vmState_e::Suspended;
}

vmState_e ScriptVM::State()
{
	return state;
}

threadState_e ScriptVM::ThreadState()
{
	return m_ThreadState;
}

void ScriptVM::SetThreadState(threadState_e newThreadState)
{
	m_ThreadState = newThreadState;
}

void ScriptVM::EventGoto(Event *ev)
{
	const const_str label = ev->GetConstString(1);

	SetFastData(std::move(*ev));

	const opval_t *codePos = m_ScriptClass->FindLabel(label);

	if (!codePos)
	{	
		const xstr& labelName = ScriptContext::Get().GetDirector().GetString(label);
		ScriptError("ScriptVM::EventGoto: label '%s' does not exist in '%s'.", labelName.c_str(), Filename().c_str());
	}

	m_CodePos = codePos;
}

bool ScriptVM::EventThrow(Event *ev)
{
	const const_str label = ev->GetConstString(1);

	SetFastData(std::move(*ev));

	while (1)
	{
		StateScript* const stateScript = m_ScriptClass->GetCatchStateScript(m_PrevCodePos, m_PrevCodePos);

		if (!stateScript) {
			break;
		}

		m_CodePos = stateScript->FindLabel(label);

		if (m_CodePos)
		{
			++fastIndex;
			return true;
		}
	}

	return false;
}

void ScriptVM::ReadOpcodeValue(void* outValue, size_t size)
{
	std::memcpy(outValue, m_CodePos, size);
	m_CodePos += size;
}

void ScriptVM::ReadGetOpcodeValue(void* outValue, size_t size)
{
	std::memcpy(outValue, m_CodePos, size);
}
