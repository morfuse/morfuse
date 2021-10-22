#pragma once

#include "../Common/MEM/PreAllocator.h"
#include "AbstractScript.h"
#include "StateScript.h"

namespace mfuse
{
	class Listener;
	class ScriptThread;
	class ProgramScript;
	class ScriptContext;

	class CatchBlock
	{
	public:
		CatchBlock(ProgramScript* inParent, const opval_t* inTryStart, const opval_t* inTryEnd);

		const opval_t* GetTryStartCodePos() const;
		const opval_t* GetTryEndCodePos() const;

		StateScript& GetStateScript();

	private:
		// program variable
		StateScript m_StateScript;

		// code position variables
		const opval_t* m_TryStartCodePos;
		const opval_t* m_TryEndCodePos;
	};

	class ProgramScript : public AbstractScript
	{
	public:
		ProgramScript(const_str fileName);
		~ProgramScript();

		void Archive(Archiver& arc);
		static void Archive(Archiver& arc, const ProgramScript *&scr);
		void ArchiveCodePos(Archiver& arc, const opval_t*& codePos) const;

		void Close();
		void Load(std::istream& stream);

		bool GetCodePos(opval_t *codePos, const_str& filename, uintptr_t& pos);
		bool SetCodePos(opval_t *&codePos, const_str filename, uintptr_t pos);

		bool IsCompileSuccess() const;
		size_t GetRequiredStackSize() const;

		bool labelExists(const_str labelName);

		const opval_t* GetProgBuffer() const;
		size_t GetProgLength() const;

		void ReserveCatchStates(size_t count);
		void ReserveStateScripts(size_t count);

		StateScript& GetStateScript();
		const StateScript& GetStateScript() const;
		StateScript* CreateCatchStateScript(const opval_t* try_begin_code_pos, const opval_t* try_end_code_pos);
		StateScript* CreateSwitchStateScript();

		StateScript* GetCatchStateScript(const opval_t* in, const opval_t*& out) const;

	private:
		StateScript m_State;
		opval_t* m_ProgBuffer;
		size_t m_ProgLength;

		// compile variables
		bool successCompile;

		// stack variables
		unsigned int requiredStackSize;

	protected:
		// try/throw variable
		con::Container<CatchBlock, MEM::ChildPreAllocator> m_CatchBlocks;
		con::Container<StateScript, MEM::ChildPreAllocator> m_StateScripts;
	};
};
