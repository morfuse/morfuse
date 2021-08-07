#include <morfuse/Script/ScriptClass.h>
#include <morfuse/Script/ScriptThread.h>
#include <morfuse/Script/ProgramScript.h>
#include <morfuse/Script/ScriptVariable.h>
#include <morfuse/Script/ScriptException.h>
#include <morfuse/Script/ScriptVM.h>
#include <morfuse/Script/ScriptMaster.h>
#include <morfuse/Script/Context.h>

using namespace mfuse;

MFUS_CLASS_DECLARATION(Listener, ScriptClass, NULL)
{
	{ NULL, NULL }
};

ScriptClass::ScriptClass(const ProgramScript *gameScript, Listener *self)
{
	m_Self = self;
	m_Script = gameScript;
	m_Threads = NULL;

	ScriptMaster& Director = ScriptContext::Get().GetDirector();
	LL::SafeAddFront<ScriptClass*, &ScriptClass::Next, &ScriptClass::Prev>(Director.ContainerHead, this);
}

ScriptClass::ScriptClass()
{
	m_Self = NULL;
	m_Script = NULL;
	m_Threads = NULL;

	ScriptMaster& Director = ScriptContext::Get().GetDirector();
	LL::SafeAddFront<ScriptClass*, &ScriptClass::Next, &ScriptClass::Prev>(Director.ContainerHead, this);
}

ScriptClass::~ScriptClass()
{
	if (m_Script == NULL)
	{
		//ScriptError("Attempting to delete dead class.");
		return;
	}

	ScriptMaster& Director = ScriptContext::Get().GetDirector();
	LL::SafeRemoveRoot<ScriptClass*, &ScriptClass::Next, &ScriptClass::Prev>(Director.ContainerHead, this);

	KillThreads();

	if (!m_Script->ConstFilename())
	{
		// This is a temporary gamescript so delete it
		delete m_Script;
	}
}

void* ScriptClass::operator new(size_t size)
{
	return ScriptContext::Get().GetAllocator().GetBlock<ScriptClass>().Alloc();
	//return allocateMemory(size);
}

void ScriptClass::operator delete(void* ptr)
{
	ScriptContext::Get().GetAllocator().GetBlock<ScriptClass>().Free(ptr);
	//return freeMemory(ptr);
}

void ScriptClass::Archive(Archiver& arc)
{
}

void ScriptClass::ArchiveInternal(Archiver& arc)
{
	Listener::Archive(arc);

	/*
	arc.ArchiveObjectPosition(this);
	arc.ArchiveSafePointer(&m_Self);
	ProgramScript::Archive(arc, m_Script);
	*/
}

void ScriptClass::ArchiveScript(Archiver& arc, ScriptClass **obj)
{
	/*
	ScriptClass *scr;
	ScriptVM *m_current;
	ScriptThread *m_thread;
	int num;
	int i;

	if (arc.Saving())
	{
		scr = *obj;
		scr->ArchiveInternal(arc);

		num = 0;
		for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next)
			num++;

		arc.ArchiveInteger(&num);

		for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next)
			m_current->m_Thread->ArchiveInternal(arc);
	}
	else
	{
		scr = new ScriptClass();
		scr->ArchiveInternal(arc);

		arc.ArchiveInteger(&num);

		for (i = 0; i < num; i++)
		{
			m_thread = new ScriptThread(scr, NULL);
			m_thread->ArchiveInternal(arc);
		}

		*obj = scr;
	}
	*/
}

void ScriptClass::ArchiveCodePos(Archiver& arc, opval_t **codePos)
{
	m_Script->ArchiveCodePos(arc, codePos);
}

ScriptThread *ScriptClass::CreateThreadInternal(const ScriptVariable& label)
{
	const ProgramScript *scr;
	ScriptThread *thread = NULL;

	ScriptMaster& Director = ScriptContext::Get().GetDirector();

	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		ScriptClass *scriptClass = Director.CurrentScriptClass();
		scr = scriptClass->GetScript();

		if (label.GetType() == variableType_e::ConstString)
			thread = Director.CreateScriptThread(scr, m_Self, label.constStringValue());
		else
			thread = Director.CreateScriptThread(scr, m_Self, label.stringValue());
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		const ScriptVariable *script = label[1];
		const ScriptVariable *labelname = label[2];

		if (script->GetType() == variableType_e::ConstString)
			scr = Director.GetGameScript(script->constStringValue());
		else
			scr = Director.GetGameScript(script->stringValue());

		if (labelname->GetType() == variableType_e::ConstString)
			thread = Director.CreateScriptThread(scr, m_Self, labelname->constStringValue());
		else
			thread = Director.CreateScriptThread(scr, m_Self, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptClass::CreateThreadInternal: bad argument format");
	}

	return thread;
}

ScriptThread *ScriptClass::CreateScriptInternal(const ScriptVariable& label)
{
	ScriptThread *thread = nullptr;

	ScriptMaster& Director = ScriptContext::Get().GetDirector();

	if (label.GetType() == variableType_e::String || label.GetType() == variableType_e::ConstString)
	{
		if (label.GetType() == variableType_e::ConstString)
			thread = Director.CreateScriptThread(Director.GetGameScript(label.stringValue()), m_Self, "");
		else
			thread = Director.CreateScriptThread(Director.GetGameScript(label.constStringValue()), m_Self, "");
	}
	else if (label.GetType() == variableType_e::ConstArray && label.arraysize() > 1)
	{
		const ProgramScript* scr;
		const ScriptVariable *script = label[1];
		const ScriptVariable *labelname = label[2];

		if (script->GetType() == variableType_e::ConstString)
			scr = Director.GetGameScript(script->constStringValue());
		else
			scr = Director.GetGameScript(script->stringValue());

		if (labelname->GetType() == variableType_e::ConstString)
			thread = Director.CreateScriptThread(scr, m_Self, labelname->constStringValue());
		else
			thread = Director.CreateScriptThread(scr, m_Self, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptClass::CreateScriptInternal: bad label type '%s'", label.GetTypeName());
	}

	return thread;
}

void ScriptClass::AddThread(ScriptVM *m_ScriptVM)
{
	m_ScriptVM->SetNext(m_Threads);
	m_Threads = m_ScriptVM;
}

void ScriptClass::KillThreads()
{
	if (!m_Threads)
	{
		return;
	}

	ScriptVM *m_current;
	ScriptVM *m_next;

	m_current = m_Threads;

	do
	{
		m_current->ClearScriptClass();

		m_next = m_current->GetNext();
		delete m_current->GetScriptThread();

	} while ((m_current = m_next));

	m_Threads = NULL;
}

void ScriptClass::RemoveThread(ScriptVM *m_ScriptVM)
{
	if (m_Threads == m_ScriptVM)
	{
		m_Threads = m_ScriptVM->GetNext();

		if (m_Threads == NULL) {
			delete this;
		}
	}
	else
	{
		ScriptVM *m_current = m_Threads;
		ScriptVM *i;

		for (i = m_Threads->GetNext(); i != m_ScriptVM; i = i->GetNext()) {
			m_current = i;
		}

		m_current->SetNext(i->GetNext());
	}
}

const xstr& ScriptClass::Filename() const
{
	return m_Script->Filename();
}

const opval_t *ScriptClass::FindLabel(const rawchar_t* label) const
{
	return m_Script->GetStateScript().FindLabel(label);
}

const opval_t *ScriptClass::FindLabel(const_str label) const
{
	return m_Script->GetStateScript().FindLabel(label);
}

StateScript *ScriptClass::GetCatchStateScript(const opval_t *in, const opval_t *&out) const
{
	return m_Script->GetCatchStateScript(in, out);
}

const ProgramScript *ScriptClass::GetScript() const
{
	return m_Script;
}

Listener *ScriptClass::GetSelf() const
{
	return static_cast<Listener *>(m_Self.Pointer());
}

void ScriptClass::SetSelf(Listener* l)
{
	m_Self = l;
}

ScriptClass* ScriptClass::GetNext() const
{
	return Next;
}

ScriptClass* ScriptClass::GetPrev() const
{
	return Prev;
}
