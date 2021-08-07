#include "Compiler.h"
#include <morfuse/Script/Parm.h>
#include <morfuse/Script/Level.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptOpcodes.h>
#include <morfuse/Script/Archiver.h>
#include <morfuse/Script/EventSystem.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Context.h>
#include <morfuse/Script/SourceException.h>
#include <morfuse/Common/short3.h>
#include <morfuse/Common/OutputInfo.h>
#include <Parser/yyParser.hpp>
//#include <Parser/yyLexer.hpp>
#include <Parser/Lexer.hpp>
#include "../Parser/parsetree.h"

#include <algorithm>
#include <cstdarg>
#include <sstream>
#include <charconv>
#include <iomanip>

namespace mfuse
{
	class ScriptCountManager;
	class ScriptProgramManager;
}

using namespace mfuse;

void Parser::error(const location_type& loc, const std::string& msg)
{
	const prchar_t* text = lexer.YYText();
	const uint32_t lineNumber = lexer.lineno();
	const uint32_t leng = lexer.YYLeng();

	//parsetree.pos -= yyleng;
	lexer.exception.text = text;
	lexer.exception.msg = msg.c_str();
	lexer.exception.loc.line = lexer.get_prev_lex() != token::TOKEN_EOL ? (loc.lineno - 1) : loc.begin.line;
	lexer.exception.loc.column = loc.begin.column;
	lexer.exception.loc.sourcePos = loc.sourcePos;

	//xstr line = ScriptEmitter::GetLine( parsetree.sourceBuffer, parsetree.exc.yylineno );

	//printf("parse error:\n%s:\n"), pt.exc.token.c_str();

	// FIXME
	//pt.gameScript->PrintSourcePos(parsetree.pos - leng);
	//pt.pos++;
}

class mfuse::ScriptCountManager : public IScriptManager
{
public:
	ScriptCountManager()
		: prevop{0}
	{
		curop = prevop;
	}

	StateScript* CreateCatchStateScript(const opval_t* try_begin_code_pos, const opval_t* code_pos, size_t labelCount) override
	{
		++info.numCatches;
		return nullptr;
	}

	StateScript* CreateSwitchStateScript(size_t labelCount) override
	{
		++info.numSwitches;
		return nullptr;
	}

	bool AddLabel(StateScript* stateScript, const prchar_t* name, const opval_t* code_pos, bool privateLabel) override
	{
		++info.numStrings;
		++info.numLabels;
		return true;
	}

	bool AddCaseLabel(StateScript* stateScript, const prchar_t* name, const opval_t* code_pos) override
	{
		++info.numStrings;
		++info.numCaseLabels;
		return true;
	}

	const_str AddString(const strview& value) override
	{
		++info.numStrings;
		return info.numStrings;
	}

	size_t GetCodePosition() override
	{
		return 0;
	}

	opval_t* GetCodePositionPtr() override
	{
		return nullptr;
	}

	size_t GetProgLength() override
	{
		return 0;
	}

	opval_t* MoveCodeBack(size_t backCount) override
	{
		const size_t pos = curop - prevop;

		size_t cback = backCount;
		while (cback > prevopSize) {
			cback -= prevopSize;
		}

		opval_t* op = curop;
		if (cback > pos) {
			curop = prevop_last_ptr() - cback;
		}
		else {
			curop -= cback;
		}

		return op;
	}

	opval_t* MoveCodeForward(size_t fwdCount) override
	{
		info.progLength += fwdCount;

		opval_t* op = curop;
		const size_t newPos = (curop + fwdCount) - prevop;
		curop = prevop + (newPos % prevopSize);
		return op;
	}

	void SetValueAtCodePosition(opval_t* pos, const void* value, size_t size) override
	{
	}

	void GetValueAt(uintptr_t backOffset, void* value, size_t size) override
	{
		size_t csize = size;
		while (csize > prevopSize) {
			csize -= prevopSize;
		}

		const size_t cback = backOffset > prevopSize ? prevopSize : backOffset;
		const size_t pos = curop - prevop;
		opval_t* off = curop;
		if (cback > pos) {
			off = prevop_last_ptr() - cback;
		} else {
			off = curop - cback;
		}

		const size_t remainingLast = prevopSize - pos;
		if (csize > remainingLast)
		{
			memcpy(value, off, remainingLast);
			memcpy(value, prevop, csize - remainingLast);
		}
		else {
			memcpy(value, off, csize);
		}

	}

	void WriteOpcodeValue(const void* value, size_t size) override
	{
		info.progLength += size;

		size_t csize = size;
		while (csize > 0)
		{
			const size_t bytesToWrite = size > prevopSize ? prevopSize : size;
			memcpy(curop, value, bytesToWrite);
			const size_t newPos = (curop + bytesToWrite) - prevop;
			curop = prevop + (newPos % prevopSize);
			csize -= bytesToWrite;
		}
	}

	void AddSourcePos(const opval_t* code_pos, sourceLocation_t sourceLoc) override
	{
	}

	const sizeInfo_t& getSizeInfo() const
	{
		return info;
	}

	opval_t* prevop_end_ptr()
	{
		return prevop + prevopSize;
	}

	opval_t* prevop_last_ptr()
	{
		return prevop + prevopSize - 1;
	}

private:
	sizeInfo_t info;

	static constexpr size_t prevopSize = sizeof(op_offset_t) * 8;
	opval_t prevop[prevopSize];
	opval_t* curop;
};

class mfuse::ScriptProgramManager : public IScriptManager
{
public:
	ScriptProgramManager(ScriptMaster& directorValue, ProgramScript* scriptValue, opval_t* progBuffer, size_t progLength)
		: director(directorValue)
		, script(scriptValue)
		, prog_ptr(progBuffer)
		, prog_end_ptr(progBuffer + progLength)
		, code_pos(progBuffer)
	{
		// clear the program first
		std::fill(prog_ptr, prog_ptr + progLength, 0);
	}

	StateScript* CreateCatchStateScript(const opval_t* try_begin_code_pos, const opval_t* code_pos, size_t labelCount) override
	{
		StateScript* stateScript = script->CreateCatchStateScript(try_begin_code_pos, code_pos);
		stateScript->Reserve(labelCount);
		return stateScript;
	}

	StateScript* CreateSwitchStateScript(size_t labelCount) override
	{
		StateScript* stateScript = script->CreateSwitchStateScript();
		stateScript->Reserve(labelCount);
		return stateScript;
	}

	bool AddLabel(StateScript* stateScript, const prchar_t* name, const opval_t* code_pos, bool privateLabel) override
	{
		return stateScript->AddLabel(name, code_pos, privateLabel);
	}

	bool AddCaseLabel(StateScript* stateScript, const prchar_t* name, const opval_t* code_pos) override
	{
		return stateScript->AddLabel(name, code_pos);
	}

	const_str AddString(const strview& value) override
	{
		return director.AddString(value);
	}

	size_t GetCodePosition() override
	{
		return code_pos - prog_ptr;
	}

	opval_t* GetCodePositionPtr() override
	{
		return code_pos;
	}

	size_t GetProgLength() override
	{
		return prog_end_ptr - prog_ptr;
	}

	opval_t* MoveCodeBack(size_t backCount) override
	{
		opval_t* pos = code_pos;
		code_pos -= backCount;
		return pos;
	}

	opval_t* MoveCodeForward(size_t fwdCount) override
	{
		opval_t* pos = code_pos;
		code_pos += fwdCount;
		return pos;
	}

	void SetValueAtCodePosition(opval_t* pos, const void* value, size_t size) override
	{
		memcpy(pos, value, size);
	}

	void GetValueAt(uintptr_t backOffset, void* value, size_t size) override
	{
		memcpy(value, code_pos + backOffset, size);
	}

	void WriteOpcodeValue(const void* value, size_t size) override
	{
		assert(code_pos + size <= prog_end_ptr);
		memcpy(code_pos, value, size);
		code_pos += size;
	}

	void AddSourcePos(const opval_t* code_pos, sourceLocation_t sourceLoc) override
	{
		script->AddSourcePos(code_pos - prog_ptr, sourceLoc);
	}

private:
	ScriptMaster& director;
	ProgramScript* script;
	opval_t* code_pos;
	opval_t* prog_ptr;
	opval_t* prog_end_ptr;
};

ScriptParser::ScriptParser()
	: info(nullptr)
{
}

const prchar_t* ScriptParser::Preprocess(const prchar_t* sourceBuffer)
{
	// FIXME: Preprocessor (#defines and #includes)
	return sourceBuffer;
}

struct membuf : std::streambuf
{
	membuf(const prchar_t* begin, const prchar_t* end) {
		this->setg((prchar_t*)begin, (prchar_t*)begin, (prchar_t*)end);
	}
};

ParseTree ScriptParser::Parse(const prchar_t* scriptName, const prchar_t* sourceBuffer, uint64_t sourceLength)
{
	ParseTree parseTree;

	membuf buf(sourceBuffer, sourceBuffer + sourceLength);
	std::istream stream(&buf);

	std::ostream* verb = info ? info->GetOutput(outputLevel_e::Verbose) : nullptr;
	Lexer lexer(parseTree, stream, verb);

	Parser parser(lexer, parseTree);

	if (verb)
	{
		lexer.set_debug(1);
		parser.set_debug_level(1);
		parser.set_debug_stream(*verb);
	}

	if (parser.parse() != 0)
	{
		// an error occured
		if (info && info->GetOutput(outputLevel_e::Error))
		{
			std::ostream& out = *info->GetOutput(outputLevel_e::Error);

			out << "parse error: \"" << lexer.exception.msg.c_str() << "\"\n";
			printSourcePos(lexer.exception.loc, scriptName, sourceBuffer, sourceLength, out);
		}

		throw ParseException::ParseError(lexer.exception.text, lexer.exception.msg, lexer.exception.loc);

		// FIXME: Throw
		return {};
	}

	return parseTree;
}

void ScriptParser::SetOutputInfo(OutputInfo* infoValue)
{
	info = infoValue;
}

ScriptEmitter::ScriptEmitter(IScriptManager& managerValue, StateScript& stateScriptValue, OutputInfo* infoValue, size_t maxDepth)
	: manager(managerValue)
	, eventSystem(EventSystem::Get())
	, stateScript(&stateScriptValue)
	, info(infoValue)
	, depth(maxDepth)
	, switchDepth(0)
{
	Reset();
}

void ScriptEmitter::Reset()
{
	//code_pos = nullptr;
	//prog_ptr = nullptr;
	//prog_end_ptr = nullptr;
	current_label = 0;
	info = nullptr;

	for (size_t i = 0; i < BREAK_JUMP_LOCATION_COUNT; i++)
	{
		apucBreakJumpLocations[i] = 0;
		apucContinueJumpLocations[i] = 0;
		prev_opcodes[i].opcode = 0;
		prev_opcodes[i].VarStackOffset = 0;
	}

	iBreakJumpLocCount = 0;
	iContinueJumpLocCount = 0;

	m_iInternalMaxVarStackOffset = 0;
	m_iMaxCallStackOffset = 0;
	m_iMaxExternalVarStackOffset = 0;
	m_iVarStackOffset = 0;

	canBreak = false;
	canContinue = false;
	hasExternal = false;

	prev_opcode_pos = 0;
}

template<typename...Args>
void ScriptEmitter::CompileError(sourceLocation_t sourceLoc, const prchar_t* format, Args&& ...args)
{
	if (info)
	{
		info->Error(format, args...);

		std::ostream* err = info->GetOutput(outputLevel_e::Error);
		if (err)
		{
			*err << std::endl;
			printSourcePos(sourceLoc, fileName, sourceBuffer, sourceLength, *err);
		}
	}
}

opval_t ScriptEmitter::PrevOpcode()
{
	return prev_opcodes[prev_opcode_pos].opcode;
}

prchar_t ScriptEmitter::PrevVarStackOffset()
{
	return prev_opcodes[prev_opcode_pos].VarStackOffset;
}

void ScriptEmitter::AbsorbPrevOpcode()
{
	m_iVarStackOffset -= PrevVarStackOffset();

	//code_pos -= OpcodeLength(PrevOpcode());
	manager.MoveCodeBack(OpcodeLength(PrevOpcode()));

	if (!prev_opcode_pos) {
		prev_opcode_pos = 100;
	}

	prev_opcode_pos--;
}

void ScriptEmitter::ClearPrevOpcode()
{
	prev_opcodes[prev_opcode_pos].opcode = OP_PREVIOUS;
}

void ScriptEmitter::AccumulatePrevOpcode(opval_t op, int32_t iVarStackOffset)
{
	if (IsDebugEnabled())
	{
		*info->GetOutput(outputLevel_e::Debug)
			<< "\t\t"
			<< std::setfill('0') << std::setw(8)
			<< code_pos()
			<< std::setw(0)
			<< ": " << OpcodeName(op) << " (" << m_iVarStackOffset << ") "
			<< (IsExternalOpcode(op) ? "[external]" : "")
			<< std::endl;
	}

	prev_opcode_pos = (prev_opcode_pos + 1) % MAX_PREV_OPCODES;
	prev_opcodes[prev_opcode_pos].opcode = op;
	prev_opcodes[prev_opcode_pos].VarStackOffset = iVarStackOffset;
	prev_opcodes[(prev_opcode_pos + 1) % MAX_PREV_OPCODES].opcode = OP_PREVIOUS;
}

void ScriptEmitter::AddBreakJumpLocation(opval_t* pos, sourceLocation_t sourceLoc)
{
	if (iBreakJumpLocCount < BREAK_JUMP_LOCATION_COUNT)
	{
		apucBreakJumpLocations[iBreakJumpLocCount++] = pos;
	}
	else
	{
		iBreakJumpLocCount = 0;
		CompileError(sourceLoc, "Increase BREAK_JUMP_LOCATION_COUNT and recompile.");
		throw CompileException::BreakJumpLocOverflow(sourceLoc);
	}
}

void ScriptEmitter::AddContinueJumpLocation(opval_t* pos, sourceLocation_t sourceLoc)
{
	if (iContinueJumpLocCount < CONTINUE_JUMP_LOCATION_COUNT)
	{
		apucContinueJumpLocations[iContinueJumpLocCount++] = pos;
	}
	else
	{
		iContinueJumpLocCount = 0;
		CompileError(sourceLoc, "Increase CONTINUE_JUMP_LOCATION_COUNT and recompile.");
		throw CompileException::ContinueJumpLocOverflow(sourceLoc);
	}
}

void ScriptEmitter::AddJumpLocation(opval_t* pos)
{
	op_offset_t offset = (op_offset_t)(code_pos() - sizeof(op_offset_t) - pos);

	manager.SetValueAtCodePosition(pos, &offset, sizeof(op_offset_t));
	//*reinterpret_cast<uint32_t*>(pos) = offset;
	ClearPrevOpcode();
}

void ScriptEmitter::AddJumpBackLocation(opval_t* pos)
{
	op_offset_t offset = (op_offset_t)(code_pos() - pos);

	WriteOpValue<op_offset_t>(offset);
	//*reinterpret_cast<op_offset_t*>(code_pos) = offset;
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();
}

void ScriptEmitter::AddJumpToLocation(opval_t* pos)
{
	op_offset_t offset = (op_offset_t)(pos - code_pos() - 1);

	WriteOpValue<op_offset_t>(offset);
	//*reinterpret_cast<op_offset_t*>(code_pos) = offset;
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();
}

bool ScriptEmitter::BuiltinReadVariable(sourceLocation_t sourceLoc, uint8_t type, const prchar_t* name, uintptr_t eventnum)
{
	const ClassDef* c;

	switch (type)
	{
	case method_game:
		c = nullptr;
		break;

	case method_level:
		c = Level::staticclass();
		break;

	case method_local:
		c = ScriptThread::staticclass();
		break;

	case method_parm:
		c = Parm::staticclass();
		break;

	case method_group:
		c = ScriptClass::staticclass();
		break;

	default:
		return true;
	}

	const EventDef* const def = c->GetDef(eventnum);

	if (def)
	{
		if (def->GetAttributes().GetType() == evType_e::Getter)
		{
			// the event is a valid getter
			return true;
		}
		else
		{
			CompileError(sourceLoc, "Cannot get a write-only variable");
			throw CompileException::WriteOnly(name, sourceLoc);
		}
	}

	return false;
}

bool ScriptEmitter::BuiltinWriteVariable(sourceLocation_t sourceLoc, uint8_t type, const prchar_t* name, uintptr_t eventnum)
{
	const ClassDef* c;

	switch (type)
	{
	case method_game:
		c = nullptr;
		break;

	case method_level:
		c = Level::staticclass();
		break;

	case method_local:
		c = ScriptThread::staticclass();
		break;

	case method_parm:
		c = Parm::staticclass();
		break;

	case method_group:
		c = ScriptClass::staticclass();
		break;

	default:
		return true;
	}

	const EventDef* const def = c->GetDef(eventnum);

	if (def)
	{
		if (def->GetAttributes().GetType() == evType_e::Setter)
		{
			// the event is a valid setter
			return true;
		}
		else
		{
			CompileError(sourceLoc, "Cannot get a read-only variable");
			throw CompileException::ReadOnly(name, sourceLoc);
		}
	}

	return false;
}

void ScriptEmitter::EmitAssignmentStatement(sval_t lhs, sourceLocation_t sourceLoc)
{
	sval_t listener_val;
	const prchar_t* name = lhs.node[2].stringValue;

	if (lhs.node[0].type != statementType_e::Field)
	{
		if (lhs.node[0].type == statementType_e::ArrayExpr)
		{
			EmitRef(lhs.node[1], sourceLoc);
			EmitValue(lhs.node[2]);
			EmitOpcode(OP_LOAD_ARRAY_VAR, *lhs.node[3].sourceLocValue);

			return;
		}
		else
		{
			CompileError(sourceLoc, "bad lvalue: %d (expecting field or array)", lhs.node[0].type);
			throw CompileException::BadLeftValueExpectFieldArray((uint8_t)lhs.node[0].type, sourceLoc);
		}
	}

	const eventName_t eventName = eventSystem.GetEventConstName(name);
	const eventNum_t setterNum = eventSystem.FindSetterEventNum(eventName);

	listener_val = lhs.node[1];

	if (listener_val.node[0].type != statementType_e::Listener || (setterNum && BuiltinWriteVariable(sourceLoc, listener_val.node[1].byteValue, name, setterNum)))
	{
		EmitValue(listener_val);
		EmitOpcode(OP_LOAD_FIELD_VAR, *lhs.node[3].sourceLocValue);
	}
	else
	{
		EmitOpcode(OP_LOAD_GAME_VAR + listener_val.node[1].byteValue, *lhs.node[3].sourceLocValue);
	}

	const_str index = manager.AddString(name);
	WriteOpValue<op_name_t>(index);
	WriteOpValue<op_evName_t>((op_evName_t)eventName);
	//*reinterpret_cast<op_name_t*>(code_pos) = index;
	//code_pos += sizeof(op_name_t);
}

void ScriptEmitter::EmitBoolJumpFalse(sourceLocation_t sourceLoc)
{
	if (PrevOpcode() == OP_UN_CAST_BOOLEAN)
	{
		AbsorbPrevOpcode();
		EmitOpcode(OP_VAR_JUMP_FALSE4, sourceLoc);
	}
	else
	{
		EmitOpcode(OP_BOOL_JUMP_FALSE4, sourceLoc);
	}
}

void ScriptEmitter::EmitBoolJumpTrue(sourceLocation_t sourceLoc)
{
	if (PrevOpcode() == OP_UN_CAST_BOOLEAN)
	{
		AbsorbPrevOpcode();
		EmitOpcode(OP_VAR_JUMP_TRUE4, sourceLoc);
	}
	else
	{
		EmitOpcode(OP_BOOL_JUMP_TRUE4, sourceLoc);
	}
}

void ScriptEmitter::EmitBoolNot(sourceLocation_t sourceLoc)
{
	int prev = PrevOpcode();

	if (prev == OP_BOOL_STORE_TRUE)
	{
		AbsorbPrevOpcode();
		return EmitOpcode(OP_BOOL_STORE_FALSE, sourceLoc);
	}
	else if (prev > OP_BOOL_STORE_TRUE && prev == OP_BOOL_UN_NOT)
	{
		AbsorbPrevOpcode();
		return EmitNil(sourceLoc);
	}
	else if (prev == OP_BOOL_STORE_FALSE)
	{
		AbsorbPrevOpcode();
		return EmitOpcode(OP_BOOL_STORE_TRUE, sourceLoc);
	}

	return EmitOpcode(OP_BOOL_UN_NOT, sourceLoc);
}

void ScriptEmitter::EmitBoolToVar(sourceLocation_t sourceLoc)
{
	if (PrevOpcode() == OP_UN_CAST_BOOLEAN)
	{
		AbsorbPrevOpcode();
		EmitOpcode(OP_UN_CAST_BOOLEAN, sourceLoc);
	}
	else {
		AccumulatePrevOpcode(OP_BOOL_TO_VAR, 0);
	}
}

void ScriptEmitter::EmitBreak(sourceLocation_t sourceLoc)
{
	if (canBreak)
	{
		EmitOpcode(OP_JUMP4, sourceLoc);
		opval_t* pos = code_pos();
		//code_pos += sizeof(op_offset_t);
		manager.MoveCodeForward(sizeof(op_offset_t));
		ClearPrevOpcode();

		AddBreakJumpLocation(pos, sourceLoc);
	}
	else
	{
		CompileError(sourceLoc, "illegal break");
		throw CompileException::IllegalBreak(sourceLoc);
	}
}

void ScriptEmitter::EmitCaseLabel(sval_t case_parm, sval_t parameter_list, sourceLocation_t sourceLoc)
{
	if (case_parm.node[0].type == statementType_e::Integer)
	{
		EmitCaseLabel(case_parm.node[1].intValue, sourceLoc);
	}
	else if (case_parm.node[0].type == statementType_e::String)
	{
		EmitCaseLabel(case_parm.node[1].stringValue, sourceLoc);
	}
	else if (case_parm.node[0].type == statementType_e::Func1Expr && case_parm.node[1].byteValue == OP_UN_MINUS)
	{
		EmitCaseLabel(-(int32_t)case_parm.node[2].node[1].intValue, sourceLoc);
	}
	else
	{
		CompileError(sourceLoc, "bad case value: %d (expected integer or string)", case_parm.node[0].type);
		throw CompileException::BadCaseValueExpectIntString((uint8_t)case_parm.node[0].type, sourceLoc);
	}

	EmitLabelParameterList(parameter_list, sourceLoc);
}

void ScriptEmitter::EmitCaseLabel(const prchar_t* name, sourceLocation_t sourceLoc)
{
	if (IsDebugEnabled())
	{
		*info->GetOutput(outputLevel_e::Debug) << "<" << name << ">:" << std::endl;
	}

	if (!manager.AddCaseLabel(stateScript, name, code_pos()))
	{
		CompileError(sourceLoc, "Duplicate label '%s'", name);
		throw CompileException::DuplicateLabel(name, sourceLoc);
	}
}

void ScriptEmitter::EmitCaseLabel(int32_t label, sourceLocation_t sourceLoc)
{
	prchar_t name[11]{};
	std::to_chars(name, name + sizeof(name), label);

	EmitCaseLabel(name, sourceLoc);
}

void ScriptEmitter::EmitCatch(sval_t val, const opval_t* try_begin_code_pos, sourceLocation_t sourceLoc)
{
	EmitOpcode(OP_JUMP4, sourceLoc);

	opval_t* old_code_pos = code_pos();
	manager.MoveCodeForward(sizeof(op_offset_t));
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();

	ScriptCountManager countManager;
	ScriptEmitter emitter(countManager, *stateScript, info);
	emitter.EmitRoot(val);

	const sizeInfo_t& info = countManager.getSizeInfo();

	StateScript* const oldStateScript = stateScript;
	stateScript = manager.CreateCatchStateScript(try_begin_code_pos, code_pos(), info.numCatchLabels);

	EmitValue(val);

	stateScript = oldStateScript;

	AddJumpLocation(old_code_pos);
}

void ScriptEmitter::EmitCommandMethod(sval_t listener, const prchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc)
{
	const uintptr_t eventnum = eventSystem.FindNormalEventNum(commandName);

	if (!eventnum)
	{
		CompileError(sourceLoc, "unknown command: %s", commandName);
		throw CompileException::UnknownCommand(commandName, sourceLoc);

		EmitValue(listener);
		uint32_t iParamCount = EmitParameterList(parameter_list);

		EmitFunction(iParamCount, commandName, sourceLoc);
		EmitOpcode(OP_LOAD_LOCAL_VAR, sourceLoc);

		WriteOpValue<op_name_t>(STRING_EMPTY);
		//*reinterpret_cast<op_name_t*>(code_pos) = STRING_EMPTY;
		//code_pos += sizeof(op_name_t);
	}
	else
	{
		const uint32_t iParamCount = EmitParameterList(parameter_list);

		EmitValue(listener);

		if (iParamCount > 5)
		{
			EmitOpcodeWithStack(OP_EXEC_CMD_METHOD_COUNT1, -(int32_t)iParamCount - 1, sourceLoc);

			WriteOpValue<op_parmNum_t>(iParamCount);
			//*code_pos++ = iParamCount;
		}
		else
		{
			EmitOpcode(OP_EXEC_CMD_METHOD0 + iParamCount, sourceLoc);
		}

		WriteOpValue<op_ev_t>((op_ev_t)eventnum);
		//*reinterpret_cast<op_ev_t*>(code_pos) = (uint32_t)eventnum;
		//code_pos += sizeof(op_ev_t);
	}
}

void ScriptEmitter::EmitCommandMethodRet(sval_t listener, const prchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc)
{
	const uintptr_t eventnum = eventSystem.FindReturnEventNum(commandName);

	if (!eventnum)
	{
		CompileError(sourceLoc, "unknown return command: %s", commandName);
		throw CompileException::UnknownCommandRet(commandName, sourceLoc);

		const uint32_t iParamCount = EmitParameterList(parameter_list);
		EmitValue(listener);
		EmitFunction(iParamCount, commandName, sourceLoc);
	}
	else
	{
		const uint32_t iParamCount = EmitParameterList(parameter_list);
		EmitValue(listener);
		EmitMethodExpression(iParamCount, eventnum, sourceLoc);
	}
}

void ScriptEmitter::EmitCommandScript(const prchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc)
{
	const uintptr_t eventnum = eventSystem.FindNormalEventNum(commandName);

	if (!eventnum)
	{
		CompileError(sourceLoc, "unknown command: %s", commandName);
		throw CompileException::UnknownCommand(commandName, sourceLoc);

		uint32_t iParamCount = EmitParameterList(parameter_list);
		EmitOpcode(OP_STORE_LOCAL, sourceLoc);
		EmitFunction(iParamCount, commandName, sourceLoc);
		EmitOpcode(OP_LOAD_LOCAL_VAR, sourceLoc);

		WriteOpValue<op_name_t>(STRING_EMPTY);
		//*reinterpret_cast<op_name_t*>(code_pos) = STRING_EMPTY;
		//code_pos += sizeof(op_name_t);
	}
	else
	{
		const uint32_t iParamCount = EmitParameterList(parameter_list);

		if (iParamCount > 5)
		{
			EmitOpcodeWithStack(OP_EXEC_CMD_COUNT1, -(int32_t)iParamCount, sourceLoc);

			WriteOpValue<op_parmNum_t>(iParamCount);
			//*code_pos++ = iParamCount;
		}
		else
		{
			EmitOpcode(OP_EXEC_CMD0 + iParamCount, sourceLoc);
		}

		WriteOpValue<op_ev_t>((op_ev_t)eventnum);
		//*reinterpret_cast<op_ev_t*>(code_pos) = (uint32_t)eventnum;
		//code_pos += sizeof(op_ev_t);
	}
}

void ScriptEmitter::EmitCommandScriptRet(const prchar_t* commandName, sval_t parameter_list, sourceLocation_t sourceLoc)
{
	const uintptr_t eventnum = eventSystem.FindReturnEventNum(commandName);

	if (!eventnum)
	{
		CompileError(sourceLoc, "unknown return command: %s", commandName);
		throw CompileException::UnknownCommandRet(commandName, sourceLoc);
	}
	else
	{
		uint32_t iParamCount = EmitParameterList(parameter_list);
		EmitOpcode(OP_STORE_LOCAL, sourceLoc);
		EmitMethodExpression(iParamCount, eventnum, sourceLoc);
	}
}

void ScriptEmitter::EmitConstArray(sval_t lhs, sval_t rhs, sourceLocation_t sourceLoc)
{
	uint32_t iCount = 1;

	do
	{
		iCount++;

		EmitValue(lhs);

		if (rhs.node->type != statementType_e::ConstArrayExpr) {
			break;
		}

		lhs = rhs.node[1];
		rhs = rhs.node[2];
	} while (rhs.node->type == statementType_e::ConstArrayExpr);

	EmitValue(rhs);
	EmitConstArrayOpcode(iCount, sourceLoc);
}

void ScriptEmitter::EmitConstArrayOpcode(uint32_t iCount, sourceLocation_t sourceLoc)
{
	/*if( iCount > 255 )
	{
		CompileError( -1, "Max const array parameters exceeded") ;
	}
	else
	{
		EmitOpcodeWithStack(OP_LOAD_CONST_ARRAY1, 1 - iCount, sourceLoc);

		*code_pos++ = iCount;
	}*/

	EmitOpcodeWithStack(OP_LOAD_CONST_ARRAY1, 1 - iCount, sourceLoc);

	WriteOpValue<op_arrayParmNum_t>(iCount);
	//*reinterpret_cast<op_arrayParmNum_t*>(code_pos) = iCount;
	//code_pos += sizeof(op_arrayParmNum_t);
}

void ScriptEmitter::EmitContinue(sourceLocation_t sourceLoc)
{
	if (canContinue)
	{
		EmitOpcode(OP_JUMP4, sourceLoc);
		opval_t* pos = code_pos();
		//code_pos += sizeof(op_offset_t);
		manager.MoveCodeForward(sizeof(op_offset_t));
		ClearPrevOpcode();

		AddContinueJumpLocation(pos, sourceLoc);
	}
	else
	{
		CompileError(sourceLoc, "illegal continue");
		throw CompileException::IllegalContinue(sourceLoc);
	}
}

void ScriptEmitter::EmitDoWhileJump(sval_t while_stmt, sval_t while_expr, sourceLocation_t sourceLoc)
{
	opval_t* pos = code_pos();
	uintptr_t label1;
	uintptr_t label2;

	if( IsDebugEnabled() )
	{
		label1 = current_label++;
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label1 << ":" << std::endl;
	}

	ClearPrevOpcode();

	bool old_canBreak = canBreak;
	bool old_canContinue = canContinue;
	uint16_t breakCount = iBreakJumpLocCount;
	uint16_t continueCount = iContinueJumpLocCount;

	canBreak = true;
	canContinue = true;

	EmitValue(while_stmt);

	ProcessContinueJumpLocations(continueCount);

	canContinue = old_canContinue;

	EmitValue(while_expr);
	EmitVarToBool(sourceLoc);

	label2 = EmitNot(sourceLoc);

	opval_t* jmp = code_pos();

	manager.MoveCodeForward(sizeof(op_offset_t));
	//code_pos += sizeof(op_offset_t);

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "JUMP_BACK4 <LABEL" << label1 << ">" << std::endl;
	}

	EmitJumpBack(pos, sourceLoc);

	ClearPrevOpcode();

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label2 << ">:" << std::endl;
	}

	AddJumpLocation(jmp);

	ProcessBreakJumpLocations(breakCount);

	canBreak = old_canBreak;
}

void ScriptEmitter::EmitEof(sourceLocation_t sourceLoc)
{
	if (PrevOpcode() != OP_DONE) {
		EmitOpcode(OP_DONE, sourceLoc);
	} else {
		EmitNil(sourceLoc);
	}
}

void ScriptEmitter::EmitField(sval_t listener_val, sval_t field_val, sourceLocation_t sourceLoc)
{
	const const_str index = manager.AddString(field_val.stringValue);

	const eventName_t eventName = eventSystem.GetEventConstName(field_val.stringValue);
	const eventNum_t getterNum = eventSystem.FindGetterEventNum(eventName);

	//prev_index = *reinterpret_cast<op_name_t*>(code_pos() - sizeof(op_name_t));
	const op_name_t prev_index = ReadOpValue<op_name_t>((intptr_t)sizeof(op_name_t));

	if (listener_val.node[0].type != statementType_e::Listener || (getterNum && BuiltinReadVariable(sourceLoc, listener_val.node[1].byteValue, field_val.stringValue, getterNum)))
	{
		EmitValue(listener_val);
		EmitOpcode(OP_STORE_FIELD, sourceLoc);

		WriteOpValue<op_name_t>(index);
		WriteOpValue<op_evName_t>((op_evName_t)eventName);
		//*reinterpret_cast<op_name_t*>(code_pos) = index;
	}
	else if (PrevOpcode() != (OP_LOAD_GAME_VAR + listener_val.node[1].byteValue) || (const_str)prev_index != index)
	{
		EmitOpcode(OP_STORE_GAME_VAR + listener_val.node[1].byteValue, sourceLoc);

		WriteOpValue<op_name_t>(index);
		WriteOpValue<op_evName_t>((op_evName_t)eventName);
		//*reinterpret_cast<op_name_t*>(code_pos) = index;
	}
	else
	{
		AbsorbPrevOpcode();
		EmitOpcode(OP_LOAD_STORE_GAME_VAR + listener_val.node[1].byteValue, sourceLoc);

		// Move forward because the previous opcode was replaced
		manager.MoveCodeForward(sizeof(op_name_t));
	}

	//code_pos += sizeof(op_name_t);
}

void ScriptEmitter::EmitFloat(float value, sourceLocation_t sourceLoc)
{
	if (value == (float)(int32_t)value)
	{
		// if the float value is the same as an int value
		// then make it an integer directly
		return EmitInteger((uint32_t)value, sourceLoc);
	}

	EmitOpcode(OP_STORE_FLOAT, sourceLoc);

	WriteOpValue<float>(value);
	//*reinterpret_cast<float*>(code_pos) = value;
	//code_pos += sizeof(float);
}

void ScriptEmitter::EmitFunc1(opval_t opcode, sourceLocation_t sourceLoc)
{
	if (opcode == OP_UN_MINUS)
	{
		ScriptVariable var;

		if (EvalPrevValue(var))
		{
			AbsorbPrevOpcode();
			var.minus();

			return EmitValue(var, sourceLoc);
		}
	}

	EmitOpcode(opcode, sourceLoc);
}

void ScriptEmitter::EmitFunction(uint32_t iParamCount, const prchar_t* functionName, sourceLocation_t sourceLoc)
{
	strview filename;
	strview label;
	bool found = false;

	const prchar_t* p = functionName;
	while (*p)
	{
		if (p[0] == ':' && p[1] == ':')
		{
			filename = strview(functionName, p - functionName);
			label = strview(p + 2);

			found = true;

			break;
		}

		p++;
	}

	EmitOpcodeWithStack(OP_FUNC, -(int32_t)iParamCount, sourceLoc);

	if (!found)
	{
		WriteOpValue<bool>(false);
		WriteOpValue<op_name_t>(manager.AddString(functionName));
		//*code_pos++ = false;
		// no file so only put the label
		//*reinterpret_cast<op_name_t*>(code_pos) = Director.AddString(functionName);
	}
	else
	{
		WriteOpValue<bool>(true);
		WriteOpValue<op_name_t>(manager.AddString(functionName));
		WriteOpValue<op_name_t>(manager.AddString(label));
		//*code_pos++ = true;
		//*reinterpret_cast<op_name_t*>(code_pos) = Director.AddString(filename);
		//code_pos += sizeof(op_name_t);
		//*reinterpret_cast<op_name_t*>(code_pos) = Director.AddString(label);
	}

	//code_pos += sizeof(uint32_t);

	WriteOpValue<op_parmNum_t>(iParamCount);
	//*code_pos++ = iParamCount;
}

void ScriptEmitter::EmitIfElseJump(sval_t if_stmt, sval_t else_stmt, sourceLocation_t sourceLoc)
{
	opval_t* jmp1, * jmp2;

	const uintptr_t label1 = EmitNot(sourceLoc);

	jmp1 = code_pos();
	manager.MoveCodeForward(sizeof(op_offset_t));
	//code_pos += sizeof(op_offset_t);
	ClearPrevOpcode();

	EmitValue(if_stmt);

	if( IsDebugEnabled() )
	{
		const uintptr_t label2 = current_label++;
		*info->GetOutput(outputLevel_e::Debug) << "JUMP <LABEL" << label2 << ">" << std::endl;
	}

	EmitOpcode(OP_JUMP4, sourceLoc);
	jmp2 = code_pos();
	manager.MoveCodeForward(sizeof(op_offset_t));
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label1 << ">:" << std::endl;
	}

	AddJumpLocation(jmp1);
	EmitValue(else_stmt);

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label1 << ">:" << std::endl;
	}

	AddJumpLocation(jmp2);
}

void ScriptEmitter::EmitIfJump(sval_t if_stmt, sourceLocation_t sourceLoc)
{
	opval_t* jmp;

	const uintptr_t label = EmitNot(sourceLoc);
	jmp = code_pos();
	manager.MoveCodeForward(sizeof(op_offset_t));
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();

	EmitValue(if_stmt);

	if (IsDebugEnabled())
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label << "" << std::endl;
	}

	AddJumpLocation(jmp);
}

void ScriptEmitter::EmitInteger(uint64_t value, sourceLocation_t sourceLoc)
{
	if (value == 0)
	{
		EmitOpcode(OP_STORE_INT0, sourceLoc);
	}
	else if (value <= (1 << 8))
	{
		EmitOpcode(OP_STORE_INT1, sourceLoc);

		WriteOpValue<uint8_t>(static_cast<uint8_t>(value));
		//*code_pos++ = value;
	}
	else if (value <= (1 << 16))
	{
		EmitOpcode(OP_STORE_INT2, sourceLoc);

		WriteOpValue<uint16_t>(static_cast<uint16_t>(value));
		//*reinterpret_cast<uint16_t*>(code_pos) = value;
		//code_pos += sizeof(uint16_t);
	}
	else if (value <= (1 << 24))
	{
		EmitOpcode(OP_STORE_INT3, sourceLoc);

		WriteOpValue<short3>(static_cast<short3>(value));
		//*reinterpret_cast<short3*>(code_pos) = value;
		//code_pos += sizeof(short3);
	}
	else if(value <= (1ll << 32ll))
	{
		EmitOpcode(OP_STORE_INT4, sourceLoc);

		WriteOpValue<uint32_t>(static_cast<uint32_t>(value));
		//*reinterpret_cast<uint32_t*>(code_pos) = value;
		//code_pos += sizeof(uint32_t);
	}
	else
	{
		// 64-bit value
		EmitOpcode(OP_STORE_INT8, sourceLoc);

		WriteOpValue<uint64_t>(value);
		//*reinterpret_cast<uint64_t*>(code_pos) = value;
		//code_pos += sizeof(uint64_t);
	}
}

void ScriptEmitter::EmitJump(opval_t* pos, sourceLocation_t sourceLoc)
{
	EmitOpcode(OP_JUMP4, sourceLoc);
	AddJumpToLocation(pos);
}

void ScriptEmitter::EmitJumpBack(opval_t* pos, sourceLocation_t sourceLoc)
{
	EmitOpcode(OP_JUMP_BACK4, sourceLoc);
	AddJumpBackLocation(pos);
}

void ScriptEmitter::EmitLabel(const prchar_t* name, sourceLocation_t sourceLoc)
{
	if(IsDebugEnabled())
	{
		*info->GetOutput(outputLevel_e::Debug) << "<" << name << ">:" << std::endl;
	}

	if(!switchDepth)
	{
		if (!manager.AddLabel(stateScript, name, code_pos()))
		{
			CompileError(sourceLoc, "Duplicate label '%s'", name);
			throw CompileException::DuplicateLabel(name, sourceLoc);
		}
	}
	else
	{
		if (!manager.AddCaseLabel(stateScript, name, code_pos()))
		{
			CompileError(sourceLoc, "Duplicate label '%s'", name);
			throw CompileException::DuplicateLabel(name, sourceLoc);
		}
	}
}

void ScriptEmitter::EmitLabel(unsigned int number, sourceLocation_t sourceLoc)
{
	prchar_t name[11]{};
	std::to_chars(name, name + sizeof(name), number);

	EmitLabel(name, sourceLoc);
}

void ScriptEmitter::EmitLabelParameterList(sval_t parameter_list, sourceLocation_t sourceLoc)
{
	ClearPrevOpcode();

	if (parameter_list.node)
	{
		EmitOpcode(OP_MARK_STACK_POS, sourceLoc);

		for(const sval_t* param = parameter_list.node->node; param; param = param[1].node) {
			EmitParameter(param->node, sourceLoc);
		};

		EmitOpcode(OP_RESTORE_STACK_POS, sourceLoc);
	}
}

void ScriptEmitter::EmitLabelPrivate(const prchar_t* name, sourceLocation_t sourceLoc)
{
	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "<" << name << ">:" << std::endl;
	}

	if (!manager.AddLabel(stateScript, name, code_pos(), true))
	{
		CompileError(sourceLoc, "Duplicate label '%s'", name);
		throw CompileException::DuplicateLabel(name, sourceLoc);
	}
}

void ScriptEmitter::EmitAndJump(sval_t logic_stmt, sourceLocation_t sourceLoc)
{
	opval_t* jmp;

	uintptr_t label;
	if( IsDebugEnabled() )
	{
		label = current_label++;
		*info->GetOutput(outputLevel_e::Debug) << "BOOL_LOGICAL_AND <LABEL" << label << ">" << std::endl;
	}

	EmitOpcode(OP_BOOL_LOGICAL_AND, sourceLoc);
	jmp = manager.MoveCodeForward(sizeof(op_offset_t));
	//jmp = code_pos();
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();

	EmitValue(logic_stmt);
	EmitVarToBool(sourceLoc);

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label << ">:" << std::endl;
	}

	AddJumpLocation(jmp);
	//EmitBoolToVar(sourceLoc);
	AccumulatePrevOpcode(OP_BOOL_LOGICAL_AND, 0);
}

void ScriptEmitter::EmitOrJump(sval_t logic_stmt, sourceLocation_t sourceLoc)
{
	opval_t* jmp;

	uintptr_t label;
	if (IsDebugEnabled())
	{
		label = current_label++;
		*info->GetOutput(outputLevel_e::Debug) << "BOOL_LOGICAL_OR <LABEL" << label << ">" << std::endl;
	}

	EmitOpcode(OP_BOOL_LOGICAL_OR, sourceLoc);
	jmp = manager.MoveCodeForward(sizeof(op_offset_t));
	//jmp = code_pos;
	//code_pos += sizeof(op_offset_t);

	ClearPrevOpcode();

	EmitValue(logic_stmt);
	EmitVarToBool(sourceLoc);

	if (IsDebugEnabled())
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label << ">:" << std::endl;
	}

	AddJumpLocation(jmp);
	//EmitBoolToVar(sourceLoc);
	AccumulatePrevOpcode(OP_BOOL_LOGICAL_AND, 0);
}

void ScriptEmitter::EmitMakeArray(sval_t val, sourceLocation_t sourceLoc)
{
	uint32_t iCount = 0;
	for (sval_t* node = val.node[0].node; node != nullptr; iCount++, node = node[1].node)
	{
		EmitValue(node[0]);
	}

	EmitConstArrayOpcode(iCount, sourceLoc);
}

void ScriptEmitter::EmitMethodExpression(uint32_t iParamCount, uintptr_t eventnum, sourceLocation_t sourceLoc)
{
	if (iParamCount > 5)
	{
		EmitOpcodeWithStack(OP_EXEC_METHOD_COUNT1, -(int32_t)iParamCount, sourceLoc);

		WriteOpValue<op_parmNum_t>(iParamCount);
		//*code_pos++ = iParamCount;
	}
	else
	{
		EmitOpcode(OP_EXEC_METHOD0 + iParamCount, sourceLoc);
	}

	WriteOpValue<op_ev_t>(static_cast<op_ev_t>(eventnum));
	//*reinterpret_cast<op_ev_t*>(code_pos) = (op_ev_t)eventnum;
	//code_pos += sizeof(op_ev_t);
}

void ScriptEmitter::EmitNil(sourceLocation_t sourceLoc)
{
	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug)
			<< "\t\t"
			<< std::setfill('0') << std::setw(8)
			<< manager.GetCodePosition()
			<< ":" << std::endl
			<< std::setw(0);
	}
}

void ScriptEmitter::EmitNop()
{
	EmitOpcode(OP_NOP, -1);
}

uintptr_t ScriptEmitter::EmitNot(sourceLocation_t sourceLoc)
{
	uintptr_t label = 0;

	if (PrevOpcode() == OP_BOOL_UN_NOT)
	{
		AbsorbPrevOpcode();

		if( IsDebugEnabled() )
		{
			label = current_label++;
			*info->GetOutput(outputLevel_e::Debug) << "BOOL_JUMP_TRUE <LABEL" << label << ">" << std::endl;
		}

		EmitBoolJumpTrue(sourceLoc);
	}
	else
	{
		if( IsDebugEnabled() )
		{
			label = current_label++;
			*info->GetOutput(outputLevel_e::Debug) << "BOOL_JUMP_FALSE <LABEL" << label << ">" << std::endl;
		}

		EmitBoolJumpFalse(sourceLoc);
	}

	return label;
}

void ScriptEmitter::EmitOpcode(opval_t opcode, sourceLocation_t sourceLoc)
{
	return EmitOpcodeWithStack(opcode, OpcodeVarStackOffset(opcode), sourceLoc);
}

void ScriptEmitter::EmitOpcodeWithStack(opval_t opcode, int32_t varStackOffset, sourceLocation_t sourceLoc)
{
	//assert(code_pos());
	//assert(manager.GetCodePosition() < manager.GetProgLength());

	manager.AddSourcePos(code_pos(), sourceLoc);

	const bool isExternal = IsExternalOpcode(opcode);

	if (isExternal)
	{
		if (m_iVarStackOffset > m_iMaxExternalVarStackOffset) {
			m_iMaxExternalVarStackOffset = m_iVarStackOffset;
		}

		hasExternal = true;
	}

	m_iVarStackOffset += varStackOffset;

	if (!isExternal)
	{
		if (m_iVarStackOffset > m_iInternalMaxVarStackOffset) {
			m_iInternalMaxVarStackOffset = m_iVarStackOffset;
		}
	}

	/*if( m_iInternalMaxVarStackOffset + 9 * m_iMaxExternalVarStackOffset + 1 > 255 )
	{
		CompileError( sourceLoc,
			"The required variable stack size of %d exceeds the statically allocated variable stack of size %d.\nIncrease SCRIPTTHREAD_VARSTACK_SIZE to at least %d and recompile." << std::endl,
			m_iInternalMaxVarStackOffset + 9 * m_iMaxExternalVarStackOffset + 1,
			255,
			m_iInternalMaxVarStackOffset + 9 * m_iMaxExternalVarStackOffset + 1
		);
	}*/

	AccumulatePrevOpcode(opcode, varStackOffset);

	WriteOpValue<opval_t>(opcode);
	//*(code_pos++) = opcode;
}

void ScriptEmitter::EmitParameter(sval_t lhs, sourceLocation_t sourceLoc)
{
	if (lhs.node[0].type != statementType_e::Field)
	{
		CompileError(sourceLoc, "bad parameter lvalue: %d (expecting field)", lhs.node[0].type);
		throw CompileException::BadParameterLValueExpectField((uint8_t)lhs.node[0].type, sourceLoc);
	}

	const sval_t* const listener_val = lhs.node[1].node;
	const prchar_t* const name = lhs.node[2].stringValue;

	const eventName_t eventName = eventSystem.GetEventConstName(name);
	const eventNum_t setterNum = eventSystem.FindSetterEventNum(eventName);

	if (listener_val[0].type != statementType_e::Listener || (setterNum && BuiltinWriteVariable(sourceLoc, listener_val[1].byteValue, name, setterNum)))
	{
		CompileError(sourceLoc, "built-in field '%s' not allowed", name);
		throw CompileException::NotAllowed(name, *lhs.node[3].sourceLocValue);
	}
	else
	{
		EmitOpcode(OP_STORE_PARAM, *lhs.node[3].sourceLocValue);
		EmitOpcode(OP_LOAD_GAME_VAR + listener_val[1].byteValue, *lhs.node[3].sourceLocValue);

		const uint32_t index = manager.AddString(name);
		WriteOpValue<op_name_t>(index);
		WriteOpValue<op_evName_t>(static_cast<op_evName_t>(eventName));

		//*reinterpret_cast<op_name_t*>(code_pos) = index;
		//code_pos += sizeof(op_name_t);
	}
}

uint32_t ScriptEmitter::EmitParameterList(sval_t event_parameter_list)
{
	sval_t* node;
	uint32_t iParamCount = 0;

	if (!event_parameter_list.node) {
		return 0;
	}

	for (node = event_parameter_list.node->node; node; node = node[1].node)
	{
		EmitValue(node->node);

		iParamCount++;
	}

	return iParamCount;
	/*
	const linked_node_t* node;
	uint32_t iParamCount = 0;

	for (node = event_parameter_list.node[1].node[0].linkednode; node; node = (const linked_node_t*)node->next)
	{
		if (node->prev->type == sval_none)
		{
			continue;
		}

		EmitValue(node->prev);

		iParamCount++;
	}

	return iParamCount;
	*/
}

void ScriptEmitter::EmitRef(sval_t val, sourceLocation_t sourceLoc)
{
	if (val.node[0].type != statementType_e::Field)
	{
		if (val.node[0].type == statementType_e::ArrayExpr)
		{
			EmitRef(val.node[1], sourceLoc);
			EmitValue(val.node[2]);
			EmitOpcode(OP_STORE_ARRAY_REF, *val.node[3].sourceLocValue);

			return;
		}
		else
		{
			CompileError(sourceLoc, "bad lvalue: %d (expecting field or array)", val.node[0].type);
			throw CompileException::BadLeftValueExpectFieldArray((uint8_t)val.node[0].type, sourceLoc);
		}
	}

	//index = director.AddString(val.node[2].stringValue);
	const prchar_t* const name = val.node[2].stringValue;
	const const_str index = manager.AddString(name);
	const eventName_t eventName = eventSystem.GetEventConstName(name);

	EmitValue(val.node[1]);
	EmitOpcode(OP_STORE_FIELD_REF, sourceLoc);

	WriteOpValue<op_name_t>(index);
	WriteOpValue<op_evName_t>(static_cast<op_evName_t>(eventName));
	//*reinterpret_cast<op_name_t*>(code_pos) = index;
	//code_pos += sizeof(op_name_t);
}

void ScriptEmitter::EmitStatementList(sval_t val)
{
	for (const sval_t* node = val.node[0].node; node != nullptr; node = node[1].node)
	{
		EmitValue(*node);
	}

	/*
	for (const linked_node_t* node = val.linkednode; node; node = (const linked_node_t*)node->next)
	{
		EmitValue(node->prev->node);
	}
	*/
}

void ScriptEmitter::EmitString(const prchar_t* value, sourceLocation_t sourceLoc)
{
	const_str index = manager.AddString(value);

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "\t\tSTRING \"" << value << "\"" << std::endl;
	}

	EmitOpcode(OP_STORE_STRING, sourceLoc);

	WriteOpValue<op_name_t>(index);
	//*reinterpret_cast<op_name_t*>(code_pos) = index;
	//code_pos += sizeof(op_name_t);
}

void ScriptEmitter::EmitSwitch(sval_t val, sourceLocation_t sourceLoc)
{
	bool bStartCanBreak;
	int iStartBreakJumpLocCount;
	StateScript* oldStateScript;

	++switchDepth;

	ScriptCountManager countManager;
	ScriptEmitter emitter(countManager, *stateScript, info, 2);
	emitter.canBreak = true;
	emitter.switchDepth = 1;
	emitter.EmitRoot(val);

	const sizeInfo_t& info = countManager.getSizeInfo();

	oldStateScript = stateScript;
	// reserve number of case
	stateScript = manager.CreateSwitchStateScript(info.numCaseLabels);

	// predict the number or labels
	//const sizeInfo_t info = CalcValue(val, 2);

	EmitOpcode(OP_SWITCH, sourceLoc);

	WriteOpValue<StateScript*>(stateScript);
	//*reinterpret_cast<StateScript**>(code_pos) = stateScript;
	//code_pos += sizeof(StateScript*);

	bStartCanBreak = canBreak;
	iStartBreakJumpLocCount = iBreakJumpLocCount;

	canBreak = true;

	// add a point to the future switch exit
	EmitBreak(sourceLoc);

	// emit the compound statement of the switch
	EmitValue(val);
	
	// set the exit point for the previously emitted break
	ProcessBreakJumpLocations(iStartBreakJumpLocCount);

	canBreak = bStartCanBreak;
	stateScript = oldStateScript;
	--switchDepth;
}

void ScriptEmitter::EmitValue(ScriptVariable& var, sourceLocation_t sourceLoc)
{
	switch(var.GetType())
	{
	case variableType_e::Integer:
		EmitInteger(var.intValue(), sourceLoc);
		break;
	case variableType_e::Float:
		EmitFloat(var.floatValue(), sourceLoc);
		break;
	default:
		break;
	}
}

void ScriptEmitter::EmitValue(sval_t val)
{
	if (!depth) return;
	else if (depth != -1) --depth;

__emit:

	switch (val.node->type)
	{
	case statementType_e::None:
		break;

	case statementType_e::Next:
		val = val.node[1];
		// prevent stack overflow by calling the function recursively
		goto __emit;

	case statementType_e::StatementList:
		EmitStatementList(val.node[1]);
		break;

	case statementType_e::Labeled:
		EmitLabel(val.node[1].stringValue, *val.node[3].sourceLocValue);
		EmitLabelParameterList(val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::NegIntLabeled:
		val.node[1].intValue = -(int32_t)val.node[1].intValue;

	case statementType_e::IntLabeled:
		EmitCaseLabel(val.node[1], val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::Assignment:
		EmitValue(val.node[2]);
		EmitAssignmentStatement(val.node[1], *val.node[3].sourceLocValue);
		break;

	case statementType_e::If:
		EmitValue(val.node[1]);
		EmitVarToBool(*val.node[3].sourceLocValue);
		EmitIfJump(val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::IfElse:
		EmitValue(val.node[1]);
		EmitVarToBool(*val.node[4].sourceLocValue);
		EmitIfElseJump(val.node[2], val.node[3], *val.node[4].sourceLocValue);
		break;

	case statementType_e::While:
		EmitWhileJump(val.node[1], val.node[2], val.node[3], *val.node[4].sourceLocValue);
		break;

	case statementType_e::LogicalAnd:
		EmitValue(val.node[1]);
		EmitVarToBool(*val.node[3].sourceLocValue);
		EmitAndJump(val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::LogicalOr:
		EmitValue(val.node[1]);
		EmitVarToBool(*val.node[3].sourceLocValue);
		EmitOrJump(val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::MethodEvent:
		EmitCommandMethod(val.node[1], val.node[2].stringValue, val.node[3], *val.node[4].sourceLocValue);
		break;

	case statementType_e::MethodEventExpr:
		EmitCommandMethodRet(val.node[1], val.node[2].stringValue, val.node[3], *val.node[4].sourceLocValue);
		break;

	case statementType_e::CmdEvent:
		EmitCommandScript(val.node[1].stringValue, val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::CmdEventExpr:
		EmitCommandScriptRet(val.node[1].stringValue, val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::Field:
		EmitField(val.node[1], val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::Listener:
		EmitOpcode(OP_STORE_GAME + val.node[1].byteValue, *val.node[2].sourceLocValue);
		break;

	case statementType_e::String:
		EmitString(val.node[1].stringValue, *val.node[2].sourceLocValue);
		break;

	case statementType_e::Integer:
		EmitInteger(val.node[1].intValue, *val.node[2].sourceLocValue);
		break;

	case statementType_e::Float:
		EmitFloat(val.node[1].floatValue, *val.node[2].sourceLocValue);
		break;

	case statementType_e::Vector:
		EmitValue(val.node[1]);
		EmitValue(val.node[2]);
		EmitValue(val.node[3]);
		EmitOpcode(OP_CALC_VECTOR, *val.node[4].sourceLocValue);
		break;

	case statementType_e::NIL:
		EmitOpcode(OP_STORE_NIL, *val.node[1].sourceLocValue);
		break;

	case statementType_e::NULLPTR:
		EmitOpcode(OP_STORE_NULL, *val.node[1].sourceLocValue);
		break;

	case statementType_e::Func1Expr:
		EmitValue(val.node[2]);
		EmitFunc1(val.node[1].byteValue, *val.node[3].sourceLocValue);
		break;

	case statementType_e::Func2Expr:
		EmitValue(val.node[2]);
		EmitValue(val.node[3]);
		EmitOpcode(val.node[1].byteValue, *val.node[4].sourceLocValue);
		break;

	case statementType_e::BoolNot:
		EmitValue(val.node[1]);
		EmitVarToBool(*val.node[2].sourceLocValue);
		EmitBoolNot(*val.node[2].sourceLocValue);
		EmitBoolToVar(*val.node[2].sourceLocValue);
		break;

	case statementType_e::ArrayExpr:
		EmitValue(val.node[1]);
		EmitValue(val.node[2]);
		EmitOpcode(OP_STORE_ARRAY, *val.node[3].sourceLocValue);
		break;

	case statementType_e::ConstArrayExpr:
		EmitConstArray(val.node[1], val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::MakeArray:
		EmitMakeArray(val.node[1], *val.node[2].sourceLocValue);
		break;

	case statementType_e::Try:
	{
		opval_t* old_code_pos = code_pos();

		ClearPrevOpcode();
		EmitValue(val.node[1]);
		EmitCatch(val.node[2], old_code_pos, *val.node[3].sourceLocValue);
		break;
	}

	case statementType_e::Switch:
		EmitValue(val.node[1]);
		EmitSwitch(val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::Break:
		EmitBreak(*val.node[1].sourceLocValue);
		break;

	case statementType_e::Continue:
		EmitContinue(*val.node[1].sourceLocValue);
		break;

	case statementType_e::Do:
		EmitDoWhileJump(val.node[1], val.node[2], *val.node[3].sourceLocValue);
		break;

	case statementType_e::PrivateLabeled:
		EmitLabelPrivate(val.node[1].stringValue, *val.node[3].sourceLocValue);
		EmitLabelParameterList(val.node[2], *val.node[3].sourceLocValue);
		break;

	default:
		CompileError(-1, "unknown type %d", val.node[0].type);
		throw CompileException::UnknownNodeType((uint8_t)val.node->type);
	}

	if(depth != -1) ++depth;
}

void ScriptEmitter::EmitVarToBool(sourceLocation_t sourceLoc)
{
	const opval_t prev = PrevOpcode();

	if (prev == OP_STORE_INT0)
	{
		AbsorbPrevOpcode();
		return EmitOpcode(OP_BOOL_STORE_FALSE, sourceLoc);
	}
	else if (prev > OP_STORE_INT0 && prev <= OP_STORE_INT8)
	{
		AbsorbPrevOpcode();
		return EmitOpcode(OP_BOOL_STORE_TRUE, sourceLoc);
	}
	else if (prev == OP_BOOL_TO_VAR)
	{
		AbsorbPrevOpcode();
		return EmitNil(sourceLoc);
	}

	return EmitOpcode(OP_UN_CAST_BOOLEAN, sourceLoc);
}

void ScriptEmitter::EmitWhileJump(sval_t while_expr, sval_t while_stmt, sval_t inc_stmt, sourceLocation_t sourceLoc)
{
	opval_t* pos = code_pos();
	uintptr_t label1 = 0;

	if( IsDebugEnabled() )
	{
		label1 = current_label++;
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label1 << ">:" << std::endl;
	}

	ClearPrevOpcode();

	EmitValue(while_expr);
	EmitVarToBool(sourceLoc);

	const uintptr_t label2 = EmitNot(sourceLoc);
	//opval_t* jmp = code_pos;
	//code_pos += sizeof(op_offset_t);
	opval_t* jmp = manager.MoveCodeForward(sizeof(op_offset_t));
	ClearPrevOpcode();

	bool old_canBreak = canBreak;
	bool old_canContinue = canContinue;
	int breakCount = iBreakJumpLocCount;
	int continueCount = iContinueJumpLocCount;

	canBreak = true;
	canContinue = true;

	EmitValue(while_stmt);
	ProcessContinueJumpLocations(continueCount);

	canContinue = old_canContinue;

	EmitValue(inc_stmt);

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "JUMP_BACK4 <LABEL" << label1 << ">" << std::endl;
	}

	EmitJumpBack(pos, sourceLoc);

	ClearPrevOpcode();

	if( IsDebugEnabled() )
	{
		*info->GetOutput(outputLevel_e::Debug) << "<LABEL" << label2 << ">:" << std::endl ;
	}

	AddJumpLocation(jmp);

	ProcessBreakJumpLocations(breakCount);

	canBreak = old_canBreak;
}

void ScriptEmitter::EmitRoot(sval_t root)
{
	EmitValue(root);
	EmitEof(-1);
}

bool ScriptEmitter::EvalPrevValue(ScriptVariable& var)
{
	switch (PrevOpcode())
	{
	case OP_STORE_INT0:
		var.setIntValue(0);
		break;

	case OP_STORE_INT1:
		//var.setIntValue(*(code_pos - sizeof(prchar_t)));
		var.setIntValue(ReadOpValue<uint8_t>(sizeof(uint8_t)));
		break;

	case OP_STORE_INT2:
		//var.setIntValue(*reinterpret_cast<int16_t*>(code_pos - sizeof(int16_t)));
		var.setIntValue(ReadOpValue<uint16_t>(sizeof(uint16_t)));
		break;

	case OP_STORE_INT3:
		//var.setIntValue(*reinterpret_cast<short3*>(code_pos - sizeof(short3)));
		var.setIntValue(ReadOpValue<short3>(sizeof(short3)));
		break;

	case OP_STORE_INT4:
		//var.setIntValue(*reinterpret_cast<int32_t*>(code_pos - sizeof(int32_t)));
		var.setIntValue(ReadOpValue<uint32_t>(sizeof(uint32_t)));
		break;

	case OP_STORE_INT8:
		//var.setLongValue(*reinterpret_cast<int32_t*>(code_pos - sizeof(int32_t)));
		var.setLongValue(ReadOpValue<uint64_t>(sizeof(uint64_t)));
		break;

	case OP_STORE_FLOAT:
		//var.setFloatValue(*reinterpret_cast<float*>(code_pos - sizeof(float)));
		var.setFloatValue(ReadOpValue<float>(sizeof(float)));
		return true;

	default:
		return false;
	}

	return true;
}

void ScriptEmitter::OptimizeInstructions(opval_t* code, opval_t* op1, opval_t* op2)
{
	int intValue1 = 0, intValue2 = 0, intValue3 = 0;
	float floatValue1 = 0.0f, floatValue2 = 0.0f, floatValue3 = 0.0f;
	int type1, type2;

	if (!(*op1 >= OP_STORE_INT0 && *op1 <= OP_STORE_FLOAT &&
		*op2 >= OP_STORE_INT0 && *op2 <= OP_STORE_FLOAT
		))
	{
		return;
	}

	if (*op1 >= OP_STORE_INT0 && *op1 <= OP_STORE_INT4)
	{
		memcpy(&intValue1, op1 + 1, *op1 - OP_STORE_INT0);
		type1 = OP_STORE_INT4;
	}
	else
	{
		type1 = *op1;
	}

	if (*op2 >= OP_STORE_INT0 && *op2 <= OP_STORE_INT4)
	{
		memcpy(&intValue2, op2 + 1, *op2 - OP_STORE_INT0);
		type2 = OP_STORE_INT4;
	}
	else
	{
		type2 = *op2;
	}

	if (*op1 == OP_STORE_FLOAT)
	{
		memcpy(&floatValue1, op1 + 1, sizeof(float));
	}

	if (*op2 == OP_STORE_FLOAT)
	{
		memcpy(&floatValue2, op2 + 1, sizeof(float));
	}

	*op1 = OP_NOP;
	*op2 = OP_NOP;

	switch (type1 + type2 * OP_MAX)
	{
	case OP_STORE_INT4 + OP_STORE_INT4 * OP_MAX:
		*op1 = OP_STORE_INT4;
		*(uint32_t*)(op1 + 1) = OptimizeValue(intValue1, intValue2, *code);
		break;
	}

	*code = OP_NOP;
}

int ScriptEmitter::OptimizeValue(int val1, int val2, opval_t opcode)
{
	switch (opcode)
	{
	case OP_BIN_BITWISE_AND:
		return val1 & val2;
	case OP_BIN_BITWISE_OR:
		return val1 | val2;
	case OP_BIN_BITWISE_EXCL_OR:
		return val1 ^ val2;
	case OP_BIN_EQUALITY:
		return val1 == val2;
	case OP_BIN_INEQUALITY:
		return val1 != val2;
	case OP_BIN_LESS_THAN:
		return val1 > val2;
	case OP_BIN_GREATER_THAN:
		return val1 < val2;
	case OP_BIN_LESS_THAN_OR_EQUAL:
		return val1 >= val2;
	case OP_BIN_GREATER_THAN_OR_EQUAL:
		return val1 <= val2;
	case OP_BIN_PLUS:
		return val1 + val2;
	case OP_BIN_MINUS:
		return val1 - val2;
	case OP_BIN_MULTIPLY:
		return val1 * val2;
	case OP_BIN_DIVIDE:
		return val1 / val2;
	case OP_BIN_PERCENTAGE:
		return val1 % val2;
	default:
		return 0;
	}
}

void ScriptEmitter::ProcessBreakJumpLocations(int iStartBreakJumpLocCount)
{
	if (iBreakJumpLocCount > iStartBreakJumpLocCount)
	{
		do
		{
			iBreakJumpLocCount--;

			op_offset_t offset = (op_offset_t)(code_pos() - apucBreakJumpLocations[iBreakJumpLocCount] - sizeof(op_offset_t));

			manager.SetValueAtCodePosition(apucBreakJumpLocations[iBreakJumpLocCount], &offset, sizeof(offset));
			//*reinterpret_cast<uint32_t*>(apucBreakJumpLocations[iBreakJumpLocCount]) = offset;
		} while (iBreakJumpLocCount > iStartBreakJumpLocCount);

		ClearPrevOpcode();
	}
}

void ScriptEmitter::ProcessContinueJumpLocations(int iStartContinueJumpLocCount)
{
	if (iContinueJumpLocCount > iStartContinueJumpLocCount)
	{
		do
		{
			iContinueJumpLocCount--;

			const op_offset_t offset = (op_offset_t)(code_pos() - sizeof(op_offset_t) - apucContinueJumpLocations[iContinueJumpLocCount]);

			manager.SetValueAtCodePosition(apucContinueJumpLocations[iBreakJumpLocCount], &offset, sizeof(offset));
			//*reinterpret_cast<uint32_t*>(apucContinueJumpLocations[iContinueJumpLocCount]) = offset;
		} while (iContinueJumpLocCount > iStartContinueJumpLocCount);

		ClearPrevOpcode();
	}
}

bool ScriptEmitter::GetCompiledScript(ProgramScript* scr)
{
	/*
	Archiver arc;

	arc.SetSilent( true );
	arc.Read( scr->Filename(), false );

	if( !arc.NoErrors() )
	{
		return false;
	}

	arc.SetSilent( false );

	scr->Archive( arc );

	arc.Close();

	return true;
	*/
	return false;
}

bool ScriptEmitter::IsDebugEnabled() const
{
	return info && info->GetOutput(outputLevel_e::Debug) != nullptr;
}

opval_t* ScriptEmitter::code_pos() const
{
	return manager.GetCodePositionPtr();
}

uint32_t ScriptEmitter::GetInternalMaxVarStackOffset() const
{
	return m_iInternalMaxVarStackOffset;
}

uint32_t ScriptEmitter::GetMaxExternalVarStackOffset() const
{
	return m_iMaxExternalVarStackOffset;
}

ScriptCompiler::ScriptCompiler()
	: director(ScriptContext::Get().GetDirector())
{
}

size_t ScriptCompiler::Compile(ProgramScript* script, sval_t rootNode, opval_t*& progBuffer)
{
	// preallocate the right amount of memory as long as it's possible
	// this has the advantage to :
	//  1) avoid the fragmentation
	//  2) avoid reallocating containers each time
	//  3) perform allocation only once, thus avoid lock contention and avoid monopolizing the memory manager
	//  4) if there is no memory available, it will not halt in the middle of the compilation so there is no cleanup to do
	const size_t progLength = Preallocate(script, rootNode, progBuffer);
	compileSuccess = true;

	// start emitting program instructions
	EmitProgram(script, rootNode, progBuffer, progLength);
	return progLength;
}

void ScriptCompiler::EmitProgram(ProgramScript* script, sval_t rootNode, opval_t*& progBuffer, size_t progLength)
{
	ScriptProgramManager manager(director, script, progBuffer, progLength);
	// invoke the ScriptEmitter and use the program manager interface to write opcodes
	// and to create state scripts
	ScriptEmitter emitter(manager, script->GetStateScript(), info);
	emitter.EmitRoot(rootNode);

	m_iInternalMaxVarStackOffset = emitter.GetInternalMaxVarStackOffset();
	m_iMaxExternalVarStackOffset = emitter.GetMaxExternalVarStackOffset();
}

size_t ScriptCompiler::Preallocate(ProgramScript* script, sval_t rootNode, opval_t*& progBuffer)
{
	ScriptCountManager manager;

	// invoke the script emitter
	// and connect the interface for counting opcodes and stateScripts
	ScriptEmitter emitter(manager, script->GetStateScript(), info);
	emitter.EmitRoot(rootNode);

	MEM::PreAllocator& allocator = script->GetAllocator();

	const sizeInfo_t& sizeInfo = manager.getSizeInfo();

	size_t totalAllocation = sizeInfo.progLength;
	totalAllocation += sizeof(StateScript) * sizeInfo.numSwitches;
	totalAllocation += sizeof(CatchBlock) * sizeInfo.numCatches;
	totalAllocation += labelMap::countEntryBytes(sizeInfo.numLabels);
	totalAllocation += labelMap::countEntryBytes(sizeInfo.numCaseLabels);

	const bool isDeveloperMode = ScriptContext::Get().GetSettings().IsDeveloperEnabled();
	if (isDeveloperMode)
	{
		// also preallocate length for source map
		totalAllocation += sizeof(sourcePosMap_t) * sizeInfo.progLength; //sourcePosMap_t::countEntryBytes(sizeInfo.progLength);
	}
	allocator.PreAllocate(totalAllocation);

	if (isDeveloperMode)
	{
		// create program source map to be able to view error location
		// and also allocate only once
		script->CreateProgToSource(sizeInfo.progLength);
	}

	if (sizeInfo.numStrings)
	{
		// preallocate enough strings to avoid fragmenting memory due to frequent reallocation of the table
		director.AllocateMoreString(sizeInfo.numStrings);
	}

	if (sizeInfo.numCatches)
	{
		// now allocate try/catch state scripts
		script->ReserveCatchStates(sizeInfo.numCatches);
	}

	if (sizeInfo.numSwitches)
	{
		// and also allocate switch state scripts
		script->ReserveStateScripts(sizeInfo.numSwitches);
	}

	// noew allocate the program buffer
	progBuffer = new (allocator) opval_t [sizeInfo.progLength];

	// reserve the initial label and other labels
	script->GetStateScript().Reserve(sizeInfo.numLabels);

	return sizeInfo.progLength;
}

void ScriptCompiler::Optimize(opval_t* sourceBuffer)
{
#if 0
	size_t length = code_pos - prog_ptr;
	opval_t* prevcodePos1 = nullptr;
	opval_t* prevcodePos2 = nullptr;

	code_pos = sourceBuffer;

	if (!length)
	{
		return;
	}

	while (code_pos < prog_end_ptr)
	{
		if (*code_pos >= OP_BIN_BITWISE_AND && *code_pos <= OP_BIN_PERCENTAGE)
		{
			assert(prevcodePos1);
			assert(prevcodePos2);

			OptimizeInstructions(code_pos, prevcodePos1, prevcodePos2);
		}

		prevcodePos2 = prevcodePos1;
		prevcodePos1 = code_pos;

		code_pos += OpcodeLength(*code_pos);

		if (*code_pos == OP_DONE) {
			break;
		}
	}
#endif
}

uint32_t ScriptCompiler::GetInternalMaxVarStackOffset() const
{
	return m_iInternalMaxVarStackOffset;
}

uint32_t ScriptCompiler::GetMaxExternalVarStackOffset() const
{
	return m_iMaxExternalVarStackOffset;
}

void ScriptCompiler::SetOutputInfo(OutputInfo* infoValue)
{
	info = infoValue;
}

bool ScriptCompiler::IsDebugEnabled() const
{
	return info && info->GetOutput(outputLevel_e::Debug) != nullptr;
}

sizeInfo_t::sizeInfo_t()
	: length(0)
	, numLabels(0)
	, numCaseLabels(0)
	, numCatchLabels(0)
	, numCommands(0)
	, numStrings(0)
	, numCatches(0)
	, numSwitches(0)
	, progLength(0)
{}
