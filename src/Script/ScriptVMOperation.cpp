#include <morfuse/Script/ScriptVM.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/SimpleEntity.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Common/short3.h>
#include "ScriptVMOperation.h"

#include <utility>

using namespace mfuse;

class ScriptEvent : public Event
{
public:
	ScriptEvent(eventNum_t eventNum);
	ScriptEvent(eventNum_t eventNum, size_t numArgs);
};

ScriptEvent::ScriptEvent(eventNum_t eventNum)
	: Event(eventNum)
{
	fromScript = true;
}

ScriptEvent::ScriptEvent(eventNum_t eventNum, size_t numArgs)
	: Event(eventNum, numArgs)
{
	fromScript = true;
}

template<>
void ScriptVM::executeCommand<false, false>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount) : ScriptEvent(eventnum);
	return executeCommandInternal(ev, listener, pTop + 1, iParamCount);
}

template<>
void ScriptVM::executeCommand<true, false>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount) : ScriptEvent(eventnum);
	return executeCommandInternal(ev, listener, pTop + 1, iParamCount);
}

template<>
void ScriptVM::executeCommand<false, true>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount + 1) : ScriptEvent(eventnum, 1);
	return executeCommandInternal(ev, listener, pTop, iParamCount);
}

template<>
void ScriptVM::executeCommand<true, true>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount + 1) : ScriptEvent(eventnum, 1);
	return executeCommandInternal(ev, listener, pTop, iParamCount);
}

void ScriptVM::executeCommandInternal(Event& ev, Listener* listener, ScriptVariable* fromVar, op_parmNum_t iParamCount)
{
	for (uint16_t i = 0; i < iParamCount; i++)
	{
		ev.AddValue(fromVar[i]);
	}

	listener->ProcessScriptEvent(ev);

	if (ev.NumArgs() > iParamCount) {
		*pTop = std::move(ev.GetValue(ev.NumArgs()));
	}
	else {
		pTop->Clear();
	}
}

bool ScriptVM::executeGetter(EventSystem& eventSystem, Listener* listener, op_evName_t eventName)
{
	const eventInfo_t& eventInfo = eventSystem.FindEventInfoChecked((eventName_t)eventName);

	if (eventInfo.getterNum && listener->classinfo()->GetDef(eventInfo.getterNum))
	{
		ScriptEvent ev(eventInfo.getterNum);

		listener->ProcessScriptEvent(ev);

		if (ev.NumArgs() > 0)
		{
			*pTop = std::move(ev.GetValue(ev.NumArgs()));
		}
		else
		{
			pTop->Clear();
		}

		return true;
	}
	else if (eventInfo.setterNum && listener->classinfo()->GetDef(eventInfo.setterNum))
	{
		throw ScriptException("Cannot get a write-only variable");
	}

	return false;
}

bool ScriptVM::executeSetter(EventSystem& eventSystem, Listener* listener, op_evName_t eventName)
{
	const eventInfo_t& eventInfo = eventSystem.FindEventInfoChecked((eventName_t)eventName);

	if (eventInfo.setterNum && listener->classinfo()->GetDef(eventInfo.setterNum))
	{
		ScriptEvent ev(eventInfo.setterNum, 1);

		ev.AddValue(*pTop);

		listener->ProcessScriptEvent(ev);

		return true;
	}
	else if (eventInfo.getterNum && listener->classinfo()->GetDef(eventInfo.getterNum))
	{
		throw ScriptException("Cannot set a read-only variable");
	}

	return false;
}

void ScriptVM::jump(op_offset_t offset)
{
	m_CodePos += offset;
}

void ScriptVM::jumpBack(op_offset_t offset)
{
	m_CodePos -= offset;
}

void ScriptVM::jumpBool(op_offset_t offset, bool booleanValue)
{
	if (booleanValue)
	{
		jump(offset);
	}
}

bool ScriptVM::jumpVar(op_offset_t offset, bool booleanValue)
{
	if (booleanValue)
	{
		jump(offset);
		return true;
	}
	else
	{
		pTop--;
		return false;
	}
}

void ScriptVM::doJumpIf(bool booleanValue)
{
	const op_offset_t offset = ReadOpcodeValue<op_offset_t>();
	jumpBool(offset, booleanValue);
}

bool ScriptVM::doJumpVarIf(bool booleanValue)
{
	const op_offset_t offset = ReadOpcodeValue<op_offset_t>();
	return jumpVar(offset, booleanValue);
}

static ScriptVariable vars[3];

template<bool noTop>
void ScriptVM::loadTop(EventSystem& eventSystem, Listener* listener)
{
	const const_str variable = ReadOpcodeValue<op_name_t>();
	const op_evName_t eventName = ReadOpcodeValue<op_evName_t>();

	if (!eventName || !executeSetter(eventSystem, listener, eventName))
	{
		// just set the variable
		if (pTop >= localStack && pTop < stackBottom)
		{
			const uintptr_t varIndex = pTop - localStack;
			ScriptVariable* const listenerVar = listenerVarPtr[varIndex];
			if (!listenerVar || listenerVar->GetKey() != variable) {
				listener->Vars()->SetVariable(variable, std::move(*pTop));
			}
			else {
				*listenerVar = std::move(*pTop);
			}
		}
		else
		{
			listener->Vars()->SetVariable(variable, std::move(*pTop));
		}
	}

	if constexpr (!noTop) pTop--;
}

template<bool noTop>
ScriptVariable* ScriptVM::storeTop(EventSystem& eventSystem, Listener* listener)
{
	const const_str variable = ReadOpcodeValue<op_name_t>();
	const op_evName_t eventName = ReadOpcodeValue<op_evName_t>();
	ScriptVariable* listenerVar;

	if constexpr (!noTop) pTop++;

	if (!eventName || !executeGetter(eventSystem, listener, eventName))
	{
		const uintptr_t varIndex = pTop - localStack;
		listenerVar = listenerVarPtr[varIndex];
		if (!listenerVar || listenerVar->GetKey() != variable)
		{
			listenerVar = listener->Vars()->GetOrCreateVariable(variable);
			listenerVarPtr[varIndex] = listenerVar;
		}

		*pTop = *listenerVar;
	} else {
		listenerVar = nullptr;
	}

	return listenerVar;
}

void ScriptVM::storeField(op_name_t fieldName, Listener* listener)
{
	const uintptr_t varIndex = pTop - localStack;
	ScriptVariable* listenerVar = listenerVarPtr[varIndex];
	if (!listenerVar || listenerVar->GetKey() != fieldName)
	{
		listenerVar = listener->Vars()->GetOrCreateVariable(fieldName);
		listenerVarPtr[varIndex] = listenerVar;
	}

	*pTop = *listenerVar;
}

void ScriptVM::skipField()
{
	m_CodePos += sizeof(op_name_t) + sizeof(op_evName_t);
}

void ScriptVM::ExecCmdCommon(op_parmNum_t param)
{
	const op_ev_t eventNum = ReadOpcodeValue<op_ev_t>();

	pTop -= param;

	executeCommand(GetScriptThread(), param, eventNum);
}

void ScriptVM::ExecCmdMethodCommon(op_parmNum_t param)
{
	const ScriptVariable* const a = pTop--;
	const op_ev_t eventNum = ReadOpcodeValue<op_ev_t>();

	pTop -= param;

	const size_t arraysize = a->arraysize();
	if (arraysize == (size_t)-1)
	{
		throw ScriptVMErrors::NilListenerCommand(eventNum);
	}

	if (arraysize > 1)
	{
		if (a->IsConstArray())
		{
			for (uintptr_t i = arraysize; i > 0; i--)
			{
				Listener* const listener = a->listenerAt(i);
				// if the listener is NULL, don't throw an exception
				// it would be unfair if the other listeners executed the command
				if (listener) {
					executeCommand<true>(listener, param, eventNum);
				}
			}
		}
		else
		{
			ScriptVariable array = *a;
			// must cast into a const array value
			array.CastConstArrayValue();

			for (uintptr_t i = array.arraysize(); i > 0; i--)
			{
				Listener* const listener = array[i].listenerAt(i);
				if (listener) {
					executeCommand<true>(listener, param, eventNum);
				}
			}
		}
	}
	else
	{
		// avoid useless allocations of const array
		Listener* const listener = a->listenerValue();
		if (!listener)
		{
			throw ScriptVMErrors::NullListenerCommand(eventNum);
		}

		executeCommand<true>(listener, param, eventNum);
	}
}

void ScriptVM::ExecMethodCommon(op_parmNum_t param)
{
	const ScriptVariable* const a = pTop--;
	const op_ev_t eventNum = ReadOpcodeValue<op_ev_t>();

	pTop -= param;
	// push the return value
	pTop++;

	Listener* const listener = a->listenerValue();
	if (!listener)
	{
		throw ScriptVMErrors::NullListenerCommand(eventNum);
	}

	executeCommand<true, true>(listener, param, eventNum);
}

void ScriptVM::ExecFunction(ScriptMaster& Director)
{
	if (!ReadOpcodeValue<bool>())
	{
		const op_name_t label = ReadOpcodeValue<op_name_t>();
		const op_parmNum_t params = ReadOpcodeValue<op_parmNum_t>();

		Listener* listener;

		try
		{
			listener = pTop->listenerValue();

			if (!listener)
			{
				const str& labelName = Director.GetDictionary().Get(label);
				throw ScriptException("function '" +  labelName + "' applied to NULL listener");
			}
		}
		catch (...)
		{
			pTop -= params;
			throw;
		}

		pTop--;

		ScriptVariableContainer data;
		data.Resize(params + 1);

		ScriptVariable* labelVar = new (data) ScriptVariable;
		labelVar->setConstStringValue(label);

		const ScriptVariable* var = pTop;
		pTop -= params;

		for (int i = 0; i < params; var++, i++)
		{
			data.AddObject(*var);
		}

		pTop++;
		EnterFunction(std::move(data));

		GetScriptClass()->SetSelf(listener);
	}
	else
	{
		const op_name_t filename = ReadOpcodeValue<op_name_t>();
		const op_name_t label = ReadOpcodeValue<op_name_t>();
		const op_parmNum_t params = ReadOpcodeValue<op_parmNum_t>();

		Listener* listener;
		try
		{
			listener = pTop->listenerValue();

			if (!listener)
			{
				const str& labelStr = Director.GetDictionary().Get(label);
				const str& fileStr = Director.GetDictionary().Get(filename);
				throw ScriptException("function '" + labelStr + "' in '" + fileStr + "' applied to NULL listener");
			}
		}
		catch (...)
		{
			pTop -= params;
			throw;
		}

		pTop--;

		ScriptVariable constarray;
		ScriptVariable* pVar = new ScriptVariable[2];

		pVar[0].setConstStringValue(filename);
		pVar[1].setConstStringValue(label);

		constarray.setConstArrayValue(pVar, 2);

		delete[] pVar;

		Event ev(EV_Listener_WaitCreateReturnThread, params);

		const ScriptVariable* var = pTop;
		pTop -= params;

		for (int i = 0; i < params; var++, i++) {
			ev.AddValue(*var);
		}

		pTop++;
		*pTop = listener->ProcessEventReturn(ev);
	}
}

void ScriptVM::Execute(const VarListView& data, const StringResolvable& label)
{
	if (!label.IsEmpty())
	{
		// Throw if label is not found
		const script_label_t* const s = m_ScriptClass->FindLabel(label.GetConstString());
		if (!s)
		{
			throw StateScriptErrors::LabelNotFound(label, Filename());
		}

		m_CodePos = s->codepos;
	}

	ScriptContext& context = ScriptContext::Get();
	ScriptMaster& Director = context.GetDirector();
	const OutputInfo& info = context.GetOutputInfo();
	std::ostream* dbg = info.GetOutput(outputLevel_e::Debug);

	// in case the stack execution fails with an exception
	// make the VM ready to be destroyed
	state = vmState_e::Idling;
	ScriptExecutionStack executionStack;

	state = vmState_e::Running;

	if (data.NumObjects())
	{
		// parameters given to the interpreter
		SetFastData(data);
	}

	const TimeManager& timeManager = context.GetTimeManager();
	// set the next time at which the VM will timeout if it spends too many time executing
	const uinttime_t maxExecTime = Director.GetThreadExecutionProtection().GetMaxExecutionTime();
	uinttime_t nextTime = 
		maxExecTime
		? (timeManager.GetTime() + maxExecTime)
		: 0;

	//VMOperation operation(*this, ScriptContext::Get(), Director, EventSystem::Get());
	
	bool doneProcessing = false;
	do
	{
		try
		{
			doneProcessing = Process(context, nextTime);
		}
		catch (ScriptVMErrors::CommandOverflow&)
		{
			if (Director.GetThreadExecutionProtection().ShouldDrop())
			{
				HandleScriptExceptionAbort(info);
				throw;
			}

			*dbg << "Update of script position - This is not an error." << std::endl;
			*dbg << "=================================================" << std::endl;
			const ProgramScript* const scr = m_ScriptClass->GetScript();
			scr->PrintSourcePos(*dbg, m_CodePos - scr->GetProgBuffer());
			*dbg << "=================================================" << std::endl;

			nextTime = timeManager.GetTime() + Director.GetThreadExecutionProtection().GetMaxExecutionTime();
		}
		catch (ScriptExceptionBase& exc)
		{
			HandleScriptException(exc, info);
		}
		catch (std::exception&)
		{
			HandleScriptExceptionAbort(info);
			throw;
		}
	}
	while (!doneProcessing);

	switch (state)
	{
	case vmState_e::Suspended:
		state = vmState_e::Idling;
		// FIXME: use m_Stack to save top?
		break;
	case vmState_e::Destroy:
		delete m_Thread;
		m_Thread = nullptr;
		// also destroy the VM
	case vmState_e::Destroyed:
		assert(pTop == localStack);
		delete this;
		break;
	default:
		break;
	}
}

bool ScriptVM::Process(ScriptContext& context, uinttime_t interruptTime)
{
	ScriptMaster& Director = context.GetDirector();
	EventSystem& eventSystem = EventSystem::Get();
	const TargetList& targetList = context.GetTargetList();
	const TimeManager& timeManager = context.GetTimeManager();

	std::ostream* dbg = context.GetOutputInfo().GetOutput(outputLevel_e::Debug);

	uinttime_t cmdTime = timeManager.GetTime();

	while (state == vmState_e::Running)
	{
		if (!m_bMarkStack)
		{
			const bool stackOverrun = pTop < localStack || pTop >= stackBottom;
			assert(!stackOverrun);
			if (stackOverrun)
			{
				state = vmState_e::Idling;
				throw ScriptVMErrors::StackError(pTop - localStack);
			}
		}

		m_PrevCodePos = m_CodePos;
		const opval_t opcode = ReadOpcodeValue<opval_t>();
		switch (opcode)
		{
		case OP_DONE:
		{
			End();
			break;
		}

		case OP_BIN_BITWISE_AND:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b &= *a;
			break;
		}

		case OP_BIN_BITWISE_OR:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b |= *a;
			break;
		}

		case OP_BIN_BITWISE_EXCL_OR:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b ^= *a;
			break;
		}

		case OP_BIN_EQUALITY:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			b->setIntValue(*b == *a);
			break;
		}

		case OP_BIN_INEQUALITY:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			b->setIntValue(*b != *a);
			break;
		}

		case OP_BIN_GREATER_THAN:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			b->greaterthan(*a);
			break;
		}

		case OP_BIN_GREATER_THAN_OR_EQUAL:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			b->greaterthanorequal(*a);
			break;
		}

		case OP_BIN_LESS_THAN:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			b->lessthan(*a);
			break;
		}

		case OP_BIN_LESS_THAN_OR_EQUAL:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			b->lessthanorequal(*a);
			break;
		}

		case OP_BIN_PLUS:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b += *a;
			break;
		}

		case OP_BIN_MINUS:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b -= *a;
			break;
		}

		case OP_BIN_MULTIPLY:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b *= *a;
			break;
		}

		case OP_BIN_DIVIDE:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b /= *a;
			break;
		}

		case OP_BIN_PERCENTAGE:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b %= *a;
			break;
		}

		case OP_BIN_SHIFT_LEFT:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b <<= *a;
			break;
		}

		case OP_BIN_SHIFT_RIGHT:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop;

			*b >>= *a;
			break;
		}

		case OP_BOOL_JUMP_FALSE4:
		{
			doJumpIf(!pTop->GetData().long64Value);
			pTop--;
			break;
		}

		case OP_BOOL_JUMP_TRUE4:
		{
			doJumpIf(pTop->GetData().long64Value ? true : false);
			pTop--;
			break;
		}

		case OP_VAR_JUMP_FALSE4:
		{
			doJumpIf(!pTop->booleanValue());
			pTop--;
			break;
		}

		case OP_VAR_JUMP_TRUE4:
		{
			doJumpIf(pTop->booleanValue());
			pTop--;
			break;
		}

		case OP_BOOL_LOGICAL_AND:
		{
			doJumpVarIf(!pTop->GetData().long64Value);
			break;
		}

		case OP_BOOL_LOGICAL_OR:
		{
			doJumpVarIf(pTop->GetData().long64Value);
			break;
		}

		case OP_VAR_LOGICAL_AND:
		{
			if (!doJumpVarIf(pTop->booleanValue()))
			{
				pTop->SetFalse();
			}
			break;
		}

		case OP_VAR_LOGICAL_OR:
		{
			if (!doJumpVarIf(!pTop->booleanValue()))
			{
				pTop->SetTrue();
			}
			break;
		}

		case OP_BOOL_STORE_FALSE:
		{
			pTop++;
			pTop->SetFalse();
			break;
		}

		case OP_BOOL_STORE_TRUE:
		{
			pTop++;
			pTop->SetTrue();
			break;
		}

		case OP_BOOL_UN_NOT:
		{
			pTop->GetData().long64Value = (pTop->GetData().long64Value == 0);
			break;
		}

		case OP_CALC_VECTOR:
		{
			const ScriptVariable* const c = pTop--;
			const ScriptVariable* const b = pTop--;
			const ScriptVariable* const a = pTop;

			pTop->setVectorValue(Vector(a->floatValue(), b->floatValue(), c->floatValue()));
			break;
		}

		case OP_EXEC_CMD0:
		{
			ExecCmdCommon(0);
			break;
		}

		case OP_EXEC_CMD1:
		{
			ExecCmdCommon(1);
			break;
		}

		case OP_EXEC_CMD2:
		{
			ExecCmdCommon(2);
			break;
		}

		case OP_EXEC_CMD3:
		{
			ExecCmdCommon(3);
			break;
		}

		case OP_EXEC_CMD4:
		{
			ExecCmdCommon(4);
			break;
		}

		case OP_EXEC_CMD5:
		{
			ExecCmdCommon(5);
			break;
		}

		case OP_EXEC_CMD_COUNT1:
		{
			const op_parmNum_t numParms = ReadOpcodeValue<op_parmNum_t>();
			ExecCmdCommon(numParms);
			break;
		}

		case OP_EXEC_CMD_METHOD0:
		{
			ExecCmdMethodCommon(0);
			break;
		}

		case OP_EXEC_CMD_METHOD1:
		{
			ExecCmdMethodCommon(1);
			break;
		}

		case OP_EXEC_CMD_METHOD2:
		{
			ExecCmdMethodCommon(2);
			break;
		}

		case OP_EXEC_CMD_METHOD3:
		{
			ExecCmdMethodCommon(3);
			break;
		}

		case OP_EXEC_CMD_METHOD4:
		{
			ExecCmdMethodCommon(4);
			break;
		}

		case OP_EXEC_CMD_METHOD5:
		{
			ExecCmdMethodCommon(5);
			break;
		}

		case OP_EXEC_CMD_METHOD_COUNT1:
		{
			const op_parmNum_t numParms = ReadOpcodeValue<op_parmNum_t>();
			ExecCmdMethodCommon(numParms);
			break;
		}

		case OP_EXEC_METHOD0:
		{
			ExecMethodCommon(0);
			break;
		}

		case OP_EXEC_METHOD1:
		{
			ExecMethodCommon(1);
			break;
		}

		case OP_EXEC_METHOD2:
		{
			ExecMethodCommon(2);
			break;
		}

		case OP_EXEC_METHOD3:
		{
			ExecMethodCommon(3);
			break;
		}

		case OP_EXEC_METHOD4:
		{
			ExecMethodCommon(4);
			break;
		}

		case OP_EXEC_METHOD5:
		{
			ExecMethodCommon(5);
			break;
		}

		case OP_EXEC_METHOD_COUNT1:
		{
			const op_parmNum_t numParms = ReadOpcodeValue<op_parmNum_t>();
			ExecMethodCommon(numParms);
			break;
		}

		case OP_FUNC:
		{
			ExecFunction(Director);
			break;
		}

		case OP_JUMP4:
		{
			jump(ReadOpcodeValue<op_offset_t>());
			break;
		}

		case OP_JUMP_BACK4:
		{
			// negative offset to jump back
			jumpBack(ReadGetOpcodeValue<op_offset_t>());
			break;
		}

		case OP_LOAD_ARRAY_VAR:
		{
			const ScriptVariable* const a = pTop--;
			ScriptVariable* const b = pTop--;
			const ScriptVariable* const c = pTop--;

			b->setArrayAt(*a, *c);
			break;
		}

		case OP_LOAD_FIELD_VAR:
		{
			ScriptVariable* const a = pTop--;
			bool eventCalled = false;

			try
			{
				Listener* listener = a->listenerValue();

				if (listener == nullptr)
				{
					throw ScriptVMErrors::NullListenerField(ReadGetOpcodeValue<op_name_t>());
				}
				else
				{
					eventCalled = true;
					loadTop(eventSystem, listener);
				}
			}
			catch (...)
			{
				pTop--;

				if (!eventCalled) {
					skipField();
				}

				throw;
			}
			break;
		}

		case OP_LOAD_CONST_ARRAY1:
		{
			op_arrayParmNum_t numParms = ReadOpcodeValue<op_arrayParmNum_t>();

			pTop -= numParms - 1;
			pTop->setConstArrayValue(pTop, numParms);
			break;
		}

		case OP_LOAD_GAME_VAR:
		{
			loadTop(eventSystem, context.GetGame());
			break;
		}

		case OP_LOAD_GROUP_VAR:
		{
			loadTop(eventSystem, GetScriptClass());
			break;
		}

		case OP_LOAD_LEVEL_VAR:
		{
			loadTop(eventSystem, context.GetLevel());
			break;
		}

		case OP_LOAD_LOCAL_VAR:
		{
			loadTop(eventSystem, GetScriptThread());
			break;
		}

		case OP_LOAD_OWNER_VAR:
		{
			if (!GetScriptClass()->GetSelf())
			{
				pTop--;
				skipField();
				throw ScriptException("self is NULL");
			}

			if (!GetScriptClass()->GetSelf()->GetScriptOwner())
			{
				pTop--;
				skipField();
				throw ScriptException("self.owner is NULL");
			}

			loadTop(eventSystem, GetScriptClass()->GetSelf()->GetScriptOwner());
			break;
		}

		case OP_LOAD_PARM_VAR:
		{
			loadTop(eventSystem, &Director.GetParm());
			break;
		}

		case OP_LOAD_SELF_VAR:
		{
			if (!GetScriptClass()->GetSelf())
			{
				pTop--;
				skipField();
				throw ScriptException("self is NULL");
			}

			loadTop(eventSystem, GetScriptClass()->GetSelf());
			break;
		}

		case OP_LOAD_STORE_GAME_VAR:
		{
			loadTop<true>(eventSystem, context.GetGame());
			break;
		}

		case OP_LOAD_STORE_GROUP_VAR:
		{
			loadTop<true>(eventSystem, GetScriptClass());
			break;
		}

		case OP_LOAD_STORE_LEVEL_VAR:
		{
			loadTop<true>(eventSystem, context.GetLevel());
			break;
		}

		case OP_LOAD_STORE_LOCAL_VAR:
		{
			loadTop<true>(eventSystem, GetScriptThread());
			break;
		}

		case OP_LOAD_STORE_OWNER_VAR:
		{
			if (!GetScriptClass()->GetSelf())
			{
				skipField();
				throw ScriptException("self is NULL");
			}

			if (!GetScriptClass()->GetSelf()->GetScriptOwner())
			{
				skipField();
				throw ScriptException("self.owner is NULL");
			}

			loadTop<true>(eventSystem, GetScriptClass()->GetSelf()->GetScriptOwner());
			break;
		}

		case OP_LOAD_STORE_PARM_VAR:
		{
			loadTop<true>(eventSystem, &Director.GetParm());
			break;
		}

		case OP_LOAD_STORE_SELF_VAR:
		{
			if (!GetScriptClass()->GetSelf())
			{
				throw ScriptException("self is NULL");
			}

			loadTop<true>(eventSystem, GetScriptClass()->GetSelf());
			break;
		}

		case OP_MARK_STACK_POS:
		{
			m_StackPos = pTop;
			m_bMarkStack = true;
			break;
		}

		case OP_STORE_PARAM:
		{
			if (fastIndex < fastEvent.NumArgs())
			{
				pTop = &fastEvent.GetValueChecked(++fastIndex);
			}
			else
			{
				pTop = m_StackPos + 1;
				pTop->Clear();
			}
			break;
		}

		case OP_RESTORE_STACK_POS:
		{
			pTop = m_StackPos;
			m_bMarkStack = false;
			break;
		}

		case OP_STORE_ARRAY:
		{
			try
			{
				const ScriptVariable* const aVar = pTop--;
				pTop->evalArrayAt(*aVar);
			}
			catch (...)
			{
				pTop->Clear();
				throw;
			}
			break;
		}

		case OP_STORE_ARRAY_REF:
		{
			ScriptVariable* const aVar = pTop--;
			pTop->setArrayRefValue(*aVar);
			break;
		}

		case OP_STORE_FIELD_REF:
		{
			try
			{
				Listener* listener = pTop->listenerValue();

				if (listener == nullptr)
				{
					const op_name_t fieldName = ReadGetOpcodeValue<op_name_t>();
					skipField();
					throw ScriptVMErrors::NullListenerField(fieldName);
				}
				else
				{
					ScriptVariable* const listenerVar = storeTop<true>(eventSystem, listener);

					if (listenerVar)
					{
						// having a listener variable means the variable was just created
						pTop->setRefValue(listenerVar);
					}
				}
			}
			catch (...)
			{
				pTop->setRefValue(pTop);
				throw;
			}
			break;
		}

		case OP_STORE_FIELD:
			try
			{
				Listener* listener = pTop->listenerValue();

				if (listener == nullptr)
				{
					throw ScriptVMErrors::NullListenerField(ReadGetOpcodeValue<op_name_t>());
				}
				else
				{
					storeTop<true>(eventSystem, listener);
				}
				break;
			}
			catch (...)
			{
				skipField();
				throw;
			}

		case OP_STORE_FLOAT:
		{
			pTop++;
			pTop->setFloatValue(ReadOpcodeValue<float>());
			break;
		}

		case OP_STORE_INT0:
		{
			pTop++;
			pTop->setIntValue(0);
			break;
		}

		case OP_STORE_INT1:
		{
			pTop++;
			pTop->setIntValue(ReadOpcodeValue<uint8_t>());
			break;
		}

		case OP_STORE_INT2:
		{
			pTop++;
			pTop->setIntValue(ReadOpcodeValue<uint16_t>());
			break;
		}

		case OP_STORE_INT3:
		{
			pTop++;
			pTop->setIntValue(ReadOpcodeValue<short3>());
			break;
		}

		case OP_STORE_INT4:
		{
			pTop++;
			pTop->setIntValue(ReadOpcodeValue<uint32_t>());
			break;
		}

		case OP_STORE_INT8:
		{
			pTop++;
			pTop->setLongValue(ReadOpcodeValue<uint64_t>());
			break;
		}

		case OP_STORE_GAME_VAR:
		{
			storeTop(eventSystem, context.GetGame());
			break;
		}

		case OP_STORE_GROUP_VAR:
		{
			storeTop(eventSystem, GetScriptClass());
			break;
		}

		case OP_STORE_LEVEL_VAR:
		{
			storeTop(eventSystem, context.GetLevel());
			break;
		}

		case OP_STORE_LOCAL_VAR:
		{
			storeTop(eventSystem, GetScriptThread());
			break;
		}

		case OP_STORE_OWNER_VAR:
		{
			if (!GetScriptClass()->GetSelf())
			{
				pTop++;
				skipField();
				throw ScriptException("self is NULL");
			}

			if (!GetScriptClass()->GetSelf()->GetScriptOwner())
			{
				pTop++;
				skipField();
				throw ScriptException("self.owner is NULL");
			}

			storeTop(eventSystem, GetScriptClass()->GetSelf()->GetScriptOwner());
			break;
		}

		case OP_STORE_PARM_VAR:
		{
			storeTop(eventSystem, &Director.GetParm());
			break;
		}

		case OP_STORE_SELF_VAR:
		{
			if (!GetScriptClass()->GetSelf())
			{
				pTop++;
				skipField();
				throw ScriptException("self is NULL");
			}

			storeTop(eventSystem, GetScriptClass()->GetSelf());
			break;
		}

		case OP_STORE_GAME:
		{
			pTop++;
			pTop->setListenerValue(context.GetGame());
			break;
		}

		case OP_STORE_GROUP:
		{
			pTop++;
			pTop->setListenerValue(GetScriptClass());
			break;
		}

		case OP_STORE_LEVEL:
		{
			pTop++;
			pTop->setListenerValue(context.GetLevel());
			break;
		}

		case OP_STORE_LOCAL:
		{
			pTop++;
			pTop->setListenerValue(GetScriptThread());
			break;
		}

		case OP_STORE_OWNER:
		{
			pTop++;

			if (!GetScriptClass()->GetSelf())
			{
				pTop++;
				throw ScriptException("self is NULL");
			}

			pTop->setListenerValue(GetScriptClass()->GetSelf()->GetScriptOwner());
			break;
		}

		case OP_STORE_PARM:
		{
			pTop++;
			pTop->setListenerValue(&Director.GetParm());
			break;
		}

		case OP_STORE_SELF:
		{
			pTop++;
			pTop->setListenerValue(GetScriptClass()->GetSelf());
			break;
		}

		case OP_STORE_NIL:
		{
			pTop++;
			pTop->Clear();
			break;
		}

		case OP_STORE_NULL:
		{
			pTop++;
			pTop->setListenerValue(nullptr);
			break;
		}

		case OP_STORE_STRING:
		{
			pTop++;
			pTop->setConstStringValue(ReadOpcodeValue<op_name_t>());
			break;
		}

		case OP_STORE_VECTOR:
		{
			pTop++;
			pTop->setVectorValue(ReadOpcodeValue<Vector>());
			break;
		}

		case OP_SWITCH:
		{
			if (!Switch(ReadGetOpcodeValue<StateScript*>(), *pTop))
			{
				m_CodePos += sizeof(StateScript*);
			}

			pTop--;
			break;
		}

		case OP_UN_CAST_BOOLEAN:
		{
			pTop->CastBoolean();
			break;
		}

		case OP_UN_COMPLEMENT:
		{
			pTop->complement();
			break;
		}

		case OP_UN_MINUS:
		{
			pTop->minus();
			break;
		}

		case OP_UN_DEC:
		{
			(*pTop)--;
			break;
		}

		case OP_UN_INC:
		{
			(*pTop)++;
			break;
		}

		case OP_UN_SIZE:
		{
			pTop->setLongValue((uintptr_t)pTop->size());
			break;
		}

		case OP_UN_TARGETNAME:
		{
			const ConTarget* const foundTargetList = targetList.GetExistingConstTargetList(pTop->constStringValue());

			if (!foundTargetList || !foundTargetList->NumObjects())
			{
				const_str targetName = pTop->constStringValue();
				pTop->setListenerValue(nullptr);

				throw TargetListErrors::NoTargetException(targetName);
			}
			else if (foundTargetList->NumObjects() == 1)
			{
				Listener* const targetListener = foundTargetList->ObjectAt(1);
				pTop->setListenerValue(targetListener);
			}
			else if (foundTargetList->NumObjects() > 1)
			{
				pTop->setContainerValue(foundTargetList);
			}
			break;
		}

		case OP_VAR_UN_NOT:
		{
			pTop->setIntValue(pTop->booleanValue());
			break;
		}

		case OP_NOP:
		{
			break;
		}
		default:
			*dbg << "unknown opcode " << (uint32_t)*m_PrevCodePos << std::endl;
			break;
		}

		if (interruptTime && cmdTime >= interruptTime) {
			throw ScriptVMErrors::CommandOverflow();
		}

		cmdTime = timeManager.GetTime();
	}

	return state != vmState_e::Running;
}
