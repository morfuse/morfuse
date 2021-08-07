#pragma once

#include "Listener.h"
#include "ScriptOpcodes.h"

namespace mfuse
{
	class ProgramScript;
	class ScriptThread;
	class ScriptVM;
	class StateScript;

	class ScriptClass : public Listener
	{
		MFUS_CLASS_PROTOTYPE(ScriptClass);

	public:
		ScriptClass(const ProgramScript *gameScript, Listener *self);
		ScriptClass();
		virtual ~ScriptClass();

		void* operator new(size_t size);
		void operator delete(void* ptr);

		mfuse_EXPORTS const xstr& Filename() const;
		mfuse_EXPORTS const opval_t* FindLabel(const rawchar_t* label) const;
		mfuse_EXPORTS const opval_t* FindLabel(const_str label) const;
		mfuse_EXPORTS Listener* GetSelf() const;
		mfuse_EXPORTS void SetSelf(Listener* l);

		void Archive(Archiver& arc) override;
		void ArchiveInternal(Archiver& arc);
		static void ArchiveScript(Archiver& arc, ScriptClass **obj);
		void ArchiveCodePos(Archiver& arc, opval_t **codePos);

		ScriptClass* GetNext() const;
		ScriptClass* GetPrev() const;

		ScriptThread* CreateThreadInternal(const ScriptVariable& label) override;
		ScriptThread* CreateScriptInternal(const ScriptVariable& label) override;

		void AddThread(ScriptVM * m_ScriptVM);
		void KillThreads();
		void RemoveThread(ScriptVM * m_ScriptVM);

		StateScript* GetCatchStateScript(const opval_t *in, const opval_t *&out) const;

		const ProgramScript* GetScript() const;

	private:
		/** Current game script. */
		const ProgramScript* m_Script;

		/** The self variable. */
		ListenerPtr m_Self;

		/** The first thread in list. */
		ScriptVM* m_Threads;

		ScriptClass* Next;
		ScriptClass* Prev;
	};
}
