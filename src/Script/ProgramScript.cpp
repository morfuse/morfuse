#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/SourceException.h>
#include <morfuse/Script/Context.h>
#include "Compiler.h"

#include <iostream>

using namespace mfuse;

ProgramScript::ProgramScript(const_str fileName)
	: m_State(this)
{
	successCompile = false;

	m_ProgBuffer = nullptr;
	m_ProgLength = 0;
	m_ProgToSource = nullptr;

	m_SourceBuffer = nullptr;
	m_SourceLength = 0;

	m_Filename = fileName;

	requiredStackSize = 0;

	m_CatchBlocks.GetAllocator().SetAllocator(GetAllocator());
	m_StateScripts.GetAllocator().SetAllocator(GetAllocator());
}

ProgramScript::~ProgramScript()
{
	Close();
}

struct pfixup_t {
	bool			isString;
	unsigned int	*ptr;
};

#if 0
static con::Container< const_str > archivedEvents;
static con::Container< const_str > archivedStrings;
static con::Container< pfixup_t * > archivedPointerFixup;

void ArchiveOpcode(Archiver& arc, opval_t *code)
{
	/*
	unsigned int index;

	arc.ArchiveByte(code);

	switch (*code)
	{
	case OP_STORE_NIL:
	case OP_STORE_NULL:
	case OP_DONE:
		break;

	case OP_EXEC_CMD_COUNT1:
	case OP_EXEC_CMD_METHOD_COUNT1:
	case OP_EXEC_METHOD_COUNT1:
		arc.ArchiveByte(code + 1);
		goto __exec;

	case OP_EXEC_CMD0:
	case OP_EXEC_CMD1:
	case OP_EXEC_CMD2:
	case OP_EXEC_CMD3:
	case OP_EXEC_CMD4:
	case OP_EXEC_CMD5:
	case OP_EXEC_CMD_METHOD0:
	case OP_EXEC_CMD_METHOD1:
	case OP_EXEC_CMD_METHOD2:
	case OP_EXEC_CMD_METHOD3:
	case OP_EXEC_CMD_METHOD4:
	case OP_EXEC_CMD_METHOD5:
	case OP_EXEC_METHOD0:
	case OP_EXEC_METHOD1:
	case OP_EXEC_METHOD2:
	case OP_EXEC_METHOD3:
	case OP_EXEC_METHOD4:
	case OP_EXEC_METHOD5:
		code--;
	__exec:
		if (!arc.Loading())
		{
			index = archivedEvents.AddUniqueObject(*reinterpret_cast<unsigned int *>(code + 2));
		}

		arc.ArchiveUnsigned(&index);

		if (arc.Loading())
		{
			pfixup_t *p = new pfixup_t;

			p->isString = false;
			p->ptr = reinterpret_cast<unsigned int *>(code + 2);

			*reinterpret_cast<unsigned int *>(code + 2) = index;
			archivedPointerFixup.AddObject(p);
		}
		break;

	case OP_LOAD_FIELD_VAR:
	case OP_LOAD_GAME_VAR:
	case OP_LOAD_GROUP_VAR:
	case OP_LOAD_LEVEL_VAR:
	case OP_LOAD_LOCAL_VAR:
	case OP_LOAD_OWNER_VAR:
	case OP_LOAD_PARM_VAR:
	case OP_LOAD_SELF_VAR:
	case OP_LOAD_STORE_GAME_VAR:
	case OP_LOAD_STORE_GROUP_VAR:
	case OP_LOAD_STORE_LEVEL_VAR:
	case OP_LOAD_STORE_LOCAL_VAR:
	case OP_LOAD_STORE_OWNER_VAR:
	case OP_LOAD_STORE_PARM_VAR:
	case OP_LOAD_STORE_SELF_VAR:
	case OP_STORE_FIELD:
	case OP_STORE_FIELD_REF:
	case OP_STORE_GAME_VAR:
	case OP_STORE_GROUP_VAR:
	case OP_STORE_LEVEL_VAR:
	case OP_STORE_LOCAL_VAR:
	case OP_STORE_OWNER_VAR:
	case OP_STORE_PARM_VAR:
	case OP_STORE_SELF_VAR:
	case OP_STORE_STRING:
		if (!arc.Loading())
		{
			index = archivedStrings.AddUniqueObject(*reinterpret_cast<unsigned int *>(code + 1));
		}

		arc.ArchiveUnsigned(&index);

		if (arc.Loading())
		{
			pfixup_t *p = new pfixup_t;

			p->isString = true;
			p->ptr = reinterpret_cast<unsigned int *>(code + 1);

			*reinterpret_cast<unsigned int *>(code + 1) = index;
			archivedPointerFixup.AddObject(p);
		}
		break;

	default:
		if (OpcodeLength(*code) > 1)
			arc.ArchiveRaw(code + 1, OpcodeLength(*code) - 1);
	}
	*/
}

template<>
void con::Entry <opval_t *, sourceinfo_t>::Archive(Archiver& arc)
{
	/*
	unsigned int offset;

	if (arc.Loading())
	{
		arc.ArchiveUnsigned(&offset);
		key = current_progBuffer + offset;
	}
	else
	{
		offset = key - current_progBuffer;
		arc.ArchiveUnsigned(&offset);
	}

	arc.ArchiveUnsigned(&value.sourcePos);
	arc.ArchiveInteger(&value.column);
	arc.ArchiveInteger(&value.line);
	*/
}
#endif

void ProgramScript::Archive(Archiver& arc)
{
	/*
	int count = 0, i;
	opval_t *p, *code_pos, *code_end;
	const_str s;
	command_t *c, cmd;

	arc.ArchiveSize((long *)&m_ProgLength);

	if (arc.Saving())
	{
		p = m_ProgBuffer;
		current_progBuffer = m_ProgBuffer;

		// archive opcodes
		while (*p != OP_DONE)
		{
			ArchiveOpcode(arc, p);

			p += OpcodeLength(*p);
		}

		ArchiveOpcode(arc, p);

		// archive string dictionary list
		i = archivedStrings.NumObjects();
		arc.ArchiveInteger(&i);

		for (; i > 0; i--)
		{
			GetScriptManager()->ArchiveString(arc, archivedStrings.ObjectAt(i));
		}

		// archive event list
		i = archivedEvents.NumObjects();
		arc.ArchiveInteger(&i);

		for (; i > 0; i--)
		{
			c = Event::GetEventInfo(archivedEvents.ObjectAt(i));

			arc.ArchiveString(&c->command);
			arc.ArchiveInteger(&c->flags);
			arc.ArchiveByte(&c->type);
		}
	}
	else
	{
		m_ProgBuffer = (opval_t *)glbs.Malloc(m_ProgLength);
		code_pos = m_ProgBuffer;
		code_end = m_ProgBuffer + m_ProgLength;

		current_progBuffer = m_ProgBuffer;

		do
		{
			ArchiveOpcode(arc, code_pos);

			code_pos += OpcodeLength(*code_pos);
		} while (*code_pos != OP_DONE && arc.NoErrors());

		if (!arc.NoErrors())
		{
			return;
		}

		// retrieve the string dictionary list
		arc.ArchiveInteger(&i);
		archivedStrings.Resize(i + 1);

		for (; i > 0; i--)
		{
			GetScriptManager()->ArchiveString(arc, s);
			archivedStrings.AddObjectAt(i, s);
		}

		// retrieve the event list
		arc.ArchiveInteger(&i);
		archivedEvents.Resize(i + 1);

		for (; i > 0; i--)
		{
			arc.ArchiveString(&cmd.command);
			arc.ArchiveInteger(&cmd.flags);
			arc.ArchiveByte(&cmd.type);

			archivedEvents.AddObjectAt(i, Event::GetEventWithFlags(cmd.command, cmd.flags, cmd.type));
		}

		// fix program string/event pointers
		for (i = archivedPointerFixup.NumObjects(); i > 0; i--)
		{
			pfixup_t *fixup = archivedPointerFixup.ObjectAt(i);

			if (fixup->isString)
			{
				*fixup->ptr = archivedStrings.ObjectAt(*fixup->ptr);
			}
			else
			{
				*fixup->ptr = archivedEvents.ObjectAt(*fixup->ptr);
			}

			delete fixup;
		}

		successCompile = true;
	}

	// cleanup
	archivedStrings.FreeObjectList();
	archivedEvents.FreeObjectList();
	archivedPointerFixup.FreeObjectList();

	if (!arc.Loading())
	{
		if (m_ProgToSource)
		{
			count = m_ProgToSource->size();
			arc.ArchiveInteger(&count);

			m_ProgToSource->Archive(arc);
		}
		else
		{
			arc.ArchiveInteger(&count);
		}
	}
	else
	{
		arc.ArchiveInteger(&count);

		if (count)
		{
			m_ProgToSource = new con_set < opval_t *, sourceinfo_t >;
			m_ProgToSource->Archive(arc);
		}
	}

	arc.ArchiveUnsigned(&requiredStackSize);
	arc.ArchiveBool(&m_bPrecompiled);

	if (!m_bPrecompiled && arc.Loading())
	{
		fileHandle_t filehandle = nullptr;

		m_SourceLength = glbs.FS_ReadFile(Filename().c_str(), (void **)&m_SourceBuffer, true);

		if (m_SourceLength > 0)
		{
			m_SourceBuffer = (rawchar_t *)glbs.Malloc(m_SourceLength);

			glbs.FS_Read(m_SourceBuffer, m_SourceLength, filehandle);
			glbs.FS_FCloseFile(filehandle);
		}
	}

	m_State->Archive(arc);

	current_progBuffer = nullptr;
	*/
}

void ProgramScript::Archive(Archiver& arc, ProgramScript *& scr)
{
	/*
	xstr filename;

	if (!arc.Saving())
	{
		arc.ArchiveString(&filename);

		if (filename != "")
		{
			scr = GetScriptManager()->GetScript(filename);
		}
		else
		{
			scr = nullptr;
		}
	}
	else
	{
		filename = scr->Filename();

		arc.ArchiveString(&filename);
	}
	*/
}

void ProgramScript::ArchiveCodePos(Archiver& arc, opval_t **codePos) const
{
	/*
	int pos = 0;
	xstr filename;

	if (!arc.Saving())
	{
		pos = *codePos - m_ProgBuffer;
		if (pos >= 0 && pos < m_ProgLength)
		{
			filename = Filename();
		}
	}

	arc.ArchiveInteger(&pos);
	arc.ArchiveString(&filename);

	if (arc.Loading())
	{
		if (Filename() == filename)
		{
			*codePos = m_ProgBuffer + pos;
		}
	}
	*/
}

void ProgramScript::Close()
{
/*
	for (uintptr_t i = m_CatchBlocks.NumObjects(); i > 0; i--)
	{
		delete m_CatchBlocks.ObjectAt(i);
	}
*/
	m_CatchBlocks.FreeObjectList();
	m_StateScripts.FreeObjectList();

	if (m_ProgToSource)
	{
		delete m_ProgToSource;
		m_ProgToSource = nullptr;
	}

	if (m_ProgBuffer)
	{
		//delete[] m_ProgBuffer;
		m_ProgBuffer = nullptr;
	}

	if (m_SourceBuffer)
	{
		delete[] (rawchar_t*)m_SourceBuffer;
		m_SourceBuffer = nullptr;
	}

	m_ProgLength = 0;
	m_SourceLength = 0;
}

void ProgramScript::Load(const void *sourceBuffer, uint64_t sourceLength)
{
	char* newBuf = new char[sourceLength + 1];
	newBuf[sourceLength] = 0;
	memcpy(newBuf, sourceBuffer, sourceLength);

	m_SourceBuffer = newBuf;
	m_SourceLength = sourceLength;

	try
	{
		ScriptParser parser;
		OutputInfo& info = ScriptContext::Get().GetOutputInfo();
		parser.SetOutputInfo(&info);
		// preprocess the buffer
		const char* m_PreprocessedBuffer = parser.Preprocess(m_SourceBuffer);
		// parse the buffer into an Abstract Syntax Tree
		const ParseTree parseTree = parser.Parse(Filename().c_str(), m_PreprocessedBuffer, sourceLength);

		ScriptCompiler Compiler;
		Compiler.SetOutputInfo(&info);
		// compile the AST into opcodes
		m_ProgLength = Compiler.Compile(this, parseTree.getRootNode(), m_ProgBuffer);

		// get the required stack size for the VM to preallocate enough stack
		requiredStackSize = Compiler.GetInternalMaxVarStackOffset() + 9 * Compiler.GetMaxExternalVarStackOffset() + 1;

		successCompile = true;
	}
	catch(ParseException::Base&)
	{
		std::ostream* err = ScriptContext::Get().GetOutputInfo().GetOutput(outputLevel_e::Error);
		if (err) *err << "^~^~^ Script file compile error:  Couldn't parse '" << Filename().c_str() << "'\n";
		Close();
		throw;
	}
	catch (CompileException::Base&)
	{
		std::ostream* err = ScriptContext::Get().GetOutputInfo().GetOutput(outputLevel_e::Error);
		if (err) *err << "^~^~^ Script file compile error:  Couldn't compile '" << Filename().c_str() << "'\n";
		Close();
		throw;
	}
	catch(std::exception& e)
	{
		std::ostream* err = ScriptContext::Get().GetOutputInfo().GetOutput(outputLevel_e::Error);
		if (err) *err << "^~^~^ Unknown script compile error:  Couldn't load '" << Filename().c_str() << "': " << e.what() << "\n";
		Close();
		throw;
	}
}

bool ProgramScript::GetCodePos(opval_t *codePos, xstr& filename, uintptr_t& pos)
{
	pos = codePos - m_ProgBuffer;

	if (pos >= 0 && pos < m_ProgLength)
	{
		filename = Filename();
		return true;
	}
	else
	{
		return false;
	}
}

bool ProgramScript::SetCodePos(opval_t *&codePos, const rawchar_t* filename, uintptr_t pos)
{
	if (Filename() == filename)
	{
		codePos = m_ProgBuffer + pos;
		return true;
	}
	else
	{
		return false;
	}
}

size_t ProgramScript::GetRequiredStackSize() const
{
	return requiredStackSize;
}

bool ProgramScript::labelExists(const rawchar_t *name)
{
	xstr labelname;

	// if we got passed a nullptr than that means just run the script so of course it exists
	if (!name)
	{
		return true;
	}

	if (m_State.FindLabel(name))
	{
		return true;
	}

	return false;
}

StateScript& ProgramScript::GetStateScript()
{
	return m_State;
}

const StateScript& ProgramScript::GetStateScript() const
{
	return m_State;
}

StateScript *ProgramScript::CreateCatchStateScript(const opval_t* try_begin_code_pos, const opval_t* try_end_code_pos)
{
	CatchBlock *catchBlock = new (m_CatchBlocks) CatchBlock(this, try_begin_code_pos, try_end_code_pos);
	return &catchBlock->GetStateScript();
}

StateScript *ProgramScript::CreateSwitchStateScript()
{
	return new (m_StateScripts) StateScript(this);
}

StateScript *ProgramScript::GetCatchStateScript(const opval_t* in, const opval_t*&out) const
{
	CatchBlock *bestCatchBlock = nullptr;

	for (intptr_t i = m_CatchBlocks.NumObjects(); i > 0; i--)
	{
		CatchBlock& catchBlock = m_CatchBlocks.ObjectAt(i);

		if (in >= catchBlock.GetTryStartCodePos() && in < catchBlock.GetTryEndCodePos())
		{
			if (!bestCatchBlock || catchBlock.GetTryEndCodePos() < bestCatchBlock->GetTryStartCodePos())
			{
				bestCatchBlock = &catchBlock;
			}
		}
	}

	if (bestCatchBlock)
	{
		out = bestCatchBlock->GetTryEndCodePos();

		return &bestCatchBlock->GetStateScript();
	}
	else
	{
		return nullptr;
	}
}

bool ProgramScript::IsCompileSuccess() const
{
	return successCompile;
}

const opval_t* ProgramScript::GetProgBuffer() const
{
	return m_ProgBuffer;
}

size_t ProgramScript::GetProgLength() const
{
	return m_ProgLength;
}

void ProgramScript::ReserveCatchStates(size_t count)
{
	m_CatchBlocks.Resize(count);
}

void ProgramScript::ReserveStateScripts(size_t count)
{
	m_StateScripts.Resize(count);
}

CatchBlock::CatchBlock(ProgramScript* inParent, const opval_t* inTryStart, const opval_t* inTryEnd)
	: m_StateScript(inParent)
	, m_TryStartCodePos(inTryStart)
	, m_TryEndCodePos(inTryEnd)
{
}

const opval_t* CatchBlock::GetTryStartCodePos() const
{
	return m_TryStartCodePos;
}

const opval_t* CatchBlock::GetTryEndCodePos() const
{
	return m_TryEndCodePos;
}

StateScript& CatchBlock::GetStateScript()
{
	return m_StateScript;
}
