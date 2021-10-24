#pragma once

#include "Listener.h"
#include "ScriptOpcodes.h"
#include "StringResolvable.h"

namespace mfuse
{
	class ProgramScript;
	class ScriptMaster;
	class ScriptThread;
	class ScriptVM;
	class StateScript;
	struct script_label_t;

	class ScriptClass : public Listener
	{
		MFUS_CLASS_PROTOTYPE(ScriptClass);

		friend class ScriptMaster;

	public:
		ScriptClass(const ProgramScript *gameScript, Listener *self);
		ScriptClass(ScriptMaster& director);
		ScriptClass();
		virtual ~ScriptClass();

		void* operator new(size_t size);
		void operator delete(void* ptr);

		mfuse_EXPORTS const_str Filename() const;
		mfuse_EXPORTS const script_label_t* FindLabel(const_str label) const;
		mfuse_EXPORTS Listener* GetSelf() const;
		mfuse_EXPORTS void SetSelf(Listener* l);

		void Archive(Archiver& arc) override;
		void ArchiveInternal(Archiver& arc);
		static void ArchiveScript(ScriptMaster& director, Archiver& arc, ScriptClass*& classRef);
		void ArchiveCodePos(Archiver& arc, const opval_t*& codePos);

		ScriptClass* GetNext() const;
		ScriptClass* GetPrev() const;

		ScriptThread* CreateThreadInternal(const ScriptVariable& label) override;
		ScriptThread* CreateScriptInternal(const ScriptVariable& label) override;

		void AddThread(ScriptVM * m_ScriptVM);
		void KillThreads();
		void RemoveThread(ScriptVM * m_ScriptVM);
		ScriptVM* FirstThread() const;
		ScriptVM* NextThread(ScriptVM* vm) const;
		const ScriptVM* NextThread(const ScriptVM* vm) const;

		StateScript* GetCatchStateScript(const opval_t *in, const opval_t *&out) const;

		const ProgramScript* GetScript() const;

	private:
		/** Current game script. */
		const ProgramScript* m_Script;

		/** The self variable. */
		ListenerPtr m_Self;

		/** The first thread in list. */
		ScriptVM* m_Threads;

		ScriptClass*& headScript;
		ScriptClass* Next;
		ScriptClass* Prev;
	};
}
