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
	return executeCommandInternal(ev, listener, pTop + 1, iParamCount, eventnum);
}

template<>
void ScriptVM::executeCommand<true, false>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount) : ScriptEvent(eventnum);
	return executeCommandInternal(ev, listener, pTop + 1, iParamCount, eventnum);
}

template<>
void ScriptVM::executeCommand<false, true>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount + 1) : ScriptEvent(eventnum, 1);
	return executeCommandInternal(ev, listener, pTop, iParamCount, eventnum);
}

template<>
void ScriptVM::executeCommand<true, true>(Listener* listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
	ScriptEvent ev = iParamCount ? ScriptEvent(eventnum, iParamCount + 1) : ScriptEvent(eventnum, 1);
	return executeCommandInternal(ev, listener, pTop, iParamCount, eventnum);
}

void ScriptVM::executeCommandInternal(Event& ev, Listener* listener, ScriptVariable* fromVar, op_parmNum_t iParamCount, op_evName_t eventnum)
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

		if (listener->ProcessScriptEvent(ev))
		{
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
		else
		{
			return false;
		}
	}
	else if (eventInfo.setterNum && listener->classinfo()->GetDef(eventInfo.setterNum))
	{
		ScriptError("Cannot get a write-only variable");
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

		if (listener->ProcessScriptEvent(ev))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (eventInfo.getterNum && listener->classinfo()->GetDef(eventInfo.getterNum))
	{
		ScriptError("Cannot set a read-only variable");
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

void ScriptVM::ExecCmdCommon(EventSystem& eventSystem, op_parmNum_t param)
{
	const op_ev_t eventNum = ReadOpcodeValue<op_ev_t>();

	pTop -= param;

	try
	{
		executeCommand(GetScriptThread(), param, eventNum);
	}
	catch (ScriptException&)
	{
		throw;
	}
}

void ScriptVM::ExecCmdMethodCommon(EventSystem& eventSystem, op_parmNum_t param)
{
	const ScriptVariable* const a = pTop--;
	const op_ev_t eventNum = ReadOpcodeValue<op_ev_t>();

	try
	{
		pTop -= param;

		const size_t arraysize = a->arraysize();
		if (arraysize == -1)
		{
			ScriptError("command '%s' applied to NIL", eventSystem.GetEventName(eventNum));
		}

		if (arraysize > 1)
		{
			if (a->IsConstArray())
			{
				for (uintptr_t i = a->arraysize(); i > 0; i--)
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
					Listener* const listener = array[i]->listenerAt(i);
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
				ScriptError("command '%s' applied to null listener", eventSystem.GetEventName(eventNum));
			}

			executeCommand<true>(listener, param, eventNum);
		}
	}
	catch (ScriptException&)
	{
		throw;
	}
}

void ScriptVM::ExecMethodCommon(EventSystem& eventSystem, op_parmNum_t param)
{
	const ScriptVariable* const a = pTop--;
	const op_ev_t eventNum = ReadOpcodeValue<op_ev_t>();

	try
	{
		pTop -= param;
		// push the return value
		pTop++;

		Listener* const listener = a->listenerValue();
		if (!listener)
		{
			ScriptError("command '%s' applied to NULL listener", eventSystem.GetEventName(eventNum));
		}

		executeCommand<true, true>(listener, param, eventNum);
	}
	catch (ScriptException&)
	{
		throw;
	}
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
				const str& labelName = Director.GetString(label);
				ScriptError("function '%s' applied to NULL listener", labelName.c_str());
			}
		}
		catch (ScriptException&)
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
		EnterFunction(Director, std::move(data));

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
				const str& labelStr = Director.GetString(label);
				const str& fileStr = Director.GetString(filename);
				ScriptError("function '%s' in '%s' applied to NULL listener", labelStr.c_str(), fileStr.c_str());
			}
		}
		catch (ScriptException&)
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
		*pTop = std::move(listener->ProcessEventReturn(ev));
	}
}

void ScriptVM::Execute(const ScriptVariable* data, size_t dataSize, const char* label)
{
	if (label)
	{
		// Throw if label is not found
		if (!(m_CodePos = m_ScriptClass->FindLabel(label)))
		{
			ScriptError("ScriptVM::Execute: label '%s' does not exist in '%s'.", label, Filename().c_str());
		}
	}

	ScriptContext& context = ScriptContext::Get();
	ScriptMaster& Director = context.GetDirector();
	EventSystem& eventSystem = EventSystem::Get();

	std::ostream* dbg = context.GetOutputInfo().GetOutput(outputLevel_e::Debug);

	if (Director.GetStackCount() >= MAX_STACK_DEPTH)
	{
		state = vmState_e::Execution;

		ScriptException::next_abort = -1;
		ScriptException exc("stack overflow");

		throw exc;
	}

	Director.AddStack();

	if (dataSize)
	{
		SetFastData(data, dataSize);
	}

	// set the current state as running
	state = vmState_e::Running;

	const TimeManager& timeManager = context.GetTimeManager();
	uinttime_t cmdTime = timeManager.GetTime();
	// set the next time at which the VM will timeout if it spends too many time executing
	const uinttime_t nextTime = cmdTime + 5000;

	//VMOperation operation(*this, ScriptContext::Get(), Director, EventSystem::Get());

	bool doneProcessing = false;
	bool deleteThread = false;

__restart:
	bool bShouldRestart = false;

	// Put the try/catch outside of the loop
	try
	{
		while (!doneProcessing && state == vmState_e::Running)
		{
			if (!m_bMarkStack)
			{
				const bool stackOverrun = pTop < localStack || pTop >= stackBottom;
				assert(!stackOverrun);
				if (stackOverrun)
				{
					deleteThread = true;
					state = vmState_e::Execution;

					if (pTop < localStack) {
						error("VM stack error. Negative stack value %d.", intptr_t(pTop - localStack));
					}
					else {
						error("VM stack error. Exceeded the maximum stack size %d.", stackBottom - localStack);
					}
					break;
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
				doJumpVarIf(pTop->GetData().long64Value);
				break;
			}

			case OP_BOOL_LOGICAL_OR:
			{
				doJumpVarIf(!pTop->GetData().long64Value);
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
				ExecCmdCommon(eventSystem, 0);
				break;
			}

			case OP_EXEC_CMD1:
			{
				ExecCmdCommon(eventSystem, 1);
				break;
			}

			case OP_EXEC_CMD2:
			{
				ExecCmdCommon(eventSystem, 2);
				break;
			}

			case OP_EXEC_CMD3:
			{
				ExecCmdCommon(eventSystem, 3);
				break;
			}

			case OP_EXEC_CMD4:
			{
				ExecCmdCommon(eventSystem, 4);
				break;
			}

			case OP_EXEC_CMD5:
			{
				ExecCmdCommon(eventSystem, 5);
				break;
			}

			case OP_EXEC_CMD_COUNT1:
			{
				const op_parmNum_t numParms = ReadOpcodeValue<op_parmNum_t>();
				ExecCmdCommon(eventSystem, numParms);
				break;
			}

			case OP_EXEC_CMD_METHOD0:
			{
				ExecCmdMethodCommon(eventSystem, 0);
				break;
			}

			case OP_EXEC_CMD_METHOD1:
			{
				ExecCmdMethodCommon(eventSystem, 1);
				break;
			}

			case OP_EXEC_CMD_METHOD2:
			{
				ExecCmdMethodCommon(eventSystem, 2);
				break;
			}

			case OP_EXEC_CMD_METHOD3:
			{
				ExecCmdMethodCommon(eventSystem, 3);
				break;
			}

			case OP_EXEC_CMD_METHOD4:
			{
				ExecCmdMethodCommon(eventSystem, 4);
				break;
			}

			case OP_EXEC_CMD_METHOD5:
			{
				ExecCmdMethodCommon(eventSystem, 5);
				break;
			}

			case OP_EXEC_CMD_METHOD_COUNT1:
			{
				const op_parmNum_t numParms = ReadOpcodeValue<op_parmNum_t>();
				ExecCmdMethodCommon(eventSystem, numParms);
				break;
			}

			case OP_EXEC_METHOD0:
			{
				ExecMethodCommon(eventSystem, 0);
				break;
			}

			case OP_EXEC_METHOD1:
			{
				ExecMethodCommon(eventSystem, 1);
				break;
			}

			case OP_EXEC_METHOD2:
			{
				ExecMethodCommon(eventSystem, 2);
				break;
			}

			case OP_EXEC_METHOD3:
			{
				ExecMethodCommon(eventSystem, 3);
				break;
			}

			case OP_EXEC_METHOD4:
			{
				ExecMethodCommon(eventSystem, 4);
				break;
			}

			case OP_EXEC_METHOD5:
			{
				ExecMethodCommon(eventSystem, 5);
				break;
			}

			case OP_EXEC_METHOD_COUNT1:
			{
				const op_parmNum_t numParms = ReadOpcodeValue<op_parmNum_t>();
				ExecMethodCommon(eventSystem, numParms);
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
						const str& value = Director.GetString(ReadGetOpcodeValue<op_name_t>());
						ScriptError("Field '%s' applied to NULL listener", value.c_str());
					}
					else
					{
						eventCalled = true;
						loadTop(eventSystem, listener);
					}
				}
				catch (ScriptException&)
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
					ScriptError("self is NULL");
				}

				if (!GetScriptClass()->GetSelf()->GetScriptOwner())
				{
					pTop--;
					skipField();
					ScriptError("self.owner is NULL");
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
					ScriptError("self is NULL");
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
					ScriptError("self is NULL");
				}

				if (!GetScriptClass()->GetSelf()->GetScriptOwner())
				{
					skipField();
					ScriptError("self.owner is NULL");
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
					ScriptError("self is NULL");
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

			/*
				if (fastEvent.dataSize)
				{
					pTop = fastEvent.data++;
					fastEvent.dataSize--;
				}
				else
				{
					pTop = m_StackPos + 1;
					pTop->Clear();
				}
			*/
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
				pTop--;
				pTop->evalArrayAt(*(pTop + 1));
				break;
			}

			case OP_STORE_ARRAY_REF:
			{
				pTop--;
				pTop->setArrayRefValue(*(pTop + 1));
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
						const str& value = Director.GetString(fieldName);
						skipField();
						ScriptError("Field '%s' applied to NULL listener", value.c_str());
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
				catch (ScriptException&)
				{
					pTop->setRefValue(pTop);
					throw;
				}
				break;
			}

			case OP_STORE_FIELD:
			{
				Listener* listener = pTop->listenerValue();

				if (listener == nullptr)
				{
					const str& value = Director.GetString(ReadGetOpcodeValue<op_name_t>());
					skipField();
					ScriptError("Field '%s' applied to NULL listener", value.c_str());
				}
				else
				{
					storeTop<true>(eventSystem, listener);
				}
				break;
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
					ScriptError("self is NULL");
				}

				if (!GetScriptClass()->GetSelf()->GetScriptOwner())
				{
					pTop++;
					skipField();
					ScriptError("self.owner is NULL");
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
					ScriptError("self is NULL");
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
					ScriptError("self is NULL");
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
				ConSimple* targetList = context.GetTargetList().GetExistingTargetList(pTop->constStringValue());

				if (!targetList)
				{
					pTop->setListenerValue(nullptr);

					const opval_t opcodeValue = ReadOpcodeValue<opval_t>();
					if ((opcodeValue >= OP_BIN_EQUALITY && opcodeValue <= OP_BIN_GREATER_THAN_OR_EQUAL)
					|| (opcodeValue >= OP_BOOL_UN_NOT && opcodeValue <= OP_UN_CAST_BOOLEAN)) {
						ScriptError("Targetname '%s' does not exist.", pTop->stringValue().c_str());
					}
				}

				if (targetList->NumObjects() == 1)
				{
					pTop->setListenerValue(targetList->ObjectAt(1));
				}
				else if (targetList->NumObjects() > 1)
				{
					pTop->setContainerValue((con::Container<SafePtr<Listener>> *)targetList);
				}
				else
				{

					pTop->setListenerValue(nullptr);

					const opval_t opcodeValue = ReadOpcodeValue<opval_t>();
					if ((opcodeValue >= OP_BIN_EQUALITY && opcodeValue <= OP_BIN_GREATER_THAN_OR_EQUAL)
					|| (opcodeValue >= OP_BOOL_UN_NOT && opcodeValue <= OP_UN_CAST_BOOLEAN))
					{
						const str value = pTop->stringValue();
						ScriptError("Targetname '%s' does not exist.", value.c_str());
					}
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

			if (cmdTime > nextTime)
			{
				if (Director.HasLoopProtection())
				{
					deleteThread = true;
					state = vmState_e::Execution;

					if (Director.HasLoopDrop())
					{
						ScriptException::next_abort = -1;
					}

					ScriptError("Command overflow. Possible infinite loop in thread.\n");
				}

				//VM_DPrintf("Update of script position - This is not an error.\n");
				//VM_DPrintf("=================================================\n");
				const ProgramScript* const scr = m_ScriptClass->GetScript();
				scr->PrintSourcePos(m_CodePos - scr->GetProgBuffer());
				//VM_DPrintf("=================================================\n");
			}

			/*
			if (*opcode < OP_MAX)
			{
				operation.Handle(*opcode);

				if (*opcode == OP_DONE)
				{
					doneProcessing = true;
				}
			}
			else
			{
				if (dbg)
				{
					*dbg << "unknown opcode " << (uint32_t)*opcode << std::endl;
				}
			}
			*/
			/*
				if (*opcode < OP_MAX)
				{
					//glbs.DPrintf("unknown opcode %d ('%s')\n", *opcode, OpcodeName(*opcode));
				}
				else
				{
					//glbs.DPrintf("unknown opcode %d\n", *opcode);
				}
			*/

			cmdTime = timeManager.GetTime();
		}
	}
	catch (ScriptException& exc)
	{
		HandleScriptException(exc, dbg);
		bShouldRestart = true;
	}

	if (bShouldRestart) goto __restart;

	assert(!doneProcessing || pTop == localStack);

	Director.RemoveStack();

	switch (state)
	{
	case vmState_e::Suspended:
		state = vmState_e::Execution;
		// FIXME: use m_Stack to save top?
		break;
	case vmState_e::Waiting:
		delete m_Thread;
		m_Thread = nullptr;
		break;
	case vmState_e::Destroyed:
		delete this;
		break;
	default:
		break;
	}

	if (deleteThread && m_Thread)
	{
		delete m_Thread;
		m_Thread = nullptr;
	}
}

#if 0

// By using a map it makes sure that opcode get properly mapped
// into its execution method
constexpr opMap_t opMap[] =
{
	{ OP_BIN_BITWISE_AND, &VMOperation::Exec_BinAnd },
	{ OP_BIN_BITWISE_OR, &VMOperation::Exec_BinOr },
	{ OP_BIN_BITWISE_EXCL_OR, &VMOperation::Exec_BinExclOr },
	{ OP_BIN_EQUALITY, &VMOperation::Exec_BinEquality },
	{ OP_BIN_INEQUALITY, &VMOperation::Exec_BinInequality },
	{ OP_BIN_GREATER_THAN, &VMOperation::Exec_BinGreater },
	{ OP_BIN_GREATER_THAN_OR_EQUAL, &VMOperation::Exec_BinGreaterEq },
	{ OP_BIN_LESS_THAN, &VMOperation::Exec_BinLess },
	{ OP_BIN_LESS_THAN_OR_EQUAL, &VMOperation::Exe_BinLessEq },
	{ OP_BIN_PLUS, &VMOperation::Exec_BinPlus },
	{ OP_BIN_MINUS, &VMOperation::Exec_BinMinus },
	{ OP_BIN_MULTIPLY, &VMOperation::Exec_BinMult },
	{ OP_BIN_DIVIDE, &VMOperation::Exec_BinDiv },
	{ OP_BIN_PERCENTAGE, &VMOperation::Exec_BinMod },
	{ OP_BIN_SHIFT_LEFT, &VMOperation::Exec_BinShl },
	{ OP_BIN_SHIFT_RIGHT, &VMOperation::Exec_BinShr },
	{ OP_BOOL_JUMP_FALSE4, &VMOperation::Exec_BoolJumpFalse },
	{ OP_BOOL_JUMP_TRUE4, &VMOperation::Exec_BoolJumpTrue },
	{ OP_VAR_JUMP_FALSE4, &VMOperation::Exec_VarJumpFalse },
	{ OP_VAR_JUMP_TRUE4, &VMOperation::Exec_VarJumpTrue },
	{ OP_BOOL_LOGICAL_AND, &VMOperation::Exec_BoolLogicalAnd },
	{ OP_BOOL_LOGICAL_OR, &VMOperation::Exec_BoolLogicalOr },
	{ OP_VAR_LOGICAL_AND, &VMOperation::Exec_VarLogicalAnd },
	{ OP_VAR_LOGICAL_OR, &VMOperation::Exec_VarLogicalOr },
	{ OP_BOOL_STORE_FALSE, &VMOperation::Exec_BoolStoreFalse },
	{ OP_BOOL_STORE_TRUE, &VMOperation::Exec_BoolStoreTrue },
	{ OP_BOOL_UN_NOT, &VMOperation::Exec_BoolUnNot },
	{ OP_CALC_VECTOR, &VMOperation::Exec_CalcVec },
	{ OP_EXEC_CMD0, &VMOperation::Exec_Cmd0 },
	{ OP_EXEC_CMD1, &VMOperation::Exec_Cmd1 },
	{ OP_EXEC_CMD2, &VMOperation::Exec_Cmd2 },
	{ OP_EXEC_CMD3, &VMOperation::Exec_Cmd3 },
	{ OP_EXEC_CMD4, &VMOperation::Exec_Cmd4 },
	{ OP_EXEC_CMD5, &VMOperation::Exec_Cmd5 },
	{ OP_EXEC_CMD_COUNT1, &VMOperation::Exec_CmdCount },
	{ OP_EXEC_CMD_METHOD0, &VMOperation::Exec_CmdMethod0 },
	{ OP_EXEC_CMD_METHOD1, &VMOperation::Exec_CmdMethod1 },
	{ OP_EXEC_CMD_METHOD2, &VMOperation::Exec_CmdMethod2 },
	{ OP_EXEC_CMD_METHOD3, &VMOperation::Exec_CmdMethod3 },
	{ OP_EXEC_CMD_METHOD4, &VMOperation::Exec_CmdMethod4 },
	{ OP_EXEC_CMD_METHOD5, &VMOperation::Exec_CmdMethod5 },
	{ OP_EXEC_CMD_METHOD_COUNT1, &VMOperation::Exec_CmdMethodCount },
	{ OP_EXEC_METHOD0, &VMOperation::Exec_Method0 },
	{ OP_EXEC_METHOD1, &VMOperation::Exec_Method1 },
	{ OP_EXEC_METHOD2, &VMOperation::Exec_Method2 },
	{ OP_EXEC_METHOD3, &VMOperation::Exec_Method3 },
	{ OP_EXEC_METHOD4, &VMOperation::Exec_Method4 },
	{ OP_EXEC_METHOD5, &VMOperation::Exec_Method5 },
	{ OP_EXEC_METHOD_COUNT1, &VMOperation::Exec_MethodCount },
	{ OP_FUNC, &VMOperation::Exec_Func },
	{ OP_JUMP4, &VMOperation::Exec_Jump4 },
	{ OP_JUMP_BACK4, &VMOperation::Exec_JumpBack4 },
	{ OP_LOAD_ARRAY_VAR, &VMOperation::Exec_LoadArrayVar },
	{ OP_LOAD_FIELD_VAR, &VMOperation::Exec_LoadFieldVar },
	{ OP_LOAD_CONST_ARRAY1, &VMOperation::Exec_LoadConstArray },
	{ OP_LOAD_GAME_VAR, &VMOperation::Exec_LoadGameVar },
	{ OP_LOAD_GROUP_VAR, &VMOperation::Exec_LoadGroupVar },
	{ OP_LOAD_LEVEL_VAR, &VMOperation::Exec_LoadLevelVar },
	{ OP_LOAD_LOCAL_VAR, &VMOperation::Exec_LoadLocalVar },
	{ OP_LOAD_OWNER_VAR, &VMOperation::Exec_LoadOwnerVar },
	{ OP_LOAD_PARM_VAR, &VMOperation::Exec_LoadParmVar },
	{ OP_LOAD_SELF_VAR, &VMOperation::Exec_LoadSelfVar },
	{ OP_LOAD_STORE_GAME_VAR, &VMOperation::Exec_LoadStoreGameVar },
	{ OP_LOAD_STORE_GROUP_VAR, &VMOperation::Exec_LoadStoreGroupVar },
	{ OP_LOAD_STORE_LEVEL_VAR, &VMOperation::Exec_LoadStoreLevelVar },
	{ OP_LOAD_STORE_LOCAL_VAR, &VMOperation::Exec_LoadStoreLocalVar },
	{ OP_LOAD_STORE_OWNER_VAR, &VMOperation::Exec_LoadStoreOwnerVar },
	{ OP_LOAD_STORE_PARM_VAR, &VMOperation::Exec_LoadStoreParmVar },
	{ OP_LOAD_STORE_SELF_VAR, &VMOperation::Exec_LoadStoreSelfVar },
	{ OP_MARK_STACK_POS, &VMOperation::Exec_MarkStackPos },
	{ OP_STORE_PARAM, &VMOperation::Exec_StoreParam },
	{ OP_RESTORE_STACK_POS, &VMOperation::Exec_RestoreStackPos },
	{ OP_STORE_ARRAY, &VMOperation::Exec_StoreArray },
	{ OP_STORE_ARRAY_REF, &VMOperation::Exec_StoreArrayRef },
	{ OP_STORE_FIELD_REF, &VMOperation::Exec_StoreFieldRef },
	{ OP_STORE_FIELD, &VMOperation::Exec_StoreField },
	{ OP_STORE_FLOAT, &VMOperation::Exec_StoreFloat },
	{ OP_STORE_INT0, &VMOperation::Exec_StoreInt0 },
	{ OP_STORE_INT1, &VMOperation::Exec_StoreInt1 },
	{ OP_STORE_INT2, &VMOperation::Exec_StoreInt2 },
	{ OP_STORE_INT3, &VMOperation::Exec_StoreInt3 },
	{ OP_STORE_INT4, &VMOperation::Exec_StoreInt4 },
	{ OP_STORE_INT8, &VMOperation::Exec_StoreInt8 },
	{ OP_STORE_GAME_VAR, &VMOperation::Exec_StoreGameVar },
	{ OP_STORE_GROUP_VAR, &VMOperation::Exec_StoreGroupVar },
	{ OP_STORE_LEVEL_VAR, &VMOperation::Exec_StoreLevelVar },
	{ OP_STORE_LOCAL_VAR, &VMOperation::Exec_StoreLocalVar },
	{ OP_STORE_OWNER_VAR, &VMOperation::Exec_StoreOwnerVar },
	{ OP_STORE_PARM_VAR, &VMOperation::Exec_StoreParmVar },
	{ OP_STORE_SELF_VAR, &VMOperation::Exec_StoreSelfVar },
	{ OP_STORE_GAME, &VMOperation::Exec_StoreGame },
	{ OP_STORE_GROUP, &VMOperation::Exec_StoreGroup },
	{ OP_STORE_LEVEL, &VMOperation::Exec_StoreLevel },
	{ OP_STORE_LOCAL, &VMOperation::Exec_StoreLocal },
	{ OP_STORE_OWNER, &VMOperation::Exec_StoreOwner },
	{ OP_STORE_PARM, &VMOperation::Exec_StoreParm },
	{ OP_STORE_SELF, &VMOperation::Exec_StoreSelf },
	{ OP_STORE_NIL, &VMOperation::Exec_StoreNil },
	{ OP_STORE_NULL, &VMOperation::Exec_StoreNull },
	{ OP_STORE_STRING, &VMOperation::Exec_StoreString },
	{ OP_STORE_VECTOR, &VMOperation::Exec_StoreVector },
	{ OP_SWITCH, &VMOperation::Exec_Switch },
	{ OP_UN_CAST_BOOLEAN, &VMOperation::Exec_UnCastBoolean },
	{ OP_UN_COMPLEMENT, &VMOperation::Exec_UnComplement },
	{ OP_UN_MINUS, &VMOperation::Exec_UnMinus},
	{ OP_UN_DEC, &VMOperation::Exec_UnDec },
	{ OP_UN_INC, &VMOperation::Exec_UnInc },
	{ OP_UN_SIZE, &VMOperation::Exec_UnSize },
	{ OP_UN_TARGETNAME, &VMOperation::Exec_UnTargetname },
	{ OP_VAR_UN_NOT, &VMOperation::Exec_VarUnNot },
	{ OP_DONE, &VMOperation::Exec_Done },
	{ OP_NOP, &VMOperation::Exec_Nop }
};

const opList_c<OP_MAX, sizeof(opMap) / sizeof(opMap[0])> opList(opMap);

VMOperation::VMOperation(ScriptVM& vmParent, ScriptContext& contextValue, ScriptMaster& directorValue, EventSystem& eventSystemValue)
	: vm(vmParent)
	, context(contextValue)
	, director(directorValue)
	, eventSystem(eventSystemValue)
{
}

void VMOperation::executeCommand(Listener* listener, uint16_t iParamCount, int eventnum, bool bMethod, bool bReturn)
{
	ScriptVariable* var;

	Event ev(eventnum);

	if (bReturn)
	{
		var = vm.pTop;
	}
	else
	{
		var = vm.pTop + 1;
	}

	ev.dataSize = iParamCount;
	if (iParamCount)
	{
		ev.data = new ScriptVariable[ev.dataSize];
	}

	ev.fromScript = true;

	if (!bMethod)
	{
		for (uint16_t i = 0; i < iParamCount; i++)
		{
			ev.data[i] = var[i];
		}
	}
	else
	{
		for (uint16_t i = 0; i < iParamCount; i++)
		{
			ev.data[i] = var[i + 1];
		}
	}

	listener->ProcessScriptEvent(ev);

	if (ev.NumArgs() > iParamCount)
	{
		*vm.pTop = std::move(ev.GetValue(ev.NumArgs()));
	}
	else
	{
		vm.pTop->Clear();
	}
}

bool VMOperation::executeGetter(Listener* listener, op_evName_t eventName)
{
	const eventInfo_t* eventInfo = eventSystem.FindEventInfo((eventName_t)eventName);
	if (!eventInfo)
	{
		// not an existing event
		return false;
	}

	if (eventInfo.getterNum && listener->classinfo()->GetDef(eventInfo.getterNum))
	{
		Event ev(eventInfo.getterNum);
		ev.fromScript = true;

		if (listener->ProcessScriptEvent(ev))
		{
			if (ev.NumArgs() > 0)
			{
				*vm.pTop = std::move(ev.GetValue(ev.NumArgs()));
			}
			else
			{
				vm.pTop->Clear();
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else if (eventInfo.setterNum && listener->classinfo()->GetDef(eventInfo.setterNum))
	{
		ScriptError("Cannot get a write-only variable");
	}

	return false;
}

bool VMOperation::executeSetter(Listener* listener, op_evName_t eventName)
{
	const eventInfo_t* eventInfo = eventSystem.FindEventInfo((eventName_t)eventName);
	if (!eventInfo)
	{
		// not an existing event
		return false;
	}

	if (eventInfo.setterNum && listener->classinfo()->GetDef(eventInfo.setterNum))
	{
		Event ev(eventInfo.setterNum);
		ev.fromScript = true;

		ev.AddValue(*vm.pTop);

		if (listener->ProcessScriptEvent(ev))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (eventInfo.getterNum && listener->classinfo()->GetDef(eventInfo.getterNum))
	{
		ScriptError("Cannot set a read-only variable");
	}

	return false;
}

void VMOperation::jump(op_offset_t offset)
{
	vm.m_CodePos += offset;
}

void VMOperation::jumpBool(op_offset_t offset, bool booleanValue)
{
	if (booleanValue)
	{
		jump(offset);
	}
	else
	{
		vm.m_CodePos += sizeof(op_offset_t);
	}
}

template<>
void VMOperation::loadTop<false>(Listener* listener)
{
	const const_str variable = *reinterpret_cast<const op_name_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_name_t);
	const op_evName_t eventName = *reinterpret_cast<const op_evName_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_evName_t);

	if (!executeSetter(listener, eventName))
	{
		// just set the variable
		listener->Vars()->SetVariable(variable, std::move(*vm.pTop));
	}

	vm.pTop--;
}

template<>
void VMOperation::loadTop<true>(Listener* listener)
{
	const const_str variable = *reinterpret_cast<const op_name_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_name_t);
	const op_evName_t eventName = *reinterpret_cast<const op_evName_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_evName_t);

	if (!executeSetter(listener, eventName))
	{
		// just set the variable
		listener->Vars()->SetVariable(variable, std::move(*vm.pTop));
	}
}

template<>
void VMOperation::storeTop<false>(Listener* listener)
{
	const const_str variable = *reinterpret_cast<const op_name_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_name_t);
	const op_evName_t eventName = *reinterpret_cast<const op_evName_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_evName_t);

	vm.pTop++;

	if (!executeGetter(listener, eventName))
	{
		*vm.pTop = *listener->Vars()->GetOrCreateVariable(variable);
	}
}

template<>
void VMOperation::storeTop<true>(Listener* listener)
{
	const const_str variable = *reinterpret_cast<const op_name_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_name_t);
	const op_evName_t eventName = *reinterpret_cast<const op_evName_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_evName_t);

	if (!executeGetter(listener, eventName))
	{
		*vm.pTop = *listener->Vars()->GetOrCreateVariable(variable);
	}
}

void VMOperation::Handle(opval_t op)
{
	(this->*opList[op])();
}

void VMOperation::Exec_Done()
{
	vm.End();
}

void VMOperation::Exec_BinAnd()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b &= *a;
}

void VMOperation::Exec_BinOr()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b |= *a;
}

void VMOperation::Exec_BinExclOr()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b ^= *a;
}

void VMOperation::Exec_BinEquality()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	b->setIntValue(*b == *a);
}

void VMOperation::Exec_BinInequality()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	b->setIntValue(*b != *a);
}

void VMOperation::Exec_BinGreater()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	b->greaterthan(*a);
}

void VMOperation::Exec_BinGreaterEq()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	b->greaterthanorequal(*a);
}

void VMOperation::Exec_BinLess()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	b->lessthan(*a);
}

void VMOperation::Exe_BinLessEq()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	b->lessthanorequal(*a);
}

void VMOperation::Exec_BinPlus()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b += *a;
}

void VMOperation::Exec_BinMinus()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b -= *a;
}

void VMOperation::Exec_BinMult()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b *= *a;
}

void VMOperation::Exec_BinDiv()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b /= *a;
}

void VMOperation::Exec_BinMod()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b %= *a;
}

void VMOperation::Exec_BinShl()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b <<= *a;
}

void VMOperation::Exec_BinShr()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop;

	*b >>= *a;
}

void VMOperation::Exec_BoolJumpFalse()
{
	jumpBool(*reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t), !vm.pTop->GetData().intValue);
	vm.pTop--;
}

void VMOperation::Exec_BoolJumpTrue()
{
	jumpBool(*reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t), vm.pTop->GetData().intValue ? true : false);
	vm.pTop--;
}

void VMOperation::Exec_VarJumpFalse()
{

	jumpBool(*reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t), !vm.pTop->booleanValue());
	vm.pTop--;
}

void VMOperation::Exec_VarJumpTrue()
{
	jumpBool(*reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t), vm.pTop->booleanValue());
	vm.pTop--;
}

void VMOperation::Exec_BoolLogicalAnd()
{
	if (vm.pTop->GetData().intValue)
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_offset_t);
	}
	else
	{
		vm.m_CodePos += *reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t);
	}
}

void VMOperation::Exec_BoolLogicalOr()
{
	if (!vm.pTop->GetData().intValue)
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_offset_t);
	}
	else
	{
		vm.m_CodePos += *reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t);
	}
}

void VMOperation::Exec_VarLogicalAnd()
{
	if (vm.pTop->booleanValue())
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_offset_t);
	}
	else
	{
		vm.pTop->SetFalse();
		vm.m_CodePos += *reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t);
	}
}

void VMOperation::Exec_VarLogicalOr()
{
	if (!vm.pTop->booleanValue())
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_offset_t);
	}
	else
	{
		vm.pTop->SetTrue();
		vm.m_CodePos += *reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t);
	}
}

void VMOperation::Exec_BoolStoreFalse()
{
	vm.pTop++;
	vm.pTop->SetFalse();
}

void VMOperation::Exec_BoolStoreTrue()
{
	vm.pTop++;
	vm.pTop->SetTrue();
}

void VMOperation::Exec_BoolUnNot()
{
	vm.pTop->GetData().intValue = (vm.pTop->GetData().intValue == 0);
}

void VMOperation::Exec_CalcVec()
{
	const ScriptVariable* const c = vm.pTop--;
	const ScriptVariable* const b = vm.pTop--;
	const ScriptVariable* const a = vm.pTop;

	vm.pTop->setVectorValue(Vector(a->floatValue(), b->floatValue(), c->floatValue()));
}

void VMOperation::Exec_Cmd0()
{
	Exec_CmdCommon(0);
}

void VMOperation::Exec_Cmd1()
{
	Exec_CmdCommon(1);
}

void VMOperation::Exec_Cmd2()
{
	Exec_CmdCommon(2);
}

void VMOperation::Exec_Cmd3()
{
	Exec_CmdCommon(3);
}

void VMOperation::Exec_Cmd4()
{
	Exec_CmdCommon(4);
}

void VMOperation::Exec_Cmd5()
{
	Exec_CmdCommon(5);
}

void VMOperation::Exec_CmdCount()
{
	const op_parmNum_t numParms = *reinterpret_cast<const op_parmNum_t*>(vm.m_CodePos);
	vm.m_CodePos += numParms;
	Exec_CmdCommon(numParms);
}

void VMOperation::Exec_CmdCommon(size_t param)
{
	const op_ev_t eventNum = *reinterpret_cast<const op_ev_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_ev_t);

	vm.pTop -= param;

	try
	{
		executeCommand(GetThread(), param, eventNum);
	}
	catch (ScriptException&)
	{
		throw;
	}
}

void VMOperation::Exec_CmdMethod0()
{
	Exec_CmdMethodCommon(0);
}

void VMOperation::Exec_CmdMethod1()
{
	Exec_CmdMethodCommon(1);
}

void VMOperation::Exec_CmdMethod2()
{
	Exec_CmdMethodCommon(2);
}

void VMOperation::Exec_CmdMethod3()
{
	Exec_CmdMethodCommon(3);
}

void VMOperation::Exec_CmdMethod4()
{
	Exec_CmdMethodCommon(4);
}

void VMOperation::Exec_CmdMethod5()
{
	Exec_CmdMethodCommon(5);
}

void VMOperation::Exec_CmdMethodCount()
{
	const op_parmNum_t numParms = *reinterpret_cast<const op_parmNum_t*>(vm.m_CodePos);
	vm.m_CodePos += numParms;
	Exec_CmdMethodCommon(numParms);
}

void VMOperation::Exec_CmdMethodCommon(size_t param)
{
	const ScriptVariable* const a = vm.pTop--;

	try
	{
		EventSystem& eventManager = eventSystem;

		const op_ev_t eventNum = *reinterpret_cast<const op_ev_t*>(vm.m_CodePos);

		vm.pTop -= param;

		if (a->arraysize() < 0)
		{
			ScriptError("command '%s' applied to NIL", eventManager.GetEventName(eventNum));
		}

		ScriptVariable array = *a;
		Listener* listener;

		array.CastConstArrayValue();

		for (intptr_t i = array.arraysize(); i > 0; i--)
		{
			if (!(listener = array[i]->listenerValue()))
			{
				ScriptError("command '%s' applied to NULL listener", eventManager.GetEventName(eventNum));
			}

			executeCommand(listener, param, eventNum, true);
		}
	}
	catch (ScriptException&)
	{
		vm.m_CodePos += sizeof(op_parmNum_t) + sizeof(op_ev_t);
		throw;
	}

	vm.m_CodePos += sizeof(op_parmNum_t) + sizeof(op_ev_t);
}

void VMOperation::Exec_Method0()
{
	Exec_MethodCommon(0);
}

void VMOperation::Exec_Method1()
{
	Exec_MethodCommon(1);
}

void VMOperation::Exec_Method2()
{
	Exec_MethodCommon(2);
}

void VMOperation::Exec_Method3()
{
	Exec_MethodCommon(3);
}

void VMOperation::Exec_Method4()
{
	Exec_MethodCommon(4);
}

void VMOperation::Exec_Method5()
{
	Exec_MethodCommon(5);
}

void VMOperation::Exec_MethodCount()
{
	const op_parmNum_t numParms = *reinterpret_cast<const op_parmNum_t*>(vm.m_CodePos);
	vm.m_CodePos += numParms;
	Exec_MethodCommon(numParms);
}

void VMOperation::Exec_MethodCommon(size_t param)
{
	const ScriptVariable* const a = vm.pTop--;

	try
	{
		EventSystem& eventManager = eventSystem;
		const op_ev_t eventNum = *reinterpret_cast<const op_ev_t*>(vm.m_CodePos);

		vm.pTop -= param;
		vm.pTop++; // push the return value

		Listener* listener = a->listenerValue();
		if (!listener)
		{
			ScriptError("command '%s' applied to NULL listener", eventManager.GetEventName(eventNum));
		}

		executeCommand(listener, param, eventNum, true, true);
	}
	catch (ScriptException&)
	{
		vm.m_CodePos += sizeof(op_parmNum_t) + sizeof(op_ev_t);
		throw;
	}

	vm.m_CodePos += sizeof(op_parmNum_t) + sizeof(op_ev_t);
}

void VMOperation::Exec_Func()
{
	Event ev;

	ScriptMaster& Director = director;

	if (!*vm.m_CodePos++)
	{
		const str& label = Director.GetString(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));

		vm.m_CodePos += sizeof(op_name_t);

		Listener* listener;
		try
		{
			listener = vm.pTop->listenerValue();

			if (!listener)
			{
				ScriptError("function '%s' applied to NULL listener", label.c_str());
			}
		}
		catch (ScriptException&)
		{
			vm.pTop -= *vm.m_CodePos++;
			throw;
		}

		vm.pTop--;

		ev.AddString(label);

		op_parmNum_t params = *vm.m_CodePos++;

		const ScriptVariable* var = vm.pTop;
		vm.pTop -= params;

		for (int i = 0; i < params; var++, i++)
		{
			ev.AddValue(*var);
		}

		vm.pTop++;
		vm.EnterFunction(&ev);

		GetScriptClass()->SetSelf(listener);
	}
	else
	{
		str filename, label;

		filename = Director.GetString(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));
		vm.m_CodePos += sizeof(op_name_t);
		label = Director.GetString(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));
		vm.m_CodePos += sizeof(op_name_t);

		Listener* listener;
		try
		{
			listener = vm.pTop->listenerValue();

			if (!listener)
			{
				ScriptError("function '%s' in '%s' applied to NULL listener", label.c_str(), filename.c_str());
			}
		}
		catch (ScriptException&)
		{
			vm.pTop -= *vm.m_CodePos++;
			throw;
		}

		vm.pTop--;

		ScriptVariable constarray;
		ScriptVariable* pVar = new ScriptVariable[2];

		pVar[0].setStringValue(filename);
		pVar[1].setStringValue(label);

		constarray.setConstArrayValue(pVar, 2);

		delete[] pVar;

		Event ev2(EV_Listener_WaitCreateReturnThread);
		ev.AddValue(constarray);

		op_parmNum_t params = *vm.m_CodePos++;

		const ScriptVariable* var = vm.pTop;
		vm.pTop -= params;

		for (int i = 0; i < params; var++, i++) {
			ev2.AddValue(*var);
		}

		vm.pTop++;
		*vm.pTop = std::move(listener->ProcessEventReturn(ev2));
	}
}

void VMOperation::Exec_Jump4()
{
	vm.m_CodePos += *reinterpret_cast<const op_offset_t*>(vm.m_CodePos) + sizeof(op_offset_t);
}

void VMOperation::Exec_JumpBack4()
{
	vm.m_CodePos -= *reinterpret_cast<const op_offset_t*>(vm.m_CodePos);
}

void VMOperation::Exec_LoadArrayVar()
{
	const ScriptVariable* const a = vm.pTop--;
	ScriptVariable* const b = vm.pTop--;
	const ScriptVariable* const c = vm.pTop--;

	b->setArrayAt(*a, *c);
}

void VMOperation::Exec_LoadFieldVar()
{
	ScriptVariable* const a = vm.pTop--;
	bool eventCalled = false;

	try
	{
		Listener* listener = a->listenerValue();

		if (listener == nullptr)
		{
			ScriptMaster& Director = director;
			const str& value = Director.GetString(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));
			ScriptError("Field '%s' applied to NULL listener", value.c_str());
		}
		else
		{
			eventCalled = true;
			loadTop(listener);
		}
	}
	catch (ScriptException&)
	{
		vm.pTop--;

		if (!eventCalled) {
			vm.m_CodePos += sizeof(op_name_t);
		}

		throw;
	}
}

void VMOperation::Exec_LoadConstArray()
{
	op_arrayParmNum_t numParms = *reinterpret_cast<const op_arrayParmNum_t*>(vm.m_CodePos);
	vm.m_CodePos += sizeof(op_arrayParmNum_t);

	vm.pTop -= numParms - 1;
	vm.pTop->setConstArrayValue(vm.pTop, numParms);
}

void VMOperation::Exec_LoadGameVar()
{
	loadTop(context.GetGame());
}

void VMOperation::Exec_LoadGroupVar()
{
	loadTop(GetScriptClass());
}

void VMOperation::Exec_LoadLevelVar()
{
	loadTop(context.GetLevel());
}

void VMOperation::Exec_LoadLocalVar()
{
	loadTop(GetThread());
}

void VMOperation::Exec_LoadOwnerVar()
{
	if (!GetScriptClass()->GetSelf())
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self is NULL");
	}

	if (!GetScriptClass()->GetSelf()->GetScriptOwner())
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self.owner is NULL");
	}

	loadTop(GetScriptClass()->GetSelf()->GetScriptOwner());
}

void VMOperation::Exec_LoadParmVar()
{
	loadTop(&director.GetParm());
}

void VMOperation::Exec_LoadSelfVar()
{
	if (!GetScriptClass()->GetSelf())
	{
		vm.pTop--;
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self is NULL");
	}

	loadTop(GetScriptClass()->GetSelf());
}

void VMOperation::Exec_LoadStoreGameVar()
{
	loadTop<true>(context.GetGame());
}

void VMOperation::Exec_LoadStoreGroupVar()
{
	loadTop<true>(GetScriptClass());
}

void VMOperation::Exec_LoadStoreLevelVar()
{
	loadTop<true>(context.GetLevel());
}

void VMOperation::Exec_LoadStoreLocalVar()
{
	loadTop<true>(GetThread());
}

void VMOperation::Exec_LoadStoreOwnerVar()
{
	if (!GetScriptClass()->GetSelf())
	{
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self is NULL");
	}

	if (!GetScriptClass()->GetSelf()->GetScriptOwner())
	{
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self.owner is NULL");
	}

	loadTop<true>(GetScriptClass()->GetSelf()->GetScriptOwner());
}

void VMOperation::Exec_LoadStoreParmVar()
{
	loadTop<true>(&director.GetParm());
}

void VMOperation::Exec_LoadStoreSelfVar()
{
	if (!GetScriptClass()->GetSelf())
	{
		ScriptError("self is NULL");
	}

	loadTop<true>(GetScriptClass()->GetSelf());
}

void VMOperation::Exec_MarkStackPos()
{
	vm.m_StackPos = vm.pTop;
	vm.m_bMarkStack = true;
}

void VMOperation::Exec_StoreParam()
{
	if (vm.fastEvent.dataSize)
	{
		vm.pTop = vm.fastEvent.data++;
		vm.fastEvent.dataSize--;
	}
	else
	{
		vm.pTop = vm.m_StackPos + 1;
		vm.pTop->Clear();
	}
}

void VMOperation::Exec_RestoreStackPos()
{
	vm.pTop = vm.m_StackPos;
	vm.m_bMarkStack = false;
}

void VMOperation::Exec_StoreArray()
{
	vm.pTop--;
	vm.pTop->evalArrayAt(*(vm.pTop + 1));
}

void VMOperation::Exec_StoreArrayRef()
{
	vm.pTop--;
	vm.pTop->setArrayRefValue(*(vm.pTop + 1));
}

void VMOperation::Exec_StoreFieldRef()
{
	bool eventCalled = false;
	try
	{
		const str& value = director.GetString(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));

		Listener* listener = vm.pTop->listenerValue();

		if (listener == nullptr)
		{
			ScriptError("Field '%s' applied to NULL listener", value.c_str());
		}
		else
		{
			eventCalled = true;
			storeTop<true>(listener);
		}

		vm.pTop->setRefValue(listener->Vars()->GetOrCreateVariable(value));
	}
	catch (ScriptException&)
	{
		vm.pTop->setRefValue(vm.pTop);

		if (!eventCalled) {
			vm.m_CodePos += sizeof(op_name_t);
		}
		throw;
	}
}

void VMOperation::Exec_StoreField()
{
	bool eventCalled = false;
	try
	{
		const str& value = director.GetString(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));

		Listener* listener = vm.pTop->listenerValue();

		if (listener == nullptr)
		{
			ScriptError("Field '%s' applied to NULL listener", value.c_str());
		}
		else
		{
			eventCalled = true;
			storeTop<true>(listener);
		}
	}
	catch (ScriptException&)
	{
		if (!eventCalled) {
			vm.m_CodePos += sizeof(op_name_t);
		}
		throw;
	}
}

void VMOperation::Exec_StoreFloat()
{
	vm.pTop++;
	vm.pTop->setFloatValue(*reinterpret_cast<const float*>(vm.m_CodePos));
	vm.m_CodePos += sizeof(float);
}

void VMOperation::Exec_StoreInt0()
{
	vm.pTop++;
	vm.pTop->setIntValue(0);
}

void VMOperation::Exec_StoreInt1()
{
	vm.pTop++;
	vm.pTop->setIntValue(*vm.m_CodePos++);
}

void VMOperation::Exec_StoreInt2()
{
	vm.pTop++;
	vm.pTop->setIntValue(*reinterpret_cast<const uint16_t*>(vm.m_CodePos));

	vm.m_CodePos += sizeof(uint16_t);
}

void VMOperation::Exec_StoreInt3()
{
	vm.pTop++;
	vm.pTop->setIntValue(*reinterpret_cast<const short3*>(vm.m_CodePos));

	vm.m_CodePos += sizeof(short3);
}

void VMOperation::Exec_StoreInt4()
{
	vm.pTop++;
	vm.pTop->setIntValue(*reinterpret_cast<const uint32_t*>(vm.m_CodePos));

	vm.m_CodePos += sizeof(uint32_t);
}

void VMOperation::Exec_StoreInt8()
{
	vm.pTop++;
	vm.pTop->setLongValue(*reinterpret_cast<const uint64_t*>(vm.m_CodePos));

	vm.m_CodePos += sizeof(uint64_t);
}

void VMOperation::Exec_StoreGameVar()
{
	storeTop(context.GetGame());
}

void VMOperation::Exec_StoreGroupVar()
{
	storeTop(GetScriptClass());
}

void VMOperation::Exec_StoreLevelVar()
{
	storeTop(context.GetLevel());
}

void VMOperation::Exec_StoreLocalVar()
{
	storeTop(GetThread());
}

void VMOperation::Exec_StoreOwnerVar()
{
	if (!GetScriptClass()->GetSelf())
	{
		vm.pTop++;
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self is NULL");
	}

	if (!GetScriptClass()->GetSelf()->GetScriptOwner())
	{
		vm.pTop++;
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self.owner is NULL");
	}

	storeTop(GetScriptClass()->GetSelf()->GetScriptOwner());
}

void VMOperation::Exec_StoreParmVar()
{
	storeTop(&director.GetParm());
}

void VMOperation::Exec_StoreSelfVar()
{
	if (!GetScriptClass()->GetSelf())
	{
		vm.pTop++;
		vm.m_CodePos += sizeof(op_name_t);
		ScriptError("self is NULL");
	}

	storeTop(GetScriptClass()->GetSelf());
}

void VMOperation::Exec_StoreGame()
{
	vm.pTop++;
	vm.pTop->setListenerValue(context.GetGame());
}

void VMOperation::Exec_StoreGroup()
{
	vm.pTop++;
	vm.pTop->setListenerValue(GetScriptClass());
}

void VMOperation::Exec_StoreLevel()
{
	vm.pTop++;
	vm.pTop->setListenerValue(context.GetLevel());
}

void VMOperation::Exec_StoreLocal()
{
	vm.pTop++;
	vm.pTop->setListenerValue(GetThread());
}

void VMOperation::Exec_StoreOwner()
{
	vm.pTop++;

	if (!GetScriptClass()->GetSelf())
	{
		vm.pTop++;
		ScriptError("self is NULL");
	}

	vm.pTop->setListenerValue(GetScriptClass()->GetSelf()->GetScriptOwner());
}

void VMOperation::Exec_StoreParm()
{
	vm.pTop++;
	vm.pTop->setListenerValue(&director.GetParm());
}

void VMOperation::Exec_StoreSelf()
{
	vm.pTop++;
	vm.pTop->setListenerValue(GetScriptClass()->GetSelf());
}

void VMOperation::Exec_StoreNil()
{
	vm.pTop++;
	vm.pTop->Clear();
}

void VMOperation::Exec_StoreNull()
{
	vm.pTop++;
	vm.pTop->setListenerValue(nullptr);
}

void VMOperation::Exec_StoreString()
{
	vm.pTop++;
	vm.pTop->setConstStringValue(*reinterpret_cast<const op_name_t*>(vm.m_CodePos));

	vm.m_CodePos += sizeof(op_name_t);
}

void VMOperation::Exec_StoreVector()
{
	vm.pTop++;
	vm.pTop->setVectorValue(*reinterpret_cast<const Vector*>(vm.m_CodePos));

	vm.m_CodePos += sizeof(Vector);
}

void VMOperation::Exec_Switch()
{
	if (!vm.Switch(*reinterpret_cast<StateScript* const*>(vm.m_CodePos), *vm.pTop))
	{
		vm.m_CodePos += sizeof(StateScript*);
	}

	vm.pTop--;
}

void VMOperation::Exec_UnCastBoolean()
{
	vm.pTop->CastBoolean();
}

void VMOperation::Exec_UnComplement()
{
	vm.pTop->complement();
}

void VMOperation::Exec_UnMinus()
{
	vm.pTop->minus();
}

void VMOperation::Exec_UnDec()
{
	(*vm.pTop)--;
}

void VMOperation::Exec_UnInc()
{
	(*vm.pTop)++;
}

void VMOperation::Exec_UnSize()
{
	vm.pTop->setLongValue((uintptr_t)vm.pTop->size());
}

void VMOperation::Exec_UnTargetname()
{
	ConSimple* targetList = context.GetTargetList().GetExistingTargetList(vm.pTop->constStringValue());

	if (!targetList)
	{
		vm.pTop->setListenerValue(nullptr);

		if ((*vm.m_CodePos >= OP_BIN_EQUALITY && *vm.m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL) || (*vm.m_CodePos >= OP_BOOL_UN_NOT && *vm.m_CodePos <= OP_UN_CAST_BOOLEAN)) {
			ScriptError("Targetname '%s' does not exist.", vm.pTop->stringValue().c_str());
		}
	}

	if (targetList->NumObjects() == 1)
	{
		vm.pTop->setListenerValue(targetList->ObjectAt(1));
	}
	else if (targetList->NumObjects() > 1)
	{
		vm.pTop->setContainerValue((Container<SafePtr<Listener>> *)targetList);
	}
	else
	{
		const str value = vm.pTop->stringValue();

		vm.pTop->setListenerValue(nullptr);

		if ((*vm.m_CodePos >= OP_BIN_EQUALITY && *vm.m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL) || (*vm.m_CodePos >= OP_BOOL_UN_NOT && *vm.m_CodePos <= OP_UN_CAST_BOOLEAN)) {
			ScriptError("Targetname '%s' does not exist.", value.c_str());
		}
	}
}

void VMOperation::Exec_VarUnNot()
{
	vm.pTop->setIntValue(vm.pTop->booleanValue());
}

void VMOperation::Exec_Nop()
{

}

ScriptThread* VMOperation::GetThread() const
{
	return vm.GetScriptThread();
}

ScriptClass* VMOperation::GetScriptClass() const
{
	return vm.GetScriptClass();
}
#endif
