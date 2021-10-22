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
#include <morfuse/Script/Archiver.h>

#include <morfuse/Common/short3.h>
#include "ScriptVMOperation.h"
#include <cstdarg>

using namespace mfuse;

/** The maximum number of thread execution depth. */
constexpr unsigned int MAX_STACK_DEPTH_DEFAULT = 20;

thread_local size_t ScriptExecutionStack::stackDepth = 0;
thread_local size_t ScriptExecutionStack::maxStackDepth = MAX_STACK_DEPTH_DEFAULT;

ScriptStack::ScriptStack()
	: localStack(nullptr)
	, stackBottom(nullptr)
	, pTop(nullptr)
{
}

ScriptStack::ScriptStack(size_t stackSize)
{
	if (!stackSize) {
		stackSize = 1;
	}

	// allocate at once
	uint8_t* data = (uint8_t*)allocateMemory((sizeof(ScriptVariable) + sizeof(ScriptVariable*)) * stackSize);
	localStack = new (data) ScriptVariable[stackSize];
	data += sizeof(ScriptVariable) * stackSize;

	listenerVarPtr = new (data) ScriptVariable * [stackSize]();

	pTop = localStack;
	stackBottom = localStack + stackSize;
}

ScriptStack::ScriptStack(ScriptStack&& other)
	: localStack(other.localStack)
	, stackBottom(other.stackBottom)
	, pTop(other.pTop)
	, listenerVarPtr(other.listenerVarPtr)
{
	other.localStack = other.stackBottom = nullptr;
	other.pTop = nullptr;
	other.listenerVarPtr = nullptr;
}

ScriptStack& ScriptStack::operator=(ScriptStack&& other)
{
	localStack = other.localStack;
	stackBottom = other.stackBottom;
	pTop = other.pTop;
	listenerVarPtr = other.listenerVarPtr;
	other.localStack = other.stackBottom = nullptr;
	other.pTop = nullptr;
	other.listenerVarPtr = nullptr;

	return *this;
}

ScriptStack::~ScriptStack()
{
	const size_t localStackSize = GetStackSize();
	for (uintptr_t i = 0; i < localStackSize; ++i) {
		localStack[i].~ScriptVariable();
	}

	uint8_t* const data = (uint8_t*)localStack;
	if (data) {
		freeMemory(data);
	}
}

size_t ScriptStack::GetStackSize() const
{
	return stackBottom - localStack;
}

ScriptVariable& ScriptStack::SetTop(ScriptVariable& newTop)
{
	return *(pTop = &newTop);
}

ScriptVariable& ScriptStack::GetTop() const
{
	return *pTop;
}

ScriptVariable& ScriptStack::GetTop(size_t offset) const
{
	return *(pTop + offset);
}

ScriptVariable* ScriptStack::GetTopPtr() const
{
	return pTop;
}

ScriptVariable* ScriptStack::GetTopPtr(size_t offset) const
{
	return pTop + offset;
}

ScriptVariable* ScriptStack::GetTopArray(size_t offset) const
{
	return pTop + offset;
}

uintptr_t ScriptStack::GetIndex() const
{
	return pTop - localStack;
}

ScriptVariable& ScriptStack::Pop()
{
	return *(pTop--);
}

ScriptVariable& ScriptStack::Pop(size_t offset)
{
	ScriptVariable& old = *pTop;
	pTop -= offset;
	return old;
}

ScriptVariable& ScriptStack::PopAndGet()
{
	return *--pTop;
}

ScriptVariable& ScriptStack::PopAndGet(size_t offset)
{
	pTop -= offset;
	return *pTop;
}

ScriptVariable& ScriptStack::Push()
{
	return *(pTop++);
}

ScriptVariable& ScriptStack::Push(size_t offset)
{
	ScriptVariable& old = *pTop;
	pTop += offset;
	return old;
}

ScriptVariable& ScriptStack::PushAndGet()
{
	return *++pTop;
}

ScriptVariable& ScriptStack::PushAndGet(size_t offset)
{
	pTop += offset;
	return *pTop;
}

void ScriptStack::MoveTop(ScriptVariable&& other)
{
	*pTop = std::move(other);
}

void ScriptStack::Archive(Archiver& arc)
{
	uint32_t stackSize = 0;

	if (arc.Loading())
	{
		arc.ArchiveUInt32(stackSize);
		*this = ScriptStack(stackSize);
	}
	else
	{
		stackSize = (uint32_t)GetStackSize();
		arc.ArchiveUInt32(stackSize);
	}

	for (uint32_t i = 0; i < stackSize; i++) {
		localStack[i].ArchiveInternal(arc);
	}
}

ScriptVariable* ScriptStack::GetListenerVar(uintptr_t index)
{
	return listenerVarPtr[index];
}

void ScriptStack::SetListenerVar(uintptr_t index, ScriptVariable* newVar)
{
	listenerVarPtr[index] = newVar;
}

ScriptVM::ScriptVM()
	: next(nullptr)
	, m_Thread(nullptr)
	, m_ScriptClass(nullptr)
	, m_CodePos(nullptr)
	, m_PrevCodePos(nullptr)
	, m_StackPos(nullptr)
	, fastIndex(0)
	, state(vmState_e::Idling)
	, m_bMarkStack(false)
{
}

ScriptVM::ScriptVM(ScriptClass *scriptClass, const opval_t *pCodePos, ScriptThread *thread)
	: m_Stack(scriptClass->GetScript()->GetRequiredStackSize())
{
	next = nullptr;

	m_Thread = thread;
	m_ScriptClass = scriptClass;

	m_PrevCodePos = NULL;
	m_CodePos = pCodePos;

	state = vmState_e::Running;

	m_bMarkStack = false;
	m_StackPos = NULL;

	fastIndex = 0;

	m_ScriptClass->AddThread(this);
}

ScriptVM::~ScriptVM()
{
	// clean-up the call stack
	while (callStack.NumObjects()) {
		LeaveFunction();
	}
}

void* ScriptVM::operator new(size_t)
{
	return ScriptContext::Get().GetAllocator().GetBlock<ScriptVM>().Alloc();
}

void ScriptVM::operator delete(void* ptr)
{
	ScriptContext::Get().GetAllocator().GetBlock<ScriptVM>().Free(ptr);
}

/*
====================
Archive
====================
*/
void ScriptVM::Archive(Archiver& arc)
{
	m_Stack.Archive(arc);
	m_ReturnValue.ArchiveInternal(arc);
	m_ScriptClass->ArchiveCodePos(arc, m_PrevCodePos);
	m_ScriptClass->ArchiveCodePos(arc, m_CodePos);
	arc.ArchiveEnum(state);
}

const opval_t *ScriptVM::ProgBuffer()
{
	return m_CodePos;
}

void ScriptVM::EnterFunction(ScriptVariableContainer&&)
{
	// TODO: implement
#if 0
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
#endif
}

void ScriptVM::LeaveFunction()
{
	// TODO: implement
#if 0
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
#else
	delete m_Thread;
#endif
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
			fill("VM stack error. Negative stack value " + str(stack));
		}
		else {
			fill("VM stack error. Exceeded the maximum stack size " + str(stack));
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
		fill("command '" + str(eventSystem.GetEventName(eventNum)) + "' applied to NIL");
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
		fill("command '" + str(eventSystem.GetEventName(eventNum)) + "' applied to NULL listener");
	}

	return Messageable::what();
}
