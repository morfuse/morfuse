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
#include <morfuse/Script/PredefinedString.h>
#include <morfuse/Common/short3.h>
#include "ScriptVMOperation.h"
#include <cstdarg>

using namespace mfuse;

/** The maximum number of thread execution depth. */
constexpr unsigned int MAX_STACK_DEPTH_DEFAULT = 20;

thread_local size_t ScriptExecutionStack::stackDepth = 0;
thread_local size_t ScriptExecutionStack::maxStackDepth = MAX_STACK_DEPTH_DEFAULT;

ScriptVM::ScriptVM(ScriptClass *scriptClass, const opval_t *pCodePos, ScriptThread *thread)
{
	next = NULL;

	m_Thread = thread;
	m_ScriptClass = scriptClass;

	m_Stack = NULL;

	m_PrevCodePos = NULL;
	m_CodePos = pCodePos;

	state = vmState_e::Running;

	//m_pOldData = NULL;
	//m_OldDataSize = 0;

	m_bMarkStack = false;
	m_StackPos = NULL;

	size_t localStackSize = m_ScriptClass->GetScript()->GetRequiredStackSize();
	if (!localStackSize) {
		localStackSize = 1;
	}

	// allocate at once
	uint8_t* data = (uint8_t*)allocateMemory((sizeof(ScriptVariable) + sizeof(ScriptVariable*)) * localStackSize);
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
	
	const size_t localStackSize = GetLocalStackSize();
	for(uintptr_t i = 0; i < localStackSize; ++i) {
		localStack[i].~ScriptVariable();
	}
	
	uint8_t* data = (uint8_t*)localStack;
	freeMemory(data);
	// listenerVarPtr will also be deleted as it was allocated in the same chunk
	//delete fastEvent;
}

void* ScriptVM::operator new(size_t)
{
	return ScriptContext::Get().GetAllocator().GetBlock<ScriptVM>().Alloc();
}

void ScriptVM::operator delete(void* ptr)
{
	ScriptContext::Get().GetAllocator().GetBlock<ScriptVM>().Free(ptr);
}

size_t ScriptVM::GetLocalStackSize() const
{
	return stackBottom - localStack;
}

/*
====================
Archive
====================
*/
void ScriptVM::Archive(Archiver&)
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

void ScriptVM::EnterFunction(ScriptVariableContainer&& data)
{
	ScriptCallStack *stack;
	const const_str label = data[0].constStringValue();

	SetFastData(std::move(data));

	const script_label_t *s = m_ScriptClass->FindLabel(label);

	if (!s)
	{
		throw StateScriptErrors::LabelNotFound(label, Filename());
	}

	stack = new ScriptCallStack;

	stack->codePos = m_CodePos;

	stack->pTop = pTop;
	stack->returnValue = m_ReturnValue;
	stack->localStack = localStack;
	stack->m_Self = m_ScriptClass->GetSelf();

	callStack.AddObject(stack);

	m_CodePos = s->codepos;

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

void ScriptVM::HandleScriptException(const std::exception& exc, const OutputInfo& info)
{
	std::ostream* out = info.GetOutput(outputLevel_e::Warn);

	if (m_ScriptClass)
	{
		const ProgramScript* const scr = m_ScriptClass->GetScript();
		scr->PrintSourcePos(*out, m_PrevCodePos - scr->GetProgBuffer());
	}

	if (out)
	{
		// write debug message
		*out << "^~^~^ Script Warning : " << exc.what() << std::endl << std::endl;
	}
}

void ScriptVM::HandleScriptExceptionAbort(const OutputInfo& info)
{
	std::ostream* out = info.GetOutput(outputLevel_e::Error);
	std::ostream* verb = info.GetOutput(outputLevel_e::Verbose);
	if (verb) {
		*verb << "----FRAME: " << ScriptExecutionStack::GetStackDepth() << "(" << this << ") -------------------------------------------------------------------" << std::endl;
	}

	if (m_ScriptClass && out)
	{
		const ProgramScript* const scr = m_ScriptClass->GetScript();
		scr->PrintSourcePos(*out, m_PrevCodePos - scr->GetProgBuffer());
	}

	state = vmState_e::Idling;
}

void ScriptVM::SetFastData(const VarListView& data)
{
	fastEvent.Clear();
	fastIndex = 0;

	if (data.NumObjects())
	{
		fastEvent.data.SetNumObjects(data.NumObjects());
		fastIndex = 0;

		for (uintptr_t i = 0; i < data.NumObjects(); i++)
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
		throw ScriptException("Attempting to delete a dead thread.");
		break;

	case vmState_e::Running:
	case vmState_e::Suspended:
	case vmState_e::Destroy:
		state = vmState_e::Destroyed;

		if (m_ScriptClass) {
			m_ScriptClass->RemoveThread(this);
		}

		break;

	case vmState_e::Idling:
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
		throw ScriptException("Cannot suspend a dead thread.");
	}
	else if (state == vmState_e::Running) {
		state = vmState_e::Suspended;
	}
}

bool ScriptVM::Switch(const StateScript *stateScript, ScriptVariable& var)
{
	//fastEvent.dataSize = 0;

	const script_label_t* s = stateScript->FindLabel(var.constStringValue());

	if (!s)
	{
		s = stateScript->FindLabel(STRING_EMPTY);

		if (!s) {
			return false;
		}
	}

	m_CodePos = s->codepos;

	return true;
}

const_str ScriptVM::Filename()
{
	return m_ScriptClass->Filename();
}

const_str ScriptVM::Label()
{
	// FIXME: Find VM label
	//const_str label = m_ScriptClass->NearestLabel(m_CodePos);
	return ConstStrings::Empty;
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

void ScriptVM::EventGoto(Event *ev)
{
	const const_str label = ev->GetConstString(1);

	SetFastData(std::move(*ev));

	const script_label_t *s = m_ScriptClass->FindLabel(label);

	if (!s)
	{	
		throw StateScriptErrors::LabelNotFound(label, Filename());
	}

	m_CodePos = s->codepos;
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

		const script_label_t* s = stateScript->FindLabel(label);
		s = stateScript->FindLabel(label);

		if (s)
		{
			m_CodePos = s->codepos;
			++fastIndex;
			return true;
		}

		m_CodePos = nullptr;
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

ScriptVMErrors::StackError::StackError(intptr_t stackVal)
	: stack(stackVal)
{
}

intptr_t ScriptVMErrors::StackError::GetStack() const noexcept
{
	return stack;
}

const char* ScriptVMErrors::StackError::what() const noexcept
{
	if (!filled())
	{
		if (stack < 0) {
			fill("VM stack error. Negative stack value " + xstr(stack));
		}
		else {
			fill("VM stack error. Exceeded the maximum stack size " + xstr(stack));
		}
	}

	return Messageable::what();
}

const char* ScriptVMErrors::CommandOverflow::what() const noexcept
{
	return "Command overflow. Possible infinite loop in thread.";
}

const char* ScriptVMErrors::MaxStackDepth::what() const noexcept
{
	return "stack overflow";
}

const char* ScriptVMErrors::NegativeStackDepth::what() const noexcept
{
	return "stack underflow";
}

ScriptExecutionStack::ScriptExecutionStack()
{
	if (stackDepth > maxStackDepth) {
		throw ScriptVMErrors::MaxStackDepth();
	}

	stackDepth++;
}

ScriptExecutionStack::~ScriptExecutionStack()
{
	stackDepth--;
}

void ScriptExecutionStack::SetMaxStackDepth(size_t maxDepth)
{
	maxStackDepth = maxDepth;
}

size_t ScriptExecutionStack::GetMaxStackDepth()
{
	return maxStackDepth;
}

size_t ScriptExecutionStack::GetStackDepth()
{
	return stackDepth;
}

ScriptVMErrors::NullListenerField::NullListenerField(const_str fieldNameValue)
	: fieldName(fieldNameValue)
{
}

const_str ScriptVMErrors::NullListenerField::GetFieldName() const
{
	return fieldName;
}

const char* ScriptVMErrors::NullListenerField::what() const noexcept
{
	if (!filled())
	{
		const ScriptContext& context = ScriptContext::Get();
		const StringDictionary& dict = context.GetDirector().GetDictionary();
		fill("Field '" + dict.Get(fieldName) + "' applied to NULL listener");
	}

	return Messageable::what();
}

ScriptVMErrors::NilListenerCommand::NilListenerCommand(eventNum_t eventNumValue)
	: eventNum(eventNumValue)
{
}

eventNum_t ScriptVMErrors::NilListenerCommand::GetEventNum() const
{
	return eventNum;
}

const char* ScriptVMErrors::NilListenerCommand::what() const noexcept
{
	if (!filled())
	{
		const EventSystem& eventSystem = EventSystem::Get();
		fill("command '" + xstr(eventSystem.GetEventName(eventNum)) + "' applied to NIL");
	}

	return Messageable::what();
}

ScriptVMErrors::NullListenerCommand::NullListenerCommand(eventNum_t eventNumValue)
	: eventNum(eventNumValue)
{

}

eventNum_t ScriptVMErrors::NullListenerCommand::GetEventNum() const
{
	return eventNum;
}

const char* ScriptVMErrors::NullListenerCommand::what() const noexcept
{
	if (!filled())
	{
		const EventSystem& eventSystem = EventSystem::Get();
		fill("command '" + xstr(eventSystem.GetEventName(eventNum)) + "' applied to NULL listener");
	}

	return Messageable::what();
}
